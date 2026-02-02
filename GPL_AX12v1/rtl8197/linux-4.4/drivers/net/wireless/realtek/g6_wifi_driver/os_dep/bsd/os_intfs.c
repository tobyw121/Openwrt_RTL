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
#define _OS_INTFS_C_

#include <drv_types.h>

#if defined (PLATFORM_LINUX) && defined (PLATFORM_FREEBSD)

#error "Shall be Linux or FreeBSD, but not both!\n"

#endif

#include <sys/unistd.h>


/* module param defaults */
int rtw_chip_version = 0x00;
int rtw_rfintfs = HWPI;
int rtw_lbkmode = 0;//RTL8712_AIR_TRX;
#ifdef DBG_LA_MODE
int rtw_la_mode_en=0;
#endif
int rtw_network_mode = Ndis802_11IBSS;//Ndis802_11Infrastructure;//infra, ad-hoc, auto	  
//NDIS_802_11_SSID	ssid;
int rtw_channel = 1;//ad-hoc support requirement 
int rtw_wireless_mode = WIRELESS_MODE_MAX;
module_param(rtw_wireless_mode, int, 0644);
int rtw_vrtl_carrier_sense = AUTO_VCS;
int rtw_vcs_type = RTS_CTS;//*
int rtw_rts_thresh = 2347;//*
int rtw_frag_thresh = 2346;//*
int rtw_preamble = PREAMBLE_LONG;//long, short, auto
int rtw_scan_mode = 1;//active, passive
int rtw_adhoc_tx_pwr = 1;
int rtw_soft_ap = 0;
//int smart_ps = 1;  
#ifdef CONFIG_POWER_SAVING
int rtw_power_mgnt = 1;
#ifdef CONFIG_IPS_LEVEL_2
int rtw_ips_mode = IPS_LEVEL_2;
#else
int rtw_ips_mode = IPS_NORMAL;
#endif
#else
int rtw_power_mgnt = PS_MODE_ACTIVE;
int rtw_ips_mode = IPS_NONE;
#endif

#ifdef CONFIG_TX_EARLY_MODE
int rtw_early_mode=1;
#endif

#ifndef PLATFORM_FREEBSD
module_param(rtw_ips_mode, int, 0644);
#endif //PLATFORM_FREEBSD

int rtw_radio_enable = 1;
int rtw_long_retry_lmt = 7;
int rtw_short_retry_lmt = 7;
int rtw_busy_thresh = 40;
//int qos_enable = 0; //*
int rtw_ack_policy = NORMAL_ACK;
int rtw_mp_mode = 0;
int rtw_software_encrypt = 0;
int rtw_software_decrypt = 0;	  
 
int rtw_wmm_enable = 1;// default is set to enable the wmm.
int rtw_uapsd_enable = 0;	  
int rtw_uapsd_max_sp = NO_LIMIT;
int rtw_uapsd_acbk_en = 0;
int rtw_uapsd_acbe_en = 0;
int rtw_uapsd_acvi_en = 0;
int rtw_uapsd_acvo_en = 0;
	
#ifdef CONFIG_80211N_HT
int rtw_ht_enable = 1;
// 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160MHz, 4: 80+80MHz
// 2.4G use bit 0 ~ 3, 5G use bit 4 ~ 7
// 0x21 means enable 2.4G 40MHz & 5G 80MHz
int rtw_bw_mode = 0x21;
int rtw_ampdu_enable = 1;//for enable tx_ampdu
#endif

int rtw_rf_config = RF_TYPE_AUTO;
int rtw_low_power = 0;
int rtw_wifi_spec = 0;
int rtw_channel_plan = CONFIG_RTW_CHPLAN;

#ifdef CONFIG_BTC
int rtw_bt_iso = 2;// 0:Low, 1:High, 2:From Efuse
int rtw_bt_sco = 3;// 0:Idle, 1:None-SCO, 2:SCO, 3:From Counter, 4.Busy, 5.OtherBusy
int rtw_bt_ampdu =1 ;// 0:Disable BT control A-MPDU, 1:Enable BT control A-MPDU.
#endif
int rtw_AcceptAddbaReq = _TRUE;// 0:Reject AP's Add BA req, 1:Accept AP's Add BA req.

int  rtw_antdiv_cfg = 2; // 0:OFF , 1:ON, 2:decide by Efuse config

