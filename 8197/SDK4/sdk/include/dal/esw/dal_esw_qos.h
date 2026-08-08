/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 10010 $
 * $Date: 2010-06-03 19:33:05 +0800 (Thu, 03 Jun 2010) $
 *
 * Purpose : Definition those public QoS APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Ingress priority decision
 *           2) Egress remark
 *           3) Queue scheduling
 *           4) Congestion avoidance
 */

#ifndef __DAL_ESW_QOS_H__
#define __DAL_ESW_QOS_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/qos.h>

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
 *      dal_esw_qos_init
 * Description:
 *      Configure QoS initial settings with queue number assigment to each port
 * Input:
 *      unit     - unit id
 *      queueNum - Queue number of each port, it is available at 1~8
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Note:
 *      This API will initialize related QoS setting with queue number assignment.
 *      The initialization does the following actions:
 *      1. set input bandwidth control parameters to default values
 *      2. set priority decision parameters
 *      3. set scheduling parameters
 *      4. disable port remark ability
 *      5. set flow control thresholds
 */
extern int32
dal_esw_qos_init(uint32 unit, uint32 queueNum);

/* Module Name    : QoS                       */
/* Sub-module Name: Ingress priority decision */

/* Function Name:
 *      dal_esw_qos_priSelGroup_get
 * Description:
 *      Get weight of each priority assignment on specified priority selection group.
 * Input:
 *      unit            - unit id
 *      grp_idx         - index of priority selection group
 * Output:
 *      pWeightOfPriSel - pointer to weight of each priority assignment
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_priSelGroup_get(uint32 unit, uint32 grp_idx, rtk_qos_priSelWeight_t *pWeightOfPriSel);

/* Function Name:
 *      dal_esw_qos_priSelGroup_set
 * Description:
 *      Set weight of each priority assignment on specified priority selection group.
 * Input:
 *      unit            - unit id
 *      grp_idx         - index of priority selection group
 *      pWeightOfPriSel - weight of each priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_priSelGroup_set(uint32 unit, uint32 grp_idx, rtk_qos_priSelWeight_t *pWeightOfPriSel);

/* Function Name:
 *      dal_esw_qos_portPriSelGroup_get
 * Description:
 *      Get priority selection group for specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pPriSelGrp_idx  - pointer to index of priority selection group
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
dal_esw_qos_portPriSelGroup_get(uint32 unit, rtk_port_t port, uint32 *pPriSelGrp_idx);

/* Function Name:
 *      dal_esw_qos_portPriSelGroup_set
 * Description:
 *      Set priority selection group for specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      priSelGrp_idx   - index of priority selection group
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
dal_esw_qos_portPriSelGroup_set(uint32 unit, rtk_port_t port, uint32 priSelGrp_idx);

/* Function Name:
 *      dal_esw_qos_portPri_get
 * Description:
 *      Get internal priority of one port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pInt_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                  the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_portPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pInt_pri);

/* Function Name:
 *      dal_esw_qos_portPri_set
 * Description:
 *      Set internal priority of one port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      int_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                 the highest prioirty)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Note:
 *    This API can set port to 3 bits internal priority mapping.
 *    When a packet is received from a port, a port based priority will be assigned
 *    by the mapping setting.
 *    By default, the mapping priorities for all ports are 0.
 */
extern int32
dal_esw_qos_portPri_set(uint32 unit, rtk_port_t port, rtk_pri_t int_pri);

/* Function Name:
 *      dal_esw_qos_portDp_get
 * Description:
 *      Get drop precedence of specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDp  - pointer to drop precedence
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
dal_esw_qos_portDp_get(uint32 unit, rtk_port_t port, uint32 *pDp);

/* Function Name:
 *      dal_esw_qos_portDp_set
 * Description:
 *      Set drop precedence of specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      dp   - drop precedence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      None
 */
extern int32
dal_esw_qos_portDp_set(uint32 unit, rtk_port_t port, uint32 dp);

