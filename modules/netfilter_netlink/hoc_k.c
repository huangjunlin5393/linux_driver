#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_arp.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/inet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <linux/socket.h>/*PF_INET*/
#include <linux/if_packet.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/semaphore.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <net/arp.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>
//#include <netinet/ether.h>

#include "hoc_k.h"

/* ----------��̬����---------- */
static struct nf_hook_ops 	g_aNfHookOps[2];
static int 				g_UserPid = 0;     //�û�̬����ID
static struct sock               *g_kNlFd  = NULL;
 s32 u32_ind= 0, u32_rcv = 0, u32_rcv_all= 0;


 
static STRU_IP_MAC_LOCAL_USER_TABLE    Hoc_ip_mac_user_Table;  //����hub����
static STRU_ROUTE_INFO_RSP                    Hoc_route_hub_Info, Hoc_route_hub_Info_B;    //ȫ��·��
static STRU_USR_INT_IP_RSP                     Hoc_ip_mac_All_INfo; //ȫ��ip  mac
static STRU_RTE_DATA_IND                       Hoc_RTE_DATA_IND;

static u8 u8route_flag = 0;

int AGTK_SendToUser(u8* pu8UserData, u32 u32UserDataLen) //���͵��û��ռ�
{
    int size;
    struct sk_buff *skb;
    unsigned char *old_tail;
    struct nlmsghdr *pNlHd; //����ͷ
    
    int retval;
    
    if((0==g_kNlFd) || (0==g_UserPid))
    {
        printk("%s: g_kNlFd=%p, g_UserPid=%d\n", __func__, g_kNlFd, g_UserPid);
        return -1;
    }
    
    size = sizeof(struct nlmsghdr) + u32UserDataLen; //���Ĵ�С
    skb = alloc_skb(size, GFP_ATOMIC); //����һ���µ��׽��ֻ���,ʹ��GFP_ATOMIC��־���̲�>�ᱻ��Ϊ˯��
    
    //��ʼ��һ��netlink��Ϣ�ײ�
    pNlHd = nlmsg_put(skb, 0, 0, 0, u32UserDataLen, 0);
    old_tail = skb->tail;
    memcpy((u8*)pNlHd + sizeof(struct nlmsghdr), pu8UserData, u32UserDataLen); //���������
    pNlHd->nlmsg_len = sizeof(struct nlmsghdr) + u32UserDataLen; //������Ϣ����
    
    //���ÿ����ֶ�
    
    NETLINK_CB(skb).creds.pid = 0; //��openwrt 3.3.8���ں��еİ汾
    
    
    NETLINK_CB(skb).dst_group = 0;
    
    //��������
    retval = netlink_unicast(g_kNlFd, skb, g_UserPid, MSG_DONTWAIT);
    if(retval <0)
    {
        printk("%s: netlink_unicast = %d\n", __func__, retval);
    }
    
    return 0;
}



// 0: same
// else: not same
int AGTK_MacAddrCmp(const u8* pu8Mac1, const u8* pu8Mac2)
{
    u32 u32i=0;
    for(u32i=0; u32i<6; u32i++)
    {
        if(*(pu8Mac1+u32i) != *(pu8Mac2+u32i))
        {
            return 1;
        }
    }
    
    return 0;
}

// 1:on bridge
// 0:not on bridge
int AGTK_IsIpOnBridge(u32 u32Ip)
{
    u32 u32Num = Hoc_ip_mac_user_Table.u32Num;
    u32 u32i=0;
    
    for(u32i=0; u32i<u32Num; u32i++)
    {
        if(Hoc_ip_mac_user_Table.astruUserDetail[u32i].u32UserIp == u32Ip)
        {
            printk("%s: u32Num=%d, ip %08X\n", __func__, u32Num, u32Ip);
            return 1;
        }
    }
    
    return 0;
}

