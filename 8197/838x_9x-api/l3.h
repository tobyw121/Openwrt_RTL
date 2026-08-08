/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 30946 $
 * $Date: 2012-07-13 13:32:32 +0800 (Fri, 13 Jul 2012) $
 *
 * Purpose : Definition those public Layer3 routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Layer3 routing error handling
 */

#ifndef __RTK_L3_H__
#define __RTK_L3_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */
typedef enum rtk_l3_ttlExpireType_e
{
    TTL_EXPIRE_UCAST = 0,
    TTL_EXPIRE_MCAST,
    TTL_EXPIRE_END
} rtk_l3_ttlExpireType_t;

/*
 * Data Declaration
 */

/* L3 route entry */
typedef struct rtk_l3_routeEntry_s
{
    rtk_mac_t   hostMac;    /* next hop host mac address */
#if defined(CONFIG_SDK_RTL8390)
    uint32      swMac_idx;  /* index to switch MAC address */
#endif
} rtk_l3_routeEntry_t;

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_l3_init
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
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Must initialize L3 module before calling any L3 APIs.
 */
extern int32
rtk_l3_init(uint32 unit);

/* Module Name    : Layer3 routing                */
/* Sub-module Name: Layer3 routing error handling */

/* Function Name:
 *      rtk_l3_ttlExpireAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid TTL expire type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Type of IP ttl expire is as following:
 *          - TTL_EXPIRE_UCAST
 *          - TTL_EXPIRE_MCAST
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 */
extern int32
rtk_l3_ttlExpireAction_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t *pAction);

/* Function Name:
 *      rtk_l3_ttlExpireAction_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_INPUT      - invalid TTL expire type
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      (1) Type of IP ttl expire is as following:
 *          - TTL_EXPIRE_UCAST
 *          - TTL_EXPIRE_MCAST
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 */
extern int32
rtk_l3_ttlExpireAction_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_action_t action);

/* Function Name:
 *      rtk_l3_ttlExpireTrapPri_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid TTL expire type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapPri_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_l3_ttlExpireTrapPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid TTL expire type
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapPri_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_pri_t priority);

/* Function Name:
 *      rtk_l3_ttlExpireTrapPriEnable_get
 * Description:
 *      Get priority enable status of trapped packets when IP ttl expire.
 * Input:
 *      unit    - unit id
 *      type    - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of defining trap priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid TTL expire type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapPriEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_l3_ttlExpireTrapPriEnable_set
 * Description:
 *      Set priority define status when IP ttl expire traped to cpu.
 * Input:
 *      unit   - unit id
 *      type   - type of IP ttl expire
 *      enable -  priority status of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid TTL expire type
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapPriEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_l3_ttlExpireTrapDP_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid TTL expire type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapDP_get(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 *pDp);

/* Function Name:
 *      rtk_l3_ttlExpireTrapDP_set
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_INPUT           - invalid TTL expire type
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapDP_set(uint32 unit, rtk_l3_ttlExpireType_t type, uint32 dp);

/* Function Name:
 *      rtk_l3_ttlExpireTrapDPEnable_get
 * Description:
 *      Get drop procedence enable status of trapped packets when IP ttl expire.
 * Input:
 *      unit    - unit id
 *      type    - type of IP ttl expire
 * Output:
 *      pEnable - pointer to enable status of defining trap drop procedence
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid TTL expire type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapDPEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_l3_ttlExpireTrapDPEnable_set
 * Description:
 *      Set drop procedence define status when IP ttl expire traped to cpu.
 * Input:
 *      unit   - unit id
 *      type   - type of IP ttl expire
 *      enable - drop procedence status of trapped packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid TTL expire type
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireTrapDPEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_l3_ttlExpireAddCPUTagEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireAddCPUTagEnable_get(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_l3_ttlExpireAddCPUTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      Type of IP ttl expire is as following:
 *      - TTL_EXPIRE_UCAST
 *      - TTL_EXPIRE_MCAST
 */
extern int32
rtk_l3_ttlExpireAddCPUTagEnable_set(uint32 unit, rtk_l3_ttlExpireType_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_l3_routeEntry_get
 * Description:
 *      Get L3 rounting entry.
 * Input:
 *      unit   - unit id
 *      index  - index of host MAC address
 * Output:
 *      pEntry - L3 route entry
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
 *      The route entry is indexed by L2 NextHop entry.
 *      Valid index range:
 *      - 0~2047 in 8390
 *      - 0~511 in 8380
 */
extern int32
rtk_l3_routeEntry_get(uint32 unit, uint32 index, rtk_l3_routeEntry_t *pEntry);

/* Function Name:
 *      rtk_l3_routeEntry_set
 * Description:
 *      Set L3 rounting entry.
 * Input:
 *      unit   - unit id
 *      index  - index of host MAC address
 *      pEntry - L3 route entry
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
 *      8390, 8380
 * Note:
 *      The route entry is indexed by L2 NextHop entry.
 *      Valid index range:
 *      - 0~2047 in 8390
 *      - 0~511 in 8380
 */
extern int32
rtk_l3_routeEntry_set(uint32 unit, uint32 index, rtk_l3_routeEntry_t *pEntry);

/* Function Name:
 *      rtk_l3_routeSwitchMacAddr_get
 * Description:
 *      Get rounting switch MAC address.
 * Input:
 *      unit  - unit id
 *      index - index of switch MAC address
 * Output:
 *      pMac  - pointer to switch MAC address
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
 *      Multiple switch MAC addresses are supported for unicast routing.
 *      Valid index value is 0~15 in 8390.
 */
extern int32
rtk_l3_routeSwitchMacAddr_get(uint32 unit, uint32 index, rtk_mac_t *pMac);

/* Function Name:
 *      rtk_l3_routeSwitchMacAddr_set
 * Description:
 *      Set rounting switch MAC address.
 * Input:
 *      unit  - unit id
 *      index - index of switch MAC address
 *      pMac  - pointer to switch MAC address
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
 *      8390
 * Note:
 *      Multiple switch MAC addresses are supported for unicast routing.
 *      Valid index value is 0~15 in 8390.
 */
extern int32
rtk_l3_routeSwitchMacAddr_set(uint32 unit, uint32 index, rtk_mac_t *pMac);

#endif /* __RTK_L3_H__ */
