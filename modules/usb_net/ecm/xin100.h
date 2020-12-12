
#ifndef __XIN100_Ethernet__
#define __XIN100_Ethernet__

#define XIN100_ENDPOINT_NUMBERS (3)
#define XIN100_ETH_HEADER_SIZE             14
#define XIN100_ETH_MAX_DATA_SIZE           1500
#define XIN100_ETH_MAX_PACKET_SIZE         (XIN100_ETH_HEADER_SIZE + XIN100_ETH_MAX_DATA_SIZE)
#define XIN100_ETH_MIN_PACKET_SIZE         60

#define XIN100_EP0_IN_USE		1
#define XIN100_DISCONNECTING            2
#define XIN100_MAX_REPORT_SIZE           64

#define HID_OUTPUTREPORT       (0x02)
#define XIN100_MINOR_BASE	176


typedef struct Xin100 {
	struct usb_device	*usb;
	struct usb_interface	*intf;
	unsigned		flags;
	struct urb		*ctrl_urb;
	struct usb_ctrlrequest*	dr;
	wait_queue_head_t	ctrl_wait;
        char *                  ctrl_out_buf;
} Xin100_t;

#endif

