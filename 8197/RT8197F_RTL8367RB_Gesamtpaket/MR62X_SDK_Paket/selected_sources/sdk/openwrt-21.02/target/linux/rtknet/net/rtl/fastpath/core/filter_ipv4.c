#include <linux/proc_fs.h>
#include <linux/string.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <linux/icmp.h>
#include <net/tcp.h>//brad
#include <net/rtl/rtl_types.h>
#include <net/rtl/fastpath/fastpath_core.h>


#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

#define HashSize 256
#define TableSize 1024
#define HighSensitivity 10
#define LowSensitivity 200

#define TCP_FIN 1
#define TCP_SYN 2
#define TCP_RST 4
#define TCP_ACK 16

//#define SmartHighThres 4000
//#define SmartLowThres  500
//#define control_message 0x8000
//#define connect_control 0xc
//#define stop_control 0x4

struct s_dos_pkt {
	char use;
	int syn_cnt;
	int fin_cnt;
	int udp_cnt;
	int icmp_cnt;
	int ping_cnt;
	int  scan_cnt;
	u_int32_t ip;
	u_int16_t id;
	u_int16_t offset;
	u_int16_t dest;
};

/*for dos_item_global*/
enum {
	EnableDosSet=0x1,
	WholeSynFloodSet=0x2,
	WholeFinFloodSet=0x4,
	WholeUdpFloodSet=0x8,
	WholeIcmpFloodSet=0x10,
	PerSynFloodSet=0x20,
	PerFinFloodSet=0x40,
	PerUdpFloodSet=0x80,
	PerIcmpFloodSet=0x100,
	TcpUdpPortScanSet=0x200,
	IcmpSmurfSet=0x400,
	IpLandSet=0x800,
	IpSpoofSet=0x1000,
	TearDropSet=0x2000,
	PingOfDeathSet=0x4000,
	TcpScanSet=0x8000,
	TcpSynWithDataSet=0x10000,
	UdpBombSet=0x20000,
	UdpEchoChargenSet=0x40000,
	UdpFraggleSet=0x80000,
	WholePingFloodSet=0x100000,
	PerPingFloodSet=0x200000,
	IpBlockSet=0x400000,
	SensitivitySet=0x800000,
	IcmpEchoStormSet=0x1000000,
	UdpSnorkAttackSet=0x2000000,
	TcpInvalidFlagsSet=0x4000000,
	TcpRstAttackSet=0x8000000,
	BcMcSrcIpAttackSet=0x10000000,
    ShortIpPktSet=0x20000000,
    IcmpPingSweepSet=0x40000000,
    IcmpEchoWithoutReplySet = 0x80000000,
};

/*for dos_item_global_0*/
enum{
    TcpInvalidOptionsSet = 0x1,
    LoopBackEchoChargenSet = 0x2,
};

enum {
	PerSynFlood=1, //1
	PerFinFlood, //2
	PerUdpFlood, //3
	PerIcmpFlood, //4
	PerPingFlood, //5
	TcpUdpPortScan,//6
	IcmpSmurf, //7
	IpLand, //8
	IpSpoof, //9
	TearDrop, //10
	PingOfDeath, //11
	TcpScan, //12
	TcpSynWithData,//13
	UdpBomb, //14
	UdpEchoChargen,//15
	UdpFraggle, //16
	IcmpEchoStorm, //17
	UdpSnorkAttack, //18
	TcpInvalidFlags, //19
	TcpRstAttack, //20
	BcMcSrcIpAttack, //21
    ShortIpPkt,//22
    IcmpPingSweep, //23
    IcmpEchoWithoutReply,//24
    TcpInvalidOptions, //25
    LoopBackEchoChargen, //26
	MaxDosType //27
};
static char *rtl_dos_name[] = {
    "NULL",
	"PerSynFlood", //1
	"PerFinFlood", //2
	"PerUdpFlood", //3
	"PerIcmpFlood", //4
	"PerPingFlood", //5
	"TcpUdpPortScan",//6
	"IcmpSmurf", //7
	"IpLand", //8
	"IpSpoof", //9
	"TearDrop", //10
	"PingOfDeath", //11
	"TcpScan", //12
	"TcpSynWithData",//13
	"UdpBomb", //14
	"UdpEchoChargen",//15
	"UdpFraggle", //16
	"IcmpEchoStorm", //17
	"UdpSnorkAttack", //18
	"TcpInvalidFlags", //19
	"TcpRstAttack", //20
	"BcMcSrcIpAttack", //21
    "ShortIpPkt",//22
    "IcmpPingSweep", //23
    "IcmpEchoWithoutReply",//24
    "TcpInvalidOptions", //25
    "LoopBackEchoChargen",//26
	"MaxDosType" //27
};

__DRAM_L34_FWD extern u_int32_t dos_item_global;
__DRAM_L34_FWD extern u_int32_t dos_item_global_0;
extern int dosFilter_block;

static char _tcpDosScanBitmap[64];
static u_int32_t LogFlag;
static struct s_dos_pkt dos_pkt[TableSize], *cur_p_pkt;
static u_int32_t attack_saddr[MaxDosType],attack_daddr[MaxDosType];

extern int whole_syn_threshold;
extern int whole_fin_threshold;
extern int whole_udp_threshold;
extern int whole_icmp_threshold;
extern int per_syn_threshold;
extern int per_fin_threshold;
extern int per_udp_threshold;
extern int per_icmp_threshold;
extern int per_ping_threshold;
extern char dos_op_mode;

extern u_int32_t ConnectedIp[HashSize];
extern u_int32_t dos_lan_addr,dos_lan_mask;
extern char dos_flag[80];

#if 1
extern int scrlog_printk(const char * fmt, ...);
#else
#if 1
#define scrlog_printk printk
#else
#define scrlog_printk(format, args...)
#endif
#endif

#define IPNAME_LEN 16
static char ip_name0[IPNAME_LEN];
static char ip_name1[IPNAME_LEN];

