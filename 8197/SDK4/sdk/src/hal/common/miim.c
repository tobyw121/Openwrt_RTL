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
 * $Revision: 54450 $
 * $Date: 2014-12-30 14:31:41 +0800 (Tue, 30 Dec 2014) $
 *
 * Purpose : MIIM service APIs in the SDK.
 *
 * Feature : MIIM service APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_8214f.h>
#if defined(CONFIG_SDK_RTL8212B) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8214FB)
#include <hal/phy/phy_8214fb.h>
#endif
#if defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC)
#include <hal/phy/phy_8218b.h>
#endif
#include <hal/mac/drv.h>


/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      hal_miim_read
 * Description:
 *      Get PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData)
{
    int32   ret = RT_ERR_FAILED;
    uint32  access_page, fixed_page;
    hal_control_t   *pHalCtrl;
    rtk_port_media_t media;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x", unit, port, page, phy_reg);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_read)
        return RT_ERR_FAILED;

    if (NULL == pHalCtrl->pPhy_ctrl[port])
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, page, phy_reg, pData);
        return ret;
    }

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);

    /* E0005375 */
    /* Patch for RTL8329 switch + RTL8208 PHY
     * Access PHY by currently page id
     */
    if ((PHY_MODEL_ID_RTL8208G == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (RTL8329M_CHIP_ID == pHalCtrl->chip_id || RTL8329_CHIP_ID == pHalCtrl->chip_id))
    {
        access_page = fixed_page;    /* Patch case: access the currently page */
    }   /* End of E0005375 */
    else if ((PHY_MODEL_ID_RTL8212F == pHalCtrl->pPhy_ctrl[port]->phy_model_id) && (PHY_PAGE_0 == page))
    {
        /* change the access_page to 0x40 (copper) and 0x50 (fiber) */
        if (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_media_get(unit, port, &media) != RT_ERR_OK)
        {
            return RT_ERR_FAILED;
        }
        switch (media)
        {
            case PORT_MEDIA_COPPER:
            case PORT_MEDIA_COPPER_AUTO:
                access_page = 0x40;
                break;
            case PORT_MEDIA_FIBER:
            case PORT_MEDIA_FIBER_AUTO:
                access_page = 0x50;
                break;
            default:
                return RT_ERR_FAILED;
        }
    }
    else
    {
        access_page = page;
    }

    if (access_page > fixed_page)
    {
        /* Use currently page access mechanism */
        MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, access_page);
        access_page = fixed_page;
    }

    ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, access_page, phy_reg, pData);
#if defined(__MODEL_USER__)
    *pData = 0;
#else

#if defined(CONFIG_SDK_RTL8212B) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8214FB)
    if ((PHY_MODEL_ID_RTL8214F == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9) &&
        ((access_page == PHY_PAGE_0) || (access_page>=7 && access_page<=15)))
    {
        uint32  auto_1000f_shadow = 0;

        if (((*pData)&0x0200) == 0)
        {
            if ((ret = phy_8214fb_auto_1000f_get(unit, port, &auto_1000f_shadow)) != RT_ERR_OK)
            {
                RT_LOG(LOG_WARNING, MOD_HAL, "phy_8214fb_auto_1000f_get(unit=%d, port=%d) failed", unit, port);
                return ret;
            }
            (*pData) |= (auto_1000f_shadow<<9);
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC)
    if ((PHY_MODEL_ID_RTL8218B_EXT == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9))
    {
        uint32  auto_1000f_shadow = 0;

        if (((*pData)&0x0200) == 0)
        {
            if ((ret = phy_8218b_auto_1000f_get(unit, port, &auto_1000f_shadow)) != RT_ERR_OK)
            {
                RT_LOG(LOG_WARNING, MOD_HAL, "phy_8218b_auto_1000f_get(unit=%d, port=%d) failed", unit, port);
                return ret;
            }
            (*pData) |= (auto_1000f_shadow<<9);
        }
    }
#endif
#endif  /* defined(__MODEL_USER__) */

    /* E0007313 */
    if (((PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id) ||
        (PHY_MODEL_ID_RTL8218_TC == pHalCtrl->pPhy_ctrl[port]->phy_model_id)) && (access_page != fixed_page))
    {
        MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0);
    }
    /* End of E0007313 */
    return ret;
} /* end of hal_miim_read */


