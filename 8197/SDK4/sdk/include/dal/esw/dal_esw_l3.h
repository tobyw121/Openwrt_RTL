/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 10406 $
 * $Date: 2010-06-23 19:26:54 +0800 (Wed, 23 Jun 2010) $
 *
 * Purpose : Definition those public L3 APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) L3 routing
 *
 */

#ifndef __DAL_ESW_L3_H__
#define __DAL_ESW_L3_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/l3.h>


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_esw_l3_init
 * Description:
 *      Initialize L3 module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize L3 module before calling any L3 APIs.
 */
extern int32
dal_esw_l3_init(uint32 unit);

/* Module Name    : Layer3 routing                */
/* Sub-module Name: Layer3 routing error handling */

/* Function Name:
 *      dal_esw_l3_ttlExpireAction_get
 * Description:
 *      Get forwarding action when IP ttl expire.
 * Input:
 *      unit    - unit id
 *      type    - type of IP ttl expire
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 *      
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_esw_l3_ttlExpireAction_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t *pAction);

/* Function Name:
 *      dal_esw_l3_ttlExpireAction_set
 * Description:
 *      Set forwarding action when IP ttl expire.
 * Input:
 *      unit   - unit id
 *      type   - type of IP ttl expire
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 *      
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_esw_l3_ttlExpireAction_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t action);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPri_get
 * Description:
 *      Get priority of trapped packets when IP ttl expire.
 * Input:
 *      unit      - unit id
 *      type      - type of IP ttl expire
 * Output:
 *      pPriority - pointer to priority of trapped packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapPri_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPri_set
 * Description:
 *      Set priority of trapped packets when IP ttl expire.
 * Input:
 *      unit     - unit id
 *      type     - type of IP ttl expire
 *      priority - priority of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapPri_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPriEnable_get
 * Description:
 *      Get priority enable status of trapped packets when IP ttl expire.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of defining trap priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapPriEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapPriEnable_set
 * Description:
 *      Set priority define status when IP ttl expire traped to cpu.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 *      enable        -  priority status of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapPriEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDP_get
 * Description:
 *      Get drop precedence of trapped packet when IP ttl expire.
 * Input:
 *      unit - unit id
 *      type - type of IP ttl expire
 * Output:
 *      pDp  - pointer to drop precedence of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapDP_get(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 *pDp);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDP_set
 * Description:
 *      Set drop precedence of trapped packet when IP ttl expire.
 * Input:
 *      unit - unit id
 *      type - type of IP ttl expire
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapDP_set(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 dp);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDPEnable_get
 * Description:
 *      Get drop procedence enable status of trapped packets when IP ttl expire.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of defining trap drop procedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid TTL expire type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapDPEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l3_ttlExpireTrapDPEnable_set
 * Description:
 *      Set drop procedence define status when IP ttl expire traped to cpu.
 * Input:
 *      unit            - unit id
 *      type            - type of IP ttl expire
 *      enable        -  drop procedence status of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 *      RT_ERR_INPUT            - invalid TTL expire type
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
dal_esw_l3_ttlExpireTrapDPEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_l3_ttlExpireAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      type    - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_l3_ttlExpireAddCPUTagEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l3_ttlExpireAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      type   - type of IP ttl expire
 *      enable - enable status of CPU tag adding
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
dal_esw_l3_ttlExpireAddCPUTagEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable);

#endif /*__DAL_ESW_L3_H__*/

