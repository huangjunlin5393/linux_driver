/******************************************************************************
* CopyRIGHT(C): Xinwei Telecom Technology Inc
* FileName:      main.c
* Author:        huangjunlin
* Date£∫        07-22-2013
* OverView:GPIO interface for signal configure
******************************************************************************/
#include "bsp_gpio.h"
#include <unistd.h>

static UINT32 *rGPDIR =0;   
static UINT32 *rGPDAT =0;

extern UINT8 *g_map_baseaddr;

int Init_GPIO()
{
 	 UINT32 *rSICRH = 	0;
	 
     rSICRH = (UINT32*)(g_map_baseaddr+SICRH);
	 rGPDIR = (UINT32*)(g_map_baseaddr+GPDIR);
	 rGPDAT = (UINT32*)(g_map_baseaddr+GPDAT);
	 
	 LOG_DEBUG("rSICRH 0x%x\n", rSICRH);
	 *rSICRH |= 0xFDB3F100;
	 LOG_DEBUG("*rSICRH: 0x%x\n", *rSICRH);


	 LOG_DEBUG("rGPDIR: 0x%x\n", rGPDIR);
	 LOG_DEBUG("rGPDAT: 0x%x\n", rGPDAT);	 
	 LOG_DEBUG("*rGPDIR: 0x%x\n", *rGPDIR);
	 LOG_DEBUG("*rGPDAT: 0x%x\n", *rGPDAT);
	 return APP_OK;
}
/************************************
pin:the pin index
dir:
	0: ‰»Î
	1: ‰≥ˆ
*************************************/
void gpio_setdir(UINT8 pin, SINT8 dir)
{
	if(dir==0)
		*rGPDIR &= ~(1<<(31-pin));

	else if(dir ==1)
		*rGPDIR |= 1<<(31-pin);
}

UINT32 gpio_read( UINT8 pin)
{
	UINT32 val = 0;
	
	if(pin> 23)
		return 0;

	gpio_setdir(pin, 0);
	val = *rGPDAT;
	
	val =val>>(31-pin);
	val &=0x01;
//	LOG("val:%d",val);
	return val;
	
}

void gpio_write( UINT8 pin, UINT8 value)
{
	if(pin>23||(value!=0&&value!=1))
		return;	

	gpio_setdir(pin, 1);
	
	if(value==1)
		*rGPDAT |= 1<<(31-pin);
	
	else
		*rGPDAT &= ~(1<<(31-pin));
}

#if 0
int testgpio()  
{  
	UINT8 val = 0;
	
	val = gpio_read(20);
	LOG_DEBUG("val = 0x%x\n", val);	

	val = ~val;
	gpio_write(20, 1);
	val = gpio_read(20);
	LOG_DEBUG("val = 0x%x\n", val);

	return SUCC;  
}
#endif

