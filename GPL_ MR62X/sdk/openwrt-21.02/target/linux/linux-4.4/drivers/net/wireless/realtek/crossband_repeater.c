#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <net/rtl/rtw_crossband_repeater.h>

#define MAC_EQU(a, b) (((a)[0] == (b)[0] && (a)[1] == (b)[1] && (a)[2] == (b)[2] && (a)[3] == (b)[3] && (a)[4] == (b)[4] && (a)[5] == (b)[5]) ? 1 : 0)

struct crossband_devinfo crossband_devInfo;
int (*crossband_get_dev_status_2G)(struct net_device *dev, struct crossband_dev_status *dev_status) = NULL;
int (*crossband_get_dev_status_5G)(struct net_device *dev, struct crossband_dev_status *dev_status) = NULL;
CROSSBAND_CALLBACK crossband_start_xmit_2G = NULL;
CROSSBAND_CALLBACK crossband_start_xmit_5G = NULL;

int crossband_dev_register(struct net_device *dev, unsigned char *mac, int band, CROSSBAND_CALLBACK func)
{
	if(!dev || ((band != CROSSBAND_2G) && (band != CROSSBAND_5G)))
		return -1;

	if(band == CROSSBAND_2G)
	{
		crossband_devInfo.dev_2G = dev;
		memcpy(crossband_devInfo.dev_mac_2G, mac, ETH_ALEN);
		crossband_start_xmit_2G = func;
	}
	else if(band == CROSSBAND_5G)
	{
		crossband_devInfo.dev_5G = dev;
		memcpy(crossband_devInfo.dev_mac_5G, mac, ETH_ALEN);
		crossband_start_xmit_5G = func;
	}

	return 0;
}

int crossband_dev_unregister(struct net_device *dev, int band)
{
	if(!dev || ((band != CROSSBAND_2G) && (band != CROSSBAND_5G)))
		return -1;

	if(band == CROSSBAND_2G)
	{
		crossband_devInfo.dev_2G = NULL;
		memset(crossband_devInfo.dev_mac_2G, 0, ETH_ALEN);
		crossband_start_xmit_2G = NULL;
	}
	else if(band == CROSSBAND_5G)
	{
		crossband_devInfo.dev_5G = NULL;
		memset(crossband_devInfo.dev_mac_5G, 0, ETH_ALEN);
		crossband_start_xmit_5G = NULL;
	}

	return 0;
}

int crossband_check_assoc(int band)
{
	struct crossband_dev_status dev_status;
	int ret = -1;
	int crossband_assoc = 0;

	if((band != CROSSBAND_2G) && (band != CROSSBAND_5G))
		return 0;

	if(band == CROSSBAND_2G)
	{
		if(crossband_devInfo.dev_5G && crossband_get_dev_status_5G)
			ret = crossband_get_dev_status_5G(crossband_devInfo.dev_5G, &dev_status);
	}
	if(band == CROSSBAND_5G)
	{
		if(crossband_devInfo.dev_2G && crossband_get_dev_status_2G)
			ret = crossband_get_dev_status_2G(crossband_devInfo.dev_2G, &dev_status);
	}

	if(ret == 0)
	{
		if(dev_status.crossband_assoc)
			crossband_assoc = 1;
	}

	return crossband_assoc;
}

int crossband_check_prefer(int band)
{
	struct crossband_dev_status dev_status;
	int ret = -1;
	int crossband_prefer = 0;

	if((band != CROSSBAND_2G) && (band != CROSSBAND_5G))
		return 0;

	if(band == CROSSBAND_2G)
	{
		if(crossband_devInfo.dev_5G && crossband_get_dev_status_5G)
			ret = crossband_get_dev_status_5G(crossband_devInfo.dev_5G, &dev_status);
	}
	if(band == CROSSBAND_5G)
	{
		if(crossband_devInfo.dev_2G && crossband_get_dev_status_2G)
			ret = crossband_get_dev_status_2G(crossband_devInfo.dev_2G, &dev_status);
	}

	if(ret == 0)
	{
		if(dev_status.crossband_prefer)
			crossband_prefer = 1;
	}

	return crossband_prefer;
}

