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
 * $Revision: 46645 $
 * $Date: 2014-02-26 19:43:48 +0800 (Wed, 26 Feb 2014) $
 *
 * Purpose : Definition of TIME API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) IEEE 1588
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
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
int32
rtk_time_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->time_init(unit);
} /* end of rtk_time_init */

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
int32
rtk_time_portPtpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->time_portPtpEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->time_portPtpEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_time_portPtpEnable_get */

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
int32
rtk_time_portPtpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->time_portPtpEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->time_portPtpEnable_set(unit, port, enable);
#endif
} /* end of rtk_time_portPtpEnable_set */

/* Function Name:
 *      rtk_time_portPtpRxTimestamp_get
 * Description:
 *      Get PTP timstamp according to the PTP identifier on the dedicated port from the specified device.
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
int32
rtk_time_portPtpRxTimestamp_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_timeStamp_t        *pTimeStamp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->time_portPtpRxTimestamp_get(unit, phy_port, identifier, pTimeStamp);
    }
#else
    return RT_MAPPER(unit)->time_portPtpRxTimestamp_get(unit, port, identifier, pTimeStamp);
#endif
} /* end of rtk_time_portPtpRxTimestamp_get */

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
int32
rtk_time_portPtpTxTimestamp_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_timeStamp_t        *pTimeStamp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->time_portPtpTxTimestamp_get(unit, phy_port, identifier, pTimeStamp);
    }
#else
    return RT_MAPPER(unit)->time_portPtpTxTimestamp_get(unit, port, identifier, pTimeStamp);
#endif
} /* end of rtk_time_portPtpTxTimestamp_get */

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
int32
rtk_time_portPtpTxTimestampCallback_register(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_ptpTime_cb_f       *fCb)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->time_portPtpTxTimestampCallback_register(unit, phy_port, identifier, fCb);
    }
#else
    return RT_MAPPER(unit)->time_portPtpTxTimestampCallback_register(unit, port, identifier, fCb);
#endif
} /* end of rtk_time_portPtpTxTimestampCallback_register */

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
int32
rtk_time_refTime_get(uint32 unit, rtk_time_timeStamp_t *pTimeStamp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->time_refTime_get(unit, pTimeStamp);
} /* end of rtk_time_refTime_get */

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
int32
rtk_time_refTime_set(uint32 unit, rtk_time_timeStamp_t timeStamp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->time_refTime_set(unit, timeStamp);
} /* end of rtk_time_refTime_set */

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
int32
rtk_time_refTimeAdjust_set(uint32 unit, uint32 sign, rtk_time_timeStamp_t timeStamp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->time_refTimeAdjust_set(unit, sign, timeStamp);
} /* end of rtk_time_refTimeAdjust_set */

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
int32
rtk_time_refTimeEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->time_refTimeEnable_get(unit, pEnable);
} /* end of rtk_time_refTimeEnable_get */

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
int32
rtk_time_refTimeEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->time_refTimeEnable_set(unit, enable);
} /* end of rtk_time_refTimeEnable_set */
