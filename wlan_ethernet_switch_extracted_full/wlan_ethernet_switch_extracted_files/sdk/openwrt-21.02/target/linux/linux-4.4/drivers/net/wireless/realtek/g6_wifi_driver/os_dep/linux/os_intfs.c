/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#define _OS_INTFS_C_

#include <drv_types.h>
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
#include <net/rtl/rtl_brsc.h>
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION(DRIVERVERSION);


int netdev_open(struct net_device *pnetdev);
int netdev_close(struct net_device *pnetdev);
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
#include <net/rtl/rtw_crossband_repeater.h>
extern int (*crossband_get_dev_status_5G)(struct net_device *dev, struct crossband_dev_status *dev_status);
extern int rtw_crossband_get_dev_status(struct net_device *dev, struct crossband_dev_status *dev_status);
#else
_adapter	*tmp_crossband_vxd_sc = NULL;
#endif
#endif


/**
 * rtw_net_set_mac_address
 * This callback function is used for the Media Access Control address
 * of each net_device needs to be changed.
 *
 * Arguments:
 * @pnetdev: net_device pointer.
 * @addr: new MAC address.
 *
 * Return:
 * ret = 0: Permit to change net_device's MAC address.
 * ret = -1 (Default): Operation not permitted.
 *
 * Auther: Arvin Liu
 * Date: 2015/05/29
 */
int rtw_net_set_mac_address(struct net_device *pnetdev, void *addr)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct sockaddr *sa = (struct sockaddr *)addr;
	int ret = -1;

	/* only the net_device is in down state to permit modifying mac addr */
	if ((pnetdev->flags & IFF_UP) == _TRUE) {
		RTW_INFO(FUNC_ADPT_FMT": The net_device's is not in down state\n"
			 , FUNC_ADPT_ARG(padapter));

		return ret;
	}

	/* if the net_device is linked, it's not permit to modify mac addr */
	if (check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) ||
	    check_fwstate(pmlmepriv, WIFI_ASOC_STATE) ||
	    check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY)) {
		RTW_INFO(FUNC_ADPT_FMT": The net_device's is not idle currently\n"
			 , FUNC_ADPT_ARG(padapter));

		return ret;
	}

	/* check whether the input mac address is valid to permit modifying mac addr */
	if (rtw_check_invalid_mac_address(sa->sa_data, _FALSE) == _TRUE) {
		RTW_INFO(FUNC_ADPT_FMT": Invalid Mac Addr for "MAC_FMT"\n"
			 , FUNC_ADPT_ARG(padapter), MAC_ARG(sa->sa_data));

		return ret;
	}

	_rtw_memcpy(adapter_mac_addr(padapter), sa->sa_data, ETH_ALEN); /* set mac addr to adapter */
	_rtw_memcpy(pnetdev->dev_addr, sa->sa_data, ETH_ALEN); /* set mac addr to net_device */

	rtw_ps_deny(padapter, PS_DENY_IOCTL);
	LeaveAllPowerSaveModeDirect(padapter); /* leave PS mode for guaranteeing to access hw register successfully */

	rtw_hal_set_hwreg(padapter, HW_VAR_MAC_ADDR, sa->sa_data); /* set mac addr to mac register */

	rtw_ps_deny_cancel(padapter, PS_DENY_IOCTL);


	RTW_INFO(FUNC_ADPT_FMT": Set Mac Addr to "MAC_FMT" Successfully\n"
		 , FUNC_ADPT_ARG(padapter), MAC_ARG(sa->sa_data));

	ret = 0;

	return ret;
}

static struct net_device_stats *rtw_net_get_stats(struct net_device *pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct recv_priv *precvpriv = &(padapter->recvpriv);

	padapter->stats.tx_packets = pxmitpriv->tx_pkts;/* pxmitpriv->tx_pkts++; */
	padapter->stats.rx_packets = precvpriv->rx_pkts;/* precvpriv->rx_pkts++; */
	padapter->stats.tx_dropped = pxmitpriv->tx_drop;
	padapter->stats.rx_dropped = precvpriv->rx_drop;
	padapter->stats.tx_bytes = pxmitpriv->tx_bytes;
	padapter->stats.rx_bytes = precvpriv->rx_bytes;

	return &padapter->stats;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
/*
 * AC to queue mapping
 *
 * AC_VO -> queue 0
 * AC_VI -> queue 1
 * AC_BE -> queue 2
 * AC_BK -> queue 3
 */
static const u16 rtw_1d_to_queue[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

/* Given a data frame determine the 802.1p/1d tag to use. */
unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263)
		return skb->priority - 256;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}


static u16 rtw_select_queue(struct net_device *dev, struct sk_buff *skb
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	, struct net_device *sb_dev
	#else
	, void *accel_priv
	#endif
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
	, select_queue_fallback_t fallback
	#endif
#endif
)
{
	_adapter	*padapter = rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	skb->priority = rtw_classify8021d(skb);

	if (pmlmepriv->acm_mask != 0)
		skb->priority = qos_acm(pmlmepriv->acm_mask, skb->priority);

	return rtw_1d_to_queue[skb->priority];
}

u16 rtw_recv_select_queue(struct sk_buff *skb)
{
	struct iphdr *piphdr;
	unsigned int dscp;
	u16	eth_type;
	u32 priority;
	u8 *pdata = skb->data;

	_rtw_memcpy(&eth_type, pdata + (ETH_ALEN << 1), 2);

	switch (eth_type) {
	case htons(ETH_P_IP):

		piphdr = (struct iphdr *)(pdata + ETH_HLEN);

		dscp = piphdr->tos & 0xfc;

		priority = dscp >> 5;

		break;
	default:
		priority = 0;
	}

	return rtw_1d_to_queue[priority];

}

#endif

static u8 is_rtw_ndev(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
	return ndev->netdev_ops
		&& ndev->netdev_ops->ndo_do_ioctl
		&& ndev->netdev_ops->ndo_do_ioctl == rtw_ioctl;
#else
	return ndev->do_ioctl
		&& ndev->do_ioctl == rtw_ioctl;
#endif
}


#define _netdev_status_msg(_ndev, state, sts_str)		\
	RTW_INFO(FUNC_NDEV_FMT" state:%lu - %s\n", FUNC_NDEV_ARG(_ndev), state, sts_str);

static int rtw_ndev_notifier_call(struct notifier_block *nb, unsigned long state, void *ptr)
{
	struct net_device *ndev;

	if (ptr == NULL)
		return NOTIFY_DONE;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	ndev = netdev_notifier_info_to_dev(ptr);
#else
	ndev = ptr;
#endif

	if (ndev == NULL)
		return NOTIFY_DONE;

	if (!is_rtw_ndev(ndev))
		return NOTIFY_DONE;

	switch (state) {
	case NETDEV_CHANGE:
		_netdev_status_msg(ndev, state, "netdev change");
		break;
	case NETDEV_GOING_DOWN:
		_netdev_status_msg(ndev, state, "netdev going down");
		break;
	case NETDEV_DOWN:
		_netdev_status_msg(ndev, state, "netdev down");
		break;
	case NETDEV_UP:
		_netdev_status_msg(ndev, state, "netdev up");
		break;
	case NETDEV_REBOOT:
		_netdev_status_msg(ndev, state, "netdev reboot");
		break;
	case NETDEV_CHANGENAME:
		rtw_adapter_proc_replace(ndev);
		_netdev_status_msg(ndev, state, "netdev chang ename");
		break;
	case NETDEV_PRE_UP :
		{
			_adapter *adapter = rtw_netdev_priv(ndev);
			int ret = _SUCCESS;

			ret = rtw_pwr_wakeup(adapter);
		}
		_netdev_status_msg(ndev, state, "netdev pre up");
		break;
	case NETDEV_JOIN:
		_netdev_status_msg(ndev, state, "netdev join");
		break;
	default:
		_netdev_status_msg(ndev, state, " ");
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block rtw_ndev_notifier = {
	.notifier_call = rtw_ndev_notifier_call,
};

int rtw_ndev_notifier_register(void)
{
	return register_netdevice_notifier(&rtw_ndev_notifier);
}

void rtw_ndev_notifier_unregister(void)
{
	unregister_netdevice_notifier(&rtw_ndev_notifier);
}

int rtw_ndev_init(struct net_device *dev)
{
	_adapter *adapter = rtw_netdev_priv(dev);

	RTW_PRINT(FUNC_ADPT_FMT" if%d mac_addr="MAC_FMT"\n"
		, FUNC_ADPT_ARG(adapter), (adapter->iface_id + 1), MAC_ARG(dev->dev_addr));
	strncpy(adapter->old_ifname, dev->name, IFNAMSIZ);
	adapter->old_ifname[IFNAMSIZ - 1] = '\0';
	rtw_adapter_proc_init(dev);

#if defined(CONFIG_ARCH_CORTINA) || defined(CONFIG_RTL8672) || defined(CONFIG_LUNA_G3_SERIES)
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	dev->rtk_priv_flags = RTK_IFF_DOMAIN_WLAN;
#else
	dev->priv_flags = IFF_DOMAIN_WLAN;
#endif
#endif

	return 0;
}

void rtw_ndev_uninit(struct net_device *dev)
{
	_adapter *adapter = rtw_netdev_priv(dev);

	RTW_PRINT(FUNC_ADPT_FMT" if%d\n"
		  , FUNC_ADPT_ARG(adapter), (adapter->iface_id + 1));
	rtw_adapter_proc_deinit(dev);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops rtw_netdev_ops = {
	.ndo_init = rtw_ndev_init,
	.ndo_uninit = rtw_ndev_uninit,
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	.ndo_select_queue	= rtw_select_queue,
#endif
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
};
#endif

int rtw_init_netdev_name(struct net_device *pnetdev, const char *ifname)
{
	if (dev_alloc_name(pnetdev, ifname) < 0)
		RTW_ERR("dev_alloc_name, fail!\n");

	rtw_netif_carrier_off(pnetdev);
	/* rtw_netif_stop_queue(pnetdev); */

	return 0;
}

void rtw_hook_if_ops(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
	ndev->netdev_ops = &rtw_netdev_ops;
#else
	ndev->init = rtw_ndev_init;
	ndev->uninit = rtw_ndev_uninit;
	ndev->open = netdev_open;
	ndev->stop = netdev_close;
	ndev->hard_start_xmit = rtw_xmit_entry;
	ndev->set_mac_address = rtw_net_set_mac_address;
	ndev->get_stats = rtw_net_get_stats;
	ndev->do_ioctl = rtw_ioctl;
#endif
}

#ifdef CONFIG_CONCURRENT_MODE
static void rtw_hook_vir_if_ops(struct net_device *ndev);
#endif
struct net_device *rtw_init_netdev(_adapter *old_padapter)
{
	_adapter *padapter;
	struct net_device *pnetdev;

	if (old_padapter != NULL) {
		rtw_os_ndev_free(old_padapter);
		pnetdev = rtw_alloc_etherdev_with_old_priv(sizeof(_adapter), (void *)old_padapter);
	} else
		pnetdev = rtw_alloc_etherdev(sizeof(_adapter));

	if (!pnetdev)
		return NULL;

	padapter = rtw_netdev_priv(pnetdev);
	padapter->pnetdev = pnetdev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
	SET_MODULE_OWNER(pnetdev);
#endif

	rtw_hook_if_ops(pnetdev);
#ifdef CONFIG_CONCURRENT_MODE
	if (!is_primary_adapter(padapter))
		rtw_hook_vir_if_ops(pnetdev);
#endif /* CONFIG_CONCURRENT_MODE */


#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
        pnetdev->features |= (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
        pnetdev->hw_features |= (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
#endif
#endif

#ifdef CONFIG_RTW_NETIF_SG
        pnetdev->features |= NETIF_F_SG;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
        pnetdev->hw_features |= NETIF_F_SG;
#endif
#endif

	if ((pnetdev->features & NETIF_F_SG) && (pnetdev->features & NETIF_F_IP_CSUM)) {
		pnetdev->features |= (NETIF_F_TSO | NETIF_F_GSO);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
		pnetdev->hw_features |= (NETIF_F_TSO | NETIF_F_GSO);
#endif
	}
	/* pnetdev->tx_timeout = NULL; */
	pnetdev->watchdog_timeo = HZ * 3; /* 3 second timeout */

#ifdef CONFIG_WIRELESS_EXT
	pnetdev->wireless_handlers = (struct iw_handler_def *)&rtw_handlers_def;
#endif

#ifdef WIRELESS_SPY
	/* priv->wireless_data.spy_data = &priv->spy_data; */
	/* pnetdev->wireless_data = &priv->wireless_data; */
#endif

#if defined(CONFIG_TX_AMSDU_SW_MODE)
	pnetdev->needed_headroom += 8;	/* +8 for rfc1042 header */
	pnetdev->needed_headroom += 4;	/* +4 for padding */
#endif

#if defined(CONFIG_TXSC_AMSDU)
	pnetdev->needed_headroom += 8;	/* for rfc1042 header */
	pnetdev->needed_tailroom += 3;	/* for padding */
#endif

	return pnetdev;
}
#ifdef CONFIG_PCI_HCI
#include <rtw_trx_pci.h>
#endif
int rtw_os_ndev_alloc(_adapter *adapter, enum nl80211_iftype type)
{
	int ret = _FAIL;
	struct net_device *ndev = NULL;

	ndev = rtw_init_netdev(adapter);
	if (ndev == NULL) {
		rtw_warn_on(1);
		goto exit;
	}
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 0)
	SET_NETDEV_DEV(ndev, dvobj_to_dev(adapter_to_dvobj(adapter)));
#endif

#ifdef CONFIG_PCI_HCI
	if (is_pci_support_dma64(adapter_to_dvobj(adapter)))
		ndev->features |= NETIF_F_HIGHDMA;
	ndev->irq = dvobj_to_pci(adapter_to_dvobj(adapter))->irq;
#endif

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_ndev_res_alloc(adapter, type) != _SUCCESS) {
		rtw_warn_on(1);
	} else
#endif
	ret = _SUCCESS;

	if (ret != _SUCCESS && ndev)
		rtw_free_netdev(ndev);
exit:
	return ret;
}

void rtw_os_ndev_free(_adapter *adapter)
{
#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_ndev_res_free(adapter);
#endif

	/* free the old_pnetdev */
	if (adapter->rereg_nd_name_priv.old_pnetdev) {
		rtw_free_netdev(adapter->rereg_nd_name_priv.old_pnetdev);
		adapter->rereg_nd_name_priv.old_pnetdev = NULL;
	}

	if (adapter->pnetdev) {
		rtw_free_netdev(adapter->pnetdev);
		adapter->pnetdev = NULL;
	}
}

/* For ethtool +++ */
#ifdef CONFIG_IOCTL_CFG80211
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 8))
static void rtw_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct wireless_dev *wdev = NULL;
	_adapter *padapter = NULL;

	wdev = dev->ieee80211_ptr;
	if (wdev) {
		strlcpy(info->driver, wiphy_dev(wdev->wiphy)->driver->name,
			sizeof(info->driver));
	} else {
		strlcpy(info->driver, "N/A", sizeof(info->driver));
	}

	strlcpy(info->version, DRIVERVERSION, sizeof(info->version));

	padapter = (_adapter *)rtw_netdev_priv(dev);


	/*GEORGIA_TODO_FIXIT*/
	#if 0
	HAL_DATA_TYPE *hal_data = NULL;
	if (padapter) {
		hal_data = GET_HAL_DATA(padapter);
	}
	if (hal_data) {
		scnprintf(info->fw_version, sizeof(info->fw_version), "%d.%d",
			  hal_data->firmware_version, hal_data->firmware_sub_version);
	} else
	#endif
	{
		strlcpy(info->fw_version, "N/A", sizeof(info->fw_version));
	}

	if (wdev) {
		strlcpy(info->bus_info, dev_name(wiphy_dev(wdev->wiphy)),
			sizeof(info->bus_info));
	}
}

static const char rtw_ethtool_gstrings_sta_stats[][ETH_GSTRING_LEN] = {
	"rx_packets", "rx_bytes", "rx_dropped",
	"tx_packets", "tx_bytes", "tx_dropped",
};

#define RTW_ETHTOOL_STATS_LEN	ARRAY_SIZE(rtw_ethtool_gstrings_sta_stats)

static int rtw_ethtool_get_sset_count(struct net_device *dev, int sset)
{
	int rv = 0;

	if (sset == ETH_SS_STATS)
		rv += RTW_ETHTOOL_STATS_LEN;

	if (rv == 0)
		return -EOPNOTSUPP;

	return rv;
}

static void rtw_ethtool_get_strings(struct net_device *dev, u32 sset, u8 *data)
{
	int sz_sta_stats = 0;

	if (sset == ETH_SS_STATS) {
		sz_sta_stats = sizeof(rtw_ethtool_gstrings_sta_stats);
		_rtw_memcpy(data, rtw_ethtool_gstrings_sta_stats, sz_sta_stats);
	}
}

static void rtw_ethtool_get_stats(struct net_device *dev,
				  struct ethtool_stats *stats,
				  u64 *data)
{
	int i = 0;
	_adapter *padapter = NULL;
	struct xmit_priv *pxmitpriv = NULL;
	struct recv_priv *precvpriv = NULL;

	memset(data, 0, sizeof(u64) * RTW_ETHTOOL_STATS_LEN);

	padapter = (_adapter *)rtw_netdev_priv(dev);
	if (padapter) {
		pxmitpriv = &(padapter->xmitpriv);
		precvpriv = &(padapter->recvpriv);

		data[i++] = precvpriv->rx_pkts;
		data[i++] = precvpriv->rx_bytes;
		data[i++] = precvpriv->rx_drop;

		data[i++] = pxmitpriv->tx_pkts;
		data[i++] = pxmitpriv->tx_bytes;
		data[i++] = pxmitpriv->tx_drop;
	} else {
		data[i++] = 0;
		data[i++] = 0;
		data[i++] = 0;

		data[i++] = 0;
		data[i++] = 0;
		data[i++] = 0;
	}
}

static const struct ethtool_ops rtw_ethtool_ops = {
	.get_drvinfo = rtw_ethtool_get_drvinfo,
	.get_link = ethtool_op_get_link,
	.get_strings = rtw_ethtool_get_strings,
	.get_ethtool_stats = rtw_ethtool_get_stats,
	.get_sset_count = rtw_ethtool_get_sset_count,
};
#endif // LINUX_VERSION_CODE >= 3.7.8
#endif /* CONFIG_IOCTL_CFG80211 */
/* For ethtool --- */

int rtw_os_ndev_register(_adapter *adapter, const char *name)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	int ret = _SUCCESS;
	struct net_device *ndev = adapter->pnetdev;
	u8 rtnl_lock_needed = rtw_rtnl_lock_needed(dvobj);

#ifdef CONFIG_RTW_NAPI
	netif_napi_add(ndev, &adapter->napi, rtw_recv_napi_poll, RTL_NAPI_WEIGHT);
