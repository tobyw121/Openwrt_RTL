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
 * $Revision: 21987 $
 * $Date: 2011-09-05 16:53:49 +0800 (Mon, 05 Sep 2011) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) mirror
 *
 */
 
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>


int32 rtk_mirror_portBased_get(uint32 unit, uint32 mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirroring_port = mirroring_port;   
    GETSOCKOPT(RTDRV_MIRROR_ENTRY_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    memcpy(pMirrored_rx_portmask, &mirror_cfg.rx_portmask, sizeof(rtk_portmask_t));
    memcpy(pMirrored_tx_portmask, &mirror_cfg.tx_portmask, sizeof(rtk_portmask_t));
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_portBased_set(uint32 unit, uint32 mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirroring_port = mirroring_port;    
    memcpy(&mirror_cfg.rx_portmask, pMirrored_rx_portmask, sizeof(rtk_portmask_t));
    memcpy(&mirror_cfg.tx_portmask, pMirrored_tx_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_MIRROR_ENTRY_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_portBased_create(uint32 unit, uint32 mirroring_port)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirroring_port = mirroring_port;    
    SETSOCKOPT(RTDRV_MIRROR_ENTRY_CREATE, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_portBased_destroy(uint32 unit, uint32 mirroring_port)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirroring_port = mirroring_port;    
    SETSOCKOPT(RTDRV_MIRROR_ENTRY_DESTROY, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_portBased_destroyAll(uint32 unit)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    SETSOCKOPT(RTDRV_MIRROR_ENTRY_DESTROYALL, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_group_init(
    uint32              unit,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    GETSOCKOPT(RTDRV_MIRROR_GROUP_INIT, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    memcpy(pMirrorEntry, &(mirror_cfg.mirrorEntry), sizeof(rtk_mirror_entry_t));
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_group_get(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit         = unit;
    mirror_cfg.mirror_id    = mirror_id;
    GETSOCKOPT(RTDRV_MIRROR_GROUP_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    memcpy(pMirrorEntry, &(mirror_cfg.mirrorEntry), sizeof(rtk_mirror_entry_t));
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_group_set(
    uint32              unit,
    uint32              mirror_id,
    rtk_mirror_entry_t  *pMirrorEntry)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirror_id = mirror_id;    
    memcpy(&mirror_cfg.mirrorEntry, pMirrorEntry, sizeof(rtk_mirror_entry_t));
    SETSOCKOPT(RTDRV_MIRROR_GROUP_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_egrMode_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    *pEgrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.port    = port;
    GETSOCKOPT(RTDRV_MIRROR_EGR_MODE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pEgrMode = mirror_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_egrMode_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_mirror_egrMode_t    egrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;    
    mirror_cfg.data = egrMode;
    SETSOCKOPT(RTDRV_MIRROR_EGR_MODE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_portRspanIgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t *pIgrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.port    = port;
    GETSOCKOPT(RTDRV_MIRROR_PORT_RSPAN_IGR_MODE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pIgrMode = mirror_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_portRspanIgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrMode_t igrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;    
    mirror_cfg.data = igrMode;
    SETSOCKOPT(RTDRV_MIRROR_PORT_RSPAN_IGR_MODE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_portRspanEgrMode_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t *pEgrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.port    = port;
    GETSOCKOPT(RTDRV_MIRROR_PORT_RSPAN_EGR_MODE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pEgrMode = mirror_cfg.data;
    
    return RT_ERR_OK;    
}


int32 rtk_mirror_portRspanEgrMode_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrMode_t egrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;    
    mirror_cfg.data = egrMode;
    SETSOCKOPT(RTDRV_MIRROR_PORT_RSPAN_EGR_MODE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_rspanIgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t *pIgrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.mirror_id    = mirror_id;
    GETSOCKOPT(RTDRV_MIRROR_RSPAN_IGR_MODE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);
    *pIgrMode = mirror_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_mirror_rspanIgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanIgrMode_t igrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirror_id = mirror_id;
    mirror_cfg.data = igrMode;
    SETSOCKOPT(RTDRV_MIRROR_RSPAN_IGR_MODE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_mirror_rspanEgrMode_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t *pEgrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.mirror_id    = mirror_id;
    GETSOCKOPT(RTDRV_MIRROR_RSPAN_EGR_MODE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);
    *pEgrMode = mirror_cfg.data;

    return RT_ERR_OK;
}


int32 rtk_mirror_rspanEgrMode_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanEgrMode_t egrMode)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirror_id = mirror_id;
    mirror_cfg.data = egrMode;
    SETSOCKOPT(RTDRV_MIRROR_RSPAN_EGR_MODE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_mirror_rspanIgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.port    = port;
    GETSOCKOPT(RTDRV_MIRROR_RSPAN_IGR_TAG_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    memcpy(pIgrTag, &(mirror_cfg.rspan_igrTag), sizeof(rtk_mirror_rspanIgrTag_t));
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_rspanIgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanIgrTag_t *pIgrTag)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;    
    memcpy(&(mirror_cfg.rspan_igrTag), pIgrTag, sizeof(rtk_mirror_rspanIgrTag_t));
    SETSOCKOPT(RTDRV_MIRROR_RSPAN_IGR_TAG_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_rspanEgrTag_get(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.port    = port;
    GETSOCKOPT(RTDRV_MIRROR_RSPAN_EGR_TAG_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    memcpy(pEgrTag, &(mirror_cfg.rspan_egrTag), sizeof(rtk_mirror_rspanEgrTag_t));
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_rspanEgrTag_set(uint32 unit, rtk_port_t port, rtk_mirror_rspanEgrTag_t *pEgrTag)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;    
    memcpy(&(mirror_cfg.rspan_egrTag), pEgrTag, sizeof(rtk_mirror_rspanEgrTag_t));
    SETSOCKOPT(RTDRV_MIRROR_RSPAN_EGR_TAG_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_rspanTag_get(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.mirror_id = mirror_id;
    GETSOCKOPT(RTDRV_MIRROR_RSPAN_TAG_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);
    memcpy(pTag, &(mirror_cfg.rspan_tag), sizeof(rtk_mirror_rspanTag_t));

    return RT_ERR_OK;
}

int32 rtk_mirror_rspanTag_set(uint32 unit, uint32 mirror_id, rtk_mirror_rspanTag_t *pTag)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirror_id = mirror_id;
    memcpy(&(mirror_cfg.rspan_tag), pTag, sizeof(rtk_mirror_rspanTag_t));
    SETSOCKOPT(RTDRV_MIRROR_RSPAN_TAG_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_mirror_sflowMirrorSeed_get(uint32 unit, uint32 *pSeed)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_MIRROR_SEED_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pSeed = mirror_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowMirrorSeed_set(uint32 unit, uint32 seed)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.data = seed;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_MIRROR_SEED_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowMirrorSampleEnable_get(uint32 unit, uint32 mirror_id, rtk_enable_t *pEnable)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.mirror_id    = mirror_id;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_ENABLE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pEnable = mirror_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowMirrorSampleEnable_set(uint32 unit, uint32 mirror_id, rtk_enable_t enable)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirror_id    = mirror_id;
    mirror_cfg.enable = enable;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_ENABLE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowMirrorSampleRate_get(uint32 unit, uint32 mirror_id, uint32 *pRate)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.mirror_id    = mirror_id;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_RATE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pRate = mirror_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowMirrorSampleRate_set(uint32 unit, uint32 mirror_id, uint32 rate)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.mirror_id    = mirror_id;
    mirror_cfg.data = rate;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_RATE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowMirrorSampleStat_get(uint32 unit, uint32 mirror_id, rtk_mirror_sampleStat_t *pStat)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    mirror_cfg.mirror_id    = mirror_id;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_STAT_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    memcpy(pStat, &mirror_cfg.sample_stat, sizeof(rtk_mirror_sampleStat_t));
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortSeed_get(uint32 unit, uint32 *pSeed)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit    = unit;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_SEED_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pSeed = mirror_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortSeed_set(uint32 unit, uint32 seed)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.data = seed;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_SEED_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortIgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit     = unit;
    mirror_cfg.port     = port;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_ENABLE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pEnable = mirror_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortIgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;
    mirror_cfg.enable = enable;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_ENABLE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortIgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit     = unit;
    mirror_cfg.port     = port;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_RATE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pRate = mirror_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortIgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;
    mirror_cfg.data = rate;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_RATE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortEgrSampleEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit     = unit;
    mirror_cfg.port     = port;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_ENABLE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pEnable = mirror_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortEgrSampleEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;
    mirror_cfg.enable = enable;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_ENABLE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortEgrSampleRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit     = unit;
    mirror_cfg.port     = port;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_RATE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pRate = mirror_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowPortEgrSampleRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.port = port;
    mirror_cfg.data = rate;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_RATE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_mirror_sflowAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit     = unit;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_ADD_CPU_TAG_ENABLE_GET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
    *pEnable = mirror_cfg.enable;
    
    return RT_ERR_OK;    
}


int32 rtk_mirror_sflowAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)

{
    rtdrv_mirrorCfg_t mirror_cfg;

    mirror_cfg.unit = unit;
    mirror_cfg.enable = enable;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_ADD_CPU_TAG_ENABLE_SET, &mirror_cfg, rtdrv_mirrorCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_mirror_sflowSampleCtrl_get(uint32 unit, rtk_sflowSampleCtrl_t *pCtrl)
{
    rtdrv_mirrorCfg_t cfg;

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_MIRROR_SFLOW_SAMPLE_CTRL_GET, &cfg, rtdrv_mirrorCfg_t, 1);
    *pCtrl = cfg.sample_ctrl;

    return RT_ERR_OK;
}    /* end of rtk_mirror_sflowSampleCtrl_get */

int32
rtk_mirror_sflowSampleCtrl_set(uint32 unit, rtk_sflowSampleCtrl_t ctrl)
{
    rtdrv_mirrorCfg_t cfg;

    cfg.unit        = unit;
    cfg.sample_ctrl = ctrl;
    SETSOCKOPT(RTDRV_MIRROR_SFLOW_SAMPLE_CTRL_SET, &cfg, rtdrv_mirrorCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_mirror_sflowSampleCtrl_set */
