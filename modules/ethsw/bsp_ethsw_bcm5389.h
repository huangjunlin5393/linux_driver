/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_bcm5389.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/

/******************************** 头文件保护开头 ******************************/
#ifndef DD_ETHSW_BCM5389_H
#define DD_ETHSW_BCM5389_H


/******************************** 宏和常量定义 ********************************/

/* 初始化标记 */
#define ETHSW_SPI_INIT_FLAG            0x01
#define ETHSW_CHIP_INIT_FLAG           0x02

/* 打印选项 */
#define ETHSW_PRINT_PARAMETER          0x1   /* 参数错误 */
#define ETHSW_PRINT_TIMEOUT            0x2   /* 超时错误 */
#define ETHSW_PRINT_EXCEPTION          0x4   /* 异常错误 */
#define ETHSW_PRINT_FAIL               0x8  /* 函数执行失败打印返回值 */
#define ETHSW_PRINT_INFO               0x10  /* 打印提示信息 */

#define RETURN_IF_ERROR(op)    {s32 _rv_; if ((_rv_ = (s32)(op)) < 0) {if ((g_u32PrintRules & ETHSW_PRINT_FAIL) != 0) \
                                {(void)printf((char *)"The ReturnValue is %d\n", _rv_);} return (_rv_);} }



#define MAX_USED_PORTS                 0x09U
#define ONE_BYTE_WIDTH                 0x01U  /* 寄存器的宽度为1个字节 */
#define TWO_BYTE_WIDTH                 0x02U  /* 寄存器的宽度为2个字节 */
#define FOUR_BYTE_WIDTH                0x04U  /* 寄存器的宽度为4个字节 */
#define SIX_BYTE_WIDTH                 0x06U  /* 寄存器的宽度为6个字节 */
#define EIGHT_BYTE_WIDTH               0x08U  /* 寄存器的宽度为8个字节 */
#define ETHSW_TIMEOUT_VAL              100

/* 最大可能用到的ARL表的数目 */
#define MAX_POSSIBLE_USED_ARL          0x100U



#define ETHSW_DELAY(a)         {vu32 _i_;  for (_i_ = 0; _i_ < 4 * (a); _i_++) {__asm(" sync");}}



/* 错误返回值定义 */
#define E_ETHSW_OK                       0  /* 成功 */
#define E_ETHSW_INIT_FAILED            -1
#define E_ETHSW_FAIL                    -2
#define E_ETHSW_WRONG_PARAM            -3  /* 参数错误 */
#define E_ETHSW_TIMEOUT                 -4  /* 读写ARL表/寄存器时超时 */

#define E_ETHSW_ARL_TOO_MANY           -10  /* 有效的ARL表太多,输入参数空间不够 */
#define E_ETHSW_ARL_NOT_FOUND          -11  /* 没查找到相匹配的ARL表项 */
#define E_ETHSW_BUCKET_FULL            -12
#define E_ETHSW_CHIP_REINIT            -13  /* 芯片重复初始化 */


/* BCM5389 GLOBAL PAGE ADDRESS LISTS */
#define ETHSW_CTRL_PAGE                0x00U  /* Control registers */
#define ETHSW_STAT_PAGE                0x01U  /* Status register */
#define ETHSW_MGMT_PAGE                0x02U  /* Management Mode registers */

#define ETHSW_ARL_CTRL_PAGE            0x04U  /* ARL Control Registers */
#define ETHSW_ARL_ACCESS_PAGE          0x05U  /* ARL Access Registers */
#define ETHSW_SERDES_PAGE(n)           (0x10+n)
#define ETHSW_PORTMIB_PAGE(n)          (0x20+n)

#define ETHSW_QOS_PAGE                  0x30U  /* Quality of Service (QoS) Registers */
#define ETHSW_PVLAN_PAGE                0x31U  /* PVLAN registers */
#define ETHSW_QVLAN_PAGE                0x34U  /* IEEE Standard 802.1Q VLAN registers */



/* (PAGE0x00) CONTROL Register页面下所有寄存器的定义 */
#define PORT_TRAFIC_CTRL(n)            (n)    /* Port Control Register */
#define IMP_TRAFIC_CTRL                0x08U  /*  8b:IMP Port Control Register */
#define SWITCH_MODE                    0x0BU  /*  8b:Switch Mode Register */
#define IMP_PORT_STATE_OR              0x0EU  /*  8b:IMP State Override register */
#define PORT_FORWARD_CTRL              0x21U  /*  8b: Port Forward Control Register */

#define UNI_DLF_FW_MAP                 0x32U  /* 16b:Unicast Lookup Failed Forward Map Register */
#define MUL_DLF_FW_MAP                 0x34U  /* 16b:MULTICAST LOOKUP FAIL FORWARD MAP REGISTER */
#define EXTERN_PHY_SCAN_RESULT(n)     (0x50U+(n)) 
#define MII_PORT_STATE_OR(n)          (0x58U+(n)) /*  8b:PortN State Override register */

