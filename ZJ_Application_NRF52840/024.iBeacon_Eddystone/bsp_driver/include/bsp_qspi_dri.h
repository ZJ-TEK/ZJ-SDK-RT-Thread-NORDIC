#ifndef BSP_QSPI_DRI_H__
#define BSP_QSPI_DRI_H__
#include "nrf_drv_qspi.h"
#include "nrf_gpio.h"

#define BSP_QSPI_SCK_PIN   NRF_GPIO_PIN_MAP(0, 19)
#define BSP_QSPI_CSN_PIN   NRF_GPIO_PIN_MAP(0, 17)
#define BSP_QSPI_IO0_PIN   NRF_GPIO_PIN_MAP(0, 20)
#define BSP_QSPI_IO1_PIN   NRF_GPIO_PIN_MAP(0, 21)
#define BSP_QSPI_IO2_PIN   NRF_GPIO_PIN_MAP(0, 22)
#define BSP_QSPI_IO3_PIN   NRF_GPIO_PIN_MAP(0, 23)

/* ��λ���� */
#define RESET_ENABLE_CMD                     0x66
#define RESET_MEMORY_CMD                     0x99
/* �Ĵ������� */
#define WRITE_STATUS_REG_CMD                  0x01
#define READ_STATUS_REG1_CMD                  0x05
#define READ_STATUS_REG2_CMD                  0x35
/* д���� */
#define WRITE_ENABLE_CMD                     0x06
#define WRITE_STATUS_ENABLE_CMD              0x50
#define WRITE_DISABLE_CMD                    0x04
////////////////////////////////////////////////////
//#define ENTER_QPI_MODE_CMD                   0x38
//#define EXIT_QPI_MODE_CMD                    0xFF
//
///* ʶ����� */
//#define READ_ID_CMD                          0x90
//#define DUAL_READ_ID_CMD                     0x92
//#define QUAD_READ_ID_CMD                     0x94
//#define READ_JEDEC_ID_CMD                    0x9F
//
///* ������ */
//#define READ_CMD                             0x03
//#define FAST_READ_CMD                        0x0B
//#define DUAL_OUT_FAST_READ_CMD               0x3B
//#define DUAL_INOUT_FAST_READ_CMD             0xBB
//#define QUAD_OUT_FAST_READ_CMD               0x6B
//#define QUAD_INOUT_FAST_READ_CMD             0xEB
//
///* ��̲��� */
//#define PAGE_PROG_CMD                        0x02
//#define QUAD_INPUT_PAGE_PROG_CMD             0x32
//#define EXT_QUAD_IN_FAST_PROG_CMD            0x12
//
///* �������� */
//#define SECTOR_ERASE_CMD                     0x20
//#define CHIP_ERASE_CMD                       0xC7
//#define PROG_ERASE_RESUME_CMD                0x7A
//#define PROG_ERASE_SUSPEND_CMD               0x75

uint32_t bsp_qspi_flash_write(void const * p_tx_buffer,
                          uint32_t     tx_buffer_length,
                          uint32_t     dst_address);

uint32_t bsp_qspi_flash_read(void  * p_rx_buffer,
                         uint32_t     rx_buffer_length,
                         uint32_t     dst_address);

void bsp_qspi_flash_program(uint32_t WriteAddr,uint8_t* pBuffer,int16_t NumByteToWrite);

#endif










