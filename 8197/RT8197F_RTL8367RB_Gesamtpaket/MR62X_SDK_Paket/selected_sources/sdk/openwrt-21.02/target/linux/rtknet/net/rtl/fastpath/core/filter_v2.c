#ifdef FASTPATH_FILTER
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
#include <net/rtl/fastpath/fastpath_core.h>

#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
/*---------------------------------------------netlink--start---------------------------------------------*/
#include <net/netlink.h>
/*---------------------------------------------netlink--end---------------------------------------------*/

///////////////////////////////////////////////////////////////////////////
#ifdef URL_CONTENT_AUTHENTICATION
#include <asm/semaphore.h>
#include <linux/wait.h>
#endif

static char get_info_from_usr_space[1024];
static char log_info[32];
static int   log_enable=0;
#define URL_FILTER_MODE_BLACK	0
#define URL_FILTER_MODE_WHITE	1
static int url_filter_mode=0;
static int url_filter_flag=0;
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
#define URL_FILTER_DEFAULT_POLICY_DROP 0
#define URL_FILTER_DEFAULT_POLICY_PASS 1
static uint32 defaultRuleChanged=0;
static int default_policy=URL_FILTER_DEFAULT_POLICY_PASS;
#endif

extern struct net_device *rtl_get_wan_dev(void);
extern int rtl_strcmp_wan_dev_name(const char *dst);


