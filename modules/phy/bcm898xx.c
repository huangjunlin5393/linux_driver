/*

 */

#include <linux/module.h>
#include <linux/phy.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>


#define PHY_ID_BCM89820	0x03625cd2

#define BCM898XX_DSPRW  0x15
#define BCM898XX_DSPADDR  0x17
#define BCM898XX_EXPREG00 0x0f00
#define BCM898XX_EXPREG04 0x0f04
#define BCM898XX_EXPREG05 0x0f05
#define BCM898XX_EXPREG06 0x0f06
#define BCM898XX_EXPREG0e 0x0f0e
#define BCM898XX_EXPREG70 0x0f70
#define BCM898XX_EXPREG90 0x0f90
#define BCM898XX_EXPREG91 0x0f91
#define BCM898XX_EXPREG92 0x0f92
//LRT reg
#define BCM898XX_LRECTR 0x00
#define BCM898XX_LRESTA 0x01
#define BCM898XX_LREPHYID1 0x02
#define BCM898XX_LREPHYID2  0x03
#define BCM898XX_LREEXSTA 0x0f
//aux reg
#define BCM898XX_AUXBRR   0x10
#define BCM898XX_AUXPHYEXSTU 0x11
#define BCM898XX_AUXREVCNT   0x12
#define BCM898XX_AUXFCSCNT   0x13
#define BCM898XX_AUXREVNCNT  0x14
//shadow acess 18h
#define BCM898XX_AUXSHADOWA  0x18
//[14:12]
#define BCM898XX_REG18SHA0   0x00
#define BCM898XX_REG18SHA001   0x01
#define BCM898XX_REG18SHA010   0x02
#define BCM898XX_REG18SHA100   0x04
#define BCM898XX_REG18SHA111   0x07

//int 
#define BCM898XX_INSR   0x1a
#define BCM898XX_INM    0x1b

//shadow 1ch
#define BCM898XX_REG1C        0x1c

#define BCM898XX_REG1CSPA1    0x02
#define BCM898XX_REG1CCLKC    0x03
#define BCM898XX_REG1CSPA2    0x04
#define BCM898XX_REG1CLEDS    0x08
#define BCM898XX_REG1CLEDC    0x09
#define BCM898XX_REG1CLEDSEL1 0x0d
#define BCM898XX_REG1CLEDSEL2 0x0e
#define BCM898XX_REG1CGPIO    0x0f
#define BCM898XX_REG1CEXCTL    0x0b

//serdes reg
#define BCM898XX_SERDESMIICTL  0x00
#define BCM898XX_SERDESMIISTA  0x01
#define BCM898XX_SERDESAUTONEO 0x04
#define BCM898XX_SERDESANLPA   0x05

static unsigned phy_speed = SPEED_1000;
module_param(phy_speed, uint, 0644);
static unsigned phy_debug = 1;
module_param(phy_debug, uint, 0644);

#define bcm898xx_debug(phydev) do {\
	    if (phy_debug) {\
			dev_info(&phydev->dev, "%s\n", __func__);\
			bcm898xx_dump(phydev);\
	    }\
}while(0)

static void bcm898xx_dump(const struct phy_device *phydev)
{
	dev_info(&phydev->dev,"state=%d,link=%d,autoneg=%d,speed=%d,pause=%d,a_pause=%d",
			phydev->state,phydev->link,phydev->autoneg,
			phydev->speed,phydev->duplex,phydev->pause,
			phydev->asym_pause);
}

static ssize_t phy_state_show(struct device *dev, struct device_attribute *attr,char *buff)	
{
	struct phy_device *phydev = to_phy_device(dev);
	return sprintf(buff, "0x%.8lx\n",(unsigned long)phydev->phy_id);
}

static int reg;
static ssize_t phy_reg_show(struct device *dev, struct device_attribute *attr,char *buff)	
{
	u16 value=0;
	struct phy_device *phydev = to_phy_device(dev);
	value = phy_read(phydev,reg);
	dev_err(&phydev->dev,"read bcm89820 reg=0x%x value=0x%x\n",reg,value);
	return sprintf(buff, "0x%x\n",value);
}

static ssize_t phy_reg_store(struct device *dev, struct device_attribute *attr,
			    const char *buff, size_t size)
{
	u16 value;
	struct phy_device *phydev = to_phy_device(dev);
	sscanf(buff, "%x", &value);
	phy_write(phydev,reg,value);
	dev_err(&phydev->dev,"write bcm89820 reg=0x%x value=0x%x\n",reg,value);
	return size;
}

