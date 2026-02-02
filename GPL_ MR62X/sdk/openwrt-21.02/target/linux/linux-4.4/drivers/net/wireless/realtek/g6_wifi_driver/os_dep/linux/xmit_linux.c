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
#define _XMIT_OSDEP_C_

#include <drv_types.h>

#ifdef CONFIG_RTL_VLAN_8021Q
#include <linux/if_vlan.h>
extern int linux_vlan_enable;
extern linux_vlan_ctl_t *vlan_ctl_p;
#endif

#define DBG_DUMP_OS_QUEUE_CTL 0

#ifdef CONFIG_TX_MCAST2UNI
u32 ipv4_mc_reserved_addr[] = {
	0xE0000001, /*all-nodes: 224.0.0.1*/
	0xE0000002, /*all-routers: 224.0.0.2*/
	0xE0000004, /*DVMRP: 224.0.0.4*/
	0xE0000005, /*OSPF: 224.0.0.5*/
	0xE0000006, /*OSPF DR: 224.0.0.6*/
	0xE0000007, /* ST-router: 224.0.0.7*/
	0xE0000008, /* ST-host: 224.0.0.8*/
	0xE0000009, /* RIP: 224.0.0.9*/
	0xE000000A, /* EIGRP: 224.0.0.10*/
	0xE000000B, /* 224.0.0.11*/
	0xE000000C, /* all dhcp server: 224.0.0.12*/
	0xE000000D, /* PIM: 224.0.0.13*/
	0xE000000E, /* RSVP: 224.0.0.14*/
	0xE000000F, /*CBT: 224.0.0.15*/
	0xE0000010, /*SBM: 224.0.0.16*/
	0xE0000011, /*SBMS: 224.0.0.17*/
	0xE0000012, /*VRRP: 224.0.0.18*/
	0xE00000FB, /*mdns: 224.0.0.251*/
	0xE00001B2, /*iapp:224.0.1.178*/
	0xEFFFFFFA  /*upnp: 239.255.255.250*/
};

u32 ipv6_mc_reserved_addr[][4] = {
	{0xff010000, 0x00000000, 0x00000000, 0x00000001}, /* all-nodes:ff01::1*/
	{0xff010000, 0x00000000, 0x00000000, 0x00000002}, /* all-routers:ff00::2*/
	//{0xff020000, 0x00000000, 0x00000000, 0x00000003}, /* unused:ff02::3*/
	{0xff020000, 0x00000000, 0x00000000, 0x00000004}, /* DVMRP:ff02::4*/
	{0xff020000, 0x00000000, 0x00000000, 0x00000005}, /* OSPFIGP:ff02::5*/
	{0xff020000, 0x00000000, 0x00000000, 0x00000006}, /* OSPF:ff02::6*/
	{0xff020000, 0x00000000, 0x00000000, 0x00000007}, /* ST-router:ff02::7*/
	{0xff020000, 0x00000000, 0x00000000, 0x00000008}, /* ST-host:ff02::8*/
	{0xff020000, 0x00000000, 0x00000000, 0x00000009}, /* RIP:ff02::9*/
	{0xff020000, 0x00000000, 0x00000000, 0x0000000a}, /* EIGRP:ff02::a*/
	{0xff020000, 0x00000000, 0x00000000, 0x0000000b}, /* ff02::b*/
	{0xff020000, 0x00000000, 0x00000000, 0x0000000c}, /* UPNP:ff02::c*/
	{0xff020000, 0x00000000, 0x00000000, 0x0000000e}, /* RSVP:ff02::e*/
	{0xff020000, 0x00000000, 0x00000000, 0x0000006a}, /* all snooper:ff02::6a*/
	{0xff020000, 0x00000000, 0x00000000, 0x000000fb}, /* MDNS:ff02::fb*/
	{0xff020000, 0x00000000, 0x00000000, 0x00010001}, /* ff02::1:4*/
	{0xff020000, 0x00000000, 0x00000000, 0x00010002}, /* DHCPV6:ff02::1:2*/
	{0xff020000, 0x00000000, 0x00000000, 0x00010003}, /* ff02::1:3*/
	{0xff020000, 0x00000000, 0x00000000, 0x00010004}, /* DTCP:ff02::1:2*/
	{0xff050000, 0x00000000, 0x00000000, 0x00000002}, /* all routers:ff05::2*/
	{0xff050000, 0x00000000, 0x00000000, 0x00010003} /* all dhcp server:ff05::1:3*/
	//{0xff050000, 0x00000000, 0x00000000, 0x00010004}  /* unused:ff05::1:4*/
};

#define IPV6_RESERVED_ADDRESS_RANGE(group_address) \
	((((group_address[0] & 0xFFFFFFFF)==0xFF020000) && ((group_address[1] & 0xFFFFFFFF)==0x0) \
		&& ((group_address[2] & 0xFFFFFFFF)==0x1) && ((group_address[3] & 0xFF000000)==0xFF000000)) \
		|| (((group_address[0] & 0xFFFFFFFF)==0xFF050000) && ((group_address[1] & 0xFFFFFFFF)==0x0) \
		&& ((group_address[2] & 0xFFFFFFFF)==0x0) && ((group_address[3] & 0xFFFFFC00)==0x00011000)))

#define IPV6_ADDRESS_EQU(address1, address2) (address1[0] == address2[0] && address1[1] == address2[1] && address1[2] == address2[2] && address1[3] == address2[3])

#define IPV6_ADDRESS_GTR(address1, address2) \
	((address1[0] > address2[0]) \
		|| (address1[0] == address2[0] && address1[1] > address2[1]) \
		|| (address1[0] == address2[0] && address1[1] == address2[1] \
			&& address1[2] > address2[2]) \
		|| (address1[0] == address2[0] && address1[1] == address2[1] \
			&& address1[2] == address2[2] && address1[3] > address2[3]))
#endif

uint rtw_remainder_len(struct pkt_file *pfile)
{
	return pfile->buf_len - ((SIZE_PTR)(pfile->cur_addr) - (SIZE_PTR)(pfile->buf_start));
}

void _rtw_open_pktfile(struct sk_buff *pktptr, struct pkt_file *pfile)
{

	pfile->pkt = pktptr;
	pfile->cur_addr = pfile->buf_start = pktptr->data;
	pfile->pkt_len = pfile->buf_len = pktptr->len;

	pfile->cur_buffer = pfile->buf_start ;

}

uint _rtw_pktfile_read(struct pkt_file *pfile, u8 *rmem, uint rlen)
{
	uint	len = 0;


	len =  rtw_remainder_len(pfile);
	len = (rlen > len) ? len : rlen;

	if (rmem)
		skb_copy_bits(pfile->pkt, pfile->buf_len - pfile->pkt_len, rmem, len);

	pfile->cur_addr += len;
	pfile->pkt_len -= len;


	return len;
}

sint rtw_endofpktfile(struct pkt_file *pfile)
{

	if (pfile->pkt_len == 0) {
		return _TRUE;
	}


	return _FALSE;
}

void rtw_set_tx_chksum_offload(struct sk_buff *pkt, struct pkt_attrib *pattrib)
{
#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	struct sk_buff *skb = (struct sk_buff *)pkt;
	struct iphdr *iph = NULL;
	struct ipv6hdr *i6ph = NULL;
	struct udphdr *uh = NULL;
	struct tcphdr *th = NULL;
	u8 	protocol = 0xFF;

	if (skb->protocol == htons(ETH_P_IP)) {
		iph = (struct iphdr *)skb_network_header(skb);
		protocol = iph->protocol;
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		i6ph = (struct ipv6hdr *)skb_network_header(skb);
		protocol = i6ph->nexthdr;
	} else
		{}

	/*	HW unable to compute CSUM if header & payload was be encrypted by SW(cause TXDMA error) */
	if (pattrib->bswenc == _TRUE) {
		if (skb->ip_summed == CHECKSUM_PARTIAL)
			skb_checksum_help(skb);
		return;
	}

	/*	For HW rule, clear ipv4_csum & UDP/TCP_csum if it is UDP/TCP packet	*/
	switch (protocol) {
	case IPPROTO_UDP:
		uh = (struct udphdr *)skb_transport_header(skb);
		uh->check = 0;
		if (iph)
			iph->check = 0;
		pattrib->hw_csum = _TRUE;
		break;
	case IPPROTO_TCP:
		th = (struct tcphdr *)skb_transport_header(skb);
		th->check = 0;
		if (iph)
			iph->check = 0;
		pattrib->hw_csum = _TRUE;
		break;
	default:
		break;
	}
#endif

}
#if 0 /*CONFIG_CORE_XMITBUF*/
int rtw_os_xmit_resource_alloc(_adapter *padapter, struct xmit_buf *pxmitbuf, u32 alloc_sz, u8 flag)
{
	if (alloc_sz > 0) {
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
		struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);
		struct usb_device	*pusbd = dvobj_to_usb(pdvobjpriv)->pusbdev;

		pxmitbuf->pallocated_buf = rtw_usb_buffer_alloc(pusbd, (size_t)alloc_sz, &pxmitbuf->dma_transfer_addr);
		pxmitbuf->pbuf = pxmitbuf->pallocated_buf;
		if (pxmitbuf->pallocated_buf == NULL)
			return _FAIL;
#else /* CONFIG_USE_USB_BUFFER_ALLOC_TX */

		pxmitbuf->pallocated_buf = rtw_zmalloc(alloc_sz);
		if (pxmitbuf->pallocated_buf == NULL)
			return _FAIL;

		pxmitbuf->pbuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitbuf->pallocated_buf), XMITBUF_ALIGN_SZ);

#endif /* CONFIG_USE_USB_BUFFER_ALLOC_TX */
	}

	if (flag) {
#ifdef CONFIG_USB_HCI
		int i;
		for (i = 0; i < 8; i++) {
			pxmitbuf->pxmit_urb[i] = usb_alloc_urb(0, GFP_KERNEL);
			if (pxmitbuf->pxmit_urb[i] == NULL) {
				RTW_INFO("pxmitbuf->pxmit_urb[i]==NULL");
				return _FAIL;
			}
		}
#endif
	}

	return _SUCCESS;
}

void rtw_os_xmit_resource_free(_adapter *padapter, struct xmit_buf *pxmitbuf, u32 free_sz, u8 flag)
{
	if (flag) {
#ifdef CONFIG_USB_HCI
		int i;

		for (i = 0; i < 8; i++) {
			if (pxmitbuf->pxmit_urb[i]) {
				/* usb_kill_urb(pxmitbuf->pxmit_urb[i]); */
				usb_free_urb(pxmitbuf->pxmit_urb[i]);
			}
		}
#endif
	}

	if (free_sz > 0) {
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
		struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);
		struct usb_device	*pusbd = dvobj_to_usb(pdvobjpriv)->pusbdev;

		rtw_usb_buffer_free(pusbd, (size_t)free_sz, pxmitbuf->pallocated_buf, pxmitbuf->dma_transfer_addr);
		pxmitbuf->pallocated_buf =  NULL;
		pxmitbuf->dma_transfer_addr = 0;
#else	/* CONFIG_USE_USB_BUFFER_ALLOC_TX */
		if (pxmitbuf->pallocated_buf)
			rtw_mfree(pxmitbuf->pallocated_buf, free_sz);
#endif /* CONFIG_USE_USB_BUFFER_ALLOC_TX */
	}
}
#else
u8 rtw_os_xmit_resource_alloc(_adapter *padapter, struct xmit_frame *pxframe)
{
	u32 alloc_sz = SZ_XMITFRAME_EXT + SZ_ALIGN_XMITFRAME_EXT;

#if 0 /*def CONFIG_USE_USB_BUFFER_ALLOC_TX*/
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct usb_device *pusbd = dvobj_to_usb(dvobj)->pusbdev;

	pxframe->prealloc_buf_addr = rtw_usb_buffer_alloc(pusbd, (size_t)alloc_sz, &pxframe->dma_transfer_addr);
	if (pxframe->prealloc_buf_addr == NULL) {
		RTW_ERR("%s prealloc_buf_addr failed\n", __func__);
		rtw_warn_on(1);
		return _FAIL;
	}
	pxframe->buf_addr = pxframe->prealloc_buf_addr;
#else
	pxframe->prealloc_buf_addr = rtw_zmalloc(alloc_sz);
	if (pxframe->prealloc_buf_addr == NULL) {
		RTW_ERR("%s prealloc_buf_addr failed\n", __func__);
		rtw_warn_on(1);
		return _FAIL;
	}
	pxframe->buf_addr = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxframe->prealloc_buf_addr), SZ_ALIGN_XMITFRAME_EXT);
#endif
	return _SUCCESS;
}

void rtw_os_xmit_resource_free(_adapter *padapter, struct xmit_frame *pxframe)
{
	u32 free_sz = SZ_XMITFRAME_EXT + SZ_ALIGN_XMITFRAME_EXT;

#if 0 /*def CONFIG_USE_USB_BUFFER_ALLOC_TX*/
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct usb_device *pusbd = dvobj_to_usb(dvobj)->pusbdev;

	if (pxframe->prealloc_buf_addr) {
		rtw_usb_buffer_free(pusbd, (size_t)free_sz, pxframe->prealloc_buf_addr, pxframe->dma_transfer_addr);
		pxframe->prealloc_buf_addr = NULL;
		pxframe->buf_addr = NULL;
		pxframe->dma_transfer_addr = 0;
	}
#else
	if (pxframe->prealloc_buf_addr) {
		rtw_mfree(pxframe->prealloc_buf_addr, free_sz);
		pxframe->prealloc_buf_addr = NULL;
		pxframe->buf_addr = NULL;
	}
#endif
}
#endif

void dump_os_queue(void *sel, _adapter *padapter)
{
	struct net_device *ndev = padapter->pnetdev;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	int i;

	for (i = 0; i < 4; i++) {
		RTW_PRINT_SEL(sel, "os_queue[%d]:%s\n"
			, i, __netif_subqueue_stopped(ndev, i) ? "stopped" : "waked");
	}
#else
	RTW_PRINT_SEL(sel, "os_queue:%s\n"
		      , netif_queue_stopped(ndev) ? "stopped" : "waked");
#endif
}

#define WMM_XMIT_THRESHOLD_2G	(NR_XMITFRAME_2G * 2 / 5)
#define WMM_XMIT_THRESHOLD_5G	(NR_XMITFRAME_5G * 2 / 5)

static inline bool rtw_os_need_wake_queue(_adapter *padapter, u16 qidx)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	int wmm_xmit_thr = GET_HAL_SPEC(padapter->dvobj)->band_cap & BAND_CAP_5G ? WMM_XMIT_THRESHOLD_5G : WMM_XMIT_THRESHOLD_2G;

	if (padapter->registrypriv.wifi_spec) {
		if (pxmitpriv->hwxmits[qidx].accnt < wmm_xmit_thr)
			return _TRUE;
#ifdef DBG_CONFIG_ERROR_DETECT
#ifdef DBG_CONFIG_ERROR_RESET
	} else if (rtw_hal_sreset_inprogress(padapter) == _TRUE) {
		return _FALSE;
#endif/* #ifdef DBG_CONFIG_ERROR_RESET */
#endif/* #ifdef DBG_CONFIG_ERROR_DETECT */
	} else {
#ifdef CONFIG_MCC_MODE
		if (MCC_EN(padapter)) {
			if (rtw_hal_check_mcc_status(padapter, MCC_STATUS_DOING_MCC)
			    && MCC_STOP(padapter))
				return _FALSE;
		}
#endif /* CONFIG_MCC_MODE */
		return _TRUE;
	}
	return _FALSE;
#else
#ifdef CONFIG_MCC_MODE
	if (MCC_EN(padapter)) {
		if (rtw_hal_check_mcc_status(padapter, MCC_STATUS_DOING_MCC)
		    && MCC_STOP(padapter))
			return _FALSE;
	}
#endif /* CONFIG_MCC_MODE */
	return _TRUE;
#endif
}

static inline bool rtw_os_need_stop_queue(_adapter *padapter, u16 qidx)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	int wmm_xmit_thr = GET_HAL_SPEC(padapter->dvobj)->band_cap & BAND_CAP_5G ? WMM_XMIT_THRESHOLD_5G : WMM_XMIT_THRESHOLD_2G;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	if (padapter->registrypriv.wifi_spec) {
		/* No free space for Tx, tx_worker is too slow */
		if (pxmitpriv->hwxmits[qidx].accnt > wmm_xmit_thr)
			return _TRUE;
	} else {
		if (padapter->pfree_txreq_queue->qlen <= 4)
			return _TRUE;
	}
#else
	if (padapter->pfree_txreq_queue->qlen <= 4)
		return _TRUE;
#endif
	return _FALSE;
}

__IMEM_WLAN_SECTION__
void rtw_os_pkt_complete(_adapter *padapter, struct sk_buff *pkt)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	u16	qidx;

	if (pkt == NULL)
		return;

	qidx = skb_get_queue_mapping(pkt);
	if (rtw_os_need_wake_queue(padapter, qidx)) {
		if (DBG_DUMP_OS_QUEUE_CTL)
			RTW_INFO(FUNC_ADPT_FMT": netif_wake_subqueue[%d]\n", FUNC_ADPT_ARG(padapter), qidx);
		netif_wake_subqueue(padapter->pnetdev, qidx);
	}
#else
	if (pkt == NULL)
		return;

	if (rtw_os_need_wake_queue(padapter, 0)) {
		if (DBG_DUMP_OS_QUEUE_CTL)
			RTW_INFO(FUNC_ADPT_FMT": netif_wake_queue\n", FUNC_ADPT_ARG(padapter));
		netif_wake_queue(padapter->pnetdev);
	}
