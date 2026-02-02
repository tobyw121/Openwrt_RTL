/*
 *  Point-to-Point Tunneling Protocol for Linux
 *
 *	Authors: Dmitry Kozlov <xeb@mail.ru>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 */

#include "asm-generic/int-ll64.h"
#include "linux/gfp.h"
#include "linux/jiffies.h"
#include "linux/list.h"
#include "linux/mutex.h"
#include "linux/printk.h"
#include "linux/timer.h"
#include <linux/unaligned/be_byteshift.h>
#include "net/checksum.h"
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/skbuff.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_pppox.h>
#include <linux/ppp-ioctl.h>
#include <linux/notifier.h>
#include <linux/file.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/rcupdate.h>
#include <linux/spinlock.h>

#include <net/sock.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/gre.h>

#include <linux/uaccess.h>

#define PPTP_DRIVER_VERSION "0.8.5"

#define MAX_CALLID 65535

static DECLARE_BITMAP(callid_bitmap, MAX_CALLID + 1);
static struct pppox_sock __rcu **callid_sock;

static DEFINE_SPINLOCK(chan_lock);

static struct proto pptp_sk_proto __read_mostly;
static const struct ppp_channel_ops pptp_chan_ops;
static const struct proto_ops pptp_ops;

#define PPP_LCP_ECHOREQ 0x09
#define PPP_LCP_ECHOREP 0x0A
#define SC_RCV_BITS	(SC_RCV_B7_1|SC_RCV_B7_0|SC_RCV_ODDP|SC_RCV_EVNP)

#define MISSING_WINDOW 20
#define WRAPPED(curseq, lastseq)\
	((((curseq) & 0xffffff00) == 0) &&\
	(((lastseq) & 0xffffff00) == 0xffffff00))

#define PPTP_GRE_PROTO  0x880B
#define PPTP_GRE_VER    0x1

#define PPTP_GRE_FLAG_C	0x80
#define PPTP_GRE_FLAG_R	0x40
#define PPTP_GRE_FLAG_K	0x20
#define PPTP_GRE_FLAG_S	0x10
#define PPTP_GRE_FLAG_A	0x80

#define PPTP_GRE_IS_C(f) ((f)&PPTP_GRE_FLAG_C)
#define PPTP_GRE_IS_R(f) ((f)&PPTP_GRE_FLAG_R)
#define PPTP_GRE_IS_K(f) ((f)&PPTP_GRE_FLAG_K)
#define PPTP_GRE_IS_S(f) ((f)&PPTP_GRE_FLAG_S)
#define PPTP_GRE_IS_A(f) ((f)&PPTP_GRE_FLAG_A)

#define PPTP_HEADER_OVERHEAD (2+sizeof(struct pptp_gre_header))
struct pptp_gre_header {
	u8  flags;
	u8  ver;
	__be16 protocol;
	__be16 payload_len;
	__be16 call_id;
	__be32 seq;
	__be32 ack;
} __packed;

static struct pppox_sock *lookup_chan(u16 call_id, __be32 s_addr)
{
	struct pppox_sock *sock;
	struct pptp_opt *opt;

	rcu_read_lock();
	sock = rcu_dereference(callid_sock[call_id]);
	if (sock) {
		opt = &sock->proto.pptp;
		if (opt->dst_addr.sin_addr.s_addr != s_addr)
			sock = NULL;
		else
			sock_hold(sk_pppox(sock));
	}
	rcu_read_unlock();

	return sock;
}

static int lookup_chan_dst(u16 call_id, __be32 d_addr)
{
	struct pppox_sock *sock;
	struct pptp_opt *opt;
	int i;

	rcu_read_lock();
	i = 1;
	for_each_set_bit_from(i, callid_bitmap, MAX_CALLID) {
		sock = rcu_dereference(callid_sock[i]);
		if (!sock)
			continue;
		opt = &sock->proto.pptp;
		if (opt->dst_addr.call_id == call_id &&
			  opt->dst_addr.sin_addr.s_addr == d_addr)
			break;
	}
	rcu_read_unlock();

	return i < MAX_CALLID;
}

static int add_chan(struct pppox_sock *sock,
		    struct pptp_addr *sa)
{
	static int call_id;

	spin_lock(&chan_lock);
	if (!sa->call_id)	{
		call_id = find_next_zero_bit(callid_bitmap, MAX_CALLID, call_id + 1);
		if (call_id == MAX_CALLID) {
			call_id = find_next_zero_bit(callid_bitmap, MAX_CALLID, 1);
			if (call_id == MAX_CALLID)
				goto out_err;
		}
		sa->call_id = call_id;
	} else if (test_bit(sa->call_id, callid_bitmap)) {
		goto out_err;
	}

	sock->proto.pptp.src_addr = *sa;
	set_bit(sa->call_id, callid_bitmap);
	rcu_assign_pointer(callid_sock[sa->call_id], sock);
	spin_unlock(&chan_lock);

	return 0;

out_err:
	spin_unlock(&chan_lock);
	return -1;
}

