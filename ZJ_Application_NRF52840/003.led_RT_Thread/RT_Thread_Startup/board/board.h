#ifndef _BOARD_H_
#define _BOARD_H_

#include <rtthread.h>

#include "nrf.h"

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define NRF_HEAP_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define NRF_HEAP_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end__;
#define NRF_HEAP_BEGIN   (&__bss_end__)
#endif

#define CHIP_HEAP_END       (NRF_HEAP_BEGIN + 10*1024)

/**@brief Macro for converting milliseconds to ticks.
 *
 * @param[in] TIME          Number of milliseconds to convert.
 */
#define MSEC_TO_OS_TICK(TIME) ((TIME) * (RT_TIMER_TICK_PER_SECOND)/1000)

void rt_hw_board_init(void);

#endif
