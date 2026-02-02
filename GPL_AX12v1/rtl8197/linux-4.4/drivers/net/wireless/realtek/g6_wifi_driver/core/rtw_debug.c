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
#define _RTW_DEBUG_C_

#include <drv_types.h>
#include "_hal_rate.h"
#include "../phl/phl_headers.h"

#ifdef CONFIG_RTW_DEBUG
const char *rtw_log_level_str[] = {
	"_DRV_NONE_ = 0",
	"_DRV_ALWAYS_ = 1",
	"_DRV_ERR_ = 2",
	"_DRV_WARNING_ = 3",
	"_DRV_INFO_ = 4",
	"_DRV_DEBUG_ = 5",
	"_DRV_MAX_ = 6",
};

const char *phl_log_level_str[] = {
	"_PHL_NONE_ = 0",
	"_PHL_ALWAYS_ = 1",
	"_PHL_ERR_ = 2",
	"_PHL_WARNING_ = 3",
	"_PHL_INFO_ = 4",
	"_PHL_DEBUG_ = 5",
	"_PHL_MAX_ = 6",
};
#endif

#ifdef CONFIG_DEBUG_RTL871X
	u64 GlobalDebugComponents = 0;
#endif /* CONFIG_DEBUG_RTL871X */

#include <rtw_version.h>

#ifdef CONFIG_TDLS
	#define TDLS_DBG_INFO_SPACE_BTWN_ITEM_AND_VALUE	41
#endif

void dump_drv_version(void *sel)
{
	RTW_PRINT_SEL(sel, "%s %s\n", DRV_NAME, DRIVERVERSION);
	RTW_PRINT_SEL(sel, "build time: %s %s\n", __DATE__, __TIME__);
}

#ifdef PLATFORM_ECOS
int wfo_seqdump(const char *fmt, ...)
{

	va_list ap;
	int proc_log_size = 0;
	u32 wfo_radio_cmd_sender_pass_log(wfo_cfg80211_t *);

	va_start(ap, fmt);

	if (proc_cfg != NULL) {
		proc_log_size = diag_vprintf_null(fmt, ap);
		//proc_log_size = diag_vprintf(fmt, ap);

		if (proc_cfg->pass_log.write_offset + proc_log_size < MAX_PROC_LOG_LEN) {
			diag_vsnprintf(proc_cfg->pass_log.str + proc_cfg->pass_log.write_offset, MAX_PROC_LOG_LEN - proc_cfg->pass_log.write_offset ,fmt, ap);
			proc_cfg->pass_log.write_offset = proc_cfg->pass_log.write_offset + strlen(proc_cfg->pass_log.str + proc_cfg->pass_log.write_offset);
		}
		else {
			wfo_radio_cmd_sender_pass_log(proc_cfg);
			memset(proc_cfg->pass_log.str, 0, strlen(proc_cfg->pass_log.str) + 1);
			diag_vsnprintf(proc_cfg->pass_log.str, MAX_PROC_LOG_LEN ,fmt, ap);
			proc_cfg->pass_log.write_offset = strlen(proc_cfg->pass_log.str);
		}
	}
	va_end(ap);

	return proc_log_size;
}

#endif

void dump_drv_cfg(void *sel)
{
#ifndef PLATFORM_ECOS
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
	char *kernel_version = utsname()->release;

	RTW_PRINT_SEL(sel, "\nKernel Version: %s\n", kernel_version);
#endif
#endif /* PLATFORM_ECOS */

	RTW_PRINT_SEL(sel, "Driver Version: %s\n", DRIVERVERSION);
	RTW_PRINT_SEL(sel, "------------------------------------------------\n");
#ifdef CONFIG_IOCTL_CFG80211
	RTW_PRINT_SEL(sel, "CFG80211\n");
#ifdef RTW_USE_CFG80211_STA_EVENT
	RTW_PRINT_SEL(sel, "RTW_USE_CFG80211_STA_EVENT\n");
#endif
	#ifdef CONFIG_RADIO_WORK
	RTW_PRINT_SEL(sel, "CONFIG_RADIO_WORK\n");
	#endif
#else
	RTW_PRINT_SEL(sel, "WEXT\n");
#endif

	RTW_PRINT_SEL(sel, "DBG:%d\n", DBG);
#ifdef CONFIG_RTW_DEBUG
	RTW_PRINT_SEL(sel, "CONFIG_RTW_DEBUG\n");
#endif

#ifdef CONFIG_CONCURRENT_MODE
	RTW_PRINT_SEL(sel, "CONFIG_CONCURRENT_MODE\n");
#endif

#ifdef CONFIG_POWER_SAVING
	RTW_PRINT_SEL(sel, "CONFIG_POWER_SAVING\n");
	#ifdef CONFIG_IPS
	RTW_PRINT_SEL(sel, "CONFIG_IPS\n");
	#endif
	#ifdef CONFIG_LPS
		RTW_PRINT_SEL(sel, "CONFIG_LPS\n");
		#ifdef CONFIG_LPS_LCLK
		RTW_PRINT_SEL(sel, "CONFIG_LPS_LCLK\n");
		#ifdef CONFIG_DETECT_CPWM_BY_POLLING
		RTW_PRINT_SEL(sel, "CONFIG_DETECT_CPWM_BY_POLLING\n");
		#endif
		#endif /*CONFIG_LPS_LCLK*/
		#ifdef CONFIG_LPS_CHK_BY_TP
		RTW_PRINT_SEL(sel, "CONFIG_LPS_CHK_BY_TP\n");
		#endif
		#ifdef CONFIG_LPS_ACK
		RTW_PRINT_SEL(sel, "CONFIG_LPS_ACK\n");
		#endif
	#endif/*CONFIG_LPS*/
#endif

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	RTW_PRINT_SEL(sel, "LOAD_PHY_PARA_FROM_FILE - REALTEK_CONFIG_PATH=%s\n", REALTEK_CONFIG_PATH);
	#if defined(CONFIG_MULTIDRV) || defined(REALTEK_CONFIG_PATH_WITH_IC_NAME_FOLDER)
	RTW_PRINT_SEL(sel, "LOAD_PHY_PARA_FROM_FILE - REALTEK_CONFIG_PATH_WITH_IC_NAME_FOLDER\n");
	#endif

/* configurations about TX power */
#ifdef CONFIG_CALIBRATE_TX_POWER_BY_REGULATORY
	RTW_PRINT_SEL(sel, "CONFIG_CALIBRATE_TX_POWER_BY_REGULATORY\n");
#endif
#ifdef CONFIG_CALIBRATE_TX_POWER_TO_MAX
	RTW_PRINT_SEL(sel, "CONFIG_CALIBRATE_TX_POWER_TO_MAX\n");
#endif
#endif
	RTW_PRINT_SEL(sel, "RTW_DEF_MODULE_REGULATORY_CERT=0x%02x\n", RTW_DEF_MODULE_REGULATORY_CERT);

	RTW_PRINT_SEL(sel, "CONFIG_TXPWR_BY_RATE=%d\n", CONFIG_TXPWR_BY_RATE);
	RTW_PRINT_SEL(sel, "CONFIG_TXPWR_BY_RATE_EN=%d\n", CONFIG_TXPWR_BY_RATE_EN);
	RTW_PRINT_SEL(sel, "CONFIG_TXPWR_LIMIT=%d\n", CONFIG_TXPWR_LIMIT);
	RTW_PRINT_SEL(sel, "CONFIG_TXPWR_LIMIT_EN=%d\n", CONFIG_TXPWR_LIMIT_EN);


#ifdef CONFIG_DISABLE_ODM
	RTW_PRINT_SEL(sel, "CONFIG_DISABLE_ODM\n");
#endif

#ifdef CONFIG_MINIMAL_MEMORY_USAGE
	RTW_PRINT_SEL(sel, "CONFIG_MINIMAL_MEMORY_USAGE\n");
#endif

	RTW_PRINT_SEL(sel, "CONFIG_RTW_ADAPTIVITY_EN = %d\n", CONFIG_RTW_ADAPTIVITY_EN);
#if (CONFIG_RTW_ADAPTIVITY_EN)
	RTW_PRINT_SEL(sel, "ADAPTIVITY_MODE = %s\n", (CONFIG_RTW_ADAPTIVITY_MODE) ? "carrier_sense" : "normal");
#endif

#ifdef CONFIG_WOWLAN
	RTW_PRINT_SEL(sel, "CONFIG_WOWLAN - ");

#ifdef CONFIG_GPIO_WAKEUP
	RTW_PRINT_SEL(sel, "CONFIG_GPIO_WAKEUP - WAKEUP_GPIO_IDX:%d\n", WAKEUP_GPIO_IDX);
#endif
#endif

#ifdef CONFIG_TDLS
	RTW_PRINT_SEL(sel, "CONFIG_TDLS\n");
#endif

#ifdef CONFIG_RTW_80211R
	RTW_PRINT_SEL(sel, "CONFIG_RTW_80211R\n");
#endif

#ifdef CONFIG_RTW_NETIF_SG
	RTW_PRINT_SEL(sel, "CONFIG_RTW_NETIF_SG\n");
#endif

#ifdef CONFIG_RTW_WIFI_HAL
	RTW_PRINT_SEL(sel, "CONFIG_RTW_WIFI_HAL\n");
#endif

#ifdef RTW_BUSY_DENY_SCAN
	RTW_PRINT_SEL(sel, "RTW_BUSY_DENY_SCAN\n");
	RTW_PRINT_SEL(sel, "BUSY_TRAFFIC_SCAN_DENY_PERIOD = %u ms\n", \
		      BUSY_TRAFFIC_SCAN_DENY_PERIOD);
#endif


#ifdef CONFIG_USB_HCI
#ifdef CONFIG_SUPPORT_USB_INT
	RTW_PRINT_SEL(sel, "CONFIG_SUPPORT_USB_INT\n");
#endif
#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	RTW_PRINT_SEL(sel, "CONFIG_USB_INTERRUPT_IN_PIPE\n");
#endif
#ifdef CONFIG_USB_TX_AGGREGATION
	RTW_PRINT_SEL(sel, "CONFIG_USB_TX_AGGREGATION\n");
#endif
#ifdef CONFIG_USB_RX_AGGREGATION
	RTW_PRINT_SEL(sel, "CONFIG_USB_RX_AGGREGATION\n");
#endif
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
	RTW_PRINT_SEL(sel, "CONFIG_USE_USB_BUFFER_ALLOC_TX\n");
#endif
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
	RTW_PRINT_SEL(sel, "CONFIG_USE_USB_BUFFER_ALLOC_RX\n");
#endif
#ifdef CONFIG_PREALLOC_RECV_SKB
	RTW_PRINT_SEL(sel, "CONFIG_PREALLOC_RECV_SKB\n");
#endif
#ifdef CONFIG_FIX_NR_BULKIN_BUFFER
	RTW_PRINT_SEL(sel, "CONFIG_FIX_NR_BULKIN_BUFFER\n");
#endif
#endif /*CONFIG_USB_HCI*/

#ifdef CONFIG_SDIO_HCI
#ifdef CONFIG_TX_AGGREGATION
	RTW_PRINT_SEL(sel, "CONFIG_TX_AGGREGATION\n");
#endif
#ifdef CONFIG_RX_AGGREGATION
	RTW_PRINT_SEL(sel, "CONFIG_RX_AGGREGATION\n");
#endif
#ifdef RTW_XMIT_THREAD_HIGH_PRIORITY
	RTW_PRINT_SEL(sel, "RTW_XMIT_THREAD_HIGH_PRIORITY\n");
#endif
#ifdef RTW_XMIT_THREAD_HIGH_PRIORITY_AGG
	RTW_PRINT_SEL(sel, "RTW_XMIT_THREAD_HIGH_PRIORITY_AGG\n");
#endif

#ifdef DBG_SDIO
	RTW_PRINT_SEL(sel, "DBG_SDIO = %d\n", DBG_SDIO);
#endif
#endif /*CONFIG_SDIO_HCI*/

#ifdef CONFIG_PCI_HCI
#endif

	RTW_PRINT_SEL(sel, "CONFIG_IFACE_NUMBER = %d\n", CONFIG_IFACE_NUMBER);


#ifdef CONFIG_PCI_TX_POLLING
	RTW_PRINT_SEL(sel, "CONFIG_PCI_TX_POLLING\n");
#endif

	/*GEORGIA_TODO_TRX - need get trx buff accroding to IC spec*/
	RTW_PRINT_SEL(sel, "\n=== XMIT-INFO ===\n");
	RTW_PRINT_SEL(sel, "NR_XMITFRAME = %d\n", NR_XMITFRAME);
	/*
	RTW_PRINT_SEL(sel, "NR_XMITBUFF = %d\n", NR_XMITBUFF);
	RTW_PRINT_SEL(sel, "MAX_XMITBUF_SZ = %d\n", MAX_XMITBUF_SZ);
	RTW_PRINT_SEL(sel, "NR_XMIT_EXTBUFF = %d\n", NR_XMIT_EXTBUFF);
	RTW_PRINT_SEL(sel, "MAX_XMIT_EXTBUF_SZ = %d\n", MAX_XMIT_EXTBUF_SZ);
	*/
	RTW_PRINT_SEL(sel, "MAX_CMDBUF_SZ = %d\n", MAX_CMDBUF_SZ);

	RTW_PRINT_SEL(sel, "\n=== RECV-INFO ===\n");
	RTW_PRINT_SEL(sel, "NR_RECVFRAME = %d\n", NR_RECVFRAME);
	/*
	RTW_PRINT_SEL(sel, "NR_RECVBUFF = %d\n", NR_RECVBUFF);
	RTW_PRINT_SEL(sel, "MAX_RECVBUF_SZ = %d\n", MAX_RECVBUF_SZ);
	*/

	#ifdef CONFIG_RTW_PERSIST_IF
	RTW_PRINT_SEL(sel, "CONFIG_RTW_PERSIST_IF = %d\n", CONFIG_RTW_PERSIST_IF);
	#endif
	#ifdef CONFIG_RTW_CLIENT_MODE_SUPPORT
	RTW_PRINT_SEL(sel, "CONFIG_RTW_CLIENT_MODE_SUPPORT = %d\n", CONFIG_RTW_CLIENT_MODE_SUPPORT);
	#endif
	#ifdef CONFIG_RTK_WFO
	RTW_PRINT_SEL(sel, "CONFIG_RTK_WFO = %d\n", CONFIG_RTK_WFO);
	#endif
}

void dump_log_level(void *sel)
{
#ifdef CONFIG_RTW_DEBUG
	int i;

	RTW_PRINT_SEL(sel, "drv_log_level:%d phl_log_level:%d\n", rtw_drv_log_level, phl_log_level);
	for (i = 0; i <= _DRV_MAX_; i++) {
		if (rtw_log_level_str[i])
			RTW_PRINT_SEL(sel, "%c %s = %d\n",
				(rtw_drv_log_level == i) ? '+' : ' ', rtw_log_level_str[i], i);
	}
	RTW_PRINT_SEL(sel, "\n");
	for (i = 0; i <= _PHL_MAX_; i++) {
		if(phl_log_level_str[i])
			RTW_PRINT_SEL(sel, "%c %s = %d\n",
				(phl_log_level == i) ? '+' : ' ', phl_log_level_str[i], i);
	}
#else
	RTW_PRINT_SEL(sel, "CONFIG_RTW_DEBUG is disabled\n");
#endif
}


void rtw_sink_rtp_seq_dbg(_adapter *adapter, u8 *ehdr_pos)
{
	struct recv_priv *precvpriv = &(adapter->recvpriv);
	if (precvpriv->sink_udpport > 0) {
		if (*((u16 *)(ehdr_pos + 0x24)) == cpu_to_be16(precvpriv->sink_udpport)) {
			precvpriv->pre_rtp_rxseq = precvpriv->cur_rtp_rxseq;
			precvpriv->cur_rtp_rxseq = be16_to_cpu(*((u16 *)(ehdr_pos + 0x2C)));
			if (precvpriv->pre_rtp_rxseq + 1 != precvpriv->cur_rtp_rxseq) {
				if(precvpriv->pre_rtp_rxseq == 65535 ) {
					if( precvpriv->cur_rtp_rxseq != 0) {
						RTW_INFO("%s : RTP Seq num from %d to %d\n", __FUNCTION__, precvpriv->pre_rtp_rxseq, precvpriv->cur_rtp_rxseq);
					}
				} else {
					RTW_INFO("%s : RTP Seq num from %d to %d\n", __FUNCTION__, precvpriv->pre_rtp_rxseq, precvpriv->cur_rtp_rxseq);
				}
			}
		}
	}
}

void sta_rx_reorder_ctl_dump(void *sel, struct sta_info *sta)
{
	struct recv_reorder_ctrl *reorder_ctl;
	int i;

	for (i = 0; i < 16; i++) {
		reorder_ctl = &sta->recvreorder_ctrl[i];
		if (reorder_ctl->ampdu_size != RX_AMPDU_SIZE_INVALID || reorder_ctl->indicate_seq != 0xFFFF) {
			RTW_PRINT_SEL(sel, "tid=%d, enable=%d, ampdu_size=%u, indicate_seq=%u\n"
				, i, reorder_ctl->enable, reorder_ctl->ampdu_size, reorder_ctl->indicate_seq
				     );
		}
	}
}

void dump_tx_rate_bmp(void *sel, struct dvobj_priv *dvobj)
{
	_adapter *adapter = dvobj_get_primary_adapter(dvobj);
	struct rf_ctl_t *rfctl = dvobj_to_rfctl(dvobj);
	u8 bw;

	RTW_PRINT_SEL(sel, "%-6s", "bw");
	if (rtw_hw_chk_proto_cap(dvobj, PROTO_CAP_11AC))
		_RTW_PRINT_SEL(sel, " %-15s", "vht");

	_RTW_PRINT_SEL(sel, " %-11s %-4s %-3s\n", "ht", "ofdm", "cck");

	for (bw = CHANNEL_WIDTH_20; bw <= CHANNEL_WIDTH_160; bw++) {
		if (!rtw_hw_is_band_support(adapter_to_dvobj(adapter), bw))
			continue;

		RTW_PRINT_SEL(sel, "%6s", ch_width_str(bw));
		if (rtw_hw_chk_proto_cap(dvobj, PROTO_CAP_11AC)) {
			_RTW_PRINT_SEL(sel, " %03x %03x %03x %03x"
				, RATE_BMP_GET_VHT_4SS(rfctl->rate_bmp_vht_by_bw[bw])
				, RATE_BMP_GET_VHT_3SS(rfctl->rate_bmp_vht_by_bw[bw])
				, RATE_BMP_GET_VHT_2SS(rfctl->rate_bmp_vht_by_bw[bw])
				, RATE_BMP_GET_VHT_1SS(rfctl->rate_bmp_vht_by_bw[bw])
			);
		}

		_RTW_PRINT_SEL(sel, " %02x %02x %02x %02x"
			, bw <= CHANNEL_WIDTH_40 ? RATE_BMP_GET_HT_4SS(rfctl->rate_bmp_ht_by_bw[bw]) : 0
			, bw <= CHANNEL_WIDTH_40 ? RATE_BMP_GET_HT_3SS(rfctl->rate_bmp_ht_by_bw[bw]) : 0
			, bw <= CHANNEL_WIDTH_40 ? RATE_BMP_GET_HT_2SS(rfctl->rate_bmp_ht_by_bw[bw]) : 0
			, bw <= CHANNEL_WIDTH_40 ? RATE_BMP_GET_HT_1SS(rfctl->rate_bmp_ht_by_bw[bw]) : 0
		);

		_RTW_PRINT_SEL(sel, "  %03x   %01x\n"
			, bw <= CHANNEL_WIDTH_20 ? RATE_BMP_GET_OFDM(rfctl->rate_bmp_cck_ofdm) : 0
			, bw <= CHANNEL_WIDTH_20 ? RATE_BMP_GET_CCK(rfctl->rate_bmp_cck_ofdm) : 0
		);
	}
}

void dump_adapters_status(void *sel, struct dvobj_priv *dvobj)
{
	struct rf_ctl_t *rfctl = dvobj_to_rfctl(dvobj);
	int i;
	_adapter *iface;
	u8 u_ch, u_bw, u_offset;
	dump_mi_status(sel, dvobj);

	RTW_PRINT_SEL(sel, "dev status:%s%s\n\n"
		, dev_is_surprise_removed(dvobj) ? " SR" : ""
		, dev_is_drv_stopped(dvobj) ? " DS" : ""
	);

#ifdef CONFIG_P2P
#define P2P_INFO_TITLE_FMT	" %-3s %-4s"
#define P2P_INFO_TITLE_ARG	, "lch", "p2ps"
#ifdef CONFIG_IOCTL_CFG80211
#define P2P_INFO_VALUE_FMT	" %3u %c%3u"
#define P2P_INFO_VALUE_ARG	, iface->wdinfo.listen_channel, iface->wdev_data.p2p_enabled ? 'e' : ' ', rtw_p2p_state(&iface->wdinfo)
#else
#define P2P_INFO_VALUE_FMT	" %3u %4u"
#define P2P_INFO_VALUE_ARG	, iface->wdinfo.listen_channel, rtw_p2p_state(&iface->wdinfo)
#endif
#define P2P_INFO_DASH		"---------"
#else
#define P2P_INFO_TITLE_FMT	""
#define P2P_INFO_TITLE_ARG
#define P2P_INFO_VALUE_FMT	""
#define P2P_INFO_VALUE_ARG
#define P2P_INFO_DASH
#endif

#define INFO_FMT	""
#define INFO_ARG
#define INFO_CNT_FMT	""
#define INFO_CNT_ARG


	RTW_PRINT_SEL(sel, "%-2s %-15s %c %-3s %-17s %-4s %-7s %-5s"
		P2P_INFO_TITLE_FMT
		" %s"INFO_FMT"\n"
		, "id", "ifname", ' ', "netif_up", "macaddr", "port", "ch", "class"
		P2P_INFO_TITLE_ARG
		, "status"INFO_ARG);

	RTW_PRINT_SEL(sel, "---------------------------------------------------------------"
		P2P_INFO_DASH
		"-------\n");

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if (iface) {
			RTW_PRINT_SEL(sel, "%2d %-15s %c %3u "MAC_FMT" %4hhu %3u,%u,%u %5u"
				P2P_INFO_VALUE_FMT
				" "MLME_STATE_FMT" " INFO_CNT_FMT"\n"
				, i, iface->registered ? ADPT_ARG(iface) : NULL
				, iface->registered ? 'R' : ' '
				, iface->netif_up
				, MAC_ARG(adapter_mac_addr(iface))
				, rtw_hal_get_port(iface)
				, iface->mlmeextpriv.cur_channel
				, iface->mlmeextpriv.cur_bwmode
				, iface->mlmeextpriv.cur_ch_offset
				, rtw_get_op_class_by_chbw(iface->mlmeextpriv.cur_channel
					, iface->mlmeextpriv.cur_bwmode
					, iface->mlmeextpriv.cur_ch_offset)
				P2P_INFO_VALUE_ARG
				, MLME_STATE_ARG(iface)
				INFO_CNT_ARG
			);
		}
	}

	RTW_PRINT_SEL(sel, "---------------------------------------------------------------"
		P2P_INFO_DASH
		"-------\n");

	if (rtw_mi_get_ch_setting_union(dvobj_get_primary_adapter(dvobj), &u_ch, &u_bw, &u_offset))
		RTW_PRINT_SEL(sel, "%55s %3u,%u,%u %5u\n"
			, "union:"
			, u_ch, u_bw, u_offset, rtw_get_op_class_by_chbw(u_ch, u_bw, u_offset));

	RTW_PRINT_SEL(sel, "%55s %3u,%u,%u offch_state:%d\n"
		, "oper:"
		, dvobj->oper_channel
		, dvobj->oper_bwmode
		, dvobj->oper_ch_offset
		, rfctl->offch_state
	);

#ifdef CONFIG_DFS_MASTER
	if (rfctl->radar_detect_ch != 0) {
		RTW_PRINT_SEL(sel, "%55s %3u,%u,%u"
			, "radar_detect:"
			, rfctl->radar_detect_ch
			, rfctl->radar_detect_bw
			, rfctl->radar_detect_offset
		);

		if (rfctl->radar_detect_by_others)
			_RTW_PRINT_SEL(sel, ", by AP of STA link");
		else {
			u32 non_ocp_ms;
			u32 cac_ms;
			u8 dfs_domain = rtw_dfs_get_region_domain(dvobj);

			_RTW_PRINT_SEL(sel, ", dfs_domain:%u", dfs_domain);

			rtw_get_ch_waiting_ms(rfctl
				, rfctl->radar_detect_ch
				, rfctl->radar_detect_bw
				, rfctl->radar_detect_offset
				, &non_ocp_ms
				, &cac_ms
			);

			if (non_ocp_ms)
				_RTW_PRINT_SEL(sel, ", non_ocp:%d", non_ocp_ms);
			if (cac_ms)
				_RTW_PRINT_SEL(sel, ", cac:%d", cac_ms);
		}

		_RTW_PRINT_SEL(sel, "\n");
	}
#endif /* CONFIG_DFS_MASTER */
}

#define SEC_CAM_ENT_ID_TITLE_FMT "%-2s"
#define SEC_CAM_ENT_ID_TITLE_ARG "id"
#define SEC_CAM_ENT_ID_VALUE_FMT "%2u"
#define SEC_CAM_ENT_ID_VALUE_ARG(id) (id)

#define SEC_CAM_ENT_TITLE_FMT "%-6s %-17s %-32s %-3s %-8s %-2s %-2s %-5s"
#define SEC_CAM_ENT_TITLE_ARG "ctrl", "addr", "key", "kid", "type", "MK", "GK", "valid"
#define SEC_CAM_ENT_VALUE_FMT "0x%04x "MAC_FMT" "KEY_FMT" %3u %-8s %2u %2u %5u"
#define SEC_CAM_ENT_VALUE_ARG(ent) \
	(ent)->ctrl \
	, MAC_ARG((ent)->mac) \
	, KEY_ARG((ent)->key) \
	, ((ent)->ctrl) & 0x03 \
	, (((ent)->ctrl) & 0x200) ? \
	security_type_str((((ent)->ctrl) >> 2 & 0x7) | _SEC_TYPE_256_) : \
	security_type_str(((ent)->ctrl) >> 2 & 0x7) \
	, (((ent)->ctrl) >> 5) & 0x01 \
	, (((ent)->ctrl) >> 6) & 0x01 \
	, (((ent)->ctrl) >> 15) & 0x01

void dump_sec_cam_ent(void *sel, struct sec_cam_ent *ent, int id)
{
	if (id >= 0) {
		RTW_PRINT_SEL(sel, SEC_CAM_ENT_ID_VALUE_FMT " " SEC_CAM_ENT_VALUE_FMT"\n"
			, SEC_CAM_ENT_ID_VALUE_ARG(id), SEC_CAM_ENT_VALUE_ARG(ent));
	} else
		RTW_PRINT_SEL(sel, SEC_CAM_ENT_VALUE_FMT"\n", SEC_CAM_ENT_VALUE_ARG(ent));
}

void dump_sec_cam_ent_title(void *sel, u8 has_id)
{
	if (has_id) {
		RTW_PRINT_SEL(sel, SEC_CAM_ENT_ID_TITLE_FMT " " SEC_CAM_ENT_TITLE_FMT"\n"
			, SEC_CAM_ENT_ID_TITLE_ARG, SEC_CAM_ENT_TITLE_ARG);
	} else
		RTW_PRINT_SEL(sel, SEC_CAM_ENT_TITLE_FMT"\n", SEC_CAM_ENT_TITLE_ARG);
}

void dump_sec_cam(void *sel, _adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct cam_ctl_t *cam_ctl = &dvobj->cam_ctl;
	struct sec_cam_ent ent;
	int i;

	_rtw_memset(&ent, 0, sizeof(ent));
	RTW_PRINT_SEL(sel, "HW sec cam:\n");
	dump_sec_cam_ent_title(sel, 1);
	for (i = 0; i < cam_ctl->num; i++) {
		rtw_hal_sec_read_cam_ent(adapter, i, (u8 *)(&ent.ctrl), ent.mac, ent.key);
		dump_sec_cam_ent(sel , &ent, i);
	}
}

void dump_sec_cam_cache(void *sel, _adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct cam_ctl_t *cam_ctl = &dvobj->cam_ctl;
	int i;

	RTW_PRINT_SEL(sel, "SW sec cam cache:\n");
	dump_sec_cam_ent_title(sel, 1);
	for (i = 0; i < cam_ctl->num; i++) {
		if (dvobj->cam_cache[i].ctrl != 0)
			dump_sec_cam_ent(sel, &dvobj->cam_cache[i], i);
	}

}

static u8 del_rx_ampdu_test_no_tx_fail = 0;

bool rtw_del_rx_ampdu_test_trigger_no_tx_fail(void)
{
	if (del_rx_ampdu_test_no_tx_fail) {
		RTW_PRINT("del_rx_ampdu test case: trigger no_tx_fail\n");
		del_rx_ampdu_test_no_tx_fail--;
		return _TRUE;
	}
	return _FALSE;
}

static u32 g_wait_hiq_empty_ms = 0;

u32 rtw_get_wait_hiq_empty_ms(void)
{
	return g_wait_hiq_empty_ms;
}

static systime sta_linking_test_start_time = 0;
static u32 sta_linking_test_wait_ms = 0;
static u8 sta_linking_test_force_fail = 0;

void rtw_sta_linking_test_set_start(void)
{
	sta_linking_test_start_time = rtw_get_current_time();
}

bool rtw_sta_linking_test_wait_done(void)
{
	return rtw_get_passing_time_ms(sta_linking_test_start_time) >= sta_linking_test_wait_ms;
}

bool rtw_sta_linking_test_force_fail(void)
{
	return sta_linking_test_force_fail;
}

#ifdef CONFIG_AP_MODE
static u16 ap_linking_test_force_auth_fail = 0;
static u16 ap_linking_test_force_asoc_fail = 0;

u16 rtw_ap_linking_test_force_auth_fail(void)
{
	return ap_linking_test_force_auth_fail;
}

u16 rtw_ap_linking_test_force_asoc_fail(void)
{
	return ap_linking_test_force_asoc_fail;
}
#endif

#ifdef CONFIG_RTW_MIRROR_DUMP
void rtw_pkt_mirror_dump(_adapter *adapter, void *pkt, bool tx)
{
#define PKT_MD_TX		BIT(4)
#define PKT_MD_RX		BIT(5)
	struct sk_buff *skb = (struct sk_buff *)pkt;
	void *phl = adapter->dvobj->phl;
	u32 msk = 0;

	if (tx)
		msk = PKT_MD_TX;
	else
		msk = PKT_MD_RX;

	if (adapter->registrypriv.wifi_mib.mirror_dump & msk)
		rtw_phl_mirror_dump_buf(phl, _PHL_MIR_CAT_PKT, skb->data, skb->len);
#undef PKT_MD_TX
#undef PKT_MD_RX
}
#endif

int proc_get_defs_param(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;

	RTW_PRINT_SEL(m, "%s %15s\n", "lmt_sta", "lmt_time");
	RTW_PRINT_SEL(m, "%-15u %-15u\n"
		, mlme->defs_lmt_sta
		, mlme->defs_lmt_time
	);

	return 0;
}

ssize_t proc_set_defs_param(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;

	char tmp[32];
	u32 defs_lmt_sta;
	u32 defs_lmt_time;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%u %u", &defs_lmt_sta, &defs_lmt_time);

		if (num >= 1)
			mlme->defs_lmt_sta = defs_lmt_sta;
		if (num >= 2)
			mlme->defs_lmt_time = defs_lmt_time;
	}

	return count;

}

ssize_t proc_set_write_reg(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	char tmp[32];
	u32 addr, val, len;

	if (count < 3) {
		RTW_INFO("argument size is less than 3\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%x %x %x", &addr, &val, &len);

		if (num !=  3) {
			RTW_INFO("invalid write_reg parameter!\n");
			return count;
		}

		switch (len) {
		case 1:
			rtw_phl_write8(dvobj->phl, addr, (u8)val);
			break;
		case 2:
			if (addr & 0x1) {
				RTW_INFO("Unaligned reg_write16(0x%x)\n", addr);
				break;
			}
			rtw_phl_write16(dvobj->phl, addr, (u16)val);
			break;
		case 4:
			if (addr & 0x3) {
				RTW_INFO("Unaligned reg_write32(0x%x)\n", addr);
				break;
			}
			rtw_phl_write32(dvobj->phl, addr, val);
			break;
		default:
			RTW_INFO("error write length=%d", len);
			break;
		}

	}
	return count;

}

static u32 proc_get_read_addr = 0xeeeeeeee;
static u32 proc_get_read_len = 0x4;

int proc_get_read_reg(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if (proc_get_read_addr == 0xeeeeeeee) {
		RTW_PRINT_SEL(m, "address not initialized\n");
		return 0;
	}

	switch (proc_get_read_len) {
	case 1:
		RTW_PRINT_SEL(m, "reg_read8(0x%x)=0x%02x\n", proc_get_read_addr,
			rtw_phl_read8(dvobj->phl, proc_get_read_addr));
		break;
	case 2:
		if (proc_get_read_addr & 0x1) {
			RTW_PRINT_SEL(m, "Unaligned reg_read16(0x%x)\n", proc_get_read_addr);
			break;
		}
		RTW_PRINT_SEL(m, "reg_read16(0x%x)=0x%04x\n", proc_get_read_addr,
			rtw_phl_read16(dvobj->phl, proc_get_read_addr));
		break;
	case 4:
		if (proc_get_read_addr & 0x3) {
			RTW_PRINT_SEL(m, "Unaligned reg_read32(0x%x)\n", proc_get_read_addr);
			break;
		}
		RTW_PRINT_SEL(m, "reg_read32(0x%x)=0x%08x\n", proc_get_read_addr,
			rtw_phl_read32(dvobj->phl, proc_get_read_addr));
		break;
	default:
		RTW_PRINT_SEL(m, "error read length=%d\n", proc_get_read_len);
		break;
	}

	return 0;
}

