#ifndef _RT_nRF_HAL_GPIO_H_
#define _RT_nRF_HAL_GPIO_H_

#define RT_GPIO_SENSE_Pos (0UL) /*!< Position of SENSE field. */
#define RT_GPIO_SENSE_Msk (0x3UL << RT_GPIO_SENSE_Pos) /*!< Bit mask of SENSE field. */

#define RT_GPIO_POLARITY_Pos (2UL) /*!< Position of SENSE field. */
#define RT_GPIO_POLARITY_Msk (0x3UL << RT_GPIO_POLARITY_Pos) /*!< Bit mask of SENSE field. */

#define RT_GPIO_MODE_SET(polarity,sense)   (((polarity << RT_GPIO_POLARITY_Pos) & RT_GPIO_POLARITY_Msk) | ((sense << RT_GPIO_SENSE_Pos) & RT_GPIO_SENSE_Msk)) /*!< Bit mask of SENSE field. */

#define ZJ_LED1    NRF_GPIO_PIN_MAP(0,13)
#define ZJ_LED2    NRF_GPIO_PIN_MAP(0,14)
#define ZJ_LED3    NRF_GPIO_PIN_MAP(0,15)
#define ZJ_LED4    NRF_GPIO_PIN_MAP(0,16)

#define ZJ_KEY1    NRF_GPIO_PIN_MAP(0,11)
#define ZJ_KEY2    NRF_GPIO_PIN_MAP(0,12)
#define ZJ_KEY3    NRF_GPIO_PIN_MAP(0,13)
#define ZJ_KEY4    NRF_GPIO_PIN_MAP(0,14)

#endif

