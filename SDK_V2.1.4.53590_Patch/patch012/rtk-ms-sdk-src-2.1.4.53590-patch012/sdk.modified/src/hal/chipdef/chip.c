/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 57050 $
 * $Date: 2015-03-23 14:36:24 +0800 (Mon, 23 Mar 2015) $
 *
 * Purpose : chip symbol and data type definition in the SDK.
 *
 * Feature : chip symbol and data type definition
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/debug/rt_log.h>
#include <osal/lib.h>
#include <ioal/mem32.h>
#include <hal/chipdef/chip.h>
#include <hal/common/halctrl.h>
#if defined (CONFIG_SDK_RTL8390)
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#endif
#if defined (CONFIG_SDK_RTL8380)
#include <hal/chipdef/maple/rtk_maple_reg_struct.h>
#endif
#include <hal/mac/reg.h>
#if defined(CONFIG_SDK_RTL8380)
#include <drv/swcore/rtl8380.h>
#endif

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
#if defined(CONFIG_SDK_RTL8389)
static rt_portinfo_t rtl8389_port_info =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    /* Internal FPGA Board Usage */
    {
        RT_FE_PORT   /*P0 */, RT_FE_PORT   /*P1 */, RT_FE_PORT   /*P2 */, RT_FE_PORT   /*P3 */, RT_FE_PORT   /*P4 */,
        RT_PORT_NONE /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_PORT_NONE /*P8 */, RT_PORT_NONE /*P9 */,
        RT_PORT_NONE /*P10*/, RT_PORT_NONE /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE /*P13*/, RT_PORT_NONE /*P14*/,
        RT_PORT_NONE /*P15*/, RT_PORT_NONE /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE /*P18*/, RT_PORT_NONE /*P19*/,
        RT_PORT_NONE /*P20*/, RT_PORT_NONE /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE /*P23*/, RT_PORT_NONE /*P24*/,
        RT_PORT_NONE /*P25*/, RT_PORT_NONE /*P26*/, RT_PORT_NONE /*P27*/, RT_CPU_PORT  /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
     },
#else
    /* Normal 8389 Chip Port Information */
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT /*P2 */, RT_GE_PORT  /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT   /*P5 */, RT_GE_PORT   /*P6 */, RT_GE_PORT /*P7 */, RT_GE_PORT  /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT /*P12*/, RT_GE_PORT  /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT /*P17*/, RT_GE_PORT  /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT /*P22*/, RT_GE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT /*P27*/, RT_CPU_PORT /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
     },
#endif /* end of CONFIG_SDK_FPGA_PLATFORM */
}; /* end of rtl8389_port_info */

/* Normal 8377 Chip Port Information */
static rt_portinfo_t rtl8377_port_info =
{
    {
        RT_PORT_NONE /*P0 */, RT_PORT_NONE /*P1 */, RT_PORT_NONE /*P2 */, RT_PORT_NONE /*P3 */, RT_PORT_NONE /*P4 */,
        RT_PORT_NONE /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_GE_PORT   /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT   /*P12*/, RT_GE_PORT   /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT   /*P17*/, RT_GE_PORT   /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT   /*P22*/, RT_GE_PORT   /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT   /*P27*/, RT_CPU_PORT  /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
    },
}; /* end of rtl8377_port_info */

/* Normal 8329 Chip Port Information */
static rt_portinfo_t rtl8329_port_info =
{
    {
        RT_FE_PORT   /*P0 */, RT_FE_PORT   /*P1 */, RT_FE_PORT /*P2 */, RT_FE_PORT  /*P3 */, RT_FE_PORT   /*P4 */,
        RT_FE_PORT   /*P5 */, RT_FE_PORT   /*P6 */, RT_FE_PORT /*P7 */, RT_FE_PORT  /*P8 */, RT_FE_PORT   /*P9 */,
        RT_FE_PORT   /*P10*/, RT_FE_PORT   /*P11*/, RT_FE_PORT /*P12*/, RT_FE_PORT  /*P13*/, RT_FE_PORT   /*P14*/,
        RT_FE_PORT   /*P15*/, RT_FE_PORT   /*P16*/, RT_FE_PORT /*P17*/, RT_FE_PORT  /*P18*/, RT_FE_PORT   /*P19*/,
        RT_FE_PORT   /*P20*/, RT_FE_PORT   /*P21*/, RT_FE_PORT /*P22*/, RT_FE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT /*P27*/, RT_CPU_PORT /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
    },
}; /* end of rtl8329_port_info */

static rt_register_capacity_t rtl8389_capacityInfo =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    /* Internal FPGA Board Usage */
    .max_num_of_mirror              = 2,
    .max_num_of_trunk               = 8,
    .max_num_of_trunkMember         = 8,
    .max_num_of_dumb_trunkMember    = 4,
    .max_num_of_trunkHashVal        = 32,
    .max_num_of_msti                = 16,
    .max_num_of_metering            = 128,
    .max_num_of_pie_block           = 2,
    .max_num_of_pie_blockSize       = 4,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_cvlan_tpid          = 0,
    .max_num_of_svlan_tpid          = 1,
    .flowctrl_thresh_max            = 0x7FF,
    .pri_of_selection_max           = 3,
    .pri_of_selection_min           = 0,
    .queue_weight_max               = 128,
    .rate_of_bandwidth_max          = 0xFFFF,
    .rate_of_storm_control_max      = 0xFFFFF,
    .internal_priority_max          = 7,
    .acl_rate_max                   = 0xFFFF,
    .max_num_of_l2_hash_algo        = 2,
    .l2_learn_limit_cnt_max         = 0x4040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x4000,
    .l2_learn_limit_cnt_disable     = 0x7FFF,
    .l2_fid_learn_limit_entry_max   = 0,
    .sflow_rate_max                 = 0,
    .miim_page_id_max               = 31,
    .max_frame_len                  = 9216,
#else
    /* Normal 8389 Chip Port Information */
    .max_num_of_mirror              = 2,
    .max_num_of_trunk               = 8,
    .max_num_of_trunkMember         = 8,
    .max_num_of_dumb_trunkMember    = 4,
    .max_num_of_trunkHashVal        = 32,
    .max_num_of_msti                = 16,
    .max_num_of_metering            = 128,
    .max_num_of_pie_block           = 8,
    .max_num_of_pie_blockSize       = 64,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_cvlan_tpid          = 0,
    .max_num_of_svlan_tpid          = 1,
    .flowctrl_thresh_max            = 0x7FF,
    .pri_of_selection_max           = 3,
    .pri_of_selection_min           = 0,
    .queue_weight_max               = 128,
    .rate_of_bandwidth_max          = 0xFFFF,
    .rate_of_storm_control_max      = 0xFFFFF,
    .internal_priority_max          = 7,
    .acl_rate_max                   = 0xFFFF,
    .max_num_of_l2_hash_algo        = 2,
    .l2_learn_limit_cnt_max         = 0x4040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x4000,
    .l2_learn_limit_cnt_disable     = 0x7FFF,
    .l2_fid_learn_limit_entry_max   = 0,
    .sflow_rate_max                 = 0,
    .miim_page_id_max               = 31,
    .max_frame_len                  = 9216,
#endif /* end of CONFIG_SDK_FPGA_PLATFORM */
}; /* end of rtl8389_capacityInfo */

/* Normal 8389 Chip PER_PORT block information */
static rt_macPpInfo_t rtl8389_macPpInfo[] =
{
    {
      /* lowerbound_addr */ 0x0000,
      /* upperbound_addr */ 0x1CFF,
      /* interval */         0x100,
    },
};
#endif /* end of defined(CONFIG_SDK_RTL8389) */

#if defined(CONFIG_SDK_RTL8328)
/* Normal 8328 Chip Port Information */
static rt_portinfo_t rtl8328_port_info =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    /* Internal FPGA Board Usage */
    {
        RT_FE_PORT   /*P0 */, RT_PORT_NONE /*P1 */, RT_PORT_NONE /*P2 */, RT_PORT_NONE /*P3 */, RT_PORT_NONE /*P4 */,
        RT_PORT_NONE /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_FE_PORT   /*P8 */, RT_PORT_NONE /*P9 */,
        RT_PORT_NONE /*P10*/, RT_PORT_NONE /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE /*P13*/, RT_PORT_NONE /*P14*/,
        RT_PORT_NONE /*P15*/, RT_PORT_NONE /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE /*P18*/, RT_PORT_NONE /*P19*/,
        RT_PORT_NONE /*P20*/, RT_PORT_NONE /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE /*P23*/, RT_GE_PORT   /*P24*/,
        RT_PORT_NONE /*P25*/, RT_GE_PORT   /*P26*/, RT_PORT_NONE /*P27*/, RT_CPU_PORT  /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
    },
#else
    /* Normal 8328 Chip Port Information */
    {
        RT_INT_FE_PORT   /*P0 */, RT_INT_FE_PORT   /*P1 */, RT_INT_FE_PORT /*P2 */, RT_INT_FE_PORT  /*P3 */, RT_INT_FE_PORT   /*P4 */,
        RT_INT_FE_PORT   /*P5 */, RT_INT_FE_PORT   /*P6 */, RT_INT_FE_PORT /*P7 */, RT_INT_FE_PORT  /*P8 */, RT_INT_FE_PORT   /*P9 */,
        RT_INT_FE_PORT   /*P10*/, RT_INT_FE_PORT   /*P11*/, RT_INT_FE_PORT /*P12*/, RT_INT_FE_PORT  /*P13*/, RT_INT_FE_PORT   /*P14*/,
        RT_INT_FE_PORT   /*P15*/, RT_FE_PORT   /*P16*/, RT_FE_PORT /*P17*/, RT_FE_PORT  /*P18*/, RT_FE_PORT   /*P19*/,
        RT_FE_PORT   /*P20*/, RT_FE_PORT   /*P21*/, RT_FE_PORT /*P22*/, RT_FE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT /*P27*/, RT_CPU_PORT /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
    },