static char *IP2NAME(const unsigned int ip, char *buf)  
{  
    memset(buf,0,IPNAME_LEN);
    sprintf(buf, "%u.%u.%u.%u",  
        (unsigned char )*((char *)&ip + 0),  
        (unsigned char )*((char *)&ip + 1),  
        (unsigned char )*((char *)&ip + 2),  
        (unsigned char )*((char *)&ip + 3));  
    return buf;  
}

void dos_read_proc_handler(struct seq_file *s, void *v)
{
	  int i;
      struct s_dos_pkt *p_pkt;
      seq_printf(s, "%s\n", dos_flag);
      for(i = 0; i < HashSize; i++)
      {
          if(ConnectedIp[i] != 0)
          {
              seq_printf(s,"connectip = %s\n",IP2NAME(ConnectedIp[i],ip_name0));
          }
      }
	  
      p_pkt=&dos_pkt[0];
      local_bh_disable();
      for(i=0; i< TableSize;i++)
      {
          if(p_pkt->use == 1)
          {
              //printk("[%s:] %d i = %d\n",__func__,__LINE__,i);
              seq_printf(s,"pkt---IP:%s, syn_cnt:%d, fin_cnt:%d, udp_nct:%d, icmp_cnt:%d, scan_cnt:%d\n",IP2NAME(p_pkt->ip,ip_name0),p_pkt->syn_cnt,p_pkt->fin_cnt,p_pkt->udp_cnt,p_pkt->icmp_cnt,p_pkt->scan_cnt);
          }
          p_pkt++;
      } 

      for(i = 0; i < MaxDosType; i++)
      {
          if((attack_daddr[i] !=0)||(attack_saddr[i] !=0))
          {
              //printk("[%s:] %d i = %d\n",__func__,__LINE__,i);
              //printk("[%s:] %d i = %d, rtl_dos_name = %s\n",__func__,__LINE__,i,rtl_dos_name[i]);
              seq_printf(s,"attack_saddr[%s]=%s,attack_daddr[%s]=%s\n",rtl_dos_name[i],IP2NAME(attack_saddr[i],ip_name0),rtl_dos_name[i],IP2NAME(attack_daddr[i],ip_name1));
          }
      }

      local_bh_enable();

      seq_printf(s,"op_mode = %d, lan_addr = %s, lan_mask = %s, dos_item_global = 0x%x,dos_item_global_0 = 0x%x\n",dos_op_mode,IP2NAME(dos_lan_addr,ip_name0),IP2NAME(dos_lan_mask,ip_name1),dos_item_global,dos_item_global_0);

}


/*add by cl*/
void clear_attack_address_ipv4(void)
{
	int i;
	for(i = 0; i < MaxDosType; i++)
	{
		attack_saddr[i] = 0;
		attack_daddr[i] = 0;
	}
	
	return;
}

/*tcp syn flood*/
static int dos_pkt_syn_flood(u_int32_t item,struct iphdr *iph,struct tcphdr *tcph)
{
    unsigned char *tflag;
    struct s_dos_pkt *p_pkt;
    p_pkt=cur_p_pkt;	
    tflag=(void *) tcph + 13;

	if(iph->protocol==IPPROTO_TCP && (*tflag & 0x3f)==TCP_SYN)
	{
	        if(dosFilter_block==1 && attack_saddr[PerSynFlood]==iph->saddr)	
			return FAILED;
      		(p_pkt->syn_cnt)++;
			
		if(p_pkt->syn_cnt > per_syn_threshold && (item & PerSynFloodSet)==PerSynFloodSet && per_syn_threshold > 0)
		{
		 	attack_saddr[PerSynFlood]=iph->saddr;
			attack_daddr[PerSynFlood]=iph->daddr;
			dosFilter_block=1;
			LogFlag |=PerSynFloodSet;
		}
	}
	return SUCCESS;

}

static int dos_pkt_fin_flood(u_int32_t item,struct iphdr *iph,struct tcphdr *tcph)
{
    unsigned char *tflag;
	struct s_dos_pkt *p_pkt;
    p_pkt=cur_p_pkt;	
    tflag=(void *) tcph + 13;
	if(iph->protocol==IPPROTO_TCP && (*tflag & 0x3f)==TCP_FIN )
	{
	        if(dosFilter_block==1 && attack_saddr[PerFinFlood]==iph->saddr)	
			return FAILED;
       		(p_pkt->fin_cnt)++;
		if(p_pkt->fin_cnt > per_fin_threshold && (item & PerFinFloodSet)==PerFinFloodSet && per_fin_threshold > 0)
		{
		 	attack_saddr[PerFinFlood]=iph->saddr;
			attack_daddr[PerFinFlood]=iph->daddr;
			dosFilter_block=1;
			LogFlag |=PerFinFloodSet;
		}
	}
	return SUCCESS;
}

static int dos_pkt_udp_flood(u_int32_t item,struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
        p_pkt=cur_p_pkt;	
	if(iph->protocol==IPPROTO_UDP)
	{
        if(dosFilter_block==1 && attack_saddr[PerUdpFlood]==iph->saddr)	
		return FAILED;
  		(p_pkt->udp_cnt)++;
		
		if(p_pkt->udp_cnt > per_udp_threshold && (item & PerUdpFloodSet)==PerUdpFloodSet && per_udp_threshold > 0)
		{
		 	attack_saddr[PerUdpFlood]=iph->saddr;
			attack_daddr[PerUdpFlood]=iph->daddr;
			dosFilter_block=1;
			LogFlag |=PerUdpFloodSet;
		}
	}
	return SUCCESS;
}

static int dos_pkt_icmp_flood(u_int32_t item,struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
        p_pkt=cur_p_pkt;	
	if(iph->protocol==IPPROTO_ICMP)
	{
	        if(dosFilter_block==1 && attack_saddr[PerIcmpFlood]==iph->saddr)	
			return FAILED;
       		(p_pkt->icmp_cnt)++;
		if(p_pkt->icmp_cnt > per_icmp_threshold && (item & PerIcmpFloodSet)==PerIcmpFloodSet && per_icmp_threshold >0)
		{
		 	attack_saddr[PerIcmpFlood]=iph->saddr;
			attack_daddr[PerIcmpFlood]=iph->daddr;
			dosFilter_block=1;
			LogFlag |=PerIcmpFloodSet;
		}
	}
	return SUCCESS;
}

