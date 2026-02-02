#include <linux/module.h>	
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
//#include <linux/brlock.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/version.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/raw.h>
#include <net/checksum.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>
#include <linux/inetdevice.h>
#include <linux/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>//brad

#include <net/rtl/rtl_types.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <uapi/linux/netfilter/nf_conntrack_tuple_common.h>
#endif
#include <net/rtl/fastpath/fastpath_core.h>
#if defined(CONFIG_RTL_FAST_FILTER)
#else
#ifdef FASTPATH_FILTER
#include "filter_v2.c"
//#include "filter_v3.c"
#endif
#endif
//#ifdef CONFIG_RTL8196B_KLD
//#define DNS_QUERY_FILTER_FOR_HTTPS_URL_FILTER
//#endif
//#ifdef URL_CONTENT_AUTHENTICATION
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
//#include <common/rtl865x_netif_local.h>
//#include <AsicDriver/rtl865x_asicCom.h>
//#include <AsicDriver/rtl865x_asicL2.h>
//#endif

///////////////////////////////////////////////////////////////////////////
#ifdef URL_CONTENT_AUTHENTICATION
#include <asm/semaphore.h>
#include <linux/wait.h>
#endif
#ifdef CONFIG_FAST_PATH_MODULE
extern int32 rtl865x_del_acl(rtl865x_AclRule_t *rule, char *netifName,int32 chainNo);
#endif

extern struct net_device *rtl_get_wan_dev(void);
extern int rtl_strcmp_wan_dev_name(const char *dst);

//#define DOS_LOG_SENDMAIL

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#endif

#ifdef DOS_FILTER
#if !defined(SUCCESS)
#define SUCCESS 0
#endif
#if !defined(FAILED)
#define FAILED 1
#endif

#define HashSize 256
#define IpBlockSet 0x400000
#define Ipv6EnableDosSet 0x1
#define EnableDosSet 0x1
#define NONE_DOS_PKT_THRES 3000 //david

typedef int (*dos_filter_handler)(void *skb);
dos_filter_handler filter_dos_ipv4_handler = NULL;
dos_filter_handler filter_dos_ipv6_handler = NULL;

typedef void (*dos_handler_func)(void);
dos_handler_func dos_timer_ipv4_func = NULL;
dos_handler_func dos_timer_ipv6_func = NULL;

void (*dos_read_proc_func)(struct seq_file *s, void *v)  = NULL;
#endif // DOS_FILTER

#ifdef URL_FILTER 
#if defined(CUSTOM_RSP_PACKET) 
#define URL_FILTER_BLOCK_PAGE_MESSAGE \
	"HTTP/1.1 401 The web site is blocked by administrator\r\nServer: Embedded HTTP Server 2.00\r\nConnection: close\r\n\r\n"\
	"<HTML><HEAD><TITLE>401 The web site is blocked by administrator</TITLE></HEAD>"\
	"\n<BODY BGCOLOR=\"#ffffff\"><H4>401 The web site is blocked by administrator</H4></BODY></HTML>\n"
static char *block_message = NULL;
struct filter_trace
{
	__u16 ip_id;
	__u32 wanside_ip;
	__u32 lanside_ip;
	__u16 tcp_window;                  
	__u8 ip_ttl;         
	__u8 isSent401;      
	__u8 isSentFinAck;
};
static struct filter_trace url_cache={0,0,0,0,0,0};
#endif // CONFIG_RTL8186_TR
#endif

#ifdef DOS_FILTER
 u_int32_t ConnectedIp[HashSize];
static struct timer_list dos_timer;

char dos_op_mode;
int whole_syn_threshold;
int whole_fin_threshold;
int whole_udp_threshold;
int whole_icmp_threshold;
int per_syn_threshold;
int per_fin_threshold;
int per_udp_threshold;
int per_icmp_threshold;
int per_ping_threshold;//
int dosFilter_block=0;
static int block_time=0,block_count=0;

int Ipv6_smurf_threshold;
int Ipv6_icmp_too_big_threshold;
int Ipv6_ns_threshold;

__DRAM_L34_FWD  u_int32_t dos_item_global=0;
__DRAM_L34_FWD  u_int32_t dos_item_global_0=0;

__DRAM_L34_FWD  u_int32_t dos_item_ipv6=0;
//u_int32_t dos_item_ipv6_backup=0;
u_int32_t dos_lan_addr,dos_lan_mask;
//static u_int32_t pre_saddr,pre_daddr;
//static u_int16_t pre_tot_len;
 char dos_flag[80];
__DRAM_L34_FWD static u_int32_t none_dos_drop_pkt_cnt=0; // david
static u_int32_t dos_item_backup=0; // david
#endif // DOS_FILTER

#ifdef URL_FILTER
__DRAM_L34_FWD static char url_count=0;
#endif

#ifdef URL_CONTENT_AUTHENTICATION
//DECLARE_MUTEX(queueBuf_rw);

typedef struct _sch_time_s
{
	struct list_head list;
	uint32 weekMask; /*bit0: sunday, bit 1: monday, .... bit 6 saturday*/
	uint32 startTime; /*minutes, ex. 5:21 = 5*60+21 minutes*/
	uint32 endTime; /*minutes*/	
}sch_time_t;

typedef struct _content_auth_schedule_s
{
	uint32 srcIpAddr; /*src ipaddr*/
	uint32 dstIpAddr; /*dst ip address*/	
	uint32 valid:1,
		   action:1, /*0:permit, 1:drop*/
		flags; /*reserved now*/
	struct list_head sch_time_listHead;
}content_auth_schedule_t;

typedef struct _content_cache_connection_s
{
	uint32 srcaddr_start; /*src ipaddr start*/	
	uint32 srcaddr_end; /*src address end*/
	uint32 dstaddr_start; /*dst ipaddr start*/	
	uint32 dstaddr_end; /*dst ip address end*/
	
	uint32 srcport_start:16, /*src port start*/
		   srcport_end:16; /*src port end*/

	uint32 dstport_start:16, /*dst port start*/
		   dstport_end:16; /*dst port end*/
	
	uint32 valid:1,
		   action:1, /*0:permit, 1:drop*/
		   reserved;
	
}content_cache_connection_t;

DECLARE_MUTEX(recv_newSkb_flag);
static DECLARE_WAIT_QUEUE_HEAD(unAuth_url_content);
static int recv_flag = 0;
int urlContAuth_enable = 0;
LIST_HEAD(unAuth_skb_list);
LIST_HEAD(waitAuthResult_skb_list);

char* HttpRedirectHead = 
	"HTTP/1.1 302 Object Moved\r\n"
	"Location: http://%s/fw_netstar_pass.asp?errorno=%d&id=%d\r\n"
	"Server: rtl865x-gateway\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: %d\r\n"
	"\r\n"
	"%s";
char* HttpRedirectContent = 
	"<html><head><title>Object Moved</title></head>"
	"<body><h1>Object Moved</h1>This Object may be found in "
	"<a HREF=\"http://%s/fw_netstar_pass.asp?errorno=%d&id=%d\">here</a></body><html>";
	
#define CONTENT_AUTH_SCH_NUM 8
#define CONTENT_AUTH_CACHE_NUM 16
content_auth_schedule_t rtl_content_auth_sch_list[CONTENT_AUTH_SCH_NUM];
content_cache_connection_t rtl_content_auth_cache_list[CONTENT_AUTH_CACHE_NUM];
int rtl_content_auth_cache_firstIdx;

#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER
int  Del_Pattern_ACL_For_ContentFilter(void);
int  Add_Pattern_ACL_For_ContentFilter(void);
#endif

//static char flag='1';
#if 1
extern int scrlog_printk(const char * fmt, ...);
#else
#if 1
#define scrlog_printk printk
#else
#define scrlog_printk(format, args...)
#endif
#endif
///////////////////////////////////////////////////////////////////////////
#ifdef URL_FILTER
#if defined(CUSTOM_RSP_PACKET) 
//#define ENQUEUED_RSP_SKB 
#if defined (ENQUEUED_RSP_SKB)
#define RSP_SKB_EXPIRED_TIME HZ
#define MAX_RSP_SESSION 16
struct tcp_rsp_skb_info
{
	void *skb;
	unsigned int rspAckSeq;
	unsigned int rspSendSeq; 
	unsigned long jiffies;
};

static struct tcp_rsp_skb_info tcp_rsp_skb_queue[MAX_RSP_SESSION];

int initTcpRspSkbQueue(void)
{
	memset(tcp_rsp_skb_queue, 0 , sizeof(tcp_rsp_skb_queue));
	return 0;
}

int enqueueTcpRspSkb(void *skb, unsigned int rspSendSeq, unsigned rspAckSeq)
{
	int i;
	unsigned int sendSeq; 
	unsigned long jiffies;
	unsigned long oldestJiffies;
	unsigned long oldestIdx=0;
	
	/*find  matched one*/
	for(i=0; i<MAX_RSP_SESSION; i++)
	{
		if((tcp_rsp_skb_queue[i].rspSendSeq==rspSendSeq) && (tcp_rsp_skb_queue[i].rspAckSeq==rspAckSeq))
		{
			if((tcp_rsp_skb_queue[i].skb!=NULL) && (tcp_rsp_skb_queue[i].skb!=skb))
			{
				kfree_skb((struct sk_buff *)(tcp_rsp_skb_queue[i].skb));
			}
			tcp_rsp_skb_queue[i].skb=skb;
			tcp_rsp_skb_queue[i].jiffies=jiffies;
			return 0;
		}
		
	}
	
	/*no matched one, find an empty one*/
	for(i=0; i<MAX_RSP_SESSION; i++)
	{
		if(tcp_rsp_skb_queue[i].skb==NULL)
		{
			tcp_rsp_skb_queue[i].skb=skb;
			tcp_rsp_skb_queue[i].jiffies=jiffies;
			tcp_rsp_skb_queue[i].rspSendSeq=rspSendSeq;
			tcp_rsp_skb_queue[i].rspAckSeq=rspAckSeq;
			return 0;
		}
	}
	
	/*all entries are used, find oldest one*/

	oldestJiffies=tcp_rsp_skb_queue[0].jiffies;
	oldestIdx=0;
	for(i=0; i<MAX_RSP_SESSION; i++)
	{
		if(time_before(tcp_rsp_skb_queue[i].jiffies,oldestJiffies))
		{
			oldestJiffies=tcp_rsp_skb_queue[i].jiffies;
			oldestIdx=i;
		}
	}

	if(tcp_rsp_skb_queue[oldestIdx].skb)
	{
		kfree_skb((struct sk_buff *)(tcp_rsp_skb_queue[oldestIdx].skb));
	}
	
	tcp_rsp_skb_queue[oldestIdx].skb=skb;
	tcp_rsp_skb_queue[oldestIdx].jiffies=jiffies;
	tcp_rsp_skb_queue[oldestIdx].rspSendSeq=rspSendSeq;
	tcp_rsp_skb_queue[oldestIdx].rspAckSeq=rspAckSeq;
	

	
	return 0;
}

