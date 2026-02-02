/***************************************************************************
 * Linux PPP over X - Generic PPP transport layer sockets
 * Linux PPP over Ethernet (PPPoE) Socket Implementation (RFC 2516) 
 *
 * This file supplies definitions required by the PPP over Ethernet driver
 * (pppox.c).  All version information wrt this file is located in pppox.c
 *
 * License:
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */
#ifndef __LINUX_IF_PPPOX_H
#define __LINUX_IF_PPPOX_H

#include "asm-generic/int-ll64.h"
#include "linux/types.h"
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/ppp_channel.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <uapi/linux/if_pppox.h>

static inline struct pppoe_hdr *pppoe_hdr(const struct sk_buff *skb)
{
	return (struct pppoe_hdr *)skb_network_header(skb);
}

struct pppoe_opt {
	struct net_device      *dev;	  /* device associated with socket*/
	int			ifindex;  /* ifindex of device associated with socket */
	struct pppoe_addr	pa;	  /* what this socket is bound to*/
	struct sockaddr_pppox	relay;	  /* what socket data will be
					     relayed to (PPPoE relaying) */
	struct work_struct      padt_work;/* Work item for handling PADT */
};

struct meta {
	u32 sequence;
	u32 timestamp;
};

static inline struct meta *skb_meta(struct sk_buff *skb)
{
	return (struct meta *)skb->cb;
}

#define PPTP_TIME_SCALE		1024			/* milliseconds */
#define PPTP_XMIT_WIN		4			/* max xmit window */
#define PPTP_RCV_WIN		128			/* max rcv window */
#define PPTP_MIN_TIMEOUT	(PPTP_TIME_SCALE / 83)	/* 12 milliseconds */
#define PPTP_MAX_TIMEOUT	(3 * PPTP_TIME_SCALE)	/* 3 seconds */

/* When we receive a packet, we wait to see if there's an outgoing packet
   we can piggy-back the ACK off of. These parameters determine the minimum
   and maxmimum length of time we're willing to wait in order to do that.
   These have no effect unless "enableDelayedAck" is turned on. */
#define PPTP_MIN_ACK_DELAY	(PPTP_TIME_SCALE / 7)	/* 100 milliseconds */
#define PPTP_MAX_ACK_DELAY	(PPTP_TIME_SCALE / 2)	/* 500 milliseconds */

/* See RFC 2637 section 4.4 */
#define PPTP_ACK_ALPHA(x)	(((x) + 7) >> 3)	/* alpha = 0.125 */
#define PPTP_ACK_BETA(x)	(((x) + 3) >> 2)	/* beta = 0.25 */
#define PPTP_ACK_CHI(x) 	((x) << 2)	/* chi = 4 */
#define PPTP_ACK_DELTA(x) 	((x) << 1)	/* delta = 2 */

#define PPTP_SEQ_DIFF(x,y)	((s32)(x) - (s32)(y))

static inline unsigned long pptp_get_time(void)
{
	struct timespec tv;
	unsigned long t;

	ktime_get_real_ts(&tv);
	t = tv.tv_sec * PPTP_TIME_SCALE;
	t += tv.tv_nsec / (1000000000 / PPTP_TIME_SCALE);
	return t;
}

struct pptp_opt {
	struct pptp_addr src_addr;
	struct pptp_addr dst_addr;
	u32 ack_sent, ack_recv, win_ack;
	u32 seq_sent, seq_recv;
	u16 ip_id, win_sent;
	s32 sample, ato, dev, rtt, rtt_max;
	unsigned long time_sent[PPTP_XMIT_WIN];
	struct timer_list rack_timer;
	struct timer_list reorder_timer;
	struct timer_list sack_timer;
	struct mutex mtx;
	struct net_device_stats stats;
	int ppp_flags;
	u16 mtx_init;
};
#include <net/sock.h>

struct pppox_sock {
	/* struct sock must be the first member of pppox_sock */
	struct sock sk;
	struct ppp_channel chan;
	struct pppox_sock	*next;	  /* for hash table */
	union {
		struct pppoe_opt pppoe;
		struct pptp_opt  pptp;
	} proto;
	__be16			num;
};
#define pppoe_dev	proto.pppoe.dev
#define pppoe_ifindex	proto.pppoe.ifindex
#define pppoe_pa	proto.pppoe.pa
#define pppoe_relay	proto.pppoe.relay

static inline struct pppox_sock *pppox_sk(struct sock *sk)
{
	return (struct pppox_sock *)sk;
}

static inline struct sock *sk_pppox(struct pppox_sock *po)
{
	return (struct sock *)po;
}

struct module;

struct pppox_proto {
	int		(*create)(struct net *net, struct socket *sock, int kern);
	int		(*ioctl)(struct socket *sock, unsigned int cmd,
				 unsigned long arg);
	struct module	*owner;
};
extern void pptp_start_recv_ack_timer(struct pptp_opt *opt);
extern void pptp_recv_ack_timeout(unsigned long priv);
extern void pptp_start_reorder_timer(struct pptp_opt *opt);
extern void pptp_reorder_timeout(unsigned long priv);
extern void pptp_ack(struct pptp_opt *opt);
extern int register_pppox_proto(int proto_num, const struct pppox_proto *pp);
extern void unregister_pppox_proto(int proto_num);
extern void pppox_unbind_sock(struct sock *sk);/* delete ppp-channel binding */
extern int pppox_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg);
typedef void (*pptp_snt_acc_hook)(struct net *net, struct sock *sk, struct sk_buff *skb);
extern pptp_snt_acc_hook pptp_snt_acc;
typedef void (*pptp_rcv_acc_hook)(struct ppp_channel *chan, struct sk_buff *skb);
extern pptp_rcv_acc_hook pptp_rcv_acc;
/* PPPoX socket states */
enum {
    PPPOX_NONE		= 0,  /* initial state */
    PPPOX_CONNECTED	= 1,  /* connection established ==TCP_ESTABLISHED */
    PPPOX_BOUND		= 2,  /* bound to ppp device */
    PPPOX_RELAY		= 4,  /* forwarding is enabled */
    PPPOX_ZOMBIE	= 8,  /* dead, but still bound to ppp device */
    PPPOX_DEAD		= 16  /* dead, useless, please clean me up!*/
};

#endif /* !(__LINUX_IF_PPPOX_H) */
