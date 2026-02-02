/*!Copyright(c) 2017-2018 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     libxt_pctl.c
 *\brief    userspace/iptables part for parental control. 
 *
 *\author   Hu Luyao
 *\version  1.0.0
 *\date     23Dec13
 *
 *\history  \arg 1.0.0, creat this based on "httphost" from IPF
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
#include "xt_pctl.h"


/***************************************************************************/
/*                      DEFINES                      */
/***************************************************************************/

 
/***************************************************************************/
/*                      TYPES                            */
/***************************************************************************/
static const struct option pctl_opts[] = {
	{.name = "advancedMode", .has_arg = true, .val = '0'},//add by wanghao
    {.name = "id", .has_arg = true, .val = '1'},
    {.name = "blocked", .has_arg = true, .val = '2'},
	/* add by wanghao */
	{.name = "workdays", .has_arg = true, .val = '3'},
	{.name = "today_bonus_time", .has_arg = true, .val = '4'},
	/* add end */
    {.name = "workday_limit", .has_arg = true, .val = '5'},
    {.name = "workday_time", .has_arg = true, .val = '6'},
    {.name = "workday_bedtime", .has_arg = true, .val = '7'},
    {.name = "workday_begin", .has_arg = true, .val = '8'},
    {.name = "workday_end", .has_arg = true, .val = '9'},
    {.name = "weekend_limit", .has_arg = true, .val = 'a'},
    {.name = "weekend_time", .has_arg = true, .val = 'b'},
    {.name = "weekend_bedtime", .has_arg = true, .val = 'c'},
    {.name = "weekend_begin", .has_arg = true, .val = 'd'},
    {.name = "weekend_end", .has_arg = true, .val = 'e'},
    /* add by wanghao */
    {.name = "cat_map", .has_arg = true, .val = 'f'},
    {.name = "host_wl", .has_arg = true, .val = 16},
    /* add end */
    {.name = "host", .has_arg = true, .val = 17},
	/* add by wanghao */
    {.name = "sun_time", .has_arg = true, .val = 18},
    {.name = "sun_forenoon", .has_arg = true, .val = 19},
    {.name = "sun_afternoon", .has_arg = true, .val = 20},
    {.name = "mon_time", .has_arg = true, .val = 21},
    {.name = "mon_forenoon", .has_arg = true, .val = 22},
    {.name = "mon_afternoon", .has_arg = true, .val = 23},
    {.name = "tue_time", .has_arg = true, .val = 24},
    {.name = "tue_forenoon", .has_arg = true, .val = 25},
    {.name = "tue_afternoon", .has_arg = true, .val = 26},
    {.name = "wed_time", .has_arg = true, .val = 27},
    {.name = "wed_forenoon", .has_arg = true, .val = 28},
    {.name = "wed_afternoon", .has_arg = true, .val = 29},
    {.name = "thu_time", .has_arg = true, .val = 30},
    {.name = "thu_forenoon", .has_arg = true, .val = 31},
    {.name = "thu_afternoon", .has_arg = true, .val = 32},
    {.name = "fri_time", .has_arg = true, .val = 33},
    {.name = "fri_forenoon", .has_arg = true, .val = 34},
    {.name = "fri_afternoon", .has_arg = true, .val = 35},
    {.name = "sat_time", .has_arg = true, .val = 36},
    {.name = "sat_forenoon", .has_arg = true, .val = 37},
    {.name = "sat_afternoon", .has_arg = true, .val = 38},
    {.name = "advanced_enable", .has_arg = true, .val = 39},
	/* add end */
    {.name = "hosts_type", .has_arg = true, .val = 40},
    {.name = "today_reward_time", .has_arg = true, .val = 41},
    XT_GETOPT_TABLEEND,
};


/***************************************************************************/
/*                      EXTERN_PROTOTYPES                    */
/***************************************************************************/


/***************************************************************************/
/*                      LOCAL_PROTOTYPES                     */
/***************************************************************************/
/*!
 *\fn           static void pctl_help(void)
 *\brief        help information
 *\return       N/A
 */
static void pctl_help(void);

/* add by wanghao */
/*!
 *\fn           static int _parse_write_wl(const char *host, size_t len, struct _xt_pctl_info *info)
 *\brief        write urls into info->hosts_wl[]
 *\return       N/A
 */
