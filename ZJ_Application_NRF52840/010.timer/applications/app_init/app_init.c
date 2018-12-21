#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "nrf_log.h"
#include "board.h"
#include "app_init.h"
#include "app_uart.h"
#include "app_gpio.h"

static rt_timer_t zj_timer1 = RT_NULL;
static rt_timer_t zj_timer2 = RT_NULL;

static uint32_t last_ticks1 = 0;
static uint32_t last_ticks2 = 0;

static void zj_timer1_callback(void* parameter)
{	
    uint32_t tick_cnt1;
    tick_cnt1 = (uint32_t)rt_tick_get(); 
    rt_kprintf("Timer1回调时间差= %dms\n", TICK_TO_OS_MSEC(ABS(tick_cnt1 - last_ticks1))); 
    last_ticks1 = tick_cnt1;
}

static void zj_timer2_callback(void* parameter)
{
    static bool pin_toggle=false;
    uint32_t tick_cnt2;
    tick_cnt2 = (uint32_t)rt_tick_get();
    rt_kprintf("Timer2回调时间差= %dms\n", TICK_TO_OS_MSEC(ABS(tick_cnt2 - last_ticks2)));
    last_ticks2 = tick_cnt2;
    app_led_cortrol(ZJ_LED2, pin_toggle);
    pin_toggle =! pin_toggle;
}

int zj_timer_test(void)
{
    rt_kprintf("ZJ-SDK:RT-Thread定时器在ZJ曹孟德开发板上的应用\n");   
    rt_kprintf("zj_timer1_callback 4s后只会执行一次就注销\n");
    rt_kprintf("zj_timer2_callback 会每隔2s循环执行\n");
    zj_timer1 = rt_timer_create("zj_timer1_callback",
                            zj_timer1_callback,
                            0, 
                            MSEC_TO_OS_TICK(4000),
                            RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    APP_ERROR_CHECK(zj_timer1==RT_NULL);
    if (zj_timer1 != RT_NULL)
    {
       rt_timer_start(zj_timer1);
    }
    
    zj_timer2 = rt_timer_create("zj_timer2_callback", 
                            zj_timer2_callback,
                            0, 
                            MSEC_TO_OS_TICK(2000), 
                            RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    APP_ERROR_CHECK(zj_timer2==RT_NULL);  
    if (zj_timer2 != RT_NULL)
    {
        rt_timer_start(zj_timer2);
    }
    last_ticks1 = (uint32_t)rt_tick_get();
    last_ticks2 = last_ticks1;
}
FINSH_FUNCTION_EXPORT_ALIAS(zj_timer_test, zj_ttest, this is RT-Thraed timer test)
FINSH_FUNCTION_EXPORT_ALIAS(zj_timer_test, __cmd_zj_ttest, this is RT-Thraed timer test)

int main(void)
{
//    uint32_t cnt=0;
    bool pin_toggle=false;
    NRF_LOG_INFO("RT-Thread for nrf52840 started.");
    while(1)
    {
       app_led_cortrol(ZJ_LED1, pin_toggle); 
       rt_thread_mdelay(500);
//       NRF_LOG_INFO("This is nrf_log test %d.",cnt);
//       rt_kprintf("This is rt_kprintf test %d.",cnt);
//       cnt++;
        pin_toggle = !pin_toggle;
    }
    return RT_TRUE;
}


/**
 * @}
 */
