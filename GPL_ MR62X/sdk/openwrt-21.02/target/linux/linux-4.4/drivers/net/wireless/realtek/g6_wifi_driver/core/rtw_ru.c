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
#define _RTW_RU_C_

#include <drv_types.h>
#include "_hal_rate.h"
#include "../phl/phl_headers.h"

#ifdef CONFIG_WFA_OFDMA_Logo_Test

#define MAX_RU_PARA_STR_LEN 200

#define RU26_POSITION           72      // 36*2, RU26
#define RU52_POSITION           106     // 53*2, RU52
#define RU106_POSITION          122     // 61*2, RU106
#define RU242_POSITION          130     // 65*2, RU242
#define RU484_POSITION          134     // 67*2, RU484

#define PSD_CMP_RU52            12      // u(x,2)
#define PSD_CMP_RU106           24      // u(x,2)
#define PSD_CMP_RU242           39      // u(x,2)
#define PSD_CMP_RU484           51      // u(x,2)
#define PSD_CMP_RU996           63      // u(x,2)

char *get_next_ru_para_str(char *para)
{
	return (para+MAX_RU_PARA_STR_LEN);
}

void dump_ru_common(_adapter *adapter){

	void *phl = adapter->dvobj->phl;

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	DBGP("grp_once:%d\n", ru_ctrl->GRP_CALLBACK_ONCE);
	DBGP("dl_grp:%d\n", ru_ctrl->GRP_DL_ON);
	DBGP("ul_grp:%d\n", ru_ctrl->GRP_UL_ON);
	DBGP("fill_dl_tbl:%d\n", ru_ctrl->GRP_FORCE_FILL_DL_TBL);
	DBGP("fill_ul_tbl:%d\n", ru_ctrl->GRP_FORCE_FILL_UL_TBL);
	DBGP("TX_TP_THRD:%d\n", ru_ctrl->TX_TP_THRD);
	DBGP("RX_TP_THRD:%d\n", ru_ctrl->RX_TP_THRD);
	DBGP("tbl_exist:%d\n", ru_ctrl->tbl_exist);
	DBGP("tx_phase:%d\n", ru_ctrl->tx_phase);
	DBGP("rotate:%d\n", ru_ctrl->rotate);
	DBGP("ofdma_WFA_mode:%d\n", ru_ctrl->ofdma_WFA_mode);
	DBGP("auto_config:%d\n", ru_ctrl->auto_config);
	DBGP("rsp_type:%d (0: BA-BAR; 2: MUBAR)\n", ru_ctrl->rsp_type);
	DBGP("prot_type:%d (0: no prot; 2: RTS; 4: MURTS)\n", ru_ctrl->prot_type);
	DBGP("force_sumuru_en:%d\n", ru_ctrl->force_sumuru_en);
	DBGP("forceru:%d\n", ru_ctrl->forceru);
	DBGP("forcesu:%d\n", ru_ctrl->forcesu);
	DBGP("ulmacid_cfg:%d\n", ru_ctrl->ulmacid_cfg);
	DBGP("ulmacid_cfg_fix:%d\n", ru_ctrl->ulmacid_cfg_fix);
	DBGP("ul_psd:%d (0: hard code; 1: decided by rssi_m)\n", ru_ctrl->ul_psd);
	DBGP("psd_ofst1:%d\n", ru_ctrl->psd_ofst1);
	DBGP("psd_ofst2:%d\n", ru_ctrl->psd_ofst2);
	DBGP("ru_rate_idx1:%d\n", ru_ctrl->ru_rate_idx1);
	DBGP("ru_rate_idx2:%d\n", ru_ctrl->ru_rate_idx2);
	DBGP("ru_rssi_level1:%d\n", ru_ctrl->ru_rssi_level1);
	DBGP("ru_rssi_level2:%d\n", ru_ctrl->ru_rssi_level2);
	DBGP("ul_crc32:%d (0: disable crc32 pkt; 1: enable crc32 pkt)\n", ru_ctrl->ul_crc32);
	DBGP("netif_drop_thd:%d\n", ru_ctrl->netif_drop_thd);
	DBGP("phl_wd_hi_thd:%d\n", ru_ctrl->phl_wd_hi_thd);
	DBGP("phl_wd_hold_cnt_thd:%d\n", ru_ctrl->phl_wd_hold_cnt_thd);
	DBGP("phl_wd_quota[0]:%d\n", ru_ctrl->phl_wd_quota[0]);
	DBGP("phl_wd_quota[1]:%d\n", ru_ctrl->phl_wd_quota[1]);
	DBGP("phl_wd_quota[2]:%d\n", ru_ctrl->phl_wd_quota[2]);
	DBGP("phl_wd_quota[3]:%d\n", ru_ctrl->phl_wd_quota[3]);
	DBGP("phl_wd_quota[4]:%d\n", ru_ctrl->phl_wd_quota[4]);
	DBGP("phl_wd_quota[5]:%d\n", ru_ctrl->phl_wd_quota[5]);
	DBGP("phl_wd_quota[6]:%d\n", ru_ctrl->phl_wd_quota[6]);
	DBGP("phl_wd_quota[7]:%d\n", ru_ctrl->phl_wd_quota[7]);
	DBGP("phl_wd_quota[8]:%d\n", ru_ctrl->phl_wd_quota[8]);
	DBGP("mu_edca:%d\n", ru_ctrl->mu_edca);

	return;
}

void dump_dl_ru_grp(_adapter *adapter){
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;

	DBGP("tx_pwr:0x%x\n", dl_ru_grp->tx_pwr);
	DBGP("ppdu_bw:%d\n", dl_ru_grp->ppdu_bw);
	DBGP("txpwr_ofld_en:0x%x\n", dl_ru_grp->txpwr_ofld_en);
	DBGP("pwrlim_dis:%d\n", dl_ru_grp->pwrlim_dis);

	DBGP("tf_rate:%d\n", dl_ru_grp->tf.tf_rate);
	DBGP("tb_ppdu_bw:%d\n", dl_ru_grp->tf.tb_ppdu_bw);
	DBGP("gi_ltf:%d\n", dl_ru_grp->tf.gi_ltf);
	DBGP("fix_ba:%d\n", dl_ru_grp->tf.fix_ba);
	DBGP("ru_psd:%d\n", dl_ru_grp->tf.ru_psd);
	DBGP("rf_gain_fix:%d\n", dl_ru_grp->tf.rf_gain_fix);
	DBGP("rf_gain_idx:%d\n", dl_ru_grp->tf.rf_gain_idx);
	DBGP("doppler:%d\n", dl_ru_grp->tf.doppler);
	DBGP("stbc:%d\n", dl_ru_grp->tf.stbc);
	DBGP("sta_coding:%d\n", dl_ru_grp->tf.sta_coding);
	DBGP("tb_t_pe_nom:%d\n", dl_ru_grp->tf.tb_t_pe_nom);
	DBGP("pr20_bw_en:%d\n", dl_ru_grp->tf.pr20_bw_en);
	DBGP("rate_dcm:%d\n", dl_ru_grp->tf.rate.dcm);
	DBGP("rate_ss:%d\n", dl_ru_grp->tf.rate.ss);
	DBGP("rate_mcs:%d\n", dl_ru_grp->tf.rate.mcs);

	return;
}

void dump_dl_ru_fix_grp(_adapter *adapter){
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;

	int i;

	DBGP("max_sta_num:%d\n", dl_ru_fix_grp->max_sta_num);
	DBGP("min_sta_num:%d\n", dl_ru_fix_grp->min_sta_num);
	DBGP("rupos_csht_flag:%d\n", dl_ru_fix_grp->rupos_csht_flag);
	DBGP("ru_swp_flg:%d\n", dl_ru_fix_grp->ru_swp_flg);
	DBGP("gi_ltf:%d\n", dl_ru_fix_grp->gi_ltf);
	DBGP("fixru_flag:%d\n", dl_ru_fix_grp->fixru_flag);
	for(i=0; i<8; i++){
		DBGP("macid[%d]:%d\n", i, dl_ru_fix_grp->sta_info[i].macid);
		DBGP("mcs[%d]:%d\n", i, dl_ru_fix_grp->sta_info[i].mcs);
		DBGP("ss[%d]:%d\n", i, dl_ru_fix_grp->sta_info[i].ss);
		DBGP("fix_rate[%d]:%d\n", i, dl_ru_fix_grp->sta_info[i].fix_rate);
		DBGP("coding[%d]:%d\n", i, dl_ru_fix_grp->sta_info[i].coding);
		DBGP("ru_pos[%d]:%d,%d,%d,%d,%d,%d,%d\n", i, dl_ru_fix_grp->sta_info[i].ru_pos[0],
									dl_ru_fix_grp->sta_info[i].ru_pos[1],
									dl_ru_fix_grp->sta_info[i].ru_pos[2],
									dl_ru_fix_grp->sta_info[i].ru_pos[3],
									dl_ru_fix_grp->sta_info[i].ru_pos[4],
									dl_ru_fix_grp->sta_info[i].ru_pos[5],
									dl_ru_fix_grp->sta_info[i].ru_pos[6]);
	}
	return;

}

void dump_ul_ru_grp(_adapter *adapter){
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;

	DBGP("ppdu_bw:%d\n", ul_ru_grp->ppdu_bw);
	DBGP("grp_psd_max:0x%x\n", ul_ru_grp->grp_psd_max);
	DBGP("grp_psd_min:0x%x\n", ul_ru_grp->grp_psd_min);
	DBGP("fix_tf_rate:%d\n", ul_ru_grp->fix_tf_rate);
	DBGP("rf_gain_fix:%d\n", ul_ru_grp->rf_gain_fix);
	DBGP("fix_mode_flags:%d\n", ul_ru_grp->fix_mode_flags);
	DBGP("tf_rate:%d\n", ul_ru_grp->tf_rate);
	DBGP("rf_gain_idx:%d\n", ul_ru_grp->rf_gain_idx);

	return;
}

void dump_ul_ru_fix_grp(_adapter *adapter){
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	int i;

	DBGP("min_sta_num:%d\n", ul_ru_fix_grp->min_sta_num);
	DBGP("max_sta_num:%d\n", ul_ru_fix_grp->max_sta_num);
	DBGP("gi_ltf:%d\n", ul_ru_fix_grp->gi_ltf);
	DBGP("fixru_flag:%d\n", ul_ru_fix_grp->fixru_flag);
	for(i=0; i<8; i++){			// Mark.CS_update
		DBGP("macid[%d]:%d\n", i, ul_ru_fix_grp->sta_info[i].macid);
		DBGP("mcs[%d]:%d\n", i, ul_ru_fix_grp->sta_info[i].mcs);
		DBGP("ss[%d]:%d\n", i, ul_ru_fix_grp->sta_info[i].ss);
		DBGP("fix_rate[%d]:%d\n", i, ul_ru_fix_grp->sta_info[i].fix_rate);
		DBGP("coding[%d]:%d\n", i, ul_ru_fix_grp->sta_info[i].coding);
		DBGP("ru_pos[%d]:%d,%d,%d\n", i, ul_ru_fix_grp->sta_info[i].ru_pos[0],
								ul_ru_fix_grp->sta_info[i].ru_pos[1],
								ul_ru_fix_grp->sta_info[i].ru_pos[2]);
		DBGP("fix_tgt_rssi[%d]:%d\n", i, ul_ru_fix_grp->sta_info[i].fix_tgt_rssi);
		DBGP("tgt_rssi[%d]:%d,%d,%d,%d,%d,%d,%d\n", i, ul_ru_fix_grp->sta_info[i].tgt_rssi[0],
								ul_ru_fix_grp->sta_info[i].tgt_rssi[1],
								ul_ru_fix_grp->sta_info[i].tgt_rssi[2],
								ul_ru_fix_grp->sta_info[i].tgt_rssi[3],
								ul_ru_fix_grp->sta_info[i].tgt_rssi[4],
								ul_ru_fix_grp->sta_info[i].tgt_rssi[5],
								ul_ru_fix_grp->sta_info[i].tgt_rssi[6]);
	}

	return;
}

