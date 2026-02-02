#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <iptables.h>
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter/nf_nat.h>

#define NF_NAT_RANGE_CHECKPORTS_MAX 	128

enum {
	O_TO_SRC = 0,
	O_RANDOM,
	O_PERSISTENT,
	O_X_TO_SRC,
	F_TO_SRC   = 1 << O_TO_SRC,
	F_RANDOM   = 1 << O_RANDOM,
	F_X_TO_SRC = 1 << O_X_TO_SRC,
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

/* Source NAT data consists of a multi-range, indicating where to map
   to. */
struct ipt_natinfo
{
	struct xt_entry_target t;
	struct nf_nat_ipv4_multi_range_compat mr;
};

struct ipt_checkports_natinfo
{
	struct xt_entry_target t;
	struct nf_nat_ipv4_multi_range_checkports_compat mr;
};

static void CHECKPORTS_help(void)
{
	printf(
"V6PORTS target options:\n"
" --param [ dstMatchIpAddr ]-[ fakeIpAddr ]:[ fileOfPortsSet ]\n"
"				Address map source by Not In Ports Set.\n");
}

static const struct xt_option_entry CHECKPORTS_opts[] = {
	{.name = "param", .id = O_TO_SRC, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_MULTI},
	XTOPT_TABLEEND,
};

/*
static struct ipt_natinfo *
append_range(struct ipt_natinfo *info, const struct nf_nat_ipv4_range *range)
*/
static struct ipt_checkports_natinfo *
append_range(struct ipt_checkports_natinfo *info, const struct nf_nat_ipv4_range_checkports *range)
{
	unsigned int size;

	/* One rangesize already in struct ipt_natinfo */
	size = XT_ALIGN(sizeof(*info) + info->mr.rangesize * sizeof(*range));
	
	info = realloc(info, size);
	if (!info)
		xtables_error(OTHER_PROBLEM, "Out of memory\n");

	info->t.u.target_size = size;
	info->mr.range[info->mr.rangesize] = *range;
	info->mr.rangesize++;
	
	return info;
}

/* Ranges expected in network order. */
/*
static struct xt_entry_target *
parse_to(const char *orig_arg, int portok, struct ipt_natinfo *info)
*/
static struct xt_entry_target *
parse_to(const char *orig_arg, int portok, struct ipt_checkports_natinfo *info)
{
	//struct nf_nat_ipv4_range range;
	
	struct nf_nat_ipv4_range_checkports range;
	FILE * 	fp ; 
	
	char *arg, *colon, *dash, *error;
	const struct in_addr *ip;
	int		i = 1;
	int 	offset ;
	int		psid ;
	int		psidlen;
	
	arg = strdup(orig_arg);
	if (arg == NULL)
		xtables_error(RESOURCE_PROBLEM, "strdup");
	memset(&range, 0, sizeof(range));
	colon = strchr(arg, ':');

	if (colon) {
		int 	port1;
		int 	port2;

		if (!portok)
			xtables_error(PARAMETER_PROBLEM,
				   "Need TCP, UDP, SCTP or DCCP with port specification");

		range.flags |= NF_NAT_RANGE_PROTO_SPECIFIED;
		
		fp = fopen(colon+1, "r");
		if(fp)
		{
			fscanf(fp, "%d", &offset);
			fscanf(fp, "%d", &psid);
			fscanf(fp, "%d", &psidlen);
			i=1;
			while(!feof(fp))
			{
				if (i >= NF_NAT_RANGE_CHECKPORTS_MAX)
					xtables_error(RESOURCE_PROBLEM, "port range too much");

				fscanf(fp, "%d-%d", &port1, &port2);	
				range.min[i].tcp.port = htons(port1);
				range.max[i].tcp.port = htons(port2);
				i++;
			}			
			fclose(fp);
			range.min[0].tcp.port = htons(i-1);
			range.max[0].tcp.port = htons(i-1);
		}
		
		/* Starts with a colon? No IP info...*/
		if (colon == arg) {
			free(arg);
			return &(append_range(info, &range)->t);
		}
		*colon = '\0';
	}

	range.flags |= NF_NAT_RANGE_MAP_IPS;
	dash = strchr(arg, '-');
	if (colon && dash && dash > colon)
		dash = NULL;

	if (dash)
		*dash = '\0';

	ip = xtables_numeric_to_ipaddr(arg);
	if (!ip)
		xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
			   arg);
	range.match_ip = ip->s_addr;
	if (dash) {
		ip = xtables_numeric_to_ipaddr(dash+1);
		if (!ip)
			xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
				   dash+1);
		range.fake_ip = ip->s_addr;
	} else
		range.fake_ip = range.match_ip;
	
	free(arg);
	return &(append_range(info, &range)->t);
}

