/* 匹配规则，自动执行NAT loopback 的DNAT部分 */
/* (C) 2018/10/24 By SuWeilin
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
#include <linux/dccp.h>
#include <linux/netfilter/xt_tcpudp.h>
#include <linux/netfilter/xt_dccp.h>
#include <linux/sctp.h>
#include <linux/netfilter/xt_sctp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_nat.h>
#include <linux/version.h>

#include "compat_xtables.h"

 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SuWeilin <suweilin@tp-link.com.cn>");
MODULE_DESCRIPTION("Xtables: DNAT loopback");

#define BR_LAN             "br-lan"
#define BR_LAN_LEN        6
#define DNAT               "DNAT"
#define DNAT_LEN          4
#define TCP                "tcp"
#define TCP_LEN           3
#define UDP                "udp"
#define UDP_LEN           3
#define UDP_LITE          "udplite"
#define UDP_LITE_LEN     7
#define DCCP               "dccp"
#define DCCP_LEN          4
#define SCTP               "sctp"
#define SCTP_LEN          4



 struct xt_LOOPBACKDNAT_info
 {
	 __be32 dst_ip;
 };

/* Performance critical */
static inline struct ipt_entry *
get_entry(const void *base, unsigned int offset)
{
	return (struct ipt_entry *)(base + offset);
}

/* Returns 1 if the port is matched by the range, 0 otherwise */
static inline bool
port_match(u_int16_t min, u_int16_t max, u_int16_t port)
{
	return (port >= min && port <= max);
}

static void xt_nat_convert_range(struct nf_nat_range2 *dst,
				 const struct nf_nat_ipv4_range *src)
{
	memset(&dst->min_addr, 0, sizeof(dst->min_addr));
	memset(&dst->max_addr, 0, sizeof(dst->max_addr));

	dst->flags	 = src->flags;
	dst->min_addr.ip = src->min_ip;
	dst->max_addr.ip = src->max_ip;
	dst->min_proto	 = src->min;
	dst->max_proto	 = src->max;
}

#ifdef NATLOOPBACK_DEBUG
static void print_ct(struct sk_buff *skb)
{
	struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;		
	struct net *net;
	struct nf_conntrack_tuple_hash *h;
	struct hlist_nulls_node *n;
	unsigned int i;

	ct = nf_ct_get(skb, &ctinfo);
	net = nf_ct_net(ct);
	printk(KERN_INFO "[ %s ]print ct\n\n", __FUNCTION__);
	for (i = 0; i < nf_conntrack_htable_size; i++) {
		hlist_nulls_for_each_entry_rcu(h, n, &net->ct.hash[i], hnnode)
		{
			printk(KERN_INFO "tuple %p: %u %pI4:%hu -> %pI4:%hu dir: %d\n",
			       h, h->tuple.dst.protonum,
			       &h->tuple.src.u3.ip, ntohs(h->tuple.src.u.all),
			       &h->tuple.dst.u3.ip, ntohs(h->tuple.dst.u.all),
			       h->tuple.dst.dir);
			ct = nf_ct_tuplehash_to_ctrack(h);
			printk(KERN_INFO "tuple proto: %u\n%pI4:%hu -> %pI4:%hu\n%pI4:%hu -> %pI4:%hu\nstatus %lu\n",
					ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum,
					&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip, ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all),
					&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip, ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all),
					&ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip, ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all),
					&ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip, ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all),
					ct->status);
		}
	}
}

static void print_expect_ct(struct sk_buff *skb)
{
	struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;		
	struct net *net;
	struct nf_conntrack_expect *exp;
	unsigned int i;

	ct = nf_ct_get(skb, &ctinfo);
	net = nf_ct_net(ct);

	for (i = 0; i < nf_ct_expect_hsize; i++) {
		hlist_for_each_entry_rcu(exp, &net->ct.expect_hash[i], hnode) {
			printk(KERN_INFO "tuple %p: %u %pI4:%hu -> %pI4:%hu dir: %d name: %s\nsave: %pI4:%hu dir: %d\n",
			       exp, exp->tuple.dst.protonum,
			       &exp->tuple.src.u3.ip, ntohs(exp->tuple.src.u.all),
			       &exp->tuple.dst.u3.ip, ntohs(exp->tuple.dst.u.all),
			       exp->tuple.dst.dir, exp->helper?exp->helper->name:NULL,
			       &exp->saved_addr.ip, ntohs(exp->saved_proto.all), exp->dir);
		}
	}
}

