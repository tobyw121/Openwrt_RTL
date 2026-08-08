/*
Linux Kernel Hacking:
	net/core/neighbour.c						// ARP
	net/ipv4/fib_hash.c						// ROUTE
	net/ipv4/netfilter/ip_conntrack_core.c				// NAPT (PATH*2)
	net/ipv4/netfilter/ip_nat_core.c				// NAPT (PATH*2)
	net/ipv4/ip_input.c						// FastPath_Enter()
	net/ipv4/ip_output.c						// FastPath_Track()
*/
/*skb->h.raw = skb->nh.raw = skb->data*/

#ifndef __KERNEL__
#define	__KERNEL__
#endif

#if defined(CONFIG_NET_SCHED)
#include <linux/netfilter_ipv4/ip_tables.h>
extern int gQosEnabled;
#endif

#if defined(IMPROVE_QOS) || defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <net/arp.h>
#include <net/rtl/rtl865x_netif.h>
#endif
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
#include <linux/interrupt.h>
#include <linux/netfilter.h>
#endif

#include <net/netfilter/nf_conntrack.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/fastpath/fastpath_core.h>
#include <linux/inetdevice.h>
#include <net/route.h>
#if defined(CONFIG_RTL_FASTBRIDGE)
#include <net/rtl/features/fast_bridge.h>
#endif
#ifdef CONFIG_FAST_PATH_MODULE
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#endif
#include <net/rtl/features/rtl_ps_log.h>
#ifdef	MODULE_VERSION
#undef	MODULE_VERSION
#endif
#define	MODULE_NAME		"Realtek FastPath"
#define	MODULE_VERSION	"v1.03"

/*when udp checksum 0 ingore it*/
#define UDP_ZERO_CHECKSUM


//Brad disable
#if 0
static void flush_all_table(void);
#endif

#if	defined(CONFIG_RTL_FAST_FILTER)
#define NF_DROP 	0
#define NF_FASTPATH	1
#define NF_LINUX	2
#define NF_MARK 	3
#define NF_REPEAT	4
#define NF_OMIT 	5
#endif

#if 1
extern unsigned int _br0_mask;
#endif

#if defined(IMPROVE_QOS) && defined(CONFIG_NET_SCHED)
#include <net/sch_generic.h>
#include <net/pkt_sched.h>
#endif
//#if defined(IMPROVE_QOS) && defined(CONFIG_NET_SCHED)
//#include <net/arp.h>
//#endif
#include <net/netfilter/nf_conntrack_core.h>
#include <net/rtl/rtl_nic.h>
#include <net/rtl/rtl865x_fdb_api.h>
#if defined(CONFIG_RTL_819X)
#include <net/ip_fib.h>
#include <net/ip_vs.h>
#include <net/rtl/features/rtl_features.h>
#endif
#if defined(CONFIG_BRIDGE)
#include <bridge/br_private.h>
#endif

/**/
#if defined(CONFIG_RTL_FAST_GRE)
struct pptp_gre_hdr {
	__u8  flags;		/* bitfield */
	__u8  version;		/* should be PPTP_GRE_VER (enhanced GRE) */
	__u16 protocol;		/* should be PPTP_GRE_PROTO (ppp-encaps) */
	__u16 payload_len;	/* size of ppp payload, not inc. gre header */
	__u16 call_id;		/* peer's call_id for this session */
	__u32 seq;		/* sequence number.  Present if S==1 */
	__u32 ack;		/* seq number of highest packet recieved by */
					/*  sender in this session */
};
#define GRE_STREAM_TIMEOUT	(180 * HZ)
#endif
/* =========================
=========================================================================== */
//static uint8 fastpath_forward_flag = 1;		/* default: On */

#ifndef NO_ARP_USED
/* --- ARP Table Structures --- */
struct Arp_List_Entry
{
	uint8 vaild;
	ipaddr_t ip;
	ether_addr_t mac;
	enum ARP_FLAGS flags;
	CTAILQ_ENTRY(Arp_List_Entry) arp_link;
	CTAILQ_ENTRY(Arp_List_Entry) tqe_link;
};

struct Arp_Table
{
	CTAILQ_HEAD(Arp_list_entry_head, Arp_List_Entry) *list;
};

CTAILQ_HEAD(Arp_list_inuse_head, Arp_List_Entry) arp_list_inuse;
CTAILQ_HEAD(Arp_list_free_head, Arp_List_Entry) arp_list_free;

#ifdef CONFIG_FAST_PATH_MODULE
static struct Arp_Table *table_arp=NULL;
#else
struct Arp_Table *table_arp;
#endif

#endif

/* --- Route Table Structures --- */
struct Route_List_Entry
{
	uint8 vaild;
	ipaddr_t ip;
	ipaddr_t mask;
	ipaddr_t gateway;
	uint8 ifname[IFNAME_LEN_MAX];
	enum RT_FLAGS flags;
	CTAILQ_ENTRY(Route_List_Entry) route_link;
	CTAILQ_ENTRY(Route_List_Entry) tqe_link;
};

struct Route_Table
{
	CTAILQ_HEAD(Route_list_entry_head, Route_List_Entry) *list;
};


CTAILQ_HEAD(Route_list_inuse_head, Route_List_Entry) route_list_inuse;
CTAILQ_HEAD(Route_list_free_head, Route_List_Entry) route_list_free;

#ifndef DEL_ROUTE_TBL
#ifdef CONFIG_FAST_PATH_MODULE
static struct Route_Table *table_route=NULL;
#else
struct Route_Table *table_route;
#endif
static int route_table_list_max;
#endif

#define	RTL_FP_NAPT_VALID	0xff
#define	RTL_FP_NAPT_INVALID	0x00
#if	!defined(DEL_NAPT_TBL)
/* --- NAPT Table Structures --- */
struct Napt_List_Entry
{
	uint8 vaild;
	enum NP_PROTOCOL protocol;
	ipaddr_t	intIp;
	uint32		intPort;
	ipaddr_t	extIp;
	uint32		extPort;
	ipaddr_t	remIp;
	uint32		remPort;
	enum NP_FLAGS 	flags;
	CTAILQ_ENTRY(Napt_List_Entry) napt_link;
	CTAILQ_ENTRY(Napt_List_Entry) tqe_link;
};

struct Napt_Table
{
	#if defined(CONFIG_SMP)
	spinlock_t			napt_table_lock;
	#endif
	CTAILQ_HEAD(Napt_list_entry_head, Napt_List_Entry) *list;
};

CTAILQ_HEAD(Napt_list_inuse_head, Napt_List_Entry) napt_list_inuse;
CTAILQ_HEAD(Napt_list_free_head, Napt_List_Entry) napt_list_free;

#ifdef CONFIG_FAST_PATH_MODULE
static struct Napt_Table *table_napt=NULL;
#else
struct Napt_Table *table_napt;
#endif
static int napt_table_list_max;

#endif // DEL_NAPT_TBL

#if defined(IMPROVE_QOS)
#define	MARK_UNDEFINE		0x5d309b1c
#endif


#if 0
#define CTAILQ_HEAD(name, type)						\
struct name {\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last next element */		\
	int tqh_count;\
}
#endif
//============================================================================
struct Path_Table
{
	#if defined(CONFIG_SMP)
	spinlock_t			path_table_lock;
	#endif
	CTAILQ_HEAD(Path_list_entry_head, Path_List_Entry) *list;
};

CTAILQ_HEAD(Path_list_inuse_head, Path_List_Entry) path_list_inuse;
CTAILQ_HEAD(Path_list_free_head, Path_List_Entry) path_list_free;

struct Path_Table *table_path;
static int path_table_list_max;

#if 0
/* --- InterFace Table Structures --- */
struct If_List_Entry
{
	uint8			vaild;
	uint8			ifname[IFNAME_LEN_MAX];
	ipaddr_t		ipAddr;
	ether_addr_t	mac;
	uint32			mtu;
	enum IF_FLAGS	flags;
	CTAILQ_ENTRY(If_List_Entry) if_link;
	CTAILQ_ENTRY(If_List_Entry) tqe_link;
};

CTAILQ_HEAD(If_list_inuse_head, If_List_Entry) if_list_inuse;
CTAILQ_HEAD(If_list_free_head, If_List_Entry) if_list_free;
#endif

#ifdef	DEBUG_PROCFILE
struct proc_dir_entry * fp_arp;
struct proc_dir_entry *fp_path;
struct proc_dir_entry *fp_route;
struct proc_dir_entry *fp_napt;
#endif

/* ==================================================================================================== */
#ifdef CONFIG_UDP_FRAG_CACHE
#define FRAG_IDLE 0
#define FRAG_FORWADING 1
#define FRAG_COMPLETE 2
#define FRAG_IN_KERNEL 3

//#define FRAG_CACHE_TIMEOUT  IP_FRAG_TIME/2;
//#define FRAG_CACHE_TIMEOUT (10 * HZ)

struct Udp_FragCache_Entry {
	uint8	status;
	uint8	protocol;
	uint16	frag_id;
	uint32	src_ip;
	uint32	dst_ip;
	uint16	src_port;
	uint16	dst_port;
	struct timer_list timer;
	CTAILQ_ENTRY(Udp_FragCache_Entry) path_link;
	CTAILQ_ENTRY(Udp_FragCache_Entry) tqe_link;
};

struct Udp_FragCache_Table
{
	#if defined(CONFIG_SMP)
	spinlock_t	udp_frag_table_lock;
	#endif
	CTAILQ_HEAD(Udp_cache_list_head, Udp_FragCache_Entry) *list;
};

CTAILQ_HEAD(Udp_cache_free_head,Udp_FragCache_Entry) udp_cache_list_free;

struct Udp_FragCache_Table *udp_cache_table;
static int max_udp_frag_entry;

struct Negative_FragCache_Entry {
	uint8	status;
	uint8	protocol;
	uint16	frag_id;
	uint32	src_ip;
	uint32	dst_ip;
	struct timer_list timer; 
	CTAILQ_ENTRY(Negative_FragCache_Entry) path_link;
	CTAILQ_ENTRY(Negative_FragCache_Entry) tqe_link;
};

struct Negative_FragCache_Table
{
	#if defined(CONFIG_SMP)
	spinlock_t	negative_frag_table_lock;
	#endif
	CTAILQ_HEAD(Negative_cache_list_head, Negative_FragCache_Entry) list[MAX_UDP_FRAG_ENTRY];
};

CTAILQ_HEAD(Negative_cache_free_head,Negative_FragCache_Entry) negative_cache_list_free;

struct Negative_FragCache_Table *negative_cache_table;
#endif

unsigned int fastpath_packet_bypassed_num = 10;
unsigned int (*rtl_init_bypass_packet_hook)(struct Path_List_Entry *entry_path) = NULL;
unsigned int (*rtl_check_bypass_packet_hook)(struct Path_List_Entry *entry_path) = NULL;
unsigned int rtl_init_bypass_packet(struct Path_List_Entry *entry_path)
{
    entry_path->packet_bypass_checked = 0;
	entry_path->packet_bypassed = 0;
}
unsigned int rtl_check_bypass_packet(struct Path_List_Entry *entry_path)
{
    if((entry_path->packet_bypass_checked)||((++entry_path->packet_bypassed) > fastpath_packet_bypassed_num))
	{
		entry_path->packet_bypass_checked = 1;
		return 0;
	}
	return 1;
}

#ifdef CONFIG_UDP_FRAG_CACHE
static inline unsigned int frag_hashfn(uint16 id, uint32 saddr, uint32 daddr, uint8 prot)
{
	unsigned int h = saddr ^ daddr;

	h ^= (h>>16)^id;
	h ^= (h>>8)^prot;
	return h & (max_udp_frag_entry - 1);
}

static inline void free_fragEntry(struct Udp_FragCache_Entry *entry)
{
	unsigned int hash = frag_hashfn(entry->frag_id, entry->src_ip, entry->dst_ip, entry->protocol);
	#if defined(CONFIG_SMP)
	spin_lock(&udp_cache_table->udp_frag_table_lock);
	#endif

	entry->status= FRAG_IDLE;
	CTAILQ_REMOVE(&udp_cache_table->list[hash], entry, path_link);
	CTAILQ_INSERT_TAIL(&udp_cache_list_free, entry, tqe_link);
	#if defined(CONFIG_SMP)
	spin_unlock(&udp_cache_table->udp_frag_table_lock);
	#endif
	statistic_udp_frag_free++;
}

static void cache_timeout(unsigned long arg)
{
	struct Udp_FragCache_Entry *entry = (struct Udp_FragCache_Entry *)arg;
	//####issue !!! need lock ? , mark_dbg#####
	if(entry->status == FRAG_IDLE)
	{
		//panic_printk(" timeout But IDLE\n");
		return;
	}
	free_fragEntry(entry);
}

static inline void free_cache(struct Udp_FragCache_Entry *entry)
{
	del_timer(&entry->timer);
	free_fragEntry(entry);
}

int udp_fragCache_init(int udp_frag_entry_max)
{
	int i;
	udp_cache_table = (struct Udp_FragCache_Table *)kmalloc(sizeof(struct Udp_FragCache_Table), GFP_ATOMIC);
	if (udp_cache_table == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Udp_FragCache_Table) \n");
		return 0;
	}
	CTAILQ_INIT(&udp_cache_list_free);

	max_udp_frag_entry=udp_frag_entry_max;
	udp_cache_table->list=(struct Udp_cache_list_head *)kmalloc(udp_frag_entry_max*sizeof(struct Udp_cache_list_head), GFP_ATOMIC);
	if (udp_cache_table->list == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Udp_FragCache_Table List) \n");
		return -1;
	}
	for (i=0; i<udp_frag_entry_max; i++) {
		CTAILQ_INIT(&udp_cache_table->list[i]);
	}
	for (i=0; i<udp_frag_entry_max; i++) {
		struct Udp_FragCache_Entry *entry = (struct Udp_FragCache_Entry *)kmalloc(sizeof(struct Udp_FragCache_Entry), GFP_ATOMIC);
		if (entry == NULL) {
			DEBUGP_SYS("MALLOC Failed! (Path Table Entry) \n");
			return 0;
		}
		//init timer , start timer in add_entry!!!
		init_timer(&entry->timer);
		entry->timer.data = (unsigned long)entry;	/* pointer to queue	*/
		entry->timer.function = cache_timeout;		/* expire function	*/
		CTAILQ_INSERT_TAIL(&udp_cache_list_free, entry, tqe_link);
	}
	#if defined(CONFIG_SMP)
	spin_lock_init(&udp_cache_table->udp_frag_table_lock);
	#endif
	return 1;
}

static inline struct Udp_FragCache_Entry *find_fragEntry(uint16 id,uint32 sip,uint32 dip,uint8 protocol)
{
	unsigned int hash = frag_hashfn(id, sip, dip, protocol);
	struct Udp_FragCache_Entry *entry;
	
	#if defined(CONFIG_SMP)
	spin_lock(&udp_cache_table->udp_frag_table_lock);
	#endif

	CTAILQ_FOREACH(entry, &udp_cache_table->list[hash], path_link)
	{
		if ((entry->frag_id== id) &&
			(entry->src_ip== sip) &&
			(entry->dst_ip== dip) &&
			(entry->protocol== protocol) &&
			(entry->status == FRAG_FORWADING))
		{
			#if defined(CONFIG_SMP)
			spin_unlock(&udp_cache_table->udp_frag_table_lock);
			#endif
			return ((struct Udp_FragCache_Entry *)entry);
		}
	}
	#if defined(CONFIG_SMP)
	spin_unlock(&udp_cache_table->udp_frag_table_lock);
	#endif

	return NULL;
}

