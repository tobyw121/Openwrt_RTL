#ifndef __RTW_PKT_TRACE_H__
#define __RTW_PKT_TRACE_H__

enum rtw_pkt_type {
	RTW_PKT_NONE=0,
	RTW_PKT_WIFI_MGT,
	RTW_PKT_WIFI_DATA,
	RTW_PKT_WIFI_CTRL,
	RTW_PKT_LLC,
	RTW_PKT_ETH,
	RTW_PKT_ARP,
	RTW_PKT_IP,
	RTW_PKT_IPV6,
	RTW_PKT_PPPOE_D,
	RTW_PKT_PPPOE_C,
	RTW_PKT_EAPOL,
	RTW_PKT_UDP,
	RTW_PKT_TCP,
	RTW_PKT_DHCP,
	RTW_PKT_DNS,
	RTW_PKT_PING,
	RTW_PKT_HTTP,
	RTW_PKT_TELNET,
	RTW_PKT_MAX,
};

enum rtw_pkt_arp_type {
	RTW_PKT_ARP_REQ,
	RTW_PKT_ARP_RPLY,
};

enum rtw_pkt_dhcp_type {
	RTW_PKT_DHCP_DISCOVER,
	RTW_PKT_DHCP_OFFER,
	RTW_PKT_DHCP_REQUEST,
	RTW_PKT_DHCP_DECLINE,
	RTW_PKT_DHCP_ACK,
};

enum rtw_pkt_dns_type {
	RTW_PKT_DNS_REQ,
	RTW_PKT_DNS_RPLY,
};

enum rtw_pkt_ping_type {
	RTW_PKT_PING_REQ,
	RTW_PKT_PING_RPLY,
};

enum rtw_pkt_dump_level {
	RTW_PKT_DUMP_NONE,
	RTW_PKT_DUMP_HEADER,
	RTW_PKT_DUMP_ALL,
};

struct rtw_eth_type_desc {
	char name[32];
	unsigned short eth_type; /*host endian*/
	enum rtw_pkt_type pkt_type;
};


struct rtw_pkt_type_desc {
	char name[32];
 	enum rtw_pkt_type pkt_type;
};

struct rtw_filter_opt {
	unsigned int filter_flag;
	char opt_name[16];
};

enum rtw_pkt_trace_dir {
	PKT_TX,
	PKT_RX
};

struct rtw_pkt_trace_info {
	u8 *ptr;
	#if defined(CONFIG_RTW_HW_RX_AMSDU_CUT) || defined(WFO_STRUCT_ALIGNED)
	u8 *hdr;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 hdrlen;
	u8 llc_offset;
	u16 pkt_len;
	u8 pktype;
	u8 subtype;
	u8 parsed;
	u8 dir; /* 0: TX, 1: RX */
	u8 *dmac;
	u8 *smac;
	u16 eth_type;
	u8	ip_protocol;
	u8	wl_type;
	u8  wl_subtype;
	u32 sip;
	u32 dip;
	u16 sport;
	u16 dport;
	u16 ipid;
	u32 transaction_id;
};

struct rtw_pkt_filter {
	unsigned int  used;
	unsigned int  match_flag;
	unsigned char wl_type;
	unsigned char wl_subtype;
	unsigned char pktype;
	unsigned char smac[ETH_ALEN];
	unsigned char dmac[ETH_ALEN];
	unsigned int  sip;
	unsigned int  dip;
	/*IPv6 TODO*/
	unsigned short eth_type;
	unsigned short ip_protocol;
	unsigned short dport;
	unsigned short sport;
};

