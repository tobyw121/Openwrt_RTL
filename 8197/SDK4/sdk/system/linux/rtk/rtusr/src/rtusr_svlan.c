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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) svlan
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_svlan_create(uint32 unit, rtk_vlan_t svid)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid = svid;
    SETSOCKOPT(RTDRV_SVLAN_SVID_CREATE, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_svlan_destroy(uint32 unit, rtk_vlan_t svid)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid = svid;
    SETSOCKOPT(RTDRV_SVLAN_SVID_DESTROY, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_svlan_memberPort_get(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid = svid;   
    GETSOCKOPT(RTDRV_SVLAN_MEMBER_GET, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
    memcpy(pSvlan_portmask, &svlan_cfg.svlan_portmask, sizeof(rtk_portmask_t));
    
    return RT_ERR_OK;    
}

int32 rtk_svlan_memberPort_set(uint32 unit, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid = svid;
    memcpy(&svlan_cfg.svlan_portmask, pSvlan_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_SVLAN_MEMBER_SET, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_svlan_memberPortEntry_get(uint32 unit, uint32 svid_idx, rtk_vlan_t *pSvid, rtk_portmask_t *pSvlan_portmask)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid_idx = svid_idx;   
    GETSOCKOPT(RTDRV_SVLAN_MEMBER_ENTRY_GET, &svlan_cfg, rtdrv_svlanCfg_t, 1); 
    *pSvid = svlan_cfg.svid;    
    memcpy(pSvlan_portmask, &svlan_cfg.svlan_portmask, sizeof(rtk_portmask_t));
    
    return RT_ERR_OK;    
}

int32 rtk_svlan_memberPortEntry_set(uint32 unit, uint32 svid_idx, rtk_vlan_t svid, rtk_portmask_t *pSvlan_portmask)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid_idx = svid_idx;   
    svlan_cfg.svid = svid;
    memcpy(&svlan_cfg.svlan_portmask, pSvlan_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_SVLAN_MEMBER_ENTRY_SET, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_svlan_portSvid_get(uint32 unit, rtk_port_t port, uint32 *pSvid)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.port = port;   
    GETSOCKOPT(RTDRV_SVLAN_PORT_SVID_GET, &svlan_cfg, rtdrv_svlanCfg_t, 1); 
    *pSvid = svlan_cfg.svid;    
    
    return RT_ERR_OK;    
}

int32 rtk_svlan_portSvid_set(uint32 unit, rtk_port_t port, uint32 svid)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.port = port;   
    svlan_cfg.svid = svid;
    SETSOCKOPT(RTDRV_SVLAN_PORT_SVID_SET, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_svlan_servicePort_get(uint32 unit, rtk_portmask_t *pSvlan_portmask)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SVLAN_SERVICE_PORT_GET, &svlan_cfg, rtdrv_svlanCfg_t, 1); 
    memcpy(pSvlan_portmask, &svlan_cfg.svlan_portmask, sizeof(rtk_portmask_t));
    
    return RT_ERR_OK;    
}

int32 rtk_svlan_servicePort_set(uint32 unit, rtk_portmask_t *pSvlan_portmask)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    memcpy(&svlan_cfg.svlan_portmask, pSvlan_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_SVLAN_SERVICE_PORT_SET, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_svlan_tpidEntry_get(uint32 unit, uint32 svlan_index, uint32 *pSvlan_tag_id)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid_idx = svlan_index;
    GETSOCKOPT(RTDRV_SVLAN_TPID_ENTRY_GET, &svlan_cfg, rtdrv_svlanCfg_t, 1); 
    *pSvlan_tag_id = svlan_cfg.svlan_tag_id;
    
    return RT_ERR_OK;    
}

int32 rtk_svlan_tpidEntry_set(uint32 unit, uint32 svlan_index, uint32 svlan_tag_id)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid_idx = svlan_index;
    svlan_cfg.svlan_tag_id = svlan_tag_id;
    SETSOCKOPT(RTDRV_SVLAN_TPID_ENTRY_SET, &svlan_cfg, rtdrv_svlanCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_svlan_nextValidMemberPortEntry_get(uint32 unit, int32 *pSvid_idx, rtk_vlan_t *pSvid, rtk_portmask_t *pSvlan_portmask)
{
    rtdrv_svlanCfg_t svlan_cfg;

    svlan_cfg.unit = unit;
    svlan_cfg.svid_idx = *pSvid_idx;
    GETSOCKOPT(RTDRV_SVLAN_VALID_MEMBER_ENTRY_GETNEXT, &svlan_cfg, rtdrv_svlanCfg_t, 1); 
    *pSvid_idx = svlan_cfg.svid_idx;
    *pSvid = svlan_cfg.svid;
    memcpy(pSvlan_portmask, &svlan_cfg.svlan_portmask, sizeof(rtk_portmask_t));
    
    return RT_ERR_OK;    
}
