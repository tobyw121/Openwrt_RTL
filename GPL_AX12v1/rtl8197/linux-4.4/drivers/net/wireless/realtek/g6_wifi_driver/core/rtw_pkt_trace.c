#include <linux/inet.h>
#include <drv_types.h>

#ifdef RTW_CORE_PKT_TRACE
enum rtw_pkt_type rtw_pkt_wifi_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_llc_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_arp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_ip_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_icmp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_udp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_tcp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_dns_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_dhcp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_http_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_telnet_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_eth_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);
enum rtw_pkt_type rtw_pkt_ipv6_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length);

void rtw_pkt_print_filter(struct rtw_pkt_filter *pfilter);
unsigned int rtw_get_filter_flag_by_name(char *name);


struct rtw_pkt_type_desc rtw_pkt_types[] = {
		{"NONE",	RTW_PKT_NONE},
		{"MGT",		RTW_PKT_WIFI_MGT},
		{"DATA",	RTW_PKT_WIFI_DATA},
		{"CTRL",	RTW_PKT_WIFI_CTRL},
		{"LLC",		RTW_PKT_LLC},
		{"ETH",		RTW_PKT_ETH},
		{"ARP",		RTW_PKT_ARP},
		{"IP",		RTW_PKT_IP},
		{"IPV6",	RTW_PKT_IPV6},
		{"PPPOE_D",		RTW_PKT_PPPOE_D},
		{"PPPOE_C",		RTW_PKT_PPPOE_C},
		{"EAPOL",	RTW_PKT_EAPOL},
		{"UDP",		RTW_PKT_UDP},
		{"TCP",		RTW_PKT_TCP},
		{"DHCP",	RTW_PKT_DHCP},
		{"DNS",		RTW_PKT_DNS},
		{"PING",	RTW_PKT_PING},
		{"HTTP",	RTW_PKT_HTTP},
		{"TELNET",	RTW_PKT_TELNET},
	};


struct rtw_pkt_parser rtw_pkt_parsers[]= {
	{RTW_PKT_WIFI_DATA,	rtw_pkt_wifi_parser},
	{RTW_PKT_ETH,		rtw_pkt_eth_parser},
	{RTW_PKT_LLC,		rtw_pkt_llc_parser},
	{RTW_PKT_ARP, 		rtw_pkt_arp_parser},
	{RTW_PKT_IP,		rtw_pkt_ip_parser},
	{RTW_PKT_IPV6,		rtw_pkt_ipv6_parser},
	{RTW_PKT_PING,		rtw_pkt_icmp_parser},
	{RTW_PKT_UDP,		rtw_pkt_udp_parser},
	{RTW_PKT_TCP,		rtw_pkt_tcp_parser},
	{RTW_PKT_DNS,		rtw_pkt_dns_parser},
	{RTW_PKT_DHCP,		rtw_pkt_dhcp_parser},
	{RTW_PKT_HTTP,		rtw_pkt_http_parser},
	{RTW_PKT_TELNET,	rtw_pkt_telnet_parser},
};

/*
  cmd example:
  iwpriv wlan0 phl_test pktrace,enable
  iwpriv wlan0 phl_test pktrace,disable
  iwpriv wlan0 phl_test pktrace,dumphdr
  iwpriv wlan0 phl_test pktrace,dumpall
  iwpriv wlan0 phl_test pktrace,dumpnone
  iwpriv wlan0 phl_test pktrace,show
  iwpriv wlan0 phl_test pktrace,wifi,MGT
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,ASSOC_REQ
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,ASSOC_RSP
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,REASSOC_REQ
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,REASSOC_RSP
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,PROBE_REQ
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,PROBE_RSP
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,BEACON
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,DISASSOC
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,AUTH
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,DEAUTH
  iwpriv wlan0 phl_test pktrace,wifi,MGT,wifi_mgt_type,ACTION
  iwpriv wlan0 phl_test pktrace,wifi,CTRL
  iwpriv wlan0 phl_test pktrace,wifi,CTRL,wifi_ctrl_type,PSPOLL
  iwpriv wlan0 phl_test pktrace,wifi,CTRL,wifi_ctrl_type,NDPA
  iwpriv wlan0 phl_test pktrace,wifi,CTRL,wifi_ctrl_type,BAR
  iwpriv wlan0 phl_test pktrace,wifi,DATA
  iwpriv wlan0 phl_test pktrace,eth,ARP
  iwpriv wlan0 phl_test pktrace,eth,PING
  iwpriv wlan0 phl_test pktrace,eth,DNS
  iwpriv wlan0 phl_test pktrace,eth,DHCP
  iwpriv wlan0 phl_test pktrace,smac,001122334455,dmac,001122334455,sip,192.168.1.1,dip,192.168.2.1,ipproto,UDP,sport,100,dport,600
  iwpriv wlan0 phl_test pktrace,sip,192.168.1.1,dip,192.168.2.1,ipproto,UDP,sport,100,dport,600
  iwpriv wlan0 phl_test pktrace,sip,192.168.1.1,dip,192.168.2.1,ipproto,UDP,sport,100,dport,600
  iwpriv wlan0 phl_test pktrace,invalid
*/

