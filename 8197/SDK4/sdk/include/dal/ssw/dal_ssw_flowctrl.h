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
 * $Revision: 20135 $
 * $Date: 2011-07-28 17:51:53 +0800 (Thu, 28 Jul 2011) $
 *
 * Purpose : Definition those public global APIs and its data type in the SDK.
 *
 * Feature : (1) Include chip-supported conditions for flow control on/off
 *           (2) Get/set the threshold parameters for the flow control on/off
 *
 */

#ifndef __DAL_SSW_FLOWCTRL_H__
#define __DAL_SSW_FLOWCTRL_H__


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
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_ssw_flowctrl_init
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
dal_ssw_flowctrl_init(uint32 unit);

/*
 * Flow Control ON
 */

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemPauseThresh_get
 * Description:
 *      Get ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit           - unit id
 * Output:
 *      *pThresh       - pointer to the threshold structure in the system used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_igrSystemPauseThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemPauseThresh_set
 * Description:
 *      Set ingress system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit          - unit id
 *      *pThresh      - pointer to the threshold structure in the system used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE   - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_igrSystemPauseThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh);


/* Function Name:
 *      dal_ssw_flowctrl_igrPortPauseThresh_get
 * Description:
 *      Get used page high/low threshold for the specified ingress port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      *pThresh       - pointer to the threshold structure for the port used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_igrPortPauseThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_igrPortPauseThresh_set
 * Description:
 *      Set used page high/low threshold for the specified ingress port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      *pThresh      - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE   - input parameter out of range
 *      RT_ERR_INPUT          - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 lowOn and lowOff threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned
 */
extern int32
dal_ssw_flowctrl_igrPortPauseThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);


/*
 * Flow Control OFF
 */

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemCongestThresh_get
 * Description:
 *      Get system used page high/low drop threshold paramters of the specific unit
 * Input:
 *      unit           - unit id
 * Output:
 *      *pThresh       - pointer to the threshold structure in the public used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_igrSystemCongestThresh_get(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_igrSystemCongestThresh_set
 * Description:
 *      Set system used page high/low threshold paramters of the specific unit
 * Input:
 *      unit          - unit id
 *      *pThresh      - pointer to the threshold structure in the public used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE   - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_igrSystemCongestThresh_set(uint32 unit, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_igrPortCongestThresh_get
 * Description:
 *      Get used page high drop threshold for the specified ingress port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      *pThresh       - pointer to the threshold structure for the port used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_igrPortCongestThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_igrPortCongestThresh_set
 * Description:
 *      Set used page high drop threshold for the specified ingress port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      *pThresh      - pointer to the threshold structure for the port used page count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE   - input parameter out of range
 *      RT_ERR_INPUT          - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 lowOn and lowOff threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned
 */
extern int32
dal_ssw_flowctrl_igrPortCongestThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_egrSystemDropThresh_get
 * Description:
 *      Get egress system drop threshold for the specified unit
 * Input:
 *      unit           - unit id
 * Output:
 *      *pThresh       - pointer to the drop threshold
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
dal_ssw_flowctrl_egrSystemDropThresh_get(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_egrSystemDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit         - unit id
 *      *pThresh     - pointer to the drop threshold
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
dal_ssw_flowctrl_egrSystemDropThresh_set(uint32 unit, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_egrPortDropThresh_get
 * Description:
 *      Get egress drop threshold for the specified egress port
 * Input:
 *      unit           - unit id
 *      port           - port id
 * Output:
 *      *pThresh       - pointer to the drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_PORT_ID         - invalid port id
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_egrPortDropThresh_get(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_egrPortDropThresh_set
 * Description:
 *      Set egress drop threshold for the specified egress port
 * Input:
 *      unit         - unit id
 *      port         - port id
 *      *pThresh     - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PORT_ID        - invalid port id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE   - input parameter out of range
 *      RT_ERR_INPUT          - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 high and low drop threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned.
 */
extern int32
dal_ssw_flowctrl_egrPortDropThresh_set(uint32 unit, rtk_port_t port, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_egrQueueDropThresh_get
 * Description:
 *      Get egress global drop threshold for the egress queue
 * Input:
 *      unit         - unit id
 *      queue        - queue id
 * Output:
 *      *pThresh     - pointer to the global drop threshold
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_QUEUE_ID       - invalid queue id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_flowctrl_egrQueueDropThresh_get(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);

/* Function Name:
 *      dal_ssw_flowctrl_egrQueueDropThresh_set
 * Description:
 *      Set egress gloabl drop threshold for the egress queue
 * Input:
 *      unit         - unit id
 *      queue        - queue id
 *      *pThresh     - pointer to the drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_QUEUE_ID       - invalid queue id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE   - input parameter out of range
 *      RT_ERR_INPUT          - invalid input parameter
 * Note:
 *      1. For RTL8389/RTL8329 high and low threshold should be set to the same value,
 *         otherwise RT_ERR_INPUT will be returned.
 */
extern int32
dal_ssw_flowctrl_egrQueueDropThresh_set(uint32 unit, rtk_qid_t queue, rtk_flowctrl_drop_thresh_t *pThresh);


#endif /* __DAL_SSW_FLOWCTRL_H__ */