/*
return value:
	0 don't need filter
	1 need filter
*/
int need_filter(void *skb)
{
	struct iphdr *iph;
	struct in_device *in_dev;
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	void *landev;
	void *wandev;

	iph = rtl_ip_hdr(skb);

	if ((landev = (void*)rtl_get_dev_by_name(RTL_PS_BR0_DEV_NAME)) != NULL)
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
					return 0;
				}
			}
		}
	}

	if ((wandev = (void*)rtl_get_dev_by_name(RTL_PS_PPP0_DEV_NAME)) != NULL){
      		ifa =NULL;
		if ((in_dev=__in_dev_get_rcu(wandev)) != NULL) {
			for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next) {
				if (strcmp(RTL_PS_PPP0_DEV_NAME, ifa->ifa_label) == 0){
					break; 
				}
			}
			//accept the http packet if the dest ip is our wan ip, always
			if(ifa !=NULL){
				if(iph->daddr == ifa->ifa_local){
					return 0;
				}
			}
		}
	}
	else if ((wandev = rtl_get_wan_dev()) != NULL)
	{
			ifa =NULL;
			if ((in_dev=__in_dev_get_rcu(wandev)) != NULL) {
				for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next) {
				if (rtl_strcmp_wan_dev_name(ifa->ifa_label)==0)
					{
						break; 
					}
				}
			//accept the http packet if the dest ip is our wan ip, always
			if(ifa != NULL){
				if(iph->daddr == ifa->ifa_address){
					return 0;
				}
			}
		}
	}
	else if ((wandev = rtl_get_dev_by_name(RTL_PS_WLAN0_DEV_NAME)) != NULL){
			ifa =NULL;
      			if ((in_dev=__in_dev_get_rcu(wandev)) != NULL) {
      				for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next) {
					if (strcmp(RTL_PS_WLAN0_DEV_NAME, ifa->ifa_label) == 0){
						break; 
					}
				}
				//accept the http packet if the dest ip is our wan ip, always
				if(ifa !=NULL){
					if(iph->daddr == ifa->ifa_address){
						return 0;
					}
				}
		}
	}
	return 1;
}
#if 0
#if 0
/**
*Ip range filter:
* flag 
*	0: src ipaddress
*	1: dst ipaddress
*/
static int rtl_isInipRange_fastpath(struct list_head *listhead, ipaddr_t ipAddr,uint32 flag)
{
	//printk("%s-----%d\n",__FUNCTION__,__LINE__);
	int ret = 0, match_action=0;	
 	rtl_ipRange_fastpath *entry=NULL;
	struct list_head *lh;	

	if(filter_table_cnt[0]==0) return 0;
	
	list_for_each(lh, &filter_table_entry[0].filter_items)
 	{
		entry=list_entry(lh, rtl_ipRange_fastpath, list);
		/*flag 0: src */
		if((flag == 0) )
		{
			if((entry->flag & 0x1 <<2)|| (!(entry->flag & 0x1 <<1)))
			{
				if(entry->addr_start<= ipAddr && entry->addr_end>= ipAddr)
				{
					ret = 1;
					match_action=entry->flag & 0x1;
					break;
				}
			}
		}	
		else
		{/*flag 1: dest */
			if((entry->flag & 0x1 <<2)|| (entry->flag & 0x1 <<1))
			{
				if(entry->addr_start<= ipAddr && entry->addr_end>= ipAddr)
				{
					ret = 1;
					match_action=entry->flag & 0x1;
					break;
				}
			}	
		}
		continue;
	}
	printk("ret=%d	match_actionr=%d\n",ret,match_action);
	#if 1
	if(ret)
	{
		return match_action?0:1;
	}
	else
	{
		return 0;
	}	
	#else
	return ret;
	#endif
}
#endif
/**
* mac address
*/
#endif
static int scan_sch_url_tbl(const char *data, size_t dlen, const char *pattern, size_t plen, char term)
{
	int i;
	/*
	if(pattern[plen] == '\0')
	{
		plen--;
	}
	*/
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

unsigned long str2hexnum(unsigned char *p)
{
  unsigned long val=0;
  unsigned long c=0;
  for(val=0;*p!='\0';val=(val<< 4 ) + c, p++)
  {
     c=*p;
     if(c >= '0' && c <='9')
	   c=c-'0';
     else if(c >='a' && c <='f')
	   c=c-'a'+10;
     else if(c >='A' &&  c<='F')
	 	c=c-'A'+10;
     else
	   return -1;
  }
  return val;

}

static int find_pattern(const char *data, size_t dlen, const char *pattern, size_t plen, char term, unsigned int *numoff, unsigned int *numlen)
{
	size_t i,j,k;
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
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
static int touperString(char* str)
{
	int i;
	if(str==NULL)return -1;
	for(i=0;i<strlen(str);i++)
	{
		if(str[i]>='a'&&str[i]<='z')
			str[i]-=0x20;
	}
	return 0;
}
static int url_filter_ip_in_list(uint32 ip)
{
	//printk("%s\n",__FUNCTION__);
	struct list_head *lh,*lh2;
	filter_table_list *tbl_entry;
	filter_item_entry *entry;
	list_for_each(lh,&table_list_head.table_list)
	{
		tbl_entry=list_entry(lh,filter_table_list,table_list);
		if(tbl_entry->type==URL_KEY_TABLE)
		{
			list_for_each(lh2,&tbl_entry->item_list)
			{
				entry=list_entry(lh2,filter_item_entry,item_list);
				if(entry->userMode=='I'&&entry->ipAddr==ip)
				{
					//printk("found\n");
					return 1;
				}
			}
		}
	}
	return 0;
}
static int url_filter_mac_in_list(char* mac)
{
	struct list_head *lh,*lh2;	
	filter_table_list *tbl_entry;
	filter_item_entry *entry;
	list_for_each(lh,&table_list_head.table_list)
	{
		tbl_entry=list_entry(lh,filter_table_list,table_list);
		if(tbl_entry->type==URL_KEY_TABLE)
		{
			list_for_each(lh2,&tbl_entry->item_list)
			{
				entry=list_entry(lh2,filter_item_entry,item_list);
				if(entry->userMode=='M'&&strcmp(entry->macAddr,mac)==0)
				{
					//printk("found\n");
					return 1;
				}
			}
		}
	}
	return 0;
}
#endif

extern struct tcphdr *rtl_get_tcp_header(void *skb);
extern int get_payload_len(void *skb);

/**
* url/keywords filter
*/
int rtl_url_filter_fastpath(void *skb, void *data1)
{
	struct iphdr *iph;
 	struct tcphdr *tcph;
    unsigned char *data;
	int found=0, offset,hostlen,pathlen;
	int datalen;
	char *str;
 	int ret;
	
	if(strlen(data1) <=1) return 0;
	iph=rtl_ip_hdr(skb);

	tcph = rtl_get_tcp_header(skb);
	if(!tcph)
		return 0;

	data = (void *)tcph + tcph->doff*4;
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
	filter_item_entry *entry_item=NULL;
	entry_item=list_entry(data1, filter_item_entry, data);
	char macAddr[2*ETHER_ADDR_LEN+1]={'\0'};
#endif
	//printk("saddr=%x  daddr=%x ---%d\n",iph->saddr,iph->daddr,__LINE__);
	str = kmalloc(sizeof(char) * 2048,GFP_KERNEL);
	if(str == NULL)
		return 0;

	datalen = get_payload_len(skb);
	if(!datalen)
	{
		kfree(str);
		return 0;
	}
	
	if(memcmp(data, "GET ",sizeof("GET ") -1)!=0)
	{
		kfree(str);
		return 0;
	}
	found = find_pattern(data,datalen,"Host: ",sizeof("Host: ")-1,'\r',&offset, &hostlen);
	if(!found)
	{	
		kfree(str);
		return 0;
	}
	strncpy(str,data+offset,hostlen);
	*(str+hostlen)=0;
	found = find_pattern(data,datalen,"GET ",sizeof("GET ")-1,'\r',&offset, &pathlen);
	if (!found || (pathlen -= (sizeof(" HTTP/x.x") - 1)) <= 0)
	{
		kfree(str);
		return 0;	
	}
	strncpy(str+hostlen,data+offset,pathlen);
	*(str+hostlen+pathlen)='\0';

#if CONFIG_URL_FILTER_USER_MODE_SUPPORT
	sprintf(macAddr,"%02X%02X%02X%02X%02X%02X",
					rtl_skb_mac_header(skb)[ETHER_ADDR_LEN+0],
					rtl_skb_mac_header(skb)[ETHER_ADDR_LEN+1],
					rtl_skb_mac_header(skb)[ETHER_ADDR_LEN+2],
					rtl_skb_mac_header(skb)[ETHER_ADDR_LEN+3],
					rtl_skb_mac_header(skb)[ETHER_ADDR_LEN+4],
					rtl_skb_mac_header(skb)[ETHER_ADDR_LEN+5]);
	if(url_filter_mode)//whitelist mode
	{
		if(!url_filter_ip_in_list(iph->saddr)&&!url_filter_mac_in_list(macAddr))
			if(!defaultRuleChanged){
				//printk("apply default rule.\n");
				if(default_policy)//default policy is pass
				{
					kfree(str);
					return 1;
				}
				else
				{
					//printk("default policy is drop\n");
					url_filter_flag = 1;
					kfree(str);
					return 0;
				}
			}
	}
	if(entry_item->userMode=='I')
	{
		if(entry_item->ipAddr==iph->saddr)
		{
			ret=scan_sch_url_tbl(str,strlen(str),data1,strlen(data1),'\0'); 
		}
		else
			ret=0;
	}
	else if(entry_item->userMode=='M')
	{
		if(strcmp(entry_item->macAddr,macAddr)==0)
		{
			ret=scan_sch_url_tbl(str,strlen(str),data1,strlen(data1),'\0');
			//if(ret!=0)printk("MAC match\n");
		}
		else
			ret=0;
	}
	else
	{
		ret=scan_sch_url_tbl(str,strlen(str),data1,strlen(data1),'\0');
	}
#else
	ret=scan_sch_url_tbl(str,strlen(str),data1,strlen(data1),'\0');		
#endif
	snprintf(log_info,sizeof(log_info),"%s",str);
	//printk("[%d]-------ret=%d\n",__LINE__,ret);
	kfree(str);
	
	/*white list and matching fails*/
	if(url_filter_mode == URL_FILTER_MODE_WHITE && ret==0){
		url_filter_flag = 1;
	}

	return ret?1:0;
}
//////////////////////////////////////////////
//
int rtl_mac_filter_fastpath(void *skb,void *data)
{
	int i;
	char *smac = data;
	
	for(i = 0; i<ETHER_ADDR_LEN;i++)
	{

		if((smac[i] & 0xff) != rtl_skb_mac_header(skb)[ETHER_ADDR_LEN+i])
			break;
	}
	//printk("%s---i=%d\n",__FUNCTION__,i);	
	if(i == ETHER_ADDR_LEN)
		return 1;
	else 
		return 0;
}

//////////////////////////////////////////////
//
static uint32 rtl_ip_range_filter_fastpath(void *skb,void *data)
{
	struct iphdr *iph;
	uint32 *start,*end;

	iph=rtl_ip_hdr(skb);
	start=(uint32 *)data;
	end=(uint32 *)(data+4);
//	printk("Source=%u.%u.%u.%u[%s]\n",NIPQUAD(iph->saddr),__FUNCTION__);
//	printk("Source=0x%x, start=0x%x, end=0x%x\n",iph->saddr,*start,*end);

	if(*start<= iph->saddr && *end>= iph->saddr)
	{
		return 1;
	}
	return 0;
}

//////////////////////////////////////////////
//
static uint32 rtl_schedule_filter_fastpath(void *skb,void *data)
{
	struct timeval tv;
	uint32 today, hour,minute;
	uint32 curtime;
	uint32 *start,*end,*day;

	start=(uint32 *)data;
	end=(uint32 *)(data+4);
	day=(uint32 *)(data+8);
	/*get system time*/
	do_gettimeofday(&tv);
	today = ((tv.tv_sec/86400) + 4)%7;
#if (!(defined(CONFIG_RTL8186_KB_N) ||defined(CONFIG_RTL8186_KB)))	
	hour = (tv.tv_sec/3600)%24;
#endif
	minute = (tv.tv_sec/60)%60;
	curtime = hour * 60 + minute;
//	printk("start=0x%x, end=0x%x, day=%x\n",*start,*end,*day);
//	printk("hour=%d, today=0x%x\n",hour,today);
	
	if((*day & (0x1 << 7)) || (*day & (0x1 << today)))
	{//day		
		if( ((*start == 0 ) && (*end == 0)) || ((*start <= curtime) && (curtime <=*end)))
		{//sch
			//printk("%s---1--%d\n",__FUNCTION__,__LINE__);
			return 1;
		}
	}
	//printk("%s--0---%d\n",__FUNCTION__,__LINE__);
	return 0;
}

filter_table_info filter_tlb[]={
	{0,NULL},
	{IP_RANGE_TABLE,(int (*)(struct sk_buff *skb, void *data)) rtl_ip_range_filter_fastpath},
	{MAC_TABLE,(int (*)(struct sk_buff *skb, void *data)) rtl_mac_filter_fastpath},
	{URL_KEY_TABLE,(int (*)(struct sk_buff *skb, void *data)) rtl_url_filter_fastpath},
	{SCHEDULT_TABLE,(int (*)(struct sk_buff *skb, void *data)) rtl_schedule_filter_fastpath}
};

static uint32 filter_rule_num;
filter_table_list table_list_head;

int filter_table_head_init(void)
{
	memset(&table_list_head,0,sizeof(filter_table_list));
	
	INIT_LIST_HEAD(&table_list_head.table_list);
	INIT_LIST_HEAD(&table_list_head.item_list);
	table_list_head.type=0;
	table_list_head.flag=0;
	table_list_head.func=NULL;
	table_list_head.num=1;
	
	return 0;
}

int filter_table_regist(int type, void * func)
{
	filter_table_list *entry;
	//printk("%s-----%d\n",__FUNCTION__,__LINE__);
	if(table_list_head.num==0)
	{
		filter_table_head_init();
	}
	
	entry = kmalloc(sizeof(filter_table_list),GFP_KERNEL);	

	entry->type=type;
	entry->flag=0;
	entry->num=1;
	entry->func=func;//need regist
	INIT_LIST_HEAD(&entry->table_list);
	INIT_LIST_HEAD(&entry->item_list);

	list_add(&entry->table_list,&table_list_head.table_list);
	table_list_head.num++;
	return 0;
}

int filter_table_flush(int flag)
{
	//struct list_head *lh,*lh2,*lh2_next;
	filter_table_list *entry,*entry_next;
	filter_item_entry *entry_item,*entry_item_next;
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
	if(url_filter_mode)
		defaultRuleChanged=0;
#endif
	list_for_each_entry_safe(entry, entry_next,&table_list_head.table_list,table_list)
	{		
		list_for_each_entry_safe(entry_item, entry_item_next,&entry->item_list,item_list)
		{				
			list_del(&entry_item->item_list);
			kfree(entry_item);
			entry->num--;
		}
		
		if(entry->type == URL_KEY_TABLE && entry->num == 1)
		{
			extern int  Del_Pattern_ACL_For_ContentFilter(void);
			Del_Pattern_ACL_For_ContentFilter();
		}
		list_del(&entry->table_list);
		kfree(entry);
		table_list_head.num--;
	}	
	
	filter_rule_num=1;
	log_enable=0;
	url_filter_mode=0;
	return 0;
}

//rtl_url_filter_fastpath
int filter_item_regist(int type, struct list_head * new_item)
{
	struct list_head *lh;
	filter_table_list *entry;
	
	list_for_each(lh, &table_list_head.table_list)
	{		
		entry=list_entry(lh, filter_table_list, table_list);
		if(entry->type == type)
		{
			list_add(new_item,&entry->item_list);
			entry->num++;
			if(type == URL_KEY_TABLE && entry->num == 2)	
			{
				extern int  Add_Pattern_ACL_For_ContentFilter(void);
				Add_Pattern_ACL_For_ContentFilter();
			}
			return 0;
		}
	}
	
	return -1;
}

//////////////////////////////////////////////
//

int check_esc_end(char*str,char escCh)
{
	int i=0,count=0,len=strlen(str);
	//printk("strlen=%d %d\n",len,__LINE__);
	for(i=len-1;i>=0;i--)
	{
		if(str[i]==escCh)
			count++;
		else
			break;
	}
	//printk("count=%d %d\n",count,__LINE__);
	return count%2;
}

/**
*7/1 00-11-22-33-44-55/2 baidu google
*flag/table_type data/table_type data;
*/
static int filter_table_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
  	char *tmpbuf;
  	char *tokptr, *entryptr,*tokptrUrl,*strptr;
  	filter_item_entry *entry;
	int i,j;

	int flag,type,url_len;
	int flag_item;
#if CONFIG_URL_FILTER_USER_MODE_SUPPORT
	unsigned int ip_addr;
	char userMode;
    char *tokptrIp;
#endif
	char strim[1] ;
	strim[0] = 22;		//use unseen char to distinguish,dzh mod.
	tmpbuf = kmalloc(sizeof(char) * 1024,GFP_KERNEL);
	if(tmpbuf == NULL)
		return -EFAULT;
	memset(tmpbuf,0,sizeof(char) * 1024);
	strptr=tmpbuf;
	//printk("buffer=%s count=%d %d\n",buffer,count,__LINE__);
	memset(get_info_from_usr_space,0,strlen(get_info_from_usr_space));
    if (buffer && !copy_from_user(&get_info_from_usr_space, buffer, count))
 	{
	     	strncpy(tmpbuf,get_info_from_usr_space,count);
	      	tmpbuf[strlen(get_info_from_usr_space)-1]='\0';	
			//strcpy(strptr2,strptr);
		if(memcmp(strptr,"init", strlen("init")) == 0)
		{
			//entryptr = strsep(&strptr,";");
			tokptr = strsep(&strptr," ");
			do{
				//printk("%s-----%d\n",__FUNCTION__,__LINE__);
				if(strptr==NULL)goto _err;
				tokptr = strsep(&strptr," ");
				flag= str2hexnum(tokptr);	
				//printk("flag1=%x\n",flag);
				filter_table_regist(flag,filter_tlb[flag].func);				
			}while(strptr!=NULL);	
		}
		else if(memcmp(strptr,"flush", strlen("flush")) == 0)
		{
			//printk("2tmpbuf=%s\n",tmpbuf);	
			filter_table_flush(0);		
		}
		else if(memcmp(strptr,"enable_log", strlen("enable_log")) == 0)
		{
			log_enable=1;
		}
		else if(memcmp(strptr,"white", strlen("white")) == 0)
		{
			url_filter_mode=URL_FILTER_MODE_WHITE;
		}
		else if(memcmp(strptr,"black", strlen("black")) == 0)
		{
			url_filter_mode=URL_FILTER_MODE_BLACK;
		}
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
		else if(memcmp(strptr,"drop",strlen("drop"))==0)
		{
			default_policy = URL_FILTER_DEFAULT_POLICY_DROP;
		}
		else if(memcmp(strptr,"pass",strlen("pass"))==0)
		{
			default_policy = URL_FILTER_DEFAULT_POLICY_PASS;
		}
#endif
		else if(memcmp(strptr,"add:", strlen("add:")) == 0)
		{			
			//printk("1:strptr=%s %d\n",strptr,__LINE__);
			//add:0#3 3 baidu.com c0a80165;\#\ \;\:sina.com c0a80164;...
			strptr+=strlen("add:");
			if(!strptr[0]) goto _err;
			//printk("2:strptr=%s %d\n",strptr,__LINE__);
			//0#3 3 baidu.com c0a80165;\#\ \;\:sina.com c0a80164;...
			tokptr = strsep(&strptr,"#"); 
			if(!strptr) goto _err;
			if((flag= str2hexnum(tokptr))==-1)
				goto _err;
			//printk("3:strptr=%s %d\n",strptr,__LINE__);
			//3 3 baidu.com c0a80165;\#\ \;\:sina.com c0a80164;...
			tokptr = strsep(&strptr," "); 
			if(!strptr) goto _err;
			if((type= str2hexnum(tokptr))==-1)
				goto _err;
			//if(type!=URL_KEY_TABLE)
			//	goto other_table_process;
			//3 baidu.com c0a80165;\#\ \;\:sina.com c0a80164;...
			tokptr = strsep(&strptr," "); 
			if(!strptr) goto _err;
			if((flag_item= str2hexnum(tokptr))==-1)
				goto _err;
			//baidu.com c0a80165;\#\ \;\:sina.com c0a80164;...
			do{
				//printk("strptr=%s %d\n",strptr,__LINE__);
				entryptr = strsep(&strptr,";");
				while(check_esc_end(entryptr,'\\'))
				{//ESC charact
					entryptr[strlen(entryptr)]=';';
					strsep(&strptr,";");
					if(!strptr) break;
				}
				
				//baidu.com c0a80165
				
				//printk("entryptr=%s %d\n",entryptr,__LINE__);
				tokptrUrl=strsep(&entryptr," ");
				if(!entryptr) goto _err;
				while(check_esc_end(tokptrUrl,'\\'))
				{
					
					tokptrUrl[strlen(tokptrUrl)]=' ';
					
					strsep(&entryptr," ");
				
				}
				//printk("tokptrUrl=%s %d\n",tokptrUrl,__LINE__);
				//baidu.com
				//delete ESC characts
				for(i=0;tokptrUrl[i];i++)
				{
					if(tokptrUrl[i]=='\\')
					{
						for(j=i;tokptrUrl[j+1];j++)
							tokptrUrl[j]=tokptrUrl[j+1];
						tokptrUrl[j]='\0';
					}
				}
				//printk("tokptrUrl=%s %d\n",tokptrUrl,__LINE__);
#if CONFIG_URL_FILTER_USER_MODE_SUPPORT
				//c0a80165
				//printk("entryptr=%s %d\n",entryptr,__LINE__);
				tokptrIp=strsep(&entryptr," ");
				if(entryptr) goto _err;//should be null
				userMode=tokptrIp[0];
				tokptrIp++;
				if(userMode=='I')//ip address
				{
					if((ip_addr= str2hexnum(tokptrIp))==-1)
						goto _err;
				}
#endif				
				entry = kmalloc(sizeof(filter_item_entry),GFP_KERNEL);		
				if(entry == NULL)
				{
					//printk("Not enough memory for filter table...\n");
					kfree(tmpbuf);
					return count;
				}
				INIT_LIST_HEAD(&entry->item_list);
				INIT_LIST_HEAD(&entry->rule_list);
				entry->flag=0;
				entry->index=filter_rule_num;
#if CONFIG_URL_FILTER_USER_MODE_SUPPORT
				entry->userMode = userMode;
				if(userMode=='I')
					entry->ipAddr=ip_addr;
				else if(userMode=='M')
				{
					strncpy(entry->macAddr,tokptrIp,2*ETHER_ADDR_LEN);
					entry->macAddr[2*ETHER_ADDR_LEN]='\0';
					touperString(entry->macAddr);
				}
				else
				{
					defaultRuleChanged=1;
				}
#endif
				entry->relation_flag=flag_item;
				
				url_len=strlen(tokptrUrl)>RTL_URL_FILTER_CONTENT_MAXNUM_FASTPATH?(RTL_URL_FILTER_CONTENT_MAXNUM_FASTPATH-1) : strlen(tokptrUrl);
				strncpy(entry->data,tokptrUrl, url_len);
				entry->data[url_len]='\0';
				//printk("entry->data=%s %d\n",entry->data,__LINE__);
				filter_item_regist(URL_KEY_TABLE,&entry->item_list);
				filter_rule_num++;				
			}while(strptr!=NULL && strptr[0]);
//			filter_rule_num++;
			#if 0
other_table_process:
			entryptr = strsep(&strptr2,":");
			do{
				entryptr = strsep(&strptr2,";");
				//printk("entryptr=%s\n",entryptr);

				if(entryptr == NULL) break;
				if(!memcmp(entryptr," ",1) && strlen(entryptr)==1)  break;		
				
				tokptr = strsep(&entryptr,strim); 
				
	
				flag= str2hexnum(tokptr);		
				if(flag == -1) break;
				//printk("flag=%x\n",flag);
				
				do{
//					printk("%s-----%d\n",__FUNCTION__,__LINE__);
		
					tokptr = strsep(&entryptr,strim);
					tokptr2 = strsep(&tokptr," ");
					type = str2hexnum(tokptr2);			
					//printk("type=%x\n",type);
					tokptr2 = strsep(&tokptr," ");
					flag_item = str2hexnum(tokptr2);
					//printk("flag_item=%x\n",flag_item);
					switch(type)
					{
						case IP_RANGE_TABLE:
						{
							int ip_range;
							do{
								entry = kmalloc(sizeof(filter_item_entry),GFP_KERNEL);		
								if(entry == NULL)
								{
									panic_printk("Not enough memory for filter table...\n");
									kfree(tmpbuf);
									return count;
								}
								INIT_LIST_HEAD(&entry->item_list);
								INIT_LIST_HEAD(&entry->rule_list);
								entry->flag=0;
								entry->index=filter_rule_num;
								
								entry->relation_flag=flag_item;
								
								tokptr2 = strsep(&tokptr," ");
								ip_range=str2hexnum(tokptr2);
								memcpy(entry->data,&ip_range,sizeof(int));
								
								tokptr2 = strsep(&tokptr," ");
								ip_range=str2hexnum(tokptr2);
								memcpy(entry->data+4,&ip_range,sizeof(uint32));
								filter_item_regist(IP_RANGE_TABLE,&entry->item_list);
							}while(tokptr!=NULL);
							break;
						}
						case MAC_TABLE:
						{
							entry = kmalloc(sizeof(filter_item_entry),GFP_KERNEL);		
							if(entry == NULL)
							{
								panic_printk("Not enough memory for filter table...\n");
								kfree(tmpbuf);
								return count;
							}
							INIT_LIST_HEAD(&entry->item_list);
							INIT_LIST_HEAD(&entry->rule_list);
							entry->flag=0;
							entry->index=filter_rule_num;
							
							entry->relation_flag=flag_item;
							
							char *mac_ptr;
							tokptr2 = strsep(&tokptr," ");
							//printk("mac=%s\n",tokptr2);							
							
							i=0;
							do
							{
								mac_ptr = strsep(&tokptr2,"-");
								entry->data[i]=str2hexnum(mac_ptr) & 0xff;
								//printk("mac%d=%02x\n",i,entry->data[i]);							
								i++;
							}while(tokptr2 !=NULL );
							//printk("index=%d[%d]\n",entry->index,__LINE__);
							filter_item_regist(MAC_TABLE,&entry->item_list);
							break;
						}
						case URL_KEY_TABLE:
						{
							do{
								entry = kmalloc(sizeof(filter_item_entry),GFP_KERNEL);		
								if(entry == NULL)
								{
									panic_printk("Not enough memory for filter table...\n");
									kfree(tmpbuf);
									return count;
								}
								tokptr2 = strsep(&tokptr," ");
								//printk("tokptr2=%x\n",tokptr2[0]);
								if(tokptr2[0]==0) continue;
								
								INIT_LIST_HEAD(&entry->item_list);
								INIT_LIST_HEAD(&entry->rule_list);
								entry->flag=0;
								entry->index=filter_rule_num;
								
								entry->relation_flag=flag_item;
								
								
								url_len=strlen(tokptr2)>RTL_URL_FILTER_CONTENT_MAXNUM_FASTPATH?(RTL_URL_FILTER_CONTENT_MAXNUM_FASTPATH-1) : strlen(tokptr2);
								strncpy(entry->data,tokptr2, url_len);
								entry->data[url_len]='\0';
								filter_item_regist(URL_KEY_TABLE,&entry->item_list);
							}while(tokptr!=NULL);
							break;
						}
						case SCHEDULT_TABLE:
						{
							int i_sch;
							do{
								entry = kmalloc(sizeof(filter_item_entry),GFP_KERNEL);		
								if(entry == NULL)
								{
									panic_printk("Not enough memory for filter table...\n");
									kfree(tmpbuf);
									return count;
								}
								INIT_LIST_HEAD(&entry->item_list);
								INIT_LIST_HEAD(&entry->rule_list);
								entry->flag=0;
								entry->index=filter_rule_num;
								
								entry->relation_flag=flag_item;
								
								tokptr2 = strsep(&tokptr," ");
								i_sch=str2hexnum(tokptr2);
								memcpy(entry->data,&i_sch,sizeof(int));
								
								tokptr2 = strsep(&tokptr," ");

								if((i_sch==0) && (str2hexnum(tokptr2)==0))
								{
									i_sch=24*60;
								}
								else
								{
									i_sch=str2hexnum(tokptr2);
								}
								memcpy(entry->data+4,&i_sch,sizeof(uint32));
								
								//day
								tokptr2 = strsep(&tokptr," ");
								i_sch=str2hexnum(tokptr2);
								//printk("day=%d",i_sch);
								if(i_sch == 0) i_sch=0xff;
								memcpy(entry->data+8,&i_sch,sizeof(int));
								filter_item_regist(SCHEDULT_TABLE,&entry->item_list);
							}while(tokptr!=NULL);
							break;
						}
						default:
							break;
					}
				}while(entryptr!=NULL);		
				filter_rule_num++;
			}while(strptr2!=NULL);
			#endif
		}
		kfree(tmpbuf);
    		return count;
     	}
_err:		
	kfree(tmpbuf);
      	return -EFAULT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int filter_table_read_proc(struct seq_file *s, void *v)
{
	char *buf, *tmp;
	struct list_head *lh,*lh2;
	filter_table_list *entry=NULL;
	filter_item_entry *entry_item;

	buf = kmalloc(sizeof(char) * 512,GFP_KERNEL);
	if(buf == NULL)
		return 0;
	memset(buf,0,512);

	tmp = kmalloc(sizeof(char) * 256,GFP_KERNEL);
	if(tmp == NULL)
	{
		kfree(buf );
		return 0;
	}
	memset(tmp,0,256);	
		
	list_for_each(lh, &table_list_head.table_list)
	{		
		entry=list_entry(lh, filter_table_list, table_list);
		//printk("type=%d num=%d\n",entry->type,entry->num-1);
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
		if(entry->type==URL_KEY_TABLE&&url_filter_mode)
			sprintf(buf,"type=%d num=%d default policy=%s\n",entry->type,entry->num-1,default_policy?"pass":"drop");
		else
			sprintf(buf,"type=%d num=%d\n",entry->type,entry->num-1);
#else
		sprintf(buf,"type=%d num=%d\n",entry->type,entry->num-1);
#endif
		switch(entry->type)
		{
			case MAC_TABLE:
				list_for_each(lh2,&(entry->item_list))
				{
					entry_item=list_entry(lh2, filter_item_entry, item_list);
					sprintf(tmp,"index=%d	",entry_item->index);
					//printk("index=%d	",entry_item->index);
					strcat(buf,tmp);
					sprintf(tmp,"mac=%02x:%02x:%02x:%02x:%02x:%02x\n",entry_item->data[0]&0xff,entry_item->data[1]&0xff,entry_item->data[2]&0xff,entry_item->data[3]&0xff,entry_item->data[4]&0xff,entry_item->data[5]&0xff);	
					strcat(buf,tmp);
					//printk("mac=%02x:%02x:%02x:%02x:%02x:%02x\n",entry_item->data[0]&0xff,entry_item->data[1]&0xff,entry_item->data[2]&0xff,entry_item->data[3]&0xff,entry_item->data[4]&0xff,entry_item->data[5]&0xff);	
				}					
				break;
			case URL_KEY_TABLE:
				list_for_each(lh2,&(entry->item_list))
				{
					entry_item=list_entry(lh2, filter_item_entry, item_list);
					sprintf(tmp,"index=%d		",entry_item->index);
					strcat(buf,tmp);
					//printk("index=%d		",entry_item->index);
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
					sprintf(tmp,"url=%s    ",entry_item->data);	
#else
					sprintf(tmp,"url=%s\n",entry_item->data);	
#endif
					strcat(buf,tmp);
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
					if(entry_item->userMode=='I')
					{
						sprintf(tmp,"user ip=%u.%u.%u.%u\n",
								(0xFF000000&entry_item->ipAddr)>>24,
								(0xFF0000&entry_item->ipAddr)>>16,
								(0xFF00&entry_item->ipAddr)>>8,
								(0xFF&entry_item->ipAddr));
						strcat(buf,tmp);
					}
					else if(entry_item->userMode=='M')
					{
						sprintf(tmp,"user mac=%s\n",entry_item->macAddr);
						strcat(buf,tmp);
					}
					else
					{
						sprintf(tmp,"user=all\n");
						strcat(buf,tmp);
					}
#endif
					//printk("url=%s\n",entry_item->data);	
				}					
				break;
			case SCHEDULT_TABLE:
			{
				uint32 *start, *end,*day;				
				list_for_each(lh2,&(entry->item_list))
				{
					entry_item=list_entry(lh2, filter_item_entry, item_list);
					sprintf(tmp,"index=%d	",entry_item->index);
					strcat(buf,tmp);
					//printk("index=%d	",entry_item->index);
					start=(uint32 *)entry_item->data;
					end=(uint32 *)(entry_item->data+4);
					day=(uint32 *)(entry_item->data+8);
					sprintf(tmp,"start=%x	",*start);	
					strcat(buf,tmp);
					//printk("start=%x	",*start);	
					sprintf(tmp,"end=%x	",*end);
					strcat(buf,tmp);
					//printk("end=%x	",*end);
					sprintf(tmp,"day=%x\n",*day);
					strcat(buf,tmp);
					//printk("day=%x\n",*day);
				}					
				break;
			}
			default:
				sprintf(tmp,"unknow table type\n" );
				strcat(buf,tmp);
				//printk("unknow table type\n" );
				break;
		}
	}
	seq_printf(s, "Mode:%d %s\n", url_filter_mode, buf);
	
	kfree(buf );
	kfree(tmp);
      	return 0;
}

int filter_table_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, filter_table_read_proc, NULL));
}

