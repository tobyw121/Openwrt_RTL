/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 22392 $
 * $Date: 2011-09-13 16:19:27 +0800 (Tue, 13 Sep 2011) $
 *
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *

 * Feature : The file have include the following module and sub-modules
 *           1) Flow control configuration
 *           2) Egress drop configuration
 *
 */

#ifndef __DAL_ESW_FLOWCTRL_H__
#define __DAL_ESW_FLOWCTRL_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/flowctrl.h>

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
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
extern int32
dal_esw_flowctrl_init(uint32 unit);

/* Module Name    : Flow Control               */
/* Sub-module Name: Flow control configuration */

/* Function Name:
 *      dal_esw_flowctrl_portEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_flowctrl_portEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_flowctrl_portPauseForceModeEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_portPauseForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_flowctrl_portPauseForceModeEnable_set
 * Description:
 *      Set enable status of force pause on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - nable status of force pause
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
dal_esw_flowctrl_portPauseForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAction_get
 * Description:
 *      Get action of packet receive when pause on frame sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer to action of packet receive
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Action of packet receive is as following
 *      - PAUSE_ON_RECEIVE
 *      - PAUSE_ON_DROP
 */
extern int32
dal_esw_flowctrl_pauseOnAction_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    *pAction);

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAction_set
 * Description:
 *      Set action of packet receive when pause on frame sent.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - action of packet receive
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
 *      Action of packet receive is as following
 *      - PAUSE_ON_RECEIVE
 *      - PAUSE_ON_DROP
 */
extern int32
dal_esw_flowctrl_pauseOnAction_set(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_flowctrl_pauseOnAction_t    action);

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPageNum_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_pauseOnAllowedPageNum_get(uint32 unit, rtk_port_t port, uint32 *pPageNum);

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPageNum_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_pauseOnAllowedPageNum_set(uint32 unit, rtk_port_t port, uint32 pageNum);

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPktNum_get
 * Description:
 *      Get number of allowed packet when pause on frame sent.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pPktNum - pointer to number of received packet
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
dal_esw_flowctrl_pauseOnAllowedPktNum_get(uint32 unit, rtk_port_t port, uint32 *pPktNum);

/* Function Name:
 *      dal_esw_flowctrl_pauseOnAllowedPktNum_set
 * Description:
 *      Set number of allowed packet when pause on frame sent.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pktNum - number of received packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_pauseOnAllowedPktNum_set(uint32 unit, rtk_port_t port, uint32 pktNum);

/* Function Name:
 *      dal_esw_flowctrl_igrSystemPauseThresh_get
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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_igrSystemPauseThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_igrSystemPauseThresh_set
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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_igrSystemPauseThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_igrPortPauseThresh_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_igrPortPauseThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_igrPortPauseThresh_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_igrPortPauseThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Module Name    : Flow Control              */
/* Sub-module Name: Egress drop configuration */

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropMode_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Egress drop mode is as following
 *      - EGR_DROP_FOR_RX_AND_TX_PKTS
 *      - EGR_DROP_FOR_RX_PKTS
 *      - EGR_DROP_FOR_TX_PKTS
 *      - EGR_DROP_DISABLE
 */
extern int32
dal_esw_flowctrl_egrPortDropMode_get(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t *pDropMode);

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropMode_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Egress drop mode is as following
 *      - EGR_DROP_FOR_RX_AND_TX_PKTS
 *      - EGR_DROP_FOR_RX_PKTS
 *      - EGR_DROP_FOR_TX_PKTS
 *      - EGR_DROP_DISABLE
 */
extern int32
dal_esw_flowctrl_egrPortDropMode_set(uint32 unit, rtk_port_t port, rtk_flowctrl_egrDropMode_t dropMode);

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropForceModeEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_egrPortDropForceModeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropForceModeEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      When enable force drop mode, this port will ignore flowcontrol result from autonegotiation
 */
extern int32
dal_esw_flowctrl_egrPortDropForceModeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_flowctrl_egrSystemDropThresh_get
 * Description:
 *      Get egress system drop threshold for the specified unit
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_egrSystemDropThresh_get(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_egrSystemDropThresh_set
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
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_egrSystemDropThresh_set(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropThresh_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_egrPortDropThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropThresh_set
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 high and low drop threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned.
 */
extern int32
dal_esw_flowctrl_egrPortDropThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_egrPortQueueDropThresh_get
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
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_QUEUE_ID           - invalid queue id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      1. For RTL8389/RTL8329, RT_ERR_CHIP_NOT_SUPPORTED will always be returned
 */
extern int32
dal_esw_flowctrl_egrPortQueueDropThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_egrPortQueueDropThresh_set
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
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_PORT_ID            - invalid port id
 *      RT_ERR_QUEUE_ID           - invalid queue id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      1. For RTL8389/RTL8329, RT_ERR_CHIP_NOT_SUPPORTED will always be returned
 */
extern int32
dal_esw_flowctrl_egrPortQueueDropThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_flowctrl_drop_thresh_t  *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropRefCongestEnable_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_egrPortDropRefCongestEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_flowctrl_egrPortDropRefCongestEnable_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_egrPortDropRefCongestEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/*
 * Flow Control OFF
 */
/* Function Name:
 *      dal_esw_flowctrl_igrSystemCongestThresh_get
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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_igrSystemCongestThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_esw_flowctrl_igrSystemCongestThresh_set
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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_flowctrl_igrSystemCongestThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

#endif  /* __DAL_ESW_FLOWCTRL_H__ */

