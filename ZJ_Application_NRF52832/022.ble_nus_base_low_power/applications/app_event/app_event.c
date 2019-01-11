#include "app_event.h"
#include "board.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

typedef void (*event_func_t)( void *arg );
typedef struct
{
    void *pargs;
    uint32_t debug;
    event_func_t pfunc;
} event_data_handle_t;

#define MAX_DELAY_EVENT 64
typedef struct
{
    uint32_t  event[MAX_DELAY_EVENT];
    uint32_t  args[MAX_DELAY_EVENT];
    uint32_t  delay[MAX_DELAY_EVENT];
    uint32_t  debug[MAX_DELAY_EVENT];
} event_delay_treat_t;

#define APP_EVENT_MESQ_NUM 12
static struct rt_messagequeue app_event_mesq;
static uint8_t event_mq_buffer[APP_EVENT_MESQ_NUM*sizeof(event_data_handle_t)];

static struct rt_mutex app_event_mutex;
static struct rt_timer app_event_timer;
static struct rt_semaphore timeout_event_sem;

#define EVENT_TASK_SIZE 1024
#define EVENT_THREAD_PRIORITY    3
static void app_event_process_thread(void *args);
static struct rt_thread  app_event_thread;
static uint8_t event_task_stack_buffer[EVENT_TASK_SIZE];

#define TIMEOUT_TASK_SIZE 1024
#define TIMEOUT_THREAD_PRIORITY   4
static void event_delay_timeout_handler(void *parameter);
static void app_timeout_process_thread(void *args);
static struct rt_thread  app_timeout_thread;
static uint8_t timer_task_stack_buffer[TIMEOUT_TASK_SIZE];

static volatile uint32_t s_event_start_ticks = 0;
static volatile event_delay_treat_t event_delay_treat;
static volatile uint32_t event_current_debug;
static volatile uint32_t event_current_event;

#define event_printf(...) 	//NRF_LOG_INFO(__VA_ARGS__)
///////////////////////////////////////////////////////////////////////////////////
static int app_event_init(void)
{
    rt_err_t ret;
    event_printf("app_event_mesq..\r\n");
    ret =  rt_mq_init(&app_event_mesq,
                     "evtmqt",
                      &event_mq_buffer[0],
                      sizeof(event_data_handle_t), 
                      sizeof(event_mq_buffer),
                      RT_IPC_FLAG_FIFO);
    APP_ERROR_CHECK(ret);

    if (ret)
    {
        event_printf("app_event_mesq create failure\n");
    }

    rt_timer_init(&app_event_timer,
                  "evttimer",
                  event_delay_timeout_handler,  
                  NULL,  
                  RT_WAITING_FOREVER,  
                  RT_TIMER_FLAG_ONE_SHOT);      

    if(rt_thread_find("evttask"))
    {
        rt_thread_detach(&app_event_thread);
    }

    if(rt_thread_find("evttask") == NULL)
    {
        ret = rt_thread_init(&app_event_thread,
                             "evttask",
                             app_event_process_thread,
                             RT_NULL,
                             &event_task_stack_buffer[0],
                             EVENT_TASK_SIZE,
                             EVENT_THREAD_PRIORITY,
                             20);
        APP_ERROR_CHECK(ret);

        if (ret == RT_EOK) 
        {
              rt_thread_startup(&app_event_thread);
        }
    }
    event_printf("app_event_thread creat ok!!\r\n");
    if(rt_thread_find("evttimer"))
    {
        rt_thread_detach(&app_event_thread);
    } 
    if(rt_thread_find("evttimer") == NULL)
    {
        ret = rt_thread_init(&app_timeout_thread,
                             "evttimer",
                              app_timeout_process_thread, 
                              RT_NULL,
                              &timer_task_stack_buffer[0],
                              TIMEOUT_TASK_SIZE,
                              TIMEOUT_THREAD_PRIORITY,
                              20);
        APP_ERROR_CHECK(ret);
        if (ret == RT_EOK) 
        {
            rt_thread_startup(&app_timeout_thread);
        }
    }
    event_printf("app_timeout_thread creat ok!!\r\n");    
    ret = rt_sem_init(&timeout_event_sem,"evtsem", 0, RT_IPC_FLAG_FIFO);
    APP_ERROR_CHECK(ret);
    if (ret)
    {
        event_printf("timeout_event_sem create failure = %d\n",ret);
    }

    event_printf("app_event_mutex.owner = %x\r\n",app_event_mutex.owner);
    if(app_event_mutex.owner == NULL)
    {
        event_printf("wait app event init\r\n");
        while(1)
        {
            if(&app_event_timer)
            {
                break;
            }
        }
        ret = rt_mutex_init(&app_event_mutex,"mutex", RT_IPC_FLAG_FIFO);
        APP_ERROR_CHECK(ret);
    }
    event_printf("app event init complite\r\n");
    return RT_TRUE;
}
INIT_APP_EXPORT(app_event_init);

static void app_event_process_thread(void *args)
{
    uint32_t status;
    event_data_handle_t app_event;
    event_printf("app_event_process_thread\r\n");

    for(;;)
    {
        status = rt_mq_recv(&app_event_mesq, &app_event, sizeof(event_data_handle_t), RT_WAITING_FOREVER);
        if(status != RT_EOK)
        {
            event_printf("app treat event invalid status %d\r\n", status);
            continue;
        }

        event_printf("app treat event=0x%x tick=%d++\r\n",(uint32_t)app_event.pfunc,rt_tick_get());
        event_current_event = (uint32_t)app_event.pfunc;
        event_current_debug = app_event.debug;
        app_event.pfunc(app_event.pargs);
        event_current_event = 0;
        event_current_debug = 0;
        event_printf("app treat event=0x%x tick=%d--\r\n",(uint32_t)app_event.pfunc,rt_tick_get());
    }
}