static int _parse_write_wl(const char *host, size_t len, struct _xt_pctl_info *info);

/*!
 *\fn           static void _parse_spilt_wl(const char *arg, struct _xt_pctl_info *info)
 *\brief        
 *\return       none
 */
static void _parse_spilt_wl(const char *arg, struct _xt_pctl_info *info);
/* add end */

/*!
 *\fn           static int _parse_write(const char *host, size_t len, struct _xt_pctl_info *info)
 *\brief        write urls into info->hosts[]
 *\return       N/A
 */
static int _parse_write(const char *host, size_t len, struct _xt_pctl_info *info);

/*!
 *\fn           static void _parse_spilt(const char *arg, struct _xt_pctl_info *info)
 *\brief        
 *\return       none
 */
static void _parse_spilt(const char *arg, struct _xt_pctl_info *info);

/*!
 *\fn          static int pctl_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match)
 *\brief        xt_entry_match **match 
 *\return       success or not
 */
static int pctl_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match);

/*!
 *\fn           static void pctl_check(unsigned int flags)
 *\brief        check the flags. 0 means error.
 *\return       none
 */
static void pctl_check(unsigned int flags);

/*!
 *\fn           static void pctl_print(const void *ip, const struct xt_entry_match *match, int numeric)
 *\brief        iptables print
 *\return       none
 */
static void pctl_print(const void *ip, const struct xt_entry_match *match, int numeric);

/*!
 *\fn           static void pctl_save(const void *ip, const struct xt_entry_match *match)
 *\brief        iptables save
 *\return       none
 */
static void pctl_save(const void *ip, const struct xt_entry_match *match);

/*!
 *\fn           static void pctl_init(struct xt_entry_match *match)
 *\brief        iptables init
 *\return       none
 */
static void pctl_init(struct xt_entry_match *match);

/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/
static struct xtables_match pctl_match = { 
    .family         = NFPROTO_UNSPEC,
    .name           = "pctl",
    .version        = XTABLES_VERSION,
    .size           = XT_ALIGN(sizeof(struct _xt_pctl_info)),
    .userspacesize  = XT_ALIGN(sizeof(struct _xt_pctl_info)),
    .help           = pctl_help,
    .parse          = pctl_parse,
    .init           = pctl_init,
    .final_check    = pctl_check,
    .print          = pctl_print,
    .save           = pctl_save,
    .extra_opts     = pctl_opts,
};

 
/***************************************************************************/
/*                      LOCAL_FUNCTIONS                  */
/***************************************************************************/
/*!
 *\fn           static void pctl_help(void)
 *\brief        help information
 *\return       N/A
 */
static void pctl_help(void)
{
	printf(
"IPMARK target options:\n"
"  --advancedMode value          time limit mode.\n"
"  --id value               Child's id\n"
"  --blocked value          Internet Paused.\n"
"  --workdays value          work day definition.\n"
"  --today_bonus_time value          bonus time of today.\n"
"  --today_reward_time value          rewards time of today.\n"
"  --workday_limit value    Is workday limit enabled ?\n"
"  --workday_time value     \n"
"  --workday_bedtime value     \n"
"  --workday_begin value    \n"
"  --workday_end value      \n"
"  --weekend_limit value    Is weekend limit enabled ?\n"
"  --weekend_time value     \n"
"  --weekend_bedtime value     \n"
"  --weekend_begin value    \n"
"  --weekend_end value      \n"
"  --cat_map value     categories that not accessible \n"
"  --host_wl host1[,host2][,host3]    white list of URLs\n"
"  --host host1[,host2][,host3]    black list of URLs\n"
"  --advanced_enable value          \n"
"  --sun_time value          \n"
"  --sun_forenoon value          \n"
"  --sun_afternoon value          \n"
"  --mon_time value          \n"
"  --mon_forenoon value          \n"
"  --tue_afternoon value          \n"
"  --tue_time value          \n"
"  --tue_forenoon value          \n"
"  --tue_afternoon value          \n"
"  --wed_time value          \n"
"  --wed_forenoon value          \n"
"  --wed_afternoon value          \n"
"  --thu_time value          \n"
"  --thu_forenoon value          \n"
"  --thu_afternoon value          \n"
"  --fri_time value          \n"
"  --fri_forenoon value          \n"
"  --fri_afternoon value          \n"
"  --sat_time value          \n"
"  --sat_forenoon value          \n"
"  --sat_afternoon value          \n"
"  --hosts_type value    \n"
"\n");
}

