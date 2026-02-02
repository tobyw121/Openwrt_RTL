
#if defined (CONFIG_RTL_FAST_PPPOE)
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

#include <linux/in.h>
#include <linux/udp.h>
#include <linux/if_tunnel.h>
#include <linux/if_ether.h>
#include <net/rtl/rtl_types.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <uapi/linux/netfilter/nf_conntrack_tuple_common.h>
#endif
#include <net/rtl/fastpath/fastpath_core.h>

#ifdef CONFIG_BRIDGE
#include <bridge/br_private.h>
#endif

#if defined(CONFIG_NET_SCHED)
extern int gQosEnabled;
#endif

#include <linux/if_pppox.h>
#include <linux/if_ether.h>

int fast_pppoe_fw=0; 
struct pppoe_info fast_pppoe_info[MAX_PPPOE_ENTRY];

#if defined(CONFIG_PROC_FS)
struct proc_dir_entry *fast_pppoe_proc=NULL;
#endif

static int fast_pppoe_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char l2tp_tmp;

      if (count < 2)
	    return -EFAULT;

      if (buffer && !copy_from_user(&l2tp_tmp, buffer, 1)) 
	  {
	    	fast_pppoe_fw = l2tp_tmp - '0';

			return count;
      }
      return -EFAULT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int fast_pppoe_read_proc(struct seq_file *s, void *v)
{
	int i;

	seq_printf(s, "fast pppoe enable:%c\n", fast_pppoe_fw + '0');
	
	for(i=0; i<MAX_PPPOE_ENTRY; i++)
	{
		if(fast_pppoe_info[i].valid==0)
		{
			
			seq_printf(s, "[%d] null,null,0\n",i);

		}
		else
		{
			
			#if defined (CONFIG_RTL_FAST_PPPOE_DEBUG)
			seq_printf(s, "[%d] %s,%s,%d,0x%x:%x:%x:%x:%x:%x,%d,%d\n",i,fast_pppoe_info[i].ppp_dev, fast_pppoe_info[i].wan_dev, fast_pppoe_info[i].sid,
			fast_pppoe_info[i].peer_mac[0],fast_pppoe_info[i].peer_mac[1],fast_pppoe_info[i].peer_mac[2],fast_pppoe_info[i].peer_mac[3],fast_pppoe_info[i].peer_mac[4],fast_pppoe_info[i].peer_mac[5],fast_pppoe_info[i].total_rx,fast_pppoe_info[i].total_tx);
			#else
			seq_printf(s, "[%d] %s<-%s->%s,%d,0x%x:%x:%x:%x:%x:%x\n",i,
			((struct net_device *)fast_pppoe_info[i].ppp0_dev)->name,
			((struct net_device *)fast_pppoe_info[i].wan_dev)->name,
			((struct net_device *)fast_pppoe_info[i].wan_dev_slave)->name, fast_pppoe_info[i].sid,
			fast_pppoe_info[i].peer_mac[0], fast_pppoe_info[i].peer_mac[1],
			fast_pppoe_info[i].peer_mac[2],fast_pppoe_info[i].peer_mac[3], 
			fast_pppoe_info[i].peer_mac[4],fast_pppoe_info[i].peer_mac[5]);
			#endif
		}
	
	}
	
	return 0;
}

int fastpath_pppoe_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, fast_pppoe_read_proc, NULL));
}

static ssize_t fastpath_pppoe_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return fast_pppoe_write_proc(file, userbuf,count, off);
}

struct file_operations fastpath_pppoe_proc_fops = {
        .open           = fastpath_pppoe_single_open,
	 .write		= fastpath_pppoe_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#else
static int fast_pppoe_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;
	int i;

	len = sprintf(page, "fast pppoe enable:%c\n", fast_pppoe_fw + '0');
	
