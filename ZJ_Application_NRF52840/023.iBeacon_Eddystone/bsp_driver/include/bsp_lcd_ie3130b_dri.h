#ifndef BSP_LCD_IE3130B_DRI_H__
#define BSP_LCD_IE3130B_DRI_H__

#include <stdint.h>

// IE3130B LCD pin assignments
#define LCD_RESET   NRF_GPIO_PIN_MAP(1,  9)
#define LCD_D_C     NRF_GPIO_PIN_MAP(1,  5)
#define LCD_CS      NRF_GPIO_PIN_MAP(1,  6)
#define LCD_SCK     NRF_GPIO_PIN_MAP(1,  7)
#define LCD_MOSI    NRF_GPIO_PIN_MAP(1,  8)
#define LCD_MISO    NRFX_SPIM_PIN_NOT_USED

// IE3130B LCD size is 240 x 240 dots.
#define LCD_IE3130B_WIDTH       240
#define LCD_IE3130B_HEIGHT      240

/**@brief Function to initiate LCD.
 */
int lcd_ie3130b_init(void);

/**@brief Function to put a graphics on LCD.
 */
void lcd_ie3130b_put_gfx(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *p_lcd_data);

#endif // LCD_IE3130B_H__