#ifdef CONFIG_USB_AUTOSUSPEND
int rtw_enusbss = 1;//0:disable,1:enable
#else
int rtw_enusbss = 0;//0:disable,1:enable
#endif

int rtw_hwpdn_mode=2;//0:disable,1:enable,2: by EFUSE config

#ifdef CONFIG_HW_PWRP_DETECTION
int rtw_hwpwrp_detect = 1; 
#else
int rtw_hwpwrp_detect = 0; //HW power  ping detect 0:disable , 1:enable
#endif

#ifdef CONFIG_USB_HCI
int rtw_hw_wps_pbc = 1;
#else
int rtw_hw_wps_pbc = 0;
#endif

#ifdef CONFIG_DUALMAC_CONCURRENT
int rtw_dmsp = 0;
#endif	// CONFIG_DUALMAC_CONCURRENT

char* ifname = "wlan%d";

char* rtw_initmac = 0;  // temp mac address if users want to use instead of the mac address in Efuse
#ifndef PLATFORM_FREEBSD
module_param(ifname, charp, 0644);
module_param(rtw_initmac, charp, 0644);
module_param(rtw_channel_plan, int, 0644);
module_param(rtw_chip_version, int, 0644);
module_param(rtw_rfintfs, int, 0644);
module_param(rtw_lbkmode, int, 0644);
module_param(rtw_network_mode, int, 0644);
module_param(rtw_channel, int, 0644);
module_param(rtw_mp_mode, int, 0644);
module_param(rtw_wmm_enable, int, 0644);
module_param(rtw_vrtl_carrier_sense, int, 0644);
module_param(rtw_vcs_type, int, 0644);
module_param(rtw_busy_thresh, int, 0644);
#ifdef CONFIG_80211N_HT
module_param(rtw_ht_enable, int, 0644);
module_param(rtw_bw_mode, int, 0644);
module_param(rtw_ampdu_enable, int, 0644);
module_param(rtw_rx_stbc, int, 0644);
module_param(rtw_ampdu_amsdu, int, 0644);
#endif //CONFIG_80211N_HT
#ifdef CONFIG_80211AC_VHT
module_param(rtw_vht_enable, int, 0644);
#endif //CONFIG_80211AC_VHT
#ifdef CONFIG_BEAMFORMING
module_param(rtw_beamform_cap, int, 0644);
#endif

#ifdef DBG_LA_MODE
module_param(rtw_la_mode_en, int, 0644);
#endif
module_param(rtw_rf_config, int, 0644);
module_param(rtw_power_mgnt, int, 0644);
module_param(rtw_low_power, int, 0644);
module_param(rtw_wifi_spec, int, 0644);

module_param(rtw_antdiv_cfg, int, 0644);

module_param(rtw_enusbss, int, 0644);
module_param(rtw_hwpdn_mode, int, 0644);
module_param(rtw_hwpwrp_detect, int, 0644);

module_param(rtw_hw_wps_pbc, int, 0644);

#ifdef CONFIG_TX_EARLY_MODE
module_param(rtw_early_mode, int, 0644);
#endif

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
char *rtw_adaptor_info_caching_file_path= "/data/misc/wifi/rtw_cache";
module_param(rtw_adaptor_info_caching_file_path, charp, 0644);
#endif //CONFIG_ADAPTOR_INFO_CACHING_FILE

#ifdef CONFIG_LAYER2_ROAMING
uint rtw_max_roaming_times=2;
module_param(rtw_max_roaming_times, uint, 0644);
#endif //CONFIG_LAYER2_ROAMING

#ifdef CONFIG_DUALMAC_CONCURRENT
module_param(rtw_dmsp, int, 0644);
#endif	// CONFIG_DUALMAC_CONCURRENT
#endif //PLATFORM_FREEBSD
//static int netdev_open (struct net_device *pnetdev);
//static int netdev_close (struct net_device *pnetdev);