#endif /* end of CONFIG_SDK_FPGA_PLATFORM */
}; /* end of rtl8328_port_info */

/* Normal 8328 Chip Port Information */
static rt_register_capacity_t rtl8328m_capacityInfo =
{
    .max_num_of_mirror              = 4,
    .max_num_of_trunk               = 8,
    .max_num_of_trunkMember         = 8,
    .max_num_of_dumb_trunkMember    = 8,
    .max_num_of_trunkHashVal        = 16,
    .max_num_of_msti                = 128,
    .max_num_of_metering            = 256,
    .max_num_of_pie_block           = 32,
    .max_num_of_pie_logical_block   = 16,
    .max_num_of_pie_blockSize       = 64,
    .max_num_of_pie_counter         = 1024,
    .max_num_of_pie_action          = 2048,
    .max_num_of_pie_template        = 16,
    .max_num_of_pie_payload         = 2,
    .max_num_of_field_selector      = 2,
    .max_num_of_range_check_srcPort = 16,
    .max_num_of_range_check_ip      = 16,
    .max_num_of_range_check_vid     = 32,
    .max_num_of_range_check_l4Port  = 16,
    .max_num_of_pattern_match_data  = 32,
    .pattern_match_port_max         = 27,
    .pattern_match_port_min         = 24,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_cvlan_tpid          = 0,
    .max_num_of_svlan_tpid          = 0,
    .tpid_entry_idx_max             = 3,
    .tpid_entry_mask_max            = 0xf,
    .protocol_vlan_idx_max          = 7,
    .vlan_fid_max                   = 127,
    .flowctrl_thresh_max            = 0x7FF,
    .flowctrl_pauseOn_page_packet_max   = 0xFF,
    .pri_of_selection_max           = 7,
    .pri_of_selection_min           = 0,
    .pri_sel_group_index_max        = 7,
    .queue_weight_max               = 127,
    .rate_of_bandwidth_max          = 0xFFFF,
    .thresh_of_igr_bw_flowctrl_max  = 0xFF,
    .max_num_of_fastPath_of_rate    = 3,
    .rate_of_storm_control_max      = 0xFFFFF,
    .burst_rate_of_storm_control_max      = 0xFFFFFF,
    .internal_priority_max          = 7,
    .drop_precedence_max            = 3,
    .priority_remap_group_idx_max   = 3,
    .priority_remark_group_idx_max  = 7,
    .wred_weight_max                = 0x3FF,
    .wred_mpd_max                   = 0xF,
    .acl_rate_max                   = 0xFFFF,
    .l2_learn_limit_cnt_max         = 0x4040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x4000,
    .l2_learn_limit_cnt_disable     = 0x4040,
    .l2_fid_learn_limit_entry_max   = 32,
    .eee_queue_thresh_max           = 0xFF,
    .sec_minIpv6FragLen_max         = 0xFFFF,
    .sec_maxPingLen_max             = 0xFFFF,
    .sec_smurfNetmaskLen_max        = 31,
    .sflow_rate_max                 = 0xFFFF,
    .max_num_of_mcast_fwd           = 1024,
    .miim_page_id_max               = 127,
    .max_num_of_l2_hash_algo	    = 2,
    .max_frame_len                  = 9216,
};

static rt_register_capacity_t rtl8328s_capacityInfo =
{
    .max_num_of_mirror              = 4,
    .max_num_of_trunk               = 8,
    .max_num_of_trunkMember         = 8,
    .max_num_of_dumb_trunkMember    = 8,
    .max_num_of_trunkHashVal        = 16,
    .max_num_of_msti                = 128,
    .max_num_of_metering            = 256,
    .max_num_of_pie_block           = 8,
    .max_num_of_pie_logical_block   = 4,
    .max_num_of_pie_blockSize       = 64,
    .max_num_of_pie_counter         = 512,
    .max_num_of_pie_action          = 512,
    .max_num_of_pie_template        = 16,
    .max_num_of_pie_payload         = 2,
    .max_num_of_field_selector      = 2,
    .max_num_of_range_check_srcPort = 16,
    .max_num_of_range_check_ip      = 16,
    .max_num_of_range_check_vid     = 32,
    .max_num_of_range_check_l4Port  = 16,
    .max_num_of_pattern_match_data  = 32,
    .pattern_match_port_max         = 27,
    .pattern_match_port_min         = 24,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_cvlan_tpid          = 0,
    .max_num_of_svlan_tpid          = 0,
    .tpid_entry_idx_max             = 3,
    .tpid_entry_mask_max            = 0xf,
    .protocol_vlan_idx_max          = 7,
    .vlan_fid_max                   = 127,
    .flowctrl_thresh_max            = 0x7FF,
    .flowctrl_pauseOn_page_packet_max   = 0xFF,
    .pri_of_selection_max           = 7,
    .pri_of_selection_min           = 0,
    .pri_sel_group_index_max        = 7,
    .queue_weight_max               = 127,
    .rate_of_bandwidth_max          = 0xFFFF,
    .thresh_of_igr_bw_flowctrl_max  = 0xFF,
    .max_num_of_fastPath_of_rate    = 3,
    .rate_of_storm_control_max      = 0xFFFFF,
    .burst_rate_of_storm_control_max      = 0xFFFFFF,
    .internal_priority_max          = 7,
    .drop_precedence_max            = 3,
    .priority_remap_group_idx_max   = 3,
    .priority_remark_group_idx_max  = 7,
    .wred_weight_max                = 0x3FF,
    .wred_mpd_max                   = 0xF,
    .acl_rate_max                   = 0xFFFF,
    .l2_learn_limit_cnt_max         = 0x4040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x4000,
    .l2_learn_limit_cnt_disable     = 0x4040,
    .l2_fid_learn_limit_entry_max   = 32,
    .eee_queue_thresh_max           = 0xFF,
    .sec_minIpv6FragLen_max         = 0xFFFF,
    .sec_maxPingLen_max             = 0xFFFF,
    .sec_smurfNetmaskLen_max        = 31,
    .sflow_rate_max                 = 0xFFFF,
    .max_num_of_mcast_fwd           = 512,
    .miim_page_id_max               = 127,
    .max_num_of_l2_hash_algo	    = 2,
    .max_frame_len                  = 9216,
};

static rt_register_capacity_t rtl8328s_capacityInfo_C =
{
    .max_num_of_mirror              = 4,
    .max_num_of_trunk               = 8,
    .max_num_of_trunkMember         = 8,
    .max_num_of_dumb_trunkMember    = 8,
    .max_num_of_trunkHashVal        = 16,
    .max_num_of_msti                = 128,
    .max_num_of_metering            = 256,
    .max_num_of_pie_block           = 16,
    .max_num_of_pie_logical_block   = 8,
    .max_num_of_pie_blockSize       = 64,
    .max_num_of_pie_counter         = 1024,
    .max_num_of_pie_action          = 1024,
    .max_num_of_pie_template        = 16,
    .max_num_of_pie_payload         = 2,
    .max_num_of_field_selector      = 2,
    .max_num_of_range_check_srcPort = 16,
    .max_num_of_range_check_ip      = 16,
    .max_num_of_range_check_vid     = 32,
    .max_num_of_range_check_l4Port  = 16,
    .max_num_of_pattern_match_data  = 32,
    .pattern_match_port_max         = 27,
    .pattern_match_port_min         = 24,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_cvlan_tpid          = 0,
    .max_num_of_svlan_tpid          = 0,
    .tpid_entry_idx_max             = 3,
    .tpid_entry_mask_max            = 0xf,
    .protocol_vlan_idx_max          = 7,
    .vlan_fid_max                   = 127,
    .flowctrl_thresh_max            = 0x7FF,
    .flowctrl_pauseOn_page_packet_max   = 0xFF,
    .pri_of_selection_max           = 7,
    .pri_of_selection_min           = 0,
    .pri_sel_group_index_max        = 7,
    .queue_weight_max               = 127,
    .rate_of_bandwidth_max          = 0xFFFF,
    .thresh_of_igr_bw_flowctrl_max  = 0xFF,
    .max_num_of_fastPath_of_rate    = 3,
    .rate_of_storm_control_max      = 0xFFFFF,
    .burst_rate_of_storm_control_max      = 0xFFFFFF,
    .internal_priority_max          = 7,
    .drop_precedence_max            = 3,
    .priority_remap_group_idx_max   = 3,
    .priority_remark_group_idx_max  = 7,
    .wred_weight_max                = 0x3FF,
    .wred_mpd_max                   = 0xF,
    .acl_rate_max                   = 0xFFFF,
    .l2_learn_limit_cnt_max         = 0x4040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x4000,
    .l2_learn_limit_cnt_disable     = 0x4040,
    .l2_fid_learn_limit_entry_max   = 32,
    .eee_queue_thresh_max           = 0xFF,
    .sec_minIpv6FragLen_max         = 0xFFFF,
    .sec_maxPingLen_max             = 0xFFFF,
    .sec_smurfNetmaskLen_max        = 31,
    .sflow_rate_max                 = 0xFFFF,
    .max_num_of_mcast_fwd           = 512,
    .miim_page_id_max               = 127,
    .max_num_of_l2_hash_algo	    = 2,
    .max_frame_len                  = 9216,
};


/* Normal 8328 Chip PER_PORT block information */
static rt_macPpInfo_t rtl8328_macPpInfo[] =
{
    {
      /* lowerbound_addr */ 0x800000,
      /* upperbound_addr */ 0xFFFFFF,
      /* interval */           0x100,
    },
};
#endif  /* end of defined(CONFIG_SDK_RTL8328) */