// 1, changed
// 0, no change
// -1, full
int AGTK_UpdateLocalUserInfo(const struct sk_buff* skb, const struct net_device *in)
{
    STRU_ARP_HEAD* pArpHd = (STRU_ARP_HEAD*)arp_hdr(skb);
    
    u32 u32Num = Hoc_ip_mac_user_Table.u32Num;
    u32 u32i=0;
    s32 s32Kflag  = -1;
    
    if(*(u32*)(pArpHd->ar_sip) == 0)
    {
        return -1;
    }
    
    for(u32i=0; u32i<u32Num; u32i++)
    {
	 if(Hoc_ip_mac_user_Table.astruUserDetail[u32i].u32UserIp == 0)
	{
		s32Kflag = u32i;
	 }
	
	 if(Hoc_ip_mac_user_Table.astruUserDetail[u32i].u32UserIp == *(u32*)(pArpHd->ar_sip))
        {
            // no change
            if((0 == AGTK_MacAddrCmp(Hoc_ip_mac_user_Table.astruUserDetail[u32i].au8UserMac, pArpHd->ar_sha)) &&
                    (0 == AGTK_MacAddrCmp(Hoc_ip_mac_user_Table.astruUserDetail[u32i].au8LinkIfaceMac, in->dev_addr)))
            {
                Hoc_ip_mac_user_Table.astruUserDetail[u32i].s32TimeToLive = USER_LIVE_TIME;
                return 0;
            }
            else //replace
            {
                memcpy(Hoc_ip_mac_user_Table.astruUserDetail[u32i].au8UserMac, pArpHd->ar_sha, 6);
                memcpy(Hoc_ip_mac_user_Table.astruUserDetail[u32i].au8LinkIfaceMac, in->dev_addr, 6);
                Hoc_ip_mac_user_Table.astruUserDetail[u32i].s32TimeToLive = USER_LIVE_TIME;
                return 1;
            }
        }
        
        
    }
    
    // check if enough
    if(Hoc_ip_mac_user_Table.u32Num+1 > AGT_MAX_LOCAL_USER_NUM)
    {
        printk("%s: u32Num=%d > AGT_MAX_LOCAL_USER_NUM\n", __func__, Hoc_ip_mac_user_Table.u32Num);
        return -1;
    }
  //  printk("add new ip %x\n", pArpHd->ar_sip);
    // add new ip
    if(s32Kflag < 0)
    {
	s32Kflag = u32Num;
    }
	
    Hoc_ip_mac_user_Table.astruUserDetail[s32Kflag].u32UserIp = *(u32*)(pArpHd->ar_sip);
    memcpy(Hoc_ip_mac_user_Table.astruUserDetail[s32Kflag].au8UserMac, pArpHd->ar_sha, 6);
    memcpy(Hoc_ip_mac_user_Table.astruUserDetail[s32Kflag].au8LinkIfaceMac, in->dev_addr, 6);
    Hoc_ip_mac_user_Table.astruUserDetail[s32Kflag].s32TimeToLive = USER_LIVE_TIME;
    
    Hoc_ip_mac_user_Table.u32Num++;
    
    return 1;
    
}
int AGTK_NetFilter_onArpMsg(const struct sk_buff * skb, const struct net_device *in)
{
    
    u8 au8Mac1Buf[20];
    u8 au8Mac2Buf[20];
    u8 au8Ip1Buf[20];
    u8 au8Ip2Buf[20];
    
    
    struct ethhdr* pEth = eth_hdr(skb);
    STRU_ARP_HEAD* pArpHead = (STRU_ARP_HEAD*)((u8*)pEth + 14);
    
    
    
    
    memset(au8Mac1Buf, 0, sizeof(au8Mac1Buf));
    memset(au8Mac2Buf, 0, sizeof(au8Mac2Buf));
    memset(au8Ip1Buf, 0, sizeof(au8Ip1Buf));
    memset(au8Ip2Buf, 0, sizeof(au8Ip2Buf));
    
    
    
    // �����û���Ϣ�����仯�������AGTU
    AGTK_UpdateLocalUserInfo(skb, in);
    
    Hoc_ip_mac_user_Table.u32AgtMsgId = O_KER_USR_IP_MAC_INFO;
    if(-1 == AGTK_SendToUser((u8*)&Hoc_ip_mac_user_Table, sizeof(STRU_IP_MAC_LOCAL_USER_TABLE)))
    {
        printk("%s: -1 == AGTK_SendToUser struMsg\n", __func__);
    }
    
    /*-----------------------------------------------------------*/
    
    // �ж��Ƿ�Ϊ����������Ϣ
    // arm��mips������armΪС�ˣ�mipsΪ��ˣ���������
    if((MANAGER_IP_U32 == MESH_NTOHL(*(u32*)(pArpHead->ar_tip))) ||
            (LOOP_IP_U32 == MESH_NTOHL(*(u32*)(pArpHead->ar_tip)))	)
    {
        return NF_ACCEPT;
    }
    
    // ARP request
    // arm��mips������armΪС�ˣ�mipsΪ��ˣ���������
    if(0x0001 == MESH_NTOHS(pArpHead->ar_op))
    {
        // �Ƿ�Ϊspecial ARP
        if(0 == *(u32*)(pArpHead->ar_sip))
        {
            return NF_DROP;
        }
        
        // �Ƿ�Ϊgratuitous ARP
        if(*(u32*)(pArpHead->ar_sip) == *(u32*)(pArpHead->ar_tip))
        {
            return NF_DROP;
        }
        
        // ���Ŀ��IP����Bridge�ϣ����������Ƿ��������У�һ�ɻ���Ŀ��MAC��ַΪ��MESH�豸��MAC��ַ
        if(0 == AGTK_IsIpOnBridge(*(u32*)(pArpHead->ar_tip)))
        {
            
            arp_send(ARPOP_REPLY,
                     ETH_P_ARP,
                     *(u32*)(pArpHead->ar_sip),
                     skb->dev,
                     *(u32*)(pArpHead->ar_tip),
                     pArpHead->ar_sha,
                     in->dev_addr,
                     pArpHead->ar_sha);
            //printk("%s, %d\n", __func__, __LINE__);
        }
    }
    
    return NF_DROP;
}

