
#include <drv_types.h>
#include "_hal_rate.h"
#include "../phl/hal_g6/hal_general_def.h"
#include "../phl/hal_g6/hal_def.h"
#include "../phl/phl_headers.h"

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
extern int wireless_mode_to_str(u32 mode, char *str);
extern void dump_sec_to_str(struct seq_file *m, unsigned int time_in_sec, char* topic);
extern const char *rtw_data_rate_str(enum rtw_data_rate rate);
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
extern u32 rtw_wapi_gcm_sm4_test(u8 index);
#endif

const u8* LEGACY_DATA_RATE[12] =
{
	"1", "2", "5.5", "11", "6", "9",
	"12", "18", "24", "36", "48", "54"
};

const unsigned char* MCS_DATA_RATEStr[2][2][32] =
{
	{{"6.5", "13", "19.5", "26", "39", "52", "58.5", "65",
	  "13", "26", "39" ,"52", "78", "104", "117", "130",
	  "19.5", "39", "58.5", "78" ,"117", "156", "175.5", "195",
	  "26", "52", "78", "104", "156", "208", "234", "260"},					// Long GI, 20MHz

	 {"7.2", "14.4", "21.7", "28.9", "43.3", "57.8", "65", "72.2",
	  "14.4", "28.9", "43.3", "57.8", "86.7", "115.6", "130", "144.5",
	  "21.7", "43.3", "65", "86.7", "130", "173.3", "195", "216.7",
	  "28.9", "57.8", "86.7", "115.6", "173.3", "231.1", "260", "288.9"}},	// Short GI, 20MHz

	{{"13.5", "27", "40.5", "54", "81", "108", "121.5", "135",
	  "27", "54", "81", "108", "162", "216", "243", "270",
	  "40.5", "81", "121.5", "162", "243", "324", "364.5", "405",
	  "54", "108", "162", "216", "324", "432", "486", "540"},				// Long GI, 40MHz

	 {"15", "30", "45", "60", "90", "120", "135", "150",
	  "30", "60", "90", "120", "180", "240", "270", "300",
	  "45", "90", "135", "180", "270", "360", "405", "450",
	  "60", "120", "180", "240", "360", "480", "540", "600"}	}			// Short GI, 40MHz
};

const u16 VHT_MCS_DATA_RATEStr[4][2][40] =
{	{	{13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
		 26, 52, 78, 104, 156, 208, 234, 260, 312, 312,
		 39, 78, 117, 156, 234, 312, 351, 390, 468, 520,
		 52, 104, 156, 208, 312, 416, 468, 520, 624, 624},					// Long GI, 20MHz

		{14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
		 29, 58, 87, 116, 173, 231, 260, 289, 347, 347,
		 43, 87, 130, 173, 260, 347, 390, 433, 520, 578,
		 58, 116, 173, 231, 347, 462, 520, 578, 693, 693}			},		// Short GI, 20MHz

	{	{27, 54, 81, 108, 162, 216, 243, 270, 324, 360,
		 54, 108, 162, 216, 324, 432, 486, 540, 648, 720,
		 81, 162, 243, 324, 486, 648, 729, 810, 972, 1080,
		 108, 216, 324, 432, 648, 864, 972, 1080, 1296, 1440},				// Long GI, 40MHz

		{30, 60, 90, 120, 180, 240, 270, 300, 360, 400,
		 60, 120, 180, 240, 360, 480, 540, 600, 720, 800,
		 90, 180, 270, 360, 540, 720, 810, 900, 1080, 1200,
		 120, 240, 360, 480, 720, 960, 1080, 1200, 1440, 1600}		},		// Short GI, 40MHz

	{	{59, 117, 176, 234, 351, 468, 527, 585, 702, 780,
		 117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560,
		 176, 351, 527, 702, 1053, 1404, 1580, 1755, 2106, 2340,
		 234, 468, 702, 936, 1404, 1872, 2106, 2340, 2808, 3120},			// Long GI, 80MHz

		{65, 130, 195, 260, 390, 520, 585, 650, 780, 867,
		 130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1734,
		 195, 390, 585, 780, 1170, 1560, 1755, 1950, 2340, 2600,
		 260, 520, 780, 1040, 1560, 2080, 2340, 2600, 3120, 3467}	},		// Short GI, 80MHz

	{	{117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560,
		 234, 468, 702, 936, 1404, 1872, 2160, 2340, 2808, 3120,
		 351, 702, 1053, 1404, 2106, 2808, 3159, 3510, 4212, 4680,
		 468, 936, 1404, 1872, 2808, 3744, 4212, 4680, 5616, 6240},			//Long GI, 160MHz

		{130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1734,
		 260, 520, 780, 1040, 1560, 2080, 2340, 2600, 3120, 3467,
		 390, 780, 1170, 1560, 2340, 3120, 3510, 3900, 4680, 5200,
		 520, 1040, 1560, 2080, 3120, 4160, 4680, 5200, 6240, 6934}	}		//Short GI, 160MHz

};

const u16 HE_MCS_DATA_RATEStr[4][3][48] =
{
	{	{14, 29, 43, 58, 87, 117, 131, 146, 175, 195, 219, 243,
		 29, 58, 87, 117, 175, 234, 263, 292, 351, 390, 438, 487,
		 43, 87, 131, 175, 263, 351, 394, 438, 526, 585, 658, 731,
		 58, 117, 175, 234, 351, 468, 526, 585, 702, 780, 877, 975},			// 0.8, 20MHz

		{16, 32, 48, 65, 97, 130, 146, 162, 195, 216, 243, 270,
		 32, 65, 97, 130, 195, 260, 292, 325, 390, 433, 487, 541,
		 48, 97, 146, 195, 292, 390, 438, 487, 585, 650, 731, 812,
		 65, 130, 195, 260, 390, 520, 585, 650, 780, 866, 975, 1083},			// 1.6, 20MHz

		{17, 34, 52, 69, 103, 138, 155, 172, 207, 229, 258, 287,
		 34, 69, 103, 138, 207, 275, 310, 344, 413, 459, 516, 574,
		 52, 103, 155, 206, 310, 413, 465, 516, 620, 688, 774, 860,
		 69, 138, 207, 275, 413, 551, 619, 688, 826, 918, 1032, 1147}			// 3.2, 20MHz
	},

	{	{29, 58, 87, 117, 175, 234, 263, 292, 351, 390, 438, 487,
		 58, 117, 175, 234, 351, 468, 526, 585, 702, 780, 877, 975,
		 87, 175, 263, 351, 526, 702, 789, 877, 1053, 1170, 1316, 1462,
		 117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, 1755, 1950},		// 3.2, 40MHz

		{32, 65, 97, 130, 195, 260, 292, 325, 390, 433, 487, 541,
		 65, 130, 195, 260, 390, 520, 585, 650, 780, 866, 975, 1083,
		 97, 195, 292, 390, 585, 780, 877, 975, 1170, 1300, 1462, 1625,
		 130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1733, 1970, 2166},		// 1.6, 40MHz

		{34, 69, 103, 138, 207, 275, 310, 344, 413, 459, 516, 574,
		 69, 138, 207, 275, 413, 551, 619, 688, 826, 918, 1032, 1147,
		 103, 206, 310, 413, 620, 826, 929, 1032, 1239, 1376, 1549, 1721,
		 138, 275, 413, 551, 826, 1101, 1239, 1377, 1652, 1835, 2065, 2294}		// 0.8, 40MHz
	},

	{	{61, 122, 183, 245, 367, 490, 551, 612, 735, 816, 918, 1020,
		 122, 245, 367, 490, 735, 980, 1102, 1225, 1470, 1633, 1837, 2041,
		 183, 367, 551, 735, 1102, 1470, 1653, 1837, 2205, 2450, 2756, 3062,
		 245, 490, 735, 980, 1470, 1960, 2205, 2450, 2940, 3266, 3675, 4083},		// 3.2, 80MHz

		{68, 136, 204, 272, 408, 544, 612, 680, 816, 907, 1020, 1134,
		 136, 272, 408, 544, 816, 1088, 1225, 1361, 1633, 1814, 2041, 2268,
		 204, 408, 612, 816, 1225, 1633, 1837, 2041, 2450, 2722, 3062, 3402,
		 272, 544, 816, 1088, 1633, 2177, 2450, 2722, 3266, 3629, 4083, 4537},		// 1.6, 80MHz

		{72, 144, 216, 288, 432, 577, 649, 721, 865, 961, 1081, 1201,
		 144, 288, 432, 577, 865, 1153, 1297, 1441, 1729, 1922, 2162, 2402,
		 216, 432, 649, 865, 1297, 1730, 1946, 2162, 2594, 2882, 3243, 3603,
		 288, 577, 865, 1153, 1729, 2306, 2594, 2882, 3459, 3843, 4324, 4804}		// 0.8, 80MHz
	},

	{	{122, 245, 367, 490, 735, 980, 1102, 1225, 1470, 1633, 1837, 2041,
		 245, 490, 735, 980, 1470, 1960, 2205, 2450, 2940, 3266, 3675, 4083,
		 367, 735, 1102, 1470, 2205, 2940, 3307, 3675, 4410, 4900, 5512, 6125,
		 490, 980, 1470, 1960, 2940, 3920, 4410, 4900, 5880, 6533, 7350, 8166},	// 3.2, 160MHz

		{136, 272, 408, 544, 816, 1088, 1225, 1361, 1633, 1814, 2041, 2268,
		 272, 544, 816, 1088, 1633, 2177, 2450, 2722, 3266, 3629, 4083, 4537,
		 408, 816, 1225, 1633, 2450, 3266, 3675, 4083, 4900, 5444, 6125, 6805,
		 544, 1088, 1633, 2177, 3266, 4355, 4900, 5444, 6533, 7259, 8166, 9074},	// 1.6, 160MHz

		{144, 288, 432, 577, 865, 1211, 1297, 1441, 1729, 1922, 2162, 2402,
		 288, 577, 865, 1153, 1729, 2396, 2594, 2882, 3459, 3843, 4324, 4804,
		 432, 865, 1297, 1729, 2594, 3555, 3891, 4324, 5188, 5765, 6485, 7206,
		 577, 1153, 1729, 2306, 3459, 4689, 5188, 5765, 6918, 7686, 8647, 9608}		// 0.8, 160MHz
	}
};

void dump_phl_tring_status(struct seq_file *m, _adapter *padapter, struct sta_info *psta)
{
	int i = 0;
	u16 tring_len = 0;
	u16 macid = 0;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	if (!rtw_phl_get_one_txring_mode(dvobj->phl))
		macid = psta->phl_sta->macid;

	RTW_PRINT_SEL(m, "PHL_tring_len=");
	for (i = 0; i < MAX_PHL_RING_CAT_NUM; i++) {
		tring_len = rtw_phl_tring_rsc(padapter->dvobj->phl, macid, i);
		RTW_PRINT_SEL(m, "%d ", tring_len);
	}
	RTW_PRINT_SEL(m, "\n");
}

void dump_vendor_to_str(struct seq_file *m, char *str, u8 vendor)
{
	u8 vendor_s[32];

	switch (vendor) {
	case HT_IOT_PEER_BROADCOM:
		_os_snprintf(vendor_s, sizeof(vendor_s), "BROADCOM");
		break;
	case HT_IOT_PEER_APPLE:
		_os_snprintf(vendor_s, sizeof(vendor_s), "APPLE");
		break;
	case HT_IOT_PEER_VERIWAVE:
		_os_snprintf(vendor_s, sizeof(vendor_s), "VERIWAVE");
		break;
	case HT_IOT_PEER_SPIRENT:
		_os_snprintf(vendor_s, sizeof(vendor_s), "SPIRENT");
		break;
	case HT_IOT_PEER_OCTOSCOPE:
		_os_snprintf(vendor_s, sizeof(vendor_s), "OCTOSCOPE");
		break;
	default:
		_os_snprintf(vendor_s, sizeof(vendor_s), "--");
		break;
	}
	RTW_PRINT_SEL(m, "%s: %s\n", str, vendor_s);
}