struct rtw_filter_opt  rtw_filter_opts[] = {
	{RTW_PKT_FILTER_WIFI_PKT,"wifi"},
	{RTW_PKT_FILTER_ETH_PKT,"eth"},
	{RTW_PKT_FILTER_WIFI_MGT_PKT,"wifi_mgt_type"},
	{RTW_PKT_FILTER_WIFI_CTRL_PKT,"wifi_ctrl_type"},
	{RTW_PKT_FILTER_SMAC,"smac"},
	{RTW_PKT_FILTER_DMAC,"dmac"},
	{RTW_PKT_FILTER_SIP,"sip"},
	{RTW_PKT_FILTER_DIP,"dip"},
	{RTW_PKT_FILTER_ETHTYPE,"ethtype"},
	{RTW_PKT_FILTER_IPPROTO,"ipproto"},
	{RTW_PKT_FILTER_SPORT,"sport"},
	{RTW_PKT_FILTER_DPORT,"dport"},
	{RTW_PKT_FILTER_INVALID_ETHTYPE,"invalid"},
};

struct rtw_eth_type_desc  rtw_valid_eth_types[] = {
	{"ip",		0x0800, RTW_PKT_IP},
	{"arp",		0x0806, RTW_PKT_ARP},
	{"ipv6", 	0x86dd, RTW_PKT_IPV6},
	{"PPPOE-D",	0x8864,	RTW_PKT_PPPOE_D},
	{"PPPOE-C",	0x8863, RTW_PKT_PPPOE_C},
	{"EAPOL",	0x888e,	RTW_PKT_EAPOL},
	};

void rtw_flush_pkt_filter(_adapter *adapter)
{
	int i;
	for(i=0;i<RTW_PKT_MAX_FILTERS;i++)
	{
		if(adapter->rtw_pkt_trace_filters[i].used == 1)
		{
			memset(&(adapter->rtw_pkt_trace_filters[i]),0,sizeof(struct rtw_pkt_filter));
		}

	}

}

void rtw_dump_pkt_filter(_adapter *adapter)
{
	int i;
	for(i=0;i<RTW_PKT_MAX_FILTERS;i++)
	{
		if(adapter->rtw_pkt_trace_filters[i].used == 1)
		{
			_dbgdump("Filter %d:\n",i);
			rtw_pkt_print_filter(&(adapter->rtw_pkt_trace_filters[i]));
			_dbgdump("\n");
		}
	}
}

int rtw_add_pkt_filter(_adapter *adapter,struct rtw_pkt_filter *pfilter)
{
	int i;
	for(i=0;i<RTW_PKT_MAX_FILTERS;i++)
	{
		if(adapter->rtw_pkt_trace_filters[i].used == 0)
		{
			adapter->rtw_pkt_trace_filters[i]=*pfilter;
			adapter->rtw_pkt_trace_filters[i].used=1;
			break;
		}
	}
	if(i== RTW_PKT_MAX_FILTERS)
		RTW_ERR("MAX %d reached\n",RTW_PKT_MAX_FILTERS);
	return 0;
}

enum rtw_pkt_type rtw_get_pktype_by_name(char *name)
{
	int i;
	for(i=0;i<sizeof(rtw_pkt_types)/sizeof(struct rtw_pkt_type_desc);i++)
	{
		if(0 == strncmp(name,rtw_pkt_types[i].name,strlen(rtw_pkt_types[i].name)))
		{
			return rtw_pkt_types[i].pkt_type;
		}
	}
	return RTW_PKT_NONE;
}

char *rtw_get_name_by_pktype(enum rtw_pkt_type type)
{
	int i;
	for(i=0;i<sizeof(rtw_pkt_types)/sizeof(struct rtw_pkt_type_desc);i++)
	{
		if(rtw_pkt_types[i].pkt_type == type)
		{
			return rtw_pkt_types[i].name;
		}
	}
	return rtw_pkt_types[0].name;
}


unsigned int rtw_get_filter_flag_by_name(char *name)
{
	int i;
	for(i=0;i<sizeof(rtw_filter_opts)/sizeof(struct rtw_filter_opt);i++)
	{
		if(0==strcmp(name,rtw_filter_opts[i].opt_name))
			return rtw_filter_opts[i].filter_flag;
	}
	return 0;
}