	for(i=0; i<MAX_PPPOE_ENTRY; i++)
	{
		if(fast_pppoe_info[i].valid==0)
		{
			
			len += sprintf(page+len, "[%d] null,null,0\n",i);

		}
		else
		{
			
			#if defined (CONFIG_RTL_FAST_PPPOE_DEBUG)
			len += sprintf(page+len, "[%d] %s,%s,%d,0x%x:%x:%x:%x:%x:%x,%d,%d\n",i,fast_pppoe_info[i].ppp_dev, fast_pppoe_info[i].wan_dev, fast_pppoe_info[i].sid,
			fast_pppoe_info[i].peer_mac[0],fast_pppoe_info[i].peer_mac[1],fast_pppoe_info[i].peer_mac[2],fast_pppoe_info[i].peer_mac[3],fast_pppoe_info[i].peer_mac[4],fast_pppoe_info[i].peer_mac[5],fast_pppoe_info[i].total_rx,fast_pppoe_info[i].total_tx);
			#else
			len += sprintf(page+len, "[%d] %s,%s,%d,0x%x:%x:%x:%x:%x:%x\n",i,fast_pppoe_info[i].ppp_dev, fast_pppoe_info[i].wan_dev, fast_pppoe_info[i].sid,
			fast_pppoe_info[i].peer_mac[0],fast_pppoe_info[i].peer_mac[1],fast_pppoe_info[i].peer_mac[2],fast_pppoe_info[i].peer_mac[3],fast_pppoe_info[i].peer_mac[4],fast_pppoe_info[i].peer_mac[5]);
			#endif
		}
	
	}
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;

	return len;
}
#endif

int  __init fast_pppoe_init(void)
{

#if defined(CONFIG_PROC_FS)
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	fast_pppoe_proc = proc_create_data("fast_pppoe", 0, &proc_root,
			 &fastpath_pppoe_proc_fops, NULL);
	#else
	fast_pppoe_proc = create_proc_entry("fast_pppoe", 0, NULL);
	if (fast_pppoe_proc) {
		fast_pppoe_proc->read_proc = fast_pppoe_read_proc;
		fast_pppoe_proc->write_proc = fast_pppoe_write_proc;
	}
	#endif
#endif

	memset(fast_pppoe_info, 0 , sizeof(fast_pppoe_info));
	fast_pppoe_fw=1;
	
	return 0;
}

int __exit fast_pppoe_exit(void)
{

#if defined(CONFIG_PROC_FS)
	if (fast_pppoe_proc) {
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fast_pppoe", &proc_root);	
		#else
		remove_proc_entry("fast_pppoe", fast_pppoe_proc);	
		#endif
		fast_pppoe_proc = NULL;
	}
#endif

	return 0;
}

int clear_pppoe_info(struct net_device *ppp0_dev, struct net_device *wan_dev, unsigned short sid,
								unsigned int our_ip,unsigned int	peer_ip,
								unsigned char * our_mac, unsigned char *peer_mac)
{
	int i;
	for(i=0; i<MAX_PPPOE_ENTRY; i++)
	{
		if(fast_pppoe_info[i].valid==0)
		{
			continue;
		}

		if ((ppp0_dev == NULL || fast_pppoe_info[i].ppp0_dev == ppp0_dev) &&
			(wan_dev == NULL || fast_pppoe_info[i].wan_dev == wan_dev) &&
			(peer_mac == NULL || memcmp(fast_pppoe_info[i].peer_mac,peer_mac,6) == 0) &&
			(our_mac == NULL || memcmp(fast_pppoe_info[i].our_mac,our_mac,6) == 0) &&
			(peer_ip == 0 || fast_pppoe_info[i].peer_ip == peer_ip) &&
			(our_ip == 0 || fast_pppoe_info[i].our_ip == our_ip) &&
			(sid == 0 || fast_pppoe_info[i].sid == sid))
		{
			if (ppp0_dev == NULL && wan_dev == NULL &&
				peer_mac == NULL && our_mac == NULL &&
				peer_ip == 0 && our_ip == 0 && sid == 0);
			else
				memset(&fast_pppoe_info[i], 0 , sizeof(struct pppoe_info));
		}
	}
	return 0;
}

static struct pppoe_info* find_pppoe_info_fast( unsigned short sid, unsigned char *peer_mac)
{
	int i;

	
	for(i=0; i<MAX_PPPOE_ENTRY; i++)
	{
		if(fast_pppoe_info[i].valid==0)
		{
			continue;
		}
		
		if((fast_pppoe_info[i].sid==sid) && (memcmp(fast_pppoe_info[i].peer_mac,peer_mac,6)==0))
		{
			return &fast_pppoe_info[i];
		}

	}
	return NULL;
}