static inline int add_fragEntry(uint16 id,uint32 sip, uint16 sport, uint32 dip, uint16 dport, uint8 protocol)
{
	unsigned int hash = frag_hashfn(id, sip, dip, protocol);
	struct Udp_FragCache_Entry *entry;

	#if defined(CONFIG_SMP)
	spin_lock(&udp_cache_table->udp_frag_table_lock);
	#endif

	if(CTAILQ_EMPTY(&udp_cache_list_free))
	{
		//panic_printk("table full ,fail!!!\n");
		#if defined(CONFIG_SMP)
		spin_unlock(&udp_cache_table->udp_frag_table_lock);
		#endif
		statistic_udp_frag_tbl_full++;
		return 0;
	}

	entry = CTAILQ_FIRST(&udp_cache_list_free);

	entry->frag_id= id;
	entry->src_ip= sip;
	entry->dst_ip= dip;
	entry->protocol= protocol;
	entry->src_port= sport;
	entry->dst_port= dport;
	entry->status= FRAG_FORWADING;
	CTAILQ_REMOVE(&udp_cache_list_free, entry, tqe_link);
	CTAILQ_INSERT_TAIL(&udp_cache_table->list[hash], entry, path_link);
	#if defined(CONFIG_SMP)
	spin_unlock(&udp_cache_table->udp_frag_table_lock);
	#endif

	entry->timer.expires = jiffies + FRAG_CACHE_TIMEOUT;
	add_timer(&entry->timer);
	statistic_udp_frag_add++;
	return 1;
}

static inline void free_negative_fragEntry(struct Negative_FragCache_Entry *entry)
{
	unsigned int hash = frag_hashfn(entry->frag_id, entry->src_ip, entry->dst_ip, entry->protocol);	
	
	//panic_printk("free_negative_fragEntry:id=0x%X\n",entry->frag_id); 
	#if defined(CONFIG_SMP)
	spin_lock(&negative_cache_table->negative_frag_table_lock);
	#endif
	entry->status= FRAG_IDLE;
	CTAILQ_REMOVE(&negative_cache_table->list[hash], entry, path_link);	
	CTAILQ_INSERT_TAIL(&negative_cache_list_free, entry, tqe_link);
	#if defined(CONFIG_SMP)
	spin_unlock(&negative_cache_table->negative_frag_table_lock);
	#endif
}
	
static void negative_cache_timeout(unsigned long arg)
{
	//panic_printk("negative_cache_timeout will destory Negative_FragCache_Entry\n"); 
	struct Negative_FragCache_Entry *entry = (struct Negative_FragCache_Entry *)arg;
	free_negative_fragEntry(entry);	
}

int negative_fragCache_init(void) 
{
	int i;
	negative_cache_table = (struct Negative_FragCache_Table *)kmalloc(sizeof(struct Negative_FragCache_Table), GFP_ATOMIC);
	if (negative_cache_table == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Udp_FragCache_Table) \n");
		return 0;
	}
	CTAILQ_INIT(&negative_cache_list_free);

	for (i=0; i<MAX_UDP_FRAG_ENTRY; i++) {
		CTAILQ_INIT(&negative_cache_table->list[i]);
	}
	for (i=0; i<MAX_UDP_FRAG_ENTRY; i++) {
		struct Negative_FragCache_Entry *entry = (struct Negative_FragCache_Entry *)kmalloc(sizeof(struct Negative_FragCache_Entry), GFP_ATOMIC);
		if (entry == NULL) {
			DEBUGP_SYS("MALLOC Failed! (Negative FragCache Table Entry) \n");
			return 0;
		}
		init_timer(&entry->timer);
		entry->timer.data = (unsigned long)entry;	/* pointer to queue	*/
		entry->timer.function = negative_cache_timeout;		/* expire function	*/		
		CTAILQ_INSERT_TAIL(&negative_cache_list_free, entry, tqe_link);
	}

	#if defined(CONFIG_SMP)
	spin_lock_init(&negative_cache_table->negative_frag_table_lock);
	#endif
	//panic_printk("sizeof(struct Negative_FragCache_Table)=%d\n",sizeof(struct Negative_FragCache_Table));
	//panic_printk("sizeof(struct Negative_FragCache_Entry)=%d\n", sizeof(struct Negative_FragCache_Entry));
	return 1;
}

static inline struct Negative_FragCache_Entry *find_negative_fragEntry(uint16 id,uint32 sip,uint32 dip,uint8 protocol)
{
	unsigned int hash = frag_hashfn(id, sip, dip, protocol);
	struct Negative_FragCache_Entry *entry;

	#if defined(CONFIG_SMP)
	spin_lock(&negative_cache_table->negative_frag_table_lock);
	#endif

	CTAILQ_FOREACH(entry, &negative_cache_table->list[hash], path_link) 
	{
		if ((entry->frag_id== id) && 			
			(entry->src_ip== sip) &&
			(entry->dst_ip== dip) &&
			(entry->protocol== protocol) && 
			(entry->status == FRAG_IN_KERNEL))
		{	
			#if defined(CONFIG_SMP)
			spin_unlock(&negative_cache_table->negative_frag_table_lock);
			#endif
			return ((struct Negative_FragCache_Entry *)entry);
		}
	}
	
	#if defined(CONFIG_SMP)
	spin_unlock(&negative_cache_table->negative_frag_table_lock);
	#endif
	return NULL;
}

static inline int add_negative_fragEntry(uint16 id, uint32 sip, uint32 dip, uint8 protocol)
{
	unsigned int hash = frag_hashfn(id, sip, dip, protocol);
	struct Negative_FragCache_Entry *entry;

	#if defined(CONFIG_SMP)
	spin_lock(&negative_cache_table->negative_frag_table_lock);
	#endif

	if(CTAILQ_EMPTY(&negative_cache_list_free))
	{
		//printk("add_negative_fragEntry table full ,fail!!!\n"); 
		#if defined(CONFIG_SMP)
		spin_unlock(&negative_cache_table->negative_frag_table_lock);
		#endif
		return 0;
	}
				
	entry = CTAILQ_FIRST(&negative_cache_list_free);
				
	entry->frag_id= id;
	entry->src_ip= sip;
	entry->dst_ip= dip;
	entry->protocol= protocol;
	entry->status= FRAG_IN_KERNEL;	
	CTAILQ_REMOVE(&negative_cache_list_free, entry, tqe_link);
	CTAILQ_INSERT_TAIL(&negative_cache_table->list[hash], entry, path_link);

	#if defined(CONFIG_SMP)
	spin_unlock(&negative_cache_table->negative_frag_table_lock);
	#endif

	entry->timer.expires = jiffies + FRAG_CACHE_TIMEOUT;
	add_timer(&entry->timer);	
	//panic_printk("add_negative_fragEntry:id=0x%X\n",entry->frag_id); 
	return 1;
}
#endif

#ifndef NO_ARP_USED
static uint32 FastPath_Hash_ARP_Entry(ipaddr_t ip)
{
	return (ip % 16);
}
#endif

#ifndef DEL_ROUTE_TBL
static uint32
FastPath_Hash_ROUTE_Entry(ipaddr_t ip, ipaddr_t mask)
{
	int i;
	ipaddr_t tmp = (ip & mask);

	for(i=0; i<32; i++) {
		if (tmp & 0x00000001) {
			return (tmp + (uint32)i) % route_table_list_max;
		}
		tmp = tmp >> 1;
	}

	return 0;
}
#endif

// david
#if 0
static uint32
FastPath_Hash_NAPT_Entry(ipaddr_t intIp,uint32 intPort,
			ipaddr_t extIp, uint32 extPort,
			ipaddr_t remIp, uint32 remPort, int protoocal)
{
	uint32 hash;

	hash = (0xff000000 & intIp) >> 24;
	hash ^= (0x00ff0000 & intIp) >> 16;
	hash ^= (0x0000ff00 & intIp) >> 8;
	hash ^= (0x000000ff & intIp);
	hash ^= (0x0000ff00 & intPort) >> 8;
	hash ^= (0x000000ff & intPort);

	hash ^= (0xff000000 & extIp) >> 24;
	hash ^= (0x00ff0000 & extIp) >> 16;
	hash ^= (0x0000ff00 & extIp) >> 8;
	hash ^= (0x000000ff & extIp);
	hash ^= (0x0000ff00 & extPort) >> 8;
	hash ^= (0x000000ff & extPort);

	hash ^= (0xff000000 & remIp) >> 24;
	hash ^= (0x00ff0000 & remIp) >> 16;
	hash ^= (0x0000ff00 & remIp) >> 8;
	hash ^= (0x000000ff & remIp);
	hash ^= (0x0000ff00 & remPort) >> 8;
	hash ^= (0x000000ff & remPort);

	return 0x000003ff & (hash ^ (hash >> 12));
}
#endif

#if	!defined(DEL_NAPT_TBL)
static uint32
FastPath_Hash_NAPT_Entry(ipaddr_t sip, uint32 sport, ipaddr_t dip, uint32 dport, uint16 protocol)
{
	register uint32 hash;

	hash = ((sip>>16)^sip);
	hash ^= ((dip>>16)^dip);
	hash ^= sport;
	hash ^= dport;
	hash ^= protocol;
	return ((napt_table_list_max - 1) & (hash ^ (hash >> 12)));
}
#endif

__IRAM_GEN inline static uint32
FastPath_Hash_PATH_Entry(ipaddr_t sip, uint32 sport, ipaddr_t dip, uint32 dport, uint16 protocol)
{
	register uint32 hash;

// david ------------------------------
#if 0
	hash = ((sip>>16)^sip);
	hash ^= ((dip>>16)^dip);
	hash ^= sport;
	hash ^= dport;
	return 0x000003ff & (hash ^ (hash >> 12));
#endif
	hash = ((sip>>8)^sip);
	hash ^= ((dip>>16)^dip);
	hash ^= sport>>4;
	hash ^= dport;
	hash ^= protocol;
	return (path_table_list_max-1) & (hash ^ (hash >> 12));

//-------------------------------------

}


/* ====================================================================== */

#if 0
enum LR_RESULT
rtk_addFdbEntry(uint32 vid,
		uint32 fid,
		ether_addr_t* mac,
		uint32 portmask,
		enum FDB_FLAGS flags)
{
	DEBUGP_API("addFdbEntry: vid=%u fid=%u mac=%p portmask=0x%08X flasg=0x%08X \n", vid, fid, mac, portmask, flags);

	return LR_SUCCESS;
}

enum LR_RESULT
rtk_delFdbEntry(uint32 vid,
		uint32 fid,
		ether_addr_t* mac)
{
	DEBUGP_API("delFdbEntry: vid=%u fid=%u mac=%p \n", vid, fid, mac);

	return LR_SUCCESS;
}
#endif
/*======================================================================= */
enum LR_RESULT
rtk_addArp(ipaddr_t ip,
		ether_addr_t* mac,
		enum ARP_FLAGS flags)
{
#ifndef NO_ARP_USED
	uint32 hash = FastPath_Hash_ARP_Entry(ip);
	struct Arp_List_Entry *ep;

	DEBUGP_API("addArp: ip=0x%08X mac=%02X:%02X:%02X:%02X:%02X:%02X flags=0x%08X Hash=%u \n", ip, MAC2STR(*mac), flags, hash);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_arp->list[hash], arp_link) {
		if (ep->ip == ip) {
		//	DEBUGP_SYS("addArp: ERROR - arp(ip=0x%08X) already exist! \n", ip);
			return LR_EXIST;
		}
	}

	/* Create */
	if(!CTAILQ_EMPTY(&arp_list_free)) {
		struct Arp_List_Entry *entry_arp;
		entry_arp = CTAILQ_FIRST(&arp_list_free);
		entry_arp->ip = ip;
		entry_arp->mac = *mac;
		entry_arp->flags = flags;
		entry_arp->vaild = 0xff;
		CTAILQ_REMOVE(&arp_list_free, entry_arp, tqe_link);
		CTAILQ_INSERT_TAIL(&arp_list_inuse, entry_arp, tqe_link);
		CTAILQ_INSERT_TAIL(&table_arp->list[hash], entry_arp, arp_link);
	} else {
		DEBUGP_SYS("addArp: ERROR - arp_list_free is empty! \n");
		return LR_FAILED;
	}
#endif
	return LR_SUCCESS;
}
/*======================================================================= */
enum LR_RESULT
rtk_modifyArp(ipaddr_t ip,
		ether_addr_t* mac,
		enum ARP_FLAGS flags)
{
#ifndef NO_ARP_USED
	uint32 hash = FastPath_Hash_ARP_Entry(ip);
	struct Arp_List_Entry *ep;

	DEBUGP_API("modifyArp: ip=0x%08X mac=%02X:%02X:%02X:%02X:%02X:%02X flags=0x%08X \n", ip, MAC2STR(*mac), flags);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_arp->list[hash], arp_link) {
		if (ep->ip == ip) {
			ep->mac = *mac;
			ep->flags = flags;
			return LR_SUCCESS;
		}
	}
#endif
	return LR_SUCCESS;
}
/*======================================================================= */
/*
	delArp() - Delete an entry of Arp Table.
*/
enum LR_RESULT
rtk_delArp(ipaddr_t ip)
{
#ifndef NO_ARP_USED
	uint32 hash = FastPath_Hash_ARP_Entry(ip);
	struct Arp_List_Entry *ep;

	DEBUGP_API("delArp: ip=0x%08X \n", ip);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_arp->list[hash], arp_link) {
		if (ep->ip == ip) {
			ep->vaild = RTL_FP_NAPT_INVALID;
			CTAILQ_REMOVE(&table_arp->list[hash], ep, arp_link);
			CTAILQ_REMOVE(&arp_list_inuse, ep, tqe_link);
			CTAILQ_INSERT_TAIL(&arp_list_free, ep, tqe_link);
			return LR_SUCCESS;
		}
	}
#endif
	return LR_NONEXIST;
}
/*======================================================================= */
enum LR_RESULT
rtk_addRoute(ipaddr_t ip,
		ipaddr_t mask,
		ipaddr_t gateway,
		uint8* ifname,
		enum RT_FLAGS flags)
{
#ifndef DEL_ROUTE_TBL
	uint32 hash = FastPath_Hash_ROUTE_Entry(ip, mask);

	DEBUGP_API("addRoute: ip=0x%08X mask=0x%08X gateway=0x%08X ifname=%s flags=0x%08X Hash=%u \n",
		ip, mask, gateway, ifname, flags, hash);
	if(!CTAILQ_EMPTY(&route_list_free)) {
		struct Route_List_Entry *entry_route;
		entry_route = CTAILQ_FIRST(&route_list_free);
		entry_route->ip = ip;
		entry_route->mask = mask;
		entry_route->gateway = gateway;
		memcpy(&entry_route->ifname, ifname, IFNAME_LEN_MAX - 1);
		entry_route->flags = flags;
		entry_route->vaild = 0xff;
		CTAILQ_REMOVE(&route_list_free, entry_route, tqe_link);
		CTAILQ_INSERT_TAIL(&route_list_inuse, entry_route, tqe_link);
		CTAILQ_INSERT_TAIL(&table_route->list[hash], entry_route, route_link);
	} else {
		DEBUGP_SYS("addRoute: ERROR - Route_list_free is empty! \n");
		return LR_FAILED;
	}
#endif

	return LR_SUCCESS;
}
/*======================================================================= */
enum LR_RESULT
rtk_modifyRoute(ipaddr_t ip,
		ipaddr_t mask,
		ipaddr_t gateway,
		uint8* ifname,
		enum RT_FLAGS flags)
{

#ifndef DEL_ROUTE_TBL
	uint32 hash = FastPath_Hash_ROUTE_Entry(ip, mask);
	struct Route_List_Entry *ep;

	DEBUGP_API("modifyRoute: ip=0x%08X mask=0x%08X gateway=0x%08X ifname=%s flags=0x%08X \n",
		ip, mask, gateway, ifname, flags);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_route->list[hash], route_link) {
		if (ep->ip == ip && ep->mask == mask) {
			ep->gateway = gateway;
			memcpy(&ep->ifname, ifname, IFNAME_LEN_MAX - 1);
			ep->flags = flags;
			CTAILQ_REMOVE(&table_route->list[hash], ep, route_link);
			CTAILQ_REMOVE(&route_list_inuse, ep, tqe_link);
			CTAILQ_INSERT_TAIL(&route_list_free, ep, tqe_link);
			return LR_SUCCESS;
		}
	}
