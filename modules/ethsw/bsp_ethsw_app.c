/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_app.c 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
/************************** �����ļ����� **********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"
#include "bsp_ethsw_bcm5389.h"
#include "bsp_ethsw_port.h"
#include "bsp_ethsw_arl.h"
#include "bsp_ethsw_app.h"
#include "bsp_ethsw_mdio.h"

/******************************* �ֲ��궨�� *********************************/


/*********************** ȫ�ֱ�������/��ʼ�� **************************/
/* ��ӡ���,�������ƴ�ӡ�������,������ʲô�����ӡ���:
   0x0:����ӡ;0x1:������������;
   0x2:��ӡ��ʱ��Ϣ;
   0x4:��ӡ�쳣��Ϣ;
   0x8:����ִ��ʧ�ܴ�ӡ����ֵ;
   0x10:��ӡ��ʾ��Ϣ;
   ����:���ִ�ӡ��� ȱʡֵΪ
   ������ӡ,���ڵ���ʱ��̬�޸Ĵ�ֵ,�Դ�ӡ��������� */
   
u32 g_u32PrintRules = 0x0f;

/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/
/*******************************************************************************
* ��������: ethsw_create_arl_map
* ��������: ethsw_ioctl�ķ�֧����,��Ҫ������:����������MAC��ַ��˿�ת����ϵ��
* ����ĵ�:
* ��������:
* ��������:    ����                    ����/���  ����
* u16MapNum    u16                     ����       Ҫ������MAC��ַ��˿�ת����
                                                  ϵ����
* pstruMacPort STRU_ETHSW_MAC_PORT *   ����       Ҫ�����Ķ˿���MAC��ַӳ���
                                                  ϵ�ṹָ��
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
* ˵   ��: ��ioctl ���Ƶ����������
*******************************************************************************/
s32 ethsw_create_arl_map(u16 u16MapNum, const STRU_ETHSW_MAC_PORT *pstruMacPort)
{
	u8 u8CastType; /* �������鲥���,0Ϊ����,1Ϊ�鲥 */
	s32 s32Rv;      /* ����ֵ */
	u32 u32i;          /* ѭ������ */
	
	/* MAC��ַ��˿ڶ�Ӧ��ϵ,��ʱ����,ע��˽ṹ������Ľṹ��ͬ */
	STRU_ETHSW_MAC_PORT   struMacPort;

	/* У���������Ч�� */
	if ((0 == u16MapNum) || (NULLPTR == pstruMacPort))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_create_arl_map:invalid parameters!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	for (u32i = 0; u32i < u16MapNum; u32i++)
	{
		struMacPort.u16PortId = (pstruMacPort + u32i)->u16PortId; /* ����ʱΪ�˿ں�/�鲥ʱΪ�˿�MAP */
		memcpy(struMacPort.u8MacAddr, (pstruMacPort + u32i)->u8MacAddr, 6);
		if (0 != (struMacPort.u8MacAddr[0] & 0x1))
		{
			u8CastType = 1; /* MAC ��ַbit40Ϊ1ʱ,��ʾ�鲥��ַ */
		}
		else
		{
			u8CastType = 0; /* MAC ��ַbit40Ϊ0ʱ,��ʾ������ַ */
		}
		
		/* ���ú�������һ��ARL�� */
		s32Rv = ethsw_add_arl_entry(u8CastType, (STRU_ETHSW_MAC_PORT *)&struMacPort);
		if (0 != s32Rv)
		{
			return (s32Rv);
		}
	}
	
	return (E_ETHSW_OK);
	
}

s32 addarl(int flag)
{
	STRU_ETHSW_MAC_PORT   struMacPort;
	struMacPort.u16PortId = 1;
	u8 mac[6];
	mac[0] = 0x36;
	mac[1] = 0x37;
	mac[2] = 0x32;
	mac[3] = 0x33;
	mac[4] = 0x24;
	mac[5] = 0x42;
	
	memcpy(struMacPort.u8MacAddr, mac, 6);
	ethsw_add_arl_entry(flag, (STRU_ETHSW_MAC_PORT *)&struMacPort);
	return (E_ETHSW_OK);
}


