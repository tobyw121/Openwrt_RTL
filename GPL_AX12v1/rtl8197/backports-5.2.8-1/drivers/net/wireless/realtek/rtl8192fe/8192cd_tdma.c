/*
 *  TDMA-MAC routine 
 */

#define _8192CD_TDMA_C_

#include "8192cd.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"

#ifdef RTK_TDMA_ATM

int rtl8192cd_tdma_swq_dequeue(struct rtl8192cd_priv *priv, struct stat_info *pstatd);

void tdma_pause_macid(struct rtl8192cd_priv *priv,struct stat_info *pstat)
{
#ifdef SMP_SYNC
	unsigned long flags = 0;
	int locked = 0;
#endif
	SMP_TRY_LOCK_ASOC_LIST(flags, locked);
	if(!pstat->txpause_flag){
#ifdef CONFIG_WLAN_HAL	
		if (IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 1, REMAP_AID(pstat));	
		} else 
#endif				
		{
#ifdef CONFIG_RTL_8812_SUPPORT				
			RTL8812_MACID_PAUSE(priv, 1, REMAP_AID(pstat));
#endif
		}
			priv->pshare->paused_sta_num++; 
			pstat->txpause_flag = 1;
	}
	if(locked)
		SMP_UNLOCK_ASOC_LIST(flags);
	
}

void tdma_release_macid(struct rtl8192cd_priv *priv,struct stat_info *pstat)
{
#ifdef SMP_SYNC
		unsigned long flags = 0;
		int locked = 0;
#endif
		SMP_TRY_LOCK_ASOC_LIST(flags, locked);

	if ((pstat->state & WIFI_SLEEP_STATE) == 0) 
	if(pstat->txpause_flag){
#ifdef CONFIG_WLAN_HAL	
		if (IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));	
		} else 
#endif				
		{
#ifdef CONFIG_RTL_8812_SUPPORT				
			RTL8812_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
#endif
		}
			priv->pshare->paused_sta_num--; 
			pstat->txpause_flag = 0;
	}
	if(locked)
		SMP_UNLOCK_ASOC_LIST(flags);
}

void check_ttl_active_stanum(struct rtl8192cd_priv *priv)
{
	int i;
	struct stat_info *pstat;

	priv->pshare->atm_ttl_stanum = 0;
	priv->pshare->atm_ttl_active_stanum = 0;
	
	for(i=0; i<NUM_STAT; i++){ 
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)){
			pstat = &(priv->pshare->aidarray[i]->station);
			priv->pshare->atm_ttl_stanum++;//ttl client in this radio 2.4/5G
			if(REMAP_AID(pstat))
				priv->pshare->atm_ttl_active_stanum++;
		}
	}
	
	return;
}

void tdma_exit_polling(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	priv->pshare->tdma_last_polling_is_ap = 2;
	priv->pmib->tdmaControl.tdma_settimer = 0;
	priv->pshare->tdma_last_polling_macID = 0;
	priv->pshare->tdma_last_polling_aid = 0;
	priv->pshare->tdma_swq_en = 0;
	
    if(pstat){
      tdma_release_macid(priv,pstat);
	  pstat->sta_paused = 0;
	}

	check_ttl_active_stanum(priv);
	return;
}

