
#include <8192cd.h>
#include <8192cd_cfg.h>

#include <core_sync.h>

#if defined(TAROKO_0)
static struct pe_controlblock *pe_cb = 0xA6141000; 

struct pe_controlblock* get_pe_cb_base(void)
{
	return pe_cb;
} 
#endif

#if 0
unsigned char rxinfo_buf[NUM_RXINFO_PER_REPORT * RXINFO_SIZE_PER_PACKET];
unsigned short num_of_rxinfo = 0;
#endif

#define _OFFSET(field)	((int)(long *)&(((struct wifi_mib *)0)->field))
#define _SIZE(field)	sizeof(((struct wifi_mib *)0)->field)

#define _OFFSET_priv(field)	((int)(long *)&(((struct rtl8192cd_priv *)0)->field))
#define _SIZE_priv(field)	sizeof(((struct rtl8192cd_priv *)0)->field)

#define _OFFSET_pshared(field)	((int)(long *)&(((struct priv_shared_info *)0)->field))
#define _SIZE_pshared(field)	sizeof(((struct priv_shared_info *)0)->field)

#define _OFFSET_phw(field)	((int)(long *)&(((struct rtl8192cd_hw *)0)->field))
#define _SIZE_phw(field)	sizeof(((struct rtl8192cd_hw *)0)->field)

#define _OFFSET_rf(field)	((int)(long *)&(((struct rf_finetune_var *)0)->field))
#define _SIZE_rf(field)		sizeof(((struct rf_finetune_var *)0)->field)

#define _OFFSET_netstats(field)	((int)(long *)&(((struct net_device_stats *)0)->field))
#define _SIZE_netstats(field)		sizeof(((struct net_device_stats *)0)->field)

#define _OFFSET_extstats(field)	((int)(long *)&(((struct extra_stats *)0)->field))
#define _SIZE_extstats(field)		sizeof(((struct extra_stats *)0)->field)

#define _OFFSET_stainfo(field)	((int)(long *)&(((struct stat_info *)0)->field))
#define _SIZE_stainfo(field)	sizeof(((struct stat_info *)0)->field)
#define _SIZE_STRUC_stainfo(field)	sizeof(struct field)

struct var_arg var_list[]=
{
	//PRIV
	{"", VAR_if_id, VAR_PRIV, _SIZE_priv(if_id), _OFFSET_priv(if_id)},
#if defined(RTK_BR_EXT)
	{"", VAR_br_mac, VAR_PRIV, _SIZE_priv(br_mac), _OFFSET_priv(br_mac)},
#endif
	
	//MIB, dot11StationConfigEntry
	{"", VAR_dot11Bssid, VAR_MIB, _SIZE(dot11StationConfigEntry.dot11Bssid), _OFFSET(dot11StationConfigEntry.dot11Bssid)},
	{"fixrate", VAR_fixedTxRate, VAR_MIB, _SIZE(dot11StationConfigEntry.fixedTxRate), _OFFSET(dot11StationConfigEntry.fixedTxRate)},
	{"autorate", VAR_autoRate, VAR_MIB, _SIZE(dot11StationConfigEntry.autoRate), _OFFSET(dot11StationConfigEntry.autoRate)},
	{"lowestMlcstRate", VAR_lowestMlcstRate, VAR_MIB, _SIZE(dot11StationConfigEntry.lowestMlcstRate), _OFFSET(dot11StationConfigEntry.lowestMlcstRate)},
	{"swTkipMic", VAR_swTkipMic, VAR_MIB, _SIZE(dot11StationConfigEntry.swTkipMic), _OFFSET(dot11StationConfigEntry.swTkipMic)},
	{"swcrypto", VAR_dot11swcrypto, VAR_MIB, _SIZE(dot11StationConfigEntry.dot11swcrypto), _OFFSET(dot11StationConfigEntry.dot11swcrypto)},
	{"", VAR_dot11OperationalRateSet, VAR_MIB, _SIZE(dot11StationConfigEntry.dot11OperationalRateSet), _OFFSET(dot11StationConfigEntry.dot11OperationalRateSet)},
	{"", VAR_dot11OperationalRateSetLen, VAR_MIB, _SIZE(dot11StationConfigEntry.dot11OperationalRateSetLen), _OFFSET(dot11StationConfigEntry.dot11OperationalRateSetLen)},
	//MIB, dot1180211AuthEntry
	{"encmode", VAR_dot11PrivacyAlgrthm, VAR_MIB, _SIZE(dot1180211AuthEntry.dot11PrivacyAlgrthm), _OFFSET(dot1180211AuthEntry.dot11PrivacyAlgrthm)},
	{"psk_enable", VAR_dot11EnablePSK, VAR_MIB, _SIZE(dot1180211AuthEntry.dot11EnablePSK), _OFFSET(dot1180211AuthEntry.dot11EnablePSK)},
	{"wpa_cipher", VAR_dot11WPACipher, VAR_MIB, _SIZE(dot1180211AuthEntry.dot11WPACipher), _OFFSET(dot1180211AuthEntry.dot11WPACipher)},
	//MIB, dot118021xAuthEntry
	{"802_1x", VAR_dot118021xAlgrthm, VAR_MIB, _SIZE(dot118021xAuthEntry.dot118021xAlgrthm), _OFFSET(dot118021xAuthEntry.dot118021xAlgrthm)},
	//MIB, dot11DefaultKeysTable
	{"", VAR_dot11DefaultKeysTable, VAR_MIB, _SIZE_STRUC_stainfo(Dot11DefaultKeysTable), _OFFSET(dot11DefaultKeysTable)},
	//MIB, dot11GroupKeysTable
	{"", VAR_dot11Privacy, VAR_MIB, _SIZE(dot11GroupKeysTable.dot11Privacy), _OFFSET(dot11GroupKeysTable.dot11Privacy)},
	{"", VAR_keyInCam, VAR_MIB, _SIZE(dot11GroupKeysTable.keyInCam), _OFFSET(dot11GroupKeysTable.keyInCam)},
	{"", VAR_keyid, VAR_MIB, _SIZE(dot11GroupKeysTable.keyid), _OFFSET(dot11GroupKeysTable.keyid)},
	{"", VAR_dot11EncryptKey, VAR_MIB, _SIZE_STRUC_stainfo(Dot11EncryptKey), _OFFSET(dot11GroupKeysTable.dot11EncryptKey)},
	{"", VAR_dot11EncryptKey2, VAR_MIB, _SIZE_STRUC_stainfo(Dot11EncryptKey), _OFFSET(dot11GroupKeysTable.dot11EncryptKey2)},
	//MIB, dot11OperationEntry
	{"", VAR_hwaddr, VAR_MIB, _SIZE(dot11OperationEntry.hwaddr), _OFFSET(dot11OperationEntry.hwaddr)},
	{"", VAR_opmode, VAR_MIB, _SIZE(dot11OperationEntry.opmode), _OFFSET(dot11OperationEntry.opmode)},
	{"rtsthres", VAR_dot11RTSThreshold, VAR_MIB, _SIZE(dot11OperationEntry.dot11RTSThreshold), _OFFSET(dot11OperationEntry.dot11RTSThreshold)},
	{"fragthres", VAR_dot11FragmentationThreshold, VAR_MIB, _SIZE(dot11OperationEntry.dot11FragmentationThreshold), _OFFSET(dot11OperationEntry.dot11FragmentationThreshold)},
	{"shortretry", VAR_dot11ShortRetryLimit, VAR_MIB, _SIZE(dot11OperationEntry.dot11ShortRetryLimit), _OFFSET(dot11OperationEntry.dot11ShortRetryLimit)},
	{"longretry", VAR_dot11LongRetryLimit, VAR_MIB, _SIZE(dot11OperationEntry.dot11LongRetryLimit), _OFFSET(dot11OperationEntry.dot11LongRetryLimit)},
	{"block_relay", VAR_block_relay, VAR_MIB, _SIZE(dot11OperationEntry.block_relay), _OFFSET(dot11OperationEntry.block_relay)},
	{"disable_txsc", VAR_disable_txsc, VAR_MIB, _SIZE(dot11OperationEntry.disable_txsc), _OFFSET(dot11OperationEntry.disable_txsc)},
	{"disable_amsdu_txsc", VAR_disable_amsdu_txsc, VAR_MIB, _SIZE(dot11OperationEntry.disable_amsdu_txsc), _OFFSET(dot11OperationEntry.disable_amsdu_txsc)},
	{"guest_access", VAR_guest_access, VAR_MIB, _SIZE(dot11OperationEntry.guest_access), _OFFSET(dot11OperationEntry.guest_access)},
	//MIB, dot11RFEntry
	{"MIMO_TR_mode", VAR_MIMO_TR_mode, VAR_MIB, _SIZE(dot11RFEntry.MIMO_TR_mode), _OFFSET(dot11RFEntry.MIMO_TR_mode)},
	{"phyBandSelect", VAR_phyBandSelect, VAR_MIB, _SIZE(dot11RFEntry.phyBandSelect), _OFFSET(dot11RFEntry.phyBandSelect)},
	{"txpwr_reduction", VAR_txpwr_reduction, VAR_MIB, _SIZE(dot11RFEntry.txpwr_reduction), _OFFSET(dot11RFEntry.txpwr_reduction)},
	{"bcnagc", VAR_bcnagc, VAR_MIB, _SIZE(dot11RFEntry.bcnagc), _OFFSET(dot11RFEntry.bcnagc)},
	{"preamble", VAR_shortpreamble, VAR_MIB, _SIZE(dot11RFEntry.shortpreamble), _OFFSET(dot11RFEntry.shortpreamble)},
#if (BEAMFORMING_SUPPORT == 1)	
	{"txbf", VAR_txbf, VAR_MIB, _SIZE(dot11RFEntry.txbf), _OFFSET(dot11RFEntry.txbf)},
	{"txbf_mu", VAR_txbf_mu, VAR_MIB, _SIZE(dot11RFEntry.txbf_mu), _OFFSET(dot11RFEntry.txbf_mu)},
#endif	
	//MIB, dot11Bss
	{"", VAR_bssid, VAR_MIB, _SIZE(dot11Bss.bssid), _OFFSET(dot11Bss.bssid)},
	//MIB, dot11BssType
	{"band", VAR_net_work_type, VAR_MIB, _SIZE(dot11BssType.net_work_type), _OFFSET(dot11BssType.net_work_type)},
	//MIB, dot11ErpInfo
	{"", VAR_protection, VAR_MIB, _SIZE(dot11ErpInfo.protection), _OFFSET(dot11ErpInfo.protection)},
	{"cts2self", VAR_ctsToSelf, VAR_MIB, _SIZE(dot11ErpInfo.ctsToSelf), _OFFSET(dot11ErpInfo.ctsToSelf)},
	{"", VAR_longPreambleStaNum, VAR_MIB, _SIZE(dot11ErpInfo.longPreambleStaNum), _OFFSET(dot11ErpInfo.longPreambleStaNum)},
	//MIB, ethBrExtInfo
	{"nat25_disable", VAR_nat25_disable, VAR_MIB, _SIZE(ethBrExtInfo.nat25_disable), _OFFSET(ethBrExtInfo.nat25_disable)},
	{"macclone_enable", VAR_macclone_enable, VAR_MIB, _SIZE(ethBrExtInfo.macclone_enable), _OFFSET(ethBrExtInfo.macclone_enable)},
	{"dhcp_bcst_disable", VAR_dhcp_bcst_disable, VAR_MIB, _SIZE(ethBrExtInfo.dhcp_bcst_disable), _OFFSET(ethBrExtInfo.dhcp_bcst_disable)},
	{"add_pppoe_tag", VAR_addPPPoETag, VAR_MIB, _SIZE(ethBrExtInfo.addPPPoETag), _OFFSET(ethBrExtInfo.addPPPoETag)},
	{"nat25sc_disable", VAR_nat25sc_disable, VAR_MIB, _SIZE(ethBrExtInfo.nat25sc_disable), _OFFSET(ethBrExtInfo.nat25sc_disable)},
	{"macclone_method", VAR_macclone_method, VAR_MIB, _SIZE(ethBrExtInfo.macclone_method), _OFFSET(ethBrExtInfo.macclone_method)},
	{"macclone_eth_method", VAR_macclone_eth_method, VAR_MIB, _SIZE(ethBrExtInfo.macclone_eth_method), _OFFSET(ethBrExtInfo.macclone_eth_method)},
	//MIB, miscEntry
#if defined(MBSSID)
	{"vap_enable", VAR_vap_enable, VAR_MIB, _SIZE(miscEntry.vap_enable), _OFFSET(miscEntry.vap_enable)},
#endif
	{"a4_enable", VAR_a4_enable, VAR_MIB, _SIZE(miscEntry.a4_enable), _OFFSET(miscEntry.a4_enable)},
	
