/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
 *
 *
 ******************************************************************************/
#define  _IOCTL_FREEBSD_C_

#include <drv_types.h>

//#ifdef CONFIG_MP_INCLUDED
#include <rtw_mp_ioctl.h>
#include <linux/wireless.h>
//#endif


#ifdef PLATFORM_FREEBSD
#define	IEEE80211_MODE_MAX	(IEEE80211_MODE_QUARTER+1)
#endif


#define RTL_IOCTL_WPA_SUPPLICANT	SIOCIWFIRSTPRIV+30

#define SCAN_ITEM_SIZE 768
#define MAX_CUSTOM_LEN 64
#define RATE_COUNT 4

#ifdef CONFIG_SIGNAL_PID_WHILE_DRV_INIT
extern int ui_pid;
#endif

#ifdef PLATFORM_FREEBSD
#ifdef CONFIG_AP_MODE

#define RTW_IEEE80211_APPIE_BEACON (IEEE80211_FC0_TYPE_MGT|IEEE80211_FC0_SUBTYPE_BEACON)
#define RTW_IEEE80211_APPIE_PROBE_RESP (IEEE80211_FC0_TYPE_MGT|IEEE80211_FC0_SUBTYPE_PROBE_RESP)
#define RTW_IEEE80211_APPIE_ASSOC_RESP (IEEE80211_FC0_TYPE_MGT|IEEE80211_FC0_SUBTYPE_ASSOC_RESP)

static int rtw_del_sta(_adapter *padapter, u_long cmd, struct ieee80211req *ireq);
static int rtw_set_wps_beacon(_adapter *padapter, u_long cmd, struct ieee80211req *ireq);
static int rtw_set_wps_probe_resp(_adapter *padapter, u_long cmd, struct ieee80211req *ireq);
static int rtw_set_wps_assoc_resp(_adapter *padapter, u_long cmd, struct ieee80211req *ireq);

#endif
#endif

extern u8 key_2char2num(u8 hch, u8 lch);
extern u8 str_2char2num(u8 hch, u8 lch);

u32 rtw_rates[] = {1000000,2000000,5500000,11000000,
	6000000,9000000,12000000,18000000,24000000,36000000,48000000,54000000};

static const char * const iw_operation_mode[] = 
{ 
	"Auto", "Ad-Hoc", "Managed",  "Master", "Repeater", "Secondary", "Monitor" 
};

void rtw_request_wps_pbc_event(_adapter *padapter);
void rtw_request_wps_pbc_event(_adapter *padapter)
{
}

void indicate_wx_scan_complete_event(_adapter *padapter);
void indicate_wx_scan_complete_event(_adapter *padapter)
{	
	struct ifnet *ifp = padapter->pifp;

//	CURVNET_SET(ifp->if_vnet);
	rt_ieee80211msg(ifp, RTM_IEEE80211_SCAN, NULL, 0);
//	CURVNET_RESTORE();
}



void rtw_indicate_wx_assoc_event(_adapter *padapter);
void rtw_indicate_wx_assoc_event(_adapter *padapter)
{
	struct ifnet *ifp = padapter->pifp;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct ieee80211_join_event iev;

//	CURVNET_SET(ifp->if_vnet);	
	_rtw_memset(&iev, 0, sizeof(iev));
	_rtw_memcpy(iev.iev_addr, pmlmepriv->cur_network.network.MacAddress, ETH_ALEN);
	rt_ieee80211msg(ifp, RTM_IEEE80211_ASSOC, &iev, sizeof(iev));
//	CURVNET_RESTORE();
}


void rtw_indicate_wx_disassoc_event(_adapter *padapter);
void rtw_indicate_wx_disassoc_event(_adapter *padapter)
{	
	struct ifnet *ifp = padapter->pifp;

//	CURVNET_SET(ifp->if_vnet);	
	rt_ieee80211msg(ifp, RTM_IEEE80211_DISASSOC, NULL, 0);
//	CURVNET_RESTORE();
}

static char *translate_scan(_adapter *padapter, 
				struct wlan_network *pnetwork,
				char *start)
{
	struct ieee80211req_scan_result *sr = (struct ieee80211req_scan_result *) start;
	
	u16 cap;
	u32 i = 0;	
	u8 *cp;

#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;
#endif //CONFIG_P2P

#ifdef CONFIG_TDLS
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	
	if(pnetwork->network.Reserved[1]!='T')
		return start;
#endif

#ifdef CONFIG_P2P
	if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE) && !rtw_p2p_chk_state(pwdinfo, P2P_STATE_IDLE))
	{
		u32	blnGotP2PIE = _FALSE;
		
		//	User is doing the P2P device discovery
		//	The prefix of SSID should be "DIRECT-" and the IE should contains the P2P IE.
		//	If not, the driver should ignore this AP and go to the next AP.

		//	Verifying the SSID
		if ( _rtw_memcmp( pnetwork->network.Ssid.Ssid, pwdinfo->p2p_wildcard_ssid, P2P_WILDCARD_SSID_LEN ) )
		{
			u32	p2pielen = 0;

			//	Verifying the P2P IE
			if ( rtw_get_p2p_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen) )
			{
				blnGotP2PIE = _TRUE;
			}

		}

		if ( blnGotP2PIE == _FALSE )
		{
			return start;
		}
		
	}

#endif //CONFIG_P2P

	if(pnetwork->network.Ssid.SsidLength==0)
		return start;
	
	sr->isr_ie_off = sizeof(struct ieee80211req_scan_result);

	/*  AP MAC address  */
	_rtw_memcpy(sr->isr_bssid, pnetwork->network.MacAddress, ETH_ALEN);

	/* Add the ESSID */
	sr->isr_ssid_len = min((u16)pnetwork->network.Ssid.SsidLength, (u16)32);
	cp = ((u8 *)sr) + sr->isr_ie_off;

	_rtw_memcpy(cp, pnetwork->network.Ssid.Ssid, sr->isr_ssid_len);
	cp += sr->isr_ssid_len;

	 /* Add frequency/channel */
 	if(pnetwork->network.Configuration.DSConfig<1)// || pnetwork->network.Configuration.DSConfig>14)
		pnetwork->network.Configuration.DSConfig = 1;

	if ( pnetwork->network.Configuration.DSConfig <= 14 )
	{
		sr->isr_freq = 2412 + 5*(pnetwork->network.Configuration.DSConfig-1);
		sr->isr_flags = 0;
		if ((rtw_is_cckratesonly_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE){
			sr->isr_flags |= IEEE80211_CHAN_B;
		}
		else if ((rtw_is_cckrates_included((u8*)&pnetwork->network.SupportedRates)) == _TRUE){
			sr->isr_flags |= IEEE80211_CHAN_G;
		}
	}
	else
	{
		sr->isr_freq = 5180 + 5*(pnetwork->network.Configuration.DSConfig-36);
		sr->isr_flags = IEEE80211_CHAN_A;
	}
	
	/* Add quality statistics */
	sr->isr_rssi = pnetwork->network.Rssi;//dBM
	sr->isr_noise = 0; // noise level

	/*Add beacon interval */
	sr->isr_intval = pnetwork->network.Configuration.BeaconPeriod;

	/* Add mode */
	_rtw_memcpy((u8 *)&cap, rtw_get_capability_from_ie(pnetwork->network.IEs), 2);
	cap = le16_to_cpu(cap);
	sr->isr_capinfo = cap;

	/*Add basic and extended rates */
	while(pnetwork->network.SupportedRates[i]!=0)
	{
		sr->isr_rates[i] =  pnetwork->network.SupportedRates[i];
		i++;
	}
	sr->isr_nrates = i;

	/*Add mesh ID*/
	sr->isr_meshid_len=0;
	
	/*Add IE; IE length*/
	sr->isr_ie_len = pnetwork->network.IELength - 12;
	_rtw_memcpy(cp, &pnetwork->network.IEs[12], sr->isr_ie_len);
	cp += sr->isr_ie_len;

	sr->isr_len = sizeof(struct ieee80211req_scan_result) +sr->isr_ssid_len + sr->isr_meshid_len + sr->isr_ie_len;

#if 1 //it seems that panasonic want 2 bits padding, maybe avoid alignment issue
	sr->isr_len = (sr->isr_len + sizeof(int) - 1) & ~(sizeof(int)-1);
	cp = ((u8 *)start) + sr->isr_len;
#endif

	return cp;
	
}

static int wpa_set_auth_algs(_adapter *padapter, u32 value)
{	
	int ret = 0;

	if (value & IEEE80211_AUTH_AUTO)
	{
		//RTW_INFO("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY and  AUTH_ALG_OPEN_SYSTEM [value:0x%lu]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
	} 
	else if (value & IEEE80211_AUTH_SHARED)
	{
		//RTW_INFO("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY  [value:0x%lu]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeShared;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Shared;
#endif
	} 
	else if(value & IEEE80211_AUTH_OPEN)
	{
		RTW_INFO("wpa_set_auth_algs, AUTH_ALG_OPEN_SYSTEM\n");
		//padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		if(padapter->securitypriv.ndisauthtype < Ndis802_11AuthModeWPAPSK)
		{
#ifdef CONFIG_PLATFORM_MT53XX
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
 			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
#endif
		}
		
	}
	else
	{
		RTW_INFO("wpa_set_auth_algs, error!\n");
		ret = -EINVAL;
	}

	return ret;
	
}


static int wpa_set_encryption(_adapter *padapter, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	NDIS_802_11_WEP	 *pwep = NULL;	
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;		
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#ifdef CONFIG_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif //CONFIG_P2P


	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	if (param_len < (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		if (param->u.crypt.idx > WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}
	} else {
		ret = -EINVAL;
		goto exit;
	}

	if (strcmp(param->u.crypt.alg, "WEP") == 0)
	{
		RTW_INFO("wpa_set_encryption, crypt.alg = WEP\n");

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;	

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		//RTW_INFO("(1)wep_key_idx=%lu\n", wep_key_idx);

		if (wep_key_idx > WEP_KEYS)
			return -EINVAL;


		if (wep_key_len > 0) 
		{
		 	wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
		 	pwep =(NDIS_802_11_WEP	 *) rtw_malloc(wep_total_len);
			if(pwep == NULL){
				goto exit;
			}

		 	_rtw_memset(pwep, 0, wep_total_len);

		 	pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

			if(wep_key_len==13)
			{
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			}
		}
		else {		
			ret = -EINVAL;
			goto exit;
		}

		pwep->KeyIndex = wep_key_idx;
		pwep->KeyIndex |= 0x80000000;

		_rtw_memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if(param->u.crypt.set_tx)
		{
			RTW_INFO("wep, set_tx=1\n");

			if(rtw_set_802_11_add_wep(padapter, pwep) == (u8)_FAIL)
			{
				ret = -EOPNOTSUPP ;
			}
		}
		else
		{
			RTW_INFO("wep, set_tx=0\n");
			
			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and 
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can rtw_set_key to fw/cam
			
			if (wep_key_idx >= WEP_KEYS) {
				ret = -EOPNOTSUPP ;
				goto exit;
			}				
			
		      _rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);
			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;	
			rtw_set_key(padapter, psecuritypriv, wep_key_idx, 0);
		}

		goto exit;		
	}

	if(padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) // 802_1x
	{
		struct sta_info * psta,*pbcmc_sta;
		struct sta_priv * pstapriv = &padapter->stapriv;

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_MP_STATE) == _TRUE) //sta mode
		{
			psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));				
			if (psta == NULL) {
				//DEBUG_ERR( ("Set wpa_set_encryption: Obtain Sta_info fail \n"));
			}
			else
			{
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (strcmp(param->u.crypt.alg, "none") != 0) 
					psta->ieee8021x_blocked = _FALSE;
				
				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					psta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}		

				if(param->u.crypt.set_tx ==1)//pairwise key
				{ 
					_rtw_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					
					if(strcmp(param->u.crypt.alg, "TKIP") == 0)//set mic key
					{						
						//DEBUG_ERR(("\nset key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
						_rtw_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
						_rtw_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

						padapter->securitypriv.busetkipkey=_FALSE;
					}

					//DEBUG_ERR((" param->u.crypt.key_len=%d\n",param->u.crypt.key_len));
					RTW_INFO(" ~~~~set sta key:unicastkey\n");
					
					rtw_setstakey_cmd(padapter, psta, _TRUE);
				}
				else//group key
				{ 					
					_rtw_memcpy(padapter->securitypriv.dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					_rtw_memcpy(padapter->securitypriv.dot118021XGrptxmickey[param->u.crypt.idx-1].skey,&(param->u.crypt.key[16]),8);
					_rtw_memcpy(padapter->securitypriv.dot118021XGrprxmickey[param->u.crypt.idx-1].skey,&(param->u.crypt.key[24]),8);
                                        padapter->securitypriv.binstallGrpkey = _TRUE;	
					//DEBUG_ERR((" param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
					RTW_INFO(" ~~~~set sta key:groupkey\n");

					padapter->securitypriv.dot118021XGrpKeyid = param->u.crypt.idx;

					rtw_set_key(padapter,&padapter->securitypriv,param->u.crypt.idx, 1);
#ifdef CONFIG_P2P
					if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_PROVISIONING_ING))
					{
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_DONE);
					}
#endif //CONFIG_P2P
					
				}						
			}

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta==NULL)
			{
				//DEBUG_ERR( ("Set OID_802_11_ADD_KEY: bcmc stainfo is null \n"));
			}
			else
			{
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (strcmp(param->u.crypt.alg, "none") != 0) 
					pbcmc_sta->ieee8021x_blocked = _FALSE;
				
				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{							
					pbcmc_sta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}					
			}				
		}
		else if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE)) //adhoc mode
		{		
		}			
	}

exit:
	
	if (pwep) {
		rtw_mfree((u8 *)pwep, wep_total_len);		
	}	
	
	
	return ret;	
}

static int rtw_set_wpa_ie(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{

	u8 *buf=NULL, *pos=NULL;	
	int group_cipher = 0, pairwise_cipher = 0;
	int ret = 0;

	unsigned short ielen = (unsigned short)ireq->i_len;
	u8 *pie = (u8 *)ireq->i_data;
	
#ifdef CONFIG_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif //CONFIG_P2P

	if((ielen > MAX_WPA_IE_LEN) || (pie == NULL)){
		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		if(pie == NULL)	
			return ret;
		else
			return -EINVAL;
	}

	if(ielen)
	{		
		buf = rtw_zmalloc(ielen);
		if (buf == NULL){
			ret =  -ENOMEM;
			goto exit;
		}

		copyin(pie, buf, ielen);

		//dump
		{
			int i;
			RTW_INFO("\n wpa_ie(length:%d):\n", ielen);
			for(i=0;i<ielen;i=i+8)
				RTW_INFO("0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x \n",buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
		}
	
		pos = buf;
		if(ielen < RSN_HEADER_LEN){
			ret  = -1;
			goto exit;
		}

#if 0
		pos += RSN_HEADER_LEN;
		left  = ielen - RSN_HEADER_LEN;
		
		if (left >= RSN_SELECTOR_LEN){
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}		
		else if (left > 0){
			ret =-1;
			goto exit;
		}
#endif		
		
		if(rtw_parse_wpa_ie(buf, ielen, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPAPSK;
		}
	
		if(rtw_parse_wpa2_ie(buf, ielen, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPA2PSK;		
		}
			
		switch(group_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot118021XGrpPrivacy=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot118021XGrpPrivacy=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:	
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}

		switch(pairwise_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:	
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}
		
		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		{//set wps_ie	
			u16 cnt = 0;	
			u8 eid, wps_oui[4]={0x0,0x50,0xf2,0x04};
			 
			while( cnt < ielen )
			{
				eid = buf[cnt];
		
				if((eid==_VENDOR_SPECIFIC_IE_)&&(_rtw_memcmp(&buf[cnt+2], wps_oui, 4)==_TRUE))
				{
					RTW_INFO("SET WPS_IE\n");

					padapter->securitypriv.wps_ie_len = ( (buf[cnt+1]+2) < (MAX_WPA_IE_LEN<<2)) ? (buf[cnt+1]+2):(MAX_WPA_IE_LEN<<2);
					
					_rtw_memcpy(padapter->securitypriv.wps_ie, &buf[cnt], padapter->securitypriv.wps_ie_len);
					
					set_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS);
					
#ifdef CONFIG_P2P
					if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_OK))
					{
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_ING);
					}
#endif //CONFIG_P2P

					cnt += buf[cnt+1]+2;
					
					break;
				} else {
					cnt += buf[cnt+1]+2; //goto next	
				}				
			}			
		}		
	}
	
 	
exit:

	if (buf) rtw_mfree(buf, ielen);
	
	return ret;	
}

#ifndef PLATFORM_FREEBSD
static int rtw_wx_get_name(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u16 cap;
	u32 ht_ielen = 0;
	char *p;
	u8 ht_cap=_FALSE;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;
	NDIS_802_11_RATES_EX* prates = NULL;



	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE|WIFI_ADHOC_MASTER_STATE) == _TRUE)
	{
		//parsing HT_CAP_IE
		p = rtw_get_ie(&pcur_bss->IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pcur_bss->IELength-12);
		if(p && ht_ielen>0)
		{
			ht_cap = _TRUE;
		}

		prates = &pcur_bss->SupportedRates;

		if (rtw_is_cckratesonly_included((u8*)prates) == _TRUE)
		{
			if(ht_cap == _TRUE)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11b");
		}
		else if ((rtw_is_cckrates_included((u8*)prates)) == _TRUE)
		{
			if(ht_cap == _TRUE)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bgn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bg");
		}
		else
		{
			if(pcur_bss->Configuration.DSConfig > 14)
			{
				if(ht_cap == _TRUE)
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11an");
				else
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11a");
			}
			else
			{
				if(ht_cap == _TRUE)
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11gn");
				else
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11g");
			}
		}
	}
	else
	{
		//prates = &padapter->registrypriv.dev_network.SupportedRates;
		//snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11g");
		snprintf(wrqu->name, IFNAMSIZ, "unassociated");
	}


	return 0;
}

static int rtw_wx_set_freq(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{	


	
	return 0;
}

static int rtw_wx_get_freq(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;
	
	if(check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE)
	{
		//wrqu->freq.m = ieee80211_wlan_frequencies[pcur_bss->Configuration.DSConfig-1] * 100000;
		wrqu->freq.m = rtw_ch2freq(pcur_bss->Configuration.DSConfig) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = pcur_bss->Configuration.DSConfig;

	}
	else{
		wrqu->freq.m = rtw_ch2freq(padapter->mlmeextpriv.cur_channel) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = padapter->mlmeextpriv.cur_channel;
	}

	return 0;
}

static int rtw_wx_set_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	NDIS_802_11_NETWORK_INFRASTRUCTURE networkType ;
	int ret = 0;
	
	
	#if 1 // Jeff: Remove this to test IQK fail
	if (padapter->hw_init_completed==_FALSE){
		ret = -EPERM;
		goto exit;
	}
	#endif
	
	
	switch(wrqu->mode)
	{
		case IW_MODE_AUTO:
			networkType = Ndis802_11AutoUnknown;
			RTW_INFO("set_mode = IW_MODE_AUTO\n");	
			break;				
		case IW_MODE_ADHOC:		
			networkType = Ndis802_11IBSS;
			RTW_INFO("set_mode = IW_MODE_ADHOC\n");			
			break;
		case IW_MODE_MASTER:		
			networkType = Ndis802_11APMode;
			RTW_INFO("set_mode = IW_MODE_MASTER\n");
			break;				
		case IW_MODE_INFRA:
			networkType = Ndis802_11Infrastructure;
			RTW_INFO("set_mode = IW_MODE_INFRA\n");			
			break;
	
		default :
			ret = -EINVAL;;
			goto exit;
	}
	
	if (rtw_set_802_11_infrastructure_mode(padapter, networkType, 0) ==_FALSE){

		ret = -1;
		goto exit;

	}

	// leave power saving mode after new fw_state is applied
	if(_FAIL == rtw_pwr_wakeup(padapter)) {
		ret= _FAIL;
		goto exit;
	}

	rtw_setopmode_cmd(padapter, networkType, RTW_CMDF_WAIT_ACK);

exit:
	
	
	return ret;
	
}

static int rtw_wx_get_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	

	
	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE)
	{
		wrqu->mode = IW_MODE_INFRA;
	}
	else if  ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE) ||
		       (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE))
		
	{
		wrqu->mode = IW_MODE_ADHOC;
	}
	else if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
	{
		wrqu->mode = IW_MODE_MASTER;
	}
	else
	{
		wrqu->mode = IW_MODE_AUTO;
	}

	
	return 0;
	
}


static int rtw_wx_set_pmkid(struct net_device *dev,
	                     struct iw_request_info *a,
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8          j,blInserted = _FALSE;
	int         intReturn = _FALSE;
	struct mlme_priv  *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
        struct iw_pmksa*  pPMK = ( struct iw_pmksa* ) extra;
        u8     strZeroMacAddress[ ETH_ALEN ] = { 0x00 };
        u8     strIssueBssid[ ETH_ALEN ] = { 0x00 };
        
/*
        struct iw_pmksa
        {
            __u32   cmd;
            struct sockaddr bssid;
            __u8    pmkid[IW_PMKID_LEN];   //IW_PMKID_LEN=16
        }
        There are the BSSID information in the bssid.sa_data array.
        If cmd is IW_PMKSA_FLUSH, it means the wpa_suppplicant wants to clear all the PMKID information.
        If cmd is IW_PMKSA_ADD, it means the wpa_supplicant wants to add a PMKID/BSSID to driver.
        If cmd is IW_PMKSA_REMOVE, it means the wpa_supplicant wants to remove a PMKID/BSSID from driver.
        */

	_rtw_memcpy( strIssueBssid, pPMK->bssid.sa_data, ETH_ALEN);
        if ( pPMK->cmd == IW_PMKSA_ADD )
        {
                RTW_INFO( "[rtw_wx_set_pmkid] IW_PMKSA_ADD!\n" );
                if ( _rtw_memcmp( strIssueBssid, strZeroMacAddress, ETH_ALEN ) == _TRUE )
                {
                    return( intReturn );
                }
                else
                {
                    intReturn = _TRUE;
                }
		blInserted = _FALSE;
		
		//overwrite PMKID
		for(j=0 ; j<NUM_PMKID_CACHE; j++)
		{
			if( _rtw_memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==_TRUE )
			{ // BSSID is matched, the same AP => rewrite with new PMKID.
                                
                                RTW_INFO( "[rtw_wx_set_pmkid] BSSID exists in the PMKList.\n" );

				_rtw_memcpy( psecuritypriv->PMKIDList[j].PMKID, pPMK->pmkid, IW_PMKID_LEN);
                                psecuritypriv->PMKIDList[ j ].bUsed = _TRUE;
				psecuritypriv->PMKIDIndex = j+1;
				blInserted = _TRUE;
				break;
			}	
	        }

	        if(!blInserted)
                {
		    // Find a new entry
                    RTW_INFO( "[rtw_wx_set_pmkid] Use the new entry index = %d for this PMKID.\n",
                            psecuritypriv->PMKIDIndex );

	            _rtw_memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].Bssid, strIssueBssid, ETH_ALEN);
		    _rtw_memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].PMKID, pPMK->pmkid, IW_PMKID_LEN);

                    psecuritypriv->PMKIDList[ psecuritypriv->PMKIDIndex ].bUsed = _TRUE;
		    psecuritypriv->PMKIDIndex++ ;
		    if(psecuritypriv->PMKIDIndex==16)
                    {
		        psecuritypriv->PMKIDIndex =0;
                    }
		}
        }
        else if ( pPMK->cmd == IW_PMKSA_REMOVE )
        {
                RTW_INFO( "[rtw_wx_set_pmkid] IW_PMKSA_REMOVE!\n" );
                intReturn = _TRUE;
		for(j=0 ; j<NUM_PMKID_CACHE; j++)
		{
			if( _rtw_memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==_TRUE )
			{ // BSSID is matched, the same AP => Remove this PMKID information and reset it. 
                                _rtw_memset( psecuritypriv->PMKIDList[ j ].Bssid, 0x00, ETH_ALEN );
                                psecuritypriv->PMKIDList[ j ].bUsed = _FALSE;
				break;
			}	
	        }
        }
        else if ( pPMK->cmd == IW_PMKSA_FLUSH ) 
        {
            RTW_INFO( "[rtw_wx_set_pmkid] IW_PMKSA_FLUSH!\n" );
            _rtw_memset( &psecuritypriv->PMKIDList[ 0 ], 0x00, sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
            psecuritypriv->PMKIDIndex = 0;
            intReturn = _TRUE;
        }
    return( intReturn );
}

static int rtw_wx_get_sens(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	
	wrqu->sens.value = 0;
	wrqu->sens.fixed = 0;	/* no auto select */
	wrqu->sens.disabled = 1;
	
	return 0;

}

static int rtw_wx_get_range(struct net_device *dev, 
				struct iw_request_info *info, 
				union iwreq_data *wrqu, char *extra)
{
	struct iw_range *range = (struct iw_range *)extra;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	u16 val;
	int i;
	
	

	wrqu->data.length = sizeof(*range);
	_rtw_memset(range, 0, sizeof(*range));

	/* Let's try to keep this struct in the same order as in
	 * linux/include/wireless.h
	 */

	/* TODO: See what values we can set, and remove the ones we can't
	 * set, or fill them with some default data.
	 */

	/* ~5 Mb/s real (802.11b) */
	range->throughput = 5 * 1000 * 1000;     

	// TODO: Not used in 802.11b?
//	range->min_nwid;	/* Minimal NWID we are able to set */
	// TODO: Not used in 802.11b?
//	range->max_nwid;	/* Maximal NWID we are able to set */

