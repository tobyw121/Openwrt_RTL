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
 * $Revision: 34020 $
 * $Date: 2012-11-02 19:10:52 +0800 (Fri, 02 Nov 2012) $
 *
 * Purpose : Definition of Flow Control API
 *
 * Feature : Configure Threshold Parameters of Flow Control
 *
 */

#ifndef __RTK_FLOWCTRL_H__
#define __RTK_FLOWCTRL_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
typedef struct rtk_flowctrl_thresh_s
{
    uint32 highOn;
    uint32 highOff;
    uint32 lowOn;
    uint32 lowOff;
} rtk_flowctrl_thresh_t;

typedef struct rtk_flowctrl_drop_thresh_s
{
    uint32 high;
    uint32 low;
} rtk_flowctrl_drop_thresh_t;

typedef enum rtk_flowctrl_pauseOnAction_e
{
    PAUSE_ON_RECEIVE = 0,
    PAUSE_ON_DROP,
    PAUSE_ON_END
} rtk_flowctrl_pauseOnAction_t;

/* Egress drop mode */
typedef enum rtk_flowctrl_egrDropMode_e
{
    EGR_DROP_FOR_RX_AND_TX_PKTS = 0,
    EGR_DROP_FOR_RX_PKTS,
    EGR_DROP_FOR_TX_PKTS,
    EGR_DROP_DISABLE,
    EGR_DROP_END
} rtk_flowctrl_egrDropMode_t;

/* HOL traffic type */
typedef enum rtk_flowctrl_holTrafficType_e
{
    HOL_TRAFFIC_TYPE_UNKN_UC = 0,
    HOL_TRAFFIC_TYPE_L2_MC,
    HOL_TRAFFIC_TYPE_IP_MC,
    HOL_TRAFFIC_TYPE_BC,
    HOL_TRAFFIC_TYPE_END
} rtk_flowctrl_holTrafficType_t;

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_flowctrl_init
 * Description:
 *      Initialize flowctrl module of the specified device.
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
 *      Must initialize flowctrl module before calling any flowctrl APIs.
 */
extern int32
rtk_flowctrl_init(uint32 unit);


/* Module Name    : Flow Control               */
/* Sub-module Name: Flow control configuration */

/* Function Name:
 *      rtk_flowctrl_portEnable_get
 * Description:
 *      Get enable status of flowcontrol on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of flowcontrol
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
 *      None
 */
extern int32
rtk_flowctrl_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_portEnable_set
 * Description:
 *      Set enable status of flowcontrol on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of flowcontrol
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
 *      None
 */
extern int32
rtk_flowctrl_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_portPauseForceModeEnable_get
 * Description:
 *      Get enable status of force pause on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of force pause
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
 *      None
 */
extern int32
rtk_flowctrl_portPauseForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_portPauseForceModeEnable_set
 * Description:
 *      Set enable status of force pause on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of force pause
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
 *      None
 */
extern int32
rtk_flowctrl_portPauseForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_pauseOnAction_get
 * Description:
 *      Get action for the scenario that packet keeps coming in after pause on frame is sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer to action of packet receive
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
 *      Action of packet receive is as following
 *      - PAUSE_ON_RECEIVE: after Pause ON frame is sent, incoming packets always can be received.
 *      - PAUSE_ON_DROP: after Pause ON frame is sent, incoming packets can be received up to one specified number.
 */
extern int32
rtk_flowctrl_pauseOnAction_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    *pAction);

/* Function Name:
 *      rtk_flowctrl_pauseOnAction_set
 * Description:
 *      Set action for the scenario that packet keeps coming in after pause on frame is sent.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - action of packet receive
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
 *      Action of packet receive is as following
 *      - PAUSE_ON_RECEIVE: after Pause ON frame is sent, incoming packets always can be received.
 *      - PAUSE_ON_DROP: after Pause ON frame is sent, incoming packets can be received up to one specified number.
 *        The specified number can be configured by rtk_flowctrl_pauseOnAllowedPageNum_set,
 *        rtk_flowctrl_pauseOnAllowedPktNum_set, rtk_flowctrl_pauseOnAllowedPktLen_set APIs
 */
extern int32
rtk_flowctrl_pauseOnAction_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    action);