	{"func_off", VAR_func_off, VAR_MIB, _SIZE(miscEntry.func_off), _OFFSET(miscEntry.func_off)},
	{"raku_only", VAR_raku_only, VAR_MIB, _SIZE(miscEntry.raku_only), _OFFSET(miscEntry.raku_only)},
	{"update_rxbd_th", VAR_update_rxbd_th, VAR_MIB, _SIZE(miscEntry.update_rxbd_th), _OFFSET(miscEntry.update_rxbd_th)},
	{"manual_priority", VAR_manual_priority, VAR_MIB, _SIZE(miscEntry.manual_priority), _OFFSET(miscEntry.manual_priority)},
	//MIB, dot11QosEntry
	{"qos_enable", VAR_dot11QosEnable, VAR_MIB, _SIZE(dot11QosEntry.dot11QosEnable), _OFFSET(dot11QosEntry.dot11QosEnable)},
	{"apsd_enable", VAR_dot11QosAPSD, VAR_MIB, _SIZE(dot11QosEntry.dot11QosAPSD), _OFFSET(dot11QosEntry.dot11QosAPSD)},
	{"manual_edca", VAR_ManualEDCA, VAR_MIB, _SIZE(dot11QosEntry.ManualEDCA), _OFFSET(dot11QosEntry.ManualEDCA)},
	//MIB, wscEntry
	{"wsc_enable", VAR_wsc_enable, VAR_MIB, _SIZE(wscEntry.wsc_enable), _OFFSET(wscEntry.wsc_enable)},
	//MIB, dot11nConfigEntry
	{"shortGI20M", VAR_dot11nShortGIfor20M, VAR_MIB, _SIZE(dot11nConfigEntry.dot11nShortGIfor20M), _OFFSET(dot11nConfigEntry.dot11nShortGIfor20M)},
	{"shortGI40M", VAR_dot11nShortGIfor40M, VAR_MIB, _SIZE(dot11nConfigEntry.dot11nShortGIfor40M), _OFFSET(dot11nConfigEntry.dot11nShortGIfor40M)},
	{"shortGI80M", VAR_dot11nShortGIfor80M, VAR_MIB, _SIZE(dot11nConfigEntry.dot11nShortGIfor80M), _OFFSET(dot11nConfigEntry.dot11nShortGIfor80M)},
	{"shortGI160M", VAR_dot11nShortGIfor160M, VAR_MIB, _SIZE(dot11nConfigEntry.dot11nShortGIfor160M), _OFFSET(dot11nConfigEntry.dot11nShortGIfor160M)},
	{"amsduMax", VAR_dot11nAMSDURecvMax, VAR_MIB, _SIZE(dot11nConfigEntry.dot11nAMSDURecvMax), _OFFSET(dot11nConfigEntry.dot11nAMSDURecvMax)},
	{"lgyEncRstrct", VAR_dot11nLgyEncRstrct, VAR_MIB, _SIZE(dot11nConfigEntry.dot11nLgyEncRstrct), _OFFSET(dot11nConfigEntry.dot11nLgyEncRstrct)},
	{"txnoack", VAR_dot11nTxNoAck, VAR_MIB, _SIZE(dot11nConfigEntry.dot11nTxNoAck), _OFFSET(dot11nConfigEntry.dot11nTxNoAck)},
	//MIB, reorderCtrlEntry
	{"rc_enable", VAR_ReorderCtrlEnable, VAR_MIB, _SIZE(reorderCtrlEntry.ReorderCtrlEnable), _OFFSET(reorderCtrlEntry.ReorderCtrlEnable)},
	{"rc_winsz", VAR_ReorderCtrlWinSz, VAR_MIB, _SIZE(reorderCtrlEntry.ReorderCtrlWinSz), _OFFSET(reorderCtrlEntry.ReorderCtrlWinSz)},
	{"rc_timeout", VAR_ReorderCtrlTimeout, VAR_MIB, _SIZE(reorderCtrlEntry.ReorderCtrlTimeout), _OFFSET(reorderCtrlEntry.ReorderCtrlTimeout)},
	{"rc_timeout_cli", VAR_ReorderCtrlTimeoutCli, VAR_MIB, _SIZE(reorderCtrlEntry.ReorderCtrlTimeoutCli), _OFFSET(reorderCtrlEntry.ReorderCtrlTimeoutCli)},

	//PSHARE
	{"", VAR_CurrentChannelBW, VAR_PSHARE, _SIZE_pshared(CurrentChannelBW), _OFFSET_pshared(CurrentChannelBW)},
	{"", VAR_intel_rty_lmt, VAR_PSHARE, _SIZE_pshared(intel_rty_lmt), _OFFSET_pshared(intel_rty_lmt)},
	{"", VAR_is_40m_bw, VAR_PSHARE, _SIZE_pshared(is_40m_bw), _OFFSET_pshared(is_40m_bw)},
	{"", VAR_offset_2nd_chan, VAR_PSHARE, _SIZE_pshared(offset_2nd_chan), _OFFSET_pshared(offset_2nd_chan)},
	{"", VAR_Reg_RRSR_2, VAR_PSHARE, _SIZE_pshared(Reg_RRSR_2), _OFFSET_pshared(Reg_RRSR_2)},
	{"", VAR_Reg_81b, VAR_PSHARE, _SIZE_pshared(Reg_81b), _OFFSET_pshared(Reg_81b)},
	{"", VAR_skip_mic_chk, VAR_PSHARE, _SIZE_pshared(skip_mic_chk), _OFFSET_pshared(skip_mic_chk)},
	{"", VAR_curr_band, VAR_PSHARE, _SIZE_pshared(curr_band), _OFFSET_pshared(curr_band)},
	{"", VAR_txsc_20, VAR_PSHARE, _SIZE_pshared(txsc_20), _OFFSET_pshared(txsc_20)},
	{"", VAR_txsc_40, VAR_PSHARE, _SIZE_pshared(txsc_40), _OFFSET_pshared(txsc_40)},
	{"", VAR_txsc_80, VAR_PSHARE, _SIZE_pshared(txsc_80), _OFFSET_pshared(txsc_80)},
#ifdef SW_TX_QUEUE
	{"", VAR_swq_en, VAR_PSHARE, _SIZE_pshared(swq_en), _OFFSET_pshared(swq_en)},	
#endif

	//PHW
	{"", VAR_TXPowerOffset, VAR_PHW, _SIZE_phw(TXPowerOffset), _OFFSET_phw(TXPowerOffset)},
	{"", VAR_MIMO_TR_hw_support, VAR_PHW, _SIZE_phw(MIMO_TR_hw_support), _OFFSET_phw(MIMO_TR_hw_support)},