int checkQueuedTcpResSkb(void *skb)
{
	int i;
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned int clientSendSeq=0;
	unsigned int clientAckSeq=0;
	void *clonedSkb;

	iph=ip_hdr(skb);
	
	if((iph==NULL) || (iph->protocol!=IPPROTO_TCP))
	{
		return 0;
	}

	tcph = (struct tcphdr *)((unsigned int*)iph + iph->ihl);
	clientSendSeq=tcph->seq;
	clientAckSeq=tcph->ack_seq;
	

	
	for(i=0; i<MAX_RSP_SESSION; i++)
	{
		if(tcp_rsp_skb_queue[i].skb!=NULL)
		{
			/*do aging out*/
			if(time_before(tcp_rsp_skb_queue[i].jiffies+RSP_SKB_EXPIRED_TIME,jiffies))
			{
				kfree_skb((struct sk_buff *)(tcp_rsp_skb_queue[i].skb));
				memset(&tcp_rsp_skb_queue[i],0,sizeof(struct tcp_rsp_skb_info));
				continue;
			}

			/*check to re-send response packet*/
			if(clientSendSeq == tcp_rsp_skb_queue[i].rspAckSeq)
			{
				if(clientAckSeq==tcp_rsp_skb_queue[i].rspSendSeq)
				{
					clonedSkb=skb_clone(tcp_rsp_skb_queue[i].skb,GFP_ATOMIC);
					if(clonedSkb!=NULL)
					{
						dev_queue_xmit(clonedSkb); 
						return 1;
					}
					else
					{
						return 0;
					}
				}
				else
				{	
					kfree_skb((struct sk_buff *)(tcp_rsp_skb_queue[i].skb));
					memset(&tcp_rsp_skb_queue[i],0,sizeof(struct tcp_rsp_skb_info));
					return 0;
				}

			}
			
		}
	}

	
	return 0;
}
#endif

static unsigned short finStateClientPort=0;
static unsigned int finStateClientSeq=0;
static unsigned short endStateClientPort=0;
static unsigned int endStateClientSeq=0;
//Brad add
static int  GenerateTCPENDACK(void *skb)
{
	struct iphdr *iph;
    struct tcphdr *tcph;
    void *nskb=NULL;
    struct iphdr *oiph;
    struct tcphdr *otcph;
	u_int16_t tmp_port;
	u_int32_t tmp_seq;
	u_int32_t tmp_addr;
  //  struct iphdr niph;
  //  struct tcphdr ntcph;
   	void *neigh;
	unsigned int tcplen;
	//void *hh;
    struct hh_cache *hh;
	void *lan_dev;
    int seqnum;

	unsigned int tmplen = 20;
	unsigned int tcphdrlen=0;
	unsigned int optionkind;
	unsigned int optionlen;
	unsigned int tsval;
	unsigned int tsecr;
	unsigned char *ptmp;

	lan_dev = (void*)rtl_get_dev_by_name(RTL_PS_BR0_DEV_NAME);
	iph=rtl_ip_hdr(skb);

	otcph = (struct tcphdr *)((u_int32_t*)iph + iph->ihl);

	if((endStateClientPort!=otcph->source) || (endStateClientSeq!=otcph->seq))
	{
		return 0;
	}
	
	neigh = (void*)rtl_neigh_lookup(&iph->saddr, lan_dev);
	if (neigh == NULL || rtl_get_hh_from_neigh(neigh) == NULL) {
		//printk("%s: neigh_lookup() failed\n", __FUNCTION__);
		if(neigh!=NULL)
		{
			neigh_release(neigh);       
		}
		return 0;
	}         	

 	nskb = (void*)skb_copy_expand(skb, LL_MAX_HEADER,0, GFP_ATOMIC);
	if (!nskb) {
		if(neigh!=NULL)
		{
			neigh_release(neigh);       
		}
		scrlog_printk("dos_filter: alloc skb fail!\n");
		return 0;
	}

		oiph= (void *) rtl_ip_hdr(nskb);
		otcph =(struct tcphdr *)((char *)oiph + oiph->ihl*4);
	
		//skb_trim(nskb, oiph->ihl*4 + sizeof(struct tcphdr));
		skb_trim(nskb, oiph->ihl*4 + otcph->doff * 4);
	
		skb_put(nskb,0);
	
		hh = (struct hh_cache *)rtl_get_hh_from_neigh(neigh);
		//lock1 = rtl_get_lock_from_hh(hh);
		do{
		//read_lock_bh(&lock1);
			seqnum=read_seqbegin(&hh->hh_lock);
			memcpy(rtl_get_skb_data(nskb) - 16, rtl_get_data_from_hh(hh), 16);
		//read_unlock_bh(&lock1);
		}while(read_seqretry(&hh->hh_lock,seqnum));
		skb_push(nskb, rtl_get_len_from_hh(hh));
		neigh_release(neigh);
	
		rtl_set_skb_nfct(nskb, NULL);
	//	nskb->nfcache = 0;
	//	nskb->nfmark = 0;
		tmp_addr = oiph->saddr;
		oiph->saddr = oiph->daddr;
		oiph->daddr = tmp_addr;
		tcph = (struct tcphdr *)((u_int32_t*)oiph + oiph->ihl);

		tmp_port = tcph->source;
		tcph->source = tcph->dest;
		tcph->dest = tmp_port;
		
		tmp_seq= tcph->seq;
		tcph->seq = tcph->ack_seq;
		tcph->ack_seq = tcph->fin?htonl(ntohl(tmp_seq)+1):tmp_seq;
		tcph->window = url_cache.tcp_window;
		((u_int8_t *)tcph)[13] = 0;
		tcph->rst	  = 0;
		tcph->ack	  = 1;
		tcph->fin	  = 0;
		tcph->urg_ptr = 0;
	
		tcplen = rtl_get_skb_len(nskb) - oiph->ihl*4 - 14;
		
		tcphdrlen= otcph->doff*4;
		ptmp = (unsigned char *)tcph;

		while(tmplen<tcphdrlen)
		{
			optionkind = ptmp[tmplen];
			if((optionkind == 0x0) || (optionkind == 0x1))
			{
				tmplen++;
				continue;
			}
			tmplen++;
			optionlen = ptmp[tmplen];
			tmplen++;
			if(optionkind == 0x8)
			{
				memcpy((void *)(&tsval),(void *)(&ptmp[tmplen]),4);
				memcpy((void *)(&tsecr),(void *)(&ptmp[tmplen+4]),4);
				memcpy((void *)(&ptmp[tmplen]),(void *)(&tsecr),4);
				memcpy((void *)(&ptmp[tmplen+4]),(void *)(&tsval),4);
				break;
			}
			tmplen += (optionlen-2);		
		}
		
		tcph->check = 0;
		tcph->check = csum_tcpudp_magic(oiph->saddr, oiph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)tcph, tcplen, 0));
		oiph->tot_len = htons(rtl_get_skb_len(nskb)-14);
		oiph->ttl = url_cache.ip_ttl;
		oiph->frag_off = 0;
		oiph->id = url_cache.ip_id + 1;
	
		oiph->check = 0;
		oiph->check = ip_fast_csum((unsigned char *)oiph, oiph->ihl);
		url_cache.isSentFinAck=0;
	
		dev_queue_xmit(nskb);
		
	return 0;
}

static int  GenerateTCPFINACK(void *skb)
{
	struct iphdr *iph;
    struct tcphdr *tcph;
    void *nskb=NULL;
    struct iphdr *oiph;
    struct tcphdr *otcph;
	u_int16_t tmp_port;
	u_int32_t tmp_seq;
	u_int32_t tmp_addr;
    //struct iphdr niph;
    //struct tcphdr ntcph;
	void *neigh;
       // struct neighbour *neigh;
	unsigned int tcplen;
	//void *hh;
	struct hh_cache *hh;
	void *lan_dev;

	//seqlock_t lock;
    int seqnum = 0;
	
	unsigned int tmplen = 20;
	unsigned int tcphdrlen=0;
	unsigned int optionkind;
	unsigned int optionlen;
	unsigned int tsval;
	unsigned int tsecr;
	unsigned char *ptmp;
	
#if defined (ENQUEUED_RSP_SKB)
	if(checkQueuedTcpResSkb(skb)==1)
	{
		return 0;
	}
#endif

	lan_dev = (void*)rtl_get_dev_by_name(RTL_PS_BR0_DEV_NAME);
	iph=rtl_ip_hdr(skb);
	otcph = (struct tcphdr *)((u_int32_t*)iph + iph->ihl);

	if((finStateClientPort!=otcph->source) || (finStateClientSeq!=otcph->seq))
	{
		return 0;
	}
	
	neigh = (void*)rtl_neigh_lookup(&iph->saddr, lan_dev);
	if (neigh == NULL || rtl_get_hh_from_neigh(neigh) == NULL) {
		//printk("%s: neigh_lookup() failed\n", __FUNCTION__);
		if(neigh!=NULL)
		{
			neigh_release(neigh);       
		}
		return 0;
	}

 	nskb = (void*)skb_copy_expand(skb, LL_MAX_HEADER,0, GFP_ATOMIC);
	if (!nskb) {
		if(neigh!=NULL)
		{
			neigh_release(neigh);       
		}
		scrlog_printk("dos_filter: alloc skb fail!\n");
		return 0;
	}

	oiph= (void *) rtl_ip_hdr(nskb);
	otcph =(struct tcphdr *)((char *)oiph + oiph->ihl*4);

	//skb_trim(nskb, oiph->ihl*4 + sizeof(struct tcphdr));
	skb_trim(nskb, oiph->ihl*4 + otcph->doff * 4);

	skb_put(nskb,0);
 	hh = (struct hh_cache *)rtl_get_hh_from_neigh(neigh);
	//lock = rtl_get_lock_from_hh(hh);
    do{
        //read_lock_bh(&lock);
        seqnum=read_seqbegin(&hh->hh_lock);
        memcpy(rtl_get_skb_data(nskb) - 16, rtl_get_data_from_hh(hh), 16);
        //read_unlock_bh(&lock);
    }while(read_seqretry(&hh->hh_lock,seqnum));
   	skb_push(nskb, rtl_get_len_from_hh(hh));
   	neigh_release(neigh);

	rtl_set_skb_nfct(nskb, NULL);
//	nskb->nfcache = 0;
//	nskb->nfmark = 0;
	tmp_addr = oiph->saddr;
	oiph->saddr = oiph->daddr;
	oiph->daddr = tmp_addr;
	tcph = (struct tcphdr *)((u_int32_t*)oiph + oiph->ihl);
	tmp_port = tcph->source;
	tcph->source = tcph->dest;
	tcph->dest = tmp_port;
	
	tmp_seq= tcph->seq;
	tcph->seq = tcph->ack_seq;
	tcph->ack_seq = tmp_seq;
	tcph->window = url_cache.tcp_window;
	((u_int8_t *)tcph)[13] = 0;
	tcph->rst = 0;
	tcph->ack = 1;
	tcph->fin =1;
	tcph->urg_ptr = 0;

	tcplen = rtl_get_skb_len(nskb) - oiph->ihl*4 - 14;
	
	tcphdrlen= otcph->doff*4;
	ptmp = (unsigned char *)tcph;

	while(tmplen<tcphdrlen)
	{
		optionkind = ptmp[tmplen];
		if((optionkind == 0x0) || (optionkind == 0x1))
		{
			tmplen++;
			continue;
		}
		tmplen++;
		optionlen = ptmp[tmplen];
		tmplen++;
		if(optionkind == 0x8)
		{
			memcpy((void *)(&tsval),(void *)(&ptmp[tmplen]),4);
			memcpy((void *)(&tsecr),(void *)(&ptmp[tmplen+4]),4);
			memcpy((void *)(&ptmp[tmplen]),(void *)(&tsecr),4);
			memcpy((void *)(&ptmp[tmplen+4]),(void *)(&tsval),4);
			break;
		}
		tmplen += (optionlen-2);		
	}
	endStateClientPort=tcph->dest;
	endStateClientSeq=tcph->ack_seq;

 	tcph->check = 0;
	tcph->check = csum_tcpudp_magic(oiph->saddr, oiph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)tcph, tcplen, 0));
	oiph->tot_len = htons(rtl_get_skb_len(nskb)-14);
	oiph->ttl = url_cache.ip_ttl;
	oiph->frag_off = 0;
	oiph->id = url_cache.ip_id + 1;
	oiph->check = 0;
	oiph->check = ip_fast_csum((unsigned char *)oiph, oiph->ihl);
	
	url_cache.isSent401 = 0;
	url_cache.isSentFinAck = 1;
	dev_queue_xmit(nskb);

	return 0;
}