void tdma_update_polling(struct rtl8192cd_priv *priv, unsigned short macID)
{
    panic_printk("[%s]tdma_last_polling_macID:%d,leave macID:%d\n",__FUNCTION__,priv->pshare->tdma_last_polling_macID,macID);
	if(priv->pshare->tdma_last_polling_macID != macID)
		return;
	
#ifdef SMP_SYNC
			unsigned long flags = 0;
			int locked = 0;
#endif


	int i, j, aid;
	struct stat_info	*pstat;
	struct list_head	*phead, *plist, *pfirst, *plast, *plist_bak;
	struct rtl8192cd_priv *priv_first = NULL, *priv_last = NULL, *priv_prev = NULL, *priv_temp;

	for(i = 0; i < RTL8192CD_NUM_VWLAN + 2; i++){
		if(i == 0 && !list_empty(&priv->asoc_list)){
			panic_printk("Find sta on root interface\n");
			priv_first = priv;
			priv_last = priv;
		}
		else if(i == (RTL8192CD_NUM_VWLAN + 1) && IS_DRV_OPEN(GET_ROOT(priv)->pvxd_priv) && !list_empty(&(GET_ROOT(priv)->pvxd_priv)->asoc_list)){
			panic_printk("Find AP on vxd\n");
			if(!priv_first)
				priv_first = GET_ROOT(priv)->pvxd_priv;
			priv_last = GET_ROOT(priv)->pvxd_priv;
		}		
		else if(i > 0 && i <= RTL8192CD_NUM_VWLAN && IS_DRV_OPEN(priv->pvap_priv[i-1]) && !list_empty(&(priv->pvap_priv[i-1])->asoc_list)){
			panic_printk("Find sta on vap%d\n", i-1);
			if(!priv_first)
				priv_first = priv->pvap_priv[i-1];
			priv_last = priv->pvap_priv[i-1];
		}
		else
			continue;
	}

	if(priv_first)
		pfirst = (&priv_first->asoc_list)->next;
	if(priv_last)
		plast = (&priv_last->asoc_list)->prev;
	
	// this is the only associated sta
	if(pfirst == plast){
		panic_printk("%s: only sta, exit polling!\n", __FUNCTION__);
		tdma_exit_polling(priv, NULL);
		return; 
	}
	aid = priv->pshare->tdma_last_polling_aid;
	for(i = RTL8192CD_NUM_VWLAN + 1; i >= 0; i--){
		if(i == 0 && !list_empty(&priv->asoc_list))
			priv_prev = NULL;
        else if(i == (RTL8192CD_NUM_VWLAN + 1) && IS_DRV_OPEN(GET_ROOT(priv)->pvxd_priv) && !list_empty(&(GET_ROOT(priv)->pvxd_priv)->asoc_list)){
			priv = GET_ROOT(priv)->pvxd_priv;
            for(j = i - 2; j >= 0; j--){
				priv_temp = (GET_ROOT(priv))->pvap_priv[j];
				if(IS_DRV_OPEN(priv_temp) && !list_empty(&(priv_temp)->asoc_list)){
					priv_prev = priv_temp;
					break;
				}
				if(j == 0){
					if(!list_empty(&(GET_ROOT(priv))->asoc_list))
						priv_prev = GET_ROOT(priv);
					else
						priv_prev = NULL;
				}
			}
		}
		else if(i > 0 && i <= RTL8192CD_NUM_VWLAN && IS_DRV_OPEN(priv->pvap_priv[i-1]) && !list_empty(&(priv->pvap_priv[i-1])->asoc_list)){
			priv = priv->pvap_priv[i-1];
			//vap0 check root interface
			if(i == 1){
				if(!list_empty(&(GET_ROOT(priv))->asoc_list))
					priv_prev = GET_ROOT(priv);
				else
					priv_prev = NULL;
			}
			//vap1~3 check pervious vap and root interface 
			for(j = i - 2; j >= 0; j--){
				priv_temp = (GET_ROOT(priv))->pvap_priv[j];
				if(IS_DRV_OPEN(priv_temp) && !list_empty(&(priv_temp)->asoc_list)){
					priv_prev = priv_temp;
					break;
				}
				if(j == 0){
					if(!list_empty(&(GET_ROOT(priv))->asoc_list))
						priv_prev = GET_ROOT(priv);
					else
						priv_prev = NULL;
				}
			}
		}
		else
			continue;
		
		phead = &priv->asoc_list;
		priv = GET_ROOT(priv);

		SMP_TRY_LOCK_ASOC_LIST(flags, locked);

		plist = phead->prev;
		while (plist != phead){
			pstat = list_entry(plist, struct stat_info, asoc_list);
			plist = plist->prev;
		
			// shift to the macID ahead of current 
			if(pstat->cmn_info.aid == aid)
			{
                plist_bak = plist;
				if (plist == phead){
					if(priv_prev == NULL)
						priv_prev = priv_last;
					plist = &priv_prev->asoc_list;
					pstat = list_entry(plist->prev, struct stat_info, asoc_list);
				}
				else
					pstat = list_entry(plist, struct stat_info, asoc_list);

				if(pstat->cmn_info.aid == priv->pshare->tdma_last_polling_aid){
					panic_printk("%s: only active sta, exit polling!\n", __FUNCTION__);
					tdma_exit_polling(priv, pstat);
					if(locked)
					SMP_UNLOCK_ASOC_LIST(flags);
					return; 
				}
					
				if(REMAP_AID(pstat) == 0){
					aid = pstat->cmn_info.aid;
					if(plist_bak == phead){ 
						if(priv_prev == priv_last)
							i = RTL8192CD_NUM_VWLAN + 2;
						break;
					}
                    else
						continue;  
				}
                else{	
				priv->pshare->tdma_last_polling_is_ap = 1;
					priv->pshare->tdma_last_polling_aid = pstat->cmn_info.aid;
				priv->pshare->tdma_last_polling_macID = REMAP_AID(pstat);
				    panic_printk("New last polling macID:%d, hwaddr:%02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->tdma_last_polling_macID,
							pstat->cmn_info.mac_addr[0],pstat->cmn_info.mac_addr[1],pstat->cmn_info.mac_addr[2],
				            pstat->cmn_info.mac_addr[3],pstat->cmn_info.mac_addr[4],pstat->cmn_info.mac_addr[5]);
							if(locked)
	                        SMP_UNLOCK_ASOC_LIST(flags);
					goto out_of_loop;
			}
		}
		}
		if(locked)
		SMP_UNLOCK_ASOC_LIST(flags);
	}

out_of_loop:
		
	return;
}