        /* Old Frequency (backward compat - moved lower ) */
//	range->old_num_channels; 
//	range->old_num_frequency;
//	range->old_freq[6]; /* Filler to keep "version" at the same offset */

	/* signal level threshold range */

	//percent values between 0 and 100.
	range->max_qual.qual = 100;	
	range->max_qual.level = 100;
	range->max_qual.noise = 100;
	range->max_qual.updated = 7; /* Updated all three */


	range->avg_qual.qual = 92; /* > 8% missed beacons is 'bad' */
	/* TODO: Find real 'good' to 'bad' threshol value for RSSI */
	range->avg_qual.level = 20 + -98;
	range->avg_qual.noise = 0;
	range->avg_qual.updated = 7; /* Updated all three */

	range->num_bitrates = RATE_COUNT;

	for (i = 0; i < RATE_COUNT && i < IW_MAX_BITRATES; i++) {
		range->bitrate[i] = rtw_rates[i];
	}

	range->min_frag = MIN_FRAG_THRESHOLD;
	range->max_frag = MAX_FRAG_THRESHOLD;

	range->pm_capa = 0;

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 16;

//	range->retry_capa;	/* What retry options are supported */
//	range->retry_flags;	/* How to decode max/min retry limit */
//	range->r_time_flags;	/* How to decode max/min retry life */
//	range->min_retry;	/* Minimal number of retries */
//	range->max_retry;	/* Maximal number of retries */
//	range->min_r_time;	/* Minimal retry lifetime */
//	range->max_r_time;	/* Maximal retry lifetime */

	for (i = 0, val = 0; i < rfctl->max_chan_nums; i++) {

		// Include only legal frequencies for some countries
		if (rfctl->channel_set[i].ChannelNum != 0) {
			range->freq[val].i = rfctl->channel_set[i].ChannelNum;
			range->freq[val].m = rtw_ch2freq(rfctl->channel_set[i].ChannelNum) * 100000;
			range->freq[val].e = 1;
			val++;
		}

		if (val == IW_MAX_FREQUENCIES)
			break;
	}

	range->num_channels = val;
	range->num_frequency = val;

// Commented by Albert 2009/10/13
// The following code will proivde the security capability to network manager.
// If the driver doesn't provide this capability to network manager,
// the WPA/WPA2 routers can't be choosen in the network manager.

/*
#define IW_SCAN_CAPA_NONE		0x00
#define IW_SCAN_CAPA_ESSID		0x01
#define IW_SCAN_CAPA_BSSID		0x02
#define IW_SCAN_CAPA_CHANNEL	0x04
#define IW_SCAN_CAPA_MODE		0x08
#define IW_SCAN_CAPA_RATE		0x10
#define IW_SCAN_CAPA_TYPE		0x20
#define IW_SCAN_CAPA_TIME		0x40
*/

#if WIRELESS_EXT > 17
	range->enc_capa = IW_ENC_CAPA_WPA|IW_ENC_CAPA_WPA2|
			  IW_ENC_CAPA_CIPHER_TKIP|IW_ENC_CAPA_CIPHER_CCMP;
#endif

#ifdef IW_SCAN_CAPA_ESSID //WIRELESS_EXT > 21
	range->scan_capa = IW_SCAN_CAPA_ESSID | IW_SCAN_CAPA_TYPE |IW_SCAN_CAPA_BSSID|
					IW_SCAN_CAPA_CHANNEL|IW_SCAN_CAPA_MODE|IW_SCAN_CAPA_RATE;
#endif



	return 0;

}
#endif

//set bssid flow
//s1. rtw_set_802_11_infrastructure_mode()
//s2. rtw_set_802_11_authentication_mode()
//s3. set_802_11_encryption_mode()
//s4. rtw_set_802_11_bssid()
static int rtw_wx_set_wap(_adapter *padapter, u_long cmd, struct ieee80211req *macaddr)
{
	uint ret = 0;
	char *bssid = (char *) macaddr;
//	struct sockaddr *temp = (struct sockaddr *)awrq;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	_list	*phead;
	u8 *dst_bssid, *src_bssid;
	_queue	*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	NDIS_802_11_AUTHENTICATION_MODE	authmode;

	
	if(_FAIL == rtw_pwr_wakeup(padapter))
	{
		ret= 1;
		goto exit;
	}
	
	if(!padapter->bup){
		ret = 1;
		goto exit;
	}	
	authmode = padapter->securitypriv.ndisauthtype;

       phead = get_list_head(queue);
       pmlmepriv->pscanned = get_next(phead);

	while (1)
	 {
			
		if ((rtw_end_of_queue_search(phead, pmlmepriv->pscanned)) == _TRUE)
		{
#if 0		
			ret = -EINVAL;
			goto exit;

			if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)
			{
	            		rtw_set_802_11_bssid(padapter, temp->sa_data);
	    			goto exit;                    
			}
			else
			{
				ret = -EINVAL;
				goto exit;
			}
#endif

			if (rtw_set_802_11_bssid(padapter, bssid) == _FALSE)
				ret = -1;

	    		goto exit;   			
		}
	
		pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

		pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

		dst_bssid = pnetwork->network.MacAddress;

		src_bssid = bssid;

		if ((_rtw_memcmp(dst_bssid, src_bssid, ETH_ALEN)) == _TRUE)
		{			
			if(!rtw_set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode, 0))
			{
				ret = -1;
				goto exit;
			}

				break;			
		}

	}		

	rtw_set_802_11_authentication_mode(padapter, authmode);

	//set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);
	
	if (rtw_set_802_11_bssid(padapter, bssid) == _FALSE) {
		ret = -1;
		goto exit;		
	}	
	
exit:
	
	
	return ret;	
}

static int rtw_wx_get_wap(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;	



	if  ( ((check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) == _TRUE) || 
			((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)) == _TRUE) ||
			((check_fwstate(pmlmepriv, WIFI_AP_STATE)) == _TRUE) )
	{
		copyout(pcur_bss->MacAddress, ireq->i_data, ETH_ALEN);
//		_rtw_memcpy(ireq->i_data, pcur_bss->MacAddress, ETH_ALEN);
	}
	else
	{
		u8 bssid[6];
	 	_rtw_memset(bssid, 0, ETH_ALEN);
		copyout(bssid, ireq->i_data, ETH_ALEN);
	}		

	
	return 0;
	
}

//set ssid flow
//s1. rtw_set_802_11_infrastructure_mode()
//s2. set_802_11_authenticaion_mode()
//s3. set_802_11_encryption_mode()
//s4. rtw_set_802_11_ssid()
static int rtw_wx_set_essid(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	_queue *queue = &pmlmepriv->scanned_queue;
	_list *phead;
//	s8 status = _TRUE;
	struct wlan_network *pnetwork = NULL;

	NDIS_802_11_AUTHENTICATION_MODE authmode;	
	NDIS_802_11_SSID ndis_ssid;	
	u8 *dst_ssid, *src_ssid=NULL;
	uint ret = 0, len=0;



	if(_FAIL == rtw_pwr_wakeup(padapter))
	{		
		return _FAIL;
	}

	if (ireq->i_len > IW_ESSID_MAX_SIZE){
		ret= -E2BIG;
		goto exit;
	}
	
	if(check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		ret = -1;
		goto exit;
	}		
	
	authmode = padapter->securitypriv.ndisauthtype;
	RTW_INFO("=>%s\n",__FUNCTION__);
	if (ireq->i_len)
	{
		// Commented by Albert 20100519
		// We got the codes in "set_info" function of iwconfig source code.
		//	=========================================
		//	wrq.u.essid.length = strlen(essid) + 1;
	  	//	if(we_kernel_version > 20)
		//		wrq.u.essid.length--;
		//	=========================================
		//	That means, if the WIRELESS_EXT less than or equal to 20, the correct ssid len should subtract 1.

		len = (ireq->i_len < IW_ESSID_MAX_SIZE) ? ireq->i_len : IW_ESSID_MAX_SIZE;

		_rtw_memset(&ndis_ssid, 0, sizeof(NDIS_802_11_SSID));
		ndis_ssid.SsidLength = len;
		copyin(ireq->i_data, ndis_ssid.Ssid, len);
		src_ssid = rtw_malloc(len);
		if(src_ssid==NULL)
			goto exit;
		_rtw_memcpy(src_ssid, ndis_ssid.Ssid, len);

		RTW_INFO("ssid=%s, len=%d\n", (char *)ireq->i_data, (int)len);

		
	       phead = get_list_head(queue);
              pmlmepriv->pscanned = get_next(phead);

		while (1)
		{			
			if (rtw_end_of_queue_search(phead, pmlmepriv->pscanned) == _TRUE)
			{
#if 0			
				if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)
				{
	            			rtw_set_802_11_ssid(padapter, &ndis_ssid);

		    			goto exit;                    
				}
				else
				{
					ret = -EINVAL;
					goto exit;
				}
#endif			

				break;
			}
	
			pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

			pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

			dst_ssid = pnetwork->network.Ssid.Ssid;


			if ((_rtw_memcmp(dst_ssid, src_ssid, ndis_ssid.SsidLength) == _TRUE) &&
				(pnetwork->network.Ssid.SsidLength==ndis_ssid.SsidLength))
			{
				
				if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE)
				{
					if(pnetwork->network.InfrastructureMode != pmlmepriv->cur_network.network.InfrastructureMode)
						continue;
				}	
					
				if (rtw_set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode, 0) == _FALSE)
				{
					ret = -1;
					goto exit;
				}

				break;			
			}
		}

		rtw_set_802_11_authentication_mode(padapter, authmode);
		//set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);
		if (rtw_set_802_11_ssid(padapter, &ndis_ssid) == _FALSE) {
			ret = -1;
			goto exit;
		}	
	}			
	RTW_INFO("<=%s\n",__FUNCTION__);
exit:
	

	if(src_ssid)
		rtw_mfree(src_ssid, len);
	
	return ret;	
}

static int rtw_wx_get_essid(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	u32 len,ret = 0;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;



	if ( (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE) ||
	      (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE))
	{
		len = pcur_bss->Ssid.SsidLength;

		ireq->i_len = len;

		copyout(pcur_bss->Ssid.Ssid, ireq->i_data, len);
//		_rtw_memcpy(ireq->i_data, pcur_bss->Ssid.Ssid, len);

	}
	else
	{
		u8 ssid[32];
		ireq->i_len = 32;
		_rtw_memset(ssid, 0, ireq->i_len);
		copyout(ssid, ireq->i_data, ireq->i_len);
		goto exit;
	}

exit:
	
	
	return ret;
	
}

static int rtw_wx_set_mlme(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret=0;
	u16 reason;
	struct ieee80211req_mlme *mlme = (struct ieee80211req_mlme *) ireq->i_data;
	struct ieee80211req *ireq_essid;
	u8 macaddr[ETH_ALEN]={0, 0, 0, 0, 0, 0}, i;
	u8 ssid_len=0;
	u8 *ssid;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	ssid = (u8 *)rtw_malloc(WLAN_SSID_MAXLEN);
	_rtw_memset(ssid, 0, WLAN_SSID_MAXLEN);
	ireq_essid = (struct ieee80211req *)rtw_malloc(sizeof(struct ieee80211req));
	_rtw_memset(ireq_essid, 0, sizeof(struct ieee80211req));

	if(mlme==NULL)
		return -1;

	reason = cpu_to_le16(mlme->im_reason);

	switch (mlme->im_op) 
	{
		case IEEE80211_MLME_DEAUTH:
		
			if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
			{
#ifdef CONFIG_AP_MODE
				ret = rtw_del_sta(padapter, cmd, ireq);
#endif	
			}
			else
			{
				for (i = 0; i < WLAN_SSID_MAXLEN; i++)
					ssid[i] = (u8)rtw_get_current_time() & 0xFF;

				ireq_essid->i_len = WLAN_SSID_MAXLEN;
				ireq_essid->i_data = ssid;
				rtw_wx_set_wap(padapter, cmd, (struct ieee80211req *) macaddr);
				rtw_wx_set_essid(padapter, cmd, ireq_essid);
				if(!rtw_set_802_11_disassociate(padapter))
					ret = -1;				
			}
	
			break;
			
		case IEEE80211_MLME_DISASSOC:			
		
			if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
			{
#ifdef CONFIG_AP_MODE
				ret = rtw_del_sta(padapter, cmd, ireq);
#endif	
			}
			else
			{
				for (i = 0; i < WLAN_SSID_MAXLEN; i++)
					ssid[i] = (u8)rtw_get_current_time() & 0xFF;
				
				ireq_essid->i_len = WLAN_SSID_MAXLEN;
				ireq_essid->i_data = ssid;
				if(!rtw_set_802_11_disassociate(padapter))
					ret = -1;
				
				rtw_wx_set_wap(padapter, cmd, (struct ieee80211req *) macaddr);
				rtw_wx_set_essid(padapter, cmd, ireq_essid);
			}
			
			break;

		case IEEE80211_MLME_ASSOC:
			rtw_wx_set_wap(padapter, cmd, (struct ieee80211req *) macaddr);

			copyin(&mlme->im_ssid_len, &ssid_len, 1);
			copyin(mlme->im_ssid, ssid, ssid_len);
			ireq_essid->i_len = ssid_len;
			ireq_essid->i_data = ssid;
			
			rtw_wx_set_essid(padapter, cmd, ireq_essid);
			rtw_wx_set_wap(padapter, cmd, (struct ieee80211req *) mlme->im_macaddr);
			break;
		default:
			return -EOPNOTSUPP;
	}

	rtw_mfree(ssid, WLAN_SSID_MAXLEN);
	rtw_mfree((u8 *)ireq_essid, sizeof(struct ieee80211req));
	return ret;
	
}

static int rtw_wx_set_scan(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	u8 _status;
	int ret = 0;	
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);	
#endif
#ifdef DBG_LA_MODE
	struct registry_priv *registry_par = &(padapter->registrypriv);
#endif /*DBG_LA_MODE*/

if (padapter->registrypriv.mp_mode == 1)
{
#ifdef CONFIG_MP_INCLUDED
	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)
	{
		ret = -1;
		goto exit;
	}
#endif
}

#ifdef DBG_LA_MODE
	if(registry_par->la_mode_en == 1 && MLME_IS_ASOC(padapter)) {
		RTW_INFO("%s LA debug mode block Scan request \n", __func__);
		ret = -EPERM;
		goto exit;
	}
#endif

	if(_FAIL == rfpwrstate_check(padapter))
	{
		ret= _FAIL;
		goto exit;
	}

	if(padapter->bDriverStopped){
           //RTW_INFO("bDriverStopped=%lx\n", padapter->bDriverStopped);
		ret= -1;
		goto exit;
	}
	
	if (padapter->hw_init_completed==_FALSE){
		ret = -1;
		goto exit;
	}

	// When Busy Traffic, driver do not site survey. So driver return success.
	// wpa_supplicant will not issue SIOCSIWSCAN cmd again after scan timeout.
	// modify by thomas 2011-02-22.
	if (pmlmepriv->LinkDetectInfo.bBusyTraffic == _TRUE)
	{
		goto exit;
	} 

	if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY|WIFI_UNDER_LINKING) == _TRUE)
	{
		indicate_wx_scan_complete_event(padapter);
		goto exit;
	} 

#ifdef CONFIG_P2P
	if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE) && !rtw_p2p_chk_state(pwdinfo, P2P_STATE_IDLE))
	{
		rtw_p2p_set_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH);
		rtw_p2p_findphase_ex_set(pwdinfo, P2P_FINDPHASE_EX_FULL);
	}
#endif

	_status = rtw_set_802_11_bssid_list_scan(padapter, NULL);

	if(_status == _FALSE)
		ret = -1;

exit:


	return ret;	
}
int rtw_wx_get_scan(_adapter *padapter, u_long cmd, struct ieee80211req *ireq);
int rtw_wx_get_scan(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	_irqL	irqL;
	_list					*plist, *phead;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);	
	_queue				*queue	= &(pmlmepriv->scanned_queue);	
	struct	wlan_network	*pnetwork = NULL;
	char *ev = (char *)ireq->i_data;
	u32 ret = 0;
	u32 cnt=0;
	u32 wait_for_surveydone;
//use temp buffer to copy from scan queue,avoid to copyout during mtx_lock period
#ifdef PLATFORM_FREEBSD 
	char *buf, *pbuf, *pbuf_stop;
	pbuf = buf = rtw_malloc(ireq->i_len);
	pbuf_stop = buf + ireq->i_len;
	if(!buf)
	{
		printf("%s,%d, buffer allocate failed\n",__FUNCTION__,__LINE__);
		goto exit;
	}
#endif //PLATFORM_FREEBSD
	
//	u32 count = 0;
//	char *ev_2;
	
#ifdef CONFIG_P2P
	struct	wifidirect_info*	pwdinfo = &padapter->wdinfo;
#endif



	if(adapter_to_pwrctl(padapter)->brfoffbyhw && padapter->bDriverStopped)
	{
		return EINVAL;
	}
  
#ifdef CONFIG_P2P
	if(!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		//	P2P is enabled
		wait_for_surveydone = 200;
	}
	else
#endif
	{
		wait_for_surveydone = 100;
	}

 	while((check_fwstate(pmlmepriv, (WIFI_UNDER_SURVEY|WIFI_UNDER_LINKING))) == _TRUE)
	{	
		rtw_msleep_os(30);
		cnt++;
		if(cnt > wait_for_surveydone )
			break;
	}

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1)
	{
		if (rtw_end_of_queue_search(phead,plist)== _TRUE){
			break;
		}

#if 0 //test scan
		if((stop - ev) < SCAN_ITEM_SIZE) {
			ret = E2BIG;
			break;
		}
#endif 
		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
//use temp buffer to copy from scan queue,avoid to copyout during mtx_lock period
#ifdef PLATFORM_FREEBSD 

		//sizeof(int) - 1   is  for alignment issue
		if(pbuf_stop - pbuf < SCAN_ITEM_SIZE + sizeof(int) - 1){
			ret = -E2BIG;
			break;
		}
#endif //PLATFORM_FREEBSD

		//report network only if the current channel set contains the channel to which this network belongs
		if (rtw_chset_search_ch(adapter_to_chset(padapter), pnetwork->network.Configuration.DSConfig) >= 0
			&& rtw_mlme_band_check(padapter, pnetwork->network.Configuration.DSConfig) == _TRUE
			&& _TRUE == rtw_validate_ssid(&(pnetwork->network.Ssid))
		)
		{
			pbuf=translate_scan(padapter, pnetwork, pbuf);	//test scan
		}

		plist = get_next(plist);
	
	}        

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
//use temp buffer to copy from scan queue,avoid to copyout during mtx_lock period
#ifdef PLATFORM_FREEBSD 
	copyout(buf,ev,pbuf-buf);
	ireq->i_len = pbuf-buf;
	rtw_mfree(buf, ireq->i_len);
#endif //PLATFORM_FREEBSD
exit:		
	return ret ;
	
}

#ifndef PLATFORM_FREEBSD
static int rtw_wx_set_rate(struct net_device *dev, 
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra)
{
	int	i, ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u8	datarates[NumRates];
	u32	target_rate = wrqu->bitrate.value;
	u32	fixed = wrqu->bitrate.fixed;
	u32	ratevalue = 0;
	 u8 mpdatarate[NumRates]={11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0xff};


	
	if(target_rate == -1){
		ratevalue = 11;
		goto set_rate;
	}
	target_rate = target_rate/100000;

	switch(target_rate){
	case 10:
			ratevalue = 0;
			break;
		case 20:
			ratevalue = 1;
			break;
		case 55:
			ratevalue = 2;
			break;
		case 60:
			ratevalue = 3;
			break;
		case 90:
			ratevalue = 4;
			break;
		case 110:
			ratevalue = 5;
			break;
		case 120:
			ratevalue = 6;
			break;
		case 180:
			ratevalue = 7;
			break;
		case 240:
			ratevalue = 8;
			break;
		case 360:
			ratevalue = 9;
			break;
		case 480:
			ratevalue = 10;
			break;
		case 540:
			ratevalue = 11;
			break;
		default:
			ratevalue = 11;
			break;
	}

set_rate:

	for(i=0; i<NumRates; i++)
	{
		if(ratevalue==mpdatarate[i])
		{
			datarates[i] = mpdatarate[i];
			if(fixed == 0)
				break;
		}
		else{
			datarates[i] = 0xff;
		}

	}

	if( rtw_setdatarate_cmd(padapter, datarates) !=_SUCCESS){
		ret = -1;
	}


	return ret;
}

static int rtw_wx_get_rate(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{	
	int i;
	u8 *p;
	u16 rate = 0, max_rate = 0, ht_cap=_FALSE;
	u32 ht_ielen = 0;	
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	WLAN_BSSID_EX  *pcur_bss = &pmlmepriv->cur_network.network;
	struct rtw_ieee80211_ht_cap *pht_capie;
	u8	bw_40MHz=0, short_GI=0;
	u16	mcs_rate=0;
	u8	rf_type = 0;
	struct registry_priv *pregpriv = &padapter->registrypriv;


	i=0;
if (padapter->registrypriv.mp_mode == 1)
{
#ifdef CONFIG_MP_INCLUDED
	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == _TRUE)
		return -1;
#endif
}
	if((check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE) || (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE))
	{
		p = rtw_get_ie(&pcur_bss->IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pcur_bss->IELength-12);
		if(p && ht_ielen>0)
		{
			ht_cap = _TRUE;	

			pht_capie = (struct rtw_ieee80211_ht_cap *)(p+2);
		
			_rtw_memcpy(&mcs_rate , pht_capie->supp_mcs_set, 2);

			bw_40MHz = (pht_capie->cap_info&IEEE80211_HT_CAP_SUP_WIDTH) ? 1:0;

			short_GI = (pht_capie->cap_info&(IEEE80211_HT_CAP_SGI_20|IEEE80211_HT_CAP_SGI_40)) ? 1:0;
		}

		while( (pcur_bss->SupportedRates[i]!=0) && (pcur_bss->SupportedRates[i]!=0xFF))
		{
			rate = pcur_bss->SupportedRates[i]&0x7F;
			if(rate>max_rate)
				max_rate = rate;

			wrqu->bitrate.fixed = 0;	/* no auto select */
			//wrqu->bitrate.disabled = 1/;
		
			i++;
		}
	
		if(ht_cap == _TRUE)
		{
#if 0 //have some issue,neet to debug - 20101008-georgia
			if(mcs_rate&0x8000)//MCS15
			{
				max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);
			
			}
			else if(mcs_rate&0x0080)//MCS7
			{
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
			}
			else//default MCS7
			{
				//RTW_INFO("wx_get_rate, mcs_rate_bitmap=0x%x\n", mcs_rate);
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
			}
#else
			padapter->hal_func.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
			if(rf_type == RF_1T1R)
				max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);				
			else
				max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);
#endif
			max_rate = max_rate*2;//Mbps/2			
			wrqu->bitrate.value = max_rate*500000;
			
		}
		else
		{
			wrqu->bitrate.value = max_rate*500000;
		}

	}
	else
	{
		return -1;
	}

	return 0;
	
}

static int rtw_wx_get_rts(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	
	
	wrqu->rts.value = padapter->registrypriv.rts_thresh;
	wrqu->rts.fixed = 0;	/* no auto select */
	//wrqu->rts.disabled = (wrqu->rts.value == DEFAULT_RTS_THRESHOLD);
	
	
	return 0;
}

static int rtw_wx_set_frag(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	
	if (wrqu->frag.disabled)
		padapter->xmitpriv.frag_len = MAX_FRAG_THRESHOLD;
	else {
		if (wrqu->frag.value < MIN_FRAG_THRESHOLD ||
		    wrqu->frag.value > MAX_FRAG_THRESHOLD)
			return -EINVAL;
		
		padapter->xmitpriv.frag_len = wrqu->frag.value & ~0x1;
	}
	
	
	return 0;
	
}


static int rtw_wx_get_frag(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	
	
	wrqu->frag.value = padapter->xmitpriv.frag_len;
	wrqu->frag.fixed = 0;	/* no auto select */
	//wrqu->frag.disabled = (wrqu->frag.value == DEFAULT_FRAG_THRESHOLD);
	
	
	return 0;
}

static int rtw_wx_get_retry(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	
	wrqu->retry.value = 7;
	wrqu->retry.fixed = 0;	/* no auto select */
	wrqu->retry.disabled = 1;
	
	return 0;

}	
#endif

