/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/neighbour.h>
#include <net/arp.h>
#include <linux/export.h>
#include <linux/rculist.h>
#include "br_private.h"
#if defined(CONFIG_RTL_819X)&&defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
#include <net/rtl/rtl_nic.h>
#endif
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <linux/ip.h>
#include <linux/in.h>
#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <net/mld.h>
#endif
#include <linux/igmp.h>
#include <net/checksum.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>

#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined (CONFIG_NETFILTER)
#include <linux/netfilter_ipv4/ip_tables.h>
#endif
#if defined CONFIG_RTL_HARDWARE_MULTICAST
#include <net/rtl/rtl865x_multicast.h>
#if defined CONFIG_RTL_IGMP_PROXY_MULTIWAN
#include <net/rtl/rtl_multi_wan.h>
#endif
#endif
#if defined(CONFIG_RTL_DNS_TRAP)
#include <net/rtl/rtl_dnstrap.h>
#endif
#if defined(CONFIG_RTL_HTTP_REDIRECT)
extern int redirect_on;
extern unsigned int http_redirect_enter(struct sk_buff *skb);
extern int is_http_packet(struct sk_buff *skb);
extern int is_http_resp_packet(struct sk_buff *skb);

#endif
extern int igmpsnoopenabled;
#if defined (CONFIG_RTL_MLD_SNOOPING)
extern int mldSnoopEnabled;
#endif
extern unsigned int brIgmpModuleIndex;
extern unsigned int br0SwFwdPortMask;
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
extern unsigned int brIgmpModuleIndex_2;
extern unsigned int br1SwFwdPortMask;
#endif
#if defined (CONFIG_RTL_HW_MCAST_WIFI)
extern int hwwifiEnable;
#endif
extern int igmp_netfilter;
#if defined CONFIG_RTL_VLAN_8021Q && (defined(CONFIG_RTL_MULTICAST_PORT_MAPPING) || defined (CONFIG_RTL_CUSTOM_PASSTHRU))
extern int linux_vlan_enable;
#endif
#if defined (MCAST_TO_UNICAST)
extern int IGMPProxyOpened;
#if defined (IPV6_MCAST_TO_UNICAST)
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/icmpv6.h>
#if defined (CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
static char ICMPv6_check(struct sk_buff *skb , unsigned char *gmac, unsigned int *gIndex,unsigned int *moreFlag, int type);
#else
static char ICMPv6_check(struct sk_buff *skb , unsigned char *gmac, unsigned int *gIndex,unsigned int *moreFlag);
#endif
#endif	//end of IPV6_MCAST_TO_UNICAST
#endif	//end of MCAST_TO_UNICAST
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
#include <net/udp.h>
#endif
#if defined (CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag, int type);
#else
static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag);
#endif
static void br_update_igmp_snoop_fdb(unsigned char op, struct net_bridge *br, struct net_bridge_port *p, unsigned char *gmac
									,struct sk_buff *skb);
#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
extern int rtl_get_brIgmpModueIndx(struct net_bridge *br);
extern unsigned int rtl_get_brSwFwdPortMask(struct net_bridge *br);
#if defined (CONFIG_RTL_VLAN_8021Q) && defined (CONFIG_RTL_MULTICAST_PORT_MAPPING)
unsigned int rtl865x_getPhyPortMapMaskbyVlan(struct net_bridge *br, struct sk_buff *skb);
#endif
#endif
#endif	//end of CONFIG_RTL_IGMP_SNOOPING
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
#if defined(CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_multicast.h>
extern int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br,
													 unsigned int srcPort,
													 unsigned int srcVlanId,
													 unsigned int srcIpAddr,
													 struct net_bridge_mdb_entry *mdst);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
extern int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h);
#endif/*CONFIG_RTL_8198C*/
#endif/*CONFIG_RTL_HARDWARE_MULTICAST*/
#endif/*CONFIG_BRIDGE_IGMP_SNOOPING*/
/* Hook for brouter */
br_should_route_hook_t __rcu *br_should_route_hook __read_mostly;
EXPORT_SYMBOL(br_should_route_hook);

static int
br_netif_receive_skb(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	br_drop_fake_rtable(skb);
	return netif_receive_skb(skb);
}

static int br_pass_frame_up(struct sk_buff *skb)
{
	struct net_device *indev, *brdev = BR_INPUT_SKB_CB(skb)->brdev;
	struct net_bridge *br = netdev_priv(brdev);
	struct net_bridge_vlan_group *vg;
	struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->stats);

	u64_stats_update_begin(&brstats->syncp);
	brstats->rx_packets++;
	brstats->rx_bytes += skb->len;
	u64_stats_update_end(&brstats->syncp);

	vg = br_vlan_group_rcu(br);
	/* Bridge is just like any other port.  Make sure the
	 * packet is allowed except in promisc modue when someone
	 * may be running packet capture.
	 */
	if (!(brdev->flags & IFF_PROMISC) &&
	    !br_allowed_egress(vg, skb)) {
		kfree_skb(skb);
		return NET_RX_DROP;
	}

	indev = skb->dev;
	skb->dev = brdev;
	skb = br_handle_vlan(br, vg, skb);
	if (!skb)
		return NET_RX_DROP;

	return NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN,
		       dev_net(indev), NULL, skb, indev, NULL,
		       br_netif_receive_skb);
}

#if defined(CONFIG_RTL_MLD_SNOOPING)
extern int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h);
#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST) 
#if defined(CONFIG_RTL_IGMP_SNOOPING)

#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
extern int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr, unsigned int mapPortMask);
#else
extern int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr);
#endif
#if defined(CONFIG_RTL_MLD_SNOOPING)
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
extern int rtl865x_ipv6MulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
													unsigned int srcPort,unsigned int srcVlanId, 
													struct in6_addr srcIpAddr,struct in6_addr destIpAddr, unsigned int mapPortMask);
#else
extern int rtl865x_ipv6MulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
													unsigned int srcPort,unsigned int srcVlanId, 
													struct in6_addr srcIpAddr,struct in6_addr destIpAddr);
#endif
#endif
#endif
#endif
#endif
#if defined (CONFIG_RTL_QUERIER_SELECTION)
#define HOP_BY_HOP_OPTIONS_HEADER 0
#define ROUTING_HEADER 43
#define FRAGMENT_HEADER 44
#define DESTINATION_OPTION_HEADER 60
#define NO_NEXT_HEADER 59
#define ICMP_PROTOCOL 58
#define IPV4_ROUTER_ALTER_OPTION 0x94040000
#define IPV6_ROUTER_ALTER_OPTION 0x05020000
#define IPV6_HEADER_LENGTH 40
#define MLD_QUERY 130
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143
#define S_FLAG_MASK 0x08

extern int br_updateQuerierInfo(unsigned int version, unsigned char *devName, unsigned int* querierIp);

int check_igmpQueryExist(struct iphdr * iph)
{

	if(iph==NULL)
	{
		return 0;
	}

	if(*(unsigned char *)((unsigned char*)iph+((iph->ihl)<<2))==0x11)
	{
		return 1;
	}
	
	return 0;
}