/* add by wanghao */
/*!
 *\fn           static int _parse_write(const char *host, size_t len, struct _xt_pctl_info *info)
 *\brief        write urls into info->hosts_wl[]
 *\return       N/A
 */
static int _parse_write_wl(const char *host, size_t len, struct _xt_pctl_info *info)
{
    /*
    *  merge from deco: if host too long, copy 'max_len - 1' characters.
    */

    if(len >= PCTL_URL_LEN) {
        if (strncpy(info->hosts_wl[info->num_wl], host, PCTL_URL_LEN-1) == NULL)
        {
            return FALSE;
        }
    }
    else if (strncpy(info->hosts_wl[info->num_wl], host, len) == NULL)
    {
        return FALSE;
    }
    info->num_wl ++;
    return TRUE;
}

/*!
 *\fn           static void _parse_spilt_wl(const char *arg, struct _xt_pctl_info *info)
 *\brief        
 *\return       none
 */
static void _parse_spilt_wl(const char *arg, struct _xt_pctl_info *info)
{
    char *pIndex;

    /* write arg to info*/
    while ((pIndex = strchr(arg, ',')) != NULL)
    {   
        if (pIndex == arg || !_parse_write_wl(arg, pIndex - arg, info))
        {
            xtables_error(PARAMETER_PROBLEM, "wl:Bad host name \"%s\"", arg);
        }
        arg = pIndex + 1;
    }

    if (!*arg)
    {
        xtables_error(PARAMETER_PROBLEM, "\"--host_wl\" requires a list of "
                          "host name with no spaces, e.g. "
                          "www.baidu.com,www.taobao.com");
    }

    if (strlen(arg) == 0 || !_parse_write_wl(arg, strlen(arg), info))
    {
        xtables_error(PARAMETER_PROBLEM, "wl:Bad host name \"%s\"", arg);
    }
}
/* add end */

/*!
 *\fn           static int _parse_write(const char *host, size_t len, struct _xt_pctl_info *info)
 *\brief        write urls into info->hosts[]
 *\return       N/A
 */
static int _parse_write(const char *host, size_t len, struct _xt_pctl_info *info)
{
    /*
    *  merge from deco: if host too long, copy 'max_len - 1' characters.
    */

    if(len >= PCTL_URL_LEN) {
        if (strncpy(info->hosts[info->num], host, PCTL_URL_LEN-1) == NULL)
        {
            return FALSE;
        }
    }
    else if (strncpy(info->hosts[info->num], host, len) == NULL)
    {
        return FALSE;
    }
    info->num ++;
    return TRUE;
}

/*!
 *\fn           static void _parse_spilt(const char *arg, struct _xt_pctl_info *info)
 *\brief        
 *\return       none
 */
static void _parse_spilt(const char *arg, struct _xt_pctl_info *info)
{
    char *pIndex;

    /* write arg to info*/
    while ((pIndex = strchr(arg, ',')) != NULL)
    {   
        if (pIndex == arg || !_parse_write(arg, pIndex - arg, info))
        {
            xtables_error(PARAMETER_PROBLEM, "Bad host name \"%s\"", arg);
        }
        arg = pIndex + 1;
    }

    if (!*arg)
    {
        xtables_error(PARAMETER_PROBLEM, "\"--host\" requires a list of "
                          "host name with no spaces, e.g. "
                          "www.baidu.com,www.taobao.com");
    }

    if (strlen(arg) == 0 || !_parse_write(arg, strlen(arg), info))
    {
        xtables_error(PARAMETER_PROBLEM, "Bad host name \"%s\"", arg);
    }
}

/*!
 *\fn          static int pctl_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match)
 *\brief        xt_entry_match **match 
 *\return       success or not
 */
