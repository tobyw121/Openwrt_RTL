/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 31282 $
 * $Date: 2012-07-23 16:36:21 +0800 (Mon, 23 Jul 2012) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) trap
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/trap.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_trap_rmaAction_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    GETSOCKOPT(RTDRV_TRAP_RMAACTION_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pRma_action = trap_cfg.rma_action;

    return RT_ERR_OK;
} /* end of rtk_trap_rmaAction_get */


int32 rtk_trap_rmaAction_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    trap_cfg.rma_action = rma_action;
    SETSOCKOPT(RTDRV_TRAP_RMAACTION_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_rmaAction_set */


int32 rtk_trap_1xMacChangePort2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_1XMACCHANGEPORT2CPUENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_1xMacChangePort2CpuEnable_get */


int32 rtk_trap_1xMacChangePort2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_1XMACCHANGEPORT2CPUENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_1xMacChangePort2CpuEnable_set */


int32 rtk_trap_igmpCtrlPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_IGMPCTRLPKT2CPUENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_igmpCtrlPkt2CpuEnable_get */


int32 rtk_trap_igmpCtrlPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_IGMPCTRLPKT2CPUENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_igmpCtrlPkt2CpuEnable_set */


int32 rtk_trap_l2McastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_L2MCASTPKT2CPUENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_l2McastPkt2CpuEnable_get */


int32 rtk_trap_l2McastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_L2MCASTPKT2CPUENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
}  /* end of rtk_trap_l2McastPkt2CpuEnable_set */


int32 rtk_trap_ipMcastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_IPMCASTPKT2CPUENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_ipMcastPkt2CpuEnable_get */


int32 rtk_trap_ipMcastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_IPMCASTPKT2CPUENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_ipMcastPkt2CpuEnable_set */


