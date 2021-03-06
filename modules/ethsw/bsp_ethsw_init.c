/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_init.c 
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
#include "bsp_ethsw_init.h"
#include "bsp_ethsw_mdio.h"

/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 **************************/
u16 g_pVlan[10] = {0x1ff,0x100,0x100,0x100,0x1ff,0x1ff,0xff,0x1ff,0x1ff,0x1ff};
u32 g_u32InitFlag = 0x0;
extern u32 g_u32PrintRules;

/************************** 局部常数和类型定义 ************************/

/*************************** 局部函数原型声明 **************************/

/************************************ 函数实现 ************************* ****/
/*******************************************************************************
* 函数名称: ethsw_set_switch_mode
* 函数功能: 设置端口的交换功能,Enable/Disable芯片转发功能
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        是否设置芯片的转发功能,0:关闭,1:打开
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
s32 ethsw_set_switch_mode(u8 u8Enable)
{

    STRU_ETHSW_SWITCH_MODE  struSwitchMode; /* 定义交换模式结构 */
    /* Read-Modified-Write */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    struSwitchMode.SwFwdEn = u8Enable;
    struSwitchMode.SwFwdMode = 1-u8Enable;//1 - u8Enable; modify by huangjl
    /* 写交换模式寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    

	printf("struSwitchMode.SwFwdMode:%d\n",struSwitchMode.SwFwdMode);
	printf("struSwitchMode.SwFwdEn:%d\n",struSwitchMode.SwFwdEn);
    
    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_arl_multicast
* 函数功能: 设置ARL表是否支持Multicast功能
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        设置芯片支持组播功能,0:不支持组播,1:支持组播
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
s32 ethsw_set_arl_multicast(u8 u8Enable)
{
    STRU_ETHSW_MCAST_CTRL  struMcastCtrl;    /* 定义组播控制结构 */

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    struMcastCtrl.MulticastEn = u8Enable; /* 修改此位 */

    /* 写组播控制寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_set_dlf_forward
* 函数功能: 对单播和组播DLF(目的地址查找失败)包的转发进行设置
* 相关文档:
* 函数参数:
* 参数名称:       类型        输入/输出   描述
* u8MultiDlfEn    u8          输入        1:组播包DLF包按照Multicast Lookup Fail
                                            Forward Map register设置进行转发
                                          0:组播包DLF包广播(flood)
* u32MultiPortMap u32         输入        u8MultiDlfEn为1时组播表DLF包的转发PortMap
* u8UniDlfEn      u8          输入        1:单播包DLF包按照Multicast Lookup Fail
                                            Forward Map register设置进行转发
                                          0:单播包DLF包广播(flood)
* u32UniPortMap   u32         输入        u16UniPortMap为1时单播表DLF包的转发PortMap
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
s32 ethsw_set_dlf_forward(u8 u8MultiDlfEn, u16 u16MultiPortMap, u8 u8UniDlfEn, u16 u16UniPortMap)
{
    u16                    u16Temp;
    STRU_ETHSW_MCAST_CTRL  struMcastCtrl; /* 定义组播控制结构 */

    /* 只需要在使能组播表DLF转发规则时才需要设置MULTICAST LOOKUP FAIL FORWARD MAP寄存器 */
    if (1 == u8MultiDlfEn)
    {
        u16Temp = u16MultiPortMap;
    }
    else
    {
        u16Temp = 0;
    }
    /* 写MULTICAST LOOKUP FAIL FORWARD MAP寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, MUL_DLF_FW_MAP, (u8 *)&u16Temp, TWO_BYTE_WIDTH));

    /* 只需要在使能单播表DLF转发规则时才需要设置UNICAST LOOKUP FAIL FORWARD MAP寄存器 */
    if (1 == u8UniDlfEn)
    {
        u16Temp = u16UniPortMap;
    }
    else
    {
        u16Temp = 0;
    }
    /* 写UNICAST LOOKUP FAIL FORWARD MAP寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, UNI_DLF_FW_MAP, (u8 *)&u16Temp, TWO_BYTE_WIDTH));

    /* Read-Modified-Write */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    /* 对组播/单播转发规则使能标记进行设置 */
    struMcastCtrl.MlfFmEn = u8MultiDlfEn;
    struMcastCtrl.UniFmEn = u8UniDlfEn;
    /* 写组播控制寄存器 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, (u8 *)&struMcastCtrl, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_set_port_stat
* 函数功能: 设置端口状态
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8PortId        u8          输入        端口ID
* u8PhyScanEn     u8          输入        使能标志
* u8OverrideEn    u8          输入        u8Override使能标志
* u8TxFlowEn      u8          输入        Tx流控使能标志
* u8RxFlowEn      u8          输入        Rx流控使能标志
* u8Speed         u8          输入        端口速率
* u8Duplex        u8          输入        双工状态
* u8LinkState     u8          输入        连接状态

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
s32 ethsw_set_port_stat(u8 u8PortId, u8 u8PhyScanEn, u8 u8OverrideEn, u8 u8TxFlowEn, u8 u8RxFlowEn, u8 u8Speed, u8 u8Duplex, u8 u8LinkState)
{
    STRU_ETHSW_PORT_STAT struPortStat;
    struPortStat.PhyScanEn = u8PhyScanEn;
    struPortStat.OverrideEn = u8OverrideEn; /* 采用此寄存器中设置的速率,双工和LINK状态 */
    struPortStat.TxFlowEn = u8TxFlowEn;   /* 发送流控使能 */
    struPortStat.RxFlowEn = u8RxFlowEn;   /* 接收流控使能 */
    struPortStat.Speed = u8Speed;      /* 速率 */
    struPortStat.Duplex = u8Duplex;    /* 双工状态 */
    struPortStat.LinkState = u8LinkState; 
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, MII_PORT_STATE_OR(u8PortId), (u8 *)&struPortStat, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}

