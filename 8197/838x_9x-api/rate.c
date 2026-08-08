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
 * $Revision: 41929 $
 * $Date: 2013-08-09 10:57:40 +0800 (Fri, 09 Aug 2013) $
 *
 * Purpose : Definition of Port Bandwidth Control and Storm Control API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of Ingress Port Bandwidth Control [Ingress Rate Limit]
 *           (2) Configuration of Egress  Port Bandwidth Control [Egress  Rate Limit]
 *           (3) Configuration of Storm Control
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
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
 *      rtk_rate_init
 * Description:
 *      Initial the rate module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize rate module before calling any rate APIs.
 */
int32
rtk_rate_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_init(unit);
} /* end of rtk_rate_init */

/* Module Name    : Rate                                            */
/* Sub-module Name: Configuration of ingress port bandwidth control */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of ingress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_rate_igrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->rate_igrBandwidthCtrlEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_rate_igrBandwidthCtrlEnable_get */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlEnable_set
 * Description:
 *      Set the ingress bandwidth control status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of ingress bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of ingress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_rate_igrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->rate_igrBandwidthCtrlEnable_set(unit, port, enable);
#endif
} /* end of rtk_rate_igrBandwidthCtrlEnable_set */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity is 16Kbps in 8389, 8328, 8390 and 8380.
 *      (3) The valid range of rate is 0 ~ 0xFFFF in 8389 and 8328.
 */
int32
rtk_rate_igrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlRate_get(unit, phy_port, pRate);
    }
#else
    return RT_MAPPER(unit)->rate_igrBandwidthCtrlRate_get(unit, port, pRate);
#endif
} /* end of rtk_rate_igrBandwidthCtrlRate_get */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set the ingress bandwidth control rate.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - ingress bandwidth control rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_RATE     - Invalid input rate
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity is 16Kbps in 8389, 8328, 8390 and 8380.
 *      (3) The valid range of rate is 0 ~ 0xFFFF in 8389 and 8328.
 */
int32
rtk_rate_igrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlRate_set(unit, phy_port, rate);
    }
#else
    return RT_MAPPER(unit)->rate_igrBandwidthCtrlRate_set(unit, port, rate);
#endif
} /* end of rtk_rate_igrBandwidthCtrlRate_set */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_igrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlIncludeIfg_get(unit, pIfg_include);
} /* end of rtk_rate_igrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of ingress bandwidth control includes IFG or not.
 * Input:
 *      unit        - unit id
 *      ifg_include - include IFG or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_igrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlIncludeIfg_set(unit, ifg_include);
} /* end of rtk_rate_igrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlBypass_get
 * Description:
 *      Get the status of bypass ingress bandwidth control for specified packet type.
 * Input:
 *      unit       - unit id
 *      bypassType - bypass type
 * Output:
 *      pEnable    - pointer to enable status of bypass ingress bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      If a certain packet type is selected to bypass the ingress bandwidth control, the packet will
 *      not be blocked by the ingress bandwidth control and will not consume the bandwidth.
 */
int32
rtk_rate_igrBandwidthCtrlBypass_get(uint32 unit, rtk_rate_igr_bypassType_t bypassType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlBypass_get(unit, bypassType, pEnable);
} /* end of rtk_rate_igrBandwidthCtrlBypass_get */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlBypass_set
 * Description:
 *      Set the status of bypass ingress bandwidth control for specified packet type.
 * Input:
 *      unit       - unit id
 *      bypassType - bypass type
 *      enable     - status of bypass ingress bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      If a certain packet type is selected to bypass the ingress bandwidth control, the packet will
 *      not be blocked by the ingress bandwidth control and will not consume the bandwidth.
 */
int32
rtk_rate_igrBandwidthCtrlBypass_set(uint32 unit, rtk_rate_igr_bypassType_t bypassType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlBypass_set(unit, bypassType, enable);
} /* end of rtk_rate_igrBandwidthCtrlBypass_set */

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlIncludeIfg_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_portIgrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_port_t port, rtk_enable_t *pIfg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portIgrBandwidthCtrlIncludeIfg_get(unit, port, pIfg_include);
} /* end of rtk_rate_portIgrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of ingress bandwidth control includes IFG or not on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      ifg_include - include IFG or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) Ingress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_port_t port, rtk_enable_t ifg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portIgrBandwidthCtrlIncludeIfg_set(unit, port, ifg_include);
} /* end of rtk_rate_portIgrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      rtk_rate_igrBandwidthFlowctrlEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_igrBandwidthFlowctrlEnable_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthFlowctrlEnable_get(unit, port, pEnable);
} /* end of rtk_rate_igrBandwidthFlowctrlEnable_get */

/* Function Name:
 *      rtk_rate_igrBandwidthFlowctrlEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_igrBandwidthFlowctrlEnable_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_enable_t        enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthFlowctrlEnable_set(unit, port, enable);
} /* end of rtk_rate_igrBandwidthFlowctrlEnable_set */

/* Function Name:
 *      rtk_rate_igrBandwidthFlowctrlThresh_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The actual rate is "pThresh * chip granularity".
 *      (2) The unit of granularity is 1KBytes in 8328.
 *      (3) The pThresh->FC_On and pThresh->FC_Off is up to 0xFF in 8328.
 */
int32
rtk_rate_igrBandwidthFlowctrlThresh_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthFlowctrlThresh_get(unit, port, pThresh);
} /* end of rtk_rate_igrBandwidthFlowctrlThresh_get */

/* Function Name:
 *      rtk_rate_igrBandwidthFlowctrlThresh_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The actual rate is "pThresh * chip granularity".
 *      (2) The unit of granularity is 1KBytes in 8328.
 *      (3) The Thresh->FC_On and pThresh->FC_Off is up to 0xFF in 8328.
 */
