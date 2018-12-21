#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "nrf_log.h"
#include "board.h"
#include "app_init.h"
#include "nrf_gpio.h"

int main(void)
{
    uint32_t cnt=0;
    nrf_gpio_cfg_output(ZJ_LED1);
    NRF_LOG_INFO("RT-Thread for nrf52840 started.");
    while(1)
    {
       nrf_gpio_pin_toggle(ZJ_LED1);
       rt_thread_mdelay(500);
       NRF_LOG_INFO("This is nrf_log test %d.",cnt);
       rt_kprintf("This is uart test %d.\r\n",cnt);
       cnt++;
    }
    return RT_TRUE;
}


/**
 * @}
 */
