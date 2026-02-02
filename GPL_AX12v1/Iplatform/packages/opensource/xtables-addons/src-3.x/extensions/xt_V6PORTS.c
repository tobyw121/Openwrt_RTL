/* 匹配规则，批量映射端口 */
/* (C) 2019/8/20 By Liqiang
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
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include <linux/version.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("liqiang <liqiang@tp-link.com.cn>");
MODULE_DESCRIPTION("Xtables: map the ports by multiple");

#define NF_NAT_RANGE_V6PORTS_MAX 	128

static DEFINE_SPINLOCK(nf_nat_v6ports_lock);

static u16 tcp_port_rover;
static u16 udp_port_rover;

static u16 udplite_port_rover;
static u16 dccp_port_rover;
static u16 sctp_port_rover;

extern struct hlist_head *nf_nat_bysource;
extern unsigned int nf_nat_htable_size;
extern unsigned int nf_nat_hash_rnd;

extern spinlock_t nf_nat_locks[CONNTRACK_LOCKS];

struct nf_nat_range_v6ports {
	unsigned int			flags;
	union nf_inet_addr		min_addr;
	union nf_inet_addr		max_addr;
	union nf_conntrack_man_proto	min_proto[NF_NAT_RANGE_V6PORTS_MAX];
	union nf_conntrack_man_proto	max_proto[NF_NAT_RANGE_V6PORTS_MAX];
};

struct nf_nat_ipv4_range_v6ports {
	unsigned int			flags;
	__be32				min_ip;
	__be32				max_ip;
	union nf_conntrack_man_proto	min[NF_NAT_RANGE_V6PORTS_MAX];
	union nf_conntrack_man_proto	max[NF_NAT_RANGE_V6PORTS_MAX];
};

struct nf_nat_ipv4_multi_range_v6ports_compat {
	unsigned int			rangesize;
	struct nf_nat_ipv4_range_v6ports	range[1];
};
/*
static const struct nf_nat_l3proto __rcu *nf_nat_V6PORTS_l3protos[NFPROTO_NUMPROTO]
						__read_mostly;
*/
/*
static const struct nf_nat_l4proto __rcu **nf_nat_l4protos[NFPROTO_NUMPROTO]
						__read_mostly;
*/					
/*
inline const struct nf_nat_l3proto *__nf_nat_V6PORTS_l3proto_find(u8 family)
{
	return rcu_dereference(nf_nat_V6PORTS_l3protos[family]);
}
*/

/*
inline const struct nf_nat_l4proto * __nf_nat_l4proto_find(u8 family, u8 protonum)
{
	return rcu_dereference(nf_nat_l4protos[family][protonum]);
}
EXPORT_SYMBOL_GPL(__nf_nat_l4proto_find);
*/
/* We keep an extra hash for each conntrack, for fast searching. */
static unsigned int
hash_by_src(const struct net *n, const struct nf_conntrack_tuple *tuple)
{
	unsigned int hash;

	get_random_once(&nf_nat_hash_rnd, sizeof(nf_nat_hash_rnd));

	/* Original src, to ensure we map it consistently if poss. */
	hash = jhash2((u32 *)&tuple->src, sizeof(tuple->src) / sizeof(u32),
			  tuple->dst.protonum ^ nf_nat_hash_rnd ^ net_hash_mix(n));

	return reciprocal_scale(hash, nf_nat_htable_size);
}


static inline int
same_src(const struct nf_conn *ct,
	 const struct nf_conntrack_tuple *tuple)
{
	const struct nf_conntrack_tuple *t;

	t = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	return (t->dst.protonum == tuple->dst.protonum &&
		nf_inet_addr_cmp(&t->src.u3, &tuple->src.u3) &&
		t->src.u.all == tuple->src.u.all);
}