static void print_rules(struct sk_buff *skb, unsigned int hook)
{
	struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;		
	struct net *net;
	struct xt_table *table;
	const void *table_base;
	struct ipt_entry *e, *iter;
	const struct xt_entry_target *t;
	unsigned int cpu;
	const struct xt_table_info *private;
	struct ipt_ip *ipinfo;
	const struct xt_tcp *tcpinfo;
	const struct xt_entry_match *ematch;
	const struct nf_nat_ipv4_multi_range_compat *mr;

	printk(KERN_INFO "[ %s ]print rules\n\n", __FUNCTION__);
    ct = nf_ct_get(skb, &ctinfo);
	net = nf_ct_net(ct);
	table = net->ipv4.nat_table;
	private = table->private;
	cpu = smp_processor_id();
	table_base = private->entries[cpu];
	e = get_entry(table_base, private->hook_entry[hook]);
	
    NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

	xt_entry_foreach(iter, e, private->size - private->hook_entry[hook])
	{
		ipinfo = &iter->ip;		
		printk(KERN_INFO "ipt_entry src: %pI4 smsk: %pI4 dst: %pI4 dmsk: %pI4 in: %s out: %s proto: %u\n",
			&ipinfo->src, &ipinfo->smsk, &ipinfo->dst, &ipinfo->dmsk, ipinfo->iniface, ipinfo->outiface, ipinfo->proto);

		xt_ematch_foreach(ematch, iter) {
			if (ematch->u.kernel.match)
			{
				tcpinfo = (struct xt_tcp *)ematch->data;
				if (tcpinfo)
				{
					printk(KERN_INFO "match src range: %u to %u dst ramge: %u to %u name: %s\n",
						tcpinfo->spts[0], tcpinfo->spts[1], tcpinfo->dpts[0], tcpinfo->dpts[1], ematch->u.kernel.match->name);
				}
			}
		}
		
		t = ipt_get_target(iter);
		if (t->u.kernel.target)
		{
			mr = (struct nf_nat_ipv4_multi_range_compat *)t->data;
			if (mr)
			{
				printk(KERN_INFO "target range max: %pI4:%u min: %pI4:%u name: %s\n",
					&mr->range[0].max_ip, ntohs(mr->range[0].max.all), &mr->range[0].min_ip, ntohs(mr->range[0].min.all), t->u.kernel.target->name);
			}
		}
	}
}
#endif

