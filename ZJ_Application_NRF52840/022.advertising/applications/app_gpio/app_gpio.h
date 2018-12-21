#ifndef APP_LED_H__
#define APP_LED_H__
#include <stdint.h>
#include "rt_nrf_hal_gpio.h"
#include "nrfx_gpiote.h"
void app_led_cortrol(uint32_t pin,uint8_t state);

void led_event_start(void * p_context);
void led_event_stop(void * p_context);
#endif










