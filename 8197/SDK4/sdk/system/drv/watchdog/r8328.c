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
 * $Revision: 34373 $
 * $Date: 2012-11-15 16:09:52 +0800 (Thu, 15 Nov 2012) $
 *
 * Purpose : Definition those public watchdog APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) mode set & get
 *            2) scale set & get
 */

/*
 * Include Files
 */
#include <soc/soc.h>
#include <soc/type.h>
#include <ioal/mem32.h>
#include <drv/watchdog/r8328.h>
#include <drv/swcore/rtl8328.h>

/*
 * Symbol Definition
 */

/*
 * Data Type Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      r8328_watchdog_init
 * Description:
 *      Init the watchdog module of the specified device.
 * Input:
 *      unit - unit id 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
r8328_watchdog_init(uint32 unit)
{
    return RT_ERR_OK;
} /* end of r8328_watchdog_init */

/* Function Name:
 *      r8328_watchdog_mode_set
 * Description:
 *      Set watchdog as normal or interrupt mode
 * Input:
 *      unit - unit id
 *      mode - watchdog mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - set watchdog mode success.
 *      RT_ERR_FAILED - set watchdog mode fail.
 * Note:
 *      None
 */
int32
r8328_watchdog_mode_set(uint32 unit, drv_watchdog_mode_t mode)
{
    switch (mode) 
    {
        case WATCHDOG_MODE_NORMAL:
    		ioal_mem32_field_write(unit, RTL8328_RESET_GLOBAL_CONTROL_ADDR, RTL8328_RESET_GLOBAL_CONTROL_WDOG_INT_EN_OFFSET, RTL8328_RESET_GLOBAL_CONTROL_WDOG_INT_EN_MASK, 0);
            break;

        case WATCHDOG_MODE_INTERRUPT:
    		ioal_mem32_field_write(unit, RTL8328_RESET_GLOBAL_CONTROL_ADDR, RTL8328_RESET_GLOBAL_CONTROL_WDOG_INT_EN_OFFSET, RTL8328_RESET_GLOBAL_CONTROL_WDOG_INT_EN_MASK, 1);
            break;

        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of r8328_watchdog_mode_set */


/* Function Name:
 *      r8328_watchdog_mode_get
 * Description:
 *      Get watchdog mode
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - watchdog mode
 * Return:
 *      RT_ERR_OK           - get watchdog mode successfully.
 *      RT_ERR_NULL_POINTER - pMode is a null pointer.
 * Note:
 *      None
 */
int32
r8328_watchdog_mode_get(uint32 unit, drv_watchdog_mode_t *pMode)
{
    uint32 temp = 0;

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);      

    /* Get watchdog mode */
    ioal_mem32_field_read(unit, RTL8328_RESET_GLOBAL_CONTROL_ADDR, RTL8328_RESET_GLOBAL_CONTROL_WDOG_INT_EN_OFFSET, RTL8328_RESET_GLOBAL_CONTROL_WDOG_INT_EN_MASK, &temp);
    if (0 == temp)
    {
        *pMode = WATCHDOG_MODE_NORMAL;
    }
    else
    {
        *pMode = WATCHDOG_MODE_INTERRUPT;
    }

    return RT_ERR_OK;
} /* end of r8328_watchdog_mode_get */

/* Function Name:
 *      r8328_watchdog_scale_set
 * Description:
 *      Set watchdog expired period
 * Input:
 *      unit  - unit id
 *      scale - period scale
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - succeed in setting watchdog expired period.
 *      RT_ERR_FAILED - fail to set watchdog expired period.
 * Note:
 *      None
 */
int32 
r8328_watchdog_scale_set(uint32 unit, drv_watchdog_scale_t scale)
{
    uint32 newWDTCNR = 0;

    /* set ovsel field to be 2b 00 */
    newWDTCNR = (REG32(WDTCNR) & (~WDT_SCALE_MASK));

    switch (scale)
    {
        case WATCHDOG_SCALE_1:                     /* fire after 0.16 sec */
            newWDTCNR = (newWDTCNR | WDT_SCALE_1); /* 2b 00, 2^15 */
            break;

        case WATCHDOG_SCALE_2:                     /* fire after 0.32 sec */
            newWDTCNR = (newWDTCNR | WDT_SCALE_2); /* 2b 01, 2^16 */
            break;

        case WATCHDOG_SCALE_3:                     /* fire after 0.65 sec */
            newWDTCNR = (newWDTCNR | WDT_SCALE_3); /* 2b 10, 2^17 */
            break;

        case WATCHDOG_SCALE_4:                     /* fire after 1.31 sec */
            newWDTCNR = (newWDTCNR | WDT_SCALE_4); /* 2b 11, 2^18 */
            break;

        default:
            return RT_ERR_FAILED;
    }

    REG32(WDTCNR) = newWDTCNR;

    return RT_ERR_OK;
} /* end of r8328_watchdog_scale_set */

/* Function Name:
 *      r8328_watchdog_scale_get
 * Description:
 *      Get watchdog expired period scale
 * Input:
 *      unit   - unit id
 * Output:
 *      pScale - period scale
 * Return:
 *      RT_ERR_OK           - get watchdog expired period scale successfully.
 *      RT_ERR_FAILED       - fail to get get watchdog expired period scale. 
 *      RT_ERR_NULL_POINTER - pScale is a null pointer.
 * Note:
 *      None
 */
int32 
r8328_watchdog_scale_get(uint32 unit, drv_watchdog_scale_t *pScale)
{
    /* parameter check */
    RT_PARAM_CHK((NULL == pScale), RT_ERR_NULL_POINTER);     

    /* Get overflow select */
    *pScale = (REG32(WDTCNR) & WDT_SCALE_MASK) >> WDT_SCALE_OFFSET;

    return RT_ERR_OK;
} /* end of r8328_watchdog_scale_get */

/* Function Name:
 *      r8328_watchdog_enable_set
 * Description:
 *      Set watchdog enable/disable
 * Input:
 *      unit   - unit id
 *      enable - enable or disable request
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - enable watchdog successfully.
 *      RT_ERR_FAILED - fail to enable watchdog.
 * Note:
 *      None
 */
int32 
r8328_watchdog_enable_set(uint32 unit, uint32 enable)
{
    uint32 v_wdtcnr = 0;

    /* Get old watchdog controller value */
    v_wdtcnr = REG32(WDTCNR);
    
    /* Clear watch dog enable/disable field. */
    v_wdtcnr = v_wdtcnr & (~WDT_ENABLE_MASK);

    /* Set watchdog enable/disable */
    switch (enable) 
    {
        case DISABLED:          /* Disable */
            REG32(WDTCNR) = v_wdtcnr | WDT_DISABLE;
            break;

        case ENABLED:           /* Enable */
            REG32(WDTCNR) = v_wdtcnr | WDT_ENABLE;
            break;

        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of r8328_watchdog_enable_set */

/* Function Name:
 *      r8328_watchdog_enable_get
 * Description:
 *      Get watchdog enable/disable status
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - watchdog enable/disable status
 * Return:
 *      RT_ERR_OK           - get watchdog enable/disable status successfully.
 *      RT_ERR_NULL_POINTER - pEnable is a null pointer.
 * Note:
 *      None
 */
int32 
r8328_watchdog_enable_get(uint32 unit, uint32 *pEnable)
{
    uint32 v_wdtcnr = 0;

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);       

    /* Get watchdog enable/disable field value */
    v_wdtcnr = REG32(WDTCNR) & WDT_ENABLE_MASK;
    
    /* watchdog disable */
    if (WDT_DISABLE == v_wdtcnr)
    {
        *pEnable = DISABLED;
    }
    else
    {
        *pEnable = ENABLED;
    }

    return RT_ERR_OK;
} /* end of r8328_watchdog_enable_get */

/* Function Name:
 *      r8328_watchdog_kick
 * Description:
 *      Kick watchdog
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - kick watchdog successfully.
 * Note:
 *      None
 */
int32 
r8328_watchdog_kick(uint32 unit)
{
    /* Clear watchdog pending flag */
    REG32(WDTCNR) |= WDT_CLR;

    return RT_ERR_OK;
} /* end of r8328_watchdog_kick */
