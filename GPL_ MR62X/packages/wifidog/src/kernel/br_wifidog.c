/*! Copyright(c) 2008-2020 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     br_wifidog.c
 *\brief    used to mark pkg from portal interface
 *			and support wifidog portal in L2 layer
 *\details  
 *
 *\author   Ma Rujun
 *\version  1.0.0
 *\date     2020/10/21
 *
 *\warning  
 *
 *\history \arg 1.0.0, 2020/10/21, Ma Rujun, Create the file.
 */

/**************************************************************************************************/
/*                                      CONFIGURATIONS                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      INCLUDE_FILES                                             */
/**************************************************************************************************/

#include "br_wifidog.h"

/**************************************************************************************************/
/*                                      DEFINES                                                   */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      TYPES                                                     */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      EXTERN_PROTOTYPES                                         */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      LOCAL_PROTOTYPES                                          */
/**************************************************************************************************/

static struct proc_dir_entry *filter_dir = NULL;

static struct proc_dir_entry *portal_interface_file = NULL;

static struct net_device *local_bridge = NULL;

static int maccmp(const unsigned char *src, const unsigned char *dst);
int do_br_wifidog_set_ctl(struct sock *sk, int cmd, void __user *user, unsigned int len);
int do_br_wifidog_get_ctl(struct sock *sk, int cmd, void __user *user, int* len);




/**************************************************************************************************/
/*                                      VARIABLES                                                 */
/**************************************************************************************************/


/*
 *  redirect http code
 */
#if !PORTAL_USE_JS
const char http_redir_format[] = 
{
	"HTTP/1.0 302 Redirect to login page\n"
	"Server: Hughes Technologies Embedded Server\n"
	"Location: %s\n"
	"Connection: close\n"
	"Content-Type: text/html\n"
	"\n"
	"redirect"
	
};
#else
#define PORTAL_JS_LEN             (512)
const char http_redir_format[] = 
{
	"HTTP/1.1 200 OK\n"
	"Content-Type: text/html\n"
	"Content-Length: %d\n"
	"Connection: close\n"
	"\n"
	"%s"
};

const char http_redir_js[] =
{
    "<html>"
    "<head>"
    "</head>"
    "<body>"
    "<script type=\"text/javascript\" language=\"javascript\">window.location.href=\"%s?url=\"+window.location.href;</script>"
    "</body>"
    "</html>"
};
#endif

/* portal interface list */
static unsigned char portal_interface_list[MAX_INTERFACE_LIST*IFNAMSIZ] = {0};

/* portal authed client list */
static struct allow_entry allow_array[MAX_CLIENT] = {0};
static LIST_HEAD(allow_list);


char g_portal_url[PORTAL_URL_LEN] = {0};

static DEFINE_SPINLOCK(portal_interface_lock);
static DEFINE_SPINLOCK(br_wifidog_lock);

int                 g_portal_error      = 1;
int                 g_portal_debug_core = 0;
int 				g_filter_syn		= 1;

/* socket to add/remove mac to allow list, communicate with wifidog */
static struct nf_sockopt_ops br_wifidog_sockopts = {
	.pf			= PF_INET,
	.set_optmin	= BR_WIFIDOG_BASE_CTL,
	.set_optmax	= BR_WIFIDOG_SET_MAX+1,
	.set		= do_br_wifidog_set_ctl,
	.get_optmin	= BR_WIFIDOG_BASE_CTL,
	.get_optmax	= BR_WIFIDOG_GET_MAX+1,
	.get		= do_br_wifidog_get_ctl,
};


/**************************************************************************************************/ 
/*                                           LOCAL_FUNCTIONS                                      */ 
/**************************************************************************************************/ 

/* compare two mac, return 0 if same */
static int maccmp(const unsigned char *src, const unsigned char *dst)
{
	int index = ETH_ALEN;

	while (index--) {
		if ((*src++ - *dst++) != 0){
			return ETH_ALEN - index;
		}
	}

	return 0;
}


/* add and del auth portal client */
static int add_allow_entry(unsigned char *mac)
{
	int index = 0;

	for (index = 0; index < MAX_CLIENT; index++){
		if (allow_array[index].used == 0){
			memcpy(allow_array[index].src_mac, mac, ETH_ALEN);
			allow_array[index].used = 1;
			list_add(&allow_array[index].list, &allow_list);
			return 0;
		}
	}

	return -1;
}

