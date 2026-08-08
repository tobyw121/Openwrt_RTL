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
#ifndef __XMIT_OSDEP_H_
#define __XMIT_OSDEP_H_


struct pkt_file {
	struct sk_buff *pkt;
	SIZE_T pkt_len;	 /* the remainder length of the open_file */
	_buffer *cur_buffer;
	u8 *buf_start;
	u8 *cur_addr;
	SIZE_T buf_len;
};

#if defined(CONFIG_WFA_OFDMA_Logo_Test) || defined(CONFIG_DYN_ALLOC_XMITFRAME)
#define NR_XMITFRAME_2G	MAX_TX_RING_NUM_2G // MAX(MAX_TX_RING_NUM_5G, MAX_TX_RING_NUM_2G)
#define NR_XMITFRAME_5G	MAX_TX_RING_NUM_5G // MAX(MAX_TX_RING_NUM_5G, MAX_TX_RING_NUM_2G)
#else
#define NR_XMITFRAME_2G	512 // MAX(MAX_TX_RING_NUM_5G, MAX_TX_RING_NUM_2G)
#define NR_XMITFRAME_5G	512 // MAX(MAX_TX_RING_NUM_5G, MAX_TX_RING_NUM_2G)
#endif

#define NR_XMITFRAME_CMD	32
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
#define NR_XMITFRAME_EXT 	128
#else
#define NR_XMITFRAME_EXT	32
#endif
#define SZ_XMITFRAME_EXT	1536	/*MGNT frame*/

#ifdef CONFIG_PCI_HCI
	#define SZ_ALIGN_XMITFRAME_EXT	4
#else
	#ifdef USB_XMITBUF_ALIGN_SZ
		#define SZ_ALIGN_XMITFRAME_EXT (USB_XMITBUF_ALIGN_SZ)
	#else
		#define SZ_ALIGN_XMITFRAME_EXT 512
	#endif
#endif


struct xmit_priv;
struct pkt_attrib;
struct sta_xmit_priv;
struct xmit_frame;
struct xmit_buf;


#ifdef PLATFORM_FREEBSD
extern int rtw_xmit_entry(struct sk_buff *pkt, _nic_hdl pnetdev);
extern void rtw_xmit_entry_wrap(_nic_hdl pifp);
#endif /* PLATFORM_FREEBSD */

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ECOS)
extern int _rtw_xmit_entry(struct sk_buff *pkt, _nic_hdl pnetdev);
extern int rtw_xmit_entry(struct sk_buff *pkt, _nic_hdl pnetdev);

#if defined(CONFIG_AP_MODE) || defined(CONFIG_TDLS)
extern sint mgnt_tx_enqueue_for_sleeping_sta(_adapter *padapter, struct xmit_frame *pmgntframe);
#endif

//#ifdef CONFIG_VW_REFINE
#define _RTW_QUE_FULL   3
#define _RTW_QUE_EMPTY  4
//#endif

//#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
enum _SKB_CB {
	/* TX used */
	_SKB_CB_PRIORITY = 1,
	_SKB_CB_QNUM = 2,
	_SKB_CB_FLAGS = 3,
	_SKB_CB_AIRTIME = 4,	/* occupy 4 bytes */
	_SKB_CB_TIME = 8,	/* 8bytes for 64bit platform */
	/* A4_TX_MCAST2UNI */
	_SKB_CB_MC2U_RA = 16,	/* 6 bytes */
	_SKB_CB_BWC_ = 22, /* SBWC & GBWC, 8 bytes for 64 bit platform */
	/*for guest access*/
	_SKB_CB_GUEST_ACCESS = 30,
#ifdef CONFIG_ETHER_PKT_AGG
	_SKB_CB_ETH_AGG = RTK_NIC_WIFI_CB_NUMBER, //32
	_SKB_CB_ETH_AGG_2 = RTK_NIC_WIFI_CB_NUMBER+1, //33
#endif /* CONFIG_ETHER_PKT_AGG */
	_SKB_CB_ETH_PROTOCOL = 34,	/* 2 bytes */
	_SKB_CB_ETH_PROTOCOL_2 = _SKB_CB_ETH_PROTOCOL+1,//35
	/* CONFIG_VW_REFINE */
	_SKB_VW_LAST = 45,
	_SKB_VW_FLAG = 46,
	_SKB_CB_AMSDU_TXSC = 47,

