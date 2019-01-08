#include <rtthread.h>
#include "app_qspi.h"
#include "nrf_log.h"
#include "board.h"
#include "app_timer.h"

#define qspi_printf(...)  //NRF_LOG_INFO(__VA_ARGS__)

#define WAIT_FOR_PERIPH() do { \
        while (!m_finished) {rt_thread_delay(RT_TICK_PER_SECOND>>4);} \
        m_finished = false;    \
    } while (0)

static volatile bool m_finished = false;

static void qspi_handler(nrf_drv_qspi_evt_t event, void * p_context)
{
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(p_context);
    m_finished = true;
}

static void configure_memory()
{
    uint8_t temporary = 0x40;
    uint32_t err_code;
    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode    = QSPI_STD_CMD_RSTEN,
        .length    = NRF_QSPI_CINSTR_LEN_1B,
        .io2_level = true,
        .io3_level = true,
        .wipwait   = true,
        .wren      = true
    };

    // Send reset enable
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Send reset command
    cinstr_cfg.opcode = QSPI_STD_CMD_RST;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Switch to qspi mode
    cinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, &temporary, NULL);
    APP_ERROR_CHECK(err_code);
}


int app_qspi_flash_init(void)
{
    uint32_t err_code;
    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;

    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    qspi_printf("QSPI example started.");

    configure_memory();
    m_finished = false;
    return RT_TRUE;
}
INIT_DEVICE_EXPORT(app_qspi_flash_init);

void app_qspi_flash_uninit(void)
{
    nrf_drv_qspi_uninit();
}

void app_qspi_flash_erase(nrf_qspi_erase_len_t length, uint32_t start_address)
{
    uint32_t err_code;

    err_code = nrf_drv_qspi_erase(length, start_address);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    qspi_printf("erasing");
}

void app_qspi_flash_write(void const * p_tx_buffer,
                          uint32_t     tx_buffer_length,
                          uint32_t     dst_address)
{
    uint32_t err_code;
    err_code = nrf_drv_qspi_write(p_tx_buffer, tx_buffer_length, dst_address);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    qspi_printf("data writing");
}

void app_qspi_flash_read(void  * p_rx_buffer,
                         uint32_t     rx_buffer_length,
                         uint32_t     dst_address)
{
    uint32_t err_code;
    err_code = nrf_drv_qspi_read(p_rx_buffer, rx_buffer_length, dst_address);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    qspi_printf("Data read");
}

#if 1
void qspi_flash_test(void)
{
    uint32_t i;
    static uint32_t start_tick = 0;
    static uint32_t finish_tick = 0;
    static uint32_t use_time[3] ;

#define QSPI_TEST_DATA_SIZE 512
    uint8_t test_buffer_tx[QSPI_TEST_DATA_SIZE];
    uint8_t test_buffer_rx[QSPI_TEST_DATA_SIZE];
    for (i = 0; i < QSPI_TEST_DATA_SIZE; ++i)
    {
        test_buffer_tx[i] = (uint8_t)i;
    }
    finish_tick =0;
    start_tick =rt_tick_get();
    app_qspi_flash_erase(NRF_QSPI_ERASE_LEN_64KB, 0);
    use_time[0] = app_timer_cnt_diff_compute(rt_tick_get(), start_tick);
    finish_tick = use_time[0];

    start_tick =rt_tick_get();
    app_qspi_flash_write(test_buffer_tx, QSPI_TEST_DATA_SIZE, 0);
    use_time[1] = app_timer_cnt_diff_compute(rt_tick_get(), start_tick);\
    finish_tick += use_time[1];

    start_tick =rt_tick_get();
    app_qspi_flash_read(test_buffer_rx, QSPI_TEST_DATA_SIZE, 0);
    use_time[2] = app_timer_cnt_diff_compute(rt_tick_get(), start_tick);\
    finish_tick += use_time[2];

    rt_kprintf("erase... = %dms\r\n",TICK_TO_OS_MSEC(use_time[0]));
    rt_kprintf("write... = %dms\r\n",TICK_TO_OS_MSEC(use_time[1]));
    rt_kprintf("read ... = %dms\r\n",TICK_TO_OS_MSEC(use_time[2]));
    rt_kprintf("total... = %dms %dms\r\n",TICK_TO_OS_MSEC(use_time[0]+use_time[1]+use_time[2]),TICK_TO_OS_MSEC(finish_tick));

    if (memcmp(test_buffer_tx, test_buffer_rx, QSPI_TEST_DATA_SIZE) == 0)
    {
        rt_kprintf("Data consistent");
    }
    else
    {
        rt_kprintf("Data inconsistent");
    }
//    app_qspi_flash_uninit();
}
FINSH_FUNCTION_EXPORT_ALIAS(qspi_flash_test, qspi, this is qspi flash test)
#endif