int crossband_dev_xmit(struct sk_buff *skb, int band)
{
	if(!skb || ((band != CROSSBAND_2G) && (band != CROSSBAND_5G)))
		return -1;

	if(band == CROSSBAND_2G)
	{
		if(crossband_devInfo.dev_5G && netif_running(crossband_devInfo.dev_5G))
		{
			skb->dev = crossband_devInfo.dev_5G;
			crossband_start_xmit_5G(skb, crossband_devInfo.dev_5G);
#if 0
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
			crossband_devInfo.dev_5G->hard_start_xmit(skb, crossband_devInfo.dev_5G);
#else
			crossband_devInfo.dev_5G->netdev_ops->ndo_start_xmit(skb, crossband_devInfo.dev_5G);
#endif
#endif
			return 0;
		}
	}
	else if(band == CROSSBAND_5G)
	{
		if(crossband_devInfo.dev_2G && netif_running(crossband_devInfo.dev_2G))
		{
			skb->dev = crossband_devInfo.dev_2G;
			crossband_start_xmit_2G(skb, crossband_devInfo.dev_2G);
#if 0
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
			crossband_devInfo.dev_2G->hard_start_xmit(skb, crossband_devInfo.dev_2G);
#else
			crossband_devInfo.dev_2G->netdev_ops->ndo_start_xmit(skb, crossband_devInfo.dev_2G);
#endif
#endif
			return 0;
		}
	}

	return -1;
}

int crossband_check_loopback(unsigned char *src_mac, int band)
{
	if(!src_mac || ((band != CROSSBAND_2G) && (band != CROSSBAND_5G)))
		return 0;

	if(band == CROSSBAND_2G)
	{
		if(crossband_devInfo.dev_5G)
		{
			if(MAC_EQU(src_mac, crossband_devInfo.dev_mac_5G))
				return 1;
		}
	}
	else if(band == CROSSBAND_5G)
	{
		if(crossband_devInfo.dev_2G)
		{
			if(MAC_EQU(src_mac, crossband_devInfo.dev_mac_2G))
				return 1;
		}
	}

	return 0;
}

EXPORT_SYMBOL(crossband_get_dev_status_2G);
EXPORT_SYMBOL(crossband_get_dev_status_5G);
EXPORT_SYMBOL(crossband_dev_register);
EXPORT_SYMBOL(crossband_dev_unregister);
EXPORT_SYMBOL(crossband_check_assoc);
EXPORT_SYMBOL(crossband_check_prefer);
EXPORT_SYMBOL(crossband_dev_xmit);
EXPORT_SYMBOL(crossband_check_loopback);


static int rtk_write_proc_crossband_repeater(struct file *file, const char __user *buffer,
                                size_t count, loff_t *data)
{
	return count;
}
								
static int rtk_read_proc_crossband_repeater(struct seq_file *m, void *v)
{
	seq_printf(m, "2G dev: %s, mac: %02x:%02x:%02x:%02x:%02x:%02x\n", crossband_devInfo.dev_2G->name, 
		crossband_devInfo.dev_mac_2G[0], crossband_devInfo.dev_mac_2G[1], crossband_devInfo.dev_mac_2G[2], 
		crossband_devInfo.dev_mac_2G[3], crossband_devInfo.dev_mac_2G[4], crossband_devInfo.dev_mac_2G[5]);
	seq_printf(m, "5G dev: %s, mac: %02x:%02x:%02x:%02x:%02x:%02x\n", crossband_devInfo.dev_5G->name, 
		crossband_devInfo.dev_mac_5G[0], crossband_devInfo.dev_mac_5G[1], crossband_devInfo.dev_mac_5G[2], 
		crossband_devInfo.dev_mac_5G[3], crossband_devInfo.dev_mac_5G[4], crossband_devInfo.dev_mac_5G[5]);
	return 0;
}

static int rtk_open_proc_crossband_repeater(struct inode *inode, struct file *file)
{
	return single_open(file, rtk_read_proc_crossband_repeater, NULL);
}

static const struct file_operations crossband_repeater_proc_ops = {
	.write          = rtk_write_proc_crossband_repeater,
	.open           = rtk_open_proc_crossband_repeater,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int crossband_repeater_init(void)
{
	memset(&crossband_devInfo, 0, sizeof(struct crossband_devinfo));
	proc_create("crossband_repeater", 0, NULL, &crossband_repeater_proc_ops);
	return 0;
}

static void crossband_repeater_exit(void)
{
	return 0;
}

module_init(crossband_repeater_init);
module_exit(crossband_repeater_exit);

