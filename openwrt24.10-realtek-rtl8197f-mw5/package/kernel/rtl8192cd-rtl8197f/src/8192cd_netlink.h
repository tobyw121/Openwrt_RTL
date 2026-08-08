/*
 *  Header file for Netlink-compatible handling routines
 *
 *  Copyright (c) 2017 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
 

#ifndef _8192CD_NETLINK_H_
#define _8192CD_NETLINK_H_

#include "8192cd_cfg.h"

#ifdef CONFIG_RTK_COMMON_NETLINK

#define NETLINK_RTK_PROTOCOL 31

enum NL_COMMUNICATION_DAEMON {
	NL_COMM_MAP,
	NL_COMM_WLAN_MANAGER
};



int rtk_netlink_init(void);
void rtk_netlink_exit(void);
void rtk_netlink_rcv(struct sk_buff *skb);
void rtk_netlink_send(char *data, int data_len, int to_daemon);




#endif /* CONFIG_RTK_COMMON_NETLINK */
#endif /* _8192CD_NETLINK_H_ */