static struct pppoe_info* find_pppoe_info(struct net_device *ppp0_dev, struct net_device *wan_dev, unsigned short sid,
												unsigned int our_ip,unsigned int	peer_ip,
												unsigned char * our_mac, unsigned char *peer_mac)
{
	int i;

	
	for(i=0; i<MAX_PPPOE_ENTRY; i++)
	{
		if(fast_pppoe_info[i].valid==0)
		{
			continue;
		}
		if ((ppp0_dev == NULL || fast_pppoe_info[i].ppp0_dev == ppp0_dev) &&
			(wan_dev == NULL || fast_pppoe_info[i].wan_dev == wan_dev) &&
			(peer_mac == NULL || memcmp(fast_pppoe_info[i].peer_mac,peer_mac,6) == 0) &&
			(our_mac == NULL || memcmp(fast_pppoe_info[i].our_mac,our_mac,6) == 0) &&
			(peer_ip == 0 || fast_pppoe_info[i].peer_ip == peer_ip) &&
			(our_ip == 0 || fast_pppoe_info[i].our_ip == our_ip) &&
			(sid == 0 || fast_pppoe_info[i].sid == sid))
		{
			if (ppp0_dev == NULL && wan_dev == NULL &&
				peer_mac == NULL && our_mac == NULL &&
				peer_ip == 0 && our_ip == 0 && sid == 0);
			else
				return &fast_pppoe_info[i];
		}
	}
	return NULL;
}


static struct pppoe_info* alloc_pppoe_info(void)
{
	int i;
	for(i=0; i<MAX_PPPOE_ENTRY; i++)
	{
		if(fast_pppoe_info[i].valid==0)
		{
			return &fast_pppoe_info[i];
		}

	}

	memset(fast_pppoe_info, 0 , sizeof(fast_pppoe_info));

	return &fast_pppoe_info[0];
	
}

struct channel {
	struct ppp_file	file;		/* stuff for read/write/poll */
	struct list_head list;		/* link in all/new_channels list */
	struct ppp_channel *chan;	/* public channel data structure */
	struct rw_semaphore chan_sem;	/* protects `chan' during chan ioctl */
	spinlock_t	downl;		/* protects `chan', file.xq dequeue */
	struct ppp	*ppp;		/* ppp unit we're connected to */
	struct net	*chan_net;	/* the net channel belongs to */
	struct list_head clist;		/* link in list of channels per unit */
	rwlock_t	upl;		/* protects `ppp' */
#ifdef CONFIG_PPP_MULTILINK
	u8		avail;		/* flag used in multilink stuff */
	u8		had_frag;	/* >= 1 fragments have been sent */
	u32		lastseq;	/* MP: last sequence # received */
	int		speed;		/* speed of the corresponding ppp channel*/
#endif /* CONFIG_PPP_MULTILINK */
#if defined(CONFIG_RTL_PPPOE_HWACC) || defined (CONFIG_RTL_FAST_PPPOE)
	u8		pppoe;
	u8		rsv1;
	u16		rsv2;
#endif /* CONFIG_RTL865X_HW_TABLES */
};

struct ppp_channel *get_pppoe_chan(struct sock **sk_out, struct pppoe_info *pppoe_info_ptr)
{
	struct net_device *pptpInf = (struct net_device *)pppoe_info_ptr->ppp0_dev;
	struct ppp *ppp = NULL;
	struct list_head *list = NULL;
	struct channel *pch = NULL;
	struct sock *sk = NULL;

	if (IS_ERR_OR_NULL(pptpInf))
		return NULL;
	ppp = netdev_priv(pptpInf);
	if (IS_ERR_OR_NULL(ppp)) {
		printk("[%s-%u] ppp is invalid, return\n", __func__, __LINE__);
		return NULL;
	}
	if (ppp->n_channels != 1) {
		printk("[%s-%u] ppp %s ->n_channels = %d\n",
			__func__, __LINE__, pptpInf->name,  ppp->n_channels);
		return NULL;
	}
	list = &ppp->channels;
	list = list->next;
	pch = list_entry(list, struct channel, clist);
	if (IS_ERR_OR_NULL(pch)) {
		printk("[%s-%u] pch is invalid, return\n", __func__, __LINE__);
		return NULL;
	}
	if (IS_ERR_OR_NULL(pch->chan)) {
		printk("[%s-%u] pch->chan is invalid, return\n", __func__, __LINE__);
		return NULL;
	}
	sk = (struct sock *)pch->chan->private;
	if (sk_out)
		*sk_out = sk;
	return pch->chan;
}