int rtw_generate_pkt_filter(struct rtw_pkt_filter *pfilter, char *name ,char *value)
{
	unsigned int filter_flag=0;
	int ret=0;
	unsigned int pktype=0;
	filter_flag = rtw_get_filter_flag_by_name(name);
	if(filter_flag)
		pfilter->match_flag |= filter_flag;
	switch(filter_flag)
	{
		case RTW_PKT_FILTER_WIFI_PKT:
			if(0==strncmp(value,"MGT",strlen("MGT"))) {
				pfilter->wl_type = WIFI_MGT_TYPE;
			} else if(0==strncmp(value,"DATA",strlen("DATA"))) {
				pfilter->wl_type = WIFI_DATA_TYPE;
			} else if(0==strncmp(value,"CTRL",strlen("CTRL"))) {
				pfilter->wl_type = WIFI_CTRL_TYPE;
			}
			break;

		case RTW_PKT_FILTER_WIFI_MGT_PKT:
			if(0==strncmp(value,"ASSOC_REQ",strlen("ASSOC_REQ"))) {
				pfilter->wl_subtype = WIFI_ASSOCREQ;
			} else if(0==strncmp(value,"ASSOC_RSP",strlen("ASSOC_RSP"))) {
				pfilter->wl_subtype = WIFI_ASSOCRSP;
			} else if(0==strncmp(value,"REASSOC_REQ",strlen("REASSOC_REQ"))) {
				pfilter->wl_subtype = WIFI_REASSOCREQ;
			} else if(0==strncmp(value,"REASSOC_RSP",strlen("REASSOC_RSP"))) {
				pfilter->wl_subtype = WIFI_REASSOCRSP;
			} else if(0==strncmp(value,"PROBE_REQ",strlen("PROBE_REQ"))) {
				pfilter->wl_subtype = WIFI_PROBEREQ;
			} else if(0==strncmp(value,"PROBE_RSP",strlen("PROBE_RSP"))) {
				pfilter->wl_subtype = WIFI_PROBERSP;
			} else if(0==strncmp(value,"BEACON",strlen("BEACON"))) {
				pfilter->wl_subtype = WIFI_BEACON;
			} else if(0==strncmp(value,"ATIM",strlen("ATIM"))) {
				pfilter->wl_subtype = WIFI_ATIM;
			} else if(0==strncmp(value,"DISASSOC",strlen("DISASSOC"))) {
				pfilter->wl_subtype = WIFI_DISASSOC;
			} else if(0==strncmp(value,"AUTH",strlen("AUTH"))) {
				pfilter->wl_subtype = WIFI_AUTH;
			} else if(0==strncmp(value,"DEAUTH",strlen("DEAUTH"))) {
				pfilter->wl_subtype = WIFI_DEAUTH;
			} else if(0==strncmp(value,"ACTION",strlen("ACTION"))) {
				pfilter->wl_subtype = WIFI_ACTION;
			} else if(0==strncmp(value,"ACTION_NOACK",strlen("ACTION_NOACK"))) {
				pfilter->wl_subtype = WIFI_ACTION_NOACK;
			}
			break;

		case RTW_PKT_FILTER_WIFI_CTRL_PKT:
			if(0==strncmp(value,"PSPOLL",strlen("PSPOLL"))) {
				pfilter->wl_subtype = WIFI_PSPOLL;
			} else if(0==strncmp(value,"NDPA",strlen("NDPA"))) {
				pfilter->wl_subtype = WIFI_NDPA;
			} else if(0==strncmp(value,"BAR",strlen("BAR"))) {
				pfilter->wl_subtype = WIFI_BAR;
			}
			break;

		case RTW_PKT_FILTER_ETH_PKT:
			pfilter->pktype =rtw_get_pktype_by_name(value);
			break;

		case RTW_PKT_FILTER_SMAC:
			hexstr2bin(value,pfilter->smac,strlen(value));
			break;
		case RTW_PKT_FILTER_DMAC :
			hexstr2bin(value,pfilter->dmac,strlen(value));
			break;
		case RTW_PKT_FILTER_SIP:
			pfilter->sip = ntohl(in_aton(value));
			break;
		case RTW_PKT_FILTER_DIP:
			pfilter->dip = ntohl(in_aton(value));
			break;
		case RTW_PKT_FILTER_ETHTYPE:
			pfilter->eth_type = simple_strtoul(value,NULL,16);
			break;
		case RTW_PKT_FILTER_IPPROTO:
			if(0==strncmp(value,"TCP",3)) {
				pfilter->ip_protocol = 0x6;
			} else if(0==strncmp(value,"UDP",3)) {
				pfilter->ip_protocol = 0x11;
			}
			break;
		case RTW_PKT_FILTER_SPORT:
			pfilter->sport = simple_strtoul(value,NULL,10);
			break;
		case RTW_PKT_FILTER_DPORT :
			pfilter->dport = simple_strtoul(value,NULL,10);
			break;
		case RTW_PKT_FILTER_INVALID_ETHTYPE:
			break;
		default:
			RTW_ERR("unknow fiter flag\n");
			ret=-1;
			break;
	}
	return ret;
}


int rtw_same_mac(unsigned char *mac1, unsigned char *mac2)
{
	int i=0;
	for(i=0;i<ETH_ALEN;i++)
	{
		if(mac1[i] != mac2[i])
			return 0;
	}
	return 1;
}

int rtw_valid_eth_type(unsigned short eth_type)
{
	int i;
	for(i=0;i<sizeof(rtw_valid_eth_types)/sizeof(struct rtw_eth_type_desc);i++)
	{
		if(rtw_valid_eth_types[i].eth_type == eth_type)
			return 1;
	}
	return 0;
}

unsigned int rtw_get_pktype_by_ethtype(unsigned short eth_type)
{
	int i;
	for(i=0;i<sizeof(rtw_valid_eth_types)/sizeof(struct rtw_eth_type_desc);i++)
	{
		if(rtw_valid_eth_types[i].eth_type == eth_type)
			return rtw_valid_eth_types[i].pkt_type;
	}
	return RTW_PKT_NONE;

}

