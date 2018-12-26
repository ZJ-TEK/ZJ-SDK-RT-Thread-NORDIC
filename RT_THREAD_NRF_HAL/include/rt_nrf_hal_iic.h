#ifndef RT_NRF_HAL_IIC__
#define RT_NRF_HAL_IIC__
#include "nrf_gpio.h"


#define BSP_TWI_INSTANCE      0
#define BSP_IIC_SCL_PIN   NRF_GPIO_PIN_MAP(0, 27)
#define BSP_IIC_SDA_PIN   NRF_GPIO_PIN_MAP(0, 26)
#define EEPROM_ADDRESS_LEN_BYTES 1
#define EEPROM_PHY_7BITS_ADDRESS 0x50
#endif










