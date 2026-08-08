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
 * Purpose : PHY 8214F Driver APIs.
 *
 * Feature : PHY 8214F Driver APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <osal/time.h>
#include <hal/common/miim.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_common.h>
#include <hal/phy/phy_8214f.h>
#include <rtk/default.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
rt_phydrv_t phy_8214Fdrv_ge =
{
    RT_PHYDRV_RTL8214F,
    phy_8214f_init,
    phy_8214f_media_get,
    phy_8214f_media_set,
    phy_common_autoNegoEnable_get,
    phy_common_autoNegoEnable_set,
    phy_8214f_autoNegoAbility_get,
    phy_8214f_autoNegoAbility_set,
    phy_common_duplex_get,
    phy_common_duplex_set,
    phy_8214f_speed_get,
    phy_8214f_speed_set,
    phy_8214f_enable_set,
    phy_8214f_rtctResult_get,
    phy_8214f_rtct_start,
    phy_8214f_greenEnable_get,
    phy_8214f_greenEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8214f_crossOverMode_get,
    phy_8214f_crossOverMode_set,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,       
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t))phy_common_unavail,
    phy_8214f_linkDownPowerSavingEnable_get,
    phy_8214f_linkDownPowerSavingEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8214f_gigaLiteEnable_get,
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
    (int32 (*)(uint32, rtk_port_t, rtk_port_masterSlave_t *, rtk_port_masterSlave_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_masterSlave_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
}; /* end of phy_8214Fdrv_ge */

rt_phydrv_t phy_8214drv_ge =
{
    RT_PHYDRV_RTL8214,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8214_media_get,
    (int32 (*)(uint32, rtk_port_t, rtk_port_media_t))phy_common_unavail,
    phy_common_autoNegoEnable_get,
    phy_common_autoNegoEnable_set,
    phy_8214f_autoNegoAbility_get,
    phy_8214f_autoNegoAbility_set,
    phy_common_duplex_get,
    phy_common_duplex_set,
    phy_8214f_speed_get,
    phy_8214_speed_set,
    phy_8214f_enable_set,
    phy_8214f_rtctResult_get,
    phy_8214f_rtct_start,
    phy_8214f_greenEnable_get,
    phy_8214f_greenEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8214f_crossOverMode_get,
    phy_8214f_crossOverMode_set,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,       
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t))phy_common_unavail,
    phy_8214f_linkDownPowerSavingEnable_get,
    phy_8214f_linkDownPowerSavingEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8214f_gigaLiteEnable_get,
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
    (int32 (*)(uint32, rtk_port_t, rtk_port_masterSlave_t *, rtk_port_masterSlave_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_masterSlave_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
}; /* end of phy_8214drv_ge */

/*
 * Function Declaration
 */

/* Function Name:
 *      phy_8214f_media_get
 * Description:
 *      Get PHY 8214F media type.
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
phy_8214f_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    uint32  val;
    uint32  base_port = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    base_port = port - (port % PORT_NUM_IN_8214F);
    if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
        return ret;

    /* media stauts: bit[15:12]
     * bit[12] mean phy0; bit[13] mean phy1; bit[14] mean phy2; bit[15] mean phy3;
     * - 0b0 mean copper media
     * - 0b1 mean fiber media
     */
    if (0 == (val & (0x1000 << (port - base_port))))
        *pMedia = PORT_MEDIA_COPPER;
    else
        *pMedia = PORT_MEDIA_FIBER;

    return RT_ERR_OK;
} /* end of phy_8214f_media_get */

/* Function Name:
 *      phy_8214f_media_set
 * Description:
 *      Get PHY 8214F media type.
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
 *      3. PORT_MEDIA_COPPER_AUTO & PORT_MEDIA_FIBER_AUTO return RT_ERR_CHIP_NOT_SUPPORTED.
 */
