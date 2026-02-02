/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _RECV_OSDEP_C_

#include <drv_types.h>
#include <mld.h>

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
#ifdef CONFIG_RTW_A4_STA
extern int check_srcmac_in_fdb_for_ax_driver(struct net_device *dev, const unsigned char *addr);
#endif
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
#include <linux/if_vlan.h>
extern int linux_vlan_enable;
extern linux_vlan_ctl_t *vlan_ctl_p;
#endif

int rtw_os_recvframe_duplicate_skb(_adapter *padapter,
			union recv_frame *pcloneframe, struct sk_buff *pskb)
{
	int res = _SUCCESS;
	struct sk_buff *pkt_copy = NULL;

	if (pskb == NULL) {
		RTW_INFO("%s [WARN] skb == NULL, drop frag frame\n", __func__);
		return _FAIL;
	}
#if 1
	pkt_copy = rtw_skb_copy(pskb);

	if (pkt_copy == NULL) {
		RTW_INFO("%s [WARN] rtw_skb_copy fail , drop frag frame\n", __func__);
		return _FAIL;
	}
#else
	pkt_copy = rtw_skb_clone(pskb);

	if (pkt_copy == NULL) {
		RTW_INFO("%s [WARN] rtw_skb_clone fail , drop frag frame\n", __func__);
		return _FAIL;
	}
#endif
	pkt_copy->dev = padapter->pnetdev;

	pcloneframe->u.hdr.pkt = pkt_copy;
	pcloneframe->u.hdr.rx_head = pkt_copy->head;
	pcloneframe->u.hdr.rx_data = pkt_copy->data;
	pcloneframe->u.hdr.rx_end = skb_end_pointer(pkt_copy);
	pcloneframe->u.hdr.rx_tail = skb_tail_pointer(pkt_copy);
	pcloneframe->u.hdr.len = pkt_copy->len;

	return res;
}

int rtw_os_alloc_recvframe(_adapter *padapter,
		union recv_frame *precvframe, u8 *pdata, struct sk_buff *pskb)
{
	int res = _SUCCESS;
	u8	shift_sz = 0;
	u32	skb_len, alloc_sz;
	struct sk_buff	*pkt_copy = NULL;
	struct rx_pkt_attrib *pattrib = &precvframe->u.hdr.attrib;


	if (pdata == NULL) {
		precvframe->u.hdr.pkt = NULL;
		res = _FAIL;
		return res;
	}


	/*	Modified by Albert 20101213 */
	/*	For 8 bytes IP header alignment. */
	shift_sz = pattrib->qos ? 6 : 0; /*	Qos data, wireless lan header length is 26 */

	skb_len = pattrib->pkt_len;

	/* for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet. */
	/* modify alloc_sz for recvive crc error packet by thomas 2011-06-02 */
	if ((pattrib->mfrag == 1) && (pattrib->frag_num == 0)) {
		/* alloc_sz = 1664;	 */ /* 1664 is 128 alignment. */
		alloc_sz = (skb_len <= 1650) ? 1664 : (skb_len + 14);
	} else {
		alloc_sz = skb_len;
		/*	6 is for IP header 8 bytes alignment in QoS packet case. */
		/*	8 is for skb->data 4 bytes alignment. */
		alloc_sz += 14;
	}

	pkt_copy = rtw_skb_alloc(alloc_sz);

	if (pkt_copy) {
		pkt_copy->dev = padapter->pnetdev;
		pkt_copy->len = skb_len;
		precvframe->u.hdr.pkt = pkt_copy;
		precvframe->u.hdr.rx_head = pkt_copy->head;
		precvframe->u.hdr.rx_end = pkt_copy->data + alloc_sz;
		skb_reserve(pkt_copy, 8 - ((SIZE_PTR)(pkt_copy->data) & 7));  /* force pkt_copy->data at 8-byte alignment address */
		skb_reserve(pkt_copy, shift_sz);/* force ip_hdr at 8-byte alignment address according to shift_sz. */
		_rtw_memcpy(pkt_copy->data, pdata, skb_len);
		precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
	} else {

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
		RTW_INFO("%s:can not allocate memory for skb copy\n", __func__);

		precvframe->u.hdr.pkt = NULL;

		/* rtw_free_recvframe(precvframe, pfree_recv_queue); */
		/*exit_rtw_os_recv_resource_alloc;*/

		res = _FAIL;
#else
		if ((pattrib->mfrag == 1) && (pattrib->frag_num == 0)) {
			RTW_INFO("%s: alloc_skb fail , drop frag frame\n", __FUNCTION__);
			/* rtw_free_recvframe(precvframe, pfree_recv_queue); */
			res = _FAIL;
			goto exit_rtw_os_recv_resource_alloc;
		}

		if (pskb == NULL) {
			res = _FAIL;
			goto exit_rtw_os_recv_resource_alloc;
		}

		precvframe->u.hdr.pkt = rtw_skb_clone(pskb);
		if (precvframe->u.hdr.pkt) {
			precvframe->u.hdr.pkt->dev = padapter->pnetdev;
			precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pdata;
			precvframe->u.hdr.rx_end =  pdata + alloc_sz;
		} else {
			RTW_INFO("%s: rtw_skb_clone fail\n", __FUNCTION__);
			/* rtw_free_recvframe(precvframe, pfree_recv_queue); */
			/*exit_rtw_os_recv_resource_alloc;*/
			res = _FAIL;
		}
#endif
	}

exit_rtw_os_recv_resource_alloc:

	return res;

}

void rtw_os_free_recvframe(union recv_frame *precvframe)
{
	if (precvframe->u.hdr.pkt) {
		/* ToDo: Instead of free RX SKB, put it back to RX pool ? */
		rtw_skb_free(precvframe->u.hdr.pkt);
		precvframe->u.hdr.pkt = NULL;
	}
}

/* init os related resource in struct recv_priv */
int rtw_os_recv_resource_init(struct recv_priv *precvpriv, _adapter *padapter)
{
	int	res = _SUCCESS;


#ifdef CONFIG_RTW_NAPI
	skb_queue_head_init(&precvpriv->rx_napi_skb_queue);
#endif /* CONFIG_RTW_NAPI */

	return res;
}

/* alloc os related resource in union recv_frame */
int rtw_os_recv_resource_alloc(_adapter *padapter, union recv_frame *precvframe)
{
	int	res = _SUCCESS;

	precvframe->u.hdr.pkt = NULL;

	return res;
}

/* free os related resource in union recv_frame */
void rtw_os_recv_resource_free(struct recv_priv *precvpriv)
{
	sint i;
	union recv_frame *precvframe;
	precvframe = (union recv_frame *) precvpriv->precv_frame_buf;


#ifdef CONFIG_RTW_NAPI
	if (skb_queue_len(&precvpriv->rx_napi_skb_queue))
		RTW_WARN("rx_napi_skb_queue not empty\n");
	rtw_skb_queue_purge(&precvpriv->rx_napi_skb_queue);
#endif /* CONFIG_RTW_NAPI */

	for (i = 0; i < NR_RECVFRAME; i++) {
		rtw_os_free_recvframe(precvframe);
		precvframe++;
	}
}

/* alloc os related resource in struct recv_buf */
int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf)
{
	int res = _SUCCESS;

#ifdef CONFIG_USB_HCI
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);
	struct usb_device	*pusbd = dvobj_to_usb(pdvobjpriv)->pusbdev;
