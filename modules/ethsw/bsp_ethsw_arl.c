/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_arl.c 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
/************************** �����ļ����� **********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>
#include <string.h>

/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"
#include "bsp_ethsw_bcm5389.h"
#include "bsp_ethsw_arl.h"
#include "bsp_ethsw_mdio.h"

/******************************* �ֲ��궨�� *********************************/


/*********************** ȫ�ֱ�������/��ʼ�� **************************/
extern u32 g_u32PrintRules;

/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/
/*******************************************************************************
* ��������: save_arl_struct
* ��������: ����ARL_TABLE_MAC_VID_ENTRY0/1,ARL_TABLE_DATA_ENTRY0/1,
*           ARL_TABLE_SEARCH_MAC_RESULT/ARL_TABLE_SEARCH_DATA_RESULT�Ĵ����ж���
*           �������Ϣ,ת��ΪSTRU_ETHSW_ARL_TABLE�ṹ�����ֵ
* ����ĵ�:
* ��������:
* ��������:        ����                           ����/���   ����
* pstruMacEntry    STRU_ETHSW_ARL_MAC_VID_ENTRY*  ����        ��MAC_VID/MAC_RESULT
                                                              �Ĵ�������������(8�ֽ�)
* pstruDataEntry   STRU_ETHSW_ARL_DATA_ENTRY*     ����        ��DATA�Ĵ�����������
                                                              ��(2�ֽ�)
* pstruArlTable    STRU_ETHSW_ARL_TABLE*          ���        ��������Ϣת���󱣴�
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
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
* ��������: ethsw_lookup_arl_entry
* ��������: ����ָ��MAC��ַ��VLAN ID��ARL��,�������ҳ�����ARL���浽���������
* ����ĵ�:
* ��������:
* ��������:      ����                    ����/���  ����
* pu8MacAddr     u8*                     ����       ��Ҫ��ȡ��mac��ַ������׵�ַ
* pstruMacEntry  STRU_ETHSW_ARL_MAC_VID* ���       �����ҵ���ARL���MAC��ַVID��
                                                    ���ڴ˽ṹ��,,��һ����Ӧ��ENTRY0,
                                                    �ڶ�����Ӧ������Ӧ��ENTRY1
* pstruDataEntry STRU_ETHSW_ARL_DATA*    ���       �����ҵ���ARL���PortId��Ч��̬
                                                    ����Ϣ����ڴ˽ṹ��,,��һ����
                                                    Ӧ��ENTRY0,�ڶ�����Ӧ������Ӧ��
                                                    ENTRY1
*
* ����ֵ:   (��������ֵ��ע��)
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
*******************************************************************************/
s32 ethsw_lookup_arl_entry(const u8 *pu8MacAddr, STRU_ETHSW_ARL_MAC_VID *pstruMacEntry, STRU_ETHSW_ARL_DATA *pstruDataEntry)
{
	u32   u32i;              /* ѭ������ */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;  /* ��ʱ����,����ʱ�Ŀ��ƽṹ */

	if ((NULLPTR == pu8MacAddr) || (NULLPTR == pstruMacEntry) || (NULLPTR == pstruDataEntry))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_lookup_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}
	
	/* Step 1:����MAC��ַindex,6�ֽ� */
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, MAC_ADDRESS_INDEX, pu8MacAddr, SIX_BYTE_WIDTH));

	/* Step 2:����VLAN ID index,2�ֽ�,�����ڳ�ʼ��ʱDisable VLAN,���Բ���Ҫ���д��� */
	/* RETURN_IF_ERROR(ethsw_write_reg(u8ChipId, ETHSW_ARL_ACCESS_PAGE, VLAN_ID_INDEX, ?, TWO_BYTE_WIDTH)); */

	/* Step 3,4:���ö����,����ʼ;Ϊ�˷�ֹ��ʼ����д����,������bit0,���ܲ���Ҫ���� */
	struArlRWCtrl.ArlStart = 1;/* ��ʼ��д��� */
	struArlRWCtrl.ArlRW = 1;   /* ����� */
	struArlRWCtrl.rsvd = 0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	/* ��ѯ�������Ƿ���� */
	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	
	if (u32i >= ETHSW_TIMEOUT_VAL) /* ��ʱ */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_TIMEOUT))
		{
			printf("ethsw_lookup_arl_entry:Timed Out!\n");
		}
		return (E_ETHSW_TIMEOUT);
	}
	else  /* û�г�ʱ,��2��Entry�����ݶ��������� */
	{
		/* ��ȡMAC_VID_ENRTY0������,��ȡ8�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)pstruMacEntry, EIGHT_BYTE_WIDTH));
		/* ��ȡDATA_ENRTY0������,��ȡ2�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,(u8 *)pstruDataEntry, TWO_BYTE_WIDTH));

		/* ��ȡMAC_VID_ENRTY1������,��ȡ8�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)(pstruMacEntry + 1), EIGHT_BYTE_WIDTH));
		/* ��ȡDATA_ENRTY1������,��ȡ2�ֽ����� */
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)(pstruDataEntry + 1), TWO_BYTE_WIDTH));
	}
	return (E_ETHSW_OK);
	
}

