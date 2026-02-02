/*!Copyright(c) 2013-2014 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     xt_pctl.c
 *\brief    kernel/netfilter part for parental control. 
 *
 *\author   Miao Wen
 *\version  1.0.0
 *\date     10Mar17
 *
 *\history  \arg 1.0.0, creat this based on "multiurl" mod from soho.  
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
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ip6_checksum.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/x_tables.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/sort.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/neighbour.h>
#include "compat_xtables.h"

#include <net/tcp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/arp.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 2, 0)
#include <linux/proc_fs.h>
#endif

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

#include "xt_pctl.h"
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
#include "compat_xtnu.h"
#endif

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
#include <linux/timer.h>
#include <linux/jiffies.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
extern struct hlist_nulls_head *nf_conntrack_hash __read_mostly;
extern unsigned int nf_conntrack_htable_size __read_mostly;
#endif


/* add by wanghao  */
#if defined(CONFIG_RA_HW_NAT) || defined(CONFIG_RA_HW_NAT_MODULE)
#include <../net/nat/hw_nat/foe_fdb.h>
extern void (*clearHnatCachePtr)(struct _ipv4_hnapt *ipv4_hnapt);
#endif

/*Hanjiayan add*/
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
#include <net/ppa_api.h>
#include <net/ppa_hook.h>
#endif

#if defined(SUPPORT_SHORTCUT_FE)
void (*clearSfeRulePtr)(struct sfe_connection_destroy *sid) = NULL;
EXPORT_SYMBOL(clearSfeRulePtr);
#endif
/* add end  */

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#define CONFIG_TP_FC_PCTL_SUPPORT	1
#endif
/***************************************************************************/
/*                      DEBUG                        */
/***************************************************************************/
/* xt_pctl */
#if 0
#define PCTL_DEBUG(fmt, args...)   \
	printk("[DEBUG]%s %d: "fmt"\n", __FUNCTION__, __LINE__, ##args)
#else
#define PCTL_DEBUG(fmt, args...)
#endif

#define PCTL_ERROR(fmt, args...)   \
	printk("[ERROR]%s %d: "fmt"\n", __FUNCTION__, __LINE__, ##args)

/* url_class and some others */
#define printErr(fmt, args...)	printk("[ERROR]\033[1m[ %s ] %03d: "fmt"\033[0m", __FUNCTION__, __LINE__, ##args)
#define printWar(fmt, args...)	//printk("[DEBUG]\033[4m[ %s ] %03d: "fmt"\033[0m", __FUNCTION__, __LINE__, ##args)
/***************************************************************************/
/*                      DEFINES                      */
/***************************************************************************/
#define PCTL_HTTP_REFERER  0
#define PCTL_REDIRECT  1
#define PCTL_DNS_REDIRECT  1
#define PCTL_DEVICE_INFO   1

#ifdef PCTL_BLOCK_URL
#define PCTL_BLOCK_URL_CATEGORY   999
#endif

#define HOST_STR     "\r\nHost: "
#define HOST_END_STR      "\r\n"
#define ACCEPT_STR      "\r\nAccept: "

#if PCTL_HTTP_REFERER
#define REFERER_STR  "\r\nReferer: "
#define REFERER_END_STR  "\r\n"
#define REFERER_END_STR2  "/"
#endif

#if PCTL_DEVICE_INFO
#define USER_AGENT_STR     "\r\nUser-Agent: "
#define USER_AGENT_END_STR      "\r\n"
#define DEVICE_INFO_NUM  256
#define DEVICE_INFO_USER_AGENT_LAN  255

typedef enum _DEVICE_TYPE{
    DEVICE_TYPE_NONE = -1,
    DEVICE_TYPE_OTHER = 0,
    DEVICE_TYPE_PC,
    DEVICE_TYPE_PHONE,
    DEVICE_TYPE_LABTOP,
    DEVICE_TYPE_TABLET,
    DEVICE_TYPE_ENTERTAINMENT,
    DEVICE_TYPE_PRINTER,
    DEVICE_TYPE_IOT,
}DEVICE_TYPE;

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
#define BLOG_CONNECTION_KEEPALIVE_INTERVAL 8 * HZ
#endif

typedef struct _device_info{
    unsigned char  mac[ETH_ALEN];
    DEVICE_TYPE    type;
    bool            noRedirect; //add by wanghao
#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
    unsigned long   keepaliveTime;
#endif
}device_info;
#endif

#define HTTP_STR  "http://"
#define HTTPS_STR  "https://"

#define GET_STR   "GET "
#define FILTER_HOST_IP    0x0a000002    /* 10.0.0.2 */
#ifdef PCTL_SUPPORT_IPV6
#define FILTER_HOST_IP_6  "1999::1"
#endif
#define IS_TCP_FLAG_SYN(tcpFlag)    ((tcpFlag) == 0x0002)
#define GET_TCP_FLAG(pTcpHdr)       (((ntohl(tcp_flag_word((pTcpHdr))) & 0x0fff0000)<<4)>>20)

#define DEBUG   0

#define H_NMACQUAD(x) \
	((unsigned char*)x)[0],\
	((unsigned char*)x)[1],\
	((unsigned char*)x)[2],\
	((unsigned char*)x)[3],\
	((unsigned char*)x)[4],\
	((unsigned char*)x)[5]

#define H_MACFMT 	"%02x:%02x:%02x:%02x:%02x:%02x"

#define DNS_PORT 53
#define HTTP_PORT 80
#define HTTPS_PORT 443

#define HANDSHAKE 22 /*ssl: content type.SSL_TYPE*/
#define CLIENT_HELLO 1 /*handshake: content type.HANDSHAKE_TYPE*/
#define SERVER_NAME 0 /*extension type in client hello(can only appear once in client hello).EXTENSION_TYPE*/
#define HOST_NAME 0 /*content type in SNI(in server_name extension).SERVER_NAME_TYPE*/

#define DNS_QUERY_TYPE_AAAA_VALUE 0x001c
#define DNS_QUERY_TYPE_A_VALUE    0x0001        

#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
#define PCTL_HANDLING_MARK (0x8)
#define PCTL_HANDLE_OK_MARK (0x4)
#endif

typedef struct _dns_header {
    unsigned short  transID;     /* packet ID */
    unsigned short  flags;       /* flag */
    unsigned short  nQDCount;    /* question section */
    unsigned short  nANCount;    /* answer section */
    unsigned short  nNSCount;    /* authority records section */
    unsigned short  nARCount;    /* additional records section */
}dns_header;

typedef struct __packed _dns_ans{
	unsigned short name;
	unsigned short _type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short len;
	unsigned int ipaddr;
}dns_ans;

#ifdef PCTL_SUPPORT_IPV6
typedef struct __packed _dns_ans_ipv6{
	unsigned short name;
	unsigned short _type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short len;
	struct in6_addr ipv6addr;
}dns_ans_ipv6;
#endif

#if PCTL_REDIRECT
#define ETH_HTTP_LEN 14
#define RES_HTML_MAX_LEN 1024
#define RES_CONTENT_MAX_LEN ((RES_HTML_MAX_LEN) * 2)
#endif

#define PCTL_URL_HASH_SIZE  64
#define PCTL_LOG_NUM            256

#define PCTL_HISTORY_DAY_NUM    6

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
#define PCTL_HISTORY_LOG_NUM    256
#else
#define PCTL_HISTORY_LOG_NUM    100
#endif

/* Adjust for local timezone */ 
extern struct timezone sys_tz; 
#define GET_TIMESTAMP()  ( get_seconds() - 60 * sys_tz.tz_minuteswest ) 

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)     

#define xtables_ack(__skb, __nlh, __err) \
       do\
       {\
               netlink_ack(__skb, __nlh, __err, NULL);\
       }while(0)
       
#else

#define xtables_ack(__skb, __nlh, __err) \
       do\
       {\
               netlink_ack(__skb, __nlh, __err);\

       }while(0)
#endif

/***************************************************************************/
/*                      TYPES                            */
/***************************************************************************/
union if_addr {
	unsigned int addr4;
	struct in6_addr addr6;
};

typedef enum _PCTL_STATUS{
    PCTL_STATUS_OK = 0,
    PCTL_STATUS_BLOCKED,        /* internet paused */
    PCTL_STATUS_TIME_LIMIT,     /* time limit */
    PCTL_STATUS_BEDTIME,        /* bedtime */
    PCTL_STATUS_FILTER,         /* url filter */
    /* add by wanghao */
    PCTL_STATUS_CONTENT,         /* url filter content */
    PCTL_STATUS_UNSAFE,         /* url filter unsafe */
	/* add end */
	PCTL_STATUS_INBONUS,
}PCTL_STATUS;

typedef struct _PROTOCOL_VERSION
{
	uint8_t majorVersion;
	uint8_t minorVersion;
}PROTOCOL_VERSION;

typedef struct _SSL_MSG{
	uint8_t type; /*len:1 byte*/
	PROTOCOL_VERSION version; /*len:2 bytes*/
	uint16_t length; /* The length (in bytes) of the following TLSPlaintext.fragment.*/
	uint8_t *pContent; /*  The application data,type is specified by the type field.*/
}SSL_MSG;

typedef uint32_t uint24_t;

typedef struct{
	uint16_t length;
	uint8_t *pData;
}CIPHER_SUITE,CH_EXTENSIONS;

typedef struct{
	uint8_t length;
	uint8_t *pData;
}SESSION_ID,COMPRESSION_METHOD;

typedef struct _TLS_EXTENSION{
	uint16_t type;
	uint16_t length;
	uint8_t *pData;
}TLS_EXTENSION;/*TLS(client hello) extension*/

typedef struct _HANDSHAKE_CLIENT_HELLO{
	uint8_t type; /*len:1 byte*/
	uint24_t length;
	PROTOCOL_VERSION clientVersion;
    uint8_t *random;/*the length is 32,but we don't need this field.So only give pointer to start position*/
    SESSION_ID sessionID;
    CIPHER_SUITE cipherSuites;
    COMPRESSION_METHOD compression_methods;
    uint8_t *pExtensions /*pointer to extensions length field*/;
}HANDSHAKE_CLIENT_HELLO;

typedef struct _pctl_stats{
    unsigned int   timestamp;    /* last visited */
    unsigned int  total;        /* minutes */
	unsigned int  totalBonus;   /* minutes bonus time*/
	unsigned int  countTimeStamp; /* time stamp for counting, add by wanghao */
	unsigned int  count;        /* count, add by wanghao */
}pctl_stats;

typedef struct _traffic_stat {
    u_int64_t old;          // traffic accumulated when ct is destroy
    u_int64_t curr;         // traffic of all active ct currently
    u_int64_t curr_prev;    // previous curr
    rwlock_t lock;
} traffic_stat;

typedef struct _pctl_stats_day{
	unsigned int	timestamp;    /* last visited */
	unsigned int	total;        /* minutes */
	unsigned int	totalBonus;   /* minutes bonus time*/
	unsigned int	hours[24];    /* per hour */
	int  			time_limit;   /* time_limit */
}pctl_stats_day;

typedef struct _pctl_log_entry{
    unsigned    host_len;           /* host len */
    char        host[PCTL_URL_LEN];     /* host */
    pctl_stats  stats;
	bool v6;
	union if_addr	ip; //add by wanghao
    traffic_stat traffic;
}pctl_log_entry;

typedef struct _pctl_log{
    struct list_head log_node;
    struct hlist_node hash_node;
    pctl_log_entry entry;
}pctl_log;

typedef struct _pctl_history{
    //pctl_stats  day_stats;
    pctl_stats_day day_stats;
    unsigned    int num; /* entry num */
    pctl_log_entry  log_entry[PCTL_HISTORY_LOG_NUM];
}pctl_history;

typedef struct _client_info {
    struct list_head list;
    unsigned char mac[ETH_ALEN];
} client_info;

typedef struct _pctl_owner{
    unsigned int id;

    /* today */
    //pctl_stats today_stats;
    pctl_stats_day today_stats;
    unsigned int log_len;
    struct list_head  log_list; /* point to pctl_log */
    struct hlist_head hash_list[PCTL_URL_HASH_SIZE];
    struct list_head client_list;
    traffic_stat traffic;    

	/* yesterday */
	//pctl_stats yesterday_stats;//add by wanghao
	pctl_stats_day yesterday_stats;

    /* history */
    int day_idx;    /* next day */
    pctl_history* day[PCTL_HISTORY_DAY_NUM]; 

    PCTL_STATUS status;
    struct proc_dir_entry* proc_file;
    rwlock_t lock;
#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
    bool status_block;
#endif
}pctl_owner;

struct xtm {
	u_int8_t month;    /* (1-12) */
	u_int8_t monthday; /* (1-31) */
	u_int8_t weekday;  /* (1-7) */
	u_int8_t hour;     /* (0-23) */
	u_int8_t minute;   /* (0-59) */
	u_int8_t second;   /* (0-59) */
    unsigned int seconds_day;
    unsigned int minutes_day;
	unsigned int dse;
};
/***************************************************************************/
/*                      EXTERN_PROTOTYPES                    */
/***************************************************************************/

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
extern int pctl_cb_register(pctl_cb_t skb_cb, pctl_cb_t fkb_cb);
extern void tp_fcache_flush_client(const unsigned char* client_mac);
#endif

/***************************************************************************/
/*                      LOCAL_PROTOTYPES                     */
/***************************************************************************/

/*!
 *\fn           unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, 
                                        const unsigned char* strCharSet)
 *\brief        find the url in str zone
 *\param[in]    start           start ptr of str zone.
 *\param[in]    end             end ptr of str zone.
 *\param[in]    strCharSet      the url you want to find
 *\return       url postion
 */
static unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, const unsigned char* strCharSet);

/*!
 *\fn           static bool match(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns)
 *\return       found or not
 */
static bool match(const struct sk_buff *skb, struct xt_action_param *par);

/*!
 *\fn           static int __init pctl_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init pctl_init(void);

/*!
 *\fn           static void __exit pctl_exit(void)
 *\brief        mod exit
 *\return       none
 */

static void __exit pctl_exit(void);
/*
 * fn		static bool extractHandshakeFromSSL(const uint8_t *pSSLBuff, uint8_t **ppHandshake)
 * brief	extract the handshake From SSL packet.
 * param[in]	pSSL - pointer to the start of SSL packet in skb_buff.
 * param[out]	ppHandshake - address of pointer to the start of handshake message wrapped with SSLv3/TLS.
 * return	BOOL
 * retval	true  succeed to extract handshake
 *		false fail to extract handshake
 */
static bool extractHandshakeFromSSL(const uint8_t *pSSL, uint8_t **ppHandshake);

/* 
 * fn		static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t *ppSNIExt) 
 * brief	extract SNI extension form extensions.
 * param[in]	pExtensions - pointer to start of extensionList.
 * param[out]	ppSNIExt      - address of pointer to SNI extension.
 * return	bool
 * retval	true - extract SNI extension successfully.
 *       	false - extract SNI extension unsuccessfully.	
 */
static bool extractSNIFromExtensions(const uint8_t *pExtensions,uint8_t **ppSNIExt);

/* 
 * fn		static  bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt) 
 * brief	extract SNI extension(Server_name)represents host_name from client_hello.
 * param[in]	pClientHello - pointer to start position of client_hello message.
 * param[out]	ppSNIExt - address of pointer to the start position of SNI extension in client_hello.
 * return	bool
 * retval	true -get the SNI represents host_name.
 *		false - doesn't get the right SNI.
 */
static bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt);

static int log_update_history(int id, unsigned int now);
static int log_clear(int id, bool initFlag);
static int log_clear_history(int id);//add by wanghao

static int http_response(struct sk_buff *skb, struct net_device *in, int reason);
static int traffic_update_current(void);
static inline u_int64_t traffic_get(traffic_stat *traffic);
static inline void traffic_reset(traffic_stat *traffic);
#ifdef PCTL_SUPPORT_IPV6
static int http_response_ipv6(struct sk_buff *skb, struct net_device *in, int reason);

/*!
 *\fn           static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns)
 *\return       found or not
 */
static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *par);
#endif

/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/
static struct xt_match pctl_match[] = {
    { 
        .name           = "pctl",
        .family         = NFPROTO_IPV4,
        .match          = match,
        .matchsize      = XT_ALIGN(sizeof(struct _xt_pctl_info)),
        .me             = THIS_MODULE,
    },
#ifdef PCTL_SUPPORT_IPV6
    { 
        .name           = "pctl",
        .family         = NFPROTO_IPV6,
        .match          = match_ipv6,
        .matchsize      = XT_ALIGN(sizeof(struct _xt_pctl_info)),
        .me             = THIS_MODULE,
    },
#endif
};

#if PCTL_REDIRECT
#if defined(MERCUSYS_TYPE_SUPPORT)
#define REDIRECT_DOMAIN "http://mwlogin.net/webpages/blocking.html?"
#elif defined(SUPPORT_HOMECARE_PRO_BLOCKING)
#define REDIRECT_DOMAIN "http://tplinkwifi.net/webpages/blocking_new.html?"
#else
#define REDIRECT_DOMAIN "http://tplinkwifi.net/webpages/blocking.html?"
#endif

static char *redirect_url[] = {
    REDIRECT_DOMAIN"pid=1",
    REDIRECT_DOMAIN"pid=2",
    REDIRECT_DOMAIN"pid=3",
    REDIRECT_DOMAIN"pid=4",
};

const char http_redirection_format[] = {
	"HTTP/1.1 200 OK\r\n"
	"Connection: close\r\n"
	"Content-type: text/html\r\n"
	"Content-length: %d\r\n"
	"\r\n"
	"%s"
};
const char http_redirection_html[] = {
	"<html><head></head><body>"
	"<script type=\"text/javascript\" language=\"javascript\">location.href=\"%s\";</script>"
	"</body></html>"
};

#define ACKTIMESTAMPKIND 8
#define ACKNOPKIND 1
#endif

static pctl_owner owners[PCTL_OWNER_NUM];

/* add by wanghao */
#define URL_LOG_COUNT_INTERVAL			5 * 60//5mins

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
static struct timer_list urlclass_timer;
static int urlclass_entry_used = 0;
#define URLCLASS_TIMER_INTERVAL (30 * HZ)

#define PCTL_MEMORY_DYNAMIC_KMALLOC	1

#define MAX_MSGSIZE 					1024
#define MAX_URL_CLASS_ENTRY_LEN			512//256
#define ENTRY_BUSY						256//512/2
#define ENTRY_TIMEOUT					60//60 seconds
#define MAX_URL_LEN						256
#define MAX_URL_ADDR_ENTRY_LEN			2048//1024
#define MAX_URL_SEND_TIME  				msecs_to_jiffies(2)
#define MAX_LEN_ALERTID					64

#define URL_CAT_MALWARE					0x1 << 12
#define URL_CAT_PHISHING				0x1 << 13
#define URL_CAT_SECURITY				(URL_CAT_MALWARE | URL_CAT_PHISHING)

static struct sock *url_class_nl __read_mostly;
static DEFINE_SPINLOCK(url_class_nl_lock);
#ifdef PCTL_BLOCK_URL 
static DEFINE_SPINLOCK(block_url_nl_lock);
#endif

enum url_class_req {
	URL_CLASS_REQ_VOID = 0,
	URL_CLASS_REQ_HELLO,
	URL_CLASS_REQ_CAT,
	URL_CLASS_REQ_LOG,
	URL_CLASS_REQ_LOG_HISTORY,
	URL_CLASS_REQ_BYE,
	URL_CLASS_REQ_END
};

enum url_send_status {
	URL_SEND_INIT = 0,
	URL_SEND_PENDING,	//waiting for sending to user
	URL_SEND_DONE,		//receive cat id from user, wait to be deleted
	URL_SEND_END
};

typedef enum _url_class_error {
	URL_CLASS_OK = 0,
	URL_CLASS_INTERNAL = 401,
	URL_CLASS_FRAGMENT = 402,
	URL_CLASS_DONE = 455, //not error, just make sure return can be sent via error_msg.
	URL_CLASS_ERROR_END
}url_class_error;

struct url_class_entry {
	int id;
	int info_id;
	unsigned char url_len;
	unsigned char url_addr_len;
	unsigned char send_flag;
	unsigned short dns_id;
	unsigned short query_type;
	unsigned int cat_id;
	unsigned int cat_map;
    bool v6;
    union if_addr dev_addr;
	unsigned int timestamp;
	struct list_head url_addr;
	struct list_head list;
	char url[MAX_URL_LEN];
};

struct url_class_carrier {
	int id;
	int info_id;
	unsigned char url_len;
	unsigned int cat_id;
	unsigned int cat_map;
	char url[MAX_URL_LEN];
};

struct url_log_carrier {
#ifdef USER_32BIT_COMPAT_KERNEL_64BIT
	unsigned long long time;
#else
	unsigned long time;
#endif
	unsigned int count;
	bool v6;
	union if_addr ip;
	char url[PCTL_URL_LEN];
    u_int64_t traffic;
};

struct block_rule_tuple {
	bool v6;
	union if_addr src_ip;
	union if_addr dst_ip;
	unsigned short src_port;
	unsigned short dst_port;
	unsigned char protocol;
};

struct block_rule_carrier {
	unsigned char reason_id;
	unsigned short duration;
	struct block_rule_tuple tuple;
	char url[MAX_URL_LEN];
	unsigned int cat_id;
	char alertid[MAX_LEN_ALERTID];
};

#ifdef PCTL_BLOCK_URL
struct block_rule_entry {
	unsigned char valid;
	unsigned char reason_id;
	unsigned char aging_type;
	unsigned char security_flag;
	unsigned long aging_time;
	unsigned long time_stamp;
	unsigned int cat_id;
	unsigned int stats;
	struct hlist_node hlist;
	struct block_rule_tuple tuple;
	char url[MAX_URL_LEN];
};
#endif

struct url_addr_entry {
	bool v6;
	union if_addr addr;
	struct list_head list;
};

static unsigned int g_pid = 0;//from process
#ifndef PCTL_MEMORY_DYNAMIC_KMALLOC
struct url_class_entry url_class_array[MAX_URL_CLASS_ENTRY_LEN] = {0};
#endif
static LIST_HEAD(url_class_buffer_list);
static LIST_HEAD(url_class_list);
struct task_struct *url_class_task = NULL;
#ifndef PCTL_MEMORY_DYNAMIC_KMALLOC
struct url_addr_entry url_addr_array[MAX_URL_ADDR_ENTRY_LEN] = {0};
#endif
static LIST_HEAD(url_addr_list);

static unsigned char *url_class_exception_list[] = {
	"in-addr.arpa",
	"tplink",
	"tp-link",
	"safethings.avira",
	"safethings.com",
	"kasasmart.com",
	NULL
};

// add for visit site records
static unsigned char *visit_exception_list[] = {
	"in-addr.arpa",
	"tplinkcloud.com",
	"safethings.avira",
	"safethings.com",
	"tplinknbu.com",
	"tplinkra.com",
	"tplinkra-ipc.com",
	"kasasmart.com",
	NULL
};

static int _url_class_list_init(void);
#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC	
static int _url_class_list_clean(void);
static int _url_class_list_destroy(void);
#endif
static int _url_addr_list_init(void);
#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC	
static int _url_addr_list_clean(void);
static int _url_addr_list_destroy(void);
#endif

#endif

#ifdef PCTL_BLOCK_URL
int block_url_add(struct block_rule_entry *rule, struct ethhdr *eth, struct sk_buff *skb); 
#endif

#ifdef SUPPORT_HOMECARE_PRO_BLOCKING
extern int block_rule_add(struct block_rule_carrier *rule, unsigned int cat, unsigned short duration, char *url, unsigned char len);
extern int (*block_http_redirect)(const struct sk_buff *skb, int reason);
#endif

//HTTP redirect
#define URL_VERCODE_PERIOD				1 * 60
static unsigned char *vercode_exception_list[] = {
	"captive.apple.com",
	"connectivitycheck.gstatic.com",
	NULL
};

struct time_limit_info {
	unsigned int time;
	unsigned int bonus_time;
    unsigned int reward_time;
	unsigned int forenoon;
	unsigned int afternoon;
};

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
static int urlclass_timer_handle(void);
int url_class_del(struct url_class_entry *entry);
#endif

/* add end */

/***************************************************************************/
/*                      PROC                         */
/***************************************************************************/
/* root proc dir */
#define PCTL_PROC_DIR    "pctl"
static struct proc_dir_entry *proc_dir = NULL;

/* pctl log */
static int log_proc_read(struct seq_file *s, void *unused);
static ssize_t log_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data);
static int log_proc_open(struct inode *inode, struct file *file);

static const struct file_operations log_proc_fops = {
    .open       = log_proc_open,
    .read       = seq_read,
    .write      = log_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};
/* pctl log */

/* device info */
#if PCTL_DEVICE_INFO
#define DEVICE_INFO_PROC_FILENAME	"devices"
device_info devices[DEVICE_INFO_NUM];
rwlock_t device_info_lock;

static int device_proc_read(struct seq_file *s, void *unused);
static int device_proc_open(struct inode *inode, struct file *file);
static ssize_t device_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data);

struct proc_dir_entry* device_proc_file = NULL;
static const struct file_operations device_proc_fops = {
    .open       = device_proc_open,
    .read       = seq_read,
    .write      = device_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#endif
/* device info */

/* vercode */
#define VERCODE_PROC_FILENAME		"vercode"

unsigned long vercode = 0;
unsigned char vercode_mac[ETH_ALEN] = {0};
unsigned long vercode_stamp = 0;
rwlock_t vercode_lock;

static int vercode_proc_open(struct inode *inode, struct file *file);
static ssize_t vercode_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data);

struct proc_dir_entry* vercode_proc_file = NULL;
static const struct file_operations vercode_proc_fops = {
    .open       = vercode_proc_open,
    .read       = seq_read,
    .write      = vercode_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};
/* vercode */

/* payment */
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
#define PAYMENT_PROC_FILENAME   "payment"

unsigned char payment = 0;
rwlock_t payment_lock;

static int payment_proc_open(struct inode *inode, struct file *file);
static ssize_t payment_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data);

struct proc_dir_entry* payment_proc_file = NULL;
static const struct file_operations payment_proc_fops = {
    .open       = payment_proc_open,
    .read       = seq_read,
    .write      = payment_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};
/* payment */

/* url_class_list */
#define URL_CLASS_LIST_FILENAME			"url_class_list"

static int urlclass_proc_open(struct inode *inode, struct file *file);
static ssize_t urlclass_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data);

struct proc_dir_entry* urlclass_proc_file = NULL;
static const struct file_operations urlclass_proc_fops = {
    .open       = urlclass_proc_open,
    .read       = seq_read,
    .write      = urlclass_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#endif
/* url_class_list */
/***************************************************************************/
/*                      LOCAL_FUNCTIONS                  */
/***************************************************************************/

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
static int urlclass_timer_handle(void)
{
	struct url_class_entry *pos = NULL;
	struct url_class_entry *n = NULL;
	unsigned int stamp = GET_TIMESTAMP();

	//u64 t1,t2,d0;
	//t1 = ktime_get_real_ns();

	spin_lock_bh(&url_class_nl_lock);

	if (urlclass_entry_used >= ENTRY_BUSY) {
		list_for_each_entry_safe(pos, n, &url_class_list, list) {
			if ( (stamp - pos->timestamp) >= ENTRY_TIMEOUT)
			{
				url_class_del(pos);
			}
		}
	}

	spin_unlock_bh(&url_class_nl_lock);

	mod_timer(&urlclass_timer, jiffies + URLCLASS_TIMER_INTERVAL);

	//t2 = ktime_get_real_ns();
	//d0 = t2 - t1;
	//printErr("duration: %llu.\n",d0);
    return 0;
}
#endif

#ifdef PCTL_SUPPORT_IPV6
static bool is_empty_addr6(struct in6_addr *a)
{
    return (a->s6_addr[0] ==0 && a->s6_addr[1] ==0 && a->s6_addr[2] ==0 && a->s6_addr[3] ==0);
}

static bool ipv6_addr_comp(struct in6_addr *a, struct in6_addr *b)
{
    int i;
    for (i = 0; i < sizeof(struct in6_addr); i++)
    {
        if (a->s6_addr[i] != b->s6_addr[i])
        return false;
    }
    return true;
}

static int inet_pton6(const char *src, unsigned char *dst)
{
    int IN6ADDRSZ = 16;
    static const char xdigits_l[] = "0123456789abcdef";
    static const char xdigits_u[] = "0123456789ABCDEF";
    unsigned char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
    const char *xdigits, *curtok;
    int ch, saw_xdigit;
    size_t val;

    memset((tp = tmp), 0, IN6ADDRSZ);
    endp = tp + IN6ADDRSZ;
    colonp = NULL;

    if(*src == ':')
    {
        if(*++src != ':')
        {
            return (0);
        }
    }
    curtok = src;
    saw_xdigit = 0;
    val = 0;
    while((ch = *src++) != '\0')
    {
        const char *pch;

        if((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
        {
            pch = strchr((xdigits = xdigits_u), ch);
        }
        if(pch != NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
            if(++saw_xdigit > 4)
                return (0);
            continue;
        }
        if(ch == ':')
        {
            curtok = src;
            if(!saw_xdigit)
            {
                if(colonp)
                {
                    return (0);
                }
                colonp = tp;
                continue;
            }
            if(tp + 2 > endp)
            {
                return (0);
            }
            *tp++ = (unsigned char) (val >> 8) & 0xff;
            *tp++ = (unsigned char) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
        return (0);
    }
    if(saw_xdigit) {
        if(tp + 2 > endp)
        {
            return (0);
        }
        *tp++ = (unsigned char) (val >> 8) & 0xff;
        *tp++ = (unsigned char) val & 0xff;
    }
    if(colonp != NULL)
    {
        const ssize_t n = tp - colonp;
        ssize_t i;

        if(tp == endp)
        {
            return (0);
        }
        for(i = 1; i <= n; i++) 
        {
            *(endp - i) = *(colonp + n - i);
            *(colonp + n - i) = 0;
        }
        tp = endp;
    }
    if(tp != endp)
    {
        return (0);
    }
    memcpy(dst, tmp, IN6ADDRSZ);
    return (1);
}
#endif

#if PCTL_REDIRECT

static unsigned char js_buf[RES_CONTENT_MAX_LEN + 1];
static unsigned char js_str[RES_HTML_MAX_LEN];

static void
handle_time_stamps(unsigned char *start,int len)
{
	unsigned int TLVp = 0;
	int i = 0;

	while(TLVp<len)
	{
		if(start[TLVp] == 0)  /*reach the EOL(End of Option List)*/
		{
			return ;  
		}
		
		if(start[TLVp] == ACKNOPKIND)
		{
			TLVp++;
			continue;
		}
		
		if(start[TLVp] == ACKTIMESTAMPKIND)
		{
			if(len - TLVp < 10)
				return;
			
			for(i = 0; i <10; i++)
			{			
				start[TLVp++] = ACKNOPKIND;
			}
			return;
		}

		if(start[TLVp+1])
		{
			TLVp += start[TLVp+1];
		}  
		else /*abnormal option length,just return,to avoid dead loop! */
		{
		  	return; 
		}			
	}
}

static int http_ack(struct sk_buff *skb, struct net_device *in)
{
    struct sk_buff *pNewSkb = NULL;
    struct sk_buff * pSkb = skb;

    unsigned short eth_len = 0;
    unsigned short ip_len = 0;
    unsigned short ip_payload_len = 0;
    unsigned short tcp_len = 0;
    unsigned short tcp_flag = 0;

    unsigned char tmp_mac[ETH_ALEN] = {0};
    unsigned int tmp_ip = 0;
    unsigned short tmp_port;

    struct ethhdr *pEthHdr;
    struct iphdr *pIpHdr;
    struct tcphdr *pTcpHdr;

    unsigned char *pTcpPayload;
    unsigned char *pOption = NULL;
    unsigned int  option_len = 0;

    eth_len = ETH_HTTP_LEN;
    PCTL_DEBUG("http_ack");
    pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
    if (NULL == pEthHdr)
    {
        PCTL_ERROR("---->>> Get ethhdr error!");
        return -1;
    }

    pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
    if (NULL == pIpHdr)
    {
        PCTL_ERROR("--->>> Get iphdr error!");
        return -1;
    }
    ip_len = (pIpHdr->ihl) << 2;
    ip_payload_len = ntohs(pIpHdr->tot_len);

    pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
    if (NULL == pTcpHdr)
    {
        PCTL_ERROR("--->>> Get tcphdr error!");
        return -1;
    }
    tcp_len = (ntohl(tcp_flag_word(pTcpHdr)) & 0xf0000000) >> 26;
    tcp_flag = GET_TCP_FLAG(pTcpHdr);
    pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb)){
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr *)skb_mac_header(pNewSkb);
        pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
        pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
        skb_pull(pSkb, eth_len);
    }else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
        {
            PCTL_ERROR("clone(simple copy) fail");
            return -1;
        }
    }

    if ((ip_payload_len == (ip_len + tcp_len) && IS_TCP_FLAG_SYN(tcp_flag)))
    {
        memcpy(tmp_mac, pEthHdr->h_dest, ETH_ALEN);
        memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
        memcpy(pEthHdr->h_source, tmp_mac, ETH_ALEN);

        /* ip header */
        tmp_ip        = pIpHdr->saddr;
        pIpHdr->saddr = pIpHdr->daddr;
        pIpHdr->daddr = tmp_ip;
        pIpHdr->check = 0;
        pIpHdr->check = ip_fast_csum(pIpHdr, pIpHdr->ihl);
        
        /* tcp header */
        tmp_port           = pTcpHdr->source;
        pTcpHdr->source   = pTcpHdr->dest;
        pTcpHdr->dest     = tmp_port;
        pTcpHdr->ack      = 1;
        pTcpHdr->ack_seq  = htonl(ntohl(pTcpHdr->seq) + 1);
        pTcpHdr->seq      = htonl(0x32bfa0f1); /* hard code the server seq num */
        pTcpHdr->check    = 0;
		
		pOption      = ((unsigned char*)pTcpHdr)+sizeof(struct tcphdr);
		option_len    = tcp_len - sizeof(struct tcphdr);
        handle_time_stamps(pOption,option_len);
		
        pTcpHdr->check    = tcp_v4_check(tcp_len, pIpHdr->saddr, pIpHdr->daddr, 
                                         csum_partial(pTcpHdr, tcp_len, 0));
		
        /* send the modified pkt */
        pNewSkb->dev = in;
        if (0 > dev_queue_xmit(pNewSkb))
        {
            PCTL_ERROR("send http ack pkt fail.");
            return -1;
        }
    }

    return 0;
}