static int del_allow_entry(unsigned char *mac)
{
	struct allow_entry *temp = NULL;
	struct allow_entry *n = NULL;

	list_for_each_entry_safe(temp, n, &allow_list, list) {
		if (!maccmp(temp->src_mac, mac)){
			temp->used = 0;
			list_del(&temp->list);
			return 0;
		}
	}

	return -1;
}


/**
 * @description: redirect http for portal
 * @param {struct sk_buff *skb, struct net_device *in} 
 * @return 1--success redirect 0--pass 
 */
static int httpRedirect(struct sk_buff *skb, struct net_device *in)
{
	int					DROP				= 1;
	int					ACCEPT				= 0;
	struct iphdr 	 	*p_ip_hdr			= NULL;
    struct vlan_ethhdr 	*p_vlan_eth_hdr		= NULL;
	struct ethhdr 	 	*p_eth_hdr			= NULL;
	struct tcphdr 	 	*p_tcp_hdr			= NULL;
	struct udphdr       *p_udp_hdr			= NULL;

	struct allow_entry 	*temp				= NULL;
	unsigned char 	 	*src_mac			= NULL;

    char             	*p_tcp_payload		= NULL;

    u16              	eth_len				= 0;
    u16              	ip_len				= 0;
    u16              	tcp_len				= 0;
    u16              	ip_payload_len		= 0;
    u16              	tcp_payload_len		= 0;

    u16              	eth_proto			= 0;
    u16              	ip_proto			= 0;
    u16              	tcp_source			= 0;
    u16              	tcp_dest			= 0;
    u16              	tcp_flag			= 0;
	int              	vlan_id				= 0;

	u16              	udp_source			= 0;
    u16              	udp_dest			= 0;

    u8               	tmpMac[ETH_ALEN]	= {0};
    u32              	tmp_ip				= 0;
    u16              	tmp_port			= 0;
    u32              	tmp_seq				= 0;
	u32              	tmp_tcp_payload_len = 0;

#if PORTAL_USE_JS
	u16              max_payload_len    	= 0;
	unsigned char    js_buf[PORTAL_JS_LEN*2 + 1] = {0};
    u16              js_real_len;
    u8               jsStr[PORTAL_JS_LEN] 		 = {0};
#endif

    struct sk_buff *p_new_skb = NULL;

	/* get skb eth_hdr */
	if (NULL == (p_eth_hdr = (struct ethhdr*)skb_mac_header(skb)))
    {
		PORTAL_ERROR(g_portal_error, "skbuff_ethhdr is null");
		return ACCEPT;
    }
	
	/* check allow list */
	src_mac = p_eth_hdr->h_source;
	list_for_each_entry(temp, &allow_list, list) {
		if (!maccmp(temp->src_mac, src_mac)){
			PORTAL_DEBUG(g_portal_debug_core, "Mac in allow list, allow forward, %s", src_mac);
			return ACCEPT;//allow forward
		}
	}
	
	/*analyse the pkt vlan type*/
	if (ETH_P_8021Q != ntohs(p_eth_hdr->h_proto))
	{
		eth_len   = ETH_LEN;
		eth_proto = ntohs(p_eth_hdr->h_proto);
		vlan_id   = PORTAL_DFT_VLAN;
	}
	else
	{
		PORTAL_DEBUG(g_portal_debug_core, "ethvlan pkg");
		eth_len = VLAN_ETH_LEN;
		p_vlan_eth_hdr = vlan_eth_hdr(skb);
		eth_proto    = ntohs(p_vlan_eth_hdr->h_vlan_encapsulated_proto);
		vlan_id      = PORTAL_GET_VLAN_ID(p_vlan_eth_hdr->h_vlan_TCI);
	}


	if (ETH_P_ARP == eth_proto || ETH_P_IP != eth_proto)
	{
		PORTAL_DEBUG(g_portal_debug_core, "not IP pkg, pass");
		return ACCEPT;
	}

	/* get ip header */
    if (NULL == (p_ip_hdr = (struct iphdr*)((s8*)p_eth_hdr + eth_len)))
    {
		PORTAL_ERROR(g_portal_error, "skbuff_iphdr is null");
		return ACCEPT;
    }

	ip_len = (p_ip_hdr->ihl)<<2;
    ip_proto = p_ip_hdr->protocol;

	if (IP_PROTO_TCP == ip_proto)
    {
        /* get tcp header */
        if (NULL == (p_tcp_hdr = (struct tcphdr*)((s8*)p_ip_hdr + ip_len)))
        {
			PORTAL_ERROR(g_portal_error, "skbuff_tcphdr is null");
			return 0;
        }
        tcp_source = ntohs(p_tcp_hdr->source);
        tcp_dest   = ntohs(p_tcp_hdr->dest);

		if(tcp_dest == DNS_PORT || tcp_dest == DHCP_PORT)
		{
			PORTAL_DEBUG(g_portal_debug_core, "tcp dns or dhcp pkg, pass");
			return 0;
		}
    }
	else if(IP_PROTO_UDP == ip_proto)
	{
		/* get udp header */
        if (NULL == (p_udp_hdr = (struct udphdr*)((s8*)p_ip_hdr + ip_len)))
        {
			PORTAL_ERROR(g_portal_error, "skbuff_udphdr is null");
			return ACCEPT;
        }
        udp_source = ntohs(p_udp_hdr->source);
        udp_dest   = ntohs(p_udp_hdr->dest);

		if(udp_dest == DNS_PORT || udp_dest == DHCP_PORT)
		{
			PORTAL_DEBUG(g_portal_debug_core, "udp dns or dhcp pkg, pass");
			return ACCEPT;
		}
		else
		{
			PORTAL_DEBUG(g_portal_debug_core, "udp but not dns or dhcp pkg, drop");
			return DROP;
		}
	}
	else
	{
		PORTAL_DEBUG(g_portal_debug_core, "not tcp and udp, drop");
		return DROP;
	}


	if (TCP_HTTP_PORT != tcp_dest)
    {
		PORTAL_DEBUG(g_portal_debug_core, "not HTTP pkg, drop");
		return DROP;
    }

	ip_payload_len = ntohs(p_ip_hdr->tot_len);
    tcp_len       = GET_TCP_LEN(p_tcp_hdr);
    tcp_flag      = GET_TCP_FLAG(p_tcp_hdr);
    p_tcp_payload  = (s8*)((s8*)p_ip_hdr + tcp_len);

	/* tcp syn pkg, reply */
	if ((ip_payload_len == (ip_len + tcp_len) && IS_TCP_FLAG_SYN(tcp_flag))&& g_filter_syn)
	{
		skb_push(skb,ETH_LEN);
		
		PORTAL_DEBUG(g_portal_debug_core, "come a tcp syn pkg");
		if (NULL == (p_new_skb = skb_copy(skb, GFP_ATOMIC)))
		{
			PORTAL_ERROR(g_portal_error, "skb_copy failed 1");
			return ACCEPT;
		}
		p_eth_hdr     = (struct ethhdr*)skb_mac_header(p_new_skb);
		p_ip_hdr      = (struct iphdr*)((s8*)p_eth_hdr + eth_len);
		p_tcp_hdr     = (struct tcphdr*)((s8*)p_ip_hdr + ip_len);

		skb_pull(skb,ETH_LEN);
		
		/* eth header, here no need to consider vlan */
        memcpy(tmpMac, p_eth_hdr->h_dest, ETH_ALEN);
        memcpy(p_eth_hdr->h_dest, p_eth_hdr->h_source, ETH_ALEN);
        memcpy(p_eth_hdr->h_source, tmpMac, ETH_ALEN);

        /* ip header */
        tmp_ip         = p_ip_hdr->saddr;
        p_ip_hdr->saddr = p_ip_hdr->daddr;
        p_ip_hdr->daddr = tmp_ip;
        p_ip_hdr->check = 0;
        p_ip_hdr->check = ip_fast_csum(p_ip_hdr, p_ip_hdr->ihl);
        
        /* tcp header */
        tmp_port           = p_tcp_hdr->source;
        p_tcp_hdr->source   = p_tcp_hdr->dest;
        p_tcp_hdr->dest     = tmp_port;
        p_tcp_hdr->ack      = 1;
        p_tcp_hdr->ack_seq  = htonl(ntohl(p_tcp_hdr->seq) + 1);
        p_tcp_hdr->seq      = 0; /* hard code the server seq num */
        p_tcp_hdr->check    = 0;
		p_tcp_hdr->check    = tcp_v4_check(tcp_len, p_ip_hdr->saddr, p_ip_hdr->daddr, 
            csum_partial(p_tcp_hdr, tcp_len, 0));

		
        /* send the modified pkt */
		p_new_skb->dev  = in;
        if (0 > dev_queue_xmit(p_new_skb))
        {
			PORTAL_ERROR(g_portal_error, "send tcp syn pkg error");
			return ACCEPT;
        }
		PORTAL_DEBUG(g_portal_debug_core, "reply a tcp syn+ack pkg");
		return DROP;
	}
	else if ((ip_payload_len > (ip_len + tcp_len))) /* TCP pkg, not syn, redirect */
    {
		skb_push(skb,ETH_LEN);
		PORTAL_DEBUG(g_portal_debug_core, "come a redir http");
		if (NULL == (p_new_skb = skb_copy(skb, GFP_ATOMIC)))
		{
			PORTAL_ERROR(g_portal_error, "skb_copy failed 1");
			return ACCEPT;
		}
		p_eth_hdr     = (struct ethhdr*)skb_mac_header(p_new_skb);
		p_ip_hdr      = (struct iphdr*)((s8*)p_eth_hdr + eth_len);
		p_tcp_hdr     = (struct tcphdr*)((s8*)p_ip_hdr + ip_len);
		p_tcp_payload = (s8*)((s8*)p_tcp_hdr + tcp_len);
		skb_pull(skb,ETH_LEN);
		
		tmp_tcp_payload_len = ip_payload_len - ip_len - tcp_len;

		/* get local bridge ip as portal url */
		sprintf(g_portal_url, HTTP_CONTENT, NIPQUAD(local_bridge->ip_ptr->ifa_list->ifa_address));

#if !PORTAL_USE_JS
        memset(p_tcp_payload, 0, tmp_tcp_payload_len);
     
        snprintf(p_tcp_payload, tmp_tcp_payload_len - 1, http_redir_format, 
            g_portal_url);
		tcp_payload_len = strlen(p_tcp_payload);
		p_new_skb->tail -= tmp_tcp_payload_len - tcp_payload_len;
		p_new_skb->len  -= tmp_tcp_payload_len - tcp_payload_len;
#else
        memset(js_buf, 0, sizeof(js_buf));
        memset(jsStr, 0, sizeof(jsStr));
        snprintf(jsStr, sizeof(jsStr) - 1, http_redir_js, g_portal_url);		
        js_real_len = strlen(jsStr);
        snprintf(js_buf, sizeof(js_buf) - 1, http_redir_format, js_real_len, jsStr);

		tcp_payload_len = strlen(js_buf);
		max_payload_len = tmp_tcp_payload_len + p_new_skb->end - p_new_skb->tail;


		if ((tcp_payload_len + 1) > max_payload_len)
		{
			//pkt is not big enough
			PORTAL_ERROR(g_portal_error, "pkt is not big enough");
			dev_kfree_skb(p_new_skb);
			return ACCEPT;
		}
		p_new_skb->tail = p_new_skb->tail + ip_len + tcp_len + tcp_payload_len - ip_payload_len;
		p_new_skb->len = p_new_skb->len + ip_len + tcp_len + tcp_payload_len - ip_payload_len;
		
		memcpy(p_tcp_payload, js_buf, tcp_payload_len + 1);

#endif

       /* eth header, here no need to consider vlan */
        memcpy(tmpMac, p_eth_hdr->h_dest, ETH_ALEN);
        memcpy(p_eth_hdr->h_dest, p_eth_hdr->h_source, ETH_ALEN);
        memcpy(p_eth_hdr->h_source, tmpMac, ETH_ALEN);

        /* ip header */
        tmp_ip           = p_ip_hdr->saddr;
        p_ip_hdr->saddr   = p_ip_hdr->daddr;
        p_ip_hdr->daddr   = tmp_ip;
        p_ip_hdr->tot_len = htons(ip_len + tcp_len + tcp_payload_len);
        p_ip_hdr->check   = 0;
        p_ip_hdr->check   = ip_fast_csum(p_ip_hdr, p_ip_hdr->ihl);
        
        /* tcp header */
        tmp_port           = p_tcp_hdr->source;
        tmp_seq            = p_tcp_hdr->seq;
        p_tcp_hdr->source   = p_tcp_hdr->dest;
        p_tcp_hdr->dest     = tmp_port;
        p_tcp_hdr->urg      = 0;
        p_tcp_hdr->ack      = 1;
        p_tcp_hdr->psh      = 0;
        p_tcp_hdr->rst      = 0;
        p_tcp_hdr->syn      = 0;
        p_tcp_hdr->fin      = 1;
		
        p_tcp_hdr->seq      = p_tcp_hdr->ack_seq;
        p_tcp_hdr->ack_seq  = htonl(ntohl(tmp_seq) + tmp_tcp_payload_len);
        p_tcp_hdr->check    = 0;
        p_tcp_hdr->check    = tcp_v4_check(tcp_len + tcp_payload_len, p_ip_hdr->saddr, p_ip_hdr->daddr, 
            csum_partial(p_tcp_hdr, tcp_len + tcp_payload_len, 0));
        
        /* send the modified pkt */
		p_new_skb->dev  = in;
		if (0 > dev_queue_xmit(p_new_skb))
        {
			PORTAL_ERROR(g_portal_error, "send redir http failed");
			return ACCEPT;
        }

		return DROP;
	}
	else 
	{
		PORTAL_DEBUG(g_portal_debug_core, "tcp ack pkg, drop");
		return DROP;
	}
	return ACCEPT;

}


