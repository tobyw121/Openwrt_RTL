/******************************************************************************
 *
 * Copyright(c) 2007 - 2020 Realtek Corporation.
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

#include <drv_types.h>
#include <linux/jhash.h>
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
#include <net/rtl/rtl_brsc.h>
#endif

#ifdef CONFIG_RTW_A4_STA

#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
extern int rtk_fc_external_lut_process(bool add, char *wlan_devname, char *sta_mac);
void rtw_del_fc_external_lut_process(_adapter *adapter, unsigned char *mac_list, unsigned int mac_cnt)
{
	int i;
	unsigned char *mac = mac_list;

	if(mac_cnt > MAX_A4_TBL_NUM)
	{
		DBGP("error mac_list size exceed db entry size\n");
		return;
	}
	for(i = 0; i < mac_cnt; i++)
	{
		rtk_fc_external_lut_process(0, adapter->pnetdev->name, mac);
		mac += ETH_ALEN;
	}
	return;
}
#endif

#ifdef CONFIG_A4_LOOPBACK
static u32 a4_loop_mac_hash(u8 *networkAddr)
{
    u32 x;

    x = networkAddr[0] ^ networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^
		networkAddr[4] ^ networkAddr[5];

    return x & (A4_LOOP_HASH_SIZE - 1);
}

struct rtw_a4_loopback_entry *rtw_find_a4_loop_entry(_adapter *adapter, unsigned char *mac)
{
	struct rtw_a4_loopback_entry *entry=NULL;
	if(!adapter->a4_enable || !MLME_IS_STA(adapter))
		return NULL;

	if(adapter->a4_loop_cache && EQ_MAC_ADDR(mac, adapter->a4_loop_cache->mac))
	{
		return adapter->a4_loop_cache;
	}
	else
	{
		hlist_for_each_entry(entry,&adapter->a4_loop_list[a4_loop_mac_hash(mac)], hnode)
		{
			if(EQ_MAC_ADDR(mac,entry->mac))
			{
				adapter->a4_loop_cache = entry;
				return entry;
			}
		}
	}
	return NULL;
}

int rtw_check_a4_loop_entry(_adapter *adapter, unsigned char *mac)
{
	struct rtw_a4_loopback_entry *entry=NULL;

	if(!adapter->a4_enable || !MLME_IS_STA(adapter))
		return 0;

	entry = rtw_find_a4_loop_entry(adapter, mac);
	if(entry)
	{
		if(rtw_get_passing_time_ms(entry->stime) <= 2000)
			return 1;
	}

	return 0;
}

void rtw_update_a4_loop_entry(_adapter *adapter, unsigned char *mac)
{
	struct rtw_a4_loopback_entry *entry=NULL;

	if(!adapter->a4_enable || !MLME_IS_STA(adapter))
		return;

	if(adapter->a4_loop_entry)
	{
		entry = rtw_find_a4_loop_entry(adapter, mac);
		if(NULL == entry) {
			entry = &adapter->a4_loop_entry[adapter->replace_idx++];
			if(adapter->replace_idx >= MAX_A4_LOOPBACK_ENTRY_NUM)
				adapter->replace_idx = 0;
			memcpy(entry->mac,mac,ETH_ALEN);
			hlist_del_init(&entry->hnode);
			hlist_add_head(&entry->hnode,&adapter->a4_loop_list[a4_loop_mac_hash(mac)]);
			adapter->a4_loop_cache = entry;
		}
		entry->stime = rtw_get_current_time();
	}
	return;
}

void rtw_invalid_a4_loop_entry(_adapter *adapter, unsigned char *mac)
{
	struct rtw_a4_loopback_entry *entry=NULL;

	if(!adapter->a4_enable || !MLME_IS_STA(adapter))
		return;

	entry = rtw_find_a4_loop_entry(adapter, mac);
	if(entry)
		entry->stime = 0;

	return;
}

#endif

static u32 mac_hash(u8 *networkAddr)
{
    u32 x;

    x = networkAddr[0] ^ networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^
    	networkAddr[4] ^ networkAddr[5];

    return x & (A4_STA_HASH_SIZE - 1);
}

static void mac_hash_link(_adapter *adapter,
	struct rtw_a4_db_entry *entry, s8 hash)
{
    entry->next_hash = adapter->machash[hash];

    if (entry->next_hash)
        entry->next_hash->pprev_hash = &entry->next_hash;

    adapter->machash[hash] = entry;

    entry->pprev_hash = &adapter->machash[hash];
}

static void mac_hash_unlink(struct rtw_a4_db_entry *entry)
{
    *(entry->pprev_hash) = entry->next_hash;

    if (entry->next_hash != NULL)
    	entry->next_hash->pprev_hash = entry->pprev_hash;

    entry->next_hash = NULL;
    entry->pprev_hash = NULL;
}

static struct rtw_a4_db_entry *alloc_a4_entry(_adapter *adapter)
{
#ifdef CONFIG_DYN_ALLOC_A4_TBL

	struct rtw_a4_db_entry *entry = NULL;

	entry = (struct rtw_a4_db_entry *)rtw_zmalloc(sizeof(struct rtw_a4_db_entry));

	return entry;
#else
    u32 i;

    for (i = 0; i < MAX_A4_TBL_NUM; i++) {
        if (!adapter->a4_entry[i].used) {
            adapter->a4_entry[i].used = 1;
            return &adapter->a4_entry[i].entry;
        }
    }
#endif
    return NULL;
}

static void free_a4_entry(_adapter *adapter,  struct rtw_a4_db_entry *entry)
{
#ifdef CONFIG_DYN_ALLOC_A4_TBL
	rtw_mfree(entry, sizeof(struct rtw_a4_db_entry));
#else
	u32 i;

	for (i=0; i<MAX_A4_TBL_NUM; i++) {
		if (adapter->a4_entry[i].used &&
			(entry == &adapter->a4_entry[i].entry)) {
			adapter->a4_entry[i].used = 0;
			break;
		}
	}
#endif
}

void a4_del_source_db_entry(_adapter *adapter, u8 *mac)
{
	struct rtw_a4_db_entry *db;
	u32 hash;

	hash = mac_hash(mac);
	machash_spinlock_bh(hash);
	db = adapter->machash[hash];

	while (db != NULL) {
		if (!memcmp(db->mac, mac, ETH_ALEN)) {
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
			rtw_flush_brsc_info(NULL, mac);
#endif
			mac_hash_unlink(db);
			free_a4_entry(adapter, db);
#if 0
	printk("%s A4 STA DEL emac:%pM, wmac:%pM\n",
		adapter->pnetdev->name, db->mac, db->psta->phl_sta->mac_addr);
#endif
			machash_spinunlock_bh(hash);
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
			rtw_del_fc_external_lut_process(adapter, mac, 1);
#endif
			return;
		}

		db = db->next_hash;
	}
	machash_spinunlock_bh(hash);
}

void a4_del_wl_sta(_adapter *adapter, struct sta_info *psta)
{
	struct sta_priv *pstapriv = &adapter->stapriv;
	u8 updated = _FALSE;
/* CONFIG_RTW_A4_STA, MLME_IS_AP ?? */