static int pctl_parse(int c, char **argv, int invert, unsigned int *flags,
                         const void *entry, struct xt_entry_match **match)
{
    int i;
    struct _xt_pctl_info *info = (struct _xt_pctl_info *)(*match)->data;
    /* c means the "--" option. look "value" in urlfilter_opts[] */
    switch (c) {
	/* add by wanghao */
	case '0':	/* advancedMode */
		info->advancedMode = atoi(optarg);
        break;
	/* add end */
		
    case '1':       /* id */
        info->id = atoi(optarg);
        *flags = 1;
        break;

    case '2':       /* blocked */
        info->blocked = atoi(optarg);
        break;

	/* add by wanghao */
	case '3':       /* workdays */
        info->workdays = atoi(optarg);
        break;

    case '4':       /* today_bonus_time */
        info->today_bonus_time = atoi(optarg);
        break;
	/* add end */
		
    case '5':       /* workday_limit */
        info->workday_limit = atoi(optarg);
        break;

    case '6':       /* workday_time */
        info->workday_time = atoi(optarg);
        break;

    case '7':       /* workday_bedtime */
        info->workday_bedtime = atoi(optarg);
        break;

    case '8':       /* workday_begin */
        info->workday_begin = atoi(optarg);
        break;

    case '9':       /* workday_end */
        info->workday_end = atoi(optarg);
        break;

    case 'a':       /* weekend_limit */
        info->weekend_limit = atoi(optarg);
        break;

    case 'b':       /* weekend_time */
        info->weekend_time = atoi(optarg);
        break;

    case 'c':       /* weekend_bedtime */
        info->weekend_bedtime = atoi(optarg);
        break;

    case 'd':       /* weekend_begin */
        info->weekend_begin = atoi(optarg);
        break;

    case 'e':       /* weekend_end */
        info->weekend_end = atoi(optarg);
        break;

	/* add by wanghao */		
	case 'f':		/* cat_map */
		info->cat_map = atoi(optarg);
		break;
	
	case 16:       /* hosts_wl */
        _parse_spilt_wl(optarg, info);
        for (i = 0; i < PCTL_URL_NUM; ++i)
        {
            if ('\0' == *(info->hosts_wl[i]))
            {
                break;
            }
        }
        break;
	/* add end */

    case 17:     /* hosts */
        _parse_spilt(optarg, info);
        for (i = 0; i < PCTL_URL_NUM; ++i)
        {
            if ('\0' == *(info->hosts[i]))
            {
                break;
            }
        }
        break;

	/* add by wanghao */
	case 18:
		info->sun_time = atoi(optarg);
		break;
	case 19:
		info->sun_forenoon = atoi(optarg);
		break;
	case 20:
		info->sun_afternoon = atoi(optarg);
		break;
	case 21:
		info->mon_time = atoi(optarg);
		break;
	case 22:
		info->mon_forenoon = atoi(optarg);
		break;
	case 23:
		info->mon_afternoon = atoi(optarg);
		break;
	case 24:
		info->tue_time = atoi(optarg);
		break;
	case 25:
		info->tue_forenoon = atoi(optarg);
		break;
	case 26:
		info->tue_afternoon = atoi(optarg);
		break;
	case 27:
		info->wed_time = atoi(optarg);
		break;
	case 28:
		info->wed_forenoon = atoi(optarg);
		break;
	case 29:
		info->wed_afternoon = atoi(optarg);
		break;
	case 30:
		info->thu_time = atoi(optarg);
		break;
	case 31:
		info->thu_forenoon = atoi(optarg);
		break;
	case 32:
		info->thu_afternoon = atoi(optarg);
		break;
	case 33:
		info->fri_time = atoi(optarg);
		break;
	case 34:
		info->fri_forenoon = atoi(optarg);
		break;
	case 35:
		info->fri_afternoon = atoi(optarg);
		break;
	case 36:
		info->sat_time = atoi(optarg);
		break;
	case 37:
		info->sat_forenoon = atoi(optarg);
		break;
	case 38:
		info->sat_afternoon = atoi(optarg);
		break;
	case 39:
		info->advanced_enable = atoi(optarg);
		break;
	/* add end */

	case 40:
		info->hosts_type = atoi(optarg);
		break;

    case 41:       /* today_reward_time */
		info->today_reward_time = atoi(optarg);
		break;

    default:
        return FALSE;
    }
    return TRUE;
}

/*!
 *\fn           static void pctl_check(unsigned int flags)
 *\brief        check the flags. 0 means error.
 *\return       none
 */
static void pctl_check(unsigned int flags)
{
    if (flags == 0)
    {
        xtables_error(PARAMETER_PROBLEM, "pctl match: You must specify `--id'\n ");
    }
}

