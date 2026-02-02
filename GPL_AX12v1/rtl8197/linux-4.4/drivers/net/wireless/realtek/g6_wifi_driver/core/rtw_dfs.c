#define _RTW_DFS_C_

#include <drv_types.h>


enum rtw_dfs_regd rtw_rfctl_get_dfs_domain(struct rf_ctl_t *rfctl)
{
#ifdef CONFIG_DFS_MASTER
	return rfctl->dfs_region_domain;
#else
	return RTW_DFS_REGD_NONE;
#endif
}

bool rtw_rfctl_dfs_domain_unknown(struct rf_ctl_t *rfctl)
{
#ifdef CONFIG_DFS_MASTER
	return rtw_rfctl_get_dfs_domain(rfctl) == RTW_DFS_REGD_NONE;
#else
	return true;
#endif
}

void rtw_rfctl_dfs_init(struct rf_ctl_t *rfctl, struct registry_priv *regsty)
{
	rfctl->ch_sel_within_same_band = 1;

#ifdef CONFIG_DFS_MASTER
	rfctl->dfs_region_domain = regsty->dfs_region_domain;
	rfctl->cac_start_time = rfctl->cac_end_time = RTW_CAC_STOPPED;
	rtw_init_timer(&(rfctl->radar_detect_timer), rtw_dfs_rd_timer_hdl, rfctl);
#endif

#if CONFIG_DFS_SLAVE_WITH_RADAR_DETECT
	rfctl->dfs_slave_with_rd = 1;
#endif
}

#ifdef CONFIG_DFS_MASTER
void rtw_dfs_backup_non_ocp_time(RT_CHANNEL_INFO *ch_set, systime *non_ocp_arr) {
	int i,chan_num;

	for (i = 0; i < MAX_CHANNEL_NUM && ch_set[i].ChannelNum != 0; i++) {
		if ((ch_set[i].flags & RTW_CHF_DFS) && CH_IS_NON_OCP(&ch_set[i]))
			non_ocp_arr[i] = ch_set[i].non_ocp_end_time;
	}
}

void rtw_dfs_restore_non_ocp_time(RT_CHANNEL_INFO *ch_set, systime *non_ocp_arr) {
	int i,chan_num;

	for (i = 0; i < MAX_CHANNEL_NUM && ch_set[i].ChannelNum != 0; i++) {
		if ((ch_set[i].flags & RTW_CHF_DFS) && non_ocp_arr[i]!=0)
			ch_set[i].non_ocp_end_time = non_ocp_arr[i];
	}
}

/*
* called in rtw_dfs_rd_enable()
* assume the request channel coverage is DFS range
* base on the current status and the request channel coverage to check if need to reset complete CAC time
*/
bool rtw_is_cac_reset_needed(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset)
{
	bool needed = _FALSE;
	u32 cur_hi, cur_lo, hi, lo;

	if (rfctl->radar_detected == 1) {
		needed = _TRUE;
		goto exit;
	}

	if (rfctl->radar_detect_ch == 0) {
		needed = _TRUE;
		goto exit;
	}

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == _FALSE) {
		RTW_ERR("request detection range ch:%u, bw:%u, offset:%u\n", ch, bw, offset);
		rtw_warn_on(1);
	}

	if (rtw_chbw_to_freq_range(rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset, &cur_hi, &cur_lo) == _FALSE) {
		RTW_ERR("cur detection range ch:%u, bw:%u, offset:%u\n", rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset);
		rtw_warn_on(1);
	}

	if (hi <= lo || cur_hi <= cur_lo) {
		RTW_ERR("hi:%u, lo:%u, cur_hi:%u, cur_lo:%u\n", hi, lo, cur_hi, cur_lo);
		rtw_warn_on(1);
	}

	if (rtw_is_range_a_in_b(hi, lo, cur_hi, cur_lo)) {
		/* request is in current detect range */
		goto exit;
	}

	/* check if request channel coverage has new range and the new range is in DFS range */
	if (!rtw_is_range_overlap(hi, lo, cur_hi, cur_lo)) {
		/* request has no overlap with current */
		needed = _TRUE;
	} else if (rtw_is_range_a_in_b(cur_hi, cur_lo, hi, lo)) {
		/* request is supper set of current */
		if ((hi != cur_hi && rtw_chset_is_dfs_range(rfctl->channel_set, hi, cur_hi))
			|| (lo != cur_lo && rtw_chset_is_dfs_range(rfctl->channel_set, cur_lo, lo)))
			needed = _TRUE;
	} else {
		/* request is not supper set of current, but has overlap */
		if ((lo < cur_lo && rtw_chset_is_dfs_range(rfctl->channel_set, cur_lo, lo))
			|| (hi > cur_hi && rtw_chset_is_dfs_range(rfctl->channel_set, hi, cur_hi)))
			needed = _TRUE;
	}

exit:
	return needed;
}

bool _rtw_rfctl_overlap_radar_detect_ch(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset)
{
	bool ret = _FALSE;
	u32 hi = 0, lo = 0;
	u32 r_hi = 0, r_lo = 0;
	int i;

	if(ch == 0)
		goto exit;

	if (rfctl->radar_detect_by_others)
		goto exit;

	if (rfctl->radar_detect_ch == 0)
		goto exit;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == _FALSE) {
		rtw_warn_on(1);
		goto exit;
	}

	if (rtw_chbw_to_freq_range(rfctl->radar_detect_ch
			, rfctl->radar_detect_bw, rfctl->radar_detect_offset
			, &r_hi, &r_lo) == _FALSE) {
		rtw_warn_on(1);
		goto exit;
	}

	if (rtw_is_range_overlap(hi, lo, r_hi, r_lo))
		ret = _TRUE;