#if 0
s32 ethsw_force_port_up(u8 u8PortId)
{
    RETURN_IF_ERROR(ethsw_set_port_stat(u8PortId,0,1,1,1,2,1,1));
}
#endif
/*******************************************************************************
* 函数名称: ethsw_set_phy_scan
* 函数功能: 对交换芯片的PHY 自动扫描功能进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
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
s32 ethsw_set_phy_scan(void)
{
    u8   u8PortId;

    for(u8PortId = 0; u8PortId < 8; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_set_port_stat(u8PortId,1,0,0,0,0,0,0));
    }

    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_arl_hash
* 函数功能: 设置ARL表的配置,HASH_DISABLE,芯片查找ARL表时是先计算HASH INDEX,置0
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Val         u8          输入        设置的值
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
s32 ethsw_set_arl_hash(u8 u8Val)
{
    u8   u8Temp;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, &u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Val)
    {
        u8Temp &= 0xfe; /* 将最低一位置为0 */
    }
    else
    {
        u8Temp |= 0x1;/* 将最低一位置为1 */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, &u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_qos
* 函数功能: 对芯片QoS寄存器进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        使能或止能QoS功能
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
s32 ethsw_set_qos(u8 u8Enable)
{
    u8   u8Temp;
    /* 设置QoS使能寄存器 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, QOS_GB_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0xbf;  /* Bit6设为0,Port Base QOS Disable */
    }
    else
    {
        u8Temp |= 0x40;  /* Bit6设为1,Port Base QOS En  */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QOS_PAGE, QOS_GB_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    /* 设置TxQ控制寄存器 */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QOS_PAGE, TX_QUEUE_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0xf3;  /* Bit3:2设为0,QOS_MODE b00: Singe Queue (No QoS) b01: Two Queues Mode
                                                 b10: Three Queues Mode    b11: Four Queues Mode */
    }
    else
    {
        u8Temp |= 0x0c;  /* Bit3:2设为11,QOS_MODE Four Queues */
    }
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QOS_PAGE, TX_QUEUE_CTRL, (u8 *)&u8Temp, ONE_BYTE_WIDTH));
    return (E_ETHSW_OK);
}
/*******************************************************************************
* 函数名称: ethsw_set_pvlan
* 函数功能: 对PVLAN寄存器进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
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
s32 ethsw_set_pvlan(void)
{
    u8    u8PortId;    /* 端口ID */
    u16   u16VlanMask; /* VLAN MASK */
    u8    u8i;
    u16VlanMask = 0x1ff;/* 所有的端口都属于同一个PVLAN */
    g_pVlan[0] = u16VlanMask;
    for(u8i = 1; u8i < 10; u8i++)
    {
        g_pVlan[u8i] = 0x100;
    }
    for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
    {
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortId), (u8 *)&u16VlanMask, TWO_BYTE_WIDTH));
    }
    return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_add_port_vlan
