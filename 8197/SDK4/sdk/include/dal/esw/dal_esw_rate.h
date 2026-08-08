/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 40320 $
 * $Date: 2013-06-19 16:42:11 +0800 (Wed, 19 Jun 2013) $
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

#ifndef __DAL_ESW_RATE_H__
#define __DAL_ESW_RATE_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
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
 *      dal_esw_rate_init
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
dal_esw_rate_init(uint32 unit);

/* Module Name    : Rate                                            */
/* Sub-module Name: Configuration of ingress port bandwidth control */

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlEnable_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_rate_igrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlEnable_set
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
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *    The status of ingress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_rate_igrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlRate_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_esw_rate_igrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlRate_set
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
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_esw_rate_igrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of ingress bandwidth control includes IFG or not on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pIfg_include - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIfg_include);


/* Function Name:
 *      dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of ingress bandwidth control includes IFG or not on specified port.
 * Input:
 *      unit        - unit id
 *      port         - port id
 *      ifg_include - include IFG or not
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *      1. Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *
 *      2. The status of ifg_include:
 *         - DISABLED
 *         - ENABLED
 */
extern int32
dal_esw_rate_portIgrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_port_t port, rtk_enable_t ifg_include);


/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlEnable_get
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_igrBandwidthFlowctrlEnable_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        *pEnable);

/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlEnable_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_rate_igrBandwidthFlowctrlEnable_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        enable);

/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlThresh_get
 * Description:
 *      Get threshold of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to threshold of flowcontrol
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_igrBandwidthFlowctrlThresh_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh);

/* Function Name:
 *      dal_esw_rate_igrBandwidthFlowctrlThresh_set
 * Description:
 *      Set threshold of flowcontrol for ingress bandwidth control on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - threshold of flowcontrol
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_igrBandwidthFlowctrlThresh_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh);

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlFPEntry_get
 * Description:
 *      Get Fast Path entry for ingress bandwidth control on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      fpIndex  - index for fast path entry
 * Output:
 *      pFpEntry - Pointer to fast path entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_igrBandwidthCtrlFPEntry_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t *pFpEntry);

/* Function Name:
 *      dal_esw_rate_igrBandwidthCtrlFPEntry_set
 * Description:
 *      Set Fast Path entry for ingress bandwidth control on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      fpIndex  - index for fast path entry
 *      pFpEntry - Fast path entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_igrBandwidthCtrlFPEntry_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t *pFpEntry);

/* Module Name    : Rate                                           */
/* Sub-module Name: Configuration of egress port bandwidth control */

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlEnable_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_rate_egrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlEnable_set
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
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - Invalid input parameter
 * Note:
 *    The status of egress bandwidth control is as following:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_rate_egrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlRate_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_esw_rate_egrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate);

/* Function Name:
 *      dal_esw_rate_egrBandwidthCtrlRate_set
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
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8389/RTL8329 is 16Kbps.
 */
extern int32
dal_esw_rate_egrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate);

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get status of egress storm control includes IFG
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pIfg_include - pointer to enable status of egress storm control includes IFG
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pIfg_include);

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set status of egress storm control includes IFG
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      ifg_include - enable status of egress storm control includes IFG
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_rate_portEgrBandwidthCtrlIncludeIfg_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    ifg_include);

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlBurstSize_get
 * Description:
 *      Get the egress bandwidth control burst size.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pBurst_size - egress bandwidth control burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is byte.
 *    2) The maximum value of burst_size is 0xFFFF.
 */
extern int32
dal_esw_rate_portEgrBandwidthCtrlBurstSize_get(uint32 unit, rtk_port_t port, uint32 *pBurst_size);

/* Function Name:
 *      dal_esw_rate_portEgrBandwidthCtrlBurstSize_set
 * Description:
 *      Set the egress bandwidth control burst size.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      burst_size - egress bandwidth control burst size
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is byte.
 *    2) The maximum value of burst_size is 0xFFFF.
 */
extern int32
dal_esw_rate_portEgrBandwidthCtrlBurstSize_set(uint32 unit, rtk_port_t port, uint32 burst_size);

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of egress bandwidth control on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_egrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable);

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_rate_egrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable);

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlRate_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8328M is 16Kbps.
 */
extern int32
dal_esw_rate_egrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate);

/* Function Name:
 *      dal_esw_rate_egrQueueBwCtrlRate_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_RATE             - Invalid input rate
 * Note:
 *    The actual rate is "rate * chip granularity".
 *    The unit of granularity in RTL8328M is 16Kbps.
 */
extern int32
dal_esw_rate_egrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate);

/* Function Name:
 *      dal_esw_rate_egrPortQueueBwCtrlBurstSize_get
 * Description:
 *      Get burst size of egress bandwidth control on specified queue.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      queue       - queue id
 * Output:
 *      pBurst_size - pointer to burst size of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is KBytes.
 *    2) The maximum value of burst_size is 63. (mean 63 KBytes)
 */
