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
#define _PHL_MIB_C_
#include "phl_headers.h"

#ifdef CONFIG_LIFETIME_FEATURE
enum rtw_phl_status
rtw_phl_set_lifetime(void *phl, u8 enable, u16 timeout)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);

	hsts = rtw_hal_set_lifetime(phl_info->hal, enable, timeout);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
rtw_phl_get_lifetime(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);

	hsts = rtw_hal_get_lifetime(phl_info->hal);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}
#endif

#ifdef POWER_PERCENT_ADJUSTMENT
enum rtw_phl_status
rtw_phl_set_ref_power(void *phl, u8 band, int txagc_ref)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
	int diff_level = txagc_ref;
	s8 ofst_bw = 0, ofst_mode = 0;

	if (diff_level > 14)
		diff_level = 14;
	else if (diff_level < -16)
		diff_level = -16;
	else
		diff_level = txagc_ref;
	ofst_mode = (diff_level / 2) + (diff_level & 1);
	ofst_bw = (diff_level / 2);

	hsts = _phl_set_power_offset(phl, band, ofst_mode, ofst_bw);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}
#endif /* POWER_PERCENT_ADJUSTMENT */