#define PAUSE_FRAME_DETECT_CTRL       0x80U  /*  8b:Pause Frame Detection Control Register,Pause帧检测设置寄存器 */

#define FAST_AGING_CTRL                0x88U  /*  8b:Fast-Aging Control Register */
#define FAST_AGING_PORT                0x89U  /*  8b:Fast-Aging Port Register */
#define FAST_AGING_VID                 0x8AU  /* 16b:Fast-Aging VID Register */

/* Note SPI Data/IO Registers*/
#define ETHSW_SPI_DATA_IO_0            0xf0U  /* SPI Data I/O 0 */
#define ETHSW_SPI_DATA_IO_1            0xf1U  /* SPI Data I/O 1 */
#define ETHSW_SPI_DATA_IO_2            0xf2U  /* SPI Data I/O 2 */
#define ETHSW_SPI_DATA_IO_3            0xf3U  /* SPI Data I/O 3 */
#define ETHSW_SPI_DATA_IO_4            0xf4U  /* SPI Data I/O 4 */
#define ETHSW_SPI_DATA_IO_5            0xf5U  /* SPI Data I/O 5 */
#define ETHSW_SPI_DATA_IO_6            0xf6U  /* SPI Data I/O 6 */
#define ETHSW_SPI_DATA_IO_7            0xf7U  /* SPI Data I/O 7 */
#define ETHSW_SPI_STATUS               0xfeU  /* SPI Status Registers */
#define ETHSW_SPI_ADDR                 0xffU  /* Page Registers */












/* (PAGE0x01) STATUS Register页面下所有寄存器的定义 */
#define LINK_STATUS_SUMMARY            0x00U  /* 16b:Link状态寄存器,Link Status Summary */
#define LINK_STATUS_CHANGE             0x02U  /* 16b:Link状态变化寄存器,Link Status Change */
#define PORT_SPEED_SUMMARY             0x04U  /* 32b:端口速率寄存器,Port Speed Summary */
#define DUPLEX_STATUS_SUMMARY          0x08U  /* 16b:端口双工状态寄存器,Duplex Status Summary */
#define PAUSE_STATUS_SUMMARY           0x0AU  /* 32b:端口PAUSE帧发送能力状态寄存器,Tx PAUSE Status Summary */

/* (PAGE0x01) End of Status Register */

/* (PAGE0x02) AGING/MIRRORING REGISTERS页面下所有寄存器的定义 */
#define GLOBAL_CONFIG                  0x00U  /*  8b:Global Management Config: 8bit*/
#define AGING_TIME_CTRL                0x06U  /* 32b:Aging Time Control */
/* MIRROR功能设置寄存器 */
#define MIRROR_CAPTURE_CTRL            0x10U  /* 16b:Mirror Capture Control Register */
#define INGRESS_MIRROR_CTRL            0x12U  /* 16b:Ingress Mirror Control Register */
#define INGRESS_MIRROR_DIVIDER         0x14U  /* 16b:Ingress Mirror Divider Register */

#define EGRESS_MIRROR_CTRL             0x1CU  /* 16b:Egress Mirror Control Register */

/* (PAGE0x02) End of AGING/MIRRORING Register */


/* (PAGE0x04) ARL控制寄存器页面下所有寄存器的定义,缺省值满足要求可以不设置 */
#define GLOBAL_ARL_CFG                 0x00U  /*  8b:Global ARL Configuration Register */
#define BPDU_MULTICAST_ADDR            0x04U  /* 48b:BPDU Multicast Address */
#define MULTIPORT_ADDR1                0x10U  /* 48b:BPDU Multicast Address1 */
#define MULTIPORT_VECTOR1              0x16U  /* 32b:Multiport Vector 1 */
#define MULTIPORT_ADDR2                0x20U  /* 48b:BPDU Multicast Address2 */
#define MULTIPORT_VECTOR2              0x26U  /* 32b:Multiport Vector 2 */
/* (PAGE0x04) End of ARL CONTROL REGISTER */

