#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "board.h"
#include "app_init.h"
#include "nrf_gpio.h"

int main(void)
{
    uint32_t cnt=0;
    nrf_gpio_cfg_output(ZJ_LED1);
    rt_kprintf("RT-Thread for nrf52840 started.\r\n");
    while(1)
    {
       nrf_gpio_pin_toggle(ZJ_LED1);
       rt_thread_mdelay(500);
       rt_kprintf("This is RTT test %d.\r\n",cnt++);
    }
    return RT_TRUE;
}


/**
 * @}
 */
