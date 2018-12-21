#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"


void memory_copy(uint32_t fadd,uint32_t raddstart,uint32_t raddend)
{
    uint32_t flash_add = fadd;
    uint32_t ram_add_start = raddstart;
    uint32_t ram_add_end = raddend;
    //uint8_t datas;
    uint16_t len = ram_add_end - ram_add_start;
    for(uint16_t i = 0;i<len;i++)
    { 
      *((uint8_t*)ram_add_start) = *((uint8_t*)flash_add);
      flash_add++;
      ram_add_start++;
    }
   // memcpy((uint8_t*)ram_add_start,(uint8_t*)flash_add,len);
}


void memory_set(uint32_t raddstart,uint32_t raddend)
{
    uint32_t ram_add_start = raddstart;
    uint32_t ram_add_end = raddend;
    //uint8_t datas;
    uint16_t len = ram_add_end - ram_add_start;
    for(uint16_t i = 0;i<len;i++)
    { 
      *((uint8_t*)ram_add_start) = 0;
      ram_add_start++;
    }
   // memcpy((uint8_t*)ram_add_start,(uint8_t*)flash_add,len);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    nrf_gpio_cfg_output(13);

    /* Toggle LEDs. */
    while (true)
    {
       nrf_gpio_pin_toggle(13);
       nrf_delay_ms(500);
    }
}

/**
 *@}
 **/