/*!
 *\fn           static void pctl_print(const void *ip, const struct xt_entry_match *match, int numeric)
 *\brief        iptables print
 *\return       none
 */
static void pctl_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
    struct _xt_pctl_info *info = (struct _xt_pctl_info *)match->data;
    int i = 0;

	printf(" --advancedMode %d", info->advancedMode);//add by wanghao
    printf(" --id %d", info->id);

    if(PCTL_OWNER_ID_ALL != info->id) {
        printf(" --blocked %d", info->blocked);
		/* add by wanghao */
		if (!info->advancedMode) {
			printf(" --workdays %d", info->workdays);
		}
		printf(" --today_bonus_time %d", info->today_bonus_time);
        printf(" --today_reward_time %d", info->today_reward_time);
		/* add end */
		if (!info->advancedMode) {
	        printf(" --workday_limit %d", info->workday_limit);
	        printf(" --workday_time %d", info->workday_time);
	        printf(" --workday_bedtime %d", info->workday_bedtime);
	        printf(" --workday_begin %d", info->workday_begin);
	        printf(" --workday_end %d", info->workday_end);
	        printf(" --weekend_limit %d", info->weekend_limit);
	        printf(" --weekend_time %d", info->weekend_time);
	        printf(" --weekend_bedtime %d", info->weekend_bedtime);
	        printf(" --weekend_begin %d", info->weekend_begin);
	        printf(" --weekend_end %d", info->weekend_end);
	        printf(" --sun_time %d", info->sun_time);
	        printf(" --mon_time %d", info->mon_time);
	        printf(" --tue_time %d", info->tue_time);
	        printf(" --wed_time %d", info->wed_time);
	        printf(" --thu_time %d", info->thu_time);
	        printf(" --fri_time %d", info->fri_time);
	        printf(" --sat_time %d", info->sat_time);
	        printf(" --hosts_type %d", info->hosts_type);
		}
		printf(" --cat_map %d", info->cat_map);

		/* add by wanghao */
		if(info->num_wl > 0) 
        {
            printf(" --host_wl ");
            for(i=0; i<info->num_wl; i++)
            {
                if(i < info->num_wl - 1)
                {
                    printf("%s,",info->hosts_wl[i]);
                }else
                {
                    printf("%s",info->hosts_wl[i]);
                }
            }
        }
		/* add end */

        if(info->num > 0) 
        {
            printf(" --host ");
            for(i=0; i<info->num; i++)
            {
                if(i < info->num - 1)
                {
                    printf("%s,",info->hosts[i]);
                }else
                {
                    printf("%s",info->hosts[i]);
                }
            }
        }

		/* add by wanghao */
		if (info->advancedMode) {
			printf(" --advanced_enable %d", info->advanced_enable);
			printf(" --sun_time %d", info->sun_time);
			printf(" --sun_forenoon %d", info->sun_forenoon);
			printf(" --sun_afternoon %d", info->sun_afternoon);
			printf(" --mon_time %d", info->mon_time);
			printf(" --mon_forenoon %d", info->mon_forenoon);
			printf(" --mon_afternoon %d", info->mon_afternoon);
			printf(" --tue_time %d", info->tue_time);
			printf(" --tue_forenoon %d", info->tue_forenoon);
			printf(" --tue_afternoon %d", info->tue_afternoon);
			printf(" --wed_time %d", info->wed_time);
			printf(" --wed_forenoon %d", info->wed_forenoon);
			printf(" --wed_afternoon %d", info->wed_afternoon);
			printf(" --thu_time %d", info->thu_time);
			printf(" --thu_forenoon %d", info->thu_forenoon);
			printf(" --thu_afternoon %d", info->thu_afternoon);
			printf(" --fri_time %d", info->fri_time);
			printf(" --fri_forenoon %d", info->fri_forenoon);
			printf(" --fri_afternoon %d", info->fri_afternoon);
			printf(" --sat_time %d", info->sat_time);
			printf(" --sat_forenoon %d", info->sat_forenoon);
			printf(" --sat_afternoon %d", info->sat_afternoon);
		}
		/* add end*/
    }
    printf("\n");
}

/*!
 *\fn           static void pctl_save(const void *ip, const struct xt_entry_match *match)
 *\brief        iptables save
 *\return       none
 */
