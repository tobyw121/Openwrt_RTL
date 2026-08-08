/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * $Revision: 30055 $
 * $Date: 2012-06-19 15:33:08 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) pie
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32
rtk_pie_pieRuleEntryFieldSize_get(uint32 unit, rtk_pie_fieldType_t type, uint32 *pField_size)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.field_type = type;
    GETSOCKOPT(RTDRV_PIE_ENTRY_FIELD_SIZE_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pField_size = pie_cfg.size;
    
    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntrySize_get(uint32 unit, uint32 *pEntry_size)
{
    rtdrv_pieCfg_t pie_cfg;
    
    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    GETSOCKOPT(RTDRV_PIE_ENTRY_SIZE_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pEntry_size = pie_cfg.size;
    
    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntryField_get(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    rtdrv_pieCfg_t pie_cfg;
    
    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.phase = phase;
    pie_cfg.index = entry_idx;
    memcpy(pie_cfg.entry_buffer, pEntry_buffer, RTDRV_PIE_ENTRY_BUFFER_SIZE);
    pie_cfg.field_type = type;
    GETSOCKOPT(RTDRV_PIE_ENTRY_FIELD_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pData, pie_cfg.field_data, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    memcpy(pMask, pie_cfg.field_mask, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    
    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntryField_set(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)

{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.phase = phase;
    pie_cfg.index = entry_idx;
    memcpy(pie_cfg.entry_buffer, pEntry_buffer, RTDRV_PIE_ENTRY_BUFFER_SIZE);
    pie_cfg.field_type = type;
    memcpy(pie_cfg.field_data, pData, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    memcpy(pie_cfg.field_mask, pMask, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    GETSOCKOPT(RTDRV_PIE_ENTRY_FIELD_SET_TO_BUFFER, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pEntry_buffer, pie_cfg.entry_buffer, RTDRV_PIE_ENTRY_BUFFER_SIZE);
    
    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntryField_read(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    rtdrv_pieCfg_t pie_cfg;
    
    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.phase = phase;
    pie_cfg.index = entry_idx;
    pie_cfg.field_type = type;
    GETSOCKOPT(RTDRV_PIE_ENTRY_FIELD_READ, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pData, pie_cfg.field_data, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    memcpy(pMask, pie_cfg.field_mask, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    
    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntryField_write(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)

{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.phase = phase;
    pie_cfg.index = entry_idx;
    pie_cfg.field_type = type;
    memcpy(pie_cfg.field_data, pData, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    memcpy(pie_cfg.field_mask, pMask, RTDRV_PIE_ENTRY_FIELD_BUFFER_SIZE);
    SETSOCKOPT(RTDRV_PIE_ENTRY_FIELD_WRITE, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_pie_piePreDefinedRuleEntry_get(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry)
{
    rtdrv_pieCfg_t pie_cfg;
    
    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(pie_cfg.entry_buffer, pEntry_buffer, RTDRV_PIE_ENTRY_BUFFER_SIZE);
    GETSOCKOPT(RTDRV_PIE_PREDEFINED_ENTRY_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pPredefined_entry, &pie_cfg.predefined_entry, sizeof(rtk_pie_preDefinedRuleEntry_t));
    
    return RT_ERR_OK;    
}

int32
rtk_pie_piePreDefinedRuleEntry_set(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.predefined_entry, pPredefined_entry, sizeof(rtk_pie_preDefinedRuleEntry_t));
    GETSOCKOPT(RTDRV_PIE_BUFFER_ENTRY_FROM_PREDEF_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pEntry_buffer, pie_cfg.entry_buffer, RTDRV_PIE_ENTRY_BUFFER_SIZE);

    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntry_read(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    rtdrv_pieCfg_t pie_cfg;
    
    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = entry_idx;
    GETSOCKOPT(RTDRV_PIE_ENTRY_READ, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pEntry_buffer, pie_cfg.entry_buffer, RTDRV_PIE_ENTRY_BUFFER_SIZE);
    
    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntry_write(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = entry_idx;
    memcpy(pie_cfg.entry_buffer, pEntry_buffer, RTDRV_PIE_ENTRY_BUFFER_SIZE);
    SETSOCKOPT(RTDRV_PIE_ENTRY_WRITE, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntry_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.block_content, pContent, sizeof(rtk_pie_clearBlockContent_t));
    SETSOCKOPT(RTDRV_PIE_ENTRY_DEL, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntry_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.move_content, pContent, sizeof(rtk_pie_movePieContent_t));
    SETSOCKOPT(RTDRV_PIE_ENTRY_MOVE, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieRuleEntry_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.move_content, pContent, sizeof(rtk_pie_movePieContent_t));
    SETSOCKOPT(RTDRV_PIE_ENTRY_SWAP, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieRuleAction_get(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = action_idx;
    GETSOCKOPT(RTDRV_PIE_ACTION_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pAction, &pie_cfg.action, sizeof(rtk_pie_actionTable_t));
    
    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleAction_set(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = action_idx;
    memcpy(&pie_cfg.action, pAction, sizeof(rtk_pie_actionTable_t));
    SETSOCKOPT(RTDRV_PIE_ACTION_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;   
}

int32
rtk_pie_pieRuleAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.block_content, pContent, sizeof(rtk_pie_clearBlockContent_t));
    SETSOCKOPT(RTDRV_PIE_ACTION_DEL, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.move_content, pContent, sizeof(rtk_pie_movePieContent_t));
    SETSOCKOPT(RTDRV_PIE_ACTION_MOVE, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieRuleAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.move_content, pContent, sizeof(rtk_pie_movePieContent_t));
    SETSOCKOPT(RTDRV_PIE_ACTION_SWAP, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieRuleEntryAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.block_content, pContent, sizeof(rtk_pie_clearBlockContent_t));
    SETSOCKOPT(RTDRV_PIE_ENTRY_ACTION_DEL, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_pie_pieRuleEntryAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.move_content, pContent, sizeof(rtk_pie_movePieContent_t));
    SETSOCKOPT(RTDRV_PIE_ENTRY_ACTION_MOVE, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieRuleEntryAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.move_content, pContent, sizeof(rtk_pie_movePieContent_t));
    SETSOCKOPT(RTDRV_PIE_ENTRY_ACTION_SWAP, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieRulePolicer_get(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = policer_idx;
    GETSOCKOPT(RTDRV_PIE_POLICER_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pPolicer, &pie_cfg.policer, sizeof(rtk_pie_policerEntry_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieRulePolicer_set(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = policer_idx;
    memcpy(&pie_cfg.policer, pPolicer, sizeof(rtk_pie_policerEntry_t));
    SETSOCKOPT(RTDRV_PIE_POLICER_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;   
}

int32
rtk_pie_pieHitIndication_get(
    uint32                       unit,
    rtk_pie_id_t                 lblock_idx,
    rtk_pie_hitIndicationEntry_t *pStatus)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = lblock_idx;
    GETSOCKOPT(RTDRV_PIE_HIT_INDICATION_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pStatus, &pie_cfg.hit_status, sizeof(rtk_pie_hitIndicationEntry_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieStat_get(uint32 unit, rtk_pie_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = log_id;
    GETSOCKOPT(RTDRV_PIE_COUNTER_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pPkt_cnt = pie_cfg.pkt_cnt;
    *pByte_cnt = pie_cfg.byte_cnt;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieStat_set(uint32 unit, rtk_pie_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = log_id;
    pie_cfg.pkt_cnt = pkt_cnt;
    pie_cfg.byte_cnt = byte_cnt;
    SETSOCKOPT(RTDRV_PIE_COUNTER_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;   
}

int32
rtk_pie_pieStat_clearAll(uint32 unit)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    SETSOCKOPT(RTDRV_PIE_COUNTER_CLEARALL, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;     
}

int32
rtk_pie_pieTemplateSelector_get(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            *pTemplate_idx)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = pblock_idx;
    pie_cfg.phase = phase;
    GETSOCKOPT(RTDRV_PIE_TMP_SELECTOR_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pTemplate_idx = pie_cfg.index1;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieTemplateSelector_set(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            template_idx)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = pblock_idx;
    pie_cfg.phase = phase;
    pie_cfg.index1 = template_idx;
    SETSOCKOPT(RTDRV_PIE_TMP_SELECTOR_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;   
}

int32
rtk_pie_pieUserTemplate_get(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = template_idx;
    GETSOCKOPT(RTDRV_PIE_USER_TMP_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pTemplate, &pie_cfg.template, sizeof(rtk_pie_template_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieUserTemplate_set(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = template_idx;
    memcpy(&pie_cfg.template, pTemplate, sizeof(rtk_pie_template_t));
    SETSOCKOPT(RTDRV_PIE_USER_TMP_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;   
}

int32
rtk_pie_pieL34ChecksumErr_get(uint32 unit, rtk_pie_l34ChecksumErrOper_t *pOperation)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    GETSOCKOPT(RTDRV_PIE_L34_CHECKSUN_ERR_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pOperation = pie_cfg.checksum_err_op;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieL34ChecksumErr_set(uint32 unit, rtk_pie_l34ChecksumErrOper_t operation)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.checksum_err_op = operation;
    SETSOCKOPT(RTDRV_PIE_L34_CHECKSUN_ERR_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieUserTemplatePayloadOffset_get(
    uint32        unit,
    rtk_pie_id_t  pblock_idx,
    rtk_pie_id_t  offset_idx,
    uint32        *pOffset)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = pblock_idx;
    pie_cfg.index1 = offset_idx;
    GETSOCKOPT(RTDRV_PIE_TMP_PAYLOAD_OFFSET_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pOffset = pie_cfg.offset;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieUserTemplatePayloadOffset_set(
    uint32        unit,
    rtk_pie_id_t  pblock_idx,
    rtk_pie_id_t  offset_idx,
    uint32        offset)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = pblock_idx;
    pie_cfg.index1 = offset_idx;
    pie_cfg.offset = offset;
    SETSOCKOPT(RTDRV_PIE_TMP_PAYLOAD_OFFSET_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieResultReverse_get(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  *pOperation)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = entry_idx;
    GETSOCKOPT(RTDRV_PIE_RESULT_REVERSE_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pOperation = pie_cfg.reverse_op;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieResultReverse_set(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  operation)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = entry_idx;
    pie_cfg.reverse_op = operation;
    SETSOCKOPT(RTDRV_PIE_RESULT_REVERSE_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieResultAggregator_get(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   *pType)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = pblockRange_idx;
    pie_cfg.index1 = entry_idx;
    GETSOCKOPT(RTDRV_PIE_RESULT_AGGREGATOR_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pType = pie_cfg.aggregator_type;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieResultAggregator_set(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   type)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = pblockRange_idx;
    pie_cfg.index1 = entry_idx;
    pie_cfg.aggregator_type = type;
    SETSOCKOPT(RTDRV_PIE_RESULT_AGGREGATOR_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK; 
}

int32
rtk_pie_pieBlockPriority_get(uint32 unit, rtk_pie_id_t lblock_idx, uint32 *pPriority)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = lblock_idx;
    GETSOCKOPT(RTDRV_PIE_BLOCK_PRI_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pPriority = pie_cfg.priority;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieBlockPriority_set(uint32 unit, rtk_pie_id_t lblock_idx, uint32 priority)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = lblock_idx;
    pie_cfg.priority = priority;
    SETSOCKOPT(RTDRV_PIE_BLOCK_PRI_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_pieGroupCtrl_get(
    uint32                      unit,
    rtk_pie_groupCtrlRange_t    lblockRange_idx,
    rtk_pie_groupCtrl_t         *pOperation)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = lblockRange_idx;
    GETSOCKOPT(RTDRV_PIE_GROUP_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pOperation = pie_cfg.group_op;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieGroupCtrl_set(
    uint32                      unit,
    rtk_pie_groupCtrlRange_t    lblockRange_idx,
    rtk_pie_groupCtrl_t         operation)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = lblockRange_idx;
    pie_cfg.group_op = operation;
    SETSOCKOPT(RTDRV_PIE_GROUP_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_pieEgrAclLookupCtrl_get(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    GETSOCKOPT(RTDRV_PIE_EGR_ACL_CTRL_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pControl, &pie_cfg.egr_acl_ctrl, sizeof(rtk_pie_egrAclLookupCtrl_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieEgrAclLookupCtrl_set(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.egr_acl_ctrl, pControl, sizeof(rtk_pie_egrAclLookupCtrl_t));
    SETSOCKOPT(RTDRV_PIE_EGR_ACL_CTRL_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_piePortLookupPhaseEnable_get(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     *pEnable)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.phase = phase;
    GETSOCKOPT(RTDRV_PIE_PORT_LP_ENABLE_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pEnable = pie_cfg.enable;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_piePortLookupPhaseEnable_set(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     enable)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.phase = phase;
    pie_cfg.enable = enable;
    SETSOCKOPT(RTDRV_PIE_PORT_LP_ENABLE_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_piePortLookupPhaseMiss_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   *pAction)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.phase = phase;
    GETSOCKOPT(RTDRV_PIE_PORT_LP_MISS_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pAction = pie_cfg.miss_action;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_piePortLookupPhaseMiss_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   action)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.phase = phase;
    pie_cfg.miss_action = action;
    SETSOCKOPT(RTDRV_PIE_PORT_LP_MISS_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_pieCounterIndicationMode_get(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  *pMode)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = lblock_idx;
    GETSOCKOPT(RTDRV_PIE_COUNTER_MODE_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pMode = pie_cfg.counter_mode;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_pieCounterIndicationMode_set(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  mode)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = lblock_idx;
    pie_cfg.counter_mode = mode;
    SETSOCKOPT(RTDRV_PIE_COUNTER_MODE_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_piePolicerCtrl_get(uint32 unit, rtk_pie_policerCtrl_t *pControl)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    GETSOCKOPT(RTDRV_PIE_POLICER_CTRL_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pControl, &pie_cfg.policer_ctrl, sizeof(rtk_pie_policerCtrl_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_piePolicerCtrl_set(uint32 unit, rtk_pie_policerCtrl_t *pControl)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    memcpy(&pie_cfg.policer_ctrl, pControl, sizeof(rtk_pie_policerCtrl_t));
    SETSOCKOPT(RTDRV_PIE_POLICER_CTRL_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_rangeCheckL4Port_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    GETSOCKOPT(RTDRV_PIE_RC_L4PORT_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pData, &pie_cfg.rc_l4port_data, sizeof(rtk_pie_rangeCheck_l4Port_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_rangeCheckL4Port_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    memcpy(&pie_cfg.rc_l4port_data, pData, sizeof(rtk_pie_rangeCheck_l4Port_t));
    SETSOCKOPT(RTDRV_PIE_RC_L4PORT_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_rangeCheckVid_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    GETSOCKOPT(RTDRV_PIE_RC_VID_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pData, &pie_cfg.rc_vid_data, sizeof(rtk_pie_rangeCheck_vid_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_rangeCheckVid_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    memcpy(&pie_cfg.rc_vid_data, pData, sizeof(rtk_pie_rangeCheck_vid_t));
    SETSOCKOPT(RTDRV_PIE_RC_VID_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_rangeCheckIp_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    GETSOCKOPT(RTDRV_PIE_RC_IP_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pData, &pie_cfg.rc_ip_data, sizeof(rtk_pie_rangeCheck_ip_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_rangeCheckIp_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    memcpy(&pie_cfg.rc_ip_data, pData, sizeof(rtk_pie_rangeCheck_ip_t));
    SETSOCKOPT(RTDRV_PIE_RC_IP_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_rangeCheckSrcPort_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    GETSOCKOPT(RTDRV_PIE_RC_SRC_PORT_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pData, &pie_cfg.rc_src_port_data, sizeof(rtk_pie_rangeCheck_srcPortMask_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_rangeCheckSrcPort_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.index = index;
    memcpy(&pie_cfg.rc_src_port_data, pData, sizeof(rtk_pie_rangeCheck_srcPortMask_t));
    SETSOCKOPT(RTDRV_PIE_RC_SRC_PORT_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_fieldSelectorEnable_get(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t *pEnable)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.index = fs_idx;
    GETSOCKOPT(RTDRV_PIE_FS_ENABLE_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pEnable = pie_cfg.enable;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_fieldSelectorEnable_set(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t enable)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.index = fs_idx;
    pie_cfg.enable = enable;
    SETSOCKOPT(RTDRV_PIE_FS_ENABLE_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_fieldSelectorContent_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.index = fs_idx;
    GETSOCKOPT(RTDRV_PIE_FS_CONTENT_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pFs, &pie_cfg.fs_content, sizeof(rtk_pie_fieldSelector_data_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_fieldSelectorContent_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.index = fs_idx;
    memcpy(&pie_cfg.fs_content, pFs, sizeof(rtk_pie_fieldSelector_data_t));
    SETSOCKOPT(RTDRV_PIE_FS_CONTENT_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_patternMatchEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));    
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    GETSOCKOPT(RTDRV_PIE_PM_ENABLE_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    *pEnable = pie_cfg.enable;
    
    return RT_ERR_OK;  
}

int32
rtk_pie_patternMatchEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    pie_cfg.enable = enable;
    SETSOCKOPT(RTDRV_PIE_PM_ENABLE_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_pie_patternMatchContent_get(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent)
{
    rtdrv_pieCfg_t pie_cfg;
    
    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    GETSOCKOPT(RTDRV_PIE_PM_CONTENT_GET, &pie_cfg, rtdrv_pieCfg_t, 1);
    memcpy(pContent, &pie_cfg.pm_content, sizeof(rtk_pie_patternMatch_content_t));
    
    return RT_ERR_OK;  
}

int32
rtk_pie_patternMatchContent_set(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent)
{
    rtdrv_pieCfg_t pie_cfg;

    memset(&pie_cfg, 0, sizeof(rtdrv_pieCfg_t));
    pie_cfg.unit = unit;
    pie_cfg.port = port;
    memcpy(&pie_cfg.pm_content, pContent, sizeof(rtk_pie_patternMatch_content_t));
    SETSOCKOPT(RTDRV_PIE_PM_CONTENT_SET, &pie_cfg, rtdrv_pieCfg_t, 1); 

    return RT_ERR_OK;
}

