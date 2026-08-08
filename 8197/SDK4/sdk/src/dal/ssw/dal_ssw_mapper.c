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
 * $Revision: 46680 $
 * $Date: 2014-02-28 13:31:29 +0800 (Fri, 28 Feb 2014) $
 *
 * Purpose : Mapper Layer is used to seperate different kind of software or hardware platform
 * 
 * Feature : Just dispatch information to Multiplex layer
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/debug/rt_log.h>
#include <osal/lib.h>
#include <dal/dal_mapper.h>     
#include <dal/dal_common.h>
#include <dal/ssw/dal_ssw_mapper.h>
#include <dal/ssw/dal_ssw_vlan.h>
#include <dal/ssw/dal_ssw_l2.h>
#include <dal/ssw/dal_ssw_port.h>
#include <dal/ssw/dal_ssw_trunk.h>
#include <dal/ssw/dal_ssw_stp.h>
#include <dal/ssw/dal_ssw_rate.h>
#include <dal/ssw/dal_ssw_qos.h>
#include <dal/ssw/dal_ssw_trap.h>
#include <dal/ssw/dal_ssw_stat.h>
#include <dal/ssw/dal_ssw_switch.h>
#include <dal/ssw/dal_ssw_dot1x.h>
#include <dal/ssw/dal_ssw_mirror.h>
#include <dal/ssw/dal_ssw_filter.h>
#include <dal/ssw/dal_ssw_flowctrl.h>
#include <dal/ssw/dal_ssw_svlan.h>
#include <dal/ssw/dal_ssw_eee.h>
#include <dal/ssw/dal_ssw_led.h>
#include <dal/ssw/dal_ssw_time.h>
#include <dal/ssw/dal_ssw_diag.h>
#include <rtk/default.h>

/* 
 * Symbol Definition
 */

/*
 * Data Declaration
 */
