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
#ifndef _RTW_XMIT_SHORTCUT_H_
#define _RTW_XMIT_SHORTCUT_H_

#ifdef CONFIG_CORE_TXSC

#ifdef CONFIG_RTW_TXSC_USE_HW_SEQ
#define USE_ONE_WLHDR /* do not use one hdr when use sw amsdu in txsc */

#ifdef USE_ONE_WLHDR
#define WMM_NUM 4 /* HW SEQ TID number */
#define WLHDR_NUM 6 /* wlhdr type: (ampdu, amsdu (00, 01)) * (mdata, eosp (00,10,01)) */
static const u32 hwswq_tid_remap[4] = {0, 1, 4, 6}; // BE, BK, VO, VI
#endif
#endif

#define CIRC_CNT_RTW(head,tail,size)    ((head>=tail)?(head-tail):(size-tail+head))
#define MAX_TIME_SIZE 0xFFFFFFFF
#define DEFAULT_CORE_TXSC_THRES 10000 // pps
#define DEFAULT_CORE_TXSC_TIME_DURATION 4 // jiffies
#define CORE_TXSC_ENTRY_NUM 8
#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)
#define CORE_TXSC_WLHDR_SIZE _ALIGN((WLHDR_SIZE + SNAP_SIZE + 2 + 18), 32)
#else
#define CORE_TXSC_WLHDR_SIZE (WLHDR_SIZE + SNAP_SIZE + 2 + _AES_IV_LEN_)
#endif

#define CORE_TXSC_DEBUG_BUF_SIZE (sizeof(struct rtw_xmit_req) + sizeof(struct rtw_pkt_buf_list) * 2)

enum txsc_action_type {
	TXSC_NONE,
	TXSC_SKIP,
	TXSC_ADD,
	TXSC_APPLY,
	TXSC_AMSDU_APPLY,
};

enum full_cnt_type {
	PHL_WD_EMPTY,
	PHL_BD_FULL,
	PHL_WD_RECYCLE_NOTHING,
	PHL_WD_RECYCLE_OK,
};

#ifdef CONFIG_TXSC_AMSDU

#define MAX_AMSDU_ENQ_NUM 128//512//256
/* AMSDU_BY_SIZE */
#define AMSDU_0_SIZE 3839
/*
 * When modifying the R_AX_RCR register, we need to modify this value(AMSDU_1_SIZE/AMSDU_2_SIZE) at the same time.
 * hdr_conv_rx_8192xb.c: MAC_REG_W32(R_AX_RCR, val)
 */
#define AMSDU_1_SIZE (7991 - 128 - 64)
#define AMSDU_2_SIZE (11454 - 128 - 64)

#define AMSDU_0_MAX_NUM 2
#define AMSDU_1_MAX_NUM 5
#define AMSDU_2_MAX_NUM 7

enum txsc_amsdu_timer_type {
	TXSC_AMSDU_TIMER_UNSET,
	TXSC_AMSDU_TIMER_SETTING,
	TXSC_AMSDU_TIMER_TIMEOUT,
};

enum txsc_amsdu_enq_type {
	TXSC_AMSDU_NEED_DEQ,
	TXSC_AMSDU_ENQ_ABORT,
	TXSC_AMSDU_ENQ_SUCCESS,
};

struct txsc_amsdu_swq {
	_lock	txsc_amsdu_lock;
	struct sk_buff *skb_q[MAX_AMSDU_ENQ_NUM];
	u32 skb_qlen;/* AMSDU_BY_SIZE */
	u32 wptr;
	u32 rptr;
	u32 cnt;
#ifdef CONFIG_AMSDU_HW_TIMER
	u32 hw_timer_cnt;
	_list list;
	struct sta_info *psta;
	u8 ac;
	u8 is_hw_timer_pending;
#endif
	 u8 is_normal_deq;
};
#endif

