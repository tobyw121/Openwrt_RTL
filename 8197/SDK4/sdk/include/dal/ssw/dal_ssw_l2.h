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
 * $Revision: 29728 $
 * $Date: 2012-06-11 18:27:30 +0800 (Mon, 11 Jun 2012) $
 *
 * Purpose : Definition those public L2 APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) l2 address table
 * 
 */ 
#ifndef __DAL_SSW_L2_H__ 
#define __DAL_SSW_L2_H__

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/l2.h>

/* 
 * Symbol Definition 
 */

/* 
 * Data Declaration 
 */

/*
 * Macro Definition
 */
 
 
/* 
 * Function Declaration 
 */

/* Module Name    : L2     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_ssw_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize l2 module before calling any l2 APIs.
 */
extern int32
dal_ssw_l2_init(uint32 unit);


/* Function Name:
 *      dal_ssw_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      *pEnable - pointer buffer of state of HW clear linkdown port mac
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Make sure chip have supported the function before using the API.
 *      2. The API is apply to whole system.
 *      3. The status of flush linkdown port address is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_ssw_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      unit   - unit id
 *      enable - configure value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. Make sure chip have supported the function before using the API.
 *      2. The API is apply to whole system.
 *      3. The status of flush linkdown port address is as following:
 *		- DISABLED
 *		- ENABLED
 */
extern int32
dal_ssw_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_ssw_l2_ucastAddr_flush
 * Description:
 *      Flush unicast address
 * Input:
 *      unit    - unit id
 *      pConfig - flush config
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      (1) The API have the same function as dal_ssw_l2_flushType_set, 
 *          but use structure argument for future new chip extension.
 *      (2) Some fields of structure is inactive in ssw series chips as following:
 *          - pConfig->flushByMac
 *          - pConfig->ucastAddr
 *          - pConfig->flushStaticAddr
 *          - pConfig->flushAddrOnAllPorts
 */
extern int32
dal_ssw_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig);


/* Function Name:
 *      dal_ssw_l2_portLearningCnt_get
 * Description:
 *      Get the mac learning counts of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      *pMac_cnt - pointer buffer of mac learning counts of the port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The mac learning counts only calculate dynamic mac numbers.
 *      2. RTL8329/RTL8389 chip have support the counter per port, but don't provide
 *         API to get right now.
 */
extern int32
dal_ssw_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt);


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCnt_get
 * Description:
 *      Get the maximum mac learning counts of the port from the specified device.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      *pMac_cnt - pointer buffer of maximum mac learning counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The maximum mac learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the port.
 */
extern int32
dal_ssw_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt);


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCnt_set
 * Description:
 *      Set the maximum mac learning counts of the port to the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mac_cnt - maximum mac learning counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_LIMITED_L2ENTRY_NUM - invalid limited L2 entry number
 * Note:
 *      1. The maximum mac learning counts only limit for dynamic learning mac
 *      address, not apply to static mac address.
 *      2. Set the mac_cnt to 0 mean disable learning in the port.
 */
extern int32
dal_ssw_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt);


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCntAction_get
 * Description:
 *      Get the action when over learning maximum mac counts of the port from the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      *pAction - pointer buffer of action when over learning maximum mac counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
extern int32
dal_ssw_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);


/* Function Name:
 *      dal_ssw_l2_portLimitLearningCntAction_set
 * Description:
 *      Set the action when over learning maximum mac counts of the port to the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - action when over learning maximum mac counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
extern int32
dal_ssw_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action);


/* Function Name:
 *      dal_ssw_l2_aging_get
 * Description:
 *      Get the dynamic address aging time from the specified device.
 * Input:
 *      unit         - unit id
 * Output:
 *      *pAging_time - pointer buffer of aging time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Get aging_time as 0 mean disable aging mechanism. (seconds)
 */
extern int32
dal_ssw_l2_aging_get(uint32 unit, uint32 *pAging_time);


