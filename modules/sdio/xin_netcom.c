#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/pm_runtime.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include "../../host/mediatek/mt6735/mt_sd.h"
#include "xin_netcom.h"

//#define NET_COM_DEBUG

//#######################tty function start####################################

static struct xin_serial 	*xin_table[XIN_TTY_MINORS];	/* initially all NULL */
static struct xin_hid		*g_xinhid = NULL;
static struct tty_driver 	*xin_tty_driver = NULL;
static sdio_irq_handler_t *lte_sdio_eirq_handler;
static void *lte_sdio_eirq_data;
unsigned int varies_hz=0;

struct gpio_rd_req gstGpioRdReq;

struct gpio_rd_req *get_gpio_rd_req(void)
{
	return &gstGpioRdReq;
}

static int xin_get_subtype_by_tty(struct xin_serial *xin)
{
	int i = 0;
	int subtype = SUB_TYPE_INVALID;
	for(i = 0; i < XIN_TTY_MINORS; i++)
	{
		if(xin == xin_table[i])
		{
		    break;
		}
	}

	if(i == XIN_TTY_MINORS)
	{
		return SUB_TYPE_INVALID;
	}

	if(i == 0)
	{
		subtype = SUB_TYPE_AtCommand;
	}
    else if(i == 1)
	{
		subtype = SUB_TYPE_ROM_UPDATE;
	}

	return subtype;
}

static struct xin_serial * xin_get_xin_serial_by_subtype(int subtype)
{
	struct xin_serial * serial = NULL;
	if(subtype == SUB_TYPE_AtCommand)
	{
		serial = xin_table[0]; 
	}
        else if(subtype == SUB_TYPE_ROM_UPDATE)
	{
		serial = xin_table[1]; 
	}
	else //all other type for logs
	{
		serial = xin_table[2];
	}
	
	if(!serial)
	{		
		printk(KERN_ERR "[%s] destination VCOM does not exist\n", __FUNCTION__);
		goto exit;
	}

	if (!serial->tty)
	{
	//	printk(KERN_ERR "[%s] no destination ttyVCOM is not opened\n", __FUNCTION__);

		/* port was not opened */
                serial = NULL;
		goto exit;
	}

exit:
	return serial;
}

static int xin_open(struct tty_struct *tty, struct file *file)
{
	struct xin_serial *xin = tty->driver_data;
	int index;
	int result = 0;

	mutex_lock(&global_mutex);

	index = tty->index;
	printk(KERN_ERR "[%s] open ttyVCOM%d\n", __FUNCTION__, index);
	if (xin != xin_table[index]) 
	{
		printk(KERN_ERR "[%s] open information for ttyVCOM%d is changed!\n", __FUNCTION__, index );
		result = -EINVAL;
		goto Exit;
	}

	//printk(KERN_ERR "[%s] before call open, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
	//printk(KERN_ERR "[%s] index = %d, before call open: port->kref.refcount = %d\n", __FUNCTION__, index, xin->port.kref.refcount);
	result = tty_port_open(&xin->port, tty, file);
	if(result == 0)
	{ 
		printk(KERN_ERR "[%s] open ttyVCOM%d OK\n", __FUNCTION__, index);
	}
	else
	{
		printk(KERN_ERR "[%s] failed to call tty_port_open for ttyVCOM%d\n", __FUNCTION__, index);
	}
   //printk(KERN_ERR "[%s] after call open, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
	//printk(KERN_ERR "[%s] index = %d, after call open: port->kref.refcount = %d\n", __FUNCTION__, index, xin->port.kref.refcount);
	
Exit:
	mutex_unlock(&global_mutex);
	return result;

}


static void xin_close(struct tty_struct *tty, struct file *file)
{
	struct xin_serial *xin = tty->driver_data;
        int index;

	printk(KERN_ERR "[%s] close ttyVCOM%d\n", __FUNCTION__, tty->index);

        index = tty->index;	
	if (xin != xin_table[index]) 
	{
	    printk(KERN_ERR "[%s] close information for ttyVCOM%d is changed!\n", __FUNCTION__, index );
        return;
	}

    //mutex_lock(&global_mutex);
    if(xin == NULL || g_xinhid == NULL)
    {
         //printk(KERN_ERR "[%s] xin is free, xin_tty_driver->kref.refcount = %u\n", __FUNCTION__, xin_tty_driver->kref.refcount);
         return;
    }

    //printk(KERN_ERR "[%s] before call close, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
    //printk(KERN_ERR "[%s] index = %d, before call close: port->kref.refcount = %d\n", __FUNCTION__, tty->index, xin->port.kref.refcount);

    tty_port_close(&xin->port, tty, file);

    //printk(KERN_ERR "[%s] after call close, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
    //printk(KERN_ERR "[%s] index = %d, after call close: port->kref.refcount = %d\n", __FUNCTION__, tty->index, xin->port.kref.refcount);

    //mutex_unlock(&global_mutex); 
}	

static int xin_write(struct tty_struct *tty, const unsigned char *buffer, int incount)
{
	struct xin_serial *xin = tty->driver_data;
	struct xin_hid* xinhid = g_xinhid;
	struct 	pkt_data *ptr_pkt_infor;	
	
	int subtype = SUB_TYPE_INVALID;
	int retval = -EINVAL;
	int copylength = 0;
	USHORT toSendLen = 0;
	u8 *pchar=NULL;
	u16 padSize = 0;	
	int count;
	//int idx=0;

	USB_HDR	*pUSB_HDR;
	padSize = incount&0x3;
	if (padSize)
	{
		padSize = 4 - padSize;
		count = incount + padSize;
	}
	else
	{
		count = incount;
	}
	
//	printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@\n");
//	for(idx=0;idx<incount;idx++)
//		printk("%c:0x%x, ",buffer[idx],buffer[idx]);
//	printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	

	mutex_lock(&global_mutex);
	
//	printk(KERN_INFO "[%s] ttyVCOM%d write count:%d bytes,padSize=%d,incount=%d\n", __FUNCTION__, tty->index, count,padSize,incount);

	if(xinhid == NULL)
	{
		printk(KERN_ERR "[%s] xin hid device is not connected", __FUNCTION__);	
		retval = -ENODEV;
		goto exit;
	}

	if (!xin)
	{	
		printk(KERN_ERR "[%s] ttyVCOM%d doesnot exist", __FUNCTION__, tty->index);
		retval = -ENODEV;
		goto exit;
	}

	if(!count)
	{
		printk(KERN_ERR "[%s] data length is ZERO, so shortcut!", __FUNCTION__);
	    retval = 0;
		goto exit;		
	}

/*
	if (!xin->open_count)
	{
		printk(KERN_ERR "[%s] ttyVCOM%d is not opened", __FUNCTION__, tty->index);

		goto exit;
	}
*/
	subtype = xin_get_subtype_by_tty(xin);
	if(subtype == SUB_TYPE_INVALID)
	{
		printk(KERN_ERR "[%s] cannot get subtype for ttyVCOM%d", __FUNCTION__, tty->index);
		goto exit;
	}
#if 0	
	if(SUB_TYPE_AtCommand==subtype&&incount>=2)
	{
		if((buffer[0]!='a'&&buffer[1]!='t')||(buffer[0]!='A'||buffer[1]!='T'))
		{
			retval=0;
			goto exit;	
		}

	}
	else if(SUB_TYPE_AtCommand==subtype&&incount<2)
	{
		retval=0;
		goto exit;
	}
	printk(KERN_INFO "[%s] ttyVCOM%d write %d bytes,padSize=%d\n", __FUNCTION__, tty->index, incount,padSize);

#endif
	
	memset(&xinhid->write_buffer, 0, sizeof(SDIO_BUF));
	toSendLen = sizeof(xin_hdr)+count+sizeof(struct pkt_data)+sizeof(USB_HDR);

	xinhid->write_buffer.Hdr.magicnum 	= MAGIC_NUMBER;
	xinhid->write_buffer.Hdr.pkt_sumsize = count+sizeof(struct  pkt_data)+sizeof(USB_HDR);
    xinhid->write_buffer.Hdr.pkt_num	=	1;
	ptr_pkt_infor						=	xinhid->write_buffer.Hdr.pkt_infor;
	ptr_pkt_infor->pkt_type				=	MAIN_TYPE_DEBUG;
	ptr_pkt_infor->pkt_subtype			=	subtype;
	ptr_pkt_infor->pkt_size				=	count+sizeof(struct  pkt_data)+sizeof(USB_HDR);
	ptr_pkt_infor->padsize 				= padSize;

	pUSB_HDR=	(USB_HDR *)ptr_pkt_infor->payload;	

	pUSB_HDR->magicNumber 	= 0x55aa;
	pUSB_HDR->length 		= sizeof(USB_HDR) + incount;
	pUSB_HDR->m_type 		= MAIN_TYPE_DEBUG;
    pUSB_HDR->inOutFlag 	= DIRECTION_WRITE;
	pUSB_HDR->sub_type 		= subtype;
	memcpy(ptr_pkt_infor->payload+sizeof(USB_HDR), buffer, incount);
	copylength = toSendLen;
	
	pchar=(u8 *)&xinhid->write_buffer;

	//printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	//for(idx=0;idx<copylength;idx++)
	//	printk("0x%x, ",pchar[idx]);
	//printk("\n@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	sdio_claim_host(xinhid->func);
	retval=sdio_memcpy_toio(xinhid->func, 0, &xinhid->write_buffer, copylength);
	if(retval<0)
	{
		printk("%s:invalid tx address 0x%x len=%d\n\n",__FUNCTION__,pchar[0],copylength);
		sdio_release_host(xinhid->func);
		goto exit;
		
	}
//	udelay(100);							//if we really need
	sdio_release_host(xinhid->func);
	retval = incount;

	printk(KERN_INFO "[%s] ttyVCOM%d write count:%d bytes,padSize=%d,incount=%d\n", __FUNCTION__, tty->index, count,padSize,incount);


exit:
	mutex_unlock(&global_mutex);
	return retval;
}

