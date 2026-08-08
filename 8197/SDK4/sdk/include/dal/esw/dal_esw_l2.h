/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 21903 $
 * $Date: 2011-09-02 18:36:53 +0800 (Fri, 02 Sep 2011) $
 *
 * Purpose : Definition those public L2 APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Mac address flush
 *           2) Address learning limit
 *           3) Parameter for L2 lookup and learning engine
 *           4) Unicast address
 *           5) L2 multicast
 *           6) IP multicast
 *           7) Multicast forwarding table
 *           8) CPU mac
 *           9) Port move
 *           10) Parameter for lookup miss
 *           11) Parameter for MISC
 *
 */

#ifndef __DAL_ESW_L2_H__
#define __DAL_ESW_L2_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>
#include <rtk/l2.h>


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Module Name    : L2     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_esw_l2_init
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
dal_esw_l2_init(uint32 unit);

/* Module Name    : L2                */
/* Sub-module Name: Mac address flush */


/* Function Name:
 *      dal_esw_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer buffer of state of HW clear linkdown port mac
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Make sure chip have supported the function before using the API.
 *      2. The API is apply to whole system.
 *      3. The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_flushLinkDownPortAddrEnable_set
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
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_esw_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_l2_ucastAddr_flush
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
 *      None
 */
extern int32
dal_esw_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig);

/* Module Name    : L2                     */
/* Sub-module Name: Address learning limit */


/* Function Name:
 *      dal_esw_l2_portLearningCnt_get
 * Description:
 *      Get the mac learning counts of the port from the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pMac_cnt - pointer buffer of mac learning counts of the port
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
dal_esw_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt);


/* Function Name:
 *      dal_esw_l2_portLimitLearningCntEnable_get
 * Description:
 *      Get enable status of limiting MAC learning on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of limiting MAC learning
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
dal_esw_l2_portLimitLearningCntEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_portLimitLearningCntEnable_set
 * Description:
 *      Set enable status of limiting MAC learning on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of limiting MAC learning
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
dal_esw_l2_portLimitLearningCntEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_portLimitLearningCnt_get
 * Description:
 *      Get the maximum mac learning counts of the port from the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pMac_cnt - pointer buffer of maximum mac learning counts
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
dal_esw_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt);


/* Function Name:
 *      dal_esw_l2_portLimitLearningCnt_set
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
dal_esw_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt);


/* Function Name:
 *      dal_esw_l2_portLimitLearningCntAction_get
 * Description:
 *      Get the action when over learning maximum mac counts of the port from the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer buffer of action when over learning maximum mac counts
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
dal_esw_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);


/* Function Name:
 *      dal_esw_l2_portLimitLearningCntAction_set
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
 * Note:
 *      1. The action symbol as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
extern int32
dal_esw_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action);


/* Function Name:
 *      dal_esw_l2_portLastLearnedMac_get
 * Description:
 *      Get lastest learned MAC address on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pFid - lastest learned fid
 *      pMac - lastest learned mac address
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
dal_esw_l2_portLastLearnedMac_get(uint32 unit, rtk_port_t port, rtk_fid_t *pFid, rtk_mac_t *pMac);


/* Function Name:
 *      dal_esw_l2_fidLimitLearningEntry_get
 * Description:
 *      Get FID MAC limit entry.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 * Output:
 *      pFidMacLimitEntry - pointer to MAC limit entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
extern int32
dal_esw_l2_fidLimitLearningEntry_get(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry);


/* Function Name:
 *      dal_esw_l2_fidLimitLearningEntry_set
 * Description:
 *      Set FID MAC limit entry.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 *      pFidMacLimitEntry - MAC limit entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_FID                  - invalid fid
 *      RT_ERR_PORT_MASK            - invalid portmask
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - invalid limited L2 entry number
 *      RT_ERR_FWD_ACTION           - invalid forwarding action
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - LIMIT_LEARN_CNT_ACTION_DROP
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
extern int32
dal_esw_l2_fidLimitLearningEntry_set(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry);


/* Function Name:
 *      dal_esw_l2_fidLearningCnt_get
 * Description:
 *      Get number of learned MAC addresses on specified fid.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 * Output:
 *      pNum - number of learned MAC addresses
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_l2_fidLearningCnt_get(uint32 unit, uint32 fid_macLimit_idx, uint32 *pNum);


/* Function Name:
 *      dal_esw_l2_fidLearningCnt_reset
 * Description:
 *      Reset number of learned MAC addresses on specified entry of fid MAC limit.
 * Input:
 *      unit             - unit id
 *      fid_macLimit_idx - index of FID MAC limit entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_l2_fidLearningCnt_reset(uint32 unit, uint32 fid_macLimit_idx);


/* Function Name:
 *      dal_esw_l2_fidLastLearnedMac_get
 * Description:
 *      Get lastest learned MAC address on specifid fid.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of FID MAC limit entry
 * Output:
 *      pFid              - pointer to filter id
 *      pMac              - lastest learned mac address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_l2_fidLastLearnedMac_get(
    uint32      unit,
    uint32      fid_macLimit_idx,
    rtk_fid_t   *pFid,
    rtk_mac_t   *pMac);


/* Function Name:
 *      dal_esw_l2_limitLearningTrapPri_get
 * Description:
 *      Get priority of trapped packet when number mac address over limitation.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority of trapped packet
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
dal_esw_l2_limitLearningTrapPri_get(uint32 unit, rtk_pri_t *pPriority);


/* Function Name:
 *      dal_esw_l2_limitLearningTrapPri_set
 * Description:
 *      Set priority of trapped packet when number mac address over limitation.
 * Input:
 *      unit     - unit id
 *      priority - priority of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_limitLearningTrapPri_set(uint32 unit, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_l2_limitLearningTrapPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_limitLearningTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l2_limitLearningTrapPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_limitLearningTrapPriEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_limitLearningTrapDP_get
 * Description:
 *      Get drop precedence of trapped packet when number mac address over limitation.
 * Input:
 *      unit - unit id
 * Output:
 *      pDp  - drop precedence of trapped packet
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
dal_esw_l2_limitLearningTrapDP_get(uint32 unit, uint32 *pDp);


/* Function Name:
 *      dal_esw_l2_limitLearningTrapDP_set
 * Description:
 *      Set drop precedence of trapped packet when number mac address over limitation.
 * Input:
 *      unit - unit id
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      None
 */