static ssize_t filter_table_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return filter_table_write_proc(file, userbuf,count, off);
}

struct file_operations filter_table_proc_fops = {
        .open           = filter_table_single_open,
	 	.write		= filter_table_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#else
static int filter_table_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	 int len;
	char *buf, *tmp;
	struct list_head *lh,*lh2;
	filter_table_list *entry=NULL;
	filter_item_entry *entry_item;

	buf = kmalloc(sizeof(char) * 512,GFP_KERNEL);
	if(buf == NULL)
		return 0;
	memset(buf,0,512);

	tmp = kmalloc(sizeof(char) * 256,GFP_KERNEL);
	if(tmp == NULL)
	{
		kfree(buf );
		return 0;
	}
	memset(tmp,0,256);	
		
	list_for_each(lh, &table_list_head.table_list)
	{		
		entry=list_entry(lh, filter_table_list, table_list);
		//printk("type=%d num=%d\n",entry->type,entry->num-1);
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
		if(entry->type==URL_KEY_TABLE&&url_filter_mode)
			sprintf(buf,"type=%d num=%d default policy=%s\n",entry->type,entry->num-1,default_policy?"pass":"drop");
		else
			sprintf(buf,"type=%d num=%d\n",entry->type,entry->num-1);
#else
		sprintf(buf,"type=%d num=%d\n",entry->type,entry->num-1);
#endif
		switch(entry->type)
		{
			case MAC_TABLE:
				list_for_each(lh2,&(entry->item_list))
				{
					entry_item=list_entry(lh2, filter_item_entry, item_list);
					sprintf(tmp,"index=%d	",entry_item->index);
					//printk("index=%d	",entry_item->index);
					strcat(buf,tmp);
					sprintf(tmp,"mac=%02x:%02x:%02x:%02x:%02x:%02x\n",entry_item->data[0]&0xff,entry_item->data[1]&0xff,entry_item->data[2]&0xff,entry_item->data[3]&0xff,entry_item->data[4]&0xff,entry_item->data[5]&0xff);	
					strcat(buf,tmp);
					//printk("mac=%02x:%02x:%02x:%02x:%02x:%02x\n",entry_item->data[0]&0xff,entry_item->data[1]&0xff,entry_item->data[2]&0xff,entry_item->data[3]&0xff,entry_item->data[4]&0xff,entry_item->data[5]&0xff);	
				}					
				break;
			case URL_KEY_TABLE:
				list_for_each(lh2,&(entry->item_list))
				{
					entry_item=list_entry(lh2, filter_item_entry, item_list);
					sprintf(tmp,"index=%d		",entry_item->index);
					strcat(buf,tmp);
					//printk("index=%d		",entry_item->index);
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
					sprintf(tmp,"url=%s    ",entry_item->data);	
#else
					sprintf(tmp,"url=%s\n",entry_item->data);	
#endif
					strcat(buf,tmp);
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
					if(entry_item->userMode=='I')
					{
						sprintf(tmp,"user ip=%u.%u.%u.%u\n",
								(0xFF000000&entry_item->ipAddr)>>24,
								(0xFF0000&entry_item->ipAddr)>>16,
								(0xFF00&entry_item->ipAddr)>>8,
								(0xFF&entry_item->ipAddr));
						strcat(buf,tmp);
					}
					else if(entry_item->userMode=='M')
					{
						sprintf(tmp,"user mac=%s\n",entry_item->macAddr);
						strcat(buf,tmp);
					}
					else
					{
						sprintf(tmp,"user=all\n");
						strcat(buf,tmp);
					}
#endif
					//printk("url=%s\n",entry_item->data);	
				}					
				break;
			case SCHEDULT_TABLE:
			{
				uint32 *start, *end,*day;				
				list_for_each(lh2,&(entry->item_list))
				{
					entry_item=list_entry(lh2, filter_item_entry, item_list);
					sprintf(tmp,"index=%d	",entry_item->index);
					strcat(buf,tmp);
					//printk("index=%d	",entry_item->index);
					start=(uint32 *)entry_item->data;
					end=(uint32 *)(entry_item->data+4);
					day=(uint32 *)(entry_item->data+8);
					sprintf(tmp,"start=%x	",*start);	
					strcat(buf,tmp);
					//printk("start=%x	",*start);	
					sprintf(tmp,"end=%x	",*end);
					strcat(buf,tmp);
					//printk("end=%x	",*end);
					sprintf(tmp,"day=%x\n",*day);
					strcat(buf,tmp);
					//printk("day=%x\n",*day);
				}					
				break;
			}
			default:
				sprintf(tmp,"unknow table type\n" );
				strcat(buf,tmp);
				//printk("unknow table type\n" );
				break;
		}
	}
	len = sprintf(page, "Mode:%d %s\n", url_filter_mode,buf);
	