int32
phy_8214f_media_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    uint32  val;
    uint32  base_port = 0;
    uint32  old_state = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  old_an_stat = 0;

    base_port = port - (port % PORT_NUM_IN_8214F);

    switch (media)
    {
        case PORT_MEDIA_COPPER:
            /* turn-off fiber */
            if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
                return ret;
            if (0x0000 == ((val >> (port - base_port)) & 0x1000))
            {
                /* no change media */
                break;
            }
            val |= (0x1000 << (port - base_port));
            if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, 9, 24, &val)) != RT_ERR_OK)
                return ret;
            if (0x0008 == (val & 0x000C))
            {
                old_state = 1;
                val |= 0x0004;
                if ((ret = hal_miim_write(unit, port, 9, 24, val)) != RT_ERR_OK)
                    return ret;

                /* E0005376 */
                /* keep original status */
                if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &old_an_stat)) != RT_ERR_OK)
                {
                    return ret;
                }

                /* force AN */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, (old_an_stat|0x1200))) != RT_ERR_OK)
                {
                    return ret;
                }
                /* End of E0005376 */

                val &= 0xFFF7;
                if ((ret = hal_miim_write(unit, port, 9, 24, val)) != RT_ERR_OK)
                    return ret;

                /* E0005376 */
                /* write original state */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, old_an_stat)) != RT_ERR_OK)
                {
                    return ret;
                }
                /* End of E0005376 */

            }

            /* turn-on copper */
            if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(0x1000 << (port - base_port));
            if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            if (1 == old_state)
            {
                if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
                    return ret;
                val &= 0xF7FF;
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, val)) != RT_ERR_OK)
                    return ret;
            }
            break; /* end of switch-case PORT_MEDIA_COPPER */

        case PORT_MEDIA_FIBER:
            /* turn-off copper */
            if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
                return ret;
            if (0x1000 == ((val >> (port - base_port)) & 0x1000))
            {
                /* no change media */
                 break;
            }
            val &= ~(0x1000 << (port - base_port));
            if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
                return ret;
            if (0x0000 == (val & 0x0800))
            {
                old_state = 1;

                val |= 0x0800;
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, val)) != RT_ERR_OK)
                    return ret;
            }

            /* turn-on fiber */
            if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
                return ret;
            val |= (0x1000 << (port - base_port));
            if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            if (1 == old_state)
            {
                if ((ret = hal_miim_read(unit, port, 9, 24, &val)) != RT_ERR_OK)
                    return ret;
                val &= 0xFFF3;
                val |= 0x0008;
                if ((ret = hal_miim_write(unit, port, 9, 24, val)) != RT_ERR_OK)
                    return ret;
            }
            break; /* end of switch-case PORT_MEDIA_FIBER */

        case PORT_MEDIA_COPPER_AUTO:
        case PORT_MEDIA_FIBER_AUTO:
            return RT_ERR_CHIP_NOT_SUPPORTED;

        default:
            break;
    }

    return RT_ERR_OK;
} /* end of phy_8214f_media_set */