#ifdef PCTL_SUPPORT_IPV6
static int http_ack_ipv6(struct sk_buff *skb, struct net_device *in)
{
    struct sk_buff *pNewSkb = NULL;
    struct sk_buff * pSkb = skb;

    unsigned short eth_len = 0;
    unsigned short ip_len = 0;
    unsigned short ip_payload_len = 0;
    unsigned short tcp_len = 0;
    unsigned short tcp_flag = 0;

    unsigned char tmp_mac[ETH_ALEN] = {0};
    unsigned int tmp_ip = 0;
    unsigned short tmp_port;

    struct in6_addr tmpIp;
    struct tcphdr tcph_buf;
    struct ethhdr *pEthHdr;
    struct ipv6hdr *pIpHdr;
    struct tcphdr *pTcpHdr;

    unsigned char *pOption = NULL;
    unsigned int  option_len = 0;
    unsigned int offset_tcp = 0;

    eth_len = ETH_HTTP_LEN;
    PCTL_DEBUG("http_ack_ipv6");
    pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
    if (NULL == pEthHdr)
    {
        PCTL_ERROR("---->>> Get ethhdr error!");
        return -1;
    }

    pIpHdr = (struct ipv6hdr *)((unsigned char *)pEthHdr + eth_len);
    if (NULL == pIpHdr)
    {
        PCTL_ERROR("--->>> Get iphdr error!");
        return -1;
    }

    if (ipv6_find_hdr(pSkb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)  
    {   
        PCTL_ERROR("--->>> Get offset error!");     
        return -1;  
    }    
    pTcpHdr = skb_header_pointer(pSkb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
    ip_len = offset_tcp;
    ip_payload_len = ntohs(pIpHdr->payload_len) + sizeof(struct ipv6hdr);

    if (NULL == pTcpHdr)
    {
        PCTL_ERROR("--->>> Get tcphdr error!");
        return -1;
    }
    tcp_len = (ntohl(tcp_flag_word(pTcpHdr)) & 0xf0000000) >> 26;
    tcp_flag = GET_TCP_FLAG(pTcpHdr);

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb)){
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr *)skb_mac_header(pNewSkb);
        pIpHdr = (struct ipv6hdr *)((unsigned char *)pEthHdr + eth_len);
        pTcpHdr = skb_header_pointer(pNewSkb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        skb_pull(pSkb, eth_len);
    }
    else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
        {
            PCTL_ERROR("clone(simple copy) fail");
            return -1;
        }
    }

    if (ip_payload_len == (ip_len + tcp_len) && IS_TCP_FLAG_SYN(tcp_flag))
    {
        memcpy(tmp_mac, pEthHdr->h_dest, ETH_ALEN);
        memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
        memcpy(pEthHdr->h_source, tmp_mac, ETH_ALEN);

        /* ip header */
        tmpIp         = pIpHdr->saddr;
        pIpHdr->saddr = pIpHdr->daddr;
        pIpHdr->daddr = tmpIp;
        
        /* tcp header */
        tmp_port          = pTcpHdr->source;
        pTcpHdr->source   = pTcpHdr->dest;
        pTcpHdr->dest     = tmp_port;
        pTcpHdr->ack      = 1;
        pTcpHdr->ack_seq  = htonl(ntohl(pTcpHdr->seq) + 1);
        pTcpHdr->seq      = htonl(0x32bfa0f1); /* hard code the server seq num */
        pTcpHdr->check    = 0;
		
		pOption      = ((unsigned char*)pTcpHdr)+sizeof(struct tcphdr);
		option_len    = tcp_len - sizeof(struct tcphdr);
        handle_time_stamps(pOption,option_len);
		
        pTcpHdr->check = tcp_v6_check(tcp_len, &pIpHdr->saddr, &pIpHdr->daddr, csum_partial(pTcpHdr, tcp_len, 0));
		
        /* send the modified pkt */
        pNewSkb->dev = in;
        if (0 > dev_queue_xmit(pNewSkb))
        {
            PCTL_ERROR("send http ack pkt fail.");
            return -1;
        }
    }

    return 0;
}
#endif

/* add by wanghao */
static bool reasonCheck(int reason, int *pid)
{
	bool ret = false;

	switch (reason) {
	case PCTL_STATUS_TIME_LIMIT:
		*pid = 1;
		ret = true;
		
		break;
	case PCTL_STATUS_FILTER:
	case PCTL_STATUS_CONTENT:
		*pid = 2;
		ret = true;
		
		break;
	case PCTL_STATUS_UNSAFE:
		*pid = 3;
		ret = true;
		
		break;
	case PCTL_STATUS_BEDTIME:
		*pid = 4;
		ret = true;
		
		break;
	case PCTL_STATUS_BLOCKED:
		*pid = 5;
		ret = true;
		
		break;



	default:
		*pid = 1;
		
		break;
	}
	
	return ret;
}

static int generateVercode(unsigned char *mac, bool override)
{
	unsigned long rnd = 0;

	write_lock_bh(&vercode_lock);
	if (!override && vercode && time_in_range_open(get_seconds(), vercode_stamp, vercode_stamp + URL_VERCODE_PERIOD)) {
		write_unlock_bh(&vercode_lock);
		return 1;
	}
	
	get_random_bytes(&rnd, sizeof(rnd));
	vercode = 100000 + rnd % 899999;
	vercode_stamp = get_seconds();
	memcpy(vercode_mac, mac, ETH_ALEN);
	
	write_unlock_bh(&vercode_lock);
        
	return 0;
}

static int vercode_proc_read(struct seq_file *s, void *unused)
{
    read_lock_bh(&vercode_lock);
	seq_printf(s, "%lu %02X-%02X-%02X-%02X-%02X-%02X %lu\n", vercode, 
		vercode_mac[0], vercode_mac[1], vercode_mac[2], vercode_mac[3], vercode_mac[4], vercode_mac[5], vercode_stamp);
    read_unlock_bh(&vercode_lock);
	
    return 0;
}

static int vercode_proc_open(struct inode *inode, struct file *file)  
{  
    return single_open(file, vercode_proc_read, NULL);
}

static ssize_t vercode_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data)
{
    char cmd = 0;
	char kbuf[2] = {0};
    if (count != 2) {
         printErr("count = %d.\n", count);
         return -1;
    }
	if(copy_from_user(kbuf, buf, 2))
	{
		return -EFAULT;
	}
    cmd = kbuf[0];

    write_lock_bh(&vercode_lock);
    if (cmd == 'f') {
        vercode = 0;
		vercode_stamp = 0;
		memset(vercode_mac, 0, ETH_ALEN);
    }
    else {
        printErr("bad cmd %c.\n", cmd);
    }
    write_unlock_bh(&vercode_lock);

    return count;
}

int vercode_http_redirect(const struct sk_buff *skb, int reason)
{
	int ret = -1;
	const struct iphdr *iph;
	const struct tcphdr *tcph;
	unsigned char* http_payload_start;
	unsigned char* http_payload_end;
#ifdef PCTL_SUPPORT_IPV6
	const struct ipv6hdr *iph6;
	unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
#endif

#ifdef PCTL_SUPPORT_IPV6	
	if (4 == ip_hdr(skb)->version)
	{
		iph = ip_hdr(skb);
		if (iph->protocol != IPPROTO_TCP) {
			return 1;
		}
		tcph = (void *)iph + iph->ihl*4;
		http_payload_start = (unsigned char *)tcph + tcph->doff*4;
 		http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
	}
	else if (6 == ipv6_hdr(skb)->version)
	{
		iph6 = ipv6_hdr(skb);
		if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
		{
			return 1;
		}
		tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
		http_payload_start = (unsigned char *)tcph + tcph->doff*4;
		http_payload_end = (unsigned char *)(iph6 + ntohs(iph6->payload_len) - 1);		
	}
	else
	{
		return ret;
	}
#else
	iph = ip_hdr(skb);
	if (iph->protocol != IPPROTO_TCP) {
		return 1;
	}
	tcph = (void *)iph + iph->ihl*4;
	http_payload_start = (unsigned char *)tcph + tcph->doff*4;
	http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
#endif

	//http validation
	if (ntohs(tcph->dest) != HTTP_PORT) {
		return 1;
	}

	if (!(http_payload_start < http_payload_end) || !_url_strstr(http_payload_start, http_payload_end, GET_STR)) {
		//not GET requst
		return 1;
	}

#ifdef PCTL_SUPPORT_IPV6
	if (4 == ip_hdr(skb)->version)
	{
		ret = http_response(skb, skb->dev, reason);
	}
	else if (6 == ipv6_hdr(skb)->version)
	{
		ret = http_response_ipv6(skb, skb->dev, reason);
	}
#else
	ret = http_response(skb, skb->dev, reason);
#endif

    return ret;
}
EXPORT_SYMBOL(vercode_http_redirect);

static int redirectSkip(unsigned char *mac)
{
	int index;
	
	for (index=0; index < DEVICE_INFO_NUM; index++) {
		if (!memcmp(devices[index].mac, mac, ETH_ALEN)) {
			if (devices[index].noRedirect) {
				return 1;
			}
			else {
				return 0;
			}
        }
    }

	return 0;
}

static int redirectUpdate(void)
{
	int index;
	unsigned char mac_null[ETH_ALEN] = {0};
	
	for (index=0; index < DEVICE_INFO_NUM; index++) {
		if (memcmp(devices[index].mac, mac_null, ETH_ALEN) != 0) {
			devices[index].noRedirect = false;
        }
    }

	return 0;
}

/* add end */

static int http_response(struct sk_buff *skb, struct net_device *in, int reason)
{
	struct sk_buff *pNewSkb = NULL;
	struct sk_buff * pSkb = skb;
	
	unsigned short eth_len = 0;
	unsigned short ip_len = 0;
	unsigned short ip_payload_len = 0;
	unsigned short tcp_len = 0;
	unsigned short tcp_payload_len = 0;
	unsigned short max_payload_len = 0;

	unsigned char tmp_mac[ETH_ALEN] = {0};
	unsigned int tmp_ip = 0;
	unsigned short tmp_port = 0;
	unsigned int tmp_seq = 0;
	unsigned int tmp_tcp_payload_len = 0;

	int js_real_len = 0;

	struct ethhdr * pEthHdr = NULL;
	struct iphdr * pIpHdr = NULL;
	struct tcphdr * pTcpHdr = NULL;

	unsigned char * pTcpPayload = NULL;

	/* add by wanghao */
	char urlBuffer[256] = {0};
	char domain[128] = {0};
	int domainLen = 0;
	unsigned char* http_payload_start = NULL;
    unsigned char* http_payload_end = NULL;
    unsigned char* host_start = NULL;
    unsigned char* host_end = NULL;
	int pid = 0;
	int index = 0;
	bool spec = false;
	/* add end */

	eth_len = ETH_HTTP_LEN;
	PCTL_DEBUG("http_response");
	pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
	if (NULL == pEthHdr)
	{
		PCTL_ERROR("---->>> Get ethhdr error!");
		return -1;
	}
	
	/* add by wanghao */
	/*if (redirectSkip(pEthHdr->h_source)) {
		PCTL_ERROR("---->>> Skip HTTP redirect!");
		return -1;
	}*/
	/* add end */
	
	pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
	if (NULL == pIpHdr)
	{
		PCTL_ERROR("--->>> Get iphdr error!");
		return -1;
	}
	ip_len = (pIpHdr->ihl) << 2;
	ip_payload_len = ntohs(pIpHdr->tot_len);

	pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
	if (NULL == pTcpHdr)
	{
		PCTL_ERROR("--->>> Get tcphdr error!");
		return -1;
	}
	tcp_len = (ntohl(tcp_flag_word(pTcpHdr)) & 0xf0000000) >> 26;
	pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);

	/* add by wanghao */
	http_payload_start = (unsigned char *)pTcpHdr + pTcpHdr->doff*4;
    http_payload_end = http_payload_start + (ntohs(pIpHdr->tot_len) - pIpHdr->ihl*4 - pTcpHdr->doff*4) - 1;

	PCTL_ERROR("[TPIPF] http_payload_end - http_payload_start=%d", http_payload_end - http_payload_start);

    if (http_payload_start < http_payload_end) {
        host_start = _url_strstr(http_payload_start, http_payload_end, HOST_STR);
        if (host_start) {
            host_start += 8;
            host_end = _url_strstr(host_start, http_payload_end, HOST_END_STR);
            if (!host_end) {
                host_start = NULL;
            }
        }
    }

	if (host_start) {
		domainLen = host_end - host_start;
		if (domainLen > sizeof(domain)) {
			domainLen = sizeof(domain);
		}
		strncpy(domain, host_start, domainLen);
		domain[domainLen] = '\0';
		
		PCTL_ERROR("[TPIPF] domain=%s", domain);
	}
	
	for (index = 0; vercode_exception_list[index] != NULL; index++) {
		if (_url_strstr(host_start, host_end, vercode_exception_list[index])) {
			spec = true;
			//printWar("exception list match, ignor...%s\n", vercode_exception_list[index]);
		}
	}
	
	if (reasonCheck(reason, &pid)) {
		if (spec) {
			if (_url_strstr(http_payload_start, http_payload_end, ACCEPT_STR)) {
				generateVercode(pEthHdr->h_source, true);
			}
		}
		else {
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS			
			if (generateVercode(pEthHdr->h_source, false)) {
				PCTL_DEBUG("vercode has been generated!\n");
				return -1;
			}
#else
			generateVercode(pEthHdr->h_source, true);
#endif
		}
	}
	/* add end */

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb)){
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr *)skb_mac_header(pNewSkb);
        pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
        pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
        pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);
        skb_pull(pSkb, eth_len);
    }else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
		{
			PCTL_ERROR("clone(simple copy) fail");
            return -1;
		}
    }

	/* tcp payload */
	tmp_tcp_payload_len = ip_payload_len - ip_len - tcp_len;	
	memset(js_str, 0, sizeof(js_str));
	memset(js_buf, 0, sizeof(js_buf));
	
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING
	/* add by wanghao */	
	if (reasonCheck(reason, &pid)) {
		read_lock_bh(&vercode_lock);
		if (pid == 4)
		{
			read_lock_bh(&payment_lock);
			if (payment == '1')
			{
				snprintf(urlBuffer, sizeof(urlBuffer) - 1, 
				"%spid=%d&token=%lu&mac=%02X-%02X-%02X-%02X-%02X-%02X&url=%s&state=1",
					REDIRECT_DOMAIN, pid, vercode, pEthHdr->h_source[0], pEthHdr->h_source[1], 
					pEthHdr->h_source[2], pEthHdr->h_source[3], pEthHdr->h_source[4], pEthHdr->h_source[5], domain);
			}
			else
			{
				snprintf(urlBuffer, sizeof(urlBuffer) - 1, 
				"%spid=%d&token=%lu&mac=%02X-%02X-%02X-%02X-%02X-%02X&url=%s&state=0",
					REDIRECT_DOMAIN, pid, vercode, pEthHdr->h_source[0], pEthHdr->h_source[1], 
					pEthHdr->h_source[2], pEthHdr->h_source[3], pEthHdr->h_source[4], pEthHdr->h_source[5], domain);
			}
			read_unlock_bh(&payment_lock);
		}
		else
		{
			snprintf(urlBuffer, sizeof(urlBuffer) - 1, "%spid=%d&token=%lu&mac=%02X-%02X-%02X-%02X-%02X-%02X&url=%s",
				REDIRECT_DOMAIN, pid, vercode, pEthHdr->h_source[0], pEthHdr->h_source[1], 
				pEthHdr->h_source[2], pEthHdr->h_source[3], pEthHdr->h_source[4], pEthHdr->h_source[5], domain);
		}
		read_unlock_bh(&vercode_lock);
		snprintf(js_str, sizeof(js_str) - 1, http_redirection_html, urlBuffer);
	}
	else {
		snprintf(js_str, sizeof(js_str) - 1, http_redirection_html, redirect_url[reason-1]);
	}
	/* add end */
#else
	snprintf(js_str, sizeof(js_str) - 1, http_redirection_html, redirect_url[reason-1]);
#endif

	js_real_len = strlen(js_str);
	snprintf(js_buf, sizeof(js_buf) - 1, http_redirection_format, js_real_len, js_str);
	
	tcp_payload_len = strlen(js_buf);
	max_payload_len = pNewSkb->end - pNewSkb->tail + tmp_tcp_payload_len;
	if ((tcp_payload_len + 1) > max_payload_len)
	{
		PCTL_ERROR("--->>> Construct tcp payload error!");
		dev_kfree_skb(pNewSkb);
		return -1;
	}
	pNewSkb->tail = pNewSkb->tail + ip_len + tcp_len + tcp_payload_len - ip_payload_len;
	pNewSkb->len = pNewSkb->len + ip_len + tcp_len + tcp_payload_len - ip_payload_len;
	memcpy(pTcpPayload, js_buf, tcp_payload_len + 1);
	
	/* eth header */
	memcpy(tmp_mac, pEthHdr->h_dest, ETH_ALEN);
	memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
	memcpy(pEthHdr->h_source, tmp_mac, ETH_ALEN);

	/* ip header */
	tmp_ip = pIpHdr->saddr;
	pIpHdr->saddr = pIpHdr->daddr;
	pIpHdr->daddr = tmp_ip;
	pIpHdr->tot_len = htons(ip_len + tcp_len + strlen(pTcpPayload));
	pIpHdr->check = 0;
	pIpHdr->check = ip_fast_csum(pIpHdr, pIpHdr->ihl);

	/* tcp header */
	tmp_port = pTcpHdr->source;
	tmp_seq = pTcpHdr->seq;
	pTcpHdr->source = pTcpHdr->dest;
	pTcpHdr->dest = tmp_port;
	pTcpHdr->urg = 0;
	pTcpHdr->ack = 1;
	pTcpHdr->psh = 0;
	pTcpHdr->rst = 0;
	pTcpHdr->syn = 0;
	pTcpHdr->fin = 1;
	pTcpHdr->seq = pTcpHdr->ack_seq;
	pTcpHdr->ack_seq = htonl(ntohl(tmp_seq) + tmp_tcp_payload_len);
	pTcpHdr->check = 0;
	pTcpHdr->check = tcp_v4_check(tcp_len + strlen(pTcpPayload), pIpHdr->saddr, 
			pIpHdr->daddr, 
			csum_partial(pTcpHdr, tcp_len + strlen(pTcpPayload), 0));

	pNewSkb->dev = in;

	if (dev_queue_xmit(pNewSkb) < 0)
	{
		PCTL_ERROR("--->>> Send Http Response error!");
		return -1;
	}

	return 0;
}

#ifdef PCTL_SUPPORT_IPV6
static int http_response_ipv6(struct sk_buff *skb, struct net_device *in, int reason)
{
	struct sk_buff *pNewSkb = NULL;
	struct sk_buff * pSkb = skb;
	
	unsigned short eth_len = 0;
	unsigned short tcp_len = 0;
	unsigned short ip_payload_len = 0;
	unsigned short tcp_payload_len = 0;
	unsigned short max_payload_len = 0;

	unsigned char tmp_mac[ETH_ALEN] = {0};
	unsigned short tmp_port = 0;
	unsigned int tmp_seq = 0;
	unsigned int tmp_tcp_payload_len = 0;
	unsigned int offset_tcp = 0;

	int js_real_len = 0;

	struct in6_addr tmpIp;
	struct tcphdr tcph_buf;
	struct ethhdr * pEthHdr = NULL;
	struct ipv6hdr * pIpHdr = NULL;
	struct tcphdr * pTcpHdr = NULL;

	unsigned char * pTcpPayload = NULL;
	char urlBuffer[256] = {0};
	char domain[128] = {0};
	int domainLen = 0;
	unsigned char* http_payload_start = NULL;
    unsigned char* http_payload_end = NULL;
    unsigned char* host_start = NULL;
    unsigned char* host_end = NULL;
	int pid = 0;
	int index = 0;
	bool spec = false;
	eth_len = ETH_HTTP_LEN;
	
	pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
	if (NULL == pEthHdr)
	{
		PCTL_ERROR("---->>> Get ethhdr error!");
		return -1;
	}
	
	pIpHdr = (struct ipv6hdr *)((unsigned char *)pEthHdr + eth_len);
	if (NULL == pIpHdr)
	{
		PCTL_ERROR("--->>> Get iphdr error!");
		return -1;
	}
	
	if (ipv6_find_hdr(pSkb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)	
	{	
		PCTL_ERROR("--->>> Get offset error!");		
		return -1;	
	}
	ip_payload_len = ntohs(pIpHdr->payload_len) + sizeof(struct ipv6hdr);
	pTcpHdr = skb_header_pointer(pSkb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
	if (NULL == pTcpHdr)
	{
		PCTL_ERROR("--->>> Get tcphdr error!");
		return -1;
	}
	tcp_len = pTcpHdr->doff << 2;
	pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);

	PCTL_DEBUG("offset_tcp=%d tcp_len=%d", offset_tcp, tcp_len);

	http_payload_start = (unsigned char *)pTcpHdr + pTcpHdr->doff*4;
    http_payload_end = (unsigned char *)(pIpHdr + ntohs(pIpHdr->payload_len) - 1);

	if (http_payload_start < http_payload_end) {
		host_start = _url_strstr(http_payload_start, http_payload_end, HOST_STR);
		if (host_start) {
			host_start += 8;
			host_end = _url_strstr(host_start, http_payload_end, HOST_END_STR);
			if (!host_end) {
				host_start = NULL;
			}
		}
	}

	if (host_start) {
		domainLen = host_end - host_start;
		if (domainLen > sizeof(domain)) {
			domainLen = sizeof(domain);
		}
		strncpy(domain, host_start, domainLen);
		domain[domainLen] = '\0';
	}

	for (index = 0; vercode_exception_list[index] != NULL; index++) {
		if (_url_strstr(host_start, host_end, vercode_exception_list[index])) {
			spec = true;
			//printWar("exception list match, ignor...%s\n", vercode_exception_list[index]);
		}
	}

	if (reasonCheck(reason, &pid)) {
		if (spec) {
			if (_url_strstr(http_payload_start, http_payload_end, ACCEPT_STR)) {
				generateVercode(pEthHdr->h_source, true);
			}
		}
		else {
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS			
			if (generateVercode(pEthHdr->h_source, false)) {
				PCTL_DEBUG("vercode has been generated!\n");
				return -1;
			}
#else
			generateVercode(pEthHdr->h_source, true);
#endif
		}
	}
	
    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb)){
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr *)skb_mac_header(pNewSkb);
        pIpHdr = (struct ipv6hdr *)((unsigned char *)pEthHdr + eth_len);
        pTcpHdr = skb_header_pointer(pNewSkb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);
        skb_pull(pSkb, eth_len);
    }else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
		{
			PCTL_ERROR("clone(simple copy) fail");
            return -1;
		}
    }

	/* tcp payload */
	tmp_tcp_payload_len = ip_payload_len - offset_tcp - tcp_len; //pNewSkb->tail - pTcpPayload + 1;
	
	memset(js_str, 0, sizeof(js_str));
	memset(js_buf, 0, sizeof(js_buf));	
	
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING	
	if (reasonCheck(reason, &pid)) {
		read_lock_bh(&vercode_lock);
		if (pid == 4)
		{
			read_lock_bh(&payment_lock);
			if (payment == '1')
			{
				snprintf(urlBuffer, sizeof(urlBuffer) - 1, 
				"%spid=%d&token=%lu&mac=%02X-%02X-%02X-%02X-%02X-%02X&url=%s&state=1",
					REDIRECT_DOMAIN, pid, vercode, pEthHdr->h_source[0], pEthHdr->h_source[1], 
					pEthHdr->h_source[2], pEthHdr->h_source[3], pEthHdr->h_source[4], pEthHdr->h_source[5], domain);
			}
			else
			{
				snprintf(urlBuffer, sizeof(urlBuffer) - 1, 
				"%spid=%d&token=%lu&mac=%02X-%02X-%02X-%02X-%02X-%02X&url=%s&state=0",
					REDIRECT_DOMAIN, pid, vercode, pEthHdr->h_source[0], pEthHdr->h_source[1], 
					pEthHdr->h_source[2], pEthHdr->h_source[3], pEthHdr->h_source[4], pEthHdr->h_source[5], domain);
			}
			read_unlock_bh(&payment_lock);
		}
		else
		{
			snprintf(urlBuffer, sizeof(urlBuffer) - 1, "%spid=%d&token=%lu&mac=%02X-%02X-%02X-%02X-%02X-%02X&url=%s",
				REDIRECT_DOMAIN, pid, vercode, pEthHdr->h_source[0], pEthHdr->h_source[1], 
				pEthHdr->h_source[2], pEthHdr->h_source[3], pEthHdr->h_source[4], pEthHdr->h_source[5], domain);
		}
		read_unlock_bh(&vercode_lock);
		snprintf(js_str, sizeof(js_str) - 1, http_redirection_html, urlBuffer);
	}
	else {
		snprintf(js_str, sizeof(js_str) - 1, http_redirection_html, redirect_url[reason-1]);
	}
#else
	snprintf(js_str, sizeof(js_str) - 1, http_redirection_html, redirect_url[reason-1]);
#endif
	
	js_real_len = strlen(js_str);
	snprintf(js_buf, sizeof(js_buf) - 1, http_redirection_format, js_real_len, js_str);
	
	tcp_payload_len = strlen(js_buf);
	max_payload_len = pNewSkb->end - pNewSkb->tail + tmp_tcp_payload_len;
	if ((tcp_payload_len + 1) > max_payload_len)
	{
		PCTL_ERROR("--->>> Construct tcp payload error!");
		dev_kfree_skb(pNewSkb);
		return -1;
	}
	pNewSkb->tail = pNewSkb->tail - tmp_tcp_payload_len + tcp_payload_len;
	pNewSkb->len = pNewSkb->len - tmp_tcp_payload_len + tcp_payload_len;
	memcpy(pTcpPayload, js_buf, tcp_payload_len + 1);
	
	/* eth header */
	memcpy(tmp_mac, pEthHdr->h_dest, ETH_ALEN);
	memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
	memcpy(pEthHdr->h_source, tmp_mac, ETH_ALEN);

	/* ip header */
	tmpIp = pIpHdr->saddr;
	pIpHdr->saddr = pIpHdr->daddr;
	pIpHdr->daddr = tmpIp;
	PCTL_DEBUG("payload_len_old=%d", ntohs(pIpHdr->payload_len));
	pIpHdr->payload_len = htons(ntohs(pIpHdr->payload_len) - tmp_tcp_payload_len + strlen(pTcpPayload));

	PCTL_DEBUG("payload_len=%d, tmp_tcp_payload_len=%d, strlen(pTcpPayload)=%d", ntohs(pIpHdr->payload_len), tmp_tcp_payload_len, strlen(pTcpPayload));
	
	/* tcp header */
	tmp_port = pTcpHdr->source;
	tmp_seq = pTcpHdr->seq;
	pTcpHdr->source = pTcpHdr->dest;
	pTcpHdr->dest = tmp_port;
	pTcpHdr->urg = 0;
	pTcpHdr->ack = 1;
	pTcpHdr->psh = 0;
	pTcpHdr->rst = 0;
	pTcpHdr->syn = 0;
	pTcpHdr->fin = 1;
	pTcpHdr->seq = pTcpHdr->ack_seq;
	pTcpHdr->ack_seq = htonl(ntohl(tmp_seq) + tmp_tcp_payload_len);
	pTcpHdr->check = 0;
	pTcpHdr->check = tcp_v6_check(tcp_len + strlen(pTcpPayload), &pIpHdr->saddr, &pIpHdr->daddr,
                                  csum_partial(pTcpHdr, tcp_len + strlen(pTcpPayload), 0));

	pNewSkb->dev = in;
	if (dev_queue_xmit(pNewSkb) < 0)
	{
		PCTL_ERROR("--->>> Send Http Response error!");
		return -1;
	}

	return 0;
}
#endif
#endif

#if PCTL_DNS_REDIRECT
static bool
acceptDNSReq(dns_header* dnshdr)
{
	if((0 != ntohs(dnshdr->nARCount)) || (0 != ntohs(dnshdr->nNSCount))
		|| (0 != ntohs(dnshdr->nANCount)) || (1 != ntohs(dnshdr->nQDCount))
		|| (0 != (ntohs(dnshdr->flags)&0xfcff))) //0 0000 0 x x 0 000 0000
	{
		return false;
	}
	return true;
}

static int getDNSReqLen(char* DNSReqData)
{
	int len;
	len = 0;
	while(*DNSReqData)
	{
		len += (unsigned char)(*DNSReqData) + 1;
		DNSReqData += (unsigned char)(*DNSReqData) + 1;
		if(len>512)
		{
			return -1;
		}
	}
	len ++;    //last 0x00
	len += 4;  //req type && req class;
	return len;
}

