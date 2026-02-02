#define _RTW_WLAN_MANAGER_C_

#include <drv_types.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>


#ifdef CONFIG_WLAN_MANAGER

static struct sock *nl_sock = NULL;
static _adapter *nl_adapter[BAND_MAX][CONFIG_IFACE_NUMBER] = {{NULL}};
static u8 nl_adapter_hook[BAND_MAX][CONFIG_IFACE_NUMBER] = {{0}};
static u8 nl_daemon_on = 0;

#if defined(PLATFORM_LINUX) && defined(CPTCFG_WFO_VIRT_MODULE)
int (* wfo_sender_netlink_recv_msg)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(wfo_sender_netlink_recv_msg);
#endif

void rtw_wlan_manager_recv_msg(struct nl_message *msg)
{
	_adapter *adapter = NULL;
	_list *phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = NULL;
	u8 adapter_hook = 0;
	u8 band;
	u8 ssid;
	struct elm_header *hdr = NULL;
	struct elm_intf *intf = NULL;
	struct elm_sta_info *sta_info = NULL;
	struct elm_roam_info *roam_info = NULL;
	u32 offset = 0;

	/* message type */
	switch(msg->type) {
	case NL_DAEMON_ON_TYPE:
		nl_daemon_on = 1;
		for (band = 0; band < BAND_MAX; band++) {
			for (ssid = 0; ssid < CONFIG_IFACE_NUMBER; ssid++) {
				adapter = nl_adapter[band][ssid];
				adapter_hook = nl_adapter_hook[band][ssid];

				if (!adapter_hook)
					continue;
				if (adapter == NULL || adapter->netif_up == _FALSE)
					continue;
	
#ifdef CONFIG_BAND_STEERING
				_band_steering_init(adapter);
#endif
				/* send associated station list to daemon */
				pstapriv = &(adapter->stapriv);
	
				_rtw_spinlock_bh(&(pstapriv->asoc_list_lock));
	
				phead = &(pstapriv->asoc_list);
				plist = get_next(phead);
				while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
					psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
					plist = get_next(plist);
					if (psta == NULL || psta->phl_sta == NULL)
						continue;
	
					rtw_netlink_send_new_sta_msg(adapter, psta->phl_sta->mac_addr);
				}
	
				_rtw_spinunlock_bh(&(pstapriv->asoc_list_lock));
			}
		}
		break;
	case NL_DAEMON_OFF_TYPE:
		nl_daemon_on = 0;
		break;
#ifdef CONFIG_BAND_STEERING
	case NL_B_STEER_BLOCK_ADD_TYPE:
	case NL_B_STEER_BLOCK_DEL_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;
	
			/* element type: should handle wrong length */
			switch(hdr->id) {
			case ELM_INTF_ID:
				intf = (struct elm_intf *)(msg->content + offset);
				break;
			case ELM_STA_INFO_ID:
				sta_info = (struct elm_sta_info *)(msg->content + offset);
				break;
			default:
				RTW_ERR("[NETLINK] unknown element id.\n");
				break;
			}
			offset += hdr->len;
		}
	
		if (intf && sta_info) {
			band = intf->band;
			ssid = intf->ssid;
			adapter = nl_adapter[band][ssid];
			adapter_hook = nl_adapter_hook[band][ssid];
	
			if (!adapter_hook)
				break;
			if (adapter == NULL || adapter->netif_up == _FALSE)
				break;
	
			if (msg->type == NL_B_STEER_BLOCK_ADD_TYPE)
				_band_steering_block_entry_add(adapter, sta_info->mac);
			else if (msg->type == NL_B_STEER_BLOCK_DEL_TYPE)
				_band_steering_block_entry_del(adapter, sta_info->mac);
		}
		break;
	case NL_B_STEER_ROAM_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;
	
			/* element type: should handle wrong length */
			switch(hdr->id) {
			case ELM_INTF_ID:
				intf = (struct elm_intf *)(msg->content + offset);
				break;
			case ELM_ROAM_INFO_ID:
				roam_info = (struct elm_roam_info *)(msg->content + offset);
				break;
			default:
				RTW_ERR("[NETLINK] unknown element id.\n");
				break;
			}
			offset += hdr->len;
		}
	
		if (intf && roam_info) {
			band = intf->band;
			ssid = intf->ssid;
			adapter = nl_adapter[band][ssid];
			adapter_hook = nl_adapter_hook[band][ssid];
	
			if (!adapter_hook)
				break;
			if (adapter == NULL || adapter->netif_up == _FALSE)
				break;
	
			printk("[NETLINK] NL_B_STEER_ROAM_TYPE (hostapd_cli)!\n");
			printk("[NETLINK] sta_mac="MAC_FMT"\n", MAC_ARG(roam_info->sta_mac));
			printk("[NETLINK] bss_mac="MAC_FMT" bss_ch=%u method=%s\n", 
				MAC_ARG(roam_info->bss_mac),
				roam_info->bss_ch,
				roam_info->method == 0 ? "11V" : "Deauth");
	
			if (roam_info->method == 1)
				_band_steering_roam_block_entry_add(adapter, roam_info->sta_mac);
		}
		break;
