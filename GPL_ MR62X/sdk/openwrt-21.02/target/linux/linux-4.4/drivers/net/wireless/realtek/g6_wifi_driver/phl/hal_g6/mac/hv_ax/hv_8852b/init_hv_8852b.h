/** @file */
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

#ifndef _MAC_AX_INIT_HV_8852B_H_
#define _MAC_AX_INIT_HV_8852B_H_

#include "../../hv_type.h"
#if MAC_AX_8852B_SUPPORT

/**
 * @brief get_hv_8852b_ops
 *
 * @param void
 * @return Please Place Description here.
 * @retval  hv_ax_ops
 */
struct hv_ax_ops *get_hv_8852b_ops(void);

#endif /* #if MAC_AX_8852B_SUPPORT */
#endif