#if 0
#define IW_ENCODE_INDEX		0x00FF	/* Token index (if needed) */
#define IW_ENCODE_FLAGS		0xFF00	/* Flags defined below */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#define IW_ENCODE_DISABLED	0x8000	/* Encoding disabled */
#define IW_ENCODE_ENABLED	0x0000	/* Encoding enabled */
#define IW_ENCODE_RESTRICTED	0x4000	/* Refuse non-encoded packets */
#define IW_ENCODE_OPEN		0x2000	/* Accept non-encoded packets */
#define IW_ENCODE_NOKEY		0x0800  /* Key is write only, so not present */
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */
/*
iwconfig wlan0 key on -> flags = 0x6001 -> maybe it means auto
iwconfig wlan0 key off -> flags = 0x8800
iwconfig wlan0 key open -> flags = 0x2800
iwconfig wlan0 key open 1234567890 -> flags = 0x2000
iwconfig wlan0 key restricted -> flags = 0x4800
iwconfig wlan0 key open [3] 1234567890 -> flags = 0x2003
iwconfig wlan0 key restricted [2] 1234567890 -> flags = 0x4002
iwconfig wlan0 key open [3] -> flags = 0x2803
iwconfig wlan0 key restricted [2] -> flags = 0x4802
*/
#endif
#ifndef PLATFORM_FREEBSD
static int rtw_wx_set_enc(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *keybuf)
{	
	u32 key, ret = 0;
	u32 keyindex_provided;
	NDIS_802_11_WEP	 wep;	
	NDIS_802_11_AUTHENTICATION_MODE authmode;

	struct iw_point *erq = &(wrqu->encoding);
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_INFO("+rtw_wx_set_enc, flags=0x%x\n", erq->flags);

	_rtw_memset(&wep, 0, sizeof(NDIS_802_11_WEP));
	
	key = erq->flags & IW_ENCODE_INDEX;
	

	if (erq->flags & IW_ENCODE_DISABLED)
	{
		RTW_INFO("EncryptionDisabled\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
  		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
     		
		goto exit;
	}

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
		keyindex_provided = 1;
	} 
	else
	{
		keyindex_provided = 0;
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
		RTW_INFO("rtw_wx_set_enc, key=%d\n", key);
	}
	
	//set authentication mode	
	if(erq->flags & IW_ENCODE_OPEN)
	{
		RTW_INFO("rtw_wx_set_enc():IW_ENCODE_OPEN\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;//Ndis802_11EncryptionDisabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open;
#endif

		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
  		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	}	
	else if(erq->flags & IW_ENCODE_RESTRICTED)
	{		
		RTW_INFO("rtw_wx_set_enc():IW_ENCODE_RESTRICTED\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

#ifdef CONFIG_PLATFORM_MT53XX
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
#else
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Shared;
#endif

		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;			
		authmode = Ndis802_11AuthModeShared;
		padapter->securitypriv.ndisauthtype=authmode;
	}
	else
	{
		RTW_INFO("rtw_wx_set_enc():erq->flags=0x%x\n", erq->flags);

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;//Ndis802_11EncryptionDisabled;
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
  		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	}
	
	wep.KeyIndex = key;
	if (erq->length > 0)
	{
		wep.KeyLength = erq->length <= 5 ? 5 : 13;

		wep.Length = wep.KeyLength + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
	}
	else
	{
		wep.KeyLength = 0 ;
		
		if(keyindex_provided == 1)// set key_id only, no given KeyMaterial(erq->length==0).
		{
			padapter->securitypriv.dot11PrivacyKeyIndex = key;

			RTW_INFO("(keyindex_provided == 1), keyid=%d, key_len=%d\n", key, padapter->securitypriv.dot11DefKeylen[key]);

			switch(padapter->securitypriv.dot11DefKeylen[key])
			{
				case 5:
					padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;					
					break;
				case 13:
					padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;					
					break;
				default:
					padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;					
					break;
			}
				
			goto exit;
			
		}
		
	}

	wep.KeyIndex |= 0x80000000;

	_rtw_memcpy(wep.KeyMaterial, keybuf, wep.KeyLength);
	
	if (rtw_set_802_11_add_wep(padapter, &wep) == _FALSE) {
		ret = -EOPNOTSUPP;
		goto exit;
	}	

exit:
	
	
	return ret;
	
}

static int rtw_wx_get_enc(struct net_device *dev, 
			    struct iw_request_info *info, 
			    union iwreq_data *wrqu, char *keybuf)
{
	uint key, ret =0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_point *erq = &(wrqu->encoding);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	
	if(check_fwstate(pmlmepriv, WIFI_ASOC_STATE) != _TRUE)
	{
		 if(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) != _TRUE)
		 {
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;
		return 0;
	}	
	}	

	
	key = erq->flags & IW_ENCODE_INDEX;

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
	} else
	{
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
	}	

	erq->flags = key + 1;

	//if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen)
	//{
	//      erq->flags |= IW_ENCODE_OPEN;
	//}	  
	
	switch(padapter->securitypriv.ndisencryptstatus)
	{
		case Ndis802_11EncryptionNotSupported:
		case Ndis802_11EncryptionDisabled:

		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;
	
		break;
		
		case Ndis802_11Encryption1Enabled:					
		
		erq->length = padapter->securitypriv.dot11DefKeylen[key];		

		if(erq->length)
		{
			_rtw_memcpy(keybuf, padapter->securitypriv.dot11DefKey[key].skey, padapter->securitypriv.dot11DefKeylen[key]);
		
		erq->flags |= IW_ENCODE_ENABLED;

			if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen)
			{
	     			erq->flags |= IW_ENCODE_OPEN;
			}
			else if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeShared)
			{
		erq->flags |= IW_ENCODE_RESTRICTED;
			}	
		}	
		else
		{
			erq->length = 0;
			erq->flags |= IW_ENCODE_DISABLED;
		}

		break;

		case Ndis802_11Encryption2Enabled:
		case Ndis802_11Encryption3Enabled:

		erq->length = 16;
		erq->flags |= (IW_ENCODE_ENABLED | IW_ENCODE_OPEN | IW_ENCODE_NOKEY);

		break;
	
		default:
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;

		break;
		
	}
	
	
	return ret;
	
}				     

static int rtw_wx_get_power(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	
	wrqu->power.value = 0;
	wrqu->power.fixed = 0;	/* no auto select */
	wrqu->power.disabled = 1;
	
	return 0;

}
#endif

static int rtw_wx_set_gen_ie(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret;
	uint8_t fc0;

	fc0 = ireq->i_val & 0xff;

	if(fc0==IEEE80211_APPIE_WPA)
	       ret = rtw_set_wpa_ie(padapter, cmd, ireq);
#ifdef CONFIG_AP_MODE
	else if(fc0&RTW_IEEE80211_APPIE_BEACON)
	{
		ret = rtw_set_wps_beacon(padapter, cmd, ireq);
	}
	else if(fc0&RTW_IEEE80211_APPIE_PROBE_RESP)
	{
		ret = rtw_set_wps_probe_resp(padapter, cmd, ireq);
	}
	else if(fc0&RTW_IEEE80211_APPIE_ASSOC_RESP)
	{
		ret = rtw_set_wps_assoc_resp(padapter, cmd, ireq);
	}
#endif	
	else
		ret=0;
	
	return ret;
}	


static int rtw_wx_set_auth(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret = 0;
	switch(ireq->i_type){
		case IEEE80211_IOC_COUNTERMEASURES:
			if(ireq->i_val)
		               padapter->securitypriv.btkip_countermeasure = _TRUE; 
			else
		               padapter->securitypriv.btkip_countermeasure = _FALSE;
			break;
		case IEEE80211_IOC_DROPUNENCRYPTED:
			if(padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption1Enabled)
			{
				break;//it means init value, or using wep, ndisencryptstatus = Ndis802_11Encryption1Enabled, 
						// then it needn't reset it;
			}

			if(!ireq->i_val){	//counter value between Linux and BSD
				padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
				padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; //open system
				padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeOpen;
			}
			break;
		case IEEE80211_IOC_AUTHMODE:
			ret = wpa_set_auth_algs(padapter, ireq->i_val);
			break;
		default:
			return -EOPNOTSUPP;
	}

	return ret;
	
}

static int rtw_wx_set_wpakey(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	char *alg_name;
	u32 param_len;
	u32 key_len = 0;
	struct ieee_param *param = NULL;
	struct ieee80211req_key *wk = NULL;
	struct ieee80211req_key *wk_keydata = NULL;
	int ret=0;

	wk_keydata = (struct ieee80211req_key *)ireq->i_data; 
	wk = (struct ieee80211req_key *)rtw_malloc(sizeof(struct ieee80211req_key));
	if(wk==NULL)
		return ENOMEM;
	copyin(ireq->i_data, wk, sizeof(struct ieee80211req_key));

	key_len = wk->ik_keylen;
	param_len = sizeof(struct ieee_param) + key_len;
	param = (struct ieee_param *)rtw_malloc(param_len);
	if (param == NULL)
		return ENOMEM;
	_rtw_memset(param, 0, param_len);

	param->cmd = IEEE_CMD_SET_ENCRYPTION;
	_rtw_memset(param->sta_addr, 0xff, ETH_ALEN);

	switch (wk->ik_type) {
	case IEEE80211_CIPHER_WEP:
		alg_name = "WEP";
		break;
	case IEEE80211_CIPHER_TKIP:
		alg_name = "TKIP";
		break;
	case IEEE80211_CIPHER_AES_CCM:
		alg_name = "CCMP";
		break;
	default:	
		return -1;
	}
	strncpy((char *)param->u.crypt.alg, alg_name, IEEE_CRYPT_ALG_NAME_LEN);
	
	if(wk->ik_flags & IEEE80211_KEY_GROUP){
		param->u.crypt.set_tx = 0;
		param->u.crypt.idx = wk->ik_keyix;
	}
	if(wk->ik_flags & IEEE80211_KEY_XMIT){
		param->u.crypt.set_tx = 1;
		if(wk->ik_keyix==IEEE80211_KEYIX_NONE)
			param->u.crypt.idx = 0;
		else
			param->u.crypt.idx = wk->ik_keyix;
	}		
	if (wk->ik_flags & IEEE80211_KEY_RECV) 
		copyin(&wk->ik_keyrsc, param->u.crypt.seq, 8);

	if(wk->ik_keylen)
	{
		param->u.crypt.key_len = wk->ik_keylen;
		copyin(wk_keydata->ik_keydata, param->u.crypt.key, wk_keydata->ik_keylen);
	}	

	ret =  wpa_set_encryption(padapter, param, param_len);	

	if(wk)
		rtw_mfree((u8 *)wk, sizeof(struct ieee80211req_key));	
	if(param)
		rtw_mfree((u8*)param, param_len);

	return ret;
}

static int rtw_wx_delkey(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	char *alg_name;
	u32 param_len;
	u32 key_len = 0;
	struct ieee_param *param = NULL;
	struct ieee80211req_del_key *dk = NULL;
	int ret=0;

	return ret;

	dk = (struct ieee80211req_del_key *)rtw_malloc(sizeof(struct ieee80211req_del_key));
	if(dk==NULL)
		return ENOMEM;
	copyin(ireq->i_data, dk, sizeof(struct ieee80211req_del_key));

	key_len = 16;
	param_len = sizeof(struct ieee_param) + key_len;
	param = (struct ieee_param *)rtw_malloc(param_len);
	if (param == NULL)
		return ENOMEM;
	_rtw_memset(param, 0, param_len);
	
	param->cmd = IEEE_CMD_SET_ENCRYPTION;
	_rtw_memset(param->sta_addr, 0xff, ETH_ALEN);

	alg_name = "none";
	strncpy((char *)param->u.crypt.alg, alg_name, IEEE_CRYPT_ALG_NAME_LEN);			
	param->u.crypt.set_tx = 0;
	param->u.crypt.idx = dk->idk_keyix+1;
	param->u.crypt.key_len = 32;
	_rtw_memset(param->u.crypt.key, 0, param->u.crypt.key_len);

	ret =  wpa_set_encryption(padapter, param, param_len);	

	if(dk)
		rtw_mfree((u8 *)dk, sizeof(struct ieee80211req_del_key));
	if(param)
		rtw_mfree((u8*)param, param_len);
	return ret;

}

#ifdef PLATFORM_FREEBSD
static int rtw_wx_get_wpakey(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_priv * pmlmepriv = &padapter->mlmepriv;
	struct ieee80211req_key ik;
	struct sta_info *psta = NULL;
	u8 *sta_addr=NULL;
	u32 kid;
	int error;

	if (ireq->i_len != sizeof(ik))
		return EINVAL;
	error = copyin(ireq->i_data, &ik, sizeof(ik));
	if (error)
		return error;
	kid = ik.ik_keyix;
	if (kid == IEEE80211_KEYIX_NONE) {
		return ENOENT;
	} else {
		if (kid >= IEEE80211_WEP_NKID)
			return EINVAL;
		_rtw_memcpy(ik.ik_macaddr, adapter_mac_addr(padapter), ETH_ALEN);
	}

	switch(psecuritypriv->dot11PrivacyAlgrthm){
		case _NO_PRIVACY_:
			ik.ik_type = IEEE80211_CIPHER_NONE;
			ik.ik_keylen = 0;
			_rtw_memset(ik.ik_keydata, 0, sizeof(ik.ik_keydata));
			break;
		case _WEP40_:
		case _WEP104_:
			ik.ik_type = IEEE80211_CIPHER_WEP;
			ik.ik_keylen = psecuritypriv->dot11DefKeylen[kid];
			_rtw_memcpy(ik.ik_keydata, &psecuritypriv->dot11DefKey[kid], ik.ik_keylen);
			break;
		case _TKIP_:
			ik.ik_type = IEEE80211_CIPHER_TKIP;
			sta_addr=get_bssid(pmlmepriv);
			if(sta_addr==NULL){
				ik.ik_keylen = 0;
				_rtw_memset(ik.ik_keydata, 0, sizeof(ik.ik_keydata));
			} else{
				psta = rtw_get_stainfo(&padapter->stapriv ,sta_addr );
				ik.ik_keylen = 16;
				_rtw_memcpy(ik.ik_keydata, &psta->dot118021x_UncstKey.skey[0], ik.ik_keylen);
			}
#if 0 //copy from ieee80211.c; todo
			if (cip->ic_cipher == IEEE80211_CIPHER_TKIP) {
			memcpy(ik.ik_keydata+wk->wk_keylen,
				wk->wk_key + IEEE80211_KEYBUF_SIZE,
				IEEE80211_MICBUF_SIZE);
			ik.ik_keylen += IEEE80211_MICBUF_SIZE;
			}
#endif					
			break;
		case _AES_:
			ik.ik_type = IEEE80211_CIPHER_AES_CCM;
			sta_addr=get_bssid(pmlmepriv);
			if(sta_addr==NULL){
				ik.ik_keylen = 0;
				_rtw_memset(ik.ik_keydata, 0, sizeof(ik.ik_keydata));
			} else{
				psta = rtw_get_stainfo(&padapter->stapriv ,sta_addr );
				ik.ik_keylen = 16;
				_rtw_memcpy(ik.ik_keydata, &psta->dot118021x_UncstKey.skey[0], ik.ik_keylen);
			}
			break;
	}

	ik.ik_flags |= (IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV | IEEE80211_KEY_DEFAULT);

	return copyout(&ik, ireq->i_data, sizeof(ik));
}

static int rtw_wx_gettxparams(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	struct ieee80211_txparam txparam[IEEE80211_MODE_MAX];
	int i;
	
	int len = ireq->i_len;
	/* NB: accept short requests for backwards compat */
	if (len > sizeof(txparam))
		len = sizeof(txparam);

	for(i=0;i<IEEE80211_MODE_MAX;i++){
		txparam[i].ucastrate = 108;	/* ucast data rate (legacy/MCS|0x80) */
		txparam[i].mgmtrate = 2;		/* mgmt frame rate (legacy/MCS|0x80) */
		txparam[i].mcastrate = 108;	/* multicast rate (legacy/MCS|0x80) */
		txparam[i].maxretry = 7;		/* max unicast data retry count */
	}

	return copyout(&txparam, ireq->i_data, len);
}

static int rtw_wx_getdevcaps(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	struct ieee80211_devcaps_req *dc;
	struct ieee80211_channel *ev;
	char *cp, *rc;
	int nchans = 1;

	cp = rtw_zmalloc(sizeof(u32)*3+sizeof(int)+ sizeof(struct ieee80211_channel)*nchans);
	_rtw_memset(cp, 0, sizeof(u32)*3+sizeof(int)+ sizeof(struct ieee80211_channel)*nchans);
	if(cp==NULL)
		return ENOMEM;
	dc = (struct ieee80211_devcaps_req *)cp;
	dc->dc_drivercaps = IEEE80211_C_STA
					| IEEE80211_C_IBSS
					| IEEE80211_C_PMGT
					| IEEE80211_C_HOSTAP
					| IEEE80211_C_SHSLOT
					| IEEE80211_C_SHPREAMBLE
					| IEEE80211_C_DFS
					| IEEE80211_C_WPA
					| IEEE80211_C_WME;

	dc->dc_cryptocaps = IEEE80211_CRYPTO_WEP
					| IEEE80211_CRYPTO_TKIP
					| IEEE80211_CRYPTO_AES_CCM
					| IEEE80211_CRYPTO_TKIPMIC;

	dc->dc_htcaps = IEEE80211_HTCAP_CHWIDTH40
					| IEEE80211_HTCAP_SMPS_DYNAMIC
					| IEEE80211_HTCAP_GREENFIELD
					| IEEE80211_HTCAP_SHORTGI20
					| IEEE80211_HTCAP_SHORTGI40
					| IEEE80211_HTCAP_DELBA
					| IEEE80211_HTCAP_40INTOLERANT;

	rc = (char *)&dc->dc_chaninfo;
	_rtw_memcpy(rc, &nchans, 4);
	rc+=4;
	ev = (struct ieee80211_channel *)rc;
	ev->ic_flags = IEEE80211_CHAN_G;
	ev->ic_freq = 2412;//+5*i;
	ev->ic_ieee = 1;//+i;
	ev+=2;

	copyout(dc, ireq->i_data, sizeof(u32)*3+sizeof(int)+ sizeof(struct ieee80211_channel)*nchans);
	rtw_mfree((char *)dc, sizeof(u32)*3+sizeof(int)+ sizeof(struct ieee80211_channel)*nchans);
	return 0;
						
}


/*
 * Roaming-related defaults.  RSSI thresholds are as returned by the
 * driver (.5dBm).  Transmit rate thresholds are IEEE rate codes (i.e
 * .5M units) or MCS.
 */
/* rssi thresholds */
#define	ROAM_RSSI_11A_DEFAULT		14	/* 11a bss */
#define	ROAM_RSSI_11B_DEFAULT		14	/* 11b bss */
#define	ROAM_RSSI_11BONLY_DEFAULT	14	/* 11b-only bss */
/* transmit rate thresholds */
#define	ROAM_RATE_11A_DEFAULT		2*12	/* 11a bss */
#define	ROAM_RATE_11B_DEFAULT		2*5	/* 11b bss */
#define	ROAM_RATE_11BONLY_DEFAULT	2*1	/* 11b-only bss */
#define	ROAM_RATE_HALF_DEFAULT		2*6	/* half-width 11a/g bss */
#define	ROAM_RATE_QUARTER_DEFAULT	2*3	/* quarter-width 11a/g bss */
#define	ROAM_MCS_11N_DEFAULT		(1 | IEEE80211_RATE_MCS) /* 11n bss */

static struct ieee80211_roamparam defroam[IEEE80211_MODE_MAX] = {
	[IEEE80211_MODE_11A]	= { .rssi = ROAM_RSSI_11A_DEFAULT,
				    .rate = ROAM_RATE_11A_DEFAULT },
	[IEEE80211_MODE_11G]	= { .rssi = ROAM_RSSI_11B_DEFAULT,
				    .rate = ROAM_RATE_11B_DEFAULT },
	[IEEE80211_MODE_11B]	= { .rssi = ROAM_RSSI_11BONLY_DEFAULT,
				    .rate = ROAM_RATE_11BONLY_DEFAULT },
	[IEEE80211_MODE_TURBO_A]= { .rssi = ROAM_RSSI_11A_DEFAULT,
				    .rate = ROAM_RATE_11A_DEFAULT },
	[IEEE80211_MODE_TURBO_G]= { .rssi = ROAM_RSSI_11A_DEFAULT,
				    .rate = ROAM_RATE_11A_DEFAULT },
	[IEEE80211_MODE_STURBO_A]={ .rssi = ROAM_RSSI_11A_DEFAULT,
				    .rate = ROAM_RATE_11A_DEFAULT },
	[IEEE80211_MODE_HALF]	= { .rssi = ROAM_RSSI_11A_DEFAULT,
				    .rate = ROAM_RATE_HALF_DEFAULT },
	[IEEE80211_MODE_QUARTER]= { .rssi = ROAM_RSSI_11A_DEFAULT,
				    .rate = ROAM_RATE_QUARTER_DEFAULT },
	[IEEE80211_MODE_11NA]	= { .rssi = ROAM_RSSI_11A_DEFAULT,
				    .rate = ROAM_MCS_11N_DEFAULT },
	[IEEE80211_MODE_11NG]	= { .rssi = ROAM_RSSI_11B_DEFAULT,
				    .rate = ROAM_MCS_11N_DEFAULT },
};

static int rtw_wx_getroam(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	struct ieee80211_roamparam getroam[IEEE80211_MODE_MAX];

	int len = ireq->i_len;
	/* NB: accept short requests for backwards compat */
	if (len > sizeof(getroam))
		len = sizeof(getroam);

	_rtw_memcpy(getroam, defroam, sizeof(defroam));
	return copyout(getroam, ireq->i_data, len);
}
#endif

#ifndef PLATFORM_FREEBSD
static int rtw_wx_get_nick(struct net_device *dev, 
			     struct iw_request_info *info, 
			     union iwreq_data *wrqu, char *extra)
{	
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	 //struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	 //struct security_priv *psecuritypriv = &padapter->securitypriv;

	if(extra)
	{
		wrqu->data.length = 14;
		wrqu->data.flags = 1;
		_rtw_memcpy(extra, "<WIFI@REALTEK>", 14);
	}

	//rtw_signal_process(pid, SIGUSR1); //for test

	//dump debug info here	
/*
	u32 dot11AuthAlgrthm;		// 802.11 auth, could be open, shared, and 8021x
	u32 dot11PrivacyAlgrthm;	// This specify the privacy for shared auth. algorithm.
	u32 dot118021XGrpPrivacy;	// This specify the privacy algthm. used for Grp key 
	u32 ndisauthtype;
	u32 ndisencryptstatus;
*/

	//RTW_INFO("auth_alg=0x%x, enc_alg=0x%x, auth_type=0x%x, enc_type=0x%x\n", 
	//		psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm,
	//		psecuritypriv->ndisauthtype, psecuritypriv->ndisencryptstatus);
	
	//RTW_INFO("enc_alg=0x%x\n", psecuritypriv->dot11PrivacyAlgrthm);
	//RTW_INFO("auth_type=0x%x\n", psecuritypriv->ndisauthtype);
	//RTW_INFO("enc_type=0x%x\n", psecuritypriv->ndisencryptstatus);

#if 0
	RTW_INFO("dbg(0x210)=0x%x\n", rtw_read32(padapter, 0x210));
	RTW_INFO("dbg(0x608)=0x%x\n", rtw_read32(padapter, 0x608));
	RTW_INFO("dbg(0x280)=0x%x\n", rtw_read32(padapter, 0x280));
	RTW_INFO("dbg(0x284)=0x%x\n", rtw_read32(padapter, 0x284));
	RTW_INFO("dbg(0x288)=0x%x\n", rtw_read32(padapter, 0x288));
	
	RTW_INFO("dbg(0x664)=0x%x\n", rtw_read32(padapter, 0x664));


	RTW_INFO("\n");

	RTW_INFO("dbg(0x430)=0x%x\n", rtw_read32(padapter, 0x430));
	RTW_INFO("dbg(0x438)=0x%x\n", rtw_read32(padapter, 0x438));

	RTW_INFO("dbg(0x440)=0x%x\n", rtw_read32(padapter, 0x440));
	
	RTW_INFO("dbg(0x458)=0x%x\n", rtw_read32(padapter, 0x458));
	
	RTW_INFO("dbg(0x484)=0x%x\n", rtw_read32(padapter, 0x484));
	RTW_INFO("dbg(0x488)=0x%x\n", rtw_read32(padapter, 0x488));
	
	RTW_INFO("dbg(0x444)=0x%x\n", rtw_read32(padapter, 0x444));
	RTW_INFO("dbg(0x448)=0x%x\n", rtw_read32(padapter, 0x448));
	RTW_INFO("dbg(0x44c)=0x%x\n", rtw_read32(padapter, 0x44c));
	RTW_INFO("dbg(0x450)=0x%x\n", rtw_read32(padapter, 0x450));
#endif
	
	return 0;

}

static int rtw_wx_read32(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	u32 addr;
	u32 data32;


	addr = *(u32*)extra;
	data32 = rtw_read32(padapter, addr);
	sprintf(extra, "0x%08x", data32);

	return 0;
}

static int rtw_wx_write32(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	u32 addr;
	u32 data32;


	addr = *(u32*)extra;
	data32 = *((u32*)extra + 1);
	rtw_write32(padapter, addr, data32);

	return 0;
}

static int rtw_wx_read_rf(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = padapter->hal_func.read_rfreg(padapter, path, addr, 0xFFFFF);
//	RTW_INFO("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32);
	/*
	 * IMPORTANT!!
	 * Only when wireless private ioctl is at odd order,
	 * "extra" would be copied to user space.
	 */
	sprintf(extra, "0x%05x", data32);

	return 0;
}

static int rtw_wx_write_rf(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = *((u32*)extra + 2);
//	RTW_INFO("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32);
	padapter->hal_func.write_rfreg(padapter, path, addr, 0xFFFFF, data32);

	return 0;
}

static int rtw_wx_priv_null(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu, char *b)
{
	return -1;
}

static int dummy(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu, char *b)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	//struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	//RTW_INFO("cmd_code=%x, fwstate=0x%x\n", a->cmd, get_fwstate(pmlmepriv));
	
	return -1;
	
}

static int rtw_wx_set_mtk_wps_probe_ie(struct net_device *dev,
		struct iw_request_info *a,
		union iwreq_data *wrqu, char *b)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

#endif
	return 0;
}

static int rtw_wx_get_sensitivity(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *buf)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

    //wrqu->qual.level = (u8)padapter->mlmepriv.cur_network.network.Rssi;

	wrqu->qual.level = padapter->recvpriv.fw_rssi;

    RTW_INFO(" level = %u\n",  wrqu->qual.level );
#endif
	return 0;
}

static int rtw_wx_set_mtk_wps_ie(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
#ifdef CONFIG_PLATFORM_MT53XX
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	return rtw_set_wpa_ie(padapter, wrqu->data.pointer, wrqu->data.length);
#else
	return 0;
#endif
}
#endif

