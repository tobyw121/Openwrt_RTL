/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 42658 $
 * $Date: 2013-09-06 20:49:50 +0800 (Fri, 06 Sep 2013) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) ACL
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/acl.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>


int32
rtk_acl_ruleEntryFieldSize_get(uint32 unit, rtk_acl_fieldType_t type, uint32 *pField_size)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.field_type = type;
    GETSOCKOPT(RTDRV_ACL_ENTRY_FIELD_SIZE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pField_size = acl_cfg.size;

    return RT_ERR_OK;
}

int32
rtk_acl_ruleEntrySize_get(uint32 unit, rtk_acl_phase_t phase, uint32 *pEntry_size)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    GETSOCKOPT(RTDRV_ACL_ENTRY_SIZE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pEntry_size = acl_cfg.size;

    return RT_ERR_OK;
}

int32
rtk_acl_ruleEntry_read(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    rtdrv_aclCfg_t acl_cfg;
    uint32 entry_size = 0;

    memset(&acl_cfg, 0, sizeof(rtdrv_aclCfg_t));
    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    GETSOCKOPT(RTDRV_ACL_ENTRY_DATA_READ, &acl_cfg, rtdrv_aclCfg_t, 1);

    rtk_acl_ruleEntrySize_get(unit, phase, &entry_size);
    memcpy(pEntry_buffer, acl_cfg.entry_buffer, entry_size);

    return RT_ERR_OK;
}

