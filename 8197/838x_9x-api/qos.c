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
 * $Revision: 34041 $
 * $Date: 2012-11-05 14:34:13 +0800 (Mon, 05 Nov 2012) $
 *
 * Purpose : Definition of QoS API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Ingress Priority Decision
 *           (2) Egress Remarking
 *           (3) Queue Scheduling
 *           (4) Congestion avoidance
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/qos.h>
#include <rtk/default.h>

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
 *      rtk_qos_init
 * Description:
 *      Configure QoS initial settings with queue number assigment to each port
 * Input:
 *      unit     - unit id
 *      queueNum - Queue number of each port, ranges from 1~8
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) This API will initialize related QoS setting with queue number assignment.
 *      (2) The initialization does the following actions:
 *          - set input bandwidth control parameters to default values
 *          - set priority decision parameters
 *          - set scheduling parameters
 *          - disable port remark ability
 *          - set flow control thresholds
 */
int32
rtk_qos_init(uint32 unit, uint32 queueNum)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_init(unit, queueNum);
} /* end of rtk_qos_init */

/* Module Name    : QoS                       */
/* Sub-module Name: Ingress priority decision */

/* Function Name:
 *      rtk_qos_priSel_get
 * Description:
 *      Get the priority among different priority mechanism.
 * Input:
 *      unit       - unit id
 * Output:
 *      pPort_pri  - Priority assign for port based selection.
 *      pClass_pri - Priority assign for classifier selection.
 *      pAcl_pri   - Priority assign for ingress acl selection.
 *      pDscp_pri  - Priority assign for dscp selection.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of port_pri, class_pri, acl_pri and dscp_pri is 0~3
 *      (2) The priority value 3 is mean the highest priority
 */
int32
rtk_qos_priSel_get(
    uint32      unit,
    rtk_pri_t   *pPort_pri,
    rtk_pri_t   *pClass_pri,
    rtk_pri_t   *pAcl_pri,
    rtk_pri_t   *pDscp_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_priSel_get(unit, pPort_pri, pClass_pri, pAcl_pri, pDscp_pri);
} /* end of rtk_qos_priSel_get */

/* Function Name:
 *      rtk_qos_priSel_set
 * Description:
 *      Set the priority among different priority mechanism.
 * Input:
 *      unit      - unit id
 *      port_pri  - Priority assign for port based selection.
 *      class_pri - Priority assign for classifier selection.
 *      acl_pri   - Priority assign for ingress acl selection.
 *      dscp_pri  - Priority assign for dscp selection.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - Invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_QOS_SEL_PORT_PRI   - Invalid port selection priority
 *      RT_ERR_QOS_SEL_CLASS_PRI  - Invalid classifier selection priority
 *      RT_ERR_QOS_SEL_IN_ACL_PRI - Invalid ingress ACL selection priority
 *      RT_ERR_QOS_SEL_DSCP_PRI   - Invalid dscp selection priority
 * Applicable:
 *      8389
 * Note:
 *      (1) The valid range of port_pri, class_pri, acl_pri and dscp_pri is 0~3
 *      (2) The priority value 3 is mean the highest priority
 *      (3) ASIC will follow user priority setting of mechanisms to select mapped queue priority for
 *          receiving frame.
 *      (4) If more than one priorities of mechanisms are the same, ASIC will choice the highest
 *          priority from mechanisms to assign queue priority to receiving frame.
 *      (5) This API can set priority for four mechanisms :
 *          Port based priority, Classifier priority, Ingress ACL priority, and DSCP priority.
 */
int32
rtk_qos_priSel_set(
    uint32      unit,
    rtk_pri_t   port_pri,
    rtk_pri_t   class_pri,
    rtk_pri_t   acl_pri,
    rtk_pri_t   dscp_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_priSel_set(unit, port_pri, class_pri, acl_pri, dscp_pri);
} /* end of rtk_qos_priSel_set */

/* Function Name:
 *      rtk_qos_priSelGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Each port can bind a priority selection group through 'rtk_qos_portPriSelGroup_set'.
 *      (2) The valid range of grp_idx is 0~7 in 8328.
 *      (3) The valid range of grp_idx is 0~3 in 8390 & 8380.
 */
int32
rtk_qos_priSelGroup_get(uint32 unit, uint32 grp_idx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_priSelGroup_get(unit, grp_idx, pWeightOfPriSel);
} /* end of rtk_qos_priSelGroup_get */

/* Function Name:
 *      rtk_qos_priSelGroup_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Each port can bind a priority selection group through 'rtk_qos_portPriSelGroup_set'.
 *      (2) The valid range of grp_idx is 0~7 in 8328.
 *      (3) The valid range of grp_idx is 0~3 in 8390 & 8380.
 */
int32
rtk_qos_priSelGroup_set(uint32 unit, uint32 grp_idx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_priSelGroup_set(unit, grp_idx, pWeightOfPriSel);
} /* end of rtk_qos_priSelGroup_set */

/* Function Name:
 *      rtk_qos_portPriSelGroup_get
 * Description:
 *      Get priority selection group binding for specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      pPriSelGrp_idx - pointer to index of priority selection group
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
 *      (1) Priority selection group is configured by 'rtk_qos_priSelGroup_set'.
 *      (2) The valid range of priSelGrp_idx is 0~7 in 8328.
 *      (3) The valid range of priSelGrp_idx is 0~3 in 8390 & 8380.
 */
int32
rtk_qos_portPriSelGroup_get(uint32 unit, rtk_port_t port, uint32 *pPriSelGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portPriSelGroup_get(unit, phy_port, pPriSelGrp_idx);
    }
#else
    return RT_MAPPER(unit)->qos_portPriSelGroup_get(unit, port, pPriSelGrp_idx);
#endif
} /* end of rtk_qos_portPriSelGroup_get */

/* Function Name:
 *      rtk_qos_portPriSelGroup_set
 * Description:
 *      Set priority selection group binding for specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      priSelGrp_idx - index of priority selection group
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
 *      (1) Priority selection group is configured by 'rtk_qos_priSelGroup_set'.
 *      (2) The valid range of priSelGrp_idx is 0~7 in 8328.
 *      (3) The valid range of priSelGrp_idx is 0~3 in 8390 & 8380.
 */
int32
rtk_qos_portPriSelGroup_set(uint32 unit, rtk_port_t port, uint32 priSelGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portPriSelGroup_set(unit, phy_port, priSelGrp_idx);
    }
#else
    return RT_MAPPER(unit)->qos_portPriSelGroup_set(unit, port, priSelGrp_idx);
#endif
} /* end of rtk_qos_portPriSelGroup_set */

/* Function Name:
 *      rtk_qos_portPri_get
 * Description:
 *      Get internal priority of specific port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pInt_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                 the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390
 * Note:
 *      None
 */
int32
rtk_qos_portPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pInt_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPri_get(unit, port, pInt_pri);
} /* end of rtk_qos_portPri_get */

/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Set internal priority of specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      int_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                the highest prioirty)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Applicable:
 *      8389, 8328, 8390
 * Note:
 *      (1) This API can set port to 3 bits internal priority mapping.
 *      (2) When a packet is received from a port, a port based priority will be assigned
 *          by the mapping setting.
 *      (3) By default, the mapping priorities for all ports are 0.
 */
int32
rtk_qos_portPri_set(uint32 unit, rtk_port_t port, rtk_pri_t int_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPri_set(unit, port, int_pri);
} /* end of rtk_qos_portPri_set */

/* Function Name:
 *      dal_cypress_qos_portPriRemapEnable_get
 * Description:
 *      Get status of port-based priority remapping.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable   - status of port-based priority remapping
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *    None.
 */
int32
rtk_qos_portPriRemapEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPriRemapEnable_get(unit, pEnable);
} /* end of rtk_qos_portPriRemapEnable_get */

/* Function Name:
 *      dal_cypress_qos_portPriRemapEnable_set
 * Description:
 *      Set status of port-based priority remapping.
 * Input:
 *      unit      - unit id
 *      pEnable   - status of port-based priority remapping
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32 
rtk_qos_portPriRemapEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPriRemapEnable_set(unit, enable);
} /* end of rtk_qos_portPriRemapEnable_set */

/* Function Name:
 *      dal_cypress_qos_portPriRemapSel_get
 * Description:
 *      Get port-based priority remapping table.
 * Input:
 *      unit    - unit id
 *      pType   - remapping table selection
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_portPriRemapSel_get(uint32 unit, rtk_qos_portPriRemapSel_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPriRemapSel_get(unit, pType);
} /* end of rtk_qos_portPriRemapSel_get */

/* Function Name:
 *      dal_cypress_qos_portPriRemapEnable_set
 * Description:
 *      Set port-based priority remapping table.
 * Input:
 *      unit    - unit id
 *      type    - remapping table selection
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_portPriRemapSel_set(uint32 unit, rtk_qos_portPriRemapSel_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPriRemapSel_set(unit, type);
} /* end of rtk_qos_portPriRemapSel_set */

/* Function Name:
 *      rtk_qos_portInnerPri_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8380
 * Note:
 *      None
 */
int32
rtk_qos_portInnerPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portInnerPri_get(unit, phy_port, pPri);
    }
#else
    return RT_MAPPER(unit)->qos_portInnerPri_get(unit, port, pPri);
#endif
} /* end of rtk_qos_portInnerPri_get */

/* Function Name:
 *      rtk_qos_portInnerPri_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_VLAN_PRIORITY - invalid priority
 * Applicable:
 *      8328, 8380
 * Note:
 *      None
 */
int32
rtk_qos_portInnerPri_set(uint32 unit, rtk_port_t port, rtk_pri_t pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portInnerPri_set(unit, phy_port, pri);
    }
#else
    return RT_MAPPER(unit)->qos_portInnerPri_set(unit, port, pri);
#endif
} /* end of rtk_qos_portInnerPri_set */

/* Function Name:
 *      rtk_qos_portDp_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_portDp_get(uint32 unit, rtk_port_t port, uint32 *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDp_get(unit, port, pDp);
} /* end of rtk_qos_portDp_get */

/* Function Name:
 *      rtk_qos_portDp_set
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_portDp_set(uint32 unit, rtk_port_t port, uint32 dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDp_set(unit, port, dp);
} /* end of rtk_qos_portDp_set */

/* Function Name:
 *      rtk_qos_dpSrcSel_get
 * Description:
 *      Get drop precedence source.
 * Input:
 *      unit  - unit id
 * Output:
 *      pType - DP mapping source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The mapping of DEI -> DP and DSCP -> DP are set through 'rtk_qos_deiDpRemap_set' and
 *      'rtk_qos_dscpDpRemap_set' respectively.
 */
int32
rtk_qos_dpSrcSel_get(uint32 unit, rtk_qos_dpSrc_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dpSrcSel_get(unit, pType);
} /* end of rtk_qos_dpSrcSel_get */