/*
*netfilter����arp
*
*/
u32 AGTK_Hook_Fun_Arp(// const struct nf_hook_ops *ops, //��ubuntu-linux-3.13.0-32-generic���ں��еİ汾
                      const struct nf_hook_ops *ops,    // ��openwrt 3.3.8���ں��еİ汾
                      struct sk_buff * skb,
                      const struct net_device *in,
                      const struct net_device *out,
                      int (*okfn)(struct sk_buff *))
{
    // struct ethhdr* pEthHdr;
    
    if(skb == NULL)
    {
        return NF_ACCEPT;
    }
    if(strcmp(in->name, "eth0") == 0)
    {
   
        return AGTK_NetFilter_onArpMsg(skb, in);
        
    }
    /*********wifi���յ���arp*****************/
   printk("wlan0 recv an arp pack\n");
    
    return NF_ACCEPT;
    
}



// �ú����������IP��Port���ձ���˳��
int AGTK_SendRteAsUdp(struct net_device *odev,
					  u16 u16LocalPort,
					  u32 u32RemoteIp,
					  u16 u16RemotePort,
					  u8 *au8Msg,
					  int s32DataLen)
{
    struct sk_buff *skb;
    int s32TotalLen, s32IpLen, s32UdpLen, s32HeaderLen;
    struct udphdr *pUdpHead;
    struct iphdr  *pIpHead;
    struct ethhdr *pEthHead;
    u32 u32LocalIp = MANAGER_IP_U32;
    u8  au8RemoteMac[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};

   
    // ���ø���Э�����ݳ���
    s32UdpLen = s32DataLen + sizeof(*pUdpHead);
    s32IpLen  = s32UdpLen + sizeof(*pIpHead);
    s32TotalLen =  s32IpLen + ETH_HLEN + NET_IP_ALIGN; //©��ip_len�����ش���
    s32HeaderLen = s32TotalLen - s32DataLen;

    // ����skb
    skb = alloc_skb( s32TotalLen + LL_MAX_HEADER, GFP_ATOMIC );
    if ( !skb )
    {
        return -1;
    }

    // Ԥ�ȱ���skb��Э���ײ����ȴ�С
    skb_reserve(skb, LL_MAX_HEADER + s32HeaderLen);

    // ������������
    skb_copy_to_linear_data(skb, au8Msg, s32DataLen);
    skb->len = s32DataLen;

    // skb->data �ƶ���udp�ײ�
    skb_push(skb, sizeof(*pUdpHead));
    skb_reset_transport_header(skb);
    pUdpHead = udp_hdr(skb);
    pUdpHead->source = MESH_HTONS(u16LocalPort);
    pUdpHead->dest   = MESH_HTONS(u16RemotePort);
    pUdpHead->len    = MESH_HTONS(s32UdpLen);
    pUdpHead->check  = 0;
    pUdpHead->check  = csum_tcpudp_magic(MESH_HTONL(u32LocalIp), /* u32���˳�� */
            						     MESH_HTONL(u32RemoteIp),/* u32���˳�� */
            						     skb->len, /* ���ȣ�����˳�� */
            						     IPPROTO_UDP,
            						     csum_partial(pUdpHead, s32UdpLen, 0));
    if (pUdpHead->check == 0)
    {
    	pUdpHead->check = CSUM_MANGLED_0;
    }

    // skb->data �ƶ���ip�ײ�
    skb_push(skb, sizeof(*pIpHead));
    skb_reset_network_header(skb);
    pIpHead = ip_hdr(skb);

    /* pIpHead->version = 4; pIpHead->ihl = 5; */
    put_unaligned(0x45, (unsigned char *)pIpHead);
    pIpHead->tos      = 1|(1<<5);  //priority&&max throught
    put_unaligned(MESH_HTONS(s32IpLen), &(pIpHead->tot_len));
    pIpHead->id       = 0;  /*u16*/
    pIpHead->frag_off = 0;  /*u16*/
    pIpHead->ttl      = 64; /*8*/
    pIpHead->protocol = IPPROTO_UDP; /*8*/
    pIpHead->check    = 0;  /*u16*/
    put_unaligned(MESH_HTONL(u32LocalIp), &(pIpHead->saddr));
    put_unaligned(MESH_HTONL(u32RemoteIp), &(pIpHead->daddr));
    pIpHead->check    = ip_fast_csum((unsigned char *)pIpHead, pIpHead->ihl);

    //printk("dev_queue_xmit   pIpHead->check = %d\n", pIpHead->check);

    // skb->data �ƶ���eth�ײ�
    pEthHead = (struct ethhdr *) skb_push(skb, ETH_HLEN);
    skb_reset_mac_header(skb);
    skb->protocol = pEthHead->h_proto = MESH_HTONS(ETH_P_IP);
    memcpy(pEthHead->h_source, odev->dev_addr, ETH_ALEN);
    memcpy(pEthHead->h_dest, au8RemoteMac, ETH_ALEN);

    skb->dev = odev;

    // ֱ�ӷ���
    if(dev_queue_xmit(skb)<0)
    {
	free_skb(skb);
	printk("dev_queue_xmit failed\n");
    }
	
   //printk("/**********kernel  send ind: %d***datalen:%d*********\n", ++u32_ind , s32DataLen);


    return 0;
}