int check_mldQueryExist(struct ipv6hdr* ipv6h)
{

	unsigned char *ptr=NULL;
	unsigned char *startPtr=NULL;
	unsigned char *lastPtr=NULL;
	unsigned char nextHeader=0;
	unsigned short extensionHdrLen=0;

	unsigned char  optionDataLen=0;
	unsigned char  optionType=0;
	unsigned int ipv6RAO=0;

	if(ipv6h==NULL)
	{
		return 0;
	}

	if(ipv6h->version!=6)
	{
		return 0;
	}

	startPtr= (unsigned char *)ipv6h;
	lastPtr=startPtr+sizeof(struct ipv6hdr)+ntohs(ipv6h->payload_len);
	nextHeader= ipv6h ->nexthdr;
	ptr=startPtr+sizeof(struct ipv6hdr);

	while(ptr<lastPtr)
	{
		switch(nextHeader)
		{
			case HOP_BY_HOP_OPTIONS_HEADER:
				/*parse hop-by-hop option*/
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+2;

				while(ptr<(startPtr+extensionHdrLen+sizeof(struct ipv6hdr)))
				{
					optionType=ptr[0];
					/*pad1 option*/
					if(optionType==0)
					{
						ptr=ptr+1;
						continue;
					}

					/*padN option*/
					if(optionType==1)
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}

					/*router altert option*/
					if(ntohl(*(uint32 *)(ptr))==IPV6_ROUTER_ALTER_OPTION)
					{
						ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
						ptr=ptr+4;
						continue;
					}

					/*other TLV option*/
					if((optionType!=0) && (optionType!=1))
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}


				}

				break;

			case ROUTING_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
                            ptr=ptr+extensionHdrLen;
				break;

			case FRAGMENT_HEADER:
				nextHeader=ptr[0];
				ptr=ptr+8;
				break;

			case DESTINATION_OPTION_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+extensionHdrLen;
				break;

			case ICMP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if(ptr[0]==MLD_QUERY)
				{
					return 1;

				}
				break;

			default:
				/*not ipv6 multicast protocol*/
				return 0;
		}

	}
	return 0;
}

#endif/*CONFIG_RTL_QUERIER_SELECTION*/


static void br_do_proxy_arp(struct sk_buff *skb, struct net_bridge *br,
			    u16 vid, struct net_bridge_port *p)
{
	struct net_device *dev = br->dev;
	struct neighbour *n;
	struct arphdr *parp;
	u8 *arpptr, *sha;
	__be32 sip, tip;

	BR_INPUT_SKB_CB(skb)->proxyarp_replied = false;

	if (dev->flags & IFF_NOARP)
		return;

	if (!pskb_may_pull(skb, arp_hdr_len(dev))) {
		dev->stats.tx_dropped++;
		return;
	}
	parp = arp_hdr(skb);

	if (parp->ar_pro != htons(ETH_P_IP) ||
	    parp->ar_op != htons(ARPOP_REQUEST) ||
	    parp->ar_hln != dev->addr_len ||
	    parp->ar_pln != 4)
		return;

	arpptr = (u8 *)parp + sizeof(struct arphdr);
	sha = arpptr;
	arpptr += dev->addr_len;	/* sha */
	memcpy(&sip, arpptr, sizeof(sip));
	arpptr += sizeof(sip);
	arpptr += dev->addr_len;	/* tha */
	memcpy(&tip, arpptr, sizeof(tip));

	if (ipv4_is_loopback(tip) ||
	    ipv4_is_multicast(tip))
		return;

	n = neigh_lookup(&arp_tbl, &tip, dev);
	if (n) {
		struct net_bridge_fdb_entry *f;

		if (!(n->nud_state & NUD_VALID)) {
			neigh_release(n);
			return;
		}

		f = __br_fdb_get(br, n->ha, vid);
		if (f && ((p->flags & BR_PROXYARP) ||
			  (f->dst && (f->dst->flags & BR_PROXYARP_WIFI)))) {
			arp_send(ARPOP_REPLY, ETH_P_ARP, sip, skb->dev, tip,
				 sha, n->ha, sha);
			BR_INPUT_SKB_CB(skb)->proxyarp_replied = true;
		}

		neigh_release(n);
	}
}


#if defined (CONFIG_RTL_CUSTOM_PASSTHRU) && defined(CONFIG_RTL_VLAN_8021Q) && defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG)
unsigned int rtl865x_getPortMaskbyVlan(struct net_bridge *br,struct sk_buff *skb)
{	
	unsigned int PortMask = 0;
	struct net_bridge_port *p, *n;
	struct dev_priv *devPriv =NULL;

	int src_vid = 0;
	if (*(uint16*)skb->linux_vlan_src_tag == __constant_htons(ETH_P_8021Q)) 
 	{ 
  		src_vid = ntohs(*(short *)(skb->linux_vlan_src_tag+2)) & 0x0fff;
 	}

	if(br==NULL)
	{
		return 0;
	}

	list_for_each_entry_safe(p, n, &br->port_list, list) 
	{

		devPriv=(struct dev_priv *)netdev_priv(p->dev);
		if(devPriv)
		{
			if(p->dev->vlan_id == src_vid)
			{
				//panic_printk("vlanid:%s, [%s:%d]\n", strvlanid, __FUNCTION__, __LINE__);
				PortMask |= (1<<p->port_no);
			}
		}
	}
	
	return PortMask;
}
#endif
/* note: already called with rcu_read_lock */
int br_handle_frame_finish(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;

#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
	struct net_bridge_mdb_entry *mdst;
#endif
	struct sk_buff *skb2;
	bool unicast = true;
	u16 vid = 0;
#if defined (CONFIG_BRIDGE_IGMP_SNOOPING)
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl865x_mcast_fwd_descriptor_t fwdDescriptor;
	unsigned int srcPort=skb->srcPort;
	unsigned int srcVlanId=skb->srcVlanId;
#endif
#endif

#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
	int type = 0;
#endif

	if (!p || p->state == BR_STATE_DISABLED)
		   
		   goto drop;

	if (!br_allowed_ingress(p->br, nbp_vlan_group_rcu(p), skb, &vid))
		   
		   goto out;

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
	if (p->flags & BR_LEARNING
#if defined(CONFIG_RTL_DNS_TRAP)
		&&(!is_dns_packet((struct sk_buff*)skb))
#endif
#if defined(CONFIG_RTL_HTTP_REDIRECT)
		&&(!is_http_resp_packet((struct sk_buff *)skb))
#endif
	)
		br_fdb_update(br, p, eth_hdr(skb)->h_source, vid, false);

	if (!is_broadcast_ether_addr(dest) && is_multicast_ether_addr(dest) &&
	    br_multicast_rcv(br, p, skb, vid))
		{
		  goto drop;
	    	}
	if (p->state == BR_STATE_LEARNING)
		goto drop;

	BR_INPUT_SKB_CB(skb)->brdev = br->dev;

	/* The packet skb2 goes to the local host (NULL to skip). */
	skb2 = NULL;

	if (br->dev->flags & IFF_PROMISC)
		skb2 = skb;

	dst = NULL;

	if (IS_ENABLED(CONFIG_INET) && skb->protocol == htons(ETH_P_ARP))
		br_do_proxy_arp(skb, br, vid, p);

	
#if !defined(CONFIG_RTL_IGMP_SNOOPING)
	if (is_broadcast_ether_addr(dest)) {
		skb2 = skb;
		unicast = false;
	} else
#endif

	if (is_multicast_ether_addr(dest)) {
		
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
		mdst = br_mdb_get(br, skb, vid);
		if ((mdst || BR_INPUT_SKB_CB_MROUTERS_ONLY(skb)) &&
		    br_multicast_querier_exists(br, eth_hdr(skb))) {
			if ((mdst && mdst->mglist) ||
			    br_multicast_is_router(br))
				skb2 = skb;
			br_multicast_forward(mdst, skb, skb2);
#if defined(CONFIG_RTL_HARDWARE_MULTICAST)
			if(mdst)
			{
				if(MULTICAST_MAC(dest))
				{
					struct iphdr *iph=(struct iphdr *)skb_network_header(skb);
					if(iph->protocol==IPPROTO_UDP||iph->protocol==IPPROTO_TCP)
					{
						if((srcVlanId!=0) && (srcPort!=0xFFFF))
						{
							rtl865x_ipMulticastHardwareAccelerate(br,srcPort,srcVlanId,iph->saddr,mdst);
						}
					}
				}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
				else if(IPV6_MULTICAST_MAC(dest))
				{
					struct ipv6hdr *ipv6h=(struct ipv6hdr *)skb_network_header(skb);
					unsigned char proto = re865x_getIpv6TransportProtocol(ipv6h);
					if(proto==IPPROTO_UDP||proto==IPPROTO_TCP)
					{
					}
				}
#endif/*CONFIG_RTL_8198C*/
			}
#endif/*CONFIG_RTL_HARDWARE_MULTICAST*/
			skb = NULL;
			if (!skb2)
				goto out;
		} else
#endif/*CONFIG_BRIDGE_IGMP_SNOOPING*/

			skb2 = skb;

		unicast = false;
		br->dev->stats.multicast++;
	} else if ((dst = __br_fdb_get(br, dest, vid)) &&
			dst->is_local) {
			
		skb2 = skb;
		/* Do not forward the packet since it's local. */
		skb = NULL;
	}
#if !defined(CONFIG_RTL_IGMP_SNOOPING)
	if (skb) {
#if defined(CONFIG_RTL_WLAN_BLOCK_RELAY)
		if(rtl_wlan_block_relay_enable && dst){
			if(!memcmp(skb->dev->name,RTL_WLAN_INT_PREFIX,sizeof(RTL_WLAN_INT_PREFIX)-1)){
				if(!memcmp(dst->dst->dev->name,RTL_WLAN_INT_PREFIX,sizeof(RTL_WLAN_INT_PREFIX)-1))
					goto drop;
			}
		}
#endif
		if (dst) {
			dst->used = jiffies;
			br_forward(dst->dst, skb, skb2);
		} else
			br_flood_forward(br, skb, skb2,unicast);
	}
	if (skb2)
		return br_pass_frame_up(skb2);
#endif/*CONFIG_BRIDGE_IGMP_SNOOPING*/
#if defined(CONFIG_RTL_IGMP_SNOOPING)
#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
        
		//printk("[%s:%d] br->dev->name is %s\n",__FUNCTION__,__LINE__,br->dev->name);
		if((MULTICAST_MAC(dest)||IPV6_MULTICAST_MAC(dest)) && strcmp(br->dev->name, RTL_PS_BR0_DEV_NAME)!=0)
		{
		}
		else
		{
			if (skb2 == skb)
				skb2 = skb_clone(skb, GFP_ATOMIC);
			if (skb2)
				br_pass_frame_up(skb2);
		}
#else
		if (skb2 == skb)
			skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2)
			br_pass_frame_up(skb2);