exit:
	return ret;
}

bool rtw_rfctl_overlap_radar_detect_ch(struct rf_ctl_t *rfctl)
{
	return _rtw_rfctl_overlap_radar_detect_ch(rfctl
				, rfctl_to_dvobj(rfctl)->oper_channel
				, rfctl_to_dvobj(rfctl)->oper_bwmode
				, rfctl_to_dvobj(rfctl)->oper_ch_offset);
}

bool rtw_rfctl_is_tx_blocked_by_ch_waiting(struct rf_ctl_t *rfctl)
{
	return rtw_rfctl_overlap_radar_detect_ch(rfctl) && IS_CH_WAITING(rfctl);
}

bool rtw_chset_is_chbw_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset)
{
	bool ret = _FALSE;
	u32 hi = 0, lo = 0;
	int i;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == _FALSE)
		goto exit;

	for (i = 0; i < MAX_CHANNEL_NUM && ch_set[i].ChannelNum != 0; i++) {
		if (!rtw_ch2freq(ch_set[i].ChannelNum)) {
			rtw_warn_on(1);
			continue;
		}

		if (!CH_IS_NON_OCP(&ch_set[i]))
			continue;

#if defined (CONFIG_DFS_CHAN_SEL_R_SHORTEST_WAIT)
		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi) {
			ret = _TRUE;
			break;
		}
#endif

#ifdef CONFIG_DFS_CHAN_SEL_G_RANDOM
		if(ch_set[i].ChannelNum == ch) {
			ret = _TRUE;
			break;
		}
#endif
	}

exit:
	return ret;
}

bool rtw_chset_is_ch_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch)
{
	return rtw_chset_is_chbw_non_ocp(ch_set, ch, CHANNEL_WIDTH_20, CHAN_OFFSET_NO_EXT);
}

u32 rtw_chset_get_ch_non_ocp_ms(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset)
{
	int ms = 0;
	systime current_time;
	u32 hi = 0, lo = 0;
	int i;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == _FALSE)
		goto exit;

	current_time = rtw_get_current_time();

	for (i = 0; i < MAX_CHANNEL_NUM && ch_set[i].ChannelNum != 0; i++) {
		if (!rtw_ch2freq(ch_set[i].ChannelNum)) {
			rtw_warn_on(1);
			continue;
		}

		if (!CH_IS_NON_OCP(&ch_set[i]))
			continue;

		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi
		) {
			if (rtw_systime_to_ms(ch_set[i].non_ocp_end_time - current_time) > ms)
				ms = rtw_systime_to_ms(ch_set[i].non_ocp_end_time - current_time);
		}
	}

exit:
	return ms;
}

/**
 * rtw_chset_update_non_ocp - update non_ocp_end_time according to the given @ch, @bw, @offset into @ch_set
 * @ch_set: the given channel set
 * @ch: channel number on which radar is detected
 * @bw: bandwidth on which radar is detected
 * @offset: bandwidth offset on which radar is detected
 * @ms: ms to add from now to update non_ocp_end_time, ms < 0 means use NON_OCP_TIME_MS
 */
static bool _rtw_chset_update_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset, int ms)
{
	u32 hi = 0, lo = 0;
	int i;
	bool updated = 0;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == _FALSE)
		goto exit;

	for (i = 0; i < MAX_CHANNEL_NUM && ch_set[i].ChannelNum != 0; i++) {
		if (!rtw_ch2freq(ch_set[i].ChannelNum)) {
			rtw_warn_on(1);
			continue;
		}

#if defined (CONFIG_DFS_CHAN_SEL_R_SHORTEST_WAIT)
		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi) {

			if (ms >= 0)
				ch_set[i].non_ocp_end_time = rtw_get_current_time() + rtw_ms_to_systime(ms);
			else
				ch_set[i].non_ocp_end_time = rtw_get_current_time() + rtw_ms_to_systime(NON_OCP_TIME_MS);
		}
#endif

#ifdef CONFIG_DFS_CHAN_SEL_G_RANDOM
		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi) {
				ch_set[i].is_bw_selected = 1;
		}

		if(ch_set[i].ChannelNum == ch) {
			if (ms >= 0)
				ch_set[i].non_ocp_end_time = rtw_get_current_time() + rtw_ms_to_systime(ms);
			else
				ch_set[i].non_ocp_end_time = rtw_get_current_time() + rtw_ms_to_systime(NON_OCP_TIME_MS);
		}
#endif
	}

exit:
	return updated;
}

inline bool rtw_chset_update_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset)
{
	return _rtw_chset_update_non_ocp(ch_set, ch, bw, offset, -1);
}

inline bool rtw_chset_update_non_ocp_ms(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset, int ms)
{
	return _rtw_chset_update_non_ocp(ch_set, ch, bw, offset, ms);
}

