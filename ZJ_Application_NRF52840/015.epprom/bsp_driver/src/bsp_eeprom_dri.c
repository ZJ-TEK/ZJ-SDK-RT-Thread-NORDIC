#include <rtthread.h>
#include <rtdevice.h>
#include "bsp_eeprom_dri.h"
#include "nrf_log.h"
#include "board.h"
#include "app_timer.h"

#define eeprom_printf(...)  //rt_kprintf(__VA_ARGS__)

#ifndef EEP_I2CBUS_NAME
#define EEP_I2CBUS_NAME          "i2c1" 
#endif

#define EEPROM_CHECK_ADDR           256  
#define EEPROM_TOTAL_SIZE           256  
#define EEPROM_ONE_SEQ_WRITE_MAX    8
#define EEPROM_PAGE_ALGIN           8

static struct rt_i2c_bus_device *eep_i2c_bus = RT_NULL;
static int bsp_eeprom_init(void)
{
    eep_i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(EEP_I2CBUS_NAME);
    if(eep_i2c_bus == RT_NULL)
    {    
       eeprom_printf("\ni2c_bus %s for EEPROM not found!\n",EEP_I2CBUS_NAME);
       return -RT_ENOSYS;
    }
    eeprom_printf("\ni2c_bus %s for EEPROM had found!\n",EEP_I2CBUS_NAME);
    return 0;
}
INIT_APP_EXPORT(bsp_eeprom_init);