int set_pppoe_info(struct net_device *ppp0_dev, struct net_device *wan_dev, unsigned short sid,
							unsigned int our_ip,unsigned int	peer_ip,
							unsigned char * our_mac, unsigned char *peer_mac)
{
	struct pppoe_info* pppoe_info_ptr;
	
	pppoe_info_ptr=find_pppoe_info(ppp0_dev, NULL, 0, our_ip, peer_ip,our_mac, peer_mac);
	
	if(pppoe_info_ptr==NULL)
	{
		pppoe_info_ptr=alloc_pppoe_info();
		if(pppoe_info_ptr == NULL)
		{
			return 0;
		}
	}

	if(sid!=0)
	{
		pppoe_info_ptr->sid=sid;
	}
	
	if(our_ip!=0)
	{
		pppoe_info_ptr->our_ip=our_ip;
	}


	if(peer_ip!=0)
	{
		pppoe_info_ptr->peer_ip=peer_ip;
	}
			
	if (ppp0_dev!=NULL)
	{
		pppoe_info_ptr->ppp0_dev = (void *)ppp0_dev;
		pppoe_info_ptr->chan = get_pppoe_chan(&pppoe_info_ptr->sk, pppoe_info_ptr);
	}

	if (wan_dev != NULL)
	{
		pppoe_info_ptr->wan_dev = (void *)wan_dev;
		pppoe_info_ptr->wan_dev_slave = br_get_first_slave(wan_dev);
		if (pppoe_info_ptr->wan_dev_slave == NULL)
		{
			pppoe_info_ptr->wan_dev_slave = wan_dev;
		}
	}
	
	
	if(peer_mac!=NULL)
	{
		memcpy(pppoe_info_ptr->peer_mac,peer_mac,6);
	}

	if(our_mac!=NULL)
	{
		memcpy(pppoe_info_ptr->our_mac,our_mac,6);
	}
	
	pppoe_info_ptr->valid=1;
	
	return 0;
	
}

unsigned long get_pppoe_last_rx_tx(struct net_device *ppp0_dev, struct net_device *wan_dev, unsigned short sid,
								unsigned int our_ip,unsigned int	peer_ip,
								unsigned char * our_mac, unsigned char *peer_mac,
								unsigned long *last_rx, unsigned long *last_tx)
{
	struct pppoe_info* pppoe_info_ptr;
	
	pppoe_info_ptr=find_pppoe_info(ppp0_dev, wan_dev, sid, our_ip, peer_ip,our_mac, peer_mac);
	if(pppoe_info_ptr!=NULL)
	{
		*last_rx=pppoe_info_ptr->last_rx;
		*last_tx=pppoe_info_ptr->last_tx;
	}
	return 0;
}


int pppoe_fast_enter(struct sk_buff *skb)
{
	unsigned char *mac_hdr=NULL;
	unsigned short sid;
	struct pppoe_info* pppoe_info_ptr=NULL;
	u16 ppp_proto;

	/*
	if(fast_pppoe_fw==0)
	{
		return 0;
	}
	*/
	
	rtl_set_skb_pppoe_flag(skb,0);
	//printk("%s:%d,%s\n",__FUNCTION__,__LINE__,skb->dev->name);
	mac_hdr=rtl_skb_mac_header(skb);
	/*
	if(mac_hdr==NULL)
	{
		return 0;
	}
	*/
	if (*((u16 *)(mac_hdr + 12)) == htons(ETH_P_PPP_SES))
	{
		ppp_proto = ntohs(*((u16 *)(mac_hdr + 20)));
		if (ppp_proto != PPP_IP && ppp_proto != PPP_IPV6)
			return 0;
	}
	else
		return 0;

	/*should use sid to find pppoe info here*/
	sid = ntohs(*(unsigned short *)(&mac_hdr[16]));
	pppoe_info_ptr=find_pppoe_info_fast(sid,&mac_hdr[6]);
	if(pppoe_info_ptr==NULL)
	{
		return 0;
	}
	if ((pppoe_info_ptr->ppp0_dev == NULL) || (pppoe_info_ptr->wan_dev == NULL) ||
		pppoe_info_ptr->chan == NULL || pppoe_info_ptr->sk == NULL)
	{
		return 0;
	}
	rtl_set_skb_network_header(skb,mac_hdr+22);	
	rtl_set_skb_transport_header(skb,mac_hdr+22);
	rtl_skb_pull(skb,8);
	rtl_set_skb_rx_dev(skb,rtl_get_skb_dev(skb));
	/*printk("[%s-%u] %s,%s,%s: 0x%x\n", __func__, __LINE__,
	rtl_get_skb_dev(skb)->name, ((struct net_device *)pppoe_info_ptr->ppp0_dev)->name,
	((struct net_device *)pppoe_info_ptr->wan_dev)->name, ppp_proto);*/
	rtl_set_skb_protocol(skb, ppp_proto == 0x0021 ? htons(ETH_P_IP) : htons(ETH_P_IPV6));
	rtl_set_skb_dev(skb,(struct net_device *)pppoe_info_ptr->ppp0_dev);
	rtl_get_skb_dev(skb)->stats.rx_packets++;
	rtl_get_skb_dev(skb)->stats.rx_bytes += rtl_get_skb_len(skb);
	rtl_set_skb_pppoe_flag(skb,1);
	pppoe_info_ptr->last_rx=jiffies;
#if defined(CONFIG_RTL_FAST_PPPOE_DEBUG)
	pppoe_info_ptr->total_rx++;
#endif
	if (
#ifdef CONFIG_RTL_FAST_IPV6
(fast_ipv6_fw && ppp_proto == PPP_IPV6 && ipv6_fast_enter(skb) == NET_RX_DROP) ||
#endif
		(ppp_proto == PPP_IP && enter_fast_path((void *)skb) == NET_RX_DROP)) {

	} else {
		skb_push(skb, 2);
		((struct sk_buff *)skb)->dev = pppoe_info_ptr->wan_dev;
		ppp_input(pppoe_info_ptr->chan, skb);
	}
	return 1;
}