/*******************************************************************************
* ��������: ethsw_delete_arl_map
* ��������: ɾ��������MAC��ַ��˿�ת����ϵ��
* ����ĵ�:
* ��������:
* ��������:    ����                    ����/���  ����
* u16MapNum    u16                     ����       Ҫ������MAC��ַ��˿�ת����
                                                  ϵ����
* pstruMacPort STRU_ETHSW_MAC_PORT *   ����       Ҫ�����Ķ˿���MAC��ַӳ���
                                                  ϵ�ṹָ��
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
s32 ethsw_delete_arl_map(u16 u16MapNum, const STRU_ETHSW_MAC_ADDR *pstruMacAddr)
{
	s32                  s32Rv;  /* ����ֵ */
	u32                  u32i;      /* ѭ������ */
	/* MAC��ַ��˿ڶ�Ӧ��ϵ,��ʱ����,ע��˽ṹ������Ľṹ��ͬ */
	STRU_ETHSW_MAC_ADDR  struMacAddr;
	
	/* У���������Ч�� */
	if ((0 == u16MapNum) || (NULLPTR == pstruMacAddr))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_delete_arl_map:invalid parameters!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}

	for (u32i = 0; u32i < u16MapNum; u32i++)
	{
		/* ����MAC��ַ */
		memcpy(struMacAddr.u8MacAddr, (pstruMacAddr + u32i)->u8MacAddr, 6);
		
		/* ���ú���ɾ��һ��ARL�� */
		s32Rv = ethsw_remove_arl_entry(&struMacAddr);
		if (0 != s32Rv)
		{
			return (s32Rv);
		}
	}
	return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_search_arl_map
* ��������: ��Ҫ������:����������MAC��ַ��˿�ת����ϵ��
* ����ĵ�:
* ��������:
* ��������:    ����                  ����/���  ����
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
s32 ethsw_search_arl_map(void)
{
    s32   s32Rv = 0;   /* ����ֵ */    
    s32Rv = ethsw_dump_arl_entry(0,NULLPTR);
    return (s32Rv);
	
}
/*******************************************************************************
* ��������: ethsw_set_port_mirror
* ��������: ��Ҫ������:���ö˿ڵ�mirror����
* ����ĵ�:
* ��������:
* ��������:       ����          ����/���  ����
* u8MirrorEnable  u8            ����       ʹ��MIRROR����
* u8MirrorRules   u8            ����       MIRROR�Ĺ���
* u8MirroredPort  u8            ����       ��MIRROR�Ķ˿�
* u8MirrorPort    u8            ����       MIRROR�˿�(���Ӷ˿�)
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
* ?
*******************************************************************************/
s32 ethsw_set_port_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort)
{
	u8   u8Enable;
	s32  s32Rv;
	
	/* �Բ��������ж� */
	if ((u8MirrorRules > 3) || (u8MirroredPort > MAX_USED_PORTS - 1) || (u8MirrorPort > MAX_USED_PORTS - 1))
	{
		if (0 != (g_u32PrintRules & ETHSW_PRINT_PARAMETER))
		{
			printf("ethsw_set_port_mirror:invalid parameters!\n");
		}
		return (E_ETHSW_WRONG_PARAM);
	}
	
	/* MIRRORʹ�ܱ������ */
	if (0 == u8MirrorEnable)
	{
		u8Enable = 0;
	}
	else
	{
		u8Enable = 1;
	}
	
	/* ���ú�����MIRROR���ܽ������� */
	s32Rv = ethsw_set_mirror(u8Enable, u8MirrorRules, u8MirroredPort, u8MirrorPort);
	
	return (s32Rv);
}