static ssize_t phy_reg_addr_store(struct device *dev, struct device_attribute *attr,
			    const char *buff, size_t size)
{
	sscanf(buff, "%x", &reg);
	printk("reg=0x%x",reg);
	return size;
}

static ssize_t phy_reg_addr_show(struct device *dev, struct device_attribute *attr,char *buff)	
{
	return sprintf(buff, "0x%x\n",reg);
}


static DEVICE_ATTR_RO(phy_state);
static DEVICE_ATTR(phy_reg, S_IRUGO | S_IWUSR, phy_reg_show, phy_reg_store);

static DEVICE_ATTR(phy_reg_addr, S_IRUGO | S_IWUSR, phy_reg_addr_show, phy_reg_addr_store);

int bcm898xx_config_aneg(struct phy_device *phydev)
{
	bcm898xx_debug(phydev);
	//return -EINVAL;
	//phydev->speed = SPEED_100;
	return 0;
}

static int bcm898xx_read_status(struct phy_device *phydev)
{
	int ret = 0;
	int err;
	
	ret = phy_read(phydev, BCM898XX_AUXPHYEXSTU);
	if (1 == (ret & (1 << 15)))
	{
	    printk("derek - Auto-negotiation Base Page Mismatch\n");
	}
#if 1
	err = genphy_update_link(phydev);
	if (err)
	{
		printk("derek - genphy_update_link error\n");
		return err;
	}
#else
	ret = phy_read(phydev, BCM898XX_LRESTA);
	if (0 == (ret & (1 << 2)))
	{
	    goto no_link;
	}
#endif
	//if(!phydev->link)
	{
		//phydev->link =1;
		phydev->speed = phy_speed;
		phydev->duplex = DUPLEX_FULL;
		//phydev->autoneg = AUTONEG_DISABLE;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	}
no_link:
	//phydev->link = 0;
	bcm898xx_debug(phydev);
	return ret;
}

static int bcm898xx_aneg_done(struct phy_device *phydev)
{
	bcm898xx_debug(phydev);
	return BMSR_ANEGCOMPLETE;
}

static int bcm898xx_config_init(struct phy_device *phydev)
{
    //static int enter_count = 0;

    //if (!enter_count)
    {
		//init
        printk("bcm898xx_config_init in!!!!!\n");
        //enter_count = 1;
        bcm898xx_debug(phydev);
#if 0 //emc
        //phy_write(phydev,0x00,0x8000);//reset
        //msleep(2);
		//msleep(5);
        phy_write(phydev,0x17,0x0F93);
        phy_write(phydev,0x15,0x107F);//exp93=0x107F

        phy_write(phydev,0x17,0x0F90);
        phy_write(phydev,0x15,0x0001);//exp90=0x01

        phy_write(phydev,0x00,0x3000);
        phy_write(phydev,0x00,0x0200);//slave mode
		//msleep(5);
        //aux
        phy_write(phydev,0x18,0x0C00);

        phy_write(phydev,0x17,0x0F90);
        phy_write(phydev,0x15,0x0000);//exp90=0x00
        phy_write(phydev,0x00,0x0100);

        phy_write(phydev,0x17,0x0001);
        phy_write(phydev,0x15,0x0027);

        phy_write(phydev,0x17,0x000E);
        phy_write(phydev,0x15,0x9B52);

        phy_write(phydev,0x17,0x000F);
        phy_write(phydev,0x15,0xA04D);

        phy_write(phydev,0x17,0x0F90);
        phy_write(phydev,0x15,0x0001);

        phy_write(phydev,0x17,0x0F92);
        phy_write(phydev,0x15,0x9225);

        phy_write(phydev,0x17,0x000A);
        phy_write(phydev,0x15,0x0323);

        phy_write(phydev,0x17,0x0FFD);
        phy_write(phydev,0x15,0x1C3F);

        phy_write(phydev,0x17,0x0FFE);
        phy_write(phydev,0x15,0x1C3F);

        phy_write(phydev,0x17,0x0F99);
        phy_write(phydev,0x15,0x7180);
        phy_write(phydev,0x17,0x0F9A);
        phy_write(phydev,0x15,0x34C0);
#endif
		//phy_write(phydev,0x00,0x208);//master mode
		
		phy_write(phydev,0x00,0x208);//master mode
		msleep(5);
		phy_write(phydev,0x00,0x200);//slave mode
		msleep(5);
		phy_write(phydev,0x00,0x208);//master mode
    }	
	
	//phydev->duplex = DUPLEX_FULL;
	//phydev->speed = SPEED_1000;
	//phydev->supported = SUPPORTED_100baseT_Full;
	//phydev->advertising = SUPPORTED_100baseT_Full;
	//phydev->state = PHY_NOLINK;
	phydev->autoneg = AUTONEG_DISABLE;
	
	return 0;
} 

static int bcm898xx_soft_reset(struct phy_device *phydev)
{
	phy_write(phydev,0x00,0x8000);//reset
	//msleep(5);
	bcm898xx_debug(phydev);
	return 0;
}


static int bcm898xx_match_phy_device(struct phy_device *phydev)
{
	dev_info(&phydev->dev, "phy_id=0x%08x\n", phydev->phy_id);

	if(phydev->phy_id = PHY_ID_BCM89820)
	{
	    printk("derek - phy id bcm89820 match\n");
		phy_speed = SPEED_100;
	}
	else
	{
	    printk("derek - phy id bcm89820 mismatch\n");
	}
	return 1;
}

static int bcm898xx_probe(struct phy_device *phydev)
{
	device_create_file(&phydev->dev, &dev_attr_phy_state);
	device_create_file(&phydev->dev, &dev_attr_phy_reg);
	device_create_file(&phydev->dev, &dev_attr_phy_reg_addr);	
	bcm898xx_debug(phydev);
	return 0;
}

static void bm898xx_remove(struct phy_device *phydev)
{
	return;
}



//reg operation
static void bcm898xx_write_exp_reg(struct phy_device *phydev,u32 reg, u16 value)
{
	phy_write(phydev, BCM898XX_DSPADDR,reg);
    phy_write(phydev, BCM898XX_DSPRW,value);
}

static u16 bcm898xx_read_exp_reg(struct phy_device *phydev,u32 reg)
{
	phy_write(phydev, BCM898XX_DSPADDR,reg);
    return phy_read(phydev, BCM898XX_DSPRW);
}
//u8 reg is the shadow value
static u16 bcm898xx_read_shadow18_reg(struct phy_device *phydev,u8 reg)
{
	u16 reg_select;
	reg_select = (reg_select & 0x7fff);
	reg_select = (reg_select | (reg << 12));
	reg_select = (reg_select | 0x03);
	//which reg to read;
	phy_write(phydev, BCM898XX_AUXSHADOWA,reg_select);
	return phy_read(phydev, BCM898XX_AUXSHADOWA);
}
//u8 reg is the shadow value
static void bcm898xx_write_shadow18_reg(struct phy_device *phydev,u8 reg, u16 value)
{
	u16 write_value;
	//reg [2:0]
	write_value = write_value | reg;
	phy_write(phydev, BCM898XX_AUXSHADOWA, write_value);
}

static void bcm898xx_write_shadow1c_reg(struct phy_device *phydev,u8 reg, u16 value)
{
	u16 write_value;
	//reg [14:10]
	write_value = (write_value | 0x8000);
	write_value = (write_value | (reg <<10));
	phy_write(phydev, BCM898XX_REG1C, write_value);
}

static u16 bcm898xx_read_shadow1c_reg(struct phy_device *phydev,u8 reg)
{
	u16 write_value;
	//reg [14:10]
	write_value = (write_value | 0x7fff);
	write_value = (write_value | (reg <<10));
	
	//which reg to read;
	phy_write(phydev, BCM898XX_REG1C,write_value);
	return phy_read(phydev, BCM898XX_REG1C);
}


static struct phy_driver bcm898xx_driver[] = {
{
	.phy_id		= PHY_ID_BCM89820,
	.phy_id_mask	= 0xffffffff,
	.name		= "Broadcom BCM89820",
	.config_init		= bcm898xx_config_init,
	.match_phy_device = bcm898xx_match_phy_device,
	.probe = bcm898xx_probe,
	.soft_reset = bcm898xx_soft_reset,
	.features    = PHY_GBIT_FEATURES,
	.config_aneg   = bcm898xx_config_aneg,
	.aneg_done     = bcm898xx_aneg_done,
	.read_status = bcm898xx_read_status,
	//.read_status = genphy_read_status,
	.suspend = genphy_suspend,
	.resume = genphy_resume,
	.flags			= 0,
	.remove 	    = bm898xx_remove,
	.driver		= { .owner = THIS_MODULE },
}
};

static int __init bcm898xx_init(void)
{
	return phy_drivers_register(bcm898xx_driver,
		ARRAY_SIZE(bcm898xx_driver));
}
module_init(bcm898xx_init);

static void __exit bcm898xx_exit(void)
{
	phy_drivers_unregister(bcm898xx_driver,
		ARRAY_SIZE(bcm898xx_driver));
}
module_exit(bcm898xx_exit);

MODULE_LICENSE("GPL");