#endif

	rtw_skb_free(pkt);
}

void rtw_os_xmit_complete(_adapter *padapter, struct xmit_frame *pxframe)
{
	if (pxframe->pkt)
		rtw_os_pkt_complete(padapter, pxframe->pkt);

	pxframe->pkt = NULL;
}

void rtw_os_xmit_schedule(_adapter *padapter)
{
#if 0 /*defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)*/
	_adapter *pri_adapter;

	if (!padapter)
		return;
	pri_adapter = GET_PRIMARY_ADAPTER(padapter);

	if (_rtw_queue_empty(&padapter->xmitpriv.pending_xmitbuf_queue) == _FALSE)
		_rtw_up_sema(&pri_adapter->xmitpriv.xmit_sema);


#elif defined(CONFIG_PCI_HCI) || defined(CONFIG_USB_HCI)
	struct xmit_priv *pxmitpriv;

	if (!padapter)
		return;

	pxmitpriv = &padapter->xmitpriv;

	_rtw_spinlock_bh(&pxmitpriv->lock);

	if (rtw_txframes_pending(padapter))
		rtw_tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);

	_rtw_spinunlock_bh(&pxmitpriv->lock);

#if 0 /*defined(CONFIG_PCI_HCI) && defined(CONFIG_XMIT_THREAD_MODE)*/
	if (_rtw_queue_empty(&padapter->xmitpriv.pending_xmitbuf_queue) == _FALSE)
		_rtw_up_sema(&padapter->xmitpriv.xmit_sema);
#endif


#endif
}

__IMEM_WLAN_SECTION__
static bool rtw_check_xmit_resource(_adapter *padapter, struct sk_buff *pkt)
{
	bool busy = _FALSE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	u16	qidx;

	qidx = skb_get_queue_mapping(pkt);
	if (rtw_os_need_stop_queue(padapter, qidx)) {
		if (DBG_DUMP_OS_QUEUE_CTL)
			RTW_INFO(FUNC_ADPT_FMT": netif_stop_subqueue[%d]\n", FUNC_ADPT_ARG(padapter), qidx);
		netif_stop_subqueue(padapter->pnetdev, qidx);
		busy = _TRUE;
	}
#else
	if (rtw_os_need_stop_queue(padapter, 0)) {
		if (DBG_DUMP_OS_QUEUE_CTL)
			RTW_INFO(FUNC_ADPT_FMT": netif_stop_queue\n", FUNC_ADPT_ARG(padapter));
		rtw_netif_stop_queue(padapter->pnetdev);
		busy = _TRUE;
	}
#endif
	return busy;
}

void rtw_os_wake_queue_at_free_stainfo(_adapter *padapter, int *qcnt_freed)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	int i;

	for (i = 0; i < 4; i++) {
		if (qcnt_freed[i] == 0)
			continue;

		if (rtw_os_need_wake_queue(padapter, i)) {
			if (DBG_DUMP_OS_QUEUE_CTL)
				RTW_INFO(FUNC_ADPT_FMT": netif_wake_subqueue[%d]\n", FUNC_ADPT_ARG(padapter), i);
			netif_wake_subqueue(padapter->pnetdev, i);
		}
	}
#else
	if (qcnt_freed[0] || qcnt_freed[1] || qcnt_freed[2] || qcnt_freed[3]) {
		if (rtw_os_need_wake_queue(padapter, 0)) {
			if (DBG_DUMP_OS_QUEUE_CTL)
				RTW_INFO(FUNC_ADPT_FMT": netif_wake_queue\n", FUNC_ADPT_ARG(padapter));
			netif_wake_queue(padapter->pnetdev);
		}
	}
#endif
}

#ifdef CONFIG_VW_REFINE
inline u8 is_tcp_ip_ack(u8 *eth_packet)
{
    return ( eth_packet[12]==0x08 && eth_packet[13]==0x00
             && eth_packet[23]==0x06 &&eth_packet[47] == 0x10);
}

inline u8 is_tcp_ip(u8 *eth_packet)
{
    return ( eth_packet[12]==0x08 && eth_packet[13]==0x00
             && eth_packet[23]==0x06);
}

inline u8 is_udp_ip(u8 *eth_packet)
{
    return ( eth_packet[12]==0x08 && eth_packet[13]==0x00
             && eth_packet[23]==0x11);
}

inline u8 is_bmc(u8 *eth_packet)
{
    return (( ((*eth_packet) & 0x01) == 0x01) );
}

inline u8 is_icmpv6_pkt(u8 *data)
{
	unsigned char *ptr = NULL;
	unsigned short L3_protocol = 0;
	int ret = 0;

	ptr = data + 2 * ETH_ALEN;
	L3_protocol = *(unsigned short *)ptr;

	if(L3_protocol == __constant_htons(ETH_P_8021Q))
	{
		ptr = ptr + 4;
		L3_protocol = *(unsigned short *)ptr;
	}

	if(L3_protocol == __constant_htons(ETH_P_IPV6))
	{
		if((ptr[8] == 0x3a) || (ptr[8] == 0x0 && ptr[42] == 0x3a))
			ret = 1;
	}

	return ret;
}

inline u8 is_dhcp_pkt(u8 *eth_packet)
{
    unsigned short *ptr = NULL;
    unsigned short src_port, dst_port;

    ptr = (unsigned short *)(eth_packet + ETH_HLEN + 20);
    src_port = *((unsigned short *)(ptr));
    dst_port = *((unsigned short *)(ptr + 1));

    return ( is_udp_ip(eth_packet) &&
            ( (src_port == __constant_htons(0x43) && (dst_port == __constant_htons(0x44))) ||
              (src_port == __constant_htons(0x44) && (dst_port == __constant_htons(0x43))) ));
}

inline void dump_port(u8 *eth_packet)
{
    unsigned short *ptr = NULL;
    unsigned short src_port, dst_port;

    ptr = (unsigned short *)(eth_packet + ETH_HLEN + 20);
    src_port = *((unsigned short *)(ptr));
    dst_port = *((unsigned short *)(ptr + 2));

    printk(" src:%x dst =%x \n", src_port, dst_port);
}

u32 rtw_get_tsf(_adapter *padapter)
{
        return rtw_phl_read32(padapter->dvobj->phl, 0xC438);
}

u32 rtw_read_reg(_adapter *padapter, u32 addr)
{
       return rtw_phl_read32(padapter->dvobj->phl, addr);
}

#define RTW_TSF_LESS(a, b)	(((a - b) & 0x80000000) != 0)
#define RTW_TSF_DIFF(a, b)	((a >= b)? (a - b):(0xffffffff - b + a + 1))
#define RTW_DIFF(a,b) (((a) > (b)) ? (a-b) : (0))

void core_tx_swq_amsdu_timeout1(_adapter *padapter, u8 i);

#ifdef CONFIG_SWQ_SKB_ARRAY
#define RTL_CIRC_CNT(head,tail,size) ((head>=tail)?(head-tail):(size-tail+head))

bool vw_skb_enq(_adapter *padapter, u8 idx, struct sk_buff *pskb)
{
	u16 wptr, rptr;

	if (pskb == NULL)
		return _FAIL;

	_rtw_spinlock_bh(&padapter->dvobj->skb_q.lock);
	wptr = padapter->swq_skb_array[idx].wptr;
	rptr = padapter->swq_skb_array[idx].rptr;

	wptr = (wptr + 1) % VW_SWQ_SKB_NR;

	if (wptr == rptr) {
		_rtw_spinunlock_bh(&padapter->dvobj->skb_q.lock);
		printk_ratelimited(KERN_WARNING "TX DROP: skb ring full!(s:%d w:%d)\n", VW_SWQ_SKB_NR, wptr);
		return _FAIL;
	}

	padapter->swq_skb_array[idx].skb_array[wptr] = pskb;
	padapter->swq_skb_array[idx].wptr = wptr;

	_rtw_spinunlock_bh(&padapter->dvobj->skb_q.lock);
	return _SUCCESS;
}

struct sk_buff *vw_skb_deq(_adapter *padapter, u8 idx)
{
	struct sk_buff *pskb = NULL;
	u16 wptr, rptr;

	wptr = padapter->swq_skb_array[idx].wptr;
	rptr = padapter->swq_skb_array[idx].rptr;

	if (wptr == rptr)
		return NULL;

	rptr = (rptr + 1) % VW_SWQ_SKB_NR;

	pskb = padapter->swq_skb_array[idx].skb_array[rptr];
	padapter->swq_skb_array[idx].rptr = rptr;

	return pskb;
}

u16 vw_skb_len(_adapter *padapter, u8 idx)
{
	return RTL_CIRC_CNT(padapter->swq_skb_array[idx].wptr,
		padapter->swq_skb_array[idx].rptr, VW_SWQ_SKB_NR);
}
#else
bool vw_skb_enq(_adapter *padapter, u8 idx, struct sk_buff *pskb)
{
	u32 skb_qlen, used_tx_cnt;
	u32 wde_free, ple_free, free_p;
	_queue *pfree_txreq_queue = padapter->pfree_txreq_queue;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	skb_qlen = ATOMIC_READ(&padapter->skb_xmit_queue_len);
	used_tx_cnt = padapter->max_tx_ring_cnt - pfree_txreq_queue->qlen + 1;

	if (pxmitpriv->txsc_amsdu_enable)
		used_tx_cnt = used_tx_cnt * padapter->tx_amsdu;

	if ((used_tx_cnt + skb_qlen) > padapter->max_enq_len) {
		if (padapter->vw_enable != 2) {
			wde_free = rtw_read_reg(padapter, 0x8c08) >> 16;
			ple_free = rtw_read_reg(padapter, 0x9008) >> 16;
			free_p = rtw_read_reg(padapter, 0x8A50) >> 12;

			printk_ratelimited(KERN_WARNING "TX DROP: exceed skb Q!(s:%d a:%d t:%d m:%d) "
				"wf=%x pf=%x fp=%x\n", skb_qlen, padapter->swq_amsdu_cnt, used_tx_cnt,
				padapter->max_enq_len, wde_free, ple_free, free_p);
		}
		return _FAIL;
	}

	skb_queue_tail(&padapter->skb_xmit_queue[idx], pskb);
	ATOMIC_INC(&padapter->skb_xmit_queue_len);

	return _SUCCESS;
}

struct sk_buff *vw_skb_deq(_adapter *padapter, u8 idx)
{
	struct sk_buff *pskb = NULL;

	pskb = skb_dequeue(&padapter->skb_xmit_queue[idx]);

	if (pskb) {
		ATOMIC_DEC(&padapter->skb_xmit_queue_len);

		if (0 != pskb->cb[_SKB_VW_FLAG])
			DBG_COUNTER(padapter->tx_logs.core_vw_amsdu_enq);
	}

	return pskb;
}

u16 vw_skb_len(_adapter *padapter, u8 idx)
{
	return skb_queue_len(&padapter->skb_xmit_queue[idx]);
}
#endif


void core_tx_swq_xmit_tasklet(_adapter *padapter)
{
	u8 i = 0, cur = 0, my_pos = 0;
	unsigned long start_time, time = 0;
	struct sk_buff *skb;
	u32 skb_cnt, deq_cnt, all_deq_cnt = 0;
	u8 need_tx_req = 0;
	s32 ret;

	start_time = jiffies;
	time = start_time + RTL_MILISECONDS_TO_JIFFIES(padapter->xmit_dsr_time);

	my_pos = padapter->pre_pos + 1;

	for (i = 0; i < MAX_SKB_XMIT_QUEUE; i++) {
		if (0 == padapter->pfree_txreq_queue->qlen)
			break;

		cur = (my_pos + i) % MAX_SKB_XMIT_QUEUE;

		skb_cnt = vw_skb_len(padapter, cur);

		if (0 == skb_cnt) {
			padapter->skb_que_ts[cur] = jiffies + msecs_to_jiffies(padapter->swq_timeout);
			continue;
		} else if (time_before(jiffies, padapter->skb_que_ts[cur]) && (skb_cnt < padapter->tx_lmt))
			continue;

		need_tx_req = padapter->sta_deq_len / padapter->tx_amsdu;

		if (need_tx_req > RTW_DIFF(padapter->pfree_txreq_queue->qlen, padapter->ring_lmt))
			goto out;

		deq_cnt = 0;

		while ((skb = vw_skb_deq(padapter, cur)) != NULL) {
			skb->cb[_SKB_CB_AMSDU_TXSC] = 0;

			if ((1 == padapter->with_bk) && (deq_cnt == padapter->sta_deq_len))
				skb->cb[_SKB_VW_LAST] = 1;

			ret = rtw_core_tx(padapter, &skb, NULL);

			if (ret == FAIL) {
				padapter->skb_que_ts[cur] = jiffies + msecs_to_jiffies(padapter->swq_timeout);
				goto xmit_drop_cnt;
			}

			deq_cnt++;
			all_deq_cnt++;

			if (deq_cnt > padapter->sta_deq_len) {
				padapter->skb_que_ts[cur] = jiffies + msecs_to_jiffies(padapter->swq_timeout);;
				break;
			}

			if ((0 == padapter->pfree_txreq_queue->qlen) ||
				(all_deq_cnt > padapter->max_deq_len) ||
				time_after(jiffies, time)) {
				padapter->skb_que_ts[cur] = jiffies + msecs_to_jiffies(padapter->swq_timeout);
				goto out;
			}
		}

		padapter->skb_que_ts[cur] = jiffies + msecs_to_jiffies(padapter->swq_timeout);
	}

	padapter->pre_pos = cur % MAX_SKB_XMIT_QUEUE;
out:
	return;

xmit_drop_cnt:
	padapter->pre_pos = cur % MAX_SKB_XMIT_QUEUE;
}

#ifdef CONFIG_TXSC_AMSDU
u32 swq_process_ac_que(_adapter *padapter, struct sta_info *psta, u8 ac, u8 *sts)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
    u32 dq_cnt = 0, org_cnt =0;
    u8 ret = FAIL;

    org_cnt = pxmitpriv->cnt_txsc_amsdu_deq[ac];

    do {
         ret = txsc_amsdu_timeout_tx(psta, ac);

         if ( ret != SUCCESS ) {
            *sts = ret;
             break;
         }

         if ( 0 == dq_cnt ) {
            *sts = _RTW_QUE_EMPTY;
             break;
         }
         dq_cnt += (pxmitpriv->cnt_txsc_amsdu_deq[ac] - org_cnt);
     } while ( 0 );

     return dq_cnt;
}

void update_que_ts(_adapter *padapter, struct sta_info *psta, u8 idx)
{
      u32 tx_tp;

      if ( psta == NULL )
         return;

      tx_tp = psta->sta_stats.tx_tp_kbits >> 10;

      if ( 0 != padapter->swq_timeout ) {
           padapter->skb_que_ts[idx] = padapter->swq_timeout;
           return;
      }

      if ( tx_tp < 10 )
          padapter->skb_que_ts[idx] = 1000000;
      else if ( tx_tp < 100 )
          padapter->skb_que_ts[idx] = 10000;
      else if ( tx_tp < 200 )
          padapter->skb_que_ts[idx] = 10000;
      else
          padapter->skb_que_ts[idx] = 5000;
}

void core_tx_swq_amsdu_timeout1(_adapter *padapter, u8 i)
{
      struct sta_info *psta = NULL;
      u8 j;
      u8 sts;

           psta = padapter->swq_psta[i];

           if ( NULL == psta ) {
               return;
           }

           if ( NULL == psta->phl_sta ) {
               return;
           }

           for (j = 0; j < 4; j++ ) {
               if ( padapter->swq_staq_bitmap[i] & BIT(j) ) {
                   swq_process_ac_que(padapter, psta, j, &sts);

                   if ( _RTW_QUE_EMPTY == sts )  {
                         padapter->swq_staq_bitmap[i] &= ~BIT(j);
                   } else if ( _RTW_QUE_FULL == sts ) {
                         goto exit_amsdu_timout;
                   } else if ( FAIL == sts ) {
                         goto exit_amsdu_timout;
                   }
               }
           }

           if ( 0 == padapter->swq_staq_bitmap[i] )
               padapter->swq_psta[i] = NULL;

exit_amsdu_timout:
      return;
}

void core_tx_swq_amsdu_timeout(_adapter *padapter)
{
      struct sta_info *psta = NULL;
      u8 i, j;
      u8 sts;

      //if ( 0 == padapter->pfree_txreq_queue->qlen )
      //     return;

      for (i = 0; i < MAX_SKB_XMIT_QUEUE; i++)  {
           psta = padapter->swq_psta[i];

           if ( NULL == psta ) {
               padapter->swq_staq_bitmap[i] = 0;
               continue;
           }

           if ( NULL == psta->phl_sta ) {
               padapter->swq_psta[i] = NULL;
               continue;
           }

           for (j = 0; j < 4; j++ ) {
               if ( padapter->swq_staq_bitmap[i] & BIT(j) ) {
                   swq_process_ac_que(padapter, psta, j, &sts);

                   if ( _RTW_QUE_EMPTY == sts )  {
                         padapter->swq_staq_bitmap[i] &= ~BIT(j);
                         break;
                   } else if ( _RTW_QUE_FULL == sts ) {
                         goto exit_amsdu_timout;
                   }
             }
          }

          /// update_que_ts(padapter, psta, i);

          if ( 0 == padapter->swq_staq_bitmap[i] )
              padapter->swq_psta[i] = NULL;
     }

exit_amsdu_timout:
      return;
}
#endif

extern void rtw_core_set_gt3(_adapter *padapter, u8 enable, long timeout);
extern void _phl_tx_callback_pcie(void *context);
extern void core_pci_xmit_tasklet(_adapter *padapter);

