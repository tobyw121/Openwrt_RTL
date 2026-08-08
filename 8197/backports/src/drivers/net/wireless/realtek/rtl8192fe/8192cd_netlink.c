/*
 *  API-compatible handling routines
 *
 *  Copyright (c) 2017 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_NETLINK_C_


#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/timer.h>
#include "8192cd.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"
#include "ieee802_mib.h"
#include "8192cd_netlink.h"


#ifdef CONFIG_RTK_COMMON_NETLINK


#define MAX_PAYLOAD 2048


/*		Wlan Manager Definition		*/

// wlan manager pid
#define NL_WLAN_MANAGER_PID			5183

// Netlink Max Message Size
#define NL_MAX_MSG_SIZE				768

// Netlink Message Type List
#define NL_DAEMON_ON_TYPE			1
#define NL_DAEMON_OFF_TYPE			2

// Wlan Manager protocal
struct nl_message {
	u32 type;
	u32 len;
	u8  content[NL_MAX_MSG_SIZE];
};


static struct sock *rtk_nl_sock = NULL;
static unsigned char *rtk_multi_ap_prefix = "rtk_multi_ap";
static int	rtk_multi_ap_user_pid		= 0;
static int	rtk_wlan_manager_user_pid	= 0;



int rtk_netlink_init(void)
{

	#if defined(__LINUX_3_10__)
	struct netlink_kernel_cfg cfg = {
		.input = rtk_netlink_rcv,
	};

	printk("======>initialize netlink[%s][%u]\n", __FUNCTION__, __LINE__);

	rtk_nl_sock		= netlink_kernel_create(&init_net, NETLINK_RTK_PROTOCOL, &cfg);
	#else
	rtk_nl_sock		= netlink_kernel_create(&init_net, NETLINK_RTK_PROTOCOL, 0, rtk_netlink_rcv, NULL, THIS_MODULE);
	#endif

	if (!rtk_nl_sock) {
		panic_printk(KERN_ERR "rtk_netlink_init: Cannot create netlink socket");
		return -ENOMEM;
	}

	return 0;
}

void rtk_netlink_exit(void)
{
	netlink_kernel_release(rtk_nl_sock);
	rtk_multi_ap_user_pid		= 0;
	rtk_wlan_manager_user_pid	= 0;
}


void rtk_netlink_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	struct nl_message *msg = NULL;

	if (skb == NULL) {
		panic_printk(KERN_INFO "%s: skb is NULL\n", __FUNCTION__);
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;

	if (0 == memcmp(NLMSG_DATA(nlh), rtk_multi_ap_prefix, strlen(rtk_multi_ap_prefix))) {
		rtk_multi_ap_user_pid = nlh->nlmsg_pid;
	}
	else{
#ifdef CONFIG_RTK_WLAN_MANAGER
		if(nlh->nlmsg_pid == NL_WLAN_MANAGER_PID) {
			msg = (struct nl_message *)NLMSG_DATA(nlh);
			if(msg->type == NL_DAEMON_ON_TYPE)
				rtk_wlan_manager_user_pid = NL_WLAN_MANAGER_PID;
			else if(msg->type == NL_DAEMON_OFF_TYPE)
				rtk_wlan_manager_user_pid = 0;

			rtw_wlan_manager_recv_msg(msg);
		}
#endif
	}
}

void rtk_netlink_send(char *data, int data_len, int to_daemon)
{
	struct nlmsghdr *nlh;
	struct sk_buff * skb = NULL;
	const char *     fn = NULL;
	int              err=0, pid=0;

	switch(to_daemon) {
		case NL_COMM_MAP:
			pid = rtk_multi_ap_user_pid;
			break;
#ifdef CONFIG_RTK_WLAN_MANAGER
		case NL_COMM_WLAN_MANAGER:
			pid = rtk_wlan_manager_user_pid;
			break;
#endif
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

	skb = alloc_skb(NLMSG_SPACE(data_len), GFP_ATOMIC);

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

#if defined(__LINUX_3_10__)
	NETLINK_CB(skb).portid = 0; //from kernel
#else
	NETLINK_CB(skb).pid = 0; //from kernel
#endif
	NETLINK_CB(skb).dst_group = 0; //unicast

	memcpy(NLMSG_DATA(nlh), data, data_len);


	if(rtk_nl_sock) {
		err = netlink_unicast(rtk_nl_sock, skb, pid, MSG_DONTWAIT);
	}
	else {
		printk("[%s %u]rtk_nl_sock is NULL\n", __FUNCTION__, __LINE__);
		fn  = "nl_sock";
		goto msg_fail_skb;
	}

	if (err < 0) {
		fn = "nlmsg_unicast";
		goto msg_fail; //nlmsg_unicast already kfree_skb
	}

	return;

msg_fail_skb:
	if (skb)
		kfree_skb(skb);

msg_fail:
	if (pid)
		panic_printk("[%s] drop netlink msg: to_daemon=%d pid=%d msglen=%d %s: err=%d\n", __FUNCTION__, to_daemon, pid, data_len, fn, err);
	return;
}

#endif
