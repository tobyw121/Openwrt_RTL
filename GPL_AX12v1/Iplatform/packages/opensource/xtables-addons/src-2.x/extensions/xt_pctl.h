/*!Copyright(c) 2013-2014 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     xt_pctl.h
 *\brief    kernel/netfilter part for http host filter. 
 *
 *\author   Hu Luyao
 *\version  1.0.0
 *\date     23Dec13
 *
 *\history  \arg 1.0.0, creat this based on "multiurl" mod from soho.  
 *                  
 */
/***************************************************************************/
/*                      CONFIGURATIONS                   */
/***************************************************************************/
#ifndef _XT_PCTL_H_
#define _XT_PCTL_H_


/***************************************************************************/
/*                      INCLUDE_FILES                    */
/***************************************************************************/


/***************************************************************************/
/*                      DEFINES                      */
/***************************************************************************/
#define     PCTL_OWNER_NUM                    (16)//(4)
#define     PCTL_OWNER_ID_ALL                 0xffff
#define     PCTL_ID_DNS_RESP                  0xeeee //add by wanghao
#define     PCTL_WEB_URL_ID_ALL               0xfffe


#define     PCTL_URL_NUM                        (64)

#define     PCTL_URL_LEN                        (65) //make the actual max length to 64;so the total len is 65;
#define     PCTL_MAX_DNS_SIZE                   (256)

#define     TRUE                                (1)
#define     FALSE                               (0)

#ifdef PCTL_SUPPORT_IPV6
#define     IN6ADDRSZ (16)
#endif
/***************************************************************************/
/*                      TYPES                            */
/***************************************************************************/
typedef struct _xt_pctl_info
{
	bool advancedMode;//add by wanghao
    int id; /* owner id */

    bool blocked;    /* paused */

	/* add by wanghao */
	unsigned int workdays;
	unsigned int today_bonus_time;
	unsigned int today_reward_time;
	/* add end */

    bool workday_limit;
    unsigned int workday_time;

    bool workday_bedtime;
    unsigned int workday_begin;
    unsigned int workday_end;

    bool weekend_limit;
    unsigned int weekend_time;

    bool weekend_bedtime;
    unsigned int weekend_begin;
    unsigned int weekend_end;

	/* add by wanghao */
	unsigned int cat_map;// access to the URLs for these categories is not allowed
	int num_wl;
    char hosts_wl[PCTL_URL_NUM][PCTL_URL_LEN]; /* hosts in white list */
	/* add end */
	
    int num; 
    char hosts[PCTL_URL_NUM][PCTL_URL_LEN]; /* hosts */

	/* add by wanghao */
	//advanced settings
	unsigned int advanced_enable;
	unsigned int sun_time;
	unsigned int sun_forenoon;
	unsigned int sun_afternoon;
	
	unsigned int mon_time;
	unsigned int mon_forenoon;
	unsigned int mon_afternoon;

	unsigned int tue_time;
	unsigned int tue_forenoon;
	unsigned int tue_afternoon;

	unsigned int wed_time;
	unsigned int wed_forenoon;
	unsigned int wed_afternoon;

	unsigned int thu_time;
	unsigned int thu_forenoon;
	unsigned int thu_afternoon;

	unsigned int fri_time;
	unsigned int fri_forenoon;
	unsigned int fri_afternoon;

	unsigned int sat_time;
	unsigned int sat_forenoon;
	unsigned int sat_afternoon;
	/* add end */

	int hosts_type; /* hosts type, 0 is not support hosts_type, 1 is black list, 2 is white list */
	
} xt_pctl_info;



#if defined(SUPPORT_SHORTCUT_FE)

struct sfe_ipv6_addr {
	__be32 addr[4];
};

typedef union {
	__be32			ip;
	struct sfe_ipv6_addr	ip6[1];
} sfe_ip_addr_t;

struct sfe_connection_destroy {
	int protocol;
	sfe_ip_addr_t src_ip;
	sfe_ip_addr_t dest_ip;
	__be16 src_port;
	__be16 dest_port;
};
#endif
/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/


/***************************************************************************/
/*                      FUNCTIONS                        */
/***************************************************************************/
#endif