/* Function Name:
 *      hal_miim_write
 * Description:
 *      Set PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    int32   ret = RT_ERR_FAILED;
    uint32  access_page, fixed_page;
    hal_control_t   *pHalCtrl;
    rtk_port_media_t media;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, phy_reg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_write)
        return RT_ERR_FAILED;

    if (NULL == pHalCtrl->pPhy_ctrl[port])
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, page, phy_reg, data);
        return ret;
    }

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);

    /* E0005375 */
    /* Patch for RTL8329 switch + RTL8208 PHY
     * Access PHY by currently page id
     */
    if ((PHY_MODEL_ID_RTL8208G == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (RTL8329M_CHIP_ID == pHalCtrl->chip_id || RTL8329_CHIP_ID == pHalCtrl->chip_id))
    {
        access_page = fixed_page;   /* Patch case: access the currently page */
    }   /* End of E0005375 */
    else if ((PHY_MODEL_ID_RTL8212F == pHalCtrl->pPhy_ctrl[port]->phy_model_id) && (PHY_PAGE_0 == page))
    {
        /* change the access_page to 0x40 (copper) and 0x50 (fiber) */
        if (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_media_get(unit, port, &media) != RT_ERR_OK)
        {
            return RT_ERR_FAILED;
        }
        switch (media)
        {
            case PORT_MEDIA_COPPER:
            case PORT_MEDIA_COPPER_AUTO:
                access_page = 0x40;
                break;
            case PORT_MEDIA_FIBER:
            case PORT_MEDIA_FIBER_AUTO:
                access_page = 0x50;
                break;
            default:
                return RT_ERR_FAILED;
        }
    }
    else
    {
        access_page = page;
    }

    if (access_page > fixed_page)
    {
        /* Use currently page access mechanism */
        MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, access_page);
        access_page = fixed_page;
    }
#if defined(__MODEL_USER__)
#else
#if defined(CONFIG_SDK_RTL8212B) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8214FB)
    if ((PHY_MODEL_ID_RTL8214F == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9) &&
        ((access_page == PHY_PAGE_0) || (access_page>=7 && access_page<=15)))
    {
        uint32  auto_1000f_shadow = 0;

        auto_1000f_shadow = (data >> 9) & 0x1;
        if ((ret = phy_8214fb_auto_1000f_set(unit, port, auto_1000f_shadow)) != RT_ERR_OK)
        {
            RT_LOG(LOG_WARNING, MOD_HAL, "phy_8214fb_auto_1000f_set(unit=%d, port=%d) failed", unit, port);
            return ret;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC)
    if ((PHY_MODEL_ID_RTL8218B_EXT == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9))
    {
        uint32  auto_1000f_shadow = 0;

        auto_1000f_shadow = (data >> 9) & 0x1;
        if ((ret = phy_8218b_auto_1000f_set(unit, port, auto_1000f_shadow)) != RT_ERR_OK)
        {
            RT_LOG(LOG_WARNING, MOD_HAL, "phy_8218b_auto_1000f_set(unit=%d, port=%d) failed", unit, port);
            return ret;
        }
    }
#endif
#endif  /* defined(__MODEL_USER__) */

    ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, access_page, phy_reg, data);

    /* E0007313 */
    if (((PHY_MODEL_ID_RTL8218 == pHalCtrl->pPhy_ctrl[port]->phy_model_id) ||
        (PHY_MODEL_ID_RTL8218_TC == pHalCtrl->pPhy_ctrl[port]->phy_model_id)) && (access_page != fixed_page))
    {
        MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 0, 31, 0);
    }
    /* End of E0007313 */
    return ret;
} /* end of hal_miim_write */


/* Function Name:
 *      hal_miim_park_read
 * Description:
 *      Get PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_park_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData)
{
    int32   ret = RT_ERR_FAILED;
    uint32  access_page, fixed_page;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, parkPage=0x%x, phy_reg=0x%x", unit, port, page, parkPage, phy_reg);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_park_read)
        return RT_ERR_FAILED;

    if (NULL == pHalCtrl->pPhy_ctrl[port])
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_park_read(unit, port, page, parkPage, phy_reg, pData);
        return ret;
    }

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);

    access_page = page;

    ret = MACDRV(pHalCtrl)->fMdrv_miim_park_read(unit, port, access_page, parkPage, phy_reg, pData);

#if defined(__MODEL_USER__)
    *pData = 0;
#else
#if defined(CONFIG_SDK_RTL8212B) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8214FB)
    if ((PHY_MODEL_ID_RTL8214F == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9) &&
        ((access_page == PHY_PAGE_0) || (access_page>=7 && access_page<=15)))
    {
        uint32  auto_1000f_shadow = 0;

        if (((*pData)&0x0200) == 0)
        {
            if ((ret = phy_8214fb_auto_1000f_get(unit, port, &auto_1000f_shadow)) != RT_ERR_OK)
            {
                RT_LOG(LOG_WARNING, MOD_HAL, "phy_8214fb_auto_1000f_get(unit=%d, port=%d) failed", unit, port);
                return ret;
            }
            (*pData) |= (auto_1000f_shadow<<9);
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC)
    if ((PHY_MODEL_ID_RTL8218B_EXT == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9))
    {
        uint32  auto_1000f_shadow = 0;

        if (((*pData)&0x0200) == 0)
        {
            if ((ret = phy_8218b_auto_1000f_get(unit, port, &auto_1000f_shadow)) != RT_ERR_OK)
            {
                RT_LOG(LOG_WARNING, MOD_HAL, "phy_8218b_auto_1000f_get(unit=%d, port=%d) failed", unit, port);
                return ret;
            }
            (*pData) |= (auto_1000f_shadow<<9);
        }
    }
#endif
#endif  /* defined(__MODEL_USER__) */

    return ret;
} /* end of hal_miim_read */


