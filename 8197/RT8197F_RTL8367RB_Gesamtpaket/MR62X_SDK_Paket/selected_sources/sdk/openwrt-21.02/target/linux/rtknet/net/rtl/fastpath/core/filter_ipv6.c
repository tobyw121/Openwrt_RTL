#include <linux/proc_fs.h>
#include <linux/string.h>
//#include <net/ip.h>
#include <net/protocol.h>
//#include <linux/icmp.h>
#include <net/tcp.h>//brad
#include <net/rtl/rtl_types.h>
#include <net/rtl/fastpath/fastpath_core.h>

#define NIP6(addr) \
    ntohs((addr).s6_addr16[0]), \
    ntohs((addr).s6_addr16[1]), \
    ntohs((addr).s6_addr16[2]), \
    ntohs((addr).s6_addr16[3]), \
    ntohs((addr).s6_addr16[4]), \
    ntohs((addr).s6_addr16[5]), \
    ntohs((addr).s6_addr16[6]), \
    ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"

#define TableSize 1024

struct s_dos_pkt {
	char use;	
	int ipv6_smurf_cnt;
	int ipv6_icmp_too_big_cnt;
	int ipv6_ns_cnt;
	struct in6_addr ipv6_addr;
};

static struct s_dos_pkt dos_pkt[TableSize], *cur_p_pkt;

/* for dos_item_ipv6*/
enum{
   Ipv6EnableDosSet = 0x1,
   Ipv6SmurfFloodSet = 0x2,
   Ipv6IcmpTooBigFloodSet = 0x4,
   Ipv6NSFloodSet = 0x8,
};

enum{
	ipv6SmurfFlood=1,
	ipv6IcmpTooBigFlood,// 2
	ipv6NSFlood, // 3
	ipv6MaxDosType
};
static char *rtl_ipv6_dos_name[] = {
	"NULL",
	"ipv6SmurfFlood", // 1
	"ipv6IcmpTooBigFlood",// 2
	"ipv6NSFlood" // 3
};

__DRAM_L34_FWD extern u_int32_t dos_item_ipv6;
//extern u_int32_t dos_item_ipv6_backup;
static struct in6_addr attack_v6_saddr[ipv6MaxDosType],attack_v6_daddr[ipv6MaxDosType];

static u_int32_t LogIpv6Flag;
static struct s_dos_pkt smurf_pkts;
static struct s_dos_pkt ipv6Ns_pkts;

extern int Ipv6_smurf_threshold;
extern int Ipv6_icmp_too_big_threshold;
extern int Ipv6_ns_threshold;
extern int dosFilter_block;

#if 1
extern int scrlog_printk(const char * fmt, ...);
#else
#if 1
#define scrlog_printk printk
#else
#define scrlog_printk(format, args...)
#endif
#endif

static void* _rtl_get_ipv6_skip_exthdr(void *skb,uint8_t protocol)
{
	struct ipv6hdr *ip6h=ipv6_hdr(skb);
	uint8_t nexthdr;
	__be16 frag_off;
	int offset;
	if(!skb) return NULL;
	ip6h=ipv6_hdr(skb);
	if(!ip6h||ip6h->version!=6) return NULL;
	nexthdr = ip6h->nexthdr;
	//printk("%s:%d protocol=%d nexthdr=%d\n",__FUNCTION__,__LINE__,protocol,nexthdr);
	if (ipv6_ext_hdr(nexthdr)){
		offset = ipv6_skip_exthdr(skb, sizeof(struct ipv6hdr), &nexthdr, &frag_off);
		//printk("%s:%d offset=%d nexthdr=%d\n",__FUNCTION__,__LINE__,offset,nexthdr);
		if(offset<0 || nexthdr!=protocol) return NULL;
	} else if (protocol==nexthdr){
		//printk("%s:%d offset=%d nexthdr=%d\n",__FUNCTION__,__LINE__,offset,nexthdr);
		offset = sizeof(struct ipv6hdr);
	} else
		return NULL;
	
	//printk("%s:%d offset=%d nexthdr=%d\n",__FUNCTION__,__LINE__,offset,nexthdr);
	return (void*)((void *)ip6h+offset);
}
struct tcphdr * rtl_get_tcpv6_header(void *skb)
{
	return (struct tcphdr *)_rtl_get_ipv6_skip_exthdr(skb,NEXTHDR_TCP);
}
static struct icmp6hdr * rtl_get_icmpv6_header(void *skb)
{
	return (struct icmp6hdr *)_rtl_get_ipv6_skip_exthdr(skb,NEXTHDR_ICMP);
}