int32
rtk_rate_igrBandwidthFlowctrlThresh_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_rate_thresh_t   *pThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthFlowctrlThresh_set(unit, port, pThresh);
} /* end of rtk_rate_igrBandwidthFlowctrlThresh_set */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlFPEntry_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The fpIndex valid range is 0 ~ 2
 *      (2) The pFpEntry can combine by following items
 *          - inner or outer vlan id
 *          - inner priority value
 *          - outrt priority value
 *          - dscp value
 */
int32
rtk_rate_igrBandwidthCtrlFPEntry_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t  *pFpEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlFPEntry_get(unit, port, fpIndex, pFpEntry);
} /* end of rtk_rate_igrBandwidthCtrlFPEntry_get */

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlFPEntry_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The fpIndex valid range is 0 ~ 2
 *      (2) The pFpEntry can combine by following items
 *          - inner or outer vlan id
 *          - inner priority value
 *          - outrt priority value
 *          - dscp value
 */
int32
rtk_rate_igrBandwidthCtrlFPEntry_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  fpIndex,
    rtk_rate_igr_fpEntry_t  *pFpEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthCtrlFPEntry_set(unit, port, fpIndex, pFpEntry);
} /* end of rtk_rate_igrBandwidthCtrlFPEntry_set */

/* Function Name:
 *      rtk_rate_igrBandwidthLowThresh_get
 * Description:
 *      Get the flow control turn off threshold of ingress bandwidth.
 * Input:
 *      unit         - unit id
 * Output:
 *      pLowThresh   - pointer to flow control turn ON/OFF low threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The threshold unit is 128 Byte.
 */
int32
rtk_rate_igrBandwidthLowThresh_get(uint32 unit, uint32 *pLowThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthLowThresh_get(unit, pLowThresh);
} /* end of rtk_rate_igrBandwidthLowThresh_get */

/* Function Name:
 *      rtk__rate_igrBandwidthLowThresh_set
 * Description:
 *      Set the flow control turn ON/OFF low threshold of ingress bandwidth.
 * Input:
 *      unit        - unit id
 *      lowThresh   - flow control turn ON/OFF low threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      The threshold unit is 128 Byte.
 *      After setting the API, please adjust the low threshold by rtk_rate_igrBwCtrlBurstSize_set.
 */
int32
rtk_rate_igrBandwidthLowThresh_set(uint32 unit, uint32 lowThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthLowThresh_set(unit, lowThresh);
} /* end of rtk_rate_igrBandwidthLowThresh_set */

/* Function Name:
 *      rtk_rate_portIgrBandwidthHighThresh_get
 * Description:
 *      Get the flow control turn ON/OFF high threshold of ingress bandwidth for specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pHighThresh - pointer to flow control turn ON/OFF high threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The threshold unit is 128 Byte.
 */
int32
rtk_rate_portIgrBandwidthHighThresh_get(uint32 unit, rtk_port_t port, uint32 *pHighThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portIgrBandwidthHighThresh_get(unit, port, pHighThresh);
} /* end of rtk_rate_portIgrBandwidthHighThresh_get */

/* Function Name:
 *      rtk_rate_portIgrBandwidthHighThresh_set
 * Description:
 *      Set the flow control turn ON/OFF high threshold of ingress bandwidth for specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      highThresh - flow control turn ON/OFF high threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      The threshold unit is 128 Byte.
 *      The (minimum, maximum) of highThresh setting range is (22, 65535). 
 */
int32
rtk_rate_portIgrBandwidthHighThresh_set(uint32 unit, rtk_port_t port, uint32 highThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portIgrBandwidthHighThresh_set(unit, port, highThresh);
} /* end of rtk_rate_portIgrBandwidthHighThresh_set */

/* Function Name:
 *      rtk_rate_igrBwCtrlBurstSize_get
 * Description:
 *      Get burst size of ingress bandwidth
 * Input:
 *      unit        - unit id
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The burst size unit is 128 Byte.
 */
int32
rtk_rate_igrBwCtrlBurstSize_get(uint32 unit, uint32 *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBwCtrlBurstSize_get(unit, pBurst_size);
}

/* Function Name:
 *      rtk_rate_igrBwCtrlBurstSize_set
 * Description:
 *      Set burst size of ingress bandwidth 
 * Input:
 *      unit        - unit id
 *      burst_size  - burst size
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Applicable:
 *      8390
 * Note:
 *      The burst size unit is 128 Byte.
 *      The (minimum, maximum) burst size setting range is (22, 65535). 
 *      After setting the API, please adjust the low threshold by rtk_rate_igrBandwidthLowThresh_set.
 */
int32
rtk_rate_igrBwCtrlBurstSize_set(uint32 unit, uint32 burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBwCtrlBurstSize_set(unit, burst_size);
}

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlExceed_get
 * Description:
 *      Get exceed status of ingress bandwidth control on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pIsExceed - pointer to exceed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Exceed status is as following
 *      - TRUE      - flow control high on threshold ever exceeds.
 *      - FALSE     - flow control high on threshold never exceeds.
 */
int32
rtk_rate_portIgrBandwidthCtrlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  *pIsExceed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portIgrBandwidthCtrlExceed_get(unit, port, pIsExceed);
} /* end of rtk_rate_portIgrBandwidthCtrlExceed_get */

/* Function Name:
 *      rtk_rate_portIgrBandwidthCtrlExceed_reset
 * Description:
 *      Reset exceed status of ingress bandwidth control on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_portIgrBandwidthCtrlExceed_reset(
    uint32                  unit,
    rtk_port_t              port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portIgrBandwidthCtrlExceed_reset(unit, port);
} /* end of rtk_rate_portIgrBandwidthCtrlExceed_reset */