uint loadparam(_adapter *padapter)
{
	uint status = _SUCCESS;
	struct registry_priv  *registry_par = &padapter->registrypriv;


	registry_par->chip_version = (u8)rtw_chip_version;
	registry_par->rfintfs = (u8)rtw_rfintfs;
	registry_par->lbkmode = (u8)rtw_lbkmode;	
	//registry_par->hci = (u8)hci;
	registry_par->network_mode  = (u8)rtw_network_mode;	

     	_rtw_memcpy(registry_par->ssid.Ssid, "ANY", 3);
	registry_par->ssid.SsidLength = 3;

	registry_par->channel = (u8)rtw_channel;
	registry_par->wireless_mode = (u8)rtw_wireless_mode;

	if (is_supported_24g(registry_par->band_type) && (!is_supported_5g(registry_par->band_type))
		&& (registry_par->channel > 14)) {
		registry_par->channel = 1;
	} else if (is_supported_5g(registry_par->band_type) && (!is_supported_24g(registry_par->band_type))
		&& (registry_par->channel <= 14)) {
		registry_par->channel = 36;
	}
	
	registry_par->vrtl_carrier_sense = (u8)rtw_vrtl_carrier_sense ;
	registry_par->vcs_type = (u8)rtw_vcs_type;
	registry_par->rts_thresh=(u16)rtw_rts_thresh;
	registry_par->frag_thresh=(u16)rtw_frag_thresh;
	registry_par->preamble = (u8)rtw_preamble;
	registry_par->scan_mode = (u8)rtw_scan_mode;
	registry_par->adhoc_tx_pwr = (u8)rtw_adhoc_tx_pwr;
	registry_par->soft_ap=  (u8)rtw_soft_ap;
	//registry_par->smart_ps =  (u8)rtw_smart_ps;  
	registry_par->power_mgnt = (u8)rtw_power_mgnt;
	registry_par->ips_mode = (u8)rtw_ips_mode;
	registry_par->radio_enable = (u8)rtw_radio_enable;
	registry_par->long_retry_lmt = (u8)rtw_long_retry_lmt;
	registry_par->short_retry_lmt = (u8)rtw_short_retry_lmt;
  	registry_par->busy_thresh = (u16)rtw_busy_thresh;
  	//registry_par->qos_enable = (u8)rtw_qos_enable;
    	registry_par->ack_policy = (u8)rtw_ack_policy;
	registry_par->mp_mode = (u8)rtw_mp_mode;	
	registry_par->software_encrypt = (u8)rtw_software_encrypt;
	registry_par->software_decrypt = (u8)rtw_software_decrypt;	  

	 //UAPSD
	registry_par->wmm_enable = (u8)rtw_wmm_enable;
	registry_par->uapsd_enable = (u8)rtw_uapsd_enable;	  
	registry_par->uapsd_max_sp = (u8)rtw_uapsd_max_sp;
	registry_par->uapsd_acbk_en = (u8)rtw_uapsd_acbk_en;
	registry_par->uapsd_acbe_en = (u8)rtw_uapsd_acbe_en;
	registry_par->uapsd_acvi_en = (u8)rtw_uapsd_acvi_en;
	registry_par->uapsd_acvo_en = (u8)rtw_uapsd_acvo_en;

#ifdef CONFIG_80211N_HT
	registry_par->ht_enable = (u8)rtw_ht_enable;
	if (registry_par->ht_enable && is_supported_ht(registry_par->wireless_mode)) {
		registry_par->bw_mode = (u8)rtw_bw_mode;
		registry_par->ampdu_enable = (u8)rtw_ampdu_enable;
	}
#endif
#ifdef DBG_LA_MODE
	registry_par->la_mode_en = (u8)rtw_la_mode_en;
#endif
#ifdef CONFIG_TX_EARLY_MODE
	registry_par->early_mode = (u8)rtw_early_mode;
#endif

	registry_par->rf_config = (u8)rtw_rf_config;
	registry_par->low_power = (u8)rtw_low_power;

	
	registry_par->wifi_spec = (u8)rtw_wifi_spec;

	registry_par->channel_plan = (u8)rtw_channel_plan;

#ifdef CONFIG_BTC
	registry_par->bt_iso = (u8)rtw_bt_iso;
	registry_par->bt_sco = (u8)rtw_bt_sco;
	registry_par->bt_ampdu = (u8)rtw_bt_ampdu;
#endif
	registry_par->bAcceptAddbaReq = (u8)rtw_AcceptAddbaReq;

	registry_par->antdiv_cfg = (u8)rtw_antdiv_cfg;
#ifdef SUPPORT_HW_RFOFF_DETECTED
	registry_par->hwpdn_mode = (u8)rtw_hwpdn_mode;//0:disable,1:enable,2:by EFUSE config
	registry_par->hwpwrp_detect = (u8)rtw_hwpwrp_detect;//0:disable,1:enable
#endif

	registry_par->hw_wps_pbc = (u8)rtw_hw_wps_pbc;

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
	snprintf(registry_par->adaptor_info_caching_file_path, PATH_LENGTH_MAX, "%s",rtw_adaptor_info_caching_file_path);
	registry_par->adaptor_info_caching_file_path[PATH_LENGTH_MAX-1]=0;
#endif

#ifdef CONFIG_LAYER2_ROAMING
	registry_par->max_roaming_times = (u8)rtw_max_roaming_times;
#endif

#ifdef CONFIG_DUALMAC_CONCURRENT
	registry_par->dmsp= (u8)rtw_dmsp;
#endif


	return status;

}

