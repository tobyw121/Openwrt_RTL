/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
#define _PHL_GROUP_C_
#include "phl_headers.h"

#define PHL_STA_MAC "%02x:%02x:%02x:%02x:%02x:%02x"
#define PHL_STA_MAC_ARG(x) ((u8 *)(x))[0], ((u8 *)(x))[1], ((u8 *)(x))[2], \
			((u8 *)(x))[3], ((u8 *)(x))[4], ((u8 *)(x))[5]

#define PHL_RU26_POSITION           72      // 36*2, RU26
#define PHL_RU52_POSITION           106     // 53*2, RU52
#define PHL_RU106_POSITION          122     // 61*2, RU106
#define PHL_RU242_POSITION          130     // 65*2, RU242
#define PHL_RU484_POSITION          134     // 67*2, RU484

#define PHL_PSD_CMP_RU52            12      // u(x,2)
#define PHL_PSD_CMP_RU106           24      // u(x,2)
#define PHL_PSD_CMP_RU242           39      // u(x,2)
#define PHL_PSD_CMP_RU484           51      // u(x,2)
#define PHL_PSD_CMP_RU996           63      // u(x,2)

bool phl_is_he_rate(u16 rate)
{
	return (RTW_HE_MODE == RTW_GET_RATE_MODE(rate))? true : false;
}

u16 phl_cal_he_rate_level(u16 rate, u8 *idx)
{
	u16 result = 0;

	if((rate & 0x1ff) >= RTW_DATA_RATE_HE_NSS4_MCS0){
		result = (rate & 0x1ff) - RTW_DATA_RATE_HE_NSS4_MCS0;
		*idx = 3;
	}else if((rate & 0x1ff) >= RTW_DATA_RATE_HE_NSS3_MCS0){
		result = (rate & 0x1ff) - RTW_DATA_RATE_HE_NSS3_MCS0;
		*idx = 2;
	}else if((rate & 0x1ff) >= RTW_DATA_RATE_HE_NSS2_MCS0){
		result = (rate & 0x1ff) - RTW_DATA_RATE_HE_NSS2_MCS0;
		*idx = 1;
	}else if((rate & 0x1ff) >= RTW_DATA_RATE_HE_NSS1_MCS0){
		result = (rate & 0x1ff) - RTW_DATA_RATE_HE_NSS1_MCS0;
		*idx = 0;
	}

	return result;
}

#ifdef CONFIG_WFA_OFDMA_Logo_Test
/* dbg dump */
void phl_grp_dump_info_su(struct phl_info_t *phl_info)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *sta, *n;
	u8 i = 0;
	if (grp_obj == NULL)
		return;

	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[SU_GRP] su_idle_cnt %d\n",
		  grp_obj->su.idle_cnt);
	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[SU_GRP] DUMP SU BUSY LIST\n");
	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->su.busy_list, list) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[SU_GRP] mac_id 0x%x ; tx_avg_tp %d \n",
			  sta->sta_info->macid,
			  sta->sta_info->stats.tx_moving_average_tp);
	}
}

void phl_grp_dump_info_mu(struct phl_info_t *phl_info)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *sta, *n;
	u8 i = 0;
	if (grp_obj == NULL)
		return;

	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU_GRP] mu_idle_cnt %d\n",
		  grp_obj->mu.idle_cnt);
	for (i = 0; i < MAX_MU_GROUP_NUM; i++) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU_GRP] mu_grp_sta_num[%d] %d\n",
			  i, grp_obj->mu.grp_sta_num[i]);
	}
	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU_GRP] DUMP MU BUSY LIST\n");
	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->mu.busy_list, list) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU_GRP] mac_id 0x%x ; grp_bitmap 0x%x ; tx_avg_tp %d \n",
			  sta->sta_info->macid, sta->grp_bitmap,
			  sta->sta_info->stats.tx_moving_average_tp);
	}
}

void phl_grp_dump_assoc_info(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *gsta, *gn;
	struct rtw_phl_stainfo_t *self, *psta, *n;
	_os_spinlockfg sp_flags;
	u8 i = 0, j =0;
	struct rtw_stats *sta_stats = NULL;

	printk("[Assoc STA Queue]: \n");
	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
			       &wrole->assoc_sta_queue.queue, list) {
		if (self == psta)
			continue;
		if (psta->active == false)
			continue;

		sta_stats = &psta->stats;

		printk("  mac_id 0x%x, mac_addr:"PHL_STA_MAC" (MU:%d, DLRU:%d, ULRU:%d) quota:%d\n",
			psta->macid, PHL_STA_MAC_ARG(psta->mac_addr),
			psta->grp_info.is_mu_sta, psta->grp_info.is_dlru_sta, psta->grp_info.is_ulru_sta,
			(psta->grp_info.is_dlru_sta || psta->grp_info.is_mu_sta) ? psta->phl_muru_wd_quota : phl_info->su_wd_quota);
		printk("     [TX] TP: %d (Mbps)\n",	sta_stats->tx_tp_kbits >> 10);
		printk("          average_HE_tx_rate_last:0x%x, average_HE_tx_rate_new:0x%x, cur_tx_data_rate:0x%x\n",
			sta_stats->average_HE_tx_rate_last, sta_stats->average_HE_tx_rate_new, psta->cur_tx_data_rate);
		printk("          tx_cap_grp_idx:0x%x, tx_grp_idx:0x%x\n",
			psta->grp_info.tx_cap_grp_idx, psta->grp_info.tx_grp_idx);
		printk("     [RX] TP: %d (Mbps)\n", sta_stats->rx_tp_kbits >> 10);
		printk("          average_HE_rx_rate:0x%x, cur_rx_data_rate:0x%x\n",
			sta_stats->average_HE_rx_rate, psta->cur_rx_data_rate);
		printk("          rx_cap_grp_idx:0x%x, rx_grp_idx:0x%x\n",
			psta->grp_info.rx_cap_grp_idx, psta->grp_info.rx_grp_idx);
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
}

void phl_grp_dump_info_dlru(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *gsta, *gn;
	struct rtw_phl_stainfo_t *self, *psta, *n;
	u8 i = 0, j =0;

	if (grp_obj == NULL)
		return;

	printk("[dlru_idle_cnt]: %d\n", grp_obj->dlru.idle_cnt);

	printk("[dlru_busy_cnt]: %d\n", grp_obj->dlru.busy_cnt);
	_os_spinlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
	phl_list_for_loop_safe(gsta, gn, struct grp_sta,
			       &grp_obj->dlru.busy_list, list) {
		printk(" mac_id 0x%x; grp_bitmap 0x%x; tx_avg_tp %d (Mbps)\n",
			  gsta->sta_info->macid, gsta->grp_bitmap,
			  gsta->sta_info->stats.tx_tp_kbits >> 10);

	}
	_os_spinunlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
	printk("\n");

	printk("[dlru.fill_tbl_cnt]: %d\n", grp_obj->dlru.fill_tbl_cnt);

	printk("[dlru.grp_num]: %d\n", grp_obj->dlru.grp_num);
	for (i = 0; i < grp_obj->dlru.grp_num; i++) {
		printk("dlru_grp: %d\n", grp_obj->dlru.grp[i].tbl_idx);
		printk("==> tx_pwr_group: 0x%x\n", grp_obj->dlru.grp[i].tx_pwr_group);
		printk("==> sta_num: %d\n", grp_obj->dlru.grp[i].sta_cnt);
		for (j = 0; j < grp_obj->dlru.grp[i].sta_cnt; j++) {
			printk("      macid[%d] mac_addr:"PHL_STA_MAC"\n",
			  grp_obj->dlru.grp[i].sta[j]->macid, PHL_STA_MAC_ARG(grp_obj->dlru.grp[i].sta[j]->mac_addr));
		}
		printk("\n");
	}
	printk("\n");
}

void phl_grp_dump_info_ulru(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *gsta, *gn;
	struct rtw_phl_stainfo_t *self, *psta, *n;
	//_os_spinlockfg sp_flags;
	u8 i = 0, j =0;

	if (grp_obj == NULL)
		return;

	printk("[ulru_idle_cnt]: %d\n", grp_obj->ulru.idle_cnt);

	printk("[ulru_busy_cnt]: %d\n", grp_obj->ulru.busy_cnt);
	_os_spinlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
	phl_list_for_loop_safe(gsta, gn, struct grp_sta,
			       &grp_obj->ulru.busy_list, list) {
		printk(" mac_id 0x%x; grp_bitmap 0x%x; rx_avg_tp %d (Mbps)\n",	// Mark.CS_update
			  gsta->sta_info->macid, gsta->grp_bitmap,
			  gsta->sta_info->stats.rx_tp_kbits >> 10);

	}
	_os_spinunlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
	printk("\n");

	printk("[ulru.fill_tbl_cnt]: %d\n", grp_obj->ulru.fill_tbl_cnt);

	printk("[ulru.grp_num]: %d\n", grp_obj->ulru.grp_num);
	for (i = 0; i < grp_obj->ulru.grp_num; i++) {
		printk("ulru_grp: %d\n", grp_obj->ulru.grp[i].tbl_idx);
		printk("==> sta_num: %d\n", grp_obj->ulru.grp[i].sta_cnt);
		for (j = 0; j < grp_obj->ulru.grp[i].sta_cnt; j++) {
			printk("      macid[%d] mac_addr:"PHL_STA_MAC"\n",
			  grp_obj->ulru.grp[i].sta[j]->macid, PHL_STA_MAC_ARG(grp_obj->ulru.grp[i].sta[j]->mac_addr));
		}
	}
	printk("\n");
}

void phl_grp_clean_info_HETB(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_stainfo_t *self, *psta, *n;
	_os_spinlockfg sp_flags;
	u8 i = 0, j =0;
	struct rtw_stats *stats;

	printk("phl_grp_clean_info_HETB:\n");

	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
				   &wrole->assoc_sta_queue.queue, list) {
		if (self == psta)
			continue;
		if (psta->active == false)
			continue;

		stats = &psta->stats;

		stats->rx_cnt= 0;
		stats->rx_crc32_cnt = 0;

		stats->HE_TB_stat.HE_TB_rx_cnt = 0;
		stats->HE_TB_stat.QoSnull_cnt = 0;
		stats->HE_TB_stat.crc32_cnt = 0;
		stats->HE_TB_stat.ampdu_cnt = 0;
		stats->HE_TB_stat.ampdu_cnt_2 = 0;
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
}

void phl_grp_dump_info_HETB(void *phl, struct rtw_wifi_role_t *wrole, u32 macid)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_stainfo_t *self, *psta, *n;
	_os_spinlockfg sp_flags;
	u8 i = 0, j =0;
	struct rtw_stats stats;

	if(macid == 0)
		printk("[Assoc STA Queue]: \n");

	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
			       &wrole->assoc_sta_queue.queue, list) {
		if (self == psta)
			continue;
		if (psta->active == false)
			continue;

		if(macid!=0 && psta->macid!=macid)
			continue;

		stats = psta->stats;

		printk("mac_id 0x%x, mac_addr:"PHL_STA_MAC"\n",
			psta->macid, PHL_STA_MAC_ARG(psta->mac_addr));

		printk("     QoSnull_cnt:%d\n", stats.HE_TB_stat.QoSnull_cnt);

		printk("     rx_cnt:%d, crc32_cnt:%d (crc32 ratio: %d %%)\n",
					stats.rx_cnt, stats.rx_crc32_cnt,
					stats.rx_cnt? ((stats.rx_crc32_cnt * 100)/stats.rx_cnt): 0);

		printk("     [SU][during last 1 sec] ampdu_cnt:%d, avg_ampdu_agg:%d, avg_ampdu_size:%d\n",
			stats.ampdu_cnt_per_sec,
			stats.avg_ampdu_agg,
			stats.avg_ampdu_size);

		printk("     HE_TB_rx_cnt:%d, HE_TB_crc32_cnt:%d (crc32 ratio: %d %%)\n",
			stats.HE_TB_stat.HE_TB_rx_cnt, stats.HE_TB_stat.crc32_cnt,
			stats.HE_TB_stat.HE_TB_rx_cnt? ((stats.HE_TB_stat.crc32_cnt * 100)/stats.HE_TB_stat.HE_TB_rx_cnt): 0);

		printk("     HE_TB_avg_rx_rate:0x%x\n", stats.HE_TB_stat.HE_TB_avg_rx_rate);

		printk("     ampdu_cnt:%d\n", stats.HE_TB_stat.ampdu_cnt);

		printk("     [during last 50 ampdu] avg_ampdu_agg:%d, avg_ampdu_size:%d, avg_zero_padding:%d\n",
			stats.HE_TB_stat.avg_ampdu_agg_1,
			stats.HE_TB_stat.avg_ampdu_size_1,
			stats.HE_TB_stat.avg_zero_padding_1);

		printk("     [during last 1 sec] ampdu_cnt:%d, avg_ampdu_agg:%d, avg_ampdu_size:%d, avg_zero_padding:%d\n",
			stats.HE_TB_stat.ampdu_cnt_per_sec,
			stats.HE_TB_stat.avg_ampdu_agg_2,
			stats.HE_TB_stat.avg_ampdu_size_2,
			stats.HE_TB_stat.avg_zero_padding_2);
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
}


/* Start of internal static common functions */
bool _check_wrole_grp_condition(struct phl_info_t *phl_info,
	    		  struct rtw_wifi_role_t *wrole)
{
	void *drv = phl_to_drvpriv(phl_info);
	_os_spinlockfg sp_flags;
	bool ret = false;
	do {
		if (NULL == phl_info->grp_obj)
			return ret;

		if (NULL == wrole)
			return ret;

		if (PHL_RTYPE_AP != wrole->type)
			return ret;

		_os_spinlock(drv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
		if (0 == wrole->assoc_sta_queue.cnt)
			return ret;
		_os_spinunlock(drv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);

		ret = true;
	} while (0);

	return ret;
}



bool _check_snd_need(struct phl_info_t *phl_info,
		     struct rtw_wifi_role_t *wrole)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct rtw_phl_stainfo_t *psta = NULL;
	bool ret = false;
	u8 i = 0;

	/* check MU */
	for (i = 0; i < MAX_MU_GROUP_NUM; i++) {
		if (grp_obj->mu.grp_sta_num[i] >= 2) {
			ret = true;
			break;
		}
	}
	/* check SU busy list is empty */
	if (false == list_empty(&grp_obj->su.busy_list)) {
		ret = true;
	}

	return ret;
}

#if 0
void _trigger_sounding(struct phl_info_t *phl_info,
		       struct rtw_wifi_role_t *wrole)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	u8 polling_cnt = 5;
	u8 snd_period = 40;
	if (phl_snd_is_inprogress(phl_info)) {
		if (RTW_PHL_STATUS_SUCCESS == rtw_phl_sound_abort(phl_info)) {
			do {
				if (0 == phl_snd_is_inprogress(phl_info)) {
					rtw_phl_sound_start(
						phl_info,
						wrole->id,
						0,
						snd_period,
						PHL_SND_TEST_F_PASS_STS_CHK);
					break;
				}
				_os_sleep_ms(drv_priv, 1);
				polling_cnt--;
			} while (polling_cnt > 0);
		}
	} else {
		rtw_phl_sound_start(phl_info, wrole->id, 0, snd_period,
				    PHL_SND_TEST_F_PASS_STS_CHK);
	}
}
#endif

/* START of MU-MIMO related */
/*mu entry*/
static struct grp_sta *_query_idle_mu_entry(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;

	_os_list *idle_list = &grp_obj->mu.idle_list;
	struct grp_sta *entry = NULL;

	//_os_spinlock_bh(drv_priv, &grp_obj->mu.list_lock);
	_os_spinlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);
	if (true == list_empty(idle_list)) {
		entry = NULL;
	} else {
		entry = list_first_entry(idle_list, struct grp_sta,
					 list);
		grp_obj->mu.idle_cnt--;
		list_del(&entry->list);
	}
	//_os_spinunlock_bh(drv_priv, &grp_obj->mu.list_lock);
	_os_spinunlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);

	return entry;
}

static enum rtw_phl_status _enqueue_idle_mu_entry(
				struct phl_info_t *phl_info,
				struct grp_sta *entry)
{
	enum rtw_hal_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->mu.idle_list;

	if (entry != NULL) {
		//_os_spinlock_bh(drv_priv, &grp_obj->mu.list_lock);
		_os_spinlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		grp_obj->mu.idle_cnt++;
		//_os_spinunlock_bh(drv_priv, &grp_obj->mu.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}

static enum rtw_phl_status _enqueue_busy_mu_entry(
				struct phl_info_t *phl_info,
				struct grp_sta *entry)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->mu.busy_list;

	if (entry != NULL) {
		//_os_spinlock_bh(drv_priv, &grp_obj->mu.list_lock);
		_os_spinlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		//_os_spinunlock_bh(drv_priv, &grp_obj->mu.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}

void _grp_init_mu(struct phl_info_t *phl_info)
{
	u8 i = 0;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *mu_sta = grp_obj->mu.sta;

	for (i = 0 ; i < MAX_SUPPORT_MU_STA_NUM; i++) {
		mu_sta[i].grp_bitmap = 0;
		mu_sta[i].sta_info = NULL;
		_enqueue_idle_mu_entry(phl_info, &mu_sta[i]);
	}
	grp_obj->mu.para.allow_he_to_vht = 1;
	grp_obj->mu.para.mu_tp_crit_min = MIN_MU_TP_CRITERIA;
}

/**
 * PHASE 0 : Reset MU Group List
 */
void
phl_group_mu_phase_0(struct phl_info_t *phl_info)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *sta, *n;
	u8 i = 0;

	/* Reset busy list and enqueue to idle list */
	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->mu.busy_list, list) {
		//_os_spinlock_bh(drv_priv, &grp_obj->mu.list_lock);
		_os_spinlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);
		list_del(&sta->list);
		//_os_spinunlock_bh(drv_priv, &grp_obj->mu.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->mu.list_lock, _bh, NULL);
		sta->grp_bitmap = 0;
		sta->sta_info = NULL;
		_enqueue_idle_mu_entry(phl_info, sta);
	}
	for (i = 0; i < MAX_MU_GROUP_NUM; i++) {
		grp_obj->mu.grp_sta_num[i] = 0;
	}
}


/**
 * PHASE 1 : Get all of the MU STAs which can pass MU criteria
 */

bool _grp_mu_phase_1_check(
	struct phl_info_t *phl_info,
	struct rtw_phl_stainfo_t *sta)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	bool ret = false;
	do {
		/* 1. Check MU Capability */
		if ((0 == sta->asoc_cap.he_mu_bfme) &&
		    (0 == sta->asoc_cap.vht_mu_bfme))
			break;
		if (!(sta->wmode & WLAN_MD_11AX) && !(sta->wmode & WLAN_MD_11AC))
			break;

		/* 3. Check Performance */
		if (sta->stats.tx_moving_average_tp <
		    grp_obj->mu.para.mu_tp_crit_min) {
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
				  "[MU_GRP] Phase_1 : macid %d Avg.TxTP %d < criteria \n",
				  sta->macid, sta->stats.tx_moving_average_tp);
			break;
		}
		/* TODO:
		if (is in mu black list)
			break;
		*/

		ret = true;
	} while (0);

	return ret;
}