//Brad add end

int  GenerateHTTP401(void *skb)
{
	struct iphdr *iph;
    struct tcphdr *tcph;
	
    void *nskb=NULL;
    struct iphdr *oiph;
    struct tcphdr *otcph;

	void *neigh;
    unsigned char *data;
	unsigned int tcplen;
	u_int16_t tmp_port;
	u_int32_t tmp_seq;
	u_int32_t tmp_addr;
    struct hh_cache *hh;
	void *lan_dev;
	//int i;
	//seqlock_t lock;
	//seqlock_t lock1;
//#if defined(CUSTOM_RSP_PACKET) 
#if defined(ENQUEUED_RSP_SKB) 
	void *cloneSkb=NULL;
#endif
	unsigned int tmplen = 20;
	unsigned int tcphdrlen=0;

	unsigned int optionkind;
	unsigned int optionlen;
	unsigned int tsval;
	unsigned int tsecr;
	unsigned char *ptmp;
    int seqnum;

	lan_dev =(void*)rtl_get_dev_by_name(RTL_PS_BR0_DEV_NAME);

	iph=(void *) rtl_ip_hdr(skb);
	otcph = (struct tcphdr *)((u_int32_t*)iph + iph->ihl);
	//get lan host ip address by arp table
	neigh = (void*)rtl_neigh_lookup(&iph->saddr, lan_dev);
	if (neigh == NULL || rtl_get_hh_from_neigh(neigh)  == NULL) {
		//printk("%s: neigh_lookup() failed\n", __FUNCTION__);
		if(neigh!=NULL)
		{
			neigh_release(neigh);       
		}
		return 0;
	}
	
#if 0
	nskb1 = (void*)skb_copy_expand(skb, LL_MAX_HEADER,0, GFP_ATOMIC);
	if (!nskb1) {
		if(neigh!=NULL)
		{
			neigh_release(neigh);       
		}
		scrlog_printk("dos_filter: alloc skb fail!\n");
		return 0;
	}

	skb_trim(nskb1, rtl_ip_hdr(nskb1)->ihl*4 + sizeof(struct tcphdr));
	skb_put(nskb1,0);

     	hh = (void*)rtl_get_hh_from_neigh(neigh);
	lock = rtl_get_lock_from_hh(hh);
       	read_lock_bh(&lock);
	memcpy(rtl_get_skb_data(nskb) - 16, rtl_get_data_from_hh(hh), 16);
       	read_unlock_bh(&lock);
       	skb_push(nskb1, rtl_get_len_from_hh(hh));
     //neigh_release(neigh); 	

      	rtl_set_skb_nfct(nskb, NULL);
//	nskb1->nfcache = 0;
//	nskb1->nfmark = 0;
	
       tcph=&ntcph;
	tcph->source = otcph->dest;
	tcph->dest = otcph->source;
       	tmp_seq= otcph->seq;
	tcph->seq = otcph->ack_seq;
	//tcph->ack_seq= tmp_seq+rtl_get_skb_len(skb)-40;
	tcph->ack_seq= tmp_seq+rtl_get_skb_len(skb)-otcph->doff * 4 - iph->ihl*4;
	tmp_seq = tcph->seq;
	tcph->doff=5;
	((u_int8_t *)tcph)[13] = 0;
	tcph->rst = 0;
	tcph->ack = 1;
	tcph->psh =0;
	tcph->urg_ptr = 0;
	tcph->window=  url_cache.tcp_window;
	tcplen = rtl_get_skb_len(nskb1) - 20-14;
     	tcph->check = 0;
       tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)tcph, tcplen, 0));
       memcpy(rtl_get_skb_data(nskb1) + ETH_HLEN+20, tcph, sizeof(ntcph));
// 	memcpy(nskb1->data +20, tcph, sizeof(ntcph));
       	//fill ip header
	niph.version	=	4;
	niph.ihl		=	sizeof(struct iphdr) >> 2;
	niph.frag_off	=	0;			
	niph.protocol	=	IPPROTO_TCP;
	niph.tos		=	0;
    	niph.daddr		=	iph->saddr;
    	niph.saddr		=	iph->daddr;
    	niph.ttl 		=	url_cache.ip_ttl;      
    	//nskb1->ip_summed		=	CHECKSUM_NONE;			
    	niph.tot_len	=	htons(40);		
    	niph.id			=	url_cache.ip_id+1;
    	niph.check		=	0;
    	niph.check		=	ip_fast_csum((unsigned char *)&niph, (niph.ihl));
    	memcpy(rtl_get_skb_data(nskb1) + ETH_HLEN, &niph, sizeof(niph));
// 	memcpy(nskb1->data, &niph, sizeof(niph));
 	memcpy(&skb_data, rtl_get_skb_data(nskb1), 54);

	dev_queue_xmit(nskb1); //send tcp ack;
#endif	

	if (block_message)
      	nskb = (void*)skb_copy_expand(skb, LL_MAX_HEADER, /*skb_tailroom(skb) + */strlen(block_message), GFP_ATOMIC);
	else
		nskb = (void*)skb_copy_expand(skb, LL_MAX_HEADER, strlen(URL_FILTER_BLOCK_PAGE_MESSAGE), GFP_ATOMIC);

	if (!nskb) {
		if(neigh!=NULL)
		{
			neigh_release(neigh);       
		}
		scrlog_printk("dos_filter: alloc skb fail!\n");
		return 0;
	}
	oiph= (void *) rtl_ip_hdr(nskb);
	otcph =(struct tcphdr *)((char *)oiph + oiph->ihl*4);

	//skb_trim(nskb, oiph->ihl*4 + sizeof(struct tcphdr));
	skb_trim(nskb, oiph->ihl*4 + otcph->doff * 4);

	if (block_message)
		skb_put(nskb,strlen(block_message));
	else
		skb_put(nskb,strlen(URL_FILTER_BLOCK_PAGE_MESSAGE));

 	hh = (struct hh_cache *)rtl_get_hh_from_neigh(neigh);
	//lock1 = rtl_get_lock_from_hh(hh);
    do{
   	//read_lock_bh(&lock1);
        seqnum=read_seqbegin(&hh->hh_lock);
        memcpy(rtl_get_skb_data(nskb) - 16, rtl_get_data_from_hh(hh), 16);
   	//read_unlock_bh(&lock1);
    }while(read_seqretry(&hh->hh_lock,seqnum));
   	skb_push(nskb, rtl_get_len_from_hh(hh));
   	neigh_release(neigh);

	rtl_set_skb_nfct(nskb, NULL);
//	nskb->nfcache = 0;
//	nskb->nfmark = 0;
	tmp_addr = oiph->saddr;
	oiph->saddr = oiph->daddr;
	oiph->daddr = tmp_addr;
	tcph = (struct tcphdr *)((u_int32_t*)oiph + oiph->ihl);
	data = (void *)tcph + tcph->doff*4;
	tmp_port = tcph->source;
	tcph->source = tcph->dest;
	tcph->dest = tmp_port;

	if (block_message)
		memcpy(data, block_message, strlen(block_message)+1);
	else
		memcpy(data, URL_FILTER_BLOCK_PAGE_MESSAGE, strlen(URL_FILTER_BLOCK_PAGE_MESSAGE));

	tmp_seq= tcph->seq;
	tcph->seq = tcph->ack_seq;
	tcph->ack_seq = ntohl(tmp_seq) + rtl_get_skb_len(skb) - otcph->doff*4 -oiph->ihl*4;
	tcph->ack_seq = htonl(tcph->ack_seq);
	tcph->window = url_cache.tcp_window;
	((u_int8_t *)tcph)[13] = 0;
	tcph->rst = 0;
	tcph->ack = 1;
	tcph->psh =1;
	tcph->fin =0;
	tcph->urg_ptr = 0;

	tcplen = rtl_get_skb_len(nskb) - oiph->ihl*4-14;
	
	tcphdrlen= otcph->doff*4;
	ptmp = (unsigned char *)tcph;

	while(tmplen<tcphdrlen)
	{
		optionkind = ptmp[tmplen];
		if((optionkind == 0x0) || (optionkind == 0x1))
		{
			tmplen++;
			continue;
		}
		tmplen++;
		optionlen = ptmp[tmplen];
		tmplen++;
		if(optionkind == 0x8)
		{
			memcpy((void *)(&tsval),(void *)(&ptmp[tmplen]),4);
			memcpy((void *)(&tsecr),(void *)(&ptmp[tmplen+4]),4);
			memcpy((void *)(&ptmp[tmplen]),(void *)(&tsecr),4);
			memcpy((void *)(&ptmp[tmplen+4]),(void *)(&tsval),4);
			break;
		}
		tmplen += (optionlen-2);		
	}
	
 	tcph->check = 0;
	tcph->check = csum_tcpudp_magic(oiph->saddr, oiph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)tcph, tcplen, 0));
	oiph->tot_len = htons(rtl_get_skb_len(nskb)-14);
	oiph->ttl = url_cache.ip_ttl;
	oiph->frag_off = 0;
	oiph->id = url_cache.ip_id+2;

	oiph->check = 0;
	oiph->check = ip_fast_csum((unsigned char *)oiph, oiph->ihl);
	url_cache.isSent401=1;

	finStateClientPort=tcph->dest;
	finStateClientSeq=tcph->ack_seq;
#if defined(ENQUEUED_RSP_SKB) 
	 cloneSkb=skb_clone(nskb,GFP_ATOMIC);
	 if(cloneSkb)
	 {
		 enqueueTcpRspSkb((void*)cloneSkb,tcph->seq,tcph->ack_seq);
	 }
#endif
	dev_queue_xmit(nskb);
	return 0;
}
#endif

#ifndef FASTPATH_FILTER
int  find_pattern(char *data, int dlen,  char *pattern, int plen, char term, unsigned int *numoff, unsigned int *numlen)
{
	int i,j,k;
	int state =0;
	*numoff = *numlen=0;
	for(i=0; i <= (dlen -plen);i++)
	{
	      if (*(data + i) == '\r')
	      {
            	  if (!(state % 2)) state++;  /* forwarding move */
              	  else state = 0;             /* reset */
              }
	      else if (*(data + i) == '\n')
	      {
	          if (state % 2) state++;
	          else state = 0;
              }
              else state = 0;

	      if (state >= 4)
	           break;
	      if(memcmp(data + i, pattern, plen)!=0)
		      continue;
	      *numoff=i + plen;
	      for (j = *numoff, k = 0; data[j] != term; j++, k++)
	        if (j > dlen) return 0 ;   /* no terminal char */
	      *numlen = k;
	      return 1;
		      
	}
 return 0;

}
#endif
#if 0
static int find_url(const char *data, size_t dlen, const char *pattern, size_t plen, char term)
{
	int i;

	if(plen > dlen)
	  return 0;
	for(i=0; data[i+plen] !=term ;i++)
	{
	      if(memcmp(data + i, pattern, plen)!=0)
		      continue;
	      else
			return 1;
	}
  return 0;
}
#endif
#if defined(CONFIG_RTL_FAST_FILTER)
#else
#ifdef FASTPATH_FILTER

static int  FilterWeb(void *skb)
{
	return FilterWeb_v2(skb);
}

#endif
#endif

