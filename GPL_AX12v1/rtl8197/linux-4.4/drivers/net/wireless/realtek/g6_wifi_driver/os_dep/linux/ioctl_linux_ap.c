#include <drv_types.h>

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
int rtw_ioctl_del_sta(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	//TODO
	return 0;
}

#ifdef RTW_BLOCK_STA_CONNECT
int rtw_ioctl_block_sta(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8  ret = 0;
	u32 len;
	u8 buff[100]={0};
	RTK_WLAN_BLOCK_STA blockSta;
	int i;

	if(!rtw_is_adapter_up(padapter)){
		printk("[RTW](%s)is not open\n", dev->name);
		return -1;
	}

	len = wrqu->data.length;
	_rtw_memcpy(buff, extra, len);
	memset(&blockSta, 0, sizeof(RTK_WLAN_BLOCK_STA));
	memcpy(&blockSta, (RTK_WLAN_BLOCK_STA *)buff, sizeof(RTK_WLAN_BLOCK_STA));

	rtw_block_sta_conn_lookup(padapter, blockSta);

	return 0;
}
#endif

int rtw_ioctl_daemon_req(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	//TODO
	return 0;
}


int rtw_wifi_ss_req(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	ret = rtw_site_survey_start(dev, info, wrqu, extra);
	return ret;
}

int rtw_wifi_join(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	NDIS_802_11_SSID ndis_ssid;
	u8 ret = 0;

	_rtw_memset(&ndis_ssid, 0, sizeof(NDIS_802_11_SSID));
	ndis_ssid.SsidLength = pmibpriv->connect_ssid_len;
	_rtw_memcpy(ndis_ssid.Ssid, (u8 *)pmibpriv->connect_ssid, pmibpriv->connect_ssid_len);

	if(ndis_ssid.SsidLength == 0){
		ret = -1;
		RTW_ERR("rtw_wifi_join fail : ssidLength = 0 \n");
		goto end;
	}

	if (rtw_set_802_11_connect(padapter, (u8 *)pmibpriv->bssid, &ndis_ssid, pmibpriv->connect_ch) == _FALSE) {
		ret = -1;
	}

end:
	return ret;
}

#ifdef CONFIG_RTW_MULTI_AP
#ifdef DEBUG_MAP_UNASSOC
int rtw_ioctl_unassoc_sta_metric_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	int len  = 0, i = 0;
	char *buf = NULL;
	if (extra == NULL) {
		RTW_ERR("extra is NULL! \n");
		return -EFAULT;
	}
	buf = extra;

	RTW_INFO("\n");

	if (!rtw_is_adapter_up(padapter))
		return -EFAULT;

	if (core_map_ioctl_get_unassoc_metric(padapter, buf))
		return 0;
	else
		return -EFAULT;;
}
#endif

int rtw_ioctl_update_bss(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	unsigned char bss_type;

	bss_type = extra[0];

	pmibpriv->multiap_bss_type = bss_type;
	if(bss_type & BIT4) {
		// Teardown BSS
		pmibpriv->func_off = 1;
	} else if(bss_type & BIT5 || bss_type & BIT6) {
		pmibpriv->func_off = 0;
	}

	core_mib_func_off(padapter, NULL, 1);  // RTW_MIB_SET

	//rtw_wifi_syn_priv_mib_to_registrypriv(dev);
	return 0;
}

int rtw_ioctl_send_disassoc(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	struct sta_info *psta = NULL;
	unsigned char sta_mac[MAC_ADDR_LEN] = {0};
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	_rtw_memcpy((void *)sta_mac, (void *)extra, MAC_ADDR_LEN);

	RTW_INFO("rtw_ioctl_send_disassoc_to " MAC_FMT "\n", MAC_ARG(sta_mac));

	if (check_fwstate(pmlmepriv, (WIFI_ASOC_STATE | WIFI_AP_STATE)) != _TRUE)
		return -EINVAL;

	if (sta_mac[0] == 0xff && sta_mac[1] == 0xff &&
	    sta_mac[2] == 0xff && sta_mac[3] == 0xff &&
	    sta_mac[4] == 0xff && sta_mac[5] == 0xff)
		return -EINVAL;

	psta = rtw_get_stainfo(pstapriv, sta_mac);
	if (psta) {
		u8 updated = _FALSE;

		/* RTW_INFO("free psta=%p, aid=%d\n", psta, psta->phl_sta->aid); */

		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		if (rtw_is_list_empty(&psta->asoc_list) == _FALSE) {
			rtw_list_delete(&psta->asoc_list);
			pstapriv->asoc_list_cnt--;
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
			if (psta->tbtx_enable)
				pstapriv->tbtx_asoc_list_cnt--;
#endif
			updated = ap_free_sta(padapter, psta, _TRUE, WLAN_REASON_DISASSOC_DUE_BSS_TRANSITION, _TRUE, _FALSE);
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

		associated_clients_update(padapter, updated, STA_INFO_UPDATE_ALL);

		psta = NULL;

	} else {
		RTW_INFO("rtw_ioctl_send_disassoc(), sta has already been removed or never been added\n");
		/* ret = -1; */
	}

	return 0;
}

int rtw_ioctl_send_disassoc_vxd(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	rtw_free_self_stainfo(padapter);

	return 0;
}

int rtw_ioctl_set_tx_max_power(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	u8 max_tx_pwr = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	max_tx_pwr = (extra[0] * 2);
	rtw_phl_set_power_to_struct(GET_HAL_INFO(padapter->dvobj), max_tx_pwr, 0);

	return 0;
}

#ifdef CONFIG_RTW_MULTI_AP_R2
int rtw_ioctl_process_cac_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 cac_oper_type = extra[0];

	if (0 == cac_oper_type) {
		core_map_trigger_cac(padapter, extra);
	}
	if (1 == cac_oper_type) {
		core_map_terminate_cac(padapter);
	}

	return 0;
}
#endif /* defined(CONFIG_RTW_MULTI_AP_R2) */
#endif /* CONFIG_RTW_MULTI_AP */

#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_RTW_OPCLASS_CHANNEL_SCAN)
int rtw_ioctl_available_channels_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	int len  = 0, i = 0;
	char *buf = extra;

	if(!rtw_is_adapter_up(padapter)) {
		RTW_ERR("[RTW](%s)is not open\n", dev->name);
		return -1;
	}

	len += 1;
	for(i = 0; i < rfctl->max_chan_nums; i++) {
		if (WIFI_ROLE_IS_ON_24G(padapter)) {
			if (rfctl->channel_set[i].ChannelNum <= 14) {
				buf[len] = rfctl->channel_set[i].ChannelNum;
				len += 1;
			}
		}
		else if (WIFI_ROLE_IS_ON_5G(padapter)) {
			if (rfctl->channel_set[i].ChannelNum > 14) {
				buf[len] = rfctl->channel_set[i].ChannelNum;
				len += 1;
			}
		}
		else {
			buf[len] = rfctl->channel_set[i].ChannelNum;
			len += 1;
		}
	}

	buf[0] = len - 1; /* available channel number */

	wrqu->data.length = len;
	return 0;
}
#endif

#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
void rtw_opclass_site_survey_free(_adapter *padapter)
{
	u32 i, k;

	if (padapter->opclass_scan_result != NULL) {
		if (padapter->opclass_scan_result->channels != NULL) {
			for (i = 0; i < padapter->opclass_scan_result->channel_nr; i++) {
				if (padapter->opclass_scan_result->channels[i].neighbors != NULL) {
					for (k = 0; k < padapter->opclass_scan_result->channels[i].neighbor_nr; k++)
					{
						if (padapter->opclass_scan_result->channels[i].neighbors[k].ssid != NULL)
						{
							rtw_mfree(padapter->opclass_scan_result->channels[i].neighbors[k].ssid, padapter->opclass_scan_result->channels[i].neighbors[k].ssid_length);
							padapter->opclass_scan_result->channels[i].neighbors[k].ssid = NULL;
						}
					}
					rtw_mfree(padapter->opclass_scan_result->channels[i].neighbors, padapter->opclass_scan_result->channels[i].neighbor_nr *sizeof(struct channel_scan_neighbor));
					padapter->opclass_scan_result->channels[i].neighbors = NULL;
				}
			}

			rtw_mfree(padapter->opclass_scan_result->channels, (padapter->opclass_scan_result->channel_nr * sizeof(struct channel_scan_result_per_channel)));
			padapter->opclass_scan_result->channels = NULL;
		}
		rtw_mfree(padapter->opclass_scan_result, sizeof(struct channel_scan_result_per_radio));
		padapter->opclass_scan_result = NULL;
	}

	return;
}

int rtw_opclass_trigger_site_survey(_adapter *padapter)
{
	u8 ss_chk, i;
	u8 status = _FALSE;
	struct sitesurvey_parm scan_parm;

	ss_chk = rtw_sitesurvey_condition_check(padapter, _FALSE);
	if (ss_chk != SS_ALLOW) {
		RTW_INFO("can't do site survey now (reason=%u)!\n", ss_chk);
		if (ss_chk == SS_DENY_SELF_AP_UNDER_SURVEY)
			return -EALREADY;
		else
			return -EPERM;
	}

	rtw_init_sitesurvey_parm(padapter, &scan_parm);
	scan_parm.ch_num = MIN(padapter->opclass_requested_channel_nr, RTW_CHANNEL_SCAN_AMOUNT);
	for (i = 0; i < scan_parm.ch_num; i++) {
		scan_parm.ch[i].hw_value = padapter->opclass_requested_channels[i];
		/*scan_parm->ch[i].flags = RTW_IEEE80211_CHAN_PASSIVE_SCAN;*/
	}

	padapter->opclass_channel_scan = 1;
	padapter->multiap_nl_send = 0;
	padapter->opclass_channel_proc_done = 0;
	padapter->opclass_sync_result = 0;

	RTW_INFO("oclass channel scan num is:%d\n", scan_parm.ch_num);

	/*status = rtw_set_802_11_bssid_list_scan(padapter, &scan_parm);*/
	status = rtw_sitesurvey_cmd(padapter, &scan_parm);
	if (status == _FALSE) {
		RTW_INFO("site survey cmd failed!\n");
		return -EPERM;
	}

	return 0;
}

int rtw_opclass_do_site_survey(struct net_device *dev,
	struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int i = 0, ret = 0;
	u8 *ptmp = NULL;

	if (wrqu->data.length == 0 || !(wrqu->data.pointer))
	{
			ret = -EINVAL;
			goto exit;
	}

	ptmp = (u8 *)rtw_malloc(wrqu->data.length);
	if (ptmp == NULL) {
			ret = -ENOMEM;
			goto exit;
	}

	if (copy_from_user(ptmp, wrqu->data.pointer, wrqu->data.length)) {
			ret = -ENOMEM;
			goto exit;
	}

	if (padapter->opclass_requested_channels != NULL) {
		rtw_mfree(padapter->opclass_requested_channels, padapter->opclass_requested_channel_nr);
		padapter->opclass_requested_channels = NULL;
		padapter->opclass_requested_channel_nr = 0;
	}
/* mem free*/
	rtw_opclass_site_survey_free(padapter);

/*init*/
	padapter->opclass_channel_scan = 0;
	padapter->multiap_nl_send = 0;
	padapter->opclass_channel_proc_done = 0;
	padapter->opclass_sync_result = 0;

	_rtw_memcpy(&padapter->opclass_requested_channel_nr, ptmp, 1);
	padapter->opclass_requested_channels = (u8*)rtw_malloc(padapter->opclass_requested_channel_nr);
	_rtw_memcpy(padapter->opclass_requested_channels, ptmp+1, padapter->opclass_requested_channel_nr);

	padapter->opclass_scan_result	= (struct channel_scan_result_per_radio*)rtw_malloc(sizeof(struct channel_scan_result_per_radio));
	padapter->opclass_scan_result->channel_nr	= padapter->opclass_requested_channel_nr;
	padapter->opclass_scan_result->channels 	= (struct channel_scan_result_per_channel*)rtw_malloc(padapter->opclass_requested_channel_nr * sizeof(struct channel_scan_result_per_channel));
	for (i = 0; i < padapter->opclass_scan_result->channel_nr; i++) {
		padapter->opclass_scan_result->channels[i].channel				= padapter->opclass_requested_channels[i];
		padapter->opclass_scan_result->channels[i].scan_status			= 0;
		_rtw_memset(padapter->opclass_scan_result->channels[i].timestamp, 0, 31);
		padapter->opclass_scan_result->channels[i].channel_utilization	= 0;
		padapter->opclass_scan_result->channels[i].noise				= 0;
		padapter->opclass_scan_result->channels[i].neighbor_nr			= 0;
		padapter->opclass_scan_result->channels[i].neighbors			= NULL;
	}

	rtw_opclass_trigger_site_survey(padapter);

	ret = 0;
exit:
	if (ptmp)
		rtw_mfree(ptmp, wrqu->data.length);

	return ret;
}

int rtw_opclass_obtain_site_survey_result(_adapter *padapter)
{
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &padapter->dvobj->scanned_queue;
#else
	_queue	 *queue = &(padapter->mlmepriv.scanned_queue);
#endif
	_list	 *plist, *phead;
	s32 ret = -EPERM;
	u8	ss_chk;
	u8 i, j;

	if (!padapter || !padapter->opclass_channel_scan || padapter->opclass_sync_result)
		goto func_return;

	ss_chk = rtw_sitesurvey_condition_check(padapter, _FALSE);
	if (ss_chk != SS_ALLOW) {
		RTW_INFO("can't do site survey now (reason=%u)!\n", ss_chk);
		if (ss_chk == SS_DENY_SELF_AP_UNDER_SURVEY || ss_chk == SS_DENY_BUDDY_UNDER_SURVEY)
			ret = -EINPROGRESS;
		else
			ret = -EPERM;

		goto func_return;
	}

	_rtw_spinlock_bh(&(queue->lock));

	phead = get_list_head(queue);
	if (!phead) {
		ret = -EPERM;
		goto queue_unlock;
	}

	plist = get_next(phead);
	if (!plist) {
		ret = -EPERM;
		goto queue_unlock;
	}

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		struct wlan_network *pnetwork = NULL;
		struct beacon_keys	 recv_bcn = {{0}};
		WLAN_BSSID_EX		*pbss = NULL;
		u32 frame_len;
		u8 *frame = NULL;
		u8	i;
		u32 timetamp= 0;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		plist = get_next(plist);

		pbss = &(pnetwork->network);
		frame_len = pbss->IELength + WLAN_HDR_A3_LEN;
		frame = (u8 *)rtw_malloc(frame_len);
		if (NULL == frame) {
			ret = -EPERM;
			goto queue_unlock;
		}

		_rtw_memcpy(frame + WLAN_HDR_A3_LEN, pbss->IEs, pbss->IELength);

		if (rtw_get_bcn_keys(padapter, frame, frame_len, &recv_bcn) == _FAIL)
			continue;

		rtw_mfree(frame, frame_len);

		for (j = 0; j < padapter->opclass_requested_channel_nr; j++)
		{
			padapter->opclass_scan_result->channels[j].channel_utilization = 0;
			padapter->opclass_scan_result->channels[j].noise = 0;
			//find the channel entry
			if (padapter->opclass_scan_result->channels[j].channel == recv_bcn.ch)
			{
				padapter->opclass_scan_result->channels[j].neighbors =
						(struct channel_scan_neighbor*)krealloc(padapter->opclass_scan_result->channels[j].neighbors, (padapter->opclass_scan_result->channels[j].neighbor_nr + 1) * sizeof(struct channel_scan_neighbor), GFP_ATOMIC);

				timetamp = le64_to_cpu(*(u64 *)rtw_get_timestampe_from_ie(pnetwork->network.IEs));
				_rtw_memcpy(padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].timestamp
						, &timetamp
						, sizeof(padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].timestamp));
				_rtw_memcpy(padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].bssid, pbss->MacAddress, ETH_ALEN);
				padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].ssid_length			= strlen(recv_bcn.ssid);
				padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].ssid					= (s8*)rtw_malloc(padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].ssid_length + 1);
				_rtw_memcpy(padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].ssid, recv_bcn.ssid, padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].ssid_length + 1);
				padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].signal_strength		= pbss->PhyInfo.SignalStrength;;

				if (CHANNEL_WIDTH_160 == recv_bcn.bw) {
					padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].channel_band_width = MAP_BAND_WIDTH_160M_HZ;
				} else if (CHANNEL_WIDTH_80_80 == recv_bcn.bw) {
					padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].channel_band_width = MAP_BAND_WIDTH_80P80M_HZ;
				} else {
					padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].channel_band_width = recv_bcn.bw + 1;
				}
				padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].channel_utilization	= 0;
				padapter->opclass_scan_result->channels[j].neighbors[padapter->opclass_scan_result->channels[j].neighbor_nr].station_count			= 0;

				padapter->opclass_scan_result->channels[j].neighbor_nr++;
				break;
			}
		}
	}

	padapter->opclass_sync_result = 1;

	ret = 0;

