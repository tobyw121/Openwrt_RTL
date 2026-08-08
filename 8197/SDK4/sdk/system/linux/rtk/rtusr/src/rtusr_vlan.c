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
 * $Revision: 52260 $
 * $Date: 2014-10-17 14:14:26 +0800 (Fri, 17 Oct 2014) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) vlan
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_vlan_port_add(uint32 unit, rtk_vlan_t vid, rtk_port_t port, uint32 is_untag)
{
    rtdrv_vlan_port_t vlan_port;

    vlan_port.unit = unit;
    vlan_port.vid = vid;
    vlan_port.port = port;
    vlan_port.is_untag = is_untag;
    SETSOCKOPT(RTDRV_VLAN_PORT_ADD, &vlan_port, rtdrv_vlan_port_t, 1);    

    return RT_ERR_OK;    
}

int32 rtk_vlan_port_get(uint32 unit, rtk_vlan_t vid, rtk_portmask_t *pMember_portmask, rtk_portmask_t *pUntag_portmask)
{
    rtdrv_vlan_port_t vlan_port;

    vlan_port.unit = unit;
    vlan_port.vid = vid;   
    GETSOCKOPT(RTDRV_VLAN_PORT_GET, &vlan_port, rtdrv_vlan_port_t, 1);    
    memcpy(pMember_portmask, &vlan_port.member, sizeof(rtk_portmask_t));
    memcpy(pUntag_portmask, &vlan_port.untag, sizeof(rtk_portmask_t));
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_port_set(uint32 unit, rtk_vlan_t vid, rtk_portmask_t *pMember_portmask, rtk_portmask_t *pUntag_portmask)
{
    rtdrv_vlan_port_t vlan_port;

    vlan_port.unit = unit;
    vlan_port.vid = vid;   
    memcpy(&vlan_port.member, pMember_portmask, sizeof(rtk_portmask_t));
    memcpy(&vlan_port.untag, pUntag_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_VLAN_PORT_SET, &vlan_port, rtdrv_vlan_port_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_vlan_port_del(uint32 unit, rtk_vlan_t vid, rtk_port_t port)
{
    rtdrv_vlan_port_t vlan_port;

    vlan_port.unit = unit;
    vlan_port.vid = vid;
    vlan_port.port = port;
    SETSOCKOPT(RTDRV_VLAN_PORT_DEL, &vlan_port, rtdrv_vlan_port_t, 1);    

    return RT_ERR_OK;    
}

int32 rtk_vlan_portPvid_get(uint32 unit, rtk_port_t port, uint32 *pPvid)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_PVID_GET, &port_cfg, rtdrv_portCfg_t, 1);    
    *pPvid = port_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portPvid_set(uint32 unit, rtk_port_t port, uint32 pvid)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    port_cfg.data = pvid;
    SETSOCKOPT(RTDRV_VLAN_PORT_PVID_SET, &port_cfg, rtdrv_portCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_igrFilterEnable_get(uint32 unit, rtk_enable_t *pIgr_filter)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    GETSOCKOPT(RTDRV_VLAN_EN_IGR_FILTER_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pIgr_filter = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_igrFilterEnable_set(uint32 unit, rtk_enable_t igr_filter)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = igr_filter;
    SETSOCKOPT(RTDRV_VLAN_EN_IGR_FILTER_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIgr_filter)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_EN_PORT_IGR_FILTER_GET, &port_cfg, rtdrv_portCfg_t, 1);    
    *pIgr_filter = port_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t igr_filter)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    port_cfg.data = igr_filter;
    SETSOCKOPT(RTDRV_VLAN_EN_PORT_IGR_FILTER_SET, &port_cfg, rtdrv_portCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_tagMode_get(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_TAG_MODE_GET, &port_cfg, rtdrv_portCfg_t, 1);    
    *pTag_mode = port_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_tagMode_set(uint32 unit, rtk_port_t port, rtk_vlan_tagMode_t tag_mode)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    port_cfg.data = tag_mode;
    SETSOCKOPT(RTDRV_VLAN_TAG_MODE_SET, &port_cfg, rtdrv_portCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portAcceptFrameType_get(uint32 unit, rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_ACCEPT_FRAME_TYPE_GET, &port_cfg, rtdrv_portCfg_t, 1);    
    *pAccept_frame_type = port_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portAcceptFrameType_set(uint32 unit, rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    port_cfg.data = accept_frame_type;
    SETSOCKOPT(RTDRV_VLAN_PORT_ACCEPT_FRAME_TYPE_SET, &port_cfg, rtdrv_portCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portOuterAcceptFrameType_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  *pAccept_frame_type)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_OUTER_ACCEPT_FRAME_TYPE_GET, &port_cfg, rtdrv_portCfg_t, 1);    
    *pAccept_frame_type = port_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portOuterAcceptFrameType_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_vlan_acceptFrameType_t  accept_frame_type)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    port_cfg.data = accept_frame_type;
    SETSOCKOPT(RTDRV_VLAN_PORT_OUTER_ACCEPT_FRAME_TYPE_SET, &port_cfg, rtdrv_portCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrFilterEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_FILTER_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrFilterEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_FILTER_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_mcastLeakyEnable_get(uint32 unit, rtk_enable_t *pLeaky)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    GETSOCKOPT(RTDRV_VLAN_EN_MCAST_LEAKY_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    *pLeaky = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_mcastLeakyEnable_set(uint32 unit, rtk_enable_t leaky)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    unit_cfg.data = leaky;
    SETSOCKOPT(RTDRV_VLAN_EN_MCAST_LEAKY_SET, &unit_cfg, rtdrv_unitCfg_t, 1);
   
    return RT_ERR_OK;    
}

int32 rtk_vlan_mcastLeakyPortEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pLeaky)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_EN_PORT_MCAST_LEAKY_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pLeaky = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_mcastLeakyPortEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t leaky)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = leaky;
    SETSOCKOPT(RTDRV_VLAN_EN_PORT_MCAST_LEAKY_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_stg_get(uint32 unit, rtk_vlan_t vid, rtk_stg_t *pStg)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;   
    GETSOCKOPT(RTDRV_VLAN_STG_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pStg = vlan_cfg.data;    
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_stg_set(uint32 unit, rtk_vlan_t vid, rtk_stg_t stg)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;  
    vlan_cfg.data = stg;  
    SETSOCKOPT(RTDRV_VLAN_STG_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
          
    return RT_ERR_OK;    
}

int32 rtk_vlan_fid_get(uint32 unit, rtk_vlan_t vid, rtk_fid_t *pFid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;   
    GETSOCKOPT(RTDRV_VLAN_FID_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pFid = vlan_cfg.data;    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_fid_set(uint32 unit, rtk_vlan_t vid, rtk_fid_t fid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;  
    vlan_cfg.data = fid;  
    SETSOCKOPT(RTDRV_VLAN_FID_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_create(uint32 unit, rtk_vlan_t vid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;        
    SETSOCKOPT(RTDRV_VLAN_CREATE, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_destroy(uint32 unit, rtk_vlan_t vid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;        
    SETSOCKOPT(RTDRV_VLAN_DESTROY, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_destroyAll(uint32 unit, uint32 restore_default_vlan)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    unit_cfg.data = restore_default_vlan;
    SETSOCKOPT(RTDRV_VLAN_DESTROY_ALL, &unit_cfg, rtdrv_unitCfg_t, 1);

    return RT_ERR_OK;    
}

int32 rtk_vlan_vlanFunctionEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    GETSOCKOPT(RTDRV_VLAN_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_vlanFunctionEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = enable;  
    SETSOCKOPT(RTDRV_VLAN_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portOuterPvid_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pPvid)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_OUTER_PVID_GET, &port_cfg, rtdrv_portCfg_t, 1);    
    *pPvid = port_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portOuterPvid_set(uint32 unit, rtk_port_t port, rtk_vlan_t pvid)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;   
    port_cfg.data = pvid;
    SETSOCKOPT(RTDRV_VLAN_PORT_OUTER_PVID_SET, &port_cfg, rtdrv_portCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_protoGroup_get(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = protoGroup_idx;   
    GETSOCKOPT(RTDRV_VLAN_PROTO_GROUP_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    memcpy(pProtoGroup, &(vlan_cfg.protoGroup), sizeof(rtk_vlan_protoGroup_t));
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_protoGroup_set(
    uint32                  unit,
    uint32                  protoGroup_idx,
    rtk_vlan_protoGroup_t   *pProtoGroup)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = protoGroup_idx;      
    memcpy(&(vlan_cfg.protoGroup), pProtoGroup, sizeof(rtk_vlan_protoGroup_t));
    SETSOCKOPT(RTDRV_VLAN_PROTO_GROUP_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.idx = protoGroup_idx;   
    GETSOCKOPT(RTDRV_VLAN_PORT_PROTO_VLAN_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    memcpy(pVlan_cfg, &(vlan_cfg.protoVlanCfg), sizeof(rtk_vlan_protoVlanCfg_t));
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.idx = protoGroup_idx;      
    memcpy(&(vlan_cfg.protoVlanCfg), pVlan_cfg, sizeof(rtk_vlan_protoVlanCfg_t));
    SETSOCKOPT(RTDRV_VLAN_PORT_PROTO_VLAN_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portOuterProtoVlan_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.idx = protoGroup_idx;   
    GETSOCKOPT(RTDRV_VLAN_PORT_OUTER_PROTO_VLAN_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    memcpy(pVlan_cfg, &(vlan_cfg.protoVlanCfg), sizeof(rtk_vlan_protoVlanCfg_t));
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portOuterProtoVlan_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  protoGroup_idx,
    rtk_vlan_protoVlanCfg_t *pVlan_cfg)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.idx = protoGroup_idx;      
    memcpy(&(vlan_cfg.protoVlanCfg), pVlan_cfg, sizeof(rtk_vlan_protoVlanCfg_t));
    SETSOCKOPT(RTDRV_VLAN_PORT_OUTER_PROTO_VLAN_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portTpidEntry_get(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 *pTpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    vlan_cfg.idx = tpid_idx;
    GETSOCKOPT(RTDRV_VLAN_PORT_TPID_ENTRY_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portTpidEntry_set(uint32 unit, rtk_port_t port, uint32 tpid_idx, uint32 tpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.idx = tpid_idx;  
    vlan_cfg.data = tpid;
    SETSOCKOPT(RTDRV_VLAN_PORT_TPID_ENTRY_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_TPID_MODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid_mode = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tpid_mode;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_TPID_MODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_INNER_TPID_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid_idx_mask = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tpid_idx_mask;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_INNER_TPID_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_TPID_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid_idx = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tpid_idx;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_TPID_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterTpidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t *pTpid_mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_TPID_MODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid_mode = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterTpidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_egrTpidMode_t tpid_mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tpid_mode;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_TPID_MODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_OUTER_TPID_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid_idx_mask = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tpid_idx_mask;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_OUTER_TPID_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_TPID_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid_idx = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tpid_idx;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_TPID_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrExtraTpid_get(uint32 unit, rtk_port_t port, uint32 *pTpid_idx_mask)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_EXTRA_TPID_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pTpid_idx_mask = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrExtraTpid_set(uint32 unit, rtk_port_t port, uint32 tpid_idx_mask)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tpid_idx_mask;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_EXTRA_TPID_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrIgnoreInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_IGNORE_INNER_TAG_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrIgnoreInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_IGNORE_INNER_TAG_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrIgnoreOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_IGNORE_OUTER_TAG_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrIgnoreOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_IGNORE_OUTER_TAG_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_TAG_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_TAG_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_TAG_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_TAG_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_EXTRA_TAG_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_EXTRA_TAG_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrExtraTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_EXTRA_TAG_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pEnable = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrExtraTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_EXTRA_TAG_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_VID_SOURCE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pVidSource = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = vidSource;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_VID_SOURCE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_PRI_SOURCE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pPriSource = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrInnerPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = priSource;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_INNER_PRI_SOURCE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pVidSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_VID_SOURCE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pVidSource = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t vidSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = vidSource;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_VID_SOURCE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterPriSource_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t *pPriSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_PRI_SOURCE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pPriSource = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrOuterPriSource_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSource_t priSource)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = priSource;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_OUTER_PRI_SOURCE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrTagKeepEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pKeepOuter,
    rtk_enable_t    *pKeepInner)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_TAG_KEEP_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pKeepOuter = vlan_cfg.data;
    *pKeepInner = vlan_cfg.data1;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portIgrTagKeepEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    keepOuter,
    rtk_enable_t    keepInner)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = keepOuter;
    vlan_cfg.data1 = keepInner;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_TAG_KEEP_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrTagKeepEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pKeepOuter,
    rtk_enable_t    *pKeepInner)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_VLAN_PORT_EGR_TAG_KEEP_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pKeepOuter = vlan_cfg.data;
    *pKeepInner = vlan_cfg.data1;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_portEgrTagKeepEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    keepOuter,
    rtk_enable_t    keepInner)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = keepOuter;
    vlan_cfg.data1 = keepInner;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGR_TAG_KEEP_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_fwdMode_get(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t *pMode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;   
    GETSOCKOPT(RTDRV_VLAN_FWD_MODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
    *pMode = vlan_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_vlan_fwdMode_set(uint32 unit, rtk_vlan_t vid, rtk_vlan_fwdMode_t mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;
    vlan_cfg.data = mode;
    SETSOCKOPT(RTDRV_VLAN_FWD_MODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);    
        
    return RT_ERR_OK;    
}

int32 rtk_vlan_l2UcastLookupMode_get(uint32 unit, rtk_vlan_t vid, rtk_l2_ucastLookupMode_t *pMode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;
    GETSOCKOPT(RTDRV_VLAN_UCAST_LUTMODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pMode = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_l2UcastLookupMode_set(uint32 unit, rtk_vlan_t vid, rtk_l2_ucastLookupMode_t mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;
    vlan_cfg.data = mode;
    SETSOCKOPT(RTDRV_VLAN_UCAST_LUTMODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_l2McastLookupMode_get(uint32 unit, rtk_vlan_t vid, rtk_l2_mcastLookupMode_t *pMode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;
    GETSOCKOPT(RTDRV_VLAN_MCAST_LUTMODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pMode = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_l2McastLookupMode_set(uint32 unit, rtk_vlan_t vid, rtk_l2_mcastLookupMode_t mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;
    vlan_cfg.data = mode;
    SETSOCKOPT(RTDRV_VLAN_MCAST_LUTMODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_profileIdx_get(uint32 unit, rtk_vlan_t vid, uint32 *pIdx)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;
    GETSOCKOPT(RTDRV_VLAN_PROFILE_IDX_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pIdx = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_profileIdx_set(uint32 unit, rtk_vlan_t vid, uint32 idx)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.vid = vid;
    vlan_cfg.data = idx;
    SETSOCKOPT(RTDRV_VLAN_PROFILE_IDX_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_profile_get(uint32 unit, uint32 idx, rtk_vlan_profile_t *pProfile)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = idx;
    GETSOCKOPT(RTDRV_VLAN_PROFILE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pProfile = vlan_cfg.profile;

    return RT_ERR_OK;
}

int32 rtk_vlan_profile_set(uint32 unit, uint32 idx, rtk_vlan_profile_t *pProfile)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = idx;
    vlan_cfg.profile = *pProfile;
    SETSOCKOPT(RTDRV_VLAN_PROFILE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_portIgrFilter_get(uint32 unit, rtk_port_t port, rtk_vlan_ifilter_t *pIgr_filter)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_IGR_FILTER_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pIgr_filter = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_portIgrFilter_set(uint32 unit, rtk_port_t port, rtk_vlan_ifilter_t igr_filter)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = igr_filter;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGR_FILTER_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_portPvidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t *pMode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_PVID_MODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pMode = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_portPvidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = mode;
    SETSOCKOPT(RTDRV_VLAN_PORT_PVID_MODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_portOuterPvidMode_get(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t *pMode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_OPVID_MODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pMode = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_portOuterPvidMode_set(uint32 unit, rtk_port_t port, rtk_vlan_pbVlan_mode_t mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = mode;
    SETSOCKOPT(RTDRV_VLAN_PORT_OPVID_MODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_macBasedVlan_get(uint32 unit, uint32 index, uint32 *valid,
        rtk_mac_t *smac, rtk_vlan_t *vid, rtk_pri_t *priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_MAC_BASED_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *valid = vlan_cfg.data;
    *smac = vlan_cfg.mac;
    *vid = vlan_cfg.vid;
    *priority = vlan_cfg.data1;

    return RT_ERR_OK;
}

int32 rtk_vlan_macBasedVlan_set(uint32 unit, uint32 index, uint32 valid,
        rtk_mac_t *smac, rtk_vlan_t vid, rtk_pri_t priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    vlan_cfg.data = valid;
    vlan_cfg.mac = *smac;
    vlan_cfg.vid = vid;
    vlan_cfg.data1 = priority;
    SETSOCKOPT(RTDRV_VLAN_MAC_BASED_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_macBasedVlanWithMsk_get(uint32 unit, uint32 index, uint32 *valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, rtk_vlan_t *vid, rtk_pri_t *priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_MAC_BASED_WITH_MSK_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *valid = vlan_cfg.data;
    *smac = vlan_cfg.mac;
    *smsk = vlan_cfg.msk;
    *vid = vlan_cfg.vid;
    *priority = vlan_cfg.data1;

    return RT_ERR_OK;
}

int32 rtk_vlan_macBasedVlanWithMsk_set(uint32 unit, uint32 index, uint32 valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, rtk_vlan_t vid, rtk_pri_t priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    vlan_cfg.data = valid;
    vlan_cfg.mac = *smac;
    vlan_cfg.msk = *smsk;
    vlan_cfg.vid = vid;
    vlan_cfg.data1 = priority;
    SETSOCKOPT(RTDRV_VLAN_MAC_BASED_WITH_MSK_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_macBasedVlanWithPort_get(uint32 unit, uint32 index, uint32 *valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, rtk_port_t *port, rtk_port_t *pmsk, rtk_vlan_t *vid, rtk_pri_t *priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_MAC_BASED_WITH_PORT_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *valid = vlan_cfg.data;
    *smac = vlan_cfg.mac;
    *smsk = vlan_cfg.msk;
    *port = vlan_cfg.port;
    *pmsk = vlan_cfg.port_msk;
    *vid = vlan_cfg.vid;
    *priority = vlan_cfg.data1;

    return RT_ERR_OK;
}

int32 rtk_vlan_macBasedVlanWithPort_set(uint32 unit, uint32 index, uint32 valid,
        rtk_mac_t *smac, rtk_mac_t *smsk, rtk_port_t port, rtk_port_t pmsk, rtk_vlan_t vid, rtk_pri_t priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    vlan_cfg.data = valid;
    vlan_cfg.mac = *smac;
    vlan_cfg.msk = *smsk;
    vlan_cfg.port = port;
    vlan_cfg.port_msk = pmsk;
    vlan_cfg.vid = vid;
    vlan_cfg.data1 = priority;
    SETSOCKOPT(RTDRV_VLAN_MAC_BASED_WITH_PORT_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_vlan_ipSubnetBasedVlan_get(uint32 unit, uint32 index, uint32 *valid,
        ipaddr_t *sip, ipaddr_t *sip_mask, rtk_vlan_t *vid, rtk_pri_t *priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_IP_SUBNET_BASED_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *valid = vlan_cfg.data;
    *sip = vlan_cfg.sip;
    *sip_mask = vlan_cfg.sip_msk;
    *vid = vlan_cfg.vid;
    *priority = vlan_cfg.data1;

    return RT_ERR_OK;
}

int32
rtk_vlan_ipSubnetBasedVlan_set(uint32 unit, uint32 index, uint32 valid,
        ipaddr_t sip, ipaddr_t sip_mask, rtk_vlan_t vid, rtk_pri_t priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    vlan_cfg.data = valid;
    vlan_cfg.sip = sip;
    vlan_cfg.sip_msk = sip_mask;
    vlan_cfg.vid = vid;
    vlan_cfg.data1 = priority;
    SETSOCKOPT(RTDRV_VLAN_IP_SUBNET_BASED_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_vlan_ipSubnetBasedVlanWithPort_get(uint32 unit, uint32 index, uint32 *valid,
        ipaddr_t *sip, ipaddr_t *sip_mask, rtk_port_t *port, rtk_port_t *port_mask, rtk_vlan_t *vid, rtk_pri_t *priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_IP_SUBNET_BASED_WITH_PORT_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *valid = vlan_cfg.data;
    *sip = vlan_cfg.sip;
    *sip_mask = vlan_cfg.sip_msk;
    *port = vlan_cfg.port;
    *port_mask = vlan_cfg.port_msk;
    *vid = vlan_cfg.vid;
    *priority = vlan_cfg.data1;

    return RT_ERR_OK;
}

int32
rtk_vlan_ipSubnetBasedVlanWithPort_set(uint32 unit, uint32 index, uint32 valid,
        ipaddr_t sip, ipaddr_t sip_mask, rtk_port_t port, rtk_port_t port_mask, rtk_vlan_t vid, rtk_pri_t priority)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    vlan_cfg.data = valid;
    vlan_cfg.sip = sip;
    vlan_cfg.sip_msk = sip_mask;
    vlan_cfg.port = port;
    vlan_cfg.port_msk = port_mask;
    vlan_cfg.vid = vid;
    vlan_cfg.data1 = priority;
    SETSOCKOPT(RTDRV_VLAN_IP_SUBNET_BASED_WITH_PORT_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_innerTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = tpid_idx;
    GETSOCKOPT(RTDRV_VLAN_ITPID_ENTRY_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pTpid = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_innerTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = tpid_idx;
    vlan_cfg.data = tpid;
    SETSOCKOPT(RTDRV_VLAN_ITPID_ENTRY_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_outerTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = tpid_idx;
    GETSOCKOPT(RTDRV_VLAN_OTPID_ENTRY_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pTpid = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_outerTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = tpid_idx;
    vlan_cfg.data = tpid;
    SETSOCKOPT(RTDRV_VLAN_OTPID_ENTRY_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_extraTpidEntry_get(uint32 unit, uint32 tpid_idx, uint32 *pTpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = tpid_idx;
    GETSOCKOPT(RTDRV_VLAN_ETPID_ENTRY_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pTpid = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_extraTpidEntry_set(uint32 unit, uint32 tpid_idx, uint32 tpid)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = tpid_idx;
    vlan_cfg.data = tpid;
    SETSOCKOPT(RTDRV_VLAN_ETPID_ENTRY_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_portEgrInnerTagSts_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t *pSts)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_EGR_ITAG_STS_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pSts = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_portEgrInnerTagSts_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t sts)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = sts;
    SETSOCKOPT(RTDRV_VLAN_EGR_ITAG_STS_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_portEgrOuterTagSts_get(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t *pSts)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_EGR_OTAG_STS_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pSts = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_portEgrOuterTagSts_set(uint32 unit, rtk_port_t port, rtk_vlan_tagSts_t sts)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = sts;
    SETSOCKOPT(RTDRV_VLAN_EGR_OTAG_STS_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_igrVlanCnvtBlkMode_get(uint32 unit, uint32 blk_idx, rtk_vlan_igrVlanCnvtBlk_mode_t *pMode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = blk_idx;
    GETSOCKOPT(RTDRV_VLAN_IGRVLANCNVT_BLKMODE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pMode = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_igrVlanCnvtBlkMode_set(uint32 unit, uint32 blk_idx, rtk_vlan_igrVlanCnvtBlk_mode_t mode)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = blk_idx;
    vlan_cfg.data = mode;
    SETSOCKOPT(RTDRV_VLAN_IGRVLANCNVT_BLKMODE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_igrVlanCnvtEntry_get(uint32 unit, uint32 index, rtk_vlan_igrVlanCnvtEntry_t *pData)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_IGRVLANCNVT_ENTRY_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pData = vlan_cfg.igrCnvtEntry;

    return RT_ERR_OK;
}

int32 rtk_vlan_igrVlanCnvtEntry_set(uint32 unit, uint32 index, rtk_vlan_igrVlanCnvtEntry_t *pData)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    vlan_cfg.igrCnvtEntry = *pData;
    SETSOCKOPT(RTDRV_VLAN_IGRVLANCNVT_ENTRY_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_egrVlanCnvtDblTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    GETSOCKOPT(RTDRV_VLAN_EGRVLANCNVT_DBLTAG_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pEnable = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_egrVlanCnvtDblTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_EGRVLANCNVT_DBLTAG_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_egrVlanCnvtVidSource_get(uint32 unit, rtk_l2_vlanMode_t *pSrc)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    GETSOCKOPT(RTDRV_VLAN_EGRVLANCNVT_VIDSRC_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pSrc = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_egrVlanCnvtVidSource_set(uint32 unit, rtk_l2_vlanMode_t src)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = src;
    SETSOCKOPT(RTDRV_VLAN_EGRVLANCNVT_VIDSRC_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_egrVlanCnvtEntry_get(uint32 unit, uint32 index, rtk_vlan_egrVlanCnvtEntry_t *pData)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_EGRVLANCNVT_ENTRY_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pData = vlan_cfg.egrCnvtEntry;

    return RT_ERR_OK;
}

int32 rtk_vlan_egrVlanCnvtEntry_set(uint32 unit, uint32 index, rtk_vlan_egrVlanCnvtEntry_t *pData)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    vlan_cfg.egrCnvtEntry = *pData;
    SETSOCKOPT(RTDRV_VLAN_EGRVLANCNVT_ENTRY_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_vlan_portVlanAggrEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_VLANAGGR_ENABLE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pEnable = vlan_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_vlan_portVlanAggrEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = enable;
    SETSOCKOPT(RTDRV_VLAN_PORT_VLANAGGR_ENABLE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_vlan_leakyStpFilter_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;

    GETSOCKOPT(RTDRV_VLAN_LEAKYSTPFILTER_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    *pEnable = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_leakyStpFilter_get */

int32
rtk_vlan_leakyStpFilter_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = enable;

    SETSOCKOPT(RTDRV_VLAN_LEAKYSTPFILTER_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_leakyStpFilter_set */

int32
rtk_vlan_except_get(uint32 unit, rtk_action_t *pAction)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;

    GETSOCKOPT(RTDRV_VLAN_EXCEPT_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    *pAction = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_except_get */

int32
rtk_vlan_except_set(uint32 unit, rtk_action_t action)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.data = action;

    SETSOCKOPT(RTDRV_VLAN_EXCEPT_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_except_set */

int32
rtk_vlan_portIgrCnvtDfltAct_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;

    GETSOCKOPT(RTDRV_VLAN_PORTIGRCNVTDFLTACT_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    *pAction = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_portIgrCnvtDfltAct_get */

int32
rtk_vlan_portIgrCnvtDfltAct_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = action;
    SETSOCKOPT(RTDRV_VLAN_PORTIGRCNVTDFLTACT_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_portIgrCnvtDfltAct_set */

int32
rtk_vlan_portIgrTagKeepType_get(uint32 unit, rtk_port_t port, rtk_vlan_tagKeepType_t * pKeeptypeOuter, rtk_vlan_tagKeepType_t * pKeeptypeInner)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_IGRTAGKEEPTYPE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pKeeptypeOuter = vlan_cfg.data;
    *pKeeptypeInner = vlan_cfg.data1;

    return RT_ERR_OK;
} /* end of rtk_vlan_portIgrTagKeepType_get */

int32
rtk_vlan_portIgrTagKeepType_set(uint32 unit, rtk_port_t port, rtk_vlan_tagKeepType_t keeptypeOuter, rtk_vlan_tagKeepType_t keeptypeInner)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = keeptypeOuter;
    vlan_cfg.data1 = keeptypeInner;
    SETSOCKOPT(RTDRV_VLAN_PORT_IGRTAGKEEPTYPE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_vlan_portIgrTagKeepType_set */

int32
rtk_vlan_portEgrTagKeepType_get(uint32 unit, rtk_port_t port, rtk_vlan_tagKeepType_t * pKeeptypeOuter, rtk_vlan_tagKeepType_t * pKeeptypeInner)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_EGRTAGKEEPTYPE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pKeeptypeOuter = vlan_cfg.data;
    *pKeeptypeInner = vlan_cfg.data1;

    return RT_ERR_OK;
} /* end of rtk_vlan_portIgrTagKeepType_get */

int32
rtk_vlan_portEgrTagKeepType_set(uint32 unit, rtk_port_t port, rtk_vlan_tagKeepType_t keeptypeOuter, rtk_vlan_tagKeepType_t keeptypeInner)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = keeptypeOuter;
    vlan_cfg.data1 = keeptypeInner;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGRTAGKEEPTYPE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_vlan_portIgrTagKeepType_set */

int32
rtk_vlan_igrVlanCnvtEntry_delAll(uint32 unit)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    SETSOCKOPT(RTDRV_VLAN_IGRVLANCNVTENTRY_DELALL, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_igrVlanCnvtEntry_delAll */

int32
rtk_vlan_egrVlanCnvtEntry_delAll(uint32 unit)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    SETSOCKOPT(RTDRV_VLAN_EGRVLANCNVTENTRY_DELALL, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_egrVlanCnvtEntry_delAll */

int32
rtk_vlan_egrVlanCnvtRangeCheckVid_get(uint32 unit, uint32 index,
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t *pData)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    GETSOCKOPT(RTDRV_VLAN_EGRVLANCNVTRANGECHECKVID_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    memcpy(pData, &vlan_cfg.egrRangeCheck, sizeof(rtk_vlan_egrVlanCnvtRangeCheck_vid_t));

    return RT_ERR_OK;
}    /* end of rtk_vlan_egrVlanCnvtRangeCheckVid_get */

int32
rtk_vlan_egrVlanCnvtRangeCheckVid_set(uint32 unit, uint32 index,
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t *pData)
{
    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.idx = index;
    memcpy(&vlan_cfg.egrRangeCheck, pData, sizeof(rtk_vlan_egrVlanCnvtRangeCheck_vid_t));
    SETSOCKOPT(RTDRV_VLAN_EGRVLANCNVTRANGECHECKVID_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_egrVlanCnvtRangeCheckVid_set */


int32
rtk_vlan_portVlanAggrVidSource_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pSrc)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_VLANAGGRVIDSOURCE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pSrc = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_portVlanAggrVidSource_get */

int32
rtk_vlan_portVlanAggrVidSource_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t src)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = src;
    SETSOCKOPT(RTDRV_VLAN_PORT_VLANAGGRVIDSOURCE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_portVlanAggrVidSource_set */

int32
rtk_vlan_portVlanAggrPriTagVidSource_get(uint32 unit, rtk_port_t port, rtk_vlan_priTagVidSrc_t *pSrc)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_VLANAGGRPRITAGVIDSOURCE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pSrc = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_portVlanAggrPriTagVidSource_get */

int32
rtk_vlan_portVlanAggrPriTagVidSource_set(uint32 unit, rtk_port_t port, rtk_vlan_priTagVidSrc_t src)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = src;
    SETSOCKOPT(RTDRV_VLAN_PORT_VLANAGGRPRITAGVIDSOURCE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_portVlanAggrPriTagVidSource_set */

int32
rtk_vlan_portEgrVlanCnvtVidSource_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pSrc)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_EGRVLANCNVTVIDSOURCE_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pSrc = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_portEgrVlanCnvtVidSource_get */

int32
rtk_vlan_portEgrVlanCnvtVidSource_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t src)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = src;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGRVLANCNVTVIDSOURCE_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_portEgrVlanCnvtVidSource_set */

int32
rtk_vlan_portEgrVlanCnvtVidTarget_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pTgt)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_EGRVLANCNVTVIDTARGET_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pTgt = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_portEgrVlanCnvtVidTarget_get */

int32
rtk_vlan_portEgrVlanCnvtVidTarget_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t tgt)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = tgt;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGRVLANCNVTVIDTARGET_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_portEgrVlanCnvtVidTarget_set */

int32
rtk_vlan_portEgrVlanCnvtLookupMissAct_get(uint32 unit, rtk_port_t port, rtk_vlan_lookupMissAct_t *pAct)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    GETSOCKOPT(RTDRV_VLAN_PORT_EGRVLANCNVTLOOKUPMISSACT_GET, &vlan_cfg, rtdrv_vlanCfg_t, 1);
    *pAct = vlan_cfg.data;

    return RT_ERR_OK;
}    /* end of rtk_vlan_portEgrVlanCnvtLookupMissAct_get */

int32
rtk_vlan_portEgrVlanCnvtLookupMissAct_set(uint32 unit, rtk_port_t port, rtk_vlan_lookupMissAct_t act)
{

    rtdrv_vlanCfg_t vlan_cfg;

    vlan_cfg.unit = unit;
    vlan_cfg.port = port;
    vlan_cfg.data = act;
    SETSOCKOPT(RTDRV_VLAN_PORT_EGRVLANCNVTLOOKUPMISSACT_SET, &vlan_cfg, rtdrv_vlanCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_vlan_portEgrVlanCnvtLookupMissAct_set */