enum rtw_phl_status
phl_group_mu_phase_1(struct phl_info_t *phl_info,
	     	     struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct rtw_grp_info *gsta, *gn;
	struct rtw_phl_stainfo_t *psta;
	struct grp_sta *mu_sta = NULL;
	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "==> phl_group_mu_phase_1\n");
	phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
			       &grp_obj->tx_tp_queue.queue, tx_list) {
		psta = (struct rtw_phl_stainfo_t *)gsta->sta_info;
		if (0 == grp_obj->mu.idle_cnt)
			break;
		if (_grp_mu_phase_1_check(phl_info, psta)) {
			mu_sta = _query_idle_mu_entry(phl_info);
			if (mu_sta == NULL) {
				PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
					  "[MU_GRP] Phase_0 :Get MU Entry FAIL\n");
				break;
			}
			_enqueue_busy_mu_entry(phl_info, mu_sta);
			mu_sta->sta_info = psta;
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
				  "macid 0x%x ; wmode 0x%x ; mu_bfme %d ; vht_mu_bfme %d \n",
				  psta->macid, psta->wmode,
				  psta->asoc_cap.he_mu_bfme,
				  psta->asoc_cap.vht_mu_bfme);
			if ((psta->wmode & WLAN_MD_11AX) &&
			    psta->asoc_cap.he_mu_bfme) {
				mu_sta->grp_bitmap |= BIT(GRP_MU_GRP_HE);
				grp_obj->mu.grp_sta_num[GRP_MU_GRP_HE]++;
				PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
					  "[MU_GRP] Phase_0 :Add to HE MU List\n");
			}
			if ((psta->wmode & WLAN_MD_11AC) &&
			    psta->asoc_cap.vht_mu_bfme) {
				mu_sta->grp_bitmap |= BIT(GRP_MU_GRP_VHT);
				grp_obj->mu.grp_sta_num[GRP_MU_GRP_VHT]++;
				PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
					  "[MU_GRP] Phase_0 :Add to VHT MU List\n");
			}
			if (mu_sta->grp_bitmap == 0) {
				PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
					  "[MU_GRP] Phase_0 : error , mu_sta(macid = 0x%x) group bitmap = 0\n",
					  psta->macid);
			}
		}
	}
	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "<== phl_group_mu_phase_1\n");
	return pstatus;
}


/**
 * PHASE 2 : Separate to Multiple MU Groups if needed.
 **/
enum phl_grp_mu_bitmap
_grp_mu_phase_2_he(struct rtw_phl_stainfo_t *psta)
{
	enum phl_grp_mu_bitmap gidx = GPR_MU_GRP_HE_0;
	/* TODO: HE MU Group Algorithm */
	if (psta->chandef.bw >= CHANNEL_WIDTH_80) {
		gidx = GPR_MU_GRP_HE_2;
	} else if (psta->chandef.bw == CHANNEL_WIDTH_40) {
		gidx = GPR_MU_GRP_HE_1;

	} else if (psta->chandef.bw == CHANNEL_WIDTH_20) {
		gidx = GPR_MU_GRP_HE_0;
	}
	return gidx;
}

enum phl_grp_mu_bitmap
_grp_mu_phase_2_vht(struct rtw_phl_stainfo_t *psta)
{
	enum phl_grp_mu_bitmap gidx = GPR_MU_GRP_VHT_0;
	/* TODO: VHT MU Group Algorithm */
	if (psta->chandef.bw >= CHANNEL_WIDTH_80) {
		gidx = GPR_MU_GRP_VHT_2;
	} else if (psta->chandef.bw == CHANNEL_WIDTH_40) {
		gidx = GPR_MU_GRP_VHT_1;

	} else if (psta->chandef.bw == CHANNEL_WIDTH_20) {
		gidx = GPR_MU_GRP_VHT_0;
	}

	return gidx;
}

enum rtw_phl_status
phl_group_mu_phase_2(struct phl_info_t *phl_info,
	     	     struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *sta, *n;
	struct rtw_phl_stainfo_t *psta = NULL;
	enum phl_grp_mu_bitmap gidx = 0;

	/*Process HE MU STAs*/
	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->mu.busy_list, list) {
		if (0 == (sta->grp_bitmap & BIT(GRP_MU_GRP_HE)))
			continue;
		psta = sta->sta_info;
		gidx = _grp_mu_phase_2_he(psta);
		sta->grp_bitmap |= BIT(gidx);
		grp_obj->mu.grp_sta_num[gidx]++;
		PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
			  "[MU_GRP] Phase_0 :Add to HE MU GRP %d \n", gidx);

	}

	/*Process VHT MU STAs*/
	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->mu.busy_list, list) {
		if (0 == (sta->grp_bitmap & BIT(GRP_MU_GRP_VHT)))
			continue;
		/*if in HE MU list, skip add into VHT list. unless support HE to VHT mixed mode */
		if((0 == grp_obj->mu.para.allow_he_to_vht) &&
		   (0 != (sta->grp_bitmap & BIT(GRP_MU_GRP_HE))))
		   	continue;
		psta = sta->sta_info;
		gidx = _grp_mu_phase_2_vht(psta);
		sta->grp_bitmap |= BIT(gidx);
		grp_obj->mu.grp_sta_num[gidx]++;
		PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
			  "[MU_GRP] Phase_0 :Add to VHT MU GRP %d\n", gidx);
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_group_mu(struct phl_info_t *phl_info,
	      struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	int sta_cnt = 0;
	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU GRP] MU Grouping Start \n");

	if (!_check_wrole_grp_condition(phl_info, wrole)) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU_GRP] SKIP by check condition fail.\n");
		return pstatus;
	}

	//_os_spinlock_bh(drv_priv, &grp_obj->grp_lock);
	_os_spinlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);
	grp_obj->is_grp_in_progress = true;
	//_os_spinunlock_bh(drv_priv, &grp_obj->grp_lock);
	_os_spinunlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);

	/* Phase 0 Reset MU Group Resource */
	phl_group_mu_phase_0(phl_info);

	grp_obj->wrole = wrole;


	/* PHASE 1 : Classify HE / VHT STAs */
	pstatus = phl_group_mu_phase_1(phl_info, wrole);
	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU_GRP] PHASE 1 FAIL \n");
		return pstatus;
	}

	/* PHASE 2 : Other grouping condition */
	pstatus = phl_group_mu_phase_2(phl_info, wrole);
	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU_GRP] PHASE 2 FAIL \n");
		return pstatus;
	}

	//_os_spinlock_bh(drv_priv, &grp_obj->grp_lock);
	_os_spinlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);
	grp_obj->is_grp_in_progress = false;
	//_os_spinunlock_bh(drv_priv, &grp_obj->grp_lock);
	_os_spinunlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);
	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[MU GRP] MU Grouping Done \n");

#ifdef CONFIG_RTW_DEBUG
	phl_grp_dump_info_mu(phl_info);
#endif

	return pstatus;
}

/**
 * phl_grp_mu_get_sta_num_by_gidx(...)
 * @gidx: MU Group Index for query sta number
 */
u8
phl_grp_mu_get_sta_num_by_gidx(struct phl_info_t *phl_info, u8 gidx)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	u8 ret = 0;

	if (gidx >= MAX_MU_GROUP_NUM)
		return ret;
	if (true == phl_grp_is_in_progress(phl_info))
		return ret;

	ret = grp_obj->mu.grp_sta_num[gidx];

	return ret;
}

/**
 * phl_grp_mu_get_macid_list_by_gidx( ... )
 * input :
 * @gidx: targer mu group index 0 ~ 31
 * @macid_list: buffer u16 array for return values.
 * @num: buffer size and retrun sta number in list.
 */
enum rtw_phl_status
phl_grp_mu_get_macid_list_by_gidx(struct phl_info_t *phl_info,
			    	  struct rtw_wifi_role_t *wrole,
		   		  u8 gidx, u16 *macid_list, u8 *num)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *sta, *n;
	u8 cnt = 0;

	if (grp_obj->wrole != wrole)
		return pstatus;
	if (NULL == macid_list)
		return pstatus;
	if (gidx >= MAX_MU_GROUP_NUM)
		return pstatus;
	if (true == phl_grp_is_in_progress(phl_info))
		return pstatus;


	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->mu.busy_list, list) {
		if (0 == (sta->grp_bitmap & BIT(gidx)))
			continue;
		if (false == sta->sta_info->active)
			continue;
		macid_list[cnt] = sta->sta_info->macid;
		cnt++;
		if (cnt >= *num)
			break;
	}

	(*num) = cnt;

	if (0 != *num)
		pstatus = RTW_PHL_STATUS_SUCCESS;

	return pstatus;
}




  /* START of RU ralated */
  /* ru entry */
static struct grp_sta *_query_idle_ulru_entry(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;

	_os_list *idle_list = &grp_obj->ulru.idle_list;
	struct grp_sta *entry = NULL;

	//_os_spinlock_bh(drv_priv, &grp_obj->dlru.list_lock);
	_os_spinlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
	if (true == list_empty(idle_list)) {
		entry = NULL;
	} else {
	    entry = list_first_entry(idle_list, struct grp_sta,
				   list);
		grp_obj->ulru.idle_cnt--;
		list_del(&entry->list);
	}
	//_os_spinunlock_bh(drv_priv, &grp_obj->dlru.list_lock);
	_os_spinunlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);

	return entry;
}

static enum rtw_phl_status _enqueue_idle_ulru_entry(
		  struct phl_info_t *phl_info,
		  struct grp_sta *entry)
{
	enum rtw_hal_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
    void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->ulru.idle_list;

	if (entry != NULL) {
		//_os_spinlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		grp_obj->ulru.idle_cnt++;
		//_os_spinunlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}


static enum rtw_phl_status _enqueue_busy_ulru_entry(
		  struct phl_info_t *phl_info,
		  struct grp_sta *entry)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->ulru.busy_list;

	if (entry != NULL) {
		//_os_spinlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		grp_obj->ulru.busy_cnt++;
		//_os_spinunlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}

void _grp_init_ulru(struct phl_info_t *phl_info)
{
	u8 i = 0;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *ru_sta = grp_obj->ulru.sta;

	for (i = 0 ; i < MAX_SUPPORT_RU_STA_NUM; i++) {
	  ru_sta[i].grp_bitmap = 0;
	  ru_sta[i].sta_info = NULL;
	  _enqueue_idle_ulru_entry(phl_info, &ru_sta[i]);
	}
	for (i = 0 ; i < MAX_RU_GROUP_NUM; i++) {
	  grp_obj->ulru.grp[i].grp_status = 0;
	  grp_obj->ulru.grp[i].sta_cnt = 0;
	  grp_obj->ulru.grp[i].dev_class = BIT(3);
	  //grp_obj->ulru.grp[i].txpwr_lv = BIT(4);
	  grp_obj->ulru.grp[i].bw = i & (BIT(0)|BIT(1)) ;
	  //grp_obj->ulru.grp[i].stbc = i & BIT(2);
	}
}

/* START of RU ralated */
/* ru entry */
static struct grp_sta *_query_idle_dlru_entry(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;

	_os_list *idle_list = &grp_obj->dlru.idle_list;
	struct grp_sta *entry = NULL;

	//_os_spinlock_bh(drv_priv, &grp_obj->dlru.list_lock);
	_os_spinlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
	if (true == list_empty(idle_list)) {
		entry = NULL;
	} else {
		entry = list_first_entry(idle_list, struct grp_sta,
					 list);
		grp_obj->dlru.idle_cnt--;
		list_del(&entry->list);
	}
	//_os_spinunlock_bh(drv_priv, &grp_obj->dlru.list_lock);
	_os_spinunlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);

	return entry;
}

static enum rtw_phl_status _enqueue_idle_dlru_entry(
				struct phl_info_t *phl_info,
				struct grp_sta *entry)
{
	enum rtw_hal_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->dlru.idle_list;

	if (entry != NULL) {
		//_os_spinlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		grp_obj->dlru.idle_cnt++;
		//_os_spinunlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}


static enum rtw_phl_status _enqueue_busy_dlru_entry(
				struct phl_info_t *phl_info,
				struct grp_sta *entry)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->dlru.busy_list;

	if (entry != NULL) {
		_os_spinlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		grp_obj->dlru.busy_cnt++;
		_os_spinunlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}


struct grp_sta *
phl_grp_ulru_get_from_busy_list(struct phl_info_t *phl_info,
			struct rtw_phl_stainfo_t *psta)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *sta, *n, *target = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);

	_os_spinlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
	phl_list_for_loop_safe(sta, n, struct grp_sta,
				   &grp_obj->ulru.busy_list, list) {
		if (psta == sta->sta_info) {
			target = sta;
			break;
		}
	}
	_os_spinunlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
	return target;
}

enum rtw_phl_status
rtw_phl_grp_ulru_add_sta_to_grp(struct phl_info_t *phl_info,
			  struct rtw_phl_stainfo_t *psta,
			  u8 gidx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *ru_sta = NULL;
	if (psta == NULL)
		return pstatus;
	if (0 == grp_obj->ulru.idle_cnt)
		return pstatus;

	ru_sta = phl_grp_ulru_get_from_busy_list(phl_info, psta);
	if (NULL == ru_sta) {
		ru_sta = _query_idle_ulru_entry(phl_info);
		_enqueue_busy_ulru_entry(phl_info, ru_sta);
		ru_sta->sta_info = psta;
	}


	if (0 != (ru_sta->grp_bitmap & BIT(gidx))) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
			  "[DLRU_GRP] MAC_ID 0x%x is already in ULRU Group(%d), skip updating\n",
			  psta->macid, gidx);
	} else {
		ru_sta->grp_bitmap |= BIT(gidx);
		ru_sta->sta_info = psta;
		psta->grp_info.is_ulru_sta = 1;
		psta->grp_info.rx_grp_idx = gidx;
		grp_obj->ulru.grp[gidx].sta[grp_obj->ulru.grp[gidx].sta_cnt] = ru_sta->sta_info;
		grp_obj->ulru.grp[gidx].sta_cnt++;
		#if 1  // Mark.CS_update
		grp_obj->ulru.grp[gidx].bw = psta->chandef.bw;
		#endif
	}
	pstatus = RTW_PHL_STATUS_SUCCESS;

	return pstatus;
}

enum rtw_phl_status
rtw_phl_stop_ulru_grp(struct phl_info_t *phl_info, u8 tbl_idx) {
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;
	struct rtw_phl_mac_ss_dl_grp_upd info ={0};

	pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl_info, true, &mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
		  "[DLRU_GRP] Get Mac fix mode para Fail\n");
		return pstatus;
	}
	mac_fix_tbl->forceru = 0;
	mac_fix_tbl->tbl_hdr.idx = tbl_idx;
	mac_fix_tbl->rugrpid = tbl_idx;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl_info, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
		  "[DLRU_GRP] Set mac fixmode mib Fail\n");
		return pstatus;
	}
	return pstatus;
}

void phl_grp_ulru_remove_from_sta_ary(struct phl_info_t *phl_info,
		  u8 gidx, struct rtw_phl_stainfo_t *psta)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 i = 0, found = 0;
	struct ulru_grp_para *grp = &grp_obj->ulru.grp[gidx];

	for(i=0; i < grp->sta_cnt; i++) {
		if(found) {
			grp->sta[i-1] = grp->sta[i];
			grp->sta[i] = NULL;

		}
		else if(grp->sta[i] == psta) {
			grp->sta[i] = NULL;
			found = 1;
		}
	}

	grp->sta_cnt--;

}

enum rtw_phl_status
rtw_phl_grp_ulru_remove_sta_whole_grp(struct phl_info_t *phl_info,
		  struct rtw_phl_stainfo_t *psta, u8 disasoc)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *ru_sta = NULL;
	u8 gidx = 0, tbl_idx = 0;
	u8 idx;

	if (psta == NULL)
		return pstatus;

	ru_sta = phl_grp_ulru_get_from_busy_list(phl_info, psta);
	if (NULL != ru_sta) {
		_os_spinlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		list_del(&ru_sta->list);
		_os_spinunlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		for (gidx = 0; gidx < MAX_RU_GROUP_NUM; gidx++) {
			if(ru_sta->grp_bitmap & BIT(gidx)) {
				phl_grp_ulru_remove_from_sta_ary(phl_info, gidx, psta);
				if(grp_obj->ulru.grp[gidx].tbl_idx_bitmap & BIT(tbl_idx)){
					if(disasoc)
						;// Do nothing, stop by JOININFO
					else
						rtw_phl_stop_ulru_grp(phl_info, tbl_idx);
				}
			}
		}
		ru_sta->grp_bitmap = 0;
		ru_sta->sta_info = NULL;
		_enqueue_idle_ulru_entry(phl_info, ru_sta);
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[ULRU_GRP] MAC_ID 0x%x Release ULRU STA Entry OK \n",
			  psta->macid);

	} else {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[ULRU_GRP] MAC_ID 0x%x cannot be found in ULRU list. \n",
			  psta->macid);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

enum rtw_phl_status
rtw_phl_grp_ulru_remove_sta_from_grp(struct phl_info_t *phl_info,
			 struct rtw_phl_stainfo_t *psta, u8 gidx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *ru_sta = NULL;
	if (psta == NULL)
		return pstatus;

	ru_sta = phl_grp_ulru_get_from_busy_list(phl_info, psta);
	if (NULL != ru_sta) {
		if (0 != (ru_sta->grp_bitmap & BIT(gidx))) {
			ru_sta->grp_bitmap &= ~(BIT(gidx));
			grp_obj->ulru.grp[gidx].sta_cnt--;
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
				  "[ULRU_GRP] MAC_ID 0x%x Update ULRU STA Group Bitmap 0x%x OK \n",
				  psta->macid, ru_sta->grp_bitmap);
		} else {
			PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
				  "[ULRU_GRP] MAC_ID 0x%x ULRU is not in Group(%d), sta grp bitmap = 0x%x\n",
				   psta->macid, gidx, ru_sta->grp_bitmap);
		}

	} else {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
			  "[ULRU_GRP] MAC_ID 0x%x ULRU is not in ULRU busy list already.\n",
			  psta->macid);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

