/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************/
#include <stdio.h>
#include "bsp_types.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "bsp_ethsw_mdio.h"
#include "bsp_ethsw_bcm5389.h"
#include "bsp_gpio.h"
extern UINT8 	*g_map_baseaddr; 

#define MIIMIND_BUSY            		0x00000001
#define MIIMIND_NOTVALID        		0x00000004
#define MII_READ_COMMAND       			0x00000001
#define MIIMCOM_READ_CYCLE				0x00000001

typedef struct bsp_mdio 
{
	u32 miimcfg;		/* MII management configuration reg */
	u32 miimcom;		/* MII management command reg */
	u32 miimadd;		/* MII management address reg */
	u32 miimcon;		/* MII management control reg */
	u32 miimstat;		/* MII management status reg */
	u32 miimind;		/* MII management indication reg */
} bsp_mdio_t;

extern  u8 *g_map_baseaddr;

inline void out_be32(volatile unsigned  *addr, int val)
{
	__asm__ __volatile__("sync; stw%U0%X0 %1,%0" : "=m" (*addr) : "r" (val));
}

inline unsigned in_be32(const volatile unsigned  *addr)
{
	unsigned ret;

	__asm__ __volatile__("sync; lwz%U1%X1 %0,%1;\n"
			     "twi 0,%0,0;\n"
			     "isync" : "=r" (ret) : "m" (*addr));
	return ret;
}

#define		OFFSET		0x24520

s32  eth_bcm54618s_read(u8 u8PhyAddr, u8 u8RegAddr, u16 *pu16Value)
{
	int value;
	int timeout = 1000000;

	bsp_mdio_t *phyregs = (bsp_mdio_t *) (void *)(g_map_baseaddr + OFFSET);

	/* Put the address of the phy, and the register
	 * number into MIIMADD */
	out_be32(&phyregs->miimadd, (u8PhyAddr << 8) | (u8RegAddr & 0x1f));

	/* Clear the command register, and wait */
	out_be32(&phyregs->miimcom, 0);
	asm("sync");

	/* Initiate a read command, and wait */
	out_be32(&phyregs->miimcom, MIIMCOM_READ_CYCLE);
	asm("sync");

	/* Wait for the the indication that the read is done */
	while ((in_be32(&phyregs->miimind) & (MIIMIND_NOTVALID | MIIMIND_BUSY))
			&& timeout--)
		;

	/* Grab the value read from the PHY */
	*pu16Value = in_be32(&phyregs->miimstat);

    return 0;
}

s32 eth_bcm54618s_write(u8 u8PhyAddr, u8 u8RegAddr, u16 u16Value)
{
	int timeout = 1000000;
	unsigned int tempaddr;

	bsp_mdio_t *phyregs = (bsp_mdio_t *) (void *)(g_map_baseaddr + OFFSET);

	//tempaddr = &phyregs->miimcfg;
//	LOG("miimcfg addr = 0x%x\n",tempaddr);

	out_be32(&phyregs->miimadd, (u8PhyAddr << 8) | (u8RegAddr & 0x1f));
	out_be32(&phyregs->miimcon, u16Value);
	asm("sync");
	while ((in_be32(&phyregs->miimind) & MIIMIND_BUSY) && timeout--);
}