void dump_ulmacid_para(_adapter *adapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ulmacid_cfg_set_para *ulmacid_cfg = &rugrptable->ulmacid_cfg_set;

	int i;

	for(i=0; i<4; i++){
		DBGP("ulmacid_cfg[%d].macid : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].macid);
		DBGP("ulmacid_cfg[%d].endcmd : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].endcmd);
		DBGP("ulmacid_cfg[%d].ul_su_bw : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_bw);
		DBGP("ulmacid_cfg[%d].ul_su_gi_ltf : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_gi_ltf);
		DBGP("ulmacid_cfg[%d].ul_su_doppler_ctrl : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_doppler_ctrl);
		DBGP("ulmacid_cfg[%d].ul_su_dcm : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_dcm);
		DBGP("ulmacid_cfg[%d].ul_su_ss : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_ss);
		DBGP("ulmacid_cfg[%d].ul_su_mcs : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_mcs);
		DBGP("ulmacid_cfg[%d].ul_su_stbc : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_stbc);
		DBGP("ulmacid_cfg[%d].ul_su_coding : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_coding);
		DBGP("ulmacid_cfg[%d].ul_su_rssi_m : %d\n", i, ulmacid_cfg->ulmacid_cfg[i].ul_su_rssi_m);
	}
}

void _rtw_set_ru_common(_adapter *adapter, char *para, u32 para_num){

	u32 value = 0, value_2 = 0;
	void *phl = adapter->dvobj->phl;

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	if(para_num<=0){
		DBGP("please enter cmd : ru_common,<param>,<value>\n");
		return;
	}

	if(!strcmp(para, "dump")){
		dump_ru_common(adapter);
		return;
	}

	if (!strcmp(para, "grp_once")) {
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("grp_once=%d \n", value);
		ru_ctrl->GRP_CALLBACK_ONCE = value;

		ru_ctrl->GRP_DL_ON = 1;
		ru_ctrl->GRP_UL_ON = 1;
	}else if(!strcmp(para, "dl_grp")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("dl_tbl=%d \n", value);
		ru_ctrl->GRP_DL_ON = value;
	}else if(!strcmp(para, "ul_grp")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ul_grp=%d \n", value);
		ru_ctrl->GRP_UL_ON = value;
	}else if(!strcmp(para, "dl_tbl")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fill_dl_tbl=%d \n", value);
		ru_ctrl->GRP_FORCE_FILL_DL_TBL = value;
	}else if(!strcmp(para, "ul_tbl")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fill_ul_tbl=%d \n", value);
		ru_ctrl->GRP_FORCE_FILL_UL_TBL = value;
	}else if(!strcmp(para, "TX_TP_THRD")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("TX_TP_THRD=%d \n", value);
		ru_ctrl->TX_TP_THRD = value;
	}else if(!strcmp(para, "RX_TP_THRD")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("RX_TP_THRD=%d \n", value);
		ru_ctrl->RX_TP_THRD = value;
	}else if(!strcmp(para, "tbl_exist")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("tbl_exist=%d \n", value);
		ru_ctrl->tbl_exist = value;
	}else if(!strcmp(para, "tx_phase")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("tx_phase=%d \n", value);
		ru_ctrl->tx_phase = value;
	}else if(!strcmp(para, "rotate")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rotate=%d \n", value);
		ru_ctrl->rotate = value;
	}else if(!strcmp(para, "ofdma_WFA_mode")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ofdma_WFA_mode=%d \n", value);
		ru_ctrl->ofdma_WFA_mode = value;
	}else if(!strcmp(para, "auto_config")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("auto_config=%d \n", value);
		ru_ctrl->auto_config = value;
	}else if(!strcmp(para, "rsp_type")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rsp_type=%d \n", value);
		ru_ctrl->rsp_type = value;
	}else if(!strcmp(para, "prot_type")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("prot_type=%d \n", value);
		ru_ctrl->prot_type = value;
	}else if(!strcmp(para, "force_sumuru_en")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("force_sumuru_en=%d \n", value);
		ru_ctrl->force_sumuru_en = value;
	}else if(!strcmp(para, "forceru")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("forceru=%d \n", value);
		ru_ctrl->forceru = value;
	}else if(!strcmp(para, "forcesu")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("forcesu=%d \n", value);
		ru_ctrl->forcesu = value;
	}else if(!strcmp(para, "ulmacid_cfg")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg=%d \n", value);
		ru_ctrl->ulmacid_cfg = value;
	}else if(!strcmp(para, "ulmacid_cfg_fix")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg_fix=%d \n", value);
		ru_ctrl->ulmacid_cfg_fix = value;
	}else if(!strcmp(para, "ul_psd")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ul_psd=%d \n", value);
		ru_ctrl->ul_psd = value;
	}else if(!strcmp(para, "psd_ofst1")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("psd_ofst1=%d \n", value);
		ru_ctrl->psd_ofst1 = value;
	}else if(!strcmp(para, "psd_ofst2")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("psd_ofst2=%d \n", value);
		ru_ctrl->psd_ofst2 = value;
	}else if(!strcmp(para, "ru_rate_idx1")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ru_rate_idx1=%d \n", value);
		ru_ctrl->ru_rate_idx1 = value;
	}else if(!strcmp(para, "ru_rate_idx2")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ru_rate_idx2=%d \n", value);
		ru_ctrl->ru_rate_idx2 = value;
	}else if(!strcmp(para, "ru_rssi_level1")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ru_rssi_level1=%d \n", value);
		ru_ctrl->ru_rssi_level1 = value;
	}else if(!strcmp(para, "ru_rssi_level2")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ru_rssi_level2=%d \n", value);
		ru_ctrl->ru_rssi_level2 = value;
	}else if(!strcmp(para, "ul_crc32")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ul_crc32=%d \n", value);
		ru_ctrl->ul_crc32 = value;
	}else if(!strcmp(para, "phl_wd_hi_thd")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("phl_wd_hi_thd=%d \n", value);
		ru_ctrl->phl_wd_hi_thd = value;
	}else if(!strcmp(para, "netif_drop_thd")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("netif_drop_thd=%d \n", value);
		ru_ctrl->netif_drop_thd = value;
	}else if(!strcmp(para, "phl_wd_hold_cnt_thd")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("phl_wd_hold_cnt_thd=%d \n", value);
		ru_ctrl->phl_wd_hold_cnt_thd = value;
	}else if(!strcmp(para, "phl_wd_quota")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		if(value < 9){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value_2);
			DBGP("cur phl_wd_quota[%d]:%d\n", value, value_2);
			ru_ctrl->phl_wd_quota[value] = value_2;
		}else
			DBGP("indx > 8\n");
	}else if(!strcmp(para, "mu_edca")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mu_edca=%d \n", value);
		ru_ctrl->mu_edca = value;
	}

	return;
}

void _rtw_set_dl_grp(_adapter *adapter, char *para, u32 para_num){

	u32 value = 0, value_2 = 0;

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;

	if(para_num<=0){
		DBGP("please enter cmd : dl_grp,<param>,<value>\n");
		return;
	}

	if(!strcmp(para, "dump")){
		dump_dl_ru_grp(adapter);
		return;
	}

	if(!strcmp(para, "tx_pwr")){
		para=get_next_ru_para_str(para);
		sscanf(para, "0x%x", &value);
		DBGP("tx_pwr=0x%x \n", value);
		dl_ru_grp->tx_pwr = value;
		return;
	}else if(!strcmp(para, "ppdu_bw")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ppdu_bw=%d \n", value);
		dl_ru_grp->ppdu_bw = value;
		return;
	}else if(!strcmp(para, "txpwr_ofld_en")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("txpwr_ofld_en=%d \n", value);
		dl_ru_grp->txpwr_ofld_en = value;
		return;
	}else if(!strcmp(para, "pwrlim_dis")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("pwrlim_dis=%d \n", value);
		dl_ru_grp->pwrlim_dis = value;
		return;
	}else if(!strcmp(para, "tf_rate")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("tf_rate=%d \n", value);
		dl_ru_grp->tf.tf_rate = value;
		return;
	}else if(!strcmp(para, "tb_ppdu_bw")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("tb_ppdu_bw=%d \n", value);
		dl_ru_grp->tf.tb_ppdu_bw = value;
		return;
	}else if(!strcmp(para, "gi_ltf")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("gi_ltf=%d \n", value);
		dl_ru_grp->tf.gi_ltf = value;
		return;
	}else if(!strcmp(para, "fix_ba")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_ba=%d \n", value);
		dl_ru_grp->tf.fix_ba = value;
		return;
	}else if(!strcmp(para, "ru_psd")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ru_psd=%d \n", value);
		dl_ru_grp->tf.ru_psd = value;
		return;
	}else if(!strcmp(para, "rf_gain_fix")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rf_gain_fix=%d \n", value);
		dl_ru_grp->tf.rf_gain_fix = value;
		return;
	}else if(!strcmp(para, "rf_gain_idx")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rf_gain_idx=%d \n", value);
		dl_ru_grp->tf.rf_gain_idx = value;
		return;
	}else if(!strcmp(para, "doppler")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("doppler=%d \n", value);
		dl_ru_grp->tf.doppler = value;
		return;
	}else if(!strcmp(para, "stbc")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("stbc=%d \n", value);
		dl_ru_grp->tf.stbc = value;
		return;
	}else if(!strcmp(para, "sta_coding")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("sta_coding=%d \n", value);
		dl_ru_grp->tf.sta_coding = value;
		return;
	}else if(!strcmp(para, "tb_t_pe_nom")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("tb_t_pe_nom=%d \n", value);
		dl_ru_grp->tf.tb_t_pe_nom = value;
		return;
	}else if(!strcmp(para, "pr20_bw_en")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("pr20_bw_en=%d \n", value);
		dl_ru_grp->tf.pr20_bw_en = value;
		return;
	}else if(!strcmp(para, "sta_coding")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("sta_coding=%d \n", value);
		dl_ru_grp->tf.sta_coding = value;
		return;
	}else if(!strcmp(para, "rate_mcs")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rate_mcs=%d \n", value);
		dl_ru_grp->tf.rate.mcs = value;
		return;
	}else if(!strcmp(para, "rate_dcm")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rate_dcm=%d \n", value);
		dl_ru_grp->tf.rate.dcm = value;
		return;
	}else if(!strcmp(para, "rate_ss")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rate_ss=%d \n", value);
		dl_ru_grp->tf.rate.ss = value;
		return;
	}

	return;
}

void _rtw_set_dl_fix_grp(_adapter *adapter, char *para, u32 para_num){

	u32 value = 0, value_2 = 0;
	int i;

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;

	if(para_num<=0){
		DBGP("please enter cmd : dl_fix_grp,<param>,<value>\n");
		return;
	}

	if(!strcmp(para, "dump")){
		dump_dl_ru_fix_grp(adapter);
		return;
	}

	if(!strcmp(para, "max_sta_num")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("max_sta_num=%d \n", value);
		dl_ru_fix_grp->max_sta_num = value;
		return;
	}else if(!strcmp(para, "min_sta_num")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("min_sta_num=%d \n", value);
		dl_ru_fix_grp->min_sta_num = value;
		return;
	}else if(!strcmp(para, "rupos_csht_flag")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rupos_csht_flag=%d \n", value);
		dl_ru_fix_grp->rupos_csht_flag = value;
		return;
	}else if(!strcmp(para, "ru_swp_flg")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ru_swp_flg=%d \n", value);
		dl_ru_fix_grp->ru_swp_flg = value;
		return;
	}else if(!strcmp(para, "gi_ltf")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("gi_ltf=%d \n", value);
		dl_ru_fix_grp->gi_ltf = value;
		return;
	}else if(!strcmp(para, "fixru_flag")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fixru_flag=%d \n", value);
		dl_ru_fix_grp->fixru_flag = value;
		return;
	}else if(!strcmp(para, "macid[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[0]=%d \n", value);
		dl_ru_fix_grp->sta_info[0].macid = value;
		return;
	}else if(!strcmp(para, "mcs[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[0]=%d \n", value);
		dl_ru_fix_grp->sta_info[0].mcs = value;
		return;
	}else if(!strcmp(para, "ss[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[0]=%d \n", value);
		dl_ru_fix_grp->sta_info[0].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[0]=%d \n", value);
		dl_ru_fix_grp->sta_info[0].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[0]=%d \n", value);
		dl_ru_fix_grp->sta_info[0].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[0]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[0],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[0] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[0].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[1]=%d \n", value);
		dl_ru_fix_grp->sta_info[1].macid = value;
		return;
	}else if(!strcmp(para, "mcs[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[1]=%d \n", value);
		dl_ru_fix_grp->sta_info[1].mcs = value;
		return;
	}else if(!strcmp(para, "ss[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[1]=%d \n", value);
		dl_ru_fix_grp->sta_info[1].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[1]=%d \n", value);
		dl_ru_fix_grp->sta_info[1].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[1]=%d \n", value);
		dl_ru_fix_grp->sta_info[1].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[1]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[1],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[1] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[1].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[2]=%d \n", value);
		dl_ru_fix_grp->sta_info[2].macid = value;
		return;
	}else if(!strcmp(para, "mcs[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[2]=%d \n", value);
		dl_ru_fix_grp->sta_info[2].mcs = value;
		return;
	}else if(!strcmp(para, "ss[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[2]=%d \n", value);
		dl_ru_fix_grp->sta_info[2].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[2]=%d \n", value);
		dl_ru_fix_grp->sta_info[2].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[2]=%d \n", value);
		dl_ru_fix_grp->sta_info[2].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[2]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[2],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[2] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[2].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[3]=%d \n", value);
		dl_ru_fix_grp->sta_info[3].macid = value;
		return;
	}else if(!strcmp(para, "mcs[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[3]=%d \n", value);
		dl_ru_fix_grp->sta_info[3].mcs = value;
		return;
	}else if(!strcmp(para, "ss[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[3]=%d \n", value);
		dl_ru_fix_grp->sta_info[3].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[3]=%d \n", value);
		dl_ru_fix_grp->sta_info[3].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[3]=%d \n", value);
		dl_ru_fix_grp->sta_info[3].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[3]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[3],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[3] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[3].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[4]=%d \n", value);
		dl_ru_fix_grp->sta_info[4].macid = value;
		return;
	}else if(!strcmp(para, "mcs[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[4]=%d \n", value);
		dl_ru_fix_grp->sta_info[4].mcs = value;
		return;
	}else if(!strcmp(para, "ss[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[4]=%d \n", value);
		dl_ru_fix_grp->sta_info[4].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[4]=%d \n", value);
		dl_ru_fix_grp->sta_info[4].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[4]=%d \n", value);
		dl_ru_fix_grp->sta_info[4].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[4]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[4],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[4] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[4].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[5]=%d \n", value);
		dl_ru_fix_grp->sta_info[5].macid = value;
		return;
	}else if(!strcmp(para, "mcs[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[5]=%d \n", value);
		dl_ru_fix_grp->sta_info[5].mcs = value;
		return;
	}else if(!strcmp(para, "ss[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[5]=%d \n", value);
		dl_ru_fix_grp->sta_info[5].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[5]=%d \n", value);
		dl_ru_fix_grp->sta_info[5].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[5]=%d \n", value);
		dl_ru_fix_grp->sta_info[5].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[5]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[5],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[5] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[5].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[6]=%d \n", value);
		dl_ru_fix_grp->sta_info[6].macid = value;
		return;
	}else if(!strcmp(para, "mcs[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[6]=%d \n", value);
		dl_ru_fix_grp->sta_info[6].mcs = value;
		return;
	}else if(!strcmp(para, "ss[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[6]=%d \n", value);
		dl_ru_fix_grp->sta_info[6].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[6]=%d \n", value);
		dl_ru_fix_grp->sta_info[6].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[6]=%d \n", value);
		dl_ru_fix_grp->sta_info[6].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[6]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[6],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[6] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[6].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[7]=%d \n", value);
		dl_ru_fix_grp->sta_info[7].macid = value;
		return;
	}else if(!strcmp(para, "mcs[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[7]=%d \n", value);
		dl_ru_fix_grp->sta_info[7].mcs = value;
		return;
	}else if(!strcmp(para, "ss[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[7]=%d \n", value);
		dl_ru_fix_grp->sta_info[7].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[7]=%d \n", value);
		dl_ru_fix_grp->sta_info[7].fix_rate = value;
		return;
	}else if(!strcmp(para, "coding[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[7]=%d \n", value);
		dl_ru_fix_grp->sta_info[7].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[7]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[7],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[7] ru_pos[%d]=%d \n", i, value);
			dl_ru_fix_grp->sta_info[7].ru_pos[i]= value;
		}
		return;
	}

	return;
}

void _rtw_set_ul_grp(_adapter *adapter, char *para, u32 para_num){

	u32 value = 0, value_2 = 0;

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;

	if(para_num<=0){
		DBGP("please enter cmd : ul_grp,<param>,<value>\n");
		return;
	}

	if(!strcmp(para, "dump")){
		dump_ul_ru_grp(adapter);
		return;
	}

	if(!strcmp(para, "ppdu_bw")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ppdu_bw=%d \n", value);
		ul_ru_grp->ppdu_bw = value;
		return;
	}else if(!strcmp(para, "grp_psd_max")){
		para=get_next_ru_para_str(para);
		sscanf(para, "0x%x", &value);
		DBGP("grp_psd_max=0x%x \n", value);
		ul_ru_grp->grp_psd_max = value;
		return;
	}else if(!strcmp(para, "grp_psd_min")){
		para=get_next_ru_para_str(para);
		sscanf(para, "0x%x", &value);
		DBGP("grp_psd_min=0x%x \n", value);
		ul_ru_grp->grp_psd_min = value;
		return;
	}else if(!strcmp(para, "fix_tf_rate")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tf_rate=%d \n", value);
		ul_ru_grp->fix_tf_rate = value;
		return;
	}else if(!strcmp(para, "rf_gain_fix")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rf_gain_fix=%d \n", value);
		ul_ru_grp->rf_gain_fix = value;
		return;
	}else if(!strcmp(para, "fix_mode_flags")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_mode_flags=%d \n", value);
		ul_ru_grp->fix_mode_flags = value;
		return;
	}else if(!strcmp(para, "tf_rate")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("tf_rate=%d \n", value);
		ul_ru_grp->tf_rate = value;
		return;
	}else if(!strcmp(para, "rf_gain_idx")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("rf_gain_idx=%d \n", value);
		ul_ru_grp->rf_gain_idx = value;
		return;
	}

	return;
}

void _rtw_set_ul_fix_grp(_adapter *adapter, char *para, u32 para_num){

	u32 value = 0, value_2 = 0;
	int i;

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	if(para_num<=0){
		DBGP("please enter cmd : ul_fix_grp,<param>,<value>\n");
		return;
	}

	if(!strcmp(para, "dump")){
		dump_ul_ru_fix_grp(adapter);
		return;
	}

	if(!strcmp(para, "min_sta_num")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("min_sta_num=%d \n", value);
		ul_ru_fix_grp->min_sta_num = value;
		return;
	}else if(!strcmp(para, "max_sta_num")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("max_sta_num=%d \n", value);
		ul_ru_fix_grp->max_sta_num = value;
		return;
	}else if(!strcmp(para, "gi_ltf")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("gi_ltf=%d \n", value);
		ul_ru_fix_grp->gi_ltf = value;
		return;
	}else if(!strcmp(para, "fixru_flag")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fixru_flag=%d \n", value);
		ul_ru_fix_grp->fixru_flag = value;
		return;
	}else if(!strcmp(para, "macid[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[0]=%d \n", value);
		ul_ru_fix_grp->sta_info[0].macid = value;
		return;
	}else if(!strcmp(para, "mcs[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[0]=%d \n", value);
		ul_ru_fix_grp->sta_info[0].mcs = value;
		return;
	}else if(!strcmp(para, "ss[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[0]=%d \n", value);
		ul_ru_fix_grp->sta_info[0].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[0]=%d \n", value);
		ul_ru_fix_grp->sta_info[0].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[0]=%d \n", value);
		ul_ru_fix_grp->sta_info[0].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[0]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[0]=%d \n", value);
		ul_ru_fix_grp->sta_info[0].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[0]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[0],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[0] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[0].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[0]")){
		if(para_num < 4){
			DBGP("please enter cmd : tgt_rssi[0],<rssi_1>,<rssi_2>,<rssi_3>\n");
			return;
		}
		for(i=0; i<3; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[0] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[0].tgt_rssi[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[1]=%d \n", value);
		ul_ru_fix_grp->sta_info[1].macid = value;
		return;
	}else if(!strcmp(para, "mcs[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[1]=%d \n", value);
		ul_ru_fix_grp->sta_info[1].mcs = value;
		return;
	}else if(!strcmp(para, "ss[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[1]=%d \n", value);
		ul_ru_fix_grp->sta_info[1].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[1]=%d \n", value);
		ul_ru_fix_grp->sta_info[1].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[1]=%d \n", value);
		ul_ru_fix_grp->sta_info[1].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[1]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[1]=%d \n", value);
		ul_ru_fix_grp->sta_info[1].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[1]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[1],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[1] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[1].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[1]")){
		if(para_num < 4){
			DBGP("please enter cmd : tgt_rssi[1],<rssi_1>,<rssi_2>,<rssi_3>\n");
			return;
		}
		for(i=0; i<3; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[1] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[1].tgt_rssi[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[2]=%d \n", value);
		ul_ru_fix_grp->sta_info[2].macid = value;
		return;
	}else if(!strcmp(para, "mcs[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[2]=%d \n", value);
		ul_ru_fix_grp->sta_info[2].mcs = value;
		return;
	}else if(!strcmp(para, "ss[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[2]=%d \n", value);
		ul_ru_fix_grp->sta_info[2].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[2]=%d \n", value);
		ul_ru_fix_grp->sta_info[2].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[2]=%d \n", value);
		ul_ru_fix_grp->sta_info[2].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[2]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[2]=%d \n", value);
		ul_ru_fix_grp->sta_info[2].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[2]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[2],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[2] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[2].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[2]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[2],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[2] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[2].tgt_rssi[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[3]=%d \n", value);
		ul_ru_fix_grp->sta_info[3].macid = value;
		return;
	}else if(!strcmp(para, "mcs[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[3]=%d \n", value);
		ul_ru_fix_grp->sta_info[3].mcs = value;
		return;
	}else if(!strcmp(para, "ss[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[3]=%d \n", value);
		ul_ru_fix_grp->sta_info[3].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[3]=%d \n", value);
		ul_ru_fix_grp->sta_info[3].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[3]=%d \n", value);
		ul_ru_fix_grp->sta_info[3].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[3]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[3]=%d \n", value);
		ul_ru_fix_grp->sta_info[3].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[3]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[3],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[3] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[3].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[3]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[0],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[3] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[3].tgt_rssi[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[4]=%d \n", value);
		ul_ru_fix_grp->sta_info[4].macid = value;
		return;
	}else if(!strcmp(para, "mcs[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[4]=%d \n", value);
		ul_ru_fix_grp->sta_info[4].mcs = value;
		return;
	}else if(!strcmp(para, "ss[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[4]=%d \n", value);
		ul_ru_fix_grp->sta_info[4].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[4]=%d \n", value);
		ul_ru_fix_grp->sta_info[4].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[4]=%d \n", value);
		ul_ru_fix_grp->sta_info[4].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[4]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[4]=%d \n", value);
		ul_ru_fix_grp->sta_info[4].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[4]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[4],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[4] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[4].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[4]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[4],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[4] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[4].tgt_rssi[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[5]=%d \n", value);
		ul_ru_fix_grp->sta_info[5].macid = value;
		return;
	}else if(!strcmp(para, "mcs[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[5]=%d \n", value);
		ul_ru_fix_grp->sta_info[5].mcs = value;
		return;
	}else if(!strcmp(para, "ss[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[5]=%d \n", value);
		ul_ru_fix_grp->sta_info[5].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[5]=%d \n", value);
		ul_ru_fix_grp->sta_info[5].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[5]=%d \n", value);
		ul_ru_fix_grp->sta_info[5].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[5]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[5]=%d \n", value);
		ul_ru_fix_grp->sta_info[5].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[5]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[5],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[5] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[5].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[5]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[5],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[5] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[5].tgt_rssi[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[6]=%d \n", value);
		ul_ru_fix_grp->sta_info[6].macid = value;
		return;
	}else if(!strcmp(para, "mcs[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[6]=%d \n", value);
		ul_ru_fix_grp->sta_info[6].mcs = value;
		return;
	}else if(!strcmp(para, "ss[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[6]=%d \n", value);
		ul_ru_fix_grp->sta_info[6].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[6]=%d \n", value);
		ul_ru_fix_grp->sta_info[6].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[6]=%d \n", value);
		ul_ru_fix_grp->sta_info[6].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[6]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[6]=%d \n", value);
		ul_ru_fix_grp->sta_info[6].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[6]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[6],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[6] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[6].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[6]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[6],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[6] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[6].tgt_rssi[i]= value;
		}
		return;
	}else if(!strcmp(para, "macid[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("macid[7]=%d \n", value);
		ul_ru_fix_grp->sta_info[7].macid = value;
		return;
	}else if(!strcmp(para, "mcs[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("mcs[7]=%d \n", value);
		ul_ru_fix_grp->sta_info[7].mcs = value;
		return;
	}else if(!strcmp(para, "ss[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ss[7]=%d \n", value);
		ul_ru_fix_grp->sta_info[7].ss = value;
		return;
	}else if(!strcmp(para, "fix_rate[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_rate[7]=%d \n", value);
		ul_ru_fix_grp->sta_info[7].fix_rate = value;
		return;
	}else if(!strcmp(para, "fix_tgt_rssi[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("fix_tgt_rssi[7]=%d \n", value);
		ul_ru_fix_grp->sta_info[7].fix_tgt_rssi = value;
		return;
	}else if(!strcmp(para, "coding[7]")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("coding[7]=%d \n", value);
		ul_ru_fix_grp->sta_info[7].coding = value;
		return;
	}else if(!strcmp(para, "ru_pos[7]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[7],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[7] ru_pos[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[7].ru_pos[i]= value;
		}
		return;
	}else if(!strcmp(para, "tgt_rssi[7]")){
		if(para_num < 8){
			DBGP("please enter cmd : ru_pos[7],<pos_1>,<pos_2>,<pos_3>,<pos_4>,<pos_5>,<pos_6>,<pos_7>\n");
			return;
		}
		for(i=0; i<7; i++){
			para=get_next_ru_para_str(para);
			sscanf(para, "%d", &value);
			DBGP("macid[7] tgt_rssi[%d]=%d \n", i, value);
			ul_ru_fix_grp->sta_info[7].tgt_rssi[i]= value;
		}
		return;
	}

	return;
}

void _rtw_set_ulmacid_cfg(_adapter *adapter, char *para, u32 para_num)
{

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ulmacid_cfg_set_para *ulmacid_para = &rugrptable->ulmacid_cfg_set;

	int index;
	u32 value = 0;
	//char *para;

	if(para_num<=0){
		DBGP("please enter cmd : ulmacid_cfg,<indx>,<param>,<value>\n");
		return;
	}

	if(!strcmp(para, "dump")){
		dump_ulmacid_para(adapter);
		return;
	}

	sscanf(para, "%d", &index);
	if (index > 7 || index < 0) {
		DBGP("please enter indx (0~7)\n");
		return;
	}

	para=get_next_ru_para_str(para);

	if(!strcmp(para, "macid")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].macid=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].macid = value;
		return;
	}else if(!strcmp(para, "endcmd")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].endcmd=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].endcmd = value;
		return;
	}else if(!strcmp(para, "ul_su_bw")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_bw=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_bw = value;
		return;
	}else if(!strcmp(para, "ul_su_gi_ltf")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_gi_ltf=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_gi_ltf = value;
		return;
	}else if(!strcmp(para, "ul_su_doppler_ctrl")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_doppler_ctrl=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_doppler_ctrl = value;
		return;
	}else if(!strcmp(para, "ul_su_dcm")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_dcm=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_dcm = value;
		return;
	}else if(!strcmp(para, "ul_su_ss")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_ss=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_ss = value;
		return;
	}else if(!strcmp(para, "ul_su_mcs")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_mcs=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_mcs = value;
		return;
	}else if(!strcmp(para, "ul_su_stbc")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_stbc=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_stbc = value;
		return;
	}else if(!strcmp(para, "ul_su_coding")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_coding=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_coding = value;
		return;
	}else if(!strcmp(para, "ul_su_rssi_m")){
		para=get_next_ru_para_str(para);
		sscanf(para, "%d", &value);
		DBGP("ulmacid_cfg[%d].ul_su_rssi_m=%d \n", index, value);
		ulmacid_para->ulmacid_cfg[index].ul_su_rssi_m = value;
		return;
	}
}

void apply_dl_ofdma_setting(_adapter *adapter, void *phl, struct rtw_phl_dlru_fix_tbl *fix_tbl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct sta_info *psta;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	int i;

	fix_tbl->tbl_hdr.type = RTW_PHL_RU_TBL_HW;
	fix_tbl->max_sta_num = dl_ru_fix_grp->max_sta_num;
	fix_tbl->min_sta_num = dl_ru_fix_grp->min_sta_num;
	fix_tbl->doppler = 0;
	fix_tbl->stbc = 0;
	fix_tbl->gi_ltf = dl_ru_fix_grp->gi_ltf;
	fix_tbl->ma_type = 0;
	fix_tbl->fixru_flag = dl_ru_fix_grp->fixru_flag;
	fix_tbl->rupos_csht_flag = dl_ru_fix_grp->rupos_csht_flag;
	fix_tbl->ru_swp_flg = dl_ru_fix_grp->ru_swp_flg;

	for(i=0; i<RTW_PHL_MAX_RU_STA_NUM; i++){
		fix_tbl->sta[i].mac_id = dl_ru_fix_grp->sta_info[i].macid;
		fix_tbl->sta[i].ru_pos[0] = dl_ru_fix_grp->sta_info[i].ru_pos[0];
		fix_tbl->sta[i].ru_pos[1] = dl_ru_fix_grp->sta_info[i].ru_pos[1];
		fix_tbl->sta[i].ru_pos[2] = dl_ru_fix_grp->sta_info[i].ru_pos[2];
		fix_tbl->sta[i].ru_pos[3] = dl_ru_fix_grp->sta_info[i].ru_pos[3];
		fix_tbl->sta[i].ru_pos[4] = dl_ru_fix_grp->sta_info[i].ru_pos[4];
		fix_tbl->sta[i].ru_pos[5] = dl_ru_fix_grp->sta_info[i].ru_pos[5];
		fix_tbl->sta[i].ru_pos[6] = dl_ru_fix_grp->sta_info[i].ru_pos[6];
		fix_tbl->sta[i].fix_rate = dl_ru_fix_grp->sta_info[i].fix_rate;
		fix_tbl->sta[i].rate.mcs = dl_ru_fix_grp->sta_info[i].mcs;
		fix_tbl->sta[i].rate.ss = dl_ru_fix_grp->sta_info[i].ss;
		fix_tbl->sta[i].rate.dcm = 0;
		fix_tbl->sta[i].fix_coding = 1;
		fix_tbl->sta[i].coding = dl_ru_fix_grp->sta_info[i].coding;
		fix_tbl->sta[i].fix_txbf = 1;
		fix_tbl->sta[i].txbf = 0;
		fix_tbl->sta[i].fix_pwr_fac = 1;
		fix_tbl->sta[i].pwr_boost_fac = 0;
	}
}

void apply_ul_ofdma_setting(_adapter *adapter, void *phl, struct rtw_phl_ulru_fix_tbl *fix_tbl){

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;
	int i;

	fix_tbl->max_sta_num = ul_ru_fix_grp->max_sta_num;
	fix_tbl->min_sta_num = ul_ru_fix_grp->min_sta_num;
	fix_tbl->doppler = 0;
	fix_tbl->ma_type = 0;
	fix_tbl->gi_ltf = ul_ru_fix_grp->gi_ltf;
	fix_tbl->stbc = 0;
	fix_tbl->fix_tb_t_pe_nom = 1;
	fix_tbl->tb_t_pe_nom = 2;
	fix_tbl->fixru_flag = 1;

	for(i=0; i<RTW_PHL_MAX_RU_STA_NUM; i++){
		fix_tbl->sta[i].mac_id = ul_ru_fix_grp->sta_info[i].macid;
		fix_tbl->sta[i].ru_pos[0] = ul_ru_fix_grp->sta_info[i].ru_pos[0];
		fix_tbl->sta[i].ru_pos[1] = ul_ru_fix_grp->sta_info[i].ru_pos[1];
		fix_tbl->sta[i].ru_pos[2] = ul_ru_fix_grp->sta_info[i].ru_pos[2];
		fix_tbl->sta[i].ru_pos[3] = ul_ru_fix_grp->sta_info[i].ru_pos[3];
		fix_tbl->sta[i].ru_pos[4] = ul_ru_fix_grp->sta_info[i].ru_pos[4];
		fix_tbl->sta[i].ru_pos[5] = ul_ru_fix_grp->sta_info[i].ru_pos[5];
		fix_tbl->sta[i].ru_pos[6] = ul_ru_fix_grp->sta_info[i].ru_pos[6];
		fix_tbl->sta[i].fix_tgt_rssi = ul_ru_fix_grp->sta_info[i].fix_tgt_rssi;
		fix_tbl->sta[i].tgt_rssi[0] = ul_ru_fix_grp->sta_info[i].tgt_rssi[0];
		fix_tbl->sta[i].tgt_rssi[1] = ul_ru_fix_grp->sta_info[i].tgt_rssi[1];
		fix_tbl->sta[i].tgt_rssi[2] = ul_ru_fix_grp->sta_info[i].tgt_rssi[2];
		fix_tbl->sta[i].tgt_rssi[3] = ul_ru_fix_grp->sta_info[i].tgt_rssi[3];
		fix_tbl->sta[i].tgt_rssi[4] = ul_ru_fix_grp->sta_info[i].tgt_rssi[4];
		fix_tbl->sta[i].tgt_rssi[5] = ul_ru_fix_grp->sta_info[i].tgt_rssi[5];
		fix_tbl->sta[i].tgt_rssi[6] = ul_ru_fix_grp->sta_info[i].tgt_rssi[6];
		fix_tbl->sta[i].fix_rate = ul_ru_fix_grp->sta_info[i].fix_rate;
		fix_tbl->sta[i].rate.mcs = ul_ru_fix_grp->sta_info[i].mcs;
		fix_tbl->sta[i].rate.ss = ul_ru_fix_grp->sta_info[i].ss;
		fix_tbl->sta[i].rate.dcm = 0;
		fix_tbl->sta[i].fix_coding = 1;
		fix_tbl->sta[i].coding = ul_ru_fix_grp->sta_info[i].coding;
	}
}


void rtw_del_dl_ofdma_grp(_adapter *padapter, int tble_num)
{
	void *phl = padapter->dvobj->phl;
	struct sta_priv *pstapriv = &padapter->stapriv;

	int num_ru_sta;
	struct sta_info *psta_ru[MAX_RU_STA];
	enum rtw_phl_status pstatus;
	_list *phead, *plist;
	int i=0;

	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;
	struct rtw_phl_mac_ss_dl_grp_upd info ={0};

	pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl, true, &mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Get Mac fix mode para Fail\n");
		return;
	}

	mac_fix_tbl->forceru = 0;

	info.grp_valid = 0;
	for (i = 0; i < 8; i++) {
		*(&info.macid_u0 +i) = 0xff;
	}

	DBGP("%s \n",__func__);

	for(i = 0; i < tble_num; i++){

		mac_fix_tbl->tbl_hdr.idx = i+1;			// STA macid
		mac_fix_tbl->rugrpid = i;
		pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
		if (pstatus != RTW_PHL_STATUS_SUCCESS) {
			DBGP("[MAC Fix mode] Set mac fixmode mib (gid:%d)(prim psta%d) Fail\n", mac_fix_tbl->rugrpid, mac_fix_tbl->tbl_hdr.idx);
			return;
		}

		info.grp_id = i;
		pstatus = rtw_phl_mac_set_dl_grp_info(phl, &info);
		if (pstatus != RTW_PHL_STATUS_SUCCESS) {
			DBGP("[MAC DL GRP] Set mac dl grp info (gid:%d) fail\n", mac_fix_tbl->rugrpid);
			return;
		}
	}

}

void rtw_del_ul_ofdma_grp(_adapter *padapter)
{
	void *phl = padapter->dvobj->phl;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct rtw_phl_sw_grp_set sw_grp_set;
	int i =0;
	enum rtw_phl_status pstatus;
	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;	// Mark.CS_update

	_rtw_memset(&sw_grp_set, 0, sizeof(struct rtw_phl_sw_grp_set));
	DBGP("%s \n",__func__);

	for(i = 0; i < 8; i++){
		sw_grp_set.phl_swgrp_bitmap[i].macid = i;
		sw_grp_set.phl_swgrp_bitmap[i].en_upd_ul_swgrp = 0;
		sw_grp_set.phl_swgrp_bitmap[i].ul_sw_grp_bitmap = 0;
	}
	sw_grp_set.phl_swgrp_bitmap[i-1].cmdend = 1;

	pstatus = rtw_phl_set_swgrp_set(phl, &sw_grp_set);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("Set swgrp_set fail\n");
		return;
	}

	rtw_set_ru_ulmacid_cfg(padapter, 0);

	rtw_add_ul_ru_ofdma_grp_3(padapter);

#if 1   // Mark.CS_update
	pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl, true, &mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Get Mac fix mode para Fail\n");
		return;
	}

	mac_fix_tbl->tbl_hdr.rw = 1;
	mac_fix_tbl->set_ulmode = 0;
	mac_fix_tbl->ulmode = 0;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Set mac fixmode mib (gid:%d)(prim psta%d) Fail\n", mac_fix_tbl->rugrpid, mac_fix_tbl->tbl_hdr.idx);
		return;
	}
#endif

}

void rtw_clean_ofdma_grp(_adapter *adapter){
	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;

	struct sta_info *psta;
	struct rtw_phl_ru_sta_info *ru_sta = NULL;
	enum rtw_phl_status pstatus;
	_list	*phead, *plist;

	//DBGP("rtw_clean_ofdma_grp\n");

#if 0
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);

		pstatus = rtw_phl_ru_query_ru_sta_res(phl, true,
					      psta->phl_sta, &ru_sta);
		if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (ru_sta == NULL)) {
			DBGP("GET RU STA Fail\n");
			break;
		}
		/* Update FW RU STA grp bitmap = 0 */
		ru_sta->dl_swgrp_bitmap = 0;
		ru_sta->ul_swgrp_bitmap = 0;
		rtw_phl_ru_set_ru_sta_fw(phl, ru_sta);

		ru_sta->phl_sta_info = NULL;

	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
#endif

	/*Free the RU STA*/
	rtw_phl_ru_release_all_ru_sta_res(phl);

	rtw_phl_ru_release_all_dlru_tbl_res(phl);

	rtw_phl_ru_release_all_ulru_tbl_res(phl);

	rtw_phl_ru_release_all_fixmode_tbl_res(phl);
}

void rtw_stop_ofdma_grp(_adapter *adapter){
	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;

	struct rtw_phl_dlru_tbl *tbl = NULL;
	struct rtw_phl_dlru_fix_tbl	*fix_tbl = NULL;

	struct rtw_phl_ru_sta_info *ru_sta_1 = NULL;
	struct rtw_phl_ru_sta_info *ru_sta_2 = NULL;
	struct rtw_phl_ru_sta_info *ru_sta_3 = NULL;
	struct rtw_phl_ru_sta_info *ru_sta_4 = NULL;

	enum rtw_phl_status pstatus;
	_list	*phead, *plist;
	int i=0;

	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;
	struct rtw_phl_mac_ss_dl_grp_upd info ={0};

	if(pstapriv->asoc_list_cnt < 4){
		DBGP("associated client less than four, can't do ofdma\n");
		return;
	}

	pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl, true, &mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Get Mac fix mode para Fail\n");
		return;
	}

	mac_fix_tbl->forceru = 0;
	mac_fix_tbl->tbl_hdr.idx = 0;
	mac_fix_tbl->rugrpid = 0x0;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Set mac fixmode mib (psta1) Fail\n");
		return;
	}

	info.grp_valid = 0;
	info.macid_u4 = 0xff;
	info.macid_u5 = 0xff;
	info.macid_u6 = 0xff;
	info.macid_u7 = 0xff;
	info.macid_u0 = 0xff;
	info.macid_u1 = 0xff;
	info.macid_u2 = 0xff;
	info.macid_u3 = 0xff;

	info.grp_id = 0;
	pstatus = rtw_phl_mac_set_dl_grp_info(phl, &info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC DL GRP] Set mac dl grp info (id:%d) fail\n", info.grp_id);
		return;
	}

	mac_fix_tbl->tbl_hdr.idx = 1;
	mac_fix_tbl->rugrpid = 0x1;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Set mac fixmode mib (psta2) Fail\n");
		return;
	}
	info.grp_id = 1;
	pstatus = rtw_phl_mac_set_dl_grp_info(phl, &info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC DL GRP] Set mac dl grp info (id:%d) fail\n", info.grp_id);
		return;
	}

	mac_fix_tbl->tbl_hdr.idx = 2;
	mac_fix_tbl->rugrpid = 0x2;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Set mac fixmode mib (psta3) Fail\n");
		return;
	}
	info.grp_id = 2;
	pstatus = rtw_phl_mac_set_dl_grp_info(phl, &info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC DL GRP] Set mac dl grp info (id:%d) fail\n", info.grp_id);
		return;
	}

	mac_fix_tbl->tbl_hdr.idx = 3;
	mac_fix_tbl->rugrpid = 0x3;
	pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Set mac fixmode mib (psta4) Fail\n");
		return;
	}
	info.grp_id = 3;
	pstatus = rtw_phl_mac_set_dl_grp_info(phl, &info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC DL GRP] Set mac dl grp info (id:%d) fail\n", info.grp_id);
		return;
	}

}