static void dos_pkt_init(void)
{
	memset(dos_pkt,0,sizeof(dos_pkt));
	
	memset(&smurf_pkts,0,sizeof(struct s_dos_pkt));
}      
static int dos_pkg_ipv6Smurf_flood(void* skb)
{
	struct icmp6hdr *icmp6h;
	struct ipv6hdr *ip6h;	

	icmp6h=rtl_get_icmpv6_header(skb);
	ip6h=ipv6_hdr(skb);
	
	//if(icmp6h)		
		//printk("%s------->%d icmp6_type=%d\n",__FUNCTION__,__LINE__,icmp6h->icmp6_type);
	if(!icmp6h || icmp6h->icmp6_type!=ICMPV6_ECHO_REPLY) return SUCCESS;
	//printk("%s------->%d\n",__FUNCTION__,__LINE__);

	//if(dosFilter_block==1) return FAILED;
	if((LogIpv6Flag&Ipv6SmurfFloodSet) && dosFilter_block==1) return FAILED;
	smurf_pkts.ipv6_smurf_cnt++;

	if(smurf_pkts.ipv6_smurf_cnt>Ipv6_smurf_threshold && Ipv6_smurf_threshold>0)
	{
		dosFilter_block=1;
		LogIpv6Flag |=Ipv6SmurfFloodSet;
		memcpy(&attack_v6_saddr[ipv6SmurfFlood],&ip6h->saddr,sizeof(struct in6_addr));
		memcpy(&attack_v6_daddr[ipv6SmurfFlood],&ip6h->daddr,sizeof(struct in6_addr));		
	}
	
	return SUCCESS;
}
static int dos_pkg_ipv6IcmpTooBig_flood(void* skb)
{
	struct icmp6hdr *icmp6h;
	struct ipv6hdr *ip6h;
	struct s_dos_pkt *p_pkt;

	icmp6h=rtl_get_icmpv6_header(skb);
	ip6h=ipv6_hdr(skb);
	p_pkt=cur_p_pkt;
	if(!icmp6h || icmp6h->icmp6_type!=ICMPV6_PKT_TOOBIG) return SUCCESS;
	//printk("%s------->%d icmp6h->icmp6_type=%d\n",__FUNCTION__,__LINE__,icmp6h->icmp6_type);

	if(dosFilter_block==1&& memcmp(&attack_v6_saddr[ipv6IcmpTooBigFlood],&ip6h->saddr,sizeof(struct	in6_addr))==0) 
		return FAILED;

	p_pkt->ipv6_icmp_too_big_cnt++;
	//printk("%s------->%d count=%d\n",__FUNCTION__,__LINE__,p_pkt->ipv6_icmp_too_big_cnt);

	if(p_pkt->ipv6_icmp_too_big_cnt>Ipv6_icmp_too_big_threshold && Ipv6_icmp_too_big_threshold>0)
	{
		dosFilter_block=1;
		LogIpv6Flag |=Ipv6IcmpTooBigFloodSet;
		memcpy(&attack_v6_saddr[ipv6IcmpTooBigFlood],&ip6h->saddr,sizeof(struct in6_addr));
		memcpy(&attack_v6_daddr[ipv6IcmpTooBigFlood],&ip6h->daddr,sizeof(struct in6_addr));
		
		//printk("%s:%d\n",__FUNCTION__,__LINE__);					
	}	
	return SUCCESS;
}