#ifdef DNS_QUERY_FILTER_FOR_HTTPS_URL_FILTER
typedef struct _header {
	unsigned short int	id;
	unsigned short		u;

	short int	qdcount;
	short int	ancount;
	short int	nscount;
	short int	arcount;
} dnsheader_t;
int modifyDnsReply(struct sk_buff *skb)
{
    struct iphdr *iph;
    struct udphdr *udph;
    unsigned char *data;
	int record_num = 0;
	unsigned int ipaddr = 0;
	unsigned char change_flag = 0;
    int i = 0;
	iph=rtl_ip_hdr(skb);
	dnsheader_t *dns_hdr = NULL;
    udph=(void *) iph + iph->ihl*4;
	dns_hdr = (dnsheader_t*)((void*)udph + sizeof(struct udphdr));
    data = (void *)udph + sizeof(struct udphdr) + 12;
    if (ntohs(udph->source) != 53) // DNS response
	{
		return 0;
    }
	//printk("[%s:%d]ntohs(dns_hdr->u):%x\n",__FUNCTION__,__LINE__,ntohs(dns_hdr->u));
	if(dns_hdr == NULL)
	{
		return 0;
	}
	if(((dns_hdr->u & 0x8000)>>15) != 1)/*Not an answer*/
	{
		return 0;
	}
	record_num = ntohs(dns_hdr->ancount);
	if(record_num <= 0)
	{
		return 0;
	}
	//printk("record_num:%d\n",record_num);
	while(*data){
		data ++;
	}//query domain name end
	data += 5;//(1:to answer,type:2byte ,class:2byte) to answer
	unsigned short typeT,data_len;
	do 
	{
		data += 2;//(domain name pointer,2byte)current:type
		typeT = ntohs((*(unsigned short*)data));
		data += 8;//type:2bytes+class:2bytes+timetolive:4bytes,current:data_len
		data_len = ntohs(*(unsigned short*)data);
		data += 2;//(datalen:2byte),current:data
		if(typeT == 0x01 && data_len == 4)//IPV4 address
		{
#if	0		
			ipaddr = (unsigned int)(*(unsigned int*)data);
			printk("IP address:\n");
			printk("%d.%d.%d.%d\n",(ipaddr&0xff000000)>>24,
									(ipaddr&0x00ff0000)>>16,
									(ipaddr&0x0000ff00)>>8,
									(ipaddr&0x000000ff)>>0);
#endif			
			memset(data,0,4);
			change_flag = 1;
		}
		else if(typeT == 0x1c && data_len == 16)//IPV6 address
		{
			memset(data,0,16);
			change_flag = 1;
		}
		data += data_len;//to next record
		i ++;
	}while(i < record_num);
	if(change_flag)
	{
		/* ip checksum */
		skb->ip_summed = CHECKSUM_NONE;
		iph->check = 0;
		iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
		
		/* udp checksum */
		udph->check = 0;
		udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
						ntohs(udph->len), IPPROTO_UDP,
						csum_partial((char *)udph,
									 ntohs(udph->len), 0));
	}
    return 0;
}
static int should_block_domain_name(char* domain_name)
{	
	struct list_head *lh,*lh2;
	filter_table_list *entry=NULL;
	filter_item_entry *entry_item;
	int found = 0;
	if(domain_name == NULL){
		return 1;
	}
	list_for_each(lh, &table_list_head.table_list)
	{		
		entry=list_entry(lh, filter_table_list, table_list);		
		if(entry->type==URL_KEY_TABLE)
		{				
			list_for_each(lh2,&(entry->item_list))
			{
				entry_item=list_entry(lh2, filter_item_entry, item_list);
				if(strstr(domain_name,entry_item->data)){
					found =1;
					break;
				}
			}
			if(url_filter_mode)//white list
			{
				if(found){
					return 0;
				}
				else{
					return 1;
				}
			}
			else//black list
			{
				if(found){
					return 1;
				}
				else{
					return 0;
				}
			}
		}
	}
	return 0;
}
int FilterDnsQuery(struct sk_buff *skb)
{
//	printk("%s------->%d\n",__FUNCTION__,__LINE__);
        struct iphdr *iph;
        struct udphdr *udph;
        unsigned char *data;
        int datalen;
        char name[256];
 
		iph=rtl_ip_hdr(skb);
        udph=(void *) iph + iph->ihl*4;
        data = (void *)udph + sizeof(struct udphdr) + 12;
 	
        if (ntohs(udph->dest) != 53 && ntohs(udph->source) != 53) // DNS Query
                return 0;
 	 //printk("%s------->%d\n",__FUNCTION__,__LINE__);
        /*************************************************************************************
 
                RFC 1035 ( Chap 4.1 , p.25 ) :
                All communications inside of domain protocol are carried in a single format called a MESSAGE,
                The top level format of MESSAGE is divided into 5 sections shown below:
 
                                                    +---------------------+
                                                    |             Header              |
                                                    +---------------------+
                                                    |            Question             | the question for the name server
                                                    +---------------------+
                                                    |             Answer              | RRs answering the question
                                                    +---------------------+
                                                    |           Authority              | RRs pointing toward an authority
                                                    +---------------------+
                                                    |           Additional             | RRs holding additional information
                                                    +---------------------+
 
                We would parse these sections step by step.
 
           *************************************************************************************/
 
 
        /*************************************************************************************
                [HEADER]
 
                RFC 1035 ( 4.1.1. , p.26-p.27 ) :
 
                The header contains the following fields:
 
                                                                                1    1    1    1    1    1
                       0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5
                    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                    |                                            ID                                           |
                    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                    |QR|      Opcode     |AA|TC|RD|RA|       Z      |      RCODE     |
                    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                    |                                       QDCOUNT                                     |
                    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                    |                                       ANCOUNT                                      |
                    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                    |                                       NSCOUNT                                      |
                    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                    |                                       ARCOUNT                                      |
                    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 
                where:
 
                ID                      Identifier
                QR                      query (0), response (1).
                OPCODE          Define the kind of this message.
                AA                      Indicate if this message is sent by the AUTHORITY server for first replied query or not.
                TC                      Indicate if this packet is truncated or not.
                RD                      Request DNS server to pursue this query recursively
                RA                      Indicate if the DNS server support recursive process or not.
                Z                       reserved, must be 0.
                RCODE           return value: 0 means NO ERROR, others mean something wrong.
                QDCOUNT # of entries in the question section.
                ANCOUNT # of resource records in the answer section.
                NSCOUNT # of name server resource records in the authority records section.
                ARCOUNT # of resource records in the additional records section.
 
        *************************************************************************************/
 
 
        datalen = ntohs(udph->len) - sizeof(struct udphdr) - 12; // 12 is ID+Flags+QDCOUNT+ANCOUNT+NSCOUNT+ARCOUNT
        memset(name, 0, sizeof(name));
 
        while (datalen > 0) {
                if (*data) {
						if((strlen(name) + (*data)) >= (sizeof(name)-1))
							return 0;	
                        strncpy(name+strlen(name), data+1, *data);
                        datalen -= (*data + 1);
                        data += (*data + 1);
                        if (*data) {
                                strcat(name, ".");
                        }
                }
                else
                        break;
        }

	if(should_block_domain_name(name))
	{
		modifyDnsReply(skb);
		return 0;
	}
 	return 0;
}
#endif

int strtoint(char *p)
{
  int val=0;
  int c=0;
  for(val=0;*p!='\0';val=(val*10) + c, p++)
  {
     c=*p;
     if(c >= '0' && c <='9')
	   c=c-'0';
     else
	   break;
  }
  return val;

}
#endif //URL_FILTER


#ifdef URL_CONTENT_AUTHENTICATION

#if 1
static void print_content_list(void)
{
	sch_time_t *sch_time;
	
	int i;
	panic_printk("----------------------------------------\n");
	for(i = 0; i < CONTENT_AUTH_SCH_NUM; i++)
	{
		panic_printk("idx(%d),valid(%d),srcIp(0x%x),dstIp(0x%x),action(%d)\n",i,
			rtl_content_auth_sch_list[i].valid,rtl_content_auth_sch_list[i].srcIpAddr,rtl_content_auth_sch_list[i].dstIpAddr,
			rtl_content_auth_sch_list[i].action);
		list_for_each_entry(sch_time,&rtl_content_auth_sch_list[i].sch_time_listHead,list)
		{
			panic_printk("start(%d),end(%d),weekday(0x%x)\n",sch_time->startTime,sch_time->endTime,sch_time->weekMask);
		}
	}
	panic_printk("----------------------------------------\n");
}

static void print_cache_list(void)
{
	int i;
	panic_printk("----------------------------------------\n");
	for(i = 0; i < CONTENT_AUTH_CACHE_NUM;i++)
	{
		panic_printk("valid(%d),action(%d),dstip_s(0x%x),dstip_e(0x%x)\n",rtl_content_auth_cache_list[i].valid,rtl_content_auth_cache_list[i].action,rtl_content_auth_cache_list[i].dstaddr_start,rtl_content_auth_cache_list[i].dstaddr_end);
	}
	panic_printk("----------------------------------------\n");
}
#endif

static unsigned long ipstrtohex(char *str)
{
	uint32 ipAddr;
	uint8 oct;
	char *tokptr;

	ipAddr = 0;
	if(str == NULL)
		goto ret;

	tokptr = strsep(&str,".");
	if(tokptr == NULL)
		goto ret;
	oct = strtoint(tokptr);
	ipAddr |= oct << 24;

	tokptr = strsep(&str,".");
	if(tokptr == NULL)
		goto ret;
	oct = strtoint(tokptr);
	ipAddr |= oct << 16;

	tokptr = strsep(&str,".");
	if(tokptr == NULL)
		goto ret;
	oct = strtoint(tokptr);
	ipAddr |= oct << 8;

	tokptr = strsep(&str,".");
	if(tokptr == NULL)
		goto ret;
	oct = strtoint(tokptr);
	ipAddr |= oct << 0;
	
	
ret:
	return ipAddr;
}

static int timestrtominute(char *str)
{
	int min,hour,retval;
	char *tokptr;
	retval = 0;

	tokptr = strsep(&str, ":");
	if(tokptr == NULL)
		goto ret;
	hour = strtoint(tokptr);

	tokptr = strsep(&str, ":");
	if(tokptr == NULL)
		goto ret;
	min = strtoint(tokptr);

	retval = hour * 60 + min;
	
ret:
	return retval;
}