//old compiler needs this... forward declaration
int rtw_init_netdev_name(struct ifnet *pifp);

int rtw_init_netdev_name(struct ifnet *pifp)
{
	//_adapter *padapter = rtw_netdev_priv(pifp);
#ifndef PLATFORM_FREEBSD
#ifdef CONFIG_EASY_REPLACEMENT
	struct net_device	*TargetNetdev = NULL;
	_adapter			*TargetAdapter = NULL;
	struct net 		*devnet = NULL;

	if(padapter->bDongle == 1)
	{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		TargetNetdev = dev_get_by_name("wlan0");
#else
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
		devnet = pnetdev->nd_net;
	#else
		devnet = dev_net(pnetdev);
	#endif
		TargetNetdev = dev_get_by_name(devnet, "wlan0");
#endif
		if(TargetNetdev) {
			RTW_INFO("Force onboard module driver disappear !!!\n");
			TargetAdapter = rtw_netdev_priv(TargetNetdev);
			TargetAdapter->DriverState = DRIVER_DISAPPEAR;

			padapter->pid[0] = TargetAdapter->pid[0];
			padapter->pid[1] = TargetAdapter->pid[1];
			padapter->pid[2] = TargetAdapter->pid[2];
			
			dev_put(TargetNetdev);
			unregister_netdev(TargetNetdev);

			padapter->DriverState = DRIVER_REPLACE_DONGLE;
		}
	}
#endif

	dev_alloc_name(pnetdev, ifname);
#endif //PLATFORM_FREEBSD
    //third paramater need to change by device, modify again
    if_initname(pifp, "rtw", 0);
	//netif_carrier_off(pifp);
	//rtw_netif_stop_queue(pifp);

	return 0;
}

static int netdev_open(struct ifnet *pnetdev);
static int netdev_close(struct ifnet *pnetdev);
int	rtw_ioctl_wrap	(struct ifnet *ifp, u_long cmd, caddr_t data);
int	rtw_ioctl_wrap	(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	//_adapter *padapter = ifp->if_softc;
	int error = 0;

	switch (cmd) {
	case SIOCSIFFLAGS:
		if (ifp->if_flags & IFF_UP) 
		{
			if (ifp->if_drv_flags & IFF_DRV_RUNNING) 
			{
				;
			} 
			else 
			{
				netdev_open(ifp);
				ifp->if_drv_flags |= IFF_DRV_RUNNING;
			}
		} 
		else 
		{
			netdev_close(ifp);
			break;
			
		}
		//sc->sc_if_flags = ifp->if_flags;
		break;
	case SIOCSIFADDR:
		{
			struct ifaddr *ifa = (struct ifaddr *)data;
			if (ifa->ifa_addr->sa_family == AF_INET) {
				ifp->if_drv_flags &= ~IFF_DRV_OACTIVE;
				ifp->if_drv_flags |= IFF_DRV_RUNNING;
				arp_ifinit(ifp, ifa);
			} 
			else
		       	error = EINVAL;
		}
		break;

	default:
		error = rtw_ioctl(ifp, cmd, data);
//		error = EINVAL;
		break;
	}

	return (error);
}

PADAPTER prtw_lock=NULL;


struct ifnet *rtw_init_netdev(void);
struct ifnet *rtw_init_netdev(void)	
{
	_adapter *padapter=NULL;
	struct ifnet *pifp=NULL;
	//temp solution
	//uint8_t myaddr[IEEE80211_ADDR_LEN]={0,0x8,0x1,0x2,0x3,0x4};
	