static int xin_write_room(struct tty_struct *tty)
{
	struct xin_serial *xin = tty->driver_data;
	int room = -EINVAL;

	if (!xin)
		return -ENODEV;

	/* calculate how much room is left in the device */
	room = XIN_BUFFER_SIZE;

	return room;

}

#define RELEVANT_IFLAG(iflag) ((iflag) & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

static void xin_set_termios(struct tty_struct *tty, struct ktermios *old_termios)
{
	unsigned int cflag;

	cflag = tty->termios.c_cflag;//notice huangjunlin

	/* check that they really want us to change something */
	if (old_termios) {//notice huangjunlin
		if ((cflag == old_termios->c_cflag) &&
		    (RELEVANT_IFLAG(tty->termios.c_iflag) == 
		     RELEVANT_IFLAG(old_termios->c_iflag))) {
			printk(KERN_DEBUG " - nothing to change...\n");
			return;
		}
	}

	/* get the byte size */
	switch (cflag & CSIZE) {
		case CS5:
			printk(KERN_DEBUG " - data bits = 5\n");
			break;
		case CS6:
			printk(KERN_DEBUG " - data bits = 6\n");
			break;
		case CS7:
			printk(KERN_DEBUG " - data bits = 7\n");
			break;
		default:
		case CS8:
			printk(KERN_DEBUG " - data bits = 8\n");
			break;
	}
	
	/* determine the parity */
	if (cflag & PARENB)
		if (cflag & PARODD)
			printk(KERN_DEBUG " - parity = odd\n");
		else
			printk(KERN_DEBUG " - parity = even\n");
	else
		printk(KERN_DEBUG " - parity = none\n");

	/* figure out the stop bits requested */
	if (cflag & CSTOPB)
		printk(KERN_DEBUG " - stop bits = 2\n");
	else
		printk(KERN_DEBUG " - stop bits = 1\n");

	/* figure out the hardware flow control settings */
	if (cflag & CRTSCTS)
		printk(KERN_DEBUG " - RTS/CTS is enabled\n");
	else
		printk(KERN_DEBUG " - RTS/CTS is disabled\n");
	
	/* determine software flow control */
	/* if we are implementing XON/XOFF, set the start and 
	 * stop character in the device */
	if (I_IXOFF(tty) || I_IXON(tty)) {
		unsigned char stop_char  = STOP_CHAR(tty);
		unsigned char start_char = START_CHAR(tty);

		/* if we are implementing INBOUND XON/XOFF */
		if (I_IXOFF(tty))
			printk(KERN_DEBUG " - INBOUND XON/XOFF is enabled, "
				"XON = %2x, XOFF = %2x", start_char, stop_char);
		else
			printk(KERN_DEBUG" - INBOUND XON/XOFF is disabled");

		/* if we are implementing OUTBOUND XON/XOFF */
		if (I_IXON(tty))
			printk(KERN_DEBUG" - OUTBOUND XON/XOFF is enabled, "
				"XON = %2x, XOFF = %2x", start_char, stop_char);
		else
			printk(KERN_DEBUG" - OUTBOUND XON/XOFF is disabled");
	}

	/* get the baud rate wanted */
	printk(KERN_DEBUG " - baud rate = %d", tty_get_baud_rate(tty));
}


static int xin_tiocmget(struct tty_struct *tty)
{
	struct xin_serial *xin = tty->driver_data;

	unsigned int result = 0;
	unsigned int msr = xin->msr;
	unsigned int mcr = xin->mcr;

	result = ((mcr & MCR_DTR)  ? TIOCM_DTR  : 0) |	/* DTR is set */
             ((mcr & MCR_RTS)  ? TIOCM_RTS  : 0) |	/* RTS is set */
             ((mcr & MCR_LOOP) ? TIOCM_LOOP : 0) |	/* LOOP is set */
             ((msr & MSR_CTS)  ? TIOCM_CTS  : 0) |	/* CTS is set */
             ((msr & MSR_CD)   ? TIOCM_CAR  : 0) |	/* Carrier detect is set*/
             ((msr & MSR_RI)   ? TIOCM_RI   : 0) |	/* Ring Indicator is set */
             ((msr & MSR_DSR)  ? TIOCM_DSR  : 0);	/* DSR is set */

	return result;
}

static int xin_tiocmset(struct tty_struct *tty,
                         unsigned int set, unsigned int clear)
{
	struct xin_serial *xin = tty->driver_data;
	unsigned int mcr = xin->mcr;

	if (set & TIOCM_RTS)
		mcr |= MCR_RTS;
	if (set & TIOCM_DTR)
		mcr |= MCR_RTS;

	if (clear & TIOCM_RTS)
		mcr &= ~MCR_RTS;
	if (clear & TIOCM_DTR)
		mcr &= ~MCR_RTS;

	/* set the new MCR value in the device */
	xin->mcr = mcr;
	return 0;
}


static void xin_tty_read(unsigned char subtype,unsigned char *transfer_buffer, int datalength)
{
//	struct tty_struct *tty=NULL;
	struct xin_serial *xin=NULL;

	if(SUB_TYPE_AtCommand==subtype)
	{
//		printk("%s:get com at command\n",__FUNCTION__);
	}
	
//	tty = xin_get_tty_by_subtype(subtype);
	xin = xin_get_xin_serial_by_subtype(subtype);
	if(xin != NULL)
	{
//		 tty_insert_flip_string(tty, transfer_buffer, datalength);
//		 tty_flip_buffer_push(tty);
		 tty_insert_flip_string(&xin->port, transfer_buffer, datalength);
		 tty_flip_buffer_push(&xin->port);
	}
//	else
//		printk(KERN_ERR "[%s] invalid tty subtype:%d\n", __FUNCTION__,subtype);

}

static void xin_port_destruct(struct tty_port *port)
{
    printk(KERN_ERR "[%s] just return\n", __FUNCTION__);
}

static const struct tty_port_operations xin_port_ops = {
    .destruct = xin_port_destruct,
};