#endif

	precvbuf->irp_pending = _FALSE;
	precvbuf->purb = usb_alloc_urb(0, GFP_KERNEL);
	if (precvbuf->purb == NULL)
		res = _FAIL;

	precvbuf->pskb = NULL;

	precvbuf->pallocated_buf  = precvbuf->pbuf = NULL;

	precvbuf->pdata = precvbuf->phead = precvbuf->ptail = precvbuf->pend = NULL;

	precvbuf->transfer_len = 0;

	precvbuf->len = 0;

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
	precvbuf->pallocated_buf = rtw_usb_buffer_alloc(pusbd, (size_t)precvbuf->alloc_sz, &precvbuf->dma_transfer_addr);
	precvbuf->pbuf = precvbuf->pallocated_buf;
	if (precvbuf->pallocated_buf == NULL)
		return _FAIL;
#endif /* CONFIG_USE_USB_BUFFER_ALLOC_RX */

#endif /* CONFIG_USB_HCI */

	return res;
}

/* free os related resource in struct recv_buf */
int rtw_os_recvbuf_resource_free(_adapter *adapter, struct recv_buf *precvbuf)
{
	int ret = _SUCCESS;

#ifdef CONFIG_USB_HCI

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX

	struct dvobj_priv	*dvobj = adapter_to_dvobj(adapter);
	struct usb_device	*pusbd = dvobj_to_usb(dvobj)->pusbdev;

	rtw_usb_buffer_free(pusbd, (size_t)precvbuf->alloc_sz, precvbuf->pallocated_buf, precvbuf->dma_transfer_addr);
	precvbuf->pallocated_buf =  NULL;
	precvbuf->dma_transfer_addr = 0;

#endif /* CONFIG_USE_USB_BUFFER_ALLOC_RX */

	if (precvbuf->purb) {
		/* usb_kill_urb(precvbuf->purb); */
		usb_free_urb(precvbuf->purb);
	}

#endif /* CONFIG_USB_HCI */


	if (precvbuf->pskb) {
#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
		if (rtw_free_skb_premem(precvbuf->pskb) != 0)
#endif
			rtw_skb_free(precvbuf->pskb);
	}
	return ret;

}

struct sk_buff *rtw_os_alloc_msdu_pkt(union recv_frame *prframe,
		const u8 *da, const u8 *sa, u8 *msdu ,u16 msdu_len)
{
	u16	eth_type;
	u8	*data_ptr;
	struct sk_buff *sub_skb;
	struct rx_pkt_attrib *pattrib;

	pattrib = &prframe->u.hdr.attrib;

#ifdef CONFIG_SKB_COPY
	sub_skb = rtw_skb_alloc(msdu_len + 14);
	if (sub_skb) {
		skb_reserve(sub_skb, 14);
		data_ptr = (u8 *)skb_put(sub_skb, msdu_len);
		_rtw_memcpy(data_ptr, msdu, msdu_len);
	} else
#endif /* CONFIG_SKB_COPY */
	{
		sub_skb = rtw_skb_clone(prframe->u.hdr.pkt);
		if (sub_skb) {
			sub_skb->data = msdu;
			sub_skb->len = msdu_len;
			skb_set_tail_pointer(sub_skb, msdu_len);
		} else {
			RTW_INFO("%s(): rtw_skb_clone() Fail!!!\n", __FUNCTION__);
			return NULL;
		}
	}

	eth_type = RTW_GET_BE16(&sub_skb->data[6]);

	if (sub_skb->len >= 8
		&& ((_rtw_memcmp(sub_skb->data, rtw_rfc1042_header, SNAP_SIZE)
				&& eth_type != ETH_P_AARP && eth_type != ETH_P_IPX)
			|| _rtw_memcmp(sub_skb->data, rtw_bridge_tunnel_header, SNAP_SIZE))
	) {
		/* remove RFC1042 or Bridge-Tunnel encapsulation and replace EtherType */
		skb_pull(sub_skb, SNAP_SIZE);
		_rtw_memcpy(skb_push(sub_skb, ETH_ALEN), sa, ETH_ALEN);
		_rtw_memcpy(skb_push(sub_skb, ETH_ALEN), da, ETH_ALEN);
	} else {
		/* Leave Ethernet header part of hdr and full payload */
		u16 len;

		len = htons(sub_skb->len);
		_rtw_memcpy(skb_push(sub_skb, 2), &len, 2);
		_rtw_memcpy(skb_push(sub_skb, ETH_ALEN), sa, ETH_ALEN);
		_rtw_memcpy(skb_push(sub_skb, ETH_ALEN), da, ETH_ALEN);
	}

	return sub_skb;
}

#ifdef CONFIG_RTW_NAPI
static int napi_recv(_adapter *padapter, int budget)
{
	struct sk_buff *pskb;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	int work_done = 0;
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	u8 rx_ok;


	while ((work_done < budget) &&
	       (!skb_queue_empty(&precvpriv->rx_napi_skb_queue))) {
		pskb = skb_dequeue(&precvpriv->rx_napi_skb_queue);
		if (!pskb)
			break;

		rx_ok = _FALSE;

#ifdef CONFIG_RTW_GRO
		if (pregistrypriv->en_gro) {
			if (rtw_napi_gro_receive(&padapter->napi, pskb) != GRO_DROP)
				rx_ok = _TRUE;
			goto next;
		}
#endif /* CONFIG_RTW_GRO */

		if (rtw_netif_receive_skb(padapter->pnetdev, pskb) == NET_RX_SUCCESS)
			rx_ok = _TRUE;

next:
		if (rx_ok == _TRUE) {
			work_done++;
			DBG_COUNTER(padapter->rx_logs.os_netif_ok);
		} else {
			DBG_COUNTER(padapter->rx_logs.os_netif_err);
		}
	}

	return work_done;
}

int rtw_recv_napi_poll(struct napi_struct *napi, int budget)
{
	_adapter *padapter = container_of(napi, _adapter, napi);
	int work_done = 0;
	struct recv_priv *precvpriv = &padapter->recvpriv;


	work_done = napi_recv(padapter, budget);
	if (work_done < budget) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)) && defined(CONFIG_PCI_HCI)
		napi_complete_done(napi, work_done);
#else
		napi_complete(napi);
#endif
		if (!skb_queue_empty(&precvpriv->rx_napi_skb_queue))
			napi_schedule(napi);
	}

	return work_done;
}

#ifdef CONFIG_RTW_NAPI_DYNAMIC
void dynamic_napi_th_chk (_adapter *adapter)
{

	if (adapter->registrypriv.en_napi) {
		struct dvobj_priv *dvobj;
		struct registry_priv *registry;

		dvobj = adapter_to_dvobj(adapter);
		registry = &adapter->registrypriv;
		if (dvobj->traffic_stat.cur_rx_tp > registry->napi_threshold)
			dvobj->en_napi_dynamic = 1;
		else
			dvobj->en_napi_dynamic = 0;
	}

}
#endif /* CONFIG_RTW_NAPI_DYNAMIC */
#endif /* CONFIG_RTW_NAPI */

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
static inline unsigned char* rtw_get_skb_ip_header(struct sk_buff *skb, unsigned char v4)
{
	unsigned char *ptr=NULL;
	if(!skb || !skb->data) return NULL;

	ptr = (unsigned char *)(skb->data + 2*ETH_ALEN);

	if(*(unsigned short *)(ptr) == __constant_htons(ETH_P_8021Q))
		ptr += 4;
	if(*(unsigned short *)(ptr) == __constant_htons(ETH_P_IP))
		return (v4 == 1) ? (ptr + 2) : NULL;
	else if(*(unsigned short *)(ptr) == __constant_htons(ETH_P_IPV6))
		return (v4 == 0) ? (ptr + 2) : NULL;
	else
		return NULL;
}
static inline __u32 rtw_get_ip(__u32 *pip)
{
	__u32 dip;
	__u16 *pdip=(__u16 *)&dip;
	__u16 *psip=(__u16 *)pip;
	*pdip++=*psip++;
	*pdip++=*psip++;
	return __constant_ntohl(dip);
}
static unsigned int rtw_check_mcastL2L3Diff(struct sk_buff *skb)
{
	unsigned int DaIpAddr;
	struct iphdr *iph = (struct iphdr *)rtw_get_skb_ip_header(skb, 1);
	if(!iph) return 0;

#ifdef PLATFORM_ECOS
	DaIpAddr = get_unaligned((unsigned int *)&(iph->daddr));
#else
	DaIpAddr = rtw_get_ip(&iph->daddr);
#endif
	//printk("ip:%d, %d ,%d ,%d\n",(DaIpAddr>>24) ,(DaIpAddr<<8)>>24,(DaIpAddr<<16)>>24,(DaIpAddr<<24)>>24);

	if (((DaIpAddr & 0xFF000000) >= 0xE0000000) && ((DaIpAddr & 0xFF000000) <= 0xEF000000))
	{
		if (!IP_MCAST_MAC(skb->data))
			return DaIpAddr;
	}
	return 0;
}

