/*
 * Copyright(c) 2018 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#include <net/dst.h>			   /* for skb_dst_drop */
#include <net/xfrm.h>			   /* for secpath_reset */

#include "rtw_hwsim.h"
#include "rtw_hwsim_intf.h"

void rtw_hwsim_build_metadata(struct rtw_hwsim_tx_metadata *m,
                              const struct xmit_frame *x, size_t buflen)
{
	static u16 seq = 0;

	/* build metadata */
	m->hdr_len = cpu_to_le16(x->attrib.hdrlen);
	m->pkt_len = cpu_to_le16(x->attrib.pktlen);
	m->buf_len = cpu_to_le16((u16)buflen);
	m->mid = cpu_to_le16(sim_machine_id);
	m->seq = cpu_to_le16(seq++);
	m->icv_len = x->attrib.icv_len;
	m->qos_en = x->attrib.qos_en;
	m->priority = x->attrib.priority;
	m->rate = x->attrib.rate;
	m->encrypt = x->attrib.encrypt;
	m->nr_frags = x->attrib.nr_frags;
}

void rtw_hwsim_rx_translate(struct _ADAPTER *adapter, struct rtw_hwsim_frame *f,
                            union recv_frame *r)
{
	struct rtw_hwsim_tx_metadata *m;
	struct rx_pkt_attrib *a;

	m = &f->m;

	/* init recv_frame */
	_rtw_init_listhead(&r->u.hdr.list);
	r->u.hdr.len = 0;
	a = &r->u.hdr.attrib;
	_rtw_memset(a, 0, sizeof(*a));

	/* fill sec related attrib, iv_len and icv_len will be filled by
	 * validate_recv_data_frame() */
	a->crc_err = 0;
	a->icv_err = 0;
	a->encrypt = m->encrypt;

	/* fill rx pkt attrib */
	a->hdrlen = f->m.hdr_len;
	a->bw = CHANNEL_WIDTH_MAX;
	a->pkt_len = le16_to_cpu(f->m.buf_len);
	a->pkt_rpt_type = NORMAL_RX;
	a->drvinfo_sz = 0;
	a->bdecrypted = (_NO_PRIVACY_ < m->encrypt) ? 1 : 0;
	a->qos = f->m.qos_en;
	a->priority = f->m.priority;
	a->amsdu = 0;
	a->mdata = 0;
	a->mfrag = 0;
	a->seq_num = 0;
	a->frag_num = 0;
	a->data_rate = DESC_RATE6M;
	a->ppdu_cnt = 1;
	a->free_cnt = 0;
	a->data_rate = m->rate;
}

int rtw_hwsim_medium_init(struct rtw_hwsim_vif *vif, int medium)
{
	struct rtw_hwsim_medium *med;

	med = vif_to_med(vif);
	med->medium = medium;
	med->vif = vif;

	if (medium == RTW_HWSIM_MEDIUM_LOCAL)
		rtw_hwsim_medium_local_ops(med);
	else if (medium == RTW_HWSIM_MEDIUM_SOCKET_UDP)
		rtw_hwsim_medium_sock_udp_ops(med);
	else if (medium == RTW_HWSIM_MEDIUM_LOOPBACK)
		rtw_hwsim_medium_loopback_ops(med);
	else
		return -1;

	if (med->ops->init(med) < 0)
		return -1;

	return 0;
}

void rtw_hwsim_medium_deinit(struct rtw_hwsim_vif *vif)
{
	struct rtw_hwsim_medium *med;

	med = vif_to_med(vif);

	med->ops->deinit(med);
}

netdev_tx_t rtw_hwsim_medium_tx_handler(struct rtw_hwsim_vif *vif,
                                        struct sk_buff *skb)
{
	struct rtw_hwsim_medium *med;

	med = vif_to_med(vif);

	if (med->ops->tx_handler(med, skb) < 0) {
		RTW_ERR("%s(): medium tx handler failed\n", __func__);
		return NETDEV_TX_BUSY;
	}

