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

#include "rtw_hwsim.h"
#include "rtw_hwsim_intf.h"     /* for rtw_hwsim_medium_tx() */

void rtw_hwsim_rx_tasklet(unsigned long priv);

static int rtw_hwsim_ndo_init(struct net_device *dev)
{
	RTW_INFO("hwsim ndo init\n");
	return 0;
}

static void rtw_hwsim_ndo_uninit(struct net_device *dev)
{
	RTW_INFO("hwsim ndo uninit\n");
}

static int rtw_hwsim_ndo_open(struct net_device *dev)
{
	struct rtw_hwsim_vif *vif;
	struct _ADAPTER *adapter;
	struct dvobj_priv *devobj;

	vif = netdev_priv(dev);
	adapter = vif_to_adapter(vif);
	devobj = vif_to_devobj(vif);

	RTW_INFO("hwsim ndo open\n");

	netif_carrier_on(dev);
	netif_start_queue(dev);
	netdev_reset_queue(dev);

	RTW_INFO("%s(): bup=%d bup_hwsim=%d\n", __func__, adapter->bup, adapter->bup_hwsim); 

	/* the following is moved from _netdev_open() */


	if (!dapter->bup_hwsim) {
		dev_clr_surprise_removed(devobj);
		dev_clr_drv_stopped(devobj);
		rtw_hw_start(devobj);
		rtw_start_drv_threads(adapter);
		rtw_cfg80211_init_wdev_data(adapter);
		adapter->bup_hwsim = _TRUE;
		adapter->bup = _TRUE;
	}

	adapter->net_closed = _FALSE;
	_set_timer(&devobj->dynamic_chk_timer, 2000);

#ifndef CONFIG_IPS_CHECK_IN_WD
	/* rtw_set_pwr_state_check_timer(pwrctrlpriv); */
#endif  /* CONFIG_IPS_CHECK_IN_WD */

	return 0;
}

static int rtw_hwsim_ndo_stop(struct net_device *dev)
{
	struct rtw_hwsim_vif *vif;

	RTW_INFO("hwsim ndo stop\n");
	netdev_reset_queue(dev);
	netif_stop_queue(dev);
	netif_carrier_off(dev);

	vif = netdev_priv(dev);

	return 0;
}

static netdev_tx_t rtw_hwsim_ndo_start_xmit(struct sk_buff *skb,
					    struct net_device *dev)
{
	struct rtw_hwsim_vif *vif;

	vif = netdev_priv(dev);

	return rtw_hwsim_medium_tx_handler(vif, skb);
}

static struct net_device_stats *rtw_hwsim_ndo_get_stats(struct net_device *dev)
{
	struct rtw_hwsim_vif *vif;
	struct _ADAPTER *adapter;
	struct xmit_priv *x;
	struct recv_priv *r;

	vif = ndev_to_vif(dev);
	adapter = vif_to_adap(vif);
	x = &adapter->xmitpriv;
	r = &adapter->recvpriv;

	/* this is copied from rtw_net_get_stats() */
	adapter->stats.tx_packets = x->tx_pkts;
	adapter->stats.rx_packets = r->rx_pkts;
	adapter->stats.tx_dropped = x->tx_drop;
	adapter->stats.rx_dropped = r->rx_drop; /*
	                                         * note that even this is always
	                                         * filled with 0, it still
	                                         * increases in ifconfig display
	                                         */
	adapter->stats.tx_bytes = x->tx_bytes;
	adapter->stats.rx_bytes = r->rx_bytes;

	return &adapter->stats;
}

static const struct net_device_ops rtw_hwsim_netdev_ops = {
	.ndo_init = rtw_hwsim_ndo_init,
	.ndo_uninit = rtw_hwsim_ndo_uninit,
	.ndo_open = rtw_hwsim_ndo_open,
	.ndo_stop = rtw_hwsim_ndo_stop,
	.ndo_start_xmit = rtw_hwsim_ndo_start_xmit,
	.ndo_set_features = NULL,
	.ndo_set_rx_mode = NULL,
	.ndo_get_stats = rtw_hwsim_ndo_get_stats
};