#if defined(CONFIG_SDK_RTL8390)
/* Normal 8352 Chip Port Information */
static rt_portinfo_t rtl8352_port_info =
{
    {
        RT_FE_PORT   /*P0 */, RT_FE_PORT   /*P1 */, RT_FE_PORT  /*P2 */, RT_FE_PORT  /*P3 */, RT_FE_PORT   /*P4 */,
        RT_FE_PORT   /*P5 */, RT_FE_PORT   /*P6 */, RT_FE_PORT  /*P7 */, RT_FE_PORT  /*P8 */, RT_FE_PORT   /*P9 */,
        RT_FE_PORT   /*P10*/, RT_FE_PORT   /*P11*/, RT_FE_PORT  /*P12*/, RT_FE_PORT  /*P13*/, RT_FE_PORT   /*P14*/,
        RT_FE_PORT   /*P15*/, RT_FE_PORT   /*P16*/, RT_FE_PORT  /*P17*/, RT_FE_PORT  /*P18*/, RT_FE_PORT   /*P19*/,
        RT_FE_PORT   /*P20*/, RT_FE_PORT   /*P21*/, RT_FE_PORT  /*P22*/, RT_FE_PORT  /*P23*/, RT_PORT_NONE /*P24*/,
        RT_PORT_NONE /*P25*/, RT_PORT_NONE /*P26*/, RT_PORT_NONE/*P27*/, RT_PORT_NONE/*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/, RT_PORT_NONE/*P32*/, RT_PORT_NONE/*P33*/, RT_PORT_NONE /*P34*/,
        RT_PORT_NONE /*P35*/, RT_PORT_NONE /*P36*/, RT_PORT_NONE/*P37*/, RT_PORT_NONE/*P38*/, RT_PORT_NONE /*P39*/,
        RT_PORT_NONE /*P40*/, RT_PORT_NONE /*P41*/, RT_PORT_NONE/*P42*/, RT_PORT_NONE/*P43*/, RT_PORT_NONE /*P44*/,
        RT_PORT_NONE /*P45*/, RT_PORT_NONE /*P46*/, RT_PORT_NONE/*P47*/, RT_GE_PORT  /*P48*/, RT_GE_PORT   /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT /*P52*/, RT_PORT_NONE/*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE/*P57*/, RT_PORT_NONE/*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE/*P62*/, RT_PORT_NONE/*P63*/
    },
}; /* end of rtl8352_port_info */

/* Normal 8353 Chip Port Information */
static rt_portinfo_t rtl8353_port_info =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    /* Internal FPGA Board Usage */
    {
        RT_FE_PORT   /*P0 */, RT_FE_PORT   /*P1 */, RT_FE_PORT   /*P2 */, RT_FE_PORT /*P3 */,   RT_FE_PORT /*P4 */,
        RT_FE_PORT   /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_PORT_NONE /*P8 */, RT_PORT_NONE /*P9 */,
        RT_PORT_NONE /*P10*/, RT_PORT_NONE /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE /*P13*/, RT_PORT_NONE /*P14*/,
        RT_PORT_NONE /*P15*/, RT_PORT_NONE /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE /*P18*/, RT_PORT_NONE /*P19*/,
        RT_PORT_NONE /*P20*/, RT_PORT_NONE /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE /*P23*/, RT_10GE_PORT /*P24*/,
        RT_PORT_NONE /*P25*/, RT_PORT_NONE /*P26*/, RT_PORT_NONE /*P27*/, RT_PORT_NONE /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/, RT_PORT_NONE /*P32*/, RT_PORT_NONE /*P33*/, RT_PORT_NONE /*P34*/,
        RT_PORT_NONE /*P35*/, RT_PORT_NONE /*P36*/, RT_PORT_NONE /*P37*/, RT_PORT_NONE /*P38*/, RT_PORT_NONE /*P39*/,
        RT_PORT_NONE /*P40*/, RT_PORT_NONE /*P41*/, RT_PORT_NONE /*P42*/, RT_PORT_NONE /*P43*/, RT_PORT_NONE /*P44*/,
        RT_PORT_NONE /*P45*/, RT_PORT_NONE /*P46*/, RT_PORT_NONE /*P47*/, RT_GE_PORT /*P48*/,   RT_GE_PORT /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT  /*P52*/, RT_PORT_NONE /*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE /*P57*/, RT_PORT_NONE /*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE /*P62*/, RT_PORT_NONE /*P63*/
    },
#else
    {
        RT_FE_PORT   /*P0 */, RT_FE_PORT   /*P1 */, RT_FE_PORT  /*P2 */, RT_FE_PORT  /*P3 */, RT_FE_PORT   /*P4 */,
        RT_FE_PORT   /*P5 */, RT_FE_PORT   /*P6 */, RT_FE_PORT  /*P7 */, RT_FE_PORT  /*P8 */, RT_FE_PORT   /*P9 */,
        RT_FE_PORT   /*P10*/, RT_FE_PORT   /*P11*/, RT_FE_PORT  /*P12*/, RT_FE_PORT  /*P13*/, RT_FE_PORT   /*P14*/,
        RT_FE_PORT   /*P15*/, RT_FE_PORT   /*P16*/, RT_FE_PORT  /*P17*/, RT_FE_PORT  /*P18*/, RT_FE_PORT   /*P19*/,
        RT_FE_PORT   /*P20*/, RT_FE_PORT   /*P21*/, RT_FE_PORT  /*P22*/, RT_FE_PORT  /*P23*/, RT_FE_PORT   /*P24*/,
        RT_FE_PORT   /*P25*/, RT_FE_PORT   /*P26*/, RT_FE_PORT  /*P27*/, RT_FE_PORT  /*P28*/, RT_FE_PORT   /*P29*/,
        RT_FE_PORT   /*P30*/, RT_FE_PORT   /*P31*/, RT_FE_PORT  /*P32*/, RT_FE_PORT  /*P33*/, RT_FE_PORT   /*P34*/,
        RT_FE_PORT   /*P35*/, RT_FE_PORT   /*P36*/, RT_FE_PORT  /*P37*/, RT_FE_PORT  /*P38*/, RT_FE_PORT   /*P39*/,
        RT_FE_PORT   /*P40*/, RT_FE_PORT   /*P41*/, RT_FE_PORT  /*P42*/, RT_FE_PORT  /*P43*/, RT_FE_PORT   /*P44*/,
        RT_FE_PORT   /*P45*/, RT_FE_PORT   /*P46*/, RT_FE_PORT  /*P47*/, RT_GE_PORT  /*P48*/, RT_GE_PORT   /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT /*P52*/, RT_PORT_NONE/*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE/*P57*/, RT_PORT_NONE/*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE/*P62*/, RT_PORT_NONE/*P63*/
    },
#endif
}; /* end of rtl8353_port_info */

/* Normal 8391 Chip Port Information */
static rt_portinfo_t rtl8391_port_info =
{
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT  /*P2 */, RT_GE_PORT  /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT   /*P5 */, RT_GE_PORT   /*P6 */, RT_GE_PORT  /*P7 */, RT_GE_PORT  /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT  /*P12*/, RT_GE_PORT  /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT  /*P17*/, RT_GE_PORT  /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT  /*P22*/, RT_GE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT  /*P27*/, RT_PORT_NONE/*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/, RT_PORT_NONE/*P32*/, RT_PORT_NONE/*P33*/, RT_PORT_NONE /*P34*/,
        RT_PORT_NONE /*P35*/, RT_PORT_NONE /*P36*/, RT_PORT_NONE/*P37*/, RT_PORT_NONE/*P38*/, RT_PORT_NONE /*P39*/,
        RT_PORT_NONE /*P40*/, RT_PORT_NONE /*P41*/, RT_PORT_NONE/*P42*/, RT_PORT_NONE/*P43*/, RT_PORT_NONE /*P44*/,
        RT_PORT_NONE /*P45*/, RT_PORT_NONE /*P46*/, RT_PORT_NONE/*P47*/, RT_PORT_NONE/*P48*/, RT_PORT_NONE /*P49*/,
        RT_PORT_NONE /*P50*/, RT_PORT_NONE /*P51*/, RT_CPU_PORT /*P52*/, RT_PORT_NONE/*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE/*P57*/, RT_PORT_NONE/*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE/*P62*/, RT_PORT_NONE/*P63*/
    },
}; /* end of rtl8391_port_info */

/* Normal 8392 Chip Port Information */
static rt_portinfo_t rtl8392_port_info =
{
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT  /*P2 */, RT_GE_PORT  /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT   /*P5 */, RT_GE_PORT   /*P6 */, RT_GE_PORT  /*P7 */, RT_GE_PORT  /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT  /*P12*/, RT_GE_PORT  /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT  /*P17*/, RT_GE_PORT  /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT  /*P22*/, RT_GE_PORT  /*P23*/, RT_PORT_NONE /*P24*/,
        RT_PORT_NONE /*P25*/, RT_PORT_NONE /*P26*/, RT_PORT_NONE/*P27*/, RT_PORT_NONE/*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/, RT_PORT_NONE/*P32*/, RT_PORT_NONE/*P33*/, RT_PORT_NONE /*P34*/,
        RT_PORT_NONE /*P35*/, RT_PORT_NONE /*P36*/, RT_PORT_NONE/*P37*/, RT_PORT_NONE/*P38*/, RT_PORT_NONE /*P39*/,
        RT_PORT_NONE /*P40*/, RT_PORT_NONE /*P41*/, RT_PORT_NONE/*P42*/, RT_PORT_NONE/*P43*/, RT_PORT_NONE /*P44*/,
        RT_PORT_NONE /*P45*/, RT_PORT_NONE /*P46*/, RT_PORT_NONE/*P47*/, RT_GE_PORT  /*P48*/, RT_GE_PORT   /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT /*P52*/, RT_PORT_NONE/*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE/*P57*/, RT_PORT_NONE/*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE/*P62*/, RT_PORT_NONE/*P63*/
    },
}; /* end of rtl8392_port_info */

