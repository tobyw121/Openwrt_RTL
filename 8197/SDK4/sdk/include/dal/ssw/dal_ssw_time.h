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
 * $Revision: 27031 $
 * $Date: 2012-03-06 20:43:11 +0800 (Tue, 06 Mar 2012) $
 *
 * Purpose : Definition of TIME API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) IEEE 1588
 *
 */

#ifndef __DAL_SSW_TIME_H__
#define __DAL_SSW_TIME_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/time.h>


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

/* Module Name : TIME */

/* Function Name:
 *      dal_ssw_time_init
 * Description:
 *      Initialize ptp module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize ptp module before calling any ptp APIs.
 */
extern int32
dal_ssw_time_init(uint32 unit);

/* Function Name:
 *      dal_ssw_time_portPtpEnable_get
 * Description:
 *      Get TIME status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT          - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_time_portPtpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_time_portPtpEnable_set
 * Description:
 *      Set TIME status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_ssw_time_portPtpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_time_portPtpRxTimestamp_get
 * Description:
 *      Get TIME timstamp of the TIME identifier of the dedicated port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      identifier - indentifier of TIME packet
 * Output:
 *      pTimeStamp - pointer buffer of TIME timestamp
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_time_portPtpRxTimestamp_get(
    uint32 unit,
    rtk_port_t port,
    rtk_time_ptpIdentifier_t identifier,
    rtk_time_timeStamp_t *pTimeStamp);

/* Function Name:
 *      dal_ssw_time_portPtpTxTimestampCallback_register
 * Description:
 *      Register TIME transmission callback function of the TIME identifier of the dedicated port to the specified device.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      identifier        - indentifier of TIME packet
 *      rtk_ptp_time_cb_f - callback function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Note:
 *      None
 */
extern int32
dal_ssw_time_portPtpTxTimestampCallback_register(
    uint32 unit,
    rtk_port_t port,
    rtk_time_ptpIdentifier_t identifier,
    rtk_time_ptpTime_cb_f *fCb);

/* Function Name:
 *      dal_ssw_time_refTime_get
 * Description:
 *      Get the reference time of TIME of the specified device.
 * Input:
 *      unit  - unit id
 * Output:
 *      pTimeStamp - pointer buffer of TIME reference time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_time_refTime_get(uint32 unit, rtk_time_timeStamp_t *pTimeStamp);

/* Function Name:
 *      dal_ssw_time_refTime_set
 * Description:
 *      Set the reference time of TIME of the specified device.
 * Input:
 *      unit      - unit id
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
extern int32
dal_ssw_time_refTime_set(uint32 unit, rtk_time_timeStamp_t timeStamp);

/* Function Name:
 *      dal_ssw_ptp_timeAdjust_set
 * Description:
 *      Adjust TIME reference time.
 * Input:
 *      unit      - unit id
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_ssw_time_refTimeAdjust_set(uint32 unit, uint32 sign, rtk_time_timeStamp_t timeStamp);

/* Function Name:
 *      dal_ssw_time_refTimeEnable_get
 * Description:
 *      Get the enable state of reference time of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_time_refTimeEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_time_refTimeEnable_set
 * Description:
 *      Set the enable state of reference time of the specified device.
 * Input:
 *      unit   - unit id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_ssw_time_refTimeEnable_set(uint32 unit, rtk_enable_t enable);

#endif /* __DAL_SSW_TIME_H__ */