static void rtw_convertMCastIPtoMMac(unsigned int group, unsigned char *gmac)
{
	unsigned int u32tmp, tmp;
	static int i;

	u32tmp = group & 0x007FFFFF;
	gmac[0] = 0x01;
	gmac[1] = 0x00;
	gmac[2] = 0x5e;

	for (i=5; i>=3; i--) {
		tmp = u32tmp & 0xFF;
		gmac[i] = tmp;
		u32tmp >>= 8;
	}
}

static void rtw_convertMCastIPv6toMMac(unsigned char* icmpv6_McastAddr, unsigned char *gmac)
{
	/*ICMPv6 valid addr 2^32 -1*/
	gmac[0] = 0x33;
	gmac[1] = 0x33;
	gmac[2] = icmpv6_McastAddr[12];
	gmac[3] = icmpv6_McastAddr[13];
	gmac[4] = icmpv6_McastAddr[14];
	gmac[5] = icmpv6_McastAddr[15];
}

static void rtw_checkUDPandU2M(struct sk_buff *pskb)
{
	int MultiIP;

	MultiIP = rtw_check_mcastL2L3Diff(pskb);
	if (MultiIP)
	{
		unsigned char mactmp[6];
		rtw_convertMCastIPtoMMac(MultiIP, mactmp);
		//printk("%02x%02x%02x:%02x%02x%02x\n", mactmp[0],mactmp[1],mactmp[2],
		//      mactmp[3],mactmp[4],mactmp[5]);
		memcpy(pskb->data, mactmp, 6);
#if defined(__LINUX_2_6__)
	/*added by qinjunjie,warning:should not remove next line*/
	pskb->pkt_type = PACKET_MULTICAST;
#endif
	}
}

static void rtw_checkV6UDPandU2M(struct sk_buff *pskb)
{
	struct ipv6hdr *iph;
	unsigned char *DDA;

	iph = (struct ipv6hdr *)rtw_get_skb_ip_header(pskb, 0);
	DDA = (unsigned char *)pskb->data;

	if(!iph || !DDA) return;

	/*ip(v6) format is multicast ip*/
	if (iph->daddr.s6_addr[0] == 0xff)
	{
		/*mac is not ipv6 multicase mac*/
		if(!ICMPV6_MCAST_MAC(DDA) )
		{
			/*change mac (DA) to (ipv6 multicase mac) format by (ipv6 multicast ip)*/
			DDA[0] = 0x33;
			DDA[1] = 0x33;
			memcpy(DDA+2, &iph->daddr.s6_addr[12], 4);
		}
	}
}

void rtw_update_sta_mcast_entry(struct sta_priv *pstapriv, u8 *mac, u8 *mcmac, bool add)
{
	_adapter *padapter = pstapriv->padapter;
	struct sta_info *psta = NULL;
	u8 i;
	bool found = _FALSE;

	psta = rtw_get_stainfo(pstapriv, mac);
#ifdef CONFIG_RTW_A4_STA
	if (padapter->a4_enable == 1 && psta == NULL) {
		psta = core_a4_get_fwd_sta(padapter, mac);
	}
#endif

	if (!psta)
		return;

	for (i = 0; i < psta->ipmc_num; i++) {
		if (_rtw_memcmp(psta->ipmc[i].mcmac, mcmac, 6) == _TRUE) {
			found = _TRUE;
			break;
		}
	}

	if (found == _TRUE) {
		if (add == _FALSE) {
			RTW_INFO("[%s] remove %pM from sta=%pM mcmac entry\n", __func__, mcmac, mac);
			if (i != (psta->ipmc_num - 1))
				_rtw_memcpy(psta->ipmc[i].mcmac, psta->ipmc[psta->ipmc_num - 1].mcmac, 6);
			psta->ipmc_num--;
		}
	} else {
		if (add == _TRUE) {
			if (psta->ipmc_num < MAX_IP_MC_ENTRY) {
				RTW_INFO("[%s] add %pM to sta=%pM mcmac entry\n", __func__, mcmac, mac);
				_rtw_memcpy(psta->ipmc[psta->ipmc_num].mcmac, mcmac, 6);
				psta->ipmc_num++;
			}
		}
	}
}

bool rtw_in_multicast(u32 addr)
{
	return ((addr & 0xf0000000) == 0xe0000000);
}

void rtw_igmp3_report_check(struct sta_priv	*pstapriv, struct sk_buff *pkt, struct igmphdr *igmph)
{
	struct igmpv3_report *igmpv3 = (struct igmpv3_report *)igmph;
	struct igmpv3_grec *igmpv3grec = &igmpv3->grec[0];
	struct ethhdr *eth = (struct ethhdr *)pkt->data;
	u16 rec_id = 0, srcnum;
	u32 group, op = 0;
	u8 mcmac[6] = {0};
	u16 loop_cnt = ntohs(igmpv3->ngrec);

	if (loop_cnt > 0xFFFF) {    // for coverity check
		RTW_ERR("loop_cnt > 0xFFFF\n");
		return;
	}

	if(pkt->len < (sizeof(struct igmpv3_grec)*loop_cnt)){
		RTW_PRINT("pkt len:%d, sz:%zd, loop:%d\n",
			pkt->len, sizeof(struct igmpv3_grec), loop_cnt);
		return;
	}

	while (rec_id < loop_cnt)
	{
		group = be32_to_cpu(igmpv3grec->grec_mca);
		srcnum = ntohs(igmpv3grec->grec_nsrcs);

		if (rtw_in_multicast(group)) {
			switch(igmpv3grec->grec_type) {
			case IGMPV3_MODE_IS_INCLUDE:
			case IGMPV3_CHANGE_TO_INCLUDE:
				op = (srcnum == 0) ? 2 : 1;
				break;
			case IGMPV3_MODE_IS_EXCLUDE:
			case IGMPV3_CHANGE_TO_EXCLUDE:
			case IGMPV3_ALLOW_NEW_SOURCES:
				op = 1;
				break;
			case IGMPV3_BLOCK_OLD_SOURCES:
				op = 2;
				break;
			default:
				break;
			}
		}

		if (op != 0) {
			rtw_convertMCastIPtoMMac(group, mcmac);
			rtw_update_sta_mcast_entry(pstapriv, eth->h_source, mcmac, (op == 1) ? _TRUE : _FALSE);
			op = 0;
		}

		rec_id++;
		igmpv3grec = (struct igmpv3_grec *)((u8*)igmpv3grec + sizeof(struct igmpv3_grec) + (igmpv3grec->grec_auxwords + srcnum) * sizeof(u32));
	}
}