	if (len <= off+count) *eof = 1;
     	*start = page + off;
      	len -= off;
      	if (len>count) len = count;
      	if (len<0) len = 0;
	kfree(buf );
	kfree(tmp);
      	return len;
}
#endif
#if 0
int parse_flag(int type,int flag)
{
	int ret;
	ret=(flag>>(4*(type-1))) & 0xf;
	if(flag & (0x1 << (16+type-1)))
	{
		ret|=0x100;
	}
	return ret;
	
}
#endif

int scan_table(void *skb,int index,filter_table_list *entry,struct list_head *_lh)
{
	struct list_head *lh;
	filter_item_entry *entry_item=NULL;
	filter_table_list *next_entry;
	int ret=0;
	
	url_filter_flag=0;
//	printk("type=%d[%d]\n",entry->type,__LINE__);
	if((entry==NULL) || (entry->func == NULL)) return -1;
	if(entry->num<=1||list_empty(&entry->item_list)) goto empty_process;
	list_for_each(lh,&(entry->item_list))
	{
		entry_item=list_entry(lh, filter_item_entry, item_list);
		if((entry_item->relation_flag & 0x1) || ((!(entry_item->relation_flag & 0x1)) && (index == entry_item->index)))
		{
			if(entry_item->relation_flag & 0x200)
			{
				//printk("type=%d	",entry->type);
				next_entry=list_entry(_lh->next, filter_table_list, table_list);
				if(entry_item->relation_flag & 0x2)
				{
					if(next_entry==NULL)
					{
						return -1;
					}
					ret=scan_table(skb,entry_item->index,next_entry,&(next_entry->table_list));
				}
				
				//printk("ret=%d[%d]\n",ret,__LINE__);
				if(ret != 0) 
				{
						//printk("%s-----%d\n",__FUNCTION__,__LINE__);
						return 1;
				}
			}
			else if((entry_item->relation_flag & 0x100)||(entry->func(skb,entry_item->data)) )
			{
				if(!(entry_item->relation_flag & 0x2)) 
				{
					if(entry_item->relation_flag & 0x1) 
					{
						//printk("%s-----ret=%d\n",__FUNCTION__,(entry_item->relation_flag & 0x8)?0:1);
						return (entry_item->relation_flag & 0x8)?0:1;
					}
					else
					{
						//printk("%s-----%d\n",__FUNCTION__,__LINE__);
						return 1;
					}
				}
				else
				{
					//printk("type=%d	",entry->type);
					next_entry=list_entry(_lh->next, filter_table_list, table_list);
					if(entry_item->relation_flag & 0x2)
					{
						if(next_entry==NULL)
						{
							return -1;
						}
						
						ret=scan_table(skb,entry_item->index,next_entry,&(next_entry->table_list));
					}
					//printk("ret=%d[%d]\n",ret,__LINE__);
					if(ret != 0) 
					{
						if(entry_item->relation_flag & 0x1) 
						{
							if(url_filter_mode)	/*white list*/
								return (entry_item->relation_flag & 0x8)?1:0;
							else
							//printk("%s-----%d\n",__FUNCTION__,__LINE__);
							return (entry_item->relation_flag & 0x8)?0:1;
						}
						else
						{
							//printk("%s-----%d\n",__FUNCTION__,__LINE__);
							if(url_filter_mode)	/*white list*/
								return 0;
							else
							return 1;
						}				
					}
				}
			}
		}
		continue;
	}
empty_process:
	if(entry_item==NULL)
	{
#ifdef CONFIG_URL_FILTER_USER_MODE_SUPPORT
		if(url_filter_mode)
		{
			if(default_policy==URL_FILTER_DEFAULT_POLICY_DROP)
				return 1;
			else
				return -1;
		}
		return -1;
#else
		if(url_filter_mode)	
			return 1;
		else
		return -1;
#endif
	}

	if(url_filter_flag)		/*white list and match fail*/
		return 1;
	
	if(entry_item->relation_flag & 0x1) 
	{
		//printk("%s-----%d\n",__FUNCTION__,__LINE__);
		return (entry_item->relation_flag & 0x8)?1:0;
	}
	else
	{
		//printk("%s-----%d\n",__FUNCTION__,__LINE__);
		return 0;
	}
}
extern int scrlog_printk(const char * fmt, ...);
int log_fastFilter(int type, void *skb)
{
	//[type][time] [pkt type] source Info ==> destination Info [Blocked]
	struct iphdr *iph;
	unsigned char *mac=NULL;
	struct timeval tv;
	uint32 today, hour,minute;
	do_gettimeofday(&tv);
	today = ((tv.tv_sec/86400) + 4)%7;
	hour = (tv.tv_sec/3600)%24;
	minute = (tv.tv_sec/60)%60;

	//ip
	iph=rtl_ip_hdr(skb);
	mac=(unsigned char *)rtl_eth_hdr(skb);
	switch(type)
	{
		case IP_RANGE_TABLE:
		{
			if(iph==NULL)
			{
				break;
			}
			scrlog_printk("[ip_range_filter]%d.%d.%d.%d ===> %d.%d.%d.%d  [Blocked]\n",(iph->saddr >> 24) & 0xff, 
																					(iph->saddr >> 16) & 0xff, 
																					(iph->saddr >> 8) & 0xff, 
																					iph->saddr& 0xff, 
																					(iph->daddr >> 24) & 0xff, 
																					(iph->daddr >> 16) & 0xff, 
																					(iph->daddr >> 8) & 0xff, 
																					iph->daddr& 0xff);
			break;
		}	
		case MAC_TABLE:
		{
			if((mac==NULL) || (iph==NULL))
			{
				break;
			}
			scrlog_printk("[mac_filter]%d.%d.%d.%d(%02x:%02x:%02x:%02x:%02x:%02x) ===> %d.%d.%d.%d [Blocked]\n",
							(iph->saddr >> 24) & 0xff,(iph->saddr >> 16) & 0xff,(iph->saddr >> 8) & 0xff, iph->saddr& 0xff,
							rtl_eth_hdr(skb)[ETHER_ADDR_LEN],rtl_eth_hdr(skb)[ETHER_ADDR_LEN+1],rtl_eth_hdr(skb)[ETHER_ADDR_LEN+2],
							rtl_eth_hdr(skb)[ETHER_ADDR_LEN+3],rtl_eth_hdr(skb)[ETHER_ADDR_LEN+4],rtl_eth_hdr(skb)[ETHER_ADDR_LEN+5],
																					(iph->daddr >> 24) & 0xff,
																					(iph->daddr >> 16) & 0xff,
																					(iph->daddr >> 8) & 0xff,
																					iph->daddr& 0xff);
			break;
		}			
		case URL_KEY_TABLE:
		{
			if(iph==NULL)
			{
				break;
			}
			scrlog_printk("[url_key_filter]%d.%d.%d.%d ==> %d.%d.%d.%d(%s)  [Blocked]\n",(iph->saddr >> 24) & 0xff, 
																					(iph->saddr >> 16) & 0xff, 
																					(iph->saddr >> 8) & 0xff, 
																					iph->saddr& 0xff, 
																					(iph->daddr >> 24) & 0xff, 
																					(iph->daddr >> 16) & 0xff, 
																					(iph->daddr >> 8) & 0xff, 
																					iph->daddr& 0xff,
																					log_info);
			break;
		}					
		case SCHEDULT_TABLE:
		{
			if(iph==NULL)
			{
				break;
			}
			scrlog_printk("[schedule_filter]%d.%d.%d.%d ===> %d.%d.%d.%d  [Blocked]\n",(iph->saddr >> 24) & 0xff, 
																					(iph->saddr >> 16) & 0xff, 
																					(iph->saddr >> 8) & 0xff, 
																					iph->saddr& 0xff, 
																					(iph->daddr >> 24) & 0xff, 
																					(iph->daddr >> 16) & 0xff, 
																					(iph->daddr >> 8) & 0xff, 
																					iph->daddr& 0xff);
			break;
		}
		default:
			printk("Unkown Type [Blocked]\n");
	}

	return 0;
}
int do_filter(void *skb)
{
	struct list_head *lh;
	filter_table_list *entry;
	//filter_item_entry *entry_item;
	int ret=0;
	//printk("%s-----%d\n",__FUNCTION__,__LINE__);
	list_for_each(lh, &table_list_head.table_list)
	{
		entry=list_entry(lh, filter_table_list, table_list);
		ret = scan_table(skb,0,entry,lh);
		if(ret == -1)
		{
			return 0;
		}
		else if(ret == 1)
		{
			if(log_enable)	
				log_fastFilter(entry->type,skb);			
			return 1;
		}
	}
	return 0;
}


