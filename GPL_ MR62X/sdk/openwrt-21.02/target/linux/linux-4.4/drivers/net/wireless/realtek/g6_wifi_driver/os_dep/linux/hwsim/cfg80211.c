/*
 * Copyright(c) 2018 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#include "rtw_hwsim.h"

extern struct cfg80211_ops rtw_cfg80211_ops;

#define CHAN2G(_channel, _freq, _flags) {			\
		.band = NL80211_BAND_2GHZ,	\
		.center_freq = (_freq),		\
		.hw_value = (_channel),		\
		.flags			= (_flags),			\
		.max_antenna_gain	= 0,				\
		.max_power		= 30,				\
	}

#define CHAN5G(_channel, _flags) {				\
		.band = NL80211_BAND_5GHZ,	\
		.center_freq		= 5000 + (5 * (_channel)),	\
		.hw_value = (_channel),		\
		.flags			= (_flags),			\
		.max_antenna_gain	= 0,				\
		.max_power		= 30,				\
	}


static struct ieee80211_channel hwsim_channels_2ghz[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

static struct ieee80211_channel hwsim_channels_5ghz[] = {
	CHAN5G(36, 0),	CHAN5G(40, 0),	CHAN5G(44, 0),	CHAN5G(48, 0),

	CHAN5G(52, 0),	CHAN5G(56, 0),	CHAN5G(60, 0),	CHAN5G(64, 0),

	CHAN5G(100, 0),	CHAN5G(104, 0),	CHAN5G(108, 0),	CHAN5G(112, 0),
	CHAN5G(116, 0),	CHAN5G(120, 0),	CHAN5G(124, 0),	CHAN5G(128, 0),
	CHAN5G(132, 0),	CHAN5G(136, 0),	CHAN5G(140, 0),	CHAN5G(144, 0),

	CHAN5G(149, 0),	CHAN5G(153, 0),	CHAN5G(157, 0),	CHAN5G(161, 0),
	CHAN5G(165, 0),	CHAN5G(169, 0),	CHAN5G(173, 0),	CHAN5G(177, 0),
};

static struct ieee80211_rate hwsim_rates[] = {
	{ .bitrate = 10 },
	{ .bitrate = 20, .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 55, .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 110, .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 60 },
	{ .bitrate = 90 },
	{ .bitrate = 120 },
	{ .bitrate = 180 },
	{ .bitrate = 240 },
	{ .bitrate = 360 },
	{ .bitrate = 480 },
	{ .bitrate = 540 }
};

static struct ieee80211_txrx_stypes
rtw_hwsim_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4) |
		BIT(IEEE80211_STYPE_ASSOC_RESP >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_DEVICE] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
};

static const u32 rtw_hwsim_cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
	WLAN_CIPHER_SUITE_WEP104
	/* WLAN_CIPHER_SUITE_AES_CMAC, */
	/* WLAN_CIPHER_SUITE_GCMP, */
	/* WLAN_CIPHER_SUITE_GCMP_256, */
	/* WLAN_CIPHER_SUITE_CCMP_256, */
	/* WLAN_CIPHER_SUITE_BIP_GMAC_128, */
	/* WLAN_CIPHER_SUITE_BIP_GMAC_256, */
	/* WLAN_CIPHER_SUITE_BIP_CMAC_256, */
	/* WLAN_CIPHER_SUITE_SMS4 */
};

static int rtw_hwsim_init_supported_band(
	       struct ieee80211_supported_band *sband,
	       enum nl80211_band band)
{
	if (band == (enum nl80211_band)NL80211_BAND_2GHZ) {
		sband->channels = hwsim_channels_2ghz;
		sband->n_channels = ARRAY_SIZE(hwsim_channels_2ghz);
		sband->bitrates = hwsim_rates;
		sband->n_bitrates = ARRAY_SIZE(hwsim_rates);
	} else if (band == (enum nl80211_band)NL80211_BAND_5GHZ) {
		sband->channels = hwsim_channels_5ghz;
		sband->n_channels = ARRAY_SIZE(hwsim_channels_5ghz);
		sband->bitrates = hwsim_rates + 4;
		sband->n_bitrates = ARRAY_SIZE(hwsim_rates) - 4;

		sband->vht_cap.vht_supported = true;
		sband->vht_cap.cap =
			IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
			IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ |
			IEEE80211_VHT_CAP_RXLDPC |
			IEEE80211_VHT_CAP_SHORT_GI_80 |
			IEEE80211_VHT_CAP_SHORT_GI_160 |
			IEEE80211_VHT_CAP_TXSTBC |
			IEEE80211_VHT_CAP_RXSTBC_1 |
			IEEE80211_VHT_CAP_RXSTBC_2 |
			IEEE80211_VHT_CAP_RXSTBC_3 |
			IEEE80211_VHT_CAP_RXSTBC_4 |
			IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
		sband->vht_cap.vht_mcs.rx_mcs_map =
			cpu_to_le16(IEEE80211_VHT_MCS_SUPPORT_0_9 << 0 |
				    IEEE80211_VHT_MCS_SUPPORT_0_9 << 2 |
				    IEEE80211_VHT_MCS_SUPPORT_0_9 << 4 |
				    IEEE80211_VHT_MCS_SUPPORT_0_9 << 6 |
				    IEEE80211_VHT_MCS_SUPPORT_0_9 << 8 |
				    IEEE80211_VHT_MCS_SUPPORT_0_9 << 10 |
				    IEEE80211_VHT_MCS_SUPPORT_0_9 << 12 |
				    IEEE80211_VHT_MCS_SUPPORT_0_9 << 14);
		sband->vht_cap.vht_mcs.tx_mcs_map =
			sband->vht_cap.vht_mcs.rx_mcs_map;
	} else {
		return -1;
	}