/* Function Name:
 *      rtk_flowctrl_pauseOnAllowedPageNum_get
 * Description:
 *      Get number of allowed page when pause on frame sent.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pPageNum - pointer to number of received page
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8380
 * Note:
 *      (1) The setting is active if pause on action is PAUSE_ON_DROP, refer to rtk_flowctrl_pauseOnAction_set.
 *      (2) The valid range of pageNum is 0 ~ 0xFF, mean allowed receive page number after pause on frame sent.
 */
extern int32
rtk_flowctrl_pauseOnAllowedPageNum_get(uint32 unit, rtk_port_t port, uint32 *pPageNum);

/* Function Name:
 *      rtk_flowctrl_pauseOnAllowedPageNum_set
 * Description:
 *      Set number of allowed page when pause on frame sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pageNum - number of received page
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328, 8380
 * Note:
 *      (1) The setting is active if pause on action is PAUSE_ON_DROP, refer to rtk_flowctrl_pauseOnAction_set.
 *      (2) The valid range of pageNum is 0 ~ 0xFF, mean allowed receive page number after pause on frame sent.
 */
extern int32
rtk_flowctrl_pauseOnAllowedPageNum_set(uint32 unit, rtk_port_t port, uint32 pageNum);

/* Function Name:
 *      rtk_flowctrl_pauseOnAllowedPktLen_get
 * Description:
 *      Get allowed receiving packet length after pause on frame is sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pPktLen - pointer to number of received packet length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) The setting is active if pause on action is PAUSE_ON_DROP, refer to rtk_flowctrl_pauseOnAction_set.
 *      (2) The length unit is byte.
 */
extern int32
rtk_flowctrl_pauseOnAllowedPktLen_get(uint32 unit, rtk_port_t port, uint32 *pPktLen);

/* Function Name:
 *      rtk_flowctrl_pauseOnAllowedPktLen_set
 * Description:
 *      Set allowed receiving packet length after pause on frame is sent.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pktLen - number of received packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390
 * Note:
 *      (1) The setting is active if pause on action is PAUSE_ON_DROP, refer to rtk_flowctrl_pauseOnAction_set.
 *      (2) The length unit is byte.
 */
extern int32
rtk_flowctrl_pauseOnAllowedPktLen_set(uint32 unit, rtk_port_t port, uint32 pktLen);

/* Function Name:
 *      rtk_flowctrl_pauseOnAllowedPktNum_get
 * Description:
 *      Get number of allowed receiving packet when pause on frame is sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pPktNum - pointer to number of received packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      The setting is active if pause on action is PAUSE_ON_DROP, refer to rtk_flowctrl_pauseOnAction_set.
 */
extern int32
rtk_flowctrl_pauseOnAllowedPktNum_get(uint32 unit, rtk_port_t port, uint32 *pPktNum);

/* Function Name:
 *      rtk_flowctrl_pauseOnAllowedPktNum_set
 * Description:
 *      Set number of allowed receiving packet when pause on frame is sent.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pktNum - number of received packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328, 8390
 * Note:
 *      The setting is active if pause on action is PAUSE_ON_DROP, refer to rtk_flowctrl_pauseOnAction_set.
 */
extern int32
rtk_flowctrl_pauseOnAllowedPktNum_set(uint32 unit, rtk_port_t port, uint32 pktNum);


/*
 * Flow Control ON
 */

/* Function Name:
 *      rtk_flowctrl_igrSystemPauseThresh_get
 * Description:
 *      Get ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the threshold structure in the system used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in system.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389, 8328 and 8380, 0 ~ 0xFFF in 8390
 */
extern int32
rtk_flowctrl_igrSystemPauseThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrSystemPauseThresh_set
 * Description:
 *      Set ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the threshold structure in the system used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in system.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389, 8328 and 8380, 0 ~ 0xFFF in 8390
 */
extern int32
rtk_flowctrl_igrSystemPauseThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrPortPauseThresh_get
 * Description:
 *      Get used page high/low threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in the port.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328.
 *      (5) The 8389 have only one low threshold, mean lowOn and lowOff are equal.
 */
extern int32
rtk_flowctrl_igrPortPauseThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrPortPauseThresh_set
 * Description:
 *      Set used page high/low threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in the port.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328.
 *      (5) The 8389 have only one low threshold, mean lowOn and lowOff should be set to the same value.
 *          Otherwise RT_ERR_INPUT will be returned
 */