/* Function Name:
 *      dal_esw_qos_portInnerPri_get
 * Description:
 *      Get priority of inner tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pPri - pointer to priority of inner tag
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
dal_esw_qos_portInnerPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPri);

/* Function Name:
 *      dal_esw_qos_portInnerPri_set
 * Description:
 *      Set priority of inner tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pri  - priority of inner tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_VLAN_PRIORITY    - invalid priority
 * Note:
 *      None
 */
extern int32
dal_esw_qos_portInnerPri_set(uint32 unit, rtk_port_t port, rtk_pri_t pri);

/* Function Name:
 *      dal_esw_qos_portOuterPri_get
 * Description:
 *      Get priority of outer tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pPri - pointer to priority of outer tag
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
dal_esw_qos_portOuterPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPri);

/* Function Name:
 *      dal_esw_qos_portOuterPri_set
 * Description:
 *      Set priority of outer tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pri  - priority of outer tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_VLAN_PRIORITY    - invalid priority
 * Note:
 *      None
 */
extern int32
dal_esw_qos_portOuterPri_set(uint32 unit, rtk_port_t port, rtk_pri_t pri);

/* Function Name:
 *      dal_esw_qos_portOuterDEI_get
 * Description:
 *      Get DEI of outer tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDEI - pointer to dei bit of outer tag
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
dal_esw_qos_portOuterDEI_get(uint32 unit, rtk_port_t port, uint32 *pDEI);

/* Function Name:
 *      dal_esw_qos_portOuterDEI_set
 * Description:
 *      Set DEI of outer tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      DEI  - dei bit of outer tag
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
dal_esw_qos_portOuterDEI_set(uint32 unit, rtk_port_t port, uint32 DEI);

/* Function Name:
 *      dal_esw_qos_dscpPriRemapGroup_get
 * Description:
 *      Get remapped internal priority of DSCP on specified DSCP remapping group.
 * Input:
 *      unit     - unit id
 *      grp_idx  - index of dscp remapping group
 *      dscp     - DSCP
 * Output:
 *      pInt_pri - pointer to internal priority
 *      pDp      - pointer to drop precedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE   - invalid DSCP value
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_dscpPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    uint32      dscp,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp);

/* Function Name:
 *      dal_esw_qos_dscpPriRemapGroup_set
 * Description:
 *      Set remapped internal priority of DSCP on specified DSCP remapping group.
 * Input:
 *      unit    - unit id
 *      grp_idx - index of dscp remapping group
 *      dscp    - DSCP
 *      int_pri - internal priority
 *      dp      - drop precedence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE   - invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_dscpPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    uint32      dscp,
    rtk_pri_t   int_pri,
    uint32      dp);

/* Function Name:
 *      dal_esw_qos_portDscpPriRemapGroup_get
 * Description:
 *      Get selected DSCP remapping group for specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pDscpGrp_idx - pointer to index of selected DSCP remapping group
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
dal_esw_qos_portDscpPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pDscpGrp_idx);

/* Function Name:
 *      dal_esw_qos_portDscpPriRemapGroup_set
 * Description:
 *      Set selected DSCP remapping group for specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      dscpGrp_idx - index of selected DSCP remapping group
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
dal_esw_qos_portDscpPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 dscpGrp_idx);

/* Function Name:
 *      dal_esw_qos_1pPriRemapGroup_get
 * Description:
 *      Get remapped internal priority of dot1p priority on specified dot1p priority remapping group.
 * Input:
 *      unit      - unit id
 *      grp_idx   - index of outer dot1p remapping group
 *      dot1p_pri - dot1p priority
 * Output:
 *      pInt_pri  - pointer to internal priority
 *      pDp       - pointer to drop precedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY - invalid dot1p priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_1pPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp);

/* Function Name:
 *      dal_esw_qos_1pPriRemapGroup_set
 * Description:
 *      Set remapped internal priority of dot1p priority on specified dot1p priority remapping group.
 * Input:
 *      unit      - unit id
 *      grp_idx   - index of dot1p remapping group
 *      dot1p_pri - dot1p priority
 *      int_pri   - internal priority
 *      dp        - drop precedence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_1pPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    rtk_pri_t   int_pri,
    uint32      dp);

/* Function Name:
 *      dal_esw_qos_port1pPriRemapGroup_get
 * Description:
 *      Get selected dot1p priority remapping group for specified port.
 * Input:
 *      unit             - unit id
 *      port             - port id
 * Output:
 *      pInnerPriGrp_idx - pointer to index of selected dot1p priority remapping group
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
dal_esw_qos_port1pPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pInnerPriGrp_idx);

/* Function Name:
 *      dal_esw_qos_port1pPriRemapGroup_set
 * Description:
 *      Set selected dot1p priority remapping group for specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      innerPriGrp_idx - index of selected dot1p priority remapping group
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
dal_esw_qos_port1pPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 innerPriGrp_idx);

/* Function Name:
 *      dal_esw_qos_outer1pPriRemapGroup_get
 * Description:
 *      Get remapped internal priority of outer dot1p priority
 *      on specified outer dot1p priority remapping group.
 * Input:
 *      unit      - unit id
 *      grp_idx   - index of outer dot1p remapping group
 *      dot1p_pri - dot1p priority
 *      dei       - DEI
 * Output:
 *      pInt_pri  - pointer to internal priority
 *      pDp       - pointer to drop precedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY - invalid dot1p priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_outer1pPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    uint32      dei,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp);

/* Function Name:
 *      dal_esw_qos_outer1pPriRemapGroup_set
 * Description:
 *      Set remapped internal priority of outer dot1p priority
 *      on specified outer dot1p priority remapping group.
 * Input:
 *      unit      - unit id
 *      grp_idx   - index of dot1p remapping group
 *      dot1p_pri - dot1p priority
 *      dei       - DEI
 *      int_pri   - internal priority
 *      dp        - drop precedence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_outer1pPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    uint32      dei,
    rtk_pri_t   int_pri,
    uint32      dp);

/* Function Name:
 *      dal_esw_qos_portOuter1pPriRemapGroup_get
 * Description:
 *      Get selected dot1p outer priority remapping group for specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pOuterPriGrp_idx - pointer to index of selected outer dot1p priority remapping group
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
dal_esw_qos_portOuter1pPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuterPriGrp_idx);

/* Function Name:
 *      dal_esw_qos_portOuter1pPriRemapGroup_set
 * Description:
 *      Set selected dot1p outer priority remapping group for specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      outerPriGrp_idx - index of selected outer dot1p priority remapping group
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
dal_esw_qos_portOuter1pPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 outerPriGrp_idx);

/* Function Name:
 *      dal_esw_qos_portPriMap_get
 * Description:
 *      Get the value of internal priority to QID mapping table on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      pri    - internal priority.
 * Output:
 *      pQueue - pointer to queue id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid internal priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_portPriMap_get(uint32 unit, rtk_port_t port, rtk_pri_t pri, rtk_qid_t *pQueue) ;

/* Function Name:
 *      dal_esw_qos_portPriMap_set
 * Description:
 *      Set the value of internal priority to QID mapping table on specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      pri   - internal priority.
 *      queue - queue id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid internal priority
 *      RT_ERR_QUEUE_ID         - Invalid queue ID
 * Note:
 *      None
 */
