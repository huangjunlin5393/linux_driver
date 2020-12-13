#include "bsp_i2c.h"

//#include "spinlock.h"
#define IIC_PORT     (4)

//static volatile spinlock_t  i2cspinlock[IIC_PORT];

void BspShowIICInfo(void)
{}

void BspReadI2cReg(UINT32 reg)
{}

void BspCpuI2cInit(UINT32 reg)
{
    struct fsl_i2c *dev;
    dev = (struct fsl_i2c *) (g_map_baseaddr+reg);
    unsigned char	dummy;
    out_8 (&dev->cr, (I2C_CR_MSTA));
    out_8 (&dev->cr, (I2C_CR_MEN | I2C_CR_MSTA));
    dummy = in_8(&dev->dr);
    dummy = in_8(&dev->dr);
    if (dummy != 0xff)
    {
        dummy = in_8(&dev->dr);
    }
	
    out_8 (&dev->cr, (I2C_CR_MEN));
    out_8 (&dev->cr, 0x00);
    out_8 (&dev->cr, (I2C_CR_MEN));
}

void BspCpuI2cInitPort(UINT32 port)
{
    switch(port)
    {
	    case P8308_IIC0:
	    BspCpuI2cInit(CONFIG_SYS_I2C1_OFFSET);	
	    break;
	    case P8308_IIC1:
	    BspCpuI2cInit(CONFIG_SYS_I2C2_OFFSET);
	    break;
	    case P8308_IIC2:
	    BspCpuI2cInit(CONFIG_SYS_I2C3_OFFSET);
	    break;
	    case P8308_IIC3:
	    BspCpuI2cInit(CONFIG_SYS_I2C4_OFFSET);
	    break;
	default:
		break;
	}
}

 
int i2c_wait(int write,int port)
{
	unsigned int csr;
	unsigned long long timeval = 0;
	const unsigned long long timeout = 0;
	volatile void *piicsr;
	switch(port)
	{
		case P8308_IIC0:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0xc;
		    break;
		case P8308_IIC1:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0xc;
			break;
		case P8308_IIC2:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0xc;
			break;
		case P8308_IIC3:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0xc;
			break;
		default:
			break;
	}		
	do {
		csr = readb(piicsr);
		if (!(csr & I2C_SR_MIF))
			continue;
	    csr = readb(piicsr);
	    writeb(0x0, piicsr);
		if (csr & I2C_SR_MAL) {
			
			return -1;
		}
		if (!(csr & I2C_SR_MCF))	{
			
			return -1;
		}
		if (write == I2C_WRITE_BIT && (csr & I2C_SR_RXAK)) {
			 
			return -1;
		}
		return 0;
	} while (1);
	return -1;
}

static unsigned int set_i2c_bus_speed(const struct fsl_i2c *dev,unsigned int i2c_clk, unsigned int speed)
{
	unsigned short divider = min(i2c_clk / speed, (unsigned short) -1);
	UCHAR dfsr, fdr ; /* Default if no FDR found */
	/* a, b and dfsr matches identifiers A,B and C respectively in AN2919 */
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(fsl_i2c_speed_map); i++)
		if (fsl_i2c_speed_map[i].divider >= divider) {
			UCHAR fdr;
			LOG_DEBUG("set_i2c_bus_speed\n");
			fdr = fsl_i2c_speed_map[i].fdr;
			speed = i2c_clk / fsl_i2c_speed_map[i].divider;
			//writeb(fdr, &dev->fdr);		/* set bus speed */
			writeb(0x2d, &dev->fdr);//3f-->2d
			break;
		}
	return speed;
}