/* Module Name    : Rate                                           */
/* Sub-module Name: Configuration of egress port bandwidth control */

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of egress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_rate_egrBandwidthCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrBandwidthCtrlEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->rate_egrBandwidthCtrlEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_rate_egrBandwidthCtrlEnable_get */

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlEnable_set
 * Description:
 *      Set the egress bandwidth control status.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of egress bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of egress bandwidth control is as following:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_rate_egrBandwidthCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrBandwidthCtrlEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->rate_egrBandwidthCtrlEnable_set(unit, port, enable);
#endif
} /* end of rtk_rate_egrBandwidthCtrlEnable_set */

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity is 16Kbps in 8389, 8328 and 8380.
 *      (3) The valid range of rate is 0 ~ 0xFFFF in 8389 and 8328.
 *      (4) The unit of granularity is 16Kbps in 8390 normal port.
 *      (5) The unit of granularity is 16Kbps or 1pps in 8390 CPU port depend on mode set by 'rtk_rate_cpuEgrBandwidthCtrlRateMode_set'.
 */
int32
rtk_rate_egrBandwidthCtrlRate_get(uint32 unit, rtk_port_t port, uint32 *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrBandwidthCtrlRate_get(unit, phy_port, pRate);
    }
#else
    return RT_MAPPER(unit)->rate_egrBandwidthCtrlRate_get(unit, port, pRate);
#endif
} /* end of rtk_rate_egrBandwidthCtrlRate_get */

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate.
 * Input:
 *      unit - unit id
 *      port - port id
 *      rate - egress bandwidth control rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_RATE     - Invalid input rate
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity is 16Kbps in 8389, 8328 and 8380.
 *      (3) The valid range of rate is 0 ~ 0xFFFF in 8389 and 8328.
 *      (4) The unit of granularity is 16Kbps in 8390 normal port.
 *      (5) The unit of granularity is 16Kbps or 1pps in 8390 CPU port depend on mode set by 'rtk_rate_cpuEgrBandwidthCtrlRateMode_set'.
 */
int32
rtk_rate_egrBandwidthCtrlRate_set(uint32 unit, rtk_port_t port, uint32 rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrBandwidthCtrlRate_set(unit, phy_port, rate);
    }
#else
    return RT_MAPPER(unit)->rate_egrBandwidthCtrlRate_set(unit, port, rate);
#endif
} /* end of rtk_rate_egrBandwidthCtrlRate_set */

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlIncludeIfg_get
 * Description:
 *      Get the status of egress bandwidth control includes IFG or not.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - include IFG or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_egrBandwidthCtrlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_egrBandwidthCtrlIncludeIfg_get(unit, pIfg_include);
} /* end of rtk_rate_egrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlIncludeIfg_set
 * Description:
 *      Set the status of egress bandwidth control includes IFG or not.
 * Input:
 *      unit        - unit id
 *      ifg_include - include IFG or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_egrBandwidthCtrlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_egrBandwidthCtrlIncludeIfg_set(unit, ifg_include);
} /* end of rtk_rate_egrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      rtk_rate_cpuEgrBandwidthCtrlRateMode_get
 * Description:
 *      Get rate counting mode of CPU port egress bandwidth control.
 * Input:
 *      unit       - unit id
 * Output:
 *      pRate_mode - pointer to rate counting mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The rate mode are as following:
 *      - RATE_MODE_BYTE
 *      - RATE_MODE_PKT
 */
int32
rtk_rate_cpuEgrBandwidthCtrlRateMode_get(uint32 unit, rtk_rate_rateMode_t *pRate_mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_cpuEgrBandwidthCtrlRateMode_get(unit, pRate_mode);
} /* end of rtk_rate_cpuEgrBandwidthCtrlRateMode_get */

/* Function Name:
 *      rtk_rate_cpuEgrBandwidthCtrlRateMode_set
 * Description:
 *      Set rate counting mode of CPU port egress bandwidth control.
 * Input:
 *      unit      - unit id
 *      rate_mode - Rate counting mode
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
 *      The rate mode are as following:
 *      - RATE_MODE_BYTE
 *      - RATE_MODE_PKT
 */
int32
rtk_rate_cpuEgrBandwidthCtrlRateMode_set(uint32 unit, rtk_rate_rateMode_t rate_mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_cpuEgrBandwidthCtrlRateMode_set(unit, rate_mode);
} /* end of rtk_rate_cpuEgrBandwidthCtrlRateMode_set */

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlIncludeIfg_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_portEgrBandwidthCtrlIncludeIfg_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pIfg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portEgrBandwidthCtrlIncludeIfg_get(unit, port, pIfg_include);
} /* end of rtk_rate_portEgrBandwidthCtrlIncludeIfg_get */

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlIncludeIfg_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) Egress bandwidth control includes/excludes the Preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_rate_portEgrBandwidthCtrlIncludeIfg_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    ifg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portEgrBandwidthCtrlIncludeIfg_set(unit, port, ifg_include);
} /* end of rtk_rate_portEgrBandwidthCtrlIncludeIfg_set */

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlBurstSize_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Applicable:
 *      8328
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is byte.
 *    2) The maximum value of burst_size is 0xFFFF.
 */
int32
rtk_rate_portEgrBandwidthCtrlBurstSize_get(uint32 unit, rtk_port_t port, uint32 *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);
    
    return RT_MAPPER(unit)->rate_portEgrBandwidthCtrlBurstSize_get(unit, port, pBurst_size);
} /* end of rtk_rate_portEgrBandwidthCtrlBurstSize_get */