void eth_bcm54618s_dump_reg(u8 u8PhyId)
{
    u16 u16RegVal;
    
    (void)eth_bcm54618s_read(u8PhyId, MII_CTRL, &u16RegVal);
    LOG("MII_CTRL:       0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_STAT, &u16RegVal);
    LOG("MII_STAT:       0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_PHY_ID0, &u16RegVal);
    LOG("MII_PHY_ID0:    0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_PHY_ID1, &u16RegVal);
    LOG("MII_PHY_ID1:    0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_ANA, &u16RegVal);
    LOG("MII_ANA:        0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_ANP, &u16RegVal);
    LOG("MII_ANP:        0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_AN_EXP_REG, &u16RegVal);
    LOG("MII_AN_EXP_REG: 0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_GB_CTRL, &u16RegVal);
    LOG("MII_GB_CTRL:    0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_GB_STAT, &u16RegVal);
    LOG("MII_GB_STAT:    0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_ESR, &u16RegVal);
    LOG("MII_ESR:        0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_ECR, &u16RegVal);
    LOG("MII_ECR:        0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x0007);
    (void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
    LOG("AUX_CTRL:       0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x1007);
    (void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
    LOG("AUX_REG:        0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x2007);
    (void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
    LOG("POWER_REG:      0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_write(u8PhyId, MII_AUX_CTRL_SHADOW, 0x7007);
    (void)eth_bcm54618s_read(u8PhyId, MII_AUX_CTRL_SHADOW, &u16RegVal);
    LOG("MISC_CTRL:      0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    (void)eth_bcm54618s_read(u8PhyId, MII_ASSR, &u16RegVal);
    LOG("MII_ASSR:       0x%04x\n", u16RegVal, 0,0,0,0,0);

    (void)eth_bcm54618s_write(u8PhyId, MII_GSR, 0x7c00);
    (void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16RegVal);
    LOG("MII_GSR:      0x%04x\n", u16RegVal, 0,0,0,0,0);


    (void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16RegVal);
    LOG("SECONDARY SERDES CONTROL:      0x%04x\n", u16RegVal, 0,0,0,0,0);
	(void)eth_bcm54618s_write(u8PhyId, MII_GSR, 0x5000);
    (void)eth_bcm54618s_read(u8PhyId, MII_GSR, &u16RegVal);
    LOG("SECONDARY SERDES CONTROL:      0x%04x\n", u16RegVal, 0,0,0,0,0);

    (void)eth_bcm54618s_read(u8PhyId, MII_CTRL, &u16RegVal);
    LOG("SECONDARY SERDES CONTROL:      0x%04x\n", u16RegVal, 0,0,0,0,0);
	(void)eth_bcm54618s_write(u8PhyId, MII_CTRL, 0x3100);
    (void)eth_bcm54618s_read(u8PhyId, MII_CTRL, &u16RegVal);
    LOG("SECONDARY SERDES CONTROL:      0x%04x\n", u16RegVal, 0,0,0,0,0);
    
    return;   
}

/*******************************************************************************
* 函数名称: ethsw_write_reg
* 函数功能: 通过SPI接口写BCM53xx芯片寄存器
* u8ChipId      u8          输入        chipID(single should be 0)
* u8RegPage     u8          输入        page number
* u8RegAddr     u8          输入        地址
* pu8Buf        u8*         输入        写进的数值
* u8Len         u8          输入        写进的数的长度
*******************************************************************************/
s32 ethsw_write_reg(u8 u8RegPage, u8 u8RegAddr, const u8 *pu8Buf, u8 u8Len)
{
//	u16 value = (u16*)pu8Buf;
	u16 *ppu8Buf = (u16*)pu8Buf;
	
	int i =0;
	u16 check;

//	sw_writereg(16, (u8RegPage<<8|1));

	eth_bcm54618s_write(0x1e, 16, (u8RegPage<<8|1));
	
	switch(u8Len)
	{
		case ONE_BYTE_WIDTH:
		{
			u8 value = *pu8Buf;
		//	LOG("in ONE_BYTE_WIDTH value:0x%x\n",value);
			eth_bcm54618s_write(0x1e,24, value);
	//		LOG("in ONE_BYTE_WIDTH:0x%x\n", value);
			break;
		}
		case TWO_BYTE_WIDTH:
		{
			eth_bcm54618s_write(0x1e,24, *(ppu8Buf+0));
		//	LOG("in TWO_BYTE_WIDTH:0x%x\n", *(ppu8Buf+0));
			break;
		}		
		case FOUR_BYTE_WIDTH:
		{
			eth_bcm54618s_write(0x1e,25, *(ppu8Buf+0));
 			eth_bcm54618s_write(0x1e,24, *(ppu8Buf+1));
		
		//	LOG("in FOUR_BYTE_WIDTH:0x%x\n", *(ppu8Buf+0));
		//	LOG("in FOUR_BYTE_WIDTH:0x%x\n", *(ppu8Buf+1));

			break;
		}
		case SIX_BYTE_WIDTH:
		{
			eth_bcm54618s_write(0x1e,26, *(ppu8Buf+0));
			eth_bcm54618s_write(0x1e,25, *(ppu8Buf+1));
			eth_bcm54618s_write(0x1e,24, *(ppu8Buf+2));
			break;
		}
		case EIGHT_BYTE_WIDTH:
		{
			eth_bcm54618s_write(0x1e,27, *(ppu8Buf+0));
			eth_bcm54618s_write(0x1e,26, *(ppu8Buf+1));
			eth_bcm54618s_write(0x1e,25, *(ppu8Buf+2));
			eth_bcm54618s_write(0x1e,24, *(ppu8Buf+3));
			break;
		}
	}
	eth_bcm54618s_write(0x1e,17, u8RegAddr<<8|1);
	
 #if 1
  	eth_bcm54618s_read(0x1e,17, &check);
//	LOG("check:0x%x\n", check);
	while(check&0x3)
	{	
		i++;
		eth_bcm54618s_read(0x1e,17, &check);
		if(i>=65534)
		{
			LOG("ethsw_write_reg timeout\n");
			return E_ETHSW_FAIL;
		}
	}
#endif

	return (E_ETHSW_OK);
}

void bcm_write()
{
	gpio_write(7,1);
	eth_bcm54618s_write(8, 16, (0x00<<8|1));
//	eth_bcm54618s_write(8, 16, 0x3100);
	gpio_write(7,0);
}

/*******************************************************************************
* 函数名称: ethsw_write_reg
* 函数功能: 通过SPI接口写BCM53xx芯片寄存器
* u8ChipId      u8          输入        chipID(single should be 0)
* u8RegPage     u8          输入        page number
* u8RegAddr     u8          输入        地址
* pu8Buf        u8         输入         写进的数值
* u8Len         u8          输入        写进的数的长度
*******************************************************************************/
void testwrite()
{
	int u8RegPage 	= 	0x0;
	int u8RegAddr 	=	0x8;
	u8 pu8Buf 		= 	0x1c;
	u8 u8Len 		= 	1;

	gpio_write(7,1);
	ethsw_write_reg(u8RegPage, u8RegAddr, &pu8Buf, u8Len);
	gpio_write(7,0);
	
}

void testwrite1(int u8RegPage,int u8RegAddr, u8 pu8Buf)
{
	ethsw_write_reg(u8RegPage, u8RegAddr, &pu8Buf, 1);
}


s32 ethsw_read_reg(u8 u8RegPage, u8 u8RegAddr, u8 *pu8Buf, u8 u8Len)
{
	u16 check;
	u32 i =0;
	u16 *pstpu8Buf = (u16*)pu8Buf;
	u16 readval;
	eth_bcm54618s_write(0x1e,16, (u8RegPage<<8|1));
	
//	eth_bcm54618s_read(0x1e,16, &check);
//	printf("check:0x%x\n", check);
	
	eth_bcm54618s_write(0x1e,17, (u8RegAddr<<8|2));
//	check = sw_readreg(17);
//	eth_bcm54618s_read(0x1e,17, &check);
//	LOG("check:0x%x\n", check);

	while(check&0x3)
	{	
		i++;

		eth_bcm54618s_read(0x1e,17, &check);
		if(i>=65534)
		{
			LOG("ethsw_read_reg timeout\n");
			return E_ETHSW_FAIL;
		}
	}
	
	switch(u8Len)
	{
		case ONE_BYTE_WIDTH:
		{
			//*pu8Buf = (sw_readreg(24) &0xff);
			eth_bcm54618s_read(0x1e,24,pstpu8Buf);
		//	LOG("ONE_BYTE_WIDTH1:0x%x\n", *pstpu8Buf);
			*pu8Buf = *pstpu8Buf;
		//	LOG("ONE_BYTE_WIDTH2:0x%x\n", *pu8Buf);
			break;
		}
		case TWO_BYTE_WIDTH:
		{
			eth_bcm54618s_read(0x1e,24,pstpu8Buf);
		//	LOG("TWO_BYTE_WIDTH:0x%x\n", *pstpu8Buf);
//			*pu8Buf = *pbuf;
//			LOG("TWO_BYTE_WIDTH:0x%x\n", *pu8Buf);
			break;
		}
		case FOUR_BYTE_WIDTH:
		{
			eth_bcm54618s_read(0x1e,25,(pstpu8Buf+0));
			eth_bcm54618s_read(0x1e,24,(pstpu8Buf+1));
#if 0
			LOG("TWO_BYTE_WIDTH1:0x%x\n", *(pbuf+0));
			LOG("TWO_BYTE_WIDTH2:0x%x\n", *(pbuf+1));
			eth_bcm54618s_read(0x1e,25,(pstpu8Buf+0));
			eth_bcm54618s_read(0x1e,24,(pstpu8Buf+1));
			LOG("FOUR_BYTE_WIDTH3:0x%x\n", *(pstpu8Buf+0));
			LOG("FOUR_BYTE_WIDTH4:0x%x\n", *(pstpu8Buf+1));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+0));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+1));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+2));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+3));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+4));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+5));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+6));
			LOG("FOUR_BYTE_WIDTH:0x%x\n", *(pu8Buf+7));
#endif
			break;
		}
		case SIX_BYTE_WIDTH:
		{
			eth_bcm54618s_read(0x1e,26,pstpu8Buf);
			eth_bcm54618s_read(0x1e,25,(pstpu8Buf+1));
			eth_bcm54618s_read(0x1e,24,(pstpu8Buf+2));
			
			break;
		}
		case EIGHT_BYTE_WIDTH:
		{
			eth_bcm54618s_read(0x1e,27,pstpu8Buf);
			eth_bcm54618s_read(0x1e,26,(pstpu8Buf+1));
			eth_bcm54618s_read(0x1e,25,(pstpu8Buf+2));
			eth_bcm54618s_read(0x1e,24,(pstpu8Buf+3));
			break;
		}
	}
	return (E_ETHSW_OK);
}

void testread()
{
	int u8RegPage = 0;
	int u8RegAddr =0x8;
	u8 	pu8Buf=0;
	u8 u8Len = 1;
	
	ethsw_read_reg(u8RegPage, u8RegAddr, &pu8Buf, u8Len);

	LOG("pu8Buf:0x%x\n", pu8Buf);
}

/******************************* 源文件结束 ***********************************/