/* Function Name:
 *      dal_ssw_l2_aging_set
 * Description:
 *      Set the dynamic address aging time to the specified device.
 * Input:
 *      unit       - unit id
 *      aging_time - aging time
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      1. RTL8329/RTL8389 aging time is not configurable.
 *      2. apply aging_time as 0 mean disable aging mechanism.
 */
extern int32
dal_ssw_l2_aging_set(uint32 unit, uint32 aging_time);


/* Module Name    : L2      */
/* Sub-module Name: Unicast */

/* Function Name:
 *      dal_ssw_l2_addr_add
 * Description:
 *      Add L2 entry to ASIC.
 * Input:
 *      unit      - unit id
 *      pL2_addr - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 entry before add it.
 */
extern int32
dal_ssw_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);

/* Function Name:
 *      dal_ssw_l2_authAddr_add
 * Description:
 *      Add a L2 unicast auth address entry to the specified device.
 * Input:
 *      unit      - unit id
 *      vid       - vlan id
 *      *pMac     - mac address
 *      port      - port id
 *      is_static - the attribute of mac address is static or dynamic.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_L2_NO_EMPTY_ENTRY - no empty entry in L2 table
 * Note:
 *      1. vid is same as fid in IVL mode.
 */
extern int32
dal_ssw_l2_authAddr_add(
    uint32      unit,
    rtk_vlan_t  vid,
    rtk_mac_t   *pMac,
    rtk_port_t  port,
    uint32      is_static);

/* Function Name:
 *      dal_ssw_l2_addr_del
 * Description:
 *      Delete a L2 unicast address entry from the specified device.
 * Input:
 *      unit   - unit id
 *      vid    - vlan id
 *      *pMac  - mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. vid is same as fid in IVL mode.
 *      2. For IVL and SVL co-work mode, need to discuss API late.
 */
extern int32
dal_ssw_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);


/* Function Name:
 *      dal_ssw_l2_addr_get
 * Description:
 *      Get a L2 unicast address entry from the specified device.
 * Input:
 *      unit      - unit id
 *      *pL2_data - structure of l2 address data
 * Output:
 *      *pL2_data - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. vid is same as fid in IVL mode.
 *      2. For IVL and SVL co-work mode, need to discuss API late.
 *      3. The *pL2_data.vid and *pL2_data.mac is input key
 *      4. The *pL2_data.port, *pL2_data.auth, *pL2_data.sa_block, 
 *         *pL2_data.da_block and *pL2_data.is_static is output.
 */
extern int32
dal_ssw_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_data);


/* Function Name:
 *      dal_ssw_l2_addr_set
 * Description:
 *      Update content of L2 entry.
 * Input:
 *      unit     - unit id
 *      pL2_addr - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vlan id
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_INPUT             - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_addr_set(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);


/* Function Name:
 *      dal_ssw_l2_addr_delAll
 * Description:
 *      Delete all L2 unicast address entry from the specified device.
 * Input:
 *      unit           - unit id
 *      include_static - include static mac or not?
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_addr_delAll(uint32 unit, uint32 include_static);


/* Function Name:
 *      dal_ssw_l2_nextValidAddr_get
 * Description:
 *      Get next valid L2 unicast address entry from the specified device.
 * Input:
 *      unit                - unit id
 *      *pScan_idx          - currently scan index of l2 table to get next.
 *      include_static      - the get type, include static mac or not.
 * Output:
 *      *pL2_data           - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 multicast and ip multicast entry and 
 *         reply next valid L2 unicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
extern int32
dal_ssw_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              include_static,
    rtk_l2_ucastAddr_t  *pL2_data);


/* Function Name:
 *      dal_ssw_l2_nextValidMcastAddr_get
 * Description:
 *      Get next valid L2 multicast address entry from the specified device.
 * Input:
 *      unit                - unit id
 *      *pScan_idx          - currently scan index of l2 table to get next.
 * Output:
 *      *pL2_data           - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 unicast and ip multicast entry and 
 *         reply next valid L2 multicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
extern int32
dal_ssw_l2_nextValidMcastAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_mcastAddr_t  *pL2_data);


/* Function Name:
 *      dal_ssw_l2_nextValidIpMcastAddr_get
 * Description:
 *      Get next valid L2 ip multicast address entry from the specified device.
 * Input:
 *      unit                - unit id
 *      *pScan_idx          - currently scan index of l2 table to get next.
 * Output:
 *      *pL2_data           - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. The function will skip valid l2 unicast and multicast entry and 
 *         reply next valid L2 ip multicast address is based on index order of l2 table.
 *      2. Please input -1 for get the first entry of l2 table.
 *      3. The *pScan_idx is the input and also is the output argument.
 */