static void xin_disconnect(struct sdio_func *intf)
{
	
	 struct xin_hid *xinhid = g_xinhid;
	 int i = 0;
	 printk(KERN_ERR "[%s] xin_disconnect\n", __FUNCTION__);

	 if(!xinhid)
	 	return;

	 mutex_lock(&global_mutex);
 
	 for (i = 0; i < XIN_TTY_MINORS; ++i)
		 tty_unregister_device(xin_tty_driver, i);	 
 
//	 sdio_set_drvdata(intf, NULL);
	 if(xinhid) {
 // 	 usb_kill_urb(xinhid->irq);
//		 kfree(xinhid->send_buffer);
		 kfree(xinhid);
		 xinhid = NULL;
	 }
	 g_xinhid = NULL;
	 mutex_unlock(&global_mutex);
 
	 printk(KERN_ERR "[%s] xin_disconnect success\n", __FUNCTION__);
 
}

 static int xin_probe(struct sdio_func *func, const struct sdio_device_id *id)
 {
 
	 struct xin_hid *xinhid;
	 int error = -ENOMEM;
	 int i = 0;
	 struct xin_serial *xin = NULL;

	 if (func->class == SDIO_CLASS_UART) {
		 printk(KERN_INFO"[%s] xin_probe SDIO_CLASS_UART\n", __FUNCTION__);
	 }
 
	 printk(KERN_ERR "@@@@[%s] start funcnum=%d,func->class=%d@@@@@\n", __FUNCTION__,func->num,func->class);
	 
//	 mutex_lock(&global_mutex);
	 if(g_xinhid)
	 {
		 printk(KERN_ERR "[%s] there is already connected xin100 hid device!\n", __FUNCTION__);
		 mutex_unlock(&global_mutex);	 
		 return error;
	 }	 
//	 mutex_unlock(&global_mutex);	 
 
	 xinhid = kzalloc(sizeof(struct xin_hid), GFP_KERNEL);
	 if (!xinhid)
	 {
		 printk(KERN_ERR "[%s] failed to allocate memory for struct xinhid!\n", __FUNCTION__);
		 goto fail1;
	 }
	 
 #if 0
	 if (!(xinhid->send_buffer = kmalloc(WRITE_PACKET_SIZE, GFP_KERNEL)))
	 {
		 printk(KERN_ERR "[%s] failed to allocate memory for urb and buffer!\n", __FUNCTION__);
		 goto fail2;
	 }
 #endif
 
	 xinhid->read_buffer_total_length = 0;
	 xinhid->read_buffer_index = 0;
	 xinhid->func=func;
  
//	 printk(KERN_ERR "[%s] xin_probe before register tty vcom deivce, xin_tty_driver = 0x%08x", __FUNCTION__, (unsigned int)xin_tty_driver);
 
	 for (i = 0; i < XIN_TTY_MINORS; ++i)
	 {
			 xin_table[i] = NULL;
 
			 xin = kmalloc(sizeof(struct xin_serial), GFP_KERNEL);
			 if (!xin)
			 {
				 printk(KERN_ERR "[%s] failed to allocate memory for ttyVCOM%d\n", __FUNCTION__ ,i);
				 error = -ENOMEM;
				 goto fail3;
			 }
 
			tty_port_init(&xin->port);
			xin->port.ops = &xin_port_ops;

			xin->tty = NULL;
			xin->dev = &func->dev;

			xin_table[i] = xin;
			xin->msr = (MSR_CD | MSR_DSR | MSR_CTS);
 
			printk(KERN_ERR "[%s] register ttyVCOM%d\n", __FUNCTION__ ,i);
 
			//printk(KERN_ERR "[%s] before call register, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
			 //printk(KERN_ERR "[%s] index = %d, before call register: port->kref.refcount = %d\n", __FUNCTION__, i, xin->port.kref.refcount);
 
	 		tty_port_register_device(&xin->port, xin_tty_driver, i, xin->dev);
 
			 //printk(KERN_ERR "[%s] after call register, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
			 //printk(KERN_ERR "[%s] index = %d, after call register: port->kref.refcount = %d\n", __FUNCTION__, i, xin->port.kref.refcount);
	 }

	 printk(KERN_ERR "[%s] xin_probe success!\n", __FUNCTION__);
 
	 g_xinhid=xinhid;
	 
	 return 0;
fail3:
	for (i = 0; i < XIN_TTY_MINORS; ++i)
    {
        xin = xin_table[i];
        if(xin)
       	{
        	tty_unregister_device(xin_tty_driver, i);
            tty_port_destroy(&xin->port);
            kfree(xin);
            xin_table[i] = NULL;
        }
    }
fail1:
	 kfree(xinhid);
	 
	 return error;
 }

 static void  xin_exit(void)
 {
	 printk(KERN_ERR "[%s] uninit xin com driver\n", __FUNCTION__);
	 
//	 printk(KERN_ERR "[%s] xin_tty_driver->kref.refcount = %ul\n", __FUNCTION__, xin_tty_driver->kref.refcount); 
	 
	 tty_unregister_driver(xin_tty_driver);
	 put_tty_driver(xin_tty_driver);
	 xin_tty_driver = NULL;
	 
	 printk(KERN_ERR "[%s] uninit xin com driver success\n", __FUNCTION__);
	 
 }

 static int xin_install(struct tty_driver *driver, struct tty_struct* tty)
 {
	 struct xin_serial *xin;
	 int index;
	 int result = 0;
 
	 mutex_lock(&global_mutex);
 
	 /* initialize the pointer in case something fails */
	 tty->driver_data = NULL;
 
	 /* get the serial object associated with this tty pointer */
	 index = tty->index;
	 printk(KERN_ERR "[%s] install ttyVCOM%d\n", __FUNCTION__, index);
	 xin = xin_table[index];
	 if (xin == NULL) 
	 {
		 printk(KERN_ERR "[%s] ttyVCOM%d is not ready!\n", __FUNCTION__, index );
 
		result = -EINVAL;
		 goto Exit;
	 }
		 
	 //printk(KERN_ERR "[%s] before call install, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
	// printk(KERN_ERR "[%s] index = %d, before call install: port->kref.refcount = %d\n", __FUNCTION__, index, xin->port.kref.refcount);

	 result = tty_port_install(&xin->port, driver, tty);
	 if(result)
	 {
	 	printk(KERN_ERR "[%s] failed to call tty_port_install for ttyVCOM%d !\n", __FUNCTION__, index );
	 	goto Exit; 		
	 }
 
	 /* save our structure within the tty structure */
	 tty->driver_data = xin;
	 xin->tty = tty;
	 //printk(KERN_ERR "[%s] after call install, xin_tty_driver->kref.refcount = %d\n", __FUNCTION__, xin_tty_driver->kref.refcount);
	// printk(KERN_ERR "[%s] index = %d, after call install: port->kref.refcount = %d\n", __FUNCTION__, index, xin->port.kref.refcount);
 
	 printk(KERN_ERR "[%s] install ttyVCOM%d OK\n", __FUNCTION__, index);
 
 Exit:
	 mutex_unlock(&global_mutex);
	 return result;
 }


 static void xin_tty_cleanup(struct tty_struct *tty)
 {
	 
	 struct xin_serial *xin = tty->driver_data;
	 int index = 0;
 
	 index = tty->index; 
	 if (xin != xin_table[index]) {
		 printk(KERN_ERR "[%s] xin_tty_cleanup error ttyVCOM%d is changed!\n", __FUNCTION__, index );
			 return;
	 }
 
	 printk(KERN_ERR "[%s] xin_tty_cleanup ttyVCOM%d\n", __FUNCTION__, tty->index);
	  //printk(KERN_ERR "[%s]  index = %d, before tty_port_cleanup: port->kref.refcount = %d\n", __FUNCTION__, tty->index, xin->port.kref.refcount);  
	 
	 //tty_port_put(&xin->port);
	 tty_port_destroy(&xin->port);
	  //printk(KERN_ERR "[%s]  index = %d, after tty_port_cleanup: port->kref.refcount = %d\n", __FUNCTION__, tty->index, xin->port.kref.refcount);
 }
 
 static void xin_tty_hangup(struct tty_struct *tty)
 {
	 
	 struct xin_serial *xin = tty->driver_data;
	 int index = 0;
 
	 index = tty->index; 
	 if (xin != xin_table[index]) {
		 printk(KERN_ERR "[%s] xin_tty_hangup error ttyVCOM%d is changed!\n", __FUNCTION__, index );
			 return;
	 }
 
	 printk(KERN_ERR "[%s] xin_tty_hangup ttyVCOM%d\n", __FUNCTION__, tty->index);
	  //printk(KERN_ERR "[%s]  index = %d, before tty_port_hangup: port->kref.refcount = %d\n", __FUNCTION__, tty->index, xin->port.kref.refcount);   
	 
	 tty_port_hangup(&xin->port);
	  //printk(KERN_ERR "[%s]  index = %d, after tty_port_hangup: port->kref.refcount = %d\n", __FUNCTION__, tty->index, xin->port.kref.refcount);
 }


 static struct tty_operations serial_ops = {
	.install 		= xin_install,
	.open 			= xin_open,
	.close 			= xin_close,
	.cleanup 		= xin_tty_cleanup,
	.hangup 		= xin_tty_hangup,
	.write 			= xin_write,
	.write_room 	= xin_write_room,
	.set_termios 	= xin_set_termios,
	.tiocmget 		= xin_tiocmget,	 
	.tiocmset 		= xin_tiocmset,

 };




 
 int init_tty(void)
 {
 
	 int retval;
 
 
	 xin_tty_driver = alloc_tty_driver(XIN_TTY_MINORS);
	 if (!xin_tty_driver)
	 {
		 printk(KERN_ERR "[%s] failed to alloc tty driver", __FUNCTION__);
		 //usb_deregister(&xin_hid_driver);
		 return -ENOMEM;
	 }
 
	 //printk(KERN_ERR "[%s] xin_tty_driver is 0x%p", __FUNCTION__, (unsigned int)xin_tty_driver);
 
	 /* initialize the tty driver */
	 xin_tty_driver->owner		 = THIS_MODULE;
	 xin_tty_driver->driver_name = "vcom";
	 xin_tty_driver->name		 = "ttyVCOM";
	 xin_tty_driver->major		 = 0;//XIN_TTY_MAJOR;
	 xin_tty_driver->minor_start = 0;
	 xin_tty_driver->type		 = TTY_DRIVER_TYPE_SERIAL;
	 xin_tty_driver->subtype	 = SERIAL_TYPE_NORMAL;
	 xin_tty_driver->flags		 = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	 xin_tty_driver->init_termios			 = tty_std_termios;
	 xin_tty_driver->init_termios.c_cflag	 = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	 xin_tty_driver->init_termios.c_lflag &= ~(ECHO|ECHONL|ICANON|ECHOE);
	 tty_set_operations(xin_tty_driver, &serial_ops);
 
	 /* register the tty driver */
	 retval = tty_register_driver(xin_tty_driver);
	 if (retval) {
		 printk(KERN_ERR "[%s] failed to register xin tty driver0000", __FUNCTION__);
		 put_tty_driver(xin_tty_driver);
		 //usb_deregister(&xin_hid_driver);
		 return retval;
	 }
	 return retval;
 }