dal_mapper_t dal_ssw_mapper =
{
    0x8389,
    ._init = dal_ssw_init,
    
    /* VLAN */
    .vlan_init = dal_ssw_vlan_init,
    .vlan_create = dal_ssw_vlan_create,
    .vlan_destroy = dal_ssw_vlan_destroy,
    .vlan_destroyAll = dal_ssw_vlan_destroyAll,
    .vlan_fid_get = dal_ssw_vlan_fid_get,
    .vlan_fid_set = dal_ssw_vlan_fid_set,
    .vlan_port_add = dal_ssw_vlan_port_add,
    .vlan_port_del = dal_ssw_vlan_port_del,
    .vlan_port_get = dal_ssw_vlan_port_get,
    .vlan_port_set = dal_ssw_vlan_port_set,
    .vlan_stg_get = dal_ssw_vlan_stg_get,
    .vlan_stg_set = dal_ssw_vlan_stg_set,
    .vlan_portAcceptFrameType_get = dal_ssw_vlan_portAcceptFrameType_get,
    .vlan_portAcceptFrameType_set = dal_ssw_vlan_portAcceptFrameType_set,
    .vlan_portOuterAcceptFrameType_get = (int32 (*)(uint32, rtk_port_t, rtk_vlan_acceptFrameType_t *))dal_common_unavail,
    .vlan_portOuterAcceptFrameType_set = (int32 (*)(uint32, rtk_port_t, rtk_vlan_acceptFrameType_t ))dal_common_unavail,
    .vlan_vlanFunctionEnable_get = dal_ssw_vlan_vlanFunctionEnable_get,
    .vlan_vlanFunctionEnable_set = dal_ssw_vlan_vlanFunctionEnable_set,
    .vlan_portIgrFilterEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portIgrFilterEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_igrFilterEnable_get = dal_ssw_vlan_igrFilterEnable_get,
    .vlan_igrFilterEnable_set = dal_ssw_vlan_igrFilterEnable_set,
    .vlan_portEgrFilterEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portEgrFilterEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_mcastLeakyEnable_get = dal_ssw_vlan_mcastLeakyEnable_get,
    .vlan_mcastLeakyEnable_set = dal_ssw_vlan_mcastLeakyEnable_set,
    .vlan_mcastLeakyPortEnable_get = (int32 (*)(uint32 , rtk_port_t, rtk_enable_t *))dal_common_unavail,
    .vlan_mcastLeakyPortEnable_set = (int32 (*)(uint32 , rtk_port_t, rtk_enable_t ))dal_common_unavail,
    .vlan_portPvid_get = dal_ssw_vlan_portPvid_get,
    .vlan_portPvid_set = dal_ssw_vlan_portPvid_set,
    .vlan_portOuterPvid_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .vlan_portOuterPvid_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .vlan_protoGroup_get = (int32 (*)(uint32 , uint32 , rtk_vlan_protoGroup_t *))dal_common_unavail,
    .vlan_protoGroup_set = (int32 (*)(uint32 , uint32 , rtk_vlan_protoGroup_t *))dal_common_unavail,
    .vlan_portProtoVlan_get = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_vlan_protoVlanCfg_t *))dal_common_unavail,
    .vlan_portProtoVlan_set = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_vlan_protoVlanCfg_t *))dal_common_unavail,
    .vlan_portOuterProtoVlan_get = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_vlan_protoVlanCfg_t *))dal_common_unavail,
    .vlan_portOuterProtoVlan_set = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_vlan_protoVlanCfg_t *))dal_common_unavail,
    .vlan_portTpidEntry_get = (int32 (*)(uint32 , rtk_port_t , uint32 , uint32 *))dal_common_unavail,
    .vlan_portTpidEntry_set = (int32 (*)(uint32 , rtk_port_t , uint32 , uint32 ))dal_common_unavail,
    .vlan_portEgrInnerTpidMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_egrTpidMode_t *))dal_common_unavail,
    .vlan_portEgrInnerTpidMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_egrTpidMode_t ))dal_common_unavail,
    .vlan_portIgrInnerTpid_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .vlan_portIgrInnerTpid_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .vlan_portEgrInnerTpid_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .vlan_portEgrInnerTpid_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .vlan_portEgrOuterTpidMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_egrTpidMode_t *))dal_common_unavail,
    .vlan_portEgrOuterTpidMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_egrTpidMode_t ))dal_common_unavail,
    .vlan_portIgrOuterTpid_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .vlan_portIgrOuterTpid_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .vlan_portEgrOuterTpid_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .vlan_portEgrOuterTpid_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .vlan_portIgrExtraTpid_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .vlan_portIgrExtraTpid_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .vlan_portIgrIgnoreInnerTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portIgrIgnoreInnerTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_portIgrIgnoreOuterTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portIgrIgnoreOuterTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_portEgrInnerTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portEgrInnerTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_portEgrOuterTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portEgrOuterTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_portIgrExtraTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portIgrExtraTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_portEgrExtraTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .vlan_portEgrExtraTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .vlan_portEgrInnerVidSource_get = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t *))dal_common_unavail,
    .vlan_portEgrInnerVidSource_set = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t ))dal_common_unavail,
    .vlan_portEgrInnerPriSource_get = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t *))dal_common_unavail,
    .vlan_portEgrInnerPriSource_set = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t ))dal_common_unavail,
    .vlan_portEgrOuterVidSource_get = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t *))dal_common_unavail,
    .vlan_portEgrOuterVidSource_set = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t ))dal_common_unavail,
    .vlan_portEgrOuterPriSource_get = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t *))dal_common_unavail,
    .vlan_portEgrOuterPriSource_set = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_tagSource_t ))dal_common_unavail,
    .vlan_tagMode_get = dal_ssw_vlan_tagMode_get,
    .vlan_tagMode_set = dal_ssw_vlan_tagMode_set,
    .vlan_portIgrTagKeepEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *, rtk_enable_t *))dal_common_unavail,
    .vlan_portIgrTagKeepEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t , rtk_enable_t ))dal_common_unavail,
    .vlan_portEgrTagKeepEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *, rtk_enable_t *))dal_common_unavail,
    .vlan_portEgrTagKeepEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t , rtk_enable_t ))dal_common_unavail,
    .vlan_fwdMode_get = (int32 (*)(uint32 , rtk_vlan_t , rtk_vlan_fwdMode_t *))dal_common_unavail,
    .vlan_fwdMode_set = (int32 (*)(uint32 , rtk_vlan_t , rtk_vlan_fwdMode_t ))dal_common_unavail,
    
    /* L2 */
    .l2_init = dal_ssw_l2_init,
    .l2_flushLinkDownPortAddrEnable_get = dal_ssw_l2_flushLinkDownPortAddrEnable_get,
    .l2_flushLinkDownPortAddrEnable_set = dal_ssw_l2_flushLinkDownPortAddrEnable_set,
    .l2_ucastAddr_flush = dal_ssw_l2_ucastAddr_flush,
    .l2_learningCnt_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .l2_limitLearningCnt_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .l2_limitLearningCnt_set = (int32 (*)(uint32 , uint32))dal_common_unavail,
    .l2_limitLearningCntAction_get = (int32 (*)(uint32 , rtk_l2_limitLearnCntAction_t *))dal_common_unavail,
    .l2_limitLearningCntAction_set = (int32 (*)(uint32 , rtk_l2_limitLearnCntAction_t))dal_common_unavail,
    .l2_portLearningCnt_get = dal_ssw_l2_portLearningCnt_get,
    .l2_portLimitLearningCnt_get = dal_ssw_l2_portLimitLearningCnt_get,
    .l2_portLimitLearningCnt_set = dal_ssw_l2_portLimitLearningCnt_set,
    .l2_portLimitLearningCntAction_get = dal_ssw_l2_portLimitLearningCntAction_get,
    .l2_portLimitLearningCntAction_set = dal_ssw_l2_portLimitLearningCntAction_set,    
    .l2_portLastLearnedMac_get = (int32 (*)(uint32 , rtk_port_t , rtk_fid_t *, rtk_mac_t *))dal_common_unavail,
    .l2_fidLimitLearningEntry_get = (int32 (*)(uint32 , uint32 , rtk_l2_fidMacLimitEntry_t *))dal_common_unavail,
    .l2_fidLimitLearningEntry_set = (int32 (*)(uint32 , uint32 , rtk_l2_fidMacLimitEntry_t *))dal_common_unavail,
    .l2_fidLearningCnt_get = (int32 (*)(uint32 , uint32 , uint32 *))dal_common_unavail,
    .l2_fidLearningCnt_reset = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .l2_fidLastLearnedMac_get = (int32 (*)(uint32 , uint32 , rtk_fid_t *, rtk_mac_t *))dal_common_unavail,
    .l2_limitLearningTrapPri_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .l2_limitLearningTrapPri_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .l2_limitLearningTrapPriEnable_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .l2_limitLearningTrapPriEnable_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .l2_limitLearningTrapDP_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .l2_limitLearningTrapDP_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .l2_limitLearningTrapDPEnable_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .l2_limitLearningTrapDPEnable_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .l2_limitLearningTrapAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .l2_limitLearningTrapAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .l2_aging_get = dal_ssw_l2_aging_get,
    .l2_aging_set = dal_ssw_l2_aging_set,
    .l2_camEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .l2_camEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .l2_hashAlgo_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .l2_hashAlgo_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .l2_vlanMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_l2_vlanMode_t *))dal_common_unavail,
    .l2_vlanMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_l2_vlanMode_t ))dal_common_unavail,
    .l2_learningEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .l2_learningEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .l2_newMacOp_get = (int32 (*)(uint32 , rtk_port_t , rtk_l2_newMacLrnMode_t *, rtk_action_t *))dal_common_unavail,
    .l2_newMacOp_set = (int32 (*)(uint32 , rtk_port_t , rtk_l2_newMacLrnMode_t , rtk_action_t ))dal_common_unavail,
    .l2_LRUEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .l2_LRUEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .l2_ucastLookupMode_get = (int32 (*)(uint32 , rtk_l2_ucastLookupMode_t *))dal_common_unavail,
    .l2_ucastLookupMode_set = (int32 (*)(uint32 , rtk_l2_ucastLookupMode_t ))dal_common_unavail,
    .l2_addr_init = (int32 (*)(uint32 , rtk_vlan_t , rtk_mac_t *, rtk_l2_ucastAddr_t *))dal_common_unavail,
    .l2_addr_add = dal_ssw_l2_addr_add,
    .l2_addr_del = dal_ssw_l2_addr_del,
    .l2_addr_get = dal_ssw_l2_addr_get,
    .l2_addr_set = dal_ssw_l2_addr_set,
    .l2_addr_delAll = dal_ssw_l2_addr_delAll,
    .l2_nextValidAddr_get = dal_ssw_l2_nextValidAddr_get,
    .l2_nextValidMcastAddr_get = dal_ssw_l2_nextValidMcastAddr_get,
    .l2_nextValidIpMcastAddr_get = dal_ssw_l2_nextValidIpMcastAddr_get,
    .l2_mcastLookupMode_get = (int32 (*)(uint32 , rtk_l2_mcastLookupMode_t *, rtk_fid_t *))dal_common_unavail,
    .l2_mcastLookupMode_set = (int32 (*)(uint32 , rtk_l2_mcastLookupMode_t , rtk_fid_t ))dal_common_unavail,
    .l2_mcastBlockPortmask_get = (int32 (*)(uint32 , rtk_portmask_t *))dal_common_unavail,
    .l2_mcastBlockPortmask_set = (int32 (*)(uint32 , rtk_portmask_t *))dal_common_unavail,
    .l2_mcastAddr_init = (int32 (*)(uint32 , rtk_vlan_t , rtk_mac_t *, rtk_l2_mcastAddr_t *))dal_common_unavail,
    .l2_mcastAddr_add = dal_ssw_l2_mcastAddr_add,
    .l2_mcastAddr_del = dal_ssw_l2_mcastAddr_del,
    .l2_mcastAddr_get = dal_ssw_l2_mcastAddr_get,
    .l2_mcastAddr_set = dal_ssw_l2_mcastAddr_set,
    .l2_mcastAddr_add_with_index = dal_ssw_l2_mcastAddr_add_with_index,
    .l2_mcastAddr_get_with_index = dal_ssw_l2_mcastAddr_get_with_index,
    .l2_ipmcEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .l2_ipmcEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .l2_ipmcMode_get = (int32 (*)(uint32 , rtk_l2_ipmcMode_t *))dal_common_unavail,
    .l2_ipmcMode_set = (int32 (*)(uint32 , rtk_l2_ipmcMode_t ))dal_common_unavail,
    .l2_ipMcastAddr_init = (int32 (*)(uint32 , ipaddr_t , ipaddr_t , rtk_l2_ipMcastAddr_t *))dal_common_unavail,
    .l2_ipMcastAddr_add = dal_ssw_l2_ipMcastAddr_add,
    .l2_ipMcastAddr_del = dal_ssw_l2_ipMcastAddr_del,
    .l2_ipMcastAddr_get = dal_ssw_l2_ipMcastAddr_get,
    .l2_ipMcastAddr_set = dal_ssw_l2_ipMcastAddr_set,
    .l2_ipMcastAddr_add_with_index = dal_ssw_l2_ipMcastAddr_add_with_index,
    .l2_ipMcastAddr_get_with_index = dal_ssw_l2_ipMcastAddr_get_with_index,
    .l2_ipmc_routerPorts_get = (int32 (*)(uint32 , rtk_portmask_t *))dal_common_unavail,
    .l2_ipmc_routerPorts_set = (int32 (*)(uint32 , rtk_portmask_t *))dal_common_unavail,
    .l2_ipmcDstAddrMismatchAction_get = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_l2_ipmcMismatch_action_t *))dal_common_unavail,
    .l2_ipmcDstAddrMismatchAction_set = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_l2_ipmcMismatch_action_t ))dal_common_unavail,
    .l2_ipmcDstAddrMismatchPri_get = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_pri_t *))dal_common_unavail,
    .l2_ipmcDstAddrMismatchPri_set = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_pri_t ))dal_common_unavail,
    .l2_ipmcDstAddrMismatchPriEnable_get = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_pri_t *))dal_common_unavail,
    .l2_ipmcDstAddrMismatchPriEnable_set = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_pri_t ))dal_common_unavail,
    .l2_ipmcDstAddrMismatchDP_get = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , uint32 *))dal_common_unavail,
    .l2_ipmcDstAddrMismatchDP_set = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , uint32))dal_common_unavail,
    .l2_ipmcDstAddrMismatchDPEnable_get = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , uint32 *))dal_common_unavail,
    .l2_ipmcDstAddrMismatchDPEnable_set = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , uint32))dal_common_unavail,
    .l2_ipmcDstAddrMismatchAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_enable_t *))dal_common_unavail,
    .l2_ipmcDstAddrMismatchAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_l2_ipmc_mismatchType_t , rtk_enable_t ))dal_common_unavail,
    .l2_mcastFwdIndex_alloc = dal_ssw_l2_mcastFwdIndex_alloc,
    .l2_mcastFwdIndex_free = dal_ssw_l2_mcastFwdIndex_free,
    .l2_mcastFwdIndexFreeCount_get = dal_ssw_l2_mcastFwdIndexFreeCount_get,
    .l2_mcastFwdPortmask_set = dal_ssw_l2_mcastFwdPortmask_set,
    .l2_mcastFwdPortmask_get = dal_ssw_l2_mcastFwdPortmask_get,
    .l2_cpuMacAddr_add = dal_ssw_l2_cpuMacAddr_add,
    .l2_cpuMacAddr_del = dal_ssw_l2_cpuMacAddr_del,
    .l2_legalMoveToPorts_get = (int32 (*)(uint32 , rtk_port_t , rtk_portmask_t *))dal_common_unavail,
    .l2_legalMoveToPorts_set = (int32 (*)(uint32 , rtk_port_t , rtk_portmask_t *))dal_common_unavail,
    .l2_illegalPortMoveAction_get = (int32 (*)(uint32 , rtk_port_t , rtk_action_t *))dal_common_unavail,
    .l2_illegalPortMoveAction_set = (int32 (*)(uint32 , rtk_port_t , rtk_action_t ))dal_common_unavail,
    .l2_legalPortMoveAction_get = (int32 (*)(uint32 , rtk_port_t , rtk_action_t *))dal_common_unavail,
    .l2_legalPortMoveAction_set = (int32 (*)(uint32 , rtk_port_t , rtk_action_t ))dal_common_unavail,
    .l2_lookupMissFloodPortMask_get = dal_ssw_l2_lookupMissFloodPortMask_get,
    .l2_lookupMissFloodPortMask_set = dal_ssw_l2_lookupMissFloodPortMask_set,
    .l2_lookupMissFloodPortMask_add = dal_ssw_l2_lookupMissFloodPortMask_add,
    .l2_lookupMissFloodPortMask_del = dal_ssw_l2_lookupMissFloodPortMask_del,
    .l2_lookupMissAction_get = (int32 (*)(uint32 , rtk_l2_lookupMissType_t , rtk_action_t *))dal_common_unavail,
    .l2_lookupMissAction_set = (int32 (*)(uint32 , rtk_l2_lookupMissType_t , rtk_action_t ))dal_common_unavail,
    .l2_lookupMissPri_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .l2_lookupMissPri_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .l2_lookupMissPriEnable_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .l2_lookupMissPriEnable_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .l2_lookupMissDP_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .l2_lookupMissDP_set = (int32 (*)(uint32 , uint32))dal_common_unavail,
    .l2_lookupMissDPEnable_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .l2_lookupMissDPEnable_set = (int32 (*)(uint32 , uint32))dal_common_unavail,
    .l2_lookupMissAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .l2_lookupMissAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .l2_srcPortEgrFilterMask_get = dal_ssw_l2_srcPortEgrFilterMask_get,
    .l2_srcPortEgrFilterMask_set = dal_ssw_l2_srcPortEgrFilterMask_set,
    .l2_srcPortEgrFilterMask_add = dal_ssw_l2_srcPortEgrFilterMask_add,
    .l2_srcPortEgrFilterMask_del = dal_ssw_l2_srcPortEgrFilterMask_del,
    .l2_exceptionAddrAction_get = (int32 (*)(uint32 , rtk_l2_exceptionAddrType_t , rtk_action_t *))dal_common_unavail,
    .l2_exceptionAddrAction_set = (int32 (*)(uint32 , rtk_l2_exceptionAddrType_t , rtk_action_t ))dal_common_unavail,
    .l2_trapPri_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .l2_trapPri_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .l2_trapPriEnable_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .l2_trapPriEnable_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .l2_trapAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .l2_trapAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .l2_addrEntry_get = dal_ssw_l2_addrEntry_get,
    .l2_conflictAddr_get = dal_ssw_l2_conflictAddr_get,

    /* port related function */
    .port_init = dal_ssw_port_init,
    .port_link_get = dal_ssw_port_link_get,
    .port_speedDuplex_get = dal_ssw_port_speedDuplex_get,
    .port_flowctrl_get = dal_ssw_port_flowctrl_get,
    .port_phyAutoNegoEnable_get = dal_ssw_port_phyAutoNegoEnable_get,
    .port_phyAutoNegoEnable_set = dal_ssw_port_phyAutoNegoEnable_set,
    .port_phyAutoNegoAbility_get = dal_ssw_port_phyAutoNegoAbility_get,
    .port_phyAutoNegoAbility_set = dal_ssw_port_phyAutoNegoAbility_set,
    .port_phyForceModeAbility_get = dal_ssw_port_phyForceModeAbility_get,
    .port_phyForceModeAbility_set = dal_ssw_port_phyForceModeAbility_set,
    .port_phyReg_get = dal_ssw_port_phyReg_get,
    .port_phyReg_set = dal_ssw_port_phyReg_set,
    .port_cpuPortId_get = dal_ssw_port_cpuPortId_get,
    .port_isolation_get = dal_ssw_port_isolation_get,
    .port_isolation_set = dal_ssw_port_isolation_set,
    .port_isolation_add = dal_ssw_port_isolation_add,
    .port_isolation_del = dal_ssw_port_isolation_del,
    .port_phyComboPortMedia_get = dal_ssw_port_phyComboPortMedia_get,
    .port_phyComboPortMedia_set = dal_ssw_port_phyComboPortMedia_set,
    .port_adminEnable_get = dal_ssw_port_adminEnable_get,
    .port_adminEnable_set = dal_ssw_port_adminEnable_set,
    .port_backpressureEnable_get = dal_ssw_port_backpressureEnable_get,
    .port_backpressureEnable_set = dal_ssw_port_backpressureEnable_set,
    .port_linkChange_register = dal_ssw_port_linkChange_register,
    .port_linkChange_unregister = dal_ssw_port_linkChange_unregister,
    //.port_rtctResult_get = dal_ssw_port_rtctResult_get,
    //.port_rtct_start = dal_ssw_port_rtct_start,
    .port_greenEnable_get = dal_ssw_port_greenEnable_get,
    .port_greenEnable_set = dal_ssw_port_greenEnable_set,
    .port_udldEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .port_udldEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .port_udldLinkUpAutoTriggerEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .port_udldLinkUpAutoTriggerEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .port_udldTrigger_start = (int32 (*)(uint32 , rtk_port_t ))dal_common_unavail,
    .port_udldStatus_get = (int32 (*)(uint32 , rtk_port_t , rtk_port_udldStatus_t *))dal_common_unavail,
    .port_udldAutoDisableFailedPortEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .port_udldAutoDisableFailedPortEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .port_udldInterval_get = (int32 (*)(uint32 , rtk_port_udldInterval_t *))dal_common_unavail,
    .port_udldInterval_set = (int32 (*)(uint32 , rtk_port_udldInterval_t ))dal_common_unavail,
    .port_udldRetryCount_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .port_udldRetryCount_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .port_udldLedIndicateEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .port_udldLedIndicateEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .port_rldpEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .port_rldpEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .port_rldpStatus_get = (int32 (*)(uint32 , rtk_port_t , rtk_port_rldpNormalStatus_t *, rtk_port_rldpSelfStatus_t *))dal_common_unavail,
    .port_rldpAutoBlockEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .port_rldpAutoBlockEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .port_rldpInterval_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .port_rldpInterval_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .port_rldpSelfLoopAgingTime_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .port_rldpSelfLoopAgingTime_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .port_rldpNormalLoopAgingTime_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .port_rldpNormalLoopAgingTime_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .port_txEnable_get = dal_ssw_port_txEnable_get,
    .port_txEnable_set = dal_ssw_port_txEnable_set,
    .port_rxEnable_get = dal_ssw_port_rxEnable_get,
    .port_rxEnable_set = dal_ssw_port_rxEnable_set,
    .port_specialCongest_set = dal_ssw_port_specialCongest_set,
    .port_phyCrossOverMode_get = dal_ssw_port_phyCrossOverMode_get,
    .port_phyCrossOverMode_set = dal_ssw_port_phyCrossOverMode_set,
    .port_flowCtrlEnable_get = dal_ssw_port_flowCtrlEnable_get,
    .port_flowCtrlEnable_set = dal_ssw_port_flowCtrlEnable_set,
    .port_phyComboPortFiberMedia_get = dal_ssw_port_phyComboPortFiberMedia_get,
    .port_phyComboPortFiberMedia_set = dal_ssw_port_phyComboPortFiberMedia_set,
    .port_linkMedia_get = dal_ssw_port_linkMedia_get,
    .port_phyMasterSlave_get = (int32 (*)(uint32 , rtk_port_t, rtk_port_masterSlave_t *, rtk_port_masterSlave_t *))dal_common_unavail,
    .port_phyMasterSlave_set = (int32 (*)(uint32 , rtk_port_t, rtk_port_masterSlave_t ))dal_common_unavail,
    .port_linkDownPowerSavingEnable_get = dal_ssw_port_linkDownPowerSavingEnable_get,
    .port_linkDownPowerSavingEnable_set = dal_ssw_port_linkDownPowerSavingEnable_set,
    .port_gigaLiteEnable_get = dal_ssw_port_gigaLiteEnable_get,
    .port_gigaLiteEnable_set = dal_ssw_port_gigaLiteEnable_set,
    .port_fiberOAMLoopBackEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,

    /* trunk related function */
    .trunk_init = dal_ssw_trunk_init,
    .trunk_distributionAlgorithm_get = dal_ssw_trunk_distributionAlgorithm_get,
    .trunk_distributionAlgorithm_set = dal_ssw_trunk_distributionAlgorithm_set,
    .trunk_hashMappingTable_get = dal_ssw_trunk_hashMappingTable_get,
    .trunk_hashMappingTable_set = dal_ssw_trunk_hashMappingTable_set,
    .trunk_mode_get = dal_ssw_trunk_mode_get,
    .trunk_mode_set = dal_ssw_trunk_mode_set,
    .trunk_port_get = dal_ssw_trunk_port_get,
    .trunk_port_set = dal_ssw_trunk_port_set,
    .trunk_port_link_notification = dal_ssw_trunk_port_link_notification,
    .trunk_representPort_get = (int32 (*)(uint32 , uint32 , rtk_port_t *))dal_common_unavail,
    .trunk_representPort_set = (int32 (*)(uint32 , uint32 , rtk_port_t ))dal_common_unavail,
    .trunk_floodMode_get = (int32 (*)(uint32 , uint32 , rtk_trunk_floodMode_t *))dal_common_unavail,
    .trunk_floodMode_set = (int32 (*)(uint32 , uint32 , rtk_trunk_floodMode_t ))dal_common_unavail,
    .trunk_floodPort_get = (int32 (*)(uint32 , uint32 , rtk_port_t *))dal_common_unavail,
    .trunk_floodPort_set = (int32 (*)(uint32 , uint32 , rtk_port_t ))dal_common_unavail,
    
    /* spanning tree function */
    .stp_init = dal_ssw_stp_init,
    .stp_mstpInstance_create = dal_ssw_stp_mstpInstance_create,
    .stp_mstpInstance_destroy = dal_ssw_stp_mstpInstance_destroy,
    .stp_isMstpInstanceExist_get = dal_ssw_stp_isMstpInstanceExist_get,
    .stp_mstpState_get = dal_ssw_stp_mstpState_get,
    .stp_mstpState_set = dal_ssw_stp_mstpState_set,
    
    /* rate function */
    .rate_init = dal_ssw_rate_init,
    .rate_igrBandwidthCtrlEnable_get = dal_ssw_rate_igrBandwidthCtrlEnable_get,
    .rate_igrBandwidthCtrlEnable_set = dal_ssw_rate_igrBandwidthCtrlEnable_set,
    .rate_igrBandwidthCtrlRate_get = dal_ssw_rate_igrBandwidthCtrlRate_get,
    .rate_igrBandwidthCtrlRate_set = dal_ssw_rate_igrBandwidthCtrlRate_set,
    .rate_igrBandwidthCtrlIncludeIfg_get = dal_ssw_rate_igrBandwidthCtrlIncludeIfg_get,
    .rate_igrBandwidthCtrlIncludeIfg_set = dal_ssw_rate_igrBandwidthCtrlIncludeIfg_set,
    .rate_portIgrBandwidthCtrlIncludeIfg_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .rate_portIgrBandwidthCtrlIncludeIfg_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .rate_igrBandwidthFlowctrlEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .rate_igrBandwidthFlowctrlEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .rate_igrBandwidthFlowctrlThresh_get = (int32 (*)(uint32 , rtk_port_t , rtk_rate_thresh_t *))dal_common_unavail,
    .rate_igrBandwidthFlowctrlThresh_set = (int32 (*)(uint32 , rtk_port_t , rtk_rate_thresh_t *))dal_common_unavail,
    .rate_igrBandwidthCtrlFPEntry_get = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_rate_igr_fpEntry_t *))dal_common_unavail,
    .rate_igrBandwidthCtrlFPEntry_set = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_rate_igr_fpEntry_t *))dal_common_unavail,
    .rate_egrBandwidthCtrlEnable_get = dal_ssw_rate_egrBandwidthCtrlEnable_get,
    .rate_egrBandwidthCtrlEnable_set = dal_ssw_rate_egrBandwidthCtrlEnable_set,
    .rate_egrBandwidthCtrlRate_get = dal_ssw_rate_egrBandwidthCtrlRate_get,
    .rate_egrBandwidthCtrlRate_set = dal_ssw_rate_egrBandwidthCtrlRate_set,
    .rate_egrBandwidthCtrlIncludeIfg_get = dal_ssw_rate_egrBandwidthCtrlIncludeIfg_get,
    .rate_egrBandwidthCtrlIncludeIfg_set = dal_ssw_rate_egrBandwidthCtrlIncludeIfg_set,
    .rate_portEgrBandwidthCtrlIncludeIfg_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .rate_portEgrBandwidthCtrlIncludeIfg_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .rate_portEgrBandwidthCtrlBurstSize_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .rate_portEgrBandwidthCtrlBurstSize_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .rate_egrQueueBwCtrlEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , rtk_enable_t *))dal_common_unavail,
    .rate_egrQueueBwCtrlEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , rtk_enable_t ))dal_common_unavail,
    .rate_egrQueueBwCtrlRate_get = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , uint32 *))dal_common_unavail,
    .rate_egrQueueBwCtrlRate_set = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , uint32 ))dal_common_unavail,
    .rate_egrPortQueueBwCtrlBurstSize_get = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , uint32 *))dal_common_unavail,
    .rate_egrPortQueueBwCtrlBurstSize_set = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , uint32 ))dal_common_unavail,
    .rate_stormControlRate_get = dal_ssw_rate_stormControlRate_get,
    .rate_stormControlRate_set = dal_ssw_rate_stormControlRate_set,
    .rate_stormControlEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_rate_storm_group_t , rtk_enable_t *))dal_common_unavail,
    .rate_stormControlEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_rate_storm_group_t , rtk_enable_t ))dal_common_unavail,
    .rate_portStormControlRateMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_rate_storm_group_t , rtk_rate_storm_rateMode_t *))dal_common_unavail,
    .rate_portStormControlRateMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_rate_storm_group_t , rtk_rate_storm_rateMode_t ))dal_common_unavail,
    .rate_stormControlRateMode_get = (int32 (*)(uint32 , rtk_rate_storm_rateMode_t *))dal_common_unavail,
    .rate_stormControlRateMode_set = (int32 (*)(uint32 , rtk_rate_storm_rateMode_t ))dal_common_unavail,    
    .rate_portStormControlBurstSize_get = (int32 (*)(uint32 , rtk_port_t , rtk_rate_storm_group_t , uint32 *))dal_common_unavail,
    .rate_portStormControlBurstSize_set = (int32 (*)(uint32 , rtk_port_t , rtk_rate_storm_group_t , uint32 ))dal_common_unavail,
    .rate_stormControlIncludeIfg_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .rate_stormControlIncludeIfg_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .rate_stormControlExceed_get = (int32 (*)(uint32 , rtk_port_t , rtk_rate_storm_group_t , uint32 *))dal_common_unavail,
    .rate_igrBandwidthFCOffRate_set = dal_ssw_rate_igrBandwidthFCOffRate_set,
    .rate_stormControlRefreshMode_get = (int32 (*)(uint32 , rtk_rate_storm_rateMode_t *))dal_common_unavail,
    .rate_stormControlRefreshMode_set = (int32 (*)(uint32 , rtk_rate_storm_rateMode_t ))dal_common_unavail,

    /* Qos function */
    .qos_init = dal_ssw_qos_init,
    .qos_priSel_get = dal_ssw_qos_priSel_get,
    .qos_priSel_set = dal_ssw_qos_priSel_set,
    .qos_priSelGroup_get = (int32 (*)(uint32 , uint32 , rtk_qos_priSelWeight_t *))dal_common_unavail,
    .qos_priSelGroup_set = (int32 (*)(uint32 , uint32 , rtk_qos_priSelWeight_t *))dal_common_unavail,
    .qos_portPriSelGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portPriSelGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_portPri_get = dal_ssw_qos_portPri_get,
    .qos_portPri_set = dal_ssw_qos_portPri_set,
    .qos_portDp_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portDp_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_portInnerPri_get = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t *))dal_common_unavail,
    .qos_portInnerPri_set = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t ))dal_common_unavail,
    .qos_portOuterPri_get = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t *))dal_common_unavail,
    .qos_portOuterPri_set = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t ))dal_common_unavail,
    .qos_portOuterDEI_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portOuterDEI_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_dscpPriRemap_get = dal_ssw_qos_dscpPriRemap_get,
    .qos_dscpPriRemap_set = dal_ssw_qos_dscpPriRemap_set,
    .qos_dscpPriRemapGroup_get = (int32 (*)(uint32 , uint32 , uint32 , rtk_pri_t *, uint32 *))dal_common_unavail,
    .qos_dscpPriRemapGroup_set = (int32 (*)(uint32 , uint32 , uint32 , rtk_pri_t , uint32 ))dal_common_unavail,
    .qos_portDscpPriRemapGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portDscpPriRemapGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_1pPriRemap_get = dal_ssw_qos_1pPriRemap_get,
    .qos_1pPriRemap_set = dal_ssw_qos_1pPriRemap_set,
    .qos_1pPriRemapGroup_get = (int32 (*)(uint32 , uint32 , rtk_pri_t , rtk_pri_t *, uint32 *))dal_common_unavail,
    .qos_1pPriRemapGroup_set = (int32 (*)(uint32 , uint32 , rtk_pri_t , rtk_pri_t , uint32 ))dal_common_unavail,
    .qos_port1pPriRemapGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_port1pPriRemapGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_outer1pPriRemapGroup_get = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , rtk_pri_t *, uint32 *))dal_common_unavail,
    .qos_outer1pPriRemapGroup_set = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , rtk_pri_t , uint32 ))dal_common_unavail,
    .qos_portOuter1pPriRemapGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portOuter1pPriRemapGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_queueNum_get = dal_ssw_qos_queueNum_get,
    .qos_queueNum_set = dal_ssw_qos_queueNum_set,
    .qos_priMap_get = dal_ssw_qos_priMap_get,
    .qos_priMap_set = dal_ssw_qos_priMap_set,
    .qos_portPriMap_get = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t , rtk_qid_t *))dal_common_unavail,
    .qos_portPriMap_set = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t , rtk_qid_t ))dal_common_unavail,
    .qos_1pRemarkEnable_get = dal_ssw_qos_1pRemarkEnable_get,
    .qos_1pRemarkEnable_set = dal_ssw_qos_1pRemarkEnable_set,
    .qos_1pRemark_get = dal_ssw_qos_1pRemark_get,
    .qos_1pRemark_set = dal_ssw_qos_1pRemark_set,
    .qos_1pRemarkGroup_get = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , rtk_pri_t *))dal_common_unavail,
    .qos_1pRemarkGroup_set = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , rtk_pri_t ))dal_common_unavail,
    .qos_port1pRemarkGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_port1pRemarkGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_port1pPriMapGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_port1pPriMapGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_out1pRemarkEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .qos_out1pRemarkEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .qos_outer1pRemarkGroup_get = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , rtk_pri_t *, uint32 *))dal_common_unavail,
    .qos_outer1pRemarkGroup_set = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , rtk_pri_t , uint32))dal_common_unavail,
    .qos_portOuter1pRemarkGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portOuter1pRemarkGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_portOuter1pPriMapGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portOuter1pPriMapGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_dscpRemarkEnable_get = dal_ssw_qos_dscpRemarkEnable_get,
    .qos_dscpRemarkEnable_set = dal_ssw_qos_dscpRemarkEnable_set,
    .qos_dscpRemark_get = dal_ssw_qos_dscpRemark_get,
    .qos_dscpRemark_set = dal_ssw_qos_dscpRemark_set,
    .qos_dscpRemarkGroup_get = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , uint32 *))dal_common_unavail,
    .qos_dscpRemarkGroup_set = (int32 (*)(uint32 , uint32 , rtk_pri_t , uint32 , uint32 ))dal_common_unavail,
    .qos_portdscpRemarkGroup_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .qos_portdscpRemarkGroup_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .qos_schedulingAlgorithm_get = dal_ssw_qos_schedulingAlgorithm_get,
    .qos_schedulingAlgorithm_set = dal_ssw_qos_schedulingAlgorithm_set,
    .qos_schedulingQueue_get = dal_ssw_qos_schedulingQueue_get,
    .qos_schedulingQueue_set = dal_ssw_qos_schedulingQueue_set,
    .qos_wfqFixedBandwidthEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , rtk_enable_t *))dal_common_unavail,
    .qos_wfqFixedBandwidthEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , rtk_enable_t ))dal_common_unavail,
    .qos_congAvoidAlgo_get = (int32 (*)(uint32 , rtk_qos_congAvoidAlgo_t *))dal_common_unavail,
    .qos_congAvoidAlgo_set = (int32 (*)(uint32 , rtk_qos_congAvoidAlgo_t ))dal_common_unavail,
    .qos_congAvoidQueueThreshEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .qos_congAvoidQueueThreshEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .qos_congAvoidPortThreshEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .qos_congAvoidPortThreshEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .qos_congAvoidSysThreshEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .qos_congAvoidSysThreshEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .qos_congAvoidSysThresh_get = (int32 (*)(uint32 ,uint32 , rtk_qos_congAvoidThresh_t *))dal_common_unavail,
    .qos_congAvoidSysThresh_set = (int32 (*)(uint32 ,uint32 , rtk_qos_congAvoidThresh_t *))dal_common_unavail,
    .qos_congAvoidPortThresh_get = (int32 (*)(uint32 ,rtk_port_t ,uint32 , rtk_qos_congAvoidThresh_t *))dal_common_unavail,
    .qos_congAvoidPortThresh_set = (int32 (*)(uint32 ,rtk_port_t ,uint32 , rtk_qos_congAvoidThresh_t *))dal_common_unavail,
    .qos_congAvoidQueueThresh_get = (int32 (*)(uint32 ,rtk_port_t , rtk_qid_t , rtk_qos_congAvoidThresh_t *))dal_common_unavail,
    .qos_congAvoidQueueThresh_set = (int32 (*)(uint32 ,rtk_port_t , rtk_qid_t , rtk_qos_congAvoidThresh_t *))dal_common_unavail,
    .qos_wredSysThresh_get = (int32 (*)(uint32 , uint32 , rtk_qos_wredThresh_t *))dal_common_unavail,
    .qos_wredSysThresh_set = (int32 (*)(uint32 , uint32 , rtk_qos_wredThresh_t *))dal_common_unavail,
    .qos_wredWeight_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .qos_wredWeight_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .qos_wredMpd_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .qos_wredMpd_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .qos_wredEcnEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .qos_wredEcnEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .qos_wredCntReverseEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .qos_wredCntReverseEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    
    /* trap function */
    .trap_init = dal_ssw_trap_init,
    .trap_1xMacChangePort2CpuEnable_get = dal_ssw_trap_1xMacChangePort2CpuEnable_get,
    .trap_1xMacChangePort2CpuEnable_set = dal_ssw_trap_1xMacChangePort2CpuEnable_set,
    .trap_igmpCtrlPkt2CpuEnable_get = dal_ssw_trap_igmpCtrlPkt2CpuEnable_get,
    .trap_igmpCtrlPkt2CpuEnable_set = dal_ssw_trap_igmpCtrlPkt2CpuEnable_set,
    .trap_l2McastPkt2CpuEnable_get = dal_ssw_trap_l2McastPkt2CpuEnable_get,
    .trap_l2McastPkt2CpuEnable_set = dal_ssw_trap_l2McastPkt2CpuEnable_set,
    .trap_ipMcastPkt2CpuEnable_get = dal_ssw_trap_ipMcastPkt2CpuEnable_get,
    .trap_ipMcastPkt2CpuEnable_set = dal_ssw_trap_ipMcastPkt2CpuEnable_set,
    .trap_reasonTrapToCPUPriority_get = dal_ssw_trap_reasonTrapToCPUPriority_get,
    .trap_reasonTrapToCPUPriority_set = dal_ssw_trap_reasonTrapToCPUPriority_set,
    .trap_pkt2CpuEnable_get = dal_ssw_trap_pkt2CpuEnable_get,
    .trap_pkt2CpuEnable_set = dal_ssw_trap_pkt2CpuEnable_set,
    .trap_rmaAction_get = dal_ssw_trap_rmaAction_get,
    .trap_rmaAction_set = dal_ssw_trap_rmaAction_set,
    .trap_rmaPri_get = (int32 (*)(uint32 , rtk_mac_t *, rtk_pri_t *))dal_common_unavail,
    .trap_rmaPri_set = (int32 (*)(uint32 , rtk_mac_t *, rtk_pri_t ))dal_common_unavail,
    .trap_rmaPriEnable_get = (int32 (*)(uint32 , rtk_mac_t *, rtk_enable_t *))dal_common_unavail,
    .trap_rmaPriEnable_set = (int32 (*)(uint32 , rtk_mac_t *, rtk_enable_t ))dal_common_unavail,
    .trap_rmaCpuTagAddEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_rmaCpuTagAddEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_rmaVlanCheckEnable_get = (int32 (*)(uint32 , rtk_mac_t *, rtk_enable_t *))dal_common_unavail,
    .trap_rmaVlanCheckEnable_set = (int32 (*)(uint32 , rtk_mac_t *, rtk_enable_t ))dal_common_unavail,
    .trap_userDefineRma_get = (int32 (*)(uint32 , uint32 , rtk_trap_userDefinedRma_t *))dal_common_unavail,
    .trap_userDefineRma_set = (int32 (*)(uint32 , uint32 , rtk_trap_userDefinedRma_t *))dal_common_unavail,
    .trap_userDefineRmaAction_get = (int32 (*)(uint32 , uint32 , rtk_trap_rma_action_t *))dal_common_unavail,
    .trap_userDefineRmaAction_set = (int32 (*)(uint32 , uint32 , rtk_trap_rma_action_t ))dal_common_unavail,
    .trap_userDefineRmaPri_get = (int32 (*)(uint32 , uint32 , rtk_pri_t *))dal_common_unavail,
    .trap_userDefineRmaPri_set = (int32 (*)(uint32 , uint32 , rtk_pri_t ))dal_common_unavail,
    .trap_userDefineRmaPriEnable_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_userDefineRmaPriEnable_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_userDefineRmaVlanCheckEnable_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_userDefineRmaVlanCheckEnable_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_userDefineRmaStpBlockEnable_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_userDefineRmaStpBlockEnable_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_mgmtFrameAction_get = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_action_t *))dal_common_unavail,
    .trap_mgmtFrameAction_set = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_action_t ))dal_common_unavail,
    .trap_mgmtFramePri_get = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_pri_t *))dal_common_unavail,
    .trap_mgmtFramePri_set = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_pri_t ))dal_common_unavail,
    .trap_mgmtFramePriEnable_get = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_enable_t *))dal_common_unavail,
    .trap_mgmtFramePriEnable_set = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_enable_t ))dal_common_unavail,
    .trap_mgmtFrameVlanCheck_get = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_enable_t *))dal_common_unavail,
    .trap_mgmtFrameVlanCheck_set = (int32 (*)(uint32 , rtk_trap_mgmtType_t , rtk_enable_t ))dal_common_unavail,
    .trap_userDefineMgmt_get = (int32 (*)(uint32 , uint32 , rtk_trap_userDefinedMgmt_t *))dal_common_unavail,
    .trap_userDefineMgmt_set = (int32 (*)(uint32 , uint32 , rtk_trap_userDefinedMgmt_t *))dal_common_unavail,
    .trap_userDefineMgmtAction_get = (int32 (*)(uint32 , uint32 , rtk_action_t *))dal_common_unavail,
    .trap_userDefineMgmtAction_set = (int32 (*)(uint32 , uint32 , rtk_action_t ))dal_common_unavail,
    .trap_userDefineMgmtPri_get = (int32 (*)(uint32 , uint32 , rtk_pri_t *))dal_common_unavail,
    .trap_userDefineMgmtPri_set = (int32 (*)(uint32 , uint32 , rtk_pri_t ))dal_common_unavail,
    .trap_userDefineMgmtPriEnable_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_userDefineMgmtPriEnable_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_userDefineMgmtVlanCheck_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_userDefineMgmtVlanCheck_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_portMgmtFrameAction_get = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_action_t *))dal_common_unavail,
    .trap_portMgmtFrameAction_set = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_action_t ))dal_common_unavail,
    .trap_portMgmtFramePri_get = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_pri_t *))dal_common_unavail,
    .trap_portMgmtFramePri_set = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_pri_t ))dal_common_unavail,
    .trap_portMgmtFramePriEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_enable_t *))dal_common_unavail,
    .trap_portMgmtFramePriEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_enable_t ))dal_common_unavail,
    .trap_portMgmtFrameVlanCheck_get = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_enable_t *))dal_common_unavail,
    .trap_portMgmtFrameVlanCheck_set = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_enable_t ))dal_common_unavail,
    .trap_portMgmtFrameCrossVlan_get = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_enable_t *))dal_common_unavail,
    .trap_portMgmtFrameCrossVlan_set = (int32 (*)(uint32 , rtk_port_t , rtk_trap_mgmtType_t , rtk_enable_t ))dal_common_unavail,
    .trap_ipWithOptionHeaderAction_get = (int32 (*)(uint32 , rtk_port_t , rtk_ip_family_t , rtk_action_t *))dal_common_unavail,
    .trap_ipWithOptionHeaderAction_set = (int32 (*)(uint32 , rtk_port_t , rtk_ip_family_t , rtk_action_t ))dal_common_unavail,
    .trap_ipWithOptionHeaderPri_get = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t *))dal_common_unavail,
    .trap_ipWithOptionHeaderPri_set = (int32 (*)(uint32 , rtk_port_t , rtk_pri_t ))dal_common_unavail,
    .trap_ipWithOptionHeaderPriEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .trap_ipWithOptionHeaderPriEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .trap_ipWithOptionHeaderAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .trap_ipWithOptionHeaderAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .trap_pktWithCFIAction_get = (int32 (*)(uint32 , rtk_action_t *))dal_common_unavail,
    .trap_pktWithCFIAction_set = (int32 (*)(uint32 , rtk_action_t ))dal_common_unavail,
    .trap_pktWithCFIPri_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .trap_pktWithCFIPri_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .trap_pktWithCFIPriEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .trap_pktWithCFIPriEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .trap_pktWithCFIAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .trap_pktWithCFIAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .trap_cfmFrameAction_get = (int32 (*)(uint32 , uint32 , rtk_action_t *))dal_common_unavail,
    .trap_cfmFrameAction_set = (int32 (*)(uint32 , uint32 , rtk_action_t ))dal_common_unavail,
    .trap_cfmFrameTrapPri_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .trap_cfmFrameTrapPri_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .trap_cfmFrameTrapPriEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_cfmFrameTrapPriEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_cfmFrameTrapAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .trap_cfmFrameTrapAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .trap_oamPDUAction_get = (int32 (*)(uint32 , rtk_action_t *))dal_common_unavail,
    .trap_oamPDUAction_set = (int32 (*)(uint32 , rtk_action_t ))dal_common_unavail,
    .trap_oamPDUPri_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .trap_oamPDUPri_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .trap_oamPDUPriEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .trap_oamPDUPriEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .trap_oamPDUTrapAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .trap_oamPDUTrapAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .trap_mgmtIpCheck_get = (int32 (*)(uint32 , rtk_trap_mgmtIpType_t , rtk_enable_t *))dal_common_unavail,
    .trap_mgmtIpCheck_set = (int32 (*)(uint32 , rtk_trap_mgmtIpType_t , rtk_enable_t ))dal_common_unavail,
    
    /* stat function */
    .stat_init = dal_ssw_stat_init,
    .stat_global_reset = dal_ssw_stat_global_reset,
    .stat_port_reset = dal_ssw_stat_port_reset,
    .stat_global_get = dal_ssw_stat_global_get,
    .stat_global_getAll = dal_ssw_stat_global_getAll,
    .stat_port_get = dal_ssw_stat_port_get,
    .stat_port_getAll = dal_ssw_stat_port_getAll,
    .stat_smon_get = (int32 (*)(uint32 , rtk_pri_t , rtk_stat_smon_type_t ,  uint64 *))dal_common_unavail,
    .stat_smon_getAll = (int32 (*)(uint32 , rtk_pri_t , rtk_stat_smon_cntr_t *))dal_common_unavail,
    
    /* switch function */
    .switch_init = dal_ssw_switch_init,
    .switch_maxPktLen_get = dal_ssw_switch_maxPktLen_get,
    .switch_maxPktLen_set = dal_ssw_switch_maxPktLen_set,
    .switch_portMaxPktLen_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .switch_portMaxPktLen_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .switch_portSnapMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_snapMode_t *))dal_common_unavail,
    .switch_portSnapMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_snapMode_t ))dal_common_unavail,
    .switch_chksumFailAction_get = (int32 (*)(uint32 , rtk_port_t , rtk_switch_chksum_fail_t , rtk_action_t *))dal_common_unavail,
    .switch_chksumFailAction_set = (int32 (*)(uint32 , rtk_port_t , rtk_switch_chksum_fail_t , rtk_action_t ))dal_common_unavail,
    .switch_recalcCRCEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .switch_recalcCRCEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .switch_mgmtVlanId_get = (int32 (*)(uint32 , rtk_vlan_t *))dal_common_unavail,
    .switch_mgmtVlanId_set = (int32 (*)(uint32 , rtk_vlan_t ))dal_common_unavail,
    .switch_outerMgmtVlanId_get = (int32 (*)(uint32 , rtk_vlan_t *))dal_common_unavail,
    .switch_outerMgmtVlanId_set = (int32 (*)(uint32 , rtk_vlan_t ))dal_common_unavail,
    .switch_mgmtMacAddr_get = dal_ssw_switch_mgmtMacAddr_get,
    .switch_mgmtMacAddr_set = dal_ssw_switch_mgmtMacAddr_set,
    .switch_IPv4Addr_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .switch_IPv4Addr_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .switch_IPv6Addr_get = (int32 (*)(uint32 , rtk_ipv6_addr_t *))dal_common_unavail,
    .switch_IPv6Addr_set = (int32 (*)(uint32 , rtk_ipv6_addr_t ))dal_common_unavail,
    .switch_hwInterfaceDelayEnable_get = dal_ssw_switch_hwInterfaceDelayEnable_get,
    .switch_hwInterfaceDelayEnable_set = dal_ssw_switch_hwInterfaceDelayEnable_set,
    .switch_pkt2CpuFormat_get = (int32 (*)(uint32 , rtk_pktFormat_t *))dal_common_unavail,
    .switch_pkt2CpuFormat_set = (int32 (*)(uint32 , rtk_pktFormat_t ))dal_common_unavail,
    .switch_softwareResetCounter_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,

    /* dot1x function */
    .dot1x_init = dal_ssw_dot1x_init,
    .dot1x_unauthPacketOper_get = dal_ssw_dot1x_unauthPacketOper_get,
    .dot1x_unauthPacketOper_set = dal_ssw_dot1x_unauthPacketOper_set,
    .dot1x_portUnauthPacketOper_get = (int32 (*)(uint32 , rtk_port_t port, rtk_dot1x_unauth_action_t *))dal_common_unavail,
    .dot1x_portUnauthPacketOper_set = (int32 (*)(uint32 , rtk_port_t port, rtk_dot1x_unauth_action_t ))dal_common_unavail,
    .dot1x_portUnauthTagPacketOper_get = (int32 (*)(uint32 , rtk_port_t , rtk_dot1x_unauth_action_t *))dal_common_unavail,
    .dot1x_portUnauthTagPacketOper_set = (int32 (*)(uint32 , rtk_port_t , rtk_dot1x_unauth_action_t ))dal_common_unavail,
    .dot1x_portUnauthUntagPacketOper_get = (int32 (*)(uint32 , rtk_port_t , rtk_dot1x_unauth_action_t *))dal_common_unavail,
    .dot1x_portUnauthUntagPacketOper_set = (int32 (*)(uint32 , rtk_port_t , rtk_dot1x_unauth_action_t ))dal_common_unavail,
    .dot1x_eapolFrame2CpuEnable_get = dal_ssw_dot1x_eapolFrame2CpuEnable_get,
    .dot1x_eapolFrame2CpuEnable_set = dal_ssw_dot1x_eapolFrame2CpuEnable_set,
    .dot1x_portBasedEnable_get = dal_ssw_dot1x_portBasedEnable_get,
    .dot1x_portBasedEnable_set = dal_ssw_dot1x_portBasedEnable_set,
    .dot1x_portBasedAuthStatus_get = dal_ssw_dot1x_portBasedAuthStatus_get,
    .dot1x_portBasedAuthStatus_set = dal_ssw_dot1x_portBasedAuthStatus_set,
    .dot1x_portBasedDirection_get = dal_ssw_dot1x_portBasedDirection_get,
    .dot1x_portBasedDirection_set = dal_ssw_dot1x_portBasedDirection_set,
    .dot1x_macBasedEnable_get = dal_ssw_dot1x_macBasedEnable_get,
    .dot1x_macBasedEnable_set = dal_ssw_dot1x_macBasedEnable_set,
    .dot1x_macBasedAuthMac_add = dal_ssw_dot1x_macBasedAuthMac_add,
    .dot1x_macBasedAuthMac_del = dal_ssw_dot1x_macBasedAuthMac_del,
    .dot1x_macBasedDirection_get = dal_ssw_dot1x_macBasedDirection_get,
    .dot1x_macBasedDirection_set = dal_ssw_dot1x_macBasedDirection_set,
    .dot1x_portGuestVlan_get = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_t *))dal_common_unavail,
    .dot1x_portGuestVlan_set = (int32 (*)(uint32 , rtk_port_t , rtk_vlan_t ))dal_common_unavail,
    .dot1x_guestVlanBehavior_get = (int32 (*)(uint32 , rtk_dot1x_guestVlanBehavior_t *))dal_common_unavail,
    .dot1x_guestVlanBehavior_set = (int32 (*)(uint32 , rtk_dot1x_guestVlanBehavior_t ))dal_common_unavail,
    .dot1x_guestVlanRouteBehavior_get = (int32 (*)(uint32 , rtk_action_t *))dal_common_unavail,
    .dot1x_guestVlanRouteBehavior_set = (int32 (*)(uint32 , rtk_action_t ))dal_common_unavail,
    .dot1x_trapPri_get = (int32 (*)(uint32 , rtk_pri_t *))dal_common_unavail,
    .dot1x_trapPri_set = (int32 (*)(uint32 , rtk_pri_t ))dal_common_unavail,
    .dot1x_trapPriEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .dot1x_trapPriEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .dot1x_trapAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .dot1x_trapAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,

    /* Mirror Function */
    .mirror_init = dal_ssw_mirror_init,
    .mirror_portBased_create = dal_ssw_mirror_portBased_create,
    .mirror_portBased_destroy = dal_ssw_mirror_portBased_destroy,
    .mirror_portBased_destroyAll = dal_ssw_mirror_portBased_destroyAll,
    .mirror_portBased_get = dal_ssw_mirror_portBased_get,
    .mirror_portBased_set = dal_ssw_mirror_portBased_set,
    .mirror_group_init = (int32 (*)(uint32 , rtk_mirror_entry_t *))dal_common_unavail,
    .mirror_group_get = (int32 (*)(uint32 , uint32 , rtk_mirror_entry_t *))dal_common_unavail,
    .mirror_group_set = (int32 (*)(uint32 , uint32 , rtk_mirror_entry_t *))dal_common_unavail,
    .mirror_egrMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_egrMode_t *))dal_common_unavail,
    .mirror_egrMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_egrMode_t ))dal_common_unavail,
    .mirror_portRspanIgrMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanIgrMode_t *))dal_common_unavail,
    .mirror_portRspanIgrMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanIgrMode_t ))dal_common_unavail,
    .mirror_portRspanEgrMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanEgrMode_t *))dal_common_unavail,
    .mirror_portRspanEgrMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanEgrMode_t ))dal_common_unavail,
    .mirror_rspanIgrTag_get = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanIgrTag_t *))dal_common_unavail,
    .mirror_rspanIgrTag_set = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanIgrTag_t *))dal_common_unavail,
    .mirror_rspanEgrTag_get = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanEgrTag_t *))dal_common_unavail,
    .mirror_rspanEgrTag_set = (int32 (*)(uint32 , rtk_port_t , rtk_mirror_rspanEgrTag_t *))dal_common_unavail,
    .mirror_sflowMirrorSeed_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .mirror_sflowMirrorSeed_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .mirror_sflowMirrorSampleEnable_get = (int32 (*)(uint32 , uint32 , rtk_enable_t *))dal_common_unavail,
    .mirror_sflowMirrorSampleEnable_set = (int32 (*)(uint32 , uint32 , rtk_enable_t ))dal_common_unavail,
    .mirror_sflowMirrorSampleRate_get = (int32 (*)(uint32 , uint32 , uint32 *))dal_common_unavail,
    .mirror_sflowMirrorSampleRate_set = (int32 (*)(uint32 , uint32 , uint32 ))dal_common_unavail,
    .mirror_sflowMirrorSampleStat_get = (int32 (*)(uint32 , uint32 , rtk_mirror_sampleStat_t *))dal_common_unavail,
    .mirror_sflowPortSeed_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .mirror_sflowPortSeed_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .mirror_sflowPortIgrSampleEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .mirror_sflowPortIgrSampleEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .mirror_sflowPortIgrSampleRate_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .mirror_sflowPortIgrSampleRate_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .mirror_sflowPortEgrSampleEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .mirror_sflowPortEgrSampleEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .mirror_sflowPortEgrSampleRate_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .mirror_sflowPortEgrSampleRate_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .mirror_sflowAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_enable_t *))dal_common_unavail,
    .mirror_sflowAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    
    /* filter function */
    .filter_blkCutline_get = dal_ssw_filter_blkCutline_get,
    .filter_blkCutline_set = dal_ssw_filter_blkCutline_set,
    .filter_pieEnable_get = dal_ssw_filter_pieEnable_get,
    .filter_pieEnable_set = dal_ssw_filter_pieEnable_set,
    .filter_init = dal_ssw_filter_init,
    .filter_patternMatch_get = dal_ssw_filter_patternMatch_get,
    .filter_patternMatch_set = dal_ssw_filter_patternMatch_set,
    .filter_flowTbl_del = dal_ssw_filter_flowTbl_del,
    .filter_flowTbl_delAll = dal_ssw_filter_flowTbl_delAll,
    .filter_flowTbl_get = dal_ssw_filter_flowTbl_get,
    .filter_flowTbl_set = dal_ssw_filter_flowTbl_set,
    .filter_flowTbl_add = dal_ssw_filter_flowTbl_add,
    .filter_flowTbl_validate = dal_ssw_filter_flowTbl_validate,
    .filter_flowTbl_invalidate = dal_ssw_filter_flowTbl_invalidate,
    .filter_igrAcl_del = dal_ssw_filter_igrAcl_del,
    .filter_igrAcl_delAll = dal_ssw_filter_igrAcl_delAll,
    .filter_igrAcl_get = dal_ssw_filter_igrAcl_get,
    .filter_igrAcl_set = dal_ssw_filter_igrAcl_set,
    .filter_igrAcl_add = dal_ssw_filter_igrAcl_add,
    .filter_igrAcl_validate = dal_ssw_filter_igrAcl_validate,
    .filter_igrAcl_invalidate = dal_ssw_filter_igrAcl_invalidate,
    .filter_igrAclRateLimit_get = dal_ssw_filter_igrAclRateLimit_get,
    .filter_igrAclRateLimit_set = dal_ssw_filter_igrAclRateLimit_set,
    .filter_stat_get = dal_ssw_filter_stat_get,
    .filter_stat_set = dal_ssw_filter_stat_set,
    .filter_macBasedVlan_add = dal_ssw_filter_macBasedVlan_add,
    .filter_macBasedVlan_del = dal_ssw_filter_macBasedVlan_del,
    .filter_macBasedVlan_delAll = dal_ssw_filter_macBasedVlan_delAll,
    .filter_igrVlanXlate_add = dal_ssw_filter_igrVlanXlate_add,
    .filter_igrVlanXlate_del = dal_ssw_filter_igrVlanXlate_del,
    .filter_igrVlanXlate_delAll = dal_ssw_filter_igrVlanXlate_delAll,
    .filter_egrVlanXlate_add = dal_ssw_filter_egrVlanXlate_add,
    .filter_egrVlanXlate_del = dal_ssw_filter_egrVlanXlate_del,
    .filter_egrVlanXlate_delAll = dal_ssw_filter_egrVlanXlate_delAll,
    .filter_stagVlan_add = dal_ssw_filter_stagVlan_add,
    .filter_stagVlan_del = dal_ssw_filter_stagVlan_del,
    .filter_stagVlan_delAll = dal_ssw_filter_stagVlan_delAll,
    .filter_ipSubnetBasedVlan_add = dal_ssw_filter_ipSubnetBasedVlan_add,
    .filter_ipSubnetBasedVlan_del = dal_ssw_filter_ipSubnetBasedVlan_del,
    .filter_ipSubnetBasedVlan_delAll = dal_ssw_filter_ipSubnetBasedVlan_delAll,
    .filter_protoAndPortBasedVlan_add = dal_ssw_filter_protoAndPortBasedVlan_add,
    .filter_protoAndPortBasedVlan_del = dal_ssw_filter_protoAndPortBasedVlan_del,
    .filter_protoAndPortBasedVlan_delAll = dal_ssw_filter_protoAndPortBasedVlan_delAll,
    
    /* flowcontrol function */
    .flowctrl_init = dal_ssw_flowctrl_init,
    .flowctrl_portEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .flowctrl_portEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .flowctrl_portPauseForceModeEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .flowctrl_portPauseForceModeEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .flowctrl_pauseOnAction_get = (int32 (*)(uint32 , rtk_port_t , rtk_flowctrl_pauseOnAction_t *))dal_common_unavail,
    .flowctrl_pauseOnAction_set = (int32 (*)(uint32 , rtk_port_t , rtk_flowctrl_pauseOnAction_t ))dal_common_unavail,
    .flowctrl_pauseOnAllowedPageNum_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .flowctrl_pauseOnAllowedPageNum_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .flowctrl_pauseOnAllowedPktNum_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .flowctrl_pauseOnAllowedPktNum_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .flowctrl_igrSystemPauseThresh_get = dal_ssw_flowctrl_igrSystemPauseThresh_get,
    .flowctrl_igrSystemPauseThresh_set = dal_ssw_flowctrl_igrSystemPauseThresh_set,
    .flowctrl_igrPortPauseThresh_get = dal_ssw_flowctrl_igrPortPauseThresh_get,
    .flowctrl_igrPortPauseThresh_set = dal_ssw_flowctrl_igrPortPauseThresh_set,
    .flowctrl_egrPortDropMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_flowctrl_egrDropMode_t *))dal_common_unavail,
    .flowctrl_egrPortDropMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_flowctrl_egrDropMode_t ))dal_common_unavail,
    .flowctrl_egrPortDropForceModeEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .flowctrl_egrPortDropForceModeEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .flowctrl_igrSystemCongestThresh_get = dal_ssw_flowctrl_igrSystemCongestThresh_get,
    .flowctrl_igrSystemCongestThresh_set = dal_ssw_flowctrl_igrSystemCongestThresh_set,
    .flowctrl_igrPortCongestThresh_get = dal_ssw_flowctrl_igrPortCongestThresh_get,
    .flowctrl_igrPortCongestThresh_set = dal_ssw_flowctrl_igrPortCongestThresh_set,
    .flowctrl_egrSystemDropThresh_get = dal_ssw_flowctrl_egrSystemDropThresh_get,
    .flowctrl_egrSystemDropThresh_set = dal_ssw_flowctrl_egrSystemDropThresh_set,
    .flowctrl_egrPortDropThresh_get = dal_ssw_flowctrl_egrPortDropThresh_get,
    .flowctrl_egrPortDropThresh_set = dal_ssw_flowctrl_egrPortDropThresh_set,
    .flowctrl_egrPortQueueDropThresh_get = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , rtk_flowctrl_drop_thresh_t *))dal_common_unavail,
    .flowctrl_egrPortQueueDropThresh_set = (int32 (*)(uint32 , rtk_port_t , rtk_qid_t , rtk_flowctrl_drop_thresh_t *))dal_common_unavail,
    .flowctrl_egrQueueDropThresh_get = dal_ssw_flowctrl_egrQueueDropThresh_get,
    .flowctrl_egrQueueDropThresh_set = dal_ssw_flowctrl_egrQueueDropThresh_set,
    .flowctrl_egrPortDropRefCongestEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .flowctrl_egrPortDropRefCongestEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,

    /* SVLAN function */
    .svlan_init = dal_ssw_svlan_init,
    .svlan_create = dal_ssw_svlan_create,
    .svlan_destroy = dal_ssw_svlan_destroy,
    .svlan_portSvid_get = dal_ssw_svlan_portSvid_get,
    .svlan_portSvid_set = dal_ssw_svlan_portSvid_set,
    .svlan_servicePort_add = dal_ssw_svlan_servicePort_add,
    .svlan_servicePort_del = dal_ssw_svlan_servicePort_del,
    .svlan_servicePort_get = dal_ssw_svlan_servicePort_get,
    .svlan_servicePort_set = dal_ssw_svlan_servicePort_set,
    .svlan_memberPort_add = dal_ssw_svlan_memberPort_add,
    .svlan_memberPort_del = dal_ssw_svlan_memberPort_del,
    .svlan_memberPort_get = dal_ssw_svlan_memberPort_get,
    .svlan_memberPort_set = dal_ssw_svlan_memberPort_set,
    .svlan_memberPortEntry_get = dal_ssw_svlan_memberPortEntry_get,
    .svlan_memberPortEntry_set = dal_ssw_svlan_memberPortEntry_set,
    .svlan_nextValidMemberPortEntry_get = dal_ssw_svlan_nextValidMemberPortEntry_get,
    .svlan_tpidEntry_get = dal_ssw_svlan_tpidEntry_get,
    .svlan_tpidEntry_set = dal_ssw_svlan_tpidEntry_set,

    /* EEE function */
    .eee_init = dal_ssw_eee_init,
    .eee_portEnable_get = dal_ssw_eee_portEnable_get,
    .eee_portEnable_set = dal_ssw_eee_portEnable_set,
    .eeep_portEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .eeep_portEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,

    /* Security function */
    .sec_init = (int32 (*)(uint32 ))dal_common_unavail,
    .sec_minIPv6FragLen_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .sec_minIPv6FragLen_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .sec_maxPingLen_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .sec_maxPingLen_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .sec_minTCPHdrLen_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .sec_minTCPHdrLen_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .sec_smurfNetmaskLen_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .sec_smurfNetmaskLen_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    
    /* L3 function */
    .l3_init = (int32 (*)(uint32 ))dal_common_unavail,
    .l3_ttlExpireAction_get = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_action_t *))dal_common_unavail,
    .l3_ttlExpireAction_set = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_action_t ))dal_common_unavail,
    .l3_ttlExpireTrapPri_get = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_pri_t *))dal_common_unavail,
    .l3_ttlExpireTrapPri_set = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_pri_t ))dal_common_unavail,
    .l3_ttlExpireTrapPriEnable_get = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_pri_t *))dal_common_unavail,
    .l3_ttlExpireTrapPriEnable_set = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_pri_t ))dal_common_unavail,
    .l3_ttlExpireTrapDP_get = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , uint32 *))dal_common_unavail,
    .l3_ttlExpireTrapDP_set = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , uint32 ))dal_common_unavail,
    .l3_ttlExpireTrapDPEnable_get = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_pri_t *))dal_common_unavail,
    .l3_ttlExpireTrapDPEnable_set = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_pri_t ))dal_common_unavail,
    .l3_ttlExpireAddCPUTagEnable_get = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_enable_t *))dal_common_unavail,
    .l3_ttlExpireAddCPUTagEnable_set = (int32 (*)(uint32 , rtk_l3_ttlExpireType_t , rtk_enable_t ))dal_common_unavail,
    
    /* OAM function */
    .oam_init = (int32 (*)(uint32 ))dal_common_unavail,
    .oam_oamCounter_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .oam_txTestFrame_start = (int32 (*)(uint32 , rtk_oam_testFrameCfg_t *))dal_common_unavail,
    .oam_txTestFrame_stop = (int32 (*)(uint32 ))dal_common_unavail,
    .oam_txStatusOfTestFrame_get = (int32 (*)(uint32 , rtk_oam_testFrameTxStatus_t *, uint64 *))dal_common_unavail,
    .oam_loopbackMode_get = (int32 (*)(uint32 , rtk_port_t , rtk_oam_loopbackMode_t *))dal_common_unavail,
    .oam_loopbackMode_set = (int32 (*)(uint32 , rtk_port_t , rtk_oam_loopbackMode_t ))dal_common_unavail,
    .oam_loopbackCtrl_get = (int32 (*)(uint32 , rtk_port_t , rtk_oam_loopbackCtrl_t *))dal_common_unavail,
    .oam_loopbackCtrl_set = (int32 (*)(uint32 , rtk_port_t , rtk_oam_loopbackCtrl_t *))dal_common_unavail,
    .oam_DyingGaspSend_start = (int32 (*)(uint32 , rtk_portmask_t *))dal_common_unavail,
    .oam_dyingGaspPayload_set = (int32 (*)(uint32 , rtk_port_t , uint8 * ))dal_common_unavail,
    .oam_dyingGaspSend_set = (int32 (*)(uint32 , rtk_enable_t ))dal_common_unavail,
    .oam_autoDyingGaspEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .oam_autoDyingGaspEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .oam_dyingGaspTLV_get = (int32 (*)(uint32 , rtk_port_t , rtk_oam_dyingGaspTLV_t *))dal_common_unavail,
    .oam_dyingGaspTLV_set = (int32 (*)(uint32 , rtk_port_t , rtk_oam_dyingGaspTLV_t *))dal_common_unavail,
    .oam_dyingGaspWaitTime_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .oam_dyingGaspWaitTime_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .oam_cfmEntry_get = (int32 (*)(uint32 , uint32 , rtk_oam_cfm_t *))dal_common_unavail,
    .oam_cfmEntry_set = (int32 (*)(uint32 , uint32 , rtk_oam_cfm_t *))dal_common_unavail,
    .oam_cfmPortEntry_get = (int32 (*)(uint32 , rtk_port_t , rtk_oam_cfmPort_t *))dal_common_unavail,
    .oam_cfmPortEntry_set = (int32 (*)(uint32 , rtk_port_t , rtk_oam_cfmPort_t *))dal_common_unavail,
    .oam_cfmMepEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .oam_cfmMepEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .oam_txCCMFrame_start = (int32 (*)(uint32 , rtk_portmask_t *))dal_common_unavail,
    .oam_txCCMFrame_stop = (int32 (*)(uint32 ))dal_common_unavail,
    .oam_cfmCCMFrame_get = (int32 (*)(uint32 , uint32 , rtk_oam_ccmFrame_t *))dal_common_unavail,
    .oam_cfmCCMFrame_set = (int32 (*)(uint32 , uint32 , rtk_oam_ccmFrame_t *))dal_common_unavail,
    .oam_cfmCCMSnapOui_get = (int32 (*)(uint32 , rtk_snapOui_t *))dal_common_unavail,
    .oam_cfmCCMSnapOui_set = (int32 (*)(uint32 , rtk_snapOui_t *))dal_common_unavail,
    .oam_cfmCCMEtype_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .oam_cfmCCMEtype_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .oam_cfmCCMOpcode_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .oam_cfmCCMOpcode_set = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .oam_cfmCCMFlag_get = (int32 (*)(uint32 , uint32 , uint32 *))dal_common_unavail,
    .oam_cfmCCMFlag_set = (int32 (*)(uint32 , uint32 , uint32 ))dal_common_unavail,
    .oam_cfmCCMInterval_get = (int32 (*)(uint32 , rtk_oam_ccmInterval_t *))dal_common_unavail,
    .oam_cfmCCMInterval_set = (int32 (*)(uint32 , rtk_oam_ccmInterval_t ))dal_common_unavail,
    .oam_cfmIntfStatus_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .oam_cfmIntfStatus_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .oam_cfmPortStatus_get = (int32 (*)(uint32 , rtk_port_t , uint32 *))dal_common_unavail,
    .oam_cfmPortStatus_set = (int32 (*)(uint32 , rtk_port_t , uint32 ))dal_common_unavail,
    .oam_cfmRemoteMep_del = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .oam_cfmRemoteMep_add = (int32 (*)(uint32 , uint32 ))dal_common_unavail,
    .oam_cfmCCStatus_get = (int32 (*)(uint32 , rtk_port_t , uint32 , uint32 *, rtk_oam_ccStatus_t *))dal_common_unavail,
    .oam_cfmCCStatus_reset = (int32 (*)(uint32 , rtk_port_t ))dal_common_unavail,
    .oam_cfmLoopbackReplyEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .oam_cfmLoopbackReplyEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .oam_cfmLoopbackReplyCtrl_get = (int32 (*)(uint32 , rtk_oam_cfmLoopbackCtrl_t *))dal_common_unavail,
    .oam_cfmLoopbackReplyCtrl_set = (int32 (*)(uint32 , rtk_oam_cfmLoopbackCtrl_t *))dal_common_unavail,
    
    /* PIE function */
    .pie_init = (int32 (*)(uint32 ))dal_common_unavail,
    .pie_pieRuleEntryFieldSize_get = (int32 (*)(uint32 , rtk_pie_fieldType_t , uint32 *))dal_common_unavail,
    .pie_pieRuleEntrySize_get = (int32 (*)(uint32 , uint32 *))dal_common_unavail,
    .pie_pieRuleEntryField_get = (int32 (*)(uint32 , rtk_pie_phase_t , rtk_pie_id_t , uint8 *, rtk_pie_fieldType_t , uint8 *, uint8 *))dal_common_unavail,
    .pie_pieRuleEntryField_set = (int32 (*)(uint32 , rtk_pie_phase_t , rtk_pie_id_t , uint8 *, rtk_pie_fieldType_t , uint8 *, uint8 *))dal_common_unavail,
    .pie_pieRuleEntryField_read = (int32 (*)(uint32 , rtk_pie_phase_t , rtk_pie_id_t , rtk_pie_fieldType_t , uint8 *, uint8 *))dal_common_unavail,
    .pie_pieRuleEntryField_write = (int32 (*)(uint32 , rtk_pie_phase_t , rtk_pie_id_t , rtk_pie_fieldType_t , uint8 *, uint8 *))dal_common_unavail,
    .pie_piePreDefinedRuleEntry_get = (int32 (*)(uint32 , uint8 * , rtk_pie_preDefinedRuleEntry_t *))dal_common_unavail,
    .pie_piePreDefinedRuleEntry_set = (int32 (*)(uint32 , uint8 * , rtk_pie_preDefinedRuleEntry_t *))dal_common_unavail,
    .pie_pieRuleEntry_read = (int32 (*)(uint32 , rtk_pie_id_t , uint8 *))dal_common_unavail,
    .pie_pieRuleEntry_write = (int32 (*)(uint32 , rtk_pie_id_t , uint8 *))dal_common_unavail,
    .pie_pieRuleEntry_del = (int32 (*)(uint32 , rtk_pie_clearBlockContent_t *))dal_common_unavail,
    .pie_pieRuleEntry_move = (int32 (*)(uint32 , rtk_pie_movePieContent_t *))dal_common_unavail,
    .pie_pieRuleEntry_swap = (int32 (*)(uint32 , rtk_pie_movePieContent_t *))dal_common_unavail,
    .pie_pieRuleAction_get = (int32 (*)(uint32 , rtk_pie_id_t , rtk_pie_actionTable_t *))dal_common_unavail,
    .pie_pieRuleAction_set = (int32 (*)(uint32 , rtk_pie_id_t , rtk_pie_actionTable_t *))dal_common_unavail,
    .pie_pieRuleAction_del = (int32 (*)(uint32 , rtk_pie_clearBlockContent_t *))dal_common_unavail,
    .pie_pieRuleAction_move = (int32 (*)(uint32 , rtk_pie_movePieContent_t *))dal_common_unavail,
    .pie_pieRuleAction_swap = (int32 (*)(uint32 , rtk_pie_movePieContent_t *))dal_common_unavail,
    .pie_pieRulePolicer_get = (int32 (*)(uint32 , rtk_pie_id_t , rtk_pie_policerEntry_t *))dal_common_unavail,
    .pie_pieRulePolicer_set = (int32 (*)(uint32 , rtk_pie_id_t , rtk_pie_policerEntry_t *))dal_common_unavail,
    .pie_pieHitIndication_get = (int32 (*)(uint32 , uint32 , rtk_pie_hitIndicationEntry_t *))dal_common_unavail,
    .pie_pieStat_get = (int32 (*)(uint32 , uint32 , uint32 *, uint64 *))dal_common_unavail,
    .pie_pieStat_set = (int32 (*)(uint32 , uint32 , uint32 , uint64 ))dal_common_unavail,
    .pie_pieStat_clearAll = (int32 (*)(uint32 ))dal_common_unavail,
    .pie_pieTemplateSelector_get = (int32 (*)(uint32 , uint32 , rtk_pie_phase_t , uint32 *))dal_common_unavail,
    .pie_pieTemplateSelector_set = (int32 (*)(uint32 , uint32 , rtk_pie_phase_t , uint32 ))dal_common_unavail,
    .pie_pieUserTemplate_get = (int32 (*)(uint32 , uint32 , rtk_pie_template_t *))dal_common_unavail,
    .pie_pieUserTemplate_set = (int32 (*)(uint32 , uint32 , rtk_pie_template_t *))dal_common_unavail,
    .pie_pieL34ChecksumErr_get = (int32 (*)(uint32 , rtk_pie_l34ChecksumErrOper_t *))dal_common_unavail,
    .pie_pieL34ChecksumErr_set = (int32 (*)(uint32 , rtk_pie_l34ChecksumErrOper_t ))dal_common_unavail,
    .pie_pieUserTemplatePayloadOffset_get = (int32 (*)(uint32 , uint32 , uint32 , uint32 *))dal_common_unavail,
    .pie_pieUserTemplatePayloadOffset_set = (int32 (*)(uint32 , uint32 , uint32 , uint32 ))dal_common_unavail,
    .pie_pieResultReverse_get = (int32 (*)(uint32 , uint32 , rtk_pie_resultReverseOper_t *))dal_common_unavail,
    .pie_pieResultReverse_set = (int32 (*)(uint32 , uint32 , rtk_pie_resultReverseOper_t ))dal_common_unavail,
    .pie_pieResultAggregator_get = (int32 (*)(uint32 , rtk_pie_resultAggregatorRange_t , uint32 , rtk_pie_resultAggregatorType_t *))dal_common_unavail,
    .pie_pieResultAggregator_set = (int32 (*)(uint32 , rtk_pie_resultAggregatorRange_t , uint32 , rtk_pie_resultAggregatorType_t ))dal_common_unavail,
    .pie_pieBlockPriority_get = (int32 (*)(uint32 , uint32 , uint32 *))dal_common_unavail,
    .pie_pieBlockPriority_set = (int32 (*)(uint32 , uint32 , uint32 ))dal_common_unavail,
    .pie_pieGroupCtrl_get = (int32 (*)(uint32 , rtk_pie_groupCtrlRange_t , rtk_pie_groupCtrl_t *))dal_common_unavail,
    .pie_pieGroupCtrl_set = (int32 (*)(uint32 , rtk_pie_groupCtrlRange_t , rtk_pie_groupCtrl_t ))dal_common_unavail,
    .pie_pieEgrAclLookupCtrl_get = (int32 (*)(uint32 , rtk_pie_egrAclLookupCtrl_t *))dal_common_unavail,
    .pie_pieEgrAclLookupCtrl_set = (int32 (*)(uint32 , rtk_pie_egrAclLookupCtrl_t *))dal_common_unavail,
    .pie_piePortLookupPhaseEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_pie_phase_t , rtk_enable_t *))dal_common_unavail,
    .pie_piePortLookupPhaseEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_pie_phase_t , rtk_enable_t ))dal_common_unavail,
    .pie_piePortLookupPhaseMiss_get = (int32 (*)(uint32 , rtk_port_t , rtk_pie_phase_t , rtk_pie_lookupMissAction_t *))dal_common_unavail,
    .pie_piePortLookupPhaseMiss_set = (int32 (*)(uint32 unit, rtk_port_t , rtk_pie_phase_t , rtk_pie_lookupMissAction_t ))dal_common_unavail,
    .pie_pieCounterIndicationMode_get = (int32 (*)(uint32 , uint32 , rtk_pie_counterIndicationMode_t *))dal_common_unavail,
    .pie_pieCounterIndicationMode_set = (int32 (*)(uint32 , uint32 , rtk_pie_counterIndicationMode_t s))dal_common_unavail,
    .pie_piePolicerCtrl_get = (int32 (*)(uint32 , rtk_pie_policerCtrl_t *))dal_common_unavail,
    .pie_piePolicerCtrl_set = (int32 (*)(uint32 , rtk_pie_policerCtrl_t *))dal_common_unavail,
    .pie_rangeCheckL4Port_get = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_l4Port_t *))dal_common_unavail,
    .pie_rangeCheckL4Port_set = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_l4Port_t *))dal_common_unavail,
    .pie_rangeCheckVid_get = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_vid_t *))dal_common_unavail,
    .pie_rangeCheckVid_set = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_vid_t *))dal_common_unavail,
    .pie_rangeCheckIp_get = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_ip_t *))dal_common_unavail,
    .pie_rangeCheckIp_set = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_ip_t *))dal_common_unavail,
    .pie_rangeCheckSrcPort_get = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_srcPortMask_t *))dal_common_unavail,
    .pie_rangeCheckSrcPort_set = (int32 (*)(uint32 , uint32 , rtk_pie_rangeCheck_srcPortMask_t *))dal_common_unavail,
    .pie_fieldSelectorEnable_get = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_enable_t *))dal_common_unavail,
    .pie_fieldSelectorEnable_set = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_enable_t ))dal_common_unavail,
    .pie_fieldSelectorContent_get = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_pie_fieldSelector_data_t *))dal_common_unavail,
    .pie_fieldSelectorContent_set = (int32 (*)(uint32 , rtk_port_t , uint32 , rtk_pie_fieldSelector_data_t *))dal_common_unavail,
    .pie_patternMatchEnable_get = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t *))dal_common_unavail,
    .pie_patternMatchEnable_set = (int32 (*)(uint32 , rtk_port_t , rtk_enable_t ))dal_common_unavail,
    .pie_patternMatchContent_get = (int32 (*)(uint32 , rtk_port_t , rtk_pie_patternMatch_content_t *))dal_common_unavail,
    .pie_patternMatchContent_set = (int32 (*)(uint32 , rtk_port_t , rtk_pie_patternMatch_content_t *))dal_common_unavail,

    /* LED function */
    .led_init = dal_ssw_led_init,
    .led_portEnable_get = dal_ssw_led_portEnable_get,
    .led_portEnable_set = dal_ssw_led_portEnable_set,
    .led_sysEnable_get = dal_ssw_led_sysEnable_get,
    .led_sysEnable_set = dal_ssw_led_sysEnable_set,

    /* Time function */
    .time_init = dal_ssw_time_init,
    .time_portPtpEnable_get = dal_ssw_time_portPtpEnable_get,
    .time_portPtpEnable_set = dal_ssw_time_portPtpEnable_set,
    .time_portPtpRxTimestamp_get = dal_ssw_time_portPtpRxTimestamp_get,
    .time_portPtpTxTimestamp_get = (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))dal_common_unavail,
    .time_portPtpTxTimestampCallback_register = dal_ssw_time_portPtpTxTimestampCallback_register,
    .time_refTime_get = dal_ssw_time_refTime_get,
    .time_refTime_set = dal_ssw_time_refTime_set,
    .time_refTimeAdjust_set = dal_ssw_time_refTimeAdjust_set,
    .time_refTimeEnable_get = dal_ssw_time_refTimeEnable_get,
    .time_refTimeEnable_set = dal_ssw_time_refTimeEnable_set,

    /* Diagnostic */
    .diag_init              = dal_ssw_diag_init,
    .diag_portMacRemoteLoopbackEnable_get = dal_ssw_diag_portMacRemoteLoopbackEnable_get,
    .diag_portMacRemoteLoopbackEnable_set = dal_ssw_diag_portMacRemoteLoopbackEnable_set,
    .diag_portMacLocalLoopbackEnable_get = dal_ssw_diag_portMacLocalLoopbackEnable_get,
    .diag_portMacLocalLoopbackEnable_set = dal_ssw_diag_portMacLocalLoopbackEnable_set,
    .diag_portRtctResult_get = dal_ssw_diag_portRtctResult_get,
    .diag_rtct_start = dal_ssw_diag_portRtct_start,
};

