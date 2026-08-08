// SPDX-License-Identifier: GPL-2.0
/*
 * DSA tagger for the Realtek four-byte CPU tag used by Tenda Nova MW5.
 *
 * Hardware capture on RTL8197FS + RTL8367C/R-family proves that the two
 * directions do not use the same second tag word:
 *
 *   CPU -> switch (software TX):
 *     DA | SA | 0x8899 | protocol[15:12]=9 | destination-mask[7:0] |
 *     EtherType | payload
 *
 * The Linux rtl4a reference tagger constructs this four-byte Realtek TX word
 * from the protocol nibble plus the destination port mask only.  v43.11 also
 * set bit 9 as a presumed learn-disable flag, producing 0x9202/0x9208.  The
 * MW5 hardware consumed those DMA descriptors but never accounted egress on
 * the selected RTL8367 user port.  v43.12 removes that unsupported extra bit,
 * producing 0x9002 for LAN/port 1 and 0x9008 for WAN/port 3.
 *
 *   switch -> CPU (wire RX):
 *     DA | SA | 0x8899 | 0x0400 marker | source-port[5:0] |
 *     EtherType | payload
 *
 * The first v43.8 MW5 capture is 0x8899 0x0401 for physical port 1.  Older
 * bring-up code incorrectly applied the TX protocol-nibble test to RX and
 * dropped that valid frame as "protocol 0".  Keep a protocol-9 RX compatibility
 * path for descriptor-synthesised/older images, but prefer the observed
 * switch-to-CPU 0x0400 marker layout.
 */
#include <linux/atomic.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/bits.h>
#include <linux/etherdevice.h>
#include <asm/unaligned.h>

#include "tag.h"

#define RTL4_9_NAME                     "rtl4_9"
#define RTL4_9_TAG_LEN                  4

#define RTL4_9_TX_PROTOCOL              GENMASK(15, 12)
#define RTL4_9_TX_PROTOCOL_VALUE        0x9
#define RTL4_9_TX_PORT                  GENMASK(7, 0)

#define RTL4_9_RX_MARKER                BIT(10)
#define RTL4_9_RX_MARKER_MASK           GENMASK(15, 10)
#define RTL4_9_RX_PORT                  GENMASK(5, 0)

#define RTL4_9_TRACE_LIMIT              64

static atomic_t rtl4_9_tx_frames = ATOMIC_INIT(0);
static atomic_t rtl4_9_rx_frames = ATOMIC_INIT(0);
static atomic_t rtl4_9_rx_drops = ATOMIC_INIT(0);

static bool rtl4_9_trace_first(atomic_t *counter)
{
	return atomic_inc_return(counter) <= RTL4_9_TRACE_LIMIT;
}

static bool rtl4_9_trace_drop(void)
{
	return atomic_inc_return(&rtl4_9_rx_drops) <= RTL4_9_TRACE_LIMIT;
}

static struct sk_buff *rtl4_9_tag_xmit(struct sk_buff *skb,
				       struct net_device *dev)
{
	struct dsa_port *dp = dsa_slave_to_port(dev);
	u16 encapsulated_proto;
	u32 port_mask;
	__be16 *tag16;
	u16 tag;

	if (unlikely(dp->index > 5)) {
		netdev_err(dev, "port %u is outside protocol-9 mask\n",
			   dp->index);
		return NULL;
	}

	if (unlikely(!pskb_may_pull(skb, ETH_HLEN)))
		return NULL;

	port_mask = BIT(dp->index);
	encapsulated_proto = get_unaligned_be16(skb->data + 2 * ETH_ALEN);

	/* The switch removes the four-byte CPU tag before user-port egress. */
	if (unlikely(__skb_put_padto(skb, ETH_ZLEN, false)))
		return NULL;

	skb_push(skb, RTL4_9_TAG_LEN);
	dsa_alloc_etype_header(skb, RTL4_9_TAG_LEN);
	tag16 = dsa_etype_header_pos_tx(skb);

	tag16[0] = htons(ETH_P_REALTEK);
	tag = FIELD_PREP(RTL4_9_TX_PROTOCOL, RTL4_9_TX_PROTOCOL_VALUE) |
	      FIELD_PREP(RTL4_9_TX_PORT, port_mask);
	tag16[1] = htons(tag);

	if (rtl4_9_trace_first(&rtl4_9_tx_frames))
		netdev_info(dev,
			    "rtl4_9 v43.12 TX port=%u mask=0x%x tag=0x%04x encap=0x%04x len=%u\n",
			    dp->index, port_mask, tag, encapsulated_proto,
			    skb->len);

