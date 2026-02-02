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

#ifndef _MAC_AX_PHY_RPT_HV_H_
#define _MAC_AX_PHY_RPT_HV_H_

#include "../hv_type.h"

#define PPDU_TYPE_LCCK 0
#define PPDU_TYPE_SCCK 1
#define PPDU_TYPE_OFDM 2
#define PPDU_TYPE_HT 3
#define PPDU_TYPE_HTGF 4
#define PPDU_TYPE_VHTSU 5
#define PPDU_TYPE_VHTMU 6
#define PPDU_TYPE_HESU 7
#define PPDU_TYPE_HEERSEU 8
#define PPDU_TYPE_HEMU 9
#define PPDU_TYPE_HETB 10

/**
 * @struct plcp_hdr
 * @brief plcp_hdr
 *
 * @var plcp_hdr::ppdu_type
 * Please Place Description here.
 * @var plcp_hdr::rsvd
 * Please Place Description here.
 */
struct plcp_hdr {
	u32 ppdu_type:4;
	u32 rsvd:28;
};

/**
 * @struct plcp_cck
 * @brief plcp_cck
 *
 * @var plcp_cck::sfd
 * Please Place Description here.
 * @var plcp_cck::cck_rate
 * Please Place Description here.
 * @var plcp_cck::service
 * Please Place Description here.
 * @var plcp_cck::len
 * Please Place Description here.
 * @var plcp_cck::crc
 * Please Place Description here.
 */
struct plcp_cck {
	u32 sfd:16;
	u32 cck_rate:8;
	u32 service:8;
	u32 len:16;
	u32 crc:16;
};

#pragma pack(1)
/**
 * @struct plcp_ofdm
 * @brief plcp_ofdm
 *
 * @var plcp_ofdm::rate
 * Please Place Description here.
 * @var plcp_ofdm::rsvd
 * Please Place Description here.
 * @var plcp_ofdm::lsig_len
 * Please Place Description here.
 * @var plcp_ofdm::parity
 * Please Place Description here.
 * @var plcp_ofdm::zero
 * Please Place Description here.
 */
struct plcp_ofdm {
	u32 rate:4;
	u32 rsvd:1;
	u32 lsig_len:12;
	u32 parity:1;
	u32 zero:6;
};
#pragma pack()

#pragma pack(1)
/**
 * @struct plcp_ht
 * @brief plcp_ht
 *
 * @var plcp_ht::ofdm_rate
 * Please Place Description here.
 * @var plcp_ht::rsvd0
 * Please Place Description here.
 * @var plcp_ht::lsig_len
 * Please Place Description here.
 * @var plcp_ht::parity
 * Please Place Description here.
 * @var plcp_ht::zero0
 * Please Place Description here.
 * @var plcp_ht::rsvd1
 * Please Place Description here.
 * @var plcp_ht::mcs_rate
 * Please Place Description here.
 * @var plcp_ht::bw
 * Please Place Description here.
 * @var plcp_ht::len
 * Please Place Description here.
 * @var plcp_ht::smth
 * Please Place Description here.
 * @var plcp_ht::not_snd
 * Please Place Description here.
 * @var plcp_ht::rsvd2
 * Please Place Description here.
 * @var plcp_ht::agg
 * Please Place Description here.
 * @var plcp_ht::stbc
 * Please Place Description here.
 * @var plcp_ht::ldpc
 * Please Place Description here.
 * @var plcp_ht::short_gi
 * Please Place Description here.
 * @var plcp_ht::ext_spatial
 * Please Place Description here.
 * @var plcp_ht::crc_5_0
 * Please Place Description here.
 * @var plcp_ht::crc_7_6
 * Please Place Description here.
 * @var plcp_ht::zero1
 * Please Place Description here.
 */