static const char *rtw_gi_str(u8 gi, u16 rate)
{
	if (rate >= RTW_DATA_RATE_HE_NSS1_MCS0) {
		/* HE data rates */
		switch (gi) {
		case 0:
			return "4x3.2";
		case 1:
			return "4x0.8";
		case 2:
			return "2x1.6";
		case 3:
			return "2x0.8";
		case 4:
			return "1x1.6";
		case 5:
			return "1x0.8";
		default:
			return "err";
		}
	} else {
		/* non-HE data rates */
		if(gi)
			return "short-gi";
		else
			return "long-gi";
	}
}

int proc_get_all_sta_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	struct sta_info *psta;
	struct sta_xmit_priv *pstaxmitpriv;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	int i, j;
	_list	*plist, *phead;
	enum wlan_mode wmode;
	int rate = 0;
	unsigned char tmp_rate[20] = {0};

	RTW_MAP_DUMP_SEL_ALWAYS(m, "sta_dz_bitmap=", pstapriv->sta_dz_bitmap, pstapriv->aid_bmp_len);
	RTW_MAP_DUMP_SEL_ALWAYS(m, "tim_bitmap=", pstapriv->tim_bitmap, pstapriv->aid_bmp_len);

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

			plist = get_next(plist);

			/* if(extra_arg == psta->phl_sta->aid) */
			{
				RTW_PRINT_SEL(m, "==============================\n");
				RTW_PRINT_SEL(m, "sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->phl_sta->mac_addr));
				wmode = pmibpriv->band & psta->phl_sta->wmode;
				RTW_PRINT_SEL(m, "wmode: 0x%x %s%s%s\n", wmode,
					(wmode&WLAN_MD_11AX)?"AX":"", (wmode&WLAN_MD_11AC)?"AC":"", (wmode&WLAN_MD_11N)?"N":"");
				dump_sec_to_str(m, psta->link_time, "link_time");
				RTW_PRINT_SEL(m, "expire timer=%d\n", psta->expire_to*2);
				RTW_PRINT_SEL(m, "ieee8021x_blocked=%d\n", psta->ieee8021x_blocked);
				RTW_PRINT_SEL(m, "rtsen=%d, cts2slef=%d\n", psta->phl_sta->rts_en, psta->phl_sta->cts2self);
				/* ToDo: need API to query hal_sta->ra_info.rate_id */
				/* RTW_PRINT_SEL(m, "state=0x%x, aid=%d, macid=%d, raid=%d\n",
					psta->state, psta->phl_sta->aid, psta->phl_sta->macid, psta->phl_sta->hal_sta->ra_info.rate_id); */
				RTW_PRINT_SEL(m, "state=0x%x, aid=%d, macid=%d\n",
					psta->state, psta->phl_sta->aid, psta->phl_sta->macid);
#ifdef CONFIG_RTS_FULL_BW
				if(psta->vendor_8812)
					RTW_PRINT_SEL(m,"Vendor Realtek 8812\n");
#endif/*CONFIG_RTS_FULL_BW*/
#ifdef CONFIG_80211N_HT
				RTW_PRINT_SEL(m, "qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);
				RTW_PRINT_SEL(m, "bwmode=%d, ch_offset=%d, sgi_20m=%d,sgi_40m=%d\n"
					, psta->phl_sta->chandef.bw, psta->htpriv.ch_offset, psta->htpriv.sgi_20m, psta->htpriv.sgi_40m);
				RTW_PRINT_SEL(m, "ampdu_enable = %d\n", psta->htpriv.ampdu_enable);
				RTW_PRINT_SEL(m, "tx_amsdu_enable = %d\n", psta->htpriv.tx_amsdu_enable);
				RTW_PRINT_SEL(m, "Is_8K_AMSDU = %d\n", (psta->htpriv.ht_cap.cap_info & IEEE80211_HT_CAP_MAX_AMSDU)?1:0);
				RTW_PRINT_SEL(m, "agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);
				RTW_PRINT_SEL(m, "SMPS = %d\n", psta->phl_sta->asoc_cap.sm_ps);
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
				RTW_PRINT_SEL(m, "tx_nss=%d\n", rtw_get_sta_tx_nss(padapter, psta));
				RTW_PRINT_SEL(m, "rx_nss=%d\n", rtw_get_sta_rx_nss(padapter, psta));

				pstaxmitpriv = &psta->sta_xmitpriv;
				RTW_PRINT_SEL(m, "sleep=%s\n", ((psta->state & WIFI_SLEEP_STATE) ? "yes":"no"));
				RTW_PRINT_SEL(m, "nr_sleep=%d\n", psta->sta_stats.nr_sleep);
				RTW_PRINT_SEL(m, "uapsd_bitmap=%04X\n", psta->uapsd_bitmap);
				RTW_PRINT_SEL(m, "ps_trigger=%d\n", !rtw_is_list_empty(&pstaxmitpriv->ps_trigger));
				RTW_PRINT_SEL(m, "ps_trigger_type=%x\n", pstaxmitpriv->ps_trigger_type);
				RTW_PRINT_SEL(m, "tx_pending=%d\n", !rtw_is_list_empty(&pstaxmitpriv->tx_pending));
				RTW_PRINT_SEL(m, "tx_pending_bitmap=%lx\n", pstaxmitpriv->tx_pending_bitmap);
				RTW_PRINT_SEL(m, "txq_total_len=%d\n", ATOMIC_READ(&pstaxmitpriv->txq_total_len));
				RTW_PRINT_SEL(m, "txq[VO],empty=%d,qlen=%d\n",
					skb_queue_empty(&pstaxmitpriv->txq[TXQ_VO].qhead),
					pstaxmitpriv->txq[TXQ_VO].qhead.qlen);
				RTW_PRINT_SEL(m, "txq[VI],empty=%d,qlen=%d\n",
					skb_queue_empty(&pstaxmitpriv->txq[TXQ_VI].qhead),
					pstaxmitpriv->txq[TXQ_VI].qhead.qlen);
				RTW_PRINT_SEL(m, "txq[BE],empty=%d,qlen=%d\n",
					skb_queue_empty(&pstaxmitpriv->txq[TXQ_BE].qhead),
					pstaxmitpriv->txq[TXQ_BE].qhead.qlen);
				RTW_PRINT_SEL(m, "txq[BK],empty=%d,qlen=%d\n",
					skb_queue_empty(&pstaxmitpriv->txq[TXQ_BK].qhead),
					pstaxmitpriv->txq[TXQ_BK].qhead.qlen);
				RTW_PRINT_SEL(m, "txq[MGT],empty=%d,qlen=%d\n",
					_rtw_queue_empty(&pstaxmitpriv->mgt_q),
					pstaxmitpriv->mgt_q.qlen);
#ifdef CONFIG_LMT_TXREQ
				RTW_PRINT_SEL(m, "lmt_pending_txreq=%u\n", psta->lmt_pending_txreq);
				RTW_PRINT_SEL(m, "phl_pending_txreq=%u\n", ATOMIC_READ(&psta->num_pending_txreq));
				RTW_PRINT_SEL(m, "lmt_pending_txreq_ac=%u %u %u %u\n", psta->lmt_pending_txreq_ac[0]
											, psta->lmt_pending_txreq_ac[1]
											, psta->lmt_pending_txreq_ac[2]
											, psta->lmt_pending_txreq_ac[3]);
				RTW_PRINT_SEL(m, "phl_pending_txreq_ac=%u %u %u %u\n", ATOMIC_READ(&psta->num_pending_txreq_ac[0])
											, ATOMIC_READ(&psta->num_pending_txreq_ac[1])
											, ATOMIC_READ(&psta->num_pending_txreq_ac[2])
											, ATOMIC_READ(&psta->num_pending_txreq_ac[3]));
				RTW_PRINT_SEL(m, "lmt_txreq_drop=%u\n", psta->lmt_txreq_drop);
#endif

				RTW_PRINT_SEL(m, "capability=0x%x\n", psta->capability);
				RTW_PRINT_SEL(m, "flags=0x%x\n", psta->flags);
				RTW_PRINT_SEL(m, "isPMF=%d\n", (psta->flags & WLAN_STA_MFP)?1:0);
				RTW_PRINT_SEL(m, "wpa_psk=0x%x\n", psta->wpa_psk);
				RTW_PRINT_SEL(m, "wpa2_group_cipher=0x%x\n", psta->wpa2_group_cipher);
				RTW_PRINT_SEL(m, "wpa2_pairwise_cipher=0x%x\n", psta->wpa2_pairwise_cipher);
				RTW_PRINT_SEL(m, "qos_info=0x%x\n", psta->qos_info);
				RTW_PRINT_SEL(m, "dot118021XPrivacy=0x%x\n", psta->dot118021XPrivacy);
				RTW_PRINT_SEL(m, "AuthAlgrthm=0x%x\n", psta->authalg);
				RTW_PRINT_SEL(m, "curPN=%lld\n", psta->dot11txpn.val);

				sta_rx_reorder_ctl_dump(m, psta);

#ifdef CONFIG_TDLS
				RTW_PRINT_SEL(m, "tdls_sta_state=0x%08x\n", psta->tdls_sta_state);
				RTW_PRINT_SEL(m, "PeerKey_Lifetime=%d\n", psta->TDLS_PeerKey_Lifetime);
#endif /* CONFIG_TDLS */
#ifdef CONFIG_RTW_80211K
				RTW_PRINT_SEL(m, "rm_en_cap="RM_CAP_FMT"\n", RM_CAP_ARG(psta->rm_en_cap));
#endif
				dump_st_ctl(m, &psta->st_ctl);

				if (STA_OP_WFD_MODE(psta))
					RTW_PRINT_SEL(m, "op_wfd_mode:0x%02x\n", STA_OP_WFD_MODE(psta));

				if(psta->bssratelen <= 16)
					RTW_MAP_DUMP_SEL_ALWAYS(m, "bssrateset=", psta->bssrateset, psta->bssratelen);

				/* rx related */
				RTW_PRINT_SEL(m, "rx_data_uc_pkts=%llu\n", sta_rx_data_uc_pkts(psta));
				RTW_PRINT_SEL(m, "rx_data_mc_pkts=%llu\n", psta->sta_stats.rx_data_mc_pkts);
				RTW_PRINT_SEL(m, "rx_data_bc_pkts=%llu\n", psta->sta_stats.rx_data_bc_pkts);
				RTW_PRINT_SEL(m, "rx_uc_bytes=%llu\n", sta_rx_uc_bytes(psta));
				RTW_PRINT_SEL(m, "rx_mc_bytes=%llu\n", psta->sta_stats.rx_mc_bytes);
				RTW_PRINT_SEL(m, "rx_bc_bytes=%llu\n", psta->sta_stats.rx_bc_bytes);
				RTW_PRINT_SEL(m, "rx_data_retry_pkts=%llu\n", psta->sta_stats.rx_data_retry_pkts);
				if (psta->sta_stats.rx_tp_kbits >> 10)
					RTW_PRINT_SEL(m, "rx_tp =%d (Mbps)\n", psta->sta_stats.rx_tp_kbits >> 10);
				else
					RTW_PRINT_SEL(m, "rx_tp =%d (Kbps)\n", psta->sta_stats.rx_tp_kbits);

				/* tx related */
				RTW_PRINT_SEL(m, "tx_data_pkts   =%llu\n", psta->sta_stats.tx_pkts);
				RTW_PRINT_SEL(m, "tx_pkts_cur    =%d\n", psta->sta_stats.tx_data_pkts_cur);/* PPS */
				RTW_PRINT_SEL(m, "tx_core_add_req_data=%llu\n", psta->sta_stats.tx_add_req_data);
				RTW_PRINT_SEL(m, "tx_data_retry_pkts=%llu\n", psta->sta_stats.tx_data_retry_pkts);
				RTW_PRINT_SEL(m, "tx_core_bytes=%llu\n", psta->sta_stats.tx_bytes);
				if (psta->sta_stats.tx_tp_kbits >> 10)
					RTW_PRINT_SEL(m, "tx_tp =%d (Mbps)\n", psta->sta_stats.tx_tp_kbits >> 10);
				else
					RTW_PRINT_SEL(m, "tx_tp =%d (Kbps)\n", psta->sta_stats.tx_tp_kbits);

				RTW_PRINT_SEL(m, "traffic_mode=%d\n", psta->traffic_mode);
				#ifdef CONFIG_BW160M_EXTREME_THROUGHPUT_RX
				RTW_PRINT_SEL(m, "extreme_rx_traffic=%d\n", psta->extreme_rx_traffic);
				#endif
				get_current_rate(psta, 0, &rate, tmp_rate);
				RTW_PRINT_SEL(m, "current_rx_rate=%s %s\n", rtw_data_rate_str(psta->cur_rx_data_rate), tmp_rate);
				RTW_PRINT_SEL(m, "cur_rx_bw=%d\n", psta->cur_rx_bw);
				RTW_PRINT_SEL(m, "cur_rx_gi=%s\n", rtw_gi_str(psta->cur_rx_gi_ltf, psta->cur_rx_data_rate));

				get_current_rate(psta, 1, &rate, tmp_rate);
				RTW_PRINT_SEL(m, "current_tx_rate=%s %s\n", rtw_data_rate_str(psta->cur_tx_data_rate), tmp_rate);
				RTW_PRINT_SEL(m, "cur_tx_bw=%d\n", psta->cur_tx_bw);
				RTW_PRINT_SEL(m, "cur_tx_gi=%s\n", rtw_gi_str(psta->cur_tx_gi_ltf, psta->cur_tx_data_rate));
				/* ToDo: need API to query hal_sta->rssi_stat.rssi */
				RTW_PRINT_SEL(m, "rssi=%d(%d %d %d %d)\n", (psta->phl_sta->hal_sta->rssi_stat.rssi >> 1),
							(psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[0] >> 5),
							(psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[1] >> 5),
							(psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[2] >> 5),
							(psta->phl_sta->hal_sta->rssi_stat.rssi_ma_path[3] >> 5));
				RTW_PRINT_SEL(m, "SNR=%d\n", psta->phl_sta->hal_sta->rssi_stat.snr_ma >> 4);
				#ifdef CONFIG_CORE_TXSC
				#ifdef CONFIG_TXSC_AMSDU
				RTW_PRINT_SEL(m, "amsdu_num=%d\n", psta->txsc_amsdu_num);
				RTW_PRINT_SEL(m, "amsdu_num_max=%d\n", psta->txsc_amsdu_max);
				#endif
				RTW_PRINT_SEL(m, "sec_cam_idx=%d\n", psta->txsc_sec_cam_idx);
				#endif
				dump_phl_tring_status(m, padapter, psta);

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
				for (j = 0 ; j < WLAN_MAX_VENDOR_IE_NUM ; j++) {
					if (psta->vendor_ielen[j]) {
						RTW_PRINT_SEL(m, "VENDOR_IE[%d]\n  ", j);
						RTW_DUMP_SEL(m, psta->vendor_ie[j], psta->vendor_ielen[j]);
					}
				}
#endif
#ifdef CONFIG_CTC_FEATURE
				RTW_PRINT_SEL(m, "[CTC]rssi indicate=%d, wait_cnt=%d\n",
							psta->roaming_indicate, psta->roaming_wait_cnt);
#endif
#ifdef TX_BEAMFORMING
				if (padapter->registrypriv.wifi_mib.txbf) {
					RTW_PRINT_SEL(m, "is_zld_exist_csi: %u \n", psta->phl_sta->is_zld_exist_csi);
					RTW_PRINT_SEL(m, "trig_su_bfm_fb: %u \n", psta->phl_sta->asoc_cap.trig_su_bfm_fb);
					RTW_PRINT_SEL(m, "vht_su_bfmr=%d\n", psta->phl_sta->asoc_cap.vht_su_bfmr);
					RTW_PRINT_SEL(m, "vht_su_bfme=%d\n", psta->phl_sta->asoc_cap.vht_su_bfme);
					RTW_PRINT_SEL(m, "he_su_bfmr=%d\n", psta->phl_sta->asoc_cap.he_su_bfmr);
					RTW_PRINT_SEL(m, "he_su_bfme=%d\n", psta->phl_sta->asoc_cap.he_su_bfme);
					if (padapter->registrypriv.wifi_mib.txbf_mu) {
						RTW_PRINT_SEL(m, "trig_mu_bfm_fb: %u \n", psta->phl_sta->asoc_cap.trig_mu_bfm_fb);
						RTW_PRINT_SEL(m, "vht_mu_bfmr=%d\n", psta->phl_sta->asoc_cap.vht_mu_bfmr);
						RTW_PRINT_SEL(m, "vht_mu_bfme=%d\n", psta->phl_sta->asoc_cap.vht_mu_bfme);
						RTW_PRINT_SEL(m, "he_mu_bfmr=%d\n", psta->phl_sta->asoc_cap.he_mu_bfmr);
						RTW_PRINT_SEL(m, "he_mu_bfme=%d\n", psta->phl_sta->asoc_cap.he_mu_bfme);
					}
				}
#endif
				RTW_PRINT_SEL(m, "STBC Rx(HT,VHT,HE)=(0x%02X,0x%02X,0x%02X)\n"
											,psta->phl_sta->asoc_cap.stbc_ht_rx
											, psta->phl_sta->asoc_cap.stbc_vht_rx
											, psta->phl_sta->asoc_cap.stbc_he_rx);
				RTW_PRINT_SEL(m, "LDPC Rx(HT,VHT,HE)=(0x%02X,0x%02X,0x%02X)\n"
											, psta->phl_sta->asoc_cap.ht_ldpc
											, psta->phl_sta->asoc_cap.vht_ldpc
											, psta->phl_sta->asoc_cap.he_ldpc);
#ifdef CONFIG_TX_MCAST2UNI
				RTW_PRINT_SEL(m, "ipmc_num=%d\n", psta->ipmc_num);
				for (j = 0; j < psta->ipmc_num; j++)
					RTW_PRINT_SEL(m, "mcmac=" MAC_FMT "\n", MAC_ARG(psta->ipmc[j].mcmac));
#endif
				dump_vendor_to_str(m, "chip vendor", psta->vendor);

				RTW_PRINT_SEL(m, "==============================\n");
			}

		}

	}

	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	return 0;
}