//#######################tty function end####################################





//#######################netdevice function start#############################

#define Xin_vendorID 		0x0297
#define Xin_devID    		0x5348
#define XIN1_CLASS			0


static const struct sdio_device_id if_sdio_ids[]={
	{ SDIO_DEVICE(Xin_vendorID,Xin_devID) },
	{ SDIO_DEVICE_CLASS(XIN1_CLASS) },
	{ /* end: all zeroes */			},
};
MODULE_DEVICE_TABLE(sdio, if_sdio_ids);


typedef struct
{
	u8				ucRead;
	u8				ucWrite;
	u16 			usDataCnt;
	spinlock_t		dataunit_lock;
	struct sk_buff *pastrxDataUnit[256];
} RxPktBufferInfo;

RxPktBufferInfo gstRxPktBufferInfo;

bool PushRxDataPkt(struct sk_buff *pUartTxDataUnit,RxPktBufferInfo *gstUartTxDataInfo)
{
	spin_lock(&gstUartTxDataInfo->dataunit_lock);
	if (gstUartTxDataInfo->usDataCnt>= UART_TX_MAX_BUFFER_CNT)	
	{
		spin_unlock(&gstUartTxDataInfo->dataunit_lock);
//		osal_debug("\nERROR:Uart_PushTxDataPkt is full\n");
//		DPrint(DBG_ALL, ("\nPushDataPkt too quick\n"));
		return false;
	}
	gstUartTxDataInfo->pastrxDataUnit[gstUartTxDataInfo->ucWrite] = pUartTxDataUnit;
	gstUartTxDataInfo->ucWrite = RING_INDEX_NEXT(gstUartTxDataInfo->ucWrite + 1, UART_TX_MAX_BUFFER_CNT);
	gstUartTxDataInfo->usDataCnt++;

	spin_unlock(&gstUartTxDataInfo->dataunit_lock);
	return true;
	
}



struct sk_buff *PopRxDataPkt(RxPktBufferInfo *gstUartTxDataInfo)
{
	DataUnit* pUartTxDataUnit = NULL;

	if(gstUartTxDataInfo->usDataCnt <= 0)
	{
//		DPrint(DBG_ALL, ("\nPopDataPkt too quick0\n"));
		return NULL;
	}
	spin_lock(&gstUartTxDataInfo->dataunit_lock);
	if (gstUartTxDataInfo->usDataCnt == 0)	
	{
		spin_unlock(&gstUartTxDataInfo->dataunit_lock);
//		osal_debug("\nERROR:Uart_PopTxDataPkt is null\n");
		return NULL;
	}
	pUartTxDataUnit = gstUartTxDataInfo->pastrxDataUnit[gstUartTxDataInfo->ucRead];	
	gstUartTxDataInfo->ucRead = RING_INDEX_NEXT(gstUartTxDataInfo->ucRead + 1, UART_TX_MAX_BUFFER_CNT);
	gstUartTxDataInfo->usDataCnt--;
	spin_unlock(&gstUartTxDataInfo->dataunit_lock);
	
	return pUartTxDataUnit;	
}





void initDataBuffer(RxDataBufferInfo *gstRxDataBufferInfo, RxDataPktInfor *gstUartTxDataInfo)
{
	u32 loop = 0;

	gstRxDataBufferInfo->ucRead = 0;
	gstRxDataBufferInfo->ucWrite = 0;
	gstRxDataBufferInfo->usFreeCnt = UART_TX_MAX_BUFFER_CNT;

	for (loop = 0;loop < UART_TX_MAX_BUFFER_CNT;loop++)
	{
		gstRxDataBufferInfo->astTxDataUnit[loop].length = UART_TX_MAX_DATA_SEG_LEN;
		gstRxDataBufferInfo->astTxDataUnit[loop].pDataSeg = gstRxDataBufferInfo->aucTxDataBuffer[loop];
		gstRxDataBufferInfo->pastTxDataUnit[loop]=&gstRxDataBufferInfo->astTxDataUnit[loop];
	}
	spin_lock_init(&gstRxDataBufferInfo->databuff_lock);
	gstUartTxDataInfo->ucRead		=0;
	gstUartTxDataInfo->ucWrite		=0;
	gstUartTxDataInfo->usDataCnt	=0;
	spin_lock_init(&gstUartTxDataInfo->dataunit_lock);

	gstRxPktBufferInfo->ucRead		=0;
	gstRxPktBufferInfo->ucWrite		=0;
	gstRxPktBufferInfo->usDataCnt	=0;
	spin_lock_init(&gstRxPktBufferInfo->dataunit_lock);
	
}


DataUnit* PopDataBuffer(RxDataBufferInfo *gstRxDataBufferInfo)
{
	DataUnit* pUartTxDataUnit = NULL;

	spin_lock(&gstRxDataBufferInfo->databuff_lock);
	if (gstRxDataBufferInfo->usFreeCnt <= 0)	
	{	
//		DPrint(DBG_ALL, ("\nPopDataBuffer too quick\n"));
		spin_unlock(&gstRxDataBufferInfo->databuff_lock);
		return NULL;
	}
	pUartTxDataUnit = gstRxDataBufferInfo->pastTxDataUnit[gstRxDataBufferInfo->ucRead];
	gstRxDataBufferInfo->ucRead=RING_INDEX_NEXT(gstRxDataBufferInfo->ucRead + 1, UART_TX_MAX_BUFFER_CNT);
	gstRxDataBufferInfo->usFreeCnt--;
	spin_unlock(&gstRxDataBufferInfo->databuff_lock);
//	printk("\nPopDataBuffer FreeCnt:%u,ucRead:%d\n",gstRxDataBufferInfo->usFreeCnt,gstRxDataBufferInfo->ucRead);

	return pUartTxDataUnit;
}



bool PushDataBuffer(DataUnit* pUartTxDataUnit,RxDataBufferInfo *gstRxDataBufferInfo)
{	
	spin_lock(&gstRxDataBufferInfo->databuff_lock);
	if (gstRxDataBufferInfo->usFreeCnt >= UART_TX_MAX_BUFFER_CNT)	
	{	
		spin_unlock(&gstRxDataBufferInfo->databuff_lock);
//		DPrint(DBG_ALL, ("\nPushDataBuffer too quick\n"));
		return false;
	}
	gstRxDataBufferInfo->pastTxDataUnit[gstRxDataBufferInfo->ucWrite]=pUartTxDataUnit;
	gstRxDataBufferInfo->ucWrite = RING_INDEX_NEXT(gstRxDataBufferInfo->ucWrite + 1, UART_TX_MAX_BUFFER_CNT);
	gstRxDataBufferInfo->usFreeCnt++;
//	printk("\nPushDataBuffer FreeCnt:%u,ucWrite:%d\n",gstRxDataBufferInfo->usFreeCnt,gstRxDataBufferInfo->ucWrite);

	spin_unlock(&gstRxDataBufferInfo->databuff_lock);
	return true;
}

bool PushDataPkt(DataUnit *pUartTxDataUnit,RxDataPktInfor *gstUartTxDataInfo)
{
	spin_lock(&gstUartTxDataInfo->dataunit_lock);
	if (gstUartTxDataInfo->usDataCnt>= UART_TX_MAX_BUFFER_CNT)	
	{
		spin_unlock(&gstUartTxDataInfo->dataunit_lock);
//		osal_debug("\nERROR:Uart_PushTxDataPkt is full\n");
//		DPrint(DBG_ALL, ("\nPushDataPkt too quick\n"));
		return false;
	}
	gstUartTxDataInfo->pastTxDataUnit[gstUartTxDataInfo->ucWrite] = pUartTxDataUnit;
	gstUartTxDataInfo->ucWrite = RING_INDEX_NEXT(gstUartTxDataInfo->ucWrite + 1, UART_TX_MAX_BUFFER_CNT);
	gstUartTxDataInfo->usDataCnt++;

	spin_unlock(&gstUartTxDataInfo->dataunit_lock);
	return true;
	
}

u16 GetDataPktNum(RxDataPktInfor *gstUartTxDataInfo)
{
	return gstUartTxDataInfo->usDataCnt;

}