u8 rtl8192cd_swq_is_empty(struct dvobj_priv *dvobj)
{
	u8 i = 0, is_empty = 1;
	_adapter *prim_adp = NULL;

	for (i = 0; i < CONFIG_IFACE_NUMBER; i++) {
		prim_adp = dvobj->padapters[i];

		if ( NULL == prim_adp )
	      	continue;

	    	if ( prim_adp->netif_up == _FALSE )
	    	continue;

		if (ATOMIC_READ(&prim_adp->skb_xmit_queue_len) > 0) {
			is_empty = 0;
			break;
		}
	}

	return is_empty;
}

void rtl8192cd_swq_timeout_g6(unsigned long data)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)data;
	u8 i = 0, trigger_timer = 1;
	_adapter *padapter = NULL;

	if (RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO("%s => bDriverStopped or bSurpriseRemoved\n",
			__func__);
		return;
	}

	for (i = 0; i < dvobj->iface_nums; i++) {

		padapter = dvobj->padapters[i];

		if (!padapter || padapter->netif_up == _FALSE)
			continue;

		core_tx_swq_xmit_tasklet(padapter);
	}

rtw_swq_end:
	if (dvobj->tx_mode == 0) {
		if (rtl8192cd_swq_is_empty(dvobj))
			trigger_timer = 0;
	}

	if (trigger_timer) {
		padapter = dvobj_get_primary_adapter(dvobj);
		padapter->hw_swq_cnt = (padapter->hw_swq_cnt + 1) % 8192;

		if (0 != padapter->hw_swq_timeout)
			rtw_core_set_gt3(padapter, 1, padapter->hw_swq_timeout);
		else
			rtw_core_set_gt3(padapter, 1, 10000);
	}
}
#endif

#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
sint mgnt_tx_enqueue_for_sleeping_sta(_adapter *padapter, struct xmit_frame *pmgntframe)
{
	struct sta_info *psta;
	struct sta_xmit_priv *pstaxmitpriv;
	unsigned char *pframe;
	struct rtw_ieee80211_hdr *pwlanhdr;
	u8 subtype;
	u8 ps;
	sint ret = _FALSE;

#ifdef CONFIG_DYN_ALLOC_XMITFRAME
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	_queue *queue = &pxmitpriv->free_xframe_ext_queue;
#endif
	/* No need to handle client's sleep state for non-unicast packet */
	if (pmgntframe->phl_txreq->mdata.bc
	    || pmgntframe->phl_txreq->mdata.mc)
		return _FALSE;

	if (!MLME_IS_AP(padapter) && !MLME_IS_MESH(padapter)
#ifdef CONFIG_TDLS
		&& !(padapter->tdlsinfo.link_established == _TRUE && (psta->tdls_sta_state & TDLS_LINKED_STATE))
#endif
		)
		return _FALSE;

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;

	/* PS bit in AP mode is used to ignore subframe type but to
	 * check client's sleep status
	 */
	ps = GetPwrMgt(pframe);
	ClearPwrMgt(pframe);

	subtype = get_frame_sub_type(pframe); /* bit(7)~bit(2) */
	if (   (ps == 0)
	    && (   WIFI_PROBERSP == subtype
		|| WIFI_DATA_NULL == subtype
		|| WIFI_QOS_DATA_NULL == subtype))
		return _FALSE;

	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;
	psta = rtw_get_stainfo(&padapter->stapriv, pwlanhdr->addr1);
	if (NULL == psta)
		return _FALSE;

	pstaxmitpriv = &psta->sta_xmitpriv;

	if ((psta->state & (WIFI_ASOC_STATE|WIFI_SLEEP_STATE)) ==
		(WIFI_ASOC_STATE|WIFI_SLEEP_STATE)) {
#ifdef CONFIG_DYN_ALLOC_XMITFRAME
		_rtw_spinlock_bh(&queue->lock);
		rtw_list_delete(&pmgntframe->list);
		_rtw_spinunlock_bh(&queue->lock);
#endif
		_rtw_spinlock_bh(&pstaxmitpriv->mgt_q.lock);
		rtw_list_insert_tail(&pmgntframe->list, get_list_head(&pstaxmitpriv->mgt_q));
		pstaxmitpriv->mgt_q.qlen++;
		if (pstaxmitpriv->mgt_q.qlen == 1)
			rtw_set_bit(TXQ_MGT, &pstaxmitpriv->tx_pending_bitmap);
		_rtw_spinunlock_bh(&pstaxmitpriv->mgt_q.lock);

#ifdef CONFIG_TDLS
		if (padapter->tdlsinfo.link_established == _TRUE
			&& (psta->tdls_sta_state & TDLS_LINKED_STATE)) {
			/* Transmit TDLS PTI via AP */
			if (ATOMIC_READ(&pstaxmitpriv->txq_total_len) == 1)
				rtw_tdls_cmd(padapter, psta->phl_sta->mac_addr, TDLS_ISSUE_PTI);
		} else
#endif
		if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) {
			struct sta_priv *pstapriv = &padapter->stapriv;
			if (!rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
				rtw_tim_map_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
				update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
			}
		}

		ret = _TRUE;
	} else if (pstaxmitpriv->tx_pending_bitmap & BIT(TXQ_MGT)) {
#ifdef CONFIG_DYN_ALLOC_XMITFRAME
		_rtw_spinlock_bh(&queue->lock);
		rtw_list_delete(&pmgntframe->list);
		_rtw_spinunlock_bh(&queue->lock);
#endif
		_rtw_spinlock_bh(&pstaxmitpriv->mgt_q.lock);
		rtw_list_insert_tail(&pmgntframe->list, get_list_head(&pstaxmitpriv->mgt_q));
		pstaxmitpriv->mgt_q.qlen++;
		if (pstaxmitpriv->mgt_q.qlen == 1)
			rtw_set_bit(TXQ_MGT, &pstaxmitpriv->tx_pending_bitmap);
		_rtw_spinunlock_bh(&pstaxmitpriv->mgt_q.lock);

		ret = _TRUE;
	}

	return ret;
}

struct xmit_frame *rtw_dequeue_mgtq_xmitframe(_adapter *padapter, struct sta_xmit_priv *pstaxmitpriv)
{
	_queue *queue = &pstaxmitpriv->mgt_q;
	_list *phead, *plist;
	struct xmit_frame *pxframe = NULL;

	phead = get_list_head(queue);

	_rtw_spinlock_bh(&queue->lock);
	plist = get_next(phead);
	if (plist != phead) {
		pxframe = LIST_CONTAINOR(plist, struct xmit_frame, list);
		rtw_list_delete(plist);
		queue->qlen--;
		if (queue->qlen == 0)
			rtw_clear_bit(TXQ_MGT, &pstaxmitpriv->tx_pending_bitmap);
	}
	_rtw_spinunlock_bh(&queue->lock);

	return pxframe;
}

void set_mgntframe_mdata(struct xmit_frame *pmgntframe)
{
	u8 *pframe;
	struct rtw_ieee80211_hdr *pwlanhdr;

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;
	SetMData(&pwlanhdr->frame_ctl);
}
#endif /* CONFIG_AP_MODE || CONFIG_TDLS */

#ifdef RTW_PHL_TX
extern s32 rtw_core_tx_mgmt(_adapter *padapter, struct xmit_frame *pxframe);
#endif

#ifdef CONFIG_ONE_TXQ
unsigned int is_dhcp_pkt2(struct sk_buff *pskb)
{
#define DHCP_MAGIC 0x63825363

	struct dhcpMessage {
		u8 op;
		u8 htype;
		u8 hlen;
		u8 hops;
		__be32 xid;
		__be16 secs;
		__be16 flags;
		__be32 client_ip;
		__be32 your_ip;
		__be32 server_ip;
		__be32 relay_ip;
		u8 hw_addr[16];
		u8 serv_name[64];
		u8 boot_file[128];
		u32 cookie;
#if 0
		u8 options[308]; /* 312 - cookie */
#endif
	};

	u16 protocol;
	struct udphdr *udph;

	protocol = *((u16 *)(pskb->data + 2 * ETH_ALEN));

	if (protocol == __constant_htons(ETH_P_IP)) {
		struct iphdr* iph;
		iph = (struct iphdr *)(pskb->data + ETH_HLEN);

		if (iph->protocol == 17) { /* UDP */
			udph = (struct udphdr *)((u8 *)iph + (iph->ihl << 2));

			if ((udph->source == __constant_htons(0x43) && udph->dest == __constant_htons(0x44))
				|| (udph->source == __constant_htons(0x44) && udph->dest == __constant_htons(0x43))) {
				struct dhcpMessage *dhcph;
				dhcph = (struct dhcpMessage *)((u8 *)udph + sizeof(struct udphdr));

				if ((unsigned long)dhcph & 0x03) { //not 4-byte alignment
					u_int32_t cookie;
					char *pdhcphcookie;
					char *pcookie = (char *)&cookie;

					pdhcphcookie = (char *)&dhcph->cookie;
					pcookie[0] = pdhcphcookie[0];
					pcookie[1] = pdhcphcookie[1];
					pcookie[2] = pdhcphcookie[2];
					pcookie[3] = pdhcphcookie[3];
					if (cookie == htonl(DHCP_MAGIC))
						return _TRUE;
				} else {
					if(dhcph->cookie == htonl(DHCP_MAGIC))
						return _TRUE;
				}
			}
		}
	} else if (protocol == __constant_htons(ETH_P_IPV6)) {
		struct ipv6hdr *ipv6h;
		ipv6h = (struct ipv6hdr *)(pskb->data + ETH_HLEN);

		if (ipv6h->nexthdr == 17) { /* UDP */
			udph = (struct udphdr *)((u8 *)ipv6h + sizeof(struct ipv6hdr));

			if ((udph->source == __constant_htons(546) && udph->dest == __constant_htons(547))
				|| (udph->source == __constant_htons(547) && udph->dest == __constant_htons(546))) {
				return _TRUE;
			}
		}
	}

	return _FALSE;
}

u8 check_pkt_type(struct sk_buff *pskb)
{
#define DHCP_MAGIC 0x63825363

	struct dhcpMessage {
		u8 op;
		u8 htype;
		u8 hlen;
		u8 hops;
		__be32 xid;
		__be16 secs;
		__be16 flags;
		__be32 client_ip;
		__be32 your_ip;
		__be32 server_ip;
		__be32 relay_ip;
		u8 hw_addr[16];
		u8 serv_name[64];
		u8 boot_file[128];
		u32 cookie;
#if 0
		u8 options[308]; /* 312 - cookie */
#endif
	};

	u16 protocol;
	u8 pkt_type;
	u8* pkt_buf = pskb->data;
	struct udphdr *udph;

	pkt_type = 0;
	protocol = *(u16 *)(pkt_buf + 2 * ETH_ALEN);

	if (protocol == __constant_htons(ETH_P_IP)) {
		struct iphdr* iph = (struct iphdr *)(pkt_buf + ETH_HLEN);

		if (iph->protocol == 6) { /* TCP */
			struct tcphdr *tcph = (struct tcphdr *)((u8*)iph + iph->ihl*4);

			if (iph->ihl*4+tcph->doff*4 == ntohs(iph->tot_len)) {
				int tcp_flag = tcp_flag_word(tcph) & __cpu_to_be32(0x00ff0000);
				if (TCP_FLAG_ACK == tcp_flag)
					pkt_type = _PKT_TYPE_TCPACK;
			}
		} else if (iph->protocol == 17) { /* UDP */
			udph = (struct udphdr *)((u8 *)iph + (iph->ihl << 2));

			if ((udph->source == __constant_htons(67) && udph->dest == __constant_htons(68))
				|| (udph->source == __constant_htons(68) && udph->dest == __constant_htons(67))) {
				struct dhcpMessage *dhcph;
				dhcph = (struct dhcpMessage *)((u8 *)udph + sizeof(struct udphdr));

				if ((unsigned long)dhcph & 0x03) { //not 4-byte alignment
					u_int32_t cookie;
					char *pdhcphcookie;
					char *pcookie = (char *)&cookie;

					pdhcphcookie = (char *)&dhcph->cookie;
					pcookie[0] = pdhcphcookie[0];
					pcookie[1] = pdhcphcookie[1];
					pcookie[2] = pdhcphcookie[2];
					pcookie[3] = pdhcphcookie[3];
					if (cookie == htonl(DHCP_MAGIC))
						pkt_type = (_PKT_TYPE_URGENT | _PKT_TYPE_DHCP);
				} else {
					if(dhcph->cookie == htonl(DHCP_MAGIC))
						pkt_type = (_PKT_TYPE_URGENT | _PKT_TYPE_DHCP);
				}
			}
		}
	} else if (protocol == __constant_htons(ETH_P_IPV6)) {
		struct ipv6hdr *ipv6h = (struct ipv6hdr *)(pkt_buf + ETH_HLEN);

		if (ipv6h->nexthdr == 17) { /* UDP */
			udph = (struct udphdr *)((u8 *)ipv6h + sizeof(struct ipv6hdr));

			if ((udph->source == __constant_htons(546) && udph->dest == __constant_htons(547))
				|| (udph->source == __constant_htons(547) && udph->dest == __constant_htons(546))) {
				pkt_type = (_PKT_TYPE_URGENT | _PKT_TYPE_DHCP);
			}
		}
	} else if (protocol == __constant_htons(0x888e)) {
		pkt_type = _PKT_TYPE_URGENT;
	}

	return pkt_type;
}

int tcp_ack_merge(struct sta_tx_queue *txq)
{
	struct sk_buff_head skb_list;
	struct sk_buff *phead, *plist;
	struct sk_buff *skb1, *skb2;
	int num = 0;

	__skb_queue_head_init(&skb_list);
	phead = (struct sk_buff *)&txq->qhead;

	_rtw_spinlock_bh(&txq->qhead.lock);
	plist = phead->prev;
	while (plist != phead) {
		if (plist->cb[_SKB_CB_FLAGS] & _PKT_TYPE_TCPACK)
			break;
		plist = plist->prev;
	}
	if (plist != phead) {
		skb1 = plist;
		plist = plist->prev;
		while (plist != phead) {
			skb2 = plist;
			plist = plist->prev;
			/* Drop old TCP Ack if it's at the same tcp connection */
			if (skb2->cb[_SKB_CB_FLAGS] & _PKT_TYPE_TCPACK) {
				if (!memcmp(skb1->data+26, skb2->data+26, 12)) {
					__skb_unlink(skb2, &txq->qhead);
					__skb_queue_tail(&skb_list, skb2);
				} else
					skb1 = skb2;
			}
		}
	}
	_rtw_spinunlock_bh(&txq->qhead.lock);

	if (skb_list.qlen) {
		num = skb_list.qlen;
		while (skb_list.qlen) {
			skb1 = __skb_dequeue(&skb_list);
			rtw_skb_free(skb1);
		}
	}

	return num;
}

enum TXQ_STATE {
	TXQ_STATE_OK = 0,
	TXQ_STATE_EMPTY,
	TXQ_STATE_WAITING,
	TXQ_STATE_TIMEOUT,
	TXQ_STATE_EXCEED_TS_LIMIT,
	TXQ_STATE_EXCEED_TXREQ_TS_LIMIT,
	TXQ_STATE_INSUFFICIENT_TXREQ
};

int pspoll_trigger_hdl(struct dvobj_priv *dvobj, struct sta_info *psta)
{
	_adapter *padapter = psta->padapter;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	u16 tx_pending_bitmap;
	int q_idx;
	struct sta_tx_queue *txq;
	struct sk_buff *pskb = NULL;
	struct xmit_frame *pmgntframe = NULL;
	int status = TXQ_STATE_OK;
	_queue *sta_queue;
	_queue *pfree_txreq_queue = padapter->pfree_txreq_queue;

	tx_pending_bitmap = pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap;
	if (tx_pending_bitmap) {
		if (tx_pending_bitmap & BIT(TXQ_MGT)) {
			pmgntframe = rtw_dequeue_mgtq_xmitframe(padapter, pstaxmitpriv);
			if (pmgntframe) {
				if (pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap)
					set_mgntframe_mdata(pmgntframe);
				rtw_core_tx_mgmt(padapter, pmgntframe);
				goto update_state;
			}
		}

		if (0 == pfree_txreq_queue->qlen) {
			status = TXQ_STATE_INSUFFICIENT_TXREQ;
			goto out;
		}

		for (q_idx = 0; q_idx < MAX_TXQ; q_idx++) {
			if (!(tx_pending_bitmap & BIT(q_idx)))
				continue;

			txq = &pstaxmitpriv->txq[q_idx];

			_rtw_spinlock_bh(&txq->qhead.lock);
			pskb = __skb_dequeue(&txq->qhead);
			if (pskb) {
				struct sk_buff *pskb_next = skb_peek(&txq->qhead);
				if (pskb_next)
					txq->timeout = *(systime *)&pskb_next->cb[_SKB_CB_TIME];
				else
					rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
			}
			_rtw_spinunlock_bh(&txq->qhead.lock);

			if (pskb) {
				ATOMIC_DEC(&dvobj->txq_total_len);
				ATOMIC_DEC(&pstaxmitpriv->txq_total_len);
				rtw_core_tx(padapter, &pskb, psta);
				break;
			}
		}
	}

update_state:

	tx_pending_bitmap = pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap;
	if (!tx_pending_bitmap) {
		if (rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
			rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
			update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
		}

		if (!pskb && !pmgntframe) {
			RTW_INFO("no buffered packets to xmit\n");
			/* issue nulldata with More data bit = 0 to indicate we have no buffered packets */
			issue_nulldata(padapter, psta->phl_sta->mac_addr, 0, 0, 0);
		}
	}

	sta_queue = &dvobj->ps_trigger_sta_queue;

	_rtw_spinlock_bh(&sta_queue->lock);
	pstaxmitpriv->ps_trigger_type &= ~ BIT0;
	if (!pstaxmitpriv->ps_trigger_type) {
		if (rtw_is_list_empty(&pstaxmitpriv->ps_trigger) == _FALSE) {
			rtw_list_delete(&pstaxmitpriv->ps_trigger);
			sta_queue->qlen--;
		}
	}
	_rtw_spinunlock_bh(&sta_queue->lock);

out:
	return status;
}