/*match1 && match2 && match3 ...*/
int rtw_ptk_filter_match(struct rtw_pkt_filter *pfilter, struct rtw_pkt_trace_info *pattrib)
{
	if(!(pfilter->match_flag))
		return 0;
	if(!pattrib->parsed)
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_WIFI_PKT) && (pfilter->wl_type != pattrib->wl_type))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_WIFI_MGT_PKT) && (pfilter->wl_subtype != pattrib->wl_subtype))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_WIFI_CTRL_PKT) && (pfilter->wl_subtype != pattrib->wl_subtype))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_ETH_PKT) && (pfilter->pktype != pattrib->pktype))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_SMAC) && !rtw_same_mac(pfilter->smac,pattrib->smac))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_DMAC) && !rtw_same_mac(pfilter->dmac,pattrib->dmac))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_ETHTYPE) && (pfilter->eth_type != pattrib->eth_type))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_SIP) && (pfilter->sip != pattrib->sip))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_DIP) && (pfilter->dip != pattrib->dip))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_IPPROTO) && (pfilter->ip_protocol != pattrib->ip_protocol))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_SPORT) && (pfilter->sport != pattrib->sport))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_DPORT) && (pfilter->dport != pattrib->dport))
		return 0;
	if((pfilter->match_flag & RTW_PKT_FILTER_INVALID_ETHTYPE) && rtw_valid_eth_type(pfilter->eth_type))
		return 0;

	return 1;
}


/*First match First out*/
struct rtw_pkt_filter *rtw_ptk_find_filter(_adapter *adapter, struct rtw_pkt_trace_info *pattrib)
{
	int i=0;
	struct rtw_pkt_filter *pfilter;
	for(i=0;i<sizeof(adapter->rtw_pkt_trace_filters)/sizeof(struct rtw_pkt_filter);i++)
	{
		pfilter = &(adapter->rtw_pkt_trace_filters[i]);
		if(pfilter->used && rtw_ptk_filter_match(pfilter,pattrib))
		{
			return pfilter;
		}
		if(!pfilter->used)
			break;
	}
	return NULL;
}

void rtw_pkt_print_filter(struct rtw_pkt_filter *pfilter)
{
	unsigned int ip;
	//_dbgdump("match_flag 0x%x\n",pfilter->match_flag);
	if((pfilter->match_flag & RTW_PKT_FILTER_WIFI_PKT))
		_dbgdump("%s ",rtw_get_name_by_pktype((pfilter->wl_type == WIFI_MGT_TYPE) ? 1:((pfilter->wl_type == WIFI_DATA_TYPE) ? 2:3)));
	if((pfilter->match_flag & RTW_PKT_FILTER_WIFI_MGT_PKT))
		_dbgdump("MGT type: %x ", pfilter->wl_subtype);
	if((pfilter->match_flag & RTW_PKT_FILTER_WIFI_CTRL_PKT))
		_dbgdump("CTRL type: %x ", pfilter->wl_subtype);
	if((pfilter->match_flag & RTW_PKT_FILTER_ETH_PKT))
		_dbgdump("%s ",rtw_get_name_by_pktype(pfilter->pktype));
	if((pfilter->match_flag & RTW_PKT_FILTER_SMAC))
		_dbgdump("smac:%pM ",pfilter->smac);
	if((pfilter->match_flag & RTW_PKT_FILTER_DMAC))
		_dbgdump("dmac:%pM ",pfilter->dmac);
	if((pfilter->match_flag & RTW_PKT_FILTER_ETHTYPE))
		_dbgdump("eth_type:0x%x ",pfilter->eth_type);
	if((pfilter->match_flag & RTW_PKT_FILTER_SIP)) {
		ip = htonl(pfilter->sip);
		_dbgdump("sip:%pI4  ",&ip);
	}
	if((pfilter->match_flag & RTW_PKT_FILTER_DIP)) {
		ip = htonl(pfilter->dip);
		_dbgdump("dip:%pI4 ",&ip);
	}
	if((pfilter->match_flag & RTW_PKT_FILTER_IPPROTO))
		_dbgdump("ipproto:0x%x ",pfilter->ip_protocol);
	if((pfilter->match_flag & RTW_PKT_FILTER_SPORT))
		_dbgdump("sport:%d ",pfilter->sport);
	if((pfilter->match_flag & RTW_PKT_FILTER_DPORT))
		_dbgdump("dport:%d ",pfilter->dport);
	if((pfilter->match_flag & RTW_PKT_FILTER_INVALID_ETHTYPE))
		_dbgdump("invalid eth type");
	_dbgdump("\t");
}