void rtw_add_dl_ru_ofdma_grp(_adapter *adapter){
	void *phl = adapter->dvobj->phl;
	struct rtw_wifi_role_t *wrole = adapter->phl_role;	// Mark.CS_update
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;

	struct rtw_phl_dlru_tbl *tbl = NULL;
	struct rtw_phl_dlru_fix_tbl	*fix_tbl = NULL;

	struct sta_info *psta;
	enum rtw_phl_status pstatus;
	_list	*phead, *plist;
	int i;

	DBGP("%s\n", __func__);

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	// #1. Get tbl resource with primary STA
	pstatus = rtw_phl_ru_query_dlru_tbl_res(phl, psta->phl_sta, RTW_PHL_RU_TBL_SW, true, &tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (tbl == NULL)) {
		DBGP("Get DL RU TBL Fail\n");
		return;
	}

#if 1 // Mark.CS_update
	rtw_phl_grp_bw_setting_apply(phl, psta->phl_sta->chandef.bw);
#endif

	tbl->tbl_hdr.type = RTW_PHL_RU_TBL_HW;
	tbl->fix_mode_flag = false;
	tbl->ppdu_bw = dl_ru_grp->ppdu_bw;
	tbl->tx_pwr = dl_ru_grp->tx_pwr;
	tbl->txpwr_ofld_en = dl_ru_grp->txpwr_ofld_en;
	tbl->pwrlim_dis = dl_ru_grp->pwrlim_dis;
	tbl->pwr_boost_fac = 0;
	tbl->fix_mode_flag=1;
	tbl->tf.fix_ba = dl_ru_grp->tf.fix_ba;
	tbl->tf.ru_psd = dl_ru_grp->tf.ru_psd;
	tbl->tf.tf_rate = dl_ru_grp->tf.tf_rate;
	tbl->tf.rf_gain_fix = dl_ru_grp->tf.rf_gain_fix;
	tbl->tf.rf_gain_idx = dl_ru_grp->tf.rf_gain_idx;
	tbl->tf.tb_ppdu_bw = dl_ru_grp->tf.tb_ppdu_bw;
	tbl->tf.rate.mcs = dl_ru_grp->tf.rate.mcs;
	tbl->tf.rate.dcm = dl_ru_grp->tf.rate.dcm;
	tbl->tf.rate.ss = dl_ru_grp->tf.rate.ss;
	tbl->tf.gi_ltf = dl_ru_grp->tf.gi_ltf;
	tbl->tf.doppler = dl_ru_grp->tf.doppler;
	tbl->tf.stbc = dl_ru_grp->tf.stbc;
	tbl->tf.sta_coding = dl_ru_grp->tf.sta_coding;
	tbl->tf.tb_t_pe_nom = dl_ru_grp->tf.tb_t_pe_nom;
	tbl->tf.pr20_bw_en = dl_ru_grp->tf.pr20_bw_en;

	// #2. Get Fix tbl resource with normal tbl
	pstatus = rtw_phl_ru_query_dlru_fix_tbl_res(phl, true, psta->phl_sta, tbl, &fix_tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (fix_tbl == NULL)) {
		DBGP("Get DL RU Fix TBL Fail\n");
		rtw_phl_ru_release_dlru_tbl_res(phl, tbl);
		return;
	}

	i = 0;

	if(ru_ctrl->auto_config){
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);
			if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
				continue;

			DBGP("macid:%d, mac_addr:"MAC_FMT", nss:%d, tx_cap:%d\n", psta->phl_sta->macid, MAC_ARG(psta->phl_sta->mac_addr),
					psta->phl_sta->asoc_cap.nss_tx-1, psta->phl_sta->asoc_cap.he_tx_mcs[0]& 0x3);

			dl_ru_fix_grp->sta_info[i].macid = psta->phl_sta->macid;

			if ((psta->phl_sta->asoc_cap.he_tx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC11){
				dl_ru_fix_grp->sta_info[i].mcs = 11;
				dl_ru_fix_grp->sta_info[i].coding = 1;
				//tbl->tx_pwr = 0x24;
			}else if((psta->phl_sta->asoc_cap.he_tx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC9){
				dl_ru_fix_grp->sta_info[i].mcs = 9;
				dl_ru_fix_grp->sta_info[i].coding = 0;
				//tbl->tx_pwr = 0x28;
			}else{
				dl_ru_fix_grp->sta_info[i].mcs = 7;
				dl_ru_fix_grp->sta_info[i].coding = 0;
				//tbl->tx_pwr = 0x3c;
			}

			dl_ru_fix_grp->sta_info[i].ss =  psta->phl_sta->asoc_cap.nss_tx-1;

			i++;

			if (i>=8)
				break;
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	}

	if(ru_ctrl->ofdma_WFA_mode){
		for(i = 0; i<8; i++)
			dl_ru_fix_grp->sta_info[i].fix_rate = 1;
	}

	apply_dl_ofdma_setting(adapter, phl, fix_tbl);

	// #3. Set tbl into fw
	/* prepare dl ru table OK */
	if (NULL != tbl){
		pstatus = rtw_phl_ru_set_dlru_tbl_fw(phl, tbl);
		if ((RTW_PHL_STATUS_SUCCESS != pstatus)) {
			DBGP("Set DLRU Tbl Fail\n");
			rtw_phl_ru_release_dlru_tbl_res(phl, tbl);
			return;
		}
	}
	/* prepare dl ru fix table OK */
	if (NULL != fix_tbl){
		pstatus = rtw_phl_ru_set_dlru_fix_tbl_fw(phl, fix_tbl);
		if ((RTW_PHL_STATUS_SUCCESS != pstatus)) {
			DBGP("Set DLRU Fix Tbl Fail\n");
			rtw_phl_ru_release_dlru_tbl_res(phl, tbl);
			return;
		}
	}

	// #4. Set dlmacid_cfg
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			continue;

		rtw_set_ru_dlmacid_cfg(adapter, psta);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	return;
}

void rtw_add_dl_ru_ofdma_grp_2(_adapter *adapter){
	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	struct rtw_phl_dlru_tbl *tbl = NULL;
	struct rtw_phl_dlru_fix_tbl	*fix_tbl = NULL;
	u8 macid_info[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	struct sta_info *psta;
	enum rtw_phl_status pstatus;
	_list	*phead, *plist;
	int i;

	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;
	struct rtw_phl_mac_ss_dl_grp_upd info ={0};

	DBGP("%s\n", __func__);

	// #5. set mac parameter
	pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl, true, &mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Get Mac fix mode para Fail\n");
		return;
	}

	mac_fix_tbl->tbl_hdr.rw = 1;
	mac_fix_tbl->force_sumuru_en = ru_ctrl->force_sumuru_en;
	mac_fix_tbl->forceru = ru_ctrl->forceru;
	mac_fix_tbl->forcesu = ru_ctrl->forcesu;
	mac_fix_tbl->fix_fe_heru_en = 1;
	mac_fix_tbl->fix_frame_seq_heru = 1;
	mac_fix_tbl->is_dlruhwgrp = 1;
	mac_fix_tbl->prot_type_heru = ru_ctrl->prot_type;
	info.next_protecttype = ru_ctrl->prot_type;
	mac_fix_tbl->resp_type_heru = ru_ctrl->rsp_type;
	info.next_rsptype = ru_ctrl->rsp_type;

	i = 0;
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			continue;

		macid_info[i] = psta->phl_sta->macid;
		mac_fix_tbl->tbl_hdr.idx = psta->phl_sta->macid;
		mac_fix_tbl->rugrpid = i;
		pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
		if (pstatus != RTW_PHL_STATUS_SUCCESS) {
			DBGP("[MAC Fix mode] Set mac fixmode mib (psta:%d) Fail\n", i);
			_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
			return;
		}

		i++;

		if (i>=8)
			break;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	// #6. set dl_grp_info
	info.grp_valid = 1;
	info.is_hwgrp = 1;
	//info.mru = 0;

	info.grp_id = 0;

	info.macid_u0 = macid_info[0];
	info.macid_u1 = macid_info[1];
	info.macid_u2 = macid_info[2];
	info.macid_u3 = macid_info[3];
	info.macid_u4 = macid_info[4];
	info.macid_u5 = macid_info[5];
	info.macid_u6 = macid_info[6];
	info.macid_u7 = macid_info[7];

	//info.ac_bitmap_u0 = 0xff;
	//info.ac_bitmap_u1 = 0xff;
	//info.ac_bitmap_u2 = 0xff;
	//info.ac_bitmap_u3 = 0xff;
	//info.ac_bitmap_u4 = 0xff;
	//info.ac_bitmap_u5 = 0xff;
	//info.ac_bitmap_u6 = 0xff;
	//info.ac_bitmap_u7 = 0xff;

	pstatus = rtw_phl_mac_set_dl_grp_info(phl, &info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC DL GRP] Set mac dl grp info fail\n");
		return;
	}
}