/* Function Name:
 *      hal_miim_park_write
 * Description:
 *      Set PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_park_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data)
{
    int32   ret = RT_ERR_FAILED;
    uint32  access_page, fixed_page;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, parkPage=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, parkPage, phy_reg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_park_write)
        return RT_ERR_FAILED;

    if (NULL == pHalCtrl->pPhy_ctrl[port])
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_park_write(unit, port, page, parkPage, phy_reg, data);
        return ret;
    }

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);

    access_page = page;
#if defined(__MODEL_USER__)
#else
#if defined(CONFIG_SDK_RTL8212B) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8214FB)
    if ((PHY_MODEL_ID_RTL8214F == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9) &&
        ((access_page == PHY_PAGE_0) || (access_page>=7 && access_page<=15)))
    {
        uint32  auto_1000f_shadow = 0;

        auto_1000f_shadow = (data >> 9) & 0x1;
        if ((ret = phy_8214fb_auto_1000f_set(unit, port, auto_1000f_shadow)) != RT_ERR_OK)
        {
            RT_LOG(LOG_WARNING, MOD_HAL, "phy_8214fb_auto_1000f_set(unit=%d, port=%d) failed", unit, port);
            return ret;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC)
    if ((PHY_MODEL_ID_RTL8218B_EXT == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
        (PHY_REV_NO_B == pHalCtrl->pPhy_ctrl[port]->phy_rev_id) && (phy_reg == 9))
    {
        uint32  auto_1000f_shadow = 0;

        auto_1000f_shadow = (data >> 9) & 0x1;
        if ((ret = phy_8218b_auto_1000f_set(unit, port, auto_1000f_shadow)) != RT_ERR_OK)
        {
            RT_LOG(LOG_WARNING, MOD_HAL, "phy_8218b_auto_1000f_set(unit=%d, port=%d) failed", unit, port);
            return ret;
        }
    }
#endif
#endif  /* defined(__MODEL_USER__) */

    ret = MACDRV(pHalCtrl)->fMdrv_miim_park_write(unit, port, access_page, parkPage, phy_reg, data);

    return ret;
} /* end of hal_miim_write */

/* Function Name:
 *      hal_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data)
{
    uint32  access_page;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%x, page=0x%x, phy_reg=0x%x, data=0x%x", unit, portmask.bits[0], page, phy_reg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_portmask_write)
        return RT_ERR_FAILED;

    /* E0005375 */
    /* Patch for RTL8329 switch + RTL8208 PHY
     * Access PHY by currently page id
     */
//    if (NULL == pHalCtrl->pPhy_ctrl[port])
//    {
//        return RT_ERR_FAILED;
//    }
//
//    if (((PHY_MODEL_ID_RTL8208G == pHalCtrl->pPhy_ctrl[port]->phy_model_id) &&
//         (RTL8329M_CHIP_ID == pHalCtrl->chip_id || RTL8329_CHIP_ID == pHalCtrl->chip_id)) ||
//         (PHY_MODEL_ID_RTL8212F == pHalCtrl->pPhy_ctrl[port]->phy_model_id))
//    {
//        access_page = PHY_PAGE_31;   /* Patch case: access the currently page */
//    }
//    else
//    {
        access_page = page;  /* Normal case: access the page number */
//    }
//    /* End of E0005375 */

    return (MACDRV(pHalCtrl)->fMdrv_miim_portmask_write(unit, portmask, access_page, phy_reg, data));
} /* end of hal_miim_portmask_write */

/* Function Name:
 *      hal_miim_broadcast_write
 * Description:
 *      Set PHY registers with broadcast mechanism.
 * Input:
 *      unit    - unit id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. page valid range is 0 ~ 31
 *      2. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_broadcast_write(
    uint32      unit,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, reg=0x%x \
           data=0x%x", unit, page, phy_reg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_broadcast_write)
        return RT_ERR_FAILED;
    else
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_broadcast_write(unit, page, phy_reg, data);
        return ret;
    }
} /* end of hal_miim_extParkPage_write */

/* Function Name:
 *      hal_miim_extParkPage_read
 * Description:
 *      Get PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_extParkPage_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mainPage,
    uint32      extPage,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData)
{
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x",
           unit, port, mainPage, extPage, parkPage, phy_reg);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_extParkPage_read)
        return RT_ERR_FAILED;
    else
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_extParkPage_read(unit, port, mainPage, extPage, parkPage, phy_reg, pData);
        return ret;
    }
} /* end of hal_miim_extParkPage_read */


