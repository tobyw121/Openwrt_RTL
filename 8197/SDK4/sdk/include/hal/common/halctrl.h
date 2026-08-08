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
 * $Revision: 56168 $
 * $Date: 2015-02-13 16:48:38 +0800 (Fri, 13 Feb 2015) $
 *
 * Purpose : Hardware Abstraction Layer (HAL) control structure and definition in the SDK.
 *
 * Feature : HAL control structure and definition
 *
 */

#ifndef __HAL_COMMON_HALCTRL_H__
#define __HAL_COMMON_HALCTRL_H__

/*
 * Include Files
 */
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <osal/lib.h>
#include <osal/sem.h>
#include <hal/chipdef/driver.h>
#include <hal/chipdef/chip.h>
#include <hal/chipdef/allmem.h>


/*
 * Symbol Definition
 */
#define HAL_CHIP_INITED     (1 << 0)
#define HAL_CHIP_ATTACHED   (1 << 1)

/*
 * Data Type Definition
 */
typedef struct hal_control_s
{
    uint32          chip_flags;
    uint32          chip_id;
    uint32          chip_rev_id;
    uint32          semaphore;
    rt_driver_t     *pChip_driver;
    rt_device_t     *pDev_info;
    rt_phyctrl_t    *pPhy_ctrl[RTK_MAX_NUM_OF_PORTS];

    /* HAL layer semaphore protection mechanism */
    osal_mutex_t    reg_sem;    /* register semaphore */
    osal_mutex_t    tbl_sem[RTK_INDIRECT_CTRL_GROUP_END];    /* table semaphore */
    osal_mutex_t    phy_sem;    /* PHY semaphore */
} hal_control_t;

extern hal_control_t hal_ctrl[RTK_MAX_NUM_OF_UNIT];
/*
 * Macro Definition
 */
#define HAL_GET_CHIP_ID(unit)                   (hal_ctrl[unit].chip_id)
#define HAL_GET_CHIP_REV_ID(unit)               (hal_ctrl[unit].chip_rev_id)
#define HAL_GET_FAMILY_ID(unit)                 (hal_ctrl[unit].pDev_info->chip_family_id)
#define HAL_IS_RTL8389_FAMILY_ID(unit)          ((hal_ctrl[unit].pDev_info->chip_family_id == RTL8389_FAMILY_ID)?1:0)
#define HAL_IS_RTL8328_FAMILY_ID(unit)          ((hal_ctrl[unit].pDev_info->chip_family_id == RTL8328_FAMILY_ID)?1:0)
#define HAL_IS_RTL8390_FAMILY_ID(unit)          ((hal_ctrl[unit].pDev_info->chip_family_id == RTL8390_FAMILY_ID)?1:0)
#define HAL_IS_RTL8350_FAMILY_ID(unit)          ((hal_ctrl[unit].pDev_info->chip_family_id == RTL8350_FAMILY_ID)?1:0)
#define HAL_IS_RTL8380_FAMILY_ID(unit)          ((hal_ctrl[unit].pDev_info->chip_family_id == RTL8380_FAMILY_ID)?1:0)
#define HAL_IS_RTL8330_FAMILY_ID(unit)          ((hal_ctrl[unit].pDev_info->chip_family_id == RTL8330_FAMILY_ID)?1:0)
/* macro for driver information */
#define MACDRV(pHalCtrl)                        ((pHalCtrl)->pChip_driver->pMacdrv)
#define HAL_GET_MAX_REG_IDX(unit)               (hal_ctrl[unit].pChip_driver->reg_idx_max)
#define HAL_GET_MAX_REGFIELD_IDX(unit)          (hal_ctrl[unit].pChip_driver->regField_idx_max)
#define HAL_GET_MAX_TABLE_IDX(unit)             (hal_ctrl[unit].pChip_driver->table_idx_max)
#define HAL_GET_REG_WORD_NUM(unit, reg)         ((hal_ctrl[unit].pChip_driver->pReg_list[reg]->bit_offset>32)?((hal_ctrl[unit].pChip_driver->pReg_list[reg]->bit_offset+31)>>5):1)
#define HAL_IS_PORTMASK_INDEX(unit, reg)        ((hal_ctrl[unit].pChip_driver->pReg_list[reg]->portlist_index)?1:0)
#define HAL_GET_PORTMASK_INDEX(unit, reg)       (hal_ctrl[unit].pChip_driver->pReg_list[reg]->portlist_index)