static int dos_pkg_ipv6Ns_flood(void* skb)
{
	struct icmp6hdr *icmp6h;
	struct ipv6hdr *ip6h;
	//struct s_dos_pkt *p_pkt;

	icmp6h=rtl_get_icmpv6_header(skb);
	ip6h=ipv6_hdr(skb);
	
	if(!icmp6h || icmp6h->icmp6_type!=NDISC_NEIGHBOUR_SOLICITATION) return SUCCESS;
	
	//printk("%s:%d src="NIP6_FMT" getsrc="NIP6_FMT"\n",__FUNCTION__,__LINE__,NIP6(attack_v6_saddr[ipv6NSFlood]),NIP6(ip6h->saddr));				

	//if(dosFilter_block==1) 
	//	return FAILED;
	if((LogIpv6Flag&Ipv6NSFloodSet)&& dosFilter_block==1) return FAILED;
	//printk("%s------->%d count=%d Ipv6_ns_threshold=%d \n",__FUNCTION__,__LINE__,ipv6Ns_pkts.ipv6_ns_cnt,Ipv6_ns_threshold);

	ipv6Ns_pkts.ipv6_ns_cnt++;

	if(ipv6Ns_pkts.ipv6_ns_cnt>Ipv6_ns_threshold && Ipv6_ns_threshold>0)
	{
		dosFilter_block=1;
		LogIpv6Flag |=Ipv6NSFloodSet;
		memcpy(&attack_v6_saddr[ipv6NSFlood],&ip6h->saddr,sizeof(struct in6_addr));
		memcpy(&attack_v6_daddr[ipv6NSFlood],&ip6h->daddr,sizeof(struct in6_addr));
		
		//printk("%s:%d src="NIP6_FMT" dst="NIP6_FMT"\n",__FUNCTION__,__LINE__,NIP6(ip6h->saddr),NIP6(ip6h->daddr));			
		//printk("%s:%d src="NIP6_FMT" dst="NIP6_FMT"\n",__FUNCTION__,__LINE__,NIP6(attack_v6_saddr[ipv6NSFlood]),NIP6(attack_v6_daddr[ipv6NSFlood]));						
	}
	
	return SUCCESS;
}

static int ipv6_dos_pkt_locate(void* skb)
{
	struct s_dos_pkt *p_pkt;
	int16_t idx=0;
	struct ipv6hdr *ip6h;

	ip6h=ipv6_hdr(skb);
	idx=ip6h->saddr.s6_addr32[3]%TableSize;
	//printk("%s:%d idx=%d\n",__FUNCTION__,__LINE__,idx);
	p_pkt=&dos_pkt[idx];
	if(memcmp(&ip6h->saddr,&p_pkt->ipv6_addr,sizeof(struct in6_addr))!=0 && p_pkt->use ==0)
	{
		memcpy(&p_pkt->ipv6_addr,&ip6h->saddr,sizeof(struct in6_addr));
		p_pkt->use=1;
 		cur_p_pkt=p_pkt;
		return 1;
	}
	else if(memcmp(&ip6h->saddr,&p_pkt->ipv6_addr,sizeof(struct in6_addr))==0 && p_pkt->use ==1)
	{
		cur_p_pkt=p_pkt;		
		return 1;
	}
	else
		return 0;
}

static void dos_whole_flood(void)
{
	struct s_dos_pkt *p_pkt;

	int ipv6_smurf_pkt=0;
	int ipv6_icmp_too_big_pkt=0;
	int ipv6_ns_pkt=0;

	int idx;
	if (dos_item_ipv6 & (Ipv6SmurfFloodSet|Ipv6IcmpTooBigFloodSet|Ipv6NSFloodSet))
	{
		for(idx=0,p_pkt=&dos_pkt[0]; idx< TableSize;idx++,p_pkt++)
		{
			if(p_pkt->use ==1 && (dos_item_ipv6 & Ipv6IcmpTooBigFloodSet) && p_pkt->ipv6_icmp_too_big_cnt >0)
				ipv6_icmp_too_big_pkt+=p_pkt->ipv6_icmp_too_big_cnt;
			if(p_pkt->use ==1 && (dos_item_ipv6 & Ipv6NSFloodSet) && p_pkt->ipv6_ns_cnt >0)
				ipv6_ns_pkt+=p_pkt->ipv6_ns_cnt;
		}

		if((dos_item_ipv6 & Ipv6SmurfFloodSet) && smurf_pkts.ipv6_smurf_cnt >0)//smurf need to check all icmpv6 echo request pkt
			ipv6_smurf_pkt+=smurf_pkts.ipv6_smurf_cnt;
		
		if(ipv6_smurf_pkt > Ipv6_smurf_threshold && (dos_item_ipv6 & Ipv6SmurfFloodSet) && Ipv6_smurf_threshold >0)
			LogIpv6Flag |=Ipv6SmurfFloodSet;
		if(ipv6_icmp_too_big_pkt > Ipv6_icmp_too_big_threshold && (dos_item_ipv6 & Ipv6IcmpTooBigFloodSet) && Ipv6_icmp_too_big_threshold >0)
			LogIpv6Flag |=Ipv6IcmpTooBigFloodSet;
		if(ipv6_ns_pkt > Ipv6_ns_threshold && (dos_item_ipv6 & Ipv6NSFloodSet) && Ipv6_ns_threshold >0)
			LogIpv6Flag |=Ipv6NSFloodSet;
	}
}

