/* 匹配规则，自动执行NAT loopback 的DNAT部分 */
/* (C) 2018/11/01 By SuWeilin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
 struct xt_LOOPBACKDNAT_info
 {
	 __be32 dst_ip;
 };
 
 struct ipt_natinfo
{
        struct xt_entry_target t;
        struct xt_LOOPBACKDNAT_info info;
};
 
 enum {
	O_DSTIP = 0,
};
 
static void LOOPBACKDNAT_help(void)
{
        printf(
                "LOOPBACKDNAT target options:\n"
				"[!] --dstip destination IP\n");
}

static void LOOPBACKDNAT_parse(struct xt_option_call *cb)
{
	const struct ipt_entry *entry = cb->xt_entry;
	struct ipt_natinfo *iptinfo = (void *)(*cb->target);
	char *arg;
	const struct in_addr *ip;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_DSTIP:
		arg = strdup(cb->arg);
		if (arg == NULL)
			xtables_error(RESOURCE_PROBLEM, "strdup");
		ip = xtables_numeric_to_ipaddr(arg);
		if (!ip)
			xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
				   arg);
		iptinfo->info.dst_ip = ip->s_addr;
		free(arg);
		break;
	}
}

static void LOOPBACKDNAT_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	const struct xt_LOOPBACKDNAT_info *info = (void *)target->data;
	struct in_addr a;
	
	a.s_addr = info->dst_ip;
	printf(" dstip %s", xtables_ipaddr_to_numeric(&a));
}

static void LOOPBACKDNAT_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_LOOPBACKDNAT_info *info = (void *)target->data;
	struct in_addr a;

	a.s_addr = info->dst_ip;
	printf(" --dstip %s", xtables_ipaddr_to_numeric(&a));
}

static const struct xt_option_entry LOOPBACKDNAT_opts[] = {
	{.name = "dstip", .id = O_DSTIP, .type = XTTYPE_STRING,
	 .flags = XTOPT_PUT,
	 XTOPT_POINTER(struct xt_LOOPBACKDNAT_info, dst_ip)},
	XTOPT_TABLEEND,
};
 
static struct xtables_target loopbackDNAT_tg_reg = {
        .name           = "LOOPBACKDNAT",
        .version        = XTABLES_VERSION,
        .family         = NFPROTO_IPV4,
        .size			= XT_ALIGN(sizeof(struct xt_LOOPBACKDNAT_info)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_LOOPBACKDNAT_info)),
        .help           = LOOPBACKDNAT_help,
		.print			= LOOPBACKDNAT_print,
		.save			= LOOPBACKDNAT_save,
		.x6_parse       = LOOPBACKDNAT_parse,
        .x6_options     = LOOPBACKDNAT_opts,
};
 
void _init(void)
{
        xtables_register_target(&loopbackDNAT_tg_reg);
}
