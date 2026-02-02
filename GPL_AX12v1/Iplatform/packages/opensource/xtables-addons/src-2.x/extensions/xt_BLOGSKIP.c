/*
 *  xt_BLOGSKIP - Netfilter module to blog skip for some flows.
 *
 *  (C) 2020-2025 Wang Lian <wanglian@tp-link.com.cn>
 *  Copyright ? TP-LINK
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_conntrack.h>

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/version.h>
#include <linux/blog.h>
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Xtables: skip blog learning.");
MODULE_ALIAS("ipt_BLOGSKIP");

static unsigned int
bcm_fcache_blog_skip_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
    if(blog_ptr(skb))
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,38))
        blog_skip(skb, blog_skip_reason_dpi);
#else
        blog_skip(skb);
#endif
    }
#endif

    return XT_CONTINUE;
}

static struct xt_target bcm_fcache_blog_skip_tg_reg __read_mostly = {
    .name           = "BLOGSKIP",
    .revision       = 1,
    .family         = NFPROTO_UNSPEC,
    .target         = bcm_fcache_blog_skip_tg,
    .targetsize     = 0,
    .me             = THIS_MODULE,
};

static int __init blog_skip_mt_init(void)
{
    int ret;
    ret = xt_register_target(&bcm_fcache_blog_skip_tg_reg);
    
    if (ret < 0)
        return ret;
    return 0;
}

static void __exit blog_skip_mt_exit(void)
{
    xt_unregister_target(&bcm_fcache_blog_skip_tg_reg);
}

module_init(blog_skip_mt_init);
module_exit(blog_skip_mt_exit);