uint8_t app_event_push(void *pfunc, void *pargs, uint32_t debug)
{
    uint32_t ret;
    event_data_handle_t app_event;
    if(rt_thread_find("evttask") == NULL)
    {
        return 0;
    }
    app_event.pfunc = (event_func_t)pfunc;
    app_event.pargs = pargs;
    ret = rt_mq_send(&app_event_mesq, (void *)&app_event, sizeof(event_data_handle_t));
    if(ret)
    {
        //APP_ERROR_CHECK(ret);
        event_printf("app_event_push error 0x%x, func=0x%x debug=%d\r\n",ret,event_current_event,event_current_debug);
    }
    return ret;
}

static void app_timeout_process_thread(void *args)
{
    while(1)
    { 
        rt_sem_take(&timeout_event_sem, RT_WAITING_FOREVER);
        int i;
        uint32_t event;
        int32_t next_delay = INT32_MAX;
       
        uint32_t elapsed = 0;//(uint32_t)app_event_timer->init_tick;
        rt_timer_control(&app_event_timer, RT_TIMER_CTRL_GET_TIME, &elapsed);
        APP_ERROR_CHECK(rt_mutex_take(&app_event_mutex, 1000));
        for (i = 0; i < MAX_DELAY_EVENT; i++)
        {
            if (event_delay_treat.event[i])
            {
                if(event_delay_treat.delay[i] > elapsed)
                {
                    event_delay_treat.delay[i] -= elapsed;
                }
                else
                {
                    event_delay_treat.delay[i] = 0;
                }
       
                if (event_delay_treat.delay[i] <= 0)
                {
                    event = event_delay_treat.event[i];
                    event_delay_treat.event[i] = 0x00;
                    app_event_push((void *)event, (void *)event_delay_treat.args[i], event_delay_treat.debug[i]);
                }
            }
        }

        //get next event delay
        for (i = 0; i < MAX_DELAY_EVENT; i++)
        {
            if (event_delay_treat.event[i])
            {
                APP_ERROR_CHECK_BOOL(event_delay_treat.delay[i] > 0);
                if (event_delay_treat.delay[i] < next_delay)
                {
                    next_delay = event_delay_treat.delay[i];
                }
            }
        }
        
        APP_ERROR_CHECK(rt_mutex_release(&app_event_mutex));
        
        //schedule again
        if ((INT32_MAX != next_delay) && (next_delay > 0))
        {
            rt_timer_control(&app_event_timer, RT_TIMER_CTRL_SET_TIME, &next_delay);
            rt_timer_start(&app_event_timer);
        
            s_event_start_ticks = rt_tick_get();
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
static void event_delay_timeout_handler(void *parameter)
{
    rt_sem_release(&timeout_event_sem);         
}

void app_event_push_delayed(void *pfunc, void *pargs, uint32_t debug, uint32_t delay)
{
    int i = 0;
    uint32_t delay_diff;
    uint32_t delay_ticks = MSEC_TO_OS_TICK(delay);

    event_printf("app push event=0x%x delay=%d tick=%d\r\n", pfunc, delay, rt_tick_get());

    if(delay_ticks <= 1)
    {
        app_event_push((void *)pfunc, pargs, debug);
    }
    else
    {
        //find idle
        APP_ERROR_CHECK(rt_mutex_take(&app_event_mutex, 1000));
        for (i = 0; i < MAX_DELAY_EVENT; i++)
        {
            if (0x00 == event_delay_treat.event[i])
            {
                break;
            }
        }

        if (i < MAX_DELAY_EVENT)
        {
            event_delay_treat.event[i] = (uint32_t)pfunc;
            event_delay_treat.args[i] = (uint32_t)pargs;
            event_delay_treat.debug[i] = debug;
            delay_diff = rt_tick_get() - s_event_start_ticks;//ms
            event_delay_treat.delay[i] = delay_ticks + delay_diff;
        }
        APP_ERROR_CHECK(rt_mutex_release(&app_event_mutex));
        APP_ERROR_CHECK(i >= MAX_DELAY_EVENT);

        if (i < MAX_DELAY_EVENT)
        {
            rt_timer_stop(&app_event_timer);
            rt_timer_control(&app_event_timer, RT_TIMER_CTRL_SET_TIME, &delay_diff);
            event_delay_timeout_handler(NULL);//
        }
    }
}

uint8_t app_event_remove_delayed(void *pfunc)
{
    uint8_t removed = 0;
    event_printf("app_event_remove_delayed %p\r\n", pfunc);

    APP_ERROR_CHECK(rt_mutex_take(&app_event_mutex, 1000));
    for (int i = 0; i < MAX_DELAY_EVENT; i++)
    {
        if ((uint32_t)pfunc == event_delay_treat.event[i])
        {
            event_delay_treat.event[i] = 0x00;
            removed = 1;
            event_printf("app remove event=0x%x tick=%d\r\n", pfunc, rt_tick_get());
        }
    }
    APP_ERROR_CHECK(rt_mutex_release(&app_event_mutex));

    if (removed)
    {
        uint32_t ticks_diff = rt_tick_get() - s_event_start_ticks;//ms
        rt_timer_stop(&app_event_timer);
        rt_timer_control(&app_event_timer, RT_TIMER_CTRL_SET_TIME, &ticks_diff);
        event_delay_timeout_handler(NULL);
    }
    return removed;
}
