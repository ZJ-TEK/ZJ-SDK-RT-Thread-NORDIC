#include <rtthread.h>
#include <rthw.h>
#include <drivers/pin.h>
#include "app_gpio.h"
#include "rt_nrf_hal_gpio.h"

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


/**
 * @}
 */