#ifdef CONFIG_ONE_TXQ
int proc_get_txq_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	_queue *sta_queue;
	struct sta_info *psta;
	struct sta_xmit_priv *pstaxmitpriv;
	_list *phead, *plist;

	/* dump ps_trigger_sta_queue */
	sta_queue = &dvobj->ps_trigger_sta_queue;
	phead = get_list_head(sta_queue);

	_rtw_spinlock_bh(&sta_queue->lock);

	RTW_PRINT_SEL(m, "ps_trigger_sta_queue,empty=%d,qlen=%d\n",
		_rtw_queue_empty(sta_queue), sta_queue->qlen);
	if (sta_queue->qlen)
		RTW_PRINT_SEL(m, "   iface macid type total_qlen\n");

	plist = get_next(phead);
	while (plist != phead) {
		psta = LIST_CONTAINOR(plist, struct sta_info, sta_xmitpriv.ps_trigger);
		plist = get_next(plist);

		pstaxmitpriv = &psta->sta_xmitpriv;
		RTW_PRINT_SEL(m, "   %5d %5d %4d %d\n",
			psta->padapter->iface_id, psta->phl_sta->macid,
			pstaxmitpriv->ps_trigger_type, ATOMIC_READ(&pstaxmitpriv->txq_total_len));
	}

	_rtw_spinunlock_bh(&sta_queue->lock);

	/* dump tx_pending_sta_queue */
	sta_queue = &dvobj->tx_pending_sta_queue;
	phead = get_list_head(sta_queue);

	_rtw_spinlock_bh(&sta_queue->lock);

	RTW_PRINT_SEL(m, "tx_pending_sta_queue,empty=%d,qlen=%d\n",
		_rtw_queue_empty(sta_queue), sta_queue->qlen);
	if (sta_queue->qlen)
		RTW_PRINT_SEL(m, "   iface macid bw gi rate bitmap total_qlen\n");

	plist = get_next(phead);
	while (plist != phead) {
		psta = LIST_CONTAINOR(plist, struct sta_info, sta_xmitpriv.tx_pending);
		plist = get_next(plist);

		pstaxmitpriv = &psta->sta_xmitpriv;
		RTW_PRINT_SEL(m, "   %5d %5d %2d %2d %4x %6ld %d\n",
			psta->padapter->iface_id, psta->phl_sta->macid,
			psta->cur_tx_bw, psta->cur_tx_gi_ltf, psta->cur_tx_data_rate,
			pstaxmitpriv->tx_pending_bitmap, ATOMIC_READ(&pstaxmitpriv->txq_total_len));
	}

	_rtw_spinunlock_bh(&sta_queue->lock);

	RTW_PRINT_SEL(m, "txq_total_len=%d\n", ATOMIC_READ(&dvobj->txq_total_len));
	RTW_PRINT_SEL(m, "txq_full_drop=%u\n", dvobj->txq_full_drop);
	RTW_PRINT_SEL(m, "txq_timeout_seq=%u\n", dvobj->txq_timeout_seq);
	RTW_PRINT_SEL(m, "txq_pkt_trigger=%u\n", dvobj->txq_pkt_trigger);
	RTW_PRINT_SEL(m, "txq_timeout_trigger=%u\n", dvobj->txq_timeout_trigger);
	RTW_PRINT_SEL(m, "txq_serv_timeout=%u\n", dvobj->txq_serv_timeout);
	RTW_PRINT_SEL(m, "free_txreq_queue,qlen=%u\n", padapter->pfree_txreq_queue->qlen);
	RTW_PRINT_SEL(m, "txq_timeout_avg=%d\n", dvobj->txq_timeout_avg);
	return 0;
}
#endif

