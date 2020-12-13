/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           Bsp_ethsw_arl.c 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <string.h>

/**************************** 私用头文件* **********************************/
#include "bsp_types.h"
#include "bsp_ethsw_bcm5389.h"
#include "bsp_ethsw_arl.h"
#include "bsp_ethsw_mdio.h"

/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 **************************/
extern u32 g_u32PrintRules;

/************************** 局部常数和类型定义 ************************/



/*************************** 局部函数原型声明 **************************/

/************************************ 函数实现 ************************* ****/
/*******************************************************************************
* 函数名称: save_arl_struct
* 函数功能: 根据ARL_TABLE_MAC_VID_ENTRY0/1,ARL_TABLE_DATA_ENTRY0/1,
*           ARL_TABLE_SEARCH_MAC_RESULT/ARL_TABLE_SEARCH_DATA_RESULT寄存器中读出
*           的相关信息,转化为STRU_ETHSW_ARL_TABLE结构中相关值
* 相关文档:
* 函数参数:
* 参数名称:        类型                           输入/输出   描述
* pstruMacEntry    STRU_ETHSW_ARL_MAC_VID_ENTRY*  输入        从MAC_VID/MAC_RESULT
                                                              寄存器读出的内容(8字节)
* pstruDataEntry   STRU_ETHSW_ARL_DATA_ENTRY*     输入        从DATA寄存器读出的内
                                                              容(2字节)
* pstruArlTable    STRU_ETHSW_ARL_TABLE*          输出        将输入信息转化后保存
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
void save_arl_struct(const STRU_ETHSW_ARL_MAC_VID *pstruMacEntry, const STRU_ETHSW_ARL_DATA *pstruDataEntry, STRU_ETHSW_ARL_TABLE *pstruArlTable)
{
	memcpy(pstruArlTable->u8MacAddr, pstruMacEntry->u8MacAddr, 6);
	pstruArlTable->valid = pstruDataEntry->valid;
	pstruArlTable->VlanId = pstruMacEntry->VlanId;
	pstruArlTable->StaticEntry = pstruDataEntry->StaticEntry;
	pstruArlTable->age = pstruDataEntry->age;
	pstruArlTable->PortId = pstruDataEntry->PortId;
	return;
	
}
/*******************************************************************************
* 函数名称: ethsw_lookup_arl_entry
* 函数功能: 查找指定MAC地址和VLAN ID的ARL表,并将查找出来的ARL表保存到输出变量中
* 相关文档:
* 函数参数:
* 参数名称:      类型                    输入/输出  描述
* pu8MacAddr     u8*                     输入       将要读取的mac地址数组的首地址
* pstruMacEntry  STRU_ETHSW_ARL_MAC_VID* 输出       将查找到的ARL表的MAC地址VID存
                                                    放在此结构中,,第一条对应于ENTRY0,
                                                    第二条对应于条对应于ENTRY1
* pstruDataEntry STRU_ETHSW_ARL_DATA*    输出       将查找到的ARL表的PortId有效静态
                                                    等信息存放在此结构中,,第一条对
                                                    应于ENTRY0,第二条对应于条对应于
                                                    ENTRY1
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
s32 ethsw_lookup_arl_entry(const u8 *pu8MacAddr, STRU_ETHSW_ARL_MAC_VID *pstruMacEntry, STRU_ETHSW_ARL_DATA *pstruDataEntry)
{
	u32   u32i;              /* 循环变量 */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;  /* 临时变量,查找时的控制结构 */

	if ((NULLPTR == pu8MacAddr) || (NULLPTR == pstruMacEntry) || (NULLPTR == pstruDataEntry))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_lookup_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}
	
	/* Step 1:设置MAC地址index,6字节 */
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, MAC_ADDRESS_INDEX, pu8MacAddr, SIX_BYTE_WIDTH));

	/* Step 2:设置VLAN ID index,2字节,由于在初始化时Disable VLAN,所以不需要此行代码 */
	/* RETURN_IF_ERROR(ethsw_write_reg(u8ChipId, ETHSW_ARL_ACCESS_PAGE, VLAN_ID_INDEX, ?, TWO_BYTE_WIDTH)); */

	/* Step 3,4:设置读标记,读开始;为了防止开始的是写操作,先设置bit0,可能不需要这样 */
	struArlRWCtrl.ArlStart = 1;/* 开始读写标记 */
	struArlRWCtrl.ArlRW = 1;   /* 读标记 */
	struArlRWCtrl.rsvd = 0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	/* 轮询读操作是否结束 */
	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	
	if (u32i >= ETHSW_TIMEOUT_VAL) /* 超时 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_lookup_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	else  /* 没有超时,将2个Entry的内容都保存起来 */
	{
		/* 读取MAC_VID_ENRTY0的内容,读取8字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)pstruMacEntry, EIGHT_BYTE_WIDTH));
		/* 读取DATA_ENRTY0的内容,读取2字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)pstruDataEntry, TWO_BYTE_WIDTH));

		/* 读取MAC_VID_ENRTY1的内容,读取8字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)(pstruMacEntry + 1), EIGHT_BYTE_WIDTH));
		/* 读取DATA_ENRTY1的内容,读取2字节内容 */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)(pstruDataEntry + 1), TWO_BYTE_WIDTH));
	}
	return (E_ETHSW_OK);
	
}

