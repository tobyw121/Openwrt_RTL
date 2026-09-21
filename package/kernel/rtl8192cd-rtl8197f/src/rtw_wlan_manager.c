#define _RTW_WLAN_MANAGER_C_

#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>

#include "./rtw_wlan_manager.h"
#include "./8192cd_netlink.h"
#include "./8192cd.h"

#ifdef CONFIG_RTK_WLAN_MANAGER


static struct sock *nl_sock = NULL;
static struct rtl8192cd_priv *nl_adapter[BAND_MAX][RTL8192CD_NUM_VWLAN+1] = {{NULL}};
static u8 nl_adapter_hook[BAND_MAX][RTL8192CD_NUM_VWLAN+1] = {{0}};
static u8 nl_daemon_on = 0;

void rtw_wlan_manager_recv_msg(struct nl_message *msg)
{
	struct rtl8192cd_priv *priv = NULL;
	struct list_head *phead, *plist;
	struct stat_info *pstat = NULL;
	u8 adapter_hook = 0;
	u8 band;
	u8 ssid;
#ifdef CONFIG_RTK_BAND_STEERING
	struct elm_header *hdr = NULL;
	struct elm_intf *intf = NULL;
	struct elm_sta_info *sta_info = NULL;
	struct elm_roam_info *roam_info = NULL;
	u32 offset = 0;
#endif
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif

	/* message type */
	switch(msg->type)
	{
	case NL_DAEMON_ON_TYPE:
		printk("[NETLINK] NL_DAEMON_ON_TYPE!\n");
		nl_daemon_on = 1;
		for (band = 0; band < BAND_MAX; band++) {
			for (ssid = 0; ssid < RTL8192CD_NUM_VWLAN + 1; ssid++) {
				priv = nl_adapter[band][ssid];
				adapter_hook = nl_adapter_hook[band][ssid];

				if (!adapter_hook)
					continue;
				if (!IS_DRV_OPEN(priv))
					continue;

#ifdef CONFIG_RTK_BAND_STEERING
				_band_steering_init(priv);
#endif
				/* send associated station list to daemon */

#ifdef SMP_SYNC
				SMP_LOCK_ASOC_LIST(flags);
#endif

				phead = &(priv->asoc_list);
				plist = phead;
				while (!list_empty(plist) && (plist = plist->next) != phead) {
					pstat = list_entry(plist, struct stat_info, asoc_list);
#ifdef SMP_SYNC
					SMP_UNLOCK_ASOC_LIST(flags);
#endif
					rtw_netlink_send_new_sta_msg(priv, pstat->cmn_info.mac_addr);	// check sleep ?
#ifdef SMP_SYNC
					SMP_LOCK_ASOC_LIST(flags);
#endif
				}

#ifdef SMP_SYNC
				SMP_UNLOCK_ASOC_LIST(flags);
#endif
			}
		}
		break;
	case NL_DAEMON_OFF_TYPE:
		printk("[NETLINK] NL_DAEMON_OFF_TYPE!\n");
		nl_daemon_on = 0;
		break;
#ifdef CONFIG_RTK_BAND_STEERING
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
				panic_printk(KERN_INFO "[NETLINK] unknown element id.\n");
				break;
			}
			offset += hdr->len;
		}

		if (intf && sta_info) {
			band = intf->band;
			ssid = intf->ssid;
			priv = nl_adapter[band][ssid];
			adapter_hook = nl_adapter_hook[band][ssid];

			if (!adapter_hook)
				break;
			if (!IS_DRV_OPEN(priv))
				break;

			if (msg->type == NL_B_STEER_BLOCK_ADD_TYPE)
				_band_steering_block_entry_add(priv, sta_info->mac);
			else if (msg->type == NL_B_STEER_BLOCK_DEL_TYPE)
				_band_steering_block_entry_del(priv, sta_info->mac);
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
				panic_printk(KERN_INFO "[NETLINK] unknown element id.\n");
				break;
			}
			offset += hdr->len;
		}

		if (intf && roam_info) {
			band = intf->band;
			ssid = intf->ssid;
			priv = nl_adapter[band][ssid];
			adapter_hook = nl_adapter_hook[band][ssid];

			if (!adapter_hook)
				break;
			if (!IS_DRV_OPEN(priv))
				break;

			printk("[NETLINK] NL_B_STEER_ROAM_TYPE (hostapd_cli)!\n");
			printk("[NETLINK] sta_mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
				roam_info->sta_mac[0], roam_info->sta_mac[1], roam_info->sta_mac[2],
				roam_info->sta_mac[3], roam_info->sta_mac[4], roam_info->sta_mac[5]);
			printk("[NETLINK] bss_mac=%02x:%02x:%02x:%02x:%02x:%02x bss_ch=%u method=%s\n",
				roam_info->bss_mac[0], roam_info->bss_mac[1], roam_info->bss_mac[2], 
				roam_info->bss_mac[3], roam_info->bss_mac[4], roam_info->bss_mac[5], 
				roam_info->bss_ch,
				roam_info->method == 0 ? "11V" : "Deauth");

			if (roam_info->method == 1)
				_band_steering_roam_block_entry_add(priv, roam_info->sta_mac);
		}
		break;