DataUnit *PopDataPkt(RxDataPktInfor *gstUartTxDataInfo)
{
	DataUnit* pUartTxDataUnit = NULL;

	if(gstUartTxDataInfo->usDataCnt <= 0)
	{
//		DPrint(DBG_ALL, ("\nPopDataPkt too quick0\n"));
		return NULL;
	}
	spin_lock(&gstUartTxDataInfo->dataunit_lock);
	if (gstUartTxDataInfo->usDataCnt == 0)	
	{
		spin_unlock(&gstUartTxDataInfo->dataunit_lock);
//		osal_debug("\nERROR:Uart_PopTxDataPkt is null\n");
		return NULL;
	}
	pUartTxDataUnit = gstUartTxDataInfo->pastTxDataUnit[gstUartTxDataInfo->ucRead];	
	gstUartTxDataInfo->ucRead = RING_INDEX_NEXT(gstUartTxDataInfo->ucRead + 1, UART_TX_MAX_BUFFER_CNT);
	gstUartTxDataInfo->usDataCnt--;
	spin_unlock(&gstUartTxDataInfo->dataunit_lock);
	
	return pUartTxDataUnit;	
}


static int xin_sdio_read_rx_len(struct sdio_func	*func)
{
	int ret;
	int rx_len;
	u8 count_low=0,count_mid=0,count_high=0;

	sdio_readb(func,0x4,&ret);

	count_low=sdio_readb(func,AHB_TRANSFER_COUNT_ADDR1,&ret);
	if (ret)
	{
		printk("%s:count_low:%d\n",__FUNCTION__,count_low);
		return 0;
	}

	count_mid=sdio_readb(func,AHB_TRANSFER_COUNT_ADDR2,&ret);
	if (ret)
	{
		printk("%s:count_mid%d\n",__FUNCTION__,count_mid);
		return 0;
	}

	count_high=sdio_readb(func,AHB_TRANSFER_COUNT_ADDR3,&ret);
	if (ret)
	{
		printk("%s:count_high:%d\n",__FUNCTION__,count_high);
		return 0;
	}
	rx_len=(count_high)<<16|(count_mid)<<8|count_low;
//	printk("%s:rx_len:%d\n",__FUNCTION__,rx_len);
	return rx_len;
}

static int xin_process_rxed_packet(struct xin_net_card *card,u8 *payload,int size)
{
//	int ret;
//	int i=0;
//	int actlen=0;
	struct sk_buff *skb_up;
	struct net_device *netdev=card->priv->netdev;

//	printk("in xin_process_rxed_packet\n");
	
	skb_up=netdev_alloc_skb(netdev,size+2);
	if(!skb_up)
	{
		printk("%s:netdev_alloc_skb_ip_align error\n",__FUNCTION__);
		netdev->stats.rx_dropped++;
		return -ENOMEM;
	}
	skb_reserve(skb_up,2);
	memcpy(skb_up->data,payload,size);
//	memcpy(skb_up->data,netdev->dev_addr, ETH_HLEN);
	skb_put(skb_up,size);
	skb_up->dev			=	netdev;
	skb_up->protocol	=	eth_type_trans(skb_up,netdev);
	skb_up->ip_summed	=	CHECKSUM_UNNECESSARY;
#if 0
	printk("######xin_process_rxed_packet########\n");
	for(i=0;i<size+2;i++)
		printk("0x%x,",skb_up->data[i]);
	printk("\n####xin_process_rxed_packet######\n");
#endif
	netif_rx(skb_up);
	netdev->stats.rx_packets++;
	netdev->stats.rx_bytes+=size;
	
	return 0;
}



static int send_rx_error_notify(struct sdio_func	*func)
{
	u8 txbuff[64];
	int ret=0;
	struct xin_hdr	*pxin_hdr=NULL;
	struct pkt_data *pkt_data=NULL;
	memset(txbuff, 0,256);
	pxin_hdr=(struct xin_hdr *)txbuff;
	pkt_data=(struct pkt_data*)(txbuff+sizeof(struct xin_hdr));


	pxin_hdr->magicnum		=	MAGICNUM;
	pxin_hdr->pkt_num		=	1;
	pxin_hdr->pkt_sumsize	=	sizeof(struct pkt_data);


	pkt_data->pkt_type		=	PKT_TYPE_RX_ERROR;
	pkt_data->pkt_subtype	=	0;
	pkt_data->pkt_size		=	sizeof(struct pkt_data);
	pkt_data->padsize		= 	0;
	
	ret=sdio_memcpy_toio(func, 0, txbuff, pxin_hdr->pkt_sumsize+8);
	if(ret<0)
	{
		printk("%s:ret:%d,invalid tx address 0x%x,0x%x, len=%d\n\n",__FUNCTION__,ret,txbuff[0],txbuff[1],pxin_hdr->pkt_sumsize+8);
	}
	//udelay(3);
	return ret;
}


static void if_sdio_card_to_host_worker(struct work_struct *work)
{
	struct xin_net_card *netcard=NULL;
	struct sk_buff	*skb=NULL;
	int ret,i =0;	
	//unsigned char reg;
	//static int j=0;
	static int sendCnt = 0;
	u32 sts=0,csts=0;
	struct sdio_func *func=NULL;
	struct net_device *pdev=NULL;
	
	struct xin_hdr	*sdio_hdr;
	struct pkt_data *pktdata;

	u16 pkt_num=0,pktsize,pkttype;
	u32 skbbuf_offset=0;
//	u8 subtype=0;
	u32 sumsize=0;

	netcard = container_of(work, struct xin_net_card, rx_pkt_worker);
	func=netcard->func;
	pdev=netcard->priv->netdev;


	while(1)
	{
		skb=PopRxDataPkt(&gstRxPktBufferInfo);
		if (!skb)
		{
			break;
		}		
	
		sdio_hdr=(struct xin_hdr*)skb->data;
	
		if(sdio_hdr->magicnum!=0x5a5b)
		{
			pdev->stats.rx_frame_errors++;
			printk("%s:sdio_hdr->magicnum error\n",__FUNCTION__);
		//	printk("%s size=%d\n",__FUNCTION__,size);
		//	for(i=0;i<20;i++)
		//		printk("0x%x,",skb->data[i]);
		//	printk("\n##################\n");
			goto end;
		}
		
		sumsize = sdio_hdr->pkt_sumsize;
		
	//	if(sumsize+8!=size)
	//	{
	//		printk("%s:WARNNING!!!!:sumsize:%d,size=%d\n",__FUNCTION__,sumsize,size);
	//	}
		
		pkt_num 		=	sdio_hdr->pkt_num;
		skbbuf_offset	=	sizeof(struct xin_hdr);
	
	//	printk("%s pkt_num=0x%x,sumsize=%d\n",__FUNCTION__,pkt_num,sumsize);
	
		if(pkt_num>512)
			pkt_num=512;
		
		for(i=0;i<pkt_num;i++)
		{
			pktdata=(struct pkt_data *)(skb->data+skbbuf_offset);
			pkttype=pktdata->pkt_type;
			pktsize=pktdata->pkt_size;
	
			if(pktsize<=4||pktsize>1520)
			{
				skbbuf_offset+=pktsize;
				pdev->stats.rx_frame_errors++;
				printk("\nWARNNING:pktsize<=4\n");
				continue;
			}

			switch (pkttype)
			{
				case PKT_TYPE_ETHER_PKT:
				{
	//				printk("%s:get eth pkt\n",__FUNCTION__);
					ret = xin_process_rxed_packet(card, pktdata->payload,pktsize-sizeof(struct pkt_data)-pktdata->padsize);
					break;
				}
				case PKT_TYPE_COM_PKT:
				{
	//				xin_tty_read(pktdata->pkt_subtype,pktdata->payload+sizeof(USB_HDR),pktsize-sizeof(struct pkt_data)-sizeof(USB_HDR));
					xin_tty_read(pktdata->pkt_subtype,pktdata->payload,pktsize-sizeof(struct pkt_data)-pktdata->padsize);
					break;
				}
				default:
				{
					printk("%s:invalid pkttype (%d) from card\n\n",__FUNCTION__,pkttype);
					pdev->stats.rx_frame_errors++;
					ret = -EINVAL;
				}
			}
	
			skbbuf_offset+=pktsize;
	
//			cond_resched();
	
		}
		kfree_skb(skb);
	}

}


