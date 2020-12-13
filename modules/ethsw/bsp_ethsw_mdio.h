/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_spi.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#define MII_CTRL                       0x00  /* MII Control Register : r/w */
#define MII_STAT                       0x01  /* MII Status Register: ro */ 
#define MII_PHY_ID0                    0x02  /* MII PHY ID register: r/w */
#define MII_PHY_ID1                    0x03  /* MII PHY ID register: r/w */
#define MII_ANA                        0x04  /* MII Auto-Neg Advertisement: r/w */
#define MII_ANP                        0x05  /* MII Auto-Neg Link Partner: ro */
#define MII_AN_EXP_REG                 0x06  /* MII Auto-Neg Expansion: ro */
#define MII_GB_CTRL                    0x09  /* MII 1000Base-T control register */
#define MII_GB_STAT                    0x0a  /* MII 1000Base-T Status register */
//#define MII_ESR                        0x0f  /* MII Extended Status register */

/* Non-standard MII Registers */
#define MII_ECR                        0x10  /* MII Extended Control Register */
#define MII_AUX_CTRL_SHADOW            0x18  /* MII AUXILIARY CONTROL SHADOW Reg */
#define MII_ASSR                       0x19  /* MII Auxiliary Status Summary Reg */
#define MII_GSR                        0x1c  /* MII General status (BROADCOM) */
#define MII_MSSEED                     0x1d  /* MII Master/slave seed (BROADCOM) */
#define MII_TEST2                      0x1f  /* MII Test reg (BROADCOM) */
#define MII_ER42					0X42	/*EXPANSION REGISTER 42H: OPERATING MODE STATUS*/

/* MII Control Register: bit definitions */
#define MII_CTRL_SS_MSB                (1 << 6)  /* Speed select, MSb */
#define MII_CTRL_CST                   (1 << 7)  /* Collision Signal test */
#define MII_CTRL_FD                    (1 << 8)  /* Full Duplex */
#define MII_CTRL_RAN                   (1 << 9)  /* Restart Autonegotiation */
#define MII_CTRL_IP                    (1 << 10) /* Isolate Phy */
#define MII_CTRL_PD                    (1 << 11) /* Power Down */
#define MII_CTRL_AE                    (1 << 12) /* Autonegotiation enable */
#define MII_CTRL_SS_LSB                (1 << 13) /* Speed select, LSb */
#define MII_CTRL_LE                    (1 << 14) /* Loopback enable */
#define MII_CTRL_RESET                 (1 << 15) /* PHY reset */

/* MII Extended Control Register (BROADCOM) */
#define MII_ECR_FE                     (1 << 0)  /* FIFO Elasticity */
#define MII_ECR_TLLM                   (1 << 1)  /* Three link LED mode */
#define MII_ECR_ET_IPG                 (1 << 2)  /* Extended XMIT IPG mode */
#define MII_ECR_FLED_OFF               (1 << 3)  /* Force LED off */
#define MII_ECR_FLED_ON                (1 << 4)  /* Force LED on */
#define MII_ECR_ELT                    (1 << 5)  /* Enable LED traffic */
#define MII_ECR_RS                     (1 << 6)  /* Reset Scrambler */
#define MII_ECR_BRSA                   (1 << 7)  /* Bypass Receive Sym. align */
#define MII_ECR_BMLT3                  (1 << 8)  /* Bypass MLT3 Encoder/Decoder */
#define MII_ECR_BSD                    (1 << 9)  /* Bypass Scramble/Descramble */
#define MII_ECR_B4B5B                  (1 << 10) /* Bypass 4B/5B Encode/Decode */
#define MII_ECR_FI                     (1 << 11) /* Force Interrupt */
#define MII_ECR_ID                     (1 << 12) /* Interrupt Disable */
#define MII_ECR_TD                     (1 << 13) /* XMIT Disable */
#define MII_ECR_DAMC                   (1 << 14) /* DIsable Auto-MDI Crossover */
#define MII_ECR_10B                    (1 << 15) /* Reserved*/

/* 1000Base-T Control Register */
#define MII_GB_CTRL_MS_MAN             (1 << 12) /* Manual Master/Slave mode */
#define MII_GB_CTRL_MS                 (1 << 11) /* Master/Slave negotiation mode */
#define MII_GB_CTRL_PT                 (1 << 10) /* Port type */
#define MII_GB_CTRL_ADV_1000FD         (1 << 9)  /* Advertise 1000Base-T FD */
#define MII_GB_CTRL_ADV_1000HD         (1 << 8)  /* Advertise 1000Base-T HD */

