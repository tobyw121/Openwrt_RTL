/******************************************************************************
 *
 * Copyright(c) 2012 - 2020 Realtek Corporation.
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

#if MAC_AX_8851B_SUPPORT

#ifdef PHL_FEATURE_NIC
#ifdef MAC_FW_8851B_U1
extern u8 array_8851b_u1_nic[269472];
extern u32 array_length_8851b_u1_nic;
#endif /*MAC_FW_8851B_U1*/
#ifdef MAC_FW_8851B_U1
extern u8 array_8851b_u1_nic_ple[287384];
extern u32 array_length_8851b_u1_nic_ple;
#endif /*MAC_FW_8851B_U1*/
#ifdef CONFIG_WOWLAN
#ifdef MAC_FW_8851B_U1
extern u8 array_8851b_u1_wowlan[249560];
extern u32 array_length_8851b_u1_wowlan;
#endif /*MAC_FW_8851B_U1*/
#endif /*CONFIG_WOWLAN*/
#endif /*PHL_FEATURE_NIC*/

#endif /*MAC_AX_8851B_SUPPORT*/
