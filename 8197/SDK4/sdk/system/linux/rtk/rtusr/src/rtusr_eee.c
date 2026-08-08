/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 * $Revision: 25501 $
 * $Date: 2011-11-25 19:49:08 +0800 (Fri, 25 Nov 2011) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) eee
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/eee.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_eee_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_eeeCfg_t eee_cfg;

    eee_cfg.unit = unit;
    eee_cfg.port = port;
    GETSOCKOPT(RTDRV_EEE_PORT_ENABLE_GET, &eee_cfg, rtdrv_eeeCfg_t, 1);
    *pEnable = eee_cfg.enable;
     
    return RT_ERR_OK;
}

int32 rtk_eee_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_eeeCfg_t eee_cfg;

    eee_cfg.unit = unit;
    eee_cfg.port = port;
    eee_cfg.enable = enable;
    SETSOCKOPT(RTDRV_EEE_PORT_ENABLE_SET, &eee_cfg, rtdrv_eeeCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_eee_portState_get(
    uint32 unit, 
    rtk_port_t port, 
    rtk_enable_t *pState)
{
    rtdrv_eeeCfg_t eee_cfg;

    eee_cfg.unit = unit;
    eee_cfg.port = port;
    
    GETSOCKOPT(RTDRV_EEE_PORT_STATE_GET, &eee_cfg, rtdrv_eeeCfg_t, 1);

    *pState = eee_cfg.enable;

    return RT_ERR_OK;
}    /* end of rtk_eee_portState_get */

int32 rtk_eeep_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_eeeCfg_t eee_cfg;

    eee_cfg.unit = unit;
    eee_cfg.port = port;
    GETSOCKOPT(RTDRV_EEEP_PORT_ENABLE_GET, &eee_cfg, rtdrv_eeeCfg_t, 1);
    *pEnable = eee_cfg.enable;

    return RT_ERR_OK;
}

int32 rtk_eeep_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_eeeCfg_t eee_cfg;

    eee_cfg.unit = unit;
    eee_cfg.port = port;
    eee_cfg.enable = enable;
    SETSOCKOPT(RTDRV_EEEP_PORT_ENABLE_SET, &eee_cfg, rtdrv_eeeCfg_t, 1);

    return RT_ERR_OK;
}