extern int32
dal_esw_l2_limitLearningTrapDP_set(uint32 unit, uint32 dp);

/* Function Name:
 *      dal_esw_l2_limitLearningTrapDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
dal_esw_l2_limitLearningTrapDPEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l2_limitLearningTrapDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - drop procedence assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_limitLearningTrapDPEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_limitLearningTrapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_l2_limitLearningTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_limitLearningTrapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
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
dal_esw_l2_limitLearningTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

/* Module Name    : L2                                          */
/* Sub-module Name: Parameter for L2 lookup and learning engine */


/* Function Name:
 *      dal_esw_l2_aging_get
 * Description:
 *      Get the dynamic address aging time from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pAging_time - pointer buffer of aging time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Get aging_time as 0 mean disable aging mechanism. (seconds)
 */
extern int32
dal_esw_l2_aging_get(uint32 unit, uint32 *pAging_time);


/* Function Name:
 *      dal_esw_l2_aging_set
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
dal_esw_l2_aging_set(uint32 unit, uint32 aging_time);


/* Function Name:
 *      dal_esw_l2_camEnable_get
 * Description:
 *      Get enable status of CAM entry.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CAM entry
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
dal_esw_l2_camEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_camEnable_set
 * Description:
 *      Set enable status of CAM entry.
 * Input:
 *      unit   - unit id
 *      enable - enable status of CAM entry
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
dal_esw_l2_camEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_hashAlgo_get
 * Description:
 *      Get hash algorithm of layer2 switching.
 * Input:
 *      unit       - unit id
 * Output:
 *      pHash_algo - pointer to hash algorithm of layer2 switching
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
dal_esw_l2_hashAlgo_get(uint32 unit, uint32 *pHash_algo);


/* Function Name:
 *      dal_esw_l2_hashAlgo_set
 * Description:
 *      Set hash algorithm of layer2 switching.
 * Input:
 *      unit            - unit id
 *      hash_algo       - hash algorithm of layer2 switching
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
dal_esw_l2_hashAlgo_set(uint32 unit, uint32 hash_algo);


/* Function Name:
 *      dal_esw_l2_vlanMode_get
 * Description:
 *      Get vlan(inner/outer vlan) for L2 lookup on specified port.
 * Input:
 *      unit            - unit id
 *      port            - port id
 * Output:
 *      pVlanMode       - pointer to inner/outer vlan
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      - BASED_ON_INNER_VLAN
 *      - BASED_ON_OUTER_VLAN
 */
extern int32
dal_esw_l2_vlanMode_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pVlanMode);


/* Function Name:
 *      dal_esw_l2_vlanMode_set
 * Description:
 *      Set vlan(inner/outer vlan) for L2 lookup on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      vlanMode - inner/outer vlan
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
 *      Vlan mode is as following
 *      - BASED_ON_INNER_VLAN
 *      - BASED_ON_OUTER_VLAN
 */
extern int32
dal_esw_l2_vlanMode_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t vlanMode);

/* Module Name    : L2      */
/* Sub-module Name: Unicast */