static bool match_rules(struct sk_buff *skb, const struct xt_LOOPBACKDNAT_info *info, unsigned int hook, struct nf_nat_ipv4_multi_range_compat **mr)
{
	const struct iphdr *ip = ip_hdr(skb);
	bool found_match = false;
	struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;		
	struct net *net;
	struct xt_table *table;
	const void *table_base;
	struct ipt_entry *e, *iter;
	const struct xt_entry_target *t;
	unsigned int cpu;
	const struct xt_table_info *private;
	struct ipt_ip *ipinfo;
	const struct xt_tcp *tcpinfo;
	const struct tcphdr *th;
	struct tcphdr _tcph;
	const struct xt_udp *udpinfo;
	const struct udphdr *uh;
	struct udphdr _udph;
	const struct xt_dccp_info *dccpinfo;
	const struct dccp_hdr *dh;
	struct dccp_hdr _dh;
	const struct xt_sctp_info *sctpinfo;
	const struct sctphdr *sh;
	struct sctphdr _sh;
	const struct xt_entry_match *ematch;

    ct = nf_ct_get(skb, &ctinfo);
	net = nf_ct_net(ct);
	table = net->ipv4.nat_table;
	private = table->private;
//	cpu = smp_processor_id();
	table_base = private->entries;
	e = get_entry(table_base, private->hook_entry[hook]);
	
	xt_entry_foreach(iter, e, private->size - private->hook_entry[hook])
	{
		ipinfo = &iter->ip;		
		t = ipt_get_target(iter);
		if (!strncmp(t->u.kernel.target->name, DNAT , DNAT_LEN))
		{
			found_match = false;
			xt_ematch_foreach(ematch, iter) {
				if (ematch->u.kernel.match)
				{
					if (!strncmp(ematch->u.kernel.match->name, TCP , TCP_LEN))
					{
						found_match = true;
						if (IPPROTO_TCP == ip->protocol)
						{
							th = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_tcph), &_tcph);
							tcpinfo = (struct xt_tcp *)ematch->data;
							if (tcpinfo)
							{
								if (port_match(tcpinfo->dpts[0], tcpinfo->dpts[1], ntohs(th->dest)) &&
									(ipinfo->dst.s_addr == ip->daddr || (ipinfo->dst.s_addr == 0 && ip->daddr == info->dst_ip)))
								{
									*mr = (struct nf_nat_ipv4_multi_range_compat *)t->data;
									return true;
								}
							}
						}
					}
					else if (!strncmp(ematch->u.kernel.match->name, UDP , UDP_LEN) ||
						!strncmp(ematch->u.kernel.match->name, UDP_LITE , UDP_LITE_LEN))
					{
						found_match = true;
						if (IPPROTO_UDP == ip->protocol || IPPROTO_UDPLITE == ip->protocol)
						{
							uh = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_udph), &_udph);
							udpinfo = (struct xt_udp *)ematch->data;
							if (udpinfo)
							{
								if (port_match(udpinfo->dpts[0], udpinfo->dpts[1], ntohs(uh->dest)) &&
									(ipinfo->dst.s_addr == ip->daddr || (ipinfo->dst.s_addr == 0 && ip->daddr == info->dst_ip)))
								{
									*mr = (struct nf_nat_ipv4_multi_range_compat *)t->data;
									return true;
								}
							}
						}
					}
					else if (!strncmp(ematch->u.kernel.match->name, DCCP , DCCP_LEN))
					{
						found_match = true;
						if (IPPROTO_DCCP == ip->protocol)
						{
							dh = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_dh), &_dh);
							dccpinfo = (struct xt_dccp_info *)ematch->data;
							if (dccpinfo)
							{
								if (port_match(dccpinfo->dpts[0], dccpinfo->dpts[1], ntohs(dh->dccph_dport)) &&
									(ipinfo->dst.s_addr == ip->daddr || (ipinfo->dst.s_addr == 0 && ip->daddr == info->dst_ip)))
								{
									*mr = (struct nf_nat_ipv4_multi_range_compat *)t->data;
									return true;
								}
							}
						}
					}
					else if (!strncmp(ematch->u.kernel.match->name, SCTP , SCTP_LEN))
					{
						found_match = true;
						if (IPPROTO_SCTP == ip->protocol)
						{
							sh = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_sh), &_sh);
							sctpinfo = (struct xt_sctp_info *)ematch->data;
							if (sctpinfo)
							{
								if (port_match(sctpinfo->dpts[0], sctpinfo->dpts[1], ntohs(sh->dest)) &&
									(ipinfo->dst.s_addr == ip->daddr || (ipinfo->dst.s_addr == 0 && ip->daddr == info->dst_ip)))
								{
									*mr = (struct nf_nat_ipv4_multi_range_compat *)t->data;
									return true;
								}
							}
						}
					}
				}
			}
			if (!found_match && ipinfo->dst.s_addr == ip->daddr)
			{
				*mr = (struct nf_nat_ipv4_multi_range_compat *)t->data;
				return true;
			}
		}
	}
	
	return false;
}

