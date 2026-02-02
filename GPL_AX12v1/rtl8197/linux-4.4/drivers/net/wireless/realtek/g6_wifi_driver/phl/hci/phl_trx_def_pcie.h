/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#ifndef _PHL_TRX_DEF_PCIE_H_
#define _PHL_TRX_DEF_PCIE_H_

#ifdef RTW_WKARD_WD0_SER
#define MAX_WD_PAGE_NUM (256*2)
#else
#define MAX_WD_PAGE_NUM 256
#endif
#define MAX_RX_BD_NUM 256
#define MAX_RX_BUF_NUM (MAX_RX_BD_NUM*3) //512

#define WP_TID_INDIC_RESERVED_BIT BIT(14)
#define WP_RESERVED_SEQ 0xFFF
#define WP_MAX_SEQ_NUMBER WP_RESERVED_SEQ
#define WP_USED_SEQ 4800
#ifdef CONFIG_TRUNCATE_MODE
#define ADDR_INFO_SIZE 8
#else
#define ADDR_INFO_SIZE 12
#endif
#define WD_NUM_IN_ONE_UPDATE_TXBD 32

struct tx_base_desc {
	u8 *vir_addr;
	u32 phy_addr_l;
	u32 phy_addr_h;
	u32 buf_len;
	void *os_rsvd[1];
	u8 cache;
	u16 host_idx;
	u16 hw_idx;
	_os_atomic avail_num;
	_os_lock txbd_lock;
};

struct rtw_wd_page_list {
	struct rtw_wd_page *wd_page[WD_NUM_IN_ONE_UPDATE_TXBD];
	u16 handled_wd_num;
};

struct rtw_wd_page {
	_os_list list;
	u8 *vir_addr;
	u32 phy_addr_l;
	u32 phy_addr_h;
	u32 buf_len;
	void *os_rsvd[1];
	u8 ls;
	u8 cache;
	u16 wp_seq;
	u16 mac_id;
	u8 tid;
	u16 host_idx;
};

struct rtw_rx_buf {
	/* struct rtw_rx_buf_base as header */
	_os_list list;
	void *os_priv;
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	u8 *frame;
	u32 frame_len;
	u8 *hdr;
	u32 hdr_len;
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

	u8 *vir_addr;
	u32 phy_addr_l;
	u32 phy_addr_h;
	u32 buf_len;
	#ifdef PHL_INV_CACHE_AT_RECYCLE
	u32 dma_len;
	#endif /* PHL_INV_CACHE_AT_RECYCLE */
	u8 cache;
	u8 dynamic;
#ifdef CONFIG_DYNAMIC_RX_BUF
	bool reuse;
#endif
};

struct rx_base_desc {
	u8 *vir_addr;
	u32 phy_addr_l;
	u32 phy_addr_h;
	u32 buf_len;
	void *os_rsvd[1];
	u8 cache;
	u16 host_idx;
	u16 avail_num;
};

static inline void phl_sync_rx_buf(void *d, struct rtw_rx_buf *rx_buf, u32 sz)
{
	#ifdef CONFIG_RTW_VM_CACHE_HANDLING
	_os_vm_cache_inv(d, rx_buf->vir_addr, sz);
	#else /* CONFIG_RTW_VM_CACHE_HANDLING */
	_os_cache_inv(d, &rx_buf->phy_addr_l, &rx_buf->phy_addr_h, sz,
		      PCI_DMA_FROMDEVICE);
	#endif /*CONFIG_RTW_VM_CACHE_HANDLING */
}

#endif	/* _PHL_TRX_DEF_PCIE_H_ */
