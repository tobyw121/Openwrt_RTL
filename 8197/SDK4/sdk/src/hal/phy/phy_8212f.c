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
 * $Revision: 53061 $
 * $Date: 2014-11-14 16:25:10 +0800 (Fri, 14 Nov 2014) $
 *
 * Purpose : PHY 8212F Driver APIs.
 *
 * Feature : PHY 8212F Driver APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <osal/time.h>
#include <hal/common/halctrl.h>
#include <hal/common/miim.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_common.h>
#include <hal/phy/phy_8212f.h>
#include <rtk/default.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
rt_phydrv_t phy_8212Fdrv_ge =
{
    RT_PHYDRV_RTL8212F,
    phy_8212f_init,
    phy_8212f_media_get,
    phy_8212f_media_set,
    phy_8212f_autoNegoEnable_get,
    phy_8212f_autoNegoEnable_set,
    phy_8212f_autoNegoAbility_get,
    phy_8212f_autoNegoAbility_set,
    phy_8212f_duplex_get,
    phy_8212f_duplex_set,
    phy_8212f_speed_get,
    phy_8212f_speed_set,
    phy_8212f_enable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_rtctResult_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8212f_greenEnable_get,
    phy_8212f_greenEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8212f_crossOverMode_get,
    phy_8212f_crossOverMode_set,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,     
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t))phy_common_unavail,
    phy_8212f_linkDownPowerSavingEnable_get,
    phy_8212f_linkDownPowerSavingEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8212f_gigaLiteEnable_get,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,    
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8212Fdrv_ge */

/*
 * Function Declaration
 */

/* Function Name:
 *      phy_8212f_media_get
 * Description:
 *      Get PHY 8212F media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 */
int32
phy_8212f_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    uint32  val, cur_page_mode, dflt_page_mode = 0, is_link_up = 0;
    uint32  fixed_page;
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        is_link_up = 1;
        if ((ret = hal_miim_read(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, &cur_page_mode)) != RT_ERR_OK)
            return ret;
    }
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &val)) != RT_ERR_OK)
        return ret;
    switch (val & 0x3)
    {
        case 0:
            *pMedia = PORT_MEDIA_COPPER;
            dflt_page_mode = 0x40;
            break;
        case 1:
            *pMedia = PORT_MEDIA_FIBER;
            dflt_page_mode = 0x50;
            break;
        case 2:
            *pMedia = PORT_MEDIA_COPPER_AUTO;
            dflt_page_mode = 0x40;
            break;
        case 3:
            *pMedia = PORT_MEDIA_FIBER_AUTO;
            dflt_page_mode = 0x50;
            break;
    }
    if (1 == is_link_up)
    {
        if ((ret = hal_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, cur_page_mode)) != RT_ERR_OK)
            return ret;
    }
    else
    {
        if ((ret = hal_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, dflt_page_mode)) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of phy_8212f_media_get */

/* Function Name:
 *      phy_8212f_media_set
 * Description:
 *      Get PHY 8212F media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 *      2. there are 3 major steps for change the media from original to new
 *         - turn off original media
 *         - turn on new media
 *         - set new media to first priority
 */
int32
phy_8212f_media_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    uint32  val, cur_page_mode, is_link_up = 0, restore_page = 0;
    uint32  old_state = 0;
    uint32  fixed_page;
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        is_link_up = 1;
        if ((ret = hal_miim_read(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, &cur_page_mode)) != RT_ERR_OK)
            return ret;
    }
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &val)) != RT_ERR_OK)
        return ret;
    /* Check the auto-sense and prefered media is same as original setting? */
    if ((val & 0x3) == media) /* media no change */
    {
        if (1 == is_link_up)
            restore_page = cur_page_mode;
        else
            restore_page = (0x40 |((val & 0x1)<<4));
        return hal_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, restore_page);
    }

    /* media have changed */
    val &= ~(0x2);
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40 |((val & 0x1)<<4)))) != RT_ERR_OK)
        return ret;

    /* Save currently media PHY power down status and power down the media */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
        return ret;
    if (0x0000 == (val & 0x0800))
    {
        old_state = 1; /* normal operation */
    }
    val |= 0x0800;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, val)) != RT_ERR_OK)
        return ret;

    /* configure media to input argument */
    switch (media)
    {
        case PORT_MEDIA_COPPER:
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(0x3);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x40)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_MEDIA_COPPER */

        case PORT_MEDIA_FIBER:
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(0x2);
            val |= (0x1);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x50)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_MEDIA_FIBER */

        case PORT_MEDIA_COPPER_AUTO:
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &val)) != RT_ERR_OK)
                return ret;
            val |= (0x2);
            val &= ~(0x1);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x40)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_MEDIA_COPPER_AUTO */

        case PORT_MEDIA_FIBER_AUTO:
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &val)) != RT_ERR_OK)
                return ret;
            val |= (0x3);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x50)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_MEDIA_FIBER_AUTO */

        default:
            break;
    }

    /* Restore PHY power down status to new media */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
        return ret;
    if (old_state)
        val &= 0xF7FF;
    else
        val |= 0x0800;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, val)) != RT_ERR_OK)
        return ret;
    return RT_ERR_OK;
} /* end of phy_8212f_media_set */