ssize_t proc_set_read_reg(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[16];
	u32 addr, len;

	if (count < 2) {
		RTW_INFO("argument size is less than 2\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%x %x", &addr, &len);

		if (num !=  2) {
			RTW_INFO("invalid read_reg parameter!\n");
			return count;
		}

		proc_get_read_addr = addr;

		proc_get_read_len = len;
	}

	return count;

}

ssize_t proc_set_mac_dbg_status_dump(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	char tmp[32];
	u32 val[2];

	if (count < 1) {
		RTW_ERR("argument size is less than 1\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		u32 en;
		u8 en_u8;
		int num = sscanf(tmp, "%x", &en);

		/* Always use u8 to call rtw_phl_mac_dbg_status_dump to
		 * prevent from endianess issue.
		 */
		en_u8 = en;

		if (num !=  1) {
			RTW_ERR("invalid mac_dbg_status_dump parameter!\n");
			return count;
		}

		rtw_phl_mac_dbg_status_dump(dvobj->phl, (u32 *)&val, (u8 *)&en_u8);

		if (en & BIT0)
			RTW_PRINT_DUMP("ss_dbgpkg", (const u8 *)val, 16);
	}
	return count;
}

#ifdef CONFIG_PROC_DEBUG

int proc_get_rx_stat(struct seq_file *m, void *v)
{
	_list	*plist, *phead;
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_info *psta = NULL;
	struct stainfo_stats	*pstats = NULL;
	struct sta_priv		*pstapriv = &(adapter->stapriv);
	u32 i, j;
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 null_addr[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);
			pstats = &psta->sta_stats;

			if (pstats == NULL)
				continue;
			if ((_rtw_memcmp(psta->phl_sta->mac_addr, bc_addr, ETH_ALEN) !=  _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, null_addr, ETH_ALEN) != _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, adapter_mac_addr(adapter), ETH_ALEN) != _TRUE)) {
				RTW_PRINT_SEL(m, "MAC :\t\t"MAC_FMT "\n", MAC_ARG(psta->phl_sta->mac_addr));
				RTW_PRINT_SEL(m, "data_rx_cnt :\t%llu\n", sta_rx_data_uc_pkts(psta) - pstats->last_rx_data_uc_pkts);
				pstats->last_rx_data_uc_pkts = sta_rx_data_uc_pkts(psta);
				RTW_PRINT_SEL(m, "duplicate_cnt :\t%u\n", pstats->duplicate_cnt);
				pstats->duplicate_cnt = 0;
				RTW_PRINT_SEL(m, "rx_per_rate_cnt :\n");

				for (j = 0; j < RTW_DATA_RATE_MAX; j++) {
					RTW_PRINT_SEL(m, "%08u  ", pstats->rxratecnt[j]);
					pstats->rxratecnt[j] = 0;
					if ((j%8) == 7)
						RTW_PRINT_SEL(m, "\n");
				}
				RTW_PRINT_SEL(m, "\n");
			}
		}
	}
	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);
	return 0;
}

int proc_get_tx_stat(struct seq_file *m, void *v)
{
	_list	*plist, *phead;
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_info *psta = NULL;
	u8 sta_mac[NUM_STA][ETH_ALEN] = {{0}};
	uint mac_id[NUM_STA];
	struct stainfo_stats	*pstats = NULL;
	struct sta_priv	*pstapriv = &(adapter->stapriv);
	struct sta_priv	*pstapriv_primary = &(GET_PRIMARY_ADAPTER(adapter))->stapriv;
	u32 i, macid_rec_idx = 0;
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 null_addr[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	struct submit_ctx gotc2h;

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);
			if ((_rtw_memcmp(psta->phl_sta->mac_addr, bc_addr, ETH_ALEN) !=  _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, null_addr, ETH_ALEN) != _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, adapter_mac_addr(adapter), ETH_ALEN) != _TRUE)) {
				_rtw_memcpy(&sta_mac[macid_rec_idx][0], psta->phl_sta->mac_addr, ETH_ALEN);
				mac_id[macid_rec_idx] = psta->phl_sta->macid;
				macid_rec_idx++;
			}
		}
	}
	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);
	for (i = 0; i < macid_rec_idx; i++) {
		_rtw_memcpy(pstapriv_primary->c2h_sta_mac, &sta_mac[i][0], ETH_ALEN);
		pstapriv_primary->c2h_adapter_id = adapter->iface_id;
		rtw_sctx_init(&gotc2h, 60);
		pstapriv_primary->gotc2h = &gotc2h;
		rtw_hal_reqtxrpt(adapter, mac_id[i]);
		if (rtw_sctx_wait(&gotc2h, __func__)) {
			psta = rtw_get_stainfo(pstapriv, &sta_mac[i][0]);
			if(psta) {
				pstats = &psta->sta_stats;
#ifndef ROKU_PRIVATE
				RTW_PRINT_SEL(m, "data_sent_cnt :\t%u\n", pstats->tx_ok_cnt + pstats->tx_fail_cnt);
				RTW_PRINT_SEL(m, "success_cnt :\t%u\n", pstats->tx_ok_cnt);
				RTW_PRINT_SEL(m, "failure_cnt :\t%u\n", pstats->tx_fail_cnt);
				RTW_PRINT_SEL(m, "retry_cnt :\t%u\n\n", pstats->tx_retry_cnt);
#else
				RTW_PRINT_SEL(m, "MAC: " MAC_FMT " sent: %u fail: %u retry: %u\n",
				MAC_ARG(&sta_mac[i][0]), pstats->tx_ok_cnt, pstats->tx_fail_cnt, pstats->tx_retry_cnt);
#endif /* ROKU_PRIVATE */

			} else
				RTW_PRINT_SEL(m, "STA is gone\n");
		} else {
			//to avoid c2h modify counters
			pstapriv_primary->gotc2h = NULL;
			_rtw_memset(pstapriv_primary->c2h_sta_mac, 0, ETH_ALEN);
			pstapriv_primary->c2h_adapter_id = CONFIG_IFACE_NUMBER;
			RTW_PRINT_SEL(m, "Warming : Query timeout, operation abort!!\n");
			break;
		}
		pstapriv_primary->gotc2h = NULL;
		_rtw_memset(pstapriv_primary->c2h_sta_mac, 0, ETH_ALEN);
		pstapriv_primary->c2h_adapter_id = CONFIG_IFACE_NUMBER;
	}
	return 0;
}

int proc_get_fwstate(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	RTW_PRINT_SEL(m, "fwstate=0x%x\n", get_fwstate(pmlmepriv));

	return 0;
}

int proc_get_sec_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct security_priv *sec = &padapter->securitypriv;

	RTW_PRINT_SEL(m, "auth_alg=0x%x, enc_alg=0x%x, auth_type=0x%x, enc_type=0x%x\n",
		sec->dot11AuthAlgrthm, sec->dot11PrivacyAlgrthm,
		sec->ndisauthtype, sec->ndisencryptstatus);

	RTW_PRINT_SEL(m, "hw_decrypted=%d\n", sec->hw_decrypted);
	RTW_PRINT_SEL(m, "wpa_psk=");
	if(sec->wpa_psk & BIT(0))
		RTW_PRINT_SEL(m, "WPA ");
	if(sec->wpa_psk & BIT(1))
		RTW_PRINT_SEL(m, "WPA2 ");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "wpa2_group_cipher=%d\n", sec->wpa2_group_cipher);
	RTW_PRINT_SEL(m, "wpa2_pairwise_cipher=%d\n", sec->wpa2_pairwise_cipher);
	RTW_PRINT_SEL(m, "wpa_group_cipher=%d\n", sec->wpa_group_cipher);
	RTW_PRINT_SEL(m, "wpa_pairwise_cipher=%d\n", sec->wpa_pairwise_cipher);
	RTW_PRINT_SEL(m, "80211W_enable=%d\n", sec->mfp_opt);
	RTW_PRINT_SEL(m, "akm_suite_bitmap=0x%x\n", sec->akmp);

#ifdef DBG_SW_SEC_CNT
	RTW_PRINT_SEL(m, "==sw counters(bc, mc, uc)==\n");
	RTW_PRINT_SEL(m, "wep_sw_enc_cnt=%llu, %llu, %llu\n"
		, sec->wep_sw_enc_cnt_bc , sec->wep_sw_enc_cnt_mc, sec->wep_sw_enc_cnt_uc);
	RTW_PRINT_SEL(m, "wep_sw_dec_cnt=%llu, %llu, %llu\n"
		, sec->wep_sw_dec_cnt_bc , sec->wep_sw_dec_cnt_mc, sec->wep_sw_dec_cnt_uc);

	RTW_PRINT_SEL(m, "tkip_sw_enc_cnt=%llu, %llu, %llu\n"
		, sec->tkip_sw_enc_cnt_bc , sec->tkip_sw_enc_cnt_mc, sec->tkip_sw_enc_cnt_uc);
	RTW_PRINT_SEL(m, "tkip_sw_dec_cnt=%llu, %llu, %llu\n"
		, sec->tkip_sw_dec_cnt_bc , sec->tkip_sw_dec_cnt_mc, sec->tkip_sw_dec_cnt_uc);

	RTW_PRINT_SEL(m, "aes_sw_enc_cnt=%llu, %llu, %llu\n"
		, sec->aes_sw_enc_cnt_bc , sec->aes_sw_enc_cnt_mc, sec->aes_sw_enc_cnt_uc);
	RTW_PRINT_SEL(m, "aes_sw_dec_cnt=%llu, %llu, %llu\n"
		, sec->aes_sw_dec_cnt_bc , sec->aes_sw_dec_cnt_mc, sec->aes_sw_dec_cnt_uc);

	RTW_PRINT_SEL(m, "gcmp_sw_enc_cnt=%llu, %llu, %llu\n"
		, sec->gcmp_sw_enc_cnt_bc , sec->gcmp_sw_enc_cnt_mc, sec->gcmp_sw_enc_cnt_uc);
	RTW_PRINT_SEL(m, "gcmp_sw_dec_cnt=%llu, %llu, %llu\n"
		, sec->gcmp_sw_dec_cnt_bc , sec->gcmp_sw_dec_cnt_mc, sec->gcmp_sw_dec_cnt_uc);
#endif /* DBG_SW_SEC_CNT */

	return 0;
}

int proc_get_mlmext_state(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	RTW_PRINT_SEL(m, "pmlmeinfo->state=0x%x\n", pmlmeinfo->state);

	return 0;
}

#ifdef CONFIG_LAYER2_ROAMING
int proc_get_roam_flags(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "0x%02x\n", rtw_roam_flags(adapter));

	return 0;
}

ssize_t proc_set_roam_flags(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	char tmp[32];
	u8 flags;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &flags);

		if (num == 1)
			rtw_assign_roam_flags(adapter, flags);
	}

	return count;

}

int proc_get_roam_param(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;

	RTW_PRINT_SEL(m, "%12s %15s %26s %16s\n", "rssi_diff_th", "scanr_exp_ms", "scan_interval(unit:2 sec)", "rssi_threshold");
	RTW_PRINT_SEL(m, "%-15u %-13u %-27u %-11u\n"
		, mlme->roam_rssi_diff_th
		, mlme->roam_scanr_exp_ms
		, mlme->roam_scan_int
		, mlme->roam_rssi_threshold
	);

	return 0;
}

ssize_t proc_set_roam_param(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlme = &adapter->mlmepriv;

	char tmp[32];
	u8 rssi_diff_th;
	u32 scanr_exp_ms;
	u32 scan_int;
	u8 rssi_threshold;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %u %u %hhu", &rssi_diff_th, &scanr_exp_ms, &scan_int, &rssi_threshold);

		if (num >= 1)
			mlme->roam_rssi_diff_th = rssi_diff_th;
		if (num >= 2)
			mlme->roam_scanr_exp_ms = scanr_exp_ms;
		if (num >= 3)
			mlme->roam_scan_int = scan_int;
		if (num >= 4)
			mlme->roam_rssi_threshold = rssi_threshold;
	}

	return count;

}

ssize_t proc_set_roam_tgt_addr(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	char tmp[32];
	u8 addr[ETH_ALEN];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", addr, addr + 1, addr + 2, addr + 3, addr + 4, addr + 5);
		if (num == 6)
			_rtw_memcpy(adapter->mlmepriv.roam_tgt_addr, addr, ETH_ALEN);

		RTW_INFO("set roam_tgt_addr to "MAC_FMT"\n", MAC_ARG(adapter->mlmepriv.roam_tgt_addr));
	}

	return count;
}
#endif /* CONFIG_LAYER2_ROAMING */

#ifdef CONFIG_RTW_80211R
ssize_t proc_set_ft_flags(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	char tmp[32];
	u8 flags;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%hhx", &flags);

		if (num == 1)
			adapter->mlmepriv.ft_roam.ft_flags = flags;
	}

	return count;

}

int proc_get_ft_flags(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "0x%02x\n", adapter->mlmepriv.ft_roam.ft_flags);

	return 0;
}
#endif

int proc_get_qos_option(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	RTW_PRINT_SEL(m, "qos_option=%d\n", pmlmepriv->qospriv.qos_option);

	return 0;
}

int proc_get_ht_option(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

#ifdef CONFIG_80211N_HT
	RTW_PRINT_SEL(m, "ht_option=%d\n", pmlmepriv->htpriv.ht_option);
#endif /* CONFIG_80211N_HT */

	return 0;
}

int proc_get_rf_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	RTW_PRINT_SEL(m, "cur_ch=%d, cur_bw=%d, cur_ch_offet=%d\n",
		pmlmeext->cur_channel, pmlmeext->cur_bwmode, pmlmeext->cur_ch_offset);

	RTW_PRINT_SEL(m, "oper_ch=%d, oper_bw=%d, oper_ch_offet=%d\n",
		rtw_get_oper_ch(padapter), rtw_get_oper_bw(padapter),  rtw_get_oper_choffset(padapter));

	return 0;
}

int proc_get_scan_param(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	struct ss_res *ss = &mlmeext->sitesurvey_res;

#define SCAN_PARAM_TITLE_FMT "%10s"
#define SCAN_PARAM_VALUE_FMT "%-10u"
#define SCAN_PARAM_TITLE_ARG , "scan_ch_ms"
#define SCAN_PARAM_VALUE_ARG , ss->scan_ch_ms
#ifdef CONFIG_80211N_HT
#define SCAN_PARAM_TITLE_FMT_HT " %15s %13s"
#define SCAN_PARAM_VALUE_FMT_HT " %-15u %-13u"
#define SCAN_PARAM_TITLE_ARG_HT , "rx_ampdu_accept", "rx_ampdu_size"
#define SCAN_PARAM_VALUE_ARG_HT , ss->rx_ampdu_accept, ss->rx_ampdu_size
#else
#define SCAN_PARAM_TITLE_FMT_HT ""
#define SCAN_PARAM_VALUE_FMT_HT ""
#define SCAN_PARAM_TITLE_ARG_HT
#define SCAN_PARAM_VALUE_ARG_HT
#endif
#ifdef CONFIG_SCAN_BACKOP
#define SCAN_PARAM_TITLE_FMT_BACKOP " %9s %12s"
#define SCAN_PARAM_VALUE_FMT_BACKOP " %-9u %-12u"
#define SCAN_PARAM_TITLE_ARG_BACKOP , "backop_ms", "scan_cnt_max"
#define SCAN_PARAM_VALUE_ARG_BACKOP , ss->backop_ms, ss->scan_cnt_max
#else
#define SCAN_PARAM_TITLE_FMT_BACKOP ""
#define SCAN_PARAM_VALUE_FMT_BACKOP ""
#define SCAN_PARAM_TITLE_ARG_BACKOP
#define SCAN_PARAM_VALUE_ARG_BACKOP
#endif

	RTW_PRINT_SEL(m,
		SCAN_PARAM_TITLE_FMT
		SCAN_PARAM_TITLE_FMT_HT
		SCAN_PARAM_TITLE_FMT_BACKOP
		"\n"
		SCAN_PARAM_TITLE_ARG
		SCAN_PARAM_TITLE_ARG_HT
		SCAN_PARAM_TITLE_ARG_BACKOP
	);

	RTW_PRINT_SEL(m,
		SCAN_PARAM_VALUE_FMT
		SCAN_PARAM_VALUE_FMT_HT
		SCAN_PARAM_VALUE_FMT_BACKOP
		"\n"
		SCAN_PARAM_VALUE_ARG
		SCAN_PARAM_VALUE_ARG_HT
		SCAN_PARAM_VALUE_ARG_BACKOP
	);

	return 0;
}

ssize_t proc_set_scan_param(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	struct ss_res *ss = &mlmeext->sitesurvey_res;

	char tmp[32] = {0};

	u16 scan_ch_ms;
#define SCAN_PARAM_INPUT_FMT "%hu"
#define SCAN_PARAM_INPUT_ARG , &scan_ch_ms
#ifdef CONFIG_80211N_HT
	u8 rx_ampdu_accept;
	u8 rx_ampdu_size;
#define SCAN_PARAM_INPUT_FMT_HT " %hhu %hhu"
#define SCAN_PARAM_INPUT_ARG_HT , &rx_ampdu_accept, &rx_ampdu_size
#else
#define SCAN_PARAM_INPUT_FMT_HT ""
#define SCAN_PARAM_INPUT_ARG_HT
#endif
#ifdef CONFIG_SCAN_BACKOP
	u16 backop_ms;
	u8 scan_cnt_max;
#define SCAN_PARAM_INPUT_FMT_BACKOP " %hu %hhu"
#define SCAN_PARAM_INPUT_ARG_BACKOP , &backop_ms, &scan_cnt_max
#else
#define SCAN_PARAM_INPUT_FMT_BACKOP ""
#define SCAN_PARAM_INPUT_ARG_BACKOP
#endif

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp,
			SCAN_PARAM_INPUT_FMT
			SCAN_PARAM_INPUT_FMT_HT
			SCAN_PARAM_INPUT_FMT_BACKOP
			SCAN_PARAM_INPUT_ARG
			SCAN_PARAM_INPUT_ARG_HT
			SCAN_PARAM_INPUT_ARG_BACKOP
		);

		if (num-- > 0)
			ss->scan_ch_ms = scan_ch_ms;
#ifdef CONFIG_80211N_HT
		if (num-- > 0)
			ss->rx_ampdu_accept = rx_ampdu_accept;
		if (num-- > 0)
			ss->rx_ampdu_size = rx_ampdu_size;
#endif
#ifdef CONFIG_SCAN_BACKOP
		if (num-- > 0)
			ss->backop_ms = backop_ms;
		if (num-- > 0)
			ss->scan_cnt_max = scan_cnt_max;
#endif
	}

	return count;
}

int proc_get_scan_abort(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u32 pass_ms;

	pass_ms = rtw_scan_abort_timeout(adapter, 10000);

	RTW_PRINT_SEL(m, "%u\n", pass_ms);

	return 0;
}

int proc_get_survey_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &padapter->dvobj->scanned_queue;
#else
	_queue	*queue	= &(pmlmepriv->scanned_queue);
#endif
	struct wlan_network	*pnetwork = NULL;
	_list	*plist, *phead;
	s32 notify_signal;
	s16 notify_noise = 0;
	u16  index = 0, ie_cap = 0;
	unsigned char *ie_wpa = NULL, *ie_wpa2 = NULL, *ie_wps = NULL;
	unsigned char *ie_p2p = NULL, *ssid = NULL;
	char flag_str[64];
	int ielen = 0;
	u32 wpsielen = 0;
#ifdef CONFIG_RTW_MESH
	const char *ssid_title_str = "ssid/mesh_id";
#else
	const char *ssid_title_str = "ssid";
#endif

	_rtw_spinlock_bh(&(queue->lock));
	phead = get_list_head(queue);
	if (!phead)
		goto _exit;
	plist = get_next(phead);
	if (!plist)
		goto _exit;

	RTW_PRINT_SEL(m, "%5s  %-17s  %3s  %-3s  %-4s  %-4s  %5s  %32s  %32s\n", "index", "bssid", "ch", "RSSI", "SdBm", "Noise", "age", "flag", ssid_title_str);
	while (1) {
		if (rtw_end_of_queue_search(phead, plist) == _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (!pnetwork)
			break;

		if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE &&
		    is_same_network(&pmlmepriv->cur_network.network, &pnetwork->network, 0)) {
			notify_signal = translate_percentage_to_dbm(padapter->recvpriv.signal_strength);/* dbm */
		} else {
			notify_signal = translate_percentage_to_dbm(pnetwork->network.PhyInfo.SignalStrength);/* dbm */
		}

#if 0 /*def CONFIG_BACKGROUND_NOISE_MONITOR*/
		if (IS_NM_ENABLE(padapter))
			notify_noise = rtw_noise_query_by_chan_num(padapter, pnetwork->network.Configuration.DSConfig);
#endif

		ie_wpa = rtw_get_wpa_ie(&pnetwork->network.IEs[12], &ielen, pnetwork->network.IELength - 12);
		ie_wpa2 = rtw_get_wpa2_ie(&pnetwork->network.IEs[12], &ielen, pnetwork->network.IELength - 12);
		ie_cap = rtw_get_capability(&pnetwork->network);
		ie_wps = rtw_get_wps_ie(&pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &wpsielen);
		ie_p2p = rtw_get_p2p_ie(&pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &ielen);
		ssid = pnetwork->network.Ssid.Ssid;
		sprintf(flag_str, "%s%s%s%s%s%s%s%s",
			(ie_wpa) ? "[WPA]" : "",
			(ie_wpa2) ? "[WPA2]" : "",
			(!ie_wpa && !ie_wpa2 && ie_cap & BIT(4)) ? "[WEP]" : "",
			(ie_wps) ? "[WPS]" : "",
			(pnetwork->network.InfrastructureMode == Ndis802_11IBSS) ? "[IBSS]" :
				(pnetwork->network.InfrastructureMode == Ndis802_11_mesh) ? "[MESH]" : "",
			(ie_cap & BIT(0)) ? "[ESS]" : "",
			(pnetwork->network.Reserved[0] == BSS_TYPE_BCN) ? "[B]" : \
			(pnetwork->network.Reserved[0] == BSS_TYPE_PROB_RSP) ? "[P]" : "[U]",
			(ie_p2p) ? "[P2P]" : "");
		RTW_PRINT_SEL(m, "%5d  "MAC_FMT"  %3d  %3d  %4d  %4d    %5d  %32s  %32s\n",
			++index,
			MAC_ARG(pnetwork->network.MacAddress),
			pnetwork->network.Configuration.DSConfig,
			(int)pnetwork->network.Rssi,
			notify_signal,
			notify_noise,
			rtw_get_passing_time_ms(pnetwork->last_scanned),
			flag_str,
			pnetwork->network.InfrastructureMode == Ndis802_11_mesh ? pnetwork->network.mesh_id.Ssid : pnetwork->network.Ssid.Ssid
		);
		plist = get_next(plist);
	}
_exit:
	_rtw_spinunlock_bh(&(queue->lock));

	return 0;
}

ssize_t proc_set_survey_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 _status = _FALSE;
	u8 ssc_chk;
	char tmp[32] = {0};
	char cmd[8] = {0};
	bool acs = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%s", cmd);

		if (num < 1)
			return count;

		if (strcmp("acs", cmd) == 0)
			acs = 1;
	}

#if 1
	ssc_chk = rtw_sitesurvey_condition_check(padapter, _FALSE);
	if (ssc_chk != SS_ALLOW)
		goto exit;

	rtw_ps_deny(padapter, PS_DENY_SCAN);
	if (_FAIL == rtw_pwr_wakeup(padapter))
		goto cancel_ps_deny;
	if (!rtw_is_adapter_up(padapter)) {
		RTW_INFO("scan abort!! adapter cannot use\n");
		goto cancel_ps_deny;
	}
#else
#ifdef CONFIG_MP_INCLUDED
	if (rtw_mp_mode_check(padapter)) {
		RTW_INFO("MP mode block Scan request\n");
		goto exit;
	}
#endif
	if (rtw_is_scan_deny(padapter)) {
		RTW_INFO(FUNC_ADPT_FMT  ": scan deny\n", FUNC_ADPT_ARG(padapter));
		goto exit;
	}

	rtw_ps_deny(padapter, PS_DENY_SCAN);
	if (_FAIL == rtw_pwr_wakeup(padapter))
		goto cancel_ps_deny;

	if (!rtw_is_adapter_up(padapter)) {
		RTW_INFO("scan abort!! adapter cannot use\n");
		goto cancel_ps_deny;
	}

	if (rtw_mi_busy_traffic_check(padapter)) {
		RTW_INFO("scan abort!! BusyTraffic == _TRUE\n");
		goto cancel_ps_deny;
	}

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) && check_fwstate(pmlmepriv, WIFI_UNDER_WPS)) {
		RTW_INFO("scan abort!! AP mode process WPS\n");
		goto cancel_ps_deny;
	}
	if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY | WIFI_UNDER_LINKING) == _TRUE) {
		RTW_INFO("scan abort!! fwstate=0x%x\n", pmlmepriv->fw_state);
		goto cancel_ps_deny;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_mi_buddy_check_fwstate(padapter,
		       WIFI_UNDER_SURVEY | WIFI_UNDER_LINKING | WIFI_UNDER_WPS)) {
		RTW_INFO("scan abort!! buddy_fwstate check failed\n");
		goto cancel_ps_deny;
	}
#endif
#endif

	if (acs) {
		#ifdef CONFIG_RTW_ACS
		acs_reset_info(padapter);
		_status = rtw_set_acs_sitesurvey(padapter);
		#endif
	} else
		_status = rtw_set_802_11_bssid_list_scan(padapter, NULL);

cancel_ps_deny:
	rtw_ps_deny_cancel(padapter, PS_DENY_SCAN);
exit:
	return count;
}
#ifdef ROKU_PRIVATE
int proc_get_infra_ap(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct ht_priv_infra_ap *phtpriv = &pmlmepriv->htpriv_infra_ap;
#ifdef CONFIG_80211AC_VHT
	struct vht_priv_infra_ap *pvhtpriv = &pmlmepriv->vhtpriv_infra_ap;
#endif
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &padapter->stapriv;

	if (MLME_IS_STA(padapter)) {
		psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
		if (psta) {
			unsigned int i, j;
			unsigned int Rx_ss = 0, Tx_ss = 0;
			struct recv_reorder_ctrl *preorder_ctrl;

			RTW_PRINT_SEL(m, "SSID=%s\n", pmlmeinfo->network.Ssid.Ssid);
			RTW_PRINT_SEL(m, "sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->phl_sta->mac_addr));
			RTW_PRINT_SEL(m, "Supported rate=");
			for (i = 0; i < NDIS_802_11_LENGTH_RATES_EX; i++) {
				if (pmlmeinfo->SupportedRates_infra_ap[i] == 0)
					break;
				RTW_PRINT_SEL(m, " 0x%x", pmlmeinfo->SupportedRates_infra_ap[i]);
			}
			RTW_PRINT_SEL(m, "\n");
#ifdef CONFIG_80211N_HT
			if (pmlmeinfo->ht_vht_received & BIT(0)) {
				RTW_PRINT_SEL(m, "Supported MCS set=");
				for (i = 0; i < 16 ; i++)
					RTW_PRINT_SEL(m, " 0x%02x",  phtpriv->MCS_set_infra_ap[i]);
				RTW_PRINT_SEL(m, "\n");
				RTW_PRINT_SEL(m, "highest supported data rate=0x%x\n", phtpriv->rx_highest_data_rate_infra_ap);
				RTW_PRINT_SEL(m, "HT_supported_channel_width_set=0x%x\n", phtpriv->channel_width_infra_ap);
				RTW_PRINT_SEL(m, "sgi_20m=%d, sgi_40m=%d\n", phtpriv->sgi_20m_infra_ap, phtpriv->sgi_40m_infra_ap);
				RTW_PRINT_SEL(m, "ldpc_cap=0x%x, stbc_cap=0x%x\n", phtpriv->ldpc_cap_infra_ap, phtpriv->stbc_cap_infra_ap);
				RTW_PRINT_SEL(m, "HT_number_of_stream=%d\n", phtpriv->Rx_ss_infra_ap);
			}
#endif

#ifdef CONFIG_80211AC_VHT
			if (pmlmeinfo->ht_vht_received & BIT(1)) {
				RTW_PRINT_SEL(m, "VHT_supported_channel_width_set=0x%x\n", pvhtpriv->channel_width_infra_ap);
				RTW_PRINT_SEL(m, "vht_ldpc_cap=0x%x, vht_stbc_cap=0x%x, vht_beamform_cap=0x%x\n", pvhtpriv->ldpc_cap_infra_ap, pvhtpriv->stbc_cap_infra_ap, pvhtpriv->beamform_cap_infra_ap);
				RTW_PRINT_SEL(m, "Rx_vht_mcs_map=0x%x, Tx_vht_mcs_map=0x%x\n", *(u16 *)pvhtpriv->vht_mcs_map_infra_ap, *(u16 *)pvhtpriv->vht_mcs_map_tx_infra_ap);
				RTW_PRINT_SEL(m, "VHT_number_of_stream=%d\n", pvhtpriv->number_of_streams_infra_ap);
			}
#endif
		} else
			RTW_PRINT_SEL(m, "can't get sta's macaddr, cur_network's macaddr:" MAC_FMT "\n", MAC_ARG(cur_network->network.MacAddress));
	} else
		RTW_PRINT_SEL(m, "this only applies to STA mode\n");
	return 0;
}

#endif /* ROKU_PRIVATE */

int wireless_mode_to_str(u32 mode, char *str)
{
	str[0]='\0';
	if (mode & WLAN_MD_11A)
		sprintf(str+strlen(str),"%s","A/");
	if (mode & WLAN_MD_11B)
		sprintf(str+strlen(str),"%s","B/");
	if (mode & WLAN_MD_11G)
		sprintf(str+strlen(str),"%s","G/");
	#ifdef CONFIG_80211N_HT
	if (mode & WLAN_MD_11N)
		sprintf(str+strlen(str),"%s","N/");
	#endif
	#ifdef CONFIG_80211AC_VHT
	if (mode & WLAN_MD_11AC)
		sprintf(str+strlen(str),"%s","AC/");
	#endif
	#ifdef CONFIG_80211AX_HE
	if (mode & WLAN_MD_11AX)
		sprintf(str+strlen(str),"%s","AX/");
	#endif
	if (strlen(str)>1)
		str[strlen(str)-1]='\0';

	return strlen(str);
}

void dump_sec_to_str(struct seq_file *m, unsigned int time_in_sec, char* topic)
{
	unsigned int d, n, i;
	int idx = 0;
	char tmp[32];

	d = time_in_sec / 86400;
	n = time_in_sec % 86400;
	if (d)	idx += snprintf(tmp, sizeof(tmp), "%d day ", d);
	d = n / 3600;
	n = n % 3600;
	if (d)	idx += snprintf(tmp+idx, sizeof(tmp)-idx, "%d hr ", d);
	d = n / 60;
	n = n % 60;
	if (d)	idx += snprintf(tmp+idx, sizeof(tmp)-idx, "%d min ", d);
	idx += snprintf(tmp+idx, sizeof(tmp)-idx, "%d sec ", n);

	RTW_PRINT_SEL(m, "%s: %s\n", topic, tmp);
}

#ifdef CONFIG_RTW_MULTI_AP
int proc_get_map_block_list(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	dump_blacklist(m, &(padapter->black_list), "map_block_list");

	return 0;
}
#endif

#ifdef CONFIG_BAND_STEERING
int proc_get_band_steering_block_list(struct seq_file *m, void *v)
{
	u8 i;
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct b_steer_block_entry *ent = NULL;

	if (!rtw_is_adapter_up(padapter))
		return 0;

	RTW_PRINT_SEL(m, "==========================\n");
	RTW_PRINT_SEL(m, "[BAND_STEERING] block_list\n");
	RTW_PRINT_SEL(m, "==========================\n");

	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(padapter->bsteerpriv.block_list[i]);
		if (ent->used) {
			RTW_PRINT_SEL(m, "mac="MAC_FMT"\n", MAC_ARG(ent->mac));
			RTW_PRINT_SEL(m, "entry_expire=%u\n", ent->entry_expire);
			RTW_PRINT_SEL(m, "---------------------------\n");
		}
	}

	return 0;
}
#endif

int proc_get_ap_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rtw_phl_com_t *phl_com = padapter->dvobj->phl_com;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	char wl_mode[16], cur_wl_mode[16];
	u8 tmp_wireless_mode = 0;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_env_report *env_rpt = &(dvobj->env_rpt);

	/* ap vendor */
	char vendor[VENDOR_NAME_LEN] = {0};
	get_assoc_AP_Vendor(vendor,pmlmeinfo->assoc_AP_vendor);
	RTW_PRINT_SEL(m,"AP Vendor %s\n", vendor);
	RTW_PRINT_SEL(m, "ic name: %s\n", phl_com->hal_spec.ic_name);
	dump_sec_to_str(m, padapter->up_time, "Up time");
