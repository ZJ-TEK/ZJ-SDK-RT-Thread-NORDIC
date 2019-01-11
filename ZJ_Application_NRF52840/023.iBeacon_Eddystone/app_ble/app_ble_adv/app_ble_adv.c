#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_nus.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_ble_init.h"
#include "app_conn_params.h"
#include "app_ble_adv.h"
#include "crc32.h"

#define USE_iBEACON           1
#define USE_NRF_BEACON        2
#define USE_EDDYSTONE         3

uint8_t iBEACON_NRF_BEACON_EDDYSTONE_ENABLE = USE_NRF_BEACON;  //1 iBeacon 2 nRF_Beacon 3 Eddystone


#define NON_CONNECTABLE_ADV_INTERVAL     MSEC_TO_UNITS(100, UNIT_0_625_MS)  /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define APP_BEACON_INFO_LENGTH          0x17                               /**< Total length of information advertised by the Beacon. */
#define APP_ADV_DATA_LENGTH             0x15                               /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                               /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                               /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_NORDIC_COMPANY_IDENTIFIER   0x0059                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_APPLE_IDENTIFIER            0x004C                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE                 0x01, 0x02                         /**< Major value used to identify Beacons. */
#define APP_MINOR_VALUE                 0x03, 0x04                         /**< Minor value used to identify Beacons. */
//#define APP_BEACON_UUID                 0x01, 0x12, 0x23, 0x34, \
//                                        0x45, 0x56, 0x67, 0x78, \
//                                        0x89, 0x9a, 0xab, 0xbc, \
//                                        0xcd, 0xde, 0xef, 0xf0            /**< Proprietary UUID for Beacon. */

//#define APP_BEACON_UUID                 0x59, 0x46, 0x50, 0xA2, \
//                                        0x86, 0x21, 0x40, 0x1f, \
//                                        0xB5, 0xDE, 0x6e, 0xb3, \
//                                        0xee, 0x39, 0x81, 0x70            /**< Proprietary UUID for Beacon. */
                                        
#define APP_BEACON_UUID                 0xe2, 0xc5, 0x6d, 0xb5, \
                                        0xdf, 0xfb, 0x48, 0xD2, \
                                        0xb0, 0x60, 0xd0, 0xf5, \
                                        0xa7, 0x10, 0x96, 0xe0            /**< Proprietary UUID for Beacon. */

#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
#define MAJ_VAL_OFFSET_IN_BEACON_INFO   18                                 /**< Position of the MSB of the Major Value in m_beacon_info array. */
#define UICR_ADDRESS                    0x10001080                         /**< Address of the UICR register used by this example. The major and minor versions to be encoded into the advertising data will be picked up from this location. */
#endif

static uint8_t m_beacon_info[APP_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
                         // implementation.
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
                         // manufacturer specific data in this implementation.
    APP_BEACON_UUID,     // 128 bit UUID value.
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
                         // this implementation.
};


#define BLE_UUID_GOOGLE_EDDYSTONE 0xFEAA    
#define EDDYSTONE_URL_FRAME_TYPE  EDDYSTONE_URL_FRAME_URL_TYPE
#define EDDYSTONE_TX_POWER        0xEE

//#define USER_URL "bbs.codertown.cn"
//#define EDDYSTONE_URL_PREFIXES    EDDYSTONE_URL_PREFIXES_http
#define USER_URL "rt-thread.org/" //https://www.
#define EDDYSTONE_URL_PREFIXES    EDDYSTONE_URL_PREFIXES_https_www




ble_eddystone_url_type_t ble_eddystone_url_type=
{
   .url_frame_type = EDDYSTONE_URL_FRAME_TYPE,
   .Tx_power       = EDDYSTONE_TX_POWER,
   .url_prefixes   = EDDYSTONE_URL_PREFIXES,    
   .data           = USER_URL,
   .len            = (sizeof(USER_URL)-1) + 3, 
};


static ble_gap_adv_params_t m_adv_params;                                  /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */
static uint8_t              m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = NULL,
        .len    = 0

    }
};

