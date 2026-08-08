/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#include "trx_desc_hv_8852d.h"
#include "../../txdesc.h"
#if MAC_AX_8852D_SUPPORT

u32 hv_tx_post_agg_8852d(struct mac_ax_adapter *adapter,
			 struct hv_aggregator_t *agg)
{
	u32 dw1 = ((struct wd_body *)agg->pkt)->dword1;

	((struct wd_body *)agg->pkt)->dword1 =
		SET_CLR_WORD(dw1, agg->agg_num, AX_TXD_DMA_TXAGG_NUM_V1);

	return 0;
}

#endif /* #if MAC_AX_8852D_SUPPORT */