	/* RX used */
	_SKB_CB_IFACE_ID = 47
};

#ifdef CONFIG_ETHER_PKT_AGG
#define _PKT_TYPE_AGG_MULTI_PKT		AGGQ_PKT_BUFF_MODE //0x6A
#define _PKT_TYPE_AGG_PKTLIST		AGGQ_PKT_LIST_MODE //0x5A
//#define _PKT_TYPE_AGG_SINGLE_PKT	0xDA
#define _AGG_TYPE_MULTI_PKT 		1
#define _AGG_TYPE_PKTLIST 			2
#endif /* CONFIG_ETHER_PKT_AGG */

#define _PKT_TYPE_DHCP      BIT0
#define _PKT_TYPE_URGENT    BIT1
#define _PKT_TYPE_TCPACK    BIT2

#ifdef RTW_PHL_TX
extern int rtw_os_tx(struct sk_buff *pkt, _nic_hdl pnetdev);
#endif
#endif /* PLATFORM_LINUX */

void rtw_os_xmit_schedule(_adapter *padapter);

#if 0 /*CONFIG_CORE_XMITBUF*/
int rtw_os_xmit_resource_alloc(_adapter *padapter, struct xmit_buf *pxmitbuf, u32 alloc_sz, u8 flag);
void rtw_os_xmit_resource_free(_adapter *padapter, struct xmit_buf *pxmitbuf, u32 free_sz, u8 flag);
#else
u8 rtw_os_xmit_resource_alloc(_adapter *padapter, struct xmit_frame *pxframe);
void rtw_os_xmit_resource_free(_adapter *padapter, struct xmit_frame *pxframe);
#endif
extern void rtw_set_tx_chksum_offload(struct sk_buff *pkt, struct pkt_attrib *pattrib);

extern uint rtw_remainder_len(struct pkt_file *pfile);
extern void _rtw_open_pktfile(struct sk_buff *pkt, struct pkt_file *pfile);
extern uint _rtw_pktfile_read(struct pkt_file *pfile, u8 *rmem, uint rlen);
extern sint rtw_endofpktfile(struct pkt_file *pfile);

extern void rtw_os_pkt_complete(_adapter *padapter, struct sk_buff *pkt);
extern void rtw_os_xmit_complete(_adapter *padapter, struct xmit_frame *pxframe);

void rtw_os_wake_queue_at_free_stainfo(_adapter *padapter, int *qcnt_freed);

void dump_os_queue(void *sel, _adapter *padapter);

void rtw_coalesce_tx_amsdu(_adapter *padapter, struct xmit_frame *pxframes[],
			   int xf_nr, bool amsdu, u32 *pktlen);


#if defined(CONFIG_TX_AMSDU_SW_MODE)
void ieee8023_header_to_rfc1042(struct sk_buff *skb, int pads);
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
int rtw_linux_vlan_tx_process(_adapter *padapter, struct sk_buff **ppskb);
#endif

#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT
int rtw_vlan_passthrough_tx_process(struct sk_buff **ppskb);
#endif

#ifdef CONFIG_VW_REFINE
void rtl8192cd_swq_timeout_g6(unsigned long data);
struct sk_buff *vw_skb_deq(_adapter *padapter, u8 cur);
bool vw_skb_enq(_adapter *padapter, u8 cur, struct sk_buff *pskb);
u16 vw_skb_len(_adapter *padapter, u8 cur);
#endif

#endif /* __XMIT_OSDEP_H_ */