static int xin_sdio_card_to_host(struct xin_net_card *card)
{
	int ret=0,i;
	int size;// type;
	struct sk_buff *skb;


	struct net_device *netdev=card->priv->netdev;

	gpio_xinlte_apready_set_value(1);
	card->pgpioReq->rd_irq = 0;
	
	size = xin_sdio_read_rx_len(card->func);
	
	//printk("%s size=%d\n",__FUNCTION__,size);

	if(size<8)//one more packet
	{
		printk("%s:xin_sdio_read_rx_len error\n",__FUNCTION__);
		netdev->stats.rx_length_errors++;
		return -EIO;
	}
	
	skb=alloc_skb(size, GFP_ATOMIC);
	if (!skb) {
		printk("%s:alloc_skb error\n",__FUNCTION__);
		netdev->stats.rx_over_errors++;
		return -ENOMEM;
	}
	for(i=0;i<3;i++)
	{
		ret=sdio_memcpy_fromio(card->func, skb->data, 0, size);
		if(ret>=0)
			break;
	}
	if(ret<0)
	{
		printk("%s:sdio_memcpy_fromio error\n",__FUNCTION__);
		netdev->stats.rx_dropped++;
//		we may add send message to device to notify
		send_rx_error_notify(card->func);
		goto end;
	}	
	skb_put(skb, size);
	if(PushRxDataPkt(skb,&gstRxPktBufferInfo))
		queue_work(card->rx_workqueue, &card->rx_pkt_worker); 

	return 0;

}


void xin_wait_sdio_claim_host(struct xin_net_card *Pnetcard)
{
	struct sdio_func *func = Pnetcard->func;
	//struct device *dev = &func->dev;
	struct gpio_rd_req *pgpioReq = Pnetcard->pgpioReq;
	unsigned long delay = msecs_to_jiffies(200);

	while (1)
	{
		sdio_claim_host(func);
		if (pgpioReq->rd_irq == 1 && time_before(jiffies, pgpioReq->jiffies+delay))
		{
			sdio_release_host(func);
			udelay(50);
		}
		else
		{
			if (pgpioReq->rd_irq == 1)
			{
				printk("\n%s:wati device to host irq error\n",__FUNCTION__);
				pgpioReq->rd_irq = 0;
			}
			break;
		}
	}
}

static u16 get_pkt_num(DataUnit* pDataUnit)
{
	struct xin_hdr	*pxin_hdr=(struct xin_hdr *)pDataUnit->pDataSeg;
	return pxin_hdr->pkt_num;
}

//#include "../../core/sdio_ops.h"
void checkctcsandsts(u32 *csts,u32*sts)
{
	void __iomem *base = ((struct msdc_host *)lte_sdio_eirq_data)->base;
	*csts = sdr_read32(SDC_CSTS);
	*sts = sdr_read32(SDC_STS);
}

static void if_sdio_host_to_card_worker(struct work_struct *work)
{
	struct xin_net_card *netcard=NULL;
	DataUnit	*pDataUnit=NULL;
	unsigned long flags;
	int ret;//,i =0;	
	//unsigned char reg;
	//static int j=0;
	static int sendCnt = 0;
	u32 sts=0,csts=0;
	struct sdio_func *func=NULL;
	struct net_device *pdev=NULL;	
	netcard = container_of(work, struct xin_net_card, tx_pkt_worker);
	func=netcard->func;
	pdev=netcard->priv->netdev;
	
//	u8 *ptr;
	
	while(1)
	{
		spin_lock_irqsave(&netcard->tx_lock, flags);
		pDataUnit=PopDataPkt(&netcard->sdioTxDataInfo);
		if (!pDataUnit)
		{
			//start up tx queue
			if(netif_queue_stopped(pdev))
				netif_wake_queue(pdev);
			spin_unlock_irqrestore(&netcard->tx_lock, flags);
			break;
		}		
		spin_unlock_irqrestore(&netcard->tx_lock, flags);


		sdio_claim_host(netcard->func);
		//xin_wait_sdio_claim_host(netcard);
//		udelay(50);
		
//		printk("######if_sdio_host_to_card_worker length=%d########\n",pDataUnit->length);
		//udelay(100);
		/*
		while(1)
		{
			ret=mmc_io_rw_direct(func->card, 0,0,0x03,0, &reg);
			if(ret)
				continue;
			
			if((reg&0x1)==0)
				break;
			i++;

			if(i>2000)
			{
				j++;
				break;
			}
			udelay(10);
		}
		if(j>100)
		{

			printk("%s:xin timeout\n",__FUNCTION__);
			j=0;

		}*/
		
		
		
		ret=sdio_memcpy_toio(func, 0, pDataUnit->pDataSeg, pDataUnit->length<UART_TX_MAX_DATA_SEG_LEN?pDataUnit->length:UART_TX_MAX_DATA_SEG_LEN);
		if(ret<0)
		{
			printk("%s:ret:%d,invalid tx address 0x%x,0x%x, len=%d\n\n",__FUNCTION__,ret,pDataUnit->pDataSeg[0],pDataUnit->pDataSeg[1],pDataUnit->length);
			pdev->stats.tx_dropped+=get_pkt_num(pDataUnit);
		}

		checkctcsandsts(&csts,&sts);
		if ((sendCnt&0xff) == 0)
		{
			printk("sdio intsts  0x%x.0x%x\n",csts,sts); 
		}
		sendCnt++;
		if (sts&0x1)
		{
			printk("sdio error0:intsts  0x%x.0x%x\n",csts,sts); 
			udelay(100);	
			checkctcsandsts(&csts,&sts);
			printk("sdio error1:intsts  0x%x.0x%x\n",csts,sts); 
		}
		
		udelay(100);
		//printk("intsts  0x%x\n",sdr_read32(MSDC_INT)); 
		sdio_release_host(netcard->func);
		//udelay(200);
	
	//	printk("\n@@@@@@@@%s len=%d@@@@@@@@@\n",__FUNCTION__,pDataUnit->length);
		
//		ptr=pDataUnit->pDataSeg;
//		for(i=0;i<10;i++)
//			printk("0x%x,",pDataUnit->pDataSeg[i]);
			
		spin_lock_irqsave(&netcard->tx_lock, flags);
//		printk("\n@\n");
		if(false==PushDataBuffer(pDataUnit,&netcard->RxDataBufferInfo))
		{
			printk("\n%s:PushDataBuffer failed\n",__FUNCTION__);
		}
		spin_unlock_irqrestore(&netcard->tx_lock, flags);

	}
}


static void xin_sdio_interrupt(struct sdio_func *func)
{
	int ret;
	struct xin_net_card *netcard;
	
	//printk("%s func->num=%d\n",__FUNCTION__,func->num);
	
	netcard = sdio_get_drvdata(func);

	ret = xin_sdio_card_to_host(netcard);
}

static void tx_timer_handle(unsigned long arg)
{
	//unsigned long flags;
	struct xin_net_card *netcard=(struct xin_net_card *)arg;
//	printk("##########%s 0000#############\n",__FUNCTION__);
	if(!netcard)
		return;
//	spin_lock_irqsave(&netcard->tx_lock, flags);
	spin_lock(&netcard->tx_lock);

	if(netcard->current_data_unit)
	{
		if(PushDataPkt(netcard->current_data_unit,&netcard->sdioTxDataInfo))
		{
			netcard->current_data_unit=NULL;
			queue_work(netcard->tx_workqueue, &netcard->tx_pkt_worker);
		}
		else
		{
			printk("%s PushDataPkt failed\n",__FUNCTION__);
		}
	}
	else
	{
		queue_work(netcard->tx_workqueue, &netcard->tx_pkt_worker);
	}
//	spin_unlock_irqrestore(&netcard->tx_lock, flags);
	spin_unlock(&netcard->tx_lock);

}

void card_infor(RxDataBufferInfo *gstRxDataBufferInfo,int idx)
{
	printk("ucRead:%d\n",gstRxDataBufferInfo->ucRead);
	printk("ucWrite:%d\n",gstRxDataBufferInfo->ucWrite);
	printk("gstRxDataBufferInfo->usFreeCnt:%u,%d\n",gstRxDataBufferInfo->usFreeCnt,idx);
}



int xin_init_card(struct xin_net_card *netcard)
{
	struct sdio_func *func=netcard->func;
	int ret=0;
	spin_lock_init(&netcard->tx_lock);
	spin_lock_init(&netcard->rx_lock);
	netcard->tx_workqueue = create_singlethread_workqueue("xin_sdio_tx_workqueue");
	INIT_WORK(&netcard->tx_pkt_worker, if_sdio_host_to_card_worker);

	netcard->rx_workqueue = create_singlethread_workqueue("xin_sdio_rx_workqueue");
	INIT_WORK(&netcard->rx_pkt_worker, if_sdio_card_to_host_worker);




	initDataBuffer(&netcard->RxDataBufferInfo,&netcard->sdioTxDataInfo);
	netcard->current_data_unit=NULL;
//	card_infor(&netcard->RxDataBufferInfo,0);
	
	init_timer(&netcard->mytimer);
//	card_infor(&netcard->RxDataBufferInfo,10);
	
	sdio_claim_host(func);
	ret = sdio_enable_func(func);
	if (ret)
		goto release;

	ret=sdio_claim_irq(func,xin_sdio_interrupt);
	if (ret<0)
		goto disable;
//	card_infor(&netcard->RxDataBufferInfo,11);
	
	sdio_release_host(func);
	
//	card_infor(&netcard->RxDataBufferInfo,12);

	return 0;
	
disable:
	sdio_disable_func(func);
release:
	sdio_release_host(func);
	return ret;
}