/* Function Name:
 *      hal_miim_extParkPage_write
 * Description:
 *      Set PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_extParkPage_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mainPage,
    uint32      extPage,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data)
{
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x \
           data=0x%x", unit, port, mainPage, extPage, parkPage, phy_reg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_extParkPage_write)
        return RT_ERR_FAILED;
    else
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_extParkPage_write(unit, port, mainPage, extPage, parkPage, phy_reg, data);
        return ret;
    }
} /* end of hal_miim_extParkPage_write */


/* Function Name:
 *      hal_miim_extParkPage_portmask_write
 * Description:
 *      Set PHY registers in those portmask.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg  - PHY register
 *      data     - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_extParkPage_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mainPage,
    uint32          extPage,
    uint32          parkPage,
    uint32          phy_reg,
    uint32          data)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%8x 0x%8x, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x \
           data=0x%x", unit, portmask.bits[1], portmask.bits[0], mainPage, extPage, parkPage, phy_reg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_extParkPage_portmask_write)
        return RT_ERR_FAILED;

    return (MACDRV(pHalCtrl)->fMdrv_miim_extParkPage_portmask_write(unit, portmask, mainPage, extPage, parkPage, phy_reg, data));
} /* end of hal_miim_extParkPage_portmask_write */

/* Function Name:
 *      hal_miim_mmd_read
 * Description:
 *      Get PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData)
{
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x, reg=0x%x",
           unit, port, mmdAddr, mmdReg);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_mmd_read)
        return RT_ERR_FAILED;
    else
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, mmdAddr, mmdReg, pData);
        return ret;
    }
} /* end of hal_miim_read */


/* Function Name:
 *      hal_miim_mmd_write
 * Description:
 *      Set PHY registers.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data)
{
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, port, mmdAddr, mmdReg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_mmd_write)
        return RT_ERR_CHIP_NOT_SUPPORTED;
    else
    {
        ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_write(unit, port, mmdAddr, mmdReg, data);
        return ret;
    }
} /* end of hal_miim_mmd_write */


/* Function Name:
 *      hal_miim_mmd_portmask_write
 * Description:
 *      Set PHY registers in those portmask.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      mmdAddr  - mmd device address
 *      mmdReg   - mmd reg id
 *      data     - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
hal_miim_mmd_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mmdAddr,
    uint32          mmdReg,
    uint32          data)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%8x 0x%8x, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, portmask.bits[1], portmask.bits[0], mmdAddr, mmdReg, data);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (NULL == MACDRV(pHalCtrl)->fMdrv_miim_mmd_portmask_write)
        return RT_ERR_FAILED;

    return (MACDRV(pHalCtrl)->fMdrv_miim_mmd_portmask_write(unit, portmask, mmdAddr, mmdReg, data));
} /* end of hal_miim_mmd_portmask_write */

/* Function Name:
 *      hal_miim_pollingEnable_get
 * Description:
 *      Get the mac polling PHY status of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
hal_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PORT_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (MACDRV(pHalCtrl)->fMdrv_miim_pollingEnable_get(unit, port, pEnabled));
} /* end of hal_miim_pollingEnable_get */

/* Function Name:
 *      hal_miim_pollingEnable_set
 * Description:
 *      Set the mac polling PHY status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enabled - mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
hal_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PORT_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (MACDRV(pHalCtrl)->fMdrv_miim_pollingEnable_set(unit, port, enabled));
} /* end of hal_miim_pollingEnable_set */

/* Function Name:
 *      hal_miim_globalPollingEnable_get
 * Description:
 *      Get the global mac polling PHY status.
 * Input:
 *      unit     - unit id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
hal_miim_globalPollingEnable_get(
    uint32          unit,
    rtk_enable_t    *pEnabled)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d", unit);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    return (MACDRV(pHalCtrl)->fMdrv_miim_globalPollingEnable_get(unit, pEnabled));
} /* end of hal_miim_globalPollingEnable_get */

/* Function Name:
 *      hal_miim_globalPollingEnable_set
 * Description:
 *      Set the global mac polling PHY status.
 * Input:
 *      unit    - unit id
 *      enabled - mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
hal_miim_globalPollingEnable_set(
    uint32          unit,
    rtk_enable_t    enabled)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d", unit);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    return (MACDRV(pHalCtrl)->fMdrv_miim_globalPollingEnable_set(unit, enabled));
} /* end of hal_miim_globalPollingEnable_set */

/* Function Name:
 *      phy_media_get
 * Description:
 *      Get PHY media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 */
int32
phy_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_media_get(unit, port, pMedia));
} /* end of phy_media_get */


/* Function Name:
 *      phy_media_set
 * Description:
 *      Get PHY media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 */
int32
phy_media_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, media=%d", unit, port, media);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_media_set(unit, port, media));
} /* end of phy_media_set */


