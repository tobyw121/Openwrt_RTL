#ifndef	_CORE_SYNC_H_
#define _CORE_SYNC_H_

#define MAX_VAR_LEN	128

#define PE_INFO_SYNC
#define PE_RX_INFO_REPORT

/*
*	var num
*/
typedef enum {
	//PRIV
	VAR_if_id = 0,
#if defined(RTK_BR_EXT)
	VAR_br_mac,
#endif
	//MIB, dot11StationConfigEntry 
	VAR_dot11Bssid,
	VAR_fixedTxRate,
	VAR_autoRate,
	VAR_lowestMlcstRate,
	VAR_swTkipMic,
	VAR_dot11swcrypto,
	VAR_dot11OperationalRateSet,
	VAR_dot11OperationalRateSetLen,

	//MIB, dot1180211AuthEntry
	VAR_dot11PrivacyAlgrthm,
	VAR_dot11EnablePSK,
	VAR_dot11WPACipher,

	//MIB, dot118021xAuthEntry
	VAR_dot118021xAlgrthm,

	//MIB, dot11DefaultKeysTable
	VAR_dot11DefaultKeysTable,

	//MIB, dot11GroupKeysTable
	VAR_dot11Privacy,
	VAR_keyInCam,
	VAR_keyid,
	VAR_dot11EncryptKey,
	VAR_dot11EncryptKey2,
	
	//MIB, Dot11OperationEntry
	VAR_hwaddr,
	VAR_opmode,
	VAR_dot11RTSThreshold,
	VAR_dot11FragmentationThreshold,
	VAR_dot11ShortRetryLimit,
	VAR_dot11LongRetryLimit,
	VAR_block_relay,
	VAR_disable_txsc,
	VAR_disable_amsdu_txsc,
	VAR_guest_access,
	
	//MIB, dot11RFEntry
	VAR_MIMO_TR_mode,
	VAR_phyBandSelect,
	VAR_txpwr_reduction,
	VAR_bcnagc,
	VAR_shortpreamble,
#if (BEAMFORMING_SUPPORT == 1)		
	VAR_txbf,
	VAR_txbf_mu,
#endif
	//MIB, dot11Bss
	VAR_bssid, 
	
	//MIB, dot11BssType
	VAR_net_work_type,

	//MIB, dot11ErpInfo
	VAR_protection, 
	VAR_ctsToSelf,
	VAR_longPreambleStaNum,

	//MIB, ethBrExtInfo
	VAR_nat25_disable,
	VAR_macclone_enable,
	VAR_dhcp_bcst_disable,
	VAR_addPPPoETag,
	VAR_nat25sc_disable,
	VAR_macclone_method,
	VAR_macclone_eth_method,

	//MIB, miscEntry
#if defined(MBSSID)
	VAR_vap_enable,
#endif
	VAR_a4_enable,
	VAR_func_off,
	VAR_raku_only,
	VAR_update_rxbd_th,
	VAR_manual_priority,

	//MIB, dot11QosEntry
	VAR_dot11QosEnable,
	VAR_dot11QosAPSD,
	VAR_ManualEDCA,

	//MIB, wscEntry
	VAR_wsc_enable,

	//MIB, dot11nConfigEntry
	VAR_dot11nShortGIfor20M,
	VAR_dot11nShortGIfor40M,
	VAR_dot11nShortGIfor80M,
	VAR_dot11nShortGIfor160M,
	VAR_dot11nAMSDURecvMax,
	VAR_dot11nLgyEncRstrct,
	VAR_dot11nTxNoAck,

	//MIB, reorderCtrlEntry
	VAR_ReorderCtrlEnable,
	VAR_ReorderCtrlWinSz,
	VAR_ReorderCtrlTimeout,
	VAR_ReorderCtrlTimeoutCli,

	//PSHARE
	VAR_CurrentChannelBW,
	VAR_intel_rty_lmt,
	VAR_is_40m_bw,
	VAR_offset_2nd_chan,
	VAR_Reg_RRSR_2,
	VAR_Reg_81b,
	VAR_skip_mic_chk,
	VAR_curr_band,
	VAR_txsc_20,
	VAR_txsc_40,
	VAR_txsc_80,
#ifdef SW_TX_QUEUE
	VAR_swq_en,
#endif

	//PHW
	VAR_TXPowerOffset,
	VAR_MIMO_TR_hw_support,
	
	//RF, 
	VAR_rts_init_rate,
	VAR_bcn_pwr_max,
	VAR_bcn_pwr_idex,
	VAR_tx_pwr_ctrl,
	VAR_mc2u_disable,
	VAR_mc2u_drop_unknown,
	VAR_mc2u_flood_ctrl,
	VAR_drop_multicast,
	VAR_lmtdesc,
	VAR_lmt1,
	VAR_lmt2,
	VAR_lmt3,
	VAR_msta_refine,
	VAR_probersp_retry,
	VAR_diffAmpduSz,
	VAR_wifi_beq_iot,
	VAR_bcast_to_dzq,
	VAR_no_rtscts,
	VAR_cca_rts,
	VAR_txforce,
	VAR_rts_iot_th,
	VAR_sgiforce,
	VAR_aggforce,
#ifdef SW_TX_QUEUE
	VAR_swq_udelay1,
	VAR_swq_udelay2,
	VAR_swq_dbg,
	VAR_swq_aggnum,
	VAR_swqturboaggnum,
	VAR_swqmaxturbotime,
	VAR_swq_max_xmit,
	VAR_timeout_thd,
	VAR_timeout_thd2,
	VAR_timeout_thd3,
	VAR_tri_time1,
	VAR_tri_time2,
	VAR_tri_time3,
	VAR_tri_time4,
	VAR_tri_time5,
	VAR_tri_time6,
	VAR_tri_time7,
	VAR_tri_time8,
	VAR_udp_tri_time1,
	VAR_udp_tri_time2,
	VAR_udp_tri_time3,
	VAR_udp_tri_time4,
	VAR_udp_tri_time5,
	VAR_udp_tri_time6,
	VAR_udp_tri_time7,
	VAR_udp_tri_time8,
	VAR_swq_max_enqueue_len,
#endif	
#if (MU_BEAMFORMING_SUPPORT == 1)
	VAR_qtime,
	VAR_qlmt,
	VAR_dqnum,
	VAR_mudqnum,
	VAR_mu_bdfull_cnt,
	VAR_mu_swqempty_break,
	VAR_mu_group_mode,
	VAR_mutp_th_up,
	VAR_mutp_th_lower,
	VAR_beamformee_mu_cnt,
#endif
	VAR_t2h_sync,
	
	VAR_MAX,
} IPC_VAR_NUM;