void rtw_igmp_report_check(struct sta_priv *pstapriv, struct sk_buff *pkt, struct igmphdr *igmph)
{
	u32 group, op = 0;
	struct ethhdr *eth = (struct ethhdr *)pkt->data;
	u8 mcmac[6] = {0};

	group = be32_to_cpu(igmph->group);
	if (!rtw_in_multicast(group))
		return;

	if (igmph->type == IGMP_HOST_MEMBERSHIP_REPORT ||
		igmph->type == IGMPV2_HOST_MEMBERSHIP_REPORT)
		op = 1;
	else if (igmph->type == IGMP_HOST_LEAVE_MESSAGE)
		op = 2;

	if (op) {
		rtw_convertMCastIPtoMMac(group, mcmac);
		rtw_update_sta_mcast_entry(pstapriv, eth->h_source, mcmac, (op == 1) ? _TRUE : _FALSE);
	}
}

void rtw_igmp_type_check(struct sta_priv	*pstapriv, struct sk_buff *pkt)
{
	struct iphdr *iph = NULL;
	struct igmphdr *igmph = NULL;
	u8 hdrlen;
	u16 tot_len;
	u32 group;

	iph = (struct iphdr *)rtw_get_skb_ip_header(pkt, 1);

	if (!iph || iph->protocol != IPPROTO_IGMP)
		return;

	hdrlen = iph->ihl << 2;
	if (iph->version != 4 || hdrlen < 20)
		return;

	/*if (ip_fast_csum((u8 *)iph, iph->ihl) != 0)
		return;*/

	tot_len = ntohs(iph->tot_len);
	if (pkt->len < tot_len || tot_len < hdrlen)
		return;

	igmph = (struct igmphdr *)((u8*)iph + hdrlen);

	if (igmph->type == IGMPV3_HOST_MEMBERSHIP_REPORT)
		rtw_igmp3_report_check(pstapriv, pkt, igmph);
	else
		rtw_igmp_report_check(pstapriv, pkt, igmph);
}

void rtw_mld_icmpv6_report_check(struct sta_priv	*pstapriv, struct sk_buff *pkt, u8 *ptr)
{
	struct ethhdr *eth = (struct ethhdr *)pkt->data;
	u8 gmac[6] = {0};
	u8 op = 0;
	u16 srcnum = 0;

	if (ptr[0] == ICMPV6_MLD2_REPORT) {
		struct mld2_grec *grec = NULL;
		u16 grpnum = 0;
		int i;

		grpnum = (ptr[6] << 8) + ptr[7];

		if (grpnum > 0xFFFF) { // For coverity check
			RTW_ERR("grpnum > 0xFFFF\n");
			return;
		}

		grec = (struct mld2_grec *)(ptr + 8);

		for (i = 0; i < grpnum; i++) {
			srcnum = ntohs(grec->grec_nsrcs);
			rtw_convertMCastIPv6toMMac((unsigned char *)&grec->grec_mca, gmac);
			if ((grec->grec_type == MLD2_CHANGE_TO_EXCLUDE) || (grec->grec_type == MLD2_MODE_IS_EXCLUDE) || (grec->grec_type == MLD2_ALLOW_NEW_SOURCES))
				op = 1;
			else if ((grec->grec_type == MLD2_CHANGE_TO_INCLUDE) || (grec->grec_type == MLD2_MODE_IS_INCLUDE))
				op = (srcnum == 0) ? 2 : 1;
			else if(grec->grec_type == MLD2_BLOCK_OLD_SOURCES)
				op = 2;

			if (op != 0) {
				rtw_update_sta_mcast_entry(pstapriv, eth->h_source, gmac, (op == 1) ? _TRUE : _FALSE);
				op = 0;
			}
			grec = (struct mld2_grec *)((u8*)grec + sizeof(struct mld2_grec) + srcnum * sizeof(struct in6_addr));
		}
	} else if (ptr[0] == ICMPV6_MGM_REPORT) {
		rtw_convertMCastIPv6toMMac(ptr + 8, gmac);
		rtw_update_sta_mcast_entry(pstapriv, eth->h_source, gmac, _TRUE);
	} else if (ptr[0] == ICMPV6_MGM_REDUCTION) {
		rtw_convertMCastIPv6toMMac(ptr + 8, gmac);
		rtw_update_sta_mcast_entry(pstapriv, eth->h_source, gmac, _FALSE);
	}
}

void rtw_mld_type_check(struct sta_priv	*pstapriv, struct sk_buff *pkt)
{
	struct ipv6hdr *ipv6h;
	u8 *ptr;
	u8 *startPtr = NULL;
	u8 *lastPtr = NULL;
	u8 nextHeader = 0;
	u16 extensionHdrLen = 0;

	ptr = rtw_get_skb_ip_header(pkt, 0);

	ipv6h = (struct ipv6hdr *)ptr;

	if (!ipv6h || ipv6h->version != 6)
		return;

	startPtr = ptr;
	lastPtr = startPtr + sizeof(struct ipv6hdr) + ntohs(ipv6h->payload_len);
	nextHeader = ipv6h->nexthdr;
	ptr = startPtr + sizeof(struct ipv6hdr);

	while (ptr < lastPtr) {
		switch (nextHeader) {
			case NEXTHDR_HOP:
				/*parse hop-by-hop option*/
				nextHeader = ptr[0];
				extensionHdrLen = ((u16)(ptr[1]) + 1) * 8;
				ptr = ptr + extensionHdrLen;
				break;

			case NEXTHDR_ROUTING:
				nextHeader = ptr[0];
				extensionHdrLen = ((u16)(ptr[1]) + 1) * 8;
				ptr = ptr + extensionHdrLen;
				break;

			case NEXTHDR_FRAGMENT:
				nextHeader = ptr[0];
				ptr = ptr + 8;
				break;

			case NEXTHDR_DEST:
				nextHeader = ptr[0];
				extensionHdrLen = ((u16)(ptr[1]) + 1) * 8;
				ptr = ptr + extensionHdrLen;
				break;

			case NEXTHDR_ICMP:
				rtw_mld_icmpv6_report_check(pstapriv, pkt, ptr);
				return;
			default:
				return;
		}
	}
}

int rtw_process_u2mc(_adapter *padapter, struct sk_buff *pkt)
{
	int ret = _SUCCESS;
	unsigned short L3_protocol = 0;
	struct sta_priv	*pstapriv = &padapter->stapriv;

	if(!pkt || !pkt->data)
		return _FAIL;

	L3_protocol = *(unsigned short *)(pkt->data + 2*ETH_ALEN);
	if(L3_protocol == __constant_htons(ETH_P_8021Q))
		L3_protocol = *(unsigned short *)(pkt->data + 2*ETH_ALEN + 4);

	if(L3_protocol == __constant_htons(ETH_P_IP))
	{
		rtw_checkUDPandU2M(pkt);
		rtw_igmp_type_check(pstapriv, pkt);
	}
	else if(L3_protocol == __constant_htons(ETH_P_IPV6))
	{
		rtw_checkV6UDPandU2M(pkt);
		rtw_mld_type_check(pstapriv, pkt);
	}

	return ret;
}
#endif


#ifdef CONFIG_SMP_NETIF_RX
void rtw_netif_rx_enq(_adapter *padapter, struct sk_buff *pskb)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct rtw_netif_rx_ring *ring = &dvobj->netif_rx_ring;
	int err;
	u16 next_idx;

	next_idx = (ring->write_idx +1) & (MAX_NETIF_RX_RING_ENTRY - 1);
	if (next_idx == ring->read_idx) {
		ring->full++;
		err = rtw_netif_rx(padapter->pnetdev, pskb);
		if (NET_RX_SUCCESS == err)
			DBG_COUNTER(padapter->rx_logs.os_netif_ok);
		else
			DBG_COUNTER(padapter->rx_logs.os_netif_err);
		return;
	}

	pskb->cb[_SKB_CB_IFACE_ID] = padapter->iface_id;
	ring->entry[next_idx] = pskb;
#ifdef CONFIG_MIPS
	barrier();