void rtw_ptk_print_pkt(struct rtw_pkt_trace_info *pattrib)
{
	if(pattrib->wl_type == WIFI_MGT_TYPE)
	{
		switch(pattrib->wl_subtype) {
			case WIFI_ASSOCREQ:
				_dbgdump("assoc_req ");
				break;
			case WIFI_ASSOCRSP:
				_dbgdump("assoc_rsp ");
				break;
			case WIFI_REASSOCREQ:
				_dbgdump("reassoc_req ");
				break;
			case WIFI_REASSOCRSP:
				_dbgdump("reassoc_rsp ");
				break;
			case WIFI_PROBEREQ:
				_dbgdump("probe_req ");
				break;
			case WIFI_PROBERSP:
				_dbgdump("probe_rsp ");
				break;
			case WIFI_BEACON:
				_dbgdump("beacon ");
				break;
			case WIFI_ATIM:
				_dbgdump("atim ");
				break;
			case WIFI_DISASSOC:
				_dbgdump("disassoc ");
				break;
			case WIFI_AUTH:
				_dbgdump("auth ");
				break;
			case WIFI_DEAUTH:
				_dbgdump("deauth ");
				break;
			case WIFI_ACTION:
				_dbgdump("action ");
				break;
			case WIFI_ACTION_NOACK:
				_dbgdump("action noack ");
				break;
			default:
				_dbgdump("unknow wifi mgmt type: %x ", pattrib->wl_subtype);
				break;
		}
		_dbgdump("smac:%pM ",pattrib->smac);
		_dbgdump("dmac:%pM ",pattrib->dmac);
	}

	if(pattrib->wl_type == WIFI_CTRL_TYPE)
	{
		switch(pattrib->wl_subtype) {
			case WIFI_PSPOLL:
				_dbgdump("pspoll ");
				break;
			case WIFI_NDPA:
				_dbgdump("ndpa ");
				break;
			case WIFI_BAR:
				_dbgdump("bar ");
				break;
			default:
				_dbgdump("unknow wifi ctrl type: %x ", pattrib->wl_subtype);
				break;
		}
		_dbgdump("smac:%pM ",pattrib->smac);
		_dbgdump("dmac:%pM ",pattrib->dmac);
	}

	switch(pattrib->pktype) {
		case RTW_PKT_ARP:
			_dbgdump("%s ",(pattrib->subtype == RTW_PKT_ARP_REQ ? "req":(pattrib->subtype == RTW_PKT_ARP_RPLY ? "rply": "none")));
			break;
		case RTW_PKT_DHCP:
			switch(pattrib->subtype)
			{
				case RTW_PKT_DHCP_DISCOVER:
					_dbgdump("discover ");
					break;
				case RTW_PKT_DHCP_OFFER:
					_dbgdump("offer ");
					break;
				case RTW_PKT_DHCP_REQUEST:
					_dbgdump("request ");
					break;
				case RTW_PKT_DHCP_DECLINE:
					_dbgdump("decline ");
					break;
				case RTW_PKT_DHCP_ACK:
					_dbgdump("ack ");
					break;
				default:
					break;
			}
			_dbgdump("XID 0x%x ",pattrib->transaction_id);
			break;
		case RTW_PKT_PING:
			_dbgdump("%s ",(pattrib->subtype == RTW_PKT_PING_REQ ? "req":(pattrib->subtype == RTW_PKT_PING_RPLY ? "rply": "none")));
			break;
		case RTW_PKT_DNS:
			_dbgdump("%s ",(pattrib->subtype == RTW_PKT_DNS_REQ ? "req":(pattrib->subtype == RTW_PKT_DNS_RPLY ? "rply": "none")));
			_dbgdump("XID 0x%x ",pattrib->transaction_id);
			break;
		default:
			break;
	}
	if(0x0800 == pattrib->eth_type)
		_dbgdump("ipid 0x%x",pattrib->ipid);
	_dbgdump("\n");
	return;
}

void rtw_pkt_dump(unsigned char *ptr, int len)
{
	int i;

	if (ptr == NULL) {
		RTW_ERR("%s: NULL pointer.\n", __FUNCTION__);
		dump_stack();
		return;
	}

	_dbgdump("### %pX %uB: #################\n", ptr, len);
	for (i = 0; i < len; i = i + 8) {
		unsigned char *d = ptr + i;
		_dbgdump("%02X %02X %02X %02X  %02X %02X %02X %02X\n",
		         d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
	}
	_dbgdump("#############################\n");
}

void rtw_pkt_dump_header(struct rtw_pkt_trace_info *pattrib)
{
#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (pattrib->dir == PKT_RX) {
		if (   pattrib->pktype == RTW_PKT_WIFI_DATA
		    || pattrib->pktype == RTW_PKT_WIFI_CTRL
		    || pattrib->pktype == RTW_PKT_WIFI_MGT)
			rtw_pkt_dump(pattrib->hdr, pattrib->hdrlen + LLC_HEADER_SIZE + 2);
		else
			rtw_pkt_dump(pattrib->hdr, pattrib->hdrlen);
		return;
	}
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	rtw_pkt_dump(pattrib->ptr, pattrib->hdrlen);
}

void rtw_pkt_dump_all(struct rtw_pkt_trace_info *pattrib)
{
	rtw_pkt_dump(pattrib->ptr,pattrib->pkt_len);
}


/*
 * print matched filter & pkt base info
 *
 */
void rtw_pkt_trace_dump(struct rtw_pkt_filter *pfilter,struct rtw_pkt_trace_info *pattrib)
{
	rtw_pkt_print_filter(pfilter);
	rtw_ptk_print_pkt(pattrib);
}

void rtw_trace_packet(const char *hint, _adapter *adapter, struct rtw_pkt_trace_info *pattrib, const char *funtion, int line)
{
	struct rtw_pkt_filter *pfilter;
	if(!adapter->pkt_trace_enable)
		return;
	pfilter=rtw_ptk_find_filter(adapter, pattrib);
	if(pfilter)
	{
		_dbgdump("[%s %d]: %s\n",funtion,line,hint);
		rtw_pkt_trace_dump(pfilter,pattrib);
		if(adapter->pkt_trace_level == RTW_PKT_DUMP_HEADER)
		{
			rtw_pkt_dump_header(pattrib);
		} else if(adapter->pkt_trace_level == RTW_PKT_DUMP_ALL)
		{
			rtw_pkt_dump_all(pattrib);
		}
	}
}


