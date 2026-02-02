#include "linux/ppp_channel.h"
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
#include <linux/version.h>

#include <linux/in.h>
#include <linux/udp.h>
#include <linux/if_tunnel.h>
#include <linux/if_ether.h>
#include <net/rtl/rtl_types.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <uapi/linux/netfilter/nf_conntrack_tuple_common.h>
#endif
#include <net/rtl/fastpath/fastpath_core.h>

#ifdef FAST_L2TP
#include "../fast_l2tp_core.h"
#endif

/*common*/
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
#include <linux/if_bridge.h>
#include <linux/if_vlan.h>

#if defined(CONFIG_NET_SCHED)
extern int gQosEnabled;
#endif

#if defined(FAST_L2TP)

////static struct l2tp_info l2tpInfo={NULL};
struct l2tp_info l2tpInfo={NULL};
uint32 state;
#define L2TP_INITED 0
#define L2TP_REPLY_RECEIVED 1
#define L2TP_CONNECTED 2

//#ifdef CONFIG_SUPPORT_RUSSIA_FEATURES
unsigned int l2tp_ppp_imagic = 0 ;
//#endif
#if 1 // sync from voip customer for multiple ppp
int is_l2tp_device(char *ppp_device)
{
	if (l2tpInfo.ppp0_dev && !strcmp(rtl_get_ppp_dev_name(l2tpInfo.ppp0_dev), ppp_device))
	{
		//printk("%s: l2tp device = %s\n", __FUNCTION__, ppp_device);
		return 1;
	}
	return 0;
}

void set_l2tp_device(char *ppp_device)
{
	if (fast_l2tp_fw)
	{
		//printk("%s: l2tp device = %s\n", __FUNCTION__, ppp_device);
		l2tpInfo.ppp0_dev = (void *)rtl_get_dev_by_name(ppp_device);
	}
}
#endif

void event_ppp_dev_down(const char *name)
{
	if(l2tpInfo.valid)
	{
		if(l2tpInfo.ppp0_dev && strcmp(rtl_get_ppp_dev_name(l2tpInfo.ppp0_dev),name) == 0)
		{
			memset(&l2tpInfo, 0, sizeof(l2tpInfo));
			state = 0;
		}
	}
}

static inline unsigned int rtl_get_skb_l2tp_flag(struct sk_buff *skb)
{
	return skb->l2tp_flag & 0xffffu;
}

static inline rtl_set_skb_l2tp_flag(struct sk_buff *skb, unsigned int l2tp_flag)
{
	skb->l2tp_flag = l2tp_flag | (skb->l2tp_flag & 0xffff0000u);
}

static inline unsigned int rtl_get_skb_l2tp_len(struct sk_buff *skb)
{
	return skb->l2tp_flag >> 16;
}

static inline rtl_set_skb_l2tp_len(struct sk_buff *skb, unsigned int len)
{
	skb->l2tp_flag = (len << 16) | (skb->l2tp_flag & 0xffffu);
}

static inline struct net_device *rtl_get_skb_rx_l2tp_dev(struct sk_buff *skb)
{
	return skb->rx_l2tp_dev;
}