#endif

	if (skb) {
		
#if defined(CONFIG_RTL_WLAN_BLOCK_RELAY)
	if(rtl_wlan_block_relay_enable && dst){
		if(!memcmp(skb->dev->name,RTL_WLAN_INT_PREFIX,sizeof(RTL_WLAN_INT_PREFIX)-1)){
			if(!memcmp(dst->dst->dev->name,RTL_WLAN_INT_PREFIX,sizeof(RTL_WLAN_INT_PREFIX)-1))
				goto drop;
		}
	}
#endif

#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
	type = rtl_is_pppoe_igmp_bridge_frame(skb,0,dest);
	if ((is_multicast_ether_addr(dest)||(type != 0)) && (igmpsnoopenabled || mldSnoopEnabled))
#else		
	if (is_multicast_ether_addr(dest) && (igmpsnoopenabled || mldSnoopEnabled))
#endif
	{
					struct iphdr *iph=NULL;
#if defined (CONFIG_RTL_MLD_SNOOPING) 	
					struct ipv6hdr *ipv6h=NULL;
#endif/*CONFIG_RTL_MLD_SNOOPING*/
					uint32 fwdPortMask=0;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
					unsigned int srcPort=skb->srcPort;
					unsigned int srcVlanId=skb->srcVlanId;
#endif/*CONFIG_RTL_HARDWARE_MULTICAST*/
			
					unsigned char proto=0;
					unsigned char reserved=0;
					int ret=FAILED;
					
					unsigned char macAddr[6];
					unsigned char operation;
					char tmpOp;
					unsigned int gIndex=0;
					unsigned int moreFlag=1;
					
					struct rtl_multicastDataInfo multicastDataInfo;
					struct rtl_multicastFwdInfo multicastFwdInfo;
		#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
					unsigned int igmpModuleIndex=0xFFFFFFFF;
					unsigned int swFwdPortMask =0xFFFFFFFF;
					igmpModuleIndex =rtl_get_brIgmpModueIndx(br);
					swFwdPortMask = rtl_get_brSwFwdPortMask(br);
		#endif
				
		#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
					unsigned int mapPortMask = 0xFFFFFFFF;
		#if defined CONFIG_RTL_IGMP_PROXY_MULTIWAN
					struct smux_dev_info *dev_info;
		#endif
		#endif
			
		#if defined (CONFIG_RTL_HW_MCAST_WIFI)
					rtl865x_tblDrv_mCast_t * existMulticastEntry;
		#endif	
				
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
					//mc data forwarded in bridge with pppoe header will be handled by hw.
					if ( !(br->dev->flags & IFF_PROMISC) 
						&&( (MULTICAST_MAC(dest)  && (eth_hdr(skb)->h_proto == htons(ETH_P_IP))) || type == PPPOE_IGMP)
						&& igmpsnoopenabled)
#else
					if ( !(br->dev->flags & IFF_PROMISC) 
						&& MULTICAST_MAC(dest) 
						&& (eth_hdr(skb)->h_proto == htons(ETH_P_IP))
						&& igmpsnoopenabled)
#endif
					{
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
						if (type == PPPOE_IGMP)
							iph = rtl_get_pppoe_ipv4_hdr(skb->data, 0, skb->protocol);
						else
							iph = (struct iphdr *)skb_network_header(skb);
						if (iph == NULL)
							goto drop;
#else
						iph=(struct iphdr *)skb_network_header(skb);
#endif
				
			#if defined(CONFIG_USB_UWIFI_HOST)
						if(iph->daddr == htonl(0xEFFFFFFA) || iph->daddr == htonl(0xE1010101))
			#else
						if(iph->daddr == htonl(0xEFFFFFFA) || iph->daddr == htonl(0xE00000FB))
			#endif
						
						{
							/*for microsoft upnp*/
							reserved=1;
						}
#if 0
						if((iph->daddr&0xFFFFFF00)==0xE0000000)
						reserved=1;
#endif
						proto =  iph->protocol;  
						if (proto == IPPROTO_IGMP) 
						{	
#if defined (CONFIG_NETFILTER)
							//filter igmp pkts by upper hook like iptables 
							
							//printk("[%s:%d]igmp_netfilter is %x,IgmpRxFilter_Hook is %x\n",__FUNCTION__,__LINE__,igmp_netfilter,IgmpRxFilter_Hook);
							if(igmp_netfilter && IgmpRxFilter_Hook != NULL)
							{
								struct net_device	*origDev=skb->dev;
								struct net_bridge_port *p=br_port_get_rcu(skb->dev);
			#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
								unsigned char *data_ori = skb->data;
								unsigned short network_ori = skb->network_header;
			#endif
								if(p && p->br)
								{
									skb->dev=p->br->dev;
								}
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
								if(type == PPPOE_IGMP)
								{	
									skb->data = (unsigned char*)iph;
									skb->network_header = (unsigned char*)iph - skb->head;				
								}
#endif
								if(IgmpRxFilter_Hook(skb, NF_INET_LOCAL_IN, skb->dev, NULL,dev_net(skb->dev)->ipv4.iptable_filter) !=NF_ACCEPT)
								{
								
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
									if(type == PPPOE_IGMP)
									{
										skb->data = data_ori;
										skb->network_header = network_ori;
									}
#endif
									skb->dev=origDev;
									DEBUG_PRINT(" filter a pkt:%d %s:% \n", k, skb->dev->name, &(dev_net(skb->dev)->ipv4.iptable_filter->name[0]));
									goto drop;
								}
								else
								{						
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
									if(type == PPPOE_IGMP)
									{
										skb->data = data_ori;
										skb->network_header = network_ori;
									}
#endif
									skb->dev=origDev;
								}
									
								
								
							}else
								DEBUG_PRINT("IgmpRxFilter_Hook is NULL\n");
#endif/*CONFIG_NETFILTER*/
							while(moreFlag)
							{
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
								tmpOp=igmp_type_check(skb, macAddr, &gIndex, &moreFlag, type);
#else
								tmpOp=igmp_type_check(skb, macAddr, &gIndex, &moreFlag);
#endif

								if(tmpOp>0)
								{
								    
									operation=(unsigned char)tmpOp;
									//rtnl_lock();
									br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
									//rtnl_unlock();
								}
							}
							
			#if defined (CONFIG_RTL_QUERIER_SELECTION)
							if(check_igmpQueryExist(iph)==1)
							{
								struct net_bridge_port *p=br_port_get_rcu(skb->dev);
								/*igmp query packet*/
								if(p)
								{
									br_updateQuerierInfo(4,p->br->dev->name,(unsigned int*)&(iph->saddr));
								}
								else
								{
									br_updateQuerierInfo(4,skb->dev->name,(unsigned int*)&(iph->saddr));
								}
							}
			#endif
					
			#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)

							rtl_igmpMldProcess(igmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);
			#else
	        	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
							if(!strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME))
							{
								rtl_igmpMldProcess(brIgmpModuleIndex_2, skb_mac_header(skb), p->port_no, &fwdPortMask);
								//flooding igmp packet
								fwdPortMask=(~(1<<(p->port_no))) & 0xFFFFFFFF;
							}
							else
			#endif
			       
							rtl_igmpMldProcess(brIgmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);
			#endif
							br_multicast_forward(br, fwdPortMask, skb, 0);
						}
						else if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP)||(proto ==Any_0_hop_protocl)) && (reserved ==0))
						{

						
			#if !defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
							iph=(struct iphdr *)skb_network_header(skb);
			#endif
							multicastDataInfo.ipVersion=4;
							multicastDataInfo.sourceIp[0]=	(uint32)(iph->saddr);
							multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
							multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
							multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
			#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
							ret= rtl_getMulticastDataFwdInfo(igmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			#else
	        #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
							if(!strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME))
							{
								ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex_2, &multicastDataInfo, &multicastFwdInfo);
							}
							else
			#endif
							ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			#endif

					
			#if defined (CONFIG_RTL_HW_MCAST_WIFI)
							if(hwwifiEnable)
							{
								existMulticastEntry=rtl865x_findMCastEntry(multicastDataInfo.groupAddr[0], multicastDataInfo.sourceIp[0], (unsigned short)srcVlanId, (unsigned short)srcPort);
								if(existMulticastEntry!=NULL)
								{
									/*it's already in cache, only forward to wlan */
			#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
									multicastFwdInfo.fwdPortMask &= swFwdPortMask;
			#else
									multicastFwdInfo.fwdPortMask &= br0SwFwdPortMask;

			#endif
								}
							}
			#endif

			#if defined (CONFIG_RTL_HW_MCAST_WIFI)
							if((hwwifiEnable && (ret==SUCCESS)) ||
							   (hwwifiEnable ==0 && (ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0)))
			#else
							if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
			#endif
							{
				#if defined  (CONFIG_RTL_HARDWARE_MULTICAST)
				
								if((srcVlanId!=0) && (srcPort!=0xFFFF))
								{
				#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
				#if defined CONFIG_RTL_IGMP_PROXY_MULTIWAN
									if(skb->from_dev)
									{
										dev_info = SMUX_DEV_INFO(skb->from_dev);
										mapPortMask = dev_info->member;
									}
									else
										mapPortMask = 0xFFFFFFFF;
								//printk("srcPort = %d, fwdPortMask = %x, [%s:%d]\n", srcPort,	multicastFwdInfo.fwdPortMask, __FUNCTION__, __LINE__);
				#elif defined CONFIG_RTL_VLAN_8021Q
									if(linux_vlan_enable)
										mapPortMask = rtl865x_getPhyPortMapMaskbyVlan(br, skb);
									else 
										mapPortMask = 0xFFFFFFFF;
				#endif
								rtl865x_ipMulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0], mapPortMask);

				#else
						#if defined(CONFIG_RTK_VLAN_SUPPORT)
								if(rtk_vlan_support_enable == 0)
								{
									rtl865x_ipMulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0]);
								}
						#else
								rtl865x_ipMulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0]);
						#endif
				#endif
								}	
				#endif
							}
					
							br_multicast_forward(br, multicastFwdInfo.fwdPortMask, skb, 0);
						}
						else
						{
							br_flood_forward(br, skb);
						}
					}
