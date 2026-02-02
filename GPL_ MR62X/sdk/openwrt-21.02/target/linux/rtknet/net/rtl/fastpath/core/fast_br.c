/*
* Copyright c                  Realtek Semiconductor Corporation, 2010 
* All rights reserved.
* 
* Program : bridge fast path
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <net/rtl/fastpath/fast_br.h>

#include <linux/version.h>

static fast_br_head rtl_fast_br_head[RTL_FAST_BR_HEAD_SIZE];
static fast_br_cache_entry rtl_fast_br_entry[RTL_FAST_BR_ENTRY_NUM];

static void rtl_fast_br_flush_entry(void);

static inline int fast_br_mac_hash(const unsigned char *mac)
{
	return (mac[0]^mac[1]^mac[2]^mac[3]^mac[4]^mac[5]) & (RTL_FAST_BR_HASH_SIZE - 1);
}

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *br_proc = NULL;
static int write_fast_br_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{      
	unsigned char tmpbuf[32];
	//struct net *net;
	
	if (count < 1) 
		return -EFAULT;
	
	if (buffer && !copy_from_user(tmpbuf, buffer, 32))  {
		//printk("input(%c)\n",tmpbuf[0]);
		if(tmpbuf[0]=='0')
		{
			rtl_fast_br_flush_entry();
		}
		return count;     
	}
	  
	return -EFAULT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <linux/seq_file.h>
extern struct proc_dir_entry proc_root;

static int read_fast_br_proc(struct seq_file *s, void *v)
{
#if 1	  
	int i = 0,hash_cnt = 0;
	struct hlist_node *node = NULL;
	fast_br_cache_entry *entry = NULL; 

	for(i = 0; i < RTL_FAST_BR_HEAD_SIZE; i++)
	{
		panic_printk("list(%d):\n",i);
		for(hash_cnt = 0; hash_cnt < RTL_FAST_BR_HASH_SIZE; hash_cnt++)
		{
			panic_printk("hash(%d):\n",hash_cnt);
			hlist_for_each(node,&rtl_fast_br_head[i].fast_br_hash[hash_cnt])
			{
				entry = hlist_entry(node,struct _fast_br_cache_entry,hlist);
				panic_printk("valid(%d),to(%s),mac(%x:%x:%x:%x:%x:%x)\n", entry->valid,	entry->to_dev?entry->to_dev->name:NULL,
					entry->mac_addr[0],entry->mac_addr[1],entry->mac_addr[2],
					entry->mac_addr[3],entry->mac_addr[4],entry->mac_addr[5]);			
			}
		}
	}
#endif
	  
      return 0;

}

int fast_br_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, read_fast_br_proc, NULL));
}

static ssize_t fast_br_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return write_fast_br_proc(file, userbuf,count, off);
}


struct file_operations fast_br_proc_fops = {
        .open           = fast_br_single_open,
	 .write		= fast_br_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#else
static int read_fast_br_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
      int len = 0;
#if 1	  
	int i = 0,hash_cnt = 0;
	struct hlist_node *node = NULL;
	fast_br_cache_entry *entry = NULL; 

	for(i = 0; i < RTL_FAST_BR_HEAD_SIZE; i++)
	{
		printk("list(%d):\n",i);
		for(hash_cnt = 0; hash_cnt < RTL_FAST_BR_HASH_SIZE; hash_cnt++)
		{
			printk("hash(%d):\n",hash_cnt);
			hlist_for_each(node,&rtl_fast_br_head[i].fast_br_hash[hash_cnt])
			{
				entry = hlist_entry(node,struct _fast_br_cache_entry,hlist);
				printk("valid(%d),to(%s),mac(%x:%x:%x:%x:%x:%x)\n", entry->valid,	entry->to_dev?entry->to_dev->name:NULL,
					entry->mac_addr[0],entry->mac_addr[1],entry->mac_addr[2],
					entry->mac_addr[3],entry->mac_addr[4],entry->mac_addr[5]);			
			}
		}
	}
#endif
	  
      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}
#endif
#endif

static fast_br_cache_entry* rtl_find_fast_br_entry(const unsigned char *addr)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	struct hlist_node *node=NULL;
#endif
	fast_br_cache_entry *entry=NULL;
	int hash_val = 0;

	hash_val = fast_br_mac_hash(addr);
	//only used head[0] now...
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	hlist_for_each_entry_rcu(entry, &rtl_fast_br_head[0].fast_br_hash[hash_val], hlist) 
	#else
	hlist_for_each_entry_rcu(entry, node, &rtl_fast_br_head[0].fast_br_hash[hash_val], hlist) 
	#endif
	{
		if (!compare_ether_addr(entry->mac_addr, addr) && entry->valid == 1)
			return entry;
	}
	
	return NULL;
}

static int rtl_clean_expired_fast_br_entry(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	struct hlist_node *node=NULL;
#endif
	fast_br_cache_entry *entry=NULL;
	int i = 0,hash_cnt=0;

	for(i = 0; i < RTL_FAST_BR_HEAD_SIZE; i++)
	{
		for(hash_cnt = 0; hash_cnt < RTL_FAST_BR_HASH_SIZE;hash_cnt++)
		{
            #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
			hlist_for_each_entry_rcu(entry, &rtl_fast_br_head[i].fast_br_hash[hash_cnt], hlist) 
			#else
			hlist_for_each_entry_rcu(entry, node, &rtl_fast_br_head[i].fast_br_hash[hash_cnt], hlist) 
			#endif
			{
				if (time_before_eq(entry->ageing_timer + RTL_FAST_BR_ENTRY_TIME_OUT, jiffies))
				{
					hlist_del_rcu(&entry->hlist);
					memset(entry,0,sizeof(fast_br_cache_entry));
				}
			}
		}
	}
	
	return RTL_FAST_BR_SUCCESS;
}

int rtl_add_fast_br_entry(fast_br_cache_entry *entry)
{
	int i = 0,retried = 0;	
	int hash_val = 0;
	fast_br_cache_entry *add_entry;

	add_entry = rtl_find_fast_br_entry(entry->mac_addr);

	if(add_entry)
	{
		add_entry->ageing_timer = jiffies;
		return ERR_RTL_FAST_BR_ENTRY_EXIST;
	}
retry:
	for(i = 0; i < RTL_FAST_BR_ENTRY_NUM; i++)
	{
		if(0==rtl_fast_br_entry[i].valid)
			break;
	}

	if(i == RTL_FAST_BR_ENTRY_NUM && retried == 0)		
	{
		//return ERR_RTL_FAST_BR_NO_BUFFER;
		rtl_clean_expired_fast_br_entry();
		retried = 1;
		goto retry;
	}

	if(i == RTL_FAST_BR_ENTRY_NUM)
		return ERR_RTL_FAST_BR_NO_BUFFER;

	hash_val = fast_br_mac_hash(entry->mac_addr);
	memcpy(rtl_fast_br_entry[i].mac_addr,entry->mac_addr,6);
	rtl_fast_br_entry[i].to_dev = entry->to_dev;	
	rtl_fast_br_entry[i].ageing_timer = jiffies;	

	rtl_fast_br_entry[i].valid = 1;
	//use head[0] now...
	hlist_add_head_rcu(&rtl_fast_br_entry[i].hlist,&rtl_fast_br_head[0].fast_br_hash[hash_val]);
	return RTL_FAST_BR_SUCCESS;	
}

static int rtl_del_fast_br_entry(fast_br_cache_entry *entry)
{	
	if(NULL == entry)
		return ERR_RTL_FAST_BR_ENTRY_NOT_EXIST;
	
	hlist_del_rcu(&entry->hlist);
	memset(entry,0,sizeof(fast_br_cache_entry));
	
	return RTL_FAST_BR_SUCCESS;
}

static int rtl_fast_br_entry_has_expired(fast_br_cache_entry *entry)
{		
	if(time_before_eq(entry->ageing_timer + RTL_FAST_BR_ENTRY_TIME_OUT, jiffies))
		return RTL_FAST_BR_ENTRY_EXPIRED;

	return RTL_FAST_BR_ENTRY_NOT_EXPIRED;
}



static int rtl_update_fast_br_entry_timer(fast_br_cache_entry *entry)
{
	entry->ageing_timer = jiffies;	
	return RTL_FAST_BR_SUCCESS;
}

static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? 4 : 0);
}

static void rtl_fast_br_flush_entry(void)
{
	int i = 0,hash_cnt=0;
	fast_br_cache_entry *entry;
	struct hlist_node *h, *n;

	for(i = 0; i < RTL_FAST_BR_HEAD_SIZE; i++)
	{
		for(hash_cnt = 0; hash_cnt < RTL_FAST_BR_HASH_SIZE;hash_cnt++)
		{
			hlist_for_each_entry_safe(entry,h,n,&rtl_fast_br_head[i].fast_br_hash[hash_cnt],hlist)
			{
				rtl_del_fast_br_entry(entry);
			}
		}
	}	
}

int rtl_fast_br_forwarding(struct sk_buff *skb)
{
	fast_br_cache_entry *dst_entry=NULL;
	fast_br_cache_entry *src_entry=NULL;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	const unsigned char *source = eth_hdr(skb)->h_source;

	skb->fast_br_forwarding_flags = 0;
	dst_entry = rtl_find_fast_br_entry(dest);
	src_entry = rtl_find_fast_br_entry(source);
	if(src_entry)
	{
		//mac changed bridge port!
		if(skb->dev != src_entry->to_dev)
		{	
		#if 0
			printk("====%s(%d),skb->dev(%s),entry->to_dev(%s)\n",__FUNCTION__,__LINE__,skb->dev->name,src_entry->to_dev->name);
			printk("%x:%x:%x:%x:%x:%x ==> %x:%x:%x:%x:%x:%x \n", 
			source[0], source[1], source[2],
			source[3], source[4], source[5], 
			dest[0], dest[1], dest[2], 
			dest[3], dest[4], dest[5]);
		#endif
			rtl_del_fast_br_entry(src_entry);
		}
		else
			rtl_update_fast_br_entry_timer(src_entry);
	}
	
	if(dst_entry == NULL)
		return RTL_FAST_BR_FAILED;

	if(rtl_fast_br_entry_has_expired(dst_entry) == RTL_FAST_BR_ENTRY_EXPIRED)
	{
		rtl_del_fast_br_entry(dst_entry);
		return RTL_FAST_BR_FAILED;
	}

	if (packet_length(skb) > dst_entry->to_dev->mtu && !skb_is_gso(skb))
		return RTL_FAST_BR_FAILED;

	if(unlikely((dst_entry->to_dev->flags & IFF_UP) == 0))
		return RTL_FAST_BR_FAILED;

	//forwarding it!
	skb->dev = dst_entry->to_dev;
	skb->fast_br_forwarding_flags = 1;
	skb_push(skb, ETH_HLEN);		
	dst_entry->to_dev->netdev_ops->ndo_start_xmit(skb,skb->dev);	
	
	return RTL_FAST_BR_SUCCESS;	
}

static int __init fast_br_init(void)
{
	int i;
	int hash_cnt = 0;

	for(i = 0; i < RTL_FAST_BR_ENTRY_NUM; i++)
	{
		rtl_fast_br_entry[i].valid = 0;		
	}

	for (i = 0; i < RTL_FAST_BR_HEAD_SIZE; i++)
	{
		for(hash_cnt = 0; hash_cnt < RTL_FAST_BR_HASH_SIZE; hash_cnt++)
			rtl_fast_br_head[i].fast_br_hash[hash_cnt].first = NULL;
	}
	
	
#ifdef CONFIG_PROC_FS
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	br_proc = proc_create_data("fast_bridge", 0, &proc_root,
			 &fast_br_proc_fops, NULL);
	#else
	br_proc=create_proc_entry("fast_bridge",0,NULL);
	if (br_proc) {
	    br_proc->read_proc=read_fast_br_proc;
	    br_proc->write_proc=write_fast_br_proc;
	}
	#endif
#endif

	//printk("%s %s\n",MODULE_NAME, MODULE_VERSION);
	
	return 0;
}

static void __exit fast_br_exit(void)
{
#ifdef CONFIG_PROC_FS
	if (br_proc) {
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fast_bridge", &proc_root);	
		#else
		remove_proc_entry("fast_bridge", br_proc);	
		#endif
		br_proc = NULL;
	}
#endif	
}


module_init(fast_br_init);
module_exit(fast_br_exit);