struct txsc_pkt_entry {
	enum txsc_action_type step;
	struct sta_info *psta;
	struct rtw_xmit_req *ptxreq;

	u8 txsc_id;
	u8 priority;

	struct sk_buff *xmit_skb[MAX_TXSC_SKB_NUM];
#ifdef RTW_CORE_PKT_TRACE
	struct rtw_pkt_trace_info pktinfo[MAX_TXSC_SKB_NUM];
#endif
	u8 skb_cnt;

#ifdef CONFIG_ETHER_PKT_AGG
	u8 isAggPkt;
#endif /* CONFIG_ETHER_PKT_AGG */

#ifdef CONFIG_TXSC_AMSDU
	u8 ac;
	bool amsdu;
	bool is_amsdu_timeout;
#endif
#ifdef CONFIG_VW_REFINE
	u8 vw_cnt;
#endif
	u16 err;
};

struct txsc_entry {
	u8 txsc_is_used;
	u32 use_cnt;
	u8 txsc_ethdr[ETH_HLEN];

	/* wlhdr --- */
	#ifdef USE_ONE_WLHDR
	u8 *txsc_wlhdr[WMM_NUM * WLHDR_NUM];
	struct rtw_pkt_buf_list txsc_pkt_list0[WMM_NUM * WLHDR_NUM];
	#else
	u8 txsc_wlhdr[CORE_TXSC_WLHDR_SIZE];
	#endif
	u8 txsc_wlhdr_len;
	u32 ampdu_wlhdr_len;
	u32 amsdu_wlhdr_len;
	u8 is_amsdu_pkt_list0;
	/* wlhdr --- */

	struct rtw_t_meta_data txsc_mdata;
	u32	txsc_frag_len;/* for pkt frag check */
	u8 txsc_phl_id;/* CONFIG_PHL_TXSC */
#ifdef CONFIG_TXSC_AMSDU
	bool txsc_amsdu;
#endif
	u32 txsc_cache_hit;
	unsigned long last_hit_time;
	u32 last_hit_cnt;
	u32 cache_hit_average;
#ifdef CONFIG_RTW_A4_STA
	u32 txsc_a4_hit;
#endif
};

#ifdef CONFIG_CORE_TXSC
void _print_txreq_mdata(struct rtw_t_meta_data *mdata, const char *func);
static inline void print_txreq_mdata(struct rtw_t_meta_data *mdata, const char *func)
{
	extern u8 DBG_PRINT_MDATA_ONCE;
	if (DBG_PRINT_MDATA_ONCE == 1) {
		_print_txreq_mdata(mdata, func);
		DBG_PRINT_MDATA_ONCE = 0;
	}
}

#endif
void _print_txreq_pklist(struct xmit_frame *pxframe, struct rtw_xmit_req *ptxsc_txreq, u8 *data, const char *func);
static inline void print_txreq_pklist(struct xmit_frame *pxframe, struct rtw_xmit_req *ptxreq, u8 *data, const char *func)
{
	extern u8 DBG_PRINT_TXREQ_ONCE;

	if (pxframe == NULL && ptxreq == NULL)
		return;

	if(DBG_PRINT_TXREQ_ONCE == 1) {
		DBG_PRINT_TXREQ_ONCE = 0;
		_print_txreq_pklist(pxframe, ptxreq, data, func);
	}
}