static int rtl_generateHttpRedirect(struct sk_buff *skb,int errno,int id)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	struct sk_buff *nskb=NULL;
	struct iphdr *oiph;
	struct tcphdr *otcph;
	u_int16_t tmp_port;
	u_int32_t tmp_addr;	
	char szRedirectPack[512];
	char szRedirectContent[260];
	char ip_addr[32];
	char smac[6], dmac[6];
	char *dptr = NULL;
	struct hh_cache *hh;
	struct net_device *lan_dev;
	struct in_device *in_dev;
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	u_int32_t lan_ip = 0;
	int i;

	lan_dev =__dev_get_by_name(RTL_PS_BR0_DEV_NAME);	
	if(lan_dev == NULL)
	{
		panic_printk("error , br0 is not exist!!!\n");
		goto free_skb;
	}
	
	skb->nh.raw = skb->data;

	memcpy(dmac,skb->data - 14, 6);
	memcpy(smac,skb->data - 8, 6);
	
	//oiph= (void *) skb->nh.iph;
	oiph= (void *) rtl_ip_hdr(skb);
	otcph = (struct tcphdr *)((u_int32_t*)skb->nh.iph + skb->nh.iph->ihl);

	if ((in_dev = __in_dev_get_rtnl(lan_dev)) != NULL)
	{
		for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
		     ifap = &ifa->ifa_next) {

			lan_ip = (u_int32_t )(ifa->ifa_address);
			break;
		 }
	}

	sprintf(ip_addr,"%u.%u.%u.%u",(lan_ip>>24)&0xff,
									(lan_ip>>16)&0xff,
									(lan_ip>>8)&0xff,
									(lan_ip)&0xff);
	
	sprintf(szRedirectContent, HttpRedirectContent, ip_addr,errno,id);

	//printk("%s\n",szRedirectContent);
	//printk("-------%s(%d),http redirect length(%d)\n",__FUNCTION__,__LINE__,strlen(szRedirectContent));
	sprintf(szRedirectPack, HttpRedirectHead, ip_addr, errno,id,strlen(szRedirectContent), szRedirectContent); 

	//printk("%s\n",szRedirectPack);
	//printk("-------%s(%d),datalength(%d)\n",__FUNCTION__,__LINE__,strlen(szRedirectPack));

	nskb = skb_copy_expand(skb, LL_MAX_HEADER,skb_tailroom(skb) + strlen(szRedirectPack), GFP_ATOMIC);
	if (!nskb) {
		panic_printk("dos_filter: alloc skb fail!\n");
		goto free_skb;
	}

	/*
	printk("=============================\n");	
	for(i = 0; i < 48; i++)
		printk("0x%x ", *(nskb->data -14 + i));
	printk("\n===============================\n");	
	*/
	skb_push(nskb, 14);
	nskb->data = (unsigned char *) (nskb->data - 14);
     	memcpy(nskb->data,smac, 6);
	memcpy((char *)(nskb->data + 6),dmac, 6);
	*(u_int16_t *)((char *)(nskb->data + 12)) = 0x0800;
	
	
       nskb->nfct = NULL;
	nskb->nfmark = 0;
	//nskb->dev = lan_dev;
	//printk("skb->len(%d)\n",nskb->len);

	nskb->nh.raw = (unsigned char *)(nskb->data + 14);

	memcpy(oiph, oiph, sizeof(struct iphdr));	
	tcph = (struct tcphdr *)((u_int32_t*)oiph + oiph->ihl);

	/* Swap source and dest */
	oiph->saddr = oiph->daddr; 
	oiph->daddr = oiph->saddr; 	
	
	tcph->source = otcph->dest;
	tcph->dest = otcph->source;

	/* Truncate to length (no data) */
	tcph->doff = sizeof(struct tcphdr)>>2;	
	skb_trim(nskb, (oiph->ihl<<2) + sizeof(struct tcphdr) + strlen(szRedirectPack) + 14);
		
	/* fill in data */
	dptr =  (char *)((char*)tcph  + (tcph->doff <<2));
	memcpy(dptr, szRedirectPack, strlen(szRedirectPack));	

	tcph->seq = otcph->ack_seq;	
	tcph->ack_seq = htonl(ntohl(otcph->seq) + otcph->syn + otcph->fin
	      + skb->len - (skb->nh.iph->ihl<<2) - (otcph->doff<<2));

	/* Reset flags */
	((u_int8_t *)tcph)[13] = 0;
	tcph->ack = 1;
	tcph->psh = 1;

	tcph->window = 0;
	tcph->urg_ptr = 0;
	

	/* Adjust TCP checksum */
	tcph->check = 0;

	tcph->check = tcp_v4_check(tcph, sizeof(struct tcphdr) + strlen(szRedirectPack),
				   oiph->saddr,
				   oiph->daddr,
				   csum_partial((char *)tcph,
						sizeof(struct tcphdr) + strlen(szRedirectPack), 0));

	//printk("tot_len(%d),skb->len(%d)\n",oiph->ihl*4+sizeof(struct tcphdr) + strlen(szRedirectPack),nskb->len);

	
	oiph->tot_len = oiph->ihl*4+sizeof(struct tcphdr) + strlen(szRedirectPack);
	
	/* Set DF, id = 0 */
	oiph->frag_off = htons(IP_DF);
	oiph->id = 0;

	nskb->ip_summed = CHECKSUM_NONE;

	/* Adjust IP TTL, DF */
	oiph->ttl = MAXTTL;

	/* Adjust IP checksum */
	oiph->check = 0;
	oiph->check = ip_fast_csum((unsigned char *)oiph, 
					   oiph->ihl);

	nskb->len = oiph->tot_len + 14;

	//printk("------skb->len(%d),skb->ip.len(%d), skb->tcp.len(%d),skb->tcpdata.len(%d)\n",nskb->len,oiph->tot_len,sizeof(struct tcphdr),strlen(szRedirectPack));
	
	//printk("=============================\n");	
	//for(i = 0; i < 64; i++)
		//printk("0x%x ", *(nskb->data + i));

	//memDump(skb->data,64,"redirect");
	//printk("=============================\n");	

	
	lan_dev = __dev_get_by_name(skb->dev->name);
	lan_dev->hard_start_xmit(nskb,lan_dev);

	/*send a reset packet to server??*/
	

free_skb:	
	//free skb now
	kfree_skb(skb);
	return 0;
}
/*
	flag	http methoe 
	0x1	  GET
*/
static int rtl_getHttpUrl(struct sk_buff *skb, char *str, uint32 flag)
{
	struct iphdr *iph;
 	struct tcphdr *tcph;
	iph=(void *) skb->nh.iph;
	tcph=(void *) iph + iph->ihl*4;
	unsigned char *data = (void *)tcph + tcph->doff*4;
	int found=0, offset,hostlen,pathlen;
	int datalen,i;
	//char str[2048];
	
	datalen= ntohs(iph->tot_len) -(iph->ihl*4)-(tcph->doff*4);
	if(memcmp(data, "GET ",sizeof("GET ") -1)!=0)
		return -1;
	
	found = find_pattern(data,datalen,"Host: ",sizeof("Host: ")-1,'\r',&offset, &hostlen);
	if(!found)
		return -1;
	strncpy(str,data+offset,hostlen);
	*(str+hostlen)=0;

	found = find_pattern(data,datalen,"GET ",sizeof("GET ")-1,'\r',&offset, &pathlen);
        if (!found || (pathlen -= (sizeof(" HTTP/x.x") - 1)) <= 0)
		return -1;	
	
	strncpy(str+hostlen,data+offset,pathlen);
	*(str+hostlen+pathlen)='\0';

	//printk("%s(%d) str=%s\n",__FUNCTION__,__LINE__,str);
	
	return 0;
}