static int dns_response(struct sk_buff *skb, struct net_device *in, int reason, unsigned short query_type)
{
    struct sk_buff *pNewSkb = NULL;
	struct sk_buff * pSkb = skb;

	unsigned short eth_len;
	unsigned short ip_len;
	unsigned short ip_payload_len;

    struct ethhdr * pEthHdr;
	struct iphdr * pIpHdr;
	struct udphdr *pUdpHdr  = NULL;
	dns_header *pDNSHdr  = NULL;
	dns_ans *pDNSAns  = NULL;

	char             *pDNSReqData     = NULL;
	int              DNSReqLen        = 0;

    u8    tmpMac[ETH_ALEN] = {0};
    u32   tmpIp            = 0;
    u16   tmpPort          = 0;

    enum ip_conntrack_info ctinfo;
	const struct nf_conn *ct;

    eth_len = ETH_HTTP_LEN;
	PCTL_DEBUG("tp_dns_response");

    pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
    if (NULL == pEthHdr)
    {
        PCTL_ERROR("---->>> Get ethhdr error!");
        return -1;
    }

    pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
    if (NULL == pIpHdr)
    {
        PCTL_ERROR("--->>> Get iphdr error!");
        return -1;
    }
    ip_len = (pIpHdr->ihl) << 2;
    ip_payload_len = ntohs(pIpHdr->tot_len);

	/*handle DNS here*/
    pUdpHdr = (struct udphdr*)((s8*)pIpHdr + ip_len);
    if(NULL == pUdpHdr) 
    {
        PCTL_ERROR("--->>> Get udhdr error!");
        return -1;
    }

	pDNSHdr = (dns_header*)((s8*)pUdpHdr + sizeof(struct udphdr));
	if(!acceptDNSReq(pDNSHdr))
	{
        PCTL_ERROR("not acceptDNSReq.");
        return -1;
	}

	pDNSReqData = (char*)((s8*)pDNSHdr + sizeof(dns_header));
	DNSReqLen = getDNSReqLen(pDNSReqData);
	if(DNSReqLen < 0)
	{
		PCTL_ERROR("analyse pkt error.");
        return -1;
	}

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb))
    {
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr*)skb_mac_header(pNewSkb);
        pIpHdr  = (struct iphdr*)((s8*)pEthHdr + eth_len);
        pUdpHdr = (struct udphdr*)((s8*)pIpHdr + ip_len);
        pDNSHdr = (dns_header*)((s8*)pUdpHdr + sizeof(struct udphdr));
        pDNSReqData = (char*)((s8*)pDNSHdr + sizeof(dns_header));
        skb_pull(pSkb,eth_len);
    }else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
		{
			PCTL_ERROR("clone(simple copy) fail");
            return -1;
		}
    }

    memcpy(tmpMac, pEthHdr->h_dest, ETH_ALEN);
    memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
    memcpy(pEthHdr->h_source, tmpMac, ETH_ALEN);

    /* ip header */
    tmpIp         = pIpHdr->saddr;
    ct = nf_ct_get(skb, &ctinfo);
	if (ct) {
        pIpHdr->saddr = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
    }else
    {
        pIpHdr->saddr = pIpHdr->daddr;
    }
    pIpHdr->daddr = tmpIp;
    pIpHdr->check = 0;
    pIpHdr->tot_len = htons(ntohs(pIpHdr->tot_len)+sizeof(dns_ans));
    pIpHdr->check = ip_fast_csum(pIpHdr, pIpHdr->ihl);
	
    /* udp header */
    tmpPort           = pUdpHdr->source;
    pUdpHdr->source   = pUdpHdr->dest;
    pUdpHdr->dest     = tmpPort;
    pUdpHdr->check    = 0;

	pDNSHdr->nANCount = htons(0x0001);
	pDNSHdr->flags    = htons(0x8580);
	if(pNewSkb->end - pNewSkb->tail < sizeof(dns_ans))
	{
		u16 udpOffset = (unsigned char *)pUdpHdr - pNewSkb->data;
		u16 dnsReqOffset = (unsigned char *)pDNSReqData - pNewSkb->data;
        /*Hanjiayan modified, expand tailroom*/
          	PCTL_ERROR("skb is not big enough, try to skb_pad");
		if(skb_pad(pNewSkb, sizeof(dns_ans))){
			PCTL_ERROR("skb is not big enough.");
			return -1;
		}	
#if 0
	        PCTL_ERROR("skb is not big enough.");
			dev_kfree_skb(pNewSkb);
	        return -1;
#endif
		pUdpHdr = (struct udphdr *)(pNewSkb->data + udpOffset);
		pDNSReqData = pNewSkb->data + dnsReqOffset;
		pNewSkb->len += sizeof(dns_ans);
		skb_set_tail_pointer(pNewSkb, pNewSkb->len);
	}
	else
		skb_put(pNewSkb, sizeof(dns_ans));

	pDNSAns           = (dns_ans*)((s8*)pDNSReqData + DNSReqLen);
	pDNSAns->name     = htons(0xc000 + sizeof(dns_header));
    if (query_type == DNS_QUERY_TYPE_AAAA_VALUE)
    {
        pDNSAns->_type  = htons(DNS_QUERY_TYPE_AAAA_VALUE);
    } 
    else 
    {
        pDNSAns->_type  = htons(DNS_QUERY_TYPE_A_VALUE);
    }    
	pDNSAns->_class   = htons(0x0001);
	pDNSAns->ttl      = 0;
	pDNSAns->len      = htons(0x0004);
	pDNSAns->ipaddr   = htonl(FILTER_HOST_IP);
	
    pUdpHdr->len      = htons(ntohs(pUdpHdr->len)+sizeof(dns_ans));

	/* send the modified pkt */
    pNewSkb->dev = in;
    if (dev_queue_xmit(pNewSkb) < 0)
    {
        PCTL_ERROR("send dns response pkt fail.");
        return -1;
    }

    return 0;
}

#ifdef PCTL_SUPPORT_IPV6
static int dns_response_ipv6(struct sk_buff *skb, struct net_device *in, int reason, unsigned short query_type)
{

    struct sk_buff *pNewSkb = NULL;
	struct sk_buff * pSkb = skb;

	unsigned short eth_len;
	unsigned int offset_udp = 0;

    struct ethhdr * pEthHdr;
	struct ipv6hdr * pIpHdr;
	struct udphdr udph_buf;
	struct in6_addr tmpIp;
	struct in6_addr filter_host_ip6;
	struct udphdr *pUdpHdr = NULL;
	dns_header *pDNSHdr = NULL;
	dns_ans *pDNSAns = NULL;
	dns_ans_ipv6 *pDNSAns6 = NULL;

	char *pDNSReqData = NULL;
	int DNSReqLen = 0;
	int DNSAnsLen = 0;

    u8    tmpMac[ETH_ALEN] = {0};
    u16   tmpPort          = 0;

    enum ip_conntrack_info ctinfo;
	const struct nf_conn *ct;

	eth_len = ETH_HTTP_LEN;
	//PCTL_DEBUG("tp_dns_response");

    pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
    if (NULL == pEthHdr)
    {
        PCTL_ERROR("---->>> Get ethhdr error!");
        return -1;
    }

    pIpHdr = (struct ipv6hdr *)((unsigned char *)pEthHdr + eth_len);
    if (NULL == pIpHdr)
    {
        PCTL_ERROR("--->>> Get ipv6hdr error!");
        return -1;
    }

	/*handle DNS here*/
	
	if (ipv6_find_hdr(pSkb, &offset_udp, IPPROTO_UDP, NULL, NULL) < 0)
	{
		PCTL_ERROR("--->>> Get offset error!");
		return -1;
	}
    pUdpHdr = skb_header_pointer(pSkb, offset_udp, sizeof(udph_buf), &udph_buf);
	//PCTL_DEBUG("offset_udp=%d,skb->len=%d,payload_len=%d", offset_udp, skb->len, ntohs(pIpHdr->payload_len));
	
    if(NULL == pUdpHdr) 
    {
        PCTL_ERROR("--->>> Get udhdr error!");
        return -1;
    }

	pDNSHdr = (dns_header*)((s8*)pUdpHdr + sizeof(struct udphdr));
	if(!acceptDNSReq(pDNSHdr))
	{
        PCTL_ERROR("not acceptDNSReq.");
        return -1;
	}

	pDNSReqData = (char*)((s8*)pDNSHdr + sizeof(dns_header));
	DNSReqLen = getDNSReqLen(pDNSReqData);
	if(DNSReqLen < 0)
	{
		PCTL_ERROR("analyse pkt error.");
        return -1;
	}

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb))
    {
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr*)skb_mac_header(pNewSkb);
        pIpHdr  = (struct ipv6hdr*)((s8*)pEthHdr + eth_len);
		PCTL_DEBUG("offset_udp=%d", offset_udp);
        pUdpHdr = skb_header_pointer(pNewSkb, offset_udp, sizeof(udph_buf), &udph_buf);
        pDNSHdr = (dns_header*)((s8*)pUdpHdr + sizeof(struct udphdr));
        pDNSReqData = (char*)((s8*)pDNSHdr + sizeof(dns_header));
        skb_pull(pSkb,eth_len);
    }else
    {
    	//PCTL_DEBUG("pkt not cloned!!!!!!");
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
		{
			PCTL_ERROR("clone(simple copy) fail");
            return -1;
		}
    }

    memcpy(tmpMac, pEthHdr->h_dest, ETH_ALEN);
    memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
    memcpy(pEthHdr->h_source, tmpMac, ETH_ALEN);

    /* ipv6 header */
    tmpIp         = pIpHdr->saddr;
    //ct = nf_ct_get(skb, &ctinfo);
	//if (ct) {
    //    pIpHdr->saddr = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
    //}else
    //{
        pIpHdr->saddr = pIpHdr->daddr;
    //}
    pIpHdr->daddr = tmpIp;
	DNSAnsLen = query_type==DNS_QUERY_TYPE_A_VALUE?sizeof(dns_ans):sizeof(dns_ans_ipv6);
    pIpHdr->payload_len = htons(ntohs(pIpHdr->payload_len) + DNSAnsLen);
	
    /* udp header */
    tmpPort           = pUdpHdr->source;
    pUdpHdr->source   = pUdpHdr->dest;
    pUdpHdr->dest     = tmpPort;

	pDNSHdr->nANCount = htons(0x0001);
	pDNSHdr->flags    = htons(0x8580);
	if(pNewSkb->end - pNewSkb->tail < DNSAnsLen)
	{
		u16 udpOffset = (unsigned char *)pUdpHdr - pNewSkb->data;
		u16 dnsReqOffset = (unsigned char *)pDNSReqData - pNewSkb->data;
        /*Hanjiayan modified, expand tailroom*/
          	PCTL_ERROR("skb is not big enough, try to skb_pad");
		if(skb_pad(pNewSkb, DNSAnsLen)){
			PCTL_ERROR("skb is not big enough.");
			return -1;
		}	
#if 0
	        PCTL_ERROR("skb is not big enough.");
			dev_kfree_skb(pNewSkb);
	        return -1;
#endif
		pUdpHdr = (struct udphdr *)(pNewSkb->data + udpOffset);
		pDNSReqData = pNewSkb->data + dnsReqOffset;
		pNewSkb->len += DNSAnsLen;
		skb_set_tail_pointer(pNewSkb, pNewSkb->len);
	}
	else
	{
		skb_put(pNewSkb, DNSAnsLen);
	}
	
	if (query_type == DNS_QUERY_TYPE_A_VALUE)
	{
		pDNSAns          = (dns_ans*)((s8*)pDNSReqData + DNSReqLen);
		pDNSAns->name    = htons(0xc000 + sizeof(dns_header));
		pDNSAns->_type   = htons(DNS_QUERY_TYPE_A_VALUE);
		pDNSAns->_class  = htons(0x0001);
		pDNSAns->ttl     = 0;
		pDNSAns->len     = htons(0x0004);
		pDNSAns->ipaddr  = htonl(FILTER_HOST_IP);
	} 
	else 
	{
		inet_pton6((unsigned char *)&FILTER_HOST_IP_6, (unsigned char *)&filter_host_ip6);
		pDNSAns6         = (dns_ans_ipv6*)((s8*)pDNSReqData + DNSReqLen);
		pDNSAns6->name   = htons(0xc000 + sizeof(dns_header));
		pDNSAns6->_type  = htons(DNS_QUERY_TYPE_AAAA_VALUE);
		pDNSAns6->_class = htons(0x0001);
		pDNSAns6->ttl    = 0;
		pDNSAns6->len    = htons(0x0010);
		pDNSAns6->ipv6addr = filter_host_ip6;
	}    
	
    pUdpHdr->len      = htons(ntohs(pUdpHdr->len) + DNSAnsLen);
	pUdpHdr->check    = 0;

	//PCTL_DEBUG("pUdpHdr->len=%d", ntohs(pUdpHdr->len));
    pUdpHdr->check    = udp_v6_check(ntohs(pUdpHdr->len), &pIpHdr->saddr, &pIpHdr->daddr, 
		                             csum_partial(pUdpHdr, ntohs(pUdpHdr->len), 0));

	/* send the modified pkt */
    pNewSkb->dev = in;
	//PCTL_DEBUG("send dns ipv6 response packet");
    if (dev_queue_xmit(pNewSkb) < 0)
    {
        PCTL_ERROR("send dns response pkt fail.");
        return -1;
    }

    return 0;
}
#endif
#endif

static inline void localtime_1(struct xtm *r, unsigned int time)
{
	/* Each day has 86400s, so finding the hour/minute is actually easy. */
	r->seconds_day = time % 86400;
	r->second = r->seconds_day % 60;
	r->minutes_day = r->seconds_day / 60;
	r->minute = r->minutes_day % 60;
	r->hour   = r->minutes_day / 60;
}

static inline void localtime_2(struct xtm *r, unsigned int time)
{
	/*
	 * Here comes the rest (weekday, monthday). First, divide the SSTE
	 * by seconds-per-day to get the number of _days_ since the epoch.
	 */
	r->dse = time / 86400;

	/*
	 * 1970-01-01 (w=0) was a Thursday (4).
	 * -1 and +1 map Sunday properly onto 7.
	 */
	r->weekday = (4 + r->dse - 1) % 7 + 1;
}

static inline void init_stats(pctl_stats* pStats)
{
    pStats->total = 0;
    pStats->totalBonus = 0;
    pStats->timestamp = 0;
	//add by wanghao
	pStats->count = 1;
	pStats->countTimeStamp = 0;
	//add end
}

static inline void reset_stats(pctl_stats* pStats)
{
    pStats->total = 0;
    pStats->count = 0;
}

static inline void init_stats_daytime(pctl_stats_day* pStats)
{
    int i = 0;
    pStats->total = 0;
    pStats->totalBonus = 0;
    pStats->timestamp = 0;
    pStats->time_limit = -1;
    for(i = 0; i < 24; i++)
    {
        pStats->hours[i] = 0;
    }
}

static inline void update_stats(pctl_stats* pStats, unsigned int stamp, int isBonus)
{
    int min_after = stamp / 60 - pStats->timestamp / 60;
    if(min_after > 0) 
    {
        pStats->timestamp = stamp;
        if (isBonus)
        {
            pStats->totalBonus++;
        }
        pStats->total++;
    }else if(min_after < 0) 
    {
        PCTL_DEBUG("stamp < pStats->timestamp, reset timestamp.");
        pStats->timestamp = stamp;
    }

	//add by wanghao
	if (pStats->countTimeStamp && stamp - pStats->countTimeStamp >= URL_LOG_COUNT_INTERVAL) {
		pStats->count++;
	}
	pStats->countTimeStamp = stamp;
	//add end
}

static inline void update_stats_daytime(pctl_stats_day* pStats, unsigned int stamp, int isBonus)
{
    struct xtm current_time;
    int min_after = stamp / 60 - pStats->timestamp / 60;
    
    localtime_1(&current_time, stamp);
    localtime_2(&current_time, stamp);

    if(min_after > 0) 
    {
        pStats->timestamp = stamp;
        pStats->total++;
        if (isBonus)
        {
            pStats->totalBonus++;
        }
        pStats->hours[current_time.hour]++;
    }else if(min_after < 0) 
    {
        PCTL_DEBUG("stamp < pStats->timestamp, reset timestamp.");
        pStats->timestamp = stamp;
    }
}

static inline void update_stats_time_limit(pctl_stats_day* pStats, unsigned int stamp, const struct _xt_pctl_info *info)
{
    struct xtm current_time;
    int min_after = stamp / 60 - pStats->timestamp / 60;
    
    localtime_1(&current_time, stamp);
    localtime_2(&current_time, stamp);

    if(min_after > 0) 
    {
		/* Record the actual time_limit of the day */
		/* workday */
	    if(current_time.weekday >=1 && current_time.weekday <=5) 
	    {
	        if(info->workday_limit)
	        {
	            pStats->time_limit = info->workday_time;
	        }
			else
			{
				pStats->time_limit = -1;
			}
	    }
		/* weekend */
		else
		{
			if(info->weekend_limit)
	        {
	            pStats->time_limit = info->weekend_time;
	        }
			else
			{
				pStats->time_limit = -1;
			}
		}
    }
}

//add by wanghao
static inline int strtoint(char *string)
{
    int res = 0;
    while (*string>='0' && *string <='9')
    {
	res *= 10;
	res += *string-'0';
	string++;
    }

    return res;
}
//add end

static int log_proc_read(struct seq_file *s, void *unused)
{
    pctl_owner *pOwner = NULL;
    pctl_log *pLog = NULL;
    pctl_history *pDay = NULL;
    client_info *pos = NULL;

    char* filename = s->private;
    int id = 0;
    int i = 0, j = 0;

    if(!filename) 
    {
        PCTL_ERROR("filename is NULL.");
        return -1;
    }

    id = simple_strtol(filename, NULL, 10);
    if(id < 0 || id >= PCTL_OWNER_NUM) 
    {
        PCTL_ERROR("id is out of range. id = %d.", id);
        return -1;
    }

    pOwner = owners + id;
    log_update_history(id, GET_TIMESTAMP());
	read_lock_bh(&pOwner->lock);
	
    /* print today */
    seq_printf(s, "%u %u %u %u %d\n",pOwner->today_stats.total, pOwner->today_stats.timestamp, pOwner->log_len, pOwner->yesterday_stats.total, pOwner->today_stats.time_limit);
    
    if (pOwner->today_stats.total != 0 || pOwner->today_stats.timestamp != 0 || pOwner->log_len != 0)
    {
        for(i = 0; i < 24; i++)
        {
            seq_printf(s, "%u ",pOwner->today_stats.hours[i]);
        }
        seq_printf(s, "\n");
    }

	j=0;
    list_for_each_entry(pLog, &pOwner->log_list, log_node)
    {
        seq_printf(s, "%d %s %u %u %u\n",
                   j,
                   pLog->entry.host,
                   pLog->entry.stats.total,
                   pLog->entry.stats.count,//add by wanghao
                   //pLog->entry.stats.timestamp - 60 * sys_tz.tz_minuteswest);
                   //fix bug244329,dut only need to report UTC timestamp. web UI will transfer it to the right timezone.
                   pLog->entry.stats.timestamp + 60 * sys_tz.tz_minuteswest);

        j++;
    }

    /* print history */
    for (i=1; i<= PCTL_HISTORY_DAY_NUM; i++ )
    {
        int idx = ((pOwner->day_idx + PCTL_HISTORY_DAY_NUM - i) % PCTL_HISTORY_DAY_NUM);
        pDay = pOwner->day[idx];

        if(pDay) 
        {
            seq_printf(s, "%u %u %u %d\n",
                       pDay->day_stats.total, pDay->day_stats.timestamp, pDay->num, pDay->day_stats.time_limit);

            for(j = 0; j < 24; j++)
            {
                seq_printf(s, "%u ",pDay->day_stats.hours[j]);
            }
            seq_printf(s, "\n");

            for(j=0; j<pDay->num; j++) 
            {
                seq_printf(s, "%d %s %u %u %u\n",
                           j,
                           pDay->log_entry[j].host,
                           pDay->log_entry[j].stats.total,
                           pDay->log_entry[j].stats.count,
                           //pDay->log_entry[j].stats.timestamp - 60 * sys_tz.tz_minuteswest);
                           //fix bug244329,dut only need to report UTC timestamp. web UI will transfer it to the right timezone.
                           pDay->log_entry[j].stats.timestamp + 60 * sys_tz.tz_minuteswest);
            }
        }else
        {
            seq_printf(s, "%u %u %u\n",0, 0, 0);
        }
    }

    list_for_each_entry(pos, &pOwner->client_list, list) {
    	PCTL_DEBUG("%pM\n", pos->mac);
    }

    read_unlock_bh(&pOwner->lock);

    return 0;
}

static int log_proc_open(struct inode *inode, struct file *file)  
{  
    return single_open(file, log_proc_read, file->f_path.dentry->d_iname);
}

static void client_list_clear(pctl_owner *pOwner)
{
    client_info *pos = NULL, *n = NULL;

    write_lock_bh(&pOwner->lock);		
    list_for_each_entry_safe(pos, n, &pOwner->client_list, list) {
        list_del(&pos->list);
        kfree(pos);
    }
    write_unlock_bh(&pOwner->lock);
}

static ssize_t log_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data)
{
    int id  = 0;
    char cmd = 0;
    unsigned char kbuf[1024] = {0};

	if (count > 1024)
	{
		PCTL_ERROR("input count %d too large. id = %d.", count);
		return -1;
	}
	if (copy_from_user(kbuf, buf, count) != 0)
	{
		return -EFAULT;
	}
	
    char* filename = file->f_path.dentry->d_iname;
    pctl_owner *pOwner = NULL;

    id = simple_strtol(filename, NULL, 10);
    if(id < 0 || id >= PCTL_OWNER_NUM) 
    {
        PCTL_ERROR("id is out of range. id = %d.", id);
        return -1;
    }

	//remove by wanghao
    /*if(count != 2) 
    {
         PCTL_ERROR("count = %d.", count);
         return -1;
    }*/


    cmd = kbuf[0];
    pOwner = owners + id;

    if('f' == cmd) 
    {
        /* clear log */
        log_clear(id, true);

    }
	//add by wanghao
	else if (cmd == 't') {
		printWar("buf=%d\n", strtoint(kbuf + 2));
		/* changed by CCy, will conflict with log_clear()'s  write_lock_bh(&pOwner->lock), so move it here.*/
		write_lock_bh(&pOwner->lock);		
		pOwner->today_stats.total += strtoint(kbuf + 2);
		write_unlock_bh(&pOwner->lock);	
	}
	else if (cmd == 'c') {
		log_clear(id, false);
	}
	else if (cmd == 'h') {
		log_clear_history(id);
	}
    else if (cmd == 'm') {
        char *mac_list = kbuf + 2;
        char *token = NULL;
        char mac[ETH_ALEN] = { 0 };
        client_info *pos = NULL;

        PCTL_DEBUG("mac_list=%s\n", mac_list);
        client_list_clear(pOwner);

		write_lock_bh(&pOwner->lock);
        while ((token = strsep(&mac_list, " ")) != NULL) {
            PCTL_DEBUG("token=%s\n", token);

            if (sscanf(token, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                    &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == ETH_ALEN) {
                client_info *c = kmalloc(sizeof(client_info), GFP_KERNEL);

                if (!c) {
                    printErr("kmalloc\n");
                    break;
                }
                memcpy(c->mac, mac, ETH_ALEN);
                list_add(&c->list, &pOwner->client_list);
            } else {
                printErr("%s: invalid mac address\n", token);
            }
        }
		write_unlock_bh(&pOwner->lock);

        list_for_each_entry(pos, &pOwner->client_list, list) {
            PCTL_DEBUG("ownder_id=%d, client_mac=%pM\n", pOwner->id, pos->mac);
        }
    }	    
	//add end
    else
    {
        PCTL_ERROR("bad cmd %c.", cmd);
    }

    return count;
}

// AP Hash Function
static unsigned int log_hash(const char *str, const int len)
{
    unsigned int hash = 0;
    int i;
 
    for (i=0; i<len; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }
 
    return (hash & 0x7FFFFFFF) % PCTL_URL_HASH_SIZE;
}

static int log_add(const struct sk_buff *skb, int id, const char* host, int host_len, unsigned int stamp)
{
    pctl_owner *pOwner = NULL;
    pctl_log *pLog = NULL;

    struct hlist_head* pHash_list = NULL;
    struct hlist_node* pHash_node = NULL;
    struct iphdr *iph;
#ifdef PCTL_SUPPORT_IPV6
    struct ipv6hdr *iph6;
#endif
    int ret = 0;
    unsigned int hash = 0;

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
	int index = 0;
#endif

#ifdef PCTL_SUPPORT_IPV6
    if (4 == ip_hdr(skb)->version)
    {
        iph = ip_hdr(skb);
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
    }
#else
    iph = ip_hdr(skb);
#endif

    if(!host)
    {
        PCTL_ERROR("host is NULL.");
        return -1;
    }

    if(host_len < 0 || host_len >= PCTL_URL_LEN)
    {
        PCTL_ERROR("host_len is out of range, host_len=%d.",host_len);
        return -1;
    }

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
    for (index = 0; visit_exception_list[index] != NULL; index++) {
        if (_url_strstr(host, host + host_len, visit_exception_list[index])) {
            printWar("exception list match, ignor...%s\n", host);
            return -1;
        }
    }
#endif

    pOwner = owners + id;
    write_lock_bh(&pOwner->lock);

    /* 1. find host in hash. if entry exists, update it. */
    hash = log_hash(host, host_len);
    pHash_list = pOwner->hash_list + hash;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
    hlist_for_each_entry(pLog, pHash_list, hash_node)
#else
    hlist_for_each_entry(pLog, pHash_node, pHash_list, hash_node)
#endif
    {
        if(pLog->entry.host_len == host_len && !memcmp(pLog->entry.host, host, host_len)) 
        {
            PCTL_DEBUG("host is found in hash, update. hash=%d.",hash);

            /* update info */
            update_stats(&pLog->entry.stats, stamp, 0);
#ifdef PCTL_SUPPORT_IPV6
            if (4 == ip_hdr(skb)->version)
            {
                pLog->entry.v6 = false;
                pLog->entry.ip.addr4= ntohl(iph->daddr);//add by wangha
            }
            else if (6 == ipv6_hdr(skb)->version)
            {
                pLog->entry.v6 = true;
                memcpy((unsigned char *)&pLog->entry.ip.addr6, (unsigned char *)&iph6->daddr, sizeof(struct in6_addr));
            }
#else
            pLog->entry.v6 = false;
            pLog->entry.ip.addr4= ntohl(iph->saddr);//add by wangha
#endif      
            /* move it to list head */
            list_move(&pLog->log_node, &pOwner->log_list);
            goto out;
        }
    }

    /* 2. It is a new entry. */
    if(pOwner->log_len < PCTL_LOG_NUM) 
    {
        PCTL_DEBUG("alloc new entry. hash=%d.",hash);
         
        /* alloc new entry */
        pLog = kmalloc(sizeof(pctl_log), GFP_KERNEL);
        if(!pLog) {
            PCTL_ERROR("kmalloc failed.");
            ret = -1;
            goto out;
        }

        memset(pLog, 0, sizeof(pctl_log));
        INIT_LIST_HEAD(&pLog->log_node);
        INIT_HLIST_NODE(&pLog->hash_node);

        memcpy(pLog->entry.host, host, host_len);
        pLog->entry.host[host_len] = '\0';
        pLog->entry.host_len = host_len;
#ifdef PCTL_SUPPORT_IPV6
		if (4 == ip_hdr(skb)->version)
		{
			pLog->entry.v6 = false;
			pLog->entry.ip.addr4= ntohl(iph->daddr);//add by wangha
		}
		else if (6 == ipv6_hdr(skb)->version)
		{
			pLog->entry.v6 = true;
			memcpy((unsigned char *)&pLog->entry.ip.addr6, (unsigned char *)&iph6->daddr, sizeof(struct in6_addr));
		}
#else
		pLog->entry.v6 = false;
		pLog->entry.ip.addr4= ntohl(iph->saddr);//add by wangha
#endif
        init_stats(&pLog->entry.stats);
        update_stats(&pLog->entry.stats, stamp, 0);

        /* add to log list */
        list_add(&pLog->log_node, &pOwner->log_list);
        pOwner->log_len++;

        /* add to log hash */
        hlist_add_head(&pLog->hash_node, &pOwner->hash_list[hash]);

    }else /* log list is full */
    {
        PCTL_DEBUG("replace tail node. hash=%d.",hash);

        /* replace tail */
        pLog = list_entry(pOwner->log_list.prev, pctl_log, log_node);
        if(!pLog) 
        {
            PCTL_ERROR("SHOULD NOT happen! log_len=%d.",pOwner->log_len);
            ret = -1;
            goto out;
        }

        memcpy(pLog->entry.host, host, host_len);
        pLog->entry.host[host_len] = '\0';
        pLog->entry.host_len = host_len;
#ifdef PCTL_SUPPORT_IPV6
		if (4 == ip_hdr(skb)->version)
		{
			pLog->entry.v6 = false;
			pLog->entry.ip.addr4= ntohl(iph->daddr);//add by wangha
		}
		else if (6 == ipv6_hdr(skb)->version)
		{
			pLog->entry.v6 = true;
			memcpy((unsigned char *)&pLog->entry.ip.addr6, (unsigned char *)&iph6->daddr, sizeof(struct in6_addr));
		}
#else
		pLog->entry.v6 = false;
		pLog->entry.ip.addr4= ntohl(iph->saddr);//add by wangha
#endif
        init_stats(&pLog->entry.stats);
        update_stats(&pLog->entry.stats, stamp, 0);

        /* update hash */
        hlist_del(&pLog->hash_node);
        hlist_add_head(&pLog->hash_node, &pOwner->hash_list[hash]);

        /* move it to list head */
        list_move(&pLog->log_node, &pOwner->log_list);
    }

out:
    write_unlock_bh(&pOwner->lock);
    return ret;
}

static int log_clear(int id, bool initFlag)
{
    int i = 0;
    pctl_owner *pOwner = owners + id;
    pctl_log* pLog = NULL;
    pctl_log* pTmp = NULL;

    write_lock_bh(&pOwner->lock);

	//add by wanghao
	/*
	if (initFlag) {
    init_stats(&pOwner->today_stats);
		init_stats(&pOwner->yesterday_stats);
	}
	*/
	//add end
	
	if (initFlag) {
    init_stats_daytime(&pOwner->today_stats);
		init_stats_daytime(&pOwner->yesterday_stats);
	}
	traffic_reset(&pOwner->traffic);
    /* free log entry */
    list_for_each_entry_safe(pLog, pTmp, &pOwner->log_list, log_node)
    {
        hlist_del(&pLog->hash_node);
        list_del(&pLog->log_node);
        if (!initFlag && pLog->entry.traffic.curr) {
            PCTL_DEBUG("%s is still active\n", pLog->entry.host);
            traffic_reset(&pLog->entry.traffic);
            reset_stats(&pLog->entry.stats);
            continue;
        }
        PCTL_DEBUG("%s is not active\n", pLog->entry.host);
        if(pLog)
        {
            kfree(pLog);
            pLog = NULL;
        }
        pOwner->log_len--;
    }

    if(initFlag && pOwner->log_len != 0)
    {
        PCTL_ERROR("pOwner->log_len != 0!");
    }

#if 0
	/* do not clear history here, for these will be used in daily report */
    /* free history */
    for(i=0; i<PCTL_HISTORY_DAY_NUM ;i++) 
    {
        if(pOwner->day[i]) 
        {
            kfree(pOwner->day[i]);
            pOwner->day[i] = NULL;
        }
    }
    pOwner->day_idx = 0;
#endif

    write_unlock_bh(&pOwner->lock);

    return 0;
}

/* add by wanghao */
static int log_clear_history(int id)
{
    int i = 0;
    pctl_owner *pOwner = owners + id;

    write_lock_bh(&pOwner->lock);
	
    for(i=0; i<PCTL_HISTORY_DAY_NUM ;i++) 
    {
        if(pOwner->day[i]) 
        {
            kfree(pOwner->day[i]);
            pOwner->day[i] = NULL;
        }
    }
    pOwner->day_idx = 0;

    write_unlock_bh(&pOwner->lock);

    return 0;
}
/* add end */

static inline 
int sort_by_total(const void *entry1, const void *entry2)
{
    return ((const pctl_log_entry *)entry2)->stats.total - 
            ((const pctl_log_entry *)entry1)->stats.total;
}

static inline 
int sort_by_timestamp(const void *entry1, const void *entry2)
{
    return ((const pctl_log_entry *)entry2)->stats.timestamp - 
            ((const pctl_log_entry *)entry1)->stats.timestamp;
}

static int log_update_history(int id, unsigned int now)
{
    pctl_log *pLog = NULL;
    pctl_log *pTmp = NULL;
    pctl_history* pDay = NULL;
    pctl_log_entry *entry[PCTL_LOG_NUM] = {0};
    pctl_owner *pOwner = owners + id;
    int day_after = 0;
    int i = 0, num = 0;

    /* save log to history array */
    write_lock_bh(&pOwner->lock);

    day_after = now / 86400 - pOwner->today_stats.timestamp / 86400;
    if (0 == day_after || 0 == pOwner->today_stats.timestamp)
    {
		write_unlock_bh(&pOwner->lock);
        /* do nothing */
        return 0;
    }else if( day_after < 0 || day_after >= PCTL_HISTORY_DAY_NUM )
    {
        /* clear log */
        PCTL_ERROR("clear history. owner_id = %d.",id);
        write_unlock_bh(&pOwner->lock);
        log_clear(id, true);
        return 0;
    }

    PCTL_DEBUG("new day event. day_after=%d.",day_after);

    /* save log to history array */
    pDay = pOwner->day[pOwner->day_idx];
    if(NULL == pDay) 
    {
        PCTL_DEBUG("kmalloc id=%d, day_idx=%d.",id, pOwner->day_idx);
        pDay = kmalloc(sizeof(pctl_history), GFP_KERNEL);
        if(!pDay) 
        {
            PCTL_ERROR("kmalloc failed.");
            write_unlock_bh(&pOwner->lock);
            return -1;
        }
        pOwner->day[pOwner->day_idx] = pDay;
        memset(pDay, 0, sizeof(pctl_history));
    }else
    {
        PCTL_DEBUG("override id=%d, day_idx=%d.",id, pOwner->day_idx);
        /* override */
        memset(pDay, 0, sizeof(pctl_history));
    }

    list_for_each_entry(pLog, &pOwner->log_list, log_node)
    {
        entry[i] = &pLog->entry;
        i++;
    }
    num = i;
    if(num != pOwner->log_len) 
    {
        PCTL_ERROR("num != pOwner->log_len, %d %d.",num, pOwner->log_len);
        write_unlock_bh(&pOwner->lock);
        return -1;
    }

    if(num > PCTL_HISTORY_LOG_NUM) 
    {
        /* 1. sort by count */
        sort(entry, num, sizeof(void*), sort_by_total, NULL);
        num = PCTL_HISTORY_LOG_NUM;
        /* 2. sort by timestamp */
        sort(entry, num, sizeof(void*), sort_by_timestamp, NULL);
    }

    /* 3. save history */
    pDay->day_stats = pOwner->today_stats;
    pDay->num = num;

	/* add by wanghao, save to yesterday */
	pOwner->yesterday_stats = pOwner->today_stats;
	//redirectUpdate();
	/* add end */
	
    for(i=0; i<num; i++) 
    {
        pDay->log_entry[i] = *entry[i];
    }

    /* 4. clear history not in PCTL_HISTORY_DAY_NUM days. */
    for(i=1; i<day_after; i++) 
    {
        int clear_day_idx = ((pOwner->day_idx + i) % PCTL_HISTORY_DAY_NUM);
        if(pOwner->day[clear_day_idx]) 
        {
            kfree(pOwner->day[clear_day_idx]);
            pOwner->day[clear_day_idx] = NULL;
        }
    }
    pOwner->day_idx = ((pOwner->day_idx + day_after) % PCTL_HISTORY_DAY_NUM);

    /* 5. clear today's log */
    //init_stats(&pOwner->today_stats);
	init_stats_daytime(&pOwner->today_stats);
    /* free log entry */
    list_for_each_entry_safe(pLog, pTmp, &pOwner->log_list, log_node)
    {
        hlist_del(&pLog->hash_node);
        list_del(&pLog->log_node);
        if(pLog)
        {
            kfree(pLog);
            pLog = NULL;
        }
        pOwner->log_len--;
    }

    if(pOwner->log_len != 0)
    {
        PCTL_ERROR("pOwner->log_len != 0!");
    }

    write_unlock_bh(&pOwner->lock);

    return 0;
}

/* 
 * fn		static bool extractHandshakeFromSSL(const uint8_t *pSSLBuff, uint8_t **ppHandshake) 
 * brief	extract the handshake From SSL packet.
 * details	only get address of the pointer to handshake.
 *
 * param[in]	pSSL - pointer to the start of SSL packet in skb_buff.
 * param[out]	ppHandshake - address of pointer to the start of handshake message wrapped with SSLv3/TLS.
 *
 * return	BOOL
 * retval	true  succeed to extract handshake 
 *		false fail to extract handshake  
 * note		
 */
static bool extractHandshakeFromSSL(const uint8_t *pSSL, uint8_t **ppHandshake)
{
	SSL_MSG ssl;
	
	if ((ssl.type = *pSSL++) != HANDSHAKE)
	{
		return false;
	}
	/*
	ssl.version.majorVersion = *pSSL++;
	ssl.version.minorVersion = *pSSL++;
	*/
	pSSL += 2;
	
	ssl.length = ntohs(*((uint16_t *)pSSL));
	pSSL += 2;
	
	if(0 == ssl.length)
	{
		return false;
	}
	/*ssl.pContent = pSSL;*/
	*ppHandshake = (uint8_t *)pSSL;

	
	return true;
}
/* 
 * fn		static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t *ppSNIExt) 
 * brief	extract SNI extension form extensions.
 * details	get pointer to start position of SNI extension that exists in server name extension.
 *
 * param[in]	pExtensions - pointer to start of extensionList.
 * param[out]	ppSNIExt      - address of pointer to SNI extension.
 *
 * return	bool
 * retval	true - extract SNI extension successfully.
 *          false - extract SNI extension unsuccessfully.
 * note		
 */
static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t **ppSNIExt)
{
	int extensionsLen; /*length of all extensions.*/
	int handledExtLen;/*length of handled extensions.*/
	TLS_EXTENSION ext;

	extensionsLen = ntohs(*((uint16_t *)pExtensions));
	pExtensions += 2;
	
	for (handledExtLen = 0; handledExtLen < extensionsLen; )
	{
		ext.type = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.length = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.pData = (ext.length ? (uint8_t *)pExtensions : NULL);
		if (SERVER_NAME == ext.type)
		{
			*ppSNIExt = ext.pData;
			if (ext.length)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		pExtensions += ext.length;
		handledExtLen += (2 + 2 + ext.length);
	}

	return false;
}
/* 
 * fn		static  bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt) 
 * brief	extract SNI extension(Server_name)represents host_name from client_hello.
 * details	get pointer to start position of SNI extension from client_hello message.
 *
 * param[in]	pClientHello - pointer to start position of client_hello message.
 * param[out]	ppSNIExt - address of pointer to the start position of SNI extension in client_hello.
 *
 * return	bool
 * retval	true -get the SNI represents host_name.
 *			false - doesn't get the right SNI.
 * note		
 */
static bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt)
{
	HANDSHAKE_CLIENT_HELLO clientHello;
	/*
	clientHello.type = *pClientHello++;
	clientHello.length = NET_3BYTES_TO_HOST_UINT32(pClientHello);
	pClientHello += 3;
	Ignore type and length of client_hello.
	*/
	pClientHello += 4;
	
	clientHello.clientVersion.majorVersion = *pClientHello++;
	clientHello.clientVersion.minorVersion = *pClientHello++;
	/*SNI extension is not supported until TLS 1.0(version 0x0301)*/
	if (clientHello.clientVersion.majorVersion < 3
	 || (3 == clientHello.clientVersion.majorVersion && 0 == clientHello.clientVersion.minorVersion))
	{
		return false;
	}
	/*clientHello.random = pClientHello;*/
	pClientHello += 32;/*length of random is fixed.*/
	clientHello.sessionID.length = *pClientHello++;
	/*clientHello.sessionID.pData = pClientHello;*/
	pClientHello += clientHello.sessionID.length;
	clientHello.cipherSuites.length = ntohs(*((uint16_t *)pClientHello));
	pClientHello += 2;
	/*clientHello.cipherSuites.pData = pClientHello;*/
	pClientHello += clientHello.cipherSuites.length;
	clientHello.compression_methods.length = *pClientHello++;
	/*clientHello.compression_methods.pData = pClientHello;*/
	
	pClientHello += clientHello.compression_methods.length;
	clientHello.pExtensions = (uint8_t *)pClientHello;

	return extractSNIFromExtensions(clientHello.pExtensions, ppSNIExt);
}