static void ShowLog(u_int32_t ipv6_flag)
{ 
	if(ipv6_flag & Ipv6SmurfFloodSet){
		scrlog_printk("DoS: ipv6 Smurf Attack source="NIP6_FMT" destination="NIP6_FMT"\n", NIP6(attack_v6_saddr[ipv6SmurfFlood]),NIP6(attack_v6_daddr[ipv6SmurfFlood]));
	}
	if(ipv6_flag & Ipv6IcmpTooBigFloodSet){
		scrlog_printk("DoS: ipv6 icmp too big Attack source="NIP6_FMT" destination="NIP6_FMT"\n", NIP6(attack_v6_saddr[ipv6IcmpTooBigFlood]),NIP6(attack_v6_daddr[ipv6IcmpTooBigFlood]));
	}
	if(ipv6_flag & Ipv6NSFloodSet){
		scrlog_printk("DoS: ipv6 neighbor solication Attack source="NIP6_FMT" destination="NIP6_FMT"\n", NIP6(attack_v6_saddr[ipv6NSFlood]),NIP6(attack_v6_daddr[ipv6NSFlood]));
	}
	
	LogIpv6Flag=0;
}

int dos_filter_ipv6__action(void *skb)
{
	int ret=0;

	//struct icmp6hdr *icmp6h=rtl_get_icmpv6_header(skb);
	//if(icmp6h)		
	//	printk("%s------->%d icmp6_type=%d\n",__FUNCTION__,__LINE__,icmp6h->icmp6_type);
	if(dos_item_ipv6 & (Ipv6SmurfFloodSet|Ipv6IcmpTooBigFloodSet|Ipv6NSFloodSet))
	{
		if(dos_item_ipv6 & (Ipv6SmurfFloodSet)) //ipv6 smurf record in smurf_pkts, and count all icmp pkts
				ret|=dos_pkg_ipv6Smurf_flood(skb);
		if(dos_item_ipv6 & Ipv6NSFloodSet)
				ret|=dos_pkg_ipv6Ns_flood(skb);  
		//printk("%s------->%d ret=%d\n",__FUNCTION__,__LINE__,ret);
		if(ipv6_dos_pkt_locate(skb))					
		{
		//printk("%s------->%d\n",__FUNCTION__,__LINE__);				
			if(dos_item_ipv6 & Ipv6IcmpTooBigFloodSet)
				ret|=dos_pkg_ipv6IcmpTooBig_flood(skb);			
		}
	}
	
	return ret;
}
	
void dos_timer_ipv6_handler(void)
{
	dos_whole_flood(); /*aount all the dos pkt, add HZ every time*/
	ShowLog(LogIpv6Flag); /*print assured attack address, in HZ*/
	dos_pkt_init(); /*clear the suspicious dos and dos pkt record every HZ*/
}

void clear_attack_address_ipv6(void)
{
	memset(attack_v6_saddr,0,sizeof(attack_v6_saddr));
	memset(attack_v6_daddr,0,sizeof(attack_v6_daddr));	
    return;
}

