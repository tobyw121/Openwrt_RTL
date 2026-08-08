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
 * $Revision: 56168 $
 * $Date: 2015-02-13 16:48:38 +0800 (Fri, 13 Feb 2015) $
 *
 * Purpose : PHY identify service APIs in the SDK.
 *
 * Feature : PHY identify service APIs
 *
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <common/rt_autoconf.h>
#include <drv/gpio/gpio.h>
#include <osal/lib.h>
#include <hal/common/halctrl.h>
#include <hal/common/miim.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/phy/identify.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_common.h>
#if defined(CONFIG_SDK_RTL8201)
#include <hal/phy/phy_8201.h>
#endif
#if defined(CONFIG_SDK_RTL8208)
#include <hal/phy/phy_8208.h>
#endif
#if (defined(CONFIG_SDK_RTL8214) || defined(CONFIG_SDK_RTL8214F))
#include <hal/phy/phy_8214f.h>
#endif
#if (defined(CONFIG_SDK_RTL8218) || defined(CONFIG_SDK_RTL8218B))
#include <hal/phy/phy_8218.h>
#endif
#if defined(CONFIG_SDK_RTL8212F)
#include <hal/phy/phy_8212f.h>
#endif
#if defined(CONFIG_SDK_RTL8389)
#include <hal/phy/phy_8389.h>
#endif
#if (defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8214FB) || defined(CONFIG_SDK_RTL8212B))
#include <hal/phy/phy_8214fb.h>
#endif
#if defined(CONFIG_SDK_RTL8328)
#include <hal/phy/phy_8328.h>
#endif
#if (defined(CONFIG_SDK_RTL8218B) || defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC))
#include <hal/phy/phy_8218b.h>
#endif
#if defined(CONFIG_SDK_RTL8380)
#include <hal/chipdef/maple/rtk_maple_reg_struct.h>
#include <hal/phy/phy_8380.h>
#endif
#if defined(CONFIG_SDK_RTL8390)
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#include <hal/phy/phy_8390.h>
#endif
#if defined(CONFIG_SDK_RTL8218C)
#include <hal/phy/phy_8218c.h>
#endif

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
#if defined(CONFIG_SDK_RTL8208) || defined(CONFIG_SDK_RTL8218) || defined(CONFIG_SDK_RTL8212F) || defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8380)
static int32 _phy_identify_default(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if (defined(CONFIG_SDK_RTL8214F) || defined(CONFIG_SDK_RTL8214))
static int32 _phy_identify_8214F(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
static int32 _phy_identify_8214(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if defined(CONFIG_SDK_RTL8201)
static int32 _phy_identify_8201(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if defined(CONFIG_SDK_RTL8389)
static int32 _phy_identify_8389_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if (defined(CONFIG_SDK_RTL8214FB) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8212B))
static int32 _phy_identify_8214FB(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
static int32 _phy_identify_8214B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
static int32 _phy_identify_8212B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if defined(CONFIG_SDK_RTL8328)
static int32 _phy_identify_8328_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if defined(CONFIG_SDK_RTL8218B)
static int32 _phy_identify_8218B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if defined(CONFIG_SDK_RTL8218C)
static int32 _phy_identify_8218C(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if (defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC))
static int32 _phy_identify_8218FB(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
static int32 _phy_identify_8214FC(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
static int32 _phy_identify_8218FB_MP(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
static int32 _phy_identify_8214FC_MP(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if defined(CONFIG_SDK_RTL8380)
static int32 _phy_identify_8380_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif
#if defined(CONFIG_SDK_RTL8390)
static int32 _phy_identify_8390_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
static int32 _phy_identify_8390_serdes_10ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id);
#endif

/* supported external PHY chip lists */
static rt_phyctrl_t supported_phys[] =
{
#if defined(CONFIG_SDK_RTL8208)
    {_phy_identify_default, PHY_MODEL_ID_RTL8208G, PHY_REV_NO_A, &phy_8208Gdrv_fe, PHY_AFLAG_NULL, NULL},
    {_phy_identify_default, PHY_MODEL_ID_RTL8208G, PHY_REV_NO_B, &phy_8208Gdrv_fe, PHY_AFLAG_NULL, NULL},
#endif
#if (defined(CONFIG_SDK_RTL8214F) || defined(CONFIG_SDK_RTL8214))
    {_phy_identify_8214F, PHY_MODEL_ID_RTL8214F, PHY_REV_NO_A, &phy_8214Fdrv_ge, PHY_AFLAG_COMBO, NULL},
    {_phy_identify_8214, PHY_MODEL_ID_RTL8214,  PHY_REV_NO_A, &phy_8214drv_ge , PHY_AFLAG_NULL, NULL},
#endif
#if defined(CONFIG_SDK_RTL8201)
    {_phy_identify_8201, PHY_MODEL_ID_RTL8201,  PHY_REV_NO_C, &phy_8201drv_fe , PHY_AFLAG_NULL, NULL},
#endif
#if defined(CONFIG_SDK_RTL8218)
    {_phy_identify_default, PHY_MODEL_ID_RTL8218_TC,  PHY_REV_NO_A, &phy_8218drv_ge , PHY_AFLAG_NULL, NULL},
    {_phy_identify_default, PHY_MODEL_ID_RTL8218,  PHY_REV_NO_A, &phy_8218drv_ge , PHY_AFLAG_NULL, NULL},
#endif
#if defined(CONFIG_SDK_RTL8212F)
    {_phy_identify_default, PHY_MODEL_ID_RTL8212F, PHY_REV_NO_A, &phy_8212Fdrv_ge, PHY_AFLAG_COMBO, NULL},
#endif
#if (defined(CONFIG_SDK_RTL8214FB) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8212B))
    {_phy_identify_8214FB, PHY_MODEL_ID_RTL8214F, PHY_REV_NO_B, &phy_8214FBdrv_ge, PHY_AFLAG_COMBO, NULL},
    {_phy_identify_8214B, PHY_MODEL_ID_RTL8214,  PHY_REV_NO_B, &phy_8214Bdrv_ge , PHY_AFLAG_COMBO, NULL},
    {_phy_identify_8212B, PHY_MODEL_ID_RTL8214,  PHY_REV_NO_B, &phy_8212Bdrv_ge , PHY_AFLAG_COMBO, NULL},
#endif
#if defined(CONFIG_SDK_RTL8208)
    {_phy_identify_default, PHY_MODEL_ID_RTL8208D_EXT, PHY_REV_NO_C, &phy_8208Ddrv_fe, PHY_AFLAG_NULL, NULL},
#endif
#if defined(CONFIG_SDK_RTL8218B)
    {_phy_identify_8218B, PHY_MODEL_ID_RTL8218B_EXT,  PHY_REV_NO_A, &phy_8218drv_ge , PHY_AFLAG_NULL, &phy_8218B_info},
    {_phy_identify_8218B, PHY_MODEL_ID_RTL8218B_EXT,  PHY_REV_NO_B, &phy_8218Bdrv_ge , PHY_AFLAG_NULL, &phy_8218B_info},
#endif
#if (defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC))
    {_phy_identify_8218FB, PHY_MODEL_ID_RTL8218FB,  PHY_REV_NO_B, &phy_8218FBdrv_ge , PHY_AFLAG_COMBO, &phy_8218FB_info},
    {_phy_identify_8214FC, PHY_MODEL_ID_RTL8218FB,  PHY_REV_NO_B, &phy_8214FCdrv_ge , PHY_AFLAG_COMBO, &phy_8214FC_info},
    {_phy_identify_8218FB_MP, PHY_MODEL_ID_RTL8218FB,  PHY_REV_NO_A, &phy_8218FBdrv_MP_ge , PHY_AFLAG_COMBO, &phy_8218FB_info},
    {_phy_identify_8214FC_MP, PHY_MODEL_ID_RTL8218FB,  PHY_REV_NO_A, &phy_8214FCdrv_MP_ge , PHY_AFLAG_COMBO, &phy_8214FC_info},
#endif
#if defined(CONFIG_SDK_RTL8218C)
    {_phy_identify_8218C, PHY_MODEL_ID_RTL8218C_EXT,  PHY_REV_NO_C, &phy_8218Cdrv_ge , PHY_AFLAG_NULL, NULL},
#endif


}; /* end of supported_phys */

/* supported internal PHY chip lists */
static rt_phyctrl_t supported_int_phys[] =
{
#if defined(CONFIG_SDK_RTL8389)
    {_phy_identify_8389_serdes_ge, (uint32)NULL, (uint32)NULL, &phy_8389_serdes_ge, PHY_AFLAG_FIBER, NULL},
#endif
#if defined(CONFIG_SDK_RTL8328)
    {_phy_identify_8328_serdes_ge, (uint32)NULL, (uint32)NULL, &phy_8328_serdes_ge, PHY_AFLAG_FIBER, NULL},
#endif
#if defined(CONFIG_SDK_RTL8328)
    {_phy_identify_default, PHY_MODEL_ID_RTL8208D_INT, PHY_REV_NO_A, &phy_8328drv_int_fe, PHY_AFLAG_NULL, NULL},
#endif
#if defined(CONFIG_SDK_RTL8380)
#if defined(CONFIG_SDK_RTL8218B)
    {_phy_identify_default, PHY_MODEL_ID_RTL8218B_INT,  PHY_REV_NO_A, &phy_8380drv_int_ge , PHY_AFLAG_NULL, NULL},
#endif
    {_phy_identify_8380_serdes_ge, (uint32)NULL, (uint32)NULL, &phy_8380_serdes_ge, PHY_AFLAG_FIBER, NULL},
#endif
#if defined(CONFIG_SDK_RTL8390)
    {_phy_identify_8390_serdes_ge, (uint32)NULL, (uint32)NULL, &phy_8390_serdes_ge, PHY_AFLAG_FIBER, NULL},
    {_phy_identify_8390_serdes_10ge, (uint32)NULL, (uint32)NULL, &phy_8390_serdes_ge, PHY_AFLAG_10G_SERDES, NULL},
#endif
};

/*
 * Function Declaration
 */

/* Static Function Body */

/* Function Name:
 *      _phy_identify_default
 * Description:
 *      Identify the port is match input PHY information or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is match the PHY information
 *      RT_ERR_FAILED - is not match the PHY information
 * Note:
 *      None
 */
#if defined(CONFIG_SDK_RTL8208) || defined(CONFIG_SDK_RTL8218) || defined(CONFIG_SDK_RTL8212F) || defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8380)
static int32
_phy_identify_default(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        return RT_ERR_FAILED;
    }

    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
        return RT_ERR_OK;

    return RT_ERR_FAILED;
} /* end of _phy_identify_default */
#endif

#if defined(CONFIG_SDK_RTL8201)
/* Function Name:
 *      _phy_identify_8201
 * Description:
 *      Identify the port is 8201 PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8201 PHY
 *      RT_ERR_FAILED - is not 8201 PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8201(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
        return RT_ERR_OK;

    return RT_ERR_FAILED;
} /* end of _phy_identify_8201 */
#endif

#if (defined(CONFIG_SDK_RTL8214F) || defined(CONFIG_SDK_RTL8214))
/* Function Name:
 *      _phy_identify_8214
 * Description:
 *      Identify the port is 8214 PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8214 PHY
 *      RT_ERR_FAILED - is not 8214 PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8214(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        return RT_ERR_FAILED;
    }

    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8214F);
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, PHY_PAGE_8, 17, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0xF == ((data >> 10) & 0xF))
            return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8214 */

/* Function Name:
 *      _phy_identify_8214F
 * Description:
 *      Identify the port is 8214F PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8214F PHY
 *      RT_ERR_FAILED - is not 8214F PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8214F(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        return RT_ERR_FAILED;
    }

    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id == rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8214F);
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, PHY_PAGE_8, 17, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0xF != ((data >> 10) & 0xF))
            return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8214F */
#endif

#if defined(CONFIG_SDK_RTL8389)
/* Function Name:
 *      _phy_identify_8389_serdes_ge
 * Description:
 *      Identify the port is 8389 internal PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8389 internal  PHY
 *      RT_ERR_FAILED - is not 8389 internal  PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8389_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    hal_control_t   *pHalCtrl = NULL;
    uint32  value = 0;
    uint32  is_fiber = 0, intra_speed = 0, serdes_mode = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8389_serdes_ge(unit %d, port %d) function!!", unit, port);

    if (HAL_IS_GE_PORT(unit, port) && (port >= 24) &&
        (RTL8329M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8377M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8389M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8389L_CHIP_ID == pHalCtrl->chip_id))
    {
#if 0   /* OLD detect mechanism */
        gpioId = GPIO_ID(GPIO_PORT_A, 0);
        if (drv_gpio_dataBit_get(gpioId, &data) != RT_ERR_OK)
        {
            return RT_ERR_FAILED;
        }
        /* GPIOA[0]: 0 mean intra-link for 2 port fixed fiber
         *           1 mean intra-link for 2 port unused
         */

        if ((0 == data) && ((port % 4) < 2))
        {
            return RT_ERR_OK;
        }
        else
            return RT_ERR_FAILED;
#endif
        /* NEW detect mechanism */
        switch ((port % 4))
        {
            case 0:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8389_serdes_ge(unit %d, port %d) switch case 0 function!!", unit, port);
                if ((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL0r, SSW_SEL_INTRA_SERDES_MODEf, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, &value)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_get(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_FX0f, &is_fiber, &value)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_get(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_RS_SPD0f, &intra_speed, &value)) != RT_ERR_OK)
                    return ret;
                if (0 == serdes_mode && 1 == is_fiber && 1 == intra_speed)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 1:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8389_serdes_ge(unit %d, port %d) switch case 1 function!!", unit, port);
                if ((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL0r, SSW_SEL_INTRA_SERDES_MODEf, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, &value)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_get(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_FX1f, &is_fiber, &value)) != RT_ERR_OK)
                    return ret;
                if ((ret = reg_field_get(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_RS_SPD1f, &intra_speed, &value)) != RT_ERR_OK)
                    return ret;
                if (0 == serdes_mode && 1 == is_fiber && 1 == intra_speed)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 2:
            case 3:
            default:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8389_serdes_ge(unit %d, port %d) switch case 2/3 function!!", unit, port);
                return RT_ERR_FAILED;
        }
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8389_serdes_ge */
#endif

#if defined(CONFIG_SDK_RTL8328)
/* Function Name:
 *      _phy_identify_8328_serdes_ge
 * Description:
 *      Identify the port is 8328 intra serdes PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8328 intra serdes PHY
 *      RT_ERR_FAILED - is not 8328 intra serdes PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8328_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    hal_control_t   *pHalCtrl = NULL;
    uint32  serdes_mode = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8328_serdes_ge(unit %d, port %d) function!!", unit, port);

    if (HAL_IS_GE_PORT(unit, port) &&
        (RTL8328M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8328S_CHIP_ID == pHalCtrl->chip_id ||
         RTL8328L_CHIP_ID == pHalCtrl->chip_id))
    {
        switch ((port % 4))
        {
            case 0:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8328_serdes_ge(unit %d, port %d) switch case 0 function!!", unit, port);
                if ((ret = reg_field_read(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL0r, ESW_SEL_SDS0_MODE_1_0f, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if (3 == serdes_mode)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 2:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8328_serdes_ge(unit %d, port %d) switch case 1 function!!", unit, port);
                if ((ret = reg_field_read(unit, ESW_GLOBAL_MAC_INTERFACE_CONTROL0r, ESW_SEL_SDS1_MODE_1_0f, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if (3 == serdes_mode)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 1:
            case 3:
            default:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8328_serdes_ge(unit %d, port %d) switch case 2/3 function!!", unit, port);
                return RT_ERR_FAILED;
        }
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8328_serdes_ge */
#endif

#if defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      _phy_identify_8380_serdes_ge
 * Description:
 *      Identify the port is 8380 intra serdes PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8380 intra serdes PHY
 *      RT_ERR_FAILED - is not 8380 intra serdes PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8380_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    hal_control_t   *pHalCtrl = NULL;
    uint32  serdes_mode = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8380_serdes_ge(unit %d, port %d) function!!", unit, port);

#if 0
    if (HAL_IS_GE_PORT(unit, port) &&
        (RTL8328M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8328S_CHIP_ID == pHalCtrl->chip_id ||
         RTL8328L_CHIP_ID == pHalCtrl->chip_id))
#endif
    {
        /*For RTL8380 only port24 & port26 can be configured as fiber port*/
        switch (port)
        {
            case 24:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8380_serdes_ge(unit %d, port %d) switch case 0 function!!", unit, port);
                if ((ret = reg_field_read(unit, MAPLE_INT_MODE_CTRLr, MAPLE_SDS4_INTF_MODE_2_0f, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if (1 == serdes_mode)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 26:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8380_serdes_ge(unit %d, port %d) switch case 1 function!!", unit, port);
                if ((ret = reg_field_read(unit, MAPLE_INT_MODE_CTRLr, MAPLE_SDS5_INTF_MODE_2_0f, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if (1 == serdes_mode)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 1:
            case 3:
            default:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8380_serdes_ge(unit %d, port %d) switch case 2/3 function!!", unit, port);
                return RT_ERR_FAILED;
        }
    }

    return RT_ERR_FAILED;
} /* end  _phy_identify_8380_serdes_ge */
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      _phy_identify_8390_serdes_ge
 * Description:
 *      Identify the port is 8390 intra serdes PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8380 intra serdes PHY
 *      RT_ERR_FAILED - is not 8380 intra serdes PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8390_serdes_ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    hal_control_t   *pHalCtrl = NULL;
    uint32  serdes_mode = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8390_serdes_ge(unit %d, port %d) function!!", unit, port);

#if 0
    if (HAL_IS_GE_PORT(unit, port) &&
        (RTL8328M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8328S_CHIP_ID == pHalCtrl->chip_id ||
         RTL8328L_CHIP_ID == pHalCtrl->chip_id))
#endif
    {
        /*For RTL8390 only port48 & port49 can be configured as fiber port*/
        switch ((port%4))
        {
            case 0:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8390_serdes_ge(unit %d, port %d) switch case 0 function!!", unit, port);
                if ((ret = reg_array_field_read(unit, CYPRESS_MAC_SERDES_IF_CTRLr, REG_ARRAY_INDEX_NONE, 12, CYPRESS_SERDES_SPD_SELf, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if (7 == serdes_mode)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 1:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8390_serdes_ge(unit %d, port %d) switch case 1 function!!", unit, port);
                if ((ret = reg_array_field_read(unit, CYPRESS_MAC_SERDES_IF_CTRLr, REG_ARRAY_INDEX_NONE, 13, CYPRESS_SERDES_SPD_SELf, &serdes_mode)) != RT_ERR_OK)
                    return ret;
                if (7 == serdes_mode)
                    return RT_ERR_OK;
                else
                    return RT_ERR_FAILED;
                break;
            case 2:
            case 3:
            default:
                RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8390_serdes_ge(unit %d, port %d) switch case 2/3 function!!", unit, port);
                return RT_ERR_FAILED;
        }
    }

    return RT_ERR_FAILED;
} /* end  _phy_identify_8390_serdes_ge */

/* Function Name:
 *      _phy_identify_8390_serdes_10ge
 * Description:
 *      Identify the port is 8390 intra serdes PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8380 intra serdes PHY
 *      RT_ERR_FAILED - is not 8380 intra serdes PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8390_serdes_10ge(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    hal_control_t   *pHalCtrl = NULL;
    uint32  serdes_mode0 = 0;
    uint32  sdsId0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8390_serdes_10ge(unit %d, port %d) function!!", unit, port);

    /*For RTL8390 only port24 & port36 can be configured as 10G fiber port*/
    if (port == 24)
    {
        sdsId0 = 8;
    }
    else if (port == 36)
    {
        sdsId0 = 12;
    }
    else
        return RT_ERR_FAILED;

    RT_LOG(LOG_EVENT, MOD_HAL, "_phy_identify_8390_serdes_10ge(unit %d, port %d)!!", unit, port);
    if ((ret = reg_array_field_read(unit, CYPRESS_MAC_SERDES_IF_CTRLr, REG_ARRAY_INDEX_NONE, sdsId0, CYPRESS_SERDES_SPD_SELf, &serdes_mode0)) != RT_ERR_OK)
        return ret;
    switch (serdes_mode0)
    {
        case 1:
            return RT_ERR_OK;
        default:
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_FAILED;
} /* end  _phy_identify_8390_serdes_ge */

#endif

#if (defined(CONFIG_SDK_RTL8214FB) || defined(CONFIG_SDK_RTL8214B) || defined(CONFIG_SDK_RTL8212B))
/* Function Name:
 *      _phy_identify_8214B
 * Description:
 *      Identify the port is 8214B PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8214B PHY
 *      RT_ERR_FAILED - is not 8214B PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8214B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        return RT_ERR_FAILED;
    }

    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8214FB);
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 10, 18, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0x0 == ((data) & 0xF))
        {
            if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port+3, 8, 18, 0x93f0) != RT_ERR_OK)
                return RT_ERR_FAILED;
            if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port+3, 8, 19, &data) != RT_ERR_OK)
                return RT_ERR_FAILED;

            if (((data & 0xF) == 0xF) || ((data & 0xF) == 0xC) || ((data & 0xF) == 0x0))
                return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8214B */

/* Function Name:
 *      _phy_identify_8214FB
 * Description:
 *      Identify the port is 8214FB PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8214FB PHY
 *      RT_ERR_FAILED - is not 8214FB PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8214FB(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) failed - real_model_id = 0x%x, real_rev_id = 0x%x!!", unit, port, real_model_id, real_rev_id);
        return RT_ERR_FAILED;
    }

    RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) real_model_id = 0x%x, real_rev_id= 0x%x!!", unit, port, real_model_id, real_rev_id);
    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_OUI_check(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8214FB);
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 10, 18, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0xC == ((data) & 0xF))
            return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8214FB */

/* Function Name:
 *      _phy_identify_8212B
 * Description:
 *      Identify the port is 8212B PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8212B PHY
 *      RT_ERR_FAILED - is not 8212B PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8212B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        return RT_ERR_FAILED;
    }

    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8214FB);
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 10, 18, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0x0 == ((data) & 0xF))
        {
            if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port+3, 8, 18, 0x93f0) != RT_ERR_OK)
                return RT_ERR_FAILED;
            if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port+3, 8, 19, &data) != RT_ERR_OK)
                return RT_ERR_FAILED;

            if (((data & 0xF) == 0xE) || ((data & 0xF) == 0x8))
            {
                if ((port-base_port) == 0 || (port-base_port) == 1)
                    return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8212B */
#endif

#if defined(CONFIG_SDK_RTL8218B)
/* Function Name:
 *      _phy_identify_8218B
 * Description:
 *      Identify the port is 8218B PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8218B PHY
 *      RT_ERR_FAILED - is not 8218B PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8218B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) failed - real_model_id = 0x%x, real_rev_id = 0x%x!!", unit, port, real_model_id, real_rev_id);
        return RT_ERR_FAILED;
    }

    RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) real_model_id = 0x%x, real_rev_id= 0x%x!!", unit, port, real_model_id, real_rev_id);
    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_OUI_check(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        if ((PHY_REV_NO_A == real_rev_id) && (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8380_FAMILY_ID(unit)))
        {
            return RT_ERR_OK;
        }

        base_port = port - (port % PORT_NUM_IN_8218B);
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a42, 29, 0x0008) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0278, 18, 0x0455) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 0x0260, 18, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a42, 29, 0x0000) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0xD == ((data >> CFG_MODE_CHIP_OFFSET) & 0xF) || 0xF == ((data >> CFG_MODE_CHIP_OFFSET) & 0xF))
            return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8218B */
#endif

#if defined(CONFIG_SDK_RTL8218C)
/* Function Name:
 *      _phy_identify_8218C
 * Description:
 *      Identify the port is 8218C PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8218C PHY
 *      RT_ERR_FAILED - is not 8218C PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8218C(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) failed - real_model_id = 0x%x, real_rev_id = 0x%x!!", unit, port, real_model_id, real_rev_id);
        return RT_ERR_FAILED;
    }
	
    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_OUI_check(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_FAILED;
    }
	
    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        if ((PHY_REV_NO_C == real_rev_id) && (HAL_IS_RTL8390_FAMILY_ID(unit) || HAL_IS_RTL8380_FAMILY_ID(unit)))
        {
            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8218C */
#endif


#if (defined(CONFIG_SDK_RTL8218FB) || defined(CONFIG_SDK_RTL8214FC))
/* Function Name:
 *      _phy_identify_8218FB
 * Description:
 *      Identify the port is 8218FB PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8218FB PHY
 *      RT_ERR_FAILED - is not 8218FB PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8218FB(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) failed - real_model_id = 0x%x, real_rev_id = 0x%x!!", unit, port, real_model_id, real_rev_id);
        return RT_ERR_FAILED;
    }

    RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) real_model_id = 0x%x, real_rev_id= 0x%x!!", unit, port, real_model_id, real_rev_id);
    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_OUI_check(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8218FB);

        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a42, 29, 0x0008) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0278, 18, 0x0455) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 0x0260, 18, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a42, 29, 0x0000) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0xC == ((data) & 0xF) || 0xE == ((data) & 0xF))
        {
            if (0xC == ((data>>4) & 0xF) || 0xE == ((data>>4) & 0xF))
                return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8218FB */

/* Function Name:
 *      _phy_identify_8218FB_MP
 * Description:
 *      Identify the port is 8218FB_MP PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8218FB_MP PHY
 *      RT_ERR_FAILED - is not 8218FB_MP PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8218FB_MP(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) failed - real_model_id = 0x%x, real_rev_id = 0x%x!!", unit, port, real_model_id, real_rev_id);
        return RT_ERR_FAILED;
    }

    RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) real_model_id = 0x%x, real_rev_id= 0x%x!!", unit, port, real_model_id, real_rev_id);
    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_OUI_check(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8218FB);

        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0008) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 0x260, 18, &data) != RT_ERR_OK)
            goto ERR;
        if (0xC != ((data>>4) & 0xF) && 0xE != ((data>>4) & 0xF))
            goto ERR;

        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0001) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a43, 19, 0x0002) != RT_ERR_OK)
            goto ERR;
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 0x0a43, 20, &data) != RT_ERR_OK)
            goto ERR;
        if (0x6276 != data)
            goto ERR;

        MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0000);
        return RT_ERR_OK;
    }

ERR:
    MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0000);

    return RT_ERR_FAILED;
} /* end of _phy_identify_8218FB_MP */

/* Function Name:
 *      _phy_identify_8214FC
 * Description:
 *      Identify the port is 8214FC PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8214FC PHY
 *      RT_ERR_FAILED - is not 8214FC PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8214FC(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) failed - real_model_id = 0x%x, real_rev_id = 0x%x!!", unit, port, real_model_id, real_rev_id);
        return RT_ERR_FAILED;
    }

    RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) real_model_id = 0x%x, real_rev_id= 0x%x!!", unit, port, real_model_id, real_rev_id);
    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_OUI_check(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8214FC);
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a42, 29, 0x0008) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0278, 18, 0x0455) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 0x0260, 18, &data) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0278, 18, 0x0) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a42, 29, 0x0000) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (0x4 == ((data>>4) & 0xF) || 0x6 == ((data>>4) & 0xF))
            return RT_ERR_OK;
    }

    return RT_ERR_FAILED;
} /* end of _phy_identify_8214FC */

/* Function Name:
 *      _phy_identify_8214FC_MP
 * Description:
 *      Identify the port is 8214FC_MP PHY or not?
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - is 8214FC_MP PHY
 *      RT_ERR_FAILED - is not 8214FC_MP PHY
 * Note:
 *      None
 */
static int32
_phy_identify_8214FC_MP(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
    uint32 real_model_id, real_rev_id;
    uint32 base_port = 0, data;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

    if (RT_ERR_OK != phy_identify_phyid_get(unit, port, &real_model_id, &real_rev_id))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) failed - real_model_id = 0x%x, real_rev_id = 0x%x!!", unit, port, real_model_id, real_rev_id);
        return RT_ERR_FAILED;
    }

    RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_phyid_get(unit %d, port %d) real_model_id = 0x%x, real_rev_id= 0x%x!!", unit, port, real_model_id, real_rev_id);
    if (RT_ERR_OK != phy_identify_OUI_check(unit, port))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "phy_identify_OUI_check(unit %d, port %d) failed!!", unit, port);
        return RT_ERR_FAILED;
    }

    if ((real_model_id == model_id) && (real_rev_id >= rev_id))
    {
        base_port = port - (port % PORT_NUM_IN_8214FC);

        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0008) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 0x260, 18, &data) != RT_ERR_OK)
            goto ERR;
        if (0x4 != ((data>>4) & 0xF) && 0x6 != ((data>>4) & 0xF))
            goto ERR;

        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0001) != RT_ERR_OK)
            return RT_ERR_FAILED;
        if (MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, 0x0a43, 19, 0x0002) != RT_ERR_OK)
            goto ERR;
        if (MACDRV(pHalCtrl)->fMdrv_miim_read(unit, base_port, 0x0a43, 20, &data) != RT_ERR_OK)
            goto ERR;
        if (0x6276 != data)
            goto ERR;

        MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0000);
        return RT_ERR_OK;
    }

ERR:
    MACDRV(pHalCtrl)->fMdrv_miim_write(unit, base_port, PHY_PAGE_0, 29, 0x0000);

    return RT_ERR_FAILED;
} /* end of _phy_identify_8214FC_MP */
#endif