void _grp_init_dlru(struct phl_info_t *phl_info)
{
	u8 i = 0;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *ru_sta = grp_obj->dlru.sta;

	for (i = 0 ; i < MAX_SUPPORT_RU_STA_NUM; i++) {
		ru_sta[i].grp_bitmap = 0;
		ru_sta[i].sta_info = NULL;
		_enqueue_idle_dlru_entry(phl_info, &ru_sta[i]);
	}
	for (i = 0 ; i < MAX_RU_GROUP_NUM; i++) {
		grp_obj->dlru.grp[i].grp_status = 0;
		grp_obj->dlru.grp[i].sta_cnt = 0;
		grp_obj->dlru.grp[i].dev_class = BIT(3);
		grp_obj->dlru.grp[i].txpwr_lv = BIT(4);
		grp_obj->dlru.grp[i].bw = i & (BIT(0)|BIT(1)) ;
		grp_obj->dlru.grp[i].stbc = i & BIT(2);
	}
}

bool _grp_sta_cap_chk_ulru(struct rtw_phl_stainfo_t *psta, u32 RX_TP_THRD)
{
	bool ret = false;

	do {
		if (NULL == psta)
			break;
		if (false == psta->active)
			break;
		if (0 == (psta->wmode & WLAN_MD_11AX))
			break;
		//if ((psta->stats.rx_tp_kbits >> 10) < MIN_RU_TP_CRITERIA)
		if ((psta->stats.rx_tp_kbits >> 10) < RX_TP_THRD)
			break;

		/* TODO: add other check here, ex: OM's DISALBL DL RU */
		/* TODO:
		if (is in ru black list)
			break;
		*/
		ret = true;
	} while (0);

	return ret;
}

bool _grp_sta_cap_chk_dlru(struct rtw_phl_stainfo_t *psta, u32 TX_TP_THRD)
{
	bool ret = false;

	do {
		if (NULL == psta)
			break;
		if (false == psta->active)
			break;
		if (0 == (psta->wmode & WLAN_MD_11AX))
			break;
		//if ((psta->stats.tx_tp_kbits >> 10) < MIN_RU_TP_CRITERIA)
		if ((psta->stats.tx_tp_kbits >> 10) < TX_TP_THRD)
			break;

		/* TODO: add other check here, ex: OM's DISALBL DL RU */
		/* TODO:
		if (is in ru black list)
			break;
		*/
		ret = true;
	} while (0);

	return ret;
}

struct grp_sta *
phl_grp_dlru_get_from_busy_list(struct phl_info_t *phl_info,
			    struct rtw_phl_stainfo_t *psta)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *sta, *n, *target = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);

	_os_spinlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->dlru.busy_list, list) {
		if (psta == sta->sta_info) {
			target = sta;
			break;
		}
	}
	_os_spinunlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
	return target;
}

enum rtw_phl_status
rtw_phl_grp_dlru_add_sta_to_grp(struct phl_info_t *phl_info,
			      struct rtw_phl_stainfo_t *psta,
			      u8 gidx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *ru_sta = NULL;

	if (psta == NULL)
		return pstatus;
	if (0 == grp_obj->dlru.idle_cnt)
		return pstatus;

	ru_sta = phl_grp_dlru_get_from_busy_list(phl_info, psta);
	if (NULL == ru_sta) {
		ru_sta = _query_idle_dlru_entry(phl_info);
		_enqueue_busy_dlru_entry(phl_info, ru_sta);
		ru_sta->sta_info = psta;
	}


	if (0 != (ru_sta->grp_bitmap & BIT(gidx))) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
			  "[DLRU_GRP] MAC_ID 0x%x is already in DLRU Group(%d), skip updating\n",
			  psta->macid, gidx);
	} else {
		ru_sta->grp_bitmap |= BIT(gidx);
		ru_sta->sta_info = psta;

		psta->grp_info.is_dlru_sta = 1;
		psta->grp_info.tx_grp_idx = gidx;
		grp_obj->dlru.grp[gidx].sta[grp_obj->dlru.grp[gidx].sta_cnt] = ru_sta->sta_info;
		grp_obj->dlru.grp[gidx].sta_cnt++;
		#if 1  // Mark.CS_update
		grp_obj->dlru.grp[gidx].bw = psta->chandef.bw;
		#endif
	}
	pstatus = RTW_PHL_STATUS_SUCCESS;

	return pstatus;
}

enum rtw_phl_status
rtw_phl_stop_dlru_grp(struct phl_info_t *phl_info, u8 tbl_idx) {
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;
	struct rtw_phl_mac_ss_dl_grp_upd info ={0};

	pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl_info, true, &mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] Get Mac fix mode para Fail\n");
		return pstatus;
	}
	mac_fix_tbl->forceru = 0;
	mac_fix_tbl->tbl_hdr.idx = tbl_idx;
	mac_fix_tbl->rugrpid = tbl_idx;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl_info, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] Set mac fixmode mib Fail\n");
		return pstatus;
	}

	info.grp_valid = 0;
	info.macid_u4 = 0xff;
	info.macid_u5 = 0xff;
	info.macid_u6 = 0xff;
	info.macid_u7 = 0xff;
	info.macid_u0 = 0xff;
	info.macid_u1 = 0xff;
	info.macid_u2 = 0xff;
	info.macid_u3 = 0xff;

	info.grp_id = tbl_idx;
	pstatus = rtw_phl_mac_set_dl_grp_info(phl_info, &info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] Set mac dl grp info (id:%d) fail\n", info.grp_id);
		return pstatus;
	}
	return pstatus;
}

void phl_grp_dlru_remove_from_sta_ary(struct phl_info_t *phl_info,
		  u8 gidx, struct rtw_phl_stainfo_t *psta)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 i = 0, found = 0;
	struct dlru_grp_para *grp = &grp_obj->dlru.grp[gidx];

	if(grp->sta_cnt == 0)
		return;

	for(i=0; i < grp->sta_cnt; i++) {
		if(found) {
			grp->sta[i-1] = grp->sta[i];
			grp->sta[i] = NULL;

		}
		else if(grp->sta[i] == psta) {
			grp->sta[i] = NULL;
			found = 1;
		}
	}

	grp->sta_cnt--;

}


enum rtw_phl_status
rtw_phl_grp_dlru_remove_sta_whole_grp(struct phl_info_t *phl_info,
			  struct rtw_phl_stainfo_t *psta, u8 disasoc)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *ru_sta = NULL;
	u8 gidx = 0, tbl_idx = 0;
	u8 idx;
	if (psta == NULL)
		return pstatus;

	ru_sta = phl_grp_dlru_get_from_busy_list(phl_info, psta);
	if (NULL != ru_sta) {
		//_os_spinlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		list_del(&ru_sta->list);
		//_os_spinunlock_bh(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		for (gidx = 0; gidx < MAX_RU_GROUP_NUM; gidx++) {
			if(ru_sta->grp_bitmap & BIT(gidx)) {
				phl_grp_dlru_remove_from_sta_ary(phl_info, gidx, psta);
				for(tbl_idx = 0; tbl_idx < MAX_RU_GROUP_NUM; tbl_idx++) {
					if(grp_obj->dlru.grp[gidx].tbl_idx_bitmap & BIT(tbl_idx)){
						if(disasoc)
							;// Do nothing, stop by JOININFO
						else
							rtw_phl_stop_dlru_grp(phl_info, tbl_idx);
					}
				}
			}
		}
		ru_sta->grp_bitmap = 0;
		ru_sta->sta_info = NULL;
		_enqueue_idle_dlru_entry(phl_info, ru_sta);
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[DLRU_GRP] MAC_ID 0x%x Release DLRU STA Entry OK \n",
			  psta->macid);

	} else {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[DLRU_GRP] MAC_ID 0x%x cannot be found in DLRU list. \n",
			  psta->macid);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

enum rtw_phl_status
rtw_phl_grp_dlru_remove_sta_from_grp(struct phl_info_t *phl_info,
			  	 struct rtw_phl_stainfo_t *psta, u8 gidx)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *ru_sta = NULL;
	if (psta == NULL)
		return pstatus;

	ru_sta = phl_grp_dlru_get_from_busy_list(phl_info, psta);
	if (NULL != ru_sta) {
		if (0 != (ru_sta->grp_bitmap & BIT(gidx))) {
			ru_sta->grp_bitmap &= ~(BIT(gidx));
			grp_obj->dlru.grp[gidx].sta_cnt--;
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  	  "[DLRU_GRP] MAC_ID 0x%x Update DLRU STA Group Bitmap 0x%x OK \n",
			 	  psta->macid, ru_sta->grp_bitmap);
		} else {
			PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
			  	  "[DLRU_GRP] MAC_ID 0x%x DLRU is not in Group(%d), sta grp bitmap = 0x%x\n",
			 	   psta->macid, gidx, ru_sta->grp_bitmap);
		}

	} else {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_DEBUG_,
			  "[DLRU_GRP] MAC_ID 0x%x DLRU is not in DLRU busy list already.\n",
			  psta->macid);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

void
phl_grp_ulru_reset(void *phl)
{
	 struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	 struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	 void *drv_priv = phl_to_drvpriv(phl_info);
	 struct rtw_grp_info *sta, *n;
	 struct grp_sta *gsta, *gn;
	 u8 i = 0;

	 phl_list_for_loop_safe(sta, n, struct rtw_grp_info, &grp_obj->rx_tp_queue.queue, rx_list) {
		 sta->is_ulru_sta = 0;
		 sta->rx_grp_idx = 255;
		 sta->rx_cap_grp_idx = 0;
	 }

	 phl_list_for_loop_safe(gsta, gn, struct grp_sta,
					&grp_obj->ulru.busy_list, list) {
		 _os_spinlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		 list_del(&gsta->list);
		 grp_obj->ulru.busy_cnt--;
		 _os_spinunlock(drv_priv, &grp_obj->ulru.list_lock, _bh, NULL);
		 gsta->sta_info = NULL;
		 gsta->grp_bitmap = 0;
		 _enqueue_idle_ulru_entry(phl_info, gsta);
	 }
	 for (i = 0; i < MAX_RU_GROUP_NUM; i++) {
		 /* reset to default value */
		 grp_obj->ulru.grp[i].grp_status = 0;
		 grp_obj->ulru.grp[i].sta_cnt =0;
	 }

	 grp_obj->ulru.grp_num = 0;

	 for(i = 0; i < MAX_CAP_GRP_NUM; i++) {
		grp_obj->ulru.rx_cap_grp_num[i] = 0;
	 }
}


void
phl_grp_dlru_reset(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_grp_info *sta, *n;
	struct grp_sta *gsta, *gn;
	u8 i = 0;

	phl_list_for_loop_safe(sta, n, struct rtw_grp_info, &grp_obj->tx_tp_queue.queue, tx_list) {
		sta->is_dlru_sta = 0;
		sta->tx_grp_idx = 255;
		sta->tx_cap_grp_idx = 0;
	}

	phl_list_for_loop_safe(gsta, gn, struct grp_sta,
			       &grp_obj->dlru.busy_list, list) {
		_os_spinlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		list_del(&gsta->list);
		grp_obj->dlru.busy_cnt--;
		_os_spinunlock(drv_priv, &grp_obj->dlru.list_lock, _bh, NULL);
		gsta->sta_info = NULL;
		gsta->grp_bitmap = 0;
		_enqueue_idle_dlru_entry(phl_info, gsta);
	}

	for (i = 0; i < MAX_RU_GROUP_NUM; i++) {
		/* reset to default value */
		grp_obj->dlru.grp[i].grp_status = 0;
		grp_obj->dlru.grp[i].sta_cnt =0;
		//grp_obj->dlru.grp[i].table_cnt =0;
	}
	grp_obj->dlru.grp_num = 0;

	for(i = 0; i < MAX_CAP_GRP_NUM; i++) {
		grp_obj->dlru.tx_cap_grp_num[i] = 0;
	}
}


enum rtw_phl_status
rtw_phl_free_ru_sta(void *phl,
	struct rtw_phl_stainfo_t *psta)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *) phl;

	pstatus = rtw_phl_grp_dlru_remove_sta_whole_grp(phl_info, psta, 1);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] rtw_phl_grp_dlru_remove_sta_whole_grp Fail\n");
		return pstatus;
	}

	pstatus = rtw_phl_grp_ulru_remove_sta_whole_grp(phl_info, psta, 1);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] rtw_phl_grp_ulru_remove_sta_whole_grp Fail\n");
		return pstatus;
	}
	return pstatus;
}

u8
_phl_grp_dlru_calculate_rate_level(struct phl_info_t *phl_info, u16 rate){

	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	u8 ru_rate_group;
	u8 mcs_idx;

	mcs_idx = rate % 16;

	ru_rate_group = (!phl_is_he_rate(rate))  ? 255 :
		(11 >= mcs_idx && mcs_idx >= ru_ctrl->ru_rate_idx1) ? 0 :
		(ru_ctrl->ru_rate_idx1 > mcs_idx && mcs_idx >=ru_ctrl->ru_rate_idx2) ? 1 :
		(ru_ctrl->ru_rate_idx2 > mcs_idx && mcs_idx >=0) ? 2 : 255;

	return ru_rate_group;
}

u8
_phl_grp_dlru_calculate_rssi_level(struct phl_info_t *phl_info, u8 rssi){

	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	u16 rssi_m;
	u8 ru_rssi_group;

	rssi_m = rssi >> 1;

	ru_rssi_group = (rssi_m >= ru_ctrl->ru_rssi_level1) ? 0 :
					(ru_ctrl->ru_rssi_level1 > rssi_m && rssi_m >=ru_ctrl->ru_rssi_level2) ? 1 : 2;

	return ru_rssi_group;
}

u8
_phl_grp_dlru_classify_phase_1(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *psta)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	u8 tx_cap_grp_idx = 0;
	u8 ru_rate_group_last, ru_rate_group_new;
	u8 ru_rate_group_used =0;
	u8 ru_rssi_group_last, ru_rssi_group_new;
	u16 tmp;

	struct rtw_stats *sta_stats = NULL;

	sta_stats = &psta->stats;

	/* BIT(0)~BIT(1) : FOR BW */
	if (psta->chandef.bw >= CHANNEL_WIDTH_80)
		tx_cap_grp_idx = 2;
	else if (psta->chandef.bw == CHANNEL_WIDTH_40)
		tx_cap_grp_idx = 1;
	else if (psta->chandef.bw == CHANNEL_WIDTH_20)
		tx_cap_grp_idx = 0;


	/* BIT(2) : FOR STBC */
	if (psta->asoc_cap.stbc_he_rx != 0)
		tx_cap_grp_idx |= BIT(2);

	/* BIT(3)~BIT(4) : FOR RATE LEVEL */
	if(sta_stats->average_HE_tx_rate_last == 0)
		sta_stats->average_HE_tx_rate_last = sta_stats->average_HE_tx_rate_new;
	if(sta_stats->last_rssi == 0)
		sta_stats->last_rssi = psta->hal_sta->rssi_stat.rssi;

	ru_rate_group_new = _phl_grp_dlru_calculate_rate_level(phl_info, sta_stats->average_HE_tx_rate_new);
	ru_rate_group_last = _phl_grp_dlru_calculate_rate_level(phl_info, sta_stats->average_HE_tx_rate_last);

	if (ru_rate_group_new != ru_rate_group_last){
		tmp = (sta_stats->average_HE_tx_rate_new > sta_stats->average_HE_tx_rate_last)? (sta_stats->average_HE_tx_rate_new - sta_stats->average_HE_tx_rate_last) : (sta_stats->average_HE_tx_rate_last - sta_stats->average_HE_tx_rate_new);

		ru_rssi_group_new = _phl_grp_dlru_calculate_rssi_level(phl_info, psta->hal_sta->rssi_stat.rssi);

		ru_rssi_group_last = _phl_grp_dlru_calculate_rssi_level(phl_info, sta_stats->last_rssi);

		if (ru_rssi_group_new == ru_rssi_group_last) {
			if (tmp >= 3) {
				ru_rate_group_used = ru_rate_group_new;
				sta_stats->average_HE_tx_rate_last = sta_stats->average_HE_tx_rate_new;
			}
			else if (tmp == 2){
				ru_rate_group_used = ru_rate_group_last;
				sta_stats->average_HE_tx_rate_last =
					(sta_stats->average_HE_tx_rate_new + sta_stats->average_HE_tx_rate_last)/2;
			}else {
				ru_rate_group_used = ru_rate_group_last;
				//sta_stats->average_HE_tx_rate_last = sta_stats->average_HE_tx_rate_new;
			}
		}
		else  {
			if (tmp >=2) {
				ru_rate_group_used = ru_rate_group_new;
				sta_stats->average_HE_tx_rate_last = sta_stats->average_HE_tx_rate_new;
			}else {
				ru_rate_group_used = ru_rate_group_last;
				//sta_stats->average_HE_tx_rate_last = sta_stats->average_HE_tx_rate_new;
		}
	}
	}
	else {
		ru_rate_group_used = ru_rate_group_new;
		sta_stats->average_HE_tx_rate_last = sta_stats->average_HE_tx_rate_new;
	}

	sta_stats->last_rssi = psta->hal_sta->rssi_stat.rssi;

	if(ru_rate_group_used == 255)/* not HE rate */
		return BIT(7);

	tx_cap_grp_idx |= ((ru_rate_group_used & 0x3) << 3);


	return tx_cap_grp_idx;
}

enum rtw_phl_status
phl_grp_dlru_phase_1(struct phl_info_t *phl_info,
		     struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct rtw_grp_info *gsta, *gn;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_stainfo_t *psta;
	u8 gidx = 0;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
			       &grp_obj->tx_tp_queue.queue, tx_list) {
		if (grp_obj->dlru.idle_cnt == 0)
			break;
		psta = (struct rtw_phl_stainfo_t *)gsta->sta_info;
		if (psta == NULL)
			continue;
		if (true != _grp_sta_cap_chk_dlru(psta, ru_ctrl->TX_TP_THRD))
			continue;

		gsta->tx_cap_grp_idx = _phl_grp_dlru_classify_phase_1(phl_info, psta);
		if(gsta->tx_cap_grp_idx < MAX_CAP_GRP_NUM)
			grp_obj->dlru.tx_cap_grp_num[gsta->tx_cap_grp_idx]++;

	}
	return pstatus;
}

u8
_phl_grp_ulru_classify_phase_1(struct phl_info_t *phl_info,struct rtw_phl_stainfo_t *psta)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct rtw_stats *sta_stats = NULL;

	u8 rx_cap_grp_idx = 0;
	u8 ru_rate_group, mcs_idx;

	sta_stats = &psta->stats;

	/* BIT(0)~BIT(1) : FOR BW */
	if (psta->chandef.bw >= CHANNEL_WIDTH_80)
		rx_cap_grp_idx = 2;
	else if (psta->chandef.bw == CHANNEL_WIDTH_40)
		rx_cap_grp_idx = 1;
	else if (psta->chandef.bw == CHANNEL_WIDTH_20)
		rx_cap_grp_idx = 0;

	/* BIT(2) : FOR STBC */
	if (psta->asoc_cap.stbc_he_rx != 0)
			rx_cap_grp_idx |= BIT(2);

	/* BIT(3)~BIT(4) : FOR RATE LEVEL */
	mcs_idx = sta_stats->average_HE_rx_rate % 16;
	ru_rate_group = (!phl_is_he_rate(sta_stats->average_HE_rx_rate))  ? 255 :
		(11 >= mcs_idx && mcs_idx >= ru_ctrl->ru_rate_idx1) ? 0 :
		(ru_ctrl->ru_rate_idx1 > mcs_idx && mcs_idx >=ru_ctrl->ru_rate_idx2) ? 1 :
		(ru_ctrl->ru_rate_idx2 > mcs_idx && mcs_idx >=0) ? 2 : 255;

	if(ru_rate_group == 255)/* not HE rate */
		return BIT(7);

	rx_cap_grp_idx |= ((ru_rate_group & 0x3) << 3);

	return rx_cap_grp_idx;
}