/* Function Name:
 *      rtk_qos_dpSrcSel_set
 * Description:
 *      Set drop precedence source.
 * Input:
 *      unit - unit id
 *      type - DP mapping source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      The mapping of DEI -> DP and DSCP -> DP are set through 'rtk_qos_deiDpRemap_set' and
 *      'rtk_qos_dscpDpRemap_set' respectively.
 */
int32
rtk_qos_dpSrcSel_set(uint32 unit, rtk_qos_dpSrc_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dpSrcSel_set(unit, type);
} /* end of rtk_qos_dpSrcSel_set */

/* Function Name:
 *      rtk_qos_portDEISrcSel_get
 * Description:
 *      Get DEI source of specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pType  - DEI source
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
 *      Each port can specify the DEI is either from inner-tag or outer-tag.
 *      DEI can be the DP mapping source through configure 'rtk_qos_dpSrcSel_set'.
 */
int32
rtk_qos_portDEISrcSel_get(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDEISrcSel_get(unit, port, pType);
} /* end of rtk_qos_portDEISrcSel_get */

/* Function Name:
 *      rtk_qos_portDEISrcSel_set
 * Description:
 *      Set DEI source of specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      type - DEI source
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
 *      8390
 * Note:
 *      Each port can specify the DEI is either from inner-tag or outer-tag.
 *      DEI can be the DP mapping source through configure 'rtk_qos_dpSrcSel_set'.
 */
int32
rtk_qos_portDEISrcSel_set(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDEISrcSel_set(unit, port, type);
} /* end of rtk_qos_portDEISrcSel_set */

/* Function Name:
 *      rtk_qos_deiDpRemap_get
 * Description:
 *      Get DEI mapping to drop precedence.
 * Input:
 *      unit - unit id
 *      dei  - DEI
 * Output:
 *      pDp  - pointer to drop precedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The API can get configuration of DEI to DP remapping table.
 */
int32
rtk_qos_deiDpRemap_get(uint32 unit, uint32 dei, uint32 *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_deiDpRemap_get(unit, dei, pDp);
} /* end of rtk_qos_deiDpRemap_get */

/* Function Name:
 *      rtk_qos_deiDpRemap_set
 * Description:
 *      Set DEI mapping to drop precedence.
 * Input:
 *      unit - unit id
 *      dei  - DEI
 *      dp   - drop precedence
 * Output:
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_INPUT           - invalid input parameter
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 * Applicable:
 *      8390
 * Note:
 *      The API can configure DEI to DP remapping table.
 */
int32
rtk_qos_deiDpRemap_set(uint32 unit, uint32 dei, uint32 dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_deiDpRemap_set(unit, dei, dp);
} /* end of rtk_qos_deiDpRemap_set */

/* Function Name:
 *      rtk_qos_dscpDpRemap_get
 * Description:
 *      Get DSCP mapping to drop precedence.
 * Input:
 *      unit - unit id
 *      dscp - DSCP value of receiving frame (0~63)
 * Output:
 *      pDp  - pointer to drop precedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - Invalid DSCP value
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The API can get configuration of DSCP to DP remapping table.
 */
int32
rtk_qos_dscpDpRemap_get(uint32 unit, uint32 dscp, uint32 *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpDpRemap_get(unit, dscp, pDp);
} /* end of rtk_qos_dscpDpRemap_get */

/* Function Name:
 *      rtk_qos_dscpDpRemap_set
 * Description:
 *      Set DSCP mapping to drop precedence.
 * Input:
 *      unit - unit id
 *      dscp - DSCP value of receiving frame (0~63)
 *      dp   - drop precedence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE  - Invalid DSCP value
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 * Applicable:
 *      8390
 * Note:
 *      The API can configure DSCP to DP remapping table.
 */
int32
rtk_qos_dscpDpRemap_set(uint32 unit, uint32 dscp, uint32 dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpDpRemap_set(unit, dscp, dp);
} /* end of rtk_qos_dscpDpRemap_set */

/* Function Name:
 *      rtk_qos_portOuterPri_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8380
 * Note:
 *      None
 */
int32
rtk_qos_portOuterPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portOuterPri_get(unit, phy_port, pPri);
    }
#else
    return RT_MAPPER(unit)->qos_portOuterPri_get(unit, port, pPri);
#endif
} /* end of rtk_qos_portOuterPri_get */

/* Function Name:
 *      rtk_qos_portOuterPri_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_VLAN_PRIORITY - invalid priority
 * Applicable:
 *      8328, 8380
 * Note:
 *      None
 */
int32
rtk_qos_portOuterPri_set(uint32 unit, rtk_port_t port, rtk_pri_t pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portOuterPri_set(unit, phy_port, pri);
    }
#else
    return RT_MAPPER(unit)->qos_portOuterPri_set(unit, port, pri);
#endif
} /* end of rtk_qos_portOuterPri_set */

/* Function Name:
 *      rtk_qos_portOuterDEI_get
 * Description:
 *      Get DEI of outer tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDei - pointer to dei bit of outer tag
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
int32
rtk_qos_portOuterDEI_get(uint32 unit, rtk_port_t port, uint32 *pDei)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuterDEI_get(unit, port, pDei);
} /* end of rtk_qos_portOuterDEI_get */

/* Function Name:
 *      rtk_qos_portOuterDEI_set
 * Description:
 *      Set DEI of outer tag on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      dei  - dei bit of outer tag
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
int32
rtk_qos_portOuterDEI_set(uint32 unit, rtk_port_t port, uint32 dei)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuterDEI_set(unit, port, dei);
} /* end of rtk_qos_portOuterDEI_set */

/* Function Name:
 *      rtk_qos_dscpPriRemap_get
 * Description:
 *      Get the internal priority that DSCP value remap.
 * Input:
 *      unit     - unit id
 *      dscp     - DSCP value of receiving frame (0~63)
 * Output:
 *      pInt_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                  the highest prioirty)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - Invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - Invalid DSCP value
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_dscpPriRemap_get(uint32 unit, uint32 dscp, rtk_pri_t *pInt_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpPriRemap_get(unit, dscp, pInt_pri);
} /* end of rtk_qos_dscpPriRemap_get */

/* Function Name:
 *      rtk_qos_dscpPriRemap_set
 * Description:
 *      Set the internal priority that DSCP value remap.
 * Input:
 *      unit    - unit id
 *      dscp    - DSCP value of receiving frame (0~63)
 *      int_pri - Priorities assigment for specific port. (range from 0 ~ 7, 7 is
 *                the highest prioirty)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviours.
 *      As a selector, there is no implication that a numerically greater DSCP implies a better
 *      network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS.
 *      So if values of DSCP are carefully chosen then backward compatibility can be achieved.
 */
int32
rtk_qos_dscpPriRemap_set(uint32 unit, uint32 dscp, rtk_pri_t int_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpPriRemap_set(unit, dscp, int_pri);
} /* end of rtk_qos_dscpPriRemap_set */

/* Function Name:
 *      rtk_qos_dscpPriRemapGroup_get
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
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - invalid DSCP value
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_INPUT          - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~3
 */
int32
rtk_qos_dscpPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    uint32      dscp,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpPriRemapGroup_get(unit, grp_idx, dscp, pInt_pri, pDp);
} /* end of rtk_qos_dscpPriRemapGroup_get */

/* Function Name:
 *      rtk_qos_dscpPriRemapGroup_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE   - invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~3
 */
int32
rtk_qos_dscpPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    uint32      dscp,
    rtk_pri_t   int_pri,
    uint32      dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpPriRemapGroup_set(unit, grp_idx, dscp, int_pri, dp);
} /* end of rtk_qos_dscpPriRemapGroup_set */

/* Function Name:
 *      rtk_qos_portDscpPriRemapGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of dscpGrp_idx is 0~3
 */
int32
rtk_qos_portDscpPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pDscpGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDscpPriRemapGroup_get(unit, port, pDscpGrp_idx);
} /* end of rtk_qos_portDscpPriRemapGroup_get */

/* Function Name:
 *      rtk_qos_portDscpPriRemapGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of dscpGrp_idx is 0~3
 */
int32
rtk_qos_portDscpPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 dscpGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDscpPriRemapGroup_set(unit, port, dscpGrp_idx);
} /* end of rtk_qos_portDscpPriRemapGroup_set */

/* Function Name:
 *      rtk_qos_1pPriRemap_get
 * Description:
 *      Get the internal priority that 802.1p priority remap.
 * Input:
 *      unit      - unit id
 *      dot1p_pri - 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      pInt_pri  - internal priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - Invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY - Invalid 802.1p priority
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_1pPriRemap_get(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pPriRemap_get(unit, dot1p_pri, pInt_pri);
} /* end of rtk_qos_1pPriRemap_get */

/* Function Name:
 *      rtk_qos_1pPriRemap_set
 * Description:
 *      Set the internal priority that 802.1p priority remap.
 * Input:
 *      unit      - unit id
 *      dot1p_pri - 802.1p priority value (range from 0 ~ 7)
 *      int_pri   - internal priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid 802.1p priority
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_1pPriRemap_set(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t int_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pPriRemap_set(unit, dot1p_pri, int_pri);
} /* end of rtk_qos_1pPriRemap_set */

/* Function Name:
 *      rtk_qos_outer1pPriRemap_get
 * Description:
 *      Get the internal priority that outer 802.1p priority remap.
 * Input:
 *      unit      - unit id
 *      dot1p_pri - 802.1p priority value (range from 0 ~ 7)
 *      dei       - DEI
 * Output:
 *      pInt_pri  - internal priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - Invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY - Invalid 802.1p priority
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      DEI 0,1 can have different outer 1P priority to internal priority mapping.
 */
int32
rtk_qos_outer1pPriRemap_get(uint32 unit, rtk_pri_t dot1p_pri, uint32 dei, rtk_pri_t *pInt_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pPriRemap_get(unit, dot1p_pri, dei, pInt_pri);
} /* end of rtk_qos_outer1pPriRemap_get */

/* Function Name:
 *      rtk_qos_outer1pPriRemap_set
 * Description:
 *      Set the internal priority that outer 802.1p priority remap.
 * Input:
 *      unit      - unit id
 *      dot1p_pri - 802.1p priority value (range from 0 ~ 7)
 *      dei       - DEI
 *      int_pri   - internal priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid 802.1p priority
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 *      RT_ERR_QOS_DEI_VALUE    - invalid dei
 * Applicable:
 *      8390, 8380
 * Note:
 *      DEI 0,1 can have different outer 1P priority to internal priority mapping.
 */
int32
rtk_qos_outer1pPriRemap_set(uint32 unit, rtk_pri_t dot1p_pri, uint32 dei, rtk_pri_t int_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pPriRemap_set(unit, dot1p_pri, dei, int_pri);
} /* end of rtk_qos_outer1pPriRemap_set */