void txsc_init(_adapter *padapter);
void txsc_clear(_adapter *padapter, u8 free_self);
void txsc_dump(_adapter *padapter, void *m);
void txsc_dump_data(u8 *buf, u16 buf_len, const char *prefix);
u8 txsc_get_sc_cached_entry(_adapter *padapter, struct sk_buff *pskb, struct txsc_pkt_entry *txsc_pkt, struct sta_info *psta);
void txsc_add_sc_cache_entry(_adapter *padapter, struct xmit_frame *pxframe, struct txsc_pkt_entry *txsc_pkt);
u8 txsc_apply_sc_cached_entry(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt, u8 vw_pkt);
#ifdef CONFIG_PCI_HCI
void txsc_fill_txreq_phyaddr(_adapter *padapter, struct rtw_pkt_buf_list *pkt_list);
void txsc_recycle_txreq_phyaddr(_adapter *padapter, struct rtw_xmit_req *txreq);
#endif
void txsc_free_txreq(_adapter *padapter, struct rtw_xmit_req *txreq);
void txsc_debug_sc_entry(_adapter *padapter, struct xmit_frame *pxframe, struct txsc_pkt_entry *txsc_pkt);
void txsc_issue_addbareq_cmd(_adapter *padapter, u8 priority, struct sta_info *psta, u8 issue_when_busy);
#ifdef CONFIG_TXSC_AMSDU
void txsc_amsdu_process_check(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt);
void txsc_amsdu_queue_free(_adapter *padapter, struct sta_info *psta, u8 reset);
void txsc_amsdu_queue_init(_adapter *padapter, struct sta_info *psta);
u8 txsc_amsdu_enqueue(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt, u8 *status);
s32 txsc_amsdu_timeout_tx(struct sta_info *psta, u8 ac);
void txsc_amsdu_sta_init(_adapter *padapter, struct sta_info* psta);
void txsc_amsdu_clear(_adapter *padapter);
void txsc_amsdu_dump(_adapter *padapter, void *m);
void txsc_amsdu_reset(_adapter *padapter);
#ifdef CONFIG_VW_REFINE
u8 txsc_amsdu_dequeue_swq(_adapter *padapter, struct txsc_pkt_entry *txsc_pkt);
#endif
void ieee8023_header_to_rfc1042_txsc(struct sk_buff *skb, bool add_pad);
void ieee8023_header_to_rfc1042_shortcut(_adapter *padapter, struct sk_buff *skb, struct txsc_entry* txsc, struct sta_info *psta, bool add_pad);
#endif /* CONFIG_TXSC_AMSDU */
u8 txsc_update_sec_cam_idx(_adapter *padapter, struct sta_info *psta, u8 keyid);
void txsc_acquire_entry_use_cnt(_adapter *padapter, struct txsc_pkt_entry *txsc);
void txsc_release_entry_use_cnt(_adapter *padapter, struct txsc_pkt_entry *txsc);
void txsc_recycle_entry_use_cnt(_adapter *padapter, struct rtw_xmit_req *txreq);

#ifdef CONFIG_PHL_TXSC
void _rtw_clear_phl_txsc(struct rtw_phl_stainfo_t *phl_sta);
#endif
void core_txsc_clean_entry(_adapter *adapter, struct sta_info *sta);

#ifdef CONFIG_TXSC_ONE_LOCK
#define _txsc_spinlock_bh(lock) {}
#define _txsc_spinunlock_bh(lock) {}
#define _txsc_one_spinlock_bh(lock1, lock2) \
	do {\
		if (lock1)\
			_rtw_spinlock_bh(lock1);\
	} while (0)
#define _txsc_one_spinunlock_bh(lock1, lock2)\
	do {\
		if (lock1)\
			_rtw_spinunlock_bh(lock1);\
	} while (0)
#else
#define _txsc_spinlock_bh(lock) _rtw_spinlock_bh(lock)
#define _txsc_spinunlock_bh(lock) _rtw_spinunlock_bh(lock)
#define _txsc_one_spinlock_bh(lock1, lock2)\
	do {\
		if (lock2)\
			_rtw_spinlock_bh(lock2);\
	} while (0)
#define _txsc_one_spinunlock_bh(lock1, lock2)\
	do {\
		if (lock2)\
			_rtw_spinunlock_bh(lock2);\
	} while (0)
#endif

#endif /* CONFIG_CORE_TXSC */
#endif /* _RTW_XMIT_SHORTCUT_H_ */