void AGTK_onMsg_UK_TX_RTE_DATA(u8* pu8UserData)
{
	STRU_TX_RTE_DATA* pMsg = (STRU_TX_RTE_DATA*)pu8UserData;
	struct net_device *odev;
	

	odev = dev_get_by_name(&init_net, "wlan0");

// �ú����������IP��Port���ձ���˳��
	AGTK_SendRteAsUdp(odev, 0x1122, BCAST_IP_U32, 0x3344, pMsg->au8RteData, pMsg->u32Len);
		
	
}
void AGTK_onUsr_ARP_INFO(u8 *pu8UserData)
{
	u8 u8i;
	u8 u8Num = Hoc_ip_mac_user_Table.u32Num;
	struct net_device * dev = NULL;

	if(u8Num == 0)
		return;
	
	
	for(u8i=0; u8i<16; u8i++)
	{
		

		//printk("Hoc_ip_mac_user_Table.astruUserDetail[%d].s32TimeToLive :%d\n",  u8i,Hoc_ip_mac_user_Table.astruUserDetail[u8i].s32TimeToLive);
		/***********delete offline ip**************/
		if((Hoc_ip_mac_user_Table.astruUserDetail[u8i].s32TimeToLive <= 0)&&(Hoc_ip_mac_user_Table.astruUserDetail[u8i].u32UserIp != 0))
		{
			printk("USER IP(%08X) offline\n", Hoc_ip_mac_user_Table.astruUserDetail[u8i].u32UserIp);
			memset(&Hoc_ip_mac_user_Table.astruUserDetail[u8i], 0 , sizeof(Hoc_ip_mac_user_Table.astruUserDetail[u8i]));
			Hoc_ip_mac_user_Table.u32Num--;	

			Hoc_ip_mac_user_Table.u32AgtMsgId = O_KER_USR_IP_MAC_INFO;
    			if(-1 == AGTK_SendToUser((u8*)&Hoc_ip_mac_user_Table, sizeof(STRU_IP_MAC_LOCAL_USER_TABLE)))
   			 {
       			 printk("%s: -1 == AGTK_SendToUser struMsg\n", __func__);
    			}

		}
		else if(Hoc_ip_mac_user_Table.astruUserDetail[u8i].s32TimeToLive > 0)
		{
			Hoc_ip_mac_user_Table.astruUserDetail[u8i].s32TimeToLive -= 2;
		}
		

		
		dev = dev_get_by_name(&init_net, "eth0");
		if(NULL == dev)
		{
			return;
		}

		if((Hoc_ip_mac_user_Table.astruUserDetail[u8i].s32TimeToLive <= 1)&&(Hoc_ip_mac_user_Table.astruUserDetail[u8i].u32UserIp != 0))
		{
		//	printk("arp_send ready!!\n");
			arp_send(ARPOP_REQUEST,  ETH_P_ARP,  Hoc_ip_mac_user_Table.astruUserDetail[u8i].u32UserIp, 
				dev, MESH_HTONL(MANAGER_IP_U32), 
				Hoc_ip_mac_user_Table.astruUserDetail[u8i].au8UserMac, 
				Hoc_ip_mac_user_Table.astruUserDetail[u8i].au8LinkIfaceMac, 0);
		//	printk("arp_send go!!\n");
			
		}

		
	}
	
	
}


// �����û�̬����������Ϣ
void AGTK_UserMsgMain(u32 u32Pid, u8* pu8UserData, u32 u32UserDataLen)
{
    u32 u32AgtMsgId = 0;

    if((NULL == pu8UserData) || (0 == u32UserDataLen) )
    {
        printk("%s�� ERROR, pu8UserData=%08X, u32UserDataLen=%d\n",__func__, (u32)pu8UserData, u32UserDataLen);
        return;
    }
    
    u32AgtMsgId = *(u32*)pu8UserData;
    
    switch(u32AgtMsgId)
    {
    case O_USR_KER_ROUTE_INFO:
			if(u8route_flag == 0)
			{
			memcpy(&Hoc_route_hub_Info_B,  pu8UserData, u32UserDataLen);
			u8route_flag =1;
			}
			else
			{
			memcpy(&Hoc_route_hub_Info,  pu8UserData, u32UserDataLen);
			u8route_flag =0;
			}

        break;
    case O_USR_KER_NET_IP_INFO:
       memcpy(&Hoc_ip_mac_All_INfo,  pu8UserData, u32UserDataLen);
        break;
    case O_USR_KER_DATA_IND:
        AGTK_onMsg_UK_TX_RTE_DATA(pu8UserData);
		break;
    case O_USR_KER_ARP_TIMER:
        AGTK_onUsr_ARP_INFO(pu8UserData);
        break;
    default:
        break;
    }
    
    return;
}