int get_current_rate(struct sta_info *psta, u8 is_direction_tx, int *rate, unsigned char *rate_str)
{
	int  band_with = 0, is_short_gi = 0, idx = 0;
	u8   current_giltf = 0, giltf_level = 0;
	u16 current_rate;
	u8 current_bw;

	if(psta == NULL || rate == NULL)
		return -1;

	if (is_direction_tx) {
		current_bw = psta->cur_tx_bw;
		current_rate = psta->cur_tx_data_rate;
		current_giltf = psta->cur_tx_gi_ltf;
	} else {
		current_bw = psta->cur_rx_bw;
		current_rate = psta->cur_rx_data_rate;
		current_giltf = psta->cur_rx_gi_ltf;
	}

	if(current_bw == CHANNEL_WIDTH_160)
		band_with = 3;
	else if(current_bw == CHANNEL_WIDTH_80)
		band_with = 2;
	else if(current_bw == CHANNEL_WIDTH_40)
		band_with = 1;
	else
		band_with = 0;

#ifdef CONFIG_80211N_HT
	if(psta->htpriv.sgi_20m || psta->htpriv.sgi_40m)
		is_short_gi = current_giltf?1:0;
#endif
#ifdef CONFIG_80211AC_VHT
	if(psta->vhtpriv.sgi_80m)
		is_short_gi = current_giltf?1:0;
#endif
#ifdef CONFIG_80211AX_HE
	switch (current_giltf) {
		case 0: /* 4x3.2 */
			giltf_level = 0;
			break;
		case 2: /* 2x1.6 */
		case 4: /* 1x1.6 */
			giltf_level = 1;
			break;
		case 1: /* 4x0.8 */
		case 3: /* 2x0.8 */
		case 5: /* 1x0.8 */
			giltf_level = 2;
			break;
		default:
			giltf_level = 0;
			break;
	}
#endif

	if(current_rate >= RTW_DATA_RATE_HE_NSS1_MCS0 && current_rate <= RTW_DATA_RATE_HE_NSS4_MCS11)
	{
		idx = (current_rate - RTW_DATA_RATE_HE_NSS1_MCS0) / 16 * 12 + (current_rate - RTW_DATA_RATE_HE_NSS1_MCS0) % 16;
		*rate = HE_MCS_DATA_RATEStr[band_with][giltf_level][(idx < 48) ? idx : 0];
		*rate = *rate >> 1;
		snprintf(rate_str, 10, "%d", *rate);
	}
	else if(current_rate >= RTW_DATA_RATE_VHT_NSS1_MCS0 && current_rate <= RTW_DATA_RATE_VHT_NSS4_MCS9)
	{
		idx = ((int)((current_rate - RTW_DATA_RATE_VHT_NSS1_MCS0) / 16)) * 10 + ((current_rate - RTW_DATA_RATE_VHT_NSS1_MCS0) % 16);
		*rate = VHT_MCS_DATA_RATEStr[band_with][is_short_gi][(idx < 40) ? idx : 0];
		*rate = *rate >> 1;
		snprintf(rate_str, 10, "%d", *rate);
	}
	else if (current_rate >= RTW_DATA_RATE_MCS0 && current_rate <= RTW_DATA_RATE_MCS31)
	{
		char *r = (char *)MCS_DATA_RATEStr[band_with <= 1 ? band_with : 1][is_short_gi][(current_rate - RTW_DATA_RATE_MCS0)];
		strncpy(rate_str, r, 10);
		*rate = rtw_atoi(r);
	}
	else if(current_rate <= RTW_DATA_RATE_OFDM54)//legacy rate
	{
		char *r = (char *)LEGACY_DATA_RATE[current_rate];
		strncpy(rate_str, r, 10);
		*rate = rtw_atoi(r);
	}
	else
		printk("[WARNING][%s:%d]current_rate=%d is wrong,plz check\n", __FUNCTION__, __LINE__, current_rate);

	return 0;
}

/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
void _write_sta_info(struct seq_file *m, struct sta_info *psta, int staNum, int cli_conn_success)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	int rate = 0;
	unsigned char tmp_rate[20] = {0};
	u64 curr_tx_bytes_1s = 0, curr_rx_bytes_1s = 0;
	u8 *mac;

	RTW_PRINT_SEL(m, "%d: stat_info...\n", staNum);
	RTW_PRINT_SEL(m, "    ieee8021x_ctrlport: 1\n"); //TODO

	if (cli_conn_success)
		mac = cur_network->network.MacAddress;
	else
		mac = psta->phl_sta->mac_addr;

	RTW_PRINT_SEL(m, "    hwaddr: " "%02x%02x%02x%02x%02x%02x" "\n", MAC_ARG(mac));
	RTW_PRINT_SEL(m, "    aid: %d\n", psta->phl_sta->aid);
	RTW_PRINT_SEL(m, "    MACID: %d\n", psta->phl_sta->macid);
	RTW_PRINT_SEL(m, "    link_time: %d\n", psta->link_time); //client TODO
	RTW_PRINT_SEL(m, "    last RX time: %d\n",
	              rtw_get_passing_time_ms(psta->sta_stats.last_rx_time));
	RTW_PRINT_SEL(m, "    state: %08X\n", psta->state);
	RTW_PRINT_SEL(m, "    tx_bytes: %llu\n", psta->sta_stats.tx_bytes);
	RTW_PRINT_SEL(m, "    rx_bytes: %llu\n", psta->sta_stats.rx_bytes);
	curr_tx_bytes_1s = 0;
	curr_rx_bytes_1s = 0;
	curr_tx_bytes_1s = ((u64)psta->sta_stats.tx_tp_kbits << 10) / 8;
	curr_rx_bytes_1s = ((u64)psta->sta_stats.rx_tp_kbits << 10) / 8;

	RTW_PRINT_SEL(m, "    tx_bytes_1s: %llu\n", curr_tx_bytes_1s);
	RTW_PRINT_SEL(m, "    rx_bytes_1s: %llu\n", curr_rx_bytes_1s);

	get_current_rate(psta, 1, &rate, tmp_rate);
	RTW_PRINT_SEL(m, "    current_tx_rate: %s %s\n", rtw_data_rate_str(psta->cur_tx_data_rate), tmp_rate);

	get_current_rate(psta, 0, &rate, tmp_rate);
	RTW_PRINT_SEL(m, "    current_rx_rate: %s %s\n", rtw_data_rate_str(psta->cur_rx_data_rate), tmp_rate);

#ifdef SBWC
	RTW_PRINT_SEL(m, "    tx_limit: %d\n", psta->SBWC_tx_limit);
	RTW_PRINT_SEL(m, "    rx_limit: %d\n", psta->SBWC_rx_limit);
	RTW_PRINT_SEL(m, "    tx_limit_byte: %d\n", psta->SBWC_tx_limit_byte);
	RTW_PRINT_SEL(m, "    rx_limit_byte: %d\n", psta->SBWC_rx_limit_byte);
	RTW_PRINT_SEL(m, "    tx_count: %d\n", psta->SBWC_tx_count);
	RTW_PRINT_SEL(m, "    rx_count: %d\n", psta->SBWC_rx_count);
#endif
	RTW_PRINT_SEL(m, "    rssi: %d\n", rtw_phl_get_sta_rssi(psta->phl_sta));
	RTW_PRINT_SEL(m, "    snr: %d %s\n", psta->phl_sta->hal_sta->rssi_stat.snr_ma >> 4, "(0 0)");//TODO
#ifdef CONFIG_IEEE80211V
	RTW_PRINT_SEL(m, "    BSS Trans Support: %d\n", rtw_wnm_get_ext_cap_btm(psta->ext_capab_ie_data));
	RTW_PRINT_SEL(m, "    BSS Trans Rsp Status Code: %d\n", psta->bss_trans_status_code);
#endif
#ifdef CONFIG_RTW_80211K
	RTW_PRINT_SEL(m, "    RRM Support:"RM_CAP_FMT"\n", RM_CAP_ARG(psta->rm_en_cap));
#endif
#ifdef CONFIG_RECORD_CLIENT_HOST
	{
	char client_ip[24];
	if(psta->client_host_ip[0]){
		snprintf(client_ip,24,"%d:%d:%d:%d",psta->client_host_ip[0],psta->client_host_ip[1],psta->client_host_ip[2],psta->client_host_ip[3]);
		RTW_PRINT_SEL(m, "    Client host ip: %s\n", client_ip);
		}
	}
#endif
#ifdef CONFIG_RTW_A4_STA
	RTW_PRINT_SEL(m, "    A4 STA: %s\n", (psta->flags & WLAN_STA_A4) ? "Y" : "N");
#endif
#if defined(CONFIG_RTW_MULTI_AP)
	RTW_PRINT_SEL(m, "    multiap profile: %d\n", psta->multiap_profile);
#endif
#ifdef RTW_STA_BWC
	RTW_PRINT_SEL(m, "	  sta_bwc_limit: %u Kbps\n", psta->sta_bwc_tx_limit);
#endif
	RTW_PRINT_SEL(m, "==============================\n");
}

#if defined(WFO_SYNC_STA_STATUS) && defined(PLATFORM_ECOS)
#define WFO_SYNC_STA_ST 1
extern void __wfo_sync_sta_st(struct sta_info *psta, struct wfo_sta_st_s *sta_st, int staNum);
#else /* !(WFO_SYNC_STA_STATUS & PLATFORM_ECOS) */
#define WFO_SYNC_STA_ST 0
#endif /* WFO_SYNC_STA_STATUS & PLATFORM_ECOS */

int __proc_get_sta_info(struct seq_file *m, void *v)
{
	struct net_device *dev;
	struct sta_info *psta;
	struct sta_info *pfirsta;
	_list	*plist, *phead;
	_adapter *padapter;
	struct sta_priv *pstapriv;
	struct mlme_priv *pmlmepriv;
	struct wlan_network *cur_network;
	int i = 0, staNum = 0;
	unsigned char self_mac[18], bmc_mac[18], mac[18];
	struct mlme_ext_priv *pmlmeext;
	struct mlme_ext_info *pmlmeinfo;

	int do_proc = 1;

#if (WFO_SYNC_STA_ST==1)
	struct wfo_sta_ptr_s *p = NULL;
	int (*wfo_sync_sta_st_fp)(struct sta_info *psta, struct wfo_sta_st_s *sta_st, int staNum) = NULL;

	if (((void*)m) == (void *)(__wfo_sync_sta_st)) {
		p = (struct wfo_sta_ptr_s *)(v);
		dev = p->dev;
		wfo_sync_sta_st_fp = m;
		do_proc = 0;
	} else {
		dev = m->private;
	}
#else /* !WFO_SYNC_STA_ST */
	dev = m->private;
#endif /* WFO_SYNC_STA_ST */

	padapter = (_adapter *)rtw_netdev_priv(dev);
	pstapriv = &padapter->stapriv;
	pmlmepriv = &(padapter->mlmepriv);
	cur_network = &(pmlmepriv->cur_network);
	pmlmeext = &(padapter->mlmeextpriv);
	pmlmeinfo = &(pmlmeext->mlmext_info);

	if(rtw_is_adapter_up(padapter))
	{
		if(MLME_IS_STA(padapter) && !(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS))
			pfirsta = rtw_get_stainfo(pstapriv, padapter->phl_role->mac_addr);
		else
			pfirsta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);

		if(pfirsta == NULL)
		{
			if (do_proc)RTW_PRINT_SEL(m, "-- STA info table -- (active: %d)\n", 0);
			return 0;
		}

		memset(self_mac, 0, sizeof(self_mac));
		memset(bmc_mac, 0, sizeof(bmc_mac));

		if(pfirsta->phl_sta)
			snprintf(self_mac, sizeof(self_mac), MAC_FMT, MAC_ARG(pfirsta->phl_sta->mac_addr));
		else
		{
			if (do_proc)RTW_PRINT_SEL(m, "-- STA info table -- (active: %d)\n", 0);
			return 0;
		}

		snprintf(bmc_mac, sizeof(bmc_mac), "%s", "ff:ff:ff:ff:ff:ff");

		if(MLME_IS_STA(padapter))
		{
			if(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
			{
				if(strncmp(self_mac, bmc_mac, sizeof(self_mac)))
				{
				#if (WFO_SYNC_STA_ST==1)
					if (wfo_sync_sta_st_fp&&p&&p->st)wfo_sync_sta_st_fp(pfirsta, p->st, staNum);
				#endif /* WFO_SYNC_STA_ST */
					if (do_proc)RTW_PRINT_SEL(m, "-- STA info table -- (active: %d)\n", 1);
					staNum++;
					if (do_proc)_write_sta_info(m, pfirsta, staNum, 1);
				}
				else
					if (do_proc)RTW_PRINT_SEL(m, "-- STA info table -- (active: %d)\n", 0);
			}
			else
				if (do_proc)RTW_PRINT_SEL(m, "-- STA info table -- (active: %d)\n", 0);
		}
		else
		{
			if (do_proc)RTW_PRINT_SEL(m, "-- STA info table -- (active: %d)\n", pstapriv->asoc_sta_count-1); //exclude AP itself for ap mode
			_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
			for (i = 0; i < NUM_STA; i++) {
				phead = &(pstapriv->sta_hash[i]);
				plist = get_next(phead);
				while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
					psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
					plist = get_next(plist);

					memset(mac, 0, sizeof(mac));
					if(psta->phl_sta)
					{
						snprintf(mac, sizeof(mac), MAC_FMT, MAC_ARG(psta->phl_sta->mac_addr));
						if(strncmp(mac, self_mac, sizeof(mac)) && strncmp(mac, bmc_mac, sizeof(mac)))
						{
						#if (WFO_SYNC_STA_ST==1)
							if (wfo_sync_sta_st_fp&&p&&p->st)wfo_sync_sta_st_fp(psta, p->st, staNum);
						#endif /* WFO_SYNC_STA_ST */
							staNum++;
							if (do_proc)_write_sta_info(m, psta, staNum, 0);
						}
					}
				}
			}
			_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);
		}
	}
	else
	{
		RTW_INFO("%s(%d) The interface is not running.\n", __FUNCTION__, __LINE__);
	}

	return 0;
}

