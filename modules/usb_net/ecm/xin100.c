/*
 * Xin100 USB Ethernet Adapter Driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// #define	DEBUG			// error path messages, extra info
// #define	VERBOSE			// more; success messages

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/workqueue.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>
#include <linux/usb/usbnet.h>
#include <linux/hid.h>
#include <linux/hiddev.h>
#include <linux/hid-debug.h>
#include <linux/hidraw.h>
#include <linux/spinlock.h>
#include "xin100.h"

#define MAC_ADDRESS_SIZE (6)

#define XIN100_USB_VENDOR_REQUEST_READ_MACADDRESS (0xA1)

#define XIN100_VENDOR_ID	0x055F
#define XIN100_PRODUCT_ID	0x017B

//#define XIN100_VENDOR_ID	0x0988
//#define XIN100_PRODUCT_ID	0x0100


static int xin100_get_mac_address(struct usbnet *dev, unsigned char *data)
{
	unsigned char * macaddress = NULL;
	int ret = -ENODEV;

	if((dev == NULL) || (data == NULL))
	{
		printk(KERN_ERR "Xin100 Ether: dev or data is NULL in xin100_get_mac_address!!!");
		return -ENODEV;
	}

	macaddress = (unsigned char*)kmalloc(MAC_ADDRESS_SIZE, GFP_KERNEL);
	if (!macaddress)
	{
		printk(KERN_ERR "Xin100 Ether: failed to allocate memory for mac address!!!");
		return -ENOMEM;
	}

	ret = usb_control_msg(dev->udev, usb_rcvctrlpipe(dev->udev, 0),
		XIN100_USB_VENDOR_REQUEST_READ_MACADDRESS,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		0, 0, macaddress, MAC_ADDRESS_SIZE, USB_CTRL_GET_TIMEOUT);

	if (ret < 0)
	{	printk(KERN_ERR "Xin100 Ether: failed to send control message to get mac address from device, error = %d!!!", ret);
                kfree(macaddress);
		macaddress = NULL;
		return ret;
	}

	printk(KERN_ERR "Xin100 Ether: MAC Address: %02x - %02x - %02x - %02x - %02x - %02x\n", macaddress[0], macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5]);

	memcpy(data, macaddress, MAC_ADDRESS_SIZE);

	kfree(macaddress);
	macaddress = NULL;

	return ret;
}

int usbnet_generic_xin100_bind(struct usbnet *dev, struct usb_interface *intf)
{
	struct usb_interface_descriptor	*d = &(intf->cur_altsetting->desc);
	int				status;

	status = usbnet_get_endpoints(dev, intf);
	if (status < 0) {
		/* ensure immediate exit from usbnet_disconnect */
		return status;
	}

	if ((d->bNumEndpoints !=  XIN100_ENDPOINT_NUMBERS) && !dev->status) {
		printk(KERN_ERR "Xin100 Ether: the endpoints settings for xin100 are incorrect!\n");
		return -ENODEV;
	} 
	
	return 0;
}

void xin100_unbind(struct usbnet *dev, struct usb_interface *intf)
{
	if(dev == NULL)
	{
		printk(KERN_ERR "dev is NULL in unbind!!!");
		return;
	}

	return;
}

static int xin100_bind(struct usbnet *dev, struct usb_interface *intf)
{
	int status;
	int i = 0;
	//unsigned char xin100_mac_address[MAC_ADDRESS_SIZE] = {0x02, 0x50, 0xF2, 0x00, 0x01, 0xFF};
	unsigned char xin100_mac_address[MAC_ADDRESS_SIZE] = {0};

	status = usbnet_generic_xin100_bind(dev, intf);
	if (status < 0)
		return status;

	// get the available MAC address from the pool
	status = xin100_get_mac_address(dev, &xin100_mac_address[0]);
	if(status < 0)
	{
	    printk(KERN_ERR "get mac addr status is error!!!");
            return status;
	}
		
        
	// update the MAC address into the usbnet structure
    	for (i = 0; i < MAC_ADDRESS_SIZE; i++)
	{
		dev->net->dev_addr [i] = xin100_mac_address[i];	
	}
	//modify net name	
	strcpy(dev->net->name,"lte%d");
	
	dev->hard_mtu = XIN100_ETH_MAX_PACKET_SIZE;

	return 0;
}

void xin100_status(struct usbnet *dev, struct urb *urb)
{
	struct usb_cdc_notification	*event;

	if (urb->actual_length < sizeof *event)
	{
		printk(KERN_ERR "Xin100 Ether: the size of status message is incorrect!\n");
		return;
	}

	event = urb->transfer_buffer;
	switch (event->bNotificationType) {
	case USB_CDC_NOTIFY_NETWORK_CONNECTION:
		printk(KERN_ERR  "Xin100 Ether: CDC: carrier %s\n",
			  event->wValue ? "on" : "off");
		if (event->wValue)
			netif_carrier_on(dev->net);
		else
			netif_carrier_off(dev->net);
		break;

	default:
		printk(KERN_ERR  "Xin100 Ether: CDC: unexpected notification %02x!\n",
			   event->bNotificationType);
		break;
	}
}
//.status =	xin100_status,
static const struct driver_info	xin100_info = {
	.description =	"Xin100 USB Ethernet Adapter",
	//.flags =	FLAG_ETHER,
	.bind =		xin100_bind,
	.unbind =	xin100_unbind,	
        //.status =	xin100_status,
        .status =	usbnet_cdc_status,
};

/*-------------------------------------------------------------------------*/
static const struct usb_device_id products[] = {
	{
		USB_DEVICE_INTERFACE_PROTOCOL(XIN100_VENDOR_ID, XIN100_PRODUCT_ID, 0xFF),
		.driver_info = (unsigned long) &xin100_info,
	},
	{},
};
MODULE_DEVICE_TABLE(usb, products);



static struct usb_driver xin100_driver = {
	.name =		"xin100_ether",
	.id_table =	products,
	.probe =	usbnet_probe,
	.disconnect =	usbnet_disconnect,
	.suspend =	usbnet_suspend,
	.resume =	usbnet_resume,
	.reset_resume =	usbnet_resume,
	.supports_autosuspend = 1,
};


static int __init xin100_init(void)
{
    return usb_register(&xin100_driver);
}
module_init(xin100_init);

static void __exit xin100_exit(void)
{
 	usb_deregister(&xin100_driver);
}
module_exit(xin100_exit);

MODULE_AUTHOR("huang");
MODULE_DESCRIPTION("Xin100 USB LTE Adapter");
MODULE_LICENSE("GPL");




