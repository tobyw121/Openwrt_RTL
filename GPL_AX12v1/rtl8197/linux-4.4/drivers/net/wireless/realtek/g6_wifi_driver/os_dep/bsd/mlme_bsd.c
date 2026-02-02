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


#define _MLME_OSDEP_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <mlme_osdep.h>


#ifdef RTK_DMP_PLATFORM
void Linkup_workitem_callback(struct work_struct *work)
{
	struct mlme_priv *pmlmepriv = container_of(work, struct mlme_priv, Linkup_workitem);
	_adapter *padapter = container_of(pmlmepriv, _adapter, mlmepriv);



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
	kobject_uevent(&padapter->pnetdev->dev.kobj, KOBJ_LINKUP);
#else
	kobject_hotplug(&padapter->pnetdev->class_dev.kobj, KOBJ_LINKUP);
#endif

}

void Linkdown_workitem_callback(struct work_struct *work)
{
	struct mlme_priv *pmlmepriv = container_of(work, struct mlme_priv, Linkdown_workitem);
	_adapter *padapter = container_of(pmlmepriv, _adapter, mlmepriv);



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
	kobject_uevent(&padapter->pnetdev->dev.kobj, KOBJ_LINKDOWN);
#else
	kobject_hotplug(&padapter->pnetdev->class_dev.kobj, KOBJ_LINKDOWN);
#endif

}
#endif

extern void rtw_indicate_wx_assoc_event(_adapter *padapter);
extern void rtw_indicate_wx_disassoc_event(_adapter *padapter);

void rtw_os_indicate_connect(_adapter *adapter)
{


	rtw_indicate_wx_assoc_event(adapter);
	//netif_carrier_on(adapter->pnetdev);

#ifdef RTK_DMP_PLATFORM
	_set_workitem(&adapter->mlmepriv.Linkup_workitem);
#endif


}

static RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];
void rtw_reset_securitypriv( _adapter *adapter )
{
   u8              backupPMKIDIndex = 0;
   u8              backupTKIPCountermeasure = 0x00;

	if(adapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)//802.1x
	{		 
		// Added by Albert 2009/02/18
		// We have to backup the PMK information for WiFi PMK Caching test item.
		//
		// Backup the btkip_countermeasure information.
		// When the countermeasure is trigger, the driver have to disconnect with AP for 60 seconds.

		_rtw_memset( &backupPMKIDList[ 0 ], 0x00, sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );

		_rtw_memcpy( &backupPMKIDList[ 0 ], &adapter->securitypriv.PMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
		backupPMKIDIndex = adapter->securitypriv.PMKIDIndex;
		backupTKIPCountermeasure = adapter->securitypriv.btkip_countermeasure;

		_rtw_memset((unsigned char *)&adapter->securitypriv, 0, sizeof (struct security_priv));

		// Added by Albert 2009/02/18
		// Restore the PMK information to securitypriv structure for the following connection.
		_rtw_memcpy( &adapter->securitypriv.PMKIDList[ 0 ], &backupPMKIDList[ 0 ], sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
		adapter->securitypriv.PMKIDIndex = backupPMKIDIndex;
		adapter->securitypriv.btkip_countermeasure = backupTKIPCountermeasure;

		adapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
		adapter->securitypriv.ndisencryptstatus = Ndis802_11WEPDisabled;

	}
	else //reset values in securitypriv 
	{
		//if(adapter->mlmepriv.fw_state & WIFI_STATION_STATE)
		//{
		struct security_priv *psec_priv=&adapter->securitypriv;

		psec_priv->dot11AuthAlgrthm =dot11AuthAlgrthm_Open;  //open system
		psec_priv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
		psec_priv->dot11PrivacyKeyIndex = 0;

		psec_priv->dot118021XGrpPrivacy = _NO_PRIVACY_;
		psec_priv->dot118021XGrpKeyid = 1;

		psec_priv->ndisauthtype = Ndis802_11AuthModeOpen;
		psec_priv->ndisencryptstatus = Ndis802_11WEPDisabled;
		//}
	}
}

void rtw_os_indicate_disconnect( _adapter *adapter )
{
   //RT_PMKID_LIST   backupPMKIDList[ NUM_PMKID_CACHE ];
  

	rtw_indicate_wx_disassoc_event(adapter);	
//	netif_carrier_off(adapter->pnetdev);

#ifdef RTK_DMP_PLATFORM
	_set_workitem(&adapter->mlmepriv.Linkdown_workitem);
#endif
	 rtw_reset_securitypriv( adapter );


}

void rtw_report_sec_ie(_adapter *adapter,u8 authmode,u8 *sec_ie)
{
#ifndef PLATFORM_FREEBSD	
	uint	len;
	u8	*buff,*p,i;
	union iwreq_data wrqu;



	buff = NULL;
	if(authmode==_WPA_IE_ID_)
	{
		
		buff = rtw_malloc(IW_CUSTOM_MAX);
		
		_rtw_memset(buff,0,IW_CUSTOM_MAX);
		
		p=buff;
		
		p+=sprintf(p,"ASSOCINFO(ReqIEs=");

		len = sec_ie[1]+2;
		len =  (len < IW_CUSTOM_MAX) ? len:IW_CUSTOM_MAX;
			
		for(i=0;i<len;i++){
			p+=sprintf(p,"%02x",sec_ie[i]);
		}

		p+=sprintf(p,")");
		
		_rtw_memset(&wrqu,0,sizeof(wrqu));
		
		wrqu.data.length=p-buff;
		
		wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;
		
		wireless_send_event(adapter->pnetdev,IWEVCUSTOM,&wrqu,buff);

		if(buff)
		    rtw_mfree(buff, IW_CUSTOM_MAX);
		
	}

#else
printf("%s,%d,not implement!!",__FUNCTION__,__LINE__);
#endif
}

#ifdef CONFIG_AP_MODE

void rtw_indicate_sta_assoc_event(_adapter *padapter, struct sta_info *psta)
{
	struct ieee80211_join_event iev;
	struct ifnet *ifp = padapter->pifp;	
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(psta==NULL)
		return;

	if (psta->phl_sta->aid > pstapriv->max_aid)
		return;

	if(pstapriv->sta_aid[psta->aid - 1] != psta)
		return;
	
	
	RTW_INFO("+rtw_indicate_sta_assoc_event\n");
	
	_rtw_memset(&iev, 0, sizeof(iev));

	_rtw_memcpy(iev.iev_addr, psta->hwaddr, ETH_ALEN);
	
	rt_ieee80211msg(ifp, RTM_IEEE80211_JOIN, &iev, sizeof(iev));	

}

void rtw_indicate_sta_disassoc_event(_adapter *padapter, struct sta_info *psta)
{
	struct ieee80211_leave_event iev;
	struct ifnet *ifp = padapter->pifp;	
	struct sta_priv *pstapriv = &padapter->stapriv;

	if(psta==NULL)
		return;

	if (psta->phl_sta->aid > pstapriv->max_aid)
		return;

	if(pstapriv->sta_aid[psta->aid - 1] != psta)
		return;
	
	
	RTW_INFO("+rtw_indicate_sta_disassoc_event\n");

	_rtw_memset(&iev, 0, sizeof(iev));
	
	_rtw_memcpy(iev.iev_addr, psta->hwaddr, ETH_ALEN);
	
	rt_ieee80211msg(ifp, RTM_IEEE80211_LEAVE, &iev, sizeof(iev));
	
}


#ifdef CONFIG_HOSTAPD_MLME

static int mgnt_xmit_entry(struct sk_buff *skb, struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = rtw_netdev_priv(pnetdev);
	_adapter *padapter = (_adapter *)phostapdpriv->padapter;

	//RTW_INFO("%s\n", __FUNCTION__);

	return padapter->hal_func.hostap_mgnt_xmit_entry(padapter, skb);
}

static int mgnt_netdev_open(struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = rtw_netdev_priv(pnetdev);

	RTW_INFO("mgnt_netdev_open: MAC Address:" MAC_FMT "\n", MAC_ARG(pnetdev->dev_addr));


	init_usb_anchor(&phostapdpriv->anchored);
	
	rtw_netif_wake_queue(pnetdev);

	netif_carrier_on(pnetdev);
		
	//rtw_write16(phostapdpriv->padapter, 0x0116, 0x0100);//only excluding beacon 
		
	return 0;	
}
static int mgnt_netdev_close(struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = rtw_netdev_priv(pnetdev);

	RTW_INFO("%s\n", __FUNCTION__);

	usb_kill_anchored_urbs(&phostapdpriv->anchored);

	netif_carrier_off(pnetdev);

	rtw_netif_stop_queue(pnetdev);

	//rtw_write16(phostapdpriv->padapter, 0x0116, 0x3f3f);
	
	return 0;	
}

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,29))
static const struct net_device_ops rtl871x_mgnt_netdev_ops = {
	.ndo_open = mgnt_netdev_open,
       .ndo_stop = mgnt_netdev_close,
       .ndo_start_xmit = mgnt_xmit_entry,
       //.ndo_set_mac_address = r871x_net_set_mac_address,
       //.ndo_get_stats = r871x_net_get_stats,
       //.ndo_do_ioctl = r871x_mp_ioctl,
};
#endif