/* Function Name:
 *      dal_esw_l2_learningEnable_get
 * Description:
 *      Get enable status of address learning on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of address learning
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
dal_esw_l2_learningEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_learningEnable_set
 * Description:
 *      Set enable status of address learning on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of address learning
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
dal_esw_l2_learningEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_newMacOp_get
 * Description:
 *      Get learning mode and forwarding action of new learned address on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pLrnMode   - pointer to learning mode
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 *
 *      Learning mode is as following
 *      - HARDWARE_LEARNING
 *      - SOFTWARE_LEARNING
 *      - NOT_LEARNING
 */
extern int32
dal_esw_l2_newMacOp_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_l2_newMacLrnMode_t  *pLrnMode,
    rtk_action_t            *pFwdAction);


/* Function Name:
 *      dal_esw_l2_newMacOp_set
 * Description:
 *      Set learning mode and forwarding action of new learned address on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      lrnMode   - learning mode
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 *
 *      Learning mode is as following
 *      - HARDWARE_LEARNING
 *      - SOFTWARE_LEARNING
 *      - NOT_LEARNING
 */
extern int32
dal_esw_l2_newMacOp_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_l2_newMacLrnMode_t  lrnMode,
    rtk_action_t            fwdAction);


/* Function Name:
 *      dal_esw_l2_LRUEnable_get
 * Description:
 *      Get enable status of least recent used address replace.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of least recent used address replace
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
dal_esw_l2_LRUEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_LRUEnable_set
 * Description:
 *      Set enable status of least recent used address replace.
 * Input:
 *      unit   - unit id
 *      enable - enable status of least recent used address replace
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
dal_esw_l2_LRUEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_ucastLookupMode_get
 * Description:
 *      Get lookup mode for unicast address.
 * Input:
 *      unit              - unit id
 * Output:
 *      pUcast_lookupMode - pointer to lookup mode for unicast address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode for multicast address is as following
 *      - UC_LOOKUP_ON_VID
 *      - UC_LOOKUP_ON_FID
 */
extern int32
dal_esw_l2_ucastLookupMode_get(uint32 unit, rtk_l2_ucastLookupMode_t *pUcast_lookupMode);


/* Function Name:
 *      dal_esw_l2_ucastLookupMode_set
 * Description:
 *      Set lookup mode for unicast address.
 * Input:
 *      unit             - unit id
 *      ucast_lookupMode - lookup mode for unicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Lookup mode for multicast address is as following
 *      - UC_LOOKUP_ON_VID
 *      - UC_LOOKUP_ON_FID
 */
extern int32
dal_esw_l2_ucastLookupMode_set(uint32 unit, rtk_l2_ucastLookupMode_t ucast_lookupMode);

/* Module Name    : L2              */
/* Sub-module Name: Unicast address */


/* Function Name:
 *      dal_esw_l2_addr_init
 * Description:
 *      Initialize content of buffer of L2 entry.
 *      Will fill vid ,MAC address and reset other field of L2 entry.
 * Input:
 *      unit     - unit id
 *      vid      - vlan id
 *      pMac     - MAC address
 *      pL2_addr - L2 entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 entry before add it.
 */
extern int32
dal_esw_l2_addr_init(
    uint32              unit,
    rtk_vlan_t          vid,
    rtk_mac_t           *pMac,
    rtk_l2_ucastAddr_t  *pL2_addr);


/* Function Name:
 *      dal_esw_l2_addr_add
 * Description:
 *      Add L2 entry to ASIC.
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
 *      RT_ERR_L2_NO_EMPTY_ENTRY - no empty entry in L2 table
 * Note:
 *      (1) Need to initialize L2 entry before add it.
 *      (2) The API can supported add by port or trunk-id view both.
 *          - If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->trk_gid is valid and pL2_addr->port is invalid.
 *          - If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
extern int32
dal_esw_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);


/* Function Name:
 *      dal_esw_l2_addr_del
 * Description:
 *      Delete a L2 unicast address entry from the specified device.
 * Input:
 *      unit  - unit id
 *      vid   - vlan id
 *      pMac  - mac address
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
dal_esw_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);


/* Function Name:
 *      dal_esw_l2_addr_get
 * Description:
 *      Get L2 entry based on specified vid and MAC address
 * Input:
 *      unit     - unit id
 * Output:
 *      pL2_addr - pointer to L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      (1) If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag,
 *          mean the pL2_addr->trk_gid is valid and pL2_addr->port is valid also.
 *          The pL2_addr->port value is the represent port of pL2_addr->trk_gid.
 *      (2) If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag,
 *          mean the pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
extern int32
dal_esw_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);


/* Function Name:
 *      dal_esw_l2_addr_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      (1) The API can supported add by port or trunk-id view both.
 *          - If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->trk_gid is valid and pL2_addr->port is invalid.
 *          - If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the 
 *            pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
extern int32
dal_esw_l2_addr_set(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);

/* Function Name:
 *      dal_esw_l2_addr_delAll
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
dal_esw_l2_addr_delAll(uint32 unit, uint32 include_static);


/* Function Name:
 *      dal_esw_l2_nextValidAddr_get
 * Description:
 *      Get next valid L2 unicast address entry from the specified device.
 * Input:
 *      unit           - unit id
 *      pScan_idx      - currently scan index of l2 table to get next.
 *      include_static - the get type, include static mac or not.
 * Output:
 *      pL2_data       - structure of l2 address data
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
 *      3. The pScan_idx is the input and also is the output argument.
 */
