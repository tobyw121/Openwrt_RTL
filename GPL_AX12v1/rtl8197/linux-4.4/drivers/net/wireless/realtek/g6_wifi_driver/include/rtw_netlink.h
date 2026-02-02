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

#ifndef _RTW_NETLINK_H_
#define _RTW_NETLINK_H_

#ifdef CONFIG_RTW_COMMON_NETLINK

#define NETLINK_RTW_PROTOCOL 27
enum NL_COMMUNICATION_DAEMON {
	NL_COMM_MAP,
	NL_COMM_WLAN_MANAGER
};



int rtw_netlink_init_dev(void);
void rtw_netlink_exit_dev(void);
void rtw_netlink_rcv(struct sk_buff *skb);
void rtw_netlink_send(char *data, int data_len, int to_daemon);


#endif /* CONFIG_RTW_COMMON_NETLINK */

#endif /* _RTW_NETLINK_H_ */