/* proc file read and write */
static int procfile_read_str(struct file * file, char *data, size_t len, loff_t *off)
{
	
	if(!data || !off || *off > 0)
    {
        return 0;
    }
	spin_lock_bh(&portal_interface_lock);
    if(copy_to_user(data,portal_interface_list,strlen(portal_interface_list)))
    {
        return -EFAULT;
    }
	spin_unlock_bh(&portal_interface_lock);
    *off += strlen(portal_interface_list);

    return strlen(portal_interface_list);
}

static int procfile_write_str(struct file *file, const char *data, size_t len, loff_t *off)
{
    int length = 0;
    unsigned char tmp_buf[MAX_INTERFACE_LIST*IFNAMSIZ] = {'\0'};
   
    if(len > (MAX_INTERFACE_LIST*IFNAMSIZ-1))
        length = MAX_INTERFACE_LIST*IFNAMSIZ-1;
    else
        length = len;
		
    if(copy_from_user((void *)tmp_buf, data, length))
		return -EFAULT;

    spin_lock_bh(&portal_interface_lock);
	memset(portal_interface_list,0,MAX_INTERFACE_LIST*IFNAMSIZ);
    memcpy(portal_interface_list,tmp_buf,length);
    portal_interface_list[length-1]=0;
    spin_unlock_bh(&portal_interface_lock);
    
    return length;
}