extern int32
dal_ssw_l2_nextValidIpMcastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ipMcastAddr_t    *pL2_data);


/* Module Name    : L2           */
/* Sub-module Name: l2 multicast */

/* Function Name:
 *      dal_ssw_l2_mcastAddr_add
 * Description:
 *      Add L2 multicast entry to ASIC.
 * Input:
 *      unit       - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
extern int32
dal_ssw_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);


/* Function Name:
 *      dal_ssw_l2_mcastAddr_get
 * Description:
 *      Update content of L2 multicast entry.
 * Input:
 *      unit       - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

/* Function Name:
 *      dal_ssw_l2_mcastAddr_set
 * Description:
 *      Get L2 multicast entry based on specified vid and MAC address
 * Input:
 *      unit       - unit id
 * Output:
 *      pMcast_addr - pointer to L2 multicast entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_mcastAddr_set(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

/* Function Name:
 *      dal_ssw_l2_mcastAddr_del
 * Description:
 *      Delete a L2 multicast address entry from the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      *pMac - multicast mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_L2_HASH_KEY    - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);

/* Function Name:
 *      dal_ssw_l2_mcastAddr_add_with_index
 * Description:
 *      Add a L2 multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                   - unit id
 *      vid                    - vlan id
 *      pMcast_addr            - content of L2 multicast address entry
 * Output:
 *      pMacast_addr->fwdIndex - index of multicast forwarding entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index 
 *          and won't config portmask.
 *
 *      If fwdIndex is smaller than 0, will allocate a free index and return it.
 *          It will also config portmask.
 */
extern int32
dal_ssw_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

/* Function Name:
 *      dal_esw_l2_mcastAddr_get_with_index
 * Description:
 *      Get a L2 multicast address entry and multicast index from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pMcast_addr - pointer to content of L2 multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_ENTRY_NOTFOUND    - specified entry not found
 * Note:
 *      None.
 */
extern int32
dal_ssw_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

/* Module Name    : L2           */
/* Sub-module Name: ip multicast */

/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_add
 * Description:
 *      Add IP multicast entry to ASIC.
 * Input:
 *      unit       - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
extern int32
dal_ssw_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr);


/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_get
 * Description:
 *      Get IP multicast entry on specified dip and sip.
 * Input:
 *      unit       - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
extern int32
dal_ssw_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr);

/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_set
 * Description:
 *      Update content of IP multicast entry.
 * Input:
 *      unit       - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_IPV4_ADDRESS - Invalid IPv4 address
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr);


/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_del
 * Description:
 *      Delete a L2 ip multicast address entry from the specified device.
 * Input:
 *      unit - unit id
 *      sip  - source ip address
 *      dip  - destination ip address
 *      vid  - vlan id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_L2_HASH_KEY    - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      1. In vlan unaware mode (SVL), the vid will be ignore, suggest to 
 *         input vid=0 in vlan unaware mode.
 *      2. In vlan aware mode (IVL), the vid will be care.
 */
extern int32
dal_ssw_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid);

/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_add_with_index
 * Description:
 *      Add a IP multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                     - unit id
 *      vid                      - vlan id
 *      pIpMcast_addr            - content of IP multicast address entry
 * Output:
 *      pIpMacast_addr->fwdIndex - index of multicast forwarding entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index 
 *          and won't config portmask.
 *
 *      If fwdIndex is smaller than 0, will allocate a free index and return it.
 *          It will also config portmask.
 */