queue_unlock:
	_rtw_spinunlock_bh(&(queue->lock));

func_return:
	padapter->opclass_channel_scan = 0;
	return ret;
}

#define MAX_CHANNELSCAN_BUF_PALOAD 4096
int rtw_opclass_scan_rusult_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	s32 ret = -EPERM;
	int i, j, offset = 0;
	u16 data_len = 0;

	char *send_buf = extra;

	if (!padapter->opclass_sync_result)
		goto func_return;

	data_len += 2; //data_len
	data_len += 1; //band
	data_len += 1; //channel_nr
	for (i = 0; i < padapter->opclass_scan_result->channel_nr; i++) {
		data_len += 1; //channel
		data_len += 1; //scan_status
		data_len += 31; //timestamp
		data_len += 1; //channel_utilization
		data_len += 1; //noise
		data_len += 2; //neighbor_nr
		for (j = 0; j < padapter->opclass_scan_result->channels[i].neighbor_nr; j++) {
			data_len += 6; //bssid
			data_len += 1; //ssid_length
			data_len += padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length; //ssid
			data_len += 1; //signal strength
			data_len += 1; //channel_band_width
			data_len += 1; //channel_utilization
			data_len += 2; //station_count
		}
	}

	if (data_len > MAX_CHANNELSCAN_BUF_PALOAD)
		goto func_return;

/*write buf*/
	_rtw_memcpy(&send_buf[offset], &data_len, 2); //data_len
	offset += 2;

	if (NULL != strstr(padapter->pnetdev->name, "wlan")) {
		send_buf[offset++]	= padapter->pnetdev->name[4] - '0';
	} else {
		send_buf[offset++]	= 0xFF; //error
	}
	send_buf[offset++]	= padapter->opclass_scan_result->channel_nr;
	for (i = 0; i < padapter->opclass_scan_result->channel_nr; i++) {
		send_buf[offset++]	= padapter->opclass_scan_result->channels[i].channel; //channel
		send_buf[offset++]	= padapter->opclass_scan_result->channels[i].scan_status; //scan_status
		_rtw_memcpy(&send_buf[offset], padapter->opclass_scan_result->channels[i].timestamp, 31);//timestamp
		offset += 31;
		if(padapter->opclass_scan_result->channels[i].channel_utilization < 1) {
			padapter->opclass_scan_result->channels[i].channel_utilization = 1;
		}
		send_buf[offset++]	= padapter->opclass_scan_result->channels[i].channel_utilization; //channel_utilization
		send_buf[offset++]	= padapter->opclass_scan_result->channels[i].noise; //noise
		_rtw_memcpy(&send_buf[offset], &padapter->opclass_scan_result->channels[i].neighbor_nr, 2);//neighbor_nr
		offset += 2;
		for (j = 0; j < padapter->opclass_scan_result->channels[i].neighbor_nr; j++) {
			_rtw_memcpy(&send_buf[offset], padapter->opclass_scan_result->channels[i].neighbors[j].bssid, 6); //neighbor_nr
			offset += 6;
			send_buf[offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length; //ssid_length
			_rtw_memcpy(&send_buf[offset], padapter->opclass_scan_result->channels[i].neighbors[j].ssid, padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length); //ssid
			offset += padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length;
			send_buf[offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].signal_strength; //signal_strength
			send_buf[offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].channel_band_width; //channel_band_width
			send_buf[offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].channel_utilization; //channel_utilization
			_rtw_memcpy(&send_buf[offset], &padapter->opclass_scan_result->channels[i].neighbors[j].station_count, 2); //station_count
			offset += 2;
		}
	}

	wrqu->data.length = data_len;

	ret = 0;

func_return:
	return ret;

}
#endif/* CONFIG_RTW_OPCLASS_CHANNEL_SCAN */

int rtw_atoi_on_base(char *s, int base)
{
	int k = 0;
	int sign = 1;
	if (NULL == s){
		return 0;
	}

	k = 0;
	if (base == 10) {
		if(*s== '-') {
			sign = -1;
			s++;
		}
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
		k *= sign;
	}
	else {
		while (*s != '\0') {
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				//RTW_ERR("error hex format! %c\n", s);
				return k;
			}

			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

#ifdef BEACON_VS_IE
int rtw_beacon_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	unsigned char tmpbuf = 0;
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	int i, tmp_idx = -1;
	u8	service_name_len;
	u8	service_name[64];
	u32	duration;
	u8	data_len;
	u8	ie_data[WLAN_MAX_VENDOR_IE_LEN-3];
	u8	oui[3];
	u32	left_len;
	union iwreq_data wdata;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u32 vendor_ie_mask = 0;
	u8 beacon_num = 0;
	char *data = extra;


	_rtw_memcpy(&wdata, wrqu, sizeof(wdata));
	left_len = wdata.data.length;

	RTW_INFO("[BEACON_VS_IE] set_bcn_vsie\n");

	memcpy(oui, data, 3);
	data += 3;
	left_len -= 3;

	service_name_len = data[0];
	data++;
	left_len--;
	RTW_INFO("service_name_len = %d\n",service_name_len);

	if(left_len < (unsigned int)service_name_len || service_name_len > 64)
	{
		tmpbuf = 1;
		RTW_INFO("service_name_len error: left_len: %d, service_name_len: %d\n",left_len, service_name_len);
		goto exit_bcn_vsie;
	}
	memcpy(service_name, data, service_name_len);
	data += service_name_len;
	left_len -= service_name_len;

	memcpy(&duration, data, sizeof(unsigned int));
	data += sizeof(unsigned int);
	left_len -= sizeof(unsigned int);

	if(duration > 300)
		duration = 300;
	RTW_INFO("duration = %u\n",duration);
	data_len = data[0];
	data++;
	left_len--;

	RTW_INFO("data_len = %d\n",data_len);
	if(left_len < (unsigned int)data_len || data_len > (WLAN_MAX_VENDOR_IE_LEN-3))
	{
		tmpbuf = 1;
		RTW_INFO("data_len error: left_len: %d, data_len: %d\n",left_len,data_len);
		goto exit_bcn_vsie;
	}
	memcpy(ie_data, data, data_len);


	for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
	{
		vendor_ie_mask = pmlmepriv->vendor_ie_mask[i];
		if(vendor_ie_mask & WIFI_BEACON_VENDOR_IE_BIT)
		{
			beacon_num++;
			if(pmlmepriv->vendor_service_name_len[i] == service_name_len &&
			   (memcmp(service_name, pmlmepriv->vendor_service_name[i], service_name_len) == 0 ) &&
				(memcmp(oui,&(pmlmepriv->vendor_ie[i][2]),3) == 0)){
				RTW_INFO("[Exist] Beacon VS IE idx: %d\n", i);
				if(duration == 0)
				{
					_rtw_memset(pmlmepriv->vendor_ie[i] , 0 , sizeof(pmlmepriv->vendor_ie[i]));
					pmlmepriv->vendor_ielen[i] = 0;
					pmlmepriv->vendor_ie_mask[i] = 0;
					_rtw_memset(pmlmepriv->vendor_service_name[i] , 0 , sizeof(pmlmepriv->vendor_service_name[i]));
					pmlmepriv->vendor_service_name_len[i] = 0;
					pmlmepriv->vendor_ie_duration[i] = 0;
					pmlmepriv->vendor_ie_duration_count[i] = 0;

				}
				else
				{
					pmlmepriv->vendor_ie[i][1] = data_len+3;
					memcpy(&(pmlmepriv->vendor_ie[i][5]), ie_data, data_len);
					pmlmepriv->vendor_ie_duration[i] = duration;
					pmlmepriv->vendor_ie_duration_count[i] = 0;
					pmlmepriv->vendor_ielen[i] = data_len+5;
				}
				tmpbuf = 0;
				break;
			}
		}

		if(tmp_idx == -1 && vendor_ie_mask == 0)
			tmp_idx = i;
	}

	if(i == WLAN_MAX_VENDOR_IE_NUM)
	{
		if(duration == 0)
		{
			tmpbuf = 0;
			goto exit_bcn_vsie;
		}

		if(beacon_num == NUM_BCNVSIE_SERVICE)
		{
			tmpbuf = 2;
			goto exit_bcn_vsie;
		}
		else
		{
			if(tmp_idx != -1)
			{
				RTW_INFO("[Insert] Beacon VS IE idx: %d\n", tmp_idx);
				pmlmepriv->vendor_ie[tmp_idx][0] = _VENDOR_SPECIFIC_IE_;
				pmlmepriv->vendor_ie[tmp_idx][1] = data_len+3;
				memcpy(&(pmlmepriv->vendor_ie[tmp_idx][2]), oui, 3);
				memcpy(&(pmlmepriv->vendor_ie[tmp_idx][5]), ie_data, data_len);
				pmlmepriv->vendor_service_name_len[tmp_idx] = service_name_len;
				memcpy(pmlmepriv->vendor_service_name[tmp_idx], service_name, service_name_len);
				pmlmepriv->vendor_ie_duration[tmp_idx] = duration;
				pmlmepriv->vendor_ie_duration_count[tmp_idx] = 0;
				pmlmepriv->vendor_ielen[tmp_idx] = data_len+5;
				pmlmepriv->vendor_ie_mask[tmp_idx] = WIFI_BEACON_VENDOR_IE_BIT;
				tmpbuf = 0;
			}
			else
			{
				tmpbuf = 1;
				RTW_INFO("Beacon VS IE CAN NOT ADD\n");
				goto exit_bcn_vsie;
			}
		}
	}

	if(tmpbuf == 0)
		_update_beacon(padapter, 0xFFF, NULL, _TRUE, 0, "update vendor ie");
#endif
exit_bcn_vsie:
	ret = tmpbuf;
	if (copy_to_user(wrqu->data.pointer, &tmpbuf, 1))
		ret = -EFAULT;

	return ret;
}

int rtw_probe_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	unsigned char tmpbuf = 0;
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	int i, tmp_idx = -1;
	unsigned char	type; //1: add, 0: delete
	unsigned char	oui[3];
	unsigned char	service_name_len;
	unsigned char	service_name[64];
	u32 len ;
	union iwreq_data wdata;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	char *data = extra;
	u32 vendor_ie_mask = 0;
	u8 probe_num = 0;

	_rtw_memcpy(&wdata, wrqu, sizeof(wdata));
	len = wdata.data.length;
	if(len < 6)
	{
		RTW_INFO("Length too short: %d, at least 6\n", len);
		tmpbuf = 1;
		goto exit_prb_vsie;
	}
	type = data[0];
	memcpy(oui, data+1, 3);
	service_name_len = data[4];
	if(len < 5 + service_name_len)
	{
		RTW_INFO("Length too short: %d, at least %d\n", len, 5 + service_name_len);
		tmpbuf = 1;
		goto exit_prb_vsie;
	}
	if(service_name_len < sizeof(service_name))
		memcpy(service_name, data + 5, service_name_len);
	else
	{
		RTW_INFO("service_name length too long: %d, at most %zd\n", service_name_len, sizeof(service_name) - 1);
		tmpbuf = 1;
		goto exit_prb_vsie;
	}

	if(type == 1)
	{

		for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
		{
			vendor_ie_mask = pmlmepriv->vendor_ie_mask[i];
			if(vendor_ie_mask & WIFI_PROBEREQ_RX_VENDOR_IE_BIT)
			{
				probe_num++;
				if(pmlmepriv->vendor_service_name_len[i] == service_name_len &&
				(memcmp(service_name, pmlmepriv->vendor_service_name[i], service_name_len) == 0 ) &&
				(memcmp(oui,&(pmlmepriv->vendor_ie[i][2]),3) == 0)){
					RTW_INFO("[Exist] PROBE VS IE idx: %d\n", i);
					tmpbuf = 0;
					break;
				}
			}

			if(tmp_idx == -1 && vendor_ie_mask == 0)
				tmp_idx = i;

		}

		if(i == WLAN_MAX_VENDOR_IE_NUM)
		{
			if(probe_num == NUM_PRBVSIE_SERVICE)// Probe VSIE table full
			{
				tmpbuf = 2;
				goto exit_prb_vsie;
			}
			else
			{
				if(tmp_idx != -1)
				{
					RTW_INFO("[Insert] PROBE VS IE idx: %d\n", tmp_idx);
					pmlmepriv->vendor_ie[tmp_idx][0] = _VENDOR_SPECIFIC_IE_;
					pmlmepriv->vendor_ie[tmp_idx][1] = 3;
					memcpy(&(pmlmepriv->vendor_ie[tmp_idx][2]), oui, 3);
					pmlmepriv->vendor_service_name_len[tmp_idx] = service_name_len;
					memcpy(pmlmepriv->vendor_service_name[tmp_idx], service_name, service_name_len);
					pmlmepriv->vendor_ielen[tmp_idx] = 5;
					pmlmepriv->vendor_ie_mask[tmp_idx] = WIFI_PROBEREQ_RX_VENDOR_IE_BIT;
					tmpbuf = 0;
				}
				else
				{
					tmpbuf = 1;
					RTW_INFO("PROBE VS IE CAN NOT ADD\n");
					goto exit_prb_vsie;
				}
			}
		}
	}
	else if(type == 0)
	{
		tmpbuf = 0;
		for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
		{
			vendor_ie_mask = pmlmepriv->vendor_ie_mask[i];
			if(vendor_ie_mask & WIFI_PROBEREQ_RX_VENDOR_IE_BIT)
			{
				if(pmlmepriv->vendor_service_name_len[i] == service_name_len &&
				(memcmp(service_name, pmlmepriv->vendor_service_name[i], service_name_len) == 0 ) &&
				(memcmp(oui,&(pmlmepriv->vendor_ie[i][2]),3) == 0)){
				    RTW_INFO("[PROBERSP] DEL:Probe VS IE idx: %d", i);
			    	_rtw_memset(pmlmepriv->vendor_ie[i] , 0 , sizeof(pmlmepriv->vendor_ie[i]));
					pmlmepriv->vendor_ielen[i] = 0;
					pmlmepriv->vendor_ie_mask[i] = 0;
					pmlmepriv->vendor_service_name_len[i] = 0;
					memset(pmlmepriv->vendor_service_name[i], '\0', sizeof(pmlmepriv->vendor_service_name[i]));
					tmpbuf = 0;
					break;
				}
			}
		}
	}
	else
	{
		tmpbuf = 1;
	}
#endif
exit_prb_vsie:
	ret = tmpbuf;
	if (copy_to_user(wrqu->data.pointer, &tmpbuf, 1))
		ret = -EFAULT;

	return ret;

}