	padapter = (_adapter *) rtw_zvmalloc(sizeof(_adapter));
	if(!padapter)
		return (struct ifnet *)NULL;
	pifp = if_alloc(IFT_ETHER);
	if (pifp == NULL) 
	{
		rtw_vmfree((u8 *)padapter, sizeof(_adapter));
		return (struct ifnet *)NULL;
	}
	_rtw_mutex_init(&padapter->glock);
	prtw_lock=padapter;
	padapter->pifp = pifp;
	//padapter->pifp = pnetdev;	
	pifp->if_mtu = ETHERMTU;
	pifp->if_softc = padapter;
	pifp->if_flags = IFF_SIMPLEX | IFF_BROADCAST | IFF_MULTICAST;
	pifp->if_start = rtw_xmit_entry_wrap;
	pifp->if_ioctl = rtw_ioctl_wrap;
	//which is init func?
	//ifp->if_init = urtw_init;
	pifp->if_addrlen = IEEE80211_ADDR_LEN;
	IFQ_SET_MAXLEN(&pifp->if_snd, IFQ_MAXLEN);
	pifp->if_snd.ifq_drv_maxlen = IFQ_MAXLEN;
	IFQ_SET_READY(&pifp->if_snd);

	return pifp;

}

u32 rtw_start_drv_threads(_adapter *padapter)
{

	u32 _status = _SUCCESS;
	struct proc *p;
	struct thread *td;
	
#ifndef PLATFORM_FREEBSD
#ifdef CONFIG_SDIO_HCI
	padapter->xmitThread = kernel_thread(rtw_xmit_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->xmitThread < 0)
		_status = _FAIL;
#endif

#ifdef CONFIG_RECV_THREAD_MODE
	padapter->recvThread = kernel_thread(recv_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->recvThread < 0)
		_status = _FAIL;	
#endif

	padapter->cmdThread = kernel_thread(rtw_cmd_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->cmdThread < 0)
		_status = _FAIL;		

#ifdef CONFIG_EVENT_THREAD_MODE
	padapter->evtThread = kernel_thread(event_thread, padapter, CLONE_FS|CLONE_FILES);
	if(padapter->evtThread < 0)
		_status = _FAIL;		
#endif
#else
//	padapter->cmdThread = kernel_thread(rtw_cmd_thread, padapter, CLONE_FS|CLONE_FILES);
//  padapter->cmdThread = kernel_thread(rtw_cmd_thread, padapter, CLONE_FS|CLONE_FILES);
	p=NULL;
	td=NULL;
	  printf("%s(),%d:  \n",__FUNCTION__,__LINE__);
   if(kproc_kthread_add(rtw_cmd_thread, padapter, &p,
		    &td,RFHIGHPID,
	    	    0, "cmdThread","cmdThread")< 0){
		 _status = _FAIL;	
			 padapter->cmdThread=0;
		}
		 else{
			padapter->cmdThread=1;
		 }
		
#endif
	return _status;

}

void rtw_stop_drv_threads (_adapter *padapter)
{

	//Below is to termindate rtw_cmd_thread & event_thread...
	_rtw_up_sema(&padapter->cmdpriv.cmd_queue_sema);
	//_rtw_up_sema(&padapter->cmdpriv.cmd_done_sema);
	if(padapter->cmdThread){
		_rtw_down_sema(&padapter->cmdpriv.terminate_cmdthread_sema);
	}

#ifdef CONFIG_EVENT_THREAD_MODE
        _rtw_up_sema(&padapter->evtpriv.evt_notify);
	if(padapter->evtThread){
		_rtw_down_sema(&padapter->evtpriv.terminate_evtthread_sema);
	}
#endif

#ifdef CONFIG_XMIT_THREAD_MODE
	// Below is to termindate tx_thread...
	_rtw_up_sema(&padapter->xmitpriv.xmit_sema);	
	_rtw_down_sema(&padapter->xmitpriv.terminate_xmitthread_sema);
#endif
	 
#ifdef CONFIG_RECV_THREAD_MODE	
	// Below is to termindate rx_thread...
	_rtw_up_sema(&padapter->recvpriv.recv_sema);
	_rtw_down_sema(&padapter->recvpriv.terminate_recvthread_sema);
#endif


}