/* Macro for port information */
#define HAL_GET_PORTNUM(unit)                   (hal_ctrl[unit].pDev_info->pPortinfo->port_number)
#if defined (CONFIG_SDK_MODEL_MODE) && !defined (CONFIG_SDK_FPGA_PLATFORM) /* [FIXME] */
#define HAL_IS_PORT_EXIST(unit, port)           (1)
#define HAL_IS_ETHER_PORT(unit, port)           (1)
#else
#define HAL_IS_PORT_EXIST(unit, port)           RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->all.portmask, port)
#define HAL_IS_ETHER_PORT(unit, port)           RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->ether.portmask, port)
#endif
#define HAL_IS_CPU_PORT(unit, port)             ((((int32)port == hal_ctrl[unit].pDev_info->pPortinfo->cpuPort)? 1:0))
#define HAL_IS_FE_PORT(unit, port)              RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->fe.portmask, port)
#define HAL_IS_INT_FE_PORT(unit, port)          RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->fe_int.portmask, port)
#define HAL_IS_10GE_PORT(unit, port)            RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->ge_10ge.portmask, port)
#define HAL_IS_GE_PORT(unit, port)              RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->ge.portmask, port)
#define HAL_IS_GE_COMBO_PORT(unit, port)        RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->ge_combo.portmask, port)
#define HAL_IS_SERDES_PORT(unit, port)          RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->serdes.portmask, port)
#define HAL_IS_SERDES_10GE_PORT(unit, port)     RTK_PORTMASK_IS_PORT_SET(hal_ctrl[unit].pDev_info->pPortinfo->serdes_10ge.portmask, port)
#define HAL_GET_MACPP_MIN_ADDR(unit)            (hal_ctrl[unit].pDev_info->pMacPpInfo->lowerbound_addr)
#define HAL_GET_MACPP_MAX_ADDR(unit)            (hal_ctrl[unit].pDev_info->pMacPpInfo->upperbound_addr)
#define HAL_GET_MACPP_INTERVAL(unit)            (hal_ctrl[unit].pDev_info->pMacPpInfo->interval)
#define HAL_GET_MACPP_BLK_NUM(unit)             (hal_ctrl[unit].pDev_info->macPpInfo_blockNum)
#define HAL_GET_MACPP_BLK_MIN_ADDR(unit, blkIdx)    (hal_ctrl[unit].pDev_info->pMacPpInfo[blkIdx].lowerbound_addr)
#define HAL_GET_MACPP_BLK_MAX_ADDR(unit, blkIdx)    (hal_ctrl[unit].pDev_info->pMacPpInfo[blkIdx].upperbound_addr)
#define HAL_GET_MACPP_BLK_INTERVAL(unit, blkIdx)    (hal_ctrl[unit].pDev_info->pMacPpInfo[blkIdx].interval)
#define HAL_GET_PORT_TYPE(unit, port)           ((hal_ctrl[unit].pDev_info->pPortinfo->portType[port]))
#define HAL_GET_CPU_PORT(unit)                  ((hal_ctrl[unit].pDev_info->pPortinfo->cpuPort))
#define HAL_GET_MAX_PORT(unit)                  ((hal_ctrl[unit].pDev_info->pPortinfo->all.max))
#define HAL_GET_MIN_PORT(unit)                  ((hal_ctrl[unit].pDev_info->pPortinfo->all.min))
#define HAL_GET_MAX_ETHER_PORT(unit)            ((hal_ctrl[unit].pDev_info->pPortinfo->ether.max))
#define HAL_GET_MIN_ETHER_PORT(unit)            ((hal_ctrl[unit].pDev_info->pPortinfo->ether.min))
#define HAL_GET_MAX_GE_PORT(unit)               ((hal_ctrl[unit].pDev_info->pPortinfo->ge.max))
#define HAL_GET_MIN_GE_PORT(unit)               ((hal_ctrl[unit].pDev_info->pPortinfo->ge.min))
#define HAL_GET_MAX_10GE_PORT(unit)             ((hal_ctrl[unit].pDev_info->pPortinfo->ge_10ge.max))
#define HAL_GET_MIN_10GE_PORT(unit)             ((hal_ctrl[unit].pDev_info->pPortinfo->ge_10ge.min))
#define HAL_GET_MAX_FE_PORT(unit)               ((hal_ctrl[unit].pDev_info->pPortinfo->fe.max))
#define HAL_GET_MIN_FE_PORT(unit)               ((hal_ctrl[unit].pDev_info->pPortinfo->fe.min))
#define HAL_GET_MAX_INT_FE_PORT(unit)           ((hal_ctrl[unit].pDev_info->pPortinfo->fe_int.max))
#define HAL_GET_MIN_INT_FE_PORT(unit)           ((hal_ctrl[unit].pDev_info->pPortinfo->fe_int.min))
#define HAL_GET_MAX_SERDES_PORT(unit)           ((hal_ctrl[unit].pDev_info->pPortinfo->serdes.max))
#define HAL_GET_MIN_SERDES_PORT(unit)           ((hal_ctrl[unit].pDev_info->pPortinfo->serdes.min))
#define HAL_GET_SERDES_PORT_NUM(unit)           ((hal_ctrl[unit].pDev_info->pPortinfo->serdes.portNum))
#define HAL_GET_ALL_PORTMASK(unit, dstPortmask)    RTK_PORTMASK_ASSIGN((dstPortmask), (hal_ctrl[unit].pDev_info->pPortinfo->all.portmask))
#define HAL_GET_ETHER_PORTMASK(unit, dstPortmask)  RTK_PORTMASK_ASSIGN((dstPortmask), (hal_ctrl[unit].pDev_info->pPortinfo->ether.portmask))
#define HAL_GET_GE_PORTMASK(unit, dstPortmask)     RTK_PORTMASK_ASSIGN((dstPortmask), (hal_ctrl[unit].pDev_info->pPortinfo->ge.portmask))
#define HAL_GET_FE_PORTMASK(unit, dstPortmask)     RTK_PORTMASK_ASSIGN((dstPortmask), (hal_ctrl[unit].pDev_info->pPortinfo->fe.portmask))
#define HAL_GET_MAX_BANDWIDTH_OF_PORT(unit, port) (hal_portMaxBandwidth_ret(unit, port))
#define HAL_IS_PHY_EXIST(unit, port)            ((hal_ctrl[unit].pPhy_ctrl[port] != NULL)? 1:0)
#define HAL_IS_PHY_8218(unit, port)             (((hal_ctrl[unit].pPhy_ctrl[port]->phy_model_id == PHY_MODEL_ID_RTL8218) || (hal_ctrl[unit].pPhy_ctrl[port]->phy_model_id == PHY_MODEL_ID_RTL8218_TC))? 1:0)
#define HAL_IS_PHY_8214_8214F(unit, port)       (((hal_ctrl[unit].pPhy_ctrl[port]->phy_model_id == PHY_MODEL_ID_RTL8214) || (hal_ctrl[unit].pPhy_ctrl[port]->phy_model_id == PHY_MODEL_ID_RTL8214F))? 1:0)
#define HAL_IS_PHY_INFO_EXIST(unit, port)       ((hal_ctrl[unit].pPhy_ctrl[port]->pPhyInfo != NULL)? 1:0)
#define HAL_GET_PHY_NUM(unit, port)             (hal_ctrl[unit].pPhy_ctrl[port]->pPhyInfo->phy_num)