int rtw_probersp_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	unsigned char tmpbuf = 0;
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	int i, tmp_idx = -1;
	u8	type; //1: add, 0: delete
	u8	mac[ETH_ALEN];
	u8	iedata_len;
	u8	ie_data[WLAN_MAX_VENDOR_IE_LEN-3];
	u32 len ;
	union iwreq_data wdata;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	char *data = extra;
	u32 vendor_ie_mask = 0;
	u8 prbresp_num = 0;

	_rtw_memcpy(&wdata, wrqu, sizeof(wdata));
	len = wdata.data.length;
	if(len < 6)
	{
		RTW_INFO("Length too short: %d, at least 6\n", len);
		tmpbuf = 1;
		goto exit_prbrsp_vsie;
	}

	type = data[0];
	memcpy(mac, data+1, ETH_ALEN);
	iedata_len = data[1+ETH_ALEN];

	if(len < 1 + ETH_ALEN + 1 + iedata_len)
	{
		RTW_INFO("Length too short: %d, at least %d\n", len, 1 + ETH_ALEN + 1 + iedata_len);
		tmpbuf = 1;
		goto exit_prbrsp_vsie;
	}
	if(iedata_len < (WLAN_MAX_VENDOR_IE_LEN-3))
		memcpy(ie_data, data + 1 + ETH_ALEN + 1, iedata_len);
	else
	{
		memcpy(ie_data, data + 1 + ETH_ALEN + 1, (WLAN_MAX_VENDOR_IE_LEN-3));
		iedata_len = WLAN_MAX_VENDOR_IE_LEN-3;
		RTW_INFO("Length too long: %d, at most %d\n", iedata_len, WLAN_MAX_VENDOR_IE_LEN-3);
	}

	if(type == 1)
	{
		for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
		{
			vendor_ie_mask = pmlmepriv->vendor_ie_mask[i];
			if(vendor_ie_mask & WIFI_PROBERESP_VENDOR_IE_BIT)
			{
				prbresp_num++;
				if(pmlmepriv->vendor_mac_is_set[i] && (memcmp(mac, pmlmepriv->vendor_mac[i], ETH_ALEN) == 0 ) && pmlmepriv->vendor_ielen[i]){
				    RTW_INFO("[Exist] PROBERSP VS IE idx: %d\n", i);
					pmlmepriv->vendor_ie[i][1] = iedata_len;
				    memcpy(&(pmlmepriv->vendor_ie[i][2]), ie_data, iedata_len);
					pmlmepriv->vendor_ielen[i] = iedata_len+2;
					tmpbuf = 0;
					break;
				}
			}

			if(tmp_idx == -1 && vendor_ie_mask == 0)
				tmp_idx = i;

		}

		if(i == WLAN_MAX_VENDOR_IE_NUM)
		{
			if(prbresp_num == NUM_PRBVSIE_SERVICE)// Probe VSIE table full
			{
				tmpbuf = 2;
				goto exit_prbrsp_vsie;
			}
			else
			{
				if(tmp_idx != -1)
				{
					RTW_INFO("[Insert] PROBERSP VS IE idx: %d\n", tmp_idx);
					pmlmepriv->vendor_ie[tmp_idx][0] = _VENDOR_SPECIFIC_IE_;
					pmlmepriv->vendor_ie[tmp_idx][1] = iedata_len;
					memcpy(&(pmlmepriv->vendor_ie[tmp_idx][2]), ie_data, iedata_len);
					pmlmepriv->vendor_mac_is_set[tmp_idx] = 1;
					memcpy(pmlmepriv->vendor_mac[tmp_idx], mac, ETH_ALEN);
					pmlmepriv->vendor_ielen[tmp_idx] = iedata_len+2;
					pmlmepriv->vendor_ie_mask[tmp_idx] = WIFI_PROBERESP_VENDOR_IE_BIT;
					tmpbuf = 0;
				}
				else
				{
					tmpbuf = 1;
					RTW_INFO("PROBERSP VS IE CAN NOT ADD\n");
					goto exit_prbrsp_vsie;
				}
			}
		}
	}
	else if(type == 0)
	{
		tmpbuf = 0;
		for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
		{
			vendor_ie_mask = pmlmepriv->vendor_ie_mask[i];
			if(vendor_ie_mask & WIFI_PROBERESP_VENDOR_IE_BIT)
			{
				if(pmlmepriv->vendor_mac_is_set[i] && (memcmp(mac, pmlmepriv->vendor_mac[i], ETH_ALEN) == 0 ) && pmlmepriv->vendor_ielen[i]){
				    RTW_INFO("[PROBERSP] DEL:Probersp VS IE idx: %d", i);
			    	_rtw_memset(pmlmepriv->vendor_ie[i] , 0 , sizeof(pmlmepriv->vendor_ie[i]));
					pmlmepriv->vendor_ielen[i] = 0;
					pmlmepriv->vendor_ie_mask[i] = 0;
					_rtw_memset(pmlmepriv->vendor_mac[i] , 0 , sizeof(pmlmepriv->vendor_mac[i]));
					pmlmepriv->vendor_mac_is_set[i] = 0;
					tmpbuf = 0;
					break;
				}
			}
		}
	}
	else
	{
		tmpbuf = 1;
	}

#endif
exit_prbrsp_vsie:
	ret = tmpbuf;
	if (copy_to_user(wrqu->data.pointer, &tmpbuf, 1))
		ret = -EFAULT;

	return ret;
}

int rtw_set_vender_ie(struct net_device *dev, u8 type, u32 vendor_ie_mask, u8 *vendor_ie, u32 vendor_ielen)
{
	int ret = 0;
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	int i, tmp_idx = -1;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if(type == WIFI_VSIE_ADD)
	{
		for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
		{
			if(vendor_ie_mask & pmlmepriv->vendor_ie_mask[i])
			{
				if(pmlmepriv->vendor_ielen[i] && (vendor_ielen == (pmlmepriv->vendor_ielen[i] - 2)) && _rtw_memcmp(vendor_ie, &(pmlmepriv->vendor_ie[i][2]), vendor_ielen)){
					RTW_INFO("[Exist] VS IE idx: %d\n", i);
					ret = 0;
					break;
				}
			}

			if(tmp_idx == -1 && pmlmepriv->vendor_ie_mask[i] == 0)
				tmp_idx = i;

		}

		if(i == WLAN_MAX_VENDOR_IE_NUM)
		{
			if(tmp_idx != -1)
			{
				RTW_INFO("[Insert] VS IE idx: %d\n", tmp_idx);
				pmlmepriv->vendor_ie[tmp_idx][0] = _VENDOR_SPECIFIC_IE_;
				pmlmepriv->vendor_ie[tmp_idx][1] = vendor_ielen;
				memcpy(&(pmlmepriv->vendor_ie[tmp_idx][2]), vendor_ie, vendor_ielen);
				pmlmepriv->vendor_ielen[tmp_idx] = vendor_ielen+2;
				pmlmepriv->vendor_ie_mask[tmp_idx] = vendor_ie_mask;
				ret = 0;
			}
			else
			{
				ret = 1;
				RTW_ERR("VS IE CAN NOT ADD\n");
				goto exit_vender_ie;
			}
		}
	}
	else if(type == WIFI_VSIE_DEL)
	{
		ret = 0;
		for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
		{
			if(vendor_ie_mask & pmlmepriv->vendor_ie_mask[i])
			{
				if(pmlmepriv->vendor_ielen[i] && (vendor_ielen == (pmlmepriv->vendor_ielen[i] - 2)) && _rtw_memcmp(vendor_ie, &(pmlmepriv->vendor_ie[i][2]), vendor_ielen)){
					RTW_INFO("[DEL] VS IE idx: %d\n", i);
					_rtw_memset(pmlmepriv->vendor_ie[i] , 0 , sizeof(pmlmepriv->vendor_ie[i]));
					pmlmepriv->vendor_ielen[i] = 0;
					pmlmepriv->vendor_ie_mask[i] = 0;
					ret = 0;
					break;
				}
			}
		}
	}
	else
	{
		ret = 1;
	}

#endif
exit_vender_ie:

	return ret;
}

int rtw_change_cmd_to_vender_ie(struct net_device *dev, char *tmp_ie, u8 type)
{
	int ret = 0;
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	int i;
	char *tmp2, *tmp_oui, *tmp_content, oui_B[3]={'\0'};
	unsigned char vsie[WLAN_MAX_VENDOR_IE_LEN]={0};
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	if(dev == NULL || tmp_ie == NULL)
	{
		RTW_ERR("tmp_ie is NULL\n");
		ret =1;
		goto exit_cmd_to_vender_ie;
	}

	tmp2 = strstr(tmp_ie, ",");
	if(tmp2)
	{
		*tmp2 = '\0';
		tmp_oui = tmp_ie;
		tmp_content = tmp2+1;
		for(i=0;i<3;i++) {
			memcpy(oui_B, tmp_oui+i*2,2);
			vsie[i] = rtw_atoi_on_base(oui_B,16);
		}

		if(strlen(tmp_content) > (WLAN_MAX_VENDOR_IE_LEN - 5))
		{
			RTW_ERR("VSIE Length too long: %zu, at least short than %d\n", strlen(tmp_content),WLAN_MAX_VENDOR_IE_LEN - 5);
			ret = 1;
			goto exit_cmd_to_vender_ie;
		}

		strcpy(&(vsie[3]), tmp_content);
		if(MLME_IS_AP(padapter))
		{
			if(rtw_set_vender_ie(dev, type, WIFI_BEACON_VENDOR_IE_BIT, vsie, strlen(tmp_content)+3) == 0)
				_update_beacon(padapter, 0xFFF, NULL, _TRUE, 0, "update vendor ie");
			rtw_set_vender_ie(dev, type, WIFI_PROBERESP_VENDOR_IE_BIT, vsie, strlen(tmp_content)+3);
		}

		if(MLME_IS_STA(padapter))
			rtw_set_vender_ie(dev, type, WIFI_PROBEREQ_VENDOR_IE_BIT, vsie, strlen(tmp_content)+3);
	}

#endif

exit_cmd_to_vender_ie:

	return ret;
}
int rtw_vsie_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	unsigned char tmpbuf = 0;
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	int i, is_up_beacon=0;
	u32 left_len;
	unsigned char type, vsie_num=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u32 vendor_ie_mask = 0;
	char *data = extra;
	char *tmp, *tmp_ie;
	union iwreq_data wdata;

	_rtw_memcpy(&wdata, wrqu, sizeof(wdata));
	if(wdata.data.length < 7)
	{
		RTW_ERR("Length too short: %d, at least 6\n", wdata.data.length-1);
		goto exit_set_vsie;
	}

	if(strstr(data,"ADD"))
		type = WIFI_VSIE_ADD;
	else if(strstr(data,"DELALL"))
		type = WIFI_VSIE_DELALL;
	else if(strstr(data,"DEL"))
		type = WIFI_VSIE_DEL;
	else
	{
		RTW_ERR("error cmd\n");
		goto exit_set_vsie;
	}

	if(type == WIFI_VSIE_DELALL)
	{
		for(i = 0; i < WLAN_MAX_VENDOR_IE_NUM; i++)
		{
			if(pmlmepriv->vendor_ie_mask[i] && pmlmepriv->vendor_ielen[i])
			{
				RTW_INFO("[DELALL] VS IE idx: %d", i);
				if(is_up_beacon == 0)
				{
					if(pmlmepriv->vendor_ie_mask[i] & WIFI_BEACON_VENDOR_IE_BIT)
						is_up_beacon = 1;
				}
				_rtw_memset(pmlmepriv->vendor_ie[i] , 0 , sizeof(pmlmepriv->vendor_ie[i]));
				pmlmepriv->vendor_ielen[i] = 0;
				pmlmepriv->vendor_ie_mask[i] = 0;
			}
		}

		if(is_up_beacon == 1)
			_update_beacon(padapter, 0xFFF, NULL, _TRUE, 0, "update vendor ie");

		goto exit_set_vsie;
	}

	data += 4;

	left_len = strlen(data);
	if(left_len < 8)
	{
		RTW_ERR("Length too short: %d, at least 8\n", left_len);
		goto exit_set_vsie;
	}

	while(data && (tmp = strstr(data, ";")))
	{
		*tmp = '\0';
		tmp_ie = data;
		data = tmp+1;

		rtw_change_cmd_to_vender_ie(dev, tmp_ie, type);
	}
	rtw_change_cmd_to_vender_ie(dev, data, type);

#endif
exit_set_vsie:
	ret = tmpbuf;
	if (copy_to_user(wrqu->data.pointer, &tmpbuf, 1))
		ret = -EFAULT;

	return ret;
}
#endif

#ifdef CONFIG_RTW_80211K
int rtw_rm_beacon_measurement_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	//TODO
	return 0;
}
#ifdef CONFIG_RTW_WNM
int rtw_bss_trans_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	//TODO
	return 0;
}
#endif
#endif

#ifdef CONFIG_IEEE80211V
int rtw_ctc_bss_trans_request(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = -EPERM;
	int input_len = ETH_ALEN*2 + 1; /* 2 mac address + 1 channel */
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *temp_buf = NULL;
	u8 sta_mac[ETH_ALEN];
	u8 target_bssid[ETH_ALEN];
	u8 channel;
	struct btm_req_hdr btm_para;

	if (wrqu->data.length < input_len || !(wrqu->data.pointer)) {
		ret = -EINVAL;
		goto exit;
	}

	temp_buf = (u8 *)rtw_malloc(input_len);
	if (temp_buf == NULL) {
		ret = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(temp_buf, wrqu->data.pointer, input_len)) {
		ret = -ENOMEM;
		goto exit;
	}

	if(!padapter->registrypriv.wifi_mib.roaming_enable)
		goto exit;

	_rtw_memcpy(sta_mac, temp_buf, ETH_ALEN);
	_rtw_memcpy(&channel, temp_buf + ETH_ALEN, 1);
	_rtw_memcpy(target_bssid, temp_buf + ETH_ALEN + 1, ETH_ALEN);

	_rtw_memset(&btm_para, 0, sizeof(struct btm_req_hdr));
	btm_para.req_mode = (PREFERRED_CANDIDATE_LIST_INCLUDED|ABRIDGED);
	btm_para.disassoc_timer = 0;
	btm_para.validity_interval = 50;

	if(rtw_wnm_issue_btm_request(padapter, sta_mac,
								target_bssid, channel, btm_para, 0) == _SUCCESS)
		ret = 0;
exit:
	if (temp_buf)
		rtw_mfree(temp_buf, wrqu->data.length);
	return ret;
}
#endif /* CONFIG_IEEE80211V */

int rtw_sta_num_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;

	*(u8*)extra = pstapriv->asoc_list_cnt;

	wrqu->data.length = sizeof(pstapriv->asoc_list_cnt);

	return 0;
}

int rtw_drive_version_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	unsigned char *tmpbuf = (u8 *)extra;

	tmpbuf[0] = DRV_VERSION_H;
	tmpbuf[1] = DRV_VERSION_L;
	tmpbuf[2] = DRV_VERSION_SUBL;

	wrqu->data.length = 3;
	return 0;
}

int rtw_ss_status_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	ret = rtw_site_survey_result(dev, info, wrqu, extra);
	return ret;
}

int rtw_join_status_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	return pmlmepriv->to_join;
}

int rtw_bss_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *registrypriv = &(padapter->registrypriv);
	WLAN_BSSID_EX *pbss_network = &padapter->mlmepriv.cur_network.network;
	bss_info_2_web *pBss = (bss_info_2_web *)extra;

	if (rtw_is_adapter_up(padapter) == _FALSE)
	{
		pBss->state = STATE_DISABLED;
	}

	if (padapter->registrypriv.wifi_mib.func_off) {
		pBss->state = STATE_DISABLED;
	}

	if(padapter->mlmepriv.fw_state & WIFI_UNDER_SURVEY)
	{
		pBss->state = STATE_SCANNING;
	}
	else if(padapter->mlmepriv.fw_state & WIFI_UNDER_KEY_HANDSHAKE)
	{
		pBss->state = STATE_WAITFORKEY;
	}
	else if(padapter->mlmepriv.fw_state & WIFI_ASOC_STATE)
	{
		pBss->state = STATE_CONNECTED;
	}
	else
		pBss->state = 0; //AP

	pBss->channel = rtw_get_oper_ch(padapter);

	if(pbss_network->Ssid.SsidLength < sizeof(pBss->ssid))
	{
		memcpy(pBss->ssid, pbss_network->Ssid.Ssid, pbss_network->Ssid.SsidLength);
		pBss->ssid[pbss_network->Ssid.SsidLength] = '\0';
	}
	else
	{
		memcpy(pBss->ssid, pbss_network->Ssid.Ssid, sizeof(pBss->ssid) - 1);
		pBss->ssid[sizeof(pBss->ssid) - 1] = '\0';
		RTW_ERR("In %s(%d), overrunning pBss->ssid\n", __FUNCTION__, __LINE__);
	}

	memcpy(pBss->bssid, pbss_network->MacAddress , ETH_ALEN);

	if (padapter->netif_up == _FALSE || RTW_CANNOT_RUN(adapter_to_dvobj(padapter))) {
		pBss->channel = 0;
		memset(pBss->ssid, 0, sizeof(pBss->ssid));
		memset(pBss->bssid, '\0', MAC_ADDR_LEN);
	}

	wrqu->data.length = sizeof(bss_info_2_web);

	return 0;
}