/*!
 *\fn           unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, 
                                        const unsigned char* strCharSet)
 *\brief        find the url in str zone
 *\param[in]    start           start ptr of str zone.
 *\param[in]    end             end ptr of str zone.
 *\param[in]    strCharSet      the url you want to find
 *\return       url postion
 */
static unsigned char *_url_strstr(const unsigned char* start, 
                                  const unsigned char* end, const unsigned char* strCharSet)
{
    const unsigned char *s_temp = start;        /*the s_temp point to the s*/

    int l1, l2;

    l2 = strlen(strCharSet);
    
    if (!l2)
    {
        return (unsigned char *)start;
    }

    l1 = end - s_temp + 1;

    while (l1 >= l2)
    {
        l1--;

        if (!memcmp(s_temp, strCharSet, l2))
        {
            return (unsigned char *)s_temp;
        }

        s_temp++;
    }

    return NULL;
}

/* add by wanghao */
static inline int daysMatch(unsigned char *days, unsigned char weekday)
{
	int index = 0;

	for (index = 0; days[index] > 0; index++) {
		if (weekday == days[index]) {
			return index;
		}
	}

	printErr("weekday error: %d.", weekday);
	return -1;
}

static inline int hourIndex_match(unsigned int timeinfo, struct xtm *current_time)
{
    int ret = PCTL_STATUS_OK;
    int hourIndex = 0;

    hourIndex = current_time->hour;
    if ((0x1 << hourIndex) & timeinfo) {
        ret = PCTL_STATUS_OK;
    }
    else
    {
        ret = PCTL_STATUS_BEDTIME;
    }
    PCTL_DEBUG("ret = %d.",ret);
    
    return ret;
}

static inline int halfhourIndex_match(pctl_owner *pOwner, bool timeLimit, bool bedTime, struct time_limit_info *timeInfo, struct xtm *current_time)
{
	int ret = PCTL_STATUS_OK;
	int halfhourIndex = 0;
	unsigned int allowTime = 1440; //all day

	if (timeLimit) {
		allowTime = timeInfo->time + timeInfo->bonus_time;
		if ( (timeInfo->reward_time >= allowTime) || (pOwner->today_stats.total >= allowTime - timeInfo->reward_time) )
		{
			ret = PCTL_STATUS_TIME_LIMIT;
			PCTL_DEBUG("ret = %d.",ret);
			goto out;
		}
	}
	else
	{
		allowTime += timeInfo->bonus_time;
		if ( (timeInfo->reward_time >= allowTime) || (pOwner->today_stats.total >= allowTime - timeInfo->reward_time) )
		{
			ret = PCTL_STATUS_TIME_LIMIT;
			PCTL_DEBUG("ret = %d.",ret);
			goto out;
		}
	}

	//add by wanghao
	if (timeInfo->bonus_time) {
		/* if bonus time remained, skip bedTime && offTime */
		if (pOwner->today_stats.totalBonus < timeInfo->bonus_time) {
			ret = PCTL_STATUS_INBONUS;
			PCTL_DEBUG("In bonus time");
			goto out;
		}
	}

	if (bedTime) {
		if (current_time->hour >= 12) {
			halfhourIndex = (current_time->hour - 12) * 2;
			halfhourIndex += current_time->minute >= 30 ? 1 : 0;
			if ((0x1 << halfhourIndex) & timeInfo->afternoon) {
				ret = PCTL_STATUS_BEDTIME;
	            PCTL_DEBUG("ret = %d.",ret);
	            goto out;
			}
		}
		else {
			halfhourIndex = current_time->hour * 2;
			halfhourIndex += current_time->minute >= 30 ? 1 : 0;
			if ((0x1 << halfhourIndex) & timeInfo->forenoon) {
				ret = PCTL_STATUS_BEDTIME;
	            PCTL_DEBUG("ret = %d.",ret);
	            goto out;
			}
		}
	}

out:
	return ret;
}

static int match_time_advanced(const struct sk_buff *skb, const struct _xt_pctl_info *info, unsigned int stamp)
{
	int ret = PCTL_STATUS_OK;
    pctl_owner *pOwner = owners + info->id;
	struct xtm current_time;
	bool timelimit = false;
	bool bedTime = true;
	struct time_limit_info timeInfo;

	/* check internet pause */
    if (info->blocked) {
        PCTL_DEBUG("ret = %d.",ret);
        ret = PCTL_STATUS_BLOCKED;
        goto out;
    }

	localtime_1(&current_time, stamp);
    localtime_2(&current_time, stamp);

	read_lock_bh(&pOwner->lock);
	switch (current_time.weekday) {
	case 7:
		timelimit = info->advanced_enable & (0x1 << 6) ? true : false;
		bedTime = true;
		timeInfo.time = info->sun_time;
		timeInfo.bonus_time = info->today_bonus_time;
        timeInfo.reward_time = info->today_reward_time;
		timeInfo.forenoon = info->sun_forenoon;
		timeInfo.afternoon = info->sun_afternoon;
		ret = halfhourIndex_match(pOwner, timelimit, bedTime, &timeInfo, &current_time);
		break;
	case 1:
		timelimit = info->advanced_enable & (0x1 << 5) ? true : false;
		bedTime = true;
		timeInfo.time = info->mon_time;
		timeInfo.bonus_time = info->today_bonus_time;
        timeInfo.reward_time = info->today_reward_time;
		timeInfo.forenoon = info->mon_forenoon;
		timeInfo.afternoon = info->mon_afternoon;
		ret = halfhourIndex_match(pOwner, timelimit, bedTime, &timeInfo, &current_time);
		break;
	case 2:
		timelimit = info->advanced_enable & (0x1 << 4) ? true : false;
		bedTime = true;
		timeInfo.time = info->tue_time;
		timeInfo.bonus_time = info->today_bonus_time;
        timeInfo.reward_time = info->today_reward_time;
		timeInfo.forenoon = info->tue_forenoon;
		timeInfo.afternoon = info->tue_afternoon;
		ret = halfhourIndex_match(pOwner, timelimit, bedTime, &timeInfo, &current_time);
		break;
	case 3:
		timelimit = info->advanced_enable & (0x1 << 3) ? true : false;
		bedTime = true;
		timeInfo.time = info->wed_time ;
		timeInfo.bonus_time = info->today_bonus_time;
        timeInfo.reward_time = info->today_reward_time;
		timeInfo.forenoon = info->wed_forenoon;
		timeInfo.afternoon = info->wed_afternoon;
		ret = halfhourIndex_match(pOwner, timelimit, bedTime, &timeInfo, &current_time);
		break;
	case 4:
		timelimit = info->advanced_enable & (0x1 << 2) ? true : false;
		bedTime = true;
		timeInfo.time = info->thu_time ;
		timeInfo.bonus_time = info->today_bonus_time;
        timeInfo.reward_time = info->today_reward_time;
		timeInfo.forenoon = info->thu_forenoon;
		timeInfo.afternoon = info->thu_afternoon;
		ret = halfhourIndex_match(pOwner, timelimit, bedTime, &timeInfo, &current_time);
		break;
	case 5:
		timelimit = info->advanced_enable & (0x1 << 1) ? true : false;
		bedTime = true;
		timeInfo.time = info->fri_time ;
		timeInfo.bonus_time = info->today_bonus_time;
        timeInfo.reward_time = info->today_reward_time;
		timeInfo.forenoon = info->fri_forenoon;
		timeInfo.afternoon = info->fri_afternoon;
		ret = halfhourIndex_match(pOwner, timelimit, bedTime, &timeInfo, &current_time);
		break;
	case 6:
		timelimit = info->advanced_enable & (0x1 << 0) ? true : false;
		bedTime = true;
		timeInfo.time = info->sat_time ;
		timeInfo.bonus_time = info->today_bonus_time;
        timeInfo.reward_time = info->today_reward_time;
		timeInfo.forenoon = info->sat_forenoon;
		timeInfo.afternoon = info->sat_afternoon;
		ret = halfhourIndex_match(pOwner, timelimit, bedTime, &timeInfo, &current_time);
		break;
	default:
		printErr("Error weekday...\n");
		break;
	}
	read_unlock_bh(&pOwner->lock);
	
out:
	return ret;
}
/* add end */

static int match_time(const struct sk_buff *skb, const struct _xt_pctl_info *info, unsigned int stamp)
{
    int ret = PCTL_STATUS_OK;
    pctl_owner *pOwner = owners + info->id;

	struct xtm current_time;
	unsigned int week_timeinfo;

	//add by wanghao
	unsigned char days[] = {6, 5, 4, 3, 2, 1, 7, 0};
	int offset = 0;
	unsigned char tmp = 0;
	bool workday = false;
	bool workdayTime = false; //the workday before weekend
	bool weekendNight = false; //the weekend before workday
	//add end

    /* check internet pause */
    if(info->blocked) {
        ret = PCTL_STATUS_BLOCKED;
        PCTL_DEBUG("ret = %d.",ret);
        goto out;
    }

    localtime_1(&current_time, stamp);
    localtime_2(&current_time, stamp);

    //PCTL_DEBUG("%d %d %d %d %d %d",current_time.month, current_time.monthday, current_time.weekday,
    //                               current_time.hour, current_time.minute, current_time.second);

	//add by wanghao
	offset = daysMatch(days, current_time.weekday);
	workday = ((0x1 << offset) & info->workdays) ? true : false;

	if (workday) {
		if (current_time.weekday == 7) {
			tmp = 1;
		}
		else {
			tmp = current_time.weekday + 1;
		}
		offset = daysMatch(days, tmp);
		if (!((0x1 << offset) & info->workdays)) {
			workdayTime = true;
		}
	}
	else {
		if (current_time.weekday == 7) {
			tmp = 1;
		}
		else {
			tmp = current_time.weekday + 1;
		}
		offset = daysMatch(days, tmp);
		if ((0x1 << offset) & info->workdays) {
			weekendNight = true;
		}
	}
	
	//add end
	
    /* workday */
    //if(current_time.weekday >=1 && current_time.weekday <=5) 
	if (workday) {
		if (info->workday_limit) {
			read_lock_bh(&pOwner->lock);
			if (pOwner->today_stats.total >= info->workday_time + info->today_bonus_time) {
				read_unlock_bh(&pOwner->lock);
				ret = PCTL_STATUS_TIME_LIMIT;
				PCTL_DEBUG("ret = %d.",ret);
				goto out;
            }
			//add by wanghao
			else if (info->today_bonus_time) {
				read_unlock_bh(&pOwner->lock);
				ret = PCTL_STATUS_OK;
				PCTL_DEBUG("ret = %d.",ret);
				goto out;
			}
			read_unlock_bh(&pOwner->lock);	
        }
    }

    /* weekend */
    //if(current_time.weekday >=6 && current_time.weekday <=7) 
    if (!workday) {
        if (info->weekend_limit) {
            read_lock_bh(&pOwner->lock);
            if (pOwner->today_stats.total >= info->weekend_time + info->today_bonus_time) {
                 read_unlock_bh(&pOwner->lock);
                 ret = PCTL_STATUS_TIME_LIMIT;
                 PCTL_DEBUG("ret = %d.",ret);
                 goto out;
            }
            //add by wanghao
            else if (info->today_bonus_time) {
                read_unlock_bh(&pOwner->lock);
                ret = PCTL_STATUS_OK;
                PCTL_DEBUG("ret = %d.",ret);
                goto out;
            }
            read_unlock_bh(&pOwner->lock);
        }
    
    }
	
    /* hosts type, 0 is not support parental_control_v2 optimize*/
    if (info->hosts_type != 1 && info->hosts_type != 2) {
        /* Sunday to Thursday, school nights */
        /* Should also consider Friday, which is school day morning */
        //if(current_time.weekday != 6) 
        if (workday || weekendNight) {
            if (info->workday_bedtime) {
                /* contain 24:00 */
                if (info->workday_begin > info->workday_end) {
                    //if (current_time.weekday == 7)
                    if (weekendNight) {
                        /* Only limit night on Sunday */
                        if (current_time.minutes_day >= info->workday_begin) {
                            ret = PCTL_STATUS_BEDTIME;
                            PCTL_DEBUG("ret = %d.",ret);
                            goto out;
                        }
                    }
                    //else if (current_time.weekday == 5)
                    else if (workdayTime) {
                        /* Only limit morning on Friday */
                        if (current_time.minutes_day <= info->workday_end) {
                            ret = PCTL_STATUS_BEDTIME;
                            PCTL_DEBUG("ret = %d.",ret);
                            goto out;
                        }
                    }
                    else {
                        /* Monday to Thursday, limit both morning and night */
                        if(current_time.minutes_day >= info->workday_begin || 
                           current_time.minutes_day <= info->workday_end) {
                            ret = PCTL_STATUS_BEDTIME;
                            PCTL_DEBUG("ret = %d.",ret);
                            goto out;
                        }
                    }
                }

                /* not contain 24:00 */
                if (info->workday_begin <= info->workday_end) {
                    /* In this case no Friday */
                    if(current_time.minutes_day >= info->workday_begin && 
                       current_time.minutes_day <= info->workday_end &&
                       //current_time.weekday != 5)
                       !workdayTime) {
                        ret = PCTL_STATUS_BEDTIME;
                        PCTL_DEBUG("ret = %d.",ret);
                        goto out;
                    }
                }
            }
        }

        /* Friday and Saturday, weekend nights */
        /* Should also consider Sunday, which is weekend morning */
        //if(current_time.weekday >= 5 && current_time.weekday <= 7) 
        if (!workday || workdayTime) {
            if (info->weekend_bedtime) {
                /* contain 24:00 */
                if (info->weekend_begin > info->weekend_end) {
                    //if (current_time.weekday == 5)
                    if (workdayTime) {
                        /* Only limit night on Friday */
                        if (current_time.minutes_day >= info->weekend_begin) {
                            ret = PCTL_STATUS_BEDTIME;
                            PCTL_DEBUG("ret = %d.",ret);
                            goto out;
                        }
                    }
                    //else if (current_time.weekday == 7)
                    else if (weekendNight) {
                        /* Only limit morning on Sunday */
                        if (current_time.minutes_day <= info->weekend_end) {
                            ret = PCTL_STATUS_BEDTIME;
                            PCTL_DEBUG("ret = %d.",ret);
                            goto out;
                        }
                    }
                    else {
                        /* Limit both morning and night on Saturday*/
                        if(current_time.minutes_day >= info->weekend_begin || 
                           current_time.minutes_day <= info->weekend_end) {
                            ret = PCTL_STATUS_BEDTIME;
                            PCTL_DEBUG("ret = %d.",ret);
                            goto out;
                        }
                    }
                }

                /* not contain 24:00 */
                if (info->weekend_begin <= info->weekend_end) {
                    /* In this case no Sunday */
                    if(current_time.minutes_day >= info->weekend_begin &&
                       current_time.minutes_day <= info->weekend_end &&
                       //current_time.weekday != 7) 
                       !weekendNight) {
                        ret = PCTL_STATUS_BEDTIME;
                        PCTL_DEBUG("ret = %d.",ret);
                        goto out;
                    }
                }
            }
        }
    }

    /* hosts type, 0 is not support parental_control_v2 optimize*/
    if (info->hosts_type == 1 || info->hosts_type == 2) {
        /*parental_control_v2 optimize, Sunday to Saturday , all weekday*/
        switch (current_time.weekday) {
        case 7:
            week_timeinfo = info->sun_time;
            ret = hourIndex_match(week_timeinfo, &current_time);
            break;
        case 1:
            week_timeinfo = info->mon_time;
            ret = hourIndex_match(week_timeinfo, &current_time);
            break;
        case 2:
            week_timeinfo = info->tue_time;
            ret = hourIndex_match(week_timeinfo, &current_time);
            break;
        case 3:
            week_timeinfo = info->wed_time;
            ret = hourIndex_match(week_timeinfo, &current_time);
            break;
        case 4:
            week_timeinfo = info->thu_time;
            ret = hourIndex_match(week_timeinfo, &current_time);
            break;
        case 5:
            week_timeinfo = info->fri_time;
            ret = hourIndex_match(week_timeinfo, &current_time);
            break;
        case 6:
            week_timeinfo = info->sat_time;
            ret = hourIndex_match(week_timeinfo, &current_time);
            break;
        default:
            PCTL_ERROR("Error weekday...\n");
            break;
        }
    }

out:
    return ret;
}

static int match_http(const struct sk_buff *skb, const struct _xt_pctl_info *info, 
                      int status, unsigned int stamp)
{
    int i;
    int ret = PCTL_STATUS_OK;

    const struct iphdr *iph;
    struct tcphdr *tcph;
    unsigned char* http_payload_start;
    unsigned char* http_payload_end;
#ifdef PCTL_SUPPORT_IPV6
    const struct ipv6hdr *iph6;
    unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
    struct in6_addr filter_host_ip6;
#endif

/* add for homeshield block url, by Cheng Ming */
#ifdef PCTL_BLOCK_URL
    struct block_rule_entry rule; 
    char domain[128] = {0};
	int domainLen = 0;
    struct ethhdr *eth = eth_hdr(skb); 
#endif
/* add end */

#ifdef PCTL_SUPPORT_IPV6
    if (4 == ip_hdr(skb)->version)
    {
        iph = ip_hdr(skb);
        tcph = (void *)iph + iph->ihl*4;
        http_payload_start = (unsigned char *)tcph + tcph->doff*4;
        http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
        if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
        {
            return ret;
        }
        tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        http_payload_start = (unsigned char *)tcph + tcph->doff*4;
        http_payload_end = (unsigned char *)(iph6 + ntohs(iph6->payload_len) - 1);
    }
    else
    {
        goto out;
    }
#else
    iph = ip_hdr(skb);
    tcph = (void *)iph + iph->ihl*4;
    http_payload_start = (unsigned char *)tcph + tcph->doff*4;
    http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
#endif

    unsigned char* host_start = NULL;
    unsigned char* host_end = NULL;

#if PCTL_HTTP_REFERER
    unsigned char* referer_start = NULL;
    unsigned char* referer_end = NULL;
#endif

#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
		struct nf_conn *ct;
		enum ip_conntrack_info ctinfo;
#endif

    if (http_payload_start < http_payload_end)
    {
        host_start = _url_strstr(http_payload_start, http_payload_end, HOST_STR);
        if (host_start)
        {
            host_start += 8;
            host_end = _url_strstr(host_start, http_payload_end, HOST_END_STR);
            if(!host_end) 
            {
                host_start = NULL;
            }
        }

#if PCTL_HTTP_REFERER
        referer_start = _url_strstr(http_payload_start, http_payload_end, REFERER_STR);
        if (referer_start)
        {
            referer_start += 11;

            referer_start = _url_strstr(http_payload_start, http_payload_end, HTTP_STR);
            if(referer_start) 
            {
                referer_start += 7;
            }else
            {
                referer_start = _url_strstr(http_payload_start, http_payload_end, HTTPS_STR);
                if(referer_start)
                {
                    referer_start += 8;
                }
            }

            referer_end = _url_strstr(referer_start, http_payload_end, REFERER_END_STR);
            if(referer_end)
            {
                pTmp = _url_strstr(referer_start, referer_end, REFERER_END_STR2);
                if(pTmp) 
                {
                    referer_end = pTmp;
                }
            }else
            {
                 referer_start = NULL;
            }
        }
#endif
    }

#if DEBUG
    {
        unsigned char* pStr;
        if(host_start) 
        {
            printk("HTTP HOST: ");
            for (pStr = host_start; pStr != host_end; ++pStr)
            {
                printk("%c", *pStr);
            }
        }
#if PCTL_HTTP_REFERER        
        if(referer_start) 
        {
            printk(" REFERER: ");
            for (pStr = referer_start; pStr != referer_end; ++pStr)
            {
                printk("%c", *pStr);
            }     
        }
#endif
        if(host_start)
        {
            printk("\n");
        }
    }
#endif

    if (host_start)
    {
        /* blocked by time check, no need to check */
        if(PCTL_STATUS_OK != status) 
        {
            ret = status;
            goto out;
        }

        /* hosts type, 0 is not support hosts_type, 1 is black list, 2 is white list */
        if (info->hosts_type == 0 || info->hosts_type == 2)
        {
            /* white list is empty, parental control_v2 optimize is not allowed to surf the Internet */
            if (info->hosts_type == 2 && info->num_wl == 0)
            {
                ret = PCTL_STATUS_FILTER;
                goto out;
            }

            /* add by wanghao */
            for (i = 0; i < info->num_wl; ++i)
            {
                if ( _url_strstr(host_start, host_end, info->hosts_wl[i]) )
                {
                    PCTL_DEBUG("==== host_wl matched %s ====", info->hosts_wl[i]);
                    ret = PCTL_STATUS_OK;
                    /* add access record to log list */
                    log_add(skb, info->id, host_start, host_end - host_start, stamp);
                    goto out;
                }
                
#if PCTL_HTTP_REFERER
                if(referer_start) 
                {
                    if ( _url_strstr(referer_start, referer_end, info->hosts_wl[i]) )
                    {
                        PCTL_DEBUG("==== referer white list matched %s ====", info->hosts_wl[i]);
                        ret = PCTL_STATUS_OK;
                        /* add access record to log list */
                        log_add(skb, info->id, referer_start, referer_end - referer_start, stamp);
                        goto out;
                    }
                }
#endif
            }
            /* add end */
			
            /* hosts type,  2 is white list*/
            if (info->hosts_type == 2)
            {
                ret = PCTL_STATUS_FILTER;
                goto out;
            }
        }

        if (info->hosts_type == 0 || info->hosts_type == 1)
        {
            for (i = 0; i < info->num; ++i)
            {
                if ( _url_strstr(host_start, host_end, info->hosts[i]) )
                {
                    PCTL_DEBUG("==== host matched %s ====", info->hosts[i]);
                    ret = PCTL_STATUS_FILTER;
                    goto out;
                }
#if PCTL_HTTP_REFERER
                if(referer_start) 
                {
                     if ( _url_strstr(referer_start, referer_end, info->hosts[i]) )
                     {
                         PCTL_DEBUG("==== referer matched %s ====", info->hosts[i]);
                         ret = PCTL_STATUS_FILTER;
                         goto out;
                     }
                }
#endif
            }
        }

#if PCTL_HTTP_REFERER
       if(referer_start)
       {

           log_add(skb, info->id, referer_start, referer_end - referer_start, stamp);

       }else
#endif
       {
           log_add(skb, info->id, host_start, host_end - host_start, stamp);
       }

#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
	   /* This connection is OK to pass, let SFE know */
	   ct = nf_ct_get(skb, &ctinfo);
	   if (ct) 
	   {
		   ct->mark |= PCTL_HANDLE_OK_MARK;
	   }
#endif


    }else
    {
        /* not http 'get' packet, just let it go */
        ret = PCTL_STATUS_OK;
    }

out:
#if PCTL_REDIRECT
#ifdef PCTL_SUPPORT_IPV6
    if (4 == ip_hdr(skb)->version)
    {
        if (host_start &&  PCTL_STATUS_OK != ret && _url_strstr(http_payload_start, http_payload_end, GET_STR))
        {
            /* add for homeshield block url, by Cheng Ming */
#ifdef PCTL_BLOCK_URL
            domainLen = host_end - host_start; 
            if (domainLen > sizeof(domain)) {
                domainLen = sizeof(domain); 
            }
            strncpy(domain, host_start, domainLen); 
            domain[domainLen] = '\0'; 

            PCTL_ERROR("[Blocking URL] domain=%s", domain);

            spin_lock_bh(&block_url_nl_lock);
            memset(&rule, 0, sizeof(struct block_rule_entry));
            rule.cat_id = PCTL_BLOCK_URL_CATEGORY; 
            rule.time_stamp = jiffies; 
            rule.stats++; 
            strncpy(rule.url, domain, domainLen);
            
            block_url_add(&rule, eth, skb);  
            spin_unlock_bh(&block_url_nl_lock);
#endif
            /* add end */

            http_response(skb, skb->dev, ret);
        }
        else if (iph->daddr == htonl(FILTER_HOST_IP))
        {
            ret = PCTL_STATUS_FILTER;

            if (IS_TCP_FLAG_SYN(GET_TCP_FLAG(tcph)))
            {
                http_ack(skb, skb->dev);
            }
        }
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        inet_pton6((unsigned char *)&FILTER_HOST_IP_6, (unsigned char *)&filter_host_ip6);
        if (host_start &&  PCTL_STATUS_OK != ret && _url_strstr(http_payload_start, http_payload_end, GET_STR))
        {
            PCTL_DEBUG("http_response_ipv6 begin!!!");
            http_response_ipv6(skb, skb->dev, ret);
        }
        else if (ipv6_addr_comp(&iph6->daddr, &filter_host_ip6))
        {
            ret = PCTL_STATUS_FILTER;

            if (IS_TCP_FLAG_SYN(GET_TCP_FLAG(tcph)))
            {
                http_ack_ipv6(skb, skb->dev);
            }
        }
    }
#else
    if (host_start &&  PCTL_STATUS_OK != ret && _url_strstr(http_payload_start, http_payload_end, GET_STR))
    {
        /* add for homeshield block url, by Cheng Ming */
#ifdef PCTL_BLOCK_URL
            domainLen = host_end - host_start; 
            if (domainLen > sizeof(domain)) {
                domainLen = sizeof(domain); 
            }
            strncpy(domain, host_start, domainLen); 
            domain[domainLen] = '\0'; 

            PCTL_ERROR("[Blocking URL] domain=%s", domain);

            spin_lock_bh(&block_url_nl_lock);
            memset(&rule, 0, sizeof(struct block_rule_entry));
            rule.cat_id = PCTL_BLOCK_URL_CATEGORY; 
            rule.time_stamp = jiffies; 
            rule.stats++; 
            strncpy(rule.url, domain, domainLen);
            
            block_url_add(&rule, eth, skb);  
            spin_unlock_bh(&block_url_nl_lock);
#endif
            /* add end */
            
        http_response(skb, skb->dev, ret);
    }
    else if (iph->daddr == htonl(FILTER_HOST_IP))
    {
        ret = PCTL_STATUS_FILTER;

        if (IS_TCP_FLAG_SYN(GET_TCP_FLAG(tcph)))
        {
            http_ack(skb, skb->dev);
        }
    }
#endif
#endif

    return ret;
}