#endif
	default:
		RTW_ERR("[NETLINK] unknown message type.\n");
		break;
	}
	return;

}

void rtw_netlink_recv_msg(struct sk_buff *skb)
{
	_adapter *adapter = NULL;
	_list *phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = NULL;
	u8 adapter_hook = 0;
	u8 band;
	u8 ssid;
	struct nlmsghdr *nlh = NULL;
	struct nl_message *msg = NULL;
	struct elm_header *hdr = NULL;
	struct elm_intf *intf = NULL;
	struct elm_sta_info *sta_info = NULL;
	struct elm_roam_info *roam_info = NULL;
	u32 offset = 0;

	if (skb == NULL) {
		RTW_ERR("[NETLINK] skb is null.\n");
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	msg = (struct nl_message *)NLMSG_DATA(nlh);

	/* message type */
	switch(msg->type) {
	case NL_DAEMON_ON_TYPE:
		nl_daemon_on = 1;

#if defined(PLATFORM_LINUX) && defined(CPTCFG_WFO_VIRT_MODULE)
	if (wfo_enable) {
		if(wfo_sender_netlink_recv_msg)
			wfo_sender_netlink_recv_msg(skb);
		else
			RTW_ERR("[NETLINK] wfo_sender_netlink_recv_msg is null.\n");
	}
#endif
		for (band = 0; band < BAND_MAX; band++) {
			for (ssid = 0; ssid < CONFIG_IFACE_NUMBER; ssid++) {
				adapter = nl_adapter[band][ssid];
				adapter_hook = nl_adapter_hook[band][ssid];

				if (!adapter_hook)
					continue;
				if (adapter == NULL || adapter->netif_up == _FALSE)
					continue;

#ifdef CONFIG_BAND_STEERING
				_band_steering_init(adapter);
#endif
				/* send associated station list to daemon */
				pstapriv = &(adapter->stapriv);

				_rtw_spinlock_bh(&(pstapriv->asoc_list_lock));

				phead = &(pstapriv->asoc_list);
				plist = get_next(phead);
				while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
					psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
					plist = get_next(plist);
					if (psta == NULL || psta->phl_sta == NULL)
						continue;

					rtw_netlink_send_new_sta_msg(adapter, psta->phl_sta->mac_addr);
				}

				_rtw_spinunlock_bh(&(pstapriv->asoc_list_lock));
			}
		}
		break;
	case NL_DAEMON_OFF_TYPE:
		nl_daemon_on = 0;
#if defined(PLATFORM_LINUX) && defined(CPTCFG_WFO_VIRT_MODULE)
	if (wfo_enable) {
		if(wfo_sender_netlink_recv_msg)
			wfo_sender_netlink_recv_msg(skb);
		else
			RTW_ERR("[NETLINK] wfo_sender_netlink_recv_msg is null.\n");
	}
#endif
		break;
#ifdef CONFIG_BAND_STEERING
	case NL_B_STEER_BLOCK_ADD_TYPE:
	case NL_B_STEER_BLOCK_DEL_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element type: should handle wrong length */
			switch(hdr->id) {
			case ELM_INTF_ID:
				intf = (struct elm_intf *)(msg->content + offset);
				break;
			case ELM_STA_INFO_ID:
				sta_info = (struct elm_sta_info *)(msg->content + offset);
				break;
			default:
				RTW_ERR("[NETLINK] unknown element id.\n");
				break;
			}
			offset += hdr->len;
		}

		if (intf && sta_info) {
			band = intf->band;
#if defined(PLATFORM_LINUX) && defined(CPTCFG_WFO_VIRT_MODULE)
		if (wfo_enable) {
#if defined(CPTCFG_WFO_OFFLOAD_2G)
			if (band == BAND_ON_24G)
#elif defined(CPTCFG_WFO_OFFLOAD_5G)
			if (band == BAND_ON_5G)
#endif
			{
				if(wfo_sender_netlink_recv_msg)
					wfo_sender_netlink_recv_msg(skb);
				else
					RTW_ERR("[NETLINK] wfo_sender_netlink_recv_msg is null.\n");
				return;
			}
		}
#endif
			ssid = intf->ssid;
			adapter = nl_adapter[band][ssid];
			adapter_hook = nl_adapter_hook[band][ssid];

			if (!adapter_hook)
				break;
			if (adapter == NULL || adapter->netif_up == _FALSE)
				break;

			if (msg->type == NL_B_STEER_BLOCK_ADD_TYPE)
				_band_steering_block_entry_add(adapter, sta_info->mac);
			else if (msg->type == NL_B_STEER_BLOCK_DEL_TYPE)
				_band_steering_block_entry_del(adapter, sta_info->mac);
		}
		break;
	case NL_B_STEER_ROAM_TYPE:
		while (offset < msg->len) {
			hdr = (struct elm_header *)(msg->content + offset);
			offset += ELM_HEADER_LEN;

			/* element type: should handle wrong length */
			switch(hdr->id) {
			case ELM_INTF_ID:
				intf = (struct elm_intf *)(msg->content + offset);
				break;
			case ELM_ROAM_INFO_ID:
				roam_info = (struct elm_roam_info *)(msg->content + offset);
				break;
			default:
				RTW_ERR("[NETLINK] unknown element id.\n");
				break;
			}
			offset += hdr->len;
		}

		if (intf && roam_info) {
			band = intf->band;
#if defined(PLATFORM_LINUX) && defined(CPTCFG_WFO_VIRT_MODULE)
		if (wfo_enable) {
#if defined(CPTCFG_WFO_OFFLOAD_2G)
			if (band == BAND_ON_24G)
#elif defined(CPTCFG_WFO_OFFLOAD_5G)
			if (band == BAND_ON_5G)
#endif
			{
				if(wfo_sender_netlink_recv_msg)
					wfo_sender_netlink_recv_msg(skb);
				else
					RTW_ERR("[NETLINK] wfo_sender_netlink_recv_msg is null.\n");
				return;
			}
		}
#endif
			ssid = intf->ssid;
			adapter = nl_adapter[band][ssid];
			adapter_hook = nl_adapter_hook[band][ssid];

			if (!adapter_hook)
				break;
			if (adapter == NULL || adapter->netif_up == _FALSE)
				break;

			printk("[NETLINK] NL_B_STEER_ROAM_TYPE (hostapd_cli)!\n");
			printk("[NETLINK] sta_mac="MAC_FMT"\n", MAC_ARG(roam_info->sta_mac));
			printk("[NETLINK] bss_mac="MAC_FMT" bss_ch=%u method=%s\n", 
				MAC_ARG(roam_info->bss_mac),
				roam_info->bss_ch,
				roam_info->method == 0 ? "11V" : "Deauth");

			if (roam_info->method == 1)
				_band_steering_roam_block_entry_add(adapter, roam_info->sta_mac);
		}
		break;
#endif
	default:
		RTW_ERR("[NETLINK] unknown message type.\n");
		break;
	}

	return;
}
EXPORT_SYMBOL(rtw_netlink_recv_msg);


