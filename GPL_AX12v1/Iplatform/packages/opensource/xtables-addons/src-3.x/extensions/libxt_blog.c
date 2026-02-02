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
#include "xt_blog.h"


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
 *\fn           static void blog_mt_help(void)
 *\brief        help information
 *\return       N/A
 */
static void blog_mt_help(void);

/*!
 *\fn          static int blog_mt_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match)
 *\return       success or not
 */
static int blog_mt_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match);

/*!
 *\fn           static void blog_mt_check(unsigned int flags)
 *\brief        check the flags. 0 means error.
 *\return       none
 */
static void blog_mt_check(unsigned int flags);

/*!
 *\fn           static void blog_mt_print(const void *entry, const struct xt_entry_match *match, int numeric)
 *\brief        iptables print
 *\return       none
 */
static void blog_mt_print(const void *entry, const struct xt_entry_match *match, int numeric);

/*!
 *\fn           static void blog_mt_save(const void *entry, const struct xt_entry_match *match)
 *\brief        iptables save
 *\return       none
 */
static void blog_mt_save(const void *entry, const struct xt_entry_match *match);


/*!
 *\fn           static void blog_mt_init(struct xt_entry_match *match)
 *\brief        iptables init
 *\return       none
 */
static void blog_mt_init(struct xt_entry_match *match);

/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/

static const struct option blog_mt_opts[] = {
    {.name = "blogptr", .has_arg = true, .val = '1'},
    XT_GETOPT_TABLEEND,
};

static struct xtables_match blog_mt_reg = { 
    .family         = NFPROTO_UNSPEC,
    .name           = "blog",
    .version        = XTABLES_VERSION,
    .size           = XT_ALIGN(sizeof(struct _xt_blog_mtinfo)),
    .userspacesize  = XT_ALIGN(sizeof(struct _xt_blog_mtinfo)),
    .help           = blog_mt_help,
    .parse          = blog_mt_parse,
    .init           = blog_mt_init,
    .final_check    = blog_mt_check,
    .print          = blog_mt_print,
    .save           = blog_mt_save,
    .extra_opts     = blog_mt_opts,
};

/***************************************************************************/
/*                      LOCAL_FUNCTIONS                  */
/***************************************************************************/
/*!
 *\fn           static void blog_mt_help(void)
 *\brief        help information
 *\return       N/A
 */
static void blog_mt_help(void)
{
    printf("blog match options:\n"
    "-m blog --blogptr 0/1\n");
}

/*!
 *\fn           static void blog_mt_print(const void *entry, const struct xt_entry_match *match, int numeric)
 *\brief        iptables print
 *\return       none
 */
static void blog_mt_print(const void *entry, const struct xt_entry_match *match, int numeric)
{
	printf(" -m blog");
	blog_mt_save(entry, match);    
}

/*!
 *\fn           static void blog_mt_save(const void *entry, const struct xt_entry_match *match)
 *\brief        iptables save
 *\return       none
 */
static void blog_mt_save(const void *entry, const struct xt_entry_match *match)
{
    struct _xt_blog_mtinfo *info = (struct _xt_blog_mtinfo *)match->data;
	
    printf(" --blogptr %d", info->blog_ptr);
	printf(" ");
}

/*!
 *\fn          static int blog_mt_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match)
 *\return       success or not
 */
static int blog_mt_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match)
{
    struct _xt_blog_mtinfo *info = (struct _xt_blog_mtinfo *)(*match)->data;
    /* c means the "--" option. look "value" in urlfilter_opts[] */
    switch (c) {
    case '1':
        info->blog_ptr = atoi(optarg);
		//printf("blog_ptr %s --> %d\n", optarg, info->blog_ptr);
        *flags = 1;
        break;

    default:
        return FALSE;
    }
    return TRUE;
}
						 

/*!
 *\fn           static void blog_check(unsigned int flags)
 *\brief        check the flags. 0 means error.
 *\return       none
 */
static void blog_mt_check(unsigned int flags)
{
    if (flags == 0)
    {
        xtables_error(PARAMETER_PROBLEM, "blog match: You must specify `--blogptr'\n ");
    }
}


/*!
 *\fn           static void blog_mt_init(struct xt_entry_match *match)
 *\brief        iptables init
 *\return       none
 */
static void blog_mt_init(struct xt_entry_match *match)
{
    struct _xt_blog_mtinfo *info = (struct _xt_blog_mtinfo *)match->data;
    memset(info, 0, sizeof(struct _xt_blog_mtinfo));
}

/***************************************************************************/
/*                      PUBLIC_FUNCTIONS                     */
/***************************************************************************/
/*!
 *\fn           static void _init(void)
 *\brief        iptables register
 *\return       none
 */
static __attribute__((constructor)) void _blog_mt_ldr(void)
{
    xtables_register_match(&blog_mt_reg);
}

/***************************************************************************/
/*                      GLOBAL_FUNCTIONS                     */
/***************************************************************************/