	//RF, rf_ft_var
	{"rts_init_rate", VAR_rts_init_rate, VAR_RF, _SIZE_rf(rts_init_rate), _OFFSET_rf(rts_init_rate)},
	{"bcn_pwr_max", VAR_bcn_pwr_max, VAR_RF, _SIZE_rf(bcn_pwr_max), _OFFSET_rf(bcn_pwr_max)},
	{"", VAR_bcn_pwr_idex, VAR_RF, _SIZE_rf(bcn_pwr_idex), _OFFSET_rf(bcn_pwr_idex)},
	{"tx_pwr_ctrl", VAR_tx_pwr_ctrl, VAR_RF, _SIZE_rf(tx_pwr_ctrl), _OFFSET_rf(tx_pwr_ctrl)},
	{"mc2u_disable", VAR_mc2u_disable, VAR_RF, _SIZE_rf(mc2u_disable), _OFFSET_rf(mc2u_disable)},
	{"mc2u_drop_unknown", VAR_mc2u_drop_unknown, VAR_RF, _SIZE_rf(mc2u_drop_unknown), _OFFSET_rf(mc2u_drop_unknown)},
	{"mc2u_flood_ctrl", VAR_mc2u_flood_ctrl, VAR_RF, _SIZE_rf(mc2u_flood_ctrl), _OFFSET_rf(mc2u_flood_ctrl)},
	{"drop_mcast", VAR_drop_multicast, VAR_RF, _SIZE_rf(drop_multicast), _OFFSET_rf(drop_multicast)},
	{"lmtdesc", VAR_lmtdesc, VAR_RF, _SIZE_rf(lmtdesc), _OFFSET_rf(lmtdesc)},
	{"lmt1", VAR_lmt1, VAR_RF, _SIZE_rf(lmt1), _OFFSET_rf(lmt1)},
	{"lmt2", VAR_lmt2, VAR_RF, _SIZE_rf(lmt2), _OFFSET_rf(lmt2)},
	{"lmt3", VAR_lmt3, VAR_RF, _SIZE_rf(lmt3), _OFFSET_rf(lmt3)},
	{"msta", VAR_msta_refine, VAR_RF, _SIZE_rf(msta_refine), _OFFSET_rf(msta_refine)},
	{"prsprty", VAR_probersp_retry, VAR_RF, _SIZE_rf(probersp_retry), _OFFSET_rf(probersp_retry)},
	{"diffAmpduSz", VAR_diffAmpduSz, VAR_RF, _SIZE_rf(diffAmpduSz), _OFFSET_rf(diffAmpduSz)},
	{"wifi_beq_iot", VAR_wifi_beq_iot, VAR_RF, _SIZE_rf(wifi_beq_iot), _OFFSET_rf(wifi_beq_iot)},
	{"bcast_to_dzq", VAR_bcast_to_dzq, VAR_RF, _SIZE_rf(bcast_to_dzq), _OFFSET_rf(bcast_to_dzq)},
	{"no_rtscts", VAR_no_rtscts, VAR_RF, _SIZE_rf(no_rtscts), _OFFSET_rf(no_rtscts)},
	{"cca_rts", VAR_cca_rts, VAR_RF, _SIZE_rf(cca_rts), _OFFSET_rf(cca_rts)},
	{"txforce", VAR_txforce, VAR_RF, _SIZE_rf(txforce), _OFFSET_rf(txforce)},
	{"rts_iot_th", VAR_rts_iot_th, VAR_RF, _SIZE_rf(rts_iot_th), _OFFSET_rf(rts_iot_th)},
	{"sgiforce", VAR_sgiforce, VAR_RF, _SIZE_rf(sgiforce), _OFFSET_rf(sgiforce)},
	{"aggforce", VAR_aggforce, VAR_RF, _SIZE_rf(aggforce), _OFFSET_rf(aggforce)},
#ifdef SW_TX_QUEUE
	{"swq_udelay1", VAR_swq_udelay1, VAR_RF, _SIZE_rf(swq_udelay1), _OFFSET_rf(swq_udelay1)},
	{"swq_udelay2", VAR_swq_udelay2, VAR_RF, _SIZE_rf(swq_udelay2), _OFFSET_rf(swq_udelay2)},
	{"swq_dbg", VAR_swq_dbg, VAR_RF, _SIZE_rf(swq_dbg), _OFFSET_rf(swq_dbg)},	
	{"swq_aggnum", VAR_swq_aggnum, VAR_RF, _SIZE_rf(swq_aggnum), _OFFSET_rf(swq_aggnum)},
	{"swqturboaggnum", VAR_swqturboaggnum, VAR_RF, _SIZE_rf(manual_swqturboaggnum), _OFFSET_rf(manual_swqturboaggnum)},
	{"swqmaxturbotime", VAR_swqmaxturbotime, VAR_RF, _SIZE_rf(swqmaxturbotime), _OFFSET_rf(swqmaxturbotime)},
	{"swq_max_xmit", VAR_swq_max_xmit, VAR_RF, _SIZE_rf(swq_max_xmit), _OFFSET_rf(swq_max_xmit)},
	{"timeout_thd", VAR_timeout_thd, VAR_RF, _SIZE_rf(timeout_thd), _OFFSET_rf(timeout_thd)},
	{"timeout_thd2", VAR_timeout_thd2, VAR_RF, _SIZE_rf(timeout_thd2), _OFFSET_rf(timeout_thd2)},
	{"timeout_thd3", VAR_timeout_thd3, VAR_RF, _SIZE_rf(timeout_thd3), _OFFSET_rf(timeout_thd3)},
	{"tri_time1", VAR_tri_time1, VAR_RF, _SIZE_rf(tri_time1), _OFFSET_rf(tri_time1)},
	{"tri_time2", VAR_tri_time2, VAR_RF, _SIZE_rf(tri_time2), _OFFSET_rf(tri_time2)},
	{"tri_time3", VAR_tri_time3, VAR_RF, _SIZE_rf(tri_time3), _OFFSET_rf(tri_time3)},
	{"tri_time4", VAR_tri_time4, VAR_RF, _SIZE_rf(tri_time4), _OFFSET_rf(tri_time4)},
	{"tri_time5", VAR_tri_time5, VAR_RF, _SIZE_rf(tri_time5), _OFFSET_rf(tri_time5)},
	{"tri_time6", VAR_tri_time6, VAR_RF, _SIZE_rf(tri_time6), _OFFSET_rf(tri_time6)},
	{"tri_time7", VAR_tri_time7, VAR_RF, _SIZE_rf(tri_time7), _OFFSET_rf(tri_time7)},
	{"tri_time8", VAR_tri_time8, VAR_RF, _SIZE_rf(tri_time8), _OFFSET_rf(tri_time8)},
	{"udp_tri_time1", VAR_udp_tri_time1, VAR_RF, _SIZE_rf(udp_tri_time1), _OFFSET_rf(udp_tri_time1)},
	{"udp_tri_time2", VAR_udp_tri_time2, VAR_RF, _SIZE_rf(udp_tri_time2), _OFFSET_rf(udp_tri_time2)},
	{"udp_tri_time3", VAR_udp_tri_time3, VAR_RF, _SIZE_rf(udp_tri_time3), _OFFSET_rf(udp_tri_time3)},
	{"udp_tri_time4", VAR_udp_tri_time4, VAR_RF, _SIZE_rf(udp_tri_time4), _OFFSET_rf(udp_tri_time4)},
	{"udp_tri_time5", VAR_udp_tri_time5, VAR_RF, _SIZE_rf(udp_tri_time5), _OFFSET_rf(udp_tri_time5)},
	{"udp_tri_time6", VAR_udp_tri_time6, VAR_RF, _SIZE_rf(udp_tri_time6), _OFFSET_rf(udp_tri_time6)},
	{"udp_tri_time7", VAR_udp_tri_time7, VAR_RF, _SIZE_rf(udp_tri_time7), _OFFSET_rf(udp_tri_time7)},
	{"udp_tri_time8", VAR_udp_tri_time8, VAR_RF, _SIZE_rf(udp_tri_time8), _OFFSET_rf(udp_tri_time8)},
	{"swq_max_enqueue_len", VAR_swq_max_enqueue_len, VAR_RF, _SIZE_rf(swq_max_enqueue_len), _OFFSET_rf(swq_max_enqueue_len)},	
#endif
#if (MU_BEAMFORMING_SUPPORT == 1)
	{"qtime", VAR_qtime, VAR_RF, _SIZE_rf(qtime), _OFFSET_rf(qtime)},
	{"qlmt", VAR_qlmt, VAR_RF, _SIZE_rf(qlmt), _OFFSET_rf(qlmt)},
	{"dqnum", VAR_dqnum, VAR_RF, _SIZE_rf(dqnum), _OFFSET_rf(dqnum)},	
	{"mudqnum", VAR_mudqnum, VAR_RF, _SIZE_rf(mudqnum), _OFFSET_rf(mudqnum)},
	{"mu_bdfull_cnt", VAR_mu_bdfull_cnt, VAR_RF, _SIZE_rf(mu_bdfull_cnt), _OFFSET_rf(mu_bdfull_cnt)},	
	{"mu_swqempty_break", VAR_mu_swqempty_break, VAR_RF, _SIZE_rf(mu_swqempty_break), _OFFSET_rf(mu_swqempty_break)},
	{"mu_group_mode", VAR_mu_group_mode, VAR_RF, _SIZE_rf(mu_group_mode), _OFFSET_rf(mu_group_mode)},	
	{"mutp_th_up", VAR_mutp_th_up, VAR_RF, _SIZE_rf(mutp_th_up), _OFFSET_rf(mutp_th_up)},
	{"mutp_th_lower", VAR_mutp_th_lower, VAR_RF, _SIZE_rf(mutp_th_lower), _OFFSET_rf(mutp_th_lower)},
	{"beamformee_mu_cnt", VAR_beamformee_mu_cnt, VAR_RF, _SIZE_rf(beamformee_mu_cnt), _OFFSET_rf(beamformee_mu_cnt)},		
#endif
	{"t2h_sync", VAR_t2h_sync, 0x0, 0x0, 0x0},//this is for Taroko notify A17 sync structure
};

struct var_arg var_t2h_list[]=
{
	//net state	
	{"rx_packets", VAR_rx_packets, VAR_NETSTATS, _SIZE_netstats(rx_packets), _OFFSET_netstats(rx_packets)},
	{"tx_packets", VAR_tx_packets, VAR_NETSTATS, _SIZE_netstats(tx_packets), _OFFSET_netstats(tx_packets)},
	{"rx_bytes", VAR_rx_bytes, VAR_NETSTATS, _SIZE_netstats(rx_bytes), _OFFSET_netstats(rx_bytes)},
	{"tx_bytes", VAR_tx_bytes, VAR_NETSTATS, _SIZE_netstats(tx_bytes), _OFFSET_netstats(tx_bytes)},
	
	//ext state
	{"tx_byte_cnt", VAR_tx_byte_cnt, VAR_EXTSTATS, _SIZE_extstats(tx_byte_cnt), _OFFSET_extstats(tx_byte_cnt)},
	{"rx_byte_cnt", VAR_rx_byte_cnt, VAR_EXTSTATS, _SIZE_extstats(rx_byte_cnt), _OFFSET_extstats(rx_byte_cnt)},
	{"tx_drops", VAR_tx_drops, VAR_EXTSTATS, _SIZE_extstats(tx_drops), _OFFSET_extstats(tx_drops)},
	{"tx_drops_noasoc", VAR_tx_drops_noasoc, VAR_EXTSTATS, _SIZE_extstats(tx_drops_noasoc), _OFFSET_extstats(tx_drops_noasoc)},
	{"rx_retrys", VAR_rx_retrys, VAR_EXTSTATS, _SIZE_extstats(rx_retrys), _OFFSET_extstats(rx_retrys)},
	{"rx_decache", VAR_rx_decache, VAR_EXTSTATS, _SIZE_extstats(rx_decache), _OFFSET_extstats(rx_decache)},
	{"rx_data_drops", VAR_rx_data_drops, VAR_EXTSTATS, _SIZE_extstats(rx_data_drops), _OFFSET_extstats(rx_data_drops)},
	{"rx_mc_pn_drops", VAR_rx_mc_pn_drops, VAR_EXTSTATS, _SIZE_extstats(rx_mc_pn_drops), _OFFSET_extstats(rx_mc_pn_drops)},
#ifdef SW_TX_QUEUE
	{"swq_enque_pkt", VAR_swq_enque_pkt, VAR_EXTSTATS, _SIZE_extstats(swq_enque_pkt), _OFFSET_extstats(swq_enque_pkt)},
	{"swq_xmit_out_pkt", VAR_swq_xmit_out_pkt, VAR_EXTSTATS, _SIZE_extstats(swq_xmit_out_pkt), _OFFSET_extstats(swq_xmit_out_pkt)},
	{"swq_real_enque_pkt", VAR_swq_real_enque_pkt, VAR_EXTSTATS, _SIZE_extstats(swq_real_enque_pkt), _OFFSET_extstats(swq_real_enque_pkt)},
	{"swq_real_deque_pkt", VAR_swq_real_deque_pkt, VAR_EXTSTATS, _SIZE_extstats(swq_real_deque_pkt), _OFFSET_extstats(swq_real_deque_pkt)},
	{"swq_residual_drop_pkt", VAR_swq_residual_drop_pkt, VAR_EXTSTATS, _SIZE_extstats(swq_residual_drop_pkt), _OFFSET_extstats(swq_residual_drop_pkt)},
	{"swq_overflow_drop_pkt", VAR_swq_overflow_drop_pkt, VAR_EXTSTATS, _SIZE_extstats(swq_overflow_drop_pkt), _OFFSET_extstats(swq_overflow_drop_pkt)},
#endif	
	