static void pctl_save(const void *ip, const struct xt_entry_match *match)
{
    struct _xt_pctl_info *info = (struct _xt_pctl_info *)match->data;
    int i = 0;

	printf(" --advancedMode %d", info->advancedMode);//add by wanghao
    printf(" --id %d", info->id);

    if(PCTL_OWNER_ID_ALL != info->id) {
        printf(" --blocked %d", info->blocked);
		/* add by wanghao */
		if (!info->advancedMode) {
			printf(" --workdays %d", info->workdays);
		}
		printf(" --today_bonus_time %d", info->today_bonus_time);
        printf(" --today_reward_time %d", info->today_reward_time);
		/* add end */
		if (!info->advancedMode) {
	        printf(" --workday_limit %d", info->workday_limit);
	        printf(" --workday_time %d", info->workday_time);
	        printf(" --workday_bedtime %d", info->workday_bedtime);
	        printf(" --workday_begin %d", info->workday_begin);
	        printf(" --workday_end %d", info->workday_end);
	        printf(" --weekend_limit %d", info->weekend_limit);
	        printf(" --weekend_time %d", info->weekend_time);
	        printf(" --weekend_bedtime %d", info->weekend_bedtime);
	        printf(" --weekend_begin %d", info->weekend_begin);
	        printf(" --weekend_end %d", info->weekend_end);
		}
		printf(" --cat_map %d", info->cat_map);

		/* add by wanghao */
		if(info->num_wl > 0) 
        {
            printf(" --host_wl ");
            for(i=0; i<info->num_wl; i++)
            {
                if(i < info->num_wl - 1)
                {
                    printf("%s,",info->hosts_wl[i]);
                }else
                {
                    printf("%s",info->hosts_wl[i]);
                }
            }
        }
		/* add end */

        if(info->num > 0) 
        {
            printf(" --host ");
            for(i=0; i<info->num; i++)
            {
                if(i < info->num - 1)
                {
                    printf("%s,",info->hosts[i]);
                }else
                {
                    printf("%s",info->hosts[i]);
                }
            }
        }

		/* add by wanghao */
		if (info->advancedMode) {
			printf(" --advanced_enable %d", info->advanced_enable);
			printf(" --sun_time %d", info->sun_time);
			printf(" --sun_forenoon %d", info->sun_forenoon);
			printf(" --sun_afternoon %d", info->sun_afternoon);
			printf(" --mon_time %d", info->mon_time);
			printf(" --mon_forenoon %d", info->mon_forenoon);
			printf(" --mon_afternoon %d", info->mon_afternoon);
			printf(" --tue_time %d", info->tue_time);
			printf(" --tue_forenoon %d", info->tue_forenoon);
			printf(" --tue_afternoon %d", info->tue_afternoon);
			printf(" --wed_time %d", info->wed_time);
			printf(" --wed_forenoon %d", info->wed_forenoon);
			printf(" --wed_afternoon %d", info->wed_afternoon);
			printf(" --thu_time %d", info->thu_time);
			printf(" --thu_forenoon %d", info->thu_forenoon);
			printf(" --thu_afternoon %d", info->thu_afternoon);
			printf(" --fri_time %d", info->fri_time);
			printf(" --fri_forenoon %d", info->fri_forenoon);
			printf(" --fri_afternoon %d", info->fri_afternoon);
			printf(" --sat_time %d", info->sat_time);
			printf(" --sat_forenoon %d", info->sat_forenoon);
			printf(" --sat_afternoon %d", info->sat_afternoon);
		}
		/* add end*/
    }
    printf("\n");
}

/*!
 *\fn           static void pctl_init(struct xt_entry_match *match)
 *\brief        iptables init
 *\return       none
 */
static void pctl_init(struct xt_entry_match *match)
{
    struct _xt_pctl_info *info = (struct _xt_pctl_info *)match->data;
    memset(info, 0, sizeof(struct _xt_pctl_info));
}

/***************************************************************************/
/*                      PUBLIC_FUNCTIONS                     */
/***************************************************************************/
/*!
 *\fn           static void _init(void)
 *\brief        iptables register
 *\return       none
 */
static __attribute__((constructor)) void _pctl_mt_ldr(void)
{
    xtables_register_match(&pctl_match);
}

/***************************************************************************/
/*                      GLOBAL_FUNCTIONS                     */
/***************************************************************************/