int rtw_sta_extra_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	struct list_head *phead, *plist;
	int sta_num = 1;
	int rate = 0;
	unsigned char tmp_rate[20] = {0};

	sta_extra_info_2_web *extra_sta_info = (sta_extra_info_2_web *)extra;

	extra_sta_info++;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;

	plist = phead->next;
	while ((plist != phead))
	{
		psta = list_entry(plist, struct sta_info, asoc_list);

		_rtw_memcpy(extra_sta_info->addr, psta->phl_sta->mac_addr, ETH_ALEN);
		extra_sta_info->tx_packets = (u32)sta_tx_pkts(psta);
		extra_sta_info->rx_packets = (u32)sta_rx_pkts(psta);
		extra_sta_info->rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
		extra_sta_info->link_time = psta->link_time;
		extra_sta_info->tx_bytes = psta->sta_stats.tx_bytes;
		extra_sta_info->rx_bytes = psta->sta_stats.rx_bytes;
		if(psta->phl_sta->chandef.bw == CHANNEL_WIDTH_80)
			extra_sta_info->ht_info |= TX_USE_80M_MODE;
		else if(psta->phl_sta->chandef.bw == CHANNEL_WIDTH_40)
			extra_sta_info->ht_info |= TX_USE_40M_MODE;
#ifdef CONFIG_80211N_HT
		if(psta->htpriv.sgi_20m || psta->htpriv.sgi_40m
#ifdef CONFIG_80211AC_VHT
			|| psta->vhtpriv.sgi_80m
#endif
		)
		{
			extra_sta_info->ht_info |= TX_USE_SHORT_GI;
		}
#endif

#ifdef CONFIG_RECORD_CLIENT_HOST
		//extra_sta_info->client_host_name;
		_rtw_memcpy(extra_sta_info->client_host_ip,psta->client_host_ip,4);
#endif
#if defined(RTK_ATM) || defined(RTW_CURRENT_RATE_ACCOUNTING)
	extra_sta_info->tx_bytes_1s = (psta->sta_stats.tx_tp_kbits << 10)/8;
	extra_sta_info->rx_bytes_1s = (psta->sta_stats.rx_tp_kbits << 10)/8;

	//get_sta_cur_tx_rate(padapter, psta);
	get_current_rate(psta, 1, &rate, tmp_rate);
	snprintf(extra_sta_info->txrate, sizeof(extra_sta_info->txrate), "%s", tmp_rate);

	get_current_rate(psta, 0, &rate, tmp_rate);
	snprintf(extra_sta_info->rxrate, sizeof(extra_sta_info->rxrate), "%s", tmp_rate);
#endif

		sta_num++;
		extra_sta_info++;
		plist = plist->next;
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	wrqu->data.length = sizeof(sta_extra_info_2_web) * sta_num;

	return 0;
}

int rtw_misc_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	//TODO
	return 0;
}

int standard_convert_to_web(int band, int standard_prev)
{
	int standard_value = 0; //not defined PROTO_CAP_11A
	if(standard_prev & PROTO_CAP_11AX)
		standard_value |= WLAN_WEB_11AX;
	if(standard_prev & PROTO_CAP_11AC)
		standard_value |= WLAN_WEB_11AC;
	if(standard_prev & PROTO_CAP_11N)
		standard_value |= WLAN_WEB_11N;
	if(standard_prev & PROTO_CAP_11G)
		standard_value |= WLAN_WEB_11G;
	if(standard_prev & PROTO_CAP_11B)
		standard_value |= WLAN_WEB_11B;

	if(band == 2)
		standard_value |= WLAN_WEB_11A;

	return standard_value;
}
#ifdef AP_NEIGHBOR_INFO
int rtw_ap_neighbor_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &padapter->mlmepriv;
	struct ap_neighbor_info *ap_neighbor = &(padapter->dvobj->ap_neighbor);
	ap_neighbor_info_2_web *ent_to_web = (ap_neighbor_info_2_web *)extra;

	int i;
	unsigned long sec;

	_rtw_spinlock_bh(&mlme->lock);

	for(i=0; i<MAX_AP_NEIGHBOR_INFO_NUM; i++)
	{
		if(ap_neighbor->ap_neighbor_info_ent[i].valid == 1)
		{
			if(jiffies/HZ >= ap_neighbor->ap_neighbor_info_ent[i].sec)
				sec = jiffies/HZ - ap_neighbor->ap_neighbor_info_ent[i].sec;
			else
				sec = jiffies/HZ + ~(unsigned long)0/HZ - ap_neighbor->ap_neighbor_info_ent[i].sec;

			ap_neighbor->ap_neighbor_info_ent[i].last_seen_time = sec;
		}
	}

	_rtw_spinunlock_bh(&mlme->lock);

	_rtw_memcpy(ent_to_web->ap_nb_info_ent,ap_neighbor->ap_neighbor_info_ent,sizeof(struct ap_neighbor_info_entry)* MAX_AP_NEIGHBOR_INFO_NUM);

	wrqu->data.length = sizeof(struct ap_neighbor_info_entry) * MAX_AP_NEIGHBOR_INFO_NUM;
	for(i=0; i<MAX_AP_NEIGHBOR_INFO_NUM; i++)
	{
		//band-convert
		ent_to_web->ap_nb_info_ent[i].band = ap_neighbor->ap_neighbor_info_ent[i].band+1;

		//networktype-convert
		if(ap_neighbor->ap_neighbor_info_ent[i].networktype == Ndis802_11Infrastructure)
			ent_to_web->ap_nb_info_ent[i].networktype = 0;
		else if(ap_neighbor->ap_neighbor_info_ent[i].networktype == Ndis802_11IBSS)
			ent_to_web->ap_nb_info_ent[i].networktype = 1;
		else
			ent_to_web->ap_nb_info_ent[i].networktype = 0;

		//standard-convert
		ent_to_web->ap_nb_info_ent[i].standard = standard_convert_to_web(ent_to_web->ap_nb_info_ent[i].band, ent_to_web->ap_nb_info_ent[i].standard);

		//rssi-convert
		if(ent_to_web->ap_nb_info_ent[i].rssi > 10)
			ent_to_web->ap_nb_info_ent[i].rssi -= 10;
		if(ent_to_web->ap_nb_info_ent[i].rssi > 100)
			ent_to_web->ap_nb_info_ent[i].rssi = 100;
	}
	return 0;
}
#endif

#ifdef CONFIG_CTC_FEATURE
#define	STATUS_11K_SUPPORT BIT0
#define	STATUS_11V_SUPPORT BIT1
inline u8 _get_sta_dot11kv_support(struct sta_info *psta)
{
	int val = 0;

#ifdef CONFIG_RTW_80211K
	if(psta->rm_en_cap)
		val |= STATUS_11K_SUPPORT;
#endif
#ifdef CONFIG_RTW_WNM
	if(psta->bss_trans_support
		|| rtw_wnm_get_ext_cap_btm(psta->ext_capab_ie_data))
		val |= STATUS_11V_SUPPORT;
#endif

	return val;
}
#endif

inline u8 _get_sta_network_type(struct sta_info *psta)
{
	int val;

	if(psta->phl_sta->wmode & WLAN_MD_11AX)
		val = WLAN_WEB_11AX;
	else if(psta->phl_sta->wmode & WLAN_MD_11AC)
		val = WLAN_WEB_11AC;
	else if(psta->phl_sta->wmode & WLAN_MD_11N)
		val = WLAN_WEB_11N;
	else if(psta->phl_sta->wmode & WLAN_MD_11G)
		val = WLAN_WEB_11G;
	else if(psta->phl_sta->wmode & WLAN_MD_11A)
		val = WLAN_WEB_11A;
	else if(psta->phl_sta->wmode & WLAN_MD_11B)
		val = WLAN_WEB_11B;
	else
		val = WLAN_WEB_INVALID;

	return val;
}

inline u8 _get_sta_ht_info(struct sta_info *psta)
{
	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI

	u8 result = 0;

	//short gi
	if(psta->htpriv.sgi_20m || psta->htpriv.sgi_40m || psta->vhtpriv.sgi_80m)
		result |= BIT(1);

	switch (psta->phl_sta->chandef.bw){
		case CHANNEL_WIDTH_20:
			result &= 0xfe;
		break;
		case CHANNEL_WIDTH_40:
			result |= BIT(0);
		break;
		case CHANNEL_WIDTH_80:
			result |= BIT(2);
		break;
		case CHANNEL_WIDTH_160:
			result |= BIT(3);
		break;

		default:
			RTW_ERR("[%s][%d] bwmode error \n",__FUNCTION__,__LINE__);
	}

	return result;
}

void _fill_sta_info(sta_info_2_web *sta_info, struct sta_info *psta)
{
	if(!sta_info || !psta)
		return;

	sta_info->aid = psta->phl_sta->aid;
	_rtw_memcpy(sta_info->addr, psta->phl_sta->mac_addr, ETH_ALEN);
	/* TBD: counter 32 to 64 */
	sta_info->tx_packets = (u32)sta_tx_pkts(psta);
	sta_info->rx_packets = (u32)sta_rx_pkts(psta);
	sta_info->expired_time = psta->expire_to * 100 * 2;

	/* TBD: power state */
	sta_info->flags |= STA_INFO_FLAG_ASOC;
	if(psta->state & WIFI_SLEEP_STATE)
		sta_info->flags |= STA_INFO_FLAG_ASLEEP;

	sta_info->rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
	sta_info->link_time = psta->link_time;
	sta_info->tx_fail = psta->sta_stats.tx_fail_cnt;
	sta_info->tx_bytes = psta->sta_stats.tx_bytes;
	sta_info->rx_bytes = psta->sta_stats.rx_bytes;
	sta_info->network = _get_sta_network_type(psta);
	sta_info->RxOperaRate = psta->cur_rx_data_rate;
	sta_info->TxOperaRate = psta->cur_tx_data_rate;
	sta_info->acTxOperaRate = psta->cur_tx_data_rate;
	sta_info->ht_info = _get_sta_ht_info(psta);
	sta_info->multiap_profile = psta->multiap_profile;
#ifdef CONFIG_CTC_FEATURE
	sta_info->status_support = _get_sta_dot11kv_support(psta);
#endif
}

int rtw_sta_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	struct list_head *phead, *plist;
	int sta_num = 1;
	sta_info_2_web *sta_info = (sta_info_2_web *)extra;
	struct	mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	u8 *mybssid  = get_bssid(pmlmepriv);

	sta_info++;

	if(MLME_IS_STA(padapter))
	{
		if(check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE)
		{
			if(!_rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN))
			{
				psta = rtw_get_stainfo(pstapriv, mybssid);
				if(psta && psta->phl_sta && !is_broadcast_mac_addr(psta->phl_sta->mac_addr))
				{
					_fill_sta_info(sta_info, psta);
					sta_num++;
					sta_info++;
				}
			}
		}
	}
	else
	{
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

		phead = &pstapriv->asoc_list;

		plist = phead->next;
		while ((plist != phead))
		{
			psta = list_entry(plist, struct sta_info, asoc_list);
			_fill_sta_info(sta_info, psta);
			sta_num++;
			sta_info++;
			plist = plist->next;
		}

		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	}
	wrqu->data.length = sizeof(sta_info_2_web) * sta_num;

	return 0;
}

#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
int rtw_env_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct envinfo_data *metric_log = (struct envinfo_data *)extra;

	metric_log->rssi_metric = padapter->crossband.metric_log.rssi_metric;
	metric_log->cu_metric = padapter->crossband.metric_log.cu_metric;
	metric_log->noise_metric = padapter->crossband.metric_log.noise_metric;
	wrqu->data.length = sizeof(struct envinfo_data);

	return 0;
}
#endif

#ifdef CONFIG_RTW_80211K
int rtw_rm_beacon_report_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	int ret = -1;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *sta_mac = NULL;
	if (wrqu->data.length < ETH_ALEN|| !(wrqu->data.pointer)) {
		ret = -EINVAL;
		goto exit;
	}

	sta_mac = (u8 *)rtw_malloc(ETH_ALEN);
	if (sta_mac == NULL) {
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(sta_mac, wrqu->data.pointer, ETH_ALEN)) {
		ret = -ENOMEM;
		goto exit;
	}

	ret = rtw_rm_get_sta_beacon_report(padapter, sta_mac, extra);
	if(ret > 0) {
		wrqu->data.length = ret;
		ret               = 0;
	}

exit:
	if (sta_mac)
		rtw_mfree(sta_mac, ETH_ALEN);
	return ret;
}
#endif

struct rtw_cmd_param {
	s32 mode;
	s32 sheet;
	u8 addr[ETH_ALEN];
	s32 channel;
	s32 duration;
};

static int rtw_cmd_param_parser(
	struct rtw_cmd_param *param, struct iw_point *p, char *extra
) {
	char *cmd_raw = NULL;
	char *cmd_raw_p = NULL;
	char *cmd_pair = NULL;
	char *cmd_name = NULL;
	char *cmd_val = NULL;
	int   ret = -EPERM;

	if (p) {
		cmd_raw = (u8 *)rtw_malloc(p->length);
		cmd_raw_p = cmd_raw;

		if (NULL == cmd_raw_p)
			goto func_return;

		if (copy_from_user(cmd_raw, p->pointer, p->length))
			goto func_return;
	}
	else {
		cmd_raw = extra;
	}

	while(1) {
		cmd_pair = strsep(&cmd_raw, ",");
		if (!cmd_pair)
			break;

		cmd_name = strsep(&cmd_pair, "=");
		cmd_val = strsep(&cmd_pair, "");
		if (NULL == cmd_name || NULL == cmd_val) {
			ret = -EINVAL;
			goto func_return;
		}

		#ifdef CONFIG_WIFI_DIAGNOSIS
		if (!strcmp(cmd_name, "mode")) {
			sscanf(cmd_val, "%d", &param->mode);
			param->channel = 0;
			param->duration = 0;
			if (!(param->mode > DIAG_NONE && param->mode < DIAG_MAX)) {
				ret = -EINVAL;
				goto func_return;
			}
		} else if (!strcmp(cmd_name, "addr")) {
			sscanf(cmd_val, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
											&param->addr[0],
											&param->addr[1],
											&param->addr[2],
											&param->addr[3],
											&param->addr[4],
											&param->addr[5]);
			if (param->mode != DIAG_ADD_STA) {
				ret = -EINVAL;
				goto func_return;
			}
		} else if (!strcmp(cmd_name, "ch")) {
			sscanf(cmd_val, "%d", &param->channel);
			if (param->mode != DIAG_UNASSOC_STA
			    && param->mode != DIAG_SPEC_STA) {
				ret = -EINVAL;
				param->channel = 0;
				goto func_return;
			}
		} else if (!strcmp(cmd_name, "dur")) {
			/* unit: in sec */
			sscanf(cmd_val, "%d", &param->duration);
			if (param->mode != DIAG_UNASSOC_STA
			    && param->mode != DIAG_SPEC_STA) {
				ret = -EINVAL;
				param->duration = 0;
				goto func_return;
			}
		} else
		#endif
		if (!strcmp(cmd_name, "sheet")) {
			sscanf(cmd_val, "%d", &param->sheet);
			if (param->sheet < 0) {
				ret = -EINVAL;
				goto func_return;
			}
		}
		else {
			ret = -EINVAL;
			goto func_return;
		}
	}

	ret = 0;

func_return:
	if (NULL != cmd_raw_p)
		rtw_mfree(cmd_raw_p, p->length);

	return ret;
}

#ifdef CONFIG_WIFI_DIAGNOSIS
static u32 rtw_wifi_diag_result_unassoc_sta_process(
	struct wifi_diag_obj *wifi_diag, char *extra)
{
	struct wifi_diag_sta_entry *sta_ent = NULL;
	struct wifi_diag_bss_entry *bss_ent = NULL, *bss_ent_matched = NULL;
	wifi_diag_sta_entry_2_web *ent = (wifi_diag_sta_entry_2_web *)extra;
	u8 i, j;
	u8 ent_num = 0;