	//phw
#ifdef WIFI_WMM	
	{"VO_pkt_count", VAR_VO_pkt_count, VAR_PHW, _SIZE_phw(VO_pkt_count), _OFFSET_phw(VO_pkt_count)},
	{"VI_pkt_count", VAR_VI_pkt_count, VAR_PHW, _SIZE_phw(VI_pkt_count), _OFFSET_phw(VI_pkt_count)},
	{"VI_rx_pkt_count", VAR_VI_rx_pkt_count, VAR_PHW, _SIZE_phw(VI_rx_pkt_count), _OFFSET_phw(VI_rx_pkt_count)},
	{"BE_pkt_count", VAR_BE_pkt_count, VAR_PHW, _SIZE_phw(BE_pkt_count), _OFFSET_phw(BE_pkt_count)},
	{"BK_pkt_count", VAR_BK_pkt_count, VAR_PHW, _SIZE_phw(BK_pkt_count), _OFFSET_phw(BK_pkt_count)},
	{"VI_droppkt_count", VAR_VI_droppkt_count, VAR_PHW, _SIZE_phw(VI_droppkt_count), _OFFSET_phw(VI_droppkt_count)},
	{"VO_droppkt_count", VAR_VO_droppkt_count, VAR_PHW, _SIZE_phw(VO_droppkt_count), _OFFSET_phw(VO_droppkt_count)},
	{"BE_droppkt_count", VAR_BE_droppkt_count, VAR_PHW, _SIZE_phw(BE_droppkt_count), _OFFSET_phw(BE_droppkt_count)},
	{"BK_droppkt_count", VAR_BK_droppkt_count, VAR_PHW, _SIZE_phw(BK_droppkt_count), _OFFSET_phw(BK_droppkt_count)},
#endif
};

struct var_arg stainfo_h2t_list[]=
{
	{"auth_seq", STAINFO_auth_seq, STAINFO_PARAM, _SIZE_stainfo(auth_seq), _OFFSET_stainfo(auth_seq)},
	{"tpcache_mgt", STAINFO_tpcache_mgt, STAINFO_PARAM, _SIZE_stainfo(tpcache_mgt), _OFFSET_stainfo(tpcache_mgt)},
	{"", STAINFO_cmn_info, STAINFO_STRUC, _SIZE_STRUC_stainfo(cmn_sta_info), _OFFSET_stainfo(cmn_info)},
#ifdef WIFI_WMM
	{"QosEnabled", STAINFO_QosEnabled, STAINFO_PARAM, _SIZE_stainfo(QosEnabled), _OFFSET_stainfo(QosEnabled)},
	{"apsd_bitmap", STAINFO_apsd_bitmap, STAINFO_PARAM, _SIZE_stainfo(apsd_bitmap), _OFFSET_stainfo(apsd_bitmap)},
	{"", STAINFO_AC_seq, STAINFO_PARAM, _SIZE_stainfo(AC_seq), _OFFSET_stainfo(AC_seq)},
#endif
	{"state", STAINFO_state, STAINFO_PARAM, _SIZE_stainfo(state), _OFFSET_stainfo(state)},
	{"AuthAlgrthm", STAINFO_AuthAlgrthm, STAINFO_PARAM, _SIZE_stainfo(AuthAlgrthm), _OFFSET_stainfo(AuthAlgrthm)},
	{"ieee8021x_ctrlport", STAINFO_ieee8021x_ctrlport, STAINFO_PARAM, _SIZE_stainfo(ieee8021x_ctrlport), _OFFSET_stainfo(ieee8021x_ctrlport)},
	{"bssrateset", STAINFO_bssrateset, STAINFO_PARAM, _SIZE_stainfo(bssrateset), _OFFSET_stainfo(bssrateset)},
	{"useShortPreamble", STAINFO_useShortPreamble, STAINFO_PARAM, _SIZE_stainfo(useShortPreamble), _OFFSET_stainfo(useShortPreamble)},
	{"expire_to", STAINFO_expire_to, STAINFO_PARAM, _SIZE_stainfo(expire_to), _OFFSET_stainfo(expire_to)},
	{"is_realtek_sta", STAINFO_is_realtek_sta, STAINFO_PARAM, _SIZE_stainfo(is_realtek_sta), _OFFSET_stainfo(is_realtek_sta)},
	{"IOTPeer", STAINFO_IOTPeer, STAINFO_PARAM, _SIZE_stainfo(IOTPeer), _OFFSET_stainfo(IOTPeer)},
	{"no_rts", STAINFO_no_rts, STAINFO_PARAM, _SIZE_stainfo(no_rts), _OFFSET_stainfo(no_rts)},
	{"tx_bw", STAINFO_tx_bw, STAINFO_PARAM, _SIZE_stainfo(tx_bw), _OFFSET_stainfo(tx_bw)},
#if defined(RTK_AC_SUPPORT) || defined(RTK_AC_TX_SUPPORT)	
	{"", STAINFO_vht_cap_buf, STAINFO_STRUC, _SIZE_STRUC_stainfo(vht_cap_elmt), _OFFSET_stainfo(vht_cap_buf)},
	{"vht_cap_len", STAINFO_vht_cap_len, STAINFO_PARAM, _SIZE_stainfo(vht_cap_len), _OFFSET_stainfo(vht_cap_len)},
	{"", STAINFO_vht_oper_buf, STAINFO_STRUC, _SIZE_STRUC_stainfo(vht_oper_elmt), _OFFSET_stainfo(vht_oper_buf)},
	{"vht_oper_len", STAINFO_vht_oper_len, STAINFO_PARAM, _SIZE_stainfo(vht_oper_len), _OFFSET_stainfo(vht_oper_len)},
#endif	
	{"nss", STAINFO_nss, STAINFO_PARAM, _SIZE_stainfo(nss), _OFFSET_stainfo(nss)},
	{"", STAINFO_ht_cap_buf, STAINFO_STRUC, _SIZE_STRUC_stainfo(ht_cap_elmt), _OFFSET_stainfo(ht_cap_buf)},
	{"ht_cap_len", STAINFO_ht_cap_len, STAINFO_PARAM, _SIZE_stainfo(ht_cap_len), _OFFSET_stainfo(ht_cap_len)},
	{"", STAINFO_ht_ie_buf, STAINFO_STRUC, _SIZE_STRUC_stainfo(ht_info_elmt), _OFFSET_stainfo(ht_ie_buf)},
	{"ht_ie_len", STAINFO_ht_ie_len, STAINFO_PARAM, _SIZE_stainfo(ht_ie_len), _OFFSET_stainfo(ht_ie_len)},
	{"MIMO_ps", STAINFO_MIMO_ps, STAINFO_PARAM, _SIZE_stainfo(MIMO_ps), _OFFSET_stainfo(MIMO_ps)},
	{"amsdu_level", STAINFO_amsdu_level, STAINFO_PARAM, _SIZE_stainfo(amsdu_level), _OFFSET_stainfo(amsdu_level)},
#if (BEAMFORMING_SUPPORT == 1)
	{"is_beamforming_entry", STAINFO_is_beamforming_entry, STAINFO_PARAM, _SIZE_stainfo(is_beamforming_entry), _OFFSET_stainfo(is_beamforming_entry)},
	{"is_sounding_en", STAINFO_is_sounding_en, STAINFO_PARAM, _SIZE_stainfo(is_sounding_en), _OFFSET_stainfo(is_sounding_en)},
#if (MU_BEAMFORMING_SUPPORT == 1)
	{"muPartner_num", STAINFO_muPartner_num, STAINFO_PARAM, _SIZE_stainfo(muPartner_num), _OFFSET_stainfo(muPartner_num)},
	{"mu_deq_num", STAINFO_mu_deq_num, STAINFO_PARAM, _SIZE_stainfo(mu_deq_num), _OFFSET_stainfo(mu_deq_num)},	
	{"muPartner_aid", STAINFO_muPartner_aid, STAINFO_PARAM, _SIZE_stainfo(muPartner_aid), _OFFSET_stainfo(muPartner_aid)},		
#endif
#endif
	{"is_legacy_encrpt", STAINFO_is_legacy_encrpt, STAINFO_PARAM, _SIZE_stainfo(is_legacy_encrpt), _OFFSET_stainfo(is_legacy_encrpt)},
#ifdef CONFIG_RTL_OFFLOAD_DRIVER	
	{"", STAINFO_txie, STAINFO_PARAM, _SIZE_stainfo(txie), _OFFSET_stainfo(txie)},
	{"sta_in_dc", STAINFO_sta_in_dc, STAINFO_PARAM, _SIZE_stainfo(sta_in_dc), _OFFSET_stainfo(sta_in_dc)},
#endif
	{"frag_to", STAINFO_frag_to, STAINFO_PARAM, _SIZE_stainfo(frag_to), _OFFSET_stainfo(frag_to)},
	{"frag_count", STAINFO_frag_count, STAINFO_PARAM, _SIZE_stainfo(frag_count), _OFFSET_stainfo(frag_count)},
	//{"", STAINFO_tpcache, STAINFO_PARAM, _SIZE_stainfo(tpcache), _OFFSET_stainfo(tpcache)},
	{"prepare_to_free", STAINFO_prepare_to_free, STAINFO_PARAM, _SIZE_stainfo(prepare_to_free), _OFFSET_stainfo(prepare_to_free)},
	{"keyid", STAINFO_keyid, STAINFO_PARAM, _SIZE_stainfo(keyid), _OFFSET_stainfo(keyid)},
	//{"", STAINFO_dot11KeyMapping, STAINFO_STRUC, _SIZE_STRUC_stainfo(Dot11KeyMappingsEntry), _OFFSET_stainfo(dot11KeyMapping)},
	{"", STAINFO_dot11Privacy, STAINFO_PARAM, _SIZE_stainfo(dot11KeyMapping.dot11Privacy), _OFFSET_stainfo(dot11KeyMapping.dot11Privacy)},
	{"", STAINFO_keyInCam, STAINFO_PARAM, _SIZE_stainfo(dot11KeyMapping.keyInCam), _OFFSET_stainfo(dot11KeyMapping.keyInCam)},
	{"", STAINFO_dot11KeyMapping, STAINFO_STRUC, _SIZE_STRUC_stainfo(Dot11EncryptKey), _OFFSET_stainfo(dot11KeyMapping.dot11EncryptKey)},
	{"bssratelen", STAINFO_bssratelen, STAINFO_PARAM, _SIZE_stainfo(bssratelen), _OFFSET_stainfo(bssratelen)},
	{"hp_level", STAINFO_hp_level, STAINFO_PARAM, _SIZE_stainfo(hp_level), _OFFSET_stainfo(hp_level)},
	{"useCts2self", STAINFO_useCts2self, STAINFO_PARAM, _SIZE_stainfo(useCts2self), _OFFSET_stainfo(useCts2self)},
	{"aggre_mthd", STAINFO_aggre_mthd, STAINFO_PARAM, _SIZE_stainfo(aggre_mthd), _OFFSET_stainfo(aggre_mthd)},
	{"tx_avarage", STAINFO_tx_avarage, STAINFO_PARAM, _SIZE_stainfo(tx_avarage), _OFFSET_stainfo(tx_avarage)},
	{"rx_avarage", STAINFO_rx_avarage, STAINFO_PARAM, _SIZE_stainfo(rx_avarage), _OFFSET_stainfo(rx_avarage)},	
	{"", STAINFO_ADDBA_ready, STAINFO_PARAM, _SIZE_stainfo(ADDBA_ready), _OFFSET_stainfo(ADDBA_ready)},
	{"", STAINFO_ADDBA_req_num, STAINFO_PARAM, _SIZE_stainfo(ADDBA_req_num), _OFFSET_stainfo(ADDBA_req_num)},
	{"", STAINFO_ADDBA_sent, STAINFO_PARAM, _SIZE_stainfo(ADDBA_sent), _OFFSET_stainfo(ADDBA_sent)},
	{"AMSDU_AMPDU_support", STAINFO_AMSDU_AMPDU_support, STAINFO_PARAM, _SIZE_stainfo(AMSDU_AMPDU_support), _OFFSET_stainfo(AMSDU_AMPDU_support)},
	{"maxAggNum", STAINFO_maxAggNum, STAINFO_PARAM, _SIZE_stainfo(maxAggNum), _OFFSET_stainfo(maxAggNum)},
	{"retry_inc", STAINFO_retry_inc, STAINFO_PARAM, _SIZE_stainfo(retry_inc), _OFFSET_stainfo(retry_inc)},
	{"wireless_mode", STAINFO_wireless_mode, STAINFO_PARAM, _SIZE_stainfo(wireless_mode), _OFFSET_stainfo(wireless_mode)},
	{"low_tp_disable_ampdu", STAINFO_low_tp_disable_ampdu, STAINFO_PARAM, _SIZE_stainfo(low_tp_disable_ampdu), _OFFSET_stainfo(low_tp_disable_ampdu)},
	{"uk_timeout", STAINFO_uk_timeout, STAINFO_PARAM, _SIZE_stainfo(uk_timeout), _OFFSET_stainfo(uk_timeout)},
	{"diffAmpduSz", STAINFO_diffAmpduSz, STAINFO_PARAM, _SIZE_stainfo(diffAmpduSz), _OFFSET_stainfo(diffAmpduSz)},
	{"ipmc_num", STAINFO_ipmc_num, STAINFO_PARAM, _SIZE_stainfo(ipmc_num), _OFFSET_stainfo(ipmc_num)},
	{"", STAINFO_ip_mcast_info, STAINFO_PARAM, MAX_VAR_LEN, _OFFSET_stainfo(ipmc)},
	{"", STAINFO_ip_mcast_info2, STAINFO_PARAM, _SIZE_stainfo(ipmc)-MAX_VAR_LEN, _OFFSET_stainfo(ipmc)+MAX_VAR_LEN},
};