void check_and_restore_pppoe_hdr(struct sk_buff *skb)
{	
#if defined(CONFIG_RTL_FAST_PPPOE_DEBUG)
	struct pppoe_info* pppoe_info_ptr=NULL;
	unsigned char *mac_hdr=NULL;
#endif

	if(fast_pppoe_fw==0)
	{
		return;
	}
	
	if(rtl_get_skb_pppoe_flag(skb))
	{
		rtl_skb_push(skb,8);
		rtl_set_skb_network_header(skb,rtl_get_skb_data(skb));
		rtl_set_skb_transport_header(skb, rtl_get_skb_data(skb));
		
		rtl_get_skb_dev(skb)->stats.rx_packets--;
		rtl_get_skb_dev(skb)->stats.rx_bytes -= rtl_get_skb_len(skb);
		rtl_set_skb_dev(skb,rtl_get_skb_rx_dev(skb));
		rtl_set_skb_protocol(skb,htons(ETH_P_PPP_SES));
		rtl_set_skb_pppoe_flag(skb,0);
		#if defined(CONFIG_RTL_FAST_PPPOE_DEBUG)
		mac_hdr=rtl_skb_mac_header(skb);
		pppoe_info_ptr=find_pppoe_info(NULL,NULL,0,0,0,NULL,&mac_hdr[6]);
		if(pppoe_info_ptr)
		{
			pppoe_info_ptr->total_rx--;
		}
		#endif
	}
	
}