int get_hw_queue_num(struct rtl8192cd_priv *priv)
{
	unsigned int hw_queue_num = 0;

#if IS_RTL88XX_MAC_V3_V4
	if(_GET_HAL_DATA(priv)->MacVersion.is_MAC_v3_v4)
		hw_queue_num = 16;
#endif // IS_RTL88XX_MAC_V3_V4

#if IS_RTL88XX_MAC_V1_V2
	if(_GET_HAL_DATA(priv)->MacVersion.is_MAC_v1_v2) 
		hw_queue_num = 8;
#endif //IS_RTL88XX_MAC_V1_V2

#ifdef CONFIG_WLAN_HAL_8192FE
	if (GET_CHIP_VER(priv) == VERSION_8192F)
		hw_queue_num = 16;
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT) || defined(CONFIG_WLAN_HAL_8814BE)
	if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B || GET_CHIP_VER(priv) == VERSION_8814B)
		hw_queue_num = 8;
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if(GET_CHIP_VER(priv)== VERSION_8188E)
		hw_queue_num = 4;
#endif
    
    return hw_queue_num;
}

int check_hw_queue(struct rtl8192cd_priv *priv)
{	
#if defined (CONFIG_WLAN_HAL_8198F) || defined (CONFIG_WLAN_HAL_8197G) || defined(CONFIG_WLAN_HAL_8192FE)	
	unsigned char Q_aid[16], Q_pkt[16];
#else
	unsigned char Q_aid[8], Q_pkt[8];
#endif
	struct stat_info *pstat2 = NULL;
	int i, j;	

#if defined (CONFIG_WLAN_HAL_8814AE) || defined (CONFIG_WLAN_HAL_8822BE) || defined (CONFIG_WLAN_HAL_8197F) || defined(CONFIG_WLAN_HAL_8822CE) || defined(CONFIG_WLAN_HAL_8812FE)
	if (GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B || GET_CHIP_VER(priv) == VERSION_8197F || GET_CHIP_VER(priv) == VERSION_8822C || GET_CHIP_VER(priv) == VERSION_8812F) {
		for(i=0; i<4; i++) {
			Q_pkt[i] = (RTL_R16(0x1400+i*2) & 0xFFF);
			if(Q_pkt[i]) { 
			Q_aid[i] = (RTL_R8(0x400+i*4+3)>>1) & 0x7f;   //31:25	  7b	
            } else {
            	Q_aid[i] = 0;
            }
			Q_pkt[i+4] = (RTL_R16(0x1408+i*2) & 0xFFF);
			if(Q_pkt[i+4]) { 
			Q_aid[i+4] = (RTL_R8(0x468+i*4+3)>>1) & 0x7f; //31:25	  7b	
            } else {
            	Q_aid[i+4] = 0;
            }
		}
	}
	else
#endif
#if defined(CONFIG_WLAN_HAL_8198F) || defined(CONFIG_WLAN_HAL_8197G)
	if(GET_CHIP_VER(priv)== VERSION_8198F || GET_CHIP_VER(priv)== VERSION_8197G){
		for(i=0;i<16;i++){
			RTL_W8(0x40C,i);
			Q_pkt[i] = RTL_R16(0x404) & 0xFFF;
			if(Q_pkt[i])
				Q_aid[i] = RTL_R32(0x400) & 0xFE000000;
			else
				Q_aid[i] = 0;
		}
	}
	else
#endif
#if defined(CONFIG_WLAN_HAL_8814BE)
    if (GET_CHIP_VER(priv) == VERSION_8814B){
		for(j=0;j<2;j++){		
			RTL_W8(0x414+3,j);
			for(i=0;i<4;i++){
				Q_pkt[i+j*4] = RTL_R32(0x400+i*4) & 0xFFF00;//19:8
				if(Q_pkt[i+j*4])
					Q_aid[i+j*4] = RTL_R8(0x400+i*4);//7:0
				else
					Q_aid[i+j*4] = 0;
			}
		}
	}
	else
#endif
#if defined(CONFIG_WLAN_HAL_8192FE)
	if (GET_CHIP_VER(priv) == VERSION_8192F)
	{
		unsigned char tmpb = RTL_R8(0x414) & ~0x1f;
		for (i=0; i<16; i++) {
			RTL_W8(0x414, tmpb|i);
			Q_pkt[i] = RTL_R8(0x401);
			Q_aid[i] = RTL_R8(0x404) & 0x7f;
		}
	}
	else
#endif   
	{
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
       { /* for 92E, 8812, & 8881A*/
            for(i=0; i<4; i++) {
                Q_pkt[i] = (RTL_R8(0x400+i*4+1) & 0x7f);   // 14:8     7b
                if(Q_pkt[i]) { 
                    Q_aid[i] = (RTL_R8(0x400+i*4+3)>>1) & 0x7f;       //31:25     7b
                } else {
                    Q_aid[i] = 0;
                }
                Q_pkt[i+4] = (RTL_R8(0x468+i*4+1) & 0x7f);   // 14:8     7b
                if(Q_pkt[i+4]) { 
                    Q_aid[i+4] = (RTL_R8(0x468+i*4+3)>>1) & 0x7f; //31:25     7b
                } else {
                    Q_aid[i+4] = 0;
                }
            }
        }
#endif                
#ifdef CONFIG_RTL_88E_SUPPORT
        { /* 88E */
            for(i=0; i<4; i++) {  
                Q_pkt[i] = RTL_R8(0x400+i*4+1);
                if(Q_pkt[i]) {             // 15:8     7b
                    Q_aid[i] = (RTL_R8(0x400+i*4+3)>>2) & 0x3f;       //31:26     7b                           
                } else {
                    Q_aid[i] = 0;
                }
            }
        }
#endif  
	}

	priv->pshare->pq=0;
	for(i=0; i<priv->pshare->hw_queue_num; i++) {
		pstat2 = get_macIDinfo(priv, Q_aid[i]);		
		if( pstat2 && pstat2->txpause_flag && Q_pkt[i])
			priv->pshare->pq++;		
	}
	pstat2 = get_macIDinfo(priv, priv->pshare->tdma_last_polling_macID);

	//Find the corresponding MACID	
	for(i=0; i<priv->pshare->hw_queue_num; i++){
		if(Q_aid[i] == priv->pshare->tdma_last_polling_macID){
			if(pstat2 && Q_pkt[i]){
				pstat2->Q_pkt = Q_pkt[i];				
				tdma_release_macid(priv,pstat2);				
				pstat2->sta_paused = 0;
				return 1;				
			}
		}
	}

    //if(pstat2 && priv->pshare->atm_ttl_active_stanum<8){
	//  tdma_pause_macid(priv,pstat2);
	//  pstat2->sta_paused = 1;
    //}
      
	tdma_check_polling(priv);
  
	return 0;
}