#if (WFO_SYNC_STA_ST==1)
void __wfo_sync_sta_st(struct sta_info *psta, struct wfo_sta_st_s *sta_st, int staNum)
{
	struct wfo_sta_st_s *psta_st = (sta_st + staNum);

	memcpy(psta_st->mac_addr, psta->phl_sta->mac_addr, MAC_ALEN);
	psta_st->rx_bytes = psta->sta_stats.rx_bytes;
	psta_st->rx_pkts = psta->sta_stats.rx_data_pkts +
		psta->sta_stats.rx_ctrl_pkts + psta->sta_stats.rx_mgnt_pkts;
	psta_st->tx_bytes = psta->sta_stats.tx_bytes;
	psta_st->tx_pkts = psta->sta_stats.tx_pkts;

#if 0
	printk("sta(%d)(0x%x):\n", staNum, psta_st);
	printk("\t" "mac: "MAC_FMT"\n", MAC_ARG(psta_st->mac_addr));
	printk("\t" "tx_bytes: %20llu, tx_pkts: %20llu\n", psta_st->tx_bytes, psta_st->tx_pkts);
	printk("\t" "rx_bytes: %20llu, rx_pkts: %20llu\n", psta_st->rx_bytes, psta_st->rx_pkts);
#endif
}

void wfo_sync_sta_st(struct net_device *dev, struct wfo_sta_st_s *sta_st)
{
	struct wfo_sta_ptr_s p;

	p.dev = dev;
	p.st  = sta_st;

	memset(sta_st, 0, sizeof(struct wfo_sta_st_s)*WFO_STA_NUM);
	__proc_get_sta_info((void *)(__wfo_sync_sta_st), (void *)(&p));
}
#endif /* WFO_SYNC_STA_ST */

int proc_get_sta_info(struct seq_file *m, void *v)
{
	return __proc_get_sta_info(m, v);
}

/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
int proc_get_mib_staconfig(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	char wl_mode[16];

	psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
	if (psta) {
		wireless_mode_to_str(pmibpriv->band, wl_mode);
		RTW_PRINT_SEL(m, "    dot11SSIDtoScan:(Len %d) %s\n", cur_network->network.Ssid.SsidLength, cur_network->network.Ssid.Ssid);
		RTW_PRINT_SEL(m, "    dot11Bssid: " "%02x%02x%02x%02x%02x%02x" "\n", MAC_ARG(psta->phl_sta->mac_addr));
	}
	return 0;
}

int proc_get_temperature(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	void *phl = GET_HAL_INFO(adapter_to_dvobj(padapter));
	u32 th_idx = 0;
	u32 wifi_t_offset = 62000;
	u32 wifi_t_coefficient = 3429;
	u32 wifi_t_factor = 1000;
	u32 temp = 0;

	if (padapter->netif_up == _FALSE) {
		RTW_PRINT_SEL(m, "temperature: invalid \n");
		return 0;
	}

	rtw_phl_write_rfreg(phl, 0, 0x42, BIT(19), 0x1);
	rtw_phl_write_rfreg(phl, 0, 0x42, BIT(19), 0x0);
	rtw_phl_write_rfreg(phl, 0, 0x42, BIT(19), 0x1);
	udelay(200);
	th_idx = rtw_phl_read_rfreg(phl, 0, 0x42, 0x0007e);

	if (WIFI_ROLE_IS_ON_5G(padapter)) {
		RTW_PRINT_SEL(m, "5G thermal: %u \n", th_idx);
		if ((padapter->tp_total_tx >> 10) >= 1500)
			th_idx -= 5;
		else if ((padapter->tp_total_tx >> 10) >= 900)
			th_idx -= 4;
		else if ((padapter->tp_total_tx >> 10) >= 400)
			th_idx -= 2;
		else if ((padapter->tp_total_tx >> 10) >= 200)
			th_idx -= 1;

		if ((wifi_t_coefficient*th_idx) > wifi_t_offset)
			temp = (wifi_t_coefficient*th_idx - wifi_t_offset) / wifi_t_factor;
		RTW_PRINT_SEL(m, "5G temperature: %d \n", temp);

		wifi_t_offset = 13217;
		wifi_t_coefficient = 2196;
		if ((wifi_t_coefficient*th_idx) > wifi_t_offset)
                        temp = (wifi_t_coefficient*th_idx - wifi_t_offset) / wifi_t_factor;
		RTW_PRINT_SEL(m, "CPU temperature: %d \n", temp);
	} else if (WIFI_ROLE_IS_ON_24G(padapter)) {
		wifi_t_offset = 23252;
		wifi_t_coefficient = 2553;

		if ((wifi_t_coefficient*th_idx) > wifi_t_offset)
                        temp = (wifi_t_coefficient*th_idx - wifi_t_offset) / wifi_t_factor;

		RTW_PRINT_SEL(m, "2G thermal: %u \n", th_idx);
                RTW_PRINT_SEL(m, "2G temperature: %d \n", temp);
	}

	return 0;
}

int proc_get_mac_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	void *phl = GET_HAL_INFO(adapter_to_dvobj(padapter));
	char buf[50];

	rtw_phl_get_halmac_ver(buf, sizeof(buf));
	RTW_PRINT_SEL(m, "HALMAC version %s\n", buf);

	rtw_phl_get_fw_ver(phl, buf, sizeof(buf));
	RTW_PRINT_SEL(m, "FW version %s\n", buf);

	if (adapter_to_dvobj(padapter)->phl_com->dev_cap.fw_tx_mode){
		RTW_PRINT_SEL(m, "FW Tx mode: SW\n");
		RTW_PRINT_SEL(m, "FW force to one tx cmd: %d \n", adapter_to_dvobj(padapter)->force_tx_cmd_num);
	}else
		RTW_PRINT_SEL(m, "FW Tx mode: HW\n");

	return 0;
}

