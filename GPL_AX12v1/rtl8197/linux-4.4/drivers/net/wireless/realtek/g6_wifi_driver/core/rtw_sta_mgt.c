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
#define _RTW_STA_MGT_C_

#include <drv_types.h>
#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
#include <net/rtl/rtl_brsc.h>
#endif

bool test_st_match_rule(_adapter *adapter, u8 *local_naddr, u8 *local_port, u8 *remote_naddr, u8 *remote_port)
{
	if (ntohs(*((u16 *)local_port)) == 5001 || ntohs(*((u16 *)remote_port)) == 5001)
		return _TRUE;
	return _FALSE;
}

struct st_register test_st_reg = {
	.s_proto = 0x06,
	.rule = test_st_match_rule,
};

inline void rtw_st_ctl_init(struct st_ctl_t *st_ctl)
{
	_rtw_memset(st_ctl->reg, 0 , sizeof(struct st_register) * SESSION_TRACKER_REG_ID_NUM);
	_rtw_init_queue(&st_ctl->tracker_q);
}

inline void rtw_st_ctl_clear_tracker_q(struct st_ctl_t *st_ctl)
{
	_list *plist, *phead;
	struct session_tracker *st;

	_rtw_spinlock_bh(&st_ctl->tracker_q.lock);
	phead = &st_ctl->tracker_q.queue;
	plist = get_next(phead);
	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		st = LIST_CONTAINOR(plist, struct session_tracker, list);
		plist = get_next(plist);
		rtw_list_delete(&st->list);
		rtw_mfree((u8 *)st, sizeof(struct session_tracker));
	}
	_rtw_spinunlock_bh(&st_ctl->tracker_q.lock);
}

inline void rtw_st_ctl_deinit(struct st_ctl_t *st_ctl)
{
	rtw_st_ctl_clear_tracker_q(st_ctl);
	_rtw_deinit_queue(&st_ctl->tracker_q);
}

inline void rtw_st_ctl_register(struct st_ctl_t *st_ctl, u8 st_reg_id, struct st_register *reg)
{
	if (st_reg_id >= SESSION_TRACKER_REG_ID_NUM) {
		rtw_warn_on(1);
		return;
	}

	st_ctl->reg[st_reg_id].s_proto = reg->s_proto;
	st_ctl->reg[st_reg_id].rule = reg->rule;
}

inline void rtw_st_ctl_unregister(struct st_ctl_t *st_ctl, u8 st_reg_id)
{
	int i;

	if (st_reg_id >= SESSION_TRACKER_REG_ID_NUM) {
		rtw_warn_on(1);
		return;
	}

	st_ctl->reg[st_reg_id].s_proto = 0;
	st_ctl->reg[st_reg_id].rule = NULL;

	/* clear tracker queue if no session trecker registered */
	for (i = 0; i < SESSION_TRACKER_REG_ID_NUM; i++)
		if (st_ctl->reg[i].s_proto != 0)
			break;
	if (i >= SESSION_TRACKER_REG_ID_NUM)
		rtw_st_ctl_clear_tracker_q(st_ctl);
}

inline bool rtw_st_ctl_chk_reg_s_proto(struct st_ctl_t *st_ctl, u8 s_proto)
{
	bool ret = _FALSE;
	int i;

	for (i = 0; i < SESSION_TRACKER_REG_ID_NUM; i++) {
		if (st_ctl->reg[i].s_proto == s_proto) {
			ret = _TRUE;
			break;
		}
	}

	return ret;
}

inline bool rtw_st_ctl_chk_reg_rule(struct st_ctl_t *st_ctl, _adapter *adapter, u8 *local_naddr, u8 *local_port, u8 *remote_naddr, u8 *remote_port)
{
	bool ret = _FALSE;
	int i;
	st_match_rule rule;

	for (i = 0; i < SESSION_TRACKER_REG_ID_NUM; i++) {
		rule = st_ctl->reg[i].rule;
		if (rule && rule(adapter, local_naddr, local_port, remote_naddr, remote_port) == _TRUE) {
			ret = _TRUE;
			break;
		}
	}

	return ret;
}

void rtw_st_ctl_rx(struct sta_info *sta, u8 *ehdr_pos)
{
	_adapter *adapter = sta->padapter;
	struct ethhdr *etherhdr = (struct ethhdr *)ehdr_pos;

	if (ntohs(etherhdr->h_proto) == ETH_P_IP) {
		u8 *ip = ehdr_pos + ETH_HLEN;

		if (GET_IPV4_PROTOCOL(ip) == 0x06  /* TCP */
			&& rtw_st_ctl_chk_reg_s_proto(&sta->st_ctl, 0x06) == _TRUE
		) {
			u8 *tcp = ip + GET_IPV4_IHL(ip) * 4;

			if (rtw_st_ctl_chk_reg_rule(&sta->st_ctl, adapter, IPV4_DST(ip), TCP_DST(tcp), IPV4_SRC(ip), TCP_SRC(tcp)) == _TRUE) {
				if (GET_TCP_SYN(tcp) && GET_TCP_ACK(tcp)) {
					session_tracker_add_cmd(adapter, sta
						, IPV4_DST(ip), TCP_DST(tcp)
						, IPV4_SRC(ip), TCP_SRC(tcp));
					if (DBG_SESSION_TRACKER)
						RTW_INFO(FUNC_ADPT_FMT" local:"IP_FMT":"PORT_FMT", remote:"IP_FMT":"PORT_FMT" SYN-ACK\n"
							, FUNC_ADPT_ARG(adapter)
							, IP_ARG(IPV4_DST(ip)), PORT_ARG(TCP_DST(tcp))
							, IP_ARG(IPV4_SRC(ip)), PORT_ARG(TCP_SRC(tcp)));
				}
				if (GET_TCP_FIN(tcp)) {
					session_tracker_del_cmd(adapter, sta
						, IPV4_DST(ip), TCP_DST(tcp)
						, IPV4_SRC(ip), TCP_SRC(tcp));
					if (DBG_SESSION_TRACKER)
						RTW_INFO(FUNC_ADPT_FMT" local:"IP_FMT":"PORT_FMT", remote:"IP_FMT":"PORT_FMT" FIN\n"
							, FUNC_ADPT_ARG(adapter)
							, IP_ARG(IPV4_DST(ip)), PORT_ARG(TCP_DST(tcp))
							, IP_ARG(IPV4_SRC(ip)), PORT_ARG(TCP_SRC(tcp)));
				}
			}

		}
	}
}

#define SESSION_TRACKER_FMT IP_FMT":"PORT_FMT" "IP_FMT":"PORT_FMT" %u %d"
#define SESSION_TRACKER_ARG(st) IP_ARG(&(st)->local_naddr), PORT_ARG(&(st)->local_port), IP_ARG(&(st)->remote_naddr), PORT_ARG(&(st)->remote_port), (st)->status, rtw_get_passing_time_ms((st)->set_time)

void dump_st_ctl(void *sel, struct st_ctl_t *st_ctl)
{
	int i;
	_list *plist, *phead;
	struct session_tracker *st;

	if (!DBG_SESSION_TRACKER)
		return;

	for (i = 0; i < SESSION_TRACKER_REG_ID_NUM; i++)
		RTW_PRINT_SEL(sel, "reg%d: %u %p\n", i, st_ctl->reg[i].s_proto, st_ctl->reg[i].rule);

	_rtw_spinlock_bh(&st_ctl->tracker_q.lock);
	phead = &st_ctl->tracker_q.queue;
	plist = get_next(phead);
	while (rtw_end_of_queue_search(phead, plist) == _FALSE) {
		st = LIST_CONTAINOR(plist, struct session_tracker, list);
		plist = get_next(plist);

		RTW_PRINT_SEL(sel, SESSION_TRACKER_FMT"\n", SESSION_TRACKER_ARG(st));
	}
	_rtw_spinunlock_bh(&st_ctl->tracker_q.lock);

}

//void _rtw_init_stainfo(struct sta_info *psta);
void _rtw_init_stainfo(struct sta_info *psta, unsigned char is_first)
{
	unsigned long offset = 0;

	if (is_first) {
		_rtw_memset((u8 *)psta, 0, sizeof(struct sta_info));
	} else {
#ifdef CONFIG_IOCTL_CFG80211
            _rtw_spinlock_bh(&psta->lock);
            if ((psta->auth_len != 0) && (psta->pauth_frame != NULL)) {
                    rtw_mfree(psta->pauth_frame, psta->auth_len);
                    psta->pauth_frame = NULL;
                    psta->auth_len = 0;
            }
            if ((psta->assoc_req_len != 0) && (psta->passoc_req != NULL)) {
                    rtw_mfree(psta->passoc_req, psta->assoc_req_len);
                    psta->passoc_req = NULL;
                    psta->assoc_req_len = 0;
            }
            if ((psta->commit_len != 0) && (psta->pcommit != NULL)) {
                    rtw_mfree(psta->pcommit, psta->commit_len);
                    psta->pcommit = NULL;
                    psta->commit_len = 0;
            }
            _rtw_spinunlock_bh(&psta->lock);
#endif /* CONFIG_IOCTL_CFG80211 */

		offset = (unsigned long)(&((struct sta_info *)0)->sta_info_clear_start);
		_rtw_memset((void *)((unsigned long)psta + offset), 0, sizeof(struct sta_info) - offset);
	}

	_rtw_spinlock_init(&psta->lock);
	_rtw_init_listhead(&psta->free_list);
	_rtw_init_listhead(&psta->hash_list);
	/* _rtw_init_listhead(&psta->asoc_list); */
	/* _rtw_init_listhead(&psta->sleep_list); */
	/* _rtw_init_listhead(&psta->wakeup_list);	 */

	_rtw_init_listhead(&psta->bmc_list);
	_rtw_init_queue(&psta->sleep_q);

	_rtw_init_sta_xmit_priv(&psta->sta_xmitpriv);
	_rtw_init_sta_recv_priv(&psta->sta_recvpriv);

#ifdef CONFIG_AP_MODE
	_rtw_init_listhead(&psta->asoc_list);
	_rtw_init_listhead(&psta->auth_list);
	psta->bpairwise_key_installed = _FALSE;

#ifdef CONFIG_RTW_80211R
	psta->ft_pairwise_key_installed = _FALSE;
	psta->ft_support = _FALSE;
#endif

#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
	_rtw_memset(&psta->wapiStaInfo, 0, sizeof(RTL_WAPI_STA_INFO));
#endif
#endif /* CONFIG_AP_MODE	 */

	rtw_st_ctl_init(&psta->st_ctl);
}

