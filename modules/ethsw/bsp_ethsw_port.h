/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_port.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/




/* 端口状态控制结构,CONTROL PAGE REGISTER MAP : 8bit registers */
typedef struct tag_STRU_ETHSW_PORT_CTRL
{
    u8 StpState:3,       /* spanning tree state */
       rsvd:3,           /* reserved */
       TxDisable:1,      /* tx disable */
       RxDisable:1;      /* rx disable */
} STRU_ETHSW_PORT_CTRL;

/* 交换模式设置结构 */
typedef struct tag_STRU_ETHSW_SWITCH_MODE
{
    u8 rsvd:6,
       SwFwdEn:1,    /* Software Forwarding Enable */
       SwFwdMode:1;  /* Software Forwarding Mode,inverse of SwFwdEn */
} STRU_ETHSW_SWITCH_MODE;

typedef struct tag_STRU_ETHSW_MCAST_CTRL
{
    u8 MlfFmEn:1,     /* Multicast Lookup Fail Forward Map Enable */
       UniFmEn:1,     /* Unicast Lookup Fail Forward Map Enable */
       Rsvd:5,        /* Spare registers */
       MulticastEn:1; /* Must be set to 1 to support Multicast addresses in the ARL table */
} STRU_ETHSW_MCAST_CTRL;

/* 端口状态强制设置结构 */
typedef struct tag_STRU_ETHSW_PORT_STAT
{
    u8 PhyScanEn:1, 
       OverrideEn:1,/*CPU set software override bit to 1 to make bits [5:0]
                       affected. PHY scan register is overridden.*/
       TxFlowEn:1,   /*Software TX flow control enable */
       RxFlowEn:1,   /* Software RX flow control enable */
       Speed:2,      /*Software port speed setting:b00:10M,b01:100M,b10:1000M*/
       Duplex:1,     /*Software duplex mode setting:0: Half-duplex 1: Full-duplex*/
       LinkState:1;  /*0:Link down 1:Link up*/
} STRU_ETHSW_PORT_STAT;

/* Mirror捕获控制寄存器结构 */
typedef struct tag_STRU_ETHSW_MIRROR_CAP_CTRL
{
    u16 MirrorEnable:1,  /* Mirror功能使能标记 */
        BlkNoMirror:1,   /* 阻塞非Mirror数据标记 */
        rsvd10:10,
        CapturePort:4;   /* 捕获端口MAP,MirrorPort */
} STRU_ETHSW_MIRROR_CAP_CTRL;

/* Ingress/Egress Mirror控制寄存器结构 */
typedef struct tag_STRU_ETHSW_MIRROR_CTRL
{
    u16 filter:2,    
        DividEnable:1,
        rsvd4:4,
        MirroredPort:9; 
} STRU_ETHSW_MIRROR_CTRL;

#define PORT_LINK_UP                   0x1   /* 端口LINK UP */
#define PORT_LINK_DOWN                 0x0   /* 端口LINK DOWN */


s32 ethsw_set_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort);
s32 ethsw_set_status(u8 u8PortId, u8 u8TxEnable, u8 u8RxEnable);

/******************************* 头文件结束 ***********************************/
