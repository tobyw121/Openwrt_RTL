/*
 * Copyright(c) Realtek Semiconductor Corporation, 2010
 * All rights reserved.
 * 
 * $Revision: 30055 $
 * $Date: 2012-06-19 15:33:08 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) LED
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/led.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_led_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_ledCfg_t led_cfg;

    led_cfg.unit = unit;
    led_cfg.port = port;
    GETSOCKOPT(RTDRV_LED_PORT_ENABLE_GET, &led_cfg, rtdrv_ledCfg_t, 1);
    *pEnable = led_cfg.enable;
     
    return RT_ERR_OK;
}

int32 rtk_led_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_ledCfg_t led_cfg;

    led_cfg.unit = unit;
    led_cfg.port = port;
    led_cfg.enable = enable;
    SETSOCKOPT(RTDRV_LED_PORT_ENABLE_SET, &led_cfg, rtdrv_ledCfg_t, 1); 

    return RT_ERR_OK;
}

int32 rtk_led_sysEnable_get(uint32 unit, rtk_led_type_t type, rtk_enable_t *pEnable)
{
    rtdrv_ledCfg_t led_cfg;

    led_cfg.unit = unit;
    led_cfg.type = type;
    GETSOCKOPT(RTDRV_LED_SYS_ENABLE_GET, &led_cfg, rtdrv_ledCfg_t, 1);
    *pEnable = led_cfg.enable;
     
    return RT_ERR_OK;
}

int32 rtk_led_sysEnable_set(uint32 unit, rtk_led_type_t type, rtk_enable_t enable)
{
    rtdrv_ledCfg_t led_cfg;

    led_cfg.unit = unit;
    led_cfg.type = type;
    led_cfg.enable = enable;
    SETSOCKOPT(RTDRV_LED_SYS_ENABLE_SET, &led_cfg, rtdrv_ledCfg_t, 1); 

    return RT_ERR_OK;
}

int32
rtk_led_portLedEntitySwCtrlEnable_get(uint32 unit, rtk_port_t port,
    uint32 entity, rtk_enable_t *pEnable)
{
    rtdrv_ledCfg_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.entity = entity;

    GETSOCKOPT(RTDRV_LED_PORTLEDENTITYSWCTRLENABLE_GET, &cfg, rtdrv_ledCfg_t, 1);

    *pEnable = cfg.enable;

    return RT_ERR_OK;
}    /* end of rtk_led_portLedEntitySwCtrlEnable_get */

int32
rtk_led_portLedEntitySwCtrlEnable_set(uint32 unit, rtk_port_t port,
    uint32 entity, rtk_enable_t enable)
{
    rtdrv_ledCfg_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.entity = entity;
    cfg.enable = enable;
    SETSOCKOPT(RTDRV_LED_PORTLEDENTITYSWCTRLENABLE_SET, &cfg, rtdrv_ledCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_led_portLedEntitySwCtrlEnable_set */

int32
rtk_led_swCtrl_start(uint32 unit)
{
    rtdrv_ledCfg_t cfg;

    cfg.unit = unit;
    SETSOCKOPT(RTDRV_LED_SWCTRL_START, &cfg, rtdrv_ledCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_led_swCtrl_start */

int32
rtk_led_portLedEntitySwCtrlMode_get(uint32 unit, rtk_port_t port,
    uint32 entity, rtk_port_media_t media, rtk_led_swCtrl_mode_t *pMode)
{
    rtdrv_ledCfg_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.entity = entity;
    cfg.media = media;

    GETSOCKOPT(RTDRV_LED_PORTLEDENTITYSWCTRLMODE_GET, &cfg, rtdrv_ledCfg_t, 1);

    *pMode = cfg.mode;

    return RT_ERR_OK;
}    /* end of rtk_led_portLedEntitySwCtrlMode_get */

int32
rtk_led_portLedEntitySwCtrlMode_set(uint32 unit, rtk_port_t port,
    uint32 entity, rtk_port_media_t media, rtk_led_swCtrl_mode_t mode)
{
    rtdrv_ledCfg_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.entity = entity;
    cfg.media = media;
    cfg.mode = mode;
    SETSOCKOPT(RTDRV_LED_PORTLEDENTITYSWCTRLMODE_SET, &cfg, rtdrv_ledCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_led_portLedEntitySwCtrlMode_set */

int32
rtk_led_sysMode_get(uint32 unit, rtk_led_swCtrl_mode_t *pMode)
{
    rtdrv_ledCfg_t cfg;

    cfg.unit = unit;

    GETSOCKOPT(RTDRV_LED_SYSMODE_GET, &cfg, rtdrv_ledCfg_t, 1);

    *pMode = cfg.mode;

    return RT_ERR_OK;
}    /* end of rtk_led_sysMode_get */

int32
rtk_led_sysMode_set(uint32 unit, rtk_led_swCtrl_mode_t mode)
{
    rtdrv_ledCfg_t cfg;

    cfg.unit = unit;
    cfg.mode = mode;
    SETSOCKOPT(RTDRV_LED_SYSMODE_SET, &cfg, rtdrv_ledCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_led_sysMode_set */