int proc_get_rf_para_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = padapter->dvobj;
	void *phl = GET_HAL_INFO(adapter_to_dvobj(padapter));

	u8 rfe_type = 50;
	u8 *fem_name;
	u32 bb_para_ver, rf_para_ver;

	fem_name = rtw_malloc(64);
	if(!fem_name){
		RTW_ERR("alloc fem_name fail!\n");
		goto exit;
	}

	rfe_type = dvobj->phl_com->dev_cap.rfe_type;
	bb_para_ver = rtw_phl_read_bbreg(phl, 0xf4, 0xFFFFFFFF);
	rf_para_ver = rtw_phl_get_rf_radio_ver(phl);
	rtw_get_fem_name(dvobj, fem_name, 63);

	if (rfe_type > 50)
		RTW_PRINT_SEL(m, "RFE type: %d (%s)\n", rfe_type, fem_name);
	else
		RTW_PRINT_SEL(m, "RFE type: %d \n", rfe_type);

	RTW_PRINT_SEL(m, "BB parameter version: %x\n", bb_para_ver);
	RTW_PRINT_SEL(m, "RF parameter version: %x\n", rf_para_ver);

	/******* Get calibration data in shadow map ********/
	/*XCAP*/
	RTW_PRINT_SEL(m,"xcap: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2b9));


	/* Thermal */
	RTW_PRINT_SEL(m , "Thermal A: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2d0));
	RTW_PRINT_SEL(m , "Thermal B: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2d1));

	/* TSSI */
	RTW_PRINT_SEL(m , "2G CCK TSSI A: %x %x %x %x %x %x\n",
		      rtw_nvm_get_by_offset(dvobj, 0x210),
		      rtw_nvm_get_by_offset(dvobj, 0x211),
		      rtw_nvm_get_by_offset(dvobj, 0x212),
		      rtw_nvm_get_by_offset(dvobj, 0x213),
		      rtw_nvm_get_by_offset(dvobj, 0x214),
		      rtw_nvm_get_by_offset(dvobj, 0x215));

	RTW_PRINT_SEL(m , "2G CCK TSSI B: %x %x %x %x %x %x\n",
	              rtw_nvm_get_by_offset(dvobj, 0x23a),
	              rtw_nvm_get_by_offset(dvobj, 0x23b),
	              rtw_nvm_get_by_offset(dvobj, 0x23c),
	              rtw_nvm_get_by_offset(dvobj, 0x23d),
	              rtw_nvm_get_by_offset(dvobj, 0x23e),
	              rtw_nvm_get_by_offset(dvobj, 0x23f));

	//RTW_PRINT_SEL(m , "2G CCK TSSI C: %d\n", rtw_nvm_get_by_offset(dvobj, 0x264));
	//RTW_PRINT_SEL(m , "2G CCK TSSI D: %d\n", rtw_nvm_get_by_offset(dvobj, 0x28e));

	RTW_PRINT_SEL(m , "2G BW40 1S TSSI A: %x %x %x %x %x\n",
	              rtw_nvm_get_by_offset(dvobj, 0x216),
	              rtw_nvm_get_by_offset(dvobj, 0x217),
	              rtw_nvm_get_by_offset(dvobj, 0x218),
	              rtw_nvm_get_by_offset(dvobj, 0x219),
	              rtw_nvm_get_by_offset(dvobj, 0x21a));

	RTW_PRINT_SEL(m , "2G BW40 1S TSSI B: %x %x %x %x %x\n",
	              rtw_nvm_get_by_offset(dvobj, 0x240),
	              rtw_nvm_get_by_offset(dvobj, 0x241),
	              rtw_nvm_get_by_offset(dvobj, 0x242),
	              rtw_nvm_get_by_offset(dvobj, 0x243),
	              rtw_nvm_get_by_offset(dvobj, 0x244));

	//RTW_PRINT_SEL(m , "2G BW40 1S TSSI C: %d\n", rtw_nvm_get_by_offset(dvobj, 0x26a));
	//RTW_PRINT_SEL(m , "2G BW40 1S TSSI D: %d\n", rtw_nvm_get_by_offset(dvobj, 0x294));

	RTW_PRINT_SEL(m , "5G BW40 1S TSSI A: %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
	              rtw_nvm_get_by_offset(dvobj, 0x222),
	              rtw_nvm_get_by_offset(dvobj, 0x223),
	              rtw_nvm_get_by_offset(dvobj, 0x224),
	              rtw_nvm_get_by_offset(dvobj, 0x225),
	              rtw_nvm_get_by_offset(dvobj, 0x226),
	              rtw_nvm_get_by_offset(dvobj, 0x227),
	              rtw_nvm_get_by_offset(dvobj, 0x228),
	              rtw_nvm_get_by_offset(dvobj, 0x229),
	              rtw_nvm_get_by_offset(dvobj, 0x22a),
	              rtw_nvm_get_by_offset(dvobj, 0x22b),
	              rtw_nvm_get_by_offset(dvobj, 0x22c),
	              rtw_nvm_get_by_offset(dvobj, 0x22d),
	              rtw_nvm_get_by_offset(dvobj, 0x22e),
	              rtw_nvm_get_by_offset(dvobj, 0x22f));

	RTW_PRINT_SEL(m , "5G BW40 1S TSSI B: %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
		      rtw_nvm_get_by_offset(dvobj, 0x24c),
	              rtw_nvm_get_by_offset(dvobj, 0x24d),
	              rtw_nvm_get_by_offset(dvobj, 0x24e),
	              rtw_nvm_get_by_offset(dvobj, 0x24f),
	              rtw_nvm_get_by_offset(dvobj, 0x250),
	              rtw_nvm_get_by_offset(dvobj, 0x251),
	              rtw_nvm_get_by_offset(dvobj, 0x252),
	              rtw_nvm_get_by_offset(dvobj, 0x253),
	              rtw_nvm_get_by_offset(dvobj, 0x254),
	              rtw_nvm_get_by_offset(dvobj, 0x255),
	              rtw_nvm_get_by_offset(dvobj, 0x256),
	              rtw_nvm_get_by_offset(dvobj, 0x257),
	              rtw_nvm_get_by_offset(dvobj, 0x258),
	              rtw_nvm_get_by_offset(dvobj, 0x259));

	//RTW_PRINT_SEL(m , "5G BW40 1S TSSI C: %d\n", rtw_nvm_get_by_offset(dvobj, 0x276));
	//RTW_PRINT_SEL(m , "5G BW40 1S TSSI D: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2a0));

	/* RX Gain */
	RTW_PRINT_SEL(m , "2G RX Gain CCK: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2d6));
	RTW_PRINT_SEL(m , "2G RX Gain OFDM: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2d4));
	RTW_PRINT_SEL(m , "5G RX Gain Low: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2d8));
	RTW_PRINT_SEL(m , "5G RX Gain Mid: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2da));
	RTW_PRINT_SEL(m , "5G RX Gain High: %d\n", rtw_nvm_get_by_offset(dvobj, 0x2dc));

	/* PHY PARA FILE Buffer */
	RTW_PRINT_SEL(m , "bfree_para_info: %d, bfreed_para [0]: %d, [1] %d\n",
		dvobj->phl_com->dev_sw_cap.bfree_para_info,
		dvobj->phl_com->phy_sw_cap[0].bfreed_para,
		dvobj->phl_com->phy_sw_cap[1].bfreed_para);

exit:
	if(fem_name){
		rtw_mfree(fem_name, 64);
	}

	return 0;
}

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
int proc_get_ap_probereq_info(struct seq_file *m, void *v)
{
	int i, tmp_idx = 0;
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u32 vendor_ie_mask = 0;
	_timeval tv;
	u64 tm_tamp=0;

	RTW_PRINT_SEL(m, "AP Probereq get Info...\n");
	for(i=0; i<WLAN_MAX_VENDOR_IE_NUM; i++)
	{
		vendor_ie_mask = padapter->mlmepriv.vendor_ie_mask[i];
		if(vendor_ie_mask & WIFI_PROBERESP_VENDOR_IE_BIT)
		{
			if(padapter->mlmepriv.vendor_timestamp_is_set[i] && padapter->mlmepriv.vendor_mac_is_set[i] && padapter->mlmepriv.vendor_ielen[i])
			{
				RTW_PRINT_SEL(m, "	entry: %d\n", tmp_idx);
				RTW_PRINT_SEL(m, "		mac: %02X%02X%02X%02X%02X%02X\n", MAC_ARG(padapter->mlmepriv.vendor_mac[i]));
				rtw_do_gettimeofday(&tv);
				tm_tamp = (u64)tv.tv_sec - padapter->mlmepriv.vendor_timestamp[i];
				RTW_PRINT_SEL(m, "		Last seen: %llu seconds ago\n", tm_tamp);
				tmp_idx++;
			}
		}
	}

	return 0;
}

int proc_get_vsie_info(struct seq_file *m, void *v)
{
	int i, j, n = 0;
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char pstring[500]={0}, *data_ptr;

	RTW_PRINT_SEL(m, "USERIE get Info...\n");
	for(i=0; i<WLAN_MAX_VENDOR_IE_NUM; i++)
	{
		if(padapter->mlmepriv.vendor_ie_mask[i] && padapter->mlmepriv.vendor_ielen[i])
		{
			RTW_PRINT_SEL(m, "Index %3d\n", i);
			memset(pstring, 0, sizeof(pstring));
			n = 0;
			if (padapter->mlmepriv.vendor_ie_mask[i] & WIFI_BEACON_VENDOR_IE_BIT)
				n += snprintf(pstring + n, sizeof(pstring) - n, "[Beacon]");
			if (padapter->mlmepriv.vendor_ie_mask[i] & WIFI_PROBEREQ_VENDOR_IE_BIT)
				n += snprintf(pstring + n, sizeof(pstring) - n, "[Probe Req]");
			if (padapter->mlmepriv.vendor_ie_mask[i] & WIFI_PROBERESP_VENDOR_IE_BIT)
				n += snprintf(pstring + n, sizeof(pstring) - n, "[Probe Resp]");
			if (padapter->mlmepriv.vendor_ie_mask[i] & WIFI_ASSOCREQ_VENDOR_IE_BIT)
				n += snprintf(pstring + n, sizeof(pstring) - n, "[Assoc Req]");
			if (padapter->mlmepriv.vendor_ie_mask[i] & WIFI_ASSOCRESP_VENDOR_IE_BIT)
				n += snprintf(pstring + n, sizeof(pstring) - n, "[Assoc Resp]");
			if (padapter->mlmepriv.vendor_ie_mask[i] & WIFI_PROBEREQ_RX_VENDOR_IE_BIT)
				snprintf(pstring + n, sizeof(pstring) - n, "[Probe Req Rx]");

			RTW_PRINT_SEL(m, "	%s\n", pstring);

			memset(pstring, 0, sizeof(pstring));

			data_ptr = pstring;
			for (j = 0 ; j < padapter->mlmepriv.vendor_ielen[i]; j++)
			{
				snprintf(data_ptr , 3, "%02x" , padapter->mlmepriv.vendor_ie[i][j]);
				data_ptr += 2;
			}

			RTW_PRINT_SEL(m, "	USER IE: %s\n",pstring);
		}
	}

	return 0;
}
#endif

#ifdef MONITOR_UNASSOC_STA
/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
int proc_get_monitor_sta_info(struct seq_file *m, void *v)
{
	int i = 0;
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;
	unsigned long sec;

	if(pmibpriv->monitor_sta_enabled)
	{
		RTW_PRINT_SEL(m, "	Monitor sta info...\n");
		for(i=0; i<NUM_MONITOR; i++)
		{
			if(pmlmeext->monitor_sta_info.monitor_sta_ent[i].valid == 1)
			{
				RTW_PRINT_SEL(m, "	MacAddr[%d]: %02x%02x%02x%02x%02x%02x\n", i, MAC_ARG(pmlmeext->monitor_sta_info.monitor_sta_ent[i].mac));
				RTW_PRINT_SEL(m, "	Rssi: %d dBm\n", pmlmeext->monitor_sta_info.monitor_sta_ent[i].rssi-100);
				RTW_PRINT_SEL(m, "	IsRouter: %s\n", (pmlmeext->monitor_sta_info.monitor_sta_ent[i].isAP==1)?"Y":"N");
				if(jiffies/HZ >= pmlmeext->monitor_sta_info.monitor_sta_ent[i].sec)
					sec = jiffies/HZ - pmlmeext->monitor_sta_info.monitor_sta_ent[i].sec;
				else
					sec = jiffies/HZ + ~(unsigned long)0/HZ - pmlmeext->monitor_sta_info.monitor_sta_ent[i].sec;
				RTW_PRINT_SEL(m, "	Last seen: %lu seconds ago\n", sec);
				RTW_PRINT_SEL(m, "==============================\n");
			}
		}
	}
	else
	{
		RTW_PRINT_SEL(m, "	Monitor sta disabled\n");
	}

	return 0;
}
#endif

#ifdef RTW_BLOCK_STA_CONNECT
int proc_get_block_sta_info(struct seq_file *m, void *v)
{
	int i = 0;
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	unsigned long sec;

	RTW_PRINT_SEL(m, "Block sta info...\n");
	for(i=0; i<MACID_NUM_SW_LIMIT; i++)
	{
		if(pmlmeext->blockStaExt.block_sta[i].isvalid== 1)
		{
			RTW_PRINT_SEL(m, "	MacAddr[%d]: %02x%02x%02x%02x%02x%02x\n", i, MAC_ARG(pmlmeext->blockStaExt.block_sta[i].mac));
			RTW_PRINT_SEL(m, "	block_time: %d\n", pmlmeext->blockStaExt.block_sta[i].block_time);
			if(jiffies/HZ >= pmlmeext->blockStaExt.block_sta[i].age)
				sec = jiffies/HZ - pmlmeext->blockStaExt.block_sta[i].age;
			else
				sec = jiffies/HZ + ~(unsigned long)0/HZ - pmlmeext->blockStaExt.block_sta[i].age;
			RTW_PRINT_SEL(m, "	Last seen: %lu seconds ago\n", sec);
			RTW_PRINT_SEL(m, "==============================\n");
		}
	}

	return 0;
}
#endif

u16 rtw_get_ch_utilization(_adapter *padapter)
{
	u16 nhm_tx_cnt;
	u16 nhm_cca_cnt;
	u16 nhm_idle_cnt;
	u16 ch_utilization;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_env_report *env_rpt = &(dvobj->env_rpt);

	nhm_tx_cnt = env_rpt->nhm_tx_cnt;
	nhm_cca_cnt = env_rpt->nhm_cca_cnt;
	nhm_idle_cnt = env_rpt->nhm_idle_cnt;

	if(!nhm_idle_cnt && !nhm_cca_cnt && !nhm_idle_cnt)
		return 0;

	ch_utilization = (nhm_cca_cnt * 255) / (nhm_tx_cnt + nhm_cca_cnt + nhm_idle_cnt);

	return ch_utilization;
}

u8 rtw_get_channel_busy_time(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_env_report *env_rpt = &(dvobj->env_rpt);

	return env_rpt->nhm_cca_ratio;
}

/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
int proc_get_stats(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct dvobj_priv *pdevobjpriv = (padapter->dvobj);
	struct rf_ctl_t *rfctl = dvobj_to_rfctl(pdevobjpriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	struct phl_info_t *phl_info = GET_HAL_INFO(dvobj);

/*
	u8 u_ch, u_bw, u_offset;
	u32 non_ocp_ms, cac_ms;
*/
	u32 tx_fail = 0;
	u32 tx_fail_mgmt = 0;
	u32 tx_ok = 0;
	u32 tx_ok_mgmt = 0;

	if(rtw_is_adapter_up(padapter))
	{
		psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
/*
		u_ch = pmlmeext->cur_channel;
		u_bw = pmlmeext->cur_bwmode;
		u_offset = pmlmeext->cur_ch_offset;
		rtw_get_ch_waiting_ms(rfctl, u_ch, u_bw, u_offset, &non_ocp_ms, &cac_ms);
*/
		if(psta)
		{
			RTW_PRINT_SEL(m, "Statistics...\n");
			RTW_PRINT_SEL(m, "    hwaddr: " "%02x%02x%02x%02x%02x%02x" "\n", MAC_ARG(psta->phl_sta->mac_addr));
			dump_sec_to_str(m, padapter->up_time, "    up_time");
			RTW_PRINT_SEL(m, "    tx_bytes: %llu\n", pxmitpriv->tx_bytes);
			RTW_PRINT_SEL(m, "    tx_packets: %llu\n", pxmitpriv->tx_pkts);
			RTW_PRINT_SEL(m, "    tx_avarage: %d (Mbps)\n", (psta->sta_stats.tx_tp_kbits >> 10));
			rtw_phl_get_hw_cnt_tx_ok(dvobj->phl, &tx_ok, &tx_ok_mgmt);
			RTW_PRINT_SEL(m, "    tx_ok: %d\n", tx_ok);
			rtw_phl_get_hw_cnt_tx_fail(dvobj->phl, &tx_fail, &tx_fail_mgmt);
			RTW_PRINT_SEL(m, "    tx_fails: %d\n", tx_fail);
			RTW_PRINT_SEL(m, "    tx_retrys: %llu\n", pxmitpriv->tx_data_retry);
			RTW_PRINT_SEL(m, "    tx_drops: %llu\n", pxmitpriv->tx_drop);


			RTW_PRINT_SEL(m, "    rx_bytes: %llu\n", precvpriv->rx_bytes);
			RTW_PRINT_SEL(m, "    rx_packets: %llu\n", precvpriv->rx_pkts);
			RTW_PRINT_SEL(m, "    rx_avarage: %d\n", (psta->sta_stats.rx_tp_kbits >> 10));
			RTW_PRINT_SEL(m, "    rx_errors: %llu\n",  precvpriv->rx_errors);
			RTW_PRINT_SEL(m, "    rx_data_drops: %llu\n", precvpriv->dbg_rx_drop_count);
#ifdef CONFIG_RTW_CROSSBAND_REPEATER_SUPPORT
			RTW_PRINT_SEL(m, "    cb_pathswitch_pkts: %llu\n", precvpriv->cb_pathswitch_pkts);
#endif

			RTW_PRINT_SEL(m, "    ch_utilization: %d\n" , rtw_get_ch_utilization(padapter));
			RTW_PRINT_SEL(m, "    chbusytime: %d\n" , rtw_get_channel_busy_time(padapter));
			RTW_PRINT_SEL(m, "    sizeof(skb_shared_info): %zu\n" , sizeof(struct skb_shared_info));
			RTW_PRINT_SEL(m, "    run_cmd_en: %u\n", padapter->run_cmd_en);

#ifdef RTW_STA_BWC
			RTW_PRINT_SEL(m, "    txduty: %u\n", padapter->txduty);
			RTW_PRINT_SEL(m, "	  txduty_level: %d\n", padapter->txduty_level);
			RTW_PRINT_SEL(m, "	  sta_bwc_total_tp: %u\n", padapter->sta_bwc_total_tp);
			RTW_PRINT_SEL(m, "    tx_tp_base: %u (Kbps)\n", padapter->tx_tp_base);
			RTW_PRINT_SEL(m, "    tx_tp_limit: %u (Kbps)\n", padapter->tx_tp_limit);
			RTW_PRINT_SEL(m, "    got_limit_tp: %d\n", padapter->got_limit_tp);
#endif
		}
	}
	else
	{
		RTW_INFO("%s(%d) The interface is not running.\n", __FUNCTION__, __LINE__);
	}
	return 0;
}

/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
int proc_get_mib_operation(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &padapter->stapriv;

	psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);

	if(psta)
	{
		RTW_PRINT_SEL(m, "Dot11OperationEntry...\n");
		RTW_PRINT_SEL(m, "    hwaddr: " "%02x%02x%02x%02x%02x%02x" "\n", MAC_ARG(psta->phl_sta->mac_addr));
		RTW_PRINT_SEL(m, "    opmode: 0x%x\n", pmlmepriv->fw_state);
		RTW_PRINT_SEL(m, "==============================\n");
	}
	return 0;
}

/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
int proc_get_mib_wsc(struct seq_file *m, void *v)
{
	RTW_PRINT_SEL(m, "==============================\n");
	RTW_PRINT_SEL(m, "mib_wsc:\n");//TODO
	RTW_PRINT_SEL(m, "==============================\n");
	return 0;
}

/*****************************************************************
* NOTE:
* The user space will parse the content of the following file,
* please DO NOT change the format of the output!
******************************************************************/
int proc_get_led(struct seq_file *m, void *v)
{
    struct net_device *dev = m->private;
    _adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
    struct led_priv *ledpriv = adapter_to_led(adapter);

    RTW_PRINT_SEL(m, "mode = %d, manual_ctrl = %d, manual_opt = %d\n",
		ledpriv->mode, ledpriv->manual_ctrl, ledpriv->manual_opt);
    return 0;
}

ssize_t proc_set_led(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
    struct net_device *dev = data;
    _adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
    struct led_priv *ledpriv = adapter_to_led(adapter);
    struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

    char tmp[32];
    u8  onoff=0;
    LED_CTL_MODE mode;

    if (count < 1)
		return -EFAULT;

    if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
    }

    if (buffer && !copy_from_user(tmp, buffer, count)) {

        int num = sscanf(tmp, "%hhu", &onoff);

        if (num != 1) {
            printk("Incorrect parameter format\n");
            return -EFAULT;
        }

		if ((onoff == 0) || (onoff == 1)) {
			ledpriv->manual_opt = !!onoff ? RTW_LED_OPT_LOW : RTW_LED_OPT_HIGH;
			ledpriv->manual_ctrl = _TRUE;
			rtw_led_control(adapter, LED_CTL_MANUAL);
		} else if (onoff == 2) {
			mode = (adapter->netif_up) ? LED_CTL_UP_IDLE : LED_CTL_DOWN;
			ledpriv->manual_ctrl = _FALSE;
			rtw_led_control(adapter, mode);
		}
    }
    return count;
}

ssize_t proc_set_led_interval(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
    struct net_device *dev = data;
    _adapter *adapter = (_adapter *)rtw_netdev_priv(dev);
    struct led_priv *ledpriv = adapter_to_led(adapter);
    struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
    u32 interval=100;
    char tmp[32];

    if (count < 1)
            return -EFAULT;

    if (count > sizeof(tmp)) {
            rtw_warn_on(1);
            return -EFAULT;
    }

    if (buffer && !copy_from_user(tmp, buffer, count)) {
        int num = sscanf(tmp, "%u", &interval);
        if (num != 1) {
            printk("Incorrect parameter format\n");
            return -EFAULT;
        }
    }

    return count;
}

#ifdef CONFIG_RTW_MANUAL_EDCA
int proc_get_edca(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int i = 0;

	struct _ParaRecord *param = NULL;

	RTW_PRINT_SEL(m, "  EDCA for AP...\n");
	for(i = 0; i < 4; i++){
		param = &(padapter->registrypriv.wifi_mib.ap_manual_edca[i]);
		switch (i) {
		case VO:
			RTW_PRINT_SEL(m, "  [VO]\n");
			break;
		case VI:
			RTW_PRINT_SEL(m, "  [VI]\n");
			break;
		case BE:
			RTW_PRINT_SEL(m, "  [BE]\n");
			break;
		case BK:
			RTW_PRINT_SEL(m, "  [BK]\n");
			break;
		}
		RTW_PRINT_SEL(m, "    aifsn: %d\n", param->aifsn);
		RTW_PRINT_SEL(m, "    ecw_max: %d\n", param->ecw_max);
		RTW_PRINT_SEL(m, "    ecw_min: %d\n", param->ecw_min);
		RTW_PRINT_SEL(m, "    txop_limit: %d\n", param->txop_limit);
	}

	return 0;
}
#endif

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
ssize_t proc_set_wapi_gcm_sm4_test(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];
	u8 wapi_test_index = 0;
	if (count < 1)
		return -EFAULT;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%hhu", &wapi_test_index);
		if(num!=1){
			printk("Incorrect parameter format\n");
			return -EFAULT;
		}
		rtw_wapi_gcm_sm4_test(wapi_test_index);
	}
	return count;
}
#endif

#ifdef SBWC
int proc_sbwc(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int i = 0;

	unsigned int count = padapter->registrypriv.wifi_mib.sbwcEntry.count;
	struct _SBWC_ENTRY *entry = NULL;

	RTW_PRINT_SEL(m, "  miscSBWC...\n");
	for(i = 0; i < count; i++){
		entry = &(padapter->registrypriv.wifi_mib.sbwcEntry.entry[i]);
		RTW_PRINT_SEL(m, "    hwaddr: %02x%02x%02x%02x%02x%02x" "\n", MAC_ARG(entry->mac));
		RTW_PRINT_SEL(m, "    tx_lmt: %d kbps\n", entry->tx_lmt);
		RTW_PRINT_SEL(m, "    rx_lmt: %d kbps\n", entry->rx_lmt);
	}

	return 0;
}
#endif

#ifdef GBWC
int proc_gbwc(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int i = 0;

	unsigned int count = padapter->registrypriv.wifi_mib.gbwcEntry.count;
	struct _GBWC_ENTRY *entry = NULL;

	RTW_PRINT_SEL(m, "  miscGBWC...\n");
	RTW_PRINT_SEL(m, "    GBWCMode: %d\n", padapter->registrypriv.wifi_mib.gbwcmode);
	RTW_PRINT_SEL(m, "    GBWCThrd_tx: %d kbps\n", padapter->registrypriv.wifi_mib.gbwcthrd_tx);
	RTW_PRINT_SEL(m, "    GBWCThrd_rx: %d kbps\n", padapter->registrypriv.wifi_mib.gbwcthrd_rx);
	RTW_PRINT_SEL(m, "    Address List:");
	for(i = 0; i < count; i++){
		entry = &(padapter->registrypriv.wifi_mib.gbwcEntry.entry[i]);
		RTW_PRINT_SEL(m, "    hwaddr: %02x%02x%02x%02x%02x%02x" "\n", MAC_ARG(entry->mac));
	}
	RTW_PRINT_SEL(m, "    GBWC_timer_alive: %u\n", padapter->GBWC_timer_alive);

	return 0;
}
#endif

#ifdef CONFIG_CORE_TXSC
int proc_get_txsc_ctl(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_PRINT_SEL(m, "  txsc_ctl...\n");
	RTW_PRINT_SEL(m, "    txsc_thres: %d pps\n", (padapter->txsc_cache_thres * HZ) / padapter->txsc_time_duration);
	RTW_PRINT_SEL(m, "    txsc_time_duration: %d\n", padapter->txsc_time_duration);

	return 0;
}

ssize_t proc_set_txsc_ctl(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	int txsc_thres = 0, txsc_duration_level = 0, num = 0;

	if (count < 2)
		goto help;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		goto help;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num = sscanf(tmp, "%d %d", &txsc_duration_level, &txsc_thres);
		if(num >= 2)
		{
			switch (txsc_duration_level) {
				case 0:
					padapter->txsc_time_duration = 1;
					padapter->txsc_cache_thres = (txsc_thres * padapter->txsc_time_duration) / HZ;
					break;
				case 1:
					padapter->txsc_time_duration = 2;
					padapter->txsc_cache_thres = (txsc_thres * padapter->txsc_time_duration) / HZ;
					break;
				case 2:
					padapter->txsc_time_duration = 4;
					padapter->txsc_cache_thres = (txsc_thres * padapter->txsc_time_duration) / HZ;
					break;
				case 3:
					padapter->txsc_time_duration = 8;
					padapter->txsc_cache_thres = (txsc_thres * padapter->txsc_time_duration) / HZ;
					break;
				default:
					goto help;
			}

			RTW_INFO("set %s\n", "txsc threshold and time duration");
		}
		else
			goto help;
	}
	return count;

help:
	printk("usage: \n");
	printk("echo [txsc_duration_level] [txsc_thres]> /proc/wlanxxx/txsc_ctl (txsc_duration_level: 0~3)\n");

	return count;
}
#endif

#ifdef CONFIG_RTW_OPCLASS_CHANNEL_SCAN
void dump_opclass_channel_scan_list(_adapter *padapter, struct seq_file *m)
{
	u32 i, j;
	int index = 0;

	/*if (!padapter->opclass_sync_result || padapter->opclass_proc_done)
		return;*/

	if (!padapter->opclass_sync_result)
		return;

	_RTW_PRINT_SEL(m, "%5s  %-17s  %7s  %-9s  %-6s  %-5s  %7s  %8s  %32s\n", "index", "bssid", "channel", "Strength", "Bdwith", "Noise", "utility", "staCount", "ssid");

	for (i = 0; i < padapter->opclass_scan_result->channel_nr; i++) {
		if (padapter->opclass_scan_result->channels[i].channel_utilization < 1)
		{
			padapter->opclass_scan_result->channels[i].channel_utilization = 1;
		}

		for (j = 0; j < padapter->opclass_scan_result->channels[i].neighbor_nr; j++)
		{
			_RTW_PRINT_SEL(m, "%5d  "MAC_FMT"  %7d  %-9d  %-6d  %-5d  %7d  %8d  %32s\n",
						++index,
						MAC_ARG(padapter->opclass_scan_result->channels[i].neighbors[j].bssid),
						padapter->opclass_scan_result->channels[i].channel,
						(int)padapter->opclass_scan_result->channels[i].neighbors[j].signal_strength,
						padapter->opclass_scan_result->channels[i].neighbors[j].channel_band_width,
						padapter->opclass_scan_result->channels[i].noise,
						padapter->opclass_scan_result->channels[i].neighbors[j].channel_utilization,
						padapter->opclass_scan_result->channels[i].neighbors[j].station_count,
						padapter->opclass_scan_result->channels[i].neighbors[j].ssid
					);
		}
	}

	padapter->opclass_channel_proc_done = 1;

	return;
}

int proc_get_opclass_channel_scan_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	if (!padapter->opclass_sync_result)
		goto exit;

	dump_opclass_channel_scan_list(padapter, m);

	padapter->opclass_channel_proc_done = 1;

exit:
	return 0;
}
#endif

void _parse_dscp_str_to_table(_adapter *padapter, char* str)
{
	char dscp_tbl[100] = {0};
	int num, start, i = 0;

	num = sscanf(str, "%d,%s", &start, dscp_tbl);
	if ((num != 2) || (start >= DSCP_TABLE_SIZE) || (start < 0))
		return;

	for (i = 0; i < DSCP_TABLE_SIZE - start; i++) {
		if (dscp_tbl[i] == 0)
			return;
		if (((dscp_tbl[i] - '0') <= 7) && ((dscp_tbl[i] - '0') >= 0))
			padapter->dscp_mapping_table[i + start] = dscp_tbl[i] - '0';
	}
}

ssize_t proc_set_dscp_mapping(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[128] = {0};
	char cmd[16] = {0};
	char value[100] = {0};
	int start;
	int num;

	if (count < 1)
		return -EINVAL;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num = sscanf(tmp, "%s %s", cmd, value);
		if (num < 1)
			return -EFAULT;

		if (!strcmp(cmd, "enable") && num == 2) {
			padapter->dscp_mapping_enable = rtw_atoi(value);
			RTW_INFO("[%s]dscp_mapping_enable = %d\n", __func__, padapter->dscp_mapping_enable);
		} else if (!strcmp(cmd, "table") && num == 2) {
			/* in format echo table [start],[dscp_array] in decimal
			   maximum length of dscp_array should be (64 - start)
			   ex: echo table 10,333444 > /proc/... => set dscp mapping 10 to 12: 3, 13 to 15: 4
			*/
			_parse_dscp_str_to_table(padapter, value);
		} else if (!strcmp(cmd, "reset")) {
			rtw_init_dscp_table(padapter);
			RTW_INFO("[%s]Reset dscp mapping table!\n", __func__);
		} else {
			return -EFAULT;
		}
	}

	return count;
}