typedef enum {
	//PRIV
	
	//Net Stat
	VAR_rx_packets,
	VAR_tx_packets,	
	VAR_rx_bytes,
	VAR_tx_bytes,

	//EXT Stat
	VAR_tx_byte_cnt,
	VAR_rx_byte_cnt,
	VAR_tx_drops,
	VAR_tx_drops_noasoc,
	VAR_rx_retrys,
	VAR_rx_decache,
	VAR_rx_data_drops,
	VAR_rx_mc_pn_drops,
#ifdef SW_TX_QUEUE
	VAR_swq_enque_pkt,
	VAR_swq_xmit_out_pkt,
	VAR_swq_real_enque_pkt,
	VAR_swq_real_deque_pkt,
	VAR_swq_residual_drop_pkt,
	VAR_swq_overflow_drop_pkt,
#endif	
	//PHW
#ifdef WIFI_WMM		
	VAR_VO_pkt_count,
	VAR_VI_pkt_count,
	VAR_VI_rx_pkt_count,
	VAR_BE_pkt_count,
	VAR_BK_pkt_count,
	VAR_VI_droppkt_count,
	VAR_VO_droppkt_count,
	VAR_BE_droppkt_count,
	VAR_BK_droppkt_count,
#endif
	
	VAR_T2H_MAX,
} IPC_T2H_VAR_NUM;

/*
*	var type
*/
typedef enum {
		VAR_PRIV = 0,
        VAR_MIB,
		VAR_PSHARE,
		VAR_PHW,
		VAR_RF,
		VAR_NETSTATS,
		VAR_EXTSTATS,
} IPC_VAR_TYPE;

struct var_cb{
	s32 vap_id;
	u16 num;
	u16 len;
	u8 data[MAX_VAR_LEN];
};


struct var_arg{
	char name[32];
	unsigned int num;
	unsigned int type;
	unsigned int len;
	unsigned int offset;
};