int tdma_settimer(struct rtl8192cd_priv *priv, unsigned char settimer)
{
	unsigned int	timer_duration = 0;
	unsigned int    timer_duration_us;
#ifdef SMP_SYNC
		unsigned long flags = 0;
#endif

	timer_duration_us = priv->pmib->tdmaControl.hwpolling_duration;

	if(settimer) {			
		priv->pmib->tdmaControl.tdma_settimer = 1;
		timer_duration = RTL_MICROSECONDS_TO_GTIMERCOUNTER(timer_duration_us);
		timer_duration = BIT26 | BIT24 | (timer_duration & 0x00FFFFFF);
	}
	else
		priv->pmib->tdmaControl.tdma_settimer = 0;
	
	//RTL_W32(TC4_CTRL, timer_duration);
	RTL_W32(TC3_CTRL, timer_duration);

	return 1;
}

void tdma_timeout(unsigned long task_priv)
{
	unsigned char count1;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat2;
	unsigned long x = 0;
	int locked_xmit = 0;

	if (!IS_DRV_OPEN(priv))
		return;

	if(!priv->pmib->tdmaControl.tdma_settimer){
	    panic_printk("tdma_settimer=0 timeout return!\n");
		return;
   	}
	tdma_settimer(priv, 0);
	
	pstat2 = get_macIDinfo(priv, priv->pshare->tdma_last_polling_macID);		

	if(!pstat2){
		panic_printk("pstat NULL timeout return!tdma_last_polling_macID:%d\n",priv->pshare->tdma_last_polling_macID);
#ifdef SMP_SYNC
	unsigned long flags = 0;
	int locked = 0;
#endif	
		SMP_TRY_LOCK_ASOC_LIST(flags,locked);
        priv->pshare->tdma_last_polling_is_ap = 2;
		if(locked)
			SMP_UNLOCK_ASOC_LIST(flags);
		tdma_check_polling(priv);
		return;
	}

	SAVE_INT_AND_CLI(x);
	SMP_TRY_LOCK_XMIT(x,locked_xmit);
	if(pstat2->atm_sta_time !=0)
		count1 = pstat2->tdma_airtime/priv->pmib->tdmaControl.hwpolling_duration; 	
	else
		count1 = priv->pmib->tdmaControl.tx_duration/priv->pmib->tdmaControl.hwpolling_duration;
	
	priv->pshare->hwpolling_count++;
		
	if(check_hw_queue(priv)){				
		if (priv->pshare->hwpolling_count == count1){ //time slot timeout               		
			if(pstat2 && priv->pshare->atm_ttl_active_stanum<8){
				tdma_pause_macid(priv,pstat2);
				pstat2->sta_paused = 1;
			}
			tdma_check_polling(priv);
		}	
		else  //continue to check hw queue pkts
			tdma_settimer(priv, 1);
	}
		
	RESTORE_INT(x);
	if(locked_xmit)
	SMP_UNLOCK_XMIT(x); 

	return;
}