u16 rtw_get_user_rssi_m(_adapter *adapter){
	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);


	struct sta_info *psta;
	_list	*phead, *plist;
	int i=0;
	enum rtw_phl_status pstatus;
	u16 avg_rssi = 0;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			continue;

		printk("macid: %d, rssi_m:%d\n", psta->phl_sta->macid, psta->phl_sta->stats.average_HE_rx_rssi);
		avg_rssi += psta->phl_sta->stats.average_HE_rx_rssi;

		i++;

		if(i>3)
			break;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	printk("total_rssi:%d, i:%d\n", avg_rssi, i);

	if (i > 0)
		avg_rssi = avg_rssi / i;


	return avg_rssi;
}

u16 rtw_cal_psd_e(_adapter *adapter, u16 rssi_m){
	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	u16 psd_e, ru_position_in;

	psd_e = rssi_m << 1;   // u(8,1) => u(9,2)

	ru_position_in = ul_ru_fix_grp->sta_info[0].ru_pos[0] << 1;

	printk("%s: psd_e:%d, ru_position_in:%d\n", __func__, psd_e, ru_position_in);

	if (ru_position_in < RU26_POSITION)             // 36*2, RU26
		psd_e = psd_e;
	else if (ru_position_in < RU52_POSITION)        // 53*2, RU52
		psd_e = psd_e - PSD_CMP_RU52;
	else if (ru_position_in < RU106_POSITION)       // 61*2, RU106,
		psd_e = psd_e - PSD_CMP_RU106;
	else if (ru_position_in < RU242_POSITION)       // 65*2, RU242,
		psd_e = psd_e - PSD_CMP_RU242;
	else if (ru_position_in < RU484_POSITION)       // 67*2, RU484,
		psd_e = psd_e - PSD_CMP_RU484;
	else                                            // RU996,
		psd_e = psd_e - PSD_CMP_RU996;

	printk("%s: psd_e:%d\n", __func__, psd_e);

	return psd_e;
}