	sband->ht_cap.ht_supported = true;
	sband->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
		IEEE80211_HT_CAP_GRN_FLD |
		IEEE80211_HT_CAP_SGI_20 |
		IEEE80211_HT_CAP_SGI_40 |
		IEEE80211_HT_CAP_DSSSCCK40;
	sband->ht_cap.ampdu_factor = 0x3;
	sband->ht_cap.ampdu_density = 0x6;
	memset(&sband->ht_cap.mcs, 0,
	       sizeof(sband->ht_cap.mcs));
	sband->ht_cap.mcs.rx_mask[0] = 0xff;
	sband->ht_cap.mcs.rx_mask[1] = 0xff;
	sband->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;

	return 0;
}

static void rtw_hwsim_wiphy_init(struct wiphy *wiphy,
				 struct rtw_hwsim_data *data)
{
	enum nl80211_band band;

	/* set mac address */
	wiphy->n_addresses = 1;
	wiphy->addresses = data->addresses;

	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_AP) |
		BIT(NL80211_IFTYPE_P2P_CLIENT) |
		BIT(NL80211_IFTYPE_P2P_GO) |
		BIT(NL80211_IFTYPE_ADHOC) |
		BIT(NL80211_IFTYPE_MESH_POINT) |
		0;

	/* set band channel list */
	for (band = NL80211_BAND_2GHZ;
	     band < (enum nl80211_band)NUM_NL80211_BANDS; band++) {
		struct ieee80211_supported_band *sband;

		sband = &data->bands[band];
		if (rtw_hwsim_init_supported_band(sband, band) < 0)
			continue;
		wiphy->bands[band] = sband;

		RTW_INFO("%s(): band=%d added\n", __func__, band);
	}

	wiphy->mgmt_stypes = rtw_hwsim_mgmt_stypes;

	wiphy->max_scan_ssids = 1;	/* for scan */
	wiphy->max_scan_ie_len = 1024;

	/* cipher suites */
	wiphy->cipher_suites = rtw_hwsim_cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(rtw_hwsim_cipher_suites);
}

static struct cfg80211_ops rtw_hwsim_cfg80211_ops;

static int rtw_hwsim_cfg80211_change_iface(struct wiphy *wiphy,
                                           struct net_device *ndev,
                                           enum nl80211_iftype type,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
                                           u32 *flags,
#endif
                                           struct vif_params *params)
{
	struct rtw_hwsim_vif *vif;
	struct wireless_dev *wdev;

	vif = ndev_to_vif(ndev);
	wdev = vif_to_wdev(vif);

	wdev->iftype = type;

	RTW_INFO("%s(): change type to %d\n", __func__, type);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
	return rtw_cfg80211_ops.change_virtual_intf(wiphy, ndev, type, flags,
	                                            params);
#else
	return rtw_cfg80211_ops.change_virtual_intf(wiphy, ndev, type, params);
#endif
}

static void rtw_hwsim_cfg80211_init_ops(struct cfg80211_ops *ops)
{
	_rtw_memcpy(ops, &rtw_cfg80211_ops, sizeof(struct cfg80211_ops));

	/* override the default */
	ops->change_virtual_intf = rtw_hwsim_cfg80211_change_iface;
}

struct rtw_hwsim_data *rtw_hwsim_cfg80211_alloc(struct device *dev, int index,
                                                int mon)
{
	struct wiphy *wiphy;
	struct rtw_hwsim_data *data;

	rtw_hwsim_cfg80211_init_ops(&rtw_hwsim_cfg80211_ops);

	/* create a new wiphy for use with cfg80211 */
	wiphy = wiphy_new(&rtw_hwsim_cfg80211_ops,
			  sizeof(struct rtw_hwsim_data));
	if (!wiphy) {
		RTW_ERR("couldn't allocate wiphy device\n");
		return NULL;
	}

	data = wiphy_priv(wiphy);
	data->wiphy = wiphy;
	data->dev = dev;

	/* set device pointer for wiphy */
	set_wiphy_dev(wiphy, data->dev);

	/* init wiphy */
	rtw_hwsim_wiphy_init(wiphy, data);

	/* add monitor support for the last radio */
	if (mon) {
		wiphy->interface_modes |= BIT(NL80211_IFTYPE_MONITOR);
		wiphy->software_iftypes |= BIT(NL80211_IFTYPE_MONITOR);
	}

	return data;
}

void rtw_hwsim_cfg80211_free(struct rtw_hwsim_data *data)
{
	struct wiphy *wiphy;

	wiphy = data->wiphy;
	wiphy_free(wiphy);
}

int rtw_hwsim_cfg80211_register(struct rtw_hwsim_data *data)
{
	int err;
	struct wiphy *wiphy;

	wiphy = data->wiphy;
	err = wiphy_register(wiphy);

	if (err < 0) {
		RTW_ERR("couldn't register wiphy device\n");
		goto failed;
	}

	return 0;

failed:
	return err;
}

void rtw_hwsim_cfg80211_unregister(struct rtw_hwsim_data *data)
{
	struct wiphy *wiphy;

	wiphy = data->wiphy;
	wiphy_unregister(wiphy);
}