u32 rtw_get_ch_waiting_ms(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset, u32 *r_non_ocp_ms, u32 *r_cac_ms)
{
	struct dvobj_priv *dvobj = rfctl_to_dvobj(rfctl);
	u32 non_ocp_ms;
	u32 cac_ms;
	u8 in_rd_range = 0; /* if in current radar detection range*/

	if (rtw_chset_is_chbw_non_ocp(rfctl->channel_set, ch, bw, offset))
		non_ocp_ms = rtw_chset_get_ch_non_ocp_ms(rfctl->channel_set, ch, bw, offset);
	else
		non_ocp_ms = 0;

	if (rfctl->radar_detect_enabled) {
		u32 cur_hi, cur_lo, hi, lo;

		if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == _FALSE) {
			RTW_ERR("input range ch:%u, bw:%u, offset:%u\n", ch, bw, offset);
			rtw_warn_on(1);
		}

		if (rtw_chbw_to_freq_range(rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset, &cur_hi, &cur_lo) == _FALSE) {
			RTW_ERR("cur detection range ch:%u, bw:%u, offset:%u\n", rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset);
			rtw_warn_on(1);
		}

		if (rtw_is_range_a_in_b(hi, lo, cur_hi, cur_lo))
			in_rd_range = 1;
	}

	if (!rtw_chset_is_dfs_chbw(rfctl->channel_set, ch, bw, offset))
		cac_ms = 0;
	else if (in_rd_range && !non_ocp_ms) {
		if (IS_CH_WAITING(rfctl))
			cac_ms = rtw_systime_to_ms(rfctl->cac_end_time - rtw_get_current_time());
		else
			cac_ms = 0;
	} else if (rtw_is_long_cac_ch(ch, bw, offset, rtw_dfs_get_region_domain(dvobj)))
		cac_ms = CAC_TIME_CE_MS;
	else
		cac_ms = CAC_TIME_MS;

	if (r_non_ocp_ms)
		*r_non_ocp_ms = non_ocp_ms;
	if (r_cac_ms)
		*r_cac_ms = cac_ms;

	return non_ocp_ms + cac_ms;
}

void rtw_reset_cac(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset)
{
	u32 non_ocp_ms;
	u32 cac_ms;

	rtw_get_ch_waiting_ms(rfctl
		, ch
		, bw
		, offset
		, &non_ocp_ms
		, &cac_ms
	);

	rfctl->cac_start_time = rtw_get_current_time() + rtw_ms_to_systime(non_ocp_ms);
	rfctl->cac_end_time = rfctl->cac_start_time + rtw_ms_to_systime(cac_ms);

	/* skip special value */
	if (rfctl->cac_start_time == RTW_CAC_STOPPED) {
		rfctl->cac_start_time++;
		rfctl->cac_end_time++;
	}
	if (rfctl->cac_end_time == RTW_CAC_STOPPED)
		rfctl->cac_end_time++;
}

u32 rtw_force_stop_cac(struct rf_ctl_t *rfctl, u32 timeout_ms)
{
	struct dvobj_priv *dvobj = rfctl_to_dvobj(rfctl);
	systime start;
	u32 pass_ms;

	start = rtw_get_current_time();

	rfctl->cac_force_stop = 1;

	while (rtw_get_passing_time_ms(start) <= timeout_ms
		&& IS_UNDER_CAC(rfctl)
	) {
		if (dev_is_surprise_removed(dvobj) || dev_is_drv_stopped(dvobj))
			break;
		rtw_msleep_os(20);
	}

	if (IS_UNDER_CAC(rfctl)) {
		if (!dev_is_surprise_removed(dvobj) && !dev_is_drv_stopped(dvobj))
			RTW_INFO("%s waiting for cac stop timeout!\n", __func__);
	}

	rfctl->cac_force_stop = 0;

	pass_ms = rtw_get_passing_time_ms(start);

	return pass_ms;
}


void rtw_init_dfs_region(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct registry_priv *registry = adapter_to_regsty(padapter);

	dvobj->phl_com->dfs_info.region_domain = registry->wifi_mib.dfs_regions;
}

