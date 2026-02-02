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

#include "../rtw_hwsim.h"

static struct kmem_cache *frame_cache;

struct medium_spec {
	struct tasklet_struct rx_tasklet;
	struct list_head rx_queue;
	spinlock_t rx_lock;
};

struct rtw_hwsim_rx_frame {
	struct list_head list;
	struct rtw_hwsim_frame f;
};

#define med_to_mspec(m) ((struct medium_spec *)m->priv)
#define rxf_to_f(rxf) (&rxf->f)

static void dump_rx_frame(const struct rtw_hwsim_rx_frame *rxf)
{
	RTW_INFO("----------------\n");
	RTW_INFO("%s(): frame len: %u\n", __func__,
	         le16_to_cpu(rxf->f.m.buf_len));
	RTW_INFO_DUMP("frame: ", rxf->f.buf, le16_to_cpu(rxf->f.m.buf_len));
}

static void dump_rx_queue(const struct list_head *q)
{
	struct list_head *l;
	struct rtw_hwsim_rx_frame *rxf;

	RTW_INFO("%s() --->\n", __func__);
	list_for_each(l, q) {
		rxf = list_entry(l, struct rtw_hwsim_rx_frame, list);
		dump_rx_frame(rxf);
	}
	RTW_INFO("%s() <---\n", __func__);
}

static void reap_rx(struct medium_spec *mspec, struct list_head *q)
{
	INIT_LIST_HEAD(q);

	spin_lock_bh(&mspec->rx_lock);
	list_splice(&mspec->rx_queue, q);
	/* TODO: check why hang if we dump queue here */
	INIT_LIST_HEAD(&mspec->rx_queue);
	spin_unlock_bh(&mspec->rx_lock);
}

static int process_rx_frame(struct rtw_hwsim_vif *vif,
                            struct rtw_hwsim_frame *f)
{
	struct _ADAPTER *adapter;
	union recv_frame *r;
	u8 *buf, *data;
	size_t buflen;
	struct wireless_dev *wdev;

	adapter = vif_to_adap(vif);

	buf = f->buf;
	buflen = le16_to_cpu(f->m.buf_len);

	/* append crc32 for monitor mode */
	wdev = vif_to_wdev(vif);
	if (wdev->iftype == NL80211_IFTYPE_MONITOR) {
		u32 crc;

		crc = rtw_hwsim_crc32(buf, buflen);
		_rtw_memcpy(buf + buflen, &crc, sizeof(crc));
		buflen += sizeof(crc);
		f->m.buf_len = cpu_to_le16(buflen);
	}

	r = rtw_alloc_recvframe(&adapter->recvpriv.free_recv_queue);
	if (!r) {
		RTW_ERR("%s(): alloc memory for recvframe failed\n",
		        __func__);
		return -1;
	}

	rtw_init_recvframe(r, &adapter->recvpriv);

	rtw_hwsim_rx_translate(adapter, f, r);

	if (rtw_os_alloc_recvframe(adapter, r, buf, NULL) == _FAIL) {
		RTW_ERR("%s(): failed to allocate recv frame\n", __func__);
		rtw_free_recvframe(r, &adapter->recvpriv.free_recv_queue);
		return -1;
	}

	recvframe_put(r, buflen);

	pre_recv_entry(r, NULL);

	return 0;
}

static void rx_tasklet(unsigned long priv)
{
	struct rtw_hwsim_medium *med;
	struct list_head flist;
	struct rtw_hwsim_rx_frame *rxf, *t;

	med = (struct rtw_hwsim_medium *)priv;

	reap_rx(med_to_mspec(med), &flist);

	list_for_each_entry_safe(rxf, t, &flist, list) {
		process_rx_frame(med->vif, rxf_to_f(rxf));
		list_del(&rxf->list);
		kmem_cache_free(frame_cache, rxf);
	}
}

static int ins_rx_queue(struct rtw_hwsim_medium *med,
                        struct rtw_hwsim_rx_frame *rxf)
{
	struct medium_spec *mspec;

	mspec = med_to_mspec(med);

	INIT_LIST_HEAD(&rxf->list);

	spin_lock_bh(&mspec->rx_lock);
	list_add_tail(&rxf->list, &mspec->rx_queue);
	spin_unlock_bh(&mspec->rx_lock);

	tasklet_hi_schedule(&mspec->rx_tasklet);

	return 0;
}

static int init(struct rtw_hwsim_medium *med)
{
	struct medium_spec *mspec;

	RTW_INFO("%s(): initializing local medium\n", __func__);

	mspec = (struct medium_spec *)rtw_malloc(sizeof(*mspec));
	if (!mspec)
		return -1;

	tasklet_init(&mspec->rx_tasklet, &rx_tasklet, (unsigned long)med);
	spin_lock_init(&mspec->rx_lock);
	INIT_LIST_HEAD(&mspec->rx_queue);

	if (!frame_cache) {
		size_t size;

		size = sizeof(struct rtw_hwsim_rx_frame) +
			RTW_HWSIM_RXF_QUANTUM;
		frame_cache = kmem_cache_create("rtw_hwsim-local", size, 0,
		                                SLAB_HWCACHE_ALIGN, NULL);
		if (!frame_cache) {
			RTW_ERR("%s(): kmem_cache_create failed\n", __func__);
			return -1;
		}
	}

	med->priv = mspec;

	return 0;
}