int wmmps_trigger_hdl(struct dvobj_priv *dvobj, struct sta_info *psta)
{
	_adapter *padapter = psta->padapter;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	u16 tx_pending_bitmap;
	int q_idx;
	struct sta_tx_queue *txq;
	struct sk_buff *pskb = NULL;
	int status = TXQ_STATE_OK;
	_queue *sta_queue;
	_queue *pfree_txreq_queue = padapter->pfree_txreq_queue;

	if (pstaxmitpriv->tx_pending_bitmap & BIT(TXQ_MGT)) {
		struct xmit_frame *pmgntframe;
		u16 mask = psta->uapsd_bitmap | BIT(TXQ_MGT);
		while (1) {
			pmgntframe = rtw_dequeue_mgtq_xmitframe(padapter, pstaxmitpriv);
			if (!pmgntframe)
				break;

			if (pstaxmitpriv->tx_pending_bitmap & mask)
				set_mgntframe_mdata(pmgntframe);
			rtw_core_tx_mgmt(padapter, pmgntframe);
		}
	}

	tx_pending_bitmap = pstaxmitpriv->tx_pending_bitmap & psta->uapsd_bitmap;
	if (tx_pending_bitmap) {
		for (q_idx = 0; q_idx < MAX_TXQ; q_idx++) {
			if (!(tx_pending_bitmap & BIT(q_idx)))
				continue;

			txq = &pstaxmitpriv->txq[q_idx];
			while (1) {
				if (0 == pfree_txreq_queue->qlen) {
					_rtw_spinlock_bh(&txq->qhead.lock);
					pskb = skb_peek(&txq->qhead);
					if (pskb)
						txq->timeout = *(systime *)&pskb->cb[_SKB_CB_TIME];
					_rtw_spinunlock_bh(&txq->qhead.lock);
					status = TXQ_STATE_INSUFFICIENT_TXREQ;
					goto out;
				}

				_rtw_spinlock_bh(&txq->qhead.lock);
				pskb = __skb_dequeue(&txq->qhead);
				if (txq->qhead.qlen == 0)
					rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
				_rtw_spinunlock_bh(&txq->qhead.lock);

				if (!pskb)
					break;

				ATOMIC_DEC(&dvobj->txq_total_len);
				ATOMIC_DEC(&pstaxmitpriv->txq_total_len);
				rtw_core_tx(padapter, &pskb, psta);
			}
		}
	} else {
		/* issue one qos null frame with More data bit = 0 and the EOSP bit set (=1) */
		issue_qos_nulldata(padapter, psta->phl_sta->mac_addr, 7, 0, 0, 0);
	}

	if (psta->uapsd_bitmap == (BIT(TXQ_VO)|BIT(TXQ_VI)|BIT(TXQ_BE)|BIT(TXQ_BK))
		&& !pstaxmitpriv->tx_pending_bitmap
#ifdef CONFIG_TDLS
		&& !(psta->tdls_sta_state & TDLS_LINKED_STATE)
#endif
		) {
		if (rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
			rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
			update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
		}
	}

	sta_queue = &dvobj->ps_trigger_sta_queue;

	_rtw_spinlock_bh(&sta_queue->lock);
	pstaxmitpriv->ps_trigger_type &= ~ BIT1;
	if (!pstaxmitpriv->ps_trigger_type) {
		if (rtw_is_list_empty(&pstaxmitpriv->ps_trigger) == _FALSE) {
			rtw_list_delete(&pstaxmitpriv->ps_trigger);
			sta_queue->qlen--;
		}
	}
	_rtw_spinunlock_bh(&sta_queue->lock);

out:
	return status;
}

int ps_trigger_hdl(struct dvobj_priv *dvobj)
{
	_queue *sta_queue;
	struct sta_info *psta;
	struct sta_xmit_priv *pstaxmitpriv;
	_list *phead, *plist;
	int status = TXQ_STATE_OK;

	sta_queue = &dvobj->ps_trigger_sta_queue;
	phead = get_list_head(sta_queue);

	while ((plist = get_next(phead)) != phead) {
		psta = LIST_CONTAINOR(plist, struct sta_info, sta_xmitpriv.ps_trigger);
		pstaxmitpriv = &psta->sta_xmitpriv;

		if (pstaxmitpriv->ps_trigger_type & BIT0) {
			status = pspoll_trigger_hdl(dvobj, psta);
			if (TXQ_STATE_OK != status)
				goto out;
		}
		if (pstaxmitpriv->ps_trigger_type & BIT1) {
			status = wmmps_trigger_hdl(dvobj, psta);
			if (TXQ_STATE_OK != status)
				goto out;
		}
	}

out:
	return status;
}

int service_sta_hdl(struct dvobj_priv *dvobj, struct sta_info *psta)
{
#define TIMEOUT_TRIGGER	BIT0
#define PKT_TRIGGER	BIT1

	_adapter *padapter = psta->padapter;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	struct sta_tx_queue *txq;
	int q_idx;
	u32 rate, ts_consumed;
	struct sk_buff *pskb;
	int cnt, max_agg_num, deq_num;
	int status = TXQ_STATE_OK;
	_queue *sta_queue;
	u8 trigger = 0; /* BIT0: timeout, BIT1: pkt, BIT2: (buffered pkt >= agg_num) */
	u16 nr_tcpack_strip;
	_queue *pfree_txreq_queue = padapter->pfree_txreq_queue;
	u32 sta_atm_time = dvobj->txq_hw_timeout;
	u32 sta_ts_txreq_limit;
	int cur_ts_used;

	if (pstaxmitpriv->ts_limit)
		sta_atm_time = pstaxmitpriv->ts_limit;

	sta_ts_txreq_limit = sta_atm_time*dvobj->txq_ts_factor;
	if (dvobj->txq_deq_factor)
		sta_atm_time = sta_atm_time*(dvobj->txq_deq_factor<<dvobj->txq_serv_group_exp)/100;
	else if (dvobj->txq_timeout_avg > dvobj->txq_hw_timeout)
		sta_atm_time = sta_atm_time*(dvobj->txq_timeout_avg<<dvobj->txq_serv_group_exp)/dvobj->txq_hw_timeout;

	rate = pstaxmitpriv->tx_rate_mbps_retry;
	if (0 == rate) {
		if (WIFI_ROLE_IS_ON_24G(padapter))
			rate = 1;
		else
			rate = 6;
	}

	max_agg_num = dvobj->txq_max_agg_num;
	if (psta->phl_sta->asoc_cap.num_ampdu && psta->phl_sta->asoc_cap.num_ampdu < max_agg_num)
		max_agg_num = psta->phl_sta->asoc_cap.num_ampdu;

	if (pstaxmitpriv->tx_pending_bitmap & BIT(TXQ_MGT)) {
		struct xmit_frame *pmgntframe;
		while (1) {
			pmgntframe = rtw_dequeue_mgtq_xmitframe(padapter, pstaxmitpriv);
			if (!pmgntframe)
				break;
			rtw_core_tx_mgmt(padapter, pmgntframe);
		}
	}

	if ((psta != padapter->self_sta)) {
		cur_ts_used = ATOMIC_READ(&pstaxmitpriv->txreq_ts_used);
		if (cur_ts_used > 0 && sta_ts_txreq_limit < cur_ts_used) {
			++pstaxmitpriv->txreq_ts_limit_exceed_cnt;
			status = TXQ_STATE_EXCEED_TXREQ_TS_LIMIT;
			goto move_to_tail;
		}
	}

	for (q_idx = 0; q_idx < MAX_TXQ; q_idx++) {
		if (!(pstaxmitpriv->tx_pending_bitmap & BIT(q_idx)))
			continue;

		if (pstaxmitpriv->ts_used >= sta_atm_time) {
			status = TXQ_STATE_EXCEED_TS_LIMIT;
			break;
		}

		txq = &pstaxmitpriv->txq[q_idx];
		if (txq->qhead.qlen) {
			if (txq->qhead.qlen >= max_agg_num)
				cnt = max_agg_num;
			else if (time_after_eq(rtw_get_current_time(), txq->timeout))
				cnt = txq->qhead.qlen;
			else if (TRAFFIC_MODE_RX == psta->traffic_mode)
				cnt = 1;
			else
				continue;
			if (cnt > pfree_txreq_queue->qlen) {
				status = TXQ_STATE_INSUFFICIENT_TXREQ;
				goto out;
			}
		}

		nr_tcpack_strip = 0;
		if (dvobj->txq_tcpack_merge && TRAFFIC_MODE_RX == psta->traffic_mode) {
			nr_tcpack_strip = tcp_ack_merge(txq);
			if (nr_tcpack_strip) {
				ATOMIC_SUB(&dvobj->txq_total_len, nr_tcpack_strip);
				ATOMIC_SUB(&pstaxmitpriv->txq_total_len, nr_tcpack_strip);
			}
		}

		for (deq_num = 0; deq_num < max_agg_num; deq_num++) {
			if (!pfree_txreq_queue->qlen)
				break;

			if (pstaxmitpriv->ts_used >= sta_atm_time) {
				status = TXQ_STATE_EXCEED_TS_LIMIT;
				break;
			}

			_rtw_spinlock_bh(&txq->qhead.lock);
			pskb = __skb_dequeue(&txq->qhead);
			if (txq->qhead.qlen == 0)
				rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
			_rtw_spinunlock_bh(&txq->qhead.lock);

			if (!pskb)
				break;

			ATOMIC_DEC(&dvobj->txq_total_len);
			ATOMIC_DEC(&pstaxmitpriv->txq_total_len);

			ts_consumed = (pskb->len << 3)/rate;
			if (0 == ts_consumed)
				ts_consumed = 1;

			*(u32 *)&pskb->cb[_SKB_CB_AIRTIME] = ts_consumed;
			pstaxmitpriv->ts_used += ts_consumed;

			rtw_core_tx(padapter, &pskb, psta);
			deq_num += pstaxmitpriv->txq_extra_deq;
			pstaxmitpriv->txq_extra_deq = 0;
		}

		if (deq_num) {
			pstaxmitpriv->txq_cur_dequeue += deq_num;
			_rtw_spinlock_bh(&txq->qhead.lock);
			pskb = skb_peek(&txq->qhead);
			if (pskb)
				txq->timeout = *(systime *)&pskb->cb[_SKB_CB_TIME];
			_rtw_spinunlock_bh(&txq->qhead.lock);

			if (deq_num+nr_tcpack_strip >= max_agg_num) {
				dvobj->txq_pkt_trigger++;
				pstaxmitpriv->txq_pkt_trigger++;
				if (txq->qhead.qlen >= max_agg_num)
					trigger |= (PKT_TRIGGER|BIT2);
				else
					trigger |= PKT_TRIGGER;
			} else {
				dvobj->txq_timeout_trigger++;
				pstaxmitpriv->txq_timeout_trigger++;
				trigger |= TIMEOUT_TRIGGER;
			}
		}
	}

	if (status == TXQ_STATE_EXCEED_TXREQ_TS_LIMIT) {
		++pstaxmitpriv->txreq_ts_limit_exceed_cnt;
	} else if (TIMEOUT_TRIGGER == trigger) {
		pstaxmitpriv->txq_timeout = 1;
	} else if (pstaxmitpriv->tx_pending_bitmap) {
		if (!trigger) {
			status = TXQ_STATE_WAITING;
		} else { /* pkt trigger */
			pstaxmitpriv->txq_timeout = 0;
			if (!(trigger & BIT2)) /* buffered pkt < max_agg_num */
				status = TXQ_STATE_WAITING;
			/* else
				status = TXQ_STATE_OK; */
		}
	}

move_to_tail:

	sta_queue = &dvobj->tx_pending_sta_queue;
	_rtw_spinlock_bh(&sta_queue->lock);
	if (rtw_is_list_empty(&pstaxmitpriv->tx_pending) == _FALSE) {
		if (pstaxmitpriv->tx_pending_bitmap) {
			list_del(&pstaxmitpriv->tx_pending);
			list_add_tail(&pstaxmitpriv->tx_pending, get_list_head(sta_queue));
		} else {
			rtw_list_delete(&pstaxmitpriv->tx_pending);
			sta_queue->qlen--;
			status = TXQ_STATE_EMPTY;
		}
	}
	_rtw_spinunlock_bh(&sta_queue->lock);

out:
	return status;
}

void service_sta_q_hdl(struct dvobj_priv *dvobj)
{
	_queue *sta_queue;
	struct sta_info *psta;
	struct sta_xmit_priv *pstaxmitpriv;
	_list *phead, *plist;
	int status;
	int waiting = 0;
	systime timeout;
	u32 ts_limit_sta;
	u32 qlen = 0;
	u32 handle_cnt = 0;
	u32 serv_group;

	if (dvobj->ps_trigger_sta_queue.qlen) {
		status = ps_trigger_hdl(dvobj);
		if (status != TXQ_STATE_OK)
			return;
	}

	dvobj->txq_service++;
	timeout = rtw_get_current_time() + rtw_ms_to_systime(dvobj->txq_max_serv_time);

	sta_queue = &dvobj->tx_pending_sta_queue;
	phead = get_list_head(sta_queue);
	qlen = sta_queue->qlen;

	while ((plist = get_next(phead)) != phead) {
		if (handle_cnt >= qlen*dvobj->txq_deq_loop)
			break;

		if (rtw_time_after(rtw_get_current_time(), timeout)) {
			dvobj->txq_serv_timeout++;
			return;
		}
		psta = LIST_CONTAINOR(plist, struct sta_info, sta_xmitpriv.tx_pending);
		pstaxmitpriv = &psta->sta_xmitpriv;

		handle_cnt++;

		if (pstaxmitpriv->txq_service != dvobj->txq_service) {
			serv_group = dvobj->txq_service>>dvobj->txq_serv_group_exp;
			if (serv_group != pstaxmitpriv->txq_service_group)
				pstaxmitpriv->ts_used = 0;
			pstaxmitpriv->txq_service_group = serv_group;
			pstaxmitpriv->txq_service_update++;
		}

		pstaxmitpriv->txq_service = dvobj->txq_service;

		status = service_sta_hdl(dvobj, psta);

		if (TXQ_STATE_EXCEED_TS_LIMIT == status)
			continue;
		if (TXQ_STATE_EXCEED_TXREQ_TS_LIMIT == status)
			continue;
		if (TXQ_STATE_INSUFFICIENT_TXREQ == status)
			return;
		if (TXQ_STATE_WAITING == status && !pstaxmitpriv->txq_timeout)
			waiting++;

		if (dvobj->ps_trigger_sta_queue.qlen) {
			status = ps_trigger_hdl(dvobj);
			if (status != TXQ_STATE_OK)
				return;
		}
	}
}

#ifdef CONFIG_TXSC_AMSDU
u8 txsc_amsdu_dequeue_txq(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct sta_info *psta = txsc_pkt->psta;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_tx_queue *txq;
	struct sk_buff *pskb, *next_skb;
	u8 q_idx, deq_num;
	u32 total_len = 0, required_len;
	int max_size;
	u8 max_msdu;

	max_msdu = psta->txsc_amsdu_num;
	if (max_msdu == 0)
		max_msdu = padapter->tx_amsdu;
	if (max_msdu == 1)
		return 1;

	max_size = psta->txsc_amsdu_size;
	if (0 == max_size)
		max_size = 3144; /* = (512B+8B+4B)*6 */

	/* exclude 1st amsdu subframe */
	pskb = txsc_pkt->xmit_skb[0];
	max_size -= (pskb->len + RTW_SZ_LLC + 4);
	max_msdu--;
	deq_num = 0;

	q_idx = pskb->cb[_SKB_CB_QNUM];
	txq = &pstaxmitpriv->txq[q_idx];

	_rtw_spinlock_bh(&txq->qhead.lock);
	while (1) {
		next_skb = skb_peek(&txq->qhead);
		if (!next_skb)
			break;
		if (deq_num >= max_msdu)
			break;

		required_len = next_skb->len + RTW_SZ_LLC + 4;
		if (total_len + required_len > max_size)
			break;
		total_len += required_len;

		__skb_unlink(next_skb, &txq->qhead);
		txsc_pkt->xmit_skb[++deq_num] = next_skb;
	}
	if (next_skb)
		txq->timeout = *(systime *)&next_skb->cb[_SKB_CB_TIME];
	else
		rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
	_rtw_spinunlock_bh(&txq->qhead.lock);

	if (deq_num) {
		u32 ts_consumed;
		ATOMIC_SUB(&dvobj->txq_total_len, deq_num);
		ATOMIC_SUB(&pstaxmitpriv->txq_total_len, deq_num);
		pstaxmitpriv->txq_extra_deq = deq_num;

		ts_consumed = (total_len <<3)/pstaxmitpriv->tx_rate_mbps_retry;
		if (0 == ts_consumed)
			ts_consumed = 1;
		pstaxmitpriv->ts_used += ts_consumed;
		*(u32 *)&pskb->cb[_SKB_CB_AIRTIME] += ts_consumed;
	}

	pxmitpriv->cnt_txsc_amsdu_dump[deq_num]++;
	txsc_pkt->skb_cnt = deq_num + 1; /* include 1st amsdu subframe */

	return txsc_pkt->skb_cnt;
}

