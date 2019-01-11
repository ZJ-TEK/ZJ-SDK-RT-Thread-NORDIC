#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_nus.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_ble_init.h"
#include "app_conn_params.h"
#include "app_ble_adv.h"

#define APP_FAST_ADV_INTERVAL            MSEC_TO_UNITS(250, UNIT_0_625_MS)          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 250 ms). */
#define APP_FAST_ADV_DURATION            3000                                       /**< The advertising duration (30 seconds) in units of 10 milliseconds. */
                                                                                  
#define APP_SLOW_ADV_INTERVAL            MSEC_TO_UNITS(2000, UNIT_0_625_MS)        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 2000 ms). */
#define APP_SLOW_ADV_DURATION            18000                                         /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code;
    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
             NRF_LOG_INFO("BLE_ADV_EVT_FAST");
            break;

        case BLE_ADV_EVT_SLOW:
             rt_kprintf("BLE_ADV_EVT_SLOW\r\n");
            break;

        case BLE_ADV_EVT_IDLE:
             rt_kprintf("BLE_ADV_EVT_IDLE\r\n");
             uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_SLOW);
             APP_ERROR_CHECK(err_code);
//            sleep_mode_enter();
            break;
        default:
            break;
    }
}

/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));  
//******名字*****************/
    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
//******appearance****************/   
    init.advdata.include_appearance      = true;
    
//******falg*****************/
#if NRF_MODULE_ENABLED(PEER_MANAGER_ADV_WHITELIST)  
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
#else
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
#endif

 //******服务UUID*****************/
    static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_CYCLING_SPEED_AND_CADENCE, BLE_UUID_TYPE_BLE},
                                       {BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}}; /**< Universally unique service identifiers. */
    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;  
    
/******希望的服务UUID****************/
    static ble_uuid_t solicited_adv_uuids[1] ; 
    solicited_adv_uuids[0].uuid      = BLE_UUID_NUS_SERVICE;
    solicited_adv_uuids[0].type      = BLE_UUID_TYPE_BLE;    
    init.advdata.uuids_solicited.uuid_cnt = sizeof(solicited_adv_uuids) / sizeof(solicited_adv_uuids[0]);
    init.advdata.uuids_solicited.p_uuids  = solicited_adv_uuids;   
    
//******连接间隔****************/
    ble_advdata_conn_int_t  slave_conn_int = 
    {
      .min_conn_interval            = MSEC_TO_UNITS(7.5, UNIT_1_25_MS),
      .max_conn_interval            = MSEC_TO_UNITS(30, UNIT_1_25_MS)
    };
    init.srdata.p_slave_conn_int        = &slave_conn_int ;  

//******发射功率*****************/
    int8_t tx_power_level = 4;
    err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV,BLE_GAP_TX_POWER_ROLE_ADV,tx_power_level);
    init.srdata.p_tx_power_level        = &tx_power_level;

//******厂商数据*****************/
    ble_advdata_manuf_data_t manufData;
    ble_gap_addr_t nRF52832_add;		
    uint8_t mac_addr[6];
    err_code = sd_ble_gap_addr_get(&nRF52832_add);
    APP_ERROR_CHECK(err_code);
    for(uint8_t i=0;i < BLE_GAP_ADDR_LEN;i++)
    {
    	 mac_addr[i] = nRF52832_add.addr[5-i];
    }	
    manufData.company_identifier = 'J'<<8|'Z';;
    manufData.data.size = sizeof(mac_addr);
    manufData.data.p_data = mac_addr; 
    
    init.advdata.p_manuf_specific_data    = &manufData;

//******地址****************/
//    init.srdata.include_ble_device_addr  = true;

#if NRF_MODULE_ENABLED(PEER_MANAGER_ADV_WHITELIST)      
    init.config.ble_adv_whitelist_enabled = true;
#endif
    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_FAST_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_FAST_ADV_DURATION;
                                            
    init.config.ble_adv_slow_enabled  = true;
    init.config.ble_adv_slow_interval = APP_SLOW_ADV_INTERVAL;
    init.config.ble_adv_slow_timeout  = APP_SLOW_ADV_DURATION;

    init.evt_handler = on_adv_evt;
    
    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);   
}



/**@brief Function for starting advertising.
 */
void advertising_start(void * p_context)
{
    rt_kprintf("advertising_start.....\r\n");
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}
