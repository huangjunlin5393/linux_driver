

#ifndef XIN_COM_H
#define XIN_COM_H




//////////common data struct for net and com start//////////////////////
typedef enum {

	PKT_TYPE_ETHER_PKT=0,
	PKT_TYPE_COM_PKT,
	PKT_TYPE_RX_ERROR,
	PKT_TYPE_MAXS
 }Pkt_Type;

typedef enum {

	Type_MLOG=1,
	Type_TRACE,
	Type_AtCommand,
	Type_DspLOG,
	
 }Pkt_SubType;


 struct pkt_data
 {
	 u8 	 pkt_type;
	 u8 	 pkt_subtype;
	 u16	 pkt_size;
	 u16	 padsize;
	 u16	 rsv;
	 u8 	 payload[0];
 };
 
 
#define MAGICNUM		0x5a5b
 typedef struct xin_hdr{
	 u16				 magicnum;			 //0x5a5b
	 u16				 pkt_num;	 
	 u32				 pkt_sumsize;
	 struct  pkt_data	 pkt_infor[0];	 
 }xin_hdr;
 //////////common data struct for net and com end//////////////////////

#define		HEAD_SIZE_SUM		(sizeof(xin_hdr)+sizeof(struct pkt_data))

//////////netdevice struct and data for net//////////////////////

 struct xinnet_priv {
	 struct sdio_func *func;
	 struct net_device *netdev;
	 struct device *dev;
	 int (*hw_host_to_card) (struct xinnet_priv *priv, u8 type, u8 *payload, u16 nb);
 };
 
#define 	UART_TX_MAX_BUFFER_CNT				(64)
#define 	UART_TX_MAX_DATA_SEG_LEN			(6144)
#define		RING_INDEX_NEXT(i,total)			(i>=total?0:i)
#define		PKT_MIN_SIZE						60


#define 	AHB_TRANSFER_COUNT_ADDR1			0x1C
#define 	AHB_TRANSFER_COUNT_ADDR2			0x1D
#define 	AHB_TRANSFER_COUNT_ADDR3			0x1E


 typedef struct{
	 u32		 length;
	 u8 		 *pDataSeg;
 }DataUnit;
 
 typedef struct
 {
	 u8 			 ucRead;
	 u8 			 ucWrite;
	 u16			 usFreeCnt;
	 spinlock_t 	 databuff_lock;
	 DataUnit		 astTxDataUnit[UART_TX_MAX_BUFFER_CNT];
	 DataUnit		 *pastTxDataUnit[UART_TX_MAX_BUFFER_CNT];
	 u8 			 aucTxDataBuffer[UART_TX_MAX_BUFFER_CNT][UART_TX_MAX_DATA_SEG_LEN];//192KB
 } RxDataBufferInfo;
 
 typedef struct
 {
	 u8 			 ucRead;
	 u8 			 ucWrite;
	 u16			 usDataCnt;
	 spinlock_t 	 dataunit_lock;
	 DataUnit		 *pastTxDataUnit[UART_TX_MAX_BUFFER_CNT];
 } RxDataPktInfor;
 





 
#define PACKET_NUM_MAX	8
#define TX_BUFFER_SIZE	0x80000

struct gpio_rd_req{
	int rd_irq;
	int probed;
	unsigned long jiffies;
	struct xin_net_card* pxin_net_card;
};
 
 struct xin_net_card
 {
	 struct sdio_func		 *func;
	 struct xinnet_priv 	 *priv;
	 
	 struct workqueue_struct *tx_workqueue;
	 struct work_struct 	 tx_pkt_worker;
	 
	 u8 					 *tx_buffer;
	 spinlock_t 			 tx_lock;
	 spinlock_t 			 rx_lock;
 
	 struct timer_list		 mytimer;
	 DataUnit*				 current_data_unit;
	 RxDataBufferInfo		 RxDataBufferInfo;
	 RxDataPktInfor 		 sdioTxDataInfo;


	 struct workqueue_struct *rx_workqueue;
	 struct work_struct 	 rx_pkt_worker;	 

	 struct gpio_rd_req *pgpioReq;
 };

 //////////netdevice struct and data for net//////////////////////













 //////////tty struct and data for com//////////////////////

