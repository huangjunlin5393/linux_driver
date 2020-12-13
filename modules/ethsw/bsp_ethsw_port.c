/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           Bsp_ethsw_port.c 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
/************************** �����ļ����� **********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>

/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"
#include "bsp_ethsw_bcm5389.h"
#include "bsp_ethsw_port.h"
#include "bsp_ethsw_mdio.h"

/******************************* �ֲ��궨�� *********************************/


/*********************** ȫ�ֱ�������/��ʼ�� **************************/
extern u32 g_u32PrintRules;  /* ��ӡ���� */


/************************** �ֲ����������Ͷ��� ************************/



/*************************** �ֲ�����ԭ������ **************************/

/************************************ ����ʵ�� ************************* ****/

/*******************************************************************************
* ��������: ethsw_set_status
* ��������: ��ɶ˿ڽ��ܺͷ��͹��ܵ�����
* ����ĵ�:
* ��������:
* ��������:     ����        ����/���   ����
* u8PortId      u8          ����        �˿�ID
* u8TxEnable    u8          ����        ����ʹ��
* u8RxEnable    u8          ����        ����ʹ��
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
* 
*******************************************************************************/
s32 ethsw_set_status(u8 u8PortId, u8 u8TxEnable, u8 u8RxEnable)
{
    STRU_ETHSW_PORT_CTRL  struPortCtrl; /* �˿ڿ��ƼĴ����ṹ */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                   (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));
    struPortCtrl.StpState = 5; /* 0:unmanaged mode, do not use STP;101 = Forwarding State */
    /* �˼Ĵ���Ϊ�շ�����Disable�Ĵ���,��ʱ�Ĵ�����0,�ر�ʱ��1 */
    struPortCtrl.TxDisable = (1 - u8TxEnable);  /* ���÷���bit */
    struPortCtrl.RxDisable = (1 - u8RxEnable);  /* ���ý���bit */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                    (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));/* ����д�� */
    return (E_ETHSW_OK);
}

/*******************************************************************************
* ��������: ethsw_get_status
* ��������: ��ɻ�ȡ�˿�LINK״̬�Ĺ���
* ����ĵ�:
* ��������:
* ��������:       ����        ����/���   ����
* u8PortId        u8          ����        �˿�ID
* pu8PortStatus   u8 *        ���        �˿�״̬
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
s32 ethsw_get_status(u8 u8PortId, u8 *pu8PortStatus)
{
    u8   u8Val[8];         /* �ֲ����� */
    STRU_ETHSW_PORT_CTRL    struPortCtrl;   /* �˿ڿ��ƼĴ����ṹ */
    STRU_ETHSW_SWITCH_MODE  struSwitchMode; /* ����ģʽ�Ĵ����ṹ */

    /* ��ȡоƬ�˿��շ����ƼĴ�����ֵ */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, PORT_TRAFIC_CTRL(u8PortId),
                                   (u8 *)&struPortCtrl, ONE_BYTE_WIDTH));
    /* �˼Ĵ���Ϊ�շ�����Disable�Ĵ���,��ʱ�Ĵ�����0,�ر�ʱ��1 */
    if (0 != struPortCtrl.TxDisable)
    {
        /* PORT TX FAIL */
        if (NULLPTR == pu8PortStatus) /*���Ϊ������д�ӡ*/
        {
            printf("\nPort[%d] Tx Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN; /* ������м�¼ */
        }
        return (E_ETHSW_OK); /* ֱ�ӷ��� */
    }

    if (0 != struPortCtrl.RxDisable)
    {
        /* PORT RX FAIL */
        if (NULLPTR == pu8PortStatus) /*���Ϊ������д�ӡ*/
        {
            printf("\nPort[%d] Rx Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN; /* ������м�¼ */
        }
        return (E_ETHSW_OK);   /* ֱ�ӷ��� */
    }

    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_CTRL_PAGE, SWITCH_MODE, (u8 *)&struSwitchMode, ONE_BYTE_WIDTH));
    if (0 == struSwitchMode.SwFwdEn)
    {
        if (NULLPTR == pu8PortStatus)/*���Ϊ������д�ӡ*/
        {
            printf("\nPort[%d] FWD Disable!\n", u8PortId);
        }
        else
        {
            *pu8PortStatus = PORT_LINK_DOWN;  /* ������м�¼ */
        }
        return (E_ETHSW_OK); /* ֱ�ӷ��� */
    }

    /* ��ȡ�˿�LINK״̬�Ĵ��� */
    RETURN_IF_ERROR(ethsw_read_reg(ETHSW_STAT_PAGE, LINK_STATUS_SUMMARY, u8Val, TWO_BYTE_WIDTH));
    //printf("ETHSW_STAT_PAGE LINK_STATUS_SUMMARY is <0x%02x%02x>\n", (s32)u8Val[0], (s32)u8Val[1]);
    if(0 != (u8Val[0]&(1<<u8PortId)))
    {
                /* PORT OK */
            if (NULLPTR == pu8PortStatus) /*���Ϊ������д�ӡ*/
            {
                printf("\nPort[%d] is Link Up!\n", u8PortId);
            }
            else
            {
                *pu8PortStatus = PORT_LINK_UP; /* ������м�¼ */
            }
    }
    else
    {
            /* PORT FAIL */
            if (NULLPTR == pu8PortStatus)/*���Ϊ������д�ӡ*/
            {
                printf("\nPort[%d] is Link Down!\n", u8PortId);
            }
            else
            {
                *pu8PortStatus = PORT_LINK_DOWN;  /* ������м�¼ */
            }
    }
    return (E_ETHSW_OK); /* ��󷵻� */
}