/* Function Name:
 *      rtk_rate_portEgrBandwidthCtrlBurstSize_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is byte.
 *    2) The maximum value of burst_size is 0xFFFF.
 */
int32
rtk_rate_portEgrBandwidthCtrlBurstSize_set(uint32 unit, rtk_port_t port, uint32 burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);
    
    return RT_MAPPER(unit)->rate_portEgrBandwidthCtrlBurstSize_set(unit, port, burst_size);
} /* end of rtk_rate_portEgrBandwidthCtrlBurstSize_set */

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrQueueBwCtrlEnable_get(unit, phy_port, queue, pEnable);
    }
#else
    return RT_MAPPER(unit)->rate_egrQueueBwCtrlEnable_get(unit, port, queue, pEnable);
#endif
} /* end of rtk_rate_egrQueueBwCtrlEnable_get */

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrQueueBwCtrlEnable_set(unit, phy_port, queue, enable);
    }
#else
    return RT_MAPPER(unit)->rate_egrQueueBwCtrlEnable_set(unit, port, queue, enable);
#endif
} /* end of rtk_rate_egrQueueBwCtrlEnable_set */

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity is 16Kbps in 8328 and 8380.
 *      (3) The valid range of rate is 0 ~ 0xFFFF in 8328.
 *      (4) The unit of granularity is 16Kbps in 8390 normal port.
 *      (5) The unit of granularity is 16Kbps or 1pps in 8390 CPU port depend on mode set by 'rtk_rate_cpuEgrBandwidthCtrlRateMode_set'.
 */
int32
rtk_rate_egrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrQueueBwCtrlRate_get(unit, phy_port, queue, pRate);
    }
#else
    return RT_MAPPER(unit)->rate_egrQueueBwCtrlRate_get(unit, port, queue, pRate);
#endif
} /* end of rtk_rate_egrQueueBwCtrlRate_get */

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_RATE     - Invalid input rate
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The actual rate is "rate * chip granularity".
 *      (2) The unit of granularity is 16Kbps in 8328 and 8380.
 *      (3) The valid range of rate is 0 ~ 0xFFFF in 8328.
 *      (4) The unit of granularity is 16Kbps in 8390 normal port.
 *      (5) The unit of granularity is 16Kbps or 1pps in 8390 CPU port depend on mode set by 'rtk_rate_cpuEgrBandwidthCtrlRateMode_set'.
 */
int32
rtk_rate_egrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrQueueBwCtrlRate_set(unit, phy_port, queue, rate);
    }
#else
    return RT_MAPPER(unit)->rate_egrQueueBwCtrlRate_set(unit, port, queue, rate);
#endif
} /* end of rtk_rate_egrQueueBwCtrlRate_set */

/* Function Name:
 *      rtk_rate_egrQueueFixedBandwidthEnable_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrQueueFixedBandwidthEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrQueueFixedBandwidthEnable_get(unit, phy_port, queue, pEnable);
    }
#else
    return RT_MAPPER(unit)->rate_egrQueueFixedBandwidthEnable_get(unit, port, queue, pEnable);
#endif
} /* end of rtk_rate_wfqFixedBandwidthEnable_get */

/* Function Name:
 *      rtk_rate_egrQueueFixedBandwidthEnable_set
 * Description:
 *      Set enable status of fixed bandwidth ability on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 *      enable  - enable status of fixed bandwidth ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrQueueFixedBandwidthEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_egrQueueFixedBandwidthEnable_set(unit, phy_port, queue, enable);
    }
#else
    return RT_MAPPER(unit)->rate_egrQueueFixedBandwidthEnable_set(unit, port, queue, enable);
#endif
} /* end of rtk_rate_egrQueueFixedBandwidthEnable_set */

/* Function Name:
 *      rtk_rate_egrPortQueueBwCtrlBurstSize_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is KBytes.
 *    2) The maximum value of burst_size is 63. (mean 63 KBytes)
 */
int32
rtk_rate_egrPortQueueBwCtrlBurstSize_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);
    
    return RT_MAPPER(unit)->rate_egrPortQueueBwCtrlBurstSize_get(unit, port, queue, pBurst_size);
} /* end of rtk_rate_egrPortQueueBwCtrlBurstSize_get */

/* Function Name:
 *      rtk_rate_egrPortQueueBwCtrlBurstSize_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *    1) The burst size is "burst_size * chip granularity".
 *       The unit of granularity in RTL8328M is KBytes.
 *    2) The maximum value of burst_size is 63. (mean 63 KBytes)
 */
int32
rtk_rate_egrPortQueueBwCtrlBurstSize_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);
    
    return RT_MAPPER(unit)->rate_egrPortQueueBwCtrlBurstSize_set(unit, port, queue, burst_size);
} /* end of rtk_rate_egrPortQueueBwCtrlBurstSize_set */

/* Module Name    : Rate                           */
/* Sub-module Name: Configuration of storm control */

/* Function Name:
 *      rtk_rate_stormControlRate_get
 * Description:
 *      Get the storm control rate.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pRate      - storm control rate
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Supported management frame is as following:
 *          rtk_rate_storm_group_t \ Chip :   8389    8328    8390  8380
 *          - STORM_GROUP_UNKNOWN_UNICAST     O       O       X     O
 *          - STORM_GROUP_UNKNOWN_MULTICAST   O       O       X     O
 *          - STORM_GROUP_MULTICAST           O       O       O     O
 *          - STORM_GROUP_BROADCAST           O       O       O     O
 *          - STORM_GROUP_UNICAST             X       X       O     O
 *      (2) The actual rate is "rate * chip granularity".
 *      (3) The unit of granularity is 1pps in 8389.
 *      (4) The unit of granularity is 1Kbps or 1pps in 8328 depend on mode.
 *      (5) The unit of granularity is 16Kbps or 1pps in 8390, 8380 depend on mode set by 'rtk_rate_stormControlRateMode_set'.
 *      (6) The rate of unknown multicast and multicat is same in 8328.
 */
