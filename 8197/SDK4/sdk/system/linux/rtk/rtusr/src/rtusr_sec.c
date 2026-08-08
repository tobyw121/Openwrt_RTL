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
 * Purpose : Definition those public security APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Attack prevention
 * 
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/sec.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>


int32 rtk_sec_portAttackPrevent_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            *pAction)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    sec_cfg.attack_type = attack_type;
    GETSOCKOPT(RTDRV_SEC_PORT_ATTACK_PREVENT_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pAction = sec_cfg.action;
     
    return RT_ERR_OK;
}

int32 rtk_sec_portAttackPrevent_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    sec_cfg.attack_type = attack_type;
    sec_cfg.action = action;
    SETSOCKOPT(RTDRV_SEC_PORT_ATTACK_PREVENT_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_portMinIPv6FragLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    GETSOCKOPT(RTDRV_SEC_PORT_MIN_IPV6_FRAG_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;
     
    return RT_ERR_OK;
}

int32 rtk_sec_portMinIPv6FragLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_PORT_MIN_IPV6_FRAG_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_portMaxPingLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    GETSOCKOPT(RTDRV_SEC_PORT_MAX_PING_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;
     
    return RT_ERR_OK;
}

int32 rtk_sec_portMaxPingLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_PORT_MAX_PING_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_portMinTCPHdrLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    GETSOCKOPT(RTDRV_SEC_PORT_MIN_TCP_HDR_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;
     
    return RT_ERR_OK;
}

int32 rtk_sec_portMinTCPHdrLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_PORT_MIN_TCP_HDR_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_portSmurfNetmaskLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    GETSOCKOPT(RTDRV_SEC_PORT_SMURF_NETMASK_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;
     
    return RT_ERR_OK;
}

int32 rtk_sec_portSmurfNetmaskLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.port = port;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_PORT_SMURF_NETMASK_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_portAttackPreventEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_enable_t            *pEnable)
{
    rtdrv_secCfg_t sec_cfg;
    
    sec_cfg.unit = unit;
    sec_cfg.port = port;
    GETSOCKOPT(RTDRV_SEC_PORT_ATTACK_PREVENT_ENABLE_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pEnable = sec_cfg.enable;
    
    return RT_ERR_OK;
}

int32 rtk_sec_portAttackPreventEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_enable_t            enable)
{
    rtdrv_secCfg_t sec_cfg;
    
    sec_cfg.unit = unit;
    sec_cfg.port = port;
    sec_cfg.enable = enable;
    SETSOCKOPT(RTDRV_SEC_PORT_ATTACK_PREVENT_ENABLE_SET, &sec_cfg, rtdrv_secCfg_t, 1);
    
    return RT_ERR_OK;
}

int32 rtk_sec_attackPreventAction_get(
    uint32                  unit,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            *pAction)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.attack_type = attack_type;
    GETSOCKOPT(RTDRV_SEC_ATTACK_PREVENT_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pAction = sec_cfg.action;

    return RT_ERR_OK;
}

int32 rtk_sec_attackPreventAction_set(
    uint32                  unit,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.attack_type = attack_type;
    sec_cfg.action = action;
    SETSOCKOPT(RTDRV_SEC_ATTACK_PREVENT_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_minIPv6FragLen_get(uint32 unit, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SEC_MIN_IPV6_FRAG_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_sec_minIPv6FragLen_set(uint32 unit, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_MIN_IPV6_FRAG_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_maxPingLen_get(uint32 unit, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SEC_MAX_PING_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_sec_maxPingLen_set(uint32 unit, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_MAX_PING_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_minTCPHdrLen_get(uint32 unit, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SEC_MIN_TCP_HDR_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_sec_minTCPHdrLen_set(uint32 unit, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_MIN_TCP_HDR_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_sec_smurfNetmaskLen_get(uint32 unit, uint32 *pLength)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SEC_SMURF_NETMASK_LEN_GET, &sec_cfg, rtdrv_secCfg_t, 1);
    *pLength = sec_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_sec_smurfNetmaskLen_set(uint32 unit, uint32 length)
{
    rtdrv_secCfg_t sec_cfg;

    sec_cfg.unit = unit;
    sec_cfg.data = length;
    SETSOCKOPT(RTDRV_SEC_SMURF_NETMASK_LEN_SET, &sec_cfg, rtdrv_secCfg_t, 1);

    return RT_ERR_OK;
}