#endif /* CONFIG_RTW_NAPI */

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_ndev_res_register(adapter) != _SUCCESS) {
		rtw_warn_on(1);
		ret = _FAIL;
		goto exit;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 8))
	netdev_set_default_ethtool_ops(ndev, &rtw_ethtool_ops);
#endif /* LINUX_VERSION_CODE >= 3.7.8 */
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)) && defined(CONFIG_PCI_HCI)
	ndev->gro_flush_timeout = 100000;
#endif
	/* alloc netdev name */
	rtw_init_netdev_name(ndev, name);

	_rtw_memcpy(ndev->dev_addr, adapter_mac_addr(adapter), ETH_ALEN);

	/* Tell the network stack we exist */

	if (rtnl_lock_needed)
		ret = (register_netdev(ndev) == 0) ? _SUCCESS : _FAIL;
	else
		ret = (register_netdevice(ndev) == 0) ? _SUCCESS : _FAIL;

	if (ret == _SUCCESS)
		adapter->registered = 1;
	else
		RTW_INFO(FUNC_NDEV_FMT" if%d Failed!\n", FUNC_NDEV_ARG(ndev), (adapter->iface_id + 1));

#if defined(WIFI6_THER_CTRL) && defined(PLATFORM_LINUX)
	if (is_primary_adapter(adapter)) {
		extern void reg_wifi6_ther_support(struct net_device *dev);
		reg_wifi6_ther_support(ndev);
	}
#endif /* WIFI6_THER_CTRL && PLATFORM_LINUX */

#if defined(CONFIG_IOCTL_CFG80211)
	if (ret != _SUCCESS) {
		rtw_cfg80211_ndev_res_unregister(adapter);
	}
#endif

#if defined(CONFIG_IOCTL_CFG80211)
exit:
#endif
#ifdef CONFIG_RTW_NAPI
	if (ret != _SUCCESS)
		netif_napi_del(&adapter->napi);
#endif /* CONFIG_RTW_NAPI */

	return ret;
}

void rtw_os_ndev_unregister(_adapter *adapter)
{
	struct net_device *netdev = NULL;

	if (adapter == NULL || adapter->registered == 0)
		return;

	adapter->ndev_unregistering = 1;

	netdev = adapter->pnetdev;

#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_ndev_res_unregister(adapter);
#endif

	if (netdev) {
		struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
		u8 rtnl_lock_needed = rtw_rtnl_lock_needed(dvobj);

		if (rtnl_lock_needed)
			unregister_netdev(netdev);
		else
			unregister_netdevice(netdev);
	}

#ifdef CONFIG_RTW_NAPI
	if (adapter->napi_state == NAPI_ENABLE) {
		napi_disable(&adapter->napi);
		adapter->napi_state = NAPI_DISABLE;
	}
	netif_napi_del(&adapter->napi);
#endif /* CONFIG_RTW_NAPI */

	adapter->registered = 0;
	adapter->ndev_unregistering = 0;
}

/**
 * rtw_os_ndev_init - Allocate and register OS layer net device and relating structures for @adapter
 * @adapter: the adapter on which this function applies
 * @name: the requesting net device name
 *
 * Returns:
 * _SUCCESS or _FAIL
 */
int rtw_os_ndev_init(_adapter *adapter, const char *name, enum nl80211_iftype type)
{
	int ret = _FAIL;

	if (rtw_os_ndev_alloc(adapter, type) != _SUCCESS)
		goto exit;

	if (rtw_os_ndev_register(adapter, name) != _SUCCESS)
		goto os_ndev_free;

	ret = _SUCCESS;

os_ndev_free:
	if (ret != _SUCCESS)
		rtw_os_ndev_free(adapter);
exit:
	return ret;
}

/**
 * rtw_os_ndev_deinit - Unregister and free OS layer net device and relating structures for @adapter
 * @adapter: the adapter on which this function applies
 */
void rtw_os_ndev_deinit(_adapter *adapter)
{
	rtw_os_ndev_unregister(adapter);
	rtw_os_ndev_free(adapter);
}

int rtw_os_ndevs_alloc(struct dvobj_priv *dvobj)
{
	int i, status = _SUCCESS;
	_adapter *adapter;

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_dev_res_alloc(dvobj) != _SUCCESS) {
		rtw_warn_on(1);
		return _FAIL;
	}
#endif

	for (i = 0; i < dvobj->iface_nums; i++) {

		if (i >= CONFIG_IFACE_NUMBER) {
			RTW_ERR("%s %d >= CONFIG_IFACE_NUMBER(%d)\n", __func__, i, CONFIG_IFACE_NUMBER);
			rtw_warn_on(1);
			continue;
		}

		adapter = dvobj->padapters[i];
		if (adapter && !adapter->pnetdev) {

			#ifdef CONFIG_RTW_DYNAMIC_NDEV
			if (!is_primary_adapter(adapter))
				continue;
			#endif

			status = rtw_os_ndev_alloc(adapter, NL80211_IFTYPE_STATION);
			if (status != _SUCCESS) {
				rtw_warn_on(1);
				break;
			}
		}
	}

	if (status != _SUCCESS) {
		for (; i >= 0; i--) {
			adapter = dvobj->padapters[i];
			if (adapter && adapter->pnetdev)
				rtw_os_ndev_free(adapter);
		}
	}

#if defined(CONFIG_IOCTL_CFG80211)
	if (status != _SUCCESS)
		rtw_cfg80211_dev_res_free(dvobj);
#endif

	return status;
}

void rtw_os_ndevs_free(struct dvobj_priv *dvobj)
{
	int i;
	_adapter *adapter = NULL;

	for (i = 0; i < dvobj->iface_nums; i++) {

		if (i >= CONFIG_IFACE_NUMBER) {
			RTW_ERR("%s %d >= CONFIG_IFACE_NUMBER(%d)\n", __func__, i, CONFIG_IFACE_NUMBER);
			rtw_warn_on(1);
			continue;
		}

		adapter = dvobj->padapters[i];

		if (adapter == NULL)
			continue;

		rtw_os_ndev_free(adapter);
	}

#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_dev_res_free(dvobj);
#endif
}

#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
u32 rtw_start_drv_threads(_adapter *padapter)
{
	u32 _status = _SUCCESS;

	RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(padapter));

#ifdef CONFIG_XMIT_THREAD_MODE
#if defined(CONFIG_SDIO_HCI)
	if (is_primary_adapter(padapter))
#endif
	{
		if (padapter->xmitThread == NULL) {
			RTW_INFO(FUNC_ADPT_FMT " start RTW_XMIT_THREAD\n", FUNC_ADPT_ARG(padapter));
			padapter->xmitThread = rtw_thread_start(rtw_xmit_thread, padapter, "RTW_XMIT_THREAD");
			if (padapter->xmitThread == NULL)
				_status = _FAIL;
		}
	}
#endif /* #ifdef CONFIG_XMIT_THREAD_MODE */

#ifdef CONFIG_RECV_THREAD_MODE
	if (is_primary_adapter(padapter)) {
		if (padapter->recvThread == NULL) {
			RTW_INFO(FUNC_ADPT_FMT " start RTW_RECV_THREAD\n", FUNC_ADPT_ARG(padapter));
			padapter->recvThread = rtw_thread_start(rtw_recv_thread, padapter, "RTW_RECV_THREAD");
			if (padapter->recvThread == NULL)
				_status = _FAIL;
		}
	}
#endif

	if (is_primary_adapter(padapter)) {
		if (padapter->cmdThread == NULL) {
			RTW_INFO(FUNC_ADPT_FMT " start RTW_CMD_THREAD\n", FUNC_ADPT_ARG(padapter));
			padapter->cmdThread = rtw_thread_start(rtw_cmd_thread, padapter, "RTW_CMD_THREAD");
			if (padapter->cmdThread == NULL)
				_status = _FAIL;
			else
				_rtw_down_sema(&padapter->cmdpriv.start_cmdthread_sema); /* wait for cmd_thread to run */
		}
	}


#ifdef CONFIG_EVENT_THREAD_MODE
	if (padapter->evtThread == NULL) {
		RTW_INFO(FUNC_ADPT_FMT " start RTW_EVENT_THREAD\n", FUNC_ADPT_ARG(padapter));
		padapter->evtThread = rtw_thread_start(event_thread, padapter, "RTW_EVENT_THREAD");
		if (padapter->evtThread == NULL)
			_status = _FAIL;
	}
#endif

	_status = rtw_intf_start_xmit_frame_thread(padapter);
	return _status;

}

void rtw_stop_drv_threads(_adapter *padapter)
{
	RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(padapter));
	if (is_primary_adapter(padapter))
		rtw_stop_cmd_thread(padapter);

#ifdef CONFIG_EVENT_THREAD_MODE
	if (padapter->evtThread) {
		_rtw_up_sema(&padapter->evtpriv.evt_notify);
		rtw_thread_stop(padapter->evtThread);
		padapter->evtThread = NULL;
	}
#endif

#ifdef CONFIG_XMIT_THREAD_MODE
	/* Below is to termindate tx_thread... */
#if defined(CONFIG_SDIO_HCI)
	/* Only wake-up primary adapter */
	if (is_primary_adapter(padapter))
#endif  /*SDIO_HCI */
	{
		if (padapter->xmitThread) {
			_rtw_up_sema(&padapter->xmitpriv.xmit_sema);
			rtw_thread_stop(padapter->xmitThread);
			padapter->xmitThread = NULL;
		}
	}
#endif

#ifdef CONFIG_RECV_THREAD_MODE
	if (is_primary_adapter(padapter) && padapter->recvThread) {
		/* Below is to termindate rx_thread... */
		_rtw_up_sema(&padapter->recvpriv.recv_sema);
		rtw_thread_stop(padapter->recvThread);
		padapter->recvThread = NULL;
	}
#endif

	/*rtw_hal_stop_thread(padapter);*/
	rtw_intf_cancel_xmit_frame_thread(padapter);

}
#endif

#ifdef RTW_PHL_DBG_CMD
void reset_txforce_para(_adapter *padapter);
#endif

u8 rtw_init_default_value(_adapter *padapter)
{
	u8 ret  = _SUCCESS;
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	void *phl = padapter->dvobj->phl;

	/* xmit_priv */
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	/* pxmitpriv->rts_thresh = pregistrypriv->rts_thresh; */
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;

	/* security_priv */
	/* rtw_get_encrypt_decrypt_from_registrypriv(padapter); */
	psecuritypriv->binstallGrpkey = _FAIL;
#ifdef CONFIG_GTK_OL
	psecuritypriv->binstallKCK_KEK = _FAIL;
#endif /* CONFIG_GTK_OL */
	psecuritypriv->sw_encrypt = pregistrypriv->software_encrypt;
	psecuritypriv->sw_decrypt = pregistrypriv->software_decrypt;

	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; /* open system */
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;
#ifdef CONFIG_CONCURRENT_MODE
	psecuritypriv->dot118021x_bmc_cam_id = INVALID_SEC_MAC_CAM_ID;
#endif

	/* pwrctrl_priv */

	/* registry_priv */
	rtw_init_registrypriv_dev_network(padapter);
	rtw_update_registrypriv_dev_network(padapter);

	pregistrypriv->wireless_mode &= rtw_hw_get_wireless_mode(adapter_to_dvobj(padapter));
	pregistrypriv->band_type &= rtw_hw_get_band_type(adapter_to_dvobj(padapter));
	/*init fw_psmode_iface_id*/
	adapter_to_pwrctl(padapter)->fw_psmode_iface_id = 0xff;


	/* misc. */
	padapter->bLinkInfoDump = 0;
	padapter->bNotifyChannelChange = _FALSE;
#ifdef CONFIG_P2P
	padapter->bShowGetP2PState = 1;
#endif

	/* for debug purpose */
	padapter->fix_rate = NO_FIX_RATE;
	padapter->data_fb = 0;
	padapter->fix_bw = NO_FIX_BW;
	padapter->power_offset = 0;
	padapter->rsvd_page_offset = 0;
	padapter->rsvd_page_num = 0;
#ifdef CONFIG_AP_MODE
	padapter->bmc_tx_rate = pregistrypriv->bmc_tx_rate;
#endif
	padapter->driver_tx_bw_mode = pregistrypriv->tx_bw_mode;

	padapter->driver_ampdu_spacing = 0xFF;
	padapter->driver_rx_ampdu_factor =  0xFF;
	padapter->driver_rx_ampdu_spacing = 0xFF;
	padapter->driver_rx_amsdu_size = 0xF;
	padapter->fix_rx_ampdu_accept = RX_AMPDU_ACCEPT_INVALID;
	padapter->fix_rx_ampdu_size = RX_AMPDU_SIZE_INVALID;
#ifdef CONFIG_TX_AMSDU
	padapter->tx_amsdu = AMSDU_MAX_NUM;
	padapter->tx_amsdu_rate = 400;
#endif
	padapter->driver_tx_max_agg_num = 0xFFFF;
#ifdef DBG_RX_COUNTER_DUMP
	padapter->dump_rx_cnt_mode = 0;
	padapter->drv_rx_cnt_ok = 0;
	padapter->drv_rx_cnt_crcerror = 0;
	padapter->drv_rx_cnt_drop = 0;
#endif
#ifdef CONFIG_RTW_NAPI
	padapter->napi_state = NAPI_DISABLE;
#endif

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	ATOMIC_SET(&padapter->tbtx_tx_pause, _FALSE);
	ATOMIC_SET(&padapter->tbtx_remove_tx_pause, _FALSE);
	padapter->tbtx_capability = _TRUE;
#endif

#ifdef CONFIG_CTC_FEATURE
	padapter->registrypriv.wifi_mib.roaming_rssi_th1 = 30; /* rssi < 30 */
	padapter->registrypriv.wifi_mib.roaming_rssi_th2 = 60; /* rssi > 60 */
	padapter->registrypriv.wifi_mib.roaming_start_time = 60; /* 60 secs */
	padapter->registrypriv.wifi_mib.roaming_wait_time = 6; /* 6 secs */
#endif
#ifdef TX_BEAMFORMING
	padapter->registrypriv.wifi_mib.txbf = 1;
	padapter->registrypriv.wifi_mib.txbfer = 1;
	padapter->registrypriv.wifi_mib.txbfee = 1;
	padapter->registrypriv.wifi_mib.txbf_period = 100;
#endif

	padapter->registrypriv.wifi_mib.ldpc = 1;

#ifdef RTW_PHL_DBG_CMD
	reset_txforce_para(padapter);
#endif

#ifdef CONFIG_RTL8852A
	if (dvobj->ic_id == RTL8852A)
		rtw_phl_write32(phl, 0x220, 0xb62d98db); /* default power voltage calibration */
#endif

	/* default set duplicate rts on */
	padapter->cca_rts_mode = 3;

	return ret;
}

#ifdef CONFIG_DRV_FAKE_AP
extern void rtw_fakeap_work(struct work_struct *work);
extern void rtw_fakeap_bcn_timer_hdl(void*);
#endif /* CONFIG_DRV_FAKE_AP */

struct dvobj_priv *devobj_init(void)
{
	struct dvobj_priv *pdvobj = NULL;
	struct rf_ctl_t *rfctl;

	pdvobj = (struct dvobj_priv *)rtw_zmalloc(sizeof(*pdvobj));
	if (pdvobj == NULL)
		return NULL;

	rfctl = dvobj_to_rfctl(pdvobj);

	_rtw_mutex_init(&pdvobj->hw_init_mutex);
	_rtw_mutex_init(&pdvobj->setch_mutex);
	_rtw_mutex_init(&pdvobj->setbw_mutex);
	_rtw_mutex_init(&pdvobj->rf_read_reg_mutex);
	_rtw_mutex_init(&pdvobj->ioctrl_mutex);

#ifdef CONFIG_RTW_CUSTOMER_STR
	_rtw_mutex_init(&pdvobj->customer_str_mutex);
	_rtw_memset(pdvobj->customer_str, 0xFF, RTW_CUSTOMER_STR_LEN);
#endif

	pdvobj->processing_dev_remove = _FALSE;

	ATOMIC_SET(&pdvobj->disable_func, 0);
	/* move to phl */
	/* rtw_macid_ctl_init(&pdvobj->macid_ctl); */

	_rtw_spinlock_init(&pdvobj->cam_ctl.lock);
	_rtw_mutex_init(&pdvobj->cam_ctl.sec_cam_access_mutex);
#if defined(RTK_129X_PLATFORM) && defined(CONFIG_PCI_HCI)
	_rtw_spinlock_init(&pdvobj->io_reg_lock);
#endif

	rtw_init_timer(&(pdvobj->dynamic_chk_timer), rtw_dynamic_check_timer_handlder, pdvobj);
	rtw_init_timer(&(pdvobj->core_feature_sw_timer), rtw_sw_feature_timer_handlder, pdvobj);

#ifdef CONFIG_MCC_MODE
	_rtw_mutex_init(&(pdvobj->mcc_objpriv.mcc_mutex));
	_rtw_mutex_init(&(pdvobj->mcc_objpriv.mcc_tsf_req_mutex));
	_rtw_mutex_init(&(pdvobj->mcc_objpriv.mcc_dbg_reg_mutex));
	_rtw_spinlock_init(&pdvobj->mcc_objpriv.mcc_lock);
#endif /* CONFIG_MCC_MODE */

#ifdef CONFIG_RTW_NAPI_DYNAMIC
	pdvobj->en_napi_dynamic = 0;
#endif /* CONFIG_RTW_NAPI_DYNAMIC */

	_rtw_mutex_init(&rfctl->offch_mutex);

#if CONFIG_TXPWR_LIMIT
	_rtw_mutex_init(&rfctl->txpwr_lmt_mutex);
	_rtw_init_listhead(&rfctl->reg_exc_list);
	_rtw_init_listhead(&rfctl->txpwr_lmt_list);
#endif

#ifdef CONFIG_RTW_DRV_HAS_NVM
	rtw_nvm_init(pdvobj);
#endif /* CONFIG_RTW_DRV_HAS_NVM */

	pdvobj->scan_deny = _FALSE;
	rtw_load_dvobj_registry(pdvobj);

#ifdef CONFIG_DRV_FAKE_AP
	skb_queue_head_init(&pdvobj->fakeap.rxq);
	_init_workitem(&pdvobj->fakeap.work, rtw_fakeap_work, pdvobj);
	_init_timer(&pdvobj->fakeap.bcn_timer, rtw_fakeap_bcn_timer_hdl, pdvobj);
#endif /* CONFIG_DRV_FAKE_AP */

#ifdef CONFIG_AMSDU_HW_TIMER
	_rtw_spinlock_init(&pdvobj->amsdu_hw_timer_lock);
	_rtw_init_listhead(&pdvobj->amsdu_hw_timer_list);
#endif

	return pdvobj;

}