void tdma_check_polling(struct rtl8192cd_priv *priv)
{	
	int i, j, aid;
	int first_flag = 0;
	struct rtl8192cd_priv *priv_temp=NULL, *priv_first=NULL, *priv_next=NULL;
	struct list_head	*phead=NULL, *plist=NULL, *plist_bak=NULL;
	struct stat_info	*pstat=NULL;

#ifdef SMP_SYNC
	unsigned long flags = 0;
	int locked = 0;
#endif	

	priv->pshare->hwpolling_count = -1;

	// fist time after init, try polling first assocated client
	if(priv->pshare->tdma_last_polling_is_ap == 2)
	{
		panic_printk("%s: enter polling!\n", __FUNCTION__);
		for(i = 0; i < RTL8192CD_NUM_VWLAN + 2; i++){
			if(i > 0){
                if(i == (RTL8192CD_NUM_VWLAN + 1) && IS_DRV_OPEN(GET_ROOT(priv)->pvxd_priv))
					priv = GET_ROOT(priv)->pvxd_priv;				
				else if(i <= RTL8192CD_NUM_VWLAN && IS_DRV_OPEN(priv->pvap_priv[i-1]))
					priv = priv->pvap_priv[i-1];
				else{
					if(i != (RTL8192CD_NUM_VWLAN + 1))
						continue;
					else
						return;
				}
			}
			phead = &priv->asoc_list;
			priv = GET_ROOT(priv);
			if(list_empty(phead)){
				if(i != RTL8192CD_NUM_VWLAN + 1)
					continue;
				else
					return;
			}
			
			SMP_TRY_LOCK_ASOC_LIST(flags,locked);

			plist = phead->next;
			while (plist != phead){
				pstat = list_entry(plist, struct stat_info, asoc_list);
				plist = plist->next;
				
				if(REMAP_AID(pstat) == 0)	
					continue;  
				else{
					panic_printk("%s: find first active sta, start polling!\n", __FUNCTION__);
					priv->pshare->tdma_last_polling_aid = pstat->cmn_info.aid;
					priv->pshare->tdma_last_polling_macID = REMAP_AID(pstat);
					priv->pshare->tdma_last_polling_is_ap = 1;	
					if(check_hw_queue(priv)) 
						tdma_settimer(priv, 1);
					if(locked)
					SMP_UNLOCK_ASOC_LIST(flags);
					return;
				}
			}
			if(locked)
			SMP_UNLOCK_ASOC_LIST(flags);
		}
		panic_printk("%s: cannot find active sta, exit polling!\n", __FUNCTION__);
        tdma_exit_polling(priv, NULL);	
	}

	else if(priv->pshare->tdma_last_polling_is_ap == 1)
	{			
		
			aid = priv->pshare->tdma_last_polling_aid;
			for(i = 0; i < RTL8192CD_NUM_VWLAN + 2; i++){
				int find_flag = 1;
				priv_next = NULL;
				//root_interface
				if(i == 0){
					for(j = 0; j < RTL8192CD_NUM_VWLAN; j++){
						priv_temp = priv->pvap_priv[j];
						if(IS_DRV_OPEN(priv_temp) && !list_empty(&priv_temp->asoc_list)){
							priv_next = priv_temp;
							int find_flag = 0;
							break;
						}
					}
					if(find_flag)
					{
						priv_temp = GET_ROOT(priv)->pvxd_priv;
						if(IS_DRV_OPEN(priv_temp) &&!list_empty(&priv_temp->asoc_list)){
							priv_next = priv_temp;
						}
					}
				}
				else if(i == (RTL8192CD_NUM_VWLAN + 1) && IS_DRV_OPEN(priv->pvxd_priv)){
					priv = priv->pvxd_priv;
					priv_temp = GET_ROOT(priv);
					if(IS_DRV_OPEN(priv_temp) && !list_empty(&priv_temp->asoc_list)){
							priv_next = priv_temp;
					}
	
				}
				//vap_interface(enabled)
				else if(i > 0 && i <= RTL8192CD_NUM_VWLAN && IS_DRV_OPEN(priv->pvap_priv[i-1])){
					priv = priv->pvap_priv[i-1];
					for(j = i; j < RTL8192CD_NUM_VWLAN; j++){
						priv_temp = GET_ROOT(priv)->pvap_priv[j];
						if(IS_DRV_OPEN(priv_temp) && !list_empty(&priv_temp->asoc_list)){
							priv_next = priv_temp;
							int find_flag = 0;
							break;
						}
					}
					if(find_flag)
					{
						priv_temp = GET_ROOT(priv)->pvxd_priv;
						if(IS_DRV_OPEN(priv_temp) &&!list_empty(&priv_temp->asoc_list)){
							priv_next = priv_temp;
						}
					}
				}
				//vap_interface(disabled)
				else
					continue;
					
				phead = &priv->asoc_list;
				if(list_empty(phead)){
					priv = GET_ROOT(priv);
					continue;
				}
					
				if(first_flag == 0){
					priv_first = priv;
					first_flag = 1;
				}

				priv = GET_ROOT(priv);
				SMP_TRY_LOCK_ASOC_LIST(flags,locked);

				plist = phead->next;
				while (plist != phead){
					pstat = list_entry(plist, struct stat_info, asoc_list);
					plist = plist->next;
					
					if(pstat->cmn_info.aid == aid){
						plist_bak = plist;
						if(plist == phead){
							if(priv_next == NULL)
								priv_next = priv_first;
							plist = (&priv_next->asoc_list)->next;
							pstat = list_entry(plist, struct stat_info, asoc_list);
						}
						else
							pstat = list_entry(plist, struct stat_info, asoc_list);
						
						if(pstat->cmn_info.aid == priv->pshare->tdma_last_polling_aid){
							panic_printk("%s: only active sta, exit polling!\n", __FUNCTION__);
							tdma_exit_polling(priv, pstat);
							if(locked)
							SMP_UNLOCK_ASOC_LIST(flags);
							return; 
						}
							
						if(REMAP_AID(pstat) == 0){
							aid = pstat->cmn_info.aid;
							if(plist_bak == phead){
								if(priv_next == priv_first)
									i = -1;
								break;
							}
							else
								continue;  
						}
						else{
							if(locked)
							SMP_UNLOCK_ASOC_LIST(flags);
							priv->pshare->tdma_last_polling_aid = pstat->cmn_info.aid;
							priv->pshare->tdma_last_polling_macID = REMAP_AID(pstat);
							goto out_of_loop;
						}
					}
				}
				if(locked)
				SMP_UNLOCK_ASOC_LIST(flags);
			}
out_of_loop:
		if(pstat != NULL){			
			priv->pshare->tdma_last_polling_is_ap = 1;
				
			if(priv->pshare->atm_sta_time_min == priv->pshare->atm_sta_time_max)
				pstat->tdma_airtime = priv->pmib->tdmaControl.tx_duration;
			else
			{
				for(i=0;i<priv->pshare->atm_ttl_stanum;i++){
					if(pstat->atm_sta_time !=0){
						if((!memcmp(&pstat->cmn_info.mac_addr,&priv->pshare->rf_ft_var.atm_sta_info[i].hwaddr,MACADDRLEN)) && (pstat->atm_sta_time == priv->pshare->atm_sta_time_min))
							pstat->tdma_airtime = priv->pmib->tdmaControl.tx_duration;
						else
							pstat->tdma_airtime = priv->pmib->tdmaControl.tx_duration*pstat->atm_sta_time/priv->pshare->atm_sta_time_min;
					}
				}			
			}				
			
			if(priv->pshare->pq < priv->pshare->hw_queue_num)
				rtl8192cd_tdma_swq_dequeue(priv,pstat);
			else
				panic_printk("hw queue full, not deque!\n");
				
			tdma_settimer(priv, 1);
		}
	}	
	return;
}