#endif

	return LR_SUCCESS;
}
/*======================================================================= */
enum LR_RESULT
rtk_delRoute(ipaddr_t ip, ipaddr_t mask)
{

#ifndef DEL_ROUTE_TBL
	uint32 hash = FastPath_Hash_ROUTE_Entry(ip, mask);
	struct Route_List_Entry *ep;

	DEBUGP_API("delRoute: ip=0x%08X mask=0x%08X \n", ip, mask);
	/* Lookup */
	CTAILQ_FOREACH(ep, &table_route->list[hash], route_link) {
		if (ep->ip == ip && ep->mask == mask) {
			ep->vaild = RTL_FP_NAPT_INVALID;
			CTAILQ_REMOVE(&table_route->list[hash], ep, route_link);
			CTAILQ_REMOVE(&route_list_inuse, ep, tqe_link);
			CTAILQ_INSERT_TAIL(&route_list_free, ep, tqe_link);
			return LR_SUCCESS;
		}
	}
#endif

	return LR_NONEXIST;
}
/*======================================================================= */
enum LR_RESULT
rtk_addSession(uint8* ifname,
		enum SE_TYPE seType,
		uint32 sessionId,
		enum SE_FLAGS flags )
{
	return LR_SUCCESS;
}

enum LR_RESULT
rtk_delSession(uint8* ifname)
{
	return LR_SUCCESS;
}
/*======================================================================= */

enum LR_RESULT rtk_addNaptConnection(rtl_fp_napt_entry *fpNaptEntry,
#if defined(IMPROVE_QOS)
									void *pskb, void *ct,
#endif
									enum NP_FLAGS flags)
{
	uint32 hash;
#ifndef DEL_NAPT_TBL
	struct Napt_List_Entry *entry_napt;
#endif
	struct Path_List_Entry *entry_path;

	uint16 ipprotocol;
	unsigned long irq_flags;
	struct nf_conn* tmp = (struct nf_conn*)ct;
#if defined(IMPROVE_QOS)
	struct iphdr *iph;
	struct tcphdr *tcphupuh;  //just keep one , don't care tcp or udp //
	u_int ori_saddr, ori_daddr;
	u_short ori_sport, ori_dport;
	void *lanDev, *wanDev;
	unsigned char oriSrcMac[6],oriDstMac[6],resMac[14];
	u_short proto;
	unsigned char pppProto[2],ipProto[2];
	__u32 oriSkbMark;
	__u8  oriSrcPhyPort = 0;
	
#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
	unsigned char oriTos;
#endif
#if 1////defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK) && defined(IMPROVE_QOS)
	uint16 vlan_pri_bak;
#endif

	if(pskb==NULL)
		return LR_FAILED;
	/*bug fix for IMPROVE_QOS*/
	if(rtl_eth_hdr(pskb)==NULL)
	{
		printk("[%s-%u] eth is null\n", __func__, __LINE__);
		return LR_FAILED;
	}
#endif

	if (fpNaptEntry->protocol == NP_TCP)
		ipprotocol = IPPROTO_TCP;
	else
		ipprotocol = IPPROTO_UDP;

#if defined(CONFIG_RTL_FAST_GRE)
	if (fpNaptEntry->protocol == NP_GRE)		
		ipprotocol = IPPROTO_GRE;
#endif
	LOG_INFO("addNaptConnection: P=%s int=%u.%u.%u.%u:%u ext=%u.%u.%u.%u:%u rem=%u.%u.%u.%u:%u F=%d (Ha=%u, Hb=%u)\n",
		(fpNaptEntry->protocol==NP_TCP)? "TCP" : "UDP", NIPQUAD(fpNaptEntry->intIp), fpNaptEntry->intPort, NIPQUAD(fpNaptEntry->extIp), fpNaptEntry->extPort, NIPQUAD(fpNaptEntry->remIp), fpNaptEntry->remPort, flags,
		FastPath_Hash_PATH_Entry(fpNaptEntry->intIp, fpNaptEntry->intPort, fpNaptEntry->remIp, fpNaptEntry->remPort, ipprotocol), FastPath_Hash_PATH_Entry(fpNaptEntry->remIp, fpNaptEntry->remPort, fpNaptEntry->extIp, fpNaptEntry->extPort,ipprotocol));

	filter_addconnect(fpNaptEntry->remIp);

#if	!defined(DEL_NAPT_TBL)
	local_irq_save(irq_flags);
	hash = FastPath_Hash_NAPT_Entry(fpNaptEntry->intIp, fpNaptEntry->intPort, fpNaptEntry->remIp, fpNaptEntry->remPort, (uint16)(fpNaptEntry->protocol));
	#if defined(CONFIG_SMP)
	spin_lock(&table_napt->napt_table_lock);
	#endif
	/* Lookup */
	CTAILQ_FOREACH(entry_napt, &table_napt->list[hash], napt_link) {
		if ((entry_napt->protocol == fpNaptEntry->protocol) &&
			(entry_napt->intIp == fpNaptEntry->intIp) &&
			(entry_napt->intPort == fpNaptEntry->intPort) &&
			(entry_napt->extIp == fpNaptEntry->extIp) &&
			(entry_napt->extPort == fpNaptEntry->extPort) &&
			(entry_napt->remIp == fpNaptEntry->remIp) &&
			(entry_napt->remPort == fpNaptEntry->remPort)) {
			//DEBUGP_SYS("addNaptConnection: ERROR - the entry already exist! \n");
			rtl_conntrack_drop_check_hook(tmp, ipprotocol, ct);

			#if defined(CONFIG_SMP)
			spin_unlock(&table_napt->napt_table_lock);
			#endif
			local_irq_restore(irq_flags);
			return LR_SUCCESS;
		}
	}