	RTW_INFO("[%s]sta_num = %d, bss_num = %d\n",
			__func__,
			wifi_diag->sta_list_num,
			wifi_diag->bss_list_num);
	for (i = 0; i < wifi_diag->sta_list_num; i++) {
		sta_ent = &(wifi_diag->sta_list[i]);
		bss_ent_matched = NULL;
		for (j = 0; j < wifi_diag->bss_list_num; j++) {
			bss_ent = &(wifi_diag->bss_list[j]);

			if (!_rtw_memcmp(sta_ent->bssid, bss_ent->bssid, ETH_ALEN))
				continue;

			bss_ent_matched = bss_ent;
			break;
		}

		_rtw_memset(ent, 0, sizeof(wifi_diag_sta_entry_2_web));

		ent->channel = sta_ent->channel;
		_rtw_memcpy(ent->addr, sta_ent->mac, ETH_ALEN);
		ent->rssi = sta_ent->rssi;
		_rtw_memcpy(ent->bssid, sta_ent->bssid, ETH_ALEN);
		if (bss_ent_matched) {
			_rtw_memcpy(ent->ssid, bss_ent_matched->ssid, 33);
			ent->ssid_length = bss_ent_matched->ssid_length;
			ent->bss_encrypt = bss_ent_matched->encrypt;
			ent->bss_pairwise_cipher = bss_ent_matched->pairwise_cipher;
			ent->bss_group_cipher = bss_ent_matched->group_cipher;
			ent->bss_akm = bss_ent_matched->akm;
		}

		ent->entry = ent_num;
		ent->time_stamp = sta_ent->time_stamp;
		ent->used = 1;
		ent->status = 0;

		RTW_INFO("[STA %d]"MAC_FMT", rssi=%d, ch=%d, ssid=%s, bssid="MAC_FMT"\n",
				ent_num,
				MAC_ARG(ent->addr),
				ent->rssi,
				ent->channel,
				ent->ssid,
				MAC_ARG(ent->bssid));

		ent++;
		ent_num++;
	}

	return (sizeof(wifi_diag_sta_entry_2_web) * ent_num);
}

int rtw_wifi_diag_result(struct net_device *dev,
	struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifi_diag_obj *wifi_diag = &(padapter->dvobj->wifi_diag);
	struct iw_point *p;
	struct rtw_cmd_param  param = {0};
	s32 ret = -EPERM;;

	p = &wrqu->data;
	ret = rtw_cmd_param_parser(&param, p, NULL);
	if (ret < 0)
		goto func_return;

	/* --------------------------------------------- */
	/* start to return the result to the upper layer */
	/* --------------------------------------------- */
	switch(param.mode) {
	case DIAG_UNASSOC_STA:
	case DIAG_SPEC_STA:
		if (wifi_diag->diag_ongoing) {
			ret = -EINPROGRESS;
			goto func_return;
		}
		wrqu->data.length =
			rtw_wifi_diag_result_unassoc_sta_process(wifi_diag, extra);

		break;
	}

	ret = 0;

func_return:
	return ret;
}

static int rtw_wifi_diag_insert_target_mac(struct wifi_diag_obj *wifi_diag,
										   u8 *target_mac)
{
	int idx;
	s32 ret = -EPERM;

	if (wifi_diag->target_list_num >= WIFI_DIAG_TARGET_ENTRY_NUM) {
		RTW_ERR("[%s]Target STA list full!\n", __FUNCTION__);
		return -EALREADY;
	}

	for (idx = 0; idx < wifi_diag->target_list_num; idx++) {
		if(_rtw_memcmp(wifi_diag->target_list[idx].mac, target_mac, ETH_ALEN) == _TRUE) {
			RTW_WARN("[%s]Target STA "MAC_FMT" already in idx %d\n",
				__FUNCTION__,
				MAC_ARG(wifi_diag->target_list[idx].mac),
				idx);
			return 0;
		}
	}

	idx = wifi_diag->target_list_num;
	_rtw_memcpy(wifi_diag->target_list[idx].mac, target_mac, ETH_ALEN);
	wifi_diag->target_list_num++;
	RTW_INFO("[%s]Add Target STA "MAC_FMT", idx %d\n",
			__FUNCTION__,
			MAC_ARG(wifi_diag->target_list[idx].mac),
			idx);
	return 0;
}

static void rtw_wifi_diag_switch_channel(_adapter *adapter,
					struct rtw_cmd_param cmd_param)
{
	struct wifi_diag_obj *wifi_diag = &(adapter->dvobj->wifi_diag);

	if(cmd_param.channel == 0)
		return;

	/* target channel is the same as curent channel,
	   do not need to change channel.
	*/
	if(cmd_param.channel == rtw_get_oper_ch(adapter))
		return;

	wifi_diag->diag_ch_switch = 1;
	rtw_hw_set_ch_bw(adapter,
			cmd_param.channel,
			CHANNEL_WIDTH_20,
			CHAN_OFFSET_NO_EXT,
			_TRUE);
}

int rtw_wifi_diag_start(struct net_device *dev,
	struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifi_diag_obj *wifi_diag = &(padapter->dvobj->wifi_diag);
	struct rtw_cmd_param  param = {0};
	s32 ret = -EPERM;

	ret = rtw_cmd_param_parser(&param, NULL, extra);
	if (ret < 0)
		goto func_return;

	switch(param.mode) {
		case DIAG_UNASSOC_STA:
		case DIAG_SPEC_STA:
			if (wifi_diag->diag_ongoing) {
				ret = -EALREADY;
				goto func_return;
			}

			_rtw_memset(wifi_diag->sta_list, 0, sizeof(wifi_diag->sta_list));
			_rtw_memset(wifi_diag->bss_list, 0, sizeof(wifi_diag->bss_list));
			wifi_diag->sta_list_num = 0;
			wifi_diag->bss_list_num = 0;

			wifi_diag->diag_ch_switch = 0;
			rtw_wifi_diag_switch_channel(padapter, param);
			rtw_hw_set_rx_mode(padapter, PHL_RX_MODE_SNIFFER);
			wifi_diag->mode = param.mode;
			if (param.duration >= 2)
				wifi_diag->diag_ongoing = param.duration>>1;
			else
				wifi_diag->diag_ongoing = WIFI_DIAG_TIME;
			break;
		case DIAG_ADD_STA:
			ret = rtw_wifi_diag_insert_target_mac(wifi_diag, param.addr);
			break;
		default:
			break;
	}

	ret = 0;

func_return:
	return ret;
}
#endif /* CONFIG_WIFI_DIAGNOSIS */


#define STA_ENTRY_NUM_ONE_SHEET 16
int rtw_sta_entry_get(
	struct net_device *dev,
	struct iw_request_info *info,
	union iwreq_data *wrqu,
	char *extra
) {
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv	 *pstapriv = &padapter->stapriv;
	struct sta_info  *psta = NULL;
	struct list_head *phead, *plist;
	struct iw_point  *p;
	struct rtw_cmd_param param = {0};
	s32 ret = -EPERM;
	u8  sta_num = 0;
	u8  sta_num_skip = 0;
	sheet_hdr_2_web *sheet_hdr = (sheet_hdr_2_web *)extra;
	sta_entry_2_web *sta_entry =
		(sta_entry_2_web *)(extra + sizeof(sheet_hdr_2_web));

	p = &wrqu->data;
	ret = rtw_cmd_param_parser(&param, p, NULL);
	if (ret < 0)
		goto func_return;

	/* --------------------------------------------- */
	/* start to return the result to the upper layer */
	/* --------------------------------------------- */
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	phead = &pstapriv->asoc_list;

	plist = phead->next;
	while ((plist != phead))
	{
		psta = list_entry(plist, struct sta_info, asoc_list);

		if (sta_num_skip < param.sheet * STA_ENTRY_NUM_ONE_SHEET) {
			sta_num_skip++;
			continue;
		}

		if (sta_num == STA_ENTRY_NUM_ONE_SHEET)
			break;

		sta_entry->aid = psta->phl_sta->aid;
		_rtw_memcpy(sta_entry->addr, psta->phl_sta->mac_addr, ETH_ALEN);
		sta_entry->link_time = psta->link_time;
		sta_entry->expired_time = psta->expire_to * 100 * 2;

		sta_entry->flags |= STA_INFO_FLAG_ASOC;
		if (psta->state & WIFI_SLEEP_STATE)
			sta_entry->flags |= STA_INFO_FLAG_ASLEEP;

		sta_entry->tx_op_rate = psta->cur_tx_data_rate;
		sta_entry->rx_op_rate = psta->cur_rx_data_rate;
		sta_entry->tx_gi_ltf = psta->cur_tx_gi_ltf;
		sta_entry->rx_gi_ltf = psta->cur_rx_gi_ltf;
		sta_entry->tx_packets = sta_tx_pkts(psta);
		sta_entry->rx_packets = sta_rx_pkts(psta);

		/* TBD: tx_fails */
		sta_entry->tx_fails = 0;
		sta_entry->tx_bytes = psta->sta_stats.tx_bytes;
		sta_entry->rx_bytes = psta->sta_stats.rx_bytes;
		sta_entry->channel_bandwidth = psta->phl_sta->chandef.bw;
		sta_entry->rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
		sta_entry->wireless_mode = psta->phl_sta->wmode;

		if (psta->capability & WLAN_CAPABILITY_PRIVACY) {
			if (psta->wpa_psk & BIT(1)) {
				sta_entry->encrypt = ENCRYP_PROTOCOL_RSN;
				sta_entry->pairwise_cipher = psta->wpa2_pairwise_cipher;
				sta_entry->group_cipher = psta->wpa2_group_cipher;
			}
			else if (psta->wpa_psk & BIT(0)) {
				sta_entry->encrypt = ENCRYP_PROTOCOL_WPA;
				sta_entry->pairwise_cipher = psta->wpa_pairwise_cipher;
				sta_entry->group_cipher = psta->wpa_group_cipher;
			}
			else {
				sta_entry->encrypt = ENCRYP_PROTOCOL_WEP;
			}
		}
		else {
			sta_entry->encrypt = ENCRYP_PROTOCOL_OPENSYS;
		}
		sta_entry->akm = psta->akm_suite_type;

#ifdef CONFIG_RTW_80211K
		sta_entry->rm_cap = psta->rm_en_cap[0];
#endif
#ifdef CONFIG_IEEE80211V
		sta_entry->btm_support = rtw_wnm_get_ext_cap_btm(psta->ext_capab_ie_data);
#endif
		sta_num++;
		sta_entry++;
		plist = plist->next;
	}

	/* bss description header */
	sheet_hdr->sheet_sequence = (u8)param.sheet;
	sheet_hdr->sheet_total = (pstapriv->asoc_list_cnt / STA_ENTRY_NUM_ONE_SHEET) + 1;

	wrqu->data.length =
		sizeof(sta_entry_2_web) * sta_num + sizeof(sheet_hdr_2_web);

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	ret = 0;

func_return:
	return ret;
}

#define BSS_DESC_NUM 64
#define BSS_DESC_NUM_ONE_SHEET 16
int rtw_site_survey_result(struct net_device *dev,
	struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &padapter->dvobj->scanned_queue;
#else
	_queue   *queue = &(padapter->mlmepriv.scanned_queue);
#endif
	_list    *plist, *phead;
	struct iw_point *p;
	struct rtw_cmd_param param = {0};
	s32 ret = -EPERM;
	u8  ss_chk;
	u8  bss_num = 0;
	u8  bss_num_skip = 0;
	u8  bss_num_totale = 0;
	sheet_hdr_2_web *sheet_hdr = (sheet_hdr_2_web *)extra;
	bss_desc_2_web *bss_desc =
		(bss_desc_2_web *)(extra + sizeof(sheet_hdr_2_web));

	p = &wrqu->data;
	ret = rtw_cmd_param_parser(&param, p, NULL);
	if (ret < 0)
		goto func_return;

	/* --------------------------------------------- */
	/* start to return the result to the upper layer */
	/* --------------------------------------------- */
	ss_chk = rtw_sitesurvey_condition_check(padapter, _FALSE);
	if (ss_chk != SS_ALLOW) {
		RTW_INFO("can't do site survey now (reason=%u)!\n", ss_chk);
		if (ss_chk == SS_DENY_SELF_AP_UNDER_SURVEY || ss_chk == SS_DENY_BUDDY_UNDER_SURVEY || ss_chk == SS_DENY_SELF_STA_UNDER_SURVEY)
			ret = -EINPROGRESS;
		else
			ret = -EPERM;

		goto func_return;
	}

	_rtw_spinlock_bh(&(queue->lock));

	phead = get_list_head(queue);
	if (!phead) {
		ret = -EPERM;
		goto queue_unlock;
	}

	plist = get_next(phead);
	if (!plist) {
		ret = -EPERM;
		goto queue_unlock;
	}

	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		struct wlan_network *pnetwork = NULL;
		struct beacon_keys   recv_bcn = {{0}};
		WLAN_BSSID_EX       *pbss = NULL;
		u32 frame_len;
		u8 *frame = NULL;
		u8  i;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		plist = get_next(plist);

		if (!pnetwork || bss_num_totale == BSS_DESC_NUM)
			break;

		/* search for the begining entry according to the sheet sequence */
		if (bss_num_skip < param.sheet * BSS_DESC_NUM_ONE_SHEET) {
			bss_num_skip++;
			bss_num_totale++;
			continue;
		}

		if (bss_num == BSS_DESC_NUM_ONE_SHEET) {
			bss_num_totale++;
			continue;
		}

		pbss = &(pnetwork->network);
		frame_len = pbss->IELength + WLAN_HDR_A3_LEN;
		frame = (u8 *)rtw_malloc(frame_len);
		if (NULL == frame) {
			ret = -EPERM;
			goto queue_unlock;
		}

		_rtw_memcpy(frame + WLAN_HDR_A3_LEN, pbss->IEs, pbss->IELength);

		if (rtw_get_bcn_keys(padapter, frame, frame_len, &recv_bcn) == _FAIL)
			continue;

		rtw_mfree(frame, frame_len);

		_rtw_memset(bss_desc, 0, sizeof(bss_desc_2_web));
		_rtw_memcpy(bss_desc->bssid, pbss->MacAddress, ETH_ALEN);
		_rtw_memcpy(bss_desc->ssid, recv_bcn.ssid, recv_bcn.ssid_len);
		bss_desc->ssidlen = recv_bcn.ssid_len;
		bss_desc->channel = recv_bcn.ch;
		bss_desc->channel_bandwidth = recv_bcn.bw;
		bss_desc->rssi = pbss->PhyInfo.SignalStrength;
		if(bss_desc->rssi > 10)
			bss_desc->rssi -= 10;
		if(bss_desc->rssi > 100)
			bss_desc->rssi = 100;
		bss_desc->signal_quality = pbss->PhyInfo.SignalQuality;
		bss_desc->capability = rtw_get_capability(pbss);
		bss_desc->beacon_period = pbss->Configuration.BeaconPeriod;
		bss_desc->atim_window = pbss->Configuration.ATIMWindow;
		bss_desc->dtim_period = pbss->dtim_prd;
		bss_desc->network_type = pnetwork->network_type;
#ifdef PLATFORM_ECOS
		put_unaligned(le64_to_cpu(*(u64 *)rtw_get_timestampe_from_ie(pnetwork->network.IEs)), &(bss_desc->time_stamp));
#else
		bss_desc->time_stamp =le64_to_cpu(*(u64 *)rtw_get_timestampe_from_ie(pnetwork->network.IEs));
#endif

		for (i = 0; i < recv_bcn.rate_num; i++) {
			u8 rate = recv_bcn.rate_set[i];

			if ((rate & ~(BIT(7))) == IEEE80211_CCK_RATE_1MB) {
				bss_desc->support_rate |= IEEE80211_CCK_RATE_1MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_CCK_RATE_1MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_CCK_RATE_2MB) {
				bss_desc->support_rate |= IEEE80211_CCK_RATE_2MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_CCK_RATE_2MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_CCK_RATE_5MB) {
				bss_desc->support_rate |= IEEE80211_CCK_RATE_5MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_CCK_RATE_5MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_CCK_RATE_11MB) {
				bss_desc->support_rate |= IEEE80211_CCK_RATE_11MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_CCK_RATE_11MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_6MB) {
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_6MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_6MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_9MB) {
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_9MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_9MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_12MB) {
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_12MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_12MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_18MB) {
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_18MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_18MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_24MB)	{
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_24MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_24MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_36MB) {
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_36MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_36MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_48MB) {
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_48MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_48MB_MASK;
			}
			if ((rate & ~(BIT(7))) == IEEE80211_OFDM_RATE_54MB) {
				bss_desc->support_rate |= IEEE80211_OFDM_RATE_54MB_MASK;
				if (recv_bcn.rate_set[i] & BIT(7))
					bss_desc->basic_rate |= IEEE80211_OFDM_RATE_54MB_MASK;
			}
		}

		bss_desc->infra_mode = pbss->InfrastructureMode;

		if (recv_bcn.proto_cap & PROTO_CAP_11B)
			bss_desc->wireless_mode |= WLAN_MD_11B;
		if (recv_bcn.proto_cap & PROTO_CAP_11G)
			bss_desc->wireless_mode |= WLAN_MD_11G;
		if (recv_bcn.ch > 14)
			bss_desc->wireless_mode |= WLAN_MD_11A;
		if (recv_bcn.proto_cap & PROTO_CAP_11N)
			bss_desc->wireless_mode |= WLAN_MD_11N;
		if (recv_bcn.proto_cap & PROTO_CAP_11AC)
			bss_desc->wireless_mode |= WLAN_MD_11AC;
		if (recv_bcn.proto_cap & PROTO_CAP_11AX)
			bss_desc->wireless_mode |= WLAN_MD_11AX;
#ifdef PLATFORM_ECOS
		put_unaligned(recv_bcn.encryp_protocol, &(bss_desc->encrypt));
		put_unaligned(recv_bcn.pairwise_cipher, &(bss_desc->pairwise_cipher));
		put_unaligned(recv_bcn.group_cipher, &(bss_desc->group_cipher));
		put_unaligned(recv_bcn.akm, &(bss_desc->akm));
#else
		bss_desc->encrypt = recv_bcn.encryp_protocol;
		bss_desc->pairwise_cipher = recv_bcn.pairwise_cipher;
		bss_desc->group_cipher = recv_bcn.group_cipher;
		bss_desc->akm = recv_bcn.akm;
#endif

		bss_desc->mfp_opt = recv_bcn.mfp_opt;
		if (bss_desc->mfp_opt == MFP_INVALID) {
			RTW_INFO(FUNC_ADPT_FMT" invalid MFP setting\n", FUNC_ADPT_ARG(padapter));
			ret = -EINVAL;
			goto queue_unlock;
		}

		bss_num++;
		bss_num_totale++;
		bss_desc++;
	}
	/* bss description header */
	sheet_hdr->sheet_sequence = (u8)param.sheet;
	sheet_hdr->sheet_total = (bss_num_totale / BSS_DESC_NUM_ONE_SHEET) + 1;

	wrqu->data.length =
		sizeof(bss_desc_2_web) * bss_num + sizeof(sheet_hdr_2_web);

	ret = 0;