extern int32
dal_ssw_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

/* Function Name:
 *      dal_ssw_l2_ipMcastAddr_get_with_index
 * Description:
 *      Get a IP multicast address entry and multicast index from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pIpMcast_addr - pointer to content of IP multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_HASH_KEY          - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_ENTRY_NOTFOUND    - specified entry not found
 * Note:
 *      None.
 */
extern int32
dal_ssw_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

/* Function Name:
 *      dal_ssw_l2_mcastFwdIndex_alloc
 * Description:
 *      Allocate index for multicast forwarding entry
 * Input:
 *      unit           - unit id
 *      pFwdIndex      - pointer to index of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX       - invalid index of multicast forwarding entry
 *      RT_ERR_L2_MCAST_FWD_ENTRY_EXIST - index of forwarding entry is used.
 * Note:
 *      If *pFwdIndex is larger than or equal to 0, will use *pFwdIndex as multicast index.
 *      If *pFwdIndex is smaller than 0, will allocate a free index and return it.
 */
extern int32
dal_ssw_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex);

/* Function Name:
 *      dal_ssw_l2_mcastFwdIndex_free
 * Description:
 *      Free index for multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      index      - index of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                      - invalid unit id
 *      RT_ERR_L2_MULTI_FWD_INDEX           - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST - index of forwarding entry is not exist
 * Note:
 */
extern int32
dal_ssw_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index);

/* Function Name:
 *      dal_ssw_l2_mcastFwdIndexFreeCount_get
 * Description:
 *      Get free count of multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      pFreeCount - pointer to free count of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount);

/* Function Name:
 *      dal_ssw_l2_mcastFwdPortmask_set
 * Description:
 *      Get portmask of multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      index      - index of multicast forwarding portmask
 *      *pPortmask - pointer buffer of ip multicast ports
 *      crossVlan  - cross vlan flag of ip multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      The crossVlan will be ignore in ssw.
 */
extern int32
dal_ssw_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          crossVlan);

/* Function Name:
 *      dal_ssw_l2_mcastFwdPortmask_get
 * Description:
 *      Get portmask of multicast forwarding entry
 * Input:
 *      unit        - unit id
 *      index       - index of multicast forwarding portmask
 * Output:
 *      *pPortmask  - pointer buffer of ip multicast ports
 *      *pCrossVlan - pointer of cross vlan flag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 *      The pCrossVlan will be ignore in ssw.
 */
extern int32
dal_ssw_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          *pCrossVlan);

/* Function Name:
 *      dal_ssw_l2_cpuMacAddr_add
 * Description:
 *      Add a CPU mac adress of the vlan id to the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      *pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);


/* Function Name:
 *      dal_ssw_l2_cpuMacAddr_del
 * Description:
 *      Delete a CPU mac adress of the vlan id from the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      *pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_VLAN_VID - invalid vid
 * Note:
 *      None
 */
extern int32
dal_ssw_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_get
 * Description:
 *      Get flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit             - unit id     
 *      type        - type of lookup miss
 * Output:
 *      *pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32 
dal_ssw_l2_lookupMissFloodPortMask_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask);

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_set
 * Description:
 *      Set flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit             - unit id  
 *      type        - type of lookup miss
 *      *pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.   
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32 
dal_ssw_l2_lookupMissFloodPortMask_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask);

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_add
 * Description:
 *      Add one port member to flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit             - unit id   
 *      type        - type of lookup miss
 *      flood_port       - port id that is going to be added in flooding port mask.  
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32 
dal_ssw_l2_lookupMissFloodPortMask_add(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port);