struct var_arg stainfo_t2h_list[]=
{
	//Sync from A17 to Taroko ===
	{"rssi", STAINFO_T2H_rssi, STAINFO_PARAM, _SIZE_stainfo(rssi), _OFFSET_stainfo(rssi)},
	{"sq", STAINFO_T2H_sq, STAINFO_PARAM, _SIZE_stainfo(sq), _OFFSET_stainfo(sq)},
	{"rx_rate", STAINFO_T2H_rx_rate, STAINFO_PARAM, _SIZE_stainfo(rx_rate), _OFFSET_stainfo(rx_rate)},
	{"rx_bw", STAINFO_T2H_rx_bw, STAINFO_PARAM, _SIZE_stainfo(rx_bw), _OFFSET_stainfo(rx_bw)},
	{"rx_splcp", STAINFO_T2H_rx_splcp, STAINFO_PARAM, _SIZE_stainfo(rx_splcp), _OFFSET_stainfo(rx_splcp)},
	{"rf_info", STAINFO_T2H_rf_info, STAINFO_STRUC, _SIZE_STRUC_stainfo(rf_misc_info), _OFFSET_stainfo(rf_info)},
	{"highest_rx_rate", STAINFO_T2H_highest_rx_rate, STAINFO_PARAM, _SIZE_stainfo(highest_rx_rate), _OFFSET_stainfo(highest_rx_rate)},	
	{"current_tx_rate", STAINFO_T2H_current_tx_rate, STAINFO_PARAM, _SIZE_stainfo(current_tx_rate), _OFFSET_stainfo(current_tx_rate)},
	//Sync from A17 to Taroko ===

	//Sync from Taroko to A17 ===
	{"tx_bytes", STAINFO_T2H_tx_bytes, STAINFO_PARAM, _SIZE_stainfo(tx_bytes), _OFFSET_stainfo(tx_bytes)},
	{"tx_pkts", STAINFO_T2H_tx_pkts, STAINFO_PARAM, _SIZE_stainfo(tx_pkts), _OFFSET_stainfo(tx_pkts)},
	{"tx_pkts2", STAINFO_T2H_tx_pkts2, STAINFO_PARAM, _SIZE_stainfo(tx_pkts2), _OFFSET_stainfo(tx_pkts2)},
	{"tx_big_pkts", STAINFO_T2H_tx_big_pkts, STAINFO_PARAM, _SIZE_stainfo(tx_big_pkts), _OFFSET_stainfo(tx_big_pkts)},
	{"tx_sml_pkts", STAINFO_T2H_tx_sml_pkts, STAINFO_PARAM, _SIZE_stainfo(tx_sml_pkts), _OFFSET_stainfo(tx_sml_pkts)},
	{"tx_byte_cnt", STAINFO_T2H_tx_byte_cnt, STAINFO_PARAM, _SIZE_stainfo(tx_byte_cnt), _OFFSET_stainfo(tx_byte_cnt)},
	{"rx_bytes", STAINFO_T2H_rx_bytes, STAINFO_PARAM, _SIZE_stainfo(rx_bytes), _OFFSET_stainfo(rx_bytes)},
	{"rx_pkts", STAINFO_T2H_rx_pkts, STAINFO_PARAM, _SIZE_stainfo(rx_pkts), _OFFSET_stainfo(rx_pkts)},
	{"rx_pkts2", STAINFO_T2H_rx_pkts2, STAINFO_PARAM, _SIZE_stainfo(rx_pkts2), _OFFSET_stainfo(rx_pkts2)},
	{"rx_mgnt_pkts", STAINFO_T2H_rx_mgnt_pkts, STAINFO_PARAM, _SIZE_stainfo(rx_mgnt_pkts), _OFFSET_stainfo(rx_mgnt_pkts)},	
	{"rx_byte_cnt", STAINFO_T2H_rx_byte_cnt, STAINFO_PARAM, _SIZE_stainfo(rx_byte_cnt), _OFFSET_stainfo(rx_byte_cnt)},
	{"tcpack_pkts", STAINFO_T2H_tcpack_pkts, STAINFO_PARAM, _SIZE_stainfo(tcpack_pkts), _OFFSET_stainfo(tcpack_pkts)},
#ifdef _RX_REORDER_DEBUG_
	{"rx_rc4_count", STAINFO_T2H_rx_rc4_count, STAINFO_PARAM, _SIZE_stainfo(rx_rc4_count), _OFFSET_stainfo(rx_rc4_count)},
	{"rx_rc_timer_ovf", STAINFO_T2H_rx_rc_timer_ovf, STAINFO_PARAM, _SIZE_stainfo(rx_rc_timer_ovf), _OFFSET_stainfo(rx_rc_timer_ovf)},
	{"rx_rc_drop1", STAINFO_T2H_rx_rc_drop1, STAINFO_PARAM, _SIZE_stainfo(rx_rc_drop1), _OFFSET_stainfo(rx_rc_drop1)},
	{"rx_rc_drop3", STAINFO_T2H_rx_rc_drop3, STAINFO_PARAM, _SIZE_stainfo(rx_rc_drop3), _OFFSET_stainfo(rx_rc_drop3)},
	{"rx_rc_drop4", STAINFO_T2H_rx_rc_drop4, STAINFO_PARAM, _SIZE_stainfo(rx_rc_drop4), _OFFSET_stainfo(rx_rc_drop4)},
	{"rx_rc_reorder3", STAINFO_T2H_rx_rc_reorder3, STAINFO_PARAM, _SIZE_stainfo(rx_rc_reorder3), _OFFSET_stainfo(rx_rc_reorder3)},
	{"rx_rc_reorder4", STAINFO_T2H_rx_rc_reorder4, STAINFO_PARAM, _SIZE_stainfo(rx_rc_reorder4), _OFFSET_stainfo(rx_rc_reorder4)},
	{"rx_rc_passup2", STAINFO_T2H_rx_rc_passup2, STAINFO_PARAM, _SIZE_stainfo(rx_rc_passup2), _OFFSET_stainfo(rx_rc_passup2)},
	{"rx_rc_passup3", STAINFO_T2H_rx_rc_passup3, STAINFO_PARAM, _SIZE_stainfo(rx_rc_passup3), _OFFSET_stainfo(rx_rc_passup3)},
	{"rx_rc_passup4", STAINFO_T2H_rx_rc_passup4, STAINFO_PARAM, _SIZE_stainfo(rx_rc_passup4), _OFFSET_stainfo(rx_rc_passup4)},
	{"rx_rc_passupi", STAINFO_T2H_rx_rc_passupi, STAINFO_PARAM, _SIZE_stainfo(rx_rc_passupi), _OFFSET_stainfo(rx_rc_passupi)},			
#endif
#ifdef SW_TX_QUEUE
	//{"", STAINFO_T2H_swq, STAINFO_STRUC, _SIZE_STRUC_stainfo(sw_tx_q), _OFFSET_stainfo(swq)},
#endif	
	//Sync from Taroko to A17 ===
};


#if defined(CONFIG_PE_ENABLE)

void dump_pe_stainfo_t2h_list(struct stat_info *pstat)
{
	unsigned int i = 0;
	unsigned int shift = 0, pe_len = 0, pe_offset = 0;
	
	while(i != STAINFO_T2H_MAX){
		shift = i*sizeof(struct var_arg)+32;		
		pe_len = *(unsigned int *)(pstat->pe_stainfo_t2h_list+shift+4*2);
		pe_offset = *(unsigned int *)(pstat->pe_stainfo_t2h_list+shift+4*3);
		panic_printk("pe_len = %d, pe_offset = %d\r\n", pe_len, pe_offset);
		i++;
	}
}

int pe_stainfo_add(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct stainfo_cb var;

	memset(&var, 0, sizeof(struct stainfo_cb));
	memcpy(var.mac, pstat->cmn_info.mac_addr, 6);
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	if(IS_VXD_INTERFACE(priv))
		var.vap_id = -2;
	else
		var.vap_id = priv->vap_id;
#endif	
	tx_sta_add_cmd(&var, sizeof(struct stainfo_cb));
}

int pe_stainfo_del(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct stainfo_cb var;

	memset(&var, 0, sizeof(struct stainfo_cb));
	memcpy(var.mac, pstat->cmn_info.mac_addr, 6);
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	if(IS_VXD_INTERFACE(priv))
		var.vap_id = -2;
	else
		var.vap_id = priv->vap_id;
#endif		
	tx_sta_del_cmd(&var, sizeof(struct stainfo_cb));

	pstat->pstat_pe = NULL;
	if(pstat->pe_stainfo_t2h_list)
		kfree(pstat->pe_stainfo_t2h_list);
	pstat->pe_stainfo_t2h_list = NULL;
	
}