#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
#define HAL_IS_ESCHIP(unit)     (((HAL_GET_CHIP_ID((unit)) & 0xffff) == (REALTEK_MES_CHIP_ID & 0xffff)) ? 1 : 0)
#else
#define HAL_IS_ESCHIP(unit)     0
#endif

/* Compiler option for bypass 8328S checking or not? */
#ifdef HAL_BYPASS_8328S_CHECKED
#define HAL_IS_MAC_8328S(unit)                  (0)
#else
#define HAL_IS_MAC_8328S(unit)                  (hal_ctrl[unit].chip_id == RTL8328S_CHIP_ID)
#endif
/* Compiler option for bypass 8328L checking or not? */
#ifdef HAL_BYPASS_8328L_CHECKED
#define HAL_IS_MAC_8328L(unit)                  (0)
#else
#define HAL_IS_MAC_8328L(unit)                  (hal_ctrl[unit].chip_id == RTL8328L_CHIP_ID)
#endif

#define HAL_MAX_NUM_OF_MIRROR(unit)             (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_mirror)
#define HAL_MIRROR_ID_MAX(unit)                 (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_mirror - 1)
#define HAL_MAX_NUM_OF_TRUNK(unit)              (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_trunk)
#define HAL_MAX_NUM_OF_TRUNKMEMBER(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_trunkMember)
#define HAL_MAX_NUM_OF_TRUNK_ALGO(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_trunk_algo)
#define HAL_TRUNK_ALGO_SHIFT_MAX(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->trunk_algo_shift_max)
#define HAL_MAX_NUM_OF_DUMB_TRUNKMEMBER(unit)   (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_dumb_trunkMember)
#define HAL_MAX_NUM_OF_TRUNKHASHVAL(unit)       (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_trunkHashVal)
#define HAL_MAX_NUM_OF_MSTI(unit)               (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_msti)
#define HAL_MAX_NUM_OF_METERING(unit)           (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_metering)
#define HAL_MAX_NUM_OF_METER_BLOCK(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_meter_block)
#define HAL_MAX_NUM_OF_PIE_BLOCK(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pie_block)
#define HAL_MAX_NUM_OF_ACL_BLOCK_TEMPLATESELECTOR(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_acl_block_templateSelector)
#define HAL_MAX_NUM_OF_PIE_LOGICAL_BLOCK(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pie_logical_block)
#define HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)      (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pie_blockSize)
#define HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)      (HAL_MAX_NUM_OF_PIE_BLOCK(unit) * HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit))
#define HAL_PIE_FILTER_ID_MAX(unit)             (HAL_MAX_NUM_OF_PIE_FILTER_ID(unit) - 1)
#define HAL_PIE_FILTER_ID_MIN(unit)             (0)
#define HAL_MAX_NUM_OF_PIE_COUNTER(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pie_counter)
#define HAL_MAX_NUM_OF_PIE_ACTION(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pie_action)
#define HAL_MAX_NUM_OF_PIE_TEMPLATE(unit)       (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pie_template)
#define HAL_PIE_USER_TEMPLATE_ID_MIN(unit)      (hal_ctrl[unit].pDev_info->pCapacityInfo->pie_user_template_id_min)
#define HAL_PIE_USER_TEMPLATE_ID_MAX(unit)      (hal_ctrl[unit].pDev_info->pCapacityInfo->pie_user_template_id_max)
#define HAL_MAX_NUM_OF_PIE_PAYLOAD(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pie_payload)
#define HAL_MAX_NUM_OF_FIELD_SELECTOR(unit)     (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_field_selector)
#define HAL_MAX_NUM_OF_RANGE_CHECK_SRCPORT(unit)    (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_range_check_srcPort)
#define HAL_MAX_NUM_OF_RANGE_CHECK_DSTPORT(unit)(hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_range_check_dstPort)
#define HAL_MAX_NUM_OF_RANGE_CHECK_IP(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_range_check_ip)
#define HAL_MAX_NUM_OF_RANGE_CHECK_VID(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_range_check_vid)
#define HAL_MAX_NUM_OF_RANGE_CHECK_L4PORT(unit)     (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_range_check_l4Port)
#define HAL_MAX_NUM_OF_RANGE_CHECK_PKTLEN(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_range_check_pktLen)
#define HAL_MAX_NUM_OF_PATTERN_MATCH_DATA(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_pattern_match_data)
#define HAL_PATTERN_MATCH_PORT_MAX(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->pattern_match_port_max)
#define HAL_PATTERN_MATCH_PORT_MIN(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->pattern_match_port_min)
#define HAL_L2_HASHDEPTH(unit)                  (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_l2_hashdepth)
#define HAL_MAX_NUM_OF_QUEUE(unit)              (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_queue)
#define HAL_MIN_NUM_OF_QUEUE(unit)              (hal_ctrl[unit].pDev_info->pCapacityInfo->min_num_of_queue)
#define HAL_MAX_NUM_OF_IGR_QUEUE(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_igrQueue)
#define HAL_MIN_NUM_OF_IGR_QUEUE(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->min_num_of_igrQueue)
#define HAL_MAX_NUM_OF_CVLAN_TPID(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_cvlan_tpid)
#define HAL_MAX_NUM_OF_SVLAN_TPID(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_svlan_tpid)
#define HAL_MAX_NUM_OF_EVLAN_TPID(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_evlan_tpid)
#define HAL_TPID_ENTRY_IDX_MAX(unit)            (hal_ctrl[unit].pDev_info->pCapacityInfo->tpid_entry_idx_max)
#define HAL_TPID_ENTRY_MASK_MAX(unit)           (hal_ctrl[unit].pDev_info->pCapacityInfo->tpid_entry_mask_max)
#define HAL_PROTOCOL_VLAN_IDX_MAX(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->protocol_vlan_idx_max)
#define HAL_VLAN_FID_MAX(unit)                  (hal_ctrl[unit].pDev_info->pCapacityInfo->vlan_fid_max)
#define HAL_FLOWCTRL_THRESH_MAX(unit)           (hal_ctrl[unit].pDev_info->pCapacityInfo->flowctrl_thresh_max)
#define HAL_FLOWCTRL_PAUSEON_PAGE_PACKET_LEN_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->flowctrl_pauseOn_page_packet_len_max)
#define HAL_FLOWCTRL_PAUSEON_PAGE_PACKET_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->flowctrl_pauseOn_page_packet_max)
#define HAL_PRI_OF_SELECTION_MAX(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->pri_of_selection_max)
#define HAL_PRI_OF_SELECTION_MIN(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->pri_of_selection_min)
#define HAL_PRI_SEL_GROUP_INDEX_MAX(unit)       (hal_ctrl[unit].pDev_info->pCapacityInfo->pri_sel_group_index_max)
#define HAL_QUEUE_WEIGHT_MAX(unit)              (hal_ctrl[unit].pDev_info->pCapacityInfo->queue_weight_max)
#define HAL_RATE_OF_BANDWIDTH_MAX(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_bandwidth_max)
#define HAL_RATE_OF_10G_BANDWIDTH_MAX(unit)     (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_bandwidth_max_10ge_port)