/* Function Name:
 *      phy_8214f_init
 * Description:
 *      Initialize PHY 8214F.
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
phy_8214f_init(uint32 unit, rtk_port_t port)
{
    uint32  val;
    uint32  base_port = 0, restore_val;
    uint32  def_media = PORT_MEDIA_COPPER;
    uint32  def_port_state = RTK_DEFAULT_PORT_ADMIN_ENABLE; /* enable(1), disable(0) */
    int32   ret = RT_ERR_FAILED;

    base_port = port - (port % PORT_NUM_IN_8214F);

    /* Patch some 8328 MAC auto config phy register back to chip default
     * PHY X, page 0, reg 4, value 0x01A0 (for fiber-1000)
     * PHY X, page 0, reg 9, value 0x0000 (for fiber-1000)
     */
    if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &restore_val)) != RT_ERR_OK)
        return ret;
    val = (restore_val | (0x1100 << (port - base_port)));
    if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 4, 0x01A0)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 9, 0x0000)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, restore_val)) != RT_ERR_OK)
        return ret;

    /* Ensure the patch was the same with what bootloader do with combo ports*/
    /* VCO high-gain */
    if ((ret = hal_miim_read(unit, base_port + 2, PHY_PAGE_8, 22, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port + 2, PHY_PAGE_8, 22, (val | (0xff << 8)))) != RT_ERR_OK)
        return ret;
    /* CDR */
    if ((ret = hal_miim_read(unit, base_port, 9, 19, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 9, 19, ((val & (~(0x9 << 4))) | (0x6 << 4)))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, base_port + 1, 9, 19, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port + 1, 9, 19, ((val & (~(0x9 << 4))) | (0x6 << 4)))) != RT_ERR_OK)
        return ret;
    /* force-run */
    if ((ret = hal_miim_read(unit, base_port, 9, 16, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 9, 16, (val | (0x1 << 8)))) != RT_ERR_OK)
        return ret;
    osal_time_udelay(100 * 1000);
    if ((ret = hal_miim_write(unit, base_port, 9, 16, (val & ~(0x1 << 8)))) != RT_ERR_OK)
        return ret;
    /* Fixed the Register Set */
    if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, (val | 0x0f00))) != RT_ERR_OK)
        return ret;

    /* Echo group */
    if ((ret = hal_miim_read(unit, port, 1, 14, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 1, 14, (val | (0x1 << 3)))) != RT_ERR_OK)
        return ret;

    /* Green feature length threshold low (Viterbi) */
    if ((ret = hal_miim_read(unit, port, 3, 23, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 3, 23, (val & ~(0x1f << 6)))) != RT_ERR_OK)
        return ret;
    /* Enable En_pwrsave */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 21, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 21, (val | (0x1 << 12)))) != RT_ERR_OK)
        return ret;
    /* Enable En_stop_gftxc & En_stop_gfrxc */
    if ((ret = hal_miim_read(unit, port, 2, 24, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 2, 24, ((val & (~(0xF << 4))) | (0xA << 4)))) != RT_ERR_OK)
        return ret;
    /* RTL8214 short cable power-saving */
    if ((ret = hal_miim_read(unit, base_port, PHY_PAGE_8, 19, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, PHY_PAGE_8, 19, (val | (0x1 << 6)))) != RT_ERR_OK)
        return ret;

    /* Disable RTL8214F two pair speed down function */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 20, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 20, (val & ~(0x1 << 15)))) != RT_ERR_OK)
        return ret;

    /* Fix the Power Status */
    if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, (val | (0x1000 << (port - base_port))))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, port, 9, 19, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 9, 19, (val & ~(0x1 << 4)))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, port, 9, 16, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 9, 16, (val | (0x1 << 5)))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, port, 9, 24, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 9, 24, (val | (0x1 << 5)))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 9, 24, (val | (0x1 << 2)))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 9, 24, (val & ~(0x1 << 3)))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, (val & ~(0x1000 << (port - base_port))))) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, (val | (0x1 << 11)))) != RT_ERR_OK)
        return ret;

    /* Poweroff the RSGMII(2,3) serdes */
    if ((port - base_port) >= 2 && (port - base_port) < PORT_NUM_IN_8214F)
    {
        if ((ret = hal_miim_read(unit, port, 9, 16, &val)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 9, 16, (val | ((0x1 << 5) | (0x1 << 2))))) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 9, 16, (val & (~((0x1 << 4)) | (0x1 << 3))))) != RT_ERR_OK)
            return ret;
    }

    /* Set the default media */
    if (def_media != PORT_MEDIA_COPPER)
    {
        if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
            return ret;
        val |= (0x1000 << (port - base_port));
        if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
            return ret;

        /* Set the default port state */
        if (def_port_state)
        {
            if ((ret = hal_miim_read(unit, port, 9, 24, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(0x1 << 2);
            val |= (0x1 << 3);
            if ((ret = hal_miim_write(unit, port, 9, 24, val)) != RT_ERR_OK)
                return ret;
        }
    }
    else
    {
        if ((ret = hal_miim_read(unit, base_port + 1, PHY_PAGE_8, 16, &val)) != RT_ERR_OK)
            return ret;
        val &= ~(0x1000 << (port - base_port));
        if ((ret = hal_miim_write(unit, base_port + 1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
            return ret;

        /* Set the default port state */
        if (def_port_state)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(0x1 << 11);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, val)) != RT_ERR_OK)
                return ret;
        }
    }

    /* Fixed power force mode */
    if ((ret = hal_miim_read(unit, base_port, 9, 16, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 9, 16, (val | (0x1 << 0)))) != RT_ERR_OK)
        return ret;
    /* Fixed state-machine */
    if ((ret = hal_miim_read(unit, base_port, PHY_PAGE_8, 21, &val)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, PHY_PAGE_8, 21, (val | (0x1 << 1)))) != RT_ERR_OK)
        return ret;

    return RT_ERR_OK;
} /* end of phy_8214f_init */


