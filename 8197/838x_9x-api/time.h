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
 * $Revision: 27279 $
 * $Date: 2014-02-26 19:43:48 +0800 (Wed, 26 Feb 2014) $
 *
 * Purpose : Definition of TIME API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) IEEE 1588
 *
 */

#ifndef __RTK_TIME_H__
#define __RTK_TIME_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */

/* TIME transmission callback function prototype */
typedef int32 (rtk_time_ptpTime_cb_f)(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_timeStamp_t        time);

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
 *      rtk_time_init
 * Description:
 *      Initialize Time module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8390
 * Note:
 *      Must initialize Time module before calling any Time APIs.
 */
extern int32
rtk_time_init(uint32 unit);

/* Function Name:
 *      rtk_time_portPtpEnable_get
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_portPtpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_time_portPtpEnable_set
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_portPtpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_time_portPtpRxTimestamp_get
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_portPtpRxTimestamp_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_timeStamp_t        *pTimeStamp);

/* Function Name:
 *      rtk_time_portPtpTxTimestamp_get
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_portPtpTxTimestamp_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_timeStamp_t        *pTimeStamp);

/* Function Name:
 *      rtk_time_portPtpTxTimestampCallback_register
 * Description:
 *      Register PTP transmission callback function of the PTP identifier on the dedicated port to the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      identifier - indentifier of PTP packet
 *      fCb        - callback function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_portPtpTxTimestampCallback_register(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_ptpTime_cb_f       *fCb);

/* Function Name:
 *      rtk_time_refTime_get
 * Description:
 *      Get the reference time of the specified device.
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_refTime_get(uint32 unit, rtk_time_timeStamp_t *pTimeStamp);

/* Function Name:
 *      rtk_time_refTime_set
 * Description:
 *      Set the reference time of the specified device.
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_refTime_set(uint32 unit, rtk_time_timeStamp_t timeStamp);

/* Function Name:
 *      rtk_time_refTimeAdjust_set
 * Description:
 *      Adjust the reference time.
 * Input:
 *      unit      - unit id
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      sign=0 for positive adjustment, sign=1 for negative adjustment.
 */
extern int32
rtk_time_refTimeAdjust_set(uint32 unit, uint32 sign, rtk_time_timeStamp_t timeStamp);

/* Function Name:
 *      rtk_time_refTimeEnable_get
 * Description:
 *      Get the enable state of reference time of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_refTimeEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_time_refTimeEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_time_refTimeEnable_set(uint32 unit, rtk_enable_t enable);

#endif /* __RTK_TIME_H__ */
