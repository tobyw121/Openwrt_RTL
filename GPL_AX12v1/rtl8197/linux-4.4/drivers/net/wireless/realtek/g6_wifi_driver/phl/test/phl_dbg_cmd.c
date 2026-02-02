/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
 * Author: vincent_fann@realtek.com
 *
 *****************************************************************************/
#define _PHL_DBG_CMD_C_
#include "phl_dbg_cmd.h"
#include "phl_ps_dbg_cmd.h"
#include "hal_api.h"
#include "../phl_api_drv.h"
/*
 * proc debug command of core
 */
enum PHL_DBG__CORE_CMD_ID {
	PHL_DBG_CORE_HELP,
	PHL_DBG_CORE_GIT_INFO
};

static const struct phl_dbg_cmd_info phl_dbg_core_cmd_i[] = {
	{"git_info", PHL_DBG_CORE_GIT_INFO}
};

void phl_dbg_git_info(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
/* #REMOVE BEGIN */
#if CONFIG_GEN_GIT_INFO
#include "../phl_git_info.h"

	u32 used = 0;

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"\ncore_ver   : %s\n", RTK_CORE_TAGINFO);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"phl_ver    : %s\n", RTK_PHL_TAGINFO);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"halmac_ver : %s\n", RTK_HALMAC_TAGINFO);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"fw_ver     : %s\n", RTK_FW_TAGINFO);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"halbb_ver  : %s\n", RTK_HALBB_TAGINFO);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"halrf_ver  : %s\n", RTK_HALRF_TAGINFO);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"btc_ver    : %s\n", RTK_BTC_TAGINFO);

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"\ncore_sha1  : %s\n", RTK_CORE_SHA1);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"phl_sha1   : %s\n", RTK_PHL_SHA1);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"halmac_sha1: %s\n", RTK_HALMAC_SHA1);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"fw_sha1    : %s\n", RTK_FW_SHA1);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"halbb_sha1 : %s\n", RTK_HALBB_SHA1);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"halrf_sha1 : %s\n", RTK_HALRF_SHA1);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"btc_sha1   : %s\n", RTK_BTC_SHA1);
#endif /* CONFIG_GEN_GIT_INFO */
/* #REMOVE END */
}

void phl_dbg_core_cmd_parser(void *phl, char input[][MAX_ARGV],
		        u32 input_num, char *output, u32 out_len)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	u8 id = 0;
	u32 i;
	u32 used = 0;
	u32 phl_ary_size = sizeof(phl_dbg_core_cmd_i) /
			   sizeof(struct phl_dbg_cmd_info);

	if (phl_ary_size == 0)
		return;

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used, "\n");
	/* Parsing Cmd ID */
	if (input_num) {
		for (i = 0; i < phl_ary_size; i++) {
			if (_os_strcmp(phl_dbg_core_cmd_i[i].name, input[0]) == 0) {
				id = phl_dbg_core_cmd_i[i].id;
				PHL_INFO("[%s]===>\n", phl_dbg_core_cmd_i[i].name);
				break;
			}
		}
		if (i == phl_ary_size) {
			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "PHL CMD not found!\n");
			return;
		}
	}

	switch (id) {
	case PHL_DBG_CORE_HELP:
	{
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				 "phl_dbg_core_cmd_parser : PHL_DBG_CORE_HELP \n");
		for (i = 0; i < phl_ary_size; i++)
			PHL_DBG_MON_INFO(out_len, used, output + used,
					 out_len - used, "%-5d: %s\n",
			          (int)i, phl_dbg_core_cmd_i[i].name);

	}
	break;
	case PHL_DBG_CORE_GIT_INFO:
	{
		phl_dbg_git_info(phl_info, input, input_num, output, out_len);
	}
	break;
	default:
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "[DBG] Do not support this command\n");
		break;
	}
}

s32
phl_dbg_core_proc_cmd(struct phl_info_t *phl_info,
		 char *input, char *output, u32 out_len)
{
	char *token;
	u32 argc = 0;
	char argv[MAX_ARGC][MAX_ARGV];

	do {
		token = _os_strsep(&input, ", ");
		if (token) {
			if (_os_strlen((u8 *)token) <= MAX_ARGV)
				_os_strcpy(argv[argc], token);

			argc++;
		} else {
			break;
		}
	} while (argc < MAX_ARGC);

	phl_dbg_core_cmd_parser(phl_info, argv, argc, output, out_len);

	return 0;
}

enum rtw_phl_status
rtw_phl_dbg_core_cmd(struct phl_info_t *phl_info,
		     struct rtw_proc_cmd *incmd,
		     char *output,
		     u32 out_len)
{
	if (incmd->in_type == RTW_ARG_TYPE_BUF) {
		phl_dbg_core_proc_cmd(phl_info, incmd->in.buf, output, out_len);
	} else if(incmd->in_type == RTW_ARG_TYPE_ARRAY){
		phl_dbg_core_cmd_parser(phl_info, incmd->in.vector,
				   incmd->in_cnt_len, output, out_len);
	}
	return RTW_PHL_STATUS_SUCCESS;
}

#ifdef CONFIG_PHL_TEST_SUITE
bool
_is_hex_digit(char ch_tmp)
{
	if( (ch_tmp >= '0' && ch_tmp <= '9') ||
		(ch_tmp >= 'a' && ch_tmp <= 'f') ||
		(ch_tmp >= 'A' && ch_tmp <= 'F') ) {
		return true;
	} else {
		return false;
	}
}


u32
_map_char_to_hex_digit(char ch_tmp)
{
	if(ch_tmp >= '0' && ch_tmp <= '9')
		return (ch_tmp - '0');
	else if(ch_tmp >= 'a' && ch_tmp <= 'f')
		return (10 + (ch_tmp - 'a'));
	else if(ch_tmp >= 'A' && ch_tmp <= 'F')
		return (10 + (ch_tmp - 'A'));
	else
		return 0;
}


bool
_get_hex_from_string(char *szstr, u32 *val)
{
	char *sz_scan = szstr;

	/* Check input parameter.*/
	if (szstr == NULL || val == NULL) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			"%s: Invalid inpur argumetns! szStr: %p, pu4bVal: %p \n",
			 __FUNCTION__, szstr, val);
		return false;
	}

	/* Initialize output. */
	*val = 0;

	/* Skip leading space. */
	while(*sz_scan != '\0' && (*sz_scan == ' ' || *sz_scan == '\t')) {
		sz_scan++;
	}

	/* Skip leading '0x' or '0X'. */
	if (*sz_scan == '0' && (*(sz_scan+1) == 'x' || *(sz_scan+1) == 'X')) {
		sz_scan += 2;
	}

	if (!_is_hex_digit(*sz_scan)) {
		return false;
	}

	do {
		(*val) <<= 4;
		*val += _map_char_to_hex_digit(*sz_scan);

		sz_scan++;
	} while (_is_hex_digit(*sz_scan));

	return true;
}

bool
_get_dec_from_string(char *szstr, u32 *val)
{
	char *sz_scan = szstr;
	u32 v = 0;

	/* Check input parameter.*/
	if (szstr == NULL || val == NULL) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			"%s: Invalid inpur argumetns! szStr: %p, pu4bVal: %p \n",
			 __FUNCTION__, szstr, val);
		return false;
	}

	/* Skip leading space. */
	while(*sz_scan != '\0' && (*sz_scan == ' ' || *sz_scan == '\t')) {
		sz_scan++;
	}

	while ((*sz_scan >= '0') && (*sz_scan <= '9')) {
		v *= 10;
		v += (*sz_scan - '0');
		sz_scan++;
	}

	/* Failed if break for non-null */
	if (*sz_scan != 0)
		return false;

	*val = v;

	return true;
}

void
_phl_dbg_cmd_switch_chbw(struct phl_info_t *phl_info, char input[][MAX_ARGV],
			 u32 input_num, char *output, u32 out_len)
{
	u32 band = 0;
	u32 bw = 0;
	u32 offset = 0;
	u32 ch = 36;
	u32 used = 0;
	struct rtw_chan_def chdef = {0};

	do {
		if (input_num < 5){
			PHL_DBG_MON_INFO(out_len, used, output + used,
					 out_len - used,
					 "\n[DBG] echo phl set_ch [band(0,1)] [ch(hex)] [bw(0,1,2,3)] [offset(0,1,2,3)]\n");
			break;
		}

		if (!_get_hex_from_string(input[1], &band))
			break;

		if (band > 1)
			break;

		if (!_get_hex_from_string(input[2], &ch))
			break;


		if (!_get_hex_from_string(input[3], &bw))
			break;


		if (!_get_hex_from_string(input[4], &offset))
			break;

		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used,
				 "\n[DBG] PHL_DBG_SET_CH_BW ==> band = %d\n",
				 (int)band);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used,
				 "[DBG] PHL_DBG_SET_CH_BW ==> ch = %d\n",
				 (int)ch);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used,
				 "[DBG] PHL_DBG_SET_CH_BW ==> bw = %d\n",
				 (int)bw);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used,
				 "[DBG] PHL_DBG_SET_CH_BW ==> offset = %d\n",
				 (int)offset);

		chdef.chan = (u8)ch;
		chdef.band = rtw_phl_get_band_type(chdef.chan);
		chdef.bw = (enum channel_width)bw;
		chdef.offset = (enum chan_offset)offset;

		rtw_hal_set_ch_bw(phl_info->hal, (u8)band, &chdef, false, false, false);

	} while (0);
}

void _dump_wifi_role(struct phl_info_t *phl_info, char *output, u32 out_len)
{
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_phl_stainfo_t *sta_info = NULL;
	u32 used = 0;
	u8 j = 0;
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"==> PHL_DBG_DUMP_WROLE CH/BW information\n");
	for( j = 0; j < MAX_WIFI_ROLE_NUMBER; j++) {
		wrole = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, j);
		if (NULL == wrole)
			continue;
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "idx = 0x%x \n", j);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "active = %u \n",
				 wrole->active);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "type = 0x%x \n",
				 wrole->type);
		/* debug_dump_mac_addr(wrole->mac_addr); */
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "chandef:\n\tbw = 0x%x \n",
				 wrole->chandef.bw);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "\tband = 0x%x \n",
				 wrole->chandef.band);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "\tcenter_ch = 0x%x \n",
				 wrole->chandef.center_ch);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "\tchan = 0x%x \n",
				 wrole->chandef.chan);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "\tcenter_freq1 = %u \n",
				 (int)wrole->chandef.center_freq1);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "\tcenter_freq2 = %u \n",
				 (int)wrole->chandef.center_freq2);
		PHL_DBG_MON_INFO(out_len, used, output + used,
				 out_len - used, "\toffset = 0x%x \n",
				 wrole->chandef.offset);

		sta_info = rtw_phl_get_stainfo_self(phl_info, wrole);
		if (NULL != sta_info) {
			PHL_DBG_MON_INFO(out_len, used, output + used,
					 out_len - used, "macid = 0x%x \n",
					 sta_info->macid);
			PHL_DBG_MON_INFO(out_len, used, output + used,
					 out_len - used, "aid = 0x%x \n",
					 sta_info->aid);
			PHL_DBG_MON_INFO(out_len, used, output + used,
					 out_len - used,
					 "MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
					 sta_info->mac_addr[0],
					 sta_info->mac_addr[1],
					 sta_info->mac_addr[2],
					 sta_info->mac_addr[3],
					 sta_info->mac_addr[4],
					 sta_info->mac_addr[5]);
			PHL_DBG_MON_INFO(out_len, used, output + used,
					 out_len - used, "wmode = 0x%x \n",
					 sta_info->wmode);
		}
		PHL_DBG_MON_INFO(out_len, used, output + used,
					out_len - used, "-----------------------------\n");
	}
}