/*******************************************************************************
* ��������: ethsw_version
* ��������: �������İ汾�Ŵ�ӡ����
* ����ĵ�:
* ��������: ��
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
void ethsw_version(void)
{
    (void)printf("VERSION:<2013/10/08>\n");
}
/********************************************************************************
* ��������: ethsw_get_port_mib							
* ��    ��: ��ȡ�˿�ͳ��ֵ                             
* ����ĵ�:                    
* ��������:	int								
* ��    ��: 						     			
* ��������		   ����					����/��� 		����		
* u8PortId        u8          ����        �˿�ID

* ����ֵ: 0 ��ʾ�ɹ�������ֵ��ʾʧ�ܡ�								
* ˵   ��: 
*********************************************************************************/
s32 ethsw_get_port_mib(u8 u8PortId)
{
	u8   u8Buf[8];
	unsigned long long int u32temp1;
	ULONG u32temp2;

	if(u8PortId > MAX_USED_PORTS - 1)
	{
		printf("\nInvalid Parameter! u8PortId = %d\n",u8PortId);
		return E_ETHSW_WRONG_PARAM;
	}

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_OCTETS, u8Buf, EIGHT_BYTE_WIDTH));
		u32temp2 = (u8Buf[7])|(u8Buf[6]<<8)|(u8Buf[5]<<16)|(u8Buf[4]<<24)/*|(u8Buf[3]<<32)|(u8Buf[2]<<40)|(u8Buf[1]<<48)|(u8Buf[0]<<56)*/;
		printf("\nPort[%d] TX_OCTETS: %llu\n", u8PortId, u32temp2);
	
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_DROP_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] TX_DROP_PKTS: %llu\n", u8PortId, u32temp1);
		
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_BROADCAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] TX_BROADCAST_PKTS: %llu\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_MULTICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] TX_MULTICAST_PKTS: %lu\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_UNICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] TX_UNICAST_PKTS: %llu\n", u8PortId, u32temp1);
#if 0
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_COLLISIONS, u8Buf, EIGHT_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_COLLISIONS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_SINGLE_COLLISIONS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_SINGLE_COLLISIONS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_MULTIPLE_COLLISIONS, u8Buf, EIGHT_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_MULTIPLE_COLLISIONS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_DEFERRED_TRANSMIT, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_DEFERRED_TRANSMIT: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_LATE_COLLISION, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_LATE_COLLISION: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_EXCESSIVE_COLLISION, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_EXCESSIVE_COLLISION: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_FRAME_INDISC, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_FRAME_INDISC: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), TX_PAUSE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] TX_PAUSE_PKTS: 0x%x\n", u8PortId, u32temp1);
	
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_UNDERSIZE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_UNDERSIZE_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_PAUSE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_PAUSE_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_64_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_64_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_64TO127_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_64TO127_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_128TO255_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_128TO255_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_256TO511_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_256TO511_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_512TO1023_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_512TO1023_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_1024TO1522_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_1024TO1522_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_OVERSIZE_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_OVERSIZE_PKTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_JABBER, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_JABBER: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_ALIGNMENT_ERRS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_ALIGNMENT_ERRS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_FCS_ERRS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_FCS_ERRS: 0x%x\n", u8PortId, u32temp1);

#endif	
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_OCTETS, u8Buf, EIGHT_BYTE_WIDTH));
		u32temp2 = (u8Buf[7])|(u8Buf[6]<<8)|(u8Buf[5]<<16)|(u8Buf[4]<<24)/*|(u8Buf[3]<<32)|(u8Buf[2]<<40)|(u8Buf[1]<<48)|(u8Buf[0]<<56)*/;
		printf("\nPort[%d] RX_OCTETS: %lu\n", u8PortId, u32temp2);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_GOOD_OCTETS, u8Buf, EIGHT_BYTE_WIDTH));
		u32temp2 = (u8Buf[7])|(u8Buf[6]<<8)|(u8Buf[5]<<16)|(u8Buf[4]<<24)/*|(u8Buf[3]<<32)|(u8Buf[2]<<40)|(u8Buf[1]<<48)|(u8Buf[0]<<56)*/;
		printf("\nPort[%d] RX_GOOD_OCTETS: %lu\n", u8PortId, u32temp2);


		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_DROP_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] RX_DROP_PKTS: %lu\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_UNICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] RX_UNICAST_PKTS: %lu\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_MULTICAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] RX_MULTICAST_PKTS: %lu\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_BROADCAST_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[3]&0xff)|((u8Buf[2]<<8)&0xff00)|((u8Buf[1]<<16)&0xff0000)|((u8Buf[0]<<24)&0xff000000);
		printf("\nPort[%d] RX_BROADCAST_PKTS: %lu\n", u8PortId, u32temp1);

#if 0
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_SA_CHANGES, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_SA_CHANGES: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_FRAGMENTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_FRAGMENTS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_EXCESSSIZE_DISC, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_EXCESSSIZE_DISC: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_SYMBOL_ERR, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_SYMBOL_ERR: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), RX_QOS_PKTS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] RX_QOS_PKTS: 0x%x\n", u8PortId, u32temp1);
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_1523TO2047_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_1523TO2047_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_2048TO4095_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_2048TO4095_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_4096TO8191_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_4096TO8191_OCTETS: 0x%x\n", u8PortId, u32temp1);

		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PORTMIB_PAGE(u8PortId), PKTS_8192TO9728_OCTETS, u8Buf, FOUR_BYTE_WIDTH));
		u32temp1 = (u8Buf[0]&0xff)|((u8Buf[1]<<8)&0xff00)|((u8Buf[2]<<16)&0xff0000)|((u8Buf[3]<<24)&0xff000000);
		printf("\nPort[%d] PKTS_8192TO9728_OCTETS: 0x%x\n", u8PortId, u32temp1);