u8 rtw_dfs_rd_hdl(_adapter *adapter)
{
	struct dvobj_priv *dvobj = NULL;
	struct rf_ctl_t *rfctl = NULL;

	if (adapter) {
		dvobj = adapter_to_dvobj(adapter);
		rfctl = adapter_to_rfctl(adapter);

		if (!rfctl->radar_detect_enabled)
			goto exit;

		if (dvobj->oper_channel != rfctl->radar_detect_ch
			|| rtw_get_passing_time_ms(rtw_get_on_oper_ch_time(adapter)) < 300
		) {
			/* offchannel, bypass radar detect */
			goto cac_status_chk;
		}

		if (IS_CH_WAITING(rfctl) && !IS_UNDER_CAC(rfctl)) {
			/* non_ocp, bypass radar detect */
			goto cac_status_chk;
		}

		if (!rfctl->dbg_dfs_fake_radar_detect_cnt
#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
			&& !rfctl->radar_detect_map_member
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
			&& rtw_dfs_hal_radar_detect(adapter) != _TRUE)
			goto cac_status_chk;

		if (!rfctl->dbg_dfs_fake_radar_detect_cnt
			&& rfctl->dbg_dfs_radar_detect_trigger_non
		) {
			/* radar detect debug mode, trigger no mlme flow */
			RTW_INFO("%s radar detected on test mode, trigger no mlme flow\n", __func__);
			goto cac_status_chk;
		}

		if (rfctl->dbg_dfs_fake_radar_detect_cnt != 0) {
			RTW_INFO("%s fake radar detected, cnt:%d\n", __func__
				, rfctl->dbg_dfs_fake_radar_detect_cnt);
			rfctl->dbg_dfs_fake_radar_detect_cnt--;
#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
		} else if (rfctl->radar_detect_map_member != 0) {
			RTW_PRINT("%s map member radar detected, cnt:%d\n", __func__
				, rfctl->radar_detect_map_member);
			rfctl->radar_detect_map_member = 0;
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
		} else
			RTW_INFO("%s radar detected\n", __func__);

		if (rfctl->radar_detected)
			goto set_timer;

		rfctl->radar_detected = 1;

#ifdef CONFIG_DFS_CSA_IE
		rtw_chset_update_non_ocp(rfctl->channel_set
				, rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset);

		/* Switch Channel with CSA after CAC stopped */
		if(IS_CAC_STOPPED(rfctl)) {

			bool ch_avail = _FALSE;
			u8 sel_ch = 0;
			u8 max_bw = dvobj->oper_bwmode; /* use current bwmode as max bwmode */
			u8 cur_ch = dvobj->oper_channel;
			u8 ch = 0, bw = 0, offset = 0;

			rfctl->csa_ch = 36; /* default */
#ifdef CONFIG_DFS_CUSTOMER_CHAN_SEL
			ch_avail = rtw_customer_req_channel_sel(adapter, rfctl,
				sel_ch, max_bw, &ch, &bw, &offset
				,0 , 0, cur_ch, _FALSE, 0);
#endif
			if(ch_avail)
				rfctl->csa_ch = ch;

			DBGP("ch_avail = %s, csa_ch = %u\n", ch_avail?"true":"false", rfctl->csa_ch);

#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
			if (rfctl->csa_cntdown == 0)
				rfctl->csa_cntdown = 5;
#else /* CONFIG_RTW_MULTI_AP_DFS_EN */
			rfctl->csa_cntdown = 1;
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
			rfctl->csa_set_ie = 1;

			/* issue beacon */
			RTW_PRINT("[%s][%d], ready to send beacon, ch=%u, bw=%u\n", __FUNCTION__, __LINE__, dvobj->oper_channel, dvobj->oper_bwmode);
			rtw_mi_tx_beacon_hdl(adapter);

		}
		/* Switch Channel directly during CAC period */
		else {
			rtw_dfs_ch_switch_hdl(dvobj);
		}
#endif

		if (rfctl->radar_detect_enabled)
			goto set_timer;
		goto exit;

cac_status_chk:

		if (!IS_CAC_STOPPED(rfctl)
			&& ((IS_UNDER_CAC(rfctl) && rfctl->cac_force_stop)
				|| !IS_CH_WAITING(rfctl))
		) {
			rtw_phl_cmd_dfs_rd_set_cac_status(dvobj->phl,
							HW_BAND_0,
							false,
							PHL_CMD_DIRECTLY,
							0);

			rfctl->cac_start_time = rfctl->cac_end_time = RTW_CAC_STOPPED;

			if (rtw_mi_check_fwstate(adapter, WIFI_UNDER_LINKING|WIFI_UNDER_SURVEY) == _FALSE) {
				u8 do_rfk = _TRUE;
				u8 u_ch, u_bw, u_offset;

				if (rtw_mi_get_ch_setting_union(adapter, &u_ch, &u_bw, &u_offset)) {
					#ifndef CONFIG_RTW_LINK_PHL_MASTER
					if (adapter->phl_role)
						rtw_phl_reset_chdef(adapter->phl_role);
					#endif
					set_channel_bwmode(adapter, u_ch, u_offset, u_bw, do_rfk);
				} else {
					rtw_warn_on(1);
				}

				RTW_PRINT("[%s][%d], ready to send beacon, ch=%u, bw=%u\n", __FUNCTION__, __LINE__, dvobj->oper_channel, dvobj->oper_bwmode);
				rtw_mi_tx_beacon_hdl(adapter);
			}
		}

		if( IS_UNDER_CAC(rfctl) )
		{
			if ( rtw_systime_to_ms((rtw_get_current_time() - rfctl->cac_start_time)) % 10000 < RADAR_DETECT_POLLING_INT )  //10000: every 10 seconds to print 
				RTW_PRINT("[%s][%d], DFS CAC counting, ch=%u, bw=%u, CAC remaining time=%u secs\n", __FUNCTION__, __LINE__, dvobj->oper_channel, dvobj->oper_bwmode, rtw_systime_to_ms((rfctl->cac_end_time - rtw_get_current_time())) / 1000);
		}
	}

set_timer:
	if (rfctl) {
		_set_timer(&rfctl->radar_detect_timer
		, rtw_dfs_hal_radar_detect_polling_int_ms(dvobj));
	}

exit:
	return H2C_SUCCESS;
}

u8 rtw_dfs_rd_cmd(_adapter *adapter, bool enqueue)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct cmd_priv *cmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	u8 res = _FAIL;

	if (enqueue) {
		cmdobj = rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmdobj == NULL)
			goto exit;
		cmdobj->padapter = adapter;

		parm = rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (parm == NULL) {
			rtw_mfree(cmdobj, sizeof(struct cmd_obj));
			goto exit;
		}

		parm->ec_id = DFS_RADAR_DETECT_WK_CID;
		parm->type = 0;
		parm->size = 0;
		parm->pbuf = NULL;

		init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);
		res = rtw_enqueue_cmd(cmdpriv, cmdobj);
	} else {
		rtw_dfs_rd_hdl(adapter);
		res = _SUCCESS;
	}

exit:
	return res;
}

void rtw_dfs_rd_timer_hdl(void *ctx)
{
	struct rf_ctl_t *rfctl = (struct rf_ctl_t *)ctx;
	struct dvobj_priv *dvobj = rfctl_to_dvobj(rfctl);

	rtw_dfs_rd_cmd(dvobj_get_primary_adapter(dvobj), _TRUE);
}