#if defined(PE_INFO_SYNC)
int pe_stainfo_update(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{		
	unsigned int index = 0, max = STAINFO_H2T_MAX;

	//return 0;

	#if 0
	if(priv->pshare->rf_ft_var.swq_dbg != 0)
		max = priv->pshare->rf_ft_var.swq_dbg-1;
	#endif

	//panic_printk("STAINFO_H2T_MAX=%d [STA] %02x:%02x:%02x:%02x:%02x:%02x\n", max, 
	//	pstat->cmn_info.mac_addr[0],pstat->cmn_info.mac_addr[1],pstat->cmn_info.mac_addr[2],
	//	pstat->cmn_info.mac_addr[3],pstat->cmn_info.mac_addr[4],pstat->cmn_info.mac_addr[5]);
	
	while(index != max)
	{
		pe_stainfo_set(priv, pstat, index);
		index++;
	}
}

int pe_stainfo_aggre_update(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{		
	pe_stainfo_set(priv, pstat, STAINFO_aggre_mthd);
}

int pe_stainfo_ba_update(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{		
	unsigned int i = 0, max = STAINFO_H2T_MAX;

	for(i=STAINFO_ADDBA_ready; i<=STAINFO_maxAggNum; i++)
		pe_stainfo_set(priv, pstat, i);
	
	pe_stainfo_set(priv, pstat, STAINFO_aggre_mthd);
	pe_stainfo_set(priv, pstat, STAINFO_diffAmpduSz);
}

int pe_stainfo_mc2u_update(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	pe_stainfo_set(priv, pstat, STAINFO_ipmc_num);
	pe_stainfo_set(priv, pstat, STAINFO_ip_mcast_info);
	pe_stainfo_set(priv, pstat, STAINFO_ip_mcast_info2);
}

#if (BEAMFORMING_SUPPORT == 1)
int pe_stainfo_beamforming_update(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{		
	pe_stainfo_set(priv, pstat, STAINFO_cmn_info);
	pe_stainfo_set(priv, pstat, STAINFO_is_beamforming_entry);
	pe_stainfo_set(priv, pstat, STAINFO_is_sounding_en);
	
#if (MU_BEAMFORMING_SUPPORT == 1)
	if(priv->pmib->dot11RFEntry.txbf_mu == 1){
		if(pstat->muPartner_num != pstat->muPartner_num_pre){
			panic_printk("STA[%02x] set muPartner_num %d to %d\n", 
				pstat->cmn_info.mac_addr[5], pstat->muPartner_num_pre, pstat->muPartner_num);			
			pstat->muPartner_num_pre = pstat->muPartner_num;			
		}
		
		pe_stainfo_set(priv, pstat, STAINFO_muPartner_num);
		pe_stainfo_set(priv, pstat, STAINFO_mu_deq_num);	
		pe_stainfo_set(priv, pstat, STAINFO_muPartner_aid);
		pe_var_set(priv, VAR_beamformee_mu_cnt);
		
		
	}
#endif

}
#endif

int pe_stainfo_set(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned int index)
{	
	int i;

	#if 1//for debug
	//panic_printk("stainfo_h2t_list[%d]: pstat=0x%x num=%d, len=0x%x, offset=0x%x\n", 
	//	index, pstat, stainfo_h2t_list[index].num, stainfo_list[index].len, stainfo_list[index].offset);	
	
	u8 data[MAX_VAR_LEN];
	memset(data, 0x0, MAX_VAR_LEN);
	//if(index == STAINFO_txie){
	//	for(i=0;i<6;i++)
	//		panic_printk("[%d] pstat->txie[%d][0]=0x%x txie[%d][1]=0x%x\n", index, i, pstat->txie[i][0], i, pstat->txie[i][1]);
	//}
	//else
	{
		if(stainfo_h2t_list[index].len > MAX_VAR_LEN)
			panic_printk("[%d] pstat->%s is too large(%d) to copy(MAX:%d)\n", index, stainfo_h2t_list[index].name, stainfo_h2t_list[index].len, MAX_VAR_LEN);
		else{
			if(priv->pshare->rf_ft_var.swq_dbg == 58 && index == STAINFO_aggre_mthd){
				memcpy(data, (unsigned int)pstat + stainfo_h2t_list[index].offset, stainfo_h2t_list[index].len);			
				panic_printk("[%d] pstat->%s=0x%x\n", index, stainfo_h2t_list[index].name, *data);
			}
		}
	}
	#endif

#if 0
	if(index == STAINFO_ipmc_num)
		panic_printk("pstat->ipmc_num=%d\n", pstat->ipmc_num);

	if(index == STAINFO_ip_mcast_info){
		for(i=0;i<pstat->ipmc_num;i++)
			panic_printk("[%d] %02x%02x%02x%02x%02x%02x\n", i, 
				pstat->ipmc[i].mcmac[0],pstat->ipmc[i].mcmac[1],pstat->ipmc[i].mcmac[2],
				pstat->ipmc[i].mcmac[3],pstat->ipmc[i].mcmac[4],pstat->ipmc[i].mcmac[5]);
	}
#endif

	tx_stainfo_set_cmd(priv, pstat, stainfo_h2t_list[index].num, stainfo_h2t_list[index].len, (unsigned int)pstat + stainfo_h2t_list[index].offset);
		
}

int pe_stainfo_sync(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i=0;
	//u8 data[MAX_VAR_LEN];
	unsigned int shift=0, pe_len=0, pe_offset=0, a17_offset=0;
	unsigned int tmp;
	
	if(pstat->pstat_pe == NULL || pstat->pe_stainfo_t2h_list == NULL){
		panic_printk("%s: cannot sync taroko sta_info becoz pointer is NULL\n", __FUNCTION__);
		return 0;
	}

	//panic_printk("sizeof(struct var_arg)=%d\n", sizeof(struct var_arg));	
	//mem_dump("pe_stainfo_t2h_list", pstat->pe_stainfo_t2h_list, 96);

	while(i != STAINFO_T2H_MAX){
		shift = i*sizeof(struct var_arg)+32;		
		pe_len = *(unsigned int *)(pstat->pe_stainfo_t2h_list+shift+4*2);
		pe_offset = *(unsigned int *)(pstat->pe_stainfo_t2h_list+shift+4*3);
		a17_offset = stainfo_t2h_list[i].offset;
		
		#if 0
		memset(data, 0x0, MAX_VAR_LEN);
		panic_printk("[%d] pstat=%x num=%x type=%x len=%x offset=%x\n",
				i,
				pstat->pstat_pe,
				*(pstat->pe_stainfo_t2h_list+shift+4*0),
				*(pstat->pe_stainfo_t2h_list+shift+4*1), 
				*(pstat->pe_stainfo_t2h_list+shift+4*2),
				*(pstat->pe_stainfo_t2h_list+shift+4*3));		
		#endif
						
		//memcpy(data, (unsigned int)pstat->pstat_pe + offset, len);	
		//panic_printk("pstat(offset=%x)->%s = %d\n",  ((unsigned int)pstat->pstat_pe + offset),  stainfo_t2h_list[i].name, *(unsigned int *)data);

		if(pe_len != stainfo_t2h_list[i].len){
			panic_printk("[WARNING!!!][%s][idx:%d] pstat->%s has different size between A17(%d) and Taroko(%d), plz sync\n", 
				__FUNCTION__, i, stainfo_h2t_list[i].name, stainfo_t2h_list[i].len, pe_len);
			panic_printk("aid = %d\r\n", pstat->cmn_info.aid);
			dump_pe_stainfo_t2h_list(pstat);
			//recovery
			if(priv->pe_stainfo_t2h_list)
			{
				//panic_printk("len = %d\r\n", sizeof(struct var_arg)* STAINFO_T2H_MAX);
				memcpy(pstat->pe_stainfo_t2h_list, priv->pe_stainfo_t2h_list, sizeof(struct var_arg)* STAINFO_T2H_MAX);
			}
			return 0;
		}

		if(stainfo_t2h_list[i].num <= STAINFO_T2H_current_tx_rate){
			memcpy((unsigned int)pstat->pstat_pe + pe_offset, (unsigned int)pstat + a17_offset, pe_len);
		}else {
			if(stainfo_t2h_list[i].num == STAINFO_T2H_tx_pkts){									
				memcpy(&tmp, (unsigned int)pstat->pstat_pe + pe_offset, pe_len);
				if(pstat->tx_pkts == 0)
					pstat->tx_pkts = tmp;					
				else
					pstat->tx_pkts = (pstat->tx_pkts-pstat->pe_txpkts_pre)+tmp;

				pstat->pe_txpkts_pre = tmp;
			}
			else
				memcpy((unsigned int)pstat + a17_offset, (unsigned int)pstat->pstat_pe + pe_offset, pe_len);
		}

		//for debug
		if(priv->pshare->rf_ft_var.swq_dbg == 999){
			panic_printk("pstat->%s = %d(0x%x)\n", stainfo_t2h_list[i].name, 
				*(unsigned int *)((unsigned int)pstat + a17_offset), ((unsigned int)pstat + a17_offset));
		}
		
		i++;
	}

	return 1;
}

int pe_priv_param_sync(struct rtl8192cd_priv *priv)
{
	int i=0;
	//u8 data[MAX_VAR_LEN];
	unsigned int shift=0, pe_len=0, pe_offset=0, a17_offset=0;
	unsigned long tmp;

	if(priv->pe_net_stats == NULL || priv->pe_ext_stats == NULL || priv->pe_pshare_phw == NULL ||
		priv->pe_privparam_t2h_list == NULL){
		panic_printk("%s: cannot sync taroko priv param becoz pointer is NULL\n", __FUNCTION__);
		//pe_var_set(priv, VAR_t2h_sync);
		return 0;
	}

	//panic_printk("sizeof(struct var_arg)=%d\n", sizeof(struct var_arg));	
	//mem_dump("pe_stainfo_t2h_list", pstat->pe_stainfo_t2h_list, 96);

	while(i != VAR_T2H_MAX){
		shift = i*sizeof(struct var_arg)+32;
		
		pe_len = *(unsigned int *)(priv->pe_privparam_t2h_list+shift+4*2);
		pe_offset = *(unsigned int *)(priv->pe_privparam_t2h_list+shift+4*3);
		a17_offset = var_t2h_list[i].offset;

		//memset(data, 0x0, MAX_VAR_LEN);
		//panic_printk("index=%d shift=%d\n", i, shift);

		if(pe_len != var_t2h_list[i].len){
			panic_printk("[WARNING!!!][%s][idx:%d] priv->%s has different size between A17(%d) and Taroko(%d), plz sync\n", 
				__FUNCTION__, i, var_t2h_list[i].name, var_t2h_list[i].len, pe_len);
			i++;
			continue;
		}
			
		if(var_t2h_list[i].type == VAR_NETSTATS){			

			#if 0
			panic_printk("[%d] pe_net_stats=%x ", i, priv->pe_net_stats);
			panic_printk("num=%x ", *(priv->pe_privparam_t2h_list+shift+4*0));
			panic_printk("type=%x ", *(priv->pe_privparam_t2h_list+shift+4*1));
			panic_printk("len=%x ", len);
			panic_printk("offset=%x(%x)\n", offset, var_t2h_list[i].offset);				
			#endif
			
			//memcpy(data, (unsigned int)priv->pe_net_stats + offset, len);	

			//panic_printk("[Taroko] priv->pe_net_stats.%s(offset=%x) = %d\n", var_t2h_list[i].name, 
			//	((unsigned int)priv->pe_net_stats + offset), *(unsigned int *)data);
			
			if(var_t2h_list[i].num == VAR_tx_packets) {
				
				memcpy(&tmp, (unsigned int)priv->pe_net_stats + pe_offset, pe_len);

				if(priv->net_stats.tx_packets == 0)
					priv->net_stats.tx_packets = tmp;					
				else
					priv->net_stats.tx_packets = (priv->net_stats.tx_packets-priv->pe_netstats_txpkts_pre)+tmp;

				priv->pe_netstats_txpkts_pre = tmp;

			} else if(var_t2h_list[i].num == VAR_tx_bytes) {

				memcpy(&tmp, (unsigned int)priv->pe_net_stats + pe_offset, pe_len);

				if(priv->net_stats.tx_bytes == 0)
					priv->net_stats.tx_bytes = tmp;					
				else
					priv->net_stats.tx_bytes = (priv->net_stats.tx_bytes-priv->pe_netstats_txbytes_pre)+tmp;

				priv->pe_netstats_txbytes_pre = tmp;
				
			}  else
				memcpy((unsigned int)&priv->net_stats + a17_offset, (unsigned int)priv->pe_net_stats + pe_offset, pe_len);


			if(priv->pshare->rf_ft_var.swq_dbg == 800)
				panic_printk("[A17] priv->net_stat.%s = %d\n", var_t2h_list[i].name, *(unsigned int *)((unsigned int)&priv->net_stats + a17_offset));
			
		} else if(var_t2h_list[i].type == VAR_EXTSTATS){

			#if 0
			panic_printk("[%d] pe_ext_stats=%x ", i, priv->pe_ext_stats);
			panic_printk("num=%x ", *(priv->pe_privparam_t2h_list+shift+4*0));
			panic_printk("type=%x ", *(priv->pe_privparam_t2h_list+shift+4*1));
			panic_printk("len=%x ", len);
			panic_printk("offset=%x(%x)\n", offset, var_t2h_list[i].offset);				
			#endif
			
			//memcpy(data, (unsigned int)priv->pe_ext_stats + offset, len);	

			//panic_printk("[Taroko] priv->pe_ext_stats.%s(offset=%x) = %d\n", var_t2h_list[i].name, 
			//	(unsigned int)priv->pe_ext_stats + offset, *(unsigned int *)data);

			if(var_t2h_list[i].num == VAR_tx_byte_cnt || var_t2h_list[i].num == VAR_rx_byte_cnt) {

				memcpy(&tmp, (unsigned int)priv->pe_ext_stats + pe_offset, pe_len);

				if(var_t2h_list[i].num == VAR_tx_byte_cnt){
					if(priv->ext_stats.tx_byte_cnt == 0)
						priv->ext_stats.tx_byte_cnt = tmp-priv->pe_extstats_txbytecnt_pre;					
					else
						priv->ext_stats.tx_byte_cnt = (priv->ext_stats.tx_byte_cnt+tmp)-priv->pe_extstats_txbytecnt_pre;
					priv->pe_extstats_txbytecnt_pre = tmp;
				}else{
					if(priv->ext_stats.rx_byte_cnt == 0)
						priv->ext_stats.rx_byte_cnt = tmp-priv->pe_extstats_rxbytecnt_pre;					
					else
						priv->ext_stats.rx_byte_cnt = (priv->ext_stats.rx_byte_cnt+tmp)-priv->pe_extstats_rxbytecnt_pre;
					priv->pe_extstats_rxbytecnt_pre = tmp;
				}				
			} else
				memcpy((unsigned int)&priv->ext_stats + a17_offset, (unsigned int)priv->pe_ext_stats + pe_offset, pe_len);

			if(priv->pshare->rf_ft_var.swq_dbg == 801)
				panic_printk("[A17] priv->ext_stat.%s = %d\n", var_t2h_list[i].name, *(unsigned int *)((unsigned int)&priv->ext_stats + a17_offset));
						
		}	else if(var_t2h_list[i].type == VAR_PHW) {
		
			memcpy((unsigned int)priv->pshare->phw + a17_offset, (unsigned int)priv->pe_pshare_phw + pe_offset, pe_len);
			
			if(priv->pshare->rf_ft_var.swq_dbg == 802)
				panic_printk("[A17] priv->pshare->phw->%s = %d\n", var_t2h_list[i].name, *(unsigned int *)((unsigned int)priv->pshare->phw + a17_offset));

			if(priv->pshare->rf_ft_var.swq_dbg == 803)
				if(var_t2h_list[i].num == VAR_VO_pkt_count)
					panic_printk("VO_pkt_count=%d\n", priv->pshare->phw->VO_pkt_count);
		}
		i++;
	}
}
#endif

int pe_var_initialize(struct rtl8192cd_priv *priv)
{
	unsigned int index = 0;
	
	while(index != VAR_MAX)
	{		
		pe_var_set(priv, index);
		index++;
	}
}

int pe_var_set(struct rtl8192cd_priv *priv, unsigned int index)
{
	if(index == VAR_t2h_sync) {
		if(priv->pe_net_stats == NULL || priv->pe_ext_stats == NULL || priv->pe_privparam_t2h_list == NULL)
			tx_var_set_cmd(priv, &var_list[index], var_list[index].offset);
	} else{
		if(var_list[index].type == VAR_PRIV)
			tx_var_set_cmd(priv, &var_list[index], (unsigned int)priv + var_list[index].offset);
		else if(var_list[index].type == VAR_MIB)
			tx_var_set_cmd(priv, &var_list[index], (unsigned int)priv->pmib + var_list[index].offset);
		else if(var_list[index].type == VAR_PSHARE)
			tx_var_set_cmd(priv, &var_list[index], (unsigned int)priv->pshare + var_list[index].offset);
		else if(var_list[index].type == VAR_PHW)
			tx_var_set_cmd(priv, &var_list[index], (unsigned int)priv->pshare->phw + var_list[index].offset);
		else if(var_list[index].type == VAR_RF)
			tx_var_set_cmd(priv, &var_list[index], (unsigned int)&priv->pshare->rf_ft_var + var_list[index].offset);
	}
}

void pe_translate_rssi_sq_outsrc(struct stat_info *pstat, struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, char rate)
{
	struct phydm_phyinfo_struct *pPhyInfo = &pfrinfo->phy_info;
	struct phydm_perpkt_info_struct pktinfo;

	pktinfo.data_rate = rate;
	pktinfo.is_packet_to_self = 1;
	pktinfo.is_packet_match_bssid = 1;
	pktinfo.station_id = (pstat ? ((u1Byte)(pstat->cmn_info.aid)) : 0);	
	
	odm_phy_status_query(ODMPTR, pPhyInfo, (u1Byte *)pfrinfo->driver_info, &pktinfo);

	pfrinfo->rx_bw = pPhyInfo->band_width;
}

unsigned char pe_sta_state_get(unsigned short aid)
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	struct pe_controlblock *pe_cb = pe_addr->pe_reserved_virt_addr;
	unsigned char index_shift = ((aid - 1) / 8);
	unsigned char bit_shift = ((aid - 1) % 8);
	
	//panic_printk("index_shift = %d, bit_shift = %d\r\n", index_shift, bit_shift);
	
	return (pe_cb->sta_state[index_shift] & BIT(bit_shift));
}

unsigned char pe_sta_hw_queue_state_get(unsigned short aid)
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	struct pe_controlblock *pe_cb = pe_addr->pe_reserved_virt_addr;
	unsigned char index_shift = ((aid - 1) / 8);
	unsigned char bit_shift = ((aid - 1) % 8);
	
	//panic_printk("index_shift = %d, bit_shift = %d\r\n", index_shift, bit_shift);

	return (pe_cb->sta_hw_queue_state[index_shift] & BIT(bit_shift));
}

unsigned char pe_sta_sw_queue_state_get(unsigned short aid)
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	struct pe_controlblock *pe_cb = pe_addr->pe_reserved_virt_addr;
	unsigned char index_shift = ((aid - 1) / 8);
	unsigned char bit_shift = ((aid - 1) % 8);
	
	//panic_printk("index_shift = %d, bit_shift = %d\r\n", index_shift, bit_shift);

	return (pe_cb->sta_sw_queue_state[index_shift] & BIT(bit_shift));
}

unsigned char check_pe_ready()
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	unsigned char data = *((unsigned char*)pe_addr->pe_reserved_virt_addr);

	//panic_printk("pe_ready = %x\r\n", pe_ready);

	if(data & PE_READY)
		return TRUE;
	else
		return FALSE;
}

