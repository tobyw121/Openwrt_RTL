/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 * 
 * $Revision: 46645 $
 * $Date: 2014-02-26 19:43:48 +0800 (Wed, 26 Feb 2014) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Clock/Time
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/time.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>


int32 rtk_time_portPtpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    time_cfg.port = port;
    GETSOCKOPT(RTDRV_TIME_PORT_PTP_ENABLE_GET, &time_cfg, rtdrv_timeCfg_t, 1);
    *pEnable = time_cfg.enable;

    return RT_ERR_OK;
}

int32 rtk_time_portPtpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit   = unit;
    time_cfg.port   = port;
    time_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TIME_PORT_PTP_ENABLE_SET, &time_cfg, rtdrv_timeCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_time_portPtpRxTimestamp_get(uint32 unit, rtk_port_t port, rtk_time_ptpIdentifier_t identifier, rtk_time_timeStamp_t *pTimeStamp)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    time_cfg.port = port;
    time_cfg.identifier = identifier;
    GETSOCKOPT(RTDRV_TIME_PORT_PTP_RX_TIME_GET, &time_cfg, rtdrv_timeCfg_t, 1);
    *pTimeStamp = time_cfg.timeStamp;

    return RT_ERR_OK;
}

int32 rtk_time_portPtpTxTimestamp_get(uint32 unit, rtk_port_t port, rtk_time_ptpIdentifier_t identifier, rtk_time_timeStamp_t *pTimeStamp)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    time_cfg.port = port;
    time_cfg.identifier = identifier;
    GETSOCKOPT(RTDRV_TIME_PORT_PTP_TX_TIME_GET, &time_cfg, rtdrv_timeCfg_t, 1);
    *pTimeStamp = time_cfg.timeStamp;

    return RT_ERR_OK;
}

int32 rtk_time_refTime_get(uint32 unit, rtk_time_timeStamp_t *pTimeStamp)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TIME_REF_TIME_GET, &time_cfg, rtdrv_timeCfg_t, 1);
    *pTimeStamp = time_cfg.timeStamp;

    return RT_ERR_OK;
}

int32 rtk_time_refTime_set(uint32 unit, rtk_time_timeStamp_t timeStamp)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    time_cfg.timeStamp = timeStamp;
    SETSOCKOPT(RTDRV_TIME_REF_TIME_SET, &time_cfg, rtdrv_timeCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_time_refTimeAdjust_set(uint32 unit, uint32 sign, rtk_time_timeStamp_t timeStamp)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    time_cfg.sign = sign;
    time_cfg.timeStamp = timeStamp;
    SETSOCKOPT(RTDRV_TIME_REF_TIME_ADJUST_SET, &time_cfg, rtdrv_timeCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_time_refTimeEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TIME_REF_TIME_ENABLE_GET, &time_cfg, rtdrv_timeCfg_t, 1);
    *pEnable = time_cfg.enable;

    return RT_ERR_OK;
}

int32 rtk_time_refTimeEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_timeCfg_t time_cfg;

    time_cfg.unit = unit;
    time_cfg.enable = enable;
    SETSOCKOPT(RTDRV_TIME_REF_TIME_ENABLE_SET, &time_cfg, rtdrv_timeCfg_t, 1);

    return RT_ERR_OK;
}