int32
rtk_rate_stormControlRate_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlRate_get(unit, phy_port, storm_type, pRate);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlRate_get(unit, port, storm_type, pRate);
#endif
} /* end of rtk_rate_stormControlRate_get */

/* Function Name:
 *      rtk_rate_stormControlRate_set
 * Description:
 *      Set the storm control rate.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      rate       - storm control rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 *      RT_ERR_RATE     - Invalid input bandwidth
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Supported management frame is as following:
 *          rtk_rate_storm_group_t \ Chip :   8389    8328    8390  8380
 *          - STORM_GROUP_UNKNOWN_UNICAST     O       O       X     O
 *          - STORM_GROUP_UNKNOWN_MULTICAST   O       O       X     O
 *          - STORM_GROUP_MULTICAST           O       O       O     O
 *          - STORM_GROUP_BROADCAST           O       O       O     O
 *          - STORM_GROUP_UNICAST             X       X       O     O
 *      (2) The actual rate is "rate * chip granularity".
 *      (3) The unit of granularity is 1pps in 8389.
 *      (4) The unit of granularity is 1Kbps or 1pps in 8328 depend on mode.
 *      (5) The unit of granularity is 16Kbps or 1pps in 8390, 8380 depend on mode set by 'rtk_rate_stormControlRateMode_set'.
 *      (6) The rate of unknown multicast and multicat is same in 8328.
 */
int32
rtk_rate_stormControlRate_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlRate_set(unit, phy_port, storm_type, rate);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlRate_set(unit, port, storm_type, rate);
#endif
} /* end of rtk_rate_stormControlRate_set */

/* Function Name:
 *      rtk_rate_stormControlProtoRate_get
 * Description:
 *      Get the protocol storm control rate.
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The rate unit is 1 pps(packet-per-second).
 */
int32
rtk_rate_stormControlProtoRate_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_rate_storm_proto_group_t    storm_type,
    uint32                          *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlProtoRate_get(unit, port, storm_type, pRate);
} /* end of rtk_rate_stormControlProtoRate_get */

/* Function Name:
 *      rtk_rate_stormControlProtoRate_set
 * Description:
 *      Set the protocol storm control rate.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      rate       - storm control rate (packet-per-second).
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 *      RT_ERR_RATE     - Invalid input bandwidth
 * Applicable:
 *      8390
 * Note:
 *      The rate unit is 1 pps(packet-per-second).
 */
int32
rtk_rate_stormControlProtoRate_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_rate_storm_proto_group_t    storm_type,
    uint32                          rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlProtoRate_set(unit, port, storm_type, rate);
} /* end of rtk_rate_stormControlProtoRate_set */

/* Function Name:
 *      rtk_rate_stormControlEnable_get
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
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The storm group types are as following:
 *          - STORM_GROUP_UNKNOWN_UNICAST
 *          - STORM_GROUP_UNKNOWN_MULTICAST
 *          - STORM_GROUP_MULTICAST
 *          - STORM_GROUP_BROADCAST
 *          - STORM_GROUP_UNICAST (for 8380 only)
 *      (2) For 8390, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_rate_stormControlEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlEnable_get(unit, phy_port, storm_type, pEnable);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlEnable_get(unit, port, storm_type, pEnable);
#endif
} /* end of rtk_rate_stormControlEnable_get */

/* Function Name:
 *      rtk_rate_stormControlEnable_set
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
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_INPUT             - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The storm group types are as following:
 *          - STORM_GROUP_UNKNOWN_UNICAST
 *          - STORM_GROUP_UNKNOWN_MULTICAST
 *          - STORM_GROUP_MULTICAST
 *          - STORM_GROUP_BROADCAST
 *          - STORM_GROUP_UNICAST (for 8380 only)
 *      (2) For 8390, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_rate_stormControlEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    rtk_enable_t            enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlEnable_set(unit, phy_port, storm_type, enable);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlEnable_set(unit, port, storm_type, enable);
#endif
} /* end of rtk_rate_stormControlEnable_set */

/* Function Name:
 *      rtk_rate_portStormControlRateMode_get
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
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The storm group types are as following:
 *          - STORM_GROUP_UNKNOWN_UNICAST
 *          - STORM_GROUP_UNKNOWN_MULTICAST
 *          - STORM_GROUP_MULTICAST
 *          - STORM_GROUP_BROADCAST
 *      (2) The rate mode are as following:
 *          - BASED_ON_PKT
 *          - BASED_ON_BYTE
 */
int32
rtk_rate_portStormControlRateMode_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   *pRate_mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portStormControlRateMode_get(unit, port, storm_type, pRate_mode);
} /* end of rtk_rate_portStormControlRateMode_get */

/* Function Name:
 *      rtk_rate_portStormControlRateMode_set
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
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_INPUT             - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The storm group types are as following:
 *          - STORM_GROUP_UNKNOWN_UNICAST
 *          - STORM_GROUP_UNKNOWN_MULTICAST
 *          - STORM_GROUP_MULTICAST
 *          - STORM_GROUP_BROADCAST
 *      (2) The rate mode are as following:
 *          - BASED_ON_PKT
 *          - BASED_ON_BYTE
 */
int32
rtk_rate_portStormControlRateMode_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_rate_storm_group_t      storm_type,
    rtk_rate_storm_rateMode_t   rate_mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portStormControlRateMode_set(unit, port, storm_type, rate_mode);
} /* end of rtk_rate_portStormControlRateMode_set */

