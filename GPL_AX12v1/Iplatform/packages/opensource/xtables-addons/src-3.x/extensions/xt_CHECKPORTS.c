/* 匹配规则，批量检查端口 */
/* (C) 2019/8/23 By Liqiang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#include <linux/types.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_l3proto.h>
#include <net/netfilter/nf_nat_l4proto.h>
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_nat_helper.h>
#include <linux/netfilter_ipv4.h>
#include <linux/rculist_nulls.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/inetdevice.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/dccp.h>
#include <linux/netfilter/xt_tcpudp.h>
#include <linux/netfilter/xt_dccp.h>
#include <linux/sctp.h>
#include <linux/netfilter/xt_sctp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include "compat_xtables.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liqiang <liqiang@tp-link.com.cn>");
MODULE_DESCRIPTION("Xtables: map the ports by multiple");


#define NF_NAT_RANGE_CHECKPORTS_MAX 	128


struct nf_nat_range_checkports {
	unsigned int			flags;
	union nf_inet_addr		match_addr;
	union nf_inet_addr		fake_addr;
	union nf_conntrack_man_proto	min_proto[NF_NAT_RANGE_CHECKPORTS_MAX];
	union nf_conntrack_man_proto	max_proto[NF_NAT_RANGE_CHECKPORTS_MAX];
};

struct nf_nat_ipv4_range_checkports {
	unsigned int			flags;
	__be32				match_ip;
	__be32				fake_ip;
	union nf_conntrack_man_proto	min[NF_NAT_RANGE_CHECKPORTS_MAX];
	union nf_conntrack_man_proto	max[NF_NAT_RANGE_CHECKPORTS_MAX];
};

struct nf_nat_ipv4_multi_range_checkports_compat {
	unsigned int			rangesize;
	struct nf_nat_ipv4_range_checkports	range[1];
};


static int xt_nat_checkports_checkentry_v0(const struct xt_tgchk_param *par)
{
        //TODO
        return 0;
}

static bool checkPorts(const struct nf_nat_ipv4_multi_range_checkports_compat *pCheckParam,  __be16 port)
{
	int				num = 0;
	int 			i = 0;
	
	num = ntohs(pCheckParam->range[0].min[0].all);
	
	for (i = 1; i <= num; i ++)
	{
		if(port >= ntohs(pCheckParam->range[0].min[i].all) &&  port <= ntohs(pCheckParam->range[0].max[i].all))
		{
			return true;
		}
	}
	
	return false;
}

static bool xt_nat_checkports_handle(struct sk_buff *skb, struct nf_nat_range * dst, const struct nf_nat_ipv4_multi_range_checkports_compat *pCheckParam)
{
	struct iphdr *	iph = ip_hdr(skb);
	bool 			changeFlag = false;
	struct tcphdr *	tcph;
	struct udphdr *	udph;
	struct icmphdr*	icmph;
	
	switch (iph->protocol)
	{
		case IPPROTO_TCP:
			tcph = (void *)iph + iph->ihl*4;
			if(checkPorts(pCheckParam, ntohs(tcph->dest)))
				return changeFlag;
			break;
		case IPPROTO_UDP:
			udph = (void *)iph + iph->ihl*4;
			if(checkPorts(pCheckParam, ntohs(udph->dest)))
				return changeFlag;
			break;
		case IPPROTO_ICMP:
			icmph = (void *)iph + iph->ihl*4;
			if(checkPorts(pCheckParam, ntohs((icmph->un).echo.id)))
				return changeFlag;
			break;
	}
	
	memset(&dst->min_addr, 0, sizeof(dst->min_addr));
	memset(&dst->max_addr, 0, sizeof(dst->max_addr));

	dst->flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
	dst->min_addr.ip = pCheckParam->range[0].fake_ip;
	dst->max_addr.ip = pCheckParam->range[0].fake_ip;
	
	dst->min_proto.all	 = 0;
	dst->max_proto.all	 = 65535;
	
	changeFlag = true;
	
	return changeFlag;
}
static unsigned int
xt_checkports_target_v0(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct nf_nat_ipv4_multi_range_checkports_compat *pCheckParam = par->targinfo;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
		struct nf_nat_range2 range;
#else
		struct nf_nat_range range;
#endif	

	struct iphdr *iph = ip_hdr(skb);

	//struct nf_nat_range_checkports checkRange;
	//struct nf_nat_ipv4_multi_range_compat *mr;
	
	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct != NULL &&
		     (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED ||
		      ctinfo == IP_CT_RELATED_REPLY));

	//xt_nat_convert_range_v6plus(&range, &mr->range[0]);
	memset(&range, 0, sizeof(range));

	if(iph->daddr ==  pCheckParam->range[0].match_ip)
	{
		if( xt_nat_checkports_handle(skb, (struct nf_nat_range *)&range, pCheckParam) )
		{
			return nf_nat_setup_info(ct, &range, NF_NAT_MANIP_DST);
		}
	}
	
	return XT_CONTINUE;
}



static struct xt_target checkports_reg __read_mostly = {
	.name           = "CHECKPORTS",
	.family         = NFPROTO_IPV4,
	.target         = xt_checkports_target_v0,
	.targetsize  	= sizeof(struct nf_nat_ipv4_multi_range_checkports_compat),
	.table          = "nat",
	.hooks          = (1 << NF_INET_PRE_ROUTING),
	.checkentry     = xt_nat_checkports_checkentry_v0,
	.me             = THIS_MODULE,
};
 
static int __init checkports_target_init(void)
{
    return xt_register_target(&checkports_reg);
}
 
static void __exit checkpots_target_exit(void)
{
    xt_unregister_target(&checkports_reg);
}
 
module_init(checkports_target_init);
module_exit(checkpots_target_exit);

