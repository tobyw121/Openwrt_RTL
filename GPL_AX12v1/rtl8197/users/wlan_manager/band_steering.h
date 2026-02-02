#ifndef _BAND_STEERING_H_
#define _BAND_STEERING_H_

#include "common.h"

#if 1 /* sync wifi driver rtw_band_steering.h */

#define B_STEER_ENTRY_NUM					64

#endif /* sync wifi driver */

#define B_STEER_OUTPUT						"/var/run/band_steering.out"
#define B_STEER_STR							"[BAND_STEERING]"
#define B_STEER_PREFER_BAND_ENTRY_TIMEOUT	150
#define B_STEER_ENTRY_IGNORE_PERIOD			15
#define B_STEER_ROAM_STA_ONCE_NUMBER		1
#define CTX									(ctx)

#ifdef INITIATIVE_DUAL_BAND_DETECT
#define BSS_TM_REQ_RETRY_LIMIT (5)
#define BSS_TM_REQ_INTERVAL (30)	//30s
#define BSS_TM_REQ_TP_LIMIT (100)	//100 kbits
#endif

struct b_steer_ignore_entry {
	u8  used;
	u8  mac[6];
	u32 entry_exp;
	struct com_wifi_list list;
};

struct b_steer_prefer_band_entry { /* allow */
	u8  used;
	u8  mac[6];
	u8  rssi;
	u32 aging;
	struct com_wifi_list list;
};

struct b_steer_non_prefer_band_entry { /* block */
	u8  used;
	u8  mac[6];
	u32 entry_exp_phase1;
	u32 assoc_rty_lmt;
	u32 entry_exp_phase2;
	struct com_wifi_list list;
};

struct b_steer_context {
	u8  band_steering_enable;

	/* common parameters */
	u8  prefer_band;
	u8  non_prefer_band;
	/* bss_tm_req parameters */
	u8  bss_tm_req_th;
	u8  bss_tm_req_disassoc_imminent;
	u32 bss_tm_req_disassoc_timer;

	/* band roaming parameters */
	u8  roam_detect_th;
	u32 roam_sta_tp_th;
	/* CLM: non prefer band --> prefer band */
	u8  roam_ch_clm_th;

	/* rssi parameters */
	u8 prefer_band_rssi_high;
	u8 prefer_band_rssi_low;

	/* non prefer band parameters */
	u32 entry_exp_phase1;
	u32 assoc_rty_lmt;
	u32 entry_exp_phase2;

	/* group */
	/* index is ths ssid number of the band, */
	/* and the content is the band steering  */
	/* ssid number (another band group ssid) */
	u8 prefer_band_grp_ssid[SSID_NUM];
	u8 non_prefer_band_grp_ssid[SSID_NUM];

	/* data structure */
	struct b_steer_prefer_band_entry
		p_band_list[SSID_NUM][B_STEER_ENTRY_NUM];
	struct b_steer_non_prefer_band_entry
		np_band_list[SSID_NUM][B_STEER_ENTRY_NUM];
	struct b_steer_ignore_entry
		np_band_ignore_list[SSID_NUM][B_STEER_ENTRY_NUM];

	struct com_wifi_list *p_band_hash_tbl[SSID_NUM][HASH_TBL_SIZE];
	struct com_wifi_list *np_band_hash_tbl[SSID_NUM][HASH_TBL_SIZE];
	struct com_wifi_list *np_band_ignore_hash_tbl[SSID_NUM][HASH_TBL_SIZE];
};


/* ------------------------------------------- */
/* ---------- APIs for wlan_manager ---------- */
/* ------------------------------------------- */
void _band_steering_parse_arg(u8 *argn, s32 argc, s8 *argv[]);
void _band_steering_on_frame_rpt(
	struct com_priv *priv, u16 frame_type, u8 *mac, u8 rssi);
void _band_steering_on_time_tick(struct com_priv *priv);
void _band_steering_roam_detect(struct com_priv *priv);
void _band_steering_roam_start(struct com_priv *priv);
void _band_steering_on_cmd(struct com_priv *priv);
void _band_steering_on_config_update(s8 *config);
void _band_steering_init(struct com_device *device);
void _band_steering_deinit(void);

#endif