static void del_chan(struct pppox_sock *sock)
{
	spin_lock(&chan_lock);
	clear_bit(sock->proto.pptp.src_addr.call_id, callid_bitmap);
	RCU_INIT_POINTER(callid_sock[sock->proto.pptp.src_addr.call_id], NULL);
	spin_unlock(&chan_lock);
	synchronize_rcu();
}

pptp_snt_acc_hook pptp_snt_acc = NULL;
EXPORT_SYMBOL(pptp_snt_acc);
pptp_rcv_acc_hook pptp_rcv_acc = NULL;
EXPORT_SYMBOL(pptp_rcv_acc);

/*
 * Delivers packets from the queue "q" to upper layers. Frees delivered
 * entries with the exception of one equal to "st" that is allocated
 * on caller's stack and not on the heap.
 */
static void pptp_rcvq(struct ppp_channel *chan, struct sk_buff_head *rcv_list)
{
	struct sk_buff *skb;

	while ((skb = __skb_dequeue(rcv_list))) {
		if (pptp_rcv_acc)
			pptp_rcv_acc(chan, skb);
		else
			ppp_input(chan, skb);
	}
}

static int pptp_xmit_ack(struct ppp_channel *chan)
{
	struct sock *sk = (struct sock *) chan->private;
	struct pppox_sock *po = pppox_sk(sk);
	struct net *net = sock_net(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	struct pptp_gre_header *hdr;
	unsigned int header_len = sizeof(*hdr);
	struct flowi4 fl4;
	struct rtable *rt;
	struct net_device *tdev;
	struct iphdr  *iph;
	int    max_headroom;
	struct sk_buff *skb;

	if (sk_pppox(po)->sk_state & PPPOX_DEAD)
		return 1;

	rt = ip_route_output_ports(net, &fl4, NULL,
				   opt->dst_addr.sin_addr.s_addr,
				   opt->src_addr.sin_addr.s_addr,
				   0, 0, IPPROTO_GRE,
				   RT_TOS(0), 0);
	if (IS_ERR(rt))
		return 1;

	tdev = rt->dst.dev;
	mutex_lock(&opt->mtx);
	if (opt->ack_sent == opt->seq_recv) {
		mutex_unlock(&opt->mtx);
		return 1;
	}
	max_headroom = LL_RESERVED_SPACE(tdev) + sizeof(*iph) + sizeof(*hdr) + 2;
	skb = alloc_skb(max_headroom + NET_SKB_PAD, GFP_ATOMIC);
	if (!skb) {
		mutex_unlock(&opt->mtx);
		return 1;
	}
	skb_reserve(skb, NET_SKB_PAD);
	skb->data += max_headroom;
	/* Push down and install GRE header */
	header_len -= sizeof(hdr->seq);
	skb_push(skb, header_len);
	hdr = (struct pptp_gre_header *)(skb->data);
	hdr->flags       = PPTP_GRE_FLAG_K;
	hdr->ver         = PPTP_GRE_VER;
	hdr->protocol    = htons(PPTP_GRE_PROTO);
	hdr->call_id     = htons(opt->dst_addr.call_id);
	hdr->seq         = htonl(opt->seq_recv);
	/* send ack with this message */
	hdr->ver |= PPTP_GRE_FLAG_A;
	opt->ack_sent = opt->seq_recv;
	hdr->payload_len = htons(0);
	/*	Push down and install the IP header. */

	skb_reset_transport_header(skb);
	skb_push(skb, sizeof(*iph));
	skb_reset_network_header(skb);
	iph = ip_hdr(skb);
	iph->version =	4;
	iph->ihl = sizeof(struct iphdr) >> 2;
	if (ip_dont_fragment(sk, &rt->dst))
		iph->frag_off	=	htons(IP_DF);
	else
		iph->frag_off	=	0;
	iph->protocol = IPPROTO_GRE;
	iph->tos      = 0;
	iph->daddr    = fl4.daddr;
	iph->saddr    = fl4.saddr;
	iph->ttl      = ip4_dst_hoplimit(&rt->dst);
	iph->tot_len  = htons(skb->len);

	skb_dst_drop(skb);
	skb_dst_set(skb, &rt->dst);

	nf_reset(skb);

	skb->ip_summed = CHECKSUM_NONE;
	iph->id = htons(opt->ip_id++);
	ip_send_check(iph);
	opt->stats.rx_crc_errors++;
	mutex_unlock(&opt->mtx);
	if (pptp_snt_acc)
		pptp_snt_acc(net, skb->sk, skb);
	else
		ip_local_out(net, skb->sk, skb);
	return 1;
}

/*
 * Start the send ack timer. This assumes the timer is not
 * already running.
 */
static void pptp_start_send_ack_timer(struct pptp_opt *opt)
{
	int ack_timeout, ticks;

	/* Take 1/4 of the estimated round trip time */
	ack_timeout = (opt->rtt >> 2);
	if (ack_timeout < PPTP_MIN_ACK_DELAY)
		ack_timeout = PPTP_MIN_ACK_DELAY;
	else if (ack_timeout > PPTP_MAX_ACK_DELAY)
		ack_timeout = PPTP_MAX_ACK_DELAY;

	/* Be conservative: timeout can happen up to 1 tick early */
	ticks = ack_timeout * HZ / PPTP_TIME_SCALE;
	mod_timer(&opt->sack_timer, jiffies + ticks);
}

/*
 * We've waited as long as we're willing to wait before sending an
 * acknowledgement to the peer for received frames. We had hoped to
 * be able to piggy back our acknowledgement on an outgoing data frame,
 * but apparently there haven't been any since. So send the ack now.
 */
static void pptp_send_ack_timeout(unsigned long priv)
{
	struct pptp_opt *opt = (struct pptp_opt *)priv;
	struct pppox_sock *po = container_of(opt, struct pppox_sock , proto.pptp);

	/* Send a frame with an ack but no payload */
  	pptp_xmit_ack(&po->chan);
}

void pptp_ack(struct pptp_opt *opt)
{
	if (!(timer_pending(&opt->sack_timer))) {
		/* If delayed ACK is disabled, send it now */
		if (0) {	/* ack now */
			//ng_pptpgre_xmit(hpriv, NULL);
			/* ng_pptpgre_xmit() drops the mutex */
			return;
		}
		/* ack later */
		pptp_start_send_ack_timer(opt);
		return;
	}
}

/*
 * Start a timer for the peer's acknowledging our oldest unacknowledged
 * sequence number.  If we get an ack for this sequence number before
 * the timer goes off, we cancel the timer.  Resets currently running
 * recv ack timer, if any.
 */
void pptp_start_recv_ack_timer(struct pptp_opt *opt)
{
	int remain, ticks;

	/* Compute how long until oldest unack'd packet times out,
	   and reset the timer to that time. */
	remain = (opt->time_sent[0] + opt->ato) - pptp_get_time();
	if (remain < 0)
		remain = 0;

	/* Be conservative: timeout can happen up to 1 tick early */
	ticks = remain * HZ / PPTP_TIME_SCALE + 1;
	mod_timer(&opt->rack_timer, jiffies + ticks);
}

/*
 * The peer has failed to acknowledge the oldest unacknowledged sequence
 * number within the time allotted.  Update our adaptive timeout parameters
 * and reset/restart the recv ack timer.
 */
void pptp_recv_ack_timeout(unsigned long priv)
{
	struct pptp_opt *opt = (struct pptp_opt *)priv;

	mutex_lock(&opt->mtx);
	/* Update adaptive timeout stuff */
	opt->rtt = PPTP_ACK_DELTA(opt->rtt) + 1; /* +1 to avoid delta*0 case */
	opt->ato = opt->rtt + PPTP_ACK_CHI(opt->dev);
	if (opt->ato > PPTP_MAX_TIMEOUT)
		opt->ato = PPTP_MAX_TIMEOUT;
	else if (opt->ato < PPTP_MIN_TIMEOUT)
		opt->ato = PPTP_MIN_TIMEOUT;

	/* Reset ack and sliding window */
	opt->ack_recv = opt->seq_sent;		/* pretend we got the ack */
	opt->win_sent = (opt->win_sent + 1) / 2;	/* shrink transmit window */
	opt->win_ack = opt->ack_recv + opt->win_sent;	/* reset win expand time */
	opt->stats.tx_heartbeat_errors++;
	mutex_unlock(&opt->mtx);
}


/*
 * Start a timer for the reorder queue. This assumes the timer is not
 * already running.
 */
void pptp_start_reorder_timer(struct pptp_opt *opt)
{
	int ticks;

	/* Be conservative: timeout can happen up to 1 tick early */
	ticks = ((HZ + 1000 - 1) / 1000) + 1;
	mod_timer(&opt->reorder_timer, jiffies + ticks);
}

/*
 * The oldest packet spent too much time in the reorder queue.
 * Deliver it and next packets in sequence, if any.
 */
void pptp_reorder_timeout(unsigned long priv)
{
	struct pptp_opt *opt = (struct pptp_opt *)priv;
	struct pppox_sock *po = container_of(opt, struct pppox_sock , proto.pptp);
	struct sock *sk = (struct sock *)po;
	struct sk_buff_head rcv_list;
	struct sk_buff *skb, *skb1, *last_skb;

	if (&po->proto.pptp != opt)
		panic("opt error\n");
	mutex_lock(&opt->mtx);
	if (skb_queue_empty(&sk->sk_receive_queue)) {
		mutex_unlock(&opt->mtx);
		return;
	}
	skb_queue_head_init(&rcv_list);
	skb = __skb_dequeue(&sk->sk_receive_queue);
	skb_orphan(skb);
	__skb_queue_tail(&rcv_list, skb);
	last_skb = skb;
	/* Look if we have more packets in sequence */
	skb_queue_walk_safe(&sk->sk_receive_queue, skb, skb1) {
		if (skb_meta(skb)->sequence - skb_meta(last_skb)->sequence > 1)
			break; /* the gap in the sequence */
		__skb_unlink(skb, &sk->sk_receive_queue);
		skb_orphan(skb);
		__skb_queue_tail(&rcv_list, skb);
		/* Next packet is in sequence, move it to the rcv_list. */
		last_skb = skb;
	}
	opt->seq_recv = skb_meta(last_skb)->sequence;
	if (!skb_queue_empty(&sk->sk_receive_queue))
		pptp_start_reorder_timer(opt);
	pptp_ack(opt);
	opt->stats.rx_over_errors++;
	mutex_unlock(&opt->mtx);
	/* We need to acknowledge last packet; do it soon... */
	pptp_rcvq(&po->chan, &rcv_list);
}

static int pptp_xmit(struct ppp_channel *chan, struct sk_buff *skb)
{
	struct sock *sk = (struct sock *) chan->private;
	struct pppox_sock *po = pppox_sk(sk);
	struct net *net = sock_net(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	struct pptp_gre_header *hdr;
	unsigned int header_len = sizeof(*hdr);
	struct flowi4 fl4;
	int islcp;
	int len;
	unsigned char *data;
	u32 seq_recv;
	struct rtable *rt;
	struct net_device *tdev;
	struct iphdr  *iph;
	int    max_headroom;

	if (sk_pppox(po)->sk_state & PPPOX_DEAD) {
		opt->stats.tx_aborted_errors++;
		goto tx_error;
	}

	rt = ip_route_output_ports(net, &fl4, NULL,
				   opt->dst_addr.sin_addr.s_addr,
				   opt->src_addr.sin_addr.s_addr,
				   0, 0, IPPROTO_GRE,
				   RT_TOS(0), 0);
	if (IS_ERR(rt)) {
		opt->stats.tx_dropped++;
		goto tx_error;
	}

	tdev = rt->dst.dev;

	max_headroom = LL_RESERVED_SPACE(tdev) + sizeof(*iph) + sizeof(*hdr) + 2;

	if (skb_headroom(skb) < max_headroom || skb_cloned(skb) || skb_shared(skb)) {
		struct sk_buff *new_skb = skb_realloc_headroom(skb, max_headroom);
		if (!new_skb) {
			ip_rt_put(rt);
			opt->stats.tx_dropped++;
			goto tx_error;
		}
		if (skb->sk)
			skb_set_owner_w(new_skb, skb->sk);
		consume_skb(skb);
		skb = new_skb;
	}
	mutex_lock(&opt->mtx);
	if (opt->seq_sent - opt->ack_recv >= opt->win_sent) {
		do {
			opt->ack_recv = opt->seq_sent;
			opt->win_ack = opt->ack_recv + opt->win_sent;
			break;
			opt->stats.tx_window_errors++;
			mutex_unlock(&opt->mtx);
			goto tx_error;
		} while (0);
	}
	opt->time_sent[opt->seq_sent - opt->ack_recv] = pptp_get_time();
	data = skb->data;
	islcp = ((data[0] << 8) + data[1]) == PPP_LCP && 1 <= data[2] && data[2] <= 7;

	/* compress protocol field */
	if ((opt->ppp_flags & SC_COMP_PROT) && data[0] == 0 && !islcp)
		skb_pull(skb, 1);

	/* Put in the address/control bytes if necessary */
	if ((opt->ppp_flags & SC_COMP_AC) == 0 || islcp) {
		data = skb_push(skb, 2);
		data[0] = PPP_ALLSTATIONS;
		data[1] = PPP_UI;
	}

	len = skb->len;

	seq_recv = opt->seq_recv;

	if (opt->ack_sent == seq_recv)
		header_len -= sizeof(hdr->ack);

	/* Push down and install GRE header */
	skb_push(skb, header_len);
	hdr = (struct pptp_gre_header *)(skb->data);

	hdr->flags       = PPTP_GRE_FLAG_K;
	hdr->ver         = PPTP_GRE_VER;
	hdr->protocol    = htons(PPTP_GRE_PROTO);
	hdr->call_id     = htons(opt->dst_addr.call_id);

	hdr->flags      |= PPTP_GRE_FLAG_S;
	hdr->seq         = htonl(++opt->seq_sent);
	if (opt->ack_sent != seq_recv) {
		/* send ack with this message */
		hdr->ver |= PPTP_GRE_FLAG_A;
		hdr->ack  = htonl(seq_recv);
		opt->ack_sent = seq_recv;
		del_timer(&opt->sack_timer);
	}
	hdr->payload_len = htons(len);
	/*	Push down and install the IP header. */

	skb_reset_transport_header(skb);
	skb_push(skb, sizeof(*iph));
	skb_reset_network_header(skb);
	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));
	IPCB(skb)->flags &= ~(IPSKB_XFRM_TUNNEL_SIZE | IPSKB_XFRM_TRANSFORMED | IPSKB_REROUTED);

	iph =	ip_hdr(skb);
	iph->version =	4;
	iph->ihl =	sizeof(struct iphdr) >> 2;
	if (ip_dont_fragment(sk, &rt->dst))
		iph->frag_off	=	htons(IP_DF);
	else
		iph->frag_off	=	0;
	iph->protocol = IPPROTO_GRE;
	iph->tos      = 0;
	iph->daddr    = fl4.daddr;
	iph->saddr    = fl4.saddr;
	iph->ttl      = ip4_dst_hoplimit(&rt->dst);
	iph->tot_len  = htons(skb->len);

	skb_dst_drop(skb);
	skb_dst_set(skb, &rt->dst);

	nf_reset(skb);

	skb->ip_summed = CHECKSUM_NONE;
	iph->id = htons(opt->ip_id++);
	ip_send_check(iph);
	if (opt->seq_sent == opt->ack_recv + 1)
		pptp_start_recv_ack_timer(opt);
	mutex_unlock(&opt->mtx);
	if (pptp_snt_acc)
		pptp_snt_acc(net, skb->sk, skb);
	else
		ip_local_out(net, skb->sk, skb);
	return 1;

tx_error:
	kfree_skb(skb);
	return 1;
}

