#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf_gpio.h"
#include "nrfx_spim.h"
#include "app_error.h"
#include "bsp_lcd_ie3130b_dri.h"
#include <rtthread.h>

#define SPI_INSTANCE  3                                           /**< SPI instance index. */
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */


#define SPI_BUFF_SIZE   16

static uint8_t spi_tx_buff[SPI_BUFF_SIZE];


#define OP_NULL         0x00
#define OP_CMD          0x01
#define OP_DELAY        0x02

typedef struct {
    uint8_t             op_type;
    size_t              op_size;
    const uint8_t       *p_data;
} lcd_op_t;

static const uint8_t lcd_init_11[] = {0x11};
static const uint8_t lcd_init_36[] = {0x36, 0x00};
static const uint8_t lcd_init_3A[] = {0x3A, 0x05};
static const uint8_t lcd_init_21[] = {0x21};
static const uint8_t lcd_init_E7[] = {0xE7, 0x00};
static const uint8_t lcd_init_2A[] = {0x2A, 0x00, 0x00, 0x00, 0xEF};
static const uint8_t lcd_init_2B[] = {0x2B, 0x00, 0x00, 0x00, 0xEF};
static const uint8_t lcd_init_B2[] = {0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33};
static const uint8_t lcd_init_B7[] = {0xB7, 0x35};
static const uint8_t lcd_init_BB[] = {0xBB, 0x2A};
static const uint8_t lcd_init_C0[] = {0xC0, 0x2C};
static const uint8_t lcd_init_C2[] = {0xC2, 0x01};
static const uint8_t lcd_init_C3[] = {0xC3, 0x0B};
static const uint8_t lcd_init_C4[] = {0xC4, 0x20};
static const uint8_t lcd_init_C6[] = {0xC6, 0x0F};
static const uint8_t lcd_init_D0[] = {0xD0, 0xA4, 0xA1};
static const uint8_t lcd_init_E9[] = {0xE9, 0x11, 0x11, 0x03};
static const uint8_t lcd_init_E0[] = {0xE0, 0xF0, 0x09, 0x13, 0x0A, 0x0B, 0x06, 0x38, 0x33, 0x4F, 0x04, 0x0D, 0x19, 0x2E, 0x2F};
static const uint8_t lcd_init_E1[] = {0xE1, 0xF0, 0x09, 0x13, 0x0A, 0x0B, 0x06, 0x38, 0x33, 0x4F, 0x04, 0x0D, 0x19, 0x2E, 0x2F};
static const uint8_t lcd_init_29[] = {0x29};

static const lcd_op_t lcd_init_seq[] =
{
    {OP_CMD,   sizeof(lcd_init_11), lcd_init_11},
    {OP_DELAY, 120,                 NULL       },
    {OP_CMD,   sizeof(lcd_init_36), lcd_init_36},
    {OP_CMD,   sizeof(lcd_init_3A), lcd_init_3A},
    {OP_CMD,   sizeof(lcd_init_21), lcd_init_21},
    {OP_CMD,   sizeof(lcd_init_E7), lcd_init_E7},
    {OP_CMD,   sizeof(lcd_init_2A), lcd_init_2A},
    {OP_CMD,   sizeof(lcd_init_2B), lcd_init_2B},
    {OP_CMD,   sizeof(lcd_init_B2), lcd_init_B2},
    {OP_CMD,   sizeof(lcd_init_B7), lcd_init_B7},
    {OP_CMD,   sizeof(lcd_init_BB), lcd_init_BB},
    {OP_CMD,   sizeof(lcd_init_C0), lcd_init_C0},
    {OP_CMD,   sizeof(lcd_init_C2), lcd_init_C2},
    {OP_CMD,   sizeof(lcd_init_C3), lcd_init_C3},
    {OP_CMD,   sizeof(lcd_init_C4), lcd_init_C4},
    {OP_CMD,   sizeof(lcd_init_C6), lcd_init_C6},
    {OP_CMD,   sizeof(lcd_init_D0), lcd_init_D0},
    {OP_CMD,   sizeof(lcd_init_E9), lcd_init_E9},
    {OP_CMD,   sizeof(lcd_init_E0), lcd_init_E0},
    {OP_CMD,   sizeof(lcd_init_E1), lcd_init_E1},
    {OP_DELAY, 120,                 NULL       },
    {OP_CMD,   sizeof(lcd_init_29), lcd_init_29},
    {OP_NULL,  0,                   NULL       }
};