void rtw_add_ul_ru_ofdma_grp(_adapter *adapter){
	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	struct rtw_phl_ulru_tbl *tbl = NULL;
	struct rtw_phl_ulru_fix_tbl *fix_tbl = NULL;
	struct rtw_phl_sw_grp_set sw_grp_set;

	struct sta_info *psta;
	_list	*phead, *plist;
	int i;
	enum rtw_phl_status pstatus;
	u16 avg_rssi_m, psd_e;

	if((!ru_ctrl->ofdma_WFA_mode) && (pstapriv->asoc_list_cnt < 2)){
		DBGP("%s: associated client less than 2, can't do ofdma\n", __func__);
		return;
	}

	_rtw_memset(&sw_grp_set, 0, sizeof(struct rtw_phl_sw_grp_set));

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
	//adapter->ss[0] = psta->phl_sta->asoc_cap.nss_tx - 1;
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	// #1. Get tbl resource with primary STA
	pstatus = rtw_phl_ru_query_ulru_tbl_res(phl, psta->phl_sta, RTW_PHL_RU_TBL_SW, true, &tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (tbl == NULL)) {
		DBGP("Get DL RU TBL Fail\n");
		return;
	}

#if 1 // Mark.CS_update
	rtw_phl_grp_bw_setting_apply(phl, psta->phl_sta->chandef.bw);
#endif

	tbl->tbl_hdr.type = RTW_PHL_RU_TBL_SW;

	if(ru_ctrl->ul_psd){
		avg_rssi_m = rtw_get_user_rssi_m(adapter);
		psd_e = rtw_cal_psd_e(adapter, avg_rssi_m);

		tbl->grp_psd_max = psd_e + (ru_ctrl->psd_ofst1 << 2);
		tbl->grp_psd_min = tbl->grp_psd_max - (ru_ctrl->psd_ofst2 << 2);

		printk("avg_rssi_m:%d, psd_e:%d, grp_psd_max:%d, grp_psd_min:%d\n",
			avg_rssi_m, psd_e, tbl->grp_psd_max, tbl->grp_psd_min);
	}else {
		tbl->grp_psd_max = ul_ru_grp->grp_psd_max;
		tbl->grp_psd_min = ul_ru_grp->grp_psd_min;
	}

	tbl->tf_rate = ul_ru_grp->tf_rate;
	tbl->fix_tf_rate =	ul_ru_grp->fix_tf_rate;
	tbl->ppdu_bw = ul_ru_grp->ppdu_bw;
	tbl->rf_gain_fix = ul_ru_grp->rf_gain_fix;
	tbl->rf_gain_idx = ul_ru_grp->rf_gain_idx;
	tbl->fix_mode_flags = ul_ru_grp->fix_mode_flags;

	// #2. Get Fix tbl resource with normal tbl
	pstatus = rtw_phl_ru_query_ulru_fix_tbl_res(phl, true, psta->phl_sta, tbl, &fix_tbl);
	if ((RTW_PHL_STATUS_SUCCESS != pstatus) || (fix_tbl == NULL)) {
		DBGP("Get DL RU Fix TBL Fail\n");
		rtw_phl_ru_release_ulru_tbl_res(phl, tbl);
		return;
	}

	i = 0;
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			continue;
		DBGP("macid:%d, mac_addr:"MAC_FMT", nss:%d, rx_cap:%d\n", psta->phl_sta->macid, MAC_ARG(psta->phl_sta->mac_addr),
			psta->phl_sta->asoc_cap.nss_rx-1, psta->phl_sta->asoc_cap.he_rx_mcs[0]& 0x3);

		if(ru_ctrl->auto_config){
			ul_ru_fix_grp->sta_info[i].macid = psta->phl_sta->macid;

			if ((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC11)
				ul_ru_fix_grp->sta_info[i].mcs = 11;
			else if((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC9)
				ul_ru_fix_grp->sta_info[i].mcs = 9;
			else
				ul_ru_fix_grp->sta_info[i].mcs = 7;

			ul_ru_fix_grp->sta_info[i].ss = psta->phl_sta->asoc_cap.nss_rx-1;

			if (psta->phl_sta->asoc_cap.he_ldpc)
				ul_ru_fix_grp->sta_info[i].coding = 1; // LDPC
			else
				ul_ru_fix_grp->sta_info[i].coding = 0; // BCC
		}

		sw_grp_set.phl_swgrp_bitmap[i].macid = psta->phl_sta->macid;
		sw_grp_set.phl_swgrp_bitmap[i].en_upd_ul_swgrp = 1;
		sw_grp_set.phl_swgrp_bitmap[i].ul_sw_grp_bitmap = BIT0;
		i++;
		if (i>=8)
			break;
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	sw_grp_set.phl_swgrp_bitmap[i-1].cmdend = 1;

	if(ru_ctrl->ofdma_WFA_mode || ru_ctrl->auto_config) {
		ul_ru_fix_grp->min_sta_num = i;
		ul_ru_fix_grp->max_sta_num = i;
	}

	apply_ul_ofdma_setting(adapter, phl, fix_tbl);

	// #3. Set tbl into fw
	/* prepare dl ru table OK */
	if (NULL != tbl)
		pstatus = rtw_phl_ru_set_ulru_tbl_fw(phl, tbl);
	/* prepare dl ru fix table OK */
	if (NULL != fix_tbl)
		pstatus = rtw_phl_ru_set_ulru_fix_tbl_fw(phl, fix_tbl);

	pstatus = rtw_phl_set_swgrp_set(phl, &sw_grp_set);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("Set swgrp_set fail\n");
		return;
	}

	return;
}

void rtw_add_ul_ru_ofdma_grp_2(_adapter *adapter){

	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;

	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	struct rtw_phl_ax_ul_fixinfo tbl_b;
	struct sta_info *psta = NULL;
	_list	*phead, *plist;
	int i;
	enum rtw_phl_status pstatus;
	struct rtw_phl_mac_ax_fixmode_para *mac_fix_tbl=NULL;	// Mark.CS_update

	if((!ru_ctrl->ofdma_WFA_mode) && (pstapriv->asoc_list_cnt < 2)){
		DBGP("%s: associated client less than 2, can't do ofdma\n", __func__);
		return;
	}

	_rtw_memset(&tbl_b, 0, sizeof(struct rtw_phl_ax_ul_fixinfo));

	i = 0;
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
		  continue;
		DBGP("macid %d, aid %d, mac_addr %x:%x:%x:%x:%x:%x\n",psta->phl_sta->macid,psta->phl_sta->aid, psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2], psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5]);

		tbl_b.sta[i].macid = psta->phl_sta->macid;
		tbl_b.ulrua.sta[i].mac_id = psta->phl_sta->macid;
		if (psta->phl_sta->asoc_cap.he_ldpc)
			tbl_b.ulrua.sta[i].coding = 1; //LDPC
		else
			tbl_b.ulrua.sta[i].coding = 0; //BCC
		tbl_b.ulrua.sta[i].rate.mcs = 7;
		tbl_b.ulrua.sta[i].rate.ss = 0;
		tbl_b.ulrua.sta[i].ru_pos = (RTW_HE_RU52_1 + i)*2;
		tbl_b.ulrua.sta[i].tgt_rssi = 70;

		i++;
		if (i>=8) {
			break;
		}

	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	tbl_b.ulrua.sta_num = i;