#if defined (CONFIG_RTL_MLD_SNOOPING)
#if defined (CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
					else if(!(br->dev->flags & IFF_PROMISC) 
						&& ((IPV6_MULTICAST_MAC(dest)&& (eth_hdr(skb)->h_proto == htons(ETH_P_IPV6)))||(type==PPPOE_ICMPV6))
						&& mldSnoopEnabled)
#else
					else if(!(br->dev->flags & IFF_PROMISC) 
						&& IPV6_MULTICAST_MAC(dest)
						&& (eth_hdr(skb)->h_proto == htons(ETH_P_IPV6))
						&& mldSnoopEnabled)
#endif
					{
#if defined (CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
						if(type==PPPOE_ICMPV6)
							ipv6h = rtl_get_pppoe_ipv6_hdr(skb->data, 0, skb->protocol);
						else
#endif
							ipv6h=(struct ipv6hdr *)skb_network_header(skb);
						if(ipv6h==NULL)
							goto drop;
						
						proto =  re865x_getIpv6TransportProtocol(ipv6h);
						/*icmp protocol*/
						if (proto == IPPROTO_ICMPV6) 
						{	
#if 0
							if(net_ratelimit())
								printk("[%s:%d]ipv6 ICMPV6,\n",__FUNCTION__,__LINE__);
								
#endif
#if defined (IPV6_MCAST_TO_UNICAST)
							while(moreFlag)
							{
#if defined (CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
								tmpOp=ICMPv6_check(skb , macAddr, &gIndex, &moreFlag, type);
#else
								tmpOp=ICMPv6_check(skb , macAddr, &gIndex, &moreFlag);
#endif

								if(tmpOp > 0)
								{
									operation=(unsigned char)tmpOp;
#ifdef	DBG_ICMPv6
									if( operation == 1)
										printk("icmpv6 add from frame finish\n");
									else if(operation == 2)
										printk("icmpv6 del from frame finish\n");
#endif
									//rtnl_lock();
									br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
								    //rtnl_unlock();

								}
							}
#endif

				#if defined (CONFIG_RTL_QUERIER_SELECTION)
							if(check_mldQueryExist(ipv6h)==1)
							{
								struct net_bridge_port *p = br_port_get_rcu(skb->dev);
								if(p)
								{
									br_updateQuerierInfo(6,p->br->dev->name,(unsigned int*)&(ipv6h->saddr));
								}
								else
								{
									br_updateQuerierInfo(6,skb->dev->name,(unsigned int*)&(ipv6h->saddr));
								}
			
							}
				#endif
						
				#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
				
							rtl_igmpMldProcess(igmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);
				#else
				
							rtl_igmpMldProcess(brIgmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);	
				#endif
							br_multicast_forward(br, fwdPortMask, skb, 0);
						}
						else if ((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP)||(proto ==Any_0_hop_protocl))
						{
							multicastDataInfo.ipVersion=6;
							memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, sizeof(struct in6_addr));
							memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, sizeof(struct in6_addr));
							multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
							multicastDataInfo.sourceIp[1] = ntohl(multicastDataInfo.sourceIp[1]);
							multicastDataInfo.sourceIp[2] = ntohl(multicastDataInfo.sourceIp[2]);
							multicastDataInfo.sourceIp[3] = ntohl(multicastDataInfo.sourceIp[3]);
							multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
							multicastDataInfo.groupAddr[1] = ntohl(multicastDataInfo.groupAddr[1]);
							multicastDataInfo.groupAddr[2] = ntohl(multicastDataInfo.groupAddr[2]);
							multicastDataInfo.groupAddr[3] = ntohl(multicastDataInfo.groupAddr[3]);
			#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
							ret= rtl_getMulticastDataFwdInfo(igmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			#else
							ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			#endif

			#if defined (CONFIG_RTL_CUSTOM_PASSTHRU) && defined (CONFIG_RTL_VLAN_8021Q) && defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG)
							if(((strncmp(skb->dev->name, "peth", 4)==0) || (strncmp(skb->dev->name, "pwlan", 5)==0))&& linux_vlan_enable)
							{
								if(ret == SUCCESS)
								{
									multicastFwdInfo.fwdPortMask &= rtl865x_getPortMaskbyVlan(br, skb);
									if(multicastFwdInfo.fwdPortMask == 0)
									{
										multicastFwdInfo.fwdPortMask = rtl865x_getPortMaskbyVlan(br, skb);
										skb->passThruFlag |= FLOOD_IN_BRIDGE;
									}
								}
								else
								{
									if(multicastFwdInfo.unknownMCast==TRUE)
									{
										multicastFwdInfo.fwdPortMask &= rtl865x_getPortMaskbyVlan(br, skb);
										skb->passThruFlag |= FLOOD_IN_BRIDGE;
										
									}

								}
							}
			#endif
							br_multicast_forward(br, multicastFwdInfo.fwdPortMask, skb, 0);
#if 0
							if(net_ratelimit())
								printk("[%s:%d]ipv6 TCP&&UDP,ret=%d,mask=%X,cpuFlag=%d,srcVlanId=%d,srcPort=%d\n",
								__FUNCTION__,
								__LINE__,
								ret,
								multicastFwdInfo.fwdPortMask,
								multicastFwdInfo.cpuFlag,
								srcVlanId,
								srcPort);
#endif
#if (defined(CONFIG_RTL_8198C)||defined(CONFIG_RTL_8197F))&&defined(CONFIG_RTL_HARDWARE_MULTICAST)
							if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
							{
								if((srcVlanId!=0) && (srcPort!=0xFFFF))
								{
									struct in6_addr sip;
									struct in6_addr dip;
									memcpy(&sip,multicastDataInfo.sourceIp,sizeof(struct in6_addr));
									memcpy(&dip,multicastDataInfo.groupAddr,sizeof(struct in6_addr));
					#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
						#if defined CONFIG_RTL_IGMP_PROXY_MULTIWAN
									if(skb->from_dev)
									{
										dev_info = SMUX_DEV_INFO(skb->from_dev);
										mapPortMask = dev_info->member;
									}
									else
										mapPortMask = 0xFFFFFFFF;
									//printk("srcPort = %d, fwdPortMask = %x, [%s:%d]\n", srcPort,	multicastFwdInfo.fwdPortMask, __FUNCTION__, __LINE__);
						#elif defined CONFIG_RTL_VLAN_8021Q
									if(linux_vlan_enable)
										mapPortMask = rtl865x_getPhyPortMapMaskbyVlan(br, skb);
									else 
										mapPortMask = 0xFFFFFFFF;
						#endif							
								rtl865x_ipv6MulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, sip,dip, mapPortMask);
					#else
					#if defined(CONFIG_RTK_VLAN_SUPPORT)
									if(rtk_vlan_support_enable == 0)
									{
										rtl865x_ipv6MulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, sip,dip);
									}
					#else
									rtl865x_ipv6MulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, sip,dip);
					#endif
					#endif
								}	
							}