int32
rtk_acl_ruleEntryField_write(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    acl_cfg.field_type = type;
    memcpy(acl_cfg.field_data, pData, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memcpy(acl_cfg.field_mask, pMask, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    SETSOCKOPT(RTDRV_ACL_ENTRY_DATA_WRITE, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_acl_meterMode_get(
    uint32  unit,
    uint32  blockIdx,
    rtk_acl_meterMode_t *pMeterMode)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = blockIdx;
    GETSOCKOPT(RTDRV_ACL_METER_MODE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pMeterMode = acl_cfg.meterMode;

    return RT_ERR_OK;
}

int32
rtk_acl_meterMode_set(
    uint32  unit,
    uint32  blockIdx,
    rtk_acl_meterMode_t meterMode)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = blockIdx;
    acl_cfg.meterMode = meterMode;

    SETSOCKOPT(RTDRV_ACL_METER_MODE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_acl_meterIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    GETSOCKOPT(RTDRV_ACL_METER_INCLUDE_IFG_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pIfg_include = acl_cfg.ifg_include;

    return RT_ERR_OK;
}

int32
rtk_acl_meterIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.ifg_include = ifg_include;
    SETSOCKOPT(RTDRV_ACL_METER_INCLUDE_IFG_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_acl_meterBurstSize_get(
    uint32              unit,
    rtk_acl_meterMode_t meterMode,
    rtk_acl_meterBurstSize_t  *pBurstSize)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.meterMode = meterMode;
    GETSOCKOPT(RTDRV_ACL_METER_BURST_SIZE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pBurstSize = acl_cfg.burstSize;

    return RT_ERR_OK;
}

int32
rtk_acl_meterBurstSize_set(
    uint32              unit,
    rtk_acl_meterMode_t meterMode,
    rtk_acl_meterBurstSize_t  *pBurstSize)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.meterMode = meterMode;
    acl_cfg.burstSize = *pBurstSize;
    SETSOCKOPT(RTDRV_ACL_METER_BURST_SIZE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_acl_meterExceed_get(
    uint32  unit,
    uint32  meterIdx,
    uint32  *pIsExceed)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.meterIdx = meterIdx;
    GETSOCKOPT(RTDRV_ACL_METER_EXCEED_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pIsExceed = acl_cfg.isExceed;

    return RT_ERR_OK;
}

int32
rtk_acl_meterExceedAggregation_get(
    uint32  unit,
    uint32  *pExceedMask)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    GETSOCKOPT(RTDRV_ACL_METER_EXCEED_AGGREGATION_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pExceedMask = acl_cfg.exceedMask;

    return RT_ERR_OK;
}

int32
rtk_acl_meterEntry_get(
    uint32  unit,
    uint32  meterIdx,
    rtk_acl_meterEntry_t   *pMeterEntry)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.meterIdx = meterIdx;
    GETSOCKOPT(RTDRV_ACL_METER_ENTRY_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pMeterEntry = acl_cfg.meterEntry;

    return RT_ERR_OK;
}

int32
rtk_acl_meterEntry_set(
    uint32  unit,
    uint32  meterIdx,
    rtk_acl_meterEntry_t   *pMeterEntry)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.meterIdx = meterIdx;
    acl_cfg.meterEntry = *pMeterEntry;
    SETSOCKOPT(RTDRV_ACL_METER_ENTRY_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_acl_partition_get(uint32 unit, uint32 *pPartition)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;

    GETSOCKOPT(RTDRV_ACL_PARTITION_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pPartition = acl_cfg.blockIdx;

    return RT_ERR_OK;
}    /* end of rtk_acl_partition_get */

int32
rtk_acl_partition_set(uint32 unit, uint32 partition)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = partition;

    SETSOCKOPT(RTDRV_ACL_PARTITION_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_partition_set */

int32
rtk_acl_blockPwrEnable_get(uint32 unit, uint32 block_idx, rtk_enable_t *pEnable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;

    GETSOCKOPT(RTDRV_ACL_BLOCKPWRENABLE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pEnable = acl_cfg.status;

    return RT_ERR_OK;
}    /* end of rtk_acl_blockPwrEnable_get */

int32
rtk_acl_blockPwrEnable_set(uint32 unit, uint32 block_idx, rtk_enable_t enable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;
    acl_cfg.status = enable;

    SETSOCKOPT(RTDRV_ACL_BLOCKPWRENABLE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_blockPwrEnable_set */

int32
rtk_acl_blockLookupEnable_get(uint32 unit, uint32 block_idx, rtk_enable_t *pEnable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;

    GETSOCKOPT(RTDRV_ACL_BLOCKLOOKUPENABLE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pEnable = acl_cfg.status;

    return RT_ERR_OK;
}    /* end of rtk_acl_blockLookupEnable_get */

int32
rtk_acl_blockLookupEnable_set(uint32 unit, uint32 block_idx, rtk_enable_t enable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;
    acl_cfg.status = enable;

    SETSOCKOPT(RTDRV_ACL_BLOCKLOOKUPENABLE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_blockLookupEnable_set */

int32
rtk_acl_portLookupEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.port = port;

    GETSOCKOPT(RTDRV_ACL_PORTLOOKUPENABLE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pEnable = acl_cfg.status;

    return RT_ERR_OK;
} /* end of rtk_acl_portLookupEnable_get */

int32
rtk_acl_portLookupEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.port = port;
    acl_cfg.status = enable;

    SETSOCKOPT(RTDRV_ACL_PORTLOOKUPENABLE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_acl_portLookupEnable_set */

int32
rtk_acl_lookupMissAct_get(uint32 unit, rtk_port_t port, rtk_acl_lookupMissAct_t *pLmAct)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.port = port;

    GETSOCKOPT(RTDRV_ACL_LOOKUPMISSACT_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pLmAct = acl_cfg.lmAct;

    return RT_ERR_OK;
} /* end of rtk_acl_lookupMissAct_get */

int32
rtk_acl_lookupMissAct_set(uint32 unit, rtk_port_t port, rtk_acl_lookupMissAct_t lmAct)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.port = port;
    acl_cfg.lmAct = lmAct;

    SETSOCKOPT(RTDRV_ACL_LOOKUPMISSACT_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_acl_lookupMissAct_set */

int32
rtk_acl_ruleValidate_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              *pValid)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;

    GETSOCKOPT(RTDRV_ACL_RULEVALIDATE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pValid = acl_cfg.status;

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleValidate_get */

int32
rtk_acl_ruleValidate_set(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              valid)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    acl_cfg.status = valid;

    SETSOCKOPT(RTDRV_ACL_RULEVALIDATE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleValidate_set */

int32
rtk_acl_ruleEntry_write(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    uint32          entry_size = 0;
    rtdrv_aclCfg_t  acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;

    rtk_acl_ruleEntrySize_get(unit, phase, &entry_size);
    memcpy(acl_cfg.entry_buffer, pEntry_buffer, entry_size);

    SETSOCKOPT(RTDRV_ACL_RULEENTRY_WRITE, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleEntry_write */

int32
rtk_acl_ruleEntryField_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    uint32          entry_size = 0;
    rtdrv_aclCfg_t  acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    acl_cfg.field_type = type;

    rtk_acl_ruleEntrySize_get(unit, phase, &entry_size);
    memcpy(acl_cfg.entry_buffer, pEntry_buffer, entry_size);

    GETSOCKOPT(RTDRV_ACL_RULEENTRYFIELD_GET, &acl_cfg, rtdrv_aclCfg_t, 1);

    memcpy(pData, acl_cfg.field_data, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memcpy(pMask, acl_cfg.field_mask, RTK_MAX_SIZE_OF_ACL_USER_FIELD);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleEntryField_get */

int32
rtk_acl_ruleEntryField_set(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    uint32          entry_size = 0;
    rtdrv_aclCfg_t  acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    acl_cfg.field_type = type;

    rtk_acl_ruleEntrySize_get(unit, phase, &entry_size);
    memcpy(acl_cfg.entry_buffer, pEntry_buffer, entry_size);

    memcpy(acl_cfg.field_data, pData, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memcpy(acl_cfg.field_mask, pMask, RTK_MAX_SIZE_OF_ACL_USER_FIELD);

    SETSOCKOPT(RTDRV_ACL_RULEENTRYFIELD_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    memcpy(pEntry_buffer, acl_cfg.entry_buffer, entry_size);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleEntryField_set */

int32
rtk_acl_ruleEntryField_read(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    acl_cfg.field_type = type;

    GETSOCKOPT(RTDRV_ACL_RULEENTRYFIELD_READ, &acl_cfg, rtdrv_aclCfg_t, 1);

    memcpy(pData, acl_cfg.field_data, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memcpy(pMask, acl_cfg.field_mask, RTK_MAX_SIZE_OF_ACL_USER_FIELD);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleEntryField_read */

int32
rtk_acl_ruleEntryField_check(uint32 unit, rtk_acl_phase_t phase,
        rtk_acl_fieldType_t type)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.field_type = type;

    GETSOCKOPT(RTDRV_ACL_RULEENTRYFIELD_CHECK, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleEntryField_check */

int32
rtk_acl_ruleOperation_get(
    uint32                  unit,
    rtk_acl_phase_t         phase,
    rtk_acl_id_t            entry_idx,
    rtk_acl_operation_t     *pOperation)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;

    GETSOCKOPT(RTDRV_ACL_RULEOPERATION_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    memcpy(pOperation, &acl_cfg.oper, sizeof(rtk_acl_operation_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleOperation_get */

int32
rtk_acl_ruleOperation_set(
    uint32                  unit,
    rtk_acl_phase_t         phase,
    rtk_acl_id_t            entry_idx,
    rtk_acl_operation_t     *pOperation)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    memcpy(&acl_cfg.oper, pOperation, sizeof(rtk_acl_operation_t));

    SETSOCKOPT(RTDRV_ACL_RULEOPERATION_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleOperation_set */

int32
rtk_acl_ruleAction_get(
    uint32               unit,
    rtk_acl_phase_t      phase,
    rtk_acl_id_t         entry_idx,
    rtk_acl_action_t     *pAction)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;

    GETSOCKOPT(RTDRV_ACL_RULEACTION_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    memcpy(pAction, &acl_cfg.action, sizeof(rtk_acl_action_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleAction_get */

int32
rtk_acl_ruleAction_set(
    uint32               unit,
    rtk_acl_phase_t      phase,
    rtk_acl_id_t         entry_idx,
    rtk_acl_action_t     *pAction)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    memcpy(&acl_cfg.action, pAction, sizeof(rtk_acl_action_t));

    SETSOCKOPT(RTDRV_ACL_RULEACTION_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_ruleAction_set */

int32
rtk_acl_ruleHitIndication_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              reset,
    uint32              *pIsHit)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.index = entry_idx;
    acl_cfg.count = reset;

    GETSOCKOPT(RTDRV_ACL_RULEHITINDICATION_GET, &acl_cfg, rtdrv_aclCfg_t, 1);

    *pIsHit = acl_cfg.status;

    return RT_ERR_OK;
}

int32
rtk_acl_rule_del(uint32 unit, rtk_acl_phase_t phase, rtk_acl_clear_t *pClrIdx)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    memcpy(&acl_cfg.clear, pClrIdx, sizeof(rtk_acl_clear_t));

    SETSOCKOPT(RTDRV_ACL_RULE_DEL, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rule_del */

int32
rtk_acl_rule_move(uint32 unit, rtk_acl_phase_t phase, rtk_acl_move_t *pData)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    memcpy(&acl_cfg.move, pData, sizeof(rtk_acl_move_t));

    SETSOCKOPT(RTDRV_ACL_RULE_MOVE, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rule_move */

int32
rtk_acl_templateSelector_get(
    uint32                  unit,
    uint32                  block_idx,
    rtk_acl_templateIdx_t   *pTemplate_idx)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;

    GETSOCKOPT(RTDRV_ACL_TEMPLATESELECTOR_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    memcpy(pTemplate_idx, &acl_cfg.template_idx, sizeof(rtk_acl_templateIdx_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_templateSelector_get */

int32
rtk_acl_templateSelector_set(
    uint32                  unit,
    uint32                  block_idx,
    rtk_acl_templateIdx_t   template_idx)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;
    memcpy(&acl_cfg.template_idx, &template_idx, sizeof(rtk_acl_templateIdx_t));

    SETSOCKOPT(RTDRV_ACL_TEMPLATESELECTOR_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_templateSelector_set */

int32
rtk_acl_template_get(uint32 unit, uint32 template_idx, rtk_acl_template_t *pTemplate)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = template_idx;

    GETSOCKOPT(RTDRV_ACL_TEMPLATE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    memcpy(pTemplate, &acl_cfg.template, sizeof(rtk_acl_template_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_template_get */

int32
rtk_acl_template_set(uint32 unit, uint32 template_idx, rtk_acl_template_t *pTemplate)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = template_idx;
    memcpy(&acl_cfg.template, pTemplate, sizeof(rtk_acl_template_t));

    SETSOCKOPT(RTDRV_ACL_TEMPLATE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_template_set */

int32
rtk_acl_templateField_check(uint32 unit, rtk_acl_phase_t phase,
    rtk_acl_templateFieldType_t type)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.phase = phase;
    acl_cfg.field_type = type;
    GETSOCKOPT(RTDRV_ACL_TEMPLATEFIELD_CHECK, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_templateField_check */

int32
rtk_acl_blockResultMode_get(uint32 unit, uint32 block_idx, rtk_acl_blockResultMode_t *pMode)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;

    GETSOCKOPT(RTDRV_ACL_BLOCKRESULTMODE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pMode = acl_cfg.blk_mode;

    return RT_ERR_OK;
}    /* end of rtk_acl_blockResultMode_get */

int32
rtk_acl_blockResultMode_set(uint32 unit, uint32 block_idx, rtk_acl_blockResultMode_t mode)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;
    acl_cfg.blk_mode = mode;

    SETSOCKOPT(RTDRV_ACL_BLOCKRESULTMODE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_blockResultMode_set */

int32
rtk_acl_blockGroupEnable_get(
    uint32                     unit,
    uint32                     block_idx,
    rtk_acl_blockGroup_t       group_type,
    rtk_enable_t               *pEnable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;
    acl_cfg.blk_group = group_type;

    GETSOCKOPT(RTDRV_ACL_BLOCKAGGREGATORENABLE_GET, &acl_cfg, rtdrv_aclCfg_t, 1);

    *pEnable = acl_cfg.status;

    return RT_ERR_OK;
}    /* end of rtk_acl_blockGroupEnable_get */

int32
rtk_acl_blockGroupEnable_set(
    uint32                     unit,
    uint32                     block_idx,
    rtk_acl_blockGroup_t       group_type,
    rtk_enable_t               enable)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.blockIdx = block_idx;
    acl_cfg.blk_group = group_type;
    acl_cfg.status = enable;

    SETSOCKOPT(RTDRV_ACL_BLOCKAGGREGATORENABLE_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_blockGroupEnable_set */

int32
rtk_acl_statPktCnt_get(uint32 unit, uint32 log_id, uint32 *pPkt_cnt)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = log_id;

    GETSOCKOPT(RTDRV_ACL_STATPKTCNT_GET, &acl_cfg, rtdrv_aclCfg_t, 1);

    *pPkt_cnt = acl_cfg.size;

    return RT_ERR_OK;
}    /* end of rtk_acl_statPktCnt_get */

int32
rtk_acl_statPktCnt_clear(uint32 unit, uint32 log_id)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = log_id;

    SETSOCKOPT(RTDRV_ACL_STATPKTCNT_CLEAR, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_statPktCnt_clear */

int32
rtk_acl_statByteCnt_get(uint32 unit, uint32 log_id, uint64 *pByte_cnt)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = log_id;

    GETSOCKOPT(RTDRV_ACL_STATBYTECNT_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    *pByte_cnt = acl_cfg.count;

    return RT_ERR_OK;
}    /* end of rtk_acl_statByteCnt_get */

int32
rtk_acl_statByteCnt_clear(uint32 unit, uint32 log_id)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = log_id;

    SETSOCKOPT(RTDRV_ACL_STATBYTECNT_CLEAR, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_statByteCnt_clear */

int32
rtk_acl_stat_clearAll(uint32 unit)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;

    SETSOCKOPT(RTDRV_ACL_STAT_CLEARALL, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_stat_clearAll */

int32
rtk_acl_rangeCheckFieldSelector_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_fieldSelector_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;

    GETSOCKOPT(RTDRV_ACL_RANGECHECKFIELDSEL_GET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    memcpy(pData, &acl_cfg.range_fieldSel, sizeof(rtk_acl_rangeCheck_fieldSelector_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckL4Port_get */

int32
rtk_acl_rangeCheckFieldSelector_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_fieldSelector_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;
    memcpy(&acl_cfg.range_fieldSel, pData, sizeof(rtk_acl_rangeCheck_fieldSelector_t));

    SETSOCKOPT(RTDRV_ACL_RANGECHECKFIELDSEL_SET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckL4Port_set */

int32
rtk_acl_rangeCheckL4Port_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_l4Port_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;

    GETSOCKOPT(RTDRV_ACL_RANGECHECKL4PORT_GET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    memcpy(pData, &acl_cfg.range_l4Port, sizeof(rtk_acl_rangeCheck_l4Port_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckL4Port_get */

int32
rtk_acl_rangeCheckL4Port_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_l4Port_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;
    memcpy(&acl_cfg.range_l4Port, pData, sizeof(rtk_acl_rangeCheck_l4Port_t));

    SETSOCKOPT(RTDRV_ACL_RANGECHECKL4PORT_SET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckL4Port_set */

int32
rtk_acl_rangeCheckVid_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_vid_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;

    GETSOCKOPT(RTDRV_ACL_RANGECHECKVID_GET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    memcpy(pData, &acl_cfg.range_vid, sizeof(rtk_acl_rangeCheck_vid_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckVid_get */

int32
rtk_acl_rangeCheckVid_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_vid_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;
    memcpy(&acl_cfg.range_vid, pData, sizeof(rtk_acl_rangeCheck_vid_t));

    SETSOCKOPT(RTDRV_ACL_RANGECHECKVID_SET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckVid_set */

int32
rtk_acl_rangeCheckIp_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_ip_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;

    GETSOCKOPT(RTDRV_ACL_RANGECHECKIP_GET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);
    memcpy(pData, &acl_cfg.range_ip, sizeof(rtk_acl_rangeCheck_ip_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckIp_get */

int32
rtk_acl_rangeCheckIp_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_ip_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;
    memcpy(&acl_cfg.range_ip, pData, sizeof(rtk_acl_rangeCheck_ip_t));

    SETSOCKOPT(RTDRV_ACL_RANGECHECKIP_SET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckIp_set */

int32
rtk_acl_rangeCheckSrcPort_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;

    GETSOCKOPT(RTDRV_ACL_RANGECHECKSRCPORT_GET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);
    memcpy(pData, &acl_cfg.range_port, sizeof(rtk_acl_rangeCheck_portMask_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckSrcPort_get */

int32
rtk_acl_rangeCheckSrcPort_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;
    memcpy(&acl_cfg.range_port, pData, sizeof(rtk_acl_rangeCheck_portMask_t));

    SETSOCKOPT(RTDRV_ACL_RANGECHECKSRCPORT_SET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckSrcPort_set */

int32
rtk_acl_rangeCheckDstPort_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;

    GETSOCKOPT(RTDRV_ACL_RANGECHECKDSTPORT_GET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);
    memcpy(pData, &acl_cfg.range_port, sizeof(rtk_acl_rangeCheck_portMask_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckDstPort_get */

int32
rtk_acl_rangeCheckDstPort_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;
    memcpy(&acl_cfg.range_port, pData, sizeof(rtk_acl_rangeCheck_portMask_t));

    SETSOCKOPT(RTDRV_ACL_RANGECHECKDSTPORT_SET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckDstPort_set */

int32
rtk_acl_rangeCheckPacketLen_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_packetLen_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;

    GETSOCKOPT(RTDRV_ACL_RANGECHECKPACKETLEN_GET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);
    memcpy(pData, &acl_cfg.range_pktLen, sizeof(rtk_acl_rangeCheck_packetLen_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckPacketLen_get */

int32
rtk_acl_rangeCheckPacketLen_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_packetLen_t *pData)
{
    rtdrv_rangeCheckCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = index;
    memcpy(&acl_cfg.range_pktLen, pData, sizeof(rtk_acl_rangeCheck_packetLen_t));

    SETSOCKOPT(RTDRV_ACL_RANGECHECKPACKETLEN_SET, &acl_cfg, rtdrv_rangeCheckCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_rangeCheckPacketLen_set */

int32
rtk_acl_fieldSelector_get(
    uint32                       unit,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = fs_idx;

    GETSOCKOPT(RTDRV_ACL_FIELDSELECTOR_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    memcpy(pFs, &acl_cfg.fs, sizeof(rtk_acl_fieldSelector_data_t));

    return RT_ERR_OK;
}    /* end of rtk_acl_fieldSelector_get */

int32
rtk_acl_fieldSelector_set(
    uint32                       unit,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs)
{
    rtdrv_aclCfg_t acl_cfg;

    acl_cfg.unit = unit;
    acl_cfg.index = fs_idx;
    memcpy(&acl_cfg.fs, pFs, sizeof(rtk_acl_fieldSelector_data_t));

    SETSOCKOPT(RTDRV_ACL_FIELDSELECTOR_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_acl_fieldSelector_set */

int32
rtk_acl_templateFieldIntentVlanTag_get(uint32 unit,
    rtk_vlan_tagType_t *tagType)
{
    rtdrv_aclCfg_t acl_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == tagType), RT_ERR_NULL_POINTER);

    /* function body */
    memcpy(&acl_cfg.unit, &unit, sizeof(uint32));
    GETSOCKOPT(RTDRV_ACL_TEMPLATEFIELDINTENTVLANTAG_GET, &acl_cfg, rtdrv_aclCfg_t, 1);
    memcpy(tagType, &acl_cfg.tagType, sizeof(rtk_vlan_tagType_t));

    return RT_ERR_OK;
}   /* end of rtk_acl_templateFieldIntentVlanTag_get */

int32
rtk_acl_templateFieldIntentVlanTag_set(uint32 unit,
    rtk_vlan_tagType_t tagType)
{
    rtdrv_aclCfg_t acl_cfg;

    /* function body */
    memcpy(&acl_cfg.unit, &unit, sizeof(uint32));
    memcpy(&acl_cfg.tagType, &tagType, sizeof(rtk_vlan_tagType_t));
    SETSOCKOPT(RTDRV_ACL_TEMPLATEFIELDINTENTVLANTAG_SET, &acl_cfg, rtdrv_aclCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_acl_templateFieldIntentVlanTag_set */