void devobj_deinit(struct dvobj_priv *pdvobj)
{
	if (!pdvobj)
		return;

#ifdef CONFIG_RTW_DRV_HAS_NVM
	rtw_nvm_deinit(pdvobj);
#endif /* CONFIG_RTW_DRV_HAS_NVM */

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_dev_res_free(pdvobj);
#endif

#ifdef CONFIG_MCC_MODE
	_rtw_mutex_free(&(pdvobj->mcc_objpriv.mcc_mutex));
	_rtw_mutex_free(&(pdvobj->mcc_objpriv.mcc_tsf_req_mutex));
	_rtw_mutex_free(&(pdvobj->mcc_objpriv.mcc_dbg_reg_mutex));
	_rtw_spinlock_free(&pdvobj->mcc_objpriv.mcc_lock);
#endif /* CONFIG_MCC_MODE */

	_rtw_mutex_free(&pdvobj->hw_init_mutex);

#ifdef CONFIG_RTW_CUSTOMER_STR
	_rtw_mutex_free(&pdvobj->customer_str_mutex);
#endif

	_rtw_mutex_free(&pdvobj->setch_mutex);
	_rtw_mutex_free(&pdvobj->setbw_mutex);
	_rtw_mutex_free(&pdvobj->rf_read_reg_mutex);
	_rtw_mutex_free(&pdvobj->ioctrl_mutex);
	/* move to phl */
	/* rtw_macid_ctl_deinit(&pdvobj->macid_ctl); */

	_rtw_spinlock_free(&pdvobj->cam_ctl.lock);
	_rtw_mutex_free(&pdvobj->cam_ctl.sec_cam_access_mutex);

#if defined(RTK_129X_PLATFORM) && defined(CONFIG_PCI_HCI)
	_rtw_spinlock_free(&pdvobj->io_reg_lock);
#endif

	rtw_mfree((u8 *)pdvobj, sizeof(*pdvobj));
}

inline u8 rtw_rtnl_lock_needed(struct dvobj_priv *dvobj)
{
	if (dvobj->rtnl_lock_holder && dvobj->rtnl_lock_holder == current)
		return 0;
	return 1;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26))
static inline int rtnl_is_locked(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
	if (unlikely(rtnl_trylock())) {
		rtnl_unlock();
#else
	if (unlikely(down_trylock(&rtnl_sem) == 0)) {
		up(&rtnl_sem);
#endif
		return 0;
	}
	return 1;
}
#endif

inline void rtw_set_rtnl_lock_holder(struct dvobj_priv *dvobj, _thread_hdl_ thd_hdl)
{
	rtw_warn_on(!rtnl_is_locked());

	if (!thd_hdl || rtnl_is_locked())
		dvobj->rtnl_lock_holder = thd_hdl;

	if (dvobj->rtnl_lock_holder && 0)
		RTW_INFO("rtnl_lock_holder: %s:%d\n", current->comm, current->pid);
}

u8 rtw_reset_drv_sw(_adapter *padapter)
{
	u8	ret8 = _SUCCESS;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	/* hal_priv */
	rtw_hw_cap_init(adapter_to_dvobj(padapter));

	RTW_ENABLE_FUNC(adapter_to_dvobj(padapter), DF_RX_BIT);
	RTW_ENABLE_FUNC(adapter_to_dvobj(padapter), DF_TX_BIT);

	padapter->bLinkInfoDump = 0;

	padapter->xmitpriv.tx_pkts = 0;
	padapter->recvpriv.rx_pkts = 0;

	pmlmepriv->LinkDetectInfo.bBusyTraffic = _FALSE;

	/* pmlmepriv->LinkDetectInfo.TrafficBusyState = _FALSE; */
	pmlmepriv->LinkDetectInfo.TrafficTransitionCount = 0;
	pmlmepriv->LinkDetectInfo.LowPowerTransitionCount = 0;

	_clr_fwstate_(pmlmepriv, WIFI_UNDER_SURVEY | WIFI_UNDER_LINKING);

#ifdef DBG_CONFIG_ERROR_DETECT
	if (is_primary_adapter(padapter))
		rtw_hal_sreset_reset_value(padapter);
#endif
	pwrctrlpriv->pwr_state_check_cnts = 0;

	/* mlmeextpriv */
	mlmeext_set_scan_state(&padapter->mlmeextpriv, SCAN_DISABLE);

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	rtw_set_signal_stat_timer(&padapter->recvpriv);
#endif

	return ret8;
}

u8 devobj_data_init(struct dvobj_priv *dvobj)
{
	u8 ret = _SUCCESS;

	dev_set_drv_stopped(dvobj);/*init*/
	/*init data of dvobj*/
	if (rtw_rfctl_init(dvobj) == _FAIL) {
		ret = _FAIL;
		goto exit;
	}
	rtw_rfctl_chplan_init(dvobj); /* expect TXPWR_LMT is ready */
	rtw_hw_cap_init(dvobj);

	RTW_ENABLE_FUNC(dvobj, DF_RX_BIT);
	RTW_ENABLE_FUNC(dvobj, DF_TX_BIT);

exit:
	return 	ret;
}

void devobj_data_deinit(struct dvobj_priv *dvobj)
{
}

u8 devobj_trx_resource_init(struct dvobj_priv *dvobj)
{
	u8 ret = _SUCCESS;

#ifdef CONFIG_USB_HCI
	ret = rtw_init_lite_xmit_resource(dvobj);
	if (ret == _FAIL)
		goto exit;
	ret = rtw_init_lite_recv_resource(dvobj);
	if (ret == _FAIL)
		goto exit;
#endif
	ret = rtw_init_cmd_priv(dvobj);
	if (ret == _FAIL) {
		RTW_ERR("%s rtw_init_cmd_priv failed\n", __func__);
		goto exit;
	}

exit:
	return 	ret;
}


void devobj_trx_resource_deinit(struct dvobj_priv *dvobj)
{
#ifdef CONFIG_USB_HCI
	rtw_free_lite_xmit_resource(dvobj);
	rtw_free_lite_recv_resource(dvobj);
#endif
	rtw_free_cmd_priv(dvobj);
}

#ifdef CONFIG_AMSDU_HW_TIMER
extern void rtw_amsdu_hw_timeout(unsigned long data);
extern void rtw_core_set_gt3(_adapter *padapter, u8 enable, long timeout);
void rtw_init_amsdu_hw_timer(_adapter *padapter)
{
	_tasklet *amsdu_xmit_task = &padapter->amsdu_xmit_tasklet;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

#if defined(CONFIG_CONCURRENT_MODE)
	if (is_primary_adapter(padapter))
#endif
	{
		padapter->amsdu_hw_timer_enable = 1;
		padapter->amsdu_hw_irq_cnt = 0;
		rtw_tasklet_init(amsdu_xmit_task, rtw_amsdu_hw_timeout,
			(unsigned long)dvobj);

		printk("amsdu tasklet init padapter=%p dvobj=%p\n",
				padapter, padapter->dvobj);
	}

	printk("padapter=%p dvobj=%p\n",
			padapter, padapter->dvobj);
}
#endif

#ifdef CONFIG_VW_REFINE
void rtw_init_swq(_adapter *padapter)
{
	u8 i = 0;
	_tasklet *swq_xmit_task = &padapter->swq_xmit_tasklet;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < MAX_SKB_XMIT_QUEUE; i++) {
		skb_queue_head_init(&padapter->skb_xmit_queue[i]);
		padapter->swq_psta[i] = NULL;
		padapter->skb_que_ts[i] = 10000;
#ifdef CONFIG_SWQ_SKB_ARRAY
		padapter->swq_skb_array[i].wptr = 0;
		padapter->swq_skb_array[i].rptr = 0;
#endif
	}

	ATOMIC_SET(&padapter->skb_xmit_queue_len, 0);

#if defined(CONFIG_CONCURRENT_MODE)
	if (is_primary_adapter(padapter))
#endif
	{
		rtw_tasklet_init(swq_xmit_task, rtl8192cd_swq_timeout_g6,
			(unsigned long)dvobj);

#ifdef CONFIG_SWQ_SKB_ARRAY
		_rtw_init_queue(&dvobj->skb_q);
#endif
		printk("swq tasklet init padapter=%p dvobj=%p\n",
				padapter, padapter->dvobj);
	}

	printk("padapter=%p dvobj=%p\n",
			padapter, padapter->dvobj);
}
#endif


u8 rtw_init_drv_sw(_adapter *padapter)
{
	u8	ret8 = _SUCCESS;

#ifdef CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
	struct rtw_wdev_priv *pwdev_priv = adapter_wdev_data(padapter);
#endif

#if defined(WFO_VIRT_RECEIVER)
	alloc_wfo_mib(padapter);
#endif /* WFO_VIRT_RECEIVER */

	ret8 = rtw_init_default_value(padapter);/*load registrypriv value*/

	if ((rtw_init_evt_priv(&padapter->evtpriv)) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	if (rtw_init_mlme_priv(padapter) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

#ifdef CONFIG_P2P
	rtw_init_wifidirect_timers(padapter);
	init_wifidirect_info(padapter, P2P_ROLE_DISABLE);
	reset_global_wifidirect_info(padapter);
	#ifdef CONFIG_IOCTL_CFG80211
	rtw_init_cfg80211_wifidirect_info(padapter);
	#endif
#ifdef CONFIG_WFD
	if (rtw_init_wifi_display_info(padapter) == _FAIL)
		RTW_ERR("Can't init init_wifi_display_info\n");
#endif
#endif /* CONFIG_P2P */

	if (init_mlme_ext_priv(padapter) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

#ifdef CONFIG_TDLS
	if (rtw_init_tdls_info(padapter) == _FAIL) {
		RTW_INFO("Can't rtw_init_tdls_info\n");
		ret8 = _FAIL;
		goto exit;
	}
#endif /* CONFIG_TDLS */

#ifdef CONFIG_RTW_MESH
	rtw_mesh_cfg_init(padapter);
#endif

	if (_rtw_init_xmit_priv(&padapter->xmitpriv, padapter) == _FAIL) {
		RTW_INFO("Can't _rtw_init_xmit_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	if (_rtw_init_recv_priv(&padapter->recvpriv, padapter) == _FAIL) {
		RTW_INFO("Can't _rtw_init_recv_priv\n");
		ret8 = _FAIL;
		goto exit;
	}
	/* add for CONFIG_IEEE80211W, none 11w also can use */
	_rtw_spinlock_init(&padapter->security_key_mutex);

	/* We don't need to memset padapter->XXX to zero, because adapter is allocated by rtw_zvmalloc(). */
	/* _rtw_memset((unsigned char *)&padapter->securitypriv, 0, sizeof (struct security_priv)); */

	if (_rtw_init_sta_priv(&padapter->stapriv) == _FAIL) {
		RTW_INFO("Can't _rtw_init_sta_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	padapter->setband = WIFI_FREQUENCY_BAND_AUTO;
	padapter->fix_rate = NO_FIX_RATE;
	padapter->power_offset = 0;
	padapter->rsvd_page_offset = 0;
	padapter->rsvd_page_num = 0;

	padapter->data_fb = 0;
	padapter->fix_rx_ampdu_accept = RX_AMPDU_ACCEPT_INVALID;
	padapter->fix_rx_ampdu_size = RX_AMPDU_SIZE_INVALID;
#ifdef DBG_RX_COUNTER_DUMP
	padapter->dump_rx_cnt_mode = 0;
	padapter->drv_rx_cnt_ok = 0;
	padapter->drv_rx_cnt_crcerror = 0;
	padapter->drv_rx_cnt_drop = 0;
#endif

	rtw_init_pwrctrl_priv(padapter);

	/* _rtw_memset((u8 *)&padapter->qospriv, 0, sizeof (struct qos_priv)); */ /* move to mlme_priv */

#if 0//def CONFIG_MP_INCLUDED
	{ /* it should be to dymonic for switch normal/MP mode, modify temporary. */
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		struct mp_priv *pmppriv;

		if (init_mp_priv(padapter) == _FAIL)
				RTW_INFO("%s: initialize MP private data Fail!\n", __func__);
			if (!dvobj_get_primary_adapter(dvobj)) {
				RTW_INFO("%s primary adapter == NULL !!\n", __func__);
				goto exit;
			} else {
				padapter = dvobj_get_primary_adapter(dvobj);
			}
			pmppriv = &padapter->mppriv;
			pmppriv->phl_mp_test_mode = 0;
			rtw_test_module_init(dvobj->phl_com);  /* temp for modify , it should be wrapper to phl api*/
			RTW_INFO("%s ,test_module_init done, rtw_phl_test_init for MP sub mod\n", __func__);
	}
#endif


#ifdef CONFIG_WAPI_SUPPORT
	padapter->WapiSupport = true; /* set true temp, will revise according to Efuse or Registry value later. */
	rtw_wapi_init(padapter);
#endif

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	rtw_wapi_init(padapter);
#endif

#ifdef CONFIG_BR_EXT
	_rtw_spinlock_init(&padapter->br_ext_lock);
#endif /* CONFIG_BR_EXT */

#ifdef CONFIG_BEAMFORMING
	rtw_bf_init(padapter);
#endif /* CONFIG_BEAMFORMING */

#ifdef CONFIG_WFA_OFDMA_Logo_Test
	rtw_init_ofdma_timer(padapter);
#endif

#ifdef CONFIG_RTW_80211K
	rtw_init_rm(padapter);
#endif

#ifdef CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
	_rtw_memset(pwdev_priv->pno_mac_addr, 0xFF, ETH_ALEN);
#endif

#ifdef CONFIG_RTW_SW_LED
	rtw_init_led_priv(padapter);
#endif

#ifdef CONFIG_VW_REFINE
	rtw_init_swq(padapter);
#endif

#ifdef CONFIG_AMSDU_HW_TIMER
	rtw_init_amsdu_hw_timer(padapter);
#endif

#ifdef CONFIG_RTW_MULTI_AP
	core_map_cfg_init(padapter);
#endif

#ifdef CONFIG_DFS_MASTER
	rtw_init_dfs_region(padapter);
#endif

#ifdef CONFIG_RTW_DBG_TX_MGNT
	padapter->dbg_tx_mgnt = CONFIG_RTW_DBG_TX_MGNT;
#endif

	rtw_init_dscp_table(padapter);

exit:

	return ret8;

}

#ifdef CONFIG_WOWLAN
void rtw_cancel_dynamic_chk_timer(_adapter *padapter)
{
	_cancel_timer_ex(&adapter_to_dvobj(padapter)->dynamic_chk_timer);
}
#endif

void rtw_cancel_all_timer(_adapter *padapter)
{

	_cancel_timer_ex(&padapter->mlmepriv.assoc_timer);

	_cancel_timer_ex(&padapter->mlmepriv.scan_to_timer);

#ifdef SBWC
	_cancel_timer_ex(&padapter->SBWC_timer);
#endif

#ifdef RTW_STA_BWC
	_cancel_timer_ex(&padapter->sta_bwc_timer);
#endif

#ifdef CONFIG_TX_DEFER
	_cancel_timer_ex(&padapter->xmitpriv.tx_defer_timer);
#endif

#ifdef GBWC
	if (rtw_get_intf_type(padapter) == RTW_HCI_PCIE)
	{
		_cancel_timer_ex(&padapter->GBWC_timer);
		padapter->GBWC_timer_alive = 0;
	}
#endif

#ifdef CONFIG_DFS_MASTER
	_cancel_timer_ex(&adapter_to_rfctl(padapter)->radar_detect_timer);
#endif

	_cancel_timer_ex(&adapter_to_dvobj(padapter)->dynamic_chk_timer);

#ifdef CONFIG_POWER_SAVING
	_cancel_timer_ex(&(adapter_to_pwrctl(padapter)->pwr_state_check_timer));
#endif

#ifdef CONFIG_TX_AMSDU
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_bk_timer);
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_be_timer);
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_vo_timer);
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_vi_timer);
#endif

#ifdef CONFIG_IOCTL_CFG80211
#ifdef CONFIG_P2P
	_cancel_timer_ex(&padapter->cfg80211_wdinfo.remain_on_ch_timer);
#endif /* CONFIG_P2P */
#endif /* CONFIG_IOCTL_CFG80211 */

#ifdef CONFIG_SET_SCAN_DENY_TIMER
	_cancel_timer_ex(&padapter->mlmepriv.set_scan_deny_timer);
	rtw_clear_scan_deny(padapter);
#endif

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	_cancel_timer_ex(&padapter->recvpriv.signal_stat_timer);
#endif

#ifdef CONFIG_LPS_RPWM_TIMER
	_cancel_timer_ex(&(adapter_to_pwrctl(padapter)->pwr_rpwm_timer));
#endif /* CONFIG_LPS_RPWM_TIMER */

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	_cancel_timer_ex(&padapter->mlmeextpriv.tbtx_xmit_timer);
	_cancel_timer_ex(&padapter->mlmeextpriv.tbtx_token_dispatch_timer);
#endif

#ifdef CONFIG_RTW_MBO
	_cancel_timer_ex(&padapter->mlmepriv.mbopriv.bss_termination_timer);
	_cancel_timer_ex(&padapter->mlmepriv.mbopriv.bss_termination_dur_timer);
#endif

#ifdef CONFIG_PLATFORM_FS_MX61
	msleep(50);
#endif
}

u8 rtw_free_drv_sw(_adapter *padapter)
{

#ifdef CONFIG_WAPI_SUPPORT
	rtw_wapi_free(padapter);
#endif

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	rtw_wapi_init(padapter);
#endif


	/* we can call rtw_p2p_enable here, but: */
	/* 1. rtw_p2p_enable may have IO operation */
	/* 2. rtw_p2p_enable is bundled with wext interface */
	#ifdef CONFIG_P2P
	{
		struct wifidirect_info *pwdinfo = &padapter->wdinfo;
		if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
			_cancel_timer_ex(&pwdinfo->find_phase_timer);
			_cancel_timer_ex(&pwdinfo->restore_p2p_state_timer);
			_cancel_timer_ex(&pwdinfo->pre_tx_scan_timer);
			#ifdef CONFIG_CONCURRENT_MODE
			_cancel_timer_ex(&pwdinfo->ap_p2p_switch_timer);
			#endif /* CONFIG_CONCURRENT_MODE */
			rtw_p2p_set_state(pwdinfo, P2P_STATE_NONE);
		}
	}
	#endif
	/* add for CONFIG_IEEE80211W, none 11w also can use */
	_rtw_spinlock_free(&padapter->security_key_mutex);

#ifdef CONFIG_BR_EXT
	_rtw_spinlock_free(&padapter->br_ext_lock);
#endif /* CONFIG_BR_EXT */

	free_mlme_ext_priv(&padapter->mlmeextpriv);

#ifdef CONFIG_TDLS
	/* rtw_free_tdls_info(&padapter->tdlsinfo); */
#endif /* CONFIG_TDLS */

#ifdef CONFIG_RTW_80211K
	rtw_free_rm_priv(padapter);
#endif

#ifdef CONFIG_WFA_OFDMA_Logo_Test
	rtw_fini_ofdma_timer(padapter);
#endif

	rtw_free_evt_priv(&padapter->evtpriv);

	rtw_free_mlme_priv(&padapter->mlmepriv);

	if (is_primary_adapter(padapter))
		rtw_rfctl_deinit(adapter_to_dvobj(padapter));

	/* free_io_queue(padapter); */

	_rtw_free_xmit_priv(&padapter->xmitpriv);

	_rtw_free_sta_priv(&padapter->stapriv); /* will free bcmc_stainfo here */

	_rtw_free_recv_priv(&padapter->recvpriv);

	rtw_free_pwrctrl_priv(padapter);

#ifdef CONFIG_RTW_MULTI_AP
	core_map_cfg_free(padapter);
#endif /* CONFIG_RTW_MULTI_AP */

	/* rtw_mfree((void *)padapter, sizeof (padapter)); */

	return _SUCCESS;

}
void rtw_drv_stop_prim_iface(_adapter *adapter)
{
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(adapter);
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
		rtw_disassoc_cmd(adapter, 0, RTW_CMDF_DIRECTLY);

#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(adapter) || MLME_IS_MESH(adapter)) {
		free_mlme_ap_info(adapter);
		#ifdef CONFIG_HOSTAPD_MLME
		hostapd_mode_unload(adapter);
		#endif
	}
#endif

	RTW_INFO("==> "FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(adapter));

	if (adapter->netif_up == _TRUE) {
		#ifdef CONFIG_XMIT_ACK
		if (adapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&adapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
		#endif

		if (adapter->run_cmd_en) {
			// disable cmd state before deinit data structure
			if (rtw_set_run_cmd_en(adapter, 0, RTW_CMDF_WAIT_ACK) != _SUCCESS)
				RTW_INFO("[%s %d] rtw_set_run_cmd_en fail, value = %u\n", __FUNCTION__, __LINE__, adapter->run_cmd_en);
			else
				RTW_INFO("[%s %d] rtw_set_run_cmd_en success, value = %u\n", __FUNCTION__, __LINE__, adapter->run_cmd_en);
		}

		rtw_hw_iface_deinit(adapter);
		adapter->netif_up = _FALSE;
	}
	#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	rtw_stop_drv_threads(adapter);

	if (ATOMIC_READ(&(pcmdpriv->cmdthd_running)) == _TRUE) {
		RTW_ERR("cmd_thread not stop !!\n");
		rtw_warn_on(1);
	}
	#endif

	/* check the status of IPS */
	if (rtw_hal_check_ips_status(adapter) == _TRUE || pwrctl->rf_pwrstate == rf_off) { /* check HW status and SW state */
		RTW_PRINT("%s: driver in IPS-FWLPS\n", __func__);
		pdbgpriv->dbg_dev_unload_inIPS_cnt++;
	} else
		RTW_PRINT("%s: driver not in IPS\n", __func__);

	rtw_cancel_all_timer(adapter);
	_cancel_timer_ex(&adapter_to_dvobj(adapter)->core_feature_sw_timer);
	RTW_INFO("<== "FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(adapter));

}

#ifdef CONFIG_CONCURRENT_MODE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops rtw_netdev_vir_if_ops = {
	.ndo_init = rtw_ndev_init,
	.ndo_uninit = rtw_ndev_uninit,
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	.ndo_select_queue	= rtw_select_queue,
#endif
};
#endif