#define HAL_THRESH_OF_IGR_PORT_PAUSE_CONGEST_GROUP_IDX_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->thresh_of_igr_port_pause_congest_group_idx_max)
#define HAL_THRESH_OF_IGR_BW_FLOWCTRL_MIN(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->thresh_of_igr_bw_flowctrl_min)
#define HAL_THRESH_OF_IGR_BW_FLOWCTRL_MAX(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->thresh_of_igr_bw_flowctrl_max)
#define HAL_MAX_NUM_OF_FASTPATH_OF_RATE(unit)   (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_fastPath_of_rate)
#define HAL_RATE_OF_STORM_CONTROL_MAX(unit)     (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_storm_control_max)
#define HAL_RATE_OF_STORM_PROTO_CONTROL_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->rate_of_storm_proto_control_max)
#define HAL_BURST_RATE_OF_STORM_CONTROL_MIN(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->burst_rate_of_storm_control_min)
#define HAL_BURST_RATE_OF_STORM_CONTROL_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->burst_rate_of_storm_control_max)
#define HAL_BURST_RATE_OF_10G_STORM_CONTROL_MIN(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->burst_rate_of_10ge_storm_control_min)
#define HAL_BURST_RATE_OF_10G_STORM_CONTROL_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->burst_rate_of_10ge_storm_control_max)
#define HAL_BURST_SIZE_OF_ACL_METER_MIN(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->burst_size_of_acl_meter_min)
#define HAL_BURST_SIZE_OF_ACL_METER_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->burst_size_of_acl_meter_max)
#define HAL_INTERNAL_PRIORITY_MAX(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->internal_priority_max)
#define HAL_DROP_PRECEDENCE_MAX(unit)           (hal_ctrl[unit].pDev_info->pCapacityInfo->drop_precedence_max)
#define HAL_PRIORITY_REMAP_GROUP_IDX_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->priority_remap_group_idx_max)
#define HAL_PRIORITY_REMARK_GROUP_IDX_MAX(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->priority_remark_group_idx_max)
#define HAL_WRED_WEIGHT_MAX(unit)               (hal_ctrl[unit].pDev_info->pCapacityInfo->wred_weight_max)
#define HAL_WRED_MPD_MAX(unit)                  (hal_ctrl[unit].pDev_info->pCapacityInfo->wred_mpd_max)
#define HAL_WRED_DROP_PROBABILITY_MAX(unit)     (hal_ctrl[unit].pDev_info->pCapacityInfo->wred_drop_probability_max)
#define HAL_MAX_NUM_OF_QUEUE(unit)              (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_queue)
#define HAL_QUEUE_ID_MAX(unit)                  (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_queue - 1)
#define HAL_ACL_RATE_MAX(unit)                  (hal_ctrl[unit].pDev_info->pCapacityInfo->acl_rate_max)
#define HAL_MAX_NUM_OF_L2_HASH_ALGO(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_l2_hash_algo)
#define HAL_L2_LEARN_LIMIT_CNT_MAX(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->l2_learn_limit_cnt_max)
#define HAL_L2_LEARN_LIMIT_CNT_WO_CAM_MAX(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->l2_learn_limit_cnt_wo_cam_max)
#define HAL_L2_LEARN_LIMIT_CNT_DISABLE(unit)    (hal_ctrl[unit].pDev_info->pCapacityInfo->l2_learn_limit_cnt_disable)
#define HAL_L2_FID_LEARN_LIMIT_ENTRY_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->l2_fid_learn_limit_entry_max)
#define HAL_L2_NOTIFICATION_BP_THRESH_MAX(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->l2_notification_bp_thresh_max)
#define HAL_EEE_QUEUE_THRESH_MAX(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->eee_queue_thresh_max)
#define HAL_SEC_MINIPV6FRAGLEN_MAX(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->sec_minIpv6FragLen_max)
#define HAL_SEC_MAXPINGLEN_MAX(unit)            (hal_ctrl[unit].pDev_info->pCapacityInfo->sec_maxPingLen_max)
#define HAL_SEC_SMURFNETMASKLEN_MAX(unit)       (hal_ctrl[unit].pDev_info->pCapacityInfo->sec_smurfNetmaskLen_max)
#define HAL_SFLOW_RATE_MAX(unit)                (hal_ctrl[unit].pDev_info->pCapacityInfo->sflow_rate_max)
#define HAL_MAX_NUM_OF_MCAST_FWD(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_mcast_fwd)
#define HAL_MIIM_PAGE_ID_MAX(unit)              (hal_ctrl[unit].pDev_info->pCapacityInfo->miim_page_id_max)
#define HAL_MAX_NUM_OF_C2SC_ENTRY(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_c2sc_entry)
#define HAL_MAX_NUM_OF_C2SC_BLK_ENTRY(unit)     (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_c2sc_blk_entry)
#define HAL_MAX_NUM_OF_C2SC_BLK(unit)           (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_c2sc_blk)
#define HAL_MAX_NUM_OF_SC2C_ENTRY(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_sc2c_entry)
#define HAL_MAX_NUM_OF_VLAN_PROFILE(unit)       (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_vlan_prof)
#define HAL_MAX_ACCEPT_FRAME_LEN(unit)          (hal_ctrl[unit].pDev_info->pCapacityInfo->max_frame_len)
#define HAL_MAX_NUM_OF_MCAST_ENTRY(unit)        (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_mcast_entry)
#define HAL_MAX_NUM_OF_VLAN_PORT_ISO_ENTRY(unit)(hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_vlan_port_iso_entry)
#define HAL_MAX_NUM_OF_MPLS_LIB(unit)           (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_mpls_lib)
#define HAL_MAX_NUM_OF_ROUTE_HOST_ADDR(unit)    (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_route_host_addr)
#define HAL_MAX_NUM_OF_ROUTE_SWITCH_ADDR(unit)  (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_route_switch_addr)
#define HAL_MAX_NUM_OF_LED_ENTITY(unit)         (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_led_entity)
#define HAL_MAX_NUM_OF_DYING_GASP_PKT_CNT(unit) (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_dying_gasp_pkt_cnt)
#define HAL_DYING_GASP_SUSTAIN_TIME_MAX(unit)   (hal_ctrl[unit].pDev_info->pCapacityInfo->dying_gasp_sustain_time_max)
#define HAL_MAX_NUM_OF_RMA_USER_DEFINED(unit)   (hal_ctrl[unit].pDev_info->pCapacityInfo->max_num_of_rma_user_defined)
#define HAL_TIME_NSEC_MAX(unit)                 (hal_ctrl[unit].pDev_info->pCapacityInfo->time_nsec_max)