/* Function Name:
 *      phy_autoNegoEnable_get
 * Description:
 *      Get auto negotiation enable status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable - pointer to PHY auto negotiation status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_autoNegoEnable_get(unit, port, pEnable));
} /* end of phy_autoNegoEnable_get */

/* Function Name:
 *      phy_autoNegoEnable_set
 * Description:
 *      Set auto negotiation enable status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - enable PHY auto negotiation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_autoNegoEnable_set(unit, port, enable));
} /* end of phy_autoNegoEnable_set */

/* Function Name:
 *      phy_autoNegoAbility_get
 * Description:
 *      Get ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_autoNegoAbility_get(unit, port, pAbility));
}

/* Function Name:
 *      phy_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, pAbility=0x%x", unit, port, pAbility);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_autoNegoAbility_set(unit, port, pAbility));
}

/* Function Name:
 *      phy_duplex_get
 * Description:
 *      Get duplex mode status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDuplex - pointer to PHY duplex mode status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_duplex_get(unit, port, pDuplex));
} /* end of phy_duplex_get */

/* Function Name:
 *      phy_duplex_set
 * Description:
 *      Set duplex mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      duplex        - duplex mode of the port, full or half
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, duplex=%d", unit, port, duplex);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_duplex_set(unit, port, duplex));
} /* end of phy_duplex_set */

/* Function Name:
 *      phy_speed_get
 * Description:
 *      Get link speed status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_speed_get(unit, port, pSpeed));
} /* end of phy_speed_get */


/* Function Name:
 *      phy_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, speed=%d", unit, port, speed);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_speed_set(unit, port, speed));
} /* end of phy_speed_set */

/* Function Name:
 *      phy_enable_set
 * Description:
 *      Set PHY interface status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_enable_set(unit, port, enable);
} /* end of phy_enable_set */

/* Function Name:
 *      phy_rtct_start
 * Description:
 *      Start PHY interface RTCT test of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - chip not supported
 * Note:
 *      None
 */
int32
phy_rtct_start(uint32 unit, rtk_port_t port)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_rtct_start(unit, port));
} /* end of phy_rtct_start */

/* Function Name:
 *      phy_rtctResult_get
 * Description:
 *      Get test result of RTCT.
 * Input:
 *      unit        - unit id
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_rtctResult_get(unit, port, pRtctResult));
} /* end of phy_rtctResult_get */

/* Function Name:
 *      phy_greenEnable_get
 * Description:
 *      Get the status of green feature of the specific port in the specific unit
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable - pointer to status of green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
phy_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_greenEnable_get(unit, port, pEnable);
} /* end of phy_greenEnable_get */

/* Function Name:
 *      phy_greenEnable_set
 * Description:
 *      Set the status of green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
phy_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_greenEnable_set(unit, port, enable);
} /* end of phy_greenEnable_set */

/* Function Name:
 *      phy_eeeEnable_get
 * Description:
 *      Get enable status of EEE function in the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of EEE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_eeeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_eeeEnable_get(unit, port, pEnable));
} /* end of phy_eeeEnable_get */

/* Function Name:
 *      phy_eeeEnable_set
 * Description:
 *      Set enable status of EEE function in the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of EEE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_eeeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_eeeEnable_set(unit, port, enable);
} /* end of phy_eeeEnable_set */

/* Function Name:
 *      phy_crossOverMode_get
 * Description:
 *      Get cross over mode in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
phy_crossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_crossOverMode_get(unit, port, pMode));
} /* end of phy_crossOverMode_get */

/* Function Name:
 *      phy_crossOverMode_set
 * Description:
 *      Set cross over mode in the specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - cross over mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
phy_crossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, mode);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_crossOverMode_set(unit, port, mode);
} /* end of phy_crossOverMode_set */

/* Function Name:
 *      phy_crossOverStatus_get
 * Description:
 *      Get cross over status in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pStatus - pointer to cross over mode status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PHY_FIBER_LINKUP - This feature is not supported in this mode
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_STATUS_MDI
 *      - PORT_CROSSOVER_STATUS_MDIX
 */
int32
phy_crossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_crossOverStatus_get(unit, port, pStatus));
} /* end of phy_crossOverStatus_get */


/* Function Name:
 *      phy_fiber_media_get
 * Description:
 *      Get PHY fiber media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
phy_fiber_media_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiber_media_get(unit, port, pMedia));
} /* end of phy_fiber_media_get */


/* Function Name:
 *      phy_fiber_media_set
 * Description:
 *      Get PHY fiber media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
phy_fiber_media_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, media=%d", unit, port, media);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiber_media_set(unit, port, media));
} /* end of phy_fiber_media_set */

/* Function Name:
 *      phy_linkDownPowerSavingEnable_get
 * Description:
 *      Get the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of link-down power saving
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_linkDownPowerSavingEnable_get(unit, port, pEnable);
} /* end of phy_linkDownPowerSavingEnable_get */

