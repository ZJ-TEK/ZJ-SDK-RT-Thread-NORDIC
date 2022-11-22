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

#define font_printf(...) 	do{NRF_LOG_INFO(__VA_ARGS__);/*rt_kprintf(__VA_ARGS__);*/}while(0)

                    
                    
                    
#define LCD_BL_EN   NRF_GPIO_PIN_MAP(0,  4)
static uint16_t lcd_buffer[LCD_IE3130B_WIDTH][ LCD_IE3130B_HEIGHT ];
//static uint8_t move_lcd_buffer[LCD_IE3130B_WIDTH * 2];
typedef struct
{
    char  m_pu8Name[4]; 
    uint8_t m_bpp; 
    uint8_t m_u8px; 
    uint16_t m_u16YSize;
    uint16_t m_u16YDist;  
    uint16_t m_u16BaseLine;  
    uint16_t m_u16FirstCode; 
    uint16_t m_u16LastCode; 
}xbf_font_head_t;


xbf_font_head_t xbf_font_head ={
    .m_bpp     = 0x02,
    .m_u8px     = 0x18,
    .m_u16YSize  = 15,
    .m_u16YDist  = 15,
    .m_u16BaseLine    = 4,
    .m_u16FirstCode = 0x20,
    .m_u16LastCode  = 0x22,
};

typedef struct
{
    uint8_t adv_w; 
    uint8_t box_w;
    uint8_t box_h; 
    int8_t ofs_x;
    int8_t ofs_y;
    uint8_t BytesPerLine;
} xbf_bitmap_attribute_t;

unsigned char data[48] = {
	0x0D, 0x0B, 0x0E, 0x01, 0x05, 0x03, 0x00, 0xE0, 0x70, 0x00, 0xD0, 0xB0, 0x01, 0xC0, 0xE0, 0x02, 
	0xC0, 0xD0, 0x3F, 0xFF, 0xFC, 0x17, 0x96, 0xD4, 0x07, 0x03, 0x80, 0x0B, 0x03, 0x40, 0x0E, 0x07, 
	0x00, 0xFF, 0xFF, 0xF0, 0xAE, 0xAF, 0xA0, 0x2C, 0x0D, 0x00, 0x28, 0x1C, 0x00, 0x38, 0x1C, 0x00
};


xbf_bitmap_attribute_t  bitmap_attribute ;



uint8_t gui_get_font_xsize(void)
{
    return bitmap_attribute.box_w;
}

uint8_t gui_get_font_ysize(void)
{
    return bitmap_attribute.box_h;
}

uint8_t gui_get_font_lfheight(void)
{
    return xbf_font_head.m_u16YSize;
}

uint8_t gui_get_font_tmascent(void)
{
    return xbf_font_head.m_u16YSize - xbf_font_head.m_u16BaseLine +1;
}

void oled_go(int x, int y)
{
    uint32_t err_code;
    nrf_gpio_cfg_output(LCD_BL_EN);
    nrf_gpio_pin_set(LCD_BL_EN);

    memset(lcd_buffer, 0xff , sizeof(lcd_buffer));

    const uint8_t offsetmask[] = {6, 4, 2, 0};
    
//    bitmap_attribute.adv_w = (data[1]<< 8) | data[0];
//    bitmap_attribute.box_w = (data[3]<< 8) | data[2];
//    bitmap_attribute.box_h = (data[5]<< 8) | data[4];
//    bitmap_attribute.ofs_x = (data[7]<< 8) | data[6];
//    bitmap_attribute.ofs_y = (data[9]<< 8) | data[8];
//    bitmap_attribute.BytesPerLine = (data[11]<< 8) | data[10];
//    
//    uint8_t * font = &data[12]; 
    
    bitmap_attribute.adv_w = data[0];
    bitmap_attribute.box_w = data[1];
    bitmap_attribute.box_h = data[2];
    bitmap_attribute.ofs_x = data[3];
    bitmap_attribute.ofs_y = data[4];
    bitmap_attribute.BytesPerLine = data[5];
    
    uint8_t * font = &data[6]; 

    
    uint8_t byte, gray_color;
    
    uint16_t bgcolor = 0, fgcolor = 0xffff;
    
    uint8_t bgr = (bgcolor >> 11) & 0x1F;             
    uint8_t bgg = (bgcolor >> 5) & 0x3F;             
    uint8_t bgb = bgcolor & 0x1F;
    uint8_t fgr = (fgcolor >> 11)& 0x1F;             
    uint8_t fgg = (fgcolor >> 5)& 0x3F;             
    uint8_t fgb = fgcolor & 0x1F;  
    
 
    uint8_t bppEvtBytes = 8/xbf_font_head.m_bpp;
    

    uint16_t ypos = 0, xpos = 0;
    uint8_t bits = 0;
    bool loop = true;
    while(loop)
    {
        byte = *font++;
        for(bits = 0; bits < bppEvtBytes; bits++) //bit2
        {
            if (xpos < bitmap_attribute.box_w)
            {
                gray_color = (byte >> offsetmask[bits]) & 0x3;
                uint8_t alpha = 0;
                
                if(gray_color == 0)
                {
                    alpha = 0;
                }
                else if(gray_color ==1)
                {
                    alpha = 33;
                }
                else if(gray_color ==2)
                {
                    alpha = 33;
                }
                else if(gray_color ==3)
                {
                    alpha = 100;
                }
                
                uint8_t newfgr = ((bgr * alpha +  (fgr * (100 - alpha))) / 100) & 0x1F;           
                uint8_t newfgg = ((bgg * alpha +  (fgg * (100 - alpha))) / 100) & 0x3F;            
                uint8_t newfgb = ((bgb * alpha +  (fgb * (100 - alpha))) / 100) & 0x1F; 

                lcd_buffer[x + xpos][y + ypos] = newfgr << 11 | newfgg << 5 | newfgb;
            }
            xpos++;
            if(xpos == bitmap_attribute.BytesPerLine * 8 / 2)
            {
                xpos = 0;
                ypos++;
                if(ypos == bitmap_attribute.box_h )
                {
                    loop = false;
                }
            }
        }     
    }//while(loop)
     
    lcd_ie3130b_put_gfx(0, 0, LCD_IE3130B_WIDTH, LCD_IE3130B_HEIGHT, (uint8_t *)lcd_buffer);
}



int main(void)
{
    APP_EVENT_PUSH_DELAY(led_event_start,NULL,500);
    NRF_LOG_INFO("RT-Thread for nrf52840 started");
    oled_go(50,50);
    return RT_TRUE;
}


/**
 * @}
 */