u32	_rtw_init_sta_priv(struct	sta_priv *pstapriv)
{
	_adapter *adapter = container_of(pstapriv, _adapter, stapriv);
	/* struct macid_ctl_t *macid_ctl = adapter_to_macidctl(adapter); */
	struct sta_info *psta;
	s32 i;
	u32 ret = _FAIL;

	pstapriv->padapter = adapter;

	/* ToDo: Make stainfo shared for all VAP. Only primary adapter allocate
	 *       stainfo.
	 */
	if (is_primary_adapter(adapter)) {

		pstapriv->pallocated_stainfo_buf = rtw_zvmalloc(
			sizeof(struct sta_info) * NUM_STA + MEM_ALIGNMENT_OFFSET);
		if (!pstapriv->pallocated_stainfo_buf)
			goto exit;

		pstapriv->pstainfo_buf = pstapriv->pallocated_stainfo_buf;
		if ((SIZE_PTR)pstapriv->pstainfo_buf & MEM_ALIGNMENT_PADDING)
			pstapriv->pstainfo_buf += MEM_ALIGNMENT_OFFSET -
				((SIZE_PTR)pstapriv->pstainfo_buf & MEM_ALIGNMENT_PADDING);

		pstapriv->pfree_sta_queue = rtw_zvmalloc(sizeof(_queue));
		_rtw_init_queue(pstapriv->pfree_sta_queue);

		psta = (struct sta_info *)(pstapriv->pstainfo_buf);
		for (i = 0; i < NUM_STA; i++) {
			_rtw_init_stainfo(psta, 1);

			rtw_list_insert_tail(&psta->free_list, get_list_head(pstapriv->pfree_sta_queue));
			psta++;
		}

	} else { /* if (is_primary_adapter(adapter)) */
		_adapter *pri_adapter = GET_PRIMARY_ADAPTER(adapter);
		pstapriv->pfree_sta_queue = pri_adapter->stapriv.pfree_sta_queue;
	}

	_rtw_spinlock_init(&pstapriv->sta_hash_lock);
#ifdef RTW_WKARD_AP_CMD_DISPATCH
	_rtw_spinlock_init(&pstapriv->sta_info_lock);
#endif

	/* _rtw_init_queue(&pstapriv->asoc_q); */
	pstapriv->asoc_sta_count = 0;
	_rtw_init_queue(&pstapriv->sleep_q);
	_rtw_init_queue(&pstapriv->wakeup_q);

	for (i = 0; i < NUM_STA; i++)
		_rtw_init_listhead(&(pstapriv->sta_hash[i]));

	pstapriv->adhoc_expire_to = 4; /* 4 * 2 = 8 sec */

#ifdef CONFIG_AP_MODE
	pstapriv->max_aid = rtw_phl_get_macid_max_num(
				GET_HAL_INFO(adapter_to_dvobj(pstapriv->padapter)));
	pstapriv->rr_aid = 0;
	pstapriv->started_aid = 1;
	pstapriv->sta_aid = rtw_zmalloc(pstapriv->max_aid * sizeof(struct sta_info *));
	if (!pstapriv->sta_aid)
		goto exit;
	pstapriv->aid_bmp_len = AID_BMP_LEN(pstapriv->max_aid);
	pstapriv->sta_dz_bitmap = rtw_zmalloc(pstapriv->aid_bmp_len);
	if (!pstapriv->sta_dz_bitmap)
		goto exit;
	pstapriv->tim_bitmap = rtw_zmalloc(pstapriv->aid_bmp_len);
	if (!pstapriv->tim_bitmap)
		goto exit;

	_rtw_init_listhead(&pstapriv->asoc_list);
	_rtw_init_listhead(&pstapriv->auth_list);
	_rtw_spinlock_init(&pstapriv->asoc_list_lock);
	_rtw_spinlock_init(&pstapriv->auth_list_lock);
	pstapriv->asoc_list_cnt = 0;
	pstapriv->auth_list_cnt = 0;
#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	pstapriv->tbtx_asoc_list_cnt = 0;
#endif

	pstapriv->auth_to = 3; /* 3*2 = 6 sec */
	pstapriv->assoc_to = 3;
	/* pstapriv->expire_to = 900; */ /* 900*2 = 1800 sec = 30 min, expire after no any traffic. */
	/* pstapriv->expire_to = 30; */ /* 30*2 = 60 sec = 1 min, expire after no any traffic. */
#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
	pstapriv->expire_to = 3; /* 3*2 = 6 sec */
#else
	pstapriv->expire_to = 25; /* 25*2 = 50 sec, Checked two times before
				     hostapd's default 120 inactive time. */
	/* pstapriv->expire_to = 60; *//* 60*2 = 120 sec = 2 min, expire after no any traffic. */
#endif
#ifdef CONFIG_ATMEL_RC_PATCH
	_rtw_memset(pstapriv->atmel_rc_pattern, 0, ETH_ALEN);
#endif
	pstapriv->max_num_sta = NUM_STA;

#endif

#if CONFIG_RTW_MACADDR_ACL
	for (i = 0; i < RTW_ACL_PERIOD_NUM; i++)
		rtw_macaddr_acl_init(adapter, i);
#endif

#if CONFIG_RTW_PRE_LINK_STA
	rtw_pre_link_sta_ctl_init(pstapriv);
#endif

#if defined(DBG_ROAMING_TEST)
	rtw_set_rx_chk_limit(adapter, 1, 5);
#elif defined(CONFIG_ACTIVE_KEEP_ALIVE_CHECK) && !defined(CONFIG_LPS_LCLK_WD_TIMER)
	rtw_set_rx_chk_limit(adapter, 4, 5);
#else
	rtw_set_rx_chk_limit(adapter, 5, 5);
#endif

	ret = _SUCCESS;

exit:
	if (ret != _SUCCESS) {
		if (pstapriv->pallocated_stainfo_buf)
			rtw_vmfree(pstapriv->pallocated_stainfo_buf,
				sizeof(struct sta_info) * NUM_STA + MEM_ALIGNMENT_OFFSET);
		#ifdef CONFIG_AP_MODE
		if (pstapriv->sta_aid)
			rtw_mfree(pstapriv->sta_aid, pstapriv->max_aid * sizeof(struct sta_info *));
		if (pstapriv->sta_dz_bitmap)
			rtw_mfree(pstapriv->sta_dz_bitmap, pstapriv->aid_bmp_len);
		#endif
	}

	return ret;
}

inline int rtw_stainfo_offset(struct sta_priv *stapriv, struct sta_info *sta)
{
	int offset = (((u8 *)sta) - stapriv->pstainfo_buf) / sizeof(struct sta_info);

	if (!stainfo_offset_valid(offset))
		RTW_INFO("%s invalid offset(%d), out of range!!!", __func__, offset);

	return offset;
}

inline struct sta_info *rtw_get_stainfo_by_offset(struct sta_priv *stapriv, int offset)
{
	if (!stainfo_offset_valid(offset))
		RTW_INFO("%s invalid offset(%d), out of range!!!", __func__, offset);

	return (struct sta_info *)(stapriv->pstainfo_buf + offset * sizeof(struct sta_info));
}

void	_rtw_free_sta_xmit_priv_lock(struct sta_xmit_priv *psta_xmitpriv);
void	_rtw_free_sta_xmit_priv_lock(struct sta_xmit_priv *psta_xmitpriv)
{

	_rtw_spinlock_free(&psta_xmitpriv->lock);

	_rtw_spinlock_free(&(psta_xmitpriv->be_q.queue.lock));
	_rtw_spinlock_free(&(psta_xmitpriv->bk_q.queue.lock));
	_rtw_spinlock_free(&(psta_xmitpriv->vi_q.queue.lock));
	_rtw_spinlock_free(&(psta_xmitpriv->vo_q.queue.lock));
}

static void	_rtw_free_sta_recv_priv_lock(struct sta_recv_priv *psta_recvpriv)
{

	_rtw_spinlock_free(&psta_recvpriv->lock);

	_rtw_spinlock_free(&(psta_recvpriv->defrag_q.lock));


}

void rtw_mfree_stainfo(struct sta_info *psta);
void rtw_mfree_stainfo(struct sta_info *psta)
{

	if (&psta->lock != NULL)
		_rtw_spinlock_free(&psta->lock);

	_rtw_free_sta_xmit_priv_lock(&psta->sta_xmitpriv);
	_rtw_free_sta_recv_priv_lock(&psta->sta_recvpriv);

}


/* this function is used to free the memory of lock || sema for all stainfos */
void rtw_mfree_all_stainfo(struct sta_priv *pstapriv);
void rtw_mfree_all_stainfo(struct sta_priv *pstapriv)
{
	_list	*plist, *phead;
	struct sta_info *psta = NULL;

	if (pstapriv->pfree_sta_queue == NULL)
		return;

	_rtw_spinlock_bh(&(pstapriv->pfree_sta_queue->lock));

	phead = get_list_head(pstapriv->pfree_sta_queue);
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info , free_list);
		plist = get_next(plist);

		if (psta->padapter == pstapriv->padapter)
		rtw_mfree_stainfo(psta);
	}

	_rtw_spinunlock_bh(&(pstapriv->pfree_sta_queue->lock));
}

void rtw_mfree_sta_priv_lock(struct	sta_priv *pstapriv);
void rtw_mfree_sta_priv_lock(struct	sta_priv *pstapriv)
{
	rtw_mfree_all_stainfo(pstapriv); /* be done before free sta_hash_lock */

	if (is_primary_adapter(pstapriv->padapter) && pstapriv->pfree_sta_queue)
		_rtw_spinlock_free(&pstapriv->pfree_sta_queue->lock);

	_rtw_spinlock_free(&pstapriv->sta_hash_lock);
#ifdef RTW_WKARD_AP_CMD_DISPATCH
	_rtw_spinlock_free(&pstapriv->sta_info_lock);
#endif
	_rtw_spinlock_free(&pstapriv->wakeup_q.lock);
	_rtw_spinlock_free(&pstapriv->sleep_q.lock);

#ifdef CONFIG_AP_MODE
	_rtw_spinlock_free(&pstapriv->asoc_list_lock);
	_rtw_spinlock_free(&pstapriv->auth_list_lock);
#endif

}