#else
	smp_wmb();
#endif
	ring->write_idx = next_idx;
	rtw_tasklet_schedule(&dvobj->netif_rx_task);
}

void rtw_netif_rx_hdl(unsigned long data)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)data;
	struct rtw_netif_rx_ring *ring = &dvobj->netif_rx_ring;
	struct sk_buff *pskb;
	_adapter *padapter;
	int ret;
	u16 next_idx;

	next_idx = ring->read_idx;
	while (next_idx != ring->write_idx) {
		next_idx = (next_idx +1) & (MAX_NETIF_RX_RING_ENTRY - 1);
		pskb = ring->entry[next_idx];
		ring->read_idx = next_idx;

		padapter = dvobj->padapters[ (u8)pskb->cb[_SKB_CB_IFACE_ID] ];
		ret = _rtw_netif_rx(padapter->pnetdev, pskb);
		if (NET_RX_SUCCESS == ret)
			DBG_COUNTER(padapter->rx_logs.os_netif_ok);
		else
			DBG_COUNTER(padapter->rx_logs.os_netif_err);
	}
}

void rtw_netif_rx_init(struct dvobj_priv *dvobj)
{
	_tasklet *netif_rx_task = &dvobj->netif_rx_task;

	netif_rx_task->type = RTW_OS_HANDLER_TASKLET;
	netif_rx_task->cpu_id = CPU_ID_NETIF_RX;
	netif_rx_task->name = "netif RX";
	netif_rx_task->id = RTW_HANDLER_CORE_NETIF_RX;
	netif_rx_task->task_data.func = rtw_netif_rx_hdl;
	netif_rx_task->task_data.data = (unsigned long)dvobj;
	rtw_init_os_handler(dvobj, netif_rx_task);

	dvobj->netif_rx_ring.write_idx = 0;
	dvobj->netif_rx_ring.read_idx = 0;
}

void rtw_netif_rx_deinit(struct dvobj_priv *dvobj)
{
	struct rtw_netif_rx_ring *ring = &dvobj->netif_rx_ring;
	struct sk_buff *pskb;
	u16 next_idx;

	rtw_deinit_os_handler(dvobj, &dvobj->netif_rx_task);

	next_idx = ring->read_idx;
	while (next_idx != ring->write_idx) {
		next_idx = (next_idx +1) & (MAX_NETIF_RX_RING_ENTRY - 1);
		pskb = ring->entry[next_idx];
		rtw_skb_free(pskb);
	}
}
#endif /* CONFIG_SMP_NETIF_RX */

#ifdef CONFIG_SMP_PHL_RX_RECYCLE
void rtw_rx_recycle_enq(struct dvobj_priv *dvobj, u8* recvpkt)
{
	struct rtw_rx_recycle_ring *ring = &dvobj->rx_recycle_ring;
	u16 next_idx;

	_rtw_spinlock_bh(&ring->lock);

	next_idx = ring->write_idx;
	next_idx = next_idx + 1;
	if (next_idx >= MAX_RX_RECYCLE_RING_ENTRY)
		next_idx = 0;
	if (next_idx == ring->read_idx) {
		ring->full++;
		_rtw_spinunlock_bh(&ring->lock);
		rtw_phl_return_rxbuf(GET_HAL_INFO(dvobj), recvpkt);
		return;
	}

	ring->entry[next_idx] = recvpkt;
#ifdef CONFIG_MIPS
	barrier();
#else
	smp_wmb();
#endif
	ring->write_idx = next_idx;

	_rtw_spinunlock_bh(&ring->lock);

	rtw_tasklet_schedule(&dvobj->rx_recycle_task);
}

void rtw_rx_recycle_hdl(unsigned long data)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)data;
	void *phl = GET_HAL_INFO(dvobj);
	struct rtw_rx_recycle_ring *ring = &dvobj->rx_recycle_ring;
	u8* recvpkt;
	u16 next_idx;
	systime now, curr, diff;

	now = rtw_get_current_time();

	next_idx = ring->read_idx;
	while (next_idx != ring->write_idx) {
		next_idx = next_idx + 1;
		if (next_idx >= MAX_RX_RECYCLE_RING_ENTRY)
			next_idx = 0;
		recvpkt = ring->entry[next_idx];
		ring->read_idx = next_idx;
		rtw_phl_return_rxbuf(phl, recvpkt);
	}

	curr = rtw_get_current_time();
	diff = curr - now;
	if (diff > ring->max_time)
		ring->max_time = diff;
}

void rtw_rx_recycle_init(struct dvobj_priv *dvobj)
{
	_tasklet *rx_recycle_task = &dvobj->rx_recycle_task;

	rx_recycle_task->type = RTW_OS_HANDLER_TASKLET;
	rx_recycle_task->cpu_id = CPU_ID_RX_RECYCLE;
	rx_recycle_task->name = "RX recycle";
	rx_recycle_task->id = RTW_HANDLER_CORE_RX_RECYCLE;
	rx_recycle_task->task_data.func = rtw_rx_recycle_hdl;
	rx_recycle_task->task_data.data = (unsigned long)dvobj;
	rtw_init_os_handler(dvobj, rx_recycle_task);

	dvobj->rx_recycle_ring.write_idx = 0;
	dvobj->rx_recycle_ring.read_idx = 0;
	_rtw_spinlock_init(&dvobj->rx_recycle_ring.lock);
}

void rtw_rx_recycle_deinit(struct dvobj_priv *dvobj)
{
	void *phl = GET_HAL_INFO(dvobj);
	struct rtw_rx_recycle_ring *ring = &dvobj->rx_recycle_ring;
	u8* recvpkt;
	u16 next_idx;

	rtw_deinit_os_handler(dvobj, &dvobj->rx_recycle_task);

	next_idx = ring->read_idx;
	while (next_idx != ring->write_idx) {
		next_idx = next_idx + 1;
		if (next_idx >= MAX_RX_RECYCLE_RING_ENTRY)
			next_idx = 0;
		recvpkt = ring->entry[next_idx];
		ring->read_idx = next_idx;
		rtw_phl_return_rxbuf(phl, recvpkt);
	}

	_rtw_spinlock_free(&dvobj->rx_recycle_ring.lock);
}
#endif /* CONFIG_SMP_PHL_RX_RECYCLE */

void rtw_os_rx_init(struct dvobj_priv *dvobj)
{
#ifdef CONFIG_SMP_NETIF_RX
	rtw_netif_rx_init(dvobj);
#endif
#ifdef CONFIG_SMP_PHL_RX_RECYCLE
	rtw_rx_recycle_init(dvobj);
#endif
#ifdef DEBUG_CORE_RX
	dvobj->cnt_rx_pktsz = 1518;
#endif
}

void rtw_os_rx_deinit(struct dvobj_priv *dvobj)
{
#ifdef CONFIG_SMP_NETIF_RX
	rtw_netif_rx_deinit(dvobj);
#endif
#ifdef CONFIG_SMP_PHL_RX_RECYCLE
	rtw_rx_recycle_deinit(dvobj);
#endif
}

#if defined(CONFIG_BOLOCK_IPTV_FROM_LAN_SERVER) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
extern int block_iptv_from_lanWlan_server;

