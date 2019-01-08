#ifndef APP_EVENT_H__
#define APP_EVENT_H__
#include <rtthread.h>

//////////////////////////////////////////////////////////////////////////////////////
void app_event_push_delayed(void *pfunc, void *pargs, uint32_t debug, uint32_t delay);
uint8_t app_event_remove_delayed(void *pfunc);

#define APP_EVENT_REMOVE_DELAY(pfunc) app_event_remove_delayed(pfunc)
#define APP_EVENT_PUSH_DELAY(pfunc, pargs, delay) \
do{ \
   /*rt_kprintf("%s:%d:%d: push delay %d\r\n",__FILE__,__LINE__,bsp_get_system_tick(),delay);*/ \
   app_event_push_delayed(pfunc, pargs, __LINE__, delay); \
}while(0)

#endif




