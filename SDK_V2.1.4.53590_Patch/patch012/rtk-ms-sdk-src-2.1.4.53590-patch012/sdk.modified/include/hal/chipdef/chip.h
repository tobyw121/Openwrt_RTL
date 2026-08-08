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

#ifndef __HAL_CHIPDEF_CHIP_H__
#define __HAL_CHIPDEF_CHIP_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>


/*
 * Symbol Definition
 */

/* Definition vendor ID */
#define REALTEK_VENDOR_ID   (0x10EC)
#define REALTEK_MES_CHIP_ID (0x6966)

/* Definition chip ID */
#define RTL8389M_CHIP_ID    (0x83896800)
#define RTL8389L_CHIP_ID    (0x83896000) /* Change RTL8389_CHIP_ID to RTL8389L_CHIP_ID with new value */
#define RTL8329M_CHIP_ID    (0x83296800)
#define RTL8329_CHIP_ID     (0x83290000)
#define RTL8377M_CHIP_ID    (0x83776800)
#define RTL8328M_CHIP_ID    (0x83284800)
#define RTL8328S_CHIP_ID    (0x83289800)
#define RTL8328L_CHIP_ID    (0x83286000)
#define RTL8352M_CHIP_ID    (0x83526800)
#define RTL8353M_CHIP_ID    (0x83536800)
#define RTL8390M_CHIP_ID    (0x83906800)
#define RTL8391M_CHIP_ID    (0x83916800)
#define RTL8392M_CHIP_ID    (0x83926800)
#define RTL8393M_CHIP_ID    (0x83936800)
#define RTL8396M_CHIP_ID    (0x83966800)
#define RTL8352MES_CHIP_ID  (0x83526966)
#define RTL8353MES_CHIP_ID  (0x83536966)
#define RTL8392MES_CHIP_ID  (0x83926966)
#define RTL8393MES_CHIP_ID  (0x83936966)
#define RTL8396MES_CHIP_ID  (0x83966966)
#define RTL8330M_CHIP_ID    (0x83306800)
#define RTL8332M_CHIP_ID    (0x83326800)
#define RTL8380M_CHIP_ID    (0x83806800)
#define RTL8382M_CHIP_ID    (0x83826800)
#define RTL8330MES_CHIP_ID  (0x83306966)
#define RTL8332MES_CHIP_ID  (0x83326966)
#define RTL8380MES_CHIP_ID  (0x83806966)
#define RTL8382MES_CHIP_ID  (0x83826966)

/* Definition family ID */
#define RTL8389_FAMILY_ID   (0x83890000)
#define RTL8328_FAMILY_ID   (0x83280000)
#define RTL8390_FAMILY_ID   (0x83900000)
#define RTL8350_FAMILY_ID   (0x83500000)
#define RTL8380_FAMILY_ID   (0x83800000)
#define RTL8330_FAMILY_ID   (0x83300000)

/* Definition chip revision ID */
#define CHIP_REV_ID_A       (0x0)
#define CHIP_REV_ID_B       (0x1)
#define CHIP_REV_ID_C       (0x2)
#define CHIP_REV_ID_D       (0x3)
#define CHIP_REV_ID_MAX     (255)   /* 0xFF */


/* Definition chip attribute flags - bit-wise */
#define CHIP_AFLAG_PCI      (0x1 << 0)
#define CHIP_AFLAG_LEXRA    (0x1 << 1)


/*
 * Data Type Definition
 */

