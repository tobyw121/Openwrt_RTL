// SPDX-License-Identifier: GPL-2.0
/*
 * DSA tagger for the Realtek four-byte CPU tag used by Tenda Nova MW5.
 *
 * Hardware capture on RTL8197FS + RTL8367C/R-family proves that the two
 * directions do not use the same second tag word:
 *
 *   CPU -> switch (software TX):
 *
 * v43.13 proved that 0x9002/0x9008 frames reach RTL8367 CPU port 6 and are
 * counted on the selected user-port egress after fixing CPU-port isolation,
 * but ARP still fails.  The wire RX capture from the same RTL8367 is
 * 0x8899 0x0401 for physical port 1.  v44.66.5 then proved the decisive board
 * topology detail: the only SoC-to-switch conduit is RTL8197F P0/RGMII.
 * Encoding LAN/WAN as RTL8197F descriptor DP=BIT(1)/BIT(3) completes in DMA
 * but never reaches the external switch, so layout3 cannot be the normal path.
 *
 *     tx_layout=0: 0x8899 | 0x9000 | destination-mask (production; LAN=0x9002 WAN=0x9008)
 *     tx_layout=1: 0x8899 | 0x0400 | low-bits diagnostic (RX-format A/B only)
 *     tx_layout=2: raw/no-cputag (diagnostic only; can flood LAN/WAN)
 *     tx_layout=3: software-only RTL8197F descriptor metadata (diagnostic)
 *
 * Normal v44.66.7 TX therefore preserves the 4-byte protocol-9 RTL8367 tag in the skb
 * and the master forces the six-DWORD descriptor to DP=P0.  The external
 * switch, whose CPU_CTRL is already observed in 4-byte mode, performs the
 * final user-port selection.
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
#include <linux/moduleparam.h>
#include <asm/unaligned.h>

#include "tag.h"

#define RTL4_9_NAME                     "rtl4_9"
#define RTL4_9_TAG_LEN                  4

#define RTL4_9_TX_PROTOCOL              GENMASK(15, 12)
#define RTL4_9_TX_PROTOCOL_VALUE        0x9
#define RTL4_9_TX_PORT                  GENMASK(7, 0)

#define RTL4_9_TX_LAYOUT_PROTO9         0
#define RTL4_9_TX_LAYOUT_NATIVE0400     1
#define RTL4_9_TX_LAYOUT_RAW_UNTAGGED   2
#define RTL4_9_TX_LAYOUT_DESC_META      3
#define RTL4_9_TX_META_MAGIC            0xd000
#define RTL4_9_TX_META_PORT             GENMASK(5, 0)
#define RTL4_9_TX_NATIVE_MARKER         BIT(10)
#define RTL4_9_TX_NATIVE_PORT           GENMASK(5, 0)

#define RTL4_9_RX_MARKER                BIT(10)
#define RTL4_9_RX_MARKER_MASK           GENMASK(15, 10)
#define RTL4_9_RX_PORT                  GENMASK(5, 0)

#define RTL4_9_TRACE_LIMIT              8

static atomic_t rtl4_9_tx_frames = ATOMIC_INIT(0);
static atomic_t rtl4_9_rx_frames = ATOMIC_INIT(0);
static atomic_t rtl4_9_rx_drops = ATOMIC_INIT(0);

/*
 * Keep all historical encodings available for explicit A/B diagnostics.  The
 * production invariant is the CPU-to-switch protocol-9 destination mask over
 * the physical RTL8197F P0/RGMII cascade. The observed 0x0401 word is
 * switch-to-CPU source-port metadata and must not be reused as a TX encoding.
 */
static unsigned int rtl4_9_tx_layout = RTL4_9_TX_LAYOUT_PROTO9;
module_param_named(tx_layout, rtl4_9_tx_layout, uint, 0644);
MODULE_PARM_DESC(tx_layout,
		 "MW5 TX layout: 0=protocol9 production over P0/RGMII, 1=0x0400 source-tag diagnostic, 2=raw diagnostic (unsafe for routed isolation), 3=RTL8197F descriptor-metadata diagnostic");

static bool rtl4_9_trace_first(atomic_t *counter)
{
	/* v44.57: after the small boot diagnostic window, do not execute an
	 * atomic RMW for every packet forever. Forwarding semantics are unchanged. */
	if (likely(atomic_read(counter) >= RTL4_9_TRACE_LIMIT))
		return false;
	return atomic_inc_return(counter) <= RTL4_9_TRACE_LIMIT;
}

static bool rtl4_9_trace_drop(void)
{
	if (likely(atomic_read(&rtl4_9_rx_drops) >= RTL4_9_TRACE_LIMIT))
		return false;
	return atomic_inc_return(&rtl4_9_rx_drops) <= RTL4_9_TRACE_LIMIT;
}

static struct sk_buff *rtl4_9_tag_xmit(struct sk_buff *skb,
				       struct net_device *dev)
{
	struct dsa_port *dp = dsa_slave_to_port(dev);
	u16 encapsulated_proto;
	const char *layout_name;
	unsigned int layout;
	u32 port_mask;
	__be16 *tag16;
	u16 tag;

	if (unlikely(dp->index > 5)) {
		netdev_err(dev, "port %u is outside rtl4_9 diagnostic mask\n",
			   dp->index);
		return NULL;
	}

	if (unlikely(!pskb_may_pull(skb, ETH_HLEN)))
		return NULL;

	port_mask = BIT(dp->index);
	layout = READ_ONCE(rtl4_9_tx_layout);
	if (unlikely(layout > RTL4_9_TX_LAYOUT_DESC_META)) {
		netdev_err(dev, "rtl4_9 v44.66.7 invalid tx_layout=%u\n", layout);
		return NULL;
	}