u8 rtw_init_default_value(_adapter *padapter);
u8 rtw_init_default_value(_adapter *padapter)
{
	u8 ret  = _SUCCESS;
	struct registry_priv* pregistrypriv = &padapter->registrypriv;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	//xmit_priv
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	//pxmitpriv->rts_thresh = pregistrypriv->rts_thresh;
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;
	
		

	//recv_priv
	

	//mlme_priv
	pmlmepriv->scan_mode = SCAN_ACTIVE;
	
	//qos_priv
	//pmlmepriv->qospriv.qos_option = pregistrypriv->wmm_enable;
	
	//ht_priv
#ifdef CONFIG_80211N_HT		
	pmlmepriv->htpriv.ampdu_enable = _FALSE;//set to disabled
#endif	

	//security_priv
	//rtw_get_encrypt_decrypt_from_registrypriv(padapter);
	psecuritypriv->binstallGrpkey = _FAIL;
	psecuritypriv->sw_encrypt=pregistrypriv->software_encrypt;
	psecuritypriv->sw_decrypt=pregistrypriv->software_decrypt;
	
	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; //open system
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;
	

	//pwrctrl_priv


	//registry_priv
	rtw_init_registrypriv_dev_network(padapter);		
	rtw_update_registrypriv_dev_network(padapter);


	//hal_priv
	padapter->hal_func.init_default_value(padapter);

	//misc.
	RTW_ENABLE_FUNC(padapter, DF_RX_BIT);
	RTW_ENABLE_FUNC(padapter, DF_TX_BIT);
	padapter->bRxRSSIDisplay = 0;
	
	return ret;
}

u8 rtw_reset_drv_sw(_adapter *padapter)
{
	u8	ret8=_SUCCESS;	
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	//hal_priv
	padapter->hal_func.init_default_value(padapter);
	RTW_ENABLE_FUNC(padapter, DF_RX_BIT);
	RTW_ENABLE_FUNC(padapter, DF_TX_BIT);
	padapter->bRxRSSIDisplay = 0;
	pmlmepriv->scan_mode = SCAN_ACTIVE; // 1: active scan ,0 passive scan

	pwrctrlpriv->bips_processing = _FALSE;		

	padapter->xmitpriv.tx_pkts = 0;
	padapter->recvpriv.rx_pkts = 0;

	pmlmepriv->LinkDetectInfo.bBusyTraffic = _FALSE;

	_clr_fwstate_(pmlmepriv, WIFI_UNDER_SURVEY |WIFI_UNDER_LINKING);

#ifdef DBG_CONFIG_ERROR_DETECT
	if (padapter->hal_func.sreset_reset_value)
		padapter->hal_func.sreset_reset_value(padapter);
#endif
	pwrctrlpriv->pwr_state_check_cnts = 0;

	//mlmeextpriv
	mlmeext_set_scan_state(&padapter->mlmeextpriv, SCAN_DISABLE);

	return ret8;
}


u8 rtw_init_drv_sw(_adapter *padapter)
{

	u8	ret8=_SUCCESS;



	if ((rtw_init_cmd_priv(&padapter->cmdpriv)) == _FAIL)
	{
		ret8=_FAIL;
		goto exit;
	}
	
	padapter->cmdpriv.padapter=padapter;
	
	if ((rtw_init_evt_priv(&padapter->evtpriv)) == _FAIL)
	{
		ret8=_FAIL;
		goto exit;
	}
	
	
	if (rtw_init_mlme_priv(padapter) == _FAIL)
	{
		ret8=_FAIL;
		goto exit;
	}


	if(_rtw_init_xmit_priv(&padapter->xmitpriv, padapter) == _FAIL)
	{
		RTW_INFO("Can't _rtw_init_xmit_priv\n");
		ret8=_FAIL;
		goto exit;
	}
		
	if(_rtw_init_recv_priv(&padapter->recvpriv, padapter) == _FAIL)
	{
		RTW_INFO("Can't _rtw_init_recv_priv\n");
		ret8=_FAIL;
		goto exit;
	}

	_rtw_memset((unsigned char *)&padapter->securitypriv, 0, sizeof (struct security_priv));	

	if(_rtw_init_sta_priv(&padapter->stapriv) == _FAIL)
	{
		RTW_INFO("Can't _rtw_init_sta_priv\n");
		ret8=_FAIL;
		goto exit;
	}
	
	padapter->stapriv.padapter = padapter;	

	rtw_init_pwrctrl_priv(padapter);	

	//_rtw_memset((u8 *)&padapter->qospriv, 0, sizeof (struct qos_priv));//move to mlme_priv
	
#ifdef CONFIG_MP_INCLUDED
	if (init_mp_priv(padapter) == _FAIL) {
		RTW_INFO("%s: initialize MP private data Fail!\n", __func__);
	}
#endif

	ret8 = rtw_init_default_value(padapter);

	rtw_dm_init(padapter);

	rtw_led_init(padapter);

#ifdef DBG_CONFIG_ERROR_DETECT
	rtw_sreset_init(padapter);
#endif	

exit:
	

	
	return ret8;
	
}