s32 ethsw_set_mirror(u8 u8MirrorEnable, u8 u8MirrorRules, u8 u8MirroredPort, u8 u8MirrorPort)
{
    STRU_ETHSW_MIRROR_CTRL      struMirrorCtrl;         /* Mirror������������ƹ��� */
    STRU_ETHSW_MIRROR_CAP_CTRL  struMirrorCapCtrl;      /* Mirror������ƽṹ */

    /* ����ʹ��MIRROR����ʱ����Ҫ����Ingress/Egress Mirror���ƼĴ���  */
    if (1 == u8MirrorEnable)
    {
        struMirrorCtrl.MirroredPort = (1 << u8MirroredPort);
        struMirrorCtrl.filter = 0x0;
        struMirrorCtrl.DividEnable = 0x0;  /* �������������� */
        struMirrorCtrl.rsvd4 = 0;          /* �����ֶ���Ϊ0 */

        	
        /* ������Trunking������صļĴ���,�����ṩֻMirrorĳMAC��ַ,��һ�����Mirror�ȹ��� */
        /* INGRESS_MIRROR_DIVIDER, INGRESS_MIRROR_MAC_ADDR,EGRESS_MIRROR_DIVIDER,EGRESS_MIRROR_MAC_ADDR */
        if (0x1 == u8MirrorRules) /* INGRESS���� */
        {
            /* ����Ingress Mirror���ƼĴ��� */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
            struMirrorCtrl.MirroredPort = 0;
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
        }
        else if (0x2 == u8MirrorRules)/* EGRESS���� */
        {
            /* ����Egress Mirror���ƼĴ��� */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
            struMirrorCtrl.MirroredPort = 0;
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
        }
        else if (0x3 == u8MirrorRules) /* ˫���� */
        {
            /* ����Egress Mirror���ƼĴ��� */
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, EGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
            RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, INGRESS_MIRROR_CTRL, (u8 *)&struMirrorCtrl, TWO_BYTE_WIDTH));
        }
    }

    /* ����Mirror Capture�˿�,ֹ�ܻ�ʹ��Mirror���� */
    struMirrorCapCtrl.BlkNoMirror = 0;               /* ��������Mirror���� */
    struMirrorCapCtrl.MirrorEnable = u8MirrorEnable;     /* Mirror����ֹ�ܻ�ʹ�� */
    struMirrorCapCtrl.CapturePort =u8MirrorPort;// (1 << u8MirrorPort); /* ���ò���˿� */
    struMirrorCapCtrl.rsvd10 = 0;                         /* �����ֶ���Ϊ0 */
    RETURN_IF_ERROR(ethsw_write_reg(ETHSW_MGMT_PAGE, MIRROR_CAPTURE_CTRL, (u8 *)&struMirrorCapCtrl, TWO_BYTE_WIDTH));
    return (E_ETHSW_OK);
}