#ifdef RTW_WKARD_BCNINT_DBG
	RTW_PRINT_SEL(m, "BCN interval: %d (mib: %d)\n",
		pmlmeinfo->bcn_interval,
		GET_PRIMARY_ADAPTER(padapter)->registrypriv.wifi_mib.bcnint);
#endif
	psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
	if (psta) {
		tmp_wireless_mode = pmlmeext->cur_wireless_mode;
		if(tmp_wireless_mode & WLAN_MD_11AX)
		{
			tmp_wireless_mode |= WLAN_MD_11N;
			if(tmp_wireless_mode & WLAN_MD_11A)
				tmp_wireless_mode |= WLAN_MD_11AC;
		}
		else if(tmp_wireless_mode & WLAN_MD_11AC)
			tmp_wireless_mode |= WLAN_MD_11N;
		wireless_mode_to_str(pmibpriv->band, wl_mode);
		wireless_mode_to_str(tmp_wireless_mode, cur_wl_mode);
		RTW_PRINT_SEL(m, "SSID=%s\n", cur_network->network.Ssid.Ssid);
		RTW_PRINT_SEL(m, "sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->phl_sta->mac_addr));
		RTW_PRINT_SEL(m, "cur_channel=%d, cur_bwmode=%d(%s), cur_ch_offset=%d(%s)\n", pmlmeext->cur_channel,
			pmlmeext->cur_bwmode, ch_width_str(pmlmeext->cur_bwmode), pmlmeext->cur_ch_offset, ch_offset_str(pmlmeext->cur_ch_offset));
		RTW_PRINT_SEL(m, "wireless_mode=0x%x(%s), cur_wireless_mode=0x%x(%s), rtsen=%d, cts2slef=%d\n", pmibpriv->band, wl_mode, tmp_wireless_mode, cur_wl_mode, psta->phl_sta->rts_en, psta->phl_sta->cts2self);
		/* ToDo: need API to query hal_sta->ra_info.rate_id */
		/* RTW_PRINT_SEL(m, "state=0x%x, aid=%d, macid=%d, raid=%d\n",
			psta->state, psta->phl_sta->aid, psta->phl_sta->macid, psta->phl_sta->ra_info.rate_id); */
		RTW_PRINT_SEL(m, "state=0x%x, aid=%d, macid=%d\n",
			psta->state, psta->phl_sta->aid, psta->phl_sta->macid);
#ifdef CONFIG_80211N_HT
		RTW_PRINT_SEL(m, "qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);
		RTW_PRINT_SEL(m, "bwmode=%d, ch_offset=%d, sgi_20m=%d,sgi_40m=%d\n"
			, psta->phl_sta->chandef.bw, psta->htpriv.ch_offset, psta->htpriv.sgi_20m, psta->htpriv.sgi_40m);
		RTW_PRINT_SEL(m, "ampdu_enable = %d\n", psta->htpriv.ampdu_enable);
		RTW_PRINT_SEL(m, "agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);
		RTW_PRINT_SEL(m, "ldpc_cap=0x%x, stbc_cap=0x%x, beamform_cap=0x%x\n", psta->htpriv.ldpc_cap, psta->htpriv.stbc_cap, psta->htpriv.beamform_cap);
		RTW_PRINT_SEL(m, "op_info = 0x%X, olbc_sta=%d, olbc_ap=%d, olbc_count=%d\n", pmlmepriv->ht_op_mode, ATOMIC_READ(&pmlmepriv->olbc_sta), ATOMIC_READ(&pmlmepriv->olbc_ap), pmlmepriv->olbc_count);
#endif /* CONFIG_80211N_HT */
#ifdef CONFIG_80211AC_VHT
		RTW_PRINT_SEL(m, "vht_en=%d, vht_sgi_80m=%d\n", psta->vhtpriv.vht_option, psta->vhtpriv.sgi_80m);
		RTW_PRINT_SEL(m, "vht_ldpc_cap=0x%x, vht_stbc_cap=0x%x, vht_beamform_cap=0x%x\n", psta->vhtpriv.ldpc_cap, psta->vhtpriv.stbc_cap, psta->vhtpriv.beamform_cap);
		RTW_PRINT_SEL(m, "vht_mcs_map=0x%x, vht_highest_rate=0x%x, vht_ampdu_len=%d\n", get_unaligned((u16 *)(psta->vhtpriv.vht_mcs_map)), psta->vhtpriv.vht_highest_rate, psta->vhtpriv.ampdu_len);
		if (psta->vhtpriv.vht_option) {
			RTW_MAP_DUMP_SEL_ALWAYS(m, "vht_cap=", psta->vhtpriv.vht_cap, 32);
		} else {
			RTW_PRINT_SEL(m, "vht_cap=N/A\n");
		}
#endif
#ifdef CONFIG_80211AX_HE
		RTW_PRINT_SEL(m, "he_en=%d\n", psta->hepriv.he_option);
		if (psta->hepriv.he_option) {
			RTW_MAP_DUMP_SEL_ALWAYS(m, "he_cap=", psta->hepriv.he_cap, HE_CAP_ELE_MAX_LEN);
		} else {
			RTW_PRINT_SEL(m, "he_cap=N/A\n");
		}
#endif
		RTW_PRINT_SEL(m, "channel_loading=%u(%%)\n", env_rpt->nhm_cca_ratio);
		RTW_PRINT_SEL(m, "idle_free_time=%u(%%)\n", env_rpt->nhm_idle_ratio);
		RTW_PRINT_SEL(m, "noise_level=%d(dBm)\n", env_rpt->nhm_pwr - 110);
		RTW_PRINT_SEL(m, "CCA=%u\n", padapter->dvobj->hw_stat.cur_cca_cnt);
		RTW_PRINT_SEL(m, "FA=%u\n", padapter->dvobj->hw_stat.cur_fa_cnt);
		sta_rx_reorder_ctl_dump(m, psta);
#ifdef CONFIG_DYNAMIC_THROUGHPUT_ENGINE
		RTW_PRINT_SEL(m, "th_mode=%d\n", padapter->dvobj->th_mode);
#endif
#ifdef CONFIG_RTW_HANDLE_SER_L2
		RTW_PRINT_SEL(m, "ser_L2_cnt=%d\n", padapter->dvobj->ser_L2_cnt);
#endif
	} else
		RTW_PRINT_SEL(m, "can't get sta's macaddr, cur_network's macaddr:" MAC_FMT "\n", MAC_ARG(cur_network->network.MacAddress));

	return 0;
}

ssize_t proc_reset_trx_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_priv  *precvpriv = &padapter->recvpriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	char cmd[32] = {0};
	u8 cnt = 0;

	if (count > sizeof(cmd)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(cmd, buffer, count)) {
		int num = sscanf(cmd, "%hhx", &cnt);

		if (num == 1 && cnt == 0) {
			precvpriv->dbg_rx_ampdu_drop_count = 0;
			precvpriv->dbg_rx_ampdu_forced_indicate_count = 0;
			precvpriv->dbg_rx_ampdu_loss_count = 0;
			precvpriv->dbg_rx_dup_mgt_frame_drop_count = 0;
			precvpriv->dbg_rx_ampdu_window_shift_cnt = 0;
			precvpriv->dbg_rx_conflic_mac_addr_cnt = 0;
			precvpriv->dbg_rx_drop_count = 0;
			pxmitpriv->tx_pkts = 0;
			pxmitpriv->tx_bytes = 0;
			pxmitpriv->tx_uc_pkts = 0;
			pxmitpriv->tx_bc_pkts = 0;
			pxmitpriv->tx_mc_pkts = 0;
			pxmitpriv->tx_uc_bytes = 0;
			pxmitpriv->tx_bc_bytes = 0;
			pxmitpriv->tx_mc_bytes = 0;
			pxmitpriv->tx_mgmt_pkts = 0;
			pxmitpriv->tx_drop = 0;
			pxmitpriv->tx_data_retry = 0;
			precvpriv->rx_pkts = 0;
			precvpriv->rx_bytes = 0;
			precvpriv->rx_uc_pkts = 0;
			precvpriv->rx_bc_pkts = 0;
			precvpriv->rx_mc_pkts = 0;
			precvpriv->rx_uc_bytes = 0;
			precvpriv->rx_bc_bytes = 0;
			precvpriv->rx_mc_bytes = 0;
			precvpriv->rx_errors = 0;
			precvpriv->rx_mgmt_pkts = 0;
			#ifndef CONFIG_RTW_LINK_PHL_MASTER
			// 1bde794936d38fb11d75d96df62872d6fd410ad3 ystang
			rtw_phl_reset_hw_cnt_tx(dvobj->phl);
			#endif /* CONFIG_RTW_LINK_PHL_MASTER */
		}
	}

	return count;
}

int proc_get_trx_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	int i;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct recv_priv  *precvpriv = &padapter->recvpriv;
	struct hw_xmit *phwxmit;
	struct trx_data_buf_q  *litexmitbuf_q = &dvobj->litexmitbuf_q;
	struct trx_data_buf_q  *literecvbuf_q = &dvobj->literecvbuf_q;
#ifdef CONFIG_USB_HCI
	struct trx_urb_buf_q *xmit_urb_q = &dvobj->xmit_urb_q;
	struct trx_urb_buf_q *recv_urb_q = &dvobj->recv_urb_q;
#endif
	u16 vo_params[4]={0}, vi_params[4]={0}, be_params[4]={0}, bk_params[4]={0};
	u32 tx_ok=0, tx_fail=0, tx_ok_mgmt=0, tx_fail_mgmt=0;
	u32 rtylmt=0, lt_drop=0, macid_drop=0, wp_polluted=0;

#if 0
	rtw_hal_read_edca(padapter, vo_params, vi_params, be_params, bk_params);

	RTW_PRINT_SEL(m, "wmm_edca_vo, aifs = %u us, cw_min = %u, cw_max = %u, txop_limit = %u us\n", vo_params[0], vo_params[1], vo_params[2], vo_params[3]);
	RTW_PRINT_SEL(m, "wmm_edca_vi, aifs = %u us, cw_min = %u, cw_max = %u, txop_limit = %u us\n", vi_params[0], vi_params[1], vi_params[2], vi_params[3]);
	RTW_PRINT_SEL(m, "wmm_edca_be, aifs = %u us, cw_min = %u, cw_max = %u, txop_limit = %u us\n", be_params[0], be_params[1], be_params[2], be_params[3]);
	RTW_PRINT_SEL(m, "wmm_edca_bk, aifs = %u us, cw_min = %u, cw_max = %u, txop_limit = %u us\n", bk_params[0], bk_params[1], bk_params[2], bk_params[3]);
#endif
	dump_os_queue(m, padapter);

	#if 0 /*CONFIG_CORE_XMITBUF*/
	RTW_PRINT_SEL(m, "free_xmitbuf_cnt=%d, free_xmitframe_cnt=%d\n"
		, pxmitpriv->free_xmitbuf_cnt, pxmitpriv->free_xmitframe_cnt);
	RTW_PRINT_SEL(m, "free_ext_xmitbuf_cnt=%d, free_xframe_ext_cnt=%d\n"
		, pxmitpriv->free_xmit_extbuf_cnt, pxmitpriv->free_xframe_ext_cnt);
	#endif
	RTW_PRINT_SEL(m, "free_recvframe_cnt=%d\n"
		      , precvpriv->free_recvframe_cnt);
	RTW_PRINT_SEL(m, "free_xmitframe_cnt=%d\n"
		, pxmitpriv->free_xmitframe_cnt);
	RTW_PRINT_SEL(m, "free_xframe_ext_cnt=%d\n"
		, pxmitpriv->free_xframe_ext_cnt);
	RTW_PRINT_SEL(m, "free_txreq_queue->qlen=%d\n"
		      , padapter->pfree_txreq_queue->qlen);

	RTW_PRINT_SEL(m, "free_litexmitbuf_cnt=%d\n"
		, litexmitbuf_q->free_data_buf_cnt);
	RTW_PRINT_SEL(m, "free_recvbuf_cnt=%d\n"
		, literecvbuf_q->free_data_buf_cnt);

	for (i = 0; i < 4; i++) {
		phwxmit = pxmitpriv->hwxmits + i;
		RTW_PRINT_SEL(m, "%d, hwq.accnt=%d\n", i, phwxmit->accnt);
	}

#ifdef CONFIG_USB_HCI
	RTW_PRINT_SEL(m, "rx_urb_pending_cn=%d\n", ATOMIC_READ(&(precvpriv->rx_pending_cnt)));
	RTW_PRINT_SEL(m, "tx_urb_cnt=%d\n", xmit_urb_q->free_urb_buf_cnt);
	RTW_PRINT_SEL(m, "rx_urb_cnt=%d\n", recv_urb_q->free_urb_buf_cnt);
#endif

	dump_rx_bh_tk(m, &GET_PRIMARY_ADAPTER(padapter)->recvpriv);

	/* Folowing are RX info */
	RTW_PRINT_SEL(m, "RX: Count of Packets dropped by Driver: %llu\n", (unsigned long long)precvpriv->dbg_rx_drop_count);
	/* Counts of packets whose seq_num is less than preorder_ctrl->indicate_seq, Ex delay, retransmission, redundant packets and so on */
	RTW_PRINT_SEL(m, "Rx: Counts of Packets Whose Seq_Num Less Than Reorder Control Seq_Num: %llu\n", (unsigned long long)precvpriv->dbg_rx_ampdu_drop_count);
	/* How many times the Rx Reorder Timer is triggered. */
	RTW_PRINT_SEL(m, "Rx: Reorder Time-out Trigger Counts: %llu\n", (unsigned long long)precvpriv->dbg_rx_ampdu_forced_indicate_count);
	/* Total counts of packets loss */
	RTW_PRINT_SEL(m, "Rx: Packet Loss Counts: %llu\n", (unsigned long long)precvpriv->dbg_rx_ampdu_loss_count);
	RTW_PRINT_SEL(m, "Rx: Duplicate Management Frame Drop Count: %llu\n", (unsigned long long)precvpriv->dbg_rx_dup_mgt_frame_drop_count);
	RTW_PRINT_SEL(m, "Rx: AMPDU BA window shift Count: %llu\n", (unsigned long long)precvpriv->dbg_rx_ampdu_window_shift_cnt);
	/*The same mac addr counts*/
	RTW_PRINT_SEL(m, "Rx: Conflict MAC Address Frames Count: %llu\n", (unsigned long long)precvpriv->dbg_rx_conflic_mac_addr_cnt);

	rtw_phl_get_hw_cnt_tx_ok(dvobj->phl, &tx_ok, &tx_ok_mgmt);
	rtw_phl_get_hw_cnt_tx_fail(dvobj->phl, &tx_fail, &tx_fail_mgmt);
	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// f75eb059acc393fa745d08b33c7f5e00d4ed1a3f skylin
	rtw_phl_get_hw_cnt_tx_sts(dvobj->phl, &rtylmt, &lt_drop, &macid_drop, &wp_polluted);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	RTW_PRINT_SEL(m,
		      "tx_packets=%llu\n"
		      "tx_bytes=%llu\n"
		      "tx_uc_packets=%llu\n"
		      "tx_bc_packets=%llu\n"
		      "tx_mc_packets=%llu\n"
		      "tx_drops=%llu\n"
		      "tx_drops_noasoc=%llu\n"
		      "tx_mgmt_packets=%llu\n"
		      "tx_data_oks=%d\n"
		      "tx_data_fails=%d\n"
		      "tx_data_retrys=%llu\n"
		      "tx_mgmt_oks=%d\n"
		      "tx_mgmt_fails=%d\n"
		      "tx_fail_rtylmt=%d\n"
		      "tx_fail_lt_drop=%d\n"
		      "tx_fail_macid_drop=%d\n"
		      "wp_polluted=%d\n"
		      "rx_packets=%llu\n"
		      "rx_bytes=%llu\n"
		      "rx_uc_packets=%llu\n"
		      "rx_bc_packets=%llu\n"
		      "rx_mc_packets=%llu\n"
		      "rx_drops=%llu\n"
		      "rx_errors=%llu\n"
		      "rx_mgmt_packets=%llu\n"
		      ,
		      pxmitpriv->tx_pkts,
		      pxmitpriv->tx_bytes,
		      pxmitpriv->tx_uc_pkts,
		      pxmitpriv->tx_bc_pkts,
		      pxmitpriv->tx_mc_pkts,
		      pxmitpriv->tx_drop,
		      pxmitpriv->tx_drop_noasoc,
		      pxmitpriv->tx_mgmt_pkts,
		      (tx_ok - tx_ok_mgmt),
		      (tx_fail - tx_fail_mgmt),
		      pxmitpriv->tx_data_retry,
		      tx_ok_mgmt,
		      tx_fail_mgmt,
		      rtylmt,
		      lt_drop,
		      macid_drop,
		      wp_polluted,
		      precvpriv->rx_pkts,
		      precvpriv->rx_bytes,
		      precvpriv->rx_uc_pkts,
		      precvpriv->rx_bc_pkts,
		      precvpriv->rx_mc_pkts,
		      precvpriv->dbg_rx_drop_count,
		      precvpriv->rx_errors,
		      precvpriv->rx_mgmt_pkts
		     );

	return 0;
}

const char *rtw_data_rate_str(enum rtw_data_rate rate)
{
	if (rate >= RTW_DATA_RATE_CCK1 && rate <= RTW_DATA_RATE_CCK11) {
		switch (rate) {
#define CASE_CCK_RATE(cck)	case RTW_DATA_RATE_CCK ## cck: return "CCK_" # cck "M"
		CASE_CCK_RATE(1);
		CASE_CCK_RATE(2);
		CASE_CCK_RATE(5_5);
		CASE_CCK_RATE(11);
#undef CASE_CCK_RATE
		default:
			return "CCK_UNKNOWN";
		}
	} else if (rate >= RTW_DATA_RATE_OFDM6 && rate <= RTW_DATA_RATE_OFDM54) {
		switch (rate) {
#define CASE_OFDM_RATE(ofdm)	case RTW_DATA_RATE_OFDM ## ofdm: return "OFDM_" # ofdm "M"
		CASE_OFDM_RATE(6);
		CASE_OFDM_RATE(9);
		CASE_OFDM_RATE(12);
		CASE_OFDM_RATE(18);
		CASE_OFDM_RATE(24);
		CASE_OFDM_RATE(36);
		CASE_OFDM_RATE(48);
		CASE_OFDM_RATE(54);
#undef CASE_OFDM_RATE
		default:
			return "OFDM_UNKNOWN";
		}
	} else if (rate >= RTW_DATA_RATE_MCS0 && rate <= RTW_DATA_RATE_MCS31) {
		switch (rate) {
#define CASE_HT_RATE(mcs)	case RTW_DATA_RATE_MCS ## mcs: return "MCS_" # mcs
		CASE_HT_RATE(0);
		CASE_HT_RATE(1);
		CASE_HT_RATE(2);
		CASE_HT_RATE(3);
		CASE_HT_RATE(4);
		CASE_HT_RATE(5);
		CASE_HT_RATE(6);
		CASE_HT_RATE(7);
		CASE_HT_RATE(8);
		CASE_HT_RATE(9);
		CASE_HT_RATE(10);
		CASE_HT_RATE(11);
		CASE_HT_RATE(12);
		CASE_HT_RATE(13);
		CASE_HT_RATE(14);
		CASE_HT_RATE(15);
		CASE_HT_RATE(16);
		CASE_HT_RATE(17);
		CASE_HT_RATE(18);
		CASE_HT_RATE(19);
		CASE_HT_RATE(20);
		CASE_HT_RATE(21);
		CASE_HT_RATE(22);
		CASE_HT_RATE(23);
		CASE_HT_RATE(24);
		CASE_HT_RATE(25);
		CASE_HT_RATE(26);
		CASE_HT_RATE(27);
		CASE_HT_RATE(28);
		CASE_HT_RATE(29);
		CASE_HT_RATE(30);
		CASE_HT_RATE(31);
#undef CASE_HT_RATE
		default:
			return "HT_UNKNOWN";
		}
	} else if (rate >= RTW_DATA_RATE_VHT_NSS1_MCS0 && rate <= RTW_DATA_RATE_VHT_NSS4_MCS9) {
		switch (rate) {
#define CASE_VHT_RATE(ss, mcs)	case RTW_DATA_RATE_VHT_NSS ## ss ##_MCS ##mcs: return "VHT_SS" #ss "MCS" #mcs
		CASE_VHT_RATE(1, 0);
		CASE_VHT_RATE(1, 1);
		CASE_VHT_RATE(1, 2);
		CASE_VHT_RATE(1, 3);
		CASE_VHT_RATE(1, 4);
		CASE_VHT_RATE(1, 5);
		CASE_VHT_RATE(1, 6);
		CASE_VHT_RATE(1, 7);
		CASE_VHT_RATE(1, 8);
		CASE_VHT_RATE(1, 9);
		CASE_VHT_RATE(2, 0);
		CASE_VHT_RATE(2, 1);
		CASE_VHT_RATE(2, 2);
		CASE_VHT_RATE(2, 3);
		CASE_VHT_RATE(2, 4);
		CASE_VHT_RATE(2, 5);
		CASE_VHT_RATE(2, 6);
		CASE_VHT_RATE(2, 7);
		CASE_VHT_RATE(2, 8);
		CASE_VHT_RATE(2, 9);
		CASE_VHT_RATE(3, 0);
		CASE_VHT_RATE(3, 1);
		CASE_VHT_RATE(3, 2);
		CASE_VHT_RATE(3, 3);
		CASE_VHT_RATE(3, 4);
		CASE_VHT_RATE(3, 5);
		CASE_VHT_RATE(3, 6);
		CASE_VHT_RATE(3, 7);
		CASE_VHT_RATE(3, 8);
		CASE_VHT_RATE(3, 9);
		CASE_VHT_RATE(4, 0);
		CASE_VHT_RATE(4, 1);
		CASE_VHT_RATE(4, 2);
		CASE_VHT_RATE(4, 3);
		CASE_VHT_RATE(4, 4);
		CASE_VHT_RATE(4, 5);
		CASE_VHT_RATE(4, 6);
		CASE_VHT_RATE(4, 7);
		CASE_VHT_RATE(4, 8);
		CASE_VHT_RATE(4, 9);
#undef CASE_VHT_RATE
		default:
			return "VHT_UNKNOWN";
		}
	} else if (rate >= RTW_DATA_RATE_HE_NSS1_MCS0 && rate <= RTW_DATA_RATE_HE_NSS4_MCS11) {
		switch (rate) {
#define CASE_HE_RATE(ss, mcs)	case RTW_DATA_RATE_HE_NSS ## ss ##_MCS ##mcs: return "HE_SS" #ss "MCS" #mcs
		CASE_HE_RATE(1, 0);
		CASE_HE_RATE(1, 1);
		CASE_HE_RATE(1, 2);
		CASE_HE_RATE(1, 3);
		CASE_HE_RATE(1, 4);
		CASE_HE_RATE(1, 5);
		CASE_HE_RATE(1, 6);
		CASE_HE_RATE(1, 7);
		CASE_HE_RATE(1, 8);
		CASE_HE_RATE(1, 9);
		CASE_HE_RATE(1, 10);
		CASE_HE_RATE(1, 11);
		CASE_HE_RATE(2, 0);
		CASE_HE_RATE(2, 1);
		CASE_HE_RATE(2, 2);
		CASE_HE_RATE(2, 3);
		CASE_HE_RATE(2, 4);
		CASE_HE_RATE(2, 5);
		CASE_HE_RATE(2, 6);
		CASE_HE_RATE(2, 7);
		CASE_HE_RATE(2, 8);
		CASE_HE_RATE(2, 9);
		CASE_HE_RATE(2, 10);
		CASE_HE_RATE(2, 11);
		CASE_HE_RATE(3, 0);
		CASE_HE_RATE(3, 1);
		CASE_HE_RATE(3, 2);
		CASE_HE_RATE(3, 3);
		CASE_HE_RATE(3, 4);
		CASE_HE_RATE(3, 5);
		CASE_HE_RATE(3, 6);
		CASE_HE_RATE(3, 7);
		CASE_HE_RATE(3, 8);
		CASE_HE_RATE(3, 9);
		CASE_HE_RATE(3, 10);
		CASE_HE_RATE(3, 11);
		CASE_HE_RATE(4, 0);
		CASE_HE_RATE(4, 1);
		CASE_HE_RATE(4, 2);
		CASE_HE_RATE(4, 3);
		CASE_HE_RATE(4, 4);
		CASE_HE_RATE(4, 5);
		CASE_HE_RATE(4, 6);
		CASE_HE_RATE(4, 7);
		CASE_HE_RATE(4, 8);
		CASE_HE_RATE(4, 9);
		CASE_HE_RATE(4, 10);
		CASE_HE_RATE(4, 11);
#undef CASE_HE_RATE
		default:
			return "HE_UNKNOWN";
		}
	}

	return "ALL_UNKNOWN";
}

int proc_get_rate_ctl(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u16 data_rate = 0;
	u8 sgi = 0, data_fb = 0;

	if (adapter->fix_rate != NO_FIX_RATE) {
		data_rate = GET_FIX_RATE(adapter->fix_rate);
		sgi = GET_FIX_RATE_SGI(adapter->fix_rate);
		data_fb = adapter->data_fb ? 1 : 0;
		RTW_PRINT_SEL(m, "FIXED %s%s%s\n"
			, rtw_data_rate_str(data_rate)
			, data_rate >= RTW_DATA_RATE_MCS0 ? (sgi ? " SGI" : " LGI") : ""
			, data_fb ? " FB" : ""
		);
		RTW_PRINT_SEL(m, "0x%02x %u\n", adapter->fix_rate, adapter->data_fb);
	} else
		RTW_PRINT_SEL(m, "RA\n");

	return 0;
}

ssize_t proc_set_rate_ctl(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter_to_dvobj(adapter));
	char tmp[32];
	u16 fix_rate = NO_FIX_RATE;
	u8 data_fb = 0;


	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%hx %hhu", &fix_rate, &data_fb);

		if (num >= 1) {
			u16 fix_rate_ori = adapter->fix_rate;

			adapter->fix_rate = fix_rate;
			#if 0 /*GEORGIA_TODO_FIXIT*/
			if (fix_rate == 0xFF)
				hal_data->ForcedDataRate = 0;
			else
				hal_data->ForcedDataRate = hwrate_to_mrate(fix_rate & 0x7F);
			#endif
			if (adapter->fix_bw != NO_FIX_BW && fix_rate_ori != fix_rate)
				rtw_run_in_thread_cmd(adapter, ((void *)(rtw_update_tx_rate_bmp)), adapter_to_dvobj(adapter));
		}
		if (num >= 2)
			adapter->data_fb = data_fb ? 1 : 0;
	}

	return count;
}

#ifdef CONFIG_AP_MODE
int proc_get_bmc_tx_rate(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	if (!MLME_IS_AP(adapter) && !MLME_IS_MESH(adapter)) {
		RTW_PRINT_SEL(m, "[ERROR] Not in SoftAP/Mesh mode !!\n");
		return 0;
	}

	RTW_PRINT_SEL(m, " BMC Tx rate - %s\n", MGN_RATE_STR(adapter->bmc_tx_rate));
	return 0;
}

ssize_t proc_set_bmc_tx_rate(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 bmc_tx_rate;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &bmc_tx_rate);

		if (num >= 1)
			/*adapter->bmc_tx_rate = hwrate_to_mrate(bmc_tx_rate);*/
			adapter->bmc_tx_rate = bmc_tx_rate;
	}

	return count;
}
#endif /*CONFIG_AP_MODE*/


int proc_get_tx_power_offset(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "Tx power offset - %u\n", adapter->power_offset);
	return 0;
}

ssize_t proc_set_tx_power_offset(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 power_offset = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu", &power_offset);

		if (num >= 1) {
			if (power_offset > 5)
				power_offset = 0;

			adapter->power_offset = power_offset;
		}
	}

	return count;
}

int proc_get_bw_ctl(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	u8 data_bw = 0;

	if (adapter->fix_bw != NO_FIX_BW) {
		data_bw = adapter->fix_bw;
		RTW_PRINT_SEL(m, "FIXED %s\n", ch_width_str(data_bw));
	} else
		RTW_PRINT_SEL(m, "Auto\n");

	return 0;
}

ssize_t proc_set_bw_ctl(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 fix_bw;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%hhu", &fix_bw);

		if (num >= 1) {
			u8 fix_bw_ori = adapter->fix_bw;

			adapter->fix_bw = fix_bw;

			if (adapter->fix_rate != NO_FIX_RATE && fix_bw_ori != fix_bw)
				rtw_run_in_thread_cmd(adapter, ((void *)(rtw_update_tx_rate_bmp)), adapter_to_dvobj(adapter));
		}
	}

	return count;
}

#ifdef DBG_RX_COUNTER_DUMP
int proc_get_rx_cnt_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	int i;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "BIT0- Dump RX counters of DRV\n");
	RTW_PRINT_SEL(m, "BIT1- Dump RX counters of MAC\n");
	RTW_PRINT_SEL(m, "BIT2- Dump RX counters of PHY\n");
	RTW_PRINT_SEL(m, "BIT3- Dump TRX data frame of DRV\n");
	RTW_PRINT_SEL(m, "dump_rx_cnt_mode = 0x%02x\n", adapter->dump_rx_cnt_mode);

	return 0;
}
ssize_t proc_set_rx_cnt_dump(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 dump_rx_cnt_mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &dump_rx_cnt_mode);

		if (num == 1) {
			rtw_dump_phy_rxcnts_preprocess(adapter, dump_rx_cnt_mode);
			adapter->dump_rx_cnt_mode = dump_rx_cnt_mode;
		}
	}

	return count;
}
#endif


ssize_t proc_set_del_rx_ampdu_test_case(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count))
		sscanf(tmp, "%hhu", &del_rx_ampdu_test_no_tx_fail);

	return count;
}

ssize_t proc_set_wait_hiq_empty(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count))
		sscanf(tmp, "%u", &g_wait_hiq_empty_ms);

	return count;
}

ssize_t proc_set_sta_linking_test(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		u32 wait_ms = 0;
		u8 force_fail = 0;
		int num = sscanf(tmp, "%u %hhu", &wait_ms, &force_fail);

		if (num >= 1)
			sta_linking_test_wait_ms = wait_ms;
		if (num >= 2)
			sta_linking_test_force_fail = force_fail;
	}

	return count;
}

#ifdef CONFIG_AP_MODE
ssize_t proc_set_ap_linking_test(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		u16 force_auth_fail = 0;
		u16 force_asoc_fail = 0;
		int num = sscanf(tmp, "%hu %hu", &force_auth_fail, &force_asoc_fail);

		if (num >= 1)
			ap_linking_test_force_auth_fail = force_auth_fail;
		if (num >= 2)
			ap_linking_test_force_asoc_fail = force_asoc_fail;
	}

	return count;
}
#endif /* CONFIG_AP_MODE */

int proc_get_ps_dbg_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;

	RTW_PRINT_SEL(m, "dbg_sdio_alloc_irq_cnt=%d\n", pdbgpriv->dbg_sdio_alloc_irq_cnt);
	RTW_PRINT_SEL(m, "dbg_sdio_free_irq_cnt=%d\n", pdbgpriv->dbg_sdio_free_irq_cnt);
	RTW_PRINT_SEL(m, "dbg_sdio_alloc_irq_error_cnt=%d\n", pdbgpriv->dbg_sdio_alloc_irq_error_cnt);
	RTW_PRINT_SEL(m, "dbg_sdio_free_irq_error_cnt=%d\n", pdbgpriv->dbg_sdio_free_irq_error_cnt);
	RTW_PRINT_SEL(m, "dbg_sdio_init_error_cnt=%d\n", pdbgpriv->dbg_sdio_init_error_cnt);
	RTW_PRINT_SEL(m, "dbg_sdio_deinit_error_cnt=%d\n", pdbgpriv->dbg_sdio_deinit_error_cnt);
	RTW_PRINT_SEL(m, "dbg_suspend_error_cnt=%d\n", pdbgpriv->dbg_suspend_error_cnt);
	RTW_PRINT_SEL(m, "dbg_suspend_cnt=%d\n", pdbgpriv->dbg_suspend_cnt);
	RTW_PRINT_SEL(m, "dbg_resume_cnt=%d\n", pdbgpriv->dbg_resume_cnt);
	RTW_PRINT_SEL(m, "dbg_resume_error_cnt=%d\n", pdbgpriv->dbg_resume_error_cnt);
	RTW_PRINT_SEL(m, "dbg_deinit_fail_cnt=%d\n", pdbgpriv->dbg_deinit_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_carddisable_cnt=%d\n", pdbgpriv->dbg_carddisable_cnt);
	RTW_PRINT_SEL(m, "dbg_ps_insuspend_cnt=%d\n", pdbgpriv->dbg_ps_insuspend_cnt);
	RTW_PRINT_SEL(m, "dbg_dev_unload_inIPS_cnt=%d\n", pdbgpriv->dbg_dev_unload_inIPS_cnt);
	RTW_PRINT_SEL(m, "dbg_scan_pwr_state_cnt=%d\n", pdbgpriv->dbg_scan_pwr_state_cnt);
	RTW_PRINT_SEL(m, "dbg_downloadfw_pwr_state_cnt=%d\n", pdbgpriv->dbg_downloadfw_pwr_state_cnt);
	RTW_PRINT_SEL(m, "dbg_carddisable_error_cnt=%d\n", pdbgpriv->dbg_carddisable_error_cnt);
	RTW_PRINT_SEL(m, "dbg_fw_read_ps_state_fail_cnt=%d\n", pdbgpriv->dbg_fw_read_ps_state_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_leave_ips_fail_cnt=%d\n", pdbgpriv->dbg_leave_ips_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_leave_lps_fail_cnt=%d\n", pdbgpriv->dbg_leave_lps_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_h2c_leave32k_fail_cnt=%d\n", pdbgpriv->dbg_h2c_leave32k_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_diswow_dload_fw_fail_cnt=%d\n", pdbgpriv->dbg_diswow_dload_fw_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_enwow_dload_fw_fail_cnt=%d\n", pdbgpriv->dbg_enwow_dload_fw_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_ips_drvopen_fail_cnt=%d\n", pdbgpriv->dbg_ips_drvopen_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_poll_fail_cnt=%d\n", pdbgpriv->dbg_poll_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_rpwm_toogle_cnt=%d\n", pdbgpriv->dbg_rpwm_toogle_cnt);
	RTW_PRINT_SEL(m, "dbg_rpwm_timeout_fail_cnt=%d\n", pdbgpriv->dbg_rpwm_timeout_fail_cnt);
	RTW_PRINT_SEL(m, "dbg_sreset_cnt=%d\n", pdbgpriv->dbg_sreset_cnt);
	RTW_PRINT_SEL(m, "dbg_fw_mem_dl_error_cnt=%d\n", pdbgpriv->dbg_fw_mem_dl_error_cnt);

	return 0;
}
ssize_t proc_set_ps_dbg_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter->dvobj;
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;
	char tmp[32];
	u8 ps_dbg_cmd_id;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx", &ps_dbg_cmd_id);

		if (num == 1 && ps_dbg_cmd_id == 1) /*Clean all*/
			_rtw_memset(pdbgpriv, 0, sizeof(struct debug_priv));

	}

	return count;
}


