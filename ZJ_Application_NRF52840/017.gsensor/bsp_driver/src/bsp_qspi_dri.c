#include <rtthread.h>
#include "bsp_qspi_dri.h"
#include "nrf_log.h"
#include "board.h"
#include "app_timer.h"

#define qspi_printf(...)  NRF_LOG_INFO(__VA_ARGS__)
#define FLASH_PAGE_SIZE	        256

#define WAIT_FOR_PERIPH() do { \
        uint16_t wait_cnt = 500; \
        while (!m_finished && wait_cnt--) {/*qspi_printf("WAIT_FOR_PERIPH = %d\r\n",wait_cnt);*/rt_thread_delay(5);} \
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
    uint8_t status1 =0,status2 =0;
    uint16_t temporary = 0x202;  //switch qspi
    uint32_t err_code;
    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode    = RESET_ENABLE_CMD,
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
    cinstr_cfg.opcode = RESET_MEMORY_CMD;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);
#if 0
    // write enable
    cinstr_cfg.opcode = WRITE_ENABLE_CMD;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // write status enable
    cinstr_cfg.opcode = WRITE_STATUS_ENABLE_CMD;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // read status1
    cinstr_cfg.opcode = READ_STATUS_REG1_CMD;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, &status1, &status1);
    APP_ERROR_CHECK(err_code);

    // read status2
    cinstr_cfg.opcode = READ_STATUS_REG2_CMD;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, &status2, &status2);
    APP_ERROR_CHECK(err_code);

    temporary |= ((status2<<8) |status1);
#endif
    // Switch to qspi mode
    cinstr_cfg.opcode = WRITE_STATUS_REG_CMD;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_3B;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, (uint8_t*)&temporary, NULL);
    APP_ERROR_CHECK(err_code);
}


int bsp_qspi_flash_init(void)
{
    uint32_t err_code;
    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;

    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    qspi_printf("QSPI example started.");

    configure_memory();
    m_finished = false;
    return 0;
}
INIT_DEVICE_EXPORT(bsp_qspi_flash_init);

void bsp_qspi_flash_uninit(void)
{
    nrf_drv_qspi_uninit();
}

void bsp_qspi_flash_erase(nrf_qspi_erase_len_t length, uint32_t start_address)
{
    uint32_t err_code;

    err_code = nrf_drv_qspi_erase(length, start_address);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    qspi_printf("erasing");
}

uint32_t bsp_qspi_flash_write(void const * p_tx_buffer,
                          uint32_t     tx_buffer_length,
                          uint32_t     dst_address)
{
    uint16_t pageremain;
    uint32_t err_code;
    uint8_t *p_buffer = (uint8_t*)p_tx_buffer;
    while(tx_buffer_length)
    {
       pageremain = FLASH_PAGE_SIZE - dst_address % FLASH_PAGE_SIZE;
       pageremain = MIN(tx_buffer_length, pageremain);
            
       err_code = nrf_drv_qspi_write(p_buffer, tx_buffer_length, dst_address);
       APP_ERROR_CHECK(err_code);
       WAIT_FOR_PERIPH();
       dst_address += pageremain;
       p_buffer += pageremain;
       tx_buffer_length -= pageremain;
       qspi_printf("data writing ...\n");
    }
    return err_code;
}

uint32_t bsp_qspi_flash_read(void  * p_rx_buffer,
                         uint32_t     rx_buffer_length,
                         uint32_t     dst_address)
{
    uint32_t err_code;
    uint16_t num_read;
    uint8_t *p_buffer = (uint8_t*)p_rx_buffer;
    while(rx_buffer_length)
    {
      num_read = MIN(rx_buffer_length, FLASH_PAGE_SIZE);
      err_code = nrf_drv_qspi_read(p_buffer, num_read, dst_address);
      APP_ERROR_CHECK(err_code);
      WAIT_FOR_PERIPH();

      dst_address += num_read;
      p_buffer += num_read;
      rx_buffer_length -= num_read;
      qspi_printf("Data read ...\n");
    }
    return err_code;
}