#ifndef CONFIG_MP_INCLUDED
static void rtw_dbg_mode_hdl(_adapter *padapter, u32 id, u8 *pdata, u32 len)
{
	pRW_Reg 	RegRWStruct;
	struct rf_reg_param *prfreg;
	u8 path;
	u8 offset;
	u32 value;

	RTW_INFO("%s\n", __FUNCTION__);

	switch(id)
	{
		case GEN_MP_IOCTL_SUBCODE(MP_START):
			RTW_INFO("871x_driver is only for normal mode, can't enter mp mode\n");
			break;
		case GEN_MP_IOCTL_SUBCODE(READ_REG):
			RegRWStruct = (pRW_Reg)pdata;
			switch (RegRWStruct->width)
			{
				case 1:
					RegRWStruct->value = rtw_read8(padapter, RegRWStruct->offset);
					break;
				case 2:
					RegRWStruct->value = rtw_read16(padapter, RegRWStruct->offset);
					break;
				case 4:
					RegRWStruct->value = rtw_read32(padapter, RegRWStruct->offset);
					break;
				default:
					break;
			}
		
			break;
		case GEN_MP_IOCTL_SUBCODE(WRITE_REG):
			RegRWStruct = (pRW_Reg)pdata;
			switch (RegRWStruct->width)
			{
				case 1:
					rtw_write8(padapter, RegRWStruct->offset, (u8)RegRWStruct->value);
					break;
				case 2:
					rtw_write16(padapter, RegRWStruct->offset, (u16)RegRWStruct->value);
					break;
				case 4:
					rtw_write32(padapter, RegRWStruct->offset, (u32)RegRWStruct->value);
					break;
				default:					
				break;
			}
				
			break;
		case GEN_MP_IOCTL_SUBCODE(READ_RF_REG):

			prfreg = (struct rf_reg_param *)pdata;

			path = (u8)prfreg->path;		
			offset = (u8)prfreg->offset;	

			value = padapter->hal_func.read_rfreg(padapter, path, offset, 0xffffffff);

			prfreg->value = value;

			break;			
		case GEN_MP_IOCTL_SUBCODE(WRITE_RF_REG):

			prfreg = (struct rf_reg_param *)pdata;

			path = (u8)prfreg->path;
			offset = (u8)prfreg->offset;	
			value = prfreg->value;

			padapter->hal_func.write_rfreg(padapter, path, offset, 0xffffffff, value);
			
			break;			
                case GEN_MP_IOCTL_SUBCODE(TRIGGER_GPIO):
			RTW_INFO("==> trigger gpio 0\n");
			padapter->hal_func.set_hw_reg_handler(padapter, HW_VAR_TRIGGER_GPIO_0, 0);
			break;	
#ifdef CONFIG_BTC
		case GEN_MP_IOCTL_SUBCODE(SET_DM_BT):			
			RTW_INFO("==> set dm_bt_coexist:%x\n",*(u8 *)pdata);
			padapter->hal_func.set_hw_reg_handler(padapter, HW_VAR_BT_SET_COEXIST, pdata);
			break;
		case GEN_MP_IOCTL_SUBCODE(DEL_BA):
			RTW_INFO("==> delete ba:%x\n", *(u8 *)pdata);
			padapter->hal_func.set_hw_reg_handler(padapter, HW_VAR_BT_ISSUE_DELBA, pdata);
			break;
#endif
#ifdef DBG_CONFIG_ERROR_DETECT
		case GEN_MP_IOCTL_SUBCODE(GET_WIFI_STATUS):
			if (padapter->hal_func.sreset_get_wifi_status)
				*pdata = padapter->hal_func.sreset_get_wifi_status(padapter);
			break;
#endif
	
		default:
			break;
	}
	
}
#endif

static s32 rtw_mp_ioctl(PADAPTER padapter, union iwreq_data *wrqu)
{
	s32 ret;
	struct iw_point *p;
if (padapter->registrypriv.mp_mode == 1)
{
#ifdef CONFIG_MP_INCLUDED
	u32 BytesRead, BytesWritten, BytesNeeded;
	struct oid_par_priv	oid_par;
	struct mp_ioctl_handler	*phandler;
#endif
}
	struct mp_ioctl_param	*poidparam;
	u32 status=0;
	u16 len;
	u8 *pparmbuf = NULL, bset;


	RTW_INFO("+rtw_mp_ioctl_hdl\n");

	ret = 0;

	//mutex_lock(&ioctl_mutex);

	p = &wrqu->data;
	if ((!p->length) || (!p->pointer)) {
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	pparmbuf = NULL;
	bset = (u8)(p->flags & 0xFFFF);
	len = p->length;
	pparmbuf = (u8*)rtw_malloc(len);
	if (pparmbuf == NULL){
		ret = -ENOMEM;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	if (copy_from_user(pparmbuf, p->pointer, len)) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	poidparam = (struct mp_ioctl_param *)pparmbuf;

	if (poidparam->subcode >= MAX_MP_IOCTL_SUBCODE) {
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	//RTW_INFO("%s: %d\n", __func__, poidparam->subcode);
if (padapter->registrypriv.mp_mode == 1)
{
#ifdef CONFIG_MP_INCLUDED 
	phandler = mp_ioctl_hdl + poidparam->subcode;

	if ((phandler->paramsize != 0) && (poidparam->len < phandler->paramsize))
	{
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	if (phandler->handler)
	{
		oid_par.adapter_context = padapter;
		oid_par.oid = phandler->oid;
		oid_par.information_buf = poidparam->data;
		oid_par.information_buf_len = poidparam->len;
		oid_par.dbg = 0;

		BytesWritten = 0;
		BytesNeeded = 0;

		if (bset) {
			oid_par.bytes_rw = &BytesRead;
			oid_par.bytes_needed = &BytesNeeded;
			oid_par.type_of_oid = SET_OID;
		} else {
			oid_par.bytes_rw = &BytesWritten;
			oid_par.bytes_needed = &BytesNeeded;
			oid_par.type_of_oid = QUERY_OID;
		}

		status = phandler->handler(&oid_par);

		//todo:check status, BytesNeeded, etc.
	}
	else {
		RTW_INFO("rtw_mp_ioctl_hdl(): err!, subcode=%d, oid=%d, handler=%p\n", 
			poidparam->subcode, phandler->oid, phandler->handler);
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}
	
#endif
}	
//#else
	rtw_dbg_mode_hdl(padapter, poidparam->subcode, poidparam->data, poidparam->len);
	
	if (bset == 0x00) {//query info
		if (copy_to_user(p->pointer, pparmbuf, len))
			ret = -EFAULT;
	}

	if (status) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

_rtw_mp_ioctl_hdl_exit:

	if (pparmbuf)
		rtw_mfree(pparmbuf, len);

	//mutex_unlock(&ioctl_mutex);

	return ret;
}

#ifndef PLATFORM_FREEBSD
static int rtw_get_ap_info(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int bssid_match, ret = 0;
	u32 cnt=0, wpa_ielen;
	_irqL	irqL;
	_list	*plist, *phead;
	unsigned char *pbuf;
	u8 bssid[ETH_ALEN];
	char data[32];
	struct wlan_network *pnetwork = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);	
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct iw_point *pdata = &wrqu->data;	

	RTW_INFO("+rtw_get_aplist_info\n");

	if((padapter->bDriverStopped) || (pdata==NULL))
	{                
		ret= -EINVAL;
		goto exit;
	}		
  
 	while((check_fwstate(pmlmepriv, (WIFI_UNDER_SURVEY|WIFI_UNDER_LINKING))) == _TRUE)
	{	
		rtw_msleep_os(30);
		cnt++;
		if(cnt > 100)
			break;
	}
	

	//pdata->length = 0;//?	
	pdata->flags = 0;
	if(pdata->length>=32)
	{
		if(copy_from_user(data, pdata->pointer, 32))
		{
			ret= -EINVAL;
			goto exit;
		}
	}	
	else
	{
		ret= -EINVAL;
		goto exit;
	}	

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
	
	phead = get_list_head(queue);
	plist = get_next(phead);
       
	while(1)
	{
		if (rtw_end_of_queue_search(phead,plist)== _TRUE)
			break;


		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		//if(hwaddr_aton_i(pdata->pointer, bssid)) 
		if(hwaddr_aton_i(data, bssid)) 
		{			
			RTW_INFO("Invalid BSSID '%s'.\n", (u8*)data);
			_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
			return -EINVAL;
		}		
		
	
		if(_rtw_memcmp(bssid, pnetwork->network.MacAddress, ETH_ALEN) == _TRUE)//BSSID match, then check if supporting wpa/wpa2
		{
			RTW_INFO("BSSID:" MAC_FMT "\n", MAC_ARG(bssid));
			
			pbuf = rtw_get_wpa_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);				
			if(pbuf && (wpa_ielen>0))
			{
				pdata->flags = 1;
				break;
			}

			pbuf = rtw_get_wpa2_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);
			if(pbuf && (wpa_ielen>0))
			{
				pdata->flags = 2;
				break;
			}
			
		}

		plist = get_next(plist);		
	
	}        

	_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

	if(pdata->length>=34)
	{
		if(copy_to_user((u8*)pdata->pointer+32, (u8*)&pdata->flags, 1))
		{
			ret= -EINVAL;
			goto exit;
		}
	}	
	
exit:
	
	return ret;
		
}

static int rtw_set_pid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	struct iw_point *pdata = &wrqu->data;	

	RTW_INFO("+rtw_set_pid\n");

	if((padapter->bDriverStopped) || (pdata==NULL))
	{                
		ret= -EINVAL;
		goto exit;
	}		
  
 	//pdata->length = 0;
	//pdata->flags = 0;

	//_rtw_memcpy(&padapter->pid, pdata->pointer, sizeof(int));
	if(copy_from_user(&padapter->pid, pdata->pointer, sizeof(int)))
	{
		ret= -EINVAL;
		goto exit;
	}

#ifdef CONFIG_SIGNAL_PID_WHILE_DRV_INIT
	ui_pid = padapter->pid;
#endif

	RTW_INFO("got pid=%d\n", padapter->pid);

exit:
	
	return ret;
		
}

static int rtw_wps_start(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	
	int ret = 0;	
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	struct iw_point *pdata = &wrqu->data;
	u32   u32wps_start = 0;
        unsigned int uintRet = 0;

        uintRet = copy_from_user( ( void* ) &u32wps_start, pdata->pointer, 4 );

	if((padapter->bDriverStopped) || (pdata==NULL))
	{                
		ret= -EINVAL;
		goto exit;
	}		

       if ( u32wps_start == 0 )
       {
           u32wps_start = *extra;
       }

       RTW_INFO( "[%s] wps_start = %d\n", __FUNCTION__, u32wps_start );

#ifdef CONFIG_RTW_SW_LED
       if ( u32wps_start == 1 ) // WPS Start
		rtw_led_control(padapter, LED_CTL_START_WPS);
	else if ( u32wps_start == 2 ) // WPS Stop because of wps success
		rtw_led_control(padapter, LED_CTL_STOP_WPS);
	else if ( u32wps_start == 3 ) // WPS Stop because of wps fail
		rtw_led_control(padapter, LED_CTL_STOP_WPS_FAIL);
#endif

exit:
	
	return ret;
		
}
#endif

#ifndef	PLATFORM_FREEBSD
static int wpa_set_param(struct net_device *dev, u8 name, u32 value)
{
	uint ret=0;
	u32 flags;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	
	switch (name){
	case IEEE_PARAM_WPA_ENABLED:

		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X; //802.1x
		
		//ret = ieee80211_wpa_enable(ieee, value);
		
		switch((value)&0xff)
		{
			case 1 : //WPA
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPAPSK; //WPA_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case 2: //WPA2
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPA2PSK; //WPA2_PSK
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
		}
		
		
		break;

	case IEEE_PARAM_TKIP_COUNTERMEASURES:
		//ieee->tkip_countermeasures=value;
		break;

	case IEEE_PARAM_DROP_UNENCRYPTED: 
	{
		/* HACK:
		 *
		 * wpa_supplicant calls set_wpa_enabled when the driver
		 * is loaded and unloaded, regardless of if WPA is being
		 * used.  No other calls are made which can be used to
		 * determine if encryption will be used or not prior to
		 * association being expected.  If encryption is not being
		 * used, drop_unencrypted is set to false, else true -- we
		 * can use this to determine if the CAP_PRIVACY_ON bit should
		 * be set.
		 */
		 
#if 0	 
		struct ieee80211_security sec = {
			.flags = SEC_ENABLED,
			.enabled = value,
		};
 		ieee->drop_unencrypted = value;
		/* We only change SEC_LEVEL for open mode. Others
		 * are set by ipw_wpa_set_encryption.
		 */
		if (!value) {
			sec.flags |= SEC_LEVEL;
			sec.level = SEC_LEVEL_0;
		}
		else {
			sec.flags |= SEC_LEVEL;
			sec.level = SEC_LEVEL_1;
		}
		if (ieee->set_security)
			ieee->set_security(ieee->dev, &sec);
#endif		
		break;

	}
	case IEEE_PARAM_PRIVACY_INVOKED:	
		
		//ieee->privacy_invoked=value;
		
		break;

	case IEEE_PARAM_AUTH_ALGS:
		
		ret = wpa_set_auth_algs(dev, value);
		
		break;

	case IEEE_PARAM_IEEE_802_1X:
		
		//ieee->ieee802_1x=value;		
		
		break;
		
	case IEEE_PARAM_WPAX_SELECT:
		
		// added for WPA2 mixed mode
		//RTW_INFO(KERN_WARNING "------------------------>wpax value = %x\n", value);
		/*
		spin_lock_irqsave(&ieee->wpax_suitlist_lock,flags);
		ieee->wpax_type_set = 1;
		ieee->wpax_type_notify = value;
		spin_unlock_irqrestore(&ieee->wpax_suitlist_lock,flags);
		*/
		
		break;

	default:		


		
		ret = -EOPNOTSUPP;

		
		break;
	
	}

	return ret;
	
}

static int wpa_mlme(struct net_device *dev, u32 command, u32 reason)
{	
	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	switch (command)
	{
		case IEEE_MLME_STA_DEAUTH:

			if(!rtw_set_802_11_disassociate(padapter))
				ret = -1;		
			
			break;

		case IEEE_MLME_STA_DISASSOC:
		
			if(!rtw_set_802_11_disassociate(padapter))
				ret = -1;		
	
			break;

		default:
			ret = -EOPNOTSUPP;
			break;
	}

	return ret;
	
}

static int wpa_supplicant_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	uint ret=0;

	//down(&ieee->wx_sem);	

	if (p->length < sizeof(struct ieee_param) || !p->pointer){
		ret = -EINVAL;
		goto out;
	}
	
	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto out;
	}
	
	if (copy_from_user(param, p->pointer, p->length))
	{
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	switch (param->cmd) {

	case IEEE_CMD_SET_WPA_PARAM:
		ret = wpa_set_param(dev, param->u.wpa_param.name, param->u.wpa_param.value);
		break;

	case IEEE_CMD_SET_WPA_IE:
		//ret = wpa_set_wpa_ie(dev, param, p->length);
		ret =  rtw_set_wpa_ie((_adapter *)rtw_netdev_priv(dev), (char*)param->u.wpa_ie.data, (u16)param->u.wpa_ie.len);
		break;

	case IEEE_CMD_SET_ENCRYPTION:
		ret = wpa_set_encryption(dev, param, p->length);
		break;

	case IEEE_CMD_MLME:
		ret = wpa_mlme(dev, param->u.mlme.command, param->u.mlme.reason_code);
		break;

	default:
		RTW_INFO("Unknown WPA supplicant request: %d\n", param->cmd);
		ret = -EOPNOTSUPP;
		break;
		
	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;

	rtw_mfree((u8 *)param, p->length);
	
out:
	
	//up(&ieee->wx_sem);
	
	return ret;
	
}
#endif

#ifndef PLATFORM_FREEBSD
#ifdef CONFIG_AP_MODE
static int rtw_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	NDIS_802_11_WEP	 *pwep = NULL;
	struct sta_info *psta = NULL, *pbcmc_sta = NULL;	
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	RTW_INFO("%s\n", __FUNCTION__);

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	//sizeof(struct ieee_param) = 64 bytes;
	//if (param_len !=  (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	if (param_len !=  sizeof(struct ieee_param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		if (param->u.crypt.idx >= WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}	
	}
	else 
	{		
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if(!psta)
		{
			//ret = -EINVAL;
			RTW_INFO("rtw_set_encryption(), sta has already been removed or never been added\n");
			goto exit;
		}			
	}

	if (strcmp(param->u.crypt.alg, "none") == 0 && (psta==NULL))
	{
		//todo:clear default encryption keys

		RTW_INFO("clear default encryption keys, keyid=%d\n", param->u.crypt.idx);
		
		goto exit;
	}


	if (strcmp(param->u.crypt.alg, "WEP") == 0 && (psta==NULL))
	{		
		RTW_INFO("r871x_set_encryption, crypt.alg = WEP\n");
		
		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;
					
		RTW_INFO("r871x_set_encryption, wep_key_idx=%d, len=%d\n", wep_key_idx, wep_key_len);

		if((wep_key_idx >= WEP_KEYS) || (wep_key_len<=0))
		{
			ret = -EINVAL;
			goto exit;
		}
			

		if (wep_key_len > 0) 
		{			
		 	wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
		 	pwep =(NDIS_802_11_WEP *)rtw_malloc(wep_total_len);
			if(pwep == NULL){
				RTW_INFO(" r871x_set_encryption: pwep allocate fail !!!\n");
				goto exit;
			}
			
		 	_rtw_memset(pwep, 0, wep_total_len);
		
		 	pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;
			
		}
		
		pwep->KeyIndex = wep_key_idx;

		_rtw_memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if(param->u.crypt.set_tx)
		{
			RTW_INFO("wep, set_tx=1\n");

			psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
			psecuritypriv->ndisencryptstatus = Ndis802_11Encryption1Enabled;
			psecuritypriv->dot11PrivacyAlgrthm=_WEP40_;
			psecuritypriv->dot118021XGrpPrivacy=_WEP40_;
			
			if(pwep->KeyLength==13)
			{
				psecuritypriv->dot11PrivacyAlgrthm=_WEP104_;
				psecuritypriv->dot118021XGrpPrivacy=_WEP104_;
			}

		
			psecuritypriv->dot11PrivacyKeyIndex = wep_key_idx;
			
			_rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;

			rtw_ap_set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx, 1);
		}
		else
		{
			RTW_INFO("wep, set_tx=0\n");
			
			//don't update "psecuritypriv->dot11PrivacyAlgrthm" and 
			//"psecuritypriv->dot11PrivacyKeyIndex=keyid", but can rtw_set_key to cam
					
		      _rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx] = pwep->KeyLength;			

			rtw_ap_set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx, 0);
		}

		goto exit;
		
	}

	
	if(!psta && check_fwstate(pmlmepriv, WIFI_AP_STATE)) // //group key
	{
		if(param->u.crypt.set_tx ==1)
		{
			if(strcmp(param->u.crypt.alg, "WEP") == 0)
			{
				RTW_INFO("%s, set group_key, WEP\n", __FUNCTION__);
				
				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
					
				psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
				if(param->u.crypt.key_len==13)
				{						
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
				}
				
			}
			else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
			{						
				RTW_INFO("%s, set group_key, TKIP\n", __FUNCTION__);
				
				psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				
				//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
				//set mic key
				_rtw_memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx-1].skey, &(param->u.crypt.key[16]), 8);
				_rtw_memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx-1].skey, &(param->u.crypt.key[24]), 8);

				psecuritypriv->busetkipkey = _TRUE;
											
			}
			else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
			{
				RTW_INFO("%s, set group_key, CCMP\n", __FUNCTION__);
			
				psecuritypriv->dot118021XGrpPrivacy = _AES_;

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx-1].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
			}
			else
			{
				RTW_INFO("%s, set group_key, none\n", __FUNCTION__);
				
				psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
			}

			psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

			psecuritypriv->binstallGrpkey = _TRUE;

			psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!
								
			rtw_ap_set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);
			
			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta)
			{
				pbcmc_sta->ieee8021x_blocked = _FALSE;
				pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy			
			}	
						
		}

		goto exit;
		
	}	

	if(psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X && psta) // psk/802_1x
	{
		if(check_fwstate(pmlmepriv, WIFI_AP_STATE))
		{
			if(param->u.crypt.set_tx ==1)
			{ 
				_rtw_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				
				if(strcmp(param->u.crypt.alg, "WEP") == 0)
				{
					RTW_INFO("%s, set pairwise key, WEP\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13)
					{						
						psta->dot118021XPrivacy = _WEP104_;
					}
				}
				else if(strcmp(param->u.crypt.alg, "TKIP") == 0)
				{						
					RTW_INFO("%s, set pairwise key, TKIP\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _TKIP_;
				
					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					_rtw_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
					_rtw_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = _TRUE;
											
				}
				else if(strcmp(param->u.crypt.alg, "CCMP") == 0)
				{

					RTW_INFO("%s, set pairwise key, CCMP\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _AES_;
				}
				else
				{
					RTW_INFO("%s, set pairwise key, none\n", __FUNCTION__);
					
					psta->dot118021XPrivacy = _NO_PRIVACY_;
				}
						
				rtw_ap_set_pairwise_key(padapter, psta);
					
				psta->ieee8021x_blocked = _FALSE;
					
			} else {
				RTW_WARN(FUNC_ADPT_FMT" set group key of "MAC_FMT", not support\n"
					, FUNC_ADPT_ARG(padapter), MAC_ARG(psta->phl_sta->mac_addr));
				goto exit;
			}

		}
				
	}

exit:

	if(pwep)
	{
		rtw_mfree((u8 *)pwep, wep_total_len);		
	}	
	
	return ret;
	
}

static int rtw_set_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	unsigned char *p;
	struct sta_info *psta = NULL;
	unsigned short cap, ht_cap=_FALSE;
	unsigned int ie_len = 0;
	int group_cipher, pairwise_cipher;	
	unsigned char	channel, network_type, supportRate[NDIS_802_11_LENGTH_RATES_EX];
	int supportRateNum = 0;
	unsigned char OUI1[] = {0x00, 0x50, 0xf2,0x01};
	unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};
	unsigned char WMM_PARA_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};	
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;	
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pbss_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;	
	struct sta_priv *pstapriv = &padapter->stapriv;
	unsigned char *ie = pbss_network->IEs;
	unsigned char *pbuf = param->u.bcn_ie.buf;

	/* SSID */
	/* Supported rates */
	/* DS Params */
	/* WLAN_EID_COUNTRY */
	/* ERP Information element */
	/* Extended supported rates */
	/* WPA/WPA2 */
	/* Wi-Fi Wireless Multimedia Extensions */
	/* ht_capab, ht_oper */
	/* WPS IE */


	RTW_INFO("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;


	_rtw_memcpy(&pstapriv->max_num_sta, param->u.bcn_ie.reserved, 2);

	if((pstapriv->max_num_sta>NUM_STA) || (pstapriv->max_num_sta<=0))
		pstapriv->max_num_sta = NUM_STA;
	
	pbss_network->IELength = len-12-2;// 12 = param header, 2:no packed

	if(pbss_network->IELength>MAX_IE_SZ)
		return -ENOMEM;
	

	_rtw_memset(ie, 0, MAX_IE_SZ);
	
	_rtw_memcpy(ie, pbuf, pbss_network->IELength);


	if(pbss_network->InfrastructureMode!=Ndis802_11APMode)
		return -EINVAL;

	pbss_network->Rssi = 0;

	_rtw_memcpy(pbss_network->MacAddress, adapter_mac_addr(padapter), ETH_ALEN);
	
	//beacon interval
	p = rtw_get_beacon_interval_from_ie(ie);//ie + 8;	// 8: TimeStamp, 2: Beacon Interval 2:Capability
	//pbss_network->Configuration.BeaconPeriod = le16_to_cpu(*(unsigned short*)p);
	pbss_network->Configuration.BeaconPeriod = RTW_GET_LE16(p);
	
	//capability
	//cap = *(unsigned short *)rtw_get_capability_from_ie(ie);
	//cap = le16_to_cpu(cap);
	cap = RTW_GET_LE16(ie);

	//SSID
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _SSID_IE_, &ie_len, (pbss_network->IELength -_BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		_rtw_memset(&pbss_network->Ssid, 0, sizeof(NDIS_802_11_SSID));
		_rtw_memcpy(pbss_network->Ssid.Ssid, (p + 2), ie_len);
		pbss_network->Ssid.SsidLength = ie_len;
	}	

	//chnnel
	channel = 0;
	pbss_network->Configuration.Length = 0;
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _DSSET_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
		channel = *(p + 2);

	pbss_network->Configuration.DSConfig = channel;

	
	_rtw_memset(supportRate, 0, NDIS_802_11_LENGTH_RATES_EX);
	// get supported rates
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));	
	if (p !=  NULL) 
	{
		_rtw_memcpy(supportRate, p+2, ie_len);	
		supportRateNum = ie_len;
	}
	
	//get ext_supported rates
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &ie_len, pbss_network->IELength - _BEACON_IE_OFFSET_);	
	if (p !=  NULL)
	{
		_rtw_memcpy(supportRate+supportRateNum, p+2, ie_len);
		supportRateNum += ie_len;
	
	}

	network_type = rtw_check_network_type(supportRate, supportRateNum, channel);

	rtw_set_supported_rate(pbss_network->SupportedRates, network_type, channel);


	//parsing ERP_IE
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		ERP_IE_handler(padapter, (PNDIS_802_11_VARIABLE_IEs)p);
	}

	//parsing HT_CAP_IE
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		u8 rf_type;
		ht_cap = _TRUE;
		network_type |= WIRELESS_11_24N;

		padapter->hal_func.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));

		if(rf_type == RF_1T1R)
		{
			struct rtw_ieee80211_ht_cap *pht_cap = (struct ieee80211_ht_cap *)(p+2);
			
			pht_cap->supp_mcs_set[0] = 0xff;
			pht_cap->supp_mcs_set[1] = 0x0;				
		}			
		
		_rtw_memcpy(&pmlmepriv->htpriv.ht_cap, p+2, ie_len);		
	}

	switch(network_type)
	{
		case WIRELESS_11B:
			pbss_network->NetworkTypeInUse = Ndis802_11DS;
			break;	
		case WIRELESS_11G:
		case WIRELESS_11BG:
             case WIRELESS_11G_24N:
		case WIRELESS_11BG_24N:
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM24;
			break;
		case WIRELESS_11A:
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM5;
			break;
		default :
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM24;
			break;
	}
	
	pmlmepriv->cur_network.network_type = network_type;

	//update privacy/security
	if (cap & BIT(4))
		pbss_network->Privacy = 1;
	else
		pbss_network->Privacy = 0;

	psecuritypriv->wpa_psk = 0;

	//wpa2
	group_cipher = 0; pairwise_cipher = 0;
	psecuritypriv->wpa2_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa2_pairwise_cipher = _NO_PRIVACY_;	
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _RSN_IE_2_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));		
	if(p && ie_len>0)
	{
		if(rtw_parse_wpa2_ie(p, ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			psecuritypriv->dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			
			psecuritypriv->dot8021xalg = 1;//psk,  todo:802.1x
			psecuritypriv->wpa_psk |= BIT(1);

			psecuritypriv->wpa2_group_cipher = group_cipher;
			psecuritypriv->wpa2_pairwise_cipher = pairwise_cipher;
#if 0
			switch(group_cipher)
			{
				case WPA_CIPHER_NONE:				
				psecuritypriv->wpa2_group_cipher = _NO_PRIVACY_;
				break;
				case WPA_CIPHER_WEP40:				
				psecuritypriv->wpa2_group_cipher = _WEP40_;
				break;
				case WPA_CIPHER_TKIP:				
				psecuritypriv->wpa2_group_cipher = _TKIP_;
				break;
				case WPA_CIPHER_CCMP:				
				psecuritypriv->wpa2_group_cipher = _AES_;				
				break;
				case WPA_CIPHER_WEP104:					
				psecuritypriv->wpa2_group_cipher = _WEP104_;
				break;
			}

			switch(pairwise_cipher)
			{
				case WPA_CIPHER_NONE:			
				psecuritypriv->wpa2_pairwise_cipher = _NO_PRIVACY_;
				break;
				case WPA_CIPHER_WEP40:			
				psecuritypriv->wpa2_pairwise_cipher = _WEP40_;
				break;
				case WPA_CIPHER_TKIP:				
				psecuritypriv->wpa2_pairwise_cipher = _TKIP_;
				break;
				case WPA_CIPHER_CCMP:			
				psecuritypriv->wpa2_pairwise_cipher = _AES_;
				break;
				case WPA_CIPHER_WEP104:					
				psecuritypriv->wpa2_pairwise_cipher = _WEP104_;
				break;
			}
#endif			
		}
		
	}

	//wpa
	ie_len = 0;
	group_cipher = 0; pairwise_cipher = 0;
	psecuritypriv->wpa_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa_pairwise_cipher = _NO_PRIVACY_;	
	for (p = ie + _BEACON_IE_OFFSET_; ;p += (ie_len + 2))
	{
		p = rtw_get_ie(p, _SSN_IE_1_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));		
		if ((p) && (_rtw_memcmp(p+2, OUI1, 4)))
		{
			if(rtw_parse_wpa_ie(p, ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
			{
				psecuritypriv->dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
				
				psecuritypriv->dot8021xalg = 1;//psk,  todo:802.1x

				psecuritypriv->wpa_psk |= BIT(0);

				psecuritypriv->wpa_group_cipher = group_cipher;
				psecuritypriv->wpa_pairwise_cipher = pairwise_cipher;

#if 0
				switch(group_cipher)
				{
					case WPA_CIPHER_NONE:					
					psecuritypriv->wpa_group_cipher = _NO_PRIVACY_;
					break;
					case WPA_CIPHER_WEP40:					
					psecuritypriv->wpa_group_cipher = _WEP40_;
					break;
					case WPA_CIPHER_TKIP:					
					psecuritypriv->wpa_group_cipher = _TKIP_;
					break;
					case WPA_CIPHER_CCMP:					
					psecuritypriv->wpa_group_cipher = _AES_;				
					break;
					case WPA_CIPHER_WEP104:					
					psecuritypriv->wpa_group_cipher = _WEP104_;
					break;
				}

				switch(pairwise_cipher)
				{
					case WPA_CIPHER_NONE:					
					psecuritypriv->wpa_pairwise_cipher = _NO_PRIVACY_;
					break;
					case WPA_CIPHER_WEP40:					
					psecuritypriv->wpa_pairwise_cipher = _WEP40_;
					break;
					case WPA_CIPHER_TKIP:					
					psecuritypriv->wpa_pairwise_cipher = _TKIP_;
					break;
					case WPA_CIPHER_CCMP:					
					psecuritypriv->wpa_pairwise_cipher = _AES_;
					break;
					case WPA_CIPHER_WEP104:					
					psecuritypriv->wpa_pairwise_cipher = _WEP104_;
					break;
				}
#endif				
			}

			break;
			
		}
			
		if ((p == NULL) || (ie_len == 0))
		{
				break;
		}
		
	}

	//wmm
	ie_len = 0;
	pmlmepriv->qospriv.qos_option = 0;
	if(pregistrypriv->wmm_enable)
	{
		for (p = ie + _BEACON_IE_OFFSET_; ;p += (ie_len + 2))
		{			
			p = rtw_get_ie(p, _VENDOR_SPECIFIC_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));	
			if((p) && _rtw_memcmp(p+2, WMM_PARA_IE, 6)) 
			{
				pmlmepriv->qospriv.qos_option = 1;	

				*(p+8) |= BIT(7);//QoS Info, support U-APSD
				
				break;				
			}
			
			if ((p == NULL) || (ie_len == 0))
			{
				break;
			}			
		}		
	}

	pmlmepriv->htpriv.ht_option = _FALSE;
#ifdef CONFIG_80211N_HT
	if( (psecuritypriv->wpa2_pairwise_cipher&WPA_CIPHER_TKIP) ||
		      (psecuritypriv->wpa_pairwise_cipher&WPA_CIPHER_TKIP))
	{	
                //todo:
		//ht_cap = _FALSE;
	}
		      
	//ht_cap	
	if((pregistrypriv->ht_enable && ht_cap ==_TRUE) && is_supported_ht(pregistrypriv->wireless_mode))
	{		
		pmlmepriv->htpriv.ht_option = _TRUE;
		pmlmepriv->qospriv.qos_option = 1;

		if(pregistrypriv->ampdu_enable==1)
		{
			pmlmepriv->htpriv.ampdu_enable = _TRUE;
		}
	}
#endif


	pbss_network->Length = get_WLAN_BSSID_EX_sz((WLAN_BSSID_EX  *)pbss_network);

	//issue beacon to start bss network
	//start_bss_network(padapter, (u8*)pbss_network);
	rtw_startbss_cmd(padapter, RTW_CMDF_WAIT_ACK);
			

	//alloc sta_info for ap itself
	psta = rtw_get_stainfo(&padapter->stapriv, pbss_network->MacAddress);
	if(!psta)
	{
#ifdef RTW_WKARD_AP_CMD_DISPATCH
		psta = rtw_alloc_stainfo(&padapter->stapriv, pbss_network->MacAddress, PHL_CMD_DIRECTLY);
#else
		psta = rtw_alloc_stainfo(&padapter->stapriv, pbss_network->MacAddress, PHL_CMD_WAIT);
#endif
		if (psta == NULL) 
		{ 
			return -EINVAL;
		}	
	}	
			
	rtw_indicate_connect( padapter);

	pmlmepriv->cur_network.join_res = _TRUE;//for check if already set beacon
		
	//update bc/mc sta_info
	//update_bmc_sta(padapter);

	return ret;
	
}