/* Definition RTL model character */
typedef enum rt_model_char_e
{
    RTL_MOEDL_CHAR_NULL = 0,
    RTL_MOEDL_CHAR_A,
    RTL_MOEDL_CHAR_B,
    RTL_MOEDL_CHAR_C,
    RTL_MOEDL_CHAR_D,
    RTL_MOEDL_CHAR_E,
    RTL_MOEDL_CHAR_F,
    RTL_MOEDL_CHAR_G,
    RTL_MOEDL_CHAR_H,
    RTL_MOEDL_CHAR_I,
    RTL_MOEDL_CHAR_J = 10,
    RTL_MOEDL_CHAR_K,
    RTL_MOEDL_CHAR_L,
    RTL_MOEDL_CHAR_M,
    RTL_MOEDL_CHAR_N,
    RTL_MOEDL_CHAR_O,
    RTL_MOEDL_CHAR_P,
    RTL_MOEDL_CHAR_Q,
    RTL_MOEDL_CHAR_R,
    RTL_MOEDL_CHAR_S,
    RTL_MOEDL_CHAR_T = 20,
    RTL_MOEDL_CHAR_U,
    RTL_MOEDL_CHAR_V,
    RTL_MOEDL_CHAR_W,
    RTL_MOEDL_CHAR_X,
    RTL_MOEDL_CHAR_Y,
    RTL_MOEDL_CHAR_Z,
    RTL_MOEDL_CHAR_END = 27
} rt_model_char_t;

/* Definition Structure & Supported Device Lists */
typedef enum rt_supported_device_e
{
#if defined(CONFIG_SDK_RTL8389)
    RT_DEVICE_RTL8389M_A,
    RT_DEVICE_RTL8389_A,
    RT_DEVICE_RTL8329M_A,
    RT_DEVICE_RTL8329_A,
    RT_DEVICE_RTL8377M_A,
#endif
#if defined(CONFIG_SDK_RTL8328)
    RT_DEVICE_RTL8328M_A,
    RT_DEVICE_RTL8328S_A,
    RT_DEVICE_RTL8328L_A,
    RT_DEVICE_RTL8328S_C,
    RT_DEVICE_RTL8328L_C,
#endif
#if defined(CONFIG_SDK_RTL8390)
    RT_DEVICE_RTL8352M_A,
    RT_DEVICE_RTL8353M_A,
    RT_DEVICE_RTL8392M_A,
    RT_DEVICE_RTL8393M_A,
    RT_DEVICE_RTL8396M_A,
    RT_DEVICE_RTL8352MES_A,
    RT_DEVICE_RTL8353MES_A,
    RT_DEVICE_RTL8392MES_A,
    RT_DEVICE_RTL8393MES_A,
    RT_DEVICE_RTL8396MES_A,
#endif
#if defined(CONFIG_SDK_RTL8380)
    RT_DEVICE_RTL8382M_A,
    RT_DEVICE_RTL8380M_A,
    RT_DEVICE_RTL8332M_A,
    RT_DEVICE_RTL8330M_A,
    RT_DEVICE_RTL8382MES_A,
    RT_DEVICE_RTL8380MES_A,
    RT_DEVICE_RTL8332MES_A,
    RT_DEVICE_RTL8330MES_A,
#endif
    RT_DEVICE_END
}rt_supported_device_t;

/* Definition port type */
typedef enum rt_port_type_e
{
    RT_PORT_NONE = 0,
    RT_FE_PORT,
    RT_GE_PORT,
    RT_GE_COMBO_PORT,
    RT_GE_SERDES_PORT,
    RT_10GE_PORT,
    RT_10GE_COPPER_PORT,
    RT_10GE_FIBER_PORT,
    RT_10GE_SERDES_PORT,
    RT_CPU_PORT,
    RT_INT_FE_PORT,
    RT_PORT_TYPE_END
} rt_port_type_t;

/* Definition serdes type */
typedef enum rt_serdes_type_e
{
    RT_SERDES_NONE = 0,
    RT_SERDES_RS8MII,
    RT_SERDES_SGMII,
    RT_SERDES_5G,
    RT_SERDES_R_XAUI,
    RT_SERDES_10G_R,
    RT_SERDES_TYPE_END
} rt_serdes_type_t;

/* Definition port information for this chip */
typedef struct rt_portinfo_s
{
    rt_port_type_t  portType[RTK_MAX_NUM_OF_PORTS];
    uint32 port_number;
    int32  cpuPort; /* use (-1) for VALUE_NO_INIT */
    rt_portType_info_t fe;
    rt_portType_info_t fe_int;
    rt_portType_info_t ge;
    rt_portType_info_t ge_combo;
    rt_portType_info_t serdes;
    rt_portType_info_t ge_10ge;
    rt_portType_info_t serdes_10ge;
    rt_portType_info_t ether;
    rt_portType_info_t all;
} rt_portinfo_t;