static void CHECKPORTS_parse(struct xt_option_call *cb)
{
	const struct ipt_entry *entry = cb->xt_entry;
	//struct ipt_natinfo *info = (void *)(*cb->target);
	struct ipt_checkports_natinfo *info = (void *)(*cb->target);
	
	int portok;

	if (entry->ip.proto == IPPROTO_TCP
	    || entry->ip.proto == IPPROTO_UDP
	    || entry->ip.proto == IPPROTO_SCTP
	    || entry->ip.proto == IPPROTO_DCCP
	    || entry->ip.proto == IPPROTO_ICMP)
		portok = 1;
	else
		portok = 0;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TO_SRC:
		if (cb->xflags & F_X_TO_SRC) {
			if (!kernel_version)
				get_kernel_version();
			if (kernel_version > LINUX_VERSION(2, 6, 10))
				xtables_error(PARAMETER_PROBLEM,
					   "CHECKPORTS: Multiple --to-source not supported");
		}
		*cb->target = parse_to(cb->arg, portok, info);
		cb->xflags |= F_X_TO_SRC;
		break;
	}
}

static void CHECKPORTS_fcheck(struct xt_fcheck_call *cb)
{
	static const unsigned int f = F_TO_SRC | F_RANDOM;
	struct nf_nat_ipv4_multi_range_compat *mr = cb->data;
    /*
		do nothing 
    */
}

//static void print_range(const struct nf_nat_ipv4_range *r)
static void print_range(const struct nf_nat_ipv4_range_checkports *r)
{
	int	 i ;
	int  num;
	
	if (r->flags & NF_NAT_RANGE_MAP_IPS) {
		struct in_addr a;

		a.s_addr = r->match_ip;
		printf("%s", xtables_ipaddr_to_numeric(&a));
		if (r->fake_ip != r->match_ip) {
			a.s_addr = r->fake_ip;
			printf("-%s", xtables_ipaddr_to_numeric(&a));
		}
	}
	num = ntohs(r->min[0].all);
	printf(":");
	for(i = 1; i < num; i++)
	{
		if (r->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
			if(i != 1)
				printf(",");
			printf("%hu", ntohs(r->min[i].tcp.port));
			if (r->max[i].tcp.port != r->min[i].tcp.port)
				printf("-%hu", ntohs(r->max[i].tcp.port));
		}
	}
	
}

static void CHECKPORTS_print(const void *ip, const struct xt_entry_target *target,
                       int numeric)
{
	//const struct ipt_natinfo *info = (const void *)target;
	const struct ipt_checkports_natinfo *info = (const void *)target;
	unsigned int i = 0;

	printf(" to:");
	for (i = 0; i < info->mr.rangesize; i++) {
		print_range(&info->mr.range[i]);
	}
}

static void CHECKPORTS_save(const void *ip, const struct xt_entry_target *target)
{
	//const struct ipt_natinfo *info = (const void *)target;
	const struct ipt_checkports_natinfo *info = (const void *)target;
	unsigned int i = 0;

	for (i = 0; i < info->mr.rangesize; i++) {
		printf(" --param ");
		print_range(&info->mr.range[i]);
	}
}

static struct xtables_target checkports_tg_reg = {
	.name		= "CHECKPORTS",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV4,
	//.size		= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
	//.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
	.size		= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_checkports_compat)),
	.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_checkports_compat)),
	.help		= CHECKPORTS_help,
	.x6_parse	= CHECKPORTS_parse,
	.x6_fcheck	= CHECKPORTS_fcheck,
	.print		= CHECKPORTS_print,
	.save		= CHECKPORTS_save,
	.x6_options	= CHECKPORTS_opts,
};

void _init(void)
{
	xtables_register_target(&checkports_tg_reg);
}