static struct file_operations portal_interface_proc_ops = {
	.read    = procfile_read_str,
	.write   = procfile_write_str,
};


static int portal_interface_list_file_init(void)
{
	int ret = 0;
	memset(portal_interface_list,0,MAX_INTERFACE_LIST*IFNAMSIZ);

	filter_dir = proc_mkdir(FILTER_PROCFS_DIR, NULL);

	if( filter_dir == NULL)
    {
         ret = -ENOMEM;
         goto err_out;
    }

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,13))		
	portal_interface_file = proc_create(PORTAL_INTERFACE_LIST, NF_PROCFS_PERM, filter_dir, &portal_interface_proc_ops);
	if (NULL == portal_interface_file)
	{
		ret = -ENOMEM;
		PORTAL_ERROR(g_portal_error, "Error: Can not create /proc/%s/%s", FILTER_PROCFS_DIR, PORTAL_INTERFACE_LIST);
		goto  no_portal_interface_list;

	}

#else
	portal_interface_file = create_proc_entry(PORTAL_INTERFACE_LIST , NF_PROCFS_PERM,  filter_dir);
	if (NULL == portal_interface_file)
	{
		ret = -ENOMEM;
		PORTAL_ERROR(g_portal_error, "Error: Can not create /proc/%s/%s", FILTER_PROCFS_DIR, PORTAL_INTERFACE_LIST)
		goto  no_portal_interface_list;
	}
	portal_interface_file->proc_fops = &portal_interface_proc_ops;