void rtw_netlink_send_msg(void *msg, u32 msg_len)
{

#ifdef RTW_COMMON_NETLINK
	rtw_netlink_send(msg, msg_len, NL_COMM_WLAN_MANAGER);
#else
#if defined(PLATFORM_ECOS)
	struct net_device *ndev = dev_get_by_name(NULL, WLAN_WFOVIRT );
	if (ndev == NULL) {
		_dbgdump("In %s, net device NULL\n", __func__);
		return;
	}

	wfo_radio_cmd_sender_netlink_send_msg(ndev, msg, msg_len);
#else /* defined(PLATFORM_ECOS) */

	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	u32 skb_len;
	s32 err;

	/* allocate skb */
	skb_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);
	skb = _rtw_skb_alloc(skb_len);
	if (!skb) {
		RTW_ERR("[NETLINK] allocate skb failed.\n");
		goto func_return;
	}

	/* netlink header */
	nlh = nlmsg_put(skb, 0, 0, 0, NL_MAX_MSG_SIZE, 0);
	if (!nlh) {
		RTW_ERR("[NETLINK] put netlink header failed.\n");
		_rtw_skb_free(skb);
		goto func_return;
	}

	/* message */
	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;
	_rtw_memset(NLMSG_DATA(nlh), 0, msg_len);
	_rtw_memcpy(NLMSG_DATA(nlh), (u8 *)msg, msg_len);

	if (!nl_sock) {
		RTW_ERR("[%s %u] nl_sock is NULL\n", __FUNCTION__, __LINE__);
		goto msg_fail_skb;
	}
	/* send message */
	err = netlink_unicast(nl_sock, skb, NL_WLAN_MANAGER_PID, MSG_DONTWAIT);

	if (err < 0) {
		/* netlink_unicast() already kfree_skb */
		RTW_ERR("[NETLINK] send netlink unicast failed.\n");
		goto func_return;
	}