extern int32
dal_esw_qos_portPriMap_set(uint32 unit, rtk_port_t port, rtk_pri_t pri, rtk_qid_t queue);

/* Module Name    : QoS           */
/* Sub-module Name: Egress remark */

/* Function Name:
 *      dal_esw_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1p remark status for a port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of 802.1p remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of 802.1p remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_qos_1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1p remark status for a port
 * Input:
 *      unit   - unit id
 *      port   - port id.
 *      enable - status of 802.1p remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 * Note:
 *    The status of 802.1p remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_qos_1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_qos_1pRemarkGroup_get
 * Description:
 *      Get remarked dot1p priority of internal priority on specified dot1p remark group.
 * Input:
 *      unit       - unit id
 *      grp_idx    - index of dot1p remark group
 *      int_pri    - internal priority
 *      dp         - drop precedence
 * Output:
 *      pDot1p_pri - pointer to dot1p priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_1pRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   *pDot1p_pri);

/* Function Name:
 *      dal_esw_qos_1pRemarkGroup_set
 * Description:
 *      Set remarked dot1p priority of internal priority on specified dot1p remark group.
 * Input:
 *      unit      - unit id
 *      grp_idx   - index of dot1p remark group
 *      int_pri   - internal priority
 *      dp        - drop precedence
 *      dot1p_pri - dot1p priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_1pRemarkGroup_set(
    uint32 unit,
    uint32 grp_idx,
    rtk_pri_t int_pri,
    uint32 dp,
    rtk_pri_t dot1p_pri);

/* Function Name:
 *      dal_esw_qos_port1pRemarkGroup_get
 * Description:
 *      Get selected dot1p priority remarking group for specified port.
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:
 *      pInner1pRemarkGrp_idx - pointer to index of selected dot1p priority remarking group
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
dal_esw_qos_port1pRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pInner1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_port1pRemarkGroup_set
 * Description:
 *      Set selected dot1p priority remarking group for specified port.
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 *      inner1pRemarkGrp_idx - index of selected dot1p priority remarking group
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
dal_esw_qos_port1pRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 inner1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_port1pPriMapGroup_get
 * Description:
 *      Get dot1p priority remarking group of specified port
 *      when original packet is untag and remarking is disable.
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:
 *      pInner1pRemarkGrp_idx - pointer to index of selected dot1p priority remarking group
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
dal_esw_qos_port1pPriMapGroup_get(uint32 unit, rtk_port_t port, uint32 *pInner1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_port1pPriMapGroup_set
 * Description:
 *      Set dot1p priority remarking group of specified port
 *      when original packet is untag and remarking is disable.
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 *      inner1pRemarkGrp_idx - index of selected dot1p priority remarking group
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
dal_esw_qos_port1pPriMapGroup_set(uint32 unit, rtk_port_t port, uint32 inner1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_out1pRemarkEnable_get
 * Description:
 *      Get enable status of outer dot1p remarking on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id.
 * Output:
 *      pEnable - pointer to enable status of outer dot1p remarking
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
dal_esw_qos_out1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_out1pRemarkEnable_set
 * Description:
 *      Set enable status of outer dot1p remarking on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id.
 *      enable - enable status of outer dot1p remarking
 * Output:
 *      None.
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
dal_esw_qos_out1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_qos_outer1pRemarkGroup_get
 * Description:
 *      Get remarked outer dot1p priority of internal priority on specified outer dot1p remark group.
 * Input:
 *      unit       - unit id
 *      grp_idx    - index of dot1p outer remark group
 *      int_pri    - internal priority
 *      dp         - drop precedence
 * Output:
 *      pDot1p_pri - pointer to dot1p priority
 *      pDei       - pointer to dei bit of outer tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_outer1pRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   *pDot1p_pri,
    uint32      *pDei);

/* Function Name:
 *      dal_esw_qos_outer1pRemarkGroup_set
 * Description:
 *      Set remarked outer dot1p priority of internal priority on specified outer dot1p remark group.
 * Input:
 *      unit      - unit id
 *      grp_idx   - index of outer dot1p remark group
 *      int_pri   - internal priority
 *      dp        - drop precedence
 *      dot1p_pri - dot1p priority
 *      dei       - dei bit of outer tag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_outer1pRemarkGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   dot1p_pri,
    uint32      dei);

/* Function Name:
 *      dal_esw_qos_portOuter1pRemarkGroup_get
 * Description:
 *      Get selected outer dot1p priority remarking group for specified port.
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:
 *      pOuter1pRemarkGrp_idx - pointer to index of selected outer dot1p priority remarking group
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
dal_esw_qos_portOuter1pRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuter1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_portOuter1pRemarkGroup_set
 * Description:
 *      Set selected outer dot1p priority remarking group for specified port.
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 *      outer1pRemarkGrp_idx - index of selected outer dot1p priority remarking group
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
dal_esw_qos_portOuter1pRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 outer1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_portOuter1pPriMapGroup_get
 * Description:
 *      Get outer dot1p priority remarking group of specified port
 *      when original packet is untag and remarking is disable.
 * Input:
 *      unit                  - unit id
 *      port                  - port id
 * Output:
 *      pOuter1pRemarkGrp_idx - pointer to index of selected outer dot1p priority remarking group
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
dal_esw_qos_portOuter1pPriMapGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuter1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_portOuter1pPriMapGroup_set
 * Description:
 *      Set outer dot1p priority remarking group of specified port
 *      when original packet is untag and remarking is disable.
 * Input:
 *      unit                 - unit id
 *      port                 - port id
 *      outer1pRemarkGrp_idx - index of selected outer dot1p priority remarking group
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
dal_esw_qos_portOuter1pPriMapGroup_set(uint32 unit, rtk_port_t port, uint32 outer1pRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remark status for a port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of DSCP remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The status of DSCP remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_qos_dscpRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remark status for a port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of DSCP remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 * Note:
 *    The status of DSCP remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_esw_qos_dscpRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_qos_dscpRemarkGroup_get
 * Description:
 *      Get remarked DSCP of internal priority on specified dscp remark group.
 * Input:
 *      unit    - unit id
 *      grp_idx - index of dot1p remapping group
 *      int_pri - internal priority
 *      dp      - drop precedence
 * Output:
 *      pDscp   - pointer to DSCP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_dscpRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    uint32      *pDscp);

/* Function Name:
 *      dal_esw_qos_dscpRemarkGroup_set
 * Description:
 *      Set remarked DSCP of internal priority on specified dscp remark group.
 * Input:
 *      unit    - unit id
 *      grp_idx - index of dot1p remapping group
 *      int_pri - internal priority
 *      dp      - drop precedence
 *      dscp    - DSCP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE   - invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_dscpRemarkGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    uint32      dscp);

/* Function Name:
 *      dal_esw_qos_portdscpRemarkGroup_get
 * Description:
 *      Get selected DSCP remarking group for specified port.
 * Input:
 *      unit               - unit id
 *      port               - port id
 * Output:
 *      pDscpRemarkGrp_idx - pointer to index of selected DSCP remarking group
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
dal_esw_qos_portdscpRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pDscpRemarkGrp_idx);

/* Function Name:
 *      dal_esw_qos_portdscpRemarkGroup_set
 * Description:
 *      Set selected DSCP remarking group for specified port.
 * Input:
 *      unit              - unit id
 *      port              - port id
 *      dscpRemarkGrp_idx - index of selected DSCP remarking group
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
dal_esw_qos_portdscpRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 dscpRemarkGrp_idx);

/* Module Name    : QoS              */
/* Sub-module Name: Queue scheduling */