static bool match_expect_ct(struct sk_buff *skb, struct nf_nat_range2 *range)
{
	const struct iphdr *ip = ip_hdr(skb);
	struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;		
	struct net *net;
	struct nf_conntrack_expect *exp;
	unsigned int i;
	const struct tcphdr *th;
	struct tcphdr _tcph;
	const struct udphdr *uh;
	struct udphdr _udph;
	const struct dccp_hdr *dh;
	struct dccp_hdr _dh;
	const struct sctphdr *sh;
	struct sctphdr _sh;

	ct = nf_ct_get(skb, &ctinfo);
	net = nf_ct_net(ct);

	for (i = 0; i < nf_ct_expect_hsize; i++) {
		hlist_for_each_entry_rcu(exp, &nf_ct_expect_hash[i], hnode) {
			if (exp->tuple.dst.u3.ip == ip->daddr)
			{
				switch (ip->protocol)
				{
					case IPPROTO_TCP:
						if (exp->tuple.dst.protonum == ip->protocol)
						{
							th = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_tcph), &_tcph);
							if (th->dest == exp->tuple.dst.u.all)
							{	
								range->flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
								range->min_proto = range->max_proto = exp->saved_proto;
								range->min_addr.ip = range->max_addr.ip = exp->saved_addr.ip;

								return true;
							}
						}
					break;
					case IPPROTO_UDP:
					case IPPROTO_UDPLITE:
						if (exp->tuple.dst.protonum == ip->protocol)
						{
							uh = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_udph), &_udph);
							if (uh->dest == exp->tuple.dst.u.all)
							{
								range->flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
								range->min_proto = range->max_proto = exp->saved_proto;
								range->min_addr.ip = range->max_addr.ip = exp->saved_addr.ip;

								return true;
							}
						}
					break;
					case IPPROTO_DCCP:
						if (exp->tuple.dst.protonum == ip->protocol)
						{
							dh = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_dh), &_dh);
							if (dh->dccph_dport == exp->tuple.dst.u.all)
							{				
								range->flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
								range->min_proto = range->max_proto = exp->saved_proto;
								range->min_addr.ip = range->max_addr.ip = exp->saved_addr.ip;

								return true;
							}
						}
					break;
					case IPPROTO_SCTP:
						if (exp->tuple.dst.protonum == ip->protocol)
						{
							sh = skb_header_pointer(skb, ip_hdrlen(skb), sizeof(_sh), &_sh);
							if (sh->dest == exp->tuple.dst.u.all)
							{				
								range->flags = (NF_NAT_RANGE_MAP_IPS | NF_NAT_RANGE_PROTO_SPECIFIED);
								range->min_proto = range->max_proto = exp->saved_proto;
								range->min_addr.ip = range->max_addr.ip = exp->saved_addr.ip;

								return true;
							}
						}
					break;
				}
			}
		}
	}
	return false;
}

static int loopbackDNAT_check(const struct xt_tgchk_param *par)
{
        //TODO
        return 0;
}
 
static unsigned int
loopbackDNAT_target(struct sk_buff *skb, const struct xt_action_param *par)
{
		const struct xt_LOOPBACKDNAT_info *info = par->targinfo;
		struct nf_conn *ct;
		enum ip_conntrack_info ctinfo;
		struct nf_nat_ipv4_multi_range_compat *mr;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
		struct nf_nat_range2 range;
#else
		struct nf_nat_range range;
#endif
		const struct iphdr *ip = ip_hdr(skb);
		struct in_device *in_dev = in_dev_get(par->state->in);
		__be32 mask = 0;

		NF_CT_ASSERT(par->state->hook == NF_INET_PRE_ROUTING ||
		     par->state->hook == NF_INET_LOCAL_OUT);

		ct = nf_ct_get(skb, &ctinfo);
		memset(&range, 0, sizeof(range));

		/* Connection must be valid and new. */
		NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

		if (in_dev && in_dev->ifa_list)
	    {
	        mask = in_dev->ifa_list->ifa_mask;

			if ((ip->saddr & mask) != (ip->daddr & mask))
			{
				if (match_rules(skb, info, NF_INET_PRE_ROUTING, &mr))
				{
					in_dev_put(in_dev); 
					xt_nat_convert_range(&range, &mr->range[0]);
					return nf_nat_setup_info(ct, &range, NF_NAT_MANIP_DST);
				}
				if (match_expect_ct(skb, &range))
				{
					in_dev_put(in_dev); 
					return nf_nat_setup_info(ct, &range, NF_NAT_MANIP_DST);
				}
			}
		}
		if (in_dev)
			in_dev_put(in_dev); 
		
		return XT_CONTINUE;
}
 
static struct xt_target loopbackDNAT_reg __read_mostly = {
        .name           = "LOOPBACKDNAT",
        .family         = NFPROTO_IPV4,
        .target         = loopbackDNAT_target,
        .targetsize  	= sizeof(struct xt_LOOPBACKDNAT_info),
		.table          = "nat",
        .hooks          = (1 << NF_INET_PRE_ROUTING),
        .checkentry     = loopbackDNAT_check,
        .me             = THIS_MODULE,
};
 
static int __init loopbackDNAT_target_init(void)
{
        return xt_register_target(&loopbackDNAT_reg);
}
 
static void __exit loopbackDNAT_target_exit(void)
{
        xt_unregister_target(&loopbackDNAT_reg);
}
 
module_init(loopbackDNAT_target_init);
module_exit(loopbackDNAT_target_exit);
