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
 * $Revision: 29570 $
 * $Date: 2012-06-05 19:01:47 +0800 (Tue, 05 Jun 2012) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) flow control
 *
 */
 
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_flowctrl_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PORT_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_PORT_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_portPauseForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PORT_PAUSE_FORCE_MODE_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_portPauseForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_PORT_PAUSE_FORCE_MODE_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAction_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    *pAction)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ACTION_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pAction = flowctrl_cfg.pauseOn_action;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAction_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    action)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.pauseOn_action = action;
    SETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ACTION_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAllowedPageNum_get(uint32 unit, rtk_port_t port, uint32 *pPageNum)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PAGENUM_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pPageNum = flowctrl_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAllowedPageNum_set(uint32 unit, rtk_port_t port, uint32 pageNum)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.data = pageNum;
    SETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PAGENUM_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAllowedPktLen_get(uint32 unit, rtk_port_t port, uint32 *pPktLen)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTLEN_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pPktLen = flowctrl_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAllowedPktLen_set(uint32 unit, rtk_port_t port, uint32 pktLen)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.data = pktLen;
    SETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTLEN_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAllowedPktNum_get(uint32 unit, rtk_port_t port, uint32 *pPktNum)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTNUM_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pPktNum = flowctrl_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_pauseOnAllowedPktNum_set(uint32 unit, rtk_port_t port, uint32 pktNum)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.data = pktNum;
    SETSOCKOPT(RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTNUM_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrSystemPauseThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    GETSOCKOPT(RTDRV_FLOWCTRL_FC_ON_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.thresh, sizeof(rtk_flowctrl_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrSystemPauseThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    memcpy(&flowctrl_cfg.thresh, pThresh, sizeof(rtk_flowctrl_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_FC_ON_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrSystemCongestThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    GETSOCKOPT(RTDRV_FLOWCTRL_FC_OFF_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.thresh, sizeof(rtk_flowctrl_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrSystemCongestThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    memcpy(&flowctrl_cfg.thresh, pThresh, sizeof(rtk_flowctrl_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_FC_OFF_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrPortPauseThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PORT_FC_ON_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.thresh, sizeof(rtk_flowctrl_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrPortPauseThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    memcpy(&flowctrl_cfg.thresh, pThresh, sizeof(rtk_flowctrl_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_PORT_FC_ON_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrPauseThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    GETSOCKOPT(RTDRV_FLOWCTRL_GROUP_FC_ON_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.thresh, sizeof(rtk_flowctrl_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrPauseThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    memcpy(&flowctrl_cfg.thresh, pThresh, sizeof(rtk_flowctrl_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_GROUP_FC_ON_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrPortPauseThreshGroupSel_get(uint32 unit, rtk_port_t port, uint32 *pGrp_idx)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PORT_FC_ON_OFF_GROUP_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pGrp_idx = flowctrl_cfg.grp_idx;

    return RT_ERR_OK;    
} 

int32 rtk_flowctrl_igrPortPauseThreshGroupSel_set(uint32 unit, rtk_port_t port, uint32 grp_idx)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.grp_idx = grp_idx;
    SETSOCKOPT(RTDRV_FLOWCTRL_PORT_FC_ON_OFF_GROUP_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
} 

int32 rtk_flowctrl_egrPortDropMode_get(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t *pDropMode)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROPMODE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pDropMode = flowctrl_cfg.egrDropMode;
        
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortDropMode_set(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t dropMode)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.egrDropMode = dropMode;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROPMODE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortDropForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROPFORCEMODE_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
        
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortDropForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROPFORCEMODE_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrPortCongestThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PORT_FC_OFF_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.thresh, sizeof(rtk_flowctrl_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrPortCongestThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    memcpy(&flowctrl_cfg.thresh, pThresh, sizeof(rtk_flowctrl_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_PORT_FC_OFF_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrCongestThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    GETSOCKOPT(RTDRV_FLOWCTRL_GROUP_FC_OFF_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.thresh, sizeof(rtk_flowctrl_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_igrCongestThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    memcpy(&flowctrl_cfg.thresh, pThresh, sizeof(rtk_flowctrl_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_GROUP_FC_OFF_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrSystemDropThresh_get(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_SYSTEM_DROP_THRESH_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.dropThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrSystemDropThresh_set(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    memcpy(&flowctrl_cfg.dropThresh, pThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_SYSTEM_DROP_THRESH_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortDropThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROP_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.dropThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortDropThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    memcpy(&flowctrl_cfg.dropThresh, pThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROP_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortQueueDropThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_THRESH_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.dropThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortQueueDropThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.queue = queue;
    memcpy(&flowctrl_cfg.dropThresh, pThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_THRESH_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortQueueDropEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_QUEUE_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortQueueDropEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_QUEUE_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrQueueDropThresh_get(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_QUEUE_DROP_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.dropThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrQueueDropThresh_set(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.queue = queue;
    memcpy(&flowctrl_cfg.dropThresh, pThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_QUEUE_DROP_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrCpuQueueDropThresh_get(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_CPU_QUEUE_DROP_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    memcpy(pThresh, &flowctrl_cfg.dropThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrCpuQueueDropThresh_set(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.queue = queue;
    memcpy(&flowctrl_cfg.dropThresh, pThresh, sizeof(rtk_flowctrl_drop_thresh_t));
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_CPU_QUEUE_DROP_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortDropRefCongestEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROP_REFCONGEST_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_flowctrl_egrPortDropRefCongestEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROP_REFCONGEST_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrPortDropThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_GROUP_DROP_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pThresh = flowctrl_cfg.dropThresh;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrPortDropThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    flowctrl_cfg.dropThresh = *pThresh;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_GROUP_DROP_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrQueueDropThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_QUEUE_GROUP_DROP_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pThresh = flowctrl_cfg.dropThresh;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrQueueDropThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    flowctrl_cfg.queue = queue;
    flowctrl_cfg.dropThresh = *pThresh;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_QUEUE_GROUP_DROP_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_igrQueueDropEnable_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_DROP_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_igrQueueDropEnable_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.queue = queue;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_DROP_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_igrQueuePauseThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_FC_ON_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pThresh = flowctrl_cfg.dropThresh;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_igrQueuePauseThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    flowctrl_cfg.queue = queue;
    flowctrl_cfg.dropThresh = *pThresh;
    SETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_FC_ON_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_igrQueuePauseDropThreshGroupSel_get(uint32 unit, rtk_port_t port, uint32 *pGrp_idx)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_FC_ON_DROP_GROUP_SEL_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pGrp_idx = flowctrl_cfg.grp_idx;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set(uint32 unit, rtk_port_t port, uint32 grp_idx)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.grp_idx = grp_idx;
    SETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_FC_ON_DROP_GROUP_SEL_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrPortQueueDropThreshGroupSel_get(uint32 unit, rtk_port_t port, uint32 *pGrp_idx)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_GROUP_SEL_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pGrp_idx = flowctrl_cfg.grp_idx;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrPortQueueDropThreshGroupSel_set(uint32 unit, rtk_port_t port, uint32 grp_idx)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.grp_idx = grp_idx;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_GROUP_SEL_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrQueueDropEnable_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_QUEUE_DROP_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrQueueDropEnable_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.queue = queue;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_QUEUE_DROP_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrPortDropEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROP_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_egrPortDropEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_EGR_PORT_DROP_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_igrQueueDropThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    flowctrl_cfg.queue = queue;
    GETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_DROP_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pThresh = flowctrl_cfg.dropThresh;
    
    return RT_ERR_OK;    
} 

int32
rtk_flowctrl_igrQueueDropThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.grp_idx = grp_idx;
    flowctrl_cfg.queue = queue;
    flowctrl_cfg.dropThresh = *pThresh;
    SETSOCKOPT(RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_DROP_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_flowctrl_portHolTrafficDropEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    GETSOCKOPT(RTDRV_FLOWCTRL_PORT_HOL_TRAFFIC_DROP_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK;   
}

int32
rtk_flowctrl_portHolTrafficDropEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.port = port;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_PORT_HOL_TRAFFIC_DROP_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;   
}

int32
rtk_flowctrl_holTrafficTypeDropEnable_get(uint32 unit, rtk_flowctrl_holTrafficType_t type, rtk_enable_t *pEnable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.data = type;
    GETSOCKOPT(RTDRV_FLOWCTRL_HOL_TRAFFIC_TYPE_DROP_ENABLE_GET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
    *pEnable = flowctrl_cfg.enable;
    
    return RT_ERR_OK; 
}

int32
rtk_flowctrl_holTrafficTypeDropEnable_set(uint32 unit, rtk_flowctrl_holTrafficType_t type, rtk_enable_t enable)
{
    rtdrv_flowctrlCfg_t flowctrl_cfg;

    flowctrl_cfg.unit = unit;
    flowctrl_cfg.data = type;
    flowctrl_cfg.enable = enable;
    SETSOCKOPT(RTDRV_FLOWCTRL_HOL_TRAFFIC_TYPE_DROP_ENABLE_SET, &flowctrl_cfg, rtdrv_flowctrlCfg_t, 1);    
         
    return RT_ERR_OK;   
}