static void mac_to_calcute_major_minor(uint8_t *p_crc_data)
{
    uint32_t         err_code;
    ble_gap_addr_t   mac_addr;
    int len;
    uint32_t crcdata;
    err_code = sd_ble_gap_addr_get(&mac_addr);
    APP_ERROR_CHECK(err_code);

    crcdata = crc32_compute(mac_addr.addr, 6, NULL);
    p_crc_data[0] = (uint8_t)(crcdata>>24);
    p_crc_data[1] = (uint8_t)(crcdata>>16);
    p_crc_data[2] = (uint8_t)(crcdata>>8);
    p_crc_data[3] = (uint8_t)(crcdata>>0);
    rt_kprintf("ble mac addr: id=%d, type=%d\r\n[%02X:%02X:%02X:%02X:%02X:%02X]\r\n", \
                mac_addr.addr_id_peer, mac_addr.addr_type, \
                mac_addr.addr[5], mac_addr.addr[4], mac_addr.addr[3], \
                mac_addr.addr[2], mac_addr.addr[1], mac_addr.addr[0]);
    rt_kprintf("CRC32 data = 0x%x Major:%d Minor:%d\r\n",crcdata,(crcdata>>16)&0xffff,crcdata & 0xffff);
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED | BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE;
    
    if(iBEACON_NRF_BEACON_EDDYSTONE_ENABLE < USE_EDDYSTONE)
    {
        //******manuf data*****************/
        ble_advdata_manuf_data_t manuf_specific_data;
#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
        // If USE_UICR_FOR_MAJ_MIN_VALUES is defined, the major and minor values will be read from the
        // UICR instead of using the default values. The major and minor values obtained from the UICR
        // are encoded into advertising data in big endian order (MSB First).
        // To set the UICR used by this example to a desired value, write to the address 0x10001080
        // using the nrfjprog tool. The command to be used is as follows.
        // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val <your major/minor value>
        // For example, for a major value and minor value of 0xabcd and 0x0102 respectively, the
        // the following command should be used.
        // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val 0xabcd0102
        uint16_t major_value = ((*(uint32_t *)UICR_ADDRESS) & 0xFFFF0000) >> 16;
        uint16_t minor_value = ((*(uint32_t *)UICR_ADDRESS) & 0x0000FFFF);
    
        uint8_t index = MAJ_VAL_OFFSET_IN_BEACON_INFO;
    
        m_beacon_info[index++] = MSB_16(major_value);
        m_beacon_info[index++] = LSB_16(major_value);
    
        m_beacon_info[index++] = MSB_16(minor_value);
        m_beacon_info[index++] = LSB_16(minor_value);
#else
         #define MAJ_VAL_OFFSET_IN_BEACON_INFO   18
         mac_to_calcute_major_minor(&m_beacon_info[MAJ_VAL_OFFSET_IN_BEACON_INFO]);
#endif
    
        if(iBEACON_NRF_BEACON_EDDYSTONE_ENABLE == USE_NRF_BEACON)
        {
           manuf_specific_data.company_identifier = APP_NORDIC_COMPANY_IDENTIFIER;
        }
        else if(iBEACON_NRF_BEACON_EDDYSTONE_ENABLE == USE_iBEACON)
        {
           manuf_specific_data.company_identifier = APP_APPLE_IDENTIFIER;
        }
    
        manuf_specific_data.data.p_data = (uint8_t *) m_beacon_info;
        manuf_specific_data.data.size   = APP_BEACON_INFO_LENGTH;
        
        advdata.p_manuf_specific_data = &manuf_specific_data;
    }    
    else if (iBEACON_NRF_BEACON_EDDYSTONE_ENABLE == USE_EDDYSTONE)
    {
        //******server UUID*****************/
        static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_GOOGLE_EDDYSTONE, BLE_UUID_TYPE_BLE}};
        advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
        advdata.uuids_complete.p_uuids  = m_adv_uuids;  
    
        //******server data *****************/    
        ble_advdata_service_data_t service_data_array[] =
        {
            {.service_uuid = BLE_UUID_GOOGLE_EDDYSTONE,
            .data.size =  ble_eddystone_url_type.len,
            .data.p_data = (uint8_t*)&ble_eddystone_url_type,}
        }; 
        
        advdata.p_service_data_array = service_data_array;   
        advdata.service_data_count = sizeof(service_data_array) / sizeof(service_data_array[0]);
    }


    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration        = 0;       // Never time out.

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(void * p_context)
{
    static bool is_adv_enabled =false;
    rt_kprintf("p_context = %d\r\n",(uint8_t)((uint32_t)p_context));
    if(p_context)
    {
       iBEACON_NRF_BEACON_EDDYSTONE_ENABLE = (uint8_t)((uint32_t)p_context);
       if(is_adv_enabled == true)
       {
         sd_ble_gap_adv_stop(m_adv_handle);
         is_adv_enabled = false;
       }
       
       advertising_init();
    }
    
    if(iBEACON_NRF_BEACON_EDDYSTONE_ENABLE == USE_iBEACON)
    {
       rt_kprintf("[*iBeacon*] advertising_start...\r\n");        
    }
    else if(iBEACON_NRF_BEACON_EDDYSTONE_ENABLE == USE_NRF_BEACON)
    {
       rt_kprintf("[*nRF Beacon*] advertising_start...\r\n");                
    }
    else if(iBEACON_NRF_BEACON_EDDYSTONE_ENABLE == USE_EDDYSTONE)
    {
       rt_kprintf("[*Eddystone*] advertising_start.....\r\n");               
    }
           
    uint32_t err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);
    is_adv_enabled = true;
}
FINSH_FUNCTION_EXPORT_ALIAS(advertising_start, __cmd_Beacon_start, choose beacon type Beacon_start(n) 1=iBeacon 2=nRF_Beacon 3=Eddystone);

