/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_port.h 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/




/* �˿�״̬���ƽṹ,CONTROL PAGE REGISTER MAP : 8bit registers */
typedef struct tag_STRU_ETHSW_PORT_CTRL
{
    u8 StpState:3,       /* spanning tree state */
       rsvd:3,           /* reserved */
       TxDisable:1,      /* tx disable */
       RxDisable:1;      /* rx disable */
} STRU_ETHSW_PORT_CTRL;

/* ����ģʽ���ýṹ */
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

/* �˿�״̬ǿ�����ýṹ */
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

/* Mirror������ƼĴ����ṹ */
typedef struct tag_STRU_ETHSW_MIRROR_CAP_CTRL
{
    u16 MirrorEnable:1,  /* Mirror����ʹ�ܱ�� */
        BlkNoMirror:1,   /* ������Mirror���ݱ�� */
        rsvd10:10,
        CapturePort:4;   /* ����˿�MAP,MirrorPort */
} STRU_ETHSW_MIRROR_CAP_CTRL;

/* Ingress/Egress Mirror���ƼĴ����ṹ */
typedef struct tag_STRU_ETHSW_MIRROR_CTRL
{
    u16 filter:2,    
        DividEnable:1,
        rsvd4:4,
        MirroredPort:9; 
} STRU_ETHSW_MIRROR_CTRL;

#define PORT_LINK_UP                   0x1   /* �˿�LINK UP */
#define PORT_LINK_DOWN                 0x0   /* �˿�LINK DOWN */


s32 ethsw_set_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort);
s32 ethsw_set_status(u8 u8PortId, u8 u8TxEnable, u8 u8RxEnable);

/******************************* ͷ�ļ����� ***********************************/