int selectPortFromSets(const struct nf_nat_l3proto *l3proto,
				 struct nf_conntrack_tuple *tuple,
				 const struct nf_nat_range_v6ports *range,
				 enum nf_nat_manip_type maniptype,
				 const struct nf_conn *ct,
				 u16 *rover)
{
	
	unsigned int	i = 1;
	unsigned int	j = 0;
	
	unsigned int	num = 1;
	
	__be16  oldport;
	__be16 	*portptr;
	unsigned int range_size, min;
	int		flag = 0;
	
	u_int16_t off;
	oldport = tuple->src.u.all;
	
	if (maniptype == NF_NAT_MANIP_SRC)
	{
		portptr = &tuple->src.u.all;
		oldport = tuple->src.u.all;
	}
	else
	{
		portptr = &tuple->dst.u.all;
		oldport = tuple->dst.u.all;
	}
	num = ntohs(range->min_proto[0].all);
	/* 
	**	i = oldprot % (num-1)  + 1 =  
	**	for example :  num = 16 	
	**	oldport 1    % 15 + 1	map   min_proto[2] - max_proto[2]
	**	oldport 2    % 15 + 1	map   min_proto[3] - max_proto[3]
	**	oldport 14   % 15 + 1	map   min_proto[15] - max_proto[15]
	**	oldport 15   % 15 + 1	map   min_proto[1] - max_proto[1]
	**	oldport 16   % 15 + 1	map   min_proto[2] - max_proto[2]
	*/
	i = oldport%(num - 1) + 1;
	min = ntohs(range->min_proto[i].all);
	range_size = ntohs(range->max_proto[i].all) - min + 1;
	
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		off = l3proto->secure_port(tuple, maniptype == NF_NAT_MANIP_SRC
					  ? tuple->dst.u.all
					  : tuple->src.u.all);
	else
		off = *rover;
	
	for (j = 0; ; ++off) {
		*portptr = htons(min + off % range_size);
		
		if (++j != range_size && nf_nat_used_tuple(tuple, ct))
			continue;	
		
		if (!(range->flags & NF_NAT_RANGE_PROTO_RANDOM))
			*rover = off;

		flag = i;
		break;
	}

	
	return flag;
}


void nf_nat_l4proto_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
				 struct nf_conntrack_tuple *tuple,
				 const struct nf_nat_range_v6ports *range,
				 enum nf_nat_manip_type maniptype,
				 const struct nf_conn *ct,
				 u16 *rover)
{
	
	unsigned int index = 0;
	
	index = selectPortFromSets(l3proto, tuple, range, maniptype, ct, rover);
	
	return;
}

static void
icmp_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
		  struct nf_conntrack_tuple *tuple,
		  const struct nf_nat_range_v6ports *range,
		  enum nf_nat_manip_type maniptype,
		  const struct nf_conn *ct)
{
	static u_int16_t id;
	unsigned int range_size;
	unsigned int i;
	
	unsigned int num;
	unsigned int index ;
	unsigned int min;
	__be16	oldport;

	num = ntohs(range->min_proto[0].all);
	oldport = tuple->src.u.icmp.id;
	index = ntohs(oldport) % (num - 1) + 1;
	min = ntohs(range->min_proto[index].all);
	range_size = ntohs(range->max_proto[index].all) - min + 1;

	/*
	 * range_size = ntohs(range->max_proto.icmp.id) -
		     ntohs(range->min_proto.icmp.id) + 1;
	*/
	/* If no range specified... */
	if (!(range->flags & NF_NAT_RANGE_PROTO_SPECIFIED))
		range_size = 0xFFFF;

	for (i = 0; ; ++id) {
		tuple->src.u.icmp.id = htons(ntohs(range->min_proto[index].icmp.id) +
					     (id % range_size));
		if (++i == range_size || !nf_nat_used_tuple(tuple, ct))
			return;
	}
	return;
}

static void
tcp_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
		 struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range_v6ports *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
	nf_nat_l4proto_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct,
				    &tcp_port_rover);
}

static void
udp_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
		 struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range_v6ports *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
	nf_nat_l4proto_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct,
				    &udp_port_rover);
}

static void
udplite_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
		 struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range_v6ports *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
	nf_nat_l4proto_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct,
				    &udplite_port_rover);
}

static void
dccp_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
		 struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range_v6ports *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
	nf_nat_l4proto_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct,
				    &dccp_port_rover);
}

static void
sctp_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
		 struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range_v6ports *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
	nf_nat_l4proto_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct,
				    &sctp_port_rover);
}

static int xt_nat_v6ports_checkentry_v0(const struct xt_tgchk_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	if (mr->rangesize != 1) {
		pr_info("%s: multiple ranges no longer supported\n",
			par->target->name);
		return -EINVAL;
	}
	return 0;
}