static void rtw_dfs_rd_enable(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset, bool bypass_cac)
{
	struct dvobj_priv *dvobj = rfctl_to_dvobj(rfctl);
	_adapter *adapter = dvobj_get_primary_adapter(dvobj);

	RTW_INFO("%s on %u,%u,%u\n", __func__, ch, bw, offset);

	if (bypass_cac)
		rfctl->cac_start_time = rfctl->cac_end_time = RTW_CAC_STOPPED;
	else if (rtw_is_cac_reset_needed(rfctl, ch, bw, offset) == _TRUE)
		rtw_reset_cac(rfctl, ch, bw, offset);

	rfctl->radar_detect_by_others = _FALSE;
	rfctl->radar_detect_ch = ch;
	rfctl->radar_detect_bw = bw;
	rfctl->radar_detect_offset = offset;

	rfctl->radar_detected = 0;

	if (!rfctl->radar_detect_enabled) {
		RTW_INFO("%s set radar_detect_enabled\n", __func__);
		rfctl->radar_detect_enabled = 1;
		#ifdef CONFIG_LPS
		LPS_Leave(adapter, "RADAR_DETECT_EN");
		#endif
		_set_timer(&rfctl->radar_detect_timer
			, rtw_dfs_hal_radar_detect_polling_int_ms(dvobj));

		if (rtw_rfctl_overlap_radar_detect_ch(rfctl)) {
			if (IS_CH_WAITING(rfctl)) {
				rtw_phl_cmd_dfs_rd_set_cac_status(dvobj->phl,
								HW_BAND_0,
								true,
								PHL_CMD_DIRECTLY,
								0);
			}
			rtw_dfs_hal_radar_detect_enable(adapter);
		}
	}
}

static void rtw_dfs_rd_disable(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset, bool by_others)
{
	_adapter *adapter = dvobj_get_primary_adapter(rfctl_to_dvobj(rfctl));

	rfctl->radar_detect_by_others = by_others;

	if (rfctl->radar_detect_enabled) {
		bool overlap_radar_detect_ch = rtw_rfctl_overlap_radar_detect_ch(rfctl);

		RTW_INFO("%s clear radar_detect_enabled\n", __func__);

		rfctl->radar_detect_enabled = 0;
		rfctl->radar_detected = 0;
		rfctl->radar_detect_ch = 0;
		rfctl->radar_detect_bw = 0;
		rfctl->radar_detect_offset = 0;
		rfctl->cac_start_time = rfctl->cac_end_time = RTW_CAC_STOPPED;
		_cancel_timer_ex(&rfctl->radar_detect_timer);

		if (overlap_radar_detect_ch) {
			rtw_phl_cmd_dfs_rd_set_cac_status(adapter->dvobj->phl,
						HW_BAND_0,
						false,
						PHL_CMD_DIRECTLY,
						0);

			rtw_dfs_hal_radar_detect_disable(adapter);
		}
	}

	if (by_others) {
		rfctl->radar_detect_ch = ch;
		rfctl->radar_detect_bw = bw;
		rfctl->radar_detect_offset = offset;
	}
}

void rtw_dfs_rd_en_decision(_adapter *adapter, u8 mlme_act, u16 excl_ifbmp)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	struct mi_state mstate;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	u16 ifbmp;
	u8 u_ch, u_bw, u_offset;
	bool ld_sta_in_dfs = _FALSE;
	bool sync_ch = _FALSE; /* _FALSE: asign channel directly */
	bool needed = _FALSE;

	if (mlme_act == MLME_OPCH_SWITCH
		|| mlme_act == MLME_ACTION_NONE
	) {
		ifbmp = ~excl_ifbmp;
		rtw_mi_status_by_ifbmp(dvobj, ifbmp, &mstate);
		rtw_mi_get_ch_setting_union_by_ifbmp(dvobj, ifbmp, &u_ch, &u_bw, &u_offset);
	} else {
		ifbmp = ~excl_ifbmp & ~BIT(adapter->iface_id);
		rtw_mi_status_by_ifbmp(dvobj, ifbmp, &mstate);
		rtw_mi_get_ch_setting_union_by_ifbmp(dvobj, ifbmp, &u_ch, &u_bw, &u_offset);
		if (u_ch != 0)
			sync_ch = _TRUE;

		switch (mlme_act) {
		case MLME_STA_CONNECTING:
			MSTATE_STA_LG_NUM(&mstate)++;
			break;
		case MLME_STA_CONNECTED:
			MSTATE_STA_LD_NUM(&mstate)++;
			break;
		case MLME_STA_DISCONNECTED:
			break;
#ifdef CONFIG_AP_MODE
		case MLME_AP_STARTED:
			MSTATE_AP_NUM(&mstate)++;
			break;
		case MLME_AP_STOPPED:
#ifdef CONFIG_DFS_MASTER
			phl_com->dfs_info.radar_detect_enabled = false;
			rfctl->radar_detect_enabled = false;
#endif
			break;
#endif
#ifdef CONFIG_RTW_MESH
		case MLME_MESH_STARTED:
			MSTATE_MESH_NUM(&mstate)++;
			break;
		case MLME_MESH_STOPPED:
			break;
#endif
		default:
			rtw_warn_on(1);
			break;
		}

		if (sync_ch == _TRUE) {
			if (!MLME_IS_OPCH_SW(adapter)) {
				if (!rtw_is_chbw_grouped(mlmeext->cur_channel, mlmeext->cur_bwmode, mlmeext->cur_ch_offset, u_ch, u_bw, u_offset)) {
					RTW_INFO(FUNC_ADPT_FMT" can't sync %u,%u,%u with %u,%u,%u\n", FUNC_ADPT_ARG(adapter)
						, mlmeext->cur_channel, mlmeext->cur_bwmode, mlmeext->cur_ch_offset, u_ch, u_bw, u_offset);
					goto apply;
				}

				rtw_sync_chbw(&mlmeext->cur_channel, &mlmeext->cur_bwmode, &mlmeext->cur_ch_offset
					, &u_ch, &u_bw, &u_offset);
			}
		} else {
			u_ch = mlmeext->cur_channel;
			u_bw = mlmeext->cur_bwmode;
			u_offset = mlmeext->cur_ch_offset;
		}
	}

	if (MSTATE_STA_LG_NUM(&mstate) > 0) {
		/* STA mode is linking */
		goto apply;
	}

	if (MSTATE_STA_LD_NUM(&mstate) > 0) {
		if (rtw_chset_is_dfs_chbw(rfctl->channel_set, u_ch, u_bw, u_offset)) {
			/*
			* if operate as slave w/o radar detect,
			* rely on AP on which STA mode connects
			*/
			if (IS_DFS_SLAVE_WITH_RD(rfctl) && !rtw_rfctl_dfs_domain_unknown(rfctl))
				needed = _TRUE;
#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
			if (!rtw_rfctl_dfs_domain_unknown(rfctl) && (adapter->multi_ap_mode == MAP_MODE_BACKHAL_STA)) {
					needed = _TRUE;
					rfctl->map_dfs_wo_cac = 1;
			}
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
			ld_sta_in_dfs = _TRUE;
		}
		goto apply;
	}

	if (!MSTATE_AP_NUM(&mstate) && !MSTATE_MESH_NUM(&mstate)) {
		/* No working AP/Mesh mode */
		goto apply;
	}

	if (rtw_chset_is_dfs_chbw(rfctl->channel_set, u_ch, u_bw, u_offset))
		needed = _TRUE;