#endif
						}
						else
						{
							br_flood_forward(br, skb);
						}	

					}
#endif
					else
					{
						br_flood_forward(br, skb);
					}			
		}
else
	{
		//if (dst) {
		//	dst->used = jiffies;
		//	br_forward(dst->dst, skb, skb2);
		//} else
		//	br_flood_forward(br, skb, skb2, unicast);
		/*known/unknown unicast packet*/
		if (dst)
			br_forward(dst->dst, skb);
		else
			br_flood_forward(br, skb);
	}
}
		//if (skb2)
		//return br_pass_frame_up(skb2);
#endif/*CONFIG_RTL_IGMP_SNOOPING*/
	

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}
EXPORT_SYMBOL_GPL(br_handle_frame_finish);

/* note: already called with rcu_read_lock */
static int br_handle_local_finish(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	u16 vid = 0;

	/* check if vlan is allowed, to avoid spoofing */
	if (p->flags & BR_LEARNING && br_should_learn(p, skb, &vid))
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source, vid, false);
	return 0;	 /* process further */
}

/*
 * Return NULL if skb is handled
 * note: already called with rcu_read_lock
 */
rx_handler_result_t br_handle_frame(struct sk_buff **pskb)
{
	struct net_bridge_port *p;
	struct sk_buff *skb = *pskb;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	br_should_route_hook_t *rhook;

	if (unlikely(skb->pkt_type == PACKET_LOOPBACK))
		return RX_HANDLER_PASS;

	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto drop;

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		return RX_HANDLER_CONSUMED;

#if defined(CONFIG_RTL_DNS_TRAP)
	if (dns_filter_enable)
	{
		br_dns_filter_enter(skb);
	}
#endif
#if defined(CONFIG_RTL_HTTP_REDIRECT)
	if(redirect_on && is_http_packet(skb))
	{
		//printk("%s:%d .... skbHead-skbData=%d\n",__FUNCTION__,__LINE__,skb->head-skb->data);		
		http_redirect_enter(skb);
	}
#endif

	p = br_port_get_rcu(skb->dev);

	if (unlikely(is_link_local_ether_addr(dest))) {
		u16 fwd_mask = p->br->group_fwd_mask_required;

		/*
		 * See IEEE 802.1D Table 7-10 Reserved addresses
		 *
		 * Assignment		 		Value
		 * Bridge Group Address		01-80-C2-00-00-00
		 * (MAC Control) 802.3		01-80-C2-00-00-01
		 * (Link Aggregation) 802.3	01-80-C2-00-00-02
		 * 802.1X PAE address		01-80-C2-00-00-03
		 *
		 * 802.1AB LLDP 		01-80-C2-00-00-0E
		 *
		 * Others reserved for future standardization
		 */
		switch (dest[5]) {
		case 0x00:	/* Bridge Group Address */
			/* If STP is turned off,
			   then must forward to keep loop detection */
			if (p->br->stp_enabled == BR_NO_STP ||
			    fwd_mask & (1u << dest[5]))
				goto forward;
			break;

		case 0x01:	/* IEEE MAC (Pause) */
			goto drop;

		default:
			/* Allow selective forwarding for most other protocols */
			fwd_mask |= p->br->group_fwd_mask;
			if (fwd_mask & (1u << dest[5]))
				goto forward;
		}

		/* Deliver packet to local host only */
		if (NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN,
			    dev_net(skb->dev), NULL, skb, skb->dev, NULL,
			    br_handle_local_finish)) {
			return RX_HANDLER_CONSUMED; /* consumed by filter */
		} else {
			*pskb = skb;
			return RX_HANDLER_PASS;	/* continue processing */
		}
	}