	if(!CTAILQ_EMPTY(&napt_list_free)) {
		entry_napt = CTAILQ_FIRST(&napt_list_free);
		entry_napt->protocol = fpNaptEntry->protocol;
		entry_napt->intIp = fpNaptEntry->intIp;
		entry_napt->intPort = fpNaptEntry->intPort;
		entry_napt->extIp = fpNaptEntry->extIp;
		entry_napt->extPort = fpNaptEntry->extPort;
		entry_napt->remIp = fpNaptEntry->remIp;
		entry_napt->remPort = fpNaptEntry->remPort;
		entry_napt->flags = flags;
		entry_napt->vaild = 0xff;
		CTAILQ_REMOVE(&napt_list_free, entry_napt, tqe_link);
		CTAILQ_INSERT_TAIL(&napt_list_inuse, entry_napt, tqe_link);
		CTAILQ_INSERT_TAIL(&table_napt->list[hash], entry_napt, napt_link);
#else
	local_irq_save(irq_flags);
	if (1) {
#endif
	#if defined(IMPROVE_QOS)
		//initial
		pppProto[0]=0x00;
		pppProto[1]=0x21;
		ipProto[0]=0x08;
		ipProto[1]=0x00;
#if defined(CONFIG_RTL_819X_SWCORE)
		lanDev = (void*)rtl865x_getLanDev();
		wanDev = (void*)rtl865x_getWanDev();
#endif		
		proto = ntohs(rtl_get_skb_protocol(pskb));
		iph = rtl_ip_hdr(pskb);
		tcphupuh = (struct tcphdr*)((__u32 *)iph + iph->ihl);

		//To bak origal protol mac
		memcpy(oriSrcMac,rtl_eth_hdr(pskb)->h_source,ETH_ALEN);
		memcpy(oriDstMac,rtl_eth_hdr(pskb)->h_dest,ETH_ALEN);
		
#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
		oriTos = iph->tos;
		iph->tos = (iph->tos &0x3)|((rtl_get_skb_orig_dscp(pskb)) << 2);
#endif

#if 1////defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK)
		vlan_pri_bak = rtl_get_skb_remark_vlanpri(pskb);
		rtl_set_skb_ori_vlanpri(pskb);
#endif

		oriSrcPhyPort = rtl_get_skb_phy_port(pskb);

		//Bak orignal skb mark
		oriSkbMark = rtl_get_skb_mark(pskb);

		//check ip-based qos rule at iptables mangle table
		//To record original info
		ori_saddr=iph->saddr;
		ori_sport=tcphupuh->source;
		ori_daddr=iph->daddr;
		ori_dport=tcphupuh->dest;

		/* for dst mac match, please refer to the xt_mac.c */
		rtl_store_skb_dst(pskb);
	#endif
		#if defined(CONFIG_SMP)
		spin_lock(&table_path->path_table_lock);
		#endif
		/*===================add Path Table Entry=====================*/
		/* course = 1 (Outbound) */
		hash = FastPath_Hash_PATH_Entry(fpNaptEntry->intIp, fpNaptEntry->intPort, fpNaptEntry->remIp, fpNaptEntry->remPort, ipprotocol);


#if defined(CONFIG_RTL_FAST_GRE)
		if(ipprotocol == IPPROTO_GRE)
			hash = FastPath_Hash_PATH_Entry(fpNaptEntry->intIp, 0, fpNaptEntry->remIp, fpNaptEntry->remPort, ipprotocol);		
#endif				

		if(!CTAILQ_EMPTY(&path_list_free)) {    //brad modify 2007-11-27

		entry_path = CTAILQ_FIRST(&path_list_free);

		entry_path->protocol	= fpNaptEntry->protocol;
		entry_path->in_sIp	= fpNaptEntry->intIp;
		entry_path->in_sPort	= fpNaptEntry->intPort;
		entry_path->in_dIp	= fpNaptEntry->remIp;
		entry_path->in_dPort	= fpNaptEntry->remPort;
		entry_path->out_sIp	= fpNaptEntry->extIp;
		entry_path->out_sPort	= fpNaptEntry->extPort;
		entry_path->out_dIp	= fpNaptEntry->remIp;
		entry_path->out_dPort	= fpNaptEntry->remPort;

		entry_path->out_ifname	= FastPath_Route(entry_path->out_dIp);
#ifndef NO_ARP_USED
		entry_path->arp_entry	= NULL;
#endif
		entry_path->course		= 1;
		entry_path->vaild		= RTL_FP_NAPT_VALID;
//		entry_path->dst			= NULL;
		entry_path->type		= 0;	/* Init: Normal (Only Routing) */
        entry_path->ct      = ct;
		if(rtl_init_bypass_packet_hook)
		    rtl_init_bypass_packet_hook(entry_path);
		if (entry_path->in_sIp != entry_path->out_sIp) {
			entry_path->type |= 1;	// SNAT
		}
#if defined(CONFIG_RTL_FAST_GRE)		
		if(ipprotocol !=IPPROTO_GRE)
#endif
		if (entry_path->in_sPort != entry_path->out_sPort) {
			entry_path->type |= 2;	// SNPT
		}

#if defined(IMPROVE_QOS)
#if defined(CONFIG_NET_SCHED)
		entry_path->PreMark = 0;
		entry_path->PostMark= 0;
#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
		entry_path->policy_route_mark = 0;
#endif
#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK)
          entry_path->dscp_in = 0;
       	   entry_path->dscp_out = 0;
#endif
#if 1//defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK)
	   entry_path->vlanpri_out = 0xff;
#endif
		{
			//Replace source addr to check uplink mark
			iph->saddr=fpNaptEntry->intIp;
			tcphupuh->source=fpNaptEntry->intPort;
			iph->daddr=fpNaptEntry->remIp;
			tcphupuh->dest=fpNaptEntry->remPort;

			memset(resMac,0,14);
			rtl_set_skb_mark(pskb, 0);
			if((lanDev!=NULL)&&(rtl_arp_req_get_ha(fpNaptEntry->intIp,lanDev,resMac)==0))
			{
				//Patch for pppoe wantype: run udp chariot
				//bak skb mac header
				if((rtl_ppp_proto_check(pskb, pppProto) == 1)	//equal 0x0021
					&&(rtl_skb_mac_header_was_set(pskb)==1)
					&&(rtl_eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
				{
						rtl_skb_set_mac_header(pskb, -22);
				}

				//Replace source mac addr to check uplink mark
				memcpy(rtl_eth_hdr(pskb)->h_source,resMac, ETH_ALEN);
				rtl_set_skb_dmac(pskb, lanDev);
				rtl_set_skb_src_phy_port(pskb, resMac,0);
		
				if(proto == ETH_P_IP){
				#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
					if (rtl_hook_is_empty(pskb, PF_INET, NF_INET_PRE_ROUTING) == 0)
						rtl_ipt_do_table(pskb, NF_INET_PRE_ROUTING, lanDev,wanDev);
					entry_path->PreMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PreMark;
					#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
					entry_path->policy_route_mark = rtl_get_skb_mark(pskb);
					#endif
					//rtl_set_skb_mark(pskb, 0);
					if (rtl_hook_is_empty(pskb, PF_INET, NF_INET_POST_ROUTING) == 0)
						rtl_ipt_do_table(pskb, NF_INET_POST_ROUTING, lanDev, wanDev);
					
					entry_path->PostMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PostMark;
				#else
                #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
					(list_empty(&nf_hooks[PF_INET][NF_INET_PRE_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_INET_PRE_ROUTING, lanDev,wanDev);
					entry_path->PreMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PreMark;
					#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
					entry_path->policy_route_mark = rtl_get_skb_mark(pskb);
					#endif
					//rtl_set_skb_mark(pskb, 0);
					(list_empty(&nf_hooks[PF_INET][NF_INET_POST_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_INET_POST_ROUTING, lanDev, wanDev);
					
					entry_path->PostMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PostMark;
				#else
					(list_empty(&nf_hooks[PF_INET][NF_IP_PRE_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_IP_PRE_ROUTING, lanDev,wanDev);
					entry_path->PreMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PreMark;
					#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
					entry_path->policy_route_mark = rtl_get_skb_mark(pskb);
					#endif
					//rtl_set_skb_mark(pskb, 0);
					(list_empty(&nf_hooks[PF_INET][NF_IP_POST_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_IP_POST_ROUTING, lanDev, wanDev);
					
					entry_path->PostMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PostMark;
				#endif
				#endif
				}
			}


			/*LOG_INFO("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
							lanDev?lanDev->name:"NULL",
							wanDev?wanDev->name:"NULL",
							pskb->inDev?pskb->inDev->name:"NULL",
							pskb->dev?pskb->dev->name:"NULL", pskb->mark);*/
			

			//Replace dest addr to check uplink mark
			iph->saddr=fpNaptEntry->extIp;
			tcphupuh->source=fpNaptEntry->extPort;
			iph->daddr=fpNaptEntry->remIp;
			tcphupuh->dest=fpNaptEntry->remPort;

			memset(resMac,0,14);
			rtl_set_skb_mark(pskb, 0);
			if((wanDev!=NULL)&&(rtl_arp_req_get_ha(fpNaptEntry->remIp,wanDev,resMac)==0))
			{
				//Patch for pppoe wantype: run udp chariot
				//bak skb mac header
				if((rtl_ppp_proto_check(pskb, pppProto) == 1)	//equal 0x0021
					&&(rtl_skb_mac_header_was_set(pskb)==1)
					&&(rtl_eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
				{
						rtl_skb_set_mac_header(pskb, -22);
				}

				//Replace source mac addr to check uplink mark
				memcpy(rtl_eth_hdr(pskb)->h_dest,resMac, ETH_ALEN);
				rtl_set_skb_smac(pskb, wanDev);
		

		
				if(proto == ETH_P_IP){
					#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
					if(entry_path->PreMark)
						rtl_set_skb_mark(pskb, entry_path->PreMark);
					if (rtl_hook_is_empty(pskb, PF_INET, NF_INET_POST_ROUTING) == 0)
						rtl_ipt_do_table(pskb, NF_INET_POST_ROUTING, lanDev, wanDev);
					#else
                    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
					if(entry_path->PreMark)
						rtl_set_skb_mark(pskb, entry_path->PreMark);
					(list_empty(&nf_hooks[PF_INET][NF_INET_POST_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_INET_POST_ROUTING, lanDev, wanDev);

					#else
					(list_empty(&nf_hooks[PF_INET][NF_IP_POST_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_IP_POST_ROUTING, lanDev, wanDev);
					#endif
					#endif

					entry_path->PostMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PostMark;
				}
				/*LOG_INFO("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
								lanDev?lanDev->name:"NULL",
								wanDev?wanDev->name:"NULL",
								pskb->inDev?pskb->inDev->name:"NULL",
								pskb->dev?pskb->dev->name:"NULL", pskb->mark);*/
			}
			#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
			rtl_set_entry_dscp_out(entry_path,pskb);
			#endif
			
			#if 1//defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK) && defined(IMPROVE_QOS)		
			rtl_set_entry_vlanpri(entry_path,pskb);
			#endif 
			
			#if 1 //defined(CONFIG_RTL_VLANID_IPTABLE_CHECK) && defined(IMPROVE_QOS)
			rtl_set_entry_vlanid_out(entry_path, pskb);
			#endif

		}
#endif
		if(ipprotocol==IPPROTO_TCP)
			entry_path->timeout = rtl_tcp_get_timeouts(ct);
		else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
			entry_path->timeout = udp_get_timeouts_by_state(UDP_CT_REPLIED,(void *)ct,1);
#else
            #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
			entry_path->timeout = udp_get_timeouts_by_state(UDP_CT_REPLIED);
			#else
			entry_path->timeout = nf_ct_udp_timeout_stream;
			#endif
#endif
		}
#if defined(CONFIG_RTL_FAST_GRE)
	if(ipprotocol == IPPROTO_GRE)
	{		
		entry_path->timeout = GRE_STREAM_TIMEOUT;
	}
#endif

		entry_path->ct = ct;
#endif
		entry_path->last_used=jiffies;

		LOG_INFO("addNaptConnection0: proto[%d] src[0x%x:%d] ext[0x%x:%d] dst[0x%x:%d] mark[%d:%d].\n"
			, ipprotocol, fpNaptEntry->intIp, fpNaptEntry->intPort, fpNaptEntry->extIp, fpNaptEntry->extPort, fpNaptEntry->remIp, fpNaptEntry->remPort, entry_path->PreMark, entry_path->PostMark);
		CTAILQ_REMOVE(&path_list_free, entry_path, tqe_link);
		CTAILQ_INSERT_TAIL(&path_list_inuse, entry_path, tqe_link);
		CTAILQ_INSERT_TAIL(&table_path->list[hash], entry_path, path_link);
		}//else{
		//	printk("<<<----DEBUG course 1 got NULL ---->>>>>>\n");
		//}
		/* course = 2 (Inbound) */
		hash = FastPath_Hash_PATH_Entry(fpNaptEntry->remIp, fpNaptEntry->remPort, fpNaptEntry->extIp, fpNaptEntry->extPort, ipprotocol);

#if defined(CONFIG_RTL_FAST_GRE)
		if(ipprotocol == IPPROTO_GRE)
			hash = FastPath_Hash_PATH_Entry(fpNaptEntry->remIp, 0, fpNaptEntry->extIp, fpNaptEntry->extPort, ipprotocol);
#endif				
		if(!CTAILQ_EMPTY(&path_list_free)) {    //brad modify 2007-11-27
		entry_path = CTAILQ_FIRST(&path_list_free);
		entry_path->protocol	= fpNaptEntry->protocol;
		entry_path->in_sIp	= fpNaptEntry->remIp;
		entry_path->in_sPort	= fpNaptEntry->remPort;
		entry_path->in_dIp	= fpNaptEntry->extIp;
		entry_path->in_dPort	= fpNaptEntry->extPort;
		entry_path->out_sIp	= fpNaptEntry->remIp;
		entry_path->out_sPort	= fpNaptEntry->remPort;
		entry_path->out_dIp	= fpNaptEntry->intIp;
		entry_path->out_dPort	= fpNaptEntry->intPort;

		entry_path->out_ifname	= FastPath_Route(entry_path->out_dIp);
#ifndef NO_ARP_USED
		entry_path->arp_entry	= NULL;
#endif
		entry_path->course	= 2;
		entry_path->vaild	= RTL_FP_NAPT_VALID;
//		entry_path->dst		= NULL;
		entry_path->type	= 0;	/* Init: Normal (Only Routing) */
        entry_path->ct      = ct;
		if(rtl_init_bypass_packet_hook)
		    rtl_init_bypass_packet_hook(entry_path);
		if (entry_path->in_dIp != entry_path->out_dIp) {
			entry_path->type |= 4;	// DNAT
		}
		if (entry_path->in_dPort != entry_path->out_dPort) {
			entry_path->type |= 8;	// DNPT
		}

#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
		iph->tos = (iph->tos &0x3)|((rtl_get_skb_orig_dscp(pskb)) << 2);
#endif


#if defined(IMPROVE_QOS)
#if defined(CONFIG_NET_SCHED)
		entry_path->PreMark = 0;
		entry_path->PostMark= 0;
#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
		entry_path->policy_route_mark = 0;
#endif
#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK)
          entry_path->dscp_in = 0;
       	   entry_path->dscp_out = 0;
#endif
#if 1//defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK)
	   entry_path->vlanpri_out = 0xff;
#endif
		{
			//Replace source addr to check uplink mark
			iph->saddr=fpNaptEntry->remIp;
			tcphupuh->source=fpNaptEntry->remPort;
			iph->daddr=fpNaptEntry->extIp;
			tcphupuh->dest=fpNaptEntry->extPort;

			memset(resMac,0,14);
			rtl_set_skb_mark(pskb, 0);
			
#if 1////defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK)
			rtl_set_skb_ori_vlanpri(pskb);
#endif
			if((wanDev!=NULL)&&(rtl_arp_req_get_ha(fpNaptEntry->remIp,wanDev,resMac)==0))
			{
				//Patch for pppoe wantype: run udp chariot
				if((rtl_ppp_proto_check(pskb, pppProto) == 1)	//equal 0x0021
					&&(rtl_skb_mac_header_was_set(pskb)==1)
					&&(rtl_eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
				{
					rtl_skb_set_mac_header(pskb, -22);
				}

				//Replace source mac addr to check uplink mark
				memcpy(rtl_eth_hdr(pskb)->h_source,resMac, ETH_ALEN);
				rtl_set_skb_dmac(pskb, wanDev);

				rtl_set_skb_phy_port(pskb, oriSrcPhyPort);
				rtl_set_skb_src_phy_port(pskb, resMac,1);				

				if(proto == ETH_P_IP){
					#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
					if (rtl_hook_is_empty(pskb, PF_INET, NF_INET_PRE_ROUTING) == 0)
						rtl_ipt_do_table(pskb, NF_INET_PRE_ROUTING, wanDev, lanDev);
					#else
                    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
					(list_empty(&nf_hooks[PF_INET][NF_INET_PRE_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_INET_PRE_ROUTING, wanDev, lanDev);

					#else
					(list_empty(&nf_hooks[PF_INET][NF_IP_PRE_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_IP_PRE_ROUTING, wanDev, lanDev);
					#endif
					#endif
				}
				/*LOG_INFO("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
								lanDev?lanDev->name:"NULL",
								wanDev?wanDev->name:"NULL",
								pskb->inDev?pskb->inDev->name:"NULL",
								pskb->dev?pskb->dev->name:"NULL", pskb->mark);*/
				entry_path->PreMark = rtl_get_skb_mark(pskb)? rtl_get_skb_mark(pskb):entry_path->PreMark;
			}

			

			//Replace dest addr to check uplink mark
			iph->saddr=fpNaptEntry->remIp;
			tcphupuh->source=fpNaptEntry->remPort;
			iph->daddr=fpNaptEntry->intIp;
			tcphupuh->dest=fpNaptEntry->intPort;

			memset(resMac,0,14);
			rtl_set_skb_mark(pskb, 0);
			if ((lanDev!=NULL)&&(rtl_arp_req_get_ha(fpNaptEntry->intIp,lanDev,resMac)==0))
			{
				//Patch for pppoe wantype: run udp chariot
				if((rtl_ppp_proto_check(pskb, pppProto) == 1)	//equal 0x0021
					&&(rtl_skb_mac_header_was_set(pskb)==1)
					&&(rtl_eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
				{
					rtl_skb_set_mac_header(pskb, -22);
				}
				//Replace dest mac addr and  hh data mac to check uplink mark
				memcpy(rtl_eth_hdr(pskb)->h_dest,resMac,ETH_ALEN);
				rtl_set_skb_smac(pskb, lanDev);
			

				if(proto == ETH_P_IP){
					#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
					if(entry_path->PreMark)
						rtl_set_skb_mark(pskb, entry_path->PreMark);
					if (rtl_hook_is_empty(pskb, PF_INET, NF_INET_POST_ROUTING) == 0)
						rtl_ipt_do_table(pskb, NF_INET_POST_ROUTING, wanDev, lanDev);
					#else
                    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
					if(entry_path->PreMark)
						rtl_set_skb_mark(pskb, entry_path->PreMark);
					(list_empty(&nf_hooks[PF_INET][NF_INET_POST_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_INET_POST_ROUTING, wanDev, lanDev);
					#else
					(list_empty(&nf_hooks[PF_INET][NF_IP_POST_ROUTING]))?: \
						rtl_ipt_do_table(pskb, NF_IP_POST_ROUTING, wanDev, lanDev);
					#endif
					#endif
				}
				/*LOG_INFO("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
								lanDev?lanDev->name:"NULL",
								wanDev?wanDev->name:"NULL",
								pskb->inDev?pskb->inDev->name:"NULL",
								pskb->dev?pskb->dev->name:"NULL", pskb->mark);*/
			}
			entry_path->PostMark= rtl_get_skb_mark(pskb);
			#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK)
			rtl_set_entry_dscp_in(entry_path,pskb);
			#endif
			
			#if 1 //defined(CONFIG_RTL_VLANID_IPTABLE_CHECK) && defined(IMPROVE_QOS)
			rtl_set_entry_vlanid_in(entry_path, pskb);
			#endif
		}
#endif

		if(ipprotocol==IPPROTO_TCP)
			entry_path->timeout = rtl_tcp_get_timeouts(ct);
		else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
			entry_path->timeout = udp_get_timeouts_by_state(UDP_CT_REPLIED,(void *)ct,1);
#else
            #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
			entry_path->timeout = udp_get_timeouts_by_state(UDP_CT_REPLIED);
			#else
			entry_path->timeout = nf_ct_udp_timeout_stream;
			#endif
#endif
		}
#if defined(CONFIG_RTL_FAST_GRE)
		if(ipprotocol == IPPROTO_GRE)
		{
			entry_path->timeout = GRE_STREAM_TIMEOUT;
		}
#endif

		entry_path->ct = ct;
#endif
		entry_path->last_used=jiffies;

		LOG_INFO("addNaptConnection1: proto[%d] src[0x%x:%d] ext[0x%x:%d] dst[0x%x:%d] mark[%d:%d].\n"
			, ipprotocol, fpNaptEntry->remIp, fpNaptEntry->remPort, fpNaptEntry->extIp, fpNaptEntry->extPort, fpNaptEntry->intIp, fpNaptEntry->intPort, entry_path->PreMark, entry_path->PostMark);

		CTAILQ_REMOVE(&path_list_free, entry_path, tqe_link);
		CTAILQ_INSERT_TAIL(&path_list_inuse, entry_path, tqe_link);
		CTAILQ_INSERT_TAIL(&table_path->list[hash], entry_path, path_link);
		}//else{
		//	printk("<<<----DEBUG course 2 got NULL ---->>>>>>\n");
		//}
		/*===================add Path Table Entry=====================*/
	#if defined(IMPROVE_QOS)
		//Back to orignal protol mac
		memcpy(rtl_eth_hdr(pskb)->h_source,oriSrcMac, ETH_ALEN);
		memcpy(rtl_eth_hdr(pskb)->h_dest,oriDstMac, ETH_ALEN);

		//Back to original skb mark
		rtl_set_skb_mark(pskb, oriSkbMark);

		//back to original info
		iph->saddr=ori_saddr;
		tcphupuh->source=ori_sport;
		iph->daddr=ori_daddr;
		tcphupuh->dest=ori_dport;

		rtl_set_skb_dst(pskb);
		
		rtl_set_skb_phy_port(pskb, oriSrcPhyPort);

		
#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
		iph->tos = oriTos;
#endif

#if 1////defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK)
		rtl_restore_skb_vlanpri(pskb, vlan_pri_bak);
#endif
		if(lanDev)
			rtl_dev_put(lanDev);

		if(wanDev)
			rtl_dev_put(wanDev);
	#endif
		#if defined(CONFIG_SMP)
		spin_unlock(&table_path->path_table_lock);
		#if	!defined(DEL_NAPT_TBL)
		spin_unlock(&table_napt->napt_table_lock);
		#endif
		#endif
		local_irq_restore(irq_flags);
	} else {
		printk("addNaptConnection: ERROR - Napt_list_free is empty! \n");
		#if	!defined(DEL_NAPT_TBL)&&defined(CONFIG_SMP)
		spin_unlock(&table_napt->napt_table_lock);
		#endif
		local_irq_restore(irq_flags);
		return LR_FAILED;
	}

	rtl_conntrack_drop_check_hook(tmp, ipprotocol, ct);

	return LR_SUCCESS;
}
/*======================================================================= */
enum LR_RESULT
rtk_delNaptConnection(rtl_fp_napt_entry *fpNaptEntry)
{
// david
//	uint32 hash = FastPath_Hash_NAPT_Entry(intIp, intPort, extIp, extPort, remIp, remPort);
	uint32	hash;
#ifndef DEL_NAPT_TBL
	struct Napt_List_Entry *entry_napt;
#endif
	struct Path_List_Entry *entry_path;
	uint16 ipprotocol;
	unsigned long irq_flags;

	LOG_INFO("delNaptConnection: P=%s int=%u.%u.%u.%u:%u ext=%u.%u.%u.%u:%u rem=%u.%u.%u.%u:%u \n",
		(fpNaptEntry->protocol==NP_TCP)? "TCP" : "UDP", NIPQUAD(fpNaptEntry->intIp), fpNaptEntry->intPort, NIPQUAD(fpNaptEntry->extIp), fpNaptEntry->extPort, NIPQUAD(fpNaptEntry->remIp), fpNaptEntry->remPort);

	if (fpNaptEntry->protocol == NP_TCP)
		ipprotocol = IPPROTO_TCP;
	else
		ipprotocol = IPPROTO_UDP;

#if  defined(CONFIG_RTL_FAST_GRE)
	if (fpNaptEntry->protocol == NP_GRE)
		ipprotocol = IPPROTO_GRE;
#endif

	local_irq_save(irq_flags);

	filter_delconnect(fpNaptEntry->remIp);
#ifndef DEL_NAPT_TBL
	#if defined(CONFIG_SMP)
	spin_lock(&table_napt->napt_table_lock);
	#endif
	hash = FastPath_Hash_NAPT_Entry(fpNaptEntry->intIp, fpNaptEntry->intPort, fpNaptEntry->remIp, fpNaptEntry->remPort, (uint16)(fpNaptEntry->protocol));
	/* Lookup */
	CTAILQ_FOREACH(entry_napt, &table_napt->list[hash], napt_link) {
		if ((entry_napt->protocol == fpNaptEntry->protocol) &&
			(entry_napt->intIp == fpNaptEntry->intIp) &&
			(entry_napt->intPort == fpNaptEntry->intPort) &&
			(entry_napt->extIp == fpNaptEntry->extIp) &&
			(entry_napt->extPort == fpNaptEntry->extPort) &&
			(entry_napt->remIp == fpNaptEntry->remIp) &&
			(entry_napt->remPort == fpNaptEntry->remPort)) {
			entry_napt->vaild = RTL_FP_NAPT_INVALID;
			CTAILQ_REMOVE(&table_napt->list[hash], entry_napt, napt_link);
			CTAILQ_REMOVE(&napt_list_inuse, entry_napt, tqe_link);
			CTAILQ_INSERT_TAIL(&napt_list_free, entry_napt, tqe_link);
#else
	if (1) { {
#endif
			/* del Path Table Entry */
			#if defined(CONFIG_SMP)
			spin_lock(&table_path->path_table_lock);
			#endif
			/* course = 1 (Outbound) */
			hash = FastPath_Hash_PATH_Entry(fpNaptEntry->intIp, fpNaptEntry->intPort, fpNaptEntry->remIp, fpNaptEntry->remPort, ipprotocol);
#if  defined(CONFIG_RTL_FAST_GRE)
			if(ipprotocol == IPPROTO_GRE)
				hash = FastPath_Hash_PATH_Entry(fpNaptEntry->intIp,0, fpNaptEntry->remIp, fpNaptEntry->remPort, ipprotocol);				
#endif						
			CTAILQ_FOREACH(entry_path, &table_path->list[hash], path_link) {

// Remove entry only when ip/port are matched ---------------------
//				if ((entry_path->protocol == ep->protocol) && (entry_path->course == 1)){
				if ((entry_path->protocol == fpNaptEntry->protocol) &&
					(entry_path->course == 1)	&&
					(entry_path->in_sIp == fpNaptEntry->intIp) &&
					(entry_path->in_sPort == fpNaptEntry->intPort) &&
					(entry_path->out_sIp == fpNaptEntry->extIp) &&
					(entry_path->out_sPort == fpNaptEntry->extPort) &&
					(entry_path->in_dIp == fpNaptEntry->remIp) &&
					(entry_path->in_dPort == fpNaptEntry->remPort)) {

//					if (entry_path->dst) {//brad go back original 20080813
//						dst_release(entry_path->dst);	 //original
//						//entry_path->dst->dst_cache = NULL;
//						entry_path->dst = NULL;
//					}
//-------------------------------------------- david+2007-05-28

					entry_path->vaild = RTL_FP_NAPT_INVALID;
					CTAILQ_REMOVE(&table_path->list[hash], entry_path, path_link);
					CTAILQ_REMOVE(&path_list_inuse, entry_path, tqe_link);
					CTAILQ_INSERT_TAIL(&path_list_free, entry_path, tqe_link);
					break;
				}
			}

			/* course = 2 (Inbound) */
			hash = FastPath_Hash_PATH_Entry(fpNaptEntry->remIp, fpNaptEntry->remPort, fpNaptEntry->extIp, fpNaptEntry->extPort, ipprotocol);
#if  defined(CONFIG_RTL_FAST_GRE)
			if(ipprotocol == IPPROTO_GRE)
				hash = FastPath_Hash_PATH_Entry(fpNaptEntry->remIp, 0, fpNaptEntry->extIp, fpNaptEntry->extPort, ipprotocol);
#endif						

			CTAILQ_FOREACH(entry_path, &table_path->list[hash], path_link) {

// Remove entry only when ip/port are matched ---------------------
//				if ((entry_path->protocol == ep->protocol) && (entry_path->course == 2)){
				if ((entry_path->protocol == fpNaptEntry->protocol) &&
					(entry_path->course == 2)	&&
					(entry_path->in_dIp == fpNaptEntry->extIp) &&
					(entry_path->in_dPort == fpNaptEntry->extPort) &&
					(entry_path->out_sIp == fpNaptEntry->remIp)  &&
					(entry_path->out_sPort == fpNaptEntry->remPort) &&
					(entry_path->out_dIp== fpNaptEntry->intIp) &&
					(entry_path->out_dPort == fpNaptEntry->intPort)) {

//					if (entry_path->dst) { //Brad go back original
//						dst_release(entry_path->dst);		 //original
//						//entry_path->dst->dst_cache = NULL;
//						entry_path->dst = NULL;
//					}
//-------------------------------------------- david+2007-05-28
					entry_path->vaild = RTL_FP_NAPT_INVALID;
					CTAILQ_REMOVE(&table_path->list[hash], entry_path, path_link);
					CTAILQ_REMOVE(&path_list_inuse, entry_path, tqe_link);
					CTAILQ_INSERT_TAIL(&path_list_free, entry_path, tqe_link);
					break;
				}
			}
			#if defined(CONFIG_SMP)
			spin_unlock(&table_path->path_table_lock);
			#if !defined(DEL_NAPT_TBL)
			spin_unlock(&table_napt->napt_table_lock);
			#endif
			#endif
			local_irq_restore(irq_flags);
			return LR_SUCCESS;
		}
	}
#if !defined(DEL_NAPT_TBL)&&defined(CONFIG_SMP)
	spin_unlock(&table_napt->napt_table_lock);
#endif
	local_irq_restore(irq_flags);
	return LR_NONEXIST;
}

//#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
#if 0
#define	RTL_FP_REFRESH_INTERVAL		(HZ<<3)	/* 8 second */
unsigned long		rtl_OSConnection_refresh_time=0;

enum LR_RESULT rtk_refreshOSConnectionTimer(void)
{
	unsigned long			last_used, interval, *first_used, *latest_used;
	unsigned long			udp_first_used[2], tcp_first_used[TCP_CONNTRACK_IGNORE];
	unsigned long			udp_last_used[2], tcp_last_used[TCP_CONNTRACK_IGNORE];
	struct nf_conn 		*ct;
	struct Path_List_Entry	*entry_path;
	int					i, newstate;
	struct list_head		*state_hash;

	if (CTAILQ_EMPTY(&path_list_inuse)||
		(CTAILQ_TOTAL(&path_list_inuse)==0))
			return LR_SUCCESS;

	if (time_before(rtl_OSConnection_refresh_time, jiffies)) {
		rtl_OSConnection_refresh_time = jiffies+RTL_FP_REFRESH_INTERVAL;

		interval = nf_ct_udp_timeout_stream;
		for (i=0;i<2;i++) {
			udp_first_used[i] = jiffies;
			udp_last_used[i] = rtl_OSConnection_refresh_time+(interval>>2);
		}
		for (i=0;i<TCP_CONNTRACK_MAX;i++) {
			interval = tcp_get_timeouts_by_state(i);
			tcp_first_used[i] = jiffies;
			tcp_last_used[i] = rtl_OSConnection_refresh_time+(interval>>2);
		}
	} else {
		return LR_SUCCESS;
	}

	{
		write_lock_bh(&nf_conntrack_lock);
		CTAILQ_FOREACH(entry_path, &path_list_inuse, path_link) {
			if (entry_path->vaild == RTL_FP_NAPT_INVALID)
				continue;

			last_used = entry_path->last_used;
			ct = entry_path->ct;

			if (entry_path->protocol == NP_UDP)	{
				interval = nf_ct_udp_timeout_stream;
				if(ct->status & IPS_SEEN_REPLY)
					newstate = 1;
				else
					newstate = 0;
				state_hash = Udp_State_Hash_Head[newstate].state_hash;
				first_used = &udp_first_used[newstate];
				latest_used = &udp_last_used[newstate];
			} else {
				newstate = ct->proto.tcp.state;
				interval = tcp_get_timeouts_by_state(newstate);
				state_hash = Tcp_State_Hash_Head[newstate].state_hash;
				first_used = &tcp_first_used[newstate];
				latest_used = &tcp_last_used[newstate];
			}


			if (time_before((ct->timeout.expires), last_used+interval))	{
				mod_timer(&ct->timeout, last_used+interval);
			}

			if (time_before(*latest_used, last_used)) {
				*latest_used = last_used;
				list_move_tail(&ct->state_tuple, state_hash);
			}

			if (time_after(*first_used, last_used)) {
				*first_used = last_used;
				list_move(&ct->state_tuple, state_hash);
			}
		}
		write_unlock_bh(&nf_conntrack_lock);
	}

	return LR_SUCCESS;
}
#endif

//#if defined(IMPROVE_QOS)
//enum LR_RESULT rtk_idleNaptConnection(rtl_fp_napt_entry *fpNaptEntry, uint32 interval)
//#else
enum LR_RESULT rtk_idleNaptConnection(rtl_fp_napt_entry *fpNaptEntry, uint32 interval, unsigned long *lastUsed)
//#endif
{
	uint16 ipprotocol;
	uint32 hash;
	unsigned long now, last_used;
	struct Path_List_Entry *entry_path;

	#if defined(CONFIG_SMP)
    spin_lock(&table_path->path_table_lock);
    #endif

	now = jiffies;
	LOG_INFO("rtk_idleNaptConnection: P=%s int=%u.%u.%u.%u:%u ext=%u.%u.%u.%u:%u rem=%u.%u.%u.%u:%u \n",
		(fpNaptEntry->protocol==NP_TCP)? "TCP" : "UDP", NIPQUAD(fpNaptEntry->intIp), fpNaptEntry->intPort, NIPQUAD(fpNaptEntry->extIp), fpNaptEntry->extPort, NIPQUAD(fpNaptEntry->remIp), fpNaptEntry->remPort);

	if (fpNaptEntry->protocol == NP_TCP)
		ipprotocol = IPPROTO_TCP;
	else
		ipprotocol = IPPROTO_UDP;
	
#if defined(CONFIG_RTL_FAST_GRE)
	if (fpNaptEntry->protocol == NP_GRE)
		ipprotocol = IPPROTO_GRE;
#endif

	/* course = 1 (Outbound) */
	hash = FastPath_Hash_PATH_Entry(fpNaptEntry->intIp, fpNaptEntry->intPort, fpNaptEntry->remIp, fpNaptEntry->remPort, ipprotocol);
#if defined(CONFIG_RTL_FAST_GRE)
	if(ipprotocol == IPPROTO_GRE)
		hash = FastPath_Hash_PATH_Entry(fpNaptEntry->intIp, 0, fpNaptEntry->remIp, fpNaptEntry->remPort, ipprotocol);		
#endif		

	CTAILQ_FOREACH(entry_path, &table_path->list[hash], path_link) {
		if ((entry_path->protocol == fpNaptEntry->protocol) &&
			(entry_path->course == 1)	&&
			(entry_path->vaild == RTL_FP_NAPT_VALID) &&
			(entry_path->in_sIp == fpNaptEntry->intIp) &&
			(entry_path->in_sPort == fpNaptEntry->intPort) &&
			(entry_path->out_sIp == fpNaptEntry->extIp) &&
			(entry_path->out_sPort == fpNaptEntry->extPort) &&
			(entry_path->in_dIp == fpNaptEntry->remIp) &&
			(entry_path->in_dPort == fpNaptEntry->remPort)) {
			last_used = entry_path->last_used;
			if (time_before((now - interval), last_used))
			{
#if 0//defined(IMPROVE_QOS)
				/* update ct expires time */
				rtl_set_ct_timeout_expires(entry_path->ct,  last_used+interval);
#else
				*lastUsed=last_used;
#endif
                #if defined(CONFIG_SMP)
                spin_unlock(&table_path->path_table_lock);
                #endif
				return LR_FAILED;
			}
			break;
		}
	}

	/* course = 2 (Inbound) */
	hash = FastPath_Hash_PATH_Entry(fpNaptEntry->remIp, fpNaptEntry->remPort, fpNaptEntry->extIp, fpNaptEntry->extPort, ipprotocol);
#if defined(CONFIG_RTL_FAST_GRE)
	if(ipprotocol == IPPROTO_GRE)
		hash = FastPath_Hash_PATH_Entry(fpNaptEntry->remIp, 0, fpNaptEntry->extIp, fpNaptEntry->extPort, ipprotocol);
#endif		

	CTAILQ_FOREACH(entry_path, &table_path->list[hash], path_link) {
		if ((entry_path->protocol == fpNaptEntry->protocol) &&
			(entry_path->course == 2)	&&
			(entry_path->vaild == RTL_FP_NAPT_VALID) &&
			(entry_path->in_dIp == fpNaptEntry->extIp) &&
			(entry_path->in_dPort == fpNaptEntry->extPort) &&
			(entry_path->out_sIp == fpNaptEntry->remIp)  &&
			(entry_path->out_sPort == fpNaptEntry->remPort) &&
			(entry_path->out_dIp== fpNaptEntry->intIp) &&
			(entry_path->out_dPort == fpNaptEntry->intPort)) {
			last_used = entry_path->last_used;
			if (time_before((now - interval), last_used))
			{
#if 0//defined(IMPROVE_QOS)
				/* update ct expires time */
				rtl_set_ct_timeout_expires(entry_path->ct,  last_used+interval);
#else
				*lastUsed=last_used;
#endif
                #if defined(CONFIG_SMP)
                spin_unlock(&table_path->path_table_lock);
                #endif
				return LR_FAILED;
			}
			break;
		}
	}

    #if defined(CONFIG_SMP)
    spin_unlock(&table_path->path_table_lock);
    #endif
	return LR_SUCCESS;
}

/* return value:
	FAILED:	ct should be delete
	SUCCESS:		ct should NOT be delete.
*/
int rtl_fpTimer_update(void *ct)
{
	/*2007-12-19*/
	unsigned long expires, now;
	int		drop;
	enum NP_PROTOCOL protocol;
	rtl_fp_napt_entry rtlFpNaptEntry;
#if 1//!defined(IMPROVE_QOS)
	unsigned long lastUsed;
#endif

	now = jiffies;
	//read_lock_bh(&nf_conntrack_lock);
	if (rtl_get_ct_protonum(ct, IP_CT_DIR_ORIGINAL)  == IPPROTO_UDP)	{
		protocol = NP_UDP;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		if(rtl_get_ct_udp_status(ct) & IPS_SEEN_REPLY)
			expires = udp_get_timeouts_by_state(UDP_CT_REPLIED,(void *)ct,1);
		else
			expires = udp_get_timeouts_by_state(UDP_CT_UNREPLIED,(void *)ct,1);
#else
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
		if(rtl_get_ct_udp_status(ct) & IPS_SEEN_REPLY)
			expires = udp_get_timeouts_by_state(UDP_CT_REPLIED);
		else
			expires = udp_get_timeouts_by_state(UDP_CT_UNREPLIED);
		#else
		if(rtl_get_ct_udp_status(ct) & IPS_SEEN_REPLY)
			expires = nf_ct_udp_timeout_stream;
		else
			expires = nf_ct_udp_timeout;
		#endif
#endif
	}
	else if (rtl_get_ct_protonum(ct, IP_CT_DIR_ORIGINAL) == IPPROTO_TCP &&
		rtl_get_ct_tcp_state(ct) < TCP_CONNTRACK_LAST_ACK) {
		protocol = NP_TCP;
		expires = rtl_tcp_get_timeouts(ct);
	}
#if defined(CONFIG_RTL_FAST_GRE)
	else if(rtl_get_ct_protonum(ct, IP_CT_DIR_ORIGINAL)  == IPPROTO_GRE){
		protocol = NP_GRE;		
		expires =  GRE_STREAM_TIMEOUT;
	}
#endif	
	else {
		//read_unlock_bh(&nf_conntrack_lock);
		return FAILED;
	}

	drop = TRUE;
	/* really idle timeout, not force to destroy */
	/*if (test_bit(IPS_SEEN_REPLY_BIT, &ct->status))*/

	if (rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_ORIGINAL, 0)
			== rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_REPLY, 1)) {
			/* wan->lan */

			rtlFpNaptEntry.protocol=protocol;
			rtlFpNaptEntry.intIp=rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_REPLY, 0);
			rtlFpNaptEntry.intPort=rtl_get_ct_port_by_dir(ct, IP_CT_DIR_REPLY, 0);
			rtlFpNaptEntry.extIp=rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_ORIGINAL, 1);
			rtlFpNaptEntry.extPort=rtl_get_ct_port_by_dir(ct, IP_CT_DIR_ORIGINAL, 1);
			rtlFpNaptEntry.remIp=rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_ORIGINAL, 0);
			rtlFpNaptEntry.remPort=rtl_get_ct_port_by_dir(ct, IP_CT_DIR_ORIGINAL, 0);

		#ifdef CONFIG_FAST_PATH_MODULE
			if(FastPath_hook11!=NULL)
			{
				if (FastPath_hook11(&rtlFpNaptEntry,
						expires) != LR_SUCCESS) {
						drop = FALSE;
				}
			}
		#else
			#if 0//defined(IMPROVE_QOS)
			if (rtk_idleNaptConnection(&rtlFpNaptEntry,
					expires) != LR_SUCCESS) {
					drop = FALSE;
			}
			#else	/*	defined(IMPROVE_QOS)		*/
			if (rtk_idleNaptConnection(&rtlFpNaptEntry,
					expires, &lastUsed) != LR_SUCCESS) {
					//ct->timeout.expires = lastUsed+expires;
					rtl_set_ct_timeout_expires(ct,  lastUsed+expires);
					drop = FALSE;
			}
			#endif	/*	defined(IMPROVE_QOS)		*/
		#endif

	} else if (rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_ORIGINAL, 1)
				 == rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_REPLY, 0)) {

			rtlFpNaptEntry.protocol=protocol;
			rtlFpNaptEntry.intIp=rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_ORIGINAL, 0);
			rtlFpNaptEntry.intPort=rtl_get_ct_port_by_dir(ct, IP_CT_DIR_ORIGINAL, 0);
			rtlFpNaptEntry.extIp=rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_REPLY, 1);
			rtlFpNaptEntry.extPort=rtl_get_ct_port_by_dir(ct, IP_CT_DIR_REPLY, 1);
			rtlFpNaptEntry.remIp=rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_REPLY, 0);
			rtlFpNaptEntry.remPort=rtl_get_ct_port_by_dir(ct, IP_CT_DIR_REPLY, 0);

		#ifdef CONFIG_FAST_PATH_MODULE
			if(FastPath_hook11!=NULL)
			{
				if (FastPath_hook11(&rtlFpNaptEntry,
						expires) != LR_SUCCESS) {
						drop = FALSE;
				}
			}
		#else
			#if 0//defined(IMPROVE_QOS)
			if (rtk_idleNaptConnection(&rtlFpNaptEntry,
					expires) != LR_SUCCESS) {
					drop = FALSE;
			}
			#else	/*	defined(IMPROVE_QOS)		*/
			if (rtk_idleNaptConnection(&rtlFpNaptEntry,
					expires, &lastUsed) != LR_SUCCESS) {
					rtl_set_ct_timeout_expires(ct,  lastUsed+expires);
					drop = FALSE;
			}
			#endif	/*	defined(IMPROVE_QOS)		*/
		#endif
	}
	//read_unlock_bh(&nf_conntrack_lock);

	if (drop == FALSE) {
		/* update ct expires time in rtk_idleNaptConnection() */
		rtl_check_for_acc(ct, (now+expires));
		return SUCCESS;
	} else{
		return FAILED;
	}

}

#if defined(CONFIG_BRIDGE)
/* if topology_changing then use forward_delay (default 15 sec)
 * otherwise keep longer (default 5 minutes)
 */

int32 rtl_br_fdb_time_update(void *br_dummy, void *fdb_dummy, const unsigned char *addr)
{
	unsigned long	hwage;
	ether_addr_t *macAddr;
	//void *br;
#if defined(CONFIG_RTL_FASTBRIDGE)
	unsigned long	fb_aging;
#endif

	macAddr = (ether_addr_t *)(addr);
	/* br = (net_bridge*)br_dummy;	*/

	//lookup LAN mac agetimer
	#if defined(CONFIG_RTL_819X) && !defined(CONFIG_RTL8686_GMAC) && defined(CONFIG_RTL_819X_SWCORE)
	hwage = rtl_get_hw_fdb_age(RTL_LAN_FID, macAddr, FDB_DYNAMIC);
	#else
	hwage = 150;
	#endif
	switch(hwage)
	{
		case 450:
			rtl_set_fdb_aging(fdb_dummy, jiffies);
			break;
		case 300:
			rtl_set_fdb_aging(fdb_dummy, jiffies  -150*HZ);
			break;
		case 150:
			rtl_set_fdb_aging(fdb_dummy, jiffies  -300*HZ);
			break;
		default:
			break;
	}

	#if defined(CONFIG_RTL_FASTBRIDGE)
	fb_aging = rtl_fb_get_entry_lastused(addr);
	if (fb_aging>0) {
		if(time_before_eq(rtl_get_fdb_aging(fdb_dummy),  fb_aging))
			rtl_set_fdb_aging(fdb_dummy, fb_aging);
	}
	#endif
	/*original code if (unlikely(time_before_eq(rtl_get_fdb_aging(fdb_dummy) + rtl_hold_time(br), jiffies))) {*/
	if (unlikely(time_before_eq(rtl_get_fdb_aging(fdb_dummy) + rtl_hold_time(br_dummy), jiffies))) {
		return FAILED;
	}
	return SUCCESS;
}
#endif

/* ==================================================================================================== */
uint8 *
FastPath_Route(ipaddr_t dIp)
{
	uint8 *ifname = NULL;
	uint32 mask_max = 0x0;
	struct Route_List_Entry *ep;
	/* Lookup */
	CTAILQ_FOREACH(ep, &route_list_inuse, tqe_link) {
		if ((ep->mask >= mask_max) && ((dIp & ep->mask) == ep->ip)) {
			ifname = &ep->ifname[0];
			mask_max = ep->mask;
		}
	}

	return ifname;
}


/* ==================================================================================================== */
#ifdef INVALID_PATH_BY_FIN
void mark_path_invalid(uint32 sIp, uint16 sPort, uint32 dIp, uint16 dPort, uint16 iphProtocol)
{
	struct Path_List_Entry *entry_path;
	uint32 hash;
	uint32 extIp=0;
	uint16 extPort=0;

	hash = FastPath_Hash_PATH_Entry(sIp, sPort, dIp, dPort, iphProtocol);

	CTAILQ_FOREACH(entry_path, &table_path->list[hash], path_link) {
		if ((entry_path->in_sPort == sPort) &&
				(entry_path->in_dPort == dPort) &&
				(entry_path->in_sIp == sIp) &&
				(entry_path->in_dIp == dIp) &&
				(entry_path->vaild == RTL_FP_NAPT_VALID)) {
			entry_path->vaild = RTL_FP_NAPT_INVALID;
			if (entry_path->course == 1) {
				extIp = entry_path->out_sIp;
				extPort = entry_path->out_sPort;
			}
			else {
				extIp = entry_path->out_dIp;
				extPort = entry_path->out_dPort;
			}
			break;
		}
	}
	if (extIp == 0)
		return;

	if (entry_path->course == 1)
		hash = FastPath_Hash_PATH_Entry(dIp, dPort, extIp, extPort, iphProtocol);
	else
		hash = FastPath_Hash_PATH_Entry(extIp, extPort, sIp, sPort, iphProtocol);

	CTAILQ_FOREACH(entry_path, &table_path->list[hash], path_link) {
		if ((entry_path->out_sIp == dIp) &&
				(entry_path->out_sPort == dPort) &&
				(entry_path->out_dIp == sIp) &&
				(entry_path->out_dPort == sPort) &&
				(entry_path->vaild == RTL_FP_NAPT_VALID))
			entry_path->vaild = RTL_FP_NAPT_INVALID;
	}
}
#endif // INVALID_PATH_BY_FIN

#if 0//defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
void rtl_fp_mark_invalid(void *ct)
{
	u_int8_t reply_proto, ori_proto;
	__be32 reply_sip, reply_dip, ori_sip, ori_dip;
	__be16 reply_sport, reply_dport, ori_sport, ori_dport;

	reply_sip = rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_REPLY, 0);
	reply_dip = rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_REPLY, 1);
	ori_sip = rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_ORIGINAL, 0);
	ori_dip = rtl_get_ct_ip_by_dir(ct, IP_CT_DIR_ORIGINAL, 1);

	reply_sport = rtl_get_ct_port_by_dir(ct, IP_CT_DIR_REPLY, 0);
	reply_dport = rtl_get_ct_port_by_dir(ct, IP_CT_DIR_REPLY, 1);
	ori_sport = rtl_get_ct_port_by_dir(ct, IP_CT_DIR_ORIGINAL, 0);
	ori_dport = rtl_get_ct_port_by_dir(ct, IP_CT_DIR_ORIGINAL, 1);

	reply_proto = rtl_get_ct_protonum(ct, IP_CT_DIR_REPLY);
	ori_proto = rtl_get_ct_protonum(ct, IP_CT_DIR_ORIGINAL);

	mark_path_invalid(reply_sip, reply_sport, reply_dip, reply_dport, reply_proto);

	mark_path_invalid(ori_sip, ori_sport, ori_dip, ori_dport, ori_proto);
}
#endif // INVALID_PATH_BY_FIN

/* ==================================================================================================== */
        /* cached hardware header; allow for machine alignment needs.        */
#define HH_DATA_MOD     16
#define HH_DATA_ALIGN(__len) \
        (((__len)+(HH_DATA_MOD-1))&~(HH_DATA_MOD - 1))

#define UNCACHE_MASK            0x20000000
#define UNCACHE(addr)           ((UNCACHE_MASK)|(uint32)(addr))

#if defined(CONFIG_RTL_819X_SWCORE)
extern unsigned int statistic_fp;
extern unsigned int statistic_normal;
extern unsigned int statistic_fp_failed;
extern unsigned int statistic_match_failed;
#endif
__IRAM_GEN int enter_fast_path(void *skb)	/* Ethertype = 0x0800 (IP Packet) */
{

	struct iphdr *iph;
	uint32 sIp,dIp;
	uint32 hash;
	struct tcphdr *tcphupuh;  //just keep one , don't care tcp or udp //
	struct Path_List_Entry *entry_path;
	uint32 sPort;
	uint32 dPort;
	uint32 iphProtocol;
	uint32 frag_off;
	//u8 	protocol;
//	int has_inc_dst_ref_cnt;
	int need_l4cksm=1; //mark_add
	int32 ret;
	char *skb_dev_name;
#ifdef RTL_FSTPATH_TTL_ADJUST
	unsigned short ttl_ori, ttl_adj;
#endif

#ifdef CONFIG_UDP_FRAG_CACHE
	int first_frag=0,last_frag=0;
	int check_tcp_flag=0;
	struct Udp_FragCache_Entry *frag_entry =NULL;
	struct Negative_FragCache_Entry *negative_frag_entry=NULL;
#endif

#if defined(CONFIG_RTL_BATTLENET_ALG)
	extern unsigned int _br0_mask;
#endif

	skb_dev_name = rtl_get_skb_dev_name(skb);

	iph = rtl_ip_hdr(skb);
	if (iph->ttl <= 1)
    {
		return 0;
    }
	
	iphProtocol = iph->protocol;

#if defined(CONFIG_RTL_FAST_GRE)
	extern int fast_gre_fw;
	if(iphProtocol == IPPROTO_GRE)
		if(!fast_gre_fw)
			return 0;
#endif

// check if skb len and protocol type in advance --
	if (iphProtocol != IPPROTO_TCP && iphProtocol != IPPROTO_UDP
#if defined(CONFIG_RTL_FAST_GRE)
		&& iphProtocol != IPPROTO_GRE
#endif
	){
		return 0;
	}

#if	!defined(CONFIG_UDP_FRAG_CACHE)
	if (rtl_get_skb_len(skb) < sizeof(struct iphdr)+ sizeof(struct tcphdr))
	{
		return 0;
	}
#endif

//--------------------------------- david+2007-08-25
	tcphupuh = (struct tcphdr*)((__u32 *)iph + iph->ihl);

	ret = fast_path_pre_process_check(iph, tcphupuh, skb);
	if (ret!=NET_RX_PASSBY)
		return ret;
	/* DONT care now! */
	if (!strcmp(skb_dev_name, "lo"))
	{
		return 0;
	}

 	#if !defined(IMPROVE_QOS) && defined(CONFIG_NET_SCHED)
	if (gQosEnabled) {
		u_short proto = ntohs(rtl_get_skb_protocol(skb));
		if(proto == ETH_P_IP){
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
			if (rtl_hook_is_empty(skb, PF_INET, NF_IP_PRE_ROUTING) == 0)
				rtl_ipt_do_table(skb, NF_IP_PRE_ROUTING, NULL, NULL);
		#else
			(list_empty(&nf_hooks[PF_INET][NF_IP_PRE_ROUTING]))?: \
				rtl_ipt_do_table(skb, NF_IP_PRE_ROUTING, NULL, NULL);
		#endif
		}
	}
	#endif

	frag_off=htons(iph->frag_off);
	sIp = iph->saddr;
	dIp = iph->daddr;
	sPort = tcphupuh->source;
	dPort = tcphupuh->dest;

	
	//====
	if (IPPROTO_TCP==iphProtocol) {

		DEBUGP_PKT("==>> [%08X, %08X] SIP: %u.%u.%u.%u:%u  (%s) -> DIP: %u.%u.%u.%u:%u \n",
			rtl_get_skb_csum(skb), tcphupuh->check,
			NIPQUAD(iph->saddr), tcphupuh->source, skb_dev_name,
			NIPQUAD(iph->daddr), tcphupuh->dest);

		#ifdef CONFIG_UDP_FRAG_CACHE
		if(frag_off & 0x3fff){
			if(frag_off == 0x2000)
				check_tcp_flag=1;
			else
				check_tcp_flag=0;
		}else
			check_tcp_flag=1;

		if(check_tcp_flag==1)
		#endif
		{
			/* tcp syn pkt has already processed in fast_path_pre_process_check()	*/
		#if	defined(INVALID_PATH_BY_FIN)
			if (tcphupuh->fin) {
				mark_path_invalid(sIp, sPort, dIp, dPort, iphProtocol);
				return 0;
			}
			if (tcphupuh->rst || tcphupuh->syn) return 0;
		#else	/*	INVALID_PATH_BY_FIN	*/
			if (tcphupuh->fin || tcphupuh->rst || tcphupuh->syn) return 0;
		#endif	/*	INVALID_PATH_BY_FIN	*/
		}
		#ifndef CONFIG_UDP_FRAG_CACHE
			if (frag_off & 0x3fff)
				return 0;	/* Ignore fragment */
		#endif
	} else {
		/*	MUST be IPPROTO_UDP	*/
		DEBUGP_PKT("==>> [%08X, %08X] SIP: %u.%u.%u.%u:%u  (%s) -> DIP: %u.%u.%u.%u:%u <UDP> #0x%x\n",
			rtl_get_skb_csum(skb), tcphupuh->check,
			NIPQUAD(iph->saddr), tcphupuh->source, skb_dev_name,
			NIPQUAD(iph->daddr), tcphupuh->dest, iph->frag_off);

	
	#ifndef CONFIG_UDP_FRAG_CACHE
		if (frag_off & 0x3fff)
			return 0;	/* Ignore fragment */
	#endif
	}

		//================
		//it's a cache speed-up mechanism for UDP/TCP fragmentation
		//NOTE !!!! Because of no queuing , so we ignore those cases in mis-order UDP/TCP Frag packet process!!!
	#ifdef CONFIG_UDP_FRAG_CACHE
	if (frag_off & 0x3fff)
	{
		statistic_udp_frag_rx++;
		// Here it is commented because upNfmark and downNfmark is remembered at fastpath entry
		// So frag cache work for QoS here!

		if( frag_off  == 0x2000 ) //more = 1, offset = 0
			first_frag =1 ;
		else if( ( ( frag_off & 0x2000 ) == 0 ) && (frag_off & 0x1fff) ) //more =0 , offest !=0
			last_frag = 1;

		negative_frag_entry = find_negative_fragEntry(iph->id, iph->saddr, iph->daddr, iph->protocol);
				
		if(negative_frag_entry){
			return 0;
		}else{
			frag_entry = find_fragEntry(iph->id,iph->saddr,iph->daddr,iph->protocol);
		}

		if(frag_entry) //if got information from cache ,then use it to do fastpath!!
		{
			sPort = frag_entry->src_port;
			dPort = frag_entry->dst_port;
			if(!first_frag)
				need_l4cksm = 0; //important , dont modify L4 checksm besides first_frag !!!
		}
		else //if not cached ,all frag will go kenel besides first_frag.(to check if any chance to cache!!)
		{
			if(!first_frag)
			{
				statistic_udp_frag_ps++;
				//printk("###%s(%d)\n",__FUNCTION__,__LINE__);
				add_negative_fragEntry(iph->id,sIp, dIp, iph->protocol);
				return 0;
			}
		}
	}
	#endif
#if defined(CONFIG_RTL_FAST_GRE)
	{
		if(iph->protocol == IPPROTO_GRE)
		{
			unsigned short *pcall;
			unsigned int off_len;
			sPort = 0;
			/*version + protocol + length +*/
			off_len = 2 + 2 + 2;
			dPort = *(unsigned short*)(((unsigned char *)iph)+ sizeof(struct iphdr)+off_len);
		}
	}
#endif
	#if defined(CONFIG_SMP)
	spin_lock(&table_path->path_table_lock);
	#endif

	//================
	hash = FastPath_Hash_PATH_Entry(sIp, sPort, dIp, dPort, iphProtocol);
	CTAILQ_FOREACH(entry_path, &table_path->list[hash], path_link) {
#if defined(CONFIG_RTL_FAST_GRE)		
		if(iph->protocol == IPPROTO_GRE)
		{
			if (	//(entry_path->protocol==iphProtocol)&&
				//(entry_path->in_sPort == sPort) &&
				(entry_path->in_dPort == dPort) &&
				(entry_path->in_sIp == sIp) &&
				(entry_path->in_dIp == dIp) &&
				(entry_path->vaild == RTL_FP_NAPT_VALID) && //david
				(entry_path->protocol==NP_GRE&&iphProtocol==IPPROTO_GRE))
			{
				goto FAST_NAT_HANDLE;	
			}
			else 
			{
				continue;
			}
		}
#endif
		if (	//(entry_path->protocol==iphProtocol)&&
			(entry_path->in_sPort == sPort) &&
			(entry_path->in_dPort == dPort) &&
			(entry_path->in_sIp == sIp) &&
			(entry_path->in_dIp == dIp) &&
			(entry_path->vaild == RTL_FP_NAPT_VALID) && //david
			((entry_path->protocol==NP_TCP&&iphProtocol==IPPROTO_TCP)
			||(entry_path->protocol==NP_UDP&&iphProtocol==IPPROTO_UDP)))
#if defined(CONFIG_RTL_FAST_GRE)			
		{
			goto FAST_NAT_HANDLE;
		}
		else{			
			continue;
		}
FAST_NAT_HANDLE:
#endif

		{
			__be16	*l4Check;
			uint32	mark_bak;

			if(rtl_check_bypass_packet_hook)
			{
				if(rtl_check_bypass_packet_hook(entry_path))
				    return 0;
			}
			rtl_recheck_fp_policy_route_mark(skb, &entry_path->policy_route_mark);
			rtl_skb_policy_route_mark_set(skb, entry_path->policy_route_mark, &mark_bak);
			
			if(rtl_ip_route_input(NFPROTO_IPV4, skb, entry_path->out_dIp,entry_path->ct)){
				rtl_skb_policy_route_mark_set(skb, mark_bak, NULL);
				#if defined(CONFIG_SMP)
				spin_unlock(&table_path->path_table_lock);
				#endif
				return 0;
			}
		
			rtl_skb_policy_route_mark_set(skb, mark_bak, NULL);
			// check if dst output is ok
			
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
			if (rtl_skb_dst_check(skb, entry_path->out_dIp, iph) == FAILED)
#else
			if (rtl_skb_dst_check(skb, iph) == FAILED)
#endif
			{
				rtl_dst_release_ipv4(skb);
				#if defined(CONFIG_SMP)
				spin_unlock(&table_path->path_table_lock);
				#endif
				return 0;
			}

			if (iphProtocol== IPPROTO_TCP)
			{
				/* tcp */
				l4Check = &(((struct tcphdr *)tcphupuh)->check);
			}
#if defined(CONFIG_RTL_FAST_GRE)
			else if(iphProtocol== IPPROTO_GRE)
			{
				/**/
				need_l4cksm = 0;
				l4Check = NULL;
			}
#endif			
			else
			{
				/* udp */
			#if defined(UDP_ZERO_CHECKSUM)
				if ((((struct udphdr *)tcphupuh)->check))
					l4Check = &(((struct udphdr *)tcphupuh)->check);
				else
					l4Check = NULL;
			#else
				l4Check = &(((struct udphdr *)tcphupuh)->check);
			#endif
			}

		#ifdef CONFIG_UDP_FRAG_CACHE
			if(first_frag)
			{
				if(!add_fragEntry(iph->id,sIp, sPort, dIp, dPort, iphProtocol))//add fail , return to kernel
				{
					rtl_dst_release_ipv4(skb);
					#if defined(CONFIG_SMP)
					spin_unlock(&table_path->path_table_lock);
					#endif
					return 0;
				}
			}
			else if(last_frag)
			{
				free_cache(frag_entry);
			}
		#endif

			ret = fast_path_before_nat_check(skb, iph, iphProtocol);
			if(ret==NET_RX_DROP)
			{
				#if defined(CONFIG_SMP)
				spin_unlock(&table_path->path_table_lock);
				#endif
				return NET_RX_DROP;
			}

			/*Do change packet when decided to be forward by fastpath*/
			/*to avoid drop by pppoe receive if trim padding too early*/
			if (iphProtocol== IPPROTO_TCP)
				if(tcphupuh->ack && iph->tot_len == 40)
					skb_trim(skb, iph->tot_len);

			//DEBUGP_PKT("Type[%d] FORWARD to [%s] [%s]\n", entry_path->type, entry_path->out_ifname, skb->dst->dev->name);
			switch(entry_path->type) {
			case 0:	{	/* Only Routing */
				break;
			}
			case 1:	{	/* SNAT */
				FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_sIp), ntohl(sIp), iph->check);
				if(need_l4cksm)
				{
					if(iphProtocol== IPPROTO_UDP){
					#if defined(UDP_ZERO_CHECKSUM)
						if (l4Check && (*l4Check) !=0)
					#endif
						{
							FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_sIp), ntohl(iph->saddr), *l4Check);
						}
					}
					else
					{
						FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_sIp), ntohl(iph->saddr), *l4Check);
						if(tcphupuh->ack && iph->tot_len == htons(40))
							skb_trim(skb, iph->tot_len);
					}
				}
				iph->saddr = entry_path->out_sIp;
				break;
			}
			case 2:	/* SNPT */
			case 3:	{	/* SNAPT */
				FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_sIp),  ntohl(sIp), iph->check);
				if(need_l4cksm)
				{
					if(iphProtocol== IPPROTO_UDP)
					{
					#if defined(UDP_ZERO_CHECKSUM)
						if (l4Check && (*l4Check) !=0)
					#endif
						{
							FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_sIp), ntohl(iph->saddr), ntohs(entry_path->out_sPort), ntohs(tcphupuh->source), *l4Check);
						}
					}
					else
					{
						FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_sIp), ntohl(iph->saddr), ntohs(entry_path->out_sPort), ntohs(tcphupuh->source), *l4Check);
						if(tcphupuh->ack && iph->tot_len == htons(40))
							skb_trim(skb, iph->tot_len);
					}
					tcphupuh->source		= entry_path->out_sPort;
				}
				iph->saddr	= entry_path->out_sIp;
				break;
			}
			case 4: {	/* DNAT */
				#if defined(CONFIG_RTL_BATTLENET_ALG)
					if((sIp & _br0_mask) == (entry_path->out_dIp & _br0_mask)) {
						#if defined(CONFIG_SMP)
						spin_unlock(&table_path->path_table_lock);
						#endif
						return 0;
					}
				#endif
					FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_dIp), ntohl(dIp), iph->check);
				if(need_l4cksm)
				{
					if(iphProtocol== IPPROTO_UDP)
					{
					#if defined(UDP_ZERO_CHECKSUM)
						if (l4Check && (*l4Check) !=0)
					#endif
						{
								FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_dIp), ntohl(dIp), *l4Check);
						}
					}
					else
					{
							FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_dIp), ntohl(dIp), *l4Check);
					}
				}
				iph->daddr	= entry_path->out_dIp;
				break;
			}
			case 8: /* DNPT */
			case 12: {	/* DNAPT */
				#if defined(CONFIG_RTL_BATTLENET_ALG)
					if((sIp & _br0_mask) == (entry_path->out_dIp & _br0_mask)) {
						#if defined(CONFIG_SMP)
						spin_unlock(&table_path->path_table_lock);
						#endif
						return 0;
					}
				#endif
					FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_dIp), ntohl(dIp), iph->check);
				if(need_l4cksm)
				{
					if(iphProtocol== IPPROTO_UDP)
					{
					#if defined(UDP_ZERO_CHECKSUM)
						if (l4Check && (*l4Check) !=0)
					#endif
						{
								FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_dIp), ntohl(dIp), ntohs(entry_path->out_dPort), ntohs(tcphupuh->dest), *l4Check);
						}
					}
					else
					{
							FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_dIp), ntohl(dIp), ntohs(entry_path->out_dPort), ntohs(tcphupuh->dest), *l4Check);
					}
					tcphupuh->dest		= entry_path->out_dPort;
				}