queue_unlock:
	_rtw_spinunlock_bh(&(queue->lock));

func_return:
	return ret;

}

int rtw_site_survey_start(struct net_device *dev,
	struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 ss_chk;
	u8 status = _FALSE;

	ss_chk = rtw_sitesurvey_condition_check(padapter, _FALSE);
	if (ss_chk != SS_ALLOW) {
		RTW_INFO("can't do site survey now (reason=%u)!\n", ss_chk);
#ifdef RTW_BUSY_DENY_SCAN
		padapter->mlmepriv.lastscantime = rtw_get_current_time();
#endif
		if (ss_chk == SS_DENY_SELF_AP_UNDER_SURVEY || ss_chk == SS_DENY_BUDDY_UNDER_SURVEY || ss_chk == SS_DENY_SELF_STA_UNDER_SURVEY)
			return -EALREADY;
		else
			return -EPERM;
	}

	/* TBD: can set parameters */
	status = rtw_set_802_11_bssid_list_scan(padapter, NULL);
	if (status == _FALSE) {
		RTW_INFO("site survey cmd failed!\n");
		return -EPERM;
	}

	return 0;
}

#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_WLAN_DE_SUPPORT)
int rtw_map_get_ap_metric(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, u8 *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *ptmp = NULL;
	int ret = 0;

	if (wrqu->data.length == 0 || !(wrqu->data.pointer)) {
		ret = -EINVAL;
		goto exit;
	}
	ptmp = (u8 *)rtw_malloc(wrqu->data.length);
	if (ptmp == NULL) {
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(ptmp, wrqu->data.pointer, wrqu->data.length)) {
		ret = -ENOMEM;
		goto exit;
	}

#ifndef CONFIG_RTW_PERSIST_IF
	if(rtw_is_adapter_up(padapter))
#endif
	{
		ret = core_map_get_metric(padapter, ptmp, extra);
		if(ret == 0)
			ret = -EPERM;
	}
#ifndef CONFIG_RTW_PERSIST_IF
	else
	{
		ret = -EINVAL;
		RTW_ERR("%s(%d) The interface is not running.\n", __FUNCTION__, __LINE__);
	}
#endif
exit:
	if (ptmp)
		rtw_mfree(ptmp, wrqu->data.length);
	return ret;
}

int rtw_map_get_ap_cap(struct net_device *dev,
		struct iw_request_info *info,
		union iwreq_data *wrqu, u8 *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *ptmp = NULL;
	int ret = 0;

	if (wrqu->data.length == 0 || !(wrqu->data.pointer)) {
			ret = -EINVAL;
			goto exit;
	}
	ptmp = (u8 *)rtw_malloc(wrqu->data.length);
	if (ptmp == NULL) {
			ret = -ENOMEM;
			goto exit;
	}
	if (copy_from_user(ptmp, wrqu->data.pointer, wrqu->data.length)) {
			ret = -ENOMEM;
			goto exit;
	}
	ret = core_map_get_ap_capability(padapter, ptmp, extra);
	if(ret == 0)
		ret = -EPERM;
exit:
	if (ptmp)
		rtw_mfree(ptmp, wrqu->data.length);
	return ret;
}

int rtw_map_get_client_cap(struct net_device *dev,
		struct iw_request_info *info,
		union iwreq_data *wrqu, u8 *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *ptmp = NULL;
	int ret = 0;

	if (wrqu->data.length == 0 || !(wrqu->data.pointer)) {
			ret = -EINVAL;
			goto exit;
	}
	ptmp = (u8 *)rtw_malloc(wrqu->data.length);
	if (ptmp == NULL) {
			ret = -ENOMEM;
			goto exit;
	}
	if (copy_from_user(ptmp, wrqu->data.pointer, wrqu->data.length)) {
			ret = -ENOMEM;
			goto exit;
	}
	ret = core_map_get_client_capability(padapter, ptmp, extra);
	if(ret == 0)
		ret = -EPERM;
exit:
	if (ptmp)
		rtw_mfree(ptmp, wrqu->data.length);
	return ret;
}
#endif

#ifdef CONFIG_RTW_MULTI_AP
int rtw_map_update_acl(struct net_device *dev,
        struct iw_request_info *info,
        struct iw_point *p, u8 *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *ptmp = NULL;
	int ret = 0;

	if (p->length == 0 || !(p->pointer)) {
		ret = -EINVAL;
		goto exit;
	}
	ptmp = (u8 *)rtw_malloc(p->length);
	if (ptmp == NULL) {
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(ptmp, p->pointer, p->length)) {
		ret = -ENOMEM;
		goto exit;
	}
	ret = core_map_update_assoc_control(padapter, ptmp);
exit:
	if (ptmp)
		rtw_mfree(ptmp, p->length);
	return ret;
}

int rtw_map_update_steering_policy(struct net_device *dev,
        struct iw_request_info *info,
        struct iw_point *p, u8 *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *ptmp = NULL;
	int ret = 0;

	if (p->length == 0 || !(p->pointer)) {
		ret = -EINVAL;
		goto exit;
	}
	ptmp = (u8 *)rtw_malloc(p->length);
	if (ptmp == NULL) {
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(ptmp, p->pointer, p->length)) {
		ret = -ENOMEM;
		goto exit;
	}
	ret = core_map_update_steering_policy(padapter, ptmp);
exit:
	if (ptmp)
		rtw_mfree(ptmp, p->length);
	return ret;
}

int rtw_map_update_metric_policy(struct net_device *dev,
        struct iw_request_info *info,
        struct iw_point *p, u8 *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *ptmp = NULL;
	int ret = 0;

	if (p->length == 0 || !(p->pointer)) {
		ret = -EINVAL;
		goto exit;
	}
	ptmp = (u8 *)rtw_malloc(p->length);
	if (ptmp == NULL) {
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(ptmp, p->pointer, p->length)) {
		ret = -ENOMEM;
		goto exit;
	}
	ret = core_map_update_metric_policy(padapter, ptmp);
exit:
	if (ptmp)
		rtw_mfree(ptmp, p->length);
	return ret;
}

int rtw_map_get_asso_sta_metric(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, u8 *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 *ptmp = NULL;
	int ret = 0;

	if (wrqu->data.length == 0 || !(wrqu->data.pointer)) {
		ret = -EINVAL;
		goto exit;
	}
	ptmp = (u8 *)rtw_malloc(wrqu->data.length);
	if (ptmp == NULL) {
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(ptmp, wrqu->data.pointer, wrqu->data.length)) {
		ret = -ENOMEM;
		goto exit;
	}
	ret = core_map_get_asso_sta_metric(padapter, ptmp, extra);
	if(ret == 0)
		ret = -EPERM;
exit:
	if (ptmp)
		rtw_mfree(ptmp, wrqu->data.length);
	return ret;
}
#endif /* CONFIG_RTW_MULTI_AP */

#ifdef RTW_STA_BWC
void decide_limit_tp(_adapter *padapter, int level)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *pstat = NULL;
	unsigned int	tx_average = 0, tx_average_bytes = 0;
	int asoc_cnt = 0;
	struct list_head *phead, *plist;

	if(!pstapriv->asoc_list_cnt)
	{
		printk("SIMON_DEBUG: asoc cnt is 0\n");
		return;
	}

	if(padapter->sta_bwc_total_tp < padapter->tx_tp_limit || (padapter->tx_tp_limit == 0))
	{
		padapter->tx_tp_base = padapter->sta_bwc_total_tp;
	}
	else
	{
		padapter->tx_tp_base = padapter->tx_tp_limit;
	}

	padapter->got_limit_tp = 1;
	padapter->tx_tp_limit = padapter->tx_tp_base * (10 - level + 1) / 10;
	asoc_cnt = pstapriv->asoc_list_cnt;
	padapter->last_asoc_cnt = asoc_cnt;

	tx_average = padapter->tx_tp_limit / asoc_cnt;
	if(tx_average < LOW_BOUNT_LIMIT)
	{
		tx_average = LOW_BOUNT_LIMIT;
		padapter->tx_tp_limit = LOW_BOUNT_LIMIT * asoc_cnt;
	}
	tx_average_bytes = ((tx_average * 1024 / 8) / (HZ / STA_BWC_TO));

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = phead->next;
	while ((plist != phead))
	{
		pstat = list_entry(plist, struct sta_info, asoc_list);
		pstat->sta_bwc_tx_limit = tx_average;
		pstat->sta_bwc_tx_limit_byte = tx_average_bytes;
		plist = plist->next;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	return;
}

void cancel_limit_tp(_adapter *padapter)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *pstat = NULL;
	struct list_head *phead, *plist;

	if(padapter->got_limit_tp)
	{
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		phead = &pstapriv->asoc_list;
		plist = phead->next;
		while ((plist != phead))
		{
			pstat = list_entry(plist, struct sta_info, asoc_list);
			pstat->sta_bwc_tx_limit = 0;
			pstat->sta_bwc_tx_limit_byte = 0;
			plist = plist->next;
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
		padapter->tx_tp_base = 0;
		padapter->tx_tp_limit = 0;
		padapter->got_limit_tp = 0;
	}
	return;
}

unsigned int sta_bwc_get_total_tp(_adapter *padapter)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *pstat = NULL;
	struct list_head *phead, *plist;
	unsigned int total_tp = 0;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = phead->next;
	while ((plist != phead))
	{
		pstat = list_entry(plist, struct sta_info, asoc_list);
		total_tp += pstat->sta_stats.tx_tp_kbits;
		plist = plist->next;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	return total_tp;
}

int sta_bwc_limit_tp(struct net_device *dev, int level)
{
	_adapter *padapter_tmp = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter_tmp);
	struct sta_info *pstat = NULL;
	_adapter *padapter = NULL;
	struct mlme_priv *pmlmepriv = NULL;
	struct wlan_network *cur_network = NULL;
	struct sta_priv *pstapriv = NULL;
	int i;

	if(!dev || level < 0 || level > 10)
		return -1;

	for(i = 0; i < CONFIG_IFACE_NUMBER; i++)
	{
		padapter = dvobj->padapters[i];
		if(rtw_is_adapter_up(padapter))
		{
			if(!MLME_IS_AP(padapter))
			{
				continue;
			}

			pmlmepriv = &(padapter->mlmepriv);
			cur_network = &(pmlmepriv->cur_network);
			pstapriv = &padapter->stapriv;
			pstat = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
			if(!pstat)
			{
				continue;
			}

			padapter->txduty_level = level;
			if(level)
			{
				padapter->txduty = 1;
				padapter->sta_bwc_total_tp = sta_bwc_get_total_tp(padapter);
				decide_limit_tp(padapter, padapter->txduty_level);
			}
			else
			{
				padapter->txduty = 0;
				padapter->sta_bwc_total_tp = 0;
				cancel_limit_tp(padapter);
			}
		}
	}
	return 0;
}
#endif

#ifdef SBWC
int rtw_sbwc_setting(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 sta_addr[ETH_ALEN];
	char cmd[64] = {0}, chr=':';
	unsigned int tx_lmt, rx_lmt, sep = 0;
	struct sta_info *pstat = NULL;
	SBWC_MODE mode = SBWC_MODE_DISABLE;
	unsigned int SBWC_tx_limit_byte = 0, SBWC_rx_limit_byte = 0;
	unsigned int count = padapter->registrypriv.wifi_mib.sbwcEntry.count;
	struct _SBWC_ENTRY *entry = NULL;
	int i , ret = 0;

	if (count >= _NUM_SBWC - 1)
	{
		RTW_WARN("Exseed NUM_SBWC(%d)\n", _NUM_SBWC);
		return -1;
	}

	/* check cmd format */
	for ( i = 0 ; i < strlen(extra); ++i)
	{
		if (extra[i] == ',')
		{
			++sep;
		}
		strncat(cmd, extra+i, 1);
		if((i < 2*ETH_ALEN-1) && (i % 2))
			strncat(cmd, &chr, 1);
	}
	if (sep != 2)
	{
		RTW_WARN("sep[%d] Usage: iwpriv wlanX sta_bw_control [mac],[tx_lmt],[rx_lmt]\n", sep);
		return -1;
	}

	ret = sscanf(cmd, MAC_SFMT",%d,%d",
		&sta_addr[0], &sta_addr[1], &sta_addr[2],
		&sta_addr[3], &sta_addr[4], &sta_addr[5], &tx_lmt, &rx_lmt);
	if (ret != 8)
	{
		RTW_WARN("ret[%d] Usage: iwpriv wlanX sta_bw_control [mac],[tx_lmt],[rx_lmt]\n", ret);
		return -1;
	}

	RTW_INFO("======>%s[%d:%s]set %02X:%02X:%02X:%02X:%02X:%02X, tx_lmt[%d]kbps, rx_lmt[%d]kbps\n",
		__func__, __LINE__, dev->name,
		sta_addr[0], sta_addr[1], sta_addr[2], sta_addr[3], sta_addr[4], sta_addr[5],
		tx_lmt, rx_lmt);

	for (i = 0 ; i < count ; ++i)
	{
		if (!memcmp(padapter->registrypriv.wifi_mib.sbwcEntry.entry[i].mac, sta_addr, MAC_ADDR_LEN))
		{
			entry = &(padapter->registrypriv.wifi_mib.sbwcEntry.entry[i]);
			break;
		}
	}

	if (!entry)
	{
		RTW_WARN("Entry not exist, create one(%d)\n", count);
		entry = &(padapter->registrypriv.wifi_mib.sbwcEntry.entry[count]);
		memcpy(entry->mac, sta_addr, MAC_ADDR_LEN);
		padapter->registrypriv.wifi_mib.sbwcEntry.count++;
	}

	entry->tx_lmt = tx_lmt;
	entry->rx_lmt = rx_lmt;

	pstat = rtw_get_stainfo(&padapter->stapriv, sta_addr);

	if (!pstat)
	{
		RTW_INFO("[%s] can't find specific sta\n", __func__);
		return 0;
	}

	pstat->SBWC_tx_limit = tx_lmt;
	pstat->SBWC_rx_limit = rx_lmt;

	if (tx_lmt)
	{
		mode |= SBWC_MODE_LIMIT_STA_TX;
		SBWC_tx_limit_byte = ((pstat->SBWC_tx_limit * 1024 / 8) / (HZ / SBWC_TO));
	}

	if (rx_lmt)
	{
		mode |= SBWC_MODE_LIMIT_STA_RX;
		SBWC_rx_limit_byte = ((pstat->SBWC_rx_limit * 1024 / 8) / (HZ / SBWC_TO));
	}

	pstat->SBWC_mode = mode;
	pstat->SBWC_tx_limit_byte = SBWC_tx_limit_byte;
	pstat->SBWC_rx_limit_byte = SBWC_rx_limit_byte;

	return 0;
}
#endif