* 函数功能: 将端口添加到指定的Vlan中
* 相关文档:
* 函数参数:
* 参数名称:     类型                    输入/输出   描述
* u8ChipId      u8                      输入        芯片ID(single should be 0)
* u8PortId      u8                      输入        端口号
* u8VlanId      u8                      输入        指定的Vlan号
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
s32 ethsw_add_port_vlan(u8 u8PortId, u8 u8VlanId)
{
    u8    u8PortIdtemp;
    u32   u32VlanMask;
    u8    u8i;
    u32VlanMask = 0x1ffff;
    g_pVlan[0] = g_pVlan[0] & (~(u32)(1 << u8PortId));
    g_pVlan[u8VlanId] = g_pVlan[u8VlanId] | (1 << u8PortId);
    for (u8PortIdtemp = 0; u8PortIdtemp < MAX_USED_PORTS; u8PortIdtemp++)
    {
        for(u8i = 0;u8i < 10; u8i++)
        {
            if(0 != ((1 << u8PortIdtemp) & g_pVlan[u8i]))
            {
                u32VlanMask = g_pVlan[u8i];
                break;
            }
        }
        if(u8i == 10)
        {
            return (E_ETHSW_FAIL);
        }
        RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortIdtemp), (u8 *)&u32VlanMask, FOUR_BYTE_WIDTH));
    }

    return (E_ETHSW_OK);
}

  s32 ethsw_set_vlan(u8 u8PortId1, u16 u16Vlan)
{
	
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)(2*u8PortId1), (u8 *)&u16Vlan, TWO_BYTE_WIDTH));
	

	return (E_ETHSW_OK);

}



  s32 ethsw_set_port_vlan(u8 u8PortId1, u8 u8PortId2)
{
	u16 u16Vlan;

	u16Vlan = u16Vlan | (1<<u8PortId1);
	u16Vlan = u16Vlan | (1<<u8PortId2);
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)(2*u8PortId1), (u8 *)&u16Vlan, TWO_BYTE_WIDTH));
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_PVLAN_PAGE, (u8)(2*u8PortId2), (u8 *)&u16Vlan, TWO_BYTE_WIDTH));

	return (E_ETHSW_OK);

}
/*******************************************************************************
* 函数名称: ethsw_set_qvlan
* 函数功能: 对PVLAN寄存器进行配置
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u8Enable      u8          输入        0:Disable VLAN,在计算HASH时,不使用VID
*                                       1:Enable VLAN
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
s32 ethsw_set_qvlan(u8 u8Enable)
{
    u8   u8Temp;
	u32  u32Temp;
	u16  u16Temp;
	int i =0;
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, &u8Temp, ONE_BYTE_WIDTH));
    if (0 == u8Enable)
    {
        u8Temp &= 0x1f; /* 将最高3位置为0 */
    }
    else
    {
        u8Temp |= 0xe0; /* 将最高3位置为1 */
    }

	u8Temp |=0x80;//enable the IEEE 802.1Q vlan
    
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, &u8Temp, ONE_BYTE_WIDTH));

//----------------------------
    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x63, (u8*)&u32Temp, FOUR_BYTE_WIDTH));
	printf("u32Temp:0x%x\n", u32Temp);
//	u32Temp |=0x7ffff;
	u32Temp =0x401ff;
    RETURN_IF_ERROR(ethsw_write_reg(0x05, 0x63, (u8*)&u32Temp, FOUR_BYTE_WIDTH));
    
//----------------------------
    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x61, (u8*)&u16Temp, TWO_BYTE_WIDTH));

	printf("u16Temp:0x%x\n", u16Temp);
	u16Temp =0x0;
    RETURN_IF_ERROR(ethsw_write_reg(0x05, 0x61, (u8*)&u16Temp, TWO_BYTE_WIDTH));