/* (PAGE 0x05) ARL/VTBL控制寄存器页面下所有寄存器的定义,对芯片的ARL表进行控制与访问 */
#define ARL_TABLE_RW_CTRL              0x00U  /* 8b:ARL表读写控制,ARL Read/Write Control Register */
#define MAC_ADDRESS_INDEX              0x02U  /* 48b:MAC地址索引,MAC Address Index Register */
#define VLAN_ID_INDEX                  0x08U  /* 16b:VLAN ID索引,VID Index Register */
#define ARL_TABLE_MAC_VID_ENTRY0       0x10U  /* 64b:ARL MAC/VID Entry 0 Register */
#define ARL_TABLE_DATA_ENTRY0          0x18U  /* 16b:ARL FWD Entry 0 Register */
#define ARL_TABLE_MAC_VID_ENTRY1       0x20U  /* 64b:ARL MAC/VID Entry 1 Register */
#define ARL_TABLE_DATA_ENTRY1          0x28U  /* 16b:ARL FWD Entry 1 Register */
#define ARL_TABLE_SEARCH_CTRL          0x30U  /*  8b:ARL Search Control Register */
#define ARL_TABLE_SEARCH_MAC_RESULT   0x33U  /* 64b:ARL Search MAC/VID Result Register0 */
#define ARL_TABLE_SEARCH_DATA_RESULT  0x3bU  /* 32b:ARL Search Result Register 0 */
#define VLAN_TABLE_RW_CTRL             0x60U  /*  8b:VTBL Read/Write Control Register */
#define VLAN_TABLE_ADDR_INDEX          0x61U  /* 16b:VTBL Address Index Register */
#define VLAN_TABLE_ENTRY               0x63U  /* 32b:VTBL Entry Register */
/* (PAGE 0x05) END of ARL CONTROL REGISTER */

/* (PAGE 0x10–0x17) INTERNAL SERDES REGISTER PAGE页面下所有寄存器的定义 */
#define SERD_MII_CTRL                  0x00U  /* 16b:MII Control Register */
#define SERD_MII_STAT                  0x02U  /* 16b:MII Status Register */
#define SERD_AN_ADV                    0x08U  /* 16b:Auto-Negotiation Advertisement Register  */
#define SERD_AN_LP_ABILITY             0x0AU  /* 16b:Auto-Negotiation Link Partner Ability Register  */
#define SERD_AN_EXPANSION              0x0CU  /* 16b:Auto-Negotiation Expansion Registe */
#define SERD_EXTEND_STAT               0x1EU  /* 16b:Extended Status Register (Page 10h ~ 1Fh: Addr 1Eh ~ 1Fh) */
#define SERD_SGMII_CTRL1               0x20U  /* 16b:SerDes/SGMII Control 1 Register in Block 0 */
#define SERD_SGMII_CTRL2               0x22U  /* 16b:SerDes/SGMII Control 2 Register in Block 0 */
#define SERD_SGMII_CTRL3               0x24U  /* 16b:SerDes/SGMII Control 2 Register in Block 0 */
#define SERD_SGMII_STAT1               0x28U  /* 16b:SerDes/SGMII Status 1 Register (Page 10h ~ 1Fh: Addr 28h ~ 29h) */
#define SERD_SGMII_STAT2               0x2AU  /* 16b:SerDes/SGMII Status 2 Register (Page 10h ~ 1Fh: Addr 2Ah ~ 2Bh) */
#define SERD_SGMII_STAT3               0x2CU  /* 16b:SerDes/SGMII Status 3 Register (Page 10h ~ 1Fh: Addr 2Ch ~ 2Dh) */
#define SERD_BER_CRC_EC                0x2EU  /* 16b:BER/CRC Error Counter Register (Page 10h ~ 1Fh: Addr 2Eh ~ 2Fh) */
#define SERD_PRBS_CTRL                 0x30U  /* 16b:PRBS Control Register (Page 10h ~ 1Fh: Addr 30h ~ 31h) */
#define SERD_PRBS_STAT                 0x32U  /* 16b:PRBS Status Register (Page 10h ~ 1Fh: Addr 32h ~ 33h) */
#define SERD_PG_CTRL                   0x34U  /* 16b:Pattern Generator Control Register (Page 10h ~ 1Fh: Addr 34h ~ 35h) */
#define SERD_PG_STAT                   0x36U  /* 16b:Pattern Generator Status Register (Page 10h ~ 1Fh: Addr 36h ~ 37h) */
#define SERD_FRC_TX1                   0x3AU  /* 16b:Force Transmit 1 Register (Page 10h ~ 1Fh: Addr 3Ah ~ 3Bh) */
#define SERD_FRC_TX2                   0x3CU  /* 16b:Force Transmit 2 Register (Page 10h ~ 1Fh: Addr 3Ch ~ 3Dh) */
#define SERD_BLOCK_ADDR                0x3EU  /* 16b:Block Address Number (Page 10h ~ 1Fh: Addr 3Eh ~ 3Fh) */
/* (PAGE 0x10–0x17) END OF INTERNAL SERDES REGISTERS */