int rtw_check_group_ip_in_m2u_list(_adapter *padapter, struct sk_buff *pkt)
{
	int ret = _FALSE;
	struct sta_info *psta, *sa_psta;
	struct sta_priv *pstapriv = &padapter->stapriv;
	_list *phead, *plist;
	int i = 0, j;
	u8 chk_alive_num = 0;
	char chk_alive_list[NUM_STA];
#if !defined(A4_TX_MCAST2UNI) && !defined(CONFIG_TX_BCAST2UNI)
	u8 bc_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif
	struct ethhdr *eth = (struct ethhdr *)pkt->data;

	if (!IP_MCAST_MAC(eth->h_dest))
		return ret;

	sa_psta = rtw_get_stainfo(pstapriv, eth->h_source);

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* free sta asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		int stainfo_offset;
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
		if (stainfo_offset_valid(stainfo_offset))
			chk_alive_list[chk_alive_num++] = stainfo_offset;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	for (i = 0; i < chk_alive_num; i++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, chk_alive_list[i]);

		/* avoid come from STA1 and send back STA1 */
		if (psta == sa_psta
			|| _rtw_memcmp(psta->phl_sta->mac_addr, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) == _TRUE
#if !defined(A4_TX_MCAST2UNI) && !defined(CONFIG_TX_BCAST2UNI)
			|| _rtw_memcmp(psta->phl_sta->mac_addr, bc_addr, ETH_ALEN) == _TRUE
#endif
		) {
			continue;
		}

		for (j = 0; j < psta->ipmc_num; j++) {
			if (_rtw_memcmp(&psta->ipmc[j].mcmac[0], eth->h_dest, 6) == _TRUE) {
				ret = _TRUE;
				break;
			}
		}

		if (ret == _TRUE)
			break;
	}

	return ret;
}
#endif

void rtw_os_recv_indicate_pkt(_adapter *padapter, struct sk_buff *pkt,
						union recv_frame *rframe)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct recv_priv *precvpriv = &(padapter->recvpriv);
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
#ifdef RTW_CORE_PKT_TRACE
	struct rx_pkt_attrib *pattrib = &rframe->u.hdr.attrib;
#endif

#ifdef CONFIG_BR_EXT
	void *br_port = NULL;
	struct net_device *ndev = NULL;
#endif
	int ret;
#ifdef CONFIG_RTW_A4_STA
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
#endif
#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT
	extern int rtl_vlan_passthrough_enable;
#endif

#ifdef RTW_CORE_PKT_TRACE
	if(padapter->pkt_trace_enable)
	{
		rtw_eth_prepare_pkt_trace(padapter, &pattrib->pktinfo, pkt);
		RTW_RX_TRACE(padapter,&pattrib->pktinfo);
	}