int32 rtk_trap_reasonTrapToCPUPriority_get(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t *pPriority)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.reason = type;
    GETSOCKOPT(RTDRV_TRAP_REASONTRAPTOCPUPRIORITY_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_reasonTrapToCPUPriority_get */


int32 rtk_trap_reasonTrapToCPUPriority_set(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t priority)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.reason = type;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_REASONTRAPTOCPUPRIORITY_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_reasonTrapToCPUPriority_set */


int32 rtk_trap_pkt2CpuEnable_get(uint32 unit, rtk_trap_type_t type, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.pkt_type = type;
    GETSOCKOPT(RTDRV_TRAP_PKT2CPUENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_pkt2CpuEnable_get */


int32 rtk_trap_pkt2CpuEnable_set(uint32 unit, rtk_trap_type_t type, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.pkt_type = type;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_PKT2CPUENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_pkt2CpuEnable_set */


int32 rtk_trap_rmaPri_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t *pPriority)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    GETSOCKOPT(RTDRV_TRAP_RMAPRI_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_rmaPri_get */


int32 rtk_trap_rmaPri_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t priority)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_RMAPRI_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_rmaPri_set */


int32 rtk_trap_rmaPriEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    GETSOCKOPT(RTDRV_TRAP_RMAPRIENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_rmaPriEnable_get */


int32 rtk_trap_rmaPriEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_RMAPRIENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_rmaPriEnable_set */


int32 rtk_trap_rmaCpuTagAddEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_RMACPUTAGADDENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_rmaCpuTagAddEnable_get */


int32 rtk_trap_rmaCpuTagAddEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_RMACPUTAGADDENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_rmaCpuTagAddEnable_set */


int32 rtk_trap_rmaVlanCheckEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    GETSOCKOPT(RTDRV_TRAP_RMAVLANCHECKENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_rmaVlanCheckEnable_get */


int32 rtk_trap_rmaVlanCheckEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    memcpy(&trap_cfg.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_RMAVLANCHECKENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_rmaVlanCheckEnable_set */


int32 rtk_trap_bypassStp_get(uint32 unit, rtk_trap_bypassStpType_t frameType, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.bypassStp_frame = frameType;
    GETSOCKOPT(RTDRV_TRAP_BYPASS_STP_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
}

int32 rtk_trap_bypassStp_set(uint32 unit, rtk_trap_bypassStpType_t frameType, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.bypassStp_frame = frameType;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_BYPASS_STP_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_trap_bypassVlan_get(uint32 unit, rtk_trap_bypassVlanType_t frameType, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.bypassVlan_frame = frameType;
    GETSOCKOPT(RTDRV_TRAP_BYPASS_VLAN_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
}

int32 rtk_trap_bypassVlan_set(uint32 unit, rtk_trap_bypassVlanType_t frameType, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.bypassVlan_frame = frameType;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_BYPASS_VLAN_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_trap_userDefineRma_get(
    uint32                      unit,
    uint32                      userDefine_idx,
    rtk_trap_userDefinedRma_t   *pUserDefinedRma)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMA_GET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);
    memcpy(pUserDefinedRma, &trap_cfg.rma_frame, sizeof(rtk_trap_userDefinedRma_t));

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRma_get */


int32 rtk_trap_userDefineRma_set(
    uint32                      unit,
    uint32                      userDefine_idx,
    rtk_trap_userDefinedRma_t   *pUserDefinedRma)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    memcpy(&trap_cfg.rma_frame, pUserDefinedRma, sizeof(rtk_trap_userDefinedRma_t));
    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMA_SET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);

    return RT_ERR_OK;
}  /* end of rtk_trap_userDefineRma_set */


int32 rtk_trap_userDefineRmaEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMAENABLE_GET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
}

int32 rtk_trap_userDefineRmaEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMAENABLE_SET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_trap_userDefineRmaAction_get(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t *pAction)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMAACTION_GET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);
    *pAction = trap_cfg.rma_action;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaAction_get */


int32 rtk_trap_userDefineRmaAction_set(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t action)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    trap_cfg.rma_action = action;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMAACTION_SET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaAction_set */


int32 rtk_trap_userDefineRmaPri_get(uint32 unit, uint32 userDefine_idx, rtk_pri_t *pPriority)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMAPRI_GET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaPri_get */


int32 rtk_trap_userDefineRmaPri_set(uint32 unit, uint32 userDefine_idx, rtk_pri_t priority)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMAPRI_SET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaPri_set */


int32 rtk_trap_userDefineRmaPriEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMAPRIENABLE_GET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaPriEnable_get */


int32 rtk_trap_userDefineRmaPriEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMAPRIENABLE_SET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaPriEnable_set */


int32 rtk_trap_userDefineRmaVlanCheckEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMAVLANCHECKENABLE_GET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);
    *pEnable = trap_cfg.vlanCheck;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaVlanCheckEnable_get */


int32 rtk_trap_userDefineRmaVlanCheckEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    trap_cfg.vlanCheck = enable;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMAVLANCHECKENABLE_SET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaVlanCheckEnable_set */


int32 rtk_trap_userDefineRmaStpBlockEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMASTPBLOCKENABLE_GET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);
    *pEnable = trap_cfg.stpBlock;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaStpBlockEnable_get */


int32 rtk_trap_userDefineRmaStpBlockEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    rtdrv_trapL2userRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.rma_index = userDefine_idx;
     trap_cfg.stpBlock = enable;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMASTPBLOCKENABLE_SET, &trap_cfg, rtdrv_trapL2userRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineRmaStpBlockEnable_set */


int32 rtk_trap_mgmtFrameAction_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_MGMTFRAMEACTION_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pAction = trap_cfg.rma_action;

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFrameAction_get */


int32 rtk_trap_mgmtFrameAction_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t action)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    trap_cfg.rma_action = action;
    SETSOCKOPT(RTDRV_TRAP_MGMTFRAMEACTION_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFrameAction_set */


int32 rtk_trap_mgmtFramePri_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_MGMTFRAMEPRI_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFramePri_get */


int32 rtk_trap_mgmtFramePri_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t priority)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_MGMTFRAMEPRI_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFramePri_set */


int32 rtk_trap_mgmtFramePriEnable_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_MGMTFRAMEPRIENABLE_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFramePriEnable_get */


int32 rtk_trap_mgmtFramePriEnable_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_MGMTFRAMEPRIENABLE_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFramePriEnable_set */


int32 rtk_trap_mgmtFrameVlanCheck_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_MGMTFRAMEVLANCHECK_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pEnable = trap_cfg.vlanCheck;

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFrameVlanCheck_get */


int32 rtk_trap_mgmtFrameVlanCheck_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.frameType = frameType;
    trap_cfg.vlanCheck = enable;
    SETSOCKOPT(RTDRV_TRAP_MGMTFRAMEVLANCHECK_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFrameVlanCheck_set */


int32 rtk_trap_userDefineMgmt_get(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMT_GET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);
    memcpy(pUserDefine, &trap_cfg.userDefine, sizeof(rtk_trap_userDefinedMgmt_t));

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmt_get */


int32 rtk_trap_userDefineMgmt_set(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    memcpy(&trap_cfg.userDefine, pUserDefine, sizeof(rtk_trap_userDefinedMgmt_t));
    SETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMT_SET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmt_set */


int32 rtk_trap_userDefineMgmtAction_get(uint32 unit, uint32 mgmt_idx, rtk_action_t *pAction)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTACTION_GET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);
    *pAction = trap_cfg.rma_action;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtAction_get */


int32 rtk_trap_userDefineMgmtAction_set(uint32 unit, uint32 mgmt_idx, rtk_action_t action)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    trap_cfg.rma_action = action;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTACTION_SET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtAction_set */


int32 rtk_trap_userDefineMgmtPri_get(uint32 unit, uint32 mgmt_idx, rtk_pri_t *pPriority)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTPRI_GET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtPri_get */


int32 rtk_trap_userDefineMgmtPri_set(uint32 unit, uint32 mgmt_idx, rtk_pri_t priority)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTPRI_SET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtPri_set */


int32 rtk_trap_userDefineMgmtPriEnable_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTPRIENABLE_GET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtPriEnable_get */


int32 rtk_trap_userDefineMgmtPriEnable_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTPRIENABLE_SET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtPriEnable_set */


int32 rtk_trap_userDefineMgmtVlanCheck_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    GETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTVLANCHECK_GET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);
    *pEnable = trap_cfg.vlanCheck;

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtVlanCheck_get */


int32 rtk_trap_userDefineMgmtVlanCheck_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable)
{
    rtdrv_trapUserMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.mgmt_idx = mgmt_idx;
    trap_cfg.vlanCheck = enable;
    SETSOCKOPT(RTDRV_TRAP_USERDEFINEMGMTVLANCHECK_SET, &trap_cfg, rtdrv_trapUserMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_userDefineMgmtVlanCheck_set */


int32 rtk_trap_portMgmtFrameAction_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEACTION_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pAction = trap_cfg.rma_action;

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFrameAction_get */


int32 rtk_trap_portMgmtFrameAction_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t action)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    trap_cfg.rma_action = action;
    SETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEACTION_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFrameAction_set */


int32 rtk_trap_portMgmtFramePri_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEPRI_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFramePri_get */


int32 rtk_trap_portMgmtFramePri_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t priority)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEPRI_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFramePri_set */


int32 rtk_trap_portMgmtFramePriEnable_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEPRIENABLE_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFramePriEnable_get */


int32 rtk_trap_portMgmtFramePriEnable_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEPRIENABLE_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFramePriEnable_set */


int32 rtk_trap_portMgmtFrameVlanCheck_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEVLANCHECK_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pEnable = trap_cfg.vlanCheck;

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFrameVlanCheck_get */


int32 rtk_trap_portMgmtFrameVlanCheck_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    trap_cfg.vlanCheck = enable;
    SETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMEVLANCHECK_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFrameVlanCheck_set */


int32 rtk_trap_portMgmtFrameCrossVlan_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMECROSSVLAN_GET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);
    *pEnable = trap_cfg.vlanCross;

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFrameCrossVlan_get */


