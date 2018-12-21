#include "board.h"
#include "app_util_platform.h"
#include "nrf_drv_common.h"
#include "nrf_systick.h"
#include "nrf_rtc.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include <rtthread.h>
#include <rthw.h>

#define RT_USE_LOW_POWER_IDLE 1
#define TICK_RATE_HZ  RT_TICK_PER_SECOND
#define SYSTICK_CLOCK_HZ  ( 32768UL )

#define NRF_RTC_REG        NRF_RTC1
    /* IRQn used by the selected RTC */
#define NRF_RTC_IRQn       RTC1_IRQn
    /* Constants required to manipulate the NVIC. */
#define NRF_RTC_PRESCALER  ( (uint32_t) (ROUNDED_DIV(SYSTICK_CLOCK_HZ, TICK_RATE_HZ) - 1) )
    /* Maximum RTC ticks */
#define NRF_RTC_MAXTICKS   ((1U<<24)-1U)

static volatile uint32_t m_tick_overflow_count = 0;
#define NRF_RTC_BITWIDTH 24
#define OSTick_Handler     RTC1_IRQHandler
#define EXPECTED_IDLE_TIME_BEFORE_SLEEP   2

void SysTick_Configuration(void)
{
    nrf_drv_clock_lfclk_request(NULL);

    /* Configure SysTick to interrupt at the requested rate. */
    nrf_rtc_prescaler_set(NRF_RTC_REG, NRF_RTC_PRESCALER);

    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_TICK);
    nrf_rtc_int_enable(NRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_OVERFLOW);
    nrf_rtc_int_enable(NRF_RTC_REG, NRF_RTC_INT_OVERFLOW_MASK);

    nrf_rtc_task_trigger(NRF_RTC_REG, NRF_RTC_TASK_CLEAR);
    nrf_rtc_task_trigger(NRF_RTC_REG, NRF_RTC_TASK_START);


    NVIC_SetPriority(NRF_RTC_IRQn, 0xF);
    NVIC_EnableIRQ(NRF_RTC_IRQn);
}

void OSTick_Handler( void )
{
#if RT_USE_LOW_POWER_IDLE
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);
#endif
    rt_interrupt_enter();

    uint32_t systick_counter = nrf_rtc_counter_get(NRF_RTC_REG);
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_TICK);
    uint32_t diff;
    diff =  (systick_counter - rt_tick_get()) & NRF_RTC_MAXTICKS;

    while ((diff--) > 0)
    {
        if (rt_thread_self() != RT_NULL)
        {
          rt_tick_increase();
        }
    }
    /* leave interrupt */
    rt_interrupt_leave();
}

static void _sleep_ongo( uint32_t sleep_tick )
{
    uint32_t enterTime;
    uint32_t entry_tick;
    uint32_t wakeupTime;

    /* Make sure the SysTick reload value does not overflow the counter. */
    if ( sleep_tick > NRF_RTC_MAXTICKS - EXPECTED_IDLE_TIME_BEFORE_SLEEP )
    {
        sleep_tick = NRF_RTC_MAXTICKS - EXPECTED_IDLE_TIME_BEFORE_SLEEP;
    }

     rt_enter_critical();
   
    enterTime = nrf_rtc_counter_get(NRF_RTC_REG);
    wakeupTime = (enterTime + sleep_tick) & NRF_RTC_MAXTICKS;
    /* Stop tick events */
    nrf_rtc_int_disable(NRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

    /* Configure COMPARE interrupt */
    nrf_rtc_cc_set(NRF_RTC_REG, 0, wakeupTime);
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);
    nrf_rtc_int_enable(NRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);

    entry_tick = rt_tick_get();

    __DSB();
    /* No SD -  we would just block interrupts globally.
    * BASEPRI cannot be used for that because it would prevent WFE from wake up.
    */
    __WFE();
    __WFE();

    nrf_rtc_int_disable(NRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);

    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_TICK);
    nrf_rtc_int_enable (NRF_RTC_REG, NRF_RTC_INT_TICK_MASK);
  
    rt_exit_critical();
}

void rt_hw_system_powersave(void)
{
#if RT_USE_LOW_POWER_IDLE
    uint32_t sleep_tick;   
    sleep_tick = rt_timer_next_timeout_tick() - rt_tick_get();
    if(sleep_tick > EXPECTED_IDLE_TIME_BEFORE_SLEEP)
    {
        _sleep_ongo( sleep_tick );
    }
#endif
}

void rt_hw_console_output(const char *str)
{
#if !defined(RETARGET_ENABLED) || RETARGET_ENABLED == 0
   printf("%s",str);
#elif NRF_MODULE_ENABLED(NRF_LOG) && defined(NRF_LOG_DEFERRED)&& NRF_LOG_DEFERRED == 1
     NRF_LOG_INFO("%s",str);
#else
     SEGGER_RTT_printf(0,"%s",str);
#endif
}

void rt_hw_board_init(void)
{
//    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    nrf_drv_clock_init();
    SysTick_Configuration();

    rt_thread_idle_sethook(rt_hw_system_powersave);

#ifdef RT_USING_HEAP
    rt_system_heap_init((void*)NRF_HEAP_BEGIN, (void*)CHIP_HEAP_END);
#endif

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
    SCB->CPACR |= 0xf << 20;
        /* Lazy save always. */
    FPU->FPCCR |= FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk;

    /* Finally this port requires SEVONPEND to be active */
    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
}