#endif

	/* Indicat the packets to upper layer */
	if (pkt) {
		struct ethhdr *ehdr = (struct ethhdr *)pkt->data;

		DBG_COUNTER(padapter->rx_logs.os_indicate);

#ifdef CONFIG_RTW_MULTI_AP_R2
		if(core_map_rx_vlan_process(padapter, &pkt) != MAP_RX_CONTINUE)
		{
			DBG_COUNTER(padapter->rx_logs.os_indicate_err);
#ifdef RTW_CORE_PKT_TRACE
			if(padapter->pkt_trace_enable)
				RTW_RX_TRACE(padapter, &pattrib->pktinfo);
#endif
			goto drop;
		}
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
		if (linux_vlan_enable)
			rtw_linux_vlan_rx_process(padapter, pkt);
#endif

#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT
		if (rtl_vlan_passthrough_enable)
			rtw_vlan_passthrough_rx_process(pkt);
#endif

		/* pkt->data will be changed after insert/remove vlan tag, update ehdr */
		ehdr = (struct ethhdr *)pkt->data;
#ifdef RTW_CORE_PKT_TRACE
		if (padapter->pkt_trace_enable)
			pattrib->pktinfo.ptr = pkt->data;
#endif

		if (MLME_IS_AP(padapter)) {
			struct sk_buff *pskb2 = NULL;
			struct sta_info *psta = NULL;
			struct sta_priv *pstapriv = &padapter->stapriv;
			int bmcast = IS_MCAST(ehdr->h_dest);
			int bmcast_1905 = IS_1905_MCAST(ehdr->h_dest);

			/* RTW_INFO("bmcast=%d\n", bmcast); */

			if (_rtw_memcmp(ehdr->h_dest, adapter_mac_addr(padapter), ETH_ALEN) == _FALSE) {
				/* RTW_INFO("not ap psta=%p, addr=%pM\n", psta, ehdr->h_dest); */

				if (bmcast && !bmcast_1905) {
					psta = rtw_get_bcmc_stainfo(padapter);
					pskb2 = rtw_skb_clone(pkt);
				} else
					psta = rtw_get_stainfo(pstapriv, ehdr->h_dest);

#ifdef CONFIG_RTW_A4_STA
				if (!psta && (padapter->registrypriv.wifi_mib.a4_enable == 1)) {
					psta = core_a4_get_fwd_sta(padapter, ehdr->h_dest);
					if (psta){
						if (3 == rframe->u.hdr.attrib.to_fr_ds){
							if (rframe->u.hdr.psta == psta)
								psta = NULL;
						}
					}
				}
#endif
				if (psta && !bmcast_1905) {
					struct net_device *pnetdev = (struct net_device *)padapter->pnetdev;

					/* RTW_INFO("directly forwarding to the rtw_xmit_entry\n"); */

					/* skb->ip_summed = CHECKSUM_NONE; */
					pkt->dev = pnetdev;
					#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
					skb_set_queue_mapping(pkt, rtw_recv_select_queue(pkt));
					#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35) */

					if(!bmcast) {
						if(pregistrypriv->wifi_mib.block_relay==1){
							RTW_INFO("RX DROP: Relay unicast packet is blocked!\n");
#ifdef CONFIG_RTW_CORE_RXSC
							if(padapter->enable_rxsc)
								core_rxsc_clear_entry(padapter, psta);
#endif
#ifdef RTW_CORE_PKT_TRACE
							if(padapter->pkt_trace_enable)
								RTW_RX_TRACE(padapter,&pattrib->pktinfo);
#endif

							goto drop;
						}
					}

#if defined(CONFIG_BOLOCK_IPTV_FROM_LAN_SERVER) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
					if (block_iptv_from_lanWlan_server &&
						rtw_check_group_ip_in_m2u_list(padapter, pkt)) {
						rtw_skb_free(pkt);
						DBG_COUNTER(padapter->rx_logs.os_indicate_ap_mcast_drop);
					}
					else
#endif
					{
						rtw_os_tx(pkt, pnetdev);
					}

					if (bmcast && (pskb2 != NULL)) {
						pkt = pskb2;
						DBG_COUNTER(padapter->rx_logs.os_indicate_ap_mcast);
					} else {
						DBG_COUNTER(padapter->rx_logs.os_indicate_ap_forward);
#ifdef CONFIG_RTW_CORE_RXSC
						if (rframe->u.hdr.rxsc_entry)
							rframe->u.hdr.rxsc_entry->forward_to = RXSC_FWD_STA;
#endif
#ifdef RTW_CORE_PKT_TRACE
						if(padapter->pkt_trace_enable)
							RTW_RX_TRACE(padapter,&pattrib->pktinfo);
#endif
						return;
					}
				}
			} else { /* to APself */
				/* RTW_INFO("to APSelf\n"); */
				DBG_COUNTER(padapter->rx_logs.os_indicate_ap_self);
			}

			if(pregistrypriv->wifi_mib.guest_access){
				/*skb->cb will be memset 0 for struct inet_skb_parm (20 bytes) in ip_rcv()*/
				RTW_INFO("Guest Access, record skb->cb[%d] 0xe5!\n", _SKB_CB_GUEST_ACCESS);
				pkt->cb[_SKB_CB_GUEST_ACCESS] = 0xe5;
			}
		}

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
		if (MLME_IS_STA(padapter) && !is_primary_adapter(padapter))
		{
			if(rtw_crossband_rx_check(padapter, pkt, rframe))
				goto drop;
		}
#endif

#ifdef CONFIG_BR_EXT
#ifdef CONFIG_RTW_A4_STA /* what if STA TX A3+A4 mixed ?? */
		if (3 != rframe->u.hdr.attrib.to_fr_ds)
#endif
		if (MLME_IS_STA(padapter) || MLME_IS_ADHOC(padapter)) {
#ifdef CONFIG_RTW_A4_STA
			if (!((padapter->a4_enable == 1) && (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)))
#endif
			{
				ndev = padapter->pnetdev;

				#ifdef CONFIG_RTL_VLAN_8021Q
				if (linux_vlan_enable) {
					ndev = rtw_br_client_get_vlan_dev(padapter, pkt);
					if (!ndev)
						goto drop;
				}
				#endif

				/* Insert NAT2.5 RX here! */
				#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35))
				br_port = ndev->br_port;
				#else
				rcu_read_lock();
				br_port = rcu_dereference(ndev->rx_handler_data);
				rcu_read_unlock();
				#endif

				if (br_port) {
					int nat25_handle_frame(_adapter *priv, struct sk_buff *skb);

					if (nat25_handle_frame(padapter, pkt) == -1) {
						/* priv->ext_stats.rx_data_drops++; */
						/* DEBUG_ERR("RX DROP: nat25_handle_frame fail!\n"); */
						/* return FAIL; */

						#if 1
						/* bypass this frame to upper layer!! */
						#else
						rtw_skb_free(sub_skb);
						continue;
						#endif
					}
				}
			}
		}

#ifdef CONFIG_RTW_A4_STA /* what if STA TX A3+A4 mixed ?? */
		if (MLME_IS_STA(padapter))
		{
			if(padapter->registrypriv.wifi_mib.a4_enable == 1)
			{
				if(_rtw_memcmp(ehdr->h_source, padapter->br_mac, MAC_ADDR_LEN) == _TRUE)
				{
#ifdef RTW_CORE_PKT_TRACE
					if(padapter->pkt_trace_enable)
						RTW_RX_TRACE(padapter,&pattrib->pktinfo);
#endif
					goto drop;
				}
			}
		}
#endif
#endif /* CONFIG_BR_EXT */

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
		rtw_process_u2mc(padapter,pkt);

#ifdef CONFIG_RTW_A4_STA
		if (MLME_IS_STA(padapter) && (padapter->a4_enable == 1))
		{
			if(IS_MCAST(ehdr->h_dest))
			{
#ifdef CONFIG_A4_LOOPBACK
				if(rtw_check_a4_loop_entry(padapter, ehdr->h_source))
				{
					RTW_INFO("RX DROP: sa is recorded in other interface, drop it!\n");
#ifdef RTW_CORE_PKT_TRACE
					if(padapter->pkt_trace_enable)
						RTW_RX_TRACE(padapter,&pattrib->pktinfo);
#endif
					goto drop;
				}
#else
				struct net_device *wlan_dev = NULL;
#ifdef CONFIG_RTW_MULTI_AP_R2
				if (*((u16*)(pkt->data + (ETH_ALEN << 1))) == __constant_htons(ETH_P_8021Q))
				{
					unsigned char wlan_dev_name[IFNAMSIZ] = {0};
					int vlan_id = ntohs(*((u16*)(pkt->data+(ETH_ALEN<<1)+2))) & 0x0fff;
					snprintf(wlan_dev_name, IFNAMSIZ, "%s.%d", padapter->pnetdev->name, vlan_id);
					wlan_dev = __dev_get_by_name(&init_net, wlan_dev_name);
				}
				else
#endif
				{
					wlan_dev = (struct net_device *)padapter->pnetdev;
				}
				if(wlan_dev && check_srcmac_in_fdb_for_ax_driver(wlan_dev, ehdr->h_source))
				{
					RTW_INFO("RX DROP: sa is recorded in other interface, drop it!\n");
#ifdef RTW_CORE_PKT_TRACE
					if(padapter->pkt_trace_enable)
						RTW_RX_TRACE(padapter,&pattrib->pktinfo);
#endif
					goto drop;
				}
#endif
			}
		}
#endif
#endif

		RTW_PKT_MIRROR_DUMP(padapter, pkt, false);

		/* After eth_type_trans process , pkt->data pointer will move from ethrnet header to ip header */
		pkt->protocol = eth_type_trans(pkt, padapter->pnetdev);
		pkt->dev = padapter->pnetdev;
		pkt->ip_summed = CHECKSUM_NONE; /* CONFIG_TCP_CSUM_OFFLOAD_RX */
#ifdef CONFIG_TCP_CSUM_OFFLOAD_RX
		if ((rframe->u.hdr.attrib.csum_valid == 1)
		    && (rframe->u.hdr.attrib.csum_err == 0))
			pkt->ip_summed = CHECKSUM_UNNECESSARY;
#endif /* CONFIG_TCP_CSUM_OFFLOAD_RX */

#ifdef CONFIG_RTW_NAPI
#ifdef CONFIG_RTW_NAPI_DYNAMIC
		if (!skb_queue_empty(&precvpriv->rx_napi_skb_queue)
			&& !adapter_to_dvobj(padapter)->en_napi_dynamic
			)
			napi_recv(padapter, RTL_NAPI_WEIGHT);
#endif

		if (pregistrypriv->en_napi
			#ifdef CONFIG_RTW_NAPI_DYNAMIC
			&& adapter_to_dvobj(padapter)->en_napi_dynamic
			#endif
		) {
			skb_queue_tail(&precvpriv->rx_napi_skb_queue, pkt);
			#ifndef CONFIG_RTW_NAPI_V2
			napi_schedule(&padapter->napi);
			#endif
			return;
		}
#endif /* CONFIG_RTW_NAPI */

#ifdef RTW_CORE_PKT_TRACE
		if(padapter->pkt_trace_enable)
			RTW_RX_TRACE(padapter,&pattrib->pktinfo);
#endif
#ifdef DEBUG_CORE_RX
		if (dvobj->cnt_rx_pktsz == pkt->len+ETH_HLEN+4)
			dvobj->num_rx_pktsz_os++;
		dvobj->total_rx_pkt_os++;
#endif
#ifdef CONFIG_SMP_NETIF_RX
		rtw_netif_rx_enq(padapter, pkt);
#else
		ret = rtw_netif_rx(padapter->pnetdev, pkt);
		if (ret == NET_RX_SUCCESS)
			DBG_COUNTER(padapter->rx_logs.os_netif_ok);
		else
			DBG_COUNTER(padapter->rx_logs.os_netif_err);
#endif
#ifdef CONFIG_RTW_CORE_RXSC
		if (rframe->u.hdr.rxsc_entry)
			rframe->u.hdr.rxsc_entry->forward_to = RXSC_FWD_KERNEL;
#endif
	}
	return;
drop:
	if(pkt)
		rtw_skb_free(pkt);
	return;
}

void rtw_handle_tkip_mic_err(_adapter *padapter, struct sta_info *sta, u8 bgroup)
{
#ifdef CONFIG_IOCTL_CFG80211
	enum nl80211_key_type key_type = 0;
#endif
	union iwreq_data wrqu;
	struct iw_michaelmicfailure    ev;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	systime cur_time = 0;

	if (psecuritypriv->last_mic_err_time == 0)
		psecuritypriv->last_mic_err_time = rtw_get_current_time();
	else {
		cur_time = rtw_get_current_time();

		if (cur_time - psecuritypriv->last_mic_err_time < 60 * HZ) {
			psecuritypriv->btkip_countermeasure = _TRUE;
			psecuritypriv->last_mic_err_time = 0;
			psecuritypriv->btkip_countermeasure_time = cur_time;
		} else
			psecuritypriv->last_mic_err_time = rtw_get_current_time();
	}

#ifdef CONFIG_IOCTL_CFG80211
	if (bgroup)
		key_type |= NL80211_KEYTYPE_GROUP;
	else
		key_type |= NL80211_KEYTYPE_PAIRWISE;

	cfg80211_michael_mic_failure(padapter->pnetdev, sta->phl_sta->mac_addr, key_type, -1, NULL, GFP_ATOMIC);
#endif

	_rtw_memset(&ev, 0x00, sizeof(ev));
	if (bgroup)
		ev.flags |= IW_MICFAILURE_GROUP;
	else
		ev.flags |= IW_MICFAILURE_PAIRWISE;

	ev.src_addr.sa_family = ARPHRD_ETHER;
	_rtw_memcpy(ev.src_addr.sa_data, sta->phl_sta->mac_addr, ETH_ALEN);

	_rtw_memset(&wrqu, 0x00, sizeof(wrqu));
	wrqu.data.length = sizeof(ev);

#ifndef CONFIG_IOCTL_CFG80211
	wireless_send_event(padapter->pnetdev, IWEVMICHAELMICFAILURE, &wrqu, (char *) &ev);
#endif
}

