/*
 *	Forwarding decision
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

#include <linux/err.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/netpoll.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#include "linux/net.h"

#ifdef CONFIG_BRIDGE_VLAN_TP
#include "br_vlan_tp.h"
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <linux/ip.h>
#include <linux/in.h>
#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
#include <linux/in6.h>
#endif
#include <linux/igmp.h>
#include <net/checksum.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl865x_multicast.h>
#endif
extern int igmpsnoopenabled;
extern unsigned int brIgmpModuleIndex;
extern unsigned int br0SwFwdPortMask;
#if defined (CONFIG_RTL_MLD_SNOOPING)
extern int mldSnoopEnabled;
#if defined (CONFIG_RTL_HW_MCAST_WIFI)
extern int hwwifiEnable;
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr, unsigned int mapPortMask);
#else
extern int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr);
#endif
#endif
#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl865x_multicast.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
#endif
#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT
extern int rtl_check_brIgmpModuleName(char *name);
extern int rtl_get_brIgmpModueIndx(struct net_bridge *br);
extern int rtl_get_brSwFwdPortMaskByIndex(int idx);
extern struct net_bridge * rtl_get_brByIndex(int index, char *name);
extern unsigned int rtl_get_brSwFwdPortMask(struct net_bridge *br);
#if defined CONFIG_RTL_ISP_MULTI_WAN_SUPPORT
extern int rtl_get_brIgmpModuleIndexbyId(int idx,char *name);
extern int rtl_get_brIgmpModuleIndexbyName(char *name, int * index);
extern unsigned int rtl_get_brSwFwdPortMaskByIndex(int index);
#endif
#endif
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
#include <net/rtl/rtl_multi_wan.h>
#endif
extern unsigned int br0SwFwdPortMask;
extern unsigned int brIgmpModuleIndex;
extern unsigned int nicIgmpModuleIndex;
#endif/*CONFIG_RTL_IGMP_SNOOPING*/
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
extern unsigned int brIgmpModuleIndex_2;
extern unsigned int br1SwFwdPortMask;
extern struct net_bridge *bridge1;
#endif
#if	defined(CONFIG_RTL_819X)
#include <net/rtl/features/rtl_ps_hooks.h>
#include <net/rtl/features/rtl_ps_log.h>
#endif
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
#if defined(CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl865x_multicast.h>
#include <net/rtl/rtl_nic.h>
#endif/*CONFIG_RTL_HARDWARE_MULTICAST*/
#endif/*CONFIG_BRIDGE_IGMP_SNOOPING*/
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
extern int rtl_is_lan_dev(struct net_device *dev);
#endif
#if !defined(CONFIG_RTL_IGMP_SNOOPING)
static int deliver_clone(const struct net_bridge_port *prev,
			 struct sk_buff *skb,
			 void (*__packet_hook)(const struct net_bridge_port *p,
					       struct sk_buff *skb));
#endif

#if defined(CONFIG_RTL_DNS_TRAP)
#include <net/rtl/rtl_dnstrap.h>

int skb_from_eth(struct sk_buff *skb)
{
       if(!strncmp(skb->dev->name,"eth",strlen("eth"))) {
              //printk("skb_from_eth return 1\n");
              return 1;
       }             
       //printk("skb_from_eth return 0\n");
       return 0;
}

#endif
#if defined(CONFIG_RTL_HTTP_REDIRECT)
extern int is_http_resp_packet(struct sk_buff *skb);
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
extern int linux_vlan_enable;
#endif
/* Don't forward packets to originating port or forwarding disabled */
static inline int should_deliver(const struct net_bridge_port *p,
				 const struct sk_buff *skb)
{
#if defined(CONFIG_RTL_DNS_TRAP)
	 /* is_recaped_dns_packet condition for wlan or eth access dut's dns, 
	   * is_dns_packet && skb_from_eth for ethernet dev include multiple phy port
	   */
	 if (skb && (is_recaped_dns_packet((struct sk_buff *)skb) || (is_dns_packet(skb) && skb_from_eth(skb)))) {
		return 1;
	}
#endif
#if defined(CONFIG_RTL_HTTP_REDIRECT)
	if(skb && is_http_resp_packet((struct sk_buff*)skb)){
		return 1;
	}
#endif

#ifdef CONFIG_BRIDGE_VLAN_TP
	if(!br_vlan_forward_hook(p, skb))
		return 0;
#endif

	struct net_bridge_vlan_group *vg;

	vg = nbp_vlan_group_rcu(p);
	return ((p->flags & BR_HAIRPIN_MODE) || skb->dev != p->dev) &&
		br_allowed_egress(vg, skb) && p->state == BR_STATE_FORWARDING;
}

int br_dev_queue_push_xmit(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct vlan_dev_priv *vlan;
	struct vlan_ethhdr *veth;
	u16 vlan_tci;

	skb_push(skb, ETH_HLEN);
	if (!is_skb_forwardable(skb->dev, skb))
		goto drop;

	br_drop_fake_rtable(skb);
	skb_sender_cpu_clear(skb);

	if (skb->ip_summed == CHECKSUM_PARTIAL &&
	    (skb->protocol == htons(ETH_P_8021Q) ||
	     skb->protocol == htons(ETH_P_8021AD))) {
		int depth;

		if (!__vlan_get_protocol(skb, skb->protocol, &depth))
			goto drop;

		skb_set_network_header(skb, depth);
	}
	if (skb->protocol != htons(ETH_P_8021Q) && skb->protocol != htons(ETH_P_8021AD)) {
		vlan = vlan_dev_priv(skb->dev);
		veth = (struct vlan_ethhdr *)(skb->data);
		if ((skb->dev->priv_flags & IFF_802_1Q_VLAN) && (vlan->flags & VLAN_FLAG_REORDER_HDR)) {
			vlan_tci = vlan->vlan_id;
			vlan_tci |= vlan_dev_get_egress_qos_mask(skb->dev, skb->priority);
			__vlan_hwaccel_put_tag(skb, vlan->vlan_proto, vlan_tci);
			__vlan_hwaccel_push_inside(skb);
			skb->dev = vlan->real_dev;
		}
	}
	dev_queue_xmit(skb);

	return 0;

drop:
	kfree_skb(skb);
	return 0;
}
EXPORT_SYMBOL_GPL(br_dev_queue_push_xmit);