/* Public Function Body */

/* Function Name:
 *      phy_identify_OUI_check
 * Description:
 *      Identify the OUI is the realtek OUI or not?
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Realtek OUI
 *      RT_ERR_FAILED - not Realtek OUI
 * Note:
 *      None
 */
int32
phy_identify_OUI_check(uint32 unit, rtk_port_t port)
{
    uint32  data0, data1;
    uint32  page;
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    RT_PARAM_CHK((NULL == MACDRV(pHalCtrl)->fMdrv_miim_read), RT_ERR_FAILED);

    /* E0005375 */
    /* Patch for RTL8208 PHY
     * Access PHY by currently page id
     */
    if (HAL_IS_FE_PORT(unit, port) &&
        (RTL8329M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8329_CHIP_ID == pHalCtrl->chip_id))
    {
        page = HAL_MIIM_PAGE_ID_MAX(unit); /* Patch case: access the currently page */
    }
    else
    {
        page = PHY_PAGE_0;  /* Normal case: access the page 0 */
    }
    /* End of E0005375 */

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, page, PHY_IDENTIFIER_1_REG, &data0)) != RT_ERR_OK)
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "fMdrv_miim_read(unit %d, port %d) PHY_IDENTIFIER_1_REG failed!!", unit, port);
        return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, page, PHY_IDENTIFIER_2_REG, &data1)) != RT_ERR_OK)
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "fMdrv_miim_read(unit %d, port %d) PHY_IDENTIFIER_2_REG failed!!", unit, port);
        return ret;
    }

    if ((data0 != PHY_IDENT_OUI_03_18) ||
        ((data1 >> OUI_19_24_OFFSET) != PHY_IDENT_OUI_19_24))
    {
        RT_DBG(LOG_TRACE, MOD_HAL, "Compare OUI data0 failed(unit %d, port %d): data0= 0x%x; PHY_IDENT_OUI_03_18 = 0x%x!!", unit, port, data0, PHY_IDENT_OUI_03_18);
        RT_DBG(LOG_TRACE, MOD_HAL, "Compare OUI data1 failed(unit %d, port %d): data1.b[24:19]= 0x%x; PHY_IDENT_OUI_19_24 = 0x%x!!", unit, port, (data1 >> OUI_19_24_OFFSET), PHY_IDENT_OUI_19_24);
        return RT_ERR_FAILED;
    }

    return ret;
} /* end of phy_identify_OUI_check */