enum rtw_phl_status
phl_grp_ulru_phase_1(struct phl_info_t *phl_info,
			  struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct rtw_grp_info *gsta, *gn;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_stainfo_t *psta;
	u8 gidx = 0;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
					&grp_obj->rx_tp_queue.queue, rx_list) {
		if (grp_obj->ulru.idle_cnt == 0)
			break;
		psta = (struct rtw_phl_stainfo_t *)gsta->sta_info;
		if (psta == NULL)
			continue;
		if (true != _grp_sta_cap_chk_ulru(psta, ru_ctrl->RX_TP_THRD))
			continue;

		gsta->rx_cap_grp_idx = _phl_grp_ulru_classify_phase_1(phl_info, psta);
		if(gsta->rx_cap_grp_idx < MAX_CAP_GRP_NUM)
			grp_obj->ulru.rx_cap_grp_num[gsta->rx_cap_grp_idx]++;

	}
	return pstatus;
}

enum rtw_phl_status rtw_phl_ru_fill_dl_grp_info(struct phl_info_t *phl_info, u8 gidx, u8 rugrpid)
{
	enum rtw_phl_status pstatus;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct rtw_phl_mac_ss_dl_grp_upd info ={0};
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct rtw_phl_stainfo_t *psta;
	u8 sta_cnt = grp_obj->dlru.grp[gidx].sta_cnt;

	info.grp_valid = 1;
	info.is_hwgrp = 1;
	//info.mru = 0;
	info.grp_id = rugrpid;
	info.next_protecttype = ru_ctrl->prot_type;
	info.next_rsptype = ru_ctrl->rsp_type;

	psta = (sta_cnt > 0) ? grp_obj->dlru.grp[gidx].sta[0 % sta_cnt] : NULL;
	info.macid_u0 = psta ? psta->macid:0xff;
	psta = (sta_cnt > 1) ? grp_obj->dlru.grp[gidx].sta[1 % sta_cnt] : NULL;
	info.macid_u1 = psta ? psta->macid:0xff;
	psta = (sta_cnt > 2) ? grp_obj->dlru.grp[gidx].sta[2 % sta_cnt] : NULL;
	info.macid_u2 = psta ? psta->macid:0xff;
	psta = (sta_cnt > 3) ? grp_obj->dlru.grp[gidx].sta[3 % sta_cnt] : NULL;
	info.macid_u3 = psta ? psta->macid:0xff;
	psta = (sta_cnt > 4) ? grp_obj->dlru.grp[gidx].sta[4 % sta_cnt] : NULL;
	info.macid_u4 = psta ? psta->macid:0xff;
	psta = (sta_cnt > 5) ? grp_obj->dlru.grp[gidx].sta[5 % sta_cnt] : NULL;
	info.macid_u5 = psta ? psta->macid:0xff;
	psta = (sta_cnt > 6) ? grp_obj->dlru.grp[gidx].sta[6 % sta_cnt] : NULL;
	info.macid_u6 = psta ? psta->macid:0xff;
	psta = (sta_cnt > 7) ? grp_obj->dlru.grp[gidx].sta[7 % sta_cnt] : NULL;
	info.macid_u7 = psta ? psta->macid:0xff;

	pstatus = rtw_phl_mac_set_dl_grp_info(phl_info, &info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] Set mac dl grp info %d fail\n", gidx);
		return pstatus;
	}
	return pstatus;
}

enum rtw_phl_status rtw_phl_ru_fill_ru_fixmode_mib(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *psta)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;
	enum rtw_phl_status pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl_info, true, &mac_fix_tbl);

	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] Get Mac fix mode para Fail\n");
		return pstatus;
	}
	mac_fix_tbl->tbl_hdr.rw = 1;
	mac_fix_tbl->force_sumuru_en = ru_ctrl->force_sumuru_en;
	mac_fix_tbl->forcesu = ru_ctrl->forcesu;
	mac_fix_tbl->forceru = ru_ctrl->forceru;
	mac_fix_tbl->fix_fe_heru_en = 1;
	mac_fix_tbl->fix_frame_seq_heru = 1;
	mac_fix_tbl->is_dlruhwgrp = 1;
	mac_fix_tbl->prot_type_heru = ru_ctrl->prot_type;
	mac_fix_tbl->resp_type_heru = ru_ctrl->rsp_type;

	mac_fix_tbl->tbl_hdr.idx = psta->macid;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl_info, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Set mac fixmode mib Fail\n");
		return pstatus;
	}
	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_fill_ru_sta_info(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *psta,
					u8 dlru, u8 gidx)
{
	struct rtw_phl_ru_sta_info *ru_sta = NULL;
	enum rtw_phl_status pstatus;

	pstatus = rtw_phl_ru_query_ru_sta_res(phl_info, true, psta, &ru_sta);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (ru_sta == NULL)) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] GET RU STA Fail\n");
		return pstatus;
	}
	if(dlru) {
		ru_sta->tbl_hdr.idx = psta->macid;
		ru_sta->dlsu_info_en  = 1;
		ru_sta->dl_swgrp_bitmap = 0;
		#if 1
		ru_sta->gi_ltf_48spt = (psta->asoc_cap.ltf_gi>=1)?1:0;
		ru_sta->gi_ltf_18spt= (psta->asoc_cap.ltf_gi==5)?1:0;
		ru_sta->dlsu_bw = psta->chandef.bw;
		ru_sta->dlsu_gi_ltf = psta->asoc_cap.ltf_gi;
		ru_sta->dlsu_doppler_ctrl = psta->asoc_cap.doppler_tx;
		ru_sta->dlsu_coding = psta->asoc_cap.he_ldpc;
		ru_sta->dlsu_txbf = psta->asoc_cap.he_su_bfme;  // discuss txbf flow, another H2C cmd
		ru_sta->dlsu_stbc = 0;
		ru_sta->dl_fwcqi_flag = 0; // if has CQI, set 1
		ru_sta->dlru_ratetbl_ridx = 0; // if has CQI, need set row idx, need CQI flow
		ru_sta->csi_info_bitmap = 0; // discuss txbf flow, another H2C cmd
		ru_sta->dlsu_dcm = 0; //
		ru_sta->dlsu_rate = RTW_DATA_RATE_HE_NSS1_MCS11; // initial rate, RA decision
		ru_sta->dlsu_pwr =  0;
		#endif
	}
	else {
		ru_sta->tbl_hdr.idx = psta->macid;
		ru_sta->ulsu_info_en  = 1;
		ru_sta->ul_swgrp_bitmap = BIT(gidx);
		#if 1
		ru_sta->ul_fwcqi_flag = 0;
		ru_sta->ulru_ratetbl_ridx = 0;
		ru_sta->ulsu_gi_ltf = psta->asoc_cap.ltf_gi_cap;
		ru_sta->ulsu_bw = psta->chandef.bw;
		ru_sta->ulsu_rate.ss = (phl_is_he_rate(psta->cur_rx_data_rate ))? (psta->cur_rx_data_rate - RTW_DATA_RATE_HE_NSS1_MCS0) / 16 : 0;
		ru_sta->ulsu_rate.mcs = (phl_is_he_rate(psta->cur_rx_data_rate ))? psta->cur_rx_data_rate % 16 : 0;
		ru_sta->ulsu_rate.dcm = 0;
		ru_sta->ulsu_rssi =  0;  // ???
		ru_sta->ulsu_doppler_ctrl = 0;
		ru_sta->ulsu_coding = psta->asoc_cap.he_ldpc;
		ru_sta->ulsu_stbc = psta->asoc_cap.stbc_he_rx;
		#endif
	}

	pstatus = rtw_phl_ru_set_ru_sta_fw(phl_info, ru_sta);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[RU_STA] Set RU STA to FW Fail\n");
		return pstatus;
	}

	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_fill_dlmacid_info(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *psta)
{
	enum rtw_phl_status pstatus;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_phl_dlmacid_cfg cfg;

	_rtw_memset(&cfg, 0, sizeof(struct rtw_phl_dlmacid_cfg));

	cfg.macid = psta->macid;
	cfg.dl_su_info_en = 1;
	cfg.dl_su_rate_cfg = 0;
	cfg.dl_su_pwr_cfg = 0;

	cfg.gi_ltf_4x8_support = 1;/* To Do: parsing cap */
	cfg.gi_ltf_1x8_support = 0;/* To Do: parsing cap */
	cfg.dl_su_doppler_ctrl = 0;/* To Do: parsing cap */
	cfg.dl_su_coding = 1;/* To Do: parsing cap */

	pstatus = rtw_phl_ru_set_dlmacid_cfg(phl_info, &cfg);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("Set dlmacid_cfg fail\n");
		return pstatus;
	}

	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_fill_ulmacid_info(struct phl_info_t *phl_info, u8 gidx)
{
	enum rtw_phl_status pstatus;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct rtw_phl_ulmacid_set cfg;
	int i;

	struct rtw_phl_stainfo_t *psta;

	_rtw_memset(&cfg, 0, sizeof(struct rtw_phl_ulmacid_set));

	for(i=0; i < grp_obj->ulru.grp[gidx].sta_cnt; i++) {
		cfg.phl_ul_macid_cfg[i].macid = grp_obj->ulru.grp[gidx].sta[i]->macid;
		cfg.phl_ul_macid_cfg[i].ul_su_info_en = 1;
		cfg.phl_ul_macid_cfg[i].ul_su_bw = grp_obj->ulru.grp[gidx].sta[i]->stats.HE_rx_bw;
		cfg.phl_ul_macid_cfg[i].ul_su_gi_ltf = grp_obj->ulru.grp[gidx].sta[i]->stats.HE_rx_GI_LTF;
		cfg.phl_ul_macid_cfg[i].ul_su_doppler_ctrl = 0;
		cfg.phl_ul_macid_cfg[i].ul_su_dcm = 0;
		cfg.phl_ul_macid_cfg[i].ul_su_ss = grp_obj->ulru.grp[gidx].sta[i]->stats.HE_rx_avg_ss;
		cfg.phl_ul_macid_cfg[i].ul_su_mcs = grp_obj->ulru.grp[gidx].sta[i]->stats.HE_rx_avg_mcs;
		cfg.phl_ul_macid_cfg[i].ul_su_stbc = 0;
		cfg.phl_ul_macid_cfg[i].ul_su_coding = 1;
		//cfg.phl_ul_macid_cfg[i].ul_su_rssi_m = (u8)(psta->phl_sta->hal_sta->rssi_stat.rssi);
		cfg.phl_ul_macid_cfg[i].ul_su_rssi_m = grp_obj->ulru.grp[gidx].sta[i]->stats.average_HE_rx_rssi;
		cfg.phl_ul_macid_cfg[i-1].endcmd = 0;
	}

	cfg.phl_ul_macid_cfg[i-1].endcmd = 1;

	pstatus = rtw_phl_ru_set_ulmacid_cfg(phl_info, &cfg);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("Set dlmacid_cfg fail\n");
		return pstatus;
	}

	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_fill_swgrp_info(struct phl_info_t *phl_info, u8 dlru, u8 gidx)
{
	struct rtw_phl_sw_grp_set sw_grp_set;
	enum rtw_phl_status pstatus;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	int i;

	_rtw_memset(&sw_grp_set, 0, sizeof(struct rtw_phl_sw_grp_set));

	if(dlru) {
		for(i=0; i < grp_obj->dlru.grp[gidx].sta_cnt; i++) {
			sw_grp_set.phl_swgrp_bitmap[i].macid = grp_obj->ulru.grp[gidx].sta[i]->macid;
			sw_grp_set.phl_swgrp_bitmap[i].en_upd_dl_swgrp = 0;
			sw_grp_set.phl_swgrp_bitmap[i].dl_sw_grp_bitmap = 0;
			sw_grp_set.phl_swgrp_bitmap[i].cmdend = 0;
		}
	}
	else {
		for(i=0; i < grp_obj->ulru.grp[gidx].sta_cnt; i++) {
			sw_grp_set.phl_swgrp_bitmap[i].macid = grp_obj->ulru.grp[gidx].sta[i]->macid;
			sw_grp_set.phl_swgrp_bitmap[i].en_upd_ul_swgrp = 1;
			sw_grp_set.phl_swgrp_bitmap[i].ul_sw_grp_bitmap = grp_obj->ulru.grp[gidx].tbl_idx_bitmap;
			sw_grp_set.phl_swgrp_bitmap[i].cmdend = 0;
		}
	}
	sw_grp_set.phl_swgrp_bitmap[i-1].cmdend = 1;

	pstatus = rtw_phl_set_swgrp_set(phl_info, &sw_grp_set);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("Set swgrp_set fail\n");
		return pstatus;
	}

	return pstatus;
}

enum rtw_phl_status rtw_phl_ru_fill_dlru_tbl(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole,
			u8 gidx, struct rtw_phl_stainfo_t *psta)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp_table = &rugrptable->dl_ru_fix_grp_table;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 idx, sta_cnt;

	struct rtw_phl_dlru_tbl *tbl = NULL;
	struct rtw_phl_dlru_fix_tbl *fix_tbl = NULL;

	enum rtw_phl_status pstatus;
	u8 dl_ru_rate_grp;

	pstatus = rtw_phl_ru_query_dlru_tbl_res(phl_info, psta, RTW_PHL_RU_TBL_SW, true, &tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (tbl == NULL)) {
		 PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
		 	"[DLRU_GRP] Get DL RU TBL Fail\n");
		 return pstatus;
	}

#if 1 // Mark.CS_update
	rtw_phl_grp_bw_setting_apply(phl_info, grp_obj->dlru.grp[gidx].bw);
#endif

	tbl->tbl_hdr.type = RTW_PHL_RU_TBL_HW;
	tbl->fix_mode_flag = true;
	//tbl->ppdu_bw = wrole->chandef.bw;

	#if 1  // TODO:  check why mcs 11 ==> tx_pwr is 0x20
	dl_ru_rate_grp = ((psta->grp_info.tx_cap_grp_idx >> 3) & 0x3);
	if(dl_ru_rate_grp == 0)
		//tbl->tx_pwr = rtw_phl_get_tx_pwr_by_txrate(phl_info, tbl->ppdu_bw, 11);
		tbl->tx_pwr = 0x3c;
	else if(dl_ru_rate_grp == 1)
		//tbl->tx_pwr = rtw_phl_get_tx_pwr_by_txrate(phl_info, tbl->ppdu_bw, 8);
		tbl->tx_pwr = 0x3c;
	else if(dl_ru_rate_grp == 2)
		//tbl->tx_pwr = rtw_phl_get_tx_pwr_by_txrate(phl_info, tbl->ppdu_bw, 4);
		tbl->tx_pwr = 0x36;
	else
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[DLRU_GRP] DL RU dl_ru_rate_grp Not Support\n");
	#endif

	grp_obj->dlru.grp[gidx].tx_pwr_group = dl_ru_rate_grp;

	tbl->ppdu_bw = dl_ru_grp->ppdu_bw;		// Mark.CS_update
	tbl->pwr_boost_fac = 0;
	tbl->fix_mode_flag=1;
	tbl->txpwr_ofld_en = dl_ru_grp->txpwr_ofld_en;
	tbl->pwrlim_dis = dl_ru_grp->pwrlim_dis;
	tbl->tf.fix_ba = dl_ru_grp->tf.fix_ba;
	tbl->tf.ru_psd = dl_ru_grp->tf.ru_psd;
	tbl->tf.tf_rate = dl_ru_grp->tf.tf_rate;
	tbl->tf.rf_gain_fix = dl_ru_grp->tf.rf_gain_fix;
	tbl->tf.rf_gain_idx = dl_ru_grp->tf.rf_gain_idx;
	tbl->tf.tb_ppdu_bw = dl_ru_grp->tf.tb_ppdu_bw;
	tbl->tf.rate.mcs = dl_ru_grp->tf.rate.mcs;
	tbl->tf.rate.dcm = dl_ru_grp->tf.rate.dcm;
	tbl->tf.rate.ss = dl_ru_grp->tf.rate.ss;
	tbl->tf.gi_ltf = dl_ru_grp->tf.gi_ltf;
	tbl->tf.doppler = dl_ru_grp->tf.doppler;
	tbl->tf.stbc = dl_ru_grp->tf.stbc;
	tbl->tf.sta_coding = dl_ru_grp->tf.sta_coding;
	tbl->tf.tb_t_pe_nom = dl_ru_grp->tf.tb_t_pe_nom;
	tbl->tf.pr20_bw_en = dl_ru_grp->tf.pr20_bw_en;

	pstatus = rtw_phl_ru_query_dlru_fix_tbl_res(phl_info, true, psta, tbl, &fix_tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (fix_tbl == NULL)) {
		 PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
		 	"[DLRU_GRP] Get DL FIX RU TBL Fail\n");
		 return pstatus;
	}

	fix_tbl->tbl_hdr.type = RTW_PHL_RU_TBL_HW;
	fix_tbl->max_sta_num = grp_obj->dlru.grp[gidx].sta_cnt;
	fix_tbl->min_sta_num = grp_obj->dlru.grp[gidx].sta_cnt;
	fix_tbl->doppler=0;
	fix_tbl->stbc=0;
	fix_tbl->gi_ltf=dl_ru_fix_grp_table->gi_ltf;
	fix_tbl->ma_type=0;
	fix_tbl->fixru_flag = dl_ru_fix_grp_table->fixru_flag;
	fix_tbl->rupos_csht_flag = dl_ru_fix_grp_table->rupos_csht_flag;
	fix_tbl->ru_swp_flg = dl_ru_fix_grp_table->ru_swp_flg;

	sta_cnt = grp_obj->dlru.grp[gidx].sta_cnt;
	for(idx = 0; idx < sta_cnt; idx++) {
		fix_tbl->sta[idx].mac_id=
			grp_obj->dlru.grp[gidx].sta[idx]->macid;

		fix_tbl->sta[idx].ru_pos[0]=dl_ru_fix_grp_table->sta_info[idx].ru_pos[0];
		fix_tbl->sta[idx].ru_pos[1]=dl_ru_fix_grp_table->sta_info[idx].ru_pos[1];
		fix_tbl->sta[idx].ru_pos[2]=dl_ru_fix_grp_table->sta_info[idx].ru_pos[2];
		fix_tbl->sta[idx].ru_pos[3]=dl_ru_fix_grp_table->sta_info[idx].ru_pos[3];
		fix_tbl->sta[idx].ru_pos[4]=dl_ru_fix_grp_table->sta_info[idx].ru_pos[4];
		fix_tbl->sta[idx].ru_pos[5]=dl_ru_fix_grp_table->sta_info[idx].ru_pos[5];
		fix_tbl->sta[idx].ru_pos[6]=dl_ru_fix_grp_table->sta_info[idx].ru_pos[6];

		fix_tbl->sta[idx].fix_rate=dl_ru_fix_grp_table->sta_info[idx].fix_rate;
		fix_tbl->sta[idx].rate.mcs=dl_ru_fix_grp_table->sta_info[idx].mcs;
		fix_tbl->sta[idx].rate.ss=dl_ru_fix_grp_table->sta_info[idx].ss;
		fix_tbl->sta[idx].rate.dcm=0;
		fix_tbl->sta[idx].fix_coding=1;
		fix_tbl->sta[idx].coding=dl_ru_fix_grp_table->sta_info[idx].coding;
		fix_tbl->sta[idx].fix_txbf=1;
		fix_tbl->sta[idx].txbf=0;
		fix_tbl->sta[idx].fix_pwr_fac=1;
		fix_tbl->sta[idx].pwr_boost_fac=0;
	}

	pstatus = rtw_phl_ru_set_dlru_tbl_fw(phl_info, tbl);
	if(pstatus != RTW_PHL_STATUS_SUCCESS)
		return pstatus;

	pstatus = rtw_phl_ru_set_dlru_fix_tbl_fw(phl_info, fix_tbl);
	if(pstatus != RTW_PHL_STATUS_SUCCESS)
		return pstatus;

	grp_obj->dlru.grp[gidx].tbl_idx = tbl->tbl_hdr.idx;
	grp_obj->dlru.grp[gidx].tbl_idx_bitmap = BIT(tbl->tbl_hdr.idx);

	return pstatus;

}