#ifdef CONFIG_AP_MODE
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);

	if (_FALSE == rtw_is_list_empty(&psta->asoc_list)) {
		rtw_list_delete(&psta->asoc_list);
		pstapriv->asoc_list_cnt--;

		updated = ap_free_sta(adapter, psta, _TRUE, WLAN_REASON_DEAUTH_LEAVING, _TRUE, _TRUE);
	}

	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	associated_clients_update(adapter, updated, STA_INFO_UPDATE_ALL);
#endif
}

void a4_del_source(_adapter *adapter, u8 *mac)
{
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta = NULL;

	psta = rtw_get_stainfo(pstapriv, mac);

	if (psta)
		a4_del_wl_sta(adapter, psta);

	a4_del_source_db_entry(adapter, mac);
}

void a4_clear_source_overlap(_adapter *adapter, u8 *mac)
{
/* CONFIG_RTW_A4_STA, todo: clear overlapping source */

}

int a4_l2uf_pkt_check(union recv_frame *rframe)
{
	int i = 0;
	struct rx_pkt_attrib *rattrib = &rframe->u.hdr.attrib;
	char l2ufHex[][6] = {
		{0x00, 0x01, 0xaf, 0x81, 0x01, 0x02}, //MTK l2uf
		{0x00, 0x00, 0xf5, 0x81, 0x80, 0x00}, //QCA l2uf
		{0x00, 0x01, 0xaf, 0x81, 0x01, 0x00}, //BCM l2uf
	};

	 /* offset  = Fctrl(2) + duration(2) + hdrlen(26) + Qos(2) + ccmp(8)*/
	char *data = rframe->u.hdr.pkt->data + 4 + 26 + WHDR_QOS_LENGTH + 8; 

	for(i = 0; i < ARRAY_SIZE(l2ufHex); i++)
	{
		if(memcmp(data, l2ufHex[i], 6) == 0)
		{
			return 1;
		}
	}

	return 0;
}