/* Function Name:
 *      rtk_qos_1pPriRemapGroup_get
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_INPUT           - invalid input parameter
 *      RT_ERR_QOS_1P_PRIORITY - invalid dot1p priority
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~3
 */
int32
rtk_qos_1pPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pPriRemapGroup_get(unit, grp_idx, dot1p_pri, pInt_pri, pDp);
} /* end of rtk_qos_1pPriRemapGroup_get */

/* Function Name:
 *      rtk_qos_1pPriRemapGroup_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~3
 */
int32
rtk_qos_1pPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    rtk_pri_t   int_pri,
    uint32      dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pPriRemapGroup_set(unit, grp_idx, dot1p_pri, int_pri, dp);
} /* end of rtk_qos_1pPriRemapGroup_set */

/* Function Name:
 *      rtk_qos_port1pPriRemapGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of innerPriGrp_idx is 0~3
 */
int32
rtk_qos_port1pPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pInnerPriGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_port1pPriRemapGroup_get(unit, port, pInnerPriGrp_idx);
} /* end of rtk_qos_port1pPriRemapGroup_get */

/* Function Name:
 *      rtk_qos_port1pPriRemapGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of innerPriGrp_idx is 0~3
 */
int32
rtk_qos_port1pPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 innerPriGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_port1pPriRemapGroup_set(unit, port, innerPriGrp_idx);
} /* end of rtk_qos_port1pPriRemapGroup_set */

/* Function Name:
 *      rtk_qos_outer1pPriRemapGroup_get
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY - invalid dot1p priority
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~3
 */
int32
rtk_qos_outer1pPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    uint32      dei,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pPriRemapGroup_get(unit, grp_idx, dot1p_pri, dei, pInt_pri, pDp);
} /* end of rtk_qos_outer1pPriRemapGroup_get */

/* Function Name:
 *      rtk_qos_outer1pPriRemapGroup_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~3
 */
int32
rtk_qos_outer1pPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    uint32      dei,
    rtk_pri_t   int_pri,
    uint32      dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pPriRemapGroup_set(unit, grp_idx, dot1p_pri, dei, int_pri, dp);
} /* end of rtk_qos_outer1pPriRemapGroup_set */

/* Function Name:
 *      rtk_qos_portOuter1pPriRemapGroup_get
 * Description:
 *      Get selected dot1p outer priority remapping group for specified port.
 * Input:
 *      unit             - unit id
 *      port             - port id
 * Output:
 *      pOuterPriGrp_idx - pointer to index of selected outer dot1p priority remapping group
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
 *      (1) The valid range of outerPriGrp_idx is 0~3
 */
int32
rtk_qos_portOuter1pPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuterPriGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuter1pPriRemapGroup_get(unit, port, pOuterPriGrp_idx);
} /* end of rtk_qos_portOuter1pPriRemapGroup_get */

/* Function Name:
 *      rtk_qos_portOuter1pPriRemapGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of outerPriGrp_idx is 0~3
 */
int32
rtk_qos_portOuter1pPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 outerPriGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuter1pPriRemapGroup_set(unit, port, outerPriGrp_idx);
} /* end of rtk_qos_portOuter1pPriRemapGroup_set */

/* Function Name:
 *      rtk_qos_queueNum_get
 * Description:
 *      Get the number of queue for the system.
 * Input:
 *      unit       - unit id
 * Output:
 *      pQueue_num - the number of queue (1~8).
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8390
 * Note:
 *      (1) The valid range of queue_num is 1~8
 */
int32
rtk_qos_queueNum_get(uint32 unit, uint32 *pQueue_num)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_queueNum_get(unit, pQueue_num);
} /* end of rtk_qos_queueNum_get */

/* Function Name:
 *      rtk_qos_queueNum_set
 * Description:
 *      Set the number of queue for the system.
 * Input:
 *      unit      - unit id
 *      queue_num - the number of queue (1~8).
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_QUEUE_NUM - Invalid queue number
 * Applicable:
 *      8389, 8390
 * Note:
 *      (1) The valid range of queue_num is 1~8
 */
int32
rtk_qos_queueNum_set(uint32 unit, uint32 queue_num)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_queueNum_set(unit, queue_num);
} /* end of rtk_qos_queueNum_set */

/* Function Name:
 *      rtk_qos_priMap_get
 * Description:
 *      Get the value of internal priority to QID mapping table.
 * Input:
 *      unit      - unit id
 *      queue_num - the number of queue (1~8).
 * Output:
 *      pPri2qid  - array of internal priority on a queue
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QUEUE_NUM    - Invalid queue number
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) The valid range of queue_num is 1~8
 */
int32
rtk_qos_priMap_get(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_priMap_get(unit, queue_num, pPri2qid);
} /* end of rtk_qos_priMap_get */

/* Function Name:
 *      rtk_qos_priMap_set
 * Description:
 *      Set the value of internal priority to QID mapping table.
 * Input:
 *      unit      - unit id
 *      queue_num - the number of queue (1~8).
 *      pPri2qid  - array of internal priority on a queue
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QUEUE_NUM    - Invalid queue number
 *      RT_ERR_QUEUE_ID     - Invalid queue ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      Below is an example of internal priority to QID mapping table.
 *      When queue numbers are 8, the pri2qid are pri2qid[0]=0, pri2qid[1]=1, pri2qid[2]=2..., etc.
 *
 *      -            Number of Available Output Queue
 *      -  Priority  1   2   3   4   5   6   7   8
 *      -        0   0   0   0   0   0   0   0   0
 *      -        1   0   0   0   0   0   0   0   1
 *      -        2   0   0   0   1   1   1   1   2
 *      -        3   0   0   0   1   1   2   2   3
 *      -        4   0   1   1   2   2   3   3   4
 *      -        5   0   1   1   2   3   4   4   5
 *      -        6   0   1   2   3   4   5   5   6
 *      -        7   0   1   2   3   4   5   6   7
 */
int32
rtk_qos_priMap_set(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_priMap_set(unit, queue_num, pPri2qid);
} /* end of rtk_qos_priMap_set */

/* Function Name:
 *      rtk_qos_portPriMap_get
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid internal priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_portPriMap_get(uint32 unit, rtk_port_t port, rtk_pri_t pri, rtk_qid_t *pQueue)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPriMap_get(unit, port, pri, pQueue);
} /* end of rtk_qos_portPriMap_get */

/* Function Name:
 *      rtk_qos_portPriMap_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QOS_INT_PRIORITY - Invalid internal priority
 *      RT_ERR_QUEUE_ID         - Invalid queue ID
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_portPriMap_set(uint32 unit, rtk_port_t port, rtk_pri_t pri, rtk_qid_t queue)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portPriMap_set(unit, port, pri, queue);
} /* end of rtk_qos_portPriMap_set */

/* Function Name:
 *      rtk_qos_1pDfltPri_get
 * Description:
 *      Get default inner-priority value
 * Input:
 *      unit      - unit id
 * Output:
 *      pDot1p_pri - default 802.1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_1pDfltPri_get(uint32 unit, rtk_pri_t *pDot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pDfltPri_get(unit, pDot1p_pri);
} /* end of rtk_qos_1pDfltPri_get */

/* Function Name:
 *      rtk_qos_1pDfltPri_set
 * Description:
 *      Set default inner-priority value
 * Input:
 *      unit      - unit id
 *      dot1p_pri - default 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - Invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY - Invalid dot1p priority
 * Applicable:
 *      8390, 8380
 * Note:
 *      If inner-untag packets would be transmmited with inner-tag, system takes inner-tag default
 *      priority dot1p_pri to be inner-tag priority.
 */
int32
rtk_qos_1pDfltPri_set(uint32 unit, rtk_pri_t dot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pDfltPri_set(unit, dot1p_pri);
} /* end of rtk_qos_1pDefaultPri_set */

/* Module Name    : QoS           */
/* Sub-module Name: Egress remark */

/* Function Name:
 *      rtk_qos_1pRemarkEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_1pRemarkEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->qos_1pRemarkEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_qos_1pRemarkEnable_get */

/* Function Name:
 *      rtk_qos_1pRemarkEnable_set
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
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_1pRemarkEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->qos_1pRemarkEnable_set(unit, port, enable);
#endif
} /* end of rtk_qos_1pRemarkEnable_set */

/* Function Name:
 *      rtk_qos_1pRemark_get
 * Description:
 *      Get the internal priority (3bits) to remarkd 802.1p priority(3bits) mapping.
 * Input:
 *      unit       - unit id
 *      int_pri    - internal priority value (range from 0 ~ 7)
 * Output:
 *      pDot1p_pri - remarked 802.1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_1pRemark_get(uint32 unit, rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pRemark_get(unit, int_pri, pDot1p_pri);
} /* end of rtk_qos_1pRemark_get */

/* Function Name:
 *      rtk_qos_1pRemark_set
 * Description:
 *      Set the internal priority(3bits) to remarked 802.1p priority(3bits) mapping.
 * Input:
 *      unit      - unit id
 *      int_pri   - internal priority value (range from 0 ~ 7)
 *      dot1p_pri - remarked 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid dot1p priority
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) For 8389, 802.1p remark functionality can map the internal priority to 802.1p priority
 *          before a packet is going to be transmited.
 *      (2) For 8390, int_pri can be internal priority or original inner-priority, and it dependents
 *          on the configuration of API rtk_qos_1pRemarkSrcSel_set().
 */
int32
rtk_qos_1pRemark_set(uint32 unit, rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pRemark_set(unit, int_pri, dot1p_pri);
} /* end of rtk_qos_1pRemark_set */

/* Function Name:
 *      rtk_qos_1pRemarkGroup_get
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~7
 */
int32
rtk_qos_1pRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   *pDot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pRemarkGroup_get(unit, grp_idx, int_pri, dp, pDot1p_pri);
} /* end of rtk_qos_1pRemarkGroup_get */

/* Function Name:
 *      rtk_qos_1pRemarkGroup_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~7
 */
int32
rtk_qos_1pRemarkGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   dot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pRemarkGroup_set(unit, grp_idx, int_pri, dp, dot1p_pri);
} /* end of rtk_qos_1pRemarkGroup_set */

/* Function Name:
 *      rtk_qos_1pRemarkSrcSel_get
 * Description:
 *      Get remarking source of dot1p remarking.
 * Input:
 *      unit  - unit id
 * Output:
 *      pType - remarking source
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
rtk_qos_1pRemarkSrcSel_get(uint32 unit, rtk_qos_1pRmkSrc_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pRemarkSrcSel_get(unit, pType);
} /* end of rtk_qos_1pRemarkSrcSel_get */