extern int32
dal_esw_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              include_static,
    rtk_l2_ucastAddr_t  *pL2_data);


/* Function Name:
 *      dal_esw_l2_nextValidMcastAddr_get
 * Description:
 *      Get next valid L2 multicast address entry from the specified device.
 * Input:
 *      unit      - unit id
 *      pScan_idx - currently scan index of l2 table to get next.
 * Output:
 *      pL2_data  - structure of l2 address data
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
 *      3. The pScan_idx is the input and also is the output argument.
 */
extern int32
dal_esw_l2_nextValidMcastAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_mcastAddr_t  *pL2_data);

/* Function Name:
 *      dal_esw_l2_nextValidIpMcastAddr_get
 * Description:
 *      Get next valid L2 ip multicast address entry from the specified device.
 * Input:
 *      unit      - unit id
 *      pScan_idx - currently scan index of l2 table to get next.
 * Output:
 *      pL2_data  - structure of l2 address data
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
 *      3. The pScan_idx is the input and also is the output argument.
 */
extern int32
dal_esw_l2_nextValidIpMcastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ipMcastAddr_t    *pL2_data);

/* Module Name    : L2           */
/* Sub-module Name: l2 multicast */



/* Function Name:
 *      dal_esw_l2_mcastLookupMode_get
 * Description:
 *      Get lookup mode for multicast address.
 * Input:
 *      unit              - unit id
 * Output:
 *      pMcast_lookupMode - pointer to lookup mode for multicast address
 *      pFixed_fid        - pointer to FID for MC_LOOKUP_ON_FIXED_FID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode for multicast address is as following
 *      - MC_LOOKUP_ON_VID
 *      - MC_LOOKUP_ON_FID
 *      - MC_LOOKUP_ON_FIXED_FID
 *
 *      When use MC_LOOKUP_ON_FIXED_FID, switch will use fixed_fid as lookup fid
 */
extern int32
dal_esw_l2_mcastLookupMode_get(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    *pMcast_lookupMode,
    rtk_fid_t                   *pFixed_fid);


/* Function Name:
 *      dal_esw_l2_mcastLookupMode_set
 * Description:
 *      Set lookup mode for multicast address.
 * Input:
 *      unit             - unit id
 *      mcast_lookupMode - lookup mode for multicast address
 *      fixed_fid        - FID for MC_LOOKUP_ON_FIXED_FID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_FID              - invalid fid
 * Note:
 *      Lookup mode for multicast address is as following
 *      - MC_LOOKUP_ON_VID
 *      - MC_LOOKUP_ON_FID
 *      - MC_LOOKUP_ON_FIXED_FID
 *
 *      When use MC_LOOKUP_ON_FIXED_FID, switch will use fixed_fid as lookup fid
 */
extern int32
dal_esw_l2_mcastLookupMode_set(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    mcast_lookupMode,
    rtk_fid_t                   fixed_fid);