func_return:
	return;

msg_fail_skb:
	kfree_skb(skb);
#endif /* defined(PLATFORM_ECOS) */
#endif /* RTW_COMMON_NETLINK */
}
EXPORT_SYMBOL(rtw_netlink_send_msg);

static void rtw_netlink_set_msg(
	struct nl_message *msg, u32 *msg_len, void *elm, u32 elm_len)
{
	_rtw_memcpy(msg->content + (*msg_len), elm, elm_len);
	(*msg_len) += elm_len;

	return;
}

void rtw_netlink_send_del_sta_msg(_adapter *adapter, u8 *mac)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {{0}};
	struct elm_sta_info sta_info = {{0}};

	if (!nl_daemon_on)
		return;

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);
	
	/* element: ELM_INTF_ID */
	intf.root = 0; /* TBD */
	intf.band = adapter->phl_role->chandef.band;
	intf.ssid = adapter->iface_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	_rtw_memcpy(sta_info.mac, mac, 6);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* finish message */
	msg.type = NL_DEL_STA_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_new_sta_msg(_adapter *adapter, u8 *mac)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {{0}};
	struct elm_sta_info sta_info = {{0}};

	if (!nl_daemon_on)
		return;

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);
	
	/* element: ELM_INTF_ID */
	intf.root = 0; /* TBD */
	intf.band = adapter->phl_role->chandef.band;
	intf.ssid = adapter->iface_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	_rtw_memcpy(sta_info.mac, mac, 6);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* finish message */
	msg.type = NL_NEW_STA_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_intf_rpt_msg(_adapter *adapter)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {{0}};
	struct elm_intf_info intf_info = {0};
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rtw_env_report *env_rpt = &(dvobj->env_rpt);

	if (!nl_daemon_on)
		return;

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	_rtw_memcpy(intf.mac, adapter->mac_addr, 6);
	intf.root = 0; /* TBD */
	intf.band = adapter->phl_role->chandef.band;
	intf.ssid = adapter->iface_id;
	_rtw_memcpy(intf.name, adapter->pnetdev->name, 16);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_INTF_INFO_ID;
	hdr.len = ELM_INTF_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_INFO_ID */
	intf_info.ch = rtw_get_oper_ch(adapter);
	intf_info.ch_clm = env_rpt->nhm_cca_ratio;
	intf_info.tx_tp = 0; /* TBD */
	intf_info.rx_tp = 0; /* TBD */
	intf_info.assoc_sta_num = pstapriv->asoc_list_cnt;