int rtl8192cd_tdma_swq_enqueue(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat)
{
	int pri, q_num, q_len;
	pri = get_skb_priority(priv, skb, pstat, NULL);
    q_num = pri_to_qnum(priv, pri);	

	q_len = skb_queue_len(&pstat->tdma_swq.swq_queue[q_num]);

	if(q_len >= priv->pmib->tdmaControl.enque_max)
	{
		struct sk_buff *tmpskb;
		tmpskb = skb_dequeue(&pstat->tdma_swq.swq_queue[q_num]); 	
		if(tmpskb){
			priv->ext_stats.tx_drops++;
			pstat->atm_drop_cnt++;
			rtl_kfree_skb(priv, tmpskb, _SKB_TX_);
		}
	}
//enqueue
	{
		pstat->atm_enque_cnt++;
		skb_queue_tail(&pstat->tdma_swq.swq_queue[q_num],skb);
		pstat->tdma_swq.swq_empty[q_num] = 1;   //not empty
	}
	return 0;
}

int rtl8192cd_tdma_skb_dequeue(struct rtl8192cd_priv *priv, struct stat_info *pstat,  unsigned char qnum,unsigned int dqnum, unsigned char releaseALL)
{
	int count = 0;

    while(1)
    {
		struct sk_buff *tmpskb;
        tmpskb = skb_dequeue(&pstat->tdma_swq.swq_queue[qnum]);
        if (tmpskb == NULL)
            break;
        
#ifdef TX_EARLY_MODE
        if (GET_TX_EARLY_MODE && GET_EM_SWQ_ENABLE) 
            get_tx_early_info(priv, pstat, &pstat->tdma_swq.swq_queue[qnum]);
#endif

        __rtl8192cd_start_xmit_out(tmpskb, pstat, NULL);

        count++;

		if(!releaseALL && count >= dqnum)
			break;
		
    }
	pstat->atm_swq_sent_cnt += count;
	if(skb_queue_len(&pstat->tdma_swq.swq_queue[qnum]) == 0) {
        pstat->tdma_swq.swq_empty[qnum] = 0;  //empty   
    }
    return count;
}