	return skb;
}

static struct net_device *rtl4_9_find_slave(struct net_device *master,
					    u8 encoded_port)
{
	struct net_device *slave;
	u8 source_port;

	/* MW5 RX uses a direct physical source-port index. */
	slave = dsa_master_find_slave(master, 0, encoded_port);
	if (slave)
		return slave;

	/* Retain compatibility with SDK branches that expose a one-hot mask. */
	if (!encoded_port || (encoded_port & (encoded_port - 1)))
		return NULL;

	source_port = __ffs(encoded_port);
	return dsa_master_find_slave(master, 0, source_port);
}

static bool rtl4_9_decode_rx_tag(u16 tag, u8 *port, const char **layout)
{
	if ((tag & RTL4_9_RX_MARKER_MASK) == RTL4_9_RX_MARKER) {
		*port = FIELD_GET(RTL4_9_RX_PORT, tag);
		*layout = "s2c-0400";
		return true;
	}

	/* Compatibility with the v43.8 descriptor fallback and other SDKs that
	 * may expose the CPU-to-switch protocol-9 layout in both directions.
	 */
	if (FIELD_GET(RTL4_9_TX_PROTOCOL, tag) == RTL4_9_TX_PROTOCOL_VALUE) {
		*port = FIELD_GET(RTL4_9_TX_PORT, tag);
		*layout = "proto9-compat";
		return true;
	}

	return false;
}

static struct sk_buff *rtl4_9_tag_rcv(struct sk_buff *skb,
				      struct net_device *dev)
{
	struct net_device *slave;
	u16 encapsulated_proto;
	const char *layout;
	bool trace;
	__be16 *tag16;
	u16 tag;
	u8 port;

	if (unlikely(!pskb_may_pull(skb, RTL4_9_TAG_LEN))) {
		if (rtl4_9_trace_drop())
			netdev_info(dev, "rtl4_9 v43.12 RX short tag len=%u\n",
				    skb->len);
		return NULL;
	}

	tag16 = dsa_etype_header_pos_rx(skb);
	if (ntohs(tag16[0]) != ETH_P_REALTEK) {
		netdev_dbg(dev, "non-realtek ethertype 0x%04x\n",
			   ntohs(tag16[0]));
		return skb;
	}

	tag = ntohs(tag16[1]);
	if (unlikely(!rtl4_9_decode_rx_tag(tag, &port, &layout))) {
		if (rtl4_9_trace_drop())
			netdev_info(dev,
				    "rtl4_9 v43.12 RX unknown tag=0x%04x\n",
				    tag);
		return NULL;
	}

	slave = rtl4_9_find_slave(dev, port);
	if (!slave) {
		if (rtl4_9_trace_drop())
			netdev_info(dev,
				    "rtl4_9 v43.12 RX no slave layout=%s field=0x%x tag=0x%04x\n",
				    layout, port, tag);
		return NULL;
	}

	/* skb->data points at the second tag word after master eth_type_trans(). */
	encapsulated_proto = get_unaligned_be16(skb->data + 2);
	trace = rtl4_9_trace_first(&rtl4_9_rx_frames);

	skb->dev = slave;
	skb_pull_rcsum(skb, RTL4_9_TAG_LEN);
	dsa_strip_etype_header(skb, RTL4_9_TAG_LEN);
	skb->protocol = htons(encapsulated_proto);
	dsa_default_offload_fwd_mark(skb);

	if (trace)
		netdev_info(slave,
			    "rtl4_9 v43.12 RX layout=%s field=0x%x tag=0x%04x encap=0x%04x len=%u offload=%u\n",
			    layout, port, tag, encapsulated_proto, skb->len,
			    skb->offload_fwd_mark);

	return skb;
}

static const struct dsa_device_ops rtl4_9_netdev_ops = {
	.name = RTL4_9_NAME,
	.proto = DSA_TAG_PROTO_RTL4_9,
	.xmit = rtl4_9_tag_xmit,
	.rcv = rtl4_9_tag_rcv,
	.needed_headroom = RTL4_9_TAG_LEN,
};
module_dsa_tag_driver(rtl4_9_netdev_ops);

MODULE_DESCRIPTION("DSA tag driver for Realtek MW5 four-byte CPU tags");
MODULE_LICENSE("GPL");
MODULE_ALIAS_DSA_TAG_DRIVER(DSA_TAG_PROTO_RTL4_9, RTL4_9_NAME);