/* Normal 8393 Chip Port Information */
static rt_portinfo_t rtl8393_port_info =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    /* Internal FPGA Board Usage */
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT   /*P2 */, RT_GE_PORT /*P3 */,   RT_GE_PORT /*P4 */,
        RT_GE_PORT   /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_PORT_NONE /*P8 */, RT_PORT_NONE /*P9 */,
        RT_PORT_NONE /*P10*/, RT_PORT_NONE /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE /*P13*/, RT_PORT_NONE /*P14*/,
        RT_PORT_NONE /*P15*/, RT_PORT_NONE /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE /*P18*/, RT_PORT_NONE /*P19*/,
        RT_PORT_NONE /*P20*/, RT_PORT_NONE /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE /*P23*/, RT_10GE_PORT /*P24*/,
        RT_PORT_NONE /*P25*/, RT_PORT_NONE /*P26*/, RT_PORT_NONE /*P27*/, RT_PORT_NONE /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/, RT_PORT_NONE /*P32*/, RT_PORT_NONE /*P33*/, RT_PORT_NONE /*P34*/,
        RT_PORT_NONE /*P35*/, RT_PORT_NONE /*P36*/, RT_PORT_NONE /*P37*/, RT_PORT_NONE /*P38*/, RT_PORT_NONE /*P39*/,
        RT_PORT_NONE /*P40*/, RT_PORT_NONE /*P41*/, RT_PORT_NONE /*P42*/, RT_PORT_NONE /*P43*/, RT_PORT_NONE /*P44*/,
        RT_PORT_NONE /*P45*/, RT_PORT_NONE /*P46*/, RT_PORT_NONE /*P47*/, RT_GE_PORT /*P48*/,   RT_GE_PORT /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT  /*P52*/, RT_PORT_NONE /*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE /*P57*/, RT_PORT_NONE /*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE /*P62*/, RT_PORT_NONE /*P63*/
    },
#else
    /* Normal 8393 Chip Port Information */
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT  /*P2 */, RT_GE_PORT  /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT   /*P5 */, RT_GE_PORT   /*P6 */, RT_GE_PORT  /*P7 */, RT_GE_PORT  /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT  /*P12*/, RT_GE_PORT  /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT  /*P17*/, RT_GE_PORT  /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT  /*P22*/, RT_GE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT  /*P27*/, RT_GE_PORT  /*P28*/, RT_GE_PORT   /*P29*/,
        RT_GE_PORT   /*P30*/, RT_GE_PORT   /*P31*/, RT_GE_PORT  /*P32*/, RT_GE_PORT  /*P33*/, RT_GE_PORT   /*P34*/,
        RT_GE_PORT   /*P35*/, RT_GE_PORT   /*P36*/, RT_GE_PORT  /*P37*/, RT_GE_PORT  /*P38*/, RT_GE_PORT   /*P39*/,
        RT_GE_PORT   /*P40*/, RT_GE_PORT   /*P41*/, RT_GE_PORT  /*P42*/, RT_GE_PORT  /*P43*/, RT_GE_PORT   /*P44*/,
        RT_GE_PORT   /*P45*/, RT_GE_PORT   /*P46*/, RT_GE_PORT  /*P47*/, RT_GE_PORT  /*P48*/, RT_GE_PORT   /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT /*P52*/, RT_PORT_NONE/*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE/*P57*/, RT_PORT_NONE/*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE/*P62*/, RT_PORT_NONE/*P63*/
    },
#endif /* end of CONFIG_SDK_FPGA_PLATFORM */
}; /* end of rtl8393_port_info */

/* Normal 8396 Chip Port Information */
static rt_portinfo_t rtl8396_port_info =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    /* Internal FPGA Board Usage */
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT   /*P2 */, RT_GE_PORT   /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT   /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_PORT_NONE /*P8 */, RT_PORT_NONE /*P9 */,
        RT_PORT_NONE /*P10*/, RT_PORT_NONE /*P11*/, RT_PORT_NONE /*P12*/, RT_PORT_NONE /*P13*/, RT_PORT_NONE /*P14*/,
        RT_PORT_NONE /*P15*/, RT_PORT_NONE /*P16*/, RT_PORT_NONE /*P17*/, RT_PORT_NONE /*P18*/, RT_PORT_NONE /*P19*/,
        RT_PORT_NONE /*P20*/, RT_PORT_NONE /*P21*/, RT_PORT_NONE /*P22*/, RT_PORT_NONE /*P23*/, RT_10GE_PORT /*P24*/,
        RT_PORT_NONE /*P25*/, RT_PORT_NONE /*P26*/, RT_PORT_NONE /*P27*/, RT_PORT_NONE /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/, RT_PORT_NONE /*P32*/, RT_PORT_NONE /*P33*/, RT_PORT_NONE /*P34*/,
        RT_PORT_NONE /*P35*/, RT_PORT_NONE /*P36*/, RT_PORT_NONE /*P37*/, RT_PORT_NONE /*P38*/, RT_PORT_NONE /*P39*/,
        RT_PORT_NONE /*P40*/, RT_PORT_NONE /*P41*/, RT_PORT_NONE /*P42*/, RT_PORT_NONE /*P43*/, RT_PORT_NONE /*P44*/,
        RT_PORT_NONE /*P45*/, RT_PORT_NONE /*P46*/, RT_PORT_NONE /*P47*/, RT_GE_PORT   /*P48*/, RT_GE_PORT   /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT  /*P52*/, RT_PORT_NONE /*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE /*P57*/, RT_PORT_NONE /*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE /*P62*/, RT_PORT_NONE /*P63*/
    },
#else
    /* Normal 8396 Chip Port Information */
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT  /*P2 */, RT_GE_PORT  /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT   /*P5 */, RT_GE_PORT   /*P6 */, RT_GE_PORT  /*P7 */, RT_GE_PORT  /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT  /*P12*/, RT_GE_PORT  /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT  /*P17*/, RT_GE_PORT  /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT  /*P22*/, RT_GE_PORT  /*P23*/, RT_10GE_PORT /*P24*/,
        RT_PORT_NONE /*P25*/, RT_PORT_NONE /*P26*/, RT_PORT_NONE/*P27*/, RT_PORT_NONE/*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/, RT_PORT_NONE/*P32*/, RT_PORT_NONE/*P33*/, RT_PORT_NONE /*P34*/,
        RT_PORT_NONE /*P35*/, RT_10GE_PORT /*P36*/, RT_PORT_NONE/*P37*/, RT_PORT_NONE/*P38*/, RT_PORT_NONE /*P39*/,
        RT_PORT_NONE /*P40*/, RT_PORT_NONE /*P41*/, RT_PORT_NONE/*P42*/, RT_PORT_NONE/*P43*/, RT_PORT_NONE /*P44*/,
        RT_PORT_NONE /*P45*/, RT_PORT_NONE /*P46*/, RT_PORT_NONE/*P47*/, RT_GE_PORT  /*P48*/, RT_GE_PORT   /*P49*/,
        RT_GE_PORT   /*P50*/, RT_GE_PORT   /*P51*/, RT_CPU_PORT /*P52*/, RT_PORT_NONE/*P53*/, RT_PORT_NONE /*P54*/,
        RT_PORT_NONE /*P55*/, RT_PORT_NONE /*P56*/, RT_PORT_NONE/*P57*/, RT_PORT_NONE/*P58*/, RT_PORT_NONE /*P59*/,
        RT_PORT_NONE /*P60*/, RT_PORT_NONE /*P61*/, RT_PORT_NONE/*P62*/, RT_PORT_NONE/*P63*/
    },
#endif  /* end of CONFIG_SDK_FPGA_PLATFORM */
}; /* end of rtl8396_port_info */

static rt_serdesInfo_t rtl8393_serdes_info =
{   /* 8393 Chip Serdes Information: 48G + 4G */
    {
        RT_SERDES_5G/*SDS0 */, RT_SERDES_5G/*SDS1 */, RT_SERDES_5G/*SDS2 */, RT_SERDES_5G  /*SDS3 */, RT_SERDES_5G/*SDS4 */,
        RT_SERDES_5G/*SDS5 */, RT_SERDES_5G/*SDS6 */, RT_SERDES_5G/*SDS7 */, RT_SERDES_5G  /*SDS8 */, RT_SERDES_5G/*SDS9 */,
        RT_SERDES_5G/*SDS10*/, RT_SERDES_5G/*SDS11*/, RT_SERDES_5G/*SDS12*/, RT_SERDES_NONE/*SDS13*/,
    },
};