static rt_err_t eeprom_iic_read(rt_uint16_t read_addr, rt_uint8_t *buf, rt_uint32_t number)
{   
    rt_size_t result;
    /* Memory device supports only limited number of bytes written in sequence */
    if(number > (EEPROM_ONE_SEQ_WRITE_MAX))
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    /* All written data has to be in the same page */
    if((read_addr/(EEPROM_ONE_SEQ_WRITE_MAX)) != ((read_addr+number-1)/(EEPROM_ONE_SEQ_WRITE_MAX)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }     
#if (EEPROM_ADDRESS_LEN_BYTES == 1)    
    uint8_t *paddr = (uint8_t*)&read_addr; 
#elif (EEPROM_ADDRESS_LEN_BYTES == 2) 
    uint8_t paddr[EEPROM_ADDRESS_LEN_BYTES];
    paddr[0] = (uint8_t)(read_addr>>8); 
    paddr[1] = (uint8_t)(read_addr);
#endif    
    eeprom_printf("EEP read_addr = %d number = %d\r\n",read_addr,number);
    
    result = rt_i2c_master_send(eep_i2c_bus, EEPROM_PHY_7BITS_ADDRESS, RT_I2C_WR | RT_I2C_NO_READ_ACK, paddr, EEPROM_ADDRESS_LEN_BYTES);
    if (result == EEPROM_ADDRESS_LEN_BYTES)
    {
        eeprom_printf("EEP rx write addr ok \r\n");
    }
    else
    {
        eeprom_printf("EEP rx write addr failed ,ERR is: %d \r\n",result);
        return -RT_ERROR;       
    }    
    result = rt_i2c_master_recv(eep_i2c_bus, EEPROM_PHY_7BITS_ADDRESS, RT_I2C_RD, buf, number);
    if (result == number)
    {
        eeprom_printf("EEP read ok \r\n");
        return RT_EOK;       
    }
    else
    {
        eeprom_printf("EEP read failed ,ERR is: %d \r\n",result);
        return -RT_ERROR;       
    }    
}


static rt_err_t eeprom_iic_write(rt_uint16_t write_addr, rt_uint8_t *data, rt_uint32_t number)
{
    rt_size_t result;
    /* Memory device supports only limited number of bytes written in sequence */
    if(number > (EEPROM_ONE_SEQ_WRITE_MAX))
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    /* All written data has to be in the same page */
    if((write_addr/(EEPROM_ONE_SEQ_WRITE_MAX)) != ((write_addr+number-1)/(EEPROM_ONE_SEQ_WRITE_MAX)))
    {
        return NRF_ERROR_INVALID_ADDR;
    }   
    uint8_t buffer[number+EEPROM_ADDRESS_LEN_BYTES]; /* Addr + data */
#if (EEPROM_ADDRESS_LEN_BYTES == 1)    
    buffer[0] = (uint8_t)(write_addr);
#elif (EEPROM_ADDRESS_LEN_BYTES == 2) 
    buffer[0] = (uint8_t)(write_addr>>8);  
    buffer[1] = (uint8_t)(write_addr);
#endif                
    eeprom_printf("EEP write_addr = %d number = %d\r\n",write_addr,number);
    memcpy(&buffer[EEPROM_ADDRESS_LEN_BYTES], data, number);
  
    result = rt_i2c_master_send(eep_i2c_bus, EEPROM_PHY_7BITS_ADDRESS, RT_I2C_WR, buffer, EEPROM_ADDRESS_LEN_BYTES + number);
    if (result == (EEPROM_ADDRESS_LEN_BYTES + number))
    {
        //eeprom_printf("EEP write ok \r\n");
        return RT_EOK;       
    }
    else
    {
        eeprom_printf("EEP write failed ,ERR is: %d \r\n",result);
        return -RT_ERROR;       
    }
}

static void GetEndAddrByPageAligned(uint16_t start, uint16_t end, eep_WrRdPageRange_t* ptr2range)
{
	uint16_t page1, page2;
	page1 = start>>3;
	page2 = end>>3;
	ptr2range->start = start;
	if(page2 > page1)
	{
		 ptr2range->end =  (page1+1)*EEPROM_PAGE_ALGIN;
	}
	else
	{
		 ptr2range->end = end;
	}
}

uint16_t App_Eeprom_Write_Buffer(uint16_t addr, uint8_t const * pdata, uint16_t size)
{
    ret_code_t ret;
    uint16_t wrten_len = 0;
    eep_WrRdPageRange_t range;
    uint16_t ofest_shw = addr, end_shw = addr+size;	
    if(NULL == pdata)
    {
       return 0;
    }
    if(0 == size)
    {
       return 0;
    }
    uint8_t temp_bufer[size];
    memcpy(temp_bufer,pdata,size);
  
    while(ofest_shw<end_shw)
    {
       GetEndAddrByPageAligned(ofest_shw, end_shw, &range);
       ret = eeprom_iic_write(range.start, &temp_bufer[range.start-addr],range.end-range.start);	        
       if(NRF_SUCCESS != ret)
       {
          ofest_shw = addr;
          wrten_len = 0;
          end_shw = addr+size;
          continue;
       }				
       ofest_shw +=( range.end-range.start);
       wrten_len += (range.end-range.start);
    }
    return wrten_len;		
}

uint16_t App_Eeprom_Read_Buffer(uint16_t addr, uint8_t * pdata, uint16_t size)
{
	  ret_code_t ret;	 
	  uint16_t read_len = 0;
      eep_WrRdPageRange_t range;
      uint16_t ofest_shw=addr, end_shw=addr+size;
      if(NULL == pdata)
      {
         return 0;
      }
      if(0 == size)
      {
         return 0;
      }
	  while(ofest_shw<end_shw)
	  {
	     GetEndAddrByPageAligned(ofest_shw, end_shw, &range);
	     ret = eeprom_iic_read(range.start,&pdata[range.start-addr],range.end-range.start);
         rt_thread_mdelay(10); 
         if(NRF_SUCCESS != ret)
         {
             ofest_shw = addr;
             read_len = 0;
             end_shw = addr+size;             
             continue;
         }
	    ofest_shw += range.end-range.start;
	    read_len += range.end-range.start;
	 }
	 return read_len;	
}

#if 1
int i2c_eeprom_test(void)
{
    char write_buf[] = "this is ZJ epprom test!!";
    char read_test[strlen(write_buf)];       
    App_Eeprom_Write_Buffer(0,(uint8_t*)write_buf,strlen(write_buf));
    App_Eeprom_Read_Buffer(0,(uint8_t*)&read_test, strlen(write_buf));
    rt_kprintf("EEP read ok ,data is: %s \r\n",read_test);
}

FINSH_FUNCTION_EXPORT_ALIAS(i2c_eeprom_test, eeprom , i2c_eeprom test);
MSH_CMD_EXPORT(i2c_eeprom_test, i2c_eeprom test);
#endif