void _dump_rx_rate(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
	u32 ctrl = 0;
	u32 used = 0;
	struct rtw_stats *rx_stat = &phl_info->phl_com->phl_stats;
	if(input_num < 2)
		return;
	_get_hex_from_string(input[1], &ctrl);

	if (ctrl == 2) {
		/*TODO: Clear Counter*/
		return;
	}

	PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "\nOFDM 6M = %d ; OFDM 24M = %d ; OFDM 54M = %d\n",
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_OFDM6],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_OFDM24],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_OFDM54]);

	PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "VHT 1SS\
			\nMCS0 = %d ; MCS1 = %d ; MCS2 = %d ;\
			\nMCS3 = %d ; MCS4 = %d ; MCS5 = %d ;\
			\nMCS6 = %d ; MCS7 = %d ; MCS8 = %d ;\
			\nMCS9 = %d ;\n",
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS0],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS1],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS2],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS3],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS4],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS5],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS6],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS7],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS8],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS1_MCS9]
			);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "VHT 2SS\
			\nMCS0 = %d ; MCS1 = %d ; MCS2 = %d ;\
			\nMCS3 = %d ; MCS4 = %d ; MCS5 = %d ;\
			\nMCS6 = %d ; MCS7 = %d ; MCS8 = %d ;\
			\nMCS9 = %d ;\n",
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS0],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS1],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS2],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS3],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS4],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS5],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS6],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS7],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS8],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_VHT_NSS2_MCS9]
			);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "HE 1SS\
			\nMCS0 = %d ; MCS1 = %d ; MCS2 = %d ;\
			\nMCS3 = %d ; MCS4 = %d ; MCS5 = %d ;\
			\nMCS6 = %d ; MCS7 = %d ; MCS8 = %d ;\
			\nMCS9 = %d ; MCS10 = %d ; MCS11 = %d;\n",
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS0],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS1],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS2],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS3],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS4],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS5],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS6],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS7],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS8],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS9],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS10],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS1_MCS11]);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "HE 2SS\
			\nMCS0 = %d ; MCS1 = %d ; MCS2 = %d ;\
			\nMCS3 = %d ; MCS4 = %d ; MCS5 = %d ;\
			\nMCS6 = %d ; MCS7 = %d ; MCS8 = %d ;\
			\nMCS9 = %d ; MCS10 = %d ; MCS11 = %d ;\n",
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS0],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS1],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS2],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS3],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS4],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS5],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS6],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS7],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS8],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS9],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS10],
			(int)rx_stat->rx_rate_nmr[RTW_DATA_RATE_HE_NSS2_MCS11]
			);

}

#ifdef DEBUG_PHL_RX
void _dump_phl_rx(struct phl_info_t *phl_info, char *output, u32 out_len)
{
	u32 used = 0;
	struct phl_rx_stats *rx_stats = &phl_info->phl_com->rx_stats;

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rx_isr = %u\n", rx_stats->rx_isr);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"phl_rx = %u\n", rx_stats->phl_rx);

#ifdef CONFIG_PCI_HCI
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rdu = %u\n", rtw_phl_get_hw_cnt_rdu(phl_info));

	do {
		struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
		struct rtw_rx_buf_ring *rx_buf_ring;
		u8 ch;

		for (ch = 0; ch < hci_info->total_rxch_num; ch++) {
			u16 rxcnt, host_idx, hw_idx;
			rxcnt = rtw_hal_rx_res_query(phl_info->hal, ch, &host_idx, &hw_idx);
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"rxbd[%d]: host_idx = %3u, hw_idx = %3u, num = %3u,avail = %u\n",
				ch, host_idx, hw_idx, rtw_hal_get_rxbd_num(phl_info->hal, ch), rxcnt);
		}

		rx_buf_ring = (struct rtw_rx_buf_ring *)phl_info->hci->rxbuf_pool;

		for (ch = 0; ch < hci_info->total_rxch_num; ch++) {
#ifdef CONFIG_DYNAMIC_RX_BUF
			if (ch == 0)
				PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					"rxbuf_pool[%d]: idle_rxbuf_cnt = %3u, busy_rxbuf_cnt = %3u, empty_rxbuf_cnt = %u\n",
					ch, rx_buf_ring[ch].idle_rxbuf_cnt, rx_buf_ring[ch].busy_rxbuf_cnt,
					rx_buf_ring[ch].empty_rxbuf_cnt);
			else
#endif
				PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					"rxbuf_pool[%d]: idle_rxbuf_cnt = %3u, busy_rxbuf_cnt = %3u\n",
					ch, rx_buf_ring[ch].idle_rxbuf_cnt, rx_buf_ring[ch].busy_rxbuf_cnt);
		}
	} while (0);

	#if defined(PCIE_TRX_MIT_EN)
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			 "fixed_mitigation: %u\n",
			 phl_info->hci->fixed_mitigation);
	#endif
#endif /* CONFIG_PCI_HCI */

	#if defined(CONFIG_DYNAMIC_RX_BUF) && defined(RTW_RX_CPU_BALANCE)
	do {
		u32 cpu_id;
		u32 *refill_cpu = phl_info->phl_com->rx_stats.refill_cpu;
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				 "Refill CPU: ");
		for (cpu_id = 0; cpu_id < CONFIG_NR_CPUS; cpu_id++)
			PHL_DBG_MON_INFO(out_len, used, output + used,
			                 out_len - used,
					 "  [%u]:%u", cpu_id,
					 refill_cpu[cpu_id]);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				 "\n");
	} while (0);
	#endif /* CONFIG_DYNAMIC_RX_BUF && RTW_RX_CPU_BALANCE */

	do {
		struct rtw_phl_rx_ring *ring = &phl_info->phl_rx_ring;
		u16 wptr, rptr, ring_res;
#if 0
		wptr = (u16)_os_atomic_read(phl_to_drvpriv(phl_info), &ring->phl_idx);
		rptr = (u16)_os_atomic_read(phl_to_drvpriv(phl_info), &ring->core_idx);
#else
		wptr = ring->phl_idx;
		rptr = ring->core_idx;
#endif
		ring_res = phl_calc_avail_rptr(rptr, wptr, MAX_PHL_RING_ENTRY_NUM);

		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			"phl_rx_ring: phl_idx = %d, core_idx = %d, num = %d, rx_pkt_num = %d\n",
			wptr, rptr, MAX_PHL_RING_ENTRY_NUM, ring_res);
	} while (0);

	do {
		struct phl_rx_pkt_pool *rx_pkt_pool;

		rx_pkt_pool = (struct phl_rx_pkt_pool *)phl_info->rx_pkt_pool;
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			"phl_rx_pkt_pool: total = %d, idle_cnt = %d\n",
			MAX_PHL_RING_RX_PKT_NUM, rx_pkt_pool->idle_cnt);
	} while (0);

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rx cnt: check %u ready = %u\n",
		rx_stats->rx_chk_cnt, rx_stats->rx_cnt);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rx_type: all = %u, wifi = %u, wp = %u, ppdu = %u, c2h = %u\n",
		rx_stats->rx_type_all,
		rx_stats->rx_type_wifi, rx_stats->rx_type_wp,
		rx_stats->rx_type_ppdu, rx_stats->rx_type_c2h);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"amsdu = %u\n", rx_stats->rx_amsdu);
	#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"amsdu cut = %u\n", rx_stats->rx_amsdu_cut);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"hdr conv = %u\n", rx_stats->rx_hdr_conv);
	#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"idle_rxbuf_empty = %u, idle_rxbuf_less = %u\n",
		rx_stats->idle_rxbuf_empty, rx_stats->idle_rxbuf_less);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"drop@get_single_rx = %u (rx_rdy = %u, rxbd = %u)\n",
		rx_stats->rx_drop_get, rx_stats->rx_rdy_fail, rx_stats->rxbd_fail);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		#ifdef CONFIG_RTW_DEBUG_RX_CACHE
		"drop@reorder = %u (seq_less = %u, dup = %u, TA = %u, MACID no STA = %u)\n",
		rx_stats->rx_drop_reorder,
		rx_stats->reorder_seq_less, rx_stats->reorder_dup,
		rx_stats->ta_mismatch, rx_stats->macid_no_sta);
		#else /* CONFIG_RTW_DEBUG_RX_CACHE */
		"drop@reorder = %u (seq_less = %u, dup = %u)\n",
		rx_stats->rx_drop_reorder,
		rx_stats->reorder_seq_less, rx_stats->reorder_dup);
		#endif /* CONFIG_RTW_DEBUG_RX_CACHE */
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"drop@handle_rx_frame_list = %u\n",
		rx_stats->rx_drop_no_res);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"reorder: dont = %u, put = %u (total = %u) (indic = %u)\n",
		rx_stats->rx_dont_reorder, rx_stats->rx_put_reorder,
		rx_stats->rx_dont_reorder + rx_stats->rx_put_reorder,
		rx_stats->reorder_indicate);
#ifdef PHL_RXSC_AMPDU
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"ampdu: orig = %u, rxsc = %u (add = %u)\n",
		rx_stats->rxsc_ampdu[0], rx_stats->rxsc_ampdu[1],
		rx_stats->rxsc_ampdu[2]);
#endif
#ifdef CONFIG_DYNAMIC_RX_BUF
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rxbuf_empty = %u, rxbd_refill_fail = %u\n",
		rx_stats->rxbuf_empty, rx_stats->rxbd_refill_fail);
#endif

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"cnt_rx_pktsz  = %u\n", phl_info->phl_com->cnt_rx_pktsz);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rx_pktsz      = %u\n", rx_stats->rx_pktsz_phl);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rx_pktsz_core = %u\n", rx_stats->rx_pktsz_core);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rx_pkt        = %u\n", rx_stats->rx_type_wifi);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"rx_pkt_core   = %u\n", rx_stats->rx_pkt_core);

}