u8 txsc_amsdu_combine(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct sta_info *psta = txsc_pkt->psta;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_tx_queue *txq;
	struct sk_buff *pskb, *next_skb;
	u8 q_idx, buf_num;
	u32 total_len, required_len;
	int max_size;
	u8 max_msdu;

	u16 amsdu_merge = dvobj->txq_amsdu_merge;
	struct sk_buff_head skb_list;
	u16 merge_num[MAX_TXSC_SKB_NUM] = {0};
	u16 msdu_num = 0;
	int tailroom, i, j;

	max_msdu = psta->txsc_amsdu_num;
	if (0 == max_msdu)
		max_msdu = padapter->tx_amsdu;


	max_size = psta->txsc_amsdu_size;
	if (0 == max_size)
		max_size = 3144; /* = (512B+8B+4B)*6 */

	/* exclude 1st amsdu subframe */
	pskb = txsc_pkt->xmit_skb[0];
	max_size -= (pskb->len + RTW_SZ_LLC + 4);
	max_msdu--;
	amsdu_merge--;
	buf_num = 0;
	total_len = 0;

	__skb_queue_head_init(&skb_list);
	tailroom = skb_tailroom(pskb);
	q_idx = pskb->cb[_SKB_CB_QNUM];
	txq = &pstaxmitpriv->txq[q_idx];

	_rtw_spinlock_bh(&txq->qhead.lock);
	while (1) {
		next_skb = skb_peek(&txq->qhead);
		if (!next_skb)
			break;
		required_len = next_skb->len + RTW_SZ_LLC + 4;
		if (total_len + required_len > max_size)
			break;
		if (buf_num <= max_msdu && next_skb->len < 128 && required_len <= tailroom) {
			if (msdu_num >= amsdu_merge)
				break;
			__skb_unlink(next_skb, &txq->qhead);
			__skb_queue_tail(&skb_list, next_skb);

			total_len += required_len;
			tailroom -= required_len;
			msdu_num++;
			merge_num[buf_num]++;
			continue;
		}
		if (buf_num >= max_msdu)
			break;

		__skb_unlink(next_skb, &txq->qhead);
		txsc_pkt->xmit_skb[++buf_num] = next_skb;

		total_len += required_len;
		tailroom = skb_tailroom(next_skb);
		msdu_num++;
	}
	if (next_skb)
		txq->timeout = *(systime *)&next_skb->cb[_SKB_CB_TIME];
	else
		rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
	_rtw_spinunlock_bh(&txq->qhead.lock);

	for (i = 0; i <= buf_num; i++) {
		u16 protocol, padding;
		pskb = txsc_pkt->xmit_skb[i];
		ieee8023_header_to_rfc1042_txsc(pskb, false);

		for (j = 0; j < merge_num[i]; j++) {
			u8 *pethhdr;
			u8 *pmsdu_hdr;
			/* padding for 4-bytes alignment */
			padding = pskb->len & 0x3;
			if (padding)
				skb_put(pskb, 4-padding);

			next_skb = __skb_dequeue(&skb_list);
			if (next_skb)
				pethhdr = (u8 *)next_skb->data;
			protocol = ntohs(*(u16 *)(pethhdr + ETH_ALEN*2));
			if (protocol + ETH_HLEN > ETH_FRAME_LEN) {
				u16 len;
				pmsdu_hdr = skb_tail_pointer(pskb);
				memcpy(pmsdu_hdr, pethhdr, ETH_ALEN*2);
				len = cpu_to_be16(next_skb->len - ETH_HLEN + RTW_SZ_LLC);
				memcpy(pmsdu_hdr + ETH_ALEN*2, &len, 2);
				memcpy(pmsdu_hdr + ETH_HLEN, rtw_rfc1042_header, SNAP_SIZE);
				memcpy(pmsdu_hdr + ETH_HLEN+SNAP_SIZE, pethhdr+ETH_ALEN*2,
					next_skb->len-ETH_ALEN*2);
				skb_put(pskb, next_skb->len+RTW_SZ_LLC);
			} else
				memcpy(skb_put(pskb, next_skb->len), pethhdr, next_skb->len);
			rtw_skb_free(next_skb);
		}
		if (i < buf_num) {
			/* padding for 4-bytes alignment */
			padding = pskb->len & 0x3;
			if (padding)
				skb_put(pskb, 4-padding);
		}
	}

	if (msdu_num) {
		u32 ts_consumed;
		ATOMIC_SUB(&dvobj->txq_total_len, msdu_num);
		ATOMIC_SUB(&pstaxmitpriv->txq_total_len, msdu_num);
		pstaxmitpriv->txq_extra_deq = msdu_num;

		ts_consumed = (total_len <<3)/pstaxmitpriv->tx_rate_mbps_retry;
		if (0 == ts_consumed)
			ts_consumed = 1;
		pstaxmitpriv->ts_used += ts_consumed;
		*(u32 *)&txsc_pkt->xmit_skb[0]->cb[_SKB_CB_AIRTIME]+= ts_consumed;
	}

	pxmitpriv->cnt_txsc_amsdu_dump[buf_num]++;
	txsc_pkt->skb_cnt = buf_num + 1; /* include 1st amsdu subframe */

	return txsc_pkt->skb_cnt;
}
#endif


extern void rtw_core_set_gt3(_adapter *padapter, u8 enable, long timeout);

void rtw_txq_hdl(struct dvobj_priv *dvobj)
{
	_adapter *prim_adp = NULL;
	u32 diff;
	u32 passing_time;

	if (RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO("%s => bDriverStopped or bSurpriseRemoved\n",
			__func__);
		return;
	}

	prim_adp = dvobj_get_primary_adapter(dvobj);

	dvobj->txq_timeout_seq++;
	passing_time = rtw_get_passing_time_ms(dvobj->atm_last_time);
	if (passing_time >= 50) { /* count average timeout in 50ms*/
		if (dvobj->txq_timeout_seq >= dvobj->txq_timeout_seq_last)
			diff = dvobj->txq_timeout_seq - dvobj->txq_timeout_seq_last;
		else
			diff = 0xffffffff - dvobj->txq_timeout_seq_last + dvobj->txq_timeout_seq;
		dvobj->txq_timeout_avg = passing_time*1000/diff;
		dvobj->txq_timeout_seq_last = dvobj->txq_timeout_seq;
		dvobj->atm_last_time = rtw_get_current_time();
	}
	service_sta_q_hdl(dvobj);

	rtw_core_set_gt3(prim_adp, 1, dvobj->txq_hw_timeout);
}
#endif /* CONFIG_ONE_TXQ */

//#if !defined(CONFIG_ONE_TXQ)
void pspoll_trigger_hdl2(struct dvobj_priv *dvobj, struct sta_info *psta)
{
	_adapter *padapter = psta->padapter;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	u16 tx_pending_bitmap;
	int q_idx;
	struct sta_tx_queue *txq;
	struct sk_buff *pskb = NULL;
	struct xmit_frame *pmgntframe = NULL;
	_queue *sta_queue;

	tx_pending_bitmap = pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap;
	if (tx_pending_bitmap) {
		if (tx_pending_bitmap & BIT(TXQ_MGT)) {
			pmgntframe = rtw_dequeue_mgtq_xmitframe(padapter, pstaxmitpriv);
			if (pmgntframe) {
				if (pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap)
					set_mgntframe_mdata(pmgntframe);
				rtw_core_tx_mgmt(padapter, pmgntframe);
				goto update_state;
			}
		}

		for (q_idx = 0; q_idx < MAX_TXQ; q_idx++) {
			if (!(tx_pending_bitmap & BIT(q_idx)))
				continue;

			txq = &pstaxmitpriv->txq[q_idx];

			_rtw_spinlock_bh(&txq->qhead.lock);
			pskb = __skb_dequeue(&txq->qhead);
			if (txq->qhead.qlen == 0)
				rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
			_rtw_spinunlock_bh(&txq->qhead.lock);

			if (pskb) {
				ATOMIC_DEC(&pstaxmitpriv->txq_total_len);
				rtw_core_tx(padapter, &pskb, psta);
				break;
			}
		}
	}

update_state:

	tx_pending_bitmap = pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap;
	if (!tx_pending_bitmap) {
		if (rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
			rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
			update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
		}

		if (!pskb && !pmgntframe) {
			RTW_INFO("no buffered packets to xmit\n");
			/* issue nulldata with More data bit = 0 to indicate we have no buffered packets */
			issue_nulldata(padapter, psta->phl_sta->mac_addr, 0, 0, 0);
		}
	}

	sta_queue = &dvobj->ps_trigger_sta_queue;

	_rtw_spinlock_bh(&sta_queue->lock);
	pstaxmitpriv->ps_trigger_type &= ~ BIT0;
	if (!pstaxmitpriv->ps_trigger_type) {
		if (rtw_is_list_empty(&pstaxmitpriv->ps_trigger) == _FALSE) {
			rtw_list_delete(&pstaxmitpriv->ps_trigger);
			sta_queue->qlen--;
		}
	}
	_rtw_spinunlock_bh(&sta_queue->lock);
}

#ifdef CONFIG_ETHER_PKT_AGG
u8 agg_add_pkt_to_pktlist(struct sk_buff **pskb_head, struct sk_buff **pskb_tail, struct sk_buff *pskb)
{
	if (*pskb_head == NULL)
		*pskb_head = *pskb_tail = pskb;
	else {
		(*pskb_tail)->next = pskb;
		*pskb_tail = pskb;
	}

	if (pskb->cb[_SKB_CB_ETH_AGG_2] == 1) {
		pskb->next = NULL;
		return 1;
	}
	return 0;
}
#endif

void wmmps_trigger_hdl2(struct dvobj_priv *dvobj, struct sta_info *psta)
{
	_adapter *padapter = psta->padapter;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	u16 tx_pending_bitmap;
	int q_idx;
	struct sta_tx_queue *txq;
	struct sk_buff *pskb = NULL;
	_queue *sta_queue;
	#ifdef CONFIG_ETHER_PKT_AGG
	struct sk_buff *pskb_tail = NULL, *pskb_head = NULL;
	#endif

	if (pstaxmitpriv->tx_pending_bitmap & BIT(TXQ_MGT)) {
		struct xmit_frame *pmgntframe;
		u16 mask = psta->uapsd_bitmap | BIT(TXQ_MGT);
		while (1) {
			pmgntframe = rtw_dequeue_mgtq_xmitframe(padapter, pstaxmitpriv);
			if (!pmgntframe)
				break;

			if (pstaxmitpriv->tx_pending_bitmap & mask)
				set_mgntframe_mdata(pmgntframe);
			rtw_core_tx_mgmt(padapter, pmgntframe);
		}
	}

	tx_pending_bitmap = pstaxmitpriv->tx_pending_bitmap & psta->uapsd_bitmap;
	if (tx_pending_bitmap) {
		for (q_idx = 0; q_idx < MAX_TXQ; q_idx++) {
			if (!(tx_pending_bitmap & BIT(q_idx)))
				continue;

			txq = &pstaxmitpriv->txq[q_idx];
			while (1) {
				_rtw_spinlock_bh(&txq->qhead.lock);
				pskb = __skb_dequeue(&txq->qhead);
				if (txq->qhead.qlen == 0)
					rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
				_rtw_spinunlock_bh(&txq->qhead.lock);

				if (!pskb)
					break;

				#ifdef CONFIG_ETHER_PKT_AGG
				if (pskb->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_PKTLIST) {
					ATOMIC_DEC(&pstaxmitpriv->txq_total_len);
					if(agg_add_pkt_to_pktlist(&pskb_head, &pskb_tail, pskb) == 1) {
						rtw_core_tx(padapter, &pskb_head, psta);
						pskb_head = NULL;
					}
				} else
				#endif
				{
					ATOMIC_DEC(&pstaxmitpriv->txq_total_len);
					rtw_core_tx(padapter, &pskb, psta);
				}

			}
		}
	} else {
		/* issue one qos null frame with More data bit = 0 and the EOSP bit set (=1) */
		issue_qos_nulldata(padapter, psta->phl_sta->mac_addr, 7, 0, 0, 0);
	}

	if (psta->uapsd_bitmap == (BIT(TXQ_VO)|BIT(TXQ_VI)|BIT(TXQ_BE)|BIT(TXQ_BK))
		&& !pstaxmitpriv->tx_pending_bitmap
#ifdef CONFIG_TDLS
		&& !(psta->tdls_sta_state & TDLS_LINKED_STATE)
#endif
		) {
		if (rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
			rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
			update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
		}
	}

	sta_queue = &dvobj->ps_trigger_sta_queue;

	_rtw_spinlock_bh(&sta_queue->lock);
	pstaxmitpriv->ps_trigger_type &= ~ BIT1;
	if (!pstaxmitpriv->ps_trigger_type) {
		if (rtw_is_list_empty(&pstaxmitpriv->ps_trigger) == _FALSE) {
			rtw_list_delete(&pstaxmitpriv->ps_trigger);
			sta_queue->qlen--;
		}
	}
	_rtw_spinunlock_bh(&sta_queue->lock);
}

void ps_trigger_hdl2(struct dvobj_priv *dvobj)
{
	_queue *sta_queue;
	struct sta_info *psta;
	struct sta_xmit_priv *pstaxmitpriv;
	_list *phead, *plist;

	sta_queue = &dvobj->ps_trigger_sta_queue;
	phead = get_list_head(sta_queue);

	while ((plist = get_next(phead)) != phead) {
		psta = LIST_CONTAINOR(plist, struct sta_info, sta_xmitpriv.ps_trigger);
		pstaxmitpriv = &psta->sta_xmitpriv;

		if (pstaxmitpriv->ps_trigger_type & BIT0)
			pspoll_trigger_hdl2(dvobj, psta);
		if (pstaxmitpriv->ps_trigger_type & BIT1)
			wmmps_trigger_hdl2(dvobj, psta);
	}
}

void service_sta_hdl2(struct dvobj_priv *dvobj, struct sta_info *psta)
{
	_adapter *padapter = psta->padapter;
	struct sta_xmit_priv *pstaxmitpriv = &psta->sta_xmitpriv;
	struct sta_tx_queue *txq;
	int q_idx;
	struct sk_buff *pskb;
	#ifdef CONFIG_ETHER_PKT_AGG
	struct sk_buff *pskb_tail = NULL, *pskb_head = NULL;
	#endif
	_queue *sta_queue;

	if (pstaxmitpriv->tx_pending_bitmap & BIT(TXQ_MGT)) {
		struct xmit_frame *pmgntframe;
		while (1) {
			pmgntframe = rtw_dequeue_mgtq_xmitframe(padapter, pstaxmitpriv);
			if (!pmgntframe)
				break;
			rtw_core_tx_mgmt(padapter, pmgntframe);
		}
	}

	for (q_idx = 0; q_idx < MAX_TXQ; q_idx++) {
		if (!(pstaxmitpriv->tx_pending_bitmap & BIT(q_idx)))
			continue;

		txq = &pstaxmitpriv->txq[q_idx];
		while (1) {
			_rtw_spinlock_bh(&txq->qhead.lock);
			pskb = __skb_dequeue(&txq->qhead);
			if (txq->qhead.qlen == 0)
				rtw_clear_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
			_rtw_spinunlock_bh(&txq->qhead.lock);

			if (!pskb)
				break;

			#ifdef CONFIG_ETHER_PKT_AGG
			if (pskb->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_PKTLIST) {
				ATOMIC_DEC(&pstaxmitpriv->txq_total_len);
				if(agg_add_pkt_to_pktlist(&pskb_head, &pskb_tail, pskb) == 1) {
					rtw_core_tx(padapter, &pskb_head, psta);
					pskb_head = NULL;
				}
			} else
			#endif
			{
				ATOMIC_DEC(&pstaxmitpriv->txq_total_len);
				rtw_core_tx(padapter, &pskb, psta);
			}
		}
	}

move_to_tail:

	sta_queue = &dvobj->tx_pending_sta_queue;
	_rtw_spinlock_bh(&sta_queue->lock);
	if (rtw_is_list_empty(&pstaxmitpriv->tx_pending) == _FALSE) {
		if (pstaxmitpriv->tx_pending_bitmap) {
			list_del(&pstaxmitpriv->tx_pending);
			list_add_tail(&pstaxmitpriv->tx_pending, get_list_head(sta_queue));
		} else {
			rtw_list_delete(&pstaxmitpriv->tx_pending);
			sta_queue->qlen--;
		}
	}
	_rtw_spinunlock_bh(&sta_queue->lock);
}

void service_sta_q_hdl2(struct dvobj_priv *dvobj)
{
	_queue *sta_queue;
	struct sta_info *psta;
	_list *phead, *plist;

	if (dvobj->ps_trigger_sta_queue.qlen)
		ps_trigger_hdl2(dvobj);

	sta_queue = &dvobj->tx_pending_sta_queue;
	phead = get_list_head(sta_queue);

	while ((plist = get_next(phead)) != phead) {
		psta = LIST_CONTAINOR(plist, struct sta_info, sta_xmitpriv.tx_pending);

		service_sta_hdl2(dvobj, psta);

		if (dvobj->ps_trigger_sta_queue.qlen)
			ps_trigger_hdl2(dvobj);
	}
}