u32	_rtw_free_sta_priv(struct	sta_priv *pstapriv)
{
	_list	*phead, *plist;
	struct sta_info *psta = NULL;
	struct recv_reorder_ctrl *preorder_ctrl;
	int	index;

	if (pstapriv) {

		/*	delete all reordering_ctrl_timer		*/
		_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
		for (index = 0; index < NUM_STA; index++) {
			phead = &(pstapriv->sta_hash[index]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				int i;
				psta = LIST_CONTAINOR(plist, struct sta_info , hash_list);
				plist = get_next(plist);

				for (i = 0; i < 16 ; i++) {
					preorder_ctrl = &psta->recvreorder_ctrl[i];
					_cancel_timer_ex(&preorder_ctrl->reordering_ctrl_timer);
				}
			}
		}
		_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);
		/*===============================*/

		rtw_mfree_sta_priv_lock(pstapriv);

#if CONFIG_RTW_MACADDR_ACL
		for (index = 0; index < RTW_ACL_PERIOD_NUM; index++)
			rtw_macaddr_acl_deinit(pstapriv->padapter, index);
#endif

#if CONFIG_RTW_PRE_LINK_STA
		rtw_pre_link_sta_ctl_deinit(pstapriv);
#endif

		if (is_primary_adapter(pstapriv->padapter)) {
			if (pstapriv->pallocated_stainfo_buf)
				rtw_vmfree(pstapriv->pallocated_stainfo_buf,
					sizeof(struct sta_info) * NUM_STA + MEM_ALIGNMENT_OFFSET);
			if (pstapriv->pfree_sta_queue)
				rtw_vmfree(pstapriv->pfree_sta_queue, sizeof(_queue));
		}

		#ifdef CONFIG_AP_MODE
		if (pstapriv->sta_aid)
			rtw_mfree(pstapriv->sta_aid, pstapriv->max_aid * sizeof(struct sta_info *));
		if (pstapriv->sta_dz_bitmap)
			rtw_mfree(pstapriv->sta_dz_bitmap, pstapriv->aid_bmp_len);
		if (pstapriv->tim_bitmap)
			rtw_mfree(pstapriv->tim_bitmap, pstapriv->aid_bmp_len);
		#endif
	}

	return _SUCCESS;
}


static void rtw_init_recv_timer(struct recv_reorder_ctrl *preorder_ctrl)
{
	_adapter *padapter = preorder_ctrl->padapter;

#if defined(CONFIG_80211N_HT) && defined(CONFIG_RECV_REORDERING_CTRL)
	rtw_init_timer(&(preorder_ctrl->reordering_ctrl_timer), rtw_reordering_ctrl_timeout_handler, preorder_ctrl);
#endif
}

static u32 rtw_insert_core_stainfo_hash(struct	sta_priv *pstapriv, struct sta_info *psta, const u8 *hwaddr)
{
	s32 index;
	_list *phash_list;
	struct rtw_wifi_role_t *phl_role = pstapriv->padapter->phl_role;

	_rtw_spinlock_bh(&(pstapriv->sta_hash_lock));

	/*
	for sta mode, due to self sta info & AP sta info are the same sta info
	using self hash index for sta mode, or bmc sta will not found self sta
	*/
	switch (phl_role->type)
	{
	case PHL_RTYPE_NONE:
	case PHL_RTYPE_STATION:
	case PHL_RTYPE_ADHOC:
	case PHL_RTYPE_P2P_DEVICE:
	case PHL_RTYPE_P2P_GC:
		index = wifi_mac_hash(phl_role->mac_addr);
		break;
	default:
		index = wifi_mac_hash(hwaddr);
		break;
	}

	if (index >= NUM_STA) {
		_rtw_spinunlock_bh(&(pstapriv->sta_hash_lock));
		return _FAIL;
	}

	phash_list = &(pstapriv->sta_hash[index]);

	rtw_list_insert_tail(&psta->hash_list, phash_list);
	pstapriv->asoc_sta_count++;

	_rtw_spinunlock_bh(&(pstapriv->sta_hash_lock));

	return _SUCCESS;
}

static u32 rtw_insert_core_stainfo_freeq(struct sta_priv *pstapriv, struct sta_info *psta)
{
	if (psta == NULL)
		return _FAIL;

	_rtw_spinlock_bh(&(pstapriv->pfree_sta_queue->lock));

	if (rtw_is_list_empty(&psta->free_list) == _FALSE) {
		RTW_ERR("[%s %d] free_list is not empty, there are something wrong in sta process\n", __FUNCTION__, __LINE__);
		_rtw_spinunlock_bh(&(pstapriv->pfree_sta_queue->lock));
		return _FAIL;
	}

	rtw_list_insert_tail(&psta->free_list, get_list_head(pstapriv->pfree_sta_queue));

	_rtw_spinunlock_bh(&(pstapriv->pfree_sta_queue->lock));

	return _SUCCESS;
}

static struct sta_info *_rtw_alloc_core_stainfo(struct sta_priv *pstapriv, const u8 *hwaddr)
{
	struct sta_info *psta;
	_queue *pfree_sta_queue;
	struct recv_reorder_ctrl *preorder_ctrl;
	struct rtw_wifi_role_t *phl_role = pstapriv->padapter->phl_role;
	int i = 0;
	u16  wRxSeqInitialValue = 0xffff;

	pfree_sta_queue = pstapriv->pfree_sta_queue;

	_rtw_spinlock_bh(&(pstapriv->sta_hash_lock));

	_rtw_spinlock_bh(&(pfree_sta_queue->lock));
	if (_rtw_queue_empty(pfree_sta_queue) == _TRUE) {
		psta = NULL;
	} else {
		psta = LIST_CONTAINOR(get_next(&pfree_sta_queue->queue), struct sta_info, free_list);
		rtw_list_delete(&(psta->free_list));
	}
	_rtw_spinunlock_bh(&(pfree_sta_queue->lock));

	if (psta) {
		_rtw_init_stainfo(psta, 0);

		psta->padapter = pstapriv->padapter;

		/* Commented by Albert 2009/08/13
		 * For the SMC router, the sequence number of first packet of WPS handshake will be 0.
		 * In this case, this packet will be dropped by recv_decache function if we use the 0x00 as the default value for tid_rxseq variable.
		 * So, we initialize the tid_rxseq variable as the 0xffff. */

		for (i = 0; i < 16; i++) {
			_rtw_memcpy(&psta->sta_recvpriv.rxcache.tid_rxseq[i], &wRxSeqInitialValue, 2);
			_rtw_memcpy(&psta->sta_recvpriv.bmc_tid_rxseq[i], &wRxSeqInitialValue, 2);
			_rtw_memset(&psta->sta_recvpriv.rxcache.iv[i], 0, sizeof(psta->sta_recvpriv.rxcache.iv[i]));
		}

		rtw_init_timer(&psta->addba_retry_timer, addba_timer_hdl, psta);
#if defined(CONFIG_IEEE80211W) && !defined(CONFIG_IOCTL_CFG80211)
		rtw_init_timer(&psta->dot11w_expire_timer, sa_query_timer_hdl, psta);
#endif /* CONFIG_IEEE80211W */
#ifdef CONFIG_TDLS
		rtw_init_tdls_timer(pstapriv->padapter, psta);
#endif /* CONFIG_TDLS */

#ifdef SBWC
		psta->SBWC_mode = SBWC_MODE_DISABLE;
		psta->SBWC_tx_limit = 0;
		psta->SBWC_rx_limit = 0;
		psta->SBWC_tx_limit_byte = 0;
		psta->SBWC_rx_limit_byte = 0;
		psta->SBWC_tx_count = 0;
		psta->SBWC_rx_count = 0;
		skb_queue_head_init(&psta->SBWC_txq);
		skb_queue_head_init(&psta->SBWC_rxq);
		psta->SBWC_consuming_q = 0;

		for (i = 0 ; i < pstapriv->padapter->registrypriv.wifi_mib.sbwcEntry.count ; ++i)
		{
			struct _SBWC_ENTRY *entry = &(pstapriv->padapter->registrypriv.wifi_mib.sbwcEntry.entry[i]);
			if (!memcmp(entry->mac, hwaddr, MAC_ADDR_LEN))
			{
				psta->SBWC_tx_limit = entry->tx_lmt;
				psta->SBWC_rx_limit = entry->rx_lmt;
				psta->SBWC_tx_limit_byte = ((psta->SBWC_tx_limit * 1024 / 8) / (HZ / SBWC_TO));
				psta->SBWC_rx_limit_byte = ((psta->SBWC_rx_limit * 1024 / 8) / (HZ / SBWC_TO));
				break;
			}
		}
#endif

#ifdef RTW_STA_BWC
		psta->sta_bwc_tx_limit = 0;
		psta->sta_bwc_tx_limit_byte = 0;
		psta->sta_bwc_tx_cnt = 0;
		skb_queue_head_init(&psta->sta_bwc_txq);
		psta->sta_bwc_consuming_q = 0;
#endif

#ifdef GBWC
		psta->GBWC_in_group = 0;
		for (i = 0; i < pstapriv->padapter->registrypriv.wifi_mib.gbwcEntry.count; i++)
		{
			struct _GBWC_ENTRY *entry = &(pstapriv->padapter->registrypriv.wifi_mib.gbwcEntry.entry[i]);
			if (!memcmp(entry->mac, hwaddr, MAC_ADDR_LEN))
			{
				psta->GBWC_in_group = 1;
				break;
			}
		}
#endif

		/* for A-MPDU Rx reordering buffer control */
		for (i = 0; i < 16 ; i++) {
			preorder_ctrl = &psta->recvreorder_ctrl[i];
			preorder_ctrl->padapter = pstapriv->padapter;
			preorder_ctrl->tid = i;
			preorder_ctrl->enable = _FALSE;
			preorder_ctrl->indicate_seq = 0xffff;
			#ifdef DBG_RX_SEQ
			RTW_INFO("DBG_RX_SEQ "FUNC_ADPT_FMT" tid:%u SN_CLEAR indicate_seq:%d\n"
				, FUNC_ADPT_ARG(pstapriv->padapter), i, preorder_ctrl->indicate_seq);
			#endif
			preorder_ctrl->wend_b = 0xffff;
			/* preorder_ctrl->wsize_b = (NR_RECVBUFF-2); */
			preorder_ctrl->wsize_b = 64;/* 64; */
			preorder_ctrl->ampdu_size = RX_AMPDU_SIZE_INVALID;

			_rtw_init_queue(&preorder_ctrl->pending_recvframe_queue);

			rtw_init_recv_timer(preorder_ctrl);
			rtw_clear_bit(RTW_RECV_ACK_OR_TIMEOUT, &preorder_ctrl->rec_abba_rsp_ack);

		}
		ATOMIC_SET(&psta->keytrack, 0);

		/* ToDo: need to API to init hal_sta->ra_info->rssi_stat.rssi  */
		#if 0
		psta->phl_sta->rssi_stat.rssi = (-1);
		psta->phl_sta->rssi_stat.rssi_cck = (-1);
		psta->phl_sta->rssi_stat.rssi_ofdm = (-1);
		#endif
#ifdef CONFIG_ATMEL_RC_PATCH
		psta->flag_atmel_rc = 0;
#endif

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
		psta->tbtx_enable = _FALSE;
#endif
		/* init for the sequence number of received management frame */
		psta->RxMgmtFrameSeqNum = 0xffff;
		_rtw_memset(&psta->sta_stats, 0, sizeof(struct stainfo_stats));

		/* allocate macid move to rtw_phl_alloc_stainfo  */
		/* rtw_alloc_macid(pstapriv->padapter, psta); */

		psta->tx_q_enable = 0;
		_rtw_init_queue(&psta->tx_queue);
		_init_workitem(&psta->tx_q_work, rtw_xmit_dequeue_callback, NULL);

#ifdef CONFIG_TXSC_AMSDU
		txsc_amsdu_queue_init(pstapriv->padapter, psta);
#endif
#ifdef CONFIG_RTW_TWT
		for (i = 0; i < 4 ; i++) {
			psta->twt_cur_operate_num[i] = 0;
		}
#endif
		if (phl_role->chandef.band == BAND_ON_24G)
			psta->cur_tx_data_rate = RTW_DATA_RATE_CCK1;
		else
			psta->cur_tx_data_rate = RTW_DATA_RATE_OFDM6;

#ifdef CONFIG_RTW_CORE_RXSC
		core_rxsc_clean_entry(pstapriv->padapter, hwaddr, psta);
#endif
	}

exit:

	_rtw_spinunlock_bh(&(pstapriv->sta_hash_lock));

	//if (psta)
	//	rtw_mi_update_iface_status(&(pstapriv->padapter->mlmepriv), 0);

	return psta;
}

static u8 _rtw_alloc_phl_stainfo(struct sta_info *sta, struct	sta_priv *stapriv, const u8 *hwaddr, enum phl_cmd_type cmd_type)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct rtw_phl_stainfo_t *tmp_sta = NULL;	// prevent phl_sta NULL issue
	u8 res = _FAIL;
	struct rtw_wifi_role_t *wrole = stapriv->padapter->phl_role;
	bool alloc = _TRUE, only_hw = _FALSE;

	/* Do not use this function in interrupt context  */
	pstatus = rtw_phl_cmd_alloc_stainfo(GET_HAL_INFO(adapter_to_dvobj(stapriv->padapter)), &tmp_sta,
										(u8 *)hwaddr, wrole,
										alloc, only_hw,
										cmd_type, 0);

	if (tmp_sta) {
		/* Associate PHL sta with core's */
		sta->phl_sta = tmp_sta;
		sta->phl_sta->core_data = (void *)sta;
		//rtw_dump_phl_sta_info(RTW_DBGDUMP, sta);
		res = _SUCCESS;
	} else {
		RTW_ERR(FUNC_ADPT_FMT ": fail to alloc PHL sta "
			"for " MAC_FMT " !\n",
			FUNC_ADPT_ARG(stapriv->padapter),
			MAC_ARG(hwaddr));
	}

	return res;
}

u8 rtw_get_sta_disabled(struct rtw_phl_stainfo_t *phl_sta)
{
#ifndef CONFIG_RTW_LINK_PHL_MASTER
	return phl_sta->disabled;
#else
	return !phl_sta->active;
#endif
}

void rtw_set_sta_disabled(struct rtw_phl_stainfo_t *phl_sta, u8 disable)
{
#ifndef CONFIG_RTW_LINK_PHL_MASTER
	phl_sta->disabled = disable;
#endif
}

extern void _rtw_free_phl_stainfo(_adapter *adapter, struct rtw_phl_stainfo_t *phl_sta, u8 free_type);

struct sta_info *rtw_alloc_stainfo(struct	sta_priv *stapriv, const u8 *hwaddr, enum phl_cmd_type cmd_type)
{
	struct sta_info *sta;

#ifdef RTW_WKARD_AP_CMD_DISPATCH
	_rtw_spinlock_bh(&(stapriv->sta_info_lock));
#endif

	/* can use in interrupt context */
	sta = _rtw_alloc_core_stainfo(stapriv, hwaddr);
	if (sta) {
		/* Delayed freeing PHL stainfo, if it is not used */
		if (sta->phl_sta &&
		    rtw_get_sta_disabled(sta->phl_sta)) {
			_rtw_free_phl_stainfo(stapriv->padapter, sta->phl_sta, RTW_FREE_PHL_STA_SW);
			rtw_set_sta_disabled(sta->phl_sta, 0);
		}

		/* can not use in interrupt context */
		if (_rtw_alloc_phl_stainfo(sta, stapriv, hwaddr, cmd_type) == _FAIL)
			goto free_core_sta;

		rtw_set_sta_disabled(sta->phl_sta, 0);

		if (rtw_insert_core_stainfo_hash(stapriv, sta, hwaddr) == _FAIL) {
			RTW_WARN("[%s %d] insert core stainfo hash fail\n", __FUNCTION__, __LINE__);
			goto free_phl_sta;
		}

		rtw_set_bit(RTW_BIT_STA_ALLOC_DONE, &sta->check_state);

		rtw_mi_update_iface_status(&(stapriv->padapter->mlmepriv), 0);
	}

#ifdef RTW_WKARD_AP_CMD_DISPATCH
	_rtw_spinunlock_bh(&(stapriv->sta_info_lock));
#endif

	return sta;

free_phl_sta:

	_rtw_free_phl_stainfo(stapriv->padapter, sta->phl_sta, RTW_FREE_PHL_STA_ALL);
	rtw_set_sta_disabled(sta->phl_sta, 0);

free_core_sta:

	rtw_insert_core_stainfo_freeq(stapriv, sta);

#ifdef RTW_WKARD_AP_CMD_DISPATCH
	_rtw_spinunlock_bh(&(stapriv->sta_info_lock));
#endif

	return NULL;
}

struct sta_info *rtw_alloc_stainfo_sw(struct	sta_priv *stapriv, const u8 *hwaddr)
{
	struct sta_info *sta;
	/* can use in interrupt context */
	sta = _rtw_alloc_core_stainfo(stapriv, hwaddr);

	if (sta != NULL) {
                /*
                * TODO:
                *   For PCIe case, it is already able to I/O at STA auth/associate stage, so it is ok to alloc both sw and hw stainfo;
                *   However, for USB case, it is unable to I/O in RX flow, so it should alloc SW first, wait STA connected, and alloc HW later in cmd thread.
                */
		sta->phl_sta = rtw_phl_alloc_stainfo_sw(
			GET_HAL_INFO(adapter_to_dvobj(stapriv->padapter)),
			(u8 *)hwaddr, stapriv->padapter->phl_role);

		if (sta->phl_sta) {
			rtw_set_sta_disabled(sta->phl_sta, 0);
			if (rtw_insert_core_stainfo_hash(stapriv, sta, hwaddr) == _FAIL) {
				RTW_WARN("insert core stainfo hash fail\n");
				goto free_core_sta;
			}
			//rtw_dump_phl_sta_info(RTW_DBGDUMP, sta);
		} else {
			RTW_ERR(FUNC_ADPT_FMT ": fail to alloc PHL sta "
				"for " MAC_FMT " !\n",
				FUNC_ADPT_ARG(stapriv->padapter),
				MAC_ARG(hwaddr));
			goto free_core_sta;
		}
	}

	return sta;

free_core_sta:

	rtw_insert_core_stainfo_freeq(stapriv, sta);

	return NULL;
}

u32 rtw_alloc_stainfo_hw(struct	sta_priv *stapriv, struct sta_info *psta)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	void *phl = GET_HAL_INFO(adapter_to_dvobj(stapriv->padapter));
	struct rtw_wifi_role_t *wrole = stapriv->padapter->phl_role;
	bool alloc = _TRUE, only_hw = _TRUE;

	if ((psta != NULL) && (psta->phl_sta != NULL) && (psta->phl_sta->active == _FALSE))
		status = rtw_phl_cmd_alloc_stainfo(phl, &psta->phl_sta,
										(u8 *)psta->phl_sta->mac_addr,
										wrole, alloc, only_hw,
										PHL_CMD_DIRECTLY, 0);

	return (status == RTW_PHL_STATUS_SUCCESS) ? _SUCCESS : _FAIL;
}