static void rtw_hook_vir_if_ops(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
	ndev->netdev_ops = &rtw_netdev_vir_if_ops;
#else
	ndev->init = rtw_ndev_init;
	ndev->uninit = rtw_ndev_uninit;
	ndev->open = netdev_open;
	ndev->stop = netdev_close;

	ndev->set_mac_address = rtw_net_set_mac_address;
#endif
}
static _adapter *rtw_drv_add_vir_if(struct dvobj_priv *dvobj)
{
	int res = _FAIL;
	_adapter *padapter = NULL;
	_adapter *primary_padapter = dvobj_get_primary_adapter(dvobj);
	u8 mac[ETH_ALEN];

	/****** init adapter ******/
	padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter));
	if (padapter == NULL)
		goto exit;

	_rtw_memcpy(padapter, primary_padapter, sizeof(_adapter));

	if (rtw_load_registry(padapter) != _SUCCESS)
		goto free_adapter;

	padapter->netif_up = _FALSE;
	padapter->dir_dev = NULL;
	padapter->dir_odm = NULL;
	padapter->up_time = 0;
	padapter->sta_dump_to = 0;
	padapter->sta_dump_bitmap = 0xffff;
	padapter->sta_ru_dump_bitmap = 0x1fff;

	/*set adapter_type/iface type*/
	padapter->isprimary = _FALSE;
	padapter->adapter_type = VIRTUAL_ADAPTER;

	/****** hook vir if into dvobj ******/
	padapter->iface_id = dvobj->iface_nums;
	dvobj->padapters[dvobj->iface_nums++] = padapter;

	/*init drv data*/
	if (rtw_init_drv_sw(padapter) != _SUCCESS)
		goto free_drv_sw;


	/*get mac address from primary_padapter*/
	_rtw_memcpy(mac, adapter_mac_addr(primary_padapter), ETH_ALEN);

	/*
	* If the BIT1 is 0, the address is universally administered.
	* If it is 1, the address is locally administered
	*/
	mac[0] |= BIT(1);
	if (padapter->iface_id > IFACE_ID1)
		mac[0] ^= ((padapter->iface_id)<<2);

	_rtw_memcpy(adapter_mac_addr(padapter), mac, ETH_ALEN);

	RTW_INFO("%s if%d mac_addr : "MAC_FMT"\n", __func__, padapter->iface_id + 1, MAC_ARG(adapter_mac_addr(padapter)));
#ifdef CONFIG_P2P
	rtw_init_wifidirect_addrs(padapter, adapter_mac_addr(padapter), adapter_mac_addr(padapter));
#endif
#ifdef CONFIG_RTW_SW_LED
	rtw_led_set_ctl_en_mask_virtual(padapter);
	rtw_led_set_iface_en(padapter, 1);
#endif
#if defined(PLATFORM_ECOS) && (defined(CONFIG_WFO_VIRT_MODULE) || defined(CONFIG_RTK_WFO_NO_VIRT))
	mutex_init(&padapter->cfg_mutex);
	padapter->cfg_owner_id = 0;
#endif
	res = _SUCCESS;

free_drv_sw:
	if (res != _SUCCESS && padapter)
		rtw_free_drv_sw(padapter);
free_adapter:
	if (res != _SUCCESS && padapter) {
		rtw_vmfree((u8 *)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}
u8 rtw_drv_add_vir_ifaces(struct dvobj_priv *dvobj)
{
	u8 i;
	u8 rst = _FAIL;

	if (dvobj->virtual_iface_num > (CONFIG_IFACE_NUMBER - 1))
		dvobj->virtual_iface_num = (CONFIG_IFACE_NUMBER - 1);

	#if defined(CONFIG_RTW_DEBUG_MBSSID_VAP) && (CONFIG_RTW_DEBUG_MBSSID_VAP > 0)
	RTW_PRINT("Creating #%u virtual interface(s).\n",
	          dvobj->virtual_iface_num);
	#endif

	for (i = 0; i < dvobj->virtual_iface_num; i++) {
		if (rtw_drv_add_vir_if(dvobj) == NULL) {
			RTW_ERR("rtw_drv_add_vir_if failed! (%d)\n", i);
			goto _exit;
		}
	}
	rst = _SUCCESS;
_exit:
	return rst;
}

static void rtw_drv_stop_vir_if(_adapter *padapter)
{
	struct net_device *pnetdev = NULL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (padapter == NULL)
		return;
	RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(padapter));

	pnetdev = padapter->pnetdev;

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
		rtw_disassoc_cmd(padapter, 0, RTW_CMDF_DIRECTLY);

#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) {
		free_mlme_ap_info(padapter);
		#ifdef CONFIG_HOSTAPD_MLME
		hostapd_mode_unload(padapter);
		#endif
	}
#endif

	if (padapter->netif_up == _TRUE) {
		#ifdef CONFIG_XMIT_ACK
		if (padapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
		#endif

		if (padapter->run_cmd_en) {
			// disable cmd state before deinit data structure
			if (rtw_set_run_cmd_en(padapter, 0, RTW_CMDF_WAIT_ACK) != _SUCCESS)
				RTW_INFO("[%s %d] rtw_set_run_cmd_en fail, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);
			else
				RTW_INFO("[%s %d] rtw_set_run_cmd_en success, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);
		}

		rtw_hw_iface_deinit(padapter);
		padapter->netif_up = _FALSE;
	}
	#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	rtw_stop_drv_threads(padapter);
	#endif
	/* cancel timer after thread stop */
	rtw_cancel_all_timer(padapter);
}
void rtw_drv_stop_vir_ifaces(struct dvobj_priv *dvobj)
{
	int i;

	for (i = VIF_START_ID; i < dvobj->iface_nums; i++)
		rtw_drv_stop_vir_if(dvobj->padapters[i]);
}

static void rtw_drv_free_vir_if(_adapter *padapter)
{
	if (padapter == NULL)
		return;

	RTW_INFO(FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(padapter));
	rtw_free_drv_sw(padapter);

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndev_free(padapter);

	rtw_vmfree((u8 *)padapter, sizeof(_adapter));
}
void rtw_drv_free_vir_ifaces(struct dvobj_priv *dvobj)
{
	int i;

	for (i = VIF_START_ID; i < dvobj->iface_nums; i++)
		rtw_drv_free_vir_if(dvobj->padapters[i]);
}


#endif /*end of CONFIG_CONCURRENT_MODE*/

/* IPv4, IPv6 IP addr notifier */
static int rtw_inetaddr_notifier_call(struct notifier_block *nb,
				      unsigned long action, void *data)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)data;
	struct net_device *ndev;
	struct mlme_ext_priv *pmlmeext = NULL;
	struct mlme_ext_info *pmlmeinfo = NULL;
	_adapter *adapter = NULL;

	if (!ifa || !ifa->ifa_dev || !ifa->ifa_dev->dev)
		return NOTIFY_DONE;

	ndev = ifa->ifa_dev->dev;

	if (!is_rtw_ndev(ndev))
		return NOTIFY_DONE;

	adapter = (_adapter *)rtw_netdev_priv(ifa->ifa_dev->dev);

	if (adapter == NULL)
		return NOTIFY_DONE;

	pmlmeext = &adapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	switch (action) {
	case NETDEV_UP:
		_rtw_memcpy(pmlmeinfo->ip_addr, &ifa->ifa_address,
					RTW_IP_ADDR_LEN);
		RTW_DBG("%s[%s]: up IP: %pI4\n", __func__,
					ifa->ifa_label, pmlmeinfo->ip_addr);
	break;
	case NETDEV_DOWN:
		_rtw_memset(pmlmeinfo->ip_addr, 0, RTW_IP_ADDR_LEN);
		RTW_DBG("%s[%s]: down IP: %pI4\n", __func__,
					ifa->ifa_label, pmlmeinfo->ip_addr);
	break;
	default:
		RTW_DBG("%s: default action\n", __func__);
	break;
	}
	return NOTIFY_DONE;
}

#ifdef CONFIG_IPV6
static int rtw_inet6addr_notifier_call(struct notifier_block *nb,
				       unsigned long action, void *data)
{
	struct inet6_ifaddr *inet6_ifa = data;
	struct net_device *ndev;
	struct pwrctrl_priv *pwrctl = NULL;
	struct mlme_ext_priv *pmlmeext = NULL;
	struct mlme_ext_info *pmlmeinfo = NULL;
	_adapter *adapter = NULL;

	if (!inet6_ifa || !inet6_ifa->idev || !inet6_ifa->idev->dev)
		return NOTIFY_DONE;

	ndev = inet6_ifa->idev->dev;

	if (!is_rtw_ndev(ndev))
		return NOTIFY_DONE;

	adapter = (_adapter *)rtw_netdev_priv(inet6_ifa->idev->dev);

	if (adapter == NULL)
		return NOTIFY_DONE;

	pmlmeext =  &adapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;
	pwrctl = adapter_to_pwrctl(adapter);

	pmlmeext = &adapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	switch (action) {
	case NETDEV_UP:
#ifdef CONFIG_WOWLAN
		pwrctl->wowlan_ns_offload_en = _TRUE;
#endif
		_rtw_memcpy(pmlmeinfo->ip6_addr, &inet6_ifa->addr,
					RTW_IPv6_ADDR_LEN);
		RTW_DBG("%s: up IPv6 addrs: %pI6\n", __func__,
					pmlmeinfo->ip6_addr);
		break;
	case NETDEV_DOWN:
#ifdef CONFIG_WOWLAN
		pwrctl->wowlan_ns_offload_en = _FALSE;
#endif
		_rtw_memset(pmlmeinfo->ip6_addr, 0, RTW_IPv6_ADDR_LEN);
		RTW_DBG("%s: down IPv6 addrs: %pI6\n", __func__,
					pmlmeinfo->ip6_addr);
		break;
	default:
		RTW_DBG("%s: default action\n", __func__);
		break;
	}
	return NOTIFY_DONE;
}
#endif

static struct notifier_block rtw_inetaddr_notifier = {
	.notifier_call = rtw_inetaddr_notifier_call
};

#ifdef CONFIG_IPV6
static struct notifier_block rtw_inet6addr_notifier = {
	.notifier_call = rtw_inet6addr_notifier_call
};
#endif

void rtw_inetaddr_notifier_register(void)
{
	RTW_INFO("%s\n", __func__);
	register_inetaddr_notifier(&rtw_inetaddr_notifier);
#ifdef CONFIG_IPV6
	register_inet6addr_notifier(&rtw_inet6addr_notifier);
#endif
}

void rtw_inetaddr_notifier_unregister(void)
{
	RTW_INFO("%s\n", __func__);
	unregister_inetaddr_notifier(&rtw_inetaddr_notifier);
#ifdef CONFIG_IPV6
	unregister_inet6addr_notifier(&rtw_inet6addr_notifier);
#endif
}

int rtw_os_ndevs_register(struct dvobj_priv *dvobj)
{
	int i, status = _SUCCESS;
	int virif_num = 0;
	char virif_ifname[15];
	struct registry_priv *regsty = dvobj_to_regsty(dvobj);
	_adapter *adapter;

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_dev_res_register(dvobj) != _SUCCESS) {
		rtw_warn_on(1);
		return _FAIL;
	}
#endif

	for (i = 0; i < dvobj->iface_nums; i++) {

		if (i >= CONFIG_IFACE_NUMBER) {
			RTW_ERR("%s %d >= CONFIG_IFACE_NUMBER(%d)\n", __func__, i, CONFIG_IFACE_NUMBER);
			rtw_warn_on(1);
			continue;
		}

		adapter = dvobj->padapters[i];
		if (adapter) {
			char *name;

			#ifdef CONFIG_RTW_DYNAMIC_NDEV
			if (!is_primary_adapter(adapter))
				continue;
			#endif

			#if defined(WFO_VIRT_RECEIVER)
			wfo_set_devname(adapter, regsty);
			#endif /* WFO_VIRT_RECEIVER */

			if (!is_primary_adapter(adapter)){
#if defined(CONFIG_2G_ON_WLAN0)
				if(i == (dvobj->iface_nums - 1))
					snprintf(virif_ifname, sizeof(virif_ifname), "wlan1-vxd");
				else 
					snprintf(virif_ifname, sizeof(virif_ifname), "wlan1-vap%d", virif_num);
#else
				if(i == (dvobj->iface_nums - 1))
					snprintf(virif_ifname, sizeof(virif_ifname), "wlan0-vxd");
				else 
					snprintf(virif_ifname, sizeof(virif_ifname), "wlan0-vap%d", virif_num);
#endif /* CONFIG_2G_ON_WLAN0 */
				name = virif_ifname;
				virif_num++;
			}
			else {
				if (dvobj->dev_id == IFACE_ID0)
					name = regsty->ifname;
				else if (dvobj->dev_id == IFACE_ID1)
					name = regsty->if2name;
				else
					name = "wlan%d";
			}
			status = rtw_os_ndev_register(adapter, name);

			if (status != _SUCCESS) {
				rtw_warn_on(1);
				break;
			}
		}
	}

	if (status != _SUCCESS) {
		for (; i >= 0; i--) {
			adapter = dvobj->padapters[i];
			if (adapter)
				rtw_os_ndev_unregister(adapter);
		}
	}

#if defined(CONFIG_IOCTL_CFG80211)
	if (status != _SUCCESS)
		rtw_cfg80211_dev_res_unregister(dvobj);
#endif
	return status;
}

void rtw_os_ndevs_unregister(struct dvobj_priv *dvobj)
{
	int i;
	_adapter *adapter = NULL;

	for (i = 0; i < dvobj->iface_nums; i++) {
		adapter = dvobj->padapters[i];

		if (adapter == NULL)
			continue;

		rtw_os_ndev_unregister(adapter);
	}

#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_dev_res_unregister(dvobj);
#endif
}

/**
 * rtw_os_ndevs_init - Allocate and register OS layer net devices and relating structures for @dvobj
 * @dvobj: the dvobj on which this function applies
 *
 * Returns:
 * _SUCCESS or _FAIL
 */
int rtw_os_ndevs_init(struct dvobj_priv *dvobj)
{
	int ret = _FAIL;

	if (rtw_os_ndevs_alloc(dvobj) != _SUCCESS)
		goto exit;

	if (rtw_os_ndevs_register(dvobj) != _SUCCESS)
		goto os_ndevs_free;

	ret = _SUCCESS;

os_ndevs_free:
	if (ret != _SUCCESS)
		rtw_os_ndevs_free(dvobj);
exit:
	return ret;
}

/**
 * rtw_os_ndevs_deinit - Unregister and free OS layer net devices and relating structures for @dvobj
 * @dvobj: the dvobj on which this function applies
 */
void rtw_os_ndevs_deinit(struct dvobj_priv *dvobj)
{
	rtw_os_ndevs_unregister(dvobj);
	rtw_os_ndevs_free(dvobj);
}

#ifdef CONFIG_BR_EXT
void netdev_br_init(struct net_device *netdev)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);

#ifdef WKARD_98D
	if (netdev->priv_flags & IFF_DONT_BRIDGE) {
		DBGP("Unable to be bridged !! Unlock for this iface !!\n");
		netdev->priv_flags &= ~(IFF_DONT_BRIDGE);
	}
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	rcu_read_lock();
#endif

	/* if(check_fwstate(pmlmepriv, WIFI_STATION_STATE|WIFI_ADHOC_STATE) == _TRUE) */
	{
		/* struct net_bridge	*br = netdev->br_port->br; */ /* ->dev->dev_addr; */
#ifndef WKARD_98D
		#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35))
		if (netdev->br_port)
		#else
		if (rcu_dereference(adapter->pnetdev->rx_handler_data))
		#endif
#endif
		{
	#ifdef PLATFORM_ECOS
			memcpy((void *)(adapter->br_mac), (void *)(netdev->rx_handler_data), ETH_ALEN);
	#else /* !PLATFORM_ECOS */
			struct net_device *br_netdev;

			DBGP("CONFIG_BR_EXT_BRNAME=%s \n", CONFIG_BR_EXT_BRNAME);

			br_netdev = rtw_get_bridge_ndev_by_name(CONFIG_BR_EXT_BRNAME);
			if (br_netdev) {

				DBGP("br_netdev->dev_addr=%pM \n", br_netdev->dev_addr);

				_rtw_memcpy(adapter->br_mac, br_netdev->dev_addr, ETH_ALEN);
				dev_put(br_netdev);
				RTW_INFO(FUNC_NDEV_FMT" bind bridge dev "NDEV_FMT"("MAC_FMT")\n"
					, FUNC_NDEV_ARG(netdev), NDEV_ARG(br_netdev), MAC_ARG(br_netdev->dev_addr));
			} else {
				RTW_INFO(FUNC_NDEV_FMT" can't get bridge dev by name \"%s\"\n"
					, FUNC_NDEV_ARG(netdev), CONFIG_BR_EXT_BRNAME);
			}
	#endif /* PLATFORM_ECOS */
		}

		adapter->ethBrExtInfo.addPPPoETag = 1;
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	rcu_read_unlock();
#endif
}
#endif /* CONFIG_BR_EXT */