int pptp_rcv_core(struct sock *sk, struct sk_buff *skb)
{
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	int headersize, payload_len;
	__u8 *payload;
	struct pptp_gre_header *header;
	u32 seq;
	struct sk_buff *skb1, *last_skb;
	s32 order;
	struct sk_buff_head rcv_list;
	s32 diff;

	mutex_lock(&opt->mtx);
	if (!(sk->sk_state & PPPOX_CONNECTED)) {
		if (sock_queue_rcv_skb(sk, skb)) {
			opt->stats.rx_fifo_errors++;
			goto drop;
		}
		mutex_unlock(&opt->mtx);
		return NET_RX_SUCCESS;
	}

	header = (struct pptp_gre_header *)(skb->data);
	headersize  = sizeof(*header);

	/* test if acknowledgement present */
	if (PPTP_GRE_IS_A(header->ver)) {
		__u32 ack;

		header = (struct pptp_gre_header *)(skb->data);

		/* ack in different place if S = 0 */
		ack = PPTP_GRE_IS_S(header->flags) ? header->ack : header->seq;

		ack = ntohl(ack);
#if defined (CONFIG_RTL_FAST_PPPOE)
		if (skb->pptp_flag & 0xffu)
			goto skip_ack_check;
#endif
		order = ack - opt->ack_recv - 1;
		do {
			if ((s32)(ack - opt->seq_sent) > 0) {
				continue;
			}
			if (ack - opt->ack_recv > PPTP_XMIT_WIN) {
				continue;
			}
			opt->ack_recv = ack;
			opt->sample = (s32)(pptp_get_time() - opt->time_sent[order]);
			diff = opt->sample - opt->rtt;
			opt->rtt += PPTP_ACK_ALPHA(diff);
			if (diff < 0)
				diff = -diff;
			opt->dev += PPTP_ACK_BETA(diff - opt->dev);
			    /* +2 to compensate low precision of int math */
			opt->ato = opt->rtt + PPTP_ACK_CHI(opt->dev + 2);
			if (opt->ato > PPTP_MAX_TIMEOUT)
				opt->ato = PPTP_MAX_TIMEOUT;
			else if (opt->ato < PPTP_MIN_TIMEOUT)
				opt->ato = PPTP_MIN_TIMEOUT;

			/* Shift packet transmit times in our transmit window */
			memmove(opt->time_sent, opt->time_sent + order + 1,
			    sizeof(*opt->time_sent) * (PPTP_XMIT_WIN - (order + 1)));

			/* If we sent an entire window, increase window size */
			if ((s32)(ack - opt->win_ack) >= 0) {
				if (opt->win_sent < PPTP_XMIT_WIN)
					opt->win_sent++;
				opt->win_ack = ack + opt->win_sent;
			}

			/* Stop/(re)start receive ACK timer as necessary */
			del_timer(&opt->rack_timer);
			if (opt->ack_recv != opt->seq_sent)
				pptp_start_recv_ack_timer(opt);
		} while (0);
	} else {
		headersize -= sizeof(header->ack);
	}
#if defined (CONFIG_RTL_FAST_PPPOE)
skip_ack_check:
#endif
	/* test if payload present */
	if (!PPTP_GRE_IS_S(header->flags))
		goto drop;

	payload_len = ntohs(header->payload_len);
	seq         = ntohl(header->seq);

	/* check for incomplete packet (length smaller than expected) */
	if (!pskb_may_pull(skb, headersize + payload_len)) {
		opt->stats.rx_length_errors++;
		goto drop;
	}

	payload = skb->data + headersize;
	skb_pull(skb, headersize);

	if (payload[0] == PPP_ALLSTATIONS && payload[1] == PPP_UI) {
		/* chop off address/control */
		if (skb->len < 3) {
			opt->stats.rx_length_errors++;
			goto drop;
		}
		skb_pull(skb, 2);
	}

	if ((*skb->data) & 1) {
		/* protocol is compressed */
		skb_push(skb, 1)[0] = 0;
	}

	skb->ip_summed = CHECKSUM_NONE;
	skb_set_network_header(skb, skb->head-skb->data);
#if defined (CONFIG_RTL_FAST_PPPOE)
	if (skb->pptp_flag & 0xffu) {
		mutex_unlock(&opt->mtx);
		ppp_input(&po->chan, skb);
		return NET_RX_SUCCESS;
	}
#endif
	skb_queue_head_init(&rcv_list);
	skb_meta(skb)->sequence = seq;
	diff = seq - opt->seq_recv;
	if (diff <= 0) {
		opt->stats.rx_missed_errors++;
		goto drop;
	}
	else if (diff == 1) {
		/* the packet came in order, place it at the start of rcv_list */
		__skb_queue_tail(&rcv_list, skb);
		opt->seq_recv = seq;
		goto deliver;
	}
	/* The packet came too early, try to enqueue it.
	 */
	skb_queue_walk(&sk->sk_receive_queue, skb1) {
		diff = (s32)(skb_meta(skb1)->sequence - seq);
		if (diff == 0) {
			opt->stats.rx_frame_errors++;
			goto drop;
		}
		if (diff > 0) {
			break;
		}
	}
	if (sk->sk_receive_queue.qlen < PPTP_RCV_WIN) {
		__skb_insert(skb, skb1->prev, skb1, &sk->sk_receive_queue);
		skb_set_owner_r(skb, sk);
	} else {
		if (skb_queue_is_first(&sk->sk_receive_queue, skb1)) {
			__skb_queue_tail(&rcv_list, skb);
			opt->seq_recv = seq;
		} else {
			last_skb = __skb_dequeue(&sk->sk_receive_queue);
			opt->seq_recv = skb_meta(last_skb)->sequence;
			__skb_queue_tail(&rcv_list, last_skb);
			__skb_insert(skb, skb1->prev, skb1, &sk->sk_receive_queue);
			skb_set_owner_r(skb, sk);
		}
	}
deliver:
	/* Look if we have some packets in sequence after sendq. */
	skb_queue_walk_safe(&sk->sk_receive_queue, skb, skb1) {
		if ((s32)(skb_meta(skb)->sequence - opt->seq_recv) > 1)
			break;
		__skb_unlink(skb, &sk->sk_receive_queue);
		opt->seq_recv = skb_meta(skb)->sequence;
		skb_orphan(skb);
		__skb_queue_tail(&rcv_list, skb);
	}
	if (skb_queue_empty(&sk->sk_receive_queue)) {
		if (timer_pending(&opt->reorder_timer))
			del_timer(&opt->reorder_timer);
	} else {
		if (!timer_pending(&opt->reorder_timer))
			pptp_start_reorder_timer(opt);
	}
	if (!skb_queue_empty(&rcv_list)) {
		pptp_ack(opt);
	}
	mutex_unlock(&opt->mtx);
	pptp_rcvq(&po->chan, &rcv_list);
	return NET_RX_SUCCESS;
	
drop:
	mutex_unlock(&opt->mtx);
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int pptp_rcv(struct sk_buff *skb)
{
	struct pppox_sock *po;
	struct pptp_gre_header *header;
	struct iphdr *iph;

	if (skb->pkt_type != PACKET_HOST)
		goto drop;

	if (!pskb_may_pull(skb, 12))
		goto drop;

	iph = ip_hdr(skb);

	header = (struct pptp_gre_header *)skb->data;

	if (ntohs(header->protocol) != PPTP_GRE_PROTO || /* PPTP-GRE protocol for PPTP */
		PPTP_GRE_IS_C(header->flags) ||                /* flag C should be clear */
		PPTP_GRE_IS_R(header->flags) ||                /* flag R should be clear */
		!PPTP_GRE_IS_K(header->flags) ||               /* flag K should be set */
		(header->flags&0xF) != 0)                      /* routing and recursion ctrl = 0 */
		/* if invalid, discard this packet */
		goto drop;

	po = lookup_chan(htons(header->call_id), iph->saddr);
	if (po) {
		skb_dst_drop(skb);
		nf_reset(skb);
		return sk_receive_skb(sk_pppox(po), skb, 0);
	}
drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int pptp_bind(struct socket *sock, struct sockaddr *uservaddr,
	int sockaddr_len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_pppox *sp = (struct sockaddr_pppox *) uservaddr;
	struct pppox_sock *po = pppox_sk(sk);
	int error = 0;

	if (sockaddr_len < sizeof(struct sockaddr_pppox))
		return -EINVAL;

	lock_sock(sk);

	if (sk->sk_state & PPPOX_DEAD) {
		error = -EALREADY;
		goto out;
	}

	if (sk->sk_state & PPPOX_BOUND) {
		error = -EBUSY;
		goto out;
	}

	if (add_chan(po, &sp->sa_addr.pptp))
		error = -EBUSY;
	else
		sk->sk_state |= PPPOX_BOUND;

out:
	release_sock(sk);
	return error;
}

static int pptp_connect(struct socket *sock, struct sockaddr *uservaddr,
	int sockaddr_len, int flags)
{
	struct sock *sk = sock->sk;
	struct sockaddr_pppox *sp = (struct sockaddr_pppox *) uservaddr;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	struct rtable *rt;
	struct flowi4 fl4;
	int error = 0;

	if (sockaddr_len < sizeof(struct sockaddr_pppox))
		return -EINVAL;

	if (sp->sa_protocol != PX_PROTO_PPTP)
		return -EINVAL;

	if (lookup_chan_dst(sp->sa_addr.pptp.call_id, sp->sa_addr.pptp.sin_addr.s_addr))
		return -EALREADY;

	lock_sock(sk);
	/* Check for already bound sockets */
	if (sk->sk_state & PPPOX_CONNECTED) {
		error = -EBUSY;
		goto end;
	}

	/* Check for already disconnected sockets, on attempts to disconnect */
	if (sk->sk_state & PPPOX_DEAD) {
		error = -EALREADY;
		goto end;
	}

	if (!opt->src_addr.sin_addr.s_addr || !sp->sa_addr.pptp.sin_addr.s_addr) {
		error = -EINVAL;
		goto end;
	}

	po->chan.private = sk;
	po->chan.ops = &pptp_chan_ops;

	rt = ip_route_output_ports(sock_net(sk), &fl4, sk,
				   opt->dst_addr.sin_addr.s_addr,
				   opt->src_addr.sin_addr.s_addr,
				   0, 0,
				   IPPROTO_GRE, RT_CONN_FLAGS(sk), 0);
	if (IS_ERR(rt)) {
		error = -EHOSTUNREACH;
		goto end;
	}
	sk_setup_caps(sk, &rt->dst);

	po->chan.mtu = dst_mtu(&rt->dst);
	if (!po->chan.mtu)
		po->chan.mtu = PPP_MRU;
	po->chan.mtu -= PPTP_HEADER_OVERHEAD;

	po->chan.hdrlen = 2 + sizeof(struct pptp_gre_header);
	error = ppp_register_channel(&po->chan);
	if (error) {
		pr_err("PPTP: failed to register PPP channel (%d)\n", error);
		goto end;
	}

	opt->dst_addr = sp->sa_addr.pptp;
	sk->sk_state |= PPPOX_CONNECTED;

 end:
	release_sock(sk);
	return error;
}

static int pptp_getname(struct socket *sock, struct sockaddr *uaddr,
	int *usockaddr_len, int peer)
{
	int len = sizeof(struct sockaddr_pppox);
	struct sockaddr_pppox sp;

	memset(&sp.sa_addr, 0, sizeof(sp.sa_addr));

	sp.sa_family    = AF_PPPOX;
	sp.sa_protocol  = PX_PROTO_PPTP;
	sp.sa_addr.pptp = pppox_sk(sock->sk)->proto.pptp.src_addr;

	memcpy(uaddr, &sp, len);

	*usockaddr_len = len;

	return 0;
}

static int pptp_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct pppox_sock *po;
	struct pptp_opt *opt;
	int error = 0;

	if (!sk)
		return 0;

	lock_sock(sk);

	if (sock_flag(sk, SOCK_DEAD)) {
		release_sock(sk);
		return -EBADF;
	}

	po = pppox_sk(sk);
	opt = &po->proto.pptp;
	opt->mtx_init = 0;
	del_chan(po);

	pppox_unbind_sock(sk);
	sk->sk_state = PPPOX_DEAD;
	skb_queue_purge(&sk->sk_receive_queue);
	printk("[%s-%u]\n", __func__, __LINE__);
	del_timer(&opt->rack_timer);
	del_timer(&opt->reorder_timer);
	del_timer(&opt->sack_timer);
	sock_orphan(sk);
	sock->sk = NULL;
	release_sock(sk);
	sock_put(sk);

	return error;
}

static void pptp_sock_destruct(struct sock *sk)
{
	if (!(sk->sk_state & PPPOX_DEAD)) {
		del_chan(pppox_sk(sk));
		pppox_unbind_sock(sk);
	}
	skb_queue_purge(&sk->sk_receive_queue);
}

static int pptp_create(struct net *net, struct socket *sock, int kern)
{
	int error = -ENOMEM;
	struct sock *sk;
	struct pppox_sock *po;
	struct pptp_opt *opt;

	sk = sk_alloc(net, PF_PPPOX, GFP_KERNEL, &pptp_sk_proto, kern);
	if (!sk)
		goto out;

	sock_init_data(sock, sk);

	sock->state = SS_UNCONNECTED;
	sock->ops   = &pptp_ops;

	sk->sk_backlog_rcv = pptp_rcv_core;
	sk->sk_state       = PPPOX_NONE;
	sk->sk_type        = SOCK_STREAM;
	sk->sk_family      = PF_PPPOX;
	sk->sk_protocol    = PX_PROTO_PPTP;
	sk->sk_destruct    = pptp_sock_destruct;

	po = pppox_sk(sk);
	opt = &po->proto.pptp;
	opt->ato = PPTP_MAX_TIMEOUT;
	opt->rtt = PPTP_TIME_SCALE / 10;
	opt->dev = 0;
	opt->win_sent = (PPTP_XMIT_WIN + 1) / 2;
	opt->win_ack = opt->win_sent;

	/* Reset sequence numbers */
	opt->seq_recv = ~(0u);
	opt->ack_recv = ~(0u);
	opt->seq_sent = ~(0u);
	opt->ack_sent = ~(0u);
	mutex_init(&opt->mtx);
	opt->mtx_init = 1;
	init_timer(&opt->rack_timer);
	opt->rack_timer.expires = jiffies;
	opt->rack_timer.data = (unsigned long)opt;
	opt->rack_timer.function = pptp_recv_ack_timeout;
	init_timer(&opt->reorder_timer);
	opt->reorder_timer.expires = jiffies;
	opt->reorder_timer.data = (unsigned long)opt;
	opt->reorder_timer.function = pptp_reorder_timeout;
	init_timer(&opt->sack_timer);
	opt->sack_timer.expires = jiffies;
	opt->sack_timer.data = (unsigned long)opt;
	opt->sack_timer.function = pptp_send_ack_timeout;
	error = 0;
out:
	return error;
}

static int pptp_ppp_ioctl(struct ppp_channel *chan, unsigned int cmd,
	unsigned long arg)
{
	struct sock *sk = (struct sock *) chan->private;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int err, val;

	err = -EFAULT;
	switch (cmd) {
	case PPPIOCGFLAGS:
		val = opt->ppp_flags;
		if (put_user(val, p))
			break;
		err = 0;
		break;
	case PPPIOCSFLAGS:
		if (get_user(val, p))
			break;
		opt->ppp_flags = val & ~SC_RCV_BITS;
		err = 0;
		break;
	default:
		err = -ENOTTY;
	}

	return err;
}

static const struct ppp_channel_ops pptp_chan_ops = {
	.start_xmit = pptp_xmit,
	.ioctl      = pptp_ppp_ioctl,
};

static struct proto pptp_sk_proto __read_mostly = {
	.name     = "PPTP",
	.owner    = THIS_MODULE,
	.obj_size = sizeof(struct pppox_sock),
};

static const struct proto_ops pptp_ops = {
	.family     = AF_PPPOX,
	.owner      = THIS_MODULE,
	.release    = pptp_release,
	.bind       = pptp_bind,
	.connect    = pptp_connect,
	.socketpair = sock_no_socketpair,
	.accept     = sock_no_accept,
	.getname    = pptp_getname,
	.poll       = sock_no_poll,
	.listen     = sock_no_listen,
	.shutdown   = sock_no_shutdown,
	.setsockopt = sock_no_setsockopt,
	.getsockopt = sock_no_getsockopt,
	.sendmsg    = sock_no_sendmsg,
	.recvmsg    = sock_no_recvmsg,
	.mmap       = sock_no_mmap,
	.ioctl      = pppox_ioctl,
};

static const struct pppox_proto pppox_pptp_proto = {
	.create = pptp_create,
	.owner  = THIS_MODULE,
};

static const struct gre_protocol gre_pptp_protocol = {
	.handler = pptp_rcv,
};

static int __init pptp_init_module(void)
{
	int err = 0;
	pr_info("PPTP driver version " PPTP_DRIVER_VERSION "\n");

	callid_sock = vzalloc((MAX_CALLID + 1) * sizeof(void *));
	if (!callid_sock)
		return -ENOMEM;

	err = gre_add_protocol(&gre_pptp_protocol, GREPROTO_PPTP);
	if (err) {
		pr_err("PPTP: can't add gre protocol\n");
		goto out_mem_free;
	}

	err = proto_register(&pptp_sk_proto, 0);
	if (err) {
		pr_err("PPTP: can't register sk_proto\n");
		goto out_gre_del_protocol;
	}

	err = register_pppox_proto(PX_PROTO_PPTP, &pppox_pptp_proto);
	if (err) {
		pr_err("PPTP: can't register pppox_proto\n");
		goto out_unregister_sk_proto;
	}

	return 0;

out_unregister_sk_proto:
	proto_unregister(&pptp_sk_proto);
out_gre_del_protocol:
	gre_del_protocol(&gre_pptp_protocol, GREPROTO_PPTP);
out_mem_free:
	vfree(callid_sock);

	return err;
}

static void __exit pptp_exit_module(void)
{
	unregister_pppox_proto(PX_PROTO_PPTP);
	proto_unregister(&pptp_sk_proto);
	gre_del_protocol(&gre_pptp_protocol, GREPROTO_PPTP);
	vfree(callid_sock);
}

module_init(pptp_init_module);
module_exit(pptp_exit_module);

MODULE_DESCRIPTION("Point-to-Point Tunneling Protocol");
MODULE_AUTHOR("D. Kozlov (xeb@mail.ru)");
MODULE_LICENSE("GPL");