/* Function Name:
 *      rtk_rate_stormControlRateMode_get
 * Description:
 *      Get rate counting mode of storm control.
 * Input:
 *      unit       - unit id
 * Output:
 *      pRate_mode - pointer to rate counting mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The rate mode are as following:
 *          - BASED_ON_PKT
 *          - BASED_ON_BYTE
 */
int32
rtk_rate_stormControlRateMode_get(
    uint32                      unit,
    rtk_rate_storm_rateMode_t   *pRate_mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlRateMode_get(unit, pRate_mode);
} /* end of rtk_rate_stormControlRateMode_get */

/* Function Name:
 *      rtk_rate_stormControlRateMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The rate mode are as following:
 *          - BASED_ON_PKT
 *          - BASED_ON_BYTE
 */
int32
rtk_rate_stormControlRateMode_set(
    uint32                      unit,
    rtk_rate_storm_rateMode_t   rate_mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlRateMode_set(unit, rate_mode);
} /* end of rtk_rate_stormControlRateMode_set */

/* Function Name:
 *      rtk_rate_portStormControlBurstSize_get
 * Description:
 *      Get burst size of storm control on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      storm_type  - storm group type
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The storm group types are as following:
 *          rtk_rate_storm_group_t \ Chip :   8328    8390
 *          - STORM_GROUP_UNKNOWN_UNICAST     O       X
 *          - STORM_GROUP_UNKNOWN_MULTICAST   O       X
 *          - STORM_GROUP_MULTICAST           O       O
 *          - STORM_GROUP_BROADCAST           O       O
 *          - STORM_GROUP_UNICAST             X       O
 *      (2) The unit of burst size is 1 byte or 1 packet in 8328 and 8390 depend on mode.
 *      (3) The range of burst size is 0 ~ 0xFFFFFF in 8328
 */
int32
rtk_rate_portStormControlBurstSize_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portStormControlBurstSize_get(unit, port, storm_type, pBurst_size);
} /* end of rtk_rate_portStormControlBurstSize_get */

/* Function Name:
 *      rtk_rate_portStormControlBurstSize_set
 * Description:
 *      Set burst size of storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      burst_size - burst size
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_INPUT             - invalid input parameter
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The storm group types are as following:
 *          rtk_rate_storm_group_t \ Chip :   8328    8390
 *          - STORM_GROUP_UNKNOWN_UNICAST     O       X
 *          - STORM_GROUP_UNKNOWN_MULTICAST   O       X
 *          - STORM_GROUP_MULTICAST           O       O
 *          - STORM_GROUP_BROADCAST           O       O
 *          - STORM_GROUP_UNICAST             X       O
 *      (2) The unit of burst size is 1 byte or 1 packet in 8328 and 8390 depend on mode.
 *      (3) The range of burst size is 0 ~ 0xFFFFFF in 8328
 *      (4) burst_size range of 8390 is as following:
 *          The (minimum, maximum) of burst_rate setting range of normal port is (1700, 65535).  
 *          The (minimum, maximum) of burst_rate setting range of 10G port is (2650, 1048575).  
 */
int32
rtk_rate_portStormControlBurstSize_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_portStormControlBurstSize_set(unit, port, storm_type, burst_size);
} /* end of rtk_rate_portStormControlBurstSize_set */

/* Function Name:
 *      rtk_rate_stormControlBurstSize_get
 * Description:
 *      Get burst size of storm control.
 * Input:
 *      unit        - unit id
 *      storm_type  - storm group type
 * Output:
 *      pBurst_size - pointer to burst size
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      The storm group types are as following:
 *      - STORM_GROUP_UNKNOWN_UNICAST
 *      - STORM_GROUP_UNKNOWN_MULTICAST
 *      - STORM_GROUP_MULTICAST
 *      - STORM_GROUP_BROADCAST
 *      - STORM_GROUP_UNICAST
 */
int32
rtk_rate_stormControlBurstSize_get(
    uint32                  unit,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlBurstSize_get(unit, storm_type, pBurst_size);
} /* end of rtk_rate_stormControlBurstSize_get */

/* Function Name:
 *      rtk_rate_stormControlBurstSize_set
 * Description:
 *      Set burst size of storm control.
 * Input:
 *      unit       - unit id
 *      storm_type - storm group type
 *      burst_size - burst size
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      The storm group types are as following:
 *      - STORM_GROUP_UNKNOWN_UNICAST
 *      - STORM_GROUP_UNKNOWN_MULTICAST
 *      - STORM_GROUP_MULTICAST
 *      - STORM_GROUP_BROADCAST
 *      - STORM_GROUP_UNICAST
 */
int32
rtk_rate_stormControlBurstSize_set(
    uint32                  unit,
    rtk_rate_storm_group_t  storm_type,
    uint32                  burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlBurstSize_set(unit, storm_type, burst_size);
} /* end of rtk_rate_stormControlBurstSize_set */

/* Function Name:
 *      rtk_rate_stormControlIncludeIfg_get
 * Description:
 *      Get enable status of includes IFG for storm control.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - pointer to enable status of includes IFG
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Storm control includes/excludes the preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED: exclude preamble & IFG
 *          - ENABLED: include preamble & IFG
 */
int32
rtk_rate_stormControlIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlIncludeIfg_get(unit, pIfg_include);
} /* end of rtk_rate_stormControlIncludeIfg_get */

/* Function Name:
 *      rtk_rate_stormControlIncludeIfg_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Storm control includes/excludes the preamble & IFG (20 Bytes).
 *      (2) The status of ifg_include:
 *          - DISABLED: exclude preamble & IFG
 *          - ENABLED: include preamble & IFG
 */