/*FPGA_test*/
static int _drv_enable_trx(struct dvobj_priv *d)
{
	struct _ADAPTER *adapter;
	u32 status;


	adapter = dvobj_get_primary_adapter(d);
	if (adapter->netif_up == _FALSE) {
		status = rtw_mi_start_drv_threads(adapter);

		if (status == _FAIL) {
			RTW_ERR("%s: Start threads Failed!\n", __FUNCTION__);
			return -1;
		}
	}


	return 0;
}

void _init_ppdu_sts_filter(struct dvobj_priv *dvobj)
{
#if (defined(CONFIG_WIFI_DIAGNOSIS) || defined(DEBUG_MAP_UNASSOC)) && defined(CONFIG_PHL_RX_PSTS_PER_PKT)
	/* If HW supports Rx Drvinfo(PhyRpt) in RxDesc, do not need to enable ppdu sts per pkt feature */
	if (RTW_DEV_CAP_DISABLE == dvobj->phl_com->dev_cap.drv_info_sup) {
		/* If CONFIG_WIFI_DIAGNOSIS is defined, queue data frame with macid invalid */
		rtw_phl_init_ppdu_sts_para(dvobj->phl_com, true, false,
				(RTW_PHL_PSTS_FLTR_MGNT |
				RTW_PHL_PSTS_FLTR_CTRL |
				RTW_PHL_PSTS_FLTR_DATA |
				RTW_PHL_PSTS_FLTR_EXT_RSVD),
				true);
	}
#endif
}