#ifdef CONFIG_DBG_COUNTER

int proc_get_rx_logs(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rx_logs *rx_logs = &padapter->rx_logs;

	RTW_PRINT_SEL(m,
		      "intf_rx=%d\n"
		      "intf_rx_err_recvframe=%d\n"
		      "intf_rx_err_skb=%d\n"
		      "intf_rx_report=%d\n"
		      "core_rx_prcss=%u\n"
		      "core_rx_prcss_succ=%u\n"
		      "core_rx_allc_recvf_err=%u\n"
		      "core_rx=%d\n"
	              "core_rx_amsdu_cut=%u\n"
	              "core_rx_amsdu_cut_hdr_conv=%u\n"
	              "core_rx_hdr_conv=%u\n"
		      "core_rx_err_recvframe=%d\n"
		      "core_rx_req=%d\n"
		      "core_rx_icv_err=%d\n"
		      "core_rx_crc_err=%d\n"
		      "core_rx_rxsc_done=%d\n"
		      "core_rx_next=%d\n"
		      "core_rx_err_stop=%d\n"
		      "core_rx_pre=%d\n"
		      "core_rx_pre_ver_err=%d\n"
		      "core_rx_pre_mgmt=%d\n"
		      "core_rx_pre_mgmt_err_80211w=%d\n"
		      "core_rx_pre_mgmt_err=%d\n"
		      "core_rx_pre_ctrl=%d\n"
		      "core_rx_pre_ctrl_err=%d\n"
		      "core_rx_pre_data=%d\n"
		      "core_rx_pre_data_wapi_seq_err=%d\n"
		      "core_rx_pre_data_wapi_key_err=%d\n"
		      "drop@core_rx_pre_data_handled=%d\n"
		      "core_rx_pre_data_err=%d\n"
		      "core_rx_pre_data_unknown=%d\n"
		      "core_rx_pre_unknown=%d\n"
		      "core_rx_enqueue=%d\n"
		      "core_rx_dequeue=%d\n"
		      "core_rx_post=%d\n"
		      "core_rx_post_decrypt=%d\n"
		      "core_rx_post_decrypt_wep=%d\n"
		      "core_rx_post_decrypt_tkip=%d\n"
		      "core_rx_post_decrypt_aes=%d\n"
		      "core_rx_post_decrypt_wapi=%d\n"
		      "core_rx_post_decrypt_hw=%d\n"
		      "core_rx_post_decrypt_unknown=%d\n"
		      "core_rx_post_decrypt_err=%d\n"
		      "core_rx_post_defrag_err=%d\n"
		      "core_rx_post_portctrl_err=%d\n"
		      "core_rx_post_indicate=%d\n"
		      "core_rx_post_indicate_in_oder=%d\n"
		      "core_rx_post_indicate_reoder=%d\n"
		      "core_rx_post_indicate_err=%d\n"
		      #ifdef CONFIG_RTW_DEBUG_RX_CACHE
		      "core_rx_ta_no_sta=%u\n"
		      #endif /* CONFIG_RTW_DEBUG_RX_CACHE */
		      "os_indicate=%d\n"
		      "os_indicate_ap_mcast=%d\n"
		      "os_indicate_ap_mcast_drop=%d\n"
		      "os_indicate_ap_forward=%d\n"
		      "os_indicate_ap_self=%d\n"
		      "os_indicate_err=%d\n"
		      "os_netif_ok=%d\n"
		      "os_netif_err=%d\n",
		      rx_logs->intf_rx,
		      rx_logs->intf_rx_err_recvframe,
		      rx_logs->intf_rx_err_skb,
		      rx_logs->intf_rx_report,
		      rx_logs->core_rx_prcss,
		      rx_logs->core_rx_prcss_succ,
		      rx_logs->core_rx_allc_recvf_err,
		      rx_logs->core_rx,
		      rx_logs->core_rx_amsdu_cut,
		      rx_logs->core_rx_amsdu_cut_hdr_conv,
		      rx_logs->core_rx_hdr_conv,
		      rx_logs->core_rx_err_recvframe,
		      rx_logs->core_rx_req,
		      rx_logs->core_rx_icv_err,
		      rx_logs->core_rx_crc_err,
		      rx_logs->core_rx_rxsc_done,
		      rx_logs->core_rx_next,
		      rx_logs->core_rx_err_stop,
		      rx_logs->core_rx_pre,
		      rx_logs->core_rx_pre_ver_err,
		      rx_logs->core_rx_pre_mgmt,
		      rx_logs->core_rx_pre_mgmt_err_80211w,
		      rx_logs->core_rx_pre_mgmt_err,
		      rx_logs->core_rx_pre_ctrl,
		      rx_logs->core_rx_pre_ctrl_err,
		      rx_logs->core_rx_pre_data,
		      rx_logs->core_rx_pre_data_wapi_seq_err,
		      rx_logs->core_rx_pre_data_wapi_key_err,
		      rx_logs->core_rx_pre_data_handled,
		      rx_logs->core_rx_pre_data_err,
		      rx_logs->core_rx_pre_data_unknown,
		      rx_logs->core_rx_pre_unknown,
		      rx_logs->core_rx_enqueue,
		      rx_logs->core_rx_dequeue,
		      rx_logs->core_rx_post,
		      rx_logs->core_rx_post_decrypt,
		      rx_logs->core_rx_post_decrypt_wep,
		      rx_logs->core_rx_post_decrypt_tkip,
		      rx_logs->core_rx_post_decrypt_aes,
		      rx_logs->core_rx_post_decrypt_wapi,
		      rx_logs->core_rx_post_decrypt_hw,
		      rx_logs->core_rx_post_decrypt_unknown,
		      rx_logs->core_rx_post_decrypt_err,
		      rx_logs->core_rx_post_defrag_err,
		      rx_logs->core_rx_post_portctrl_err,
		      rx_logs->core_rx_post_indicate,
		      rx_logs->core_rx_post_indicate_in_oder,
		      rx_logs->core_rx_post_indicate_reoder,
		      rx_logs->core_rx_post_indicate_err,
		      #ifdef CONFIG_RTW_DEBUG_RX_CACHE
		      rx_logs->core_rx_ta_no_sta,
		      #endif /* CONFIG_RTW_DEBUG_RX_CACHE */
		      rx_logs->os_indicate,
		      rx_logs->os_indicate_ap_mcast,
		      rx_logs->os_indicate_ap_mcast_drop,
		      rx_logs->os_indicate_ap_forward,
		      rx_logs->os_indicate_ap_self,
		      rx_logs->os_indicate_err,
		      rx_logs->os_netif_ok,
		      rx_logs->os_netif_err
		     );

	return 0;
}

ssize_t proc_reset_rx_logs(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	memset(&padapter->rx_logs, 0, sizeof(padapter->rx_logs));
	RTW_INFO("reset rx log counters\n");

	return count;
}

int proc_get_tx_logs(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tx_logs *tx_logs = &padapter->tx_logs;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	RTW_PRINT_SEL(m,
		      "os_tx=%d\n"
		      "os_tx_err_up=%d\n"
		      "os_tx_bmc=%d\n"
		      "os_tx_uc=%d\n"
		      "os_tx_tcp=%d\n"
		      "os_tx_udp=%d\n"
		      "os_tx_normal=%d\n"
		      "os_tx_drop=%d\n"
		      "os_tx_m2u=%d\n"
		      "os_tx_m2u_ignore_fw_linked=%d\n"
		      "os_tx_m2u_ignore_self=%d\n"
		      "os_tx_m2u_entry=%d\n"
		      "os_tx_m2u_entry_err_xmit=%d\n"
		      "os_tx_m2u_entry_err_skb=%d\n"
		      "os_tx_m2u_stop=%d\n"
		      "os_tx_m2u_ps_enqueue=%d\n"
		      "os_tx_m2u_ps_drop=%d\n"
		      "core_tx=%d\n"
#ifdef CONFIG_CORE_TXSC
		      "core_tx_txsc=%d\n"
		      "core_tx_direct_phl=%d\n"
#ifdef CONFIG_TXSC_AMSDU
		      "core_tx_txsc_amsdu_enq[0]=%d\n"
		      "core_tx_txsc_amsdu_enq[1]=%d\n"
		      "core_tx_txsc_amsdu_enq[2]=%d\n"
		      "core_tx_txsc_amsdu_enq[3]=%d\n"
		      "core_tx_txsc_amsdu_deq[0]=%d\n"
		      "core_tx_txsc_amsdu_deq[1]=%d\n"
		      "core_tx_txsc_amsdu_deq[2]=%d\n"
		      "core_tx_txsc_amsdu_deq[3]=%d\n"
		      "core_tx_txsc_amsdu_deq_empty=%d\n"
		      "core_tx_txsc_amsdu_deq_ampdu=%d\n"
		      "core_tx_txsc_amsdu_timeout_deq_empty=%d\n"
#endif
#endif
		      "core_tx_ps_enqueue=%d\n"
		      "core_tx_ps_drop=%d\n"
		      "core_tx_swq_enq=%d\n"
		      "core_tx_swq_deq=%d\n"
		      "core_tx_upd_attrib=%d\n"
		      "core_tx_upd_attrib_adhoc=%d\n"
		      "core_tx_upd_attrib_sta=%d\n"
		      "core_tx_upd_attrib_ap=%d\n"
		      "core_tx_upd_attrib_unknown=%d\n"
		      "core_tx_upd_attrib_dhcp=%d\n"
		      "core_tx_upd_attrib_icmp=%d\n"
		      "core_tx_upd_attrib_active=%d\n"
		      "core_tx_add_req_data=%d\n"
		      "core_tx_add_req_mgnt=%d\n"
		      "core_tx_err_drop=%d\n"
		      "core_tx_err_pxmitframe=%d\n"
		      "core_tx_err_brtx=%d\n"
#ifdef CONFIG_CORE_TXSC
		      "core_tx_err_txsc=%d\n"
#ifdef CONFIG_TXSC_AMSDU
		      "core_tx_txsc_amsdu_timeout_err=%d\n"
		      "core_tx_txsc_amsdu_enq_abort[0]=%d\n"
		      "core_tx_txsc_amsdu_enq_abort[1]=%d\n"
		      "core_tx_txsc_amsdu_enq_abort[2]=%d\n"
		      "core_tx_txsc_amsdu_enq_abort[3]=%d\n"
#endif
#endif
		      "core_tx_upd_attrib_err_ucast_sta=%d\n"
		      "core_tx_upd_attrib_err_ucast_ap_link=%d\n"
		      "core_tx_upd_attrib_err_sta=%d\n"
		      "core_tx_upd_attrib_err_link=%d\n"
		      "core_tx_upd_attrib_err_sec=%d\n"
		      "core_tx_err_prepare_phl=%d\n"
		      "core_tx_err_add_req=%d\n"
		      #ifdef CONFIG_DBG_HNDSK_MGMT
		      "auth_pkts(tx/ok/fail)= %d/%d/%d\n"
		      "asocrsp_pkts(tx/ok/fail)= %d/%d/%d\n"
		      "eapol_4_1_pkts(tx/ok/fail)= %d/%d/%d\n"
		      "eapol_4_3_pkts(tx/ok/fail)= %d/%d/%d\n"
		      #endif
		      "tx_wmm_pkts[BE]=%llu\n"
		      "tx_wmm_pkts[BK]=%llu\n"
		      "tx_wmm_pkts[VI]=%llu\n"
		      "tx_wmm_pkts[VO]=%llu\n"
		      #ifdef CONFIG_RTW_DEBUG_SCTX_ALLOC
		      "sctx=%u/%u\n"
		      #endif
		      ,
		      tx_logs->os_tx,
			  tx_logs->os_tx_err_up,
		      tx_logs->os_tx_bmc,
		      tx_logs->os_tx_uc,
		      tx_logs->os_tx_tcp,
		      tx_logs->os_tx_udp,
		      tx_logs->os_tx_normal,
		      tx_logs->os_tx_drop,
		      tx_logs->os_tx_m2u,
		      tx_logs->os_tx_m2u_ignore_fw_linked,
		      tx_logs->os_tx_m2u_ignore_self,
		      tx_logs->os_tx_m2u_entry,
		      tx_logs->os_tx_m2u_entry_err_xmit,
		      tx_logs->os_tx_m2u_entry_err_skb,
		      tx_logs->os_tx_m2u_stop,
		      tx_logs->os_tx_m2u_ps_enqueue,
		      tx_logs->os_tx_m2u_ps_drop,
		      tx_logs->core_tx,
#ifdef CONFIG_CORE_TXSC
		      tx_logs->core_tx_txsc,
		      tx_logs->core_tx_direct_phl,
#ifdef CONFIG_TXSC_AMSDU
		      pxmitpriv->cnt_txsc_amsdu_enq[0],
		      pxmitpriv->cnt_txsc_amsdu_enq[1],
		      pxmitpriv->cnt_txsc_amsdu_enq[2],
		      pxmitpriv->cnt_txsc_amsdu_enq[3],
		      pxmitpriv->cnt_txsc_amsdu_deq[0],
		      pxmitpriv->cnt_txsc_amsdu_deq[1],
		      pxmitpriv->cnt_txsc_amsdu_deq[2],
		      pxmitpriv->cnt_txsc_amsdu_deq[3],
		      pxmitpriv->cnt_txsc_amsdu_deq_empty,
		      pxmitpriv->cnt_txsc_amsdu_deq_ampdu,
		      pxmitpriv->cnt_txsc_amsdu_timeout_deq_empty,
#endif
#endif
		      tx_logs->core_tx_ps_enqueue,
		      tx_logs->core_tx_ps_drop,
		      tx_logs->core_tx_swq_enq,
		      tx_logs->core_tx_swq_deq,
		      tx_logs->core_tx_upd_attrib,
		      tx_logs->core_tx_upd_attrib_adhoc,
		      tx_logs->core_tx_upd_attrib_sta,
		      tx_logs->core_tx_upd_attrib_ap,
		      tx_logs->core_tx_upd_attrib_unknown,
		      tx_logs->core_tx_upd_attrib_dhcp,
		      tx_logs->core_tx_upd_attrib_icmp,
		      tx_logs->core_tx_upd_attrib_active,
		      tx_logs->core_tx_add_req_data,
		      tx_logs->core_tx_add_req_mgnt,
		      tx_logs->core_tx_err_drop,
		      tx_logs->core_tx_err_pxmitframe,
		      tx_logs->core_tx_err_brtx,
#ifdef CONFIG_CORE_TXSC
		      tx_logs->core_tx_err_txsc,
#ifdef CONFIG_TXSC_AMSDU
		      tx_logs->core_tx_txsc_amsdu_timeout_err,
		      pxmitpriv->cnt_txsc_amsdu_enq_abort[0],
		      pxmitpriv->cnt_txsc_amsdu_enq_abort[1],
		      pxmitpriv->cnt_txsc_amsdu_enq_abort[2],
		      pxmitpriv->cnt_txsc_amsdu_enq_abort[3],
#endif
#endif
		      tx_logs->core_tx_upd_attrib_err_ucast_sta,
		      tx_logs->core_tx_upd_attrib_err_ucast_ap_link,
		      tx_logs->core_tx_upd_attrib_err_sta,
		      tx_logs->core_tx_upd_attrib_err_link,
		      tx_logs->core_tx_upd_attrib_err_sec,
		      tx_logs->core_tx_err_prepare_phl,
		      tx_logs->core_tx_err_add_req,
		      #ifdef CONFIG_DBG_HNDSK_MGMT
		      tx_logs->core_hndsk_tx_cnt[HNDSK_AUTH_PKT],
		      tx_logs->core_hndsk_tx_ok_cnt[HNDSK_AUTH_PKT],
		      tx_logs->core_hndsk_tx_fail_cnt[HNDSK_AUTH_PKT],
		      tx_logs->core_hndsk_tx_cnt[HNDSK_ASSOC_RSP_PKT],
		      tx_logs->core_hndsk_tx_ok_cnt[HNDSK_ASSOC_RSP_PKT],
		      tx_logs->core_hndsk_tx_fail_cnt[HNDSK_ASSOC_RSP_PKT],
		      tx_logs->core_hndsk_tx_cnt[HNDSK_EAPOL_4_1_PKT],
		      tx_logs->core_hndsk_tx_ok_cnt[HNDSK_EAPOL_4_1_PKT],
		      tx_logs->core_hndsk_tx_fail_cnt[HNDSK_EAPOL_4_1_PKT],
		      tx_logs->core_hndsk_tx_cnt[HNDSK_EAPOL_4_3_PKT],
		      tx_logs->core_hndsk_tx_ok_cnt[HNDSK_EAPOL_4_3_PKT],
		      tx_logs->core_hndsk_tx_fail_cnt[HNDSK_EAPOL_4_3_PKT],
		      #endif
		      padapter->xmitpriv.tx_wmm_pkts[XMIT_BE_QUEUE],
		      padapter->xmitpriv.tx_wmm_pkts[XMIT_BK_QUEUE],
		      padapter->xmitpriv.tx_wmm_pkts[XMIT_VI_QUEUE],
		      padapter->xmitpriv.tx_wmm_pkts[XMIT_VO_QUEUE]
		      #ifdef CONFIG_RTW_DEBUG_SCTX_ALLOC
		      , tx_logs->sctx_alloc, tx_logs->sctx_free
		      #endif
		     );

	return 0;
}

ssize_t proc_set_tx_logs(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tx_logs *tx_logs = &padapter->tx_logs;
	char tmp[32];
	u32 reset;
	int num;
	int i;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num = sscanf(tmp, "%d ", &reset);
		if (num == 1 && reset == 0) {
			memset((void *)tx_logs, 0, sizeof(struct tx_logs));
			RTW_INFO("reset tx log counters\n");
		}
	}
	return count;
}

int proc_get_int_logs(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rtw_wifi_role_t *phl_role = padapter->phl_role;
	u32 bcn_ok = 0;
	u32 bcn_fail = 0;

	if (!phl_role || !phl_role->bcn_cmn.bcn_added)
		return 0;

#ifdef CONFIG_RTW_DEBUG_BCN_TX
	#if (PHL_VER_CODE >= PHL_VERSION(0001, 0017, 0000, 0000))
	do {
		struct rtw_bcn_stats *bcn_stats;
		rtw_phl_get_beacon_cnt(GET_HAL_INFO(padapter->dvobj), padapter->iface_id,
	        	               &bcn_stats);
			               bcn_ok = bcn_stats->bcn_ok;
		bcn_fail = bcn_stats->bcn_fail;
	} while (0);
	#else
	rtw_phl_get_beacon_cnt(GET_HAL_INFO(padapter->dvobj), padapter->iface_id, &bcn_ok, &bcn_fail);
	#endif /* PHL_VER_CODE */
#endif /* CONFIG_RTW_DEBUG_BCN_TX */
	RTW_PRINT_SEL(m,
		      "rx_rdu=%d \n"
		      "bcn_ok=%d \n"
		      "bcn_fail=%d \n",
		      rtw_phl_get_hw_cnt_rdu(GET_HAL_INFO(padapter->dvobj)),
		      bcn_ok,
		      bcn_fail
		      );
	return 0;
}

#ifdef CONFIG_RTW_DEBUG_BCN_STATS
int proc_get_bcn_stats(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	#ifndef CONFIG_RTW_LINK_PHL_MASTER
	// 13188df14ef9052b13ac186f025859cf157b2132 skylin
	rtw_phl_dump_beacon_stats(GET_HAL_INFO(padapter->dvobj), 0);
	#endif /* CONFIG_RTW_LINK_PHL_MASTER */
	return 0;
}

ssize_t proc_set_bcn_stats(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 reset;
	int num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num = sscanf(tmp, "%d ", &reset);
		if (num == 1 && reset == 0) {
			RTW_INFO("reset beacon stats\n");
			#ifndef CONFIG_RTW_LINK_PHL_MASTER
			// 13188df14ef9052b13ac186f025859cf157b2132 skylin
			rtw_phl_dump_beacon_stats(GET_HAL_INFO(padapter->dvobj), 1);
			#endif /* CONFIG_RTW_LINK_PHL_MASTER */
		}
	}

	return count;
}
#endif /* CONFIG_RTW_DEBUG_BCN_STATS */

#endif /* CONFIG_DBG_COUNTER */

int proc_get_hw_status(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;
	struct registry_priv *regsty = dvobj_to_regsty(dvobj);

	if (regsty->check_hw_status == 0)
		RTW_PRINT_SEL(m, "RX FIFO full count: not check in watch dog\n");
	else if (pdbgpriv->dbg_rx_fifo_last_overflow == 1
	    && pdbgpriv->dbg_rx_fifo_curr_overflow == 1
	    && pdbgpriv->dbg_rx_fifo_diff_overflow == 1
	   )
		RTW_PRINT_SEL(m, "RX FIFO full count: no implementation\n");
	else {
		RTW_PRINT_SEL(m, "RX FIFO full count: last_time=%llu, current_time=%llu, differential=%llu\n"
			, pdbgpriv->dbg_rx_fifo_last_overflow, pdbgpriv->dbg_rx_fifo_curr_overflow, pdbgpriv->dbg_rx_fifo_diff_overflow);
	}

	return 0;
}

ssize_t proc_set_hw_status(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct registry_priv *regsty = dvobj_to_regsty(dvobj);
	char tmp[32];
	u32 enable;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &enable);

		if (num == 1 && regsty && enable <= 1) {
			regsty->check_hw_status = enable;
			RTW_INFO("check_hw_status=%d\n", regsty->check_hw_status);
		}
	}

	return count;
}

int proc_get_trx_info_debug(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	/*============  tx info ============	*/
	rtw_hal_get_def_var(padapter, HW_DEF_RA_INFO_DUMP, m);

	/*============  rx info ============	*/
	rtw_hal_set_phydm_var(padapter, HAL_PHYDM_RX_INFO_DUMP, m, _FALSE);

	return 0;
}

int proc_get_rx_signal(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

#ifdef CONFIG_SNR_RPT
	_list	*plist, *phead;
	struct sta_info *psta = NULL;
	u8 sta_mac[NUM_STA][ETH_ALEN] = {{0}};
	uint mac_id[NUM_STA];
	struct stainfo_stats	*pstats = NULL;
	struct sta_priv	*pstapriv = &(padapter->stapriv);
	u32 i, j, macid_rec_idx = 0;
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 null_addr[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);
			if ((_rtw_memcmp(psta->phl_sta->mac_addr, bc_addr, 6) !=  _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, null_addr, 6) != _TRUE)
				&& (_rtw_memcmp(psta->phl_sta->mac_addr, adapter_mac_addr(padapter), 6) != _TRUE)) {
				_rtw_memcpy(&sta_mac[macid_rec_idx][0], psta->phl_sta->mac_addr, ETH_ALEN);
				mac_id[macid_rec_idx] = psta->phl_sta->macid;
				macid_rec_idx++;
			}
		}
	}
	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	for (i = 0; i < macid_rec_idx; i++) {
		psta = rtw_get_stainfo(pstapriv, &sta_mac[i][0]);
		if(psta) {
			/*ToDo, base on the real number of rf path to show the information*/
			RTW_PRINT_SEL(m, "STA:"MAC_FMT" SNR:{%d, %d, %d, %d}\n", MAC_ARG(psta->phl_sta->mac_addr),
				psta->snr_fd_avg[0], psta->snr_fd_avg[1], psta->snr_fd_avg[2], psta->snr_fd_avg[3]);
		} else {
			RTW_INFO("STA is gone\n");
		}
	}
#endif /* CONFIG_SNR_RPT */

	RTW_PRINT_SEL(m, "rssi:%d\n", padapter->recvpriv.rssi);
#if 0//def CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1) {
		struct dm_struct *odm = adapter_to_phydm(padapter);
		if (padapter->mppriv.antenna_rx == ANTENNA_A)
			RTW_PRINT_SEL(m, "Antenna: A\n");
		else if (padapter->mppriv.antenna_rx == ANTENNA_B)
			RTW_PRINT_SEL(m, "Antenna: B\n");
		else if (padapter->mppriv.antenna_rx == ANTENNA_C)
			RTW_PRINT_SEL(m, "Antenna: C\n");
		else if (padapter->mppriv.antenna_rx == ANTENNA_D)
			RTW_PRINT_SEL(m, "Antenna: D\n");
		else if (padapter->mppriv.antenna_rx == ANTENNA_AB)
			RTW_PRINT_SEL(m, "Antenna: AB\n");
		else if (padapter->mppriv.antenna_rx == ANTENNA_BC)
			RTW_PRINT_SEL(m, "Antenna: BC\n");
		else if (padapter->mppriv.antenna_rx == ANTENNA_CD)
			RTW_PRINT_SEL(m, "Antenna: CD\n");
		else
			RTW_PRINT_SEL(m, "Antenna: __\n");

		RTW_PRINT_SEL(m, "rx_rate = %s\n", HDATA_RATE(odm->rx_rate));
		return 0;
	} else
#endif
	{
		/* RTW_PRINT_SEL(m, "rxpwdb:%d\n", padapter->recvpriv.rxpwdb); */
		RTW_PRINT_SEL(m, "signal_strength:%u\n", padapter->recvpriv.signal_strength);
		RTW_PRINT_SEL(m, "signal_qual:%u\n", padapter->recvpriv.signal_qual);
	}
#ifdef DBG_RX_SIGNAL_DISPLAY_RAW_DATA
	rtw_odm_get_perpkt_rssi(m, padapter);
	rtw_get_raw_rssi_info(m, padapter);
#endif
	return 0;
}

ssize_t proc_set_rx_signal(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 is_signal_dbg, signal_strength;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%u %u", &is_signal_dbg, &signal_strength);

		if (num < 1)
			return count;

		is_signal_dbg = is_signal_dbg == 0 ? 0 : 1;

		if (is_signal_dbg && num < 2)
			return count;

		signal_strength = signal_strength > 100 ? 100 : signal_strength;

		padapter->recvpriv.is_signal_dbg = is_signal_dbg;
		padapter->recvpriv.signal_strength_dbg = signal_strength;

		if (is_signal_dbg)
			RTW_INFO("set %s %u\n", "DBG_SIGNAL_STRENGTH", signal_strength);
		else
			RTW_INFO("set %s\n", "HW_SIGNAL_STRENGTH");

	}

	return count;

}

int proc_get_mac_rptbuf(struct seq_file *m, void *v)
{
	return 0;
}

#ifdef CONFIG_80211N_HT

int proc_get_ht_enable(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "%d\n", pregpriv->ht_enable);

	return 0;
}

ssize_t proc_set_ht_enable(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &mode);

		if ( num == 1 && pregpriv && mode < 2) {
			pregpriv->ht_enable = mode;
			RTW_INFO("ht_enable=%d\n", pregpriv->ht_enable);
		}
	}

	return count;

}

int proc_get_bw_mode(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "0x%02x\n", pregpriv->bw_mode);

	return 0;
}

ssize_t proc_set_bw_mode(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;
	u8 bw_2g;
	u8 bw_5g;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%x ", &mode);
		bw_5g = mode >> 4;
		bw_2g = mode & 0x0f;

		if (num == 1 && pregpriv && bw_2g <= 4 && bw_5g <= 4) {
			pregpriv->bw_mode = mode;
			_dbgdump("bw_mode=0x%x\n", mode);
		}
	}

	return count;

}

int proc_get_ampdu_enable(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "%d\n", pregpriv->ampdu_enable);

	return 0;
}

ssize_t proc_set_ampdu_enable(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &mode);

		if (num == 1 && pregpriv && mode < 2) {
			pregpriv->ampdu_enable = mode;
			_dbgdump("ampdu_enable=%d\n", mode);
		}

	}

	return count;

}


void dump_regsty_rx_ampdu_size_limit(void *sel, _adapter *adapter)
{
	struct registry_priv *regsty = adapter_to_regsty(adapter);
	int i;

	RTW_PRINT_SEL(sel, "%-3s %-3s %-3s %-3s %-4s\n"
		, "", "20M", "40M", "80M", "160M");
	for (i = 0; i < 4; i++)
		RTW_PRINT_SEL(sel, "%dSS %3u %3u %3u %4u\n", i + 1
			, regsty->rx_ampdu_sz_limit_by_nss_bw[i][0]
			, regsty->rx_ampdu_sz_limit_by_nss_bw[i][1]
			, regsty->rx_ampdu_sz_limit_by_nss_bw[i][2]
			, regsty->rx_ampdu_sz_limit_by_nss_bw[i][3]);
}

int proc_get_rx_ampdu(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	_RTW_PRINT_SEL(m, "accept: ");
	if (padapter->fix_rx_ampdu_accept == RX_AMPDU_ACCEPT_INVALID)
		RTW_PRINT_SEL(m, "%u%s\n", rtw_rx_ampdu_is_accept(padapter), "(auto)");
	else
		RTW_PRINT_SEL(m, "%u%s\n", padapter->fix_rx_ampdu_accept, "(fixed)");

	_RTW_PRINT_SEL(m, "size: ");
	if (padapter->fix_rx_ampdu_size == RX_AMPDU_SIZE_INVALID) {
		RTW_PRINT_SEL(m, "%u%s\n", rtw_rx_ampdu_size(padapter), "(auto) with conditional limit:");
		dump_regsty_rx_ampdu_size_limit(m, padapter);
	} else
		RTW_PRINT_SEL(m, "%u%s\n", padapter->fix_rx_ampdu_size, "(fixed)");
	RTW_PRINT_SEL(m, "\n");

	RTW_PRINT_SEL(m, "%19s %17s\n", "fix_rx_ampdu_accept", "fix_rx_ampdu_size");

	_RTW_PRINT_SEL(m, "%-19d %-17u\n"
		, padapter->fix_rx_ampdu_accept
		, padapter->fix_rx_ampdu_size);

	return 0;
}

ssize_t proc_set_rx_ampdu(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 accept;
	u8 size;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %hhu", &accept, &size);

		if (num >= 1)
			rtw_rx_ampdu_set_accept(padapter, accept, RX_AMPDU_DRV_FIXED);
		if (num >= 2)
			rtw_rx_ampdu_set_size(padapter, size, RX_AMPDU_DRV_FIXED);

		rtw_rx_ampdu_apply(padapter);
	}

	return count;
}

int proc_get_rx_ampdu_factor(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);


	if (padapter) {
		RTW_PRINT_SEL(m, "cap->num_ampdu = %d\n", padapter->phl_role->proto_role_cap.num_ampdu);
		RTW_PRINT_SEL(m, "rx_ampdu_agg_num = %d (0x%x)(def:0xff)\n",
			padapter->driver_rx_ampdu_factor, padapter->driver_rx_ampdu_factor);/* RX_AMPDU_SIZE_INVALID */
	}

	return 0;
}

ssize_t proc_set_rx_ampdu_factor(struct file *file, const char __user *buffer
				 , size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 factor;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%x ", &factor);

		if (padapter && (num == 1)) {
			if (factor  >= 256) {
				RTW_PRINT("manual rx_max_agg_num max is 254(0xFE), %d is over length\n", factor);
			} else
				padapter->driver_rx_ampdu_factor = factor;

			RTW_PRINT("set rx_ampdu_agg_num = %d (0x%x)(def:0xff)\n",
				padapter->driver_rx_ampdu_factor, padapter->driver_rx_ampdu_factor);/* RX_AMPDU_SIZE_INVALID */
		}
	}

	return count;
}

int proc_get_tx_max_agg_num(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);


	if (padapter) {
		RTW_PRINT_SEL(m, "cap->num_ampdu = %d\n", padapter->phl_role->proto_role_cap.num_ampdu);
		RTW_PRINT_SEL(m, "tx_max_agg_num = %d (0x%x)(def:0xffff)\n",
			padapter->driver_tx_max_agg_num, padapter->driver_tx_max_agg_num);
	}

	return 0;
}