bool
rtw_phl_check_dlru_grp_tbls_change(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{

	//enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	bool ret = false;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *gsta, *gn;
	struct rtw_phl_stainfo_t *self, *psta, *n;
	_os_spinlockfg sp_flags;
	int i;
	u8 dl_ru_rate_grp;

	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
			       &wrole->assoc_sta_queue.queue, list) {
		if (self == psta)
			continue;
		if (psta->active == false){
			if(psta->grp_info.tx_grp_idx!=0xff){
				psta->grp_info.tx_grp_idx = 0xff;
				psta->grp_info.tx_grp_idx_old = 0xff;
				ret = true;
			}
			continue;
		}

		if(psta->grp_info.tx_grp_idx != psta->grp_info.tx_grp_idx_old) {
			psta->grp_info.tx_grp_idx_old = psta->grp_info.tx_grp_idx;
			ret = true;
		}
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);

	if(ret)
		return ret;

	for (i = 0; i < MAX_RU_GROUP_NUM; i++) {
		if(grp_obj->dlru.grp[i].sta_cnt){
			psta = grp_obj->dlru.grp[i].sta[0];
			dl_ru_rate_grp = ((psta->grp_info.tx_cap_grp_idx >> 3) & 0x3);

			if(dl_ru_rate_grp == grp_obj->dlru.grp[i].tx_pwr_group)
				continue;
			else {
				ret = true;
				break;
			}
		}
	}

	return ret;
}

bool
rtw_phl_check_ulru_grp_tbls_change(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{

	//enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	bool ret = false;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct grp_sta *gsta, *gn;
	struct rtw_phl_stainfo_t *self, *psta, *n;
	_os_spinlockfg sp_flags;
	int i;
	u8 ul_ru_rate_grp;

	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
			       &wrole->assoc_sta_queue.queue, list) {
		if (self == psta)
			continue;
		if (psta->active == false){
			if(psta->grp_info.rx_grp_idx!=0xff){
				psta->grp_info.rx_grp_idx = 0xff;
				psta->grp_info.rx_grp_idx_old = 0xff;
				ret = true;
			}
			continue;
		}

		//printk("macid[%d] mac_addr:"PHL_STA_MAC", rx_grp_idx:%d, rx_grp_idx_old:%d\n" ,
		//	psta->macid, PHL_STA_MAC_ARG(psta->mac_addr), psta->grp_info.rx_grp_idx, psta->grp_info.rx_grp_idx_old);

		if(psta->grp_info.rx_grp_idx != psta->grp_info.rx_grp_idx_old) {
			psta->grp_info.rx_grp_idx_old = psta->grp_info.rx_grp_idx;
			ret = true;
		}
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);

	if(ret)
		return ret;

#if 0
	for (i = 0; i < MAX_RU_GROUP_NUM; i++) {
		if(grp_obj->ulru.grp[i].sta_cnt){
			psta = grp_obj->ulru.grp[i].sta[0];
			ul_ru_rate_grp = ((psta->grp_info.rx_cap_grp_idx >> 3) & 0x3);

			if(ul_ru_rate_grp == grp_obj->ulru.grp[i].tx_pwr_group)
				continue;
			else {
				ret = true;
				break;
			}
		}
	}
#endif

	return ret;
}

u16
rtw_phl_get_grp_usr_rssi_m(struct phl_info_t *phl_info, u8 dlru, u8 gidx)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	int i;
	u16 avg_rssi=0;

	if(dlru) {
		for(i=0; i < grp_obj->dlru.grp[gidx].sta_cnt; i++) {
			avg_rssi += grp_obj->dlru.grp[gidx].sta[i]->stats.average_HE_rx_rssi;
		}
		avg_rssi = avg_rssi / grp_obj->dlru.grp[gidx].sta_cnt;
	}
	else {
		for(i=0; i < grp_obj->ulru.grp[gidx].sta_cnt; i++) {
			avg_rssi += grp_obj->ulru.grp[gidx].sta[i]->stats.average_HE_rx_rssi;
		}
		avg_rssi = avg_rssi / grp_obj->ulru.grp[gidx].sta_cnt;
	}

	return avg_rssi;
}

u16
rtw_phl_cal_psd_e(struct phl_info_t *phl_info, u16 rssi_m)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;

	u16 psd_e, ru_position_in;

	psd_e = rssi_m << 1;   // u(8,1) => u(9,2)

	ru_position_in = ul_ru_fix_grp->sta_info[0].ru_pos[0] << 1;

	if (ru_position_in < PHL_RU26_POSITION)             // 36*2, RU26
		psd_e = psd_e;
	else if (ru_position_in < PHL_RU52_POSITION)        // 53*2, RU52
		psd_e = psd_e - PHL_PSD_CMP_RU52;
	else if (ru_position_in < PHL_RU106_POSITION)       // 61*2, RU106,
		psd_e = psd_e - PHL_PSD_CMP_RU106;
	else if (ru_position_in < PHL_RU242_POSITION)       // 65*2, RU242,
		psd_e = psd_e - PHL_PSD_CMP_RU242;
	else if (ru_position_in < PHL_RU484_POSITION)       // 67*2, RU484,
		psd_e = psd_e - PHL_PSD_CMP_RU484;
	else                                            // RU996,
		psd_e = psd_e - PHL_PSD_CMP_RU996;

	//printk("%s: psd_e:%d\n", __func__, psd_e);

	return psd_e;
}