void AGTK_RecvFromUser(struct sk_buff *__skb) //�ں˴��û��ռ��������
{
    struct sk_buff *skb;
    struct nlmsghdr *pNlHd = NULL;
    u8* pu8UserData = NULL;
    u32 u32UserDataLen = 0;
    
    skb = skb_get(__skb);

    
    if(skb->len < sizeof(struct nlmsghdr))
    {
        printk("%s: skb->len < sizeof(struct nlmsghdr)\n", __func__);
        return;
    }
    
    pNlHd = (struct nlmsghdr *)skb->data;
    
    if(skb->len < pNlHd->nlmsg_len)
    {
        printk("%s: skb->len < pNlHd->nlmsg_len\n", __func__);
        return;
    }
    
    if(pNlHd->nlmsg_len < sizeof(struct nlmsghdr))
    {
        printk("%s: pNlHd->nlmsg_len < sizeof(struct nlmsghdr)\n", __func__);
        return;
    }
    
    g_UserPid = pNlHd->nlmsg_pid;


    
    pu8UserData 	= (u8*)pNlHd + sizeof(struct nlmsghdr);
    u32UserDataLen  = pNlHd->nlmsg_len - sizeof(struct nlmsghdr);
    
    AGTK_UserMsgMain(pNlHd->nlmsg_pid, pu8UserData, u32UserDataLen);
    
    kfree_skb(skb);
}


// 1: in net, 0:not
int AGTK_IsIpInNet(u32 u32Ip, /*out*/u8* pu8FinalNodeMac)
{
    u32 u32i=0;
    u32 u32Num = Hoc_ip_mac_All_INfo.u32Num;
    
    for(u32i=0; u32i<u32Num; u32i++)
    {
        if(u32Ip == Hoc_ip_mac_All_INfo.astruUsrInfo[u32i].u32UserIp)
        {
            memcpy(pu8FinalNodeMac, Hoc_ip_mac_All_INfo.astruUsrInfo[u32i].au8UserMac, 6);
            return 1;
        }
    }
    
    return 0;
}




int AGTK_RedirectIpPacket(struct sk_buff *skb, unsigned char* pu8DevName, unsigned char* pu8DstMac, unsigned char* pu8SrcMac)
{
    
    struct ethhdr* pEthHead = NULL;
    
    int ret;
    
    if(NULL == skb)
    {
        return NF_ACCEPT;
    }
    
    if(NULL == pu8DevName)
    {
        return NF_ACCEPT;
    }
    
    // Only Change Dev Info And Mac
    skb->dev 		= dev_get_by_name(&init_net, pu8DevName);
    
    // �ı�MAC��ַ
    pEthHead = eth_hdr(skb);
    memcpy(pEthHead->h_dest, pu8DstMac, 6);
    memcpy(pEthHead->h_source, pu8SrcMac, 6);
    
    // ����
    skb_push(skb, ETHALEN);//��skb->dataָ��l2�㣬֮�����ݰ�ͨ��dev_queue_xmit()����
    
    // check
    if(*(u16*)(skb->data + 12) != MESH_HTONS(0x0806) &&
            *(u16*)(skb->data + 12) != MESH_HTONS(0x0800))
    {
        printk("%s: skb->data wrong!!!\n", __func__);
        return NF_DROP;
    }
	
    ret = dev_queue_xmit(skb);
    if(ret < 0)
    {
        printk("%s: dev_queue_xmit < 0\n", __func__);
        dev_put(skb->dev);
        kfree_skb(skb);
        return NF_DROP;
    }
    else
    {
        return NF_STOLEN;
    }
    
    
}