ssize_t proc_set_tx_max_agg_num(struct file *file, const char __user *buffer
				 , size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u16 agg_num;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hx ", &agg_num);

		if (padapter && (num == 1)) {

			if (agg_num <= 256 || agg_num == 0xffff) {
				padapter->driver_tx_max_agg_num = agg_num;
				RTW_PRINT("set tx_max_agg_num = %d (0x%x)(def:0xffff)\n", agg_num, agg_num);
			} else
				RTW_PRINT("tx_max_agg_num max is 256(0x100), %d is over length\n", agg_num);
		}
	}

	return count;
}

int proc_get_rx_ampdu_density(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);


	if (padapter)
		RTW_PRINT_SEL(m, "rx ampdu densityg = 0x%x\n", padapter->driver_rx_ampdu_spacing);

	return 0;
}

ssize_t proc_set_rx_ampdu_density(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 density;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &density);

		if (padapter && (num == 1)) {
			RTW_INFO("padapter->driver_rx_ampdu_spacing = %x\n", density);

			if (density > 0x07)
				padapter->driver_rx_ampdu_spacing = 0xFF;
			else
				padapter->driver_rx_ampdu_spacing = density;
		}
	}

	return count;
}

int proc_get_rx_amsdu(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;
	extern int rtw_rx_ampdu_amsdu;

	if (padapter)
		RTW_PRINT_SEL(m, "rx amsdu = %d (def: %d)\n",
				pregpriv->rx_ampdu_amsdu, rtw_rx_ampdu_amsdu);

	return 0;
}

ssize_t proc_set_rx_amsdu(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 enable = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &enable);

		if (padapter && (num == 1)) {
			if (enable < 2) {
				pregpriv->rx_ampdu_amsdu = enable;
				RTW_PRINT("set rx amsdu = %x\n", pregpriv->rx_ampdu_amsdu);
			} else
				RTW_PRINT("Invalid parameter: %d\n", enable);
		}
	}

	return count;
}

int proc_get_rx_amsdu_size(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct vht_priv *pvhtpriv = &pmlmepriv->vhtpriv;

	if (padapter)
		RTW_PRINT_SEL(m, "rx amsdu size = 0x%x (ie: 0x%x, cap: 0x%x) (0: 3.9K, 1: 7.9K, 2: 11.5K)\n",
				padapter->driver_rx_amsdu_size, pvhtpriv->max_mpdu_len, padapter->phl_role->proto_role_cap.max_amsdu_len);

	return 0;
}

ssize_t proc_set_rx_amsdu_size(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	char tmp[32];
	u32 size, max_size;
	u32 rx_buf_size = (rfctl->max_chan_nums <= MAX_CHANNEL_NUM_2G) ? RXQ_1_BASE_BUF_SIZE : RXQ_0_BASE_BUF_SIZE;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	#if 0 /* wait util g6 can receive 11.5K amsdu */
	if (rx_buf_size > 12000)
		max_size = 2; //11.5K
	else
	#endif
	if (rx_buf_size > 8000)
		max_size = 1; //7.9K
	else
		max_size = 0; //3.9K

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &size);

		if (num == 1) {

			if (size > 2)
				padapter->driver_rx_amsdu_size = 0xF;
			else {
				if (size > max_size) {
					padapter->driver_rx_amsdu_size = max_size;
					RTW_PRINT("set rx amsdu size=%d is over rx_buf_size(%d) !!\n", size, max_size);
				} else
					padapter->driver_rx_amsdu_size = size;
			}
			RTW_PRINT("set rx amsdu size = %x\n", padapter->driver_rx_amsdu_size);
		}
	}

	return count;
}


int proc_get_tx_ampdu_density(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);


	if (padapter)
		RTW_PRINT_SEL(m, "tx ampdu density = %x\n", padapter->driver_ampdu_spacing);

	return 0;
}

ssize_t proc_set_tx_ampdu_density(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 density;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &density);

		if (padapter && (num == 1)) {
			RTW_INFO("padapter->driver_ampdu_spacing = %x\n", density);

			if (density > 0x07)
				padapter->driver_ampdu_spacing = 0xFF;
			else
				padapter->driver_ampdu_spacing = density;
		}
	}

	return count;
}

int proc_get_tx_quick_addba_req(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (padapter)
		RTW_PRINT_SEL(m, "tx_quick_addba_req = %x\n", pregpriv->tx_quick_addba_req);

	return 0;
}

ssize_t proc_set_tx_quick_addba_req(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 enable;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &enable);

		if (padapter && (num == 1)) {
			pregpriv->tx_quick_addba_req = enable;
			RTW_INFO("tx_quick_addba_req = %d\n", pregpriv->tx_quick_addba_req);
		}
	}

	return count;
}
#ifdef CONFIG_TX_AMSDU
int proc_get_tx_amsdu(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	if (padapter)
	{
		RTW_PRINT_SEL(m, "tx amsdu = %d\n", padapter->tx_amsdu);
		RTW_PRINT_SEL(m, "amsdu set timer conut = %u\n", pxmitpriv->amsdu_debug_set_timer);
		RTW_PRINT_SEL(m, "amsdu  time out count = %u\n", pxmitpriv->amsdu_debug_timeout);
		RTW_PRINT_SEL(m, "amsdu coalesce one count = %u\n", pxmitpriv->amsdu_debug_coalesce_one);
		RTW_PRINT_SEL(m, "amsdu coalesce two count = %u\n", pxmitpriv->amsdu_debug_coalesce_two);
		RTW_PRINT_SEL(m, "amsdu tasklet count = %u\n", pxmitpriv->amsdu_debug_tasklet);
		RTW_PRINT_SEL(m, "amsdu enqueue count = %u\n", pxmitpriv->amsdu_debug_enqueue);
		RTW_PRINT_SEL(m, "amsdu dequeue count = %u\n", pxmitpriv->amsdu_debug_dequeue);
	}

	return 0;
}

ssize_t proc_set_tx_amsdu(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	char tmp[32];
	u32 amsdu;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%d ", &amsdu);
		if (padapter && (num == 1)) {
			RTW_INFO("[%s] amsdu = %x\n", __func__, amsdu);
			if (amsdu <= AMSDU_MAX_NUM) {
				padapter->tx_amsdu = amsdu;
			} else if(amsdu == 0xff) {
				pxmitpriv->amsdu_debug_set_timer = 0;
				pxmitpriv->amsdu_debug_timeout = 0;
				pxmitpriv->amsdu_debug_coalesce_one = 0;
				pxmitpriv->amsdu_debug_coalesce_two = 0;
				pxmitpriv->amsdu_debug_tasklet = 0;
				pxmitpriv->amsdu_debug_enqueue = 0;
				pxmitpriv->amsdu_debug_dequeue = 0;
			}
		}
	}

	return count;
}

int proc_get_tx_amsdu_rate(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	if (padapter)
		RTW_PRINT_SEL(m, "tx amsdu rate = %d Mbps\n", padapter->tx_amsdu_rate);

	return 0;
}

ssize_t proc_set_tx_amsdu_rate(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 amsdu_rate;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &amsdu_rate);

		if (padapter && (num == 1)) {
			RTW_INFO("padapter->tx_amsdu_rate = %x\n", amsdu_rate);
			padapter->tx_amsdu_rate = amsdu_rate;
		}
	}

	return count;
}
#endif /* CONFIG_TX_AMSDU */
#endif /* CONFIG_80211N_HT */
ssize_t proc_set_dyn_rrsr(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;

	char tmp[32] = {0};
	u32 num = 0, enable = 0, rrsr_val = 0; /* gpio_mode:0 input  1:output; */

	if (count < 2)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num	= sscanf(tmp, "%d 0x%x", &enable, &rrsr_val);
		RTW_INFO("num=%u enable=%d rrsr_val=0x%x\n", num, enable, rrsr_val);
		pregpriv->en_dyn_rrsr = enable;
		pregpriv->set_rrsr_value = rrsr_val;
		rtw_phydm_dyn_rrsr_en(padapter, enable);
		rtw_phydm_set_rrsr(padapter, rrsr_val, _TRUE);

	}
	return count;

}
int proc_get_dyn_rrsr(struct seq_file *m, void *v) {

	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;
	u32 init_rrsr =0xFFFFFFFF;

	if (padapter)
		RTW_PRINT_SEL(m, "en_dyn_rrsr = %d fixed_rrsr_value =0x%x %s\n"
			, pregpriv->en_dyn_rrsr
			, pregpriv->set_rrsr_value
			, (pregpriv->set_rrsr_value == init_rrsr)?"(default)":"(fixed)"
		);

	return 0;
}
int proc_get_en_fwps(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "check_fw_ps = %d , 1:enable get FW PS state , 0: disable get FW PS state\n"
			      , pregpriv->check_fw_ps);

	return 0;
}

ssize_t proc_set_en_fwps(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &mode);

		if (num == 1 && pregpriv &&  mode < 2) {
			pregpriv->check_fw_ps = mode;
			RTW_INFO("pregpriv->check_fw_ps=%d\n", pregpriv->check_fw_ps);
		}

	}

	return count;
}

/*
int proc_get_two_path_rssi(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	if(padapter)
		RTW_PRINT_SEL(m, "%d %d\n",
			padapter->recvpriv.RxRssi[0], padapter->recvpriv.RxRssi[1]);

	return 0;
}
*/
#ifdef CONFIG_80211N_HT
void rtw_dump_dft_phy_cap(void *sel, _adapter *adapter)
{
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;
	struct ht_priv	*phtpriv = &pmlmepriv->htpriv;
	#ifdef CONFIG_80211AC_VHT
	struct vht_priv *pvhtpriv = &pmlmepriv->vhtpriv;
	#endif

	#ifdef CONFIG_80211AC_VHT
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT STBC Tx : %s\n", (TEST_FLAG(pvhtpriv->stbc_cap, STBC_VHT_ENABLE_TX)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT STBC Rx : %s\n", (TEST_FLAG(pvhtpriv->stbc_cap, STBC_VHT_ENABLE_RX)) ? "V" : "X");
	#endif
	RTW_PRINT_SEL(sel, "[DFT CAP] HT STBC Tx : %s\n", (TEST_FLAG(phtpriv->stbc_cap, STBC_HT_ENABLE_TX)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] HT STBC Rx : %s\n\n", (TEST_FLAG(phtpriv->stbc_cap, STBC_HT_ENABLE_RX)) ? "V" : "X");

	#ifdef CONFIG_80211AC_VHT
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT LDPC Tx : %s\n", (TEST_FLAG(pvhtpriv->ldpc_cap, LDPC_VHT_ENABLE_TX)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT LDPC Rx : %s\n", (TEST_FLAG(pvhtpriv->ldpc_cap, LDPC_VHT_ENABLE_RX)) ? "V" : "X");
	#endif
	RTW_PRINT_SEL(sel, "[DFT CAP] HT LDPC Tx : %s\n", (TEST_FLAG(phtpriv->ldpc_cap, LDPC_HT_ENABLE_TX)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] HT LDPC Rx : %s\n\n", (TEST_FLAG(phtpriv->ldpc_cap, LDPC_HT_ENABLE_RX)) ? "V" : "X");

	#ifdef CONFIG_BEAMFORMING
	#ifdef CONFIG_80211AC_VHT
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT MU Bfer : %s\n", (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_MU_MIMO_AP_ENABLE)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT MU Bfee : %s\n", (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_MU_MIMO_STA_ENABLE)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT SU Bfer : %s\n", (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_BEAMFORMER_ENABLE)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] VHT SU Bfee : %s\n", (TEST_FLAG(pvhtpriv->beamform_cap, BEAMFORMING_VHT_BEAMFORMEE_ENABLE)) ? "V" : "X");
	#endif
	RTW_PRINT_SEL(sel, "[DFT CAP] HT Bfer : %s\n", (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMER_ENABLE))  ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DFT CAP] HT Bfee : %s\n", (TEST_FLAG(phtpriv->beamform_cap, BEAMFORMING_HT_BEAMFORMEE_ENABLE)) ? "V" : "X");
	#endif
}

void rtw_get_dft_phy_cap(void *sel, _adapter *adapter)
{
	RTW_PRINT_SEL(sel, "\n ======== PHY CAP protocol ========\n");
	rtw_ht_use_default_setting(adapter);
	#ifdef CONFIG_RTW_AP_EXT_SUPPORT
	rtw_ht_apply_mib_setting(adapter);
	#endif
	#ifdef CONFIG_80211AC_VHT
	rtw_vht_use_default_setting(adapter);
	#endif
	#ifdef CONFIG_RTW_AP_EXT_SUPPORT
	rtw_vht_apply_mib_setting(adapter);
	#endif
	#ifdef CONFIG_80211N_HT
	rtw_dump_dft_phy_cap(sel, adapter);
	#endif
}

void rtw_dump_drv_phy_cap(void *sel, _adapter *adapter)
{
	struct registry_priv	*pregistry_priv = &adapter->registrypriv;

	RTW_PRINT_SEL(sel, "\n ======== DRV's configuration ========\n");
	#if 0
	RTW_PRINT_SEL(sel, "[DRV CAP] TRx Capability : 0x%08x\n", phy_spec->trx_cap);
	RTW_PRINT_SEL(sel, "[DRV CAP] Tx Stream Num Index : %d\n", (phy_spec->trx_cap >> 24) & 0xFF); /*Tx Stream Num Index [31:24]*/
	RTW_PRINT_SEL(sel, "[DRV CAP] Rx Stream Num Index : %d\n", (phy_spec->trx_cap >> 16) & 0xFF); /*Rx Stream Num Index [23:16]*/
	RTW_PRINT_SEL(sel, "[DRV CAP] Tx Path Num Index : %d\n", (phy_spec->trx_cap >> 8) & 0xFF);/*Tx Path Num Index	[15:8]*/
	RTW_PRINT_SEL(sel, "[DRV CAP] Rx Path Num Index : %d\n", (phy_spec->trx_cap & 0xFF));/*Rx Path Num Index	[7:0]*/
	#endif
	#ifdef CONFIG_80211N_HT
	RTW_PRINT_SEL(sel, "[DRV CAP] STBC Capability : 0x%02x\n", pregistry_priv->stbc_cap);
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT STBC Tx : %s\n", (TEST_FLAG(pregistry_priv->stbc_cap, BIT1)) ? "V" : "X"); /*BIT1: Enable VHT STBC Tx*/
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT STBC Rx : %s\n", (TEST_FLAG(pregistry_priv->stbc_cap, BIT0)) ? "V" : "X"); /*BIT0: Enable VHT STBC Rx*/
	RTW_PRINT_SEL(sel, "[DRV CAP] HT STBC Tx : %s\n", (TEST_FLAG(pregistry_priv->stbc_cap, BIT5)) ? "V" : "X"); /*BIT5: Enable HT STBC Tx*/
	RTW_PRINT_SEL(sel, "[DRV CAP] HT STBC Rx : %s\n\n", (TEST_FLAG(pregistry_priv->stbc_cap, BIT4)) ? "V" : "X"); /*BIT4: Enable HT STBC Rx*/

	RTW_PRINT_SEL(sel, "[DRV CAP] LDPC Capability : 0x%02x\n", pregistry_priv->ldpc_cap);
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT LDPC Tx : %s\n", (TEST_FLAG(pregistry_priv->ldpc_cap, BIT1)) ? "V" : "X"); /*BIT1: Enable VHT LDPC Tx*/
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT LDPC Rx : %s\n", (TEST_FLAG(pregistry_priv->ldpc_cap, BIT0)) ? "V" : "X"); /*BIT0: Enable VHT LDPC Rx*/
	RTW_PRINT_SEL(sel, "[DRV CAP] HT LDPC Tx : %s\n", (TEST_FLAG(pregistry_priv->ldpc_cap, BIT5)) ? "V" : "X"); /*BIT5: Enable HT LDPC Tx*/
	RTW_PRINT_SEL(sel, "[DRV CAP] HT LDPC Rx : %s\n\n", (TEST_FLAG(pregistry_priv->ldpc_cap, BIT4)) ? "V" : "X"); /*BIT4: Enable HT LDPC Rx*/
	#endif /* CONFIG_80211N_HT */
	#ifdef CONFIG_BEAMFORMING
	#if 0
	RTW_PRINT_SEL(sel, "[DRV CAP] TxBF parameter : 0x%08x\n", phy_spec->txbf_param);
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT Sounding Dim : %d\n", (phy_spec->txbf_param >> 24) & 0xFF); /*VHT Sounding Dim [31:24]*/
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT Steering Ant : %d\n", (phy_spec->txbf_param >> 16) & 0xFF); /*VHT Steering Ant [23:16]*/
	RTW_PRINT_SEL(sel, "[DRV CAP] HT Sounding Dim : %d\n", (phy_spec->txbf_param >> 8) & 0xFF); /*HT Sounding Dim [15:8]*/
	RTW_PRINT_SEL(sel, "[DRV CAP] HT Steering Ant : %d\n", phy_spec->txbf_param & 0xFF); /*HT Steering Ant [7:0]*/
	#endif

	/*
	 * BIT0: Enable VHT SU Beamformer
	 * BIT1: Enable VHT SU Beamformee
	 * BIT2: Enable VHT MU Beamformer, depend on VHT SU Beamformer
	 * BIT3: Enable VHT MU Beamformee, depend on VHT SU Beamformee
	 * BIT4: Enable HT Beamformer
	 * BIT5: Enable HT Beamformee
	 */
	RTW_PRINT_SEL(sel, "[DRV CAP] TxBF Capability : 0x%02x\n", pregistry_priv->beamform_cap);
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT MU Bfer : %s\n", (TEST_FLAG(pregistry_priv->beamform_cap, BIT2)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT MU Bfee : %s\n", (TEST_FLAG(pregistry_priv->beamform_cap, BIT3)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT SU Bfer : %s\n", (TEST_FLAG(pregistry_priv->beamform_cap, BIT0)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DRV CAP] VHT SU Bfee : %s\n", (TEST_FLAG(pregistry_priv->beamform_cap, BIT1)) ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DRV CAP] HT Bfer : %s\n", (TEST_FLAG(pregistry_priv->beamform_cap, BIT4))  ? "V" : "X");
	RTW_PRINT_SEL(sel, "[DRV CAP] HT Bfee : %s\n", (TEST_FLAG(pregistry_priv->beamform_cap, BIT5)) ? "V" : "X");

	RTW_PRINT_SEL(sel, "[DRV CAP] Tx Bfer rf_num : %d\n", pregistry_priv->beamformer_rf_num);
	RTW_PRINT_SEL(sel, "[DRV CAP] Tx Bfee rf_num : %d\n", pregistry_priv->beamformee_rf_num);
	#endif
}

int proc_get_stbc_cap(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "0x%02x\n", pregpriv->stbc_cap);

	return 0;
}

ssize_t proc_set_stbc_cap(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &mode);

		if (num == 1 && pregpriv) {
			pregpriv->stbc_cap = mode;
			RTW_INFO("stbc_cap = 0x%02x\n", mode);
		}
	}

	return count;
}
int proc_get_rx_stbc(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "%d\n", pregpriv->rx_stbc);

	return 0;
}

ssize_t proc_set_rx_stbc(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &mode);

		if (num == 1 && pregpriv && (mode == 0 || mode == 1 || mode == 2 || mode == 3)) {
			pregpriv->rx_stbc = mode;
			_dbgdump("rx_stbc=%d\n", mode);
		}
	}

	return count;

}
int proc_get_ldpc_cap(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "0x%02x\n", pregpriv->ldpc_cap);

	return 0;
}

ssize_t proc_set_ldpc_cap(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &mode);

		if (num == 1 && pregpriv) {
			pregpriv->ldpc_cap = mode;
			RTW_INFO("ldpc_cap = 0x%02x\n", mode);
		}
	}

	return count;
}
#ifdef CONFIG_BEAMFORMING
int proc_get_txbf_cap(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "0x%02x\n", pregpriv->beamform_cap);

	return 0;
}

ssize_t proc_set_txbf_cap(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 mode;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &mode);

		if (num == 1 && pregpriv) {
			pregpriv->beamform_cap = mode;
			RTW_INFO("beamform_cap = 0x%02x\n", mode);
		}
	}

	return count;
}
#endif
#endif /* CONFIG_80211N_HT */

/*int proc_get_rssi_disp(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	return 0;
}
*/

/*ssize_t proc_set_rssi_disp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 enable=0;

	if (count < 1)
	{
		RTW_INFO("argument size is less than 1\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%x", &enable);

		if (num !=  1) {
			RTW_INFO("invalid set_rssi_disp parameter!\n");
			return count;
		}

		if(enable)
		{
			RTW_INFO("Linked info Function Enable\n");
			padapter->bLinkInfoDump = enable ;
		}
		else
		{
			RTW_INFO("Linked info Function Disable\n");
			padapter->bLinkInfoDump = 0 ;
		}

	}

	return count;

}

*/

#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
int proc_get_rtkm_info(struct seq_file *m, void *v)
{
#ifdef CONFIG_USB_HCI
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	struct recv_buf *precvbuf;

	precvbuf = (struct recv_buf *)precvpriv->precv_buf;
#endif /* CONFIG_USB_HCI */

	RTW_PRINT_SEL(m, "============[RTKM Info]============\n");
	RTW_PRINT_SEL(m, "MAX_RTKM_NR_PREALLOC_RECV_SKB: %d\n", rtw_rtkm_get_nr_recv_skb());
	RTW_PRINT_SEL(m, "MAX_RTKM_RECVBUF_SZ: %d\n", rtw_rtkm_get_buff_size());

	RTW_PRINT_SEL(m, "============[Driver Info]============\n");
	RTW_PRINT_SEL(m, "NR_PREALLOC_RECV_SKB: %d\n", NR_PREALLOC_RECV_SKB);
#ifdef CONFIG_USB_HCI
	RTW_PRINT_SEL(m, "MAX_RECVBUF_SZ: %d\n", precvbuf->alloc_sz);
#else /* !CONFIG_USB_HCI */
	RTW_PRINT_SEL(m, "MAX_RECVBUF_SZ: %d\n", MAX_RECVBUF_SZ);
#endif /* !CONFIG_USB_HCI */

	return 0;
}
#endif /* CONFIG_PREALLOC_RX_SKB_BUFFER */

#ifdef DBG_MEMORY_LEAK
#include <asm/atomic.h>
extern ATOMIC_T _malloc_cnt;
extern ATOMIC_T _malloc_size;

int proc_get_malloc_cnt(struct seq_file *m, void *v)
{
	RTW_PRINT_SEL(m, "_malloc_cnt=%d\n", atomic_read(&_malloc_cnt));
	RTW_PRINT_SEL(m, "_malloc_size=%d\n", atomic_read(&_malloc_size));

	return 0;
}
#endif /* DBG_MEMORY_LEAK */

#ifdef CONFIG_FIND_BEST_CHANNEL
int proc_get_best_channel(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	u32 i, best_channel_24G = 1, best_channel_5G = 36, index_24G = 0, index_5G = 0;

	for (i = 0; i < rfctl->max_chan_nums && rfctl->channel_set[i].ChannelNum != 0; i++) {
		if (rfctl->channel_set[i].ChannelNum == 1)
			index_24G = i;
		if (rfctl->channel_set[i].ChannelNum == 36)
			index_5G = i;
	}

	for (i = 0; i < rfctl->max_chan_nums && rfctl->channel_set[i].ChannelNum != 0; i++) {
		/* 2.4G */
		if (rfctl->channel_set[i].ChannelNum == 6) {
			if (rfctl->channel_set[i].rx_count < rfctl->channel_set[index_24G].rx_count) {
				index_24G = i;
				best_channel_24G = rfctl->channel_set[i].ChannelNum;
			}
		}

		/* 5G */
		if (rfctl->channel_set[i].ChannelNum >= 36
		    && rfctl->channel_set[i].ChannelNum < 140) {
			/* Find primary channel */
			if (((rfctl->channel_set[i].ChannelNum - 36) % 8 == 0)
			    && (rfctl->channel_set[i].rx_count < rfctl->channel_set[index_5G].rx_count)) {
				index_5G = i;
				best_channel_5G = rfctl->channel_set[i].ChannelNum;
			}
		}

		if (rfctl->channel_set[i].ChannelNum >= 149
		    && rfctl->channel_set[i].ChannelNum < 165) {
			/* find primary channel */
			if (((rfctl->channel_set[i].ChannelNum - 149) % 8 == 0)
			    && (rfctl->channel_set[i].rx_count < rfctl->channel_set[index_5G].rx_count)) {
				index_5G = i;
				best_channel_5G = rfctl->channel_set[i].ChannelNum;
			}
		}
#if 1 /* debug */
		RTW_PRINT_SEL(m, "The rx cnt of channel %3d = %d\n",
			rfctl->channel_set[i].ChannelNum, rfctl->channel_set[i].rx_count);
#endif
	}

	RTW_PRINT_SEL(m, "best_channel_5G = %d\n", best_channel_5G);
	RTW_PRINT_SEL(m, "best_channel_24G = %d\n", best_channel_24G);

	return 0;
}

ssize_t proc_set_best_channel(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int i;
		for (i = 0; i < rfctl->max_chan_nums && rfctl->channel_set[i].ChannelNum != 0; i++)
			rfctl->channel_set[i].rx_count = 0;

		RTW_INFO("set %s\n", "Clean Best Channel Count");
	}

	return count;
}
#endif /* CONFIG_FIND_BEST_CHANNEL */

#ifdef CONFIG_RTW_2G_40M_COEX
int proc_get_coex_2g_40m(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "%d\n", padapter->registrypriv.coex_2g_40m);

	return 0;
}

ssize_t proc_set_coex_2g_40m(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int coex_2g_40m;
		int num = sscanf(tmp, "%d", &coex_2g_40m);

		if (padapter && (num == 1)) {
			RTW_PRINT(FUNC_ADPT_FMT": coex_2g_40m = %d\n",
			          FUNC_ADPT_ARG(padapter), coex_2g_40m);
			padapter->registrypriv.coex_2g_40m = coex_2g_40m;
		}
	}

	return count;
}
#endif /* CONFIG_RTW_2G_40M_COEX */

#ifdef CONFIG_BTC
int proc_get_btc_dbg(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter;
	char buf[512] = {0};
	padapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_btc_get_dbg(padapter, buf, 512);

	_RTW_PRINT_SEL(m, "%s", buf);

	return 0;
}

ssize_t proc_set_btc_dbg(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter;
	u8 tmp[80] = {0};
	u32 module[2] = {0};
	u32 num;

	padapter = (_adapter *)rtw_netdev_priv(dev);

	/*	RTW_INFO("+" FUNC_ADPT_FMT "\n", FUNC_ADPT_ARG(padapter)); */

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n",
			 FUNC_ADPT_ARG(padapter));

		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n",
			 FUNC_ADPT_ARG(padapter));

		return -EFAULT;
	}

	num = count;
	if (num > (sizeof(tmp) - 1))
		num = (sizeof(tmp) - 1);

	if (copy_from_user(tmp, buffer, num)) {
		RTW_INFO(FUNC_ADPT_FMT ": copy buffer from user space FAIL!\n",
			 FUNC_ADPT_ARG(padapter));

		return -EFAULT;
	}

	num = sscanf(tmp, "%x %x", module, module + 1);
	if (1 == num) {
		if (0 == module[0])
			_rtw_memset(module, 0, sizeof(module));
		else
			_rtw_memset(module, 0xFF, sizeof(module));
	} else if (2 != num) {
		RTW_INFO(FUNC_ADPT_FMT ": input(\"%s\") format incorrect!\n",
			 FUNC_ADPT_ARG(padapter), tmp);

		if (0 == num)
			return -EFAULT;
	}

	RTW_INFO(FUNC_ADPT_FMT ": input 0x%08X 0x%08X\n",
		 FUNC_ADPT_ARG(padapter), module[0], module[1]);
	rtw_btc_set_dbg(padapter, module);

	return count;
}

int proc_get_btc_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter;

	padapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_btc_disp_btc_info(padapter, (void*)m, 0);

	return 0;
}
#endif /* CONFIG_BTC */

#if defined(DBG_CONFIG_ERROR_DETECT)
int proc_get_sreset(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(psdpriv);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	if (psrtpriv->dbg_sreset_ctrl == _TRUE) {
		RTW_PRINT_SEL(m, "self_dect_tx_cnt:%llu\n", psrtpriv->self_dect_tx_cnt);
		RTW_PRINT_SEL(m, "self_dect_rx_cnt:%llu\n", psrtpriv->self_dect_rx_cnt);
		RTW_PRINT_SEL(m, "self_dect_fw_cnt:%llu\n", psrtpriv->self_dect_fw_cnt);
		RTW_PRINT_SEL(m, "tx_dma_status_cnt:%llu\n", psrtpriv->tx_dma_status_cnt);
		RTW_PRINT_SEL(m, "rx_dma_status_cnt:%llu\n", psrtpriv->rx_dma_status_cnt);
		RTW_PRINT_SEL(m, "self_dect_case:%d\n", psrtpriv->self_dect_case);
		RTW_PRINT_SEL(m, "dbg_sreset_cnt:%d\n", pdbgpriv->dbg_sreset_cnt);
	}
	return 0;
}

ssize_t proc_set_sreset(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(adapter_to_dvobj(padapter));
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;
	char tmp[32];
	s32 trigger_point;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d", &trigger_point);

		if (num < 1)
			return count;

		if (trigger_point == SRESET_TGP_NULL)
			rtw_hal_sreset_reset(padapter);
		else if (trigger_point == SRESET_TGP_INFO)
			psrtpriv->dbg_sreset_ctrl = _TRUE;
		else
			sreset_set_trigger_point(padapter, trigger_point);
	}

	return count;

}
#endif /* DBG_CONFIG_ERROR_DETECT */

#ifdef CONFIG_PCI_HCI

ssize_t proc_set_pci_bridge_conf_space(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv       *pdvobjpriv = adapter_to_dvobj(padapter);
	PPCI_DATA pci_data = dvobj_to_pci(pdvobjpriv);
	struct pci_dev  *pdev = pci_data->ppcidev;
	struct pci_dev  *bridge_pdev = pdev->bus->self;

	char tmp[32] = { 0 };
	int num;

	u32 reg = 0, value = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		num = sscanf(tmp, "%x %x", &reg, &value);
		if (num != 2) {
			RTW_INFO("invalid parameter!\n");
			return count;
		}

		if (reg >= 0x1000) {
			RTW_INFO("invalid register!\n");
			return count;
		}

		if (value > 0xFF) {
			RTW_INFO("invalid value! Only one byte\n");
			return count;
		}

		RTW_INFO(FUNC_ADPT_FMT ": register 0x%x value 0x%x\n",
			FUNC_ADPT_ARG(padapter), reg, value);

		pci_write_config_byte(bridge_pdev, reg, value);
	}
	return count;
}


