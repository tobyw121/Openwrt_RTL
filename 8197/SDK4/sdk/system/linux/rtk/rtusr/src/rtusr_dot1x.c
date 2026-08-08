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
 * $Revision: 8992 $
 * $Date: 2010-04-12 14:18:51 +0800 (Mon, 12 Apr 2010) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) dot1x
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_dot1x_portBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;   
    GETSOCKOPT(RTDRV_DOT1X_PORT_BASED_ENABLE_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pEnable = dot1x_cfg.enable;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_portBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;   
    dot1x_cfg.enable = enable;   
    SETSOCKOPT(RTDRV_DOT1X_PORT_BASED_ENABLE_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_macBasedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;   
    GETSOCKOPT(RTDRV_DOT1X_MAC_BASED_ENABLE_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pEnable = dot1x_cfg.enable;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_macBasedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;   
    dot1x_cfg.enable = enable;   
    SETSOCKOPT(RTDRV_DOT1X_MAC_BASED_ENABLE_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_portUnauthPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    GETSOCKOPT(RTDRV_DOT1X_PORT_UNAUTH_PACKET_OPER_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pUnauthAction = dot1x_cfg.action;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_portUnauthPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    dot1x_cfg.action = unauthAction;   
    SETSOCKOPT(RTDRV_DOT1X_PORT_UNAUTH_PACKET_OPER_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_portUnauthTagPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    GETSOCKOPT(RTDRV_DOT1X_PORT_UNAUTH_TAG_PACKET_OPER_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pUnauthAction = dot1x_cfg.action;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_portUnauthTagPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    dot1x_cfg.action = unauthAction;   
    SETSOCKOPT(RTDRV_DOT1X_PORT_UNAUTH_TAG_PACKET_OPER_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_portUnauthUntagPacketOper_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   *pUnauthAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    GETSOCKOPT(RTDRV_DOT1X_PORT_UNAUTH_UNTAG_PACKET_OPER_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pUnauthAction = dot1x_cfg.action;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_portUnauthUntagPacketOper_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_dot1x_unauth_action_t   unauthAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    dot1x_cfg.action = unauthAction;   
    SETSOCKOPT(RTDRV_DOT1X_PORT_UNAUTH_UNTAG_PACKET_OPER_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_eapolFrame2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_FRAME_TO_CPU_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pEnable = dot1x_cfg.enable;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_eapolFrame2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.enable = enable;   
    SETSOCKOPT(RTDRV_DOT1X_FRAME_TO_CPU_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_portBasedAuthStatus_get(uint32 unit, rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;   
    GETSOCKOPT(RTDRV_DOT1X_PORT_BASED_AUTH_STATUS_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pPort_auth = dot1x_cfg.port_auth;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_portBasedAuthStatus_set(uint32 unit, rtk_port_t port, rtk_dot1x_auth_status_t port_auth)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;   
    dot1x_cfg.port_auth = port_auth;   
    SETSOCKOPT(RTDRV_DOT1X_PORT_BASED_AUTH_STATUS_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_portGuestVlan_get(uint32 unit, rtk_port_t port, rtk_vlan_t *pGuest_vlan)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    GETSOCKOPT(RTDRV_DOT1X_PORT_GUEST_VLAN_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pGuest_vlan = dot1x_cfg.vid;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_portGuestVlan_set(uint32 unit, rtk_port_t port, rtk_vlan_t guest_vlan)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    dot1x_cfg.vid = guest_vlan;   
    SETSOCKOPT(RTDRV_DOT1X_PORT_GUEST_VLAN_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_guestVlanBehavior_get(uint32 unit, rtk_dot1x_guestVlanBehavior_t *pBehavior)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_GUEST_VLAN_BEHAVIOR_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pBehavior = dot1x_cfg.gv_behavior;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_guestVlanBehavior_set(uint32 unit, rtk_dot1x_guestVlanBehavior_t behavior)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.gv_behavior = behavior;   
    SETSOCKOPT(RTDRV_DOT1X_GUEST_VLAN_BEHAVIOR_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_guestVlanRouteBehavior_get(uint32 unit, rtk_action_t *pFwdAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_GUEST_VLAN_ROUTE_BEHAVIOR_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pFwdAction = dot1x_cfg.rt_action;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_guestVlanRouteBehavior_set(uint32 unit, rtk_action_t fwdAction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.rt_action = fwdAction;   
    SETSOCKOPT(RTDRV_DOT1X_GUEST_VLAN_ROUTE_BEHAVIOR_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_macBasedAuthMac_add(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    dot1x_cfg.vid = vid;
    memcpy(&dot1x_cfg.auth_mac, pAuth_mac, sizeof(rtk_mac_t));
    SETSOCKOPT(RTDRV_DOT1X_AUTH_MAC_ADD, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_macBasedAuthMac_del(
    uint32      unit,
    rtk_port_t  port,
    rtk_vlan_t  vid,
    rtk_mac_t   *pAuth_mac)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    dot1x_cfg.vid = vid;
    memcpy(&dot1x_cfg.auth_mac, pAuth_mac, sizeof(rtk_mac_t));
    SETSOCKOPT(RTDRV_DOT1X_AUTH_MAC_DEL, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    
    return RT_ERR_OK;    
}

int32 rtk_dot1x_unauthPacketOper_get(uint32 unit, rtk_dot1x_unauth_action_t *pUnauth_action)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_UNAUTH_PACKET_OPER_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pUnauth_action = dot1x_cfg.action;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_unauthPacketOper_set(uint32 unit, rtk_dot1x_unauth_action_t unauth_action)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.action = unauth_action;   
    SETSOCKOPT(RTDRV_DOT1X_UNAUTH_PACKET_OPER_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_portBasedDirection_get(uint32 unit, rtk_port_t port, rtk_dot1x_direction_t *pPort_direction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    GETSOCKOPT(RTDRV_DOT1X_PORT_BASED_DIRECTION_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pPort_direction = dot1x_cfg.direction;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_portBasedDirection_set(uint32 unit, rtk_port_t port, rtk_dot1x_direction_t port_direction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.port = port;
    dot1x_cfg.direction = port_direction;   
    SETSOCKOPT(RTDRV_DOT1X_PORT_BASED_DIRECTION_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_macBasedDirection_get(uint32 unit, rtk_dot1x_direction_t *pMac_direction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_MAC_BASED_DIRECTION_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pMac_direction = dot1x_cfg.direction;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_macBasedDirection_set(uint32 unit, rtk_dot1x_direction_t mac_direction)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.direction = mac_direction;   
    SETSOCKOPT(RTDRV_DOT1X_MAC_BASED_DIRECTION_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_trapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_TRAP_PRI_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pPriority = dot1x_cfg.pri;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_trapPri_set(uint32 unit, rtk_pri_t priority)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.pri = priority;   
    SETSOCKOPT(RTDRV_DOT1X_TRAP_PRI_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_TRAP_PRI_ENABLE_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pEnable = dot1x_cfg.enable;

    return RT_ERR_OK;    
}

int32 rtk_dot1x_trapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.enable = enable;
    SETSOCKOPT(RTDRV_DOT1X_TRAP_PRI_ENABLE_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_dot1x_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DOT1X_TRAP_ADD_CPUTAG_ENABLE_GET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
    *pEnable = dot1x_cfg.enable;
    
    return RT_ERR_OK;
}

int32 rtk_dot1x_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_dot1xCfg_t dot1x_cfg;

    dot1x_cfg.unit = unit;
    dot1x_cfg.enable = enable;   
    SETSOCKOPT(RTDRV_DOT1X_TRAP_ADD_CPUTAG_ENABLE_SET, &dot1x_cfg, rtdrv_dot1xCfg_t, 1);    
         
    return RT_ERR_OK;    
}