/* using pstapriv->sta_hash_lock to protect */
u32 static _rtw_free_core_stainfo(_adapter *padapter , struct sta_info *psta)
{
	int i, j;
	struct recv_reorder_ctrl *preorder_ctrl;
	struct	sta_xmit_priv	*pstaxmitpriv;
	struct	xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct hw_xmit *phwxmit;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	int pending_qcnt[4];
#if CONFIG_RTW_PRE_LINK_STA
	u8 is_pre_link_sta = _FALSE;
#endif
#ifdef RTW_STA_BWC
	struct sk_buff *pskb = NULL;
#endif

	if (psta == NULL)
		goto exit;

#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	rtw_flush_brsc_info(NULL, psta->phl_sta->mac_addr);
#endif

#if CONFIG_RTW_PRE_LINK_STA
	is_pre_link_sta = rtw_is_pre_link_sta(pstapriv, psta->phl_sta->mac_addr);

	if (is_pre_link_sta == _TRUE) {
		_rtw_spinlock_bh(&psta->lock);
		psta->state = WIFI_FW_PRE_LINK;
		_rtw_spinunlock_bh(&psta->lock);
	} else
#endif
	{
		// delete hash_list first to prevent others from accessing
		_rtw_spinlock_bh(&(pstapriv->sta_hash_lock));
		if(pstapriv->psta_cache == psta){
			pstapriv->psta_cache = NULL;
		}
		rtw_list_delete(&psta->hash_list);
		pstapriv->asoc_sta_count--;
		_rtw_spinunlock_bh(&(pstapriv->sta_hash_lock));
		rtw_mi_update_iface_status(&(padapter->mlmepriv), 0);
	}

#ifdef CONFIG_TXSC_AMSDU
	txsc_amsdu_queue_free(padapter, psta, 0);
#endif

#ifdef RTW_STA_BWC
	while(skb_queue_len(&psta->sta_bwc_txq)) {
		pskb = skb_dequeue(&psta->sta_bwc_txq);
		if (pskb)
			rtw_os_pkt_complete(padapter, pskb);
	}
#endif

#if defined(CONFIG_CORE_TXSC) && defined(USE_ONE_WLHDR)
	/* free shortcut entry wlhdr buffer */
	for (i = 0; i < CORE_TXSC_ENTRY_NUM; i++) {
		for (j = 0; j < WMM_NUM * WLHDR_NUM; j++) {
			if (psta->txsc_entry_cache[i].txsc_wlhdr[j]) {
				rtw_mfree(psta->txsc_entry_cache[i].txsc_wlhdr[j], CORE_TXSC_WLHDR_SIZE);
				psta->txsc_entry_cache[i].txsc_wlhdr[j] = NULL;
			}
		}
	}
#endif

#ifdef CONFIG_RTW_CORE_RXSC
	core_rxsc_clean_entry(padapter, psta->phl_sta->mac_addr, psta);
#endif

#ifdef CONFIG_RTW_A4_STA /* A4_AMSDU */
	if (WLAN_STA_A4 & psta->flags)
		core_a4_sta_cleanup(padapter, psta);
#endif

#ifdef CONFIG_RTW_80211K
	rm_post_event(padapter, RM_ID_FOR_ALL(psta->phl_sta->aid), RM_EV_cancel);
#endif

	RTW_INFO("del all key before free stainfo (" MAC_FMT ")\n", MAC_ARG(psta->phl_sta->mac_addr));
	rtw_hw_del_all_key(padapter, psta, PHL_CMD_DIRECTLY, 0);

	_rtw_spinlock_bh(&psta->lock);
	psta->state &= ~WIFI_ASOC_STATE;
	_rtw_spinunlock_bh(&psta->lock);

	pstaxmitpriv = &psta->sta_xmitpriv;

	/* rtw_list_delete(&psta->sleep_list); */

	/* rtw_list_delete(&psta->wakeup_list); */

	rtw_free_xmitframe_queue(pxmitpriv, &psta->tx_queue);
	_rtw_deinit_queue(&psta->tx_queue);

	_rtw_spinlock_bh(&pxmitpriv->lock);

	rtw_core_free_sta_xmit_queue(padapter, pstaxmitpriv);
#if !defined(USE_HIQ)
	rtw_core_free_xmitframe_queue(padapter, &psta->sleep_q);
#endif

	//rtw_os_wake_queue_at_free_stainfo(padapter, pending_qcnt);

	_rtw_spinunlock_bh(&pxmitpriv->lock);


	/* re-init sta_info; 20061114 */ /* will be init in alloc_stainfo */
	/* _rtw_init_sta_xmit_priv(&psta->sta_xmitpriv); */
	/* _rtw_init_sta_recv_priv(&psta->sta_recvpriv); */
#if defined(CONFIG_IEEE80211W) && !defined(CONFIG_IOCTL_CFG80211)
	_cancel_timer_ex(&psta->dot11w_expire_timer);
	psta->sa_query_timed_out = 0;
	psta->sa_query_cnt = 0;
#endif /* CONFIG_IEEE80211W */
	_cancel_timer_ex(&psta->addba_retry_timer);

#ifdef CONFIG_TDLS
	psta->tdls_sta_state = TDLS_STATE_NONE;
#endif /* CONFIG_TDLS */

	/* for A-MPDU Rx reordering buffer control, cancel reordering_ctrl_timer */
	for (i = 0; i < 16 ; i++) {
		_list	*phead, *plist;
		union recv_frame *prframe;
		_queue *ppending_recvframe_queue;
		_queue *pfree_recv_queue = &padapter->recvpriv.free_recv_queue;

		preorder_ctrl = &psta->recvreorder_ctrl[i];
		rtw_clear_bit(RTW_RECV_ACK_OR_TIMEOUT, &preorder_ctrl->rec_abba_rsp_ack);

		_cancel_timer_ex(&preorder_ctrl->reordering_ctrl_timer);


		ppending_recvframe_queue = &preorder_ctrl->pending_recvframe_queue;

		_rtw_spinlock_bh(&ppending_recvframe_queue->lock);

		phead =	get_list_head(ppending_recvframe_queue);
		plist = get_next(phead);

		while (plist && !rtw_is_list_empty(phead)) {
			prframe = LIST_CONTAINOR(plist, union recv_frame, u);

			plist = get_next(plist);

			rtw_list_delete(&(prframe->u.hdr.list));

			rtw_free_recvframe(prframe, pfree_recv_queue);
		}

		_rtw_spinunlock_bh(&ppending_recvframe_queue->lock);

	}

	if (!((psta->state & WIFI_AP_STATE) || MacAddr_isBcst(psta->phl_sta->mac_addr))
#if CONFIG_RTW_PRE_LINK_STA
		&& is_pre_link_sta == _FALSE
#endif
		)
		rtw_hal_set_phydm_var(padapter, HAL_PHYDM_STA_INFO, psta, _FALSE);


	/* release mac id for non-bc/mc station() move to PHL */
	#if 0
	if (is_pre_link_sta == _FALSE)
		rtw_release_macid(pstapriv->padapter, psta);
	#endif

#ifdef CONFIG_AP_MODE

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	if (!rtw_is_list_empty(&psta->asoc_list)) {
		rtw_list_delete(&psta->asoc_list);
		pstapriv->asoc_list_cnt--;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	_rtw_spinlock_bh(&pstapriv->auth_list_lock);
	if (!rtw_is_list_empty(&psta->auth_list)) {
		rtw_list_delete(&psta->auth_list);
		pstapriv->auth_list_cnt--;
	}
	_rtw_spinunlock_bh(&pstapriv->auth_list_lock);

	psta->expire_to = 0;
#ifdef CONFIG_ATMEL_RC_PATCH
	psta->flag_atmel_rc = 0;
#endif
	psta->qos_info = 0;

	psta->max_sp_len = 0;
	psta->uapsd_bitmap = 0;

#ifdef CONFIG_NATIVEAP_MLME

	if (MLME_IS_AP(padapter)) {
		rtw_tim_map_clear(padapter, pstapriv->sta_dz_bitmap, psta->phl_sta->aid);
		rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);

		/* rtw_indicate_sta_disassoc_event(padapter, psta); */

		if ((psta->phl_sta->aid > 0) && (pstapriv->sta_aid[psta->phl_sta->aid - 1] == psta)) {
			pstapriv->sta_aid[psta->phl_sta->aid - 1] = NULL;
			psta->phl_sta->aid = 0;
		}
	}

#endif /* CONFIG_NATIVEAP_MLME	 */

	_rtw_spinlock_bh(&psta->lock);
	psta->state &= ~(WIFI_DELETE_STATE);
	_rtw_spinunlock_bh(&psta->lock);
#endif /* CONFIG_AP_MODE	 */

	rtw_st_ctl_deinit(&psta->st_ctl);

#if CONFIG_RTW_PRE_LINK_STA
	if (is_pre_link_sta == _FALSE)
#endif
	{
		_rtw_spinlock_free(&psta->lock);
	}

exit:
	return _SUCCESS;
}