forward:
	switch (p->state) {
	case BR_STATE_FORWARDING:
		rhook = rcu_dereference(br_should_route_hook);
		if (rhook) {
			if ((*rhook)(skb)) {
				*pskb = skb;
				return RX_HANDLER_PASS;
			}
			dest = eth_hdr(skb)->h_dest;
		}
		/* fall through */
	case BR_STATE_LEARNING:
		if (ether_addr_equal(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;

		NF_HOOK(NFPROTO_BRIDGE, NF_BR_PRE_ROUTING,
			dev_net(skb->dev), NULL, skb, skb->dev, NULL,
			br_handle_frame_finish);
		break;
	default:
drop:
		kfree_skb(skb);
	}
	return RX_HANDLER_CONSUMED;
}
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#if defined (IPV6_MCAST_TO_UNICAST)
static void CIPV6toMac
	(unsigned char* icmpv6_McastAddr, unsigned char *gmac )
{
	gmac[0] = 0x33;
	gmac[1] = 0x33;
	gmac[2] = icmpv6_McastAddr[12];
	gmac[3] = icmpv6_McastAddr[13];
	gmac[4] = icmpv6_McastAddr[14];
	gmac[5] = icmpv6_McastAddr[15];			
}
#if 0
static char ICMPv6_check(struct sk_buff *skb , unsigned char *gmac)
{
	struct ipv6hdr *ipv6h;
	char* protoType;	
	ipv6h = (struct ipv6hdr *)skb_network_header(skb);
	if(ipv6h->version != 6){	
		return -1;
	}
	if(ipv6h->nexthdr == 0)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );	
	}else{
		return -1;
	}
	if(protoType[0] == 0x3a){
		struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		unsigned char* icmpv6_McastAddr ;
		if(icmpv6h->icmp6_type == 0x83){
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8);
			#ifdef	DBG_ICMPv6					
			printk("Type: 0x%x (Multicast listener report) \n",icmpv6h->icmp6_type);
			#endif
		}else if(icmpv6h->icmp6_type == 0x8f){		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 + 4);
			#ifdef	DBG_ICMPv6					
			printk("Type: 0x%x (Multicast listener report v2) \n",icmpv6h->icmp6_type);
			#endif			
		}else if(icmpv6h->icmp6_type == 0x84){
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 );			
			#ifdef	DBG_ICMPv6					
			printk("Type: 0x%x (Multicast listener done ) \n",icmpv6h->icmp6_type);
			#endif			
		}
		else{
			#ifdef	DBG_ICMPv6
			printk("Type: 0x%x (unknow type)\n",icmpv6h->icmp6_type);
			#endif			
			return -1;
		}				
		#ifdef	DBG_ICMPv6			
		printk("MCAST_IPV6Addr:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			icmpv6_McastAddr[0],icmpv6_McastAddr[1],icmpv6_McastAddr[2],icmpv6_McastAddr[3],
			icmpv6_McastAddr[4],icmpv6_McastAddr[5],icmpv6_McastAddr[6],icmpv6_McastAddr[7],
			icmpv6_McastAddr[8],icmpv6_McastAddr[9],icmpv6_McastAddr[10],icmpv6_McastAddr[11],
			icmpv6_McastAddr[12],icmpv6_McastAddr[13],icmpv6_McastAddr[14],icmpv6_McastAddr[15]);
		#endif
		CIPV6toMac(icmpv6_McastAddr, gmac);
		#ifdef	DBG_ICMPv6					
		printk("group_mac [%02x:%02x:%02x:%02x:%02x:%02x] \n",
			gmac[0],gmac[1],gmac[2],
			gmac[3],gmac[4],gmac[5]);
		#endif
		if(icmpv6h->icmp6_type == 0x83){
			return 1;//icmpv6 listener report (add)
		}
		else if(icmpv6h->icmp6_type == 0x8f){
			return 1;//icmpv6 listener report v2 (add) 
		}
		else if(icmpv6h->icmp6_type == 0x84){
			return 2;//icmpv6 Multicast listener done (del)
		}
	}		
	else{
		return -1;//not icmpv6 type
	}
	return -1;
}
#else
#if defined (CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
static char ICMPv6_check(struct sk_buff *skb , unsigned char *gmac, unsigned int *gIndex,unsigned int *moreFlag, int type)
#else
static char ICMPv6_check(struct sk_buff *skb , unsigned char *gmac, unsigned int *gIndex,unsigned int *moreFlag)
#endif
{
	struct ipv6hdr *ipv6h;
	char* protoType;
	int i = 0,j = 0;
	*moreFlag=0;
#if defined (CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
	if(type==PPPOE_ICMPV6)
		ipv6h = rtl_get_pppoe_ipv6_hdr(skb->data, 0, skb->protocol);
	else
#endif
		ipv6h = (struct ipv6hdr *)skb_network_header(skb);	
	if(ipv6h->version != 6){
       	return -1;
    }
	if(ipv6h->nexthdr == 0) 
	{
       	protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );
	}
	else
	{
    	return -1;
    }
    if(protoType[0] == 0x3a)
	{
        struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		unsigned char *icmpv6_McastAddr;
        if(icmpv6h->icmp6_type == ICMPV6_MGM_REPORT)
		{
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8);
           	#ifdef  DBG_ICMPv6
         	printk("Type: 0x%x (Multicast listener report) \n",icmpv6h->icmp6_type);
         	#endif
           	if(icmpv6_McastAddr[0] != 0xFF)
            	return -1;
        	CIPV6toMac(icmpv6_McastAddr, gmac);
           	#ifdef  DBG_ICMPv6
         	printk("MCAST_IPV6Addr:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
            	icmpv6_McastAddr[0],icmpv6_McastAddr[1],icmpv6_McastAddr[2],icmpv6_McastAddr[3],
            	icmpv6_McastAddr[4],icmpv6_McastAddr[5],icmpv6_McastAddr[6],icmpv6_McastAddr[7],
            	icmpv6_McastAddr[8],icmpv6_McastAddr[9],icmpv6_McastAddr[10],icmpv6_McastAddr[11],
             	icmpv6_McastAddr[12],icmpv6_McastAddr[13],icmpv6_McastAddr[14],icmpv6_McastAddr[15]);
           	printk("group_mac [%02x:%02x:%02x:%02x:%02x:%02x] \n",
           		gmac[0],gmac[1],gmac[2],
            	gmac[3],gmac[4],gmac[5]);
          	#endif
         	return 1;//add
  		}
		else if(icmpv6h->icmp6_type == ICMPV6_MLD2_REPORT)
		{
         	icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 + 4);
        	#ifdef  DBG_ICMPv6
         	printk("Type: 0x%x (Multicast listener report v2) \n",icmpv6h->icmp6_type);
            #endif
        	struct mld2_report *mldv2report = (struct mld2_report * )icmpv6h;
           	struct mld2_grec *mldv2grec = NULL;
           	#ifdef  DBG_ICMPv6
          	printk("%s:%d,*gIndex is %d,mldv2report->ngrec is %d\n",__FUNCTION__,__LINE__,*gIndex, ntohs(mldv2report->mld2r_ngrec));
         	#endif
         	if(*gIndex >= ntohs(mldv2report->mld2r_ngrec))
            {
            	*moreFlag=0;
            	return -1;
       		}
          	for(i=0;i< ntohs(mldv2report->mld2r_ngrec);i++)
           	{
            	if(i==0)
               	{
                	mldv2grec = (struct mld2_grec *)(&(mldv2report->mld2r_grec)); /*first igmp group record*/
              	}
               	else
               	{
                	mldv2grec = (struct mld2_grec *)((unsigned char*)mldv2grec+4+16+ntohs(mldv2grec->grec_nsrcs)*16+(mldv2grec->grec_auxwords)*4);
              	}
               	if(i!=*gIndex)
               	{
                	continue;
               	}
              	if(i==(ntohs(mldv2report->mld2r_ngrec)-1))
            	{
                  	*moreFlag=0;
              	}
             	else
              	{
               		*moreFlag=1;
               	}
               	*gIndex=*gIndex+1;
             	icmpv6_McastAddr = (unsigned char *)&mldv2grec->grec_mca;
              	if(icmpv6_McastAddr[0] != 0xFF)
                  	return -1;
            	CIPV6toMac(icmpv6_McastAddr, gmac);
              	#ifdef  DBG_ICMPv6
              	printk("MCAST_IPV6Addr:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
                	icmpv6_McastAddr[0],icmpv6_McastAddr[1],icmpv6_McastAddr[2],icmpv6_McastAddr[3],
                  	icmpv6_McastAddr[4],icmpv6_McastAddr[5],icmpv6_McastAddr[6],icmpv6_McastAddr[7],
                  	icmpv6_McastAddr[8],icmpv6_McastAddr[9],icmpv6_McastAddr[10],icmpv6_McastAddr[11],
                   	icmpv6_McastAddr[12],icmpv6_McastAddr[13],icmpv6_McastAddr[14],icmpv6_McastAddr[15]);
             	printk("group_mac [%02x:%02x:%02x:%02x:%02x:%02x] \n",
                  	gmac[0],gmac[1],gmac[2],
                	gmac[3],gmac[4],gmac[5]);
               	printk("grec_type = %d, src_num = %d\n", mldv2grec->grec_type, mldv2grec->grec_nsrcs);
           		#endif
             	if(((mldv2grec->grec_type== MLD2_CHANGE_TO_INCLUDE) || (mldv2grec->grec_type == MLD2_MODE_IS_INCLUDE))&& (ntohs(mldv2grec->grec_nsrcs)==0))
             	{
               		return 2; /* leave and delete it */
                }
              	else if((mldv2grec->grec_type == MLD2_CHANGE_TO_EXCLUDE) ||
            			(mldv2grec->grec_type == MLD2_MODE_IS_EXCLUDE) ||
                       	(mldv2grec->grec_type == MLD2_ALLOW_NEW_SOURCES))
             	{
                	return 1;
            	}
             	else
               	{
                  	return 1; //unknown mld report is regarded as join
         		}
               	return -1;
           	}
         	if(i>=(ntohs(mldv2report->mld2r_ngrec)-1))
        	{
           		*moreFlag=0;
            	return -1;
         	}
     	}
		else if(icmpv6h->icmp6_type == ICMPV6_MGM_REDUCTION)
		{
        	icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 );
         	#ifdef  DBG_ICMPv6
         	printk("Type: 0x%x (Multicast listener done ) \n",icmpv6h->icmp6_type);
          	#endif
         	if(icmpv6_McastAddr[0] != 0xFF)
            	return -1;
           	CIPV6toMac(icmpv6_McastAddr, gmac);
          	#ifdef  DBG_ICMPv6
        	printk("MCAST_IPV6Addr:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
           		icmpv6_McastAddr[0],icmpv6_McastAddr[1],icmpv6_McastAddr[2],icmpv6_McastAddr[3],
            	icmpv6_McastAddr[4],icmpv6_McastAddr[5],icmpv6_McastAddr[6],icmpv6_McastAddr[7],
               	icmpv6_McastAddr[8],icmpv6_McastAddr[9],icmpv6_McastAddr[10],icmpv6_McastAddr[11],
               	icmpv6_McastAddr[12],icmpv6_McastAddr[13],icmpv6_McastAddr[14],icmpv6_McastAddr[15]);
       		printk("group_mac [%02x:%02x:%02x:%02x:%02x:%02x] \n",
             	gmac[0],gmac[1],gmac[2],
               	gmac[3],gmac[4],gmac[5]);
          	#endif
          	return 2;//del
     	}
   		else
		{
        	#ifdef  DBG_ICMPv6
          	printk("Type: 0x%x (unknow type)\n",icmpv6h->icmp6_type);
           	#endif
          	return -1;
       	}
	}
  	else
	{
		return -1;//not icmpv6 type
	}
 	return -1;
}
#endif
#endif	//end of IPV6_MCAST_TO_UNICAST
void ConvertMulticatIPtoMacAddr(__u32 group, unsigned char *gmac)
{
	__u32 u32tmp, tmp;
	int i;
	u32tmp = group & 0x007FFFFF;
	gmac[0]=0x01; gmac[1]=0x00; gmac[2]=0x5e;
	for (i=5; i>=3; i--) {
		tmp=u32tmp&0xFF;
		gmac[i]=tmp;
		u32tmp >>= 8;
	}
}
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag, int type)
#else
static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag)
#endif
{
        struct iphdr *iph;
	__u8 hdrlen;
	struct igmphdr *igmph;
	int i;
	unsigned int groupAddr=0;// add  for fit igmp v3
	*moreFlag=0;
#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
	if(type == PPPOE_IGMP)
		iph = rtl_get_pppoe_ipv4_hdr(skb->data, 0, skb->protocol);
	else	
#endif
	iph=(struct iphdr *)skb_network_header(skb);
	hdrlen = iph->ihl << 2;
	if ((iph->version != 4) &&  (hdrlen < 20))
		return -1;
	if (ip_fast_csum((u8 *)iph, iph->ihl) != 0)
		return -1;
	{ /* check the length */
		__u32 len = ntohs(iph->tot_len);
		if (skb->len < len || len < hdrlen)
			return -1; 
	}
	igmph = (struct igmphdr *)((u8*)iph+hdrlen);
	if ((igmph->type==IGMP_HOST_MEMBERSHIP_REPORT) ||
	    (igmph->type==IGMPV2_HOST_MEMBERSHIP_REPORT)) 
	{
		groupAddr = ntohl(igmph->group);
		if(!IN_MULTICAST(groupAddr))
		{			
				return -1;
		}
		ConvertMulticatIPtoMacAddr(groupAddr, gmac);
		return 1; /* report and add it */
	}
	else if (igmph->type==IGMPV3_HOST_MEMBERSHIP_REPORT)	{ 
		struct igmpv3_report *igmpv3report=(struct igmpv3_report * )igmph;
		struct igmpv3_grec	*igmpv3grec=NULL; 
		if(*gIndex>=ntohs(igmpv3report->ngrec))
		{
			*moreFlag=0;
			return -1;
		}
		for(i=0;i<ntohs(igmpv3report->ngrec);i++)
		{
			if(i==0)
			{
				igmpv3grec = (struct igmpv3_grec *)(&(igmpv3report->grec)); /*first igmp group record*/
			}
			else
			{
				igmpv3grec=(struct igmpv3_grec *)((unsigned char*)igmpv3grec+8+ntohs(igmpv3grec->grec_nsrcs)*4+(igmpv3grec->grec_auxwords)*4);
			}
			if(i!=*gIndex)
			{	
				continue;
			}
			if(i==(ntohs(igmpv3report->ngrec)-1))
			{
				*moreFlag=0;
			}
			else
			{
				*moreFlag=1;
			}
			*gIndex=*gIndex+1;	
			groupAddr= ntohl(igmpv3grec->grec_mca);
			if(!IN_MULTICAST(groupAddr))
			{			
				return -1;
			}
			ConvertMulticatIPtoMacAddr(groupAddr, gmac);
			if(((igmpv3grec->grec_type == IGMPV3_CHANGE_TO_INCLUDE) || (igmpv3grec->grec_type == IGMPV3_MODE_IS_INCLUDE))&& (ntohs(igmpv3grec->grec_nsrcs)==0))
			{	
				return 2; /* leave and delete it */	
			}
			else if((igmpv3grec->grec_type == IGMPV3_CHANGE_TO_EXCLUDE) ||
				(igmpv3grec->grec_type == IGMPV3_MODE_IS_EXCLUDE) ||
				(igmpv3grec->grec_type == IGMPV3_ALLOW_NEW_SOURCES))
			{
				return 1;
			}
			else
			{
				return 1; //unknown igmp report is regarded as join
			}
			return -1;
		}
		if(i>=(ntohs(igmpv3report->ngrec)-1))
		{
			*moreFlag=0;
			return -1;
		}
	}
	else if (igmph->type==IGMP_HOST_LEAVE_MESSAGE){
		groupAddr = ntohl(igmph->group);
		if(!IN_MULTICAST(groupAddr))
		{			
				return -1;
		}
		ConvertMulticatIPtoMacAddr(groupAddr, gmac);
		return 2; /* leave and delete it */
	}	
	return -1;
}
extern int chk_igmp_ext_entry(struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac);
extern void add_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb , unsigned char *srcMac , unsigned char portComeIn);
extern void update_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn);
extern void del_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn , unsigned char expireFlag);
static void br_update_igmp_snoop_fdb(unsigned char op, struct net_bridge *br, struct net_bridge_port *p, unsigned char *dest 
										,struct sk_buff *skb)
{
    struct net_bridge_fdb_entry *dst;
    unsigned char *src;
    unsigned short del_group_src=0;
    unsigned char port_comein;
    int tt1;
    char str_devName[16] = {0};
	u16 vid = 0;
#if defined (MCAST_TO_UNICAST)
    struct net_device *dev = NULL; 
#endif
    br_vlan_get_tag(skb, &vid);
#if defined (MCAST_TO_UNICAST)
    if(!dest)	return;
    if( !MULTICAST_MAC(dest)
        #if defined (IPV6_MCAST_TO_UNICAST)
        && !IPV6_MULTICAST_MAC(dest)
        #endif	
    )
    { 
    	return; 
    }
#endif
#if defined( CONFIG_RTL_HARDWARE_MULTICAST) || defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
    if(skb->srcPort!=0xFFFF)
    {
        port_comein = 1<<skb->srcPort;
    }
    else
    {
        port_comein=0x80;
    }
#else
    if(p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_LAN_P0_DEV_NAME, 4))
    {
        port_comein = 0x01;
    }
    if(p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4))
    {
        port_comein=0x80;
    }