static int match_https(const struct sk_buff *skb, const struct _xt_pctl_info *info, 
                       int status, unsigned int stamp)
{
    int ret = PCTL_STATUS_OK;
    const struct iphdr *iph;
    struct tcphdr *tcph;
    int i;
    unsigned char *sslStart;
    unsigned char *sslEnd;
    uint8_t *pHandshake;
    uint8_t * pSNIExt;
#ifdef PCTL_SUPPORT_IPV6
    const struct ipv6hdr *iph6;
    unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
#endif

#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
		struct nf_conn *ct;
		enum ip_conntrack_info ctinfo;
#endif


    TLS_EXTENSION SNIExt;/*format is similar with server name extension*/
    int SNIListLen;
    int handledSNILen; 

    if(PCTL_STATUS_OK != status) 
    {
        ret = status;
        goto out;
    }

#ifdef PCTL_SUPPORT_IPV6
    if (4 == ip_hdr(skb)->version)
	{
        iph = ip_hdr(skb);
        tcph = (void *)iph + iph->ihl*4;
        sslStart = (unsigned char *)tcph + tcph->doff * 4;
        sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
        if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
        {
            goto out;
        }
        tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        sslStart = (unsigned char *)tcph + tcph->doff * 4;
        sslEnd = sslStart + (ntohs(iph6->payload_len) + sizeof(struct ipv6hdr) - offset_tcp - tcph->doff * 4);
    }
    else
    {
        goto out;
    }
#else
    iph = ip_hdr(skb);
    tcph = (void *)iph + iph->ihl*4;
    sslStart = (unsigned char *)tcph + tcph->doff * 4;
    sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
#endif
	
    if (sslStart >= sslEnd)
    {
        /*UNIDENTIFY*/
        goto out;
    }
    if ((!extractHandshakeFromSSL(sslStart, &pHandshake))
        || (*pHandshake != CLIENT_HELLO)
        || (!extractSNIFromClientHello(pHandshake, &pSNIExt)))
    {
        /*UNIDENTIFY*/
        goto out;
    }

    SNIListLen = ntohs(*((uint16_t *)pSNIExt));
    pSNIExt += 2;

    for (handledSNILen = 0; handledSNILen < SNIListLen; )
    {
        SNIExt.type = *pSNIExt++;
        SNIExt.length = ntohs(*((uint16_t *)pSNIExt));
        pSNIExt += 2;
        SNIExt.pData = (uint8_t *)pSNIExt;
        pSNIExt += SNIExt.length;
        /*Does CLENT HELLO  fragment have impact on SNI?*/
        if (pSNIExt > sslEnd)
        {
            /*UNIDENTIFY*/
            goto out;
        }
        handledSNILen += (1 + 2 + SNIExt.length);

#if DEBUG
        {
            printk("HTTPS HOST: ");
            for (i=0;i<SNIExt.length;i++)
                printk("%c",*(SNIExt.pData+i));
            printk("\n");
        }
#endif

        if (HOST_NAME == SNIExt.type)
        {
            /* hosts type, 0 is not support hosts_type, 1 is black list, 2 is white list */
            if (info->hosts_type == 0 || info->hosts_type == 2)
            {
                /* white list is empty, parental control_v2 optimize is not allowed to surf the Internet */
                if (info->hosts_type == 2 && info->num_wl == 0)
                {
                    ret = PCTL_STATUS_FILTER;
                    goto out;
                }
			
                /* add by wanghao */
                for (i = 0; i < info->num_wl; ++i)
                {
                    if(_url_strstr(SNIExt.pData,pSNIExt,info->hosts_wl[i]))
                    {
                        PCTL_DEBUG("==== white list matched %s ====", info->hosts_wl[i]);
                        ret = PCTL_STATUS_OK;
                        /* add access record to log list */
                        log_add(skb, info->id, SNIExt.pData, pSNIExt - SNIExt.pData, stamp);
                        goto out;
                    }
                }
                /* add end */

                /* hosts type,  2 is white list*/
                if (info->hosts_type == 2)
                {
                    ret = PCTL_STATUS_FILTER;
                    goto out;
                }
            }

            if (info->hosts_type == 0 || info->hosts_type == 1)
            {
                for (i = 0; i < info->num; ++i)
                {
                     if(_url_strstr(SNIExt.pData,pSNIExt,info->hosts[i]))
                     {
                         PCTL_DEBUG("==== matched %s ====", info->hosts[i]);
                         ret = PCTL_STATUS_FILTER;
                     }
                }
            }
            log_add(skb, info->id, SNIExt.pData, pSNIExt - SNIExt.pData, stamp);

#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
			/* This connection is OK to pass, let SFE know */
			ct = nf_ct_get(skb, &ctinfo);
			if (ct) 
			{
				ct->mark |= PCTL_HANDLE_OK_MARK;
			}
#endif

        }
    }

out:
    return ret;    	
}

#if PCTL_DNS_REDIRECT
static unsigned int _transDomain2Buf(unsigned char *dns, 
                                     unsigned char *buf, signed int bufLen)
{
    signed int index;
    signed int orig_bufLen = bufLen;
    while(('\0' != *dns) && (bufLen > 0))
    {
        for (index = *dns; (index > 0) && (bufLen > 0); index --, bufLen --)
        {
            *(buf++) = *(++dns);
        }
        *(buf ++) = '.';
        dns ++;
        bufLen --;
    }

    if (bufLen < orig_bufLen)
    {
        bufLen ++;
        buf --;
    }
    
    *buf = '\0';
    return (orig_bufLen - bufLen);
}

static int match_dns(const struct sk_buff *skb, const struct _xt_pctl_info *info, 
                       int status, unsigned int stamp)
{
    int ret = PCTL_STATUS_OK;

    const struct iphdr *iph;
	struct udphdr *udph;
#ifdef PCTL_SUPPORT_IPV6
	const struct ipv6hdr *iph6;
	unsigned int offset_udp = 0;
	struct udphdr udph_buf;
#endif
	int dns_len;
	int i = 0;
    int i_count = 0;
    dns_header *pDnsHdr = NULL;
    unsigned char *pTmp = NULL;
    unsigned char domain[PCTL_MAX_DNS_SIZE];
    unsigned int pkt_len = 0;
    unsigned int domain_len = 0;
    unsigned short query_type = 0;

#ifdef PCTL_SUPPORT_IPV6
	if (4 == ip_hdr(skb)->version)
	{
		iph = ip_hdr(skb);
    	udph = (void *)iph + iph->ihl*4;
		dns_len = (unsigned int) ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);
	}
	else if (6 == ipv6_hdr(skb)->version)
	{
		iph6 = ipv6_hdr(skb);
		if (ipv6_find_hdr(skb, &offset_udp, IPPROTO_UDP, NULL, NULL) < 0)
		{
			goto out;
		}
    	udph = skb_header_pointer(skb, offset_udp, sizeof(udph_buf), &udph_buf);
		dns_len = (unsigned int) ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);
	}
	else
	{
		goto out;
	}
#else
	iph = ip_hdr(skb);
    udph = (void *)iph + iph->ihl*4;
	dns_len = (unsigned int) ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);
#endif

    if(PCTL_STATUS_OK != status) 
    {
        pDnsHdr = (void *) udph + sizeof(struct udphdr);
        pTmp = (unsigned char *)pDnsHdr + sizeof(dns_header);
        domain_len = _transDomain2Buf(pTmp, domain, PCTL_MAX_DNS_SIZE - 1);
        memcpy(&query_type, (pTmp + domain_len + 2), sizeof(unsigned short));
        ret = status;
        goto out;
    }

    if(dns_len < 0)
    {
        PCTL_DEBUG("dns_len = %d. (<0)", dns_len);
        ret = PCTL_STATUS_OK;
        goto out;
    }

    if (dns_len >= PCTL_MAX_DNS_SIZE)
    {
        PCTL_DEBUG("dns_len = %d > %d",dns_len, PCTL_MAX_DNS_SIZE);
        ret = PCTL_STATUS_OK;
        goto out;
    }

    pDnsHdr = (void *) udph + sizeof(struct udphdr);
    if (0 != (ntohs(pDnsHdr->flags) & 0x8000)) /* If not request */
    {
        ret = PCTL_STATUS_OK;
        goto out;
    }

    pTmp = (unsigned char *)pDnsHdr + sizeof(dns_header);
    for (i_count = 0; i_count < ntohs(pDnsHdr->nQDCount) && pkt_len < dns_len; i_count ++)
    {
        domain_len = _transDomain2Buf(pTmp, domain, PCTL_MAX_DNS_SIZE - 1);
		memcpy(&query_type, (pTmp + domain_len + 2), sizeof(unsigned short));

        /* hosts type, 0 is not support hosts_type, 1 is black list, 2 is white list */
        if (info->hosts_type == 0 || info->hosts_type == 2)
        {
            /* white list is empty, parental control_v2 optimize is not allowed to surf the Internet */
            if (info->hosts_type == 2 && info->num_wl == 0)
            {
                ret = PCTL_STATUS_FILTER;
                goto out;
            }
			
            /* add by wanghao */
            for (i = 0; i < info->num_wl; ++i)
            {    
                if (_url_strstr(domain, domain + domain_len, info->hosts_wl[i]))
                {
                    PCTL_DEBUG("==== white list matched %s ====", info->hosts_wl[i]);
                    ret = PCTL_STATUS_OK;
                    goto out;
                }
            }
            /* add end */

            /* hosts type,  2 is white list*/
            if (info->hosts_type == 2)
            {
                ret = PCTL_STATUS_FILTER;
                goto out;
            }
        }
		
        if (info->hosts_type == 0 || info->hosts_type == 1)
        {
            for (i = 0; i < info->num; ++i)
            {    
                if (_url_strstr(domain, domain + domain_len, info->hosts[i]))
                {
                    PCTL_DEBUG("==== matched %s query_type: 0X%04X====", info->hosts[i], ntohs(query_type));
                    ret = PCTL_STATUS_FILTER;
                    goto out;
                }
            }
        }
        
        pkt_len += domain_len + 4 + 1;
        pTmp    += domain_len + 4 + 1;
    }

out:
    if(PCTL_STATUS_OK != ret) 
    {
#ifdef PCTL_SUPPORT_IPV6
    	if (4 == ip_hdr(skb)->version)
    	{
	        if(dns_response(skb, skb->dev, ret, ntohs(query_type)) < 0 && (PCTL_STATUS_FILTER == ret))
	        {
	            ret = PCTL_STATUS_OK;
	        }
    	}
		else if (6 == ipv6_hdr(skb)->version)
		{
			if(dns_response_ipv6(skb, skb->dev, ret, ntohs(query_type)) < 0 && (PCTL_STATUS_FILTER == ret))
	        {
	            ret = PCTL_STATUS_OK;
	        }
		}
#else
		if(dns_response(skb, skb->dev, ret, ntohs(query_type)) < 0 && (PCTL_STATUS_FILTER == ret))
	    {
	        ret = PCTL_STATUS_OK;
	    }
#endif
    }
    return ret;
}
#endif

#if PCTL_DEVICE_INFO

static int device_proc_read(struct seq_file *s, void *unused)
{
    int index;
    unsigned char mac_null[ETH_ALEN] = {0};

    read_lock_bh(&device_info_lock);

    for(index=0; index<DEVICE_INFO_NUM; index++) 
    {
        if(0 != memcmp(devices[index].mac, mac_null, ETH_ALEN)) 
        {
            seq_printf(s, "%02X-%02X-%02X-%02X-%02X-%02X %d %d\n",
                       devices[index].mac[0],
                       devices[index].mac[1],
                       devices[index].mac[2],
                       devices[index].mac[3],
                       devices[index].mac[4],
                       devices[index].mac[5],
                       devices[index].type,
                       devices[index].noRedirect);
        }
    }
    read_unlock_bh(&device_info_lock);

/*	
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
	struct url_class_entry *pos = NULL;
	spin_lock_bh(&url_class_nl_lock);
	list_for_each_entry(pos, &url_class_list, list) {
		printWar("1111 pos=%p src=%pI4 url=%s ips=%d cat=%x dns=%x\n", pos, &pos->dev_addr, pos->url, pos->url_addr_len, pos->cat_id, pos->dns_id);
	}
	spin_unlock_bh(&url_class_nl_lock);
#endif	
*/
	
    return 0;
}

static int device_proc_open(struct inode *inode, struct file *file)  
{  
    return single_open(file, device_proc_read, NULL);
}

static ssize_t device_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data)
{
    char cmd = 0;
	char kbuf[2] = {0};
    if(count != 2) 
    {
         PCTL_ERROR("count = %d.", count);
         return -1;
    }

	if(copy_from_user(kbuf, buf, 2))
	{
		return -EFAULT;
	}
	
    cmd = kbuf[0];

    write_lock_bh(&device_info_lock);
    if('f' == cmd) 
    {
        memset(devices, 0, sizeof(devices));
    }
    else
    {
        PCTL_ERROR("bad cmd %c.", cmd);
    }
    write_unlock_bh(&device_info_lock);

    return count;
}

static int mac_hash(const unsigned char* mac)
{
    unsigned int sum = mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5];
    return (sum % DEVICE_INFO_NUM);
}

static device_info * device_mac_find_slob(const unsigned char* mac, device_info *devlist)
{
    int hash = mac_hash(mac);
    int index = 0, step = 0;

    for(step=0; step<3; step++) 
    {
        index=(hash + step) % DEVICE_INFO_NUM;
        if(!memcmp(mac, devlist[index].mac, ETH_ALEN)) 
        {
            return &devlist[index];
        }
    }
    return NULL;
}

static device_info * device_mac_find_empty_slob(const unsigned char* mac, device_info *devlist)
{
    int hash = mac_hash(mac);
    int index = 0, step = 0;
    unsigned char mac_null[ETH_ALEN] = {0};

    for(step=0; step<3; step++) 
    {
        index=(hash + step) % DEVICE_INFO_NUM;
        if(!memcmp(mac_null, devlist[index].mac, ETH_ALEN)) 
        {
            return &devlist[index];
        }
    }
    return NULL;
}

static DEVICE_TYPE check_user_agent(const unsigned char* start, const unsigned char* end)
{
    DEVICE_TYPE type = DEVICE_TYPE_OTHER;
    char buf[DEVICE_INFO_USER_AGENT_LAN + 1];
    int len = end - start;
    int i = 0;

    if(len >= DEVICE_INFO_USER_AGENT_LAN) 
    {
        PCTL_ERROR("user agent too long.");
        return type;
    }

    memset(buf, 0, DEVICE_INFO_USER_AGENT_LAN + 1);
    memcpy(buf, start, len);
    for(i=0; i<len; i++)
    {
        if(buf[i] >= 'A' && buf[i] <= 'Z') 
        {
            buf[i] = 'a' + (buf[i] - 'A');
        }
    }

    if(strstr(buf, "pad") )
    {
        type = DEVICE_TYPE_TABLET;
    }else if(strstr(buf, "android") ||
             strstr(buf, "phone") || 
             strstr(buf, "mobile") )
    {
        type = DEVICE_TYPE_PHONE;
    }else if(strstr(buf, "windows") ||
             strstr(buf, "mac") )
    {
        type = DEVICE_TYPE_PC;
    }else
    {
        type = DEVICE_TYPE_OTHER;
    }

    return type;
}

static int check_device_info(const struct sk_buff *skb)
{
    const struct ethhdr *pEthHdr = (struct ethhdr *)skb_mac_header(skb);
    device_info *device = NULL;
    DEVICE_TYPE type = DEVICE_TYPE_OTHER;
	const struct iphdr *iph; 
	const struct tcphdr *tcph;
#ifdef PCTL_SUPPORT_IPV6
	const struct ipv6hdr *iph6;
	unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
#endif
	unsigned char* http_payload_start;
	unsigned char* http_payload_end; 

#ifdef PCTL_SUPPORT_IPV6	
	if (4 == ip_hdr(skb)->version)
	{
		iph = ip_hdr(skb);
		if (iph->protocol != IPPROTO_TCP) {
			goto out;
		}
		tcph = (void *)iph + iph->ihl*4;
		http_payload_start = (unsigned char *)tcph + tcph->doff*4;
 		http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
	}
	else if (6 == ipv6_hdr(skb)->version)
	{
		iph6 = ipv6_hdr(skb);
		if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
		{
			goto out;
		}
		tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
		http_payload_start = (unsigned char *)tcph + tcph->doff*4;
		http_payload_end = (unsigned char *)(iph6 + ntohs(iph6->payload_len) - 1);		
	}
	else
	{
		goto out;
	}
#else
	iph = ip_hdr(skb);
	if (iph->protocol != IPPROTO_TCP) {
		goto out;
	}
	tcph = (void *)iph + iph->ihl*4;
	http_payload_start = (unsigned char *)tcph + tcph->doff*4;
 	http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
#endif

    unsigned char* user_agent_start = NULL;
    unsigned char* user_agent_end = NULL;

    if(!pEthHdr) 
    {
        PCTL_ERROR("pEthHdr is NULL");
        goto out;
    }

    device = device_mac_find_slob(pEthHdr->h_source, devices);
    if(device && device->type != DEVICE_TYPE_OTHER)
    {
        /* already get device type ,just return */
        goto out;
    }

    if (http_payload_start < http_payload_end)
    {
        user_agent_start = _url_strstr(http_payload_start, http_payload_end, USER_AGENT_STR);
        if (user_agent_start)
        {
            user_agent_start += 14;
            user_agent_end = _url_strstr(user_agent_start, http_payload_end, USER_AGENT_END_STR);
            if(!user_agent_end) 
            {
                user_agent_start = NULL;
            }
        }
#if DEBUG
        if(user_agent_start) 
        {
            unsigned char* pStr;
            printk("USER_AGENT: ");
            for (pStr = user_agent_start; pStr != user_agent_end; ++pStr)
            {
                printk("%c", *pStr);
            }
            printk("\n");
        }
#endif
        if(user_agent_start) 
        {
            type = check_user_agent(user_agent_start, user_agent_end);

            if(DEVICE_TYPE_OTHER != type) 
            {
                if(device)
                {
                    device->type = type;
                }
                else
                {
                    device = device_mac_find_empty_slob(pEthHdr->h_source, devices);
                    if(device) 
                    {
                        PCTL_DEBUG("device_type = %d, device=0x%p.",type, device);
                        memcpy(device->mac, pEthHdr->h_source, ETH_ALEN);
                        device->type = type;
                        device->noRedirect = false;//add by wanghao
                    }else
                    {
                        PCTL_ERROR("find empty slob failed.");
                        goto out;
                    }
                }
            }
        }
    }

out:
    return 0;
}

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
bool device_keepalive_check(struct sk_buff *skb)
{
    device_info *device = NULL;
    const unsigned char *source = NULL;

    source = eth_hdr(skb)->h_source;
    //PCTL_DEBUG("device access: "H_MACFMT"\n", H_NMACQUAD(source));
    device = device_mac_find_slob(source, devices);
    if(!device)
    {
        device = device_mac_find_empty_slob(source, devices);
        if(device)
        {
            PCTL_DEBUG("Record new online device: "H_MACFMT"\n", H_NMACQUAD(source));
            memset(device, 0 ,sizeof(device_info));
            memcpy(device->mac, source, ETH_ALEN);
            device->keepaliveTime = jiffies;
            device->type = DEVICE_TYPE_OTHER;
            device->noRedirect = false;//add by wanghao
        }
        else
        {
            PCTL_ERROR("No room, skip blog for device: "H_MACFMT"\n", H_NMACQUAD(source));
            return true;
        }
    }

    return false;
}

bool device_keepalive_cb(const unsigned char *source)
{
    device_info *device = NULL;
    
    if(!source)
    {
        return false;
    }   
    
    //PCTL_DEBUG("device access: "H_MACFMT"\n", H_NMACQUAD(source));
    device = device_mac_find_slob(source, devices);
    if(device)
    {
        if(time_after(jiffies, device->keepaliveTime + BLOG_CONNECTION_KEEPALIVE_INTERVAL))
        {
            device->keepaliveTime = jiffies;
            return true;
        }
    }

    return false;
}
bool skb_keepalive_cb(void *skb_in)
{
    const unsigned char *source = NULL;
    struct sk_buff * skb = (struct sk_buff *)skb_in;
    if(skb)
    {
        source = eth_hdr(skb)->h_source;
    }
    else
    {
        return false;
    }   
        
    return device_keepalive_cb(source);
}

bool fkb_keepalive_cb(void *pBuf)
{
    const unsigned char *source = NULL;
    struct ethhdr * ethhd = NULL;   

    if(pBuf)
    {
        ethhd = (struct ethhdr *)pBuf;
        source = ethhd->h_source;
    }
    else
    {
        return false;
    }   
    
    return device_keepalive_cb(source);
}
#endif
#endif


#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS	
/* add by wanghao */
static void skb_debugFy(const struct sk_buff *skb)
{
#define NUM2PRINT 150
	char buf[NUM2PRINT * 3 + 1];	/* 3 chars per byte */
	int i = 0;
	for (i = 0; i < skb->len && i < NUM2PRINT; i++) {
		sprintf(buf + i * 3, "%2.2x ", 0xff & skb->data[i]);
	}
	printWar("skb[%p]: %s\n", skb->head, buf);
}

static inline void strncpy_safe(char *__dest, const char *__src, size_t __n)
{
	strncpy(__dest, __src, __n);
	__dest[__n] = '\0';
}

void dump_url_addr_list(const char *funcionName, const struct list_head *head)
{
	struct url_addr_entry *entry = NULL;
	struct url_addr_entry *next = NULL;
	int index = 1;

	printErr("%s:\n", funcionName);
	list_for_each_entry_safe(entry, next, head, list) {
		if (!entry->v6)
			printErr("index = %d entry = %pI4\n", index++, &entry->addr.addr4);
		else
			printErr("index = %d entry = %pI6\n", index++, &entry->addr.addr6);
	}
}

static int _url_class_list_init(void)
{
#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC	
	int index = 0;
	struct url_class_entry *pNode = NULL;

	INIT_LIST_HEAD(&url_class_buffer_list);
	INIT_LIST_HEAD(&url_class_list);
	for (index = 0; index < MAX_URL_CLASS_ENTRY_LEN; index++) {
		pNode = kmalloc(sizeof(struct url_class_entry), GFP_KERNEL);
		if (!pNode)
		{
			printErr("failed to allocate memory for url_class_entry");
			return -1;
		}
		memset(pNode, 0, sizeof(struct url_class_entry));
		pNode->id = index;
		list_add(&pNode->list, &url_class_buffer_list);
	}

	return 0;
#else
	int index = 0;

	memset(url_class_array, 0, sizeof(url_class_array));
	INIT_LIST_HEAD(&url_class_buffer_list);
	INIT_LIST_HEAD(&url_class_list);
	for (index = 0; index < MAX_URL_CLASS_ENTRY_LEN; index++) {
		url_class_array[index].id = index;
		list_add(&url_class_array[index].list, &url_class_buffer_list);
	}

	return 0;
#endif
}

#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC	
static int _url_class_list_clean(void)
{
	struct url_class_entry *entry = NULL;
	struct url_addr_entry *pos = NULL;
	struct url_addr_entry *n = NULL;
	struct list_head tmp;
	int tmpId = 0;

	while (!list_empty(&url_class_list)) {	
		entry = list_first_entry(&url_class_list, struct url_class_entry, list);

		list_for_each_entry_safe(pos, n, &entry->url_addr, list) {
			entry->url_addr_len--;
			memset(&pos->addr.addr6, 0, sizeof(pos->addr));
			list_move(&pos->list, &url_addr_list);
		}

		memcpy(&tmp, &entry->list, sizeof(struct list_head));
		tmpId = entry->id;
		memset(entry, 0, sizeof(struct url_class_entry));
		memcpy(&entry->list, &tmp, sizeof(struct list_head));
		entry->id = tmpId;
		list_move(&entry->list, &url_class_buffer_list);
	}

	return 0;
}

static int _url_class_list_destroy(void)
{
	struct url_class_entry  *pNode = NULL;

	_url_class_list_clean();
	
	while (!list_empty(&url_class_buffer_list)) {
		pNode = list_first_entry(&url_class_buffer_list, struct url_class_entry, list);
		list_del(&pNode->list);
		kfree(pNode);
	}

	return 0;	
}
#endif


static int _url_addr_list_init(void)
{
#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC		
	int index = 0;
	struct url_addr_entry *pNode = NULL;

	INIT_LIST_HEAD(&url_addr_list);
	for (index = 0; index < MAX_URL_ADDR_ENTRY_LEN; index++) {
		pNode = kmalloc(sizeof(struct url_addr_entry), GFP_KERNEL);
		if (!pNode)
		{
			printErr("failed to allocate memory for url_addr_entry");
			return -1;
		}
		memset(pNode, 0, sizeof(struct url_addr_entry));
		list_add(&pNode->list, &url_addr_list);
	}
	return 0;
#else
	int index = 0;

	memset(url_addr_array, 0, sizeof(url_addr_array));
	INIT_LIST_HEAD(&url_addr_list);
	for (index = 0; index < MAX_URL_ADDR_ENTRY_LEN; index++) {
		list_add(&url_addr_array[index].list, &url_addr_list);
	}
	return 0;
#endif	
}

#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC	
static int _url_addr_list_clean(void)
{
	/* will clean by _url_class_list_clean, so no clean */
	return 0;
}

static int _url_addr_list_destroy(void)
{
	struct url_addr_entry  *pNode = NULL;

	while (!list_empty(&url_addr_list)) {
		pNode = list_first_entry(&url_addr_list, struct url_addr_entry, list);
		list_del(&pNode->list);
		kfree(pNode);
	}

	return 0;	
}
#endif

int url_class_add(const struct sk_buff *skb, unsigned short dns_id, unsigned char *msg, int msg_len, const struct _xt_pctl_info *info, unsigned short query_type, unsigned int stamp)
{
	int index = 0;
	struct url_class_entry *pos = NULL;
	struct url_class_entry *entry = NULL;
    const struct iphdr *iph;
#ifdef PCTL_SUPPORT_IPV6
    const struct ipv6hdr *iph6;

    if (4 == ip_hdr(skb)->version) {
        iph = ip_hdr(skb);
    }
    else if(6 == ipv6_hdr(skb)->version) {
        iph6 = ipv6_hdr(skb);
    }
    else {
        return 1;
    }
#else
    iph = ip_hdr(skb);
#endif

	if (unlikely(!msg)) {
        return -1;
    }
	
	spin_lock_bh(&url_class_nl_lock);
	if (unlikely(list_empty(&url_class_buffer_list))) {
		spin_unlock_bh(&url_class_nl_lock);
		printErr("no room for a new url entry, url=%s.  We need start urlclass timer ... \n", msg);
		return 1;
	}

	//exception list
	for (index = 0; url_class_exception_list[index] != NULL; index++) {
		if (_url_strstr(msg, msg + msg_len, url_class_exception_list[index])) {
			spin_unlock_bh(&url_class_nl_lock);
			printWar("exception list match, ignor...%s\n", msg);
			return 1;
		}
	}

	//not a duplicated entry
	list_for_each_entry(pos, &url_class_list, list) {
#ifdef PCTL_SUPPORT_IPV6
        if (!pos->v6 && 4 == ip_hdr(skb)->version) {
            if (!strncmp(pos->url, msg, msg_len) && pos->dev_addr.addr4 == ntohl(iph->saddr) && pos->query_type == query_type) { 
                pos->dns_id = dns_id;
                pos->send_flag = URL_SEND_PENDING;
				pos->timestamp = stamp;
                spin_unlock_bh(&url_class_nl_lock);
                printWar("duplicated entry match, ignor...%s\n", msg);
                return 1;
            }
        }
        else if (pos->v6 && 6 == ipv6_hdr(skb)->version) {
            if (!strncmp(pos->url, msg, msg_len) && ipv6_addr_comp(&pos->dev_addr.addr6, &iph6->saddr) && pos->query_type == query_type) {
                pos->dns_id = dns_id;
                pos->send_flag = URL_SEND_PENDING;
				pos->timestamp = stamp;
                spin_unlock_bh(&url_class_nl_lock);
                printWar("duplicated entry match, ignor...%s\n", msg);
                return 1;
            }
        }
#else
		if (!strncmp(pos->url, msg, msg_len) && pos->dev_addr.addr4 == ntohl(iph->saddr)) { 
			pos->dns_id = dns_id;
			pos->send_flag = URL_SEND_PENDING;
			pos->timestamp = stamp;
			spin_unlock_bh(&url_class_nl_lock);
			printWar("duplicated entry match, ignor...%s\n", msg);
			return 1;
		}
#endif
	}
	
	entry = list_first_entry(&url_class_buffer_list, struct url_class_entry, list);
	entry->info_id = info->id;
	entry->dns_id = dns_id;
	entry->url_addr_len = 0;
    entry->query_type = query_type;
	entry->timestamp = stamp;
#ifdef PCTL_SUPPORT_IPV6
    if (4 == ip_hdr(skb)->version) {
        entry->v6 = false;
        entry->dev_addr.addr4 = ntohl(iph->saddr);
    }
    else if(6 == ipv6_hdr(skb)->version) {
        entry->v6 = true;
        memcpy((unsigned char *)&entry->dev_addr.addr6, (unsigned char *)&iph6->saddr, sizeof(struct in6_addr));
    }
#else
    entry->v6 = false;
    entry->dev_addr.addr4 = ntohl(iph->saddr);
#endif
	entry->cat_map = info->cat_map | URL_CAT_SECURITY;
	strncpy_safe(entry->url, msg, msg_len);
	entry->url_len = msg_len;
	entry->send_flag = URL_SEND_PENDING;
	INIT_LIST_HEAD(&entry->url_addr);

	printWar("add a url entry, url=%s\n", entry->url);

	printWar("add a url entry, id=%d, dns_id=0x%x, url=%s\n", entry->id, entry->dns_id, entry->url);

	list_move(&entry->list, &url_class_list);

	urlclass_entry_used++;
	spin_unlock_bh(&url_class_nl_lock);

	return 0;
}

int url_class_del(struct url_class_entry *entry)
{
	struct url_addr_entry *pos = NULL;
	struct url_addr_entry *n = NULL;
	struct list_head tmp;
	int tmpId = 0;
	
	printWar("remove a url entry, id=%d, dns_id=0x%x, url=%s, stamp=%d\n", entry->id, entry->dns_id, entry->url, entry->timestamp);
	
	list_for_each_entry_safe(pos, n, &entry->url_addr, list) {
		entry->url_addr_len--;
		memset(&pos->addr, 0, sizeof(pos->addr));
		list_move(&pos->list, &url_addr_list);
	}

	memcpy(&tmp, &entry->list, sizeof(struct list_head));
	tmpId = entry->id;
	memset(entry, 0, sizeof(struct url_class_entry));
	memcpy(&entry->list, &tmp, sizeof(struct list_head));
	entry->id = tmpId;
	list_move(&entry->list, &url_class_buffer_list);
	urlclass_entry_used--;

	return 0;
}

int url_class_send(struct url_class_entry *entry)
{
	struct sk_buff *skb = NULL;
    struct nlmsghdr *nlhdr = NULL;
    int skb_len = NLMSG_SPACE(MAX_MSGSIZE);
	struct url_class_carrier data;
	
    if (!url_class_nl || !g_pid) {
        return -1;
    }
	
    skb = alloc_skb(skb_len, GFP_KERNEL);
    if (!skb) {
        printErr("alloc_skb error\n");
		return -1;
    }

	nlhdr = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0))
	NETLINK_CB(skb).portid = 0;
#else
	NETLINK_CB(skb).pid = 0;
