#include <linux/init.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>

#include "ther_ctrl.h"

#define MODULE_NAME 	"ther_ctrl"
#define DRV_VERSION 	"0.1.0"

static struct therctl_ops *ther_ops[3] = {0};

static struct sock *nl_sk = NULL;

void therctl_limit_tp(struct therctl_ops *ops, int level)
{
	therctl_print("%s level=%d\n", ops->dev->name, level);
	ops->set_limit_tp(ops->dev, level);
}

void therctl_bandwidth(struct therctl_ops *ops, int bw)
{
	therctl_print("%s bw=%d\n", ops->dev->name, bw);
	ops->set_bandwidth(ops->dev, bw);
}

void therctl_txduty(struct therctl_ops *ops, int level)
{
	therctl_print("%s level=%d\n", ops->dev->name, level);
	ops->set_txduty(ops->dev, level);
}

void therctl_power(struct therctl_ops *ops, int low_power)
{
	therctl_print("%s low_power=%d\n", ops->dev->name, low_power);
	ops->set_power(ops->dev, low_power);
}

void therctl_setpath(struct therctl_ops *ops, int path)
{
	therctl_print("%s path=%d\n", ops->dev->name, path);
	ops->set_path(ops->dev, path);
}

void therctl_funcoff(struct therctl_ops *ops, int enable)
{
	therctl_print("%s enable=%d\n", ops->dev->name, enable);
	ops->set_funcoff(ops->dev, enable);
}

//-----------------------------------------------------------------------------------------
void sync_mib(struct therctl_ops *ops, struct ther_info_s *info)
{
	if (!netif_running(ops->dev)) {
		info->control.cur_ther = 0;
		info->control.ther_dm = 0;
		info->control.dbg = 0;

		info->control.tx_tp = 0;
		info->control.rx_tp = 0;

		info->control.running = 0;
	} else {
		ops->sync_mib(ops->dev, info);
		info->control.running = 1;
	}
}

void get_therctl_val(struct therctl_ops *ops, struct ther_info_s *info, int val)
{
	switch (val) {
	case THERGETVAL_ALLMIB:
		sync_mib(ops, info);
		break;
	}
}

int run_therctl_func(struct therctl_ops *ops, struct ther_info_s *info)
{
	int ret = 0;
	struct ther_func_s *func = &info->func;

	switch (func->id) {
	case THERCTL_GETVAL:
		get_therctl_val(ops, info, func->val);
		ret = 1;
		break;

	case THERCTL_LIMIT_TP:
		if (netif_running(ops->dev))
		therctl_limit_tp(ops, func->val);
		break;

	case THERCTL_FUNC_OFF:
		if (netif_running(ops->dev))
		therctl_funcoff(ops, func->val);
		break;

	case THERCTL_TX_DUTY:
		if (netif_running(ops->dev))
		therctl_txduty(ops, func->val);
		break;

	case THERCTL_POWER:
		if (netif_running(ops->dev))
		therctl_power(ops, func->val);
		break;

	case THERCTL_SET_PATH:
		if (netif_running(ops->dev))
		therctl_setpath(ops, func->val);
		break;

	case THERCTL_BANDWIDTH:
		if (netif_running(ops->dev))
		therctl_bandwidth(ops, func->val);
		break;
	}

	return ret;
}

void reg_therctl_ops(struct therctl_ops *ops)
{
	int idx = -1;
	if (ops->dev) {
		if (strcmp("wlan0", ops->dev->name)==0)
			idx = 0;
		else if (strcmp("wlan1", ops->dev->name)==0)
			idx = 1;
		else if (strcmp("wlan2", ops->dev->name)==0)
			idx = 2;
		else
			therctl_info("%s is not supported!\n", ops->dev->name);

		if (idx > -1) {
			ther_ops[idx] = ops;
			therctl_info("reg %s\n", ops->dev->name);
		}
	} else {
		therctl_info("no dev !\n");
	}
}
EXPORT_SYMBOL(reg_therctl_ops);

static void nl_recv_msg(struct sk_buff *skb)
{
	int ret = 0, msg_size = sizeof(struct ther_info_s) - sizeof(struct nlmsghdr);
	struct nlmsghdr *nlh = (struct nlmsghdr*)skb->data;
	struct ther_info_s *info = NULL;
	struct ther_info_s *sinfo = (struct ther_info_s *)nlh;
	struct sk_buff *skb_out = NULL;

	struct ther_func_s *func = &sinfo->func;

	skb_out = nlmsg_new(msg_size,0);
	if (!skb_out) {
		therctl_info("failed to allocate new skb\n");
		return;
	}

	nlh=nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */

	/* Fill in */
	info = (struct ther_info_s *)nlh;
	memcpy(info->cp_beg, sinfo->cp_beg, sinfo->cp_len);

	if (ther_ops[info->idx]) {
		run_therctl_func(ther_ops[info->idx], info);
	} else {
		therctl_info("%s doesn't register ther_ops!\n", info->name);
	}

	ret = nlmsg_unicast(nl_sk, skb_out, sinfo->pid);
	if (ret < 0)
		therctl_info("failed when send msg to user\n");
}

static int __init ther_control_init(void)
{
	struct netlink_kernel_cfg cfg = {
	    .input = nl_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	if(!nl_sk) {
		therctl_info("Error creating socket.\n");
		return -10;
	}

	return 0;
}

static void __exit ther_control_exit(void)
{
	netlink_kernel_release(nl_sk);
}

module_init(ther_control_init);
module_exit(ther_control_exit);
MODULE_LICENSE("GPL");