#if defined(CONFIG_RTL_FAST_GRE)
				if(iphProtocol== IPPROTO_GRE)
				{
					struct pptp_gre_hdr *greP;
					greP = (struct pptp_gre_hdr*)(((unsigned char*)iph)+sizeof(struct iphdr));
					greP->call_id = entry_path->out_dPort;
				}
#endif
				iph->daddr	= entry_path->out_dIp;
				break;
			}
			default: {
					FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_sIp), ntohl(sIp), iph->check);
					FASTPATH_ADJUST_CHKSUM_NAT(ntohl(entry_path->out_dIp), ntohl(dIp), iph->check);

				if(need_l4cksm)
				{
					if(iphProtocol== IPPROTO_UDP)
					{
					#if defined(UDP_ZERO_CHECKSUM)
						if (l4Check && (*l4Check) !=0)
					#endif
						{
								FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_sIp), ntohl(sIp), ntohs(entry_path->out_sPort), ntohs(tcphupuh->source), *l4Check);
								FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_dIp), ntohl(dIp), ntohs(entry_path->out_dPort), ntohs(tcphupuh->dest), *l4Check);
						}
					}
					else
					{
								FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_sIp), ntohl(sIp), ntohs(entry_path->out_sPort), ntohs(tcphupuh->source), *l4Check);
								FASTPATH_ADJUST_CHKSUM_NAPT(ntohl(entry_path->out_dIp), ntohl(dIp), ntohs(entry_path->out_dPort), ntohs(tcphupuh->dest), *l4Check);
					}
					tcphupuh->source	= entry_path->out_sPort;
					tcphupuh->dest		= entry_path->out_dPort;
				}

				iph->saddr	= entry_path->out_sIp;
				iph->daddr	= entry_path->out_dIp;
				break;
			}
			}
			
			#ifdef RTL_FSTPATH_TTL_ADJUST
			ttl_ori = iph->ttl;
			iph->ttl = iph->ttl - 1;
			ttl_adj = iph->ttl;
			FASTPATH_ADJUST_CHKSUM_NPT((ttl_adj<<8) |iph->protocol, (ttl_ori<<8) |iph->protocol, iph->check);
			#endif

			rtl_set_skb_ip_summed(skb, CHECKSUM_NONE);
			rtl_set_skb_dev(skb, NULL);

		#if defined(IMPROVE_QOS) && defined(CONFIG_NET_SCHED)
			fastpath_set_qos_mark(skb, entry_path->PreMark, entry_path->PostMark);
		#endif
		
		#if 1//defined(CONFIG_RTL_DSCP_IPTABLE_CHECK) && defined(IMPROVE_QOS)
		rtl_set_iph_dscp(entry_path,iph);
		#endif

		#if 1//defined(CONFIG_RTL_VLANPRI_IPTABLE_CHECK) && defined(IMPROVE_QOS)
		rtl_set_skb_remark_vlanpri(entry_path, (struct sk_buff *)skb);
		#endif
		
		#if 1 //defined(CONFIG_RTL_VLANID_IPTABLE_CHECK) && defined(IMPROVE_QOS)
			if((entry_path->vlanid_in!=0xffff) && (entry_path->course==2))//in bound flow
				rtl_set_skb_vlanId((struct sk_buff *)skb, entry_path->vlanid_in);
				
			if((entry_path->vlanid_out!=0xffff) && (entry_path->course==1))//out bound flow
				rtl_set_skb_vlanId((struct sk_buff *)skb, entry_path->vlanid_out);
		#endif

		entry_path->last_used = jiffies;

		if (fast_path_post_process_xmit_check(iph, tcphupuh, skb)==NET_RX_DROP) {
			#if defined(CONFIG_SMP)
			spin_unlock(&table_path->path_table_lock);
			#endif
#if defined(CONFIG_RTL_819X_SWCORE)		
			statistic_fp++;
			statistic_fp_failed++;
#endif			
			return NET_RX_DROP;
		}
		#if defined(CONFIG_SMP)
		spin_unlock(&table_path->path_table_lock);
		#endif
		ip_finish_output3(skb);