/* Function Name:
 *      dal_esw_l2_mcastBlockPortmask_get
 * Description:
 *      Get portmask that block multicast forwarding.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPortmask - pointer to portmask
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
dal_esw_l2_mcastBlockPortmask_get(uint32 unit, rtk_portmask_t *pPortmask);


/* Function Name:
 *      dal_esw_l2_mcastBlockPortmask_set
 * Description:
 *      Set portmask that block multicast forwarding.
 * Input:
 *      unit      - unit id
 *      pPortmask - portmask
 * Output:
 *      None.
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
dal_esw_l2_mcastBlockPortmask_set(uint32 unit, rtk_portmask_t *pPortmask);


/* Function Name:
 *      dal_esw_l2_mcastAddr_init
 * Description:
 *      Initialize content of buffer of L2 multicast entry.
 *      Will fill vid ,MAC address and reset other field of L2 multicast entry.
 * Input:
 *      unit        - unit id
 *      vid         - vlan id
 *      pMac        - MAC address
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
extern int32
dal_esw_l2_mcastAddr_init(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac, rtk_l2_mcastAddr_t *pMcast_addr);


/* Function Name:
 *      dal_esw_l2_mcastAddr_add
 * Description:
 *      Add L2 multicast entry to ASIC.
 * Input:
 *      unit        - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
extern int32
dal_esw_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

/* Function Name:
 *      dal_esw_l2_mcastAddr_del
 * Description:
 *      Delete a L2 multicast address entry from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - multicast mac address
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
dal_esw_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);


/* Function Name:
 *      dal_esw_l2_mcastAddr_get
 * Description:
 *      Update content of L2 multicast entry.
 * Input:
 *      unit        - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);


/* Function Name:
 *      dal_esw_l2_mcastAddr_set
 * Description:
 *      Get L2 multicast entry based on specified vid and MAC address
 * Input:
 *      unit        - unit id
 * Output:
 *      pMcast_addr - pointer to L2 multicast entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_MAC              - invalid mac address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
extern int32
dal_esw_l2_mcastAddr_set(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

/* Function Name:
 *      dal_esw_l2_mcastAddr_add_with_index
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
 *      RT_ERR_L2_NO_EMPTY_ENTRY    - no empty entry in L2 table
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index.
 *      If fwdIndex is smaller than 0, will allocate a free index and return it.
 */
extern int32
dal_esw_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

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
dal_esw_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

/* Function Name:
 *      dal_esw_l2_mcastFwdIndex_alloc
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
dal_esw_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex); 

/* Function Name:
 *      dal_esw_l2_mcastFwdIndex_free
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
dal_esw_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index);

/* Function Name:
 *      dal_esw_l2_mcastFwdIndexFreeCount_get
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
dal_esw_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount);

/* Module Name    : L2           */
/* Sub-module Name: IP multicast */


/* Function Name:
 *      dal_esw_l2_ipmcEnable_get
 * Description:
 *      Get enable status of layer2 ip multicast switching.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of layer3 ip multicast switching
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
dal_esw_l2_ipmcEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_ipmcEnable_set
 * Description:
 *      Set enable status of layer2 ip multicast switching.
 * Input:
 *      unit   - unit id
 *      enable - enable status of layer2 ip multicast switching
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
dal_esw_l2_ipmcEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_ipmcMode_get
 * Description:
 *      Get lookup mode of layer2 ip multicast switching.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to lookup mode of layer2 ip multicast switching
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_ONLY
 */
extern int32
dal_esw_l2_ipmcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode);


/* Function Name:
 *      dal_esw_l2_ipmcMode_set
 * Description:
 *      Set lookup mode of layer2 ip multicast switching.
 * Input:
 *      unit - unit id
 *      mode - lookup mode of layer2 ip multicast switching
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_ONLY
 */
extern int32
dal_esw_l2_ipmcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode);


/* Function Name:
 *      dal_esw_l2_ipMcastAddr_init
 * Description:
 *      Initialize content of buffer of IP multicast entry.
 *      Will destination IP ,source IP and reset other field of IP multicast entry.
 * Input:
 *      unit          - unit id
 *      dip           - destination IP
 *      sip           - source IP
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
extern int32
dal_esw_l2_ipMcastAddr_init(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_l2_ipMcastAddr_t *pIpMcast_addr);


/* Function Name:
 *      dal_esw_l2_ipMcastAddr_add
 * Description:
 *      Add IP multicast entry to ASIC.
 * Input:
 *      unit          - unit id
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
extern int32
dal_esw_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_del
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
dal_esw_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid);


/* Function Name:
 *      dal_esw_l2_ipMcastAddr_get
 * Description:
 *      Get IP multicast entry on specified dip and sip.
 * Input:
 *      unit          - unit id
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
extern int32
dal_esw_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);


/* Function Name:
 *      dal_esw_l2_ipMcastAddr_set
 * Description:
 *      Update content of IP multicast entry.
 * Input:
 *      unit          - unit id
 *      pIpMcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV4_ADDRESS     - Invalid IPv4 address
 *      RT_ERR_VLAN_VID         - invalid vlan id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Note:
 *      None
 */
extern int32
dal_esw_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_add_with_index
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
 *      RT_ERR_L2_NO_EMPTY_ENTRY    - no empty entry in L2 table
 * Note:
 *      If fwdIndex is larger than or equal to 0, will use fwdIndex as multicast index.
 *      If fwdIndex is smaller than 0, will allocate a free index and return it.
 */
extern int32
dal_esw_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