#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
	if (rfctl->map_dfs_wo_cac)
		ld_sta_in_dfs = _TRUE;
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
apply:

	RTW_INFO(FUNC_ADPT_FMT" needed:%d, mlme_act:%u, excl_ifbmp:0x%04x\n"
		, FUNC_ADPT_ARG(adapter), needed, mlme_act, excl_ifbmp);
	RTW_INFO(FUNC_ADPT_FMT" ld_sta_num:%u, lg_sta_num:%u, ap_num:%u, mesh_num:%u, %u,%u,%u\n"
		, FUNC_ADPT_ARG(adapter), MSTATE_STA_LD_NUM(&mstate), MSTATE_STA_LG_NUM(&mstate)
		, MSTATE_AP_NUM(&mstate), MSTATE_MESH_NUM(&mstate)
		, u_ch, u_bw, u_offset);

	if (needed == _TRUE)
		rtw_dfs_rd_enable(rfctl, u_ch, u_bw, u_offset, ld_sta_in_dfs);
	else {
		rtw_dfs_rd_disable(rfctl, u_ch, u_bw, u_offset, ld_sta_in_dfs);

		if (mlme_act != MLME_AP_STOPPED
			&& (rtw_mi_check_fwstate(adapter, WIFI_UNDER_LINKING|WIFI_UNDER_SURVEY) == _FALSE)
#ifdef CONFIG_RTW_HANDLE_SER_L2
			&& (!rfctl_to_dvobj(rfctl)->ser_L2_inprogress)
#endif
		) {
			rtw_mi_tx_beacon_hdl(adapter);
		}
	}
}

u8 rtw_dfs_rd_en_decision_cmd(_adapter *adapter)
{
	struct cmd_obj *cmdobj;
	struct drvextra_cmd_parm *parm;
	struct cmd_priv *cmdpriv = &adapter_to_dvobj(adapter)->cmdpriv;
	u8 res = _FAIL;

	cmdobj = rtw_zmalloc(sizeof(struct cmd_obj));
	if (cmdobj == NULL)
		goto exit;

	cmdobj->padapter = adapter;

	parm = rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (parm == NULL) {
		rtw_mfree(cmdobj, sizeof(struct cmd_obj));
		goto exit;
	}

	parm->ec_id = DFS_RADAR_DETECT_EN_DEC_WK_CID;
	parm->type = 0;
	parm->size = 0;
	parm->pbuf = NULL;

	init_h2fwcmd_w_parm_no_rsp(cmdobj, parm, CMD_SET_DRV_EXTRA);
	res = rtw_enqueue_cmd(cmdpriv, cmdobj);

exit:
	return res;
}

void rtw_dfs_rd_detect_onoff(_adapter *padapter, u8 enable)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct mlme_ext_priv *mlmeext = &padapter->mlmeextpriv;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	u8 u_ch, u_bw, u_offset;
	bool needed = _FALSE;

	if (!MLME_IS_MESH(padapter) && !MLME_IS_AP(padapter))
		return;
	if (!is_primary_adapter(padapter))
		return;

	u_ch = mlmeext->cur_channel;
	u_bw = mlmeext->cur_bwmode;
	u_offset = mlmeext->cur_ch_offset;
	if (rtw_chset_is_dfs_chbw(rfctl->channel_set, u_ch, u_bw, u_offset))
		needed = _TRUE;

	if (needed == _TRUE) {
		if (enable)
			rtw_dfs_rd_enable(rfctl, u_ch, u_bw, u_offset, 1);
		else {
			phl_com->dfs_info.is_radar_detectd = false;
			rtw_dfs_rd_disable(rfctl, u_ch, u_bw, u_offset, 1);
		}
	}
}