#if defined(CONFIG_RTL_819X_SWCORE)		
        statistic_fp++;
#endif

		return NET_RX_DROP;
		}
		else {
			statistic_normal++;
		}
	}

	statistic_match_failed++;
	#if defined(CONFIG_SMP)
	spin_unlock(&table_path->path_table_lock);
	#endif

#if defined(IMPROVE_QOS) && defined(CONFIG_NET_SCHED)
	rtl_set_skb_inDev(skb);
#endif

	return fast_path_post_process_return_check(iph, tcphupuh, skb);
}

#ifdef	DEBUG_PROCFILE
/*
static int fastpath_forward_proc(char *buffer, char **start, off_t offset, int length)
{
	int len=0;
	len += sprnitf(buffer + len, "%d\n", fastpath_forward_flag);
	return len;
}
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)

#ifndef NO_ARP_USED
static int fastpath_arp_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	return count;
}

static int fastpath_table_arp(struct seq_file *s, void *v)
{
	struct Arp_List_Entry *ep;

	CTAILQ_FOREACH(ep, &arp_list_inuse, tqe_link) {
		panic_printk("~Arp: ip=0x%08X mac=%02X:%02X:%02X:%02X:%02X:%02X flags=0x%08X \n", ep->ip, MAC2STR(ep->mac), ep->flags);
	}

	return 0;
}

int fastpath_arp_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, fastpath_table_arp, NULL));
}

static ssize_t fastpath_arp_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return fastpath_arp_write_proc(file, userbuf,count, off);
}


struct file_operations fastpath_arp_proc_fops = {
        .open           = fastpath_arp_single_open,
	 .write		= fastpath_arp_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#ifndef DEL_ROUTE_TBL
static int fastpath_route_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	return count;
}

static int fastpath_table_route(struct seq_file *s, void *v)
{
	struct Route_List_Entry *ep;

	CTAILQ_FOREACH(ep, &route_list_inuse, tqe_link) {
		panic_printk("~Route: ip=0x%08X mask=0x%08X gateway=0x%08X ifname=%-5s flags=0x%08X \n",
			ep->ip, ep->mask, ep->gateway, ep->ifname, ep->flags);
	}

	return 0;
}

int fastpath_route_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, fastpath_table_route, NULL));
}

static ssize_t fastpath_route_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return fastpath_route_write_proc(file, userbuf,count, off);
}


struct file_operations fastpath_route_proc_fops = {
        .open           = fastpath_route_single_open,
	 .write		= fastpath_route_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#ifndef DEL_NAPT_TBL
static int fastpath_napt_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	return count;
}

static int fastpath_table_napt(struct seq_file *s, void *v)
{
	struct Napt_List_Entry *ep;

	CTAILQ_FOREACH(ep, &napt_list_inuse, tqe_link) {
		panic_printk("~Napt: [%s] int=0x%08X:%-5u ext=0x%08X:%-5u rem=0x%08X:%-5u flags=0x%08X \n",
			ep->protocol == NP_TCP ? "TCP" : "UDP",
			ep->intIp, ep->intPort, ep->extIp, ep->extPort, ep->remIp, ep->remPort,
			ep->flags);
	}

	return 0;
}

int fastpath_napt_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, fastpath_table_napt, NULL));
}

static ssize_t fastpath_napt_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return fastpath_napt_write_proc(file, userbuf,count, off);
}

struct file_operations fastpath_napt_proc_fops = {
        .open           = fastpath_napt_single_open,
	 .write		= fastpath_napt_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static int fastpath_path_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	return count;
}

static int fastpath_table_path(struct seq_file *s, void *v)
{
	struct Path_List_Entry *ep;
	
	//seq_printf(s,"entrys:\n");

	CTAILQ_FOREACH(ep, &path_list_inuse, tqe_link) {

		panic_printk("~Path: [%s] in-S=0x%08X:%-5u in-D=0x%08X:%-5u out-S=0x%08X:%-5u out-D=0x%08X:%-5u out-ifname=%-5s <%u> {%d}",
		  //printk("Path: [%s] in-S=0x%08X:%-5u in-D=0x%08X:%-5u out-S=0x%08X:%-5u out-D=0x%08X:%-5u out-ifname=%-5s <%u> {%d}\n",
			ep->protocol == NP_TCP ? "TCP" : "UDP",
			ep->in_sIp, ep->in_sPort, ep->in_dIp, ep->in_dPort,
			ep->out_sIp, ep->out_sPort, ep->out_dIp, ep->out_dPort,
			ep->out_ifname, ep->course, ep->type);
		
	#if defined(IMPROVE_QOS) && defined (CONFIG_NET_SCHED)
		panic_printk(",[PreMark:%d,PostMark:%d]",ep->PreMark, ep->PostMark);
	#endif

	#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
		panic_printk(",[policy_route_mark:%d]",ep->policy_route_mark);
	#endif
	
	#if 1//defined(CONFIG_RTL_DSCP_REMARK)
		panic_printk(",[dscp_in:%d,dscp_out:%d]",ep->dscp_in, ep->dscp_out);
	#endif

	#if 1//defined(CONFIG_RTL_DSCP_REMARK)
		panic_printk(",[vlanPrio_out:%d]", ep->vlanpri_out);
	#endif

		panic_printk("\n");
	}

	return 0;
}

int fastpath_path_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, fastpath_table_path, NULL));
}

static ssize_t fastpath_path_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return fastpath_path_write_proc(file, userbuf,count, off);
}

struct file_operations fastpath_path_proc_fops = {
        .open           = fastpath_path_single_open,
	 .write		= fastpath_path_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#else

#ifndef NO_ARP_USED
static int fastpath_table_arp(char *buffer, char **start, off_t offset, int length)
{
	struct Arp_List_Entry *ep;
	int len=0;

	CTAILQ_FOREACH(ep, &arp_list_inuse, tqe_link) {
		len += sprintf(buffer + len, "~Arp: ip=0x%08X mac=%02X:%02X:%02X:%02X:%02X:%02X flags=0x%08X \n", ep->ip, MAC2STR(ep->mac), ep->flags);
	}

	return len;
}
#endif

#ifndef DEL_ROUTE_TBL
static int fastpath_table_route(char *buffer, char **start, off_t offset, int length)
{
	struct Route_List_Entry *ep;
	int len=0;

	CTAILQ_FOREACH(ep, &route_list_inuse, tqe_link) {
		panic_printk("~Route: ip=0x%08X mask=0x%08X gateway=0x%08X ifname=%-5s flags=0x%08X \n",
			ep->ip, ep->mask, ep->gateway, ep->ifname, ep->flags);
	}

	return len;
}
#endif

#ifndef DEL_NAPT_TBL
static int fastpath_table_napt(char *buffer, char **start, off_t offset, int length)
{
	struct Napt_List_Entry *ep;
	int len=0;

	CTAILQ_FOREACH(ep, &napt_list_inuse, tqe_link) {
		panic_printk("~Napt: [%s] int=0x%08X:%-5u ext=0x%08X:%-5u rem=0x%08X:%-5u flags=0x%08X \n",
			ep->protocol == NP_TCP ? "TCP" : "UDP",
			ep->intIp, ep->intPort, ep->extIp, ep->extPort, ep->remIp, ep->remPort,
			ep->flags);
	}

	return len;
}
#endif

static int fastpath_table_path(char *buffer, char **start, off_t offset, int length)
{
	struct Path_List_Entry *ep;
	int len=0;
	//len+= sprintf(buffer,"entrys:\n");
	panic_printk("entrys:\n");

	CTAILQ_FOREACH(ep, &path_list_inuse, tqe_link) {

		panic_printk("~Path: [%s] in-S=0x%08X:%-5u in-D=0x%08X:%-5u out-S=0x%08X:%-5u out-D=0x%08X:%-5u out-ifname=%-5s <%u> {%d}",
		  //printk("Path: [%s] in-S=0x%08X:%-5u in-D=0x%08X:%-5u out-S=0x%08X:%-5u out-D=0x%08X:%-5u out-ifname=%-5s <%u> {%d}\n",
			ep->protocol == NP_TCP ? "TCP" : "UDP",
			ep->in_sIp, ep->in_sPort, ep->in_dIp, ep->in_dPort,
			ep->out_sIp, ep->out_sPort, ep->out_dIp, ep->out_dPort,
			ep->out_ifname, ep->course, ep->type);
		
	#if defined(IMPROVE_QOS) && defined (CONFIG_NET_SCHED)
		panic_printk(",[PreMark:%d,PostMark:%d]",ep->PreMark, ep->PostMark);
	#endif
	
	#if 1//defined(CONFIG_RTL_IP_POLICY_ROUTING_SUPPORT)
		panic_printk(",[policy_route_mark:%d]",ep->policy_route_mark);
	#endif
	
	#if 1//defined(CONFIG_RTL_DSCP_REMARK)
		panic_printk(",[dscp_in:%d,dscp_out:%d]",ep->dscp_in, ep->dscp_out);
	#endif

	#if 1//defined(CONFIG_RTL_DSCP_REMARK)
		panic_printk(",[vlanPrio_out:%d]", ep->vlanpri_out);
	#endif
	
		panic_printk("\n");
	}

	return len;
}

#endif

#if 0
static int fastpath_hash_path(char *buffer, char **start, off_t offset, int length)
{
	int i, len=0;

	for (i=0; i<path_table_list_max; i++) {
		len += sprintf(buffer + len, "%5d ", CTAILQ_TOTAL(&table_path->list[i]));
		if (i%12 == 11) len += sprintf(buffer + len, "\n");
	}
	len += sprintf(buffer + len, "\n");

	return len;
}
#endif
#endif	/* DEBUG_PROCFILE */

