#include <rtthread.h>
#include <rthw.h>
#include <drivers/pin.h>
#include "app_gpio.h"
#include "app_timer.h"

#define gpio_printf(...) //rt_kprintf(__VA_ARGS__)
static void key_handle(void *args)
{
   static bool pin_toggle=false;
   nrf_gpiote_polarity_t polarity = (nrf_gpiote_polarity_t)((uint32_t)args >>6);
   nrfx_gpiote_pin_t pin =  (uint32_t)args & 0x3F;
   if(pin == ZJ_KEY1 && polarity == NRF_GPIOTE_POLARITY_HITOLO)
   {
      rt_pin_write(ZJ_LED2, pin_toggle);
      pin_toggle = !pin_toggle;
   }
}
//FINSH_FUNCTION_EXPORT(key_handle, key_handle(139) led2 will toggle)
FINSH_FUNCTION_EXPORT_ALIAS(key_handle, key,  key(139) led2 will toggle)

static int led_cortrol(uint8_t ctl )
{
    if(ctl == 31)
    {
       rt_pin_write(ZJ_LED3, 0);
    }
    else if(ctl == 30)
    {
       rt_pin_write(ZJ_LED3, 1);
    }
    else if(ctl == 41)
    {
       rt_pin_write(ZJ_LED4, 0);
    }
    else if(ctl == 40)
    {
       rt_pin_write(ZJ_LED4, 1);
    }
    return 0;
}
//FINSH_FUNCTION_EXPORT(led_cortrol, led_cortrol led3(31/30) and led4(41/40))
FINSH_FUNCTION_EXPORT_ALIAS(led_cortrol, led, led(xx) led3(31/30) and led4(41/40))

void app_led_cortrol(uint32_t pin,uint8_t state)
{
   rt_pin_write(pin,state);
}

static int led_key_init(void)
{
    rt_pin_mode(ZJ_LED1,PIN_MODE_OUTPUT);
    rt_pin_mode(ZJ_LED2,PIN_MODE_OUTPUT);
    rt_pin_mode(ZJ_LED3,PIN_MODE_OUTPUT);
    rt_pin_mode(ZJ_LED4,PIN_MODE_OUTPUT);

    rt_pin_write(ZJ_LED1,PIN_HIGH);
    rt_pin_write(ZJ_LED2,PIN_HIGH);
    rt_pin_write(ZJ_LED3,PIN_HIGH);
    rt_pin_write(ZJ_LED4,PIN_HIGH);

    rt_pin_attach_irq(ZJ_KEY1, RT_GPIO_MODE_SET(NRF_GPIOTE_POLARITY_HITOLO,NRF_GPIO_PIN_SENSE_LOW),key_handle, RT_NULL);
    rt_pin_irq_enable(ZJ_KEY1, PIN_IRQ_ENABLE);
    return 0;
}
INIT_DEVICE_EXPORT(led_key_init);
/////////////////////////////////////
static uint32_t last_ticks1 = 0;
static uint32_t last_ticks2 = 0;
APP_TIMER_DEF(led_timer_id1);
APP_TIMER_DEF(led_timer_id2);
static void led_timeout1_handler(void * p_context)
{
    uint32_t tick_cnt1;
    tick_cnt1 = (uint32_t)rt_tick_get();   
    gpio_printf("Timer1 over %dms\n", TICK_TO_OS_MSEC(ABS(tick_cnt1 - last_ticks1)));
    last_ticks1 = tick_cnt1;

    static bool pin_toggle=false;
    rt_pin_write(ZJ_LED3, pin_toggle); 
    pin_toggle = !pin_toggle;
}

static void led_timeout2_handler(void * p_context)
{
    uint32_t tick_cnt2;
    tick_cnt2 = (uint32_t)rt_tick_get();   
    gpio_printf("Timer2 over %dms\n", TICK_TO_OS_MSEC(ABS(tick_cnt2 - last_ticks2)));
    last_ticks2 = tick_cnt2;

    static bool pin_toggle=false;
    rt_pin_write(ZJ_LED4, pin_toggle); 
    pin_toggle = !pin_toggle;
}

void led_timer_init(void)
{
    ret_code_t err_code;
    static bool is_timer_init = false;
    rt_kprintf("ZJ-SDK:RT-Thread定时器在ZJ曹孟德开发板上的应用\n");   
    rt_kprintf("led_timeout1_handler 4s后只会执行一次就注销\n");
    rt_kprintf("led_timeout2_handler 会每隔2s循环执行\n");
   if(is_timer_init==false)
   {
    err_code = app_timer_create(led_timer_id1, APP_TIMER_MODE_SINGLE_SHOT, led_timeout1_handler);   
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(led_timer_id2, APP_TIMER_MODE_REPEATED, led_timeout2_handler);  
    APP_ERROR_CHECK(err_code);
    is_timer_init = true;
   }
}

static void led_timer_start(void)
{
    ret_code_t err_code;
    led_timer_init();
    err_code = app_timer_start(led_timer_id1, 4000, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(led_timer_id2, 2000, NULL);
    APP_ERROR_CHECK(err_code);
    last_ticks1 = (uint32_t)rt_tick_get();
    last_ticks2 = last_ticks1;
}
FINSH_FUNCTION_EXPORT_ALIAS(led_timer_start, led_tstart, led timer start LED3/4 toggle)

static void led_timer_stop(void)
{
    ret_code_t err_code;
    err_code = app_timer_stop(led_timer_id1);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_stop(led_timer_id2);
    APP_ERROR_CHECK(err_code);
    gpio_printf("led_timer_stop\r\n");
}
FINSH_FUNCTION_EXPORT_ALIAS(led_timer_stop, led_tstop, led timer stop LED3/4 toggle)

/**
 * @}
 */