//i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,port);
#define CONFIG_SYS_I2C_SPEED		400000
SINT32 i2c_init(int speed, int slaveadd,int port)
{
	struct fsl_i2c *dev;
	unsigned int temp;
	volatile void *piiccr;
	//BspCpuI2cInitPort(port);
	switch(port)
	{
        case P8308_IIC0:
	        dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C1_OFFSET);
	        break;
	    case P8308_IIC1:
		    dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C2_OFFSET);
		    break;
	    case P8308_IIC2:
		    dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C3_OFFSET);
		    break;
	    case P8308_IIC3:
		    dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C4_OFFSET);
		    break;
	    default:
		    break;
	}
	writeb(0, &dev->cr);	/* stop I2C controller */
	usleep(5);				/* let it shutdown in peace */
	temp = set_i2c_bus_speed(dev, 100000, speed);
	//writeb(0x3F, &dev->dfsrr);
	writeb(slaveadd<<1 , &dev->adr);/* not write slave address */
	writeb(0x0, &dev->sr);			/* clear status register */
	//writeb(I2C_CR_MEN, &dev->cr); /* start I2C controller */
	writeb(0xb0, &dev->cr);
	return SUCC;
}
 

static int i2c_write_addr (UCHAR dev, UCHAR dir, int rsta,int port)
{
    volatile void *piiccr;
    volatile void *piicdr;
    switch(port)
    {
	case P8308_IIC0:
	    piiccr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0x8;
	    piicdr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0x10;
	    break;
	case P8308_IIC1:
	     	piiccr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0x8;
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0x10;
			break;
	case P8308_IIC2:
		piiccr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0x8;
		piicdr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0x10;
		break;
	case P8308_IIC3:
		piiccr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0x8;
		piicdr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0x10;
		break;
	default:
		break;
	}
	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX| (rsta ? I2C_CR_RSTA : 0),piiccr
);
	writeb((dev << 1) | dir, piicdr);
	if (i2c_wait(I2C_WRITE_BIT,port) < 0)
		return 0;
	return 1;
}

 
static int __i2c_write(UCHAR *data, int length,int port)
{
	int i=0;
	volatile void *piicdr;
	switch(port)
		{
		case P8308_IIC0:
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0x10;
			break;
		case P8308_IIC1:
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0x10;
			break;
		case P8308_IIC2:
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0x10;
			break;
		case P8308_IIC3:
		
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0x10;
			break;
		default:
			break;
		}
	for (i = 0; i < length; i++) 
	{
		writeb(data[i], piicdr);
		if (i2c_wait(I2C_WRITE_BIT,port) < 0)
		{
			LOG_DEBUG("drop!\n");
			break;
		}
	}
	return i;
}
 
static __inline__ int __i2c_read(UCHAR *data, int length,int port)
{
	int i;
	volatile void *piiccr;
	    volatile void *piicdr;
		switch(port)
		{
		case P8308_IIC0:
			piiccr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0x8;
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0x10;
			break;
		case P8308_IIC1:
			piiccr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0x8;
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0x10;
			break;
		case P8308_IIC2:
			piiccr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0x8;
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0x10;
			break;
		case P8308_IIC3:
			piiccr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0x8;
			piicdr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0x10;
			break;
		default:
			break;
		}
	writeb(I2C_CR_MEN | I2C_CR_MSTA | ((length == 1) ? I2C_CR_TXAK : 0),piiccr);
	readb(piicdr);
	for (i = 0; i < length; i++) {
		if (i2c_wait(I2C_READ_BIT,port) < 0)
			break;
		if (i == length - 2)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_TXAK,piiccr);
		if (i == length - 1)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX,piiccr);
		data[i] = readb(piicdr);
	}
	return i;
}

 
SINT32 i2c_read(unsigned char dev, unsigned int addr, int alen, unsigned char *data, int length,int port)
{
    int i = -1; /* signal error */
    volatile void *piiccr;
    switch(port)
    {
        case P8308_IIC0:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0x8;
	     break;
	 case P8308_IIC1:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0x8;
	     break;
	 case P8308_IIC2:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0x8;
	     break;
	 case P8308_IIC3:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0x8;
	     break;
	 default:
	     break;
	}
	//spin_lock((spinlock_t *) &i2cspinlock[port]);
	UCHAR *a = (UCHAR*)&addr;
	if (i2c_wait4bus(port) >= 0 && i2c_write_addr(dev, I2C_WRITE_BIT, 0,port) != 0
	    && __i2c_write(&a[4 - alen], alen,port) == alen)
		i = 0; /* No error so far */
	if (length && i2c_write_addr(dev, I2C_READ_BIT, 1,port) != 0)
		i = __i2c_read(data, length,port);
	    writeb(I2C_CR_MEN, piiccr);
	if (i2c_wait4bus(port)) 
	{
	
	}
	if (i == length)
	{
	    //spin_unlock((spinlock_t *) &i2cspinlock[port]);
	    return 0;
	}
	//spin_unlock((spinlock_t *) &i2cspinlock[port]);
	return -1;
}

 

