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
 * $Revision: 34249 $
 * $Date: 2012-11-12 17:43:18 +0800 (Mon, 12 Nov 2012) $
 *
 * Purpose : Definition those public port bandwidth control and storm control APIs and its data type
 *           in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Configuration of ingress port bandwidth control (ingress rate limit ).
 *           2) Configuration of egress port bandwidth control (egress rate limit).
 *           3) Configuration of storm control
 *
 */

#ifndef __DAL_MAPLE_RATE_H__
#define __DAL_MAPLE_RATE_H__

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
 *      dal_maple_rate_init
 * Description:
 *      Initial the rate module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32
dal_maple_rate_init(uint32 unit);

/* Function Name:
 *      dal_maple_rate_igrBandwidthCtrlEnable_get
 * Description:
 *      Get the ingress bandwidth control status.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of ingress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The status of ingress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_maple_rate_igrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_maple_rate_igrBandwidthCtrlEnable_set
 * Description:
 *      Set the ingress bandwidth control status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of ingress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      The status of ingress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_maple_rate_igrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_maple_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get the ingress bandwidth control rate.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - ingress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity in RTL8380 is 16Kbps.
 */
extern int32
dal_maple_rate_igrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_maple_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set the ingress bandwidth control rate.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - ingress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_RATE     - Invalid input rate
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity in RTL8380 is 16Kbps.
 */
extern int32
dal_maple_rate_igrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_maple_rate_bandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of  bandwidth control includes IFG or not.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1) bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      2) The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_maple_rate_bandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include);

/* Function Name:
 *      dal_maple_rate_bandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of bandwidth control includes IFG or not.
 * Input:
 *      unit        - unit id
 *      ifg_include - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      1) bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      2) The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_maple_rate_bandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include);

/* Function Name:
 *      dal_maple_rate_igrBandwidthCtrlBypass_get
 * Description:
 *      Get the status of bypass ingress bandwidth control for specified frame type.
 * Input:
 *      unit       - unit id
 *      bypassType - bypass type
 * Output:
 *      pEnable    - pointer to enable status of bypass ingress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
extern int32
dal_maple_rate_igrBandwidthCtrlBypass_get(uint32 unit, rtk_rate_igr_bypassType_t bypassType, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_maple_rate_igrBandwidthCtrlBypass_set
 * Description:
 *      Set the status of bypass ingress bandwidth control for specified packet type.
 * Input:
 *      unit       - unit id
 *      byasssType - bypass type
 *      enable     - status of bypass ingress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      None.
 */
extern int32
dal_maple_rate_igrBandwidthCtrlBypass_set(uint32 unit, rtk_rate_igr_bypassType_t bypassType, rtk_enable_t enable);

/* Function Name:
 *      dal_maple_rate_egrBandwidthCtrlEnable_get
 * Description:
 *      Get the egress bandwidth control status.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of egress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The status of egress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_maple_rate_egrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_maple_rate_egrBandwidthCtrlEnable_set
 * Description:
 *      Set the egress bandwidth control status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of egress bandwidth control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      The status of egress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_maple_rate_egrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_maple_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8380 is 16Kbps.
 */
extern int32
dal_maple_rate_egrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_maple_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_RATE     - Invalid input rate
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity in RTL8380 is 16Kbps.
 */
extern int32
dal_maple_rate_egrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_maple_rate_egrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of egress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable);

/* Function Name:
 *      dal_maple_rate_egrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of egress bandwidth control on specified queue.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable);

/* Function Name:
 *      dal_maple_rate_egrQueueBwCtrlRate_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 * Output:
 *      pRate - pointer to rate of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity is 16Kbps.
 */
extern int32
dal_maple_rate_egrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate);

/* Function Name:
 *      dal_maple_rate_egrQueueBwCtrlRate_set
 * Description:
 *      Set rate of egress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 *      rate  - rate of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_RATE     - Invalid input rate
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity is 16Kbps.
 */
extern int32
dal_maple_rate_egrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate);

