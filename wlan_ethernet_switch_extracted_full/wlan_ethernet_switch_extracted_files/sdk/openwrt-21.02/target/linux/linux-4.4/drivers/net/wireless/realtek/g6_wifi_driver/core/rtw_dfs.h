#ifndef __RTW_DFS_H__
#define __RTW_DFS_H__

#define CAC_TIME_MS (60*1000)
#define CAC_TIME_CE_MS (10*60*1000)
#define NON_OCP_TIME_MS (30*60*1000)

#define RTW_CAC_STOPPED 0 /* used by cac_start_time, cac_end_time time stamps */

#ifdef CONFIG_DFS_MASTER
#define IS_CAC_STOPPED(rfctl) ((rfctl)->cac_end_time == RTW_CAC_STOPPED)
#define IS_CH_WAITING(rfctl) (!IS_CAC_STOPPED(rfctl) && rtw_time_after((rfctl)->cac_end_time, rtw_get_current_time()))
#define IS_UNDER_CAC(rfctl) (IS_CH_WAITING(rfctl) && rtw_time_after(rtw_get_current_time(), (rfctl)->cac_start_time))
#define IS_RADAR_DETECTED(rfctl) ((rfctl)->radar_detected)
#define CH_IS_NON_OCP(rt_ch_info) (rtw_time_after((rt_ch_info)->non_ocp_end_time, rtw_get_current_time()))
#else
#define IS_CAC_STOPPED(rfctl) 1
#define IS_CH_WAITING(rfctl) 0
#define IS_UNDER_CAC(rfctl) 0
#define IS_RADAR_DETECTED(rfctl) 0
#define CH_IS_NON_OCP(rt_ch_info) 0
#endif /* CONFIG_DFS_MASTER */

#if CONFIG_DFS_SLAVE_WITH_RADAR_DETECT
#define IS_DFS_SLAVE_WITH_RD(rfctl) ((rfctl)->dfs_slave_with_rd)
#else
#define IS_DFS_SLAVE_WITH_RD(rfctl) 0
#endif

#define RADAR_DETECT_POLLING_INT 110 /* from rtw_dfs_hal_radar_detect_polling_int_ms(), although it is 100 currently, but in system it is 110ms */

#ifdef CONFIG_DFS_MASTER
void rtw_dfs_backup_non_ocp_time(RT_CHANNEL_INFO *ch_set, systime *non_ocp_arr);
void rtw_dfs_restore_non_ocp_time(RT_CHANNEL_INFO *ch_set, systime *non_ocp_arr);
u8 rtw_dfs_rd_cmd(_adapter *adapter, bool enqueue);
void rtw_dfs_rd_timer_hdl(void *ctx);
void rtw_dfs_rd_en_decision(_adapter *adapter, u8 mlme_act, u16 excl_ifbmp);
u8 rtw_dfs_rd_en_decision_cmd(_adapter *adapter);
void rtw_dfs_rd_detect_onoff(_adapter *padapter, u8 enable);
bool rtw_is_cac_reset_needed(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset);
bool _rtw_rfctl_overlap_radar_detect_ch(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset);
bool rtw_rfctl_overlap_radar_detect_ch(struct rf_ctl_t *rfctl);
bool rtw_rfctl_is_tx_blocked_by_ch_waiting(struct rf_ctl_t *rfctl);
bool rtw_chset_is_chbw_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset);
bool rtw_chset_is_ch_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch);
bool rtw_chset_update_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset);
bool rtw_chset_update_non_ocp_ms(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset, int ms);
u32 rtw_get_ch_waiting_ms(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset, u32 *r_non_ocp_ms, u32 *r_cac_ms);
void rtw_reset_cac(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset);
u32 rtw_force_stop_cac(struct rf_ctl_t *rfctl, u32 timeout_ms);
void rtw_init_dfs_region(_adapter *padapter);
u8 rtw_dfs_rd_hdl(_adapter *adapter);
bool rtw_customer_req_channel_sel(_adapter *padapter, struct rf_ctl_t *rfctl, u8 sel_ch, u8 max_bw
	, u8 *dec_ch, u8 *dec_bw, u8 *dec_offset
	, u8 e_flags, u8 d_flags, u8 cur_ch, bool by_int_info, u8 mesh_only);
#else
#define rtw_chset_is_chbw_non_ocp(ch_set, ch, bw, offset) _FALSE
#define rtw_chset_is_ch_non_ocp(ch_set, ch) _FALSE
#define rtw_rfctl_is_tx_blocked_by_ch_waiting(rfctl) _FALSE
#endif

enum rtw_dfs_regd rtw_rfctl_get_dfs_domain_g6(struct rf_ctl_t *rfctl);
bool rtw_rfctl_dfs_domain_unknown_g6(struct rf_ctl_t *rfctl);
void rtw_rfctl_dfs_init(struct rf_ctl_t *rfctl, struct registry_priv *regsty);
bool rtw_choose_shortest_waiting_ch(struct rf_ctl_t *rfctl, u8 sel_ch, u8 max_bw
	, u8 *dec_ch, u8 *dec_bw, u8 *dec_offset
	, u8 e_flags, u8 d_flags, u8 cur_ch, bool by_int_info, u8 mesh_only);

#endif