/*******************************************************************************
* ��������: ethsw_add_arl_entry
* ��������: ���һ��ARL��:��ָ��MAC��ַ(VLAN ID)��ARL��д�뵽оƬ�ڲ�����Memory��
* ����ĵ�:
* ��������:
* ��������:      ����                    ����/���  ����
* u8CastType     u8                      ����        �����鲥���,0:����,1:�鲥
* pstruMacPort   STRU_ETHSW_MAC_PORT*    ����        ���˽ṹ��ӦMAC��ַ��˿ڲ�
                                                     �뵽ARL����
*
* ����ֵ:  s32
*          0:�ɹ�
*          ��������
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
* 
*******************************************************************************/
s32 ethsw_add_arl_entry(u8 u8CastType, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	s32                     s32Rv1;
	s32                     s32Rv2;           /* ����ֵ */
	u32                     u32i;             /* ѭ������ */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;    /* ���,��Ϊ1ʱ��ʾд��ӦEntry�Ĵ��� */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];  /* ��ʱ����,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2]; /* ��ʱ����,DATA Entry */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;    /* ��ʱ����,ARL���д�����ṹ */

	if (NULLPTR == pstruMacPort)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_add_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* ��0��ʱ���� */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	/* Step1:������ص�ARL�� */
	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacPort->u8MacAddr, struMacEntry, struDataEntry));

	/* Step2,3:����д��ARL Entry0����Entry1�� */

	/* ��Entry0,Entry1��Ƚ�,���MAC��ͬ����и��� */
	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacPort->u8MacAddr, 6);

	if (0 == s32Rv1)       /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ�Ӹ��� */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)   /* 2����ͬ��MAC��ַ */
			{
				printf("ethsw_add_arl_entry:Two Same MAC ARLs have EXISTED!\n");
				return (E_ETHSW_FAIL);
			}
		}
		
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2)   /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ�Ӹ��� */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_add_arl_entry:The Same MAC ARL Entry1!\n");
		}
		u32Entry1 = 1;
	}
	else                    /* û��MAC��ַ��ͬ�ı��� */
	{
		if (0 == struDataEntry[0].valid)
		{
			u32Entry0 = 1;  /* Entry0��Ч,ѡ��д��Entry0 */
		}
		else if (0 == struDataEntry[1].valid)
		{
			u32Entry1 = 1;  /* Entry0��Ч,Entry1��Ч,ѡ��д��Entry1 */
		}
		else  /* Entry0,Entry1����Ч */
		{
			if (0 != (g_u32PrintRules & ETHSW_PRINT_EXCEPTION))
			{
				printf("ethsw_add_arl_entry:ARL Bucket have no Space!\n");
			}
			return (E_ETHSW_BUCKET_FULL);
		} /* end of else Entry0,1����Ч */
	} /* end of else,û��MAC��ַ��ͬ�ı��� */

	/* Step4:�޸�Entry0��Entry1��MAC/VID,DATA�Ĵ�����ֵ */

	/* ����ARL TABLE MAC/VID Entry���� */
	/* ������VLAN����VLAN ID */
	memcpy(struMacEntry[0].u8MacAddr, pstruMacPort->u8MacAddr, 6);
	/* ����ARL TABLE DATA Entry���� */
	struDataEntry[0].valid = 1;       /* valid */
	struDataEntry[0].StaticEntry = 1; /* static */
	struDataEntry[0].age = 1;         /* age */
	struDataEntry[0].priority = 0;    /* priority */
	struDataEntry[0].PortId = (pstruMacPort->u16PortId) & 0xf;  /* port id */
	struDataEntry[0].rsvd = 0;

	/* ���Ϊ�鲥��ַ��,����������һ��,��Ϊ�����������,���Ǳ����˲��� */
	if (1 == u8CastType)
	{
		struDataEntry[0].PortId = (pstruMacPort->u16PortId) & 0x1ff;       /* port MAP bit8-0 */
		//struDataEntry[0].StaticEntry = (pstruMacPort->u16PortId) & 0x1; /* bit0 */
		struDataEntry[0].StaticEntry = 1;
	}

	if (1 == u32Entry0)      /* ѡ�����ENTRY0 */
	{
		/* д��ARL TABLE MAC/VID Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
		/* д��ARL TABLE DATA Entry0 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0, (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
	}
	else if (1 == u32Entry1) /* ѡ�����ENTRY1 */
	{
		/* д��ARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
		/* д��ARL TABLE DATA Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
	}

	/* Step5,6:����д���,��ʼдARL�� */
	struArlRWCtrl.ArlStart = 1; /* ��д��ʼ��� */
	struArlRWCtrl.ArlRW = 0;    /* д��� */
	struArlRWCtrl.rsvd = 0;
	
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	/* ��ѯд�����Ƿ���� */
	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* ��ʱ */
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
* ��������: ethsw_remove_arl_entry
* ��������: �Ƴ�һ��ARL��:��ָ��MAC��ַ(VLAN ID)��ARL���оƬ�ڲ�����Memory��ɾ��
* ����ĵ�:
* ��������:
* ��������:      ����                    ����/���  ����
* u8CastType     u8                      ����        �����鲥���,0:����,1:�鲥
* pstruMacPort   STRU_ETHSW_MAC_PORT*    ����        ���˽ṹ��ӦMAC��ַ��˿ڲ�
                                                     �뵽ARL����
*
* ����ֵ:  s32
*          0:�ɹ�
*         -1:����ʧ��
*         -2:��������
*         -3:������ʱ
*         -5:MAC��ַ��Ч
*         -6:�˿ں���Ч
*        -11:û���ҵ���ƥ���ARL����
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
*******************************************************************************/
s32 ethsw_remove_arl_entry(const STRU_ETHSW_MAC_ADDR *pstruMacAddr)
{
	s32                     s32Rv1;
	s32                     s32Rv2;             /* ����ֵ���� */
	u32                     u32i;                  /* ѭ������ */
	u32                     u32Entry0 = 0;
	u32                     u32Entry1 = 0;      /* ���,��Ϊ1ʱ��ʾд��ӦEntry�Ĵ��� */
	STRU_ETHSW_ARL_RW_CTRL  struArlRWCtrl;      /* ��ʱ����,����ʱ�Ŀ��ƽṹ */
	STRU_ETHSW_ARL_MAC_VID  struMacEntry[2];    /* ��ʱ����,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA     struDataEntry[2];   /* ��ʱ����,DATA Entry */


	if (NULLPTR == pstruMacAddr)
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_remove_arl_entry:NULL Pointer!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	/* ��0��ʱ���� */
	memset((void *)struMacEntry, 0x00, sizeof(struMacEntry));
	memset((void *)struDataEntry, 0x00, sizeof(struDataEntry));

	/* Step1:������ص�ARL�� */
	RETURN_IF_ERROR(ethsw_lookup_arl_entry(pstruMacAddr->u8MacAddr, struMacEntry, struDataEntry));

	/* Step2,3:������ARL Entry0����Entry1��Ϊ��Ч */
	/* ����:����ȷ��2��ARL���Ƿ���MAC��ַ�뼴���Ƴ���MAC��ַ��ͬ��ƥ�����,�еĻ�
	�жϸ����Ƿ�����Ч,����Ч��ֱ�Ӹ��Ǹñ�,�����øñ�Ϊ��Ч�뾲̬,���˱���Ч
	�Ҿ�̬,��ֱ�ӷ��سɹ�; ����0��ƥ�����,�ұ�1Ϊ��Ч,�򽫱�1 copy����0;��û��
	ƥ������򷵻�ʧ��(ARL����û��Ҫɾ���ı���)
	*/
	/* ��Entry0,Entry1��Ƚ�,���MAC��ͬ����и��� */
	s32Rv1 = memcmp(struMacEntry[0].u8MacAddr, pstruMacAddr->u8MacAddr, 6);
	s32Rv2 = memcmp(struMacEntry[1].u8MacAddr, pstruMacAddr->u8MacAddr, 6);

	if (0 == s32Rv1)      /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ����Ϊ��Ч */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_remove_arl_entry:The Same MAC ARL Entry0!\n");
			if (0 == s32Rv2)  /* 2����ͬ��MAC��ַ */
			{
			printf("ethsw_remove_arl_entry:Two Same MAC ARLs have EXISTED!\n");
			return (E_ETHSW_FAIL);
		}
			}
		/* ������Ч����Ϊ��̬�Ļ���ֱ�ӷ��سɹ� */
		if ((0 == struDataEntry[0].valid) && (1 == struDataEntry[0].StaticEntry))
		{
			return (E_ETHSW_OK);
		}
		u32Entry0 = 1;
	}
	else if (0 == s32Rv2) /* �Ѿ�����һ��MAC��ַ��ͬ����Ŀ,ֱ����Ϊ��Ч */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_remove_arl_entry:The Same MAC ARL Entry1!\n");
		}
		
		/* ������Ч����Ϊ��̬�Ļ���ֱ�ӷ��سɹ� */
		if ((0 == struDataEntry[1].valid) && (1 == struDataEntry[1].StaticEntry))
		{
			return (E_ETHSW_OK);
		}
		u32Entry1 = 1;
	}
	else   /* û��MAC��ַ��ͬ�ı���,����ARL��û�ҵ����� */
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
		{
			printf("ethsw_remove_arl_entry:ARL NOT FOUND!\n");
		}
		return (E_ETHSW_ARL_NOT_FOUND);
	} /* end of else,û��MAC��ַ��ͬ�ı��� */

	/* Step4:�޸�Entry0��Entry1��MAC/VID,DATA�Ĵ�����ֵ */
	if (1 == u32Entry0) /* ENTRY0��MAC��ַ��Ҫɾ����MAC��ַƥ�� */
	{
		/* ����ARL TABLE DATA Entry����,������Ŀ��Ϊ��Ч,��̬ */
		struDataEntry[0].valid = 0;       /* invalid */
		struDataEntry[0].StaticEntry = 1; /* static */
		
		if (1 == struDataEntry[1].valid)
		{
			/* ����Ч��д��ARL TABLE MAC/VID Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,
			            (u8 *)&struMacEntry[1], EIGHT_BYTE_WIDTH));
			/* ����Ч��д��ARL TABLE DATA Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,
			            (u8 *)&struDataEntry[1], TWO_BYTE_WIDTH));
			/* ����Ч��д��ARL TABLE MAC/VID Entry1 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1,
			            (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));
			/* ����Ч��д��ARL TABLE DATA Entry1 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1,
			            (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));
		}
		else
		{
			/* д��ARL TABLE MAC/VID Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY0,
			            (u8 *)&struMacEntry[0], EIGHT_BYTE_WIDTH));  
			/* д��ARL TABLE DATA Entry0 */
			RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY0,
			            (u8 *)&struDataEntry[0], TWO_BYTE_WIDTH));  
		}
	}
	else if (1 == u32Entry1) /* ѡ�����ENTRY1 */
	{
		struDataEntry[1].valid = 0;       /* invalid */
		struDataEntry[1].StaticEntry = 1; /* static */

		/* дARL TABLE MAC/VID Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_MAC_VID_ENTRY1, (u8 *)&struMacEntry[1], EIGHT_BYTE_WIDTH));
		/* дARL TABLE DATA Entry1 */
		RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_DATA_ENTRY1, (u8 *)&struDataEntry[1], TWO_BYTE_WIDTH));
	}

	/* Step5,6:����д���,��ʼдARL�� */
	struArlRWCtrl.ArlStart = 1; /* ��д��ʼ��� */
	struArlRWCtrl.ArlRW = 0;    /* д��� */
	struArlRWCtrl.rsvd = 0;
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));

	/* ��ѯд�����Ƿ���� */
	for (u32i = 0; u32i < ETHSW_TIMEOUT_VAL; u32i++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_RW_CTRL, (u8 *)&struArlRWCtrl, ONE_BYTE_WIDTH));
		if (0 == struArlRWCtrl.ArlStart)
		{
			break;
		}
	}
	if (u32i >= ETHSW_TIMEOUT_VAL) /* ��ʱ */
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
* ��������: ethsw_dump_arl_entry
* ��������: ����ARL���в�����ָ����������Ч��ARL�����Ŀ��������ӡ
* ����ĵ�:
* ��������:
* ��������:      ����                  ����/���   ����
* u16ArlNum      u16                   ����        arl_entry�ṹ���ܴ�ŵ�ARL�����Ŀ
* pstruArlTable  STRU_ETHSW_ARL_TABLE* ���        ���ARL�������Ϣ�Ľṹ,ע�����
                                                   ��������˲���ʱӦ���ܹ����
                                                   u16Arl_Num��ARL��
*
* ����ֵ:  s32
*          0:�ɹ�
*         -2:��������
*         -3:������ʱ
*         -8:��Ч��ARL��̫��,��������ռ䲻��
* ��������: <�ص����жϡ������루���������������Ⱥ�������˵�����ͼ�ע������>
* ����˵��:���������б�ķ�ʽ˵����������ȫ�ֱ�����ʹ�ú��޸�������Լ�������
*           δ��ɻ��߿��ܵĸĶ���
*
* 1.���������ı��ȫ�ֱ����б�
* 2.���������ı�ľ�̬�����б�
* 3.���õ�û�б��ı��ȫ�ֱ����б�
* 4.���õ�û�б��ı�ľ�̬�����б�
*
* �޸�����    �汾��   �޸���  �޸�����
* -----------------------------------------------------------------
*******************************************************************************/
s32 ethsw_dump_arl_entry(u16 u16ArlNum, STRU_ETHSW_ARL_TABLE *pstruArlTable)
{
	u32                         u32i = 0;         /* ѭ������,�������ָ���Ƿ�Խ�� */
	u32                         u32Count = 0;  /* ������,������Ƕ��Ĵ����Ƿ�ʱ */

	STRU_ETHSW_ARL_SEARCH_CTRL  struSearchCtrl; /* ��ʱ����,����ʱ�Ŀ��ƽṹ */
	STRU_ETHSW_ARL_MAC_VID      struMacEntry;   /* ��ʱ����,MAC/VID Entry */
	STRU_ETHSW_ARL_DATA         struDataEntry;  /* ��ʱ����,DATA Entry */

	struSearchCtrl.ArlStart = 1;
	
	/* ��ʼ������Ч��ARL�� */
	RETURN_IF_ERROR(ethsw_write_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));

	/* ��ѯ���ҿ��ƼĴ���,�ж���������ARL���Ƿ���Ч,���������Ƿ���� */
	while (1) /* ��ѭ���ж�����,���ڲ��������� */
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_CTRL, (u8 *)&struSearchCtrl, ONE_BYTE_WIDTH));
		/* �ж���������ARL�Ƿ���Ч,����Ч�򱣴�,��Ч������Է���ʱ */
		if (0x1 == struSearchCtrl.valid)       /* ������һ����ЧARL��,�������� */
		{
			u32Count = 0;  /* ��������0 */
			/* ��ȡARL����ҽ���Ĵ���������� */
			RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_MAC_RESULT, (u8 *)&struMacEntry, EIGHT_BYTE_WIDTH));
			RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_ACCESS_PAGE, ARL_TABLE_SEARCH_DATA_RESULT, (u8 *)&struDataEntry, FOUR_BYTE_WIDTH));
			/* �Ƿ���Ҫ����ARL_TABLE_SEARCH_MAC_RESULT0��ARL_TABLE_SEARCH_DATA_RESULT0 */
			
			/* ����VID����з���,ARL_SEARCH_RESULT_VID */
			if ((NULLPTR == pstruArlTable) || (0 == u16ArlNum))
			{
				if (0 == (struMacEntry.u8MacAddr[0] & 0x1)) /* ������ַ */
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
				/* �ж�ARL�������Ƿ�Խ��,��Խ�����ܽ��б��� */
				if (u32i > (u32)(u16ArlNum - 1))
				{
					if (0 != (g_u32PrintRules & ETHSW_PRINT_INFO))
					{
						printf("ethsw_dump_arl_entry:The searched ARL is larger than u16ArlNum\n");
					}
					return (E_ETHSW_ARL_TOO_MANY);
				}
				
				/* �������������ָ����Ӧ��ַ�ռ��� */
				save_arl_struct(&struMacEntry, &struDataEntry, (pstruArlTable + u32i));
			}
			u32i++;   /* ������������ָ��Ҫ�����λ */
		}
		else /* 0x00 == struSearchCtrl.valid,��ǰ��������ARL����Ч */
		{
			/* ��������,����������,�������Ƿ�ʱ */
			u32Count++;
			if (u32Count >= ETHSW_TIMEOUT_VAL)
			{
				/* �ڽ��в��ҵ�ʱ��ʱ */
				return (E_ETHSW_TIMEOUT);
			}
		}
		/* ������������Ƿ���� */
		if (0x0 == struSearchCtrl.ArlStart) /* �������̽���,ֹͣѭ�� */
		{
			break;
		}
		/*else, 0x01 == struSearchCtrl.ArlStart,��������δ����,do nothing */
	}
	
	return (E_ETHSW_OK);
}

/******************************* Դ�ļ����� ***********************************/