/* MII Link Advertisment */
#define MII_ANA_ASF                    (1 << 0)  /* Advertise Selector Field */
#define MII_ANA_HD_10                  (1 << 5)  /* Half duplex 10Mb/s supported */
#define MII_ANA_FD_10                  (1 << 6)  /* Full duplex 10Mb/s supported */
#define MII_ANA_HD_100                 (1 << 7)  /* Half duplex 100Mb/s supported */
#define MII_ANA_FD_100                 (1 << 8)  /* Full duplex 100Mb/s supported */
#define MII_ANA_T4                     (1 << 9)  /* T4 */
#define MII_ANA_PAUSE                  (1 << 10) /* Pause supported */
#define MII_ANA_ASYM_PAUSE             (1 << 11) /* Asymmetric pause supported */
#define MII_ANA_RF                     (1 << 13) /* Remote fault */
#define MII_ANA_NP                     (1 << 15) /* Next Page */
#define MII_ANA_ASF_802_3              (1)       /* 802.3 PHY */

#define MII_CTRL_REG            0x00    /* MII Control Register : r/w */
#define MII_STAT_REG            0x01    /* MII Status Register: ro */ 
#define MII_PHY_ID0_REG         0x02    /* MII PHY ID register: r/w */
#define MII_PHY_ID1_REG         0x03    /* MII PHY ID register: r/w */
#define MII_ANA_REG             0x04    /* MII Auto-Neg Advertisement: r/w */
#define MII_ANP_REG             0x05    /* MII Auto-Neg Link Partner: ro */
#define MII_AN_EXP_REG          0x06    /* MII Auto-Neg Expansion: ro */
#define MII_GB_CTRL_REG         0x09    /* MII 1000Base-T control register */
#define MII_GB_STAT_REG         0x0a    /* MII 1000Base-T Status register */
#define MII_ESR_REG             0x0f    /* MII Extended Status register */

#define MII_ECR                        0x10  /* MII Extended Control Register */
#define MII_ESR                        0x11  /* MII Extended Status Register */
#define MII_REC                        0x12 
#define MII_FCSC                       0x13 
#define MII_RNC                        0x14 

#define MII_AUX_CTRL_SHADOW        0x18  /* MII AUXILIARY CONTROL SHADOW Reg */
#define MII_AUX_STATUS_SUMMARY     0x19




/* MII Control Register: bit definitions */
#define MII_CTRL_SS_MSB                (1 << 6)  /* Speed select, MSb */
#define MII_CTRL_CST                   (1 << 7)  /* Collision Signal test */
#define MII_CTRL_FD                    (1 << 8)  /* Full Duplex */
#define MII_CTRL_RAN                   (1 << 9)  /* Restart Autonegotiation */
#define MII_CTRL_IP                    (1 << 10) /* Isolate Phy */
#define MII_CTRL_PD                    (1 << 11) /* Power Down */
#define MII_CTRL_AE                    (1 << 12) /* Autonegotiation enable */
#define MII_CTRL_SS_LSB                (1 << 13) /* Speed select, LSb */
#define MII_CTRL_LE                    (1 << 14) /* Loopback enable */
#define MII_CTRL_RESET                 (1 << 15) /* PHY reset */

/*
 * 1000Base-T Control Register
 */
#define MII_GB_CTRL_MS_EN        (1 << 12) 
#define MII_GB_CTRL_MS_VALUE     (1 << 11) 
#define MII_GB_CTRL_PT            (1 << 10) /* Port type */
#define MII_GB_CTRL_ADV_1000FD   (1 << 9) /* Advertise 1000Base-T FD */
#define MII_GB_CTRL_ADV_1000HD   (1 << 8) /* Advertise 1000Base-T HD */



s32 ethsw_read_reg(u8 u8RegPage, u8 u8RegAddr, u8 *pu8Buf, u8 u8Len);
s32 ethsw_write_reg(u8 u8RegPage, u8 u8RegAddr, const u8 *pu8Buf, u8 u8Len);


/******************************* 头文件结束 ***********************************/