//-------set bit 0 to 0:write op---------------
    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x60, (u8*)&u8Temp, ONE_BYTE_WIDTH));

	printf("u8Temp:0x%x\n", u8Temp);
	u8Temp &=0xfe;
    RETURN_IF_ERROR(ethsw_write_reg(0x05, 0x60, (u8*)&u8Temp, ONE_BYTE_WIDTH));
    
//-------set bit 7 to 1:start write op---------------
    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x60, (u8*)&u8Temp, ONE_BYTE_WIDTH));

	printf("u8Temp:0x%x\n", u8Temp);
	u8Temp |=0x80;
    RETURN_IF_ERROR(ethsw_write_reg(0x05, 0x60, (u8*)&u8Temp, ONE_BYTE_WIDTH));
    
#if 0
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL2, &u8Temp, ONE_BYTE_WIDTH));

    u8Temp |= 0x0c; /* 将最高3,2位设置为1 */

    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL2, &u8Temp, ONE_BYTE_WIDTH));
#endif


    return (E_ETHSW_OK);
}


s32 ethsw_show_qvlan()
{
    u8   u8Temp;
	u32  u32Temp;
	u16  u16Temp;

    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x61, (u8*)&u16Temp, TWO_BYTE_WIDTH));

	printf("u16Temp:0x%x\n", u16Temp);
	u16Temp =0x0;
    RETURN_IF_ERROR(ethsw_write_reg(0x05, 0x61, (u8*)&u16Temp, TWO_BYTE_WIDTH));

//-------set bit 0 to 1:read op---------------
    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x60, &u8Temp, ONE_BYTE_WIDTH));

	printf("u8Temp:0x%x\n", u8Temp);
	u8Temp |=0x1;
    RETURN_IF_ERROR(ethsw_write_reg(0x05, 0x60, &u8Temp, ONE_BYTE_WIDTH));

//-------set bit 7 to 1:start write op---------------
    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x60, &u8Temp, ONE_BYTE_WIDTH));

	printf("u8Temp:0x%x\n", u8Temp);
	u8Temp |=0x80;
    RETURN_IF_ERROR(ethsw_write_reg(0x05, 0x60, &u8Temp, ONE_BYTE_WIDTH));


 //----------------------------
    RETURN_IF_ERROR(ethsw_read_reg(0x05, 0x63, (u8*)&u32Temp, FOUR_BYTE_WIDTH));
	printf("u32Temp:0x%x\n", u32Temp);
   
    return (E_ETHSW_OK);
}



/*******************************************************************************
* 函数名称: ethsw_set_aging_time
* 函数功能: 设置芯片的老化时间(ARL表的老化时间(由于采用静态ARL表所以无配置要求),
*           也是包的老化时间)
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
* u32Age_Time   u32         输入        此寄存器应当设置的Aging值
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

s32 ethsw_set_aging_time(u32 u32AgeTime)
{
    u32 u32buf = u32AgeTime;   /* 直接设置老化时间 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, AGING_TIME_CTRL, (u8 *)&u32buf, FOUR_BYTE_WIDTH));
    return (E_ETHSW_OK);
}


s32 ethsw_imp_init(void)
{
	u8 writevals = 0x1c;
	u8 readval;
	
//----------configure IMP CTRL----------------------------------
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, IMP_TRAFIC_CTRL, (u8 *)&readval, ONE_BYTE_WIDTH));
	LOG("readval:0x%x\n", readval);
	LOG("writevals:0x%x\n", writevals);
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, IMP_TRAFIC_CTRL, (u8 *)&writevals, ONE_BYTE_WIDTH));

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, IMP_TRAFIC_CTRL, (u8 *)&readval, ONE_BYTE_WIDTH));

	LOG("readval:0x%x\n", readval);
	
#if 0
//--------------------------------------------------------
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, 0x0E, (u8 *)&readval, ONE_BYTE_WIDTH));
	LOG("readval:0x%x\n", readval);
//---------------IMP RGMII Control Register----------------
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, 0x60, (u8 *)&readval, ONE_BYTE_WIDTH));
	LOG("readval:0x%x\n", readval);
	readval |=0x3;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, 0x60, (u8 *)&readval, ONE_BYTE_WIDTH));

//-------Global Management Configuration Register-----------------    
    RETURN_IF_ERROR(ethsw_read_reg(0x02, 0x00, (u8 *)&readval, ONE_BYTE_WIDTH));
	LOG("readval:0x%x\n", readval);
	readval &=0x7f;
    RETURN_IF_ERROR(ethsw_write_reg(0x02, 0x00, (u8 *)&readval, ONE_BYTE_WIDTH));
#endif
	return APP_OK;
}


/*******************************************************************************
* 函数名称: ethsw_bcm5389_init
* 函数功能: 初始化BCM5389芯片
* 相关文档:
* 函数参数:
* 参数名称:     类型        输入/输出   描述
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
/* 端口状态强制设置结构 */
typedef struct tag_STRU_ETHSW_IMP_STAT
{
    u8 Mii_swor:1, 
       Reserve:1,
       Tx_Flow_Ctl_cap:1,   /*Software TX flow control enable */
       Rx_Flow_Ctl_cap:1,   /* Software RX flow control enable */
       Speed:2,      /*Software port speed setting:b00:10M,b01:100M,b10:1000M*/
       Duplex:1,     /*Software duplex mode setting:0: Half-duplex 1: Full-duplex*/
       LinkState:1;  /*0:Link down 1:Link up*/
} STRU_ETHSW_IMP_STAT;