int br_forward_finish(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	return NF_HOOK(NFPROTO_BRIDGE, NF_BR_POST_ROUTING,
		       net, sk, skb, NULL, skb->dev,
		       br_dev_queue_push_xmit);

}
EXPORT_SYMBOL_GPL(br_forward_finish);

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_bridge_vlan_group *vg;

	vg = nbp_vlan_group_rcu(to);
	skb = br_handle_vlan(to->br, vg, skb);
	if (!skb)
		return;

	skb->dev = to->dev;

	if (unlikely(netpoll_tx_running(to->br->dev))) {
		skb_push(skb, ETH_HLEN);
		if (!is_skb_forwardable(skb->dev, skb))
			kfree_skb(skb);
		else
			br_netpoll_send_skb(to, skb);
		return;
	}

	NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_OUT,
		dev_net(skb->dev), NULL, skb,NULL, skb->dev,
		br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_bridge_vlan_group *vg;
	struct net_device *indev;

	if (skb_warn_if_lro(skb)) {
		kfree_skb(skb);
		return;
	}

	vg = nbp_vlan_group_rcu(to);
	skb = br_handle_vlan(to->br, vg, skb);
	if (!skb)
		return;

	indev = skb->dev;
	skb->dev = to->dev;
	skb_forward_csum(skb);

	NF_HOOK(NFPROTO_BRIDGE, NF_BR_FORWARD,
		dev_net(indev), NULL, skb, indev, skb->dev,
		br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (to && should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}
EXPORT_SYMBOL_GPL(br_deliver);

/* called with rcu_read_lock */
#if defined(CONFIG_RTL_IGMP_SNOOPING)
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}
	kfree_skb(skb);
}
#else
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb, struct sk_buff *skb0)
{
	if (to && should_deliver(to, skb) 
#ifdef CONFIG_TP_BRIDGE
		&& !(to->flags & BR_ISOLATE_MODE
#endif
	) {
		if (skb0)
			deliver_clone(to, skb, __br_forward);
		else
			__br_forward(to, skb);
		return;
	}

	if (!skb0)
		kfree_skb(skb);
}

static int deliver_clone(const struct net_bridge_port *prev,
			 struct sk_buff *skb,
			 void (*__packet_hook)(const struct net_bridge_port *p,
					       struct sk_buff *skb))
{
	struct net_device *dev = BR_INPUT_SKB_CB(skb)->brdev;

	skb = skb_clone(skb, GFP_ATOMIC);
	if (!skb) {
		dev->stats.tx_dropped++;
		return -ENOMEM;
	}

	__packet_hook(prev, skb);
	return 0;
}
#endif

#ifndef CONFIG_RTL_IGMP_SNOOPING
static struct net_bridge_port *maybe_deliver(
	struct net_bridge_port *prev, struct net_bridge_port *p,
	struct sk_buff *skb,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
	int err;

	if (!should_deliver(p, skb))
		return prev;

	if (!prev)
		goto out;

	err = deliver_clone(prev, skb, __packet_hook);
	if (err)
		return ERR_PTR(err);

out:
	return p;
}
#endif

extern int multi_phy_vir_wan_enable;
#if defined(CONFIG_RTL_IGMP_SNOOPING)
static void br_flood(struct net_bridge *br, struct sk_buff *skb,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb)
#ifdef CONFIG_TP_BRIDGE
			      , bool forward
#endif
			      )
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	prev = NULL;

	list_for_each_entry_rcu(p, &br->port_list, list) {
#ifdef CONFIG_TP_BRIDGE
		if (forward && (p->flags & BR_ISOLATE_MODE))
			continue;
#endif

#ifndef CONFIG_RTK_INBAND_HOST_HACK
		if (should_deliver(p, skb)) {
#else
		if ((should_deliver(p, skb)) && (inband_deliver_check(p, skb))) {
#endif
			/*patch for wan/lan receive duplicate unknown unicast/broadcast packet when pppoe/ipv6 passthrough enable*/
			/*except the packet dmac=33:33:xx:xx:xx:xx*/
			if((strcmp(skb->dev->name,"peth0")==0)&&(!(dest[0]==0x33&&dest[1]==0x33)))
			{
				 if((strncmp(p->dev->name,"eth",3)==0))
				 {
					continue;
				 }
			}

#if !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)

			/*patch for lan->wan duplicat packet(dmac=33:33:ff:xx:xx:xx) when pppoe/ipv6 passthrough enable*/
			if(((strcmp(skb->dev->name,"eth0")==0)
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
			||(strcmp(skb->dev->name,"eth2")==0)||(strcmp(skb->dev->name,"eth3")==0)||(strcmp(skb->dev->name,"eth4")==0)
#endif
			)&&(
			#if defined(CONFIG_RTK_BRIDGE_VLAN_SUPPORT)
				(rtk_vlan_support_enable==0)&&
			#endif
				(dest[0]==0x33)&&(dest[1]==0x33)&&(dest[2]==0xff)))
			{
				 if((strncmp(p->dev->name,"peth0",5)==0))
				 {
					continue;
				 }
			}
#endif			
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#if defined(CONFIG_RTK_VLAN_SUPPORT)
			if (rtk_vlan_support_enable == 0) 
#endif
			{
				if (is_broadcast_ether_addr(dest) || is_unicast_ether_addr(dest)) {
					if ((rtl_is_lan_dev(skb->dev)==TRUE)&&(rtl_is_lan_dev(p->dev)==TRUE))
						continue;
				}
			}
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
		   if (is_broadcast_ether_addr(dest) || is_unicast_ether_addr(dest)){
				if (((!strncmp(skb->dev->name, "eth", strlen("eth")))&&(!strncmp(p->dev->name, ALIASNAME_ETH_WAN, strlen(ALIASNAME_ETH_WAN))))
					||((!strncmp(p->dev->name, "eth", strlen("eth")))&&(!strncmp(skb->dev->name, ALIASNAME_ETH_WAN, strlen(ALIASNAME_ETH_WAN))))){
					//eth bridge with wan, receive duplicate broadcast/unknown in wan side. bridge wan always named as eth1.x??? 
					continue;
				}
			}
#endif
#if defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_MULTI_PHY_VIR_WAN)
         	if(multi_phy_vir_wan_enable)
		  	{
				if (is_broadcast_ether_addr(dest) || is_unicast_ether_addr(dest)) {
					if ((strncmp(skb->dev->name, "eth", 3)==0)&&(strncmp(p->dev->name, "eth", 3)==0))
									continue;
				}
			}
#endif

			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					LOG_MEM_ERROR("%s(%d) skb clone failed, drop it\n", __FUNCTION__, __LINE__);
					br->dev->stats.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}
#else
/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb,
		     struct sk_buff *skb0,
		     void (*__packet_hook)(const struct net_bridge_port *p,
					   struct sk_buff *skb),
		     bool unicast
#ifdef CONFIG_TP_BRIDGE
		     ,bool forward
#endif
		     )
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	prev = NULL;

	list_for_each_entry_rcu(p, &br->port_list, list) {
#ifdef CONFIG_TP_BRIDGE
		if (forward && (p->flags & BR_ISOLATE_MODE))
			continue;
#endif
#if 0
		/* Do not flood unicast traffic to ports that turn it off */
		if (unicast && !(p->flags & BR_FLOOD))
			continue;

		/* Do not flood to ports that enable proxy ARP */
		if (p->flags & BR_PROXYARP)
			continue;
		if ((p->flags & BR_PROXYARP_WIFI) &&
		    BR_INPUT_SKB_CB(skb)->proxyarp_replied)
			continue;
#endif
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if (rtk_vlan_support_enable == 0) 
#endif
	{
		const unsigned char *dest = eth_hdr(skb)->h_dest;
		if (dest && (is_broadcast_ether_addr(dest) || is_unicast_ether_addr(dest))) {
			if ((rtl_is_lan_dev(skb->dev)==TRUE)&&(rtl_is_lan_dev(p->dev)==TRUE))
				continue;
		}
	}
#endif

		prev = maybe_deliver(prev, p, skb, __packet_hook);
		if (IS_ERR(prev))
			goto out;
	}

	if (!prev)
		goto out;

	if (skb0)
		deliver_clone(prev, skb, __packet_hook);
	else
		__packet_hook(prev, skb);
	return;

out:
	if (!skb0)
		kfree_skb(skb);
}