/* Function Name:
 *      dal_esw_l2_ipMcastAddr_get_with_index
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
dal_esw_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);


/* Function Name:
 *      dal_esw_l2_ipmc_routerPorts_get
 * Description:
 *      Get router ports for ip multicast switching.
 *      All ip multicast packet will be forwarded to router ports.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPortmask - pointer to router portmask
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
dal_esw_l2_ipmc_routerPorts_get(uint32 unit, rtk_portmask_t *pPortmask);


/* Function Name:
 *      dal_esw_l2_ipmc_routerPorts_set
 * Description:
 *      Set router ports for ip multicast switching.
 *      All ip multicast packet will be forwarded to router ports.
 * Input:
 *      unit      - unit id
 *      pPortmask - router portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_MASK    - invalid portmask
 * Note:
 *      None
 */
extern int32
dal_esw_l2_ipmc_routerPorts_set(uint32 unit, rtk_portmask_t *pPortmask);


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAction_get
 * Description:
 *      Get forwarding action when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit             - unit id
 *      type             - mismatch type
 * Output:
 *      pMismatch_action - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 *
 *      Forwarding action is as following:
 *      - L2_IPMC_MIS_DROP
 *      - L2_IPMC_MIS_TRAP
 *      - L2_IPMC_MIS_FWD_AS_L2
 *      - L2_IPMC_MIS_FWD_AS_IPMC
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchAction_get(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    *pMismatch_action);


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAction_set
 * Description:
 *      Set forwarding action when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit            - unit id
 *      type            - mismatch type
 *      mismatch_action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 *
 *      Forwarding action is as following:
 *      - L2_IPMC_MIS_DROP
 *      - L2_IPMC_MIS_TRAP
 *      - L2_IPMC_MIS_FWD_AS_L2
 *      - L2_IPMC_MIS_FWD_AS_IPMC
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchAction_set(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    mismatch_action);


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPri_get
 * Description:
 *      Get priority of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit      - unit id
 *      type      - mismatch type
 * Output:
 *      pPriority - pointer to priority of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchPri_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t *pPriority);


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPri_set
 * Description:
 *      Set priority of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit     - unit id
 *      type     - mismatch type
 *      priority - priority of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchPri_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_ipmcDstAddrMismatchPriEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit     - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchPriEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDP_get
 * Description:
 *      Get drop precedence of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit - unit id
 *      type - mismatch type
 * Output:
 *      pDp  - pointer to drop precedence of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchDP_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 *pDp);

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDP_set
 * Description:
 *      Set drop precedence of trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit - unit id
 *      type - mismatch type
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid mismatch type
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchDP_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 dp);

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
dal_esw_l2_ipmcDstAddrMismatchDPEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit     - unit id
 *      enable - drop procedence assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_ipmcDstAddrMismatchDPEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      type    - mismatch type
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
dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      type   - mismatch type
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
dal_esw_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable);

/* Module Name    : L2                         */
/* Sub-module Name: Multicast forwarding table */
/* Function Name:
 *      dal_esw_l2_mcastFwdPortmask_set
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
 */
extern int32
dal_esw_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          crossVlan);

/* Function Name:
 *      dal_esw_l2_mcastFwdPortmask_get
 * Description:
 *      Get portmask of multicast forwarding entry
 * Input:
 *      unit        - unit id
 *      index       - index of multicast forwarding portmask
 * Output:
 *      *pPortmask  - pointer buffer of ip multicast ports
  *     *pCrossVlan - pointer of cross vlan flag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX   - invalid index of multicast forwarding portmask
 * Note:
 */
extern int32
dal_esw_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          *pCrossVlan);

/* Module Name    : L2              */
/* Sub-module Name: CPU MAC address */

/* Function Name:
 *      dal_esw_l2_cpuMacAddr_add
 * Description:
 *      Add a CPU mac adress of the vlan id to the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
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
dal_esw_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);

/* Function Name:
 *      dal_esw_l2_cpuMacAddr_del
 * Description:
 *      Delete a CPU mac adress of the vlan id from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
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
dal_esw_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);

/* Module Name    : L2        */
/* Sub-module Name: Port move */


/* Function Name:
 *      dal_esw_l2_legalMoveToPorts_get
 * Description:
 *      Get legal ports for moving to on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pLegalPorts - pointer to legal ports for moving to
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
dal_esw_l2_legalMoveToPorts_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts);

/* Function Name:
 *      dal_esw_l2_legalMoveToPorts_set
 * Description:
 *      Set legal ports for moving to on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      pLegalPorts - legal ports for moving to
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_MASK        - invalid portmask
 * Note:
 *      None
 */
extern int32
dal_esw_l2_legalMoveToPorts_set(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts);