/* Function Name:
 *      rtk_qos_1pRemarkSrcSel_set
 * Description:
 *      Set remarking source of dot1p remarking.
 * Input:
 *      unit - unit id
 *      type - remarking source
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
 *      In 8390, 802.1p remark functionality can map the internal priority, original 802.1p
 *      priority, original outer priority or DSCP to 802.1p priority before a packet is going 
 *      to be transmited.
 *      In 8380, 802.1p remark functionality can map the internal priority or original 802.1p
 *      priority to 802.1p priority before a packet is going to be transmited.
 */
int32
rtk_qos_1pRemarkSrcSel_set(uint32 unit, rtk_qos_1pRmkSrc_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pRemarkSrcSel_set(unit, type);
} /* end of rtk_qos_1pRemarkSrcSel_set */

/* Function Name:
 *      rtk_qos_port1pRemarkGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of inner1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_port1pRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pInner1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_port1pRemarkGroup_get(unit, port, pInner1pRemarkGrp_idx);
} /* end of rtk_qos_port1pRemarkGroup_get */

/* Function Name:
 *      rtk_qos_port1pRemarkGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of inner1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_port1pRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 inner1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_port1pRemarkGroup_set(unit, port, inner1pRemarkGrp_idx);
} /* end of rtk_qos_port1pRemarkGroup_set */

/* Function Name:
 *      rtk_qos_port1pPriMapGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of inner1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_port1pPriMapGroup_get(uint32 unit, rtk_port_t port, uint32 *pInner1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_port1pPriMapGroup_get(unit, port, pInner1pRemarkGrp_idx);
} /* end of rtk_qos_port1pPriMapGroup_get */

/* Function Name:
 *      rtk_qos_port1pPriMapGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of inner1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_port1pPriMapGroup_set(uint32 unit, rtk_port_t port, uint32 inner1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_port1pPriMapGroup_set(unit, port, inner1pRemarkGrp_idx);
} /* end of rtk_qos_port1pPriMapGroup_set */

/* Function Name:
 *      rtk_qos_out1pRemarkEnable_get
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
rtk_qos_out1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_out1pRemarkEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->qos_out1pRemarkEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_qos_out1pRemarkEnable_get */

/* Function Name:
 *      rtk_qos_out1pRemarkEnable_set
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
rtk_qos_out1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_out1pRemarkEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->qos_out1pRemarkEnable_set(unit, port, enable);
#endif
} /* end of rtk_qos_out1pRemarkEnable_set */

/* Function Name:
 *      rtk_qos_outer1pRemark_get
 * Description:
 *      Get the internal priority (3bits) to remarkd outer dot1p priority(3bits) mapping.
 * Input:
 *      unit       - unit id
 *      int_pri    - internal priority value (range from 0 ~ 7)
 * Output:
 *      pDot1p_pri - remarked outer dot1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_outer1pRemark_get(uint32 unit, rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pRemark_get(unit, int_pri, pDot1p_pri);
} /* end of rtk_qos_outer1pRemark_get */

/* Function Name:
 *      rtk_qos_outer1pRemark_set
 * Description:
 *      Set the internal priority(3bits) to remarked outer dot1p priority(3bits) mapping.
 * Input:
 *      unit      - unit id
 *      int_pri   - internal priority value (range from 0 ~ 7)
 *      dot1p_pri - remarked 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid dot1p priority
 * Applicable:
 *      8390, 8380
 * Note:
 *      int_pri can be internal priority or original outer-priority, and it dependents on the
 *      configuration of API rtk_qos_outer1pRemarkSrcSel_set.
 */
int32
rtk_qos_outer1pRemark_set(uint32 unit, rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pRemark_set(unit, int_pri, dot1p_pri);
} /* end of rtk_qos_outer1pRemark_set */

/* Function Name:
 *      rtk_qos_outer1pRemarkGroup_get
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~7
 */
int32
rtk_qos_outer1pRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   *pDot1p_pri,
    uint32      *pDei)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pRemarkGroup_get(unit, grp_idx, int_pri, dp, pDot1p_pri, pDei);
} /* end of rtk_qos_outer1pRemarkGroup_get */

/* Function Name:
 *      rtk_qos_outer1pRemarkGroup_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - invalid dot1p priority
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~7
 */
int32
rtk_qos_outer1pRemarkGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   dot1p_pri,
    uint32      dei)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pRemarkGroup_set(unit, grp_idx, int_pri, dp, dot1p_pri, dei);
} /* end of rtk_qos_outer1pRemarkGroup_set */

/* Function Name:
 *      rtk_qos_outer1pRemarkSrcSel_get
 * Description:
 *      Get remarking source of outer dot1p remarking.
 * Input:
 *      unit  - unit id
 * Output:
 *      pType - remarking source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Outer 1P remarking source is either from internal priority or original outer 1P priority.
 *      Use 'rtk_qos_out1pRemarkEnable_set' to enable the outer 1P remarking function.
 */
int32
rtk_qos_outer1pRemarkSrcSel_get(uint32 unit, rtk_qos_outer1pRmkSrc_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pRemarkSrcSel_get(unit, pType);
} /* end of rtk_qos_outer1pRemarkSrcSel_get */

/* Function Name:
 *      rtk_qos_outer1pRemarkSrcSel_set
 * Description:
 *      Set remarking source of outer dot1p remarking.
 * Input:
 *      unit - unit id
 *      type - remarking source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      Outer 1P remarking source is either from internal priority or original outer 1P priority.
 *      Use 'rtk_qos_out1pRemarkEnable_set' to enable the outer 1P remarking function.
 */
int32
rtk_qos_outer1pRemarkSrcSel_set(uint32 unit, rtk_qos_outer1pRmkSrc_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pRemarkSrcSel_set(unit, type);
} /* end of rtk_qos_outer1pRemarkSrcSel_set */

/* Function Name:
 *      rtk_qos_portOuter1pDfltPriSrcSel_get
 * Description:
 *      Get default outer-priority source of specified port
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pType - type of default outer priority source
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
 *      If received outer-untag packets would be transmmited with outer-tag, system takes outer-tag
 *      default priority configuration of egress port to decide outer-tag priority.
 */
int32
rtk_qos_portOuter1pDfltPriSrcSel_get(uint32 unit, rtk_port_t port, rtk_qos_outer1pDfltSrc_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portOuter1pDfltPriSrcSel_get(unit, phy_port, pType);
    }
#else
    return RT_MAPPER(unit)->qos_portOuter1pDfltPriSrcSel_get(unit, port, pType);
#endif
} /* end of rtk_qos_portOuter1pDfltPriSrcSel_get */

/* Function Name:
 *      rtk_qos_portOuter1pDfltPriSrcSel_set
 * Description:
 *      Set default outer-priority source of specified port
 * Input:
 *      unit - unit id
 *      port - port id
 *      type - default outer priority source
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
 *      If received outer-untag packets would be transmmited with outer-tag, system takes outer-tag
 *      default priority configuration of egress port to decide outer-tag priority.
 */
int32
rtk_qos_portOuter1pDfltPriSrcSel_set(uint32 unit, rtk_port_t port, rtk_qos_outer1pDfltSrc_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portOuter1pDfltPriSrcSel_set(unit, phy_port, type);
    }
#else
    return RT_MAPPER(unit)->qos_portOuter1pDfltPriSrcSel_set(unit, port, type);
#endif
} /* end of rtk_qos_portOuter1pDfltPriSrcSel_set */

/* Function Name:
 *      rtk_qos_portOuter1pRemarkGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of outer1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_portOuter1pRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuter1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuter1pRemarkGroup_get(unit, port, pOuter1pRemarkGrp_idx);
} /* end of rtk_qos_portOuter1pRemarkGroup_get */

/* Function Name:
 *      rtk_qos_portOuter1pRemarkGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of outer1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_portOuter1pRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 outer1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuter1pRemarkGroup_set(unit, port, outer1pRemarkGrp_idx);
} /* end of rtk_qos_portOuter1pRemarkGroup_set */

/* Function Name:
 *      rtk_qos_portOuter1pPriMapGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of outer1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_portOuter1pPriMapGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuter1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuter1pPriMapGroup_get(unit, port, pOuter1pRemarkGrp_idx);
} /* end of rtk_qos_portOuter1pPriMapGroup_get */

/* Function Name:
 *      rtk_qos_portOuter1pPriMapGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of outer1pRemarkGrp_idx is 0~7
 */
int32
rtk_qos_portOuter1pPriMapGroup_set(uint32 unit, rtk_port_t port, uint32 outer1pRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuter1pPriMapGroup_set(unit, port, outer1pRemarkGrp_idx);
} /* end of rtk_qos_portOuter1pPriMapGroup_set */

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_dscpRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_dscpRemarkEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->qos_dscpRemarkEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_qos_dscpRemarkEnable_get */

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_set
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
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_dscpRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_dscpRemarkEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->qos_dscpRemarkEnable_set(unit, port, enable);
#endif
} /* end of rtk_qos_dscpRemarkEnable_set */

/* Function Name:
 *      rtk_qos_dscpRemark_get
 * Description:
 *      Get the internal priority (3bits) to DSCP remarking mapping.
 * Input:
 *      unit    - unit id
 *      int_pri - internal priority value (range from 0 ~ 7)
 * Output:
 *      pDscp   - remarked DSCP value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Remark DSCP using the internal priority.
 *      (2) The DSCP remarking source is specified by 'rtk_qos_dscpRemarkSrcSel_set'.
 */
int32
rtk_qos_dscpRemark_get(uint32 unit, rtk_pri_t int_pri, uint32 *pDscp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpRemark_get(unit, int_pri, pDscp);
} /* end of rtk_qos_dscpRemark_get */

/* Function Name:
 *      rtk_qos_dscpRemark_set
 * Description:
 *      Set the internal priority (3bits) to DSCP remarking mapping.
 * Input:
 *      unit    - unit id
 *      int_pri - internal priority value (range from 0 ~ 7)
 *      dscp    - remarked DSCP value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) Remark DSCP using the internal priority.
 *      (2) The DSCP remarking source is specified by 'rtk_qos_dscpRemarkSrcSel_set'.
 */
int32
rtk_qos_dscpRemark_set(uint32 unit, rtk_pri_t int_pri, uint32 dscp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpRemark_set(unit, int_pri, dscp);
} /* end of rtk_qos_dscpRemark_set */

/* Function Name:
 *      rtk_qos_dscp2Dot1pRemark_get
 * Description:
 *      Get DSCP to remarked 802.1p priority(3bits) mapping.
 * Input:
 *      unit    - unit id
 *      dscp    - DSCP value
 * Output:
 *      pDot1p_pri   - remarked 802.1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Applicable:
 *      8390
 * Note:
 *      None.
 */
int32
rtk_qos_dscp2Dot1pRemark_get(uint32 unit, uint32 dscp, rtk_pri_t *pDot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscp2Dot1pRemark_get(unit, dscp, pDot1p_pri);
} /* end of rtk_qos_dscp2Dot1pRemark_get */

