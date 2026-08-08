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
 * $Revision: 10951 $
 * $Date: 2010-07-15 10:32:24 +0800 (Thu, 15 Jul 2010) $
 *
 * Purpose : Definition those public QoS APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) Ingress priority decision
 *            2) Egress remark
 *            3) Queue scheduling
 *
 */

#ifndef __DAL_SSW_QOS_H__
#define __DAL_SSW_QOS_H__


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
 *      dal_ssw_qos_init
 * Description:
 *      Configure QoS initial settings with queue number assigment to each port
 * Input:
 *      unit                - unit id
 *      queueNum            - Queue number of each port, it is available at 1~8
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_QUEUE_NUM     - Invalid queue number
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
dal_ssw_qos_init(uint32 unit, uint32 queueNum);

/* Function Name:
 *      dal_ssw_qos_priSel_get
 * Description:
 *      Get the priority among different priority mechanism.
 * Input:
 *      unit                - unit id
 * Output:
 *      *pPort_pri          - Priority assign for port based selection (0~3). 3 is the highest
 *                            priority
 *      *pClass_pri         - Priority assign for classifier selection (0~3). 3 is the highest
 *                            priority
 *      *pAcl_pri           - Priority assign for ingress acl selection (0~3). 3 is the highest
 *                            priority
 *      *pDscp_pri          - Priority assign for dscp selection (0~3). 3 is the highest priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_priSel_get(uint32 unit,
                   rtk_pri_t *pPort_pri,
                   rtk_pri_t *pClass_pri,
                   rtk_pri_t *pAcl_pri,
                   rtk_pri_t *pDscp_pri);

/* Function Name:
 *      dal_ssw_qos_priSel_set
 * Description:
 *      Set the priority among different priority mechanism.
 * Input:
 *      unit                - unit id
 *      port_pri            - Priority assign for port based selection (0~3). 3 is the highest
 *                            priority
 *      class_pri           - Priority assign for classifier selection (0~3). 3 is the highest
 *                            priority
 *      acl_pri             - Priority assign for ingress acl selection (0~3). 3 is the highest
 *                            priority
 *      dscp_pri            - Priority assign for dscp selection (0~3). 3 is the highest priority
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_SEL_PORT_PRI      - Invalid port selection priority
 *      RT_ERR_QOS_SEL_CLASS_PRI     - Invalid classifier selection priority
 *      RT_ERR_QOS_SEL_IN_ACL_PRI    - Invalid ingress ACL selection priority
 *      RT_ERR_QOS_SEL_DSCP_PRI      - Invalid dscp selection priority
 * Note:
 *      1. ASIC will follow user priority setting of mechanisms to select mapped queue priority for
 *         receiving frame.
 *
 *      2. If more than one priorities of mechanisms are the same, ASIC will chose the highest
 *         priority from mechanisms to assign queue priority to receiving frame.
 *
 *      3. This API can set priority for four mechanisms :
 *         Port based priority, Classifier priority, Ingress ACL priority, and DSCP priority.
 */
extern int32
dal_ssw_qos_priSel_set(uint32 unit,
                   rtk_pri_t port_pri,
                   rtk_pri_t class_pri,
                   rtk_pri_t acl_pri,
                   rtk_pri_t dscp_pri);

/* Function Name:
 *      dal_ssw_qos_portPri_get
 * Description:
 *      Get internal priority of one port.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      *pInt_pri           - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                            the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_PORT_ID               - Invalid port id
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *    None.
 */
extern int32
dal_ssw_qos_portPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pInt_pri);

/* Function Name:
 *      dal_ssw_qos_portPri_set
 * Description:
 *      Set internal priority of one port.
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      int_pri             - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                            the highest prioirty)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_PORT_ID               - Invalid port id
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority value
 * Note:
 *    This API can set port to 3 bits internal priority mapping.
 *    When a packet is received from a port, a port based priority will be assigned
 *    by the mapping setting.
 *    By default, the mapping priorities for all ports are 0.
 */
extern int32
dal_ssw_qos_portPri_set(uint32 unit, rtk_port_t port, rtk_pri_t int_pri);