#if defined(CONFIG_PCI_HCI) && defined(PCIE_TRX_MIT_EN)
void phl_dbg_cmd_int_mit(struct phl_info_t *phl_info, char input[][MAX_ARGV],
				u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;

	if (input_num == 2) {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				 "RX interrupt mitigation is %s!\n",
				 phl_info->hci->fixed_mitigation ? "fixed" : "dynamic");
	} else if (input_num == 3) {
		u32 val;

		if (_get_dec_from_string(input[2], &val) == false) {
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					 "\"%s\" is not number!\n", input[2]);
		} else {
			phl_info->hci->fixed_mitigation = val ? 0 : 1;
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					 "Set RX interrupt RX mitigation %s!\n",
					 phl_info->hci->fixed_mitigation ? "fixed" : "dynamic");
		}
	} else if (input_num == 4) {
		u32 timeout, count;
		bool ret;

		ret = _get_dec_from_string(input[2], &count);
		if (ret)
			ret = _get_dec_from_string(input[3], &timeout);

		if (ret == false) {
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					 "\"%s\" or \"%s\" is not number!\n", input[2], input[3]);
		} else {
			phl_info->hci->fixed_mitigation = 1;
			phl_pcie_trx_mit(phl_info, 0, 0, timeout, count, 1);
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					 "Set RX interrupt mitigation as #%u and timeout %uus!\n",
					 count, timeout);
		}
	}
}
#endif /* CONFIG_PCI_HCI && PCIE_TRX_MIT_EN */

void phl_dbg_cmd_phl_rx(struct phl_info_t *phl_info, char input[][MAX_ARGV],
				u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;

	if (1 == input_num) {
		_dump_phl_rx(phl_info, output, out_len);
	#if defined(CONFIG_PCI_HCI) && defined(PCIE_TRX_MIT_EN)
	} else if (!_os_strcmp(input[1], "int_mit")) {
		phl_dbg_cmd_int_mit(phl_info, input, input_num, output, out_len);
	#endif /* CONFIG_PCI_HCI && PCIE_TRX_MIT_EN */
	} else if (!_os_strcmp(input[1], "clear")) {
		_os_mem_set(phl_to_drvpriv(phl_info), &phl_info->phl_com->rx_stats,
			0, sizeof(phl_info->phl_com->rx_stats));
	} else if (!_os_strcmp(input[1], "cnt_rx_pktsz")) {
		int value;
		if (input_num > 2 && _os_sscanf(input[2], "%d", &value) == 1)
			phl_info->phl_com->cnt_rx_pktsz = value;
	} else if (!_os_strcmp(input[1], "sched_phl_rx")) {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			"Schedule PHL RX!\n");
		rtw_phl_start_rx_process(phl_info);
	} else if (!_os_strcmp(input[1], "sched_core_rx")) {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			"Schedule Core RX!\n");
		_phl_indic_new_rxpkt(phl_info);
	}
}
#endif /* DEBUG_PHL_RX */


#if defined(RTW_RX_CPU_BALANCE) || defined(RTW_TX_CPU_BALANCE)
static void _dbg_handler_cpu(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
	static const char *handler_name[] = {"TX handler",
					     "RX handler",
					     "Event handler"};
	u32 handler_id = 0;
	u32 cpu_id = 0;
	u8 *cpu_id_lst[3];
	char handler_type[3];
	int i;

	cpu_id_lst[0] =   (phl_info->phl_tx_handler.type == RTW_PHL_HANDLER_PRIO_LOW)
			? &phl_info->phl_tx_handler.os_handler.u.workitem.cpu_id
			: &phl_info->phl_tx_handler.os_handler.u.tasklet.cpu_id;
	cpu_id_lst[1] =   (phl_info->phl_rx_handler.type == RTW_PHL_HANDLER_PRIO_LOW)
			? &phl_info->phl_rx_handler.os_handler.u.workitem.cpu_id
			: &phl_info->phl_rx_handler.os_handler.u.tasklet.cpu_id;
	cpu_id_lst[2] =   (phl_info->phl_event_handler.type == RTW_PHL_HANDLER_PRIO_LOW)
			? &phl_info->phl_event_handler.os_handler.u.workitem.cpu_id
			: &phl_info->phl_event_handler.os_handler.u.tasklet.cpu_id;
	handler_type[0] = phl_info->phl_tx_handler.type;
	handler_type[1] = phl_info->phl_rx_handler.type;
	handler_type[2] = phl_info->phl_event_handler.type;

	if (input_num < 3) {
		for (i = 0; i < ARRAY_SIZE(handler_name); i++) {
			PHL_PRINT("%d - %s: %c@%u\n", i, handler_name[i],
			          (  (   handler_type[i]
			              == RTW_PHL_HANDLER_PRIO_LOW)
			           ? 'W'
			           : 'T'),
			          *cpu_id_lst[i]);
		}
		return;
	}

	_get_hex_from_string(input[1], &handler_id);
	_get_hex_from_string(input[2], &cpu_id);

	if (handler_id >= ARRAY_SIZE(handler_name)) {
		PHL_ERR("Invalid handler ID %u\n", handler_id);
		return;
	}

	*cpu_id_lst[handler_id] = cpu_id;
	PHL_PRINT("Set PHL %s run on CPU %u\n",
	          handler_name[handler_id], *cpu_id_lst[handler_id]);
}
#endif /* defined(RTW_RX_CPU_BALANCE) || defined(RTW_TX_CPU_BALANCE) */


#ifdef CONFIG_RTW_DEBUG_TX_RPT
static void _dbg_tx_rpt(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
	u32 count = 0;

	PHL_PRINT("Enabling TX report...\n");

	if (input_num < 1)
		return;
	_get_hex_from_string(input[1], &count);

	rtw_hal_debug_tx_report(phl_info->hal, count);
}
#endif /* CONFIG_RTW_DEBUG_TX_RPT */

static const char *_get_mac_pwr_st_str(enum rtw_mac_pwr_st st)
{
	switch (st) {
	case RTW_MAC_PWR_OFF:
		return "off";
	case RTW_MAC_PWR_ON:
		return "on";
	case RTW_MAC_PWR_LPS:
		return "lps";
	default:
		return "n/a";
	}
}

static const char *_get_wow_opmode_str(enum rtw_wow_op_mode mode)
{
	switch (mode) {
	case RTW_WOW_OP_PWR_DOWN:
		return "power down";
	case RTW_WOW_OP_DISCONNECT_STBY:
		return "disconnected standby";
	case RTW_WOW_OP_CONNECT_STBY:
		return "connected standby";
	default:
		return "n/a";
	}
}

#ifdef CONFIG_RTW_DEBUG_BCN_TX
static void _dbg_bcn_tx(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
	u32 failure_count = 0;

	PHL_PRINT("Enabling beacon TX FW report...\n");

	if (input_num < 1)
		return;
	_get_hex_from_string(input[1], &failure_count);

	rtw_hal_debug_bcn_tx(phl_info->hal, failure_count);
}
#endif /* CONFIG_RTW_DEBUG_BCN_TX */


#ifdef CONFIG_RTW_DEBUG_RX_SZ
static void _dbg_rx_sz(struct phl_info_t *phl_info)
{
	PHL_PRINT("RX size: Max = %u, Min = %u (%u)\n",
		  phl_info->max_rx_sz, phl_info->min_rx_sz, phl_info->min_rx_type);
	PHL_PRINT("RP size: Max = %u, Min = %u\n", phl_info->max_rp_sz, phl_info->min_rp_sz);
	PHL_PRINT("Max FS/non-FS size: %u / %u\n",
		  phl_info->phl_com->mr_ctrl.hal_com->max_fs_sz,
		  phl_info->phl_com->mr_ctrl.hal_com->max_non_fs_sz);
}
#endif /* CONFIG_RTW_DEBUG_RX_SZ */


/*
enum rtw_hal_status rtw_hal_rf_chl_rfk_trigger(void *hal,
						u8 phy_idx, u8 force)
*/

static void _do_rfk(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	u32 used = 0;

	rtw_hal_rf_chl_rfk_trigger(hal_com, 0, 1);
	PHL_PRINT("Run RFK!\n");
}
void _dump_wow_stats(struct phl_info_t *phl_info, char input[][MAX_ARGV],
	u32 input_num, char *output, u32 out_len)
{
#ifdef CONFIG_WOWLAN
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct phl_wow_stat *wow_stat = &wow_info->wow_stat;
	u32 used = 0;

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					 "\n%-20s : op_mode = %s, func_en = %s, mac pwr = %s, wake_rsn = %s\
					 \n%-20s : keep_alive_en = %d, disc_det_en = %d, arp_en = %d, ndp_en = %d, gtk_en = %d, dot11w_en = %d\
					 \n%-20s : init err = 0x%x, deinit err = 0x%x\
					 \n%-20s : aoac_rpt_fail_cnt = 0x%x\n",
	"[wow information]",
	_get_wow_opmode_str(wow_stat->op_mode), (wow_stat->func_en == 1 ? "yes" : "no"),
	_get_mac_pwr_st_str(wow_stat->mac_pwr), rtw_phl_get_wow_rsn_str(phl_info, wow_stat->wake_rsn),
	"[wow function]",
	wow_stat->keep_alive_en, wow_stat->disc_det_en, wow_stat->arp_en, wow_stat->ndp_en, wow_stat->gtk_en, wow_stat->dot11w_en,
	"[wow error]",
	wow_stat->err.init, wow_stat->err.deinit,
	"[wow aoac]",
	wow_stat->aoac_rpt_fail_cnt);
#endif /* CONFIG_WOWLAN */
}