unsigned char check_pe_started()
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	unsigned char data = *((unsigned char*)pe_addr->pe_reserved_virt_addr);

	if(data & PE_STARTED)
		return TRUE;
	else
		return FALSE;
}

unsigned char check_pe_stoped()
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	unsigned char data = *((unsigned char*)pe_addr->pe_reserved_virt_addr);

	if(data & PE_STOPED)
		return TRUE;
	else
		return FALSE;
}


void clear_pe_status()
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	unsigned char* data = (unsigned char*)pe_addr->pe_reserved_virt_addr;

	*data = 0x0;
}

#elif defined(TAROKO_0)
int pe_var_set(struct rtl8192cd_priv *priv, unsigned char* buf)
{
	struct var_cb* var = buf;
	int tmp=0;

#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	if(var->vap_id >= 0)
	{
		priv = priv->pvap_priv[var->vap_id];
	}
	else if(var->vap_id == -2)
		priv = priv->pvxd_priv;
#endif

#if defined(PE_INFO_SYNC)
	if(var->num == VAR_t2h_sync){
		taroko_t2h_ipc_add_priv_param(priv);
	} else 
#endif
	{
		if(var->len !=  var_list[var->num].len){
			panic_printk("[WARNING!!!][%s][idx:%d] priv->%s has different between A17 and Taroko, plz sync\n", 
				__FUNCTION__, var->num, var_list[var->num].name);
		}
		
		if(var->num == VAR_swq_en){
			tmp = priv->pshare->swq_en;
		}
		
		if(var_list[var->num].type == VAR_PRIV)
			memcpy((unsigned int)priv + var_list[var->num].offset, var->data ,var->len);
		else if(var_list[var->num].type == VAR_MIB)
			memcpy((unsigned int)priv->pmib + var_list[var->num].offset, var->data ,var->len);
		else if(var_list[var->num].type == VAR_PSHARE)
			memcpy((unsigned int)priv->pshare + var_list[var->num].offset, var->data ,var->len);
		else if(var_list[var->num].type == VAR_PHW)
			memcpy((unsigned int)priv->pshare->phw + var_list[var->num].offset, var->data ,var->len);
		else if(var_list[var->num].type == VAR_RF)
			memcpy((unsigned int)&priv->pshare->rf_ft_var + var_list[var->num].offset, var->data ,var->len);

		if(var->num == VAR_swq_en){
			if(tmp==1 && priv->pshare->swq_en==0){
				priv->pshare->swqen_keeptime = 0;	
				panic_printk("swq_en=%d\n", priv->pshare->swq_en);
			} else if(tmp==0 && priv->pshare->swq_en==1){
				priv->pshare->swqen_keeptime = priv->up_time;				
				extern void init_STA_SWQAggNum(struct rtl8192cd_priv *priv);
				init_STA_SWQAggNum(priv);
				panic_printk("swq_en=%d\n", priv->pshare->swq_en);
			}
		}
	}
}