#if defined(CONFIG_PROC_FS)
int fastpath_dump_napt_entry_num(char *page, int len)
{
	#ifndef DEL_NAPT_TBL
	int	napt_num;
	struct Napt_List_Entry *ep_napt;
	#endif
	int path_num;
	struct Path_List_Entry *ep_path;

	path_num = 0;
	CTAILQ_FOREACH(ep_path, &path_list_inuse, tqe_link) {
		path_num++;
	}

	#ifndef DEL_NAPT_TBL
	napt_num = 0;
	CTAILQ_FOREACH(ep_napt, &napt_list_inuse, tqe_link) {
		napt_num++;
	}

	len += sprintf(page+len, "napt num: %d, path num: %d.\n", napt_num, path_num);
	#else
	len += sprintf(page+len, "path num: %d.\n", path_num);
	#endif

	return len;
}
#endif
void rtl_readUdpFragEntry(void)
{
	#if defined(CONFIG_UDP_FRAG_CACHE)
	unsigned int hash = 0, total = 0;
	struct Udp_FragCache_Entry *entry;
	extern struct Udp_FragCache_Table *udp_cache_table;
	panic_printk("\nudp frag cache entry:\n");
	for (hash = 0; hash < MAX_UDP_FRAG_ENTRY; hash++){
		CTAILQ_FOREACH(entry, &udp_cache_table->list[hash], path_link)
		{
			if (entry){
				panic_printk("src_ip: 0x%08x src_port:0x%04x dst_ip: 0x%08x dst_port:0x%04x frag_id:0x%04x protocol:0x%x status:0x%x \n", 
					entry->src_ip, entry->src_port, entry->dst_ip, entry->dst_port,
					entry->frag_id, entry->protocol, entry->status);
				total++;
			}
		}
	}
	panic_printk("total num: %d \n", total);
	#endif
}