SINT32 i2c_write(unsigned char dev, unsigned int addr, int alen, unsigned char *data, int length,int port)
{
    int i = -1; /* signal error */
    UCHAR *a = (UCHAR *)&addr;
    volatile void *piiccr;	
    switch(port)
    {
        case P8308_IIC0:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0x8;		
	     break;
	 case P8308_IIC1:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0x8;		
	     break;
	 case P8308_IIC2:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0x8;		
	     break;
	 case P8308_IIC3:
	     piiccr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0x8;	
	     break;
	 default:
	     break;
	}
	//spin_lock((spinlock_t *) &i2cspinlock[port]);
	if (i2c_wait4bus(port) >= 0
	    && i2c_write_addr(dev, I2C_WRITE_BIT, 0,port) != 0
	    && __i2c_write(&a[4 - alen], alen,port) == alen) 
      {
		i = __i2c_write(data, length,port);
	}
	writeb(I2C_CR_MEN, piiccr);
	if (i2c_wait4bus(port)) /* Wait until STOP */
	{
	       
	   LOG_DEBUG("i2c_write: wait4bus timed out\n");
	}
	if (i == length)
	{
	    //spin_unlock((spinlock_t *) &i2cspinlock[port]);
	    return 0;
	}
	//spin_unlock((spinlock_t *) &i2cspinlock[port]);
	return -1;
}


int i2c_set_bus_num(unsigned int bus)
{
#if defined(CONFIG_I2C_MUX)
	if (bus < CONFIG_SYS_MAX_I2C_BUS) {
		i2c_bus_num = bus;
	} else {
		int	ret;

		ret = i2x_mux_select_mux(bus);
		if (ret)
			return ret;
		i2c_bus_num = 0;
	}
	i2c_bus_num_mux = bus;
#else
#ifdef CONFIG_SYS_I2C2_OFFSET
	if (bus > 1) {
#else
	if (bus > 0) {
#endif
		return -1;
	}
	//i2c_bus_num = bus;
#endif
	return 0;
}

int i2c_set_bus_speed(int port,unsigned int speed)
{
	unsigned int i2c_clk = 100;
	struct fsl_i2c *dev;
	switch(port)
	{
	    case P8308_IIC0:
		    dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C1_OFFSET);
		    break;
		case P8308_IIC1:
			dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C2_OFFSET);
			break;
		case P8308_IIC2:
			dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C3_OFFSET);
			break;
		case P8308_IIC3:
			dev = (struct fsl_i2c *) (g_map_baseaddr + CONFIG_SYS_I2C4_OFFSET);
			break;
		default:
			break;
	}
	writeb(0, &dev->cr);		/* stop controller */
	set_i2c_bus_speed(dev, i2c_clk, speed);/*modify by hjl*/
	writeb(I2C_CR_MEN, &dev->cr);	/* start controller */
	return 0;
}

unsigned int i2c_get_bus_num(void)
{
#if defined(CONFIG_I2C_MUX)
	return i2c_bus_num_mux;
#else
	return 0;
#endif
}