static int dos_pkt_ping_flood(u_int32_t item,struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
	struct icmphdr *icmph;
	icmph=(void *) iph + iph->ihl*4;
        p_pkt=cur_p_pkt;	
	if((iph->protocol==IPPROTO_ICMP) && ((icmph->type==ICMP_ECHO) ||(icmph->type==ICMP_ECHOREPLY)))
	{
	        if(dosFilter_block==1 && attack_saddr[PerPingFlood]==iph->saddr)	
			return FAILED;
       		(p_pkt->ping_cnt)++;
		if(p_pkt->ping_cnt > per_ping_threshold && (item & PerPingFloodSet)==PerPingFloodSet && per_ping_threshold >0)
		{
		 	attack_saddr[PerPingFlood]=iph->saddr;
			attack_daddr[PerPingFlood]=iph->daddr;
			dosFilter_block=1;
			LogFlag |=PerPingFloodSet;
		}
	}
	return SUCCESS;
}
static int dos_pkt_locate(struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
	int16_t idx=0;
    idx=iph->saddr % TableSize;	
	p_pkt=&dos_pkt[idx];
	if(iph->saddr == ConnectedIp[iph->saddr % HashSize] && p_pkt->ip==iph->saddr)/*if session connected return,if lan->wan,record dst,so here check wan->lan src pkt*/
	{
		p_pkt->use=0;
		return 0;
	}
	if(p_pkt->ip!=iph->saddr && p_pkt->use ==0)/*record suspicious pkt,find an empty place, and cur_p_pkt represent monitor curent pkt*/
	{
		p_pkt->ip=iph->saddr;
		p_pkt->use=1;
 		cur_p_pkt=p_pkt;
		return 1;
	}
	else if(p_pkt->ip==iph->saddr && p_pkt->use ==1) /*already record, account this pkt on*/
	{
 		cur_p_pkt=p_pkt;		
		return 1;
	}
	else
		return 0;
}

static int _IpSpoof(struct iphdr *iph)
{
	
	if((ntohl(iph->saddr) & dos_lan_mask)==(dos_lan_addr & dos_lan_mask) && iph->protocol!=IPPROTO_ICMP)
	{
		LogFlag |=IpSpoofSet;
		attack_saddr[IpSpoof]=iph->saddr;
		attack_daddr[IpSpoof]=iph->daddr;
		return FAILED;
	}
	return SUCCESS;
}
		