#if defined(CONFIG_RTW_WNM) || defined(CONFIG_RTW_80211K)
	intf_info.bss_info = pmlmepriv->nb_info.self_nb_rpt.bss_info;
	intf_info.reg_class = pmlmepriv->nb_info.self_nb_rpt.reg_class;
	intf_info.phy_type = pmlmepriv->nb_info.self_nb_rpt.phy_type;
#endif
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf_info, ELM_INTF_INFO_LEN);

	/* finish message */
	msg.type = NL_INTF_RPT_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_sta_rpt_msg(_adapter *adapter, struct sta_info *psta)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {{0}};
	struct elm_sta_info sta_info = {{0}};
	struct elm_sta_info_ext sta_info_ext = {{0}};

	if (!nl_daemon_on)
		return;

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	_rtw_memcpy(intf.mac, adapter->mac_addr, 6);
	intf.root = 0; /* TBD */
	intf.band = adapter->phl_role->chandef.band;
	intf.ssid = adapter->iface_id;
	_rtw_memcpy(intf.name, adapter->pnetdev->name, 16);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	_rtw_memcpy(sta_info.mac, psta->phl_sta->mac_addr, 6);
	sta_info.rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
	sta_info.link_time = psta->link_time;
	sta_info.tx_tp = psta->sta_stats.tx_tp_kbits;
	sta_info.rx_tp = psta->sta_stats.rx_tp_kbits;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_EXT_ID;
	hdr.len = ELM_STA_INFO_EXT_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_EXT_ID */
	_rtw_memcpy(sta_info_ext.mac, psta->phl_sta->mac_addr, 6);
	sta_info_ext.supported_band = psta->supported_band;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info_ext, ELM_STA_INFO_EXT_LEN);

	/* finish message */
	msg.type = NL_STA_RPT_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_frame_rpt_msg(
	_adapter *adapter, u16 frame_type, u8 *sa, u8 rssi)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_frame_info frame_info = {0};
	struct elm_intf intf = {{0}};

	if (!nl_daemon_on)
		return;

	if (frame_type == WIFI_PROBEREQ && rssi <= 5)
		return;

	if((frame_type == WIFI_PROBEREQ || frame_type == WIFI_ASSOCREQ) &&
		MLME_IS_STA(adapter))
		return;

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	intf.root = 0; /* TBD */
	intf.band = adapter->phl_role->chandef.band;
	intf.ssid = adapter->iface_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_FRAME_INFO_ID;
	hdr.len = ELM_FRAME_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);
	
	/* element: ELM_FRAME_INFO_ID */
	frame_info.frame_type = frame_type;
	_rtw_memcpy(frame_info.sa, sa, 6);
	frame_info.rssi = rssi;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&frame_info, ELM_FRAME_INFO_LEN);

	/* finish message */
	msg.type = NL_FRAME_RPT_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_time_tick_msg(_adapter *adapter)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {{0}};

	if (!nl_daemon_on)
		return;

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	intf.root = 0; /* TBD */
	intf.band = adapter->phl_role->chandef.band;
	intf.ssid = adapter->iface_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* finish message */
	msg.type = NL_TIME_TICK_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_hook(_adapter *adapter, u8 band, u8 ssid)
{
	nl_adapter[band][ssid] = adapter;
	nl_adapter_hook[band][ssid] = 1;

	return;
}

#ifdef PLATFORM_LINUX
void rtw_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = rtw_netlink_recv_msg,
	};

	nl_sock = netlink_kernel_create(&init_net, NL_RTK_PROTOCOL, &cfg);
	if (!nl_sock)
		RTW_ERR("[NETLINK] create netlink falied.\n");

	return;
}

void rtw_netlink_deinit(void)
{
	if(nl_sock)
	{
		netlink_kernel_release(nl_sock);
		nl_sock = NULL;
	}
	printk("[rtw_netlink_deinit] delete nl_sock netlink succeed.\n");

	return;
}
#endif

#endif