void phl_dbg_cmd_snd(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
#ifdef CONFIG_PHL_CMD_BF
	enum rtw_phl_status psts = RTW_PHL_STATUS_SUCCESS;
	u32 role_idx;
	u32 ctrl;
	u32 used = 0;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct phl_snd_grp *grp = NULL;
	u8 idx = 0;

	if (input_num < 2)
		return;

	_get_hex_from_string(input[1], &ctrl);
	_get_hex_from_string(input[2], &role_idx);

	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used, "ctrl 0x%x with role_idx 0x%x!!\n",
			 (int)ctrl, (int)role_idx);

	if (1 == ctrl) {
		psts = rtw_phl_sound_start(phl_info, (u8)role_idx, 0, 30,
					   PHL_SND_TEST_F_ONE_TIME|PHL_SND_TEST_F_PASS_STS_CHK|PHL_SND_TEST_F_GRP_EN_BF_FIX|PHL_SND_TEST_F_DISABLE_TP_LIMIT|PHL_SND_TEST_F_ENABLE_MU_SND);
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "SOUND START(once) : wrole %d !!\n", (int)role_idx);
	} else if (2 == ctrl) {
		psts = rtw_phl_sound_abort(phl_info);
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "SOUND ABORT!!\n");
	} else if (3 == ctrl) {
		psts = rtw_phl_sound_start(phl_info, (u8)role_idx, 0, 30,
					   PHL_SND_TEST_F_PASS_STS_CHK|PHL_SND_TEST_F_GRP_EN_BF_FIX|PHL_SND_TEST_F_DISABLE_TP_LIMIT|PHL_SND_TEST_F_ENABLE_MU_SND);
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "SOUND START(loop) : wrole %d !!\n", (int)role_idx);
	} else if (4 == ctrl) { //VHT SU
		u16 macid[4] = { 1, 0, 0, 0};
		struct rtw_wifi_role_t *role = NULL;

		role = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, (u8)role_idx);
		if (NULL != role) {

			psts = rtw_phl_snd_add_grp(phl_info, role, 0, (u16 *)&macid, 1, false, false);
			psts = rtw_phl_snd_add_grp(phl_info, role, 1, (u16 *)&macid, 1, false, false);

			psts = rtw_phl_sound_start_ex(phl_info, (u8)role_idx, 0, 100,
				    PHL_SND_TEST_F_PASS_STS_CHK);

			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "SOUND START(loop), skip group : wrole %d !!\n", (int)role_idx);
		}
	}  else if (5 == ctrl) { //HE SU
		u16 macid[4] = { 1, 0, 0, 0};
		struct rtw_wifi_role_t *role = NULL;

		role = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, (u8)role_idx);
		if (NULL != role) {

			psts = rtw_phl_snd_add_grp(phl_info, role, 0, (u16 *)&macid, 1, true, false);
			psts = rtw_phl_snd_add_grp(phl_info, role, 1, (u16 *)&macid, 1, true, false);

			psts = rtw_phl_sound_start_ex(phl_info, (u8)role_idx, 0, 100,
				    PHL_SND_TEST_F_PASS_STS_CHK);

			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "SOUND START(loop), skip group : wrole %d !!\n", (int)role_idx);
		}
	} else if (6 == ctrl) { //HE MU
		u16 macid[4] = { 1, 2, 0, 0};
		struct rtw_wifi_role_t *role = NULL;

		role = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, (u8)role_idx);
		if (NULL != role) {

			psts = rtw_phl_snd_add_grp(phl_info, role, 0, (u16 *)&macid, 2, true, true);
			psts = rtw_phl_snd_add_grp(phl_info, role, 1, (u16 *)&macid, 2, true, true);

			psts = rtw_phl_sound_start_ex(phl_info, (u8)role_idx, 0, 50,
						PHL_SND_TEST_F_PASS_STS_CHK|
						PHL_SND_TEST_F_GRP_EN_BF_FIX);

		PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "SOUND START(loop), skip group : wrole %d !!\n", (int)role_idx);
		}
	} else if (7 == ctrl) { //HE MU snd once
		u16 macid[4] = { 1, 2, 0, 0};
		struct rtw_wifi_role_t *role = NULL;

		role = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, (u8)role_idx);
		if (NULL != role) {

			psts = rtw_phl_snd_add_grp(phl_info, role, 0, (u16 *)&macid, 2, true, true);
			psts = rtw_phl_snd_add_grp(phl_info, role, 1, (u16 *)&macid, 2, true, true);

			psts = rtw_phl_sound_start_ex(phl_info, (u8)role_idx, 0, 50,
						PHL_SND_TEST_F_ONE_TIME |
					   PHL_SND_TEST_F_PASS_STS_CHK|
					   PHL_SND_TEST_F_GRP_EN_BF_FIX);

		PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "SOUND START(loop), skip group : wrole %d !!\n", (int)role_idx);
		}
	}else if (8 == ctrl) { //VHT MU
		u16 macid[4] = { 1, 2, 0, 0};
		struct rtw_wifi_role_t *role = NULL;

		role = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, (u8)role_idx);
		if (NULL != role) {

			psts = rtw_phl_snd_add_grp(phl_info, role, 0, (u16 *)&macid, 2, false, true);
			psts = rtw_phl_snd_add_grp(phl_info, role, 1, (u16 *)&macid, 2, false, true);

			psts = rtw_phl_sound_start_ex(phl_info, (u8)role_idx, 0, 50,
						PHL_SND_TEST_F_PASS_STS_CHK|PHL_SND_TEST_F_GRP_EN_BF_FIX);

		PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "SOUND START(loop), skip group : wrole %d !!\n", (int)role_idx);
		}
	} else if(9 == ctrl) {
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "DUMP BF Entry in debugview\n");
			rtw_hal_bf_dbg_dump_entry_all(phl_info->hal);
	} else if(0xa== ctrl) {
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "force remove bf entry\n");

			for(idx = 0; idx < MAX_SND_GRP_NUM; idx++) {
				grp = phl_snd_get_grp_byidx(phl_info, idx);
				if (grp != NULL) {
				DBGP("force remove_grp_idx %d\n",idx);
					pstatus = phl_snd_func_remove_grp(phl_info, grp);
					if (pstatus != RTW_PHL_STATUS_SUCCESS) {
						PHL_TRACE(COMP_PHL_SOUND, _PHL_INFO_, "Remove SND GRP[%d] Fail\n", idx);

					}
				}
			}



	} else {
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "SOUND TEST CMD not found!\n");
	}
#endif
}

void _convert_tx_rate(enum hal_rate_mode mode, u8 mcs_ss_idx, char *str, u32 str_len)
{
	switch (mode) {
	case HAL_LEGACY_MODE:
		switch (mcs_ss_idx) {
		case RTW_DATA_RATE_CCK1:	_os_snprintf(str, str_len,"CCK 1"); break;
		case RTW_DATA_RATE_CCK2:	_os_snprintf(str, str_len,"CCK 2"); break;
		case RTW_DATA_RATE_CCK5_5:	_os_snprintf(str, str_len,"CCK 5_5"); break;
		case RTW_DATA_RATE_CCK11:	_os_snprintf(str, str_len,"CCK 11"); break;
		case RTW_DATA_RATE_OFDM6:	_os_snprintf(str, str_len,"OFDM 6"); break;
		case RTW_DATA_RATE_OFDM9:	_os_snprintf(str, str_len,"OFDM 9"); break;
		case RTW_DATA_RATE_OFDM12:	_os_snprintf(str, str_len,"OFDM 12"); break;
		case RTW_DATA_RATE_OFDM18:	_os_snprintf(str, str_len,"OFDM 18"); break;
		case RTW_DATA_RATE_OFDM24:	_os_snprintf(str, str_len,"OFDM 24"); break;
		case RTW_DATA_RATE_OFDM36:	_os_snprintf(str, str_len,"OFDM 36"); break;
		case RTW_DATA_RATE_OFDM48:	_os_snprintf(str, str_len,"OFDM 48"); break;
		case RTW_DATA_RATE_OFDM54:	_os_snprintf(str, str_len,"OFDM 54"); break;
		default:
			_os_snprintf(str, str_len,"Unkown rate(0x%01x)", mcs_ss_idx);
			break;
		}
		break;
	case HAL_HT_MODE:
		_os_snprintf(str, str_len,"MCS%d", mcs_ss_idx);
		break;
	case HAL_VHT_MODE:
	case HAL_HE_MODE:
		_os_snprintf(str, str_len,"%dSS MCS%d",
		             ((mcs_ss_idx & 0x70)>> 4) + 1,
		             (mcs_ss_idx & 0x0F));
		break;
	default:
		_os_snprintf(str, str_len,"Unknown mode(0x%01x)", mode);
		break;
	}
}

