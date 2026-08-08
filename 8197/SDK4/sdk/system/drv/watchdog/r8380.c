/*
 * Copyright (C) 2012 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 37666 $
 * $Date: 2013-03-12 14:43:25 +0800 (Tue, 12 Mar 2013) $
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
#include <linux/version.h> 
#include <soc/soc.h>
#include <soc/type.h>
#include <ioal/mem32.h>
#include <drv/watchdog/r8380.h>
#include <drv/swcore/rtl8380.h>
#include <drv/swcore/chip.h>

/*
 * Symbol Definition
 */

/*
 * Data Type Definition
 */

/*
 * Data Declaration
 */
extern uint32 wdg_chipId[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      r8380_watchdog_init
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
r8380_watchdog_init(uint32 unit)
{
    int32   ret;
    drv_watchdog_threshold_t threshold;

    threshold.phase_1_threshold = 10;
    threshold.phase_2_threshold = 0;
    if ((ret = r8380_watchdog_threshold_set(unit, &threshold)) != RT_ERR_OK)
        return ret;

    return RT_ERR_OK;
} /* end of r8380_watchdog_init */

/* Function Name:
 *      r8380_watchdog_mode_set
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
r8380_watchdog_mode_set(uint32 unit, drv_watchdog_mode_t mode)
{
#if 0
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
#endif
    return RT_ERR_OK;
} /* end of r8380_watchdog_mode_set */


/* Function Name:
 *      r8380_watchdog_mode_get
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
r8380_watchdog_mode_get(uint32 unit, drv_watchdog_mode_t *pMode)
{
#if 0
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
#endif
    return RT_ERR_OK;
} /* end of r8380_watchdog_mode_get */

/* Function Name:
 *      r8380_watchdog_scale_set
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
r8380_watchdog_scale_set(uint32 unit, drv_watchdog_scale_t scale)
{
    uint32  newWDTCTRL = 0;
    uint32  chipId = 0, chipRevId = 0;
    uint32  flag_es, reg_addr;
    int32   ret;

    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    flag_es = 0;
    if ((chipId & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        reg_addr = RTL8380ES_WDTCTRL;
    else
        reg_addr = RTL8380MP_WDTCTRL;
    
    /* set WDT_CLK_SC field to be 2b 00 */
    newWDTCTRL = (REG32(reg_addr) & (~RTL8380_WDT_WDT_CLK_SC_MASK));
    switch (scale)
    {
        case WATCHDOG_SCALE_1:                     /* fire after 1.7 sec */
            newWDTCTRL = (newWDTCTRL | RTL8380_WDT_SCALE_1); /* 2b 00, 2^25 */
            break;

        case WATCHDOG_SCALE_2:                     /* fire after 3.4 sec */
            newWDTCTRL = (newWDTCTRL | RTL8380_WDT_SCALE_2); /* 2b 01, 2^26 */
            break;

        case WATCHDOG_SCALE_3:                     /* fire after 6.7 sec */
            newWDTCTRL = (newWDTCTRL | RTL8380_WDT_SCALE_3); /* 2b 10, 2^27 */
            break;

        case WATCHDOG_SCALE_4:                     /* fire after 13.4 sec */
            newWDTCTRL = (newWDTCTRL | RTL8380_WDT_SCALE_4); /* 2b 11, 2^28 */
            break;

        default:
            return RT_ERR_FAILED;
    }

    REG32(reg_addr) = newWDTCTRL;

    return RT_ERR_OK;
} /* end of r8380_watchdog_scale_set */

/* Function Name:
 *      r8380_watchdog_scale_get
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
r8380_watchdog_scale_get(uint32 unit, drv_watchdog_scale_t *pScale)
{
    uint32  chipId = 0, chipRevId = 0;
    uint32  flag_es, reg_addr;
    int32   ret;

    /* parameter check */
    RT_PARAM_CHK((NULL == pScale), RT_ERR_NULL_POINTER);     

    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    flag_es = 0;
    if ((chipId & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        reg_addr = RTL8380ES_WDTCTRL;
    else
        reg_addr = RTL8380MP_WDTCTRL;

    /* Get WDT_CLK_SC field */
    *pScale = (REG32(reg_addr) & RTL8380_WDT_WDT_CLK_SC_MASK) >> RTL8380_WDT_WDT_CLK_SC_OFFSET;

    return RT_ERR_OK;
} /* end of r8380_watchdog_scale_get */

/* Function Name:
 *      r8380_watchdog_enable_set
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
r8380_watchdog_enable_set(uint32 unit, uint32 enable)
{
    uint32  v_wdtcnr = 0;
    uint32  chipId = 0, chipRevId = 0;
    uint32  flag_es, reg_addr;
    int32   ret;

    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    flag_es = 0;
    if ((chipId & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        reg_addr = RTL8380ES_WDTCTRL;
    else
        reg_addr = RTL8380MP_WDTCTRL;

    /* Get old watchdog controller value */
    v_wdtcnr = REG32(reg_addr);
    
    /* Clear watch dog enable/disable field. */
    v_wdtcnr = v_wdtcnr & (~RTL8380_WDT_WDT_E_MASK);
	
    /* Set Reset Mode to Whole chip */
    v_wdtcnr = v_wdtcnr & (~RTL8380_WDT_RESET_MODE_MASK);
    v_wdtcnr = v_wdtcnr | (RTL8380_WDT_RESET_MODE_FULL_CHIP);

    REG32(reg_addr) = v_wdtcnr;


    /* Set watchdog enable/disable */
    switch (enable) 
    {
        case DISABLED:          /* Disable */
            REG32(WDTCNR) = v_wdtcnr;
            REG32(GIMR) &= ~(WDT_IP1_IE | WDT_IP2_IE);
            break;
        case ENABLED:           /* Enable */
            REG32(GIMR) |= (WDT_IP1_IE | WDT_IP2_IE);
            REG32(reg_addr) = v_wdtcnr | (1 << RTL8380_WDT_WDT_E_OFFSET);
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of r8380_watchdog_enable_set */

/* Function Name:
 *      r8380_watchdog_enable_get
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
r8380_watchdog_enable_get(uint32 unit, uint32 *pEnable)
{
    uint32  v_wdtcnr = 0;
    uint32  chipId = 0, chipRevId = 0;
    uint32  flag_es, reg_addr;
    int32   ret;

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);       

    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    flag_es = 0;
    if ((chipId & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        reg_addr = RTL8380ES_WDTCTRL;
    else
        reg_addr = RTL8380MP_WDTCTRL;

    /* Get watchdog controller value */
    v_wdtcnr = REG32(reg_addr);
    (*pEnable) = (v_wdtcnr & RTL8380_WDT_WDT_E_MASK) >> RTL8380_WDT_WDT_E_OFFSET;

    return RT_ERR_OK;
} /* end of r8380_watchdog_enable_get */

/* Function Name:
 *      r8380_watchdog_kick
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
r8380_watchdog_kick(uint32 unit)
{
    uint32  flag_es, reg_addr;

    flag_es = 0;
    if ((wdg_chipId[unit] & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        reg_addr = RTL8380ES_WDTCNTR;
    else
        reg_addr = RTL8380MP_WDTCNTR;

    /* Clear watchdog pending flag */
    REG32(reg_addr) |= RTL8380_WDT_KICK_MASK;

    return RT_ERR_OK;
} /* end of r8380_watchdog_kick */