int rtl8192cd_tdma_swq_dequeue(struct rtl8192cd_priv *priv, struct stat_info *pstatd)
{
	int i, qnum, q_len;
	int bd_full = 0;

	unsigned int rate;
	int dqnum;

	if(priv->pshare->rf_ft_var.atm_en==0 || priv->pshare->tdma_swq_en==0){
		panic_printk("[%s]return! atm_en:%d,tdma_swq_en:%d\n",__FUNCTION__,priv->pshare->rf_ft_var.atm_en,priv->pshare->tdma_swq_en);
		tdma_exit_polling(priv, NULL);
		return 0;
	}

#ifdef RTK_AC_SUPPORT			
	if(pstatd->current_tx_rate >= VHT_RATE_ID)
		pstatd->atm_tx_rate = query_vht_rate(pstatd);
	else
#endif			
	if (is_MCS_rate(pstatd->current_tx_rate))
	{
		rate = (unsigned char *)MCS_DATA_RATEStr[(pstatd->ht_current_tx_info&BIT(0))?1:0][(pstatd->ht_current_tx_info&BIT(1))?1:0][(pstatd->current_tx_rate - HT_RATE_ID)];
		pstatd->atm_tx_rate = _atoi(rate, 10);
	}
	else
		pstatd->atm_tx_rate = pstatd->current_tx_rate/2;	

		
	struct stat_info *pstat2;
	priv->pshare->cal_flag++;
	if(priv->pshare->cal_flag >= priv->pshare->atm_ttl_active_stanum)
		priv->pshare->cal_flag = 0;

	if(priv->pshare->cal_flag == 1){
		priv->pshare->atm_max_rate = 0;
		for(i=0; i<NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)){
				pstat2 = &(priv->pshare->aidarray[i]->station);
				
		//tx rate of sta
#ifdef RTK_AC_SUPPORT			
				if(pstat2->current_tx_rate >= VHT_RATE_ID)
					pstat2->atm_tx_rate = query_vht_rate(pstat2);
				else
#endif			
				if (is_MCS_rate(pstat2->current_tx_rate))
				{
					rate = (unsigned char *)MCS_DATA_RATEStr[(pstat2->ht_current_tx_info&BIT(0))?1:0][(pstat2->ht_current_tx_info&BIT(1))?1:0][(pstat2->current_tx_rate - HT_RATE_ID)];
					pstat2->atm_tx_rate = _atoi(rate, 10);
				}
				else
					pstat2->atm_tx_rate = pstat2->current_tx_rate/2;	
					
				//find dqnum calculation base
				if(priv->pshare->atm_sta_time_min == priv->pshare->atm_sta_time_max){
					if(pstat2->atm_tx_rate > priv->pshare->atm_max_rate){
						priv->pshare->atm_max_rate = pstat2->atm_tx_rate;
						priv->pshare->tx_avarage_max = pstat2->tx_avarage;
						for(qnum = VO_QUEUE; qnum >= BK_QUEUE; qnum--)
							priv->pshare->q_len[qnum-1] = skb_queue_len(&pstat2->tdma_swq.swq_queue[qnum]);
					}
				}
				else if(pstat2->atm_sta_time == priv->pshare->atm_sta_time_max){
					priv->pshare->atm_sta_txrate_max = pstat2->atm_tx_rate;
					priv->pshare->tx_avarage_max = pstat2->tx_avarage;
					for(qnum = VO_QUEUE; qnum >= BK_QUEUE; qnum--)
						priv->pshare->q_len[qnum-1] = skb_queue_len(&pstat2->tdma_swq.swq_queue[qnum]);
				}
			}
		}
	}
	for(qnum = VO_QUEUE; qnum >= BK_QUEUE; qnum--){ 
		bd_full = rtl8192cd_swq_bdfull(priv, 0, qnum);
		if(bd_full)
			panic_printk("[%s] swq_bd_full qnum:%d\n",__FUNCTION__,qnum);
			if(!bd_full && pstatd->tdma_swq.swq_empty[qnum]){
				if(priv->pshare->atm_sta_time_min == priv->pshare->atm_sta_time_max){
					if(priv->pshare->atm_max_rate/2 < pstatd->atm_tx_rate)
							dqnum = skb_queue_len(&pstatd->tdma_swq.swq_queue[qnum]);
					else{
						if(priv->pshare->tx_avarage_max > priv->pmib->tdmaControl.idle_thd)
							dqnum = priv->pshare->q_len[qnum-1]*pstatd->atm_tx_rate/priv->pshare->atm_max_rate;
						else
							dqnum = skb_queue_len(&pstatd->tdma_swq.swq_queue[qnum]);
					}
				} 
				else{
					if(pstatd->atm_sta_time == priv->pshare->atm_sta_time_max)
						dqnum = skb_queue_len(&pstatd->tdma_swq.swq_queue[qnum]);	
					else{
						if(priv->pshare->tx_avarage_max > priv->pmib->tdmaControl.idle_thd)
							dqnum = priv->pshare->q_len[qnum-1]*(pstatd->atm_sta_time*pstatd->atm_tx_rate)/(priv->pshare->atm_sta_time_max*priv->pshare->atm_sta_txrate_max);			
						else
							dqnum = skb_queue_len(&pstatd->tdma_swq.swq_queue[qnum]);
					}
				}
				pstatd->dqnum = dqnum;							
				rtl8192cd_tdma_skb_dequeue(priv, pstatd, qnum, dqnum, 0);	
			}
	}
	return 0;
}
#endif