#if 0
	if(tbl_b.ulrua.sta_num ==4){
		pstatus = rtw_phl_mac_set_ul_grp_info(phl);
		if (pstatus != RTW_PHL_STATUS_SUCCESS) {
			DBGP("[MAC UL GRP] Set mac ul grp info fail\n");
			return;
		}
	}
#endif

	//cfg
	tbl_b.cfg.mode = 0x2;
	tbl_b.cfg.storemode = 0x2;
	tbl_b.store_idx = 0x0;
	tbl_b.ulfix_usage = 0x3;
	tbl_b.cfg.interval = 60; //microseconds

	// tf
	tbl_b.data_rate = 0x8;
	tbl_b.data_bw = 0x0;
	tbl_b.gi_ltf = 0x0;
	tbl_b.tf_type = 0x1;

	// common
	//tbl_b.ulrua.sta_num = 0x2;
	tbl_b.ulrua.gi_ltf = 0x0;
	tbl_b.ulrua.n_ltf_and_ma = 0x0;
	tbl_b.ulrua.ppdu_bw = 0x0;
	tbl_b.apep_len = 0x20;

	if(ru_ctrl->ofdma_WFA_mode){
		tbl_b.ul_logo_test = 1;
	}

	rtw_phl_mac_set_upd_ul_fixinfo(phl, &tbl_b);


#if 1  // Mark.CS_update
	pstatus = rtw_phl_ru_query_mac_fix_mode_para(phl, true, &mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Get Mac fix mode para Fail\n");
		return;
	}

	mac_fix_tbl->tbl_hdr.rw = 1;
	mac_fix_tbl->set_ulmode = 1;
	mac_fix_tbl->ulmode = 1;

	pstatus = rtw_phl_mac_set_fixmode_mib(phl, mac_fix_tbl);
	if (pstatus != RTW_PHL_STATUS_SUCCESS) {
		DBGP("[MAC Fix mode] Set mac fixmode mib (psta:%d) Fail\n", i);
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
		return;
	}
#endif

	return;
}

void rtw_add_ul_ru_ofdma_grp_3(_adapter *adapter){
	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;

	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);

	struct rtw_phl_ax_ul_fixinfo tbl_b;
	struct sta_info *psta = NULL;
	_list	*phead, *plist;
	int i;
	enum rtw_phl_status pstatus;

	if(pstapriv->asoc_list_cnt < 2){
		DBGP("%s: associated client less than 2, can't do ofdma\n", __func__);
		return;
	}

	_rtw_memset(&tbl_b, 0, sizeof(struct rtw_phl_ax_ul_fixinfo));

	i = 0;
	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
		  continue;
		DBGP("macid %d, aid %d, mac_addr %x:%x:%x:%x:%x:%x\n",psta->phl_sta->macid,psta->phl_sta->aid, psta->phl_sta->mac_addr[0], psta->phl_sta->mac_addr[1], psta->phl_sta->mac_addr[2], psta->phl_sta->mac_addr[3], psta->phl_sta->mac_addr[4], psta->phl_sta->mac_addr[5]);

		tbl_b.sta[i].macid = psta->phl_sta->macid;
    	tbl_b.ulrua.sta[i].mac_id = psta->phl_sta->macid;
    	tbl_b.ulrua.sta[i].coding = 0;
		tbl_b.ulrua.sta[i].rate.mcs = 7;
    	tbl_b.ulrua.sta[i].rate.ss = 0;
		tbl_b.ulrua.sta[i].ru_pos = 0x4a + i*2;
		tbl_b.ulrua.sta[i].tgt_rssi = 70;

		i++;
		if (i>=8) {
			break;
		}

	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	tbl_b.ulrua.sta_num = i;

#if 0
	if(tbl_b.ulrua.sta_num ==4){
		pstatus = rtw_phl_mac_set_ul_grp_info(phl);
		if (pstatus != RTW_PHL_STATUS_SUCCESS) {
			DBGP("[MAC UL GRP] Set mac ul grp info fail\n");
			return;
		}
	}