struct plcp_ht {
	u32 ofdm_rate:4;
	u32 rsvd0:1;
	u32 lsig_len:12;
	u32 parity:1;
	u32 zero0:6;
	u32 rsvd1:8;
	u32 mcs_rate:7;
	u32 bw:1;
	u32 len:16;
	u32 smth:1;
	u32 not_snd:1;
	u32 rsvd2:1;
	u32 agg:1;
	u32 stbc:2;
	u32 ldpc:1;
	u32 short_gi:1;
	u32 ext_spatial:2;
	u32 crc_5_0:6;
	u32 crc_7_6:2;
	u32 zero1:6;
};
#pragma pack()

#pragma pack(1)
/**
 * @struct plcp_vht_su
 * @brief plcp_vht_su
 *
 * @var plcp_vht_su::ofdm_rate
 * Please Place Description here.
 * @var plcp_vht_su::rsvd0
 * Please Place Description here.
 * @var plcp_vht_su::lsig_len
 * Please Place Description here.
 * @var plcp_vht_su::parity
 * Please Place Description here.
 * @var plcp_vht_su::zero0
 * Please Place Description here.
 * @var plcp_vht_su::rsvd1
 * Please Place Description here.
 * @var plcp_vht_su::bw
 * Please Place Description here.
 * @var plcp_vht_su::one0
 * Please Place Description here.
 * @var plcp_vht_su::stbc
 * Please Place Description here.
 * @var plcp_vht_su::group_id_3_0
 * Please Place Description here.
 * @var plcp_vht_su::group_id_6_5
 * Please Place Description here.
 * @var plcp_vht_su::nsts
 * Please Place Description here.
 * @var plcp_vht_su::partial_aid
 * Please Place Description here.
 * @var plcp_vht_su::txop_all
 * Please Place Description here.
 * @var plcp_vht_su::one1
 * Please Place Description here.
 * @var plcp_vht_su::s_gi
 * Please Place Description here.
 * @var plcp_vht_su::s_gi_nsym
 * Please Place Description here.
 * @var plcp_vht_su::ldpc
 * Please Place Description here.
 * @var plcp_vht_su::ldpc_ext
 * Please Place Description here.
 * @var plcp_vht_su::vht_rate
 * Please Place Description here.
 * @var plcp_vht_su::bf
 * Please Place Description here.
 * @var plcp_vht_su::one2
 * Please Place Description here.
 * @var plcp_vht_su::crc_5_0
 * Please Place Description here.
 * @var plcp_vht_su::crc_7_6
 * Please Place Description here.
 * @var plcp_vht_su::zero1
 * Please Place Description here.
 * @var plcp_vht_su::rsvd2
 * Please Place Description here.
 * @var plcp_vht_su::sigb
 * Please Place Description here.
 */
struct plcp_vht_su {
	u32 ofdm_rate:4;
	u32 rsvd0:1;
	u32 lsig_len:12;
	u32 parity:1;
	u32 zero0:6;
	u32 rsvd1:8;
	u32 bw:2;
	u32 one0:1;
	u32 stbc:1;
	u32 group_id_3_0:4;
	u32 group_id_6_5:2;
	u32 nsts:3;
	u32 partial_aid:9;
	u32 txop_all:1;
	u32 one1:1;
	u32 s_gi:1;
	u32 s_gi_nsym:1;
	u32 ldpc:1;
	u32 ldpc_ext:1;
	u32 vht_rate:4;
	u32 bf:1;
	u32 one2:1;
	u32 crc_5_0:8;
	u32 crc_7_6:8;
	u32 zero1:6;
	u32 rsvd2:16;
	u32 sigb;
};
#pragma pack()

