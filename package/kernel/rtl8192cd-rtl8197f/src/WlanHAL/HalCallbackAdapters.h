/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef RTL8192CD_HAL_CALLBACK_ADAPTERS_H
#define RTL8192CD_HAL_CALLBACK_ADAPTERS_H

/* Include after HalPrecomp.h. Older SDK tables assigned one-argument void
 * dummies to unrelated callback types. Preserve intentional no-op operations
 * with their real ABI and give status-returning operations a defined result.
 */
static inline enum rt_status hal_init_pon_88xx(HAL_PADAPTER adapter, u4Byte clock)
{
	(void)clock; /* HALMAC owns the reference-clock setup for these parts. */
	return InitPON88XX(adapter);
}

static inline BOOLEAN hal_fill_shortcut_88xx(HAL_PADAPTER adapter, u4Byte queue,
		PVOID data, PVOID desc, u4Byte direction, u4Byte use_hw)
{
	return FillShortCutTxHwCtrl88XX(adapter, queue, data, desc, direction,
				      use_hw != 0);
}

static inline enum rt_status hal_llt_in_halmac(HAL_PADAPTER adapter)
{
	(void)adapter; /* The 8814B LLT is initialized by HALMAC. */
	return RT_STATUS_SUCCESS;
}

static inline enum rt_status hal_mbidcam_in_addrcam(HAL_PADAPTER adapter,
		u1Byte index, u1Byte valid)
{
	(void)adapter;
	(void)index;
	(void)valid; /* 8814B uses SetADDRCAMHandler instead of legacy MBIDCAM. */
	return RT_STATUS_SUCCESS;
}

static inline enum rt_status hal_stop_mbidcam_in_addrcam(HAL_PADAPTER adapter,
		u1Byte index)
{
	(void)adapter;
	(void)index;
	return RT_STATUS_SUCCESS;
}

static inline void hal_shortcut_in_ie(HAL_PADAPTER adapter, u4Byte queue, PVOID data)
{
	(void)adapter;
	(void)queue;
	(void)data; /* 8814B uses FillHwShortCutIEHandler for its IE descriptors. */
}

static inline enum rt_status hal_set_macid_pause_88xx(HAL_PADAPTER adapter,
		BOOLEAN sleep, u4Byte aid)
{
	SetMACIDPauseDriverInfo88XX(adapter, sleep, aid);
	return RT_STATUS_SUCCESS;
}

static inline BOOLEAN hal_reserved_page_unavailable(HAL_PADAPTER adapter,
		pu4Byte beacon, u4Byte length, u1Byte redownload)
{
	(void)adapter;
	(void)beacon;
	(void)length;
	(void)redownload;
	return FALSE; /* No implementation was supplied for the RTL8812F table. */
}

static inline void hal_no_atm_report(HAL_PADAPTER adapter, u1Byte enable,
		u1Byte ranges, u1Byte period, pu1Byte ids, pu1Byte lengths, u1Byte count)
{
	(void)adapter;
	(void)enable;
	(void)ranges;
	(void)period;
	(void)ids;
	(void)lengths;
	(void)count;
}

#endif