// url + mac
int  FilterWeb_v2(void *skb)
{
	struct iphdr *iph;
	int ret;

	iph=rtl_ip_hdr(skb);

	if(!(need_filter(skb))) return 0;
	
	ret = do_filter(skb);
//	printk("ret=%d\n",ret);
	
	return ret;
}
#if 0	
/*---------------------------------------------netlink--start---------------------------------------------*/
//#define NETLINK_DEBUG 1
struct sock *nl_sk = NULL;
struct test_struct
{
	int flag;
	char data[128];
};

void nl_data_ready (struct sk_buff *__skb)
{
  	int pid;
	struct test_struct send_data,recv_data;
	
	pid=rtk_nlrecvmsg(__skb,sizeof(struct test_struct),&recv_data);	
	if(pid < 0) return ;
#ifdef NETLINK_DEBUG	
 	printk("flag=%d, data:%s\n", recv_data.flag,recv_data.data);
#endif	
      	send_data.flag=pid;
    	sprintf(send_data.data,"Return Description");
      	rtk_nlsendmsg(pid,nl_sk,sizeof(struct test_struct),&send_data);
  	return;
}

static int debug_netlink(void) {
  	nl_sk = netlink_kernel_create(&init_net, NETLINK_RTK_DEBUG, 0, nl_data_ready, NULL, THIS_MODULE);

  	if (!nl_sk) {
    		printk(KERN_ERR "Netlink[Kernel] Cannot create netlink socket.\n");
    		return -EIO;
  	}	
  	printk("Netlink[Kernel] create socket ok.\n");
  	return 0;
}
/*---------------------------------------------netlink--end---------------------------------------------*/
#endif

static struct proc_dir_entry *res12=NULL;
int filter_table_init(void)
{
	filter_table_head_init();	
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	res12 = proc_create_data("filter_table", 0, &proc_root,
			 &filter_table_proc_fops, NULL);
	#else
	res12 = create_proc_entry("filter_table", 0, NULL);
	if (res12) {
	    res12->read_proc = filter_table_read_proc;
	    res12->write_proc = filter_table_write_proc;
       }
	#endif
#if 0	
	debug_netlink();
#endif
	return 0;
}
/////////////////////////////////////////////////////////
//

int filter_init_fastpath(void)
{
	filter_rule_num=1;
	filter_table_init();
	return 0;
}

int filter_exit_fastpath(void)
{
	if (res12) {
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("filter_table", &proc_root);		
		#else
		remove_proc_entry("filter_table", res12);		
		#endif
		res12 = NULL;
	}
    return 0;
}
#endif

