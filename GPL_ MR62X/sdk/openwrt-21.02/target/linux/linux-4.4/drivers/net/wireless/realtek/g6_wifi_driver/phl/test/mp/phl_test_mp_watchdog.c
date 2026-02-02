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
#define _PHL_TEST_MP_WATCHDOG_C_
#include "../../phl_headers.h"
#include "phl_test_mp_def.h"
#include "phl_test_mp_api.h"
#include "phl_test_mp_watchdog.h"
#include "../../hal_g6/test/mp/hal_test_mp_api.h"

#ifdef CONFIG_PHL_TEST_MP

void _phl_mp_timer_cb(void *context)
{
	struct mp_context *mp_ctx = (struct mp_context *)context;
	struct phl_info_t *phl_info = (struct phl_info_t *)mp_ctx->phl;
	struct phl_mp_watchdog *wdog = &(mp_ctx->mp_wdog);

	PHL_INFO("%s\n", __func__);

	if (mp_ctx->status != MP_STATUS_RUN_CMD)
		phl_watchdog_cmd_hdl(mp_ctx->phl, RTW_PHL_STATUS_SUCCESS);

	_os_set_timer(phl_to_drvpriv(phl_info),
		&wdog->wdog_timer,
		wdog->period);
}


void rtw_phl_mp_watchdog_init(struct mp_context *mp_ctx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)mp_ctx->phl;
	struct phl_mp_watchdog *wdog = &(mp_ctx->mp_wdog);

	wdog->period = MP_WATCHDOG_PERIOD;

	_os_init_timer(phl_to_drvpriv(phl_info),
			&wdog->wdog_timer,
			_phl_mp_timer_cb,
	               mp_ctx, "phl_test_mp_watchdog_timer");
}

void rtw_phl_mp_watchdog_deinit(struct mp_context *mp_ctx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)mp_ctx->phl;
	struct phl_mp_watchdog *wdog = &(mp_ctx->mp_wdog);

	_os_cancel_timer(phl_to_drvpriv(phl_info), &wdog->wdog_timer);
	_os_release_timer(phl_to_drvpriv(phl_info), &wdog->wdog_timer);
}

void rtw_phl_mp_watchdog_start(struct mp_context *mp_ctx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)mp_ctx->phl;
	struct phl_mp_watchdog *wdog = &(mp_ctx->mp_wdog);

	PHL_INFO("%s\n", __func__);

	_os_set_timer(phl_to_drvpriv(phl_info),
	              &wdog->wdog_timer,
	              wdog->period);
}

void rtw_phl_mp_watchdog_stop(struct mp_context *mp_ctx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)mp_ctx->phl;
	struct phl_mp_watchdog *wdog = &(mp_ctx->mp_wdog);

	PHL_INFO("%s\n", __func__);

	_os_cancel_timer(phl_to_drvpriv(phl_info), &wdog->wdog_timer);
}

#endif /* CONFIG_PHL_TEST_MP */