#endif


    return 0;

    no_portal_interface_list:
         remove_proc_entry(FILTER_PROCFS_DIR, NULL);
    err_out:
    return ret;

}

/**************************************************************************************************/
/*                                      PUBLIC_FUNCTIONS                                          */
/**************************************************************************************************/
int do_br_wifidog_set_ctl(struct sock *sk, int cmd, void __user *user, unsigned int len)
{
	int ret = 0;
	char mac_str[ETH_ALEN * 2 + 6] = {0};
	unsigned char mac[ETH_ALEN] = {0};

	switch(cmd)
	{
		case BR_WIFIDOG_ADD_MAC:
			spin_lock_bh(&br_wifidog_lock);
			copy_from_user(mac_str, (char *)user, sizeof(mac_str) - 1);
			mac_str[ETH_ALEN * 2 + 5] = '\0';
			sscanf(mac_str, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
			if ((ret = add_allow_entry(mac)) < 0){
				spin_unlock_bh(&br_wifidog_lock);
				PORTAL_ERROR(g_portal_error, "Failed to add, allow list is full!");
				return -1;
			}
			spin_unlock_bh(&br_wifidog_lock);
			PORTAL_DEBUG(g_portal_debug_core, "add mac list: %s", mac_str);
			break;
			
		case BR_WIFIDOG_DEL_MAC:
			spin_lock_bh(&br_wifidog_lock);
			copy_from_user(mac_str, (char *)user, sizeof(mac_str) - 1);
			mac_str[ETH_ALEN * 2 + 5] = '\0';
			sscanf(mac_str, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
			if ((ret = del_allow_entry(mac)) < 0){
				spin_unlock_bh(&br_wifidog_lock);
				PORTAL_ERROR(g_portal_error, "Failed to delete, entry not found in allow list!");
				return -1;
			}
			spin_unlock_bh(&br_wifidog_lock);

			PORTAL_DEBUG(g_portal_debug_core, "remove mac list: %s", mac_str);
			break;
			
		default:
			PORTAL_ERROR(g_portal_error, "br_wifidog: recv a unknowned set ctl command");
			ret = -1;
			break;
	}
	
	return ret;
}

int do_br_wifidog_get_ctl(struct sock *sk, int cmd, void __user *user, int* len)
{
	int ret = 0;
	struct allow_entry 	*temp = NULL;

	switch(cmd)
	{
		case BR_WIFIDOG_GET_MAC:
			list_for_each_entry(temp, &allow_list, list) {
				if (!(temp->src_mac == NULL))
				{
					PORTAL_DEBUG(g_portal_debug_core, "Mac %s in allow list", temp->src_mac);
				}
			}
			break;
			
		default:
			PORTAL_ERROR(g_portal_error, "br_wifidog: recv a unknowned set ctl command");
			ret = -1;
			break;
	}
	
	return 0;
}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,1,0))
unsigned int br_prerouting_hook(const struct nf_hook_ops *hookOps, 
					struct sk_buff *skb,
	    			const struct nf_hook_state *hookState)
{
    struct net_device *in = hookState->in;
#else
unsigned int br_prerouting_hook(unsigned int hooknum,
                       struct sk_buff *skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn)(struct sk_buff *))
{
#endif