/* Function Name:
 *      rtk_qos_dscp2Dot1pRemark_set
 * Description:
 *      Set DSCP to remarked 802.1p priority(3bits) mapping.
 * Input:
 *      unit      - unit id
 *      dscp      - DSCP value
 *      dot1p_pri - remarked 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid dot1p priority
 * Applicable:
 *      8390
 * Note:
 *      None.
 */
int32
rtk_qos_dscp2Dot1pRemark_set(uint32 unit, uint32 dscp, rtk_pri_t dot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscp2Dot1pRemark_set(unit, dscp, dot1p_pri);
} /* end of rtk_qos_dscp2Dot1pRemark_set */

/* Function Name:
 *      rtk_qos_dscp2Outer1pRemark_get
 * Description:
 *      Get DSCP to remarked outer dot1p priority(3bits) mapping.
 * Input:
 *      unit    - unit id
 *      dscp    - DSCP value
 * Output:
 *      pDot1p_pri   - remarked outer dot1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Applicable:
 *      8390
 * Note:
 *      None.
 */
int32
rtk_qos_dscp2Outer1pRemark_get(uint32 unit, uint32 dscp, rtk_pri_t *pDot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscp2Outer1pRemark_get(unit, dscp, pDot1p_pri);
} /* end of rtk_qos_dscp2Outer1pRemark_get */

/* Function Name:
 *      rtk_qos_dscp2Outer1pRemark_set
 * Description:
 *      Set DSCP to remarked outer dot1p priority(3bits) mapping.
 * Input:
 *      unit      - unit id
 *      dscp      - DSCP value
 *      dot1p_pri - remarked outer dot1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid dot1p priority
 * Applicable:
 *      8390
 * Note:
 *      None.
 */
extern int32
rtk_qos_dscp2Outer1pRemark_set(uint32 unit, uint32 dscp, rtk_pri_t dot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscp2Outer1pRemark_set(unit, dscp, dot1p_pri);
} /* end of rtk_qos_dscp2Outer1pRemark_set */

/* Function Name:
 *      rtk_qos_dscp2DscpRemark_get
 * Description:
 *      Get DSCP to DSCP remarking mapping.
 * Input:
 *      unit  - unit id
 *      dscp  - DSCP value
 * Output:
 *      pDscp - remarked DSCP value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - Invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - Invalid dscp value
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Remark DSCP using the original DSCP.
 *      (2) The DSCP remarking source is specified by 'rtk_qos_dscpRemarkSrcSel_set'.
 */
int32
rtk_qos_dscp2DscpRemark_get(uint32 unit, uint32 dscp, uint32 *pDscp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscp2DscpRemark_get(unit, dscp, pDscp);
} /* end of rtk_qos_dscp2DscpRemark_get */

/* Function Name:
 *      rtk_qos_dscp2DscpRemark_set
 * Description:
 *      Set DSCP to DSCP remarking mapping.
 * Input:
 *      unit    - unit id
 *      dscp    - DSCP value
 *      rmkDscp - remarked DSCP value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - Invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - Invalid dscp value
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Remark DSCP using the original DSCP.
 *      (2) The DSCP remarking source is specified by 'rtk_qos_dscpRemarkSrcSel_set'.
 */
int32
rtk_qos_dscp2DscpRemark_set(uint32 unit, uint32 dscp, uint32 rmkDscp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscp2DscpRemark_set(unit, dscp, rmkDscp);
} /* end of rtk_qos_dscp2DscpRemark_set */

/* Function Name:
 *      rtk_qos_dscpRemarkGroup_get
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~7
 */
int32
rtk_qos_dscpRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    uint32      *pDscp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpRemarkGroup_get(unit, grp_idx, int_pri, dp, pDscp);
} /* end of rtk_qos_dscpRemarkGroup_get */

/* Function Name:
 *      rtk_qos_dscpRemarkGroup_set
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
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE   - invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY - invalid internal priority
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of grp_idx is 0~7
 */
int32
rtk_qos_dscpRemarkGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    uint32      dscp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpRemarkGroup_set(unit, grp_idx, int_pri, dp, dscp);
} /* end of rtk_qos_dscpRemarkGroup_set */

/* Function Name:
 *      rtk_qos_dscpRemarkSrcSel_get
 * Description:
 *      Get remarking source of DSCP remarking.
 * Input:
 *      unit  - unit id
 * Output:
 *      pType - remarking source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      After specifing the remarking source, please use the corresponding API to specify the remarking mapping.
 */
int32
rtk_qos_dscpRemarkSrcSel_get(uint32 unit, rtk_qos_dscpRmkSrc_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpRemarkSrcSel_get(unit, pType);
} /* end of rtk_qos_dscpRemarkSrcSel_get */

/* Function Name:
 *      rtk_qos_dscpRemarkSrcSel_set
 * Description:
 *      Set remarking source of DSCP remarking.
 * Input:
 *      unit - unit id
 *      type - remarking source
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
 *      After specifing the remarking source, please use the corresponding API to specify the remarking mapping.
 */
int32
rtk_qos_dscpRemarkSrcSel_set(uint32 unit, rtk_qos_dscpRmkSrc_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_dscpRemarkSrcSel_set(unit, type);
} /* end of rtk_qos_dscpRemarkSrcSel_set */

/* Function Name:
 *      rtk_qos_portdscpRemarkGroup_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of dscpRemarkGrp_idx is 0~7
 */
int32
rtk_qos_portdscpRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pDscpRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portdscpRemarkGroup_get(unit, port, pDscpRemarkGrp_idx);
} /* end of rtk_qos_portdscpRemarkGroup_get */

/* Function Name:
 *      rtk_qos_portdscpRemarkGroup_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of dscpRemarkGrp_idx is 0~7
 */
int32
rtk_qos_portdscpRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 dscpRemarkGrp_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portdscpRemarkGroup_set(unit, port, dscpRemarkGrp_idx);
} /* end of rtk_qos_portdscpRemarkGroup_set */

/* Function Name:
 *      rtk_qos_deiRemark_get
 * Description:
 *      Get the internal drop precedence to DEI remarking mapping.
 * Input:
 *      unit - unit id
 *      dp   - internal drop precedence (range from 0 ~ 2)
 * Output:
 *      pDei - remarked DEI value (range from 0 ~ 1)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_QOS_DROP_PRECEDENCE - Invalid drop precedence
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Remark DEI using the internal drop precedence.
 */
int32
rtk_qos_deiRemark_get(uint32 unit, uint32 dp, uint32 *pDei)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_deiRemark_get(unit, dp, pDei);
} /* end of rtk_qos_deiRemark_get */

/* Function Name:
 *      rtk_qos_deiRemark_set
 * Description:
 *      Set the internal drop precedence to DEI remarking mapping.
 * Input:
 *      unit - unit id
 *      dp   - internal drop precedence (range from 0 ~ 2)
 *      dei  - remarked DEI value (range from 0 ~ 1)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_QOS_DEI_VALUE       - Invalid DEI value
 *      RT_ERR_QOS_DROP_PRECEDENCE - Invalid drop precedence
 * Applicable:
 *      8390
 * Note:
 *      Remark DEI using the internal drop precedence.
 */
int32
rtk_qos_deiRemark_set(uint32 unit, uint32 dp, uint32 dei)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_deiRemark_set(unit, dp, dei);
} /* end of rtk_qos_deiRemark_set */

/* Function Name:
 *      rtk_qos_deiRemarkEnable_get
 * Description:
 *      Get DEI remark status for a port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of DEI remark
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
 *    The status of DEI remark:
 *    - DISABLED
 *    - ENABLED
 */
int32
rtk_qos_deiRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_deiRemarkEnable_get(unit, port, pEnable);
} /* end of rtk_qos_deiRemarkEnable_get */

/* Function Name:
 *      rtk_qos_deiRemarkEnable_set
 * Description:
 *      Set DEI remark status for a port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of DEI remark
 * Output:
 *      None.
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
 *    The status of DEI remark:
 *    - DISABLED
 *    - ENABLED
 */
int32
rtk_qos_deiRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_deiRemarkEnable_set(unit, port, enable);
} /* end of rtk_qos_deiRemarkEnable_set */

/* Function Name:
 *      rtk_qos_portDEIRemarkTagSel_get
 * Description:
 *      Get DEI remarking VLAN tag selection of specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pType - type of DEI
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
 *      For DEI remark function, per egress port can select outer-tag or inner-tag to do DEI remarking.
 */
int32
rtk_qos_portDEIRemarkTagSel_get(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDEIRemarkTagSel_get(unit, port, pType);
} /* end of rtk_qos_portDEIRemarkTagSel_get */

/* Function Name:
 *      rtk_qos_portDEIRemarkTagSel_set
 * Description:
 *      Set DEI remarking VLAN tag selection of specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      type - type of DEI
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
 *      8390
 * Note:
 *      For DEI remark function, per egress port can select outer-tag or inner-tag to do DEI remarking.
 */
int32
rtk_qos_portDEIRemarkTagSel_set(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portDEIRemarkTagSel_set(unit, port, type);
} /* end of rtk_qos_portDEIRemarkTagSel_set */

/* Module Name    : QoS              */
/* Sub-module Name: Queue scheduling */

/* Function Name:
 *      rtk_qos_schedulingAlgorithm_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The types of scheduling algorithm:
 *      - WFQ
 *      - WRR
 */
int32
rtk_qos_schedulingAlgorithm_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   *pScheduling_type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_schedulingAlgorithm_get(unit, phy_port, pScheduling_type);
    }
#else
    return RT_MAPPER(unit)->qos_schedulingAlgorithm_get(unit, port, pScheduling_type);
#endif
} /* end of rtk_qos_schedulingAlgorithm_get */

/* Function Name:
 *      rtk_qos_schedulingAlgorithm_set
 * Description:
 *      Set the scheduling algorithm of the port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 *      scheduling_type - type of scheduling algorithm
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - Invalid scheduling algorithm type
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The types of scheduling algorithm:
 *      - WFQ
 *      - WRR
 */
int32
rtk_qos_schedulingAlgorithm_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qos_scheduling_type_t   scheduling_type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_schedulingAlgorithm_set(unit, phy_port, scheduling_type);
    }
#else
    return RT_MAPPER(unit)->qos_schedulingAlgorithm_set(unit, port, scheduling_type);
#endif
} /* end of rtk_qos_schedulingAlgorithm_set */

/* Function Name:
 *      rtk_qos_schedulingQueue_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *      If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
int32
rtk_qos_schedulingQueue_get(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_schedulingQueue_get(unit, phy_port, pQweights);
    }
#else
    return RT_MAPPER(unit)->qos_schedulingQueue_get(unit, port, pQweights);
#endif
} /* end of rtk_qos_schedulingQueue_get */

/* Function Name:
 *      rtk_qos_schedulingQueue_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - Invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The types of queue are: WFQ_WRR_PRIORITY or STRICT_PRIORITY.
 *      If the weight is 0 then the type is STRICT_PRIORITY, else the type is WFQ_WRR_PRIORITY.
 */