/* Definition serdes information for this chip */
typedef struct rt_serdesInfo_s
{
    rt_serdes_type_t  serdesType[RTK_MAX_NUM_OF_SERDES];
    uint32 serdes_number;
    rt_serdesType_info_t all;
} rt_serdesInfo_t;

/* Define Capacity for each kind of register array */
typedef struct rt_register_capacity_s
{
    uint32  max_num_of_mirror;
    uint32  max_num_of_trunk;
    uint32  max_num_of_trunkMember;
    uint32  max_num_of_trunk_algo;
    uint32  trunk_algo_shift_max;
    uint32  max_num_of_dumb_trunkMember;
    uint32  max_num_of_trunkHashVal;
    uint32  max_num_of_msti;
    uint32  max_num_of_metering;
    uint32  max_num_of_meter_block;
    uint32  max_num_of_pie_block;
    uint32  max_num_of_acl_block_templateSelector;
    uint32  max_num_of_pie_logical_block;
    uint32  max_num_of_pie_blockSize;
    uint32  max_num_of_pie_counter;
    uint32  max_num_of_pie_action;
    uint32  max_num_of_pie_template;
    uint32  pie_user_template_id_min;
    uint32  pie_user_template_id_max;
    uint32  max_num_of_pie_payload;
    uint32  max_num_of_field_selector;
    uint32  max_num_of_range_check_srcPort;
    uint32  max_num_of_range_check_dstPort;
    uint32  max_num_of_range_check_ip;
    uint32  max_num_of_range_check_vid;
    uint32  max_num_of_range_check_l4Port;
    uint32  max_num_of_range_check_pktLen;
    uint32  max_num_of_pattern_match_data;
    uint32  pattern_match_port_max;
    uint32  pattern_match_port_min;
    uint32  max_num_of_l2_hashdepth;
    uint32  max_num_of_queue;
    uint32  min_num_of_queue;
    uint32  max_num_of_igrQueue;
    uint32  min_num_of_igrQueue;
    uint32  max_num_of_cvlan_tpid;
    uint32  max_num_of_svlan_tpid;
    uint32  max_num_of_evlan_tpid;
    uint32  max_num_of_route_host_addr;
    uint32  max_num_of_route_switch_addr;
    uint32  tpid_entry_idx_max;
    uint32  tpid_entry_mask_max;
    uint32  protocol_vlan_idx_max;
    uint32  vlan_fid_max;
    uint32  flowctrl_thresh_max;
    uint32  flowctrl_pauseOn_page_packet_len_max;
    uint32  flowctrl_pauseOn_page_packet_max;
    uint32  pri_of_selection_max;
    uint32  pri_of_selection_min;
    uint32  pri_sel_group_index_max;
    uint32  queue_weight_max;
    uint32  rate_of_bandwidth_max;
    uint32  rate_of_bandwidth_max_fe_port;
    uint32  rate_of_bandwidth_max_ge_port;
    uint32  rate_of_bandwidth_max_10ge_port;
    uint32  thresh_of_igr_port_pause_congest_group_idx_max;
    uint32  thresh_of_igr_bw_flowctrl_min;
    uint32  thresh_of_igr_bw_flowctrl_max;
    uint32  rate_of_storm_control_max;
    uint32  rate_of_storm_proto_control_max;
    uint32  burst_rate_of_storm_control_min;
    uint32  burst_rate_of_storm_control_max;
    uint32  burst_rate_of_10ge_storm_control_min;
    uint32  burst_rate_of_10ge_storm_control_max;
    uint32  burst_size_of_acl_meter_min;
    uint32  burst_size_of_acl_meter_max;
    uint32  max_num_of_fastPath_of_rate;
    uint32  internal_priority_max;
    uint32  drop_precedence_max;
    uint32  priority_remap_group_idx_max;
    uint32  priority_remark_group_idx_max;
    uint32  wred_weight_max;
    uint32  wred_mpd_max;
    uint32  wred_drop_probability_max;
    uint32  acl_rate_max;
    uint32  max_num_of_l2_hash_algo;
    uint32  l2_learn_limit_cnt_max;
    uint32  l2_learn_limit_cnt_wo_cam_max;
    uint32  l2_learn_limit_cnt_disable;
    uint32  l2_fid_learn_limit_entry_max;
    uint32  l2_notification_bp_thresh_max;
    uint32  eee_queue_thresh_max;
    uint32  sec_minIpv6FragLen_max;
    uint32  sec_maxPingLen_max;
    uint32  sec_smurfNetmaskLen_max;
    uint32  sflow_rate_max;
    uint32  max_num_of_mcast_fwd;
    uint32  miim_page_id_max;
    uint32  max_num_of_c2sc_entry;
    uint32  max_num_of_c2sc_blk_entry;
    uint32  max_num_of_c2sc_blk;
    uint32  max_num_of_sc2c_entry;
    uint32  max_num_of_vlan_prof;
    uint32  max_frame_len;
    uint32  max_num_of_mpls_entry;
    uint32  max_num_of_mcast_entry;
    uint32  max_num_of_vlan_port_iso_entry;
    uint32  max_num_of_mpls_lib;
    uint32  max_num_of_led_entity;
    uint32  max_num_of_dying_gasp_pkt_cnt;
    uint32  dying_gasp_sustain_time_max;
    uint32  max_num_of_rma_user_defined;
    uint32  time_nsec_max;
    uint32  max_num_of_ethdm_rx_timestamp;
} rt_register_capacity_t;