/* Function Name:
 *      dal_ssw_qos_dscpPriRemap_get
 * Description:
 *      Get the internal priority that DSCP value remap.
 * Input:
 *      unit                - unit id
 *      dscp                - DSCP value of receiving frame (0~63)
 * Output:
 *      *pInt_pri           - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                            the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE        - Invalid DSCP value
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_dscpPriRemap_get(uint32 unit, uint32 dscp, rtk_pri_t *pInt_pri);

/* Function Name:
 *      dal_ssw_qos_dscpPriRemap_set
 * Description:
 *      Set the internal priority that DSCP value remap.
 * Input:
 *      unit                - unit id
 *      dscp                - DSCP value of receiving frame (0~63)
 *      int_pri             - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                            the highest prioirty)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE        - Invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority value
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviours.
 *      As a selector, there is no implication that a numerically greater DSCP implies a better
 *      network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS.
 *      So if values of DSCP are carefully chosen then backward compatibility can be achieved.
 */
extern int32
dal_ssw_qos_dscpPriRemap_set(uint32 unit, uint32 dscp, rtk_pri_t int_pri);

/* Function Name:
 *      dal_ssw_qos_1pPriRemap_get
 * Description:
 *      Get the internal priority that 802.1p priority remap.
 * Input:
 *      unit                    - unit id
 *      dot1p_pri               - 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      *pInt_pri               - internal priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY       - Invalid 802.1p priority
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_1pPriRemap_get(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri);

/* Function Name:
 *      dal_ssw_qos_1pPriRemap_set
 * Description:
 *      Set the internal priority that 802.1p priority remap.
 * Input:
 *      unit                    - unit id
 *      dot1p_pri               - 802.1p priority value (range from 0 ~ 7)
 *      int_pri                 - internal priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY       - Invalid 802.1p priority
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority value
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_1pPriRemap_set(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t int_pri);

/* Function Name:
 *      dal_ssw_qos_queueNum_get
 * Description:
 *      Get the number of queue for the system.
 * Input:
 *      unit                    - unit id
 * Output:
 *      *pQueue_num             - the number of queue (1~8).
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - Invalid unit id
 *      RT_ERR_NULL_POINTER      - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_queueNum_get(uint32 unit, uint32 *pQueue_num);

/* Function Name:
 *      dal_ssw_qos_queueNum_set
 * Description:
 *      Set the number of queue for the system.
 * Input:
 *      unit                    - unit id
 *      queue_num               - the number of queue (1~8).
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QUEUE_NUM             - Invalid queue number
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_queueNum_set(uint32 unit, uint32 queue_num);

/* Function Name:
 *      dal_ssw_qos_priMap_get
 * Description:
 *      Get the value of internal priority to QID mapping table.
 * Input:
 *      unit                    - unit id
 *      queue_num               - the number of queue (1~8).
 * Output:
 *     	*pPri2qid               - array of internal priority on a queue
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QUEUE_NUM             - Invalid queue number
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_priMap_get(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid);

/* Function Name:
 *      dal_ssw_qos_priMap_set
 * Description:
 *      Set the value of internal priority to QID mapping table.
 * Input:
 *      unit                    - unit id
 *      queue_num               - the number of queue (1~8).
 *      *pPri2qid               - array of internal priority on a queue
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QUEUE_NUM             - Invalid queue number
 * Note:
 *      Below is an example of internal priority to QID mapping table.
 *      When queue numbers are 8, the pri2qid are pri2qid[0]=0, pri2qid[1]=1, pri2qid[2]=2..., etc.
 *
 *                  Number of Available Output Queue
 *        Priority  1   2   3   4   5   6   7   8
 *              0   0   0   0   0   0   0   0   0
 *              1   0   0   0   0   0   0   0   1
 *              2   0   0   0   1   1   1   1   2
 *              3   0   0   0   1   1   2   2   3
 *              4   0   1   1   2   2   3   3   4
 *              5   0   1   1   2   3   4   4   5
 *              6   0   1   2   3   4   5   5   6
 *              7   0   1   2   3   4   5   6   7
 */
extern int32
dal_ssw_qos_priMap_set(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid);