static int _netdev_open(struct net_device *pnetdev)
{
	uint status;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	void *phl = padapter->dvobj->phl;

	RTW_INFO(FUNC_NDEV_FMT" start\n", FUNC_NDEV_ARG(pnetdev));

#if CONFIG_DFS
	rtw_init_dfs_region(padapter);
#endif

	if (!rtw_hw_is_init_completed(dvobj)) {
		dev_clr_surprise_removed(dvobj);
		dev_clr_drv_stopped(dvobj);
		RTW_ENABLE_FUNC(dvobj, DF_RX_BIT);
		RTW_ENABLE_FUNC(dvobj, DF_TX_BIT);
#ifdef CONFIG_DYN_FW_LOAD
		//FW external/internal setting
		if(padapter->registrypriv.wifi_mib.fw_ext)
		{
			//Debug and test mode
			dvobj->phl_com->dev_cap.fw_cap.fw_src = RTW_FW_SRC_EXTNAL;
		}
		else
		{
			dvobj->phl_com->dev_cap.fw_cap.fw_src = RTW_FW_SRC_INTNAL;
		}
#endif
		_init_ppdu_sts_filter(dvobj);

		//Turn on/off FW SW_MODE
		if(padapter->registrypriv.wifi_mib.txbf_mu
			|| padapter->registrypriv.wifi_mib.ofdma_enable
			|| padapter->registrypriv.wifi_mib.force_fw_tx)
		{
			dvobj->phl_com->dev_cap.fw_tx_mode = 1;
		}
		else
		{
			dvobj->phl_com->dev_cap.fw_tx_mode = 0;
		}
		status = rtw_hw_start(dvobj);
		if (status == _FAIL)
			goto netdev_open_error;

		if (0){
			_set_timer(&dvobj->dynamic_chk_timer, 2000);
		}
		#if 0 //ifndef CONFIG_PHL_ARCH
		_drv_enable_trx(dvobj);/*FPGA_test*/
		#endif
	}

	if (padapter->netif_up == _FALSE) {
		if (rtw_hw_iface_init(padapter) == _FAIL) {
			rtw_warn_on(1);
			goto netdev_open_error;
		}

		#ifdef CONFIG_RTW_NAPI
		if(padapter->napi_state == NAPI_DISABLE) {
			napi_enable(&padapter->napi);
			padapter->napi_state = NAPI_ENABLE;
		}
		#endif

		#ifdef CONFIG_IOCTL_CFG80211
		rtw_cfg80211_init_wdev_data(padapter);
		#endif
		/* rtw_netif_carrier_on(pnetdev); */ /* call this func when rtw_joinbss_event_callback return success */
		rtw_netif_wake_queue(pnetdev);

		#ifdef CONFIG_BR_EXT
#ifndef WKARD_98D
		if (is_primary_adapter(padapter))
#endif
			netdev_br_init(pnetdev);
		#endif /* CONFIG_BR_EXT */

		padapter->netif_up = _TRUE;
		padapter->up_time = 0;
		padapter->sta_dump_to = 0;
		padapter->sta_dump_bitmap = 0xffff;
		padapter->sta_ru_dump_bitmap = 0x1fff;

#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
		if (_is_all_other_iface_down(dvobj, padapter)) {
			printk("[%s] change tx_mode form %d to %d\n",
				__func__, dvobj->tx_mode, padapter->registrypriv.tx_mode);
			dvobj->tx_mode = padapter->registrypriv.tx_mode;
		}
#endif

		rtw_led_control(padapter, LED_CTL_UP_IDLE);
		if (is_primary_adapter(padapter))
			_set_timer(&dvobj->core_feature_sw_timer, 1000);
	}

	if (rtw_set_run_cmd_en(padapter, 1, RTW_CMDF_WAIT_ACK) != _SUCCESS)
		RTW_INFO("[%s %d] rtw_set_run_cmd_en fail, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);
	else
		RTW_INFO("[%s %d] rtw_set_run_cmd_en success, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);

	RTW_INFO(FUNC_NDEV_FMT" Success (netif_up=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->netif_up);

#if defined(RTW_WKARD_INTF_RESET) || defined(WKARD_CHANGE_IFACE)
{
	NDIS_802_11_NETWORK_INFRASTRUCTURE network_type;
	struct wireless_dev *rtw_wdev = padapter->rtw_wdev;
	int ret = _SUCCESS;

	network_type = nl80211_iftype_to_rtw_network_type(rtw_wdev->iftype);
	ret = rtw_pwr_wakeup(padapter);
	rtw_set_802_11_infrastructure_mode(padapter, network_type, 0);
	rtw_setopmode_cmd(padapter, network_type, RTW_CMDF_DIRECTLY);
}
#endif

{
	extern uint rtw_wifi_mode;
	/* page ctrl */
	if (rtw_wifi_mode == 1) {
		rtw_phl_write32(padapter->dvobj->phl, 0x8a10, 0x1da0080);
		rtw_phl_write32(padapter->dvobj->phl, 0x8a14, 0x1da0080);
		rtw_phl_write32(padapter->dvobj->phl, 0x8a18, 0x1da0080);
		rtw_phl_write32(padapter->dvobj->phl, 0x8a1c, 0x1da0080);
	}
#ifdef CONFIG_LMT_TXREQ
	if (rtw_wifi_mode == 1)
		padapter->lmt_txreq_enable = 1;
	else
		padapter->lmt_txreq_enable = 0;
#endif
}

	if (is_primary_adapter(padapter)) {
#ifdef CONFIG_LIFETIME_FEATURE
		if (padapter->registrypriv.wifi_mib.lifetime) {
			u16 us256 = RTL_MS_TO_256US(padapter->registrypriv.wifi_mib.lifetime);
			rtw_phl_set_lifetime(GET_HAL_INFO(padapter->dvobj), 1, us256);
		}
#endif
#ifdef POWER_PERCENT_ADJUSTMENT
		if (padapter->registrypriv.wifi_mib.powerpercent) {
			int diff_level = txpower_percent_to_level(padapter->registrypriv.wifi_mib.powerpercent);
			rtw_phl_set_ref_power(GET_HAL_INFO(padapter->dvobj), 0, diff_level);
		}

		if (padapter->registrypriv.wifi_mib.power_reference) {
			int ref = padapter->registrypriv.wifi_mib.power_reference;
			rtw_phl_set_ref_power(GET_HAL_INFO(padapter->dvobj), 0, (ref * 2));
		}
#endif
		/* set dig default mode from mib */
		rtw_phl_set_dig_opmode(GET_HAL_INFO(dvobj), padapter->registrypriv.wifi_mib.dig_opmode);
#ifdef CONFIG_AMSDU_HW_TIMER
		if (padapter->amsdu_hw_timer_enable)
			rtw_core_set_gt3(padapter, 1, 1000);
#endif

	}

#ifdef CONFIG_RTW_MANUAL_EDCA
	RTW_PRINT("%s, set manual edca para\n", __func__);
	if((MLME_IS_AP(padapter)) || (MLME_IS_ADHOC(padapter))) {
		default_WMM_para_g6(padapter);
		init_WMM_Para_Element_g6(padapter, padapter->registrypriv.wifi_mib.WMM_PARA_IE, sizeof(padapter->registrypriv.wifi_mib.WMM_PARA_IE));
	}
//#ifdef CLIENT_MODE
	else if(MLME_IS_STA(padapter)) {
		init_WMM_Para_Element_g6(padapter, padapter->registrypriv.wifi_mib.WMM_IE, sizeof(padapter->registrypriv.wifi_mib.WMM_IE));	//	WMM STA
	}
//#endif
#endif

#ifdef CONFIG_RTW_A4_STA
	core_a4_init(padapter);
#endif
#ifdef CONFIG_AMSDU_HW_TIMER
	padapter->amsdu_hw_timeout = 0;
#endif
#ifdef CONFIG_CORE_TXSC
	txsc_init(padapter);
#endif
#ifdef CONFIG_RTW_CORE_RXSC
	core_rxsc_init(padapter);
#endif


	#if 0 //#ifdef CONFIG_WFA_OFDMA_Logo_Test
	rtw_phl_write8(phl, 0x9e20, 0x1b);
	rtw_phl_write8(phl, 0x9e21, 0x3);

	rtw_phl_write32(phl, 0x11484, 0xf000000);
	rtw_phl_write32(phl, 0x9e1c, 0xffffffff);

	/* ss2f report from qch4 */
	rtw_phl_write32(phl, 0x9e50, 0x70b00);
	#endif
	return 0;

netdev_open_error:
	padapter->netif_up = _FALSE;

	#ifdef CONFIG_RTW_NAPI
	if(padapter->napi_state == NAPI_ENABLE) {
		napi_disable(&padapter->napi);
		padapter->napi_state = NAPI_DISABLE;
	}
	#endif

	rtw_netif_carrier_off(pnetdev);
	rtw_netif_stop_queue(pnetdev);

	RTW_ERR(FUNC_NDEV_FMT" Failed!! (netif_up=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->netif_up);

	return -1;

}

int netdev_open(struct net_device *pnetdev)
{
	int ret = _FALSE;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	struct dvobj_priv *dvobj_tmp = NULL;
	_adapter *primary_padapter_tmp = NULL;
#endif

	if (pwrctrlpriv->bInSuspend == _TRUE) {
		RTW_INFO(" [WARN] "ADPT_FMT" %s  failed, bInSuspend=%d\n", ADPT_ARG(padapter), __func__, pwrctrlpriv->bInSuspend);
		return 0;
	}

	RTW_INFO(FUNC_NDEV_FMT" , netif_up=%d\n", FUNC_NDEV_ARG(pnetdev), padapter->netif_up);
	/*rtw_dump_stack();*/
	_rtw_mutex_lock_interruptible(&(adapter_to_dvobj(padapter)->hw_init_mutex));
	ret = _netdev_open(pnetdev);
	_rtw_mutex_unlock(&(adapter_to_dvobj(padapter)->hw_init_mutex));

#ifdef SBWC
	rtw_init_timer(&(padapter->SBWC_timer), sbwc_timer_handler, padapter);
	_set_timer(&padapter->SBWC_timer, SBWC_PERIOD);
#endif

#ifdef RTW_STA_BWC
	padapter->tx_tp_base = 0;
	padapter->tx_tp_limit = 0;
	padapter->got_limit_tp = 0;
	rtw_init_timer(&(padapter->sta_bwc_timer), sta_bwc_timer_handler, padapter);
	_set_timer(&padapter->sta_bwc_timer, STA_BWC_PERIOD);
#endif

#ifdef CONFIG_TX_DEFER
	rtw_init_timer(&(padapter->xmitpriv.tx_defer_timer), txdefer_timer_handler, padapter);
#endif

#ifdef GBWC
	if (rtw_get_intf_type(padapter) == RTW_HCI_PCIE)
	{
		padapter->GBWC_rx_count = 0;
		padapter->GBWC_tx_count = 0;
		skb_queue_head_init(&padapter->GBWC_rx_queue);
		skb_queue_head_init(&padapter->GBWC_tx_queue);
		padapter->GBWC_consuming_Q = 0;
		padapter->GBWC_timer_alive = 0;
		rtw_init_timer(&(padapter->GBWC_timer), gbwc_timer_handler, padapter);
		_set_timer(&padapter->GBWC_timer, GBWC_PERIOD);
	}
#endif

#ifdef CONFIG_IOCTL_CFG80211
	/* Sync interface type after interface UP */
	if (padapter->netif_up)
		rtw_cfg80211_sync_iftype(padapter);
#endif /* CONFIG_IOCTL_CFG80211 */

#ifdef CONFIG_AUTO_AP_MODE
	if (padapter->iface_id == IFACE_ID2)
		rtw_start_auto_ap(padapter);
#endif

#ifdef CONFIG_A4_LOOPBACK
	if(MLME_IS_STA(padapter))
	{
		int i;
		padapter->replace_idx = 0;
		padapter->a4_loop_cache = NULL;
		for(i=0;i<A4_LOOP_HASH_SIZE;i++)
			INIT_HLIST_HEAD(&padapter->a4_loop_list[i]);
		padapter->a4_loop_entry = (struct rtw_a4_loopback_entry *)rtw_malloc(sizeof(struct rtw_a4_loopback_entry) * MAX_A4_LOOPBACK_ENTRY_NUM);
		if(padapter->a4_loop_entry){
			memset(padapter->a4_loop_entry, 0, sizeof(struct rtw_a4_loopback_entry) * MAX_A4_LOOPBACK_ENTRY_NUM);
		}
	}
#endif

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	if(MLME_IS_STA(padapter) && !is_primary_adapter(padapter))
	{
		dvobj_tmp = adapter_to_dvobj(padapter);
		primary_padapter_tmp = dvobj_get_primary_adapter(dvobj_tmp);
		if(primary_padapter_tmp->registrypriv.wifi_mib.crossband_enable)
		{
#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
			crossband_dev_register(padapter->pnetdev, padapter->phl_role->mac_addr, CROSSBAND_5G, rtw_os_tx);
			crossband_get_dev_status_5G = rtw_crossband_get_dev_status;
#else
			if(tmp_crossband_vxd_sc == NULL)
			{
				tmp_crossband_vxd_sc = padapter;
			}
			else
			{
				tmp_crossband_vxd_sc->crossband.crossband_vxd_sc = padapter;
				padapter->crossband.crossband_vxd_sc = tmp_crossband_vxd_sc;
				tmp_crossband_vxd_sc = NULL;
			}
#endif
			padapter->crossband.primary_sc = primary_padapter_tmp;
		}
	}
#endif

	return ret;
}

#ifdef CONFIG_IPS
int  ips_netdrv_open(_adapter *padapter)
{
	int status = _SUCCESS;
	/* struct pwrctrl_priv	*pwrpriv = adapter_to_pwrctl(padapter); */
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);


	RTW_INFO("===> %s.........\n", __FUNCTION__);


	dev_clr_drv_stopped(dvobj);
	/* padapter->netif_up = _TRUE; */
	if (!rtw_hw_is_init_completed(dvobj)) {
		status = rtw_hw_start(dvobj);
		if (status == _FAIL) {
			goto netdev_open_error;
		}
		rtw_mi_hal_iface_init(padapter);
	}
#if 0
	rtw_mi_set_mac_addr(padapter);
#endif

#if 0 /*ndef CONFIG_IPS_CHECK_IN_WD*/
	rtw_set_pwr_state_check_timer(adapter_to_pwrctl(padapter));
#endif
	_set_timer(&dvobj->dynamic_chk_timer, 2000);

	return _SUCCESS;

netdev_open_error:
	/* padapter->bup = _FALSE; */
	RTW_INFO("-ips_netdrv_open - drv_open failure, netif_up=%d\n", padapter->netif_up);

	return _FAIL;
}

int rtw_ips_pwr_up(_adapter *padapter)
{
	int result;
#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(adapter_to_dvobj(padapter));
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;
#endif/* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
	systime start_time = rtw_get_current_time();
	RTW_INFO("===>  rtw_ips_pwr_up..............\n");

#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	if (psrtpriv->silent_reset_inprogress == _TRUE)
#endif/* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
		rtw_reset_drv_sw(padapter);

	result = ips_netdrv_open(padapter);
#ifdef CONFIG_RTW_SW_LED
	rtw_led_control(padapter, LED_CTL_NO_LINK);
#endif
	RTW_INFO("<===  rtw_ips_pwr_up.............. in %dms\n", rtw_get_passing_time_ms(start_time));
	return result;

}

void rtw_ips_pwr_down(_adapter *padapter)
{
	systime start_time = rtw_get_current_time();
	RTW_INFO("===> rtw_ips_pwr_down...................\n");


	rtw_ips_dev_unload(padapter);
	RTW_INFO("<=== rtw_ips_pwr_down..................... in %dms\n", rtw_get_passing_time_ms(start_time));
}
#endif
void rtw_ips_dev_unload(_adapter *padapter)
{
#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(adapter_to_dvobj(padapter));
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;
#endif/* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
	RTW_INFO("====> %s...\n", __FUNCTION__);


#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	if (psrtpriv->silent_reset_inprogress == _TRUE)
#endif /* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
	{
		rtw_hal_set_hwreg(padapter, HW_VAR_FIFO_CLEARN_UP, 0);
	}

	if (!dev_is_surprise_removed(adapter_to_dvobj(padapter)) &&
			rtw_hw_is_init_completed(adapter_to_dvobj(padapter)))
		rtw_hw_stop(adapter_to_dvobj(padapter));

}

int _pm_netdev_open(_adapter *padapter)
{
	uint status;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);
	struct net_device *pnetdev = padapter->pnetdev;

	RTW_INFO(FUNC_NDEV_FMT" start\n", FUNC_NDEV_ARG(pnetdev));

	if (!rtw_hw_is_init_completed(dvobj)) { // ips
		dev_clr_surprise_removed(dvobj);
		dev_clr_drv_stopped(dvobj);
		status = rtw_hw_start(dvobj);
		if (status == _FAIL)
			goto netdev_open_error;
		#ifdef CONFIG_RTW_SW_LED
		rtw_led_control(padapter, LED_CTL_NO_LINK);
		#endif
		_set_timer(&dvobj->dynamic_chk_timer, 2000);

	#if 0 /*ndef CONFIG_IPS_CHECK_IN_WD*/
		rtw_set_pwr_state_check_timer(pwrctrlpriv);
	#endif /*CONFIG_IPS_CHECK_IN_WD*/
	}

	if (rtw_set_run_cmd_en(padapter, 1, RTW_CMDF_WAIT_ACK) != _SUCCESS)
		RTW_INFO("[%s %d] rtw_set_run_cmd_en fail, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);
	else
		RTW_INFO("[%s %d] rtw_set_run_cmd_en success, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);

	/*if (padapter->netif_up == _FALSE) */
	{
		rtw_hw_iface_init(padapter);

		padapter->netif_up = _TRUE;
	}

	RTW_INFO(FUNC_NDEV_FMT" Success (netif_up=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->netif_up);
	return 0;

netdev_open_error:
	padapter->netif_up = _FALSE;

	rtw_netif_carrier_off(pnetdev);
	rtw_netif_stop_queue(pnetdev);

	RTW_ERR(FUNC_NDEV_FMT" Failed!! (netif_up=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->netif_up);

	return -1;

}
int _mi_pm_netdev_open(struct net_device *pnetdev)
{
	int i;
	int status = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	_adapter *iface;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if (iface->netif_up) {
			status = _pm_netdev_open(iface);
			if (status == -1) {
				RTW_ERR("%s failled\n", __func__);
				break;
			}
		}
	}

	return status;
}

int pm_netdev_open(struct net_device *pnetdev, u8 bnormal)
{
	int status = 0;

	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);

	if (_TRUE == bnormal) {
		_rtw_mutex_lock_interruptible(&(adapter_to_dvobj(padapter)->hw_init_mutex));
		status = _mi_pm_netdev_open(pnetdev);

		_rtw_mutex_unlock(&(adapter_to_dvobj(padapter)->hw_init_mutex));
	}
#ifdef CONFIG_IPS
	else
		status = (_SUCCESS == ips_netdrv_open(padapter)) ? (0) : (-1);
#endif

	return status;
}

int _is_all_other_iface_down(struct dvobj_priv *dvobj, _adapter *padapter)
{
	int i;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (   (dvobj->padapters[i] != padapter)
		    && (dvobj->padapters[i]->netif_up))
			return 0;
	}
	return 1;
}

int netdev_close(struct net_device *pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *primary_adapter = dvobj_get_primary_adapter(dvobj);
#ifdef GBWC
	struct sk_buff *pskb = NULL;
	struct recv_priv	*precvpriv = &(primary_adapter->recvpriv);
	_queue *pfree_recv_queue = &(precvpriv->free_recv_queue);
#endif
	int all_down = _is_all_other_iface_down(dvobj, padapter);


#ifdef TX_BEAMFORMING
	rtw_core_snd_stop(padapter->dvobj);
#endif
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	rtw_flush_brsc_info(pnetdev, NULL);
#endif

	if (all_down) {
		if (dvobj->rf_ctl.trx_path.tx_nss && dvobj->rf_ctl.trx_path.rx_nss) {
			/* restore trx path setting */
			primary_adapter->registrypriv.wifi_mib.trx_path = 0;
			core_mib_trx_path(primary_adapter, NULL, 1);
		}
	}

#ifdef RTW_WKARD_INTF_RESET
	RTW_PRINT(FUNC_ADPT_FMT" , netif_up=%d\n", FUNC_ADPT_ARG(padapter),
	          padapter->netif_up);
	// DBGP("adap=0x%px prim=0x%px \n", padapter, prim_adp);

#if defined(CONFIG_MP_INCLUDED)
	rtw_mp_stop_hdl(GET_PRIMARY_ADAPTER(padapter));
#endif

	if (all_down)
		rtw_phl_disable_interrupt(dvobj->phl);

#ifdef SBWC
	_cancel_timer_ex(&padapter->SBWC_timer);
#endif
#ifdef RTW_STA_BWC
	padapter->tx_tp_base = 0;
	padapter->tx_tp_limit = 0;
	padapter->got_limit_tp = 0;
	_cancel_timer_ex(&padapter->sta_bwc_timer);
#endif
#ifdef CONFIG_TX_DEFER
	_cancel_timer_ex(&padapter->xmitpriv.tx_defer_timer);
#endif
#ifdef GBWC
	if (rtw_get_intf_type(padapter) == RTW_HCI_PCIE)
	{
		_cancel_timer_ex(&padapter->GBWC_timer);
		padapter->GBWC_rx_count = 0;
		padapter->GBWC_tx_count = 0;
		padapter->GBWC_consuming_Q = 1;
		while(skb_queue_len(&padapter->GBWC_tx_queue))
		{
			pskb = skb_dequeue(&padapter->GBWC_tx_queue);

			if (!pskb)
				break;
			/* free skb */
			rtw_skb_free(pskb);
		}
		while(skb_queue_len(&padapter->GBWC_rx_queue))
		{
			pskb = skb_dequeue(&padapter->GBWC_rx_queue);

			if (!pskb)
				break;
			/* free skb */
			rtw_free_recvframe((union recv_frame *)get_unaligned((unsigned long *)&(pskb->cb[_SKB_CB_BWC_])), pfree_recv_queue);
		}
		padapter->GBWC_consuming_Q = 0;
		padapter->GBWC_timer_alive = 0;
	}
#endif

#ifdef CONFIG_A4_LOOPBACK
	if(MLME_IS_STA(padapter))
	{
		int i;
		padapter->replace_idx = 0;
		padapter->a4_loop_cache = NULL;
		for(i=0;i<A4_LOOP_HASH_SIZE;i++)
			INIT_HLIST_HEAD(&padapter->a4_loop_list[i]);
		rtw_mfree(padapter->a4_loop_entry, sizeof(struct rtw_a4_loopback_entry) * MAX_A4_LOOPBACK_ENTRY_NUM);
		padapter->a4_loop_entry = NULL;
	}
#endif

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
	if(MLME_IS_STA(padapter) && !is_primary_adapter(padapter))
	{
#ifdef CONFIG_CROSSBAND_REPEATER_SUPPORT_WIFI5_WIFI6
		crossband_dev_unregister(padapter->pnetdev, CROSSBAND_5G);
		padapter->crossband.primary_sc = NULL;
		crossband_get_dev_status_5G = NULL;
#else
		if(padapter->crossband.crossband_vxd_sc)
		{
			padapter->crossband.crossband_vxd_sc->crossband.primary_sc = NULL;
			padapter->crossband.primary_sc = NULL;
			padapter->crossband.crossband_vxd_sc->crossband.crossband_vxd_sc = NULL;
			padapter->crossband.crossband_vxd_sc = NULL;
		}
		tmp_crossband_vxd_sc = NULL;
#endif
	}
#endif

	padapter->netif_up = _FALSE;
#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		stop_ap_mode(padapter);
	else
#endif
	{
		struct sta_info *psta;
		/* free_assoc_sta_resources */
		rtw_free_all_stainfo(padapter);

		psta = rtw_get_bcmc_stainfo(padapter);
		if (psta) {
			/* _rtw_spinlock_bh(&(pstapriv->sta_hash_lock)); */
			rtw_free_stainfo(padapter, psta);
			/* _rtw_spinunlock_bh(&(pstapriv->sta_hash_lock)); */
		}

		rtw_free_disabled_stainfo(padapter);
	}

	//_rtw_init_sta_priv(&padapter->stapriv); //down up memory leak
	padapter->stapriv.padapter = padapter;
#endif /* RTW_WKARD_INTF_RESET */

	pmlmepriv->LinkDetectInfo.bBusyTraffic = _FALSE;

	if (pwrctl->rf_pwrstate == rf_on) {
		RTW_INFO("netif_up=%d, hw_init_completed=%s\n",
			padapter->netif_up,
			rtw_hw_is_init_completed(dvobj) ? "_TRUE" : "_FALSE");

		/* s1. */
		if (pnetdev)
			rtw_netif_stop_queue(pnetdev);

#ifndef CONFIG_RTW_ANDROID
		/* s2. */
		LeaveAllPowerSaveMode(padapter);
		rtw_disassoc_cmd(padapter, 500, RTW_CMDF_WAIT_ACK);
		/* s2-2.  indicate disconnect to os */
		rtw_indicate_disconnect(padapter, 0, _FALSE);
		/* s2-3. */
		rtw_free_assoc_resources_cmd(padapter, _TRUE, RTW_CMDF_WAIT_ACK);
		/* s2-4. */
		rtw_free_network_queue(padapter, _TRUE);
#endif
	}

#ifdef CONFIG_BR_EXT
	/* if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) */
	{
		/* void nat25_db_cleanup_g6(_adapter *priv); */
		nat25_db_cleanup_g6(padapter);
	}
#endif /* CONFIG_BR_EXT */

#ifdef CONFIG_P2P
	if (!rtw_p2p_chk_role(&padapter->wdinfo, P2P_ROLE_DISABLE))
		rtw_p2p_enable(padapter, P2P_ROLE_DISABLE);
#endif /* CONFIG_P2P */
	/* stop scanning process before wifi is going to down */
	rtw_scan_abort(padapter, padapter->mlmeextpriv.sitesurvey_res.scan_timeout_ms);
#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_wait_scan_req_empty(padapter, 200);
	adapter_wdev_data(padapter)->bandroid_scan = _FALSE;
	/* padapter->rtw_wdev->iftype = NL80211_IFTYPE_MONITOR; */ /* set this at the end */
#endif /* CONFIG_IOCTL_CFG80211 */

#ifdef CONFIG_WAPI_SUPPORT
	rtw_wapi_disable_tx(padapter);
#endif

	// disable run_cmd_en before deinit data structure
	// prevent cmd racing issue by using cmd thread to change run_cmd_en
	if (rtw_set_run_cmd_en(padapter, 0, RTW_CMDF_WAIT_ACK) != _SUCCESS)
		RTW_INFO("[%s %d] rtw_set_run_cmd_en fail, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);
	else
		RTW_INFO("[%s %d] rtw_set_run_cmd_en success, value = %u\n", __FUNCTION__, __LINE__, padapter->run_cmd_en);

	rtw_hw_iface_deinit(padapter);

	RTW_INFO("-871x_drv - drv_close, netif_up=%d\n", padapter->netif_up);

#ifdef RTW_WKARD_INTF_RESET
	if (all_down) {
		rtw_drv_stop_prim_iface(padapter);
		rtw_hw_stop(padapter->dvobj);
#ifdef CONFIG_RTW_SW_LED
		rtw_led_control(GET_PRIMARY_ADAPTER(padapter), LED_CTL_DOWN);
#endif
	}
#endif

#ifdef CONFIG_RTW_A4_STA
	if (MLME_IS_STA(padapter) && padapter->a4_enable)
		core_a4_gptr_tbl_unregister(padapter);
#endif

#ifdef DEBUG_MAP_UNASSOC
	//core_map_deinit_unassoc_sta(padapter);
#endif

#ifdef CONFIG_RTW_CORE_RXSC
	core_rxsc_deinit(padapter);
#endif

	return 0;
}

int pm_netdev_close(struct net_device *pnetdev, u8 bnormal)
{
	int status = 0;

	status = netdev_close(pnetdev);

	return status;
}

void rtw_ndev_destructor(struct net_device *ndev)
{
	RTW_INFO(FUNC_NDEV_FMT"\n", FUNC_NDEV_ARG(ndev));

#ifdef CONFIG_IOCTL_CFG80211
	if (ndev->ieee80211_ptr)
		rtw_mfree((u8 *)ndev->ieee80211_ptr, sizeof(struct wireless_dev));
#endif
	free_netdev(ndev);
}

#ifdef CONFIG_ARP_KEEP_ALIVE
struct route_info {
	struct in_addr dst_addr;
	struct in_addr src_addr;
	struct in_addr gateway;
	unsigned int dev_index;
};

static void parse_routes(struct nlmsghdr *nl_hdr, struct route_info *rt_info)
{
	struct rtmsg *rt_msg;
	struct rtattr *rt_attr;
	int rt_len;

	rt_msg = (struct rtmsg *) NLMSG_DATA(nl_hdr);
	if ((rt_msg->rtm_family != AF_INET) || (rt_msg->rtm_table != RT_TABLE_MAIN))
		return;

	rt_attr = (struct rtattr *) RTM_RTA(rt_msg);
	rt_len = RTM_PAYLOAD(nl_hdr);

	for (; RTA_OK(rt_attr, rt_len); rt_attr = RTA_NEXT(rt_attr, rt_len)) {
		switch (rt_attr->rta_type) {
		case RTA_OIF:
			rt_info->dev_index = *(int *) RTA_DATA(rt_attr);
			break;
		case RTA_GATEWAY:
			rt_info->gateway.s_addr = *(u_int *) RTA_DATA(rt_attr);
			break;
		case RTA_PREFSRC:
			rt_info->src_addr.s_addr = *(u_int *) RTA_DATA(rt_attr);
			break;
		case RTA_DST:
			rt_info->dst_addr.s_addr = *(u_int *) RTA_DATA(rt_attr);
			break;
		}
	}
}

static int route_dump(u32 *gw_addr , int *gw_index)
{
	int err = 0;
	struct socket *sock;
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl nladdr;
	mm_segment_t oldfs;
	char *pg;
	int size = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 1))
	err = sock_create_kern(&init_net, AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE, &sock);
#else
	err = sock_create_kern(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE, &sock);
#endif
	if (err) {
		printk(": Could not create a datagram socket, error = %d\n", -ENXIO);
		return err;
	}

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = RTM_GETROUTE;
	req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.g.rtgen_family = AF_INET;

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
	/* referece:sock_xmit in kernel code
	 * WRITE for sock_sendmsg, READ for sock_recvmsg
	 * third parameter for msg_iovlen
	 * last parameter for iov_len
	 */
	iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, sizeof(req));
#else
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
#endif
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = MSG_DONTWAIT;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	err = sock_sendmsg(sock, &msg);
#else
	err = sock_sendmsg(sock, &msg, sizeof(req));
#endif
	set_fs(oldfs);

	if (err < 0)
		goto out_sock;

	pg = (char *) __get_free_page(GFP_KERNEL);
	if (pg == NULL) {
		err = -ENOMEM;
		goto out_sock;
	}

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
restart:
#endif

	for (;;) {
		struct nlmsghdr *h;

		iov.iov_base = pg;
		iov.iov_len = PAGE_SIZE;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
		iov_iter_init(&msg.msg_iter, READ, &iov, 1, PAGE_SIZE);
#endif

		oldfs = get_fs();
		set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		err = sock_recvmsg(sock, &msg, MSG_DONTWAIT);
#else
		err = sock_recvmsg(sock, &msg, PAGE_SIZE, MSG_DONTWAIT);
#endif
		set_fs(oldfs);

		if (err < 0)
			goto out_sock_pg;

		if (msg.msg_flags & MSG_TRUNC) {
			err = -ENOBUFS;
			goto out_sock_pg;
		}

		h = (struct nlmsghdr *) pg;

		while (NLMSG_OK(h, err)) {
			struct route_info rt_info;
			if (h->nlmsg_type == NLMSG_DONE) {
				err = 0;
				goto done;
			}

			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *errm = (struct nlmsgerr *) NLMSG_DATA(h);
				err = errm->error;
				printk("NLMSG error: %d\n", errm->error);
				goto done;
			}

			if (h->nlmsg_type == RTM_GETROUTE)
				printk("RTM_GETROUTE: NLMSG: %d\n", h->nlmsg_type);
			if (h->nlmsg_type != RTM_NEWROUTE) {
				printk("NLMSG: %d\n", h->nlmsg_type);
				err = -EINVAL;
				goto done;
			}

			memset(&rt_info, 0, sizeof(struct route_info));
			parse_routes(h, &rt_info);
			if (!rt_info.dst_addr.s_addr && rt_info.gateway.s_addr && rt_info.dev_index) {
				*gw_addr = rt_info.gateway.s_addr;
				*gw_index = rt_info.dev_index;

			}
			h = NLMSG_NEXT(h, err);
		}

		if (err) {
			printk("!!!Remnant of size %d %d %d\n", err, h->nlmsg_len, h->nlmsg_type);
			err = -EINVAL;
			break;
		}
	}

done:
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	if (!err && req.g.rtgen_family == AF_INET) {
		req.g.rtgen_family = AF_INET6;

		iov.iov_base = &req;
		iov.iov_len = sizeof(req);

		msg.msg_name = &nladdr;
		msg.msg_namelen = sizeof(nladdr);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
		iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, sizeof(req));
#else
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
#endif
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_flags = MSG_DONTWAIT;

		oldfs = get_fs();
		set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
		err = sock_sendmsg(sock, &msg);
#else
		err = sock_sendmsg(sock, &msg, sizeof(req));
#endif
		set_fs(oldfs);

		if (err > 0)
			goto restart;
	}
#endif

out_sock_pg:
	free_page((unsigned long) pg);

out_sock:
	sock_release(sock);
	return err;
}

static int arp_query(unsigned char *haddr, u32 paddr,
		     struct net_device *dev)
{
	struct neighbour *neighbor_entry;
	int	ret = 0;

	neighbor_entry = neigh_lookup(&arp_tbl, &paddr, dev);

	if (neighbor_entry != NULL) {
		neighbor_entry->used = jiffies;
		if (neighbor_entry->nud_state & NUD_VALID) {
			_rtw_memcpy(haddr, neighbor_entry->ha, dev->addr_len);
			ret = 1;
		}
		neigh_release(neighbor_entry);
	}
	return ret;
}

static int get_defaultgw(u32 *ip_addr , char mac[])
{
	int gw_index = 0; /* oif device index */
	struct net_device *gw_dev = NULL; /* oif device */

	route_dump(ip_addr, &gw_index);

	if (!(*ip_addr) || !gw_index) {
		/* RTW_INFO("No default GW\n"); */
		return -1;
	}

	gw_dev = dev_get_by_index(&init_net, gw_index);

	if (gw_dev == NULL) {
		/* RTW_INFO("get Oif Device Fail\n"); */
		return -1;
	}

	if (!arp_query(mac, *ip_addr, gw_dev)) {
		/* RTW_INFO( "arp query failed\n"); */
		dev_put(gw_dev);
		return -1;

	}
	dev_put(gw_dev);

	return 0;
}

