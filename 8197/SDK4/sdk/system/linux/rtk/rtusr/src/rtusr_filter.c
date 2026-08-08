/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * $Revision: 9051 $
 * $Date: 2010-04-15 16:07:17 +0800 (Thu, 15 Apr 2010) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) filter
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_filter_blkCutline_get(uint32 unit, uint32 *pCutline)
{
    rtdrv_filterCfg_t filter_cfg;
    
    filter_cfg.unit = unit;
    GETSOCKOPT(RTDRV_FILTER_CUTLINE_GET, &filter_cfg, rtdrv_filterCfg_t, 1);
    *pCutline = filter_cfg.cutline;
    
    return RT_ERR_OK;    
}

int32 rtk_filter_blkCutline_set(uint32 unit, uint32 cutline)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.cutline = cutline;
    SETSOCKOPT(RTDRV_FILTER_CUTLINE_SET, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_pieEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_filterCfg_t filter_cfg;
    
    filter_cfg.unit = unit;
    GETSOCKOPT(RTDRV_FILTER_ENABLE_GET, &filter_cfg, rtdrv_filterCfg_t, 1);
    *pEnable = filter_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_filter_pieEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FILTER_ENABLE_SET, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_flowTbl_get(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_flowTbl_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    rtdrv_filterCfg_t filter_cfg;
    
    filter_cfg.unit = unit;
    filter_cfg.filter_id = filter_id;
    GETSOCKOPT(RTDRV_FILTER_FLOW_TBL_GET, &filter_cfg, rtdrv_filterCfg_t, 1);
    memcpy(pFilter_cfg, &filter_cfg.flow_table_cfg, sizeof(rtk_filter_flowTbl_t));
    memcpy(pAction, &filter_cfg.action, sizeof(rtk_filter_action_t));
    
    return RT_ERR_OK;    
}

int32 rtk_filter_flowTbl_add(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_flowTbl_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.filter_id = filter_id;
    memcpy(&filter_cfg.flow_table_cfg, pFilter_cfg, sizeof(rtk_filter_flowTbl_t));
    memcpy(&filter_cfg.action, pAction, sizeof(rtk_filter_action_t));
    SETSOCKOPT(RTDRV_FILTER_FLOW_TBL_ADD, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_flowTbl_del(uint32 unit, rtk_filter_id_t filter_id)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.filter_id = filter_id;
    SETSOCKOPT(RTDRV_FILTER_FLOW_TBL_DEL, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_flowTbl_delAll(uint32 unit)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    SETSOCKOPT(RTDRV_FILTER_FLOW_TBL_DELALL, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_igrAcl_get(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_aclCfg_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    rtdrv_filterCfg_t filter_cfg;
    
    filter_cfg.unit = unit;
    filter_cfg.filter_id = filter_id;
    GETSOCKOPT(RTDRV_FILTER_IGR_ACL_GET, &filter_cfg, rtdrv_filterCfg_t, 1);
    memcpy(pFilter_cfg, &filter_cfg.acl_cfg, sizeof(rtk_filter_aclCfg_t));
    memcpy(pAction, &filter_cfg.action, sizeof(rtk_filter_action_t));
    
    return RT_ERR_OK;    
}

int32 rtk_filter_igrAcl_add(uint32 unit, rtk_filter_id_t filter_id, rtk_filter_aclCfg_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.filter_id = filter_id;
    memcpy(&filter_cfg.acl_cfg, pFilter_cfg, sizeof(rtk_filter_aclCfg_t));
    memcpy(&filter_cfg.action, pAction, sizeof(rtk_filter_action_t));
    SETSOCKOPT(RTDRV_FILTER_IGR_ACL_ADD, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_igrAcl_del(uint32 unit, rtk_filter_id_t filter_id)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.filter_id = filter_id;
    SETSOCKOPT(RTDRV_FILTER_IGR_ACL_DEL, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_igrAcl_delAll(uint32 unit)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    SETSOCKOPT(RTDRV_FILTER_IGR_ACL_DELALL, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_filter_stat_get(uint32 unit, uint32 log_index, uint32* packet_counter, uint64* byte_cpunter)
{
    rtdrv_filterCfg_t filter_cfg;
    
    filter_cfg.unit = unit;
    filter_cfg.index = log_index;
    GETSOCKOPT(RTDRV_FILTER_LOG_COUNTER_GET, &filter_cfg, rtdrv_filterCfg_t, 1);
    *packet_counter = filter_cfg.packet_counter;
    *byte_cpunter = filter_cfg.byte_counter;

    return RT_ERR_OK;    
}

int32 rtk_filter_stat_set(uint32 unit, uint32 log_index, uint32 packet_counter, uint64 byte_counter)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.index = log_index;
    filter_cfg.packet_counter = packet_counter;
    filter_cfg.byte_counter = byte_counter;
    SETSOCKOPT(RTDRV_FILTER_LOG_COUNTER_SET, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;   
}

int32 rtk_filter_patternMatch_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  *pMode,
    uint8                           *pPattern,
    uint32                          *pMask)
{
    rtdrv_patternCfg_t pattern_cfg;
    
    pattern_cfg.unit = unit;
    pattern_cfg.port = port;
    GETSOCKOPT(RTDRV_FILTER_PATTERN_MATCH_GET, &pattern_cfg, rtdrv_patternCfg_t, 1);
    *pMode = pattern_cfg.mode;
    memcpy(pPattern, &pattern_cfg.pattern, RTK_MAX_LENGTH_OF_PATTERN_MATCH);
    *pMask = pattern_cfg.mask;

    return RT_ERR_OK;    
}

int32 rtk_filter_patternMatch_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_filter_patternMatch_mode_t  mode,
    uint8                           *pPattern,
    uint32                          mask)
{
    rtdrv_patternCfg_t pattern_cfg;

    pattern_cfg.unit = unit;
    pattern_cfg.port = port;
    pattern_cfg.mode = mode;
    memcpy(&pattern_cfg.pattern, pPattern, RTK_MAX_LENGTH_OF_PATTERN_MATCH);
    pattern_cfg.mask = mask;
    SETSOCKOPT(RTDRV_FILTER_PATTERN_MATCH_SET, &pattern_cfg, rtdrv_patternCfg_t, 1); 

    return RT_ERR_OK;   
}

int32 rtk_filter_igrAclRateLimit_get(uint32 unit, rtk_meter_id_t meter_id, uint32 *pRate)
{
    rtdrv_filterCfg_t filter_cfg;
    
    filter_cfg.unit = unit;
    filter_cfg.index = meter_id;
    GETSOCKOPT(RTDRV_FILTER_RATE_LIMIT_GET, &filter_cfg, rtdrv_filterCfg_t, 1);
    *pRate = filter_cfg.rate;
    
    return RT_ERR_OK;    
}

int32 rtk_filter_igrAclRateLimit_set(uint32 unit, rtk_meter_id_t meter_id, uint32 rate)
{
    rtdrv_filterCfg_t filter_cfg;

    filter_cfg.unit = unit;
    filter_cfg.index = meter_id;
    filter_cfg.rate = rate;
    SETSOCKOPT(RTDRV_FILTER_RATE_LIMIT_SET, &filter_cfg, rtdrv_filterCfg_t, 1); 

    return RT_ERR_OK;   
}