int32
rtk_qos_schedulingQueue_set(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_schedulingQueue_set(unit, phy_port, pQweights);
    }
#else
    return RT_MAPPER(unit)->qos_schedulingQueue_set(unit, port, pQweights);
#endif
} /* end of rtk_qos_schedulingQueue_set */

/* Function Name:
 *      rtk_qos_wfqFixedBandwidthEnable_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_wfqFixedBandwidthEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_wfqFixedBandwidthEnable_get(unit, phy_port, queue, pEnable);
    }
#else
    return RT_MAPPER(unit)->qos_wfqFixedBandwidthEnable_get(unit, port, queue, pEnable);
#endif
} /* end of rtk_qos_wfqFixedBandwidthEnable_get */

/* Function Name:
 *      rtk_qos_wfqFixedBandwidthEnable_set
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
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_QUEUE_ID - invalid queue id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_wfqFixedBandwidthEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_wfqFixedBandwidthEnable_set(unit, phy_port, queue, enable);
    }
#else
    return RT_MAPPER(unit)->qos_wfqFixedBandwidthEnable_set(unit, port, queue, enable);
#endif
} /* end of rtk_qos_wfqFixedBandwidthEnable_set */

/* Module Name    : QoS              */
/* Sub-module Name: Congestion avoidance */

/* Function Name:
 *      rtk_qos_congAvoidAlgo_get
 * Description:
 *      Get algorithm of congestion avoidance.
 * Input:
 *      unit  - unit id
 * Output:
 *      pAlgo - pointer to algorithm of congestion avoidance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Algorithm is as following:
 *      - CONG_AVOID_WRED
 *      - CONG_AVOID_WTD
 *      - CONG_AVOID_SRED
 *      - CONG_AVOID_SWRED
 *      - CONG_AVOID_TD
 */
int32
rtk_qos_congAvoidAlgo_get(uint32 unit, rtk_qos_congAvoidAlgo_t *pAlgo)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidAlgo_get(unit, pAlgo);
} /* end of rtk_qos_congAvoidAlgo_get */

/* Function Name:
 *      rtk_qos_congAvoidAlgo_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Algorithm is as following:
 *      - CONG_AVOID_WRED
 *      - CONG_AVOID_WTD
 *      - CONG_AVOID_SRED
 *      - CONG_AVOID_SWRED
 *      - CONG_AVOID_TD
 */
int32
rtk_qos_congAvoidAlgo_set(uint32 unit, rtk_qos_congAvoidAlgo_t algo)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidAlgo_set(unit, algo);
} /* end of rtk_qos_congAvoidAlgo_set */

/* Function Name:
 *      rtk_qos_congAvoidQueueThreshEnable_get
 * Description:
 *      Get enable status of queue threshold for congestion avoidance.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of queue threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidQueueThreshEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidQueueThreshEnable_get(unit, pEnable);
} /* end of rtk_qos_congAvoidQueueThreshEnable_get */

/* Function Name:
 *      rtk_qos_congAvoidQueueThreshEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidQueueThreshEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidQueueThreshEnable_set(unit, enable);
} /* end of rtk_qos_congAvoidQueueThreshEnable_set */

/* Function Name:
 *      rtk_qos_congAvoidPortThreshEnable_get
 * Description:
 *      Get enable status of port threshold for congestion avoidance.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of port threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidPortThreshEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidPortThreshEnable_get(unit, pEnable);
} /* end of rtk_qos_congAvoidPortThreshEnable_get */

/* Function Name:
 *      rtk_qos_congAvoidPortThreshEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidPortThreshEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidPortThreshEnable_set(unit, enable);
} /* end of rtk_qos_congAvoidPortThreshEnable_set */

/* Function Name:
 *      rtk_qos_congAvoidSysThreshEnable_get
 * Description:
 *      Get enable status of system threshold for congestion avoidance.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of system threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidSysThreshEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidSysThreshEnable_get(unit, pEnable);
} /* end of rtk_qos_congAvoidSysThreshEnable_get */

/* Function Name:
 *      rtk_qos_congAvoidSysThreshEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidSysThreshEnable_set(uint32 unit,  rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidSysThreshEnable_set(unit, enable);
} /* end of rtk_qos_congAvoidSysThreshEnable_set */

/* Function Name:
 *      rtk_qos_congAvoidSysThresh_get
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8328, 8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidSysThresh_get(
    uint32                      unit,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidSysThresh_get(unit, dp, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidSysThresh_get */

/* Function Name:
 *      rtk_qos_congAvoidSysThresh_set
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8328, 8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidSysThresh_set(
    uint32                      unit,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidSysThresh_set(unit, dp, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidSysThresh_set */

/* Function Name:
 *      rtk_qos_congAvoidSysDropProbability_get
 * Description:
 *      Get system drop probability of congestion avoidance.
 * Input:
 *      unit         - unit id
 *      dp           - drop precedence
 * Output:
 *      pProbability - pointer to drop probability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidSysDropProbability_get(
    uint32  unit,
    uint32  dp,
    uint32  *pProbability)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidSysDropProbability_get(unit, dp, pProbability);
} /* end of rtk_qos_congAvoidSysDropProbability_get */

/* Function Name:
 *      rtk_qos_congAvoidSysDropProbability_set
 * Description:
 *      Set system drop probability of congestion avoidance.
 * Input:
 *      unit        - unit id
 *      dp          - drop precedence
 *      probability - drop probability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidSysDropProbability_set(
    uint32  unit,
    uint32  dp,
    uint32  probability)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidSysDropProbability_set(unit, dp, probability);
} /* end of rtk_qos_congAvoidSysDropProbability_set */

/* Function Name:
 *      rtk_qos_congAvoidPortThresh_get
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) There are 2 fields in rtk_qos_congAvoidThresh_t: maxThresh and minThresh
 *      (2) The valid range of maxThresh and minThresh is 0~0x7FF
 */
int32
rtk_qos_congAvoidPortThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidPortThresh_get(unit, port, dp, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidPortThresh_get */

/* Function Name:
 *      rtk_qos_congAvoidPortThresh_set
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) There are 2 fields in rtk_qos_congAvoidThresh_t: maxThresh and minThresh
 *      (2) The valid range of maxThresh and minThresh is 0~0x7FF
 */
int32
rtk_qos_congAvoidPortThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidPortThresh_set(unit, port, dp, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidPortThresh_set */

/* Function Name:
 *      rtk_qos_congAvoidQueueThresh_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) There are 2 fields in rtk_qos_congAvoidThresh_t: maxThresh and minThresh
 *      (2) The valid range of maxThresh and minThresh is 0~0x7FF
 */
int32
rtk_qos_congAvoidQueueThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidQueueThresh_get(unit, port, queue, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidQueueThresh_get */

/* Function Name:
 *      rtk_qos_congAvoidQueueThresh_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) There are 2 fields in rtk_qos_congAvoidThresh_t: maxThresh and minThresh
 *      (2) The valid range of maxThresh and minThresh is 0~0x7FF
 */
int32
rtk_qos_congAvoidQueueThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidQueueThresh_set(unit, port, queue, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidQueueThresh_set */

/* Function Name:
 *      rtk_qos_congAvoidGlobalQueueThresh_get
 * Description:
 *      Get global queue threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      queue            - queue id
 *      dp               - drop precedence
 * Output:
 *      pCongAvoidThresh - pointer to system threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QUEUE_ID        - invalid queue id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidGlobalQueueThresh_get(
    uint32                      unit,
    rtk_qid_t                   queue,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidGlobalQueueThresh_get(unit, queue, dp, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidGlobalQueueThresh_get */

/* Function Name:
 *      rtk_qos_congAvoidGlobalQueueThresh_set
 * Description:
 *      Set global queue threshold of congestion avoidance.
 * Input:
 *      unit             - unit id
 *      queue            - queue id
 *      dp               - drop precedence
 *      pCongAvoidThresh - system threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QUEUE_ID        - invalid queue id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidGlobalQueueThresh_set(
    uint32                      unit,
    rtk_qid_t                   queue,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidGlobalQueueThresh_set(unit, queue, dp, pCongAvoidThresh);
} /* end of rtk_qos_congAvoidGlobalQueueThresh_set */

/* Function Name:
 *      rtk_qos_congAvoidGlobalQueueDropProbability_get
 * Description:
 *      Get global queue drop probability of congestion avoidance.
 * Input:
 *      unit         - unit id
 *      queue        - queue id
 *      dp           - drop precedence
 * Output:
 *      pProbability - pointer to drop probability
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QUEUE_ID        - invalid queue id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidGlobalQueueDropProbability_get(
    uint32      unit,
    rtk_qid_t   queue,
    uint32      dp,
    uint32      *pProbability)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidGlobalQueueDropProbability_get(unit, queue, dp, pProbability);
} /* end of rtk_qos_congAvoidGlobalQueueDropProbability_get */

/* Function Name:
 *      rtk_qos_congAvoidGlobalQueueDropProbability_set
 * Description:
 *      Set system drop probability of congestion avoidance.
 * Input:
 *      unit        - unit id
 *      queue       - queue id
 *      dp          - drop precedence
 *      probability - drop probability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QUEUE_ID        - invalid queue id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_qos_congAvoidGlobalQueueDropProbability_set(
    uint32      unit,
    rtk_qid_t   queue,
    uint32      dp,
    uint32      probability)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_congAvoidGlobalQueueDropProbability_set(unit, queue, dp, probability);
} /* end of rtk_qos_congAvoidGlobalQueueDropProbability_set */

/*
 * Component: WRED
 */

/* Function Name:
 *      rtk_qos_wredSysThresh_get
 * Description:
 *      Get system threshold of WRED.
 * Input:
 *      unit    - unit id
 *      dp      - drop precedence
 * Output:
 *      pThresh - pointer to system threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) There are 2 fields in rtk_qos_wredThresh_t: maxThresh and minThresh
 *      (2) The valid range of maxThresh and minThresh is 0~0x7FF
 */
int32
rtk_qos_wredSysThresh_get(uint32 unit, uint32 dp, rtk_qos_wredThresh_t *pThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredSysThresh_get(unit, dp, pThresh);
} /* end of rtk_qos_wredSysThresh_get */

/* Function Name:
 *      rtk_qos_wredSysThresh_set
 * Description:
 *      Set system threshold of WRED.
 * Input:
 *      unit    - unit id
 *      dp      - drop precedence
 *      pThresh - system threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) There are 2 fields in rtk_qos_wredThresh_t: maxThresh and minThresh
 *      (2) The valid range of maxThresh and minThresh is 0~0x7FF
 */
int32
rtk_qos_wredSysThresh_set(uint32 unit, uint32 dp, rtk_qos_wredThresh_t *pThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredSysThresh_set(unit, dp, pThresh);
} /* end of rtk_qos_wredSysThresh_set */

/* Function Name:
 *      rtk_qos_wredWeight_get
 * Description:
 *      Get weight of WRED.
 * Input:
 *      unit    - unit id
 * Output:
 *      pWeight - pointer to weight of WRED
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid range of weight is 0~0x3FF
 */
int32
rtk_qos_wredWeight_get(uint32 unit, uint32 *pWeight)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredWeight_get(unit, pWeight);
} /* end of rtk_qos_wredWeight_get */

/* Function Name:
 *      rtk_qos_wredWeight_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid range of weight is 0~0x3FF
 */
int32
rtk_qos_wredWeight_set(uint32 unit, uint32 weight)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredWeight_set(unit, weight);
} /* end of rtk_qos_wredWeight_set */

/* Function Name:
 *      rtk_qos_wredMpd_get
 * Description:
 *      Get MPD(Mark Probability Denominator) of WRED.
 * Input:
 *      unit - unit id
 * Output:
 *      pMpd - pointer to MPD of WRED
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The valid range of mpd is 0~15
 */
int32
rtk_qos_wredMpd_get(uint32 unit, uint32 *pMpd)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredMpd_get(unit, pMpd);
} /* end of rtk_qos_wredMpd_get */

/* Function Name:
 *      rtk_qos_wredMpd_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The valid range of mpd is 0~15
 */
int32
rtk_qos_wredMpd_set(uint32 unit, uint32 mpd)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredMpd_set(unit, mpd);
} /* end of rtk_qos_wredMpd_set */

