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

#if MAC_AX_8192XB_SUPPORT || MAC_AX_8832BR_SUPPORT

#ifdef PHL_FEATURE_AP
#ifdef MAC_FW_8192XB_U1
extern u8 array_8192xb_u1_ap[384216];
extern u32 array_length_8192xb_u1_ap;
#endif /*MAC_FW_8192XB_U1*/
#endif /*PHL_FEATURE_AP*/
#ifdef PHL_FEATURE_AP
#ifdef MAC_FW_8192XB_U2
extern u8 array_8192xb_u2_ap[384288];
extern u32 array_length_8192xb_u2_ap;
#endif /*MAC_FW_8192XB_U2*/
#endif /*PHL_FEATURE_AP*/
#ifdef PHL_FEATURE_NIC
#ifdef MAC_FW_8192XB_U1
extern u8 array_8192xb_u1_nic[228312];
extern u32 array_length_8192xb_u1_nic;
#endif /*MAC_FW_8192XB_U1*/
#ifdef MAC_FW_8192XB_U2
extern u8 array_8192xb_u2_nic[228384];
extern u32 array_length_8192xb_u2_nic;
#endif /*MAC_FW_8192XB_U2*/
#ifdef CONFIG_WOWLAN
#ifdef MAC_FW_8192XB_U1
extern u8 array_8192xb_u1_wowlan[184592];
extern u32 array_length_8192xb_u1_wowlan;
#endif /*MAC_FW_8192XB_U1*/
#endif /*CONFIG_WOWLAN*/
#ifdef CONFIG_WOWLAN
#ifdef MAC_FW_8192XB_U2
extern u8 array_8192xb_u2_wowlan[184664];
extern u32 array_length_8192xb_u2_wowlan;
#endif /*MAC_FW_8192XB_U2*/
#endif /*CONFIG_WOWLAN*/
#endif /*PHL_FEATURE_NIC*/

#endif /*MAC_AX_8192XB_SUPPORT || MAC_AX_8832BR_SUPPORT*/