u32 rtw_init_stainfo(_adapter *padapter , struct sta_info *psta)
{
	int i, j;
	struct recv_reorder_ctrl *preorder_ctrl;
	struct	sta_xmit_priv	*pstaxmitpriv;
	struct	xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct hw_xmit *phwxmit;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	int pending_qcnt[4];

	if (psta == NULL)
		goto exit;

#if defined(CONFIG_CORE_TXSC)
	psta->txsc_cache_num = 0;
	psta->txsc_cur_idx = 0;
	psta->txsc_cache_idx = 0;
	psta->txsc_cache_hit = 0;
	psta->txsc_a4_hit = 0;
	#ifdef CONFIG_TXSC_AMSDU
	psta->txsc_amsdu_hit = 0;
	psta->txsc_a4_amsdu_hit = 0;
	#endif
	psta->txsc_cache_miss = 0;
	psta->txsc_path_slow = 0;
	#ifdef CONFIG_RTW_ENABLE_HW_TXSC
	psta->hw_txsc_hit = 0;
	psta->hw_txsc_set = 0;
	#endif
#if defined(USE_ONE_WLHDR)
	/* free shortcut entry wlhdr buffer */
	for (i = 0; i < CORE_TXSC_ENTRY_NUM; i++) {
		for (j = 0; j < WMM_NUM * WLHDR_NUM; j++) {
			if (psta->txsc_entry_cache[i].txsc_wlhdr[j]) {
				rtw_mfree(psta->txsc_entry_cache[i].txsc_wlhdr[j], CORE_TXSC_WLHDR_SIZE);
				psta->txsc_entry_cache[i].txsc_wlhdr[j] = NULL;
			}
		}
	}
#endif /* defined(CONFIG_CORE_TXSC) */
	/* reset txsc entry */
	_rtw_memset(psta->txsc_entry_cache, 0x0, sizeof(struct txsc_entry) * CORE_TXSC_ENTRY_NUM);
	_rtw_clear_phl_txsc(psta->phl_sta);
#endif /* defined(USE_ONE_WLHDR) */

#ifdef CONFIG_RTW_CORE_RXSC
	core_rxsc_clean_entry(padapter, psta->phl_sta->mac_addr, psta);
#endif

#ifdef CONFIG_TXSC_AMSDU
	txsc_amsdu_queue_free(padapter, psta, 1);
#endif

#ifdef CONFIG_RTW_A4_STA /* A4_AMSDU */
	if (WLAN_STA_A4 & psta->flags)
		core_a4_sta_cleanup(padapter, psta);
#endif

#ifdef CONFIG_RTW_80211K
	rm_post_event(padapter, RM_ID_FOR_ALL(psta->phl_sta->aid), RM_EV_cancel);
#endif

	pstaxmitpriv = &psta->sta_xmitpriv;

	rtw_free_xmitframe_queue(pxmitpriv, &psta->tx_queue);

	_rtw_spinlock_bh(&pxmitpriv->lock);

	rtw_core_free_sta_xmit_queue(padapter, pstaxmitpriv);
#if !defined(USE_HIQ)
	rtw_core_free_xmitframe_queue(padapter, &psta->sleep_q);
#endif
	_rtw_spinunlock_bh(&pxmitpriv->lock);

#ifdef CONFIG_TDLS
	psta->tdls_sta_state = TDLS_STATE_NONE;
#endif /* CONFIG_TDLS */

	/* for A-MPDU Rx reordering buffer control, cancel reordering_ctrl_timer */
	for (i = 0; i < 16 ; i++) {
		_list	*phead, *plist;
		union recv_frame *prframe;
		_queue *ppending_recvframe_queue;
		_queue *pfree_recv_queue = &padapter->recvpriv.free_recv_queue;

		preorder_ctrl = &psta->recvreorder_ctrl[i];
		rtw_clear_bit(RTW_RECV_ACK_OR_TIMEOUT, &preorder_ctrl->rec_abba_rsp_ack);

		_cancel_timer_ex(&preorder_ctrl->reordering_ctrl_timer);


		ppending_recvframe_queue = &preorder_ctrl->pending_recvframe_queue;

		_rtw_spinlock_bh(&ppending_recvframe_queue->lock);

		phead =	get_list_head(ppending_recvframe_queue);
		plist = get_next(phead);

		while (plist && !rtw_is_list_empty(phead)) {
			prframe = LIST_CONTAINOR(plist, union recv_frame, u);

			plist = get_next(plist);

			rtw_list_delete(&(prframe->u.hdr.list));

			rtw_free_recvframe(prframe, pfree_recv_queue);
		}

		_rtw_spinunlock_bh(&ppending_recvframe_queue->lock);

	}

#ifdef CONFIG_AP_MODE

#ifdef CONFIG_ATMEL_RC_PATCH
	psta->flag_atmel_rc = 0;
#endif
	psta->qos_info = 0;

	psta->max_sp_len = 0;
	psta->uapsd_bitmap = 0;

#ifdef CONFIG_NATIVEAP_MLME

	if (MLME_IS_AP(padapter)) {
		rtw_tim_map_clear(padapter, pstapriv->sta_dz_bitmap, psta->phl_sta->aid);
		rtw_tim_map_clear(padapter, pstapriv->tim_bitmap, psta->phl_sta->aid);
	}

#endif /* CONFIG_NATIVEAP_MLME	 */

	_rtw_spinlock_bh(&psta->lock);
	psta->state &= ~(WIFI_DELETE_STATE | WIFI_SLEEP_STATE | WIFI_ASOC_STATE);
	_rtw_spinunlock_bh(&psta->lock);
#endif /* CONFIG_AP_MODE	 */

exit:
	return _SUCCESS;
}

void _rtw_free_phl_stainfo(_adapter *adapter, struct rtw_phl_stainfo_t *phl_sta, u8 free_type)
{
	void *phl = GET_HAL_INFO(adapter_to_dvobj(adapter));
	enum rtw_phl_status pstaus = RTW_PHL_STATUS_SUCCESS;
	u8 hwaddr[ETH_ALEN] = {0};
	bool alloc = _FALSE, only_hw = _FALSE;

	if (phl_sta != NULL) {
		_rtw_memcpy(hwaddr, phl_sta->mac_addr, ETH_ALEN);

		if (free_type == RTW_FREE_PHL_STA_SW) {
 			pstaus = rtw_phl_free_stainfo_sw(phl, phl_sta);
 			phl_sta->core_data = NULL;
			#ifdef CONFIG_PHL_TXSC
			_rtw_clear_phl_txsc(phl_sta);
			#endif
		} else if (free_type == RTW_FREE_PHL_STA_HW) {
			pstaus = rtw_phl_cmd_alloc_stainfo(phl, &phl_sta,
										(u8 *)phl_sta->mac_addr,
										adapter->phl_role, _FALSE, _TRUE,
										PHL_CMD_DIRECTLY, 0);
		} else if (free_type == RTW_FREE_PHL_STA_ALL) {
			pstaus = rtw_phl_cmd_alloc_stainfo(phl, &phl_sta,
										(u8 *)phl_sta->mac_addr,
										adapter->phl_role, _FALSE, _FALSE,
										PHL_CMD_DIRECTLY, 0);
			phl_sta->core_data = NULL;
		} else {
			RTW_ERR(FUNC_ADPT_FMT ": unknown free_type\n", FUNC_ADPT_ARG(adapter));
			pstaus = RTW_PHL_STATUS_FAILURE;
		}

		if (pstaus != RTW_PHL_STATUS_SUCCESS)
			RTW_ERR(FUNC_ADPT_FMT ": fail to free PHL sta "
				"for " MAC_FMT " !\n",
				FUNC_ADPT_ARG(adapter),
				MAC_ARG(hwaddr));
	}
}

u32 rtw_free_stainfo(_adapter *padapter, struct sta_info *psta)
{
	struct rtw_phl_stainfo_t *phl_sta;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if (psta == NULL)
		return _FAIL;

#ifdef RTW_WKARD_AP_CMD_DISPATCH
	_rtw_spinlock_bh(&(pstapriv->sta_info_lock));
#endif

	if (!rtw_test_and_clear_bit(RTW_BIT_STA_ALLOC_DONE, &psta->check_state)) {
		RTW_WARN("[%s %d] invalid free sta\n", __FUNCTION__, __LINE__);
#ifdef RTW_WKARD_AP_CMD_DISPATCH
		_rtw_spinunlock_bh(&(pstapriv->sta_info_lock));
#endif
		return _FAIL;
	}

	phl_sta = psta->phl_sta;

	_rtw_free_core_stainfo(padapter, psta);

	/* Must free HW when core STA is being freed to ensure HW setting correct.
	   But delay freeing PHL stainfo until it this core STA structure is used again
	   to make remaining access to PHL stainfo valid */

	//_rtw_free_phl_stainfo(padapter, phl_sta, RTW_FREE_PHL_STA_ALL);
	_rtw_free_phl_stainfo(padapter, phl_sta, RTW_FREE_PHL_STA_HW);
	rtw_set_sta_disabled(phl_sta, 1);

#if CONFIG_RTW_PRE_LINK_STA
	if (!(psta->state & WIFI_FW_PRE_LINK))
#endif
	{
		rtw_insert_core_stainfo_freeq(pstapriv, psta);
	}

#ifdef RTW_WKARD_AP_CMD_DISPATCH
	_rtw_spinunlock_bh(&(pstapriv->sta_info_lock));
#endif

	return _SUCCESS;
}

u32 rtw_free_stainfo_sw(_adapter *padapter, struct sta_info *psta)
{
	struct rtw_phl_stainfo_t *phl_sta;
	struct	sta_priv *pstapriv = &padapter->stapriv;

	if (psta == NULL)
		return _FAIL;

	if (!rtw_test_and_clear_bit(RTW_BIT_STA_ALLOC_DONE, &psta->check_state)) {
		RTW_WARN("[%s %d] invalid free sta\n", __FUNCTION__, __LINE__);
		return _FAIL;
	}

	phl_sta = psta->phl_sta;

	_rtw_free_core_stainfo(padapter, psta);
	_rtw_free_phl_stainfo(padapter, phl_sta, RTW_FREE_PHL_STA_SW);
	rtw_set_sta_disabled(phl_sta, 0);

#if CONFIG_RTW_PRE_LINK_STA
	if (!(psta->state & WIFI_FW_PRE_LINK))
#endif
	{
		rtw_insert_core_stainfo_freeq(pstapriv, psta);
	}

	return _SUCCESS;
}

/* free all stainfo which in sta_hash[all] */
void rtw_free_all_stainfo(_adapter *padapter)
{
	_list	*plist, *phead;
	s32	index;
	struct sta_info *psta = NULL;
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *pbcmc_stainfo = rtw_get_bcmc_stainfo(padapter);
	u8 free_sta_num = 0;
	char free_sta_list[NUM_STA];
	int stainfo_offset;


	if (pstapriv->asoc_sta_count == 1)
		goto exit;

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	for (index = 0; index < NUM_STA; index++) {
		phead = &(pstapriv->sta_hash[index]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info , hash_list);

			plist = get_next(plist);

			if (pbcmc_stainfo != psta) {
				#if CONFIG_RTW_PRE_LINK_STA
				if (rtw_is_pre_link_sta(pstapriv, psta->phl_sta->mac_addr) == _FALSE)
				#endif
					rtw_list_delete(&psta->hash_list);

				stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
				if (stainfo_offset_valid(stainfo_offset))
					free_sta_list[free_sta_num++] = stainfo_offset;
			}

		}
	}

	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);


	for (index = 0; index < free_sta_num; index++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, free_sta_list[index]);
		rtw_free_stainfo(padapter , psta);
	}

exit:
	return;
}

void rtw_free_disabled_stainfo(_adapter *padapter)
{
	_list *plist, *phead;
	struct sta_info *psta;
	struct  sta_priv *pstapriv = &padapter->stapriv;
	struct rtw_wifi_role_t *phl_role = pstapriv->padapter->phl_role;
	_queue *pfree_sta_queue = pstapriv->pfree_sta_queue;
	char free_sta_list[NUM_STA];
	int stainfo_offset = 0;
	u32 idx = 0;
	u8 free_sta_num = 0;

	_rtw_spinlock_bh(&(pfree_sta_queue->lock));

	phead = &(pfree_sta_queue->queue);
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, free_list);
		plist = get_next(plist);

		if (psta->phl_sta) {
			stainfo_offset = rtw_stainfo_offset(pstapriv, psta);
			if (stainfo_offset_valid(stainfo_offset))
				free_sta_list[free_sta_num++] = stainfo_offset;
		}
	}

	_rtw_spinunlock_bh(&(pfree_sta_queue->lock));

	for (idx = 0; idx < free_sta_num; idx++) {
		psta = rtw_get_stainfo_by_offset(pstapriv, free_sta_list[idx]);

		if (rtw_get_sta_disabled(psta->phl_sta)) {
			RTW_INFO("[%s %d] free disabled phl_sta, %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, __LINE__,
				psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2],
				psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5]);
			/* Free PHL stainfo only, HW is cleared when it is disabled */
			_rtw_free_phl_stainfo(padapter, psta->phl_sta, RTW_FREE_PHL_STA_SW);
			rtw_set_sta_disabled(psta->phl_sta, 0);
		}
		//psta->phl_sta = NULL;
	}
}