/* Function Name:
 *      phy_8212f_init
 * Description:
 *      Initialize PHY 8212F.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_init(uint32 unit, rtk_port_t port)
{
    uint32  val, phyData;
    uint32  def_media = PORT_MEDIA_COPPER;
    uint32  def_port_state = RTK_DEFAULT_PORT_ADMIN_ENABLE; /* enable(1), disable(0) */
    uint32  fixed_page;
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if ((NULL == MACDRV(pHalCtrl)->fMdrv_miim_read) ||
        (NULL == MACDRV(pHalCtrl)->fMdrv_miim_write))
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &val)) != RT_ERR_OK)
        return ret;

    val &= ~(0x3);

    if (PORT_MEDIA_FIBER == def_media)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
            return ret;
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x40)) != RT_ERR_OK)
            return ret;
        /* Set the default port state */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;
        if (def_port_state)
            phyData &= ~(0x1 << 11);
        else
            phyData |= (0x1 << 11);
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;
        val |= 0x1;
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
            return ret;
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x50)) != RT_ERR_OK)
            return ret;
        /* Set the default port state */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;
        if (def_port_state)
            phyData &= ~(0x1 << 11);
        else
            phyData |= (0x1 << 11);
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;
    }
    else
    {
        val |= 0x1;
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
            return ret;
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x50)) != RT_ERR_OK)
            return ret;
        /* Set the default port state */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;
        if (def_port_state)
            phyData &= ~(0x1 << 11);
        else
            phyData |= (0x1 << 11);
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;
        val &= ~(0x1);
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
            return ret;
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x40)) != RT_ERR_OK)
            return ret;
        /* Set the default port state */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;
        if (def_port_state)
            phyData &= ~(0x1 << 11);
        else
            phyData |= (0x1 << 11);
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of phy_8212f_init */


/* Function Name:
 *      phy_8212f_autoNegoAbility_get
 * Description:
 *      Get ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData4;
    uint32  phyData9;
    uint32  restore_val, val;
    uint32  cur_page_mode;
    uint32  fixed_page;
    rtk_port_media_t media;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, &cur_page_mode)) != RT_ERR_OK)
            return ret;
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
            return ret;

        if (0x50 == cur_page_mode)
        {
            pAbility->FC = (phyData4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
            pAbility->AsyFC = (phyData4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
            pAbility->Half_10 = 0;
            pAbility->Full_10 = 0;
            pAbility->Half_100 = 0;
            pAbility->Full_100 = 0;
            pAbility->Half_1000 = (phyData4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
            pAbility->Full_1000 = (phyData4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;
        }
        else if (0x40 == cur_page_mode)
        {
            pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
            pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;

            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
                return ret;

            pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
    	    pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
    	    pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
    	    pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
            pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
            pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;
        }
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    if (restore_val & 0x1)
        media = PORT_MEDIA_FIBER;
    else
        media = PORT_MEDIA_COPPER;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        pAbility->FC = (phyData4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
        pAbility->AsyFC = (phyData4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
        pAbility->Half_10 = 0;
        pAbility->Full_10 = 0;
        pAbility->Half_100 = 0;
        pAbility->Full_100 = 0;
        pAbility->Half_1000 = (phyData4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
        pAbility->Full_1000 = (phyData4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;
    }
    else if (PORT_MEDIA_COPPER == media)
    {
        pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
        pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
            return ret;

        pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
	    pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
	    pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
	    pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
        pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
        pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8212f_autoNegoAbility_get */