int	rtw_gw_addr_query(_adapter *padapter)
{
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
	u32 gw_addr = 0; /* default gw address */
	unsigned char gw_mac[32] = {0}; /* default gw mac */
	int i;
	int res;

	res = get_defaultgw(&gw_addr, gw_mac);
	if (!res) {
		pmlmepriv->gw_ip[0] = gw_addr & 0xff;
		pmlmepriv->gw_ip[1] = (gw_addr & 0xff00) >> 8;
		pmlmepriv->gw_ip[2] = (gw_addr & 0xff0000) >> 16;
		pmlmepriv->gw_ip[3] = (gw_addr & 0xff000000) >> 24;
		_rtw_memcpy(pmlmepriv->gw_mac_addr, gw_mac, ETH_ALEN);
		RTW_INFO("%s Gateway Mac:\t" MAC_FMT "\n", __FUNCTION__, MAC_ARG(pmlmepriv->gw_mac_addr));
		RTW_INFO("%s Gateway IP:\t" IP_FMT "\n", __FUNCTION__, IP_ARG(pmlmepriv->gw_ip));
	} else
		RTW_INFO("Get Gateway IP/MAC fail!\n");

	return res;
}
#endif

int rtw_suspend_free_assoc_resource(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;
#endif /* CONFIG_P2P */

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	if (rtw_chk_roam_flags(padapter, RTW_ROAM_ON_RESUME)) {
		if (MLME_IS_STA(padapter)
			&& check_fwstate(pmlmepriv, WIFI_ASOC_STATE)
			#ifdef CONFIG_P2P
			&& (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)
				#if defined(CONFIG_IOCTL_CFG80211) && RTW_P2P_GROUP_INTERFACE
				|| rtw_p2p_chk_role(pwdinfo, P2P_ROLE_DEVICE)
				#endif
				)
			#endif /* CONFIG_P2P */
		) {
			RTW_INFO("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n", __FUNCTION__,
				pmlmepriv->cur_network.network.Ssid.Ssid,
				MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
				pmlmepriv->cur_network.network.Ssid.SsidLength,
				pmlmepriv->assoc_ssid.SsidLength);
			rtw_set_to_roam(padapter, 1);
		}
	}

	if (MLME_IS_STA(padapter) && check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) {
		rtw_disassoc_cmd(padapter, 0, RTW_CMDF_DIRECTLY);
		/* s2-2.  indicate disconnect to os */
		rtw_indicate_disconnect(padapter, 0, _FALSE);
	}
#ifdef CONFIG_AP_MODE
	else if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		rtw_sta_flush(padapter, _TRUE, _FALSE);
#endif

	/* s2-3. */
	rtw_free_assoc_resources(padapter, _TRUE);

	/* s2-4. */
	rtw_free_network_queue(padapter, _TRUE);

	if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY)) {
		RTW_PRINT("%s: fw_under_survey\n", __func__);
		rtw_indicate_scan_done(padapter, 1);
		clr_fwstate(pmlmepriv, WIFI_UNDER_SURVEY);
	}

	if (check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE) {
		RTW_PRINT("%s: fw_under_linking\n", __FUNCTION__);
		rtw_indicate_disconnect(padapter, 0, _FALSE);
	}

	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return _SUCCESS;
}

#ifdef CONFIG_WOWLAN
int rtw_suspend_wow(_adapter *padapter)
{
	u8 ch, bw, offset;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct wowlan_ioctl_param poidparam;
	int ret = _SUCCESS;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));


	RTW_INFO("wowlan_mode: %d\n", pwrpriv->wowlan_mode);
	RTW_INFO("wowlan_pno_enable: %d\n", pwrpriv->wowlan_pno_enable);
#ifdef CONFIG_P2P_WOWLAN
	RTW_INFO("wowlan_p2p_enable: %d\n", pwrpriv->wowlan_p2p_enable);
#endif

	if (pwrpriv->wowlan_mode == _TRUE) {
		rtw_mi_netif_stop_queue(padapter);
		#ifdef CONFIG_CONCURRENT_MODE
		rtw_mi_buddy_netif_carrier_off(padapter);
		#endif

		/* 0. Power off LED */
		#ifdef CONFIG_RTW_SW_LED
		rtw_led_control(padapter, LED_CTL_POWER_OFF);
		#endif

		/* 1. stop thread */
		dev_set_drv_stopped(dvobj);	/*for stop thread*/
		rtw_mi_stop_drv_threads(padapter);

		dev_clr_drv_stopped(dvobj);	/*for 32k command*/

		/* #ifdef CONFIG_LPS */
		/* rtw_set_ps_mode(padapter, PM_PS_MODE_ACTIVE, 0, 0, "WOWLAN"); */
		/* #endif */

		#ifdef CONFIG_SDIO_HCI
		/* 2.2 free irq */
		#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
		rtw_sdio_free_irq(dvobj);
		#endif
		#endif/*CONFIG_SDIO_HCI*/

		rtw_wow_lps_level_decide(padapter, _TRUE);
		poidparam.subcode = WOWLAN_ENABLE;
		rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);
		if (rtw_chk_roam_flags(padapter, RTW_ROAM_ON_RESUME)) {
			if (MLME_IS_STA(padapter)
			    && check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) {
				RTW_INFO("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n", __FUNCTION__,
					pmlmepriv->cur_network.network.Ssid.Ssid,
					MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
					pmlmepriv->cur_network.network.Ssid.SsidLength,
					 pmlmepriv->assoc_ssid.SsidLength);

				rtw_set_to_roam(padapter, 0);
			}
		}

		RTW_PRINT("%s: wowmode suspending\n", __func__);

		if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY) == _TRUE) {
			RTW_PRINT("%s: fw_under_survey\n", __func__);
			rtw_indicate_scan_done(padapter, 1);
			clr_fwstate(pmlmepriv, WIFI_UNDER_SURVEY);
		}

#if 1
		if (rtw_mi_check_status(padapter, MI_LINKED)) {
			ch =  rtw_mi_get_union_chan(padapter);
			bw = rtw_mi_get_union_bw(padapter);
			offset = rtw_mi_get_union_offset(padapter);
			RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n",
				 FUNC_ADPT_ARG(padapter), ch, bw, offset);
			set_channel_bwmode(padapter, ch, offset, bw, _FALSE);
		}
#else
		if (rtw_mi_get_ch_setting_union(padapter, &ch, &bw, &offset) != 0) {
			RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n",
				 FUNC_ADPT_ARG(padapter), ch, bw, offset);
			set_channel_bwmode(padapter, ch, offset, bw, _FALSE);
			rtw_mi_update_union_chan_inf(padapter, ch, offset, bw);
		}
#endif
#ifdef CONFIG_CONCURRENT_MODE
		rtw_mi_buddy_suspend_free_assoc_resource(padapter);
#endif

		if (pwrpriv->wowlan_pno_enable) {
			RTW_PRINT("%s: pno: %d\n", __func__,
				  pwrpriv->wowlan_pno_enable);
#ifdef CONFIG_FWLPS_IN_IPS
			rtw_hal_set_fw_in_ips_mode(padapter, _TRUE);
#endif
		}
#ifdef CONFIG_LPS
		else {
			if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
				rtw_set_ps_mode(padapter, pwrpriv->wowlan_power_mgmt, 0, 0, "WOWLAN");
			}
		}
#endif /* #ifdef CONFIG_LPS */

	} else
		RTW_PRINT("%s: ### ERROR ### wowlan_mode=%d\n", __FUNCTION__, pwrpriv->wowlan_mode);
	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* #ifdef CONFIG_WOWLAN */

#ifdef CONFIG_AP_WOWLAN
int rtw_suspend_ap_wow(_adapter *padapter)
{
	u8 ch, bw, offset;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct wowlan_ioctl_param poidparam;
	int ret = _SUCCESS;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	pwrpriv->wowlan_ap_mode = _TRUE;

	RTW_INFO("wowlan_ap_mode: %d\n", pwrpriv->wowlan_ap_mode);

	rtw_mi_netif_stop_queue(padapter);

	/* 0. Power off LED */
	#ifdef CONFIG_RTW_SW_LED
	rtw_led_control(padapter, LED_CTL_POWER_OFF);
	#endif

	/* 1. stop thread */
	dev_set_drv_stopped(dvobj);	/*for stop thread*/
	rtw_mi_stop_drv_threads(padapter);
	dev_clr_drv_stopped(dvobj);	/*for 32k command*/

	#ifdef CONFIG_SDIO_HCI
	/* 2.2 free irq */
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	rtw_sdio_free_irq(dvobj);
	#endif
	#endif/*CONFIG_SDIO_HCI*/

	rtw_wow_lps_level_decide(padapter, _TRUE);
	poidparam.subcode = WOWLAN_AP_ENABLE;
	rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);

	RTW_PRINT("%s: wowmode suspending\n", __func__);
#if 1
	if (rtw_mi_check_status(padapter, MI_LINKED)) {
		ch =  rtw_mi_get_union_chan(padapter);
		bw = rtw_mi_get_union_bw(padapter);
		offset = rtw_mi_get_union_offset(padapter);
		RTW_INFO("back to linked/linking union - ch:%u, bw:%u, offset:%u\n", ch, bw, offset);
		set_channel_bwmode(padapter, ch, offset, bw, _FALSE);
	}
#else
	if (rtw_mi_get_ch_setting_union(padapter, &ch, &bw, &offset) != 0) {
		RTW_INFO("back to linked/linking union - ch:%u, bw:%u, offset:%u\n", ch, bw, offset);
		set_channel_bwmode(padapter, ch, offset, bw, _FALSE);
		rtw_mi_update_union_chan_inf(padapter, ch, offset, bw);
	}
#endif

	/*FOR ONE AP - TODO :Multi-AP*/
	{
		int i;
		_adapter *iface;
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if ((iface) && rtw_is_adapter_up(iface)) {
				if (check_fwstate(&iface->mlmepriv, WIFI_AP_STATE | WIFI_MESH_STATE) == _FALSE)
					rtw_suspend_free_assoc_resource(iface);
			}
		}

	}

#ifdef CONFIG_LPS
	if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
		rtw_set_ps_mode(padapter, pwrpriv->wowlan_power_mgmt, 0, 0, "AP-WOWLAN");
	}
#endif

	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* #ifdef CONFIG_AP_WOWLAN */


int rtw_suspend_normal(_adapter *padapter)
{
	int ret = _SUCCESS;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	rtw_mi_netif_caroff_qstop(padapter);

	rtw_mi_suspend_free_assoc_resource(padapter);
#ifdef CONFIG_RTW_SW_LED
	rtw_led_control(padapter, LED_CTL_POWER_OFF);
#endif
	if ((rtw_hal_check_ips_status(padapter) == _TRUE)
	    || (adapter_to_pwrctl(padapter)->rf_pwrstate == rf_off))
		RTW_PRINT("%s: ### ERROR #### driver in IPS ####ERROR###!!!\n", __FUNCTION__);

	dev_set_drv_stopped(adapter_to_dvobj(padapter));	/*for stop thread*/
#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	rtw_stop_cmd_thread(padapter);
#endif
#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_stop_vir_ifaces(adapter_to_dvobj(padapter));
#endif
	rtw_drv_stop_prim_iface(padapter);

	if (rtw_hw_is_init_completed(adapter_to_dvobj(padapter)))
		rtw_hw_stop(adapter_to_dvobj(padapter));
	dev_set_surprise_removed(adapter_to_dvobj(padapter));

	#ifdef CONFIG_SDIO_HCI
	rtw_sdio_deinit(adapter_to_dvobj(padapter));

	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	rtw_sdio_free_irq(adapter_to_dvobj(padapter));
	#endif
	#endif /*CONFIG_SDIO_HCI*/

	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}

