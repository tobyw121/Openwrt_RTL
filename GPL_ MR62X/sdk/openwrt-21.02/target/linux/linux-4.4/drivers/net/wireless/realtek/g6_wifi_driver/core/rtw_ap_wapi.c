#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)

#include <linux/unistd.h>
#include <linux/etherdevice.h>
#include <drv_types.h>
#include <rtw_ap_wapi.h>

const u8 oui_wapi[] = {0x00, 0x14, 0x72};

u8 DBG_WAPI_USK_UPDATE_ONCE = 0;
u8 DBG_WAPI_MSK_UPDATE_ONCE = 0;
u8 DBG_WAPI_IMK_UPDATE_ONCE = 0;
u32 wapi_debug_component = WAPI_TX | WAPI_RX | WAPI_ERR;

/*
 * If PN1 < PN2, return a negative integer.
 * If PN1 = PN2, return 0.
 * If PN1 > PN2, return a positive integer.
 */
static int rtw_wapi_compare_PN(const u8 *PN1, const u8 *PN2, size_t n)
{
	for (; n && PN1[n - 1] == PN2[n - 1]; n--);
	return n ? PN1[n - 1] - PN2[n - 1] : 0;
}


/* Return 1 if overflow, otherwise return 0. */
static int rtw_wapi_increase_PN(u8 *PN, size_t n, u8 inc)
{
	u8 old = 0;

	for (; n; n--, PN++) {
		old = *PN;
		*PN += inc;
		if (*PN < old)
			inc = 1;
		else
			return 0;
	}

	return 1;
}

void rtw_wapi_init(_adapter *padapter)
{
	_rtw_memset(&padapter->wapiApInfo, 0, sizeof(RTL_WAPI_AP_INFO));
}

int rtw_validate_wapi_frame(_adapter *padapter, union recv_frame *precv_frame)
{
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct sta_info *psta = precv_frame->u.hdr.psta;
#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *hdr = precv_frame->u.hdr.wlan_hdr;
#else
	u8 *hdr = precv_frame->u.hdr.rx_data;
#endif
	WLAN_HDR_WAPI_EXT *ext = (WLAN_HDR_WAPI_EXT *)(hdr + pattrib->hdrlen);
	int bmc = IS_MCAST(pattrib->ra);
	size_t pn_len = 0;
	int queue = 0;
	u8 key_idx = 0;
	u8 *rx_pn = NULL;

	switch (pattrib->encrypt) {
	case _SMS4_:
		pn_len = WAPI_PN_LEN;
		break;
	case _GCM_SM4_:
		pn_len = WAPI_GCM_PN_LEN;
		break;
	default:
		return _SUCCESS;
	}

	if (bmc) {
		if (!MLME_IS_STA(padapter)) {
			WAPI_TRACE(WAPI_RX, "%s: BC/MC frame to AP!\n",
				   __func__);
			return _SUCCESS;
		}
	}

	/* [0 ... 15] for Data frames */
	if (pattrib->qos)
		queue = pattrib->priority;
	/* 16 for Robust Management frames */
	else if (WIFI_MGT_TYPE == GetFrameType(hdr))
		queue = TID_NUM;

	if (bmc) {
		key_idx = padapter->wapiApInfo.keyIdx;
		rx_pn = padapter->wapiApInfo.rxMcastPN[queue];
	} else {
		key_idx = psta->wapiStaInfo.keyIdx;
		rx_pn = psta->wapiStaInfo.rxUcastPN[queue];
	}

	if (key_idx != ext->keyIdx) {
		WAPI_TRACE(WAPI_RX, "%s: Invalid key index %d!\n",
			   __func__, ext->keyIdx);
		return _FAIL;
	}

	if (!bmc) {
		if (MLME_IS_STA(padapter)) {
			if (!(ext->PN[0] & 0x1)) {
				WAPI_TRACE(WAPI_RX, "%s: Even unicast PN!\n",
					   __func__);
				return _FAIL;
			}
		}

		if (MLME_IS_AP(padapter)) {
			if (ext->PN[0] & 0x1) {
				WAPI_TRACE(WAPI_RX, "%s: Odd unicast PN!\n",
					   __func__);
				return _FAIL;
			}
		}
	}

	if (rtw_wapi_compare_PN(ext->PN, rx_pn, pn_len) <= 0) {
		WAPI_TRACE(WAPI_RX, "%s: Replay detected!\n", __func__);
		return _FAIL;
	}

	_rtw_memcpy(rx_pn, ext->PN, WAPI_PN_LEN);
	return _SUCCESS;
}