int pe_stainfo_set(struct stat_info *pstat, unsigned char* buf)
{
	struct stainfo_cb* var = buf;

	//panic_printk("stainfo_h2t_list[%d]: data=0x%x num=%d len=0x%x, offset=0x%x\n", 
	//	var->num, var->data, stainfo_list[var->num].num , var->len, stainfo_list[var->num].offset);

	if(var->len !=  stainfo_h2t_list[var->num].len){
		panic_printk("[WARNING!!!][%s][idx:%d] pstat->%s has different size between A17 and Taroko, plz sync\n", 
			__FUNCTION__, var->num, stainfo_h2t_list[var->num].name);
	}
		
	memcpy((unsigned int)pstat + stainfo_h2t_list[var->num].offset, var->data , var->len);

#if (MU_BEAMFORMING_SUPPORT == 1)	
	if(var->num == STAINFO_muPartner_aid) {
		int i;
		struct rtl8192cd_priv *priv = get_root_priv();
		struct stat_info *pstat_tmp = NULL;	
		for(i=0;i<MAX_NUM_MU_PARTNER;i++) {
			if(pstat->muPartner_aid[i] != 0){
				pstat_tmp = get_aidinfo(priv, pstat->muPartner_aid[i]);
				pstat->muPartner[i] = pstat_tmp;
			}
			 
		}
	}
#endif	

	//if(var->num == STAINFO_cmn_info)
	//	panic_printk("[STA] %02x:%02x:%02x:%02x:%02x:%02x sunc sta_info\n",
	//		pstat->cmn_info.mac_addr[0], pstat->cmn_info.mac_addr[1], pstat->cmn_info.mac_addr[2],
	//		pstat->cmn_info.mac_addr[3], pstat->cmn_info.mac_addr[4], pstat->cmn_info.mac_addr[5]);
#if 0
	if(var->num == STAINFO_ipmc_num)
		panic_printk("pstat->ipmc_num\n", pstat->ipmc_num);

	if(var->num == STAINFO_ip_mcast_info){
		for(i=0;i<pstat->ipmc_num;i++)
			panic_printk("[%d] %02x%02x%02x%02x%02x%02x\n", i, 
				pstat->ipmc[i].mcmac[0],pstat->ipmc[i].mcmac[1],pstat->ipmc[i].mcmac[2],
				pstat->ipmc[i].mcmac[3],pstat->ipmc[i].mcmac[4],pstat->ipmc[i].mcmac[5]);
	}
#endif	
}

#if defined(PE_RX_INFO_REPORT)

extern struct rx_info_idx sw_idx;


__PE_IMEM__
void pe_collect_rx_info(struct rx_frinfo *pfrinfo, char aid, char rate)
{
#if 0
	//struct rtl8192cd_priv *priv = get_root_priv();
	//if(priv->pshare->rf_ft_var.swq_dbg == 1101)
	//	panic_printk("aid=%d rate=%d\n", (unsigned int)aid, (unsigned int)rate);
	// | aid | rate | rx_rate | rx_bw | rx_rx_splcp | phy info | drive info |	
	unsigned int offset = num_of_rxinfo * RXINFO_SIZE_PER_PACKET;
	//panic_printk("num_of_rxinfo=%d offset0=%d ", num_of_rxinfo, offset);

	memcpy(&rxinfo_buf[offset], &aid, sizeof(char));
	offset += sizeof(char);
	//panic_printk("offset_aid=%d ", offset);

	memcpy(&rxinfo_buf[offset], &rate, sizeof(char));
	offset += sizeof(char);
	//panic_printk("offset_rate=%d ", offset);

	memcpy(&rxinfo_buf[offset], &pfrinfo->rx_rate, sizeof(unsigned char));
	offset += sizeof(unsigned char);
	//panic_printk("offset_rx_rate=%d ", offset);

	memcpy(&rxinfo_buf[offset], &pfrinfo->rx_bw, sizeof(unsigned char));
	offset += sizeof(unsigned char);
	//panic_printk("offset_rx_bw=%d ", offset);

	memcpy(&rxinfo_buf[offset], &pfrinfo->rx_splcp, sizeof(unsigned char));
	offset += sizeof(unsigned char);
	//panic_printk("offset_rx_splcp=%d ", offset);

	memcpy(&rxinfo_buf[offset], &pfrinfo->phy_info, sizeof(struct phydm_phyinfo_struct));
	offset += sizeof(struct phydm_phyinfo_struct);
	//panic_printk("offset_phy_info=%d ", offset);

	memcpy(&rxinfo_buf[offset], pfrinfo->driver_info, pfrinfo->driver_info_size);
	offset += pfrinfo->driver_info_size;
	//panic_printk("offset_driver_info=%d\n", offset);

	num_of_rxinfo++;
	if(num_of_rxinfo == NUM_RXINFO_PER_REPORT){
		taroko_t2h_ipc_rxinfo_report(rxinfo_buf, offset);
		//if(priv->pshare->rf_ft_var.swq_dbg == 1101)
		//	panic_printk("send one NUM_RXINFO_PER_REPORT\n\n");
		num_of_rxinfo = 0;
	}
#else

	unsigned short backup_wdx = sw_idx.rx_info_wdx;
	struct pe_controlblock* pe_cb = get_pe_cb_base();
	
	sw_idx.rx_info_wdx++;
	if(sw_idx.rx_info_wdx == RX_INFO_ARRAY_SIZE)
		sw_idx.rx_info_wdx = 0;
	if(sw_idx.rx_info_wdx == sw_idx.rx_info_rdx)
	{
		//read index
		sw_idx.rx_info_rdx = pe_cb->rx_info.rx_info_rdx;
		if(sw_idx.rx_info_rdx >= RX_INFO_ARRAY_SIZE)
		{
			panic_printk("rx_info_rdx error %d\r\n", sw_idx.rx_info_wdx);
			return;
		}
		if(sw_idx.rx_info_wdx == sw_idx.rx_info_rdx)
		{
			sw_idx.rx_info_wdx = backup_wdx;
			panic_printk("rx info write fail : rx info is full");
			return;
		}
	}

	if(1)
	{
		unsigned char* rxinfo_buf = &pe_cb->rx_info.rx_info_buf[backup_wdx];
		unsigned int offset = 0;
		
		memcpy(&rxinfo_buf[offset], &aid, sizeof(char));
		offset += sizeof(char);
		//panic_printk("offset_aid=%d ", offset);
	
		memcpy(&rxinfo_buf[offset], &rate, sizeof(char));
		offset += sizeof(char);
		//panic_printk("offset_rate=%d ", offset);
	
		memcpy(&rxinfo_buf[offset], &pfrinfo->rx_rate, sizeof(unsigned char));
		offset += sizeof(unsigned char);
		//panic_printk("offset_rx_rate=%d ", offset);
	
		memcpy(&rxinfo_buf[offset], &pfrinfo->rx_bw, sizeof(unsigned char));
		offset += sizeof(unsigned char);
		//panic_printk("offset_rx_bw=%d ", offset);
	
		memcpy(&rxinfo_buf[offset], &pfrinfo->rx_splcp, sizeof(unsigned char));
		offset += sizeof(unsigned char);
		//panic_printk("offset_rx_splcp=%d ", offset);
	
		memcpy(&rxinfo_buf[offset], &pfrinfo->phy_info, sizeof(struct phydm_phyinfo_struct));
		offset += sizeof(struct phydm_phyinfo_struct);
		//panic_printk("offset_phy_info=%d ", offset);
	
		memcpy(&rxinfo_buf[offset], pfrinfo->driver_info, pfrinfo->driver_info_size);
		offset += pfrinfo->driver_info_size;
		//panic_printk("offset_driver_info=%d\n", offset);

		pe_cb->rx_info.rx_info_wdx = sw_idx.rx_info_wdx;

		if(!(sw_idx.rx_info_wdx % RX_INFO_REPORT))
		{
			taroko_t2h_ipc_rxinfo_report(rxinfo_buf, offset);
		}


	}
#endif
}
#endif

void pe_sta_state_set(unsigned short aid, unsigned char state)
{
	struct pe_controlblock* pe_cb = get_pe_cb_base();
	unsigned char index_shift = (aid / 8);
	unsigned char bit_shift = (aid % 8);

	//panic_printk("index_shift = %d, bit_shift = %d\r\n", index_shift, bit_shift);
	//state 
	// 1 : sleep
	// 0 : wakeup
	
	if(state)
		pe_cb->sta_state[index_shift] |= BIT(bit_shift);
	else
		pe_cb->sta_state[index_shift] &= ~(BIT(bit_shift));
}

void pe_sta_hw_queue_state_set(unsigned short aid, unsigned char state)
{
	struct pe_controlblock* pe_cb = get_pe_cb_base();
	unsigned char index_shift = (aid / 8);
	unsigned char bit_shift = (aid % 8);

	//panic_printk("index_shift = %d, bit_shift = %d\r\n", index_shift, bit_shift);
	//state 
	// 1 : relese
	// 0 : pause/drop
	
	if(state)
		pe_cb->sta_hw_queue_state[index_shift] |= BIT(bit_shift);
	else
		pe_cb->sta_hw_queue_state[index_shift] &= ~(BIT(bit_shift));
}

void pe_sta_sw_queue_state_set(unsigned short aid, unsigned char state)
{
	struct pe_controlblock* pe_cb = get_pe_cb_base();
	unsigned char index_shift = (aid / 8);
	unsigned char bit_shift = (aid % 8);

	//panic_printk("index_shift = %d, bit_shift = %d\r\n", index_shift, bit_shift);
	//state 
	// 1 : dz_queue is not empty
	// 0 : dz_queue is empty
	
	if(state)
		pe_cb->sta_sw_queue_state[index_shift] |= BIT(bit_shift);
	else
		pe_cb->sta_sw_queue_state[index_shift] &= ~(BIT(bit_shift));
}

void set_pe_ready(void)
{
	struct pe_controlblock* pe_cb = get_pe_cb_base();
	pe_cb->magic_number |= PE_READY;
}

void set_pe_started(void)
{
	struct pe_controlblock* pe_cb = get_pe_cb_base();
	pe_cb->magic_number |= PE_STARTED;
}

void set_pe_stoped(void)
{
	struct pe_controlblock* pe_cb = get_pe_cb_base();
	pe_cb->magic_number |= PE_STOPED;
}
#endif


