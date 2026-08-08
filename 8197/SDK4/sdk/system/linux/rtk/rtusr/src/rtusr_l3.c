/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 31272 $
 * $Date: 2012-07-23 14:42:21 +0800 (Mon, 23 Jul 2012) $
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


int32
rtk_l3_init(uint32 unit)
{
    rtdrv_unitCfg_t data;

    data.unit = unit;
    SETSOCKOPT(RTDRV_L3_INIT, &data, rtdrv_unitCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_l3_init */


int32
rtk_l3_ttlExpireAction_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t *pAction)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    GETSOCKOPT(RTDRV_L3_TTLEXPIREACTION_GET, &config, rtdrv_l3_config_t, 1);
    *pAction = config.action;

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireAction_get */


int32
rtk_l3_ttlExpireAction_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t action)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    config.action = action;
    SETSOCKOPT(RTDRV_L3_TTLEXPIREACTION_SET, &config, rtdrv_l3_config_t, 1);

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireAction_set */


int32
rtk_l3_ttlExpireTrapPri_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t *pPriority)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    GETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPPRI_GET, &config, rtdrv_l3_config_t, 1);
    *pPriority = config.priority;

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapPri_get */


int32
rtk_l3_ttlExpireTrapPri_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t priority)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    config.priority = priority;
    SETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPPRI_SET, &config, rtdrv_l3_config_t, 1);

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapPri_set */


int32
rtk_l3_ttlExpireTrapPriEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    GETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPPRIENABLE_GET, &config, rtdrv_l3_config_t, 1);
    *pEnable = config.dfPri;

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapPriEnable_get */

int32
rtk_l3_ttlExpireTrapPriEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    config.dfPri = enable;
    SETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPPRIENABLE_SET, &config, rtdrv_l3_config_t, 1);

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapPriEnable_set */


int32
rtk_l3_ttlExpireTrapDP_get(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 *pDp)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    GETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPDP_GET, &config, rtdrv_l3_config_t, 1);
    *pDp = config.dpValue;

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapDP_get */


int32
rtk_l3_ttlExpireTrapDP_set(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 dp)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    config.dpValue = dp;
    SETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPDP_SET, &config, rtdrv_l3_config_t, 1);

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapDP_set */


int32
rtk_l3_ttlExpireTrapDPEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    GETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPDPENABLE_GET, &config, rtdrv_l3_config_t, 1);
    *pEnable = config.dfDp;

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapDPEnable_get */

int32
rtk_l3_ttlExpireTrapDPEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    config.dfDp = enable;
    SETSOCKOPT(RTDRV_L3_TTLEXPIRETRAPDPENABLE_SET, &config, rtdrv_l3_config_t, 1);

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireTrapDPEnable_set */


int32
rtk_l3_ttlExpireAddCPUTagEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    GETSOCKOPT(RTDRV_L3_TTLEXPIREADDCPUTAGENABLE_GET, &config, rtdrv_l3_config_t, 1);
    *pEnable = config.insertCpuTag;

    return RT_ERR_OK;
} /* end of rtk_l3_ttlExpireAddCPUTagEnable_get */


int32
rtk_l3_ttlExpireAddCPUTagEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.type = type;
    config.insertCpuTag = enable;
    SETSOCKOPT(RTDRV_L3_TTLEXPIREADDCPUTAGENABLE_SET, &config, rtdrv_l3_config_t, 1);

    return RT_ERR_OK;
}/* end of rtk_l3_ttlExpireAddCPUTagEnable_set */

int32
rtk_l3_routeEntry_get(uint32 unit, uint32 index, rtk_l3_routeEntry_t *pEntry)
{
    rtdrv_l3_routeEntry_t entry_cfg;

    entry_cfg.unit = unit;
    entry_cfg.index = index;
    GETSOCKOPT(RTDRV_L3_ROUTE_ROUTEENTRY_GET, &entry_cfg, rtdrv_l3_routeEntry_t, 1);
    memcpy(pEntry, &entry_cfg.entry, sizeof(rtk_l3_routeEntry_t));

    return RT_ERR_OK;
}

int32
rtk_l3_routeEntry_set(uint32 unit, uint32 index, rtk_l3_routeEntry_t *pEntry)
{
    rtdrv_l3_routeEntry_t entry_cfg;

    entry_cfg.unit = unit;
    entry_cfg.index = index;
    memcpy(&entry_cfg.entry, pEntry, sizeof(rtk_l3_routeEntry_t));
    SETSOCKOPT(RTDRV_L3_ROUTE_ROUTEENTRY_SET, &entry_cfg, rtdrv_l3_routeEntry_t, 1);

    return RT_ERR_OK;
}

int32
rtk_l3_routeSwitchMacAddr_get(uint32 unit, uint32 index, rtk_mac_t *pMac)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.index = index;
    GETSOCKOPT(RTDRV_L3_ROUTE_SWITCHMACADDR_GET, &config, rtdrv_l3_config_t, 1);
    memcpy(pMac, &config.mac, sizeof(rtk_mac_t));

    return RT_ERR_OK;
}

int32
rtk_l3_routeSwitchMacAddr_set(uint32 unit, uint32 index, rtk_mac_t *pMac)
{
    rtdrv_l3_config_t config;

    config.unit = unit;
    config.index = index;
    memcpy( &config.mac, pMac, sizeof(rtk_mac_t));
    SETSOCKOPT(RTDRV_L3_ROUTE_SWITCHMACADDR_SET, &config, rtdrv_l3_config_t, 1);

    return RT_ERR_OK;
}