uint8_t get_suffixes_data(char *argv,uint8_t url_length)
{
    if(strstr(argv,".com/"))
    {
        if(strcmp(&argv[strlen(argv)-5],".com/") == 0)
        {
           url_length -= 5;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_com_; 
           return  url_length;           
        }
    }
    else if(strstr(argv,".org/"))
    {
        if(strcmp(&argv[strlen(argv)-5],".org/") == 0)
        {
           url_length -= 5;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_org_;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".edu/"))
    {
        if(strcmp(&argv[strlen(argv)-5],".edu/") == 0)
        {
           url_length -= 5;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_edu_;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".net/"))
    {
        if(strcmp(&argv[strlen(argv)-5],".net/") == 0)
        {
           url_length -= 5;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_net_;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".info/"))
    {
        if(strcmp(&argv[strlen(argv)-6],".info/") == 0)
        {
           url_length -= 6;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_info_;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".biz/"))
    {
        if(strcmp(&argv[strlen(argv)-5],".biz/") == 0)
        {
           url_length -= 5;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_biz_;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".gov/"))
    {
        if(strcmp(&argv[strlen(argv)-5],".gov/") == 0)
        {
           url_length -= 5;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_gov_;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".com"))
    {
        if(strcmp(&argv[strlen(argv)-4],".com") == 0)
        {
           url_length -= 4;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_com;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".org"))
    {
        if(strcmp(&argv[strlen(argv)-4],".org") == 0)
        {
           url_length -= 4;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_org;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".edu"))
    {
        if(strcmp(&argv[strlen(argv)-4],".edu") == 0)
        {
           url_length -= 4;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_edu;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".net"))
    {
        if(strcmp(&argv[strlen(argv)-4],".net") == 0)
        {
           url_length -= 4;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_net;               
           return  url_length;           
        }
    }
    else if(strstr(argv,".info"))
    {
        if(strcmp(&argv[strlen(argv)-5],".info") == 0)
        {
           url_length -= 5;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_info;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".biz"))
    {
        if(strcmp(&argv[strlen(argv)-4],".biz") == 0)
        {
           url_length -= 4;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_biz;  
           return  url_length;           
        }
    }
    else if(strstr(argv,".gov"))
    {
        if(strcmp(&argv[strlen(argv)-4],".gov") == 0)
        {
           url_length -= 4;
           ble_eddystone_url_type.data[url_length] = EDDYSTONE_URL_SUFFIXES_gov;  
           return  url_length;           
        }
    }
    return 0;
}
void set_Eddyston_url(int argc, char** argv)
{  
    uint8_t temp_length =0;
    uint8_t url_length =0;
    uint8_t ret =0;
    if(argc != 2)
    {
        rt_kprintf("Parameter num error(must = 2)\n");   
        return;
    }
    if(rt_strstr(argv[1],"http") == NULL)
    {
        rt_kprintf("URL format error\n");          
        return;
    }
    rt_kprintf("\r\n");
    rt_kprintf("\r\n");
    if(strncmp("http://www.", argv[1],11) == 0)
    {
        ble_eddystone_url_type.url_prefixes = EDDYSTONE_URL_PREFIXES_http_www;
        url_length = sprintf((char*)ble_eddystone_url_type.data,"%s",&argv[1][11]);
        ret = get_suffixes_data(&argv[1][11],url_length);
        if(strlen(&argv[1][11]) > (17 + (ret?4:0)))
        {
            rt_kprintf("the second Parameter Length error\n");   
            return;        
        }
        rt_kprintf("SET_URL:--->%s",argv[1]);  
    }
    else if(strncmp("https://www.", argv[1],12) == 0)
    {
        ble_eddystone_url_type.url_prefixes = EDDYSTONE_URL_PREFIXES_https_www;
        url_length = sprintf((char*)ble_eddystone_url_type.data,"%s",&argv[1][12]);
        ret = get_suffixes_data(&argv[1][12],url_length);
        if(strlen(&argv[1][12])>(17 + (ret?4:0)))
        {
            rt_kprintf("\r\nthe second Parameter Length error\n");   
            return;        
        }
        rt_kprintf("SET_URL:--->%s",argv[1]);  
    }
    else if(strncmp("http://", argv[1],7) == 0)
    {
        ble_eddystone_url_type.url_prefixes = EDDYSTONE_URL_PREFIXES_http;
        url_length = sprintf((char*)ble_eddystone_url_type.data,"%s",&argv[1][7]);
        ret = get_suffixes_data(&argv[1][7],url_length);
        if(strlen(&argv[1][7])>(17 + (ret?4:0)))
        {
            rt_kprintf("\r\nthe second Parameter Length error\n");   
            return;        
        }
        rt_kprintf("SET_URL:--->%s",argv[1]);  
    }
    else if(strncmp("https://", argv[1],8) == 0)
    {
        ble_eddystone_url_type.url_prefixes = EDDYSTONE_URL_PREFIXES_https;
        url_length = sprintf((char*)ble_eddystone_url_type.data,"%s",&argv[1][8]);
        ret = get_suffixes_data(&argv[1][8],url_length);
        if(strlen(&argv[1][8])>(17 + (ret?4:0)))
        {
            rt_kprintf("\r\nthe second Parameter Length error\n");   
            return;        
        }
        rt_kprintf("SET_URL:--->%s",argv[1]);  
    }
    else
    {
        rt_kprintf("the second Parameter HEAD error\n");   
        return;        
    }
    
    if(ret)
    {
        ble_eddystone_url_type.len = ret + 3 + 1;
    }
    else
    {
        ble_eddystone_url_type.len = url_length + 3;        
    }
    rt_kprintf("\r\n");
    rt_kprintf("\r\n");
    advertising_start((void*)USE_EDDYSTONE);    
}
FINSH_FUNCTION_EXPORT_ALIAS(set_Eddyston_url, __cmd_set_url, choose beacon type Beacon_start(n) 1=iBeacon 2=nRF_Beacon 3=Eddystone);