/* Normal 8390 Chip Port Information */
static rt_register_capacity_t rtl8390m_capacityInfo =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    .max_num_of_mirror              = 4,
    .max_num_of_trunk               = 4,
    .max_num_of_trunkMember         = 8,
    .max_num_of_trunk_algo          = 4,
    .trunk_algo_shift_max           = 4,
    .max_num_of_dumb_trunkMember    = 8,
    .max_num_of_trunkHashVal        = 32,
    .max_num_of_msti                = 256,
    .max_num_of_metering            = 256,
    .max_num_of_meter_block         = 16,
    .max_num_of_pie_block           = 18,
    .max_num_of_acl_block_templateSelector  = 2,
    .max_num_of_pie_blockSize       = 128,
    .max_num_of_pie_counter         = 1024,
    .max_num_of_pie_template        = 8,
    .pie_user_template_id_min       = 5,
    .pie_user_template_id_max       = 7,
    .max_num_of_field_selector      = 12,
    .max_num_of_range_check_srcPort = 16,
    .max_num_of_range_check_dstPort = 16,
    .max_num_of_range_check_ip      = 8,
    .max_num_of_range_check_vid     = 32,
    .max_num_of_range_check_l4Port  = 8,
    .max_num_of_range_check_pktLen  = 8,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 4,
    .min_num_of_queue               = 1,
    .max_num_of_cvlan_tpid          = 4,
    .max_num_of_svlan_tpid          = 4,
    .max_num_of_evlan_tpid          = 1,
    .max_num_of_route_host_addr     = 2048,
    .max_num_of_route_switch_addr   = 16,
    .tpid_entry_idx_max             = 3,
    .tpid_entry_mask_max            = 0xf,
    .protocol_vlan_idx_max          = 7,
    .vlan_fid_max                   = 255,
    .flowctrl_thresh_max            = 0xFFF,
    .flowctrl_pauseOn_page_packet_max       = 0xF,
    .flowctrl_pauseOn_page_packet_len_max   = 0xFFFF,
    .pri_of_selection_max           = 7,
    .pri_of_selection_min           = 0,
    .pri_sel_group_index_max        = 3,
    .queue_weight_max               = 1023,
    .rate_of_bandwidth_max          = 0xFFFF,
    .thresh_of_igr_port_pause_congest_group_idx_max   = 3,
    .thresh_of_igr_bw_flowctrl_min  = 22,
    .thresh_of_igr_bw_flowctrl_max  = 0xFFFF,
    .max_num_of_fastPath_of_rate    = 3,
    .rate_of_storm_control_max      = 0xFFFFF,
    .rate_of_storm_proto_control_max= 0xFF,
    .burst_rate_of_storm_control_min= 1700,
    .burst_rate_of_storm_control_max= 0xFFFF,
    .burst_rate_of_10ge_storm_control_min= 2650,
    .burst_rate_of_10ge_storm_control_max    = 0xFFFFF,
    .burst_rate_of_acl_meter_dlb_min= 17,
    .burst_rate_of_acl_meter_dlb_max= 0xFFFF,
    .internal_priority_max          = 7,
    .drop_precedence_max            = 2,
    .priority_remap_group_idx_max   = 3,
    .priority_remark_group_idx_max  = 0,
    .wred_weight_max                = 0x3FF,
    .wred_mpd_max                   = 0xF,
    .wred_drop_probability_max      = 0xFF,
    .acl_rate_max                   = 0xFFFF,
    .max_num_of_l2_hash_algo        = 2,
    .l2_learn_limit_cnt_max         = 0x4040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x4000,
    .l2_learn_limit_cnt_disable     = 0x7FFF,
    .l2_fid_learn_limit_entry_max   = 32,
    .l2_notification_bp_thresh_max  = 1024,
    .eee_queue_thresh_max           = 0xFF,
    .sec_minIpv6FragLen_max         = 0xFFFF,
    .sec_maxPingLen_max             = 0xFFFF,
    .sec_smurfNetmaskLen_max        = 32,
    .sflow_rate_max                 = 0xFFFF,
    .rate_of_bandwidth_max_fe_port  = 0x186A,
    .rate_of_bandwidth_max_ge_port  = 0xF424,
    .rate_of_bandwidth_max_10ge_port= 0x98968,
    .max_num_of_c2sc_entry          = 1024,
    .max_num_of_c2sc_blk_entry      = 256,
    .max_num_of_c2sc_blk            = 4,
    .max_num_of_sc2c_entry          = 1024,
    .max_num_of_vlan_prof           = 8,
    .max_frame_len                  = 12288,
    .max_num_of_mcast_entry         = 4096,
    .max_num_of_vlan_port_iso_entry = 16,
    .max_num_of_mpls_lib            = 16,
    .max_num_of_led_entity          = 3,
    .miim_page_id_max               = 8191,
    .max_num_of_dying_gasp_pkt_cnt  = 8,
    .dying_gasp_sustain_time_max    = 0xFFFF,
    .max_num_of_rma_user_defined    = 2,
    .time_nsec_max                  = 999999999,
    .max_num_of_ethdm_rx_timestamp  = 64,
#else
    .max_num_of_mirror              = 4,
    .max_num_of_trunk               = 16,
    .max_num_of_trunkMember         = 8,
    .max_num_of_trunk_algo          = 4,
    .trunk_algo_shift_max           = 4,
    .max_num_of_dumb_trunkMember    = 8,
    .max_num_of_trunkHashVal        = 32,
    .max_num_of_msti                = 256,
    .max_num_of_metering            = 512,
    .max_num_of_meter_block         = 16,
    .max_num_of_pie_block           = 18,
    .max_num_of_acl_block_templateSelector  = 2,
    .max_num_of_pie_blockSize       = 128,
    .max_num_of_pie_counter         = 1024,
    .max_num_of_pie_template        = 8,
    .pie_user_template_id_min       = 5,
    .pie_user_template_id_max       = 7,
    .max_num_of_field_selector      = 12,
    .max_num_of_range_check_srcPort = 16,
    .max_num_of_range_check_dstPort = 16,
    .max_num_of_range_check_ip      = 8,
    .max_num_of_range_check_vid     = 32,
    .max_num_of_range_check_l4Port  = 8,
    .max_num_of_range_check_pktLen  = 8,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_cvlan_tpid          = 4,
    .max_num_of_svlan_tpid          = 4,
    .max_num_of_evlan_tpid          = 1,
    .max_num_of_route_host_addr     = 2048,
    .max_num_of_route_switch_addr   = 16,
    .tpid_entry_idx_max             = 3,
    .tpid_entry_mask_max            = 0xf,
    .protocol_vlan_idx_max          = 7,
    .vlan_fid_max                   = 255,
    .flowctrl_thresh_max            = 0xFFF,
    .flowctrl_pauseOn_page_packet_max       = 0xF,
    .flowctrl_pauseOn_page_packet_len_max   = 0xFFFF,
    .pri_of_selection_max           = 7,
    .pri_of_selection_min           = 0,
    .pri_sel_group_index_max        = 3,
    .queue_weight_max               = 1023,
    .rate_of_bandwidth_max          = 0xFFFF,
    .thresh_of_igr_port_pause_congest_group_idx_max   = 3,
    .thresh_of_igr_bw_flowctrl_min  = 22,
    .thresh_of_igr_bw_flowctrl_max  = 0xFFFF,
    .max_num_of_fastPath_of_rate    = 3,
    .rate_of_storm_control_max      = 0xFFFFF,
    .rate_of_storm_proto_control_max= 0xFF,
    .burst_rate_of_storm_control_min= 1700,
    .burst_rate_of_storm_control_max= 0xFFFF,
    .burst_rate_of_10ge_storm_control_min = 2650,
    .burst_rate_of_10ge_storm_control_max = 0xFFFFF,
    .burst_size_of_acl_meter_min    = 17,
    .burst_size_of_acl_meter_max    = 0xFFFF,
    .internal_priority_max          = 7,
    .drop_precedence_max            = 2,
    .priority_remap_group_idx_max   = 3,
    .priority_remark_group_idx_max  = 0,
    .wred_weight_max                = 0x3FF,
    .wred_mpd_max                   = 0xF,
    .wred_drop_probability_max      = 0xFF,
    .acl_rate_max                   = 0xFFFF,
    .max_num_of_l2_hash_algo        = 2,
    .l2_learn_limit_cnt_max         = 0x4040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x4000,
    .l2_learn_limit_cnt_disable     = 0x7FFF,
    .l2_fid_learn_limit_entry_max   = 32,
    .l2_notification_bp_thresh_max  = 1024,
    .eee_queue_thresh_max           = 0xFF,
    .sec_minIpv6FragLen_max         = 0xFFFF,
    .sec_maxPingLen_max             = 0xFFFF,
    .sec_smurfNetmaskLen_max        = 32,
    .sflow_rate_max                 = 0xFFFF,
    .rate_of_bandwidth_max_fe_port  = 0x186A,
    .rate_of_bandwidth_max_ge_port  = 0xF424,
    .rate_of_bandwidth_max_10ge_port= 0x98968,
    .max_num_of_c2sc_entry          = 1024,
    .max_num_of_c2sc_blk_entry      = 256,
    .max_num_of_c2sc_blk            = 4,
    .max_num_of_sc2c_entry          = 1024,
    .max_num_of_vlan_prof           = 8,
    .max_frame_len                  = 12288,
    .max_num_of_mcast_entry         = 4096,
    .max_num_of_vlan_port_iso_entry = 16,
    .max_num_of_mpls_lib            = 256,
    .max_num_of_led_entity          = 3,
    .miim_page_id_max               = 8191,
    .max_num_of_dying_gasp_pkt_cnt  = 8,
    .dying_gasp_sustain_time_max    = 0xFFFF,
    .max_num_of_rma_user_defined    = 2,
    .time_nsec_max                  = 999999999,
    .max_num_of_ethdm_rx_timestamp  = 64,
#endif
};

/* Normal 8390 Chip PER_PORT block information */
static rt_macPpInfo_t rtl8390_macPpInfo[] =
{
    {
      /* lowerbound_addr */ 0x8000,
      /* upperbound_addr */ 0x9A7C,
      /* interval */          0x80,
    },
};

/* Definition Structure & Supported Mode Lists */ // add
typedef enum rt8390_supported_mode_e
{
    RT8390_MODE_48G_4G = 0,
    RT8390_MODE_48G_4G_APP_48G,
    RT8390_MODE_48G_4G_APP_44G_4G,
    RT8390_MODE_48G_4G_APP_24G_4G,
    RT8390_MODE_48G_4G_APP_20G_4G,
    RT8390_MODE_48G_2G = 8,
    RT8390_MODE_48G_2G_APP_24G_2G,
    RT8390_MODE_24G_2TG_RXAUI = 16,
    RT8390_MODE_24G_2TG_RXAUI_APP_20G_4G_2TG_RXAUI,
    RT8390_MODE_24G_2TG_XFP = 24,
    RT8390_MODE_24G_2TG_XFP_APP_20G_4G_2TG_XFP,
    RT8390_MODE_24G_2TG_XFP_APP_24FB_2TG_XFP,
    RT8390_MODE_48FE_4G = 32,
    RT8390_MODE_48FE_4G_APP_24FE_4G,
    RT8390_MODE_48FE_2G = 40,
    RT8390_MODE_48FE_2G_APP_24FE_2G,
}rt8390_supported_mode_t;
#endif  /* end of defined(CONFIG_SDK_RTL8390) */