/**
 * rtw_hwsim_init_adapter_wdata() - Initialize wdata for adapter
 * @vif: The virtual interface of the corresponding adapter
 *
 * This function is used to initialize &_ADAPTER->rtw_wdev and
 * &_ADAPTER->wdev_data which were normally done in rtw_wdev_alloc(). As we
 * already have &wireless_dev in @vif, the adapter is forced to use that one in
 * this function.
 *
 * Context: This function should be called after &rtw_hwsim_vif->wdev and
 *          &_ADAPTER->pnetdev is initialized.
 *
 * Return: 0 on success, -1 otherwise
 */
static int rtw_hwsim_init_adapter_wdata(struct rtw_hwsim_vif *vif)
{
	struct wireless_dev *wdev;
	struct _ADAPTER *adapter;
	struct rtw_wdev_priv *priv;

	wdev = vif_to_wdev(vif);
	adapter = vif_to_adapter(vif);

	/* alloc wdev, ref rtw_cfg80211_ndev_res_alloc() */
	if (rtw_wdev_alloc(adapter, vif_to_wiphy(vif)) < 0) {
		RTW_ERR("rtw_wdev_alloc() failed\n");
		return -1;
	}

	/* Remember the adapter allocated wdev so it can be freed in
	 * rtw_hwsim_uninit_adapter_wdata().
	 */
	vif->adapter_wdev = adapter->rtw_wdev;

	/* force the adapter to use our wdev */
	adapter->rtw_wdev = wdev;
	adapter->pnetdev->ieee80211_ptr = wdev;

	/* modify &_ADAPTER->wdev_data */
	priv = adapter_wdev_data(adapter);
	priv->rtw_wdev = wdev;

	return 0;
}

/**
 * rtw_hwsim_uninit_adapter_wdata() - Un-initialize wdata for adapter
 * @vif: The virtual interface of the corresponding adapter
 *
 * Return: 0 on success, -1 otherwise
 */
static void rtw_hwsim_uninit_adapter_wdata(struct rtw_hwsim_vif *vif)
{
	/* The &rtw_hwsim_vif->adapter_wdev was allocated in rtw_wdev_alloc()
	 * and it should have its wiphy and netdev set correctly (as we've
	 * already set those in the adapter), therefore rtw_wdev_free() will
	 * get correct adapter with it.
	 */
	rtw_wdev_free(vif->adapter_wdev);
}

struct rtw_hwsim_vif *rtw_hwsim_interface_add(struct rtw_hwsim_data *data)
{
	struct net_device *ndev;
	struct rtw_hwsim_vif *vif;
	int rc;

	/* alloc netdev with multi-queue, ref rtw_alloc_etherdev() */
	ndev = alloc_netdev_mq(sizeof(*vif), "rtw_hwsim%d", NET_NAME_ENUM,
	                       ether_setup, 4);
	if (!ndev) {
		RTW_ERR("alloc_netdev_mq failed\n");
		goto fail;
	}

#ifdef CONFIG_NOARP
	ndev->flags |= IFF_NOARP;
#endif

	vif = netdev_priv(ndev);
	vif->wdev.wiphy = data->wiphy;
	vif->wdev.netdev = ndev;
	vif->wdev.iftype = NL80211_IFTYPE_STATION;
	ndev->ieee80211_ptr = &vif->wdev;
	vif->data = data;
	vif->ndev = ndev;
	SET_NETDEV_DEV(ndev, wiphy_dev(vif->wdev.wiphy));

	memcpy(vif->address.addr, data->addresses[0].addr, ETH_ALEN);
	memcpy(ndev->dev_addr, data->addresses[0].addr, ETH_ALEN);

	ndev->netdev_ops = &rtw_hwsim_netdev_ops;