	return NETDEV_TX_OK;
}

int rtw_hwsim_medium_tx(struct _ADAPTER *adapter, const void *tx_ctx,
                        u8 *buf, size_t buflen)
{
	struct rtw_hwsim_vif *vif;
	struct rtw_hwsim_medium *med;

	vif = adap_to_vif(adapter);
	med = vif_to_med(vif);

	if (med->ops->medium_tx(med, tx_ctx, buf, buflen) < 0) {
		RTW_ERR("%s(): medium tx failed\n", __func__);
		return -1;
	}

	return 0;
}

void rtw_hwsim_medium_pre_netif_rx(struct sk_buff *skb)
{
	struct rtw_hwsim_vif *vif;
	struct rtw_hwsim_medium *med;

	vif = ndev_to_vif(skb->dev);
	med = vif_to_med(vif);

	if (med->ops->pre_netif_rx)
		med->ops->pre_netif_rx(med, skb);
}

int rtw_hwsim_tweak_ip_addr(struct sk_buff *skb)
{
	u8 *buf;
	struct ethhdr *eh;
	u16 prot;

	buf = skb->data;
	eh = (struct ethhdr *)buf;
	prot = ntohs(eh->h_proto);

	if (prot == ETH_P_ARP) {
#ifndef CONFIG_NOARP
		struct arphdr *ah;
		u32 *saddr, *daddr;
		u8 *arp_ptr;

		ah = (struct arphdr *)(buf + sizeof(struct ethhdr));
		arp_ptr = (u8 *)(ah + 1);

		saddr = (u32 *)(arp_ptr + ETH_ALEN);
		daddr = (u32 *)(arp_ptr + ETH_ALEN + 4 + ETH_ALEN);

		((u8 *)saddr)[2] ^= 1;
		((u8 *)daddr)[2] ^= 1;
#endif
	} else if (prot == ETH_P_IP) {
		struct iphdr *ih;
#ifdef CONFIG_NOARP
		struct rtw_hwsim_vif *vif;
#endif
		u32 *saddr, *daddr;

#ifdef CONFIG_NOARP
		vif = netdev_priv(d_dev);
		memcpy(eh->h_dest, vif->data->addresses[0].addr, ETH_ALEN);
#endif

		/*
		 * Ethhdr is 14 bytes, but the kernel arranges for iphdr
		 * to be aligned (i.e., ethhdr is unaligned)
		 */
		ih = (struct iphdr *)(buf + sizeof(struct ethhdr));
		saddr = &ih->saddr;
		daddr = &ih->daddr;

		((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
		((u8 *)daddr)[2] ^= 1;

		ih->check = 0; /* and rebuild the checksum (ip needs it) */
		ih->check = ip_fast_csum((unsigned char *)ih, ih->ihl);
	} else {
		return -1;
	}

	return 0;
	}

u32 rtw_hwsim_crc32(const u8 *in, size_t byte_num)
{
	BOOLEAN a, b;
	u8 mask, smask = 0x01;
	u32 CRCMask = 0x00000001,
		POLY = 0xEDB88320,
		CRC = 0xffffffff;
	size_t i, j;

	for (i = 0; i < byte_num; i++) {
		mask = smask;
		for (j = 0; j < 8; j++) {
			a = ((CRC&CRCMask) != 0);
			b = ((in[i] & mask) != 0);

			CRC >>= 1;
			mask <<= 1;

			if (a ^ b)
				CRC ^= POLY;
		}
	}

	return CRC ^ (u4Byte)0xffffffff;
}

void rtw_hwsim_post_tx(struct _ADAPTER *adapter, struct xmit_frame *x)
{
	struct xmit_buf *xbuf;

	xbuf = x->pxmitbuf;

	rtw_free_xmitbuf(&adapter->xmitpriv, xbuf);

	/* Note: this also free the skb */
	rtw_free_xmitframe(&adapter->xmitpriv, x);
}