#ifdef CONFIG_DFS_CUSTOMER_CHAN_SEL
#ifdef CONFIG_DFS_CHAN_SEL_G_RANDOM
bool is_bw_no_nop_ch(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset)
{
	bool ret = _FALSE;
	u32 hi = 0, lo = 0;
	int i;
	RT_CHANNEL_INFO *ch_set = rfctl->channel_set;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == _FALSE)
		goto exit;

	for (i = 0; i < MAX_CHANNEL_NUM && ch_set[i].ChannelNum != 0; i++) {
		if (!rtw_ch2freq(ch_set[i].ChannelNum)) {
			rtw_warn_on(1);
			continue;
		}

		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi) {

			if (CH_IS_NON_OCP(&ch_set[i]))
				goto exit;
		}
	}

	return _TRUE;
exit:
	return _FALSE;
}


bool rtw_dfs_chan_sel_g_random(_adapter *padapter, struct rf_ctl_t *rfctl, u8 sel_ch, u8 max_bw
	, u8 *dec_ch, u8 *dec_bw, u8 *dec_offset
	, u8 e_flags, u8 d_flags, u8 cur_ch, bool by_int_info, u8 mesh_only)
{
	struct dvobj_priv *dvobj = rfctl_to_dvobj(rfctl);
	RT_CHANNEL_INFO *ch_set = rfctl->channel_set;

	u8 aval_ch_tbl[MAX_CHANNEL_NUM];


	u8	i ;
	unsigned int random;
	int total_ch_num;
	unsigned int num,which_channel=-1;
	u8 tbl_idx=0;
	u16 cur_oper_bw = rtw_get_oper_bw(padapter);
	u8 cur_oper_offset = rtw_get_oper_choffset(padapter);

	/*fill info into table exclude non-ocp channel*/

	for (i = 0; i < MAX_CHANNEL_NUM && ch_set[i].ChannelNum != 0; i++) {

#ifdef CONFIG_DFS_MASTER
		if (ch_set[i].flags & RTW_CHF_DFS) {
			if(is_bw_no_nop_ch(rfctl, ch_set[i].ChannelNum, cur_oper_bw, cur_oper_offset))
				ch_set[i].is_bw_selected = 0;

			if (!CH_IS_NON_OCP(&ch_set[i]) && !(ch_set[i].is_bw_selected)) {
				aval_ch_tbl[tbl_idx]=ch_set[i].ChannelNum;
				tbl_idx++;
			}

		} else {
			aval_ch_tbl[tbl_idx]=ch_set[i].ChannelNum;
			tbl_idx++;
		}
#endif

	}
/* for debug
	for(i=0; i<tbl_idx; i++)
		printk("aval_ch_tbl[%d] = %d \n",i,aval_ch_tbl[i]);
	printk("[%d] tbl_idx=%d\n",__LINE__,tbl_idx);
*/
	total_ch_num = tbl_idx;

	/*radom get the item from table*/
	get_random_bytes(&random, 4);

	if(tbl_idx!=0){
		num = random % total_ch_num;
	}
	else{
		num = 0;
	}

	which_channel = aval_ch_tbl[num];

	if (which_channel != 0) {
		RTW_INFO("%s: select :ch=%u,bw=%u,offset=%u\n"
			, __func__, which_channel, cur_oper_bw, cur_oper_offset);
		*dec_ch = which_channel;
		*dec_bw = cur_oper_bw;
		*dec_offset = cur_oper_offset;
		return _TRUE;
	} else {
		RTW_ERR("%s: not found\n", __func__);
	}

	return _FALSE;
}
#endif

bool rtw_customer_req_channel_sel(_adapter *padapter
	, struct rf_ctl_t *rfctl, u8 sel_ch, u8 max_bw
	, u8 *dec_ch, u8 *dec_bw, u8 *dec_offset
	, u8 e_flags, u8 d_flags, u8 cur_ch, bool by_int_info, u8 mesh_only)
{
	bool ret;

	#ifdef CONFIG_DFS_CHAN_SEL_R_SHORTEST_WAIT
	ret = rtw_choose_shortest_waiting_ch(rfctl, sel_ch, max_bw
			, dec_ch, dec_bw, dec_offset
			, e_flags, d_flags, cur_ch, by_int_info, mesh_only);
	#endif

	#ifdef CONFIG_DFS_CHAN_SEL_G_RANDOM
	ret = rtw_dfs_chan_sel_g_random(padapter, rfctl, sel_ch, max_bw
			, dec_ch, dec_bw, dec_offset
			, e_flags, d_flags, cur_ch, by_int_info, mesh_only);
	#endif

	return ret;
}
#endif
#endif /* CONFIG_DFS_MASTER */