#endif
	NETLINK_CB(skb).dst_group = 0;
	
	data.id = entry->id;
	data.info_id = entry->info_id;
	data.cat_map = entry->cat_map;
	strncpy_safe(data.url, entry->url, entry->url_len);
	data.url_len = entry->url_len;
	printWar("send to user, url=%s len=%d id=%d\n", data.url, data.url_len, data.id);
    memcpy(NLMSG_DATA(nlhdr), &data, sizeof(struct url_class_carrier));
    netlink_unicast(url_class_nl, skb, g_pid, MSG_DONTWAIT);

	return 0;
}
int url_class_add_http(const struct sk_buff *skb, unsigned char *msg, int msg_len, const struct _xt_pctl_info *info, unsigned int stamp)
{
	int index = 0;
	struct url_class_entry *pos = NULL;
	struct url_class_entry *entry = NULL;
    const struct iphdr *iph;
#ifdef PCTL_SUPPORT_IPV6
	const struct ipv6hdr *iph6;
	if (4 == ip_hdr(skb)->version) {
		iph = ip_hdr(skb);
	}
	else if(6 == ipv6_hdr(skb)->version) {
		iph6 = ipv6_hdr(skb);
	}
	else {
		return -1;
	}
#else
	iph = ip_hdr(skb);
#endif
	if (unlikely(!msg)) {
        return -1;
    }
	spin_lock_bh(&url_class_nl_lock);
	if (unlikely(list_empty(&url_class_buffer_list))) {
		spin_unlock_bh(&url_class_nl_lock);
		printErr("no room for a new url entry, url=%s\n", msg);
		return -1;
	}
	//exception list
	for (index = 0; url_class_exception_list[index] != NULL; index++) {
		if (_url_strstr(msg, msg + msg_len, url_class_exception_list[index])) {
			spin_unlock_bh(&url_class_nl_lock);
			printWar("exception list match, ignor...%s\n", msg);
			return -1;
		}
	}
	//not a duplicated entry
	list_for_each_entry(pos, &url_class_list, list) {
#ifdef PCTL_SUPPORT_IPV6
		if (!pos->v6 && 4 == ip_hdr(skb)->version) {
			if (!strncmp(pos->url, msg, msg_len) && pos->dev_addr.addr4 == ntohl(iph->saddr)) {
				pos->send_flag = URL_SEND_PENDING;
				pos->timestamp = stamp;
				spin_unlock_bh(&url_class_nl_lock);
				printWar("duplicated entry match, ignor...%s\n", msg);
				return -1;
			}
		}
		else if (pos->v6 && 6 == ipv6_hdr(skb)->version) {
			if (!strncmp(pos->url, msg, msg_len) && ipv6_addr_comp(&iph6->saddr, &pos->dev_addr.addr6)) {
				pos->send_flag = URL_SEND_PENDING;
				pos->timestamp = stamp;
				spin_unlock_bh(&url_class_nl_lock);
				printWar("duplicated entry match, ignor...%s\n", msg);
				return -1;
			}
		}
#else
		if (!strncmp(pos->url, msg, msg_len) && pos->dev_addr.addr4 == ntohl(iph->saddr)) { 
			pos->send_flag = URL_SEND_PENDING;
			pos->timestamp = stamp;
			spin_unlock_bh(&url_class_nl_lock);
			printWar("duplicated entry match, ignor...%s\n", msg);
			return -1;
		}
#endif
	}
	entry = list_first_entry(&url_class_buffer_list, struct url_class_entry, list);
	entry->info_id = info->id;
	entry->url_addr_len = 0;
#ifdef PCTL_SUPPORT_IPV6
	if (4 == ip_hdr(skb)->version) {
		entry->v6 = false;
		entry->dev_addr.addr4 = ntohl(iph->saddr);
	}
	else if(6 == ipv6_hdr(skb)->version) {
		entry->v6 = true;
		memcpy((unsigned char *)&entry->dev_addr.addr6, (unsigned char *)&iph6->saddr, sizeof(struct in6_addr));
	}
#else
	entry->v6 = false;
	entry->dev_addr.addr4 = ntohl(iph->saddr);
#endif
	entry->cat_map = info->cat_map | URL_CAT_SECURITY;
	strncpy_safe(entry->url, msg, msg_len);
	entry->url_len = msg_len;
	entry->send_flag = URL_SEND_PENDING;
	entry->timestamp = stamp;
	INIT_LIST_HEAD(&entry->url_addr);
	printWar("add a url entry, url=%s\n", entry->url);
	list_move(&entry->list, &url_class_list);
	urlclass_entry_used++;
	spin_unlock_bh(&url_class_nl_lock);
	return 0;
}
int url_class_parse_http(const struct sk_buff *skb, const struct _xt_pctl_info *info, int status, unsigned int stamp)
{
    int i = 0;
    int ret = 0;
    const struct iphdr *iph = NULL;
    const struct tcphdr *tcph;
    char domain[128] = {0};
	int domainLen = 0;
    struct url_class_entry *pos = NULL;
    struct url_addr_entry *addr_entry = NULL;
	//struct url_addr_entry *next = NULL;
    unsigned char* http_payload_start;
    unsigned char* http_payload_end;
    unsigned char* host_start = NULL;
    unsigned char* host_end = NULL;
    bool found = false;
    int result = 0;
    //struct block_rule_carrier rule;
#ifdef PCTL_SUPPORT_IPV6
    unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
    const struct ipv6hdr *iph6;
    struct in6_addr daddr6 = {0};
    if (4 == ip_hdr(skb)->version)
    {
        iph = ip_hdr(skb);
        if (iph->protocol != IPPROTO_TCP) {
            return 1;
        }
        tcph = (void *)iph + iph->ihl*4;
        http_payload_start = (unsigned char *)tcph + tcph->doff*4;
        http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
        memcpy((unsigned char *)&daddr6, (unsigned char *)&iph6->daddr, sizeof(struct in6_addr));
        if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
        {
            return 1;
        }
        tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        http_payload_start = (unsigned char *)tcph + tcph->doff*4;
        http_payload_end = (unsigned char *)iph6 + sizeof(struct ipv6hdr) + ntohs(iph6->payload_len) - 1;		
    }
    else
    {
        return ret;
    }
#else
    iph = ip_hdr(skb);
    if (iph->protocol != IPPROTO_TCP) {
        return 1;
    }
    tcph = (void *)iph + iph->ihl*4;
    http_payload_start = (unsigned char *)tcph + tcph->doff*4;
    http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
#endif
    if (http_payload_start < http_payload_end)
    {
        host_start = _url_strstr(http_payload_start, http_payload_end, HOST_STR);
        if (host_start)
        {
            host_start += 8;
            host_end = _url_strstr(host_start, http_payload_end, HOST_END_STR);
            if(!host_end) 
            {
                host_start = NULL;
            }
        }
    if (host_start) {
		domainLen = host_end - host_start;
		if (domainLen > sizeof(domain)) {
			domainLen = sizeof(domain);
		}
		strncpy(domain, host_start, domainLen);
        if(domainLen == 128)
        {
            domain[127] = '\0';
        }
        else
        {
            domain[domainLen] = '\0';
        }
        PCTL_DEBUG("HTTP HOST :%s\n",domain);
		
	}
    }
	if (status != PCTL_STATUS_OK) {
		PCTL_DEBUG("Not a allowed pkt, status = %d. (!0)", status);
        ret = -1;
        goto out;
	}
	
    if (!host_start) {
        /* not http 'get' packet, just let it go */
        PCTL_DEBUG("Not http get packet");
        ret = -1;
        goto out;
    }
	if (!url_class_nl || !g_pid) {
        //if(!g_pid)
            //printk("g_pid is NULL\n");
        ret = -1;
        goto out;
    }
    if (host_start)
    {      
        // white list match
		for (i = 0; i < info->num_wl; ++i)
        {
            if ( _url_strstr(host_start, host_end, info->hosts_wl[i]) )
            {
                PCTL_DEBUG("==== host_wl matched %s ====", info->hosts_wl[i]);
                ret = PCTL_STATUS_OK;
                goto out;
            }
        }
        result = url_class_add_http(skb, domain, domainLen, info, stamp);
		printWar("add dns entry result: %d\n",result);
    }
	wake_up_process(url_class_task);
    
    spin_lock_bh(&url_class_nl_lock);
	list_for_each_entry(pos, &url_class_list, list) {
#ifdef PCTL_SUPPORT_IPV6
        if (!pos->v6 && 4 == ip_hdr(skb)->version) {
            if (pos->dev_addr.addr4 == ntohl(iph->saddr) && (strncmp(pos->url, domain, domainLen) == 0)) { 
                found = true;
                PCTL_DEBUG("match http get : id=%d url=%s src=%pI4 cat=%x url_addr_len:%d\n", 
                pos->id, pos->url, &pos->dev_addr.addr4, pos->cat_id, pos->url_addr_len);
                break;
            }
        }
        else if (pos->v6 && 6 == ipv6_hdr(skb)->version) {
            if (ipv6_addr_comp(&iph6->saddr, &pos->dev_addr.addr6) && (strncmp(pos->url, domain, domainLen) == 0)) {
                found = true;
                PCTL_DEBUG("match http get : id=%d url=%s src=%pI6 cat=%x url_addr_len:%d\n", 
                pos->id, pos->url, &pos->dev_addr.addr6, pos->cat_id, pos->url_addr_len);
                break;
            }
        }
#else
        if (pos->dev_addr.addr4 == ntohl(iph->saddr) && (strncmp(pos->url, domain, domainLen) == 0)) {
            found = true;
            PCTL_DEBUG("match http get : id=%d url=%s src=%pI4 cat=%x url_addr_len:%d\n", 
            pos->id, pos->url, &pos->dev_addr.addr4, pos->cat_id, pos->url_addr_len);
            break;
        }
#endif
	}
	if (!found) {
		printWar("don't match http get...\n");
		//skb_debugFy(skb);
        spin_unlock_bh(&url_class_nl_lock);
		ret = -1;  
        goto out;
	}
    if (list_empty(&url_addr_list)) {
		printErr("No enough room for url_addr\n");
        spin_unlock_bh(&url_class_nl_lock);
		ret = -1;  
        goto out;
	}
    addr_entry = list_first_entry(&url_addr_list, struct url_addr_entry, list);
    if (iph && iph->daddr && !addr_entry->addr.addr4) {
        addr_entry->v6 = false;
        addr_entry->addr.addr4 = ntohl(iph->daddr);
        pos->url_addr_len++;
        list_move(&addr_entry->list, &pos->url_addr);  //add the entry to the url_addr list
        printWar("url=%s	addr=%u.%u.%u.%u\n", pos->url, NIPQUAD(addr_entry->addr.addr4));
    }
#ifdef PCTL_SUPPORT_IPV6
    else if (is_empty_addr6(&addr_entry->addr.addr6) && !is_empty_addr6(&daddr6)) {
            addr_entry->v6 = true;
            memcpy((unsigned char *)&addr_entry->addr.addr6, (unsigned char *)&daddr6, sizeof(struct in6_addr));
            pos->url_addr_len++;
            list_move(&addr_entry->list, &pos->url_addr);  //add the entry to the url_addr list
            printWar("url=%s	addr6=%pI6\n", pos->url, &addr_entry->addr.addr6);
    }
#endif
    /*if (pos->cat_id) {
		//match content filter rule
		if (pos->cat_id & pos->cat_map) {
			//src_ip/dst_ip to block
			memset(&rule, 0, sizeof(struct block_rule_carrier));
			rule.tuple.src_ip = pos->dev_addr;
			list_for_each_entry_safe(addr_entry, next, &pos->url_addr, list) {    //get entry from pos's url_addr list
				rule.tuple.dst_ip = addr_entry->addr;
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING				
				block_rule_add(&rule, pos->cat_id, 0, pos->url, pos->url_len);
#endif
				pos->url_addr_len--;
				addr_entry->addr = 0;
				list_move(&addr_entry->list, &url_addr_list);  //move empty entry back to the url_addr_list 
                printWar("=======http====add block rule : id=%d url=%s src=%pI4 dst=%pI4 cat=%x url_addr_len:%d\n", 
                pos->id, pos->url, &pos->dev_addr, &addr_entry->addr, pos->cat_id, pos->url_addr_len);
			}
		}
		//release the addr_entry
		printWar("delete entry %s \n", pos->url);
		url_class_del(pos);
	}*/
    spin_unlock_bh(&url_class_nl_lock);
	
out:
	return ret;
}
int url_class_parse_https(const struct sk_buff *skb, const struct _xt_pctl_info *info, int status, unsigned int stamp)
{
   const struct iphdr *iph = NULL;
    struct tcphdr *tcph;
    unsigned int i = 0;     
    unsigned char *sslStart = NULL;
    unsigned char *sslEnd = NULL;
    uint8_t *pHandshake = NULL;
    uint8_t * pSNIExt = NULL;
    char domain[128] = {0};
	int domainLen = 0;
    int ret = 0;
    struct url_class_entry *pos = NULL;
    struct url_addr_entry *addr_entry = NULL;
	//struct url_addr_entry *next = NULL;
    bool found = false;
    //struct block_rule_carrier rule;
    int result = 0;
    TLS_EXTENSION SNIExt;/*format is similar with server name extension*/
    int SNIListLen = 0;
    int handledSNILen = 0; 
#ifdef PCTL_SUPPORT_IPV6
    const struct ipv6hdr *iph6;
    struct in6_addr daddr6 = {0};
    unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
    if (4 == ip_hdr(skb)->version)
    {
        iph = ip_hdr(skb);
        tcph = (void *)iph + iph->ihl*4;
        sslStart = (unsigned char *)tcph + tcph->doff * 4;
        sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
        memcpy((unsigned char *)&daddr6, (unsigned char *)&iph6->daddr, sizeof(struct in6_addr));
        if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
        {
            ret = -1;
            goto out;
        }
        tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        sslStart = (unsigned char *)tcph + tcph->doff * 4;
        sslEnd = (unsigned char *)iph6 + sizeof(struct ipv6hdr) + ntohs(iph6->payload_len);
    }
    else
    {
        ret = -1;
        goto out;
    }
#else
    iph = ip_hdr(skb);
    tcph = (void *)iph + iph->ihl*4;
    sslStart = (unsigned char *)tcph + tcph->doff * 4;
    sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
#endif
    if (sslStart >= sslEnd)
    {
        /*UNIDENTIFY*/
        ret = -1;
        goto out;
    }
    if ((!extractHandshakeFromSSL(sslStart, &pHandshake))
        || (*pHandshake != CLIENT_HELLO)
        || (!extractSNIFromClientHello(pHandshake, &pSNIExt)))
    {
        /*UNIDENTIFY*/
        ret = -1;
        goto out;
    }
    SNIListLen = ntohs(*((uint16_t *)pSNIExt));
    pSNIExt += 2;
	if (status != PCTL_STATUS_OK) {
		PCTL_DEBUG("Not a allowed pkt, status = %d. (!0)", status);
        ret = -1;
        goto out;
	}
	if (!url_class_nl || !g_pid) {
        //if(!g_pid)
            //printk("g_pid is NULL\n");
        ret = -1;
        goto out;
    }
    for (handledSNILen = 0; handledSNILen < SNIListLen; )
    {
        SNIExt.type = *pSNIExt++;
        SNIExt.length = ntohs(*((uint16_t *)pSNIExt));
        pSNIExt += 2;
        SNIExt.pData = (uint8_t *)pSNIExt;
        pSNIExt += SNIExt.length;
        /*Does CLENT HELLO  fragment have impact on SNI?*/
        if (pSNIExt > sslEnd)
        {
            /*UNIDENTIFY*/
            ret = -1;
            goto out;
        }
        handledSNILen += (1 + 2 + SNIExt.length);
        if (SNIExt.length > 0) {
	    	domainLen = SNIExt.length;
		    if (domainLen > sizeof(domain)) {
			    domainLen = sizeof(domain);
	    	}
		    strncpy(domain, SNIExt.pData, domainLen);
            if(domainLen == 128)
            {
                domain[127] = '\0';
            }
            else
            {
                domain[domainLen] = '\0';
            }
            PCTL_DEBUG("HTTPS SNI:%s\n",domain);
		
	    }
        if (HOST_NAME == SNIExt.type)
        {
        	for (i = 0; i < info->num_wl; ++i)
			{
				if(_url_strstr(SNIExt.pData,pSNIExt,info->hosts_wl[i]))
				{
					PCTL_DEBUG("==== white list matched %s ====", info->hosts_wl[i]);
					ret = PCTL_STATUS_OK;
					goto out;
				}
            }
			/* add end */
			result = url_class_add_http(skb, domain, domainLen, info, stamp);
		    printWar("add dns entry result: %d\n",result);
        }
    }  
	wake_up_process(url_class_task);
    spin_lock_bh(&url_class_nl_lock);
	list_for_each_entry(pos, &url_class_list, list) {
#ifdef PCTL_SUPPORT_IPV6
        if (!pos->v6 && 4 == ip_hdr(skb)->version) {
            if (pos->dev_addr.addr4 == ntohl(iph->saddr) && (strncmp(pos->url, domain, domainLen) == 0)) { 
                found = true;
                printWar("match https client hello : id=%d url=%s src=%pI4 cat=%x  url_addr_len:%d\n",
                pos->id, pos->url, &pos->dev_addr.addr4, pos->cat_id, pos->url_addr_len);
                break;
            }
        }
        else if (pos->v6 && 6 == ipv6_hdr(skb)->version) {
            if (ipv6_addr_comp(&iph6->saddr, &pos->dev_addr.addr6) && (strncmp(pos->url, domain, domainLen) == 0)) {
                found = true;
                printWar("match http get : id=%d url=%s src=%pI6 cat=%x url_addr_len:%d\n", 
                pos->id, pos->url, &pos->dev_addr.addr6, pos->cat_id, pos->url_addr_len);
                break;
            }
        }
#else
        if (pos->dev_addr.addr4 == ntohl(iph->saddr) && (strncmp(pos->url, domain, domainLen) == 0)) {
            found = true;
            printWar("match https client hello : id=%d url=%s src=%pI4 cat=%x  url_addr_len:%d\n",
            pos->id, pos->url, &pos->dev_addr.addr4, pos->cat_id, pos->url_addr_len);
            break;
        }
#endif
	}
	if (!found) {
		printWar("don't match https client hello...\n");
		//skb_debugFy(skb);
        spin_unlock_bh(&url_class_nl_lock);
		ret = -1;  
        goto out;
	}
    if (list_empty(&url_addr_list)) {
		printErr("No enough room for url_addr\n");
        spin_unlock_bh(&url_class_nl_lock);
		ret = -1;  
        goto out;
	}
    addr_entry = list_first_entry(&url_addr_list, struct url_addr_entry, list);
    if (iph && iph->daddr && !addr_entry->addr.addr4) {
        addr_entry->v6 = false;
        addr_entry->addr.addr4 = ntohl(iph->daddr);
        pos->url_addr_len++;
        list_move(&addr_entry->list, &pos->url_addr);  //add the entry to the url_addr list
        printWar("url=%s	addr=%u.%u.%u.%u\n", pos->url, NIPQUAD(addr_entry->addr.addr4));
    }
#ifdef PCTL_SUPPORT_IPV6
    else if (is_empty_addr6(&addr_entry->addr.addr6) && !is_empty_addr6(&daddr6)) {
            addr_entry->v6 = true;
            memcpy((unsigned char *)&addr_entry->addr.addr6, (unsigned char *)&daddr6, sizeof(struct in6_addr));
            pos->url_addr_len++;
            list_move(&addr_entry->list, &pos->url_addr);  //add the entry to the url_addr list
            printWar("url=%s	addr6=%pI6\n", pos->url, &addr_entry->addr.addr6);
    }
#endif
    spin_unlock_bh(&url_class_nl_lock);
out:
	return ret;
}

int url_class_parse(const struct sk_buff *skb, const struct _xt_pctl_info *info, int status, 
					unsigned int stamp)
{
	int ret = 0;
	int i = 0;
    int i_count = 0;
    dns_header *pDnsHdr = NULL;
    unsigned char *pTmp = NULL;
    unsigned char domain[PCTL_MAX_DNS_SIZE];
    unsigned int pkt_len = 0;
	unsigned int dns_len = 0;
    unsigned int domain_len = 0;
	const struct iphdr *iph;
#ifdef PCTL_SUPPORT_IPV6
	const struct ipv6hdr *iph6;
    unsigned int offset_udp = 0;
    struct udphdr udph_buf;
#endif
    struct udphdr *udph;
	unsigned short *type = NULL;

#ifdef PCTL_SUPPORT_IPV6
    if (4 == ip_hdr(skb)->version) {
        iph = ip_hdr(skb);
        udph = (void *)iph + iph->ihl*4;
    }
	else if(6 == ipv6_hdr(skb)->version) {
        iph6 = ipv6_hdr(skb);
        if (ipv6_find_hdr(skb, &offset_udp, IPPROTO_UDP, NULL, NULL) < 0)
        {
            goto out;
        }
        udph = skb_header_pointer(skb, offset_udp, sizeof(udph_buf), &udph_buf);
	}
    else {
        goto out;
    }
#else
    iph = ip_hdr(skb);
    udph = (void *)iph + iph->ihl*4;
#endif

    dns_len = (unsigned int) ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);

	if (status != PCTL_STATUS_OK) {
		PCTL_DEBUG("Not a allowed pkt, status = %d. (!0)", status);
        ret = -1;
        goto out;
	}
	
    if (dns_len < 0) {
        PCTL_DEBUG("dns_len = %d. (<0)", dns_len);
        ret = -1;
        goto out;
    }

    if (dns_len >= PCTL_MAX_DNS_SIZE) {
        PCTL_DEBUG("dns_len = %d > %d",dns_len, PCTL_MAX_DNS_SIZE);
        ret = -1;
        goto out;
    }

    pDnsHdr = (void *) udph + sizeof(struct udphdr);
    if (0 != (ntohs(pDnsHdr->flags) & 0x8000)) { /* If not request */
        ret = -1;
        goto out;
    }

	if (!url_class_nl || !g_pid) {
        ret = -1;
        goto out;
    }

    pTmp = (unsigned char *)pDnsHdr + sizeof(dns_header);
    for (i_count = 0; i_count < ntohs(pDnsHdr->nQDCount) && pkt_len < dns_len; i_count ++) {
        domain_len = _transDomain2Buf(pTmp, domain, PCTL_MAX_DNS_SIZE - 1);

		//Host Address, ipv4 only.
		type = (unsigned short *)(pTmp + domain_len + 2);

		for (i = 0; i < info->num_wl; ++i) {    
            if (_url_strstr(domain, domain + domain_len, info->hosts_wl[i])) {
                PCTL_DEBUG("==== white list matched %s ====", info->hosts_wl[i]);
                ret = PCTL_STATUS_OK;
                goto out;
            }
        }
		
#ifdef PCTL_SUPPORT_IPV6        
        if (ntohs(*type) == DNS_QUERY_TYPE_A_VALUE || ntohs(*type) == DNS_QUERY_TYPE_AAAA_VALUE) {
            PCTL_DEBUG("i_count=%d, domain=%s, domain_len=%d", i_count, domain, domain_len);
			url_class_add(skb, pDnsHdr->transID, domain, domain_len, info, ntohs(*type), stamp);
        }
#else
        if (ntohs(*type) == DNS_QUERY_TYPE_A_VALUE) {
            PCTL_DEBUG("i_count=%d, domain=%s, domain_len=%d", i_count, domain, domain_len);
			url_class_add(skb, pDnsHdr->transID, domain, domain_len, info, ntohs(*type), stamp);
			//printErr("add dns entry: \n");
			//skb_debugFy(skb);
		}
#endif 

        pkt_len += domain_len + 2 + 2 + 2;//domain extra len(2) + type(2) + class(2)
        pTmp += domain_len + 2 + 2 + 2;//domain extra len(2) + type(2) + class(2)
    }

	wake_up_process(url_class_task);
	
out:
	return ret;
}

static int _url_class_parse_response(const struct sk_buff *skb)
{
	unsigned char *queries = NULL;
	unsigned char *answers = NULL;
	unsigned char domain[PCTL_MAX_DNS_SIZE];
	unsigned int domain_len = 0;
	unsigned char label_len = 0;
	unsigned short data_len = 0;
	unsigned short *type = 0;
	unsigned int *addr = 0;
	int index = 0;
	unsigned char *tmp = NULL;
	unsigned int n = 0;
	bool found = false;
	struct url_class_entry *pos = NULL;
	struct url_addr_entry *entry = NULL;
	struct url_addr_entry *next = NULL;
	struct block_rule_carrier rule;
	const struct iphdr *iph;
	struct udphdr *udph;
#ifdef PCTL_SUPPORT_IPV6
	struct in6_addr *addr6;
	const struct ipv6hdr *iph6;
	unsigned int offset_udp = 0;
	struct udphdr udph_buf;
	struct neigh_table *tbl;
	struct neighbour *neigh;
	struct neighbour *neigh2;
	int addr_tmp;
	struct net_device *dev = NULL;
	int h;
	struct net *net = sock_net(skb->sk);
	struct neigh_hash_table *nht;

	if (4 == ip_hdr(skb)->version) {
		iph = ip_hdr(skb);
		udph = (void *)iph + iph->ihl*4;
	}
	else if(6 == ipv6_hdr(skb)->version) {
		iph6 = ipv6_hdr(skb);
		if (ipv6_find_hdr(skb, &offset_udp, IPPROTO_UDP, NULL, NULL) < 0)
		{
		    return -1;
		}
		udph = skb_header_pointer(skb, offset_udp, sizeof(udph_buf), &udph_buf);	
	}
	else {
		return -1;
	}
#else
	iph = ip_hdr(skb);
	udph = (void *)iph + iph->ihl*4;
#endif

	dns_header *pDnsHdr = (void *) udph + sizeof(struct udphdr);
	unsigned short trans_id = pDnsHdr->transID;
	unsigned int dns_len = (unsigned int) ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);
	unsigned short nQDCount = ntohs(pDnsHdr->nQDCount);
	unsigned short nANCount = ntohs(pDnsHdr->nANCount);
	
	if (!(ntohs(pDnsHdr->flags) & 0x8000)) { /* If not response */
        return -1;
    }
	
	queries = (unsigned char *)pDnsHdr + sizeof(dns_header);
	label_len = *queries;

	if (!nQDCount || !nANCount) {
		//do nothing
		return 0;
	}
	
    while (label_len) {
        if (label_len > 63) {
            //error length
            return -1;
        }
        
        queries += label_len + 1;
        n = queries - (unsigned char *)pDnsHdr;
        if (n > dns_len) {
            //end of the packet
            return -1;
        }
        label_len = *queries;
    }
    
    queries += 1 + 2 + 2;//end(1) + type(2) + class(2)

	spin_lock_bh(&url_class_nl_lock);
	list_for_each_entry(pos, &url_class_list, list) {
		if (pos->dns_id == trans_id) {
			found = true;
			//printWar("get a DNS response, %x\n", trans_id);
			printWar("DNS response match! pos->dns_id=0x%x id=%d url=%s answers=%d pos->cat_id=0x%x pos->cat_map=0x%x\n", pos->dns_id, pos->id, pos->url, nANCount, pos->cat_id, pos->cat_map);
			break;
		}
		/*else {
			printWar("2222 src=%pI4 url=%s ips=%d cat=%x dns=%x trans_id=%x\n", &pos->dev_addr, pos->url, pos->url_addr_len, pos->cat_id, pos->dns_id, trans_id);
		}*/
	}
	if (!found) {
		//printErr("No DNS request record...\n");
		//skb_debugFy(skb);
        spin_unlock_bh(&url_class_nl_lock);
		return -1;
	}
	
	//answer field
	if (nANCount) {
		answers = queries;
		for (index = 0; index < nANCount; index++) {		
			//compression label
			if (*answers & 0xc0) {
				tmp = answers + 2;//name(2)
			}
			//data label
			else {
				domain_len = _transDomain2Buf(answers, domain, PCTL_MAX_DNS_SIZE - 1);
				tmp = answers + domain_len + 2;//domain extra len(2)
			}
			type = (unsigned short *)tmp;
			tmp += 2 + 2 + 4;//type(2) class(2) ttl(4)
			data_len = *(unsigned short *)(tmp);
			tmp += 2;//data length(2)
			
            if (ntohs(*type) == DNS_QUERY_TYPE_A_VALUE) {//Host Address, ipv4 only.
				addr = (unsigned int *)tmp;
				if (list_empty(&url_addr_list)) {
					printErr("No enough room for url_addr, answer=%d...\n", nANCount);
					break;
				}
				
				entry = list_first_entry(&url_addr_list, struct url_addr_entry, list);
#ifdef PCTL_SUPPORT_IPV6
				if (is_empty_addr6(&entry->addr.addr6) && ntohl(*addr))
#else
				if (!entry->addr.addr4 && ntohl(*addr))
#endif
				{
					entry->v6 = false;
					entry->addr.addr4 = ntohl(*addr);
					pos->url_addr_len++;
					list_move(&entry->list, &pos->url_addr);
					printWar("url=%s, addr=%pI4\n", pos->url, addr);
				}
			}
#ifdef PCTL_SUPPORT_IPV6
			else if (ntohs(*type) == DNS_QUERY_TYPE_AAAA_VALUE) {//ipv6
				addr6 = (struct in6_addr *)tmp;
				if (list_empty(&url_addr_list)) {
					printErr("No enough room for url_addr, answer=%d...\n", nANCount);
					break;
				}
				
				entry = list_first_entry(&url_addr_list, struct url_addr_entry, list);
				if (is_empty_addr6(&entry->addr.addr6) && !is_empty_addr6(addr6)) {
					entry->v6 = true;
					memcpy((unsigned char *)&entry->addr.addr6, (unsigned char *)addr6, sizeof(struct in6_addr));
					pos->url_addr_len++;
					list_move(&entry->list, &pos->url_addr);
					printWar("url=%s, addr6=%pI6\n", pos->url, addr6);
				}
			}
#endif
			//next answer
			answers = tmp + ntohs(data_len);
		}
	}

	//if we get a URL cat
	if (pos->cat_id) {
		//match content filter rule
		if (pos->cat_id & pos->cat_map) {
            list_for_each_entry_safe(entry, next, &pos->url_addr, list) {
                memset(&rule, 0, sizeof(struct block_rule_carrier));
                //ipv4 request with A
#ifdef PCTL_SUPPORT_IPV6
                if (!pos->v6 && !entry->v6) {
#endif
                    rule.tuple.v6 = false;
                    rule.tuple.src_ip.addr4 = pos->dev_addr.addr4;
                    rule.tuple.dst_ip.addr4 = entry->addr.addr4;
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING                
                    printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
                            pos->id, pos->dns_id, pos->cat_id, pos->cat_map, pos->url, pos->url_len);
                    block_rule_add(&rule, pos->cat_id, 0, pos->url, pos->url_len);
#endif
                
#ifdef PCTL_SUPPORT_IPV6
                }
                //ipv6 request with AAAA
                else if (pos->v6 && entry->v6) {
                    rule.tuple.v6 = true;
                    memcpy((unsigned char *)&rule.tuple.src_ip.addr6, (unsigned char *)&pos->dev_addr.addr6, sizeof(struct in6_addr));
                    memcpy((unsigned char *)&rule.tuple.dst_ip.addr6, (unsigned char *)&entry->addr.addr6, sizeof(struct in6_addr));
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING                
                    printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
                            pos->id, pos->dns_id, pos->cat_id, pos->cat_map, pos->url, pos->url_len);
                    block_rule_add(&rule, pos->cat_id, 0, pos->url, pos->url_len);
#endif
                }
                //ipv4 request with AAAA
                else if (!pos->v6 && entry->v6) {
                    rule.tuple.v6 = true;
                    memcpy((unsigned char *)&rule.tuple.dst_ip.addr6, (unsigned char *)&entry->addr.addr6, sizeof(struct in6_addr));
                    
                    tbl = neigh_get_table(AF_INET); 
                    dev = dev_get_by_name(net, "br-lan");
                    addr_tmp = htonl(pos->dev_addr.addr4);
                    neigh = neigh_lookup(tbl, &addr_tmp, dev);
                    if (neigh == NULL) {
                        printWar("Cannot find the neigh which ip is:%pI4\n", &pos->dev_addr.addr4);
                        goto skip;
                    }
                    printWar("MAC:%x:%x:%x:%x:%x:%x\n", neigh->ha[0],neigh->ha[1],neigh->ha[2],neigh->ha[3],neigh->ha[4],neigh->ha[5]);

                    rcu_read_lock_bh();
                    tbl = neigh_get_table(AF_INET6);
                    nht = rcu_dereference_bh(tbl->nht);
                    for (h = 0; h < (1 << nht->hash_shift); h++) {
                        for (neigh2 = rcu_dereference_bh(nht->hash_buckets[h]); neigh2 != NULL; neigh2 = rcu_dereference_bh(neigh2->next)) {
                            if (!net_eq(dev_net(neigh2->dev), net))
                                continue;
                            //if neighbour state is NUD_INCOMPLETE|NUD_FAILED|NUD_NONE,skip
                            if (!(neigh2->nud_state & NUD_VALID))
                                continue;
                            if (neigh->ha[0] == neigh2->ha[0] && neigh->ha[1] == neigh2->ha[1] && neigh->ha[2] == neigh2->ha[2] && 
                                neigh->ha[3] == neigh2->ha[3] && neigh->ha[4] == neigh2->ha[4] && neigh->ha[5] == neigh2->ha[5]) {
                                printWar("Find neigh_table,src_ip address is:%pI6\n", (struct in6_addr *)neigh2->primary_key);
                                if ((neigh2->primary_key[0] & 0xf0) != 0xf0) {
                                    memcpy((unsigned char *)&rule.tuple.src_ip.addr6, (unsigned char *)neigh2->primary_key, sizeof(struct in6_addr));
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING
                                    printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
                                            pos->id, pos->dns_id, pos->cat_id, pos->cat_map, pos->url, pos->url_len);
                                    block_rule_add(&rule, pos->cat_id, 0, pos->url, pos->url_len);
#endif
                                }
                            }
                        }
                    }
                    rcu_read_unlock_bh();
                }
                //ipv6 request with A
                else if (pos->v6 && !entry->v6) {
                    rule.tuple.v6 = false;
                    rule.tuple.dst_ip.addr4 = entry->addr.addr4;
                    
                    tbl = neigh_get_table(AF_INET6);    
                    dev = dev_get_by_name(net, "br-lan");
                    neigh = neigh_lookup(tbl, &pos->dev_addr.addr6, dev);
                    if (neigh == NULL) {
                        printWar("Cannot find the neigh which ip is:%pI6\n", &pos->dev_addr.addr6);
                        goto skip;
                    }
                    printWar("MAC:%x:%x:%x:%x:%x:%x\n", neigh->ha[0],neigh->ha[1],neigh->ha[2],neigh->ha[3],neigh->ha[4],neigh->ha[5]);
                    
                    rcu_read_lock_bh();
                    tbl = neigh_get_table(AF_INET);
                    nht = rcu_dereference_bh(tbl->nht);
                    for (h = 0; h < (1 << nht->hash_shift); h++) {
                        for (neigh2 = rcu_dereference_bh(nht->hash_buckets[h]); neigh2 != NULL; neigh2 = rcu_dereference_bh(neigh2->next)) {
                            if (!net_eq(dev_net(neigh2->dev), net))
                                continue;
                            //if neighbour state is NUD_INCOMPLETE|NUD_FAILED|NUD_NONE,skip
                            if (!(neigh2->nud_state & NUD_VALID))
                                continue;
                            if (neigh->ha[0] == neigh2->ha[0] && neigh->ha[1] == neigh2->ha[1] && neigh->ha[2] == neigh2->ha[2] && 
                                neigh->ha[3] == neigh2->ha[3] && neigh->ha[4] == neigh2->ha[4] && neigh->ha[5] == neigh2->ha[5]) {
                                printWar("Find neigh_table,src_ip address is:%pI4\n", (int *)neigh2->primary_key);
                                rule.tuple.src_ip.addr4 = ntohl(*((int *)neigh2->primary_key));
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING                
                                printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
                                        pos->id, pos->dns_id, pos->cat_id, pos->cat_map, pos->url, pos->url_len);
                                block_rule_add(&rule, pos->cat_id, 0, pos->url, pos->url_len);
#endif
                            }
                        }
                    }
                    rcu_read_unlock_bh();
                }