s32 setIMPSpeed(u8 Mii_swor, u8 Tx_Flow_Ctl_cap, u8 Rx_Flow_Ctl_cap, u8 u8Speed, u8 u8Duplex, u8 u8LinkState)
{
	STRU_ETHSW_IMP_STAT sta;
	
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, 0x0e,(u8 *)&sta, ONE_BYTE_WIDTH));
    sta.Mii_swor 	= 	Mii_swor;
	sta.Speed 		= 	u8Speed;
	sta.Duplex		= 	u8Duplex;
	sta.LinkState	= 	u8LinkState;
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, 0x0e,(u8 *)&sta, ONE_BYTE_WIDTH));
}

s32 ethsw_bcm5389_init(void)
{
    u8   u8PortId;
    
    if (0 != (g_u32InitFlag & ETHSW_CHIP_INIT_FLAG))
    {
        (void)printf("CHIP has been initialized!\n");
        return (E_ETHSW_CHIP_REINIT);
    }
    
#if 0
    /* 3.打开ARL表支持Multicast功能 */
    RETURN_IF_ERROR(ethsw_set_arl_multicast(1));

    /* 4.对单播和组播DLF包的转发进行配置:单播组播DLF包都转发到所有端口 */
    RETURN_IF_ERROR(ethsw_set_dlf_forward(0, 0x00, 0, 0x00));


    /* 5.强制设置各端口的状态,DSP各端口需要强制设置 */
    for (u8PortId = 0; u8PortId < 8; u8PortId++)
    {
        //RETURN_IF_ERROR(ethsw_force_port_up(u8PortId));
        RETURN_IF_ERROR(ethsw_set_port_stat(u8PortId,0,1,1,1,2,1,1));//modify by huangjl-->速率设置成100M
    }
	
    /* 7.配置ARL配置寄存器,HASH_ENABLE */
    RETURN_IF_ERROR(ethsw_set_arl_hash(0));
    /* 8.配置QoS控制寄存器,Disable QoS功能 */
    RETURN_IF_ERROR(ethsw_set_qos(0));
    /* 9.配置PVLAN */
    RETURN_IF_ERROR(ethsw_set_pvlan());
    /* 10.配置VLAN控制寄存器*/
    RETURN_IF_ERROR(ethsw_set_qvlan(0));
#endif
	setIMPSpeed(1,0,0,2,1,1 );

    /* 11.配置AGING TIME控制寄存器,设置老化时间,设置为不老化*/
    RETURN_IF_ERROR(ethsw_set_aging_time(0));

	ethsw_imp_init();
    /* 芯片初始化完毕,设置标记 */
    
    g_u32InitFlag |= ETHSW_CHIP_INIT_FLAG;
    
    return (E_ETHSW_OK);
}



