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

#include "gpio_hv.h"

u32 hv_get_gpio_val(struct mac_ax_adapter *adapter, u8 gpio, u8 *val)
{
	struct mac_ax_intf_ops *ops = adapter->ops->intf_ops;
	u32 reg;

	if (gpio <= 7) {
		reg = R_AX_GPIO_PIN_CTRL;
	} else if (gpio >= 8 && gpio <= 15) {
		reg = R_AX_GPIO_EXT_CTRL + 1;
		gpio = gpio - 8;
#if MAC_AX_8852C_SUPPORT || MAC_AX_8192XB_SUPPORT
	} else if (gpio >= 16 && gpio <= 18 &&
		   (is_chip_id(adapter, MAC_AX_CHIP_ID_8852C) ||
		    is_chip_id(adapter, MAC_AX_CHIP_ID_8192XB))) {
		reg = R_AX_GPIO_16_TO_18_EXT_CTRL;
		gpio = gpio - 16;
#endif
	} else {
		PLTFM_MSG_ERR("%s: Wrong GPIO num: %d", __func__, gpio);
		return MACNOITEM;
	}

	*val = !!(MAC_REG_R8(reg) & BIT(gpio));

	return MACSUCCESS;
}