#ifdef CONFIG_HOSTAPD_MLME
void rtw_hostapd_mlme_rx(_adapter *padapter, union recv_frame *precv_frame)
{
	struct sk_buff *skb;
	struct hostapd_priv *phostapdpriv  = padapter->phostapdpriv;
	struct net_device *pmgnt_netdev = phostapdpriv->pmgnt_netdev;


	skb = precv_frame->u.hdr.pkt;

	if (skb == NULL)
		return;

	skb->data = precv_frame->u.hdr.rx_data;
	skb->tail = precv_frame->u.hdr.rx_tail;
	skb->len = precv_frame->u.hdr.len;

	/* pskb_copy = rtw_skb_copy(skb);
	*	if(skb == NULL) goto _exit; */

	skb->dev = pmgnt_netdev;
	skb->ip_summed = CHECKSUM_NONE;
	skb->pkt_type = PACKET_OTHERHOST;
	/* skb->protocol = __constant_htons(0x0019); ETH_P_80211_RAW */
	skb->protocol = __constant_htons(0x0003); /*ETH_P_80211_RAW*/

	/* RTW_INFO("(1)data=0x%x, head=0x%x, tail=0x%x, mac_header=0x%x, len=%d\n", skb->data, skb->head, skb->tail, skb->mac_header, skb->len); */

	/* skb->mac.raw = skb->data; */
	skb_reset_mac_header(skb);

	/* skb_pull(skb, 24); */
	_rtw_memset(skb->cb, 0, sizeof(skb->cb));

	rtw_netif_rx(pmgnt_netdev, skb);

	precv_frame->u.hdr.pkt = NULL; /* set pointer to NULL before rtw_free_recvframe() if call rtw_netif_rx() */
}
#endif /* CONFIG_HOSTAPD_MLME */

#ifdef CONFIG_WIFI_MONITOR
/*
   precv_frame: impossible to be NULL
   precv_frame: free by caller
 */
int rtw_recv_monitor(_adapter *padapter, union recv_frame *precv_frame)
{
	int ret = _FAIL;
	struct sk_buff *skb;

	skb = precv_frame->u.hdr.pkt;
	if (skb == NULL) {
		RTW_INFO("%s :skb==NULL something wrong!!!!\n", __func__);
		goto _recv_drop;
	}

	skb->data = precv_frame->u.hdr.rx_data;
	skb_set_tail_pointer(skb, precv_frame->u.hdr.len);
	skb->len = precv_frame->u.hdr.len;
	skb->ip_summed = CHECKSUM_NONE;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = htons(0x0019); /* ETH_P_80211_RAW */

	/* send to kernel */
	rtw_netif_rx(padapter->pnetdev, skb);

	/* pointers to NULL before rtw_free_recvframe() */
	precv_frame->u.hdr.pkt = NULL;

	ret = _SUCCESS;

_recv_drop:
	return ret;
}
#endif /* CONFIG_WIFI_MONITOR */

inline void rtw_rframe_set_os_pkt(union recv_frame *rframe)
{
	struct sk_buff *skb = rframe->u.hdr.pkt;

	skb->data = rframe->u.hdr.rx_data;
	skb_set_tail_pointer(skb, rframe->u.hdr.len);
	skb->len = rframe->u.hdr.len;
}

int rtw_recv_indicatepkt(_adapter *padapter, union recv_frame *precv_frame)
{
	struct recv_priv *precvpriv;
	_queue	*pfree_recv_queue;

	precvpriv = &(padapter->recvpriv);
	pfree_recv_queue = &(precvpriv->free_recv_queue);

	if (precv_frame->u.hdr.pkt == NULL)
		goto _recv_indicatepkt_drop;

	rtw_os_recv_indicate_pkt(padapter, precv_frame->u.hdr.pkt, precv_frame);

	precv_frame->u.hdr.pkt = NULL;
	rtw_free_recvframe(precv_frame, pfree_recv_queue);
	return _SUCCESS;

_recv_indicatepkt_drop:
	DBG_COUNTER(padapter->rx_logs.os_indicate_err);
	return _FAIL;
}

#ifdef CONFIG_RTL_VLAN_8021Q
void rtw_linux_vlan_rx_process(_adapter *padapter, struct sk_buff *pskb)
{
	u16 vid = 0;
	unsigned int index = 0;
	struct net_device *ndev = NULL;

	if (!padapter || !pskb) {
		RTW_ERR("[%s %d] NULL pointer! padapter: %p pskb: %p\n",
			__FUNCTION__, __LINE__, padapter, pskb);
		return;
	}

	ndev = padapter->pnetdev;
	if (!ndev) {
		RTW_ERR("[%s %d] ndev is NULL!\n",__FUNCTION__, __LINE__);
		return;
	}

	/* add vlan tag if pkt is untagged */
	if (PKT_ETH_TYPE(pskb) != __constant_htons(ETH_P_8021Q)) {
		/* mapping dev to pvid array's index */
		index = ndev->vlan_member_map;
		if ((index >= WLAN0_MASK_BIT) && (index <= WLAN1_VXD_MASK_BIT)) {
			vid = vlan_ctl_p->pvid[index];
			if (vid) {
				memmove(pskb->data - VLAN_HLEN, pskb->data, ETH_ALEN<<1);
				skb_push(pskb, VLAN_HLEN);
				*((u16*)(pskb->data + (ETH_ALEN<<1))) = __constant_htons(ETH_P_8021Q);
				*((u16*)(pskb->data + (ETH_ALEN<<1) + 2)) = __constant_htons(vid);

				/* For iptables vlanid match. */
				pskb->srcVlanId = vid;

				*(u16*)(pskb->linux_vlan_src_tag) = __constant_htons(ETH_P_8021Q);
				*(u16*)(pskb->linux_vlan_src_tag + 2) = __constant_htons(vid);
			}
		}
	}

	/* remove vlan tag in eapol key packet*/
	if (is_eapol_key_pkt(pskb, 1)) {
		memmove(pskb->data + VLAN_TAG_LEN, pskb->data, ETH_ALEN<<1);
		skb_pull(pskb, VLAN_TAG_LEN);
	}
}
#endif

#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT
void rtw_vlan_passthrough_rx_process(struct sk_buff *pskb)
{
	if (!pskb) {
		RTW_ERR("[%s %d] pskb is NULL!\n", __FUNCTION__, __LINE__);
		return;
	}

	if (PKT_ETH_TYPE(pskb) == htons(ETH_P_8021Q)) {
		memcpy(pskb->vlan_passthrough_saved_tag, pskb->data + ETH_ALEN*2, VLAN_TAG_LEN);
		memmove(pskb->data + VLAN_TAG_LEN, pskb->data, ETH_ALEN<<1);
		skb_pull(pskb, VLAN_TAG_LEN);
	}
}
#endif