void bsp_qspi_flash_program(uint32_t WriteAddr,uint8_t* pBuffer,int16_t NumByteToWrite)
{
    #define AT25X_SECTOR_SIZE 4096
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;

    secpos = WriteAddr / AT25X_SECTOR_SIZE;//扇区地址 0~511 for at25x16
    secoff = WriteAddr % AT25X_SECTOR_SIZE;//在扇区内的偏移
    secremain = AT25X_SECTOR_SIZE - secoff;//扇区剩余空间大小
    static uint8_t SPI_FLASH_BUF[AT25X_SECTOR_SIZE];

    if(NumByteToWrite <= secremain)secremain = NumByteToWrite;//不大于4096个字节
    while(1)
    {
        bsp_qspi_flash_read(SPI_FLASH_BUF, AT25X_SECTOR_SIZE,secpos * AT25X_SECTOR_SIZE); //读出整个扇区的内容
        for(i = 0; i < secremain; i++) //校验数据
        {
            if(SPI_FLASH_BUF[secoff + i] != 0XFF)break; //需要擦除
        }
        if(i < secremain) //需要擦除
        {
            bsp_qspi_flash_erase(NRF_QSPI_ERASE_LEN_4KB,secpos * AT25X_SECTOR_SIZE);//擦除这个扇区
            memcpy(&SPI_FLASH_BUF[secoff], pBuffer, secremain);
            bsp_qspi_flash_write(SPI_FLASH_BUF, AT25X_SECTOR_SIZE, secpos * AT25X_SECTOR_SIZE);//写入整个扇区
        }
        else bsp_qspi_flash_write(pBuffer, secremain,WriteAddr); //写已经擦除了的,直接写入扇区剩余区间.
        if(NumByteToWrite == secremain)break;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff = 0; //偏移位置为0

            pBuffer += secremain;  //指针偏移
            WriteAddr += secremain;//写地址偏移
            NumByteToWrite -= secremain;				//字节数递减
            if(NumByteToWrite > AT25X_SECTOR_SIZE)secremain = AT25X_SECTOR_SIZE;	//下一个扇区还是写不完
            else secremain = NumByteToWrite;			//下一个扇区可以写完了
        }
    }
}

#if 1
void qspi_flash_test(void)
{
    uint32_t i;
    static uint32_t start_tick = 0;
    static uint32_t finish_tick = 0;
    static uint32_t use_time[3] ;

#define QSPI_TEST_DATA_SIZE 512
    static uint8_t test_buffer_tx[QSPI_TEST_DATA_SIZE];
    static uint8_t test_buffer_rx[QSPI_TEST_DATA_SIZE];
    for (i = 0; i < QSPI_TEST_DATA_SIZE; ++i)
    {
        test_buffer_tx[i] = (uint8_t)i;
    }
    finish_tick =0;
    start_tick =rt_tick_get();
    bsp_qspi_flash_erase(NRF_QSPI_ERASE_LEN_4KB, 0);
    use_time[0] = app_timer_cnt_diff_compute(rt_tick_get(), start_tick);
    finish_tick = use_time[0];

    start_tick =rt_tick_get();
    bsp_qspi_flash_write(test_buffer_tx, QSPI_TEST_DATA_SIZE, 0);
    use_time[1] = app_timer_cnt_diff_compute(rt_tick_get(), start_tick);\
    finish_tick += use_time[1];

    start_tick =rt_tick_get();
    bsp_qspi_flash_read(test_buffer_rx, QSPI_TEST_DATA_SIZE, 0);
    use_time[2] = app_timer_cnt_diff_compute(rt_tick_get(), start_tick);\
    finish_tick += use_time[2];

    qspi_printf("erase... = %dms\r\n",TICK_TO_OS_MSEC(use_time[0]));
    qspi_printf("write... = %dms\r\n",TICK_TO_OS_MSEC(use_time[1]));
    qspi_printf("read ... = %dms\r\n",TICK_TO_OS_MSEC(use_time[2]));
    qspi_printf("total... = %dms %dms\r\n",TICK_TO_OS_MSEC(use_time[0]+use_time[1]+use_time[2]),TICK_TO_OS_MSEC(finish_tick));

    if (memcmp(test_buffer_tx, test_buffer_rx, QSPI_TEST_DATA_SIZE) == 0)
    {
        qspi_printf("Data consistent");
    }
    else
    {
        qspi_printf("Data inconsistent");
    }
//    bsp_qspi_flash_uninit();
}
FINSH_FUNCTION_EXPORT_ALIAS(qspi_flash_test, qspi, this is qspi flash test)
#endif