#endif

	return E_ETHSW_OK;
}

/*******************************************************************************
* ��������: ethsw_display_all_reg
* ��������: ��оƬ���еļĴ�����ֵ������,��ӡ����
* ����ĵ�:
* ��������: ��
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
s32 ethsw_display_all_reg(void)
{
	u8   u8Buf[8];
	u8   u8PortId;

	/* Display ETHSW_CTRL_PAGE */
	for (u8PortId = 0; u8PortId < MAX_USED_PORTS-1; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId), u8Buf, ONE_BYTE_WIDTH));
		printf("ETHSW_CTRL_PAGE PORT_TRAFIC_CTRL[%d] is <0x%02x>\n", (s32)u8PortId, (s32)u8Buf[0]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, IMP_TRAFIC_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE IMP_TRAFIC_CTRL is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE SWITCH_MODE is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_FORWARD_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE PORT_FORWARD_CTRL is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, UNI_DLF_FW_MAP, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE UNI_DLF_FW_MAP is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, MUL_DLF_FW_MAP, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE MUL_DLF_FW_MAP is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);

	for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, MII_PORT_STATE_OR(u8PortId), u8Buf, ONE_BYTE_WIDTH));
		printf("ETHSW_CTRL_PAGE MII_PORT_STATE_OR[%d] is <0x%02x>\n", (s32)u8PortId, (s32)u8Buf[0]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, IMP_PORT_STATE_OR, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE IMP_PORT_STATE_OR is <0x%02x>\n", (s32)u8Buf[0]);

	for (u8PortId = 0; u8PortId < MAX_USED_PORTS - 1; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, EXTERN_PHY_SCAN_RESULT(u8PortId), u8Buf, ONE_BYTE_WIDTH));
		printf("ETHSW_CTRL_PAGE EXTERN_PHY_SCAN_RESULT[%d] is <0x%02x>\n", (s32)u8PortId, (s32)u8Buf[0]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, FAST_AGING_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE FAST_AGING_CTRL is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, FAST_AGING_PORT, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE FAST_AGING_PORT is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, FAST_AGING_VID, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE FAST_AGING_VID is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PAUSE_FRAME_DETECT_CTRL, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_CTRL_PAGE PAUSE_FRAME_DETECT_CTRL is <0x%02x>\n", (s32)u8Buf[0]);

	/* Display ETHSW_STAT_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_SUMMARY, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE LINK_STATUS_SUMMARY is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_CHANGE, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE LINK_STATUS_CHANGE is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, PORT_SPEED_SUMMARY, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE PORT_SPEED_SUMMARY is <0x%02x%02x%02x%02x\n", (s32)u8Buf[0], (s32)u8Buf[1],(s32)u8Buf[2], (s32)u8Buf[3]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, DUPLEX_STATUS_SUMMARY, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE DUPLEX_STATUS_SUMMARY is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, PAUSE_STATUS_SUMMARY, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_STAT_PAGE PAUSE_STATUS_SUMMARY is <0x%02x%02x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1],(s32)u8Buf[2], (s32)u8Buf[3]);

	/* Display ETHSW_MGMT_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, GLOBAL_CONFIG, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE GLOBAL_CONFIG is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, AGING_TIME_CTRL, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE AGING_TIME_CTRL is <0x%02x%02x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1],(s32)u8Buf[2], (s32)u8Buf[3]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE MIRROR_CAPTURE_CTRL is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE INGRESS_MIRROR_CTRL is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_DIVIDER, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE INGRESS_MIRROR_DIVIDER is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_MGMT_PAGE EGRESS_MIRROR_CTRL is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);

	/* Display ETHSW_ARL_CTRL_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, GLOBAL_ARL_CFG, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE GLOBAL_ARL_CFG is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, BPDU_MULTICAST_ADDR, u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE BPDU_MULTICAST_ADDR is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[0], (s32)u8Buf[1], (s32)u8Buf[2], (s32)u8Buf[3], (s32)u8Buf[4], (s32)u8Buf[5]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR1, u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_ADDR1 is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[0], (s32)u8Buf[1], (s32)u8Buf[2], (s32)u8Buf[3], (s32)u8Buf[4], (s32)u8Buf[5]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_VECTOR1, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_VECTOR1 is <0x%02x%02x%02x%02x>\n", u8Buf[0], u8Buf[1],
	(s32)u8Buf[2], (s32)u8Buf[3]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_ADDR2, u8Buf, SIX_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_ADDR2 is <0x%02x%02x %02x%02x%02x%02x>\n",
	(s32)u8Buf[0], (s32)u8Buf[1], (s32)u8Buf[2], (s32)u8Buf[3], (s32)u8Buf[4], (s32)u8Buf[5]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_ARL_CTRL_PAGE, MULTIPORT_VECTOR2, u8Buf, FOUR_BYTE_WIDTH));
	printf("ETHSW_ARL_CTRL_PAGE MULTIPORT_VECTOR2 is <0x%02x%02x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1],
	(s32)u8Buf[2], (s32)u8Buf[3]);
	/* Display ETHSW_PVLAN_PAGE */
	for (u8PortId = 0; u8PortId < MAX_USED_PORTS; u8PortId++)
	{
		RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PVLAN_PAGE, (u8)PVLAN_PORT(u8PortId), u8Buf, TWO_BYTE_WIDTH));
		printf("ETHSW_PVLAN_PAGE PVLAN_PORT(%d) is <0x%02x%02x>\n", u8PortId, (s32)u8Buf[0], (s32)u8Buf[1]);
	}
	
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_PVLAN_PAGE, PVLAN_IMP, u8Buf, TWO_BYTE_WIDTH));
	printf("ETHSW_PVLAN_PAGE PVLAN_IMP is <0x%02x%02x>\n", (s32)u8Buf[0], (s32)u8Buf[1]);

	/* Display ETHSW_QVLAN_PAGE */
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL0, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL0 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL1, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL1 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL2, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL2 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL3, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL3 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL4, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL4 is <0x%02x>\n", (s32)u8Buf[0]);
	RETURN_IF_ERROR(ethsw_read_reg(ETHSW_QVLAN_PAGE, VLAN_CTRL5, u8Buf, ONE_BYTE_WIDTH));
	printf("ETHSW_VLAN_PAGE VLAN_CTRL5 is <0x%02x>\n", (s32)u8Buf[0]);

	/* Display ETHSW_PHY_PAGE,to be done */
	return (E_ETHSW_OK);
	
}
s32 ethsw_clear_port_mib(void)
{
    u8 u8data;
	u8data = 1;
    RETURN_IF_ERROR(ethsw_write_reg(0x02, 0, &u8data, ONE_BYTE_WIDTH));
	u8data = 0;
    RETURN_IF_ERROR(ethsw_write_reg(0x02, 0, &u8data, ONE_BYTE_WIDTH));
	return (E_ETHSW_OK);
}
s32 ethsw_set_jumbo(void)
{
    u8 u8data[4];
	u8data[0] = 0xff;
	u8data[1] = 0x0;
	u8data[2] = 0x0;
	u8data[3] = 0x01;
    RETURN_IF_ERROR(ethsw_write_reg(0x40, 1, u8data, FOUR_BYTE_WIDTH));
	return (E_ETHSW_OK);
}
void ethsw_help(void)
{
    printf("1.ethsw_set_port_mirror(u8MirrorEnable,u8MirrorRules,u8MirroredPort,u8MirrorPort)\n");
    printf("    u8MirrorEnable: 0-disable;1-enable;\n");
    printf("    u8MirrorRules: 1-Ingress;2-Egress;3-Ingress andEgress;\n");
    printf("2.ethsw_get_port_mib(u8PortId)\n");
    printf("3.ethsw_clear_port_mib(void)\n");
	printf("4.ethsw_display_all_reg(void)\n");
    printf("5.ethsw_get_status(u8PortId)\n");
    printf("6.ethsw_set_status(u8PortId,u8TxEnable,u8RxEnable)\n");
    printf("    u8TxEnable: 0-disable;1-enable;\n");
    printf("    u8RxEnable: 0-disable;1-enable;\n");
    printf("7.ethsw_set_jumbo(void)\n");
    return;
}

/******************************* Դ�ļ����� ********************************/