/* Function Name:
 *      dal_maple_rate_egrQueueFixedBandwidthEnable_get
 * Description:
 *      Get enable status of fixed bandwidth ability on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pEnable - pointer to enable status of fixed bandwidth ability 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrQueueFixedBandwidthEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable);

/* Function Name:
 *      dal_maple_rate_egrQueueFixedBandwidthEnable_set
 * Description:
 *      Set enable status of fixed bandwidth ability on specified queue.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of fixed bandwidth ability 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrQueueFixedBandwidthEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable);

/* Function Name:
 *      dal_maple_rate_stormControlEnable_get
 * Description:
 *      Get the storm control ability.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pEnable    - storm control ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The storm group types are as following:
 *      - STORM_GROUP_UNKNOWN_UNICAST
 *      - STORM_GROUP_UNKNOWN_MULTICAST
 *      - STORM_GROUP_MULTICAST
 *      - STORM_GROUP_BROADCAST
 *      - STORM_GROUP_UNICAST
 */
extern int32
dal_maple_rate_stormControlEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            *pEnable);

/* Function Name:
 *      dal_maple_rate_stormControlEnable_set
 * Description:
 *      Set the storm control ability.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      enable     - storm control ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      The storm group types are as following:
 *      - STORM_GROUP_UNKNOWN_UNICAST
 *      - STORM_GROUP_UNKNOWN_MULTICAST
 *      - STORM_GROUP_MULTICAST
 *      - STORM_GROUP_BROADCAST
 *      - STORM_GROUP_UNICAST
 */
extern int32
dal_maple_rate_stormControlEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            enable);

/* Function Name:
 *      dal_maple_rate_stormControlRate_get
 * Description:
 *      Get the storm control rate.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pRate      - storm control rate (packet-per-second).
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The storm group types are as following:
 *      - STORM_GROUP_UNKNOWN_UNICAST
 *      - STORM_GROUP_UNKNOWN_MULTICAST
 *      - STORM_GROUP_MULTICAST
 *      - STORM_GROUP_BROADCAST
 *      - STORM_GROUP_UNICAST
 */
extern int32
dal_maple_rate_stormControlRate_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pRate);

/* Function Name:
 *      dal_maple_rate_stormControlRate_set
 * Description:
 *      Set the storm control rate.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      rate       - storm control rate (packet-per-second).
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 *      RT_ERR_RATE     - Invalid input bandwidth
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_maple_rate_stormControlRate_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  rate);

/* Function Name:
 *      dal_maple_rate_stormControlRateMode_get
 * Description:
 *      Get rate counting mode of storm control.
 * Input:
 *      unit       - unit id
 * Output:
 *      pRate_mode - pointer to rate counting mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *    1) The rate mode are as following:
 *       - BASED_ON_PKT
 *       - BASED_ON_BYTE
 */
extern int32
dal_maple_rate_stormControlRateMode_get(
    uint32                      unit,
    rtk_rate_storm_rateMode_t   *pRate_mode);

/* Function Name:
 *      dal_maple_rate_stormControlRateMode_set
 * Description:
 *      Set rate counting mode of storm control.
 * Input:
 *      unit      - unit id
 *      rate_mode - Rate counting mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *    1) The rate mode are as following:
 *       - BASED_ON_PKT
 *       - BASED_ON_BYTE
 */
extern int32
dal_maple_rate_stormControlRateMode_set(
    uint32                      unit,
    rtk_rate_storm_rateMode_t   rate_mode);

/* Function Name:
 *      dal_maple_rate_stormCtrlExceed_get
 * Description:
 *      Get exceed status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pIsExceed  - pointer to exceed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      Exceed status is as following
 *      - TRUE      - storm rate is more than configured rate.
 *      - FALSE     - storm rate is never over then configured rate.
 */
extern int32
dal_maple_rate_stormControlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pIsExceed);

/* Function Name:
 *      dal_maple_rate_stormControlExceed_reset
 * Description:
 *      Reset exceed status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_maple_rate_stormControlExceed_reset(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type);

/* Function Name:
 *      dal_maple_rate_stormControlBurstSize_get
 * Description:
 *      Get burst size of storm control
 * Input:
 *      unit        - unit id
 *      storm_type  - storm group type
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The storm group types are as following:
 *      - STORM_GROUP_UNKNOWN_UNICAST
 *      - STORM_GROUP_UNKNOWN_MULTICAST
 *      - STORM_GROUP_MULTICAST
 *      - STORM_GROUP_BROADCAST
 *      - STORM_GROUP_UNICAST
 */
extern int32
dal_maple_rate_stormControlBurstSize_get(
    uint32                  unit,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pBurst_size);