void rtw_cancel_all_timer(_adapter *padapter)
{

	_cancel_timer_ex(&padapter->mlmepriv.assoc_timer);

	_cancel_timer_ex(&padapter->mlmepriv.scan_to_timer);
	
	_cancel_timer_ex(&padapter->mlmepriv.dynamic_chk_timer);

	// cancel sw led timer
	rtw_led_deinit(padapter);

#ifdef CONFIG_IPS
	// cancel ips timer
	_cancel_timer_ex(&(adapter_to_pwrctl(padapter)->pwr_state_check_timer));
#endif
#ifdef CONFIG_SET_SCAN_DENY_TIMER
	_cancel_timer_ex(&padapter->mlmepriv.set_scan_deny_timer);
	rtw_clear_scan_deny(padapter);
#endif

	// cancel dm  timer
	padapter->hal_func.dm_deinit(padapter);

}

u8 rtw_free_drv_sw(_adapter *padapter)
{


	struct ifnet *pnetdev = (struct ifnet *)padapter->pifp;

	free_mlme_ext_priv(&padapter->mlmeextpriv);
	
	rtw_free_cmd_priv(&padapter->cmdpriv);
	
	rtw_free_evt_priv(&padapter->evtpriv);

	rtw_free_mlme_priv(&padapter->mlmepriv);
	
	//free_io_queue(padapter);
	
	_rtw_free_xmit_priv(&padapter->xmitpriv);
	
	_rtw_free_sta_priv(&padapter->stapriv); //will free bcmc_stainfo here
	
	_rtw_free_recv_priv(&padapter->recvpriv);	

	rtw_free_pwrctrl_priv(padapter);

	//rtw_mfree((void *)padapter, sizeof (padapter));

	padapter->hal_func.free_hal_data(padapter);


	if(pnetdev)
	{
		rtw_free_netdev(pnetdev);
	}
	//adapter is self allocate in freebsd platform, we need to free it
	rtw_vmfree((u8 *)padapter, sizeof(_adapter));

	return _SUCCESS;
	
}

#ifdef PLATFORM_FREEBSD
static int netdev_open(struct ifnet *pnetdev)
{
	uint status;	
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	RTW_INFO("+871x_drv - drv_open, bup=%d\n", padapter->bup);

	if(pwrctrlpriv->ps_flag == _TRUE){
		padapter->net_closed = _FALSE;
		goto netdev_open_normal_process;
	}
		
       if(padapter->bup == _FALSE)
    	{    
		padapter->bDriverStopped = _FALSE;
	 	padapter->bSurpriseRemoved = _FALSE;	 
		padapter->bCardDisableWOHSM = _FALSE;        	
	
		status = rtw_hal_init(padapter);		
		if (status ==_FAIL)
		{			
			goto netdev_open_error;
		}
		
		//RTW_INFO("MAC Address = "MAC_FMT"\n", MAC_ARG(pnetdev->dev_addr));

		
		status=rtw_start_drv_threads(padapter);
		if(status ==_FAIL)
		{			
			goto netdev_open_error;			
		}


		if (init_mlme_ext_priv(padapter) == _FAIL)
		{
			goto netdev_open_error;
		}


		if(padapter->intf_start)
		{
			padapter->intf_start(padapter);
		}

		padapter->bup = _TRUE;
	}
	padapter->net_closed = _FALSE;

	pwrctrlpriv->bips_processing = _FALSE;
	_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);

#ifndef CONFIG_IPS_CHECK_IN_WD
	rtw_set_pwr_state_check_timer(pwrctrlpriv);
#endif 

netdev_open_normal_process:

	RTW_INFO("-871x_drv - drv_open, bup=%d\n", padapter->bup);
		
	return 0;
	
netdev_open_error:

	padapter->bup = _FALSE;
	#ifndef PLATFORM_FREEBSD
	netif_carrier_off(pnetdev);	
	rtw_netif_stop_queue(pnetdev);
	#endif //PLATFORM_FREEBSD
	
	RTW_INFO("-871x_drv - drv_open fail, bup=%d\n", padapter->bup);
	
	return (-1);
	
}
#endif //PLATFORM_FREEBSD


