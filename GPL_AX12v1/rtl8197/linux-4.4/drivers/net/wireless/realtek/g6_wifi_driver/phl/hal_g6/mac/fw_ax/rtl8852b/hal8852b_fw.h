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

#if MAC_AX_8852B_SUPPORT

#ifdef PHL_FEATURE_NIC
#ifdef MAC_FW_8852B_U2
extern u8 array_8852b_u2_nicce[270528];
extern u32 array_length_8852b_u2_nicce;
#endif /*MAC_FW_8852B_U2*/
#ifdef MAC_FW_8852B_U3
extern u8 array_8852b_u3_nicce[270584];
extern u32 array_length_8852b_u3_nicce;
#endif /*MAC_FW_8852B_U3*/
#ifdef MAC_FW_8852B_U2
extern u8 array_8852b_u2_nic[270528];
extern u32 array_length_8852b_u2_nic;
#endif /*MAC_FW_8852B_U2*/
#ifdef MAC_FW_8852B_U2
extern u8 array_8852b_u2_nic_ple[289496];
extern u32 array_length_8852b_u2_nic_ple;
#endif /*MAC_FW_8852B_U2*/
#ifdef MAC_FW_8852B_U3
extern u8 array_8852b_u3_nic[270584];
extern u32 array_length_8852b_u3_nic;
#endif /*MAC_FW_8852B_U3*/
#ifdef MAC_FW_8852B_U3
extern u8 array_8852b_u3_nic_ple[289552];
extern u32 array_length_8852b_u3_nic_ple;
#endif /*MAC_FW_8852B_U3*/
#ifdef CONFIG_WOWLAN
#ifdef MAC_FW_8852B_U2
extern u8 array_8852b_u2_wowlan[248072];
extern u32 array_length_8852b_u2_wowlan;
#endif /*MAC_FW_8852B_U2*/
#endif /*CONFIG_WOWLAN*/
#ifdef CONFIG_WOWLAN
#ifdef MAC_FW_8852B_U3
extern u8 array_8852b_u3_wowlan[248128];
extern u32 array_length_8852b_u3_wowlan;
#endif /*MAC_FW_8852B_U3*/
#endif /*CONFIG_WOWLAN*/
#endif /*PHL_FEATURE_NIC*/

#endif /*MAC_AX_8852B_SUPPORT*/