#ifdef CONFIG_ETHER_PKT_AGG
u8 agg_get_skb_from_pktlist(struct sk_buff **xmit_skb, struct sk_buff *pkt)
{
	u8 i;

	xmit_skb[0] = pkt;
	for (i = 0; i < MAX_TXSC_SKB_NUM; i++) {
		if (xmit_skb[i]->next == NULL)
			break;
		xmit_skb[i + 1] = xmit_skb[i]->next;
		xmit_skb[i]->cb[_SKB_CB_ETH_AGG_2] = 0;
	}
	xmit_skb[i]->cb[_SKB_CB_ETH_AGG_2] = 1; // last packet
	return i + 1;


}
#endif

sint os_tx_enqueue_for_sleeping_sta(_adapter *padapter, struct sta_info *psta, struct sk_buff *pkt)
{
	struct sta_xmit_priv *pstaxmitpriv;
	struct sta_tx_queue *txq;
	u8 q_idx;
	sint ret = PS_TX_ENQ_FAIL;
	u8 skb_cnt;
	#ifdef CONFIG_ETHER_PKT_AGG
	struct sk_buff *xmit_skb[MAX_TXSC_SKB_NUM];
	u8 i;
	#endif
	u32 cur_tx_tp = psta->sta_stats.tx_tp_kbits >> 10;

	if (!MLME_IS_AP(padapter) && !MLME_IS_MESH(padapter)
#ifdef CONFIG_TDLS
		&& !(padapter->tdlsinfo.link_established == _TRUE && (psta->tdls_sta_state & TDLS_LINKED_STATE))
#endif
		)
		return ret;

#ifdef CONFIG_RTW_TWT
	if(rtw_core_twt_sta_active(padapter, psta) ||
		((padapter->twt_cmd_pwrbit) && (padapter->twt_cmd_macid==psta->phl_sta->macid)))
		return ret;
#endif

	pstaxmitpriv = &psta->sta_xmitpriv;

	if (psta->state & WIFI_SLEEP_STATE) {
		q_idx = pkt->cb[_SKB_CB_QNUM];
		txq = &pstaxmitpriv->txq[q_idx];
		skb_cnt = 1;

		if((txq->qhead.qlen >  GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib.sleep_q_max_num)
			&& (cur_tx_tp == 0)){
			RTW_WARN("STA[%pM] buffered too much data, then drop pkt! txq->qhead.qlen=%d sleep_q_max_num=%d\n",
				psta->phl_sta->mac_addr, txq->qhead.qlen, GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib.sleep_q_max_num);
			return PS_TX_ENQ_DROP;
		}

		#ifdef CONFIG_ETHER_PKT_AGG
		if (pkt->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_PKTLIST)
			skb_cnt = agg_get_skb_from_pktlist(xmit_skb, pkt);
		#endif

		_rtw_spinlock_bh(&txq->qhead.lock);
		#ifdef CONFIG_ETHER_PKT_AGG
		if (skb_cnt > 1) {
			for (i = 0; i < skb_cnt; i++) {
				__skb_queue_tail(&txq->qhead, xmit_skb[i]);
			}
		} else
		#endif
		__skb_queue_tail(&txq->qhead, pkt);

		if (txq->qhead.qlen == skb_cnt)
			rtw_set_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);

		_rtw_spinunlock_bh(&txq->qhead.lock);


		#ifdef CONFIG_ONE_TXQ
		ATOMIC_ADD(&padapter->dvobj->txq_total_len, skb_cnt);
		#endif
		ATOMIC_ADD(&pstaxmitpriv->txq_total_len, skb_cnt);

#ifdef CONFIG_TDLS
		if (padapter->tdlsinfo.link_established == _TRUE
			&& (psta->tdls_sta_state & TDLS_LINKED_STATE)) {
			/* Transmit TDLS PTI via AP */
			if (ATOMIC_READ(&pstaxmitpriv->txq_total_len) == 1)
				rtw_tdls_cmd(padapter, psta->phl_sta->mac_addr, TDLS_ISSUE_PTI);
		} else
#endif
		if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) {
			if ((psta->uapsd_bitmap == (BIT(TXQ_VO)|BIT(TXQ_VI)|BIT(TXQ_BE)|BIT(TXQ_BK))
					&& pstaxmitpriv->tx_pending_bitmap)
					|| (pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap)) {
				struct sta_priv *pstapriv = &padapter->stapriv;
				if (!rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
					rtw_tim_map_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
					update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
				}
			}
		}

		ret = PS_TX_ENQ_SUCCESS;

		DBG_COUNTER(padapter->tx_logs.core_tx_ap_enqueue_ucast);
	} else if (pstaxmitpriv->tx_pending_bitmap) {
		q_idx = pkt->cb[_SKB_CB_QNUM];
		txq = &pstaxmitpriv->txq[q_idx];

		skb_cnt = 1;
		#ifdef CONFIG_ETHER_PKT_AGG
		if (pkt->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_PKTLIST)
			skb_cnt = agg_get_skb_from_pktlist(xmit_skb, pkt);
		#endif
		_rtw_spinlock_bh(&txq->qhead.lock);
		#ifdef CONFIG_ETHER_PKT_AGG
		if (skb_cnt > 1) {
			for (i = 0; i < skb_cnt; i++)
				__skb_queue_tail(&txq->qhead, xmit_skb[i]);
		} else
		#endif
			__skb_queue_tail(&txq->qhead, pkt);

		if (txq->qhead.qlen == skb_cnt)
			rtw_set_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
		_rtw_spinunlock_bh(&txq->qhead.lock);

		#ifdef CONFIG_ONE_TXQ
		ATOMIC_ADD(&padapter->dvobj->txq_total_len, skb_cnt);
		#endif
		ATOMIC_ADD(&pstaxmitpriv->txq_total_len, skb_cnt);

		ret = PS_TX_ENQ_SUCCESS;
	}

	return ret;
}
//#endif /* !CONFIG_ONE_TXQ */

#ifdef CONFIG_TX_MCAST2UNI
unsigned int ipv4_mc_reserved_num = sizeof(ipv4_mc_reserved_addr) / sizeof(u32);
unsigned int ipv6_mc_reserved_num = sizeof(ipv6_mc_reserved_addr) / (sizeof(u32) * 4);

enum mc2u_op {
	MC2U_NORMAL,
	MC2U_FORBID,
	MC2U_ANYWAY,
};

enum mc2u_op rtw_m2u_check(_adapter *padapter, struct sk_buff *skb)
{
	unsigned short L3_protocol = 0;
	unsigned char *ptr = NULL;
	unsigned int current_index = 0, tmp_current_index = 0, last_index = 0, lower_index = 0, upper_index = 0;

	ptr = skb->data + 2*ETH_ALEN;
	L3_protocol = *(unsigned short *)ptr;
	if(L3_protocol == __constant_htons(ETH_P_8021Q))
	{
		ptr = ptr + 4;
		L3_protocol = *(unsigned short *)ptr;
	}

	if(L3_protocol == __constant_htons(ETH_P_IP))
	{
		struct iphdr *iph = (struct iphdr *)(ptr + 2);
		u32 group_address = 0;
		if(iph->protocol == IPPROTO_IGMP)
		{
			if (padapter->registrypriv.wifi_mib.mc2u_ipv6_logo)
				return MC2U_ANYWAY;
			else {
				RTW_DBG("is IGMP protocol, not do m2u\n");
				return MC2U_FORBID;
			}
		}
		group_address = __constant_ntohl(iph->daddr);
		current_index = ipv4_mc_reserved_num >> 1;
		lower_index = 0;
		upper_index = ipv4_mc_reserved_num;
		while(current_index != last_index)
		{
			if(ipv4_mc_reserved_addr[current_index] == group_address)
			{
				RTW_DBG("is ipv4 reserved mc addr: %x, not do m2u\n", group_address);
				return MC2U_FORBID;
			}
			else if(ipv4_mc_reserved_addr[current_index] > group_address)
			{
				tmp_current_index = current_index;
				current_index = (current_index + lower_index) >> 1;
				upper_index = tmp_current_index;
				last_index = tmp_current_index;
			}
			else
			{
				tmp_current_index = current_index;
				current_index = (current_index + upper_index) >> 1;
				lower_index = tmp_current_index;
				last_index = tmp_current_index;
			}
		}
	}
	else if(L3_protocol == __constant_htons(ETH_P_IPV6))
	{
		struct ipv6hdr *ipv6h = (struct ipv6hdr *)(ptr + 2);
		u32 group_address[4];

		if((ptr[8] == 0x3a) || ( ptr[8] == 0x0 && ptr[42] == 0x3a)) //next header is icmpv6
		{
			if (padapter->registrypriv.wifi_mib.mc2u_ipv6_logo)
				return MC2U_ANYWAY;
			else {
				RTW_DBG("is icmpv6 protocol, not do m2u\n");
				return MC2U_FORBID;
			}
		}

		group_address[0] = __constant_ntohl(ipv6h->daddr.s6_addr32[0]);
		group_address[1] = __constant_ntohl(ipv6h->daddr.s6_addr32[1]);
		group_address[2] = __constant_ntohl(ipv6h->daddr.s6_addr32[2]);
		group_address[3] = __constant_ntohl(ipv6h->daddr.s6_addr32[3]);
		/*
			ff02::1:FFxx:xxxx
			SLP(ff05::1:1000~ff05::1:13ff)
		*/
		if(IPV6_RESERVED_ADDRESS_RANGE(group_address))
		{
			RTW_DBG("is ipv6 reserved mc addr range: %x:%x:%x:%x, not do m2u\n", group_address[0], group_address[1], group_address[2], group_address[3]);
			return MC2U_FORBID;
		}
		current_index = ipv6_mc_reserved_num >> 1;
		lower_index = 0;
		upper_index = ipv6_mc_reserved_num;
		while(current_index != last_index)
		{
			if(IPV6_ADDRESS_EQU(ipv6_mc_reserved_addr[current_index], group_address))
			{
				RTW_DBG("is ipv6 reserved mc addr: %x:%x:%x:%x, not do m2u\n", group_address[0], group_address[1], group_address[2], group_address[3]);
				return MC2U_FORBID;
			}
			else if(IPV6_ADDRESS_GTR(ipv6_mc_reserved_addr[current_index], group_address))
			{
				tmp_current_index = current_index;
				current_index = (current_index + lower_index) >> 1;
				upper_index = tmp_current_index;
				last_index = tmp_current_index;
			}
			else
			{
				tmp_current_index = current_index;
				current_index = (current_index + upper_index) >> 1;
				lower_index = tmp_current_index;
				last_index = tmp_current_index;
			}
		}
	}
	else
	{
		RTW_DBG("not ip protocol, not do m2u\n");
		return MC2U_FORBID;
	}

	return MC2U_NORMAL;
}

int rtw_mlcst2unicst(_adapter *padapter, struct sk_buff *skb)
{
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_xmit_priv *pstaxmitpriv;
	_list	*phead, *plist;
	struct sk_buff *newskb;
	struct sta_info *psta = NULL, *sa_psta = NULL;
	u8 chk_alive_num = 0;
	char chk_alive_list[NUM_STA];
#if !defined(A4_TX_MCAST2UNI) && !defined(CONFIG_TX_BCAST2UNI)
	u8 bc_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif
	int i;
	s32	res;
	enum mc2u_op op;
	int should_go_normal_path = 0;
#ifdef CONFIG_RTW_A4_STA
	u8 is_from_a4_sta = 0;
#endif

	DBG_COUNTER(padapter->tx_logs.os_tx_m2u);

	sa_psta = rtw_get_stainfo(&padapter->stapriv, skb->data+ETH_ALEN);
#ifdef CONFIG_RTW_A4_STA
	if (padapter->a4_enable == 1 && sa_psta == NULL) {
		sa_psta = core_a4_get_fwd_sta(padapter, skb->data+ETH_ALEN);
		if(sa_psta)
			is_from_a4_sta = 1;
	}
#endif

	op = rtw_m2u_check(padapter, skb);
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* free sta asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		int stainfo_offset;
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		if (op == MC2U_ANYWAY) {
			stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
			if (stainfo_offset_valid(stainfo_offset))
				chk_alive_list[chk_alive_num++] = stainfo_offset;
		} else if((IP_MCAST_MAC(skb->data) || ICMPV6_MCAST_MAC(skb->data)) && (op == MC2U_NORMAL)) {
			for (i = 0; i < psta->ipmc_num; i++) {
				if (_rtw_memcmp(psta->ipmc[i].mcmac, skb->data, MAC_ALEN) == _TRUE) {
					stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
					if (stainfo_offset_valid(stainfo_offset))
						chk_alive_list[chk_alive_num++] = stainfo_offset;
					break;
				}
			}
		}
#ifdef A4_TX_MCAST2UNI
		else if((psta->flags & WLAN_STA_A4)){
			stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
			if (stainfo_offset_valid(stainfo_offset))
				chk_alive_list[chk_alive_num++] = stainfo_offset;
		}
#endif
		else
			should_go_normal_path = 1;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	if (chk_alive_num == 0) {
		if(padapter->registrypriv.wifi_mib.mc2u_flood_ctrl)
		{
			return _FALSE;
		}
		goto func_end;
	}

	for (i = 0; i < chk_alive_num; i++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, chk_alive_list[i]);
		if (!(psta->state & WIFI_ASOC_STATE)) {
			DBG_COUNTER(padapter->tx_logs.os_tx_m2u_ignore_fw_linked);
			continue;
		}

		/* avoid come from STA1 and send back STA1 */
		if (psta == sa_psta
			|| _rtw_memcmp(psta->phl_sta->mac_addr, "\x0\x0\x0\x0\x0\x0", ETH_ALEN) == _TRUE
#if !defined(A4_TX_MCAST2UNI) && !defined(CONFIG_TX_BCAST2UNI)
			|| _rtw_memcmp(psta->phl_sta->mac_addr, bc_addr, ETH_ALEN) == _TRUE
#endif
		) {
#ifdef CONFIG_RTW_A4_STA
			if(psta == sa_psta && is_from_a4_sta)
			{
				core_a4_update_m2u_ignore_cnt(padapter, skb->data+ETH_ALEN);
			}
#endif
			DBG_COUNTER(padapter->tx_logs.os_tx_m2u_ignore_self);
			continue;
		}

		DBG_COUNTER(padapter->tx_logs.os_tx_m2u_entry);

		newskb = rtw_skb_copy(skb);

		if (newskb) {
#ifdef A4_TX_MCAST2UNI
			if (padapter->a4_enable == 1 && (psta->flags & WLAN_STA_A4))
				_rtw_memcpy(&newskb->cb[_SKB_CB_MC2U_RA], psta->phl_sta->mac_addr, ETH_ALEN);
			else
#endif
				_rtw_memcpy(newskb->data, psta->phl_sta->mac_addr, ETH_ALEN);

#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
			pstaxmitpriv = &psta->sta_xmitpriv;
			if ((psta->state & WIFI_SLEEP_STATE) || (pstaxmitpriv->tx_pending_bitmap)) {
				res = os_tx_enqueue_for_sleeping_sta(padapter, psta, newskb);
				if (res == PS_TX_ENQ_SUCCESS) {
					DBG_COUNTER(padapter->tx_logs.os_tx_m2u_ps_enqueue);
					continue;
				}else if(res == PS_TX_ENQ_DROP){
					DBG_COUNTER(padapter->tx_logs.os_tx_m2u_ps_drop);
					rtw_skb_free(newskb);
					continue;
				}
			}
#endif
			res = rtw_core_tx(padapter, &newskb, psta);
			if (res < 0) {
				DBG_COUNTER(padapter->tx_logs.os_tx_m2u_entry_err_xmit);
				RTW_INFO("%s()-%d: rtw_xmit() return error! res=%d\n", __FUNCTION__, __LINE__, res);
				pxmitpriv->tx_drop++;
			}
		} else {
			DBG_COUNTER(padapter->tx_logs.os_tx_m2u_entry_err_skb);
			RTW_INFO("%s-%d: rtw_skb_copy() failed!\n", __FUNCTION__, __LINE__);
			pxmitpriv->tx_drop++;
			/* rtw_skb_free(skb); */
			return _FALSE;	/* Caller shall tx this multicast frame via normal way. */
		}
	}

func_end:
	if(should_go_normal_path)
		return _FALSE;

	rtw_skb_free(skb);
	return _TRUE;
}
#endif /* CONFIG_TX_MCAST2UNI */