#ifdef CONFIG_IPS
int  ips_netdrv_open(_adapter *padapter)
{
	int status = _SUCCESS;
	padapter->net_closed = _FALSE;
	RTW_INFO("===> %s.........\n",__FUNCTION__);


	padapter->bDriverStopped = _FALSE;
	padapter->bSurpriseRemoved = _FALSE;
	padapter->bCardDisableWOHSM = _FALSE;
	padapter->bup = _TRUE;

	status = rtw_hal_init(padapter);
	if (status ==_FAIL)
	{
		goto netdev_open_error;
	}

	if(padapter->intf_start)
	{
		padapter->intf_start(padapter);
	}

#ifndef CONFIG_IPS_CHECK_IN_WD
	rtw_set_pwr_state_check_timer(adapter_to_pwrctl(padapter));
#endif	
  	_set_timer(&padapter->mlmepriv.dynamic_chk_timer,2000);

	 return _SUCCESS;

netdev_open_error:
	padapter->bup = _FALSE;
	RTW_INFO("-ips_netdrv_open - drv_open failure, bup=%d\n", padapter->bup);

	return _FAIL;
}

void rtw_ips_dev_unload(_adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	RTW_INFO("====> %s...\n",__FUNCTION__);

	padapter->hal_func.set_hw_reg_handler(padapter, HW_VAR_FIFO_CLEARN_UP, 0);

	if(padapter->intf_stop)
	{
		padapter->intf_stop(padapter);
	}

	//s5.
	if(padapter->bSurpriseRemoved == _FALSE)
	{
		rtw_hal_deinit(padapter);
	}

	//s6.
	if(padapter->dvobj_deinit)
	{
		padapter->dvobj_deinit(padapter);
	}


}

int rtw_ips_pwr_up(_adapter *padapter)
{	
	int result;
	systime start_time = rtw_get_current_time();
	RTW_INFO("===>  rtw_ips_pwr_up..............\n");
	rtw_reset_drv_sw(padapter);
	result = ips_netdrv_open(padapter);
 	RTW_INFO("<===  rtw_ips_pwr_up.............. in %dms\n", rtw_get_passing_time_ms(start_time));
	return result;

}

void rtw_ips_pwr_down(_adapter *padapter)
{
	systime start_time = rtw_get_current_time();
	RTW_INFO("===> rtw_ips_pwr_down...................\n");

	padapter->bCardDisableWOHSM = _TRUE;
	padapter->net_closed = _TRUE;
	
	rtw_ips_dev_unload(padapter);
	padapter->bCardDisableWOHSM = _FALSE;
	RTW_INFO("<=== rtw_ips_pwr_down..................... in %dms\n", rtw_get_passing_time_ms(start_time));
}
#endif

int pm_netdev_open(struct ifnet *pifp,u8 bnormal);
int pm_netdev_open(struct ifnet *pifp,u8 bnormal)
{
	int status;
#ifndef PLATFORM_FREEBSD
	if(bnormal)
		status = netdev_open(pnetdev);
#ifdef CONFIG_IPS
	else
		status =  (_SUCCESS == ips_netdrv_open((_adapter *)rtw_netdev_priv(pnetdev)))?(0):(-1);
#endif
#endif //PLATFORM_FREEBSD
    status = _SUCCESS;
	return status;
}

#ifdef PLATFORM_FREEBSD
static int netdev_close(struct ifnet *pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);


	if(adapter_to_pwrctl(padapter)->bInternalAutoSuspend == _TRUE)
	{
		rtw_pwr_wakeup(padapter);
	}
	padapter->net_closed = _TRUE;

/*	if(!padapter->hw_init_completed)
	{
		RTW_INFO("(1)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", padapter->bup, padapter->hw_init_completed);

		padapter->bDriverStopped = _TRUE;

		rtw_dev_unload(padapter);
	}
	else*/
	{
		RTW_INFO("(2)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", padapter->bup, padapter->hw_init_completed);

#ifndef CONFIG_ANDROID
		//s2.
		LeaveAllPowerSaveMode(padapter);
		rtw_disassoc_cmd(padapter, 500, RTW_CMDF_DIRECTLY);
		//s2-2.  indicate disconnect to os
		rtw_indicate_disconnect(padapter, 0, _FALSE);
		//s2-3. 
		rtw_free_assoc_resources(padapter);
		//s2-4.
		rtw_free_network_queue(padapter,_TRUE);
#endif
	}

	RTW_INFO("-871x_drv - drv_close, bup=%d\n", padapter->bup);
	   
	return 0;
	
}
#endif //PLATFORM_FREEBSD