enum rtw_pkt_type rtw_pkt_ip_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned char *ip = ptr;
	unsigned int nextype= RTW_PKT_NONE;

	*length= GET_IPV4_IHL(ip) * 4;
	if(leftlen < *length) {
		RTW_ERR("%s %d leftlen %d length %d\n",__FUNCTION__,__LINE__,leftlen,*length);
		return RTW_PKT_NONE;
	}

	pattrib->pktype = RTW_PKT_IP;

	pattrib->ipid = GET_IPV4_IPID(ip);
	pattrib->ip_protocol = GET_IPV4_PROTOCOL(ip);
	pattrib->sip = GET_IPV4_SRC(ip);
	pattrib->dip = GET_IPV4_DST(ip);

	if (GET_IPV4_PROTOCOL(ip) == 0x01) { /* ICMP */
		nextype = RTW_PKT_PING;
	} else if (GET_IPV4_PROTOCOL(ip) == 0x11) { /* UDP */
		nextype = RTW_PKT_UDP;
	} else if (GET_IPV4_PROTOCOL(ip) == 0x06) { /* TCP */
		nextype = RTW_PKT_TCP;
	}

	return  nextype;
}




enum rtw_pkt_type rtw_pkt_ipv6_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned int nextype= RTW_PKT_NONE;
	pattrib->pktype = RTW_PKT_IPV6;
	return  nextype;
}


enum rtw_pkt_type rtw_pkt_wifi_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	int headerlen;
	unsigned int nextype= RTW_PKT_NONE;

	headerlen = pattrib->llc_offset;

	if(leftlen < headerlen)
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_WIFI_DATA;

	/* RX header conversion could have separated wlan header.
	 * Pointer to frame should not be deducted. */
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (pattrib->ptr != pattrib->hdr) {
		headerlen = 0;
	}
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	*length = headerlen;

	return RTW_PKT_LLC;
}


enum rtw_pkt_type rtw_pkt_eth_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	u16 eth_type;
	unsigned int nextype= RTW_PKT_NONE;
	unsigned char *eth = ptr;

	if(leftlen < 14)
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_ETH;
	pattrib->dmac = eth;
	pattrib->smac = eth + ETH_ALEN;
	if(memcmp(eth + ETH_ALEN * 2 + 2, rtw_rfc1042_header, SNAP_SIZE) == 0)
	{
		*length = 14 + SNAP_SIZE + 2;
		eth_type = ntohs(*(unsigned short *)(eth+12+SNAP_SIZE+2));
		pattrib->hdrlen += SNAP_SIZE + 2;
	}
	else
	{
		*length = 14;
		eth_type = ntohs(*(unsigned short *)(eth+12));
	}
	pattrib->eth_type = eth_type;
	nextype = rtw_get_pktype_by_ethtype(eth_type);
	return nextype;
}


enum rtw_pkt_type rtw_pkt_llc_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	int headerlen;
	u16 eth_type;
	unsigned int nextype= RTW_PKT_NONE;
	unsigned char *wifi = ptr;

	if(leftlen < (LLC_HEADER_SIZE+2))
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_LLC;
	*length = (LLC_HEADER_SIZE+2);

	/* RX header conversion could have separated wlan header.
	 * Pointer to frame should not be deducted. */
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	if (pattrib->ptr != pattrib->hdr) {
		wifi = pattrib->hdr;
		*length = 0;
	}
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	eth_type = ntohs(*(unsigned short *)(wifi+LLC_HEADER_SIZE));
	pattrib->eth_type = eth_type;
	nextype = rtw_get_pktype_by_ethtype(eth_type);
	return nextype;
}


enum rtw_pkt_type rtw_pkt_arp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned char *arp;
	unsigned short opcode;
	unsigned int nextype= RTW_PKT_NONE;

	arp = ptr;
	if(leftlen < 28)
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_ARP;
	*length = 28;

	opcode = GET_ARP_OPER(arp);
	if(opcode == 0x1)
		pattrib->subtype = RTW_PKT_ARP_REQ;
	else if(opcode == 0x2)
		pattrib->subtype = RTW_PKT_ARP_RPLY;

	return nextype;
}

enum rtw_pkt_type rtw_pkt_icmp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned char *icmp, type, code;
	unsigned short opcode;
	unsigned int nextype = RTW_PKT_NONE;

	icmp=ptr;
	if(leftlen < 8)
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_PING;
	*length = 8;

	type = *ptr;
	code = *(ptr+1);

	if(type == 8 && code == 0)
		pattrib->subtype = RTW_PKT_PING_REQ;
	else if(type==0 && code ==0)
		pattrib->subtype = RTW_PKT_PING_RPLY;

	return nextype;
}

enum rtw_pkt_type rtw_pkt_udp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned char *udp;
	unsigned int nextype = RTW_PKT_NONE;

	udp=ptr;
	if(leftlen < 8)
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_UDP;
	*length = 8;

	pattrib->sport = ntohs(*(unsigned short *)udp);
	pattrib->dport = ntohs(*(unsigned short *)(udp+2));

	if(((pattrib->sport == 67) && (pattrib->dport ==68)) ||
		((pattrib->sport == 68) && (pattrib->dport ==67)))
		nextype = RTW_PKT_DHCP;
	else if((pattrib->sport==53)|| (pattrib->dport == 53))
		nextype = RTW_PKT_DNS;

	return nextype;
}