#endif
    src=(unsigned char*)(skb_mac_header(skb)+ETH_ALEN);
    dst = __br_fdb_get(br, dest,vid);
	
    if (op == 1) /* add */
    {	
        
        if(dst == NULL) {
            DEBUG_PRINT("insert one fdb entry\n");
			
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
				br_fdb_create(br, p, dest, vid);
			#else
				//rtnl_lock();
            	br_fdb_insert(br, p, dest,vid);
			    // rtnl_unlock();
			#endif
            dst = __br_fdb_get(br, dest,vid);
            if(dst !=NULL)
            {
                dst->igmpFlag=1;
                dst->is_local=0;
                dst->portlist = port_comein; 
                dst->group_src = dst->group_src | (1 << p->port_no);
            }
        }
        if(dst) {
            dst->group_src = dst->group_src | (1 << p->port_no);
            dst->updated = jiffies;
            tt1 = chk_igmp_ext_entry(dst , src); 
            if(tt1 == 0)
            {
                add_igmp_ext_entry(dst , src , port_comein);									
            }
            else
            {
                update_igmp_ext_entry(dst , src , port_comein);
            }	
            #if 0//defined (MCAST_TO_UNICAST)
            if (p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4)) 
            { 
                dst->portlist |= 0x80;
                port_comein = 0x80;
				#ifdef CONFIG_RTL_VLAN_8021Q
				int i;
				memcpy(str_devName, p->dev->name, 16); 
				for(i=0; i<16; i++)
					if(str_devName[i] == '.')
						str_devName[i] = '\0';
                dev = __dev_get_by_name(&init_net, str_devName);
				#else
                dev=p->dev;
				#endif
                if (dev) 
                {		
                    unsigned char StaMacAndGroup[20];
                    memcpy(StaMacAndGroup, dest, 6);
                    memcpy(StaMacAndGroup+6, src, 6);	
                    #if defined(CONFIG_COMPAT_NET_DEV_OPS)
                    if (dev->do_ioctl != NULL) 
                    {
                        dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B80);
                        DEBUG_PRINT("... add to wlan mcast table:  DA:%02x:%02x:%02x:%02x:%02x:%02x ; SA:%02x:%02x:%02x:%02x:%02x:%02x\n", 
                                StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
                                StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);					
                    }
                    #else
                    if (dev->netdev_ops->ndo_do_ioctl != NULL) 
                    {
                        dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B80);
                        DEBUG_PRINT("... add to wlan mcast table:  DA:%02x:%02x:%02x:%02x:%02x:%02x ; SA:%02x:%02x:%02x:%02x:%02x:%02x\n", 
                                StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
                                StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);	
                    }
                    #endif
                }
            }
            #endif            
        }
    }
    else if (op == 2 && dst) /* delete */
    {
        DEBUG_PRINT("dst->group_src = %x change to ",dst->group_src);		
        del_group_src = ~(1 << p->port_no);
        dst->group_src = dst->group_src & del_group_src;
        DEBUG_PRINT(" %x ; p->port_no=%x \n",dst->group_src ,p->port_no);
        if (p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4)) 
        { 			
            port_comein	= 0x80;
        }
        if(dst->portlist != 0)
        {
            del_igmp_ext_entry(dst , src ,port_comein,0);
        }
    }
}
#endif // CONFIG_RTL_IGMP_SNOOPING