int32 rtk_trap_portMgmtFrameCrossVlan_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    rtdrv_trapMgmRmaCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.frameType = frameType;
    trap_cfg.vlanCross = enable;
    SETSOCKOPT(RTDRV_TRAP_PORTMGMTFRAMECROSSVLAN_SET, &trap_cfg, rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portMgmtFrameCrossVlan_set */

int32 rtk_trap_ipWithOptionHeaderAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_ip_family_t     ipFamily,
    rtk_action_t        *pAction)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.ipFamily = ipFamily;
    GETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERACTION_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pAction = trap_cfg.action;

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderAction_get */


int32 rtk_trap_ipWithOptionHeaderAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_ip_family_t     ipFamily,
    rtk_action_t        action)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.ipFamily = ipFamily;
    trap_cfg.action = action;
    SETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERACTION_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderAction_set */


int32 rtk_trap_ipWithOptionHeaderPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERPRI_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderPri_get */


int32 rtk_trap_ipWithOptionHeaderPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERPRI_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderPri_set */


int32 rtk_trap_ipWithOptionHeaderPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERPRIENABLE_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderPriEnable_get */


int32 rtk_trap_ipWithOptionHeaderPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERPRIENABLE_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderPriEnable_set */


int32 rtk_trap_ipWithOptionHeaderAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERADDCPUTAGENABLE_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pEnable = trap_cfg.cputag;

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderAddCPUTagEnable_get */