int fast_pppoe_xmit(struct sk_buff *skb)
{
	struct pppoe_info *pppoe_info_ptr;
	struct pppoe_hdr *ph;
	int data_len;
	unsigned char *dev_addr=NULL;
	struct vlan_dev_priv *vlan;
	struct vlan_ethhdr *veth;

	if(fast_pppoe_fw ==0) 
	{
		return 0;
	}
	
#if 0//defined(CONFIG_NET_SCHED)
	if (gQosEnabled) 
	{
		return 0;
	}
#endif
	if((rtl_skb_headroom(skb)<22) || rtl_skb_cloned(skb) || rtl_skb_shared(skb))
	{
		return 0;
	}

	pppoe_info_ptr = find_pppoe_info(rtl_get_skb_dev(skb), NULL, 0, 0, 0,NULL, NULL);
	
	if(pppoe_info_ptr==NULL)
	{
		return 0;
	}

	if ((pppoe_info_ptr->ppp0_dev == NULL) || 
		(pppoe_info_ptr->wan_dev == NULL) ||
		(pppoe_info_ptr->sid==0))
	{
		return 0;
	}

	data_len= rtl_get_skb_len(skb);
#if 0	
	rtl_skb_push(skb, 2);
	*(unsigned short *)rtl_get_skb_data(skb)=htons(0x0021);
	
	rtl_skb_push(skb, sizeof(struct pppoe_hdr));
	ph =(struct pppoe_hdr *)rtl_get_skb_data(skb);
	ph->ver = 1;
	ph->type = 1;
	ph->code = 0;
	ph->sid = htons(pppoe_info_ptr->sid);	/*jwj: pppoe_info_ptr->sid is big endian*/
	ph->length = htons(data_len+2);
	rtl_set_skb_dev(skb,wan_dev);

	rtl_skb_push(skb, 2);
	*(unsigned short *)(rtl_get_skb_data(skb))=htons(0x8864);

	rtl_skb_push(skb, 12);
	
	memcpy(rtl_get_skb_data(skb),pppoe_info_ptr->peer_mac,6);
	memcpy(rtl_get_skb_data(skb)+6,wan_dev->dev_addr,6);
#else
	/*12+2+sizeof(*ph)+2*/
         rtl_skb_push(skb, (16+sizeof(*ph)));	
         
         memcpy(rtl_get_skb_data(skb),pppoe_info_ptr->peer_mac,6);
	  //memcpy(rtl_get_skb_data(skb)+6, wan_dev->dev_addr, 6);

	  dev_addr=rtl_get_dev_addr((struct net_device *)pppoe_info_ptr->wan_dev);	 
	  memcpy(rtl_get_skb_data(skb)+6, dev_addr, 6);      
	
         *(unsigned short *)(rtl_get_skb_data(skb)+12)=htons(0x8864);

         ph =(struct pppoe_hdr *)(rtl_get_skb_data(skb)+14);
         ph->ver = 1;
         ph->type = 1;
         ph->code = 0;
         ph->sid = htons(pppoe_info_ptr->sid);
         ph->length = htons(data_len+2);
         rtl_set_skb_dev(skb, (struct net_device *)pppoe_info_ptr->wan_dev_slave);

         *(unsigned short *)(rtl_get_skb_data(skb)+14+sizeof(*ph))=htons(0x0021);
#endif
	pppoe_info_ptr->last_tx= jiffies;
#if defined(CONFIG_RTL_FAST_PPPOE_DEBUG)
	pppoe_info_ptr->total_tx++;
#endif
	vlan = vlan_dev_priv(((struct sk_buff *)skb)->dev);
	veth = (struct vlan_ethhdr *)(((struct sk_buff *)skb)->data);
	if ((((struct sk_buff *)skb)->dev->priv_flags & IFF_802_1Q_VLAN) && (vlan->flags & VLAN_FLAG_REORDER_HDR)) {
		u16 vlan_tci;
		vlan_tci = vlan->vlan_id;
		vlan_tci |= vlan_dev_get_egress_qos_mask(((struct sk_buff *)skb)->dev, ((struct sk_buff *)skb)->priority);
		__vlan_hwaccel_put_tag(skb, vlan->vlan_proto, vlan_tci);
		__vlan_hwaccel_push_inside(skb);
		((struct sk_buff *)skb)->dev = vlan->real_dev;
	}
	if (gQosEnabled)
		dev_queue_xmit(skb);
	else
	 	rtl_call_skb_ndo_start_xmit(skb);
	return 1;
	
}

extern unsigned int statistic_ipv6_pppoe_xmit;

