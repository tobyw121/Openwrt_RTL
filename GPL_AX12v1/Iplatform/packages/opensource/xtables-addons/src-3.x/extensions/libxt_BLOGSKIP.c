/* Shared library add-on to iptables for skip blog.
 * (C) 2020 by Wang Lian <wanglian@tp-link.com.cn>
 *
 * $Id$
 *
 * This program is distributed under the terms of GNU GPL
 */
#include <stdio.h>
#include <xtables.h>

static struct xtables_target blog_skip_tg_reg = {
	.name			= "BLOGSKIP",
	.version		= XTABLES_VERSION,
	.family			= NFPROTO_UNSPEC,
	.revision		= 1,
	.size			= XT_ALIGN(0),
	.userspacesize	= XT_ALIGN(0),
};

void _init(void)
{
	xtables_register_target(&blog_skip_tg_reg);
}