/* Function Name:
 *      dal_esw_qos_schedulingAlgorithm_get
 * Description:
 *      Get the scheduling algorithm of the port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pScheduling_type - type of scheduling algorithm.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The types of scheduling algorithm:
 *    - WFQ
 *    - WRR
 */
extern int32
dal_esw_qos_schedulingAlgorithm_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   *pScheduling_type);


/* Function Name:
 *      dal_esw_qos_schedulingAlgorithm_set
 * Description:
 *      Set the scheduling algorithm of the port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      scheduling_type - type of scheduling algorithm.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - Invalid unit id
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - Invalid scheduling algorithm type
 * Note:
 *    The types of scheduling algorithm:
 *    - WFQ
 *    - WRR
 */
extern int32
dal_esw_qos_schedulingAlgorithm_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   scheduling_type);

/* Function Name:
 *      dal_esw_qos_schedulingQueue_get
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *    The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *    If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
extern int32
dal_esw_qos_schedulingQueue_get(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

/* Function Name:
 *      dal_esw_qos_schedulingQueue_set
 * Description:
 *      Set the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      pQweights - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Note:
 *    The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *    If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
extern int32
dal_esw_qos_schedulingQueue_set(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

/* Function Name:
 *      dal_esw_qos_wfqFixedBandwidthEnable_get
 * Description:
 *      Get enable status of WFQ fixed bandwidth on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pEnable - pointer to enable status of WFQ fixed bandwidth
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_wfqFixedBandwidthEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable);

/* Function Name:
 *      dal_esw_qos_wfqFixedBandwidthEnable_set
 * Description:
 *      Set enable status of WFQ fixed bandwidth on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 *      enable  - enable status of WFQ fixed bandwidth
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_wfqFixedBandwidthEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable);

/* Module Name    : QoS              */
/* Sub-module Name: Congestion avoidance */