int hostapd_mode_init(_adapter *padapter)
{
	unsigned char mac[ETH_ALEN];
	struct hostapd_priv *phostapdpriv;
	struct net_device *pnetdev;
	
	pnetdev = rtw_alloc_etherdev(sizeof(struct hostapd_priv));	
	if (!pnetdev)
	   return -ENOMEM;

	//SET_MODULE_OWNER(pnetdev);
       ether_setup(pnetdev);

	//pnetdev->type = ARPHRD_IEEE80211;
	
	phostapdpriv = rtw_netdev_priv(pnetdev);
	phostapdpriv->pmgnt_netdev = pnetdev;
	phostapdpriv->padapter= padapter;
	padapter->phostapdpriv = phostapdpriv;
	
	//pnetdev->init = NULL;
	
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,29))

	RTW_INFO("register rtl871x_mgnt_netdev_ops to netdev_ops\n");

	pnetdev->netdev_ops = &rtl871x_mgnt_netdev_ops;
	
#else

	pnetdev->open = mgnt_netdev_open;

	pnetdev->stop = mgnt_netdev_close;	
	
	pnetdev->hard_start_xmit = mgnt_xmit_entry;
	
	//pnetdev->set_mac_address = r871x_net_set_mac_address;
	
	//pnetdev->get_stats = r871x_net_get_stats;

	//pnetdev->do_ioctl = r871x_mp_ioctl;
	
#endif

	pnetdev->watchdog_timeo = HZ; /* 1 second timeout */	

	//pnetdev->wireless_handlers = NULL;

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	pnetdev->features |= NETIF_F_IP_CSUM;
#endif	

	
	
	if(dev_alloc_name(pnetdev,"mgnt.wlan%d") < 0)
	{
		RTW_INFO("hostapd_mode_init(): dev_alloc_name, fail! \n");		
	}


	//SET_NETDEV_DEV(pnetdev, pintfpriv->udev);


	mac[0]=0x00;
	mac[1]=0xe0;
	mac[2]=0x4c;
	mac[3]=0x87;
	mac[4]=0x11;
	mac[5]=0x12;
				
	_rtw_memcpy(pnetdev->dev_addr, mac, ETH_ALEN);
	

	netif_carrier_off(pnetdev);


	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0)
	{
		RTW_INFO("hostapd_mode_init(): register_netdev fail!\n");
		
		if(pnetdev)
      		{	 
			rtw_free_netdev(pnetdev);
      		}
	}
	
	return 0;
	
}

void hostapd_mode_unload(_adapter *padapter)
{
	struct hostapd_priv *phostapdpriv = padapter->phostapdpriv;
	struct net_device *pnetdev = phostapdpriv->pmgnt_netdev;

	unregister_netdev(pnetdev);
	rtw_free_netdev(pnetdev);
	
}

#endif
#endif