static int rtw_hostapd_sta_flush(struct net_device *dev)
{
	//_irqL irqL;
	//_list	*phead, *plist;
	int ret=0;	
	//struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	//struct sta_priv *pstapriv = &padapter->stapriv;

	RTW_INFO("%s\n", __FUNCTION__);

	flush_all_cam_entry(padapter);	//clear CAM

	ret = rtw_sta_flush(padapter);	

	return ret;

}

static int rtw_add_sta(struct net_device *dev, struct ieee_param *param)
{
	_irqL irqL;
	int ret=0;	
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	RTW_INFO("rtw_add_sta(aid=%d)=" MAC_FMT "\n", param->u.add_sta.aid, MAC_ARG(param->sta_addr));
	
	if(check_fwstate(pmlmepriv, (WIFI_ASOC_STATE|WIFI_AP_STATE)) != _TRUE)	
	{
		return -EINVAL;		
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		return -EINVAL;	
	}

/*
	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		RTW_INFO("rtw_add_sta(), free has been added psta=%p\n", psta);
		//_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		
		rtw_free_stainfo(padapter,  psta);		
		//_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);

		psta = NULL;
	}	
*/
	//psta = rtw_alloc_stainfo(pstapriv, param->sta_addr);
	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		int flags = param->u.add_sta.flags;			
		
		//RTW_INFO("rtw_add_sta(), init sta's variables, psta=%p\n", psta);
		
		psta->aid = param->u.add_sta.aid;//aid=1~2007

		_rtw_memcpy(psta->bssrateset, param->u.add_sta.tx_supp_rates, 16);
		
		
		//check wmm cap.
		if(WLAN_STA_WME&flags)
			psta->qos_option = 1;
		else
			psta->qos_option = 0;

		if(pmlmepriv->qospriv.qos_option == 0)	
			psta->qos_option = 0;

		
#ifdef CONFIG_80211N_HT
		if (padapter->registrypriv.ht_enable &&
			is_supported_ht(padapter->registrypriv.wireless_mode)){
			//chec 802.11n ht cap.
			if(WLAN_STA_HT&flags)
			{
				psta->htpriv.ht_option = _TRUE;
				psta->qos_option = 1;
				_rtw_memcpy((void*)&psta->htpriv.ht_cap, (void*)&param->u.add_sta.ht_cap, sizeof(struct rtw_ieee80211_ht_cap));
			}
			else		
			{
				psta->htpriv.ht_option = _FALSE;
			}
			
			if(pmlmepriv->htpriv.ht_option == _FALSE)	
				psta->htpriv.ht_option = _FALSE;
		}
#endif		


		update_sta_info_apmode(padapter, psta);
		
		
	}
	else
	{
		ret = -ENOMEM;
	}	
	
	return ret;
	
}

static int rtw_del_sta(struct net_device *dev, struct ieee_param *param)
{
	_irqL irqL;
	int ret=0;	
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	RTW_INFO("rtw_del_sta=" MAC_FMT "\n", MAC_ARG(param->sta_addr));
		
	if(check_fwstate(pmlmepriv, (WIFI_ASOC_STATE|WIFI_AP_STATE)) != _TRUE)		
	{
		return -EINVAL;		
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		return -EINVAL;	
	}

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		u8 updated;
			
		//RTW_INFO("free psta=%p, aid=%d\n", psta, psta->aid);

		updated = ap_free_sta(padapter, psta, _TRUE, WLAN_REASON_DEAUTH_LEAVING, _FALSE);

		associated_clients_update(padapter, updated, STA_INFO_UPDATE_ALL);

		psta = NULL;
		
	}
	else
	{
		RTW_INFO("rtw_del_sta(), sta has already been removed or never been added\n");
		
		//ret = -1;
	}
	
	
	return ret;
	
}

static int rtw_get_sta_wpaie(struct net_device *dev, struct ieee_param *param)
{
	int ret=0;	
	struct sta_info *psta = NULL;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	RTW_INFO("rtw_get_sta_wpaie, sta_addr: " MAC_FMT "\n", MAC_ARG(param->sta_addr));

	if(check_fwstate(pmlmepriv, (WIFI_ASOC_STATE|WIFI_AP_STATE)) != _TRUE)		
	{
		return -EINVAL;		
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) 
	{
		return -EINVAL;	
	}

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if(psta)
	{
		if((psta->wpa_ie[0] == WLAN_EID_RSN) || (psta->wpa_ie[0] == WLAN_EID_GENERIC))
		{
			int wpa_ie_len;
			int copy_len;

			wpa_ie_len = psta->wpa_ie[1];
			
			copy_len = ((wpa_ie_len+2) > sizeof(psta->wpa_ie)) ? (sizeof(psta->wpa_ie)):(wpa_ie_len+2);
				
			param->u.wpa_ie.len = copy_len;

			_rtw_memcpy(param->u.wpa_ie.reserved, psta->wpa_ie, copy_len);
		}
		else
		{
			//ret = -1;
			RTW_INFO("sta's wpa_ie is NONE\n");
		}		
	}
	else
	{
		ret = -1;
	}

	return ret;

}

static int rtw_set_wps_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	RTW_INFO("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_beacon_ie)
	{
		rtw_mfree(pmlmepriv->wps_beacon_ie, pmlmepriv->wps_beacon_ie_len);
		pmlmepriv->wps_beacon_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_beacon_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_beacon_ie_len = ie_len;
		if ( pmlmepriv->wps_beacon_ie == NULL) {
			RTW_INFO("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}

		_rtw_memcpy(pmlmepriv->wps_beacon_ie, param->u.bcn_ie.buf, ie_len);

		update_beacon(padapter, _VENDOR_SPECIFIC_IE_, wps_oui, _TRUE, 0);
		
	}
	
	
	return ret;		

}

static int rtw_set_wps_probe_resp(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	RTW_INFO("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_probe_resp_ie)
	{
		rtw_mfree(pmlmepriv->wps_probe_resp_ie, pmlmepriv->wps_probe_resp_ie_len);
		pmlmepriv->wps_probe_resp_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_probe_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_probe_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_probe_resp_ie == NULL) {
			RTW_INFO("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
		_rtw_memcpy(pmlmepriv->wps_probe_resp_ie, param->u.bcn_ie.buf, ie_len);		
	}
	
	
	return ret;

}

static int rtw_set_wps_assoc_resp(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	RTW_INFO("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

	ie_len = len-12-2;// 12 = param header, 2:no packed


	if(pmlmepriv->wps_assoc_resp_ie)
	{
		rtw_mfree(pmlmepriv->wps_assoc_resp_ie, pmlmepriv->wps_assoc_resp_ie_len);
		pmlmepriv->wps_assoc_resp_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_assoc_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_assoc_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_assoc_resp_ie == NULL) {
			RTW_INFO("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
		
		_rtw_memcpy(pmlmepriv->wps_assoc_resp_ie, param->u.bcn_ie.buf, ie_len);		
	}
	
	
	return ret;

}

static int rtw_hostapd_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	int ret=0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	//RTW_INFO("%s\n", __FUNCTION__);


	if (padapter->hw_init_completed==_FALSE){
		ret = -EPERM;
		goto out;
	}


	//if (p->length < sizeof(struct ieee_param) || !p->pointer){
	if(!p->pointer){
		ret = -EINVAL;
		goto out;
	}
	
	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto out;
	}
	
	if (copy_from_user(param, p->pointer, p->length))
	{
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	//RTW_INFO("%s, cmd=%d\n", __FUNCTION__, param->cmd);

	switch (param->cmd) 
	{	
		case RTL871X_HOSTAPD_FLUSH:

			ret = rtw_hostapd_sta_flush(dev);

			break;
	
		case RTL871X_HOSTAPD_ADD_STA:	
			
			ret = rtw_add_sta(dev, param);					
			
			break;

		case RTL871X_HOSTAPD_REMOVE_STA:

			ret = rtw_del_sta(dev, param);

			break;
	
		case RTL871X_HOSTAPD_SET_BEACON:

			ret = rtw_set_beacon(dev, param, p->length);

			break;
			
		case RTL871X_SET_ENCRYPTION:

			ret = rtw_set_encryption(dev, param, p->length);
			
			break;
			
		case RTL871X_HOSTAPD_GET_WPAIE_STA:

			ret = rtw_get_sta_wpaie(dev, param);
	
			break;
			
		case RTL871X_HOSTAPD_SET_WPS_BEACON:

			ret = rtw_set_wps_beacon(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_WPS_PROBE_RESP:

			ret = rtw_set_wps_probe_resp(dev, param, p->length);
			
	 		break;
			
		case RTL871X_HOSTAPD_SET_WPS_ASSOC_RESP:

			ret = rtw_set_wps_assoc_resp(dev, param, p->length);
			
	 		break;
			
		default:
			RTW_INFO("Unknown hostapd request: %d\n", param->cmd);
			ret = -EOPNOTSUPP;
			break;
		
	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;


	rtw_mfree((u8 *)param, p->length);
	
out:
		
	return ret;
	
}
#endif // CONFIG_AP_MODE

#else // PLATFORM_FREEBSD

#ifdef CONFIG_AP_MODE

static int rtw_set_beacon(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret=0;
	unsigned char *p;
	struct sta_info *psta = NULL;
	unsigned short cap, ht_cap=_FALSE;
	unsigned int ie_len = 0;
	int group_cipher, pairwise_cipher;	
	unsigned char	channel, network_type, supportRate[NDIS_802_11_LENGTH_RATES_EX];
	int supportRateNum = 0;
	unsigned char OUI1[] = {0x00, 0x50, 0xf2,0x01};
	//unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};
	unsigned char WMM_PARA_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};	
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;	
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	WLAN_BSSID_EX *pbss_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;	
	//struct sta_priv *pstapriv = &padapter->stapriv;
	unsigned char *ie = pbss_network->IEs;
	u16 len;
	//unsigned char *pbuf = param->u.bcn_ie.buf;

	/* SSID */
	/* Supported rates */
	/* DS Params */
	/* WLAN_EID_COUNTRY */
	/* ERP Information element */
	/* Extended supported rates */
	/* WPA/WPA2 */
	/* Wi-Fi Wireless Multimedia Extensions */
	/* ht_capab, ht_oper */
	/* WPS IE */

	len = ireq->i_len;

	RTW_INFO("%s, len=%d\n", __FUNCTION__, len);

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;

#if 0
	_rtw_memcpy(&pstapriv->max_num_sta, param->u.bcn_ie.reserved, 2);

	if((pstapriv->max_num_sta>NUM_STA) || (pstapriv->max_num_sta<=0))
		pstapriv->max_num_sta = NUM_STA;


	pbss_network->IELength = len-12-2;// 12 = param header, 2:no packed
#endif

	pbss_network->IELength = len;

	if(pbss_network->IELength>MAX_IE_SZ)
		return -ENOMEM;
	

	_rtw_memset(ie, 0, MAX_IE_SZ);
	
	//_rtw_memcpy(ie, pbuf, pbss_network->IELength);
	copyin(ireq->i_data, ie, len);


	if(pbss_network->InfrastructureMode!=Ndis802_11APMode)
		return -EINVAL;

	pbss_network->Rssi = 0;

	_rtw_memcpy(pbss_network->MacAddress, adapter_mac_addr(padapter), ETH_ALEN);
	
	//beacon interval
	p = rtw_get_beacon_interval_from_ie(ie);//ie + 8;	// 8: TimeStamp, 2: Beacon Interval 2:Capability
	pbss_network->Configuration.BeaconPeriod = *(unsigned short*)p;
	
	//capability
	cap = *(unsigned short *)rtw_get_capability_from_ie(ie);
	cap = le16_to_cpu(cap);

	//SSID
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _SSID_IE_, &ie_len, (pbss_network->IELength -_BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		_rtw_memset(&pbss_network->Ssid, 0, sizeof(NDIS_802_11_SSID));
		_rtw_memcpy(pbss_network->Ssid.Ssid, (p + 2), ie_len);
		pbss_network->Ssid.SsidLength = ie_len;
	}	

	//chnnel
	channel = 0;
	pbss_network->Configuration.Length = 0;
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _DSSET_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
		channel = *(p + 2);

	pbss_network->Configuration.DSConfig = channel;

	
	_rtw_memset(supportRate, 0, NDIS_802_11_LENGTH_RATES_EX);
	// get supported rates
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _SUPPORTEDRATES_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));	
	if (p !=  NULL) 
	{
		_rtw_memcpy(supportRate, p+2, ie_len);	
		supportRateNum = ie_len;
	}
	
	//get ext_supported rates
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &ie_len, pbss_network->IELength - _BEACON_IE_OFFSET_);	
	if (p !=  NULL)
	{
		_rtw_memcpy(supportRate+supportRateNum, p+2, ie_len);
		supportRateNum += ie_len;
	
	}

	network_type = rtw_check_network_type(supportRate, supportRateNum, channel);

	rtw_set_supported_rate(pbss_network->SupportedRates, network_type, channel);


	//parsing ERP_IE
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		ERP_IE_handler(padapter, (PNDIS_802_11_VARIABLE_IEs)p);
	}

	//parsing HT_CAP_IE
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));
	if(p && ie_len>0)
	{
		u8 rf_type;
		ht_cap = _TRUE;
		network_type |= WIRELESS_11_24N;

		padapter->hal_func.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));

		if(rf_type == RF_1T1R)
		{
			struct rtw_ieee80211_ht_cap *pht_cap = (struct ieee80211_ht_cap *)(p+2);
			
			pht_cap->supp_mcs_set[0] = 0xff;
			pht_cap->supp_mcs_set[1] = 0x0;				
		}			
		
		_rtw_memcpy(&pmlmepriv->htpriv.ht_cap, p+2, ie_len);		
	}

	switch(network_type)
	{
		case WIRELESS_11B:
			pbss_network->NetworkTypeInUse = Ndis802_11DS;
			break;	
		case WIRELESS_11G:
		case WIRELESS_11BG:
             case WIRELESS_11G_24N:
		case WIRELESS_11BG_24N:
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM24;
			break;
		case WIRELESS_11A:
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM5;
			break;
		default :
			pbss_network->NetworkTypeInUse = Ndis802_11OFDM24;
			break;
	}
	
	pmlmepriv->cur_network.network_type = network_type;

	//update privacy/security
	if (cap & BIT(4))
		pbss_network->Privacy = 1;
	else
		pbss_network->Privacy = 0;

	psecuritypriv->wpa_psk = 0;

	//wpa2
	group_cipher = 0; pairwise_cipher = 0;
	psecuritypriv->wpa2_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa2_pairwise_cipher = _NO_PRIVACY_;	
	p = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _RSN_IE_2_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_));		
	if(p && ie_len>0)
	{
		if(rtw_parse_wpa2_ie(p, ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
		{
			psecuritypriv->dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			
			psecuritypriv->dot8021xalg = 1;//psk,  todo:802.1x
			psecuritypriv->wpa_psk |= BIT(1);

			psecuritypriv->wpa2_group_cipher = group_cipher;
			psecuritypriv->wpa2_pairwise_cipher = pairwise_cipher;
#if 0
			switch(group_cipher)
			{
				case WPA_CIPHER_NONE:				
				psecuritypriv->wpa2_group_cipher = _NO_PRIVACY_;
				break;
				case WPA_CIPHER_WEP40:				
				psecuritypriv->wpa2_group_cipher = _WEP40_;
				break;
				case WPA_CIPHER_TKIP:				
				psecuritypriv->wpa2_group_cipher = _TKIP_;
				break;
				case WPA_CIPHER_CCMP:				
				psecuritypriv->wpa2_group_cipher = _AES_;				
				break;
				case WPA_CIPHER_WEP104:					
				psecuritypriv->wpa2_group_cipher = _WEP104_;
				break;
			}

			switch(pairwise_cipher)
			{
				case WPA_CIPHER_NONE:			
				psecuritypriv->wpa2_pairwise_cipher = _NO_PRIVACY_;
				break;
				case WPA_CIPHER_WEP40:			
				psecuritypriv->wpa2_pairwise_cipher = _WEP40_;
				break;
				case WPA_CIPHER_TKIP:				
				psecuritypriv->wpa2_pairwise_cipher = _TKIP_;
				break;
				case WPA_CIPHER_CCMP:			
				psecuritypriv->wpa2_pairwise_cipher = _AES_;
				break;
				case WPA_CIPHER_WEP104:					
				psecuritypriv->wpa2_pairwise_cipher = _WEP104_;
				break;
			}
#endif			
		}
		
	}

	//wpa
	ie_len = 0;
	group_cipher = 0; pairwise_cipher = 0;
	psecuritypriv->wpa_group_cipher = _NO_PRIVACY_;
	psecuritypriv->wpa_pairwise_cipher = _NO_PRIVACY_;	
	for (p = ie + _BEACON_IE_OFFSET_; ;p += (ie_len + 2))
	{
		p = rtw_get_ie(p, _SSN_IE_1_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));		
		if ((p) && (_rtw_memcmp(p+2, OUI1, 4)))
		{
			if(rtw_parse_wpa_ie(p, ie_len+2, &group_cipher, &pairwise_cipher) == _SUCCESS)
			{
				psecuritypriv->dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
				
				psecuritypriv->dot8021xalg = 1;//psk,  todo:802.1x

				psecuritypriv->wpa_psk |= BIT(0);

				psecuritypriv->wpa_group_cipher = group_cipher;
				psecuritypriv->wpa_pairwise_cipher = pairwise_cipher;

#if 0
				switch(group_cipher)
				{
					case WPA_CIPHER_NONE:					
					psecuritypriv->wpa_group_cipher = _NO_PRIVACY_;
					break;
					case WPA_CIPHER_WEP40:					
					psecuritypriv->wpa_group_cipher = _WEP40_;
					break;
					case WPA_CIPHER_TKIP:					
					psecuritypriv->wpa_group_cipher = _TKIP_;
					break;
					case WPA_CIPHER_CCMP:					
					psecuritypriv->wpa_group_cipher = _AES_;				
					break;
					case WPA_CIPHER_WEP104:					
					psecuritypriv->wpa_group_cipher = _WEP104_;
					break;
				}

				switch(pairwise_cipher)
				{
					case WPA_CIPHER_NONE:					
					psecuritypriv->wpa_pairwise_cipher = _NO_PRIVACY_;
					break;
					case WPA_CIPHER_WEP40:					
					psecuritypriv->wpa_pairwise_cipher = _WEP40_;
					break;
					case WPA_CIPHER_TKIP:					
					psecuritypriv->wpa_pairwise_cipher = _TKIP_;
					break;
					case WPA_CIPHER_CCMP:					
					psecuritypriv->wpa_pairwise_cipher = _AES_;
					break;
					case WPA_CIPHER_WEP104:					
					psecuritypriv->wpa_pairwise_cipher = _WEP104_;
					break;
				}
#endif
			}

			break;
			
		}
			
		if ((p == NULL) || (ie_len == 0))
		{
				break;
		}
		
	}

	//wmm
	ie_len = 0;
	pmlmepriv->qospriv.qos_option = 0;
	if(pregistrypriv->wmm_enable)
	{
		for (p = ie + _BEACON_IE_OFFSET_; ;p += (ie_len + 2))
		{			
			p = rtw_get_ie(p, _VENDOR_SPECIFIC_IE_, &ie_len, (pbss_network->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));	
			if((p) && _rtw_memcmp(p+2, WMM_PARA_IE, 6)) 
			{
				pmlmepriv->qospriv.qos_option = 1;	

				*(p+8) |= BIT(7);//QoS Info, support U-APSD
				
				break;				
			}
			
			if ((p == NULL) || (ie_len == 0))
			{
				break;
			}			
		}		
	}

	pmlmepriv->htpriv.ht_option = _FALSE;
#ifdef CONFIG_80211N_HT
	if( (psecuritypriv->wpa2_pairwise_cipher&WPA_CIPHER_TKIP) ||
		      (psecuritypriv->wpa_pairwise_cipher&WPA_CIPHER_TKIP))
	{	
                //todo:
		//ht_cap = _FALSE;
	}
		      
	//ht_cap	
	if(pregistrypriv->ht_enable && ht_cap ==_TRUE && is_supported_ht(pregistrypriv->wireless_mode))
	{		
		pmlmepriv->htpriv.ht_option = _TRUE;
		pmlmepriv->qospriv.qos_option = 1;

		if(pregistrypriv->ampdu_enable==1)
		{
			pmlmepriv->htpriv.ampdu_enable = _TRUE;
		}
	}
#endif


	pbss_network->Length = get_WLAN_BSSID_EX_sz((WLAN_BSSID_EX  *)pbss_network);

	rtw_startbss_cmd(padapter, RTW_CMDF_WAIT_ACK);
			

	//alloc sta_info for ap itself
	psta = rtw_get_stainfo(&padapter->stapriv, pbss_network->MacAddress);
	if(!psta)
	{
#ifdef RTW_WKARD_AP_CMD_DISPATCH
		psta = rtw_alloc_stainfo(&padapter->stapriv, pbss_network->MacAddress, PHL_CMD_DIRECTLY);
#else
		psta = rtw_alloc_stainfo(&padapter->stapriv, pbss_network->MacAddress, PHL_CMD_WAIT);
#endif
		if (psta == NULL) 
		{ 
			return -EINVAL;
		}	
	}	
			
	rtw_indicate_connect( padapter);

	pmlmepriv->cur_network.join_res = _TRUE;//for check if already set beacon
		
	//update bc/mc sta_info
	//update_bmc_sta(padapter);

	return ret;
	
}