/* choose channel with shortest waiting (non ocp + cac) time */
bool rtw_choose_shortest_waiting_ch(struct rf_ctl_t *rfctl, u8 sel_ch, u8 max_bw
	, u8 *dec_ch, u8 *dec_bw, u8 *dec_offset
	, u8 e_flags, u8 d_flags, u8 cur_ch, bool by_int_info, u8 mesh_only)
{
#ifndef DBG_CHOOSE_SHORTEST_WAITING_CH
#define DBG_CHOOSE_SHORTEST_WAITING_CH 0
#endif
	struct dvobj_priv *dvobj = rfctl_to_dvobj(rfctl);
#ifdef CONFIG_RTW_ACS
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj);/*GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));*/
#endif
	struct registry_priv *regsty = dvobj_to_regsty(dvobj);
	u8 ch, bw, offset;
	u8 ch_c = 0, bw_c = 0, offset_c = 0;
	int i;
	u32 min_waiting_ms = 0;
	u16 int_factor_c = 0;

	if (!dec_ch || !dec_bw || !dec_offset) {
		rtw_warn_on(1);
		return _FALSE;
	}

	RTW_INFO("%s: sel_ch:%u max_bw:%u e_flags:0x%02x d_flags:0x%02x cur_ch:%u within_sb:%d%s%s\n"
		, __func__, sel_ch, max_bw, e_flags, d_flags, cur_ch, rfctl->ch_sel_within_same_band
		, by_int_info ? " int" : "", mesh_only ? " mesh_only" : "");

	/* full search and narrow bw judegement first to avoid potetial judegement timing issue */
	for (bw = CHANNEL_WIDTH_20; bw <= max_bw; bw++) {
		if (!rtw_hw_is_bw_support(dvobj, bw))
			continue;

		for (i = 0; i < rfctl->max_chan_nums; i++) {
			u32 non_ocp_ms = 0;
			u32 cac_ms = 0;
			u32 waiting_ms = 0;
			u16 int_factor = 0;
			bool dfs_ch;
			bool non_ocp;
			bool long_cac;

			ch = rfctl->channel_set[i].ChannelNum;
			if (sel_ch) {
				if (ch != sel_ch)
					continue;
			} else if (rfctl->ch_sel_within_same_band && !rtw_is_same_band(cur_ch, ch))
				continue;

			if (ch > 14) {
				if (bw > REGSTY_BW_5G(regsty))
					continue;
			} else {
				if (bw > REGSTY_BW_2G(regsty))
					continue;
			}

			if (mesh_only && ch >= 5 && ch <= 9 && bw > CHANNEL_WIDTH_20)
				continue;

			if (!rtw_get_offset_by_chbw(ch, bw, &offset))
				continue;

			if (!rtw_chset_is_chbw_valid(rfctl->channel_set, ch, bw, offset, 0, 0))
				continue;

			if ((e_flags & RTW_CHF_DFS) || (d_flags & RTW_CHF_DFS)) {
				dfs_ch = rtw_chset_is_dfs_chbw(rfctl->channel_set, ch, bw, offset);
				if (((e_flags & RTW_CHF_DFS) && !dfs_ch)
					|| ((d_flags & RTW_CHF_DFS) && dfs_ch))
					continue;
			}

			if ((e_flags & RTW_CHF_LONG_CAC) || (d_flags & RTW_CHF_LONG_CAC)) {
				long_cac = rtw_is_long_cac_ch(ch, bw, offset, rtw_dfs_get_region_domain(dvobj));
				if (((e_flags & RTW_CHF_LONG_CAC) && !long_cac)
					|| ((d_flags & RTW_CHF_LONG_CAC) && long_cac))
					continue;
			}

			if ((e_flags & RTW_CHF_NON_OCP) || (d_flags & RTW_CHF_NON_OCP)) {
				non_ocp = rtw_chset_is_chbw_non_ocp(rfctl->channel_set, ch, bw, offset);
				if (((e_flags & RTW_CHF_NON_OCP) && !non_ocp)
					|| ((d_flags & RTW_CHF_NON_OCP) && non_ocp))
					continue;
			}

			#ifdef CONFIG_DFS_MASTER
			waiting_ms = rtw_get_ch_waiting_ms(rfctl, ch, bw, offset, &non_ocp_ms, &cac_ms);
			#endif

			#if 0 /* def CONFIG_RTW_ACS */
			if (by_int_info) {
				/* for now, consider only primary channel */
				int_factor = hal_data->acs.interference_time[i];
			}
			#endif

			if (DBG_CHOOSE_SHORTEST_WAITING_CH)
				RTW_INFO("%s:%u,%u,%u %u(non_ocp:%u, cac:%u), int:%u\n"
					, __func__, ch, bw, offset, waiting_ms, non_ocp_ms, cac_ms, int_factor);

			if (ch_c == 0
				/* first: smaller wating time */
				|| min_waiting_ms > waiting_ms
				/* then: less interference */
				|| (min_waiting_ms == waiting_ms && int_factor_c > int_factor)
				/* then: wider bw */
				|| (min_waiting_ms == waiting_ms && int_factor_c == int_factor && bw > bw_c)
				/* if all condition equal, same channel -> same band prefer */
				|| (min_waiting_ms == waiting_ms && int_factor_c == int_factor && bw == bw_c
					&& ((cur_ch != ch_c && cur_ch == ch)
						|| (!rtw_is_same_band(cur_ch, ch_c) && rtw_is_same_band(cur_ch, ch)))
					)
			) {
				ch_c = ch;
				bw_c = bw;
				offset_c = offset;
				min_waiting_ms = waiting_ms;
				int_factor_c = int_factor;
			}
		}
	}

	if (ch_c != 0) {
		RTW_INFO("%s: select %u,%u,%u waiting_ms:%u\n"
			, __func__, ch_c, bw_c, offset_c, min_waiting_ms);
		*dec_ch = ch_c;
		*dec_bw = bw_c;
		*dec_offset = offset_c;
		return _TRUE;
	} else {
		RTW_INFO("%s: not found\n", __func__);
		if (d_flags == 0)
			rtw_warn_on(1);
	}

	return _FALSE;
}