enum rtw_phl_status
rtw_phl_fill_dlru_grp_tbls(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{

	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 gidx, i;
	enum rtw_phl_status pstatus;

	rtw_phl_ru_release_all_dlru_tbl_res(phl_info);
	rtw_phl_ru_release_all_ru_sta_res(phl_info);
	rtw_phl_ru_release_all_fixmode_tbl_res(phl_info);

	for (gidx = 0; gidx < grp_obj->dlru.grp_num; gidx++)
	{
		pstatus = rtw_phl_ru_fill_dlru_tbl(phl_info, wrole, gidx, grp_obj->dlru.grp[gidx].sta[0]);
		if(pstatus != RTW_PHL_STATUS_SUCCESS)
			return pstatus;

		for(i=0;i<grp_obj->dlru.grp[gidx].sta_cnt;i++) {
			rtw_phl_ru_fill_dlmacid_info(phl_info, grp_obj->dlru.grp[gidx].sta[i]);
		}

		for(i=0;i<grp_obj->dlru.grp[gidx].sta_cnt;i++) {
			rtw_phl_ru_fill_ru_fixmode_mib(phl_info, grp_obj->dlru.grp[gidx].sta[i]);
			//rtw_phl_ru_fill_dl_grp_info(phl_info, gidx, grp_obj->dlru.grp[gidx].tbl_idx[i], i);
		}

		rtw_phl_ru_fill_dl_grp_info(phl_info, gidx, grp_obj->dlru.grp[gidx].tbl_idx);
	 }
	 return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_fill_ulru_tbl(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole,
		u8 gidx, struct rtw_phl_stainfo_t *psta)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp_table = &rugrptable->ul_ru_fix_grp_table;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 i;
	u16 avg_rssi_m, psd_e;
	struct rtw_phl_stainfo_t * phl_sta;

	struct rtw_phl_ulru_tbl *tbl = NULL;
	struct rtw_phl_ulru_fix_tbl *fix_tbl = NULL;
	struct rtw_phl_sw_grp_set sw_grp_set;

	_rtw_memset(&sw_grp_set, 0, sizeof(struct rtw_phl_sw_grp_set));

	if(ru_ctrl->ul_psd){
		avg_rssi_m = rtw_phl_get_grp_usr_rssi_m(phl_info, 0, gidx);

		psd_e = rtw_phl_cal_psd_e(phl_info, avg_rssi_m);
	}

	pstatus = rtw_phl_ru_query_ulru_tbl_res(phl_info, psta, RTW_PHL_RU_TBL_SW, true, &tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (tbl == NULL)) {
		 DBGP("Get UL RU TBL Fail\n");
		 return pstatus;
	}

#if 1 // Mark.CS_update
	rtw_phl_grp_bw_setting_apply(phl_info, grp_obj->ulru.grp[gidx].bw);
#endif

	tbl->tbl_hdr.type = RTW_PHL_RU_TBL_SW;
	//tbl->ppdu_bw = wrole->chandef.bw;	// Mark.CS_update

	if(ru_ctrl->ul_psd){
		tbl->grp_psd_max = psd_e + (ru_ctrl->psd_ofst1 << 2);
		tbl->grp_psd_min = tbl->grp_psd_max - (ru_ctrl->psd_ofst2 << 2);
	}else {
		tbl->grp_psd_max = ul_ru_grp->grp_psd_max;
		tbl->grp_psd_min = ul_ru_grp->grp_psd_min;
	}
	tbl->tf_rate = ul_ru_grp->tf_rate;
	tbl->fix_tf_rate =	ul_ru_grp->fix_tf_rate;
	tbl->ppdu_bw = ul_ru_grp->ppdu_bw;
	tbl->rf_gain_fix = ul_ru_grp->rf_gain_fix;
	tbl->rf_gain_idx = ul_ru_grp->rf_gain_idx;
	tbl->fix_mode_flags = ul_ru_grp->fix_mode_flags;

	pstatus = rtw_phl_ru_query_ulru_fix_tbl_res(phl_info, true, psta, tbl, &fix_tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (fix_tbl == NULL)) {
		 PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[ULRU_GRP] Get UL FIX RU TBL Fail\n");
		 return pstatus;
	}

	fix_tbl->min_sta_num = grp_obj->ulru.grp[gidx].sta_cnt;
	fix_tbl->max_sta_num = grp_obj->ulru.grp[gidx].sta_cnt;
	fix_tbl->tbl_hdr.type = RTW_PHL_RU_TBL_SW;
	fix_tbl->doppler = 0;
	fix_tbl->ma_type = 0;
	fix_tbl->gi_ltf = ul_ru_fix_grp_table->gi_ltf;
	fix_tbl->stbc = 0;
	fix_tbl->fix_tb_t_pe_nom = 1;
	fix_tbl->tb_t_pe_nom = 2;
	fix_tbl->fixru_flag = 1;

	for (i = 0; i < grp_obj->ulru.grp[gidx].sta_cnt; i++)
	{
		phl_sta = grp_obj->ulru.grp[gidx].sta[i];

		fix_tbl->sta[i].mac_id = phl_sta->macid;

		fix_tbl->sta[i].ru_pos[0] = ul_ru_fix_grp_table->sta_info[i].ru_pos[0];
		fix_tbl->sta[i].ru_pos[1] = ul_ru_fix_grp_table->sta_info[i].ru_pos[1];
		fix_tbl->sta[i].ru_pos[2] = ul_ru_fix_grp_table->sta_info[i].ru_pos[2];
		fix_tbl->sta[i].ru_pos[3] = ul_ru_fix_grp_table->sta_info[i].ru_pos[3];
		fix_tbl->sta[i].ru_pos[4] = ul_ru_fix_grp_table->sta_info[i].ru_pos[4];
		fix_tbl->sta[i].ru_pos[5] = ul_ru_fix_grp_table->sta_info[i].ru_pos[5];
		fix_tbl->sta[i].ru_pos[6] = ul_ru_fix_grp_table->sta_info[i].ru_pos[6];

		fix_tbl->sta[i].fix_tgt_rssi = ul_ru_fix_grp_table->sta_info[i].fix_tgt_rssi;
		fix_tbl->sta[i].tgt_rssi[0] = ul_ru_fix_grp_table->sta_info[i].tgt_rssi[0];
		fix_tbl->sta[i].tgt_rssi[1] = ul_ru_fix_grp_table->sta_info[i].tgt_rssi[1];
		fix_tbl->sta[i].tgt_rssi[2] = ul_ru_fix_grp_table->sta_info[i].tgt_rssi[2];
		fix_tbl->sta[i].tgt_rssi[3] = ul_ru_fix_grp_table->sta_info[i].tgt_rssi[3];
		fix_tbl->sta[i].tgt_rssi[4] = ul_ru_fix_grp_table->sta_info[i].tgt_rssi[4];
		fix_tbl->sta[i].tgt_rssi[5] = ul_ru_fix_grp_table->sta_info[i].tgt_rssi[5];
		fix_tbl->sta[i].tgt_rssi[6] = ul_ru_fix_grp_table->sta_info[i].tgt_rssi[6];

		fix_tbl->sta[i].fix_rate = ul_ru_fix_grp_table->sta_info[i].fix_rate;
		fix_tbl->sta[i].rate.dcm = 0;
		fix_tbl->sta[i].fix_coding = 1;

		if ((phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC11)
			fix_tbl->sta[i].rate.mcs = 11;
		else if((phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC9)
			fix_tbl->sta[i].rate.mcs = 9;
		else
			fix_tbl->sta[i].rate.mcs = 7;

		fix_tbl->sta[i].rate.ss = phl_sta->asoc_cap.nss_rx-1;

		if (phl_sta->asoc_cap.he_ldpc)
			fix_tbl->sta[i].coding = 1; // LDPC
		else
			fix_tbl->sta[i].coding = 0; // BCC
	}

	pstatus = rtw_phl_ru_set_ulru_tbl_fw(phl_info, tbl);
	if(pstatus != RTW_PHL_STATUS_SUCCESS)
		return pstatus;

	pstatus = rtw_phl_ru_set_ulru_fix_tbl_fw(phl_info, fix_tbl);
	if(pstatus != RTW_PHL_STATUS_SUCCESS)
		return pstatus;

	grp_obj->ulru.grp[gidx].tbl_idx = tbl->tbl_hdr.idx;
	grp_obj->ulru.grp[gidx].tbl_idx_bitmap = BIT(tbl->tbl_hdr.idx);

	return pstatus;
}

enum rtw_phl_status
rtw_phl_ru_fill_ul_fixinfo(struct phl_info_t *phl_info, u8 gidx)
{
	struct rtw_phl_ax_ul_fixinfo tbl_b;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 sta_cnt = grp_obj->ulru.grp[gidx].sta_cnt;
	struct rtw_phl_stainfo_t *psta;
	u8 i;

	_rtw_memset(&tbl_b, 0, sizeof(struct rtw_phl_ax_ul_fixinfo));

	if(sta_cnt > MAX_ULRU_GRP_STA_NUM) {
    	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			"[UL GRP] Not support STA num > %d\n", MAX_ULRU_GRP_STA_NUM);
	    return RTW_PHL_STATUS_FAILURE;
	}

	for(i=0;i<sta_cnt;i++) {
		psta = grp_obj->ulru.grp[gidx].sta[i];
		tbl_b.sta[i].macid = psta->macid;
		tbl_b.ulrua.sta[i].mac_id = psta->macid;
		if (psta->asoc_cap.he_ldpc)
			tbl_b.ulrua.sta[i].coding = 1;
		else
		tbl_b.ulrua.sta[i].coding = 0;
		tbl_b.ulrua.sta[i].rate.mcs = 7;
		tbl_b.ulrua.sta[i].rate.ss = 0;
		tbl_b.ulrua.sta[i].ru_pos = (RTW_HE_RU52_1 + i)*2;
		tbl_b.ulrua.sta[i].tgt_rssi = 70;
	}
	tbl_b.ulrua.sta_num = sta_cnt;
	tbl_b.cfg.mode = 0x2;
	tbl_b.cfg.storemode = 0x2;
	tbl_b.store_idx = 0x0;
	tbl_b.ulfix_usage = 0x3;
	tbl_b.cfg.interval = 60; //microseconds
	tbl_b.data_rate = 0x8;
	tbl_b.data_bw = 0x0;
	tbl_b.gi_ltf = 0x0;
	tbl_b.tf_type = 0x1;

	// common
	//tbl_b.ulrua.sta_num = 0x2;
	tbl_b.ulrua.gi_ltf = 0x0;
	tbl_b.ulrua.n_ltf_and_ma = 0x0;
	tbl_b.ulrua.ppdu_bw = 0x0;
	tbl_b.apep_len = 0x20;

	tbl_b.ul_logo_test = 0;

	rtw_phl_mac_set_upd_ul_fixinfo(phl_info, &tbl_b);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
rtw_phl_fill_ulru_grp_tbls(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{

	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 gidx, i;
	//struct rtw_phl_ulru_tbl *tbl;
	enum rtw_phl_status pstatus;
	u8 sta_cnt;

	rtw_phl_ru_release_all_ulru_tbl_res(phl_info);

	for (gidx = 0; gidx < grp_obj->ulru.grp_num; gidx++)
	{
		//sta_cnt = grp_obj->ulru.grp[gidx].sta_cnt;	// Mark.CS_update

		pstatus = rtw_phl_ru_fill_ulru_tbl(phl_info, wrole, gidx, grp_obj->ulru.grp[gidx].sta[0]);

		if(pstatus != RTW_PHL_STATUS_SUCCESS)
			return pstatus;

		pstatus = rtw_phl_ru_fill_swgrp_info(phl_info, 0, gidx);
		if(pstatus != RTW_PHL_STATUS_SUCCESS)
			return pstatus;

		pstatus = rtw_phl_ru_fill_ulmacid_info(phl_info, gidx);
		if(pstatus != RTW_PHL_STATUS_SUCCESS)
			return pstatus;

		pstatus = rtw_phl_ru_fill_ul_fixinfo(phl_info, gidx);
		if(pstatus != RTW_PHL_STATUS_SUCCESS)
			return pstatus;
	}
	return pstatus;
}

void phl_grp_ulru_decision(struct phl_info_t *phl_info)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 grp_idx = 0, ru_sta_num = 0;
	_list phead, plist;
	struct rtw_grp_info *gsta1, *gsta2, *gn;

	phl_list_for_loop_safe(gsta1, gn, struct rtw_grp_info, &grp_obj->rx_tp_queue.queue, rx_list) {
		if(gsta1->rx_grp_idx != 255) // Skip STA that is grouped
			continue;

		if(gsta1->rx_cap_grp_idx >= MAX_CAP_GRP_NUM)
			continue;

		if(grp_obj->ulru.rx_cap_grp_num[gsta1->rx_cap_grp_idx] > 1) {
				rtw_phl_grp_ulru_add_sta_to_grp(phl_info, (struct rtw_phl_stainfo_t *)gsta1->sta_info, grp_idx);
				grp_obj->ulru.rx_cap_grp_num[gsta1->rx_cap_grp_idx]--;
				ru_sta_num++;
		}
		else
				continue;

		for (gsta2 = list_entry(gsta1->rx_list.next, struct rtw_grp_info, rx_list);&gsta2->rx_list != (&grp_obj->rx_tp_queue.queue); gsta2 = list_entry(gsta2->rx_list.next, struct rtw_grp_info, rx_list)){
			if(gsta2->rx_cap_grp_idx == gsta1->rx_cap_grp_idx) { // same capability group
				rtw_phl_grp_ulru_add_sta_to_grp(phl_info, (struct rtw_phl_stainfo_t *)gsta2->sta_info, grp_idx);
				grp_obj->ulru.rx_cap_grp_num[gsta1->rx_cap_grp_idx]--;
				ru_sta_num++;
				if(grp_obj->ulru.grp[grp_idx].sta_cnt == MAX_ULRU_GRP_STA_NUM || ru_sta_num >= MAX_RU_GRP_STA_NUM)
					break;
			}
		}
		grp_idx++;
		if(ru_sta_num >= MAX_RU_GRP_STA_NUM - 1) //	cannot group when remaining one STA
			break;
		if(grp_idx >= MAX_RU_GROUP_NUM) //	group reach maximum (ex: 16)
			break;
	}
	grp_obj->ulru.grp_num = grp_idx;
}

void phl_grp_dlru_decision(struct phl_info_t *phl_info)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	u8 grp_idx = 0, ru_sta_num = 0;
	_list phead, plist;
	struct rtw_grp_info *gsta1, *gsta2, *gn;

	phl_list_for_loop_safe(gsta1, gn, struct rtw_grp_info, &grp_obj->tx_tp_queue.queue, tx_list) {
		if(gsta1->tx_grp_idx != 255) // Skip STA that is grouped
			continue;

		if(gsta1->tx_cap_grp_idx >= MAX_CAP_GRP_NUM)
			continue;

		if(grp_obj->dlru.tx_cap_grp_num[gsta1->tx_cap_grp_idx] > 1) {
				rtw_phl_grp_dlru_add_sta_to_grp(phl_info, (struct rtw_phl_stainfo_t *)gsta1->sta_info, grp_idx);
				grp_obj->dlru.tx_cap_grp_num[gsta1->tx_cap_grp_idx]--;
				ru_sta_num++;
		}
		else
				continue;

		for (gsta2 = list_entry(gsta1->tx_list.next, struct rtw_grp_info, tx_list);&gsta2->tx_list != (&grp_obj->tx_tp_queue.queue); gsta2 = list_entry(gsta2->tx_list.next, struct rtw_grp_info, tx_list)){
			if(gsta2->tx_cap_grp_idx == gsta1->tx_cap_grp_idx) { // same capability group
				rtw_phl_grp_dlru_add_sta_to_grp(phl_info, (struct rtw_phl_stainfo_t *)gsta2->sta_info, grp_idx);
				grp_obj->dlru.tx_cap_grp_num[gsta1->tx_cap_grp_idx]--;
				ru_sta_num++;
				if(grp_obj->dlru.grp[grp_idx].sta_cnt == MAX_DLRU_GRP_STA_NUM || ru_sta_num >= MAX_RU_GRP_STA_NUM)
					break;
			}
		}

		grp_idx++;
		if(ru_sta_num >= MAX_RU_GRP_STA_NUM - 1) //	cannot group when remaining one STA
			break;
		if(grp_idx >= MAX_RU_GROUP_NUM) //	group reach maximum (ex: 16)
			break;
	}
	grp_obj->dlru.grp_num = grp_idx;
}


void rtw_phl_get_grp_stacnt(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wrole,
			u8 *su_stanum,
			u8 *inactive_ru_stanum,
			u8 *active_ru_stanum,
			u8 *inactive_mu_stanum,
			u8 *active_mu_stanum)
{
	struct rtw_phl_stainfo_t *psta, *n, *self;
	*su_stanum = 0;
	*inactive_ru_stanum = 0;
	*active_ru_stanum = 0;
	*inactive_mu_stanum = 0;
	*active_mu_stanum = 0;

	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,&wrole->assoc_sta_queue.queue, list) {
		if(psta == self){
			continue;
		}

		if(psta->active == false){
			continue;
		}

		if(psta->grp_info.is_dlru_sta == 0 && psta->grp_info.is_mu_sta == 0)
			 (*su_stanum)++;
		if(psta->grp_info.is_dlru_sta == 1) {
			//if(psta->stats.tx_moving_average_tp > 0) {
			if((psta->stats.tx_tp_kbits >> 10) > 0) {
				(*active_ru_stanum)++;
			} else {
				(*inactive_ru_stanum)++;
			}
		} else if(psta->grp_info.is_mu_sta == 1) {
			//if(psta->stats.tx_moving_average_tp > 0) {
			if((psta->stats.tx_tp_kbits >> 10) > 0) {
				(*active_mu_stanum)++;
			} else {
				(*inactive_mu_stanum)++;
			}
		}
	}
}

#define TOTAL_WD_NUM 1600
#define NON_RU_RATIO 0.25
#define MIN_WD_QUOTA 32
void rtw_phl_wd_quota_upd(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{
	struct rtw_phl_stainfo_t *psta, *n, *self;
	u8 inactive_ru_cnt, active_ru_cnt, inactive_mu_cnt, active_mu_cnt;
	u8 inactive_muru_cnt, active_muru_cnt, muru_cnt, su_cnt;
	u16 inactive_quota, res_wd_cnt, active_quota;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_spinlockfg sp_flags;

	rtw_phl_get_grp_stacnt(phl_info, wrole, &su_cnt,
		&inactive_ru_cnt, &active_ru_cnt, &inactive_mu_cnt, &active_mu_cnt);

	inactive_muru_cnt = inactive_ru_cnt + inactive_mu_cnt;
	active_muru_cnt = active_ru_cnt + active_mu_cnt;
	muru_cnt = inactive_muru_cnt + active_muru_cnt;
	// nonru_sta quota
	phl_info->su_wd_quota = (su_cnt!=0 && muru_cnt!=0) ? NON_RU_RATIO * TOTAL_WD_NUM :
					   (su_cnt!=0 && muru_cnt == 0) ?	TOTAL_WD_NUM : 0; // for nonmu STA
	inactive_quota = (su_cnt == 0 && active_ru_cnt == 0) ?
						(TOTAL_WD_NUM / inactive_muru_cnt): MIN_WD_QUOTA; // for (RU + MU) inactive STA
	res_wd_cnt = TOTAL_WD_NUM - phl_info->su_wd_quota - (inactive_quota * inactive_muru_cnt);
	active_quota = (active_muru_cnt > 0) ? (res_wd_cnt / active_muru_cnt) : 0; // for (RU + MU) inactive STA

	/*  // debug
	printk("su_cnt:%d, muru_cnt:%d (inactive_ru_cnt:%d, active_ru_cnt:%d, inactive_mu_cnt:%d, active_mu_cnt:%d)\n",
		su_cnt, muru_cnt, inactive_ru_cnt, active_ru_cnt, inactive_mu_cnt, active_mu_cnt);
	printk("phl_info->su_wd_quota:%d, inactive_quota:%d, active_quota:%d\n",
		phl_info->su_wd_quota, inactive_quota, active_quota);
	//*/

	self = rtw_phl_get_stainfo_self(phl_info, wrole);
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,&wrole->assoc_sta_queue.queue, list)
	{
		if (self == psta){
			psta->phl_muru_wd_quota = 32;
			continue;
		}

		if (psta->active == false){
			continue;
		}

		if((psta->grp_info.is_dlru_sta || psta->grp_info.is_mu_sta) && psta->stats.tx_moving_average_tp > 0) {
			psta->phl_muru_wd_quota = active_quota;
		} else {
			psta->phl_muru_wd_quota = inactive_quota;
		}
		//printk("psta->phl_muru_wd_quota:%d\n", psta->phl_muru_wd_quota);
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
}

enum rtw_phl_status
rtw_phl_ru_group(struct phl_info_t *phl_info,
	      struct rtw_wifi_role_t *wrole, bool reset)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	int i,j;

	if (!_check_wrole_grp_condition(phl_info, wrole)) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[DLRU_GRP] SKIP by check condition fail.\n");
		return pstatus;
	}

	_os_spinlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);
	grp_obj->is_grp_in_progress = true;
	_os_spinunlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);

	grp_obj->wrole = wrole;

	if(!(phl_com->dev_cap.dlul_group_mode & BIT5)){
		if (ru_ctrl->GRP_DL_ON){

			if (ru_ctrl->GRP_CALLBACK_ONCE)
				ru_ctrl->GRP_DL_ON =0;

			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[DLRU_GRP] DL RU Grouping Start\n");

			/* PHASE-0 : Reset SW Resource if needed */
			if (true == reset)
				phl_grp_dlru_reset(phl_info);

			/* PHASE-1 : Level 1 Group by Pwr/STBC/DeviceClass */
			phl_grp_dlru_phase_1(phl_info, wrole);

			/* PHASE-2 : Other Conditions or foreced group. */
			phl_grp_dlru_decision(phl_info);

			if(rtw_phl_check_dlru_grp_tbls_change(phl_info, wrole) || ru_ctrl->GRP_FORCE_FILL_DL_TBL){
				ru_ctrl->GRP_FORCE_FILL_DL_TBL = 0;

				rtw_phl_fill_dlru_grp_tbls(phl_info, wrole);
				grp_obj->dlru.fill_tbl_cnt++;
			}

			rtw_phl_wd_quota_upd(phl_info, wrole); // for DL only

			if(grp_obj->dlru.grp_num)
				ru_ctrl->tbl_exist |= BIT0;
			else
				ru_ctrl->tbl_exist &= ~BIT(0);

			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[RU_GRP] DL RU Grouping Done\n");
		}
	}

	if(!(phl_com->dev_cap.dlul_group_mode & BIT4)){
		if (ru_ctrl->GRP_UL_ON){

			if (ru_ctrl->GRP_CALLBACK_ONCE)
				ru_ctrl->GRP_UL_ON = 0;

			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[ULRU_GRP] UL RU Grouping Start\n");

			if (true == reset)
				phl_grp_ulru_reset(phl_info);

			/* PHASE-1 : Level 1 Group by Pwr/STBC/DeviceClass */
			phl_grp_ulru_phase_1(phl_info, wrole);

			phl_grp_ulru_decision(phl_info);

			if (rtw_phl_check_ulru_grp_tbls_change(phl_info, wrole) || ru_ctrl->GRP_FORCE_FILL_UL_TBL) {

				ru_ctrl->GRP_FORCE_FILL_UL_TBL = 0;

				rtw_phl_fill_ulru_grp_tbls(phl_info, wrole);

				grp_obj->ulru.fill_tbl_cnt++;
			}

			if(grp_obj->ulru.grp_num)
				ru_ctrl->tbl_exist |= BIT4;
			else
				ru_ctrl->tbl_exist &= ~BIT(4);

			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[RU_GRP] UL RU Grouping Done\n");
		}
	}

	_os_spinlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);
	grp_obj->is_grp_in_progress = false;
	_os_spinunlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);

	PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[RU_GRP] RU Grouping Done\n");


	/* PHASE-3 : Update FW information if needed */

#ifdef CONFIG_RTW_DEBUG
	//phl_grp_dump_info_dlru(phl_info, wrole);
#endif

	return pstatus;
}

/* TxBF SU */
/*su entry*/
static struct grp_sta *_query_idle_su_entry(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;

	_os_list *idle_list = &grp_obj->su.idle_list;
	struct grp_sta *entry = NULL;

	//_os_spinlock_bh(drv_priv, &grp_obj->su.list_lock);
	_os_spinlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);
	if (true == list_empty(idle_list)) {
		entry = NULL;
	} else {
		entry = list_first_entry(idle_list, struct grp_sta,
					 list);
		grp_obj->su.idle_cnt--;
		list_del(&entry->list);
	}
	//_os_spinunlock_bh(drv_priv, &grp_obj->su.list_lock);
	_os_spinunlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);

	return entry;
}

static enum rtw_phl_status _enqueue_idle_su_entry(
				struct phl_info_t *phl_info,
				struct grp_sta *entry)
{
	enum rtw_hal_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->su.idle_list;

	if (entry != NULL) {
		//_os_spinlock_bh(drv_priv, &grp_obj->su.list_lock);
		_os_spinlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		grp_obj->su.idle_cnt++;
		//_os_spinunlock_bh(drv_priv, &grp_obj->su.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}

static enum rtw_phl_status _enqueue_busy_su_entry(
				struct phl_info_t *phl_info,
				struct grp_sta *entry)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	_os_list *list = &grp_obj->su.busy_list;

	if (entry != NULL) {
		//_os_spinlock_bh(drv_priv, &grp_obj->su.list_lock);
		_os_spinlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);
		list_add_tail(&entry->list, list);
		//_os_spinunlock_bh(drv_priv, &grp_obj->su.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);
		status = RTW_PHL_STATUS_SUCCESS;
	}

	return status;
}

void _grp_init_su(struct phl_info_t *phl_info)
{
	u8 i = 0;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct grp_sta *su_sta = grp_obj->su.sta;

	for (i = 0 ; i < MAX_SUPPORT_SU_STA_NUM; i++) {
		su_sta[i].grp_bitmap = 0;
		su_sta[i].sta_info = NULL;
		_enqueue_idle_su_entry(phl_info, &su_sta[i]);
	}

	grp_obj->su.para.allow_su_mu = 1;
	grp_obj->su.para.txbf_tp_crit_min = MIN_SU_TP_CRITERIA;
	grp_obj->su.para.vht_he_min_txbf_rate = 6;
}

bool _txbf_su_condition(struct phl_info_t *phl_info,
			struct rtw_phl_stainfo_t *psta)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	bool ret = false;
	struct grp_sta *sta, *n;
	bool in_mu_list = false;
	do {
		if (psta == NULL)
			break;
		/* 1. check txbf capability */
		if ((0 == psta->asoc_cap.vht_su_bfme) &&
		    (0 == psta->asoc_cap.he_su_bfme))
			break;
		/* 2. check tx Performace > su_criteria */
		if (psta->stats.tx_moving_average_tp <=
			grp_obj->su.para.txbf_tp_crit_min) {
				break;
		}
		/* 3. check tx rate , su tx bf won't improve highest rate.*/
		/*TODO: get ra information from tx report */

		/* 4. (option) Check MU BUSY list. if in MU list already, skip it*/
		if (!grp_obj->su.para.allow_su_mu) {
			in_mu_list = false;
			phl_list_for_loop_safe(sta, n, struct grp_sta,
					&grp_obj->mu.busy_list, list) {
				if (psta == sta->sta_info) {
					in_mu_list = true;
					break;
				}
			}
			if (in_mu_list)
				break;
		}

		ret = true;
	} while (0);

	return ret;
}

enum rtw_phl_status
rtw_phl_group_txbf_su(struct phl_info_t *phl_info,
		   struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	struct rtw_grp_info *gsta, *gn;
	struct grp_sta *sta, *n, *su_sta;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_stainfo_t *psta = NULL;

	if (!_check_wrole_grp_condition(phl_info, wrole)) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[SU_GRP] SKIP by check condition fail.\n");
		return pstatus;
	}
	/* Reset SU Busy list */
	phl_list_for_loop_safe(sta, n, struct grp_sta,
			       &grp_obj->su.busy_list, list) {
		//_os_spinlock_bh(drv_priv, &grp_obj->su.list_lock);
		_os_spinlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);
		list_del(&sta->list);
		//_os_spinunlock_bh(drv_priv, &grp_obj->su.list_lock);
		_os_spinunlock(drv_priv, &grp_obj->su.list_lock, _bh, NULL);
		sta->sta_info = NULL;
		_enqueue_idle_su_entry(phl_info, sta);
	}

	/* SU Tx BF Grouping */
	phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
			       &grp_obj->tx_tp_queue.queue, tx_list) {
		if (grp_obj->su.idle_cnt == 0)
			break;

		psta = (struct rtw_phl_stainfo_t *)gsta->sta_info;

		if (false == _txbf_su_condition(phl_info, psta))
			continue;

		/* pass all of the check */
		su_sta = _query_idle_su_entry(phl_info);
		if (su_sta == NULL)
			break;
		su_sta->sta_info = psta;
		_enqueue_busy_su_entry(phl_info, su_sta);
	}

#ifdef CONFIG_RTW_DEBUG
	phl_grp_dump_info_su(phl_info);
#endif

	return pstatus;
}