/* Function Name:
 *      phy_linkDownPowerSavingEnable_set
 * Description:
 *      Set the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-down power saving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_linkDownPowerSavingEnable_set(unit, port, enable);
} /* end of phy_linkDownPowerSavingEnable_set */

/* Function Name:
 *      phy_broadcastEnable_set
 * Description:
 *      Set enable status of broadcast mode
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - broadcast enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_broadcastEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_broadcastEnable_set(unit, port, enable);
} /* end of phy_broadcastEnable_set */

/* Function Name:
 *      phy_broadcastID_set
 * Description:
 *      Set broadcast ID
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      broadcastID   - broadcast ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_broadcastID_set(uint32 unit, rtk_port_t port, uint32 broadcastID)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, broadcastID=%d", unit, port, broadcastID);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_broadcastID_set(unit, port, broadcastID);
} /* end of phy_broadcastID_set */

/* Function Name:
 *      phy_gigaLiteEnable_get
 * Description:
 *      Get the status of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of link-down power saving
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_gigaLiteEnable_get(unit, port, pEnable);
} /* end of phy_gigaLiteEnable_get */

/* Function Name:
 *      phy_gigaLiteEnable_set
 * Description:
 *      Set the status of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-down power saving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_gigaLiteEnable_set(unit, port, enable);
} /* end of phy_gigaLiteEnable_set */

/* Function Name:
 *      phy_eeepEnable_get
 * Description:
 *      Get enable status of EEEP function in the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of EEEP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_eeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_eeepEnable_get(unit, port, pEnable));
} /* end of phy_eeepEnable_get */

/* Function Name:
 *      phy_eeepEnable_set
 * Description:
 *      Set enable status of EEEP function in the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of EEEP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_eeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_eeepEnable_set(unit, port, enable);
} /* end of phy_eeepEnable_set */


/* Function Name:
 *      phy_patch_set
 * Description:
 *      Patch the PHY.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_patch_set(uint32 unit, rtk_port_t port)
{
    hal_control_t   *pHalCtrl;
    int32           ret;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    ret = pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_patch_set(unit, port);
    if (RT_ERR_OK != ret)
        return ret;

    return ret;
} /* end of phy_patch_set */

/* Function Name:
 *      hal_mac_serdes_rst
 * Description:
 *      Reset MAC serdes.
 * Input:
 *      unit   - unit id
 *      sds_no   - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_OUT_OF_RANGE  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
hal_mac_serdes_rst(uint32 unit, uint32 sds_no)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, sds_no=%d", unit, sds_no);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    return (MACDRV(pHalCtrl)->fMdrv_mac_serdes_rst(unit, sds_no));
} /* end of phy_patch_set */

/* Function Name:
 *      phy_chk_rst_status
 * Description:
 *      Check the PHY had been reset or NOT.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      status - 1 : Had been reset
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED    - PHY doesn't support this feature
 * Note:
 *      None
 */
int32
phy_chk_rst_status(uint32 unit, rtk_port_t port, uint32 * status)
{
    hal_control_t   *pHalCtrl;
#if defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC)
    uint32  phy_data0, phy_data1;
#endif

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

#if defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC)
    if((pHalCtrl->pPhy_ctrl[port]->pPhydrv->phydrv_index == RT_PHYDRV_RTL8218B))
    {
        hal_miim_write(unit,  port, 0x0,  27, 0x801e);
        hal_miim_read(unit,  port,  0x0,  28, &phy_data0);
        /*Double check*/
        hal_miim_read(unit,  port,  0x0,  28, &phy_data1);
        if((0x1 != phy_data0) && (0x1 != phy_data1))
        {
            *status = 0x1; /*Had been reset*/
        }
        else
        {
            *status = 0x0;
        }
    }
    else if(pHalCtrl->pPhy_ctrl[port]->pPhydrv->phydrv_index == RT_PHYDRV_RTL8214FC_MP ||
            (pHalCtrl->pPhy_ctrl[port]->pPhydrv->phydrv_index == RT_PHYDRV_RTL8218FB_MP))
    {
             /* Set default value for reset checking in waMon */
             hal_miim_write(unit, port, 0, 30, 8);
             hal_miim_read(unit, port, 0x268, 16, &phy_data0);
             hal_miim_read(unit, port, 0x268, 16, &phy_data1);
             hal_miim_write(unit, port, 0, 30, 0);
             phy_data0 &= 0xF000;
             phy_data1 &= 0xF000;

        if((0xA000 != phy_data0) && (0xA000 != phy_data1))
        {
            *status = 0x1; /*Had been reset*/
        }
        else
        {
            *status = 0x0;
        }
    }
#endif

    return RT_ERR_OK;
} /* end of phy_patch_set */

