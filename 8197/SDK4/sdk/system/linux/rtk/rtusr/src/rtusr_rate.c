/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 41781 $
 * $Date: 2013-08-02 18:01:22 +0800 (Fri, 02 Aug 2013) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) rate
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_rate_igrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;   
    GETSOCKOPT(RTDRV_RATE_IGR_ENABLE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.enable = enable;
    SETSOCKOPT(RTDRV_RATE_IGR_ENABLE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    GETSOCKOPT(RTDRV_RATE_IGR_INCLUDE_IFG_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIfg_include = rate_cfg.ifg_include;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.ifg_include = ifg_include;
    SETSOCKOPT(RTDRV_RATE_IGR_INCLUDE_IFG_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_IGR_RATE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate = rate_cfg.rate;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.rate = rate;
    SETSOCKOPT(RTDRV_RATE_IGR_RATE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_portIgrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIfg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_PORT_IGR_INCLUDE_IFG_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIfg_include = rate_cfg.ifg_include;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_port_t port, rtk_enable_t ifg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.ifg_include = ifg_include;
    SETSOCKOPT(RTDRV_RATE_PORT_IGR_INCLUDE_IFG_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthFlowctrlEnable_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_ENABLE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.ifg_include;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthFlowctrlEnable_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.ifg_include = enable;
    SETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_ENABLE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthFlowctrlThresh_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_THRESH_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    memcpy(pThresh, &(rate_cfg.rate_thresh), sizeof(rtk_rate_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthFlowctrlThresh_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    memcpy(&(rate_cfg.rate_thresh), pThresh, sizeof(rtk_rate_thresh_t));
    SETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_THRESH_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlFPEntry_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t *pFpEntry)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.index = fpIndex;
    GETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_CTRL_FPENTRY_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    memcpy(pFpEntry, &(rate_cfg.fpEntry), sizeof(rtk_rate_igr_fpEntry_t));
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlFPEntry_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t *pFpEntry)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.index = fpIndex;
    memcpy(&(rate_cfg.fpEntry), pFpEntry, sizeof(rtk_rate_igr_fpEntry_t));
    SETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_CTRL_FPENTRY_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlBypass_get(uint32 unit, rtk_rate_igr_bypassType_t bypassType, rtk_enable_t *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.igrBypassType = bypassType;
    GETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_CTRL_BYPASS_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthCtrlBypass_set(uint32 unit, rtk_rate_igr_bypassType_t bypassType, rtk_enable_t enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.igrBypassType = bypassType;
    rate_cfg.enable = enable;
    SETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_CTRL_BYPASS_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthLowThresh_get(uint32 unit, uint32 *pLowThresh)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    GETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_LOW_THRESH_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pLowThresh = rate_cfg.thresh;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBandwidthLowThresh_set(uint32 unit, uint32 lowThresh)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.thresh = lowThresh;
    SETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_LOW_THRESH_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_rate_portIgrBandwidthCtrlExceed_get(
    uint32                  unit, 
    rtk_port_t              port, 
    uint32                  *pIsExceed)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_PORT_IGR_BANDWIDTH_CTRL_EXCEED_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIsExceed = rate_cfg.isExceed;
    
    return RT_ERR_OK; 
}

int32
rtk_rate_portIgrBandwidthCtrlExceed_reset(
    uint32                  unit,
    rtk_port_t              port)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    
    SETSOCKOPT(RTDRV_RATE_PORT_IGR_BANDWIDTH_CTRL_EXCEED_RESET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    
    return RT_ERR_OK;
}

int32 rtk_rate_portIgrBandwidthHighThresh_get(uint32 unit, rtk_port_t port, uint32 *pHighThresh)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_PORT_IGR_BANDWIDTH_FLOWCTRL_HIGH_THRESH_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pHighThresh = rate_cfg.thresh;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_portIgrBandwidthHighThresh_set(uint32 unit, rtk_port_t port, uint32 highThresh)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.thresh = highThresh;
    SETSOCKOPT(RTDRV_RATE_PORT_IGR_BANDWIDTH_FLOWCTRL_HIGH_THRESH_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBwCtrlBurstSize_get(uint32 unit, uint32 *pBurst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    GETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_BURST_SIZE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pBurst_size = rate_cfg.thresh;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_igrBwCtrlBurstSize_set(uint32 unit, uint32 burst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.thresh = burst_size;
    SETSOCKOPT(RTDRV_RATE_IGR_BANDWIDTH_BURST_SIZE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_egrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;   
    GETSOCKOPT(RTDRV_RATE_EGR_ENABLE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_egrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.enable = enable;
    SETSOCKOPT(RTDRV_RATE_EGR_ENABLE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_egrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    GETSOCKOPT(RTDRV_RATE_EGR_INCLUDE_IFG_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIfg_include = rate_cfg.ifg_include;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_egrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.ifg_include = ifg_include;
    SETSOCKOPT(RTDRV_RATE_EGR_INCLUDE_IFG_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_cpuEgrBandwidthCtrlRateMode_get(uint32 unit, rtk_rate_rateMode_t *pRate_mode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    GETSOCKOPT(RTDRV_RATE_CPU_EGR_BANDWIDTH_CTRL_RATE_MODE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate_mode = rate_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_cpuEgrBandwidthCtrlRateMode_set(uint32 unit, rtk_rate_rateMode_t rate_mode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.data = rate_mode;
    SETSOCKOPT(RTDRV_RATE_CPU_EGR_BANDWIDTH_CTRL_RATE_MODE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_egrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_EGR_RATE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate = rate_cfg.rate;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_egrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.rate = rate;
    SETSOCKOPT(RTDRV_RATE_EGR_RATE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_portEgrBandwidthCtrlIncludeIfg_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pIfg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_PORT_EGR_INCLUDE_IFG_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIfg_include = rate_cfg.ifg_include;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_portEgrBandwidthCtrlIncludeIfg_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    ifg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.ifg_include = ifg_include;
    SETSOCKOPT(RTDRV_RATE_PORT_EGR_INCLUDE_IFG_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_rate_portEgrBandwidthCtrlBurstSize_get(uint32 unit, rtk_port_t port, uint32 *pBurst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    GETSOCKOPT(RTDRV_RATE_PORT_EGR_BURST_SIZE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pBurst_size = rate_cfg.data;
    
    return RT_ERR_OK;
}

int32
rtk_rate_portEgrBandwidthCtrlBurstSize_set(uint32 unit, rtk_port_t port, uint32 burst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.data = burst_size;
    SETSOCKOPT(RTDRV_RATE_PORT_EGR_BURST_SIZE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_egrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    GETSOCKOPT(RTDRV_RATE_EGR_QUEUE_BWCTRL_ENABLE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_egrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    rate_cfg.enable = enable;
    SETSOCKOPT(RTDRV_RATE_EGR_QUEUE_BWCTRL_ENABLE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_egrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    GETSOCKOPT(RTDRV_RATE_EGR_QUEUE_BWCTRL_RATE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate = rate_cfg.rate;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_egrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    rate_cfg.rate = rate;
    SETSOCKOPT(RTDRV_RATE_EGR_QUEUE_BWCTRL_RATE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_egrQueueFixedBandwidthEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;
    
    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue   = queue;
    GETSOCKOPT(RTDRV_RATE_EGR_QUEUE_FIX_BW_ENABLE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);
    *pEnable    = rate_cfg.enable;
        
    return RT_ERR_OK;    
}

int32 rtk_rate_egrQueueFixedBandwidthEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue   = queue;
    rate_cfg.enable  = enable;
    SETSOCKOPT(RTDRV_RATE_EGR_QUEUE_FIX_BW_ENABLE_SET, &rate_cfg, rtdrv_rateCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_rate_egrPortQueueBwCtrlBurstSize_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pBurst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    GETSOCKOPT(RTDRV_RATE_EGR_PORT_QUEUE_BWCTRL_BURST_SIZE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pBurst_size = rate_cfg.data;
    
    return RT_ERR_OK;    
}

int32
rtk_rate_egrPortQueueBwCtrlBurstSize_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      burst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    rate_cfg.data = burst_size;
    SETSOCKOPT(RTDRV_RATE_EGR_PORT_QUEUE_BWCTRL_BURST_SIZE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_rate_egrQueueBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    GETSOCKOPT(RTDRV_RATE_EGR_QUEUE_BWCTRL_BURST_SIZE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pBurst_size = rate_cfg.data;
    
    return RT_ERR_OK;    
}

int32
rtk_rate_egrQueueBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.data = burst_size;
    SETSOCKOPT(RTDRV_RATE_EGR_QUEUE_BWCTRL_BURST_SIZE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}


int32 rtk_rate_stormControlRate_get(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type, uint32 *pRate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    GETSOCKOPT(RTDRV_RATE_STROM_CTRL_RATE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate = rate_cfg.rate;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlRate_set(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type, uint32 rate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    rate_cfg.rate = rate;
    SETSOCKOPT(RTDRV_RATE_STROM_CTRL_RATE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlProtoRate_get(uint32 unit, rtk_port_t port, rtk_rate_storm_proto_group_t storm_type, uint32 *pRate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_proto_type = storm_type;
    GETSOCKOPT(RTDRV_RATE_STROM_CTRL_PROTO_RATE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate = rate_cfg.rate;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlProtoRate_set(uint32 unit, rtk_port_t port, rtk_rate_storm_proto_group_t storm_type, uint32 rate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_proto_type = storm_type;
    rate_cfg.rate = rate;
    SETSOCKOPT(RTDRV_RATE_STROM_CTRL_PROTO_RATE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_ENABLE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    rate_cfg.enable = enable;
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_ENABLE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_portStormControlRateMode_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   *pRate_mode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    
    GETSOCKOPT(RTDRV_RATE_PORT_STORM_CONTROL_RATE_MODE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate_mode = rate_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_portStormControlRateMode_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   rate_mode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    rate_cfg.data = rate_mode;
    SETSOCKOPT(RTDRV_RATE_PORT_STORM_CONTROL_RATE_MODE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlRateMode_get(
    uint32                      unit,
    rtk_rate_storm_rateMode_t   *pRate_mode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
   
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_RATE_MODE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate_mode = rate_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlRateMode_set(
    uint32                      unit,
    rtk_rate_storm_rateMode_t   rate_mode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.data = rate_mode;
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_RATE_MODE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_portStormControlBurstSize_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pBurst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_BURST_RATE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pBurst_size = rate_cfg.rate;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_portStormControlBurstSize_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  burst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    rate_cfg.rate = burst_size;

    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_BURST_RATE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_INCLUDE_IFG_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIfg_include = rate_cfg.ifg_include;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.ifg_include = ifg_include;
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_INCLUDE_IFG_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlExceed_get(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type, uint32 *pIsExceed)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_EXCEED_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIsExceed = rate_cfg.isExceed;
    
    return RT_ERR_OK;
}

int32 rtk_rate_stormControlExceed_reset(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_EXCEED_RESET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    
    return RT_ERR_OK;
}

int32 rtk_rate_stormControlProtoExceed_get(uint32 unit, rtk_port_t port, rtk_rate_storm_proto_group_t storm_type, uint32 *pIsExceed)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_proto_type = storm_type;
    
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_PROTO_EXCEED_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIsExceed = rate_cfg.isExceed;
    
    return RT_ERR_OK;
}

int32 rtk_rate_stormControlProtoExceed_reset(uint32 unit, rtk_port_t port, rtk_rate_storm_proto_group_t storm_type)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_proto_type = storm_type;
    
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_PROTO_EXCEED_RESET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    
    return RT_ERR_OK;
}

int32 rtk_rate_stormControlRefreshMode_get(uint32 unit, rtk_rate_storm_rateMode_t *pMode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_REFRESH_MODE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pMode = rate_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlRefreshMode_set(uint32 unit, rtk_rate_storm_rateMode_t mode)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.data = mode;
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_REFRESH_MODE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlTypeSel_get(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_storm_sel_t *pStorm_sel)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_TYPE_SEL_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pStorm_sel = rate_cfg.storm_sel;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlTypeSel_set(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_storm_sel_t storm_sel)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.storm_type = storm_type;
    rate_cfg.storm_sel = storm_sel;
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_TYPE_SEL_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlBypass_get(uint32 unit, rtk_rate_storm_bypassType_t bypassType, rtk_enable_t *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.stormBypassType = bypassType;
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_BYPASS_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_rate_stormControlBypass_set(uint32 unit, rtk_rate_storm_bypassType_t bypassType, rtk_enable_t enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.stormBypassType = bypassType;
    rate_cfg.enable = enable;
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_BYPASS_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_rate_stormControlBurstSize_get(
    uint32                  unit,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pBurst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.storm_type = storm_type;
    GETSOCKOPT(RTDRV_RATE_STORM_CONTROL_BURST_SIZE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pBurst_size = rate_cfg.data;
    
    return RT_ERR_OK;    
} 

int32
rtk_rate_stormControlBurstSize_set(
    uint32                  unit,
    rtk_rate_storm_group_t  storm_type,
    uint32                  burst_size)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.storm_type = storm_type;
    rate_cfg.data = burst_size;
    SETSOCKOPT(RTDRV_RATE_STORM_CONTROL_BURST_SIZE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    

    return RT_ERR_OK;    
} 

int32
rtk_rate_igrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    GETSOCKOPT(RTDRV_RATE_IGR_QUEUE_BWCTRL_ENABLE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pEnable = rate_cfg.enable;
    
    return RT_ERR_OK; 
} 

int32
rtk_rate_igrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    rate_cfg.enable = enable;
    SETSOCKOPT(RTDRV_RATE_IGR_QUEUE_BWCTRL_ENABLE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    

    return RT_ERR_OK;    
} 

int32
rtk_rate_igrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    GETSOCKOPT(RTDRV_RATE_IGR_QUEUE_BWCTRL_RATE_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pRate = rate_cfg.rate;
    
    return RT_ERR_OK;   
} 

int32
rtk_rate_igrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    rate_cfg.rate = rate;
    SETSOCKOPT(RTDRV_RATE_IGR_QUEUE_BWCTRL_RATE_SET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    
    return RT_ERR_OK;   
} 

int32 
rtk_rate_igrQueueBwCtrlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qid_t               queue,
    uint32                  *pIsExceed)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    GETSOCKOPT(RTDRV_RATE_IGR_QUEUE_BWCTRL_EXCEED_GET, &rate_cfg, rtdrv_rateCfg_t, 1);    
    *pIsExceed = rate_cfg.isExceed;
    
    return RT_ERR_OK;   
} 

int32 
rtk_rate_igrQueueBwCtrlExceed_reset(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qid_t               queue)
{
    rtdrv_rateCfg_t rate_cfg;

    rate_cfg.unit = unit;
    rate_cfg.port = port;
    rate_cfg.queue = queue;
    SETSOCKOPT(RTDRV_RATE_IGR_QUEUE_BWCTRL_EXCEED_RESET, &rate_cfg, rtdrv_rateCfg_t, 1);    

    return RT_ERR_OK;    
} 