#endif
	default:
		panic_printk(KERN_INFO "[NETLINK] unknown message type.\n");
		break;
	}

	return;
}

static void rtw_netlink_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	struct nl_message *msg = NULL;

	if (skb == NULL) {
		panic_printk(KERN_INFO "[NETLINK] skb is null.\n");
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	msg = (struct nl_message *)NLMSG_DATA(nlh);

	rtw_wlan_manager_recv_msg(msg);
}

static void rtw_netlink_send_msg(void *msg, u32 msg_len)
{
#ifdef RTK_COMMON_NETLINK
	rtk_netlink_send(msg, msg_len, NL_COMM_WLAN_MANAGER);
#else
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	u32 skb_len;
	s32 err;

	/* allocate skb */
	skb_len = NLMSG_SPACE(NL_MAX_MSG_SIZE);
	skb = dev_alloc_skb(skb_len);
	if (!skb) {
		panic_printk(KERN_INFO "[NETLINK] allocate skb failed.\n");
		goto func_return;
	}

	/* netlink header */
	nlh = nlmsg_put(skb, 0, 0, 0, NL_MAX_MSG_SIZE, 0);
	if (!nlh) {
		panic_printk(KERN_INFO "[NETLINK] put netlink header failed.\n");
		dev_kfree_skb_any(skb);
		goto func_return;
	}

	/* message */
	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;
	memset(NLMSG_DATA(nlh), 0, msg_len);
	memcpy(NLMSG_DATA(nlh), (u8 *)msg, msg_len);

	/* send message */
	err = netlink_unicast(nl_sock, skb, NL_WLAN_MANAGER_PID, MSG_DONTWAIT);

	if (err < 0) {
		/* netlink_unicast() already kfree_skb */
		panic_printk(KERN_INFO "[NETLINK] send netlink unicast failed.\n");
		goto func_return;
	}

func_return:
	return;
#endif
}

static void rtw_netlink_set_msg(
	struct nl_message *msg, u32 *msg_len, void *elm, u32 elm_len)
{
	memcpy(msg->content + (*msg_len), elm, elm_len);
	(*msg_len) += elm_len;

	return;
}

void rtw_netlink_send_del_sta_msg(struct rtl8192cd_priv *priv, u8 *mac)
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
	intf.band = priv->pshare->curr_band;
	intf.ssid = priv->if_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	memcpy(sta_info.mac, mac, MACADDRLEN);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* finish message */
	msg.type = NL_DEL_STA_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_new_sta_msg(struct rtl8192cd_priv *priv, u8 *mac)
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
	intf.band = priv->pshare->curr_band;
	intf.ssid = priv->if_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	memcpy(sta_info.mac, mac, MACADDRLEN);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* finish message */
	msg.type = NL_NEW_STA_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_intf_rpt_msg(struct rtl8192cd_priv *priv)
{
	u32 msg_len = 0;
	struct nl_message msg = {0};
	struct elm_header hdr = {0};
	struct elm_intf intf = {{0}};
	struct elm_intf_info intf_info = {0};
#if defined(HAPD_11K) || defined(DOT11K)
	union dot11k_neighbor_report_bssinfo bssinfo;
#endif

	if (!nl_daemon_on)
		return;

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	memcpy(intf.mac, GET_MY_HWADDR, MACADDRLEN);
	intf.root = 0; /* TBD */
	intf.band = priv->pshare->curr_band;
	intf.ssid = priv->if_id;
	memcpy(intf.name, priv->dev->name, 16);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_INTF_INFO_ID;
	hdr.len = ELM_INTF_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_INFO_ID */
	intf_info.ch = priv->pmib->dot11RFEntry.dot11channel;
	intf_info.ch_clm = priv->pshare->_dmODM.dm_ccx_info.clm_ratio;
	intf_info.tx_tp = 0; /* TBD */
	intf_info.rx_tp = 0; /* TBD */
	intf_info.assoc_sta_num = priv->assoc_num;
#if defined(HAPD_11K) || defined(DOT11K)		// refer to construct_self_neighbor_report_ie
	bssinfo.value = 0;
	bssinfo.field.ap_reachability = 3;
	bssinfo.field.security = 1;
	bssinfo.field.key_scope = 1;

	if (*priv->pBeaconCapability & cpu_to_le16(BIT(8)))
		bssinfo.field.cap_spectrum = 1;
	if (*priv->pBeaconCapability & cpu_to_le16(BIT(9)))
		bssinfo.field.cap_qos = 1;
	if (*priv->pBeaconCapability & cpu_to_le16(BIT(11)))
		bssinfo.field.cap_apsd = 1;
	if (*priv->pBeaconCapability & cpu_to_le16(BIT(14)))
		bssinfo.field.cap_delay_ba = 1;
	if (*priv->pBeaconCapability & cpu_to_le16(BIT(15)))
		bssinfo.field.cap_im_ba = 1;

	if (priv->hapd_11k_ie_len)
		bssinfo.field.cap_rm = 1;

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		bssinfo.field.high_tp = 1;
		intf_info.phy_type = 7;		// DOT11_PHY_TYPE_HT
	}

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
		bssinfo.field.very_high_tp = 1;
		intf_info.phy_type = 9;		// DOT11_PHY_TYPE_VHT
	}

	intf_info.bss_info = bssinfo.value;
	intf_info.reg_class = rm_get_op_class(priv, priv->pmib->dot11RFEntry.dot11channel);
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