/* any station allocated can be searched by hash list */
__IMEM_WLAN_SECTION__
struct sta_info *rtw_get_stainfo(struct sta_priv *pstapriv, const u8 *hwaddr)
{
	_list	*plist, *phead;
	struct sta_info *psta = NULL;
	struct rtw_phl_stainfo_t *phl_sta_self = NULL;
	void *phl = GET_HAL_INFO(adapter_to_dvobj(pstapriv->padapter));
	struct rtw_wifi_role_t *phl_role = pstapriv->padapter->phl_role;
	_adapter *padapter = pstapriv->padapter;
	u32	index;

	if (phl_role == NULL)
		return NULL;

	if (hwaddr == NULL)
		return NULL;

	psta = pstapriv->psta_cache;
	if(psta && psta->phl_sta && psta->phl_sta->active && EQ_MAC_ADDR(psta->phl_sta->mac_addr, hwaddr)) {
		padapter->core_logs.rxsc_sta_get[0]++;
		return psta;
	} else {
		psta = NULL;
	}

	/* if addr is bcmc addr, return self phl_sta */
	if (IS_MCAST(hwaddr)) {
		phl_sta_self = rtw_phl_get_stainfo_self(phl, phl_role);

		if (phl_sta_self == NULL) {
			RTW_INFO("%s: get phl sta self fail", __func__);
			return NULL;
		}

		/*
		due to using self sta to replace bcmc sta
		change index to self hash index
		*/

		index = wifi_mac_hash(phl_role->mac_addr);
	} else {

		/*
		for sta mode due to self sta info & AP sta info are the same sta info
		using self hash index for sta mode, or bmc sta will not found self sta
		*/
		switch (phl_role->type) {
		case PHL_RTYPE_NONE:
		case PHL_RTYPE_STATION:
		case PHL_RTYPE_ADHOC:
		case PHL_RTYPE_P2P_DEVICE:
		case PHL_RTYPE_P2P_GC:
			index = wifi_mac_hash(phl_role->mac_addr);
		break;
		default:
			index = wifi_mac_hash(hwaddr);
		break;
		}
	}

	_rtw_spinlock_bh(&pstapriv->sta_hash_lock);

	phead = &(pstapriv->sta_hash[index]);
	plist = get_next(phead);


	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {

		psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

		if (psta->phl_sta == NULL) {
			psta = NULL;
			RTW_ERR("phl_sta of sta is NULL\n");
			plist = get_next(plist);
			continue;
		}

		/* if phl_sta is not active, it means this phl_sta is not use now */
		if (!psta->phl_sta->active) {
			psta = NULL;
			plist = get_next(plist);
			continue;
		}

		/* if add is bcmc addr, find matched self phl_sta, and return  */
		if (IS_MCAST(hwaddr) && psta->phl_sta == phl_sta_self)
			break;

		/* if add is not bcmc addr, compare mac_addr  */
		if ((_rtw_memcmp(psta->phl_sta->mac_addr, hwaddr, ETH_ALEN)) == _TRUE) {
			/* if found the matched address */
			break;
		}

		psta = NULL;
		plist = get_next(plist);
	}
	if(psta && (phl_sta_self == NULL)) {
		pstapriv->psta_cache = psta;
		padapter->core_logs.rxsc_sta_get[1]++;
	}
	_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);

	return psta;

}

struct sta_info *rtw_get_stainfo_by_macid(struct sta_priv *pstapriv, u16 macid)
{
	_adapter *padapter = pstapriv->padapter;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	struct sta_info *psta = NULL;

	phl_sta = rtw_phl_get_stainfo_by_macid(GET_HAL_INFO(padapter->dvobj), macid);
	if(phl_sta) {
		psta = (struct sta_info *)phl_sta->core_data;
		if (!psta)
			psta = rtw_get_stainfo(pstapriv, phl_sta->mac_addr);
	}
	return psta;
}

u32	rtw_free_self_stainfo(_adapter *adapter)
{
	struct sta_info *sta = NULL;
	struct sta_priv *stapriv = &adapter->stapriv;

	sta = rtw_get_stainfo(stapriv, adapter->phl_role->mac_addr);

	if (sta != NULL) {
		struct rtw_phl_stainfo_t *phl_sta = sta->phl_sta;

		if (!rtw_test_and_clear_bit(RTW_BIT_STA_ALLOC_DONE, &sta->check_state)) {
			RTW_WARN("[%s %d] invalid free sta\n", __FUNCTION__, __LINE__);
			return _FAIL;
		}

		_rtw_free_core_stainfo(adapter, sta);
		_rtw_free_phl_stainfo(adapter, phl_sta, RTW_FREE_PHL_STA_ALL);
		rtw_set_sta_disabled(phl_sta, 0);

#if CONFIG_RTW_PRE_LINK_STA
		if (!(sta->state & WIFI_FW_PRE_LINK))
#endif
		{
			rtw_insert_core_stainfo_freeq(stapriv, sta);
		}

		adapter->self_sta = NULL;
	}

	return _SUCCESS;
}

u32 rtw_init_self_stainfo(_adapter *padapter, enum phl_cmd_type cmd_type)
{

	struct sta_info *psta;
	struct tx_servq *ptxservq;
	u32 res = _SUCCESS;
	struct sta_priv *pstapriv = &padapter->stapriv;

	psta = rtw_get_stainfo(pstapriv, padapter->phl_role->mac_addr);

	if (psta == NULL) {
		psta = rtw_alloc_stainfo(pstapriv, padapter->phl_role->mac_addr, cmd_type);
		if (psta == NULL) {
			RTW_ERR("%s alloc self sta fail\n", __func__);
			res = _FAIL;
			padapter->self_sta = NULL;
			goto exit;
		}
		padapter->self_sta = psta;
	}
exit:
	return res;

}


struct sta_info *rtw_get_bcmc_stainfo(_adapter *padapter)
{
	struct sta_info	*psta;
	struct sta_priv	*pstapriv = &padapter->stapriv;
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	psta = rtw_get_stainfo(pstapriv, bc_addr);
	return psta;

}

#ifdef CONFIG_AP_MODE
u16 rtw_aid_alloc(_adapter *adapter, struct sta_info *sta)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	u16 aid, i, used_cnt = 0;

	for (i = 0; i < stapriv->max_aid; i++) {
		aid = ((i + stapriv->started_aid - 1) % stapriv->max_aid) + 1;
		if (stapriv->sta_aid[aid - 1] == NULL)
			break;
		if (++used_cnt >= stapriv->max_num_sta)
			break;
	}

	/* check for aid limit and assoc limit  */
	if (i >= stapriv->max_aid || used_cnt >= stapriv->max_num_sta)
		aid = 0;

	sta->phl_sta->aid = aid;
	if (aid) {
		stapriv->sta_aid[aid - 1] = sta;
		if (stapriv->rr_aid)
			stapriv->started_aid = (aid % stapriv->max_aid) + 1;
	}

	return aid;
}

void dump_aid_status(void *sel, _adapter *adapter)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	u8 *aid_bmp;
	u16 i, used_cnt = 0;

	aid_bmp = rtw_zmalloc(stapriv->aid_bmp_len);
	if (!aid_bmp)
		return;

	for (i = 1; i <= stapriv->max_aid; i++) {
		if (stapriv->sta_aid[i - 1]) {
			aid_bmp[i / 8] |= BIT(i % 8);
			++used_cnt;
		}
	}

	RTW_PRINT_SEL(sel, "used_cnt:%u/%u\n", used_cnt, stapriv->max_aid);
	RTW_MAP_DUMP_SEL(sel, "aid_map:", aid_bmp, stapriv->aid_bmp_len);
	RTW_PRINT_SEL(sel, "\n");

	RTW_PRINT_SEL(sel, "%-2s %-11s\n", "rr", "started_aid");
	RTW_PRINT_SEL(sel, "%2d %11d\n", stapriv->rr_aid, stapriv->started_aid);

	rtw_mfree(aid_bmp, stapriv->aid_bmp_len);
}
#endif /* CONFIG_AP_MODE */

#if CONFIG_RTW_MACADDR_ACL
const char *const _acl_period_str[RTW_ACL_PERIOD_NUM] = {
	"DEV",
	"BSS",
};

const char *const _acl_mode_str[RTW_ACL_MODE_MAX] = {
	"DISABLED",
	"ACCEPT_UNLESS_LISTED",
	"DENY_UNLESS_LISTED",
};

u8 _rtw_access_ctrl(_adapter *adapter, u8 period, const u8 *mac_addr)
{
	u8 res = _TRUE;
	_list *list, *head;
	struct rtw_wlan_acl_node *acl_node;
	u8 match = _FALSE;
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	_queue	*acl_node_q;

	if (period >= RTW_ACL_PERIOD_NUM) {
		rtw_warn_on(1);
		goto exit;
	}

	acl = &stapriv->acl_list[period];
	acl_node_q = &acl->acl_node_q;

	if (acl->mode != RTW_ACL_MODE_ACCEPT_UNLESS_LISTED
		&& acl->mode != RTW_ACL_MODE_DENY_UNLESS_LISTED)
		goto exit;

	_rtw_spinlock_bh(&(acl_node_q->lock));
	head = get_list_head(acl_node_q);
	list = get_next(head);
	while (rtw_end_of_queue_search(head, list) == _FALSE) {
		acl_node = LIST_CONTAINOR(list, struct rtw_wlan_acl_node, list);
		list = get_next(list);

		if (_rtw_memcmp(acl_node->addr, mac_addr, ETH_ALEN)) {
			if (acl_node->valid == _TRUE) {
				match = _TRUE;
				break;
			}
		}
	}

	if (acl->mode == RTW_ACL_MODE_ACCEPT_UNLESS_LISTED)
		res = (match == _TRUE) ?  _FALSE : _TRUE;
	else /* RTW_ACL_MODE_DENY_UNLESS_LISTED */
		res = (match == _TRUE) ?  _TRUE : _FALSE;

	if ((acl->mode == RTW_ACL_MODE_ACCEPT_UNLESS_LISTED)
		&& res == _FALSE)
		acl_node->hit++;

	_rtw_spinunlock_bh(&(acl_node_q->lock));

exit:
	return res;
}