	/* A recognized CPU tag should be consumed before user-port egress. */
	if (unlikely(__skb_put_padto(skb, ETH_ZLEN, false)))
		return NULL;

	if (layout == RTL4_9_TX_LAYOUT_RAW_UNTAGGED) {
		if (rtl4_9_trace_first(&rtl4_9_tx_frames)) {
			encapsulated_proto = get_unaligned_be16(skb->data + 2 * ETH_ALEN);
			netdev_info(dev,
				    "rtl4_9 v44.66.7 TX layout=raw-untagged port=%u mask=0x%x encap=0x%04x len=%u diagnostic=no-target-metadata\n",
				    dp->index, port_mask, encapsulated_proto,
				    skb->len);
		}

		return skb;
	}

	encapsulated_proto = get_unaligned_be16(skb->data + 2 * ETH_ALEN);
	skb_push(skb, RTL4_9_TAG_LEN);
	dsa_alloc_etype_header(skb, RTL4_9_TAG_LEN);
	tag16 = dsa_etype_header_pos_tx(skb);

	/* Layout 3 is deliberately not an RTL8367 CPU tag.  It is private
	 * metadata between DSA and the RTL8197F master.  The master consumes it
	 * before DMA and maps the one-hot port to OEM descriptor DVID/DP fields.
	 */
	if (layout == RTL4_9_TX_LAYOUT_DESC_META) {
		tag16[0] = htons(ETH_P_REALTEK);
		tag = RTL4_9_TX_META_MAGIC |
		      FIELD_PREP(RTL4_9_TX_META_PORT, port_mask);
		tag16[1] = htons(tag);
		if (rtl4_9_trace_first(&rtl4_9_tx_frames))
			netdev_info(dev,
				    "rtl4_9 v44.66.7 TX layout=desc-meta port=%u mask=0x%x meta=0x%04x encap=0x%04x len=%u\n",
				    dp->index, port_mask, tag,
				    encapsulated_proto, skb->len);
		return skb;
	}

	tag16[0] = htons(ETH_P_REALTEK);
	switch (layout) {
	case RTL4_9_TX_LAYOUT_PROTO9:
		tag = FIELD_PREP(RTL4_9_TX_PROTOCOL, RTL4_9_TX_PROTOCOL_VALUE) |
		      FIELD_PREP(RTL4_9_TX_PORT, port_mask);
		layout_name = "proto9";
		break;
	case RTL4_9_TX_LAYOUT_NATIVE0400:
		tag = RTL4_9_TX_NATIVE_MARKER |
		      FIELD_PREP(RTL4_9_TX_NATIVE_PORT, port_mask);
		layout_name = "native0400";
		break;
	default:
		netdev_err(dev, "rtl4_9 v44.66.7 invalid tx_layout=%u\n", layout);
		return NULL;
	}
	tag16[1] = htons(tag);

	if (rtl4_9_trace_first(&rtl4_9_tx_frames))
		netdev_info(dev,
			    "rtl4_9 v44.66.7 TX layout=%s port=%u mask=0x%x tag=0x%04x encap=0x%04x len=%u\n",
			    layout_name, dp->index, port_mask, tag,
			    encapsulated_proto, skb->len);

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
			netdev_info(dev, "rtl4_9 v43.16 RX short tag len=%u\n",
				    skb->len);
		return NULL;
	}

	tag16 = dsa_etype_header_pos_rx(skb);
	if (ntohs(tag16[0]) != ETH_P_REALTEK) {
		/* v44.62.14: Never return an unclassified frame unchanged from a
		 * DSA receive tagger. eth_type_trans() forces ETH_P_XDSA on every
		 * DSA master, so dsa_switch_rcv() would push the Ethernet header,
		 * call eth_type_trans() again with skb->dev still pointing at the
		 * master, and feed the same skb back into dsa_switch_rcv(). One
		 * untagged/SPA0 frame can therefore consume the backlog in repeated
		 * 64-packet batches and starve DHCP, DNS and routed forwarding.
		 *
		 * Valid MW5 switch ingress is either an observed 0x8899 wire tag or
		 * a descriptor-synthesised 0x8899 tag for proven ports 1/3. Frames
		 * which reach this point have no safe DSA slave identity and must be
		 * dropped. Returning NULL makes the DSA core free the skb.
		 */
		if (rtl4_9_trace_drop())
			netdev_info(dev,
				    "rtl4_9 v44.62.14 RX drop untagged master frame ethertype=0x%04x (prevents XDSA reinjection loop)\n",
				    ntohs(tag16[0]));
		return NULL;
	}

	tag = ntohs(tag16[1]);
	if (unlikely(!rtl4_9_decode_rx_tag(tag, &port, &layout))) {
		if (rtl4_9_trace_drop())
			netdev_info(dev,
				    "rtl4_9 v43.16 RX unknown tag=0x%04x\n",
				    tag);
		return NULL;
	}

	slave = rtl4_9_find_slave(dev, port);
	if (!slave) {
		if (rtl4_9_trace_drop())
			netdev_info(dev,
				    "rtl4_9 v43.16 RX no slave layout=%s field=0x%x tag=0x%04x\n",
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
	/* v44.65.6: P1/P3 are deliberately isolated to CPU6 on MW5. The
	 * external switch therefore does not autonomously bridge LAN traffic;
	 * Linux br-lan/firewall owns flooding and forwarding. Never advertise a
	 * hardware-forwarded copy here, even if a future DSA bridge callback
	 * happens to populate dp->bridge. */
	skb->offload_fwd_mark = 0;

	if (trace)
		netdev_info(slave,
			    "rtl4_9 v43.16 RX layout=%s field=0x%x tag=0x%04x encap=0x%04x len=%u offload=%u\n",
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
