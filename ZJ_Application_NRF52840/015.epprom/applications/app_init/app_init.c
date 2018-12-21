#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include "nrf_log.h"
#include "board.h"
#include "app_init.h"
#include "app_uart.h"
#include "app_gpio.h"
#include "app_event.h"

#include "bsp_lcd_ie3130b_dri.h"
#include "bsp_qspi_dri.h"
#define LCD_BL_EN   NRF_GPIO_PIN_MAP(0,  4)
static uint8_t lcd_buffer[LCD_IE3130B_WIDTH * LCD_IE3130B_HEIGHT * 2];
int oled_go(void)
{
    uint32_t err_code;
    nrf_gpio_cfg_output(LCD_BL_EN);
    nrf_gpio_pin_set(LCD_BL_EN);

    uint32_t src_addr = 0x0;       // QSPI graphics start address
    uint32_t gfx_next = 0x20000;   // next graphics offset
    uint32_t gfx_last = 0xB0000;  // last graphics address

    while (1)
    {
        err_code = bsp_qspi_flash_read(lcd_buffer, sizeof(lcd_buffer), src_addr);
        APP_ERROR_CHECK(err_code);
        lcd_ie3130b_put_gfx(0, 0, LCD_IE3130B_WIDTH, LCD_IE3130B_HEIGHT, lcd_buffer);
        rt_thread_mdelay(500);

        uint32_t x;
        uint32_t y;
        uint32_t n;
        uint32_t stp;
        uint32_t x_pos;

        stp = LCD_IE3130B_WIDTH / 20;  // shift out the picture in 20 steps
        if (!stp)
            stp = 1;

        for (x_pos = 0; x_pos < LCD_IE3130B_WIDTH; x_pos += stp)
        {
            // move the picture to the left
            n = 0;
            for (y = 0; y < LCD_IE3130B_HEIGHT; y++)
            {
                for (x = 0; x + stp < LCD_IE3130B_WIDTH; x++)
                {
                    lcd_buffer[n    ] = lcd_buffer[n + stp * 2    ];
                    lcd_buffer[n + 1] = lcd_buffer[n + stp * 2 + 1];
                    n += 2;
                }

                for ( ; x < LCD_IE3130B_WIDTH; x++)
                {
                    // black color
                    lcd_buffer[n++] = 0x00;
                    lcd_buffer[n++] = 0x00;
                }
           }
            lcd_ie3130b_put_gfx(0, 0, LCD_IE3130B_WIDTH, LCD_IE3130B_HEIGHT, lcd_buffer);
        }
        // point to next graphics address
        src_addr += gfx_next;
        if (src_addr > gfx_last)
            src_addr = 0;
    }
}



int main(void)
{
    APP_EVENT_PUSH_DELAY(led_event_start,NULL,500);
    NRF_LOG_INFO("RT-Thread for nrf52840 started");
    oled_go();
    return RT_TRUE;
}


/**
 * @}
 */