int proc_get_pci_bridge_conf_space(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	struct dvobj_priv       *pdvobjpriv = adapter_to_dvobj(padapter);
	PPCI_DATA pci_data = dvobj_to_pci(pdvobjpriv);
	struct pci_dev  *pdev = pci_data->ppcidev;
	struct pci_dev  *bridge_pdev = NULL;

	u32 tmp[4] = { 0 };
	u32 i, j;

	if (pdev == NULL) {
		RTW_ERR("[%s %d] There's null pointer here!\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	bridge_pdev = pdev->bus->self;

	if (bridge_pdev == NULL) {
		RTW_ERR("[%s %d] There's null pointer here!\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	RTW_PRINT_SEL(m, "\n*****  PCI Host Device Configuration Space*****\n\n");

	for (i = 0; i < 0x1000; i += 0x10) {
		for (j = 0 ; j < 4 ; j++)
			pci_read_config_dword(bridge_pdev, i + j * 4, tmp+j);

		RTW_PRINT_SEL(m, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, tmp[0] & 0xFF, (tmp[0] >> 8) & 0xFF, (tmp[0] >> 16) & 0xFF, (tmp[0] >> 24) & 0xFF,
			tmp[1] & 0xFF, (tmp[1] >> 8) & 0xFF, (tmp[1] >> 16) & 0xFF, (tmp[1] >> 24) & 0xFF,
			tmp[2] & 0xFF, (tmp[2] >> 8) & 0xFF, (tmp[2] >> 16) & 0xFF, (tmp[2] >> 24) & 0xFF,
			tmp[3] & 0xFF, (tmp[3] >> 8) & 0xFF, (tmp[3] >> 16) & 0xFF, (tmp[3] >> 24) & 0xFF);
	}
	return 0;
}


ssize_t proc_set_pci_conf_space(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv       *pdvobjpriv = adapter_to_dvobj(padapter);
	PPCI_DATA pci_data = dvobj_to_pci(pdvobjpriv);
	struct pci_dev  *pdev = pci_data->ppcidev;

	char tmp[32] = { 0 };
	int num;

	u32 reg = 0, value = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		num = sscanf(tmp, "%x %x", &reg, &value);

		if (num != 2) {
			RTW_INFO("invalid parameter!\n");
			return count;
		}


		if (reg >= 0x1000) {
			RTW_INFO("invalid register!\n");
			return count;
		}

		if (value > 0xFF) {
			RTW_INFO("invalid value! Only one byte\n");
			return count;
		}

		RTW_INFO(FUNC_ADPT_FMT ": register 0x%x value 0x%x\n",
			FUNC_ADPT_ARG(padapter), reg, value);

		pci_write_config_byte(pdev, reg, value);


	}
	return count;
}


int proc_get_pci_conf_space(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	struct dvobj_priv       *pdvobjpriv = adapter_to_dvobj(padapter);
	PPCI_DATA pci_data = dvobj_to_pci(pdvobjpriv);
	struct pci_dev *pdev = pci_data->ppcidev;
	struct pci_dev *bridge_pdev = pdev->bus->self;

	u32 tmp[4] = { 0 };
	u32 i, j;

	RTW_PRINT_SEL(m, "\n*****  PCI Device Configuration Space *****\n\n");

	for (i = 0; i < 0x1000; i += 0x10) {
		for (j = 0 ; j < 4 ; j++)
			pci_read_config_dword(pdev, i + j * 4, tmp+j);

		RTW_PRINT_SEL(m, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, tmp[0] & 0xFF, (tmp[0] >> 8) & 0xFF, (tmp[0] >> 16) & 0xFF, (tmp[0] >> 24) & 0xFF,
			tmp[1] & 0xFF, (tmp[1] >> 8) & 0xFF, (tmp[1] >> 16) & 0xFF, (tmp[1] >> 24) & 0xFF,
			tmp[2] & 0xFF, (tmp[2] >> 8) & 0xFF, (tmp[2] >> 16) & 0xFF, (tmp[2] >> 24) & 0xFF,
			tmp[3] & 0xFF, (tmp[3] >> 8) & 0xFF, (tmp[3] >> 16) & 0xFF, (tmp[3] >> 24) & 0xFF);
	}

	return 0;
}


int proc_get_pci_aspm(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	PPCI_DATA pci_data = dvobj_to_pci(pdvobjpriv);
	struct pci_priv	*pcipriv = &(pci_data->pcipriv);
	u8 tmp8 = 0;
	u16 tmp16 = 0;
	u32 tmp32 = 0;
	u8 l1_idle = 0;


	RTW_PRINT_SEL(m, "***** ASPM Capability *****\n");

	pci_read_config_dword(pci_data->ppcidev, pcipriv->pciehdr_offset + PCI_EXP_LNKCAP, &tmp32);

	RTW_PRINT_SEL(m, "CLK REQ:	%s\n", (tmp32&PCI_EXP_LNKCAP_CLKPM) ? "Enable" : "Disable");
	RTW_PRINT_SEL(m, "ASPM L0s:	%s\n", (tmp32&BIT10) ? "Enable" : "Disable");
	RTW_PRINT_SEL(m, "ASPM L1:	%s\n", (tmp32&BIT11) ? "Enable" : "Disable");

	tmp8 = rtw_hal_pci_l1off_capability(padapter);
	RTW_PRINT_SEL(m, "ASPM L1OFF:	%s\n", tmp8 ? "Enable" : "Disable");

	RTW_PRINT_SEL(m, "***** ASPM CTRL Reg *****\n");

	pci_read_config_word(pci_data->ppcidev, pcipriv->pciehdr_offset + PCI_EXP_LNKCTL, &tmp16);

	RTW_PRINT_SEL(m, "CLK REQ:	%s\n", (tmp16&PCI_EXP_LNKCTL_CLKREQ_EN) ? "Enable" : "Disable");
	RTW_PRINT_SEL(m, "ASPM L0s:	%s\n", (tmp16&BIT0) ? "Enable" : "Disable");
	RTW_PRINT_SEL(m, "ASPM L1:	%s\n", (tmp16&BIT1) ? "Enable" : "Disable");

	tmp8 = rtw_hal_pci_l1off_nic_support(padapter);
	RTW_PRINT_SEL(m, "ASPM L1OFF:	%s\n", tmp8 ? "Enable" : "Disable");

	RTW_PRINT_SEL(m, "***** ASPM Backdoor *****\n");

	tmp8 = rtw_hal_pci_dbi_read(padapter, 0x719);
	RTW_PRINT_SEL(m, "CLK REQ:	%s\n", (tmp8 & BIT4) ? "Enable" : "Disable");

	tmp8 = rtw_hal_pci_dbi_read(padapter, 0x70f);
	l1_idle = tmp8 & 0x38;
	RTW_PRINT_SEL(m, "ASPM L0s:	%s\n", (tmp8&BIT7) ? "Enable" : "Disable");

	tmp8 = rtw_hal_pci_dbi_read(padapter, 0x719);
	RTW_PRINT_SEL(m, "ASPM L1:	%s\n", (tmp8 & BIT3) ? "Enable" : "Disable");

	tmp8 = rtw_hal_pci_dbi_read(padapter, 0x718);
	RTW_PRINT_SEL(m, "ASPM L1OFF:	%s\n", (tmp8 & BIT5) ? "Enable" : "Disable");

	RTW_PRINT_SEL(m, "********* MISC **********\n");
	RTW_PRINT_SEL(m, "ASPM L1 Idel Time: 0x%x\n", l1_idle>>3);
	RTW_PRINT_SEL(m, "*************************\n");

#ifdef CONFIG_PCI_DYNAMIC_ASPM
	RTW_PRINT_SEL(m, "Dynamic ASPM mode: %d (%s)\n", pcipriv->aspm_mode,
		      pcipriv->aspm_mode == ASPM_MODE_PERF ? "Perf" :
		      pcipriv->aspm_mode == ASPM_MODE_PS ? "PS" : "Und");
#endif

	return 0;
}

int proc_get_rx_ring(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct recv_priv *precvpriv = &padapter->recvpriv;
	struct rtw_rx_ring *rx_ring = &precvpriv->rx_ring[RX_MPDU_QUEUE];
	int i, j;
	unsigned long sp_flags;

	RTW_PRINT_SEL(m, "rx ring (%p)\n", rx_ring);
	RTW_PRINT_SEL(m, "  dma: 0x%08x\n", (int) rx_ring->dma);
	RTW_PRINT_SEL(m, "  idx: %d\n", rx_ring->idx);

	_rtw_spinlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);
	for (i = 0; i < precvpriv->rxringcount; i++) {
#ifdef CONFIG_TRX_BD_ARCH
		struct rx_buf_desc *entry = &rx_ring->buf_desc[i];
#else
		struct recv_stat *entry = &rx_ring->desc[i];
#endif
		struct sk_buff *skb = rx_ring->rx_buf[i];

		RTW_PRINT_SEL(m, "  desc[%03d]: %p, rx_buf[%03d]: 0x%08x\n",
			i, entry, i, cpu_to_le32(*((dma_addr_t *)skb->cb)));

		for (j = 0; j < sizeof(*entry) / 4; j++) {
			if ((j % 4) == 0)
				RTW_PRINT_SEL(m, "  0x%03x", j);

			RTW_PRINT_SEL(m, " 0x%08x ", ((int *) entry)[j]);

			if ((j % 4) == 3)
				RTW_PRINT_SEL(m, "\n");
		}
	}
	_rtw_spinunlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);

	return 0;
}

int proc_get_tx_ring(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	int i, j, k;
	unsigned long sp_flags;

	_rtw_spinlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);
	for (i = 0; i < PCI_MAX_TX_QUEUE_COUNT; i++) {
		struct rtw_tx_ring *tx_ring = &pxmitpriv->tx_ring[i];

		RTW_PRINT_SEL(m, "tx ring[%d] (%p)\n", i, tx_ring);
		RTW_PRINT_SEL(m, "  dma: 0x%08x\n", (int) tx_ring->dma);
		RTW_PRINT_SEL(m, "  idx: %d\n", tx_ring->idx);
		RTW_PRINT_SEL(m, "  entries: %d\n", tx_ring->entries);
		/*		RTW_PRINT_SEL(m, "  queue: %d\n", tx_ring->queue); */
		RTW_PRINT_SEL(m, "  qlen: %d\n", tx_ring->qlen);

		for (j = 0; j < pxmitpriv->txringcount[i]; j++) {
#ifdef CONFIG_TRX_BD_ARCH
			struct tx_buf_desc *entry = &tx_ring->buf_desc[j];
			RTW_PRINT_SEL(m, "  buf_desc[%03d]: %p\n", j, entry);
#else
			struct tx_desc *entry = &tx_ring->desc[j];
			RTW_PRINT_SEL(m, "  desc[%03d]: %p\n", j, entry);
#endif

			for (k = 0; k < sizeof(*entry) / 4; k++) {
				if ((k % 4) == 0)
					RTW_PRINT_SEL(m, "  0x%03x", k);

				RTW_PRINT_SEL(m, " 0x%08x ", ((int *) entry)[k]);

				if ((k % 4) == 3)
					RTW_PRINT_SEL(m, "\n");
			}
		}
	}
	_rtw_spinunlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);

	return 0;
}

#ifdef DBG_TXBD_DESC_DUMP
int proc_get_tx_ring_ext(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct rtw_tx_desc_backup *pbuf;
	int i, j, k, idx;
	unsigned long sp_flags;

	RTW_PRINT_SEL(m, "<<<< tx ring ext dump settings >>>>\n");
	RTW_PRINT_SEL(m, " - backup frame num: %d\n", TX_BAK_FRMAE_CNT);
	RTW_PRINT_SEL(m, " - backup max. desc size: %d bytes\n", TX_BAK_DESC_LEN);
	RTW_PRINT_SEL(m, " - backup data size: %d bytes\n\n", TX_BAK_DATA_LEN);

	if (!pxmitpriv->dump_txbd_desc) {
		RTW_PRINT_SEL(m, "Dump function is disabled.\n");
		return 0;
	}

	_rtw_spinlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);
	for (i = 0; i < HW_QUEUE_ENTRY; i++) {
		struct rtw_tx_ring *tx_ring = &pxmitpriv->tx_ring[i];

		idx = rtw_get_tx_desc_backup(padapter, i, &pbuf);

		RTW_PRINT_SEL(m, "Tx ring[%d]", i);
		switch (i) {
		case 0:
			RTW_PRINT_SEL(m, " (VO)\n");
			break;
		case 1:
			RTW_PRINT_SEL(m, " (VI)\n");
			break;
		case 2:
			RTW_PRINT_SEL(m, " (BE)\n");
			break;
		case 3:
			RTW_PRINT_SEL(m, " (BK)\n");
			break;
		case 4:
			RTW_PRINT_SEL(m, " (BCN)\n");
			break;
		case 5:
			RTW_PRINT_SEL(m, " (MGT)\n");
			break;
		case 6:
			RTW_PRINT_SEL(m, " (HIGH)\n");
			break;
		case 7:
			RTW_PRINT_SEL(m, " (TXCMD)\n");
			break;
		default:
			RTW_PRINT_SEL(m, " (?)\n");
			break;
		}

		RTW_PRINT_SEL(m, "  Entries: %d\n", TX_BAK_FRMAE_CNT);
		RTW_PRINT_SEL(m, "  Last idx: %d\n", idx);

		for (j = 0; j < TX_BAK_FRMAE_CNT; j++) {
			RTW_PRINT_SEL(m, "  desc[%03d]:\n", j);

			for (k = 0; k < (pbuf->tx_desc_size) / 4; k++) {
				if ((k % 4) == 0)
					RTW_PRINT_SEL(m, "  0x%03x", k);

				RTW_PRINT_SEL(m, " 0x%08x ", ((int *)pbuf->tx_bak_desc)[k]);

				if ((k % 4) == 3)
					RTW_PRINT_SEL(m, "\n");
			}

#if 1 /* data dump */
			if (pbuf->tx_desc_size) {
				RTW_PRINT_SEL(m, "  data[%03d]:\n", j);

				for (k = 0; k < (TX_BAK_DATA_LEN) / 4; k++) {
					if ((k % 4) == 0)
						RTW_PRINT_SEL(m, "  0x%03x", k);

					RTW_PRINT_SEL(m, " 0x%08x ", ((int *)pbuf->tx_bak_data_hdr)[k]);

					if ((k % 4) == 3)
						RTW_PRINT_SEL(m, "\n");
				}
				RTW_PRINT_SEL(m, "\n");
			}
#endif

			RTW_PRINT_SEL(m, "  R/W pointer: %d/%d\n", pbuf->tx_bak_rp, pbuf->tx_bak_wp);

			pbuf = pbuf + 1;
		}
		RTW_PRINT_SEL(m, "\n");
	}
	_rtw_spinunlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);

	return 0;
}

ssize_t proc_set_tx_ring_ext(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	char tmp[32];
	u32 reset = 0;
	u32 dump = 0;
	unsigned long sp_flags;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%u %u", &dump, &reset);

		if (num != 2) {
			RTW_INFO("invalid parameter!\n");
			return count;
		}

		_rtw_spinlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);
		pxmitpriv->dump_txbd_desc = (BOOLEAN) dump;

		if (reset == 1)
			rtw_tx_desc_backup_reset();

		_rtw_spinunlock_irq(&dvobj_to_pci(pdvobjpriv)->irq_th_lock, &sp_flags);

	}

	return count;
}

#endif

#endif

#ifdef CONFIG_WOWLAN
int proc_get_wow_enable(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *registry_pair = &padapter->registrypriv;

	RTW_PRINT_SEL(m, "wow - %s\n", (registry_pair->wowlan_enable)? "enable" : "disable");
	return 0;
}

ssize_t proc_set_wow_enable(struct file *file, const char __user *buffer,
			    size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *registry_pair = &padapter->registrypriv;
	char tmp[8];
	int num = 0;
	int mode = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count))
		num = sscanf(tmp, "%d", &mode);
	else
		return -EFAULT;

	if (num != 1) {
		RTW_ERR("%s: %s - invalid parameter!\n", __func__, tmp);
		return -EINVAL;
	}

	if (mode == 1) {
		RTW_PRINT("%s: wowlan - enable\n", __func__);
	} else if (mode == 0) {
		RTW_PRINT("%s: wowlan - disable\n", __func__);
	} else {
		RTW_ERR("%s: %s - invalid parameter!, mode=%d\n",
			__func__, tmp, mode);
		return -EINVAL;
	}

	registry_pair->wowlan_enable = mode;

	return count;
}

int proc_get_pattern_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	u8 val8;
	char str_1[128];
	char *p_str;
	int i = 0 , j = 0, k = 0;
	int len = 0, max_len = 0, total = 0;

	p_str = str_1;
	max_len = sizeof(str_1);

	total = pwrpriv->wowlan_pattern_idx;

	rtw_set_default_pattern(padapter);

	/*show pattern*/
	RTW_PRINT_SEL(m, "\n======[Pattern Info.]======\n");
	RTW_PRINT_SEL(m, "pattern number: %d\n", total);
	RTW_PRINT_SEL(m, "support default patterns: %c\n",
		      (pwrpriv->default_patterns_en) ? 'Y' : 'N');

	for (k = 0; k < total ; k++) {
		RTW_PRINT_SEL(m, "\npattern idx: %d\n", k);
		RTW_PRINT_SEL(m, "pattern content:\n");

		p_str = str_1;
		max_len = sizeof(str_1);
		for (i = 0 ; i < MAX_WKFM_PATTERN_SIZE / 8 ; i++) {
			_rtw_memset(p_str, 0, max_len);
			len = 0;
			for (j = 0 ; j < 8 ; j++) {
				val8 = pwrpriv->patterns[k].content[i * 8 + j];
				len += snprintf(p_str + len, max_len - len,
						"%02x ", val8);
			}
			RTW_PRINT_SEL(m, "%s\n", p_str);
		}
		RTW_PRINT_SEL(m, "\npattern mask:\n");
		for (i = 0 ; i < MAX_WKFM_SIZE / 8 ; i++) {
			_rtw_memset(p_str, 0, max_len);
			len = 0;
			for (j = 0 ; j < 8 ; j++) {
				val8 = pwrpriv->patterns[k].mask[i * 8 + j];
				len += snprintf(p_str + len, max_len - len,
						"%02x ", val8);
			}
			RTW_PRINT_SEL(m, "%s\n", p_str);
		}

		RTW_PRINT_SEL(m, "\npriv_pattern_len:\n");
		RTW_PRINT_SEL(m, "pattern_len: %d\n", pwrpriv->patterns[k].len);
		RTW_PRINT_SEL(m, "*****************\n");
	}

	return 0;
}

ssize_t proc_set_pattern_info(struct file *file, const char __user *buffer,
			      size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct wowlan_ioctl_param poidparam;
	u8 tmp[MAX_WKFM_PATTERN_STR_LEN + 1] = {0};
	int ret = 0;
	u8 index = 0;

	poidparam.subcode = 0;

	if (count < 1)
		return -EFAULT;

	if (count >= sizeof(tmp)) {
		RTW_ERR("%s: pattern string is too long, count=%zu\n",
			__func__, count);
		return -EFAULT;
	}

	if (pwrpriv->wowlan_pattern_idx >= MAX_WKFM_CAM_NUM) {
		RTW_ERR("priv-pattern is full(idx: %d)\n",
			 pwrpriv->wowlan_pattern_idx);
		RTW_ERR("please clean priv-pattern first\n");
		return -ENOMEM;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		if (strncmp(tmp, "clean", 5) == 0) {
			poidparam.subcode = WOWLAN_PATTERN_CLEAN;
			rtw_hal_set_hwreg(padapter,
					  HW_VAR_WOWLAN, (u8 *)&poidparam);
		} else {
			index = pwrpriv->wowlan_pattern_idx;
			ret = rtw_wowlan_parser_pattern_cmd(tmp,
					    pwrpriv->patterns[index].content,
					    &pwrpriv->patterns[index].len,
					    pwrpriv->patterns[index].mask);
			if (ret == _TRUE)
				pwrpriv->wowlan_pattern_idx++;
		}
	} else {
		rtw_warn_on(1);
		return -EFAULT;
	}

	return count;
}

int proc_get_wakeup_event(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv  *registry_par = &padapter->registrypriv;

	RTW_PRINT_SEL(m, "wakeup event: %#02x\n", registry_par->wakeup_event);
	return 0;
}

ssize_t proc_set_wakeup_event(struct file *file, const char __user *buffer,
			      size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);
	struct registry_priv  *registry_par = &padapter->registrypriv;
	u32 wakeup_event = 0;

	u8 tmp[8] = {0};
	int num = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count))
		num = sscanf(tmp, "%u", &wakeup_event);
	else
		return -EFAULT;

	if (num == 1 && wakeup_event <= 0x07) {
		registry_par->wakeup_event = wakeup_event;

		if (wakeup_event & BIT(1))
			pwrctrlpriv->default_patterns_en = _TRUE;
		else
			pwrctrlpriv->default_patterns_en = _FALSE;

		rtw_wow_pattern_sw_reset(padapter);

		RTW_INFO("%s: wakeup_event: %#2x, default pattern: %d\n",
			 __func__, registry_par->wakeup_event,
			 pwrctrlpriv->default_patterns_en);
	} else {
		return -EINVAL;
	}

	return count;
}

int proc_get_wakeup_reason(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	u8 val = pwrpriv->wowlan_last_wake_reason;

	RTW_PRINT_SEL(m, "last wake reason: %#02x\n", val);
	return 0;
}
#endif /*CONFIG_WOWLAN*/

#ifdef CONFIG_GPIO_WAKEUP
int proc_get_wowlan_gpio_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	u8 val = pwrpriv->is_high_active;

	RTW_PRINT_SEL(m, "wakeup_gpio_idx: %d\n", WAKEUP_GPIO_IDX);
	RTW_PRINT_SEL(m, "high_active: %d\n", val);

	return 0;
}

ssize_t proc_set_wowlan_gpio_info(struct file *file, const char __user *buffer,
				  size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	char tmp[32] = {0};
	int num = 0;
	u32 is_high_active = 0;
	u8 val8 = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		num = sscanf(tmp, "%u", &is_high_active);

		if (num != 1) {
			RTW_INFO("Invalid format\n");
			return count;
		}

		is_high_active = is_high_active == 0 ? 0 : 1;

		pwrpriv->is_high_active = is_high_active;

		rtw_ps_deny(padapter, PS_DENY_IOCTL);
		LeaveAllPowerSaveModeDirect(padapter);

		#ifdef CONFIG_WAKEUP_GPIO_INPUT_MODE
		if (pwrpriv->is_high_active == 0)
			rtw_hal_set_input_gpio(padapter, WAKEUP_GPIO_IDX);
		else
			rtw_hal_set_output_gpio(padapter, WAKEUP_GPIO_IDX, 0);
		#else
		val8 = (pwrpriv->is_high_active == 0) ? 1 : 0;
		rtw_hal_switch_gpio_wl_ctrl(padapter, WAKEUP_GPIO_IDX, _TRUE);
		rtw_hal_set_output_gpio(padapter, WAKEUP_GPIO_IDX, val8);
		#endif
		rtw_ps_deny_cancel(padapter, PS_DENY_IOCTL);

		RTW_INFO("set %s %d\n", "gpio_high_active",
			 pwrpriv->is_high_active);
		RTW_INFO("%s: set GPIO_%d %d as default.\n",
			 __func__, WAKEUP_GPIO_IDX, val8);
	}

	return count;
}
#endif /* CONFIG_GPIO_WAKEUP */

#ifdef CONFIG_P2P_WOWLAN
int proc_get_p2p_wowlan_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	struct p2p_wowlan_info	 peerinfo = pwdinfo->p2p_wow_info;
	if (_TRUE == peerinfo.is_trigger) {
		RTW_PRINT_SEL(m, "is_trigger: TRUE\n");
		switch (peerinfo.wowlan_recv_frame_type) {
		case P2P_WOWLAN_RECV_NEGO_REQ:
			RTW_PRINT_SEL(m, "Frame Type: Nego Request\n");
			break;
		case P2P_WOWLAN_RECV_INVITE_REQ:
			RTW_PRINT_SEL(m, "Frame Type: Invitation Request\n");
			break;
		case P2P_WOWLAN_RECV_PROVISION_REQ:
			RTW_PRINT_SEL(m, "Frame Type: Provision Request\n");
			break;
		default:
			break;
		}
		RTW_PRINT_SEL(m, "Peer Addr: "MAC_FMT"\n", MAC_ARG(peerinfo.wowlan_peer_addr));
		RTW_PRINT_SEL(m, "Peer WPS Config: %x\n", peerinfo.wowlan_peer_wpsconfig);
		RTW_PRINT_SEL(m, "Persistent Group: %d\n", peerinfo.wowlan_peer_is_persistent);
		RTW_PRINT_SEL(m, "Intivation Type: %d\n", peerinfo.wowlan_peer_invitation_type);
	} else
		RTW_PRINT_SEL(m, "is_trigger: False\n");
	return 0;
}
#endif /* CONFIG_P2P_WOWLAN */
#ifdef CONFIG_POWER_SAVING
int proc_get_ps_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	u8 ips_mode = pwrpriv->ips_mode_req;
	u8 lps_mode = pwrpriv->power_mgnt;
	u8 lps_level = pwrpriv->lps_level;
#ifdef CONFIG_LPS_1T1R
	u8 lps_1t1r = pwrpriv->lps_1t1r;
#endif
#ifdef CONFIG_WOWLAN
	u8 wow_lps_mode = pwrpriv->wowlan_power_mgmt;
	u8 wow_lps_level = pwrpriv->wowlan_lps_level;
	#ifdef CONFIG_LPS_1T1R
	u8 wow_lps_1t1r = pwrpriv->wowlan_lps_1t1r;
	#endif
#endif /* CONFIG_WOWLAN */
	char *str = "";

	RTW_PRINT_SEL(m, "======Power Saving Info:======\n");
	RTW_PRINT_SEL(m, "*IPS:\n");

	if (ips_mode == IPS_NORMAL) {
#ifdef CONFIG_FWLPS_IN_IPS
		str = "FW_LPS_IN_IPS";
#else
		str = "Card Disable";
#endif
	} else if (ips_mode == IPS_NONE)
		str = "NO IPS";
	else if (ips_mode == IPS_LEVEL_2)
		str = "IPS_LEVEL_2";
	else
		str = "invalid ips_mode";

	RTW_PRINT_SEL(m, " IPS mode: %s\n", str);
	RTW_PRINT_SEL(m, " IPS enter count:%d, IPS leave count:%d\n",
		      pwrpriv->ips_enter_cnts, pwrpriv->ips_leave_cnts);
	RTW_PRINT_SEL(m, "------------------------------\n");
	RTW_PRINT_SEL(m, "*LPS:\n");

	if (lps_mode == PM_PS_MODE_ACTIVE)
		str = "NO LPS";
	else if (lps_mode == PM_PS_MODE_MIN)
		str = "MIN";
	else if (lps_mode == PM_PS_MODE_MAX)
		str = "MAX";
	else if (lps_mode == PM_PS_MODE_DTIM)
		str = "DTIM";
	else
		sprintf(str, "%d", lps_mode);

	RTW_PRINT_SEL(m, " LPS mode: %s\n", str);

	if (pwrpriv->dtim != 0)
		RTW_PRINT_SEL(m, " DTIM: %d\n", pwrpriv->dtim);
	RTW_PRINT_SEL(m, " LPS enter count:%d, LPS leave count:%d\n",
		      pwrpriv->lps_enter_cnts, pwrpriv->lps_leave_cnts);

	if (lps_level == LPS_LCLK)
		str = "LPS_LCLK";
	else if  (lps_level == LPS_PG)
		str = "LPS_PG";
	else
		str = "LPS_NORMAL";
	RTW_PRINT_SEL(m, " LPS level: %s\n", str);

#ifdef CONFIG_LPS_1T1R
	RTW_PRINT_SEL(m, " LPS 1T1R: %d\n", lps_1t1r);
#endif

#ifdef CONFIG_WOWLAN
	RTW_PRINT_SEL(m, "------------------------------\n");
	RTW_PRINT_SEL(m, "*WOW LPS:\n");

	if (wow_lps_mode == PM_PS_MODE_ACTIVE)
		str = "NO LPS";
	else if (wow_lps_mode == PM_PS_MODE_MIN)
		str = "MIN";
	else if (wow_lps_mode == PM_PS_MODE_MAX)
		str = "MAX";
	else if (wow_lps_mode == PM_PS_MODE_DTIM)
		str = "DTIM";
	else
		sprintf(str, "%d", wow_lps_mode);

	RTW_PRINT_SEL(m, " WOW LPS mode: %s\n", str);

	if (wow_lps_level == LPS_LCLK)
		str = "LPS_LCLK";
	else if  (wow_lps_level == LPS_PG)
		str = "LPS_PG";
	else
		str = "LPS_NORMAL";
	RTW_PRINT_SEL(m, " WOW LPS level: %s\n", str);

	#ifdef CONFIG_LPS_1T1R
	RTW_PRINT_SEL(m, " WOW LPS 1T1R: %d\n", wow_lps_1t1r);
	#endif
#endif /* CONFIG_WOWLAN */

	RTW_PRINT_SEL(m, "=============================\n");
	return 0;
}

ssize_t proc_set_ps_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[8];
	int num = 0;
	int mode = 0;
	int en = 0;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (!buffer || copy_from_user(tmp, buffer, count))
		goto exit;

	num = sscanf(tmp, "%d %d", &mode, &en);
	if (num >  2) {
		RTW_ERR("%s: invalid parameter!\n", __FUNCTION__);
		goto exit;
	}

	if (num == 1 && mode == 0) {
		/* back to original LPS/IPS Mode */
		RTW_INFO("%s: back to original LPS/IPS Mode\n", __FUNCTION__);

		rtw_pm_set_lps(adapter, adapter->registrypriv.power_mgnt);

		rtw_pm_set_ips(adapter, adapter->registrypriv.ips_mode);

#ifdef CONFIG_WOWLAN
		RTW_INFO("%s: back to original WOW LPS Mode\n", __FUNCTION__);

		rtw_pm_set_wow_lps(adapter, adapter->registrypriv.wow_power_mgnt);
#endif /* CONFIG_WOWLAN */

		goto exit;
	}

	if (mode == 1) {
		/* LPS */
		RTW_INFO("%s: LPS: %s, en=%d\n", __FUNCTION__, (en == 0) ? "disable":"enable", en);
		if (rtw_pm_set_lps(adapter, en) != 0 )
			RTW_ERR("%s: invalid parameter, mode=%d, level=%d\n", __FUNCTION__, mode, en);

	} else if (mode == 2) {
		/* IPS */
		RTW_INFO("%s: IPS: %s, en=%d\n", __FUNCTION__, (en == 0) ? "disable":"enable", en);
		if (rtw_pm_set_ips(adapter, en) != 0 )
			RTW_ERR("%s: invalid parameter, mode=%d, level=%d\n", __FUNCTION__, mode, en);
	}
#ifdef CONFIG_WOWLAN
	else if (mode == 3) {
		/* WOW LPS */
		RTW_INFO("%s: WOW LPS: %s, en=%d\n", __FUNCTION__, (en == 0) ? "disable":"enable", en);
		if (rtw_pm_set_wow_lps(adapter, en) != 0 )
			RTW_ERR("%s: invalid parameter, mode=%d, level=%d\n", __FUNCTION__, mode, en);
	}
#endif /* CONFIG_WOWLAN */
	else
		RTW_ERR("%s: invalid parameter, mode = %d!\n", __FUNCTION__, mode);

exit:
	return count;
}

#ifdef CONFIG_WMMPS_STA
int proc_get_wmmps_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char *uapsd_max_sp_str="";

	if (pregpriv){
		switch(pregpriv->uapsd_max_sp_len) {
			case 0:
				uapsd_max_sp_str = "NO_LIMIT";
				break;
			case 1:
				uapsd_max_sp_str = "TWO_MSDU";
				break;
			case 2:
				uapsd_max_sp_str = "FOUR_MSDU";
				break;
			case 3:
				uapsd_max_sp_str = "SIX_MSDU";
				break;
			default:
				uapsd_max_sp_str = "UNSPECIFIED";
				break;
		}

		RTW_PRINT_SEL(m, "====== WMMPS_STA Info:======\n");
		RTW_PRINT_SEL(m, "uapsd_max_sp_len=0x%02x (%s)\n", pregpriv->uapsd_max_sp_len, uapsd_max_sp_str);
		RTW_PRINT_SEL(m, "uapsd_ac_enable=0x%02x\n", pregpriv->uapsd_ac_enable);
		RTW_PRINT_SEL(m, "BIT0 - AC_VO UAPSD: %s\n", (pregpriv->uapsd_ac_enable & DRV_CFG_UAPSD_VO) ? "Enabled" : "Disabled");
		RTW_PRINT_SEL(m, "BIT1 - AC_VI UAPSD: %s\n", (pregpriv->uapsd_ac_enable & DRV_CFG_UAPSD_VI) ? "Enabled" : "Disabled");
		RTW_PRINT_SEL(m, "BIT2 - AC_BK UAPSD: %s\n", (pregpriv->uapsd_ac_enable & DRV_CFG_UAPSD_BK) ? "Enabled" : "Disabled");
		RTW_PRINT_SEL(m, "BIT3 - AC_BE UAPSD: %s\n", (pregpriv->uapsd_ac_enable & DRV_CFG_UAPSD_BE) ? "Enabled" : "Disabled");
		RTW_PRINT_SEL(m, "============================\n");
	}

	return 0;
}

ssize_t proc_set_wmmps_info(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u8 uapsd_ac_setting;
	u8 uapsd_max_sp_len_setting;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu %hhx", &uapsd_max_sp_len_setting, &uapsd_ac_setting);

		if (pregpriv) {
			if (num >= 1) {
				pregpriv->uapsd_max_sp_len = uapsd_max_sp_len_setting;
				RTW_INFO("uapsd_max_sp_len = %d\n", pregpriv->uapsd_max_sp_len);
			}

			if (num >= 2) {
				pregpriv->uapsd_ac_enable = uapsd_ac_setting;
				RTW_INFO("uapsd_ac_enable = 0x%02x\n", pregpriv->uapsd_ac_enable);
			}
		}
	}

	return count;
}
#endif /* CONFIG_WMMPS_STA */
#endif /* CONFIG_POWER_SAVING */

#ifdef CONFIG_TDLS
int proc_get_tdls_enable(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "TDLS is %s !\n", (rtw_is_tdls_enabled(padapter) == _TRUE) ? "enabled" : "disabled");

	return 0;
}

ssize_t proc_set_tdls_enable(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	char tmp[32];
	u32 en_tdls = 0;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d ", &en_tdls);

		if (num == 1 && pregpriv) {
			if (en_tdls > 0)
				rtw_enable_tdls_func(padapter);
			else
				rtw_disable_tdls_func(padapter, _TRUE);
		}
	}

	return count;
}

static int proc_tdls_display_tdls_function_info(struct seq_file *m)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	u8 SpaceBtwnItemAndValue = TDLS_DBG_INFO_SPACE_BTWN_ITEM_AND_VALUE;
	u8 SpaceBtwnItemAndValueTmp = 0;
	BOOLEAN FirstMatchFound = _FALSE;
	int j = 0;

	RTW_PRINT_SEL(m, "============[TDLS Function Info]============\n");
	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS Enable", (rtw_is_tdls_enabled(padapter) == _TRUE) ? "_TRUE" : "_FALSE");
	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS Driver Setup", (ptdlsinfo->driver_setup == _TRUE) ? "_TRUE" : "_FALSE");
	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS Prohibited", (ptdlsinfo->ap_prohibited == _TRUE) ? "_TRUE" : "_FALSE");
	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS Channel Switch Prohibited", (ptdlsinfo->ch_switch_prohibited == _TRUE) ? "_TRUE" : "_FALSE");
	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS Link Established", (ptdlsinfo->link_established == _TRUE) ? "_TRUE" : "_FALSE");
	RTW_PRINT_SEL(m, "%-*s = %d/%d\n", SpaceBtwnItemAndValue, "TDLS STA Num (Linked/Allowed)", ptdlsinfo->sta_cnt, MAX_ALLOWED_TDLS_STA_NUM);
	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS Allowed STA Num Reached", (ptdlsinfo->sta_maximum == _TRUE) ? "_TRUE" : "_FALSE");