/* Function Name:
 *      phy_8212f_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pAbility - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData0;
    uint32  phyData4;
    uint32  phyData9;
    uint32  restore_val, val;
    uint32  cur_page_mode;
    uint32  fixed_page;
    rtk_enable_t     enable;
    rtk_port_media_t media;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        phy_common_autoNegoEnable_get(unit, port, &enable);

        if ((ret = hal_miim_read(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, &cur_page_mode)) != RT_ERR_OK)
            return ret;

        /* set value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
            return ret;

        if (0x50 == cur_page_mode)
        {
            phyData4 = phyData4 & ~(_1000BaseX_Pause_R4_MASK | _1000BaseX_AsymmetricPause_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET)
                    | (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET);
            phyData4 = phyData4 & ~(_1000BaseX_HalfDuplex_R4_MASK | _1000BaseX_FullDuplex_R4_MASK);
            phyData4 = phyData4 | (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET)
                    | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
                return ret;
        }
        else if (0x40 == cur_page_mode)
        {
            phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->FC << Pause_R4_OFFSET)
                    | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
                return ret;

            phyData4 = phyData4 &
                    ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
                    | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
                    | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
                    | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

            phyData9 = phyData9 & ~(_1000Base_TFullDuplex_MASK | _1000Base_THalfDuplex_MASK);
            phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                       | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
                return ret;


            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
                return ret;
        }

        /* Force re-autonegotiation if AN is on */
        if (ENABLED == enable)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
                return ret;

            phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
            phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
                return ret;
        }
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    if (restore_val & 0x1)
        media = PORT_MEDIA_FIBER;
    else
        media = PORT_MEDIA_COPPER;

    phy_common_autoNegoEnable_get(unit, port, &enable);

    /* set value to CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        phyData4 = phyData4 & ~(_1000BaseX_Pause_R4_MASK | _1000BaseX_AsymmetricPause_R4_MASK);
        phyData4 = phyData4
                | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET)
                | (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET);
        phyData4 = phyData4 & ~(_1000BaseX_HalfDuplex_R4_MASK | _1000BaseX_FullDuplex_R4_MASK);
        phyData4 = phyData4 | (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET)
                | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
            return ret;
    }
    else if (PORT_MEDIA_COPPER == media)
    {
        phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
        phyData4 = phyData4
                | (pAbility->FC << Pause_R4_OFFSET)
                | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
            return ret;

        phyData4 = phyData4 &
                ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
        phyData4 = phyData4
                | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
                | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
                | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
                | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

        phyData9 = phyData9 & ~(_1000Base_TFullDuplex_MASK | _1000Base_THalfDuplex_MASK);
        phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                   | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
            return ret;


        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
            return ret;
    }

    /* Force re-autonegotiation if AN is on */
    if (ENABLED == enable)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8212f_autoNegoAbility_set */


/* Function Name:
 *      phy_8212f_speed_get
 * Description:
 *      Get link speed status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    uint32  fixed_page;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
                  | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
              | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8212f_speed_get */


/* Function Name:
 *      phy_8212f_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      speed - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    uint32  fixed_page;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* set value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
        phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;

    /* set value to CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8212f_speed_set */