#endif/*CONFIG_RTL_IGMP_SNOOPING*/




/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, bool unicast)
{
#if defined(CONFIG_RTL_IGMP_SNOOPING)
#ifdef CONFIG_TP_BRIDGE
		br_flood(br, skb, __br_deliver, false);
#else
		br_flood(br, skb, __br_deliver);
#endif
#else
#ifdef CONFIG_TP_BRIDGE
		br_flood(br, skb, NULL, __br_deliver, unicast, false);
#else
		br_flood(br, skb, NULL, __br_deliver, unicast);
#endif
#endif
}

/* called under bridge lock */
#if defined(CONFIG_RTL_IGMP_SNOOPING)
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb)
{
#ifdef CONFIG_TP_BRIDGE
	br_flood(br, skb, __br_forward, true);
#else
	br_flood(br, skb, __br_forward);
#endif
}
#else
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb,
		      struct sk_buff *skb2, bool unicast)
{
#ifdef CONFIG_TP_BRIDGE
	br_flood(br, skb, skb2, __br_forward, unicast, true);
#else
	br_flood(br, skb, skb2, __br_forward, unicast);
#endif
}
#endif/*CONFIG_RTL_IGMP_SNOOPING*/
#ifdef CONFIG_TP_BRIDGE
EXPORT_SYMBOL_GPL(br_forward);
#endif

#if defined( CONFIG_RTL865X_HARDWARE_MULTICAST) || defined(CONFIG_RTL865X_LANPORT_RESTRICTION) || defined(CONFIG_RTL_HARDWARE_MULTICAST)
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
extern int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr, unsigned int mapPortMask);
#else
extern int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr);
#endif
#endif
#if defined(CONFIG_RTL_HARDWARE_MULTICAST)
extern int rtl865x_curOpMode;
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
extern int32 rtl865x_getMcastFwdInfo(struct net_bridge_mdb_entry *mdst,
									  		struct multicastFwdInfo *mcastFwdInfo);