/* Function Name:
 *      phy_8214f_autoNegoAbility_get
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
phy_8214f_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData4;
    uint32  phyData9;
    rtk_enable_t     enable;
    rtk_port_media_t media;

    phy_8214f_media_get(unit, port, &media);

    phy_common_autoNegoEnable_get(unit, port, &enable);

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

    return ret;
} /* end of phy_8214f_autoNegoAbility_get */

/* Function Name:
 *      phy_8214f_autoNegoAbility_set
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
phy_8214f_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData0;
    uint32  phyData4;
    uint32  phyData9;
    rtk_enable_t     enable;
    rtk_port_media_t media;

    phy_8214f_media_get(unit, port, &media);

    phy_common_autoNegoEnable_get(unit, port, &enable);

    /* get value from CHIP*/
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

        phyData9 = phyData9 & ~(_1000Base_THalfDuplex_MASK | _1000Base_TFullDuplex_MASK);
        phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                   | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
            return ret;


        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
            return ret;
    }

    /* Force re-autonegotiation if AN is on*/
    if (ENABLED == enable)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
    }

    return ret;
} /* end of phy_8214f_autoNegoAbility_set */


/* Function Name:
 *      phy_8214f_speed_get
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
phy_8214f_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    int32   ret;
    uint32  phyData0;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
              | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);

    return ret;
} /* end of phy_8214f_speed_get */


/* Function Name:
 *      phy_8214_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
int32
phy_8214_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32   ret;
    uint32  phyData0;

    /* RT_ERR_CHIP_NOT_SUPPORTED */
    RT_PARAM_CHK(speed == PORT_SPEED_1000M, RT_ERR_CHIP_NOT_SUPPORTED);

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214_speed_set */

/* Function Name:
 *      phy_8214f_speed_set
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
phy_8214f_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32   ret;
    uint32  phyData0;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214f_speed_set */

/* Function Name:
 *      phy_8214f_enable_set
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
phy_8214f_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData;
    uint32  old_phyData;
    rtk_port_media_t  media;

    if ((ret = phy_8214f_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    if (ENABLED == enable)
    {
        if (media == PORT_MEDIA_COPPER)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
                return ret;

            phyData &= ~(PowerDown_MASK);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            if ((ret = hal_miim_read(unit, port, 9, 24, &phyData)) != RT_ERR_OK)
                return ret;

            phyData &= 0xFFF3;
            phyData |= 0x0008;

            if ((ret = hal_miim_write(unit, port, 9, 24, phyData)) != RT_ERR_OK)
                return ret;
        }
    }
    else
    {
        if (media == PORT_MEDIA_COPPER)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
                return ret;

            phyData &= ~(PowerDown_MASK);
            phyData |= (1 << PowerDown_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            if ((ret = hal_miim_read(unit, port, 9, 24, &phyData)) != RT_ERR_OK)
                return ret;

            phyData |= 0x0004;
            if ((ret = hal_miim_write(unit, port, 9, 24, phyData)) != RT_ERR_OK)
                return ret;

            /* E0005376 */
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &old_phyData)) != RT_ERR_OK)
                return ret;

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, old_phyData | 0x1200)) != RT_ERR_OK)
                return ret;

            /* End of E0005376 */

            phyData &= 0xFFF7;
            if ((ret = hal_miim_write(unit, port, 9, 24, phyData)) != RT_ERR_OK)
                return ret;

            /* E0005376 */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, old_phyData)) != RT_ERR_OK)
                return ret;
            /* End of E0005376 */
        }
    }
    return ret;
}