/* Function Name:
 *      phy_8212f_enable_set
 * Description:
 *      Set PHY interface status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData, phyData_fiber, phyData_copper;
    uint32  val, restore_val;
    uint32  fixed_page;
    int32   ret;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        val = restore_val & ~(0x2);
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
            return ret;
        if (val & 0x1)
        {
            val &= ~(0x1);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x40)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_copper)) != RT_ERR_OK)
                return ret;
            phyData_copper &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_copper |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_copper)) != RT_ERR_OK)
                return ret;

            val |= (0x1);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x50)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_fiber)) != RT_ERR_OK)
                return ret;
            phyData_fiber &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_fiber |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_fiber)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            val |= (0x1);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x50)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_fiber)) != RT_ERR_OK)
                return ret;
            phyData_fiber &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_fiber |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_fiber)) != RT_ERR_OK)
                return ret;

            val &= ~(0x1);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, val)) != RT_ERR_OK)
                return ret;
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x40)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_copper)) != RT_ERR_OK)
                return ret;
            phyData_copper &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_copper |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_copper)) != RT_ERR_OK)
                return ret;
        }
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
            return ret;
        if (restore_val & 0x1)
        {
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x50)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, 0x40)) != RT_ERR_OK)
                return ret;
        }
    }
    else
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;

        phyData &= ~(PowerDown_MASK);
        if (DISABLED == enable)
            phyData |= (1 << PowerDown_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of phy_8212f_enable_set */

/* Function Name:
 *      phy_8212f_autoNegoEnable_get
 * Description:
 *      Get autonegotiation enable status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    uint32  fixed_page;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        if (phyData0 & AutoNegotiationEnable_MASK)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    if (phyData0 & AutoNegotiationEnable_MASK)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8212f_autoNegoEnable_get */

/* Function Name:
 *      phy_8212f_autoNegoEnable_set
 * Description:
 *      Set autonegotiation enable status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8212f_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    uint32  fixed_page;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* set value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(AutoNegotiationEnable_MASK | RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | ((enable << AutoNegotiationEnable_OFFSET) | (1 << RestartAutoNegotiation_OFFSET));

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(AutoNegotiationEnable_MASK | RestartAutoNegotiation_MASK);
    phyData0 = phyData0 | ((enable << AutoNegotiationEnable_OFFSET) | (1 << RestartAutoNegotiation_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8212f_autoNegoEnable_set */


/* Function Name:
 *      phy_8212f_duplex_get
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
phy_8212f_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    uint32  fixed_page;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        *pDuplex = (phyData0 & DuplexMode_MASK) >> DuplexMode_OFFSET;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, 0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    *pDuplex = (phyData0 & DuplexMode_MASK) >> DuplexMode_OFFSET;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8212f_duplex_get */

/* Function Name:
 *      phy_8212f_duplex_get
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
phy_8212f_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    uint32  fixed_page;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    fixed_page = HAL_MIIM_PAGE_ID_MAX(unit);
    if ((ret = hal_miim_read(unit, port, fixed_page, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* set value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(DuplexMode_MASK);
        phyData0 = phyData0 | (duplex << DuplexMode_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, 14, 17, &restore_val)) != RT_ERR_OK)
        return ret;

    if (restore_val & 0x2)
    {
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, (restore_val&~(0x2)))) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(DuplexMode_MASK);
    phyData0 = phyData0 | (duplex << DuplexMode_OFFSET);

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, 14, 17, restore_val)) != RT_ERR_OK)
        return ret;
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, fixed_page, PHY_PAGE_SELECTION_REG, (0x40|((restore_val&0x1)<<4)))) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8212f_duplex_set */

/* Function Name:
 *      phy_8212f_greenEnable_get
 * Description:
 *      Get the status of link-up green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to status of link-up green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      1. The RTL8212F is supported the per-port link-up green feature.
 */
int32
phy_8212f_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  base_port = 0;
    uint32  phyData;

    base_port = port - (port % PORT_NUM_IN_8212F);

    /* setting page 15, register 27 */
    if ((ret = hal_miim_read(unit, base_port, 15, 27, &phyData)) != RT_ERR_OK)
        return ret;
    if (((phyData >> (8*(port % PORT_NUM_IN_8212F))) & 0x82) == 0x82)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
} /* end of phy_8212f_greenEnable_get */