/*
*	stainfo num
*/
typedef enum {
	STAINFO_auth_seq,
	STAINFO_tpcache_mgt,
	STAINFO_cmn_info,
#ifdef WIFI_WMM	
	STAINFO_QosEnabled,
	STAINFO_apsd_bitmap,
	STAINFO_AC_seq,
#endif
	STAINFO_state,
	STAINFO_AuthAlgrthm,
	STAINFO_ieee8021x_ctrlport,
	STAINFO_bssrateset,
	STAINFO_useShortPreamble,
	STAINFO_expire_to,
	STAINFO_is_realtek_sta,
	STAINFO_IOTPeer,
	STAINFO_no_rts,
	STAINFO_tx_bw,
#if defined(RTK_AC_SUPPORT) || defined(RTK_AC_TX_SUPPORT)	
	STAINFO_vht_cap_buf,
	STAINFO_vht_cap_len,
	STAINFO_vht_oper_buf,
	STAINFO_vht_oper_len,
#endif
	STAINFO_nss,
	STAINFO_ht_cap_buf,
	STAINFO_ht_cap_len,
	STAINFO_ht_ie_buf,
	STAINFO_ht_ie_len,
	STAINFO_MIMO_ps,
	STAINFO_amsdu_level,
#if (BEAMFORMING_SUPPORT == 1)
	STAINFO_is_beamforming_entry,
	STAINFO_is_sounding_en,
#if (MU_BEAMFORMING_SUPPORT == 1)
	STAINFO_muPartner_num,
	STAINFO_mu_deq_num,
	STAINFO_muPartner_aid,
#endif
#endif
	STAINFO_is_legacy_encrpt,
#ifdef CONFIG_RTL_OFFLOAD_DRIVER	
	STAINFO_txie,
	STAINFO_sta_in_dc,
#endif
	STAINFO_frag_to,
	STAINFO_frag_count,
	//STAINFO_tpcache,
	STAINFO_prepare_to_free,
	STAINFO_keyid,
	STAINFO_dot11Privacy,
	STAINFO_keyInCam,
	STAINFO_dot11KeyMapping,
	STAINFO_bssratelen,
	STAINFO_hp_level,
	STAINFO_useCts2self,
	STAINFO_aggre_mthd,
	STAINFO_tx_avarage,
	STAINFO_rx_avarage,
	STAINFO_ADDBA_ready,
	STAINFO_ADDBA_req_num,
	STAINFO_ADDBA_sent,
	STAINFO_AMSDU_AMPDU_support,
	STAINFO_maxAggNum,
	STAINFO_retry_inc,
	STAINFO_wireless_mode,
	STAINFO_low_tp_disable_ampdu,
	STAINFO_uk_timeout,
	STAINFO_diffAmpduSz,
	STAINFO_ipmc_num,
	STAINFO_ip_mcast_info,
	STAINFO_ip_mcast_info2,
	STAINFO_H2T_MAX,
} IPC_STAINFO_H2T_NUM;

typedef enum {
	STAINFO_T2H_rssi,
	STAINFO_T2H_sq,
	STAINFO_T2H_rx_rate,
	STAINFO_T2H_rx_bw,
	STAINFO_T2H_rx_splcp,
	STAINFO_T2H_rf_info,
	STAINFO_T2H_highest_rx_rate,	
	STAINFO_T2H_current_tx_rate,
	
	STAINFO_T2H_tx_bytes,
	STAINFO_T2H_tx_pkts,
	STAINFO_T2H_tx_pkts2,
	STAINFO_T2H_tx_big_pkts,
	STAINFO_T2H_tx_sml_pkts,
	STAINFO_T2H_tx_byte_cnt,
	STAINFO_T2H_rx_bytes,
	STAINFO_T2H_rx_pkts,
	STAINFO_T2H_rx_pkts2,
	STAINFO_T2H_rx_mgnt_pkts,		
	STAINFO_T2H_rx_byte_cnt,
	STAINFO_T2H_tcpack_pkts,
#ifdef _RX_REORDER_DEBUG_	
	STAINFO_T2H_rx_rc4_count,
	STAINFO_T2H_rx_rc_timer_ovf,
	STAINFO_T2H_rx_rc_drop1,
	STAINFO_T2H_rx_rc_drop3,
	STAINFO_T2H_rx_rc_drop4,
	STAINFO_T2H_rx_rc_reorder3,
	STAINFO_T2H_rx_rc_reorder4,
	STAINFO_T2H_rx_rc_passup2,
	STAINFO_T2H_rx_rc_passup3,
	STAINFO_T2H_rx_rc_passup4,
	STAINFO_T2H_rx_rc_passupi,
#endif
	//STAINFO_T2H_swq,
	
	STAINFO_T2H_MAX,
} IPC_STAINFO_T2H_NUM;

