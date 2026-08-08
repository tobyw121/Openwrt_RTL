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
 * $Revision: 12996 $
 * $Date: 2010-09-28 11:07:02 +0800 (Tue, 28 Sep 2010) $
 *
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *
 * Feature :  Parameter settings for the system-wise view 
 *
 */
#ifndef __DAL_SSW_SWITCH_H__ 
#define __DAL_SSW_SWITCH_H__

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/switch.h>

/* 
 * Symbol Definition 
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
 *      dal_ssw_switch_init
 * Description:
 *      Initialize switch module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
extern int32
dal_ssw_switch_init(uint32 unit);

/* Function Name:
 *      dal_ssw_switch_maxPktLen_get
 * Description:
 *      Get the max packet length setting of the specific unit
 * Input:
 *      unit                  - unit id
 * Output:
 *      pLen                 - pointer to the max packet length
 * Return:
 *      RT_ERR_OK 
 *      RT_ERR_FAILED  
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer 
 * Note:
 *      Max packet length setting
 *        - MAXPKTLEN_1522B  
 *	      - MAXPKTLEN_1536B  
 *	      - MAXPKTLEN_1552B  
 *        - MAXPKTLEN_9216B  
 */
extern int32
dal_ssw_switch_maxPktLen_get(uint32 unit, rtk_switch_maxPktLen_t *pLen);

/* Function Name:
 *      dal_ssw_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      unit           - unit id
 *      len            - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 
 *      RT_ERR_FAILED  
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid enum packet length
 * Note:
 *      Max packet length setting
 *        - MAXPKTLEN_1522B
 *	      - MAXPKTLEN_1536B
 *	      - MAXPKTLEN_1552B
 *        - MAXPKTLEN_9216B
 */
extern int32
dal_ssw_switch_maxPktLen_set(uint32 unit, rtk_switch_maxPktLen_t len);

/* Function Name:
 *      dal_ssw_switch_mgmtMacAddr_get
 * Description:
 *      Get Mac address of switch.
 * Input:
 *      unit - unit id
 * Output:
 *      pMac - pointer to Mac address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_switch_mgmtMacAddr_get(uint32 unit, rtk_mac_t *pMac);

/* Function Name:
 *      dal_ssw_switch_mgmtMacAddr_set
 * Description:
 *      Set Mac address of switch.
 * Input:
 *      unit - unit id
 *      pMac - pointer to Mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_switch_mgmtMacAddr_set(uint32 unit, rtk_mac_t *pMac);

/* Function Name:
 *      dal_ssw_switch_hwInterfaceDelayEnable_get
 * Description:
 *      Get the delay state of the specified type in the specified unit.
 * Input:
 *      unit    - unit id
 *      type    - interface delay type
 * Output:
 *      pEnable - pointer to enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_switch_hwInterfaceDelayEnable_get(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_switch_hwInterfaceDelayEnable_set
 * Description:
 *      Set the delay state of the specified type in the specified unit.
 * Input:
 *      unit   - unit id
 *      type   - interface delay type
 *      enable - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_switch_hwInterfaceDelayEnable_set(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t enable);

#endif /* __DAL_SSW_SWITCH_H__ */