static int auth_url_content_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpbuf[256];
	char *tokptr,*ipptr, *entryptr, *timeptr, *schtimeptr,*strptr=tmpbuf;
	unAuth_skb_t *unauth_skb,*nxt;
	uint32 srcIp = 0,dstIp = 0;
	int id = 0 ,auth_flag = 0;
	int sch_enable, start_time,end_time,weekmask,sch_action;
	int i;

	content_auth_schedule_t *sch_entry;
	sch_time_t *sch_time,*sch_nxt;

	if (count < 2 || count > 256)
		return -EFAULT;

	if (buffer && !copy_from_user(tmpbuf, buffer, count))
 	{	
 		if(memcmp(strptr,"enable", strlen("enable")) == 0)
		{
			if(urlContAuth_enable == 1)
				return count;
			
			urlContAuth_enable = 1;
			rtl_content_auth_cache_firstIdx = 0;
			panic_printk("enable url content authentication feature....\n");

			Add_Pattern_ACL_For_ContentFilter();
			/*init list head*/
			for(i = 0; i < CONTENT_AUTH_SCH_NUM; i++)
			{
				INIT_LIST_HEAD(&rtl_content_auth_sch_list[i].sch_time_listHead);
			}

			for(i= 0; i < CONTENT_AUTH_CACHE_NUM; i++)
			{
				memset(&rtl_content_auth_cache_list[i],0,sizeof(content_cache_connection_t));
			}
			
			return count;			
 		}
		else if(memcmp(strptr,"disable", strlen("disable")) == 0)
		{
			if(urlContAuth_enable  == 0)
			{
				return 1;
			}
			urlContAuth_enable  = 0;
			panic_printk("disable url content authentication feature....\n");

			Del_Pattern_ACL_For_ContentFilter();
			list_for_each_entry_safe(unauth_skb,nxt,&unAuth_skb_list, list)
			{
				/*free all unAuth skb*/
				list_del(&unauth_skb->list);
				kfree_skb(unauth_skb->skb);
				kfree(unauth_skb);
			}

			/*flush schedule list*/
			for(i = 0; i < CONTENT_AUTH_SCH_NUM; i++)
			{
				list_for_each_entry_safe(sch_time,sch_nxt,&rtl_content_auth_sch_list[i].sch_time_listHead,list)
				{
					list_del(&sch_time->list);
					kfree(sch_time);
				}
				rtl_content_auth_sch_list[i].valid = 0;
				rtl_content_auth_sch_list[i].action = 0;
				rtl_content_auth_sch_list[i].dstIpAddr = 0;
				rtl_content_auth_sch_list[i].srcIpAddr = 0;
			}

			/*flush cache list*/
			for(i= 0; i < CONTENT_AUTH_CACHE_NUM; i++)
			{
				memset(&rtl_content_auth_cache_list[i],0,sizeof(content_cache_connection_t));
			}
			rtl_content_auth_cache_firstIdx = 0;
			
			return count;
		}
		else
		{
			if(urlContAuth_enable == 0)
				return count;
			
			/*
			*format:
			* id action sch_enable srcIpAddr starttime/endtime/day sch_action; id2 action; id3 action; .....
			*/
			entryptr = strsep(&strptr,";");	
	             		
			while (entryptr != NULL)
		      	{
		      		/*1. id*/
				tokptr = strsep(&entryptr," ");
				if(tokptr == NULL)
					break;
		      		id = simple_strtol(tokptr,NULL,0);

				/*2. action, 0: permit, orther: block code*/	
		      		tokptr = strsep(&entryptr," ");			
				if(tokptr == NULL)
					break;		
				
				auth_flag = simple_strtol(tokptr,NULL,0);

				/*find this skb and forwarding...*/				
				list_for_each_entry(unauth_skb,&unAuth_skb_list, list)
				{
					if(unauth_skb->id == id)
					{
						/*auth_flag: 0 means authentication success, other: authentication FAILED*/
						if(auth_flag == 0)
						{
							//printk("%s(%d) send skb->id = %d, retval = %d\n",__FUNCTION__,__LINE__,id,FastPath_Enter(&unauth_skb->skb));
							if(FastPath_Enter(&unauth_skb->skb) != NET_RX_DROP)
							{
								netif_receive_skb(unauth_skb->skb);
								dev_put(unauth_skb->skb->dev);
							}
						}
						else
						{
							/*generate a http redirect packet and sent back to HOST*/
							rtl_generateHttpRedirect(unauth_skb->skb,auth_flag,id);
						}
						
						dstIp = unauth_skb->daddr;

						/*free unauth_skb_t*/
						list_del(&unauth_skb->list);
						kfree(unauth_skb);
						break;
					}
				}

				/*now, forwarding or drop all packet whose dst ip is dstIp*/
				if(dstIp)
				{
					/*find this skb and forwarding...*/				
					list_for_each_entry_safe(unauth_skb,nxt,&unAuth_skb_list, list)
					{
						if(unauth_skb->daddr == dstIp)
						{
							/*auth_flag: 0 means authentication success, other: authentication FAILED*/
							if(auth_flag == 0)
							{
								if(FastPath_Enter(&unauth_skb->skb) != NET_RX_DROP)
								{
									netif_receive_skb(unauth_skb->skb);
									dev_put(unauth_skb->skb->dev);
								}
							}
							else
							{
								/*drop...*/
								kfree_skb(unauth_skb->skb);
							}
							
							/*free unauth_skb_t*/
							list_del(&unauth_skb->list);
							kfree(unauth_skb);							
						}
					}
				}

				/*if auth_flag == 0, cache the connection for fast forwarding...*/
				if(dstIp != 0 && auth_flag == 0)
				{
					content_cache_connection_t *cache = NULL;
					/*look for free cache entry*/
					for(i = 0; i < CONTENT_AUTH_CACHE_NUM; i++)
					{
						if(rtl_content_auth_cache_list[i].valid == 0)
						{
							cache = &rtl_content_auth_cache_list[i];
							break;
						}
					}

					/*if no free entry, use first used entry*/
					if(cache == NULL)
					{
						if(rtl_content_auth_cache_firstIdx < 0 || rtl_content_auth_cache_firstIdx >= CONTENT_AUTH_CACHE_NUM)
							panic_printk("++++++++++++++++%s(%d) BUG!!!!!!!\n",__FUNCTION__,__LINE__);
						
						cache = &rtl_content_auth_cache_list[rtl_content_auth_cache_firstIdx];
						rtl_content_auth_cache_firstIdx++;

						if(rtl_content_auth_cache_firstIdx >= CONTENT_AUTH_CACHE_NUM)
							rtl_content_auth_cache_firstIdx = 0;
					}

					if(cache == NULL)
						panic_printk("++++++++++++++++%s(%d) BUG!!!!!!!\n",__FUNCTION__,__LINE__);

					/*add to cache list*/
					cache->srcaddr_start= 0;
					cache->srcaddr_end = 0xffffffff;
					cache->dstaddr_start = dstIp;
					cache->dstaddr_end = dstIp;
					
					cache->srcport_start = 0;
					cache->srcport_end = 65535;
					cache->dstport_start = 0;
					cache->dstport_end = 65535;

					cache->action = 0;
					cache->valid =1;
				}

				/*3. schedule enable, 0: disable, 1: enable*/
				tokptr = strsep(&entryptr," ");
				if(tokptr == NULL)
					goto error;

				sch_enable = strtoint(tokptr);

				if(sch_enable == 0)
					goto next;

				/*4. src ip address*/
				/**this field can be srcIp/dstIp */
				ipptr = strsep(&entryptr, " ");
				if(ipptr == NULL)
					goto error;
				{
					char *srcptr,*dstptr;
					srcptr = strsep(&ipptr,"/");
					if(srcptr == NULL)
						goto error;
					srcIp = ipstrtohex(srcptr);

					/*if user add dst ip, believe it*/
					dstptr = strsep(&ipptr,"/");					
					if(dstptr != NULL)
					{
						dstIp = ipstrtohex(dstptr);						
					}
				}

				if(dstIp == 0)
					goto error;

				/*find a valid entry*/
				for(i = 0; i < CONTENT_AUTH_SCH_NUM; i++)
				{
					if(rtl_content_auth_sch_list[i].srcIpAddr == srcIp && rtl_content_auth_sch_list[i].dstIpAddr == dstIp)
					{
						sch_entry = &rtl_content_auth_sch_list[i];
						break;
					}
					
					if(rtl_content_auth_sch_list[i].valid == 0)
						sch_entry = &rtl_content_auth_sch_list[i];
					
				}

				if(sch_entry == NULL)
					goto error;

				/*free all time schedule list node*/
				list_for_each_entry_safe(sch_time,sch_nxt,&sch_entry->sch_time_listHead,list)
				{
					list_del(&sch_time->list);
					kfree(sch_time);
				}
								
				if(sch_enable == 0)
				{
					/*if entry exist, delete it*/
					sch_entry->valid = 0;
					sch_entry->dstIpAddr = 0;
					sch_entry->srcIpAddr= 0;
					sch_entry->action= 0;
					goto next;
				}

				/*5. schedule time*/
				timeptr = strsep(&entryptr, " ");
				if(timeptr == NULL)
					goto error;

				schtimeptr = strsep(&timeptr, "/");
				while(schtimeptr)
				{
					/*5.1 start time*/
					tokptr = strsep(&schtimeptr,",");
					if(tokptr == NULL)
						goto nexttime;
					start_time = timestrtominute(tokptr);

					/*5.2 end time*/
					tokptr = strsep(&schtimeptr,",");
					if(tokptr == NULL)
						goto nexttime;
					
					end_time = timestrtominute(tokptr) +1;

					/*5.3 week mask,bit0: sunday, bit 1: monday, .... bit 6 saturday*/
					tokptr = strsep(&schtimeptr,",");
					if(tokptr == NULL)
						goto nexttime;
					
					weekmask = strtoint(tokptr);

					/*malloc a new sch_time*/
					sch_time = kmalloc(sizeof(sch_time_t),GFP_KERNEL);
					if(sch_time == NULL)
					{
						panic_printk("%s(%d), No free memory...\n",__FUNCTION__,__LINE__);
						goto nexttime;
					}

					sch_time->startTime = start_time;
					sch_time->endTime = end_time;
					sch_time->weekMask = weekmask;

					/*add this sch_time to listHead*/
					list_add(&sch_time->list, &sch_entry->sch_time_listHead);

nexttime:
					schtimeptr = strsep(&timeptr, "/");
					
				}

				/*6. sch_action, 0: permit, other: drop*/
				tokptr = strsep(&entryptr, " ");
				if(tokptr == NULL)
					goto error;
				sch_action = strtoint(tokptr);

				/*now, all information is ready...*/
				sch_entry->valid = 1;
				sch_entry->srcIpAddr = srcIp;
				sch_entry->dstIpAddr = dstIp;
				sch_entry->action = sch_action;

				goto next;
				
error:
				/*free all time schedule list node*/
				if(sch_entry)
					list_for_each_entry_safe(sch_time,sch_nxt,&sch_entry->sch_time_listHead,list)
					{
						list_del(&sch_time->list);
						kfree(sch_time);
					}	
				
next:
				entryptr = strsep(&strptr, ";");
		      }

		//print_content_list();
	    	return count;
		}
	}
free_time:

	/*free all time schedule list node*/
	list_for_each_entry_safe(sch_time,sch_nxt,&sch_entry->sch_time_listHead,list)
	{
		list_del(&sch_time->list);
		kfree(sch_time);
	}
	
	return -EFAULT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int auth_url_content_read_proc(struct seq_file *s, void *v)
{
	int len;
	char *url_content;
	unAuth_skb_t *unauth_skb;
	int cnt;

	wait_event_interruptible(unAuth_url_content ,recv_flag != 0);
	//return unAuth url to user...
	//len = sprintf(page, "%s\n","unAuth url content as following:");
	len = 0;
	cnt = 0;
	url_content = kmalloc(sizeof(char) * 1024,GFP_KERNEL);
	if(url_content == NULL)
		return 0;
	
	down_interruptible(&recv_newSkb_flag);
	recv_flag = 0;
	up(&recv_newSkb_flag);
	
	list_for_each_entry(unauth_skb,&unAuth_skb_list, list)
	{
		if(unauth_skb->flag == RTL_URL_CONTENT_READED)
			continue;
		
		if(rtl_getHttpUrl(unauth_skb->skb, url_content,1) == 0)
		{
			/*proc filesystem limitation: 4K issue*/
			if((len + strlen(url_content)) > 4000)
			{
				/*more packet need to be deal...*/
				down_interruptible(&recv_newSkb_flag);
				recv_flag = 1;
				up(&recv_newSkb_flag);
				break;
			}			
			cnt++;
			unauth_skb->flag = RTL_URL_CONTENT_READED;
			seq_printf(s,"id=%d,ip=0x%x,url=%s ", unauth_skb->id,unauth_skb->saddr, url_content);
			/*Fix jwj*/
			len += 20+8+strlen(url_content);
		}
	}
	
	seq_printf(s, "total=%d\n", cnt);

	kfree(url_content);	
	return 0;
}

int auth_url_content_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, auth_url_content_read_proc, NULL));
}

static ssize_t auth_url_content_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return auth_url_content_write_proc(file, userbuf,count, off);
}


struct file_operations auth_url_content_proc_fops = {
        .open           = auth_url_content_single_open,
	 	.write		= auth_url_content_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#else
static int auth_url_content_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;
	char *url_content;
	unAuth_skb_t *unauth_skb;
	int cnt;

	wait_event_interruptible(unAuth_url_content ,recv_flag != 0);
	//return unAuth url to user...
	//len = sprintf(page, "%s\n","unAuth url content as following:");
	len = 0;
	cnt = 0;
	url_content = kmalloc(sizeof(char) * 1024,GFP_KERNEL);
	if(url_content == NULL)
		return len;
	
	down_interruptible(&recv_newSkb_flag);
	recv_flag = 0;
	up(&recv_newSkb_flag);
	
	list_for_each_entry(unauth_skb,&unAuth_skb_list, list)
	{
		if(unauth_skb->flag == RTL_URL_CONTENT_READED)
			continue;
		
		if(rtl_getHttpUrl(unauth_skb->skb, url_content,1) == 0)
		{
			/*proc filesystem limitation: 4K issue*/
			if((len + strlen(url_content)) > 4000)
			{
				/*more packet need to be deal...*/
				down_interruptible(&recv_newSkb_flag);
				recv_flag = 1;
				up(&recv_newSkb_flag);
				break;
			}			
			cnt++;
			unauth_skb->flag = RTL_URL_CONTENT_READED;
			len += sprintf(page + len,"id=%d,ip=0x%x,url=%s ", unauth_skb->id,unauth_skb->saddr, url_content);
		}
	}
	
	len += sprintf(page + len, "total=%d\n", cnt);
	
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	kfree(url_content);	
	return len;
	
}
#endif

/*
* return value:
* 	0: success
*	-1: not found in sch_list or not in schedule time...
*/
static int is_in_contAuthSch(uint32 saddr,uint32 daddr, int *action)
{
	struct timeval tv;
	uint32 today, hour,minute;
	uint32 curtime;
	int i;
	content_auth_schedule_t *sch_entry = NULL;
	sch_time_t *sch_time,*sch_nxt;
	
	for(i = 0 ; i < CONTENT_AUTH_SCH_NUM; i++)
	{
		if(rtl_content_auth_sch_list[i].valid == 1 && rtl_content_auth_sch_list[i].srcIpAddr == saddr && rtl_content_auth_sch_list[i].dstIpAddr == daddr)
		{
			sch_entry = &rtl_content_auth_sch_list[i];
			break;
		}
 	}

	if(sch_entry == NULL)
		return -1;
	
	/*get system time*/
	do_gettimeofday(&tv);
	today = ((tv.tv_sec/86400) + 4)%7;
	hour = (tv.tv_sec/3600)%24;
	minute = (tv.tv_sec/60)%60;

	list_for_each_entry_safe(sch_time,sch_nxt,&sch_entry->sch_time_listHead,list)
	{
		if(sch_time->weekMask & (1<< today))
		{
			curtime = hour * 60 + minute;
			if( (sch_time->startTime <= curtime) && (curtime < sch_time->endTime))
			{
				/*bingo.....*/
				if(action)
					*action = sch_entry->action;
				return 0;
			}			
		}
	}
	
	return -1;
}

/*
* return value:
* 	0: success
*	-1: not found in sch_list or not in schedule time...
*/
static int is_in_contAuthCache(uint32 saddr,uint32 daddr, int *action)
{
	int i ;

	//print_cache_list();
	
	for(i = 0; i < CONTENT_AUTH_CACHE_NUM; i++)
	{
		if(rtl_content_auth_cache_list[i].valid && rtl_content_auth_cache_list[i].dstaddr_start <= daddr && rtl_content_auth_cache_list[i].dstaddr_end>= daddr)
		{
			/*bingo!!!*/
			if(action)
				*action = rtl_content_auth_cache_list[i].action;
			return 0;
		}
	}

	return -1;
}