/* Function Name:
 *      dal_esw_l2_illegalPortMoveAction_get
 * Description:
 *      Get forwarding action when illegal port moving happen on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_l2_illegalPortMoveAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        *pFwdAction);


/* Function Name:
 *      dal_esw_l2_illegalPortMoveAction_set
 * Description:
 *      Set forwarding action when illegal port moving happen on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_l2_illegalPortMoveAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        fwdAction);


/* Function Name:
 *      dal_esw_l2_legalPortMoveAction_get
 * Description:
 *      Get forwarding action when legal port moving happen on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_l2_legalPortMoveAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        *pFwdAction);


/* Function Name:
 *      dal_esw_l2_legalPortMoveAction_set
 * Description:
 *      Set forwarding action when legal port moving happen on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_l2_legalPortMoveAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        fwdAction);

/* Module Name    : L2                        */
/* Sub-module Name: Parameter for lookup miss */

/* Function Name:
 *      dal_esw_l2_lookupMissAction_get
 * Description:
 *      Get forwarding action when destination address lookup miss.
 * Input:
 *      unit    - unit id
 *      type    - type of lookup miss
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid type of lookup miss
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 *
 *      Forwarding action is as following:
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_FLOOD_IN_VLAN
 *      - ACTION_FLOOD_IN_ALL_PORT  (only for DLF_TYPE_IPMC)
 */
extern int32
dal_esw_l2_lookupMissAction_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t *pAction);


/* Function Name:
 *      dal_esw_l2_lookupMissAction_set
 * Description:
 *      Set forwarding action when destination address lookup miss.
 * Input:
 *      unit   - unit id
 *      type   - type of lookup miss
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid type of lookup miss
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 *
 *      Forwarding action is as following:
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_FLOOD_IN_VLAN
 *      - ACTION_FLOOD_IN_ALL_PORT  (only for DLF_TYPE_IPMC)
 */
extern int32
dal_esw_l2_lookupMissAction_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t action);


/* Function Name:
 *      dal_esw_l2_lookupMissPri_get
 * Description:
 *      Get priority of trapped packet when destination address lookup miss.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
extern int32
dal_esw_l2_lookupMissPri_get(uint32 unit, rtk_pri_t *pPriority);


/* Function Name:
 *      dal_esw_l2_lookupMissPri_set
 * Description:
 *      Set priority of trapped packet when destination address lookup miss.
 * Input:
 *      unit     - unit id
 *      priority - priority of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
extern int32
dal_esw_l2_lookupMissPri_set(uint32 unit, rtk_pri_t priority);


/* Function Name:
 *      dal_esw_l2_lookupMissPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_lookupMissPriEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l2_lookupMissPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_lookupMissPriEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_l2_lookupMissDP_get
 * Description:
 *      Get drop precedence of trapped packet when destination address lookup miss.
 * Input:
 *      unit - unit id
 * Output:
 *      pDp  - pointer to drop precedence of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
extern int32
dal_esw_l2_lookupMissDP_get(uint32 unit, uint32 *pDp);

/* Function Name:
 *      dal_esw_l2_lookupMissDP_set
 * Description:
 *      Set drop precedence of trapped packet when destination address lookup miss.
 * Input:
 *      unit - unit id
 *      dp   - drop precedence of trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_DROP_PRECEDENCE  - invalid drop precedence
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
extern int32
dal_esw_l2_lookupMissDP_set(uint32 unit, uint32 dp);

/* Function Name:
 *      dal_esw_l2_lookupMissDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
dal_esw_l2_lookupMissDPEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l2_lookupMissDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - drop procedence assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid drop procedence value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_lookupMissDPEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_lookupMissAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_l2_lookupMissAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_lookupMissAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
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
dal_esw_l2_lookupMissAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

/* Module Name    : L2                 */
/* Sub-module Name: Parameter for MISC */


/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_get
 * Description:
 *      Get source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id
 * Output:
 *      pFilter_portmask - source port egress filtering configuration when packet egress.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32
dal_esw_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFilter_portmask);


/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_set
 * Description:
 *      Set source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit             - unit id
 *      pFilter_portmask - source port egress filtering configuration when packet egress.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32
dal_esw_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFilter_portmask);

/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_add
 * Description:
 *      Add one port member to source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit        - unit id
 *      filter_port - port id that is going to be added in source port egress filtering mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32
dal_esw_l2_srcPortEgrFilterMask_add(uint32 unit, rtk_port_t filter_port);

/* Function Name:
 *      dal_esw_l2_srcPortEgrFilterMask_del
 * Description:
 *      Del one port member in source port egress filter mask to determine if mac need to do source filtering for an specific port
 *      when packet egress.
 * Input:
 *      unit        - unit id
 *      filter_port - port id that is going to be deleted in source port egress filtering mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      May be used when wirless device connected.
 */
extern int32
dal_esw_l2_srcPortEgrFilterMask_del(uint32 unit, rtk_port_t filter_port);


/*
 * MISC
 */



