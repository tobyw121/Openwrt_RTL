/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 21991 $
 * $Date: 2011-09-05 17:14:04 +0800 (Mon, 05 Sep 2011) $
 *
 * Purpose : Definition those public MPLS routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *              1) MPLS
 */



/*
 * Include Files
 */
#include <netinet/in.h>
#include <sys/socket.h>
#include <rtk/mpls.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Module Name : MPLS */

int32
rtk_mpls_init(uint32 unit)
{
    rtdrv_mplsCfg_t cfg;

    cfg.unit = unit;
    SETSOCKOPT(RTDRV_MPLS_INIT, &cfg, rtdrv_mplsCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_mpls_init */

int32
rtk_mpls_ttlInherit_get(uint32 unit, rtk_mpls_ttlInherit_t *inherit)
{
    rtdrv_mplsCfg_t cfg;

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_MPLS_TTLINHERIT_GET, &cfg, rtdrv_mplsCfg_t, 1);

    *inherit = cfg.u.inherit;

    return RT_ERR_OK;
}    /* end of rtk_mpls_ttlInherit_get */

int32
rtk_mpls_ttlInherit_set(uint32 unit, rtk_mpls_ttlInherit_t inherit)
{
    rtdrv_mplsCfg_t cfg;

    cfg.unit = unit;
    cfg.u.inherit = inherit;

    SETSOCKOPT(RTDRV_MPLS_TTLINHERIT_SET, &cfg, rtdrv_mplsCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_mpls_ttlInherit_set */

int32
rtk_mpls_encap_get(uint32 unit, uint32 lib_idx, rtk_mpls_encap_t *info)
{
    rtdrv_mplsCfg_t cfg;

    cfg.unit = unit;
    cfg.lib_idx = lib_idx;
    cfg.u.encap_info.oper = info->oper;

    GETSOCKOPT(RTDRV_MPLS_ENCAP_GET, &cfg, rtdrv_mplsCfg_t, 1);

    memcpy(info, &cfg.u.encap_info, sizeof(rtk_mpls_encap_t));

    return RT_ERR_OK;
}    /* end of rtk_mpls_encap_get */

int32
rtk_mpls_encap_set(uint32 unit, uint32 lib_idx, rtk_mpls_encap_t *info)
{
    rtdrv_mplsCfg_t cfg;

    cfg.unit = unit;
    cfg.lib_idx = lib_idx;
    memcpy(&cfg.u.encap_info, info, sizeof(rtk_mpls_encap_t));

    SETSOCKOPT(RTDRV_MPLS_ENCAP_SET, &cfg, rtk_mpls_encap_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_mpls_encap_set */

int32
rtk_mpls_enable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_mplsCfg_t cfg;

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_MPLS_ENABLE_GET, &cfg, rtdrv_mplsCfg_t, 1);
    *pEnable = cfg.enable;

    return RT_ERR_OK;
}    /* end of rtk_mpls_enable_get */

int32
rtk_mpls_enable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_mplsCfg_t cfg;

    cfg.unit = unit;
    cfg.enable = enable;
    SETSOCKOPT(RTDRV_MPLS_ENABLE_SET, &cfg, rtdrv_mplsCfg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_mpls_enable_set */