/* Function Name:
 *      dal_maple_rate_stormControlBurstSize_set
 * Description:
 *      Set burst size of storm control
 * Input:
 *      unit       - unit id
 *      storm_type - storm group type
 *      burst_size - burst size
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The storm group types are as following:
 *      - STORM_GROUP_UNKNOWN_UNICAST
 *      - STORM_GROUP_UNKNOWN_MULTICAST
 *      - STORM_GROUP_MULTICAST
 *      - STORM_GROUP_BROADCAST
 *      - STORM_GROUP_UNICAST
 */
extern int32
dal_maple_rate_stormControlBurstSize_set(
    uint32                  unit,
    rtk_rate_storm_group_t  storm_type,
    uint32                  burst_size);

/* Function Name:
 *      dal_maple_rate_stormControlTypeSel_get
 * Description:
 *      Get the storm control type.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pStorm_sel - storm selection
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The API is only supported in unicast and multicast, the storm group types are as following:
 *      - STORM_GROUP_UNICAST
 *      - STORM_GROUP_MULTICAST
 *
 *      The storm selection are as following:
 *      - STORM_SEL_UNKNOWN
 *      - STORM_SEL_UNKNOWN_AND_KNOWN
 */
extern int32
dal_maple_rate_stormControlTypeSel_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_rate_storm_sel_t    *pStorm_sel);

/* Function Name:
 *      dal_maple_rate_stormControlTypeSel_set
 * Description:
 *      Set the storm control type.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      storm_sel  - storm selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      The API is only supported in unicast and multicast, the storm group types are as following:
 *      - STORM_GROUP_UNICAST
 *      - STORM_GROUP_MULTICAST
 *
 *      The storm selection are as following:
 *      - STORM_SEL_ONLY_UNKNOWN
 *      - STORM_SEL_BOTH_UNKNOWN_KNOWN
 */
extern int32
dal_maple_rate_stormControlTypeSel_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_rate_storm_sel_t    storm_sel);

/* Function Name:
 *      dal_maple_rate_stormControlBypass_get
 * Description:
 *      Get the status of bypass storm control for specified packet type.
 * Input:
 *      unit       - unit id
 *      bypassType - bypass type
 * Output:
 *      pEnable    - pointer to enable status of bypass storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      None.
 */
extern int32
dal_maple_rate_stormControlBypass_get(uint32 unit, rtk_rate_storm_bypassType_t bypassType, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_maple_rate_stormControlBypass_set
 * Description:
 *      Set the status of bypass storm control for specified packet type.
 * Input:
 *      unit       - unit id
 *      bypassType - bypass type
 *      enable     - status of bypass storm control
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      None.
 */
extern int32
dal_maple_rate_stormControlBypass_set(uint32 unit, rtk_rate_storm_bypassType_t bypassType, rtk_enable_t enable);

/* Function Name:
 *      dal_maple_rate_stormControlIncludeIfg_get
 * Description:
 *      Get enable status of includes IFG for storm control.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - pointer to enable status of includes IFG
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_stormControlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include);

/* Function Name:
 *      dal_maple_rate_stormControlIncludeIfg_set
 * Description:
 *      Set enable status of includes IFG for storm control.
 * Input:
 *      unit        - unit id
 *      ifg_include - enable status of includes IFG
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
extern int32
dal_maple_rate_stormControlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include);

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of ingress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 * Output:
 *      pEnable - Pointer to enable status of ingress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable); 

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of ingress bandwidth control on specified queue.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of ingress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable);

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlRate_get
 * Description:
 *      Get rate of ingress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 * Output:
 *      pRate - pointer to rate of ingress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity is 16Kbps.
 */
extern int32
dal_maple_rate_igrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate);

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlRate_set
 * Description:
 *      Set rate of ingress bandwidth control on specified queue.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 *      rate  - rate of ingress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_RATE     - Invalid input rate
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity is 16Kbps.
 */
extern int32
dal_maple_rate_igrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate);

/* Function Name:
 *      dal_maple_rate_portIgrBandwidthCtrlExceed_get
 * Description:
 *      Get exceed status of ingress bandwidth control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pIsExceed  - pointer to exceed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      Exceed status is as following
 *      - TRUE      - ingress bandwidth rate is more than configured rate.
 *      - FALSE     -ingress bandwidth rate is never over then configured rate.
 */
extern int32 
dal_maple_rate_portIgrBandwidthCtrlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  *pIsExceed);