void _convert_rx_rate(u32 rx_rate, char *str, u32 str_len)
{
	switch(rx_rate) {
	case RTW_DATA_RATE_CCK1:	_os_snprintf(str, str_len,"CCK 1"); break;
	case RTW_DATA_RATE_CCK2:	_os_snprintf(str, str_len,"CCK 2"); break;
	case RTW_DATA_RATE_CCK5_5:	_os_snprintf(str, str_len,"CCK 5_5"); break;
	case RTW_DATA_RATE_CCK11:	_os_snprintf(str, str_len,"CCK 11"); break;
	case RTW_DATA_RATE_OFDM6:	_os_snprintf(str, str_len,"OFDM 6"); break;
	case RTW_DATA_RATE_OFDM9:	_os_snprintf(str, str_len,"OFDM 9"); break;
	case RTW_DATA_RATE_OFDM12:	_os_snprintf(str, str_len,"OFDM 12"); break;
	case RTW_DATA_RATE_OFDM18:	_os_snprintf(str, str_len,"OFDM 18"); break;
	case RTW_DATA_RATE_OFDM24:	_os_snprintf(str, str_len,"OFDM 24"); break;
	case RTW_DATA_RATE_OFDM36:	_os_snprintf(str, str_len,"OFDM 36"); break;
	case RTW_DATA_RATE_OFDM48:	_os_snprintf(str, str_len,"OFDM 48"); break;
	case RTW_DATA_RATE_OFDM54:	_os_snprintf(str, str_len,"OFDM 54"); break;

	case RTW_DATA_RATE_MCS0:	_os_snprintf(str, str_len,"MCS 0"); break;
	case RTW_DATA_RATE_MCS1:	_os_snprintf(str, str_len,"MCS 1"); break;
	case RTW_DATA_RATE_MCS2:	_os_snprintf(str, str_len,"MCS 2"); break;
	case RTW_DATA_RATE_MCS3:	_os_snprintf(str, str_len,"MCS 3"); break;
	case RTW_DATA_RATE_MCS4:	_os_snprintf(str, str_len,"MCS 4"); break;
	case RTW_DATA_RATE_MCS5:	_os_snprintf(str, str_len,"MCS 5"); break;
	case RTW_DATA_RATE_MCS6:	_os_snprintf(str, str_len,"MCS 6"); break;
	case RTW_DATA_RATE_MCS7:	_os_snprintf(str, str_len,"MCS 7"); break;
	case RTW_DATA_RATE_MCS8:	_os_snprintf(str, str_len,"MCS 8"); break;
	case RTW_DATA_RATE_MCS9:	_os_snprintf(str, str_len,"MCS 9"); break;
	case RTW_DATA_RATE_MCS10:	_os_snprintf(str, str_len,"MCS 10"); break;
	case RTW_DATA_RATE_MCS11:	_os_snprintf(str, str_len,"MCS 11"); break;
	case RTW_DATA_RATE_MCS12:	_os_snprintf(str, str_len,"MCS 12"); break;
	case RTW_DATA_RATE_MCS13:	_os_snprintf(str, str_len,"MCS 13"); break;
	case RTW_DATA_RATE_MCS14:	_os_snprintf(str, str_len,"MCS 14"); break;
	case RTW_DATA_RATE_MCS15:	_os_snprintf(str, str_len,"MCS 15"); break;
	case RTW_DATA_RATE_MCS16:	_os_snprintf(str, str_len,"MCS 16"); break;
	case RTW_DATA_RATE_MCS17:	_os_snprintf(str, str_len,"MCS 17"); break;
	case RTW_DATA_RATE_MCS18:	_os_snprintf(str, str_len,"MCS 18"); break;
	case RTW_DATA_RATE_MCS19:	_os_snprintf(str, str_len,"MCS 19"); break;
	case RTW_DATA_RATE_MCS20:	_os_snprintf(str, str_len,"MCS 20"); break;
	case RTW_DATA_RATE_MCS21:	_os_snprintf(str, str_len,"MCS 21"); break;
	case RTW_DATA_RATE_MCS22:	_os_snprintf(str, str_len,"MCS 22"); break;
	case RTW_DATA_RATE_MCS23:	_os_snprintf(str, str_len,"MCS 23"); break;
	case RTW_DATA_RATE_MCS24:	_os_snprintf(str, str_len,"MCS 24"); break;
	case RTW_DATA_RATE_MCS25:	_os_snprintf(str, str_len,"MCS 25"); break;
	case RTW_DATA_RATE_MCS26:	_os_snprintf(str, str_len,"MCS 26"); break;
	case RTW_DATA_RATE_MCS27:	_os_snprintf(str, str_len,"MCS 27"); break;
	case RTW_DATA_RATE_MCS28:	_os_snprintf(str, str_len,"MCS 28"); break;
	case RTW_DATA_RATE_MCS29:	_os_snprintf(str, str_len,"MCS 29"); break;
	case RTW_DATA_RATE_MCS30:	_os_snprintf(str, str_len,"MCS 30"); break;
	case RTW_DATA_RATE_MCS31:	_os_snprintf(str, str_len,"MCS 31"); break;

	case RTW_DATA_RATE_VHT_NSS1_MCS0:	_os_snprintf(str, str_len,"NSS1_MCS0"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS1:	_os_snprintf(str, str_len,"NSS1_MCS1"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS2:	_os_snprintf(str, str_len,"NSS1_MCS2"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS3:	_os_snprintf(str, str_len,"NSS1_MCS3"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS4:	_os_snprintf(str, str_len,"NSS1_MCS4"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS5:	_os_snprintf(str, str_len,"NSS1_MCS5"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS6:	_os_snprintf(str, str_len,"NSS1_MCS6"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS7:	_os_snprintf(str, str_len,"NSS1_MCS7"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS8:	_os_snprintf(str, str_len,"NSS1_MCS8"); break;
	case RTW_DATA_RATE_VHT_NSS1_MCS9:	_os_snprintf(str, str_len,"NSS1_MCS9"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS0:	_os_snprintf(str, str_len,"NSS2_MCS0"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS1:	_os_snprintf(str, str_len,"NSS2_MCS1"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS2:	_os_snprintf(str, str_len,"NSS2_MCS2"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS3:	_os_snprintf(str, str_len,"NSS2_MCS3"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS4:	_os_snprintf(str, str_len,"NSS2_MCS4"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS5:	_os_snprintf(str, str_len,"NSS2_MCS5"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS6:	_os_snprintf(str, str_len,"NSS2_MCS6"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS7:	_os_snprintf(str, str_len,"NSS2_MCS7"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS8:	_os_snprintf(str, str_len,"NSS2_MCS8"); break;
	case RTW_DATA_RATE_VHT_NSS2_MCS9:	_os_snprintf(str, str_len,"NSS2_MCS9"); break;

	case RTW_DATA_RATE_VHT_NSS3_MCS0:	_os_snprintf(str, str_len,"NSS3_MCS0"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS1:	_os_snprintf(str, str_len,"NSS3_MCS1"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS2:	_os_snprintf(str, str_len,"NSS3_MCS2"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS3:	_os_snprintf(str, str_len,"NSS3_MCS3"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS4:	_os_snprintf(str, str_len,"NSS3_MCS4"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS5:	_os_snprintf(str, str_len,"NSS3_MCS5"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS6:	_os_snprintf(str, str_len,"NSS3_MCS6"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS7:	_os_snprintf(str, str_len,"NSS3_MCS7"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS8:	_os_snprintf(str, str_len,"NSS3_MCS8"); break;
	case RTW_DATA_RATE_VHT_NSS3_MCS9:	_os_snprintf(str, str_len,"NSS3_MCS9"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS0:	_os_snprintf(str, str_len,"NSS4_MCS0"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS1:	_os_snprintf(str, str_len,"NSS4_MCS1"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS2:	_os_snprintf(str, str_len,"NSS4_MCS2"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS3:	_os_snprintf(str, str_len,"NSS4_MCS3"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS4:	_os_snprintf(str, str_len,"NSS4_MCS4"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS5:	_os_snprintf(str, str_len,"NSS4_MCS5"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS6:	_os_snprintf(str, str_len,"NSS4_MCS6"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS7:	_os_snprintf(str, str_len,"NSS4_MCS7"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS8:	_os_snprintf(str, str_len,"NSS4_MCS8"); break;
	case RTW_DATA_RATE_VHT_NSS4_MCS9:	_os_snprintf(str, str_len,"NSS4_MCS9"); break;

	case RTW_DATA_RATE_HE_NSS1_MCS0:	_os_snprintf(str, str_len,"HE_NSS1_MCS0"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS1:	_os_snprintf(str, str_len,"HE_NSS1_MCS1"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS2:	_os_snprintf(str, str_len,"HE_NSS1_MCS2"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS3:	_os_snprintf(str, str_len,"HE_NSS1_MCS3"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS4:	_os_snprintf(str, str_len,"HE_NSS1_MCS4"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS5:	_os_snprintf(str, str_len,"HE_NSS1_MCS5"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS6:	_os_snprintf(str, str_len,"HE_NSS1_MCS6"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS7:	_os_snprintf(str, str_len,"HE_NSS1_MCS7"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS8:	_os_snprintf(str, str_len,"HE_NSS1_MCS8"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS9:	_os_snprintf(str, str_len,"HE_NSS1_MCS9"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS10:	_os_snprintf(str, str_len,"HE_NSS1_MCS10"); break;
	case RTW_DATA_RATE_HE_NSS1_MCS11:	_os_snprintf(str, str_len,"HE_NSS1_MCS11"); break;

	case RTW_DATA_RATE_HE_NSS2_MCS0:	_os_snprintf(str, str_len,"HE_NSS2_MCS0"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS1:	_os_snprintf(str, str_len,"HE_NSS2_MCS1"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS2:	_os_snprintf(str, str_len,"HE_NSS2_MCS2"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS3:	_os_snprintf(str, str_len,"HE_NSS2_MCS3"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS4:	_os_snprintf(str, str_len,"HE_NSS2_MCS4"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS5:	_os_snprintf(str, str_len,"HE_NSS2_MCS5"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS6:	_os_snprintf(str, str_len,"HE_NSS2_MCS6"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS7:	_os_snprintf(str, str_len,"HE_NSS2_MCS7"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS8:	_os_snprintf(str, str_len,"HE_NSS2_MCS8"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS9:	_os_snprintf(str, str_len,"HE_NSS2_MCS9"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS10:	_os_snprintf(str, str_len,"HE_NSS2_MCS10"); break;
	case RTW_DATA_RATE_HE_NSS2_MCS11:	_os_snprintf(str, str_len,"HE_NSS2_MCS11"); break;

	case RTW_DATA_RATE_HE_NSS3_MCS0:	_os_snprintf(str, str_len,"HE_NSS3_MCS0"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS1:	_os_snprintf(str, str_len,"HE_NSS3_MCS1"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS2:	_os_snprintf(str, str_len,"HE_NSS3_MCS2"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS3:	_os_snprintf(str, str_len,"HE_NSS3_MCS3"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS4:	_os_snprintf(str, str_len,"HE_NSS3_MCS4"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS5:	_os_snprintf(str, str_len,"HE_NSS3_MCS5"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS6:	_os_snprintf(str, str_len,"HE_NSS3_MCS6"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS7:	_os_snprintf(str, str_len,"HE_NSS3_MCS7"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS8:	_os_snprintf(str, str_len,"HE_NSS3_MCS8"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS9:	_os_snprintf(str, str_len,"HE_NSS3_MCS9"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS10:	_os_snprintf(str, str_len,"HE_NSS3_MCS10"); break;
	case RTW_DATA_RATE_HE_NSS3_MCS11:	_os_snprintf(str, str_len,"HE_NSS3_MCS11"); break;

	case RTW_DATA_RATE_HE_NSS4_MCS0:	_os_snprintf(str, str_len,"HE_NSS4_MCS0"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS1:	_os_snprintf(str, str_len,"HE_NSS4_MCS1"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS2:	_os_snprintf(str, str_len,"HE_NSS4_MCS2"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS3:	_os_snprintf(str, str_len,"HE_NSS4_MCS3"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS4:	_os_snprintf(str, str_len,"HE_NSS4_MCS4"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS5:	_os_snprintf(str, str_len,"HE_NSS4_MCS5"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS6:	_os_snprintf(str, str_len,"HE_NSS4_MCS6"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS7:	_os_snprintf(str, str_len,"HE_NSS4_MCS7"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS8:	_os_snprintf(str, str_len,"HE_NSS4_MCS8"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS9:	_os_snprintf(str, str_len,"HE_NSS4_MCS9"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS10:	_os_snprintf(str, str_len,"HE_NSS4_MCS10"); break;
	case RTW_DATA_RATE_HE_NSS4_MCS11:	_os_snprintf(str, str_len,"HE_NSS4_MCS11"); break;

	case RTW_DATA_RATE_MAX:			_os_snprintf(str, str_len,"RATE_MAX"); break;
	default:
		_os_snprintf(str, str_len,"Unknown rate(0x%01x)", (unsigned int)rx_rate);
		break;
	}
}

void phl_dbg_cmd_asoc_sta(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
	u32 ctrl = 0, wrole_idx = 0;
	u32 used = 0;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_phl_stainfo_t *n, *psta;
	void *drv = phl_to_drvpriv(phl_info);
	char tx_rate_str[32] = {0};
	char rx_rate_str[32] = {0};

	PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "\nDUMP Associate STA infomations\n");
	if(input_num < 2) {
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "phl asoc_sta [wrole index] [1=dump list basic information]\n");
		return;
	}
	_get_hex_from_string(input[1], &ctrl);
	_get_hex_from_string(input[2], &wrole_idx);
	if (wrole_idx >= MAX_WIFI_ROLE_NUMBER) {
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "Wrole Index shall < 5\n");
		return;
	}
	wrole = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, (u8)wrole_idx);
	if (1 == ctrl) {
		_os_spinlock(drv, &wrole->assoc_sta_queue.lock, _bh, NULL);
		phl_list_for_loop_safe(psta, n, struct rtw_phl_stainfo_t,
			       &wrole->assoc_sta_queue.queue, list) {
			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "\nmac_id 0x%x ; aid 0x%x ; mac addr %02x-%02x-%02x-%02x-%02x-%02x ; AX_Support %d\n",
				(int)psta->macid, (int)psta->aid,
				psta->mac_addr[0], psta->mac_addr[1],
				psta->mac_addr[2], psta->mac_addr[3],
				psta->mac_addr[4], psta->mac_addr[5],
				(psta->wmode&WLAN_MD_11AX) ? 1 : 0);

			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "WROLE-IDX:%d wlan_mode:0x%02x, chan:%d, bw%d\n",
				psta->wrole->id, psta->wmode, psta->chandef.chan, psta->chandef.bw);

			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "[Stats] Tput -[Tx:%d KBits Rx :%d KBits]\n",
				(int)psta->stats.tx_tp_kbits, (int)psta->stats.rx_tp_kbits);
			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "[Stats] RSSI:%d\n",
				psta->hal_sta->rssi_stat.ma_rssi);

			_convert_tx_rate( psta->hal_sta->ra_info.rpt_rt_i.mode,
					psta->hal_sta->ra_info.rpt_rt_i.mcs_ss_idx,
			 		tx_rate_str, 32);
			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "[Stats] Tx Rate :%s\n",
				tx_rate_str);

			_convert_rx_rate(psta->stats.rx_rate, rx_rate_str, 32);
			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "[Stats] Rx Rate :%s\n",
				rx_rate_str);

			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "[Stats] rx_ok_cnt:%d rx_err_cnt:%d\n",
				(int)psta->hal_sta->trx_stat.rx_ok_cnt,
				(int)psta->hal_sta->trx_stat.rx_err_cnt);


		}
		_os_spinunlock(drv, &wrole->assoc_sta_queue.lock, _bh, NULL);
	}
}

#ifdef CONFIG_PCI_HCI
void _dbg_tx_stats_pcie(struct phl_info_t *phl_info, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len, u32 *used,
			u32 ctrl, u32 ch)

{
	struct hci_info_t *hci_info = (struct hci_info_t *)phl_info->hci;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	struct rtw_wp_rpt_stats *rpt_stats = NULL;

	rpt_stats = (struct rtw_wp_rpt_stats *)hal_com->trx_stat.wp_rpt_stats;

	if (ch >= hci_info->total_txch_num) {
		PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
				 "\n Invalid channel number\n");
		return;
	}
	if (0xFF == ctrl) {
		rpt_stats[ch].tx_ok_cnt = 0;
		rpt_stats[ch].rty_fail_cnt = 0;
		rpt_stats[ch].lifetime_drop_cnt = 0;
		rpt_stats[ch].macid_drop_cnt = 0;
		rpt_stats[ch].sw_drop_cnt = 0;
		rpt_stats[ch].recycle_fail_cnt = 0;
		rpt_stats[ch].delay_tx_ok_cnt = 0;
		rpt_stats[ch].delay_rty_fail_cnt = 0;
		rpt_stats[ch].delay_lifetime_drop_cnt = 0;
		rpt_stats[ch].delay_macid_drop_cnt = 0;
		return;
	}

	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"\n== wp report statistics == \n");
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"ch			: %u\n", (int)ch);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"busy count		: %u\n", (int)rpt_stats[ch].busy_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"ok count		: %u\n", (int)rpt_stats[ch].tx_ok_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"retry fail count	: %u\n",
			 (int)rpt_stats[ch].rty_fail_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"lifetime drop count	: %u\n",
			 (int)rpt_stats[ch].lifetime_drop_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"macid drop count	: %u\n",
			 (int)rpt_stats[ch].macid_drop_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"sw drop count		: %u\n",
			 (int)rpt_stats[ch].sw_drop_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"recycle fail count	: %u\n",
			 (int)rpt_stats[ch].recycle_fail_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"delay ok count			: %u\n",
			 (int)rpt_stats[ch].delay_tx_ok_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"delay retry fail count		: %u\n",
			 (int)rpt_stats[ch].delay_rty_fail_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"delay lifetime drop count	: %u\n",
			 (int)rpt_stats[ch].delay_lifetime_drop_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"delay macid drop count		: %u\n",
			 (int)rpt_stats[ch].delay_macid_drop_cnt);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"ltr tx delay count		: %u\n",
			 (int)hal_com->trx_stat.ltr_tx_dly_count);
	PHL_DBG_MON_INFO(out_len, *used, output + *used, out_len - *used,
		"ltr last tx delay time	: %u\n",
			 (int)hal_com->trx_stat.ltr_last_tx_dly_time);
}
#endif

void phl_dbg_ltr_stats(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		       u32 input_num, char *output, u32 out_len)
{
	#ifdef CONFIG_PCI_HCI
	u32 used = 0;
	#ifdef RTW_WKARD_DYNAMIC_LTR
		if (input_num == 3) {
			if (0 == _os_strcmp(input[1], "set")) {
					if (0 == _os_strcmp(input[2], "disable"))
						rtw_hal_ltr_sw_ctrl(phl_info->hal, false);
					else if (0 == _os_strcmp(input[2], "enable"))
						rtw_hal_ltr_sw_ctrl(phl_info->hal, true);
					else
						return;
			}
			return;
		}
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"\nltr sw ctrl   	   : %u\n",
				rtw_hal_ltr_is_sw_ctrl(phl_info->hal) == true ? 1 : 0);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"\nltr current state   : %u\n",
				rtw_hal_ltr_get_cur_state(phl_info->hal));
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"ltr last trigger time : %lu\n",
				rtw_hal_ltr_get_last_trigger_time(phl_info->hal));
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"ltr active trigger cnt: %lu\n",
				rtw_hal_ltr_get_tri_cnt(phl_info->hal, RTW_PCIE_LTR_SW_ACT));
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"ltr idle trigger cnt: %lu\n",
				rtw_hal_ltr_get_tri_cnt(phl_info->hal, RTW_PCIE_LTR_SW_IDLE));
	#else
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"Not Support Dynamical LTR\n");
	#endif
	#endif
}

void phl_dbg_trx_stats(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		       u32 input_num, char *output, u32 out_len)
{
 	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_stats *phl_stats = &phl_com->phl_stats;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
	u32 used = 0;
	u32 ctrl = 0;
	int i;
	#ifdef CONFIG_PCI_HCI
	u32 ch = (-1);

	if (input_num >= 2) {
		u32 tx_ch_num = rtw_phl_get_txch_num(phl_info);
		bool dump_tx_ch = _get_dec_from_string(input[1], &ch);
		u32 _ch;

		if (input_num == 3) {
			dump_tx_ch = dump_tx_ch && _get_hex_from_string(input[2], &ctrl);
		}
		if (!dump_tx_ch || ch >= tx_ch_num) {
			PHL_ERR("TX DMA channel %u is not valid. (%u)\n", ch, tx_ch_num);
			ch = (-1);
		}
	}
	#endif /* CONFIG_PCI_HCI */

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"\nunicast tx bytes	: %llu\n", phl_stats->tx_byte_uni);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"total tx bytes		: %llu\n", phl_stats->tx_byte_total);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		 "tx throughput		: %d(kbps)\n",
			 (int)phl_stats->tx_tp_kbits);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"last tx time		: %d(ms)\n",
			 (int)phl_stats->last_tx_time_ms);

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"tx request num to phl	: %d\n",
			 (int)phl_stats->txreq_num);

	#ifdef CONFIG_PCI_HCI
	if (ch != (-1)) {
		_dbg_tx_stats_pcie(phl_info, input, input_num, output,
		                   out_len, &used, ctrl, ch);
	}
	#endif /* CONFIG_PCI_HCI */

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"\nunicast rx bytes	: %llu\n", phl_stats->rx_byte_uni);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"total rx bytes		: %llu\n", phl_stats->rx_byte_total);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		 "rx throughput		: %d(kbps)\n",
			 (int)phl_stats->rx_tp_kbits);
	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"last rx time		: %d(ms)\n",
			 (int)phl_stats->last_rx_time_ms);

	/* Dump TX Pause status */
	for (i = 0; i < MAX_BAND_NUM; i++) {
		int j;

		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		                 "Band %u TX enable\n", i);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		                 "\tTXEN set: %04X\n", hal_com->band[i].txen_final);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		                 "\tTXEN set time: %u (%u)\n",
		                 hal_com->band[i].txen_set_time,
		                 _os_get_cur_time_ms());

		for (j = 0; j < PAUSE_RSON_MAX; j++) {
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
					 "\ttx_pause[%.2u]   : %04X\n",
					 j, hal_com->band[i].tx_pause[j]);
		}
	}
}

void phl_dbg_cmd_show_rssi(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		 	   u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;
	struct rtw_phl_rssi_stat *rssi_stat = NULL;
	struct rtw_phl_stainfo_t *psta = NULL;
	struct rtw_phl_com_t *phl_com = NULL;
	u32 macid = 0;

	if (NULL == phl_info)
		return;

	rssi_stat = &phl_info->phl_com->rssi_stat;
	phl_com = phl_info->phl_com;

	_os_spinlock(phl_com->drv_priv,
		     &(phl_com->rssi_stat.lock), _bh, NULL);

	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used, "\nShow Moving Average RSSI Statistics Informations\n");

	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "DATA (A1 Match, AddrCam Valid) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_DATA_ACAM_A1M]);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "DATA (A1 Not Match, AddrCam Valid) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_DATA_ACAM]);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "DATA (other) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_DATA_OTHER]);

	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "MGNT (A1 Match, AddrCam Valid) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_MGNT_ACAM_A1M]);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "MGNT (A1 Not Match, AddrCam Valid) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_MGNT_ACAM]);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "MGNT (other) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_MGNT_OTHER]);

	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "CTRL (A1 Match, AddrCam Valid) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_CTRL_ACAM_A1M]);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "CTRL (A1 Not Match, AddrCam Valid) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_CTRL_ACAM]);
	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "CTRL (other) : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_CTRL_OTHER]);

	PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "Unknown : %d\n",
			 rssi_stat->ma_rssi[RTW_RSSI_UNKNOWN]);
	_os_spinunlock(phl_com->drv_priv,
		     &(phl_com->rssi_stat.lock), _bh, NULL);

	if(input_num > 1) {
		_get_hex_from_string(input[1], &macid);
		psta = rtw_phl_get_stainfo_by_macid(phl_info, (u16)macid);
		if (psta != NULL) {
			PHL_DBG_MON_INFO(out_len, used, output + used,
			 out_len - used,
			 "STA macid : %d ; mv_avg_rssi = %d ; asoc_rssi = %d (bb : %d, %d)\n",
			 (int)macid, (int)psta->hal_sta->rssi_stat.ma_rssi,
			 (int)psta->hal_sta->rssi_stat.assoc_rssi,
			 (int)(psta->hal_sta->rssi_stat.rssi >> 1),
			 (int)(psta->hal_sta->rssi_stat.rssi_ma >> 5));
		}
	}

}


void phl_dbg_trx_cfg(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		 	   u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;
	u32 tx_path = 0, rx_path = 0;
	u32 wrole_idx = 0;
	struct rtw_wifi_role_t *wrole = NULL;

	if (input_num < 3) {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"Invalid Input\n");
		return;
	}

	if (NULL == phl_info)
		return;

	_get_hex_from_string(input[1], &wrole_idx);
	if (wrole_idx >= MAX_WIFI_ROLE_NUMBER) {
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "Wrole Index shall < 5\n");
		return;
	}
	wrole = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, (u8)wrole_idx);

	_get_hex_from_string(input[2], &tx_path);
	_get_hex_from_string(input[3], &rx_path);

	rtw_phl_tx_rx_path_cfg(phl_info, wrole,
			(enum rf_path)tx_path, (enum rf_path)rx_path);
}

#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
void _dump_phl_rx_amsdu_cut(struct phl_info_t *phl_info, char *output, u32 out_len)
{
	u32 used = 0;
	struct phl_rx_stats *rx_stats = &phl_info->phl_com->rx_stats;
	struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		"RX A-MSDU CUT:\n"
	        "\tRX A-MSDU cut cap: H%u S%u F%u\n"
	        "\tRX HDR Conv cap: H%u S%u F%u\n"
	        "\tCut #: %u\n"
 	        "\tConv #: %u\n"
	        "\tOK: %u\n"
 	        "\tbreak: %u\n"
	 	"\tunfinished: %u\n"
	 	"\tprocessed pending: %u\n"
	        "\tmaximum count: %u\n"
	        "\tpending A-MSDU: %pX\n"
	        "\tcurrent count: %u\n"
	        "\tchk_drop: %u\n"
	        "\tacc_drop: %u\n",
	        hal_com->dev_hw_cap.rx_amsdu_cut,
	        phl_info->phl_com->dev_sw_cap.rx_amsdu_cut,
	        phl_info->phl_com->dev_cap.rx_amsdu_cut,
	        hal_com->dev_hw_cap.rx_hdr_conv,
	        phl_info->phl_com->dev_sw_cap.rx_hdr_conv,
	        phl_info->phl_com->dev_cap.rx_hdr_conv,
	        rx_stats->rx_amsdu_cut,
	        rx_stats->rx_hdr_conv,
	        rx_stats->amsdu_cut_ok,
	        rx_stats->amsdu_cut_break,
	        rx_stats->amsdu_cut_unfinished,
	        rx_stats->amsdu_cut_proc_pending,
	        rx_stats->amsdu_cut_max_num,
	        phl_info->pending_amsdu,
	        rx_stats->amsdu_cut_cur_num
		#ifdef CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT
	        , rx_stats->amsdu_cut_chk_drop
	        , rx_stats->amsdu_cut_acc_drop
		#endif /* CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT */
	);
}

void phl_dbg_rx_hdr_conv(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		 	   u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;

	if (1 == input_num) {
		_dump_phl_rx_amsdu_cut(phl_info, output, out_len);
	} else if (!_os_strcmp(input[1], "clear")) {
		struct phl_rx_stats *rx_stats = &phl_info->phl_com->rx_stats;
		rx_stats->amsdu_cut_break = 0;
		rx_stats->amsdu_cut_max_num = 0;
		rx_stats->amsdu_cut_ok = 0;
		rx_stats->amsdu_cut_proc_pending = 0;
		rx_stats->amsdu_cut_unfinished = 0;
		rx_stats->rx_amsdu_cut = 0;
		rx_stats->rx_hdr_conv = 0;
		#ifdef CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT
		rx_stats->amsdu_cut_chk_drop = 0;
		rx_stats->amsdu_cut_acc_drop = 0;
		#endif /* CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT */
	} else if (!_os_strcmp(input[1], "enable")) {
		int ret = rtw_phl_enable_rx_hdr_conv(phl_info);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		                 "Enable RX header conversion: %d\n", ret);
	} else if (!_os_strcmp(input[1], "disable")) {
		rtw_phl_disable_rx_hdr_conv(phl_info);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"Disable RX header conversion!\n");
	#ifdef CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT
	} else if (!_os_strcmp(input[1], "dump")) {
		u32 count;
		u32 ret = _get_dec_from_string(input[2], &count);
		if (!ret) {
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			                 "Invalid number \"%s\"!\n", input[2]);
			count = 1;
		}
		phl_info->phl_com->rx_hdr_conv_dump = count;
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"Enable RX header conversion debug dump %u\n",
				phl_info->phl_com->rx_hdr_conv_dump);
	#endif /* CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT */
	} else {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"Unsupported command \"%s\"\n", input[1]);
	}
}

void phl_dbg_rx_amsdu_cut(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		 	   u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;

	if (1 == input_num) {
		_dump_phl_rx_amsdu_cut(phl_info, output, out_len);
	} else if (!_os_strcmp(input[1], "clear")) {
		struct phl_rx_stats *rx_stats = &phl_info->phl_com->rx_stats;
		rx_stats->amsdu_cut_break = 0;
		rx_stats->amsdu_cut_max_num = 0;
		rx_stats->amsdu_cut_ok = 0;
		rx_stats->amsdu_cut_proc_pending = 0;
		rx_stats->amsdu_cut_unfinished = 0;
		#ifdef CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT
		rx_stats->amsdu_cut_chk_drop = 0;
		rx_stats->amsdu_cut_acc_drop = 0;
		#endif /* CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT */
	} else if (!_os_strcmp(input[1], "enable")) {
		rtw_phl_enable_rx_amsdu_cut(phl_info);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		                 "Enable RX A-MSDU cut!\n");
	} else if (!_os_strcmp(input[1], "disable")) {
		rtw_phl_disable_rx_amsdu_cut(phl_info);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"Disable RX A-MSDU cut!\n");
	#ifdef CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT
	} else if (!_os_strcmp(input[1], "dump")) {
		u32 count;
		u32 ret = _get_dec_from_string(input[2], &count);
		if (!ret) {
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			                 "Invalid number \"%s\"!\n", input[2]);
			count = 1;
		}
		phl_info->phl_com->rx_amsdu_cut_dump = count;
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"Enable RX A-MSDU cut debug dump %u\n",
				phl_info->phl_com->rx_amsdu_cut_dump);
	#endif /* CONFIG_RTW_DEBUG_HW_RX_AMSDU_CUT */
	} else {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				"Unsupported command \"%s\"\n", input[1]);
	}
}
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */

static void _phl_switch_lamode(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		      u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;
	u32 mode = 0;
	struct rtw_phl_com_t *phl_com = (struct rtw_phl_com_t *) phl_info->phl_com;

	_get_hex_from_string(input[1], &mode);
	phl_com->dev_cap.la_mode = mode > 0 ? true:false;

	if (phl_com->dev_cap.la_mode){
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used, "enable lamode\n");
	}
	else {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used, "disable lamode\n");
	}
}

extern void
rtw_hal_mac_get_sh_buf(void *hal, u32 strt_addr,
                             u8 *buf, u32 len, u32 dbg_path, u8 type);
static void _dump_mac_shbuf(struct phl_info_t *phl_info)
{
	 u8 *buf = _os_mem_alloc(phl_to_drvpriv(phl_info), 0x10000);
	 int i, sec_num = 7; /* share buffer length 0x70000 */

	 if (buf == NULL) {
		 PHL_ERR("%s: OOM!\n", __FUNCTION__);
		 return;
	 }

	 for (i = 0; i < sec_num; i++) {
		 int offset_start = i * 0x10000;
		 u32 *data = (u32 *)buf;
		 u32 *data_end = data + (0x10000 / sizeof(u32));
		 rtw_hal_mac_get_sh_buf(phl_info->hal, offset_start,
		                        buf, 0x10000, 1, 0);

		 while (data < data_end) {
			 PHL_PRINT("%08X: %08X %08X %08X %08X\n", offset_start,
			           data[0], data[1], data[2], data[3]);
			 data += 4;
			 offset_start += 16;
			 if ((offset_start & 0x1000) == 0)
				 _os_kick_watchdog();
		 }
	 }
	_os_mem_free(phl_to_drvpriv(phl_info), buf, 0x10000);
}

#ifdef CONFIG_RTW_HW_TRX_WATCHDOG
static void _phl_hw_trx_watchdog_cmd(struct phl_info_t *phl_info, char input[][MAX_ARGV],
               		      u32 input_num, char *output, u32 out_len)
{
	u32 used = 0;
	u32 enable = 0;
	struct rtw_phl_com_t *phl_com;

	if (NULL == phl_info)
		return;

	phl_com = phl_info->phl_com;

	if (input_num == 1) {
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
		                 "HW T/RX watchdog: %s\n"
		                 , (phl_com->disable_hw_trx_watchdog ? "Disabled" : "Enabled")
		                );
		if (!phl_com->disable_hw_trx_watchdog) {
			struct trx_watchdog_data *trx_wdg_data = &phl_info->phl_com->mr_ctrl.hal_com->trx_stat.wdg_data;
			PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			                 "\ttrx_wdg_recovery_cnt: %u\n"
			                 "\ttrx_wdg_fail_cnt: %u\n"
			                 "\tfailure_mask: %08X\n"
			                 "\tlast_wifi_pkt_cnt: %u\n"
			                 "\tlast_rx_host_idx: %u\n"
			                 "\tlast_tx_mgnt_hw_idx: %u\n"
			                 "\tlast_tx_hiq_hw_idx: %u\n"
			                 "\tlast_tx_h2c_hw_idx: %u\n"
			                 "\tlast_mac_rx_ofdm: %u\n"
			                 "\tlast_mac_tx_ofdm: %u\n"
			                 , trx_wdg_data->trx_wdg_recovery_cnt
			                 , trx_wdg_data->trx_wdg_fail_cnt
			                 , trx_wdg_data->failure_mask
			                 , trx_wdg_data->last_wifi_pkt_cnt
			                 , trx_wdg_data->last_rx_host_idx
			                 , trx_wdg_data->last_tx_mgnt_hw_idx
			                 , trx_wdg_data->last_tx_hiq_hw_idx
			                 , trx_wdg_data->last_tx_h2c_hw_idx
			                 , trx_wdg_data->last_mac_rx_cck_ofdm
			                 , trx_wdg_data->last_mac_tx_cck_ofdm
			                );
		}
		return;
	}

	_get_hex_from_string(input[1], &enable);
	if (enable) {
		if (phl_com->disable_hw_trx_watchdog)
			PHL_PRINT("Enabling hardware T/RX watchdog.\n");
		phl_com->disable_hw_trx_watchdog = 0;
	} else {
		if (phl_com->disable_hw_trx_watchdog == 0)
			PHL_PRINT("Disabling hardware T/RX watchdog.\n");
		phl_com->disable_hw_trx_watchdog = 1;
	}
}
#endif /* CONFIG_RTW_HW_TRX_WATCHDOG */

void phl_dbg_cmd_parser(struct phl_info_t *phl_info, char input[][MAX_ARGV],
		        u32 input_num, char *output, u32 out_len)
{
	u8 id = 0;
	u32 i;
	u32 used = 0;
	u32 phl_ary_size = sizeof(phl_dbg_cmd_i) /
			   sizeof(struct phl_dbg_cmd_info);

	if (phl_ary_size == 0)
		return;

	PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used, "\n");
	/* Parsing Cmd ID */
	if (input_num) {
		for (i = 0; i < phl_ary_size; i++) {
			if (_os_strcmp(phl_dbg_cmd_i[i].name, input[0]) == 0) {
				id = phl_dbg_cmd_i[i].id;
				PHL_INFO("[%s]===>\n", phl_dbg_cmd_i[i].name);
				break;
			}
		}
		if (i == phl_ary_size) {
			PHL_DBG_MON_INFO(out_len, used, output + used,
				out_len - used, "PHL CMD not found!\n");
			return;
		}
	}

	switch (id) {
	case PHL_DBG_MON_HELP:
	{
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				 "phl_dbg_cmd_parser : PHL_DBG_MON_HELP \n");
		for (i = 0; i < phl_ary_size - 2; i++)
			PHL_DBG_MON_INFO(out_len, used, output + used,
					 out_len - used, "%-5d: %s\n",
			          (int)i, phl_dbg_cmd_i[i + 2].name);

	}
	break;
	case PHL_DBG_MON_TEST:
	{
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
				 "[DBG] PHL_DBG_MON_TEST (SAMPLE) \n");
	}
	break;
	case PHL_DBG_COMP:
	{
		u32 ctrl = 0, comp = 0;
		u32 ret = 0;

		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			"[DBG] PHL_DBG_COMP [1=set, 2=clear, 3=show] [comp_bit] \n");
		if (input_num <= 1)
			break;
		_get_hex_from_string(input[1], &ctrl);
		if (ctrl != 3 && input_num <= 2)
			break;
		_get_hex_from_string(input[2], &comp);

		ret = rtw_phl_dbg_ctrl_comp((u8)ctrl, (u8)comp);
		PHL_DBG_MON_INFO(out_len, used, output + used, out_len - used,
			"[DBG] PHL_DBG_COMP (COMP = 0x%x) \n", (int)ret);
	}
	break;
	case PHL_DBG_DUMP_WROLE:
	{
		_dump_wifi_role(phl_info, output, out_len);
	}
	break;
	case PHL_DBG_SET_CH_BW:
	{
		_phl_dbg_cmd_switch_chbw(phl_info, input, input_num,
					 output, out_len);
	}
	break;
	case PHL_DBG_SHOW_RX_RATE:
	{
		_dump_rx_rate(phl_info, input, input_num, output, out_len);
	}
	break;
#ifdef DEBUG_PHL_RX
	case PHL_DBG_PHL_RX:
	{
		phl_dbg_cmd_phl_rx(phl_info, input, input_num, output, out_len);
	}
	break;
#endif
	case PHL_DBG_SOUND :
	{
		phl_dbg_cmd_snd(phl_info, input, input_num, output, out_len);
	}
	break;
	case PHL_DBG_ASOC_STA:
	{
		phl_dbg_cmd_asoc_sta(phl_info, input, input_num, output, out_len);
	}
	break;
	#ifdef CONFIG_FSM
	case PHL_DBG_FSM:
	{
		phl_fsm_dbg(phl_info, input, input_num, output, out_len);
	}
	break;
	#endif
	case PHL_DBG_TRX_STATS:
	{
		phl_dbg_trx_stats(phl_info, input, input_num, output, out_len);
	}
	break;
	case PHL_DBG_LTR:
	{
		phl_dbg_ltr_stats(phl_info, input, input_num, output, out_len);
	}
	break;
	case PHL_SHOW_RSSI_STAT :
	{
		phl_dbg_cmd_show_rssi(phl_info, input, input_num, output, out_len);
	}
	break;
	case PHL_DBG_SER:
	{
		phl_ser_cmd_parser(phl_info, input, input_num, output, out_len);
	}
	break;
	case PHL_DBG_WOW:
	{
		_dump_wow_stats(phl_info, input, input_num, output, out_len);
	}
	break;
	case PHL_DBG_RFK:
	{
		_do_rfk(phl_info, input, input_num, output, out_len);
	}
	break;
#ifdef CONFIG_POWER_SAVE
	case PHL_DBG_PS:
	{
		phl_ps_cmd_parser(phl_info, input, input_num, output, out_len);
	}
	break;
#endif
	case PHL_DBG_TRX_CFG:
	{
		phl_dbg_trx_cfg(phl_info, input, input_num, output, out_len);
	}
	break;
#ifdef CONFIG_RTW_HW_RX_AMSDU_CUT
	case PHL_DBG_RX_AMSDU_CUT:
		phl_dbg_rx_amsdu_cut(phl_info, input, input_num, output, out_len);
	break;
	case PHL_DBG_RX_HEADER_CONV:
		phl_dbg_rx_hdr_conv(phl_info, input, input_num, output, out_len);
	break;
#endif /* CONFIG_RTW_HW_RX_AMSDU_CUT */
#if defined(RTW_RX_CPU_BALANCE) || defined(RTW_TX_CPU_BALANCE)
	case PHL_DBG_HDL_CPU:
		_dbg_handler_cpu(phl_info, input, input_num, output, out_len);
	break;
#endif
	case PHL_DBG_ECSA:
	{
#ifdef CONFIG_PHL_ECSA
		struct rtw_wifi_role_t *role = NULL;
		struct rtw_phl_ecsa_param param = {0};
		int chan = 0;
		int bw = 0;
		int offset = 0;
		int count = 0;
		int op_class = 0;
		int mode = 0;
		int delay_start_ms = 0;
		if (input_num <= 7)
			break;
		_os_sscanf(input[1], "%d", &chan);
		_os_sscanf(input[2], "%d", &bw);
		_os_sscanf(input[3], "%d", &offset);
		_os_sscanf(input[4], "%d", &count);
		_os_sscanf(input[5], "%d", &op_class);
		_os_sscanf(input[6], "%d", &mode);
		_os_sscanf(input[7], "%d", &delay_start_ms);
		role = rtw_phl_get_wrole_by_ridx(phl_info->phl_com, 2);
		if(role){
			param.ecsa_type = ECSA_TYPE_AP;
			param.ch = (u8)chan;
			param.count = (u8)count;
			param.flag = 0;
			param.mode = (u8)mode;
			param.op_class = (u8)op_class;
			param.delay_start_ms = delay_start_ms;
			param.new_chan_def.chan = (u8)chan;
			param.new_chan_def.bw = (u8)bw;
			param.new_chan_def.offset = (u8)offset;

			rtw_phl_ecsa_start(phl_info, role, &param);
		}
		else
			PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "[DBG] Role 2 is NULL\n");
#else
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "[DBG] ECSA not support!\n");
#endif /* CONFIG_PHL_ECSA */
	}
	break;
#ifdef CONFIG_RTW_DEBUG_TX_RPT
	case PHL_DBG_TX_RPT:
		_dbg_tx_rpt(phl_info, input, input_num, output, out_len);
	break;
#endif /* CONFIG_RTW_DEBUG_TX_RPT */
#ifdef CONFIG_RTW_DEBUG_BCN_TX
	case PHL_DBG_BCN_TX:
		_dbg_bcn_tx(phl_info, input, input_num, output, out_len);
	break;
#endif /* CONFIG_RTW_DEBUG_TX_RPT */
#ifdef CONFIG_RTW_DEBUG_RX_SZ
	case PHL_DBG_RX_SZ:
		_dbg_rx_sz(phl_info);
	break;
#endif /* CONFIG_RTW_DEBUG_RX_SZ */
	case PHL_DBG_LA_ENABLE:
		_phl_switch_lamode(phl_info, input, input_num, output, out_len);
	break;
	case PHL_DBG_SER_DBG: {
		struct rtw_hal_com_t *hal_com = rtw_hal_get_halcom(phl_info->hal);
		hal_com->dbg_ser = 1;
		break;
	}
	case PHL_DBG_DUMP_SHBUF:
		_dump_mac_shbuf(phl_info);
	break;
#ifdef CONFIG_RTW_HW_TRX_WATCHDOG
	case PHL_DBG_HW_TRX_WDG:
		_phl_hw_trx_watchdog_cmd(phl_info, input, input_num, output, out_len);
	break;
#endif /* CONFIG_RTW_HW_TRX_WATCHDOG */
	default:
		PHL_DBG_MON_INFO(out_len, used, output + used,
			out_len - used, "[DBG] Do not support this command\n");
		break;
	}
}

s32
phl_dbg_proc_cmd(struct phl_info_t *phl_info,
		 char *input, char *output, u32 out_len)
{
	char *token;
	u32 argc = 0;
	char argv[MAX_ARGC][MAX_ARGV];

	do {
		token = _os_strsep(&input, ", ");
		if (token) {
			if (_os_strlen((u8 *)token) <= MAX_ARGV)
				_os_strcpy(argv[argc], token);

			argc++;
		} else {
			break;
		}
	} while (argc < MAX_ARGC);

	phl_dbg_cmd_parser(phl_info, argv, argc, output, out_len);

	return 0;
}

enum rtw_hal_status
rtw_phl_dbg_proc_cmd(struct phl_info_t *phl_info,
		     struct rtw_proc_cmd *incmd,
		     char *output,
		     u32 out_len)
{
	if (incmd->in_type == RTW_ARG_TYPE_BUF) {
		phl_dbg_proc_cmd(phl_info, incmd->in.buf, output, out_len);
	} else if(incmd->in_type == RTW_ARG_TYPE_ARRAY){
		phl_dbg_cmd_parser(phl_info, incmd->in.vector,
				   incmd->in_cnt_len, output, out_len);
	}
	return RTW_HAL_STATUS_SUCCESS;
}

#endif