int proc_get_dscp_mapping(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	int i, j;
	int n = 0;
	char pstring[200]={0};

	RTW_PRINT_SEL(m, "DSCP Custom Mapping Table Enable: %d\n", padapter->dscp_mapping_enable);
	for (i = 0; i < 8; i++) {
		n += snprintf(pstring + n, sizeof(pstring) - n, "%02d-%02d	", i*8, i*8+7);
		for(j = 0; j < 8; j++)
			n += snprintf(pstring + n, sizeof(pstring) - n, "%d", padapter->dscp_mapping_table[i*8+j]);
		n += snprintf(pstring + n, sizeof(pstring) - n, "\n");
	}

	RTW_PRINT_SEL(m, "%s\n", pstring);

	return 0;
}
#ifdef CONFIG_ONE_TXQ
ssize_t proc_set_atm(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[128] = {0};
	char cmd[16] = {0};
	char value1[20] = {0};
	char value2[10] = {0};
	u8 sta_addr[ETH_ALEN];
	struct sta_info *psta = NULL;
	int empty_index = -1;
	int num;
	int i;
	u32 atm_time;

	if (count < 1)
		return -EINVAL;

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num = sscanf(tmp, "%s %s %s", cmd, value1, value2);
		if (num < 1)
			return -EFAULT;

		if (!strcmp(cmd, "if") && num == 2) {
			atm_time = rtw_atoi(value1);
			if (atm_time > 100)
				return -EFAULT;
			padapter->atm_ifsettime = rtw_atoi(value1);
			RTW_INFO("[%s]atm_iftime = %d\n", __func__, padapter->atm_ifsettime);
		} else if (!strcmp(cmd, "sta") && num == 3) {
			atm_time = rtw_atoi(value2);
			if (atm_time > 100)
				return -EFAULT;
			sscanf(value1, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
				&sta_addr[0], &sta_addr[1], &sta_addr[2],
				&sta_addr[3], &sta_addr[4], &sta_addr[5]);
			for (i = 0; i < NUM_STA; i++) {
				if (padapter->atm_sta_info[i].atm_statime == 0 && empty_index == -1)
					empty_index = i;
				if (_rtw_memcmp(padapter->atm_sta_info[i].hwaddr, sta_addr, ETH_ALEN) == _TRUE) {
					empty_index = -1;
					padapter->atm_sta_info[i].atm_statime = atm_time;
					RTW_INFO("[%s]update ATMTIME = %d in index %d\n", __func__, atm_time, i);
					break;
				}
			}

			if (i == NUM_STA && empty_index != -1) {
				padapter->atm_sta_info[empty_index].atm_statime = atm_time;
				_rtw_memcpy(padapter->atm_sta_info[empty_index].hwaddr, sta_addr, ETH_ALEN);
				RTW_INFO("[%s]set ATMTIME = %d in empty index %d\n", __func__, atm_time, empty_index);
			} else if (i == NUM_STA && empty_index == -1) {
				RTW_INFO("[%s]set ATMTIME table is full!\n", __func__);
			}
		} else {
			return -EFAULT;
		}
	}

	return count;
}