static void
gre_unique_v6ports_tuple(const struct nf_nat_l3proto *l3proto,
		 struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range_v6ports *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
	static u_int16_t key;
	__be16 *keyptr;
	unsigned int min, i, range_size;

	unsigned int index;
	unsigned int num;
	/* If there is no master conntrack we are not PPTP,
	   do not change tuples */
	if (!ct->master)
		return;

	if (maniptype == NF_NAT_MANIP_SRC)
		keyptr = &tuple->src.u.gre.key;
	else
		keyptr = &tuple->dst.u.gre.key;
	
	num = ntohs(range->min_proto[0].all);
	index = ntohs(keyptr) % (num - 1) + 1;
	
	if (!(range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)) {
		pr_debug("%p: NATing GRE PPTP\n", ct);
		min = 1;
		range_size = 0xffff;
	} else {
		min = ntohs(range->min_proto[index].gre.key);
		range_size = ntohs(range->max_proto[index].gre.key) - min + 1;
	}

	pr_debug("min = %u, range_size = %u\n", min, range_size);

	for (i = 0; ; ++key) {
		*keyptr = htons(min + key % range_size);
		if (++i == range_size || !nf_nat_used_tuple(tuple, ct))
			return;
	}

	pr_debug("%p: no NAT mapping\n", ct);
	return;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
static void xt_nat_range_v6ports_convert_range(struct nf_nat_range2 *dst,
				 const struct nf_nat_range_v6ports *src, int index )
#else
static void xt_nat_range_v6ports_convert_range(struct nf_nat_range *dst,
				 const struct nf_nat_range_v6ports *src, int index )
#endif
{
	memset(&dst->min_addr, 0, sizeof(dst->min_addr));
	memset(&dst->max_addr, 0, sizeof(dst->max_addr));
	
	if(index != 0)
	{
		dst->flags	 = src->flags;
		dst->min_addr.ip = src->min_addr.ip;
		dst->max_addr.ip = src->max_addr.ip;
		dst->min_proto	 = src->min_proto[index];
		dst->max_proto	 = src->max_proto[index];
	}
	else
	{
		dst->flags	 = src->flags;
		dst->min_addr.ip = src->min_addr.ip;
		dst->max_addr.ip = src->max_addr.ip;
		dst->min_proto.all	 = htons(0);
		dst->max_proto.all	 = htons(4095);
	}
}
/* If we source map this tuple so reply looks like reply_tuple, will
 * that meet the constraints of range.
 */
static int v6ports_in_range(const struct nf_nat_l3proto *l3proto,
		    const struct nf_nat_l4proto *l4proto,
		    const struct nf_conntrack_tuple *tuple,
		    const struct nf_nat_range_v6ports *range)
{
	int i = 1;
	int	num ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
	struct nf_nat_range2 rangeTmp;
#else
	struct nf_nat_range rangeTmp;
#endif
	__be16 port;

	port = tuple->src.u.all;
	
	xt_nat_range_v6ports_convert_range(&rangeTmp, range, 1);

	/* If we are supposed to map IPs, then we must be in the
	 * range specified, otherwise let this drag us onto a new src IP.
	 */
	if (range->flags & NF_NAT_RANGE_MAP_IPS &&
		!l3proto->in_range(tuple, &rangeTmp))
	{
		return 0;
	}

	num = ntohs(range->min_proto[0].all);
	for(i = 1; i < num; i++)
	{
		if (!(range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) ||
			l4proto->in_range(tuple, NF_NAT_MANIP_SRC,
				&range->min_proto[i], &range->max_proto[i]))
		{
			return 1;
		}
		
	}
	
	return 0;
}

/* Only called for SRC manip */
static int
find_v6ports_appropriate_src(struct net *net, u16 zone,
		     const struct nf_nat_l3proto *l3proto,
		     const struct nf_nat_l4proto *l4proto,
		     const struct nf_conntrack_tuple *tuple,
		     struct nf_conntrack_tuple *result,
		     const struct nf_nat_range_v6ports *range)
{
	unsigned int h = hash_by_src(net, tuple);
	const struct nf_conn *ct;

	hlist_for_each_entry_rcu(ct, &nf_nat_bysource[h], nat_bysource) {
		if (same_src(ct, tuple) &&
		    net_eq(net, nf_ct_net(ct)) &&
		    nf_ct_zone_equal(ct, zone, IP_CT_DIR_ORIGINAL)) {
			/* Copy source part from reply tuple. */
			nf_ct_invert_tuplepr(result,
				       &ct->tuplehash[IP_CT_DIR_REPLY].tuple);
			result->dst = tuple->dst;

			if (v6ports_in_range(l3proto, l4proto, result, range))
				return 1;
		}

	}

	return 0;
}



/* For [FUTURE] fragmentation handling, we want the least-used
 * src-ip/dst-ip/proto triple.  Fairness doesn't come into it.  Thus
 * if the range specifies 1.2.3.4 ports 10000-10005 and 1.2.3.5 ports
 * 1-65535, we don't do pro-rata allocation based on ports; we choose
 * the ip with the lowest src-ip/dst-ip/proto usage.
 */
static void
find_v6ports_best_ips_proto(u16 zone, struct nf_conntrack_tuple *tuple,
		    const struct nf_nat_range_v6ports *range,
		    const struct nf_conn *ct,
		    enum nf_nat_manip_type maniptype)
{
	union nf_inet_addr *var_ipp;
	unsigned int i, max;
	/* Host order */
	u32 minip, maxip, j, dist;
	bool full_range;

	/* No IP mapping?  Do nothing. */
	if (!(range->flags & NF_NAT_RANGE_MAP_IPS))
		return;

	if (maniptype == NF_NAT_MANIP_SRC)
		var_ipp = &tuple->src.u3;
	else
		var_ipp = &tuple->dst.u3;

	/* Fast path: only one choice. */
	if (nf_inet_addr_cmp(&range->min_addr, &range->max_addr)) {
		*var_ipp = range->min_addr;
		return;
	}

	if (nf_ct_l3num(ct) == NFPROTO_IPV4)
		max = sizeof(var_ipp->ip) / sizeof(u32) - 1;
	else
		max = sizeof(var_ipp->ip6) / sizeof(u32) - 1;

	/* Hashing source and destination IPs gives a fairly even
	 * spread in practice (if there are a small number of IPs
	 * involved, there usually aren't that many connections
	 * anyway).  The consistency means that servers see the same
	 * client coming from the same IP (some Internet Banking sites
	 * like this), even across reboots.
	 */
	j = jhash2((u32 *)&tuple->src.u3, sizeof(tuple->src.u3) / sizeof(u32),
		   range->flags & NF_NAT_RANGE_PERSISTENT ?
			0 : (__force u32)tuple->dst.u3.all[max] ^ zone);

	full_range = false;
	for (i = 0; i <= max; i++) {
		/* If first bytes of the address are at the maximum, use the
		 * distance. Otherwise use the full range.
		 */
		if (!full_range) {
			minip = ntohl((__force __be32)range->min_addr.all[i]);
			maxip = ntohl((__force __be32)range->max_addr.all[i]);
			dist  = maxip - minip + 1;
		} else {
			minip = 0;
			dist  = ~0;
		}

		var_ipp->all[i] = (__force __u32)
			htonl(minip + (((u64)j * dist) >> 32));
		if (var_ipp->all[i] != range->max_addr.all[i])
			full_range = true;

		if (!(range->flags & NF_NAT_RANGE_PERSISTENT))
			j ^= (__force u32)tuple->dst.u3.all[i];
	}
}

/* Manipulate the tuple into the range given. For NF_INET_POST_ROUTING,
 * we change the source to map into the range. For NF_INET_PRE_ROUTING
 * and NF_INET_LOCAL_OUT, we change the destination to map into the
 * range. It might not be possible to get a unique tuple, but we try.
 * At worst (or if we race), we will end up with a final duplicate in
 * __ip_conntrack_confirm and drop the packet. */
static void
get_unique_v6ports_tuple(struct nf_conntrack_tuple *tuple,
		 const struct nf_conntrack_tuple *orig_tuple,
		 const struct nf_nat_range_v6ports *range,
		 struct nf_conn *ct,
		 enum nf_nat_manip_type maniptype,
		  __u8 protocol)
{
	const struct nf_nat_l3proto *l3proto;
	const struct nf_nat_l4proto *l4proto;
	struct net *net = nf_ct_net(ct);
	u16 zone = nf_ct_zone(ct);
	
	unsigned int i = 1;
	unsigned int	num = 0;
	
	rcu_read_lock();
	
	l3proto = __nf_nat_l3proto_find(orig_tuple->src.l3num);
	
	l4proto = __nf_nat_l4proto_find(orig_tuple->src.l3num,
					orig_tuple->dst.protonum);
	
	/* 1) If this srcip/proto/src-proto-part is currently mapped,
	 * and that same mapping gives a unique tuple within the given
	 * range, use that.
	 *
	 * This is only required for source (ie. NAT/masq) mappings.
	 * So far, we don't do local source mappings, so multiple
	 * manips not an issue.
	 */
	
	if (maniptype == NF_NAT_MANIP_SRC &&
	    !(range->flags & NF_NAT_RANGE_PROTO_RANDOM)) {
			
		/* try the original tuple first */
		if (v6ports_in_range(l3proto, l4proto, orig_tuple, range)) {	
			if (!nf_nat_used_tuple(orig_tuple, ct)) {
				*tuple = *orig_tuple;
				goto out;
			}

		} else if (find_v6ports_appropriate_src(net, zone, l3proto, l4proto,
						orig_tuple, tuple, range)) {
			pr_debug("get_unique_tuple: Found current src map\n");
			if (!nf_nat_used_tuple(tuple, ct))
				goto out;
		}
	}

	/* 2) Select the least-used IP/proto combination in the given range */
	*tuple = *orig_tuple;
	find_v6ports_best_ips_proto(zone, tuple, range, ct, maniptype);

	/* 3) The per-protocol part of the manip is made to map into
	 * the range to make a unique tuple.
	 */
	
	num = ntohs(range->min_proto[0].all);
	/* Only bother mapping if it's not already in range and unique */
	if (!(range->flags & NF_NAT_RANGE_PROTO_RANDOM)) {
		if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
			for(i = 1; i < num; i ++)
			{
				if (l4proto->in_range(tuple, maniptype,
							  &range->min_proto[i],
							  &range->max_proto[i]) &&
					(range->min_proto[i].all == range->max_proto[i].all ||
					 !nf_nat_used_tuple(tuple, ct)))
				{
					goto out;
				}	
			}
		} else if (!nf_nat_used_tuple(tuple, ct)) {
			goto out;
		}
	}

	/* Last change: get protocol to try to obtain unique tuple. */
	//l4proto->unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
	switch(protocol)
	{
		case IPPROTO_TCP:
			tcp_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
			break;
		case IPPROTO_UDP:
			udp_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
			break;
		case IPPROTO_UDPLITE:
			udplite_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
			break;
		case IPPROTO_DCCP:
			dccp_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
			break;
		case IPPROTO_SCTP:
			sctp_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
			break;
		case IPPROTO_ICMP:
			icmp_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
			break;
		case IPPROTO_GRE:
			gre_unique_v6ports_tuple(l3proto, tuple, range, maniptype, ct);
			break;
		default:
			break;
	}
	
	
out:
	rcu_read_unlock();
}