static int rtw_del_sta(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret=0;	
	struct sta_info *psta = NULL;	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct ieee80211req_mlme *mlme = (struct ieee80211req_mlme *)ireq->i_data;

	//RTW_INFO("rtw_del_sta=" MACSTR "\n", MAC2STR(mlme->im_macaddr));

	if(check_fwstate(pmlmepriv, (WIFI_ASOC_STATE|WIFI_AP_STATE)) != _TRUE)		
	{
		return -EINVAL;		
	}
	

	if (mlme->im_macaddr[0] == 0xff && mlme->im_macaddr[1] == 0xff &&
	    mlme->im_macaddr[2] == 0xff && mlme->im_macaddr[3] == 0xff &&
	    mlme->im_macaddr[4] == 0xff && mlme->im_macaddr[5] == 0xff) 
	{
		//rtw_hostapd_sta_flush(padapter);
		//RTW_INFO("sta_flush, free all sta\n");

		flush_all_cam_entry(padapter);	//clear CAM

		ret = rtw_sta_flush(padapter, _TRUE, _FALSE);
		
		return ret;
		//return -EINVAL;	
	}

	if(IEEE80211_MLME_DEAUTH == mlme->im_op ||
		IEEE80211_MLME_DISASSOC == mlme->im_op)
	{		
		psta = rtw_get_stainfo(pstapriv, mlme->im_macaddr);
		if(psta)
		{
			u8 updated;
		
			RTW_INFO("free psta=%p, aid=%d\n", psta, psta->aid);

			updated = ap_free_sta(padapter, psta, _TRUE, WLAN_REASON_DEAUTH_LEAVING, _TRUE, _FALSE);

			associated_clients_update(padapter, updated, STA_INFO_UPDATE_ALL);


			psta = NULL;
		
		}
		else
		{
			//RTW_INFO("rtw_del_sta(), sta has already been removed or never been added\n");		 
		}
	}

	return ret;
	
}

#if 0
 struct ieee80211req {
         char            i_name[IFNAMSIZ];       /* if_name, e.g. "wi0" */
         uint16_t        i_type;                 /* req type */
         int16_t         i_val;                  /* Index or simple value */
         uint16_t        i_len;                  /* Index or simple value */
         void            *i_data;                /* Extra data */
 };

struct ieee80211req_wpaie {     /* old version w/ only one ie */
         uint8_t         wpa_macaddr[IEEE80211_ADDR_LEN];
         uint8_t         wpa_ie[IEEE80211_MAX_OPT_IE];
 };

#endif

static int rtw_get_sta_wpaie(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret=0;
	struct ieee80211req_wpaie *param=NULL;
	struct sta_info *psta = NULL;	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(check_fwstate(pmlmepriv, (WIFI_ASOC_STATE|WIFI_AP_STATE)) != _TRUE)		
	{
		return -EINVAL;		
	}
	
	if(ireq->i_len && ireq->i_data)
		param = (struct ieee80211req_wpaie *)rtw_malloc(ireq->i_len);

	if(param == NULL)
		return -ENOMEM;

	copyin(ireq->i_data, param, ireq->i_len);	
		
	//RTW_INFO("rtw_get_sta_wpaie, sta_addr: " MACSTR "\n", MAC2STR(param->wpa_macaddr));

	
	if (param->wpa_macaddr[0] == 0xff && param->wpa_macaddr[1] == 0xff &&
	    param->wpa_macaddr[2] == 0xff && param->wpa_macaddr[3] == 0xff &&
	    param->wpa_macaddr[4] == 0xff && param->wpa_macaddr[5] == 0xff) 
	{		
		ret = -EINVAL;
		goto _exit;		
	}

	psta = rtw_get_stainfo(pstapriv, param->wpa_macaddr);
	if(psta)
	{
		if((psta->wpa_ie[0] == WLAN_EID_RSN) || (psta->wpa_ie[0] == WLAN_EID_GENERIC))
		{
			int wpa_ie_len;
			int copy_len;

			wpa_ie_len = psta->wpa_ie[1];
			
			copy_len = ((wpa_ie_len+2) > sizeof(psta->wpa_ie)) ? (sizeof(psta->wpa_ie)):(wpa_ie_len+2);
				
			_rtw_memcpy(param->wpa_ie, psta->wpa_ie, copy_len);

			copyout((u8*)param, (u8 *)ireq->i_data, ireq->i_len);
			
		}
		else
		{			
			RTW_INFO("sta's wpa_ie is NONE\n");
		}		
	}
	else
	{
		RTW_INFO("%s, can't get stainfo\n", __FUNCTION__);
	}

_exit:

	if(param)
		rtw_mfree((u8*)param, ireq->i_len);
	
	return ret;

}

static int rtw_set_wps_beacon(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret=0;
	unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	unsigned short ie_len = (unsigned short)ireq->i_len;
	u8 *pie = (u8 *)ireq->i_data;


	RTW_INFO("%s, len=%d\n", __FUNCTION__, ie_len);


	if((ie_len > MAX_WPS_IE_LEN) || (pie == NULL)){
		return -EINVAL;
	}	

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;


	if(pmlmepriv->wps_beacon_ie)
	{
		rtw_mfree(pmlmepriv->wps_beacon_ie, pmlmepriv->wps_beacon_ie_len);
		pmlmepriv->wps_beacon_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_beacon_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_beacon_ie_len = ie_len;
		if ( pmlmepriv->wps_beacon_ie == NULL) {
			RTW_INFO("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
		
		copyin(pie, pmlmepriv->wps_beacon_ie, ie_len);

		update_beacon(padapter, _VENDOR_SPECIFIC_IE_, wps_oui, _TRUE, 0);
		
		pmlmeext->bstart_bss = _TRUE;
		
	}
	
	
	return ret;		

}

static int rtw_set_wps_probe_resp(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret=0;	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	unsigned short ie_len = (unsigned short)ireq->i_len;
	u8 *pie = (u8 *)ireq->i_data;


	RTW_INFO("%s, len=%d\n", __FUNCTION__, ie_len);


	if((ie_len > MAX_WPS_IE_LEN) || (pie == NULL)){
		return -EINVAL;
	}	

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;


	if(pmlmepriv->wps_probe_resp_ie)
	{
		rtw_mfree(pmlmepriv->wps_probe_resp_ie, pmlmepriv->wps_probe_resp_ie_len);
		pmlmepriv->wps_probe_resp_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_probe_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_probe_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_probe_resp_ie == NULL) {
			RTW_INFO("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
			
		copyin(pie, pmlmepriv->wps_probe_resp_ie, ie_len);
	}
	
	
	return ret;

}

static int rtw_set_wps_assoc_resp(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int ret=0;	
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	unsigned short ie_len = (unsigned short)ireq->i_len;
	u8 *pie = (u8 *)ireq->i_data;


	RTW_INFO("%s, len=%d\n", __FUNCTION__, ie_len);


	if((ie_len > MAX_WPS_IE_LEN) || (pie == NULL)){
		return -EINVAL;
	}	

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE) != _TRUE)
		return -EINVAL;



	if(pmlmepriv->wps_assoc_resp_ie)
	{
		rtw_mfree(pmlmepriv->wps_assoc_resp_ie, pmlmepriv->wps_assoc_resp_ie_len);
		pmlmepriv->wps_assoc_resp_ie = NULL;			
	}	

	if(ie_len>0)
	{
		pmlmepriv->wps_assoc_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_assoc_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_assoc_resp_ie == NULL) {
			RTW_INFO("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
				
		copyin(pie, pmlmepriv->wps_assoc_resp_ie, ie_len);
	}
	
	
	return ret;

}


#endif // CONFIG_AP_MODE

#endif // PLATFORM_FREEBSD

#ifndef	PLATFORM_FREEBSD
static int rtw_wx_set_priv(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *awrq,
				char *extra)
{

#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	char *ext_dbg;
#endif

	int ret = 0;
	int len = 0;
	char *ext;

	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iw_point *dwrq = (struct iw_point*)awrq;


	len = dwrq->length;
	if (!(ext = rtw_vmalloc(len)))
		return -ENOMEM;

	if (copy_from_user(ext, dwrq->pointer, len)) {
		rtw_vmfree(ext, len);
		return -EFAULT;
	}



	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV	
	if (!(ext_dbg = rtw_vmalloc(len)))
	{
		rtw_vmfree(ext, len);
		return -ENOMEM;
	}	
	
	_rtw_memcpy(ext_dbg, ext, len);
	#endif

	//added for wps2.0 @20110524
	if(dwrq->flags == 0x8766 && len > 8)
	{
		u32 cp_sz;		
		struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
		u8 *probereq_wpsie = ext;
		int probereq_wpsie_len = len;
		u8 wps_oui[4]={0x0,0x50,0xf2,0x04};		
	
		if((_VENDOR_SPECIFIC_IE_ == probereq_wpsie[0]) &&
			(_rtw_memcmp(&probereq_wpsie[2], wps_oui, 4) ==_TRUE))
		{

			cp_sz = probereq_wpsie_len>MAX_WPS_IE_LEN ? MAX_WPS_IE_LEN:probereq_wpsie_len;

			//_rtw_memcpy(pmlmepriv->probereq_wpsie, probereq_wpsie, cp_sz);
			//pmlmepriv->probereq_wpsie_len = cp_sz;
					
			printk("probe_req_wps_ielen=%d\n", cp_sz);
						
			if(pmlmepriv->wps_probe_req_ie)
			{
				u32 free_len = pmlmepriv->wps_probe_req_ie_len;
				pmlmepriv->wps_probe_req_ie_len = 0;
				rtw_mfree(pmlmepriv->wps_probe_req_ie, free_len);
				pmlmepriv->wps_probe_req_ie = NULL;			
			}	

			pmlmepriv->wps_probe_req_ie = rtw_malloc(cp_sz);
			if ( pmlmepriv->wps_probe_req_ie == NULL) {
				printk("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
				ret =  -EINVAL;
				goto FREE_EXT;
			
			}
			
			_rtw_memcpy(pmlmepriv->wps_probe_req_ie, probereq_wpsie, cp_sz);
			pmlmepriv->wps_probe_req_ie_len = cp_sz;
			
		}	
		
		goto FREE_EXT;
		
	}

FREE_EXT:

	rtw_vmfree(ext, len);
	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	rtw_vmfree(ext_dbg, len);
	#endif

	//RTW_INFO("rtw_wx_set_priv: (SIOCSIWPRIV) %s ret=%d\n", 
	//		dev->name, ret);

	return ret;
	
}
#endif

#if defined(CONFIG_MP_INCLUDED) && defined(CONFIG_MP_IWPRIV_SUPPORT)

/*
 * Input Format: %s,%d,%d
 *	%s is width, could be
 *		"b" for 1 byte
 *		"w" for WORD (2 bytes)
 *		"dw" for DWORD (4 bytes)
 *	1st %d is address(offset)
 *	2st %d is data to write
 */
static int rtw_mp_write_reg(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	char *pch, *pnext, *ptmp;
	char *width_str;
	char width;
	u32 addr, data;
	int ret;
	PADAPTER padapter = rtw_netdev_priv(dev);


	pch = extra;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) return -EINVAL;
	*pnext = 0;
	width_str = pch;

	pch = pnext + 1;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) return -EINVAL;
	*pnext = 0;
	addr = simple_strtoul(pch, &ptmp, 16);
	if (addr > 0x3FFF) return -EINVAL;

	pch = pnext + 1;
	if ((pch - extra) >= wrqu->data.length) return -EINVAL;
	data = simple_strtoul(pch, &ptmp, 16);

	ret = 0;
	width = width_str[0];
	switch (width) {
		case 'b':
			// 1 byte
			if (data > 0xFF) {
				ret = -EINVAL;
				break;
			}
			rtw_write8(padapter, addr, data);
			break;
		case 'w':
			// 2 bytes
			if (data > 0xFFFF) {
				ret = -EINVAL;
				break;
			}
			rtw_write16(padapter, addr, data);
			break;
		case 'd':
			// 4 bytes
			rtw_write32(padapter, addr, data);
			break;
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

/*
 * Input Format: %s,%d
 *	%s is width, could be
 *		"b" for 1 byte
 *		"w" for WORD (2 bytes)
 *		"dw" for DWORD (4 bytes)
 *	%d is address(offset)
 *
 * Return:
 *	%d for data readed
 */
static int rtw_mp_read_reg(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	char input[128];
	char *pch, *pnext, *ptmp;
	char *width_str;
	char width;
	u32 addr;
	u32 *data = (u32*)extra;
	int ret;
	PADAPTER padapter = rtw_netdev_priv(dev);


	if (wrqu->data.length > 128) return -EFAULT;
	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	pch = input;
	pnext = strpbrk(pch, " ,.-");
	if (pnext == NULL) return -EINVAL;
	*pnext = 0;
	width_str = pch;

	pch = pnext + 1;
	if ((pch - input) >= wrqu->data.length) return -EINVAL;
	addr = simple_strtoul(pch, &ptmp, 16);
	if (addr > 0x3FFF) return -EINVAL;

	ret = 0;
	width = width_str[0];
	switch (width) {
		case 'b':
			// 1 byte
			*(u8*)data = rtw_read8(padapter, addr);
			wrqu->data.length = 1;
			break;
		case 'w':
			// 2 bytes
			*(u16*)data = rtw_read16(padapter, addr);
			wrqu->data.length = 2;
			break;
		case 'd':
			// 4 bytes
			*data = rtw_read32(padapter, addr);
			wrqu->data.length = 4;
			break;
		default:
			wrqu->data.length = 0;
			ret = -EINVAL;
			break;
	}

	return ret;
}

/*
 * Input Format: %d,%x,%x
 *	%d is RF path, should be smaller than MAX_RF_PATH_NUMS
 *	1st %x is address(offset)
 *	2st %x is data to write
 */
static int rtw_mp_write_rf(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 path, addr, data;
	int ret;
	PADAPTER padapter = rtw_netdev_priv(dev);


	ret = sscanf(extra, "%d,%x,%x", &path, &addr, &data);
	if (ret < 3) return -EINVAL;

	if (path >= MAX_RF_PATH_NUMS) return -EINVAL;
	if (addr > 0xFF) return -EINVAL;
	if (data > 0xFFFFF) return -EINVAL;

	write_rfreg(padapter, path, addr, data);

	return 0;
}

/*
 * Input Format: %d,%x
 *	%d is RF path, should be smaller than MAX_RF_PATH_NUMS
 *	%x is address(offset)
 *
 * Return:
 *	%d for data readed
 */
static int rtw_mp_read_rf(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	char input[128];
	u32 path, addr;
	u32 *data = (u32*)extra;
	int ret;
	PADAPTER padapter = rtw_netdev_priv(dev);


	if (wrqu->data.length > 128) return -EFAULT;
	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	ret = sscanf(input, "%d,%x", &path, &addr);
	if (ret < 2) return -EINVAL;

	if (path >= MAX_RF_PATH_NUMS) return -EINVAL;
	if (addr > 0xFF) return -EINVAL;

	*data = read_rfreg(padapter, path, addr);
	wrqu->data.length = 4;

	return 0;
}

static int rtw_mp_start(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 val8;
	PADAPTER padapter = rtw_netdev_priv(dev);


	if (padapter->registrypriv.mp_mode == 0)
		return -EPERM;

	if (padapter->mppriv.mode == MP_OFF) {
		if (mp_start_test(padapter) == _FAIL)
			return -EPERM;
		padapter->mppriv.mode = MP_ON;
	}

	return 0;
}

static int rtw_mp_stop(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);


	if (padapter->mppriv.mode != MP_OFF) {
		mp_stop_test(padapter);
		padapter->mppriv.mode = MP_OFF;
	}

	return 0;
}

extern int wifirate2_ratetbl_inx(unsigned char rate);

static int rtw_mp_rate(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 rate = MPT_RATE_1M;
	PADAPTER padapter = rtw_netdev_priv(dev);

	rate = *(u32*)extra;
		
	if(rate <= 0x7f)
		rate = wifirate2_ratetbl_inx( (u8)rate);	
	else if (rate < 0x90)
        //HT  rate 0x80(MCS0)        ~ 0x8F(MCS15)       128~143
		rate =(rate - 0x80 + MPT_RATE_MCS0);
	else
		//VHT rate 0x90(VHT1SS_MCS0) ~ 0x99(VHT1SS_MCS9) 144~153
		rate =(rate - MPT_RATE_VHT1SS_MCS0); 

	//RTW_INFO("%s: rate=%d\n", __func__, rate);
	
	if (rate >= MPT_RATE_LAST )	
	return -EINVAL;

	padapter->mppriv.rateidx = rate;
	SetDataRate(padapter);

	return 0;
}

static int rtw_mp_channel(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 channel = 1;
	int cur_ch_offset;
	PADAPTER padapter = rtw_netdev_priv(dev);

	channel = *(u32*)extra;
	//RTW_INFO("%s: channel=%d\n", __func__, channel);
	
	//if (channel > 14)
	//	return -EINVAL;

	padapter->mppriv.channel = channel;

	cur_ch_offset =  rtw_get_offset_by_ch(padapter->mppriv.channel);
	set_channel_bwmode(padapter, padapter->mppriv.channel, cur_ch_offset, padapter->mppriv.bandwidth);
	//SetChannel(padapter);

	return 0;
}

static int rtw_mp_bandwidth(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 bandwidth=0, sg=0;
	int cur_ch_offset;
	u8 buffer[40];
	PADAPTER padapter = rtw_netdev_priv(dev);

	if (copy_from_user(buffer, (void*)wrqu->data.pointer, wrqu->data.length))
                return -EFAULT;
	//RTW_INFO("%s:iwpriv in=%s\n", __func__, extra);
	
	sscanf(buffer, "40M=%d,shortGI=%d", &bandwidth, &sg);
	
	if (bandwidth != CHANNEL_WIDTH_40)
		bandwidth = CHANNEL_WIDTH_20;

	//RTW_INFO("%s: bw=%d sg=%d \n", __func__, bandwidth , sg);
	padapter->mppriv.bandwidth = (u8)bandwidth;
	padapter->mppriv.preamble = sg;
	
	//SetBandwidth(padapter);
	cur_ch_offset =  rtw_get_offset_by_ch(padapter->mppriv.channel);
	set_channel_bwmode(padapter, padapter->mppriv.channel, cur_ch_offset, bandwidth);

	return 0;
}

static int rtw_mp_txpower(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 buffer[40];
	u32 idx_a,idx_b;


	PADAPTER padapter = rtw_netdev_priv(dev);
	if (copy_from_user(buffer, (void*)wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	sscanf(buffer,"patha=%d,pathb=%d",&idx_a,&idx_b);
	//RTW_INFO("%s: tx_pwr_idx_a=%x b=%x\n", __func__, idx_a, idx_b);

	padapter->mppriv.txpoweridx = (u8)idx_a;
	padapter->mppriv.txpoweridx_b = (u8)idx_b;
	
	SetAntennaPathPower(padapter);
	
	return 0;
}

static int rtw_mp_ant_tx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 i;
	u16 antenna = 0;
	PADAPTER padapter = rtw_netdev_priv(dev);

	//RTW_INFO("%s: extra=%s\n", __func__, extra);
	for (i=0; i < (wrqu->data.length-1); i++){
		switch(extra[i])
			{
				case 'a' :
								antenna|=ANTENNA_A;
								break;
				case 'b':
								antenna|=ANTENNA_B;
								break;
			}
	}
	//antenna |= BIT(extra[i]-'a');

	//RTW_INFO("%s: antenna=0x%x\n", __func__, antenna);		
	padapter->mppriv.antenna_tx = antenna;
	//RTW_INFO("%s:mppriv.antenna_rx=%d\n", __func__, padapter->mppriv.antenna_tx);
	
	SetAntenna(padapter);
	return 0;
}

static int rtw_mp_ant_rx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 i;
	u16 antenna = 0;
	u8 buffer[16];
	PADAPTER padapter = rtw_netdev_priv(dev);

	if (copy_from_user(buffer, (void*)wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	//RTW_INFO("%s: extra=%s\n", __func__, buffer);

	for (i=0; i < (wrqu->data.length-1); i++) {
		switch(extra[i])
			{
				case 'a' :
								antenna|=ANTENNA_A;
								break;
				case 'b':
								antenna|=ANTENNA_B;
								break;
			}
	}
	
	//RTW_INFO("%s: antenna=0x%x\n", __func__, antenna);		
	padapter->mppriv.antenna_rx = antenna;
	//RTW_INFO("%s:mppriv.antenna_rx=%d\n", __func__, padapter->mppriv.antenna_rx);

	SetAntenna(padapter);
	return 0;
}

static int rtw_mp_ctx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 pkTx = 1, countPkTx = 1, cotuTx = 1, CarrSprTx = 1, scTx = 1, sgleTx = 1, stop = 1;
	u32 bStartTest = 1;
	u32 count = 0;
	u8 buffer[40];
	struct mp_priv *pmp_priv;
	struct pkt_attrib *pattrib;

	PADAPTER padapter = rtw_netdev_priv(dev);


	pmp_priv = &padapter->mppriv;

	if (copy_from_user(buffer, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	RTW_INFO("%s: in=%s\n", __func__, buffer);

	countPkTx = strncmp(buffer, "count=", 5); // strncmp TRUE is 0
	cotuTx = strncmp(buffer, "background", 20);
	CarrSprTx = strncmp(buffer, "background,cs", 20);
	scTx = strncmp(buffer, "background,sc", 20);
	sgleTx = strncmp(buffer, "background,stone", 20);
	pkTx = strncmp(buffer, "background,pkt", 20);
	stop = strncmp(buffer, "stop", 5);
	sscanf(buffer, "count=%d,pkt", &count);
	//RTW_INFO("%s: count=%d countPkTx=%d cotuTx=%d CarrSprTx=%d scTx=%d sgleTx=%d pkTx=%d stop=%d\n", __func__, count, countPkTx, cotuTx, CarrSprTx, pkTx, sgleTx, scTx, stop);

	if (stop == 0) {
		bStartTest = 0; // To set Stop
		pmp_priv->tx.stop = 1;
	} else {
		bStartTest = 1;
		if (pmp_priv->mode != MP_ON) {
			if (pmp_priv->tx.stop != 1) {
				RTW_INFO("%s: MP_MODE != ON %d\n", __func__, pmp_priv->mode);
				return  -EFAULT;
			}
		}
	}

	if (pkTx == 0 || countPkTx == 0)
		pmp_priv->mode = MP_PACKET_TX;
	if (sgleTx == 0)
		pmp_priv->mode = MP_SINGLE_TONE_TX;
	if (cotuTx == 0)
		pmp_priv->mode = MP_CONTINUOUS_TX;
	if (CarrSprTx == 0)
		pmp_priv->mode = MP_CARRIER_SUPPRISSION_TX;
	if (scTx == 0)
		pmp_priv->mode = MP_SINGLE_CARRIER_TX;

	switch (pmp_priv->mode)
	{
		case MP_PACKET_TX:
			//RTW_INFO("%s:pkTx %d\n", __func__,bStartTest);
			if (bStartTest == 0) {
				pmp_priv->tx.stop = 1;
				pmp_priv->mode = MP_ON;
			} else if (pmp_priv->tx.stop == 1) {
				//RTW_INFO("%s:countPkTx %d\n", __func__,count);
				pmp_priv->tx.stop = 0;
				pmp_priv->tx.count = count;
				pmp_priv->tx.payload = 2;
#ifdef CONFIG_80211N_HT
			if (padapter->registrypriv.ht_enable && is_supported_ht(padapter->registrypriv.wireless_mode))
				pmp_priv->tx.attrib.ht_en = 1;
#endif
#ifdef CONFIG_80211AC_VHT
				pmp_priv->tx.attrib.raid = RATEID_IDX_VHT_1SS; //10
#endif
				pattrib = &pmp_priv->tx.attrib;
				pattrib->pktlen = 1000;
				_rtw_memset(pattrib->dst, 0xFF, ETH_ALEN);
				SetPacketTx(padapter);
			} else {
				//RTW_INFO("%s: pkTx not stop\n", __func__);
				return -EFAULT;
			}
			return 0;

		case MP_SINGLE_TONE_TX:
			//RTW_INFO("%s: sgleTx %d \n", __func__, bStartTest);
			SetSingleToneTx(padapter, (u8)bStartTest);
			break;

		case MP_CONTINUOUS_TX:
			//RTW_INFO("%s: cotuTx %d\n", __func__, bStartTest);
			SetContinuousTx(padapter, (u8)bStartTest);
			break;

		case MP_CARRIER_SUPPRISSION_TX:
			//RTW_INFO("%s: CarrSprTx %d\n", __func__, bStartTest);
			SetCarrierSuppressionTx(padapter, (u8)bStartTest);
			break;

		case MP_SINGLE_CARRIER_TX:
			//RTW_INFO("%s: scTx %d\n", __func__, bStartTest);
			SetSingleCarrierTx(padapter, (u8)bStartTest);
			break;

		default:
			//RTW_INFO("%s:No Match MP_MODE\n", __func__);
			return -EFAULT;
	}

	if (bStartTest) {
		struct mp_priv *pmp_priv = &padapter->mppriv;
		if (pmp_priv->tx.stop == 0) {
			pmp_priv->tx.stop = 1;
			//RTW_INFO("%s: pkt tx is running...\n", __func__);
			rtw_msleep_os(5);
		}
#ifdef CONFIG_80211N_HT
		if (padapter->registrypriv.ht_enable && is_supported_ht(padapter->registrypriv.wireless_mode))
		pmp_priv->tx.attrib.ht_en = 1;
#endif
#ifdef CONFIG_80211AC_VHT
		pmp_priv->tx.attrib.raid = RATEID_IDX_VHT_1SS; //10
#endif
		pmp_priv->tx.stop = 0;
		pmp_priv->tx.count = 1;
		SetPacketTx(padapter);
	} else {
		pmp_priv->mode = MP_ON;
	}

	return 0;
}

static int rtw_mp_arx(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 bStartRx=0;
	PADAPTER padapter = rtw_netdev_priv(dev);
	u8 buffer[40];

	if (copy_from_user(buffer, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	//RTW_INFO("%s: %s\n", __func__, buffer);

	bStartRx = (strncmp(buffer, "start", 5)==0)?1:0; // strncmp TRUE is 0

	SetPacketRx(padapter, bStartRx, _FALSE);

	return 0;
}

static int rtw_mp_trx_query(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u32 txok,txfail,rxok,rxfail;
	PADAPTER padapter = rtw_netdev_priv(dev);
	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	txok=padapter->mppriv.tx.sended;
	txfail=0;
	rxok = padapter->mppriv.rx_pktcount;
	rxfail = padapter->mppriv.rx_crcerrpktcount;

	_rtw_memset(extra, '\0', 128);

	sprintf(extra, "Tx OK:%d, Tx Fail:%d, Rx OK:%d, CRC error:%d ", txok, txfail,rxok,rxfail);

	wrqu->data.length=strlen(extra)+1;

	return 0;
}

static int rtw_mp_pwrtrk(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 enable;
	u32 thermal;
	s32 ret;
	PADAPTER padapter = rtw_netdev_priv(dev);


	enable = 1;
	if (wrqu->data.length > 1) { // not empty string
		if (strncmp(extra, "stop", 4) == 0)
			enable = 0;
		else {
			if (sscanf(extra, "ther=%d", &thermal)) {
				ret = SetThermalMeter(padapter, (u8)thermal);
				if (ret == _FAIL) return -EPERM;
			} else
				return -EINVAL;
		}
	}

	ret = SetPowerTracking(padapter, enable);
	if (ret == _FAIL) return -EPERM;

	return 0;
}

static int rtw_mp_psd(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);


	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	
	wrqu->data.length = mp_query_psd(padapter, extra);

	return 0;
}

static int rtw_mp_thermal(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	u8 val;
	PADAPTER padapter = rtw_netdev_priv(dev);


	GetThermalMeter(padapter, &val);
	*(u8*)extra = val;
	wrqu->data.length = 1;

	return 0;
}

static int rtw_mp_reset_stats(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	struct mp_priv *pmp_priv;
	struct pkt_attrib *pattrib;
	PADAPTER padapter = rtw_netdev_priv(dev);
	
	pmp_priv = &padapter->mppriv;
	
	pmp_priv->tx.sended = 0;
	padapter->mppriv.rx_pktcount = 0;
	padapter->mppriv.rx_crcerrpktcount = 0;

	return 0;
}

static int rtw_mp_dump(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	struct mp_priv *pmp_priv;
	struct pkt_attrib *pattrib;
        u32 value;
	u8 rf_type,path_nums = 0;
	u32 i,j=1,path;
	PADAPTER padapter = rtw_netdev_priv(dev);
	
	pmp_priv = &padapter->mppriv;

	
	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;
	
	if ( strncmp(extra, "all", 4)==0 )
	{
			printk("\n======= MAC REG =======\n");
			for ( i=0x0;i<0x300;i+=4 )
			{	
				if(j%4==1)	printk("0x%02x",i);
				printk(" 0x%08x ",rtw_read32(padapter,i));		
				if((j++)%4 == 0)	printk("\n");	
			}
			for( i=0x400;i<0x800;i+=4 )
			{	
				if(j%4==1)	printk("0x%02x",i);
				printk(" 0x%08x ",rtw_read32(padapter,i));		
				if((j++)%4 == 0)	printk("\n");	
			}
			
			i,j=1;
			padapter->hal_func.GetHwRegHandler(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
				
			printk("\n======= RF REG =======\n");
			if(( RF_1T2R == rf_type ) ||( RF_1T1R ==rf_type ))	
				path_nums = 1;
			else	
				path_nums = 2;
				
			for(path=0;path<path_nums;path++)
			{
#ifdef CONFIG_RTL8192D
			  for (i = 0; i < 0x50; i++)
#else
	   		 for (i = 0; i < 0x34; i++)
#endif
				{								
					/*value = phy_query_rf_reg(padapter, (RF90_RADIO_PATH_E)path,i, bMaskDWord);*/
					value = padapter->hal_func.read_rfreg(padapter, path, i, 0xffffffff);
					if(j%4==1)	printk("0x%02x ",i);
					printk(" 0x%08x ",value);
					if((j++)%4==0)	printk("\n");	
				}	
			}
	}
	return 0;
}
#endif
#ifndef PLATFORM_FREEBSD
static int rtw_mp_efuse_get(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	struct mp_priv *pmp_priv;	
	
	int i,j;
	u8 data[EFUSE_MAP_SIZE];
	u8 rawdata[EFUSE_MAX_SIZE];
	u16	mapLen=0;
	char *pch, *ptmp, *token, *tmp[3];
	u16 addr = 0, cnts = 0, max_available_size = 0,raw_cursize = 0 ,raw_maxsize = 0;
	
	_rtw_memset(data, '\0', sizeof(data));
	_rtw_memset(rawdata, '\0', sizeof(rawdata));
	
	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	pch = extra;
	RTW_INFO("%s: in=%s\n", __func__, extra);
	
	i=0;
	//mac 16 "00e04c871200"
	while ( (token = strsep (&pch,",") )!=NULL )
	{
		      	tmp[i] = token;		  
			i++;
	}
	
	if ( strcmp(tmp[0],"realmap") == 0 ) {
		
		RTW_INFO("strcmp OK =  %s \n" ,tmp[0]);

		mapLen = EFUSE_MAP_SIZE;
		 
		if (rtw_efuse_map_read(padapter, 0, mapLen, data) == _SUCCESS){
			RTW_INFO("\t  rtw_efuse_map_read \n"); 
		}else {
			RTW_INFO("\t  rtw_efuse_map_read : Fail \n");
			return -EFAULT;
		} 
		_rtw_memset(extra, '\0', sizeof(extra));
		RTW_INFO("\tOFFSET\tVALUE(hex)\n");
		sprintf(extra, "%s \n", extra);
		for ( i = 0; i < EFUSE_MAP_SIZE; i += 16 )
		{
			RTW_INFO("\t0x%02x\t", i);
			sprintf(extra, "%s \t0x%02x\t", extra,i);
			for (j = 0; j < 8; j++)
			{	  
				RTW_INFO("%02X ", data[i+j]);
				sprintf(extra, "%s %02X", extra, data[i+j]);
			}
			RTW_INFO("\t");
			sprintf(extra,"%s\t",extra);
			for (; j < 16; j++){
				RTW_INFO("%02X ", data[i+j]);
				sprintf(extra, "%s %02X", extra, data[i+j]);
			}
			RTW_INFO("\n");
			sprintf(extra,"%s\n",extra);	
		}
		RTW_INFO("\n");
		wrqu->data.length = strlen(extra);
	
		return 0;
	}
	else if ( strcmp(tmp[0],"rmap") == 0 ) {
		// rmap addr cnts
		addr = simple_strtoul(tmp[1], &ptmp, 16);

		RTW_INFO("addr = %x \n" ,addr);

		cnts=simple_strtoul(tmp[2], &ptmp,10);
		RTW_INFO("cnts = %d \n" ,cnts);
		//_rtw_memset(extra, '\0', wrqu->data.length);

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr + cnts) > max_available_size) {
			RTW_INFO("(addr + cnts parameter error \n");
			return -EFAULT;
		}
				
		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL) 
		{
			RTW_INFO("rtw_efuse_access error \n");   		
		}
		else{
			RTW_INFO("rtw_efuse_access ok \n");
		}	

		_rtw_memset(extra, '\0', sizeof(extra));	 
		for ( i = 0; i < cnts; i ++) {
			RTW_INFO("0x%02x", data[i]);
			sprintf(extra, "%s 0x%02X", extra, data[i]);
			RTW_INFO(" ");
			sprintf(extra,"%s ",extra);
		}

		wrqu->data.length = strlen(extra)+1;

		RTW_INFO("extra = %s ", extra);

		return 0;	
	}
	else if ( strcmp(tmp[0],"realraw") == 0 ) {
		addr=0;
		mapLen = EFUSE_MAX_SIZE;

		if (rtw_efuse_access(padapter, _FALSE, addr, mapLen, rawdata) == _FAIL)
		{
			RTW_INFO("\t  rtw_efuse_map_read : Fail \n");
			return -EFAULT;
		} else
		{
			RTW_INFO("\t  rtw_efuse_access raw ok \n"); 	
		}
				
		_rtw_memset(extra, '\0', sizeof(extra));
		for ( i=0; i<mapLen; i++ ) {
			RTW_INFO(" %02x", rawdata[i]);
			sprintf(extra, "%s %02x", extra, rawdata[i] );

			if ((i & 0xF) == 0xF){ 
				RTW_INFO("\n\t");
				sprintf(extra, "%s\n\t", extra);
			}
			else if ((i & 0x7) == 0x7){ 
				RTW_INFO("\t");
				sprintf(extra, "%s\t", extra);
			}
		}
		wrqu->data.length = strlen(extra);
		return 0;
	}
	else if ( strcmp(tmp[0],"mac") == 0 ) {
		#ifdef CONFIG_RTL8192C
		addr = 0x16;
		cnts = 6;
		#endif
		#ifdef CONFIG_RTL8192D
		addr = 0x19;
		cnts = 6;
		#endif
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr + mapLen) > max_available_size) {
			RTW_INFO("(addr + cnts parameter error \n");
			return -EFAULT;
		}
		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL)
		{
			RTW_INFO("rtw_efuse_access error \n");   		
		}
		else{
			RTW_INFO("rtw_efuse_access ok \n");
		}	
		_rtw_memset(extra, '\0', sizeof(extra));		 
		for ( i = 0; i < cnts; i ++) {
			RTW_INFO("0x%02x", data[i]);
			sprintf(extra, "%s 0x%02X", extra, data[i+j]);
			RTW_INFO(" ");
			sprintf(extra,"%s ",extra);
		}
		wrqu->data.length = strlen(extra);
		return 0;
	}
	else if ( strcmp(tmp[0],"vidpid") == 0 ) {
		#ifdef CONFIG_RTL8192C
		addr=0x0a;
		#endif
		#ifdef CONFIG_RTL8192D
		addr = 0x0c;
		#endif
		cnts = 4;
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		if ((addr + mapLen) > max_available_size) {
			RTW_INFO("(addr + cnts parameter error \n");
			return -EFAULT;
		}
		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL) 
		{
			RTW_INFO("rtw_efuse_access error \n");   		
		}
		else{
			RTW_INFO("rtw_efuse_access ok \n");
		}	
		_rtw_memset(extra, '\0', sizeof(extra));		 
		for ( i = 0; i < cnts; i ++) {
			RTW_INFO("0x%02x", data[i]);
			sprintf(extra, "%s 0x%02X", extra, data[i+j]);
			RTW_INFO(" ");
			sprintf(extra,"%s ",extra);
		}
		wrqu->data.length = strlen(extra);
		return 0;
	}
	else if ( strcmp(tmp[0],"ableraw") == 0 ) {
		efuse_GetCurrentSize(padapter,&raw_cursize);
		raw_maxsize = efuse_GetMaxSize(padapter);
		sprintf(extra, "%s : [ available raw size] = %d",extra,raw_maxsize-raw_cursize);
		wrqu->data.length = strlen(extra);

		return 0;
	}
	return 0;
}