/* Function Name:
 *      r8380_watchdog_threshold_set
 * Description:
 *      Set watchdog threshold counter of the specified device
 * Input:
 *      unit       - unit id
 *      pThreshold - watchdog threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - successfully.
 *      RT_ERR_NULL_POINTER - pThreshold is a null pointer.
 *      RT_ERR_INPUT        - invalid input argument
 * Note:
 *      None
 */
int32 
r8380_watchdog_threshold_set(uint32 unit, drv_watchdog_threshold_t *pThreshold)
{
    uint32  v_wdtcnr = 0;
    uint32  chipId = 0, chipRevId = 0;
    uint32  flag_es, reg_addr;
    int32   ret;

    /* parameter check */
    RT_PARAM_CHK((NULL == pThreshold), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pThreshold->phase_1_threshold > 31), RT_ERR_INPUT);
    RT_PARAM_CHK((pThreshold->phase_2_threshold > 31), RT_ERR_INPUT);

    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    flag_es = 0;
    if ((chipId & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        reg_addr = RTL8380ES_WDTCTRL;
    else
        reg_addr = RTL8380MP_WDTCTRL;

    /* Get old watchdog controller value */
    v_wdtcnr = REG32(reg_addr);
    
    /* Set the PH1_TO & PH2_TO field. */
    v_wdtcnr = v_wdtcnr & (~RTL8380_WDT_PH1_TO_MASK);
    v_wdtcnr = v_wdtcnr | (pThreshold->phase_1_threshold << RTL8380_WDT_PH1_TO_OFFSET);
    v_wdtcnr = v_wdtcnr & (~RTL8380_WDT_PH2_TO_MASK);
    v_wdtcnr = v_wdtcnr | (pThreshold->phase_2_threshold << RTL8380_WDT_PH2_TO_OFFSET);

    REG32(reg_addr) = v_wdtcnr;

    return RT_ERR_OK;
} /* end of r8380_watchdog_threshold_set */

/* Function Name:
 *      r8380_watchdog_threshold_get
 * Description:
 *      Get watchdog threshold counter of the specified device
 * Input:
 *      unit       - unit id
 * Output:
 *      pThreshold - watchdog threshold
 * Return:
 *      RT_ERR_OK - successfully.
 *      RT_ERR_NULL_POINTER - pThreshold is a null pointer.
 * Note:
 *      None
 */
int32 
r8380_watchdog_threshold_get(uint32 unit, drv_watchdog_threshold_t *pThreshold)
{
    uint32  v_wdtcnr = 0;
    uint32  chipId = 0, chipRevId = 0;
    uint32  flag_es, reg_addr;
    int32   ret;

    /* parameter check */
    RT_PARAM_CHK((NULL == pThreshold), RT_ERR_NULL_POINTER);

    if ((ret = drv_swcore_cid_get(unit, &chipId, &chipRevId)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    flag_es = 0;
    if ((chipId & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        reg_addr = RTL8380ES_WDTCTRL;
    else
        reg_addr = RTL8380MP_WDTCTRL;

    /* Get old watchdog controller value */
    v_wdtcnr = REG32(reg_addr);
    pThreshold->phase_1_threshold = (v_wdtcnr & RTL8380_WDT_PH1_TO_MASK) >> RTL8380_WDT_PH1_TO_OFFSET;
    pThreshold->phase_2_threshold = (v_wdtcnr & RTL8380_WDT_PH2_TO_MASK) >> RTL8380_WDT_PH2_TO_OFFSET;

    return RT_ERR_OK;
} /* end of r8380_watchdog_threshold_get */

