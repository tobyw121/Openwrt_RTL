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

static int init(struct rtw_hwsim_medium *med)
{
	RTW_INFO("%s(): initializing loopback medium\n", __func__);
	med->priv = NULL;
	return 0;
}

static void deinit(struct rtw_hwsim_medium *med)
{
	RTW_INFO("%s(): loopback medium deinitialized\n", __func__);
}

static int rx(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *nskb;
	u8 *buf;
	struct rtw_hwsim_vif *d_vif;
	struct _ADAPTER *d_adapter;

	nskb = skb_clone(skb, GFP_ATOMIC);
	if (!nskb) {
		RTW_ERR("dev_alloc_skb failed\n");
		goto fail;
	}

	nskb->protocol = eth_type_trans(nskb, dev);
	nskb->dev = dev;
	nskb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

	d_vif = ndev_to_vif(dev);
	d_adapter = vif_to_adap(d_vif);
	d_adapter = vif_to_adap(d_vif);
	d_adapter->recvpriv.rx_pkts++;
	d_adapter->recvpriv.rx_bytes += nskb->len;

	return netif_rx(nskb);

fail_invalid_prot:
	dev_kfree_skb(nskb);
fail:
	return -1;
}

static netdev_tx_t tx_handler(struct rtw_hwsim_medium *med, struct sk_buff *skb)
{
	struct rtw_hwsim_vif *vif, *vif2;
	struct rtw_hwsim_data *data, *data2;
	struct _ADAPTER *adapter;

	vif = med_to_vif(med);
	data = vif->data;
	adapter = vif_to_adap(vif);

	rtw_hwsim_tweak_ip_addr(skb);

	spin_lock_bh(&rtw_hwsim_data_lock);
	list_for_each_entry(data2, &g_data_list, list) {
		/* assume only 1 vif */
		vif2 = data2->vifs[0];

		if (data == data2)
			continue;

		if (!netif_running(vif2->ndev))
			continue;

		if (rx(skb, vif2->ndev) < 0)
			continue;
	}
	spin_unlock_bh(&rtw_hwsim_data_lock);

	adapter->xmitpriv.tx_pkts++;
	adapter->xmitpriv.tx_bytes += skb->len;

	dev_kfree_skb(skb);

	return NETDEV_TX_OK;
}

static int medium_tx(struct rtw_hwsim_medium *med, const void *tx_ctx,
                     u8 *buf, size_t buflen)
{
	struct xmit_frame *x;
	struct rtw_hwsim_vif *vif;
	struct xmit_buf *xbuf;

	/*
	 * Have to do the cleanup job even we don't really support medium tx in
	 * this mode.
	 */

	x = (struct xmit_frame *)tx_ctx;
	vif = med_to_vif(med);
	xbuf = x->pxmitbuf;

	rtw_sctx_done(&xbuf->sctx);
	rtw_hwsim_post_tx(vif_to_adap(vif), x);

	return 0;
}

static struct rtw_hwsim_medium_ops ops = {
	.init = init,
	.deinit = deinit,
	.tx_handler = tx_handler,
	.medium_tx = medium_tx,
};

void rtw_hwsim_medium_loopback_ops(struct rtw_hwsim_medium *med)
{
	med->ops = &ops;
}