static void deinit(struct rtw_hwsim_medium *med)
{
	struct medium_spec *mspec;

	mspec = med_to_mspec(med);
	tasklet_kill(&mspec->rx_tasklet);
	if (frame_cache) {
		kmem_cache_destroy(frame_cache);
		frame_cache = NULL;
	}
	rtw_mfree(mspec, sizeof(*mspec));

	RTW_INFO("%s(): local medium deinitialized\n", __func__);
}

static netdev_tx_t tx_handler(struct rtw_hwsim_medium *med, struct sk_buff *skb)
{
	struct rtw_hwsim_vif *vif;
	struct security_priv *sec;

	vif = med_to_vif(med);
        sec = &vif->adapter.securitypriv;

	rtw_hwsim_tweak_ip_addr(skb);

	sec->hw_decrypted = _TRUE;

	return rtw_xmit_entry(skb, vif_to_netdev(vif));
}

static void add_fake_icv(struct rtw_hwsim_frame *f)
{
	u16 buflen;

	if (_NO_PRIVACY_ < f->m.encrypt) {
		buflen = le16_to_cpu(f->m.buf_len);
		_rtw_memset(f->buf + buflen, 0xFE, f->m.icv_len);
		buflen += f->m.icv_len;
		f->m.buf_len = cpu_to_le16(buflen);
	}
}

static int local_tx(struct rtw_hwsim_medium *med, const struct xmit_frame *x,
                    const u8 *buf, size_t buflen,
                    struct rtw_hwsim_medium *target)
{
	struct rtw_hwsim_rx_frame *rxf;
	struct rtw_hwsim_frame *f;

	if (buflen > RTW_HWSIM_RXF_QUANTUM) {
		RTW_ERR("%s(): sz(%zd) exceed quantum(%d)\n", __func__, buflen,
		        RTW_HWSIM_RXF_QUANTUM);
		return -1;
	}

	rxf = kmem_cache_alloc(frame_cache, GFP_ATOMIC);
	if (!rxf) {
		RTW_ERR("%s(): failed to allocate rx frame\n", __func__);
		return -1;
	}

	f = rxf_to_f(rxf);

	/* build the frame for xmit, free after processed by target tasklet */
	rtw_hwsim_build_metadata(&f->m, x, buflen);
	_rtw_memcpy(f->buf, buf, buflen);
	add_fake_icv(f);

	if (ins_rx_queue(target, rxf) < 0)
		return -1;

	return 0;
}

static int medium_tx(struct rtw_hwsim_medium *med, const void *tx_ctx,
                     u8 *buf, size_t buflen)
{
	struct xmit_frame *x;
	struct rtw_hwsim_vif *vif, *vif2;
	struct rtw_hwsim_data *data, *data2;
	const struct pkt_attrib *a;
	struct xmit_buf *xbuf;
	struct wireless_dev *wdev2;

	x = (struct xmit_frame *)tx_ctx;
	vif = med_to_vif(med);
	data = vif->data;
	xbuf = x->pxmitbuf;
	a = &x->attrib;

	spin_lock_bh(&rtw_hwsim_data_lock);
	list_for_each_entry(data2, &g_data_list, list) {
		/* assume only 1 vif */
		vif2 = data2->vifs[0];

		if (data == data2)
			continue;

		wdev2 = vif_to_wdev(vif2);

		/* distribute all multicast frames */
		if (!IS_MCAST(a->ra) &&
		      _rtw_memcmp(a->ra, &vif2->address,
		                  ETH_ALEN) == _FALSE &&
		      wdev2->iftype != NL80211_IFTYPE_MONITOR)
			continue;

		if (!netif_running(vif2->ndev))
			continue;

		if (local_tx(med, x, buf, buflen, vif_to_med(vif2)) < 0)
			continue;
	}
	spin_unlock_bh(&rtw_hwsim_data_lock);

	rtw_sctx_done(&xbuf->sctx);

	rtw_hwsim_post_tx(vif_to_adap(vif), x);

	return 0;
}

static void pre_netif_rx(struct rtw_hwsim_medium *med, struct sk_buff *skb)
{
	/* to avoid L3 checsum validation */
	skb->ip_summed = CHECKSUM_UNNECESSARY;
}

static struct rtw_hwsim_medium_ops ops = {
	.init = init,
	.deinit = deinit,
	.tx_handler = tx_handler,
	.medium_tx = medium_tx,
	.pre_netif_rx = pre_netif_rx,
};

void rtw_hwsim_medium_local_ops(struct rtw_hwsim_medium *med)
{
	med->ops = &ops;
}