int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br,
													 unsigned int srcPort,
													 unsigned int srcVlanId,
													 unsigned int srcIpAddr,
													 struct net_bridge_mdb_entry *mdst)
{
	int ret;
	unsigned int tagged_portmask=0;
	rtl865x_tblDrv_mCast_t * existMulticastEntry;
	struct multicastFwdInfo mcastFwdInfo;
	rtl865x_mcast_fwd_descriptor_t fwdDesc;
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)!=0)
	{
		return -1;
	}
	ret=rtl865x_getMcastFwdInfo(mdst,&mcastFwdInfo);
	if(ret!=SUCCESS)
	{
		return -1;
	}
	memset(&fwdDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	strcpy(fwdDesc.netifName,RTL_PS_BR0_DEV_NAME);
	fwdDesc.dip=mdst->addr.u.ip4;
	fwdDesc.fwdPortMask=mcastFwdInfo.fwdPortMask;
	fwdDesc.toCpu=mcastFwdInfo.toCpu;
	#ifdef HW_MULTICAST_DBG
	printk("%s:%d,srcPort is %d,srcAddr is 0x%x,srcVlanId is %d,destIpAddr is 0x%x\n",__FUNCTION__,__LINE__,srcPort,srcIpAddr,srcVlanId,mdst->addr.u.ip4);
	#endif
	existMulticastEntry=rtl865x_findMCastEntry(fwdDesc.dip,srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort);
	if(existMulticastEntry!=NULL)
	{
		#ifdef HW_MULTICAST_DBG
		printk("%s multicast already in cache\n",__FUNCTION__);
		#endif
		return 0;
	}
	if(mcastFwdInfo.fwdPortMask==0)
	{
		rtl865x_blockMulticastFlow(srcVlanId, srcPort, srcIpAddr, fwdDesc.dip);
		return 0;
	}
	fwdDesc.fwdPortMask=mcastFwdInfo.fwdPortMask & (~(1<<srcPort));
#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if(rtl_hw_vlan_ignore_tagged_mc == 1)
		tagged_portmask = rtl_hw_vlan_get_tagged_portmask();
#endif
	if((fwdDesc.fwdPortMask & tagged_portmask) == 0)
	{
		ret=rtl865x_addMulticastEntry(fwdDesc.dip,srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort,
							&fwdDesc, 1, 0, 0, 0);
	}
	return 0;
}
#if defined(CONFIG_RTL_8198C) //|| defined(CONFIG_RTL_8197F)
int rtl865x_ipv6MulticastHardwareAccelerate(struct net_bridge *br,
													 unsigned int srcPort,
													 unsigned int srcVlanId,
													 unsigned int srcIpAddr,
													 struct net_bridge_mdb_entry *mdst)
{
	int ret;
	unsigned int tagged_portmask=0;
	rtl8198c_tblDrv_mCastv6_t * existMulticastEntry;
	rtl8198c_mcast_fwd_descriptor6_t fwdDesc;
	struct multicastFwdInfo mcastFwdInfo;
	ret=rtl865x_getMcastFwdInfo(mdst,&mcastFwdInfo);
	strcpy(fwdDesc.netifName,RTL_PS_BR0_DEV_NAME);
	fwdDesc.dip=mdst->addr.u.ip6.in6_u;
	fwdDesc.fwdPortMask=mcastFwdInfo.fwdPortMask;
	fwdDesc.toCpu=mcastFwdInfo.toCpu;
	fwdDesc.dip=mdst->addr.u.ip4;
	#ifdef HW_MULTICAST_DBG
	printk("%s:%d,srcPort is %d,srcAddr is 0x%x,srcVlanId is %d,destIpAddr is 0x%x\n",__FUNCTION__,__LINE__,srcPort,srcIpAddr,srcVlanId,mdst->addr.u.ip4);
	#endif
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)!=0)
	{
		return -1;
	}
	existMulticastEntry=rtl8198C_findMCastv6Entry(fwdDesc.dip,srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort);
	if(existMulticastEntry!=NULL)
	{
		#ifdef HW_MULTICAST_DBG
		printk("%s multicast already in cache\n",__FUNCTION__);
		#endif
		return 0;
	}
	if(fwdDesc.fwdPortMask==0)
	{
		rtl8198C_blockMulticastv6Flow(srcVlanId,srcPort,srcIpAddr,fwdDesc.dip);
		return 0;
	}
	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)==0)
	{
		memset(&fwdDesc, 0, sizeof(rtl8198c_mcast_fwd_descriptor6_t ));
		strcpy(fwdDesc.netifName,"eth*");
		ret=rtl865x_getFwdDescriptor(br, &fwdDesc,ip)
	}
	else if(strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)==0)
	{
		memset(&fwdDesc, 0, sizeof(rtl8198c_mcast_fwd_descriptor6_t ));
		ret=rtl865x_getFwdDescriptor(br,&fwdDesc,ip);
	}
	#else
	#endif
	if(ret!=0)
	{
		return -1;
	}
	else
	{
		fwdDesc.fwdPortMask=fwdDesc.fwdPortMask & (~(1<<srcPort));
	}
#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if(rtl_hw_vlan_ignore_tagged_mc == 1)
		tagged_portmask = rtl_hw_vlan_get_tagged_portmask();
#endif
	if((fwdDesc.fwdPortMask & tagged_portmask) == 0)
	{
		ret=rtl8198C_addMulticastv6Entry(fwdDesc.dip,srcIpAddr,(unsigned short)srcVlanId,(unsigned short)srcPort,
										 &fwdDesc,1,0,0,0);
	}
	return 0;
}
#endif/*CONFIG_RTL_8198C*/
#endif/*CONFIG_BRIDGE_IGMP_SNOOPING*/
#endif/*CONFIG_RTL_HARDWARE_MULTICAST*/
#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
/* called with rcu_read_lock */
static void br_multicast_flood(struct net_bridge_mdb_entry *mdst,
			       struct sk_buff *skb, struct sk_buff *skb0,
			       void (*__packet_hook)(
					const struct net_bridge_port *p,
					struct sk_buff *skb))
{
	struct net_device *dev = BR_INPUT_SKB_CB(skb)->brdev;
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_port *prev = NULL;
	struct net_bridge_port_group *p;
	struct hlist_node *rp;

	rp = rcu_dereference(hlist_first_rcu(&br->router_list));
	p = mdst ? rcu_dereference(mdst->ports) : NULL;
	while (p || rp) {
		struct net_bridge_port *port, *lport, *rport;

		lport = p ? p->port : NULL;
		rport = rp ? hlist_entry(rp, struct net_bridge_port, rlist) :
			     NULL;

		port = (unsigned long)lport > (unsigned long)rport ?
		       lport : rport;

		prev = maybe_deliver(prev, port, skb, __packet_hook);
		if (IS_ERR(prev))
			goto out;

		if ((unsigned long)lport >= (unsigned long)port)
			p = rcu_dereference(p->next);
		if ((unsigned long)rport >= (unsigned long)port)
			rp = rcu_dereference(hlist_next_rcu(rp));
	}

	if (!prev)
		goto out;