/* Function Name:
 *      dal_ssw_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1p remark status for a port
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      *pEnable    - status of 802.1p remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_PORT_ID               - Invalid port id
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *    The status of 802.1p remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_qos_1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1p remark status for a port
 * Input:
 *      unit        - unit id
 *      port        - port id.
 *      enable      - status of 802.1p remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_PORT_ID   - Invalid port id
 *      RT_ERR_INPUT     - invalid input parameter
 * Note:
 *    The status of 802.1p remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_qos_1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_qos_1pRemark_get
 * Description:
 *      Get the internal priority (3bits) to remarkd 802.1p priority(3bits) mapping.
 * Input:
 *      unit                - unit id
 *      int_pri             - internal priority value (range from 0 ~ 7)
 * Output:
 *      *pDot1p_pri         - remarked 802.1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_1pRemark_get(uint32 unit, rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri);

/* Function Name:
 *      dal_ssw_qos_1pRemark_set
 * Description:
 *      Set the internal priority(3bits) to remarked 802.1p priority(3bits) mapping.
 * Input:
 *      unit                - unit id
 *      int_pri             - internal priority value (range from 0 ~ 7)
 *      dot1p_pri           - remarked 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_1P_PRIORITY       - Invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority
 * Note:
 *      802.1p remark functionality can map the internal priority to 802.1p priority before a packet
 *      is going to be transmited.
 */
extern int32
dal_ssw_qos_1pRemark_set(uint32 unit, rtk_pri_t int_pri, rtk_pri_t dot1p_pri);

/* Function Name:
 *      dal_ssw_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remark status for a port
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      *pEnable            - status of DSCP remark
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The status of DSCP remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_qos_dscpRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remark status for a port
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      enable          - status of DSCP remark
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_PORT_ID   - Invalid port id
 *      RT_ERR_INPUT     - invalid input parameter
 * Note:
 *    The status of DSCP remark:
 *    - DISABLED
 *    - ENABLED
 */
extern int32
dal_ssw_qos_dscpRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_qos_dscpRemark_get
 * Description:
 *      Get the internal priority (3bits) to remarked DSCP mapping.
 * Input:
 *      unit                - unit id
 *      int_pri             - internal priority value (range from 0 ~ 7)
 * Output:
 *      *pDscp              - remarked DSCP value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority
 *      RT_ERR_NULL_POINTER          - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_qos_dscpRemark_get(uint32 unit, rtk_pri_t int_pri, uint32 *pDscp);

/* Function Name:
 *      dal_ssw_qos_dscpRemark_set
 * Description:
 *      Set the internal priority (3bits) to remarked DSCP mapping.
 * Input:
 *      unit                - unit id
 *      int_pri             - internal priority value (range from 0 ~ 7)
 *      dscp                - remarked DSCP value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - Invalid unit id
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority
 *      RT_ERR_QOS_DSCP_VALUE        - Invalid dscp value
 * Note:
 *      DSCP remark functionality can map the internal priority to DSCP before a packet is going
 *      to be transmited.
 */
extern int32
dal_ssw_qos_dscpRemark_set(uint32 unit, rtk_pri_t int_pri, uint32 dscp);

/* Function Name:
 *      dal_ssw_qos_schedulingAlgorithm_get
 * Description:
 *      Get the scheduling algorithm of the port.
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      *pScheduling_type   - type of scheduling algorithm.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *    The types of scheduling algorithm:
 *    - WFQ
 *    - WRR
 */
extern int32
dal_ssw_qos_schedulingAlgorithm_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   *pScheduling_type);

/* Function Name:
 *      dal_ssw_qos_schedulingAlgorithm_set
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
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_PORT_ID       - Invalid port id
 *      RT_ERR_QOS_SCHE_TYPE - Error scheduling algorithm type
 * Note:
 *    The types of scheduling algorithm:
 *    - WFQ
 *    - WRR
 */
extern int32
dal_ssw_qos_schedulingAlgorithm_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   scheduling_type);

/* Function Name:
 *      dal_ssw_qos_schedulingQueue_get
 * Description:
 *      Get the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      *pQweights  - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - Invalid unit id
 *      RT_ERR_PORT_ID           - Invalid port id
 *      RT_ERR_NULL_POINTER      - NULL pointer
 * Note:
 *    The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *    If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
extern int32
dal_ssw_qos_schedulingQueue_get(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

/* Function Name:
 *      dal_ssw_qos_schedulingQueue_set
 * Description:
 *      Set the scheduling types and weights of queues on specific port in egress scheduling.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      *pQweights  - the array of weights for WRR/WFQ queue (valid:1~128, 0 for STRICT_PRIORITY queue)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - Invalid unit id
 *      RT_ERR_PORT_ID           - Invalid port id
 *      RT_ERR_NULL_POINTER      - Null pointer
 *      RT_ERR_QOS_QUEUE_WEIGHT	- Invalid queue weight
 * Note:
 *    The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *    If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
extern int32
dal_ssw_qos_schedulingQueue_set(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

#endif /* __DAL_SSW_QOS_H__ */
