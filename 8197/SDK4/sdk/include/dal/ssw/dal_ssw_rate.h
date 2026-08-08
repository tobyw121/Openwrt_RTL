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
 * $Revision: 27968 $
 * $Date: 2012-04-12 14:45:14 +0800 (Thu, 12 Apr 2012) $
 *
 * Purpose : Definition those public port bandwidth control and storm control APIs and its data type
 *           in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *             1) Configuration of ingress port bandwidth control (ingress rate limit ).
 *             2) Configuration of egress port bandwidth control (egress rate limit).
 *             3) Configuration of storm control
 *
 */

#ifndef __DAL_SSW_RATE_H__
#define __DAL_SSW_RATE_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/rate.h>
#include <rtk/port.h>
#include <rtk/rate.h>


/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_ssw_rate_init
 * Description:
 *      Initial the rate module of the specified device..
 * Input:
 *      unit                - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32
dal_ssw_rate_init(uint32 unit);

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlEnable_get
 * Description:
 *      Get the ingress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pEnable            - status of ingress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_rate_igrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlEnable_set
 * Description:
 *      Set the ingress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      enable              - status of ingress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_rate_igrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get the ingress bandwidth control rate.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pRate      - ingress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_ssw_rate_igrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set the ingress bandwidth control rate.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      rate        - ingress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_RATE          - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_ssw_rate_igrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 * Output:
 *      pIfg_include           - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_ssw_rate_igrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include);

/* Function Name:
 *      dal_ssw_rate_igrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 *      ifg_include             - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_INPUT                 - Invalid input parameter
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_ssw_rate_igrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include);

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlEnable_get
 * Description:
 *      Get the egress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pEnable            - status of egress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_rate_egrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlEnable_set
 * Description:
 *      Set the egress bandwidth control status.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      enable              - status of egress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT     - Invalid input parameter
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_rate_egrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pRate          - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_ssw_rate_egrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      rate                - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_RATE          - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_ssw_rate_egrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of egress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 * Output:
 *      pIfg_include           - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_INPUT                 - Invalid input parameter
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      1. Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_ssw_rate_egrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include);

/* Function Name:
 *      dal_ssw_rate_egrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of egress bandwidth control includes IFG or not.
 * Input:
 *      unit                    - unit id
 *      ifg_include             - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_INPUT                 - Invalid input parameter
 * Note:
 *      1. Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_ssw_rate_egrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include);

/* Function Name:
 *      dal_ssw_rate_stormControlRate_get
 * Description:
 *      Get the storm control rate.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      storm_type      - storm group type
 * Output:
 *      pRate          - storm control rate (packet-per-second).
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_ssw_rate_stormControlRate_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pRate);

/* Function Name:
 *      dal_ssw_rate_stormControlRate_set
 * Description:
 *      Set the storm control rate.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      storm_type      - storm group type
 *      rate            - storm control rate (packet-per-second).
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_RATE          - Invalid input bandwidth
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_ssw_rate_stormControlRate_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  rate);

/* Function Name:
 *      dal_ssw_rate_igrBandwidthFCOffRate_set
 * Description:
 *      Set the ingress bandwidth control flow control off rate.
 * Input:
 *      unit - unit id
 *      rate - ingress bandwidth control flow control off rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      None
 */
extern int32
dal_ssw_rate_igrBandwidthFCOffRate_set(uint32 unit, uint32 rate);

#endif /* __DAL_SSW_RATE_H__ */