int proc_get_atm(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_list	*phead, *plist;
	int i;
	struct sta_info *sta;
	u8 sta_is_in_atm_list;

	if (dvobj->tx_mode != 2)
		return 0;

	RTW_PRINT_SEL(m, "ATM Interface time: %d(%d)\n", padapter->atm_ifsettime, padapter->atm_ifusetime);
	RTW_PRINT_SEL(m, "ATM STA set time:\n");
	for (i = 0; i < NUM_STA; i++) {
		if (padapter->atm_sta_info[i].atm_statime) {
			RTW_PRINT_SEL(m, "  [%02d]" MAC_FMT " time:%d\n", i, MAC_ARG(padapter->atm_sta_info[i].hwaddr), padapter->atm_sta_info[i].atm_statime);
		}
	}

	RTW_PRINT_SEL(m, "ATM STA Stats:\n");
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	/* check asoc_queue */
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		sta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		if (sta) {
			sta_is_in_atm_list = 0;
			for (i = 0; i < NUM_STA; i++) {
				if (padapter->atm_sta_info[i].atm_statime) {
					if (_rtw_memcmp(padapter->atm_sta_info[i].hwaddr,
						sta->phl_sta->mac_addr, ETH_ALEN) == _TRUE) {
						sta_is_in_atm_list = 1;
						break;
					}
				}
			}
			RTW_PRINT_SEL(m, "  STA " MAC_FMT ": time %d %%\n", MAC_ARG(sta->phl_sta->mac_addr), sta->sta_xmitpriv.ts_ratio);
			RTW_PRINT_SEL(m, "    In ATM list               = %u\n", sta_is_in_atm_list);
			RTW_PRINT_SEL(m, "    rate                      = %u Mbps\n", sta->sta_xmitpriv.tx_rate_mbps);
			RTW_PRINT_SEL(m, "    retry_ratio               = %u %%\n", sta->sta_stats.tx_retry_ratio);
			RTW_PRINT_SEL(m, "    rate(consider retry)      = %u\n", sta->sta_xmitpriv.tx_rate_mbps_retry);
			RTW_PRINT_SEL(m, "    ts_used/ts_limit(us)      = %u/%u\n", sta->sta_xmitpriv.ts_used,
							sta->sta_xmitpriv.ts_limit*(dvobj->txq_deq_factor<<dvobj->txq_serv_group_exp)/100);
			RTW_PRINT_SEL(m, "    txq_total_len/txq_limit   = %d/%u\n", ATOMIC_READ(&sta->sta_xmitpriv.txq_total_len),
							sta->sta_xmitpriv.txq_limit);
			RTW_PRINT_SEL(m, "    txq_full_drop             = %u\n", sta->sta_xmitpriv.txq_full_drop);
			RTW_PRINT_SEL(m, "    txreq_used                = %d\n", ATOMIC_READ(&sta->sta_xmitpriv.txreq_used));
			RTW_PRINT_SEL(m, "    txreq_ts_used             = %d\n", ATOMIC_READ(&sta->sta_xmitpriv.txreq_ts_used));
			RTW_PRINT_SEL(m, "    txreq_ts_used_total       = %llu\n", sta->sta_xmitpriv.txreq_ts_used_total);
			RTW_PRINT_SEL(m, "    txreq_ts_limit_exceed_cnt = %u\n", sta->sta_xmitpriv.txreq_ts_limit_exceed_cnt);
			RTW_PRINT_SEL(m, "    txq_pkt_trigger           = %u\n", sta->sta_xmitpriv.txq_pkt_trigger);
			RTW_PRINT_SEL(m, "    txq_timeout_trigger       = %u\n", sta->sta_xmitpriv.txq_timeout_trigger);
			RTW_PRINT_SEL(m, "    txq_avg_dequeue           = %u\n", sta->sta_xmitpriv.txq_avg_dequeue);
			RTW_PRINT_SEL(m, "    txq_service_update        = %u\n", sta->sta_xmitpriv.txq_service_update);
		}
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	return 0;
}
#endif /* CONFIG_ONE_TXQ */

#endif
