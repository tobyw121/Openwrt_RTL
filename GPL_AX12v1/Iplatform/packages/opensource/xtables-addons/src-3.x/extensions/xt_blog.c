/*!Copyright(c) 2020-2024 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     xt_blog.c
 *\brief    userspace/iptables part for blog. 
 *
 *\author   WangLian
 *\version  1.0.0
 *\date     24Feb20
 *
 *\history  \arg 1.0.0, creat this based on "xt_app" mod.  
 *                  
 */


/***************************************************************************/
/*                      CONFIGURATIONS                   */
/***************************************************************************/


/***************************************************************************/
/*                      INCLUDE_FILES                    */
/***************************************************************************/
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>
//#include <log.h>
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include "xt_blog.h"

/***************************************************************************/
/*                      DEFINES                      */
/***************************************************************************/
#define DEBUG       0
 
/***************************************************************************/
/*                      TYPES                            */
/***************************************************************************/


/***************************************************************************/
/*                      EXTERN_PROTOTYPES                    */
/***************************************************************************/


/***************************************************************************/
/*                      LOCAL_PROTOTYPES                     */
/***************************************************************************/

/*!
 *\fn           static bool blog_mt(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the blog_ptr in skb
 *\return       found or not
 */
static bool blog_mt(const struct sk_buff *skb, struct xt_action_param *param);

/*!
 *\fn           static int __init blog_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init blog_init(void);

/*!
 *\fn           static void __exit blog_exit(void)
 *\brief        mod exit
 *\return       none
 */
static void __exit blog_exit(void);


/*kmod-log*/

extern void _log(unsigned int proj_id, unsigned int msg_id, ...);

#define LP_STRING    (1)
#define LP_INT32     (2)
#define LP_UINT32    (3)
#define LP_UINT32HEX (4)
#define LP_IPV4      (5)

#define LP_END       (0)

#define log(proj_id, msg_id, ...)                   \
    _log(proj_id, msg_id, ##__VA_ARGS__, LP_END)

/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/
static struct xt_match blog_mt_reg __read_mostly = { 
    .name           = "blog",
    .family         = NFPROTO_UNSPEC,
    .match          = blog_mt,
    .matchsize      = XT_ALIGN(sizeof(struct _xt_blog_mtinfo)),
    .me             = THIS_MODULE,
};
 
/***************************************************************************/
/*                      LOCAL_FUNCTIONS                  */
/***************************************************************************/
/*!
 *\fn           static bool blog_mt(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the blog_ptr in skb
 *\return       found or not
 */
static bool blog_mt(const struct sk_buff *skb, struct xt_action_param *param)
{   
    const struct _xt_blog_mtinfo *info = param->matchinfo;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	if((info->blog_ptr && blog_ptr(skb))
			|| (!info->blog_ptr && !blog_ptr(skb)))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else
	return FALSE;
#endif
}

/*!
 *\fn           static int __init blog_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init blog_init(void)
{
	int ret = 0;
	ret = xt_register_match(&blog_mt_reg);
	
    return ret;
}

/*!
 *\fn           static void __exit blog_exit(void)
 *\brief        mod exit
 *\return       none
 */
static void __exit blog_exit(void)
{
    xt_unregister_match(&blog_mt_reg);
}

/***************************************************************************/
/*                      PUBLIC_FUNCTIONS                     */
/***************************************************************************/


/***************************************************************************/
/*                      GLOBAL_FUNCTIONS                     */
/***************************************************************************/
module_init(blog_init);
module_exit(blog_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WangLian <wanglian@tp-link.com.cn>");
MODULE_DESCRIPTION("Xtables: blog match");
MODULE_ALIAS("ipt_blog");