/*******************************************************************************
* 函数名称: ethsw_add_arl_entry
* 函数功能: 添加一条ARL表:将指定MAC地址(VLAN ID)的ARL表写入到芯片内部集成Memory中
* 相关文档:
* 函数参数:
* 参数名称:      类型                    输入/输出  描述
* u8CastType     u8                      输入        单播组播标记,0:单播,1:组播
* pstruMacPort   STRU_ETHSW_MAC_PORT*    输入        将此结构对应MAC地址与端口插
                                                     入到ARL表中
*
* 返回值:  s32
*          0:成功
*          其它错误
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
s32 ethsw_add_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	s32                     s32Rv1;
	s32                     s32Rv2;           /* 返回值 */
	u32                     u32i;             /* 循环变量 */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;    /* 标记,当为1时表示写相应Entry寄存器 */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];  /* 临时变量,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2]; /* 临时变量,DATA Entry */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;    /* 临时变量,ARL表读写操作结构 */

	if (NULLPTR == pstruMacPort)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_add_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* 清0临时变量 */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	/* Step1:查找相关的ARL表 */
	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacPort->u8MacAddr, struMacEntry, struDataEntry));

	/* Step2,3:决定写入ARL Entry0还是Entry1中 */

	/* 与Entry0,Entry1相比较,如果MAC相同则进行覆盖 */
	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (0 == s32Rv1)       /* 已经存在一条MAC地址相同的条目,直接覆盖 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)   /* 2条相同的MAC地址 */
			{
				printf("ethsw_add_arl_entry:Two Same MAC ARLs have EXISTED!\n");
				return (E_ETHSW_FAIL);
			}
		}
		
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2)   /* 已经存在一条MAC地址相同的条目,直接覆盖 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry1!\n");
		}
		u32Entry1 = 1;
	}
	else                    /* 没有MAC地址相同的表项 */
	{
		if (0 == struDataEntry[0].valid)
		{
			u32Entry0 = 1;  /* Entry0无效,选择写入Entry0 */
		}
		else if (0 == struDataEntry[1].valid)
		{
			u32Entry1 = 1;  /* Entry0有效,Entry1无效,选择写入Entry1 */
		}
		else  /* Entry0,Entry1均有效 */
		{
			if (0 != (g_u32PrintRules & ETHSW_PRINT_EXCEPTION))
			{
				printf("ethsw_add_arl_entry:ARL Bucket have no Space!\n");
			}
			return (E_ETHSW_BUCKET_FULL);
		} /* end of else Entry0,1均有效 */
	} /* end of else,没有MAC地址相同的表项 */

	/* Step4:修改Entry0或Entry1的MAC/VID,DATA寄存器的值 */

	/* 构造ARL TABLE MAC/VID Entry数据 */
	/* 不划分VLAN忽略VLAN ID */
	memcpy(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	/* 构造ARL TABLE DATA Entry数据 */
	struDataEntry[0].valid = 1;       /* valid */
	struDataEntry[0].StaticEntry = 1; /* static */
	struDataEntry[0].age = 1;         /* age */
	struDataEntry[0].priority = 0;    /* priority */
	struDataEntry[0].PortId = (pstruMacPort->u16PortId) & 0xf;  /* port id */
	struDataEntry[0].rsvd = 0;

	/* 如果为组播地址表,跟上述操作一样,但为了软件兼容性,还是保留此参数 */
	if (1 == u8CastType)
	{
		struDataEntry[0].PortId = (pstruMacPort->u16PortId) & 0x1ff;       /* port MAP bit8-0 */
		//struDataEntry[0].StaticEntry = (pstruMacPort->u16PortId) & 0x1; /* bit0 */
		struDataEntry[0].StaticEntry = 1;
	}

	if (1 == u32Entry0)      /* 选择的是ENTRY0 */
	{
		/* 写入ARL TABLE MAC/VID Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
		/* 写入ARL TABLE DATA Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
	}
	else if (1 == u32Entry1) /* 选择的是ENTRY1 */
	{
		/* 写入ARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
		/* 写入ARL TABLE DATA Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
	}

	/* Step5,6:设置写标记,开始写ARL表 */
	struArlRWCtrl.ArlStart = 1; /* 读写开始标记 */
	struArlRWCtrl.ArlRW = 0;    /* 写标记 */
	struArlRWCtrl.rsvd = 0;
	
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	/* 轮询写操作是否结束 */
	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* 超时 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_add_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	
	return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_remove_arl_entry
* 函数功能: 移除一条ARL表:将指定MAC地址(VLAN ID)的ARL表从芯片内部集成Memory中删除
* 相关文档:
* 函数参数:
* 参数名称:      类型                    输入/输出  描述
* u8CastType     u8                      输入        单播组播标记,0:单播,1:组播
* pstruMacPort   STRU_ETHSW_MAC_PORT*    输入        将此结构对应MAC地址与端口插
                                                     入到ARL表中
*
* 返回值:  s32
*          0:成功
*         -1:操作失败
*         -2:参数错误
*         -3:操作超时
*         -5:MAC地址无效
*         -6:端口号无效
*        -11:没查找到相匹配的ARL表项
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
s32 ethsw_remove_arl_entry(const STRU_ETHSW_MAC_ADDR *pstruMacAddr)
{
	s32                     s32Rv1;
	s32                     s32Rv2;             /* 返回值变量 */
	u32                     u32i;                  /* 循环变量 */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;      /* 标记,当为1时表示写相应Entry寄存器 */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;      /* 临时变量,查找时的控制结构 */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];    /* 临时变量,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2];   /* 临时变量,DATA Entry */


	if (NULLPTR == pstruMacAddr)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_remove_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* 清0临时变量 */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	/* Step1:查找相关的ARL表 */
	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacAddr->u8MacAddr, struMacEntry, struDataEntry));

	/* Step2,3:决定将ARL Entry0还是Entry1置为无效 */
	/* 规则:首先确定2条ARL中是否有MAC地址与即将移除的MAC地址相同的匹配表项,有的话
	判断该条是否是有效,若有效则直接覆盖该表,并设置该表为无效与静态,若此表无效
	且静态,则直接返回成功; 若表0是匹配表项,且表1为有效,则将表1 copy到表0;若没有
	匹配表项则返回失败(ARL表中没有要删除的表项)
	*/
	/* 与Entry0,Entry1相比较,如果MAC相同则进行覆盖 */
	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacAddr->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacAddr->u8MacAddr, 6);

	if (0 == s32Rv1)      /* 已经存在一条MAC地址相同的条目,直接置为无效 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_remove_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)  /* 2条相同的MAC地址 */
			{
			printf("ethsw_remove_arl_entry:Two Same MAC ARLs have EXISTED!\n");
			return (E_ETHSW_FAIL);
		}
			}
		/* 此条无效并且为静态的话则直接返回成功 */
		if ((0 == struDataEntry[0].valid) && (1 == struDataEntry[0].StaticEntry))
		{
			return (E_ETHSW_OK);
		}
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2) /* 已经存在一条MAC地址相同的条目,直接置为无效 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_remove_arl_entry:The Same MAC ARL Entry1!\n");
		}
		
		/* 此条无效并且为静态的话则直接返回成功 */
		if ((0 == struDataEntry[1].valid) && (1 == struDataEntry[1].StaticEntry))
		{
			return (E_ETHSW_OK);
		}
		u32Entry1 = 1;
	}
	else   /* 没有MAC地址相同的表项,返回ARL表没找到错误 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_remove_arl_entry:ARL NOT FOUND!\n");
		}
		return (E_ETHSW_ARL_NOT_FOUND);
	} /* end of else,没有MAC地址相同的表项 */

	/* Step4:修改Entry0或Entry1的MAC/VID,DATA寄存器的值 */
	if (1 == u32Entry0) /* ENTRY0的MAC地址与要删除的MAC地址匹配 */
	{
		/* 构造ARL TABLE DATA Entry数据,仅将条目设为无效,静态 */
		struDataEntry[0].valid = 0;       /* invalid */
		struDataEntry[0].StaticEntry = 1; /* static */
		
		if (1 == struDataEntry[1].valid)
		{
			/* 将有效表写入ARL TABLE MAC/VID Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,
			            (u8 *)&struMacEntry[1], EIGHT_BYTE_WIDTH));
			/* 将有效表写入ARL TABLE DATA Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,
			            (u8 *)&struDataEntry[1], TWO_BYTE_WIDTH));
			/* 将无效表写入ARL TABLE MAC/VID Entry1 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1,
			            (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
			/* 将无效表写入ARL TABLE DATA Entry1 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1,
			            (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
		}
		else
		{
			/* 写入ARL TABLE MAC/VID Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,
			            (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));  
			/* 写入ARL TABLE DATA Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,
			            (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));  
		}
	}
	else if (1 == u32Entry1) /* 选择的是ENTRY1 */
	{
		struDataEntry[1].valid = 0;       /* invalid */
		struDataEntry[1].StaticEntry = 1; /* static */

		/* 写ARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[1], EIGHT_BYTE_WIDTH));
		/* 写ARL TABLE DATA Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)&struDataEntry[1], TWO_BYTE_WIDTH));
	}

	/* Step5,6:设置写标记,开始写ARL表 */
	struArlRWCtrl.ArlStart = 1; /* 读写开始标记 */
	struArlRWCtrl.ArlRW = 0;    /* 写标记 */
	struArlRWCtrl.rsvd = 0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	/* 轮询写操作是否结束 */
	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* 超时 */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_remove_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	return (E_ETHSW_OK);
}

/*******************************************************************************
* 函数名称: ethsw_dump_arl_entry
* 函数功能: 查找ARL表中不超过指定条数的有效的ARL表的条目并输出或打印
* 相关文档:
* 函数参数:
* 参数名称:      类型                  输入/输出   描述
* u16ArlNum      u16                   输入        arl_entry结构中能存放的ARL表的数目
* pstruArlTable  STRU_ETHSW_ARL_TABLE* 输出        存放ARL表相关信息的结构,注意调用
                                                   函数输入此参数时应该能够存放
                                                   u16Arl_Num条ARL表
*
* 返回值:  s32
*          0:成功
*         -2:参数错误
*         -3:操作超时
*         -8:有效的ARL表太多,输入参数空间不够
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
s32 ethsw_dump_arl_entry(u16 u16ArlNum, STRU_ETHSW_ARL_TABLE *pstruArlTable)
{
	u32                         u32i = 0;         /* 循环变量,用来标记指针是否越界 */
	u32                         u32Count = 0;  /* 计数器,用来标记读寄存器是否超时 */

	STRU_ETHSW_ARL_SEARCH_CTRL  struSearchCtrl; /* 临时变量,查找时的控制结构 */
	STRU_ETHSW_ARL_MAC_VID      struMacEntry;   /* 临时变量,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA         struDataEntry;  /* 临时变量,DATA Entry */

	struSearchCtrl.ArlStart = 1;
	
	/* 开始搜索有效的ARL表 */
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));

	/* 轮询查找控制寄存器,判断搜索到的ARL表是否有效,搜索过程是否结束 */
	while (1) /* 无循环判断条件,靠内部条件跳出 */
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));
		/* 判断搜索到的ARL是否有效,如有效则保存,无效则计数以防超时 */
		if (0x1 == struSearchCtrl.valid)       /* 搜索到一条有效ARL表,保存起来 */
		{
			u32Count = 0;  /* 计数器清0 */
			/* 读取ARL表查找结果寄存器里的内容 */
			RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_MAC_RESULT, (u8 *)&struMacEntry, EIGHT_BYTE_WIDTH));
			RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_DATA_RESULT, (u8 *)&struDataEntry, FOUR_BYTE_WIDTH));
			/* 是否需要访问ARL_TABLE_SEARCH_MAC_RESULT0和ARL_TABLE_SEARCH_DATA_RESULT0 */
			
			/* 不对VID域进行访问,ARL_SEARCH_RESULT_VID */
			if ((NULLPTR == pstruArlTable) || (0 == u16ArlNum))
			{
				if (0 == (struMacEntry.u8MacAddr[0] & 0x1)) /* 单播地址 */
				{
					printf("[%03d]ARL: ", (s32)u32i);
					if (0 == struDataEntry.StaticEntry)     /* Learning */
					{
						printf("LEARNING!  ");
					}
					else
					{
						printf("STATIC!    ");        /* Static */
					}
					
					printf("MacAddr:<0x%02x-%02x-%02x-%02x-%02x-%02x>   ",
					(s32)struMacEntry.u8MacAddr[0], (s32)struMacEntry.u8MacAddr[1], (s32)struMacEntry.u8MacAddr[2],
					(s32)struMacEntry.u8MacAddr[3], (s32)struMacEntry.u8MacAddr[4], (s32)struMacEntry.u8MacAddr[5]);
					printf("PortId:<0x%01x>\n", (s32)struDataEntry.PortId);
				}
				else
				{
					printf("[%03d]MARL:STATIC!    ", (s32)u32i);
					printf("MacAddr:<0x%02x-%02x-%02x-%02x-%02x-%02x>   ",
					(s32)struMacEntry.u8MacAddr[0], (s32)struMacEntry.u8MacAddr[1], (s32)struMacEntry.u8MacAddr[2],
					(s32)struMacEntry.u8MacAddr[3], (s32)struMacEntry.u8MacAddr[4], (s32)struMacEntry.u8MacAddr[5]);
					printf("PortMap:<0x%05x>\n", (s32)(((struDataEntry.PortId) << 1) | struDataEntry.StaticEntry));
				}
			}
			else
			{
				/* 判断ARL表条数是否越界,如越界则不能进行保存 */
				if (u32i > (u32)(u16ArlNum - 1))
				{
					if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
					{
						printf("ethsw_dump_arl_entry:The searched ARL is larger than u16ArlNum\n");
					}
					return (E_ETHSW_ARL_TOO_MANY);
				}
				
				/* 保存在输出参数指针相应地址空间上 */
				save_arl_struct(&struMacEntry, &struDataEntry, (pstruArlTable + u32i));
			}
			u32i++;   /* 标记输出参数的指针要向后移位 */
		}
		else /* 0x00 == struSearchCtrl.valid,当前搜索到的ARL表无效 */
		{
			/* 继续搜索,计数器自增,并检验是否超时 */
			u32Count++;
			if (u32Count >= ETHSW_TIMEOUT_VAL)
			{
				/* 在进行查找的时候超时 */
				return (E_ETHSW_TIMEOUT);
			}
		}
		/* 检查搜索过程是否结束 */
		if (0x0 == struSearchCtrl.ArlStart) /* 搜索过程结束,停止循环 */
		{
			break;
		}
		/*else, 0x01 == struSearchCtrl.ArlStart,搜索过程未结束,do nothing */
	}
	
	return (E_ETHSW_OK);
}

/******************************* 源文件结束 ***********************************/