	/* configure, ref rtw_init_netdev() */
	ndev->watchdog_timeo = HZ * 3; /* 3 second timeout */
#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	ndev->features |= NETIF_F_IP_CSUM;
#endif

#ifdef CONFIG_RTW_NETIF_SG
        ndev->features |= NETIF_F_SG;
        ndev->hw_features |= NETIF_F_SG;
#endif

	rtnl_lock();
	rc = register_netdevice(ndev);
	rtnl_unlock();

	if (rc < 0) {
		RTW_ERR("register_netdevice failed\n");
		goto fail_register_netdevice;
	}

	data->ndev = ndev;

	vif->adapter.pnetdev = ndev;

	if (rtw_hwsim_init_adapter(vif)) {
		RTW_ERR("allocate adapter failed\n");
		goto fail_alloc_adapter;
	}

	/* init the base class */
	vif->super.priv = &vif->adapter;
	vif->super.sizeof_priv = sizeof(vif->adapter);

	if (rtw_hwsim_init_adapter_wdata(vif) < 0) {
		RTW_ERR("init adapter wdata failed\n");
		goto fail_init_adapter_wdata;
	}

	if (rtw_hwsim_medium_init(vif, data->medium) < 0) {
		RTW_ERR("init medium failed\n");
		goto fail_init_medium;
	}

	vif->ifa_addr = 0;

	return vif;

fail_init_medium:
	rtw_hwsim_uninit_adapter_wdata(vif);
fail_init_adapter_wdata:
	rtw_hwsim_deinit_adapter(vif_to_adapter(vif));
fail_alloc_adapter:
	rtnl_lock();
	unregister_netdevice(ndev);
	rtnl_unlock();
fail_register_netdevice:
	free_netdev(ndev);
fail:
	return NULL;
}

void rtw_hwsim_interface_del(struct rtw_hwsim_vif *vif)
{
	struct _ADAPTER *adapter;

	adapter = vif_to_adapter(vif);

	rtw_hwsim_medium_deinit(vif);

	rtw_hwsim_uninit_adapter_wdata(vif);

	rtw_hwsim_deinit_adapter(vif_to_adapter(vif));

	rtnl_lock();
	unregister_netdevice(vif_to_netdev(vif));
	rtnl_unlock();

	free_netdev(vif_to_netdev(vif));
}

int rtw_hwsim_inetaddr_notifier_call(struct notifier_block *nb,
                                     unsigned long action, void *data)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)data;
	struct net_device *ndev;
	struct rtw_hwsim_data *hwsim_data;
	struct rtw_hwsim_vif *vif = NULL;
	struct mlme_ext_info *mlme_ext;

	if (!ifa || !ifa->ifa_dev || !ifa->ifa_dev->dev)
		return NOTIFY_DONE;

	ndev = ifa->ifa_dev->dev;

	spin_lock_bh(&rtw_hwsim_data_lock);

	list_for_each_entry(hwsim_data, &g_data_list, list) {
		if (hwsim_data->vifs[0]->ndev == ndev) {
			vif = hwsim_data->vifs[0];
			break;
		}
	}

	if (vif) {
		mlme_ext = &vif->adapter.mlmeextpriv.mlmext_info;

		switch (action) {
		case NETDEV_UP:
			vif->ifa_addr = ifa->ifa_address;

			/* copied from rtw_inetaddr_notifier_call() */
			_rtw_memcpy(mlme_ext->ip_addr, &ifa->ifa_address,
			            RTW_IP_ADDR_LEN);
			RTW_INFO("%s(): %s up, ip=%pI4\n", __func__,
			         ifa->ifa_label, mlme_ext->ip_addr);
			break;
		case NETDEV_DOWN:
			RTW_INFO("%s(): %s down\n", __func__, ifa->ifa_label);
			vif->ifa_addr = ifa->ifa_address;

			/* copied from rtw_inetaddr_notifier_call() */
			_rtw_memset(mlme_ext->ip_addr, 0, RTW_IP_ADDR_LEN);
			break;
		default:
			break;
		}
	}

	spin_unlock_bh(&rtw_hwsim_data_lock);

	return NOTIFY_DONE;
}
