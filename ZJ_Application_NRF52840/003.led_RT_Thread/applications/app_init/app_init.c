#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "board.h"
#include "app_init.h"
#include "nrf_gpio.h"

rt_err_t sys_led_init(void)
{
    // Initialize.
    nrf_gpio_cfg_output(ZJ_LED1);
    rt_kprintf("RT-Thread for nrf52840 started.\r\n");
    while(1)
    {
       nrf_gpio_pin_toggle(ZJ_LED1);
       rt_thread_mdelay(500);
    }
    return RT_TRUE;
}


/**
 * @}
 */