/* Function Name:
 *      dal_esw_l2_exceptionAddrAction_get
 * Description:
 *      Get forwarding action of packet with exception address.
 * Input:
 *      unit       - unit id
 *      exceptType - type of exception address
 * Output:
 *      pAction    - pointer to forward action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE  - invalid exception address type
 *      RT_ERR_NULL_POINTER         - input parameter may be null pointer
 * Note:
 *      Exception address type is as following
 *      - SA_IS_MCAST
 *      - SA_IS_BCAST
 *      - SA_IS_ZERO
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32
dal_esw_l2_exceptionAddrAction_get(
    uint32                          unit,
    rtk_l2_exceptionAddrType_t      exceptType,
    rtk_action_t                    *pAction);


/* Function Name:
 *      dal_esw_l2_exceptionAddrAction_set
 * Description:
 *      Set forwarding action of packet with exception address.
 * Input:
 *      unit       - unit id
 *      exceptType - type of exception address
 *      action     - forward action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_UNIT_ID              - invalid unit id
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE  - invalid exception address type
 *      RT_ERR_FWD_ACTION           - invalid forwarding action
 * Note:
 *      Exception address type is as following
 *      - SA_IS_MCAST
 *      - SA_IS_BCAST
 *      - SA_IS_ZERO
 *
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32
dal_esw_l2_exceptionAddrAction_set(
    uint32                          unit,
    rtk_l2_exceptionAddrType_t      exceptType,
    rtk_action_t                    action);


/* Function Name:
 *      dal_esw_l2_trapPri_get
 * Description:
 *      Get priority of trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority
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
dal_esw_l2_trapPri_get(uint32 unit, rtk_pri_t *pPriority);


/* Function Name:
 *      dal_esw_l2_trapPri_set
 * Description:
 *      Set priority of trapped packet.
 * Input:
 *      unit     - unit id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_trapPri_set(uint32 unit, rtk_pri_t priority);


/* Function Name:
 *      dal_esw_l2_trapPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
dal_esw_l2_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_l2_trapPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet.
 * Input:
 *      unit     - unit id
 *      enable - priority assignment status for trapped packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_l2_trapPriEnable_set(uint32 unit, rtk_enable_t enable);


/* Function Name:
 *      dal_esw_l2_trapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_l2_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_l2_trapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
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
dal_esw_l2_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_l2_addrEntry_get
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
 *      (1) The index valid range is from 0 to (L2 hash table size + L2 CAM table size - 1)
 *          - 0 ~ (L2 hash table size - 1) entry in L2 hash table
 *          - (L2 hash table size) ~ (L2 hash table size + L2 CAM table size - 1) entry in L2 CAM table
 *      (2) The output entry have 2 variables (valid and entry_type) and its detail data structure
 *          - valid: 1 mean the entry is valid; 0: invalid
 *          - entry_type: FLOW_TYPE_UNICAST, FLOW_TYPE_L2_MULTI and FLOW_TYPE_IP_MULTI
 *                        the field is ignored if valid field is 0.
 *          - detail data structure is ignored if valid is 0, and its filed meanings is depended
 *            on the entry_type value.
 *      (3) If pL2_entry->flags have enabled the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *          pL2_entry->unicast.trk_gid is valid trunk id value.
 */
extern int32
dal_esw_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry);

/* Function Name:
 *      dal_esw_l2_conflictAddr_get
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
dal_esw_l2_conflictAddr_get(
    uint32          unit,
    rtk_l2_entry_t  *pL2Addr,
    rtk_l2_entry_t  *pCfAddrList,
    uint32          cfAddrList_size,
    uint32          *pCf_retCnt);

/* Function Name:
 *      dal_esw_l2_limitLearningCnt_workaround
 * Description:
 *      Delete all L2 unicast address entry which aging bit is 2 from the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the mac limiting count wrong problem
 */
extern int32
dal_esw_l2_limitLearningCnt_workaround(uint32 unit);

/* Function Name:
 *      dal_esw_l2_limitLearningCntByRange_workaround
 * Description:
 *      Delete all L2 unicast address entry which aging bit is less or equal 3 from the specified device.
 * Input:
 *      unit      - unit id
 *      start_idx - start index
 *      end_idx   - end index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the mac limiting count wrong problem
 */
extern int32
dal_esw_l2_limitLearningCntByRange_workaround(uint32 unit, uint32 start_idx, uint32 end_idx);

/* Function Name:
 *      dal_esw_l2_forwardTablePortmask_workaround
 * Description:
 *      Rewrite the all forward table from shadow to chip in the specified device.
 * Input:
 *      unit      - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      The API is workaround for patch the forward table portmask problem
 */
extern int32
dal_esw_l2_forwardTablePortmask_workaround(uint32 unit);

#endif /* __DAL_ESW_L2_H__ */