	if (skb0)
		deliver_clone(prev, skb, __packet_hook);
	else
		__packet_hook(prev, skb);
	return;

out:
	if (!skb0)
		kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_multicast_deliver(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb)
{
	br_multicast_flood(mdst, skb, NULL, __br_deliver);
}

/* called with rcu_read_lock */
void br_multicast_forward(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb, struct sk_buff *skb2)
{
	br_multicast_flood(mdst, skb, skb2, __br_forward);
}
#endif
#if defined(CONFIG_RTL_IGMP_SNOOPING)
int bitmask_to_id(unsigned char val)
{
	int i;
	for (i=0; i<8; i++) {
		if (val & (1 <<i))
			break;
	}
	if(i>=8)
	{
		i=7;
	}
	return (i);
}
static void br_multicast(struct net_bridge *br, unsigned int fwdPortMask, struct sk_buff *skb, int clone,
		  void (*__packet_hook)(const struct net_bridge_port *p, struct sk_buff *skb))
{
	struct net_bridge_port *prev;
	struct net_bridge_port *p, *n;
	unsigned int port_bitmask=0;
        if (clone) {
                struct sk_buff *skb2;
                if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			LOG_MEM_ERROR("%s(%d) skb clone failed, drop it\n", __FUNCTION__, __LINE__);
                        br->dev->stats.tx_dropped++;
                        return;
                }
                skb = skb2;
        }
	prev = NULL;
	list_for_each_entry_safe(p, n, &br->port_list, list) {
		port_bitmask = (1 << p->port_no);
                if ((port_bitmask & fwdPortMask) && should_deliver(p, skb)) {
                        if (prev != NULL) {
                                struct sk_buff *skb2;
                                if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					LOG_MEM_ERROR("%s(%d) skb clone failed, drop it\n", __FUNCTION__, __LINE__);
                                        br->dev->stats.tx_dropped++;
                                        kfree_skb(skb);
                                        return;
                                }
                                __packet_hook(prev, skb2);
                        }
                        prev = p;
                }
	}
        if (prev != NULL) {
                __packet_hook(prev, skb);
                return;
        }
	kfree_skb(skb);
}
void br_multicast_deliver(struct net_bridge *br, unsigned int fwdPortMask, struct sk_buff *skb, int clone)
{
	br_multicast(br, fwdPortMask, skb, clone, __br_deliver);
}
void br_multicast_forward(struct net_bridge *br, unsigned int fwdPortMask, struct sk_buff *skb, int clone)
{
	br_multicast(br, fwdPortMask, skb, clone, __br_forward);
}
extern struct net_bridge *bridge0;
extern int ipMulticastFastFwd;
extern int needCheckMfc;
#if defined (CONFIG_IP_MROUTE)
#if defined (CONFIG_RTL_IGMP_PROXY)
extern int rtl865x_checkMfcCache(struct net *net,struct net_device *dev,__be32 origin,__be32 mcastgrp);
#endif
#endif
#if defined(CONFIG_RTL_MLD_SNOOPING)
extern int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h);
#endif
extern int rtl865x_curOpMode;
#define MAX_UNKNOWN_MULTICAST_NUM 16
#define BLOCK_UNKNOWN_MULTICAST 1
unsigned int maxUnknownMcastPPS=1500;
unsigned int chkUnknownMcastEnable=1;
struct rtl865x_unKnownMCastRecord
{
	unsigned int groupAddr;
	unsigned long lastJiffies;
	unsigned long pktCnt;
	unsigned int valid;
};
struct rtl865x_unKnownMCastRecord unKnownMCastRecord[MAX_UNKNOWN_MULTICAST_NUM];
int rtl865x_checkUnknownMCastLoading(struct rtl_multicastDataInfo *mCastInfo)
{
	int i;
	if (chkUnknownMcastEnable==0)
		return 0;
	if(mCastInfo==NULL)
	{
		return 0;
	}
	if(rtl_check_ReservedMCastAddr(mCastInfo->groupAddr[0])==SUCCESS)
	{
		return 0;
	}
	for(i=0; i<MAX_UNKNOWN_MULTICAST_NUM; i++)
	{
		if((unKnownMCastRecord[i].valid==1) && (unKnownMCastRecord[i].groupAddr==mCastInfo->groupAddr[0]))
		{
			break;
		}
	}
	if(i==MAX_UNKNOWN_MULTICAST_NUM)
	{
		for(i=0; i<MAX_UNKNOWN_MULTICAST_NUM; i++)
		{
			if(unKnownMCastRecord[i].valid!=1)
			{
				break;
			}
		}
	}
	if(i==MAX_UNKNOWN_MULTICAST_NUM)
	{
		for(i=0; i<MAX_UNKNOWN_MULTICAST_NUM; i++)
		{
			if(	time_before(unKnownMCastRecord[i].lastJiffies+HZ,jiffies)
				|| time_after(unKnownMCastRecord[i].lastJiffies,jiffies+HZ)	)
			{
				break;
			}
		}
	}
	if(i==MAX_UNKNOWN_MULTICAST_NUM)
	{
		return 0;
	}
	unKnownMCastRecord[i].groupAddr=mCastInfo->groupAddr[0];
	unKnownMCastRecord[i].valid=1;
	if(time_after(unKnownMCastRecord[i].lastJiffies+HZ,jiffies))
	{
		unKnownMCastRecord[i].pktCnt++;
	}
	else
	{
		unKnownMCastRecord[i].lastJiffies=jiffies;
		unKnownMCastRecord[i].pktCnt=0;
	}
	if(unKnownMCastRecord[i].pktCnt>maxUnknownMcastPPS)
	{
		return BLOCK_UNKNOWN_MULTICAST;
	}
	return 0;
}
int rtl865x_ipMulticastFastFwd(struct sk_buff *skb)
{
	const unsigned char *dest = NULL;
	unsigned char *ptr;
	struct iphdr *iph=NULL;
	unsigned char proto=0;
	unsigned char reserved=0;
	int ret=-1;
	struct net_bridge_port *prev;
	struct net_bridge_port *p, *n;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	struct sk_buff *skb2;
	unsigned int port_bitmask=0;
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	#endif
	unsigned int fwdCnt;
#if	defined (CONFIG_RTL_IGMP_PROXY)
	struct net_device *dev=skb->dev;
#endif
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	struct net_bridge *bridge = bridge0;
	unsigned int brSwFwdPortMask = br0SwFwdPortMask;
#endif	
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	unsigned int srcPort=skb->srcPort;
	unsigned int srcVlanId=skb->srcVlanId;
	rtl865x_tblDrv_mCast_t *existMulticastEntry=NULL;
#endif
#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
	unsigned int mapPortMask = 0xFFFFFFFF;
#endif
	int i, valid_br_num = 0, br_index = 0, br_find = 0;
	struct net_bridge *br;
	unsigned char br_name[16]={0};
	unsigned int igmpModuleIndex=0xFFFFFFFF;
	unsigned int swFwdPortMask =0xFFFFFFFF;
#if defined (CONFIG_RTL_IGMP_PROXY_MULTIWAN)
	struct smux_dev_info *dev_info = NULL;
	char dev_name[IFNAMSIZ]= {0};
#endif
#endif
	#if defined (CONFIG_RTL_VLAN_8021Q) && defined(CONFIG_RT_MULTIPLE_BR_SUPPORT)
	if(linux_vlan_enable)
	{
		return -1;
	}
	#endif
	if(ipMulticastFastFwd==0)
	{
		return -1;
	}
	dest=eth_hdr(skb)->h_dest;
	if((dest[0]&0x01)==0)
	{
		return -1;
	}
	if(igmpsnoopenabled==0)
	{
		return -1;
	}
	#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
	if(skb->dev==NULL)
		return -1;
	for(i=0; i<RTL_IMGP_MAX_BRMODULE; i++)
	{
		br = rtl_get_brByIndex(i, br_name);
		if(br!=NULL && (br->dev->flags & IFF_PROMISC)==0)
		{
		 	if(strncmp(skb->dev->name, br_name, 3) == 0)
				return -1;
			valid_br_num ++;
		}
	}
	if(valid_br_num == 0)
		return -1;
	#else
	if((bridge0==NULL) ||(bridge0->dev->flags & IFF_PROMISC))
	{
		return -1;
	}
	if((skb->dev==NULL) ||(strncmp(skb->dev->name,RTL_PS_BR0_DEV_NAME,3)==0))
	{
		return -1;
	}
    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if((strncmp(skb->dev->name,RTL_PS_BR1_DEV_NAME,3)==0))
	{
		return -1;
	}
	#endif
	#endif
	if  ((dest[0]==0x01) && (dest[1]==0x00) && (dest[2]==0x5e))
	{
		ptr=(unsigned char *)eth_hdr(skb)+12;
		if(*(int16 *)(ptr)==(int16)htons(0x8100))
		{
			ptr=ptr+4;
		}
		if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IP))
		{
			return -1;
		}
		iph=(struct iphdr *)(ptr+2);
	if((iph->daddr== htonl(0xEFFFFFFA))||
		(rtl_check_ReservedMCastAddr(ntohl(iph->daddr))==SUCCESS))
		{
			reserved=1;
		}
		proto =  iph->protocol;
		 if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP)||(proto ==Any_0_hop_protocl)) && (reserved ==0))
		{
			multicastDataInfo.ipVersion=4;
			multicastDataInfo.sourceIp[0]=  (unsigned int)(iph->saddr);
			multicastDataInfo.groupAddr[0]=  (unsigned int)(iph->daddr);
			multicastDataInfo.sourceIp[0] =  ntohl(multicastDataInfo.sourceIp[0]);
			multicastDataInfo.groupAddr[0]=  ntohl(multicastDataInfo.groupAddr[0]);
		#if defined (CONFIG_RTL_IGMP_PROXY_MULTIWAN)
			dev_info = SMUX_DEV_INFO(skb->from_dev);
			if((skb->dev->priv_flags & IFF_DOMAIN_WAN) && 
				dev_info && dev_info->proto == SMUX_PROTO_PPPOE &&
				dev_info->on_phy == 0)
			{
				return -1;
			}
		#endif
			#if defined (CONFIG_IP_MROUTE)
			#if defined (CONFIG_RTL_IGMP_PROXY_MULTIWAN)
			if((skb->dev->priv_flags & IFF_DOMAIN_WAN) && 
				dev_info && dev_info->proto != SMUX_PROTO_BRIDGE && needCheckMfc)
			#else
			if((((rtl865x_curOpMode==GATEWAY_MODE)&&(strncmp(skb->dev->name,RTL_PS_WAN0_DEV_NAME,4)==0))||
           		 	((rtl865x_curOpMode==WISP_MODE)&&((strncmp(skb->dev->name,"wlan0-vxd",9)==0)||
				(strncmp(skb->dev->name,"wlan1-vxd",9)==0))))&&needCheckMfc)
			#endif
			{
				#if	defined (CONFIG_RTL_IGMP_PROXY)
				if(rtl865x_checkMfcCache(&init_net,dev,iph->saddr,iph->daddr)!=0)
				#endif	
				{
					if(rtl865x_checkUnknownMCastLoading(&multicastDataInfo)==BLOCK_UNKNOWN_MULTICAST)
					{
#if (defined( CONFIG_RTL_HARDWARE_MULTICAST)&& !defined(CONFIG_RTL_L2_HW_MULTICAST_SUPPORT))|| defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
						if((skb->srcVlanId!=0) && (skb->srcPort!=0xFFFF))
						{
							existMulticastEntry=rtl865x_findMCastEntry(multicastDataInfo.groupAddr[0], multicastDataInfo.sourceIp[0], srcVlanId, srcPort);
							if(existMulticastEntry!=NULL && existMulticastEntry->inAsic)
							{
						#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING						
							#if defined CONFIG_RTL_IGMP_PROXY_MULTIWAN
								mapPortMask = dev_info->member;
							#endif						
								rtl865x_blockMulticastFlow(srcVlanId, srcPort, multicastDataInfo.sourceIp[0],multicastDataInfo.groupAddr[0], mapPortMask);
						#else
								rtl865x_blockMulticastFlow(srcVlanId, srcPort, multicastDataInfo.sourceIp[0],multicastDataInfo.groupAddr[0]);
						#endif
							}
							else
							{
								kfree_skb(skb);
								return 0;
							}
						}
						else
#endif
						{
							kfree_skb(skb);
							return 0;
						}
					}
					return -1;
				}
			}
			else{
				return -1;
			}
			#endif
			#if 0
			multicastDataInfo.ipVersion=4;
			multicastDataInfo.sourceIp[0]=  (unsigned int)(iph->saddr);
			multicastDataInfo.groupAddr[0]=  (unsigned int)(iph->daddr);
			#endif
		#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
		#if defined (CONFIG_RTL_IGMP_PROXY_MULTIWAN)
			if((skb->dev->priv_flags & IFF_DOMAIN_WAN) && dev_info && dev_info->proto != SMUX_PROTO_BRIDGE)
			{
				br_index = rtl_check_brIgmpModuleName(RTL_PS_BR0_DEV_NAME);
				if(br_index != -1)
					br = rtl_get_brByIndex(br_index,br_name);
				else
					return -1;
			}
			else if((skb->dev->priv_flags & IFF_DOMAIN_WAN) || skb->dev->priv_flags & IFF_DOMAIN_ELAN)
			{
				br_find = 0;
				for(i=0; i<RTL_IMGP_MAX_BRMODULE; i++)
				{
					igmpModuleIndex =rtl_get_brIgmpModuleIndexbyId(i, br_name);
					if(igmpModuleIndex != 0xFFFFFFFF)
					{
						br = rtl_get_brByIndex(i,br_name);
						list_for_each_entry_safe(p, n, &br->port_list, list)
						{
							if(strcmp(p->dev->name, skb->dev->name) == 0)
							{
								br_find = 1;
								break;
							}
						}
						if(br_find)
							break;
					}
				}
				if(br_find == 0)
					return -1;
			}
			else
			{
				return -1;
			}
		#elif defined(CONFIG_RTL_VLAN_8021Q)
			if(linux_vlan_enable)
			{
			}
			else
			{
				br_index = rtl_check_brIgmpModuleName(RTL_PS_BR0_DEV_NAME);
				if(br_index != -1)
				{
					br = rtl_get_brByIndex(br_index,br_name);
					mapPortMask = 0xFFFFFFFF;
				}
				else
					return -1;
			}
		#endif
			igmpModuleIndex = rtl_get_brIgmpModueIndx(br);
			swFwdPortMask = rtl_get_brSwFwdPortMask(br);
			ret= rtl_getMulticastDataFwdInfo(igmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
		#else
            #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT //fix tim
			if(!strcmp(skb->dev->name,RTL_PS_ETH_NAME_ETH2)){
					ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex_2, &multicastDataInfo, &multicastFwdInfo);
					bridge = bridge1;
					brSwFwdPortMask = br1SwFwdPortMask;
			}
			else
		    #endif
			ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
		#endif
			 if((ret!=0)||multicastFwdInfo.reservedMCast || multicastFwdInfo.unknownMCast)
			{
			#if defined CONFIG_RTL_IGMP_PROXY_MULTIWAN
				if( multicastFwdInfo.unknownMCast && 
					(skb->dev->priv_flags & IFF_DOMAIN_WAN) && 
					(dev_info && dev_info->proto != SMUX_PROTO_BRIDGE)  && 		//only block heavyloading multicast data from wan
					(rtl865x_checkUnknownMCastLoading(&multicastDataInfo)==BLOCK_UNKNOWN_MULTICAST))
			#else
				if( multicastFwdInfo.unknownMCast && 
					(strncmp(skb->dev->name,RTL_PS_WAN0_DEV_NAME,4)==0) && 		//only block heavyloading multicast data from wan
					(rtl865x_checkUnknownMCastLoading(&multicastDataInfo)==BLOCK_UNKNOWN_MULTICAST))
			#endif
				{
#if (defined( CONFIG_RTL865X_HARDWARE_MULTICAST)&& !defined(CONFIG_RTL_L2_HW_MULTICAST_SUPPORT))|| defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
					if((skb->srcVlanId!=0) && (skb->srcPort!=0xFFFF))
					{					
					#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING						
						#if defined CONFIG_RTL_IGMP_PROXY_MULTIWAN
							mapPortMask = dev_info->member;
						#endif
						rtl865x_blockMulticastFlow(srcVlanId, srcPort, multicastDataInfo.sourceIp[0],multicastDataInfo.groupAddr[0], mapPortMask);
					#else
						rtl865x_blockMulticastFlow(srcVlanId, srcPort, multicastDataInfo.sourceIp[0],multicastDataInfo.groupAddr[0]);
					#endif
					}
					else
#endif
					{
						kfree_skb(skb);
						return 0;
					}
				}
				return -1;
			}
			#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
			existMulticastEntry=rtl865x_findMCastEntry(multicastDataInfo.groupAddr[0], multicastDataInfo.sourceIp[0], srcVlanId, srcPort);
			if(	(existMulticastEntry==NULL)||
				((existMulticastEntry!=NULL) && (existMulticastEntry->inAsic)))
			{
				if((skb->srcVlanId!=0) && (skb->srcPort!=0xFFFF))
				{
			#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
					if((swFwdPortMask & multicastFwdInfo.fwdPortMask)==0)
			#else
					#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
					if( (brSwFwdPortMask & multicastFwdInfo.fwdPortMask)==0)
					#else
					if( (br0SwFwdPortMask & multicastFwdInfo.fwdPortMask)==0)
					#endif
			#endif
					{
						return -1;
					}
			#if defined (CONFIG_RTL_HW_MCAST_WIFI)
			#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
					else
					{
						if(linux_vlan_enable)
						{
						}
						else
						{
							if(hwwifiEnable&&( ((~swFwdPortMask) & multicastFwdInfo.fwdPortMask)!=0)&&(ret==SUCCESS))
							{
							#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
								rtl865x_ipMulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,skb->srcPort,skb->srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0], mapPortMask);
							#else
								rtl865x_ipMulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,skb->srcPort,skb->srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0]);
							#endif
							}
						}
					}	
			#else
					else if(hwwifiEnable&&( ((~br0SwFwdPortMask) & multicastFwdInfo.fwdPortMask)!=0)&&(ret==SUCCESS))
					{
			#if defined(CONFIG_RTK_VLAN_SUPPORT)
						if(rtk_vlan_support_enable == 0)
			#endif
						{
							rtl865x_ipMulticastHardwareAccelerate(bridge0, multicastFwdInfo.fwdPortMask,skb->srcPort,skb->srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0]);
						}
					}
			#endif
			#endif
				}
			}