extern int32
dal_esw_rate_egrPortQueueBwCtrlBurstSize_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pBurst_size);

/* Function Name:
 *      dal_esw_rate_egrPortQueueBwCtrlBurstSize_set
 * Description:
 *      Set burst size of egress bandwidth control on specified queue.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      queue      - queue id
 *      burst_size - burst size of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is KBytes.
 *    2) The maximum value of burst_size is 63. (mean 63 KBytes)
 */
extern int32
dal_esw_rate_egrPortQueueBwCtrlBurstSize_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      burst_size);

/* Module Name    : Rate                           */
/* Sub-module Name: Configuration of storm control */

/* Function Name:
 *      dal_esw_rate_stormControlRate_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_esw_rate_stormControlRate_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pRate);

/* Function Name:
 *      dal_esw_rate_stormControlRate_set
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
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input bandwidth
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_esw_rate_stormControlRate_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  rate);

/* Function Name:
 *      dal_esw_rate_stormControlEnable_get
 * Description:
 *      Get enable status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pEnable    - pointer to enable status of storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_esw_rate_stormControlEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            *pEnable);

/* Function Name:
 *      dal_esw_rate_stormControlEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      enable     - enable status of storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_esw_rate_stormControlEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            enable);

/* Function Name:
 *      dal_esw_rate_stormControlRateMode_get
 * Description:
 *      Get rate counting mode of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pRate_mode - pointer to rate counting mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *
 *    The rate mode are as following:
 *    - BASED_ON_PKT
 *    - BASED_ON_BYTE
 */
extern int32
dal_esw_rate_stormControlRateMode_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   *pRate_mode);

/* Function Name:
 *      dal_esw_rate_stormControlRateMode_set
 * Description:
 *      Set rate counting mode of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      rate_mode  - Rate counting mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 *
 *    The rate mode are as following:
 *    - BASED_ON_PKT
 *    - BASED_ON_BYTE
 */
extern int32
dal_esw_rate_stormControlRateMode_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   rate_mode);

/* Function Name:
 *      dal_esw_rate_stormControlBurstSize_get
 * Description:
 *      Get burst size of storm control on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      storm_type  - storm group type
 * Output:
 *      pBurst_rate - pointer to burst rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_esw_rate_stormControlBurstSize_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pBurst_rate);

/* Function Name:
 *      dal_esw_rate_stormControlBurstSize_set
 * Description:
 *      Set burst size of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      burst_rate - burst rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_INPUT                - invalid input parameter
 * Note:
 *    The storm group types are as following:
 *    - STORM_GROUP_UNKNOWN_UNICAST
 *    - STORM_GROUP_UNKNOWN_MULTICAST
 *    - STORM_GROUP_MULTICAST
 *    - STORM_GROUP_BROADCAST
 */
extern int32
dal_esw_rate_stormControlBurstSize_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  burst_rate);

/* Function Name:
 *      dal_esw_rate_stormControlIncludeIfg_get
 * Description:
 *      Get enable status of includes IFG for storm control.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - pointer to enable status of includes IFG
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_rate_stormControlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include);

/* Function Name:
 *      dal_esw_rate_stormControlIncludeIfg_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_rate_stormControlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include);

/* Function Name:
 *      dal_esw_rate_stormControlExceed_get
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
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Exceed status is as following
 *      - TRUE      - storm rate is more than configured rate.
 *      - FALSE     - storm rate is never over then configured rate.
 */
extern int32
dal_esw_rate_stormControlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pIsExceed);

/* Function Name:
 *      dal_esw_rate_stormControlExceed_reset
 * Description:
 *      Clear exceed status of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_PORT_ID              - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP    - Unknown storm group
 * Note:
 *      None
 */
extern int32
dal_esw_rate_stormControlExceed_reset(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type);

/* Function Name:
 *      dal_esw_rate_stormControlRefreshMode_get
 * Description:
 *      Get refresh mode of storm control.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to refresh mode of storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The rate mode are as following:
 *      - BASED_ON_PKT
 *      - BASED_ON_BYTE
 */
extern int32
dal_esw_rate_stormControlRefreshMode_get(uint32 unit, rtk_rate_storm_rateMode_t *pMode);

/* Function Name:
 *      dal_esw_rate_stormControlRefreshMode_set
 * Description:
 *      Set refresh mode of storm control.
 * Input:
 *      unit - unit id
 *      mode - refresh mode of storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The rate mode are as following:
 *      - BASED_ON_PKT
 *      - BASED_ON_BYTE
 */
extern int32
dal_esw_rate_stormControlRefreshMode_set(uint32 unit, rtk_rate_storm_rateMode_t mode);

#endif /* __DAL_ESW_RATE_H__ */