/* Function Name:
 *      rtk_qos_wredEcnEnable_get
 * Description:
 *      Get enable status of ECN for WRED.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of ECN
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_wredEcnEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredEcnEnable_get(unit, pEnable);
} /* end of rtk_qos_wredEcnEnable_get */

/* Function Name:
 *      rtk_qos_wredEcnEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_wredEcnEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredEcnEnable_set(unit, enable);
} /* end of rtk_qos_wredEcnEnable_set */

/* Function Name:
 *      rtk_qos_wredCntReverseEnable_get
 * Description:
 *      Get enable status of counter reverse for WRED.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of counter reverse
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_wredCntReverseEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredCntReverseEnable_get(unit, pEnable);
} /* end of rtk_qos_wredCntReverseEnable_get */

/* Function Name:
 *      rtk_qos_wredCntReverseEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_qos_wredCntReverseEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_wredCntReverseEnable_set(unit, enable);
} /* end of rtk_qos_wredCntReverseEnable_set */

/* Function Name:
 *      rtk_qos_portAvbStreamReservationClassEnable_get
 * Description:
 *      Get status of the specified stream class of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      srClass - stream class
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_PORT                 - invalid port id
 *      RT_ERR_AVB_INVALID_SR_CLASS - Invalid SR Class
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_portAvbStreamReservationClassEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qos_avbSrClass_t    srClass,
    rtk_enable_t            *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portAvbStreamReservationClassEnable_get(unit, phy_port, srClass, pEnable);
    }
#else
    return RT_MAPPER(unit)->qos_portAvbStreamReservationClassEnable_get(unit, port, srClass, pEnable);
#endif
} /* end of rtk_qos_portAvbStreamReservationClassEnable_get */

/* Function Name:
 *      rtk_qos_portAvbStreamReservationClassEnable_set
 * Description:
 *      Set status of the specified stream class of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      srClass - stream class
 *      enable  - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_PORT                 - invalid port id
 *      RT_ERR_AVB_INVALID_SR_CLASS - Invalid SR Class
 *      RT_ERR_INPUT                - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_portAvbStreamReservationClassEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qos_avbSrClass_t    srClass,
    rtk_enable_t            enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portAvbStreamReservationClassEnable_set(unit, phy_port, srClass, enable);
    }
#else
    return RT_MAPPER(unit)->qos_portAvbStreamReservationClassEnable_set(unit, port, srClass, enable);
#endif
} /* end of rtk_qos_portAvbStreamReservationClassEnable_set */

/* Function Name:
 *      rtk_qos_avbStreamReservationConfig_get
 * Description:
 *      Get the configuration of Stream Reservation in the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pSrConf - pointer buffer of configuration
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
rtk_qos_avbStreamReservationConfig_get(uint32 unit, rtk_qos_avbSrConf_t *pSrConf)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_avbStreamReservationConfig_get(unit, pSrConf);
} /* end of rtk_qos_avbStreamReservationConfig_get */

/* Function Name:
 *      rtk_qos_avbStreamReservationConfig_set
 * Description:
 *      Set the configuration of Stream Reservation in the specified device.
 * Input:
 *      unit   - unit id
 *      pSrConf - pointer buffer of configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid dot1p priority
 *      RT_ERR_QUEUE_ID         - invalid queue id
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_qos_avbStreamReservationConfig_set(uint32 unit, rtk_qos_avbSrConf_t *pSrConf)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_avbStreamReservationConfig_set(unit, pSrConf);
} /* end of rtk_qos_avbStreamReservationConfig_set */

/* Function Name:
 *      rtk_qos_pkt2CpuPriRemap_get
 * Description:
 *      Get the new priority for the packets that normal forwarded to CPU.
 * Input:
 *      unit    - unit id
 *      intPri  - original internal
 * Output:
 *      pNewPri - new priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      System can remap internal priority to a new priority for the packets that normal forwarded to CPU.
 */
int32
rtk_qos_pkt2CpuPriRemap_get(uint32 unit, rtk_pri_t intPri, rtk_pri_t *pNewPri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_pkt2CpuPriRemap_get(unit, intPri, pNewPri);
} /* end of rtk_qos_pkt2CpuPriRemap_get */

/* Function Name:
 *      rtk_qos_pkt2CpuPriRemap_set
 * Description:
 *      Set the new priority for the packets that normal forwarded to CPU.
 * Input:
 *      unit   - unit id
 *      intPri - original internal
 *      newPri - new priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Applicable:
 *      8390, 8380
 * Note:
 *      System can remap internal priority to a new priority for the packets that normal forwarded to CPU.
 */
int32
rtk_qos_pkt2CpuPriRemap_set(uint32 unit, rtk_pri_t intPri, rtk_pri_t newPri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_pkt2CpuPriRemap_set(unit, intPri, newPri);
} /* end of rtk_qos_pkt2CpuPriRemap_set */

/* Function Name:
 *      rtk_qos_rspanPriRemap_get
 * Description:
 *      Get the internal priority that rspan tag  priority remap.
 * Input:
 *      unit      - unit id
 *      rspan_pri - rspan tag  priority value (range from 0 ~ 7)
 * Output:
 *      pInt_pri  - internal priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY - Invalid rspan tag  priority
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None.
 */
int32
rtk_qos_rspanPriRemap_get(uint32 unit, rtk_pri_t rspan_pri, rtk_pri_t *pInt_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_rspanPriRemap_get(unit, rspan_pri, pInt_pri);
} /* end of rtk_qos_rspanPriRemap_get */

/* Function Name:
 *      rtk_qos_rspanPriRemap_set
 * Description:
 *      Set the internal priority that rspan tag  priority remap.
 * Input:
 *      unit      - unit id
 *      dot1p_pri - rspan tag  priority value (range from 0 ~ 7)
 *      int_pri   - internal priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY  - Invalid rspan tag  priority
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority value
 * Applicable:
 *      8380
 * Note:
 *      None.
 */
int32
rtk_qos_rspanPriRemap_set(uint32 unit, rtk_pri_t rspan_pri, rtk_pri_t int_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_rspanPriRemap_set(unit, rspan_pri, int_pri);
} /* end of rtk_qos_rspanPriRemap_set */

/* Function Name:
 *      rtk_qos_portPri2IgrQMapEnable_get
 * Description:
 *      Get priority to input queue mapping ability.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - ability
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
 *      None.
 */
int32
rtk_qos_portPri2IgrQMapEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portPri2IgrQMapEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->qos_portPri2IgrQMapEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_qos_portPri2IgrQMapEnable_get */

/* Function Name:
 *      rtk_qos_portPri2IgrQMapEnable_set
 * Description:
 *      Set priority to input queue mapping ability.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - ability
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid enable value
 * Applicable:
 *      8380
 * Note:
 *      None.
 */
int32
rtk_qos_portPri2IgrQMapEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portPri2IgrQMapEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->qos_portPri2IgrQMapEnable_set(unit, port, enable);
#endif
} /* end of rtk_qos_portPri2IgrQMapEnable_set */

/* Function Name:
 *      rtk_qos_portPri2IgrQMap_get
 * Description:
 *      Get the value of internal priority to QID mapping table.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pPri2qid - array of internal priority on a queue
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None.
 */
int32
rtk_qos_portPri2IgrQMap_get(uint32 unit, rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portPri2IgrQMap_get(unit, phy_port, pPri2qid);
    }
#else
    return RT_MAPPER(unit)->qos_portPri2IgrQMap_get(unit, port, pPri2qid);
#endif
} /* end of rtk_qos_portPri2IgrQMap_get */

/* Function Name:
 *      rtk_qos_portPri2IgrQMap_set
 * Description:
 *      Set the value of internal priority to QID mapping table.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pPri2qid - array of internal priority on a queue
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None.
 */
int32
rtk_qos_portPri2IgrQMap_set(uint32 unit, rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portPri2IgrQMap_set(unit, phy_port, pPri2qid);
    }
#else
    return RT_MAPPER(unit)->qos_portPri2IgrQMap_set(unit, port, pPri2qid);
#endif
} /* end of rtk_qos_portPri2IgrQMap_set */

/* Function Name:
 *      rtk_qos_1pDfltPriSrcSel_get
 * Description:
 *      Get default inner-priority source of specified port
 * Input:
 *      unit  - unit id
 * Output:
 *      pType - pointer of default outer dot1p priority source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None.
 */
int32
rtk_qos_1pDfltPriSrcSel_get(uint32 unit, rtk_qos_1pDfltPriSrc_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pDfltPriSrcSel_get(unit, pType);
} /* end of rtk_qos_1pDfltPriSrcSel_get */

/* Function Name:
 *      rtk_qos_1pDfltPriSrcSel_set
 * Description:
 *      Set default inner-priority source of specified port
 * Input:
 *      unit - unit id
 *      type - default outer dot1p priority source
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None.
 */
int32
rtk_qos_1pDfltPriSrcSel_set(uint32 unit, rtk_qos_1pDfltPriSrc_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_1pDfltPriSrcSel_set(unit, type);
} /* end of rtk_qos_1pDfltPriSrcSel_set */

/* Function Name:
 *      rtk_qos_outer1pDfltPriCfgSrcSel_get
 * Description:
 *      Get default outer-priority configured source
 * Input:
 *      unit       - unit id
 * Output:
 *      pDflt_sel  - default selection
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None.
 */