int rtw_dynamic_acl_add_sta(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct ieee_param param;
	int i = 0, ret = 0;
	char cmd[64] = {0}, chr=':', *tmp_ptr=NULL;
	int mac_str_len = strlen(extra);
	u32 valid_time = 0;

	if (!rtw_is_adapter_up(padapter))
		return ret;

	if (mac_str_len < 12){
		RTW_ERR("mac[%s] Usage: iwpriv wlan0 add_acl_table mac_str[,time]\n", extra);
		ret = -1;
		return ret;
	}

	tmp_ptr = strstr(extra, ",");

	if(tmp_ptr)
	{
		mac_str_len = tmp_ptr - extra;
		sscanf(tmp_ptr, ",%u", &valid_time);
	}

	for ( i = 0 ; i < mac_str_len; ++i)
	{
		strncat(cmd, extra+i, 1);
		if(i % 2)
			strncat(cmd, &chr, 1);
	}

	ret = sscanf(cmd, MAC_SFMT, &param.sta_addr[0], &param.sta_addr[1], &param.sta_addr[2],
		&param.sta_addr[3], &param.sta_addr[4], &param.sta_addr[5]);

	RTW_INFO("======>%s[%d:%s]add %02X:%02X:%02X:%02X:%02X:%02X, time = %u\n",
		__func__, __LINE__, dev->name,
		param.sta_addr[0], param.sta_addr[1], param.sta_addr[2],
		param.sta_addr[3], param.sta_addr[4], param.sta_addr[5],
		valid_time);

	rtw_acl_add_sta(padapter, RTW_ACL_PERIOD_BSS, param.sta_addr, valid_time);

	if(padapter->stapriv.acl_list[RTW_ACL_PERIOD_BSS].mode == RTW_ACL_MODE_ACCEPT_UNLESS_LISTED)
		rtw_del_sta(dev, &param);

	return ret;
}

int rtw_dynami_acl_remove_sta_(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct ieee_param param;
	int i = 0, ret = 0;
	char cmd[64] = {0}, chr=':';
	int mac_str_len = strlen(extra);

	if(!rtw_is_adapter_up(padapter))
		return ret;

	if(mac_str_len != 12){
		RTW_ERR("mac[%s] Usage: iwpriv wlan0 rm_acl_table mac_str\n", extra);
		ret = -1;
		return ret;
	}

	for ( i = 0 ; i < mac_str_len; ++i)
	{
		strncat(cmd, extra+i, 1);
		if(i % 2)
			strncat(cmd, &chr, 1);
	}

	ret = sscanf(cmd, MAC_SFMT, &param.sta_addr[0], &param.sta_addr[1], &param.sta_addr[2],
		&param.sta_addr[3], &param.sta_addr[4], &param.sta_addr[5]);

	RTW_INFO("======>%s[%d:%s]remove %02X:%02X:%02X:%02X:%02X:%02X\n",
		__func__, __LINE__, dev->name,
		param.sta_addr[0], param.sta_addr[1], param.sta_addr[2],
		param.sta_addr[3], param.sta_addr[4], param.sta_addr[5]);

	rtw_acl_remove_sta(padapter, RTW_ACL_PERIOD_BSS, param.sta_addr);

	if(padapter->stapriv.acl_list[RTW_ACL_PERIOD_BSS].mode == RTW_ACL_MODE_DENY_UNLESS_LISTED)
		rtw_del_sta(dev, &param);

	return ret;
}

int rtw_dynami_acl_clear_table(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct ieee_param param;
	int ret = 0;
	/*broadcast addr means clear table*/
	param.sta_addr[0]=0xff;
	param.sta_addr[1]=0xff;
	param.sta_addr[2]=0xff;
	param.sta_addr[3]=0xff;
	param.sta_addr[4]=0xff;
	param.sta_addr[5]=0xff;

	rtw_acl_remove_sta(padapter, RTW_ACL_PERIOD_BSS, param.sta_addr);

	return ret;
}

static unsigned int is_acl_sta(_adapter *adapter, unsigned char *hwaddr, unsigned char mode, u8 period)
{
	_list *list, *head;
	int match = 0;
	struct rtw_wlan_acl_node *acl_node;
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	_queue	*acl_node_q;

	if (period >= RTW_ACL_PERIOD_NUM) {
		return match;
	}

	acl = &stapriv->acl_list[period];
	acl_node_q = &acl->acl_node_q;

	_rtw_spinlock_bh(&(acl_node_q->lock));
	head = get_list_head(acl_node_q);
	list = get_next(head);

	while (rtw_end_of_queue_search(head, list) == _FALSE) {
		acl_node = LIST_CONTAINOR(list, struct rtw_wlan_acl_node, list);
		list = get_next(list);
		if (_rtw_memcmp(acl_node->addr, hwaddr, ETH_ALEN)) {
			match = 1;
			break;
		}
	}
	_rtw_spinunlock_bh(&(acl_node_q->lock));
	return match;
}

struct sta_info * get_associated_sta_not_in_acl(struct net_device *dev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info * psta=NULL;
	struct list_head *phead, *plist;;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = phead->next;
	while ((plist != phead))
	{
		psta = list_entry(plist, struct sta_info, asoc_list);
		if(!is_acl_sta(padapter, psta->phl_sta->mac_addr, RTW_ACL_MODE_DENY_UNLESS_LISTED, RTW_ACL_PERIOD_BSS)){
			_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
			return psta;
		}
		plist = plist->next;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	return NULL;;
}

int rtw_dynami_acl_check_table(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	struct list_head *phead, *plist;
	struct ieee_param param;
	int i = 0, ret = 0;

	if(!rtw_is_adapter_up(padapter))
		return ret;

	/*Check ACL_allow list only*/
	if(padapter->stapriv.acl_list[RTW_ACL_PERIOD_BSS].mode != RTW_ACL_MODE_DENY_UNLESS_LISTED)
		return ret;

	psta=get_associated_sta_not_in_acl(dev);
	while(psta)
	{
		_rtw_memcpy(param.sta_addr, psta->phl_sta->mac_addr, ETH_ALEN);
		rtw_del_sta(dev, &param);
		psta=get_associated_sta_not_in_acl(dev);
	}

	return ret;
}

#if defined(WIFI6_THER_CTRL)
#include <ther_ctrl.h>

void sync_mib_wifi6(struct net_device *dev, struct ther_info_s *info)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);

	info->control.cur_ther = rtw_query_ther(dev, NULL, 0);

#if defined(THER_DBG)
	if (padapter->registrypriv.wifi_mib.del_ther) {
		info->control.cur_ther = padapter->registrypriv.wifi_mib.del_ther;

		info->control.ther_hi = padapter->registrypriv.wifi_mib.ther_hi;
		info->control.ther_low = padapter->registrypriv.wifi_mib.ther_low;
		info->control.ther_max = padapter->registrypriv.wifi_mib.ther_max;
	}
#endif

	info->control.ther_dm = padapter->registrypriv.wifi_mib.ther_dm;
	info->control.dbg = padapter->registrypriv.wifi_mib.dbg;

	info->control.tx_tp = (padapter->tp_total_tx>>10);
	info->control.rx_tp = (padapter->tp_total_rx>>10);
}

void set_limit_tp_wifi6(struct net_device *dev, int level)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	extern int sta_bwc_limit_tp(struct net_device *dev, int level);

#ifdef RTW_STA_BWC
	sta_bwc_limit_tp(dev, level);
#endif
}

void set_bandwidth_wifi6(struct net_device *dev, int bw)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
}

void set_txduty_wifi6(struct net_device *dev, int level)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
}

void set_power_wifi6(struct net_device *dev, int low_power)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
}

void set_path_wifi6(struct net_device *dev, int path)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
}

void set_funcoff_wifi6(struct net_device *dev, int enable)
{
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);

	if (enable) {
		rtw_ther_ctrl_set_func_off(dev);
	}
	else {
		rtw_ther_ctrl_release_func_off(dev);
	}
}

#ifdef PLATFORM_LINUX
static struct therctl_ops therctl_wifi6[NUM_WLAN_INTERFACE];

void reg_wifi6_ther_support(struct net_device *dev)
{
	struct therctl_ops *ops = NULL;

	if (!strcmp(dev->name, "wlan0"))
		ops = &therctl_wifi6[0];
	else
		ops = &therctl_wifi6[NUM_WLAN_INTERFACE - 1];

	ops->sync_mib = &sync_mib_wifi6;
	ops->set_limit_tp = &set_limit_tp_wifi6;
	ops->set_bandwidth = &set_bandwidth_wifi6;
	ops->set_txduty = &set_txduty_wifi6;
	ops->set_power = &set_power_wifi6;
	ops->set_path = &set_path_wifi6;
	ops->set_funcoff = &set_funcoff_wifi6;
	ops->dev = dev;
	reg_therctl_ops(ops);
}
EXPORT_SYMBOL(reg_wifi6_ther_support);
#endif
#endif /* WIFI6_THER_CTRL */

int rtw_query_ther(struct net_device *dev, unsigned char *data, unsigned int data_len)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 ther_a = 0,ther_b = 0;

	rtw_phl_rf_get_thermal(GET_HAL_INFO(padapter->dvobj), RF_PATH_A, &ther_a);
	rtw_phl_rf_get_thermal(GET_HAL_INFO(padapter->dvobj), RF_PATH_B, &ther_b);
	snprintf(data, data_len, "%d,%d", ther_a, ther_b);

    return ther_a;
}

void rtw_ther_ctrl_set_func_off(struct net_device *dev)
{
	_adapter *padapter_tmp = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter_tmp);
	_adapter *padapter = NULL;
	u8 i = 0;

	for (i = 0; i < CONFIG_IFACE_NUMBER; i++) {
		padapter = dvobj->padapters[i];
		if (rtw_is_adapter_up(padapter) &&
			padapter->registrypriv.wifi_mib.func_off == 0) {
			padapter->registrypriv.wifi_mib.func_off = 1;
#if defined(WIFI6_THER_CTRL)
			core_mib_func_off_thermal(padapter, NULL, 1);
#else
			core_mib_func_off(padapter, NULL, 1);
#endif
		}
	}
}

void rtw_ther_ctrl_release_func_off(struct net_device *dev)
{
	_adapter *padapter_tmp = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter_tmp);
	_adapter *padapter = NULL;
	u8 i = 0;

	for (i = 0; i < CONFIG_IFACE_NUMBER; i++) {
		padapter = dvobj->padapters[i];
		if (rtw_is_adapter_up(padapter) &&
			padapter->registrypriv.wifi_mib.func_off == 1) {
			padapter->registrypriv.wifi_mib.func_off = 0;
#if defined(WIFI6_THER_CTRL)
			core_mib_func_off_thermal(padapter, NULL, 1);
#else
			core_mib_func_off(padapter, NULL, 1);
#endif
		}
	}
}

int rtw_ioctl_private_set(struct net_device *dev, struct iwreq *wrq, u16 subcmd){
	u8 *extra = NULL;
	int ret = 0;

	if (wrq->u.data.length == 0 || !(wrq->u.data.pointer)) {
		ret = -EINVAL;
		return ret;
	}

	extra = (u8 *)rtw_malloc(wrq->u.data.length + 1);
	if (extra == NULL) {
		ret = -ENOMEM;
		return ret;
	}

	memset(extra, 0, wrq->u.data.length + 1);
	if (copy_from_user(extra, wrq->u.data.pointer, wrq->u.data.length)) {
		rtw_mfree(extra, wrq->u.data.length);
		ret = -EFAULT;
		return ret;
	}

	switch (subcmd){
		case RTL8192CD_IOCTL_SET_MIB:
			ret = rtw_mib_set(dev, NULL, NULL, extra);
			break;
		case RTL8192CD_IOCTL_DEL_STA:
			ret = rtw_ioctl_del_sta(dev, NULL, NULL, extra);
			break;
#ifdef RTW_BLOCK_STA_CONNECT
		case RTL8192CD_IOCTL_BLOCK_STA:
			ret = rtw_ioctl_block_sta(dev, NULL, &wrq->u, extra);
			break;
#endif
		case RTL8192CD_IOCTL_USER_DAEMON_REQUEST:
			ret = rtw_ioctl_daemon_req(dev, NULL, NULL, extra);
			break;
		case SIOCGIWRTLSCANREQ:
			ret = rtw_wifi_ss_req(dev, NULL, NULL, extra);
			break;
		case SIOCGIWRTLJOINREQ:
			ret = rtw_wifi_join(dev, NULL, NULL, extra);
			break;
		case SIOCMIBSYNC:
			ret = rtw_wifi_priv_mib_set(dev , NULL, NULL, extra);
			break;
#ifdef CONFIG_RTW_MULTI_AP
		case RTL8192CD_IOCTL_UPDATE_BSS:
			//TODO
			ret = rtw_ioctl_update_bss(dev , NULL, NULL, extra);
			break;
		case RTL8192CD_IOCTL_SEND_DISASSOC:
			//TODO
			ret = rtw_ioctl_send_disassoc(dev , NULL, NULL, extra);
			break;
		case RTL8192CD_IOCTL_ASSOC_CONTROL:
			ret = rtw_map_update_acl(dev, NULL, &wrq->u.data, extra);
			break;
		case RTL8192CD_IOCTL_AGENT_STEERING:
			ret = rtw_map_update_steering_policy(dev, NULL, &wrq->u.data, extra);
			break;
		case SIOCMAP_BACKHAULSTEER:
			//TODO
			break;
		case SIOCMAP_UPDATEPOLICY:
			ret = rtw_map_update_metric_policy(dev, NULL, &wrq->u.data, wrq->u.data.pointer);
			break;
		case SIOCMAP_SEND_DISASSOC_VXD:
			//TODO
			ret = rtw_ioctl_send_disassoc_vxd(dev, NULL, &wrq->u, extra);
			break;
		case SIOCMAP_SET_TXMAXPOWER:
			ret = rtw_ioctl_set_tx_max_power(dev , NULL , &wrq->u , wrq->u.data.pointer);
			break;
#ifdef CONFIG_RTW_MULTI_AP_R2
		case SIOCMAP_CAC:
			ret = rtw_ioctl_process_cac_request(dev, NULL, NULL, extra);
			break;
#endif /* defined(CONFIG_RTW_MULTI_AP_R2) */
#ifdef DEBUG_MAP_UNASSOC
		case SIOCMAP_GETUNASSOCSTAMETRIC:
			ret = rtw_ioctl_unassoc_sta_metric_get(dev , NULL, NULL, extra);
			break;
#endif
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
		case SIOCOPCLASS_CHANNELSCAN_REQ:
			ret = rtw_opclass_do_site_survey(dev, NULL, &wrq->u, extra);
			break;
#endif

#ifdef BEACON_VS_IE
		case SIOCGISETBCNVSIE:
			ret = rtw_beacon_vsie_set(dev, NULL, &wrq->u, extra);
			break;
		case SIOCGISETPRBVSIE:
			ret = rtw_probe_vsie_set(dev, NULL, &wrq->u, extra);
			break;
		case SIOCGISETPRBRSPVSIE:
			ret = rtw_probersp_vsie_set(dev, NULL, &wrq->u, extra);
			break;
#endif
#ifdef CONFIG_RTW_80211K
		case SIOC11KBEACONREQ:
			ret = rtw_rm_beacon_measurement_request(dev, NULL, NULL, extra);
			break;
#ifdef CONFIG_RTW_WNM
		case SIOC11VBSSTRANSREQ:
			ret = rtw_bss_trans_request(dev, NULL, NULL, extra);
			break;
#endif
#endif
#ifdef CONFIG_IEEE80211V
		case SIOCGICTCBSSTRANSREQ:
			ret = rtw_ctc_bss_trans_request(dev, NULL, &wrq->u, extra);
			break;
#endif /* CONFIG_IEEE80211V */
		default:
			ret = -EIO;
			break;
	}

	rtw_mfree(extra, wrq->u.data.length + 1);

	return ret;

}