/* Function Name:
 *      phy_identify_find
 * Description:
 *      Find this kind of PHY control structure from the phy supported list.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      NULL      - Not found
 *      Otherwise - Pointer of PHY control structure that found
 * Note:
 *      None
 */
rt_phyctrl_t *
phy_identify_find(uint32 unit, rtk_port_t port)
{
    int32  size = 0, i;

    size = sizeof(supported_phys) / sizeof(rt_phyctrl_t);

    for (i = size - 1; i >= 0; i--)
    {
        if ((supported_phys[i].chk_func)(unit, port, supported_phys[i].phy_model_id, supported_phys[i].phy_rev_id) == RT_ERR_OK)
        {
            return (&supported_phys[i]);
        }
    }

    return NULL;
} /* end of phy_identify_find */


/* Function Name:
 *      phy_identify_phyid_get
 * Description:
 *      Get this phy model id and its revision id from chip.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pModel_id - pointer buffer of phy model id
 *      pRev_id   - pointer buffer of phy revision id
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
phy_identify_phyid_get(
    uint32      unit,
    rtk_port_t  port,
    uint32      *pModel_id,
    uint32      *pRev_id)
{
    uint32  data;
    uint32  page;
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl = NULL;

    RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
    RT_PARAM_CHK((NULL == MACDRV(pHalCtrl)->fMdrv_miim_read), RT_ERR_FAILED);

    /* E0005375 */
    /* Patch for RTL8208 PHY
     * Access PHY by currently page id
     */
    if (HAL_IS_FE_PORT(unit, port) &&
        (RTL8329M_CHIP_ID == pHalCtrl->chip_id ||
         RTL8329_CHIP_ID == pHalCtrl->chip_id))
    {
        page = HAL_MIIM_PAGE_ID_MAX(unit); /* Patch case: access the currently page */
    }
    else
    {
        page = PHY_PAGE_0;   /* Normal case: access the page 0 */
    }
    /* End of E0005375 */

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, page, PHY_IDENTIFIER_2_REG, &data)) != RT_ERR_OK)
    {
        return ret;
    }

    if (PHY_MODEL_ID_RTL8201 == data)
    {
        *pModel_id = PHY_MODEL_ID_RTL8201;
        *pRev_id   = PHY_REV_NO_C;
    }
    else
    {
        /* model id is bit[9:4]; rev id is bit[3:0] */
        *pModel_id = (data & ModelNumber_MASK) >> ModelNumber_OFFSET;
        *pRev_id = (data & RevisionNumber_MASK) >> RevisionNumber_OFFSET;
    }

    return ret;
} /* end of phy_identify_phyid_get */


/* Function Name:
 *      phy_identify_int_find
 * Description:
 *      Find this kind of PHY control structure from the internal phy supported list.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      NULL      - Not found
 *      Otherwise - Pointer of PHY control structure that found
 * Note:
 *      None
 */
rt_phyctrl_t *
phy_identify_int_find(uint32 unit, rtk_port_t port)
{
    int32  size = 0, i;

    size = sizeof(supported_int_phys) / sizeof(rt_phyctrl_t);

    for (i = size - 1; i >= 0; i--)
    {
        if ((supported_int_phys[i].chk_func)(unit, port, supported_int_phys[i].phy_model_id, supported_int_phys[i].phy_rev_id) == RT_ERR_OK)
        {
            return (&supported_int_phys[i]);
        }
    }

    return NULL;
} /* end of phy_identify_int_find */