static int xinnet_dev_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

static int xinnet_eth_stop(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

void format_fill_head(DataUnit* pDataUnit, u8 *buf, u16 inlen)
{
	struct xin_hdr	*pxin_hdr=NULL;
	struct pkt_data *pkt_data=NULL;
	u32 pktsize;

	u16 padSize = 0;	
	u16 len;

	padSize = inlen&0x3;
	if (padSize)
	{
		padSize = 4 - padSize;
		len = inlen + padSize;
	}
	else
	{
		len = inlen;
	}


	
	pktsize=len+sizeof(struct pkt_data);
	
	pxin_hdr=(struct xin_hdr*)pDataUnit->pDataSeg;

	pxin_hdr->magicnum		=	MAGICNUM;
	pxin_hdr->pkt_num		=	1;
	pxin_hdr->pkt_sumsize	=	pktsize;
	
	pkt_data=(struct pkt_data *)(pDataUnit->pDataSeg+sizeof(struct xin_hdr));
	pkt_data->pkt_type		=	PKT_TYPE_ETHER_PKT;
	pkt_data->pkt_subtype	=	0;
	pkt_data->pkt_size		=	pktsize;//len+sizeof(struct pkt_data);
	pkt_data->padsize		= padSize;

	memcpy(pkt_data->payload, buf,inlen);
	pDataUnit->length=pxin_hdr->pkt_sumsize+sizeof(struct xin_hdr);
//	printk("##########%s dddddd#############\n",__FUNCTION__);
}

void format_add_pkt(DataUnit* pDataUnit, u8 *buf, u16 inlen)
{
	struct xin_hdr	*pxin_hdr=NULL;
	struct pkt_data *pkt_data=NULL;


	u16 padSize = 0;	
	u16 len;
	
	padSize = inlen&0x3;
	if (padSize)
	{
		padSize = 4 - padSize;
		len = inlen + padSize;
	}
	else
	{
		len = inlen;
	}

	if(pDataUnit->length+len+8<=UART_TX_MAX_DATA_SEG_LEN)
	{
		pxin_hdr=(struct xin_hdr *)pDataUnit->pDataSeg;
		pkt_data=(struct pkt_data*)(pDataUnit->pDataSeg+sizeof(struct xin_hdr)+pxin_hdr->pkt_sumsize);
		
		pkt_data->pkt_type		=PKT_TYPE_ETHER_PKT;
		pkt_data->pkt_subtype	=0;
		pkt_data->pkt_size		=len+sizeof(struct pkt_data);
		pkt_data->padsize		= padSize;
		
		memcpy(pkt_data->payload, buf,inlen);
		
		pDataUnit->length+=pkt_data->pkt_size;
		pxin_hdr->pkt_sumsize+=pkt_data->pkt_size;
		pxin_hdr->pkt_num++;
//		if(pxin_hdr->pkt_num>3)
//			printk("%s pkt num:%d\n",__FUNCTION__,pxin_hdr->pkt_num);
			
	}
	else
	{
		printk("%s discard pkt\n",__FUNCTION__);
	}
}




static void xin_sdio_tx_timeout_start(struct xin_net_card *netcard)
{
//	printk("##########%s 0000#############\n",__FUNCTION__);
	if (timer_pending(&netcard->mytimer) == 0) 
	{
		netcard->mytimer.function = &tx_timer_handle;
		netcard->mytimer.data = (unsigned long)netcard;
		netcard->mytimer.expires = jiffies + varies_hz;
		add_timer(&netcard->mytimer);
	}
}

#define MAX_PKT_LIMITE		8
static int xin_sdio_host_to_card(struct xin_net_card *netcard,u8 *buf, u16 len)
{
	int ret=0;
//	u16 size;
//	unsigned long flags;
	struct net_device *dev=netcard->priv->netdev;
	int num=0;
		
//	spin_lock_irqsave(&netcard->tx_lock, flags);
	spin_lock(&netcard->tx_lock);

	if(netcard->current_data_unit==NULL)
	{
		netcard->current_data_unit=PopDataBuffer(&netcard->RxDataBufferInfo);
		if(netcard->current_data_unit)
		{
//			printk("##########%s len=%d#############\n",__FUNCTION__,len);
			format_fill_head(netcard->current_data_unit, buf, len);
			xin_sdio_tx_timeout_start(netcard);
		}
		else
		{
			//close the tx queue
			
//			if(!netif_queue_stopped(dev))
//				netif_stop_queue(dev);
//			printk("%s PopDataBuffer failed0\n",__FUNCTION__);
			dev->stats.tx_dropped++;
			ret= -ENOSPC;
		}
	}
	else
	{
		if(netcard->current_data_unit->length+len+12>=UART_TX_MAX_DATA_SEG_LEN)	
		{

			if(false==PushDataPkt(netcard->current_data_unit,&netcard->sdioTxDataInfo))
			{
				printk("%s PushDataPkt failed\n",__FUNCTION__);
				dev->stats.tx_dropped+=get_pkt_num(netcard->current_data_unit);
			}

			netcard->current_data_unit=PopDataBuffer(&netcard->RxDataBufferInfo);
			if(netcard->current_data_unit)
			{
				format_fill_head(netcard->current_data_unit, buf, len);
				xin_sdio_tx_timeout_start(netcard);
			}
			else
			{
				//close the tx queue
	//		if(!netif_queue_stopped(dev))
	//			netif_stop_queue(dev);
	//			printk("%s PopDataBuffer failed1\n",__FUNCTION__);
				dev->stats.tx_dropped++;
				ret= -ENOSPC;
			}
		}
		else if(netcard->current_data_unit)
		{
//			printk("##########%s 3333#############\n",__FUNCTION__);
			format_add_pkt(netcard->current_data_unit,buf, len);
		}
	}

	num=GetDataPktNum(&netcard->sdioTxDataInfo);
//	printk("\nGetDataPktNum000:%d\n",num);
	
	spin_unlock(&netcard->tx_lock);

	if(num>=MAX_PKT_LIMITE)
		queue_work(netcard->tx_workqueue, &netcard->tx_pkt_worker);	
	return ret;
}

static int xinnet_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int status=0;
	u8 *payload 				=	skb->data;
	u16 len	     				=	skb->len;
	struct xinnet_priv *netpriv	=	netdev_priv(dev);
	struct sdio_func *func		=	netpriv->func;
	struct xin_net_card	*netcard	=	NULL;
	netcard = sdio_get_drvdata(func);
				
//	printk("##########%s start to tx packet#############\n",__FUNCTION__);

	status=xin_sdio_host_to_card(netcard,payload,len);
	
//	printk("##########%s end to tx packet#############\n",__FUNCTION__);

	dev->stats.tx_bytes+=skb->len;
	dev->stats.tx_packets++;
	dev_kfree_skb(skb);
	return status;
}

int xinnet_set_mac_address(struct net_device *dev, void *addr)
{

	struct sockaddr *phwaddr = addr;
	if(is_valid_ether_addr(phwaddr->sa_data))
		memcpy(dev->dev_addr, phwaddr->sa_data, ETH_ALEN);
	else
		return -EADDRNOTAVAIL;
	return 0;
}


static struct net_device_stats *xinnet_get_stats(struct net_device *dev)
{
	return &dev->stats;
}

void	xinnet_tx_timeout(struct net_device *dev)
{
	printk("@@@@@@@@@@@Enter %s @@@@@@@@@@@\n",__FUNCTION__);

	dev->trans_start = jiffies; /* prevent tx timeout */
	dev->stats.tx_errors++;
	if(netif_queue_stopped(dev))
		netif_wake_queue(dev);
}


extern int eth_validate_addr(struct net_device *dev);
extern int eth_change_mtu(struct net_device *dev, int new_mtu);

static const struct net_device_ops xinet_netdev_ops = {
	.ndo_open 				= xinnet_dev_open,
	.ndo_stop				= xinnet_eth_stop,
	.ndo_start_xmit			= xinnet_hard_start_xmit,
	.ndo_get_stats			= xinnet_get_stats,
	.ndo_set_mac_address	= xinnet_set_mac_address,
//	.ndo_set_rx_mode		= xinnet_set_multicast_list,
	.ndo_tx_timeout			= xinnet_tx_timeout,
	.ndo_change_mtu			= eth_change_mtu,
	.ndo_validate_addr		= eth_validate_addr,
};

int xin_remove_card(struct xin_net_card *netcard);


void lte_sdio_request_eirq(sdio_irq_handler_t irq_handler, void *data)
{
	printk("[lte] request interrupt from %ps\n",__builtin_return_address(0));
	/*
	ret = request_irq(lte_sdio_eirq_num, lte_sdio_eirq_handler_stub, IRQF_TRIGGER_LOW, "lte_CCCI", NULL);
	disable_irq(lte_sdio_eirq_num);
	*/
	lte_sdio_eirq_handler = irq_handler;
	lte_sdio_eirq_data    = data;
}
EXPORT_SYMBOL(lte_sdio_request_eirq);

