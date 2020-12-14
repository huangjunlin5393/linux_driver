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

/***************�ں�̬���û�̬�ӿ���ϢID***************************/
//ker to usr
#define  O_KER_USR_IP_MAC_INFO        (KER_BASE+0X1)
#define  O_KER_USR_RTE_DATA_IND      (KER_BASE+0X2)
//usr to ker
#define  O_USR_KER_ROUTE_INFO          (USR_BASE+0X1)
#define  O_USR_KER_NET_IP_INFO         (USR_BASE+0X2)
#define  O_USR_KER_DATA_IND              (USR_BASE+0X3)
#define  O_USR_KER_ARP_TIMER           (USR_BASE+0X4)


#define AGT_MAX_LOCAL_USER_NUM         (16)//����û���������
#define MAX_HUB_NUM                              (32)//���ڵ�����

//netlink  �Զ���Э��
#define    NETLINK_AGT                             17
#define    ETHALEN                              	(14)

#define	 LOOP_IP_U32	       (0x7F000001) //127.0.0.1
#define    MANAGER_IP_U32	(0xC0A80101) //������MANAGER_IP����һ��192.168.1.1
#define    BCAST_IP_U32	       (0xC0A801FF) // 192.168.1.255
/*�û��豸���ʱ�����ޣ�ÿ���1�����յ��û��豸��ARP����ʱ��
 * ����Ӧ�豸�Ĵ��ʱ������Ϊ��ֵ�������ֵ����0��
 * ����Ϊ���û��豸�Ѿ����뱾MESH�豸��
 */
#define USER_LIVE_TIME	(3)




//���·�װһ��arp��Ϣͷ��ʽ
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
		u8  au8LinkIfaceMac[8]; // �û��豸������HUB�Ľӿ�MAC
		s32 s32TimeToLive;		// ÿ1���1������ֵΪ0ʱ����ʾ�û��豸����
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


//�û�IP��ӦMAC��
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


// ----------�����ϴ�----------

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