void rtw_wapi_get_iv(_adapter *padapter, struct sta_info *psta, u8 key_id, u8 *iv)
{
	WLAN_HDR_WAPI_EXT *ext = (WLAN_HDR_WAPI_EXT *)iv;
	u8 *addr = NULL, *tx_pn = NULL, *force_update = NULL;
	enum nl80211_key_type key_type;
	int alg = 0, step = 1;
	size_t pn_len = 0;
	int overflow = 0;

	_rtw_memset(ext, 0, sizeof(*ext));

	if (psta) {
		addr = psta->phl_sta->mac_addr;
		key_type = NL80211_KEYTYPE_PAIRWISE;

		if (key_id == psta->wapiStaInfo.keyIdx) {
			alg = psta->dot118021XPrivacy;
			tx_pn = psta->wapiStaInfo.txUcastPN;
			step = 2;
			force_update = &DBG_WAPI_USK_UPDATE_ONCE;
		} else {
			WAPI_TRACE(WAPI_TX, "%s: Invalid key index %d!\n",
				   __func__, key_id);
			return;
		}
	} else {
		addr = adapter_mac_addr(padapter);
		key_type = NL80211_KEYTYPE_GROUP;

		if (key_id == padapter->wapiApInfo.keyIdx) {
			alg = padapter->securitypriv.dot118021XGrpPrivacy;
			tx_pn = padapter->wapiApInfo.txMcastPN;
			force_update = &DBG_WAPI_MSK_UPDATE_ONCE;
		}
#ifdef CONFIG_IEEE80211W
		else if (key_id == padapter->securitypriv.dot11wBIPKeyid) {
			alg = padapter->securitypriv.dot11wCipher;
			tx_pn = padapter->wapiApInfo.imk_tx_pn;
			force_update = &DBG_WAPI_IMK_UPDATE_ONCE;
		}
#endif
		else {
			WAPI_TRACE(WAPI_TX, "%s: Invalid key index %d!\n",
				   __func__, key_id);
			return;
		}
	}

	switch (alg) {
	case _SMS4_:
		pn_len = WAPI_PN_LEN;
		break;
	case _GCM_SM4_:
		pn_len = WAPI_GCM_PN_LEN;
		break;
#ifdef CONFIG_IEEE80211W
	case _BIP_CMAC_SM4_128_:
		pn_len = WAPI_PN_LEN;
		break;
#endif
	default:
		WAPI_TRACE(WAPI_TX, "%s: Invalid algorithm!\n", __func__);
		return;
	}

	overflow = rtw_wapi_increase_PN(tx_pn, pn_len, step);
	if ((overflow || (force_update && *force_update)) && MLME_IS_AP(padapter)) {
		rtw_cfg80211_indicate_wapi_key_update(padapter, addr, key_type, key_id);
		WAPI_TRACE(WAPI_TX, "%s: PN overflow!\n", __func__);
		if (force_update && *force_update)
			*force_update = 0;
	}

	ext->keyIdx = key_id & 0x1;
	ext->reserved = 0;
	_rtw_memcpy(ext->PN, tx_pn, WAPI_PN_LEN);
}