int32
rtk_rate_stormControlIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlIncludeIfg_set(unit, ifg_include);
} /* end of rtk_rate_stormControlIncludeIfg_set */

/* Function Name:
 *      rtk_rate_stormControlExceed_get
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
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_SFC_UNKNOWN_GROUP - Unknown storm group
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Exceed status is as following
 *          - TRUE      storm rate is more than configured rate.
 *          - FALSE     storm rate is never over then configured rate.
 *      (2) The routine get exceed status and clear it after read
 */
int32
rtk_rate_stormControlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type,
    uint32                  *pIsExceed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlExceed_get(unit, phy_port, storm_type, pIsExceed);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlExceed_get(unit, port, storm_type, pIsExceed);
#endif
} /* end of rtk_rate_stormControlExceed_get */

/* Function Name:
 *      rtk_rate_stormControlExceed_reset
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_stormControlExceed_reset(
    uint32                  unit,
    rtk_port_t              port,
    rtk_rate_storm_group_t  storm_type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlExceed_reset(unit, phy_port, storm_type);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlExceed_reset(unit, port, storm_type);
#endif
} /* end of rtk_rate_stormControlExceed_reset */

/* Function Name:
 *      rtk_rate_stormControlProtoExceed_get
 * Description:
 *      Get exceed status of protocol storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pIsExceed  - pointer to exceed status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Exceed status is as following
 *      - TRUE      - storm rate is more than configured rate.
 *      - FALSE     - storm rate is never over then configured rate.
 */
int32
rtk_rate_stormControlProtoExceed_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_rate_storm_proto_group_t    storm_type,
    uint32                          *pIsExceed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlProtoExceed_get(unit, port, storm_type, pIsExceed);
} /* end of rtk_rate_stormControlProtoExceed_get */

/* Function Name:
 *      rtk_rate_stormControlProtoExceed_reset
 * Description:
 *      Reset exceed status of protocol storm control on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_rate_stormControlProtoExceed_reset(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_rate_storm_proto_group_t    storm_type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlProtoExceed_reset(unit, port, storm_type);
} /* end of rtk_rate_stormControlProtoExceed_reset */

/* Function Name:
 *      rtk_rate_igrBandwidthFCOffRate_set
 * Description:
 *      Set the ingress bandwidth control flow control off rate.
 * Input:
 *      unit - unit id
 *      rate - ingress bandwidth control flow control off rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_RATE     - Invalid input rate
 * Applicable:
 *      8389
 * Note:
 *      (1) The unit of granularity is 1 byte
 *      (2) The valid range is from 0 to 0x3FFF
 */
int32
rtk_rate_igrBandwidthFCOffRate_set(uint32 unit, uint32 rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrBandwidthFCOffRate_set(unit, rate);
} /* end of rtk_rate_igrBandwidthFCOffRate_set */

/* Function Name:
 *      rtk_rate_stormControlRefreshMode_get
 * Description:
 *      Get refresh mode of storm control.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to refresh mode of storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The rate mode are as following:
 *          - BASED_ON_PKT
 *          - BASED_ON_BYTE
 *      (2) For 8390, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_rate_stormControlRefreshMode_get(uint32 unit, rtk_rate_storm_rateMode_t *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlRefreshMode_get(unit, pMode);
} /* end of rtk_rate_stormControlRefreshMode_get */

/* Function Name:
 *      rtk_rate_stormControlRefreshMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390
 * Note:
 *      (1) The rate mode are as following:
 *          - BASED_ON_PKT
 *          - BASED_ON_BYTE
 *      (2) For 8390, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_rate_stormControlRefreshMode_set(uint32 unit, rtk_rate_storm_rateMode_t mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlRefreshMode_set(unit, mode);
} /* end of rtk_rate_stormControlRefreshMode_set */

/* Function Name:
 *      rtk_rate_stormControlTypeSel_get
 * Description:
 *      Get the configuration of storm control type selection.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pStorm_sel - storm selection
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API is only supported in unicast and multicast, the storm group types are as following:
 *      - STORM_GROUP_UNICAST
 *      - STORM_GROUP_MULTICAST
 *
 *      The storm selection are as following:
 *      - STORM_SEL_UNKNOWN
 *      - STORM_SEL_UNKNOWN_AND_KNOWN
 */
int32
rtk_rate_stormControlTypeSel_get(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_storm_sel_t *pStorm_sel)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlTypeSel_get(unit, phy_port, storm_type, pStorm_sel);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlTypeSel_get(unit, port, storm_type, pStorm_sel);
#endif
} /* end of rtk_rate_stormControlTypeSel_get */

/* Function Name:
 *      rtk_rate_stormControlTypeSel_set
 * Description:
 *      Set the configuration of storm control type selection.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      storm_type - storm group type
 *      storm_sel  - storm selection
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API is only supported in unicast and multicast, the storm group types are as following:
 *      - STORM_GROUP_UNICAST
 *      - STORM_GROUP_MULTICAST
 *
 *      The storm selection are as following:
 *      - STORM_SEL_UNKNOWN
 *      - STORM_SEL_UNKNOWN_AND_KNOWN
 */
int32
rtk_rate_stormControlTypeSel_set(uint32 unit, rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_storm_sel_t storm_sel)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_stormControlTypeSel_set(unit, phy_port, storm_type, storm_sel);
    }
#else
    return RT_MAPPER(unit)->rate_stormControlTypeSel_set(unit, port, storm_type, storm_sel);
#endif
} /* end of rtk_rate_stormControlTypeSel_set */

/* Function Name:
 *      rtk_rate_stormControlBypass_get
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
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      If a certain packet type is selected to bypass the storm control, the packet will
 *      not be blocked by the storm control and will not consume the bandwidth.
 */