#endif

	//cfg
	tbl_b.cfg.mode = 0x3;
	tbl_b.cfg.storemode = 0x2;
	tbl_b.store_idx = 0x0;
	tbl_b.ulfix_usage = 0x3;
	tbl_b.cfg.interval = 0x5; //microseconds

	// tf
	tbl_b.data_rate = 0x8;
	tbl_b.data_bw = 0x0;
	tbl_b.gi_ltf = 0x0;
	tbl_b.tf_type = 0x0;

	// common
	//tbl_b.ulrua.sta_num = 0x2;
	tbl_b.ulrua.gi_ltf = 0x0;
	tbl_b.ulrua.n_ltf_and_ma = 0x0;
	tbl_b.ulrua.ppdu_bw = 0x0;
	tbl_b.apep_len = 0x20;

	tbl_b.ul_logo_test = 0;

	rtw_phl_mac_set_upd_ul_fixinfo(phl, &tbl_b);

	return;
}

void set_ru_ulmacid_cfg_as_fix (_adapter *adapter, struct rtw_phl_ulmacid_set *cfg)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ulmacid_cfg_set_para *ulmacid_cfg_set = &rugrptable->ulmacid_cfg_set;

	int i;

	for(i=0; i<8; i++){
		cfg->phl_ul_macid_cfg[i].macid = ulmacid_cfg_set->ulmacid_cfg[i].macid;
		cfg->phl_ul_macid_cfg[i].endcmd = ulmacid_cfg_set->ulmacid_cfg[i].endcmd;
		cfg->phl_ul_macid_cfg[i].ul_su_info_en = 1;
		cfg->phl_ul_macid_cfg[i].ul_su_bw = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_bw;
		cfg->phl_ul_macid_cfg[i].ul_su_gi_ltf = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_gi_ltf;
		cfg->phl_ul_macid_cfg[i].ul_su_doppler_ctrl = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_doppler_ctrl;
		cfg->phl_ul_macid_cfg[i].ul_su_dcm = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_dcm;
		cfg->phl_ul_macid_cfg[i].ul_su_ss = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_ss;
		cfg->phl_ul_macid_cfg[i].ul_su_mcs = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_mcs;
		cfg->phl_ul_macid_cfg[i].ul_su_stbc = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_stbc;
		cfg->phl_ul_macid_cfg[i].ul_su_coding = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_coding;
		cfg->phl_ul_macid_cfg[i].ul_su_rssi_m = ulmacid_cfg_set->ulmacid_cfg[i].ul_su_rssi_m;
	}

	return;
}