/* Function Name:
 *      dal_esw_qos_congAvoidAlgo_get
 * Description:
 *      Get algorithm of congestion avoidance.
 * Input:
 *      unit  - unit id
 * Output:
 *      pAlgo - pointer to algorithm of congestion avoidance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Algorithm is as following:
 *      - CONG_AVOID_WRED
 *      - CONG_AVOID_WTD
 */
extern int32
dal_esw_qos_congAvoidAlgo_get(uint32 unit, rtk_qos_congAvoidAlgo_t *pAlgo);

/* Function Name:
 *      dal_esw_qos_congAvoidAlgo_set
 * Description:
 *      Set algorithm of congestion avoidance.
 * Input:
 *      unit - unit id
 *      algo - algorithm of congestion avoidance
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Algorithm is as following:
 *      - CONG_AVOID_WRED
 *      - CONG_AVOID_WTD
 */
extern int32
dal_esw_qos_congAvoidAlgo_set(uint32 unit, rtk_qos_congAvoidAlgo_t algo);

/* Function Name:
 *      dal_esw_qos_congAvoidQueueThreshEnable_get
 * Description:
 *      Get enable status of queue threshold for congestion avoidance.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of queue threshold
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
dal_esw_qos_congAvoidQueueThreshEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_congAvoidQueueThreshEnable_set
 * Description:
 *      Set enable status of queue threshold for congestion avoidance.
 * Input:
 *      unit   - unit id
 *      enable - enable status of queue threshold
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
dal_esw_qos_congAvoidQueueThreshEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_qos_congAvoidPortThreshEnable_get
 * Description:
 *      Get enable status of port threshold for congestion avoidance.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of port threshold
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
dal_esw_qos_congAvoidPortThreshEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_congAvoidPortThreshEnable_set
 * Description:
 *      Set enable status of port threshold for congestion avoidance.
 * Input:
 *      unit   - unit id
 *      enable - enable status of port threshold
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
dal_esw_qos_congAvoidPortThreshEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_qos_congAvoidSysThreshEnable_get
 * Description:
 *      Get enable status of system threshold for congestion avoidance.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of system threshold
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
dal_esw_qos_congAvoidSysThreshEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_congAvoidSysThreshEnable_set
 * Description:
 *      Set enable status of system threshold for congestion avoidance.
 * Input:
 *      unit   - unit id
 *      enable - enable status of system threshold
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
dal_esw_qos_congAvoidSysThreshEnable_set(uint32 unit,  rtk_enable_t enable);

/* Function Name:
 *      dal_esw_qos_congAvoidSysThresh_get
 * Description:
 *      Get system threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      dp               - drop precedence
 * Output:
 *      pCongAvoidThresh - pointer to system threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_congAvoidSysThresh_get(
    uint32                      unit,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh);

/* Function Name:
 *      dal_esw_qos_congAvoidSysThresh_set
 * Description:
 *      Set system threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      dp               - drop precedence
 *      pCongAvoidThresh - system threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_congAvoidSysThresh_set(
    uint32                      unit,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh);

/* Function Name:
 *      dal_esw_qos_congAvoidPortThresh_get
 * Description:
 *      Get port threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      port             - port id
 *      dp               - drop precedence
 * Output:
 *      pCongAvoidThresh - pointer to port threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_congAvoidPortThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh);

/* Function Name:
 *      dal_esw_qos_congAvoidPortThresh_set
 * Description:
 *      Set port threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      port             - port id
 *      dp               - drop precedence
 *      pCongAvoidThresh - port threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_congAvoidPortThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh);

/* Function Name:
 *      dal_esw_qos_congAvoidQueueThresh_get
 * Description:
 *      Get queue threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      port             - port id
 *      queue            - queue id
 * Output:
 *      pCongAvoidThresh - pointer to queue threshold
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
dal_esw_qos_congAvoidQueueThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh);

/* Function Name:
 *      dal_esw_qos_congAvoidQueueThresh_set
 * Description:
 *      Set queue threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      port             - port id
 *      queue            - queue id
 *      pCongAvoidThresh - queue threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_congAvoidQueueThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh);

/*
 * Component: WRED
 */