int rtl_urlContent_auth(struct sk_buff *skb)
{
	unAuth_skb_t *unauth_skb;
	struct iphdr *iph;
 	struct tcphdr *tcph;	
	unsigned char *data;
	int found=0, offset,hostlen,pathlen;
	int datalen;
	int ret,action;

	//Brad add
	struct in_device *in_dev;
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	struct net_device *landev;
	struct net_device *wandev;
//---------------------	

	if(urlContAuth_enable == 0)
		return -1;
	
	skb->nh.raw = skb->data;

	iph= (void *)skb->nh.iph;
	if(iph == NULL)
		return -1;
	
	//tcp packet...
	if(iph->protocol != 0x06)
		return -1;

	/*schedule....*/
	action = 0;
	ret = is_in_contAuthSch(iph->saddr,iph->daddr,&action);
	//printk("============saddr(0x%x),daddr(0x%x),ret(%d),action(%d)\n",iph->saddr,iph->daddr,ret,action);
	/*
	*ret = 0: in schedule...
	*/
	if(ret == 0)
	{
		/*action 0: permit, other: drop*/
		if(action == 0)
			return -1;
		else
		{
			/*drop this packet...*/
			kfree_skb(skb);
			return NET_RX_DROP;
		}
	}


	/*in cache??*/
	action = 0;
	ret = is_in_contAuthCache(iph->saddr,iph->daddr,&action);
	/*
	*ret = 0: in cache...
	*/
	if(ret == 0)
	{
		/*action 0: permit, other: continue*/
		if(action == 0)
			return -1;
	}
	
	tcph=(void *) iph + iph->ihl*4;
	if(tcph == NULL)
		return -1;

	
	datalen= (iph->tot_len) -(iph->ihl*4)-(tcph->doff*4);
	if(datalen < 3)
		return -1;

	data = (void *)tcph + tcph->doff*4;
	if(memcmp(data, "GET ",sizeof("GET ") -1)!=0)
		return -1;
	
	found = find_pattern(data,datalen,"Host: ",sizeof("Host: ")-1,'\r',&offset, &hostlen);
	if(!found)
		return -1;

	//Brad add
       if ((landev = __dev_get_by_name(RTL_PS_BR0_DEV_NAME)) != NULL)
	{
		ifa =NULL;		
		if ((in_dev=__in_dev_get_rcu(landev)) != NULL)
		{
			for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next)
			{
				if (strcmp(RTL_PS_BR0_DEV_NAME, ifa->ifa_label) == 0)
				{
					break; 
				}
			}		
		//accept the http packet if the dest ip is our lan ip, always
			if(ifa != NULL)
			{
				if(iph->daddr == ifa->ifa_address)
				{
					return -1;
				}
			}
		}
	}
	   
	if ((wandev = __dev_get_by_name(RTL_PS_PPP0_DEV_NAME)) != NULL)
	{
      		ifa =NULL;
		if ((in_dev=__in_dev_get_rcu(wandev)) != NULL)
		{
			for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next) 
			{
				if (strcmp(RTL_PS_PPP0_DEV_NAME, ifa->ifa_label) == 0)
				{
					break; 
				}
			}
			//accept the http packet if the dest ip is our wan ip, always
			if(ifa !=NULL)
			{
				if(iph->daddr == ifa->ifa_local)
				{
					return -1;
				}
			}
		}
	}
	else if ((wandev = rtl_get_wan_dev()) != NULL)
	{
		ifa =NULL;
		if ((in_dev=__in_dev_get_rcu(wandev)) != NULL)
		{
			for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next)
			{
				if (rtl_strcmp_wan_dev_name(ifa->ifa_label)==0)
				{
						break; 
				}
			}
			//accept the http packet if the dest ip is our wan ip, always
			if(ifa != NULL)
			{
				if(iph->daddr == ifa->ifa_address)
				{
					return -1;
				}
			}
		}
	}
	
#ifndef CONFIG_RTL8186_TR	
	 if ((wandev = __dev_get_by_name(RTL_PS_WLAN0_DEV_NAME)) != NULL)
	 {
		ifa =NULL;
      		if ((in_dev=__in_dev_get_rcu(wandev)) != NULL) 
		{
      			for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next)
			{
				if (strcmp(RTL_PS_WLAN0_DEV_NAME, ifa->ifa_label) == 0)
				{
					break; 
				}
			}
			//accept the http packet if the dest ip is our wan ip, always
			if(ifa !=NULL)
			{
				if(iph->daddr == ifa->ifa_address)
				{
						return -1;
				}
			}
		}
	}
#endif
	/*ok, this packet is http get ....*/
	unauth_skb = kmalloc(sizeof(unAuth_skb_t),GFP_KERNEL);
	if(unauth_skb == NULL)
	{
		panic_printk("%s(%d), No free memory...\n",__FUNCTION__,__LINE__);
		return -1;
	}	

	unauth_skb->id = iph->id;
	unauth_skb->flag = 0;
	unauth_skb->saddr = iph->saddr;
	unauth_skb->daddr = iph->daddr;
	unauth_skb->skb = skb;

	//printk("%s(%d), unauth_skb->id = %d\n", __FUNCTION__,__LINE__,iph->id);
	list_add_tail(&unauth_skb->list,&unAuth_skb_list);
	
	down_interruptible(&recv_newSkb_flag);
	recv_flag = 1;
	up(&recv_newSkb_flag);

	wake_up_interruptible(&unAuth_url_content);

	return NF_QUEUE;
	
	
}

#endif //URL_CONTENT_AUTHENTICATION

#ifdef DOS_FILTER
extern void clear_attack_address(void);
static void dos_timer_fn(unsigned long arg)
{	

	if (dos_item_global || dos_item_ipv6) { 
		if(dosFilter_block == 1)
			block_count++; /*once address was assure some type of dos attack(need aoccount pkt),like icmp flood,will forerver block the same request, the code here will unblock the same request, here the issue, block clear 0, but attack_saddr not, so other dos cause ,like tcp syn flood,  block = 1, the old address , ever use icmp flood dos but recently have not, can not pass the icmp request again */

		if (dos_item_global && dos_timer_ipv4_func)
			dos_timer_ipv4_func();
		if(dos_item_ipv6 && dos_timer_ipv6_func)
			dos_timer_ipv6_func();
		
		if(block_count >=block_time || (dos_item_global & IpBlockSet)!= IpBlockSet ) { //here question, ipv6 cass>?
                        /*the most time will block the request or when block not set, always set unblock*/ 	
			dosFilter_block=0; /*according the comment below, here should add code for clear attack address?, attack_saddr clear*/
			block_count=0;
			/*add by cl, should be add ?*/
			clear_attack_address();
		}
	}
	
// david --------------------------------------
	if (none_dos_drop_pkt_cnt > NONE_DOS_PKT_THRES) {
		if (dos_item_global
		) {		
			dos_item_backup = dos_item_global;
			dos_item_global = 0;
			//printk("disable DoS\n");			
		}
	}	
	else {		
		if (dos_item_backup
		) {
			dos_item_global = dos_item_backup;
			dos_item_backup = 0;
			//printk("enable DoS\n");						
		}
	}
	if (none_dos_drop_pkt_cnt > 0)
		none_dos_drop_pkt_cnt=0;
//-------------------------------------------
	
      	mod_timer(&dos_timer, jiffies + HZ);
}

extern int filter_wan_check(void* skb);
static int  filter_dos(void *skb)
{
	struct iphdr *iph;
	int ret=0;

	iph=rtl_ip_hdr(skb);
	
	if(filter_wan_check(skb))
	{
		if(iph->version==6)
		{
			if(dos_item_ipv6 && filter_dos_ipv6_handler)
			{
				ret = filter_dos_ipv6_handler(skb);
			}
			if(ret)
			  return FAILED;
			else  	  
			  return SUCCESS;
		}

		if(dos_item_global && filter_dos_ipv4_handler)
		{
			ret = filter_dos_ipv4_handler(skb);
		}
	}
	
	if(ret)
	  return FAILED;
	else  	  
	  return SUCCESS;
}

#ifndef __HAVE_ARCH_STRTOK
char * ___strtok;
/**
 * strtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * WARNING: strtok is deprecated, use strsep instead.
 */
char * strtok(char * s,const char * ct)
{
	char *sbegin, *send;

	sbegin  = s ? s : ___strtok;
	if (!sbegin) {
		return NULL;
	}
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') {
		___strtok = NULL;
		return( NULL );
	}
	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___strtok = send;
	return (sbegin);
}
#endif

static int dos_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char tmpbuf[80];
      char *tokptr, *strptr=tmpbuf; 
      u_int8_t idx=1;
      u_int32_t val;
      
      if (count < 2) 
	    return -EFAULT;
      
      if(buffer){
      		memset(dos_flag,0,sizeof(dos_flag));
		memset(tmpbuf,0,sizeof(tmpbuf));
			
      }
      if (buffer && !copy_from_user(&dos_flag, buffer, count)) { /*do not 80*/
	  	dos_flag[count] = '\0';
	      strncpy(tmpbuf,dos_flag,count);
	      
		//  printk("%s:%d tmpbuf=%s\n",__FUNCTION__,__LINE__,tmpbuf);
	      while ((tokptr = strtok(strptr," ")) !=NULL) /*get one arg by blank spilt, every time, most number 13 arg*/
	      {
              	strptr=NULL;
              	val=simple_strtol(tokptr,NULL,0); /*change string to unsigned int value*/

				switch(idx) /*the number of variable in echo "var1 var2 var3 ..." > /proc/enable_dos, start num1*/
				{	
					case 1:  /*var1*/
						dos_op_mode=val;
						break;
					case 2: /*var2*/
						val=simple_strtol(tokptr,NULL,16);
						dos_lan_addr=val;
						break;
					case 3:  /*var3*/
						val=simple_strtol(tokptr,NULL,16);
						dos_lan_mask=val;
						break;
					case 4: /*var4*/
						//val=simple_strtol(tokptr,NULL,16); /*add cl*/
						dos_item_global=val;
						dos_item_backup = 0; // david			
						break;
					case 5: /*var5*/
						whole_syn_threshold=val;				
						break;
					case 6: /*var6*/
						whole_fin_threshold=val;
						break;
					case 7: /*var7*/
						whole_udp_threshold=val;
						break;
					case 8: /*var8*/
						whole_icmp_threshold=val;
						break;
					case 9: /*var9*/
						per_syn_threshold=val;
						break;
					case 10: /*var10*/
						per_fin_threshold=val;
						break;
					case 11:  /*var11*/
						per_udp_threshold=val;
						break;
					case 12: /*var12*/
						per_icmp_threshold=val;
						break;
					case 13: /*var13*/
						block_time=val;
						break;
					case 14: /*var14*/
						//val=simple_strtol(tokptr,NULL,16); /*add cl*/
						dos_item_global_0=val;
						break;
					case 15: //ipv6 feature
						dos_item_ipv6=val;
						//printk("%s:%d dos_item_ipv6=0x%x\n",__FUNCTION__,__LINE__,dos_item_ipv6);
						break;
					case 16:
						Ipv6_smurf_threshold=val;
						break;
					case 17:
						Ipv6_icmp_too_big_threshold=val;
						break;
					case 18:
						Ipv6_ns_threshold=val;
						break;

					default:
						break;	
				}
							
			  //printk("idx=%d,val=%d\n",idx,val);
	          idx++;
        }	      
	    return count;
      }
      return -EFAULT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int dos_read_proc(struct seq_file *s, void *v)
{
	if(dos_read_proc_func)
		  dos_read_proc_func(s, v);
	return 0;
}

static int dos_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, dos_read_proc, NULL));
}

static ssize_t dos_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return dos_write_proc(file, userbuf,count, off);
}