int _rtw_xmit_entry(struct sk_buff *pkt, _nic_hdl pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
#ifdef CONFIG_TX_MCAST2UNI
	extern int rtw_mc2u_disable;
#endif /* CONFIG_TX_MCAST2UNI	 */
#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	struct sk_buff *skb = pkt;
	struct sk_buff *segs, *nskb;
	netdev_features_t features = padapter->pnetdev->features;
#endif
	s32 res = 0;
	u32 num_txreq = GET_PRIMARY_ADAPTER(padapter)->dvobj->phl_com->dev_id ? MAX_TX_RING_NUM_2G : MAX_TX_RING_NUM_5G;

	if (padapter->registrypriv.mp_mode) {
		RTW_INFO("MP_TX_DROP_OS_FRAME\n");
		goto drop_packet;
	}
	DBG_COUNTER(padapter->tx_logs.os_tx);

	if (rtw_if_up(padapter) == _FALSE) {
		DBG_COUNTER(padapter->tx_logs.os_tx_err_up);
		#ifdef DBG_TX_DROP_FRAME
		RTW_INFO("DBG_TX_DROP_FRAME %s if_up fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	rtw_check_xmit_resource(padapter, pkt);

#ifdef CONFIG_TX_MCAST2UNI
	if (!rtw_mc2u_disable
		&& (!padapter->registrypriv.wifi_mib.mc2u_disable)
		&& MLME_IS_AP(padapter)
		&& (IP_MCAST_MAC(pkt->data)
			|| ICMPV6_MCAST_MAC(pkt->data)
			#ifdef CONFIG_TX_BCAST2UNI
			|| is_broadcast_mac_addr(pkt->data)
			#endif
			#ifdef A4_TX_MCAST2UNI
			|| (padapter->a4_enable && IS_MCAST(pkt->data))
			#endif
			)
		&& (padapter->registrypriv.wifi_spec == 0)
	) {
		if (padapter->pfree_txreq_queue->qlen > (num_txreq / 4)) {
			res = rtw_mlcst2unicst(padapter, pkt);
			if (res == _TRUE)
				goto exit;
		} else {
			/* RTW_INFO("Stop M2U(%d, %d)! ", pxmitpriv->free_xmitframe_cnt, pxmitpriv->free_xmitbuf_cnt); */
			/* RTW_INFO("!m2u ); */
			DBG_COUNTER(padapter->tx_logs.os_tx_m2u_stop);
		}
	}
#endif /* CONFIG_TX_MCAST2UNI	 */

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	if (skb_shinfo(skb)->gso_size) {
	/*	split a big(65k) skb into several small(1.5k) skbs */
		features &= ~(NETIF_F_TSO | NETIF_F_TSO6);
		segs = skb_gso_segment(skb, features);
		if (IS_ERR(segs) || !segs)
			goto drop_packet;

		do {
			nskb = segs;
			segs = segs->next;
			nskb->next = NULL;
			rtw_mstat_update( MSTAT_TYPE_SKB, MSTAT_ALLOC_SUCCESS, nskb->truesize);
			res = rtw_xmit(padapter, &nskb);
			if (res < 0) {
				#ifdef DBG_TX_DROP_FRAME
				RTW_INFO("DBG_TX_DROP_FRAME %s rtw_xmit fail\n", __FUNCTION__);
				#endif
				pxmitpriv->tx_drop++;
				rtw_os_pkt_complete(padapter, nskb);
			}
		} while (segs);
		rtw_os_pkt_complete(padapter, skb);
		goto exit;
	}
#endif

	res = rtw_xmit(padapter, &pkt);
	if (res < 0) {
		#ifdef DBG_TX_DROP_FRAME
		RTW_INFO("DBG_TX_DROP_FRAME %s rtw_xmit fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	goto exit;

drop_packet:
	pxmitpriv->tx_drop++;
	rtw_os_pkt_complete(padapter, pkt);

exit:


	return 0;
}

__IMEM_WLAN_SECTION__
int rtw_xmit_entry(struct sk_buff *pkt, _nic_hdl pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	int ret = 0;

	if (!netif_running(pnetdev))
		return -ENETDOWN;

	if (pkt) {
		if (check_fwstate(pmlmepriv, WIFI_MONITOR_STATE) == _TRUE) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
			rtw_monitor_xmit_entry((struct sk_buff *)pkt, pnetdev);
#endif
		}
		else {
#ifdef CONFIG_RTW_NETIF_SG
			/* After turning on SG, net stack may (0.0025%) TX
			 * strange skb that is skb_has_frag_list() but linear
			 * (i.e. skb_is_nonlinear() is false). This is out of
			 * our expectation, so I free fragment list to be
			 * compatible with our design.
			 */
			if (skb_has_frag_list(pkt)) {
				if (!skb_is_nonlinear(pkt)) {
					kfree_skb_list(skb_shinfo(pkt)->frag_list);
					skb_shinfo(pkt)->frag_list = NULL;
					RTW_DBG("%s:%d free frag list\n", __func__, __LINE__);
				} else {
					RTW_DBG("%s:%d nonlinear frag list\n", __func__, __LINE__);
				}
			}
#endif
			rtw_mstat_update(MSTAT_TYPE_SKB, MSTAT_ALLOC_SUCCESS, pkt->truesize);
			ret = rtw_os_tx(pkt, pnetdev);
		}

	}

	return ret;
}



#ifdef RTW_PHL_TX
inline int rtw_os_is_adapter_ready(_adapter *padapter, struct sk_buff *pkt)
{

	if (padapter->registrypriv.mp_mode) {
		RTW_INFO("MP_TX_DROP_OS_FRAME\n");
		return _FALSE;
	}

	DBG_COUNTER(padapter->tx_logs.os_tx);

	if ((RTW_CANNOT_RUN(adapter_to_dvobj(padapter)) ||
	    (check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE) == _FALSE))) {
		PHLTX_LOG;
		DBG_COUNTER(padapter->tx_logs.os_tx_err_up);
	#ifdef DBG_TX_DROP_FRAME
		RTW_INFO("DBG_TX_DROP_FRAME %s if_up fail\n", __FUNCTION__);
	#endif
		return _FALSE;
	}

	if (IS_CH_WAITING(adapter_to_rfctl(padapter))){
		PHLTX_LOG;
		return _FALSE;
	}

	// duplicate check in rtw_os_tx
	//if (rtw_linked_check(padapter) == _FALSE){
	//	PHLTX_LOG;
	//	return _FALSE;
	//}

	rtw_check_xmit_resource(padapter, pkt);

	return _TRUE;
}

#ifdef CONFIG_TX_MCAST2UNI
int rtw_os_is_process_mc2u(_adapter *padapter, struct sk_buff *pkt)
{
	extern int rtw_mc2u_disable;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	u32 num_txreq = GET_PRIMARY_ADAPTER(padapter)->dvobj->phl_com->dev_id ? MAX_TX_RING_NUM_2G : MAX_TX_RING_NUM_5G;

	if (!rtw_mc2u_disable
		&& (!padapter->registrypriv.wifi_mib.mc2u_disable)
		&& MLME_IS_AP(padapter)
		&& (IP_MCAST_MAC(pkt->data)
			|| ICMPV6_MCAST_MAC(pkt->data)
		#ifdef CONFIG_TX_BCAST2UNI
			|| is_broadcast_mac_addr(pkt->data)
		#endif
		#ifdef A4_TX_MCAST2UNI
			|| (padapter->a4_enable && IS_MCAST(pkt->data))
		#endif
			)
		&& (padapter->registrypriv.wifi_spec == 0))
	{
		if (padapter->pfree_txreq_queue->qlen > (num_txreq / 4))
			return rtw_mlcst2unicst(padapter, pkt); //rtw_phl_tx todo
		return FAIL;
	}

	return _FALSE;
}
#endif /* CONFIG_TX_MCAST2UNI	 */

#ifdef CONFIG_VW_REFINE
void check_vw_data( _adapter *padapter, struct sk_buff *skb)
{
	if (skb->len > 1000)
		padapter->big_pkt++;
	else
		padapter->small_pkt++;

	if ((skb->len == padapter->spec_pktsz - 4) || (skb->len == 1514) ||
		(skb->len == 1510) || (skb->len == 508) || (skb->len == 504) ||
		(skb->len == 84) || (skb->len == 80)) {
		DBG_COUNTER(padapter->tx_logs.core_vw_entry);
		skb->cb[_SKB_VW_FLAG] = 0x10;
	} else
		skb->cb[_SKB_VW_FLAG] = 0;
}

static inline u16 vw_get_q_idx(_adapter *padapter, struct sk_buff *pskb)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;

	u8 *da = pskb->data;
	u16 macid = 0;

	if (IS_MCAST(da)) {
		macid = 0;
	} else {
		psta = rtw_get_stainfo(pstapriv, da);
		macid = psta ? psta->phl_sta->macid : 0;
	}

	return macid % MAX_SKB_XMIT_QUEUE;
}
#endif

static inline struct sta_info *get_txpkt_sta_info(_adapter *padapter, struct sk_buff *pkt)
{
	struct sta_info *psta = NULL;
	u8 *ra;

	if (MLME_IS_STA(padapter)) {
#ifdef CONFIG_TDLS
		if (padapter->tdlsinfo.link_established == _TRUE) {
			psta = rtw_get_stainfo(&padapter->stapriv, pkt->data);
			if (psta && psta->tdls_sta_state & TDLS_LINKED_STATE)
				ra = pkt->data; /* For TDLS direct link Tx, set ra to be same to dst */
			else
				ra = get_bssid(&padapter->mlmepriv);
		} else
#endif
			ra = get_bssid(&padapter->mlmepriv);
	} else
		ra = pkt->data;

	psta = rtw_get_stainfo(&padapter->stapriv, ra);
#ifdef CONFIG_RTW_A4_STA
	if (!psta)
		psta = core_a4_get_fwd_sta(padapter, ra);
#endif

	if (psta && !(psta->state & WIFI_ASOC_STATE)) {
#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
		psta->os_tx_drop_cnt++;
#endif
		psta = NULL;
	}

	return psta;
}

/* refer to IEEE802.11-2016 Table R-3; Comply with IETF RFC4594 */
static inline u8 tos_to_up(_adapter *padapter, u8 tos, char skb_cb0)
{
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	u8 up = 0;
	u8 dscp = (tos >> 2);
	u8 mode = padapter->registrypriv.wifi_mib.pri_mapping_rule;
#ifdef CONFIG_RTW_MULTI_AP_R3
	_adapter * primary_adapter;
#endif
#ifdef CONFIG_FORCE_QOS_SUPPORT
	if (padapter->registrypriv.wifi_mib.force_qos
			&& (skb_cb0 > 0) && (skb_cb0 < 8)) {
		up = skb_cb0;
		goto hw_seq_remap;
	}
#endif

	if (padapter->dscp_mapping_enable) {
		if (dscp < DSCP_TABLE_SIZE)
			up = padapter->dscp_mapping_table[dscp];
	}

	/* tos precedence mapping */
#ifdef CTC_QOS_DSCP
	else if (pregistrypriv->ctc_dscp) {
		if ( dscp >= 0 && dscp <= 7 )
			up = 0;
		else if ( dscp >= 8 && dscp <= 15)
			up = 1;
		else if ( dscp >= 16 && dscp <= 23)
			up = 2;
		else if ( dscp >= 24 && dscp <= 31)
			up = 3;
		else if ( dscp >= 32 && dscp <= 39)
			up = 4;
		else if ( dscp >= 40 && dscp <= 47)
			up = 5;
		else if ( dscp >= 48 && dscp <= 55)
			up = 6;
		else if ( dscp >= 56 && dscp <= 63)
			up = 7;

		/* not use HW SEQ */
		goto tos_to_up_end;
	}
#endif
	else if (mode == 0) {
		up = tos >> 5;
	} else {
		/* refer to IEEE802.11-2016 Table R-3;
		 * DCSP 32(CS4) comply with IETF RFC4594
		 */
		if ( dscp == 0 )
			up = 0;
		else if ( dscp >= 1 && dscp <= 9)
			up = 1;
		else if ( dscp >= 10 && dscp <= 16)
			up = 2;
		else if ( dscp >= 17 && dscp <= 23)
			up = 3;
		else if ( dscp >= 24 && dscp <= 31)
			up = 4;
		else if ( dscp >= 33 && dscp <= 40)
			up = 5;
		else if ((dscp >= 41 && dscp <= 47) || (dscp == 32))
			up = 6;
		else if ( dscp >= 48 && dscp <= 63)
			up = 7;
	}

#ifdef CONFIG_RTW_SSID_PRIORITY
	if(pregistrypriv->wifi_mib.manual_priority != 0xff)
		up = pregistrypriv->wifi_mib.manual_priority;
#endif

hw_seq_remap:
#ifdef CONFIG_RTW_TXSC_USE_HW_SEQ
	// Freddie ToDo: HW SEQ EN policy. Temporarily map TID to 4 Q and use QSEL as HWSEQ index.
	do {
		static const u32 tid_remap[8] = {0, 1, 1, 0, 4, 4, 6, 6};
		up = tid_remap[up];
	} while (0);
#endif /* CONFIG_RTW_TXSC_USE_HW_SEQ */

#ifdef CONFIG_RTW_MULTI_AP_R3 // if service prioritization is needed and out_value is 8, just set up based on dscp_pcp_table.
	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);
	if (primary_adapter->service_priority_enabled == 1 && primary_adapter->service_priority_output == 8 && primary_adapter->dscp_pcp_table_enabled == 1) {
		up = *((primary_adapter->dscp_pcp_table) + dscp);
	}
#endif

tos_to_up_end:
	return up;
}

__DMEM_SECTION__ u8 pri2txqidx[] = { TXQ_BE, TXQ_BK, TXQ_BK, TXQ_BE, TXQ_VI, TXQ_VI, TXQ_VO, TXQ_VO};

__IMEM_WLAN_SECTION__
int rtw_os_tx(struct sk_buff *pkt, _nic_hdl pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	s32 res = 0;
	int is_uc_ip = 0;
	u8 i = 0;
	#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
	struct sta_xmit_priv *pstaxmitpriv = NULL;
	#endif
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
	struct dvobj_priv *dvobj;
	u8 q_idx;

#ifdef CONFIG_VW_REFINE
	bool enq_status;
#endif /* CONFIG_VW_REFINE */

#ifdef CONFIG_ONE_TXQ
	struct sta_tx_queue *txq;
	u32 skb_qlen = 0;
#endif /* CONFIG_ONE_TXQ */
#endif /* defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ) */

	s32 ret;

//#if !defined(CONFIG_ONE_TXQ)
	struct sta_info *psta = NULL;
	u16 protocol;
	u8 priority;
	u8 rtw_linked_chk = 0;
//#endif

#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT
	extern int rtl_vlan_passthrough_enable;
#endif
#ifdef CONFIG_ETHER_PKT_AGG
	struct sk_buff *pskb_next;
#endif

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
		if(MLME_IS_STA(padapter) && !is_primary_adapter(padapter))
		{
			if(!is_bmc(pkt->data))
			{
				if(rtw_crossband_tx_check(padapter, pkt))
				{
#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
					padapter->recvpriv.cb_pathswitch_pkts++;
					return 0;
#else
					padapter = padapter->crossband.crossband_vxd_sc;
					pkt->dev = padapter->pnetdev;
					pxmitpriv = &padapter->xmitpriv;
					padapter->recvpriv.cb_pathswitch_pkts++;
#endif
				}
			}
		}
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
	if (linux_vlan_enable && rtw_linux_vlan_tx_process(padapter, &pkt) == FAIL)
		goto drop_packet;
#endif

#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT
	if (rtl_vlan_passthrough_enable && rtw_vlan_passthrough_tx_process(&pkt) == FAIL)
		goto drop_packet;
#endif

#ifdef RTW_PHL_DBG_CMD
	core_add_record(padapter, padapter->record_enable, REC_TX_DATA, pkt);
#endif

	PHLTX_LOG;

	pxmitpriv->os_tx_pkts++;

	if (pkt->len == 0)
		goto drop_packet;

	if (rtw_os_is_adapter_ready(padapter, pkt) == _FALSE)
		goto drop_packet;

	/* do not count non-assoc drop to tx_drop counter */
	if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)
			|| MLME_IS_ADHOC(padapter) || MLME_IS_ADHOC_MASTER(padapter)
	) {
		if (padapter->stapriv.asoc_sta_count > 1)
			rtw_linked_chk = 1;
	} else {
		/* Station mode */
		if (check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE) == _TRUE)
			rtw_linked_chk = 1;
	}

	//	if (rtw_linked_check(padapter) == _FALSE){
	if (rtw_linked_chk == 0) {
		pxmitpriv->tx_drop_noasoc++;
		goto drop_packet_no_count;
	}

	if (padapter->registrypriv.wifi_mib.func_off)
		goto drop_packet;

	PHLTX_LOG;

#ifdef SBWC
	ret = sbwc_tx(padapter, pkt);
	if(ret == SBWC_FREE_SKB){
		goto drop_packet;
	}else if(ret == SBWC_QUEUE_SKB){
		return 0;
	}
#endif

#ifdef RTW_STA_BWC
	ret = sta_bwc_tx(padapter, pkt);
	if(ret == STA_BWC_FREE_SKB){
		goto drop_packet;
	}else if(ret == STA_BWC_QUEUE_SKB){
		return 0;
	}
#endif

	psta = get_txpkt_sta_info(padapter, pkt);
	if (!psta) {
		if (MLME_IS_AP(padapter) && is_dhcp_pkt(pkt->data)) {
			for (i = 0; i < 6; i++) {
				pkt->data[i] = 0xFF;
			}

			psta = get_txpkt_sta_info(padapter, pkt);
			if (!psta) {
				goto drop_packet;
			}
		} else {
			goto drop_packet;
		}
	}

	/* in some environment, root_ap -> vxd, skb_cb_flags is not zero, so reset it */
	pkt->cb[_SKB_CB_FLAGS] = 0x0;

	RTW_PKT_MIRROR_DUMP(padapter, pkt, true);

#ifdef CONFIG_ONE_TXQ
	/* initialize cb[_SKB_CB_AIRTIME] before handling skb */
	*(u32 *)&pkt->cb[_SKB_CB_AIRTIME] = 0;
#endif

#ifdef CONFIG_ETHER_PKT_AGG
	if (pkt->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_MULTI_PKT
			|| pkt->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_PKTLIST) { // ethernet agg packet
		protocol = *(u16 *)(pkt->data + ETH_ALEN * 2);
		priority = pkt->cb[_SKB_CB_PRIORITY];
		is_uc_ip = 1;
		//DBG_COUNTER(padapter->tx_logs.os_uc_ip_pri[priority]);
	} else
#endif /* CONFIG_ETHER_PKT_AGG */
	{
		protocol = *(u16 *)(pkt->data + ETH_ALEN * 2);
		if ((MLME_IS_STA(padapter) || !IS_MCAST(pkt->data))
			&& protocol == __constant_htons(ETH_P_IP)) {
			priority = tos_to_up(padapter, *(pkt->data + ETH_HLEN + 1), pkt->cb[0]);
			is_uc_ip = 1;
			DBG_COUNTER(padapter->tx_logs.os_uc_ip_pri[priority]);
		} else
			priority = 0;
	}