#pragma pack(1)
/**
 * @struct plcp_vht_mu
 * @brief plcp_vht_mu
 *
 * @var plcp_vht_mu::ofdm_rate
 * Please Place Description here.
 * @var plcp_vht_mu::rsvd0
 * Please Place Description here.
 * @var plcp_vht_mu::lsig_len
 * Please Place Description here.
 * @var plcp_vht_mu::parity
 * Please Place Description here.
 * @var plcp_vht_mu::zero0
 * Please Place Description here.
 * @var plcp_vht_mu::rsvd1
 * Please Place Description here.
 * @var plcp_vht_mu::bw
 * Please Place Description here.
 * @var plcp_vht_mu::one0
 * Please Place Description here.
 * @var plcp_vht_mu::zero1
 * Please Place Description here.
 * @var plcp_vht_mu::group_id_3_0
 * Please Place Description here.
 * @var plcp_vht_mu::group_id_6_5
 * Please Place Description here.
 * @var plcp_vht_mu::mu_nsts
 * Please Place Description here.
 * @var plcp_vht_mu::txop_all
 * Please Place Description here.
 * @var plcp_vht_mu::one1
 * Please Place Description here.
 * @var plcp_vht_mu::s_gi
 * Please Place Description here.
 * @var plcp_vht_mu::s_gi_nsym
 * Please Place Description here.
 * @var plcp_vht_mu::ldpc
 * Please Place Description here.
 * @var plcp_vht_mu::ldpc_ext
 * Please Place Description here.
 * @var plcp_vht_mu::mu_ldpc
 * Please Place Description here.
 * @var plcp_vht_mu::one2
 * Please Place Description here.
 * @var plcp_vht_mu::crc_5_0
 * Please Place Description here.
 * @var plcp_vht_mu::crc_7_6
 * Please Place Description here.
 * @var plcp_vht_mu::zero2
 * Please Place Description here.
 * @var plcp_vht_mu::rsvd2
 * Please Place Description here.
 * @var plcp_vht_mu::sigb
 * Please Place Description here.
 */
struct plcp_vht_mu {
	u32 ofdm_rate:4;
	u32 rsvd0:1;
	u32 lsig_len:12;
	u32 parity:1;
	u32 zero0:6;
	u32 rsvd1:8;
	u32 bw:2;
	u32 one0:1;
	u32 zero1:1;
	u32 group_id_3_0:4;
	u32 group_id_6_5:2;
	u32 mu_nsts:12;
	u32 txop_all:1;
	u32 one1:1;
	u32 s_gi:1;
	u32 s_gi_nsym:1;
	u32 ldpc:1;
	u32 ldpc_ext:1;
	u32 mu_ldpc:4;
	u32 one2:2;
	u32 crc_5_0:8;
	u32 crc_7_6:8;
	u32 zero2:6;
	u32 rsvd2:16;
	u32 sigb;
};
#pragma pack()

#pragma pack(1)
/**
 * @struct plcp_he_su
 * @brief plcp_he_su
 *
 * @var plcp_he_su::ofdm_rate
 * Please Place Description here.
 * @var plcp_he_su::rsvd0
 * Please Place Description here.
 * @var plcp_he_su::lsig_len
 * Please Place Description here.
 * @var plcp_he_su::parity
 * Please Place Description here.
 * @var plcp_he_su::zero0
 * Please Place Description here.
 * @var plcp_he_su::rsvd1
 * Please Place Description here.
 * @var plcp_he_su::format
 * Please Place Description here.
 * @var plcp_he_su::be_ch
 * Please Place Description here.
 * @var plcp_he_su::ul_dl
 * Please Place Description here.
 * @var plcp_he_su::mcs
 * Please Place Description here.
 * @var plcp_he_su::dcm
 * Please Place Description here.
 * @var plcp_he_su::bss_color
 * Please Place Description here.
 * @var plcp_he_su::one2
 * Please Place Description here.
 * @var plcp_he_su::spatial_reuse
 * Please Place Description here.
 * @var plcp_he_su::bw
 * Please Place Description here.
 * @var plcp_he_su::gi
 * Please Place Description here.
 * @var plcp_he_su::nsts
 * Please Place Description here.
 * @var plcp_he_su::txop_5_0
 * Please Place Description here.
 * @var plcp_he_su::txop_6
 * Please Place Description here.
 * @var plcp_he_su::coding
 * Please Place Description here.
 * @var plcp_he_su::ldpc_ext
 * Please Place Description here.
 * @var plcp_he_su::stbc
 * Please Place Description here.
 * @var plcp_he_su::txbf
 * Please Place Description here.
 * @var plcp_he_su::pre_fec
 * Please Place Description here.
 * @var plcp_he_su::pe
 * Please Place Description here.
 * @var plcp_he_su::one3
 * Please Place Description here.
 * @var plcp_he_su::doppler
 * Please Place Description here.
 * @var plcp_he_su::crc
 * Please Place Description here.
 * @var plcp_he_su::zero2
 * Please Place Description here.
 */