	char *list_all = NULL;
	char *dev_name = NULL;
	static unsigned char list_temp[MAX_INTERFACE_LIST*IFNAMSIZ];

	if (in == NULL || in->name == NULL)
	{
		return NF_ACCEPT;
	}
	
	/* get portal interface */
	spin_lock_bh(&portal_interface_lock);	
	memset(list_temp,0,MAX_INTERFACE_LIST*IFNAMSIZ);
	if(portal_interface_list[0] != 0)
	{
		memcpy(list_temp,portal_interface_list,strlen(portal_interface_list)+1);
	}
	spin_unlock_bh(&portal_interface_lock);	


	list_all = list_temp;


	/* check device_in is portal interface  */
    while ((dev_name = strsep(&list_all, " ")) != NULL)
	{
		if(strcmp(dev_name,in->name) == 0)
		{
			skb->mark = (skb->mark & (~WIFIDOG_INTERFACE_MASK)) ^ WIFIDOG_INTERFACE_MARK;
			PORTAL_DEBUG(g_portal_debug_core, "come br_prerouting_hook4, a portal interface pkg ,mark");
			break;
		}
	}

	return NF_ACCEPT;
}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,1,0))
unsigned int br_forward_hook(const struct nf_hook_ops *hookOps, 
					struct sk_buff *skb,
	    			const struct nf_hook_state *hookState)
{
    struct net_device *in = hookState->in;
#else
unsigned int br_forward_hook(unsigned int hooknum,
                       struct sk_buff *skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn)(struct sk_buff *))
{
#endif