u32 AGTK_Hook_Fun_Ip_PreRoute(// const struct nf_hook_ops *ops, //��ubuntu-linux-generic-3.1.3���ں��еİ汾
                              const struct nf_hook_ops *ops,    // ��openwrt 3.3.8���ں��еİ汾
                              struct sk_buff * skb,
                              const struct net_device *in,
                              const struct net_device *out,
                              int (*okfn)(struct sk_buff *))
{
    struct ethhdr* pEthHead = eth_hdr(skb);
    struct iphdr* pIpHead 	= ip_hdr(skb);
    const u8 au8BcastMac[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0};
    s32 s32Ret = 0;
 //  s32 s32i;
    u32 u32Num,u32i=0;
    
    
	
    u8 au8FinaNodeMac[8];
    u8 au8DstIfaceMac[8];
    u8 au8SrcIfaceMac[8];

#if 0

printk("���յ�IP��\n");



printk("MESH_NTOHL--->pIpHead->daddr:%x\n",  MESH_NTOHL(pIpHead->daddr));
printk("pIpHead->saddr:%x\n",  (pIpHead->saddr));


#endif

#if 1
    /***************��Ϣ�ֽ�********************/
    // �ж��Ƿ�ΪSSHЭ��
    // arm��mips������armΪС�Σ�mipsΪ��Σ���������
    if(IPPROTO_TCP == pIpHead->protocol/*u8*/) // tcp
    {
        struct tcphdr* pTcpHead= tcp_hdr(skb);
        if((22 == MESH_NTOHS(pTcpHead->dest)/*u16*/) || (22 == MESH_NTOHS(pTcpHead->source)/*u16*/))
        {
            return NF_ACCEPT;
        }
    }



    // �ж��Ƿ�Ϊ����������Ϣ*** �������ò���
    // arm��mips������armΪС�ˣ�mipsΪ��ˣ���������
    if((MANAGER_IP_U32 == MESH_NTOHL(pIpHead->daddr)/*u32*/) || (LOOP_IP_U32 == MESH_NTOHL(pIpHead->daddr)/*u32*/))
    {
        return NF_ACCEPT;
    }
    
    
    // �ж��Ƿ�ΪRTE�㲥, 192.168.1.255 special addr
    // arm��mips������armΪС�ˣ�mipsΪ��ˣ���������
    if((BCAST_IP_U32 == MESH_NTOHL(pIpHead->daddr)/*u32*/) && (IPPROTO_UDP == pIpHead->protocol /*u8*/))
    {
        
       struct udphdr* pUdpHead = udp_hdr(skb); //��Ȼ����udp_hdr���ƺ��õ�����IP�ײ�����ʱ��ͨһ�£��Ժ���˼��ԭ��		
/* printk("1_pUdpHead->source: %x\n", MESH_NTOHS(pUdpHead->source));
    printk("1_pUdpHead->source: %x\n", MESH_NTOHS(pUdpHead->dest));
	   pUdpHead = (struct udphdr*)(20+(u8*)pUdpHead);
  printk("2_pUdpHead->source: %x\n", MESH_NTOHS(pUdpHead->source));
    printk("2_pUdpHead->source: %x\n", MESH_NTOHS(pUdpHead->dest)); */
	if((0x1122 == MESH_NTOHS(pUdpHead->source)/*u16*/) && (0x3344 == MESH_NTOHS(pUdpHead->dest)/*u16*/))		
            
        {
           //printk("%s: ���յ�RTE�㲥\n", __func__);
            Hoc_RTE_DATA_IND.u32AgtMsgId = O_KER_USR_RTE_DATA_IND;
            memcpy(Hoc_RTE_DATA_IND.au8SrcMac,  pEthHead->h_source, 6);
            memcpy(Hoc_RTE_DATA_IND.au8LocalRxInterfaceMac, in->dev_addr, 6);
	     Hoc_RTE_DATA_IND.u32Len  = MESH_NTOHS(pUdpHead->len /*u16*/) - sizeof(struct udphdr);
            memcpy(Hoc_RTE_DATA_IND.au8RteData, (u8*)pUdpHead + sizeof(struct udphdr), Hoc_RTE_DATA_IND.u32Len);
			
            AGTK_SendToUser((u8*)&Hoc_RTE_DATA_IND, Hoc_RTE_DATA_IND.u32Len+24);
          
           
            
            return NF_DROP;
        }
	
    }
    
    
    
   
    // �㲥��Ϣ���ظ�Э��ջ
    if(0 == AGTK_MacAddrCmp(pEthHead->h_dest, au8BcastMac))
    {
        return NF_ACCEPT;
    }

    // ת����Ϣ������Ǹ���HUB������
    if(0 != AGTK_MacAddrCmp(pEthHead->h_dest, in->dev_addr))
    {
        return NF_DROP;
    }
     
    // �ж�Ŀ��IP�Ƿ���������  ��ȫ��IP_mac�������IP
    s32Ret = AGTK_IsIpInNet(pIpHead->daddr, au8FinaNodeMac);
    if(0 == s32Ret)
    {
        printk("%s: IP(%08X) Not In Net \n", __func__, pIpHead->daddr);
/*		for(s32Ret=0; s32Ret<4; s32Ret++)
		{
printk("%d:  IP(%08X)    ", s32Ret,  Hoc_ip_mac_All_INfo.astruUsrInfo[s32Ret].u32UserIp);
printk("%02X-%02X-%02X-%02X-%02X-%02X-\n   ",   Hoc_ip_mac_All_INfo.astruUsrInfo[s32Ret].au8UserMac[0],
	Hoc_ip_mac_All_INfo.astruUsrInfo[s32Ret].au8UserMac[1],
	Hoc_ip_mac_All_INfo.astruUsrInfo[s32Ret].au8UserMac[2],
	Hoc_ip_mac_All_INfo.astruUsrInfo[s32Ret].au8UserMac[3],
	Hoc_ip_mac_All_INfo.astruUsrInfo[s32Ret].au8UserMac[4],
	Hoc_ip_mac_All_INfo.astruUsrInfo[s32Ret].au8UserMac[5]);
		} */
        return NF_DROP;
    }
    
    
    // �ж�Ŀ��IP�Ƿ�Ϊ��HUB�������û��豸
    if(AGTK_MacAddrCmp(pEthHead->h_dest, au8FinaNodeMac) == 0)
    {
        
       // printk(" ת�������ڵ�Ĺ����û�\n");
        //��Ϊ��̫�����Ͳ鱾������MAC
        u32i=0;
        u32Num = Hoc_ip_mac_All_INfo.u32Num;
        
        for(u32i=0; u32i<u32Num; u32i++)
        {
            if(pIpHead->daddr == Hoc_ip_mac_user_Table.astruUserDetail[u32i].u32UserIp)
            {
			memcpy(au8DstIfaceMac, Hoc_ip_mac_user_Table.astruUserDetail[u32i].au8UserMac, 6);
                    memcpy(au8SrcIfaceMac, Hoc_ip_mac_user_Table.astruUserDetail[u32i].au8LinkIfaceMac, 6);
			break;
            }
        }
        if(u32i == u32Num)
        {
		printk("����û��IP(%08X)\n",  pIpHead->daddr);
		return NF_DROP;
    	 }
        
        return AGTK_RedirectIpPacket(skb, "eth0", au8DstIfaceMac, au8SrcIfaceMac);
        
    }
    else
    {
        u32i=0;
	//u8 flag = u8route_flag;
        u32Num = Hoc_route_hub_Info.u32NumOfRoute;
/*
		 printk(" �����м�ת��һ��\n");
		 printk("ԭʼ��ַ:%02x-%02x-%02x-%02x-%02x-%02x\n", pEthHead->h_source[0],pEthHead->h_source[1],pEthHead->h_source[2],pEthHead->h_source[3],pEthHead->h_source[4],pEthHead->h_source[5]);
		 printk("Ŀ�ĵ�ַ:%02x-%02x-%02x-%02x-%02x-%02x\n", au8FinaNodeMac[0],au8FinaNodeMac[1],au8FinaNodeMac[2],au8FinaNodeMac[3],au8FinaNodeMac[4],au8FinaNodeMac[5]);
	
*/		
		for(u32i=0; u32i<u32Num; u32i++)
       	 {
			if(u8route_flag == 0)
			{
				if(AGTK_MacAddrCmp(au8FinaNodeMac, Hoc_route_hub_Info.astruRouteInfo[u32i].au8DstAddr) == 0)
            			{
            		   
 				 memcpy(au8DstIfaceMac, Hoc_route_hub_Info.astruRouteInfo[u32i].au8NextIfaceAddr, 6);
               		 memcpy(au8SrcIfaceMac, Hoc_route_hub_Info.astruRouteInfo[u32i].au8InterfaceAddr, 6);
					 break;
			    
               		
            			}
			}
			else
			{
				if(AGTK_MacAddrCmp(au8FinaNodeMac, Hoc_route_hub_Info_B.astruRouteInfo[u32i].au8DstAddr) == 0)
            			{
            		   
 				 memcpy(au8DstIfaceMac, Hoc_route_hub_Info_B.astruRouteInfo[u32i].au8NextIfaceAddr, 6);
               		 memcpy(au8SrcIfaceMac, Hoc_route_hub_Info_B.astruRouteInfo[u32i].au8InterfaceAddr, 6);
					 break;
			    
               		
            			}
			}
			
        	}
		if(u32i == u32Num)
        	{
		printk("����û��ȥ%02x-%02x-%02x-%02x-%02x-%02x����һ��\n",  au8FinaNodeMac[0],\
			au8FinaNodeMac[1],au8FinaNodeMac[2],au8FinaNodeMac[3],au8FinaNodeMac[4],au8FinaNodeMac[5]);
#if 0
if(flag == 0)
{
		printk("u32NumOfRoute:%d\n", Hoc_route_hub_Info.u32NumOfRoute);
for(u32i=0; u32i<Hoc_route_hub_Info.u32NumOfRoute; u32i++)
{
printk("au8DstAddr:0x%02x          ",Hoc_route_hub_Info.astruRouteInfo[u32i].au8DstAddr[5]);
printk("au8NextIfaceAddr:0x%02x\n",Hoc_route_hub_Info.astruRouteInfo[u32i].au8NextIfaceAddr[5]);
}
}
else
{		printk("u32NumOfRoute:%d\n", Hoc_route_hub_Info_B.u32NumOfRoute);

	for(u32i=0; u32i<Hoc_route_hub_Info_B.u32NumOfRoute; u32i++)
	{
	printk("au8DstAddr:0x%02x          ",Hoc_route_hub_Info_B.astruRouteInfo[u32i].au8DstAddr[5]);
	printk("au8NextIfaceAddr:0x%02x\n",Hoc_route_hub_Info_B.astruRouteInfo[u32i].au8NextIfaceAddr[5]);
	}
}
#endif
		return NF_DROP;
    	 	}
			
	

//	printk("��һ����ַ:%02x-%02x-%02x-%02x-%02x-%02x\n", au8DstIfaceMac[0],au8DstIfaceMac[1],au8DstIfaceMac[2],au8DstIfaceMac[3],au8DstIfaceMac[4],au8DstIfaceMac[5]);
        return AGTK_RedirectIpPacket(skb, "wlan0", au8DstIfaceMac, au8SrcIfaceMac);
    }
	
  #endif




		printk("%s: %d ��Ȼ�ߵ�����\n", __func__, __LINE__);
        return NF_ACCEPT;

}





