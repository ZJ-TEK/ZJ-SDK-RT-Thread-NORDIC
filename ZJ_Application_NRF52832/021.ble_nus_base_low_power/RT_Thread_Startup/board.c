#include "board.h"
#include "app_util_platform.h"
#include "nrf_drv_common.h"
#include "nrf_systick.h"
#include "nrf_rtc.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_sdh.h"
#include <rtthread.h>
#include <rthw.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "SEGGER_RTT.h"

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

#define board_printf(...) 	do{NRF_LOG_INFO(__VA_ARGS__);/*rt_kprintf(__VA_ARGS__);*/}while(0)
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
#ifdef SOFTDEVICE_PRESENT
    do{
        uint8_t dummy = 0;
        uint32_t err_code = sd_nvic_critical_region_enter(&dummy);
        APP_ERROR_CHECK(err_code);
    }while (0);
#endif 

    enterTime = nrf_rtc_counter_get(NRF_RTC_REG);
    wakeupTime = (enterTime + sleep_tick) & NRF_RTC_MAXTICKS;
    /* Stop tick events */
    nrf_rtc_int_disable(NRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

    /* Configure COMPARE interrupt */
    nrf_rtc_cc_set(NRF_RTC_REG, 0, wakeupTime);
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);
    nrf_rtc_int_enable(NRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);

    __DSB();
#ifdef SOFTDEVICE_PRESENT
    if (nrf_sdh_is_enabled())
    {
        uint32_t err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
#endif
    /* No SD -  we would just block interrupts globally.
    * BASEPRI cannot be used for that because it would prevent WFE from wake up.
    */
    do{
        __WFE();
    } while (0 == (NVIC->ISPR[0] | NVIC->ISPR[1]));

    nrf_rtc_int_disable(NRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);

    uint32_t level;
    level = rt_hw_interrupt_disable();

    /* Correct the system ticks */
    {
       uint32_t diff;
       uint32_t exitTime;
       
       nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_TICK);
       nrf_rtc_int_enable (NRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

       exitTime = nrf_rtc_counter_get(NRF_RTC_REG);
       diff =  (exitTime - enterTime) & NRF_RTC_MAXTICKS;
       /* It is important that we clear pending here so that our corrections are latest and in sync with tick_interrupt handler */
       NVIC_ClearPendingIRQ(NRF_RTC_IRQn);
       rt_tick_set(rt_tick_get() + diff);
       board_printf("exitTime =%d ,enterTime =%d,diff = %d\r\n",exitTime ,enterTime,  (exitTime - enterTime) & NRF_RTC_MAXTICKS);
       if (rt_thread_self() != RT_NULL)
       {
       	  struct rt_thread *thread;
       
           /* check time slice */
           thread = rt_thread_self();
           if (thread->remaining_tick <= diff)
           {
               /* change to initialized tick */
               thread->remaining_tick = thread->init_tick;
       
               /* yield */
               rt_thread_yield();
           }
           else
           {
               thread->remaining_tick -= diff;
           }
       
           /* check timer */
           rt_timer_check();
       }
    }   
    rt_hw_interrupt_enable(level);
        
#ifdef SOFTDEVICE_PRESENT
    uint32_t err_code = sd_nvic_critical_region_exit(0);
    APP_ERROR_CHECK(err_code);
#endif
    rt_exit_critical();
}

void rt_hw_system_powersave(void)
{
#if NRF_MODULE_ENABLED(NRF_LOG)
    rt_thread_resume(rt_thread_find("nrf_log"));  //resume nrf_log thread
#endif
#if RT_USE_LOW_POWER_IDLE
    uint32_t sleep_tick;   
    sleep_tick = rt_timer_next_timeout_tick() - rt_tick_get();
    if(sleep_tick > EXPECTED_IDLE_TIME_BEFORE_SLEEP)
    {
        _sleep_ongo( sleep_tick );
    }
#endif
}

#if NRF_MODULE_ENABLED(NRF_LOG) && NRF_MODULE_ENABLED(NRF_LOG_BACKEND_RTT)
static void Log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void task_my_logger_process(void *parameter)
{
    UNUSED_PARAMETER(parameter);
    while(1)
    {
      NRF_LOG_FLUSH();
      rt_thread_suspend(rt_thread_self());// Suspend myself
    }
}

static int nrf_log_thread_init(void)
{
    rt_thread_t tid;
    #define LOG_THREAD_STACK_SIZE     1024
    #define LOG_THREAD_PRIORITY       (RT_THREAD_PRIORITY_MAX-1)

    tid = rt_thread_create("nrf_log", task_my_logger_process, RT_NULL,
                           LOG_THREAD_STACK_SIZE, LOG_THREAD_PRIORITY-1, 20);
    RT_ASSERT(tid != RT_NULL);
    rt_thread_startup(tid);
    return 0;
}
INIT_APP_EXPORT(nrf_log_thread_init);
#endif

void rt_hw_console_output(const char *str)
{
#if !defined(RETARGET_ENABLED) || RETARGET_ENABLED == 0
   printf("%s",str);
#elif NRF_MODULE_ENABLED(NRF_LOG) && defined(NRF_LOG_DEFERRED)&& NRF_LOG_DEFERRED == 1
     NRF_LOG_INFO("%s",str);
#endif
}

void rt_hw_board_init(void)
{
//    sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    nrf_drv_clock_init();
    SysTick_Configuration();

#if NRF_MODULE_ENABLED(NRF_LOG) && NRF_MODULE_ENABLED(NRF_LOG_BACKEND_RTT)
    Log_init();
#endif

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