extern int32
rtk_flowctrl_igrPortPauseThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrPauseThreshGroup_get
 * Description:
 *      Get ingress port used page high/low threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) lowOff threshold is unused.
 *      (2) Each port can map to a threshold group, refer to rtk_flowctrl_igrPortPauseThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrPauseThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrPauseThreshGroup_set
 * Description:
 *      Set ingress port used page high/low threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) lowOff threshold is unused.
 *      (2) Each port can map to a threshold group, refer to rtk_flowctrl_igrPortPauseThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrPauseThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrPortPauseThreshGroupSel_get
 * Description:
 *      Get ingress port used page pause threshold group for the specified port
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pGrp_idx - pointer to the index of threshold group
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
 *      None
 */
extern int32
rtk_flowctrl_igrPortPauseThreshGroupSel_get(uint32 unit, rtk_port_t port, uint32 *pGrp_idx);

/* Function Name:
 *      rtk_flowctrl_igrPortPauseThreshGroupSel_set
 * Description:
 *      Set ingress port used page pause threshold group for the specified port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      grp_idx - index of threshold group
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
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_igrPortPauseThreshGroupSel_set(uint32 unit, rtk_port_t port, uint32 grp_idx);


/* Module Name    : Flow Control              */
/* Sub-module Name: Egress drop configuration */

/* Function Name:
 *      rtk_flowctrl_egrPortDropMode_get
 * Description:
 *      Get egress drop mode on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pDropMode - pointer to egress drop mode
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
 *      Egress drop mode is as following
 *      - EGR_DROP_FOR_RX_AND_TX_PKTS
 *      - EGR_DROP_FOR_RX_PKTS
 *      - EGR_DROP_FOR_TX_PKTS
 *      - EGR_DROP_DISABLE
 */
extern int32
rtk_flowctrl_egrPortDropMode_get(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t *pDropMode);

/* Function Name:
 *      rtk_flowctrl_egrPortDropMode_set
 * Description:
 *      Set egress drop mode on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      dropMode - egress drop mode
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
 *      Egress drop mode is as following
 *      - EGR_DROP_FOR_RX_AND_TX_PKTS
 *      - EGR_DROP_FOR_RX_PKTS
 *      - EGR_DROP_FOR_TX_PKTS
 *      - EGR_DROP_DISABLE
 */
extern int32
rtk_flowctrl_egrPortDropMode_set(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t dropMode);

/* Function Name:
 *      rtk_flowctrl_egrPortDropForceModeEnable_get
 * Description:
 *      Get enable status of force drop mode on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of force drop mode
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
 *      None
 */
extern int32
rtk_flowctrl_egrPortDropForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_egrPortDropForceModeEnable_set
 * Description:
 *      Set enable status of force drop mode on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of force drop mode
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
 *      When enable force drop mode, this port will ignore flowcontrol result from auto negotiation
 */
extern int32
rtk_flowctrl_egrPortDropForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/*
 * Flow Control OFF
 */

/* Function Name:
 *      rtk_flowctrl_igrSystemCongestThresh_get
 * Description:
 *      Get system used page high/low drop threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the threshold structure in the public used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in system.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389, 8328 and 8380, 0 ~ 0xFFF in 8390.
 */
extern int32
rtk_flowctrl_igrSystemCongestThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrSystemCongestThresh_set
 * Description:
 *      Set system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the threshold structure in the public used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in system.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389, 8328 and 8380, 0 ~ 0xFFF in 8390.
 */
extern int32
rtk_flowctrl_igrSystemCongestThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrPortCongestThresh_get
 * Description:
 *      Get used page high drop threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in the port.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328.
 *      (5) The 8389 have only one low threshold, mean lowOn and lowOff are equal.
 */
extern int32
rtk_flowctrl_igrPortCongestThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrPortCongestThresh_set
 * Description:
 *      Set used page high drop threshold for the specified ingress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8389, 8328
 * Note:
 *      (1) There are four fields in rtk_flowctrl_thresh_t: highOn, highOff, lowOn, lowOff
 *      (2) The four field value are threshold of public page count in the port.
 *      (3) The four field value should be highOn > highOff > lowOn > lowOff
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328.
 *      (5) The 8389 have only one low threshold, mean lowOn and lowOff should be set to the same value.
 *          Otherwise RT_ERR_INPUT will be returned
 */