u8 rtw_access_ctrl(_adapter *adapter, const u8 *mac_addr)
{
	int i;

	for (i = 0; i < RTW_ACL_PERIOD_NUM; i++)
		if (_rtw_access_ctrl(adapter, i, mac_addr) == _FALSE)
			return _FALSE;

	return _TRUE;
}

void dump_macaddr_acl(void *sel, _adapter *adapter)
{
	struct sta_priv *stapriv = &adapter->stapriv;
	struct wlan_acl_pool *acl;
	int i, j;

	for (j = 0; j < RTW_ACL_PERIOD_NUM; j++) {
		RTW_PRINT_SEL(sel, "period:%s(%d)\n", acl_period_str(j), j);

		acl = &stapriv->acl_list[j];
		RTW_PRINT_SEL(sel, "mode:%s(%d)\n", acl_mode_str(acl->mode), acl->mode);
		RTW_PRINT_SEL(sel, "num:%d/%d\n", acl->num, NUM_ACL);
		for (i = 0; i < NUM_ACL; i++) {
			if (acl->aclnode[i].valid == _FALSE)
				continue;
			RTW_PRINT_SEL(sel, MAC_FMT"  BlockTimes: %d, ExpireTime = %u\n",
						MAC_ARG(acl->aclnode[i].addr),
						acl->aclnode[i].hit,
						acl->aclnode[i].valid_time);
		}
		RTW_PRINT_SEL(sel, "\n");
	}
}
#endif /* CONFIG_RTW_MACADDR_ACL */

#if CONFIG_RTW_PRE_LINK_STA
bool rtw_is_pre_link_sta(struct sta_priv *stapriv, u8 *addr)
{
	struct pre_link_sta_ctl_t *pre_link_sta_ctl = &stapriv->pre_link_sta_ctl;
	struct sta_info *sta = NULL;
	u8 exist = _FALSE;
	int i;

	_rtw_spinlock_bh(&(pre_link_sta_ctl->lock));
	for (i = 0; i < RTW_PRE_LINK_STA_NUM; i++) {
		if (pre_link_sta_ctl->node[i].valid == _TRUE
			&& _rtw_memcmp(pre_link_sta_ctl->node[i].addr, addr, ETH_ALEN) == _TRUE
		) {
			exist = _TRUE;
			break;
		}
	}
	_rtw_spinunlock_bh(&(pre_link_sta_ctl->lock));

	return exist;
}
#endif

#if CONFIG_RTW_PRE_LINK_STA
struct sta_info *rtw_pre_link_sta_add(struct sta_priv *stapriv, u8 *hwaddr)
{
	struct pre_link_sta_ctl_t *pre_link_sta_ctl = &stapriv->pre_link_sta_ctl;
	struct pre_link_sta_node_t *node = NULL;
	struct sta_info *sta = NULL;
	u8 exist = _FALSE;
	int i;

	if (rtw_check_invalid_mac_address(hwaddr, _FALSE) == _TRUE)
		goto exit;

	_rtw_spinlock_bh(&(pre_link_sta_ctl->lock));
	for (i = 0; i < RTW_PRE_LINK_STA_NUM; i++) {
		if (pre_link_sta_ctl->node[i].valid == _TRUE
			&& _rtw_memcmp(pre_link_sta_ctl->node[i].addr, hwaddr, ETH_ALEN) == _TRUE
		) {
			node = &pre_link_sta_ctl->node[i];
			exist = _TRUE;
			break;
		}

		if (node == NULL && pre_link_sta_ctl->node[i].valid == _FALSE)
			node = &pre_link_sta_ctl->node[i];
	}

	if (exist == _FALSE && node) {
		_rtw_memcpy(node->addr, hwaddr, ETH_ALEN);
		node->valid = _TRUE;
		pre_link_sta_ctl->num++;
	}
	_rtw_spinunlock_bh(&(pre_link_sta_ctl->lock));

	if (node == NULL)
		goto exit;

	sta = rtw_get_stainfo(stapriv, hwaddr);
	if (sta)
		goto odm_hook;

	sta = rtw_alloc_stainfo(stapriv, hwaddr);
	if (!sta)
		goto exit;

	sta->state = WIFI_FW_PRE_LINK;

odm_hook:
	rtw_hal_set_phydm_var(stapriv->padapter, HAL_PHYDM_STA_INFO, sta, _TRUE);

exit:
	return sta;
}

void rtw_pre_link_sta_del(struct sta_priv *stapriv, u8 *hwaddr)
{
	struct pre_link_sta_ctl_t *pre_link_sta_ctl = &stapriv->pre_link_sta_ctl;
	struct pre_link_sta_node_t *node = NULL;
	struct sta_info *sta = NULL;
	u8 exist = _FALSE;
	int i;

	if (rtw_check_invalid_mac_address(hwaddr, _FALSE) == _TRUE)
		goto exit;

	_rtw_spinlock_bh(&(pre_link_sta_ctl->lock));
	for (i = 0; i < RTW_PRE_LINK_STA_NUM; i++) {
		if (pre_link_sta_ctl->node[i].valid == _TRUE
			&& _rtw_memcmp(pre_link_sta_ctl->node[i].addr, hwaddr, ETH_ALEN) == _TRUE
		) {
			node = &pre_link_sta_ctl->node[i];
			exist = _TRUE;
			break;
		}
	}

	if (exist == _TRUE && node) {
		node->valid = _FALSE;
		pre_link_sta_ctl->num--;
	}
	_rtw_spinunlock_bh(&(pre_link_sta_ctl->lock));

	if (exist == _FALSE)
		goto exit;

	sta = rtw_get_stainfo(stapriv, hwaddr);
	if (!sta)
		goto exit;

	if (sta->state == WIFI_FW_PRE_LINK)
		rtw_free_stainfo(stapriv->padapter, sta);

exit:
	return;
}

void rtw_pre_link_sta_ctl_reset(struct sta_priv *stapriv)
{
	struct pre_link_sta_ctl_t *pre_link_sta_ctl = &stapriv->pre_link_sta_ctl;
	struct pre_link_sta_node_t *node = NULL;
	struct sta_info *sta = NULL;
	int i, j = 0;

	u8 addrs[RTW_PRE_LINK_STA_NUM][ETH_ALEN];

	_rtw_memset(addrs, 0, RTW_PRE_LINK_STA_NUM * ETH_ALEN);

	_rtw_spinlock_bh(&(pre_link_sta_ctl->lock));
	for (i = 0; i < RTW_PRE_LINK_STA_NUM; i++) {
		if (pre_link_sta_ctl->node[i].valid == _FALSE)
			continue;
		_rtw_memcpy(&(addrs[j][0]), pre_link_sta_ctl->node[i].addr, ETH_ALEN);
		pre_link_sta_ctl->node[i].valid = _FALSE;
		pre_link_sta_ctl->num--;
		j++;
	}
	_rtw_spinunlock_bh(&(pre_link_sta_ctl->lock));

	for (i = 0; i < j; i++) {
		sta = rtw_get_stainfo(stapriv, &(addrs[i][0]));
		if (!sta)
			continue;

		if (sta->state == WIFI_FW_PRE_LINK)
			rtw_free_stainfo(stapriv->padapter, sta);
	}
}

void rtw_pre_link_sta_ctl_init(struct sta_priv *stapriv)
{
	struct pre_link_sta_ctl_t *pre_link_sta_ctl = &stapriv->pre_link_sta_ctl;
	int i;

	_rtw_spinlock_init(&pre_link_sta_ctl->lock);
	pre_link_sta_ctl->num = 0;
	for (i = 0; i < RTW_PRE_LINK_STA_NUM; i++)
		pre_link_sta_ctl->node[i].valid = _FALSE;
}

void rtw_pre_link_sta_ctl_deinit(struct sta_priv *stapriv)
{
	struct pre_link_sta_ctl_t *pre_link_sta_ctl = &stapriv->pre_link_sta_ctl;
	int i;

	rtw_pre_link_sta_ctl_reset(stapriv);

	_rtw_spinlock_free(&pre_link_sta_ctl->lock);
}

void dump_pre_link_sta_ctl(void *sel, struct sta_priv *stapriv)
{
	struct pre_link_sta_ctl_t *pre_link_sta_ctl = &stapriv->pre_link_sta_ctl;
	int i;

	RTW_PRINT_SEL(sel, "num:%d/%d\n", pre_link_sta_ctl->num, RTW_PRE_LINK_STA_NUM);

	for (i = 0; i < RTW_PRE_LINK_STA_NUM; i++) {
		if (pre_link_sta_ctl->node[i].valid == _FALSE)
			continue;
		RTW_PRINT_SEL(sel, MAC_FMT"\n", MAC_ARG(pre_link_sta_ctl->node[i].addr));
	}
}
#endif /* CONFIG_RTW_PRE_LINK_STA */

void rtw_update_stainfo_by_asoc(_adapter *padapter, u8 *hwaddr)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *adapter = NULL;
	struct sta_info *psta = NULL;
	int i;

	if(*hwaddr & 0x1)
		return;

	for( i = 0; i < CONFIG_IFACE_NUMBER; i++)
	{
		adapter = dvobj->padapters[i];
		if(adapter && MLME_IS_AP(adapter))
		{
			if(!netif_running(adapter->pnetdev))
				continue;

#ifdef CONFIG_RTW_A4_STA
			if(adapter->a4_enable == 1)
				a4_del_source_db_entry(adapter, hwaddr);
#endif

			if(adapter == padapter)
				continue;

			psta = rtw_get_stainfo(&adapter->stapriv, hwaddr);
			if(psta)
				ap_free_sta(adapter, psta, _FALSE, WLAN_REASON_DEAUTH_LEAVING, _TRUE, _FALSE);
			psta = NULL;
		}
	}
	return;
}