s32 core_a4_data_validate_hdr(_adapter *adapter,
	union recv_frame *rframe, struct sta_info **ppsta)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	struct rx_pkt_attrib *rattrib = &rframe->u.hdr.attrib;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *whdr = rframe->u.hdr.wlan_hdr;
	#else
	u8 *whdr = get_recvframe_data(rframe);
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	u8 is_ra_bmc = 0;
	s32 ret = _FAIL;

	adapter->cnt_a4_rx++;

	if (!(MLME_STATE(adapter) & WIFI_ASOC_STATE))
		goto exit;

	if (!adapter_en_a4(adapter))
		goto exit;

	is_ra_bmc = IS_MCAST(GetAddr1Ptr(whdr)) ? 1 : 0;
	_rtw_memcpy(rattrib->ra, GetAddr1Ptr(whdr), ETH_ALEN);
	_rtw_memcpy(rattrib->ta, get_addr2_ptr(whdr), ETH_ALEN);
	_rtw_memcpy(rattrib->dst, GetAddr3Ptr(whdr), ETH_ALEN); /* may change after checking AMSDU subframe header */
	_rtw_memcpy(rattrib->src, GetAddr4Ptr(whdr), ETH_ALEN); /* may change after checking AMSDU subframe header */

	if (!is_ra_bmc)
		_rtw_memcpy(rattrib->bssid, GetAddr1Ptr(whdr), ETH_ALEN);

	*ppsta = rtw_get_stainfo(stapriv, rattrib->ta);

	if (*ppsta == NULL) {
		if (!is_ra_bmc && !IS_RADAR_DETECTED(adapter_to_rfctl(adapter))) {
			#ifndef CONFIG_CUSTOMER_ALIBABA_GENERAL
			RTW_INFO(FUNC_ADPT_FMT" issue_deauth_g6 to "MAC_FMT" with reason(7), unknown TA\n"
				, FUNC_ADPT_ARG(adapter), MAC_ARG(rattrib->ta));
			issue_deauth_g6(adapter, rattrib->ta, WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
			#endif
		}
		ret = RTW_RX_HANDLED;
		goto exit;
	}

	/* sta->flags |= WLAN_STA_A4; */

/* CONFIG_RTW_A4_STA, todo: */

	if (MLME_IS_AP(adapter)) {

		process_pwrbit_data(adapter, rframe, *ppsta);

		if (WIFI_QOS_DATA_TYPE == (get_frame_sub_type(whdr) & WIFI_QOS_DATA_TYPE))
			process_wmmps_data(adapter, rframe, *ppsta);

		if (get_frame_sub_type(whdr) & BIT(6)) {
			/* No data, will not indicate to upper layer, temporily count it here */
			count_rx_stats(adapter, rframe, *ppsta);
			ret = RTW_RX_HANDLED;
			goto exit;
		}
	} else if (MLME_IS_STA(adapter)) {

		if (!rattrib->amsdu && IS_MCAST(rattrib->dst)
			&& core_rx_a4_gptr_check(adapter, rattrib->src)
		) {
			if(a4_l2uf_pkt_check(rframe)) {
				core_a4_gptr_invalid_by_mac(adapter, rattrib->src);
			} else {
				/* will not indicate to upper layer, temporily count it here */
				///count_rx_stats(adapter, rframe, *sta);
				ret = RTW_RX_HANDLED;
				goto exit;
			}
		}
	}

	ret = _SUCCESS;

exit:
	return ret;
}

int core_a4_rx_amsdu_act_check(_adapter *adapter, const u8 *da, const u8 *sa)
{
	int act = RTW_RX_MSDU_ACT_INDICATE;

	if (IS_MCAST(da) && core_rx_a4_gptr_check(adapter, sa)) {
		act = 0;
	}

exit:
	return act;
}


/* CONFIG_RTW_A4_STA, todo: inline */

void core_a4_upt_sta_list(_adapter *adapter, struct sta_info *psta)
{
	struct list_head *phead, *plist;
	struct sta_info *psta_a4;

	/* CONFIG_RTW_A4_STA, todo: check sta flag ?? */

	if (!psta || !psta->phl_sta) {
		DBGP("error psta || psta->phl_sta \n");
		return;
	}

	phead = &adapter->a4_sta_list;
	plist = phead->next;

	while ((plist != phead) && (plist != NULL)) {

		psta_a4 = list_entry(plist, struct sta_info, a4_sta_list);

		if (!psta_a4 || !psta_a4->phl_sta) {
			DBGP("error psta_a4 || psta_a4->phl_sta \n");
			list_del(plist);
			goto retry_upt_a4_sta_list;
		}

		if (!memcmp(psta->phl_sta->mac_addr,
			psta_a4->phl_sta->mac_addr, ETH_ALEN)) {

			/* ASSERT(psta == psta_a4); */
			break;
		}

		plist = plist->next;
	}

	if (plist == phead)
		list_add_tail(&psta->a4_sta_list, &adapter->a4_sta_list);

	psta->flags |= WLAN_STA_A4;
	return;

retry_upt_a4_sta_list:
	core_a4_upt_sta_list(adapter, psta);

}