#ifdef CONFIG_RTW_MULTI_AP_R3
	if (protocol == __constant_htons(ETH_P_8021Q)) {
		priority = tos_to_up(padapter, *(pkt->data + ETH_HLEN + 4 + 1), pkt->cb[0]);
		*(pkt->data + ETH_ALEN * 2 + 2) |= priority << 5;
	}
#endif
	/* TX EAPOL/ARP/1905 in VO priority */
	if (protocol == __constant_htons(ETH_P_PAE) ||
		protocol == __constant_htons(ETH_P_ARP) ||
		protocol == __constant_htons(ETH_P_1905)) {
		priority = 6;
	}

	pkt->cb[_SKB_CB_PRIORITY] = priority;
	pkt->cb[_SKB_CB_QNUM] = pri2txqidx[priority];
	*(u16 *)(pkt->cb + _SKB_CB_ETH_PROTOCOL) = __constant_htons(protocol);

	if(is_icmpv6_pkt(pkt->data))
	{
		pkt->cb[_SKB_CB_FLAGS] = _PKT_TYPE_URGENT;
	}

	/* RESERVE_TXREQ */
	if (protocol == __constant_htons(ETH_P_PAE) || protocol == __constant_htons(ETH_P_ARP) ||
		protocol == __constant_htons(ETH_P_1905) || is_dhcp_pkt(pkt->data)){
		pkt->cb[_SKB_CB_FLAGS] = _PKT_TYPE_URGENT;
	}

#ifdef A4_TX_MCAST2UNI
	if (padapter->a4_enable)
		_rtw_memset(&pkt->cb[_SKB_CB_MC2U_RA], 0, ETH_ALEN);
#endif


	if(IS_MCAST(pkt->data)) {
		DBG_COUNTER(padapter->tx_logs.os_tx_bmc);
		#ifdef CONFIG_TX_MCAST2UNI
		ret = rtw_os_is_process_mc2u(padapter, pkt);
		if(ret == _TRUE) {
			goto exit;
		}
		else if(ret == FAIL){
			goto drop_packet;
		}
		#endif
	} else {
		DBG_COUNTER(padapter->tx_logs.os_tx_uc);
	}

	PHLTX_LOG;

#ifdef CONFIG_ADPTVTY_CONTROL
	if (padapter->registrypriv.wifi_mib.adptvty_en)
	{
		if(is_tcp_ip(pkt->data))
			DBG_COUNTER(padapter->tx_logs.os_tx_tcp);
		else if(is_udp_ip(pkt->data))
			DBG_COUNTER(padapter->tx_logs.os_tx_udp);
		else
			;
	}
#endif /* CONFIG_ADPTVTY_CONTROL */
	DBG_COUNTER(padapter->tx_logs.os_tx_normal);

#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
	dvobj = padapter->dvobj;
#endif

	//if (dvobj->phl_com->dev_cap.fw_tx_mode == 1) {
	if (dvobj->phl_com->dev_cap.fw_tx_mode == 1 && !padapter->registrypriv.wifi_mib.ofdma_enable) {
		if (is_udp_ip(pkt->data) && adapter_to_dvobj(padapter)->force_tx_cmd_num){
			rtw_phl_set_fw_txcmd_num (adapter_to_dvobj(padapter)->phl, 0 );
			adapter_to_dvobj(padapter)->force_tx_cmd_num = 0;
		} else if (is_tcp_ip(pkt->data) && !adapter_to_dvobj(padapter)->force_tx_cmd_num){
			rtw_phl_set_fw_txcmd_num (adapter_to_dvobj(padapter)->phl, 1 );
			adapter_to_dvobj(padapter)->force_tx_cmd_num = 1;
		}
	}

#ifdef CONFIG_VW_REFINE
	if (dvobj->tx_mode == 1) {
		protocol = *((u16*)(pkt->data + ETH_ALEN * 2));
		if ( is_bmc(pkt->data) || (!is_udp_ip(pkt->data) && !is_tcp_ip(pkt->data)) )
			pkt->cb[_SKB_CB_FLAGS] = _PKT_TYPE_URGENT;
		else if ( is_tcp_ip_ack(pkt->data) )
			pkt->cb[_SKB_CB_FLAGS] = _PKT_TYPE_TCPACK;
		else if ((protocol == __constant_htons(0x888e))
#if defined(CONFIG_RTL_WAPI_SUPPORT) || defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
			|| (protocol == __constant_htons(ETH_P_WAPI))
#endif
			)
			pkt->cb[_SKB_CB_FLAGS] = _PKT_TYPE_URGENT;
		else if ( is_dhcp_pkt(pkt->data) )
			pkt->cb[_SKB_CB_FLAGS] = (_PKT_TYPE_URGENT | _PKT_TYPE_DHCP);
		else
			pkt->cb[_SKB_CB_FLAGS] = 0;

		if (pkt->cb[_SKB_CB_FLAGS] & _PKT_TYPE_URGENT)
			goto direct_xmit;

		check_vw_data(padapter, pkt);
		if (padapter->direct_xmit)
			goto direct_xmit;

		q_idx = vw_get_q_idx(padapter, pkt);

		enq_status = vw_skb_enq(padapter, q_idx, pkt);
		if (_SUCCESS == enq_status) {
			DBG_COUNTER(padapter->tx_logs.core_vw_swq_enq);
			goto exit;
		} else {
			DBG_COUNTER(padapter->tx_logs.core_vw_swq_enq_fail);
			goto drop_packet;
		}
	}
#endif /* CONFIG_VW_REFINE */

#ifdef CONFIG_ONE_TXQ
	if (dvobj->tx_mode == 2) {
		pkt->cb[_SKB_CB_FLAGS] = check_pkt_type(pkt);
		if (pkt->cb[_SKB_CB_FLAGS] & _PKT_TYPE_URGENT)
			goto direct_xmit;

		if ((psta != padapter->self_sta)) {
			u32 txq_limit=0;
			if (psta->sta_xmitpriv.txq_limit)
				txq_limit = psta->sta_xmitpriv.txq_limit;
			else
				txq_limit = padapter->registrypriv.wifi_mib.txq_limit;
			if (ATOMIC_READ(&psta->sta_xmitpriv.txq_total_len) > txq_limit) {
				psta->sta_xmitpriv.txq_full_drop++;
				goto drop_packet;
			}
		}

		skb_qlen = ATOMIC_READ(&dvobj->txq_total_len);
		if (skb_qlen > dvobj->txq_max_enq_len) {
			if (dvobj->txq_debug)
				printk_ratelimited(KERN_WARNING "TX DROP: exceed skb Q!(seq %04x txq %d %d)\n",
					dvobj->txq_timeout_seq & 0xffff, skb_qlen, dvobj->txq_max_enq_len);
			dvobj->txq_full_drop++;
			psta->sta_xmitpriv.txq_full_drop++;
			goto drop_packet;
		}

		*(systime *)&pkt->cb[_SKB_CB_TIME] = rtw_get_current_time()
			+ rtw_ms_to_systime(dvobj->txq_pkt_timeout);

		q_idx = rtw_get_txq_idx(priority);
		pkt->cb[_SKB_CB_QNUM] = q_idx;

		pstaxmitpriv = &psta->sta_xmitpriv;
		txq = &pstaxmitpriv->txq[q_idx];

		_rtw_spinlock_bh(&txq->qhead.lock);

		__skb_queue_tail(&txq->qhead, pkt);
		if (txq->qhead.qlen == 1) {
			txq->timeout = *(systime *)&pkt->cb[_SKB_CB_TIME];
			rtw_set_bit(q_idx, &pstaxmitpriv->tx_pending_bitmap);
			if (!(psta->state & WIFI_SLEEP_STATE)) {
				_queue *sta_queue;
				sta_queue = &dvobj->tx_pending_sta_queue;

				_rtw_spinlock_bh(&sta_queue->lock);
				if (rtw_is_list_empty(&pstaxmitpriv->tx_pending) == _TRUE) {
					rtw_list_insert_tail(&pstaxmitpriv->tx_pending, &sta_queue->queue);
					sta_queue->qlen++;
				}
				_rtw_spinunlock_bh(&sta_queue->lock);
			}
		}

		_rtw_spinunlock_bh(&txq->qhead.lock);

		ATOMIC_INC(&pstaxmitpriv->txq_total_len);
		ATOMIC_INC(&dvobj->txq_total_len);

#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
		if (psta->state & WIFI_SLEEP_STATE) {
#ifdef CONFIG_TDLS
			if (padapter->tdlsinfo.link_established == _TRUE
				&& (psta->tdls_sta_state & TDLS_LINKED_STATE)) {
				/* Transmit TDLS PTI via AP */
				if (ATOMIC_READ(&pstaxmitpriv->txq_total_len) == 1)
					rtw_tdls_cmd(padapter, psta->phl_sta->mac_addr, TDLS_ISSUE_PTI);
			} else
#endif
			if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) {
				if ((psta->uapsd_bitmap == (BIT(TXQ_VO)|BIT(TXQ_VI)|BIT(TXQ_BE)|BIT(TXQ_BK))
						&& pstaxmitpriv->tx_pending_bitmap)
						|| (pstaxmitpriv->tx_pending_bitmap & ~ psta->uapsd_bitmap)) {
					struct sta_priv *pstapriv = &padapter->stapriv;
					if (!rtw_tim_map_is_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid)) {
						rtw_tim_map_set(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
						update_beacon(padapter, _TIM_IE_, NULL, _TRUE, 0);
					}
				}
			}
		}
#endif

		goto exit;
	}
#endif /* CONFIG_ONE_TXQ */

#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
	pstaxmitpriv = &psta->sta_xmitpriv;
	if((psta->state & WIFI_SLEEP_STATE) || (pstaxmitpriv->tx_pending_bitmap)) {
		ret = os_tx_enqueue_for_sleeping_sta(padapter, psta, pkt);
		if (ret == PS_TX_ENQ_SUCCESS) {
			DBG_COUNTER(padapter->tx_logs.core_tx_ps_enqueue);
			return 0;
		}else if(ret == PS_TX_ENQ_DROP){
			DBG_COUNTER(padapter->tx_logs.core_tx_ps_drop);
			goto drop_packet;
		}
	}
#endif
//#endif /* !CONFIG_ONE_TXQ */

#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
direct_xmit:
#endif

#ifdef CONFIG_RTW_A4_STA
	/* A4_CNT */
	if (adapter_en_a4(padapter) && (psta->flags & WLAN_STA_A4)) {
		core_a4_count_stats(padapter, pkt->data, 0, pkt->len);
	}
#endif

#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
	psta->os_tx_cnt++;
#endif

	if (rtw_core_tx(padapter, &pkt, psta) == FAIL) {
		if (is_uc_ip)
			DBG_COUNTER(padapter->tx_logs.os_uc_ip_pri_drop[priority]);
		pxmitpriv->tx_drop++;
		pxmitpriv->os_tx_drop++;
#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
		psta->os_tx_drop_cnt++;
#endif
		goto exit;
	}

	PHLTX_LOG;

	goto exit;

drop_packet:
	#ifdef CONFIG_ETHER_PKT_AGG
	if (pkt && pkt->cb[_SKB_CB_ETH_AGG] == _PKT_TYPE_AGG_PKTLIST) {
		struct sk_buff *pskb_next;
		do {
			pskb_next = pkt->next;
			rtw_os_pkt_complete(padapter, pkt);
			pkt = pskb_next;
			pxmitpriv->tx_drop++;
			pxmitpriv->os_tx_drop++;
			DBG_COUNTER(padapter->tx_logs.os_tx_drop);
		} while(pkt);

		goto exit;
	}
	#endif /* CONFIG_ETHER_PKT_AGG */
	pxmitpriv->tx_drop++;
	pxmitpriv->os_tx_drop++;

drop_packet_no_count:
	DBG_COUNTER(padapter->tx_logs.os_tx_drop);
	rtw_os_pkt_complete(padapter, pkt);

exit:
	return 0;
}
#endif

#if defined(CONFIG_TX_AMSDU_SW_MODE)
void ieee8023_header_to_rfc1042(struct sk_buff *skb, int pads)
{
	void *data;
	int pad;
	__be16 len;
	const int headroom = SNAP_SIZE + 2 + pads;

	if (!skb)
		return;

	if (skb_headroom(skb) < headroom) {
		RTW_WARN("%s: headroom=%d isn't enough\n", __func__, skb_headroom(skb));
		if (pskb_expand_head(skb, headroom, 0, GFP_ATOMIC)) {
			RTW_ERR("%s: no headroom=%d for skb\n",
				__func__, headroom);
			return;
		}
	}

	data = skb_push(skb, headroom);
	memset(data, 0, pads);
	data += pads;
	memmove(data, data + SNAP_SIZE + 2, 2 * ETH_ALEN);
	data += 2 * ETH_ALEN;
	len = cpu_to_be16(skb->len - pads - 2 * ETH_ALEN - 2);
	memcpy(data, &len, 2);
	memcpy(data + 2, rtw_rfc1042_header, SNAP_SIZE);
}

void rtw_coalesce_tx_amsdu(_adapter *padapter, struct xmit_frame *pxframes[],
			   int xf_nr, bool amsdu, u32 *pktlen)
{
	struct xmit_frame *head_xframe;
	struct xmit_frame *pxframe;
	struct sk_buff *skb;
	struct sk_buff *head_skb;
	struct sk_buff **frag_tail;
	int pads;
	int i;

	/* prepare head xmitframe */
	head_xframe = pxframes[0];
	head_skb = head_xframe->pkt;

	ieee8023_header_to_rfc1042(head_skb, 0);

	frag_tail = &skb_shinfo(head_skb)->frag_list;
	while (*frag_tail)
		frag_tail = &(*frag_tail)->next;

	for (i = 1; i < xf_nr; i++) {
		pxframe = pxframes[i];
		skb = pxframe->pkt;

		if (head_skb->len & 0x03)
			pads = 4 - (head_skb->len & 0x03);
		else
			pads = 0;

		ieee8023_header_to_rfc1042(skb, pads);

		/* free sk accounting to have TP like doing skb_linearize() */
		if (skb->destructor)
			skb_orphan(skb);

		/* add this skb to head_skb */
		head_skb->len += skb->len;
		head_skb->data_len += skb->len;
		*frag_tail = skb;
		while (*frag_tail)
			frag_tail = &(*frag_tail)->next;

		/* free this xframe */
		pxframe->pkt = NULL; /* head xframe own */
		core_tx_free_xmitframe(padapter, pxframe);
	}

	/* total skb length (includes all fragments) */
	*pktlen = head_skb->len;
}
#endif /* CONFIG_TX_AMSDU_SW_MODE */

#ifdef CONFIG_RTL_VLAN_8021Q
int rtw_linux_vlan_tx_process(_adapter *padapter, struct sk_buff **ppskb)
{
	u16 vid = 0;
	struct sk_buff *pskb = NULL, *pskb2 = NULL;
	struct net_device *ndev = NULL;

	if (!padapter || !ppskb) {
		RTW_ERR("[%s %d] NULL pointer! padapter: %p ppskb: %p\n",
			__FUNCTION__, __LINE__, padapter, ppskb);
		return FAIL;
	}

	pskb = *ppskb;
	ndev = padapter->pnetdev;

	if (!pskb || !ndev) {
		RTW_ERR("[%s %d] NULL pointer! pskb: %p ndev: %p\n",
			__FUNCTION__, __LINE__, pskb, ndev);
		return FAIL;
	}

	/* remove vlan tag if no need to tagged out */
	if (PKT_ETH_TYPE(pskb) == htons(ETH_P_8021Q)) {
		vid = ntohs(*(u16*)(pskb->data+(ETH_ALEN<<1) + 2)) & 0x0fff;
#ifndef CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG
		/* In source-tag mode, wlan tx doesn't support tagged. */
		if (!(vlan_ctl_p->group[vid].tagMemberMask & (1<<ndev->vlan_member_map)))
#endif
		{
			if (skb_cloned(pskb)) {
				pskb2 = skb_copy(pskb, GFP_ATOMIC);
				if (pskb2 == NULL) {
					RTW_ERR("[%s %d] Drop pkt due to skb_copy() failed!\n",
						__FUNCTION__, __LINE__);
					return FAIL;
				}

				rtw_skb_free(pskb);
				pskb = pskb2;
				*ppskb = pskb;
			}

			memmove(pskb->data + VLAN_HLEN, pskb->data, ETH_ALEN<<1);
			skb_pull(pskb, VLAN_HLEN);
		}
	}

	return SUCCESS;
}
#endif

#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT
extern int rtl_process_vlan_passthrough_tx(struct sk_buff **ppskb);
int rtw_vlan_passthrough_tx_process(struct sk_buff **ppskb)
{
	struct sk_buff *pskb = NULL;

	if (!ppskb) {
		RTW_ERR("[%s %d] ppskb is NULL!\n", __FUNCTION__, __LINE__);
		return FAIL;
	}

	pskb = *ppskb;
	if (!pskb) {
		RTW_ERR("[%s %d] pskb is NULL!\n", __FUNCTION__, __LINE__);
		return FAIL;
	}

	if (PKT_ETH_TYPE(pskb) != htons(ETH_P_8021Q)) {
		if(rtl_process_vlan_passthrough_tx(&pskb) == FAIL)
			return FAIL;
	}
	*ppskb = pskb;

	return SUCCESS;
}
#endif
