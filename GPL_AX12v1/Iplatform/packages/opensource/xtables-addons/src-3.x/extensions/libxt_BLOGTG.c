/*!Copyright(c) 2020-2024 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     libxt_blog.c
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
#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <netinet/in.h>
#include <xtables.h>
#include <linux/netfilter.h>
#include "xt_BLOGTG.h"


/***************************************************************************/
/*                      DEFINES                      */
/***************************************************************************/

 
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
 *\fn           static void blog_tg_help(void)
 *\brief        help information
 *\return       N/A
 */
static void blog_tg_help(void);

/*!
 *\fn          static int blog_tg_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_target **target)
 *\return       success or not
 */
static int blog_tg_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_target **target);

/*!
 *\fn           static void blog_tg_check(unsigned int flags)
 *\brief        check the flags. 0 means error.
 *\return       none
 */
static void blog_tg_check(unsigned int flags);

/*!
 *\fn           static void blog_tg_print(const void *entry, const struct xt_entry_target *target, int numeric)
 *\brief        iptables print
 *\return       none
 */
static void blog_tg_print(const void *entry, const struct xt_entry_target *target, int numeric);

/*!
 *\fn           static void blog_tg_save(const void *entry, const struct xt_entry_target *target)
 *\brief        iptables save
 *\return       none
 */
static void blog_tg_save(const void *entry, const struct xt_entry_target *target);


/*!
 *\fn           static void blog_tg_init(struct xt_entry_target *target)
 *\brief        iptables init
 *\return       none
 */
static void blog_tg_init(struct xt_entry_target *target);


/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/

static const struct option blog_tg_opts[] = {
	{.name = "set-tcflag", .has_arg = true, .val = '1'},
	{NULL},
};

static struct xtables_target blog_tg_reg = {
	.version       = XTABLES_VERSION,
	.name          = "BLOGTG",
	.family        = NFPROTO_UNSPEC,
	.revision      = 1,
	.size          = XT_ALIGN(sizeof(struct _xt_blog_tginfo)),
	.userspacesize = XT_ALIGN(sizeof(struct _xt_blog_tginfo)),
	.help          = blog_tg_help,
	.init          = blog_tg_init,
	.parse         = blog_tg_parse,
	.final_check   = blog_tg_check,
	.print         = blog_tg_print,
	.save          = blog_tg_save,
	.extra_opts    = blog_tg_opts,
};

/***************************************************************************/
/*                      LOCAL_FUNCTIONS                  */
/***************************************************************************/

/*!
 *\fn           static void blog_tg_help(void)
 *\brief        help information
 *\return       N/A
 */
static void blog_tg_help(void)
{
	
	printf(
"BLOG target options:\n"
"  --set-tcflag 0|1    mark tc flags or not\n"
"\n");
}

/*!
 *\fn          static int blog_tg_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_target **target)
 *\return       success or not
 */
static int blog_tg_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_target **target)
{
	struct _xt_blog_tginfo *info = (void *)(*target)->data;

	switch (c) {
	case '1':
		info->tc_flag = atoi(optarg);
		*flags = 1;
		return true;
	default:
		return false;
	}

	return false;
}

/*!
 *\fn           static void blog_tg_check(unsigned int flags)
 *\brief        check the flags. 0 means error.
 *\return       none
 */
static void blog_tg_check(unsigned int flags)
{
	if (flags == 0)
    {
        xtables_error(PARAMETER_PROBLEM, "blog target: You must specify `--set-tcflag'\n ");
    }
}

/*!
 *\fn           static void blog_tg_print(const void *entry, const struct xt_entry_target *target, int numeric)
 *\brief        iptables print
 *\return       none
 */
static void blog_tg_print(const void *entry, const struct xt_entry_target *target, int numeric)
{
	printf(" -j BLOGTG");
	blog_tg_save(entry, target);
}

/*!
 *\fn           static void blog_tg_save(const void *entry, const struct xt_entry_target *target)
 *\brief        iptables save
 *\return       none
 */
static void blog_tg_save(const void *entry, const struct xt_entry_target *target)
{
	const struct _xt_blog_tginfo *info = (const void *)target->data;
	printf(" --set-tcflag %d", info->tc_flag);
	printf(" ");
}

/*!
 *\fn           static void blog_tg_init(struct xt_entry_target *target)
 *\brief        iptables init
 *\return       none
 */
static void blog_tg_init(struct xt_entry_target *target)
{
	struct _xt_blog_tginfo *info = (void *)target->data;
	memset(info, 0, sizeof(struct _xt_blog_tginfo));
}

/***************************************************************************/
/*                      PUBLIC_FUNCTIONS                     */
/***************************************************************************/
/*!
 *\fn           static void _init(void)
 *\brief        iptables register
 *\return       none
 */
static __attribute__((constructor)) void _blog_tg_ldr(void)
{
	xtables_register_target(&blog_tg_reg);
}

/***************************************************************************/
/*                      GLOBAL_FUNCTIONS                     */
/***************************************************************************/