/* A4_AMSDU */
void core_a4_sta_cleanup(_adapter *adapter, struct sta_info *psta)
{
	struct list_head *phead, *plist;
	struct rtw_a4_db_entry *db_tmp0, *db_tmp1;
	struct sta_info *psta_a4;
	u32 i;
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	unsigned char mac[MAX_A4_TBL_NUM*ETH_ALEN + 1] = {0};
	unsigned int mac_cnt = 0;
#endif

	DBGP("psta = 0x%p \n", psta);

	phead = &adapter->a4_sta_list;
	plist = phead->next;

	while ((plist != NULL) && (plist != phead)) {
		psta_a4 = list_entry(plist, struct sta_info, a4_sta_list);
		if (psta_a4 == psta) {
			list_del(plist);
			break;
		}
		plist = plist->next;
	}

	for (i = 0; i < A4_STA_HASH_SIZE; i++) {
		machash_spinlock_bh(i);
		db_tmp0 = adapter->machash[i];

		while (NULL != db_tmp0) {
			db_tmp1 = db_tmp0->next_hash;
			if (psta == db_tmp0->psta) {
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
				rtw_flush_brsc_info(NULL, db_tmp0->mac);
#endif
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
				memcpy(&mac[mac_cnt*ETH_ALEN], db_tmp0->mac, ETH_ALEN);
				mac_cnt++;
#endif
				mac_hash_unlink(db_tmp0);
				free_a4_entry(adapter, db_tmp0);
			}
			db_tmp0 = db_tmp1;
		}
		machash_spinunlock_bh(i);
	}

#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	if(mac_cnt)
		rtw_del_fc_external_lut_process(adapter, mac, mac_cnt);
#endif

	return;
}

void core_a4_upt_source_db(_adapter *adapter,
	struct sta_info *psta, u8 *mac)
{
	struct rtw_a4_db_entry *db;
	u32 hash;
	u32 tmpbuf[15];

	/* ASSERT(mac); */

	hash = mac_hash(mac);

	machash_spinlock_bh(hash);
	db = adapter->machash[hash];

	while (db != NULL) {
		if (!memcmp(db->mac, mac, ETH_ALEN)) {
			db->psta = psta;
			db->ageing_timer = jiffies;
			db->m2u_ignore_cnt = 0;
			machash_spinunlock_bh(hash);
			return;
		}

		db = db->next_hash;
	}
	machash_spinunlock_bh(hash);
	if (memcmp(psta->phl_sta->mac_addr, mac, ETH_ALEN))
		a4_del_source(adapter, mac);
	machash_spinlock_bh(hash);
	db = alloc_a4_entry(adapter);

	if (!db) {
		DBGP("alloc_entry() failed for a4_sta_db_entry!\n");
		machash_spinunlock_bh(hash);
		return;
	}

	memcpy(db->mac, mac, ETH_ALEN);
	db->psta= psta;
	db->ageing_timer = jiffies;
	db->link_time = 0;/* A4_CNT */
	db->m2u_ignore_cnt = 0;

	mac_hash_link(adapter, db, hash);

#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	rtk_fc_external_lut_process(1, adapter->pnetdev->name, db->mac);
#endif

#if 1
	printk("%s A4 STA Add emac("MAC_FMT"), wmac("MAC_FMT")\n",
		adapter->pnetdev->name, MAC_ARG(db->mac), MAC_ARG(db->psta->phl_sta->mac_addr));
#endif

	machash_spinunlock_bh(hash);
	a4_clear_source_overlap(adapter, mac);
	return;
}

void core_a4_upt_db(_adapter *adapter, struct sta_info *psta, u8 *sa)
{
	core_a4_upt_sta_list(adapter, psta);

	core_a4_upt_source_db(adapter, psta, sa);
}

void core_a4_update_m2u_ignore_cnt(_adapter *adapter, u8 *mac)
{
	struct rtw_a4_db_entry *db;
	u32 hash = mac_hash(mac);

	/* ASSERT(mac); */
	machash_spinlock_bh(hash);
	db = adapter->machash[hash];

	while (db != NULL) {
		if (!memcmp(db->mac, mac, ETH_ALEN)) {
			db->m2u_ignore_cnt++;
			RTW_INFO("%s()-%d: %s A4 STA fwd emac:%pM, wmac:%pM, m2u_ignore_cnt: %d\n",
				__FUNCTION__, __LINE__, adapter->pnetdev->name, db->mac,
				db->psta->phl_sta->mac_addr, db->m2u_ignore_cnt);
			break;
		}
		db = db->next_hash;
	}
	machash_spinunlock_bh(hash);
	return;
}

struct sta_info *core_a4_get_fwd_sta(_adapter *adapter, u8 *mac)
{
	struct sta_info *psta = NULL;
	struct rtw_a4_db_entry *db;
	u32 hash = mac_hash(mac);
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	int need_del_db = 0;
#endif

	/* ASSERT(mac); */
	machash_spinlock_bh(hash);
	db = adapter->machash[hash];

