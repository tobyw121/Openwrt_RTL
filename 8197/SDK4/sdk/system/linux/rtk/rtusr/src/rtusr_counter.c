/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 * 
 * $Revision: 25501 $
 * $Date: 2011-11-25 19:49:08 +0800 (Fri, 25 Nov 2011) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) counter
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_stat_global_get(uint32 unit, rtk_stat_global_type_t cntr_idx, uint64 *pCntr)
{
    rtdrv_counterCfg_t counter_cfg;
    
    memset(&counter_cfg, 0, sizeof(rtdrv_counterCfg_t));
    counter_cfg.unit = unit;
    counter_cfg.cntr_idx = cntr_idx;
    GETSOCKOPT(RTDRV_COUNTER_GLOBAL_GET, &counter_cfg, rtdrv_counterCfg_t, 1);
    memcpy(pCntr, &counter_cfg.cntr, sizeof(uint64));
    
    return RT_ERR_OK;    
}

int32 rtk_stat_global_getAll(uint32 unit, rtk_stat_global_cntr_t *pGlobal_cntrs)
{
    rtdrv_counterCfg_t counter_cfg;
    
    memset(&counter_cfg, 0, sizeof(rtdrv_counterCfg_t));
    counter_cfg.unit = unit;
    GETSOCKOPT(RTDRV_COUNTER_GLOBAL_GETALL, &counter_cfg, rtdrv_counterCfg_t, 1);
    memcpy(pGlobal_cntrs, &counter_cfg.global_cnt, sizeof(rtk_stat_global_cntr_t));
    
    return RT_ERR_OK;    
}

int32 rtk_stat_global_reset(uint32 unit)
{
    rtdrv_counterCfg_t counter_cfg;

    counter_cfg.unit = unit;
    SETSOCKOPT(RTDRV_COUNTER_GLOBAL_RESET, &counter_cfg, rtdrv_counterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_stat_port_get(uint32 unit, rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr)
{
    rtdrv_counterCfg_t counter_cfg;
    
    memset(&counter_cfg, 0, sizeof(rtdrv_counterCfg_t));
    counter_cfg.unit = unit;
    counter_cfg.port = port;
    counter_cfg.cntr_idx = cntr_idx;
    GETSOCKOPT(RTDRV_COUNTER_PORT_GET, &counter_cfg, rtdrv_counterCfg_t, 1);
    memcpy(pCntr, &counter_cfg.cntr, sizeof(uint64));
    
    return RT_ERR_OK;    
}

int32 rtk_stat_port_getAll(uint32 unit, rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs)
{
    rtdrv_counterCfg_t counter_cfg;
    
    memset(&counter_cfg, 0, sizeof(rtdrv_counterCfg_t));
    counter_cfg.unit = unit;
    counter_cfg.port = port;
    GETSOCKOPT(RTDRV_COUNTER_PORT_GETALL, &counter_cfg, rtdrv_counterCfg_t, 1);
    memcpy(pPort_cntrs, &counter_cfg.port_cnt, sizeof(rtk_stat_port_cntr_t));
    
    return RT_ERR_OK;    
}

int32 rtk_stat_port_reset(uint32 unit, rtk_port_t port)
{
    rtdrv_counterCfg_t counter_cfg;

    counter_cfg.unit = unit;
    counter_cfg.port = port;
    SETSOCKOPT(RTDRV_COUNTER_PORT_RESET, &counter_cfg, rtdrv_counterCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_stat_smon_get(uint32 unit, rtk_pri_t pri, rtk_stat_smon_type_t cntr_idx,  uint64 *pCntr)
{
    rtdrv_counterCfg_t counter_cfg;
    
    memset(&counter_cfg, 0, sizeof(rtdrv_counterCfg_t));
    counter_cfg.unit = unit;
    counter_cfg.pri = pri;
    counter_cfg.cntr_idx = cntr_idx;
    GETSOCKOPT(RTDRV_COUNTER_SMON_GET, &counter_cfg, rtdrv_counterCfg_t, 1);
    memcpy(pCntr, &counter_cfg.cntr, sizeof(uint64));
    
    return RT_ERR_OK;    
} /* end of rtk_stat_smon_get */

int32 rtk_stat_smon_getAll(uint32 unit, rtk_pri_t pri, rtk_stat_smon_cntr_t *pCntr)
{
    rtdrv_counterCfg_t counter_cfg;
    
    memset(&counter_cfg, 0, sizeof(rtdrv_counterCfg_t));
    counter_cfg.unit = unit;
    counter_cfg.pri = pri;
    GETSOCKOPT(RTDRV_COUNTER_SMON_GETALL, &counter_cfg, rtdrv_counterCfg_t, 1);
    memcpy(pCntr, &counter_cfg.smon_cnt, sizeof(rtk_stat_smon_cntr_t));
    
    return RT_ERR_OK;    
} /* end of rtk_stat_smon_getAll */


int32 rtk_stat_tagLenCntIncEnable_get(uint32 unit, rtk_stat_tagCnt_type_t tagCnt_type, rtk_enable_t *pEnable)
{
    rtdrv_counterCfg_t counter_cfg;
    
    counter_cfg.unit = unit;
    counter_cfg.tagCnt_type = tagCnt_type;
    GETSOCKOPT(RTDRV_COUNTER_TAGLENCNT_GET, &counter_cfg, rtdrv_counterCfg_t, 1);
    *pEnable = counter_cfg.enable;
    
    return RT_ERR_OK;    
} /* end of rtk_stat_tagLenCntIncEnable_get */

int32 rtk_stat_tagLenCntIncEnable_set(uint32 unit, rtk_stat_tagCnt_type_t tagCnt_type, rtk_enable_t enable)
{
    rtdrv_counterCfg_t counter_cfg;
    
    counter_cfg.unit = unit;
    counter_cfg.tagCnt_type = tagCnt_type;
    counter_cfg.enable = enable;
    SETSOCKOPT(RTDRV_COUNTER_TAGLENCNT_SET, &counter_cfg, rtdrv_counterCfg_t, 1);
    
    return RT_ERR_OK;    
} /* end of rtk_stat_tagLenCntIncEnable_set */