struct rtw_pkt_parser {
	enum rtw_pkt_type type;
	unsigned int (*parse_func)(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
};


#define RTW_PKT_FILTER_WIFI_PKT 		BIT(0)
#define RTW_PKT_FILTER_ETH_PKT  		BIT(1)
#define RTW_PKT_FILTER_WIFI_MGT_PKT 	BIT(2)
#define RTW_PKT_FILTER_WIFI_CTRL_PKT 	BIT(3)
#define RTW_PKT_FILTER_SMAC 			BIT(11)
#define RTW_PKT_FILTER_DMAC 			BIT(12)
#define RTW_PKT_FILTER_SIP  			BIT(13)
#define RTW_PKT_FILTER_DIP  			BIT(14)
#define RTW_PKT_FILTER_ETHTYPE  		BIT(15)
#define RTW_PKT_FILTER_IPPROTO  		BIT(16)
#define RTW_PKT_FILTER_SPORT  			BIT(17)
#define RTW_PKT_FILTER_DPORT  			BIT(18)
#define RTW_PKT_FILTER_INVALID_ETHTYPE 	BIT(19)
#define RTW_PKT_MAX_FILTERS (8)

void rtw_trace_packet(const char *hint, _adapter *adapter, struct rtw_pkt_trace_info *pattrib, const char *funtion, int line);
void rtw_flush_pkt_filter(_adapter *adapter);
void rtw_flush_pkt_filter(_adapter *adapter);
void rtw_dump_pkt_filter(_adapter *adapter);
int rtw_generate_pkt_filter(struct rtw_pkt_filter *pfilter, char *name ,char *value);
int rtw_add_pkt_filter(_adapter *adapter,struct rtw_pkt_filter *pfilter);
unsigned int rtw_get_filter_flag_by_name(char *name);
int rtw_wifi_rx_prepare_pkt_trace(_adapter *adapter, void *pframe);
int rtw_wifi_tx_prepare_pkt_trace(_adapter *adapter, struct xmit_frame *pxframe);
int rtw_eth_prepare_pkt_trace(_adapter *adapter, struct rtw_pkt_trace_info *pktinfo, struct sk_buff *pkt);


#define RTW_TRACE_PACKET(hint,adpater,pattrib)  rtw_trace_packet(hint,adpater,pattrib,__FUNCTION__,__LINE__)
#define RTW_RX_TRACE(adpater,pattrib) RTW_TRACE_PACKET("RX:",adpater,pattrib)
#define RTW_TX_TRACE(adpater,pattrib) RTW_TRACE_PACKET("TX:",adpater,pattrib)

#ifdef RTW_CORE_PKT_TRACE
#define RTW_TX_TRACE_CORE(padapter, pxframe, txsc_pkt, i, skb_cnt) \
	if(padapter->pkt_trace_enable) \
	{ \
		if(pxframe) \
		{ \
			RTW_TX_TRACE(padapter, &pxframe->attrib.pktinfo); \
		} \
		else \
		{ \
			for(i = 0; i < skb_cnt; i++) \
			{ \
				RTW_TX_TRACE(padapter, &txsc_pkt.pktinfo[i]); \
			} \
		} \
	}
#define RTW_TX_TRACE_ETH_TXSC_ALL(padapter, txsc_pkt, i) \
	if(padapter->pkt_trace_enable) \
	{ \
		for(i = 0; i < txsc_pkt->skb_cnt; i++) \
		{ \
			rtw_eth_prepare_pkt_trace(padapter, &txsc_pkt->pktinfo[i], txsc_pkt->xmit_skb[i]); \
			RTW_TX_TRACE(padapter, &txsc_pkt->pktinfo[i]); \
		} \
	}
#define RTW_TX_TRACE_ETH_TXSC(padapter, txsc_pkt) \
	if(padapter->pkt_trace_enable) \
	{ \
		rtw_eth_prepare_pkt_trace(padapter, &txsc_pkt.pktinfo[0], txsc_pkt.xmit_skb[0]); \
		RTW_TX_TRACE(padapter,&txsc_pkt.pktinfo[0]); \
	}
#define RTW_TX_TRACE_ETH_XFRAME(padapter, pxframe, my_pskb) \
	if(padapter->pkt_trace_enable) \
	{ \
		rtw_eth_prepare_pkt_trace(padapter, &pxframe->attrib.pktinfo, my_pskb); \
		RTW_TX_TRACE(padapter,&pxframe->attrib.pktinfo); \
	}
#define RTW_TX_TRACE_XFRAME(padapter, pxframe) \
	if(padapter->pkt_trace_enable) \
		RTW_TX_TRACE(padapter,&pxframe->attrib.pktinfo);
#define RTW_TX_TRACE_TXSC_RESET(padapter, txsc_pkt) \
	if(padapter->pkt_trace_enable) \
		memset(&txsc_pkt->pktinfo, 0, MAX_TXSC_SKB_NUM * sizeof(struct rtw_pkt_trace_info));
#else
#define RTW_TX_TRACE_CORE(padapter, pxframe, txsc_pkt, i, skb_cnt)
#define RTW_TX_TRACE_TXSC(padapter, txsc_pkt, xmit_skb, i, skb_cnt)
#define RTW_TX_TRACE_ETH_TXSC_ALL(padapter, txsc_pkt, i)
#define RTW_TX_TRACE_ETH_TXSC(padapter, txsc_pkt)
#define RTW_TX_TRACE_ETH_XFRAME(padapter, pxframe, my_pskb)
#define RTW_TX_TRACE_XFRAME(padapter, pxframe)
#define RTW_TX_TRACE_TXSC_RESET(padapter, txsc_pkt)
#endif

#endif