unsigned int
nf_nat_setup_v6ports_info(struct nf_conn *ct,
		  const struct nf_nat_range_v6ports *range,
		  enum nf_nat_manip_type maniptype,
		  __u8 protocol)
{
	struct net *net = nf_ct_net(ct);
	struct nf_conntrack_tuple curr_tuple, new_tuple;

	
	/* Can't setup nat info for confirmed ct. */
	if (nf_ct_is_confirmed(ct))
		return NF_ACCEPT;

	WARN_ON(maniptype != NF_NAT_MANIP_SRC &&
		maniptype != NF_NAT_MANIP_DST);

	if (WARN_ON(nf_nat_initialized(ct, maniptype)))
		return NF_DROP;


	/* What we've got will look like inverse of reply. Normally
	 * this is what is in the conntrack, except for prior
	 * manipulations (future optimization: if num_manips == 0,
	 * orig_tp = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple)
	 */
	nf_ct_invert_tuplepr(&curr_tuple,
			     &ct->tuplehash[IP_CT_DIR_REPLY].tuple);

	get_unique_v6ports_tuple(&new_tuple, &curr_tuple, range, ct, maniptype, protocol);
	
	if (!nf_ct_tuple_equal(&new_tuple, &curr_tuple)) {
		struct nf_conntrack_tuple reply;

		/* Alter conntrack table so will recognize replies. */
		nf_ct_invert_tuplepr(&reply, &new_tuple);
		nf_conntrack_alter_reply(ct, &reply);

		/* Non-atomic: we own this at the moment. */
		if (maniptype == NF_NAT_MANIP_SRC)
			ct->status |= IPS_SRC_NAT;
		else
			ct->status |= IPS_DST_NAT;
	}

	if (maniptype == NF_NAT_MANIP_SRC) {
		unsigned int srchash;
		spinlock_t *lock;

		srchash = hash_by_src(net,
				      &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
		lock = &nf_nat_locks[srchash % CONNTRACK_LOCKS];
		spin_lock_bh(lock);
		hlist_add_head_rcu(&ct->nat_bysource,
				   &nf_nat_bysource[srchash]);
		spin_unlock_bh(lock);

	}

	/* It's done. */
	if (maniptype == NF_NAT_MANIP_DST)
		ct->status |= IPS_DST_NAT_DONE;
	else
		ct->status |= IPS_SRC_NAT_DONE;

	
	return NF_ACCEPT;
}

static void xt_nat_convert_range_v6plus(struct nf_nat_range_v6ports *dst,
				 const struct nf_nat_ipv4_range_v6ports *src)
{
	int  	i, num;
	
	memset(&dst->min_addr, 0, sizeof(dst->min_addr));
	memset(&dst->max_addr, 0, sizeof(dst->max_addr));

	dst->flags	 = src->flags;
	dst->min_addr.ip = src->min_ip;
	dst->max_addr.ip = src->max_ip;
	
	num =  ntohs(src->min[0].all);
	for(i = 0; i < num ; i ++)
	{
		dst->min_proto[i]	 = src->min[i];
		dst->max_proto[i]	 = src->max[i];
	}
	
}

static unsigned int
xt_v6ports_target_v0(struct sk_buff *skb, const struct xt_action_param *par)
{
	//const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;
	//struct nf_nat_range range;
	const struct nf_nat_ipv4_multi_range_v6ports_compat *mr = par->targinfo;
	struct nf_nat_range_v6ports range;
	
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	struct iphdr *iph = ip_hdr(skb);
	
			 
	ct = nf_ct_get(skb, &ctinfo);
	WARN_ON(!(ct != NULL &&
		     (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED ||
		      ctinfo == IP_CT_RELATED_REPLY)));

	xt_nat_convert_range_v6plus(&range, &mr->range[0]);
	
	if (par->state->out->name[0] == 'b' && par->state->out->name[1] == 'r')
    {
		struct in_device *in_dev = __in_dev_get_rcu(par->state->out);
		__be32 out_mask = in_dev->ifa_list->ifa_mask;
		  /* we only care about the NAT-LoopBack: 
		   * 1) out interface is br+
		  *  2) src addr and dst addr are in the same subnet
		   * 3) DNATed 
		   */
		  
		  if ((iph->saddr & out_mask ) != (iph->daddr & out_mask) ||
				 (!test_bit(IPS_DST_NAT_BIT, &ct->status) || !test_bit(IPS_DST_NAT_DONE_BIT, &ct->status)))
		  {
				 return XT_CONTINUE;
		  }
    }

	return nf_nat_setup_v6ports_info(ct, &range, NF_NAT_MANIP_SRC, iph->protocol);
}

static struct xt_target v6ports_reg __read_mostly = {
        .name           = "V6PORTS",
        .family         = NFPROTO_IPV4,
        .target         = xt_v6ports_target_v0,
        .targetsize  	= sizeof(struct nf_nat_ipv4_multi_range_v6ports_compat),
		.table          = "nat",
        .hooks          = (1 << NF_INET_POST_ROUTING) | (1 << NF_INET_LOCAL_IN),
        .checkentry     = xt_nat_v6ports_checkentry_v0,
        .me             = THIS_MODULE,
};
 
static int __init v6ports_target_init(void)
{
	return xt_register_target(&v6ports_reg);
}
 
static void __exit v6pots_target_exit(void)
{
        xt_unregister_target(&v6ports_reg);
}
 
module_init(v6ports_target_init);
module_exit(v6pots_target_exit);
