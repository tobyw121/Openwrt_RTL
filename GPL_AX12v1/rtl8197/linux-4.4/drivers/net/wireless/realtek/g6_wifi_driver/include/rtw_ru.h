/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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

#ifndef __RTW_RU_H_
#define __RTW_RU_H_

#ifdef CONFIG_WFA_OFDMA_Logo_Test

#define dynamic_grp_dlul			BIT(0)					/* 1 */
#define disable_dynamic_grp_dl		BIT(5)					/* 32 */
#define disable_dynamic_grp_ul		BIT(4)					/* 16 */
#define dynamic_grp_dl_only			BIT(0) | BIT(4)			/* 17 */
#define dynamic_grp_ul_only			BIT(0) | BIT(5)			/* 33 */

#define ul_fix_mode					BIT(1)					/* 2 */
#define dl_fix_mode					BIT(3)					/* 8 */

#define ul_fix_mode_by_assoc		BIT(2)					/* 4 */
#define dl_fix_mode_by_assoc		BIT(6)					/* 64 */

void _rtw_set_ru_common(_adapter *adapter, char *para, u32 para_num);
void _rtw_set_dl_grp(_adapter *adapter, char *para, u32 para_num);
void _rtw_set_dl_fix_grp(_adapter *adapter, char *para, u32 para_num);
void _rtw_set_ul_grp(_adapter *adapter, char *para, u32 para_num);
void _rtw_set_ul_fix_grp(_adapter *adapter, char *para, u32 para_num);
void _rtw_set_ulmacid_cfg(_adapter *adapter, char *para, u32 para_num);

void rtw_del_dl_ofdma_grp(_adapter *padapter, int tble_num);
void rtw_del_ul_ofdma_grp(_adapter *padapter);

void rtw_clean_ofdma_grp(_adapter *adapter);
void rtw_stop_ofdma_grp(_adapter *adapter);

void rtw_add_dl_ru_ofdma_grp(_adapter *adapter);
void rtw_add_dl_ru_ofdma_grp_2(_adapter *adapter);

void rtw_add_ul_ru_ofdma_grp(_adapter *adapter);
void rtw_add_ul_ru_ofdma_grp_2(_adapter *adapter);
void rtw_add_ul_ru_ofdma_grp_3(_adapter *adapter);

#if 0
void rtw_ofdma_grp_bw160_setting(_adapter *padapter);
void rtw_ofdma_grp_bw80_setting(_adapter *padapter);
void rtw_ofdma_grp_bw40_setting(_adapter *padapter);
void rtw_ofdma_grp_bw20_setting(_adapter *padapter);
#endif

void rtw_phl_check_ru_group(_adapter *padapter);
void rtw_establish_dl_grp(_adapter *padapter);
void rtw_establish_ul_grp(_adapter *padapter);

void rtw_set_ru_ulmacid_cfg(_adapter *adapter, u8 en);
void rtw_set_ru_dlmacid_cfg(_adapter *adapter, struct sta_info *psta);

void rtw_core_mac_set_ru_fwc2h_en(_adapter *adapter, u16 ru_fwc2h_en, u16 intvl);

void rtw_init_ofdma_timer(_adapter *adapter);

void rtw_ru_set_ch_bw(_adapter *adapter);
void rtw_ru_set_pwrtbl(_adapter *adapter);

void rtw_fini_ofdma_timer(_adapter *adapter);
#endif /* CONFIG_WFA_OFDMA_Logo_Test */

#endif /* __RTW_RU_H_ */