#if defined (CONFIG_RTL_HW_MCAST_WIFI)
			if(hwwifiEnable)
			{
				existMulticastEntry=rtl865x_findMCastEntry(multicastDataInfo.groupAddr[0], multicastDataInfo.sourceIp[0], (unsigned short)srcVlanId, (unsigned short)srcPort);
				if(existMulticastEntry!=NULL && (existMulticastEntry->inAsic))
				{
				#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT
					multicastFwdInfo.fwdPortMask &= swFwdPortMask;
				#else
					multicastFwdInfo.fwdPortMask &= br0SwFwdPortMask;
				#endif
				}
			}
#endif
			#endif
			skb_push(skb, ETH_HLEN);
			prev = NULL;
			fwdCnt=0;
		#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
			list_for_each_entry_safe(p, n, &br->port_list, list)
		#else
		#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
			list_for_each_entry_safe(p, n, &bridge->port_list, list) 
		#else
			list_for_each_entry_safe(p, n, &bridge0->port_list, list)
		#endif
		#endif
			{
				port_bitmask = (1 << p->port_no);
				if ((port_bitmask & multicastFwdInfo.fwdPortMask) && should_deliver(p, skb))
				{
					if (prev != NULL)
					{
						if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL)
						{
							LOG_MEM_ERROR("%s(%d) skb clone failed, drop it\n", __FUNCTION__, __LINE__);
							#if defined(CONFIG_RT_MULTIPLE_BR_SUPPORT)
							br->dev->stats.tx_dropped++;
							#else
                            #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
							bridge->dev->stats.tx_dropped++;
			    			#else
							bridge0->dev->stats.tx_dropped++;
							#endif
							#endif
							kfree_skb(skb);
							return 0;
						}
						skb2->dev=prev->dev;
						#if 0
						if(net_ratelimit())
							printk("%s:%d,prev->dev->name is %s\n",__FUNCTION__,__LINE__,prev->dev->name);
						#endif
						#if defined(CONFIG_COMPAT_NET_DEV_OPS)
						prev->dev->hard_start_xmit(skb2, prev->dev);
						#else
						prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
						#endif
						fwdCnt++;
					}
					prev = p;
				}
			}
			if (prev != NULL)
			{
				skb->dev=prev->dev;
				#if 0
				if(net_ratelimit())
					printk("%s:%d,prev->dev->name is %s\n",__FUNCTION__,__LINE__,prev->dev->name);
				#endif
		        #if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb, prev->dev);
				#else
				prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
				#endif
				fwdCnt++;
			}
			if(fwdCnt==0)
			{
				skb_pull(skb, ETH_HLEN);
				return -1;
			}
			return 0;
		}
	}