extern int32
rtk_flowctrl_igrPortCongestThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrCongestThreshGroup_get
 * Description:
 *      Get used page drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - the index of threshold group
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) lowOff threshold is unused.
 *      (2) Each port can map to a threshold group, refer to rtk_flowctrl_igrPortPauseThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrCongestThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrCongestThreshGroup_set
 * Description:
 *      Set used page drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - the index of threshold group
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) lowOff threshold is unused.
 *      (2) Each port can map to a threshold group, refer to rtk_flowctrl_igrPortPauseThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrCongestThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrSystemDropThresh_get
 * Description:
 *      Get egress system drop threshold for the specified unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in system.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328, 0 ~ 0xFFF in 8390.
 *      (5) The 8389, 8390 have only one threshold, mean high and low are equal.
 */
extern int32
rtk_flowctrl_egrSystemDropThresh_get(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrSystemDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in system.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328, 0 ~ 0xFFF in 8390.
 *      (5) The 8389, 8390 have only one threshold, mean high and low should be set to the same value.
 */
extern int32
rtk_flowctrl_egrSystemDropThresh_set(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrPortDropThresh_get
 * Description:
 *      Get egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in system.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328, 0 ~ 0xFFF in 8390.
 *      (5) The 8389 only have high threshold, and low threshold always return 0.
 */
extern int32
rtk_flowctrl_egrPortDropThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrPortDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8389, 8328, 8390
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in system.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8389 and 8328, 0 ~ 0xFFF in 8390.
 *      (5) The 8389 only have high threshold, and low threshold field will be ignored.
 */
extern int32
rtk_flowctrl_egrPortDropThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrPortQueueDropThresh_get
 * Description:
 *      Get egress drop threshold for the specified egress port and egress queue
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in the queue of the port.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8328.
 */
extern int32
rtk_flowctrl_egrPortQueueDropThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrPortQueueDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port and egress queue
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in the queue of the port.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8328.
 */
extern int32
rtk_flowctrl_egrPortQueueDropThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrPortQueueDropEnable_get
 * Description:
 *      Get egress queue drop ability for the specified port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of egress queue drop ability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Enable egress Q drop function for the specified port. The packet is dropped if the corresponding
 *      egress Q used page count over the egress Q drop threshold. Use rtk_flowctrl_egrQueueDropThresh_set
 *      to specify the queue drop thresold.
 */
extern int32
rtk_flowctrl_egrPortQueueDropEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_egrPortQueueDropEnable_set
 * Description:
 *      Set egress queue drop ability for the specified port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of egress queue drop ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      Enable egress Q drop function for the specified port. The packet is dropped if the corresponding
 *      egress Q used page count over the egress Q drop threshold. Use rtk_flowctrl_egrQueueDropThresh_set
 *      to specify the queue drop thresold.
 */
extern int32
rtk_flowctrl_egrPortQueueDropEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_egrQueueDropThresh_get
 * Description:
 *      Get egress global drop threshold for the egress queue
 * Input:
 *      unit    - unit id
 *      queue   - queue id
 * Output:
 *      pThresh - pointer to the global drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8390
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in the queue of all ports.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8389, 0 ~ 0xFFF in 8390
 */
extern int32
rtk_flowctrl_egrQueueDropThresh_get(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrQueueDropThresh_set
 * Description:
 *      Set egress gloabl drop threshold for the egress queue
 * Input:
 *      unit    - unit id
 *      queue   - queue id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8389, 8390
 * Note:
 *      (1) There are two fields in rtk_flowctrl_drop_thresh_t: high, low
 *      (2) The two field value are threshold of public page count in system.
 *      (3) The two field value should be high > low
 *      (4) The valid range is 0 ~ 0x7FF in 8389, 0 ~ 0xFFF in 8390
 */
extern int32
rtk_flowctrl_egrQueueDropThresh_set(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrCpuQueueDropThresh_get
 * Description:
 *      Get CPU port egress queue drop threshold
 * Input:
 *      unit    - unit id
 *      queue   - queue id
 * Output:
 *      pThresh - pointer to the global drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) CPU port has dedicated egress queue drop threshold configuration different from normal port.
 *          To configure normal port, use rtk_flowctrl_egrQueueDropThresh_set instead.
 */
extern int32
rtk_flowctrl_egrCpuQueueDropThresh_get(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrCpuQueueDropThresh_set
 * Description:
 *      Set egress gloabl drop threshold for the egress queue of CPU port
 * Input:
 *      unit    - unit id
 *      queue   - queue id
 *      pThresh - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390
 * Note:
 *      (1) CPU port has dedicated egress queue drop threshold configuration different from normal port.
 *          To configure normal port, use rtk_flowctrl_egrQueueDropThresh_set instead.
 */
extern int32
rtk_flowctrl_egrCpuQueueDropThresh_set(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrPortDropRefCongestEnable_set
 * Description:
 *      Set enable status of refering source port congest status for egress drop
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of refering source port congest status
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
extern int32
rtk_flowctrl_egrPortDropRefCongestEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_egrPortDropRefCongestEnable_get
 * Description:
 *      Get enable status of refering source port congest status for egress drop
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of refering source port congest status
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
extern int32
rtk_flowctrl_egrPortDropRefCongestEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_egrPortDropThreshGroup_get
 * Description:
 *      Get egress port drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 * Output:
 *      pThresh - pointer to the threshold structure for the port used page count
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
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_egrPortQueueDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_egrPortDropThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_flowctrl_drop_thresh_t * pThresh);

/* Function Name:
 *      rtk_flowctrl_egrPortDropThreshGroup_set
 * Description:
 *      Set egress port drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      pThresh - pointer to the threshold structure for the port used page count
 * Output:
 *      None
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
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_egrPortQueueDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_egrPortDropThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrQueueDropThreshGroup_get
 * Description:
 *      Get egress queue drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue   - queue id
 * Output:
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_egrPortQueueDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_egrQueueDropThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_egrQueueDropThreshGroup_set
 * Description:
 *      Set egress queue drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue   - queue id
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_egrPortQueueDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_egrQueueDropThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrQueueDropEnable_get
 * Description:
 *      Get enable status of ingress queue drop.
 * Input:
 *      unit  - unit id
 *      port  - queue id
 *      queue - queue id
 * Output:
 *      pEnable - pointer to enable status of ingress queue drop
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_igrQueueDropEnable_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_igrQueueDropEnable_set
 * Description:
 *      Set enable status of ingress queue drop.
 * Input:
 *      unit   - unit id
 *      port   - queue id
 *      queue  - queue id
 *      enable - enable status of ingress queue drop
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
 *      8380
 * Note:
 *     None
 */
extern int32
rtk_flowctrl_igrQueueDropEnable_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_igrQueuePauseDropThreshGroup_get
 * Description:
 *      Get ingress queue Pause/drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue - queue id
 * Output:
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrQueuePauseDropThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t * pThresh);

/* Function Name:
 *      rtk_flowctrl_igrQueuePauseDropThreshGroup_set
 * Description:
 *      Set ingress queue Pause/drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue   - queue id
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrQueuePauseDropThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t * pThresh);

/* Function Name:
 *      rtk_flowctrl_igrQueuePauseDropThreshGroupSel_get
 * Description:
 *      Get ingress queue used page pause/drop threshold group for the specified port
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pGrp_idx - pointer to the index of threshold group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_igrQueuePauseDropThreshGroupSel_get(uint32 unit, rtk_port_t port, uint32 *pGrp_idx);

/* Function Name:
 *      rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set
 * Description:
 *      Set ingress queue used page pause/drop threshold group for the specified port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      grp_idx - index of threshold group
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
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set(uint32 unit, rtk_port_t port, uint32 grp_idx);

/* Function Name:
 *      rtk_flowctrl_egrPortQueueDropThreshGroupSel_get
 * Description:
 *      Get egress port&queue used page drop threshold group for the specified port
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pGrp_idx - pointer to the index of threshold group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_egrPortQueueDropThreshGroupSel_get(uint32 unit, rtk_port_t port, uint32 *pGrp_idx);

/* Function Name:
 *      rtk_flowctrl_egrPortQueueDropThreshGroupSel_set
 * Description:
 *      Set egress port&queue used page drop threshold group for the specified port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      grp_idx - index of threshold group
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
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_egrPortQueueDropThreshGroupSel_set(uint32 unit, rtk_port_t port, uint32 grp_idx);

/* Function Name:
 *      rtk_flowctrl_egrQueueDropEnable_get
 * Description:
 *      Get enable status of egress queue drop.
 * Input:
 *      unit    - unit id
 *      port    - queue id
 *      queue   - queue id
 * Output:
 *      pEnable - pointer to enable status of egress queue drop
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_egrQueueDropEnable_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_egrQueueDropEnable_set
 * Description:
 *      Set enable status of egress queue drop.
 * Input:
 *      unit   - unit id
 *      port   - queue id
 *      queue  - queue id
 *      enable - enable status of egress queue drop
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
 *      8380
 * Note:
 *     None
 */
extern int32
rtk_flowctrl_egrQueueDropEnable_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_egrPortDropEnable_get
 * Description:
 *      Get enable status of egress port drop.
 * Input:
 *      unit    - unit id
 *      port    - queue id
 * Output:
 *      pEnable - pointer to enable status of egress port drop
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_egrPortDropEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_egrPortDropEnable_set
 * Description:
 *      Set enable status of egress port drop.
 * Input:
 *      unit   - unit id
 *      port   - queue id
 *      enable - enable status of egress port drop
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
 *      8380
 * Note:
 *     None
 */
extern int32
rtk_flowctrl_egrPortDropEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_specialCongestThreshold_get
 * Description:
 *      Get special congstion threshold
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the threshold structure for special congstion
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
extern int32
rtk_flowctrl_specialCongestThreshold_get(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_specialCongestThreshold_set
 * Description:
 *      Set special congstion threshold
 * Input:
 *      unit    - unit id
 *      pThresh -the pointer to threshold structure for special congstion
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_flowctrl_specialCongestThreshold_set(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);


/* Function Name:
 *      rtk_flowctrl_igrQueueDropThreshGroup_get
 * Description:
 *      Get ingress queue Drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue   - queue id
 * Output:
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      1) Each port can map to a threshold group, refer to rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrQueueDropThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrQueueDropThreshGroup_set
 * Description:
 *      Set ingress queue Drop threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue   - queue id
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8380
 * Note:
 *      1) Each port can map to a threshold group, refer to rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrQueueDropThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrQueuePauseThreshGroup_get
 * Description:
 *      Get ingress queue Pause threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue - queue id
 * Output:
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrQueuePauseThreshGroup_get(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_igrQueuePauseThreshGroup_set
 * Description:
 *      Set ingress queue Pause threshold for the specified threshold group
 * Input:
 *      unit    - unit id
 *      grp_idx - index of threshold group
 *      queue   - queue id
 *      pThresh - pointer to the threshold structure for the queue used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      (1) Each port can map to a threshold group, refer to rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set.
 */
extern int32
rtk_flowctrl_igrQueuePauseThreshGroup_set(uint32 unit, uint32 grp_idx, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      rtk_flowctrl_portHolTrafficDropEnable_get
 * Description:
 *      Get dropping ability for dropping flooding traffic when flow control is enabled.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable state
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
 *      (1) Per ingress port can enable the drop function to drop flooding traffic.
 *      (2) The function takes effect only if the flow control of ingress port is enabled.
 *      (3) Refer to rtk_flowctrl_holTrafficTypeDropEnable_set for dropping specific traffic type.
 */
extern int32
rtk_flowctrl_portHolTrafficDropEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_portHolTrafficDropEnable_set
 * Description:
 *      Set dropping ability for dropping flooding traffic when flow control is enabled.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Enable the function to prevent HOL by flooding traffic when flow control is enabled.
 *      (2) Per ingress port can enable the drop function to drop flooding traffic at the congested 
 *          egress port.
 *      (3) Refer to rtk_flowctrl_holTrafficTypeDropEnable_set for dropping specific traffic type.
 */
extern int32
rtk_flowctrl_portHolTrafficDropEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_flowctrl_holTrafficTypeDropEnable_get
 * Description:
 *      Get dropping ability for specific traffic type when flow control is enabled.
 * Input:
 *      unit    - unit id
 *      type    - traffic type
 * Output:
 *      pEnable - enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Each traffic type can enable the drop function individually.
 *      (2) The function takes effect if rtk_flowctrl_portHolTrafficDropEnable_set is enabled.
 */
extern int32
rtk_flowctrl_holTrafficTypeDropEnable_get(uint32 unit, rtk_flowctrl_holTrafficType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_flowctrl_holTrafficTypeDropEnable_set
 * Description:
 *      Set dropping ability for specific traffic type when flow control is enabled.
 * Input:
 *      unit    - unit id
 *      type    - traffic type
 *      enable  - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Each traffic type can enable the drop function individually.
 *      (2) The function takes effect if rtk_flowctrl_portHolTrafficDropEnable_set is enabled.
 */
extern int32
rtk_flowctrl_holTrafficTypeDropEnable_set(uint32 unit, rtk_flowctrl_holTrafficType_t type, rtk_enable_t enable);

#endif  /* __RTK_FLOWCTRL_H__ */

