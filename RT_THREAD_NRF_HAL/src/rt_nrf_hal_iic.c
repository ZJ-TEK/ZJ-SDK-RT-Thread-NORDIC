#include <rtthread.h>
#include <rtdevice.h>
#include "rt_nrf_hal_iic.h"
#include "nrf_log.h"
#include "board.h"
#include "app_timer.h"
#include "nrf_drv_twi.h"

#define i2c_printf(...)  //rt_kprintf(__VA_ARGS__)
static volatile bool m_xfer_done = false;
#define WAIT_FOR_PERIPH() do { \
        uint16_t wait_cnt = 500; \
        while (!m_xfer_done && wait_cnt--) {/*qspi_printf("WAIT_FOR_PERIPH = %d\r\n",wait_cnt);*/rt_thread_delay(5);} \
        m_xfer_done = false;    \
    } while (0)

static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(BSP_TWI_INSTANCE);

#if 0
static ret_code_t eeprom_write(uint16_t addr, uint8_t const * pdata, size_t size)
{
    ret_code_t ret;
    do
    {
        uint8_t buffer[size+2]; /* Addr + data */
        buffer[0] = (uint8_t)(addr>>8);
        buffer[1] = (uint8_t)addr;
        memcpy(buffer + EEPROM_ADDRESS_LEN_BYTES, pdata, size);
        ret = nrf_drv_twi_tx(&m_twi_master, EEPROM_PHY_7BITS_ADDRESS, buffer, size + EEPROM_ADDRESS_LEN_BYTES, false);
       if (NRF_SUCCESS != ret)
       {
           rt_kprintf("wx ret=%d\r\n",ret);
           break;
       }
       WAIT_FOR_PERIPH();
    }while (0);
    return ret;
}

static ret_code_t eeprom_read(uint16_t addr, uint8_t * pdata, size_t size)
{
    ret_code_t ret;
    do
    {
       uint8_t paddr[2] ={0}; 
	   paddr[0] = (uint8_t)(addr>>8);
	   paddr[1] = (uint8_t)addr;
       ret = nrf_drv_twi_tx(&m_twi_master, EEPROM_PHY_7BITS_ADDRESS, paddr, EEPROM_ADDRESS_LEN_BYTES, true);
       if (NRF_SUCCESS != ret)
       {
           rt_kprintf("rx ret=%d\r\n",ret);
           break;
       }
       WAIT_FOR_PERIPH();
       ret = nrf_drv_twi_rx(&m_twi_master, EEPROM_PHY_7BITS_ADDRESS, pdata, size);
       if (NRF_SUCCESS != ret)
       {
           rt_kprintf("rwx ret=%d\r\n",ret);
           break;
       }
       WAIT_FOR_PERIPH();
    }while (0);
    return ret;
}

static ret_code_t eeprom_writeonebyte(uint16_t addr, uint8_t data)
{
    return eeprom_write(addr, &data, 1);
}

static ret_code_t eeprom_readonebyte(uint16_t addr, uint8_t *data)
{
	return eeprom_read(addr, data, 1);
}

#if 0
void eeprom_test()
{
    char tttt[] ="this is ZJ epprom test!";
    char read_test[sizeof(tttt)];
    eeprom_write(1,(uint8_t*)tttt,sizeof(tttt));
    eeprom_read(1,(uint8_t*)read_test,sizeof(tttt));
    rt_kprintf("---%s\r\n",read_test);  
    
    eeprom_writeonebyte(25,0x55);
    uint8_t rrdd=0;
    for(uint8_t i = 0;i<sizeof(tttt);i++)
    {
      eeprom_readonebyte(i+1,&rrdd); 
      rt_kprintf("%d ",rrdd); 
    }
}
FINSH_FUNCTION_EXPORT_ALIAS(eeprom_test, eeprom, epprom test)
#endif
#endif

static void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
               m_xfer_done = true;
               i2c_printf("NRF_DRV_TWI_XFER_RX = %d\r\n",m_xfer_done);
            }
            else if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TX)
            {
               m_xfer_done = true;
               i2c_printf("NRF_DRV_TWI_XFER_TX = %d\r\n",m_xfer_done);
            }
            break;
        default:
            break;
    }
}

static int twi_master_init(void)
{
    ret_code_t ret;
    const nrf_drv_twi_config_t config =
    {
       .scl                = BSP_IIC_SCL_PIN,
       .sda                = BSP_IIC_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOW,
       .clear_bus_init     = false
    };

    ret = nrf_drv_twi_init(&m_twi_master, &config, twi_handler, NULL);

    if (NRF_SUCCESS == ret)
    {
        nrf_drv_twi_enable(&m_twi_master);
    }

    return ret;
}
//INIT_DEVICE_EXPORT(twi_master_init);

//////////////////////////////////////////////////////////////////////////
static struct rt_i2c_bus_device nrf52840_i2c;

static ret_code_t rt_bsp_iic_write(uint16_t addr, uint8_t const * pdata, size_t size,uint16_t flags)
{
    ret_code_t ret;
    do
    {
        i2c_printf("wx flags & 0x40=%x\r\n",flags & 0x40);
        while(nrf_drv_twi_is_busy(&m_twi_master));
        ret = nrf_drv_twi_tx(&m_twi_master, addr, pdata, size, ((flags & 0x40) == RT_I2C_NO_READ_ACK)?true:false);
       if (NRF_SUCCESS != ret)
       {
           rt_kprintf("wx ret=%d\r\n",ret);
           break;
       }
       WAIT_FOR_PERIPH();
    }while (0);
    return ret;
}

static ret_code_t rt_bsp_iic_read(uint16_t addr, uint8_t * pdata, size_t size)
{
    ret_code_t ret;
    do
    {
       while(nrf_drv_twi_is_busy(&m_twi_master));
       ret = nrf_drv_twi_rx(&m_twi_master, addr, pdata, size);
       if (NRF_SUCCESS != ret)
       {
           rt_kprintf("rwx ret=%d\r\n",ret);
           break;
       }
       WAIT_FOR_PERIPH();
    }while (0);
    return ret;
}

static rt_size_t rt_iic_master_xfer(struct rt_i2c_bus_device *bus,
                             struct rt_i2c_msg msgs[],
                             rt_uint32_t num)
{
    rt_size_t ret = 0;
    if((msgs->flags & 0x01) == RT_I2C_WR)
    {
        if(rt_bsp_iic_write(msgs->addr, msgs->buf, msgs->len,msgs->flags) == NRF_SUCCESS)
        {
            ret = msgs->len;
        }
    }
    else if((msgs->flags & 0x01) == RT_I2C_RD)
    {
        if(rt_bsp_iic_read(msgs->addr, msgs->buf, msgs->len) == NRF_SUCCESS)
        {
            ret = msgs->len;
        }
    }
    return (ret == msgs->len) ? msgs->len : 0;
}

static const struct rt_i2c_bus_device_ops eeprom_ops =
{
     .master_xfer = rt_iic_master_xfer,
     .slave_xfer = NULL,
     .i2c_bus_control = NULL,
};

int rt_hw_i2c_init(void)
{ 
    twi_master_init();
    nrf52840_i2c.ops= &eeprom_ops;
    rt_i2c_bus_device_register(&nrf52840_i2c, "i2c1");        
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_i2c_init);//rt_hw_i2c_init will be called in rt_components_board_init()