/* (PAGE 0x20–0x27) PAGE页面下所有寄存器的定义 */
#define TX_OCTETS                       0x00U /*64bit*/
#define TX_DROP_PKTS                    0x08U /*32bit*/
#define TX_QOS_PKTS                     0x0CU /*32bit*/
#define TX_BROADCAST_PKTS              0x10U /*32bit*/
#define TX_MULTICAST_PKTS              0x14U /*32bit*/
#define TX_UNICAST_PKTS                0x18U /*32bit*/
#define TX_COLLISIONS                   0x1CU /*32bit*/
#define TX_SINGLE_COLLISIONS           0x20U /*32bit*/
#define TX_MULTIPLE_COLLISIONS         0x24U /*32bit*/
#define TX_DEFERRED_TRANSMIT           0x28U /*32bit*/
#define TX_LATE_COLLISION               0x2CU /*32bit*/
#define TX_EXCESSIVE_COLLISION         0x30U /*32bit*/
#define TX_FRAME_INDISC                 0x34U /*32bit*/
#define TX_PAUSE_PKTS                    0x38U /*32bit*/
#define TX_QOS_OCTETS                    0x3CU /*64bit*/
#define RX_OCTETS                         0x44U /*64bit*/
#define RX_UNDERSIZE_PKTS                0x4CU /*32bit*/
#define RX_PAUSE_PKTS                    0x50U /*32bit*/
#define PKTS_64_OCTETS                    0x54U /*32bit*/
#define PKTS_64TO127_OCTETS              0x58U /*32bit*/
#define PKTS_128TO255_OCTETS             0x5CU /*32bit*/
#define PKTS_256TO511_OCTETS             0x60U /*32bit*/
#define PKTS_512TO1023_OCTETS            0x64U /*32bit*/
#define PKTS_1024TO1522_OCTETS           0x68U /*32bit*/
#define RX_OVERSIZE_PKTS                  0x6CU /*32bit*/
#define RX_JABBER                          0x70U /*32bit*/
#define RX_ALIGNMENT_ERRS                 0x74U /*32bit*/
#define RX_FCS_ERRS                        0x78U /*32bit*/
#define RX_GOOD_OCTETS                     0x7CU /*64bit*/
#define RX_DROP_PKTS                       0x84U /*32bit*/
#define RX_UNICAST_PKTS                    0x88U /*32bit*/
#define RX_MULTICAST_PKTS                  0x8CU /*32bit*/
#define RX_BROADCAST_PKTS                  0x90U /*32bit*/
#define RX_SA_CHANGES                       0x94U /*32bit*/
#define RX_FRAGMENTS                        0x98U /*32bit*/
#define RX_EXCESSSIZE_DISC                 0x9CU /*32bit*/
#define RX_SYMBOL_ERR                       0xA0U /*32bit*/
#define RX_QOS_PKTS                         0xA4U /*32bit*/
#define RX_QOS_OCTETS                       0xA8U /*64bit*/
#define PKTS_1523TO2047_OCTETS             0xB0U /*32bit*/
#define PKTS_2048TO4095_OCTETS             0xB4U /*32bit*/
#define PKTS_4096TO8191_OCTETS             0xB8U /*32bit*/
#define PKTS_8192TO9728_OCTETS             0xBCU /*32bit*/
/* (PAGE 0x20–0x27) END OF REGISTERS */

/* (PAGE 0x30)QoS REGISTERS页面下所有寄存器的定义 */
#define QOS_GB_CTRL                    0x00U  /*  8b:QoS Global Control Register */

#define TX_QUEUE_CTRL                  0x80U  /*  8b:TX Queue Control register */

/* (PAGE 0x30) END OF QoS REGISTERS */

/* (PAGE 0x31) Port-Based VLAN REGISTERS页面下所有寄存器的定义 */
#define PVLAN_PORT(n)                  (0x00U+2*(n))  /* 16b:Port-Based VLAN REGISTERS, Port n */
#define PVLAN_IMP                      0x10U  /* 16b:Port-Based VLAN REGISTERS, IMP */
/* (PAGE 0x31) END of Port-Based VLAN REGISTERS */

/* (PAGE 0x34) IEEE STANDARD 802.1Q VLAN REGISTERS页面下所有寄存器的定义 */
#define VLAN_CTRL0                     0x00U  /*  8b:VLAN Control 0 Register */
#define VLAN_CTRL1                     0x01U  /*  8b:VLAN Control 1 Register */
#define VLAN_CTRL2                     0x02U  /*  8b:VLAN Control 2 Register */
#define VLAN_CTRL3                     0x03U  /*  8b:VLAN Control 3 Register */
#define VLAN_CTRL4                     0x04U  /*  8b:VLAN Control 4 Register */
#define VLAN_CTRL5                     0x05U  /*  8b:VLAN Control 5 Register */

/* (PAGE 0x34) END OF IEEE STANDARD 802.1Q VLAN REGISTERS */






#endif
/******************************* 头文件结束 ***********************************/