/* Function Name:
 *      phy_fiberDownSpeedEnable_get
 * Description:
 *      Get fiber down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of fiber down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiberDownSpeedEnable_get(unit, port, pEnable));
}

/* Function Name:
 *      phy_fiberDownSpeedEnable_set
 * Description:
 *      Set fiber down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of fiber down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiberDownSpeedEnable_set(unit, port, enable));
}

/* Function Name:
 *      phy_downSpeedEnable_get
 * Description:
 *      Get down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_downSpeedEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_downSpeedEnable_get(unit, port, pEnable));
}

/* Function Name:
 *      phy_downSpeedEnable_set
 * Description:
 *      Set down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_downSpeedEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_downSpeedEnable_set(unit, port, enable));
}


/* Function Name:
 *      phy_drv_chk
 * Description:
 *      Check the driver is used for which PHY.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      drv_type : Chip version
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED    - PHY doesn't support this feature
 * Note:
 *      None
 */
int32
phy_drv_chk(uint32 unit, rtk_port_t port, int32 * drv_type)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    *drv_type = pHalCtrl->pPhy_ctrl[port]->pPhydrv->phydrv_index;

    return RT_ERR_OK;
} /* end of phy_drv_chk */

/* Function Name:
 *      phy_fiberNwayForceLinkEnable_get
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of fiber nway force link
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_fiberNwayForceLinkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiberNwayForceLinkEnable_get(unit, port, pEnable));
}

/* Function Name:
 *      phy_fiberNwayForceLinkEnable_set
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of fiber nway force link
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_fiberNwayForceLinkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiberNwayForceLinkEnable_set(unit, port, enable));
}

/* Function Name:
 *      phy_fiberOAMLoopBackEnable_set
 * Description:
 *      Set fiber port OAM Loopback featrue enable or not
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of fiber down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_fiberOAMLoopBackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiberOAMLoopBackEnable_set(unit, port, enable));
}


/* Function Name:
 *      phy_ptpSwitchMacAddr_get
 * Description:
 *      Get the Switch MAC address setting of PHY of the specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      pSwitchMacAddr - point to the Switch MAC Address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_ptpSwitchMacAddr_get(uint32 unit, rtk_port_t port, rtk_mac_t *pSwitchMacAddr)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpSwitchMacAddr_get(unit, port, pSwitchMacAddr));
}

/* Function Name:
 *      phy_ptpSwitchMacAddr_set
 * Description:
 *      Set the Switch MAC address setting of PHY of the specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      pSwitchMacAddr - point to the Switch MAC Address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_ptpSwitchMacAddr_set(uint32 unit, rtk_port_t port, rtk_mac_t *pSwitchMacAddr)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpSwitchMacAddr_set(unit, port, pSwitchMacAddr));
}

/* Function Name:
 *      phy_ptpRefTime_get
 * Description:
 *      Get the reference time of PHY of the specified port.
 * Input:
 *      unit       - unit id
 * Output:
 *      pTimeStamp - pointer buffer of the reference time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_ptpRefTime_get(uint32 unit, rtk_port_t port, rtk_time_timeStamp_t *pTimeStamp)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpRefTime_get(unit, port, pTimeStamp));
}

/* Function Name:
 *      phy_ptpRefTime_set
 * Description:
 *      Set the reference time of PHY of the specified port.
 * Input:
 *      unit      - unit id
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_ptpRefTime_set(uint32 unit, rtk_port_t port, rtk_time_timeStamp_t timeStamp)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpRefTime_set(unit, port, timeStamp));
}

/* Function Name:
 *      phy_ptpRefTimeAdjust_set
 * Description:
 *      Adjust the reference time of PHY of the specified port.
 * Input:
 *      unit      - unit id
 *      port    - port id
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      sign=0 for positive adjustment, sign=1 for negative adjustment.
 */
int32
phy_ptpRefTimeAdjust_set(uint32 unit, rtk_port_t port, uint32 sign, rtk_time_timeStamp_t timeStamp)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpRefTimeAdjust_set(unit, port, sign, timeStamp));
}

