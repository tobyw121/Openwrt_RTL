/******************************************************************************
 *
 * Copyright(c) 2007 - 2020 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _RTW_NETLINK_C_

#include <drv_types.h>



#ifdef CONFIG_RTW_COMMON_NETLINK

#define MAX_PAYLOAD 2048

/* Wlan Manager Definition */

// wlan manager pid
#define NL_WLAN_MANAGER_PID			5183

// Netlink Max Message Size
#define NL_MAX_MSG_SIZE				768

// Netlink Message Type List
#define NL_DAEMON_ON_TYPE			1
#define NL_DAEMON_OFF_TYPE			2



static struct sock *rtk_nl_sock = NULL;
static unsigned char *rtk_multi_ap_prefix = "rtk_multi_ap";
static int	rtk_multi_ap_user_pid		= 0;
static int	rtk_wlan_manager_user_pid	= 0;


int rtw_netlink_init_dev(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = rtw_netlink_rcv,
	};
	RTW_INFO("======>initialize netlink[%s][%u]\n", __FUNCTION__, __LINE__);

	rtk_nl_sock = netlink_kernel_create(&init_net, NETLINK_RTW_PROTOCOL, &cfg);

	if (!rtk_nl_sock) {
		RTW_ERR("%s: Cannot create netlink socket", __FUNCTION__);
		return -ENOMEM;
	}

	return 0;
}

void rtw_netlink_exit_dev(void)
{
	if(rtk_nl_sock)
	{
		netlink_kernel_release(rtk_nl_sock);
		rtk_nl_sock = NULL;
	}
	printk("[rtw_netlink_exit_dev] delete rtk_nl_sock netlink succeed.\n");
	rtk_multi_ap_user_pid		= 0;
	rtk_wlan_manager_user_pid	= 0;
}



void rtw_netlink_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	struct nl_message *msg = NULL;

	if (skb == NULL) {
		RTW_ERR("%s: skb is NULL\n", __FUNCTION__);
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;

	if (0 == memcmp(NLMSG_DATA(nlh), rtk_multi_ap_prefix, strlen(rtk_multi_ap_prefix))) {
		rtk_multi_ap_user_pid = nlh->nlmsg_pid;
	}
	else{
		if(nlh->nlmsg_pid == NL_WLAN_MANAGER_PID) {
			msg = (struct nl_message *)NLMSG_DATA(nlh);

			if(msg->type == NL_DAEMON_ON_TYPE)
				rtk_wlan_manager_user_pid = NL_WLAN_MANAGER_PID;
			else if(msg->type == NL_DAEMON_OFF_TYPE)
				rtk_wlan_manager_user_pid = 0;
			else{

			}
			rtw_wlan_manager_recv_msg(msg);
		}
	}
}

void rtw_netlink_send(char *data, int data_len, int to_daemon)
{
	struct nlmsghdr *nlh;
	struct sk_buff  *skb = NULL;
	unsigned int     skblen;
	const char *     fn = NULL;
	int              err, pid;

	switch(to_daemon) {
		case NL_COMM_MAP:
			pid = rtk_multi_ap_user_pid;
			break;
		case NL_COMM_WLAN_MANAGER:
			pid = rtk_wlan_manager_user_pid;
			break;
		default:
			pid = 0;
			break;
	}

	if(!pid){
		err = -ENOBUFS;
		fn	= "pid_err";
		goto msg_fail;
	}

	if (data_len > MAX_PAYLOAD) {
		err = -ENOBUFS;
		fn  = "data_len";
		goto msg_fail;
	}

	skb    = alloc_skb(NLMSG_SPACE(data_len), GFP_ATOMIC);

	if (!skb) {
		err = -ENOBUFS;
		fn  = "alloc_skb";
		goto msg_fail;
	}

	nlh = nlmsg_put(skb, 0, 0, 0, data_len, 0);

	if (!nlh) {
		err = -ENOBUFS;
		fn  = "nlmsg_put";
		goto msg_fail_skb;
	}

	NETLINK_CB(skb).portid = 0; //from kernel
	NETLINK_CB(skb).dst_group = 0; //unicast

	memcpy(NLMSG_DATA(nlh), data, data_len);


	if(rtk_nl_sock) {
		err = netlink_unicast(rtk_nl_sock, skb, pid, MSG_DONTWAIT);
	}
	else {
		printk("[%s %u]rtk_nl_sock is NULL\n", __FUNCTION__, __LINE__);
		fn = "nl_sock";
		goto msg_fail_skb;
	}

	if (err < 0) {
		fn = "nlmsg_unicast";
		goto msg_fail; //nlmsg_unicast already kfree_skb
	}

	return;

msg_fail_skb:
	if(skb)
		kfree_skb(skb);

msg_fail:
	if (pid)
		RTW_ERR("[%s] drop netlink msg: pid=%d msglen=%d %s: err=%d\n", __FUNCTION__, pid, data_len, fn, err);
	return;
}


#endif /* CONFIG_RTW_COMMON_NETLINK */

