/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_arl.h 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/

/******************************** �����ļ����� ********************************/

/******************************** ��ͳ������� ********************************/

/******************************** ���Ͷ��� ************************************/

/* ARL����Ϣ�ṹ */
typedef struct tag_STRU_ETHSW_ARL_TABLE
{
	u32 CastType:1,     /* ��ʾ�˱��ǵ����Ļ��Ƕಥ��0:Unicast,1:Multicast */
		VlanId:3,       /* VLAN IDӦ����12λ,�����ڸ��ֶβ���,����������λ */
		valid:1,        /* entry is valid,�Ƿ���Ч,0:��Ч;1:��Ч */
		StaticEntry:1,  /* entry is static,�Ƿ�Ϊ��̬,0:�Ǿ�̬;1:��̬ */
		age:1,          /* entry accessed/learned since ageing process */
		priority:3,     /* only valid in static entries,up to 4 priorities,but 3 bit */
		ChipId:2,       /* chip ID,00:��оƬ */
		PortId:20;      /* port ID,MAC��ַ��Ӧ�Ķ˿ڵ���0~(MAX_USED_PORTS - 1),�鲥ʱΪ�˿�MAP */
	u8  u8MacAddr[6];   /* MAC addr,��˿ڶ�Ӧ��MAC��ַ */
} STRU_ETHSW_ARL_TABLE;

/* ARL���/д���ƽṹ */
typedef struct tag_STRU_ETHSW_ARL_RW_CTRL
{
	u8 ArlStart:1;      /* ARL start/done (1 = start) */
	u8 rsvd:6;          /* reserved */
	u8 ArlRW:1;         /* ARL read/write (1 = read) */
} STRU_ETHSW_ARL_RW_CTRL;

/* MAC��ַ�ṹ */
typedef struct tag_STRU_ETHSW_MAC_ADDR
{
	u8 u8MacAddr[6];
} STRU_ETHSW_MAC_ADDR;


/* MAC��ַ�����Ӧ�Ķ˿ں�/�˿�MAP */
typedef struct tag_STRU_ETHSW_MAC_PORT
{      
	u16 u16PortId;           
	u8  u8MacAddr[6];      
} STRU_ETHSW_MAC_PORT;

/*����ARL��ʱ�Ŀ��ƽṹ */
typedef struct tag_STRU_ETHSW_ARL_SEARCH_CTRL
{
	u8 ArlStart:1;      /* ARL start/done (1=start) */
	u8 rsvd:6;          /* reserved */
	u8 valid:1;         /* ARL search result valid */
} STRU_ETHSW_ARL_SEARCH_CTRL;



/* BCM5389��ARL��ʱ,ARL Table MAC/VID Entry0/1�Ĵ����ṹ */
typedef struct tag_STRU_ETHSW_ARL_MAC_VID
{
	u16 rsvd:4;
	u16 VlanId:12;
	u8  u8MacAddr[6];   /* MAC addr,��˿ڶ�Ӧ��MAC��ַ */
} STRU_ETHSW_ARL_MAC_VID;

/* BCM5389��ARL��ʱ,ARL Table Data Entry0/1�Ĵ����ṹ */
typedef struct tag_STRU_ETHSW_ARL_DATA
{
	u16 valid:1;     
	u16 StaticEntry:1;
	u16 age:1;
	u16 priority:3;
	u16 rsvd:1;
	u16 PortId:9;
} STRU_ETHSW_ARL_DATA;

s32 ethsw_add_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort);
s32 ethsw_remove_arl_entry(const STRU_ETHSW_MAC_ADDR *pstruMacAddr);
s32 ethsw_dump_arl_entry(u16 u16ArlNum, STRU_ETHSW_ARL_TABLE *pstruArlTable);



/******************************* ͷ�ļ����� ***********************************/