/******************************************************/
/************Add CONFIG_SDK_RTL8380 in here****************/
/*****************************************************/
#if defined(CONFIG_SDK_RTL8380)
/* Normal 8380 Chip Port Information */
static rt_portinfo_t rtl8380_port_info_24G_4BX =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    {
        RT_FE_PORT /*P0 */, RT_FE_PORT /*P1 */, RT_FE_PORT /*P2 */, RT_FE_PORT/*P3 */, RT_FE_PORT /*P4 */,
        RT_FE_PORT /*P5 */, RT_FE_PORT /*P6 */, RT_FE_PORT /*P7 */, RT_FE_PORT/*P8 */, RT_FE_PORT /*P9 */,
        RT_FE_PORT /*P10*/, RT_FE_PORT /*P11*/, RT_FE_PORT /*P12*/, RT_FE_PORT/*P13*/, RT_FE_PORT /*P14*/,
        RT_FE_PORT /*P15*/, RT_FE_PORT /*P16*/, RT_FE_PORT /*P17*/, RT_FE_PORT/*P18*/, RT_FE_PORT /*P19*/,
        RT_FE_PORT /*P20*/, RT_FE_PORT /*P21*/, RT_FE_PORT /*P22*/, RT_FE_PORT/*P23*/, RT_FE_PORT   /*P24*/,
        RT_FE_PORT /*P25*/, RT_FE_PORT   /*P26*/, RT_FE_PORT /*P27*/, RT_CPU_PORT /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
     },
#else
    {
        RT_GE_PORT   /*P0 */, RT_GE_PORT   /*P1 */, RT_GE_PORT /*P2 */, RT_GE_PORT  /*P3 */, RT_GE_PORT   /*P4 */,
        RT_GE_PORT   /*P5 */, RT_GE_PORT   /*P6 */, RT_GE_PORT /*P7 */, RT_GE_PORT  /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT /*P12*/, RT_GE_PORT  /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT /*P17*/, RT_GE_PORT  /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT /*P22*/, RT_GE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT /*P27*/, RT_CPU_PORT /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
     },
#endif /* end of CONFIG_SDK_FPGA_PLATFORM */
};

static rt_portinfo_t rtl8380_port_info_16G_4BX =
{
    {
        RT_PORT_NONE /*P0 */, RT_PORT_NONE /*P1 */, RT_PORT_NONE /*P2 */, RT_PORT_NONE /*P3 */, RT_PORT_NONE /*P4 */,
        RT_PORT_NONE /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_GE_PORT  /*P8 */, RT_GE_PORT   /*P9 */,
        RT_GE_PORT   /*P10*/, RT_GE_PORT   /*P11*/, RT_GE_PORT /*P12*/, RT_GE_PORT  /*P13*/, RT_GE_PORT   /*P14*/,
        RT_GE_PORT   /*P15*/, RT_GE_PORT   /*P16*/, RT_GE_PORT /*P17*/, RT_GE_PORT  /*P18*/, RT_GE_PORT   /*P19*/,
        RT_GE_PORT   /*P20*/, RT_GE_PORT   /*P21*/, RT_GE_PORT /*P22*/, RT_GE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT /*P27*/, RT_CPU_PORT /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
     },
};

static rt_portinfo_t rtl8380_port_info_24FE_4G =
{
    {
        RT_FE_PORT   /*P0 */, RT_FE_PORT   /*P1 */, RT_FE_PORT /*P2 */, RT_FE_PORT  /*P3 */, RT_FE_PORT   /*P4 */,
        RT_FE_PORT   /*P5 */, RT_FE_PORT   /*P6 */, RT_FE_PORT /*P7 */, RT_FE_PORT  /*P8 */, RT_FE_PORT   /*P9 */,
        RT_FE_PORT   /*P10*/, RT_FE_PORT   /*P11*/, RT_FE_PORT /*P12*/, RT_FE_PORT  /*P13*/, RT_FE_PORT   /*P14*/,
        RT_FE_PORT   /*P15*/, RT_FE_PORT   /*P16*/, RT_FE_PORT /*P17*/, RT_FE_PORT  /*P18*/, RT_FE_PORT   /*P19*/,
        RT_FE_PORT   /*P20*/, RT_FE_PORT   /*P21*/, RT_FE_PORT /*P22*/, RT_FE_PORT  /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT /*P27*/, RT_CPU_PORT /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
     },
};

static rt_portinfo_t rtl8380_port_info_16FE_4G =
{
    {
        RT_PORT_NONE /*P0 */, RT_PORT_NONE /*P1 */, RT_PORT_NONE /*P2 */, RT_PORT_NONE /*P3 */, RT_PORT_NONE /*P4 */,
        RT_PORT_NONE /*P5 */, RT_PORT_NONE /*P6 */, RT_PORT_NONE /*P7 */, RT_FE_PORT   /*P8 */, RT_FE_PORT   /*P9 */,
        RT_FE_PORT   /*P10*/, RT_FE_PORT   /*P11*/, RT_FE_PORT   /*P12*/, RT_FE_PORT   /*P13*/, RT_FE_PORT   /*P14*/,
        RT_FE_PORT   /*P15*/, RT_FE_PORT   /*P16*/, RT_FE_PORT   /*P17*/, RT_FE_PORT   /*P18*/, RT_FE_PORT   /*P19*/,
        RT_FE_PORT   /*P20*/, RT_FE_PORT   /*P21*/, RT_FE_PORT   /*P22*/, RT_FE_PORT   /*P23*/, RT_GE_PORT   /*P24*/,
        RT_GE_PORT   /*P25*/, RT_GE_PORT   /*P26*/, RT_GE_PORT   /*P27*/, RT_CPU_PORT  /*P28*/, RT_PORT_NONE /*P29*/,
        RT_PORT_NONE /*P30*/, RT_PORT_NONE /*P31*/
     },
};

static rt_serdesInfo_t rtl8380_serdes_info =
{   /* 8380 Chip Serdes Information: 24G + 4G */
    {
        RT_SERDES_5G/*SDS0 */, RT_SERDES_5G/*SDS1 */, RT_SERDES_5G/*SDS2 */, RT_SERDES_5G  /*SDS3 */, RT_SERDES_5G/*SDS4 */,
        RT_SERDES_5G/*SDS5 */, RT_SERDES_NONE/*SDS6*/
    },
};

/* Normal 8380 Chip Port Information */
static rt_register_capacity_t rtl8380m_capacityInfo =
{
#if defined(CONFIG_SDK_FPGA_PLATFORM)
    .max_num_of_mirror              = 4,
    .max_num_of_trunk               = 8,
    .max_num_of_trunkMember         = 8,
    .max_num_of_trunk_algo          = 2,
    .trunk_algo_shift_max           = 3,
    .max_num_of_dumb_trunkMember    = 8,
    .max_num_of_trunkHashVal        = 16,
    .max_num_of_msti                = 64,
    .max_num_of_metering            = 4, /* 64 */
    .max_num_of_pie_block           = 12,
    .max_num_of_acl_block_templateSelector  = 3,
    .max_num_of_pie_logical_block   = 12,
    .max_num_of_pie_blockSize       = 128,
    .max_num_of_pie_counter         = 128,
    .max_num_of_pie_action          = 1024,
    .max_num_of_pie_template        = 8,
    .pie_user_template_id_min       = 5 ,
    .pie_user_template_id_max       = 7 ,
    .max_num_of_pie_payload         = 0,
    .max_num_of_field_selector      = 4,
    .max_num_of_range_check_srcPort = 16,
    .max_num_of_range_check_dstPort = 0,
    .max_num_of_range_check_ip      = 8,
    .max_num_of_range_check_vid     = 16,
    .max_num_of_range_check_l4Port  = 16,
    .max_num_of_range_check_pktLen  = 16,
    .max_num_of_pattern_match_data  = 0,
    .pattern_match_port_max         = 0,
    .pattern_match_port_min         = 0,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_igrQueue            = 4,
    .min_num_of_igrQueue            = 1,
    .max_num_of_cvlan_tpid          = 4,
    .max_num_of_svlan_tpid          = 4,
    .max_num_of_evlan_tpid          = 1,
    .max_num_of_route_host_addr 	= 512,
    .tpid_entry_idx_max             = 3,
    .tpid_entry_mask_max            = 0xf,
    .protocol_vlan_idx_max          = 0,
    .vlan_fid_max                   = 63,
    .flowctrl_thresh_max            = 0x7FF,
    .flowctrl_pauseOn_page_packet_max   = 0x1FF,
    .pri_of_selection_max           = 7,
    .pri_of_selection_min           = 0,
    .pri_sel_group_index_max        = 3,
    .queue_weight_max               = 127,
    .rate_of_bandwidth_max          = 0x3FFFF,
    .thresh_of_igr_port_pause_congest_group_idx_max   = 3,
    .thresh_of_igr_bw_flowctrl_max  = 0x3FF,
    .max_num_of_fastPath_of_rate    = 0,
    .rate_of_storm_control_max      = 0x3FFFF,
    .burst_rate_of_storm_control_max= 0xFFFF,
    .internal_priority_max          = 7,
    .drop_precedence_max            = 0,
    .priority_remap_group_idx_max   = 0,
    .priority_remark_group_idx_max  = 0,
    .wred_weight_max                = 0,
    .wred_mpd_max                   = 0,
    .acl_rate_max                   = 0x3FFFF,
    .max_num_of_l2_hash_algo		= 2,
    .l2_learn_limit_cnt_max         = 0x2040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x2000,
    .l2_learn_limit_cnt_disable     = 0x3FFF,
    .l2_fid_learn_limit_entry_max   = 8,
    .eee_queue_thresh_max           = 0xFFFF,
    .sec_minIpv6FragLen_max         = 0xFFFF,
    .sec_maxPingLen_max             = 0xFFFF,
    .sec_smurfNetmaskLen_max        = 32,
    .sflow_rate_max                 = 0xFFFF,
    .rate_of_bandwidth_max_fe_port  = 0x186A,
    .rate_of_bandwidth_max_ge_port  = 0xF424,
    .rate_of_bandwidth_max_10ge_port= 0,
    .max_num_of_mcast_entry         = 512,
    .max_num_of_vlan_port_iso_entry = 16,
    .max_num_of_sc2c_entry          = 128,
    .max_num_of_vlan_prof           = 8,
    .max_frame_len                  = 10240,
    .max_num_of_led_entity          = 3,
    .miim_page_id_max               = 0xfff,
#else
    .max_num_of_mirror              = 4,
    .max_num_of_trunk               = 8,
    .max_num_of_trunkMember         = 8,
    .max_num_of_trunk_algo          = 2,
    .trunk_algo_shift_max           = 3,
    .max_num_of_dumb_trunkMember    = 8,
    .max_num_of_trunkHashVal        = 16,
    .max_num_of_msti                = 64,
    .max_num_of_metering            = 256,
    .max_num_of_pie_block           = 12,
    .max_num_of_acl_block_templateSelector  = 3,
    .max_num_of_pie_logical_block   = 12,
    .max_num_of_pie_blockSize       = 128,
    .max_num_of_pie_counter         = 128,
    .max_num_of_pie_action          = 1024,
    .max_num_of_pie_template        = 8,
    .pie_user_template_id_min       = 5 ,
    .pie_user_template_id_max       = 7 ,
    .max_num_of_pie_payload         = 0,
    .max_num_of_field_selector      = 4,
    .max_num_of_range_check_srcPort = 16,
    .max_num_of_range_check_dstPort = 0,
    .max_num_of_range_check_ip      = 8,
    .max_num_of_range_check_vid     = 16,
    .max_num_of_range_check_l4Port  = 16,
    .max_num_of_range_check_pktLen  = 16,
    .max_num_of_pattern_match_data  = 0,
    .pattern_match_port_max         = 0,
    .pattern_match_port_min         = 0,
    .max_num_of_l2_hashdepth        = 4,
    .max_num_of_queue               = 8,
    .min_num_of_queue               = 1,
    .max_num_of_igrQueue            = 4,
    .min_num_of_igrQueue            = 1,
    .max_num_of_cvlan_tpid          = 4,
    .max_num_of_svlan_tpid          = 4,
    .max_num_of_evlan_tpid          = 1,
    .max_num_of_route_host_addr 	= 512,
    .tpid_entry_idx_max             = 3,
    .tpid_entry_mask_max            = 0xf,
    .protocol_vlan_idx_max          = 0,
    .vlan_fid_max                   = 63,
    .flowctrl_thresh_max            = 0x7FF,
    .flowctrl_pauseOn_page_packet_max   = 0x1FF,
    .pri_of_selection_max           = 7,
    .pri_of_selection_min           = 0,
    .pri_sel_group_index_max        = 3,
    .queue_weight_max               = 127,
    .rate_of_bandwidth_max          = 0x3FFFF,
    .thresh_of_igr_port_pause_congest_group_idx_max   = 3,
    .thresh_of_igr_bw_flowctrl_max  = 0x3FF,
    .max_num_of_fastPath_of_rate    = 0,
    .rate_of_storm_control_max      = 0x3FFFF,
    .burst_rate_of_storm_control_max= 0xFFFF,
    .internal_priority_max          = 7,
    .drop_precedence_max            = 0,
    .priority_remap_group_idx_max   = 0,
    .priority_remark_group_idx_max  = 0,
    .wred_weight_max                = 0,
    .wred_mpd_max                   = 0,
    .acl_rate_max                   = 0x3FFFF,
    .max_num_of_l2_hash_algo		= 2,
    .l2_learn_limit_cnt_max         = 0x2040,
    .l2_learn_limit_cnt_wo_cam_max  = 0x2000,
    .l2_learn_limit_cnt_disable     = 0x3FFF,
    .l2_fid_learn_limit_entry_max   = 8,
    .eee_queue_thresh_max           = 0xFFFF,
    .sec_minIpv6FragLen_max         = 0xFFFF,
    .sec_maxPingLen_max             = 0xFFFF,
    .sec_smurfNetmaskLen_max        = 32,
    .sflow_rate_max                 = 0xFFFF,
    .rate_of_bandwidth_max_fe_port  = 0x186A,
    .rate_of_bandwidth_max_ge_port  = 0xF424,
    .rate_of_bandwidth_max_10ge_port= 0,
    .max_num_of_mcast_entry         = 512,
    .max_num_of_vlan_port_iso_entry = 16,
    .max_num_of_sc2c_entry          = 128,
    .max_num_of_vlan_prof           = 8,
    .max_frame_len                  = 10240,
    .max_num_of_led_entity          = 3,
    .miim_page_id_max               = 0xfff,
#endif
};