unsigned int i2c_get_bus_speed(void)
{
	return i2c_bus_speed[0];
}

 
static int i2c_wait4bus(int port)
{
	volatile void *piicsr;
	int i=0;
	unsigned char itmp=0;
	switch(port)
	{
		case P8308_IIC0:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET+0xc;
			break;
		case P8308_IIC1:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET+0xc;
			break;
		case P8308_IIC2:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET+0xc;
			break;
		case P8308_IIC3:
			piicsr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET+0xc;
			break;
		default:
		    break;
	}		
	while (readb(piicsr) & I2C_SR_MBB) 
	{
		if (i>100000)
		{
		    LOG_DEBUG("exit i2cwaitbus!\n");
			break;
		}
		itmp = readb(piicsr) & I2C_SR_MBB;
		i++;
	}
	return 0;
}

 
UINT32 Bspi2cprobe(UCHAR chip,UINT32 port)
{
    volatile void *piicadr;
    switch(port)
    {
        case P8308_IIC0:
	     piicadr = g_map_baseaddr+CONFIG_SYS_I2C1_OFFSET;	
	     break;
	 case P8308_IIC1:
	     piicadr = g_map_baseaddr+CONFIG_SYS_I2C2_OFFSET;	
	     break;
	 case P8308_IIC2:
	     piicadr = g_map_baseaddr+CONFIG_SYS_I2C3_OFFSET;	
	     break;
	 case P8308_IIC3:
	     piicadr = g_map_baseaddr+CONFIG_SYS_I2C4_OFFSET;
	     break;
	 default:
	     break;
	}	
	if (chip == (readb(piicadr) >> 1))
	{
	    return -1;
	}
	return i2c_read(chip, 0, 0, 0, 0,port);
}

 
void BspDetectI2cDeviceId(UINT32 port)
{
    int i=0;
    for (i = 0; i < 128; i++) 
    {
	    if (Bspi2cprobe(i,port) == 0)
	    {
		    LOG_DEBUG("IIC port %d device id =0x%02x\n", port,i);
		    //break;
	    }
	}
}

 
UINT32 BspP8308I2cRead(UINT32  port,UINT32 uireg)
{
	UINT32	idata=0; 
	if (i2c_read(DEVICEID, uireg>>2, 2, (unsigned char*)&idata, sizeof(idata),port) != 0)
	{
	    LOG_DEBUG ("Error reading the chip.\n");
	}
	LOG_DEBUG ("reading the chip data:0x%x\n", idata);
	return idata;
}

void BspP8308I2cWrite(UINT32 port,UINT32 uireg,UINT32 value)
{
	UINT32 tmp=value;
	if (i2c_write(DEVICEID, uireg>>2, 3, (unsigned char *)&tmp, 4,port) != 0)
	{
	    LOG_DEBUG("Error writing the chip.\n");
	}
}

SINT32 i2c0_read(UINT8 chip_addr, UINT8 inner_addr_len, UINT16 inner_addr,UINT8 * buf, UINT32 buf_len)
{
	return i2c_read(chip_addr, inner_addr, inner_addr_len, buf, buf_len,0);
}

SINT32 i2c1_read(UINT8 chip_addr, UINT8 inner_addr_len, UINT16 inner_addr,UINT8 * buf, UINT32 buf_len)
{
	return i2c_read(chip_addr, inner_addr, inner_addr_len, buf, buf_len,1);
}


SINT32 i2c0_write(UINT8 chip_addr, UINT8 inner_addr_len, UINT16 inner_addr,UINT8 * buf, UINT32 buf_len)
{
	return i2c_write(chip_addr, inner_addr, inner_addr_len, buf, buf_len,0);
}
SINT32 i2c1_write(UINT8 chip_addr, UINT8 inner_addr_len, UINT16 inner_addr,UINT8 * buf, UINT32 buf_len)
{
	return i2c_write(chip_addr, inner_addr, inner_addr_len, buf, buf_len,1);

}