int AGTK_Netfilter_Init(void)
{
    int ret;
    g_aNfHookOps[0].hook 	 = AGTK_Hook_Fun_Ip_PreRoute;
    g_aNfHookOps[0].pf 		 = AF_INET ;
    g_aNfHookOps[0].hooknum  = NF_INET_PRE_ROUTING;
    g_aNfHookOps[0].priority = NF_IP_PRI_FIRST;
    
    g_aNfHookOps[1].hook 	 = AGTK_Hook_Fun_Arp;
    g_aNfHookOps[1].pf 	 	 = NFPROTO_ARP; // ubuntu-linux-generic-3.1.3 & openwrt 3.3.8 �����
    g_aNfHookOps[1].hooknum  = NF_ARP_IN;
    g_aNfHookOps[1].owner    = THIS_MODULE;
    g_aNfHookOps[1].priority = NF_IP_PRI_FILTER;


	
    ret = nf_register_hooks(g_aNfHookOps, ARRAY_SIZE(g_aNfHookOps));
    if(ret < 0)
    {
        printk("nf_register_hooks fail!");
        return ret;
    }
    
    printk("netfilter_init!\n");
    return 0;
}

void AGTK_Netfilter_Exit(void)
{
    nf_unregister_hooks(g_aNfHookOps, ARRAY_SIZE(g_aNfHookOps));
    printk(KERN_DEBUG "netfilter_exit!\n");
    return;
}