	int ret = 0;

	if (in == NULL || in->name == NULL)
	{
		return NF_ACCEPT;
	}
	
	if ((skb->mark & WIFIDOG_INTERFACE_MASK) != WIFIDOG_INTERFACE_MARK){
		PORTAL_DEBUG(g_portal_debug_core, "come br_forward_hook, not mark WIFIDOG_INTERFACE_MARK");
		return NF_ACCEPT;
	}

	
	if ( (ret = httpRedirect(skb, in)) == 0)
	{
		PORTAL_DEBUG(g_portal_debug_core, "come br_forward_hook3, no need redirect");
		return NF_ACCEPT;
	}

	PORTAL_DEBUG(g_portal_debug_core, "come br_forward_hook4, Mac not in allow list, should be Auth, redirect");
	return NF_DROP;

}


/**************************************************************************************************/
/*                                      GLOBAL_FUNCTIONS                                          */
/**************************************************************************************************/



#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,1,0))
static struct nf_hook_ops brpho = {
	.hook		= br_prerouting_hook,		/* hook function */
    .hooknum	= NF_BR_PRE_ROUTING, 	/* hook in br_prerouting */
    .pf			= PF_BRIDGE,
    .priority	= NF_BR_PRI_LAST,	/* last place */
};

static struct nf_hook_ops brfho = {
	.hook		= br_forward_hook,
    .hooknum	= NF_BR_FORWARD, 
    .pf			= PF_BRIDGE,
    .priority	= NF_BR_PRI_FIRST,
};



#else
struct nf_hook_ops	brpho;
struct nf_hook_ops	brfho;
#endif


static int __init br_wifidog_init(void)
{
    int ret = 0;

    ret = portal_interface_list_file_init();
	if(ret < 0)
	{
		PORTAL_ERROR(g_portal_error, "Unable to register proc file.");
		return ret;
	}

	ret = nf_register_sockopt(&br_wifidog_sockopts);
	if (ret != 0) {
		PORTAL_ERROR(g_portal_error, "failed to register br_wifidog sockopts.");
		return ret;
	}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,1,0))


#else
	brpho.hook		= br_prerouting_hook,
    brpho.hooknum	= NF_BR_PRE_ROUTING,
    brpho.pf		= PF_BRIDGE,
    brpho.priority	= NF_BR_PRI_LAST,

	brfho.hook		= br_forward_hook,
    brfho.hooknum	= NF_BR_FORWARD,
    brfho.pf		= PF_BRIDGE,
    brfho.priority	= NF_BR_PRI_FIRST,

#endif
    
    ret = nf_register_hook(&brpho);
    if (ret < 0)
    {
        PORTAL_ERROR(g_portal_error, "Unable to register br prerouting hook.");
        return ret;
    }

    ret = nf_register_hook(&brfho);
	if (ret < 0)
	{
		PORTAL_ERROR(g_portal_error, "Unable to register br forward hook.");
		return ret;
	}


	local_bridge = dev_get_by_name(&init_net, LOCAL_BRIDGE_NAME);
	if (!local_bridge)
	{
		PORTAL_ERROR(g_portal_error, "Unable to get local_bridge.");
		return -1;
	}
	
    return ret;
}

static void __exit br_wifidog_exit(void)
{
	if (local_bridge){
		dev_put(local_bridge);
	}
    nf_unregister_hook(&brpho);
	nf_unregister_hook(&brfho);
	nf_unregister_sockopt(&br_wifidog_sockopts);
	remove_proc_entry(PORTAL_INTERFACE_LIST, filter_dir);
    remove_proc_entry(FILTER_PROCFS_DIR, NULL);

	return;
}



module_init(br_wifidog_init);
module_exit(br_wifidog_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ma Rujun<marujun@tp-link.com.cn>");
MODULE_DESCRIPTION("Module support wifidog in bridge");