	while (db != NULL) {
		if (!memcmp(db->mac, mac, ETH_ALEN)) {
#if 0
	printk("%s A4 STA fwd emac:%pM, wmac:%pM\n",
		adapter->pnetdev->name, db->mac, db->psta->phl_sta->mac_addr);
#endif
			psta = db->psta;

			if (!psta || !psta->phl_sta || db->m2u_ignore_cnt >= 5) {
				DBGP("error psta || psta->phl_sta \n");
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
				rtw_flush_brsc_info(NULL, db->mac);
#endif
				mac_hash_unlink(db);
				free_a4_entry(adapter, db);
				psta = NULL;
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
				need_del_db = 1;
#endif
			}

			break;
		}
		db = db->next_hash;
	}
	machash_spinunlock_bh(hash);
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	if(need_del_db)
		rtw_del_fc_external_lut_process(adapter, mac, 1);
#endif
	return psta;
}

sint core_a4_fwd_pkt_to_sta(union recv_frame *prframe, struct sta_info *psta)
{
/* CONFIG_RTW_A4_STA, todo: shall be osdep */
/* CONFIG_RTW_A4_STA, todo: what if sta from diff adapter ?? */

	_adapter *adapter = prframe->u.hdr.adapter;
	struct net_device *pnetdev = (struct net_device *)adapter->pnetdev;
	struct sk_buff *pkt = NULL;

	if(wlanhdr_to_ethhdr(prframe) != _SUCCESS)
		return _FAIL;

	pkt = prframe->u.hdr.pkt;
	pkt->dev = pnetdev;

	rtw_os_tx(pkt, pnetdev);

#ifdef CONFIG_RTW_CORE_RXSC
	if (prframe->u.hdr.rxsc_entry)
		prframe->u.hdr.rxsc_entry->forward_to = RXSC_FWD_STA;
#endif
	return _SUCCESS;
}

s32 core_a4_rx_msdu_act_check(union recv_frame *prframe)
{
	_adapter *adapter = prframe->u.hdr.adapter;
	struct sta_info *fwd_sta = NULL;
	struct sta_info *rframe_sta = prframe->u.hdr.psta;
	struct rx_pkt_attrib *rattrib = &prframe->u.hdr.attrib;

	core_a4_upt_db(adapter, rframe_sta, rattrib->src);

	fwd_sta = core_a4_get_fwd_sta(adapter, rattrib->dst);

	if (fwd_sta && rframe_sta && fwd_sta != rframe_sta) {
		if (prframe->u.hdr.attrib.amsdu) /* A4_AMSDU */
			return RTW_RX_MSDU_ACT_FORWARD;
		else if (_SUCCESS == core_a4_fwd_pkt_to_sta(prframe, fwd_sta))
			return RTW_RX_MSDU_ACT_FORWARD;
		else
			return RTW_RX_MSDU_ACT_NONE;
	} else {
		return RTW_RX_MSDU_ACT_INDICATE;
	}
}

bool core_a4_check_tx(_adapter *adapter, struct sk_buff **pskb)
{
	struct sk_buff *pkt = *pskb;
	struct pkt_file pktfile;
	struct ethhdr etherhdr;

	_rtw_open_pktfile(pkt, &pktfile);
	_rtw_pktfile_read(&pktfile, (u8 *)&etherhdr, ETH_HLEN);

#ifdef DEBUG_A4_TXFORCE
		return _TRUE;
#endif

	if (core_a4_get_fwd_sta(adapter, etherhdr.h_dest))
		return _TRUE;
	else
		return _FALSE;
}

/* A4 group adddressed proxy TX record */
struct core_a4_gptr {
	u8 src[ETH_ALEN];
	systime last_update;
	rtw_rhash_head rhash;
	_adapter *adapter;
	rtw_rcu_head rcu;
};

#define CORE_A4_GPTR_EXPIRE (2 * HZ)

/* Maximum number of gptrs per interface */
#define CORE_A4_MAX_GPTRS		1024

/* Max number of paths */
#define CORE_A4_MAX_PATHS 1024

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ECOS)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
static void core_a4_wgptr_free_rcu(struct core_a4_gptr *wgptr)
{
	kfree_rcu(wgptr, rcu);
	rtw_mstat_update(MSTAT_TYPE_PHY, MSTAT_FREE, sizeof(struct core_a4_gptr));
}
#else
static void core_a4_wgptr_free_rcu_callback(rtw_rcu_head *head)
{
	struct core_a4_gptr *wgptr;

	wgptr = container_of(head, struct core_a4_gptr, rcu);
	rtw_mfree(wgptr, sizeof(struct core_a4_gptr));
}

static void core_a4_wgptr_free_rcu(struct core_a4_gptr *wgptr)
{
	call_rcu(&wgptr->rcu, rtw_wgptr_free_rcu_callback);
}
#endif
#endif /* PLATFORM_LINUX */

static void core_a4_gptr_free_rcu(struct core_a4_gptr_table *tbl, struct core_a4_gptr *wgptr)
{
	_adapter *adapter = wgptr->adapter;

	ATOMIC_DEC(&adapter->a4_gpt_record_num);

	core_a4_wgptr_free_rcu(wgptr);
}