int32
rtk_qos_outer1pDfltPriCfgSrcSel_get(uint32 unit, rtk_qos_outer1pDfltCfgSrc_t *pDflt_sel)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pDfltPriCfgSrcSel_get(unit, pDflt_sel);
} /* end of rtk_qos_outer1pDfltPriCfgSrcSel_get */

/* Function Name:
 *      rtk_qos_outer1pDfltPriCfgSrcSel_set
 * Description:
 *      Set default outer-priority configured source
 * Input:
 *      unit      - unit id
 *      dflt_sel  - default selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      For outer-untag packet, the default TX outer-priority can refer the configuration 
 *      of RX port or TX port by the API.
 */
int32
rtk_qos_outer1pDfltPriCfgSrcSel_set(uint32 unit, rtk_qos_outer1pDfltCfgSrc_t dflt_sel)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pDfltPriCfgSrcSel_set(unit, dflt_sel);
} /* end of rtk_qos_outer1pDfltPriCfgSrcSel_set */

/* Function Name:
 *      rtk_qos_portOuter1pRemarkSrcSel_get
 * Description:
 *      Get the remarking source of outer dot1p priority remarking.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pType - remarking source
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
 *      None.
 */
int32
rtk_qos_portOuter1pRemarkSrcSel_get(uint32 unit, rtk_port_t port, rtk_qos_outer1pRmkSrc_t *pType)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portOuter1pRemarkSrcSel_get(unit, phy_port, pType);
    }
#else
    return RT_MAPPER(unit)->qos_portOuter1pRemarkSrcSel_get(unit, port, pType);
#endif
} /* end of rtk_qos_portOuter1pRemarkSrcSel_get */

/* Function Name:
 *      rtk_qos_portOuter1pRemarkSrcSel_set
 * Description:
 *      Set the remarking source of outer dot1p priority remarking.
 * Input:
 *      unit - unit id
 *      port - port id
 *      type - remarking source
 * Output:
 *      None.
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
 *      None.
 */
int32
rtk_qos_portOuter1pRemarkSrcSel_set(uint32 unit, rtk_port_t port, rtk_qos_outer1pRmkSrc_t type)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portOuter1pRemarkSrcSel_set(unit, phy_port, type);
    }
#else
    return RT_MAPPER(unit)->qos_portOuter1pRemarkSrcSel_set(unit, port, type);
#endif
} /* end of rtk_qos_portOuter1pRemarkSrcSel_set */

/* Function Name:
 *      rtk_qos_outer1pDfltPri_get
 * Description:
 *      Get default outer-priority value
 * Input:
 *      unit       - unit id
 * Output:
 *      pDot1p_pri - default 802.1p priority value (range from 0 ~ 7)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None.
 */
int32
rtk_qos_outer1pDfltPri_get(uint32 unit, rtk_pri_t *pDot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pDfltPri_get(unit, pDot1p_pri);
} /* end of rtk_qos_outer1pDfltPri_get */

/* Function Name:
 *      rtk_qos_outer1pDfltPri_set
 * Description:
 *      Set default outer -priority value
 * Input:
 *      unit      - unit id
 *      dot1p_pri - default 802.1p priority value (range from 0 ~ 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_QOS_1P_PRIORITY - Invalid dot1p priority
 * Applicable:
 *      8390, 8380
 * Note:
 *      None.
 */
int32
rtk_qos_outer1pDfltPri_set(uint32 unit, rtk_pri_t dot1p_pri)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_outer1pDfltPri_set(unit, dot1p_pri);
} /* end of rtk_qos_outer1pDfltPri_set */

/* Function Name:
 *      rtk_qos_igrQueueWeight_get
 * Description:
 *      Get the weight of ingress queue.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      queue  - queue id
 * Output:
 *      pQweights -the weigh of specified queue
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *    If weight == 0, means the queue is STRICT_PRIORITY
 */
int32 
rtk_qos_igrQueueWeight_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, uint32 *pQweight)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_igrQueueWeight_get(unit, phy_port, queue, pQweight);
    }
#else
    return RT_MAPPER(unit)->qos_igrQueueWeight_get(unit, port, queue, pQweight);
#endif
}/* end of rtk_qos_igrQueueWeigh_get */

/* Function Name:
 *      rtk_qos_igrQueueWeight_set
 * Description:
 *      Get the weight of ingress queue.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      queue  - queue id
 *      qWeights -the weigh of specified queue
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_QUEUE_ID     - invalid queue id
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Applicable:
 *      8380
 * Note:
 *    If weight == 0, means the queue is STRICT_PRIORITY
 */
int32 
rtk_qos_igrQueueWeight_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, uint32 qWeight)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_igrQueueWeight_set(unit, phy_port, queue, qWeight);
    }
#else
    return RT_MAPPER(unit)->qos_igrQueueWeight_set(unit, port, queue, qWeight);
#endif
}/* end of rtk_qos_igrQueueWeigh_set */


/* Function Name:
 *      rtk_qos_invldDscpVal_get
 * Description:
 *      Get the invalid dscp value in the specified device
 * Input:
 *      unit      - unit id
 * Output:
 *      pDscp     - pointer to dscp value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None.
 */
int32
rtk_qos_invldDscpVal_get(uint32 unit, uint32 *pDscp) 
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_invldDscpVal_get( unit, pDscp);
}/* end of rtk_qos_invldDscpVal_get */



/* Function Name:
 *      rtk_qos_invldDscpVal_set
 * Description:
 *      Set the invalid dscp value in the specified device
 * Input:
 *      unit     - unit id
 *      dscp     - dscp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - input dscp out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None.
 */
int32
rtk_qos_invldDscpVal_set(uint32 unit, uint32 dscp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_invldDscpVal_set( unit, dscp);
}/* end of rtk_qos_invldDscpVal_set */

/* Function Name:
 *      rtk_qos_invldDscpMask_get
 * Description:
 *      Get the invalid dscp mask in the specified device
 * Input:
 *      unit      - unit id
 * Output:
 *      pDscpMask     - pointer to dscp mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None.
 */
int32
rtk_qos_invldDscpMask_get(uint32 unit, uint32 *pDscpMask) 
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_invldDscpMask_get( unit, pDscpMask);
}/* end of rtk_qos_invldDscpMask_get */

/* Function Name:
 *      rtk_qos_invldDscpMask_set
 * Description:
 *      Set the invalid dscp mask in the specified device
 * Input:
 *      unit     - unit id
 *      dscpMask     - dscp mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_QOS_DSCP_VALUE - input dscp mask out of range
 * Applicable:
 *      8380
 * Note:
 *      None.
 */
int32
rtk_qos_invldDscpMask_set(uint32 unit, uint32 dscpMask) 
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_invldDscpMask_set( unit, dscpMask);
}/* end of rtk_qos_invldDscpMask_set */

/* Function Name:
 *      rtk_qos_portInvldDscpEnable_get
 * Description:
 *      Get invalid DSCP status for a port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status of invalid DSCP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      The status of invalid DSCP:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_portInvldDscpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portInvldDscpEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->qos_portInvldDscpEnable_get(unit, port, pEnable);
#endif
}

/* Function Name:
 *      rtk_qos_portInvldDscpEnable_set
 * Description:
 *      Set invalid DSCP status for a port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of invalid DSCP
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - Invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      The status of invalid DSCP:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_portInvldDscpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable) 
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->qos_portInvldDscpEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->qos_portInvldDscpEnable_set(unit, port, enable);
#endif
}

/* Function Name:
 *      rtk_qos_invldDscpEnable_get
 * Description:
 *      Get invalid DSCP status
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status of invalid DSCP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The status of invalid DSCP:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_invldDscpEnable_get(uint32 unit, rtk_enable_t *pEnable) 
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_invldDscpEnable_get( unit, pEnable);
}/* end of rtk_qos_invldDscpEnable_get */

/* Function Name:
 *      rtk_qos_invldDscpEnable_set
 * Description:
 *      Set invalid DSCP status 
 * Input:
 *      unit   - unit id
 *      enable - status of invalid DSCP
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      The status of invalid DSCP:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_qos_invldDscpEnable_set(uint32 unit, rtk_enable_t enable) 
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_invldDscpEnable_set( unit, enable);
}/* end of rtk_qos_invldDscpEnable_set */


/* Function Name:
 *      rtk_qos_portInnerPriRemapEnable_get
 * Description:
 *      Get port-base inner priority remapping status
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - port-base inner priority remapping status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      port-base inner priority remapping status:
 *      - DISABLED
 *      - ENABLED
 */
int32 
rtk_qos_portInnerPriRemapEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portInnerPriRemapEnable_get( unit, pEnable);
}/* end of rtk_qos_portInnerPriRemapEnable_get */


/* Function Name:
 *      rtk_qos_portInnerPriRemapEnable_set
 * Description:
 *      Set port-base inner priority remapping status
 * Input:
 *      unit   - unit id
 *      enable - port-base inner priority remapping status
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      port-base inner priority remapping status:
 *      - DISABLED
 *      - ENABLED
 */
int32 
rtk_qos_portInnerPriRemapEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portInnerPriRemapEnable_set( unit, enable);
}/* end of rtk_qos_portInnerPriRemapEnable_set */


/* Function Name:
 *      rtk_qos_portOuterPriRemapEnable_get
 * Description:
 *      Get port-base outer priority remapping status
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - port-base outer priority remapping status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      port-base outer priority remapping status:
 *      - DISABLED
 *      - ENABLED
 */
int32 
rtk_qos_portOuterPriRemapEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuterPriRemapEnable_get( unit, pEnable);
}/* end of rtk_qos_portOuterPriRemapEnable_get */


/* Function Name:
 *      rtk_qos_portOuterPriRemapEnable_set
 * Description:
 *      Get port-base outer priority remapping status
 * Input:
 *      unit   - unit id
 *      enable - port-base outer priority remapping status
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      port-base outer priority remapping status:
 *      - DISABLED
 *      - ENABLED
 */
int32 
rtk_qos_portOuterPriRemapEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_portOuterPriRemapEnable_set( unit, enable);
}/* end of rtk_qos_portOuterPriRemapEnable_set */


/* Function Name:
 *      rtk_qos_queueStrictEnable_get
 * Description:
 *      Get enable status of egress queue strict priority.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      queue - queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue strict priority.
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
rtk_qos_queueStrictEnable_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_queueStrictEnable_get(unit, port, queue, pEnable);
}/* end of rtk_qos_queueStrictEnable_get */

/* Function Name:
 *      rtk_qos_queueStrictEnable_set
 * Description:
 *      Set enable status of egress queue strict priority.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of egress queue strict priority
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
rtk_qos_queueStrictEnable_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->qos_queueStrictEnable_set(unit, port, queue, enable);
}/* end of rtk_qos_queueStrictEnable_set */