void phl_bw160_init_8ru_pos(void *phl){
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU996_1*2;
	dl_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU996_1*2;
	dl_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU484_1*2;
	dl_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU484_1*2;
	dl_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU484_1*2;
	dl_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU484_1*2;
	dl_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU242_1*2;

	dl_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU996_1*2 + 1;
	dl_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU484_1*2 + 1;
	dl_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU484_2*2;
	dl_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU484_2*2;
	dl_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU484_2*2;
	dl_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU242_3*2;
	dl_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU242_2*2;

	dl_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU484_2*2 + 1;
	dl_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU484_1*2 + 1;
	dl_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU484_1*2 + 1;
	dl_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU242_1*2 + 1;
	dl_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU242_4*2;
	dl_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU242_3*2;

	dl_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU484_2*2 + 1;
	dl_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU242_3*2 + 1;
	dl_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU242_2*2 + 1;
	dl_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU242_1*2 + 1;
	dl_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU242_4*2;

	dl_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU242_4*2 + 1;
	dl_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU242_3*2 + 1;
	dl_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU242_2*2 + 1;
	dl_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU242_1*2 + 1;

	dl_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU242_4*2 + 1;
	dl_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU242_3*2 + 1;
	dl_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU242_2*2 + 1;

	dl_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU242_4*2 + 1;
	dl_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU242_3*2 + 1;

	dl_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1*2;
	dl_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU242_4*2 + 1;

	ul_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU996_1*2;
	ul_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU996_1*2;
	ul_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU484_1*2;
	ul_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU484_1*2;
	ul_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU484_1*2;
	ul_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU484_1*2;
	ul_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU242_1*2;

	ul_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU996_1*2 + 1;
	ul_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU484_1*2 + 1;
	ul_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU484_2*2;
	ul_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU484_2*2;
	ul_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU484_2*2;
	ul_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU242_3*2;
	ul_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU242_2*2;

	ul_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU484_2*2 + 1;
	ul_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU484_1*2 + 1;
	ul_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU484_1*2 + 1;
	ul_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU242_1*2 + 1;
	ul_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU242_4*2;
	ul_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU242_3*2;

	ul_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU484_2*2 + 1;
	ul_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU242_3*2 + 1;
	ul_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU242_2*2 + 1;
	ul_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU242_1*2 + 1;
	ul_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU242_4*2;

	ul_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU242_4*2 + 1;
	ul_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU242_3*2 + 1;
	ul_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU242_2*2 + 1;
	ul_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU242_1*2 + 1;

	ul_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU242_4*2 + 1;
	ul_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU242_3*2 + 1;
	ul_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU242_2*2 + 1;

	ul_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU242_4*2 + 1;
	ul_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU242_3*2 + 1;

	ul_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1*2;
	ul_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU242_4*2 + 1;

	return;
}

void phl_bw80_init_8ru_pos(void *phl){
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU484_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU484_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU242_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU242_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU242_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU242_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU106_1;

	dl_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU484_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU242_3;
	dl_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU242_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU242_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU242_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU106_3;
	dl_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU106_2;

	dl_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU242_4;
	dl_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU242_3;
	dl_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU242_3;
	dl_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU106_5;
	dl_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU106_4;
	dl_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU106_3;

	dl_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU242_4;
	dl_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU106_7;
	dl_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU106_6;
	dl_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU106_5;
	dl_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU106_4;

	dl_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU106_8;
	dl_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU106_7;
	dl_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU106_6;
	dl_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU106_5;

	dl_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU106_8;
	dl_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU106_7;
	dl_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU106_6;

	dl_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU106_8;
	dl_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU106_7;

	dl_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU106_8;

	ul_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU484_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU484_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU242_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU242_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU242_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU242_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU106_1;

	ul_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU484_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU242_3;
	ul_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU242_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU242_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU242_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU106_3;
	ul_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU106_2;

	ul_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU242_4;
	ul_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU242_3;
	ul_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU242_3;
	ul_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU106_5;
	ul_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU106_4;
	ul_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU106_3;

	ul_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU242_4;
	ul_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU106_7;
	ul_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU106_6;
	ul_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU106_5;
	ul_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU106_4;

	ul_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU106_8;
	ul_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU106_7;
	ul_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU106_6;
	ul_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU106_5;

	ul_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU106_8;
	ul_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU106_7;
	ul_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU106_6;

	ul_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU106_8;
	ul_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU106_7;

	ul_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU106_8;

	return;
}

void phl_bw40_init_8ru_pos(void *phl){
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU242_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU242_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU106_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU106_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU106_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU106_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU52_1;

	dl_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU242_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU106_3;
	dl_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU106_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU106_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU106_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU52_3;
	dl_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU52_2;

	dl_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU106_4;
	dl_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU106_3;
	dl_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU106_3;
	dl_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU52_5;
	dl_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU52_4;
	dl_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU52_3;

	dl_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU106_4;
	dl_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU52_7;
	dl_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU52_6;
	dl_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU52_5;
	dl_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU52_4;

	dl_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU52_8;
	dl_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU52_7;
	dl_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU52_6;
	dl_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU52_5;

	dl_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU52_8;
	dl_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU52_7;
	dl_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU52_6;

	dl_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU52_8;
	dl_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU52_7;

	dl_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU52_8;

	ul_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU242_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU242_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU106_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU106_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU106_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU106_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU52_1;

	ul_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU242_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU106_3;
	ul_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU106_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU106_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU106_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU52_3;
	ul_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU52_2;

	ul_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU106_4;
	ul_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU106_3;
	ul_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU106_3;
	ul_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU52_5;
	ul_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU52_4;
	ul_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU52_3;

	ul_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU106_4;
	ul_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU52_7;
	ul_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU52_6;
	ul_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU52_5;
	ul_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU52_4;

	ul_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU52_8;
	ul_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU52_7;
	ul_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU52_6;
	ul_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU52_5;

	ul_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU52_8;
	ul_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU52_7;
	ul_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU52_6;

	ul_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU52_8;
	ul_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU52_7;

	ul_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU52_8;

	return;
}

void phl_bw20_init_8ru_pos(void *phl){
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU106_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU106_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU52_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU52_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU52_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU52_1;
	dl_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU26_1;

	dl_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU106_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU52_3;
	dl_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU52_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU52_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU52_2;
	dl_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU26_3;
	dl_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU26_2;

	dl_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU52_4;
	dl_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU52_3;
	dl_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU52_3;
	dl_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU26_5;
	dl_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU26_4;
	dl_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU26_3;

	dl_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU52_4;
	dl_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU26_7;
	dl_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU26_6;
	dl_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU26_5;
	dl_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU26_4;

	dl_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU26_8;
	dl_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU26_7;
	dl_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU26_6;
	dl_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU26_5;

	dl_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU26_8;
	dl_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU26_7;
	dl_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU26_6;

	dl_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU26_8;
	dl_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU26_7;

	dl_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1;
	dl_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU26_8;

	ul_ru_fix_grp->sta_info[0].ru_pos[0] = RTW_HE_RU106_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[1] = RTW_HE_RU106_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[2] = RTW_HE_RU52_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[3] = RTW_HE_RU52_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[4] = RTW_HE_RU52_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[5] = RTW_HE_RU52_1;
	ul_ru_fix_grp->sta_info[0].ru_pos[6] = RTW_HE_RU26_1;

	ul_ru_fix_grp->sta_info[1].ru_pos[0] = RTW_HE_RU106_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[1] = RTW_HE_RU52_3;
	ul_ru_fix_grp->sta_info[1].ru_pos[2] = RTW_HE_RU52_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[3] = RTW_HE_RU52_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[4] = RTW_HE_RU52_2;
	ul_ru_fix_grp->sta_info[1].ru_pos[5] = RTW_HE_RU26_3;
	ul_ru_fix_grp->sta_info[1].ru_pos[6] = RTW_HE_RU26_2;

	ul_ru_fix_grp->sta_info[2].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[2].ru_pos[1] = RTW_HE_RU52_4;
	ul_ru_fix_grp->sta_info[2].ru_pos[2] = RTW_HE_RU52_3;
	ul_ru_fix_grp->sta_info[2].ru_pos[3] = RTW_HE_RU52_3;
	ul_ru_fix_grp->sta_info[2].ru_pos[4] = RTW_HE_RU26_5;
	ul_ru_fix_grp->sta_info[2].ru_pos[5] = RTW_HE_RU26_4;
	ul_ru_fix_grp->sta_info[2].ru_pos[6] = RTW_HE_RU26_3;

	ul_ru_fix_grp->sta_info[3].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[3].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[3].ru_pos[2] = RTW_HE_RU52_4;
	ul_ru_fix_grp->sta_info[3].ru_pos[3] = RTW_HE_RU26_7;
	ul_ru_fix_grp->sta_info[3].ru_pos[4] = RTW_HE_RU26_6;
	ul_ru_fix_grp->sta_info[3].ru_pos[5] = RTW_HE_RU26_5;
	ul_ru_fix_grp->sta_info[3].ru_pos[6] = RTW_HE_RU26_4;

	ul_ru_fix_grp->sta_info[4].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[4].ru_pos[3] = RTW_HE_RU26_8;
	ul_ru_fix_grp->sta_info[4].ru_pos[4] = RTW_HE_RU26_7;
	ul_ru_fix_grp->sta_info[4].ru_pos[5] = RTW_HE_RU26_6;
	ul_ru_fix_grp->sta_info[4].ru_pos[6] = RTW_HE_RU26_5;

	ul_ru_fix_grp->sta_info[5].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[5].ru_pos[4] = RTW_HE_RU26_8;
	ul_ru_fix_grp->sta_info[5].ru_pos[5] = RTW_HE_RU26_7;
	ul_ru_fix_grp->sta_info[5].ru_pos[6] = RTW_HE_RU26_6;

	ul_ru_fix_grp->sta_info[6].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[4] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[6].ru_pos[5] = RTW_HE_RU26_8;
	ul_ru_fix_grp->sta_info[6].ru_pos[6] = RTW_HE_RU26_7;

	ul_ru_fix_grp->sta_info[7].ru_pos[0] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[1] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[2] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[3] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[4] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[5] = RTW_HE_RU26_1;
	ul_ru_fix_grp->sta_info[7].ru_pos[6] = RTW_HE_RU26_8;

	return;
}

#if 1  // Mark.CS_update
void rtw_phl_grp_bw_setting_apply(void *phl, unsigned char bw)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	switch (bw) {
		case CHANNEL_WIDTH_160:
			dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_160;
			dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_160;
			dl_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;

			ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_160;
			ul_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;
			ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM54;

			phl_bw160_init_8ru_pos(phl);
			break;
		case CHANNEL_WIDTH_80:
			dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_80;
			dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_80;
			dl_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE08;

			ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_80;
			if(ru_ctrl->ofdma_WFA_mode){
				ul_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;
				ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM54;
			}else{
				ul_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;
				ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;
			}

			phl_bw80_init_8ru_pos(phl);
			break;
		case CHANNEL_WIDTH_40:
			dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_40;
			dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_40;
			dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

			ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_40;
			ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;
			ul_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

			phl_bw40_init_8ru_pos(phl);
			break;
		default:
			dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_20;
			dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_20;
			dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

			ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_20;
			ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;
			ul_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

			phl_bw20_init_8ru_pos(phl);
			break;
	}
}
#endif

void phl_init_ru_ctrl(struct ru_common_ctrl *ru_ctrl){

	ru_ctrl->GRP_CALLBACK_ONCE = 0;
	ru_ctrl->GRP_DL_ON = 1;
	ru_ctrl->GRP_UL_ON = 1;
	ru_ctrl->GRP_FORCE_FILL_DL_TBL = 0;
	ru_ctrl->GRP_FORCE_FILL_UL_TBL = 0;

	ru_ctrl->TX_TP_THRD = 150;
	ru_ctrl->RX_TP_THRD = 150;

	ru_ctrl->tbl_exist = 0; /* default grp table not exist */
	ru_ctrl->rotate = 1;
	ru_ctrl->ofdma_WFA_mode = 0; /* 0: normal mode; 1: logo mode */
	ru_ctrl->auto_config = 1; /* 0: by IO setting; 1: by STA capability */
	ru_ctrl->force_sumuru_en = 1;
	ru_ctrl->forceru = 1;
	ru_ctrl->forcesu = 0;
	ru_ctrl->rsp_type = FRAME_EXCHANGE_MU_BAR;
	ru_ctrl->prot_type = FRAME_EXCHANGE_NO_PROETCT;
	ru_ctrl->ulmacid_cfg = 1;
	ru_ctrl->ulmacid_cfg_fix = 0; /* 0: dynamic;	1: fix */

	ru_ctrl->ul_psd = 1; /* 0: hard code; 1: decided by rssi_m */
	ru_ctrl->psd_ofst1 = 3;
	ru_ctrl->psd_ofst2 = 6;
	ru_ctrl->ul_crc32 = 1; /* 0: disable crc32 pkt; 1: enable crc32 pkt */

	ru_ctrl->tx_phase = 0;
	ru_ctrl->netif_drop_thd = 4;

	ru_ctrl->phl_wd_hi_thd = 0;
	ru_ctrl->phl_wd_hold_cnt_thd = 20;

	ru_ctrl->ru_rate_idx1 = 9;
	ru_ctrl->ru_rate_idx2 = 5;

	ru_ctrl->ru_rssi_level1 = 65;
	ru_ctrl->ru_rssi_level2 = 50;

	ru_ctrl->phl_wd_quota[0] = 400;
	ru_ctrl->phl_wd_quota[1] = 400;
	ru_ctrl->phl_wd_quota[2] = 400;
	ru_ctrl->phl_wd_quota[3] = 400;
	ru_ctrl->phl_wd_quota[4] = 400;
	ru_ctrl->phl_wd_quota[5] = 400;
	ru_ctrl->phl_wd_quota[6] = 400;
	ru_ctrl->phl_wd_quota[7] = 400;
	ru_ctrl->phl_wd_quota[8] = 400;

	ru_ctrl->mu_edca = 8;

	return;
}

void phl_init_dl_ru_grp_para(struct dl_ru_grp_table_para *dl_ru_grp_table){

	dl_ru_grp_table->tx_pwr = 0x3c;
	dl_ru_grp_table->ppdu_bw = CHANNEL_WIDTH_80;
	dl_ru_grp_table->txpwr_ofld_en = 1;
	dl_ru_grp_table->pwrlim_dis = 1;
	dl_ru_grp_table->tf.rate.mcs = 4;
	dl_ru_grp_table->tf.rate.dcm = 0;
	dl_ru_grp_table->tf.rate.ss = 0;

	dl_ru_grp_table->tf.tf_rate = 8;
	dl_ru_grp_table->tf.tb_ppdu_bw = CHANNEL_WIDTH_80;
	dl_ru_grp_table->tf.gi_ltf = 0;
	dl_ru_grp_table->tf.fix_ba = 1;
	dl_ru_grp_table->tf.ru_psd = 290;
	dl_ru_grp_table->tf.rf_gain_fix = 0;
	dl_ru_grp_table->tf.rf_gain_idx = 0;
	dl_ru_grp_table->tf.doppler = 0;
	dl_ru_grp_table->tf.stbc = 0;
	dl_ru_grp_table->tf.sta_coding = 1;
	dl_ru_grp_table->tf.tb_t_pe_nom = 2;
	dl_ru_grp_table->tf.pr20_bw_en = 0;

	return;
}

void phl_init_dl_ru_fix_grp_para(struct dl_ru_fix_grp_table_para *dl_ru_fix_grp_table){

	dl_ru_fix_grp_table->max_sta_num = 4;
	dl_ru_fix_grp_table->min_sta_num = 4;
	dl_ru_fix_grp_table->rupos_csht_flag = 1;
	dl_ru_fix_grp_table->ru_swp_flg = 1;
	dl_ru_fix_grp_table->gi_ltf = RTW_GILTF_2XHE08;
	dl_ru_fix_grp_table->fixru_flag = 1;

	dl_ru_fix_grp_table->sta_info[0].macid = 1;
	dl_ru_fix_grp_table->sta_info[0].mcs = 11;
	dl_ru_fix_grp_table->sta_info[0].ss = 1;
	dl_ru_fix_grp_table->sta_info[0].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[0].coding = 1;

	dl_ru_fix_grp_table->sta_info[1].macid = 2;
	dl_ru_fix_grp_table->sta_info[1].mcs = 11;
	dl_ru_fix_grp_table->sta_info[1].ss = 1;
	dl_ru_fix_grp_table->sta_info[1].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[1].coding = 1;

	dl_ru_fix_grp_table->sta_info[2].macid = 3;
	dl_ru_fix_grp_table->sta_info[2].mcs = 11;
	dl_ru_fix_grp_table->sta_info[2].ss = 1;
	dl_ru_fix_grp_table->sta_info[2].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[2].coding = 1;

	dl_ru_fix_grp_table->sta_info[3].macid = 4;
	dl_ru_fix_grp_table->sta_info[3].mcs = 11;
	dl_ru_fix_grp_table->sta_info[3].ss = 1;
	dl_ru_fix_grp_table->sta_info[3].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[3].coding = 1;

	dl_ru_fix_grp_table->sta_info[4].macid = 5;
	dl_ru_fix_grp_table->sta_info[4].mcs = 11;
	dl_ru_fix_grp_table->sta_info[4].ss = 1;
	dl_ru_fix_grp_table->sta_info[4].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[4].coding = 1;

	dl_ru_fix_grp_table->sta_info[5].macid = 6;
	dl_ru_fix_grp_table->sta_info[5].mcs = 11;
	dl_ru_fix_grp_table->sta_info[5].ss = 1;
	dl_ru_fix_grp_table->sta_info[5].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[5].coding = 1;

	dl_ru_fix_grp_table->sta_info[6].macid = 7;
	dl_ru_fix_grp_table->sta_info[6].mcs = 11;
	dl_ru_fix_grp_table->sta_info[6].ss = 1;
	dl_ru_fix_grp_table->sta_info[6].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[6].coding = 1;

	dl_ru_fix_grp_table->sta_info[7].macid = 8;
	dl_ru_fix_grp_table->sta_info[7].mcs = 11;
	dl_ru_fix_grp_table->sta_info[7].ss = 1;
	dl_ru_fix_grp_table->sta_info[7].fix_rate = 0;
	dl_ru_fix_grp_table->sta_info[7].coding = 1;

	return;
}

void phl_init_ul_ru_grp_para(struct ul_ru_grp_table_para *ul_ru_grp_table){

	ul_ru_grp_table->ppdu_bw = CHANNEL_WIDTH_80;
	ul_ru_grp_table->grp_psd_max = 0xd1; //0x85;
	ul_ru_grp_table->grp_psd_min = 0xd1; //0x6d;
	ul_ru_grp_table->fix_tf_rate = 1;
	ul_ru_grp_table->rf_gain_fix = 0;
	ul_ru_grp_table->fix_mode_flags = 1;
	ul_ru_grp_table->tf_rate = RTW_DATA_RATE_OFDM54;
	ul_ru_grp_table->rf_gain_idx = 0;

	return;
}