/* Function Name:
 *      phy_8212f_greenEnable_set
 * Description:
 *      Set the status of link-up green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-up green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      1. The RTL8212F is supported the per-port link-up green feature.
 */
int32
phy_8212f_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  base_port = 0;
    uint32  phyData;

    base_port = port - (port % PORT_NUM_IN_8212F);

    /* For Link-On and Cable Length Power Saving (per-port)
     * setting PHY0, page 15, register 27
     */
    if ((ret = hal_miim_read(unit, base_port, 15, 27, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(0x82 << (8*(port % PORT_NUM_IN_8212F)));
    if (ENABLED == enable)
    {
        phyData |= (0x82 << (8*(port % PORT_NUM_IN_8212F)));
    }
    if ((ret = hal_miim_write(unit, base_port, 15, 27, phyData)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8212f_greenEnable_set */

/* Function Name:
 *      phy_8212f_crossOverMode_get
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
phy_8212f_crossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
    int32   ret;
    uint32  phyData, auto_mode, force_mode, mdi;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, 0, 0x10, &phyData)) != RT_ERR_OK)
        return ret;
    auto_mode = (phyData >> 8) & 0x1;
    force_mode = (phyData >> 7) & 0x1;
    mdi = (phyData >> 6) & 0x1;

    if (force_mode)
    {
        if (mdi)
            *pMode = PORT_CROSSOVER_MODE_MDI;
        else
            *pMode = PORT_CROSSOVER_MODE_MDIX;
    }
    else
        *pMode = PORT_CROSSOVER_MODE_AUTO;

    return ret;
} /* end of phy_8212f_crossOverMode_get */

/* Function Name:
 *      phy_8212f_crossOverMode_set
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
phy_8212f_crossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
    int32   ret;
    uint32  phyData, auto_mode, force_mode, mdi;

    switch (mode)
    {
        case PORT_CROSSOVER_MODE_AUTO:
            auto_mode = 1;
            force_mode = 0;
            mdi = 1;
            break;
        case PORT_CROSSOVER_MODE_MDI:
            auto_mode = 0;
            force_mode = 1;
            mdi = 1;
            break;
        case PORT_CROSSOVER_MODE_MDIX:
            auto_mode = 0;
            force_mode = 1;
            mdi = 0;
            break;
        default:
            return RT_ERR_INPUT;
    }

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, 0, 0x10, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(7 << 6);
    phyData |= (auto_mode << 8);
    phyData |= (force_mode << 7);
    phyData |= (mdi << 6);
    if ((ret = hal_miim_write(unit, port, 0, 0x10, phyData)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8212f_crossOverMode_set */

/* Function Name:
 *      phy_8212f_linkDownPowerSavingEnable_get
 * Description:
 *      Get the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of linkdown link-down power saving
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. The RTL8212F is supported the per-port link-down power saving
 */
int32
phy_8212f_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData;

    /* setting page 15, register 27 */
    if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
        return ret;
    if (((phyData >> 12) & 0x1) == 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
} /* end of phy_8212f_linkDownPowerSavingEnable_get */

/* Function Name:
 *      phy_8212f_linkDownPowerSavingEnable_set
 * Description:
 *      Set the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of linkdown link-down power saving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. The RTL8212F is supported the per-port link-down power saving
 */
int32
phy_8212f_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData;

    /* For Link-Down Power Saving (per-port)
     * setting page 0, register 21, bit12 (1:enabled, 0:disabled)
     */
    if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(0x1 << 12);
    if (ENABLED == enable)
    {
        phyData |= (0x1 << 12);
    }
    if ((ret = hal_miim_write(unit, port, 0, 21, phyData)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8212f_linkDownPowerSavingEnable_set */

/* Function Name:
 *      phy_8212f_gigaLiteEnable_get
 * Description:
 *      Get the status of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8212F is not supported the per-port Giga Lite feature.
 */
int32
phy_8212f_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    *pEnable = DISABLED;
    return RT_ERR_OK;
} /* end of phy_8212f_gigaLiteEnable_get */