static int _IcmpSmurf(struct iphdr *iph)
{
	struct icmphdr *icmph;
	icmph=(void *) iph + iph->ihl*4;
	if(iph->protocol==IPPROTO_ICMP && icmph->type == ICMP_ECHO)
	{	
	//  printk("saddr=%x,lan_addr=%x\n",ntohl(iph->saddr),dos_lan_addr);
		if((ntohl(iph->saddr) & dos_lan_mask)==(dos_lan_addr & dos_lan_mask))
		{
		   	LogFlag |=IcmpSmurfSet;
			attack_saddr[IcmpSmurf]=iph->saddr;
			attack_daddr[IcmpSmurf]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
		
}

static int _IpLand(struct iphdr *iph)
{
	if(iph->saddr==iph->daddr)
	{
        LogFlag |=IpLandSet;
		attack_saddr[IpLand]=iph->saddr;
		attack_daddr[IpLand]=iph->daddr;
		return FAILED;
	}
	
	return SUCCESS;
		   
}

/*Here block icmp echo frame whose sip is mc/bc, those icmp echo frame whose
dip is mc/bc will be blocked in icmp_rcv()*/
static int _IcmpEchoStorm(struct iphdr *iph)
{
	struct icmphdr *icmph;
	icmph=(void *) iph + iph->ihl*4;
	
	if ((iph->protocol==IPPROTO_ICMP) && ((icmph->type==ICMP_ECHO)||(icmph->type==ICMP_ECHOREPLY))) {	
		if (ipv4_is_multicast(iph->saddr) ||ipv4_is_lbcast(iph->saddr) ||((ntohl(iph->saddr)&(~dos_lan_mask))==(~dos_lan_mask))) {
	   		LogFlag |=IcmpEchoStormSet;
			attack_saddr[IcmpEchoStorm]=iph->saddr;
			attack_daddr[IcmpEchoStorm]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
		
}

/*udp fraggle attack is almost like icmp smurf attack*/
static int _UdpFraggle(struct iphdr *iph)
{
	if (iph->protocol == IPPROTO_UDP) {	
		if ((ntohl(iph->saddr)&dos_lan_mask) == (dos_lan_addr&dos_lan_mask)) {
		   	LogFlag |= UdpFraggleSet;
			attack_saddr[UdpFraggle] = iph->saddr;
			attack_daddr[UdpFraggle] = iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
		
}

/*udp Snork attack is dport=135,sport=7/19/135*/
static int _UdpSnorkAttack(struct iphdr *iph, struct udphdr *udph)
{
	if (iph->protocol == IPPROTO_UDP) {	
		if (((udph->source==htons(7))||(udph->source==htons(19))
			||(udph->source==htons(135))) && (udph->dest==htons(135))) {
		   	LogFlag |= UdpSnorkAttackSet;
			attack_saddr[UdpSnorkAttack] = iph->saddr;
			attack_daddr[UdpSnorkAttack] = iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
		
}

/*tcp invalid flags: null, syn+fin, only fin(cisco said this case, dst port < 1024).*/
static int _TcpInvalidFlags(struct iphdr *iph, struct tcphdr *tcph)
{
	unsigned char *tflag;
	tflag=(void *) tcph + 13;

	if (iph->protocol == IPPROTO_TCP) {	
		if (((*tflag&(TCP_FIN|TCP_SYN|TCP_RST|TCP_ACK))==0) ||((*tflag&(TCP_FIN|TCP_SYN))==(TCP_FIN|TCP_SYN))
			||(((*tflag&TCP_FIN)==TCP_FIN)&&((*tflag&TCP_ACK)!=TCP_ACK))) {
		   	LogFlag |= TcpInvalidFlagsSet;
			attack_saddr[TcpInvalidFlags] = iph->saddr;
			attack_daddr[TcpInvalidFlags] = iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
		
}

/*tcp rst attack protection: drop the tcp packet with rst flags.*/
static int _TcpRstAttack(struct iphdr *iph, struct tcphdr *tcph)
{
	unsigned char *tflag;
	tflag=(void *) tcph + 13;

	if ((iph->protocol==IPPROTO_TCP) && ((*tflag&TCP_RST)==TCP_RST)) {	
	   	LogFlag |= TcpRstAttackSet;
		attack_saddr[TcpRstAttack] = iph->saddr;
		attack_daddr[TcpRstAttack] = iph->daddr;
		return FAILED;
	}
	return SUCCESS;
		
}

static int _BcMcSrcIpAttack(struct iphdr *iph)
{	
	if (ipv4_is_multicast(iph->saddr) ||ipv4_is_lbcast(iph->saddr) ||((ntohl(iph->saddr)&(~dos_lan_mask))==(~dos_lan_mask))) {
   		LogFlag |=BcMcSrcIpAttackSet;
		attack_saddr[BcMcSrcIpAttack]=iph->saddr;
		attack_daddr[BcMcSrcIpAttack]=iph->daddr;
		return FAILED;
	}

	return SUCCESS;

}

static int _UdpBomb(struct iphdr *iph, struct udphdr *udph)
{
	int ipPayLoadLength;

	if(iph->protocol==IPPROTO_UDP)
	{	
        if (!(ntohs(iph->frag_off) & (IP_OFFSET|IP_MF)))
		{
        	ipPayLoadLength = ntohs(iph->tot_len) - ((iph->ihl) << 2);
       		if (ipPayLoadLength > ntohs(udph->len))
			{
	       	    LogFlag |=UdpBombSet;
				attack_saddr[UdpBomb]=iph->saddr;
				attack_daddr[UdpBomb]=iph->daddr;
				return FAILED;
			}	
		}
	}
	return SUCCESS;
}

static int _TcpSynWithData(struct iphdr *iph,struct tcphdr *tcph)
{
    unsigned char *tflag;
    tflag=(void *) tcph + 13;
	if(iph->protocol==IPPROTO_TCP && (*tflag & 0x3f)== TCP_SYN)
	{
		unsigned long datalen= ntohs(iph->tot_len)-((iph->ihl)<<2)- (tcph->doff<<2);
		if(datalen>0) {
	       	LogFlag |=TcpSynWithDataSet;
			attack_saddr[TcpSynWithData]=iph->saddr;
			attack_daddr[TcpSynWithData]=iph->daddr;
			return FAILED;
		}
		if(ntohs(iph->frag_off) & IP_MF) {
	       	LogFlag |=TcpSynWithDataSet;
			attack_saddr[TcpSynWithData]=iph->saddr;
			attack_daddr[TcpSynWithData]=iph->daddr;
			return FAILED;
		}

    }
	return SUCCESS;
}

static int _PingOfDeath(struct iphdr *iph)
{
	unsigned short iph_off = ntohs(iph->frag_off);
	unsigned long  val;

    if((iph_off & IP_MF) == 0 && (iph_off & IP_OFFSET))
    {
	    iph_off &= IP_OFFSET;
	    val = (iph_off << 3) + ntohs(iph->tot_len) -((iph->ihl) << 2);
	    if(val > 65535)
		{
	      	LogFlag |=PingOfDeathSet;
			attack_saddr[PingOfDeath]=iph->saddr;
			attack_daddr[PingOfDeath]=iph->daddr;
			return FAILED;
	    }
    }
	return SUCCESS;
}

static int _UdpEchoChargen(struct iphdr *iph, struct udphdr *udph) 
{
	if(iph->protocol==IPPROTO_UDP)
	{
        if((udph->dest==htons(7)||udph->dest==htons(17)||udph->dest==htons(19)) || (udph->source==htons(7)||udph->source==htons(17)||udph->source==htons(19)))
		{
			LogFlag |=UdpEchoChargenSet;
			attack_saddr[UdpEchoChargen]=iph->saddr;
			attack_daddr[UdpEchoChargen]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
}

static int _LoopBackEchoChargen(struct iphdr *iph,struct udphdr *udph)
{
	if((iph->protocol==IPPROTO_UDP)||(iph->protocol==IPPROTO_TCP))
	{
        /*the sport & dport have same offset in udp and tcp header*/
        if(((udph->dest==htons(7)||udph->dest==htons(17)||udph->dest==htons(19)) || (udph->source==htons(7)||udph->source==htons(17)||udph->source==htons(19)))&&(iph->saddr == iph->daddr))
		{
			attack_saddr[LoopBackEchoChargen]=iph->saddr;
			attack_daddr[LoopBackEchoChargen]=iph->daddr;
			return FAILED;
		}
	}
	return SUCCESS;
}

/*too short ip packet*/
static int _ShortIpPkt(struct iphdr *iph)
{
	int ipPayLoadLength;
    int least_length;
	if(iph->protocol==IPPROTO_ICMP)
    {
        if(!((ntohs(iph->frag_off)) & (IP_MF | IP_OFFSET)))  //no icmp tiny frag attack?
        {
            ipPayLoadLength = ntohs(iph->tot_len)-((iph->ihl) << 2);
            if(ipPayLoadLength < 4)
            {
                LogFlag |= ShortIpPktSet;
                attack_saddr[ShortIpPkt]=iph->saddr;
                attack_daddr[ShortIpPkt]=iph->daddr;
                return FAILED;
            }
        }
    }
    //if ip fragment is not zero, is a tiny fragment attack, else is a too short tcp/udp header attack
    //if(!((ntohs(iph->frag_off)) & (IP_MF | IP_OFFSET)))
	if((iph->protocol==IPPROTO_UDP) ||(iph->protocol==IPPROTO_TCP))
    {
        ipPayLoadLength = ntohs(iph->tot_len) - ((iph->ihl) << 2);
        if(iph->protocol==IPPROTO_UDP)
            least_length = 8;
        else
            least_length = 20;

        //printk("[%s:] %d,ipPayLoadLength = %d\n",__func__,__LINE__,ipPayLoadLength);
        if (ipPayLoadLength < least_length) 
        {
            LogFlag |=ShortIpPktSet;
            attack_saddr[ShortIpPkt]=iph->saddr;
            attack_daddr[ShortIpPkt]=iph->daddr;
            return FAILED;
        }
    }
    return SUCCESS;
}

/*just disable icmp echo request*/
static int _ICMP_Ping_Sweep(struct iphdr *iph)
{
	struct icmphdr *icmph;
    icmph=(void *) iph + iph->ihl*4;
	if ((iph->protocol==IPPROTO_ICMP) && ((icmph->type==ICMP_ECHO)))
    {
        LogFlag |=IcmpPingSweepSet;
        attack_saddr[IcmpPingSweep]=iph->saddr;
        attack_daddr[IcmpPingSweep]=iph->daddr;
        return FAILED;
    }
    return SUCCESS;
}

static int _TcpInvalidOptions(struct iphdr *iph)
{
    struct tcphdr *tcph;	
    //unsigned long long debug_cl;
    unsigned int tcphdr_len;
    unsigned char *option_p;
    unsigned int handle_len;
    //debug_cl = 4;
    //printk("[%s:] %d, sizeof(debug_cl) = %d\n",__func__,__LINE__,sizeof(debug_cl));
    
	if(iph->protocol==IPPROTO_TCP)
    {
        tcph=(void *) iph + iph->ihl*4;
        tcphdr_len=((*((unsigned char *)((void *) tcph + 12)))&0xf0)>>2;
        //printk("[%s:] %d, tcphdr_len = %d\n",__func__,__LINE__,tcphdr_len);
        if(tcphdr_len > 20) /*must be have tcp option*/
        {
            handle_len = 0;
            option_p =(unsigned char *)( (void *)tcph + 20);
            while(handle_len < tcphdr_len -20)
            {
                switch(*option_p)
                {
                    case 0:
                        //printk("[%s:] %d, handle_len: %d\n",__func__,__LINE__,handle_len);
                        option_p += 1;
                        handle_len++;
                        break;
                    case 1:
                        //printk("[%s:] %d, handle_len: %d\n",__func__,__LINE__,handle_len);
                        option_p += 1;
                        handle_len++;
                        break;
                    case 2:
                        //printk("[%s:] %d, handle_len: %d,option_len:%d\n",__func__,__LINE__,handle_len,*(option_p+1));
                        if(*(option_p+1) != 4) /*tmss option len must be 4 bytes*/
                        {
                            //LogFlag |=TcpInvalidOptionsSet;
                            attack_saddr[TcpInvalidOptions]=iph->saddr;
                            attack_daddr[TcpInvalidOptions]=iph->daddr;
                            return FAILED;
                        }
                        option_p += 4;
                        handle_len += 4;
                        break;
                    case 3:
                        //printk("[%s:] %d, handle_len: %d,option_len:%d\n",__func__,__LINE__,handle_len,*(option_p+1));
                        if(*(option_p+1) != 3) 
                        {
                            //LogFlag |=TcpInvalidOptionsSet;
                            attack_saddr[TcpInvalidOptions]=iph->saddr;
                            attack_daddr[TcpInvalidOptions]=iph->daddr;
                            return FAILED;
                        }
                        option_p += 3;
                        handle_len += 3;
                        break;
                    case 8:
                        //printk("[%s:] %d, handle_len: %d,option_len:%d\n",__func__,__LINE__,handle_len,*(option_p+1));
                        if(*(option_p+1) != 10) 
                        {
                            //LogFlag |=TcpInvalidOptionsSet;
                            attack_saddr[TcpInvalidOptions]=iph->saddr;
                            attack_daddr[TcpInvalidOptions]=iph->daddr;
                            return FAILED;
                        }
                        option_p += 10;
                        handle_len += 10;
                        break;
                    default:
                        //printk("[%s:] %d, handle_len: %d\n",__func__,__LINE__,handle_len);
                        //LogFlag |=TcpInvalidOptionsSet;
                        attack_saddr[TcpInvalidOptions]=iph->saddr;
                        attack_daddr[TcpInvalidOptions]=iph->daddr;
                        return FAILED;
                }
            }
        }
    }
    return SUCCESS;
}

/*only enable icmp echo reply msg if icmp request out in most 2 dos_timer_fn time */
static int icmp_echo_request;
static unsigned long icmp_echo_request_time;

int filter_out(void *skb)
{
	struct iphdr *iph;
	struct icmphdr *icmph;
	iph=rtl_ip_hdr(skb);
    icmph=(void *) iph + iph->ihl*4;
	if ((iph->protocol==IPPROTO_ICMP) && ((icmph->type==ICMP_ECHO)))
	{
		icmp_echo_request_time = jiffies;
		icmp_echo_request=1;
	}
	return 0;
}

static int _ICMP_Echo_Reply_Without_Request(struct iphdr *iph)
{
	struct icmphdr *icmph;
    icmph=(void *) iph + iph->ihl*4;
	if ((iph->protocol==IPPROTO_ICMP) && ((icmph->type==ICMP_ECHOREPLY)))
    {
		if(!icmp_echo_request) 
		{
            LogFlag |=IcmpEchoWithoutReplySet;
            attack_saddr[IcmpEchoWithoutReply]=iph->saddr;
            attack_daddr[IcmpEchoWithoutReply]=iph->daddr;
		    return FAILED;
		}
    }
    return SUCCESS;
}

static int _TcpScan(struct iphdr *iph,struct tcphdr *tcph)
{
        unsigned char *tflag;
        tflag=(void *) tcph + 13;
	if(iph->protocol==IPPROTO_TCP)
	{	
		if(_tcpDosScanBitmap[*tflag & 0x3f])
		{
	        //printk("[%s:] %d, tflag = %d\n",__func__,__LINE__,*tflag);
			LogFlag |=TcpScanSet;
			attack_saddr[TcpScan]=iph->saddr;
			attack_daddr[TcpScan]=iph->daddr;
			return FAILED; //1
		}
	}
	return SUCCESS; //0
}

static int _TearDrop(struct iphdr *iph)
{
	struct s_dos_pkt *p_pkt;
	if(dos_pkt_locate(iph))
         p_pkt=cur_p_pkt;	
    else	
		return SUCCESS;
	
	if(ntohs(iph->id)!=p_pkt->id && ntohs(p_pkt->id) !=0)
		return SUCCESS;
	
	if((ntohs(iph->frag_off)) & (IP_MF | IP_OFFSET))
	{
    	if(((ntohs(iph->frag_off) & IP_OFFSET) << 3) >= p_pkt->offset){
			if(!(ntohs(iph->frag_off) & IP_MF)){
                    	p_pkt->id=0;
                    	p_pkt->offset=0;
			} else {
				p_pkt->id=ntohs(iph->id);
				p_pkt->offset=p_pkt->offset + ntohs(iph->tot_len)-((iph->ihl) << 2);
			}
		} else {			
	       	LogFlag |=TearDropSet;
			attack_saddr[TearDrop]=iph->saddr;
			attack_daddr[TearDrop]=iph->daddr;
			return FAILED;
		}			
	}
	return SUCCESS;
}

static int _TcpUdpPortScan(struct iphdr *iph, struct tcphdr *tcph,struct udphdr *udph) 
{
	struct s_dos_pkt *p_pkt;
    p_pkt=cur_p_pkt;	
	
	if(iph->protocol==IPPROTO_TCP)
	{
		if(p_pkt->dest != 0 && p_pkt->dest!=tcph->dest)  
			(p_pkt->scan_cnt)++;
		if(p_pkt->dest == 0)  
			p_pkt->dest=tcph->dest;
	}
	if(iph->protocol==IPPROTO_UDP)
	{
		if(p_pkt->dest != 0 && p_pkt->dest!=udph->dest)  
			(p_pkt->scan_cnt)++;
		if(p_pkt->dest == 0)  
			p_pkt->dest=udph->dest;
	}
	       
	if((dos_item_global & SensitivitySet)==SensitivitySet && p_pkt->scan_cnt > HighSensitivity)
	{
	    LogFlag |=TcpUdpPortScanSet;
		attack_saddr[TcpUdpPortScan]=iph->saddr;
		attack_daddr[TcpUdpPortScan]=iph->daddr;
	}
	if((dos_item_global & SensitivitySet)!=SensitivitySet && p_pkt->scan_cnt > LowSensitivity)
	{
	    LogFlag |=TcpUdpPortScanSet;
		attack_saddr[TcpUdpPortScan]=iph->saddr;
		attack_daddr[TcpUdpPortScan]=iph->daddr;
	}
	  
	return SUCCESS;
}

static void dos_whole_flood(void)
{
	struct s_dos_pkt *p_pkt;
	int whole_syn_pkt=0;
	int whole_fin_pkt=0;
	int whole_udp_pkt=0;
	int whole_icmp_pkt=0;
	int idx;
	if((dos_item_global & ( WholeSynFloodSet | WholeFinFloodSet | WholeUdpFloodSet | WholeIcmpFloodSet)))
	{
		for(idx=0,p_pkt=&dos_pkt[0]; idx< TableSize;idx++,p_pkt++)
		{
			if(p_pkt->use ==1 && (dos_item_global &  WholeSynFloodSet)==WholeSynFloodSet && p_pkt->syn_cnt >0)
				whole_syn_pkt+=p_pkt->syn_cnt;		
			if(p_pkt->use ==1 && (dos_item_global &  WholeFinFloodSet)==WholeFinFloodSet && p_pkt->fin_cnt >0)
				whole_fin_pkt+=p_pkt->fin_cnt;
			if(p_pkt->use ==1 && (dos_item_global &  WholeUdpFloodSet)==WholeUdpFloodSet && p_pkt->udp_cnt >0)
				whole_udp_pkt+=p_pkt->udp_cnt;
			if(p_pkt->use ==1 && (dos_item_global &  WholeIcmpFloodSet)==WholeIcmpFloodSet && p_pkt->icmp_cnt >0)
				whole_icmp_pkt+=p_pkt->icmp_cnt;
		}
		
		if(whole_syn_pkt > whole_syn_threshold && (dos_item_global & WholeSynFloodSet)==WholeSynFloodSet && whole_syn_threshold > 0) 
			LogFlag |=WholeSynFloodSet;		
		if(whole_fin_pkt > whole_fin_threshold && (dos_item_global & WholeFinFloodSet)==WholeFinFloodSet && whole_fin_threshold > 0)
			LogFlag |=WholeFinFloodSet;
		if(whole_udp_pkt > whole_udp_threshold && (dos_item_global & WholeUdpFloodSet)==WholeUdpFloodSet && whole_udp_threshold > 0)
			LogFlag |=WholeUdpFloodSet;
		if(whole_icmp_pkt > whole_icmp_threshold && (dos_item_global & WholeIcmpFloodSet)==WholeIcmpFloodSet && whole_icmp_threshold >0)
			LogFlag |=WholeIcmpFloodSet;

	}

}

int dos_filter_ipv4__action(void *skb)
{
	struct iphdr *iph;
	struct tcphdr *tcph;	
	struct udphdr *udph;
	int ret=0;

	iph=rtl_ip_hdr(skb);
	tcph=(void *) iph + iph->ihl*4;
	udph=(void *) iph + iph->ihl*4;	
	
	if(dos_item_global & (WholeSynFloodSet | WholeFinFloodSet | WholeUdpFloodSet  | WholeIcmpFloodSet | PerSynFloodSet | PerFinFloodSet | PerUdpFloodSet | PerIcmpFloodSet | TcpUdpPortScanSet))
	{				  
		if(dos_pkt_locate(iph))
		{
		/* all these function need variable cur_p_pkt, so use dos_pkt_locate cur_p_pkt*/
		if(dos_item_global & (WholeSynFloodSet | PerSynFloodSet)) 
			ret|=dos_pkt_syn_flood(dos_item_global,iph,tcph);			
		if(dos_item_global & (WholeFinFloodSet | PerFinFloodSet))
			ret|=dos_pkt_fin_flood(dos_item_global,iph,tcph);
		if(dos_item_global & (WholeUdpFloodSet | PerUdpFloodSet))
			ret|=dos_pkt_udp_flood(dos_item_global,iph);
		if(dos_item_global & (WholeIcmpFloodSet | PerIcmpFloodSet))
			ret|=dos_pkt_icmp_flood(dos_item_global,iph);
		if(dos_item_global & (WholePingFloodSet | PerPingFloodSet))
			ret|=dos_pkt_ping_flood(dos_item_global,iph);
		if(dos_item_global & TcpUdpPortScanSet)
			ret|=_TcpUdpPortScan(iph,tcph,udph); 
		if(!(dos_item_global & IpBlockSet))
			ret=SUCCESS; 
		}
	}

	/*all left do not need use variable cur_p_pkt */
	if(dos_item_global_0 & TcpInvalidOptionsSet)
		ret|=_TcpInvalidOptions(iph);
	if(dos_item_global_0 & LoopBackEchoChargenSet)
		ret|=_LoopBackEchoChargen(iph, udph);		
	if(dos_item_global & ShortIpPktSet)
		ret|=_ShortIpPkt(iph);
	if(dos_item_global & IcmpPingSweepSet)
		ret|=_ICMP_Ping_Sweep(iph);
	if(dos_item_global & IcmpEchoWithoutReplySet)
		ret|=_ICMP_Echo_Reply_Without_Request(iph);

	if(dos_item_global & TcpScanSet)
		ret|=_TcpScan(iph,tcph);		
	if(dos_item_global & TcpSynWithDataSet)
		ret|=_TcpSynWithData(iph,tcph); 	
	if(dos_item_global & IpLandSet)
		ret|=_IpLand(iph);		
	if(dos_item_global & UdpEchoChargenSet)
		ret|=_UdpEchoChargen(iph, udph);		
	if(dos_item_global & UdpBombSet)
			ret|=_UdpBomb(iph, udph);			
	if(dos_item_global & PingOfDeathSet)
		ret|=_PingOfDeath(iph); 	
	if(dos_item_global & IcmpSmurfSet)
		ret|=_IcmpSmurf(iph);	
	if(dos_item_global & UdpFraggleSet)
		ret|=_UdpFraggle(iph);
	if(dos_item_global & IcmpEchoStormSet)
		ret|=_IcmpEchoStorm(iph);
	if(dos_item_global & UdpSnorkAttackSet)
		ret|=_UdpSnorkAttack(iph, udph);
	if(dos_item_global & TcpInvalidFlagsSet)
		ret|=_TcpInvalidFlags(iph, tcph);
	if(dos_item_global & TcpRstAttackSet)
		ret|=_TcpRstAttack(iph, tcph);
	if(dos_item_global & BcMcSrcIpAttackSet)
		ret|=_BcMcSrcIpAttack(iph);
	if(dos_item_global & IpSpoofSet)
			ret|=_IpSpoof(iph); 			
	if(dos_item_global & TearDropSet)
		ret|=_TearDrop(iph);//here question		

	return ret;
}

static void dos_pkt_init(void)
{
	memset(dos_pkt,0,sizeof(dos_pkt));
}  

static void ShowLog(u_int32_t flag)
{
	if(flag & WholeSynFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;note:Rule:Default deny;\n");
	}
#else
	{
		scrlog_printk("DoS: Whole System SYN Flood Attack\n");
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & WholeFinFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;note:Rule:Default deny;\n");
	}
#else
	{
		scrlog_printk("DoS: Whole System FIN Flood Attack\n");
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & WholeUdpFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;note:Rule:Default deny;\n");
	}
#else	
	{
		scrlog_printk("DoS: Whole System UDP Flood Attack\n");
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & WholeIcmpFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;note:Rule:Default deny;\n");
	}
#else
	{
	scrlog_printk("DoS: Whole System ICMP Flood Attack\n");
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif
		
	if(flag & PerSynFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[PerSynFlood]),NIPQUAD(attack_daddr[PerSynFlood]));
		scrlog_printk("DoSATTACKlog_num:13;msg: Per-source SYN Flood Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[PerSynFlood]),  NIPQUAD(attack_daddr[PerSynFlood]));
	}
#else	
	{
		scrlog_printk("DoS: Per-source SYN Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[PerSynFlood]),NIPQUAD(attack_daddr[PerSynFlood]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & PerFinFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[PerFinFlood]),NIPQUAD(attack_daddr[PerFinFlood]));
		scrlog_printk("DoSATTACKlog_num:13;msg: Per-source FIN Flood Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[PerFinFlood]),  NIPQUAD(attack_daddr[PerFinFlood]));
		
	}
#else	
	{
		scrlog_printk("DoS: Per-source FIN Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[PerFinFlood]),NIPQUAD(attack_daddr[PerFinFlood]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & PerUdpFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[PerUdpFlood]),NIPQUAD(attack_daddr[PerUdpFlood]));
		scrlog_printk("DoSATTACKlog_num:13;msg: Per-source UDP Flood Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[PerUdpFlood]),  NIPQUAD(attack_daddr[PerUdpFlood]));
	}
#else
	{
		scrlog_printk("DoS: Per-source UDP Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[PerUdpFlood]),NIPQUAD(attack_daddr[PerUdpFlood]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & PerIcmpFloodSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[PerIcmpFlood]),NIPQUAD(attack_daddr[PerIcmpFlood]));
		scrlog_printk("DoSATTACKlog_num:13;msg: Per-source ICMP Flood Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[PerIcmpFlood]),  NIPQUAD(attack_daddr[PerIcmpFlood]));
	}
#else
	{
		scrlog_printk("DoS: Per-source ICMP Flood Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[PerIcmpFlood]),NIPQUAD(attack_daddr[PerIcmpFlood]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif		
	}
#endif	

	if(flag & TcpUdpPortScanSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[TcpUdpPortScan]),NIPQUAD(attack_daddr[TcpUdpPortScan]));
		scrlog_printk("DoSATTACKlog_num:13;msg: Port Scan Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[TcpUdpPortScan]),  NIPQUAD(attack_daddr[TcpUdpPortScan]));
		
	}
#else
	{
		scrlog_printk("DoS: Port Scan Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[TcpUdpPortScan]),NIPQUAD(attack_daddr[TcpUdpPortScan]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif	
	}
#endif

	if(flag & TcpScanSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[TcpScan]),NIPQUAD(attack_daddr[TcpScan]));
		scrlog_printk("DoSATTACKlog_num:13;msg:Tcp Scan Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[TcpScan]),	NIPQUAD(attack_daddr[TcpScan]));
	}
#else
	{
	scrlog_printk("DoS: Tcp Scan Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[TcpScan]),NIPQUAD(attack_daddr[TcpScan]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif	
		
	if(flag & TcpSynWithDataSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[TcpSynWithData]),NIPQUAD(attack_daddr[TcpSynWithData]));
		scrlog_printk("DoSATTACKlog_num:13;msg:Tcp SYN With Data Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[TcpSynWithData]),	NIPQUAD(attack_daddr[TcpSynWithData]));
	}
#else
	{
		scrlog_printk("DoS: Tcp SYN With Data Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[TcpSynWithData]),NIPQUAD(attack_daddr[TcpSynWithData]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & IpLandSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[IpLand]),NIPQUAD(attack_daddr[IpLand]));
		scrlog_printk("DoSATTACKlog_num:13;msg:IpLand Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[IpLand]),  NIPQUAD(attack_daddr[IpLand]));
	}
#else	
	{
		scrlog_printk("DoS: IP Land Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[IpLand]),NIPQUAD(attack_daddr[IpLand]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif		
	}
#endif

	if(flag & UdpEchoChargenSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[UdpEchoChargen]),NIPQUAD(attack_daddr[UdpEchoChargen]));
		scrlog_printk("DoSATTACKlog_num:13;msg:UdpEchoChargen Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[UdpEchoChargen]),  NIPQUAD(attack_daddr[UdpEchoChargen]));
	}
#else
	{
		scrlog_printk("DoS: UdpEchoChargen Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[UdpEchoChargen]),NIPQUAD(attack_daddr[UdpEchoChargen]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & UdpBombSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[UdpBomb]),NIPQUAD(attack_daddr[UdpBomb]));
		scrlog_printk("DoSATTACKlog_num:13;msg:UdpBomb Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[UdpBomb]),  NIPQUAD(attack_daddr[UdpBomb]));
	}
#else
	{
	scrlog_printk("DoS: UdpBomb Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[UdpBomb]),NIPQUAD(attack_daddr[UdpBomb]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif	
		
	if(flag & PingOfDeathSet)
#ifdef CONFIG_RTL8186_TR
	{
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[PingOfDeath]),NIPQUAD(attack_daddr[PingOfDeath]));
		scrlog_printk("DoSATTACKlog_num:13;msg:PingOfDeath Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[PingOfDeath]),  NIPQUAD(attack_daddr[PingOfDeath]));
	
	}
#else
	{
	scrlog_printk("DoS: PingOfDeath Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[PingOfDeath]),NIPQUAD(attack_daddr[PingOfDeath]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif	
		
	if(flag & IcmpSmurfSet)
#ifdef CONFIG_RTL8186_TR
	{  
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[IcmpSmurf]),NIPQUAD(attack_daddr[IcmpSmurf]));
		scrlog_printk("DoSATTACKlog_num:13;msg:IcmpSmurf Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[IcmpSmurf]),  NIPQUAD(attack_daddr[IcmpSmurf]));
	}	
#else	
	{
		scrlog_printk("DoS: IcmpSmurf Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[IcmpSmurf]),NIPQUAD(attack_daddr[IcmpSmurf]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif

	if(flag & IpSpoofSet)
#ifdef  CONFIG_RTL8186_TR
	{  
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[IpSpoof]),NIPQUAD(attack_daddr[IpSpoof]));
		scrlog_printk("DoSATTACKlog_num:13;msg:IpSpoof Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[IpSpoof]),  NIPQUAD(attack_daddr[IpSpoof]));
	}
#else
	{
	scrlog_printk("DoS: IpSpoof Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[IpSpoof]),NIPQUAD(attack_daddr[IpSpoof]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif	
		
	if(flag & TearDropSet)
#ifdef  CONFIG_RTL8186_TR
	{  
		scrlog_printk("DoSDROPlog_num:13;msg:Drop packet from WAN;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Rule:Default deny;\n", NIPQUAD(attack_saddr[TearDrop]),	NIPQUAD(attack_daddr[TearDrop]));
		scrlog_printk("DoSATTACKlog_num:13;msg:TearDrop Attack Detect;src:%u.%u.%u.%u;dst:%u.%u.%u.%u;note:Packet Dropped;\n", NIPQUAD(attack_saddr[TearDrop]),  NIPQUAD(attack_daddr[TearDrop]));
	}
#else
	{
		scrlog_printk("DoS: TearDrop Attack source=%u.%u.%u.%u destination=%u.%u.%u.%u\n", NIPQUAD(attack_saddr[TearDrop]),NIPQUAD(attack_daddr[TearDrop]));
#ifdef DOS_LOG_SENDMAIL
		dosLogCount++;
#endif
	}
#endif	 	
	 			
	LogFlag=0;	
}

void dos_timer_ipv4_handler(void)
{	
	dos_whole_flood(); /*aount all the dos pkt, add HZ every time*/
	ShowLog(LogFlag); /*print assured attack address, in HZ*/
	dos_pkt_init(); /*clear the suspicious dos and dos pkt record every HZ*/
	
	if(time_after(jiffies ,icmp_echo_request_time + HZ))
			icmp_echo_request = 0;
}

void dos_ipv4_init(void)
{	
	memset(&_tcpDosScanBitmap[0], 0, sizeof(_tcpDosScanBitmap));
	_tcpDosScanBitmap[0]=_tcpDosScanBitmap[3]=_tcpDosScanBitmap[8]=_tcpDosScanBitmap[9]= _tcpDosScanBitmap[32]=_tcpDosScanBitmap[33]=_tcpDosScanBitmap[40]=_tcpDosScanBitmap[41]= _tcpDosScanBitmap[58]=_tcpDosScanBitmap[63]=1;
}

