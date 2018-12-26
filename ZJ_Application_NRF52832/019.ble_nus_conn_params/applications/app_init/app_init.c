#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "nrf_log.h"
#include "board.h"
#include "app_init.h"
#include "app_uart.h"
#include "app_gpio.h"
#include "app_event.h"
#include "app_ble_init.h"

int main(void)
{
    app_ble_init();
    APP_EVENT_PUSH_DELAY(led_event_start,NULL,500);
    NRF_LOG_INFO("RT-Thread for nrf52840 started");
    return RT_TRUE;
}


/**
 * @}
 */