void rtw_netlink_send_sta_rpt_msg(struct rtl8192cd_priv *priv, struct stat_info *pstat)
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
	memcpy(intf.mac, GET_MY_HWADDR, MACADDRLEN);
	intf.root = 0; /* TBD */
	intf.band = priv->pshare->curr_band;
	intf.ssid = priv->if_id;
	memcpy(intf.name, priv->dev->name, 16);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_ID;
	hdr.len = ELM_STA_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_ID */
	memcpy(sta_info.mac, pstat->cmn_info.mac_addr, MACADDRLEN);
	sta_info.rssi = pstat->rssi;
	sta_info.link_time = pstat->link_time;
	sta_info.tx_tp = (pstat->tx_avarage >> 7);
	sta_info.rx_tp = (pstat->rx_avarage >> 7);
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info, ELM_STA_INFO_LEN);

	/* element header */
	hdr.id = ELM_STA_INFO_EXT_ID;
	hdr.len = ELM_STA_INFO_EXT_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_STA_INFO_EXT_ID */
	memcpy(sta_info_ext.mac, pstat->cmn_info.mac_addr, MACADDRLEN);
	sta_info_ext.supported_band = pstat->supported_band;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&sta_info_ext, ELM_STA_INFO_EXT_LEN);

	/* finish message */
	msg.type = NL_STA_RPT_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_send_frame_rpt_msg(struct rtl8192cd_priv *priv, u16 frame_type, u8 *sa, u8 rssi)
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

	/* element header */
	hdr.id = ELM_INTF_ID;
	hdr.len = ELM_INTF_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_INTF_ID */
	intf.root = 0; /* TBD */
	intf.band = priv->pshare->curr_band;
	intf.ssid = priv->if_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* element header */
	hdr.id = ELM_FRAME_INFO_ID;
	hdr.len = ELM_FRAME_INFO_LEN;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&hdr, ELM_HEADER_LEN);

	/* element: ELM_FRAME_INFO_ID */
	frame_info.frame_type = frame_type;
	memcpy(frame_info.sa, sa, MACADDRLEN);
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

void rtw_netlink_send_time_tick_msg(struct rtl8192cd_priv *priv)
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
	intf.band = priv->pshare->curr_band;
	intf.ssid = priv->if_id;
	rtw_netlink_set_msg(&msg, &msg_len, (void *)&intf, ELM_INTF_LEN);

	/* finish message */
	msg.type = NL_TIME_TICK_TYPE;
	msg.len = msg_len;

	/* length += (type + len) */
	msg_len += 8;
	rtw_netlink_send_msg((void *)&msg, msg_len);

	return;
}

void rtw_netlink_hook(struct rtl8192cd_priv *priv, u8 band, u8 ssid)
{
	nl_adapter[band][ssid] = priv;
	nl_adapter_hook[band][ssid] = 1;

	return;
}

void rtw_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = rtw_netlink_recv_msg,
	};

	nl_sock = netlink_kernel_create(&init_net, NL_RTK_PROTOCOL, &cfg);
	if (!nl_sock)
		panic_printk(KERN_ERR "[NETLINK] create netlink falied.\n");
	else
		panic_printk("[NETLINK] create netlink success\n");

	return;
}

void rtw_netlink_deinit(void)
{
	netlink_kernel_release(nl_sock);

	return;
}

#endif