static int xin_gpio_irq_func(void)
{	
	#if 1
	if (lte_sdio_eirq_handler)
		lte_sdio_eirq_handler(lte_sdio_eirq_data);
	#else
	struct gpio_rd_req *pGpioReq = get_gpio_rd_req();
	//printk(" rd_irq %d\n",pGpioReq->rd_irq);	
	sdio_claim_host(pGpioReq->pxin_net_card->func);

	if (pGpioReq->rd_irq == 1)
	{
		if (lte_sdio_eirq_handler)
		lte_sdio_eirq_handler(lte_sdio_eirq_data);
	}
	else
	{
		pGpioReq->rd_irq = 1;
		pGpioReq->jiffies = jiffies;
		
		gpio_xinlte_apready_set_value(0);
	}	
	
	sdio_release_host(pGpioReq->pxin_net_card->func);
	#endif

	return 0;
}

static int xin_sdio_probe(struct sdio_func *func,const struct sdio_device_id *id)
{
	struct xinnet_priv 	*netpriv;
	struct net_device 	*netdev;
	struct xin_net_card	*netcard;
	unsigned char macaddr[ETH_ALEN];
	int ret=0;
	struct mmc_host *host;
	unsigned int clock=0;
	struct mmc_card		*card;
//	int err;
#ifdef NET_COM_DEBUG
	unsigned max_blocks;

	printk("%s func->num=%d,class=%d\n",__FUNCTION__,func->num,func->class);
	printk("%s deviceid=0x%x,vendor=0x%x\n",__FUNCTION__,func->device,func->vendor);

	printk("%s max_blksize=%d\n",__FUNCTION__,func->max_blksize);
	printk("%s cur_blksize=%d\n",__FUNCTION__,func->cur_blksize);

	printk("%s device id=%d\n",__FUNCTION__,func->device);

	printk("%s num_info=%d\n",__FUNCTION__,func->num_info);
	printk("%s multi_block=%d\n",__FUNCTION__,func->card->cccr.multi_block);

	printk("%s num_info=%d\n",__FUNCTION__,func->num_info);
	

	max_blocks = min(func->card->host->max_blk_count,func->card->host->max_seg_size / func->cur_blksize);
	max_blocks = min(max_blocks, 511u);

	printk("%s max_blk_count=%d\n",__FUNCTION__,func->card->host->max_blk_count);

	printk("%s max_seg_size=%d\n",__FUNCTION__,func->card->host->max_seg_size);
	printk("%s max_blocks=%d\n",__FUNCTION__,max_blocks);
#endif
	host=func->card->host;
	clock=host->ios.clock;
	card=func->card;
	printk("\n%s host clock=%u,host->f_max=%u\n",__FUNCTION__,clock,host->f_max);
	printk("\n%s max_dtr=%u\n",__FUNCTION__,card->cis.max_dtr);



	/*we must init tty before netdevie*/	
	ret=xin_probe(func,id);
	if(ret)
	{
		printk("%s xin_probe error\n",__FUNCTION__);
		return ret;
	}

	netcard = kzalloc(sizeof(struct xin_net_card), GFP_KERNEL);
	if (!netcard)
	{
		printk("%s kzalloc failed\n",__FUNCTION__);
		return -ENOMEM;
	}

	netcard->pgpioReq = get_gpio_rd_req();
	netcard->pgpioReq->pxin_net_card = netcard;
	netcard->pgpioReq->rd_irq = 0;

	netcard->func = func;
	ret=xin_init_card(netcard);
	if(ret)
	{
		printk("%s init card error\n",__FUNCTION__);
		return -ENOMEM;

	}
	netdev = alloc_etherdev(sizeof(struct xinnet_priv));
	if (!netdev) {
		printk("%s no memory for network device instance\n",__FUNCTION__);
		goto init_error;
	}
	sdio_set_drvdata(func, netcard);
	SET_NETDEV_DEV(netdev, &func->dev);

 	netdev->netdev_ops 		= &xinet_netdev_ops;
//	netdev->watchdog_timeo 	= 20*HZ;
//	netdev->ethtool_ops 	= &lbs_ethtool_ops;

	random_ether_addr(macaddr);
	memcpy(netdev->dev_addr,macaddr,sizeof(macaddr));

	netdev->flags 	|= IFF_NOARP;
	strcpy(netdev->name,"xinnet%d");
	
	netpriv			=	netdev_priv(netdev);
	netpriv->dev 	= 	&func->dev;
	netpriv->func 	= 	func;
	netpriv->netdev = 	netdev;
//	netpriv->hw_host_to_card = if_sdio_host_to_card;

	netcard->priv=netpriv;
//	card_infor(&netcard->RxDataBufferInfo,5);

	ret=register_netdev(netdev);
	if(ret)
	{
		printk("%s register_netdev error\n",__FUNCTION__);
		goto register_error;
	}

	netdev->dev_addr[0] = 0x02;
	netdev->dev_addr[1] = 0x50;   
	netdev->dev_addr[2] = 0xF2;   
	netdev->dev_addr[3] = 0x00;
	netdev->dev_addr[4] = 0x01;
	netdev->dev_addr[5] = 0x01;
	card_infor(&netcard->RxDataBufferInfo,6);
	
 	netif_device_attach(netdev);//netif_start_queue(dev)
 	
	printk("[%s] probe end, HZ=%d\n",__FUNCTION__,HZ);

	/* */

	gpio_xinlte_register_callback(xin_gpio_irq_func);
	
 	return 0;
	
register_error:
	free_netdev(netdev);

init_error:
	xin_remove_card(netcard);
	
	return ret;
}


void xin_stop_card(struct xinnet_priv *priv)
{
	struct net_device *netdev;

	if (!priv)
		return;
	netdev = priv->netdev;
	printk("[%s] flag1\n",__FUNCTION__);

	netif_stop_queue(netdev);//netif_device_detach(dev)
	netif_carrier_off(netdev);
	unregister_netdev(netdev);
	printk("[%s] flag2\n",__FUNCTION__);

	free_netdev(netdev);
	
}
int xin_remove_card(struct xin_net_card *netcard)
{
	del_timer_sync(&netcard->mytimer);
//	kfree(netcard->tx_buffer);
	printk("[%s] flag3\n",__FUNCTION__);

	destroy_workqueue(netcard->tx_workqueue);
	netcard->tx_workqueue=NULL;
	kfree(netcard);	
	printk("[%s] flag6\n",__FUNCTION__);

	return 0;
}

void xin_disable_func(struct sdio_func *func)
{
	sdio_claim_host(func);
	printk("[%s] flag4\n",__FUNCTION__);

	sdio_release_irq(func);
	sdio_disable_func(func);
	sdio_release_host(func);
}

static void xin_sdio_remove(struct sdio_func *func)
{
	struct xinnet_priv *netpriv=NULL;
	struct xin_net_card	*netcard=NULL;
//	int ret=0;

	xin_disconnect(func);

	netcard = sdio_get_drvdata(func);
	netpriv=netcard->priv;
	xin_disable_func(func);
	xin_stop_card(netpriv);
	printk("[%s] flag0\n",__FUNCTION__);
	xin_remove_card(netcard);
	
}


static struct sdio_driver xin_sdio_net_driver = 
{
	.name		= "xinnet_sdio",
	.id_table	= if_sdio_ids,
	.probe		= xin_sdio_probe,
	.remove		= xin_sdio_remove,
};


static int __init xin_sdio_init_module(void)
{
	int ret = 0;
	printk(KERN_INFO "huangjl libertas_sdio: enter SDIO driver,HZ=%d\n",HZ);
	if(HZ<=250)
		varies_hz=1;
	else if(HZ>250&&HZ<=500)
		varies_hz=2;
	else if(HZ>500&&HZ<=1000)
		varies_hz=5;
	else
		varies_hz=5;

		
	ret=init_tty();
	if(ret)
	{
		printk(KERN_INFO "init tty error\n");
		return ret;
	}

	ret = sdio_register_driver(&xin_sdio_net_driver);
	if(ret)
	{
		printk(KERN_ERR "[%s] failed to register xin hid driver1111", __FUNCTION__);
		put_tty_driver(xin_tty_driver);
		tty_unregister_driver(xin_tty_driver);
		return ret;
	}
	return ret;
}


static void __exit xin_sdio_exit_module(void)
{
	printk(KERN_INFO "libertas_sdio: exit SDIO driver\n");
	sdio_unregister_driver(&xin_sdio_net_driver);
	xin_exit();
}

module_init(xin_sdio_init_module);
module_exit(xin_sdio_exit_module);

MODULE_DESCRIPTION("Xincomm SDIO Net and com Driver");
MODULE_AUTHOR("xincom huangjl");
MODULE_LICENSE("GPL");