/* Function Name:
 *      dal_ssw_l2_lookupMissFloodPortMask_del
 * Description:
 *      Del one port member in flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit             - unit id   
 *      type        - type of lookup miss
 *      flood_port       - port id that is going to be added in flooding port mask.  
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32 
dal_ssw_l2_lookupMissFloodPortMask_del(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port);

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_get
 * Description:
 *      Get source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id     
 * Output:
 *      *pFilter_portmask - source port egress filtering configuration when packet egress.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32 
dal_ssw_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFilter_portmask);

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_set
 * Description:
 *      Set source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id  
 *      *pFilter_portmask - source port egress filtering configuration when packet egress.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32 
dal_ssw_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFilter_portmask);

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_add
 * Description:
 *      Add one port member to source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id   
 *      filter_port       - port id that is going to be added in source port egress filtering mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32 
dal_ssw_l2_srcPortEgrFilterMask_add(uint32 unit, rtk_port_t filter_port);

/* Function Name:
 *      dal_ssw_l2_srcPortEgrFilterMask_del
 * Description:
 *      Del one port member in source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id   
 *      filter_port       - - port id that is going to be deleted in source port egress filtering mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32 
dal_ssw_l2_srcPortEgrFilterMask_del(uint32 unit, rtk_port_t filter_port);

/* Function Name:
 *      dal_ssw_l2_addrEntry_get
 * Description:
 *      Get the L2 table entry by index of the specified unit.
 * Input:
 *      unit  - unit id
 *      index - l2 table index
 * Output:
 *      pL2_entry - pointer buffer of l2 table entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1) The index valid range is from 0 to (L2 hash table size + L2 CAM table size - 1)
 *         - 0 ~ (L2 hash table size - 1) entry in L2 hash table
 *         - (L2 hash table size) ~ (L2 hash table size + L2 CAM table size - 1) entry in L2 CAM table
 *      2) The output entry have 2 variables (valid and entry_type) and its detail data structure
 *         - valid: 1 mean the entry is valid; 0: invalid
 *         - entry_type: FLOW_TYPE_UNICAST, FLOW_TYPE_L2_MULTI and FLOW_TYPE_IP_MULTI
 *                       the field is ignored if valid field is 0.
 *         - detail data structure is ignored if valid is 0, and its filed meanings is depended
 *           on the entry_type value.
 */
extern int32
dal_ssw_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry);

/* Function Name:
 *      dal_ssw_l2_conflictAddr_get
 * Description:
 *      Get the conflict L2 table entry from one given L2 address in the specified unit.
 * Input:
 *      unit            - unit id
 *      pL2Addr         - l2 address to find its conflict entries
 *      cfAddrList_size - buffer size of the pCfAddrList
 * Output:
 *      pCfAddrList - pointer buffer of the conflict l2 table entry list
 *      pCf_retCnt  - return number of find conflict l2 table entry list
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) The function can be used if add l2 entry return RT_ERR_L2_NO_EMPTY_ENTRY.
 *          Input the pL2Addr->entry_type and its hash key to get conflict entry information.
 *      (2) User want to prepare the return buffer pCfAddrList and via. cfAddrList_size argument
 *          tell driver its size.
 *      (3) The function will return valid L2 hash entry from the same bucket and the return number
 *          is filled in pCf_retCnt, entry data is filled in pCfAddrList.
 */
extern int32
dal_ssw_l2_conflictAddr_get(
    uint32          unit,
    rtk_l2_entry_t  *pL2Addr,
    rtk_l2_entry_t  *pCfAddrList,
    uint32          cfAddrList_size,
    uint32          *pCf_retCnt);

/* Function Name:
 *      dal_ssw_port_flowCtrlEnable_get
 * Description:
 *      Get the flow control status of the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pEnable - pointer to the status of the flow control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      The API get the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
extern int32
dal_ssw_port_flowCtrlEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_port_flowCtrlEnable_set
 * Description:
 *      Set the flow control status to the specific port
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      enable    - enable status of flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      The API is apply the flow control status by port based, no matter N-WAY is enabled or disabled.
 */
extern int32
dal_ssw_port_flowCtrlEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

#endif /* __DAL_SSW_L2_H__ */
