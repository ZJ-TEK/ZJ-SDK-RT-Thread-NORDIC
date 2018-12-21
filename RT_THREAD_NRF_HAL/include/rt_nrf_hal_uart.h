#ifndef _RT_nRF_HAL_UART_H_
#define _RT_nRF_HAL_UART_H_

#define RX_PIN_NUMBER  8
#define TX_PIN_NUMBER  6
#define CTS_PIN_NUMBER 7
#define RTS_PIN_NUMBER 5

#define RT_DEVICE_CTRL_CUSTOM   0x20
#define RT_DEVICE_CTRL_PIN      0x21
#define RT_DEVICE_POWERSAVE     0x22
#define RT_DEVICE_WAKEUP        0x23

#define UART_CONFIG_BAUD_RATE_9600      1
#define UART_CONFIG_BAUD_RATE_115200    2

// #define USE_UART0_1         0

int rt_hw_uart_init(void);

#endif