struct plcp_he_su {
	u32 ofdm_rate:4;
	u32 rsvd0:1;
	u32 lsig_len:12;
	u32 parity:1;
	u32 zero0:6;
	u32 rsvd1:8;
	u32 format:1;
	u32 be_ch:1;
	u32 ul_dl:1;
	u32 mcs:4;
	u32 dcm:1;
	u32 bss_color:6;
	u32 one2:1;
	u32 spatial_reuse:4;
	u32 bw:2;
	u32 gi:2;
	u32 nsts:3;
	u32 txop_5_0:6;
	u32 txop_6:1;
	u32 coding:1;
	u32 ldpc_ext:1;
	u32 stbc:1;
	u32 txbf:1;
	u32 pre_fec:2;
	u32 pe:1;
	u32 one3:1;
	u32 doppler:1;
	u32 crc:4;
	u32 zero2:6;
};
#pragma pack()

#pragma pack(1)
/**
 * @struct plcp_he_tb
 * @brief plcp_he_tb
 *
 * @var plcp_he_tb::ofdm_rate
 * Please Place Description here.
 * @var plcp_he_tb::rsvd0
 * Please Place Description here.
 * @var plcp_he_tb::lsig_len
 * Please Place Description here.
 * @var plcp_he_tb::parity
 * Please Place Description here.
 * @var plcp_he_tb::zero0
 * Please Place Description here.
 * @var plcp_he_tb::rsvd1
 * Please Place Description here.
 * @var plcp_he_tb::format
 * Please Place Description here.
 * @var plcp_he_tb::bss_color
 * Please Place Description here.
 * @var plcp_he_tb::spatial_reuse1_0
 * Please Place Description here.
 * @var plcp_he_tb::spatial_reuse1_3_1
 * Please Place Description here.
 * @var plcp_he_tb::spatial_reuse2
 * Please Place Description here.
 * @var plcp_he_tb::spatial_reuse3
 * Please Place Description here.
 * @var plcp_he_tb::spatial_reuse4
 * Please Place Description here.
 * @var plcp_he_tb::one1
 * Please Place Description here.
 * @var plcp_he_tb::bw
 * Please Place Description here.
 * @var plcp_he_tb::txop_5_0
 * Please Place Description here.
 * @var plcp_he_tb::txop_6
 * Please Place Description here.
 * @var plcp_he_tb::rsvd2
 * Please Place Description here.
 * @var plcp_he_tb::crc
 * Please Place Description here.
 * @var plcp_he_tb::zero2
 * Please Place Description here.
 */
struct plcp_he_tb {
	u32 ofdm_rate:4;
	u32 rsvd0:1;
	u32 lsig_len:12;
	u32 parity:1;
	u32 zero0:6;
	u32 rsvd1:8;
	u32 format:1;
	u32 bss_color:6;
	u32 spatial_reuse1_0:4;
	u32 spatial_reuse1_3_1:4;
	u32 spatial_reuse2:4;
	u32 spatial_reuse3:4;
	u32 spatial_reuse4:4;
	u32 one1:1;
	u32 bw:2;
	u32 txop_5_0:6;
	u32 txop_6:1;
	u32 rsvd2:9;
	u32 crc:4;
	u32 zero2:6;
};
#pragma pack()