enum rtw_pkt_type rtw_pkt_tcp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned char *tcp;
	unsigned int nextype = RTW_PKT_NONE;

	tcp = ptr;

	if(leftlen < 20) {
		RTW_ERR("%s %d leftlen %d\n",__FUNCTION__,__LINE__,leftlen);
		return RTW_PKT_NONE;
	}

	*length = GET_TCP_DOFF(tcp) * 4 ;
	if(leftlen < (*length)) {
		RTW_ERR("%s %d leftlen %d length %d\n",__FUNCTION__,__LINE__,leftlen,*length);
		return RTW_PKT_NONE;
	}


	pattrib->pktype = RTW_PKT_TCP;
	pattrib->sport = GET_TCP_SRC(tcp);
	pattrib->dport = GET_TCP_DST(tcp);
	/*record tcp flag*/
	pattrib->subtype = *(tcp+13);

	if((pattrib->sport == 80) || (pattrib->dport == 80))
		nextype = RTW_PKT_HTTP;
	else if((pattrib->sport == 23) || (pattrib->dport == 23))
		nextype = RTW_PKT_TELNET;
	return nextype;
}

enum rtw_pkt_type rtw_pkt_dns_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned char *dns;
	unsigned int nextype = RTW_PKT_NONE;
	unsigned short flag;

	dns=ptr;
	if(leftlen < 12)
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_DNS;
	*length = 12;

	pattrib->transaction_id = ntohs(*(unsigned short *)dns);
	flag = ntohs(*(unsigned short *)(dns+2));
	if(0 == (flag & 0x1000))
		pattrib->subtype = RTW_PKT_DNS_REQ;
	else
		pattrib->subtype = RTW_PKT_DNS_RPLY;

	return nextype;
}

enum rtw_pkt_type rtw_get_dhcp_op53(unsigned char *ptr, int leftlen, unsigned char *value)
{
	unsigned char type=0;
	unsigned char len=0;

	type=*ptr;
	while((leftlen > 0) && (type != 0xFF))
	{
		if(type == 0x35) {
			*value = *(ptr+2);
			return 1;
		}
		len=*ptr+1;
		ptr += (1+1+len);
		leftlen -= (1+1+len);
	}
	return 0;
}

enum rtw_pkt_type rtw_pkt_dhcp_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned char *dhcp;
	unsigned int nextype = RTW_PKT_NONE;
	unsigned short flag = 0;
	unsigned char op53 = 0;

	dhcp=ptr;
	if(leftlen < 240)
		return RTW_PKT_NONE;

	pattrib->pktype = RTW_PKT_DHCP;
	*length = 240;
	pattrib->transaction_id = ntohl(*(unsigned int *)(dhcp+4));

	/*skip to options*/
	dhcp += 240;
	leftlen -= 240;
	if(leftlen > 0) {
		if(0==rtw_get_dhcp_op53(dhcp,leftlen,&op53)) {
			RTW_ERR("Error DHCP packet, can not find options 53\n");
			return RTW_PKT_NONE;
		}
		if(op53 == 0x1)
			pattrib->subtype = RTW_PKT_DHCP_DISCOVER;
		else if(op53 == 0x2)
			pattrib->subtype = RTW_PKT_DHCP_OFFER;
		else if(op53 == 0x3)
			pattrib->subtype = RTW_PKT_DHCP_REQUEST;
		else if(op53 == 0x4)
			pattrib->subtype = RTW_PKT_DHCP_DECLINE;
		else if(op53 == 0x5)
			pattrib->subtype = RTW_PKT_DHCP_ACK;
	}

	return nextype;
}

enum rtw_pkt_type rtw_pkt_http_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned int nextype = RTW_PKT_NONE;
	pattrib->pktype = RTW_PKT_HTTP;
	return nextype;
}
enum rtw_pkt_type rtw_pkt_telnet_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	unsigned int nextype = RTW_PKT_NONE;
	pattrib->pktype = RTW_PKT_TELNET;
	return nextype;
}


enum rtw_pkt_type rtw_pkt_default_parser(struct rtw_pkt_trace_info *pattrib, unsigned char *ptr, int leftlen, int *length)
{
	return RTW_PKT_NONE;
}

struct rtw_pkt_parser *rtw_find_parser(unsigned int pktype)
{
	int i;
	for(i=0;i<sizeof(rtw_pkt_parsers)/sizeof(struct rtw_pkt_parser );i++)
	{
		if(rtw_pkt_parsers[i].type == pktype)
			return &rtw_pkt_parsers[i];
	}
	return NULL;
}


void rtw_dump_pkt_info(struct rtw_pkt_trace_info *pattrib)
{
	u32 ip;
	_dbgdump("ptr %pX\n",pattrib->ptr);
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	_dbgdump("hdr %pX\n", pattrib->hdr);
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	_dbgdump("hdrlen %d\n",pattrib->hdrlen);
	_dbgdump("llc_offset %d\n",pattrib->llc_offset);
	_dbgdump("pkt_len %d\n",pattrib->pkt_len);
	_dbgdump("pktype %d\n",pattrib->pktype);
	_dbgdump("subtype %d\n",pattrib->subtype);
	_dbgdump("dmac %pM\n",pattrib->dmac);
	_dbgdump("smac %pM\n",pattrib->smac);
	_dbgdump("eth_type %d\n",pattrib->eth_type);
	_dbgdump("ip_protocol %d\n",pattrib->ip_protocol);
	_dbgdump("wl_type %d\n",pattrib->wl_type);
	ip=htonl(pattrib->sip);
	_dbgdump("sip %pI4\n",&ip);
	ip=htonl(pattrib->dip);
	_dbgdump("dip %pI4\n",&ip);
	_dbgdump("sport %d\n",pattrib->sport);
	_dbgdump("dport %d\n",pattrib->dport);
	_dbgdump("ipid %d\n",pattrib->ipid);
	_dbgdump("transaction_id %d\n",pattrib->transaction_id);
}

