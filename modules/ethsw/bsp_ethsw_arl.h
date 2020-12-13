/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_arl.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/

/******************************** 包含文件声明 ********************************/

/******************************** 宏和常量定义 ********************************/

/******************************** 类型定义 ************************************/

/* ARL表信息结构 */
typedef struct tag_STRU_ETHSW_ARL_TABLE
{
	u32 CastType:1,     /* 表示此表是单播的还是多播的0:Unicast,1:Multicast */
		VlanId:3,       /* VLAN ID应当是12位,但由于该字段不用,所以缩短其位 */
		valid:1,        /* entry is valid,是否有效,0:无效;1:有效 */
		StaticEntry:1,  /* entry is static,是否为静态,0:非静态;1:静态 */
		age:1,          /* entry accessed/learned since ageing process */
		priority:3,     /* only valid in static entries,up to 4 priorities,but 3 bit */
		ChipId:2,       /* chip ID,00:本芯片 */
		PortId:20;      /* port ID,MAC地址对应的端口单播0~(MAX_USED_PORTS - 1),组播时为端口MAP */
	u8  u8MacAddr[6];   /* MAC addr,与端口对应的MAC地址 */
} STRU_ETHSW_ARL_TABLE;

/* ARL表读/写控制结构 */
typedef struct tag_STRU_ETHSW_ARL_RW_CTRL
{
	u8 ArlStart:1;      /* ARL start/done (1 = start) */
	u8 rsvd:6;          /* reserved */
	u8 ArlRW:1;         /* ARL read/write (1 = read) */
} STRU_ETHSW_ARL_RW_CTRL;

/* MAC地址结构 */
typedef struct tag_STRU_ETHSW_MAC_ADDR
{
	u8 u8MacAddr[6];
} STRU_ETHSW_MAC_ADDR;


/* MAC地址及其对应的端口号/端口MAP */
typedef struct tag_STRU_ETHSW_MAC_PORT
{      
	u16 u16PortId;           
	u8  u8MacAddr[6];      
} STRU_ETHSW_MAC_PORT;

/*查找ARL表时的控制结构 */
typedef struct tag_STRU_ETHSW_ARL_SEARCH_CTRL
{
	u8 ArlStart:1;      /* ARL start/done (1=start) */
	u8 rsvd:6;          /* reserved */
	u8 valid:1;         /* ARL search result valid */
} STRU_ETHSW_ARL_SEARCH_CTRL;



/* BCM5389读ARL表时,ARL Table MAC/VID Entry0/1寄存器结构 */
typedef struct tag_STRU_ETHSW_ARL_MAC_VID
{
	u16 rsvd:4;
	u16 VlanId:12;
	u8  u8MacAddr[6];   /* MAC addr,与端口对应的MAC地址 */
} STRU_ETHSW_ARL_MAC_VID;

/* BCM5389读ARL表时,ARL Table Data Entry0/1寄存器结构 */
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



/******************************* 头文件结束 ***********************************/