void rtw_set_ru_ulmacid_cfg(_adapter *adapter, u8 en)
{

	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;

	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;
	u16 rate, ss;

	struct sta_info *psta = NULL;
	_list	*phead, *plist;
	int i=0;
	enum rtw_phl_status pstatus;

	struct rtw_phl_ulmacid_set cfg;

	_rtw_memset(&cfg, 0, sizeof(struct rtw_phl_ulmacid_set));

	if(ru_ctrl->ulmacid_cfg_fix){
		set_ru_ulmacid_cfg_as_fix(adapter, &cfg);
		rtw_phl_ru_set_ulmacid_cfg(phl, &cfg);
		return;
	}

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		plist = get_next(plist);
		if ((psta->hepriv.he_option==0) || (pmlmepriv->hepriv.he_option==0))
			continue;

		cfg.phl_ul_macid_cfg[i].macid = psta->phl_sta->macid;
		if(en)
			cfg.phl_ul_macid_cfg[i].ul_su_info_en = 1;
		else
			cfg.phl_ul_macid_cfg[i].ul_su_info_en = 0;
		cfg.phl_ul_macid_cfg[i].endcmd = 0;

		cfg.phl_ul_macid_cfg[i].ul_su_bw = psta->phl_sta->stats.HE_rx_bw;
		cfg.phl_ul_macid_cfg[i].ul_su_gi_ltf = psta->phl_sta->stats.HE_rx_GI_LTF;
		cfg.phl_ul_macid_cfg[i].ul_su_doppler_ctrl = 0;
		cfg.phl_ul_macid_cfg[i].ul_su_dcm = 0;
		cfg.phl_ul_macid_cfg[i].ul_su_ss = psta->phl_sta->stats.HE_rx_avg_ss;
		cfg.phl_ul_macid_cfg[i].ul_su_mcs = psta->phl_sta->stats.HE_rx_avg_mcs;
		cfg.phl_ul_macid_cfg[i].ul_su_stbc = 0;
		cfg.phl_ul_macid_cfg[i].ul_su_coding = 1;
		//cfg.phl_ul_macid_cfg[i].ul_su_rssi_m = (u8)(psta->phl_sta->hal_sta->rssi_stat.rssi);
		cfg.phl_ul_macid_cfg[i].ul_su_rssi_m = psta->phl_sta->stats.average_HE_rx_rssi;

		if(ru_ctrl->ofdma_WFA_mode){
			cfg.phl_ul_macid_cfg[i].ul_su_gi_ltf = RTW_GILTF_LGI_4XHE32;

			if ((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC11)
				cfg.phl_ul_macid_cfg[i].ul_su_mcs = 11;
			else if((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC9)
				cfg.phl_ul_macid_cfg[i].ul_su_mcs = 9;
			else
				cfg.phl_ul_macid_cfg[i].ul_su_mcs = 7;

			cfg.phl_ul_macid_cfg[i].ul_su_ss =	psta->phl_sta->asoc_cap.nss_rx-1;

			if (psta->phl_sta->asoc_cap.he_ldpc)
				cfg.phl_ul_macid_cfg[i].ul_su_coding = 1; //LDPC
			else
				cfg.phl_ul_macid_cfg[i].ul_su_coding = 0; //BCC

			cfg.phl_ul_macid_cfg[i].ul_su_bw = psta->phl_sta->chandef.bw;

			cfg.phl_ul_macid_cfg[i].ul_su_rssi_m = ul_ru_fix_grp->sta_info[i].tgt_rssi[0] << 1;
		}

		i++;
		if (i>=8){
			break;
		}

	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	if (i > 0) {
		cfg.phl_ul_macid_cfg[i-1].endcmd = 1;
		rtw_phl_ru_set_ulmacid_cfg(phl, &cfg);
	}

}

void rtw_set_ru_dlmacid_cfg(_adapter *adapter, struct sta_info *psta)
{
	void *phl = adapter->dvobj->phl;
	struct rtw_phl_dlmacid_cfg cfg;

	_rtw_memset(&cfg, 0, sizeof(struct rtw_phl_dlmacid_cfg));

	cfg.macid = psta->phl_sta->macid;
	cfg.dl_su_info_en = 1;
	cfg.dl_su_rate_cfg = 0;
	cfg.dl_su_pwr_cfg = 0;

	cfg.gi_ltf_4x8_support = 1; 	// To Do: parsing cap
	cfg.gi_ltf_1x8_support = 0; 	// To Do: parsing cap
	cfg.dl_su_doppler_ctrl = 0; 	// To Do: parsing cap
	cfg.dl_su_coding = 1;			// To Do: parsing cap

	rtw_phl_ru_set_dlmacid_cfg(phl, &cfg);

	return;

}

void rtw_ru_set_ch_bw(_adapter *adapter)
{
	void *phl = adapter->dvobj->phl;
	struct rtw_wifi_role_t *wrole = adapter->phl_role;
	struct rtw_phl_ch_bw_notif info;

	_rtw_memset(&info, 0, sizeof(struct rtw_phl_ch_bw_notif));
	info.band_idx = wrole->hw_band;
	info.band_type = wrole->chandef.band;
	info.pri_ch = wrole->chandef.chan;
	info.central_ch = rtw_get_center_ch(wrole->chandef.chan,
	                                    wrole->chandef.bw,
	                                    wrole->chandef.offset);
	info.cbw = wrole->chandef.bw;

	rtw_phl_ru_set_ch_bw(phl, &info);

	return;
}

void rtw_ru_set_pwrtbl(_adapter *adapter)
{
	void *phl = adapter->dvobj->phl;
	struct rtw_wifi_role_t *wrole = adapter->phl_role;
	struct rtw_phl_pwrtbl_notif info;

	_rtw_memset(&info, 0, sizeof(struct rtw_phl_pwrtbl_notif));

	info.txpwrtbl_ofld_en = 1;
	info.band_idx = wrole->hw_band;

	rtw_phl_set_pwr_tbl_notify(phl, &info);

	return;
}

void rtw_remove_grp(_adapter *adapter){

	void *phl = adapter->dvobj->phl;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	u32 tmp8, tmp16, tmp32;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	struct sta_info *psta = NULL;
	_list	*phead, *plist;

	if (ru_ctrl->tbl_exist || adapter->dynamic_grp_inited){
		DBGP("%s (tbl_exist:%d)\n", __func__, ru_ctrl->tbl_exist);
		rtw_phl_write8(phl, 0xca00, 0x10);	// ca00 bit[4]  0: zero padding; 1: EOF Padding

		// disable TCP ack trigger with data
		rtw_phl_write8(adapter->dvobj->phl, 0xc7f2, 0x0);
		rtw_phl_write8(adapter->dvobj->phl, 0xc7f3, 0x0);

		// improve BB RX
		rtw_phl_write32(adapter->dvobj->phl, 0x1429c, 0x51ac20);
		// 0x10:recovery; 0x20:ul ra; 0x40:rate fall back 1
		//rtw_phl_write32(adapter->dvobj->phl, 0xd54c, 0x0);
		rtw_phl_write32(adapter->dvobj->phl, 0x12250, 0x0);

		// pwr module disable
		//rtw_phl_write32(adapter->dvobj->phl, 0xd550, 0x0);
		rtw_phl_write32(adapter->dvobj->phl, 0x12254, 0x0);

		if(!ru_ctrl->ofdma_WFA_mode){
			// remove DL table
			rtw_del_dl_ofdma_grp(adapter, 8);
			// remove UL table
			rtw_del_ul_ofdma_grp(adapter);
		}

		rtw_clean_ofdma_grp(adapter);
		ru_ctrl->tx_phase = 0;
		ru_ctrl->tbl_exist = 0;
		adapter->no_rts = 0;
		pxmitpriv->txsc_enable = 1;
		pxmitpriv->txsc_amsdu_enable = 1;

		phl_grp_dlru_reset(phl);
		phl_grp_ulru_reset(phl);

		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);

			psta->phl_sta->grp_info.tx_grp_idx_old = 0xff;
			psta->phl_sta->grp_info.rx_grp_idx_old = 0xff;
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	}

	if(adapter->dynamic_grp_inited)
		adapter->dynamic_grp_inited = 0;

	return;
}

#define TOTAL_WD_QUOTA 1600

void rtw_establish_dl_grp(_adapter *adapter){

	void *phl = adapter->dvobj->phl;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	u32 tmp8, tmp16, tmp32;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct sta_priv *pstapriv = &adapter->stapriv;
	struct sta_info *psta;
	_list *phead, *plist;
	enum wlan_mode wmode;
	u8 AX_STA_NUM = 0;
	u32 user_quota = 0;

	//DBGP("%s\n", __func__);

	if (ru_ctrl->ofdma_WFA_mode && adapter->stapriv.asoc_list_cnt < 4){
		rtw_remove_grp(adapter);
		return;
	}

	if((ru_ctrl->tbl_exist & BIT0) &&
		!(adapter->registrypriv.wifi_mib.ofdma_enable & dl_fix_mode_by_assoc))
		return;

	if(ru_ctrl->ofdma_WFA_mode){
		if (adapter->stapriv.asoc_list_cnt >= 4) {
			DBGP("%s (%d RU)\n", __func__, adapter->stapriv.asoc_list_cnt);
			user_quota = TOTAL_WD_QUOTA / 4;
			dl_ru_fix_grp->max_sta_num = 4;
			dl_ru_fix_grp->min_sta_num = 4;
			rtw_clean_ofdma_grp(adapter);
			ru_ctrl->rsp_type = FRAME_EXCHANGE_NMINUS1USER_BA;
			rtw_add_dl_ru_ofdma_grp(adapter);
			rtw_add_dl_ru_ofdma_grp_2(adapter);
			ru_ctrl->tx_phase = 1;
			ru_ctrl->tbl_exist |= BIT0;
			adapter->no_rts = 1;
			ru_ctrl->netif_drop_thd = 4;
			ru_ctrl->phl_wd_quota[0] = user_quota;
			ru_ctrl->phl_wd_quota[1] = user_quota;
			ru_ctrl->phl_wd_quota[2] = user_quota;
			ru_ctrl->phl_wd_quota[3] = user_quota;
			ru_ctrl->phl_wd_quota[4] = user_quota;
			ru_ctrl->phl_wd_quota[5] = user_quota;
			ru_ctrl->phl_wd_quota[6] = user_quota;
			ru_ctrl->phl_wd_quota[7] = user_quota;
			ru_ctrl->phl_wd_quota[8] = user_quota;
			rtw_phl_write8(phl, 0xca00, 0x0);	// ca00 bit[4]  0: zero padding; 1: EOF Padding
			pxmitpriv->txsc_enable = 1;
			pxmitpriv->txsc_amsdu_enable = 1;
			// TCP ack trigger with data
			//rtw_phl_write8(adapter->dvobj->phl, 0xc7f2, 0x2);
			//rtw_phl_write8(adapter->dvobj->phl, 0xc7f3, 0x40);
		}

		return;
	}

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		wmode = psta->phl_sta->wmode;
		if (wmode & WLAN_MD_11AX)
			AX_STA_NUM ++;

		plist = get_next(plist);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	if (AX_STA_NUM >= 8) {
		DBGP("%s (8 RU)\n", __func__);
		if(ru_ctrl->auto_config) {
			dl_ru_fix_grp->max_sta_num = 8;
			dl_ru_fix_grp->min_sta_num = 8;
		}
		user_quota = TOTAL_WD_QUOTA / 8;
		rtw_clean_ofdma_grp(adapter);
		rtw_add_dl_ru_ofdma_grp(adapter);
		rtw_add_dl_ru_ofdma_grp_2(adapter);
		ru_ctrl->tx_phase = 1;
		ru_ctrl->tbl_exist |= BIT0;
		adapter->no_rts = 1;
		ru_ctrl->netif_drop_thd = 8;
		ru_ctrl->phl_wd_quota[0] = user_quota;
		ru_ctrl->phl_wd_quota[1] = user_quota;
		ru_ctrl->phl_wd_quota[2] = user_quota;
		ru_ctrl->phl_wd_quota[3] = user_quota;
		ru_ctrl->phl_wd_quota[4] = user_quota;
		ru_ctrl->phl_wd_quota[5] = user_quota;
		ru_ctrl->phl_wd_quota[6] = user_quota;
		ru_ctrl->phl_wd_quota[7] = user_quota;
		ru_ctrl->phl_wd_quota[8] = user_quota;
		rtw_phl_write8(phl, 0xca00, 0x0);	// ca00 bit[4]  0: zero padding; 1: EOF Padding
		pxmitpriv->txsc_enable = 1;
		pxmitpriv->txsc_amsdu_enable = 1;
		// TCP ack trigger with data
		rtw_phl_write8(adapter->dvobj->phl, 0xc7f2, 0x2);
		rtw_phl_write8(adapter->dvobj->phl, 0xc7f3, 0x40);
	}
	else if (AX_STA_NUM >=2) {
		DBGP("%s (%d RU)\n", __func__, AX_STA_NUM);
		if(ru_ctrl->auto_config) {
			if(AX_STA_NUM == 3) {
				dl_ru_fix_grp->max_sta_num = AX_STA_NUM;
				dl_ru_fix_grp->min_sta_num = AX_STA_NUM - 1;
			} else {
				dl_ru_fix_grp->max_sta_num = AX_STA_NUM;
				dl_ru_fix_grp->min_sta_num = AX_STA_NUM;
			}
		}
		user_quota = TOTAL_WD_QUOTA / AX_STA_NUM;

		rtw_clean_ofdma_grp(adapter);
		rtw_add_dl_ru_ofdma_grp(adapter);
		rtw_add_dl_ru_ofdma_grp_2(adapter);
		ru_ctrl->tx_phase = 1;
		ru_ctrl->tbl_exist |= BIT0;
		adapter->no_rts = 1;
		ru_ctrl->netif_drop_thd = AX_STA_NUM;
		ru_ctrl->phl_wd_quota[0] = user_quota;
		ru_ctrl->phl_wd_quota[1] = user_quota;
		ru_ctrl->phl_wd_quota[2] = user_quota;
		ru_ctrl->phl_wd_quota[3] = user_quota;
		ru_ctrl->phl_wd_quota[4] = user_quota;
		ru_ctrl->phl_wd_quota[5] = user_quota;
		ru_ctrl->phl_wd_quota[6] = user_quota;
		ru_ctrl->phl_wd_quota[7] = user_quota;
		ru_ctrl->phl_wd_quota[8] = user_quota;
		rtw_phl_write8(phl, 0xca00, 0x0);	// ca00 bit[4]  0: zero padding; 1: EOF Padding
		pxmitpriv->txsc_enable = 1;
		pxmitpriv->txsc_amsdu_enable = 1;
		// TCP ack trigger with data
		rtw_phl_write8(adapter->dvobj->phl, 0xc7f2, 0x2);
		rtw_phl_write8(adapter->dvobj->phl, 0xc7f3, 0x40);
	}

	return;
}

void rtw_establish_ul_grp(_adapter *adapter){

	void *phl = adapter->dvobj->phl;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	u32 tmp8, tmp16, tmp32;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
 	struct ul_ru_grp_table_para *ul_grp_tbl = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_fix_grp_tbl = &rugrptable->ul_ru_fix_grp_table;

	//DBGP("%s\n", __func__);

	if((ru_ctrl->tbl_exist & BIT4) &&
		!(adapter->registrypriv.wifi_mib.ofdma_enable & ul_fix_mode_by_assoc)){
		return;
	}

	rtw_clean_ofdma_grp(adapter);
	rtw_add_ul_ru_ofdma_grp(adapter);
	rtw_add_ul_ru_ofdma_grp_2(adapter);

	rtw_set_ru_ulmacid_cfg(adapter, 1);

	// improve BB RX
	rtw_phl_write32(adapter->dvobj->phl, 0x1429c, 0x51cc20);
	//rtw_phl_write32(padapter->dvobj->phl, 0x14814, 0x453F4753);

	rtw_hw_set_edca(adapter, 0, 0x1010);

	if(ru_ctrl->ofdma_WFA_mode){
		ru_ctrl->ulmacid_cfg = 0;
	}else {
		// 0x10:recovery; 0x20:ul ra; 0x40:rate fall back
		rtw_phl_write32(adapter->dvobj->phl, 0x12250, 0x20);

		// pwr module enable
		//rtw_phl_write32(adapter->dvobj->phl, 0x12254, 0x100);
	}

	ru_ctrl->tbl_exist |= BIT4;

	return;
}

#if 0		// Mark.CS_update
void rtw_ofdma_grp_bw160_setting(_adapter *padapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	void *phl = padapter->dvobj->phl;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_160;
	dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_160;
	dl_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;

	ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_160;
	ul_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;
	ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM54;

	phl_bw160_init_8ru_pos(phl);
}

void rtw_ofdma_grp_bw80_setting(_adapter *padapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	void *phl = padapter->dvobj->phl;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_80;
	dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_80;
	dl_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE08;

	ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_80;
	if(ru_ctrl->ofdma_WFA_mode){
		ul_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;
		ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM54;
	}else{
		ul_ru_fix_grp->gi_ltf = RTW_GILTF_2XHE16;
		ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;
	}

	phl_bw80_init_8ru_pos(phl);
}

void rtw_ofdma_grp_bw40_setting(_adapter *padapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	void *phl = padapter->dvobj->phl;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_40;
	dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_40;
	dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

	ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_40;
	ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;
	ul_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

	phl_bw40_init_8ru_pos(phl);
}

void rtw_ofdma_grp_bw20_setting(_adapter *padapter)
{
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	void *phl = padapter->dvobj->phl;
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;
	struct dl_ru_grp_table_para *dl_ru_grp = &rugrptable->dl_ru_grp_table;
	struct dl_ru_fix_grp_table_para *dl_ru_fix_grp = &rugrptable->dl_ru_fix_grp_table;
	struct ul_ru_grp_table_para *ul_ru_grp = &rugrptable->ul_ru_grp_table;
	struct ul_ru_fix_grp_table_para *ul_ru_fix_grp = &rugrptable->ul_ru_fix_grp_table;

	dl_ru_grp->ppdu_bw = CHANNEL_WIDTH_20;
	dl_ru_grp->tf.tb_ppdu_bw = CHANNEL_WIDTH_20;
	dl_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

	ul_ru_grp->ppdu_bw = CHANNEL_WIDTH_20;
	ul_ru_grp->tf_rate = RTW_DATA_RATE_OFDM24;
	ul_ru_fix_grp->gi_ltf = RTW_GILTF_LGI_4XHE32;

	phl_bw20_init_8ru_pos(phl);
}
#endif

void rtw_phl_check_ru_group(_adapter *padapter){

	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	if (padapter->netif_up == _FALSE) {
		RTW_INFO("%s: net device is down.\n", __func__);
		return;
	}

	if (padapter->mlmepriv.hepriv.he_option) {

		dvobj->phl_com->dev_cap.dlul_group_mode = padapter->registrypriv.wifi_mib.ofdma_enable;

		/* dynamic group */
		if(padapter->registrypriv.wifi_mib.ofdma_enable & dynamic_grp_dlul)
		{
			if (rugrptable->ru_ctrl.ulmacid_cfg &&
				!(padapter->registrypriv.wifi_mib.ofdma_enable & disable_dynamic_grp_ul) &&
				(rugrptable->ru_ctrl.tbl_exist & BIT4))
			{
				rtw_set_ru_ulmacid_cfg(padapter, 1);
			}

			if (!rugrptable->ru_ctrl.tbl_exist){
				if (padapter->dynamic_grp_inited) {
					ru_ctrl->GRP_DL_ON = 1;
					ru_ctrl->GRP_UL_ON = 1;

					rtw_remove_grp(padapter);

					padapter->dynamic_grp_inited = 0;
				}
			} else {
				if (!padapter->dynamic_grp_inited) {
					/* For DL OFDMA */
					if (!(padapter->registrypriv.wifi_mib.ofdma_enable & disable_dynamic_grp_dl) &&
						(rugrptable->ru_ctrl.tbl_exist & BIT0))
					{
						padapter->no_rts = 1;
						rugrptable->ru_ctrl.tx_phase = 1;

						/* ca00 bit[4]  0: zero padding; 1: EOF Padding */
						rtw_phl_write8(padapter->dvobj->phl, 0xca00, 0x0);
						/* TCP ack trigger with data */
						rtw_phl_write8(padapter->dvobj->phl, 0xc7f2, 0x2);
						rtw_phl_write8(padapter->dvobj->phl, 0xc7f3, 0x40);
					}

					/* For UL OFDMA */
					if (!(padapter->registrypriv.wifi_mib.ofdma_enable & disable_dynamic_grp_ul) &&
						(rugrptable->ru_ctrl.tbl_exist & BIT4))	// Mark.CS_update
					{
						/* improve BB RX */
						rtw_phl_write32(padapter->dvobj->phl, 0x1429c, 0x51cc20);
						/* 0x10:recovery; 0x20:ul ra; 0x40:rate fall back 1 */
						rtw_phl_write32(padapter->dvobj->phl, 0x12250, 0x20);
						/* pwr module enable */
						//rtw_phl_write32(padapter->dvobj->phl, 0x12254, 0x100);
						rtw_hw_set_edca(padapter, 0, 0x1010);
					}

					padapter->dynamic_grp_inited = 1;
				}
			}

			rtw_phl_grp_watchdog_callback(padapter->dvobj->phl, padapter->phl_role);
		}

		if (padapter->registrypriv.wifi_mib.ofdma_enable & dl_fix_mode){
			rtw_establish_dl_grp(padapter);
		}

		if (padapter->registrypriv.wifi_mib.ofdma_enable & ul_fix_mode){
			rtw_establish_ul_grp(padapter);
			if (rugrptable->ru_ctrl.ulmacid_cfg) {
				rtw_set_ru_ulmacid_cfg(padapter, 1);
			}
		}

		if (padapter->registrypriv.wifi_mib.ofdma_enable == 0){
			ru_ctrl->GRP_DL_ON = 1;
			ru_ctrl->GRP_UL_ON = 1;
			rtw_remove_grp(padapter);
		}
	}
}

void rtw_core_mac_set_ru_fwc2h_en(_adapter *adapter, u16 ru_fwc2h_en, u16 intvl)
{
	void *phl = adapter->dvobj->phl;
	struct rtw_phl_mac_ax_ru_fwc2h_en info;

	info.en = ru_fwc2h_en;
	info.intvl_ms = intvl;

	rtw_phl_mac_set_ru_fwc2h_en(phl, &info);
}

void rtw_ofdma_info_check(void *context)
{
	_adapter *padapter = (_adapter *)context;
	struct dvobj_priv *drv_priv = adapter_to_dvobj(padapter);
	void *phl = padapter->dvobj->phl;

	//printk("%s:\n", __func__);

	rtw_phl_grp_cal_rtw_stats_info(phl, padapter->phl_role);

	_set_timer(&padapter->ofdma_timer, 1000);

	return;
}

void rtw_init_ofdma_timer(_adapter *adapter)
{
	struct beamforming_info	*info;

	rtw_init_timer(&adapter->ofdma_timer, rtw_ofdma_info_check, adapter);
	//_set_timer(&adapter->ofdma_timer, 1000);
}

void rtw_fini_ofdma_timer(_adapter *adapter)
{
	u8 _cancelled = 0;
	_cancel_timer(&adapter->ofdma_timer, &_cancelled);
}

#endif /* CONFIG_WFA_OFDMA_Logo_Test */