/* Function Name:
 *      dal_maple_rate_portIgrBandwidthCtrlExceed_reset
 * Description:
 *      Reset exceed status of ingress bandwidth on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32 
dal_maple_rate_portIgrBandwidthCtrlExceed_reset(
    uint32                  unit,
    rtk_port_t              port);

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlExceed_get
 * Description:
 *      Get exceed status of ingress bandwidth control on specified queue.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      queue    - queue id
 * Output:
 *      pIsExceed  - pointer to exceed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      Exceed status is as following
 *      - TRUE      - ingress queue bandwidth rate is more than configured rate.
 *      - FALSE     -ingress queue bandwidth rate is never over then configured rate.
 */
extern int32 
dal_maple_rate_igrQueueBwCtrlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qid_t               queue,
    uint32                  *pIsExceed);

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlExceed_reset
 * Description:
 *      Reset exceed status of ingress bandwidth on specified queue.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      queue   - queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern int32 
dal_maple_rate_igrQueueBwCtrlExceed_reset(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qid_t               queue);

/* Function Name:
 *      dal_maple_rate_igrPortBwCtrlBurstSize_get
 * Description:
 *      Get burst size of port ingress bandwidth
 * Input:
 *      unit        - unit id
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrPortBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size);

/* Function Name:
 *      dal_maple_rate_igrPortBwCtrlBurstSize_set
 * Description:
 *      Set burst size of port ingress bandwidth 
 * Input:
 *      unit        - unit id
 *      burst_size - burst size
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrPortBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size);

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlBurstSize_get
 * Description:
 *      Get burst size of queue ingress bandwidth
 * Input:
 *      unit        - unit id
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrQueueBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size);

/* Function Name:
 *      dal_maple_rate_igrQueueBwCtrlBurstSize_set
 * Description:
 *      Set burst size of queue ingress bandwidth 
 * Input:
 *      unit        - unit id
 *      burst_size - burst size
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrQueueBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size);

/* Function Name:
 *      dal_maple_rate_egrPortBwCtrlBurstSize_get
 * Description:
 *      Get burst size of port egress bandwidth
 * Input:
 *      unit        - unit id
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrPortBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size);

/* Function Name:
 *      dal_maple_rate_egrPortBwCtrlBurstSize_set
 * Description:
 *      Set burst size of port egress bandwidth 
 * Input:
 *      unit        - unit id
 *      burst_size - burst size
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrPortBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size);

/* Function Name:
 *      dal_maple_rate_egrQueueBwCtrlBurstSize_get
 * Description:
 *      Get burst size of queue egress bandwidth
 * Input:
 *      unit        - unit id
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrQueueBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size);

/* Function Name:
 *      dal_maple_rate_egrQueueBwCtrlBurstSize_set
 * Description:
 *      Set burst size of queue egress bandwidth 
 * Input:
 *      unit        - unit id
 *      burst_size - burst size
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      None
 */
extern int32
dal_maple_rate_egrQueueBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size);


/* Function Name:
 *      dal_maple_rate_cpuEgrBandwidthCtrlRateMode_get
 * Description:
 *      Get rate counting mode of CPU port egress bandwidth control.
 * Input:
 *      unit       - unit id
 * Output:
 *      pMode      - pointer to rate counting mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Note:
 *      The rate mode are as following:
 *      - RATE_MODE_BYTE
 *      - RATE_MODE_PKT
 */
extern int32
dal_maple_rate_cpuEgrBandwidthCtrlRateMode_get(uint32 unit, rtk_rate_rateMode_t *pMode);

/* Function Name:
 *      dal_maple_rate_cpuEgrBandwidthCtrlRateMode_set
 * Description:
 *      Set rate counting mode of CPU port egress bandwidth control.
 * Input:
 *      unit      - unit id
 *      mode      - Rate counting mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The rate mode are as following:
 *      - RATE_MODE_BYTE
 *      - RATE_MODE_PKT
 */
extern int32
dal_maple_rate_cpuEgrBandwidthCtrlRateMode_set(uint32 unit, rtk_rate_rateMode_t mode);

#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
/* Function Name:
 *      dal_maple_rate_igrBandwidthFlowctrlEnable_get
 * Description:
 *      Get enable status of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of flowcontrol for ingress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrBandwidthFlowctrlEnable_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        *pEnable); 

/* Function Name:
 *      dal_maple_rate_igrBandwidthFlowctrlEnable_set
 * Description:
 *      Set enable status of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of flowcontrol for ingress bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_maple_rate_igrBandwidthFlowctrlEnable_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        enable); 
#endif


#endif /* __DAL_MAPLE_RATE_H__ */