int fast_pppoe_xmit6(struct sk_buff *skb)
{
	struct pppoe_info *pppoe_info_ptr;
	struct pppoe_hdr *ph;
	int data_len;
	unsigned char *dev_addr=NULL;
	struct vlan_dev_priv *vlan;
	struct vlan_ethhdr *veth;

	if(fast_pppoe_fw ==0) 
	{
		return 0;
	}

	if((rtl_skb_headroom(skb)<22) || rtl_skb_cloned(skb) || rtl_skb_shared(skb))
	{
		return 0;
	}

	pppoe_info_ptr = find_pppoe_info(rtl_get_skb_dev(skb), NULL, 0, 0, 0,NULL, NULL);
	
	if(pppoe_info_ptr==NULL)
	{
		return 0;
	}

	if ((pppoe_info_ptr->ppp0_dev == NULL) || 
		(pppoe_info_ptr->wan_dev == NULL) ||
		(pppoe_info_ptr->sid==0))
	{
		return 0;
	}
	data_len= rtl_get_skb_len(skb);
#if 0	
	rtl_skb_push(skb, 2);
	*(unsigned short *)rtl_get_skb_data(skb)=htons(0x0021);
	
	rtl_skb_push(skb, sizeof(struct pppoe_hdr));
	ph =(struct pppoe_hdr *)rtl_get_skb_data(skb);
	ph->ver = 1;
	ph->type = 1;
	ph->code = 0;
	ph->sid = htons(pppoe_info_ptr->sid);	/*jwj: pppoe_info_ptr->sid is big endian*/
	ph->length = htons(data_len+2);
	rtl_set_skb_dev(skb,wan_dev);

	rtl_skb_push(skb, 2);
	*(unsigned short *)(rtl_get_skb_data(skb))=htons(0x8864);

	rtl_skb_push(skb, 12);
	
	memcpy(rtl_get_skb_data(skb),pppoe_info_ptr->peer_mac,6);
	memcpy(rtl_get_skb_data(skb)+6,wan_dev->dev_addr,6);
#else
	/*12+2+sizeof(*ph)+2*/
     rtl_skb_push(skb, (16+sizeof(*ph)));	
     
     memcpy(rtl_get_skb_data(skb),pppoe_info_ptr->peer_mac,6);
  	//memcpy(rtl_get_skb_data(skb)+6, wan_dev->dev_addr, 6);

  	dev_addr=rtl_get_dev_addr((struct net_device *)pppoe_info_ptr->wan_dev);	 
  	memcpy(rtl_get_skb_data(skb)+6, dev_addr, 6);      
	/*printk("[%s-%u] %s,%s,%s: 0x%x\n", __func__, __LINE__,
	rtl_get_skb_dev(skb)->name, ((struct net_device *)pppoe_info_ptr->ppp0_dev)->name,
	((struct net_device *)pppoe_info_ptr->wan_dev)->name, 0x0057);*/
    *(unsigned short *)(rtl_get_skb_data(skb)+12)=htons(0x8864);

	ph =(struct pppoe_hdr *)(rtl_get_skb_data(skb)+14);
	ph->ver = 1;
	ph->type = 1;
	ph->code = 0;
	ph->sid = htons(pppoe_info_ptr->sid);
	ph->length = htons(data_len+2);
	rtl_set_skb_dev(skb,(struct net_device *)pppoe_info_ptr->wan_dev_slave);

	// ipv6 = 0x0057, ipv4 = 0x0021
	*(unsigned short *)(rtl_get_skb_data(skb)+14+sizeof(*ph))=htons(0x0057);
#endif
	statistic_ipv6_pppoe_xmit++;
	pppoe_info_ptr->last_tx= jiffies;
#if defined(CONFIG_RTL_FAST_PPPOE_DEBUG)
	pppoe_info_ptr->total_tx++;
#endif
	vlan = vlan_dev_priv(((struct sk_buff *)skb)->dev);
	veth = (struct vlan_ethhdr *)(((struct sk_buff *)skb)->data);
	if ((((struct sk_buff *)skb)->dev->priv_flags & IFF_802_1Q_VLAN) && (vlan->flags & VLAN_FLAG_REORDER_HDR)) {
		u16 vlan_tci;
		vlan_tci = vlan->vlan_id;
		vlan_tci |= vlan_dev_get_egress_qos_mask(((struct sk_buff *)skb)->dev, ((struct sk_buff *)skb)->priority);
		__vlan_hwaccel_put_tag(skb, vlan->vlan_proto, vlan_tci);
		__vlan_hwaccel_push_inside(skb);
		((struct sk_buff *)skb)->dev = vlan->real_dev;
	}
	if (gQosEnabled)
		dev_queue_xmit(skb);
	else
	 	rtl_call_skb_ndo_start_xmit(skb);
	return 1;
	
}

#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
int fast_pppoe_xmit_check(struct sk_buff *skb)
{
	struct pppoe_info *pppoe_info_ptr;

	if (fast_pppoe_fw == 0) 
		return 0;

	if ((rtl_skb_headroom(skb)<22) ||rtl_skb_cloned(skb) ||rtl_skb_shared(skb))
		return 0;

	pppoe_info_ptr = find_pppoe_info(rtl_get_skb_dev(skb), NULL, 0, 0, 0,NULL, NULL);
	
	if (pppoe_info_ptr == NULL)
		return 0;

	if ((pppoe_info_ptr->ppp0_dev == NULL) || 
		(pppoe_info_ptr->wan_dev == NULL) ||
		(pppoe_info_ptr->sid==0))
	{
		return 0;
	}

	return 1;
}