/* Function Name:
 *      phy_8214f_rtctResult_get
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
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_8214f_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData;
    rtk_port_media_t  media;

    if ((ret = phy_8214f_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    if (media == PORT_MEDIA_COPPER)
    {
        /* Page 10, PHY 0-3, Register 12 (Per-port RTCT Configuration Register)
         * bit[15:15]: Enable port 0-3 cable test. When enable port x RTCT,
         *             RTL8214 will scan port x cable status from channel A
         *             to channel D sequentially.
         *             1: Enable
         *             0: Disable
         */
        if ((ret = hal_miim_read(unit, port, 11, 27, &phyData)) != RT_ERR_OK)
            return ret;

        if (0 == (phyData & 0x4000))
            return RT_ERR_PHY_RTCT_NOT_FINISH;

        pRtctResult->linkType = PORT_SPEED_1000M;
        /* Length = (Index/64)*8ns*(0.2m/ns) = Index/40 (m) = (2.5) * Index (cm) */
        pRtctResult->ge_result.channelALen = (phyData & 0x1FFF)*5/2;

        if ((ret = hal_miim_read(unit, port, 11, 26, &phyData)) != RT_ERR_OK)
            return ret;
        pRtctResult->ge_result.channelAShort = phyData & 0x1000;
        pRtctResult->ge_result.channelBShort = phyData & 0x2000;
        pRtctResult->ge_result.channelCShort = phyData & 0x4000;
        pRtctResult->ge_result.channelDShort = phyData & 0x8000;
        pRtctResult->ge_result.channelAOpen  = phyData & 0x0100;
        pRtctResult->ge_result.channelBOpen  = phyData & 0x0200;
        pRtctResult->ge_result.channelCOpen  = phyData & 0x0400;
        pRtctResult->ge_result.channelDOpen  = phyData & 0x0800;

        if ((ret = hal_miim_read(unit, port, 11, 28, &phyData)) != RT_ERR_OK)
            return ret;
        pRtctResult->ge_result.channelBLen = (phyData & 0x1FFF)*5/2;

        if ((ret = hal_miim_read(unit, port, 11, 29, &phyData)) != RT_ERR_OK)
            return ret;
        pRtctResult->ge_result.channelCLen = (phyData & 0x1FFF)*5/2;

        if ((ret = hal_miim_read(unit, port, 11, 30, &phyData)) != RT_ERR_OK)
            return ret;
        pRtctResult->ge_result.channelDLen = (phyData & 0x1FFF)*5/2;
    }
    else
    {
        /* RTCT function is not supoprted in fiber media */
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return ret;
} /* end of phy_8214f_rtctResult_get */

/* Function Name:
 *      phy_8214f_rtct_start
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
phy_8214f_rtct_start(uint32 unit, rtk_port_t port)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData, phyData21, i;
    rtk_port_media_t  media;

    if ((ret = phy_8214f_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    if (media == PORT_MEDIA_COPPER)
    {
        /* Disable Power Saving Mode First */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 21, &phyData21)) != RT_ERR_OK)
            return ret;
        phyData = phyData21 & ~(0x1000);
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 21, phyData)) != RT_ERR_OK)
            return ret;
        osal_time_udelay(100 * 1000);
        /* Page 10, PHY 0-3, Register 12 (Per-port RTCT Configuration Register)
         * bit[15:15]: Enable port 0-3 cable test. When enable port x RTCT,
         *             RTL8214 will scan port x cable status from channel A
         *             to channel D sequentially.
         *             1: Enable
         *             0: Disable
         */
        if ((ret = hal_miim_read(unit, port, 10, 22, &phyData)) != RT_ERR_OK)
            return ret;

        phyData &= ~(0x8000);
        if ((ret = hal_miim_write(unit, port, 10, 22, phyData)) != RT_ERR_OK)
            return ret;

        osal_time_udelay(100 * 1000);
        phyData |= (0x8000);
        if ((ret = hal_miim_write(unit, port, 10, 22, phyData)) != RT_ERR_OK)
            return ret;

        for (i = 0; i < RTCT_CHECKBUSY_TIMES; i++)
        {
            osal_time_udelay(500 * 1000);
            if ((ret = hal_miim_read(unit, port, 11, 27, &phyData)) != RT_ERR_OK)
                return ret;
            if (phyData & 0x4000)
                break;
        }

        /* Restore Power Saving Mode */
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 21, phyData21)) != RT_ERR_OK)
            return ret;

        if (RTCT_CHECKBUSY_TIMES == i)
            return RT_ERR_PHY_RTCT_TIMEOUT; /* RT_ERR_TIMEOUT */
    }
    else
    {
        /* RTCT function is not supoprted in fiber media */
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return ret;
} /* end of phy_8214f_rtct_start */

