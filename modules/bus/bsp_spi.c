/******************************************************************************
* CopyRIGHT(C): Xinwei Telecom Technology Inc
* FileName:      bsp_spi.c
* Author:        huangjunlin
* Date：      07-22-2013
* OverView:spi interface for fpga read and write
******************************************************************************/
#include "bsp_spi.h"
#include "bsp_gpio.h"

extern UINT8 *g_map_baseaddr;
extern UINT8 *glbus_map_baseaddr;

void spi_cs_activate(UINT8 csflag)
{
#if 1
	if(csflag==8)
	{
		gpio_write(csflag, 0);
		gpio_write(csflag+1, 1);
	}
	else if(csflag==9)
	{
		gpio_write(csflag, 0);
		gpio_write(csflag-1, 1);
	}
#endif
//	gpio_write(csflag, 0);
	
}

void spi_cs_deactivate(UINT8 csflag)
{
	gpio_write(csflag, 1);
}

void spi_cs_deactivate_all()
{
	spi_cs_deactivate(PPC_FPGA_SPI_CS);
	spi_cs_deactivate(PPC_CPLD_SPI_CS);
}

UINT32 Init_SPI()
{
	  UINT32 *rSICRL 		  =	0;
	  volatile spi8308_t *spi = (spi8308_t*)(g_map_baseaddr+SPIOFFSET);
	  
	  rSICRL = (UINT32*)(g_map_baseaddr+SICRL);  	//system configure reg
  	  *rSICRL &= 0xCFFFFFFF;						/*配置选择SPI引脚功能*/
	  LOG_DEBUG("rSICRL 0x%x\n", *rSICRL);
	  
	  spi_cs_deactivate(PPC_FPGA_SPI_CS);//init the spi
	  spi_cs_deactivate(PPC_CPLD_SPI_CS);
		
	  spi->mode  	= SPI_MODE_REV | SPI_MODE_MS | SPI_MODE_EN|(7 << 20);
	  spi->mode 	|= 0x08000000;
	  spi->mode = (spi->mode & 0xfff0ffff);


	/*SPICLK=133/64*(0+1) = 2.07M */

	  spi->event 	= 0xffffffff;
	  spi->mask  	= 0x00000000;
	  spi->com 	 	= 0;
	  return APP_OK;
}

/**************************************************
* function : spi_send
* argument : *data_in:  读取的数据
             *data_out: 发送的数据
             length:数据长度(以bit为单位)
* return : spi_send 是否成功 0 - 成功 -1 - 失败
**************************************************/
UINT32 flg 	= 0;
UINT32 evt 	= 0;
SINT32 spi_send( UINT8 *data_in, UINT8 *data_out, UINT32 length, int cs)
{
	
	volatile spi8308_t *spi = (spi8308_t*)(g_map_baseaddr+SPIOFFSET);

	UINT32 tmpdout, tmpdin, event,*txbuf;
	int numBlks 	= length / 8 + (length % 8 ? 1 : 0);
	int tm, isRead 	= 0;
	UINT8 charSize 	= 8;
	UINT8 flag 		= 0;

//	LOG("data_out:0x%x:0x%x:0x%x\n", data_out[0],data_out[1], data_out[2]);
   
//	LOG_DEBUG("spi_xfer: dout %08X din %08X bitlen %u\n", *data_out, *data_in, length);

	spi->event = 0xffffffff;	/* Clear all SPI events */

	spi_cs_activate(cs);
	while (numBlks--)
	{
	    if( numBlks == 0 )
	    {
	        spi->com |= 0x400000;
	    }
		tmpdout = (*(UINT32 *)data_out) >> (32 - charSize);
		length -= 8;
		data_out++;

		spi->tx = tmpdout;	/* Write the data out */

	//	printf("spi->tx:0x%x\n", tmpdout);

#if 1
		for (tm = 0, isRead = 0; tm < SPI_TIMEOUT; ++tm) {
			event = spi->event;
			if (event & SPI_EV_NE) {
				tmpdin = spi->rx;
				spi->event |= SPI_EV_NE;
				isRead = 1;

				*(UINT32 *) data_in = (tmpdin << (32 - charSize));
				data_in++;
			}

			if (isRead && (event & SPI_EV_NF))
			{
			    if( 0 == numBlks )
			    {
        	        if(event & (0x80000000 >> 17))
        	        {
           			    //spi->event |= ((0x80000000 >> 17)/*|(0x80000000 >> 23)*/);
           			    spi_cs_deactivate(cs);
        				break;
        	        }
    	        }
    	        else
    	        {
    	            break;
    	        }
		    }
		}
		if (tm >= SPI_TIMEOUT)
		{
//		    LOG_DEBUG("*** spi_xfer: Time out during SPI transfer");
		    return ERROR;
	    }
#endif
//		LOG_DEBUG("*** spi_xfer: transfer ended. Value=%08x\n", tmpdout);
	}
	spi_cs_deactivate(cs);
	return RIGHT;
}

