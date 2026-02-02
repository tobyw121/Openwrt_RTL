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

#ifndef _RTW_A4_H_
#define _RTW_A4_H_

#ifdef CONFIG_RTW_A4_STA

#define A4_STA_AGEING_TIME	300

#define MAX_A4_TBL_NUM		32

#define A4_STA_HASH_BITS	3
#define A4_STA_HASH_SIZE	(1 << A4_STA_HASH_BITS)

#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */
#endif /* ETH_P_PAE */
#ifndef ETH_P_EAPOL
#define ETH_P_EAPOL ETH_P_PAE
#endif /* ETH_P_EAPOL */

#if defined(CONFIG_A4_LOOPBACK) || defined(WFO_STRUCT_ALIGNED)
#define MAX_A4_LOOPBACK_ENTRY_NUM 32
#define A4_LOOP_HASH_BITS	3
#define A4_LOOP_HASH_SIZE	(1 << A4_LOOP_HASH_BITS)
struct rtw_a4_loopback_entry {
	struct hlist_node	hnode;
	systime				stime;
	unsigned char		mac[6];
};
#endif

#ifdef CONFIG_DYN_ALLOC_A4_TBL
#define machash_spinlock_bh(idx)		_rtw_spinlock_bh(&adapter->machash_lock[idx])
#define machash_spinunlock_bh(idx)		_rtw_spinunlock_bh(&adapter->machash_lock[idx])
#else
#define machash_spinlock_bh(idx)		do { } while (0)
#define machash_spinunlock_bh(idx)		do { } while (0)
#endif

struct rtw_a4_db_entry {
	struct rtw_a4_db_entry	*next_hash;
	struct rtw_a4_db_entry	**pprev_hash;
	struct sta_info		*psta;
	unsigned char		mac[6];
	unsigned long		ageing_timer;
	/* A4_CNT */
	unsigned long long tx_bytes;
	unsigned int tx_count;
	unsigned long long rx_bytes;
	unsigned int rx_count;
	unsigned int link_time;/* unit 1sec*/
	unsigned int m2u_ignore_cnt;
};

struct rtw_a4_tbl_entry {
	int	used;
	struct rtw_a4_db_entry	entry;
};

s32 core_a4_data_validate_hdr(_adapter *adapter,
	union recv_frame *rframe, struct sta_info **ppsta);
s32 core_a4_rx_msdu_act_check(union recv_frame *prframe);
int core_a4_rx_amsdu_act_check(_adapter *adapter, const u8 *da, const u8 *sa);
bool core_a4_check_tx(_adapter *adapter, struct sk_buff **pskb);
struct sta_info *core_a4_get_fwd_sta(_adapter *adapter, u8 *mac);

struct core_a4_gptr_table {
	rtw_rhashtable rhead;
};

void dump_wgptr(void *sel, _adapter *adapter);
bool core_rx_a4_gptr_check(_adapter *adapter, const u8 *src);
void core_tx_a4_gptr_update(_adapter *adapter, const u8 *src);
void core_a4_gptr_expire(_adapter *adapter);
int core_a4_gptr_tbl_init(_adapter *adapter);
void core_a4_gptr_flush(_adapter *adapter);
void core_a4_gptr_tbl_unregister(_adapter *adapter);
void core_a4_sta_cleanup(_adapter *adapter, struct sta_info *psta);/* A4_AMSDU */
#if defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
struct rtw_a4_db_entry *core_a4_get_source_db_entry(_adapter *adapter, u8 *mac, int *del);
#else
struct rtw_a4_db_entry *core_a4_get_source_db_entry(_adapter *adapter, u8 *mac);
#endif
/* A4_CNT */
void core_a4_count_stats(_adapter *adapter, u8* mac, u8 mode, u32 size);
void core_a4_sta_expire(_adapter *adapter);
void core_a4_init(_adapter *padapter);

#ifdef CONFIG_A4_LOOPBACK
int rtw_check_a4_loop_entry(_adapter *adapter, unsigned char *mac);
void rtw_update_a4_loop_entry(_adapter *adapter, unsigned char *mac);
#endif
void a4_del_source_db_entry(_adapter *adapter, u8 *mac);
void core_a4_update_m2u_ignore_cnt(_adapter *adapter, u8 *mac);

#endif /* CONFIG_RTW_A4_STA */

#endif /* _RTW_A4_H_ */

