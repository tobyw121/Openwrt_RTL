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
 * $Revision: 41299 $
 * $Date: 2013-07-19 10:50:53 +0800 (Fri, 19 Jul 2013) $
 *
 * Purpose : PHY probe and init service APIs in the SDK.
 *
 * Feature : PHY probe and init service APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <hal/chipdef/driver.h>
#include <hal/chipdef/chip.h>
#include <hal/common/halctrl.h>
#include <hal/mac/mac_probe.h>
#include <hal/phy/identify.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Function Declaration
 */
#if !(defined(__MODEL_USER__) || defined(CONFIG_VIRTUAL_ARRAY_ONLY))
static int32 phy_probe_ext(uint32 unit, rtk_port_t port);
static int32 phy_probe_int(uint32 unit, rtk_port_t port);
#endif

/* Static Function Body */

#if !(defined(__MODEL_USER__) || defined(CONFIG_VIRTUAL_ARRAY_ONLY))
/* Function Name:
 *      phy_probe_ext
 * Description:
 *      Probe the external PHY chip in the specified chip.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
static int32
phy_probe_ext(uint32 unit, rtk_port_t port)
{
    rt_phyctrl_t    *pPhyctrl = NULL;

    if (!HAL_IS_PORT_EXIST(unit, port) || HAL_IS_CPU_PORT(unit, port))
    {  
        hal_ctrl[unit].pPhy_ctrl[port] = NULL; 
        RT_DBG(LOG_TRACE, MOD_HAL, "PHY external driver not probed (unit %d, port %d)", unit, port);
        return RT_ERR_OK;
    }

    /* Start phy_ctrl struct finding process */
    if (NULL == (pPhyctrl = phy_identify_find(unit, port)))
    {            
        hal_ctrl[unit].pPhy_ctrl[port] = NULL;
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_find(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_OK;
    }       

    hal_ctrl[unit].pPhy_ctrl[port] = pPhyctrl;
    RT_DBG(LOG_TRACE, MOD_HAL, "PHY driver probed (unit %d, port %d)", unit, port);

    /* Record the combo port information */
    if (PHY_AFLAG_COMBO == pPhyctrl->phy_aflags)
    {
        if (NULL == pPhyctrl->pPhyInfo)
            hal_portInfo_update(unit, port, RT_GE_COMBO_PORT);
        else if (pPhyctrl->pPhyInfo->isComboPhy[port % pPhyctrl->pPhyInfo->phy_num])
            hal_portInfo_update(unit, port, RT_GE_COMBO_PORT);
    }
    return RT_ERR_OK;
} /* end of phy_probe_ext */

/* Function Name:
 *      phy_probe_int
 * Description:
 *      Probe the internal PHY chip in the specified chip.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
static int32
phy_probe_int(uint32 unit, rtk_port_t port)
{
    rt_phyctrl_t    *pPhyctrl;

    pPhyctrl = hal_ctrl[unit].pPhy_ctrl[port];
    if (NULL != pPhyctrl)
        return RT_ERR_OK; /* exist external PHY already */

    if (!HAL_IS_PORT_EXIST(unit, port) || HAL_IS_CPU_PORT(unit, port))
    {  
        hal_ctrl[unit].pPhy_ctrl[port] = NULL; 
        RT_DBG(LOG_TRACE, MOD_HAL, "PHY internal driver not probed (unit %d, port %d)", unit, port);
        return RT_ERR_OK;
    }

    if (NULL == (pPhyctrl = phy_identify_int_find(unit, port)))
    {            
        hal_ctrl[unit].pPhy_ctrl[port] = NULL;
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_int_find(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_OK;
    }       

    hal_ctrl[unit].pPhy_ctrl[port] = pPhyctrl;
    RT_DBG(LOG_TRACE, MOD_HAL, "PHY driver probed (unit %d, port %d)", unit, port);

    /* Record the fiber port information */
    if (PHY_AFLAG_FIBER == pPhyctrl->phy_aflags)
        hal_portInfo_update(unit, port, RT_GE_SERDES_PORT);
    else if (PHY_AFLAG_10G_SERDES == pPhyctrl->phy_aflags)
        hal_portInfo_update(unit, port, RT_10GE_SERDES_PORT);

    return RT_ERR_OK;
} /* end of phy_probe_int */
#endif

/* Public Function Body */

/* Function Name:
 *      phy_probe
 * Description:
 *      Probe the PHY chip in the specified chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
phy_probe(uint32 unit)
{
#if !(defined(__MODEL_USER__) || defined(CONFIG_VIRTUAL_ARRAY_ONLY))
    uint32  i, min_port, max_port;
    int32   ret = RT_ERR_FAILED;

    /* Probe PHY */
    min_port = HAL_GET_MIN_PORT(unit);
    max_port = HAL_GET_MAX_PORT(unit);
    for (i = min_port; i < max_port; i++)
    {
        if ((ret = phy_probe_ext(unit, i)) != RT_ERR_OK)
        {  
            RT_LOG(LOG_TRACE, MOD_HAL, "PHY external probed (unit %d, port %d)", unit, i);
        }

        if ((ret = phy_probe_int(unit, i)) != RT_ERR_OK)
        {            
            RT_LOG(LOG_TRACE, MOD_HAL, "PHY internal probed (unit %d, port %d)", unit, i);
        }       
    }
    
/* For prevent remove 24-27 ports on 28M FPGA */
#if !defined(CONFIG_SDK_FPGA_PLATFORM)
    /* Remove ethernet port that is not PHY */
    for (i = min_port; i < max_port; i++)
    {
        if (HAL_IS_PORT_EXIST(unit, i) && !HAL_IS_PHY_EXIST(unit, i))
            hal_portInfo_update(unit, i, RT_PORT_NONE);
    }
#endif
#endif
    return RT_ERR_OK;
} /* end of phy_probe */

/* Function Name:
 *      phy_init
 * Description:
 *      Init the PHY chip in the specified chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
phy_init(uint32 unit)
{
    uint32  i;

    for (i = HAL_GET_MIN_PORT(unit); i < HAL_GET_MAX_PORT(unit); i++)
    {
        if ((NULL != hal_ctrl[unit].pPhy_ctrl[i]) &&
            (NULL != hal_ctrl[unit].pPhy_ctrl[i]->pPhydrv->fPhydrv_init))
        {
            hal_ctrl[unit].pPhy_ctrl[i]->pPhydrv->fPhydrv_init(unit, i);
            RT_LOG(LOG_INFO, MOD_HAL, "PHY driver init (unit %d port %d)", unit, i);
        }       
    }

    return RT_ERR_OK;
} /* end of phy_init */