int rtw_wapi_set_key(_adapter *padapter, struct ieee_param *param)
{
	int i = 0;
	u8 keyIdx = 0;
	enum security_type alg = _NO_PRIVACY_;
	struct sta_info *psta = NULL, *pbcmc_sta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	RTL_WAPI_STA_INFO *pwapiStaInfo = NULL;
	RTL_WAPI_AP_INFO *pwapiApInfo = &padapter->wapiApInfo;
	u8 wapiUcastPNInitValue[] = {0x37, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C};
	u8 wapiMcastPNInitValue[] = {0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C};

	if (MLME_IS_STA(padapter))
		wapiUcastPNInitValue[0] = 0x36;

	keyIdx = param->u.crypt.idx;
	if (strcmp(param->u.crypt.alg, "SMS4") == 0)
		alg = _SMS4_;
	else if (strcmp(param->u.crypt.alg, "GCM_SM4") == 0)
		alg = _GCM_SM4_;
#ifdef CONFIG_IEEE80211W
	else if (strcmp(param->u.crypt.alg, "SM4_CMAC_128") == 0)
		alg = _BIP_CMAC_SM4_128_;
#endif /* CONFIG_IEEE80211W */
	else
		return (-EINVAL);

	if ((alg == _SMS4_) || (alg == _GCM_SM4_)) {
		if ((keyIdx < 0) || (keyIdx > 1) ||
		    (param->u.crypt.key_len != 32)) {
			WAPI_TRACE(WAPI_ERR, "%s: Invalid key param, idx = %d, key_len = %d!\n",
				   __func__, keyIdx, param->u.crypt.key_len);
			return (-EINVAL);
		}
	}
#ifdef CONFIG_IEEE80211W
	else if (alg == _BIP_CMAC_SM4_128_) {
		if ((keyIdx < 4) || (keyIdx > 5) ||
		    (param->u.crypt.set_tx == 1) ||
		    (param->u.crypt.key_len != 16)) {
			WAPI_TRACE(WAPI_ERR, "%s: Invalid key param, idx = %d, set_tx = %d, key_len = %d!\n",
				   __func__, keyIdx, param->u.crypt.set_tx, param->u.crypt.key_len);
			return (-EINVAL);
		}
	}
#endif /* CONFIG_IEEE80211W */

	if (param->u.crypt.set_tx == 1) { /* usk */
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if (!psta) {
			WAPI_TRACE(WAPI_ERR, "%s: sta "MAC_FMT" not found!\n", __FUNCTION__, MAC_ARG(param->sta_addr));
			return (-EINVAL);
		}

		pwapiStaInfo = &psta->wapiStaInfo;
		pwapiStaInfo->keyIdx = keyIdx;
		psta->dot118021XPrivacy = alg;

		_rtw_memcpy(pwapiStaInfo->wapiUcastKey[keyIdx].dataKey, param->u.crypt.key, WAPI_KEY_LEN);
		_rtw_memcpy(pwapiStaInfo->wapiUcastKey[keyIdx].micKey, param->u.crypt.key + WAPI_KEY_LEN, WAPI_KEY_LEN);
		_rtw_memcpy(psta->dot118021x_UncstKey.skey, param->u.crypt.key, 32);

		_rtw_memcpy(pwapiStaInfo->txUcastPN, wapiUcastPNInitValue, WAPI_PN_LEN);
		for (i = 0; i < TID_NUM + 1; i++)
			_rtw_memcpy(pwapiStaInfo->rxUcastPN[i], wapiUcastPNInitValue, WAPI_PN_LEN);

		pwapiStaInfo->wapiUcastEnable = true;
		psta->ieee8021x_blocked = _FALSE;
		psta->bpairwise_key_installed = _TRUE;

		if (psecuritypriv->sw_encrypt == false || psecuritypriv->sw_decrypt == false) {
			if (MLME_IS_STA(padapter))
				rtw_setstakey_cmd(padapter, psta, UNICAST_KEY, _TRUE);
			else
				rtw_ap_set_sta_key(padapter, param->sta_addr, alg, param->u.crypt.key, keyIdx, 0);
		}
	}
#ifdef CONFIG_IEEE80211W
	else if (alg == _BIP_CMAC_SM4_128_) { /* imk */
		psecuritypriv->dot11wBIPKeyid = keyIdx;
		psecuritypriv->dot11wCipher = alg;
		_rtw_memcpy(psecuritypriv->dot11wBIPKey[keyIdx].skey,
			    param->u.crypt.key, param->u.crypt.key_len);
		_rtw_memcpy(pwapiApInfo->imk_tx_pn, wapiMcastPNInitValue, WAPI_PN_LEN);
		_rtw_memcpy(pwapiApInfo->imk_rx_pn, wapiMcastPNInitValue, WAPI_PN_LEN);
		psecuritypriv->binstallBIPkey = _TRUE;
	}
#endif /* CONFIG_IEEE80211W */
	else { /* msk */
		pwapiApInfo->keyIdx = keyIdx;
		psecuritypriv->dot118021XGrpKeyid = keyIdx;
		psecuritypriv->dot118021XGrpPrivacy = alg;

		_rtw_memcpy(pwapiApInfo->wapiMcastKey[keyIdx].dataKey, param->u.crypt.key, WAPI_KEY_LEN);
		_rtw_memcpy(pwapiApInfo->wapiMcastKey[keyIdx].micKey, param->u.crypt.key + WAPI_KEY_LEN, WAPI_KEY_LEN);
		_rtw_memcpy(psecuritypriv->dot118021XGrpKey[keyIdx].skey, param->u.crypt.key, 32);

		_rtw_memcpy(pwapiApInfo->txMcastPN, wapiMcastPNInitValue, WAPI_PN_LEN);
		for (i = 0; i < TID_NUM + 1; i++)
			_rtw_memcpy(pwapiApInfo->rxMcastPN[i], wapiMcastPNInitValue, WAPI_PN_LEN);

		pwapiApInfo->wapiMcastEnable = true;

		pbcmc_sta = rtw_get_bcmc_stainfo(padapter);
		if (pbcmc_sta) {
			pbcmc_sta->ieee8021x_blocked = _FALSE;
			if (MLME_IS_STA(padapter))
				pbcmc_sta->dot118021XPrivacy = psecuritypriv->dot11PrivacyAlgrthm;
			else
				pbcmc_sta->dot118021XPrivacy = alg;
		}

		psecuritypriv->binstallGrpkey = _TRUE;
		if (!MLME_IS_STA(padapter))
			psecuritypriv->dot11PrivacyAlgrthm = alg;

		if (psecuritypriv->sw_encrypt == false || psecuritypriv->sw_decrypt == false) {
			if (MLME_IS_STA(padapter))
				rtw_set_key(padapter, psecuritypriv, keyIdx, 1, _TRUE);
			else
				rtw_ap_set_group_key(padapter, param->u.crypt.key, alg, keyIdx);
		}
	}

	return 0;
}

