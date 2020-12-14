#ifndef HOC_K_H_
#define HOC_K_H_

#define MESH_BIG_ENDIAN

#ifdef MESH_BIG_ENDIAN
#define MESH_HTONL(x) (x)
#define MESH_HTONS(x) (x)

#define MESH_NTOHL(x) (x)
#define MESH_NTOHS(x) (x)

#else
#define MESH_HTONL(x)	((((x) & 0x000000ff) << 24)  | \
    (((x) & 0x0000ff00) <<  8)  | \
    (((x) & 0x00ff0000) >>  8)  | \
    (((x) & 0xff000000) >> 24))

#define MESH_HTONS(x)  ((((x) & 0x00ff) << 8) | \
    (((x) & 0xff00) >> 8))

#define MESH_NTOHL(x)	((((x) & 0x000000ff) << 24) | \
    (((x) & 0x0000ff00) <<  8) | \
    (((x) & 0x00ff0000) >>  8) | \
    (((x) & 0xff000000) >> 24))

#define MESH_NTOHS(x)  ((((x) & 0x00ff) << 8) | \
    (((x) & 0xff00) >> 8))
#endif   /******MESH_BIG_ENDIAN********/


#define  KER_BASE      0X8000
#define  USR_BASE      0X9000

/***************内核态与用户态接口信息ID***************************/
//ker to usr
#define  O_KER_USR_IP_MAC_INFO        (KER_BASE+0X1)
#define  O_KER_USR_RTE_DATA_IND      (KER_BASE+0X2)
//usr to ker
#define  O_USR_KER_ROUTE_INFO          (USR_BASE+0X1)
#define  O_USR_KER_NET_IP_INFO         (USR_BASE+0X2)
#define  O_USR_KER_DATA_IND              (USR_BASE+0X3)
#define  O_USR_KER_ARP_TIMER           (USR_BASE+0X4)


#define AGT_MAX_LOCAL_USER_NUM         (16)//最大用户接入数量
#define MAX_HUB_NUM                              (32)//最大节点数量

//netlink  自定义协议
#define    NETLINK_AGT                             17
#define    ETHALEN                              	(14)

#define	 LOOP_IP_U32	       (0x7F000001) //127.0.0.1
#define    MANAGER_IP_U32	(0xC0A80101) //必须与MANAGER_IP保持一致192.168.1.1
#define    BCAST_IP_U32	       (0xC0A801FF) // 192.168.1.255
/*用户设备存活时间上限，每秒减1。当收到用户设备的ARP请求时，
 * 把相应设备的存活时间重置为该值，如果该值减到0，
 * 则认为该用户设备已经脱离本MESH设备。
 */
#define USER_LIVE_TIME	(3)




//重新封装一下arp消息头格式
typedef struct
{
	__be16		ar_hrd;		/* format of hardware address	*/
	__be16		ar_pro;		/* format of protocol address	*/
	unsigned char	ar_hln;		/* length of hardware address	*/
	unsigned char	ar_pln;		/* length of protocol address	*/
	__be16		ar_op;		/* ARP opcode (command)		*/

	 /*
	  *	 Ethernet looks like this : This bit is variable sized however...
	  */
	unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
	unsigned char		ar_sip[4];		/* sender IP address		*/
	unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
	unsigned char		ar_tip[4];		/* target IP address		*/
} STRU_ARP_HEAD;

typedef struct
{
	u32 u32AgtMsgId;
	u32 u32Num;
	struct
	{
		u32 u32UserIp;
		u8  au8UserMac[8];
		u8  au8LinkIfaceMac[8]; // 用户设备所接入HUB的接口MAC
		s32 s32TimeToLive;		// 每1秒减1，当该值为0时，表示用户设备离线
	} astruUserDetail[AGT_MAX_LOCAL_USER_NUM];
}STRU_IP_MAC_LOCAL_USER_TABLE;



// STRU_ROUTE_INFO
typedef struct
{
	u8 au8DstAddr[8];         //dst mac
	u8 au8InterfaceAddr[8];  //which road
	u8 au8NextIfaceAddr[8];		//next_mac
} STRU_ROUTE_INFO;
typedef struct
{
	u32 u32AgtMsg;
	u32 u32NumOfRoute;
	STRU_ROUTE_INFO astruRouteInfo[MAX_HUB_NUM];
}STRU_ROUTE_INFO_RSP;


//用户IP对应MAC表
typedef struct
{
	u32 u32UserIp;
	u8  au8UserMac[8];
}STRU_USR_MAC_IP_RSP;
typedef struct
{
	u32 u32AgtMsg;
	u32 u32Num;
	STRU_USR_MAC_IP_RSP      astruUsrInfo[32];
}STRU_USR_INT_IP_RSP;


// ----------信令上传----------

typedef struct
{
	u32 u32AgtMsgId;
	u8 au8SrcMac[8]; //interface
	u8 au8LocalRxInterfaceMac[8];
	u32 u32Len;
	u8  au8RteData[4096];
} STRU_RTE_DATA_IND;


typedef struct
{
	u32 u32AgtMsgId;
	u32 u32Len;
	u8  au8RteData[2048];
} STRU_TX_RTE_DATA;



typedef struct
{
	u32 u32AgtMsgId;
} STRU_AGT_UK_TIMER_IND;

#endif  /****hoc.h*****/