//======================================
// Flush napt and path table when re-init, david+2007-05-28
//((struct Path_List_Entry *)en)->dst->dst_cache = NULL;	\   //brad move here for debug
#define FLUSH_TBL(type, list_inuse, list_free, tbl, link, max, is_path) { \
	int i; \
	struct type *en;	\
	for (i=0; i<max; i++) { \
		CTAILQ_FOREACH(en, &tbl->list[i], link) { \
			if (is_path) { \
				if (((struct Path_List_Entry *)en)->dst) { \
					((struct Path_List_Entry *)en)->dst = NULL; \
				} \
				((struct Path_List_Entry *)en)->vaild = RTL_FP_NAPT_INVALID; \
			} \
			CTAILQ_REMOVE(&tbl->list[i], en, link); \
			CTAILQ_REMOVE(&list_inuse, en, tqe_link); \
			CTAILQ_INSERT_TAIL(&list_free, en, tqe_link); \
		} \
	} \
}

//Brad disable
#if 0
static void flush_all_table(void)
{
	unsigned long flags;

	save_flags(flags); cli();

#ifndef DEL_NAPT_TBL
	FLUSH_TBL(Napt_List_Entry, napt_list_inuse, napt_list_free, table_napt, napt_link, napt_table_list_max, 0);
#endif
	FLUSH_TBL(Path_List_Entry, path_list_inuse, path_list_free, table_path, path_link, path_table_list_max, 1);

  	restore_flags(flags);
}
#endif
//======================================
#ifdef	DEBUG_PROCFILE
void init_fastpath_debug_proc(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#ifndef NO_ARP_USED
	fp_arp = proc_create_data("fp_arp", 0, &proc_root,
			 &fastpath_arp_proc_fops, NULL);
#endif
#ifndef DEL_ROUTE_TBL
	fp_route = proc_create_data("fp_route", 0, &proc_root,
			 &fastpath_route_proc_fops, NULL);
#endif
#ifndef DEL_NAPT_TBL
	fp_napt = proc_create_data("fp_napt", 0, &proc_root,
			 &fastpath_napt_proc_fops, NULL);
#endif
	fp_path = proc_create_data("fp_path", 0, &proc_root,
			 &fastpath_path_proc_fops, NULL);

#else

	/* proc file for debug */
#ifndef NO_ARP_USED
	fp_arp = create_proc_entry("fp_arp",0,NULL);
	if(fp_arp)
	{
		fp_arp->read_proc = fastpath_table_arp;
	}
#endif
#ifndef DEL_ROUTE_TBL
	fp_route = create_proc_entry("fp_route",0,NULL);
	if(fp_route)
	{
		fp_route->read_proc = fastpath_table_route;
	}
#endif
#ifndef DEL_NAPT_TBL
	fp_napt = create_proc_entry("fp_napt",0,NULL);
	if(fp_napt)
	{
		fp_napt ->read_proc = fastpath_table_napt;
	}
#endif
	fp_path = create_proc_entry("fp_path", 0, NULL);
	if(fp_path)
	{
		fp_path->read_proc = fastpath_table_path;
	}
#endif
}

void remove_fastpath_debug_proc(void)
{
#ifndef NO_ARP_USED
	if(fp_arp)
	{
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fp_arp", &proc_root);
		#else
		remove_proc_entry("fp_arp", fp_arp);
		#endif
		fp_arp = NULL;
	}
#endif

#ifndef DEL_ROUTE_TBL
	if(fp_route)
	{
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fp_route", &proc_root);
		#else
		remove_proc_entry("fp_route", fp_route);
		#endif
		fp_route = NULL;
	}
#endif

#ifndef DEL_NAPT_TBL
	if(fp_napt)
	{
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fp_napt", &proc_root);
		#else
		remove_proc_entry("fp_napt", fp_napt);
		#endif
		fp_napt = NULL;
	}
#endif

	if(fp_path)
	{
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fp_path", &proc_root);
		#else
		remove_proc_entry("fp_path", fp_path);
		#endif
		fp_path = NULL;
	}
}
#endif	/* DEBUG_PROCFILE */

#ifndef NO_ARP_USED
int init_table_arp(int arp_tbl_list_max, int arp_tbl_entry_max)
{
	int i;

	table_arp = (struct Arp_Table *)kmalloc(sizeof(struct Arp_Table), GFP_ATOMIC);
	if (table_arp == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Arp Table) \n");
		return -1;
	}
	CTAILQ_INIT(&arp_list_inuse);
	CTAILQ_INIT(&arp_list_free);

	table_arp->list=(struct Arp_list_entry_head *)kmalloc(arp_tbl_list_max*sizeof(struct Arp_list_entry_head), GFP_ATOMIC);
	if (table_arp->list == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Arp Table List) \n");
		return -1;
	}

	for (i=0; i<arp_tbl_list_max; i++) {
		CTAILQ_INIT(&table_arp->list[i]);
	}
	/* Arp-List Init */
	for (i=0; i<arp_tbl_entry_max; i++) {
		struct Arp_List_Entry *entry_arp = (struct Arp_List_Entry *)kmalloc(sizeof(struct Arp_List_Entry), GFP_ATOMIC);
		if (entry_arp == NULL) {
			DEBUGP_SYS("MALLOC Failed! (Arp Table Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&arp_list_free, entry_arp, tqe_link);
	}

	return 0;
}
#endif

#ifndef DEL_ROUTE_TBL
int init_table_route(int route_tbl_list_max, int route_tbl_entry_max)
{
	int i;

	table_route = (struct Route_Table *)kmalloc(sizeof(struct Route_Table), GFP_ATOMIC);
	if (table_route == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Route Table) \n");
		return -1;
	}
	CTAILQ_INIT(&route_list_inuse);
	CTAILQ_INIT(&route_list_free);

	route_table_list_max=route_tbl_list_max;
	table_route->list=(struct Route_list_entry_head *)kmalloc(route_tbl_list_max*sizeof(struct Route_list_entry_head), GFP_ATOMIC);
	if (table_route->list == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Route Table List) \n");
		return -1;
	}
	for (i=0; i<route_tbl_list_max; i++) {
		CTAILQ_INIT(&table_route->list[i]);
	}
	/* Route-List Init */
	for (i=0; i<route_tbl_entry_max; i++) {
		struct Route_List_Entry *entry_route = (struct Route_List_Entry *)kmalloc(sizeof(struct Route_List_Entry), GFP_ATOMIC);
		if (entry_route == NULL) {
			DEBUGP_SYS("MALLOC Failed! (Route Table Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&route_list_free, entry_route, tqe_link);
	}

	return 0;
}
#endif

#ifndef DEL_NAPT_TBL
int init_table_napt(int napt_tbl_list_max, int napt_tbl_entry_max)
{
	int i;

	table_napt = (struct Napt_Table *)kmalloc(sizeof(struct Napt_Table), GFP_ATOMIC);
	if (table_napt == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Napt Table) \n");
		return -1;
	}
	CTAILQ_INIT(&napt_list_inuse);
	CTAILQ_INIT(&napt_list_free);

	napt_table_list_max=napt_tbl_list_max;
	table_napt->list=(struct Napt_list_entry_head *)kmalloc(napt_tbl_list_max*sizeof(struct Napt_list_entry_head), GFP_ATOMIC);
	if (table_napt->list == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Napt Table List) \n");
		return -1;
	}
	for (i=0; i<napt_tbl_list_max; i++) {
		CTAILQ_INIT(&table_napt->list[i]);
	}
	/* Napt-List Init */
	for (i=0; i<napt_tbl_entry_max; i++) {
		struct Napt_List_Entry *entry_napt = \
			(struct Napt_List_Entry *)kmalloc(sizeof(struct Napt_List_Entry), GFP_ATOMIC);
		if (entry_napt == NULL) {
			DEBUGP_SYS("MALLOC Failed! (Napt Table Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&napt_list_free, entry_napt, tqe_link);
	}

	#if defined(CONFIG_SMP)
	spin_lock_init(&table_napt->napt_table_lock);
	#endif
	return 0;

}
#endif

int init_table_path(int path_tbl_list_max, int path_tbl_entry_max)
{
	int i;

	table_path = (struct Path_Table *)kmalloc(sizeof(struct Path_Table), GFP_ATOMIC);
	if (table_path == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Path Table) \n");
		return -1;
	}
	CTAILQ_INIT(&path_list_inuse);
	CTAILQ_INIT(&path_list_free);

	path_table_list_max=path_tbl_list_max;
	table_path->list=(struct Path_list_entry_head *)kmalloc(path_tbl_list_max*sizeof(struct Path_list_entry_head), GFP_ATOMIC);
	if (table_path->list == NULL) {
		DEBUGP_SYS("MALLOC Failed! (Path Table list) \n");
		return -1;
	}
	for (i=0; i<path_tbl_list_max; i++) {
		CTAILQ_INIT(&table_path->list[i]);
	}

	/* Path-List Init */
	for (i=0; i<path_tbl_entry_max; i++) {
		struct Path_List_Entry *entry_path = (struct Path_List_Entry *)kmalloc(sizeof(struct Path_List_Entry), GFP_ATOMIC);
		if (entry_path == NULL) {
			DEBUGP_SYS("MALLOC Failed! (Path Table Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&path_list_free, entry_path, tqe_link);
	}

	#if defined(CONFIG_SMP)
	spin_lock_init(&table_path->path_table_lock);
	#endif
	return 0;
}

void get_fastpath_module_info(unsigned char *buf)
{
	if(buf==NULL)
	{
		return;
	}
	sprintf(buf,"%s:%s\n",MODULE_NAME,MODULE_VERSION);
}
/*
2006-08/29:
	! Ignore TCP packet with FIN/RST/SYN flag (OR).
	! Ignore fragment of UDP packet.
2006-08/28:
	! NAT/NAPT bug fixed(RNAT/RNAPT NOT Working).
*/