static u32 core_a4_gptr_table_hash(const void *addr, u32 len, u32 seed)
{
	/* Use last four bytes of hw addr as hash index */
#ifdef PLATFORM_ECOS
	return jhash_1word(get_unaligned((u32 *)(addr+2)), seed);
#else
	return jhash_1word(*(u32 *)(addr+2), seed);
#endif
}

static const rtw_rhashtable_params core_a4_gptr_rht_params = {
	.nelem_hint = 2,
	.automatic_shrinking = true,
	.key_len = ETH_ALEN,
	.key_offset = offsetof(struct core_a4_gptr, src),
	.head_offset = offsetof(struct core_a4_gptr, rhash),
	.hashfn = core_a4_gptr_table_hash,
};

static void core_a4_gptr_rht_free(void *ptr, void *tblptr)
{
	struct core_a4_gptr *wgptr = ptr;
	struct core_a4_gptr_table *tbl = tblptr;

	core_a4_gptr_free_rcu(tbl, wgptr);
}

static struct core_a4_gptr_table *core_a4_gptr_table_alloc(void)
{
	struct core_a4_gptr_table *newtbl;

	newtbl = rtw_malloc(sizeof(struct core_a4_gptr_table));
	if (!newtbl)
		return NULL;

	return newtbl;
}

static void core_a4_gptr_table_free(struct core_a4_gptr_table *tbl)
{
	rtw_rhashtable_free_and_destroy(&tbl->rhead,
				    core_a4_gptr_rht_free, tbl);
	rtw_mfree(tbl, sizeof(struct core_a4_gptr_table));
}

static struct core_a4_gptr *core_a4_gptr_lookup(_adapter *adapter, const u8 *src)
{
	struct core_a4_gptr_table *tbl = adapter->a4_gpt_records;

	if (!tbl)
		return NULL;

	return rtw_rhashtable_lookup_fast(&tbl->rhead, src, core_a4_gptr_rht_params);
}

/**
 * Locking: must be called within a read rcu section.
 */
static struct core_a4_gptr *core_a4_gptr_lookup_by_idx(_adapter *adapter, int idx)
{
	int i = 0, ret;
	struct core_a4_gptr_table *tbl = adapter->a4_gpt_records;
	struct core_a4_gptr *wgptr = NULL;
	rtw_rhashtable_iter iter;

	if (!tbl)
		return NULL;

	ret = rtw_rhashtable_walk_enter(&tbl->rhead, &iter);
	if (ret)
		return NULL;

	ret = rtw_rhashtable_walk_start(&iter);
	if (ret && ret != -EAGAIN)
		goto err;

	while ((wgptr = rtw_rhashtable_walk_next(&iter))) {
		if (IS_ERR(wgptr) && PTR_ERR(wgptr) == -EAGAIN)
			continue;
		if (IS_ERR(wgptr))
			break;
		if (i++ == idx)
			break;
	}
err:
	rtw_rhashtable_walk_stop(&iter);
	rtw_rhashtable_walk_exit(&iter);

	if (IS_ERR(wgptr) || !wgptr)
		return NULL;

	return wgptr;
}

void dump_wgptr(void *sel, _adapter *adapter)
{
	struct core_a4_gptr *wgptr;
	int idx = 0;
	char src[ETH_ALEN];
	u32 age_ms;

	RTW_PRINT_SEL(sel, "num:%d\n", ATOMIC_READ(&adapter->a4_gpt_record_num));
	RTW_PRINT_SEL(sel, "%-17s %-6s\n"
		, "src", "age"
	);

	do {
		rtw_rcu_read_lock();

		wgptr = core_a4_gptr_lookup_by_idx(adapter, idx);
		if (wgptr) {
			_rtw_memcpy(src, wgptr->src, ETH_ALEN);
			age_ms = rtw_get_passing_time_ms(wgptr->last_update);
		}

		rtw_rcu_read_unlock();

		if (wgptr) {
			RTW_PRINT_SEL(sel, MAC_FMT" %6u\n"
				, MAC_ARG(src)
				, age_ms < 999999 ? age_ms : 999999
			);
		}

		idx++;
	} while (wgptr);
}

static struct core_a4_gptr *core_a4_gptr_new(_adapter *adapter, const u8 *src)
{
	struct core_a4_gptr *new_wgptr;

	new_wgptr = rtw_zmalloc(sizeof(struct core_a4_gptr));
	if (!new_wgptr)
		return NULL;

	new_wgptr->adapter = adapter;
	_rtw_memcpy(new_wgptr->src, src, ETH_ALEN);
	new_wgptr->last_update = rtw_get_current_time();

	return new_wgptr;
}

static struct core_a4_gptr *core_a4_gptr_add(_adapter *adapter, const u8 *src)
{
	struct core_a4_gptr_table *tbl = adapter->a4_gpt_records;
	struct core_a4_gptr *wgptr, *new_wgptr;
	int ret;

