/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "sdk_common.h"
#include <rtthread.h>
#if RT_USING_TIMER_SOFT
#include "app_timer.h"
#include <stdlib.h>
#include "nrf.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "app_util_platform.h"

ret_code_t app_timer_init(void)
{
    return NRF_SUCCESS;
}

static void _timeout_entry(void *paramter)
{
    app_timer_id_t p_timer_id;

    p_timer_id = paramter;

    if (p_timer_id->callback != RT_NULL)
    {
        p_timer_id->callback(p_timer_id->parameter);
    }
}

ret_code_t app_timer_create(app_timer_id_t              timer_id,
                            app_timer_mode_t            mode,
                            app_timer_timeout_handler_t timeout_handler)
{
    app_timer_id_t p_timer_id = timer_id;

    if (timeout_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    p_timer_id->callback = timeout_handler;

    if (mode == APP_TIMER_MODE_SINGLE_SHOT)
    {
        p_timer_id->id = rt_timer_create("apptimer", _timeout_entry, p_timer_id,
                            0, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
    }
    else
    {
        p_timer_id->id = rt_timer_create("apptimer", _timeout_entry, p_timer_id,
                            0, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
    }

    if (p_timer_id->id != RT_NULL)
    {
        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM; // This error is unspecified by rtx
    }
}

ret_code_t app_timer_start(app_timer_id_t timer_id, uint32_t timeout_ms, void * p_context)
{
    app_timer_id_t p_timer_id = timer_id;
    int32_t timeout_ticks =  APP_TIMER_TICKS(timeout_ms);
    if ((timeout_ticks < APP_TIMER_MIN_TIMEOUT_TICKS))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (p_timer_id->id == RT_NULL)
        return NRF_ERROR_INVALID_PARAM;

    if (p_timer_id->id->parent.flag & RT_TIMER_FLAG_ACTIVATED)
    {
        return NRF_SUCCESS;
    }

    p_timer_id->parameter = p_context;

    rt_timer_control(p_timer_id->id, RT_TIMER_CTRL_SET_TIME, &timeout_ticks);

    switch (rt_timer_start(p_timer_id->id))
    {
        case RT_EOK:
            return NRF_SUCCESS;

        default:
            return NRF_ERROR_INVALID_PARAM;
    }
}

ret_code_t app_timer_stop(app_timer_id_t timer_id)
{
    app_timer_id_t p_timer_id = timer_id;

    if (p_timer_id != RT_NULL && p_timer_id->id != RT_NULL)
    {
//         if (p_timer_id->id->parent.flag & RT_TIMER_FLAG_ACTIVATED)
        {
            rt_timer_stop(p_timer_id->id);
            return NRF_SUCCESS;
        }
        
    }

    return NRF_ERROR_INVALID_PARAM;
}

uint32_t app_timer_cnt_get(void)
{
    return rt_tick_get();
}

uint32_t app_timer_cnt_diff_compute(uint32_t ticks_now, uint32_t ticks_old)
{
#define MAX_RTC_COUNTER_VAL ((1U<<24)-1U)   
    return ((ticks_now - ticks_old) & MAX_RTC_COUNTER_VAL);
}

#endif //NRF_MODULE_ENABLED(APP_TIMER)