/*
 * Macro Declaration
 */ 

/*
 * Function Declaration
 */


/* Module Name    :  */

/* Function Name: 
 *      dal_ssw_init
 * Description: 
 *      Initilize DAL of smart switch 
 * Input:  
 *      unit - unit id
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK     - initialize success
 *      RT_ERR_FAILED - initialize fail
 * Note: 
 *      RTK must call this function before do other kind of action.
 */ 
int dal_ssw_init(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Same initialize sequence as original rtk_init function */
    if ((ret = dal_ssw_switch_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_switch_init Failed!");
        return ret;
    }
    
    if ((ret = dal_ssw_port_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_port_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_led_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_led_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_eee_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_eee_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_trunk_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_trunk_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_filter_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_filter_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_qos_init(unit, RTK_DEFAULT_QOS_QUEUE_NUMBER)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_qos_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_rate_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_rate_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_flowctrl_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_flowctrl_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_vlan_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_vlan_init Failed!");
        return ret;
    }


    if ((ret = dal_ssw_svlan_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_svlan_init Failed!");
        return ret;
    }


    if ((ret = dal_ssw_stp_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_stp_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_l2_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_l2_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_dot1x_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_dot1x_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_trap_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_rate_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_mirror_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_mirror_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_stat_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_stat_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_time_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_time_init Failed!");
        return ret;
    }

    if ((ret = dal_ssw_diag_init(unit)) != RT_ERR_OK)
    {
        RT_DBG(LOG_MAJOR_ERR, MOD_DAL, "dal_ssw_diag_init Failed!");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_ssw_init */