skip:
#endif
                pos->url_addr_len--;
                memset(&entry->addr, 0, sizeof(entry->addr));
                list_move(&entry->list, &url_addr_list);
            }
            printWar("match children rule from DNS response\n");
        }
        //release the entry
        printWar("delete entry %s from DNS response\n", pos->url);
		url_class_del(pos);
	}
	spin_unlock_bh(&url_class_nl_lock);

	return 0;
}

#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC	
static int _url_class_cat(void *data, int size)
{
	int tmp_id = 0;
	struct block_rule_carrier rule;
	struct url_class_carrier *url_msg = (struct url_class_carrier *)data;
	struct url_addr_entry *entry = NULL;
	struct url_addr_entry *n = NULL;

	struct url_class_entry *pos = NULL;	
	struct url_class_entry *pEntry = NULL;
#ifdef PCTL_SUPPORT_IPV6
	struct neigh_table *tbl;
	struct neighbour *neigh;
	struct neighbour *neigh2;
	struct net_device *dev = NULL;
	int i, h;
	int addr_tmp;
	struct net *net = &init_net;
	struct neigh_hash_table *nht;
#endif

    if(!url_msg)
    {
        //printk("url_msg is null!\n");
        return -1;
    }
    else if(!url_msg->id)
    {
        //printk("url_msg's cat_id or id is null!\n");
        return -1;
    }
	spin_lock_bh(&url_class_nl_lock);
	tmp_id = url_msg->id;

	list_for_each_entry(pos, &url_class_list, list) {
		if ( (tmp_id == pos->id) && (pos->url_len > 0) && !strncmp(url_msg->url, pos->url, pos->url_len) ) { 
			pEntry = pos;
		}
	}

	if (NULL == pEntry)
	{
		list_for_each_entry(pos, &url_class_buffer_list, list) {
			if ( (tmp_id == pos->id) && (pos->url_len > 0) && !strncmp(url_msg->url, pos->url, pos->url_len) ) { 
				printErr("Why in url_class_buffer_list ??? url_msg->url=%s \n", url_msg->url);
				pEntry = pos;
			}
		}		
	}
	
	if (NULL == pEntry)
	{
		spin_unlock_bh(&url_class_nl_lock);
		return 0;
	}
	printWar("id=%d, cat_id=0x%x, url=%s, url_len=%d \n", url_msg->id, url_msg->cat_id, url_msg->url, url_msg->url_len);
	if (!url_msg->cat_id)
	{
		goto del_entry;
	}

	pEntry->cat_id = url_msg->cat_id;

	//if we get the DNS response
	if (!pEntry->url_addr_len) {
		spin_unlock_bh(&url_class_nl_lock);
		return 0;
	}

	//match content filter rule
	if (pEntry->cat_id & pEntry->cat_map) {
		list_for_each_entry_safe(entry, n, &pEntry->url_addr, list) {	
			memset(&rule, 0, sizeof(struct block_rule_carrier));
			//ipv4 request with A
#ifdef PCTL_SUPPORT_IPV6
			if (!pEntry->v6 && !entry->v6) {
#endif
				rule.tuple.v6 = false;
				rule.tuple.src_ip.addr4 = pEntry->dev_addr.addr4;
				rule.tuple.dst_ip.addr4 = entry->addr.addr4;
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING				
				printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
						pEntry->id, pEntry->dns_id, pEntry->cat_id, pEntry->cat_map, pEntry->url, pEntry->url_len);
				block_rule_add(&rule, pEntry->cat_id, 0, pEntry->url, pEntry->url_len);
#endif

#ifdef PCTL_SUPPORT_IPV6
			}
			//ipv6 request with AAAA
			else if (pEntry->v6 && entry->v6) {
				rule.tuple.v6 = true;
				memcpy((unsigned char *)&rule.tuple.src_ip.addr6, (unsigned char *)&pEntry->dev_addr.addr6, sizeof(struct in6_addr));
				memcpy((unsigned char *)&rule.tuple.dst_ip.addr6, (unsigned char *)&entry->addr.addr6, sizeof(struct in6_addr));
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING				
				printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
						pEntry->id, pEntry->dns_id, pEntry->cat_id, pEntry->cat_map, pEntry->url, pEntry->url_len);
				block_rule_add(&rule, pEntry->cat_id, 0, pEntry->url, pEntry->url_len);
#endif
			}
			//ipv4 request with AAAA
			else if (!pEntry->v6 && entry->v6) {
				rule.tuple.v6 = true;
				memcpy((unsigned char *)&rule.tuple.dst_ip.addr6, (unsigned char *)&entry->addr.addr6, sizeof(struct in6_addr));
				
				tbl = neigh_get_table(AF_INET);	
				dev = dev_get_by_name(net, "br-lan");
				addr_tmp = htonl(pEntry->dev_addr.addr4);
				neigh = neigh_lookup(tbl, &addr_tmp, dev);
				if (neigh == NULL) {
					printWar("Cannot find the neigh which ip is:%pI4\n", &pEntry->dev_addr.addr4);
					goto skip;;
				}
				printWar("MAC:%x:%x:%x:%x:%x:%x\n", neigh->ha[0],neigh->ha[1],neigh->ha[2],neigh->ha[3],neigh->ha[4],neigh->ha[5]);

				rcu_read_lock_bh();
				tbl = neigh_get_table(AF_INET6);
				nht = rcu_dereference_bh(tbl->nht);
				for (h = 0; h < (1 << nht->hash_shift); h++) {
					for (neigh2 = rcu_dereference_bh(nht->hash_buckets[h]); neigh2 != NULL; neigh2 = rcu_dereference_bh(neigh2->next)) {
						if (!net_eq(dev_net(neigh2->dev), net))
							continue;
						//if neighbour state is NUD_INCOMPLETE|NUD_FAILED|NUD_NONE,skip
						if (!(neigh2->nud_state & NUD_VALID))
							continue;
						if (neigh->ha[0] == neigh2->ha[0] && neigh->ha[1] == neigh2->ha[1] && neigh->ha[2] == neigh2->ha[2] && 
							neigh->ha[3] == neigh2->ha[3] && neigh->ha[4] == neigh2->ha[4] && neigh->ha[5] == neigh2->ha[5]) {
							printWar("Find neigh_table,src_ip address is:%pI6\n", (struct in6_addr *)neigh2->primary_key);
							if ((neigh2->primary_key[0] & 0xf0) != 0xf0) {
								memcpy((unsigned char *)&rule.tuple.src_ip.addr6, (unsigned char *)neigh2->primary_key, sizeof(struct in6_addr));
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING
								printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
										pEntry->id, pEntry->dns_id, pEntry->cat_id, pEntry->cat_map, pEntry->url, pEntry->url_len);
								block_rule_add(&rule, pEntry->cat_id, 0, pEntry->url, pEntry->url_len);
#endif
							}
						}
					}
				}
				rcu_read_unlock_bh();
			}
			//ipv6 request with A
			else if (pEntry->v6 && !entry->v6) {
				rule.tuple.v6 = false;
				rule.tuple.dst_ip.addr4 = entry->addr.addr4;
				
				tbl = neigh_get_table(AF_INET6);				
				dev = dev_get_by_name(net, "br-lan");
				neigh = neigh_lookup(tbl, &pEntry->dev_addr.addr6, dev);
				if (neigh == NULL) {
					printWar("Cannot find the neigh which ip is:%pI6\n", &pEntry->dev_addr.addr6);
					goto skip;
				}
				printWar("MAC:%x:%x:%x:%x:%x:%x\n", neigh->ha[0],neigh->ha[1],neigh->ha[2],neigh->ha[3],neigh->ha[4],neigh->ha[5]);

				rcu_read_lock_bh();
				tbl = neigh_get_table(AF_INET);
				nht = rcu_dereference_bh(tbl->nht);
				for (h = 0; h < (1 << nht->hash_shift); h++) {
					for (neigh2 = rcu_dereference_bh(nht->hash_buckets[h]); neigh2 != NULL; neigh2 = rcu_dereference_bh(neigh2->next)) {
						if (!net_eq(dev_net(neigh2->dev), net))
							continue;
						//if neighbour state is NUD_INCOMPLETE|NUD_FAILED|NUD_NONE,skip
						if (!(neigh2->nud_state & NUD_VALID))
							continue;
						//printWar("MAC2:%x:%x:%x:%x:%x:%x\n", neigh->ha[0],neigh->ha[1],neigh->ha[2],neigh->ha[3],neigh->ha[4],neigh->ha[5]);
						if (neigh->ha[0] == neigh2->ha[0] && neigh->ha[1] == neigh2->ha[1] && neigh->ha[2] == neigh2->ha[2] && 
							neigh->ha[3] == neigh2->ha[3] && neigh->ha[4] == neigh2->ha[4] && neigh->ha[5] == neigh2->ha[5]) {
							printWar("Find neigh_table,src_ip address is:%pI4\n", (int *)neigh2->primary_key);
							rule.tuple.src_ip.addr4 = ntohl(*((int *)neigh2->primary_key));
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING				
							printWar("id=%d, dns_id=0x%x,  cat_id=0x%x, cat_map=0x%x, url=%s, url_len=%d \n", 
									pEntry->id, pEntry->dns_id, pEntry->cat_id, pEntry->cat_map, pEntry->url, pEntry->url_len);
							block_rule_add(&rule, pEntry->cat_id, 0, pEntry->url, pEntry->url_len);
#endif
						}
					}
				}
				rcu_read_unlock_bh();
			}
skip:
#endif
			pEntry->url_addr_len--;
			memset(&entry->addr, 0, sizeof(entry->addr));
			list_move(&entry->list, &url_addr_list);
		}
	}
	printWar("match children rule from AUC response\n");

del_entry:
	//release the entry
	printWar("delete entry %s from AUC response\n", pEntry->url);
	url_class_del(pEntry);
	
	spin_unlock_bh(&url_class_nl_lock);

	return 0;
}
#else
static int _url_class_cat(void *data, int size)
{
	int tmp_id = 0;
	struct block_rule_carrier rule;
	struct url_class_carrier *url_msg = (struct url_class_carrier *)data;
	struct url_addr_entry *entry = NULL;
	struct url_addr_entry *n = NULL;
    if(!url_msg)
    {
        //printk("url_msg is null!\n");
        return -1;
    }
    //else if(!url_msg->cat_id || !url_msg->id)
    else if (!url_msg->id)
    {
        //printk("url_msg's cat_id or id is null!\n");
        return -1;
    }

	spin_lock_bh(&url_class_nl_lock);
	tmp_id = url_msg->id;
	if ( (url_class_array[tmp_id].url_len <= 0) || strncmp(url_msg->url, url_class_array[tmp_id].url, url_class_array[tmp_id].url_len) )
	{
		printWar("The old entry maybe expired, old url = %s\n", url_msg->url);
		spin_unlock_bh(&url_class_nl_lock);
		return 0;
	}

	if (!url_msg->cat_id)
	{
		goto del_entry;
	}

	url_class_array[tmp_id].cat_id = url_msg->cat_id;

	//if we get the DNS response
	if (!url_class_array[tmp_id].url_addr_len) {
		spin_unlock_bh(&url_class_nl_lock);
		return 0;
	}

	//match content filter rule
	if (url_class_array[tmp_id].cat_id & url_class_array[tmp_id].cat_map) {
		//src_ip/dst_ip to block
		memset(&rule, 0, sizeof(struct block_rule_carrier));
		rule.tuple.src_ip = url_class_array[tmp_id].dev_addr;
		list_for_each_entry_safe(entry, n, &url_class_array[tmp_id].url_addr, list) {
			rule.tuple.dst_ip = entry->addr;
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING				
			block_rule_add(&rule, url_class_array[tmp_id].cat_id, 0, url_class_array[tmp_id].url, url_class_array[tmp_id].url_len);
#endif
			url_class_array[tmp_id].url_addr_len--;
			memset(pos->addr, 0, sizeof(pos->addr));
			list_move(&entry->list, &url_addr_list);
		}
	}
	printWar("match children rule from AUC response\n");

del_entry:
	//release the entry
	printWar("delete entry %s from AUC response\n", url_class_array[tmp_id].url);
	url_class_del(&url_class_array[tmp_id]);

	spin_unlock_bh(&url_class_nl_lock);

	return 0;
}
#endif

static url_class_error _url_class_log(void *data, int size)
{
	pctl_owner *pOwner = NULL;
	pctl_log *pLog = NULL;
	int id = 0;
	int len = 0;
	struct url_log_carrier url_log;
    u_int64_t traffic = 0;

	id = *(int *)data;
    pOwner = owners + id;

	/* changed by CCy, move it here, to aviod mutiple calls of lock, to avoid dead lock */
    log_update_history(id, GET_TIMESTAMP());
	
	read_lock_bh(&pOwner->lock);
	len = sizeof(pOwner->today_stats.total);
	memcpy(data, &pOwner->today_stats.total, len);
	traffic = traffic_get(&pOwner->traffic);
    memcpy((unsigned char *)data + len, &traffic, sizeof(traffic));
    len += sizeof(traffic);

	if (size < len + sizeof(url_log) * pOwner->log_len) {
		printErr("URL log:fragment blocking log sent to user...\n");
		read_unlock_bh(&pOwner->lock);
		return URL_CLASS_INTERNAL;
	}
	
	memcpy((unsigned char *)data + len, &pOwner->log_len, sizeof(pOwner->log_len));
	len += sizeof(pOwner->log_len);
	
    list_for_each_entry(pLog, &pOwner->log_list, log_node) {
		strcpy(url_log.url, pLog->entry.host);
		url_log.count = pLog->entry.stats.count;
		if (!pLog->entry.v6) {
			url_log.v6 = false;
			url_log.ip.addr4 = pLog->entry.ip.addr4;
		}else {
			url_log.v6 = true;
			memcpy((unsigned char *)&url_log.ip.addr6, (unsigned char *)&pLog->entry.ip.addr6, sizeof(struct in6_addr));
		}
		url_log.time = pLog->entry.stats.total;
        url_log.traffic = traffic_get(&pLog->entry.traffic);
		memcpy((unsigned char *)data + len, &url_log, sizeof(url_log));
		len += sizeof(url_log);
    }
	read_unlock_bh(&pOwner->lock);
	
	return URL_CLASS_DONE;
}

static url_class_error _url_class_log_history(void *data, int size)
{
	pctl_owner *pOwner = NULL;
	pctl_history *pDay = NULL;
	int id = 0;
	int len = 0;
	int index = 0;
	struct url_log_carrier url_log;
	int idx = 0;
	unsigned int def = 0;

	/* changed by CCy, move log_update_history() forward, to aviod mutiple calls of lock, to avoid dead lock */
	id = *(int *)data;
    log_update_history(id, GET_TIMESTAMP());

    pOwner = owners + id;
	read_lock_bh(&pOwner->lock);
	idx = ((pOwner->day_idx + PCTL_HISTORY_DAY_NUM - 1) % PCTL_HISTORY_DAY_NUM);
	pDay = pOwner->day[idx];

	if (!pDay) {
		len = sizeof(unsigned int);
		memcpy(data, &def, len);
		memcpy((unsigned char *)data + len, &def, sizeof(unsigned int));
	
		read_unlock_bh(&pOwner->lock);
		return URL_CLASS_DONE;
	}
	
	len = sizeof(pDay->day_stats.total);
	memcpy(data, &pDay->day_stats.total, len);
	
	if (size < len + sizeof(url_log) * pDay->num) {
		printErr("URL log:fragment blocking log sent to user...\n");
		read_unlock_bh(&pOwner->lock);
		return URL_CLASS_INTERNAL;
	}
	
	memcpy((unsigned char *)data + len, &pDay->num, sizeof(pDay->num));
	len += sizeof(pDay->num);
	
	for (index = 0; index < pDay->num; index++) {
		strcpy(url_log.url, pDay->log_entry[index].host);
		url_log.count = pDay->log_entry[index].stats.count;
		//url_log.ip = pDay->log_entry[index].ip;
		if (!pDay->log_entry[index].v6) {
			url_log.v6 = false;
			url_log.ip.addr4 = pDay->log_entry[index].ip.addr4;
		}else {
			url_log.v6 = true;
			memcpy((unsigned char *)&url_log.ip.addr6, (unsigned char *)&pDay->log_entry[index].ip.addr6, sizeof(struct in6_addr));
		}
		url_log.time = pDay->log_entry[index].stats.total;
        url_log.traffic = traffic_get(&pDay->log_entry[index].traffic);
		memcpy((unsigned char *)data + len, &url_log, sizeof(url_log));
		len += sizeof(url_log);
    }
	read_unlock_bh(&pOwner->lock);
	
	return URL_CLASS_DONE;
}

static int _url_class_process(void *data, struct nlmsghdr *nlhdr, int size)
{
	int ret = 0;
	
	switch (nlhdr->nlmsg_type)
	{
	case URL_CLASS_REQ_HELLO:
		spin_lock_bh(&url_class_nl_lock);
		g_pid = nlhdr->nlmsg_pid;

		//clean buffer
#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC	
		_url_class_list_clean();
		_url_addr_list_clean();
#else
		_url_class_list_init();
		_url_addr_list_init();
#endif		
		spin_unlock_bh(&url_class_nl_lock);
	
		break;
	case URL_CLASS_REQ_CAT:
		ret = _url_class_cat(data, size);
		
		break;
	case URL_CLASS_REQ_LOG:
		ret = _url_class_log(data, size);
		
		break;
	case URL_CLASS_REQ_LOG_HISTORY:
		ret = _url_class_log_history(data, size);
		
		break;
	case URL_CLASS_REQ_BYE:
		spin_lock_bh(&url_class_nl_lock);
		g_pid = 0;
		spin_unlock_bh(&url_class_nl_lock);
	
		break;
	default:
		printErr("Unknown type");
		ret = -1;
		
		break;
	}

	return ret;
}

void url_class_recv(struct sk_buff *skb)
{
	struct nlmsghdr *nlhdr;
	int ret = 0;

	printWar("msg from user\n");
	
	if (skb->len < sizeof(struct nlmsghdr)) {
		printErr("Length of skb invalid: %d", skb->len);
		ret = -1;
		goto out;
	}

	nlhdr = nlmsg_hdr(skb);
	if (!nlhdr) {
		printErr("Invalid skb, data is NULL");
		ret = -1;
		goto out;
	}
	
	if (nlhdr->nlmsg_pid < 0 || nlhdr->nlmsg_len < sizeof(struct nlmsghdr) || nlhdr->nlmsg_len > skb->len) {
		printErr("Length of nlhdr invalid: %d", nlhdr->nlmsg_len);
		ret = -1;
		goto out;
	}

	if (nlhdr->nlmsg_type < URL_CLASS_REQ_VOID || nlhdr->nlmsg_type >= URL_CLASS_REQ_END) {
		printErr("Invalid nlmsg type: %d", nlhdr->nlmsg_type);
		ret = -1;
		goto out;
	}

	ret = _url_class_process((void *)NLMSG_DATA(nlhdr), nlhdr, (int)(nlhdr->nlmsg_len - NLMSG_LENGTH(0)));
	
out:
	if (nlhdr->nlmsg_flags & NLM_F_ACK)
		xtables_ack(skb, nlhdr, ret);
	
	return;
}

static int url_class_send_thread(void *arg)
{
	struct url_class_entry *pos = NULL;
	unsigned long end = 0;

	set_current_state(TASK_INTERRUPTIBLE);
	
	while (!kthread_should_stop()) {
		__set_current_state(TASK_RUNNING);
		end = jiffies + MAX_URL_SEND_TIME;
		pos = NULL;
		spin_lock_bh(&url_class_nl_lock);
		list_for_each_entry(pos, &url_class_list, list) {
			if (pos->send_flag == URL_SEND_PENDING) {
				url_class_send(pos);
				pos->send_flag = URL_SEND_DONE;
			}

			/*if (time_after(jiffies, end)) {
				break;
			}*/
		}
		spin_unlock_bh(&url_class_nl_lock);
		set_current_state(TASK_INTERRUPTIBLE);

		schedule();
	}

	__set_current_state(TASK_RUNNING);
	
	return 0;
}
/* add end */

static int get_mac_from_neigh(const struct nf_conntrack_man *src, char mac[ETH_ALEN])
{
    struct net_device *dev;
    struct neighbour *neigh;
    struct neigh_table *tbl = neigh_get_table(src->l3num);
    int ret = 0;

    // get the network device to use
    dev = dev_get_by_name(&init_net, "br-lan");
    if (dev == NULL) {
        printErr("neigh_lookup: device not found\n");
        ret = -ENODEV;
        goto out;
    }

    // look up the neighbor entry
    neigh = neigh_lookup(tbl, &src->u3, dev);
    if (neigh == NULL) {
        PCTL_DEBUG("neigh_lookup: neighbor not found\n");
        ret = -ENOENT;
        goto out_dev_put;
    }

    // print the MAC address of the neighbor
    if (src->l3num == AF_INET) {
        PCTL_DEBUG("neigh_lookup: MAC address of neighbor %pI4 is %pM\n",
            &src->u3, neigh->ha);
    } else if (src->l3num == AF_INET6) {
        PCTL_DEBUG("neigh_lookup: MAC address of neighbor %pI6 is %pM\n",
            &src->u3, neigh->ha);
    }

    memcpy(mac, neigh->ha, ETH_ALEN);
    // release the neighbor entry
    neigh_release(neigh);

out_dev_put:
    if (dev != NULL)
        dev_put(dev);

out:
    return ret;
}

static struct hlist_nulls_node *ct_start(u_int32_t *pbucket)
	__acquires(RCU)
{
	struct net *net = &init_net;
	struct hlist_nulls_node *n;
	
	rcu_read_lock();	/* RCU  LOCK */
	
	/* Find a bucket that is not "nulls", return it's first */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	for (*pbucket = 0; *pbucket < nf_conntrack_htable_size; (*pbucket)++) {
		n = rcu_dereference(hlist_nulls_first_rcu(&nf_conntrack_hash[*pbucket]));
#else
	for (*pbucket = 0; *pbucket < net->ct.htable_size; (*pbucket)++) {
		n = rcu_dereference(hlist_nulls_first_rcu(&net->ct.hash[*pbucket]));
#endif
		if (!is_a_nulls(n))
			return n;
	}
	return NULL;
}

static struct hlist_nulls_node *ct_next(u_int32_t *pbucket,
				      struct hlist_nulls_node *head)
{
	struct net *net = &init_net;

	head = rcu_dereference(hlist_nulls_next_rcu(head));
	while (is_a_nulls(head)) {
		if (likely(get_nulls_value(head) == *pbucket)) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
			if (++(*pbucket) >= nf_conntrack_htable_size)
#else
			if (++(*pbucket) >= net->ct.htable_size)
#endif
				return NULL;
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
		head = rcu_dereference(
				hlist_nulls_first_rcu(
					&nf_conntrack_hash[*pbucket]));
#else
		head = rcu_dereference(
				hlist_nulls_first_rcu(
					&net->ct.hash[*pbucket]));
#endif
	}
	return head;
}

static void ct_stop(void)
	__releases(RCU)
{
	rcu_read_unlock();	/* RCU  UNLOCK */
}

struct stat_counter {
	u_int64_t packets;
	u_int64_t bytes;
};

/* nf_conntrack flow account, get packets and bytes of a connection */
static unsigned int ct_get_acct(const struct nf_conn *ct, int dir, struct stat_counter *c)
{
	struct nf_conn_acct *acct;
	struct nf_conn_counter *counter;

	acct = nf_conn_acct_find(ct);
	if (!acct)
		return 0;

	counter = acct->counter;
	c->packets = (unsigned long long)atomic64_read(&counter[dir].packets);
	c->bytes = (unsigned long long)atomic64_read(&counter[dir].bytes) + c->packets * ETH_HLEN;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
		BlogFcStats_t fast_stats;
		
		fast_stats.rx_packets = 0;
		fast_stats.rx_bytes = 0;
		

		if(blog_ct_get_stats(ct, ct->blog_key[dir], dir, &fast_stats) == 0)
		{
			  counter[dir].ts = fast_stats.pollTS_ms;
		}
		
		c->packets += counter[dir].cum_fast_pkts + fast_stats.rx_packets;
		c->bytes += counter[dir].cum_fast_bytes + fast_stats.rx_bytes;

		//printWar("cum_fast: %lld %lld\n", counter[dir].cum_fast_pkts, counter[dir].cum_fast_bytes);
		//printWar("fast_stats: %lld %lld\n", fast_stats.rx_packets, fast_stats.rx_bytes);
		//printWar("result all: %lld %lld\n", c->packets, c->bytes);
#else
		BlogFcStats_t fast_stats = {0};		

        blog_ct_get_stats(ct, ct->bcm_ext.blog_key[dir], dir, &fast_stats);
        c->packets  += fast_stats.rx_packets;		
		c->bytes    += fast_stats.rx_bytes;
		//printWar("fast_stats: %lld %lld\n", fast_stats.rx_packets, fast_stats.rx_bytes);
		//printWar("result all: %lld %lld\n", c->packets, c->bytes);
#endif

    }
#endif

	return 0;
}

static int ct_parse(void *p, struct nf_conn **ct, u_int64_t *bytes)
{
	struct nf_conntrack_tuple_hash *hash = p;
	struct nf_conn *_ct = nf_ct_tuplehash_to_ctrack(hash);
    struct stat_counter ori = {.packets = 0, .bytes = 0}, rep = {.packets = 0, .bytes = 0};

	/* If a conntrack entry is dying, ignore it. */
	if (unlikely(nf_ct_is_dying(_ct))) {
		return -1;
	}

    /* only original direction is considered */
	if (NF_CT_DIRECTION(hash)) {
		return -1;
	}

    ct_get_acct(_ct, IP_CT_DIR_ORIGINAL, &ori);
    ct_get_acct(_ct, IP_CT_DIR_REPLY, &rep);
    *ct = _ct;
    *bytes = ori.bytes + rep.bytes;

	return 0;
}

static inline bool is_dst_addr_match(const struct nf_conntrack_man *src, const union nf_inet_addr *dst, const pctl_log_entry *entry)
{
    if (src->l3num == AF_INET && !entry->v6) {
        PCTL_DEBUG("%pI4, %pI4\n", dst, &entry->ip.addr4);
        if (ntohl(dst->in.s_addr) == entry->ip.addr4) {
            return true;
        }
    } else if (src->l3num == AF_INET6 && entry->v6) {
        PCTL_DEBUG("%pI6, %pI6\n", dst, &entry->ip.addr6);
        if (!memcmp(&dst->in6, &entry->ip.addr6, sizeof(struct in6_addr))) {
            return true;
        }
    }

    return false;
}

static int traffic_update(struct nf_conn *ct, u_int64_t traffic, bool is_current)
{
    int i = 0;
    int ret = -1;
    client_info *client;
    pctl_log *log;
    struct nf_conntrack_man *src = &(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src);
    union nf_inet_addr *dst = &(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3);
    unsigned char mac[ETH_ALEN] = { 0 };

    if (get_mac_from_neigh(src, mac)) {
        goto out;
    }

    for (i = 0; i < PCTL_OWNER_NUM; i++) {
        if (owners[i].status != PCTL_STATUS_OK) {
            continue;
        }
        list_for_each_entry(client, &owners[i].client_list, list) {
            if (!memcmp(mac, client->mac, ETH_ALEN)) {
                write_lock_bh(&owners[i].traffic.lock);
                if (is_current) {
                    owners[i].traffic.curr += traffic;
                    PCTL_DEBUG("id=%d, traffic=%llu\n", owners[i].id, owners[i].traffic.curr);
                } else {
                    owners[i].traffic.old += traffic;
                    PCTL_DEBUG("id=%d, traffic=%llu\n", owners[i].id, owners[i].traffic.old);
                }
                write_unlock_bh(&owners[i].traffic.lock);
                /* FIXME: This process is currently time-consuming and could benefit from optimization */

                list_for_each_entry(log, &owners[i].log_list, log_node) {
                    if (is_dst_addr_match(src, dst, &log->entry)){
                        write_lock_bh(&log->entry.traffic.lock);
                        if (is_current) {
                            log->entry.traffic.curr += traffic;
                            PCTL_DEBUG("host=%s, traffic=%llu\n", log->entry.host, log->entry.traffic.curr);
                        } else {
                            log->entry.traffic.old += traffic;
                            PCTL_DEBUG("host=%s, traffic=%llu\n", log->entry.host, log->entry.traffic.old);
                        }
                        write_unlock_bh(&log->entry.traffic.lock);
                        break;
                    }
                }
                ret = 0;
                goto out;
            }
        }
    }

out:
    return ret;
}

static inline void traffic_reset_current(traffic_stat *traffic)
{
    write_lock_bh(&traffic->lock);
    traffic->curr = 0;
    write_unlock_bh(&traffic->lock);
}

/*
 * Traverse all ct entries and update traffic_curr.
 * Note that this procedure is time consuming, should not be used frequently
 */
static int traffic_update_current(void)
{
	u_int32_t bucket = 0;	
	void *p;
    struct nf_conn *ct = NULL;
    u_int64_t traffic = 0;
    static unsigned long ts = 0;
    unsigned long now = get_seconds();

    if ((now - ts) > 60) {
        /* reset old current traffic first */
        pctl_log *log;
        int i = 0;

        for (i = 0; i < PCTL_OWNER_NUM; i++) {
            if (owners[i].status != PCTL_STATUS_OK) {
                continue;
            }
            
            traffic_reset_current(&owners[i].traffic);

            list_for_each_entry(log, &owners[i].log_list, log_node) {
                traffic_reset_current(&log->entry.traffic);
            }
        }

        /* traverse all ct entries */
        p = ct_start(&bucket);
        while(p) {
            if (!ct_parse(p, &ct, &traffic))
                traffic_update(ct, traffic, true);
            p = ct_next(&bucket, p);
        }
        ct_stop();
        ts = now;
        PCTL_DEBUG("update end\n");
    } else {
        PCTL_DEBUG("called too frequently\n");
    }

    return 0;
}

static inline u_int64_t traffic_get(traffic_stat *traffic)
{
    u_int64_t tmp = 0;
    u_int64_t ret = 0;

    traffic_update_current();

    read_lock_bh(&traffic->lock);
    tmp = traffic->old + traffic->curr;

    if (tmp < traffic->curr_prev) {
        printErr("something is not right...\n");
        ret = traffic->old;
    } else {
        ret = tmp - traffic->curr_prev;
    }

    read_unlock_bh(&traffic->lock);
    return ret;
}

static inline void traffic_reset(traffic_stat *traffic)
{
    write_lock_bh(&traffic->lock);
    traffic->old = 0;
    traffic->curr_prev = traffic->curr;
    write_unlock_bh(&traffic->lock);
}

/*
 *	Callback event invoked when a conntrack connection's state changes.
 */
#if defined(CONFIG_NF_CONNTRACK_CHAIN_EVENTS)
static int ipt_stat_conntrack_event(struct notifier_block *this,
				unsigned long events, void *ptr)
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static int ipt_stat_conntrack_event(unsigned int events, struct nf_ct_event *item)
#else
static int ipt_stat_conntrack_event(struct notifier_block *this,
						   unsigned long events, void *ptr)
#endif
#endif
{
#if defined(CONFIG_NF_CONNTRACK_CHAIN_EVENTS) || (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
	struct nf_ct_event *item = ptr;
#endif
	struct nf_conn *ct = item->ct;
    struct nf_conn *_ct = NULL;
    u_int64_t traffic = 0;
	
	if (unlikely(!ct)) {
		return NOTIFY_DONE;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
    if (unlikely(test_bit(IP_CT_UNTRACKED, &ct->status))){
        return NOTIFY_DONE;
    }
#else if (LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0))
	if (unlikely(nf_ct_is_untracked(ct))) {
		return NOTIFY_DONE;
	}
#endif

	/*
	 * We're only interested in destroy events.
	 */
	if (unlikely(!(events & (1 << IPCT_DESTROY)))) {
		return NOTIFY_DONE;
	}
	
	if (!ct_parse(&ct->tuplehash[IP_CT_DIR_ORIGINAL], &_ct, &traffic))
        traffic_update(_ct, traffic, false);

	return NOTIFY_DONE;
}

/*
 * Netfilter conntrack event system to monitor connection tracking changes
 */
#if defined(CONFIG_NF_CONNTRACK_CHAIN_EVENTS)
static struct notifier_block ipt_stat_conntrack_notifier = {
	.notifier_call = ipt_stat_conntrack_event,
	.priority = -1,	/* Lowest priority */
};
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static struct nf_ct_event_notifier ipt_stat_conntrack_notifier = {
    .fcn = ipt_stat_conntrack_event,
};
#else
static struct notifier_block ipt_stat_conntrack_notifier = {
	.notifier_call = ipt_stat_conntrack_event,
	.priority = -1,	/* Lowest priority */
};
#endif 
#endif /* CONFIG_NF_CONNTRACK_CHAIN_EVENTS */



static int urlclass_proc_read(struct seq_file *s, void *unused)
{
	struct url_class_entry *pos = NULL;
	int entry_unused = 0;

	spin_lock_bh(&url_class_nl_lock);

	entry_unused = 512-urlclass_entry_used;
	printk("__urlclass_proc__. url_class_buffer_list remain =%d \n", entry_unused);
	printk("__urlclass_proc__. url_class_list used %d \n",urlclass_entry_used);

	if (urlclass_entry_used >= ENTRY_BUSY)
	{
		list_for_each_entry(pos, &url_class_list, list) {
			printk("__urlclass_proc__. id=%d timestamp=%d url=%s ips=%d cat=%x dns=%x\n", pos->id, pos->timestamp, pos->url, pos->url_addr_len, pos->cat_id, pos->dns_id);
		}
	}

	spin_unlock_bh(&url_class_nl_lock);

	return 0;
}

static int urlclass_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, urlclass_proc_read, NULL);
}