/* Function Name:
 *      dal_esw_qos_wredSysThresh_get
 * Description:
 *      Get system threshold of WRED.
 * Input:
 *      unit             - unit id
 *      dp               - drop precedence
 * Output:
 *      pCongAvoidThresh - pointer to system threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_qos_wredSysThresh_get(uint32 unit, uint32 dp, rtk_qos_wredThresh_t *pThresh);

/* Function Name:
 *      dal_esw_qos_wredSysThresh_set
 * Description:
 *      Set system threshold of WRED.
 * Input:
 *      unit             - unit id
 *      dp               - drop precedence
 *      pCongAvoidThresh - system threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_qos_wredSysThresh_set(uint32 unit, uint32 dp, rtk_qos_wredThresh_t *pThresh);

/* Function Name:
 *      dal_esw_qos_wredWeight_get
 * Description:
 *      Get weight of WRED.
 * Input:
 *      unit    - unit id
 * Output:
 *      pWeight - pointer to weight of WRED
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
dal_esw_qos_wredWeight_get(uint32 unit, uint32 *pWeight);

/* Function Name:
 *      dal_esw_qos_wredWeight_set
 * Description:
 *      Set weight of WRED.
 * Input:
 *      unit   - unit id
 *      weight - weight of WRED
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
dal_esw_qos_wredWeight_set(uint32 unit, uint32 weight);

/* Function Name:
 *      dal_esw_qos_wredMpd_get
 * Description:
 *      Get MPD(Mark Probability Denominator) of WRED.
 * Input:
 *      unit - unit id
 * Output:
 *      pMpd - pointer to MPD of WRED
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
dal_esw_qos_wredMpd_get(uint32 unit, uint32 *pMpd);

/* Function Name:
 *      dal_esw_qos_wredMpd_set
 * Description:
 *      Set MPD(Mark Probability Denominator) of WRED.
 * Input:
 *      unit - unit id
 *      mpd  - MPD of WRED
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
dal_esw_qos_wredMpd_set(uint32 unit, uint32 mpd);

/* Function Name:
 *      dal_esw_qos_wredEcnEnable_get
 * Description:
 *      Get enable status of ECN for WRED.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of ECN
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
dal_esw_qos_wredEcnEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_wredEcnEnable_set
 * Description:
 *      Set enable status of ECN for WRED.
 * Input:
 *      unit   - unit id
 *      enable - enable status of ECN
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
dal_esw_qos_wredEcnEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_qos_wredCntReverseEnable_get
 * Description:
 *      Get enable status of counter reverse for WRED.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of counter reverse
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
dal_esw_qos_wredCntReverseEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_qos_wredCntReverseEnable_set
 * Description:
 *      Set enable status of counter reverse for WRED.
 * Input:
 *      unit   - unit id
 *      enable - enable status of counter reverse
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
dal_esw_qos_wredCntReverseEnable_set(uint32 unit, rtk_enable_t enable);


#endif /* __DAL_ESW_QOS_H__ */