#define EXTRA_MAX_LEN (32*1024)
int rtw_ioctl_private_get(struct net_device *dev, struct iwreq *wrq, u16 subcmd){
	u8 *extra = NULL;
	int ret;

	u16 extra_len = 0;
	if(wrq->u.data.length > EXTRA_MAX_LEN)
		extra_len = wrq->u.data.length;
	else
		extra_len = EXTRA_MAX_LEN;

	extra = rtw_vmalloc(extra_len);
	if(!extra){
		RTW_ERR("WARNING: %s(%d) IOCTL_FLAG: %d rtw_vmalloc fail!\n", __FUNCTION__, __LINE__, subcmd);
		ret = -ENOMEM;
		goto fail;
	}

	switch (subcmd){
		case RTL8192CD_IOCTL_GET_MIB:
			ret = rtw_mib_get(dev, NULL, &wrq->u, extra, 0);
			break;
		case SIOCGIWRTLSTAINFO:
			ret = rtw_sta_info_get(dev , NULL , &wrq->u , extra);
			break;
		case SIOCGIWRTLSTANUM:
			ret = rtw_sta_num_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCGIWRTLDRVVERSION:
			ret = rtw_drive_version_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCGIWRTLGETBSSDB:
			ret = rtw_ss_status_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCGIWRTLJOINREQSTATUS:
			ret = rtw_join_status_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCGIWRTLGETBSSINFO:
			ret = rtw_bss_info_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCGIWRTLSTAEXTRAINFO:
			ret = rtw_sta_extra_info_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCMIBINIT:
			ret = rtw_wifi_priv_mib_get(dev , NULL , &wrq->u , extra);
			break;
		case SIOCGMISCDATA:
			ret = rtw_misc_data_get(dev , NULL , &wrq->u , extra);
			break;
#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_WLAN_DE_SUPPORT)
	case SIOCMAP_GETAPMETRIC:
			ret = rtw_map_get_ap_metric(dev , NULL , &wrq->u , extra);
			if(ret > 0){
				wrq->u.data.length = ret;
				ret = 0;
			}
			else
				wrq->u.data.length = 0;
			break;
	case SIOCMAP_GETAPCAPABILITY:
			ret = rtw_map_get_ap_cap(dev , NULL , &wrq->u , extra);
			if(ret > 0){
				wrq->u.data.length = ret;
				ret = 0;
			}
			else
				wrq->u.data.length = 0;
			break;
	case SIOCMAP_GETCLIENTCAPABILITY:
			ret = rtw_map_get_client_cap(dev , NULL , &wrq->u , extra);
			if(ret > 0){
				wrq->u.data.length = ret;
				ret = 0;
			}
			else
				wrq->u.data.length = 0;
			break;
#endif
#ifdef CONFIG_RTW_MULTI_AP
		case SIOCMAP_GETASSOCSTAMETRIC:
			ret = rtw_map_get_asso_sta_metric(dev , NULL , &wrq->u , extra);
			if(ret > 0){
				wrq->u.data.length = ret;
				ret = 0;
			}
			else
				wrq->u.data.length = 0;
			break;
#endif
#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_RTW_OPCLASS_CHANNEL_SCAN)
		case SIOCMAP_GET_AVAILABLE_CHANNELS:
			ret = rtw_ioctl_available_channels_get(dev , NULL , &wrq->u , extra);
			break;
#endif
#ifdef AP_NEIGHBOR_INFO
		case SIOCGAPNEIGHBORINFO:
			ret = rtw_ap_neighbor_info_get(dev , NULL , &wrq->u , extra);
			break;
#endif
#ifdef CONFIG_RTW_80211K
		case SIOC11KBEACONREP:
			ret = rtw_rm_beacon_report_get(dev , NULL , &wrq->u , extra);
			break;
#endif
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
		case SIOCROSSBANDINFOREQ:
			ret = rtw_env_info_get(dev , NULL , &wrq->u , extra);
			break;
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
		case SIOCOPCLASS_CHANNELSCAN_RESP:
			ret = rtw_opclass_scan_rusult_get(dev, NULL, &wrq->u, extra);
			break;
#endif

		default:
			ret = -EIO;
			break;
	}

	if (wrq->u.data.length > extra_len) {
		RTW_ERR("WARNING: %s(%d) IOCTL_FLAG: %d data len(%d) larger than extra_len(%d)!\n",
			__FUNCTION__, __LINE__, subcmd, wrq->u.data.length, extra_len);
		goto fail;
	}

	if (copy_to_user(wrq->u.data.pointer, extra, wrq->u.data.length))
		ret = -EFAULT;

fail:
	if (extra)
		rtw_vmfree(extra, extra_len);

	if(ret > 0){
		ret = 0; // ioctl can't return >0 value
		RTW_ERR("WARNING: %s(%d) IOCTL_FLAG: %d the return value is wrong!\n", __FUNCTION__, __LINE__, subcmd);
	}

	return ret;
}

#ifdef CONFIG_RTW_MULTI_AP
int rtw_ioctl_private_map_general(struct net_device *dev, struct iwreq *wrq)
{
	int ret = -1;
	int processed_bytes = 0;
	u16 tlv_buffer_length;
	u8 operation_type;
	u16 tlv_total_length;
	u8 *tlv_buffer;

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	if (!padapter){
		RTW_ERR("%s %u\n", __FUNCTION__, __LINE__);
		return -EIO;
	}

	/* Create a buffer large enough to hold at least wrq->u.data.length bytes */
	tlv_buffer_length = wrq->u.data.length > EXTRA_MAX_LEN ? wrq->u.data.length : EXTRA_MAX_LEN;
	tlv_buffer        = (u8 *)rtw_vmalloc(tlv_buffer_length);

	if (!tlv_buffer) {
		RTW_ERR("WARNING: %s(%d) IOCTL_FLAG: %u rtw_vmalloc fail!\n", __FUNCTION__, __LINE__, SIOCMAP_GENERAL_IOCTL);
		ret = -ENOMEM;
		goto fail;
	}

	/* First octet is operation type, second and third octets are tlv_length in bytes, can be used in the future to extend functionality */
	_rtw_memcpy(&operation_type, (u8 *)wrq->u.data.pointer, 1);
	_rtw_memcpy(&tlv_total_length, (u8 *)wrq->u.data.pointer + 1, 2);

	/* Copy the tlv_content from TLV_TYPE all the way to the end of the TLV */
	if (copy_from_user(tlv_buffer, (void *)(((u8 *)wrq->u.data.pointer) + 3), tlv_total_length)) {
		ret = -ENOMEM;
		RTW_ERR("%s wrq_len:%u tlv_len:%u tlv_buf:%u\n", padapter->pnetdev->name,
					wrq->u.data.length, tlv_total_length, tlv_buffer_length);
		goto fail;
	}

	switch (operation_type) {
	case MAP_RESERVED:
		RTW_PRINT("WARNING: %s(%d) IOCTL_FLAG: %u reserved !\n", __FUNCTION__, __LINE__, operation_type);
		break;
	case MAP_GET_GENERAL:
		ret = core_map_get_general(padapter, tlv_buffer, wrq->u.data.length, &processed_bytes);
		break;
	case MAP_SET_GENERAL:
		ret = core_map_set_general(padapter, tlv_buffer, wrq->u.data.length, &processed_bytes);
		break;
	case MAP_SEND_5G_BSS_MACS:
		ret = core_map_update_bss(padapter, tlv_buffer, tlv_total_length, wrq->u.data.length, &processed_bytes);
		break;
	case MAP_GET_LINK_METRICS:
		ret = core_map_get_link_metric(padapter, tlv_buffer, wrq->u.data.length, &processed_bytes);
		break;
	case MAP_GET_STA_INFO:
		ret = core_map_get_sta_info(padapter, tlv_buffer, tlv_buffer_length, &processed_bytes);
		break;
	case MAP_GET_CLIENTS_RSSI:
		ret = core_map_get_clients_rssi(padapter, tlv_buffer, tlv_buffer_length, &processed_bytes);
		break;
	default:
		RTW_PRINT("WARNING: %s(%d) IOCTL_FLAG: %u not defined !\n", __FUNCTION__, __LINE__, operation_type);
		break;
	}

	if (ret == 0) {
		if (processed_bytes > wrq->u.data.length) {
			RTW_ERR("WARNING: %s(%d) IOCTL_FLAG: %d processed len(%d) larger than wrq->u.data.length(%d)!\n",
			        __FUNCTION__, __LINE__, SIOCMAP_GENERAL_IOCTL, processed_bytes, wrq->u.data.length);
			ret = -ENOMEM;
			goto fail;
		} else {
			wrq->u.data.length = processed_bytes;
			if (operation_type == MAP_GET_GENERAL ||
				operation_type == MAP_GET_LINK_METRICS ||
				operation_type == MAP_GET_STA_INFO) {
				if (copy_to_user(wrq->u.data.pointer, tlv_buffer, wrq->u.data.length)) {
					wrq->u.data.length = 0;
					ret                = -EFAULT;
					goto fail;
				}
			}
		}
	} else {
		wrq->u.data.length = 0;
		goto fail;
	}

fail:
	if (tlv_buffer) {
		rtw_vmfree(tlv_buffer, tlv_buffer_length);
	}
	return ret;
}
#endif /* CONFIG_RTW_MULTI_AP */

int __rtw_ioctl_private(struct net_device *dev, struct iwreq *wrq, int exec){

	//_adapter *padapter = rtw_netdev_priv(dev);

	u16 subcmd = 0;
	int ret = 0;

	subcmd = wrq->u.data.flags;
	RTW_DBG("======>get subcmd [%04X]\n", subcmd);

	switch (subcmd){
		case RTL8192CD_IOCTL_SET_MIB:
		case RTL8192CD_IOCTL_DEL_STA:
#ifdef RTW_BLOCK_STA_CONNECT
		case RTL8192CD_IOCTL_BLOCK_STA:
#endif
		case RTL8192CD_IOCTL_USER_DAEMON_REQUEST:
		case SIOCGIWRTLSCANREQ:
		case SIOCGIWRTLJOINREQ:
		case SIOCMIBSYNC:
#ifdef CONFIG_RTW_MULTI_AP
		case RTL8192CD_IOCTL_UPDATE_BSS:
		case RTL8192CD_IOCTL_SEND_DISASSOC:
		case RTL8192CD_IOCTL_ASSOC_CONTROL:
		case RTL8192CD_IOCTL_AGENT_STEERING:
		case SIOCMAP_BACKHAULSTEER:
		case SIOCMAP_UPDATEPOLICY:
		case SIOCMAP_SEND_DISASSOC_VXD:
		case SIOCMAP_SET_TXMAXPOWER:
		case SIOCMAP_CAC:
		case SIOCMAP_GETUNASSOCSTAMETRIC:
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
		case SIOCOPCLASS_CHANNELSCAN_REQ:
#endif
#ifdef BEACON_VS_IE
		case SIOCGISETBCNVSIE:
		case SIOCGISETPRBVSIE:
		case SIOCGISETPRBRSPVSIE:
#endif
#ifdef CONFIG_RTW_80211K
		case SIOC11KBEACONREQ:
#ifdef CONFIG_RTW_WNM
		case SIOC11VBSSTRANSREQ:
#endif
#endif
#ifdef CONFIG_IEEE80211V
		case SIOCGICTCBSSTRANSREQ:
#endif /* CONFIG_IEEE80211V */
#if defined(CPTCFG_WFO_VIRT_MODULE) && !defined(PLATFORM_ECOS)
			if (exec==0) {
				/* get dir for wfo_virt_ioctl */
				ret |= (1<<WFO_IOCTL_SET);
			} else
#endif /* CPTCFG_WFO_VIRT_MODULE & !PLATFORM_ECOS */
			ret = rtw_ioctl_private_set(dev, wrq, subcmd);
			break;
		case RTL8192CD_IOCTL_GET_MIB:
		case SIOCGIWRTLSTAINFO:
		case SIOCGIWRTLSTANUM:
		case SIOCGIWRTLDRVVERSION:
		case SIOCGIWRTLGETBSSDB:
		case SIOCGIWRTLJOINREQSTATUS:
		case SIOCGIWRTLGETBSSINFO:
		case SIOCGIWRTLSTAEXTRAINFO:
		case SIOCMIBINIT:
		case SIOCGMISCDATA:
#ifdef CONFIG_RTW_MULTI_AP
		case SIOCMAP_GETASSOCSTAMETRIC:
#endif
#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_WLAN_DE_SUPPORT)
		case SIOCMAP_GETCLIENTCAPABILITY:
		case SIOCMAP_GETAPMETRIC:
		case SIOCMAP_GETAPCAPABILITY:
#endif
#ifdef AP_NEIGHBOR_INFO
		case SIOCGAPNEIGHBORINFO:
#endif
#ifdef CONFIG_RTW_80211K
		case SIOC11KBEACONREP:
#endif
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
		case SIOCROSSBANDINFOREQ:
#endif
#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_RTW_OPCLASS_CHANNEL_SCAN)
		case SIOCMAP_GET_AVAILABLE_CHANNELS:
#endif
#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
		case SIOCOPCLASS_CHANNELSCAN_RESP:
#endif
#if defined(CPTCFG_WFO_VIRT_MODULE) && !defined(PLATFORM_ECOS)
			if (exec==0) {
				/* get dir for wfo_virt_ioctl */
				ret |= (1<<WFO_IOCTL_GET);
				ret |= (1<<WFO_IOCTL_SET);
			} else
#endif /* CPTCFG_WFO_VIRT_MODULE & !PLATFORM_ECOS */
			ret = rtw_ioctl_private_get(dev, wrq, subcmd);
			break;
#ifdef CONFIG_WLAN_DE_SUPPORT
		case SIOCDE_GETPRIOINFO:
		case SIOCDE_GETRADIOTIMEUSEINFO:
		case SIOCDE_GETBSSINFO:
		case SIOCDE_GETSTAWIFI6CAPABILITY:
		case SIOCDE_GETRADIOCACINFO:
		case SIOCDE_GETRADIOWIFI6APROLE:
		case SIOCDE_GETRADIOWIFI6STAROLE:
		case SIOCDE_GETDEVCACSTATUS:
		case SIOCDE_GETSTALASTRATEINFO:
		case SIOCDE_GETSTAUSEINFO:
		case SIOCDE_GETSTATIDINFO:
		case SIOCDE_GETASSOCDATA:
		case SIOCDE_GETDISASSOCDATA:
		case SIOCDE_GETFAILCONNDATA:
			ret = rtw_ioctl_de_private_get(dev,wrq,subcmd);
			break;
		case SIOCDE_SETDEVCACREQ:
			ret = rtw_ioctl_de_private_set(dev,wrq,subcmd);
			break;
#endif
#ifdef CONFIG_RTW_MULTI_AP
		case SIOCMAP_GENERAL_IOCTL:
#if defined(CPTCFG_WFO_VIRT_MODULE) && !defined(PLATFORM_ECOS)
			if (exec==0) {
				/* get dir for wfo_virt_ioctl */
				ret |= (1<<WFO_IOCTL_GET);
				ret |= (1<<WFO_IOCTL_SET);
			} else
#endif /* CPTCFG_WFO_VIRT_MODULE & !PLATFORM_ECOS */
			ret = rtw_ioctl_private_map_general(dev, wrq);
			break;
#endif
		default:
#if defined(CPTCFG_WFO_VIRT_MODULE) && !defined(PLATFORM_ECOS)
			if (exec==0) {
				/* do nothing */
			} else
#endif /* CPTCFG_WFO_VIRT_MODULE & !PLATFORM_ECOS */
			ret = -EIO;
			break;
	}

	return ret;
}
EXPORT_SYMBOL(__rtw_ioctl_private);

int rtw_ioctl_private(struct net_device *dev, struct iwreq *wrq)
{
	return __rtw_ioctl_private(dev, wrq, 1);
}
#endif /* CONFIG_RTW_AP_EXT_SUPPORT */