#if 0
int Fpga_test()  
{  
	UINT32 data = 0x10101010;
	UINT32 retdata =0;
	
	retdata	= SPI_Read(data);
	LOG_DEBUG("read data is 0x%x\n",retdata);
	d4(g_map_baseaddr+0x7000,0x100);
	return SUCC;  
}
#endif

void testspi()
{
	//SINT32 spi_send( UINT8 *data_in, UINT8 *data_out, UINT32 length)
	UINT32 data = 0x12345678;
	UINT32 recv;
//	spi_cs_activate(PPC_FPGA_SPI_CS);
	spi_send((UINT8*)&recv, (UINT8*)&data, 32,PPC_FPGA_SPI_CS);
//	spi_cs_deactivate(PPC_FPGA_SPI_CS);

	LOG("recv:0x%8x", recv);
}


inline void lbus_write8(UINT8 addr, UINT8 val)
{
	(*(glbus_map_baseaddr+addr)) = val;
}

inline void lbus_write16(UINT8 addr, UINT16 val)
{
	UINT8 nwrite = (val &&0xff00)>>8;
	(*(glbus_map_baseaddr+addr)) 	= nwrite;

	nwrite=val&&0xff;
	(*(glbus_map_baseaddr+addr+1)) 	= nwrite;
}

//static UINT16 cnt = 1;

void bsp_fpga_write_reg(u16 *regaddr, u16 val)
{
    *regaddr = val;
}

inline void lbus_write(UINT16 regaddr, UINT16 val)
{
//	(*(glbus_map_baseaddr+addr))= val;
//	LOG("lbus_write=%d\n", (*(glbus_map_baseaddr+addr)));
	bsp_fpga_write_reg((glbus_map_baseaddr+regaddr),val);
//	(*(glbus_map_baseaddr+addr)) = val;
}

inline UINT8 lbus_read8(UINT8 addr)
{
	UINT8 val  = (UINT8)(*(glbus_map_baseaddr+addr));
	LOG("lbus_Read valH=0x%x\n", val);
	return val;
}

inline UINT16 lbus_read16(UINT8 addr)
{
	UINT16 ret = 0;
	UINT8 val  = (UINT8)(*(glbus_map_baseaddr+addr));
	ret 	   = val<<8;
	val  	   = (UINT8)(*(glbus_map_baseaddr+addr+1));
	ret 	   = ret | val;
	LOG("lbus_Read ret=0x%x\n", ret);

	return ret;
}

u16 bsp_fpga_read_reg(u16 *u8pAddr)
{
	u16 ret;
	ret = *(u16 *)u8pAddr;
	return ret;
}

inline UINT16 lbus_Read(UINT16 addr)
{
	UINT16 ret = 0;
	ret  = bsp_fpga_read_reg((UINT16*)(glbus_map_baseaddr+addr));
	
	LOG("lbus_Read ret=0x%x\n", ret);

	return ret;
}



