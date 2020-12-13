/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_port.c 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>

/**************************** 私用头文件* **********************************/
#include "bsp_types.h"
#include "bsp_ethsw_bcm5389.h"
#include "bsp_ethsw_port.h"
#include "bsp_ethsw_mdio.h"

/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 **************************/
extern u32 g_u32PrintRules;  /* 打印规则 */


/************************** 局部常数和类型定义 ************************/



/*************************** 局部函数原型声明 **************************/

/************************************ 函数实现 ************************* ****/

/*******************************************************************************
* 函数名称: ethsw_set_status
* 函数功能: 完成端口接受和发送功能的设置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8PortId      u8          输入        端口ID
* u8TxEnable    u8          输入        发送使能
* u8RxEnable    u8          输入        接收使能
*
* 返回值:   (各个返回值的注释)
* 函数类型: <回调、中断、可重入（阻塞、非阻塞）等函数必须说明类型及注意事项>
* 函数说明:（以下述列表的方式说明本函数对全局变量的使用和修改情况，以及本函数
*           未完成或者可能的改动）
*
* 1.被本函数改变的全局变量列表
* 2.被本函数改变的静态变量列表
* 3.引用但没有被改变的全局变量列表
* 4.引用但没有被改变的静态变量列表
*
* 修改日期    版本号   修改人  修改内容
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_set_status(u8 u8PortId, u8 u8TxEnable, u8 u8RxEnable)
{
    STRU_ETHSW_PORT_CTRL  struPortCtrl; /* 端口控制寄存器结构 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                   (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));
    struPortCtrl.StpState = 5; /* 0:unmanaged mode, do not use STP;101 = Forwarding State */
    /* 此寄存器为收发功能Disable寄存器,打开时寄存器设0,关闭时设1 */
    struPortCtrl.TxDisable = (1 - u8TxEnable);  /* 设置发送bit */
    struPortCtrl.RxDisable = (1 - u8RxEnable);  /* 设置接收bit */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                    (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));/* 重新写入 */
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_get_status
* 函数功能: 完成获取端口LINK状态的功能
* 相关文档:
* 函数参数:
* 参数名称:       类型        输入/输出   描述
* u8PortId        u8          输入        端口ID
* pu8PortStatus   u8 *        输出        端口状态
*
* 返回值:   (各个返回值的注释)
* 函数类型: <回调、中断、可重入（阻塞、非阻塞）等函数必须说明类型及注意事项>
* 函数说明:（以下述列表的方式说明本函数对全局变量的使用和修改情况，以及本函数
*           未完成或者可能的改动）
*
* 1.被本函数改变的全局变量列表
* 2.被本函数改变的静态变量列表
* 3.引用但没有被改变的全局变量列表
* 4.引用但没有被改变的静态变量列表
*
* 修改日期    版本号   修改人  修改内容
* -----------------------------------------------------------------
*******************************************************************************/
s32 ethsw_get_status(u8 u8PortId, u8 *pu8PortStatus)
{
    u8   u8Val[8];         /* 局部变量 */
    STRU_ETHSW_PORT_CTRL    struPortCtrl;   /* 端口控制寄存器结构 */
    STRU_ETHSW_SWITCH_MODE  struSwitchMode; /* 交换模式寄存器结构 */

    /* 读取芯片端口收发控制寄存器的值 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                   (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));
    /* 此寄存器为收发功能Disable寄存器,打开时寄存器设0,关闭时设1 */
    if (0 != struPortCtrl.TxDisable)
    {
        /* PORT TX FAIL */
        if (NULLPTR == pu8PortStatus) /*如果为空则进行打印*/
        {
            printf("\nPort[%d] Tx Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN; /* 否则进行记录 */
        }
        return (E_ETHSW_OK); /* 直接返回 */
    }

    if (0 != struPortCtrl.RxDisable)
    {
        /* PORT RX FAIL */
        if (NULLPTR == pu8PortStatus) /*如果为空则进行打印*/
        {
            printf("\nPort[%d] Rx Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN; /* 否则进行记录 */
        }
        return (E_ETHSW_OK);   /* 直接返回 */
    }

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    if (0 == struSwitchMode.SwFwdEn)
    {
        if (NULLPTR == pu8PortStatus)/*如果为空则进行打印*/
        {
            printf("\nPort[%d] FWD Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN;  /* 否则进行记录 */
        }
        return (E_ETHSW_OK); /* 直接返回 */
    }

    /* 读取端口LINK状态寄存器 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_SUMMARY, u8Val, TWO_BYTE_WIDTH));
    //printf("ETHSW_STAT_PAGE LINK_STATUS_SUMMARY is <0x%02x%02x>\n", (s32)u8Val[0], (s32)u8Val[1]);
    if(0 != (u8Val[0]&(1<<u8PortId)))
    {
                /* PORT OK */
            if (NULLPTR == pu8PortStatus) /*如果为空则进行打印*/
            {
                printf("\nPort[%d] is Link Up!\n", u8PortId);
            }
            else
            {
                *pu8PortStatus = PORT_LINK_UP; /* 否则进行记录 */
            }
    }
    else
    {
            /* PORT FAIL */
            if (NULLPTR == pu8PortStatus)/*如果为空则进行打印*/
            {
                printf("\nPort[%d] is Link Down!\n", u8PortId);
            }
            else
            {
                *pu8PortStatus = PORT_LINK_DOWN;  /* 否则进行记录 */
            }
    }
    return (E_ETHSW_OK); /* 最后返回 */
}

s32 ethsw_set_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort)
{
    STRU_ETHSW_MIRROR_CTRL      struMirrorCtrl;         /* Mirror出入端流量控制功能 */
    STRU_ETHSW_MIRROR_CAP_CTRL  struMirrorCapCtrl;      /* Mirror捕获控制结构 */

    /* 仅在使能MIRROR功能时才需要配置Ingress/Egress Mirror控制寄存器  */
    if (1 == u8MirrorEnable)
    {
        struMirrorCtrl.MirroredPort = (1 << u8MirroredPort);
        struMirrorCtrl.filter = 0x0;
        struMirrorCtrl.DividEnable = 0x0;  /* 不进行流量划分 */
        struMirrorCtrl.rsvd4 = 0;          /* 保留字段设为0 */

        	
        /* 其它和Trunking功能相关的寄存器,可以提供只Mirror某MAC地址,以一定间隔Mirror等功能 */
        /* INGRESS_MIRROR_DIVIDER, INGRESS_MIRROR_MAC_ADDR,EGRESS_MIRROR_DIVIDER,EGRESS_MIRROR_MAC_ADDR */
        if (0x1 == u8MirrorRules) /* INGRESS方向 */
        {
            /* 设置Ingress Mirror控制寄存器 */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
            struMirrorCtrl.MirroredPort = 0;
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
        }
        else if (0x2 == u8MirrorRules)/* EGRESS方向 */
        {
            /* 设置Egress Mirror控制寄存器 */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
            struMirrorCtrl.MirroredPort = 0;
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
        }
        else if (0x3 == u8MirrorRules) /* 双方向 */
        {
            /* 设置Egress Mirror控制寄存器 */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
        }
    }

    /* 设置Mirror Capture端口,止能或使能Mirror功能 */
    struMirrorCapCtrl.BlkNoMirror = 0;               /* 不阻塞非Mirror数据 */
    struMirrorCapCtrl.MirrorEnable = u8MirrorEnable;     /* Mirror功能止能或使能 */
    struMirrorCapCtrl.CapturePort =u8MirrorPort;// (1 << u8MirrorPort); /* 设置捕获端口 */
    struMirrorCapCtrl.rsvd10 = 0;                         /* 保留字段设为0 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, (u8 *)&struMirrorCapCtrl, TWO_BYTE_WIDTH));
    return (E_ETHSW_OK);
}