int rtw_wapi_check_frame_qos(u8 *whdr, uint len)
{
	u8 type, subtype;
	int offset;
	u16 qc;

	if (!whdr || (len < WLAN_HDR_A3_QOS_LEN))
		return 0;

	type = GetFrameType(whdr);
	subtype = get_frame_sub_type(whdr);
	offset = (get_tofr_ds(whdr) == 3) ? WLAN_HDR_A4_LEN : WLAN_HDR_A3_LEN;

	if ((type != WIFI_DATA_TYPE) || !(subtype & BIT(7)) ||
	    (len < offset + 2))
		return 0;

	_rtw_memcpy(&qc, whdr + offset, 2);
	qc = le16_to_cpu(qc);

	/* check bit 4~6 and 8~15 */
	return !!(qc & 0xff70);
}

#ifdef CONFIG_IEEE80211W
size_t rtw_wapi_build_mmie(_adapter *padapter, u8 *buf, size_t len)
{
	int alg = padapter->securitypriv.dot11wCipher;
	u8 key_id = padapter->securitypriv.dot11wBIPKeyid;
	struct wapi_mmie *ie = (struct wapi_mmie *)buf;

	if (alg != _BIP_CMAC_SM4_128_ || !buf || len < sizeof(*ie))
		return 0;

	_rtw_memset(ie, 0, sizeof(*ie));
	ie->eid =_VENDOR_SPECIFIC_IE_;
	ie->len = sizeof(*ie) - 2;
	_rtw_memcpy(ie->oui, oui_wapi, 3);
	ie->sub_type = cpu_to_le16(MMIE_OUI_TYPE);
	ie->sub_len = ie->len - 6;
	rtw_wapi_get_iv(padapter, NULL, key_id, (u8 *)&ie->key_id);

	/* MIC is filled afterwards in rtw_mgmt_xmitframe_coalesce */

	return sizeof(*ie);
}

int rtw_wapi_verify_mmie(u8 *mmie, u16 key_id, u8 *rx_pn)
{
	struct wapi_mmie *ie = (struct wapi_mmie *)mmie;

	if ((ie->eid != _VENDOR_SPECIFIC_IE_) ||
	    (ie->len != sizeof(*ie) - 2) ||
	    (!_rtw_memcmp(ie->oui, oui_wapi, 3)) ||
	    (ie->sub_type != cpu_to_le16(MMIE_OUI_TYPE)) ||
	    (ie->sub_len != sizeof(*ie) - 8))
		return _FAIL;

	if (ie->key_id != cpu_to_le16(key_id & 0x1)) {
		WAPI_TRACE(WAPI_RX, "%s: Invalid key index %d!\n",
			   __func__, le16_to_cpu(ie->key_id));
		return _FAIL;
	}

	if (rtw_wapi_compare_PN(ie->ipn, rx_pn, WAPI_PN_LEN) <= 0) {
		WAPI_TRACE(WAPI_RX, "%s: Replay detected!\n", __func__);
		return _FAIL;
	}

	/* MIC is verified afterwards in rtw_bip_verify */

	return _SUCCESS;
}
#endif
#endif