int32
rtk_rate_stormControlBypass_get(uint32 unit, rtk_rate_storm_bypassType_t bypassType, rtk_enable_t  *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlBypass_get(unit, bypassType, pEnable);
} /* end of rtk_rate_stormControlBypass_get */

/* Function Name:
 *      rtk_rate_stormControlBypass_set
 * Description:
 *      Set the status of bypass storm control for specified packet type.
 * Input:
 *      unit       - unit id
 *      bypassType - bypass type
 *      enable     - status of bypass storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      If a certain packet type is selected to bypass the storm control, the packet will
 *      not be blocked by the storm control and will not consume the bandwidth.
 */
int32
rtk_rate_stormControlBypass_set(uint32 unit, rtk_rate_storm_bypassType_t bypassType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_stormControlBypass_set(unit, bypassType, enable);
} /* end of rtk_rate_stormControlBypass_set */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlEnable_get
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_rate_igrQueueBwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlEnable_get(unit, phy_port, queue, pEnable);
    }
#else
    return RT_MAPPER(unit)->rate_igrQueueBwCtrlEnable_get(unit, port, queue, pEnable);
#endif
} /* end of rtk_rate_igrQueueBwCtrlEnable_get */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlEnable_set
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_rate_igrQueueBwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlEnable_set(unit, phy_port, queue, enable);
    }
#else
    return RT_MAPPER(unit)->rate_igrQueueBwCtrlEnable_set(unit, port, queue, enable);
#endif
} /* end of rtk_rate_igrQueueBwCtrlEnable_set */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlRate_get
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
 * Applicable:
 *      8380
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity is 16Kbps.
 */
int32
rtk_rate_igrQueueBwCtrlRate_get(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      *pRate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlRate_get(unit, phy_port, queue, pRate);
    }
#else
    return RT_MAPPER(unit)->rate_igrQueueBwCtrlRate_get(unit, port, queue, pRate);
#endif
} /* end of rtk_rate_igrQueueBwCtrlRate_get */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlRate_set
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
 * Applicable:
 *      8380
 * Note:
 *      The actual rate is "rate * chip granularity".
 *      The unit of granularity is 16Kbps.
 */
int32
rtk_rate_igrQueueBwCtrlRate_set(
    uint32      unit,
    rtk_port_t  port,
    rtk_qid_t   queue,
    uint32      rate)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlRate_set(unit, phy_port, queue, rate);
    }
#else
    return RT_MAPPER(unit)->rate_igrQueueBwCtrlRate_set(unit, port, queue, rate);
#endif
} /* end of rtk_rate_igrQueueBwCtrlRate_set */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlExceed_get
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
 * Applicable:
 *      8380
 * Note:
 *      Exceed status is as following
 *      - TRUE      - ingress queue bandwidth rate is more than configured rate.
 *      - FALSE     -ingress queue bandwidth rate is never over then configured rate.
 */
int32 
rtk_rate_igrQueueBwCtrlExceed_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qid_t               queue,
    uint32                  *pIsExceed)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlExceed_get(unit, phy_port, queue, pIsExceed);
    }
#else
    return RT_MAPPER(unit)->rate_igrQueueBwCtrlExceed_get(unit, port, queue, pIsExceed);
#endif
} /* end of rtk_rate_igrQueueBwCtrlExceed_get */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlExceed_reset
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32 
rtk_rate_igrQueueBwCtrlExceed_reset(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qid_t               queue)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlExceed_reset(unit, phy_port, queue);
    }
#else
    return RT_MAPPER(unit)->rate_igrQueueBwCtrlExceed_reset(unit, port, queue);
#endif
} /* end of rtk_rate_igrQueueRateExceed_reset */


/* Function Name:
 *      rtk_rate_igrPortBwCtrlBurstSize_get
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_rate_igrPortBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrPortBwCtrlBurstSize_get(unit, pBurst_size);
} /* end of rtk_rate_igrPortBwCtrlBurstSize_get */

/* Function Name:
 *      rtk_rate_igrPortBwCtrlBurstSize_set
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_rate_igrPortBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrPortBwCtrlBurstSize_set(unit, burst_size);
} /* end of rtk_rate_igrPortBwCtrlBurstSize_set */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlBurstSize_get
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_rate_igrQueueBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlBurstSize_get(unit, pBurst_size);
} /* end of rtk_rate_igrQueueBwCtrlBurstSize_get */

/* Function Name:
 *      rtk_rate_igrQueueBwCtrlBurstSize_set
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_rate_igrQueueBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_igrQueueBwCtrlBurstSize_set(unit, burst_size);
} /* end of rtk_rate_igrQueueBwCtrlBurstSize_set */

/* Function Name:
 *      rtk_rate_egrPortBwCtrlBurstSize_get
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrPortBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_egrPortBwCtrlBurstSize_get(unit, pBurst_size);
} /* end of rtk_rate_egrPortBwCtrlBurstSize_get */

/* Function Name:
 *      rtk_rate_egrPortBwCtrlBurstSize_set
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrPortBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_egrPortBwCtrlBurstSize_set(unit, burst_size);
} /* end of rtk_rate_egrPortBwCtrlBurstSize_set */

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlBurstSize_get
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrQueueBwCtrlBurstSize_get(
    uint32                  unit,
    uint32                  *pBurst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_egrQueueBwCtrlBurstSize_get(unit, pBurst_size);
} /* end of rtk_rate_egrQueueBwCtrlBurstSize_get */

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlBurstSize_set
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
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_rate_egrQueueBwCtrlBurstSize_set(
    uint32                  unit,
    uint32                  burst_size)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->rate_egrQueueBwCtrlBurstSize_set(unit, burst_size);
} /* end of rtk_rate_egrQueueBwCtrlBurstSize_set */

