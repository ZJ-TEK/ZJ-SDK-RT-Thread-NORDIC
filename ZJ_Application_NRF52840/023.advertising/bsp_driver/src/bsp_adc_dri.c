#include <rtthread.h>
#include <rtdevice.h>
#include "nrf_drv_saadc.h"
#include "nrf_log.h"
#include "bsp_adc_dri.h"
#include "board.h"


#define adc_printf(...)  rt_kprintf(__VA_ARGS__)
#define ADC_MEASURE_TIMES           4
#define ADC_MEASURE_ACCURACY        14 //NRFX_SAADC_CONFIG_RESOLUTION =3
#define ADC_MEASURE_REFERENCE       600  //SAADC_CH_CONFIG_REFSEL_Internal 0.6V
#define ADC_MEASURE_GAIN            6  // NRF_SAADC_GAIN1_6 
#define ADC_MEASURE_1_2_ADC         2  // 
#define ADC_MEASURE_BIT_TO_MV(adc_value) ((adc_value * ADC_MEASURE_REFERENCE * ADC_MEASURE_GAIN * ADC_MEASURE_1_2_ADC)>>ADC_MEASURE_ACCURACY)
__STATIC_INLINE nrf_saadc_input_t nrf_drv_saadc_gpio_to_ain(uint32_t pin)
{
    // AIN0 - AIN3
    if (pin >= 2 && pin <= 5)
    {
        // 0 means "not connected", hence this "+ 1"
        return (nrf_saadc_input_t)(pin - 2 + 1);
    }
    // AIN4 - AIN7
    else if (pin >= 28 && pin <= 31)
    {
        return (nrf_saadc_input_t)(pin - 24 + 1);
    }
    else
    {
        return NRF_SAADC_INPUT_DISABLED;
    }
}

static void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
        adc_printf("ADC event NRF_DRV_SAADC_EVT_DONE");
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
    }
}

static void saadc_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(nrf_drv_saadc_gpio_to_ain(BATTERY_1_2_ADC_PIN));
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);
}

static void saadc_uninit(void)
{
    nrf_drv_saadc_uninit();
}


uint16_t bsp_get_batt_voltage(void)
{
	ret_code_t err_code;
	float battVal=0, measureSum = 0;
	int16_t battery_adc = 0;

	saadc_init();
	for(uint8_t i = 0; i<ADC_MEASURE_TIMES;i++)
	{
		err_code = nrf_drv_saadc_sample_convert(0, &battery_adc);
        adc_printf("battery_adc = %d\r\n",battery_adc);
		APP_ERROR_CHECK(err_code);
		measureSum += battery_adc;
	}
	saadc_uninit();
	battVal = measureSum / ADC_MEASURE_TIMES;
    adc_printf("battVal = "NRF_LOG_FLOAT_MARKER"\r\n",NRF_LOG_FLOAT(battVal));
    battery_adc = ADC_MEASURE_BIT_TO_MV((uint16_t)battVal);
    adc_printf("battVal = %d mv !\r\n",battery_adc);
	return(battery_adc);
}
FINSH_FUNCTION_EXPORT_ALIAS(bsp_get_batt_voltage, bat_adc , battery adc test);
MSH_CMD_EXPORT_ALIAS(bsp_get_batt_voltage, bat_adc, battery adc test);