#define DRIVER_VERSION "v1.0"
#define DRIVER_AUTHOR "Zhang"
#define DRIVER_DESC "Linux Virtual COM Driver"
 
 
 /* Module information */
 MODULE_AUTHOR( DRIVER_AUTHOR );
 MODULE_DESCRIPTION( DRIVER_DESC );
 MODULE_LICENSE("GPL");
 
#define XIN_TTY_MAJOR		288	/* experimental range */
#define XIN_TTY_MINORS		3	/* only have 3 devices ttyVCOM0 ttyVCOM1 ttyVCOM2*/
#define XIN_BUFFER_SIZE    10240
 
 
 /* Our fake UART values */
#define MCR_DTR		0x01
#define MCR_RTS		0x02
#define MCR_LOOP	0x04
#define MSR_CTS		0x08
#define MSR_CD		0x10
#define MSR_RI		0x20
#define MSR_DSR		0x40
 
 
#define MAX_BUFFER_SIZE 			4096
#define DATA_BUFFER_SIZE 			(MAX_BUFFER_SIZE-sizeof(xin_hdr))
#define MAX_REAL_DATA_SIZE 			DATA_BUFFER_SIZE
 
#define WRITE_PACKET_SIZE 			60
#define READ_PACKET_SIZE 			(512-4)

 // for main type
#define MAIN_TYPE_IP 0
#define MAIN_TYPE_DEBUG 1
 
 // for sub type
#define SUB_TYPE_IP        			0
#define SUB_TYPE_MLOG      			1
#define SUB_TYPE_Trace     			2
#define SUB_TYPE_AtCommand 			3
#define SUB_TYPE_ROM_UPDATE 		(256)
#define SUB_TYPE_INVALID   			0xFF
 
 // for in out flag
#define DIRECTION_READ  			0
#define DIRECTION_WRITE 			1
 
 // for magic number
#define MAGIC_NUMBER 				0x5a5b
 
 static DEFINE_MUTEX(global_mutex);
 
 typedef unsigned short USHORT;
 typedef unsigned char UCHAR;
 typedef unsigned int UINT;


 typedef struct __SDIO_BUF
 {
	 xin_hdr	 Hdr;
	 UCHAR		 Buffer[DATA_BUFFER_SIZE];
 } __attribute__((__packed__)) SDIO_BUF;  // pay attention to ALIGNMENT
 
 struct xin_serial {
	 struct tty_struct	 *tty;		 /* pointer to the tty for this device */
	 /* for tiocmget and tiocmset functions */
	 int		 msr;		 /* MSR shadow */
	 int		 mcr;		 /* MCR shadow */
	 struct tty_port		 port;
	 struct device* 		 dev;
	 int opened;
 };
 
 struct xin_hid {
	 // buffer for read hid data
	 SDIO_BUF			 read_buffer;
	 int				 read_buffer_total_length;
	 int				 read_buffer_index;
 
	 // buffer for write hid data
	 SDIO_BUF			 write_buffer;
	 char				 *send_buffer;
 
	 struct sdio_func	 *func;
 };


 //////////tty struct and data for com//////////////////////


 typedef struct __USB_HDR
 {
	 USHORT magicNumber; //0x55AA
	 USHORT length;
	 USHORT m_type: 4;
	 USHORT sub_type: 12;
	 UCHAR inOutFlag; // 0: read; 1: write
	 UCHAR reserved;
 }__attribute__((__packed__)) USB_HDR; // pay attention to ALIGNMENT


typedef int (*gpio_irq_callback)(void);

void gpio_xinlte_register_callback(gpio_irq_callback callback);

void gpio_xinlte_apready_set_value(int value);
#endif