static inline void rtl_set_skb_rx_l2tp_dev(struct sk_buff *skb, struct net_device *dev)
{
	skb->rx_l2tp_dev = dev;
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

struct ppp_channel *get_l2tp_chan(struct sock **sk_out)
{
	struct net_device *pptpInf = (struct net_device *)l2tpInfo.ppp0_dev;
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

#ifdef DOS_FILTER
extern void filter_addconnect(__u32 ipaddr);
#endif

void l2tp_id(void *skb,int tx,int rx)
{
        __u16 *p_id;
        __u16 MessageType;
	__u16 *ppp_protocol;
	struct iphdr *iph;
	struct udphdr *hdr;
	struct l2tp_header *l2tp_ptr;
	unsigned char *data;


	data = rtl_get_skb_data(skb);
	iph = rtl_ip_hdr(skb);
	if(iph == NULL)
		return;
       hdr = (struct udphdr *)((u_int32_t *)iph + iph->ihl);
       l2tp_ptr = (struct l2tp_header *)((u_int8_t *)hdr+8);
//	   printk("%s:%d hdr->dest=0x%x src=0x%x\n",__FUNCTION__,__LINE__,hdr->dest,hdr->source);
       if(
	   	((rx && hdr->source==htons(1701))||(tx && hdr->dest==htons(1701))) 
	   	&&((l2tp_ptr->ver & htons(0x8000))==htons(control_message)))
	{
		if(l2tp_ptr->length < htons(20)){
			return;
		}
	
	  	if((l2tp_ptr->ver & htons(0x4000))==0)
	    		p_id=(__u16 *)((u_int8_t *)(&l2tp_ptr->ver)+2);
	  	else
	    		p_id=&l2tp_ptr->tid;

          	MessageType=*(p_id+7);
          	if(MessageType==htons(stop_control) && l2tpInfo.tid==*p_id)
	  	{
	  		memset(&l2tpInfo, '\0', sizeof(l2tpInfo));
			DEBUGP("FAST-L2TP, stop control, dev->name=%s\n", rtl_get_skb_dev_name(skb));
			state = L2TP_INITED;
	  	}

          	if(MessageType==htons(call_reply))
	  	{
	  		/*hyking:
	  		*bug fix:l2tp wantype,l2tp pass thru
	  		*/
	  		/*log mac information when from wan interface, not from br0 or to br0*/
			extern int rtl865x_curOpMode;
			
			if(state < L2TP_REPLY_RECEIVED && 
				rtl_is_wan_dev(skb)
   			)		
#if defined(CONFIG_RTL_819X_SWCORE)
	  		{
				l2tpInfo.wan_dev = (void*)rtl_get_skb_dev(skb);
				l2tpInfo.wan_dev_slave = br_get_first_slave(l2tpInfo.wan_dev);
				l2tpInfo.wan_dev_slave = l2tpInfo.wan_dev_slave ? l2tpInfo.wan_dev_slave : l2tpInfo.wan_dev;
		      		memcpy(&l2tpInfo.mac_header[0], data-ETH_HLEN+ETH_ALEN, ETH_ALEN); // da of tx pkt
				memcpy(&l2tpInfo.mac_header[ETH_ALEN], data-ETH_HLEN, ETH_ALEN); // sa of tx pkt
				memcpy(&l2tpInfo.mac_header[ETH_ALEN*2], data-2, 2); // type
				DEBUGP("FAST-L2TP: call-reply, dev->name=%s\n", rtl_get_skb_dev_name(skb));
				state = L2TP_REPLY_RECEIVED;
	  		}
#endif
	    	}

          	if(MessageType==htons(connect_control) && *(p_id+1)!= 0)
	  		{
			/*log ip/session id information when from protocal stack, not from br0*/
				if(state < L2TP_CONNECTED)
				{
			    		l2tpInfo.tid=*p_id;
			    		l2tpInfo.cid=*(p_id+1);
		      	    		l2tpInfo.saddr = iph->saddr;
		      	    		l2tpInfo.daddr = iph->daddr;
							
							l2tpInfo.sport = hdr->source;
							
							//printk("%s:%d tx=%d rx=%d src=%d dst=%d\n",__FUNCTION__,__LINE__,tx,rx,hdr->source,hdr->dest);
#ifdef DOS_FILTER
					 filter_addconnect(l2tpInfo.daddr);
#endif
					l2tpInfo.valid = 1;
					DEBUGP("FAST-L2TP: connected\n");
					state = L2TP_CONNECTED;
				}
	    	}
	  }
	//hyking: if l2tp termination happen, reset l2tpInfo & state...
	  else if ((rx && hdr->source==htons(1701))||(tx && hdr->dest==htons(1701)))
	  {
	  	//data message
		//length bit is set?
	  	if((l2tp_ptr->ver & htons(0x4000))==0)
	    		p_id=(__u16 *)((u_int8_t *)(&l2tp_ptr->ver)+2);
	  	else
	    		p_id=&l2tp_ptr->tid;

		ppp_protocol = p_id+3;
#ifdef DOS_FILTER
		if(filter_checkConnect(l2tpInfo.daddr))
			filter_addconnect(l2tpInfo.daddr);
#endif

		//link control protocol
		if(*ppp_protocol == htons(0xc021))
		{
			//termination request 
			if( *((u_int8_t *)(((u_int8_t *)ppp_protocol) + 2)) == 0x05)
			{
				printk("FAST-L2TP: termination request\n");
				memset(&l2tpInfo, 0, sizeof(l2tpInfo));
				state = 0;
			}			
			//termination ack
			if( *((u_int8_t *)(((u_int8_t *)ppp_protocol) + 2)) == 0x06)
			{
				if(l2tpInfo.tid == (*p_id) && l2tpInfo.cid == (*(p_id+1)))
				{
					printk("FAST-L2TP: termination ack\n");
					memset(&l2tpInfo, 0, sizeof(l2tpInfo));
					state = 0;
				}
			}
//#ifdef CONFIG_SUPPORT_RUSSIA_FEATURES
			//get the imagic number for tx hook Lcp request
			if( *((u_int8_t *)(((u_int8_t *)ppp_protocol) + 2)) == 0x01)
			{
				unsigned char *pdata = (unsigned char *)((ppp_protocol) + 1);	//offset 2 bytes
				__u16 Total_length = ntohs(*((unsigned short *)(pdata+2)));
				unsigned char attribute_type;
				int attribute_len;
				int cur_offset = 4;
				while(cur_offset < Total_length)
				{
					attribute_type = *(pdata+cur_offset);				
					attribute_len =  *(pdata+cur_offset+1);	
					if(attribute_type == 0x5)	//magic number type
					{
						l2tp_ppp_imagic = *((u_int32_t*)(pdata+cur_offset+2));
						break;
					}
					cur_offset += attribute_len;
				}
			}

//#endif
		}
	  }
	return;
}

//move this function to fastpath_common.c
#if 0 //def CONFIG_SUPPORT_RUSSIA_FEATURES
// direct tx ppp reply ptk to l2tp server
int Direct_Send_Reply(struct sk_buff * skb, int offset)
{	

	int header_len;
	struct iphdr *iph,*iph_new, iph_newone;
	struct Rus_l2tp_ext_hdr *l2tph, l2tphone;
	unsigned char tos;
	unsigned short frag_off;
	struct sk_buff *new_skb;
	// var define ---------
	unsigned char *data;
	data = rtl_get_skb_data(skb);	
	unsigned char req_id;
	unsigned short req_len;

	//if the info is integrity, conitune to contruct the ptk.
	if(!fast_l2tp_fw || l2tpInfo.tid==0 || l2tpInfo.cid==0 || !l2tpInfo.wan_dev)
		return 0;

	if(l2tpInfo.valid != 1 || !l2tp_ppp_imagic)
		return 0;

	//extract necessary info
	{
		req_id = data[offset+1];
		req_len = *((unsigned short*)(&data[offset+2]));
	}

	//contruct the ptk
	skb_push(skb,ETH_HLEN);			//mac header push 
	{
		// build the mac header 
		memcpy(skb->data, l2tpInfo.mac_header, ETH_HLEN);
		
		// build ip header
		iph_new = &iph_newone;
		iph_new->version	=	4;
		iph_new->ihl		=	sizeof(struct iphdr) >> 2;
		//iph_new->frag_off =	frag_off;
		iph_new->frag_off	=	0x4000;
		iph_new->protocol	=	IPPROTO_UDP;

		//need confirm the value --------------------------------------
		iph_new->tos		=	0;
		
		iph_new->daddr	=	l2tpInfo.daddr;
		iph_new->saddr	=	l2tpInfo.saddr;
		
		iph_new->ttl		=	IPDEFTTL;
		skb->ip_summed	=	CHECKSUM_NONE;
		iph_new->tot_len	=	htons(skb->len - ETH_HLEN);
		iph_new->id 	=	0;

		iph_new->check	=	0;
		iph_new->check	=	ip_fast_csum((unsigned char *)iph_new, iph_new->ihl);	
		memcpy(skb->data + ETH_HLEN, &iph_newone, sizeof(iph_newone));

		// build udp header
		l2tph = &l2tphone;
		l2tph->source	=1701;
		l2tph->dest =1701;
		//len ,need to confirm to modify
		if(req_len >8)
			l2tph->len	= 30;	
		else
			l2tph->len	= 26;				
		l2tph->checksum=0;
		
		//build l2tp header
		l2tph->type =0x0002;
		l2tph->tid	=l2tpInfo.tid;
		l2tph->cid	=l2tpInfo.cid;
		
		//build ppp header
		l2tph->addr_control= 0xff03;
		l2tph->protocol =0xc021;
		l2tph->code = 0x0a;
		l2tph->id = req_id;
		l2tph->lcp_length= req_len;
		//message && imagicNumber has the same value
		l2tph->imagicNumber = l2tp_ppp_imagic;
		if(req_len > 8 )
		l2tph->message = l2tp_ppp_imagic;
		
		memcpy(skb->data+ETH_HLEN+sizeof(struct iphdr), &l2tphone, sizeof(struct Rus_l2tp_ext_hdr)-12+req_len);

		//reset the skb info
		skb->len = ETH_HLEN+sizeof(struct iphdr)+sizeof(struct Rus_l2tp_ext_hdr)-12+req_len;
		
		//set the ethx device to xmit
		skb->dev=l2tpInfo.wan_dev;
		l2tpInfo.last_xmit = jiffies;

		//panic_printk("%s[%d] --\n",__FUNCTION__,__LINE__);
		skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
		//kfree_skb(skb);
		return 1;		
	}
}
#endif

int fast_l2tp_rx(void *skb)
{
	struct iphdr *iph=rtl_ip_hdr(skb);
	struct udphdr *hdr= (struct udphdr *)(((unsigned char *)iph) + iph->ihl*4);
	int rightShift = 0;
	unsigned char *data;
	struct ppp_channel *chan;
//#ifdef CONFIG_SUPPORT_RUSSIA_FEATURES
//	__u16 *ppp_protocol;
//#endif
	rtl_set_skb_l2tp_flag(skb, 0);
	data = rtl_get_skb_data(skb);

	if(rtl_get_skb_len(skb) < 40)
		return 0;

	if ((hdr->source==htons(1701)) 
#if 0
	&& (hdr->dest==htons(1701))
#endif
) {
	//printk("%s:%d hdr->dest=0x%x src=0x%x\n",__FUNCTION__,__LINE__,hdr->dest,hdr->source);
		if (l2tpInfo.wan_dev == NULL)
			l2tp_id(skb,0,1);

		/* skb->data[28] is L2TP Flags and version.
		 * Control flags indicating data/control packet and presence of length,
		 * sequence, offset fields.
		 * Original length of header is 6.
		 * Bit 7 is length flag. If Bit 7 is on then the length of header will extend 2 bytes
		 * Bit 2 is offset flag. If Bit 2 is on then the length of header will extend 2 bytes
		 */
		if(data[28] & 0x40)
			rightShift = 2;

		if(data[28] & 0x02) // offset option is on
			rightShift += 2;

	//move the following to function fast_l2tp_tx_lcp_echo_reply() in fastpath_common.c 	
#if 0 //def CONFIG_SUPPORT_RUSSIA_FEATURES
		//patch for russia ppp disconnect 
		{			
			ppp_protocol = &data[36+rightShift];
			
			if(*ppp_protocol == 0xc021)
			{
				if( *((u_int8_t *)(((u_int8_t *)ppp_protocol) + 2)) == 0x09)//request
				{
					//contruct the reply ptk to eth dirver to send		
					Direct_Send_Reply(skb,38+rightShift);
					return 1;
				}
			}
		}		
#endif
		chan = get_l2tp_chan(NULL);
		if (fast_l2tp_tx_lcp_echo_reply(data, rightShift, skb)==1 || l2tpInfo.ppp0_dev == NULL || !chan)
			return 1;
		if (l2tpInfo.tid!=0 && l2tpInfo.cid!=0 && (data[36+rightShift]==0 && data[37+rightShift]==0x21) &&
			l2tpInfo.wan_dev && l2tpInfo.saddr==iph->daddr
		#if 0 /// fixme: should check tunnel id & session id
			&& l2tpInfo.daddr== iph->saddr
		#endif				
				) {
#if 0
	      		memcpy(&l2tpInfo.mac_header[0], skb->data-ETH_HLEN+ETH_ALEN, ETH_ALEN); // da of tx pkt
			memcpy(&l2tpInfo.mac_header[ETH_ALEN], skb->data-ETH_HLEN, ETH_ALEN); // sa of tx pkt
			memcpy(&l2tpInfo.mac_header[ETH_ALEN*2], skb->data-2, 2); // type
#endif
			if (l2tpInfo.ppp0_dev) {
				if (((struct iphdr *)(&data[38+rightShift]))->protocol == IPPROTO_TCP ||
					((struct iphdr *)(&data[38+rightShift]))->protocol == IPPROTO_UDP ||
					((struct iphdr *)(&data[38+rightShift]))->protocol == IPPROTO_ICMP ||
					((struct iphdr *)(&data[38+rightShift]))->protocol == IPPROTO_GRE) {

					rtl_set_skb_rx_l2tp_dev((struct sk_buff *)skb, ((struct sk_buff *)skb)->dev);
					rtl_set_skb_l2tp_len((struct sk_buff *)skb, 38 + rightShift);
					rtl_set_skb_dev(skb, l2tpInfo.ppp0_dev);
					skb_pull(skb, 38+rightShift);
					rtl_skb_reset_network_header(skb);
					rtl_skb_reset_transport_header(skb);
					//skb->transport_header=skb->network_header=skb->data;
					#if 1 /* update number of received l2tp packetes into ppp0 statistic */
					{
						void *ppp;
						if(l2tpInfo.ppp0_dev && rtl_get_ppp_dev_priv(l2tpInfo.ppp0_dev)){
							ppp = rtl_get_ppp_dev_priv(l2tpInfo.ppp0_dev);
							rtl_inc_ppp_stats(ppp, 0, rtl_get_skb_len(skb));
							//get_ppp_stats(ppp)->rx_packets++;
							//get_ppp_stats(ppp)->rx_bytes += rtl_get_skb_len(skb);
						}
					}
				#endif
					//skb_reset_network_header(skb);
					//skb_reset_transport_header(skb);
					rtl_set_skb_l2tp_flag(skb, 1);
					if (enter_fast_path((void *)skb) == NET_RX_DROP) {

					} else {
						skb_push(skb, 2);
						((struct sk_buff *)skb)->dev = l2tpInfo.wan_dev;
						ppp_input(chan, skb);
					}
					return -1;
					DEBUGP("FAST-L2TP: delete l2tp header!\n");
				}
			}
		}
	  }
//#ifdef CONFIG_SUPPORT_RUSSIA_FEATURES
	return 0;
//#endif
}

void check_and_restore_l2tp_hdr(struct sk_buff *skb)
{	
	if (fast_l2tp_fw == 0)
	{
		return;
	}

	if (rtl_get_skb_l2tp_flag(skb))
	{
		rtl_get_skb_dev(skb)->stats.rx_packets--;
		rtl_get_skb_dev(skb)->stats.rx_bytes -= rtl_get_skb_len(skb);
		rtl_skb_push(skb, rtl_get_skb_l2tp_len(skb));
		rtl_set_skb_network_header(skb, rtl_get_skb_data(skb));
		rtl_set_skb_transport_header(skb, rtl_get_skb_data(skb));
		rtl_set_skb_dev(skb, rtl_get_skb_rx_l2tp_dev(skb));
		rtl_set_skb_l2tp_flag(skb,0);
	}
}

unsigned long get_fast_l2tp_lastxmit(void)
{
	if(l2tpInfo.valid == 1)
		return l2tpInfo.last_xmit;
	return 0;
}

// return 0: not to do fast l2tp to wan
// return 1: to do fast l2tp to wan
int check_for_fast_l2tp_to_wan(void *skb)
{
	struct iphdr *iph;

	iph = rtl_ip_hdr(skb);

	//if(iph->protocol==IPPROTO_ICMP)
		//return 0;

	//if (ip_hdr(skb)->frag_off & htons(IP_MF|IP_OFFSET))
		//return 0;

	// Patch for l2tp dial on-demand: pkts which will trigger l2tp dialing up should not do fast l2tp
	// dial on-demand initial ip: 10.64.64.*
	if((rtl_ip_hdr(skb)->saddr & htonl(0xffffff00)) == htonl(0xa404000))
		return 0;

	return 1;
}

extern int timeoutCheck_skipp_pkt(struct sk_buff *skb, int offset);
int fast_l2tp_to_wan(void *skb)
{
	int	header_len;
	struct iphdr *iph,*iph_new, iph_newone, *iph_tmp;
	struct l2tp_ext_hdr	*l2tph, l2tphone;
	unsigned char tos;
	unsigned short frag_off;
    	__u16 old_len;
	int offset;
	struct vlan_dev_priv *vlan;
	struct vlan_ethhdr *veth;

	if(!fast_l2tp_fw || l2tpInfo.tid==0 || l2tpInfo.cid==0 || !l2tpInfo.wan_dev)
		return 0;

	if(l2tpInfo.valid != 1)
		return 0;

	iph = rtl_ip_hdr(skb);
	old_len = ntohs(iph->tot_len);
	header_len = ETH_HLEN + sizeof(struct iphdr) +18 ;
	if (rtl_skb_headroom(skb) < header_len || rtl_skb_cloned(skb) || rtl_skb_shared(skb))
	{
		void *new_skb = (void*)skb_realloc_headroom(skb, header_len);
		if (!new_skb) {
			//panic_printk("%s: skb_realloc_headroom failed!\n", __FUNCTION__);
			return 0;
		}
		dev_kfree_skb(skb);
		skb = new_skb;
	}
	tos = iph->tos;
	frag_off = iph->frag_off;

	// build mac header
	memcpy(skb_push(skb, header_len), l2tpInfo.mac_header, ETH_HLEN);

	// build ip header
	iph_new = &iph_newone;
	iph_new->version	=	4;
	iph_new->ihl		=	sizeof(struct iphdr) >> 2;
	//iph_new->frag_off	=	frag_off;
	iph_new->frag_off	=	htons(0x4000);
	iph_new->protocol	=	IPPROTO_UDP;
	iph_new->tos		=	tos;
    	iph_new->daddr	=	l2tpInfo.daddr;
    	iph_new->saddr	=	l2tpInfo.saddr;
    	iph_new->ttl 		=	IPDEFTTL;
	rtl_set_skb_ip_summed(skb, CHECKSUM_NONE);
    	iph_new->tot_len	=	htons(rtl_get_skb_len(skb) - ETH_HLEN);
    	iph_new->id		=	0;

    	iph_new->check	=	0;
    	iph_new->check	=	ip_fast_csum((unsigned char *)iph_new, iph_new->ihl);
    	memcpy(rtl_get_skb_data(skb) + ETH_HLEN, &iph_newone, sizeof(iph_newone));
    	l2tph = &l2tphone;
 //   	l2tph->source	=htons(1701);
    	l2tph->source	= l2tpInfo.sport;
    	l2tph->dest	=htons(1701);
	l2tph->len	=htons(old_len+18);
	l2tph->checksum=0;
	l2tph->type	=htons(0x0002);
	l2tph->tid	=l2tpInfo.tid;
	l2tph->cid	=l2tpInfo.cid;
	l2tph->addr_control= htons(0xff03);
	l2tph->protocol	=htons(0x0021);
    	memcpy(rtl_get_skb_data(skb)+ETH_HLEN+sizeof(struct iphdr), &l2tphone, sizeof(struct l2tp_ext_hdr));

	rtl_set_skb_dev(skb, l2tpInfo.wan_dev_slave);

	DEBUGP("FAST-L2TP: fw to WAN!\n");

	iph_tmp =(struct iphdr *) (rtl_get_skb_data(skb)+ETH_HLEN+sizeof(struct iphdr)+sizeof(l2tphone));

	offset=ETH_HLEN+sizeof(struct iphdr)+sizeof(l2tphone);
	if(timeoutCheck_skipp_pkt(skb, offset)!=1)
		l2tpInfo.last_xmit = jiffies;


	#if 1 /* update number of transmitted l2tp packetes into ppp0 statistic */
	{
		extern struct net_device_stats *get_ppp_stats(void *ppp);
		void *ppp;
		if(l2tpInfo.ppp0_dev && rtl_get_ppp_dev_priv(l2tpInfo.ppp0_dev)){
			ppp = rtl_get_ppp_dev_priv(l2tpInfo.ppp0_dev);
			rtl_inc_ppp_stats(ppp, 1, rtl_get_skb_len(skb));
			//get_ppp_stats(ppp)->tx_packets++;
			//get_ppp_stats(ppp)->tx_bytes += rtl_get_skb_len(skb);
		}
	}
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
int fast_l2tp_to_wan_check(void *skb)
{
	if (!fast_l2tp_fw || l2tpInfo.valid != 1 ||
		l2tpInfo.tid==0 || l2tpInfo.cid==0 || !l2tpInfo.wan_dev)
		return 0;
	return 1;
}

int fast_l2tp_to_wan2(void *skb)
{
	int	header_len;
	struct iphdr *iph,*iph_new, iph_newone, *iph_tmp;
	struct l2tp_ext_hdr	*l2tph, l2tphone;
	unsigned char tos;
	unsigned short frag_off;
	int offset;
	__u16 old_len;
	struct vlan_dev_priv *vlan;
	struct vlan_ethhdr *veth;

	if(!fast_l2tp_fw || l2tpInfo.tid==0 || l2tpInfo.cid==0 || !l2tpInfo.wan_dev)
		goto fail;

	if(l2tpInfo.valid != 1)
		goto fail;

	iph = rtl_ip_hdr(skb);
	old_len = ntohs(iph->tot_len);
	header_len = ETH_HLEN + sizeof(struct iphdr) +18 ;
	if (rtl_skb_headroom(skb) < header_len || rtl_skb_cloned(skb) || rtl_skb_shared(skb))
	{
		void *new_skb = (void*)skb_realloc_headroom(skb, header_len);
		if (!new_skb) {
			//panic_printk("%s: skb_realloc_headroom failed!\n", __FUNCTION__);
			goto fail;
		}
		dev_kfree_skb(skb);
		skb = new_skb;
	}
	tos = iph->tos;
	frag_off = iph->frag_off;

	// build mac header
	memcpy(skb_push(skb, header_len), l2tpInfo.mac_header, ETH_HLEN);

	// build ip header
	iph_new = &iph_newone;
	iph_new->version	=	4;
	iph_new->ihl		=	sizeof(struct iphdr) >> 2;
	//iph_new->frag_off	=	frag_off;
	iph_new->frag_off	=	htons(0x4000);
	iph_new->protocol	=	IPPROTO_UDP;
	iph_new->tos		=	tos;
    	iph_new->daddr	=	l2tpInfo.daddr;
    	iph_new->saddr	=	l2tpInfo.saddr;
    	iph_new->ttl 		=	IPDEFTTL;
	rtl_set_skb_ip_summed(skb, CHECKSUM_NONE);
    	iph_new->tot_len	=	htons(rtl_get_skb_len(skb) - ETH_HLEN);
    	iph_new->id		=	0;

    	iph_new->check	=	0;
    	iph_new->check	=	ip_fast_csum((unsigned char *)iph_new, iph_new->ihl);
    	memcpy(rtl_get_skb_data(skb) + ETH_HLEN, &iph_newone, sizeof(iph_newone));
    	l2tph = &l2tphone;
		l2tph->source	= l2tpInfo.sport;
    	//l2tph->source	=1701;
    	l2tph->dest	=htons(1701);
	l2tph->len	= htons(old_len+18);
	l2tph->checksum=0;
	l2tph->type	= htons(0x0002);
	l2tph->tid	=l2tpInfo.tid;
	l2tph->cid	=l2tpInfo.cid;
	l2tph->addr_control= htons(0xff03);
	l2tph->protocol	= htons(0x0021);
    	memcpy(rtl_get_skb_data(skb)+ETH_HLEN+sizeof(struct iphdr), &l2tphone, sizeof(struct l2tp_ext_hdr));

	rtl_set_skb_dev(skb, l2tpInfo.wan_dev_slave);

	DEBUGP("FAST-L2TP: fw to WAN!\n");

	iph_tmp = rtl_get_skb_data(skb)+ETH_HLEN+sizeof(struct iphdr)+sizeof(l2tphone);

	offset=ETH_HLEN+sizeof(struct iphdr)+sizeof(l2tphone);
	if(timeoutCheck_skipp_pkt(skb, offset)!=1)
		l2tpInfo.last_xmit = jiffies;


	#if 1 /* update number of transmitted l2tp packetes into ppp0 statistic */
	{
		extern struct net_device_stats *get_ppp_stats(void *ppp);
		void *ppp;
		if(l2tpInfo.ppp0_dev && rtl_get_ppp_dev_priv(l2tpInfo.ppp0_dev)){
			ppp = rtl_get_ppp_dev_priv(l2tpInfo.ppp0_dev);
			rtl_inc_ppp_stats(ppp, 1, rtl_get_skb_len(skb));
			//get_ppp_stats(ppp)->tx_packets++;
			//get_ppp_stats(ppp)->tx_bytes += rtl_get_skb_len(skb);
		}
	}
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

#if 0
static int rtl_printkl2tpcache(void)
{
	printk("cid(%d),daddr(0x%x),ppp0_dev->name(%s),l2tpInfo.saddr(0x%x),l2tpInfo.tid(%d),l2tpInfo.wan_dev->name(%s),valid(%d)\n",
		l2tpInfo.cid,l2tpInfo.daddr,l2tpInfo.ppp0_dev->name,l2tpInfo.saddr,l2tpInfo.tid,
		l2tpInfo.wan_dev->name,l2tpInfo.valid);
		return 1;
}
#endif


static int l2tp_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char l2tp_tmp;

      if (count < 2)
	    return -EFAULT;

      if (buffer && !copy_from_user(&l2tp_tmp, buffer, 1)) {
	    	fast_l2tp_fw = l2tp_tmp - '0';
      		if(fast_l2tp_fw==1) {
		      l2tp_tx_id_hook=l2tp_id;
      		}
	      	else
		{
		      l2tp_tx_id_hook=NULL;
		      l2tpInfo.tid=0;
		}
		memset(&l2tpInfo, 0, sizeof(l2tpInfo));
		state = L2TP_INITED;
		return count;
      }
      return -EFAULT;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int l2tp_read_proc(struct seq_file *s, void *v)
{
	seq_printf(s, "%c\n", fast_l2tp_fw + '0');
	seq_printf(s, "FAST-L2TP: %u\n", state);
	seq_printf(s, "wan_dev %s\n", l2tpInfo.wan_dev ? ((struct net_device *)(l2tpInfo.wan_dev))->name : "null");
	seq_printf(s, "wan_dev %s\n", l2tpInfo.wan_dev_slave ? ((struct net_device *)(l2tpInfo.wan_dev_slave))->name : "null");
	seq_printf(s, "ppp0_dev %s\n", l2tpInfo.ppp0_dev ? ((struct net_device *)(l2tpInfo.ppp0_dev))->name : "null");
	seq_printf(s, "last_xmit %lu\n", l2tpInfo.last_xmit);
	seq_printf(s, "daddr %u.%u.%u.%u\n", NIPQUAD(l2tpInfo.daddr));
	seq_printf(s, "saddr %u.%u.%u.%u\n", NIPQUAD(l2tpInfo.saddr));
	seq_printf(s, "tid %u\n", l2tpInfo.tid);
	seq_printf(s, "cid %u\n", l2tpInfo.cid);
	seq_printf(s, "mac_header %02X-%02X-%02X-%02X-%02X-%02X\n", l2tpInfo.mac_header[0],
		l2tpInfo.mac_header[1], l2tpInfo.mac_header[2], l2tpInfo.mac_header[3],
		l2tpInfo.mac_header[4], l2tpInfo.mac_header[5]);
	seq_printf(s, "valid %lu\n", l2tpInfo.valid);
	seq_printf(s, "sport %lu\n", l2tpInfo.sport);
    return 0;
}

int fastpath_l2tp_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, l2tp_read_proc, NULL));
}

static ssize_t fastpath_l2tp_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return l2tp_write_proc(file, userbuf,count, off);
}


