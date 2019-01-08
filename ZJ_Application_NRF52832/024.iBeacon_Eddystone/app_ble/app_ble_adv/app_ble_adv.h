#ifndef APP_BLE_ADV_H__
#define APP_BLE_ADV_H__
#include <rtthread.h>

//////////////////////////////////////////////////////////
//Eddystone
#define EDDYSTONE_URL_PREFIXES_http_www  0x00 //"http://www."
#define EDDYSTONE_URL_PREFIXES_https_www 0x01 //"https://www."
#define EDDYSTONE_URL_PREFIXES_http      0x02 //"http://"
#define EDDYSTONE_URL_PREFIXES_https     0x03 //"https://"

#define EDDYSTONE_URL_SUFFIXES_com_  	 0x00 //".com/"
#define EDDYSTONE_URL_SUFFIXES_org_  	 0x01 //".org/"
#define EDDYSTONE_URL_SUFFIXES_edu_  	 0x02 //".edu/"
#define EDDYSTONE_URL_SUFFIXES_net_  	 0x03 //".net/"
#define EDDYSTONE_URL_SUFFIXES_info_  	 0x04 //".info/"
#define EDDYSTONE_URL_SUFFIXES_biz_  	 0x05 //".biz/"
#define EDDYSTONE_URL_SUFFIXES_gov_  	 0x06 //".gov/"
#define EDDYSTONE_URL_SUFFIXES_com   	 0x07 //".com"
#define EDDYSTONE_URL_SUFFIXES_org   	 0x08 //".org"
#define EDDYSTONE_URL_SUFFIXES_edu   	 0x09 //".edu"
#define EDDYSTONE_URL_SUFFIXES_net   	 0x0a //".net"
#define EDDYSTONE_URL_SUFFIXES_info  	 0x0b //".info"
#define EDDYSTONE_URL_SUFFIXES_biz   	 0x0c //".biz"
#define EDDYSTONE_URL_SUFFIXES_gov  	 0x0d //".gov"
                                         
#define EDDYSTONE_URL_FRAME_UID_TYPE     0x00
#define EDDYSTONE_URL_FRAME_URL_TYPE     0x10
#define EDDYSTONE_URL_FRAME_TLM_TYPE     0x20
#define EDDYSTONE_URL_FRAME_EDI_TYPE     0x30

//Eddystone URL
#define BLE_EDDYSTONE_URL_LEN_MAX         17
#ifdef __GNUC__
    #ifdef PACKED
        #undef PACKED
    #endif

    #define PACKED(TYPE) TYPE __attribute__ ((packed))
#endif
typedef PACKED( struct
{
    uint8_t url_frame_type;
    int8_t  Tx_power;
    uint8_t url_prefixes;    
    uint8_t data[BLE_EDDYSTONE_URL_LEN_MAX];
    uint8_t len;
}) ble_eddystone_url_type_t;

void advertising_init(void);
void advertising_start(void * p_context);
#endif










