#define _RTW_WLAN_EVENT_C_

#ifdef CONFIG_WLAN_EVENT_INDICATE_GENL
#include <net/sock.h>
#include <net/genetlink.h>
#include <drv_types.h>

/* Generic Netlink Family Name */
#define RTL_GENL_WLAN_EVENT			"WIFI6_INDICATE"
#define RTL_GENL_WLAN_EVENT_GROUP	"WIFI6_EVENT_GRP"

enum {
	RTL_WLAN_INDICATE_ATTR_UNSPEC,
	RTL_WLAN_INDICATE_ATTR_MSG,
	__RTL_WLAN_INDICATE_ATTR_MAX,
};
#define RTL_WLAN_INDICATE_ATTR_MAX (__RTL_WLAN_INDICATE_ATTR_MAX - 1)

enum {
	RTL_WLAN_INDICATE_CMD_UNSPEC,
	RTL_WLAN_INDICATE_CMD_EVENT,
	__RTL_WLAN_INDICATE_CMD_MAX,
};
#define RTL_WLAN_INDICATE_CMD_MAX (__RTL_WLAN_INDICATE_CMD_MAX - 1)

static int wlan_event_rcv(struct sk_buff *skb, struct genl_info *info);
/* genl command ops definition */
static struct genl_ops rtk_wlan_event_indicate_ops[] = {
	{
		.cmd = RTL_WLAN_INDICATE_CMD_EVENT,
		.flags = 0,
		.doit = wlan_event_rcv,
	},
};

/* genl group definition */
static struct genl_multicast_group rtk_wlan_event_indicate_grp[] = {
	{
		.name = RTL_GENL_WLAN_EVENT_GROUP,
	},
};

/* genl family definition */
static struct genl_family genl_wlan_indicate_family = {
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	.id = GENL_ID_GENERATE,
	#endif
	.hdrsize = 0,
	.name = RTL_GENL_WLAN_EVENT,
	.version = 1,
	.maxattr = RTL_WLAN_INDICATE_ATTR_MAX,
	.ops = rtk_wlan_event_indicate_ops,
	.n_ops = ARRAY_SIZE(rtk_wlan_event_indicate_ops),
	.mcgrps = rtk_wlan_event_indicate_grp,
	.n_mcgrps = ARRAY_SIZE(rtk_wlan_event_indicate_grp),
};

static int get_genl_grp_idx_by_name(struct genl_family *family, const char *name){
	int i = 0;
	for (i=0; i < family->n_mcgrps; i++){
		printk(KERN_DEBUG"group %d name = %s\n", i, (family->mcgrps+i)->name);
		if(!strcmp((family->mcgrps+i)->name, name))
			return i;
	}
	return -1;
}

static int userpid	= 1;

#ifdef MAC_ADDR_LEN
#define MACADDRLEN		MAC_ADDR_LEN
#else
#define MACADDRLEN		6
#endif /* MAC_ADDR_LEN */

#define WLAN_INFO_LEN					300

typedef struct wlan_event_entry_s
{
	int eventID;
	unsigned char mac[MACADDRLEN];
	char ifname[IFNAMSIZ];
	char reason;
	long timer;
    struct list_head list;
} wlan_event_entry_t;

wlan_event_entry_t rtw_wlan_event_entry_list;

