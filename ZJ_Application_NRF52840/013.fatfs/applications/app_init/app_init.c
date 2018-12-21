#include <stdint.h>
#include <string.h>
#include <rtthread.h>
#include <dfs_romfs.h>
#include "nrf_log.h"
#include "board.h"
#include "app_init.h"
#include "app_uart.h"
#include "app_gpio.h"
#include "app_event.h"

#include "dfs_elm.h"
#include "dfs_fs.h"

#ifdef RT_USING_DFS           
#ifdef RT_USING_DFS_ELMFAT
#define RT_USING_ROMFS 1
int fatfs_rom_mount(void)  
{     
#if RT_USING_ROMFS
     uint32_t ret;
     static const rt_uint8_t _romfs_root_readme_txt[] = "This is ZJ-SDK with RT-Thread for nRF52840";
     
     static const rt_uint8_t _romfs_root_qspi_txt[] = "This is ROM Flash FATFS";
     
     static const struct romfs_dirent _romfs_root_qspi[] = {
         {ROMFS_DIRENT_FILE, "zj_sdk.txt", (rt_uint8_t *)_romfs_root_qspi_txt, sizeof(_romfs_root_qspi_txt)/sizeof(_romfs_root_qspi_txt[0])}
     };
     
     static const struct romfs_dirent _romfs_root[] = {
         {ROMFS_DIRENT_FILE, "readme.txt", (rt_uint8_t *)_romfs_root_readme_txt, sizeof(_romfs_root_readme_txt)/sizeof(_romfs_root_readme_txt[0])},
         {ROMFS_DIRENT_DIR, "zjfs", (rt_uint8_t *)_romfs_root_qspi, sizeof(_romfs_root_qspi)/sizeof(_romfs_root_qspi[0])},
     };    

     static const struct romfs_dirent romfs_root = {
         ROMFS_DIRENT_DIR, "/", (rt_uint8_t *)_romfs_root, sizeof(_romfs_root)/sizeof(_romfs_root[0])
     };

     ret = dfs_mount(RT_NULL, "/", "rom", 0, &(romfs_root)) ;

     if (ret == 0)
     {
         rt_kprintf("ROM file system initializated!\n");
     }
     else
     {
         rt_kprintf("ROM file system initializated FAILED!\n");
     }
    if (dfs_mount("qspifs", "/zjfs", "elm", 0, 0) == 0)
#else
    /* mount sd card fat partition 0 as root directory */
    if (dfs_mount("qspifs", "/", "elm", 0, 0) == 0)
#endif 
    {
        rt_kprintf("qspifs mount to /zjfs OK!\n");                
    }
    else
    {      
      if(dfs_mkfs("elm", "qspifs")==0)
      {
         rt_kprintf("qspifs format OK!\n"); 
      }
      else
      {
         rt_kprintf("qspifs format failed!\n"); 
      }
#if RT_USING_ROMFS      
    if (dfs_mount("qspifs", "/zjfs", "elm", 0, 0) == 0)
#else
    /* mount sd card fat partition 0 as root directory */
    if (dfs_mount("qspifs", "/", "elm", 0, 0) == 0)
#endif 
      {
        rt_kprintf("qspifs mount to /zjfs OK!\n");        
      }
      else
      {
        rt_kprintf("qspifs mount to /zjfs FAILED!\n");
      }
    }
    return 0;
}
//INIT_COMPONENT_EXPORT(fatfs_rom_mount);
#endif /* RT_USING_DFS_ELMFAT */
        
#endif /* DFS */

int main(void)
{
    APP_EVENT_PUSH_DELAY(led_event_start,NULL,500);
    NRF_LOG_INFO("RT-Thread for nrf52840 started");
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT) 
    fatfs_rom_mount();
#endif
    return RT_TRUE;
}


/**
 * @}
 */