/* Function Name:
 *      phy_ptpRefTimeEnable_get
 * Description:
 *      Get the enable state of reference time of PHY of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_ptpRefTimeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpRefTimeEnable_get(unit, port, pEnable));
}

/* Function Name:
 *      phy_ptpRefTimeEnable_set
 * Description:
 *      Set the enable state of reference time of PHY of the specified port.
 * Input:
 *      unit   - unit id
 *      port    - port id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_ptpRefTimeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpRefTimeEnable_set(unit, port, enable));
}

/* Function Name:
 *      phy_ptpEnable_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_ptpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpEnable_get(unit, port, pEnable));
}

/* Function Name:
 *      phy_ptpEnable_set
 * Description:
 *      Set PTP status of the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_ptpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpEnable_set(unit, port, enable));
}

/* Function Name:
 *      phy_ptpRxTimestamp_get
 * Description:
 *      Get PTP Rx timstamp according to the PTP identifier on the dedicated port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      identifier - indentifier of PTP packet
 * Output:
 *      pTimeStamp - pointer buffer of TIME timestamp
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_ptpRxTimestamp_get(uint32 unit, rtk_port_t port, rtk_time_ptpIdentifier_t identifier, rtk_time_timeStamp_t *pTimeStamp)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpRxTimestamp_get(unit, port, identifier, pTimeStamp));
}

/* Function Name:
 *      phy_ptpTxTimestamp_get
 * Description:
 *      Get PTP Tx timstamp according to the PTP identifier on the dedicated port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      identifier - indentifier of PTP packet
 * Output:
 *      pTimeStamp - pointer buffer of TIME timestamp
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_ptpTxTimestamp_get(uint32 unit, rtk_port_t port, rtk_time_ptpIdentifier_t identifier, rtk_time_timeStamp_t *pTimeStamp)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d", unit, port);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_ptpTxTimestamp_get(unit, port, identifier, pTimeStamp));
}

/* Function Name:
 *      phy_masterSlave_get
 * Description:
 *      Get PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pMasterSlaveCfg     - pointer to the PHY master slave configuration
 *      pMasterSlaveActual  - pointer to the PHY master slave actual link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      This function only works on giga/ 10g port to get its master/slave mode configuration.
 */
int32
phy_masterSlave_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   *pMasterSlaveCfg,
    rtk_port_masterSlave_t   *pMasterSlaveActual)
{
    int32 ret;
    uint32 data;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;


    if (HAL_IS_10GE_PORT(unit, port))
    {
        if ((ret = hal_miim_mmd_read(unit, port, 7, 32, &data)) != RT_ERR_OK)
            return ret;

        data = (data >> 14) & 0x3;
        switch(data)
        {
            case 0x2:
                *pMasterSlaveCfg = PORT_SLAVE_MODE;
                break;
            case 0x3:
                *pMasterSlaveCfg = PORT_MASTER_MODE;
                break;
            default:
                *pMasterSlaveCfg = PORT_AUTO_MODE;
                break;
        }

        if ((ret = hal_miim_mmd_read(unit, port, 7, 33, &data)) != RT_ERR_OK)
            return ret;

        data = (data >> 14) & 0x1;
        switch(data)
        {
            case 0x0:
                *pMasterSlaveActual = PORT_SLAVE_MODE;
                break;
            case 0x1:
                *pMasterSlaveActual = PORT_MASTER_MODE;
                break;
            default:
                *pMasterSlaveActual = PORT_SLAVE_MODE;
                break;
        }
    }
    else if (HAL_IS_GE_PORT(unit, port))
    {
		return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_slaveMaster_get(unit, port, pMasterSlaveCfg, pMasterSlaveActual));
    }
    else
    {
         return RT_ERR_INPUT;
    }

    return ret;
}/* end of phy_masterSlave_get */

/* Function Name:
 *      phy_masterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      masterSlave         - PHY master slave configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_INPUT         - invalid input parameter
 * Note:
 *      None
 */
int32
phy_masterSlave_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   masterSlave)
{
    int32 ret;
    uint32 data;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, masterSlave=%d", unit, port, masterSlave);

    if (HAL_IS_10GE_PORT(unit, port))
    {
        if ((ret = hal_miim_mmd_read(unit, port, 7, 32, &data)) != RT_ERR_OK)
            return ret;

        data &= ~ (0x3 << 14);
        switch(masterSlave)
        {
            case PORT_AUTO_MODE:
                data |= 0x0 << 14;
                break;
            case PORT_SLAVE_MODE:
                data |= 0x2 << 14;
                break;
            case PORT_MASTER_MODE:
                data |= 0x3 << 14;
                break;
            default:
                return RT_ERR_INPUT;
        }

        if ((ret = hal_miim_mmd_write(unit, port, 7, 32, data)) != RT_ERR_OK)
            return ret;

    }
    else if (HAL_IS_GE_PORT(unit, port))
    {
		return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_slaveMaster_set(unit, port, masterSlave));
    }
    else
    {
        return RT_ERR_INPUT;
    }

    return ret;
}/* end of phy_masterSlave_set */

/* Function Name:
 *      phy_fiberInternalLoopBackEnable_set
 * Description:
 *      Set fiber port Internal Loopback featrue enable or not
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of fiber down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_fiberInternalLoopBackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    hal_control_t   *pHalCtrl;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, enable=%d", unit, port, enable);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if (!HAL_IS_PHY_EXIST(unit, port))
        return RT_ERR_FAILED;

    return (pHalCtrl->pPhy_ctrl[port]->pPhydrv->fPhydrv_fiberInternalLoopBackEnable_set(unit, port, enable));
} /* end of phy_fiberInternalLoopBackEnable_set */