static int rtw_mp_efuse_set(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	PADAPTER padapter = rtw_netdev_priv(dev);
	
	u8 buffer[40];
	u32 i,jj,kk;
	u8 setdata[EFUSE_MAP_SIZE];
	u8 setrawdata[EFUSE_MAX_SIZE];
	char *pch, *ptmp, *token, *edata,*tmp[5];

	u16 addr = 0, max_available_size = 0;
	u32  cnts = 0;
	
	pch = extra;
	RTW_INFO("%s: in=%s\n", __func__, extra);
	
	i=0;
	while ( (token = strsep (&pch,",") )!=NULL )
	{
		      	tmp[i] = token;
			i++;
	}
	// tmp[0],[1],[2]
	// wmap,addr,00e04c871200
	if ( strcmp(tmp[0],"wmap") == 0 ) {
      if ( ! strlen( tmp[2] )/2 > 1 ) return -EFAULT;             
			addr = simple_strtoul( tmp[1], &ptmp, 16 );
			addr = addr & 0xFF;
			RTW_INFO("addr = %x \n" ,addr);
					
			cnts = strlen( tmp[2] )/2;	
			if ( cnts == 0) return -EFAULT;
					
			RTW_INFO("cnts = %d \n" ,cnts);
			RTW_INFO("target data = %s \n" ,tmp[2]);
					
			for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
			{
				setdata[jj] = key_2char2num( tmp[2][kk], tmp[2][kk+ 1] );
			}
	
			EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
			
			if ((addr + cnts) > max_available_size) {
						RTW_INFO("parameter error \n");
						return -EFAULT;
			}	
			if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL) {			
					RTW_INFO("rtw_efuse_map_write error \n");
					return -EFAULT;
			} else
			   RTW_INFO("rtw_efuse_map_write ok \n");
		
		return 0;
	}
	else if ( strcmp(tmp[0],"wraw") == 0 ) {
			 if ( ! strlen( tmp[2] )/2 > 1 ) return -EFAULT;             
			addr = simple_strtoul( tmp[1], &ptmp, 16 );
			addr = addr & 0xFF;
			RTW_INFO("addr = %x \n" ,addr);
				
			cnts=strlen( tmp[2] )/2;
			if ( cnts == 0) return -EFAULT;

			RTW_INFO(" cnts = %d \n" ,cnts );		
			RTW_INFO("target data = %s \n" ,tmp[2] );
			
			for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
			{
					setrawdata[jj] = key_2char2num( tmp[2][kk], tmp[2][kk+ 1] );
			}
					
			if ( rtw_efuse_access( padapter, _TRUE, addr, cnts, setrawdata ) == _FAIL ){
					RTW_INFO("\t  rtw_efuse_map_read : Fail \n");
						return -EFAULT;
			} else
			  RTW_INFO("\t  rtw_efuse_access raw ok \n"); 	
			
					return 0;
		}
	else if ( strcmp(tmp[0],"mac") == 0 ) { 
			//mac,00e04c871200
			#ifdef CONFIG_RTL8192C
				addr = 0x16;
			#endif
			#ifdef CONFIG_RTL8192D
				addr = 0x19;
			#endif
				cnts = strlen( tmp[1] )/2;
				if ( cnts == 0) return -EFAULT;
				if ( cnts > 6 ){
						RTW_INFO("error data for mac addr = %s \n" ,tmp[1]);
						return -EFAULT;
				}
				
				RTW_INFO("target data = %s \n" ,tmp[1]);
				
				for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
				{
					setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+ 1]);
				}
				
				EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
		
				if ((addr + cnts) > max_available_size) {
						RTW_INFO("parameter error \n");
						return -EFAULT;
					}	
				if ( rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL ) {
					RTW_INFO("rtw_efuse_map_write error \n");
					return -EFAULT;
				} else
					RTW_INFO("rtw_efuse_map_write ok \n");
				
			return 0;
		}
		else if ( strcmp(tmp[0],"vidpid") == 0 ) { 
				// pidvid,da0b7881
				#ifdef CONFIG_RTL8192C
				       addr=0x0a;
				#endif
				#ifdef CONFIG_RTL8192D
					addr = 0x0c;
				#endif
				
				cnts=strlen( tmp[1] )/2;
				if ( cnts == 0) return -EFAULT;
				RTW_INFO("target data = %s \n" ,tmp[1]);
				
				for( jj = 0, kk = 0; jj < cnts; jj++, kk += 2 )
				{
					setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+ 1]);
				}

				EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);
				
				if ((addr + cnts) > max_available_size) {
						RTW_INFO("parameter error \n");
						return -EFAULT;
					}	
				
				if ( rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL ) {
					RTW_INFO("rtw_efuse_map_write error \n");
					return -EFAULT;
				} else
					RTW_INFO("rtw_efuse_map_write ok \n");
			
				return 0;
		}
		
	  return 0;
}

static int rtw_pm_set(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	RTW_INFO( "[%s] extra = %s\n", __FUNCTION__, extra );

	if ( _rtw_memcmp( extra, "lps=", 4 ) )
	{
		wrqu->data.length -= 5;
		rtw_pm_set_lps( dev, info, wrqu, &extra[4] );
	}
	else if ( _rtw_memcmp( extra, "ips=", 4 ) )
	{
		wrqu->data.length -= 5;
		rtw_pm_set_ips(dev, info, wrqu, &extra[4]);
	}

	return ret;
}


#if 0
//based on "driver_ipw" and for hostapd
int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	//_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret=0;

	//down(&priv->wx_sem);

	switch (cmd)
	{
	    case RTL_IOCTL_WPA_SUPPLICANT:	
			ret = wpa_supplicant_ioctl(dev, &wrq->u.data);
			break;
#ifdef CONFIG_AP_MODE
		case RTL_IOCTL_HOSTAPD:
			ret = rtw_hostapd_ioctl(dev, &wrq->u.data);			
			break;
#endif
	    default:
			ret = -EOPNOTSUPP;
			break;
	}

	//up(&priv->wx_sem);
	
	return ret;
	
}
#endif	//naming by FreeBSD

static iw_handler rtw_handlers[] =
{
	NULL,					/* SIOCSIWCOMMIT */
	rtw_wx_get_name,		/* SIOCGIWNAME */
	dummy,					/* SIOCSIWNWID */
	dummy,					/* SIOCGIWNWID */
	rtw_wx_set_freq,		/* SIOCSIWFREQ */
	rtw_wx_get_freq,		/* SIOCGIWFREQ */
	rtw_wx_set_mode,		/* SIOCSIWMODE */
	rtw_wx_get_mode,		/* SIOCGIWMODE */
	dummy,					/* SIOCSIWSENS */
	rtw_wx_get_sens,		/* SIOCGIWSENS */
	NULL,					/* SIOCSIWRANGE */
	rtw_wx_get_range,		/* SIOCGIWRANGE */
	rtw_wx_set_priv,		/* SIOCSIWPRIV */
	NULL,					/* SIOCGIWPRIV */
	NULL,					/* SIOCSIWSTATS */
	NULL,					/* SIOCGIWSTATS */
	dummy,					/* SIOCSIWSPY */
	dummy,					/* SIOCGIWSPY */
	NULL,					/* SIOCGIWTHRSPY */
	NULL,					/* SIOCWIWTHRSPY */
	rtw_wx_set_wap,		/* SIOCSIWAP */
	rtw_wx_get_wap,		/* SIOCGIWAP */
	rtw_wx_set_mlme,		/* request MLME operation; uses struct iw_mlme */
	dummy,					/* SIOCGIWAPLIST -- depricated */
	rtw_wx_set_scan,		/* SIOCSIWSCAN */
	rtw_wx_get_scan,		/* SIOCGIWSCAN */
	rtw_wx_set_essid,		/* SIOCSIWESSID */
	rtw_wx_get_essid,		/* SIOCGIWESSID */
	dummy,					/* SIOCSIWNICKN */
	rtw_wx_get_nick,		/* SIOCGIWNICKN */
	NULL,					/* -- hole -- */
	NULL,					/* -- hole -- */
	rtw_wx_set_rate,		/* SIOCSIWRATE */
	rtw_wx_get_rate,		/* SIOCGIWRATE */
	dummy,					/* SIOCSIWRTS */
	rtw_wx_get_rts,			/* SIOCGIWRTS */
	rtw_wx_set_frag,		/* SIOCSIWFRAG */
	rtw_wx_get_frag,		/* SIOCGIWFRAG */
	dummy,					/* SIOCSIWTXPOW */
	dummy,					/* SIOCGIWTXPOW */
	dummy,					/* SIOCSIWRETRY */
	rtw_wx_get_retry,		/* SIOCGIWRETRY */
	rtw_wx_set_enc,			/* SIOCSIWENCODE */
	rtw_wx_get_enc,			/* SIOCGIWENCODE */
	dummy,					/* SIOCSIWPOWER */
	rtw_wx_get_power,		/* SIOCGIWPOWER */
	NULL,					/*---hole---*/
	NULL,					/*---hole---*/
	rtw_wx_set_gen_ie,		/* SIOCSIWGENIE */
	NULL,					/* SIOCGWGENIE */
	rtw_wx_set_auth,		/* SIOCSIWAUTH */
	NULL,					/* SIOCGIWAUTH */
	rtw_wx_set_enc_ext,		/* SIOCSIWENCODEEXT */
	NULL,					/* SIOCGIWENCODEEXT */
	rtw_wx_set_pmkid,		/* SIOCSIWPMKSA */
	NULL,					/*---hole---*/
}; 
#endif

#if defined(CONFIG_MP_INCLUDED) && defined(CONFIG_MP_IWPRIV_SUPPORT)