void phl_init_ul_ru_fix_grp_para(struct ul_ru_fix_grp_table_para *ul_ru_fix_grp_table){

	ul_ru_fix_grp_table->min_sta_num = 4;
	ul_ru_fix_grp_table->max_sta_num = 4;
	ul_ru_fix_grp_table->gi_ltf = RTW_GILTF_LGI_4XHE32;
	ul_ru_fix_grp_table->fixru_flag = 1;

	ul_ru_fix_grp_table->sta_info[0].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[0].mcs = 7;
	ul_ru_fix_grp_table->sta_info[0].ss = 1;
	ul_ru_fix_grp_table->sta_info[0].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[0].coding = 1;
	ul_ru_fix_grp_table->sta_info[0].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[0].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[0].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[0].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[0].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[0].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[0].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[0].tgt_rssi[6] = 65;

	ul_ru_fix_grp_table->sta_info[1].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[1].mcs = 7;
	ul_ru_fix_grp_table->sta_info[1].ss = 1;
	ul_ru_fix_grp_table->sta_info[1].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[1].coding = 1;
	ul_ru_fix_grp_table->sta_info[1].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[1].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[1].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[1].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[1].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[1].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[1].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[1].tgt_rssi[6] = 65;

	ul_ru_fix_grp_table->sta_info[2].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[2].mcs = 7;
	ul_ru_fix_grp_table->sta_info[2].ss = 1;
	ul_ru_fix_grp_table->sta_info[2].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[2].coding = 1;
	ul_ru_fix_grp_table->sta_info[2].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[2].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[2].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[2].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[2].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[2].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[2].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[2].tgt_rssi[6] = 65;

	ul_ru_fix_grp_table->sta_info[3].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[3].mcs = 7;
	ul_ru_fix_grp_table->sta_info[3].ss = 1;
	ul_ru_fix_grp_table->sta_info[3].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[3].coding = 1;
	ul_ru_fix_grp_table->sta_info[3].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[3].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[3].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[3].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[3].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[3].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[3].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[3].tgt_rssi[6] = 65;

	ul_ru_fix_grp_table->sta_info[4].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[4].mcs = 7;
	ul_ru_fix_grp_table->sta_info[4].ss = 1;
	ul_ru_fix_grp_table->sta_info[4].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[4].coding = 1;
	ul_ru_fix_grp_table->sta_info[4].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[4].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[4].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[4].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[4].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[4].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[4].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[4].tgt_rssi[6] = 65;

	ul_ru_fix_grp_table->sta_info[5].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[5].mcs = 7;
	ul_ru_fix_grp_table->sta_info[5].ss = 1;
	ul_ru_fix_grp_table->sta_info[5].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[5].coding = 1;
	ul_ru_fix_grp_table->sta_info[5].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[5].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[5].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[5].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[5].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[5].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[5].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[5].tgt_rssi[6] = 65;

	ul_ru_fix_grp_table->sta_info[6].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[6].mcs = 7;
	ul_ru_fix_grp_table->sta_info[6].ss = 1;
	ul_ru_fix_grp_table->sta_info[6].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[6].coding = 1;
	ul_ru_fix_grp_table->sta_info[6].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[6].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[6].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[6].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[6].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[6].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[6].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[6].tgt_rssi[6] = 65;

	ul_ru_fix_grp_table->sta_info[7].macid = 0xff;
	ul_ru_fix_grp_table->sta_info[7].mcs = 7;
	ul_ru_fix_grp_table->sta_info[7].ss = 1;
	ul_ru_fix_grp_table->sta_info[7].fix_rate = 1;
	ul_ru_fix_grp_table->sta_info[7].coding = 1;
	ul_ru_fix_grp_table->sta_info[7].fix_tgt_rssi = 1;
	ul_ru_fix_grp_table->sta_info[7].tgt_rssi[0] = 65;
	ul_ru_fix_grp_table->sta_info[7].tgt_rssi[1] = 65;
	ul_ru_fix_grp_table->sta_info[7].tgt_rssi[2] = 65;
	ul_ru_fix_grp_table->sta_info[7].tgt_rssi[3] = 65;
	ul_ru_fix_grp_table->sta_info[7].tgt_rssi[4] = 65;
	ul_ru_fix_grp_table->sta_info[7].tgt_rssi[5] = 65;
	ul_ru_fix_grp_table->sta_info[7].tgt_rssi[6] = 65;

	return;
}

void phl_init_ulmacid_cfg_set_para(struct ulmacid_cfg_set_para *ulmacid_cfg_set)
{
	ulmacid_cfg_set->ulmacid_cfg[0].macid = 1;
	ulmacid_cfg_set->ulmacid_cfg[0].endcmd = 0;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_bw = 0;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_gi_ltf = 1;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_doppler_ctrl = 0;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_dcm = 1;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_ss = 2;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_mcs = 7;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_stbc = 0;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_coding = 1;
	ulmacid_cfg_set->ulmacid_cfg[0].ul_su_rssi_m = 100;

	ulmacid_cfg_set->ulmacid_cfg[1].macid = 2;
	ulmacid_cfg_set->ulmacid_cfg[1].endcmd = 0;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_bw = 0;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_gi_ltf = 2;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_doppler_ctrl = 0;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_dcm = 1;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_ss = 2;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_mcs = 8;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_stbc = 0;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_coding = 1;
	ulmacid_cfg_set->ulmacid_cfg[1].ul_su_rssi_m = 101;

	ulmacid_cfg_set->ulmacid_cfg[2].macid = 3;
	ulmacid_cfg_set->ulmacid_cfg[2].endcmd = 0;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_bw = 0;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_gi_ltf = 3;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_doppler_ctrl = 0;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_dcm = 1;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_ss = 2;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_mcs = 9;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_stbc = 0;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_coding = 1;
	ulmacid_cfg_set->ulmacid_cfg[2].ul_su_rssi_m = 102;

	ulmacid_cfg_set->ulmacid_cfg[3].macid = 4;
	ulmacid_cfg_set->ulmacid_cfg[3].endcmd = 1;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_bw = 0;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_gi_ltf = 3;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_doppler_ctrl = 0;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_dcm = 1;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_ss = 2;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_mcs = 9;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_stbc = 0;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_coding = 1;
	ulmacid_cfg_set->ulmacid_cfg[3].ul_su_rssi_m = 102;

	return;
}


enum rtw_phl_status
phl_ru_grp_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct ru_grp_table *rugrptable = &phl_info->phl_com->rugrptable;

	phl_init_ru_ctrl(&rugrptable->ru_ctrl);

	phl_init_dl_ru_grp_para(&rugrptable->dl_ru_grp_table);

	phl_init_dl_ru_fix_grp_para(&rugrptable->dl_ru_fix_grp_table);

	phl_init_ul_ru_grp_para(&rugrptable->ul_ru_grp_table);

	phl_init_ul_ru_fix_grp_para(&rugrptable->ul_ru_fix_grp_table);

	phl_bw80_init_8ru_pos(phl_info);

	phl_init_ulmacid_cfg_set_para(&rugrptable->ulmacid_cfg_set);

	return pstatus;
}

/* Start of COMMON */
enum rtw_phl_status
phl_grp_obj_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_grp_obj *grp_obj = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);
	FUNCIN();
	do {
		phl_info->grp_obj = _os_mem_alloc(drv_priv,
					sizeof(struct phl_grp_obj));
		if (NULL == phl_info->grp_obj) {
			break;
		}
		grp_obj = phl_info->grp_obj;

		pq_init(drv_priv, &grp_obj->tx_tp_queue);
		pq_init(drv_priv, &grp_obj->rx_tp_queue);

		_os_spinlock_init(drv_priv, &grp_obj->grp_lock);
		_os_spinlock_init(drv_priv, &grp_obj->su.list_lock);
		_os_spinlock_init(drv_priv, &grp_obj->mu.list_lock);
		_os_spinlock_init(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinlock_init(drv_priv, &grp_obj->ulru.list_lock);

		/* init mu entry list */
		INIT_LIST_HEAD(&grp_obj->su.idle_list);
		INIT_LIST_HEAD(&grp_obj->su.busy_list);
		INIT_LIST_HEAD(&grp_obj->mu.idle_list);
		INIT_LIST_HEAD(&grp_obj->mu.busy_list);
		INIT_LIST_HEAD(&grp_obj->dlru.idle_list);
		INIT_LIST_HEAD(&grp_obj->dlru.busy_list);
		INIT_LIST_HEAD(&grp_obj->ulru.idle_list);
		INIT_LIST_HEAD(&grp_obj->ulru.busy_list);
		_grp_init_su(phl_info);
		_grp_init_mu(phl_info);
		_grp_init_dlru(phl_info);
		_grp_init_ulru(phl_info);
		phl_ru_grp_init(phl_info);
		grp_obj->is_grp_in_progress = false;

		pstatus = RTW_HAL_STATUS_SUCCESS;
	} while (0);
	FUNCOUT();
	return pstatus;
}


enum rtw_phl_status
phl_grp_obj_deinit(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	do {
		if (phl_info->grp_obj == NULL)
			break;

		_os_spinlock_free(drv_priv, &grp_obj->grp_lock);
		pq_deinit(drv_priv, &grp_obj->tx_tp_queue);
		pq_deinit(drv_priv, &grp_obj->rx_tp_queue);
		_os_spinlock_free(drv_priv, &grp_obj->su.list_lock);
		_os_spinlock_free(drv_priv, &grp_obj->mu.list_lock);
		_os_spinlock_free(drv_priv, &grp_obj->dlru.list_lock);
		_os_spinlock_free(drv_priv, &grp_obj->ulru.list_lock);

		_os_mem_free(phl_to_drvpriv(phl_info), phl_info->grp_obj,
			     sizeof(struct phl_grp_obj));
		phl_info->grp_obj = NULL;
	} while (0);

	return pstatus;
}

bool phl_grp_is_in_progress(struct phl_info_t *phl_info)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	bool ret = false;
	//_os_spinlock_bh(drv_priv, &grp_obj->grp_lock);
	_os_spinlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);
	ret = grp_obj->is_grp_in_progress;
	//_os_spinunlock_bh(drv_priv, &grp_obj->grp_lock);
	_os_spinunlock(drv_priv, &grp_obj->grp_lock, _bh, NULL);
	return ret;
}

/*
struct rtw_phl_stainfo_t *
rtw_phl_grp_get_highest_txtp_sta(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_grp_info *gsta;
	struct rtw_phl_stainfo_t *ret = NULL;

	//_os_spinlock_bh(drv_priv, &grp_obj->tx_tp_queue.lock);
	_os_spinlock(drv_priv, &grp_obj->tx_tp_queue.lock, _bh, NULL);
	if(!list_empty(&grp_obj->tx_tp_queue.queue)) {
		gsta = list_first_entry(&grp_obj->tx_tp_queue.queue,
					struct rtw_grp_info, tx_list);
		ret = (struct rtw_phl_stainfo_t *)gsta->sta_info;
	}
	//_os_spinunlock_bh(drv_priv, &grp_obj->tx_tp_queue.lock);
	_os_spinunlock(drv_priv, &grp_obj->tx_tp_queue.lock, _bh, NULL);

	return ret;
}
*/

int
rtw_phl_grp_sort_tx_performance(struct phl_info_t *phl_info,
     				struct rtw_wifi_role_t *wrole)
{
	struct rtw_phl_stainfo_t *psta, *n, *tmp, *self;
	struct rtw_grp_info *gsta, *gn;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_queue *tx_q = &grp_obj->tx_tp_queue;
	u32 tp = 0;
	_os_spinlockfg sp_flags;

	if (!_check_wrole_grp_condition(phl_info, wrole))
		return 0;

	self = rtw_phl_get_stainfo_self(phl_info, wrole);

	/* Reset */
	pq_reset(drv_priv, tx_q, _ps);
	tx_q->cnt = 0;

	/* Copy assoc_sta_queue to tx_tp_list and sorted by Tx performace */
	/* 1. Get one asoc sta from the wrole */
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
			       &wrole->assoc_sta_queue.queue, list) {
		if (self == psta)
			continue;
		if (psta->active == false)
			continue;
		psta->grp_info.sta_info = (void *)psta;
		/* if TxPerformance Queue is empty, add to HEAD.*/
		if (tx_q->cnt == 0) {
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
				  "[GRP] tx sort : tx_q->cnt == 0, add to HEAD \n");
			//_push_back(drv_priv, tx_q, &psta->grp_info.tx_list);
			pq_push(drv_priv, tx_q, &psta->grp_info.tx_list, _tail, _ps);
			continue;
		}

		_os_spinlock(drv_priv, &tx_q->lock, _bh, NULL);
		/* 2. Compared with existed STAs in TxPerformace Queue */
		phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
			       &tx_q->queue, tx_list) {
			tmp = (struct rtw_phl_stainfo_t *)gsta->sta_info;
			#if 0
			if (psta->stats.tx_moving_average_tp >
			    tmp->stats.tx_moving_average_tp) {
			#else
			if (psta->stats.tx_tp_kbits >
			    tmp->stats.tx_tp_kbits) {
			#endif
				/* 2-1. bigger than the STA in queue, add to queqe front the STA   */
				list_add(&psta->grp_info.tx_list,
					 gsta->tx_list.prev);
				tx_q->cnt++;
				break;
			} else {
				/**
				 * 2-1. Small than the STA, find next STA unless the STA is the end of Queue.
				 **/
				if (gsta->tx_list.next == &tx_q->queue) {
					list_add(&psta->grp_info.tx_list,
						 &gsta->tx_list);
					tx_q->cnt++;
					break;
				}
			}
		}
		_os_spinunlock(drv_priv, &tx_q->lock, _bh, NULL);
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);

	if (tx_q->cnt != 0) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[GRP] DUMP Tx Performanc Sorting Result :\n");
		phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
			       &tx_q->queue, tx_list) {
			tmp = (struct rtw_phl_stainfo_t *)gsta->sta_info;
			if (tmp != NULL) {
				PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
					  "[GRP] macid : 0x%x . Tx TP = %d (Mbps)\n",
					  tmp->macid,
					  tmp->stats.tx_tp_kbits >> 10);
			}
		}
	}

	return tx_q->cnt;
}

int
rtw_phl_grp_sort_rx_performance(struct phl_info_t *phl_info,
				struct rtw_wifi_role_t *wrole)
{
	struct rtw_phl_stainfo_t *psta, *n, *tmp, *self;
	struct rtw_grp_info *gsta, *gn;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_queue *rx_q = &grp_obj->rx_tp_queue;
	u32 tp = 0;
	_os_spinlockfg sp_flags;

	if (!_check_wrole_grp_condition(phl_info, wrole))
		return 0;

	self = rtw_phl_get_stainfo_self(phl_info, wrole);

	/* Reset */
	pq_reset(drv_priv, rx_q, _ps);
	rx_q->cnt = 0;

	/* Copy assoc_sta_queue to rx_tp_list and sorted by Rx performace */
	/* 1. Get one asoc sta from the wrole */
	_os_spinlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);
	phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
				   &wrole->assoc_sta_queue.queue, list) {
		if (self == psta)
			continue;
		if (psta->active == false)
			continue;
		psta->grp_info.sta_info = (void *)psta;
		/* if RxPerformance Queue is empty, add to HEAD.*/
		if (rx_q->cnt == 0) {
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
				  "[GRP] rx sort : rx_q->cnt == 0, add to HEAD \n");
			//_push_back(drv_priv, rx_q, &psta->grp_info.rx_list);
			pq_push(drv_priv, rx_q, &psta->grp_info.rx_list, _tail, _ps);
			continue;
		}

		_os_spinlock(drv_priv, &rx_q->lock, _bh, NULL);
		/* 2. Compared with existed STAs in RxPerformace Queue */
		phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
				   &rx_q->queue, rx_list) {
			tmp = (struct rtw_phl_stainfo_t *)gsta->sta_info;
			#if 0
			if (psta->stats.rx_moving_average_tp >
				tmp->stats.rx_moving_average_tp) {
			#else
			if (psta->stats.rx_tp_kbits >
				tmp->stats.rx_tp_kbits) {
			#endif
				/* 2-1. bigger than the STA in queue, add to queqe front the STA   */
				list_add(&psta->grp_info.rx_list,
					 gsta->rx_list.prev);
				rx_q->cnt++;
				break;
			} else {
				/**
				 * 2-1. Small than the STA, find next STA unless the STA is the end of Queue.
				 **/
				if (gsta->rx_list.next == &rx_q->queue) {
					list_add(&psta->grp_info.rx_list,
						 &gsta->rx_list);
					rx_q->cnt++;
					break;
				}
			}
		}
		_os_spinunlock(drv_priv, &rx_q->lock, _bh, NULL);
	}
	_os_spinunlock(drv_priv, &wrole->assoc_sta_queue.lock, _irq, &sp_flags);

	if (rx_q->cnt != 0) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[GRP] DUMP Rx Performanc Sorting Result :\n");
		phl_list_for_loop_safe(gsta, gn, struct rtw_grp_info,
				   &rx_q->queue, rx_list) {
			tmp = (struct rtw_phl_stainfo_t *)gsta->sta_info;
			if (tmp != NULL) {
				PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
					  "[GRP] macid : 0x%x . Avg TP = %d (Mbps)\n",
					  tmp->macid,
					  tmp->stats.rx_tp_kbits >> 10);
			}
		}
	}

	return rx_q->cnt;
}


enum rtw_phl_status
rtw_phl_grp_watchdog_callback(void *phl,
			      struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	bool snd_needed = false;
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	/* Check Wrole Capability */
	if (!_check_wrole_grp_condition(phl_info, wrole)) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[GRP] WatchDog Callback SKIP by check condition fail.\n");
		return pstatus;
	}

	/* Get Tx Performace Queue */
	if(!(phl_com->dev_cap.dlul_group_mode & BIT5) &&
		ru_ctrl->GRP_DL_ON)
	{
		if(0 == rtw_phl_grp_sort_tx_performance(phl_info, wrole)) {
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[GRP] Sorting STA by Tx Performance Fail, Skip Grouping.\n");
			return pstatus;
		}
	}

	/* Get Rx Performace Queue */
	if(!(phl_com->dev_cap.dlul_group_mode & BIT4) &&
		ru_ctrl->GRP_UL_ON)
	{
		if(0 == rtw_phl_grp_sort_rx_performance(phl_info, wrole)) {
			PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_, "[GRP] Sorting STA by Rx Performance Fail, Skip Grouping.\n");
			return pstatus;
		}
	}

#if 0
	/* MU Grouping */
	rtw_phl_group_mu(phl_info, wrole);
	/* SU Grouping : shall alway after MU grouping */
	rtw_phl_group_txbf_su(phl_info, wrole);
#endif

	/* RU Grouping */
	rtw_phl_ru_group(phl_info, wrole, true);
#if 0
	/* Trigger Sounding if needed. */
	if(_check_snd_need(phl_info, wrole)) {
		PHL_TRACE(COMP_PHL_GROUP, _PHL_INFO_,
			  "[GRP] Sounding Needed, send sounding request to snd_fsm \n");
		/* TODO: wait sounding fsm ready */
		/* _trigger_sounding(phl_info, wrole); */
	}
#endif
	/* Update FW RU information if needed. */
	return pstatus;
}

u8 phl_grp_get_dlru_grp_num(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	return grp_obj->dlru.grp_num;
}

u8 phl_grp_get_ulru_grp_num(struct phl_info_t *phl_info)
{
	struct phl_grp_obj *grp_obj = phl_info->grp_obj;
	return grp_obj->ulru.grp_num;
}

void phl_grp_dump_info(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{
	phl_grp_dump_info_su(phl_info);
	phl_grp_dump_info_mu(phl_info);
	phl_grp_dump_info_dlru(phl_info, wrole);
	phl_grp_dump_info_ulru(phl_info, wrole);
}

#endif