int32 rtk_trap_ipWithOptionHeaderAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.cputag = enable;
    SETSOCKOPT(RTDRV_TRAP_IPWITHOPTIONHEADERADDCPUTAGENABLE_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_ipWithOptionHeaderAddCPUTagEnable_set */

int32 rtk_trap_pktWithCFIAction_get(uint32 unit, rtk_action_t *pAction)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_PKTWITHCFIACTION_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pAction = trap_cfg.action;

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIAction_get */


int32 rtk_trap_pktWithCFIAction_set(uint32 unit, rtk_action_t action)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.action = action;
    SETSOCKOPT(RTDRV_TRAP_PKTWITHCFIACTION_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIAction_set */


int32 rtk_trap_pktWithOuterCFIAction_get(uint32 unit, rtk_action_t *pAction)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_PKTWITHOUTERCFIACTION_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pAction = trap_cfg.action;

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithOuterCFIAction_get */


int32 rtk_trap_pktWithOuterCFIAction_set(uint32 unit, rtk_action_t action)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.action = action;
    SETSOCKOPT(RTDRV_TRAP_PKTWITHOUTERCFIACTION_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithOuterCFIAction_set */

int32 rtk_trap_portPktWithCFIAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_PORTPKTWITHCFIACTION_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pAction = trap_cfg.action;

    return RT_ERR_OK;
} /* end of rtk_trap_portPktWithCFIAction_get */


int32 rtk_trap_portPktWithCFIAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.action = action;
    SETSOCKOPT(RTDRV_TRAP_PORTPKTWITHCFIACTION_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portPktWithCFIAction_set */

int32 rtk_trap_pktWithCFIPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_PKTWITHCFIPRI_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIPri_get */


int32 rtk_trap_pktWithCFIPri_set(uint32 unit, rtk_pri_t priority)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_PKTWITHCFIPRI_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIPri_set */

int32 rtk_trap_portPktWithCFIPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_PORTPKTWITHCFIPRI_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_portPktWithCFIPri_get */


int32 rtk_trap_portPktWithCFIPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_PORTPKTWITHCFIPRI_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portPktWithCFIPri_set */

int32 rtk_trap_pktWithCFIPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_PKTWITHCFIPRIENABLE_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIPriEnable_get */


int32 rtk_trap_pktWithCFIPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_PKTWITHCFIPRIENABLE_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIPriEnable_set */


int32 rtk_trap_pktWithCFIAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_PKTWITHCFIADDCPUTAGENABLE_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pEnable = trap_cfg.cputag;

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIAddCPUTagEnable_get */


int32 rtk_trap_pktWithCFIAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_trapOtherCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.cputag = enable;
    SETSOCKOPT(RTDRV_TRAP_PKTWITHCFIADDCPUTAGENABLE_SET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_pktWithCFIAddCPUTagEnable_set */

int32 rtk_trap_cfmFrameAction_get(uint32 unit, uint32 level, rtk_action_t *pAction)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.md_level = level;
    GETSOCKOPT(RTDRV_TRAP_CFMFRAMEACTION_GET, &trap_cfg, rtdrv_trapOtherCfg_t, 1);
    *pAction = trap_cfg.action;

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameAction_get */


int32 rtk_trap_cfmFrameAction_set(uint32 unit, uint32 level, rtk_action_t action)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.md_level = level;
    trap_cfg.action = action;
    SETSOCKOPT(RTDRV_TRAP_CFMFRAMEACTION_SET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameAction_set */


int32 rtk_trap_cfmFrameTrapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_CFMFRAMETRAPPRI_GET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameTrapPri_get */


int32 rtk_trap_cfmFrameTrapPri_set(uint32 unit, rtk_pri_t priority)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_CFMFRAMETRAPPRI_SET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameTrapPri_set */


int32 rtk_trap_cfmFrameTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_CFMFRAMETRAPPRIENABLE_GET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameTrapPriEnable_get */


int32 rtk_trap_cfmFrameTrapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_CFMFRAMETRAPPRIENABLE_SET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameTrapPriEnable_set */


int32 rtk_trap_cfmFrameTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_CFMFRAMETRAPADDCPUTAGENABLE_GET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);
    *pEnable = trap_cfg.cputag;

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameTrapAddCPUTagEnable_get */


int32 rtk_trap_cfmFrameTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.cputag = enable;
    SETSOCKOPT(RTDRV_TRAP_CFMFRAMETRAPADDCPUTAGENABLE_SET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_cfmFrameTrapAddCPUTagEnable_set */


int32 rtk_trap_portOamPDUAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_PORTOAMPDUACTION_GET, &trap_cfg, rtdrv_trapPortCfg_t, 1);
    *pAction = trap_cfg.action;

    return RT_ERR_OK;
} /* end of rtk_trap_portOamPDUAction_get */

int32 rtk_trap_oamPDUAction_get(uint32 unit, rtk_action_t *pAction)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_OAMPDUACTION_GET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);
    *pAction = trap_cfg.action;

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUAction_get */

int32 rtk_trap_portOamPDUAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.action = action;
    SETSOCKOPT(RTDRV_TRAP_PORTOAMPDUACTION_SET, &trap_cfg, rtdrv_trapPortCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portOamPDUAction_set */

int32 rtk_trap_oamPDUAction_set(uint32 unit, rtk_action_t action)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.action = action;
    SETSOCKOPT(RTDRV_TRAP_OAMPDUACTION_SET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUAction_set */

int32 rtk_trap_portOamPDUPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_PORTOAMPDUPRI_GET, &trap_cfg, rtdrv_trapPortCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_portOamPDUPri_get */

int32 rtk_trap_oamPDUPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_OAMPDUPRI_GET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);
    *pPriority = trap_cfg.priority;

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUPri_get */

int32 rtk_trap_portOamPDUPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_PORTOAMPDUPRI_SET, &trap_cfg, rtdrv_trapPortCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_portOamPDUPri_set */

int32 rtk_trap_oamPDUPri_set(uint32 unit, rtk_pri_t priority)
{
    rtdrv_trapCfmCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.priority = priority;
    SETSOCKOPT(RTDRV_TRAP_OAMPDUPRI_SET, &trap_cfg, rtdrv_trapCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUPri_set */


int32 rtk_trap_oamPDUPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_OAMPDUPRIENABLE_GET, &trap_cfg, rtdrv_trapPortCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUPriEnable_get */


int32 rtk_trap_oamPDUPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_OAMPDUPRIENABLE_SET, &trap_cfg, rtdrv_trapPortCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUPriEnable_set */


int32 rtk_trap_oamPDUTrapAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    GETSOCKOPT(RTDRV_TRAP_OAMPDUTRAPADDCPUTAGENABLE_GET, &trap_cfg, rtdrv_trapPortCfg_t, 1);
    *pEnable = trap_cfg.cputag;

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUTrapAddCPUTagEnable_get */


int32 rtk_trap_oamPDUTrapAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)

{
    rtdrv_trapPortCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.port = port;
    trap_cfg.cputag = enable;
    SETSOCKOPT(RTDRV_TRAP_OAMPDUTRAPADDCPUTAGENABLE_SET, &trap_cfg, rtdrv_trapPortCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_oamPDUTrapAddCPUTagEnable_set */

int32 rtk_trap_mgmtIpCheck_get(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.ip_type = type;
    GETSOCKOPT(RTDRV_TRAP_MGMTIPCHECK_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    *pEnable = trap_cfg.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtIpCheck_get */

int32 rtk_trap_mgmtIpCheck_set(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t enable)
{
    rtdrv_trapCfg_t  trap_cfg;

    trap_cfg.unit = unit;
    trap_cfg.ip_type = type;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_MGMTIPCHECK_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtIpCheck_set */

int32
rtk_trap_cfmUnknownFrameAct_get(uint32 unit, rtk_action_t *action)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_CFMUNKNOWNFRAMEACT_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);
    *action = config.action;

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmUnknownFrameAct_get */

int32
rtk_trap_cfmUnknownFrameAct_set(uint32 unit, rtk_action_t action)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.action = action;
    SETSOCKOPT(RTDRV_TRAP_CFMUNKNOWNFRAMEACT_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmUnknownFrameAct_set */

int32
rtk_trap_cfmLoopbackLinkTraceAct_get(uint32 unit, uint32 level,
                            rtk_action_t *action)
{
    rtdrv_trapCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapCfmCfg_t));

    config.unit     = unit;
    config.md_level = level;
    GETSOCKOPT(RTDRV_TRAP_CFMLOOPBACKACT_GET, &config, rtdrv_trapCfmCfg_t, 1);
    *action = config.action;

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmLoopbackLinkTraceAct_get */

int32
rtk_trap_cfmLoopbackLinkTraceAct_set(uint32 unit, uint32 level,
                            rtk_action_t action)
{
    rtdrv_trapCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapCfmCfg_t));

    config.unit     = unit;
    config.md_level = level;
    config.action   = action;
    SETSOCKOPT(RTDRV_TRAP_CFMLOOPBACKACT_SET, &config, rtdrv_trapCfmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmLoopbackLinkTraceAct_set */

int32
rtk_trap_cfmCcmAct_get(uint32 unit, uint32 level,
                       rtk_trap_oam_action_t *action)
{
    rtdrv_trapOamCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapOamCfg_t));

    config.unit     = unit;
    config.md_level = level;
    GETSOCKOPT(RTDRV_TRAP_CFMCCMACT_GET, &config, rtdrv_trapOamCfg_t, 1);
    *action = config.action;

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmCcmAct_get */

int32
rtk_trap_cfmCcmAct_set(uint32 unit, uint32 level,
                       rtk_trap_oam_action_t action)
{
    rtdrv_trapOamCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapOamCfg_t));

    config.unit     = unit;
    config.md_level = level;
    config.action   = action;
    SETSOCKOPT(RTDRV_TRAP_CFMCCMACT_SET, &config, rtdrv_trapOamCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmCcmAct_set */

int32
rtk_trap_cfmEthDmAct_get(uint32 unit, uint32 level,
                         rtk_action_t *action)
{
    rtdrv_trapCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapCfmCfg_t));

    config.unit     = unit;
    config.md_level = level;
    GETSOCKOPT(RTDRV_TRAP_CFMETHDMACT_GET, &config, rtdrv_trapCfmCfg_t, 1);
    *action = config.action;

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmEthDmAct_get */

int32
rtk_trap_cfmEthDmAct_set(uint32 unit, uint32 level,
                         rtk_action_t action)
{
    rtdrv_trapCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapCfmCfg_t));

    config.unit     = unit;
    config.md_level = level;
    config.action   = action;
    SETSOCKOPT(RTDRV_TRAP_CFMETHDMACT_SET, &config, rtdrv_trapOamCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_cfmEthDmAct_set */

int32 rtk_trap_portOamLoopbackParAction_get(uint32 unit,
        rtk_port_t port, rtk_trap_oam_action_t *pAction)
{
    rtdrv_trapOamCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapOamCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_TRAP_PORTOAMLOOPBACKPARACTION_GET, &config, rtdrv_trapOamCfg_t, 1);

    *pAction = config.action;

    return RT_ERR_OK;
}   /* end of rtk_trap_portOamLoopbackParAction_get */

int32 rtk_trap_portOamLoopbackParAction_set(uint32 unit,
        rtk_port_t port, rtk_trap_oam_action_t action)
{
    rtdrv_trapOamCfg_t config;

    memset(&config, 0, sizeof(rtdrv_trapOamCfg_t));

    config.unit = unit;
    config.port = port;
    config.action = action;
    SETSOCKOPT(RTDRV_TRAP_PORTOAMLOOPBACKPARACTION_SET, &config, rtdrv_trapOamCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_portOamLoopbackParAction_set */

int32
rtk_trap_routeExceptionAction_get(uint32 unit,
        rtk_trap_routeExceptionType_t type, rtk_action_t *pAction)
{
    rtdrv_trapRouteExceptionCfg_t config;

    config.unit = unit;
    config.type = type;

    GETSOCKOPT(RTDRV_TRAP_ROUTEEXCEPTIONACTION_GET, &config,
            rtdrv_trapRouteExceptionCfg_t, 1);

    *pAction = config.action;

    return RT_ERR_OK;
}    /* end of rtk_trap_routeExceptionAction_get */

int32
rtk_trap_routeExceptionAction_set(uint32 unit,
        rtk_trap_routeExceptionType_t type, rtk_action_t action)
{
    rtdrv_trapRouteExceptionCfg_t config;

    config.unit = unit;
    config.type = type;
    config.action = action;

    SETSOCKOPT(RTDRV_TRAP_ROUTEEXCEPTIONACTION_SET, &config,
            rtdrv_trapRouteExceptionCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_trap_routeExceptionAction_set */

int32
rtk_trap_routeExceptionPri_get(uint32 unit,
        rtk_trap_routeExceptionType_t type, rtk_pri_t *pPriority)
{
    rtdrv_trapRouteExceptionCfg_t config;

    config.unit = unit;
    config.type = type;

    GETSOCKOPT(RTDRV_TRAP_ROUTEEXCEPTIONPRI_GET, &config,
            rtdrv_trapRouteExceptionCfg_t, 1);

    *pPriority = config.priority;

    return RT_ERR_OK;
}    /* end of rtk_trap_routeExceptionPri_get */

int32
rtk_trap_routeExceptionPri_set(uint32 unit,
        rtk_trap_routeExceptionType_t type, rtk_pri_t priority)
{
    rtdrv_trapRouteExceptionCfg_t config;

    config.unit = unit;
    config.type = type;
    config.priority = priority;

    SETSOCKOPT(RTDRV_TRAP_ROUTEEXCEPTIONPRI_SET, &config,
            rtdrv_trapRouteExceptionCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_trap_routeExceptionPri_set */

int32
rtk_trap_userDefineRmaLearningEnable_get(uint32 unit, uint32 userDefine_idx,
        rtk_enable_t *pEnable)
{
    rtdrv_trapUserMgmRmaCfg_t config;

    config.unit = unit;
    config.mgmt_idx = userDefine_idx;

    GETSOCKOPT(RTDRV_TRAP_USERDEFINERMALEARNINGENABLE_GET, &config,
            rtdrv_trapUserMgmRmaCfg_t, 1);

    *pEnable = config.enable;

    return RT_ERR_OK;
}   /* end of rtk_trap_userDefineRmaLearningEnable_set */

int32
rtk_trap_userDefineRmaLearningEnable_set(uint32 unit, uint32 userDefine_idx,
        rtk_enable_t enable)
{
    rtdrv_trapUserMgmRmaCfg_t config;

    config.unit = unit;
    config.mgmt_idx = userDefine_idx;
    config.enable = enable;

    SETSOCKOPT(RTDRV_TRAP_USERDEFINERMALEARNINGENABLE_SET, &config,
            rtdrv_trapUserMgmRmaCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_userDefineRmaLearningEnable_set */

int32
rtk_trap_rmaLearningEnable_get(uint32 unit, rtk_mac_t *pRma_frame,
        rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t config;

    config.unit = unit;
    memcpy(&config.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    GETSOCKOPT(RTDRV_TRAP_RMALEARNINGENABLE_GET, &config,
        rtdrv_trapCfg_t, 1);

    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_rmaLearningEnable_set */

int32
rtk_trap_rmaLearningEnable_set(uint32 unit, rtk_mac_t *pRma_frame,
        rtk_enable_t enable)
{
    rtdrv_trapCfg_t config;

    config.unit = unit;
    memcpy(&config.rma_frame, pRma_frame, sizeof(rtk_mac_t));
    config.enable = enable;

    SETSOCKOPT(RTDRV_TRAP_RMALEARNINGENABLE_SET, &config,
        rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_trap_rmaLearningEnable_set */

int32
rtk_trap_mgmtFrameLearningEnable_get(uint32 unit, rtk_trap_mgmtType_t frameType,
        rtk_enable_t *pEnable)
{
    rtdrv_trapMgmRmaCfg_t config;

    config.unit = unit;
    config.frameType = frameType;
    GETSOCKOPT(RTDRV_TRAP_MGMTFRAMELEARNINGENABLE_GET, &config,
            rtdrv_trapMgmRmaCfg_t, 1);

    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_trap_mgmtFrameLearningEnable_get */

int32
rtk_trap_mgmtFrameLearningEnable_set(uint32 unit, rtk_trap_mgmtType_t frameType,
        rtk_enable_t enable)
{
    rtdrv_trapMgmRmaCfg_t config;

    config.unit = unit;
    config.frameType = frameType;
    config.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_MGMTFRAMELEARNINGENABLE_SET, &config,
            rtdrv_trapMgmRmaCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_mgmtFrameLearningEnable_set */

int32
rtk_trap_mgmtFrameMgmtVlanEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapOtherCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_MGMTFRAMEMGMTVLANENABLE_GET, &config,
            rtdrv_trapOtherCfg_t, 1);

    *pEnable = config.enable;

    return RT_ERR_OK;
}    /* end of rtk_trap_mgmtFrameMgmtVlanEnable_get */

int32
rtk_trap_mgmtFrameMgmtVlanEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapOtherCfg_t config;

    config.unit = unit;
    config.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_MGMTFRAMEMGMTVLANENABLE_SET, &config,
            rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_trap_mgmtFrameMgmtVlanEnable_set */

int32
rtk_trap_bpduFloodPortmask_get(uint32 unit, rtk_portmask_t * pBPDU_flood_portmask)
{
    rtdrv_bpduFloodPmskCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_BPDUFLOODPORTMASK_GET, &config,
            rtdrv_bpduFloodPmskCfg_t, 1);

    memcpy(pBPDU_flood_portmask, &config.pmsk, sizeof(rtk_portmask_t));

    return RT_ERR_OK;
}    /* end of rtk_trap_bpduFloodPortmask_get */

int32
rtk_trap_bpduFloodPortmask_set(uint32 unit, rtk_portmask_t * pBPDU_flood_portmask)
{
    rtdrv_bpduFloodPmskCfg_t config;

    config.unit = unit;
    memcpy(&config.pmsk, pBPDU_flood_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_TRAP_BPDUFLOODPORTMASK_SET, &config,
            rtdrv_bpduFloodPmskCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_trap_bpduFloodPortmask_set */


int32
rtk_trap_rmaGroupAction_get(uint32 unit, rtk_trap_rmaGroup_frameType_t rmaGroup_frameType, rtk_trap_rma_action_t * pRma_action)
{
    rtdrv_rmaGroupType_t config;

    config.unit = unit;
    config.rmaGroup_frameType = rmaGroup_frameType;
    GETSOCKOPT(RTDRV_TRAP_RMAGROUPACTION_GET, &config,
            rtdrv_rmaGroupType_t, 1);

    memcpy(pRma_action, &config.rma_action, sizeof(rtk_trap_rma_action_t));

    return RT_ERR_OK;
}    /* end of rtk_trap_rmaGroupAction_get */

int32
rtk_trap_rmaGroupAction_set(uint32 unit, rtk_trap_rmaGroup_frameType_t rmaGroup_frameType, rtk_trap_rma_action_t rma_action)
{
    rtdrv_rmaGroupType_t config;

    config.unit = unit;
    config.rmaGroup_frameType = rmaGroup_frameType;
    memcpy(&config.rma_action, &rma_action, sizeof(rtk_trap_rma_action_t));

    SETSOCKOPT(RTDRV_TRAP_RMAGROUPACTION_SET, &config,
            rtdrv_rmaGroupType_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_trap_rmaGroupAction_set */

int32
rtk_trap_rmaGroupLearningEnable_get(uint32 unit, rtk_trap_rmaGroup_frameType_t rmaGroup_frameType, rtk_enable_t * pEnable)
{
    rtdrv_rmaGroupLearn_t config;

    config.unit = unit;
    config.rmaGroup_frameType = rmaGroup_frameType;
    GETSOCKOPT(RTDRV_TRAP_RMAGROUPLEARNINGENABLE_GET, &config,
            rtdrv_rmaGroupType_t, 1);

    *pEnable = config.enable;

    return RT_ERR_OK;
}    /* end of rtk_trap_rmaGroupLearningEnable_get */

int32
rtk_trap_rmaGroupLearningEnable_set(uint32 unit, rtk_trap_rmaGroup_frameType_t rmaGroup_frameType, rtk_enable_t enable)
{
    rtdrv_rmaGroupLearn_t config;

    config.unit = unit;
    config.rmaGroup_frameType = rmaGroup_frameType;
    config.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_RMAGROUPLEARNINGENABLE_SET, &config,
            rtdrv_rmaGroupLearn_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_trap_rmaGroupLearningEnable_set */

int32
rtk_trap_mgmtFrameSelfARPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapOtherCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_MGMTFRAMESELFARPENABLE_GET, &config,
            rtdrv_trapOtherCfg_t, 1);

    *pEnable = config.enable;

    return RT_ERR_OK;
}    /* end of rtk_trap_mgmtFrameSelfARPEnable_get */

int32
rtk_trap_mgmtFrameSelfARPEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapOtherCfg_t config;

    config.unit = unit;
    config.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_MGMTFRAMESELFARPENABLE_SET, &config,
            rtdrv_trapOtherCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_trap_mgmtFrameSelfARPEnable_set */

int32
rtk_trap_rmaLookupMissActionEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_trapCfg_t trap_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    trap_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRAP_RMALOOKUPMISSACTIONENABLE_GET, &trap_cfg, rtdrv_trapCfg_t, 1);
    memcpy(pEnable, &trap_cfg.enable, sizeof(rtk_enable_t));

    return RT_ERR_OK;
}   /* end of rtk_trap_rmaLookupMissActionEnable_get */

int32
rtk_trap_rmaLookupMissActionEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_trapCfg_t trap_cfg;

    /* function body */
    trap_cfg.unit = unit;
    trap_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TRAP_RMALOOKUPMISSACTIONENABLE_SET, &trap_cfg, rtdrv_trapCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_trap_rmaLookupMissActionEnable_set */