int rtw_suspend_common(_adapter *padapter)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
#ifdef CONFIG_WOWLAN
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct registry_priv *registry_par = &padapter->registrypriv;
#endif

	int ret = 0;
	systime start_time = rtw_get_current_time();

	RTW_PRINT(" suspend start\n");
	RTW_INFO("==> %s (%s:%d)\n", __FUNCTION__, current->comm, current->pid);

	pdbgpriv->dbg_suspend_cnt++;

	pwrpriv->bInSuspend = _TRUE;

	if ((!padapter->netif_up) || RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO("%s netif_up=%d bDriverStopped=%s bSurpriseRemoved = %s\n", __func__
			 , padapter->netif_up
			 , dev_is_drv_stopped(adapter_to_dvobj(padapter)) ? "True" : "False"
			, dev_is_surprise_removed(adapter_to_dvobj(padapter)) ? "True" : "False");
		pdbgpriv->dbg_suspend_error_cnt++;
		goto exit;
	}
	rtw_mi_scan_abort(padapter, _TRUE);
	rtw_ps_deny(padapter, PS_DENY_SUSPEND);

	rtw_mi_cancel_all_timer(padapter);
	LeaveAllPowerSaveModeDirect(padapter);

	rtw_ps_deny_cancel(padapter, PS_DENY_SUSPEND);

	if (rtw_mi_check_status(padapter, MI_AP_MODE) == _FALSE) {
#ifdef CONFIG_WOWLAN
		if (WOWLAN_IS_STA_MIX_MODE(padapter))
			pwrpriv->wowlan_mode = _TRUE;
		else if ( registry_par->wowlan_enable && check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
			pwrpriv->wowlan_mode = _TRUE;
		else if (pwrpriv->wowlan_pno_enable == _TRUE)
			pwrpriv->wowlan_mode |= pwrpriv->wowlan_pno_enable;

#ifdef CONFIG_P2P_WOWLAN
		if (!rtw_p2p_chk_state(&padapter->wdinfo, P2P_STATE_NONE) || P2P_ROLE_DISABLE != padapter->wdinfo.role)
			pwrpriv->wowlan_p2p_mode = _TRUE;
		if (_TRUE == pwrpriv->wowlan_p2p_mode)
			pwrpriv->wowlan_mode |= pwrpriv->wowlan_p2p_mode;
#endif /* CONFIG_P2P_WOWLAN */

		if (pwrpriv->wowlan_mode == _TRUE)
			rtw_suspend_wow(padapter);
		else
#endif /* CONFIG_WOWLAN */
			rtw_suspend_normal(padapter);
	} else if (rtw_mi_check_status(padapter, MI_AP_MODE)) {
#ifdef CONFIG_AP_WOWLAN
		rtw_suspend_ap_wow(padapter);
#else
		rtw_suspend_normal(padapter);
#endif /*CONFIG_AP_WOWLAN*/
	}


	RTW_PRINT("rtw suspend success in %d ms\n",
		  rtw_get_passing_time_ms(start_time));

exit:
	RTW_INFO("<===  %s return %d.............. in %dms\n", __FUNCTION__
		 , ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

#ifdef CONFIG_WOWLAN
int rtw_resume_process_wow(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;
	struct wowlan_ioctl_param poidparam;
	struct sta_info	*psta = NULL;
	struct registry_priv  *registry_par = &padapter->registrypriv;
	int ret = _SUCCESS;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	if (padapter) {
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		pdbgpriv->dbg_resume_error_cnt++;
		ret = -1;
		goto exit;
	}

	if (RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO("%s pdapter %p bDriverStopped %s bSurpriseRemoved %s\n"
			 , __func__, padapter
			 , dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");
		goto exit;
	}

	pwrpriv->wowlan_in_resume = _TRUE;
#ifdef CONFIG_PNO_SUPPORT
#ifdef CONFIG_FWLPS_IN_IPS
	if (pwrpriv->wowlan_pno_enable)
		rtw_hal_set_fw_in_ips_mode(padapter, _FALSE);
#endif /* CONFIG_FWLPS_IN_IPS */
#endif/* CONFIG_PNO_SUPPORT */

	if (pwrpriv->wowlan_mode == _TRUE) {
#ifdef CONFIG_LPS
		if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
			rtw_set_ps_mode(padapter, PM_PS_MODE_ACTIVE, 0, 0, "WOWLAN");
			rtw_wow_lps_level_decide(padapter, _FALSE);
		}
#endif /* CONFIG_LPS */

		pwrpriv->bFwCurrentInPSMode = _FALSE;

		#ifdef CONFIG_SDIO_HCI
		#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
		if (rtw_sdio_alloc_irq(dvobj) != _SUCCESS) {
			ret = -1;
			goto exit;
		}
		#endif
		#endif/*CONFIG_SDIO_HCI*/

		/* Disable WOW, set H2C command */
		poidparam.subcode = WOWLAN_DISABLE;
		rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);

#ifdef CONFIG_CONCURRENT_MODE
		rtw_mi_buddy_reset_drv_sw(padapter);
#endif

		psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
		if (psta)
			rtw_hal_sta_ra_registed(psta);


		dev_clr_drv_stopped(dvobj);
		RTW_INFO("%s: wowmode resuming, DriverStopped:%s\n", __func__, dev_is_drv_stopped(dvobj) ? "True" : "False");

		rtw_mi_start_drv_threads(padapter);

		if(registry_par->suspend_type == FW_IPS_DISABLE_BBRF && !check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) {
			if (!dev_is_surprise_removed(dvobj)) {
				/*rtw_hw_stop(dvobj);*/
				/*rtw_hw_start(dvobj);*/
			}
			RTW_INFO("FW_IPS_DISABLE_BBRF hal deinit, hal init \n");
		}

#ifdef CONFIG_CONCURRENT_MODE
		rtw_mi_buddy_netif_carrier_on(padapter);
#endif

		/* start netif queue */
		rtw_mi_netif_wake_queue(padapter);

	} else

		RTW_PRINT("%s: ### ERROR ### wowlan_mode=%d\n", __FUNCTION__, pwrpriv->wowlan_mode);

	if (padapter->pid[1] != 0) {
		RTW_INFO("pid[1]:%d\n", padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

	if (rtw_chk_roam_flags(padapter, RTW_ROAM_ON_RESUME)) {
		if (pwrpriv->wowlan_wake_reason == FW_DECISION_DISCONNECT ||
		    pwrpriv->wowlan_wake_reason == RX_DISASSOC||
		    pwrpriv->wowlan_wake_reason == RX_DEAUTH) {

			RTW_INFO("%s: disconnect reason: %02x\n", __func__,
				 pwrpriv->wowlan_wake_reason);
			rtw_indicate_disconnect(padapter, 0, _FALSE);

			rtw_sta_media_status_rpt(padapter,
					 rtw_get_stainfo(&padapter->stapriv,
					 get_bssid(&padapter->mlmepriv)), 0);

			rtw_free_assoc_resources(padapter, _TRUE);
			pmlmeinfo->state = WIFI_FW_NULL_STATE;

		} else {
			RTW_INFO("%s: do roaming\n", __func__);
			rtw_roaming(padapter, NULL);
		}
	}

	if (pwrpriv->wowlan_mode == _TRUE) {
		_set_timer(&dvobj->dynamic_chk_timer, 2000);
#if 0 /*ndef CONFIG_IPS_CHECK_IN_WD*/
		rtw_set_pwr_state_check_timer(pwrpriv);
#endif
	} else
		RTW_PRINT("do not reset timer\n");

	pwrpriv->wowlan_mode = _FALSE;

	/* Power On LED */
#ifdef CONFIG_RTW_SW_LED
	if (pwrpriv->wowlan_wake_reason == RX_DISASSOC||
	    pwrpriv->wowlan_wake_reason == RX_DEAUTH||
	    pwrpriv->wowlan_wake_reason == FW_DECISION_DISCONNECT)
		rtw_led_control(padapter, LED_CTL_NO_LINK);
	else
		rtw_led_control(padapter, LED_CTL_LINK);
#endif
	/* clean driver side wake up reason. */
	pwrpriv->wowlan_last_wake_reason = pwrpriv->wowlan_wake_reason;
	pwrpriv->wowlan_wake_reason = 0;

exit:
	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* #ifdef CONFIG_WOWLAN */

#ifdef CONFIG_AP_WOWLAN
int rtw_resume_process_ap_wow(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;
	struct wowlan_ioctl_param poidparam;
	struct sta_info	*psta = NULL;
	int ret = _SUCCESS;
	u8 ch, bw, offset;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	if (padapter) {
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		pdbgpriv->dbg_resume_error_cnt++;
		ret = -1;
		goto exit;
	}


#ifdef CONFIG_LPS
	if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
		rtw_set_ps_mode(padapter, PM_PS_MODE_ACTIVE, 0, 0, "AP-WOWLAN");
		rtw_wow_lps_level_decide(padapter, _FALSE);
	}
#endif /* CONFIG_LPS */

	pwrpriv->bFwCurrentInPSMode = _FALSE;
#if 0 /*GEORGIA_TODO_REMOVE_IT_FOR_PHL_ARCH*/

	rtw_hal_disable_interrupt(GET_HAL_DATA(dvobj));

	rtw_hal_clear_interrupt(padapter);
#endif

	#ifdef CONFIG_SDIO_HCI
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	if (rtw_sdio_alloc_irq(dvobj) != _SUCCESS) {
		ret = -1;
		goto exit;
	}
	#endif
	#endif/*CONFIG_SDIO_HCI*/
	/* Disable WOW, set H2C command */
	poidparam.subcode = WOWLAN_AP_DISABLE;
	rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);
	pwrpriv->wowlan_ap_mode = _FALSE;

	dev_clr_drv_stopped(dvobj);
	RTW_INFO("%s: wowmode resuming, DriverStopped:%s\n", __func__, dev_is_drv_stopped(dvobj) ? "True" : "False");

	rtw_mi_start_drv_threads(padapter);

#if 1
	if (rtw_mi_check_status(padapter, MI_LINKED)) {
		ch =  rtw_mi_get_union_chan(padapter);
		bw = rtw_mi_get_union_bw(padapter);
		offset = rtw_mi_get_union_offset(padapter);
		RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n", FUNC_ADPT_ARG(padapter), ch, bw, offset);
		set_channel_bwmode(padapter, ch, offset, bw, _FALSE);
	}
#else
	if (rtw_mi_get_ch_setting_union(padapter, &ch, &bw, &offset) != 0) {
		RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n", FUNC_ADPT_ARG(padapter), ch, bw, offset);
		set_channel_bwmode(padapter, ch, offset, bw, _FALSE);
		rtw_mi_update_union_chan_inf(padapter, ch, offset, bw);
	}
#endif

	/*FOR ONE AP - TODO :Multi-AP*/
	{
		int i;
		_adapter *iface;

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if ((iface) && rtw_is_adapter_up(iface)) {
				if (check_fwstate(&iface->mlmepriv, WIFI_AP_STATE | WIFI_MESH_STATE | WIFI_ASOC_STATE))
					rtw_reset_drv_sw(iface);
			}
		}

	}

	/* start netif queue */
	rtw_mi_netif_wake_queue(padapter);

	if (padapter->pid[1] != 0) {
		RTW_INFO("pid[1]:%d\n", padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

#ifdef CONFIG_RESUME_IN_WORKQUEUE
	/* rtw_unlock_suspend(); */
#endif /* CONFIG_RESUME_IN_WORKQUEUE */

	_set_timer(&dvobj->dynamic_chk_timer, 2000);
#if 0 /*ndef CONFIG_IPS_CHECK_IN_WD*/
	rtw_set_pwr_state_check_timer(pwrpriv);
#endif
	/* clean driver side wake up reason. */
	pwrpriv->wowlan_wake_reason = 0;

	/* Power On LED */
#ifdef CONFIG_RTW_SW_LED
	rtw_led_control(padapter, LED_CTL_LINK);
#endif
exit:
	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* #ifdef CONFIG_APWOWLAN */

void rtw_mi_resume_process_normal(_adapter *padapter)
{
	int i;
	_adapter *iface;
	struct mlme_priv *pmlmepriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface)) {
			pmlmepriv = &iface->mlmepriv;

			if (MLME_IS_STA(padapter)) {
				RTW_INFO(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_STATION_STATE\n", FUNC_ADPT_ARG(iface), get_fwstate(pmlmepriv));

				if (rtw_chk_roam_flags(iface, RTW_ROAM_ON_RESUME))
					rtw_roaming(iface, NULL);

			} else if (MLME_IS_AP(iface) || MLME_IS_MESH(iface)) {
				RTW_INFO(FUNC_ADPT_FMT" %s\n", FUNC_ADPT_ARG(iface), MLME_IS_AP(iface) ? "AP" : "MESH");
				rtw_ap_restore_network(iface);
			} else if (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE))
				RTW_INFO(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_ADHOC_STATE\n", FUNC_ADPT_ARG(iface), get_fwstate(pmlmepriv));
			else
				RTW_INFO(FUNC_ADPT_FMT" fwstate:0x%08x - ???\n", FUNC_ADPT_ARG(iface), get_fwstate(pmlmepriv));
		}
	}
}

int rtw_resume_process_normal(_adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv;
	struct dvobj_priv *dvobj;
	struct debug_priv *pdbgpriv;

	int ret = _SUCCESS;

	if (!padapter) {
		ret = -1;
		goto exit;
	}

	pnetdev = padapter->pnetdev;
	pwrpriv = adapter_to_pwrctl(padapter);
	dvobj = padapter->dvobj;
	pdbgpriv = &dvobj->drv_dbg;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	#ifdef CONFIG_SDIO_HCI
	/* interface init */
	if (rtw_sdio_init(dvobj) != _SUCCESS) {
		ret = -1;
		goto exit;
	}
	#endif/*CONFIG_SDIO_HCI*/

	dev_clr_surprise_removed(dvobj);
#if 0 /*GEORGIA_TODO_REMOVE_IT_FOR_PHL_ARCH*/
	rtw_hal_disable_interrupt(GET_HAL_DATA(dvobj));
#endif
	#ifdef CONFIG_SDIO_HCI
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	if (rtw_sdio_alloc_irq(dvobj) != _SUCCESS) {
		ret = -1;
		goto exit;
	}
	#endif
	#endif/*CONFIG_SDIO_HCI*/

	rtw_mi_reset_drv_sw(padapter);

	pwrpriv->bkeepfwalive = _FALSE;

	RTW_INFO("bkeepfwalive(%x)\n", pwrpriv->bkeepfwalive);
	if (pm_netdev_open(pnetdev, _TRUE) != 0) {
		ret = -1;
		pdbgpriv->dbg_resume_error_cnt++;
		goto exit;
	}

	rtw_mi_netif_caron_qstart(padapter);

	if (padapter->pid[1] != 0) {
		RTW_INFO("pid[1]:%d\n", padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

	rtw_mi_resume_process_normal(padapter);

#ifdef CONFIG_RESUME_IN_WORKQUEUE
	/* rtw_unlock_suspend(); */
#endif /* CONFIG_RESUME_IN_WORKQUEUE */
	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));

exit:
	return ret;
}

int rtw_resume_common(_adapter *padapter)
{
	int ret = 0;
	systime start_time = rtw_get_current_time();
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);

	if (pwrpriv == NULL)
		return 0;

	if (pwrpriv->bInSuspend == _FALSE)
		return 0;

	RTW_PRINT("resume start\n");
	RTW_INFO("==> %s (%s:%d)\n", __FUNCTION__, current->comm, current->pid);

	if (rtw_mi_check_status(padapter, MI_AP_MODE) == _FALSE) {
#ifdef CONFIG_WOWLAN
		if (pwrpriv->wowlan_mode == _TRUE)
			rtw_resume_process_wow(padapter);
		else
#endif
			rtw_resume_process_normal(padapter);

	} else if (rtw_mi_check_status(padapter, MI_AP_MODE)) {
#ifdef CONFIG_AP_WOWLAN
		rtw_resume_process_ap_wow(padapter);
#else
		rtw_resume_process_normal(padapter);
#endif /* CONFIG_AP_WOWLAN */
	}


	pwrpriv->bInSuspend = _FALSE;
	pwrpriv->wowlan_in_resume = _FALSE;

	RTW_PRINT("%s:%d in %d ms\n", __FUNCTION__ , ret,
		  rtw_get_passing_time_ms(start_time));


	return ret;
}

#ifdef CONFIG_GPIO_API
u8 rtw_get_gpio(struct net_device *netdev, u8 gpio_num)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_get_gpio(adapter, gpio_num);
}
EXPORT_SYMBOL(rtw_get_gpio);

int  rtw_set_gpio_output_value(struct net_device *netdev, u8 gpio_num, bool isHigh)
{
	u8 direction = 0;
	u8 res = -1;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_set_gpio_output_value(adapter, gpio_num, isHigh);
}
EXPORT_SYMBOL(rtw_set_gpio_output_value);

int rtw_config_gpio(struct net_device *netdev, u8 gpio_num, bool isOutput)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_config_gpio(adapter, gpio_num, isOutput);
}
EXPORT_SYMBOL(rtw_config_gpio);
int rtw_register_gpio_interrupt(struct net_device *netdev, int gpio_num, void(*callback)(u8 level))
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_register_gpio_interrupt(adapter, gpio_num, callback);
}
EXPORT_SYMBOL(rtw_register_gpio_interrupt);

int rtw_disable_gpio_interrupt(struct net_device *netdev, int gpio_num)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_disable_gpio_interrupt(adapter, gpio_num);
}
EXPORT_SYMBOL(rtw_disable_gpio_interrupt);

#endif /* #ifdef CONFIG_GPIO_API */

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE

int rtw_vendor_ie_get_api(struct net_device *dev, int ie_num, char *extra,
		u16 extra_len)
{
	int ret = 0;

	ret = rtw_vendor_ie_get_raw_data(dev, ie_num, extra, extra_len);
	return ret;
}
EXPORT_SYMBOL(rtw_vendor_ie_get_api);

int rtw_vendor_ie_set_api(struct net_device *dev, char *extra)
{
	return rtw_vendor_ie_set(dev, NULL, NULL, extra);
}
EXPORT_SYMBOL(rtw_vendor_ie_set_api);

#endif

#ifdef CONFIG_RTW_HANDLE_SER_L2

void _rtw_reset_ampdu_all_sta(_adapter *padapter)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 chk_alive_num = 0;
	char chk_alive_list[NUM_STA];
	int i;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* check asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		int stainfo_offset;

		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
		if (stainfo_offset_valid(stainfo_offset))
			chk_alive_list[chk_alive_num++] = stainfo_offset;
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	for (i = 0; i < chk_alive_num; i++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, chk_alive_list[i]);

		if (psta == NULL){
			RTW_INFO(FUNC_ADPT_FMT" sta_info is null\n", FUNC_ADPT_ARG(padapter));
		} else if (psta->state & WIFI_ASOC_STATE) {
			/* assume all STA enters sleep mode while HW reset */
			if (!(psta->state & WIFI_SLEEP_STATE) && psta->sta_stats.nr_sleep)
				stop_sta_xmit(padapter, psta);
			/* reset Rx AMPDU */
			send_delba(padapter, 0, psta->phl_sta->mac_addr);/* recipient */
			/* reset TX AMPDU */
			send_delba(padapter, 1, psta->phl_sta->mac_addr);/*  */ /* originator */
			psta->htpriv.agg_enable_bitmap = 0x0;/* reset */
			psta->htpriv.candidate_tid_bitmap = 0x0;/* reset */
		}
	}
}

void _rtw_recover_security_all_sta(_adapter *padapter)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 chk_alive_num = 0;
	char chk_alive_list[NUM_STA];
	int i;
	struct ieee_param param;
	struct security_priv *sec = &padapter->securitypriv;
	int ret = 0;

	if (sec->dot11PrivacyAlgrthm == _NO_PRIVACY_) {
		RTW_INFO("%s: no security, no need to recover key\n", padapter->pnetdev->name);
		return;
	}
	/* set group key */
	psta = rtw_get_stainfo(pstapriv, adapter_mac_addr(padapter));
#ifndef CONFIG_ENABLE_MAC_KEY_BACKUP
	if (sec->binstallGrpkey == _TRUE) {
		rtw_ap_set_group_key(padapter, sec->dot118021XGrpKey[sec->dot118021XGrpKeyid].skey,
							sec->dot118021XGrpPrivacy,
							sec->dot118021XGrpKeyid);
	}
#ifdef CONFIG_IEEE80211W
	if (padapter->securitypriv.binstallBIPkey == _TRUE) {
		rtw_ap_set_group_key(padapter, sec->dot11wBIPKey[sec->dot11wBIPKeyid].skey,
							sec->dot11wCipher,
							sec->dot11wBIPKeyid);
	}
#endif
#endif
	if (psta) {
		rtw_phl_cmd_set_sta_seciv(padapter->dvobj->phl,
                       				padapter->phl_role,
                       				psta->phl_sta,
                       				psta->dot11txpn.val,
                       				PHL_CMD_NO_WAIT, 0);
#ifdef CONFIG_LMT_TXREQ
		psta->dot11txpn.val += ATOMIC_READ(&psta->num_pending_txreq);
#endif
	}

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* check asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		int stainfo_offset;

		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
		if (stainfo_offset_valid(stainfo_offset))
			chk_alive_list[chk_alive_num++] = stainfo_offset;
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
#ifdef CONFIG_ENABLE_MAC_H2C_AGG
	rtw_phl_start_h2c_agg(padapter->dvobj->phl);
#endif
	for (i = 0; i < chk_alive_num; i++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, chk_alive_list[i]);

		if (psta && psta->state & WIFI_ASOC_STATE) {
			RTW_INFO("Recover key for STA macid %d, %u %u\n", psta->phl_sta->macid, ATOMIC_READ(&psta->num_pending_txreq), (u32)psta->dot11txpn.val);
#ifndef CONFIG_ENABLE_MAC_KEY_BACKUP
			rtw_ap_set_pairwise_key(padapter, psta);
#endif
			rtw_phl_cmd_set_sta_seciv(padapter->dvobj->phl,
                       				padapter->phl_role,
                       				psta->phl_sta,
                       				psta->dot11txpn.val,
                       				PHL_CMD_NO_WAIT, 0);
#ifdef CONFIG_LMT_TXREQ
			psta->dot11txpn.val += ATOMIC_READ(&psta->num_pending_txreq);
			RTW_ERR("macid %d, phl queue = %d\n", psta->phl_sta->macid, ATOMIC_READ(&psta->num_pending_txreq));
#endif
		}
	}
#ifdef CONFIG_ENABLE_MAC_H2C_AGG
	rtw_phl_stop_h2c_agg(padapter->dvobj->phl, 1);
#endif
}

void rtw_free_bcn_entry_all(struct dvobj_priv *dvobj)
{
	struct rtw_wifi_role_t *wrole;
	u8 i;

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++) {
		wrole = &dvobj->phl_com->wifi_roles[i];
		if (IS_AP_ROLE_TYPE(wrole->type)) {
			rtw_phl_free_bcn_entry(dvobj->phl, wrole);
		}
	}
}

u8 rtw_ch_bw_recover(struct dvobj_priv *dvobj)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	_adapter *pri_adapter = dvobj->padapters[0];

	if (!pri_adapter)
		return false;

	phl_status = rtw_phl_cmd_set_ch_bw(pri_adapter->phl_role,
					&pri_adapter->phl_role->chandef,
					_TRUE,
					PHL_CMD_DIRECTLY,
					0);

	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		return false;
	return true;
}

u8 rtw_core_phl_restart(struct dvobj_priv *dvobj)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	rtw_phl_rf_off(dvobj->phl);
	phl_status = rtw_phl_suspend_all_sta(dvobj->phl, PHL_CMD_DIRECTLY);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		RTW_ERR("%s rtw_phl_suspend_all_sta return status fail!\n", __func__);
	rtw_phl_ser_l2_done_notify(dvobj->phl_com);
	rtw_phl_rf_on(dvobj->phl);
	phl_status = rtw_phl_resume_all_sta(dvobj->phl, PHL_CMD_DIRECTLY);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		RTW_ERR("%s rtw_phl_resume_all_sta return status fail!\n", __func__);
	rtw_ch_bw_recover(dvobj);

	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		return false;
	return true;
}

void rtw_recover_device(struct dvobj_priv *dvobj)
{
	_adapter *iface;
	_adapter *pri_adapter = dvobj_get_primary_adapter(dvobj);
	u8 i = 0;
	struct rtw_chan_def chandef = {0};

	if (dvobj->ser_L2_inprogress) {
		RTW_ERR("L2 restore in progress!\n");
		return;
	}

	if(!rtw_is_adapter_up(dvobj->padapters[0]))
		return;
	dvobj->ser_L2_cnt++;
	dvobj->ser_L2_inprogress = 1;
#ifdef CONFIG_DFS_MASTER
	rtw_dfs_rd_detect_onoff(pri_adapter, 0);
#endif
	rtw_free_bcn_entry_all(dvobj);
	rtw_core_phl_restart(dvobj);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if (!rtw_is_adapter_up(iface))
			continue;

		RTW_INFO("[%s]%s===>\n", __func__, iface->pnetdev->name);
		if (MLME_IS_MESH(iface)
			|| MLME_IS_AP(iface)
			|| MLME_IS_ADHOC_MASTER(iface)) {
			if (send_beacon(iface) == _FAIL)
				RTW_ERR(ADPT_FMT" issue_beacon, fail!\n",
							ADPT_ARG(iface));
		}
		_rtw_recover_security_all_sta(iface);
#ifdef CONFIG_CORE_TXSC
		txsc_clear(iface, 1);
#endif
	}

	//rtw_dynamic_change_bcnint(pri_adapter, 1);

	dvobj->ser_L2_inprogress = 0;

#ifdef CONFIG_DFS_MASTER
	rtw_dfs_rd_detect_onoff(pri_adapter, 1);
#endif
	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if (!rtw_is_adapter_up(iface))
			continue;
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
		rtw_core_recover_mibs(iface);
#endif
		_rtw_reset_ampdu_all_sta(iface);
	}
	rtw_phl_get_cur_hal_chdef(pri_adapter->phl_role, &chandef);
	set_channel_bwmode(pri_adapter, chandef.chan, chandef.offset, chandef.bw, _TRUE);
}
#endif /* CONFIG_RTW_HANDLE_SER_L2 */