#pragma pack(1)
/**
 * @struct plcp_he_mu
 * @brief plcp_he_mu
 *
 * @var plcp_he_mu::ofdm_rate
 * Please Place Description here.
 * @var plcp_he_mu::rsvd0
 * Please Place Description here.
 * @var plcp_he_mu::lsig_len
 * Please Place Description here.
 * @var plcp_he_mu::parity
 * Please Place Description here.
 * @var plcp_he_mu::zero0
 * Please Place Description here.
 * @var plcp_he_mu::rsvd1
 * Please Place Description here.
 * @var plcp_he_mu::ul_dl
 * Please Place Description here.
 * @var plcp_he_mu::sigb_mcs
 * Please Place Description here.
 * @var plcp_he_mu::sigb_dcm
 * Please Place Description here.
 * @var plcp_he_mu::bss_color_2_0
 * Please Place Description here.
 * @var plcp_he_mu::bss_color_5_3
 * Please Place Description here.
 * @var plcp_he_mu::spatial_reuse
 * Please Place Description here.
 * @var plcp_he_mu::bw
 * Please Place Description here.
 * @var plcp_he_mu::num_sigb
 * Please Place Description here.
 * @var plcp_he_mu::sigb_comp
 * Please Place Description here.
 * @var plcp_he_mu::gi_size
 * Please Place Description here.
 * @var plcp_he_mu::doppler
 * Please Place Description here.
 * @var plcp_he_mu::txop_5_0
 * Please Place Description here.
 * @var plcp_he_mu::txop_6
 * Please Place Description here.
 * @var plcp_he_mu::one2
 * Please Place Description here.
 * @var plcp_he_mu::num_ltf
 * Please Place Description here.
 * @var plcp_he_mu::ldpc_ext
 * Please Place Description here.
 * @var plcp_he_mu::stbc
 * Please Place Description here.
 * @var plcp_he_mu::pre_fec_0
 * Please Place Description here.
 * @var plcp_he_mu::pre_fec_1
 * Please Place Description here.
 * @var plcp_he_mu::pe
 * Please Place Description here.
 * @var plcp_he_mu::crc
 * Please Place Description here.
 * @var plcp_he_mu::zero2
 * Please Place Description here.
 * @var plcp_he_mu::rsvd2
 * Please Place Description here.
 * @var plcp_he_mu::sigb_contents
 * Please Place Description here.
 */
struct plcp_he_mu {
	u32 ofdm_rate:4;
	u32 rsvd0:1;
	u32 lsig_len:12;
	u32 parity:1;
	u32 zero0:6;
	u32 rsvd1:8;
	u32 ul_dl:1;
	u32 sigb_mcs:3;
	u32 sigb_dcm:1;
	u32 bss_color_2_0:6;
	u32 bss_color_5_3:6;
	u32 spatial_reuse:4;
	u32 bw:3;
	u32 num_sigb:4;
	u32 sigb_comp:1;
	u32 gi_size:2;
	u32 doppler:1;
	u32 txop_5_0:6;
	u32 txop_6:1;
	u32 one2:1;
	u32 num_ltf:3;
	u32 ldpc_ext:1;
	u32 stbc:1;
	u32 pre_fec_0:2;
	u32 pre_fec_1:2;
	u32 pe:1;
	u32 crc:4;
	u32 zero2:6;
	u32 rsvd2:12;
	u32 sigb_contents;
};
#pragma pack()

/**
 * @brief hv_get_ppdu
 *
 * @param *adapter
 * @param band
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_get_ppdu(struct mac_ax_adapter *adapter, enum mac_ax_band band);

/**
 * @brief hv_chk_ps_dfs
 *
 * @param *adapter
 * @param *data
 * @param len
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_chk_ps_dfs(struct mac_ax_adapter *adapter, u8 *data, u32 len);

/**
 * @brief c2h_test_phy_rpt
 *
 * @param *adapter
 * @param *buf
 * @param len
 * @param *c2h_info
 * @return Please Place Description here.
 * @retval u32
 */
u32 c2h_test_phy_rpt(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		     struct rtw_c2h_info *c2h_info);

/**
 * @brief hv_chk_ps_ppdu
 *
 * @param *adapter
 * @param *data
 * @param len
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_chk_ps_ppdu(struct mac_ax_adapter *adapter, u8 *data, u32 len);

/**
 * @brief hv_chk_ps_ch_info
 *
 * @param *adapter
 * @param *buf
 * @param len
 * @return Please Place Description here.
 * @retval u32
 */
u32 hv_chk_ps_ch_info(struct mac_ax_adapter *adapter, u8 *buf, u32 len);
#endif