#ifdef CONFIG_TDLS_CH_SW
	RTW_PRINT_SEL(m, "%-*s =", SpaceBtwnItemAndValue, "TDLS CH SW State");
	if (ptdlsinfo->chsw_info.ch_sw_state == TDLS_STATE_NONE)
		RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_STATE_NONE");
	else {
		for (j = 0; j < 32; j++) {
			if (ptdlsinfo->chsw_info.ch_sw_state & BIT(j)) {
				if (FirstMatchFound ==  _FALSE) {
					SpaceBtwnItemAndValueTmp = 1;
					FirstMatchFound = _TRUE;
				} else
					SpaceBtwnItemAndValueTmp = SpaceBtwnItemAndValue + 3;
				switch (BIT(j)) {
				case TDLS_INITIATOR_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_INITIATOR_STATE");
					break;
				case TDLS_RESPONDER_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_RESPONDER_STATE");
					break;
				case TDLS_LINKED_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_LINKED_STATE");
					break;
				case TDLS_WAIT_PTR_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_WAIT_PTR_STATE");
					break;
				case TDLS_ALIVE_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_ALIVE_STATE");
					break;
				case TDLS_CH_SWITCH_ON_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_CH_SWITCH_ON_STATE");
					break;
				case TDLS_PEER_AT_OFF_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_PEER_AT_OFF_STATE");
					break;
				case TDLS_CH_SW_INITIATOR_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_CH_SW_INITIATOR_STATE");
					break;
				case TDLS_WAIT_CH_RSP_STATE:
					RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValue, " ", "TDLS_WAIT_CH_RSP_STATE");
					break;
				default:
					RTW_PRINT_SEL(m, "%-*sBIT(%d)\n", SpaceBtwnItemAndValueTmp, " ", j);
					break;
				}
			}
		}
	}

	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS CH SW On", (ATOMIC_READ(&ptdlsinfo->chsw_info.chsw_on) == _TRUE) ? "_TRUE" : "_FALSE");
	RTW_PRINT_SEL(m, "%-*s = %d\n", SpaceBtwnItemAndValue, "TDLS CH SW Off-Channel Num", ptdlsinfo->chsw_info.off_ch_num);
	RTW_PRINT_SEL(m, "%-*s = %d\n", SpaceBtwnItemAndValue, "TDLS CH SW Channel Offset", ptdlsinfo->chsw_info.ch_offset);
	RTW_PRINT_SEL(m, "%-*s = %d\n", SpaceBtwnItemAndValue, "TDLS CH SW Current Time", ptdlsinfo->chsw_info.cur_time);
	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS CH SW Delay Switch Back", (ptdlsinfo->chsw_info.delay_switch_back == _TRUE) ? "_TRUE" : "_FALSE");
	RTW_PRINT_SEL(m, "%-*s = %d\n", SpaceBtwnItemAndValue, "TDLS CH SW Dump Back", ptdlsinfo->chsw_info.dump_stack);
#endif

	RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "TDLS Device Discovered", (ptdlsinfo->dev_discovered == _TRUE) ? "_TRUE" : "_FALSE");

	return 0;
}

static int proc_tdls_display_network_info(struct seq_file *m)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	int i = 0;
	u8 SpaceBtwnItemAndValue = TDLS_DBG_INFO_SPACE_BTWN_ITEM_AND_VALUE;

	/* Display the linked AP/GO info */
	RTW_PRINT_SEL(m, "============[Associated AP/GO Info]============\n");

	if ((pmlmepriv->fw_state & WIFI_STATION_STATE) && (pmlmepriv->fw_state & WIFI_ASOC_STATE)) {
		RTW_PRINT_SEL(m, "%-*s = %s\n", SpaceBtwnItemAndValue, "BSSID", cur_network->network.Ssid.Ssid);
		RTW_PRINT_SEL(m, "%-*s = "MAC_FMT"\n", SpaceBtwnItemAndValue, "Mac Address", MAC_ARG(cur_network->network.MacAddress));

		RTW_PRINT_SEL(m, "%-*s = ", SpaceBtwnItemAndValue, "Wireless Mode");
		for (i = 0; i < 8; i++) {
			if (pmlmeext->cur_wireless_mode & BIT(i)) {
				switch (BIT(i)) {
				case WIRELESS_11B:
					RTW_PRINT_SEL(m, "%4s", "11B ");
					break;
				case WIRELESS_11G:
					RTW_PRINT_SEL(m, "%4s", "11G ");
					break;
				case WIRELESS_11A:
					RTW_PRINT_SEL(m, "%4s", "11A ");
					break;
				case WIRELESS_11_24N:
					RTW_PRINT_SEL(m, "%7s", "11_24N ");
					break;
				case WIRELESS_11_5N:
					RTW_PRINT_SEL(m, "%6s", "11_5N ");
					break;
				case WIRELESS_AUTO:
					RTW_PRINT_SEL(m, "%5s", "AUTO ");
					break;
				case WIRELESS_11AC:
					RTW_PRINT_SEL(m, "%5s", "11AC ");
					break;
				}
			}
		}
		RTW_PRINT_SEL(m, "\n");

		RTW_PRINT_SEL(m, "%-*s = ", SpaceBtwnItemAndValue, "Privacy");
		switch (padapter->securitypriv.dot11PrivacyAlgrthm) {
		case _NO_PRIVACY_:
			RTW_PRINT_SEL(m, "%s\n", "NO PRIVACY");
			break;
		case _WEP40_:
			RTW_PRINT_SEL(m, "%s\n", "WEP 40");
			break;
		case _TKIP_:
			RTW_PRINT_SEL(m, "%s\n", "TKIP");
			break;
		case _TKIP_WTMIC_:
			RTW_PRINT_SEL(m, "%s\n", "TKIP WTMIC");
			break;
		case _AES_:
			RTW_PRINT_SEL(m, "%s\n", "AES");
			break;
		case _WEP104_:
			RTW_PRINT_SEL(m, "%s\n", "WEP 104");
			break;
#if 0 /* no this setting */
		case _WEP_WPA_MIXED_:
			RTW_PRINT_SEL(m, "%s\n", "WEP/WPA Mixed");
			break;
#endif
		case _SMS4_:
			RTW_PRINT_SEL(m, "%s\n", "SMS4");
			break;
#ifdef CONFIG_IEEE80211W
		case _BIP_CMAC_128_:
			RTW_PRINT_SEL(m, "%s\n", "BIP");
			break;
#endif /* CONFIG_IEEE80211W */
		}

		RTW_PRINT_SEL(m, "%-*s = %d\n", SpaceBtwnItemAndValue, "Channel", pmlmeext->cur_channel);
		RTW_PRINT_SEL(m, "%-*s = ", SpaceBtwnItemAndValue, "Channel Offset");
		switch (pmlmeext->cur_ch_offset) {
		case CHAN_OFFSET_NO_EXT:
			RTW_PRINT_SEL(m, "%s\n", "SCN");
			break;
		case CHAN_OFFSET_UPPER:
			RTW_PRINT_SEL(m, "%s\n", "SCA");
			break;
		case CHAN_OFFSET_LOWER:
			RTW_PRINT_SEL(m, "%s\n", "SCB");
			break;
		}

		RTW_PRINT_SEL(m, "%-*s = ", SpaceBtwnItemAndValue, "Bandwidth Mode");
		switch (pmlmeext->cur_bwmode) {
		case CHANNEL_WIDTH_20:
			RTW_PRINT_SEL(m, "%s\n", "20MHz");
			break;
		case CHANNEL_WIDTH_40:
			RTW_PRINT_SEL(m, "%s\n", "40MHz");
			break;
		case CHANNEL_WIDTH_80:
			RTW_PRINT_SEL(m, "%s\n", "80MHz");
			break;
		case CHANNEL_WIDTH_160:
			RTW_PRINT_SEL(m, "%s\n", "160MHz");
			break;
		case CHANNEL_WIDTH_80_80:
			RTW_PRINT_SEL(m, "%s\n", "80MHz + 80MHz");
			break;
		}
	} else
		RTW_PRINT_SEL(m, "No association with AP/GO exists!\n");

	return 0;
}

static int proc_tdls_display_tdls_sta_info(struct seq_file *m)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct sta_info *psta;
	int i = 0, j = 0;
	_list	*plist, *phead;
	u8 SpaceBtwnItemAndValue = TDLS_DBG_INFO_SPACE_BTWN_ITEM_AND_VALUE;
	u8 SpaceBtwnItemAndValueTmp = 0;
	u8 NumOfTdlsStaToShow = 0;
	BOOLEAN FirstMatchFound = _FALSE;

	/* Search for TDLS sta info to display */
	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);
			if (psta->tdls_sta_state != TDLS_STATE_NONE) {
				/* We got one TDLS sta info to show */
				RTW_PRINT_SEL(m, "============[TDLS Peer STA Info: STA %d]============\n", ++NumOfTdlsStaToShow);
				RTW_PRINT_SEL(m, "%-*s = "MAC_FMT"\n", SpaceBtwnItemAndValue, "Mac Address", MAC_ARG(psta->phl_sta->mac_addr));
				RTW_PRINT_SEL(m, "%-*s =", SpaceBtwnItemAndValue, "TDLS STA State");
				SpaceBtwnItemAndValueTmp = 0;
				FirstMatchFound = _FALSE;
				for (j = 0; j < 32; j++) {
					if (psta->tdls_sta_state & BIT(j)) {
						if (FirstMatchFound ==  _FALSE) {
							SpaceBtwnItemAndValueTmp = 1;
							FirstMatchFound = _TRUE;
						} else
							SpaceBtwnItemAndValueTmp = SpaceBtwnItemAndValue + 3;
						switch (BIT(j)) {
						case TDLS_INITIATOR_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_INITIATOR_STATE");
							break;
						case TDLS_RESPONDER_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_RESPONDER_STATE");
							break;
						case TDLS_LINKED_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_LINKED_STATE");
							break;
						case TDLS_WAIT_PTR_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_WAIT_PTR_STATE");
							break;
						case TDLS_ALIVE_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_ALIVE_STATE");
							break;
						case TDLS_CH_SWITCH_ON_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_CH_SWITCH_ON_STATE");
							break;
						case TDLS_PEER_AT_OFF_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_PEER_AT_OFF_STATE");
							break;
						case TDLS_CH_SW_INITIATOR_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValueTmp, " ", "TDLS_CH_SW_INITIATOR_STATE");
							break;
						case TDLS_WAIT_CH_RSP_STATE:
							RTW_PRINT_SEL(m, "%-*s%s\n", SpaceBtwnItemAndValue, " ", "TDLS_WAIT_CH_RSP_STATE");
							break;
						default:
							RTW_PRINT_SEL(m, "%-*sBIT(%d)\n", SpaceBtwnItemAndValueTmp, " ", j);
							break;
						}
					}
				}

				RTW_PRINT_SEL(m, "%-*s = ", SpaceBtwnItemAndValue, "Wireless Mode");
				for (j = 0; j < 8; j++) {
					if (psta->phl_sta->wmode & BIT(j)) {
						switch (BIT(j)) {
						case WIRELESS_11B:
							RTW_PRINT_SEL(m, "%4s", "11B ");
							break;
						case WIRELESS_11G:
							RTW_PRINT_SEL(m, "%4s", "11G ");
							break;
						case WIRELESS_11A:
							RTW_PRINT_SEL(m, "%4s", "11A ");
							break;
						case WIRELESS_11_24N:
							RTW_PRINT_SEL(m, "%7s", "11_24N ");
							break;
						case WIRELESS_11_5N:
							RTW_PRINT_SEL(m, "%6s", "11_5N ");
							break;
						case WIRELESS_AUTO:
							RTW_PRINT_SEL(m, "%5s", "AUTO ");
							break;
						case WIRELESS_11AC:
							RTW_PRINT_SEL(m, "%5s", "11AC ");
							break;
						}
					}
				}
				RTW_PRINT_SEL(m, "\n");

				RTW_PRINT_SEL(m, "%-*s = ", SpaceBtwnItemAndValue, "Bandwidth Mode");
				switch (psta->phl_sta->chandef.bw) {
				case CHANNEL_WIDTH_20:
					RTW_PRINT_SEL(m, "%s\n", "20MHz");
					break;
				case CHANNEL_WIDTH_40:
					RTW_PRINT_SEL(m, "%s\n", "40MHz");
					break;
				case CHANNEL_WIDTH_80:
					RTW_PRINT_SEL(m, "%s\n", "80MHz");
					break;
				case CHANNEL_WIDTH_160:
					RTW_PRINT_SEL(m, "%s\n", "160MHz");
					break;
				case CHANNEL_WIDTH_80_80:
					RTW_PRINT_SEL(m, "%s\n", "80MHz + 80MHz");
					break;
				case CHANNEL_WIDTH_5:
					RTW_PRINT_SEL(m, "%s\n", "5MHz");
					break;
				case CHANNEL_WIDTH_10:
					RTW_PRINT_SEL(m, "%s\n", "10MHz");
					break;
				default:
					RTW_PRINT_SEL(m, "(%d)%s\n", psta->phl_sta->chandef.bw, "invalid");
					break;
				}

				RTW_PRINT_SEL(m, "%-*s = ", SpaceBtwnItemAndValue, "Privacy");
				switch (psta->dot118021XPrivacy) {
				case _NO_PRIVACY_:
					RTW_PRINT_SEL(m, "%s\n", "NO PRIVACY");
					break;
				case _WEP40_:
					RTW_PRINT_SEL(m, "%s\n", "WEP 40");
					break;
				case _TKIP_:
					RTW_PRINT_SEL(m, "%s\n", "TKIP");
					break;
				case _TKIP_WTMIC_:
					RTW_PRINT_SEL(m, "%s\n", "TKIP WTMIC");
					break;
				case _AES_:
					RTW_PRINT_SEL(m, "%s\n", "AES");
					break;
				case _WEP104_:
					RTW_PRINT_SEL(m, "%s\n", "WEP 104");
					break;
#if 0 /* no this setting */
				case _WEP_WPA_MIXED_:
					RTW_PRINT_SEL(m, "%s\n", "WEP/WPA Mixed");
					break;
#endif
				case _SMS4_:
					RTW_PRINT_SEL(m, "%s\n", "SMS4");
					break;
#ifdef CONFIG_IEEE80211W
				case _BIP_CMAC_128_:
					RTW_PRINT_SEL(m, "%s\n", "BIP");
					break;
#endif /* CONFIG_IEEE80211W */
				}

				RTW_PRINT_SEL(m, "%-*s = %d sec/%d sec\n", SpaceBtwnItemAndValue, "TPK Lifetime (Current/Expire)", psta->TPK_count, psta->TDLS_PeerKey_Lifetime);
				RTW_PRINT_SEL(m, "%-*s = %llu\n", SpaceBtwnItemAndValue, "Tx Packets Over Direct Link", psta->sta_stats.tx_pkts);
				RTW_PRINT_SEL(m, "%-*s = %llu\n", SpaceBtwnItemAndValue, "Rx Packets Over Direct Link", psta->sta_stats.rx_data_pkts);
			}
		}
	}
	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);
	if (NumOfTdlsStaToShow == 0) {
		RTW_PRINT_SEL(m, "============[TDLS Peer STA Info]============\n");
		RTW_PRINT_SEL(m, "No TDLS direct link exists!\n");
	}

	return 0;
}

int proc_get_tdls_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
	struct sta_info *psta;
	int i = 0, j = 0;
	_list	*plist, *phead;
	u8 SpaceBtwnItemAndValue = 41;
	u8 SpaceBtwnItemAndValueTmp = 0;
	u8 NumOfTdlsStaToShow = 0;
	BOOLEAN FirstMatchFound = _FALSE;

	if (rtw_hw_chk_wl_func(adapter_to_dvobj(padapter), WL_FUNC_TDLS) == _FALSE) {
		RTW_PRINT_SEL(m, "No tdls info can be shown since hal doesn't support tdls\n");
		return 0;
	}

	proc_tdls_display_tdls_function_info(m);
	proc_tdls_display_network_info(m);
	proc_tdls_display_tdls_sta_info(m);

	return 0;
}
#endif

int proc_get_monitor(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if (MLME_IS_MONITOR(padapter)) {
		RTW_PRINT_SEL(m, "Monitor mode : Enable\n");
		RTW_PRINT_SEL(m, "Device type  : %u\n", dev->type);

		RTW_PRINT_SEL(m, "ch=%d, ch_offset=%d, bw=%d\n",
			rtw_get_oper_ch(padapter),
			rtw_get_oper_choffset(padapter),
			rtw_get_oper_bw(padapter));
	} else
		RTW_PRINT_SEL(m, "Monitor mode : Disable\n");

	return 0;
}

ssize_t proc_set_monitor(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u16 target_type;
	u8 target_ch, target_offset, target_bw;

	if (count < 3) {
		RTW_INFO("argument size is less than 3\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = 0;

		num = sscanf(tmp, "type %hu", &target_type);
		if ((num == 1) &&
			((target_type != ARPHRD_IEEE80211) &&
			(target_type != ARPHRD_IEEE80211_RADIOTAP))) {
			dev->type = ARPHRD_IEEE80211_RADIOTAP;
			return count;
		}

		num = sscanf(tmp, "%hhu %hhu %hhu", &target_ch, &target_offset, &target_bw);
		if (num != 3) {
			RTW_INFO("invalid write_reg parameter!\n");
			return count;
		}

		padapter->mlmeextpriv.cur_channel = target_ch;
		set_channel_bwmode(padapter, target_ch, target_offset, target_bw, _FALSE);
	}

	return count;
}
#ifdef DBG_XMIT_BLOCK
int proc_get_xmit_block(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	dump_xmit_block(m, padapter);

	return 0;
}

ssize_t proc_set_xmit_block(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u8 xb_mode, xb_reason;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhx %hhx", &xb_mode, &xb_reason);

		if (num != 2) {
			RTW_INFO("invalid parameter!\n");
			return count;
		}

		if (xb_mode == 0)/*set*/
			rtw_set_xmit_block(padapter, xb_reason);
		else if (xb_mode == 1)/*clear*/
			rtw_clr_xmit_block(padapter, xb_reason);
		else
			RTW_INFO("invalid parameter!\n");
	}

	return count;
}
#endif

int proc_get_efuse_map(struct seq_file *m, void *v)
{
#if 0
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(adapter_to_dvobj(padapter));
	struct pwrctrl_priv *pwrctrlpriv  = adapter_to_pwrctl(padapter);
	PEFUSE_HAL pEfuseHal = &pHalData->EfuseHal;
	struct efuse_info *efuse = adapter_to_efuse(padapter);
	int i, j;
	u8 ips_mode = IPS_NUM;
	u16 mapLen;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, _FALSE);
	if (mapLen > EFUSE_MAX_MAP_LEN)
		mapLen = EFUSE_MAX_MAP_LEN;

	ips_mode = pwrctrlpriv->ips_mode;
	rtw_pm_set_ips(padapter, IPS_NONE);

	if (efuse->file_status == EFUSE_FILE_LOADED) {
		RTW_PRINT_SEL(m, "File eFuse Map loaded! file path:%s\nDriver eFuse Map From File\n", EFUSE_MAP_PATH);
		if (efuse->is_autoload_fail)
			RTW_PRINT_SEL(m, "File Autoload fail!!!\n");
	} else if (efuse->file_status ==  EFUSE_FILE_FAILED) {
		RTW_PRINT_SEL(m, "Open File eFuse Map Fail ! file path:%s\nDriver eFuse Map From Default\n", EFUSE_MAP_PATH);
		if (efuse->is_autoload_fail)
			RTW_PRINT_SEL(m, "HW Autoload fail!!!\n");
	} else {
		RTW_PRINT_SEL(m, "Driver eFuse Map From HW\n");
		if (efuse->is_autoload_fail)
			RTW_PRINT_SEL(m, "HW Autoload fail!!!\n");
	}
	for (i = 0; i < mapLen; i += 16) {
		RTW_PRINT_SEL(m, "0x%02x\t", i);
		for (j = 0; j < 8; j++)
			RTW_PRINT_SEL(m, "%02X ", efuse->data[i + j]);
		RTW_PRINT_SEL(m, "\t");
		for (; j < 16; j++)
			RTW_PRINT_SEL(m, "%02X ", efuse->data[i + j]);
		RTW_PRINT_SEL(m, "\n");
	}

	if (rtw_efuse_map_read(padapter, 0, mapLen, pEfuseHal->fakeEfuseInitMap) == _FAIL) {
		RTW_PRINT_SEL(m, "WARN - Read Realmap Failed\n");
		return 0;
	}

	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "HW eFuse Map\n");
	for (i = 0; i < mapLen; i += 16) {
		RTW_PRINT_SEL(m, "0x%02x\t", i);
		for (j = 0; j < 8; j++)
			RTW_PRINT_SEL(m, "%02X ", pEfuseHal->fakeEfuseInitMap[i + j]);
		RTW_PRINT_SEL(m, "\t");
		for (; j < 16; j++)
			RTW_PRINT_SEL(m, "%02X ", pEfuseHal->fakeEfuseInitMap[i + j]);
		RTW_PRINT_SEL(m, "\n");
	}

	rtw_pm_set_ips(padapter, ips_mode);
#endif
	return 0;
}

ssize_t proc_set_efuse_map(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
#if 0
	char tmp[256] = {0};
	u32 addr, cnts;
	u8 efuse_data;

	int jj, kk;

	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrctrlpriv  = adapter_to_pwrctl(padapter);
	u8 ips_mode = IPS_NUM;

	if (count < 3) {
		RTW_INFO("argument size is less than 3\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%x %d %x", &addr, &cnts, &efuse_data);

		if (num != 3) {
			RTW_INFO("invalid write_reg parameter!\n");
			return count;
		}
	}
	ips_mode = pwrctrlpriv->ips_mode;
	rtw_pm_set_ips(padapter, IPS_NONE);
	if (rtw_efuse_map_write(padapter, addr, cnts, &efuse_data) == _FAIL)
		RTW_INFO("WARN - rtw_efuse_map_write error!!\n");
	rtw_pm_set_ips(padapter, ips_mode);
#endif
	return count;
}

#if defined(CONFIG_IEEE80211W) && !defined(CONFIG_IOCTL_CFG80211)
ssize_t proc_set_tx_sa_query(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	/* macid_ctl move to phl */
	/* struct macid_ctl_t *macid_ctl = dvobj_to_macidctl(dvobj); */
	struct sta_info *psta;
	_list	*plist, *phead;
	char tmp[16];
	u8	mac_addr[NUM_STA][ETH_ALEN];
	u32 key_type;
	u8 index;
	void *phl = GET_HAL_INFO(dvobj);
	u16 macid_num = rtw_phl_get_macid_max_num(phl);

	if (count > 2) {
		RTW_INFO("argument size is more than 2\n");
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, sizeof(tmp))) {

		int num = sscanf(tmp, "%x", &key_type);

		if (num !=  1) {
			RTW_INFO("invalid read_reg parameter!\n");
			return count;
		}
		RTW_INFO("0: set sa query request , key_type=%d\n", key_type);
	}

	if (MLME_IS_STA(padapter)
	    && (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE) && SEC_IS_BIP_KEY_INSTALLED(&padapter->securitypriv) == _TRUE) {
		RTW_INFO("STA:"MAC_FMT"\n", MAC_ARG(get_my_bssid(&(pmlmeinfo->network))));
		/* TX unicast sa_query to AP */
		issue_action_SA_Query(padapter, get_my_bssid(&(pmlmeinfo->network)), 0, 0, (u8)key_type);
	} else if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE && SEC_IS_BIP_KEY_INSTALLED(&padapter->securitypriv) == _TRUE) {
		/* TX unicast sa_query to every client STA */
		_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
		for (index = 0; index < NUM_STA; index++) {
			psta = NULL;

			phead = &(pstapriv->sta_hash[index]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
				plist = get_next(plist);
				_rtw_memcpy(&mac_addr[psta->phl_sta->macid][0], psta->phl_sta->mac_addr, ETH_ALEN);
			}
		}
		_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

		for (index = 0; index < macid_num && index < NUM_STA; index++) {
			if (rtw_phl_macid_is_used(phl, index) && !rtw_phl_macid_is_bmc(phl, index)) {
				if (!_rtw_memcmp(get_my_bssid(&(pmlmeinfo->network)), &mac_addr[index][0], ETH_ALEN)
				    && !IS_MCAST(&mac_addr[index][0])) {
					issue_action_SA_Query(padapter, &mac_addr[index][0], 0, 0, (u8)key_type);
					RTW_INFO("STA[%u]:"MAC_FMT"\n", index , MAC_ARG(&mac_addr[index][0]));
				}
			}
		}
	}

	return count;
}

int proc_get_tx_sa_query(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "%s\n", __func__);
	return 0;
}

ssize_t proc_set_tx_deauth(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	/* macid_ctl move to phl */
	/* struct macid_ctl_t *macid_ctl = dvobj_to_macidctl(dvobj); */
	struct sta_info *psta;
	_list	*plist, *phead;
	char tmp[16];
	u8	mac_addr[NUM_STA][ETH_ALEN];
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u32 key_type = 0;
	u8 index;
	void *phl = GET_HAL_INFO(dvobj);
	u16 macid_num = rtw_phl_get_macid_max_num(phl);


	if (count > 2) {
		RTW_INFO("argument size is more than 2\n");
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, sizeof(tmp))) {

		int num = sscanf(tmp, "%x", &key_type);

		if (num !=  1) {
			RTW_INFO("invalid read_reg parameter!\n");
			return count;
		}
		RTW_INFO("key_type=%d\n", key_type);
	}
	if (key_type > 4)
		return count;

	if (MLME_IS_STA(padapter)
	    && (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)) {
		if (key_type == 3) /* key_type 3 only for AP mode */
			return count;
		/* TX unicast deauth to AP */
		issue_deauth_11w(padapter, get_my_bssid(&(pmlmeinfo->network)), 0, (u8)key_type);
	} else if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE) {
		u8 updated = _FALSE;

		if (key_type == 3)
			issue_deauth_11w(padapter, bc_addr, 0, IEEE80211W_RIGHT_KEY);

		/* TX unicast deauth to every client STA */
		_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
		for (index = 0; index < NUM_STA; index++) {
			psta = NULL;

			phead = &(pstapriv->sta_hash[index]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
				plist = get_next(plist);
				_rtw_memcpy(&mac_addr[psta->phl_sta->macid][0], psta->phl_sta->mac_addr, ETH_ALEN);
			}
		}
		_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

		for (index = 0; index < macid_num && index < NUM_STA; index++) {
			if (rtw_phl_macid_is_used(phl, index) && !rtw_phl_macid_is_bmc(phl, index)) {
				if (!_rtw_memcmp(get_my_bssid(&(pmlmeinfo->network)), &mac_addr[index][0], ETH_ALEN)) {
					if (key_type != 3)
						issue_deauth_11w(padapter, &mac_addr[index][0], 0, (u8)key_type);

					psta = rtw_get_stainfo(pstapriv, &mac_addr[index][0]);
					if (psta && key_type != IEEE80211W_WRONG_KEY && key_type != IEEE80211W_NO_KEY) {
						_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
						if (rtw_is_list_empty(&psta->asoc_list) == _FALSE) {
							rtw_list_delete(&psta->asoc_list);
							pstapriv->asoc_list_cnt--;
							#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
							if (psta->tbtx_enable)
								pstapriv->tbtx_asoc_list_cnt--;
							#endif
							updated |= ap_free_sta(padapter, psta, _FALSE, WLAN_REASON_PREV_AUTH_NOT_VALID, _TRUE, _FALSE);

						}
						_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
					}

					RTW_INFO("STA[%u]:"MAC_FMT"\n", index , MAC_ARG(&mac_addr[index][0]));
				}
			}
		}

		associated_clients_update(padapter, updated, STA_INFO_UPDATE_ALL);
	}

	return count;
}

int proc_get_tx_deauth(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "%s\n", __func__);
	return 0;
}

ssize_t proc_set_tx_auth(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	/* macid_ctl move to phl */
	/*struct macid_ctl_t *macid_ctl = dvobj_to_macidctl(dvobj); */
	struct sta_info *psta;
	_list	*plist, *phead;
	char tmp[16];
	u8	mac_addr[NUM_STA][ETH_ALEN];
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u32 tx_auth = 0;
	u8 index;


	if (count > 2) {
		RTW_INFO("argument size is more than 2\n");
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, sizeof(tmp))) {

		int num = sscanf(tmp, "%x", &tx_auth);

		if (num !=  1) {
			RTW_INFO("invalid read_reg parameter!\n");
			return count;
		}
		RTW_INFO("1: setnd auth, 2: send assoc request. tx_auth=%d\n", tx_auth);

		if (MLME_IS_STA(padapter)
	    		&& (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)) {
			if (tx_auth == 1) {
				/* TX unicast auth to AP */
				issue_auth(padapter, NULL, 0);
			} else if (tx_auth == 2) {
				/* TX unicast auth to AP */
				issue_assocreq(padapter);
			}
		}
	}

	return count;
}

int proc_get_tx_auth(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "%s\n", __func__);
	return 0;
}
#endif /* CONFIG_IEEE80211W */


#ifdef CONFIG_MCC_MODE
int proc_get_mcc_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	dump_adapters_status(m, adapter_to_dvobj(adapter));
	rtw_hal_dump_mcc_info(m, adapter_to_dvobj(adapter));
	return 0;
}

int proc_get_mcc_policy_table(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rtw_hal_dump_mcc_policy_table(m);
	return 0;
}

ssize_t proc_set_mcc_enable(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 en_mcc = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		_adapter *iface = NULL;
		u8 i = 0;
		int num = sscanf(tmp, "%u", &en_mcc);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: en_mcc = %d\n", __func__, en_mcc);

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if (!iface)
				continue;
			iface->registrypriv.en_mcc = en_mcc;
		}
	}

	return count;
}

ssize_t proc_set_mcc_duration(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 enable_runtime_duration = 0, mcc_duration = 0, type = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %u %u", &enable_runtime_duration, &type, &mcc_duration);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		if (num > 3) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters > 2\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		if (num == 2) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters > 2\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		if (num >= 1) {
			SET_MCC_RUNTIME_DURATION(padapter, enable_runtime_duration);
			RTW_INFO("runtime duration:%s\n", enable_runtime_duration ? "enable":"disable");
		}

		if (num == 3) {
			RTW_INFO("type:%d, mcc duration:%d\n", type, mcc_duration);
			rtw_set_mcc_duration_cmd(padapter, type, mcc_duration);
		}
	}

	return count;
}

#ifdef CONFIG_MCC_PHYDM_OFFLOAD
ssize_t proc_set_mcc_phydm_offload_enable(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_phydm_enable = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		u8 i = 0;
		int num = sscanf(tmp, "%u", &mcc_phydm_enable);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc phydm enable = %d\n", __func__, mcc_phydm_enable);
		rtw_set_mcc_phydm_offload_enable_cmd(padapter, mcc_phydm_enable, _TRUE);
	}

	return count;
}
#endif

ssize_t proc_set_mcc_single_tx_criteria(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_single_tx_criteria = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		_adapter *iface = NULL;
		u8 i = 0;
		int num = sscanf(tmp, "%u", &mcc_single_tx_criteria);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc_single_tx_criteria = %d\n", __func__, mcc_single_tx_criteria);

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if (!iface)
				continue;
			iface->registrypriv.rtw_mcc_single_tx_cri = mcc_single_tx_criteria;
		}


	}

	return count;
}


ssize_t proc_set_mcc_ap_bw20_target_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_ap_bw20_target_tp = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &mcc_ap_bw20_target_tp);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc_ap_bw20_target_tp = %d\n", __func__, mcc_ap_bw20_target_tp);

		padapter->registrypriv.rtw_mcc_ap_bw20_target_tx_tp = mcc_ap_bw20_target_tp;


	}

	return count;
}

ssize_t proc_set_mcc_ap_bw40_target_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_ap_bw40_target_tp = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &mcc_ap_bw40_target_tp);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc_ap_bw40_target_tp = %d\n", __func__, mcc_ap_bw40_target_tp);

		padapter->registrypriv.rtw_mcc_ap_bw40_target_tx_tp = mcc_ap_bw40_target_tp;


	}

	return count;
}

ssize_t proc_set_mcc_ap_bw80_target_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_ap_bw80_target_tp = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &mcc_ap_bw80_target_tp);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc_ap_bw80_target_tp = %d\n", __func__, mcc_ap_bw80_target_tp);

		padapter->registrypriv.rtw_mcc_ap_bw80_target_tx_tp = mcc_ap_bw80_target_tp;


	}

	return count;
}

ssize_t proc_set_mcc_sta_bw20_target_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_sta_bw20_target_tp = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &mcc_sta_bw20_target_tp);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc_sta_bw20_target_tp = %d\n", __func__, mcc_sta_bw20_target_tp);

		padapter->registrypriv.rtw_mcc_sta_bw20_target_tx_tp = mcc_sta_bw20_target_tp;


	}

	return count;
}

ssize_t proc_set_mcc_sta_bw40_target_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_sta_bw40_target_tp = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &mcc_sta_bw40_target_tp);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc_sta_bw40_target_tp = %d\n", __func__, mcc_sta_bw40_target_tp);

		padapter->registrypriv.rtw_mcc_sta_bw40_target_tx_tp = mcc_sta_bw40_target_tp;


	}

	return count;
}

ssize_t proc_set_mcc_sta_bw80_target_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[255];
	u32 mcc_sta_bw80_target_tp = 0;

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n", FUNC_ADPT_ARG(padapter));
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is too large\n", FUNC_ADPT_ARG(padapter));
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &mcc_sta_bw80_target_tp);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		RTW_INFO("%s: mcc_sta_bw80_target_tp = %d\n", __func__, mcc_sta_bw80_target_tp);

		padapter->registrypriv.rtw_mcc_sta_bw80_target_tx_tp = mcc_sta_bw80_target_tp;


	}

	return count;
}
#endif /* CONFIG_MCC_MODE */