/*
 * Function Declaration
 */

/* Function Name:
 *      hal_init
 * Description:
 *      Initialize the hal layer API.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - General Error
 *      RT_ERR_CHIP_NOT_FOUND   - The chip can not found
 *      RT_ERR_DRIVER_NOT_FOUND - The driver can not found
 * Note:
 *      Initialize the hal layer API, include get the chip id and chip revision
 *      id, get its driver id and driver revision id, then bind to its major
 *      driver. Also initialize its hal_ctrl structure for this specified unit.
 *      Before calling the function, bsps should already scan HW interface, like
 *      PCI device in all buses, or physical Lextra, and the total chip numbers,
 *      its chip id and chip revision id is known and store in database in lower
 *      layer.
 */
extern int32
hal_init(uint32 unit);

/* Function Name:
 *      hal_ctrlInfo_get
 * Description:
 *      Find the hal control information structure for this specified unit.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      NULL      - Not found
 *      Otherwise - Pointer of hal control information structure that found
 * Note:
 *      The function have found the exactly hal control information structure.
 */
extern hal_control_t *
hal_ctrlInfo_get(uint32 unit);


/* Function Name:
 *      hal_portMaxBandwidth_ret
 * Description:
 *      Get the max bandwith of port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      max bandwidth value
 * Note:
 *      The return value is different in FE/GE/10GE port.
 */
extern uint32
hal_portMaxBandwidth_ret(uint32 unit, rtk_port_t port);


#endif  /* __HAL_COMMON_MALCTRL_H__ */