int AGTK_Netlink_Init(void)
{
/*
	u8  test_info[48] = {0xc0,0xa8,0x01, 0x04, 0x38, 0xA2, 0x8C,0x6F,0xB8,0x2D,0x00,0x00,\
					 0xc0,0xa8,0x01, 0x03,0x38, 0xA2, 0x8C, 0x6F,0xB7,0xBC,0x00,0x00,\
					 0xc0,0xa8,0x01, 0x7a,0x38, 0xA2, 0x8C, 0x6F,0xB8,0x44,0x00,0x00,\
					 0xc0,0xa8,0x01, 0x2f,0x38, 0xA2, 0x8C, 0x6F,0xB8,0x30,0x00,0x00};
	u32 u32i =0;
*/
	struct netlink_kernel_cfg cfg = {  
        .input = AGTK_RecvFromUser,  
    };
    g_kNlFd = netlink_kernel_create(&init_net, NETLINK_AGT, &cfg); // ��openwrt 3.3.8���ں��еİ汾
    //netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg)

    if(NULL == g_kNlFd)
    {
        printk(KERN_ERR "can not create a netlink socket\n");
        return -1;
    }
/*******test   start********/
//	Hoc_ip_mac_All_INfo.u32Num = 4;
/*
memcpy(&Hoc_ip_mac_All_INfo.astruUsrInfo[0].u32UserIp, test_info, 4);
memcpy(Hoc_ip_mac_All_INfo.astruUsrInfo[0].au8UserMac, test_info+4, 8);
memcpy(&Hoc_ip_mac_All_INfo.astruUsrInfo[1], test_info+12, 12);
memcpy(&Hoc_ip_mac_All_INfo.astruUsrInfo[2], test_info+24, 12);
memcpy(&Hoc_ip_mac_All_INfo.astruUsrInfo[3], test_info+36, 12);

for(u32i=0; u32i<4; u32i++)
{
	memcpy(&(Hoc_ip_mac_All_INfo.astruUsrInfo[u32i].u32UserIp), test_info+u32i*12, 4);
	memcpy(Hoc_ip_mac_All_INfo.astruUsrInfo[u32i].au8UserMac, test_info+u32i*12+4, 8);

}*/
 /*********test end**************/   
    printk("netlink_init!\n");
    return 0;
}

void AGTK_Netlink_Exit(void)
{
    sock_release(g_kNlFd->sk_socket);
    printk(KERN_DEBUG "netlink_exit!\n");
}

int AGTK_Init(void)
{
    memset(&Hoc_ip_mac_user_Table, 0, sizeof(STRU_IP_MAC_LOCAL_USER_TABLE ));
    Hoc_ip_mac_user_Table.u32AgtMsgId = O_KER_USR_IP_MAC_INFO;
    
    
    
    AGTK_Netfilter_Init();
    AGTK_Netlink_Init();
    
    
    printk("km init\n");
    
    return 0;
}

void AGTK_Exit(void)
{
    AGTK_Netfilter_Exit();
    AGTK_Netlink_Exit();
    
    printk("km exit\n");
    return;
}
module_init(AGTK_Init);
module_exit(AGTK_Exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huang");