static const struct iw_priv_args rtw_private_args[] =
{
	{SIOCIWFIRSTPRIV + 0x00, IW_PRIV_TYPE_CHAR | 128, 0, "write_reg"},
	{SIOCIWFIRSTPRIV + 0x01, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_BYTE | 4, "read_reg"},
	{SIOCIWFIRSTPRIV + 0x02, IW_PRIV_TYPE_CHAR | 128, 0, "write_rf" },
	{SIOCIWFIRSTPRIV + 0x03, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 4, "read_rf" },
	{SIOCIWFIRSTPRIV + 0x04, IW_PRIV_TYPE_NONE, 0, "mp_start"},
	{SIOCIWFIRSTPRIV + 0x05, IW_PRIV_TYPE_NONE, 0, "mp_stop"},
	{SIOCIWFIRSTPRIV + 0x06, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "mp_rate"},
	{SIOCIWFIRSTPRIV + 0x07, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "mp_channel"},
	{SIOCIWFIRSTPRIV + 0x08, IW_PRIV_TYPE_CHAR | 40, 0, "mp_bandwidth"},
	{SIOCIWFIRSTPRIV + 0x09, IW_PRIV_TYPE_CHAR | 40, 0, "mp_txpower"},
	{SIOCIWFIRSTPRIV + 0x0a, IW_PRIV_TYPE_CHAR | IFNAMSIZ, 0, "mp_ant_tx"},
	{SIOCIWFIRSTPRIV + 0x0b, IW_PRIV_TYPE_CHAR | IFNAMSIZ, 0, "mp_ant_rx"},
	{SIOCIWFIRSTPRIV + 0x0c, IW_PRIV_TYPE_CHAR | 128, 0, "mp_ctx"},
	{SIOCIWFIRSTPRIV + 0x0d, 0, IW_PRIV_TYPE_CHAR | 128, "mp_query"},
	{SIOCIWFIRSTPRIV + 0x0e, IW_PRIV_TYPE_CHAR | 40, 0, "mp_arx"},
	{SIOCIWFIRSTPRIV + 0x0f, IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 0x7FF, "mp_psd"}, 
	{SIOCIWFIRSTPRIV + 0x10, IW_PRIV_TYPE_CHAR | 40, 0, "mp_pwrtrk"},
	{SIOCIWFIRSTPRIV + 0x11, 0, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, "mp_ther"},
	{SIOCIWFIRSTPRIV + 0x12, 0, 0, "mp_ioctl"}, // mp_ioctl
	{SIOCIWFIRSTPRIV + 0x13, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR |IW_PRIV_SIZE_FIXED |0x700 ,"efuse_get"},
	{SIOCIWFIRSTPRIV + 0x14, IW_PRIV_TYPE_CHAR | 128, 0, "efuse_set"},
	{SIOCIWFIRSTPRIV + 0x15, IW_PRIV_TYPE_CHAR | 128, 0, "mp_reset_stats"},
	{SIOCIWFIRSTPRIV + 0x16, IW_PRIV_TYPE_CHAR | 128, 0, "mp_dump"},

};

static iw_handler rtw_private_handler[] = 
{
	rtw_mp_write_reg,	// set, 0x00 = 0
	rtw_mp_read_reg,	// get, 0x01 = 1
	rtw_mp_write_rf,	// set, 0x02 = 2
	rtw_mp_read_rf,		// get, 0x03 = 3
	rtw_mp_start,
	rtw_mp_stop,
	rtw_mp_rate,
	rtw_mp_channel,
	rtw_mp_bandwidth,
	rtw_mp_txpower,
	rtw_mp_ant_tx,
	rtw_mp_ant_rx,
	rtw_mp_ctx,
	rtw_mp_trx_query,	// get, 0x0d = 13
	rtw_mp_arx,
	rtw_mp_psd,		// get, 0x0f = 15
	rtw_mp_pwrtrk,		// set, 0x10 = 16
	rtw_mp_thermal,		// get, 0x11 = 17
	rtw_mp_ioctl_hdl,
	rtw_mp_efuse_get,
	rtw_mp_efuse_set,
	rtw_mp_reset_stats,
	rtw_mp_dump,
};

#else // not inlucde MP
#ifndef PLATFORM_FREEBSD
static const struct iw_priv_args rtw_private_args[] = {
	{
		SIOCIWFIRSTPRIV + 0x0,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "rtw_write32"
	},
	{
		SIOCIWFIRSTPRIV + 0x1,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "rtw_read32"
	},
	{
		SIOCIWFIRSTPRIV + 0x2, 0, 0, "driver_ext"
	},
	{
		SIOCIWFIRSTPRIV + 0x3, 0, 0, "" // mp_ioctl
	},
	{
		SIOCIWFIRSTPRIV + 0x4,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "apinfo"
	},
	{
		SIOCIWFIRSTPRIV + 0x5,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "setpid"
	},
	{
		SIOCIWFIRSTPRIV + 0x6,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_start"
	},
//for PLATFORM_MT53XX	
	{
		SIOCIWFIRSTPRIV + 0x7,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "get_sensitivity"
	},
	{
		SIOCIWFIRSTPRIV + 0x8,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_prob_req_ie"
	},
	{
		SIOCIWFIRSTPRIV + 0x9,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_assoc_req_ie"
	},

//for RTK_DMP_PLATFORM	
	{
		SIOCIWFIRSTPRIV + 0xA,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "channel_plan"
	},

	{
		SIOCIWFIRSTPRIV + 0xB,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "dbg"
	},	
	{
		SIOCIWFIRSTPRIV + 0xC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "rfw"
	},
	{
		SIOCIWFIRSTPRIV + 0xD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "rfr"
	},
#if 0
	{
		SIOCIWFIRSTPRIV + 0xE,0,0, "wowlan_ctrl"
	},
#endif
	{
		SIOCIWFIRSTPRIV + 0x10,
		IW_PRIV_TYPE_CHAR | P2P_PRIVATE_IOCTL_SET_LEN, 0, "p2p_set"
	},
	{
		SIOCIWFIRSTPRIV + 0x11,
		IW_PRIV_TYPE_CHAR | P2P_PRIVATE_IOCTL_SET_LEN, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ , "p2p_get"
	},
	{
		SIOCIWFIRSTPRIV + 0x12,
		IW_PRIV_TYPE_CHAR | P2P_PRIVATE_IOCTL_SET_LEN, IW_PRIV_TYPE_CHAR | IFNAMSIZ , "p2p_get_wpsCM"
	},	
#ifdef CONFIG_TDLS
	{SIOCIWFIRSTPRIV + 0x13, IW_PRIV_TYPE_CHAR | 128, 0,"NULL"},
	{
		SIOCIWFIRSTPRIV + 0x14,
		IW_PRIV_TYPE_CHAR  | 64, 0, "tdls"
	},
#endif
	{
		SIOCIWFIRSTPRIV + 0x16,
		IW_PRIV_TYPE_CHAR | P2P_PRIVATE_IOCTL_SET_LEN, 0, "pm_set"
	},

	{SIOCIWFIRSTPRIV + 0x18, IW_PRIV_TYPE_CHAR | IFNAMSIZ , 0 , "rereg_nd_name"},

	{SIOCIWFIRSTPRIV + 0x1A, IW_PRIV_TYPE_CHAR | 128, 0, "efuse_set"},
	{SIOCIWFIRSTPRIV + 0x1B, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR |IW_PRIV_SIZE_FIXED |0x700 ,"efuse_get"},

};

static iw_handler rtw_private_handler[] = 
{
	rtw_wx_write32,					//0x00
	rtw_wx_read32,					//0x01
	NULL,					//0x02
	rtw_mp_ioctl_hdl,				//0x03

// for MM DTV platform
	rtw_get_ap_info,					//0x04

	rtw_set_pid,						//0x05
	rtw_wps_start,					//0x06

// for PLATFORM_MT53XX
	rtw_wx_get_sensitivity,			//0x07
	rtw_wx_set_mtk_wps_probe_ie,	//0x08
	rtw_wx_set_mtk_wps_ie,			//0x09

	NULL,							//0x0A

	rtw_dbg_port,					//0x0B
	rtw_wx_write_rf,					//0x0C
	rtw_wx_read_rf,					//0x0D

#if 0
	rtw_wowlan_ctrl,					//0x0E
#else
	rtw_wx_priv_null,				//0x0E
#endif
	rtw_wx_priv_null,				//0x0F

	rtw_p2p_set,					//0x10
	rtw_p2p_get,					//0x11
	rtw_p2p_get2,					//0x12

	NULL,							//0x13
	rtw_tdls,						//0x14
	rtw_wx_priv_null,				//0x15

	rtw_pm_set,						//0x16
	rtw_wx_priv_null,				//0x17
	rtw_rereg_nd_name,				//0x18
	rtw_wx_priv_null,				//0x19

	rtw_mp_efuse_set,				//0x1A
	rtw_mp_efuse_get,				//0x1B
	// 0x1C is reserved for hostapd
};
#endif
#endif // #if defined(CONFIG_MP_INCLUDED) && defined(CONFIG_MP_IWPRIV_SUPPORT)

#if WIRELESS_EXT >= 17	
static struct iw_statistics *rtw_get_wireless_stats(struct net_device *dev)
{
       _adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	   struct iw_statistics *piwstats=&padapter->iwstats;
	int tmp_level = 0;
	int tmp_qual = 0;
	int tmp_noise = 0;

	if (check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE) != _TRUE)
	{
		piwstats->qual.qual = 0;
		piwstats->qual.level = 0;
		piwstats->qual.noise = 0;
		//RTW_INFO("No link  level:%d, qual:%d, noise:%d\n", tmp_level, tmp_qual, tmp_noise);
	}
	else{
		#ifdef CONFIG_SIGNAL_DISPLAY_DBM
		tmp_level = padapter->recvpriv.rssi; 
		#else
		tmp_level = padapter->recvpriv.signal_strength;
		#endif
		
		tmp_qual =padapter->recvpriv.signal_strength; //padapter->recvpriv.signal_qual;
		tmp_noise =padapter->recvpriv.noise;		
		//RTW_INFO("level:%d, qual:%d, noise:%d, rssi (%d)\n", tmp_level, tmp_qual, tmp_noise,padapter->recvpriv.rssi);

		piwstats->qual.level = tmp_level;
		piwstats->qual.qual = tmp_qual;
		piwstats->qual.noise = tmp_noise;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14))
	piwstats->qual.updated = IW_QUAL_ALL_UPDATED ;//|IW_QUAL_DBM;
#else
#ifdef RTK_DMP_PLATFORM
	//IW_QUAL_DBM= 0x8, if driver use this flag, wireless extension will show value of dbm.
	//remove this flag for show percentage 0~100
	piwstats->qual.updated = 0x07;
#else
	piwstats->qual.updated = 0x0f;
#endif
#endif

	#ifdef CONFIG_ANDROID
	piwstats->qual.updated = piwstats->qual.updated | IW_QUAL_DBM;
	#endif

	return &padapter->iwstats;
}
#endif
#ifndef PLATFORM_FREEBSD
struct iw_handler_def rtw_handlers_def =
{
	.standard = rtw_handlers,
	.num_standard = sizeof(rtw_handlers) / sizeof(iw_handler),
	.private = rtw_private_handler,
	.private_args = (struct iw_priv_args *)rtw_private_args,
	.num_private = sizeof(rtw_private_handler) / sizeof(iw_handler),
 	.num_private_args = sizeof(rtw_private_args) / sizeof(struct iw_priv_args),
#if WIRELESS_EXT >= 17
	.get_wireless_stats = rtw_get_wireless_stats,
#endif
};

#endif

static int
rtw_ioctl_set80211(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{



#define RTW_BSD_HOSTAPD_SET_BEACON (1100)


	int error;

	error = 0;

//	RTW_INFO("[%s] cmd:%d \n", __FUNCTION__, ireq->i_type);

	switch (ireq->i_type) {
		case IEEE80211_IOC_SSID:	
			error=rtw_wx_set_essid(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_AUTHMODE:
			error=rtw_wx_set_auth(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_ROAMING:		//cmd 16
			break;
		case IEEE80211_IOC_PRIVACY:
			break;
		case IEEE80211_IOC_DROPUNENCRYPTED:
			error=rtw_wx_set_auth(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_WPAKEY:
			error=rtw_wx_set_wpakey(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_DELKEY:
			error=rtw_wx_delkey(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_MLME:
			error=rtw_wx_set_mlme(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_COUNTERMEASURES:
			rtw_wx_set_auth(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_WPA:			//cmd 26
			error = 0;
			break;
	       case IEEE80211_IOC_BSSID:
       	       break;
#define OLD_IEEE80211_IOC_SCAN_REQ      23
#ifdef OLD_IEEE80211_IOC_SCAN_REQ
		case OLD_IEEE80211_IOC_SCAN_REQ:
			break;
#endif /* OLD_IEEE80211_IOC_SCAN_REQ */
		case IEEE80211_IOC_SCAN_REQ:
			rtw_wx_set_scan(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_APPIE:
			error = rtw_wx_set_gen_ie(padapter, cmd, ireq);
			break;			
#ifdef CONFIG_AP_MODE
		case RTW_BSD_HOSTAPD_SET_BEACON:
			error =  rtw_set_beacon(padapter, cmd, ireq);
			break;
#endif
		default:
			error=EINVAL;
			break;
	}
	return error;
}


static int
rtw_ioctl_get80211(_adapter *padapter, u_long cmd, struct ieee80211req *ireq)
{
	int error = 0;
	int channel = 50;	// 13*2 B-band + 24 A-band
	u8 *cp;
	int i;
	struct ieee80211_channel *ev, *ev2;

	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;

//	RTW_INFO("[%s] cmd:%d \n", __FUNCTION__, ireq->i_type);
	
	switch (ireq->i_type) {
		case IEEE80211_IOC_SSID:
			error=rtw_wx_get_essid(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_NUMSSIDS:
			ireq->i_val = 1;
			break;
		case IEEE80211_IOC_WEP:
			switch(padapter->securitypriv.ndisencryptstatus)
				{
					case Ndis802_11EncryptionNotSupported:
					case Ndis802_11EncryptionDisabled:
						ireq->i_val = IEEE80211_WEP_OFF;
						break;
					case Ndis802_11Encryption1Enabled:					
						if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen)
				     			ireq->i_val = IEEE80211_WEP_MIXED;
						else if(padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeShared)
				     			ireq->i_val = IEEE80211_WEP_ON;
						break;
					case Ndis802_11Encryption2Enabled:
					case Ndis802_11Encryption3Enabled:
				     			ireq->i_val = IEEE80211_WEP_MIXED;
						break;
					default:
						ireq->i_val = IEEE80211_WEP_OFF;
					break;
				}
			break;
		case IEEE80211_IOC_NUMWEPKEYS:
			ireq->i_val = IEEE80211_WEP_NKID;
			break;			
		case IEEE80211_IOC_WEPTXKEY:
			ireq->i_val = psecuritypriv->dot118021XGrpKeyid;
			break;
		case IEEE80211_IOC_AUTHMODE:
			switch(padapter->securitypriv.dot11AuthAlgrthm){
				case dot11AuthAlgrthm_Auto:
					ireq->i_val = IEEE80211_AUTH_AUTO;
					break;
				case dot11AuthAlgrthm_Open:
					ireq->i_val = IEEE80211_AUTH_OPEN;
					break;
				case dot11AuthAlgrthm_Shared:
					ireq->i_val = IEEE80211_AUTH_SHARED;
					break;
				default:
					ireq->i_val = IEEE80211_AUTH_NONE;
					break;
			}
			break;
		case IEEE80211_IOC_CHANNEL:
			ireq->i_val = pmlmeext->cur_channel;
			break;
		case IEEE80211_IOC_POWERSAVE:
			if (pregistrypriv->power_mgnt==PS_MODE_ACTIVE)
				ireq->i_val = IEEE80211_POWERSAVE_ON;
			else
				ireq->i_val = IEEE80211_POWERSAVE_OFF;
			break;
		case IEEE80211_IOC_POWERSAVESLEEP:		//need to update
			break;
		case IEEE80211_IOC_RTSTHRESHOLD:
			ireq->i_val = padapter->registrypriv.rts_thresh;
			break;
		case IEEE80211_IOC_PROTMODE:
			switch(padapter->registrypriv.vcs_type){
				case NONE_VCS:
					ireq->i_val = 0;
					break;
				case RTS_CTS:
					ireq->i_val = 2;
					break;
				case CTS_TO_SELF:
					ireq->i_val = 1;
					break;
			}
			break;
		case IEEE80211_IOC_TXPOWER:
			ireq->i_val = 0;	//need to update
			break;
		case IEEE80211_IOC_BSSID:
			error = rtw_wx_get_wap(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_ROAMING:
			ireq->i_val = IEEE80211_ROAMING_AUTO;
			break;
		case IEEE80211_IOC_PRIVACY:
			ireq->i_val = 0;
			break;
		case IEEE80211_IOC_DROPUNENCRYPTED:
			break;
		case IEEE80211_IOC_WPAKEY:
			error = rtw_wx_get_wpakey(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_COUNTERMEASURES:
			break;
		case IEEE80211_IOC_WPA:
			ireq->i_val = 3;
			break;
		case IEEE80211_IOC_WME:
			ireq->i_val = 0;	//need to varify
			break;			
		case IEEE80211_IOC_WPAIE:
#ifdef CONFIG_AP_MODE
			error = rtw_get_sta_wpaie(padapter, cmd, ireq);
#endif			
			break;
		case IEEE80211_IOC_CHANINFO:
			cp=rtw_malloc(sizeof(struct ieee80211_channel)*channel+sizeof(int));
			_rtw_memset(cp, 0, sizeof(struct ieee80211_channel)*channel+sizeof(int));
			_rtw_memcpy(cp, &channel, 4);
			ev = (struct ieee80211_channel *)(cp+4);
			ev2 = ev+1;
			for(i=0;i<13; i++)
			{
				ev->ic_flags = IEEE80211_CHAN_B |IEEE80211_CHAN_HT;
				ev->ic_freq = 2412+5*i;
				ev->ic_ieee = 1+i;
				ev+=2;
				ev2->ic_flags = IEEE80211_CHAN_G |IEEE80211_CHAN_HT;
				ev2->ic_freq = 2412+5*i;
				ev2->ic_ieee = 1+i;
				ev2+=2;
			}
			for(i=36;i<=64; i+=4)
			{
				ev->ic_flags = IEEE80211_CHAN_A |IEEE80211_CHAN_HT;
				ev->ic_freq = 5180+5*(i-36);
				ev->ic_ieee = i;
				ev++;
			}			
			for(i=100;i<=140; i+=4)
			{
				ev->ic_flags = IEEE80211_CHAN_A | IEEE80211_CHAN_HT;
				ev->ic_freq = 5500+5*(i-100);
				ev->ic_ieee = i;
				ev++;
			}
			for(i=149;i<=165; i+=4)
			{
				ev->ic_flags = IEEE80211_CHAN_A | IEEE80211_CHAN_HT;
				ev->ic_freq = 5745+5*(i-149);
				ev->ic_ieee = i;
				ev++;
			}
			copyout(cp, (u8 *)ireq->i_data, sizeof(struct ieee80211_channel)*channel+sizeof(int));
			rtw_mfree(cp, sizeof(struct ieee80211_channel)*channel+sizeof(int));
			break;
		case IEEE80211_IOC_BEACON_INTERVAL:
			ireq->i_val = 0;	//need to update
			break;			
		case IEEE80211_IOC_PUREG:
			ireq->i_val = 0;
			break;			
		case IEEE80211_IOC_BGSCAN:
			ireq->i_val = 1;	//copy from 8187B; 1 means bgscanning ons
			break;
		case IEEE80211_IOC_BGSCAN_IDLE:
			ireq->i_val = 100;	//copy from 8187B
			break;			
		case IEEE80211_IOC_BGSCAN_INTERVAL:
			ireq->i_val = 60;	//copy from 8187B
			break;			
		case IEEE80211_IOC_SCANVALID:
			ireq->i_val = 60;		//copy from 8187B
			break;
		case IEEE80211_IOC_FRAGTHRESHOLD:
			ireq->i_val = padapter->registrypriv.frag_thresh;
			break;
		case IEEE80211_IOC_BURST:
			ireq->i_val = 1;	//need to varify
			break;
		case IEEE80211_IOC_SCAN_RESULTS:
			error=rtw_wx_get_scan(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_WPAIE2:
			break;
		case IEEE80211_IOC_BMISSTHRESHOLD:
			ireq->i_val = 7;		//copy from 8187B driver
			break;			
		case IEEE80211_IOC_CURCHAN:
			ev = (struct ieee80211_channel *)ireq->i_data;
			_rtw_memset(ireq->i_data, 0, sizeof(struct ieee80211_channel));
			if(pmlmeext->cur_channel <= 13){
				ev->ic_freq = 2412+5*((pmlmeext->cur_channel-1)>0?(pmlmeext->cur_channel-1):0);
				ev->ic_ieee = ((pmlmeext->cur_channel)>0?(pmlmeext->cur_channel):1);
				if(pmlmeext->cur_wireless_mode & WIRELESS_11B)
					ev->ic_flags |= IEEE80211_CHAN_B;
				if(pmlmeext->cur_wireless_mode & WIRELESS_11G)
					ev->ic_flags |= IEEE80211_CHAN_G;
				if(pmlmeext->cur_wireless_mode & WIRELESS_11_24N){
					if(pmlmeext->cur_bwmode == CHANNEL_WIDTH_20)
						ev->ic_flags |= IEEE80211_CHAN_HT;
					else if(pmlmeext->cur_bwmode == CHANNEL_WIDTH_40)
						ev->ic_flags |= IEEE80211_CHAN_HT40;
				}
			}else {
				ev->ic_freq = 5180+5*((pmlmeext->cur_channel-1)>0?(pmlmeext->cur_channel-1):0);
				ev->ic_ieee = ((pmlmeext->cur_channel)>0?(pmlmeext->cur_channel):1);
				if(pmlmeext->cur_wireless_mode & WIRELESS_11A)
					ev->ic_flags |= IEEE80211_CHAN_A;
				if(pmlmeext->cur_wireless_mode & WIRELESS_11_5N){
					if(pmlmeext->cur_bwmode == CHANNEL_WIDTH_20)
						ev->ic_flags |= IEEE80211_CHAN_HT;
					else if(pmlmeext->cur_bwmode == CHANNEL_WIDTH_40)
						ev->ic_flags |= IEEE80211_CHAN_HT40;
				}
			}
			break;
		case IEEE80211_IOC_DOTD:	//802.11d enable
			ireq->i_val = 0;
			break;
		case IEEE80211_IOC_DWDS:
			ireq->i_val = 0;
			break;
		case IEEE80211_IOC_DEVCAPS:
			error = rtw_wx_getdevcaps(padapter, cmd, ireq);
			break;
		case IEEE80211_IOC_HTCONF:
			ireq->i_val = 0;
			break;
		case IEEE80211_IOC_REGDOMAIN:			
			_rtw_memset(ireq->i_data, 0, sizeof(struct ieee80211_regdomain));
			break;
		case IEEE80211_IOC_ROAM:
			error = rtw_wx_getroam(padapter, cmd, ireq);
			break;			
		case IEEE80211_IOC_TXPARAMS:			
			error = rtw_wx_gettxparams(padapter, cmd, ireq);
			break;
		default:
			error=EINVAL;
                 	break;
         }
         return error;
 }

int rtw_ifmedia_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data);
int rtw_ifmedia_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	_adapter *padapter = ifp->if_softc;	
	const char *name = "rtw0";
	int nlist = 31 ;
	int list[31] = {145,  144,  143,  142,  141,  
				140,  139,  138,  136,  135,  
				134,  133,  196753,  196752,  196751,  
				196750,  196749,  196748,  196747,  196746,  
				196744,  196743,  196742,  196741,  196736,  
				131208,  131207,  131206,  131205,  131200, 
				128};
	struct ifmediareq *ifmr = (struct ifmediareq *) data;
	struct ifreq *ifr = (struct ifreq *) data;

	switch (cmd) {
		case SIOCSIFMEDIA:

			if(IFM_IEEE80211_HOSTAP & ifr->ifr_media)
			{
				printf("enter SoftAP Mode\n");
				rtw_set_802_11_infrastructure_mode(padapter, Ndis802_11APMode, 0);
				rtw_setopmode_cmd(padapter, Ndis802_11APMode, RTW_CMDF_WAIT_ACK);
			}

			break;
		case SIOCGIFMEDIA:
			ifmr->ifm_active = IFM_IEEE80211 | IFM_IEEE80211_11G;
			ifmr->ifm_current = IFM_IEEE80211;
			ifmr->ifm_mask = 0;
			if ( check_fwstate( &padapter->mlmepriv, WIFI_ASOC_STATE) )
				ifmr->ifm_status |= IFM_AVALID |IFM_ACTIVE;
			else
				ifmr->ifm_status |= IFM_AVALID ;
			ifmr->ifm_count = nlist;
			strlcpy(ifmr->ifm_name, name, IFNAMSIZ);
			
			copyout((caddr_t)list,
			    (caddr_t)ifmr->ifm_ulist,
			    ifmr->ifm_count * sizeof(int));
			break;
	}
	return 0;
}

int rtw_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	_adapter *padapter = ifp->if_softc;
	int error = 0;
	struct ieee80211req *ireq = (struct ieee80211req *)data;

	switch (cmd) {
		case SIOCADDMULTI:
		case SIOCDELMULTI:
			break;
		case SIOCSIFMEDIA:
		case SIOCGIFMEDIA:
			error=rtw_ifmedia_ioctl(ifp, cmd, data);
			break;
		case SIOCG80211:
			error = rtw_ioctl_get80211(padapter, cmd,  ireq);
			break;
		case SIOCS80211:
			error = rtw_ioctl_set80211(padapter, cmd, ireq);
			break;
		case SIOCG80211STATS:
			break;
		case SIOCSIFMTU:
         	        break;
		case SIOCGDRVSPEC:
		case SIOCSDRVSPEC:
			printf("ioctl 0x%08lx not implemented!\n", cmd);
			error = -ENODEV;
			break;
		case SIOCGPRIVATE_0:
			printf("ioctl: SIOCGPRIVATE_0\n");
{
			struct iwreq *piwreq;
			piwreq = (struct iwreq*)data;
			error = rtw_mp_ioctl(padapter, (union iwreq_data*)&piwreq->u);
}
			printf("ioctl: SIOCGPRIVATE_0 return(%d)\n", error);
			break;
		default:
			printf("No such ioctl 0x%08lx\n", cmd);
			error = -EINVAL;
			break;
	}

	return error;

}