int fast_pppoe_xmit2(struct sk_buff *skb)
{
	int data_len;
	struct pppoe_hdr *ph;
	struct pppoe_info *pppoe_info_ptr;
	unsigned char *dev_addr=NULL;
	struct vlan_dev_priv *vlan;
	struct vlan_ethhdr *veth;

	pppoe_info_ptr = find_pppoe_info(rtl_get_skb_dev(skb), NULL, 0, 0, 0,NULL, NULL);
	
	if (pppoe_info_ptr == NULL)
		goto fail;

	if ((pppoe_info_ptr->ppp0_dev == NULL) || 
		(pppoe_info_ptr->wan_dev == NULL) ||
		(pppoe_info_ptr->sid==0))
	{
		goto fail;
	}
	
	//printk("ppp_dev is %s,wan_dev is %s,sid is %d\n", pppoe_info_ptr->ppp_dev, pppoe_info_ptr->wan_dev, pppoe_info_ptr->sid);
	data_len= rtl_get_skb_len(skb);

#if 0
	rtl_skb_push(skb, 2);
	*(unsigned short *)rtl_get_skb_data(skb)=htons(0x0021);
	
	rtl_skb_push(skb, sizeof(struct pppoe_hdr));
	ph =(struct pppoe_hdr *)rtl_get_skb_data(skb);
	ph->ver = 1;
	ph->type = 1;
	ph->code = 0;
	ph->sid = htons(pppoe_info_ptr->sid);
	ph->length = htons(data_len+2);
	rtl_set_skb_dev(skb,wan_dev);

	rtl_skb_push(skb, 2);
	*(unsigned short *)(rtl_get_skb_data(skb))=htons(0x8864);

	rtl_skb_push(skb, 12);
	
	memcpy(rtl_get_skb_data(skb),pppoe_info_ptr->peer_mac,6);
	memcpy(rtl_get_skb_data(skb)+6,wan_dev->dev_addr,6);
#else
	/*12+2+sizeof(*ph)+2*/
	rtl_skb_push(skb, (16+sizeof(*ph)));
		 
	memcpy(rtl_get_skb_data(skb),pppoe_info_ptr->peer_mac,6);
	  //memcpy(rtl_get_skb_data(skb)+6, wan_dev->dev_addr, 6);

	  dev_addr=rtl_get_dev_addr(pppoe_info_ptr->wan_dev);	 
	  memcpy(rtl_get_skb_data(skb)+6, dev_addr, 6); 	 
	
	 *(unsigned short *)(rtl_get_skb_data(skb)+12)=htons(0x8864);

	 ph =(struct pppoe_hdr *)(rtl_get_skb_data(skb)+14);
	 ph->ver = 1;
	 ph->type = 1;
	 ph->code = 0;
	 ph->sid = htons(pppoe_info_ptr->sid);
	 ph->length = htons(data_len+2);
	 rtl_set_skb_dev(skb, pppoe_info_ptr->wan_dev_slave);

	 *(unsigned short *)(rtl_get_skb_data(skb)+14+sizeof(*ph))=htons(0x0021);
#endif
	pppoe_info_ptr->last_tx= jiffies;
#if defined(CONFIG_RTL_FAST_PPPOE_DEBUG)
	pppoe_info_ptr->total_tx++;
#endif
	vlan = vlan_dev_priv(((struct sk_buff *)skb)->dev);
	veth = (struct vlan_ethhdr *)(((struct sk_buff *)skb)->data);
	if ((((struct sk_buff *)skb)->dev->priv_flags & IFF_802_1Q_VLAN) && (vlan->flags & VLAN_FLAG_REORDER_HDR)) {
		u16 vlan_tci;
		vlan_tci = vlan->vlan_id;
		vlan_tci |= vlan_dev_get_egress_qos_mask(((struct sk_buff *)skb)->dev, ((struct sk_buff *)skb)->priority);
		__vlan_hwaccel_put_tag(skb, vlan->vlan_proto, vlan_tci);
		__vlan_hwaccel_push_inside(skb);
		((struct sk_buff *)skb)->dev = vlan->real_dev;
	}
	dev_queue_xmit(skb);
	return 1;

fail:
	kfree_skb(skb);
	return 0;
}

#endif

#endif