struct file_operations dos_proc_fops = {
        .open           = dos_single_open,
	 	.write			= dos_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#else
static int dos_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%s\n", dos_flag);


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}
#endif
#endif

void filter_addconnect(__u32 ipaddr)
{
#ifdef DOS_FILTER
//	if (dos_item_global & EnableDosSet) 
		 ConnectedIp[((__u32)ipaddr) % HashSize]=(__u32)ipaddr;
#endif
}

void filter_delconnect(__u32 ipaddr)
{
#ifdef DOS_FILTER
//	if (dos_item_global & EnableDosSet) 
		 ConnectedIp[((__u32)ipaddr) % HashSize]=0;
#endif
}

int filter_checkConnect(__u32 ipaddr)
{
	int ret = 0;

	if((ConnectedIp[((__u32)ipaddr) % HashSize] == 0) || 
		ConnectedIp[((__u32)ipaddr) % HashSize]!= ipaddr)
		ret = 1;

	return ret;
}	

#ifdef DOS_FILTER
#if defined(CUSTOM_RSP_PACKET) 	    

int (*fast_path_cusRsp401_func)(struct sk_buff *pskb) = NULL;
int (*fast_path_cusRspTCPFinAck_func)(struct sk_buff *pskb) = NULL;
int (*fast_path_cusRspTCPEndAck_func)(struct sk_buff *pskb) = NULL;

void register_customRspStr(char *_str)
{
	unregister_customRspStr();
	if(_str)
	{
		block_message=kmalloc(strlen(_str)+1,GFP_KERNEL);
		if(block_message)
			sprintf(block_message,"%s",_str);			
	}
	else
	{
		block_message=kmalloc(strlen(URL_FILTER_BLOCK_PAGE_MESSAGE),GFP_KERNEL);
		if(block_message)
			sprintf(block_message,"%s",URL_FILTER_BLOCK_PAGE_MESSAGE);
	}
	return;
};

void unregister_customRspStr(void)
{
	if(block_message)
	{
		kfree(block_message);
		block_message=NULL;
	}
	return;
}

void register_customRspHook(int *_cusRsp401_func,int *_cusRspTCPFinAck_func,int *_cusRspTCPEndAck_func)
{	
	if(_cusRsp401_func)
		fast_path_cusRsp401_func=(int (*)(struct sk_buff *pskb))_cusRsp401_func;
	else
		fast_path_cusRsp401_func=(int (*)(struct sk_buff *pskb))GenerateHTTP401;

	if(_cusRspTCPFinAck_func)
		fast_path_cusRspTCPFinAck_func=(int (*)(struct sk_buff *pskb))_cusRspTCPFinAck_func;
	else
		fast_path_cusRspTCPFinAck_func=(int (*)(struct sk_buff *pskb))GenerateTCPFINACK;
	
	if(_cusRspTCPEndAck_func)
		fast_path_cusRspTCPEndAck_func=(int (*)(struct sk_buff *pskb))_cusRspTCPEndAck_func;
	else
		fast_path_cusRspTCPEndAck_func=(int (*)(struct sk_buff *pskb))GenerateTCPENDACK;
	return;
}

void unregister_customRspHook(void)
{	
	fast_path_cusRsp401_func=NULL;
	fast_path_cusRspTCPFinAck_func=NULL;
	fast_path_cusRspTCPEndAck_func=NULL;
	return;
}
#endif

#if defined(CONFIG_RTL_FAST_FILTER)
extern int fast_filter(struct sk_buff *skb);
extern int rtl_fastFilterCheck(void);
extern filter_mark;
#endif

int filter_enter(void *skb)
{
#if	defined(CONFIG_RTL_FAST_FILTER)	
	int ret=0;
#endif
#ifdef DOS_FILTER
#if	defined(CONFIG_RTL_FAST_FILTER)
	if (!dos_item_global
		&& !dos_item_ipv6
		&& (TRUE!=rtl_fastFilterCheck()))
#else
	if (!dos_item_global 
		&& !dos_item_ipv6
		&& table_list_head.num <= 1)
#endif
	{
		none_dos_drop_pkt_cnt++;
//		printk("%s------->%d\n",__FUNCTION__,__LINE__);
	      return NF_ACCEPT;
	}
#endif
	  //printk("%s------->%d skb_type=%d\n",__FUNCTION__,__LINE__,rtl_get_skb_type(skb));

	  if(rtl_get_skb_type(skb) == PACKET_MULTICAST ){
	  	
#if defined(DOS_FILTER)
		if (dos_item_ipv6 & Ipv6EnableDosSet) {
			if (filter_dos(skb))  
			return NF_DROP; 	
		} 	
#endif         	     
			return NF_REPEAT;
	  }
	  
      if (rtl_get_skb_type(skb) != PACKET_HOST ) return NF_REPEAT;
	  
      if (rtl_get_skb_protocol(skb) != htons(ETH_P_IP)  	
		&&(!dos_item_ipv6||rtl_get_skb_protocol(skb) != htons(ETH_P_IPV6))) 
			return NF_REPEAT;


#if	defined(CONFIG_RTL_FAST_FILTER)	
#if 1
	if (TRUE==rtl_fastFilterCheck())
#else
	ret = need_filter(skb);
	if(ret == 1)
#endif
	{
	
		ret = fast_filter(skb);
		if(ret == NF_DROP)
			return NF_DROP;
		else if(ret == NF_MARK)
		{
			ret = NF_FASTPATH;
		}
		else if(ret == NF_FASTPATH)
		{
			ret = NF_FASTPATH;
		}
		else if(ret == NF_OMIT)
		{
			return NF_OMIT;
		}
		else if(ret == NF_LINUX)
		{
			return NF_LINUX;
		}
#if defined(CUSTOM_RSP_PACKET) 	    
		else if(fast_path_cusRsp401_func)
		{
			if(url_cache.isSent401==1)
			{
				if(fast_path_cusRspTCPFinAck_func)			 	
					fast_path_cusRspTCPFinAck_func(skb);
				return NF_DROP;
			}
			else
			{
				if(url_cache.isSentFinAck==1)
				{
					if(fast_path_cusRspTCPEndAck_func)
							fast_path_cusRspTCPEndAck_func(skb);
							return NF_DROP;
				}
			}
		}
#endif				
	}
	
#else	//CONFIG_RTL_FAST_FILTER

#ifdef URL_FILTER
	if (table_list_head.num > 1)
      {
        if(FilterWeb(skb)){				
#if defined(CUSTOM_RSP_PACKET) 
		if(fast_path_cusRsp401_func)
			fast_path_cusRsp401_func(skb);
#endif	       
	      	return NF_DROP;
	    }
#if defined(CUSTOM_RSP_PACKET) 	    
	    else if(fast_path_cusRsp401_func)
	    {
			if(url_cache.isSent401==1){
				if(fast_path_cusRspTCPFinAck_func)			 	
					fast_path_cusRspTCPFinAck_func(skb);
				return NF_DROP;
			} else {
				if(url_cache.isSentFinAck==1){
					if(fast_path_cusRspTCPEndAck_func)
						fast_path_cusRspTCPEndAck_func(skb);
					return NF_DROP;
				}
			}
	    }
#endif	
	//printk("%s------->%d\n",__FUNCTION__,__LINE__);
	#ifdef DNS_QUERY_FILTER_FOR_HTTPS_URL_FILTER
                if (rtl_ip_hdr(skb)->protocol == IPPROTO_UDP) {
                        if(FilterDnsQuery(skb)){
				    //printk("%s------->%d\n",__FUNCTION__,__LINE__);
                                return NF_DROP;
                        }
                }
        #endif
      }		
	    	
#endif  
#endif

#ifdef DOS_FILTER
      if (dos_item_global & EnableDosSet
	  	|| dos_item_ipv6 & Ipv6EnableDosSet) 
	  {     
	      	if (filter_dos(skb))  
			return NF_DROP;		
      } 	
#endif         	     

      if (rtl_ip_hdr(skb)->protocol == IPPROTO_ICMP ) return NF_REPEAT;
      // here may add the ip_MISMATCH check

#if defined(CONFIG_UDP_FRAG_CACHE)
	//hyking:fast path for fragment is ready...
	
#else
	if (rtl_ip_hdr(skb)->frag_off & htons(IP_MF|IP_OFFSET)) return NF_REPEAT;
#endif

#ifdef DOS_FILTER
	none_dos_drop_pkt_cnt++;	  
#endif         	     

      return NF_ACCEPT;
}

#if defined(CONFIG_PROC_FS)

#ifdef URL_CONTENT_AUTHENTICATION
static struct proc_dir_entry *res0=NULL;
#endif


#ifdef DOS_FILTER
static struct proc_dir_entry *res3=NULL;
#endif
#endif

#ifdef DOS_LOG_SENDMAIL
static struct proc_dir_entry *res7=NULL;
#endif

#if defined(CONFIG_RTL_FAST_FILTER)
extern 	fast_filter_init();
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
extern int igmp_delete_init_netlink(void) ;
#endif
extern void dos_func_init(void);
int __init filter_init(void)
{
#ifdef DOS_FILTER
	//Initial
	dos_item_global=0;
	dos_item_ipv6=0;
	none_dos_drop_pkt_cnt=0;
	dos_func_init();
#endif
 
#ifdef URL_FILTER
	//Initial
	url_count=0;
#if defined(CUSTOM_RSP_PACKET) && defined (ENQUEUED_RSP_SKB)
	 initTcpRspSkbQueue();
#endif
#endif // end of URL_FILTER

#if defined(CONFIG_PROC_FS)
#ifdef URL_CONTENT_AUTHENTICATION
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	res0 = proc_create_data("auth_url_content", 0, &proc_root,
			 &auth_url_content_proc_fops, NULL);
	#else
	res0 = create_proc_entry("auth_url_content",0,NULL);
	if(res0)
	{
		res0->read_proc = auth_url_content_read_proc;
		res0->write_proc = auth_url_content_write_proc;
	}
	#endif
#endif

#ifdef DOS_FILTER      
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	res3 = proc_create_data("enable_dos", 0, &proc_root,
			 &dos_proc_fops, NULL);
#else
      res3 = create_proc_entry("enable_dos", 0, NULL);
      if (res3) {
	    res3->read_proc = dos_read_proc;
	    res3->write_proc = dos_write_proc;
      }      
#endif
      init_timer(&dos_timer);
      dos_timer.expires  = jiffies + HZ;
      dos_timer.data     = 0L;
      dos_timer.function = dos_timer_fn;
      mod_timer(&dos_timer, jiffies + HZ);
      
#endif


#ifdef DOS_LOG_SENDMAIL
	//initial
	dosLogCount=0;
	
	res7 = create_proc_entry("dos_log_count", 0, NULL);
	if(res7)
	{
		res7->read_proc = dos_log_count_read_proc;
		res7->write_proc = dos_log_count_write_proc;
	}
#endif
#if defined(CONFIG_RTL_FAST_FILTER)
	fast_filter_init();
#elif defined(FASTPATH_FILTER)
	filter_init_fastpath();
#endif
#if defined(CONFIG_RTL_IGMP_SNOOPING)
	igmp_delete_init_netlink();
#endif

#endif // CONFIG_PROC_FS
	
	return 0;
}

void __exit filter_exit(void)
{
#if defined(CONFIG_PROC_FS)
#ifdef DOS_FILTER 
	if (res3) {
		remove_proc_entry("enable_dos", res3);	
		res3 = NULL;
	}
	del_timer_sync(&dos_timer);		
#endif		
#if	defined(CONFIG_RTL_FAST_FILTER)
#else
#ifdef FASTPATH_FILTER
	filter_exit_fastpath();
#endif
#endif
#endif // CONFIG_PROC_FS
}
#endif