static int wlan_event_send(int pid, int eventID, char *data, int data_len)
{
	struct sk_buff *skb;
	int rc = 0, grp_idx = -1;
	void *head = NULL;
	size_t genl_payload = 0;
	char netlink_data[sizeof(int) + (MACADDRLEN+WLAN_INFO_LEN+IFNAMSIZ+2)] = {0};
	int gfp_flag = GFP_ATOMIC;

	printk(KERN_DEBUG"Enter %s \n", __func__);

	memcpy(netlink_data, &eventID, sizeof(int));
	memcpy(netlink_data + sizeof(int), data, data_len);

	genl_payload = nla_total_size(sizeof(netlink_data)); /* total length of attribute including padding */
	printk(KERN_DEBUG"[%s] genl payload len=%zu\n", __func__, genl_payload);

	/* create a new netlink msg */
	skb = genlmsg_new(genl_payload, gfp_flag);
	if(!skb)
	{
		printk(KERN_ERR"Failed to alloc skb\n");
		return -ENOMEM;
	}

	/* Add a new netlink message to an skb */
	genlmsg_put(skb, 0, 0, &genl_wlan_indicate_family, gfp_flag, RTL_WLAN_INDICATE_CMD_EVENT);

	/* add a netlink attribute to a socket buffer */
	if ((rc = nla_put(skb, RTL_WLAN_INDICATE_ATTR_MSG, data_len+sizeof(int), netlink_data)) != 0) {
		printk(KERN_ERR"nla_put fail\n");
		goto nlmsg_fail;
	}

	head = genlmsg_data(nlmsg_data(nlmsg_hdr(skb)));
	genlmsg_end(skb, head);

#if 0 // for unicast
	printk(KERN_ERR"Sent to pid %d message\n", pid);
	rc = genlmsg_unicast(&init_net, skb, pid);
	if (rc < 0) {
		printk(KERN_ERR"Failed to unicast skb\n");
		return rc;
	}
#endif

	grp_idx = get_genl_grp_idx_by_name(&genl_wlan_indicate_family, RTL_GENL_WLAN_EVENT_GROUP);
	if (-1 == grp_idx){
		printk(KERN_ERR"get group offset Failed(%s)\n", RTL_GENL_WLAN_EVENT_GROUP);
		rc = -EINVAL;
		goto nlmsg_fail;
	}

	//printk(KERN_ERR"Sent to group (%d) message\n", grp_idx);
	rc = genlmsg_multicast(&genl_wlan_indicate_family, skb, 0, grp_idx, gfp_flag); // for first group
	if (rc < 0) {
		printk(KERN_ERR"Failed to multicast skb: ret=%d\n", rc);
		return rc;
	}

	return 0;

nlmsg_fail:
	genlmsg_cancel(skb, head);
	nlmsg_free(skb);
	return rc;
}

void rtk_eventd_genl_send(int pid, int eventID, char *ifname, char *data, int data_len)
{
	wlan_event_send(userpid, eventID, data, data_len);
}

int get_genl_eventd_pid(void)
{
    return userpid;
}

static void wlan_event_entry_list_init(void)
{
    INIT_LIST_HEAD(&rtw_wlan_event_entry_list.list);
}

static int wlan_event_rcv(struct sk_buff *skb, struct genl_info *info)
{
	struct nlmsghdr *nlh = NULL;
	struct genlmsghdr *genlhdr = NULL;
	struct nlattr *nla = NULL;
	int i = 0, nla_len = 0;

	printk(KERN_DEBUG"Enter %s\n", __func__);

	if(skb == NULL)
	{
		printk(KERN_ERR"skb is NULL\n");
		return 0;
	}
	nlh = (struct nlmsghdr *)skb->data;

	printk(KERN_DEBUG"%s:received message from pid %d: %s\n", __func__, nlh->nlmsg_pid, (char *)NLMSG_DATA(nlh));

	userpid = nlh->nlmsg_pid;

	genlhdr = nlmsg_data(nlh);
	if(genl_wlan_indicate_family.hdrsize){
		printk(KERN_ERR"[%s:%d] parser user specific header heae.\n", __func__, __LINE__);
	}

	nla = genlmsg_data(genlhdr) + genl_wlan_indicate_family.hdrsize;
	nla_len = genlmsg_len(genlhdr) - genl_wlan_indicate_family.hdrsize; 		  //len of attributes

	printk(KERN_DEBUG"%s:received message from pid %d, genl_cmd=%d\n", __func__, nlh->nlmsg_pid, genlhdr->cmd);
	for (i = 0; nla_ok(nla, nla_len); nla = nla_next(nla, &nla_len), ++i)
	{
		printk(KERN_DEBUG"%s: [%d] nla_type=%d\n", __func__, i, nla->nla_type);
	}

	return 1;
}

int rtw_genl_event_init(void)
{
	int ret = 0;

	wlan_event_entry_list_init();

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)) && !defined(CPTCFG_VERSION)
	ret = genl_register_family_with_ops_groups(&genl_wlan_indicate_family,
							rtk_wlan_event_indicate_ops, rtk_wlan_event_indicate_grp);
#else
	ret = genl_register_family(&genl_wlan_indicate_family);
#endif
	if(0 != ret){
		printk(KERN_ERR"[%s] GENL register failed (%d) !! \n", __func__, ret);
	}

	printk("%s: register GENL %s successfully\n", __func__, RTL_GENL_WLAN_EVENT);
	return 0;
}

void rtw_genl_event_deinit(void)
{
	wlan_event_entry_t *node = NULL, *tmp = NULL;
	list_for_each_entry_safe(node, tmp, &(rtw_wlan_event_entry_list.list), list)
	{
		list_del(&node->list);
		kfree(node);
	}

	genl_unregister_family(&genl_wlan_indicate_family);
	printk("%s unregister GENL %s successfully\n", __func__, RTL_GENL_WLAN_EVENT);
}
#endif /* CONFIG_WLAN_EVENT_INDICATE_GENL */

