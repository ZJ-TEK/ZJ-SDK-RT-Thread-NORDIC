#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "nrf_log.h"
#include "board.h"
#include "app_init.h"
#include "app_uart.h"
#include "app_gpio.h"

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