#if 0 //defined (CONFIG_RTL_MLD_SNOOPING)
	if  ((dest[0]==0x33) && (dest[1]==0x33) && (dest[2]!=0xff))
	{
		struct net_bridge_port *p;
		if(mldSnoopEnabled==0)
		{
			return -1;
		}
		p= rcu_dereference(skb->dev->br_port);
		if(p==NULL)
		{
			return -1;
		}
		ptr=(unsigned char *)eth_hdr(skb)+12;
		if(*(int16 *)(ptr)==(int16)htons(0x8100))
		{
			ptr=ptr+4;
		}
		if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IPV6))
		{
			return -1;
		}
		ipv6h=(struct ipv6hdr *)(ptr+2);
		proto =  re865x_getIpv6TransportProtocol(ipv6h);
		 if((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
		{
			multicastDataInfo.ipVersion=6;
			memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, sizeof(struct in6_addr));
			memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, sizeof(struct in6_addr));
			ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			if(ret!=0)
			{
				if(multicastFwdInfo.unknownMCast)
				{
					multicastFwdInfo.fwdPortMask=0xFFFFFFFF;
				}
				else
				{
					return -1;
				}
			}
			skb_push(skb, ETH_HLEN);
			prev = NULL;
			fwdCnt=0;
			list_for_each_entry_safe(p, n, &bridge0->port_list, list)
			{
				port_bitmask = (1 << p->port_no);
				if ((port_bitmask & multicastFwdInfo.fwdPortMask) && (skb->dev != p->dev && p->state == BR_STATE_FORWARDING))
				{
					if (prev != NULL)
					{
						if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL)
						{
							kfree_skb(skb);
							return 0;
						}
						skb2->dev=prev->dev;
						#if defined(CONFIG_COMPAT_NET_DEV_OPS)
						prev->dev->hard_start_xmit(skb2, prev->dev);
						#else
						prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
						#endif
						fwdCnt++;
					}
					prev = p;
				}
			}
			if (prev != NULL)
			{
				skb->dev=prev->dev;
			       #if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb, prev->dev);
				#else
				prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
				#endif
				fwdCnt++;
			}
			if(fwdCnt==0)
			{
				skb_pull(skb, ETH_HLEN);
				return -1;
			}
			return 0;
		}
	}
#endif/*if 0*/
	return -1;
}
#endif/*CONFIG_RTL_IGMP_SNOOPING*/