int rtw_parse_pkt(_adapter *adapter, struct rtw_pkt_trace_info *pattrib, enum rtw_pkt_type type)
{
	int leftlen=0,length=0,ret=0;
	enum rtw_pkt_type nextype = type;
	u8 *ptr = pattrib->ptr;
	struct rtw_pkt_parser *parser=NULL;
	//DBGP("\n");
	leftlen=pattrib->pkt_len;
	while(leftlen > 0)
	{
		parser=rtw_find_parser(nextype);
		if(NULL == parser) {
			RTW_ERR("NO parser found for %d\n",nextype);
			ret=-1;
			break;
		}
		nextype=parser->parse_func(pattrib,ptr,leftlen,&length);
		if(nextype == RTW_PKT_NONE)
			break;
		ptr += length;
		leftlen-= length;
	}
	/*
	if(ret == 0)
		rtw_dump_pkt_info(pattrib);
	*/
	return ret;
}

int rtw_wifi_rx_prepare_pkt_trace(_adapter *adapter, void *pframe)
{
	int ret=0;
	union recv_frame *precv_frame = (union recv_frame *)pframe;
	struct rtw_pkt_trace_info *pktinfo;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
        #ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	struct rtw_recv_pkt *rx_req = precv_frame->u.hdr.rx_req;
	u8 *whdr = precv_frame->u.hdr.wlan_hdr;
	#else
	u8 *whdr = precv_frame->u.hdr.rx_data;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	pktinfo = &pattrib->pktinfo;

	if(!adapter->pkt_trace_enable)
		return ret;

	memset(pktinfo, 0, sizeof(struct rtw_pkt_trace_info));
	pktinfo->ptr = precv_frame->u.hdr.rx_data;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	pktinfo->hdr = whdr;
	#endif
	pktinfo->pkt_len = pattrib->pkt_len;
	pktinfo->hdrlen = pattrib->hdrlen;
	pktinfo->wl_type = pattrib->wl_type;
	pktinfo->parsed = 1;
	pktinfo->dir = PKT_RX;

	if(pattrib->wl_type == WIFI_MGT_TYPE || pattrib->wl_type == WIFI_CTRL_TYPE)
	{
		pktinfo->wl_subtype = pattrib->wl_subtype;
		pktinfo->smac = get_addr2_ptr(whdr);
		pktinfo->dmac = GetAddr1Ptr(whdr);
		return ret;
	}
	else
	{
		pktinfo->llc_offset = pattrib->hdrlen + pattrib->iv_len + RATTRIB_GET_MCTRL_LEN(pattrib);
		pktinfo->wl_subtype = 0xff;
		pktinfo->smac = pattrib->src;
		pktinfo->dmac = pattrib->dst;
	}

	return rtw_parse_pkt(adapter,pktinfo,RTW_PKT_WIFI_DATA);
}

int rtw_wifi_tx_prepare_pkt_trace(_adapter *adapter, struct xmit_frame *pxframe)
{
	int ret=0;
	unsigned char	*pframe = NULL;
	struct pkt_attrib	*pattrib = NULL;
	struct rtw_pkt_trace_info *pktinfo;
	pframe = (u8 *)(pxframe->buf_addr) + TXDESC_OFFSET;
	pattrib = &pxframe->attrib;
	pktinfo = &pattrib->pktinfo;

	if(!adapter->pkt_trace_enable)
		return ret;

	memset(pktinfo, 0, sizeof(struct rtw_pkt_trace_info));
	pktinfo->ptr = pframe;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	pktinfo->hdr = pframe;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	pktinfo->pkt_len = pattrib->pktlen;
	pktinfo->hdrlen = pattrib->hdrlen;

	pktinfo->wl_type = pattrib->type;
	pktinfo->wl_subtype = get_frame_sub_type(pframe);
	pktinfo->smac = get_addr2_ptr(pframe);
	pktinfo->dmac = GetAddr1Ptr(pframe);
	pktinfo->parsed = 1;
	pktinfo->dir = PKT_TX;

	return ret;
}

int rtw_eth_prepare_pkt_trace(_adapter *adapter, struct rtw_pkt_trace_info *pktinfo, struct sk_buff *pkt)
{
	int ret=0;

	if(!adapter->pkt_trace_enable)
		return ret;

	memset(pktinfo, 0, sizeof(struct rtw_pkt_trace_info));
	pktinfo->ptr = pkt->data;
	pktinfo->pkt_len = pkt->len;
	pktinfo->hdrlen = ETH_HLEN;
	pktinfo->wl_type = 0xff;
	pktinfo->wl_subtype = 0xff;
	pktinfo->parsed = 1;

	return rtw_parse_pkt(adapter,pktinfo,RTW_PKT_ETH);
}


#endif