/* Define chip PER_PORT block information */
typedef struct rt_macPpInfo_s
{
    uint32 lowerbound_addr;
    uint32 upperbound_addr;
    uint32 interval;
} rt_macPpInfo_t;

/* Define chip structure */
typedef struct rt_device_s
{
    uint32 chip_id;
    uint32 chip_rev_id;
    uint32 driver_id;
    uint32 driver_rev_id;
    uint32 chip_aflags;
    uint32 chip_family_id;
    rt_portinfo_t *pPortinfo;
    rt_register_capacity_t  *pCapacityInfo;
    uint32 macPpInfo_blockNum;
    rt_macPpInfo_t *pMacPpInfo;
    rt_serdesInfo_t *pSerdesInfo;
} rt_device_t;


/*
 * Function Declaration
 */

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
 *      Pointer of mac chip structure that found
 * Note:
 *      The function have take care the forward compatible in revision.
 *      Return one recently revision if no extra match revision.
 */
extern rt_device_t *
hal_find_device(uint32 chip_id, uint32 chip_rev_id);

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
extern int32
hal_get_driver_id(
    uint32 chip_id,
    uint32 chip_rev_id,
    uint32 *pDriver_id,
    uint32 *pDriver_rev_id);

/* Function Name:
 *      hal_get_chip_id
 * Description:
 *      Get chip id and chip revision id.
 * Input:
 *      unit           - unit id
 * Output:
 *      pChip_id       - pointer buffer of chip id
 *      pChip_rev_id   - pointer buffer of chip revision id
 * Return:
 *      RT_ERR_OK      - OK
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
extern int32
hal_get_chip_id(uint32 unit, uint32 *pChip_id, uint32 *pChip_rev_id);

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
extern int32
hal_isPpBlock_check(uint32 unit, uint32 addr, uint32 *pIsPpBlock, uint32 *pPpBlockIdx);

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
 *      It's for RTL8390 only.
 */
extern int32
hal_mode_info_get(uint32 unit, uint32 *pChip_mode, uint32 *pApp_mode);

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
extern int32
hal_chip_mode_get(uint32 unit, rt_device_t *pDev);

#endif  /* __HAL_CHIPDEF_CHIP_H__ */