/* Function Name:
 *      phy_8214f_greenEnable_get
 * Description:
 *      Get the status of linkup green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to status of linkup green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      1. The RTL8214F is only supported the per-chip linkup green feature.
 *      2. The setting will be apply to 4 ports in the chip.
 */
int32
phy_8214f_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  phyData16, phyData19;
    uint32  base_port = 0;
    int32   ret = RT_ERR_FAILED;

    base_port = port - (port % PORT_NUM_IN_8214F);

    /* setting page 8, register 16, bit[8] for Tx enable/disable */
    if ((ret = hal_miim_read(unit, base_port, PHY_PAGE_8, 16, &phyData16)) != RT_ERR_OK)
        return ret;
    /* setting page 8, register 19, bit[6] for Rx enable/disable */
    if ((ret = hal_miim_read(unit, base_port, PHY_PAGE_8, 19, &phyData19)) != RT_ERR_OK)
        return ret;
    if (((phyData16 >> 8) & 0x1) && ((phyData19 >> 6) & 0x1))
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
} /* end of phy_8214f_greenEnable_get */

/* Function Name:
 *      phy_8214f_greenEnable_set
 * Description:
 *      Set the status of linkup green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of linkup green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      1. The RTL8214F is only supported the per-chip linkup green feature.
 *      2. The setting will be apply to 4 ports in the chip.
 */
int32
phy_8214f_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData;
    uint32  base_port = 0;
    int32   ret = RT_ERR_FAILED;

    base_port = port - (port % PORT_NUM_IN_8214F);

    /* For Link-On and Cable Length Power Saving (per-chip) */
    /* setting page 8, register 16, bit[8] for Tx enable/disable */
    if ((ret = hal_miim_read(unit, base_port, PHY_PAGE_8, 16, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= 0xFEFF;
    if (ENABLED == enable)
    {
        phyData |= 0x0100;
    }
    if ((ret = hal_miim_write(unit, base_port, PHY_PAGE_8, 16, phyData)) != RT_ERR_OK)
        return ret;

    /* setting page 8, register 19, bit[6] for Rx enable/disable */
    if ((ret = hal_miim_read(unit, base_port, PHY_PAGE_8, 19, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= 0xFFBF;
    if (ENABLED == enable)
    {
        phyData |= 0x0040;
    }
    if ((ret = hal_miim_write(unit, base_port, PHY_PAGE_8, 19, phyData)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214f_greenEnable_set */

/* Function Name:
 *      phy_8214f_crossOverMode_get
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
phy_8214f_crossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
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
} /* end of phy_8214f_crossOverMode_get */

/* Function Name:
 *      phy_8214f_crossOverMode_set
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
phy_8214f_crossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
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
} /* end of phy_8214f_crossOverMode_set */

/* Function Name:
 *      phy_8214f_linkDownPowerSavingEnable_get
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
 *      1. The RTL8214F is only supported the per-chip link-down power saving
 *      2. The setting will be apply to 4 ports in the chip.
 */
int32
phy_8214f_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  phyData;
    int32   ret = RT_ERR_FAILED;

    if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
        return ret;
    if (((phyData >> 12) & 0x1) == 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
} /* end of phy_8214f_linkDownPowerSavingEnable_get */

/* Function Name:
 *      phy_8214f_linkDownPowerSavingEnable_set
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
 *      1. The RTL8214F is only supported the per-chip link-down power saving
 *      2. The setting will be apply to 4 ports in the chip.
 */
int32
phy_8214f_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret = RT_ERR_FAILED;

    /* For Link-Down Power Saving (per-port) */
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
} /* end of phy_8214f_linkDownPowerSavingEnable_set */

/* Function Name:
 *      phy_8214_media_get
 * Description:
 *      Get 8214 serdes PHY media type.
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
 *      1. media type is PORT_MEDIA_COPPER
 */
int32
phy_8214_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    *pMedia = PORT_MEDIA_COPPER;
    return RT_ERR_OK;
} /* end of phy_8214_media_get */

/* Function Name:
 *      phy_8214f_gigaLiteEnable_get
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
 *      1. The RTL8214F is not supported the per-port Giga Lite feature.
 */
int32
phy_8214f_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    *pEnable = DISABLED;
    return RT_ERR_OK;
} /* end of phy_8214f_gigaLiteEnable_get */