void lcd_ie3130b_proc_op_list(const lcd_op_t *p_op_list)
{
    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(spi_tx_buff, 1);

    while (p_op_list->op_type != OP_NULL)
    {
        if (p_op_list->op_type == OP_CMD)
        {
            // send LCD command
            memcpy(spi_tx_buff, p_op_list->p_data, p_op_list->op_size);
            xfer_desc.tx_length = p_op_list->op_size;
            nrfx_err_t err_code = nrfx_spim_xfer_dcx(&spi, &xfer_desc, 0, 1);
            APP_ERROR_CHECK(err_code);
        }
        else if (p_op_list->op_type == OP_DELAY)
        {
            // wait awhile
            rt_thread_mdelay(p_op_list->op_size);
        }
        else
        {
            // something goes wrong!
            break;
        }

        p_op_list++;
    }
}

/**@brief Function to initiate LCD.
 */
int lcd_ie3130b_init(void)
{
    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
    spi_config.frequency      = NRF_SPIM_FREQ_32M;
    spi_config.mode           = NRF_SPIM_MODE_3;
    spi_config.ss_pin         = LCD_CS;
    spi_config.miso_pin       = LCD_MISO;
    spi_config.mosi_pin       = LCD_MOSI;
    spi_config.sck_pin        = LCD_SCK;
    spi_config.dcx_pin        = LCD_D_C;
    spi_config.use_hw_ss      = true;
    spi_config.ss_active_high = false;
    spi_config.ss_duration    = 8;
    APP_ERROR_CHECK(nrfx_spim_init(&spi, &spi_config, NULL, NULL));

    // set SCK to high drive
    nrf_gpio_cfg(
        LCD_SCK,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_H0H1,
        NRF_GPIO_PIN_NOSENSE);

    // set MOSI to high drive
    nrf_gpio_cfg(
        LCD_MOSI,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_H0H1,
        NRF_GPIO_PIN_NOSENSE);

    // reset ST7789H2
    nrf_gpio_pin_set(LCD_RESET);
    nrf_gpio_cfg_output(LCD_RESET);
    rt_thread_mdelay(15);
    nrf_gpio_pin_clear(LCD_RESET);
    rt_thread_mdelay(5);
    nrf_gpio_pin_set(LCD_RESET);
    rt_thread_mdelay(120);

    // send LCD init sequence
    lcd_ie3130b_proc_op_list(lcd_init_seq);
    return 0;
}
INIT_DEVICE_EXPORT(lcd_ie3130b_init);

static void lcd_ie3130b_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint8_t tx_buff[5];
    nrfx_err_t err_code;
    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(tx_buff, 5);

    tx_buff[0] = 0x2A;
    tx_buff[1] = ( x          >> 8) & 0xFF;
    tx_buff[2] = ( x              ) & 0xFF;
    tx_buff[3] = ((x + w - 1) >> 8) & 0xFF;
    tx_buff[4] = ((x + w - 1)     ) & 0xFF;
    err_code = nrfx_spim_xfer_dcx(&spi, &xfer_desc, 0, 1);
    APP_ERROR_CHECK(err_code);

    tx_buff[0] = 0x2B;
    tx_buff[1] = ( y          >> 8) & 0xFF;
    tx_buff[2] = ( y              ) & 0xFF;
    tx_buff[3] = ((y + h - 1) >> 8) & 0xFF;
    tx_buff[4] = ((y + h - 1)     ) & 0xFF;
    err_code = nrfx_spim_xfer_dcx(&spi, &xfer_desc, 0, 1);
    APP_ERROR_CHECK(err_code);
}

void lcd_ie3130b_put_gfx(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *p_lcd_data)
{
    lcd_ie3130b_set_window(x, y, w, h);

    uint8_t tx_buff[1];
    nrfx_err_t err_code;
    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(tx_buff, 1);

    // send LCD command
    tx_buff[0] = 0x2C;
    err_code = nrfx_spim_xfer_dcx(&spi, &xfer_desc, 0, 1);
    APP_ERROR_CHECK(err_code);

    uint32_t len = (uint32_t)w * (uint32_t)h * 2;
    uint32_t pos, stp;

    // send LCD data
    for (pos = 0; pos < len; pos += stp)
    {
        stp = MIN((len - pos), 65534);
        xfer_desc.tx_length = stp;
        xfer_desc.p_tx_buffer = p_lcd_data + pos;
        err_code = nrfx_spim_xfer_dcx(&spi, &xfer_desc, 0, 0);
        APP_ERROR_CHECK(err_code);
    }
}
