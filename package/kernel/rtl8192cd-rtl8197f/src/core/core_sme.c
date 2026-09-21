#ifdef __KERNEL__
#ifdef __MIPSEB__
#include <asm/addrspace.h>
#include <linux/module.h>
#endif
#include <linux/list.h>
#include <linux/random.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#ifdef CONFIG_RTL_REPORT_LINK_STATUS
#include <cyg/io/eth/rltk/819x/wrapper/if_status.h>
#endif
#endif
#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./wifi.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_rx.h"
#include "./8192cd_debug.h"
#ifdef CONFIG_FON
#include "fonclient.h"
#endif
#include "./8192cd_psk.h"
#include "./8192cd_security.h"

#ifdef CONFIG_RTL_SIMPLE_CONFIG
#include "8192cd_profile.h"
#endif
#ifdef RTK_NL80211
#include "8192cd_cfg80211.h" 
#endif

#include "./core_sme.h"


#if defined(CONFIG_IEEE80211W_CLI) || defined(CONFIG_IEEE80211R_CLI)
#include "./sha256.h"
#endif

#ifdef CONFIG_IEEE80211R_CLI 
#include "./8192cd_11r_cli.h"
#endif






#if defined(WIFI_QOS_ENHANCE)
//Query sta mac from QOS enhance table
bool qos_enhance_query_sta(struct rtl8192cd_priv *priv, unsigned char *macaddr)
{
	unsigned int keyidx = NUM_STAT;

	//try simple uniform hash first
	keyidx = (unsigned int)((unsigned int)macaddr[4]^(unsigned int)macaddr[5])%(NUM_STAT);
	
	if(!memcmp(macaddr, priv->pshare->rf_ft_var.qos_enhance_sta[keyidx].qos_enhance_sta_addr, MACADDRLEN)){
		return TRUE; //MAC addr amtch
	}else if(!memcmp("\x0\x0\x0\x0\x0\x0", priv->pshare->rf_ft_var.qos_enhance_sta[keyidx].qos_enhance_sta_addr, MACADDRLEN)){
		return FALSE; //Null MAC, and dismiatch
	}else{
		//Then, check all node
		int i = 0;
		for(i=0; i<NUM_STAT; i++){
			if(!memcmp(priv->pshare->rf_ft_var.qos_enhance_sta[i].qos_enhance_sta_addr, macaddr, MACADDRLEN)){			
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//Add sta mac from QOS enhance table
void qos_enhance_add_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned int keyidx = NUM_STAT;

#ifdef SW_TX_QUEUE
	if(priv->pshare->rf_ft_var.swq_dbg == 200)
		panic_printk("%s:%d\n",__FUNCTION__,__LINE__);
#endif

	if(!pstat){
		panic_printk("[%s:%d] pstat==NULL chk!\n",__func__,__LINE__);
		return;
	}

	if(qos_enhance_query_sta(priv, pstat->cmn_info.mac_addr)){
		return;
	}

	//try simple uniform hash first
	keyidx = (unsigned int)((unsigned int)pstat->cmn_info.mac_addr[4]^(unsigned int)pstat->cmn_info.mac_addr[5])%(NUM_STAT);

	if(!memcmp(pstat->cmn_info.mac_addr, priv->pshare->rf_ft_var.qos_enhance_sta[keyidx].qos_enhance_sta_addr, MACADDRLEN)){
		return; //MAC addr match
	}else if(!memcmp("\x0\x0\x0\x0\x0\x0", priv->pshare->rf_ft_var.qos_enhance_sta[keyidx].qos_enhance_sta_addr, MACADDRLEN)){
		memcpy(priv->pshare->rf_ft_var.qos_enhance_sta[keyidx].qos_enhance_sta_addr, pstat->cmn_info.mac_addr, MACADDRLEN); //NULL Node
		pstat->is_qosenhance_sta = 1;
	}else{
		// Then, find a unused node
		int i = 0;
		for(i=0; i<NUM_STAT; i++){
			if(!memcmp("\x0\x0\x0\x0\x0\x0", priv->pshare->rf_ft_var.qos_enhance_sta[i].qos_enhance_sta_addr, MACADDRLEN)){
				memcpy(priv->pshare->rf_ft_var.qos_enhance_sta[i].qos_enhance_sta_addr, pstat->cmn_info.mac_addr, MACADDRLEN);
				pstat->is_qosenhance_sta = 1;
				panic_printk("QOS: ADD %02x:%02x:%02x:%02x:%02x:%02x to QOS enhance table SUCCESS!!!\n",
					pstat->cmn_info.mac_addr[0],pstat->cmn_info.mac_addr[1],pstat->cmn_info.mac_addr[2],
					pstat->cmn_info.mac_addr[3],pstat->cmn_info.mac_addr[4],pstat->cmn_info.mac_addr[5]);
				break;
			}
		}
	}
	return;
}

//Delete sta mac from QOS enhance table
void qos_enhance_del_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i = 0;

	if(!pstat){
		DEBUG_ERR("QOS enhance: qos_enhance_add_sta Input Parameter is NULL!!!\n");
		return;
	}

	for(i=0; i<NUM_STAT; i++){
		if(!memcmp(priv->pshare->rf_ft_var.qos_enhance_sta[i].qos_enhance_sta_addr, pstat->cmn_info.mac_addr, MACADDRLEN)){			
			DEBUG_INFO("QOS: REMOVE %02x:%02x:%02x:%02x:%02x:%02x to QOS enhance table SUCCESS!!!\n",
				pstat->cmn_info.mac_addr[0],pstat->cmn_info.mac_addr[1],pstat->cmn_info.mac_addr[2],
				pstat->cmn_info.mac_addr[3],pstat->cmn_info.mac_addr[4],pstat->cmn_info.mac_addr[5]);
			memset(&priv->pshare->rf_ft_var.qos_enhance_sta[i], 0, sizeof(priv->pshare->rf_ft_var.qos_enhance_sta[i]));
			pstat->is_qosenhance_sta = 0;
			break;
		}
	}
	return;
}

//QOS enhance process
void qos_enhance_proc(struct rtl8192cd_priv *priv)
{
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	WPA_STA_INFO *pStaInfo = NULL;
	unsigned int ulActive = 0, Near_sta = 0, Far_sta = 0, Middle_sta = 0;
	unsigned int ulMiddleTpInterval = 0;
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif

	if((NULL == priv) || !(priv->drv_state & DRV_STATE_OPEN)){
		return;
	}
	
	if(!QOS_ENHANCE_ENABLE(priv) || (!QOS_ENABLE)){
		priv->pshare->rf_ft_var.qos_enhance_active = 0;
		return;
	}

	if(priv->assoc_num<2){
		priv->pshare->rf_ft_var.qos_enhance_active = 0;
		return;
	}

#ifdef SW_TX_QUEUE
	if(priv->pshare->rf_ft_var.swq_dbg == 200)
		panic_printk("%s:%d\n",__FUNCTION__,__LINE__);
#endif    
	
	// MidTPInterval = (TP_HI - TP_MID)/(RSSI_HI-RSSI_LO)
	ulMiddleTpInterval = ((QOS_ENHANCE_TP_HIGH_THRESHOLD(priv) - QOS_ENHANCE_TP_MIDDLE_THRESHOLD(priv))/(QOS_ENHANCE_RSSI_HIGH_THRESHOLD(priv) - QOS_ENHANCE_RSSI_LOW_THRESHOLD(priv)));

	SMP_LOCK_ASOC_LIST(flags);
	phead = &priv->asoc_list;
	plist = phead;	
	while (!list_empty(plist) && (plist = plist->next) != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);

		pStaInfo = pstat->wpa_sta_info;
#ifndef RTK_NL80211
		if((
		#ifdef RTL_WPA2
			(pStaInfo->RSNEnabled & PSK_WPA2) ||
		#endif
			(pStaInfo->RSNEnabled & PSK_WPA)) && (PSK_STATE_PTKINITDONE != pStaInfo->state)){
			if(2 == priv->assoc_num){
				break;
			}else{
				continue;
			}
		}
#endif
#ifdef SW_TX_QUEUE
		if(priv->pshare->rf_ft_var.swq_dbg == 200){
			panic_printk("%s:%d\n",__FUNCTION__,__LINE__);
			panic_printk("rssi=%d, txavg=%d\n", pstat->rssi, (pstat->tx_avarage>>7));
		}
#endif
		if ((pstat->tx_avarage>>7) > 1)	//Tx > 1Kbps
		{
			if(pstat->rssi > QOS_ENHANCE_RSSI_HIGH_THRESHOLD(priv)){ //STA with strong RSSI
				if(((int)((unsigned int)(pstat->tx_avarage>>7))) < QOS_ENHANCE_TP_HIGH_THRESHOLD(priv)){ //low throughput
					qos_enhance_add_sta(priv, pstat);
					ulActive |= 0x1;
				} else { // high throughput
					// leave high rssi sta in qos_enhance_sta when tp > high_tp*2
					if(((int)((unsigned int)(pstat->tx_avarage>>7))) > 2*QOS_ENHANCE_TP_HIGH_THRESHOLD(priv))
					qos_enhance_del_sta(priv, pstat);
				}
			}		
			else if(pstat->rssi < QOS_ENHANCE_RSSI_LOW_THRESHOLD(priv)){ //STA with weak RSSI
				if(pstat->rssi > QOS_ENHANCE_RSSI_EDGE_THRESHOLD(priv)) { //weak RSSI STA but not in edge
					if(((int)((unsigned int)(pstat->tx_avarage>>7))) < QOS_ENHANCE_TP_LOW_THRESHOLD(priv)){ //low throughput
						qos_enhance_add_sta(priv, pstat);
						ulActive |= 0x1;
					} else { // high throughput
						qos_enhance_del_sta(priv, pstat);
					}
				}else{ // edge STA
					qos_enhance_del_sta(priv, pstat);
				}
			}else{ //middle RSSI case
				if(((int)((unsigned int)(pstat->tx_avarage >>7))) < ulMiddleTpInterval*(pstat->rssi - QOS_ENHANCE_RSSI_LOW_THRESHOLD(priv)) + QOS_ENHANCE_TP_MIDDLE_THRESHOLD(priv)){ //low throughput
					qos_enhance_add_sta(priv, pstat);
					ulActive |= 0x1;
				} else { // high throughput
					qos_enhance_del_sta(priv, pstat);
				}
			}
		}
		// for not qos_enhance sta
		if ((priv->pshare->rf_ft_var.qos_enhance_enable == 2) && 
			(((pstat->tx_avarage >> 7) > QOS_ENHANCE_TP_HIGH_THRESHOLD(priv)) || ((pstat->rx_avarage >> 7) > QOS_ENHANCE_TP_HIGH_THRESHOLD(priv)))) {
			if (pstat->rssi > QOS_ENHANCE_RSSI_HIGH_THRESHOLD(priv)) {
				pstat->is_qosenhance_farsta = 0;
				Near_sta++;
			} else if (pstat->rssi < QOS_ENHANCE_RSSI_LOW_THRESHOLD(priv)) {
				pstat->is_qosenhance_farsta = 1;
				Far_sta++;				
			} else {
				pstat->is_qosenhance_farsta = 2;
				Middle_sta++;	
			}

			if ((Near_sta && Far_sta) || (Near_sta && Middle_sta))
				ulActive |= 0x1;
		}
	}

	SMP_UNLOCK_ASOC_LIST(flags);
	if(ulActive & 0x1){
		priv->pshare->rf_ft_var.qos_enhance_active = 1;
	} else {
		priv->pshare->rf_ft_var.qos_enhance_active = 0;
		memset(&priv->pshare->rf_ft_var.qos_enhance_sta, 0, sizeof(priv->pshare->rf_ft_var.qos_enhance_sta));
	}
	return;
}
#endif