	if (!tbl)
		return ERR_PTR(-ENOTSUPP);

	if (ATOMIC_INC_UNLESS(&adapter->a4_gpt_record_num, CORE_A4_MAX_PATHS) == 0)
		return ERR_PTR(-ENOSPC);

	new_wgptr = core_a4_gptr_new(adapter, src);
	if (!new_wgptr)
		return ERR_PTR(-ENOMEM);

	do {
		ret = rtw_rhashtable_lookup_insert_fast(&tbl->rhead,
						    &new_wgptr->rhash,
						    core_a4_gptr_rht_params);

		if (ret == -EEXIST)
			wgptr = rtw_rhashtable_lookup_fast(&tbl->rhead,
						       src,
						       core_a4_gptr_rht_params);

	} while (unlikely(ret == -EEXIST && !wgptr));

	if (ret && ret != -EEXIST)
		return ERR_PTR(ret);

	/* At this point either new_wgptr was added, or we found a
	 * matching entry already in the table; in the latter case
	 * free the unnecessary new entry.
	 */
	if (ret == -EEXIST) {
		rtw_mfree(new_wgptr, sizeof(struct core_a4_gptr));
		new_wgptr = wgptr;
	}

	return new_wgptr;
}

bool core_rx_a4_gptr_check(_adapter *adapter, const u8 *src)
{
	struct core_a4_gptr *wgptr;
	bool ret = 0;

	rtw_rcu_read_lock();

	wgptr = core_a4_gptr_lookup(adapter, src);
	if (wgptr)
		ret = rtw_time_after(wgptr->last_update + CORE_A4_GPTR_EXPIRE, rtw_get_current_time());

	rtw_rcu_read_unlock();

	return ret;
}

void core_tx_a4_gptr_update(_adapter *adapter, const u8 *src)
{
	struct core_a4_gptr *wgptr;

	rtw_rcu_read_lock();
	wgptr = core_a4_gptr_lookup(adapter, src);
	if (!wgptr)
		core_a4_gptr_add(adapter, src);
	else
		wgptr->last_update = rtw_get_current_time();
	rtw_rcu_read_unlock();
}

static void __core_a4_gptr_del(struct core_a4_gptr_table *tbl, struct core_a4_gptr *wgptr)
{
	rtw_rhashtable_remove_fast(&tbl->rhead, &wgptr->rhash, core_a4_gptr_rht_params);
	core_a4_gptr_free_rcu(tbl, wgptr);
}

void core_a4_gptr_expire(_adapter *adapter)
{
	struct core_a4_gptr_table *tbl = adapter->a4_gpt_records;
	struct core_a4_gptr *wgptr;
	rtw_rhashtable_iter iter;
	int ret;

	if (!tbl)
		return;

	ret = rtw_rhashtable_walk_enter(&tbl->rhead, &iter);
	if (ret)
		return;

	ret = rtw_rhashtable_walk_start(&iter);
	if (ret && ret != -EAGAIN)
		goto out;

	while ((wgptr = rtw_rhashtable_walk_next(&iter))) {
		if (IS_ERR(wgptr) && PTR_ERR(wgptr) == -EAGAIN)
			continue;
		if (IS_ERR(wgptr))
			break;
		if (rtw_time_after(rtw_get_current_time(), wgptr->last_update + CORE_A4_GPTR_EXPIRE))
			__core_a4_gptr_del(tbl, wgptr);
	}

out:
	rtw_rhashtable_walk_stop(&iter);
	rtw_rhashtable_walk_exit(&iter);
}

void core_a4_gptr_flush(_adapter *adapter)
{
	struct core_a4_gptr_table *tbl = adapter->a4_gpt_records;
	struct core_a4_gptr *wgptr;
	rtw_rhashtable_iter iter;
	int ret;

	if (!tbl)
		return;

	ret = rtw_rhashtable_walk_enter(&tbl->rhead, &iter);
	if (ret)
		return;

	ret = rtw_rhashtable_walk_start(&iter);
	if (ret && ret != -EAGAIN)
		goto out;

	while ((wgptr = rtw_rhashtable_walk_next(&iter))) {
		if (IS_ERR(wgptr) && PTR_ERR(wgptr) == -EAGAIN)
			continue;
		if (IS_ERR(wgptr))
			break;
		__core_a4_gptr_del(tbl, wgptr);
	}

out:
	rtw_rhashtable_walk_stop(&iter);
	rtw_rhashtable_walk_exit(&iter);
}


int core_a4_gptr_tbl_init(_adapter *adapter)
{
	struct core_a4_gptr_table *tbl;
	int ret;

	tbl = core_a4_gptr_table_alloc();
	if (!tbl)
		return -ENOMEM;

	rtw_rhashtable_init(&tbl->rhead, &core_a4_gptr_rht_params);

	ATOMIC_SET(&adapter->a4_gpt_record_num, 0);
	adapter->a4_gpt_records = tbl;

	return 0;
}