int proc_get_ack_timeout(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 ack_timeout_val = 0;
	u8 ack_timeout_val_cck = 0;

	rtw_hal_get_hwreg(padapter, HW_VAR_ACKTO, &ack_timeout_val);
	rtw_hal_get_hwreg(padapter, HW_VAR_ACKTO_CCK, &ack_timeout_val_cck);

	RTW_PRINT_SEL(m, "Current CCK packet ACK Timeout = %d us (0x%x).\n", ack_timeout_val_cck, ack_timeout_val_cck);
	RTW_PRINT_SEL(m, "Current non-CCK packet ACK Timeout = %d us (0x%x).\n", ack_timeout_val, ack_timeout_val);
	return 0;
}

ssize_t proc_set_ack_timeout(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 tmp_v1 = 0, tmp_v2 = 0;
	u8 ack_to_ms = 0, ack_to_cck_ms = 0;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %u", &tmp_v1, &tmp_v2);

		if (num < 1) {
			RTW_INFO(FUNC_ADPT_FMT ": input parameters < 1\n", FUNC_ADPT_ARG(padapter));
			return -EINVAL;
		}

		if (tmp_v1) {
			ack_to_ms = (u8)tmp_v1;
			rtw_hal_set_hwreg(padapter, HW_VAR_ACKTO, &ack_to_ms);
		}

		if (tmp_v2) {
			ack_to_cck_ms = (u8)tmp_v2;
			rtw_hal_set_hwreg(padapter, HW_VAR_ACKTO_CCK, &ack_to_cck_ms);
		}
		/* This register sets the Ack time out value after Tx unicast packet. It is in units of us. */
		/* This register sets the Ack time out value after Tx unicast CCK packet. It is in units of us. */
		RTW_INFO("Set CCK packet ACK Timeout to %d us.\n", ack_to_cck_ms);
		RTW_INFO("Set non-CCK packet ACK Timeout to %d us.\n", ack_to_ms);
	}

	return count;
}

ssize_t proc_set_fw_offload(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{

#if 0 /*GEORGIA_TODO_FIXIT*/
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	_adapter *pri_adapter = GET_PRIMARY_ADAPTER(adapter);
	HAL_DATA_TYPE *hal = GET_HAL_DATA(adapter_to_dvobj(adapter));
	char tmp[32];
	u32 iqk_offload_enable = 0, ch_switch_offload_enable = 0;

	if (buffer == NULL) {
		RTW_INFO("input buffer is NULL!\n");
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO("input length is 0!\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO("input length is too large\n");
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%d %d", &iqk_offload_enable, &ch_switch_offload_enable);

		if (num < 2) {
			RTW_INFO("input parameters < 1\n");
			return -EINVAL;
		}

		if (hal->RegIQKFWOffload != iqk_offload_enable) {
			hal->RegIQKFWOffload = iqk_offload_enable;
			rtw_run_in_thread_cmd(pri_adapter, ((void *)(rtw_hal_update_iqk_fw_offload_cap)), pri_adapter);
		}

		if (hal->ch_switch_offload != ch_switch_offload_enable)
			hal->ch_switch_offload = ch_switch_offload_enable;
	}

	return count;
#endif
	return count;
}

int proc_get_fw_offload(struct seq_file *m, void *v)
{
#if 0 /*GEORGIA_TODO_FIXIT*/
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	HAL_DATA_TYPE *hal = GET_HAL_DATA(adapter_to_dvobj(adapter));


	RTW_PRINT_SEL(m, "IQK FW offload:%s\n", hal->RegIQKFWOffload?"enable":"disable");
	RTW_PRINT_SEL(m, "Channel switch FW offload:%s\n", hal->ch_switch_offload?"enable":"disable");
#endif
	return 0;
}

#ifdef CONFIG_CTRL_TXSS_BY_TP
ssize_t proc_set_txss_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);

	char tmp[32];
	u32 enable = 0;
	u32 txss_tx_tp = 0;
	int txss_chk_cnt = 0;

	if (buffer == NULL) {
		RTW_INFO("input buffer is NULL!\n");
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO("input length is 0!\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO("input length is too large\n");
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %u %d",
			&enable, &txss_tx_tp, &txss_chk_cnt);

		if (num < 1) {
			RTW_INFO("input parameters < 1\n");
			return -EINVAL;
		}
		pmlmeext->txss_ctrl_en = enable;

		if (txss_tx_tp)
			pmlmeext->txss_tp_th = txss_tx_tp;
		if (txss_chk_cnt)
			pmlmeext->txss_tp_chk_cnt = txss_chk_cnt;

		RTW_INFO("%s txss_ctl_en :%s , txss_tp_th:%d, tp_chk_cnt:%d\n",
			__func__, pmlmeext->txss_tp_th ? "Y" : "N",
			pmlmeext->txss_tp_th, pmlmeext->txss_tp_chk_cnt);

	}

	return count;
}

int proc_get_txss_tp(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);

	RTW_PRINT_SEL(m, "TXSS  Control - %s\n", pmlmeext->txss_ctrl_en ? "enable" : "disable");
	RTW_PRINT_SEL(m, "TXSS  Tx TP TH - %d\n", pmlmeext->txss_tp_th);
	RTW_PRINT_SEL(m, "TXSS  check cnt - %d\n", pmlmeext->txss_tp_chk_cnt);

	return 0;
}
#ifdef DBG_CTRL_TXSS
ssize_t proc_set_txss_ctrl(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);

	char tmp[32];
	u32 tx_1ss = 0;

	if (buffer == NULL) {
		RTW_INFO("input buffer is NULL!\n");
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO("input length is 0!\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO("input length is too large\n");
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u",	&tx_1ss);

		if (num < 1) {
			RTW_INFO("input parameters < 1\n");
			return -EINVAL;
		}

		pmlmeext->txss_ctrl_en = _FALSE;

		dbg_ctrl_txss(adapter, tx_1ss);

		RTW_INFO("%s set tx to  1ss :%s\n", __func__, tx_1ss ? "Y" : "N");
	}

	return count;
}

int proc_get_txss_ctrl(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);

	RTW_PRINT_SEL(m, "TXSS  1ss - %s\n", pmlmeext->txss_1ss ? "Y" : "N");

	return 0;
}
#endif
#endif

#ifdef CONFIG_DBG_RF_CAL
int proc_get_iqk_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	return 0;
}

ssize_t proc_set_iqk(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 recovery, clear, segment;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d %d %d", &recovery, &clear, &segment);

		if (num != 3) {
			RTW_INFO("Invalid format\n");
			return count;
		}

		rtw_hal_iqk_test(padapter, recovery, clear, segment);
	}

	return count;

}

int proc_get_lck_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	return 0;
}

ssize_t proc_set_lck(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 trigger;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%d", &trigger);

		if (num != 1) {
			RTW_INFO("Invalid format\n");
			return count;
		}

		rtw_hal_lck_test(padapter);
	}

	return count;
}
#endif /* CONFIG_DBG_RF_CAL */

#ifdef CONFIG_LPS_CHK_BY_TP
ssize_t proc_set_lps_chk_tp(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);
	char tmp[32];
	u32 enable = 0;
	u32 lps_tx_tp = 0, lps_rx_tp = 0, lps_bi_tp = 0;
	int lps_chk_cnt_th = 0;
	u32 lps_tx_pkts = 0, lps_rx_pkts = 0;

	if (buffer == NULL) {
		RTW_INFO("input buffer is NULL!\n");
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO("input length is 0!\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO("input length is too large\n");
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %u %u %u %d %u %u",
			&enable, &lps_tx_tp, &lps_rx_tp, &lps_bi_tp,
			&lps_chk_cnt_th, &lps_tx_pkts, &lps_rx_pkts);

		if (num < 1) {
			RTW_INFO("input parameters < 1\n");
			return -EINVAL;
		}
		pwrpriv->lps_chk_by_tp = enable;

		if (lps_tx_tp) {
			pwrpriv->lps_tx_tp_th = lps_tx_tp;
			pwrpriv->lps_rx_tp_th = lps_tx_tp;
			pwrpriv->lps_bi_tp_th = lps_tx_tp;
		}
		if (lps_rx_tp)
			pwrpriv->lps_rx_tp_th = lps_rx_tp;
		if (lps_bi_tp)
			pwrpriv->lps_bi_tp_th = lps_bi_tp;

		if (lps_chk_cnt_th)
			pwrpriv->lps_chk_cnt_th = lps_chk_cnt_th;

		if (lps_tx_pkts)
			pwrpriv->lps_tx_pkts = lps_tx_pkts;

		if (lps_rx_pkts)
			pwrpriv->lps_rx_pkts = lps_rx_pkts;

		RTW_INFO("%s lps_chk_by_tp:%s , lps_tx_tp_th:%d, lps_tx_tp_th:%d, lps_bi_tp:%d\n",
			__func__, pwrpriv->lps_chk_by_tp ? "Y" : "N",
			pwrpriv->lps_tx_tp_th, pwrpriv->lps_tx_tp_th, pwrpriv->lps_bi_tp_th);
		RTW_INFO("%s lps_chk_cnt_th:%d , lps_tx_pkts:%d, lps_rx_pkts:%d\n",
			__func__, pwrpriv->lps_chk_cnt_th, pwrpriv->lps_tx_pkts, pwrpriv->lps_rx_pkts);
	}

	return count;
}

int proc_get_lps_chk_tp(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);

	RTW_PRINT_SEL(m, "LPS chk by tp - %s\n", pwrpriv->lps_chk_by_tp ? "enable" : "disable");
	RTW_PRINT_SEL(m, "LPS Tx TP TH - %d(Mbps)\n", pwrpriv->lps_tx_tp_th);
	RTW_PRINT_SEL(m, "LPS Rx TP TH - %d(Mbps)\n", pwrpriv->lps_rx_tp_th);
	RTW_PRINT_SEL(m, "LPS BI TP TH - %d(Mbps)\n", pwrpriv->lps_bi_tp_th);

	RTW_PRINT_SEL(m, "LPS CHK CNT - %d\n", pwrpriv->lps_chk_cnt_th);
	RTW_PRINT_SEL(m, "LPS Tx PKTs - %d\n", pwrpriv->lps_tx_pkts);
	RTW_PRINT_SEL(m, "LPS Rx PKTs - %d\n", pwrpriv->lps_rx_pkts);
	return 0;
}
#endif /*CONFIG_LPS_CHK_BY_TP*/
#ifdef CONFIG_SUPPORT_STATIC_SMPS
ssize_t proc_set_smps(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	char tmp[32];
	u32 enable = 0;
	u32 smps_en, smps_tx_tp = 0, smps_rx_tp = 0;
	u32 smps_test = 0, smps_test_en = 0;

	if (buffer == NULL) {
		RTW_INFO("input buffer is NULL!\n");
		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO("input length is 0!\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		RTW_INFO("input length is too large\n");
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %u %u %u %u", &smps_en, &smps_tx_tp, &smps_rx_tp,
			&smps_test, &smps_test_en);

		if (num < 1) {
			RTW_INFO("input parameters < 1\n");
			return -EINVAL;
		}

		pmlmeext->ssmps_en = smps_en;
		if (smps_tx_tp) {
			pmlmeext->ssmps_tx_tp_th= smps_tx_tp;
			pmlmeext->ssmps_rx_tp_th= smps_tx_tp;
		}
		if (smps_rx_tp)
			pmlmeext->ssmps_rx_tp_th = smps_rx_tp;

		#ifdef DBG_STATIC_SMPS
		if (num > 3) {
			pmlmeext->ssmps_test = smps_test;
			pmlmeext->ssmps_test_en = smps_test_en;
		}
		#endif
		RTW_INFO("SM PS : %s tx_tp_th:%d, rx_tp_th:%d\n",
			(smps_en) ? "Enable" : "Disable",
			pmlmeext->ssmps_tx_tp_th,
			pmlmeext->ssmps_rx_tp_th);
		#ifdef DBG_STATIC_SMPS
		RTW_INFO("SM PS : %s ssmps_test_en:%d\n",
			(smps_test) ? "Enable" : "Disable",
			pmlmeext->ssmps_test_en);
		#endif
	}

	return count;
}

int proc_get_smps(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);

	RTW_PRINT_SEL(m, "Static SMPS %s\n", pmlmeext->ssmps_en ? "enable" : "disable");
	RTW_PRINT_SEL(m, "Tx TP TH %d\n", pmlmeext->ssmps_tx_tp_th);
	RTW_PRINT_SEL(m, "Rx TP TH %d\n", pmlmeext->ssmps_rx_tp_th);
	#ifdef DBG_STATIC_SMPS
	RTW_PRINT_SEL(m, "test %d, test_en:%d\n", pmlmeext->ssmps_test, pmlmeext->ssmps_test_en);
	#endif
	return 0;
}
#endif /*CONFIG_SUPPORT_STATIC_SMPS*/

int proc_get_chan(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	if(!rtw_is_adapter_up(padapter)){
		RTW_PRINT("%s(%d) The interface is not running.\n", __FUNCTION__, __LINE__);
		return 0;
	}

	RTW_PRINT_SEL(m, "ch=%d, ch_offset=%d, bw=%d\n",
			rtw_get_oper_ch(padapter),
			rtw_get_oper_choffset(padapter),
			rtw_get_oper_bw(padapter));
	if(padapter->phl_role){
		RTW_PRINT_SEL(m, "PHL ch=%d\n",
				rtw_phl_get_cur_ch(padapter->phl_role));
	}
	return 0;
}

ssize_t proc_set_chan(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8 target_ch, target_offset, target_bw;
#ifdef DBG_CONFIG_CMD_DISP
	u8 cmd_type = 0xFF;
	u8 cmd_to = 0;
#endif

	if(!rtw_is_adapter_up(padapter)){
		RTW_PRINT("%s(%d) The interface is not running.\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (count < 3) {
		RTW_INFO("argument size is less than 3\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = 0;
#ifdef DBG_CONFIG_CMD_DISP
		num = sscanf(tmp, "%hhu %hhu %hhu %hhu %hhu",
			&target_ch, &target_offset, &target_bw,
			&cmd_type, &cmd_to);
		if (num != 5) {
			RTW_INFO("invalid write_reg parameter!\n");
			return count;
		}

		padapter->cmd_type = cmd_type;
		padapter->cmd_timeout = cmd_to;
#else
		num = sscanf(tmp, "%hhu %hhu %hhu",
			&target_ch, &target_offset, &target_bw);
		if (num != 3) {
			RTW_INFO("invalid write_reg parameter!\n");
			return count;
		}
#endif
		set_channel_bwmode(padapter, target_ch, target_offset, target_bw, _FALSE);
	}

	return count;
}

int proc_get_deny_legacy(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	if (pregpriv)
		RTW_PRINT_SEL(m, "deny_legacy       = 0x%x\n", pregpriv->deny_legacy);
	if (pmlmeext)
		RTW_PRINT_SEL(m, "cur_wireless_mode = 0x%x\n", pmlmeext->cur_wireless_mode);

	return 0;
}

ssize_t proc_set_deny_legacy(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;
	char tmp[32];
	u8 legacy;

	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		int num = sscanf(tmp, "%hhu ", &legacy);

		if (num == 1 && pregpriv) {
			pregpriv->deny_legacy = legacy;
			RTW_INFO("deny_legacy=%d\n", legacy);
		}
	}

	return count;
}

static const char *rtw_data_gi_str(u8 gi, u16 tx_rate)
{
	if (tx_rate >= RTW_DATA_RATE_HE_NSS1_MCS0) {
		/* HE data rates */
		switch (gi) {
		case 0:
			return "[4x3.2]";
		case 1:
			return "[4x0.8]";
		case 2:
			return "[2x1.6]";
		case 3:
			return "[2x0.8]";
		case 4:
			return "[1x1.6]";
		case 5:
			return "[1x0.8]";
		default:
			return "[err]";
		}
	} else {
		/* non-HE data rates */
		if(gi)
			return "s";
		else
			return "";
	}
}

void get_sta_cur_tx_rate(_adapter *adapter, struct sta_info *psta)
{
#if 1
	return;
#else
	u8 res1 = _FAIL;
	u32 cur_tx_rate = 0;
	res1 = get_cmac_8852ae(adapter, psta->phl_sta->macid, "DATARATE", &cur_tx_rate);
	if (res1 == _SUCCESS)
		psta->cur_tx_data_rate = cur_tx_rate;
#endif
}

void display_sta_dump(_adapter *adapter, struct sta_info *psta)
{
	u32 rx_retry_rate = 0;
	u16 sgi = 0, fix_rate = 0;
	u8 buf[200] = {0}, n = 0;
	u8 macid = 0;

	if ( !(adapter->sta_dump_to) || !adapter->sta_dump_bitmap || !psta)
		return;

	if (!psta || !psta->phl_sta)
		return;

	psta->sta_stats.my_tx_tp_kbits = psta->sta_stats.tx_tp_kbits;
	psta->sta_stats.my_tx_tp_kbits = psta->sta_stats.tx_tp_kbits;

	if (adapter->up_time % adapter->sta_dump_to)
		return;

	n += _sprintf(buf + n, "[%d/%d]", psta->phl_sta->aid, psta->phl_sta->macid);

	if (adapter->sta_dump_bitmap & RTW_STA_DUMP_RSSI) {
		/* ToDo: need to API to init hal_sta->ra_info->rssi_stat.rssi  */
		n += _sprintf(buf + n, "(Rssi %d)", psta->phl_sta->hal_sta->rssi_stat.rssi >> 1);
	}

	if (adapter->sta_dump_bitmap & RTW_STA_DUMP_RETRY) {
		/*  ToDo: tx retry not ready yet */
		if (psta->sta_stats.rx_data_pkts_cur == 0)
			rx_retry_rate = 0;
		else
			rx_retry_rate = (psta->sta_stats.rx_data_retry_pkts_cur*100)/psta->sta_stats.rx_data_pkts_cur;
		if(rx_retry_rate >= 100)
			rx_retry_rate = 100;

		n += _sprintf(buf + n, "(rty %d%%, %d%%)", psta->sta_stats.tx_retry_ratio, rx_retry_rate);
	}

	if (adapter->sta_dump_bitmap & RTW_STA_DUMP_RATE) {
		n += _sprintf(buf + n, "(");

		if (adapter->fix_rate != NO_FIX_RATE) {/* fix rate */
			fix_rate = GET_FIX_RATE(adapter->fix_rate);
			sgi = GET_FIX_RATE_SGI(adapter->fix_rate);
			n += _sprintf(buf + n, "force_%s", rtw_data_rate_str(fix_rate));
			if (sgi)
				n += _sprintf(buf + n, "%s", rtw_data_gi_str(sgi, fix_rate));
		} else if (adapter->txForce_enable &&
			adapter->txForce_rate != INV_TXFORCE_VAL) {
			n += _sprintf(buf + n, "force_%s", rtw_data_rate_str(adapter->txForce_rate));
			if (adapter->txForce_gi != INV_TXFORCE_VAL)
				n += _sprintf(buf + n, "%s", rtw_data_gi_str(adapter->txForce_gi, adapter->txForce_rate));
		} else if (psta->fixRate != 0 && psta->fixRate != NO_FIX_RATE) {
			fix_rate = GET_FIX_RATE(psta->fixRate);
			sgi = GET_FIX_RATE_SGI(psta->fixRate);
			n += _sprintf(buf + n, "force_%s", rtw_data_rate_str(fix_rate));
			if (sgi)
				n += _sprintf(buf + n, "%s", rtw_data_gi_str(sgi, fix_rate));
		} else {
			n += _sprintf(buf + n, "%s%s", rtw_data_rate_str(psta->cur_tx_data_rate),
						   rtw_data_gi_str(psta->cur_tx_gi_ltf, psta->cur_tx_data_rate));
		}
		n += _sprintf(buf + n, ", %s%s)", rtw_data_rate_str(psta->cur_rx_data_rate),
						  rtw_data_gi_str(psta->cur_rx_gi_ltf, psta->cur_rx_data_rate));
	}

	if (adapter->sta_dump_bitmap & RTW_STA_DUMP_RSSI_PATH) {
		n += snprintf(buf + n, 17, "(%d %d %d %d)", (psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[0] >> 5),
							(psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[1] >> 5),
							(psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[2] >> 5),
							(psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[3] >> 5));
	}

	if (adapter->sta_dump_bitmap & RTW_STA_DUMP_FA) {
		n += _sprintf(buf + n, "(FA %u)", adapter->dvobj->hw_stat.cur_fa_cnt);
	}

	if (adapter->sta_dump_bitmap & RTW_STA_DUMP_CCA) {
		n += _sprintf(buf + n, "(CCA %d)", adapter->dvobj->hw_stat.cur_cca_cnt);
	}

	if (adapter->sta_dump_bitmap & RTW_STA_DUMP_TP)
		n += _sprintf(buf + n, "(TP %d, %d)", (psta->sta_stats.tx_tp_kbits >> 10), (psta->sta_stats.rx_tp_kbits >> 10));

#ifdef CONFIG_LMT_TXREQ
	if (adapter->lmt_txreq_dump)
		n += _sprintf(buf + n, "(txreq L%d, Q%d, D%d)", psta->lmt_pending_txreq,
			ATOMIC_READ(&psta->num_pending_txreq), psta->lmt_txreq_drop);
#endif

	_dbgdump("%s\n", buf);
}

#ifdef CONFIG_WFA_OFDMA_Logo_Test
void display_sta_ofdma_info_dump(_adapter *adapter, struct sta_info *psta)
{
	u32 rx_retry_rate = 0;
	u32 tx_retry_rate = 0;
	u16 sgi = 0, fix_rate = 0;

	if ( !(adapter->sta_dump_to) || !adapter->sta_dump_bitmap)
		return;

	if (!psta || !psta->phl_sta)
		return;

	if (adapter->up_time % adapter->sta_dump_to)
		return;

	_dbgdump("[%d]", psta->phl_sta->aid);
	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RSSI) {
		_dbgdump("(Rssi %d)", psta->phl_sta->ru_stats.rssi_m);
	}

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_TP) {
		_dbgdump("(TP %d, %d)", (psta->sta_stats.tx_tp_kbits >> 10), (psta->sta_stats.rx_tp_kbits >> 10));
	}

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_PHY_RATE) {
		_dbgdump("(");

		_dbgdump("%s", rtw_data_rate_str(psta->phl_sta->ru_stats.ru_tx_phy_rate));

		_dbgdump(", %s)", rtw_data_rate_str(psta->phl_sta->stats.HE_TB_stat.HE_TB_avg_rx_rate));
	}

	_dbgdump("[tx]");

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_TX_RU_RATIO) {
		_dbgdump("(ru ratio %d)", psta->phl_sta->ru_stats.su_ru_ratio);
	}

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_TX_RU_FAIL_RATIO) {
		_dbgdump("(su fail %d)", psta->phl_sta->ru_stats.su_fail_ratio);
		_dbgdump("(ru fail %d)", psta->phl_sta->ru_stats.ru_fail_ratio);
	}

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_TX_AGG_CNT) {
		_dbgdump("(amsdu %d)", psta->txsc_amsdu_num);
		_dbgdump("(agg %d)", psta->phl_sta->ru_stats.ru_tx_agg_cnt);
	}

	_dbgdump("[rx]");

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_AGG_CNT)
		_dbgdump("(agg %d %d)", psta->phl_sta->stats.HE_TB_stat.avg_ampdu_agg_1, psta->phl_sta->stats.HE_TB_stat.avg_ampdu_agg_2);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_TF_CNT)
		_dbgdump("(tf cnt %d)", psta->phl_sta->ru_stats.ru_rx_tf_cnt);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_TF_BSR_LEN)
			_dbgdump("(bsr len %d)", psta->phl_sta->ru_stats.ru_rx_bsr_len);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_CCA_RATIO)
		_dbgdump("(cca %d)", psta->phl_sta->ru_stats.ru_rx_cca_miss);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_FAIL_RATIO)
		_dbgdump("(all error %d)", psta->phl_sta->ru_stats.ru_rx_error);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_CRC32_RATIO)
		_dbgdump("(crc32 %d)", psta->phl_sta->stats.HE_TB_stat.crc32_ratio);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_UPH_PW)
		_dbgdump("(headroom %d)", psta->phl_sta->ru_stats.ru_rx_uph_headroom);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_UPH_MIN_Flag)
		_dbgdump("(min_flag %d)", psta->phl_sta->ru_stats.ru_rx_min_flag_ratio);

	if (adapter->sta_ru_dump_bitmap & RTW_STA_DUMP_RU_RX_EVM)
		_dbgdump("(evm %d)", psta->phl_sta->ru_stats.ru_rx_evm);
	_dbgdump("\n");
}
#endif

void reset_accumulated_tp(_adapter *adapter)
{
	adapter->tp_total_tx = 0;
	adapter->tp_total_rx = 0;
	adapter->tp_total_trx = 0;

	return;
}

void accumulate_whole_tp(_adapter *adapter, struct sta_info *psta)
{
	adapter->tp_total_tx += psta->sta_stats.tx_tp_kbits;
	adapter->tp_total_rx += psta->sta_stats.rx_tp_kbits;
	adapter->tp_total_trx = adapter->tp_total_tx + adapter->tp_total_rx;

	return;
}

int proc_get_whole_tp(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	u32 tx_tp = 0, rx_tp = 0, trx_tp = 0;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	tx_tp = (padapter->tp_total_tx >> 10);
	rx_tp = (padapter->tp_total_rx  >> 10);
	trx_tp = (padapter->tp_total_trx >> 10);
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	RTW_PRINT_SEL(m, "tx=%u Mbps, rx=%u Mbps, trx=%u Mbps\n", tx_tp, rx_tp, trx_tp);
	return 0;
}

#if defined(CONFIG_PCI_HCI) && defined(CONFIG_PCI_TRX_RES_DBG)
int proc_get_bd_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int i;
	u16 host_idx = 0, hw_idx = 0, hw_res = 0;

	RTW_PRINT_SEL(m, "bd info...\n");
	RTW_PRINT_SEL(m, "  TX queue:\n");
	for (i = 0; i < rtw_phl_get_txch_num(GET_HAL_INFO(padapter->dvobj)); i++) {
		host_idx = 0;
		hw_idx = 0;
		hw_res = 0;
		rtw_phl_get_txbd(GET_HAL_INFO(padapter->dvobj),
						i, &host_idx, &hw_idx, &hw_res);
		RTW_PRINT_SEL(m, "    queue %2d: hostIdx/hwIdx/avail =%4d/%4d/%4d\n", i, host_idx, hw_idx, hw_res);
	}

	RTW_PRINT_SEL(m, "  RX queue:\n");
	for (i = 0; i < rtw_phl_get_rxch_num(GET_HAL_INFO(padapter->dvobj)); i++) {
		host_idx = 0;
		hw_idx = 0;
		hw_res = 0;
		rtw_phl_get_rxbd(GET_HAL_INFO(padapter->dvobj),
						i, &host_idx, &hw_idx, &hw_res);
		RTW_PRINT_SEL(m, "    queue %2d: hostIdx/hwIdx/idle  =%4d/%4d/%4d\n", i, host_idx, hw_idx, hw_res);
	}
	return 0;
}
#endif

#endif /* CONFIG_PROC_DEBUG */
#define RTW_BUFDUMP_BSIZE		16
#if 1
inline void RTW_BUF_DUMP_SEL(uint _loglevel, void *sel, u8 *_titlestring,
					bool _idx_show, const u8 *_hexdata, int _hexdatalen)
{
#ifdef CONFIG_RTW_DEBUG
	int __i;
	u8 *ptr = (u8 *)_hexdata;

	if (_loglevel <= rtw_drv_log_level) {
		if (_titlestring) {
			if (sel == RTW_DBGDUMP)
				RTW_PRINT("");
			_RTW_PRINT_SEL(sel, "%s", _titlestring);
			if (_hexdatalen >= RTW_BUFDUMP_BSIZE)
				_RTW_PRINT_SEL(sel, "\n");
		}

		for (__i = 0; __i < _hexdatalen; __i++) {
			if (((__i % RTW_BUFDUMP_BSIZE) == 0) && (_hexdatalen >= RTW_BUFDUMP_BSIZE)) {
				if (sel == RTW_DBGDUMP)
					RTW_PRINT("");
				if (_idx_show)
					_RTW_PRINT_SEL(sel, "0x%03X: ", __i);
			}
			_RTW_PRINT_SEL(sel, "%02X%s", ptr[__i], (((__i + 1) % 4) == 0) ? "  " : " ");
			if ((__i + 1 < _hexdatalen) && ((__i + 1) % RTW_BUFDUMP_BSIZE) == 0)
				_RTW_PRINT_SEL(sel, "\n");
		}
		_RTW_PRINT_SEL(sel, "\n");
	}
#endif
}

inline void RTW_BUF_DUMP_SEL_ALWAYS(void *sel, u8 *_titlestring,
					bool _idx_show, const u8 *_hexdata, int _hexdatalen)
{
	int __i;
	u8 *ptr = (u8 *)_hexdata;

	if (_titlestring) {
		if (sel == RTW_DBGDUMP)
			RTW_PRINT("");
		_RTW_PRINT_SEL(sel, "%s", _titlestring);
		if (_hexdatalen >= RTW_BUFDUMP_BSIZE)
			_RTW_PRINT_SEL(sel, "\n");
	}

	for (__i = 0; __i < _hexdatalen; __i++) {
		if (((__i % RTW_BUFDUMP_BSIZE) == 0) && (_hexdatalen >= RTW_BUFDUMP_BSIZE)) {
			if (sel == RTW_DBGDUMP)
				RTW_PRINT("");
			if (_idx_show)
				_RTW_PRINT_SEL(sel, "0x%03X: ", __i);
		}
		_RTW_PRINT_SEL(sel, "%02X%s", ptr[__i], (((__i + 1) % 4) == 0) ? "  " : " ");
		if ((__i + 1 < _hexdatalen) && ((__i + 1) % RTW_BUFDUMP_BSIZE) == 0)
			_RTW_PRINT_SEL(sel, "\n");
	}
	_RTW_PRINT_SEL(sel, "\n");
}

#else
inline void _RTW_STR_DUMP_SEL(void *sel, char *str_out)
{
	if (sel == RTW_DBGDUMP)
		_dbgdump("%s\n", str_out);
	#if defined(_seqdump)
	else
		_seqdump(sel, "%s\n", str_out);
	#endif /*_seqdump*/
}
inline void RTW_BUF_DUMP_SEL(uint _loglevel, void *sel, u8 *_titlestring,
					bool _idx_show, u8 *_hexdata, int _hexdatalen)
{
	int __i, len;
	int __j, idx;
	int block_num, remain_byte;
	char str_out[128] = {'\0'};
	char str_val[32] = {'\0'};
	char *p = NULL;
	u8 *ptr = (u8 *)_hexdata;

	if (_loglevel <= rtw_drv_log_level) {
		/*dump title*/
		p = &str_out[0];
		if (_titlestring) {
			if (sel == RTW_DBGDUMP) {
				len = snprintf(str_val, sizeof(str_val), "%s", DRIVER_PREFIX);
				strncpy(p, str_val, len);
				p += len;
			}
			len = snprintf(str_val, sizeof(str_val), "%s", _titlestring);
			strncpy(p, str_val, len);
			p += len;
		}
		if (p != &str_out[0]) {
			_RTW_STR_DUMP_SEL(sel, str_out);
			_rtw_memset(&str_out, '\0', sizeof(str_out));
		}

		/*dump buffer*/
		block_num = _hexdatalen / RTW_BUFDUMP_BSIZE;
		remain_byte = _hexdatalen % RTW_BUFDUMP_BSIZE;
		for (__i = 0; __i < block_num; __i++) {
			p = &str_out[0];
			if (sel == RTW_DBGDUMP) {
				len = snprintf(str_val, sizeof(str_val), "%s", DRIVER_PREFIX);
				strncpy(p, str_val, len);
				p += len;
			}
			if (_idx_show) {
				len = snprintf(str_val, sizeof(str_val), "0x%03X: ", __i * RTW_BUFDUMP_BSIZE);
				strncpy(p, str_val, len);
				p += len;
			}
			for (__j =0; __j < RTW_BUFDUMP_BSIZE; __j++) {
				idx = __i * RTW_BUFDUMP_BSIZE + __j;
				len = snprintf(str_val, sizeof(str_val), "%02X%s", ptr[idx], (((__j + 1) % 4) == 0) ? "  " : " ");
				strncpy(p, str_val, len);
				p += len;
			}
			_RTW_STR_DUMP_SEL(sel, str_out);
			_rtw_memset(&str_out, '\0', sizeof(str_out));
		}

		p = &str_out[0];
		if ((sel == RTW_DBGDUMP) && remain_byte) {
			len = snprintf(str_val, sizeof(str_val), "%s", DRIVER_PREFIX);
			strncpy(p, str_val, len);
			p += len;
		}
		if (_idx_show && remain_byte) {
			len = snprintf(str_val, sizeof(str_val), "0x%03X: ", block_num * RTW_BUFDUMP_BSIZE);
			strncpy(p, str_val, len);
			p += len;
		}
		for (__i = 0; __i < remain_byte; __i++) {
			idx = block_num * RTW_BUFDUMP_BSIZE + __i;
			len = snprintf(str_val, sizeof(str_val), "%02X%s", ptr[idx], (((__i + 1) % 4) == 0) ? "  " : " ");
			strncpy(p, str_val, len);
			p += len;
		}
		_RTW_STR_DUMP_SEL(sel, str_out);
	}
}

#endif