static ssize_t urlclass_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data)
{
	printErr("do nothing about urlclass...\n");
	return -1;
}

static int payment_proc_read(struct seq_file *s, void *unused)
{
	read_lock_bh(&payment_lock);
	seq_printf(s, "%d\n", payment);
	read_unlock_bh(&payment_lock);

	return 0;
}

static int payment_proc_open(struct inode *inode, struct file *file)  
{
	return single_open(file, payment_proc_read, NULL);
}

static ssize_t payment_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data)
{
	char kbuf[2] = {0};
	if (count != 2) {
		printErr("count = %d.\n", count);
		return -1;
	}

	if(copy_from_user(kbuf, buf, 2))
	{
		return -EFAULT;
	}

	write_lock_bh(&payment_lock);
	payment = kbuf[0];
	write_unlock_bh(&payment_lock);

	return count;
}

#endif

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
/*!
 *\fn           static bool time_limit_enable(const struct _xt_pctl_info *info)
 *\brief        check time limit for fc support
 *\return       enable or not
 */
static bool time_limit_enable(const struct _xt_pctl_info *info)
{
	if( info->advancedMode )
	{
		if( info->advanced_enable
				|| info->sun_forenoon || info->sun_afternoon
				|| info->mon_forenoon || info->mon_afternoon
				|| info->tue_forenoon || info->tue_afternoon
				|| info->wed_forenoon || info->wed_afternoon
				|| info->thu_forenoon || info->thu_afternoon
				|| info->fri_forenoon || info->fri_afternoon
				|| info->sat_forenoon || info->sat_afternoon )
		{
			return true;
		}
	}
	else
	{
		if( info->workday_limit || info->weekend_limit )
		{
			return true;
		}

		if (info->hosts_type != 1 && info->hosts_type != 2)
		{
			if( info->workday_bedtime || info->weekend_bedtime )
			{
				return true;
			}
		}
		else
		{
			if( info->sun_time || info->mon_time || info->tue_time || info->wed_time
						 || info->thu_time  || info->fri_time  || info->sat_time )
			{
				return true;
			}
		}
	}

	return false;
}
#endif

/*!
 *\fn           static bool match(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns or servername in https(Clienthello) )
 *\return       found or not
 */
static bool match(const struct sk_buff *skb, struct xt_action_param *par)
{   
    int status = PCTL_STATUS_OK;
    unsigned int stamp = GET_TIMESTAMP();
    unsigned int stamp_local = stamp - 60 * sys_tz.tz_minuteswest ;

    const struct _xt_pctl_info *info = par->matchinfo;
    const struct iphdr *iph = ip_hdr(skb); /* ipv4 only */
    pctl_owner *pOwner = NULL;
    int id = info->id;
	
#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
		struct nf_conn *ct;
		enum ip_conntrack_info ctinfo;
#endif


#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS	
	/* add by wanghao */
	if (id == PCTL_ID_DNS_RESP) {//if it is a DNS response match rule
		_url_class_parse_response(skb);
		return false;
	}
	/* add end */
#endif

#if PCTL_DEVICE_INFO
    if(id == PCTL_OWNER_ID_ALL)
    {
        /* check type for all device */
        check_device_info(skb);
        return false;
    }
#endif

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS	
		if(id == PCTL_WEB_URL_ID_ALL)
		{
			/* query url type for all dns */
			if (IPPROTO_UDP == iph->protocol)
			{
				struct udphdr *udph = (void *)iph + iph->ihl*4;
				if(DNS_PORT == ntohs(udph->dest)) 
				{			
					//parse URL and notify auc to do a query
					url_class_parse(skb, info, PCTL_STATUS_OK, stamp);
				}
			}
            
            // query url type for all http/https
            if(IPPROTO_TCP == iph->protocol)
            {
                struct tcphdr *tcph = (void *)iph + iph->ihl*4;
                if(HTTP_PORT == ntohs(tcph->dest))
                {			
                    //parse http URL and notify auc to do a query
                    url_class_parse_http(skb, info, PCTL_STATUS_OK, stamp);
                }
                else if (HTTPS_PORT == ntohs(tcph->dest))
                {
                    //parse https URL and notify auc to do a query
                    url_class_parse_https(skb, info, PCTL_STATUS_OK, stamp);
                }
            }
            
			return false;
		}
#endif

    if(id < 0 || id >= PCTL_OWNER_NUM) 
    {
        if (printk_ratelimit())
        	PCTL_ERROR("id is out of range. id = %d.", id);
        return false;
    }

    pOwner = owners + id;

#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
	/* PCTL is handling this connection, let SFE ignore it */
	ct = nf_ct_get(skb, &ctinfo);
#endif

    /* write_lock_bh(&pOwner->lock); */

	//add by wanghao
	if (info->advancedMode) {
    	status = match_time_advanced(skb, info, stamp);
	}
	else {
		status = match_time(skb, info, stamp);
	}

    pOwner->status = status;

	if (PCTL_STATUS_INBONUS == status)
	{
		log_update_history(id, stamp);
		write_lock_bh(&pOwner->lock);
		update_stats_time_limit(&pOwner->today_stats, stamp, info);
		update_stats_daytime(&pOwner->today_stats, stamp, 1);
		write_unlock_bh(&pOwner->lock);
		status = PCTL_STATUS_OK;
	}
    else if(PCTL_STATUS_OK == status)
    {
        log_update_history(id, stamp);
		write_lock_bh(&pOwner->lock);
		//update_stats(&pOwner->today_stats, stamp, 0);
		update_stats_time_limit(&pOwner->today_stats, stamp, info);
		update_stats_daytime(&pOwner->today_stats, stamp, 0);
		write_unlock_bh(&pOwner->lock);
    }
    else  /*still need to check new day refreash if the old day time is already limited,by liuqu */
    {
		log_update_history(id, stamp);
		update_stats_time_limit(&pOwner->today_stats, stamp, info);
    }

    if (IPPROTO_TCP == iph->protocol)
    {
        struct tcphdr *tcph = (void *)iph + iph->ihl*4;
        
        if(HTTP_PORT == ntohs(tcph->dest))
        {
#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
            if (ct) {
                ct->mark |= PCTL_HANDLING_MARK;
            }
#endif
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS				
			//parse http URL and notify auc to do a query
			url_class_parse_http(skb, info, status, stamp);
#endif
            status = match_http(skb, info, status, stamp);
        }
        else if (HTTPS_PORT == ntohs(tcph->dest))
        {
#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
            if (ct) {
                ct->mark |= PCTL_HANDLING_MARK;
            }
#endif
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS				
			//parse https URL and notify auc to do a query
			url_class_parse_https(skb, info, status, stamp);
#endif
            status = match_https(skb, info, status, stamp);
        }
        else if(DNS_PORT == ntohs(tcph->dest))
        {
            status = PCTL_STATUS_OK;
        } 
    }
    else if (IPPROTO_UDP == iph->protocol)
    {
        struct udphdr *udph = (void *)iph + iph->ihl*4;
        if(DNS_PORT == ntohs(udph->dest)) 
        {

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS				
        	/* add by wanghao */
			//parse URL and notify auc to do a query
			url_class_parse(skb, info, status, stamp);
			/* add end */
#endif
#if PCTL_DNS_REDIRECT
            status = match_dns(skb, info, status, stamp);
#else
            status = PCTL_STATUS_OK;
#endif
        }
    }

    /* write_unlock_bh(&pOwner->lock); */

/*Hanjiayan@20190610 add*/
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
	if (ct) {
		ct->mark |= SESSION_FLAG2_SAE_ONLY;
	}
#endif	

   // PCTL_ERROR("pctl_log=%d pctl_history=%d pctl_owner=%d",
   //            sizeof(pctl_log), sizeof(pctl_history), sizeof(pctl_owner));
   /* add by wanghao  */
   if (PCTL_STATUS_OK == status)
   {
#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
        if(device_keepalive_check(skb) 
            || (ct && ((ct->mark & PCTL_HANDLING_MARK) && !(ct->mark & PCTL_HANDLE_OK_MARK))))
        {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,38))
            blog_skip(skb, blog_skip_reason_dpi);
#else
            blog_skip(skb);
#endif
        }
#if 0
        else if(time_limit_enable(info))
        {
#if defined(CONFIG_TP_AVIRA_PATCH) || defined(CONFIG_TP_SPT_PATCH)
            if(!PKTWITHSPTFLAG(skb))
#endif
            {
                if(blog_ptr(skb))
                {
                    blog_ptr(skb)->wl_hw_support.is_rx_hw_acc_en = 0;
                }
            }
        }
#endif
        pOwner->status_block = 0;
#endif

	   return false;
   }
   else
   {
#if defined(SUPPORT_SHORTCUT_FE)
	   struct sfe_connection_destroy sid;
	   struct nf_conntrack_tuple orig_tuple;
   
	   if (!ct)
	   {
		   return true;
	   }
   
	   orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	   sid.protocol = (int32_t)orig_tuple.dst.protonum;
	   
	   if (sid.protocol == IPPROTO_TCP)
	   {
		   sid.src_port = orig_tuple.src.u.tcp.port;
		   sid.dest_port = orig_tuple.dst.u.tcp.port;
	   }
	   else if (sid.protocol == IPPROTO_UDP)
	   {
		   sid.src_port = orig_tuple.src.u.udp.port;
		   sid.dest_port = orig_tuple.dst.u.udp.port;
	   }
   
	   sid.src_ip.ip = (__be32)orig_tuple.src.u3.ip;
	   sid.dest_ip.ip = (__be32)orig_tuple.dst.u3.ip;
   
	   //clear ct first
	   nf_ct_kill_acct(ct, ctinfo, skb);
	   
	   //clear SFE rule
	   if (clearSfeRulePtr)
	   {
		   clearSfeRulePtr(&sid);
	   }
	   
#if defined(CONFIG_RA_HW_NAT) || defined(CONFIG_RA_HW_NAT_MODULE)
	   //clear HNAT cache
	   if (clearHnatCachePtr)
	   {
		   struct _ipv4_hnapt ipv4_hnapt;
   
		   ipv4_hnapt.sip = ntohl(sid.src_ip.ip);
		   ipv4_hnapt.dip = ntohl(sid.dest_ip.ip);
		   ipv4_hnapt.sport = ntohs(sid.src_port);
		   ipv4_hnapt.dport = ntohs(sid.dest_port);
		   
		   clearHnatCachePtr(&ipv4_hnapt);
	   }
#endif
#endif

/*Hanjiayan@20160619 add*/
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
	 if (!ct)
	 {
		 return true;
	 }

	if(ppa_hook_session_del_fn){
		ppa_hook_session_del_fn(ct, PPA_F_SESSION_ORG_DIR | PPA_F_SESSION_REPLY_DIR);	
	}

	nf_ct_kill_acct(ct, ctinfo, skb);
#endif	

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
        if (!ct || !test_bit(IPS_BLOG_BIT, &ct->status))
        {
            return true;
        }
        
        /* clear flowcache rules of src mac */
        if (PCTL_STATUS_BLOCKED == status || PCTL_STATUS_TIME_LIMIT == status || PCTL_STATUS_BEDTIME == status)
        {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
            if((!pOwner->status_block)
                || (ct->blog_key[BLOG_PARAM1_DIR_ORIG] != BLOG_KEY_FC_INVALID
                    || ct->blog_key[BLOG_PARAM1_DIR_REPLY] != BLOG_KEY_FC_INVALID))
#else
            if((!pOwner->status_block)
                || (ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_ORIG] != BLOG_KEY_FC_INVALID
                    || ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_REPLY] != BLOG_KEY_FC_INVALID))
#endif
            {
                printk("[%s:%d]Pctl ID = %d, skb = 0x%p, to flush mac: "H_MACFMT"\n", __FUNCTION__, __LINE__, 
                        id, skb, H_NMACQUAD(eth_hdr(skb)->h_source));
                tp_fcache_flush_client(eth_hdr(skb)->h_source);
                pOwner->status_block = 1;
            }
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
        /*
         * Notice: nf_ct_kill_acct won't kill ct before timeout
         */
        //nf_ct_kill_acct(ct, ctinfo, skb);
        if(del_timer(&ct->timeout))
            nf_ct_delete(ct, 0, 0);
#else
        /*
         * Notice: caused ct list access panic
         */
        //nf_ct_kill_acct(ct, ctinfo, skb);
#endif
#endif
	   
	   return true;
   }
   /* add end  */

  //  return (PCTL_STATUS_OK == status)? false : true;
}

#ifdef PCTL_SUPPORT_IPV6
/*!
 *\fn           static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns or servername in https(Clienthello)),for ipv6
 *\return       found or not
 */
static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *par)
{   
    int status = PCTL_STATUS_OK;
    unsigned int stamp = GET_TIMESTAMP();
    unsigned int stamp_local = stamp - 60 * sys_tz.tz_minuteswest ;
	unsigned int offset_tcp = 0;
	unsigned int offset_udp = 0;

    const struct _xt_pctl_info *info = par->matchinfo;
    //const struct ipv6hdr *iph6 = ipv6_hdr(skb); /* ipv6*/
    pctl_owner *pOwner = NULL;
    int id = info->id;
	
#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
		struct nf_conn *ct;
		enum ip_conntrack_info ctinfo;
#endif


#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS	
	/* add by wanghao */
	if (id == PCTL_ID_DNS_RESP) {//if it is a DNS response match rule
		_url_class_parse_response(skb);
		return false;
	}
	/* add end */
#endif

#if PCTL_DEVICE_INFO
    if(id == PCTL_OWNER_ID_ALL)
    {
        /* check type for all device */
        check_device_info(skb);
        return false;
    }
#endif

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS	
		if(id == PCTL_WEB_URL_ID_ALL)
		{
			/* query url type for all ipv6 dns */
			if (ipv6_find_hdr(skb, &offset_udp, IPPROTO_UDP, NULL, NULL) >= 0)
			{
				struct udphdr udph_buf;
				struct udphdr *udph = skb_header_pointer(skb, offset_udp, sizeof(udph_buf), &udph_buf);
				if(DNS_PORT == ntohs(udph->dest)) 
				{
					//parse URL and notify auc to do a query
					url_class_parse(skb, info, status, stamp);
				}
			}
            else if(ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) >= 0)
            {
                struct tcphdr tcph_buf;
                struct tcphdr *tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
                
                if(HTTP_PORT == ntohs(tcph->dest))
                {
                    //parse http URL and notify auc to do a query
                    url_class_parse_http(skb, info, PCTL_STATUS_OK, stamp);
                }
                else if (HTTPS_PORT == ntohs(tcph->dest))
                {
                    //parse https URL and notify auc to do a query
                    url_class_parse_https(skb, info, PCTL_STATUS_OK, stamp);
                }
            }
			return false;
		}
#endif

    if(id < 0 || id >= PCTL_OWNER_NUM) 
    {
        if (printk_ratelimit())
        	PCTL_ERROR("id is out of range. id = %d.", id);
        return false;
    }

    pOwner = owners + id;

#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
	/* PCTL is handling this connection, let SFE ignore it */
	ct = nf_ct_get(skb, &ctinfo);
#endif

    /* write_lock_bh(&pOwner->lock); */

	//add by wanghao
	if (info->advancedMode) {
    	status = match_time_advanced(skb, info, stamp);
	}
	else {
		status = match_time(skb, info, stamp);
	}

    if (PCTL_STATUS_INBONUS == status)
    {
        log_update_history(id, stamp);
        write_lock_bh(&pOwner->lock);
        update_stats_time_limit(&pOwner->today_stats, stamp, info);
        update_stats_daytime(&pOwner->today_stats, stamp, 1);
        write_unlock_bh(&pOwner->lock);
        status = PCTL_STATUS_OK;
	}
    else if(PCTL_STATUS_OK == status) 
    {
        log_update_history(id, stamp);
		write_lock_bh(&pOwner->lock);
        //update_stats(&pOwner->today_stats, stamp, 0);
        update_stats_time_limit(&pOwner->today_stats, stamp, info);
        update_stats_daytime(&pOwner->today_stats, stamp, 0);
		write_unlock_bh(&pOwner->lock);
    }
    else  /*still need to check new day refreash if the old day time is already limited,by liuqu */
    {
        log_update_history(id, stamp);
        update_stats_time_limit(&pOwner->today_stats, stamp, info);
    }

    if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) >= 0)
	{
		struct tcphdr tcph_buf;
		struct tcphdr *tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
		PCTL_DEBUG("xt_pctl: recv ipv6 tcp skb");
        
        if(HTTP_PORT == ntohs(tcph->dest))
        {
#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
            if (ct) {
                ct->mark |= PCTL_HANDLING_MARK;
            }
#endif
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS				
			//parse http URL and notify auc to do a query
			url_class_parse_http(skb, info, status, stamp);
#endif
            status = match_http(skb, info, status, stamp);
        }
        else if (HTTPS_PORT == ntohs(tcph->dest))
        {
#if defined(SUPPORT_SHORTCUT_FE) || defined(CONFIG_TP_FC_PCTL_SUPPORT) || defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
            if (ct) {
                ct->mark |= PCTL_HANDLING_MARK;
            }
#endif
#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS				
			//parse https URL and notify auc to do a query
			url_class_parse_https(skb, info, status, stamp);
#endif
            status = match_https(skb, info, status, stamp);
        }
        else if(DNS_PORT == ntohs(tcph->dest))
        {
            status = PCTL_STATUS_OK;
        } 
    }
    else if (ipv6_find_hdr(skb, &offset_udp, IPPROTO_UDP, NULL, NULL) >= 0)
	{
		struct udphdr udph_buf;
		struct udphdr *udph = skb_header_pointer(skb, offset_udp, sizeof(udph_buf), &udph_buf);
		PCTL_DEBUG("xt_pctl: recv ipv6 udp skb");
        if(DNS_PORT == ntohs(udph->dest)) 
        {

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS				
        	/* add by wanghao */
			//parse URL and notify auc to do a query
			url_class_parse(skb, info, status, stamp);
			/* add end */
#endif
#if PCTL_DNS_REDIRECT
            status = match_dns(skb, info, status, stamp);
#else
            status = PCTL_STATUS_OK;
#endif
        }
    }

    /* write_unlock_bh(&pOwner->lock); */

/*Hanjiayan@20190610 add*/
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
	if (ct) {
		ct->mark |= SESSION_FLAG2_SAE_ONLY;
	}
#endif	

   // PCTL_ERROR("pctl_log=%d pctl_history=%d pctl_owner=%d",
   //            sizeof(pctl_log), sizeof(pctl_history), sizeof(pctl_owner));
   /* add by wanghao  */
   if (PCTL_STATUS_OK == status)
   {
#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
        if(device_keepalive_check(skb) 
            || (ct && ((ct->mark & PCTL_HANDLING_MARK) && !(ct->mark & PCTL_HANDLE_OK_MARK))))
        {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,38))
            blog_skip(skb, blog_skip_reason_dpi);
#else
            blog_skip(skb);
#endif
        }
#if 0
        else if(time_limit_enable(info))
        {
#if defined(CONFIG_TP_AVIRA_PATCH) || defined(CONFIG_TP_SPT_PATCH)
            if(!PKTWITHSPTFLAG(skb))
#endif
            {
                if(blog_ptr(skb))
                {
                    blog_ptr(skb)->wl_hw_support.is_rx_hw_acc_en = 0;
                }
            }
        }
#endif
        pOwner->status_block = 0;
#endif

	   return false;
   }
   else
   {
#if defined(SUPPORT_SHORTCUT_FE)
	   struct sfe_connection_destroy sid;
	   struct nf_conntrack_tuple orig_tuple;
   
	   if (!ct)
	   {
		   return true;
	   }
   
	   orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	   sid.protocol = (int32_t)orig_tuple.dst.protonum;
	   
	   if (sid.protocol == IPPROTO_TCP)
	   {
		   sid.src_port = orig_tuple.src.u.tcp.port;
		   sid.dest_port = orig_tuple.dst.u.tcp.port;
	   }
	   else if (sid.protocol == IPPROTO_UDP)
	   {
		   sid.src_port = orig_tuple.src.u.udp.port;
		   sid.dest_port = orig_tuple.dst.u.udp.port;
	   }
   
	   sid.src_ip.ip = (__be32)orig_tuple.src.u3.ip;
	   sid.dest_ip.ip = (__be32)orig_tuple.dst.u3.ip;
   
	   //clear ct first
	   nf_ct_kill_acct(ct, ctinfo, skb);
	   
	   //clear SFE rule
	   if (clearSfeRulePtr)
	   {
		   clearSfeRulePtr(&sid);
	   }
	   
#if defined(CONFIG_RA_HW_NAT) || defined(CONFIG_RA_HW_NAT_MODULE)
	   //clear HNAT cache
	   if (clearHnatCachePtr)
	   {
		   struct _ipv4_hnapt ipv4_hnapt;
   
		   ipv4_hnapt.sip = ntohl(sid.src_ip.ip);
		   ipv4_hnapt.dip = ntohl(sid.dest_ip.ip);
		   ipv4_hnapt.sport = ntohs(sid.src_port);
		   ipv4_hnapt.dport = ntohs(sid.dest_port);
		   
		   clearHnatCachePtr(&ipv4_hnapt);
	   }
#endif
#endif

/*Hanjiayan@20160619 add*/
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
	 if (!ct)
	 {
		 return true;
	 }

	if(ppa_hook_session_del_fn){
		ppa_hook_session_del_fn(ct, PPA_F_SESSION_ORG_DIR | PPA_F_SESSION_REPLY_DIR);	
	}

	nf_ct_kill_acct(ct, ctinfo, skb);
#endif	

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
        if (!ct || !test_bit(IPS_BLOG_BIT, &ct->status))
        {
            return true;
        }
        
        /* clear flowcache rules of src mac */
        if (PCTL_STATUS_BLOCKED == status || PCTL_STATUS_TIME_LIMIT == status || PCTL_STATUS_BEDTIME == status)
        {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
            if((!pOwner->status_block)
                || (ct->blog_key[BLOG_PARAM1_DIR_ORIG] != BLOG_KEY_FC_INVALID
                    || ct->blog_key[BLOG_PARAM1_DIR_REPLY] != BLOG_KEY_FC_INVALID))
#else
            if((!pOwner->status_block)
                || (ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_ORIG] != BLOG_KEY_FC_INVALID
                    || ct->bcm_ext.blog_key[BLOG_PARAM1_DIR_REPLY] != BLOG_KEY_FC_INVALID))
#endif
            {
                printk("[%s:%d]Pctl ID = %d, skb = 0x%p, to flush mac: "H_MACFMT"\n", __FUNCTION__, __LINE__, 
                        id, skb, H_NMACQUAD(eth_hdr(skb)->h_source));
                tp_fcache_flush_client(eth_hdr(skb)->h_source);
                pOwner->status_block = 1;
            }
        }

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
        /* TODO: IPv6 check */
        if(del_timer(&ct->timeout))
            nf_ct_delete(ct, 0, 0);
#else
        /*
         * Notice: caused ct list access panic
         */
        //nf_ct_kill_acct(ct, ctinfo, skb);
#endif

#endif
	   
	   return true;
   }
   /* add end  */

  //  return (PCTL_STATUS_OK == status)? false : true;
}
#endif

/*!
 *\fn           static int __init pctl_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init pctl_init(void)
{
    int i = 0, j = 0;
    char filename[16] = {0};
    pctl_owner *pOwner = NULL;
    int ret = 0;

    /* create proc dir */
    proc_dir = proc_mkdir(PCTL_PROC_DIR, NULL);
    if(!proc_dir) 
    {
        PCTL_ERROR("proc_mkdir failed.");
        return -1;
    }

    for(i = 0; i < PCTL_OWNER_NUM; i++) 
    {
       pOwner = owners + i;

       memset(pOwner, 0, sizeof(pctl_owner));
       pOwner->id = i;

       /* init list */
       INIT_LIST_HEAD(&pOwner->log_list);
       INIT_LIST_HEAD(&pOwner->client_list);

       /* init url hash */
       for(j = 0; j < PCTL_URL_HASH_SIZE; j++) 
       {
           INIT_HLIST_HEAD(&pOwner->hash_list[j]);
       }

       /* init rwlock */
       rwlock_init(&pOwner->lock);

       /* create proc */
       sprintf(filename, "%d", pOwner->id);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,13))
       pOwner->proc_file = proc_create(filename, S_IRUGO, proc_dir, &log_proc_fops);
       if(!pOwner->proc_file) 
       {
           PCTL_ERROR("proc_create failed.");
           return -1;
       }
#else
       pOwner->proc_file = create_proc_entry(filename, S_IRUGO, proc_dir);
       if(!pOwner->proc_file) 
       {
           PCTL_ERROR("create_proc_entry failed.");
           return -1;
       }
       pOwner->proc_file->proc_fops = &log_proc_fops;
#endif
    }

#if PCTL_DEVICE_INFO
    rwlock_init(&device_info_lock);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,13))
    device_proc_file = proc_create(DEVICE_INFO_PROC_FILENAME, S_IRUGO, proc_dir, &device_proc_fops);
    if(!device_proc_file) 
    {
        PCTL_ERROR("proc_create failed.");
        return -1;
    }
#else
    device_proc_file = create_proc_entry(DEVICE_INFO_PROC_FILENAME, S_IRUGO, proc_dir);
    if(!device_proc_file) 
    {
        PCTL_ERROR("create_proc_entry failed.");
        return -1;
    }
    device_proc_file->proc_fops = &device_proc_fops;
#endif
#endif

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
    pctl_cb_register(skb_keepalive_cb, fkb_keepalive_cb);
#endif

	//HTTP redirect
	rwlock_init(&vercode_lock);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,13))
	vercode_proc_file = proc_create(VERCODE_PROC_FILENAME, S_IRUGO, proc_dir, &vercode_proc_fops);
	if(!vercode_proc_file) 
	{
		PCTL_ERROR("create_proc_entry failed.");
		return -1;
	}
#else	
	vercode_proc_file = create_proc_entry(VERCODE_PROC_FILENAME, S_IRUGO, proc_dir);
	if(!vercode_proc_file) 
	{
		PCTL_ERROR("create_proc_entry failed.");
		return -1;
	}
	vercode_proc_file->proc_fops = &vercode_proc_fops;
#endif

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
		//Avira Paid Status
	rwlock_init(&payment_lock);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,13))
		payment_proc_file = proc_create(PAYMENT_PROC_FILENAME, S_IRUGO, proc_dir, &payment_proc_fops);
		if(!payment_proc_file)
		{
			PCTL_ERROR("create_proc_entry failed.");
			return -1;
		}
#else
		payment_proc_file = create_proc_entry(PAYMENT_PROC_FILENAME, S_IRUGO, proc_dir);
		if(!payment_proc_file)
		{
			PCTL_ERROR("create_proc_entry failed.");
			return -1;
		}
	payment_proc_file->proc_fops = &payment_proc_fops;
#endif
	/* add by FJS, debug for url_class_list & url_class_buffer_list */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,13))
	urlclass_proc_file = proc_create(URL_CLASS_LIST_FILENAME, S_IRUGO, proc_dir, &urlclass_proc_fops);
	if(!urlclass_proc_file)
	{
		PCTL_ERROR("create_proc_entry failed.");
		return -1;
	}
#else
	urlclass_proc_file = create_proc_entry(URL_CLASS_LIST_FILENAME, S_IRUGO, proc_dir);
	if(!urlclass_proc_file)
	{
		PCTL_ERROR("create_proc_entry failed.");
		return -1;
	}
	urlclass_proc_file->proc_fops = &urlclass_proc_fops;
#endif

	/* add by wanghao */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0))
			struct netlink_kernel_cfg cfg = {
				.input = url_class_recv,
			};
			url_class_nl = netlink_kernel_create(&init_net, NETLINK_URL_CLASS, &cfg);
#else
 	url_class_nl = netlink_kernel_create(&init_net, NETLINK_URL_CLASS, 0, url_class_recv, NULL, THIS_MODULE);
#endif
	if (!url_class_nl) {
		printErr("create_netlink_sock for url failed.");
		return -1;
	}
	
	url_class_task = kthread_create(url_class_send_thread, NULL, "url_send");
	if (IS_ERR(url_class_task)) {
		printErr("create_kernel_thread for url failed.");
		return -1;
	}

#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC		
	_url_addr_list_init();
	_url_class_list_init();
#else
	_url_addr_list_init();
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	timer_setup(&urlclass_timer, (void *)urlclass_timer_handle, 0); 
#else
	init_timer(&urlclass_timer);
	urlclass_timer.function = (void *)urlclass_timer_handle;
	urlclass_timer.expires = jiffies + URLCLASS_TIMER_INTERVAL;
	add_timer(&urlclass_timer);
#endif
#endif
	

#ifdef SUPPORT_HOMECARE_PRO_BLOCKING	
	BUG_ON(block_http_redirect != NULL);
	RCU_INIT_POINTER(block_http_redirect, vercode_http_redirect);
	/* add end */
#endif
	ret = nf_conntrack_register_notifier(&init_net, &ipt_stat_conntrack_notifier);
	if (ret < 0) {
		printk("cannot register notifier.\n");
		return ret;
	}
    return xt_register_matches(pctl_match, ARRAY_SIZE(pctl_match));
}

/*!
 *\fn           static void __exit pctl_exit(void)
 *\brief        mod exit
 *\return       none
 */
static void __exit pctl_exit(void)
{
    int i = 0;
    char filename[16] = {0};
    pctl_owner *pOwner = NULL;

    for(i = 0; i < PCTL_OWNER_NUM; i++) 
    {
        pOwner = owners + i;
        log_clear(i, true);
        client_list_clear(pOwner);

        /* remove proc */
        if(pOwner->proc_file) 
        {
        sprintf(filename, "%d", pOwner->id);
        remove_proc_entry(filename, proc_dir);
        }
    }

#if PCTL_DEVICE_INFO
    remove_proc_entry(DEVICE_INFO_PROC_FILENAME, proc_dir);
#endif
	remove_proc_entry(VERCODE_PROC_FILENAME, proc_dir);

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS
	remove_proc_entry(PAYMENT_PROC_FILENAME, proc_dir);
	remove_proc_entry(URL_CLASS_LIST_FILENAME, proc_dir);

	del_timer_sync(&urlclass_timer);
#endif

    remove_proc_entry(PCTL_PROC_DIR, NULL);


	/* add by wanghao */
#ifdef SUPPORT_HOMECARE_PRO_BLOCKING	
	RCU_INIT_POINTER(block_http_redirect, NULL);
#endif

#ifdef SUPPORT_HOMECARE_PRO_URL_CLASS	
	spin_lock_bh(&url_class_nl_lock);
#ifdef PCTL_MEMORY_DYNAMIC_KMALLOC		
		_url_class_list_destroy();
		_url_addr_list_destroy();
#endif	
	spin_unlock_bh(&url_class_nl_lock);

	if (url_class_task) {
		kthread_stop(url_class_task);
	}

	if (url_class_nl) {
	    netlink_kernel_release(url_class_nl);
	}
	/* add end */
#endif	

#if defined(CONFIG_TP_FC_PCTL_SUPPORT)
    pctl_cb_register(NULL, NULL);
#endif

    nf_conntrack_unregister_notifier(&init_net, &ipt_stat_conntrack_notifier);
    xt_unregister_matches(pctl_match, ARRAY_SIZE(pctl_match));
}

/***************************************************************************/
/*                      PUBLIC_FUNCTIONS                     */
/***************************************************************************/

/***************************************************************************/
/*                      GLOBAL_FUNCTIONS                     */
/***************************************************************************/
module_init(pctl_init);
module_exit(pctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miao Wen <miaowen@tp-link.com.cn>");
MODULE_DESCRIPTION("Xtables: parental control");
MODULE_ALIAS("ipt_pctl");
MODULE_ALIAS("ip6t_pctl");