void core_a4_gptr_tbl_unregister(_adapter *adapter)
{
	if (adapter->a4_gpt_records) {
		core_a4_gptr_table_free(adapter->a4_gpt_records);
		adapter->a4_gpt_records = NULL;
	}
}

void core_a4_gptr_invalid_by_mac(_adapter *adapter, const u8 *src)
{
	struct core_a4_gptr *wgptr;

	rtw_rcu_read_lock();
	wgptr = core_a4_gptr_lookup(adapter, src);
	if(wgptr)
		wgptr->last_update = 0;
	rtw_rcu_read_unlock();

	return;
}

#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
struct rtw_a4_db_entry *core_a4_get_source_db_entry(_adapter *adapter, u8 *mac, int *del)
#else
struct rtw_a4_db_entry *core_a4_get_source_db_entry(_adapter *adapter, u8 *mac)
#endif
{
	struct sta_info *psta = NULL;
	struct rtw_a4_db_entry *db;

	/* ASSERT(mac); */

	db = adapter->machash[mac_hash(mac)];

	while (db != NULL) {
		if (!memcmp(db->mac, mac, ETH_ALEN)) {
#if 0
	printk("%s A4 STA fwd emac:%pM, wmac:%pM\n",
		adapter->pnetdev->name, db->mac, db->psta->phl_sta->mac_addr);
#endif
			psta = db->psta;

			if (!psta || !psta->phl_sta) {
				DBGP("error psta || psta->phl_sta \n");
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
				rtw_flush_brsc_info(NULL, db->mac);
#endif
				mac_hash_unlink(db);
				free_a4_entry(adapter, db);
				psta = NULL;
				db = NULL;
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
				*del = 1;
#endif
			}

			break;
		}
		db = db->next_hash;
	}

	return db;
}

/* A4_CNT */
void core_a4_sta_expire(_adapter *adapter)
{
	struct rtw_a4_db_entry *db = NULL, *db_next = NULL;
	u32 i;
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	unsigned char mac[MAX_A4_TBL_NUM*ETH_ALEN + 1] = {0};
	unsigned int mac_cnt = 0;
#endif

	for (i = 0 ; i < A4_STA_HASH_SIZE; i++) {
		machash_spinlock_bh(i);
		db = adapter->machash[i];

		while (db != NULL) {
			db_next = db->next_hash;
			db->link_time += 2;

			if (rtw_time_after(rtw_get_current_time(), db->ageing_timer + rtw_ms_to_systime(A4_STA_AGEING_TIME * 1000))) {
				RTW_PRINT("[%s %d][%s] A4 db timeout, emac:%pM, wmac:%pM\n", __FUNCTION__, __LINE__, adapter->pnetdev->name,
					db->mac, db->psta->phl_sta->mac_addr);
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
				memcpy(&mac[mac_cnt*ETH_ALEN], db->mac, ETH_ALEN);
				mac_cnt++;
#endif
				mac_hash_unlink(db);
				free_a4_entry(adapter, db);
			}

			db = db_next;
		}
		machash_spinunlock_bh(i);
	}
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	if(mac_cnt)
		rtw_del_fc_external_lut_process(adapter, mac, mac_cnt);
#endif
}

void core_a4_count_stats(_adapter *adapter, u8* mac, u8 mode, u32 size)
{
	struct rtw_a4_db_entry *db = NULL;
	u32 hash = mac_hash(mac);
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	int need_del_db = 0;
#endif

	machash_spinlock_bh(hash);
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	db = core_a4_get_source_db_entry(adapter, mac, &need_del_db);
#else
	db = core_a4_get_source_db_entry(adapter, mac);
#endif

	if (db) {
		if (mode == 0){
			/* tx */
			db->tx_bytes += size;
			db->tx_count++;
		} else if (mode == 1){
			/* rx */
			db->rx_bytes += size;
			db->rx_count++;
		} else
			RTW_PRINT("[%s][%d] error mode:%d\n", __func__, __LINE__, mode);
	}
	machash_spinunlock_bh(hash);
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	if(need_del_db)
		rtw_del_fc_external_lut_process(adapter, mac, 1);
#endif
}

void core_a4_init(_adapter *padapter)
{
	u32 i = 0;
	//padapter->a4_enable = 1;
	//if (padapter->a4_enable) {
		INIT_LIST_HEAD(&(padapter->a4_sta_list));

#ifdef CONFIG_DYN_ALLOC_A4_TBL
	for (i = 0; i < A4_STA_HASH_SIZE; i++) {
		_rtw_spinlock_init(&(padapter->machash_lock[i]));
	}
#endif

		if (MLME_IS_STA(padapter))
			core_a4_gptr_tbl_init(padapter);
	//}

	RTW_INFO("[TXSC_INIT][%s] a4_enable:%d, a4_sta_list/a4_gptr_tbl init.\n",
		padapter->pnetdev->name, padapter->a4_enable);
}

#endif