/*
*	stainfo type
*/
typedef enum {
    STAINFO_PARAM = 0,
	STAINFO_STRUC,
} IPC_STAINFO_TYPE;

struct stainfo_cb{
	char mac[6];
	s32 vap_id;
	u16 num;
	u16 len;
	u8 data[MAX_VAR_LEN];
};

#define AID_SIZE 1
#define RATE_SIZE 1
#define RXRATE_SIZE 1
#define RXBW_SIZE 1
#define RXSPLCP_SIZE 1
#define PHY_INFO_SIZE 54
#define DRIVER_INFO_SIZE 32
#if 0
#define NUM_RXINFO_PER_REPORT 10
#endif
#define RXINFO_SIZE_PER_PACKET (AID_SIZE + RATE_SIZE + RXRATE_SIZE + RXBW_SIZE + RXSPLCP_SIZE + PHY_INFO_SIZE + DRIVER_INFO_SIZE) 


#define RX_INFO_ARRAY_SIZE	1024
#define RX_INFO_REPORT	(RX_INFO_ARRAY_SIZE/64)

struct rx_info_idx{
	unsigned short rx_info_rdx;
	unsigned short rx_info_wdx;
};

struct rx_info_cb{
	unsigned short rx_info_rdx;
	unsigned short rx_info_wdx;
	unsigned char rx_info_buf[RX_INFO_ARRAY_SIZE][RXINFO_SIZE_PER_PACKET];
};

#define PE_READY BIT(0)
#define PE_STARTED BIT(1)
#define PE_STOPED BIT(2)


struct pe_controlblock{
	unsigned char magic_number;
	unsigned char reserved[3];
	unsigned char pe_state;
	unsigned char pe_wifi_state;
	unsigned char reserved1[2];
	unsigned int  up_time;
	//Support 2007 STAs
	unsigned char sta_state[256];
	unsigned char sta_hw_queue_state[256];
	unsigned char sta_sw_queue_state[256];
	struct rx_info_cb rx_info; 
};


struct pe_controlblock* get_pe_cb_base(void);


#if defined(CONFIG_PE_ENABLE)
int pe_var_initialize(struct rtl8192cd_priv *priv);
int pe_var_set(struct rtl8192cd_priv *priv, unsigned int index);
int pe_stainfo_add(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int pe_stainfo_del(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#if defined(PE_INFO_SYNC)
int pe_stainfo_update(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int pe_stainfo_set(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned int index);
int pe_stainfo_sync(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int pe_priv_param_sync(struct rtl8192cd_priv *priv);
int pe_stainfo_aggre_update(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int pe_stainfo_mc2u_update(struct rtl8192cd_priv *priv, struct stat_info *pstat);
int pe_stainfo_ba_update(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#if (BEAMFORMING_SUPPORT == 1)
int pe_stainfo_beamforming_update(struct rtl8192cd_priv *priv, struct stat_info *pstat);
#endif
#endif
void pe_translate_rssi_sq_outsrc(struct stat_info *pstat, struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, char rate);
unsigned char pe_sta_state_get(unsigned short aid);
unsigned char pe_sta_hw_queue_state_get(unsigned short aid);
unsigned char pe_sta_sw_queue_state_get(unsigned short aid);
unsigned char check_pe_ready(void);
unsigned char check_pe_started(void);
unsigned char check_pe_stoped(void);
void clear_pe_status(void);
#elif defined(TAROKO_0)
int pe_var_set(struct rtl8192cd_priv *priv, unsigned char* buf);
int pe_stainfo_set(struct stat_info *pstat, unsigned char* buf);
#if defined(PE_RX_INFO_REPORT)
void pe_collect_rx_info(struct rx_frinfo *pfrinfo, char aid, char rate);
#endif
void pe_sta_state_set(unsigned short aid, unsigned char state);
void pe_sta_hw_queue_state_set(unsigned short aid, unsigned char state);
void pe_sta_sw_queue_state_set(unsigned short aid, unsigned char state);
void set_pe_ready(void);
void set_pe_started(void);
void set_pe_stoped(void);
#endif

#endif 