struct file_operations fastpath_l2tp_proc_fops = {
        .open           = fastpath_l2tp_single_open,
	 .write		= fastpath_l2tp_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#else
static int l2tp_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
      int len;

      len = sprintf(page, "%c\n", fast_l2tp_fw + '0');

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}
#endif

#if defined(CONFIG_PROC_FS)
static struct proc_dir_entry *res1=NULL;
#endif

int __init fast_l2tp_init(void)
{
#if defined(CONFIG_PROC_FS)
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	res1 = proc_create_data("fast_l2tp", 0, &proc_root,
			 &fastpath_l2tp_proc_fops, NULL);
	#else
	res1 = create_proc_entry("fast_l2tp", 0, NULL);
	if (res1) {
		res1->read_proc = l2tp_read_proc;
		res1->write_proc = l2tp_write_proc;
	}
	#endif
#endif

	l2tp_tx_id_hook = NULL;
	fast_l2tp_fw = 0;
	state = 0;
	memset(&l2tpInfo, 0, sizeof(l2tpInfo));
	return 0;
}

void __exit fast_l2tp_exit(void)
{
#if defined(CONFIG_PROC_FS)
	if (res1) {
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fast_l2tp", &proc_root);
		#else
		remove_proc_entry("fast_l2tp", res1);
		#endif
		res1 = NULL;
	}
#endif
}
#endif
#if 0
#ifdef CONFIG_FAST_PATH_MODULE
EXPORT_SYMBOL(fast_l2tp_init);
EXPORT_SYMBOL(fast_l2tp_exit);
EXPORT_SYMBOL(l2tp_tx_id);
EXPORT_SYMBOL(fast_l2tp_to_wan);
EXPORT_SYMBOL(fast_l2tp_rx);
EXPORT_SYMBOL(l2tp_tx_id_hook);
EXPORT_SYMBOL(fast_l2tp_fw);
#endif
#endif