/* Normal 8380 Chip PER_PORT block information */
static rt_macPpInfo_t rtl8380_macPpInfo[] =
{
    {
      /* lowerbound_addr */ 0xd560,
      /* upperbound_addr */ 0xe3df,
      /* interval */         0x80,
    },
    {
      /* lowerbound_addr */ 0xbfe0,
      /* upperbound_addr */ 0xce5f,
      /* interval */         0x80,
    },
};
#endif  /* end of defined(CONFIG_SDK_RTL8380) */


/* Supported mac chip lists */
static rt_device_t supported_devices[] =
{
#if defined(CONFIG_SDK_RTL8389)
    /* RT_DEVICE_RTL8389M_A */
    {
        RTL8389M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8389M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8389_FAMILY_ID,
        &rtl8389_port_info,
        &rtl8389_capacityInfo,
        1,
        rtl8389_macPpInfo
    },

    /* RT_DEVICE_RTL8389_A */
    {
        RTL8389L_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8389M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8389_FAMILY_ID,
        &rtl8389_port_info,
        &rtl8389_capacityInfo,
        1,
        rtl8389_macPpInfo
    },

    /* RT_DEVICE_RTL8329M_A */
    {
        RTL8329M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8389M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8389_FAMILY_ID,
        &rtl8329_port_info,
        &rtl8389_capacityInfo,
        1,
        rtl8389_macPpInfo
    },

    /* RT_DEVICE_RTL8329_A */
    {
        RTL8329_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8389M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8389_FAMILY_ID,
        &rtl8329_port_info,
        &rtl8389_capacityInfo,
        1,
        rtl8389_macPpInfo
    },

    /* RT_DEVICE_RTL8377M_A */
    {
        RTL8377M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8389M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8389_FAMILY_ID,
        &rtl8377_port_info,
        &rtl8389_capacityInfo,
        1,
        rtl8389_macPpInfo
    },
#endif /* end of defined(CONFIG_SDK_RTL8389) */

#if defined(CONFIG_SDK_RTL8328)
    /* RT_DEVICE_RTL8328M_A */
    {
        RTL8328M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8328M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8328_FAMILY_ID,
        &rtl8328_port_info,
        &rtl8328m_capacityInfo,
        1,
        rtl8328_macPpInfo
    },

    /* RT_DEVICE_RTL8328S_A */
    {
        RTL8328S_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8328M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8328_FAMILY_ID,
        &rtl8328_port_info,
        &rtl8328s_capacityInfo,
        1,
        rtl8328_macPpInfo
    },

    /* RT_DEVICE_RTL8328L_A */
    {
        RTL8328L_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8328M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8328_FAMILY_ID,
        &rtl8328_port_info,
        &rtl8328s_capacityInfo,
        1,
        rtl8328_macPpInfo
    },

    /* RT_DEVICE_RTL8328S_C */
    {
        RTL8328S_CHIP_ID,
        CHIP_REV_ID_C,
        RTL8328M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8328_FAMILY_ID,
        &rtl8328_port_info,
        &rtl8328s_capacityInfo_C,
        1,
        rtl8328_macPpInfo
    },

    /* RT_DEVICE_RTL8328L_C */
    {
        RTL8328L_CHIP_ID,
        CHIP_REV_ID_C,
        RTL8328M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8328_FAMILY_ID,
        &rtl8328_port_info,
        &rtl8328s_capacityInfo_C,
        1,
        rtl8328_macPpInfo
    },
#endif /* end of defined(CONFIG_SDK_RTL8328) */

#if defined(CONFIG_SDK_RTL8390)
    /* RT_DEVICE_RTL8352M_A */
    {
        RTL8352M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8350_FAMILY_ID,
        &rtl8352_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8353M_A */
    {
        RTL8353M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8350_FAMILY_ID,
        &rtl8353_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8391M_A */
    {
        RTL8391M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8390_FAMILY_ID,
        &rtl8391_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8392M_A */
    {
        RTL8392M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8390_FAMILY_ID,
        &rtl8392_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8393M_A */
    {
        RTL8393M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8390_FAMILY_ID,
        &rtl8393_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8396M_A */
    {
        RTL8396M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8390_FAMILY_ID,
        &rtl8396_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8352MES_A */
    {
        RTL8352MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8350_FAMILY_ID,
        &rtl8352_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8353MES_A */
    {
        RTL8353MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8350_FAMILY_ID,
        &rtl8353_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8392MES_A */
    {
        RTL8392MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8390_FAMILY_ID,
        &rtl8392_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8393MES_A */
    {
        RTL8393MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8390_FAMILY_ID,
        &rtl8393_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },

    /* RT_DEVICE_RTL8396MES_A */
    {
        RTL8396MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8390M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8390_FAMILY_ID,
        &rtl8396_port_info,
        &rtl8390m_capacityInfo,
        1,
        rtl8390_macPpInfo,
        &rtl8393_serdes_info,
    },
#endif /* end of defined(CONFIG_SDK_RTL8390) */


/******************************************/
/* Now user 8382/8380/8332/8330 chip info */
/******************************************/
#if defined(CONFIG_SDK_RTL8380)
    /* RT_DEVICE_RTL8382M_A */
    {
        RTL8382M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8380_FAMILY_ID,
        &rtl8380_port_info_24G_4BX,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },

    /* RT_DEVICE_RTL8380M_A */
    {
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8380_FAMILY_ID,
        &rtl8380_port_info_16G_4BX,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },

    /* RT_DEVICE_RTL8332M_A */
    {
        RTL8332M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8330_FAMILY_ID,
        &rtl8380_port_info_24FE_4G,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },

    /* RT_DEVICE_RTL8330M_A */
    {
        RTL8330M_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8330_FAMILY_ID,
        &rtl8380_port_info_16FE_4G,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },
    /* For engineer sample chip */
    /* RT_DEVICE_RTL8382MES_A */
    {
        RTL8382MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8380_FAMILY_ID,
        &rtl8380_port_info_24G_4BX,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },

    /* RT_DEVICE_RTL8380MES_A */
    {
        RTL8380MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8380_FAMILY_ID,
        &rtl8380_port_info_16G_4BX,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },

    /* RT_DEVICE_RTL8332MES_A */
    {
        RTL8332MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8330_FAMILY_ID,
        &rtl8380_port_info_24FE_4G,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },

    /* RT_DEVICE_RTL8330MES_A */
    {
        RTL8330MES_CHIP_ID,
        CHIP_REV_ID_A,
        RTL8380M_CHIP_ID,
        CHIP_REV_ID_A,
        CHIP_AFLAG_LEXRA,
        RTL8330_FAMILY_ID,
        &rtl8380_port_info_16FE_4G,
        &rtl8380m_capacityInfo,
        2,
        rtl8380_macPpInfo,
        &rtl8380_serdes_info,
    },
#endif
}; /* end of supported_devices */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      hal_mode_info_get
 * Description:
 *      Get chip mode and application model.
 * Input:
 *      unit           - unit id
 * Output:
 *      pChip_mode     - pointer buffer of chip mode
 *      pApp_mode      - pointer buffer of application model
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
hal_mode_info_get(uint32 unit, uint32 *pChip_mode, uint32 *pApp_mode)
{
#if defined(CONFIG_SDK_RTL8390)
    uint32 value = 0;
#endif

#if defined(CONFIG_SDK_RTL8380)
    uint32 mode = 0;
#endif /*CONFIG_SDK_RTL8380*/

    /* parameter check */
    RT_PARAM_CHK((NULL == pChip_mode), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pApp_mode), RT_ERR_NULL_POINTER);

#if defined(CONFIG_SDK_RTL8390)
    if (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8350_FAMILY_ID(unit))
    {
        value = 0xA;
        /*if ((reg_field_write(unit, CYPRESS_CHIP_INFOr, CYPRESS_CHIP_INFO_ENf, &value)) != RT_ERR_OK)
            return RT_ERR_FAILED;*/

        if ((reg_field_read(unit, CYPRESS_MAC_EFUSE_CTRLr, CYPRESS_CHIP_MODEf, pChip_mode)) != RT_ERR_OK)
            return RT_ERR_FAILED;

        if ((reg_field_read(unit, CYPRESS_MAC_EFUSE_CTRLr, CYPRESS_APP_MODEf, pApp_mode)) != RT_ERR_OK)
            return RT_ERR_FAILED;

        value = 0;
        /*if ((reg_field_write(unit, CYPRESS_CHIP_INFOr, CYPRESS_CHIP_INFO_ENf, &value)) != RT_ERR_OK)
            return RT_ERR_FAILED;*/
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if (HAL_IS_RTL8380_FAMILY_ID(unit) || HAL_IS_RTL8330_FAMILY_ID(unit))
    {
        ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, 0x3);
        ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, 0xA0000000);

        //if ((reg_field_read(unit, MAPLE_MODEL_INFOr, MAPLE_MODEL_IDf, pChip_mode)) != RT_ERR_OK)
        //    return RT_ERR_FAILED;
        if ((ioal_mem32_field_read(unit, RTL8380_MODEL_INFO_ADDR, RTL8380_MODEL_INFO_MODEL_ID_OFFSET, RTL8380_MODEL_INFO_MODEL_ID_MASK, pChip_mode)) != RT_ERR_OK)
            return RT_ERR_FAILED;
        mode = *pChip_mode;
        (*pChip_mode) = mode & 0x03;
        (*pApp_mode) =  mode & 0x1C;

        ioal_mem32_write(unit, RTL8380_CHIP_INFO_ADDR, 0x0);
        ioal_mem32_write(unit, RTL8380_INT_RW_CTRL_ADDR, 0x0);
    }
#endif

    return RT_ERR_OK;
} /* end of hal_mode_info_get */

/* Function Name:
 *      hal_chip_mode_get
 * Description:
 *      Get chip mode to correct port info.
 * Input:
 *      unit       - unit id
 *      pDev       - pointer of device
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      It's for RTL8390 only.
 */
int32
hal_chip_mode_get(uint32 unit, rt_device_t *pDev)
{
    int32   ret = RT_ERR_FAILED;
    uint32  chip_mode = 0, app_mode = 0;

    if ((ret = hal_mode_info_get(unit, &chip_mode, &app_mode)) != RT_ERR_OK)
    {
        return ret;
    }

#if defined (CONFIG_SDK_RTL8390)
  //#if 1
  //  pDev->pPortinfo = &rtl8393_port_info;
  //  pDev->pSerdesInfo = &rtl8393_serdes_info;
  //#else
    switch ((chip_mode << 3) | (app_mode))
    {
#if 0 /* Add the extra application mode if need */
        case RT8390_MODE_48G_4G_APP_48G:
            pDev->pPortinfo = &rtl8390_port_info_48G;
            pDev->pSerdesInfo = &rtl8390_serdes_info_48G;
            break;
        case RT8390_MODE_48G_4G_APP_44G_4G:
            pDev->pPortinfo = &rtl8390_port_info_44G_4G;
            pDev->pSerdesInfo = &rtl8390_serdes_info_44G_4G;
            break;
        case RT8390_MODE_24G_2TG_XFP:
            pDev->pPortinfo = &rtl8390_port_info_24G_2_10G_fiber;
            pDev->pSerdesInfo = &rtl8390_serdes_info_24G_2_10G_fiber;
            break;
        case RT8390_MODE_48FE_4G:
            pDev->pPortinfo = &rtl8390_port_info_48FE_4G;
            pDev->pSerdesInfo = &rtl8390_serdes_info_48FE_4G;
            break;
#endif
        default:
            break;
    }
  //#endif
#endif

    return RT_ERR_OK;
} /* end of hal_chip_mode_get */


/* Function Name:
 *      hal_find_device
 * Description:
 *      Find the mac chip from SDK supported mac device lists.
 * Input:
 *      chip_id     - chip id
 *      chip_rev_id - chip revision id
 * Output:
 *      None
 * Return:
 *      NULL        - Not found
 *      Otherwise   - Pointer of mac chip structure that found
 * Note:
 *      The function have take care the forward compatible in revision.
 *      Return one recently revision if no extra match revision.
 */
rt_device_t *
hal_find_device(uint32 chip_id, uint32 chip_rev_id)
{
    uint32  dev_idx;
    uint32  most_rev_id = 0;
    rt_device_t *pMatchDevice = NULL;

    RT_PARAM_CHK((chip_rev_id > CHIP_REV_ID_MAX), NULL);

    /* find out appropriate supported revision from supported_devices lists
     */
    for (dev_idx = 0; dev_idx < RT_DEVICE_END; dev_idx++)
    {
        if (supported_devices[dev_idx].chip_id == chip_id)
        {
            if (supported_devices[dev_idx].chip_rev_id == chip_rev_id)
            {
                /* Match and return this MAC device */
                return (&supported_devices[dev_idx]);
            }
            else if ((supported_devices[dev_idx].chip_rev_id < chip_rev_id) &&
                     (supported_devices[dev_idx].chip_rev_id >= most_rev_id))
            {
                /* Match better candidate of MAC device */
                most_rev_id = supported_devices[dev_idx].chip_rev_id;
                pMatchDevice = &supported_devices[dev_idx];
            }
        }
    }

    return (pMatchDevice);
} /* end of hal_find_device */


/* Function Name:
 *      hal_get_driver_id
 * Description:
 *      Get its driver of the mac chip based on chip id and chip revision id.
 * Input:
 *      chip_id        - chip id
 *      chip_rev_id    - chip revision id
 * Output:
 *      pDriver_id     - pointer buffer of driver id
 *      pDriver_rev_id - pointer buffer of driver revision id
 * Return:
 *      RT_ERR_OK             - OK
 *      RT_ERR_NULL_POINTER   - input parameter is null pointer
 *      RT_ERR_CHIP_NOT_FOUND - The chip can not found
 * Note:
 *      None
 */
int32
hal_get_driver_id(
    uint32 chip_id,
    uint32 chip_rev_id,
    uint32 *pDriver_id,
    uint32 *pDriver_rev_id)
{
    rt_device_t *pDev = NULL;

    RT_PARAM_CHK((NULL == pDriver_id), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pDriver_rev_id), RT_ERR_NULL_POINTER);

    if ((pDev = hal_find_device(chip_id, chip_rev_id)) == NULL)
    {
        return RT_ERR_CHIP_NOT_FOUND;
    }

    *pDriver_id = pDev->driver_id;
    *pDriver_rev_id = pDev->driver_rev_id;

    return RT_ERR_OK;
} /* end of hal_get_driver_id */

/* Function Name:
 *      hal_isPpBlock_check
 * Description:
 *      Check the register is PER_PORT block or not?
 * Input:
 *      unit       - unit id
 *      addr       - register address
 * Output:
 *      pIsPpBlock - pointer buffer of chip is PER_PORT block?
 *      pPpBlockIdx - pointer buffer of PER_PORT block index
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - failed
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32
hal_isPpBlock_check(uint32 unit, uint32 addr, uint32 *pIsPpBlock, uint32 *pPpBlockIdx)
{
    uint32  i, ppBlkNum = 0;

    /* parameter check */
    RT_PARAM_CHK((NULL == pIsPpBlock), RT_ERR_NULL_POINTER);

    ppBlkNum = HAL_GET_MACPP_BLK_NUM(unit);
    for (i = 0; i < ppBlkNum; i++)
    {
        if (addr >= HAL_GET_MACPP_BLK_MIN_ADDR(unit, i) && addr <= HAL_GET_MACPP_BLK_MAX_ADDR(unit, i))
        {
            *pIsPpBlock = TRUE;
            *pPpBlockIdx = i;
            return RT_ERR_OK;
        }
    }

    if (i == ppBlkNum)
    {
        *pIsPpBlock = FALSE;
    }

    return RT_ERR_OK;
} /* end of hal_isPpBlock_check */


