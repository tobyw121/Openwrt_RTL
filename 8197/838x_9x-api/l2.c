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
 * $Revision: 41931 $
 * $Date: 2013-08-09 11:10:22 +0800 (Fri, 09 Aug 2013) $
 *
 * Purpose : Definition of L2 API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Mac address flush
 *           (2) Address learning limit
 *           (3) Parameter for L2 lookup and learning engine
 *           (4) Unicast address
 *           (5) L2 multicast
 *           (6) IP multicast
 *           (7) Multicast forwarding table
 *           (8) CPU mac
 *           (9) Port move
 *           (10) Parameter for lookup miss
 *           (11) Parameter for MISC
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/l2.h>

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Module Name    : L2     */
/* Sub-module Name: Global */

/* Function Name:
 *      rtk_l2_init
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
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize l2 module before calling any l2 APIs.
 */
int32
rtk_l2_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_init(unit);
} /* end of rtk_l2_init */

/* Module Name    : L2                */
/* Sub-module Name: Mac address flush */

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush dynamic mac entries corresponding to linkdown port configuration.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer buffer of state of HW clear linkdown port mac
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The status of flush linkdown port address is as following:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_flushLinkDownPortAddrEnable_get(unit, pEnable);
} /* end of rtk_l2_flushLinkDownPortAddrEnable_get */

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush dynamic mac entries corresponding to linkdown port configuration.
 * Input:
 *      unit   - unit id
 *      enable - configure value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The status of flush linkdown port address is as following:
 *          - DISABLED
 *          - ENABLED
 */
int32
rtk_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_flushLinkDownPortAddrEnable_set(unit, enable);
} /* end of rtk_l2_flushLinkDownPortAddrEnable_set */

/* Function Name:
 *      rtk_l2_ucastAddr_flush
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_VLAN_VID     - invalid vlan id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The L2 unicast entries in L2 table which meet the specified criteria are flushed by HW.
 */
int32
rtk_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_flushCfg_t   temp_config;

    temp_config = *pConfig;
    if ((pConfig->flushByPort == 1) && (pConfig->portOrTrunk == ENABLED))
    {
        temp_config.port = RTK_PORT_TO_PHYSICAL_PORT(unit, (*pConfig).port);
        temp_config.replacingPort = RTK_PORT_TO_PHYSICAL_PORT(unit, (*pConfig).replacingPort);
    }
    return RT_MAPPER(unit)->l2_ucastAddr_flush(unit, &temp_config);
    }
#else
    return RT_MAPPER(unit)->l2_ucastAddr_flush(unit, pConfig);
#endif
} /* end of rtk_l2_ucastAddr_flush */

/* Module Name    : L2                     */
/* Sub-module Name: Address learning limit */

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get the total mac learning counts of specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pMac_cnt - pointer buffer of system mac learning counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The mac learning counts only calculate dynamic mac numbers.
 */
int32
rtk_l2_learningCnt_get(uint32 unit, uint32 *pMac_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_learningCnt_get(unit, pMac_cnt);
} /* end of rtk_l2_learningCnt_get */

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *      Get the maximum mac learning counts of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pMac_cnt - pointer buffer of maximum mac learning counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The maximum mac learning counts only limit for dynamic learning mac
 *          address, not apply to static mac address.
 *      (2) Set the mac_cnt to 0 mean disable learning in the system.
 */
int32
rtk_l2_limitLearningCnt_get(uint32 unit, uint32 *pMac_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningCnt_get(unit, pMac_cnt);
} /* end of rtk_l2_limitLearningCnt_get */

/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *      Set the maximum mac learning counts of the specified device.
 * Input:
 *      unit    - unit id
 *      mac_cnt - maximum mac learning counts
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_LIMITED_L2ENTRY_NUM - invalid limited L2 entry number
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The maximum mac learning counts only limit for dynamic learning mac
 *          address, not apply to static mac address.
 *      (2) Set the mac_cnt to 0 mean disable learning in the system.
 */
int32
rtk_l2_limitLearningCnt_set(uint32 unit, uint32 mac_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningCnt_set(unit, mac_cnt);
} /* end of rtk_l2_limitLearningCnt_set */

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Get the action when over system learning maximum mac counts.
 * Input:
 *      unit    - unit id
 * Output:
 *      pAction - pointer buffer of action when over learning maximum mac counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The action symbol as following
 *          - LIMIT_LEARN_CNT_ACTION_DROP
 *          - LIMIT_LEARN_CNT_ACTION_FORWARD
 *          - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *          - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
rtk_l2_limitLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningCntAction_get(unit, pAction);
} /* end of rtk_l2_limitLearningCntAction_get */

/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
 * Description:
 *      Set the action when over system learning maximum mac counts.
 * Input:
 *      unit   - unit id
 *      action - action when over learning maximum mac counts
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
 *      (1) The action symbol as following
 *          - LIMIT_LEARN_CNT_ACTION_DROP
 *          - LIMIT_LEARN_CNT_ACTION_FORWARD
 *          - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *          - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
rtk_l2_limitLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningCntAction_set(unit, action);
} /* end of rtk_l2_limitLearningCntAction_set */

/* Function Name:
 *      rtk_l2_portLearningCnt_get
 * Description:
 *      Get current mac learning counts of the port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pMac_cnt - pointer buffer of mac learning counts of the port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The mac learning counts returned only calculate dynamic mac numbers.
 */
int32
rtk_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portLearningCnt_get(unit, phy_port, pMac_cnt);
    }
#else
    return RT_MAPPER(unit)->l2_portLearningCnt_get(unit, port, pMac_cnt);
#endif
} /* end of rtk_l2_portLearningCnt_get */

/* Function Name:
 *      rtk_l2_portLimitLearningCntEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      For 8390, 8380, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_l2_portLimitLearningCntEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_portLimitLearningCntEnable_get(unit, port, pEnable);
} /* end of rtk_l2_portLimitLearningCntEnable_get */

/* Function Name:
 *      rtk_l2_portLimitLearningCntEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      For 8390, 8380, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_l2_portLimitLearningCntEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_portLimitLearningCntEnable_set(unit, port, enable);
} /* end of rtk_l2_portLimitLearningCntEnable_set */

/* Function Name:
 *      rtk_l2_portLimitLearningCnt_get
 * Description:
 *      Get the maximum mac learning counts of the port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pMac_cnt - pointer buffer of maximum mac learning counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The maximum mac learning counts only limit for dynamic learning mac
 *          address, not apply to static mac address.
 *      (2) Set the mac_cnt to 0 mean disable learning in the port.
 */
int32
rtk_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portLimitLearningCnt_get(unit, phy_port, pMac_cnt);
    }
#else
    return RT_MAPPER(unit)->l2_portLimitLearningCnt_get(unit, port, pMac_cnt);
#endif
} /* end of rtk_l2_portLimitLearningCnt_get */

/* Function Name:
 *      rtk_l2_portLimitLearningCnt_set
 * Description:
 *      Set the maximum mac learning counts of the port.
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
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The maximum mac learning counts only limit for dynamic learning mac
 *          address, not apply to static mac address.
 *      (2) Set the mac_cnt to 0 mean disable learning in the port.
 */
int32
rtk_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portLimitLearningCnt_set(unit, phy_port, mac_cnt);
    }
#else
    return RT_MAPPER(unit)->l2_portLimitLearningCnt_set(unit, port, mac_cnt);
#endif
} /* end of rtk_l2_portLimitLearningCnt_set */

/* Function Name:
 *      rtk_l2_portLimitLearningCntAction_get
 * Description:
 *      Get the action when over learning maximum mac counts of the port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer buffer of action when over learning maximum mac counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The action symbol as following
 *          - LIMIT_LEARN_CNT_ACTION_DROP
 *          - LIMIT_LEARN_CNT_ACTION_FORWARD
 *          - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *          - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
rtk_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portLimitLearningCntAction_get(unit, phy_port, pAction);
    }
#else
    return RT_MAPPER(unit)->l2_portLimitLearningCntAction_get(unit, port, pAction);
#endif
} /* end of rtk_l2_portLimitLearningCntAction_get */

/* Function Name:
 *      rtk_l2_portLimitLearningCntAction_set
 * Description:
 *      Set the action when over learning maximum mac counts of the port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - action when over learning maximum mac counts
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
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The action symbol as following
 *          - LIMIT_LEARN_CNT_ACTION_DROP
 *          - LIMIT_LEARN_CNT_ACTION_FORWARD
 *          - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *          - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
rtk_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portLimitLearningCntAction_set(unit, phy_port, action);
    }
#else
    return RT_MAPPER(unit)->l2_portLimitLearningCntAction_set(unit, port, action);
#endif
} /* end of rtk_l2_portLimitLearningCntAction_set */

/* Function Name:
 *      rtk_l2_portLastLearnedMac_get
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
rtk_l2_portLastLearnedMac_get(uint32 unit, rtk_port_t port, rtk_fid_t *pFid, rtk_mac_t *pMac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_portLastLearnedMac_get(unit, port, pFid, pMac);
} /* end of rtk_l2_portLastLearnedMac_get */

/* Function Name:
 *      rtk_l2_fidLimitLearningEntry_get
 * Description:
 *      Get the specified VLAN MAC limit configuration.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of VLAN MAC limit entry
 * Output:
 *      pFidMacLimitEntry - pointer to MAC limit configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of fid_macLimit_idx is 0~31 in 8328 and 8390, 0~7 in 8380.
 *      (2) Forwarding action is as following
 *          - LIMIT_LEARN_CNT_ACTION_DROP
 *          - LIMIT_LEARN_CNT_ACTION_FORWARD
 *          - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *          - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
rtk_l2_fidLimitLearningEntry_get(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_fidMacLimitEntry_t  temp_fidMacLimitEntry;

    if ((ret = RT_MAPPER(unit)->l2_fidLimitLearningEntry_get(unit, fid_macLimit_idx, &temp_fidMacLimitEntry)) != RT_ERR_OK)
        return ret;
    (*pFidMacLimitEntry).port = PHYSICAL_PORT_TO_RTK_PORT(unit, temp_fidMacLimitEntry.port);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_fidLimitLearningEntry_get(unit, fid_macLimit_idx, pFidMacLimitEntry);
#endif
} /* end of rtk_l2_fidLimitLearningEntry_get */

/* Function Name:
 *      rtk_l2_fidLimitLearningEntry_set
 * Description:
 *      Set the specified VLAN MAC limit configuration.
 * Input:
 *      unit              - unit id
 *      fid_macLimit_idx  - index of VLAN MAC limit entry
 *      pFidMacLimitEntry - MAC limit configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_VLAN_VID     - invalid vlan id
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of fid_macLimit_idx is 0~31 in 8328 and 8390, 0~7 in 8380.
 *      (2) Forwarding action is as following
 *          - LIMIT_LEARN_CNT_ACTION_DROP
 *          - LIMIT_LEARN_CNT_ACTION_FORWARD
 *          - LIMIT_LEARN_CNT_ACTION_TO_CPU
 *          - LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU
 */
int32
rtk_l2_fidLimitLearningEntry_set(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_fidMacLimitEntry_t  temp_fidMacLimitEntry;

    temp_fidMacLimitEntry = *pFidMacLimitEntry;
    temp_fidMacLimitEntry.port = RTK_PORT_TO_PHYSICAL_PORT(unit, (*pFidMacLimitEntry).port);
    return RT_MAPPER(unit)->l2_fidLimitLearningEntry_set(unit, fid_macLimit_idx, &temp_fidMacLimitEntry);
    }
#else
    return RT_MAPPER(unit)->l2_fidLimitLearningEntry_set(unit, fid_macLimit_idx, pFidMacLimitEntry);
#endif
} /* end of rtk_l2_fidLimitLearningEntry_set */

/* Function Name:
 *      rtk_l2_fidLearningCnt_get
 * Description:
 *      Get number of learned MAC addresses on specified VLAN MAC limit entry.
 * Input:
 *      unit             - unit id
 *      fid_macLimit_idx - index of VLAN MAC limit entry
 * Output:
 *      pNum             - number of learned MAC addresses
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of fid_macLimit_idx is 0~31 in 8328 and 8390, 0~7 in 8380.
 */
int32
rtk_l2_fidLearningCnt_get(uint32 unit, uint32 fid_macLimit_idx, uint32 *pNum)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_fidLearningCnt_get(unit, fid_macLimit_idx, pNum);
} /* end of rtk_l2_fidLearningCnt_get */

/* Function Name:
 *      rtk_l2_fidLearningCnt_reset
 * Description:
 *      Reset number of learned MAC addresses on specified VLAN MAC limit entry.
 * Input:
 *      unit             - unit id
 *      fid_macLimit_idx - index of VLAN MAC limit entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of fid_macLimit_idx is 0~31 in 8328 and 8390, 0~7 in 8380.
 */
int32
rtk_l2_fidLearningCnt_reset(uint32 unit, uint32 fid_macLimit_idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_fidLearningCnt_reset(unit, fid_macLimit_idx);
} /* end of rtk_l2_fidLearningCnt_reset */

/* Function Name:
 *      rtk_l2_fidLastLearnedMac_get
 * Description:
 *      Get lastest learned MAC address on specifid fid.
 * Input:
 *      unit             - unit id
 *      fid_macLimit_idx - index of VLAN MAC limit entry
 * Output:
 *      pFid             - pointer to filter id
 *      pMac             - lastest learned mac address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fid_macLimit_idx is 0~15 in 8328.
 */
int32
rtk_l2_fidLastLearnedMac_get(
    uint32      unit,
    uint32      fid_macLimit_idx,
    rtk_fid_t   *pFid,
    rtk_mac_t   *pMac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_fidLastLearnedMac_get(unit, fid_macLimit_idx, pFid, pMac);
} /* end of rtk_l2_fidLastLearnedMac_get */

/* Function Name:
 *      rtk_l2_fidLearningCntAction_get
 * Description:
 *      Get the action when over learning maximum mac counts of VLAN MAC limit.
 * Input:
 *      unit    - unit id
 * Output:
 *      pAction - pointer buffer of action when over learning maximum mac counts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The action applied to all the VLAN MAC limit configuration.
 */
int32
rtk_l2_fidLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_fidLearningCntAction_get(unit, pAction);
} /* end of rtk_l2_fidLearningCntAction_get */

/* Function Name:
 *      rtk_l2_fidLearningCntAction_set
 * Description:
 *      Set the action when over learning maximum mac counts of VLAN MAC limit.
 * Input:
 *      unit   - unit id
 *      action - action when over learning maximum mac counts
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
 *      The action applied to all the VLAN MAC limit configuration.
 */
int32
rtk_l2_fidLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_fidLearningCntAction_set(unit, action);
} /* end of rtk_l2_fidLearningCntAction_set */

/* Function Name:
 *      rtk_l2_limitLearningTrapPri_get
 * Description:
 *      Get priority of trapped packet when number mac address over limitation.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority of trapped packet
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
rtk_l2_limitLearningTrapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapPri_get(unit, pPriority);
} /* end of rtk_l2_limitLearningTrapPri_get */

/* Function Name:
 *      rtk_l2_limitLearningTrapPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_limitLearningTrapPri_set(uint32 unit, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapPri_set(unit, priority);
} /* end of rtk_l2_limitLearningTrapPri_set */

/* Function Name:
 *      rtk_l2_limitLearningTrapPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
rtk_l2_limitLearningTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapPriEnable_get(unit, pEnable);
} /* end of rtk_l2_limitLearningTrapPriEnable_get */

/* Function Name:
 *      rtk_l2_limitLearningTrapPriEnable_set
 * Description:
 *      Set priority assignment status for trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - priority assignment status for trapped packet
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
rtk_l2_limitLearningTrapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapPriEnable_set(unit, enable);
} /* end of rtk_l2_limitLearningTrapPriEnable_set */

/* Function Name:
 *      rtk_l2_limitLearningTrapDP_get
 * Description:
 *      Get drop precedence of trapped packet when number mac address over limitation.
 * Input:
 *      unit - unit id
 * Output:
 *      pDp  - drop precedence of trapped packet
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
rtk_l2_limitLearningTrapDP_get(uint32 unit, uint32 *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapDP_get(unit, pDp);
} /* end of rtk_l2_limitLearningTrapDP_get */

/* Function Name:
 *      rtk_l2_limitLearningTrapDP_set
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_limitLearningTrapDP_set(uint32 unit, uint32 dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapDP_set(unit, dp);
} /* end of rtk_l2_limitLearningTrapDP_set */

/* Function Name:
 *      rtk_l2_limitLearningTrapDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
rtk_l2_limitLearningTrapDPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapDPEnable_get(unit, pEnable);
} /* end of rtk_l2_limitLearningTrapDPEnable_get */

/* Function Name:
 *      rtk_l2_limitLearningTrapDPEnable_set
 * Description:
 *      Set drop procedence assignment status for trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - drop procedence assignment status for trapped packet
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
rtk_l2_limitLearningTrapDPEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapDPEnable_set(unit, enable);
} /* end of rtk_l2_limitLearningTrapDPEnable_set */

/* Function Name:
 *      rtk_l2_limitLearningTrapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
rtk_l2_limitLearningTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapAddCPUTagEnable_get(unit, pEnable);
} /* end of rtk_l2_limitLearningTrapAddCPUTagEnable_get */

/* Function Name:
 *      rtk_l2_limitLearningTrapAddCPUTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_limitLearningTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_limitLearningTrapAddCPUTagEnable_set(unit, enable);
} /* end of rtk_l2_limitLearningTrapAddCPUTagEnable_set */

/* Module Name    : L2                                          */
/* Sub-module Name: Parameter for L2 lookup and learning engine */

/* Function Name:
 *      rtk_l2_aging_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The unit is second.
 *      (2) Get aging_time as 0 mean disable aging mechanism.
 */
int32
rtk_l2_aging_get(uint32 unit, uint32 *pAging_time)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_aging_get(unit, pAging_time);
} /* end of rtk_l2_aging_get */

/* Function Name:
 *      rtk_l2_aging_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The unit is second.
 *      (2) apply aging_time as 0 mean disable aging mechanism.
 */
int32
rtk_l2_aging_set(uint32 unit, uint32 aging_time)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_aging_set(unit, aging_time);
} /* end of rtk_l2_aging_set */

/* Function Name:
 *      rtk_l2_portAgingEnable_get
 * Description:
 *      Get the dynamic address aging out state configuration of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status of aging out
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
int32
rtk_l2_portAgingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portAgingEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->l2_portAgingEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_l2_portAgingEnable_get */

/* Function Name:
 *      rtk_l2_portAgingEnable_set
 * Description:
 *      Set the dynamic address aging out state configuration of the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of aging out
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
int32
rtk_l2_portAgingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portAgingEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->l2_portAgingEnable_set(unit, port, enable);
#endif
} /* end of rtk_l2_portAgingEnable_set */

/* Function Name:
 *      rtk_l2_camEnable_get
 * Description:
 *      Get enable status of CAM entry.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CAM entry
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
rtk_l2_camEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_camEnable_get(unit, pEnable);
} /* end of rtk_l2_camEnable_get */

/* Function Name:
 *      rtk_l2_camEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) Enable the CAM to have more 64 entries for store collision address.
 *      (2) 8328 is configure, but 8389 is always enable in chip.
 */
int32
rtk_l2_camEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_camEnable_set(unit, enable);
} /* end of rtk_l2_camEnable_set */

/* Function Name:
 *      rtk_l2_hashAlgo_get
 * Description:
 *      Get hash algorithm of layer2 switching.
 * Input:
 *      unit       - unit id
 * Output:
 *      pHash_algo - pointer to hash algorithm of layer2 switching
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_l2_hashAlgo_get(uint32 unit, uint32 *pHash_algo)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_hashAlgo_get(unit, pHash_algo);
} /* end of rtk_l2_hashAlgo_get */

/* Function Name:
 *      rtk_l2_hashAlgo_set
 * Description:
 *      Set hash algorithm of layer2 switching.
 * Input:
 *      unit      - unit id
 *      hash_algo - hash algorithm of layer2 switching
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
 *      The valid input is 0 and 1. 0 mean hash_algo_0; 1 mean hash_algo_1.
 */
int32
rtk_l2_hashAlgo_set(uint32 unit, uint32 hash_algo)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_hashAlgo_set(unit, hash_algo);
} /* end of rtk_l2_hashAlgo_set */

/* Function Name:
 *      rtk_l2_vlanMode_get
 * Description:
 *      Get vlan(inner/outer vlan) for L2 lookup on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pVlanMode - pointer to inner/outer vlan
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
 *      Vlan mode is as following
 *      - BASED_ON_INNER_VLAN
 *      - BASED_ON_OUTER_VLAN
 */
int32
rtk_l2_vlanMode_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pVlanMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_vlanMode_get(unit, phy_port, pVlanMode);
    }
#else
    return RT_MAPPER(unit)->l2_vlanMode_get(unit, port, pVlanMode);
#endif
} /* end of rtk_l2_vlanMode_get */

/* Function Name:
 *      rtk_l2_vlanMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Vlan mode is as following
 *      - BASED_ON_INNER_VLAN
 *      - BASED_ON_OUTER_VLAN
 */
int32
rtk_l2_vlanMode_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t vlanMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_vlanMode_set(unit, phy_port, vlanMode);
    }
#else
    return RT_MAPPER(unit)->l2_vlanMode_set(unit, port, vlanMode);
#endif
} /* end of rtk_l2_vlanMode_set */

/* Module Name    : L2      */
/* Sub-module Name: Unicast */

/* Function Name:
 *      rtk_l2_learningEnable_get
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
rtk_l2_learningEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_learningEnable_get(unit, port, pEnable);
} /* end of rtk_l2_learningEnable_get */

/* Function Name:
 *      rtk_l2_learningEnable_set
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
rtk_l2_learningEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_learningEnable_set(unit, port, enable);
} /* end of rtk_l2_learningEnable_set */

/* Function Name:
 *      rtk_l2_newMacOp_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Forwarding action is as following
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_COPY2CPU
 *
 *      (2) Learning mode is as following
 *          - HARDWARE_LEARNING
 *          - SOFTWARE_LEARNING
 *          - NOT_LEARNING
 */
int32
rtk_l2_newMacOp_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_l2_newMacLrnMode_t  *pLrnMode,
    rtk_action_t            *pFwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_newMacOp_get(unit, phy_port, pLrnMode, pFwdAction);
    }
#else
    return RT_MAPPER(unit)->l2_newMacOp_get(unit, port, pLrnMode, pFwdAction);
#endif
} /* end of rtk_l2_newMacOp_get */

/* Function Name:
 *      rtk_l2_newMacOp_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_INPUT      - invalid input parameter
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Forwarding action is as following
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_COPY2CPU
 *
 *      (2) Learning mode is as following
 *          - HARDWARE_LEARNING
 *          - SOFTWARE_LEARNING
 *          - NOT_LEARNING
 */
int32
rtk_l2_newMacOp_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_l2_newMacLrnMode_t  lrnMode,
    rtk_action_t            fwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_newMacOp_set(unit, phy_port, lrnMode, fwdAction);
    }
#else
    return RT_MAPPER(unit)->l2_newMacOp_set(unit, port, lrnMode, fwdAction);
#endif
} /* end of rtk_l2_newMacOp_set */

/* Function Name:
 *      rtk_l2_LRUEnable_get
 * Description:
 *      Get enable status of least recent used address replace.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of least recent used address replace
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
rtk_l2_LRUEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_LRUEnable_get(unit, pEnable);
} /* end of rtk_l2_LRUEnable_get */

/* Function Name:
 *      rtk_l2_LRUEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_LRUEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_LRUEnable_set(unit, enable);
} /* end of rtk_l2_LRUEnable_set */

/* Function Name:
 *      rtk_l2_ucastLookupMode_get
 * Description:
 *      Get lookup mode for unicast address.
 * Input:
 *      unit              - unit id
 * Output:
 *      pUcast_lookupMode - pointer to lookup mode for unicast address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - UC_LOOKUP_ON_VID
 *          - UC_LOOKUP_ON_FID
 *      (2) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 *          The mode is return from first exist vlan id.
 */
int32
rtk_l2_ucastLookupMode_get(uint32 unit, rtk_l2_ucastLookupMode_t *pUcast_lookupMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ucastLookupMode_get(unit, pUcast_lookupMode);
} /* end of rtk_l2_ucastLookupMode_get */

/* Function Name:
 *      rtk_l2_ucastLookupMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - UC_LOOKUP_ON_VID
 *          - UC_LOOKUP_ON_FID
 *      (2) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 *          The mode is applied to all exist vlan.
 */
int32
rtk_l2_ucastLookupMode_set(uint32 unit, rtk_l2_ucastLookupMode_t ucast_lookupMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ucastLookupMode_set(unit, ucast_lookupMode);
} /* end of rtk_l2_ucastLookupMode_set */

/* Module Name    : L2              */
/* Sub-module Name: Unicast address */

/* Function Name:
 *      rtk_l2_addr_init
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Need to initialize L2 entry before add it.
 */
int32
rtk_l2_addr_init(
    uint32              unit,
    rtk_vlan_t          vid,
    rtk_mac_t           *pMac,
    rtk_l2_ucastAddr_t  *pL2_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_addr_init(unit, vid, pMac, pL2_addr);
} /* end of rtk_l2_addr_init */

/* Function Name:
 *      rtk_l2_addr_add
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) For 8328, need to initialize L2 entry before adding entries.
 *      (2) The API can supported add by port or trunk-id view both.
 *          - If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *            pL2_addr->trk_gid is valid and pL2_addr->port is invalid.
 *          - If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *            pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 *      (3) When adding a entry with RTK_L2_UCAST_FLAG_SA_BLOCK or RTK_L2_UCAST_FLAG_SA_SECURE attribute,
 *          pL2_addr->port should be set to 0x3f in 8390 and 0x1f in 8380 if the specified MAC address
 *          would come from nowhere.
 */
int32
rtk_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ucastAddr_t  temp_l2Addr;

    temp_l2Addr = *pL2_addr;
    temp_l2Addr.port = RTK_PORT_TO_PHYSICAL_PORT(unit, (*pL2_addr).port);
    return RT_MAPPER(unit)->l2_addr_add(unit, &temp_l2Addr);
    }
#else
    return RT_MAPPER(unit)->l2_addr_add(unit, pL2_addr);
#endif
} /* end of rtk_l2_addr_add */

/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete a L2 unicast address entry from the specified device.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_VLAN_VID          - invalid vid
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      VID is same as FID in IVL mode.
 */
int32
rtk_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_addr_del(unit, vid, pMac);
} /* end of rtk_l2_addr_del */

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get L2 entry based on specified vid and MAC address
 * Input:
 *      unit     - unit id
 * Output:
 *      pL2_addr - pointer to L2 entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_VLAN_VID          - invalid vlan id
 *      RT_ERR_MAC               - invalid mac address
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) VID is same as FID in IVL mode.
 *      (2) The pL2_data.vid and pL2_data.mac is input key
 *      (3) The pL2_data.port, pL2_data.auth, pL2_data.sa_block,
 *          pL2_data.da_block and pL2_data.is_static is output.
 *      (4) If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag,
 *          mean the pL2_addr->trk_gid is valid and pL2_addr->port is valid also.
 *          The pL2_addr->port value is the represent port of pL2_addr->trk_gid.
 *      (5) If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag,
 *          mean the pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
int32
rtk_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ucastAddr_t  temp_l2Addr;

    temp_l2Addr = *pL2_addr;
    if ((ret = RT_MAPPER(unit)->l2_addr_get(unit, &temp_l2Addr)) != RT_ERR_OK)
        return ret;
    *pL2_addr = temp_l2Addr;
    (*pL2_addr).port = PHYSICAL_PORT_TO_RTK_PORT(unit, temp_l2Addr.port);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_addr_get(unit, pL2_addr);
#endif
} /* end of rtk_l2_addr_get */

/* Function Name:
 *      rtk_l2_addr_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The API can support adding entry by port or trunk-id view both.
 *          - If pL2_addr->flags have turn on the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *            pL2_addr->trk_gid is valid and pL2_addr->port is invalid.
 *          - If pL2_addr->flags have turn off the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *            pL2_addr->port is valid and pL2_addr->trk_gid is invalid.
 */
int32
rtk_l2_addr_set(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ucastAddr_t  temp_l2Addr;

    temp_l2Addr = *pL2_addr;
    temp_l2Addr.port = RTK_PORT_TO_PHYSICAL_PORT(unit, (*pL2_addr).port);
    return RT_MAPPER(unit)->l2_addr_set(unit, &temp_l2Addr);
    }
#else
    return RT_MAPPER(unit)->l2_addr_set(unit, pL2_addr);
#endif
} /* end of rtk_l2_addr_set */

/* Function Name:
 *      rtk_l2_addr_delAll
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_l2_addr_delAll(uint32 unit, uint32 include_static)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_addr_delAll(unit, include_static);
} /* end of rtk_l2_addr_delAll */

/* Function Name:
 *      rtk_l2_nextValidAddr_get
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_INPUT             - invalid input parameter
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The function will skip valid l2 multicast and ip multicast entry and
 *          returned next valid L2 unicast address which based on index order of l2 table.
 *      (2) Input -1 for getting the first entry of l2 table.
 *      (3) The pScan_idx is both the input and output argument.
 */
int32
rtk_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              include_static,
    rtk_l2_ucastAddr_t  *pL2_data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ucastAddr_t  temp_l2Addr;

    if ((ret = RT_MAPPER(unit)->l2_nextValidAddr_get(unit, pScan_idx, include_static, &temp_l2Addr)) != RT_ERR_OK)
        return ret;
    *pL2_data = temp_l2Addr;
    (*pL2_data).port = PHYSICAL_PORT_TO_RTK_PORT(unit, temp_l2Addr.port);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_nextValidAddr_get(unit, pScan_idx, include_static, pL2_data);
#endif
} /* end of rtk_l2_nextValidAddr_get */

/* Function Name:
 *      rtk_l2_nextValidMcastAddr_get
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The function will skip valid l2 unicast and ip multicast entry and
 *          returned next valid L2 multicast address which based on index order of l2 table.
 *      (2) Input -1 for getting the first entry of l2 table.
 *      (3) The pScan_idx is both the input and output argument.
 */
int32
rtk_l2_nextValidMcastAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_mcastAddr_t  *pL2_data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_mcastAddr_t  temp_l2Addr;

    if ((ret = RT_MAPPER(unit)->l2_nextValidMcastAddr_get(unit, pScan_idx, &temp_l2Addr)) != RT_ERR_OK)
        return ret;
    *pL2_data = temp_l2Addr;
    (*pL2_data).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_l2Addr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_nextValidMcastAddr_get(unit, pScan_idx, pL2_data);
#endif
} /* end of rtk_l2_nextValidMcastAddr_get */

/* Function Name:
 *      rtk_l2_nextValidIpMcastAddr_get
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
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_NULL_POINTER      - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND - specified entry not found
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The function will skip valid l2 unicast and multicast entry and
 *          returned next valid L2 ip multicast address which based on index order of l2 table.
 *      (2) Input -1 for getting the first entry of l2 table.
 *      (3) The pScan_idx is both the input and output argument.
 */
int32
rtk_l2_nextValidIpMcastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ipMcastAddr_t    *pL2_data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ipMcastAddr_t  temp_l2Addr;

    if ((ret = RT_MAPPER(unit)->l2_nextValidIpMcastAddr_get(unit, pScan_idx, &temp_l2Addr)) != RT_ERR_OK)
        return ret;
    *pL2_data = temp_l2Addr;
    (*pL2_data).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_l2Addr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_nextValidIpMcastAddr_get(unit, pScan_idx, pL2_data);
#endif
} /* end of rtk_l2_nextValidIpMcastAddr_get */

/* Module Name    : L2           */
/* Sub-module Name: l2 multicast */

/* Function Name:
 *      rtk_l2_mcastLookupMode_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - MC_LOOKUP_ON_VID
 *          - MC_LOOKUP_ON_FID
 *          - MC_LOOKUP_ON_FIXED_FID
 *      (2) When use MC_LOOKUP_ON_FIXED_FID, switch will use fixed_fid as lookup fid
 *      (3) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 *          The mode is return from first exist vlan id; fixed_fid is ignore
 */
int32
rtk_l2_mcastLookupMode_get(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    *pMcast_lookupMode,
    rtk_fid_t                   *pFixed_fid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastLookupMode_get(unit, pMcast_lookupMode, pFixed_fid);
} /* end of rtk_l2_mcastLookupMode_get */

/* Function Name:
 *      rtk_l2_mcastLookupMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_FID      - invalid fid
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Lookup mode for multicast address is as following
 *          - MC_LOOKUP_ON_VID
 *          - MC_LOOKUP_ON_FID
 *          - MC_LOOKUP_ON_FIXED_FID
 *      (2) When use MC_LOOKUP_ON_FIXED_FID, switch will use fixed_fid as lookup fid
 *      (3) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 *          The mcast_lookupMode is applied to all exist vlan; fixed_fid is ignore
 */
int32
rtk_l2_mcastLookupMode_set(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    mcast_lookupMode,
    rtk_fid_t                   fixed_fid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastLookupMode_set(unit, mcast_lookupMode, fixed_fid);
} /* end of rtk_l2_mcastLookupMode_set */

/* Function Name:
 *      rtk_l2_mcastBlockPortmask_get
 * Description:
 *      Get portmask that block multicast forwarding.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPortmask - pointer to portmask
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
rtk_l2_mcastBlockPortmask_get(uint32 unit, rtk_portmask_t *pPortmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastBlockPortmask_get(unit, pPortmask);
} /* end of rtk_l2_mcastBlockPortmask_get */

/* Function Name:
 *      rtk_l2_mcastBlockPortmask_set
 * Description:
 *      Set portmask that block multicast forwarding.
 * Input:
 *      unit      - unit id
 *      pPortmask - portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      For L2 multicast packet, its group portmask need to exclude the portmask
 *      to get the final fowarding portmask.
 */
int32
rtk_l2_mcastBlockPortmask_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastBlockPortmask_set(unit, pPortmask);
} /* end of rtk_l2_mcastBlockPortmask_set */

/* Function Name:
 *      rtk_l2_mcastAddr_init
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Need to initialize L2 multicast entry before add it.
 */
int32
rtk_l2_mcastAddr_init(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac, rtk_l2_mcastAddr_t *pMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastAddr_init(unit, vid, pMac, pMcast_addr);
} /* end of rtk_l2_mcastAddr_init */

/* Function Name:
 *      rtk_l2_mcastAddr_add
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Don't need to configure pMcast_addr->fwdIndex because driver automatically allocates a
 *          free portmask entry index and return it back to pMcast_addr->fwdIndex.
 *      (2) pMcast_addr->portmask is used to configure the allocated portmask entry.
 */
int32
rtk_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_mcastAddr_t  temp_mcastAddr;

    temp_mcastAddr = *pMcast_addr;
    temp_mcastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pMcast_addr).portmask);
    return RT_MAPPER(unit)->l2_mcastAddr_add(unit, &temp_mcastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_mcastAddr_add(unit, pMcast_addr);
#endif
} /* end of rtk_l2_mcastAddr_add */

/* Function Name:
 *      rtk_l2_mcastAddr_del
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
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_VLAN_VID       - invalid vlan id
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 *      RT_ERR_MAC            - invalid mac address
 *      RT_ERR_L2_HASH_KEY    - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY - the entry is empty(invalid)
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The corresponding portmask entry is cleared only if its reference count reaches 0.
 */
int32
rtk_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastAddr_del(unit, vid, pMac);
} /* end of rtk_l2_mcastAddr_del */

/* Function Name:
 *      rtk_l2_mcastAddr_get
 * Description:
 *      Get L2 multicast entry based on specified VID and MAC address.
 * Input:
 *      unit        - unit id
 *      pMcast_addr - L2 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_mcastAddr_t  temp_mcastAddr;

    temp_mcastAddr = *pMcast_addr;
    if ((ret = RT_MAPPER(unit)->l2_mcastAddr_get(unit, &temp_mcastAddr)) != RT_ERR_OK)
        return ret;
    *pMcast_addr = temp_mcastAddr;
    (*pMcast_addr).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_mcastAddr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_mcastAddr_get(unit, pMcast_addr);
#endif
} /* end of rtk_l2_mcastAddr_get */

/* Function Name:
 *      rtk_l2_mcastAddr_set
 * Description:
 *      Update content of L2 multicast entry.
 * Input:
 *      unit        - unit id
 * Output:
 *      pMcast_addr - pointer to L2 multicast entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_MAC          - invalid mac address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_l2_mcastAddr_set(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_mcastAddr_t  temp_mcastAddr;

    temp_mcastAddr = *pMcast_addr;
    temp_mcastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pMcast_addr).portmask);
    return RT_MAPPER(unit)->l2_mcastAddr_set(unit, &temp_mcastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_mcastAddr_set(unit, pMcast_addr);
#endif
} /* end of rtk_l2_mcastAddr_set */

/* Function Name:
 *      rtk_l2_mcastAddr_add_with_index
 * Description:
 *      Add a L2 multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                   - unit id
 *      vid                    - vlan id
 *      pMcast_addr            - content of L2 multicast address entry
 *      pMacast_addr->fwdIndex - index of portmask entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_L2_HASH_KEY        - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_VLAN_VID           - invalid vlan id
 *      RT_ERR_MAC                - invalid mac address
 *      RT_ERR_L2_MULTI_FWD_INDEX - invalid index of multicast forwarding portmask
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) If pMcast_addr->fwdIndex is larger than or equal to 0, it is used for pointing to portmask entry.
 *          In this case, the portmask entry should be configured through rtk_l2_mcastFwdPortmask_set
 *          and pMcast_addr->portmask is not used.
 *
 *      (2) If pMcast_addr->fwdIndex is smaller than 0, driver automatically allocates a free portmask entry index
 *          and return it back to pMcast_addr->fwdIndex. In this case, pMcast_addr->portmask is used to
 *          configure the allocated portmask entry.
 */
int32
rtk_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_mcastAddr_t  temp_mcastAddr;

    temp_mcastAddr = *pMcast_addr;
    temp_mcastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pMcast_addr).portmask);
    return RT_MAPPER(unit)->l2_mcastAddr_add_with_index(unit, &temp_mcastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_mcastAddr_add_with_index(unit, pMcast_addr);
#endif
} /* end of rtk_l2_mcastAddr_add_with_index */

/* Function Name:
 *      rtk_l2_mcastAddr_get_with_index
 * Description:
 *      Get a L2 multicast address entry and multicast index from the specified device.
 * Input:
 *      unit        - unit id
 * Output:
 *      pMcast_addr - pointer to content of L2 multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_VLAN_VID           - invalid vlan id
 *      RT_ERR_MAC                - invalid mac address
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_INPUT              - invalid input parameter
 *      RT_ERR_L2_MULTI_FWD_INDEX - invalid index of multicast forwarding portmask
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      pMcast_addr->rvid and pMcast_addr->mac are used to retrieve the multicast entry.
 */
int32
rtk_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_mcastAddr_t  temp_mcastAddr;

    temp_mcastAddr = *pMcast_addr;
    if ((ret = RT_MAPPER(unit)->l2_mcastAddr_get_with_index(unit, &temp_mcastAddr)) != RT_ERR_OK)
        return ret;
    *pMcast_addr = temp_mcastAddr;
    (*pMcast_addr).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_mcastAddr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_mcastAddr_get_with_index(unit, pMcast_addr);
#endif
} /* end of rtk_l2_mcastAddr_get_with_index */

/* Module Name    : L2           */
/* Sub-module Name: IP multicast */

/* Function Name:
 *      rtk_l2_ipmcEnable_get
 * Description:
 *      Get enable status of layer2 ip multicast switching.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of layer3 ip multicast switching
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
rtk_l2_ipmcEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcEnable_get(unit, pEnable);
} /* end of rtk_l2_ipmcEnable_get */

/* Function Name:
 *      rtk_l2_ipmcEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_ipmcEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcEnable_set(unit, enable);
} /* end of rtk_l2_ipmcEnable_set */

/* Function Name:
 *      rtk_l2_ipmcMode_get
 * Description:
 *      Get lookup mode of layer2 ip multicast switching.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to lookup mode of layer2 ip multicast switching
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 *      - LOOKUP_ON_DIP_ONLY
 */
int32
rtk_l2_ipmcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcMode_get(unit, pMode);
} /* end of rtk_l2_ipmcMode_get */

/* Function Name:
 *      rtk_l2_ipmcMode_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 *      - LOOKUP_ON_DIP_ONLY
 */
int32
rtk_l2_ipmcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcMode_set(unit, mode);
} /* end of rtk_l2_ipmcMode_set */

/* Function Name:
 *      rtk_l2_ipMcastAddr_init
 * Description:
 *      Initialize content of buffer of IP multicast entry.
 *      Destination IP ,source IP and other fields of IP multicast entry are reset.
 * Input:
 *      unit          - unit id
 *      dip           - destination IP
 *      sip           - source IP
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_IPV4_ADDRESS - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
rtk_l2_ipMcastAddr_init(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipMcastAddr_init(unit, sip, dip, pIpmcast_addr);
} /* end of rtk_l2_ipMcastAddr_init */

/* Function Name:
 *      rtk_l2_ipMcastAddrExt_init
 * Description:
 *      Initialize content of buffer of IP multicast entry.
 *      Will destination IP ,source IP and reset other field of IP multicast entry.
 * Input:
 *      unit          		- unit id
 *      pIpMcast_hashKey 	- the hash key to initialize content of buffer
 *      pIpMcast_addr 		- IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_IPV4_ADDRESS - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Need to initialize IP multicast entry before add it.
 */
int32
rtk_l2_ipMcastAddrExt_init(uint32 unit, rtk_l2_ipMcastHashKey_t *pIpMcast_hashKey, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipMcastAddrExt_init(unit, pIpMcast_hashKey, pIpMcast_addr);
}

/* Function Name:
 *      rtk_l2_ipMcastAddr_add
 * Description:
 *      Add IP multicast entry to ASIC.
 * Input:
 *      unit          - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_INPUT              - invalid input parameter
 *      RT_ERR_IPV4_ADDRESS       - Invalid IPv4 address
 *      RT_ERR_VLAN_VID           - invalid vlan id
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_ENTRY_EXIST     - the entry is already existed
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ipmcMode_set for switching the IP multicast lookup mode.
 *      (2) Don't need to configure pIpmcast_addr->fwdIndex because driver automatically allocates a
 *          free portmask entry index and return it back to pIpmcast_addr->fwdIndex.
 *      (3) pIpmcast_addr->portmask is used to configure the allocated portmask entry.
 */
int32
rtk_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ipMcastAddr_t  temp_ipMcastAddr;

    temp_ipMcastAddr = *pIpmcast_addr;
    temp_ipMcastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pIpmcast_addr).portmask);
    return RT_MAPPER(unit)->l2_ipMcastAddr_add(unit, &temp_ipMcastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_ipMcastAddr_add(unit, pIpmcast_addr);
#endif
} /* end of rtk_l2_ipMcastAddr_add */

/* Function Name:
 *      rtk_l2_ipMcastAddr_del
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
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_L2_HASH_KEY            - invalid L2 Hash key
 *      RT_ERR_VLAN_VID               - invalid vlan id
 *      RT_ERR_L2_EMPTY_ENTRY         - the entry is empty(invalid)
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ipmcMode_set for switching the IP multicast lookup mode.
 *      (2) The corresponding portmask entry is cleared only if its reference count reaches 0.
 */
int32
rtk_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipMcastAddr_del(unit, sip, dip, vid);
} /* end of rtk_l2_ipMcastAddr_del */

/* Function Name:
 *      rtk_l2_ipMcastAddr_get
 * Description:
 *      Get IPv4 multicast entry based on specified hash key.
 * Input:
 *      unit          - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_IPV4_ADDRESS - Invalid IPv4 address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ipmcMode_set for switching the IP multicast lookup mode.
 */
int32
rtk_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ipMcastAddr_t  temp_ipMcastAddr;

    temp_ipMcastAddr = *pIpmcast_addr;
    if ((ret = RT_MAPPER(unit)->l2_ipMcastAddr_get(unit, &temp_ipMcastAddr)) != RT_ERR_OK)
        return ret;
    *pIpmcast_addr = temp_ipMcastAddr;
    (*pIpmcast_addr).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_ipMcastAddr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_ipMcastAddr_get(unit, pIpmcast_addr);
#endif
} /* end of rtk_l2_ipMcastAddr_get */

/* Function Name:
 *      rtk_l2_ipMcastAddr_set
 * Description:
 *      Update content of IP multicast entry.
 * Input:
 *      unit          - unit id
 *      pIpmcast_addr - IP multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_IPV4_ADDRESS           - Invalid IPv4 address
 *      RT_ERR_VLAN_VID               - invalid vlan id
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ipmcMode_set for switching the IP multicast lookup mode.
 */
int32
rtk_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ipMcastAddr_t  temp_ipMcastAddr;

    temp_ipMcastAddr = *pIpmcast_addr;
    temp_ipMcastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pIpmcast_addr).portmask);
    return RT_MAPPER(unit)->l2_ipMcastAddr_set(unit, &temp_ipMcastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_ipMcastAddr_set(unit, pIpmcast_addr);
#endif
} /* end of rtk_l2_ipMcastAddr_set */

/* Function Name:
 *      rtk_l2_ipMcastAddr_add_with_index
 * Description:
 *      Add a IP multicast address entry and multicast index to the specified device.
 * Input:
 *      unit                     - unit id
 *      pIpMcast_addr            - content of IP multicast address entry
 * Output:
 *      pIpMacast_addr->fwdIndex - index of multicast forwarding entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_L2_HASH_KEY            - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX     - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ipmcMode_set for switching the IP multicast lookup mode.
 *      (2) If pIpMcast_addr->fwdIndex is larger than or equal to 0, it is used for pointing to portmask entry.
 *          In this case, the portmask entry should be configured through rtk_l2_mcastFwdPortmask_set
 *          and pIpMcast_addr->portmask is not used.
 *      (3) If pIpMcast_addr->fwdIndex is smaller than 0, driver automatically allocates a free portmask entry index
 *          and return it back to pIpMcast_addr->fwdIndex. In this case, pIpMcast_addr->portmask is used to
 *          configure the allocated portmask entry.
 */
int32
rtk_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ipMcastAddr_t  temp_ipMcastAddr;

    temp_ipMcastAddr = *pIpMcast_addr;
    temp_ipMcastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pIpMcast_addr).portmask);
    return RT_MAPPER(unit)->l2_ipMcastAddr_add_with_index(unit, &temp_ipMcastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_ipMcastAddr_add_with_index(unit, pIpMcast_addr);
#endif
} /* end of rtk_l2_ipMcastAddr_add_with_index */

/* Function Name:
 *      rtk_l2_ipMcastAddr_get_with_index
 * Description:
 *      Get a IP multicast address entry and multicast index from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pIpMcast_addr - pointer to content of IP multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_L2_HASH_KEY            - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX     - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ipmcMode_set for switching the IP multicast lookup mode.
 */
int32
rtk_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ipMcastAddr_t  temp_ipMcastAddr;

    temp_ipMcastAddr = *pIpMcast_addr;
    if ((ret = RT_MAPPER(unit)->l2_ipMcastAddr_get_with_index(unit, &temp_ipMcastAddr)) != RT_ERR_OK)
        return ret;
    *pIpMcast_addr = temp_ipMcastAddr;
    (*pIpMcast_addr).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_ipMcastAddr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_ipMcastAddr_get_with_index(unit, pIpMcast_addr);
#endif
} /* end of rtk_l2_ipMcastAddr_get_with_index */

/* Function Name:
 *      rtk_l2_ipMcastAddrChkEnable_get
 * Description:
 *      Get DIP check status for IPv4 multicast packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to DIP check status for IPv4 multicast packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Enable the function to qualify the DIP. If the function is enabled, the packet is deemed to be
 *      IPv4 multicast packet only if DMAC[40]=1 & DIP[31:28]=1110.
 */
int32
rtk_l2_ipMcastAddrChkEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipMcastAddrChkEnable_get(unit, pEnable);
} /* end of rtk_l2_ipMcastAddrChkEnable_get */

/* Function Name:
 *      rtk_l2_ipMcastAddrChkEnable_set
 * Description:
 *      Set DIP check status for IPv4 multicast packet.
 * Input:
 *      unit   - unit id
 *      enable - DIP check status for IPv4 multicast packet
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
 *      Enable the function to qualify the DIP. If the function is enabled, the packet is deemed to be
 *      IPv4 multicast packet only if DMAC[40]=1 & DIP[31:28]=1110.
 */
int32
rtk_l2_ipMcastAddrChkEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipMcastAddrChkEnable_set(unit, enable);
} /* end of rtk_l2_ipMcastAddrChkEnable_set */

/* Function Name:
 *      rtk_l2_ipMcstFidVidCompareEnable_get
 * Description:
 *      Get VID/FID comparison configuration of IP multicast entry.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to FID/VID comparison status for IPv4 multicast packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      While IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP, in addition to use SIP and DIP to
 *      lookup the IP multicast packet, VID/FID can also be used together to doing lookup through the API.
 */
int32
rtk_l2_ipMcstFidVidCompareEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipMcstFidVidCompareEnable_get(unit, pEnable);
} /* end of rtk_l2_ipMcstFidVidCompareEnable_get */

/* Function Name:
 *      rtk_l2_ipMcstFidVidCompareEnable_set
 * Description:
 *      Set VID/FID comparison configuration of IP multicast entry.
 * Input:
 *      unit   - unit id
 *      enable - configure value
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
 *      While IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP, in addition to use SIP and DIP to
 *      lookup the IP multicast packet, VID/FID can also be used to doing lookup through the API.
 */
int32
rtk_l2_ipMcstFidVidCompareEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipMcstFidVidCompareEnable_set(unit, enable);
} /* end of rtk_l2_ipMcstFidVidCompareEnable_set */

/* Function Name:
 *      rtk_l2_ipmc_routerPorts_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_ipmc_routerPorts_get(uint32 unit, rtk_portmask_t *pPortmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmc_routerPorts_get(unit, pPortmask);
} /* end of rtk_l2_ipmc_routerPorts_get */

/* Function Name:
 *      rtk_l2_ipmc_routerPorts_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_MASK    - invalid portmask
 * Applicable:
 *      8328
 * Note:
 *      For IP multicast packet, its group portmask need to include the portmask
 *      to get the final fowarding portmask.
 */
int32
rtk_l2_ipmc_routerPorts_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmc_routerPorts_set(unit, pPortmask);
} /* end of rtk_l2_ipmc_routerPorts_set */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid mismatch type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Mismatch type is as following:
 *          - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *          - L2_IPMC_MIS_TYPE_UCAST_ADDR
 *      (2) Forwarding action is as following:
 *          - L2_IPMC_MIS_DROP
 *          - L2_IPMC_MIS_TRAP
 *          - L2_IPMC_MIS_FWD_AS_L2
 *          - L2_IPMC_MIS_FWD_AS_IPMC
 */
int32
rtk_l2_ipmcDstAddrMismatchAction_get(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    *pMismatch_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchAction_get(unit, type, pMismatch_action);
} /* end of rtk_l2_ipmcDstAddrMismatchAction_get */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchAction_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_INPUT      - invalid mismatch type
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      (1) Mismatch type is as following:
 *          - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *          - L2_IPMC_MIS_TYPE_UCAST_ADDR
 *      (2) Forwarding action is as following:
 *          - L2_IPMC_MIS_DROP
 *          - L2_IPMC_MIS_TRAP
 *          - L2_IPMC_MIS_FWD_AS_L2
 *          - L2_IPMC_MIS_FWD_AS_IPMC
 */
int32
rtk_l2_ipmcDstAddrMismatchAction_set(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    mismatch_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchAction_set(unit, type, mismatch_action);
} /* end of rtk_l2_ipmcDstAddrMismatchAction_set */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchPri_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid mismatch type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchPri_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchPri_get(unit, type, pPriority);
} /* end of rtk_l2_ipmcDstAddrMismatchPri_get */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid mismatch type
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchPri_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchPri_set(unit, type, priority);
} /* end of rtk_l2_ipmcDstAddrMismatchPri_set */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit    - unit id
 *      type    - mismatch type
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchPriEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchPriEnable_get(unit, type, pEnable);
} /* end of rtk_l2_ipmcDstAddrMismatchPriEnable_get */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchPriEnable_set
 * Description:
 *      Set priority priority assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit   - unit id
 *      type   - mismatch type
 *      enable - priority assignment status for trapped packet
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
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchPriEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchPriEnable_set(unit, type, enable);
} /* end of rtk_l2_ipmcDstAddrMismatchPriEnable_set */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchDP_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid mismatch type
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchDP_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchDP_get(unit, type, pDp);
} /* end of rtk_l2_ipmcDstAddrMismatchDP_get */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchDP_set
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
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_INPUT           - invalid mismatch type
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 * Applicable:
 *      8328
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchDP_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchDP_set(unit, type, dp);
} /* end of rtk_l2_ipmcDstAddrMismatchDP_set */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit    - unit id
 *      type    - mismatch type
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchDPEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchDPEnable_get(unit, type, pEnable);
} /* end of rtk_l2_ipmcDstAddrMismatchDPEnable_get */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet when destination MAC address of ip multicast packet is mismatch.
 * Input:
 *      unit   - unit id
 *      type   - mismatch type
 *      enable - drop procedence assignment status for trapped packet
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
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchDPEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchDPEnable_set(unit, type, enable);
} /* end of rtk_l2_ipmcDstAddrMismatchDPEnable_set */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchAddCPUTagEnable_get(unit, type, pEnable);
} /* end of rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get */

/* Function Name:
 *      rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      Mismatch type is as following:
 *      - L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR
 *      - L2_IPMC_MIS_TYPE_UCAST_ADDR
 */
int32
rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ipmcDstAddrMismatchAddCPUTagEnable_set(unit, type, enable);
} /* end of rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set */

/* Function Name:
 *      rtk_l2_ip6mcMode_get
 * Description:
 *      Get lookup mode of layer2 IPv6 multicast switching.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer to lookup mode of layer2 IPv6 multicast switching
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 *      - LOOKUP_ON_DIP_ONLY
 */
int32
rtk_l2_ip6mcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ip6mcMode_get(unit, pMode);
} /* end of rtk_l2_ip6mcMode_get */

/* Function Name:
 *      rtk_l2_ip6mcMode_set
 * Description:
 *      Set lookup mode of layer2 IPv6 multicast switching.
 * Input:
 *      unit - unit id
 *      mode - lookup mode of layer2 IPv6 multicast switching
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
 *      Lookup mode of layer2 ip multicast switching is as following
 *      - LOOKUP_ON_FVID_AND_MAC
 *      - LOOKUP_ON_DIP_AND_SIP
 *      - LOOKUP_ON_DIP_AND_FVID
 *      - LOOKUP_ON_DIP_ONLY
 */
int32
rtk_l2_ip6mcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ip6mcMode_set(unit, mode);
} /* end of rtk_l2_ip6mcMode_set */

/* Function Name:
 *      rtk_l2_ip6CareByte_get
 * Description:
 *      Get the hash care-byte of IPv6 DIP/SIP address. These bytes are used to compose the LUT hash key.
 * Input:
 *      unit      - unit id
 *      type      - type of care-byte
 * Output:
 *      pCareByte - pointer to the care-byte of IPv6 DIP/SIP address
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
 *      (1) careByte = 0x3210, menas take IPv6 IP address byte 3,2,1,0 to compose the hash key.
 *      (2) Byte ordering: 2001:DB8:2de::e13, byte[15]='20', byte[0]='13'.
 */
int32
rtk_l2_ip6CareByte_get(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 *pCareByte)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ip6CareByte_get(unit, type, pCareByte);
} /* end of rtk_l2_ip6CareByte_get */

/* Function Name:
 *      rtk_l2_ip6CareByte_set
 * Description:
 *      Set the hash care-byte of IPv6 DIP/SIP address. These bytes are used to compose the LUT hash key.
 * Input:
 *      unit     - unit id
 *      type     - type of care-byte
 *      careByte - the care-byte of IPv6 DIP/SIP address
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
 *      (1) careByte = 0x3210, menas take IPv6 IP address byte 3,2,1,0 to compose the hash key.
 *      (2) Byte ordering: 2001:DB8:2de::e13, byte[15]='20', byte[0]='13'.
 */
int32
rtk_l2_ip6CareByte_set(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 careByte)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ip6CareByte_set(unit, type, careByte);
} /* end of rtk_l2_ip6CareByte_set */

/* Function Name:
 *      rtk_l2_ip6McastAddrExt_init
 * Description:
 *      Initialize content of buffer of IPv6 multicast entry.
 *      Will destination IP ,source IP (or vid) and reset other field of IPv6 multicast entry.
 * Input:
 *      unit          		- unit id
 *      pIp6Mcast_hashKey 	- the hash key to initialize content of buffer
 *      pIp6Mcast_addr    	- IPv6 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_IPV6_ADDRESS     - Invalid IPv6 address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Need to initialize IPv6 multicast entry before add it.
 */
int32
rtk_l2_ip6McastAddrExt_init(uint32 unit, rtk_l2_ip6McastHashKey_t *pIp6Mcast_hashKey, rtk_l2_ip6McastAddr_t *pIp6Mcast_addr)
{ 
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ip6McastAddrExt_init(unit, pIp6Mcast_hashKey, pIp6Mcast_addr);
}

/* Function Name:
 *      rtk_l2_ip6McastAddr_add
 * Description:
 *      Add IPv6 multicast entry to ASIC.
 * Input:
 *      unit          - unit id
 *      pIpmcast_addr - IPv6 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_IPV6_ADDRESS - Invalid IPv6 address
 *      RT_ERR_VLAN_VID     - invalid vlan id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ip6mcMode_set for switching the IP multicast lookup mode.
 *      (2) Don't need to configure pIpmcast_addr->fwdIndex because driver automatically allocates a
 *          free portmask entry index and return it back to pIpmcast_addr->fwdIndex.
 *      (3) pIpmcast_addr->portmask is used to configure the allocated portmask entry.
 */
int32
rtk_l2_ip6McastAddr_add(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ip6McastAddr_t  temp_ip6McastAddr;

    temp_ip6McastAddr = *pIpmcast_addr;
    temp_ip6McastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pIpmcast_addr).portmask);
    return RT_MAPPER(unit)->l2_ip6McastAddr_add(unit, &temp_ip6McastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_ip6McastAddr_add(unit, pIpmcast_addr);
#endif
} /* end of rtk_l2_ip6McastAddr_add */

/* Function Name:
 *      rtk_l2_ip6McastAddr_del
 * Description:
 *      Delete a L2 IPv6 multicast address entry from the specified device.
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
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_L2_HASH_KEY            - invalid L2 Hash key
 *      RT_ERR_L2_EMPTY_ENTRY         - the entry is empty(invalid)
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ip6mcMode_set for switching the IP multicast lookup mode.
 *      (2) The corresponding portmask entry is cleared only if its reference count reaches 0.
 */
int32
rtk_l2_ip6McastAddr_del(uint32 unit, rtk_ipv6_addr_t sip, rtk_ipv6_addr_t dip, rtk_vlan_t vid)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_ip6McastAddr_del(unit, sip, dip, vid);
} /* end of rtk_l2_ip6McastAddr_del */

/* Function Name:
 *      rtk_l2_ip6McastAddr_get
 * Description:
 *      Get IPv6 multicast entry based on specified hash key.
 * Input:
 *      unit          - unit id
 *      pIpmcast_addr - IPv6 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ip6mcMode_set for switching the IP multicast lookup mode.
 */
int32
rtk_l2_ip6McastAddr_get(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ip6McastAddr_t  temp_ipMcastAddr;

    temp_ipMcastAddr = *pIpmcast_addr;
    if ((ret = RT_MAPPER(unit)->l2_ip6McastAddr_get(unit, &temp_ipMcastAddr)) != RT_ERR_OK)
        return ret;
    *pIpmcast_addr = temp_ipMcastAddr;
    (*pIpmcast_addr).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_ipMcastAddr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_ip6McastAddr_get(unit, pIpmcast_addr);
#endif
} /* end of rtk_l2_ip6McastAddr_get */

/* Function Name:
 *      rtk_l2_ip6McastAddr_set
 * Description:
 *      Update content of IPv6 multicast entry.
 * Input:
 *      unit          - unit id
 *      pIpmcast_addr - IPv6 multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_VLAN_VID               - invalid vlan id
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_INPUT                  - invalid input parameter
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ip6mcMode_set for switching the IP multicast lookup mode.
 */
int32
rtk_l2_ip6McastAddr_set(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ip6McastAddr_t  temp_ip6McastAddr;

    temp_ip6McastAddr = *pIpmcast_addr;
    temp_ip6McastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pIpmcast_addr).portmask);
    return RT_MAPPER(unit)->l2_ip6McastAddr_set(unit, &temp_ip6McastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_ip6McastAddr_set(unit, pIpmcast_addr);
#endif
} /* end of rtk_l2_ip6McastAddr_set */

/* Function Name:
 *      rtk_l2_ip6McastAddr_add_with_index
 * Description:
 *      Add a IPv6 multicast entry and multicast index to the specified device.
 * Input:
 *      unit                     - unit id
 *      pIpMcast_addr            - content of IPv6 multicast address entry
 * Output:
 *      pIpMacast_addr->fwdIndex - index of multicast forwarding entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_L2_HASH_KEY            - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX     - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ip6mcMode_set for switching the IP multicast lookup mode.
 *      (2) If pIpMcast_addr->fwdIndex is larger than or equal to 0, it is used for pointing to portmask entry.
 *          In this case, the portmask entry should be configured through rtk_l2_mcastFwdPortmask_set
 *          and pIpMcast_addr->portmask is not used.
 *      (3) If pIpMcast_addr->fwdIndex is smaller than 0, driver automatically allocates a free portmask entry index
 *          and return it back to pIpMcast_addr->fwdIndex. In this case, pIpMcast_addr->portmask is used to
 *          configure the allocated portmask entry.
 */
int32
rtk_l2_ip6McastAddr_add_with_index(uint32 unit, rtk_l2_ip6McastAddr_t *pIpMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_l2_ip6McastAddr_t  temp_ip6McastAddr;

    temp_ip6McastAddr = *pIpMcast_addr;
    temp_ip6McastAddr.portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, (*pIpMcast_addr).portmask);
    return RT_MAPPER(unit)->l2_ip6McastAddr_add_with_index(unit, &temp_ip6McastAddr);
    }
#else
    return RT_MAPPER(unit)->l2_ip6McastAddr_add_with_index(unit, pIpMcast_addr);
#endif
} /* end of rtk_l2_ip6McastAddr_add_with_index */

/* Function Name:
 *      rtk_l2_ip6McastAddr_get_with_index
 * Description:
 *      Get a IPv6 multicast entry and multicast index from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pIpMcast_addr - pointer to content of IP multicast address entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_L2_HASH_KEY            - invalid L2 Hash key
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX     - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Hash key are (vid, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_FVID.
 *          Hash key are (sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is disabled.
 *          Hash key are (vid, sip, dip) if IP multicast lookup mode is LOOKUP_ON_DIP_AND_SIP & rtk_l2_ipMcstFidVidCompareEnable_set is enabled.
 *          Refer to rtk_l2_ip6mcMode_set for switching the IP multicast lookup mode.
 */
int32
rtk_l2_ip6McastAddr_get_with_index(uint32 unit, rtk_l2_ip6McastAddr_t *pIpMcast_addr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ip6McastAddr_t  temp_ipMcastAddr;

    temp_ipMcastAddr = *pIpMcast_addr;
    if ((ret = RT_MAPPER(unit)->l2_ip6McastAddr_get_with_index(unit, &temp_ipMcastAddr)) != RT_ERR_OK)
        return ret;
    *pIpMcast_addr = temp_ipMcastAddr;
    (*pIpMcast_addr).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_ipMcastAddr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_ip6McastAddr_get_with_index(unit, pIpMcast_addr);
#endif
} /* end of rtk_l2_ip6McastAddr_get_with_index */

/* Function Name:
 *      rtk_l2_nextValidIp6McastAddr_get
 * Description:
 *      Get next valid L2 IPv6 multicast entry from the specified device.
 * Input:
 *      unit      - unit id
 *      pScan_idx - currently scan index of l2 table to get next.
 * Output:
 *      pL2_data  - structure of l2 address data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_VLAN_VID               - invalid vid
 *      RT_ERR_NULL_POINTER           - input parameter may be null pointer
 *      RT_ERR_L2_ENTRY_NOTFOUND      - specified entry not found
 *      RT_ERR_L2_IPMCAST_LOOKUP_MODE - invalid IP multicast lookup mode
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The function will skip valid l2 unicast, multicast entry and IP multicast entry and
 *          returned next valid IPv6 multicast address which based on index order of L2 table.
 *      (2) Input -1 for getting the first entry of L2 table.
 *      (3) The pScan_idx is both the input and output argument.
 */
int32
rtk_l2_nextValidIp6McastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ip6McastAddr_t    *pL2_data)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_ip6McastAddr_t  temp_ipMcastAddr;

    if ((ret = RT_MAPPER(unit)->l2_nextValidIp6McastAddr_get(unit, pScan_idx, &temp_ipMcastAddr)) != RT_ERR_OK)
        return ret;
    *pL2_data = temp_ipMcastAddr;
    (*pL2_data).portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_ipMcastAddr.portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_nextValidIp6McastAddr_get(unit, pScan_idx, pL2_data);
#endif
} /* end of rtk_l2_nextValidIp6McastAddr_get */

/* Module Name    : L2                         */
/* Sub-module Name: Multicast forwarding table */

/* Function Name:
 *      rtk_l2_mcastFwdIndex_alloc
 * Description:
 *      Allocate index for multicast forwarding entry
 * Input:
 *      unit      - unit id
 *      pFwdIndex - pointer to index of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX       - invalid index of multicast forwarding entry
 *      RT_ERR_L2_MCAST_FWD_ENTRY_EXIST - Mcast forwarding entry already exist
 *      RT_ERR_L2_INDEXTBL_FULL         - L2 index table is full
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) If pFwdIndex is larger than or equal to 0, will use pFwdIndex as multicast index.
 *      (2) If pFwdIndex is smaller than 0, will allocate a free index and return it.
 *      (3) The reference count corresponds to the pFwdIndex is increased after a successfully allocation.
 */
int32
rtk_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastFwdIndex_alloc(unit, pFwdIndex);
} /* end of rtk_l2_mcastFwdIndex_alloc */

/* Function Name:
 *      rtk_l2_mcastFwdIndex_free
 * Description:
 *      Free index for multicast forwarding entry
 * Input:
 *      unit  - unit id
 *      index - index of multicast forwarding entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                      - invalid unit id
 *      RT_ERR_NOT_INIT                     - The module is not initial
 *      RT_ERR_L2_MULTI_FWD_INDEX           - invalid index of multicast forwarding portmask
 *      RT_ERR_L2_MCAST_FWD_ENTRY_NOT_EXIST - index of forwarding entry is not exist
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of indx is 0 ~ (multicast forwarding table size - 1)
 *      (2) The reference count corresponds to the pFwdIndex is decreased after a successfully free.
 */
int32
rtk_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastFwdIndex_free(unit, index);
} /* end of rtk_l2_mcastFwdIndex_free */

/* Function Name:
 *      rtk_l2_mcastFwdIndexFreeCount_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_mcastFwdIndexFreeCount_get(unit, pFreeCount);
} /* end of rtk_l2_mcastFwdIndexFreeCount_get */

/* Function Name:
 *      rtk_l2_mcastFwdPortmask_get
 * Description:
 *      Get portmask of multicast forwarding entry
 * Input:
 *      unit       - unit id
 *      index      - index of multicast forwarding portmask
 * Output:
 *      pPortmask  - pointer buffer of multicast ports
 *      pCrossVlan - pointer of cross vlan flag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX - invalid index of multicast forwarding portmask
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of indx is 0 ~ (multicast forwarding table size - 1)
 *      (2) pCrossVlan is only applicable to 8328
 */
int32
rtk_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          *pCrossVlan)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_portmask_t  temp_portmask;

    if ((ret = RT_MAPPER(unit)->l2_mcastFwdPortmask_get(unit, index, &temp_portmask, pCrossVlan)) != RT_ERR_OK)
        return ret;
    *pPortmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_portmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_mcastFwdPortmask_get(unit, index, pPortmask, pCrossVlan);
#endif
} /* end of rtk_l2_mcastFwdPortmask_get */

/* Function Name:
 *      rtk_l2_mcastFwdPortmask_set
 * Description:
 *      Set portmask of multicast forwarding entry
 * Input:
 *      unit      - unit id
 *      index     - index of multicast forwarding portmask
 *      pPortmask - pointer buffer of multicast ports
 *      crossVlan - cross vlan flag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_L2_MULTI_FWD_INDEX - invalid index of multicast forwarding portmask
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The valid range of indx is 0 ~ (multicast forwarding table size - 1)
 *      (2) crossVlan is only applicable to 8328
 */
int32
rtk_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          crossVlan)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_portmask_t  temp_portmask;

    temp_portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, *pPortmask);
    return RT_MAPPER(unit)->l2_mcastFwdPortmask_set(unit, index, &temp_portmask, crossVlan);
    }
#else
    return RT_MAPPER(unit)->l2_mcastFwdPortmask_set(unit, index, pPortmask, crossVlan);
#endif
} /* end of rtk_l2_mcastFwdPortmask_set */

/* Module Name    : L2              */
/* Sub-module Name: CPU MAC address */

/* Function Name:
 *      rtk_l2_cpuMacAddr_add
 * Description:
 *      Add a CPU mac address entry to the lookup table.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vid
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The packet destined to the CPU MAC is then forwarded to CPU port.
 */
int32
rtk_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_cpuMacAddr_add(unit, vid, pMac);
} /* end of rtk_l2_cpuMacAddr_add */

/* Function Name:
 *      rtk_l2_cpuMacAddr_del
 * Description:
 *      Delete a CPU mac address entry from the lookup table.
 * Input:
 *      unit - unit id
 *      vid  - vlan id
 *      pMac - cpu mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_VLAN_VID     - invalid vid
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_cpuMacAddr_del(unit, vid, pMac);
} /* end of rtk_l2_cpuMacAddr_del */

/* Module Name    : L2        */
/* Sub-module Name: Port move */

/* Function Name:
 *      rtk_l2_legalMoveToPorts_get
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
rtk_l2_legalMoveToPorts_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_legalMoveToPorts_get(unit, port, pLegalPorts);
} /* end of rtk_l2_legalMoveToPorts_get */

/* Function Name:
 *      rtk_l2_legalMoveToPorts_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PORT_MASK    - invalid portmask
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_legalMoveToPorts_set(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_legalMoveToPorts_set(unit, port, pLegalPorts);
} /* end of rtk_l2_legalMoveToPorts_set */

/* Function Name:
 *      rtk_l2_illegalPortMoveAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
rtk_l2_illegalPortMoveAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        *pFwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_illegalPortMoveAction_get(unit, port, pFwdAction);
} /* end of rtk_l2_illegalPortMoveAction_get */

/* Function Name:
 *      rtk_l2_illegalPortMoveAction_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
rtk_l2_illegalPortMoveAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        fwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_illegalPortMoveAction_set(unit, port, fwdAction);
} /* end of rtk_l2_illegalPortMoveAction_set */

/* Function Name:
 *      rtk_l2_legalPortMoveAction_get
 * Description:
 *      Get forwarding action when legal port moving is detected on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
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
 *      (1) For 8328, the action refer to the original port configuration.
 *      (2) For 8390 & 8380, the action refer to the new port configuration.
 */
int32
rtk_l2_legalPortMoveAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        *pFwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_legalPortMoveAction_get(unit, phy_port, pFwdAction);
    }
#else
    return RT_MAPPER(unit)->l2_legalPortMoveAction_get(unit, port, pFwdAction);
#endif
} /* end of rtk_l2_legalPortMoveAction_get */

/* Function Name:
 *      rtk_l2_legalPortMoveAction_set
 * Description:
 *      Set forwarding action when legal port moving is detected on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) For 8328, the action refer to the original port configuration.
 *      (2) For 8390 & 8380, the action refer to the new port configuration.
 */
int32
rtk_l2_legalPortMoveAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        fwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_legalPortMoveAction_set(unit, phy_port, fwdAction);
    }
#else
    return RT_MAPPER(unit)->l2_legalPortMoveAction_set(unit, port, fwdAction);
#endif
} /* end of rtk_l2_legalPortMoveAction_set */

/* Function Name:
 *      rtk_l2_legalPortMoveFlushAddrEnable_get
 * Description:
 *      Get the configuration of HW flush moved-port's mac of the specified device.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status
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
 *      (1) Enable the function to delete the port moved entry when the port moving is detected.
 *          That is, the corresponding L2 unicast entry is deleted if it is port moved.
 *      (2) Regarding the port move forwarding behavior, refer to rtk_l2_legalPortMoveAction_set.
 *      (3) The configuration applies to new port.
 */
int32
rtk_l2_legalPortMoveFlushAddrEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_legalPortMoveflushAddrEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->l2_legalPortMoveflushAddrEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_l2_legalPortMoveFlushAddrEnable_get */

/* Function Name:
 *      rtk_l2_legalPortMoveFlushAddrEnable_set
 * Description:
 *      Set the configuration of HW flush moved-port's mac of the specified device.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
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
 *      (1) Enable the function to delete the port moved entry when the port moving is detected.
 *          That is, the corresponding L2 unicast entry is deleted if it is port moved.
 *      (2) Regarding the port move forwarding behavior, refer to rtk_l2_legalPortMoveAction_set.
 *      (3) The configuration applies to new port.
 */
int32
rtk_l2_legalPortMoveFlushAddrEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_legalPortMoveflushAddrEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->l2_legalPortMoveflushAddrEnable_set(unit, port, enable);
#endif
} /* end of rtk_l2_legalPortMoveFlushAddrEnable_set */

/* Function Name:
 *      rtk_l2_staticPortMoveAction_get
 * Description:
 *      Get forwarding action when static entry port moving is detected on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 * Output:
 *      pFwdAction - pointer to forwarding action
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
 *      (1) Forwarding action is as following
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_COPY2CPU
 *      (2) The configuration applies to original port.
 */
int32
rtk_l2_staticPortMoveAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pFwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_staticPortMoveAction_get(unit, phy_port, pFwdAction);
    }
#else
    return RT_MAPPER(unit)->l2_staticPortMoveAction_get(unit, port, pFwdAction);
#endif
} /* end of rtk_l2_staticPortMoveAction_get */

/* Function Name:
 *      rtk_l2_staticPortMoveAction_set
 * Description:
 *      Set forwarding action when static entry port moving is detected on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      fwdAction - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Forwarding action is as following
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_COPY2CPU
 *      (2) The configuration applies to original port.
 */
int32
rtk_l2_staticPortMoveAction_set(uint32 unit, rtk_port_t port, rtk_action_t fwdAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_staticPortMoveAction_set(unit, phy_port, fwdAction);
    }
#else
    return RT_MAPPER(unit)->l2_staticPortMoveAction_set(unit, port, fwdAction);
#endif
} /* end of rtk_l2_staticPortMoveAction_set */

/* Module Name    : L2                        */
/* Sub-module Name: Parameter for lookup miss */

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMask_get
 * Description:
 *      Get flooding port mask which limits the lookup missed flooding domain.
 * Input:
 *      unit            - unit id
 *      type            - type of lookup miss
 * Output:
 *      pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) In 8390 & 8380, must invoke rtk_l2_lookupMissFloodPortMask_set_with_idx() first.
 *      (2) In 8390 & 8380, only DLF_TYPE_UCAST and DLF_TYPE_BCAST are supported. For DLF_TYPE_MCAST,
 *          DLF_TYPE_IPMC and DLF_TYPE_IP6MC, refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
rtk_l2_lookupMissFloodPortMask_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_portmask_t  temp_floodPortmask;

    if ((ret = RT_MAPPER(unit)->l2_lookupMissFloodPortMask_get(unit, type, &temp_floodPortmask)) != RT_ERR_OK)
        return ret;
    *pFlood_portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_floodPortmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_get(unit, type, pFlood_portmask);
#endif
} /* end of rtk_l2_lookupMissFloodPortMask_get */

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMask_set
 * Description:
 *      Set flooding port mask when unicast or multicast address lookup missed in L2 table.
 * Input:
 *      unit            - unit id
 *      type            - type of lookup miss
 *      pFlood_portmask - flooding port mask configuration when unicast/multicast lookup missed.
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
 *      8389
 * Note:
 *      None
 */
int32
rtk_l2_lookupMissFloodPortMask_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_portmask_t  temp_floodPortmask;

    temp_floodPortmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, *pFlood_portmask);
    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_set(unit, type, &temp_floodPortmask);
    }
#else
    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_set(unit, type, pFlood_portmask);
#endif
} /* end of rtk_l2_lookupMissFloodPortMask_set */

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMask_add
 * Description:
 *      Add one port member to the lookup missed flooding port mask.
 * Input:
 *      unit       - unit id
 *      type       - type of lookup miss
 *      flood_port - flooding port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) In 8390 & 8380, must invoke rtk_l2_lookupMissFloodPortMask_set_with_idx() first.
 *      (2) In 8390 & 8380, only DLF_TYPE_UCAST and DLF_TYPE_BCAST are supported. For DLF_TYPE_MCAST,
 *          DLF_TYPE_IPMC and DLF_TYPE_IP6MC, refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
rtk_l2_lookupMissFloodPortMask_add(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_flood_port = RTK_PORT_TO_PHYSICAL_PORT(unit, flood_port);

    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_add(unit, type, phy_flood_port);
    }
#else
    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_add(unit, type, flood_port);
#endif
} /* end of rtk_l2_lookupMissFloodPortMask_add */

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMask_del
 * Description:
 *      Delete one port member from the lookup missed flooding port mask.
 * Input:
 *      unit       - unit id
 *      type       - type of lookup miss
 *      flood_port - flooding port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 * Applicable:
 *      8389, 8390, 8380
 * Note:
 *      (1) In 8390 & 8380, must invoke rtk_l2_lookupMissFloodPortMask_set_with_idx() first.
 *      (2) In 8390 & 8380, only DLF_TYPE_UCAST and DLF_TYPE_BCAST are supported. For DLF_TYPE_MCAST,
 *          DLF_TYPE_IPMC and DLF_TYPE_IP6MC, refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
rtk_l2_lookupMissFloodPortMask_del(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_flood_port = RTK_PORT_TO_PHYSICAL_PORT(unit, flood_port);

    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_del(unit, type, phy_flood_port);
    }
#else
    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_del(unit, type, flood_port);
#endif
} /* end of rtk_l2_lookupMissFloodPortMask_del */

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMask_set_with_idx
 * Description:
 *      Set flooding port mask which limits the lookup missed flooding domain.
 * Input:
 *      unit            - unit id
 *      type            - type of lookup miss
 *      idx             - index to multicast portmask table
 *      pFlood_portmask - flooding port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_INPUT                    - invalid input parameter
 *      RT_ERR_L2_MCAST_FWD_ENTRY_EXIST - Mcast forwarding entry already exist
 * Applicable:
 *      8390, 8380
 * Note:
 *      In 8390 & 8380, only DLF_TYPE_UCAST and DLF_TYPE_BCAST are supported. For DLF_TYPE_MCAST,
 *      DLF_TYPE_IPMC and DLF_TYPE_IP6MC, refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
rtk_l2_lookupMissFloodPortMask_set_with_idx(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx, rtk_portmask_t *pFlood_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_portmask_t  temp_floodPortmask;

    temp_floodPortmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, *pFlood_portmask);
    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_set_with_idx(unit, type, idx, &temp_floodPortmask);
    }
#else
    return RT_MAPPER(unit)->l2_lookupMissFloodPortMask_set_with_idx(unit, type, idx, pFlood_portmask);
#endif
} /* end of rtk_l2_lookupMissFloodPortMask_set_with_idx */

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMaskIdx_get
 * Description:
 *      Get the entry index of forwarding table that is used as unicast/broadcast flooding port mask.
 * Input:
 *      unit 	- unit id
 *      type    - type of lookup miss
 * Output:
 *      pIdx	- flooding port mask configuration when unicast/multicast lookup missed.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      In 8380, 8390, flood-portmask get/set only supports DLF_TYPE_UCAST and DLF_TYPE_BCAST. For DLF_TYPE_MCAST,
 *      DLF_TYPE_IPMC and DLF_TYPE_IP6MC, please refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
rtk_l2_lookupMissFloodPortMaskIdx_get(uint32 unit, rtk_l2_lookupMissType_t type, uint32 *pIdx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissFloodPortMaskIdx_get(unit, type, pIdx);
}

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMaskIdx_set
 * Description:
 *      Set the entry index of forwarding table that is used as unicast/broadcast flooding port mask.
 * Input:
 *      unit    - unit id
 *      type    - type of lookup miss
 *      idx     - flooding port mask configuration when unicast/multicast lookup missed.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_L2_PMSK_NOT_INIT - flooding portmask is not initialized
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      In 8380, 8390, flood-portmask get/set only supports DLF_TYPE_UCAST and DLF_TYPE_BCAST. For DLF_TYPE_MCAST,
 *      DLF_TYPE_IPMC and DLF_TYPE_IP6MC, please refer to rtk_vlan_profile_set()/rtk_vlan_profile_get().
 */
int32
rtk_l2_lookupMissFloodPortMaskIdx_set(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissFloodPortMaskIdx_set(unit, type, idx);
}

/* Function Name:
 *      rtk_l2_lookupMissAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid type of lookup miss
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Type of lookup missis as following:
 *          - DLF_TYPE_IPMC
 *          - DLF_TYPE_UCAST
 *          - DLF_TYPE_BCAST
 *          - DLF_TYPE_MCAST
 *      (2) Forwarding action for 8328 is as following:
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_FLOOD_IN_VLAN
 *          - ACTION_FLOOD_IN_ALL_PORT  (only for DLF_TYPE_MCAST)
 *          - ACTION_FLOOD_IN_ROUTER_PORTS (only for DLF_TYPE_IPMC)
 
 *          Forwarding action for 8380/8390 is as following:
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_FLOOD_IN_VLAN
 *      (3) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 *          If have the per-port requirement, please call the rtk_l2_portLookupMissAction_get API.
 */
int32
rtk_l2_lookupMissAction_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissAction_get(unit, type, pAction);
} /* end of rtk_l2_lookupMissAction_get */

/* Function Name:
 *      rtk_l2_lookupMissAction_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_INPUT      - invalid type of lookup miss
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Type of lookup missis as following:
 *          - DLF_TYPE_IPMC
 *          - DLF_TYPE_IP6MC
 *          - DLF_TYPE_UCAST
 *          - DLF_TYPE_BCAST
 *          - DLF_TYPE_MCAST
 *      (2) Forwarding action for 8328 is as following:
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_FLOOD_IN_VLAN
 *          - ACTION_FLOOD_IN_ALL_PORT  (only for DLF_TYPE_MCAST)
 *          - ACTION_FLOOD_IN_ROUTER_PORTS (only for DLF_TYPE_IPMC)
 
 *          Forwarding action for 8380/8390 is as following:
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_FLOOD_IN_VLAN
 *      (3) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 *          If have the per-port requirement, please call the rtk_l2_portLookupMissAction_get API.
 */
int32
rtk_l2_lookupMissAction_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissAction_set(unit, type, action);
} /* end of rtk_l2_lookupMissAction_set */

/* Function Name:
 *      rtk_l2_portLookupMissAction_get
 * Description:
 *      Get forwarding action of specified port when destination address lookup miss.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      type    - type of lookup miss
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid type of lookup miss
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_IP6MC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_MCAST
 *
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
rtk_l2_portLookupMissAction_get(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portLookupMissAction_get(unit, phy_port, type, pAction);
    }
#else
    return RT_MAPPER(unit)->l2_portLookupMissAction_get(unit, port, type, pAction);
#endif
} /* end of rtk_l2_lookupMissAction_get */

/* Function Name:
 *      rtk_l2_portLookupMissAction_set
 * Description:
 *      Set forwarding action of specified port when destination address lookup miss.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      type   - type of lookup miss
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_INPUT      - invalid type of lookup miss
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8390, 8380
 * Note:
 *      Type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_IP6MC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_MCAST
 *
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
int32
rtk_l2_portLookupMissAction_set(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->l2_portLookupMissAction_set(unit, phy_port, type, action);
    }
#else
    return RT_MAPPER(unit)->l2_portLookupMissAction_set(unit, port, type, action);
#endif
} /* end of rtk_l2_lookupMissAction_set */

/* Function Name:
 *      rtk_l2_lookupMissPri_get
 * Description:
 *      Get priority of trapped packet when destination address lookup miss.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The priority apply to all type of lookup missis as following:
 *          - DLF_TYPE_IPMC
 *          - DLF_TYPE_UCAST
 *          - DLF_TYPE_BCAST
 *          - DLF_TYPE_MCAST
 *      (2) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_l2_lookupMissPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissPri_get(unit, pPriority);
} /* end of rtk_l2_lookupMissPri_get */

/* Function Name:
 *      rtk_l2_lookupMissPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The priority apply to all type of lookup missis as following:
 *          - DLF_TYPE_IPMC
 *          - DLF_TYPE_UCAST
 *          - DLF_TYPE_BCAST
 *          - DLF_TYPE_MCAST
 *      (2) For 8390, 8380, the function is backward compatible RTL8328 APIs.
 */
int32
rtk_l2_lookupMissPri_set(uint32 unit, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissPri_set(unit, priority);
} /* end of rtk_l2_lookupMissPri_set */

/* Function Name:
 *      rtk_l2_lookupMissPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
rtk_l2_lookupMissPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissPriEnable_get(unit, pEnable);
} /* end of rtk_l2_lookupMissPriEnable_get */

/* Function Name:
 *      rtk_l2_lookupMissPriEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
 int32
rtk_l2_lookupMissPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissPriEnable_set(unit, enable);
} /* end of rtk_l2_lookupMissPriEnable_set */

/* Function Name:
 *      rtk_l2_lookupMissDP_get
 * Description:
 *      Get drop precedence of trapped packet when destination address lookup miss.
 * Input:
 *      unit - unit id
 * Output:
 *      pDp  - pointer to drop precedence of trapped packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The DP apply to all type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
int32
rtk_l2_lookupMissDP_get(uint32 unit, uint32 *pDp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissDP_get(unit, pDp);
} /* end of rtk_l2_lookupMissDP_get */

/* Function Name:
 *      rtk_l2_lookupMissDP_set
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
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_DROP_PRECEDENCE - invalid drop precedence
 * Applicable:
 *      8328
 * Note:
 *      The DP apply to all type of lookup missis as following:
 *      - DLF_TYPE_IPMC
 *      - DLF_TYPE_UCAST
 *      - DLF_TYPE_BCAST
 *      - DLF_TYPE_MCAST
 */
int32
rtk_l2_lookupMissDP_set(uint32 unit, uint32 dp)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissDP_set(unit, dp);
} /* end of rtk_l2_lookupMissDP_set */

/* Function Name:
 *      rtk_l2_lookupMissDPEnable_get
 * Description:
 *      Get drop procedence assignment status for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to drop procedence assignment status for trapped packet
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
rtk_l2_lookupMissDPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissDPEnable_get(unit, pEnable);
} /* end of rtk_l2_lookupMissDPEnable_get */

/* Function Name:
 *      rtk_l2_lookupMissDPEnable_set
 * Description:
 *      Set drop procedence drop procedence assignment status for trapped packet.
 * Input:
 *      unit   - unit id
 *      enable - drop procedence assignment status for trapped packet
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
rtk_l2_lookupMissDPEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissDPEnable_set(unit, enable);
} /* end of rtk_l2_lookupMissDPEnable_set */

/* Function Name:
 *      rtk_l2_lookupMissAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
rtk_l2_lookupMissAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissAddCPUTagEnable_get(unit, pEnable);
} /* end of rtk_l2_lookupMissAddCPUTagEnable_get */

/* Function Name:
 *      rtk_l2_lookupMissAddCPUTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_lookupMissAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_lookupMissAddCPUTagEnable_set(unit, enable);
} /* end of rtk_l2_lookupMissAddCPUTagEnable_set */

/* Module Name    : L2                 */
/* Sub-module Name: Parameter for MISC */

/* Function Name:
 *      rtk_l2_srcPortEgrFilterMask_get
 * Description:
 *      Get loopback filtering function on specified ports.
 * Input:
 *      unit             - unit id
 * Output:
 *      pFilter_portmask - ports which turn on loopback filtering function
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
rtk_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_portmask_t  temp_filterPortmask;

    if ((ret = RT_MAPPER(unit)->l2_srcPortEgrFilterMask_get(unit, &temp_filterPortmask)) != RT_ERR_OK)
        return ret;
    *pFilter_portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_filterPortmask);
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_srcPortEgrFilterMask_get(unit, pFilter_portmask);
#endif
} /* end of rtk_l2_srcPortEgrFilterMask_get */

/* Function Name:
 *      rtk_l2_srcPortEgrFilterMask_set
 * Description:
 *      Set loopback filtering function on specified ports.
 * Input:
 *      unit             - unit id
 *      pFilter_portmask - ports which turn on loopback filtering function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
rtk_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFilter_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_portmask_t  temp_filterPortmask;

    temp_filterPortmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, *pFilter_portmask);
    return RT_MAPPER(unit)->l2_srcPortEgrFilterMask_set(unit, &temp_filterPortmask);
    }
#else
    return RT_MAPPER(unit)->l2_srcPortEgrFilterMask_set(unit, pFilter_portmask);
#endif
} /* end of rtk_l2_srcPortEgrFilterMask_set */

/* Function Name:
 *      rtk_l2_srcPortEgrFilterMask_add
 * Description:
 *      Enable the loopback filtering function on specified port.
 * Input:
 *      unit        - unit id
 *      filter_port - ports which turn on loopback filtering function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
rtk_l2_srcPortEgrFilterMask_add(uint32 unit, rtk_port_t filter_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_filter_port = RTK_PORT_TO_PHYSICAL_PORT(unit, filter_port);

    return RT_MAPPER(unit)->l2_srcPortEgrFilterMask_add(unit, phy_filter_port);
    }
#else
    return RT_MAPPER(unit)->l2_srcPortEgrFilterMask_add(unit, filter_port);
#endif
} /* end of rtk_l2_srcPortEgrFilterMask_add */

/* Function Name:
 *      rtk_l2_srcPortEgrFilterMask_del
 * Description:
 *      Disable the loopback filtering function on specified port.
 * Input:
 *      unit        - unit id
 *      filter_port - ports which turn off loopback filtering function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The loopback(egress port == ingress port) packet is dropped if the function is enabled.
 */
int32
rtk_l2_srcPortEgrFilterMask_del(uint32 unit, rtk_port_t filter_port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_filter_port = RTK_PORT_TO_PHYSICAL_PORT(unit, filter_port);

    return RT_MAPPER(unit)->l2_srcPortEgrFilterMask_del(unit, phy_filter_port);
    }
#else
    return RT_MAPPER(unit)->l2_srcPortEgrFilterMask_del(unit, filter_port);
#endif
} /* end of rtk_l2_srcPortEgrFilterMask_del */

/*
 * MISC
 */

/* Function Name:
 *      rtk_l2_exceptionAddrAction_get
 * Description:
 *      Get forwarding action of packet with exception source MAC address.
 * Input:
 *      unit       - unit id
 *      exceptType - type of exception address
 * Output:
 *      pAction    - pointer to forward action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE - invalid exception address type
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) For 8328, Exception address type is as following
 *          - SA_IS_MCAST
 *          - SA_IS_BCAST
 *          - SA_IS_ZERO
 *      (2) For 8390 and 8380, Exception address type is as following
 *          - SA_IS_BCAST_OR_MCAST
 *          - SA_IS_ZERO
 */
int32
rtk_l2_exceptionAddrAction_get(
    uint32                          unit,
    rtk_l2_exceptionAddrType_t      exceptType,
    rtk_action_t                    *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_exceptionAddrAction_get(unit, exceptType, pAction);
} /* end of rtk_l2_exceptionAddrAction_get */

/* Function Name:
 *      rtk_l2_exceptionAddrAction_set
 * Description:
 *      Set forwarding action of packet with exception source MAC address.
 * Input:
 *      unit       - unit id
 *      exceptType - type of exception address
 *      action     - forward action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_L2_EXCEPT_ADDR_TYPE - invalid exception address type
 *      RT_ERR_INPUT               - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) For 8328, Exception address type is as following
 *          - SA_IS_MCAST
 *          - SA_IS_BCAST
 *          - SA_IS_ZERO
 *      (2) For 8390 and 8380, Exception address type is as following
 *          - SA_IS_BCAST_OR_MCAST
 *          - SA_IS_ZERO
 */
int32
rtk_l2_exceptionAddrAction_set(
    uint32                          unit,
    rtk_l2_exceptionAddrType_t      exceptType,
    rtk_action_t                    action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_exceptionAddrAction_set(unit, exceptType, action);
} /* end of rtk_l2_exceptionAddrAction_set */

/* Function Name:
 *      rtk_l2_notificationEnable_get
 * Description:
 *      Get enable state of L2 notification mechanism.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_l2_notificationEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationEnable_get(unit, pEnable);
} /* end of rtk_l2_notificationEnable_get */

/* Function Name:
 *      rtk_l2_notificationEnable_set
 * Description:
 *      Set enable state of L2 notification mechanism.
 * Input:
 *      unit   - unit id
 *      enable - enable status
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
 *      None
 */
int32
rtk_l2_notificationEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationEnable_set(unit, enable);
} /* end of rtk_l2_notificationEnable_set */

/* Function Name:
 *      rtk_l2_notificationEventEnable_get
 * Description:
 *      Get L2 notification event enable configuration of the specified device.
 * Input:
 *      unit    - unit id
 *      event   - notification type
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8390 
 * Note:
 *      For 8390, only suspend learn and linkdown flush types support enable/disable state configuration
 */
int32
rtk_l2_notificationEventEnable_get(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t *pEnable)
{ 
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationEventEnable_get(unit, event, pEnable);
}/* end of rtk_l2_notificationEventEnable_get */

/* Function Name:
 *      rtk_l2_notificationEventEnable_set
 * Description:
 *      Set L2 notification event configuration of the specified device.
 * Input:
 *      unit    - unit id
 *      event   - notification type
 *      enable  - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 * Applicable:
 *      8390  
 * Note:
 *      For 8390, only suspend learn and linkdown flush types support enable/disable state configuration
 */
int32
rtk_l2_notificationEventEnable_set(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationEventEnable_set(unit, event, enable);
}/* end of rtk_l2_notificationEventEnable_set */

/* Function Name:
 *      rtk_l2_notificationBackPressureThresh_get
 * Description:
 *      Get L2 notification back-pressure threshold of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pThresh - threshold value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Back-pressure is a mechanism to avoid ASIC's notification FIFO running out
 *      (2) Back-pressure only suppresses the aging out event when the notification FIFO is over the
 *          specified threshold.
 */
int32
rtk_l2_notificationBackPressureThresh_get(uint32 unit, uint32 *pThresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationBackPressureThresh_get(unit, pThresh);
} /* end of rtk_l2_notificationBackPressureThresh_get */

/* Function Name:
 *      rtk_l2_notificationBackPressureThresh_set
 * Description:
 *      Set L2 notification back-pressure threshold of the specified device.
 * Input:
 *      unit    - unit id
 *      thresh  - threshold value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390
 * Note:
 *      (1) Back-pressure is a mechanism to avoid ASIC's notification FIFO running out
 *      (2) Back-pressure only suppresses the aging out event when the notification FIFO is over the
 *          specified threshold.
 */
int32
rtk_l2_notificationBackPressureThresh_set(uint32 unit, uint32 thresh)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationBackPressureThresh_set(unit, thresh);
} /* end of rtk_l2_notificationBackPressureThresh_set */

/* Function Name:
 *      rtk_l2_notificationFifoEmptyStatus_get
 * Description:
 *      Get FIFO empty status of L2 notification module.
 * Input:
 *      unit    - unit id
 * Output:
 *      pStatus - notification FIFO status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_l2_notificationFifoEmptyStatus_get(uint32 unit, rtk_l2_notifyFifoStatus_t *pStatus)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationFifoEmptyStatus_get(unit, pStatus);
} /* end of rtk_l2_notificationFifoEmptyStatus_get */

/* Function Name:
 *      rtk_l2_notificationEventHandler_register
 * Description:
 *      Register event handlers for L2 notification module.
 * Input:
 *      unit                  - unit id
 *      notification_callback - callback function for notification event
 *      bufferRunOut_callback - callback function for buffer run out event
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_l2_notificationEventHandler_register(
    uint32                                      unit,
    rtk_l2_notification_callback_t              notification_callback,
    rtk_l2_notification_bufferRunOut_callback_t bufferRunOut_callback)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationEventHandler_register(unit, notification_callback, bufferRunOut_callback);
} /* end of rtk_l2_notificationEventHandler_register */

/* Function Name:
 *      rtk_l2_notificationEventHandler_unregister
 * Description:
 *      Unregister event handlers for L2 notification module.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_l2_notificationEventHandler_unregister(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_notificationEventHandler_unregister(unit);
} /* end of rtk_l2_notificationEventHandler_unregister */

/* Function Name:
 *      rtk_l2_trapPri_get
 * Description:
 *      Get priority of trapped packet.
 * Input:
 *      unit      - unit id
 * Output:
 *      pPriority - pointer to priority
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
rtk_l2_trapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_trapPri_get(unit, pPriority);
} /* end of rtk_l2_trapPri_get */

/* Function Name:
 *      rtk_l2_trapPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_trapPri_set(uint32 unit, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_trapPri_set(unit, priority);
} /* end of rtk_l2_trapPri_set */

/* Function Name:
 *      rtk_l2_trapPriEnable_get
 * Description:
 *      Get priority assignment status for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to priority assignment status for trapped packet
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
rtk_l2_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_trapPriEnable_get(unit, pEnable);
} /* end of rtk_l2_trapPriEnable_get */

/* Function Name:
 *      rtk_l2_trapPriEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_trapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_trapPriEnable_set(unit, enable);
} /* end of rtk_l2_trapPriEnable_set */

/* Function Name:
 *      rtk_l2_trapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
rtk_l2_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_trapAddCPUTagEnable_get(unit, pEnable);
} /* end of rtk_l2_trapAddCPUTagEnable_get */

/* Function Name:
 *      rtk_l2_trapAddCPUTagEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_l2_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_trapAddCPUTagEnable_set(unit, enable);
} /* end of rtk_l2_trapAddCPUTagEnable_set */

/* Function Name:
 *      rtk_l2_addrEntry_get
 * Description:
 *      Get the L2 table entry by index of the specified unit.
 * Input:
 *      unit      - unit id
 *      index     - l2 table index
 * Output:
 *      pL2_entry - pointer buffer of l2 table entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The index valid range is from 0 to (L2 hash table size - 1)
 *          - 0 ~ (L2 hash table size - 1) entry in L2 hash table
 *      (2) The output entry have 2 variables (valid and entry_type) and its detail data structure
 *          - valid: 1 mean the entry is valid; 0: invalid
 *          - entry_type: FLOW_TYPE_UNICAST, FLOW_TYPE_L2_MULTI and FLOW_TYPE_IP_MULTI
 *                        the field is ignored if valid field is 0.
 *          - detail data structure is ignored if valid is 0, and its filed meanings is depended
 *            on the entry_type value.
 *      (3) If pL2_entry->flags have enabled the RTK_L2_UCAST_FLAG_TRUNK_PORT flag, mean the
 *          pL2_entry->unicast.trk_gid is valid trunk id value.
 */
int32
rtk_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32   ret;
    rtk_l2_entry_t  temp_l2Entry;

    if ((ret = RT_MAPPER(unit)->l2_addrEntry_get(unit, index, &temp_l2Entry)) != RT_ERR_OK)
        return ret;
    *pL2_entry = temp_l2Entry;
    switch (temp_l2Entry.entry_type)
    {
        case FLOW_TYPE_UNICAST:
            (*pL2_entry).unicast.port = PHYSICAL_PORT_TO_RTK_PORT(unit, temp_l2Entry.unicast.port);
            break;
        case FLOW_TYPE_L2_MULTI:
            (*pL2_entry).l2mcast.portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_l2Entry.l2mcast.portmask);
            break;
        case FLOW_TYPE_IP4_MULTI:
            (*pL2_entry).ipmcast.portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_l2Entry.ipmcast.portmask);
            break;
        case FLOW_TYPE_IP6_MULTI:
            (*pL2_entry).ip6mcast.portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_l2Entry.ip6mcast.portmask);
            break;
        default:
            return RT_ERR_FAILED;
    }
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_addrEntry_get(unit, index, pL2_entry);
#endif
} /* end of rtk_l2_addrEntry_get */

/* Function Name:
 *      rtk_l2_conflictAddr_get
 * Description:
 *      Get the conflict L2 table entry from one given L2 address in the specified unit.
 * Input:
 *      unit            - unit id
 *      pL2Addr         - l2 address to find its conflict entries
 *      cfAddrList_size - buffer size of the pCfAddrList
 * Output:
 *      pCfAddrList     - pointer buffer of the conflict l2 table entry list
 *      pCf_retCnt      - return number of find conflict l2 table entry list
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      (1) The function can be used if add l2 entry return RT_ERR_L2_NO_EMPTY_ENTRY.
 *          Input the pL2Addr->entry_type and its hash key to get conflict entry information.
 *      (2) User want to prepare the return buffer pCfAddrList and via. cfAddrList_size argument
 *          tell driver its size.
 *      (3) The function will return valid L2 hash entry from the same bucket and the return number
 *          is filled in pCf_retCnt, entry data is filled in pCfAddrList.
 */
int32
rtk_l2_conflictAddr_get(
    uint32          unit,
    rtk_l2_entry_t  *pL2Addr,
    rtk_l2_entry_t  *pCfAddrList,
    uint32          cfAddrList_size,
    uint32          *pCf_retCnt)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    uint32  i;
    int32   ret;
    rtk_l2_entry_t  temp_l2Entry;

    if ((ret = RT_MAPPER(unit)->l2_conflictAddr_get(unit, pL2Addr, pCfAddrList, cfAddrList_size, pCf_retCnt)) != RT_ERR_OK)
        return ret;
    for (i = 0; i < *pCf_retCnt; i++)
    {
        osal_memcpy(&temp_l2Entry, (pCfAddrList + i), sizeof(rtk_l2_entry_t));
        switch (temp_l2Entry.entry_type)
        {
            case FLOW_TYPE_UNICAST:
                (*(pCfAddrList + i)).unicast.port = PHYSICAL_PORT_TO_RTK_PORT(unit, temp_l2Entry.unicast.port);
                break;
            case FLOW_TYPE_L2_MULTI:
                (*(pCfAddrList + i)).l2mcast.portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_l2Entry.l2mcast.portmask);
                break;
            case FLOW_TYPE_IP4_MULTI:
            case FLOW_TYPE_IP6_MULTI:
                (*(pCfAddrList + i)).ipmcast.portmask = PHYSICAL_PORTMASK_TO_RTK_PORTMASK(unit, temp_l2Entry.ipmcast.portmask);
                break;
            default:
                return RT_ERR_FAILED;
        }
    }
    return ret;
    }
#else
    return RT_MAPPER(unit)->l2_conflictAddr_get(unit, pL2Addr, pCfAddrList, cfAddrList_size, pCf_retCnt);
#endif
} /* end of rtk_l2_conflictAddr_get */

/* Function Name:
 *      rtk_l2_zeroSALearningEnable_get
 * Description:
 *      Get enable status of all-zero-SA learning.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380, 8390
 * Note:
 *      None
 */
int32
rtk_l2_zeroSALearningEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_zeroSALearningEnable_get(unit, pEnable);
} /* end of rtk_l2_zeroSALearningEnable_get */

/* Function Name:
 *      rtk_l2_zeroSALearningEnable_set
 * Description:
 *      Set enable status of all-zero-SA learning.
 * Input:
 *      unit   - unit id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380, 8390
 * Note:
 *      None
 */
int32
rtk_l2_zeroSALearningEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_zeroSALearningEnable_set(unit, enable);
} /* end of rtk_l2_zeroSALearningEnable_set */

/* Function Name:
 *      rtk_l2_secureMacMode_get
 * Description:
 *      Get enable status of secure source MAC address mode.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380, 8390
 * Note:
 *      In secure SA mode, SA lookup missed packet should be dropped. Refer to rtk_l2_newMacOp_set to
 *      drop the SA lookup missed packet.
 */
int32
rtk_l2_secureMacMode_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_secureMacMode_get(unit, pEnable);
} /* end of rtk_l2_secureMacMode_get */

/* Function Name:
 *      rtk_l2_secureMacMode_set
 * Description:
 *      Set enable status of secure source MAC address mode.
 * Input:
 *      unit   - unit id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380, 8390
 * Note:
 *      In secure SA mode, SA lookup missed packet should be dropped. Refer to rtk_l2_newMacOp_set to
 *      drop the SA lookup missed packet.
 */
int32
rtk_l2_secureMacMode_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_secureMacMode_set(unit, enable);
} /* end of rtk_l2_secureMacMode_set */

/* Function Name:
 *      rtk_l2_portDynamicPortMoveForbidEnable_get
 * Description:
 *      Get the port moveforbiddance configuration of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status
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
 *      (1) Dynamic address entry port move in/out forbiddance enabled port is not allowed and the entry
 *          is not updated.
 *      (2) Regarding the forwrding action, refer to rtk_l2_dynamicPortMoveForbidAction_set.
 */
int32
rtk_l2_portDynamicPortMoveForbidEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_portDynamicPortMoveForbidEnable_get(unit, port, pEnable);
} /* end of rtk_l2_portDynamicPortMoveForbidEnable_get */

/* Function Name:
 *      rtk_l2_portDynamicPortMoveForbidEnable_set
 * Description:
 *      Set the port move forbiddance configuration of the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
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
 *      (1) Dynamic address entry port move in/out forbiddance enabled port is not allowed and the entry
 *          is not updated.
 *      (2) Regarding the forwrding action, refer to rtk_l2_dynamicPortMoveForbidAction_set.
 */
int32
rtk_l2_portDynamicPortMoveForbidEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_portDynamicPortMoveForbidEnable_set(unit, port, enable);
} /* end of rtk_l2_portDynamicPortMoveForbidEnable_set */

/* Function Name:
 *      rtk_l2_dynamicPortMoveForbidAction_get
 * Description:
 *      Get the forwarding action when the port moving is detected on port move forbiddance port.
 * Input:
 *      unit    - unit id
 * Output:
 *      pAction - pointer buffer of forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The action is taken either a dynamic address entry moved out from a port move forbiddance port
 *      or to a port move forbiddance port.
 */
int32
rtk_l2_dynamicPortMoveForbidAction_get(uint32 unit, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_dynamicPortMoveForbidAction_get(unit, pAction);
} /* end of rtk_l2_dynamicPortMoveForbidAction_get */

/* Function Name:
 *      rtk_l2_dynamicPortMoveForbidAction_set
 * Description:
 *      Set the forwarding action when the port moving is detected on port move forbiddance port.
 * Input:
 *      unit   - unit id
 *      action - forwarding action
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
 *      The action is taken either a dynamic address entry moved out from a port move forbiddance port
 *      or to a port move forbiddance port.
 */
int32
rtk_l2_dynamicPortMoveForbidAction_set(uint32 unit, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_dynamicPortMoveForbidAction_set(unit, action);
} /* end of rtk_l2_dynamicPortMoveForbidAction_set */

/* Function Name:
 *      rtk_l2_portMacFilterEnable_get
 * Description:
 *      Get the mac filter configuration of the specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      filterMode  - filter DA or SA
 * Output:
 *      pEnable     - enable status
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
 *      None
 */
int32
rtk_l2_portMacFilterEnable_get(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_portMacFilterEnable_get(unit, port, filterMode, pEnable);
}

/* Function Name:
 *      rtk_l2_portMacFilterEnable_set
 * Description:
 *      Set the mac filter configuration of the specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      filterMode  - filter DA or SA
 *      enable      - drop procedence assignment status for trapped packet
 * Output:
 *      None
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
 *      None
 */
int32
rtk_l2_portMacFilterEnable_set(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_portMacFilterEnable_set(unit, port, filterMode, enable);
}

/* Function Name:
 *      rtk_l2_hwNextValidAddr_get
 * Description:
 *      get next valid entry with specific method.
 * Input:
 *      unit        - unit id
 *      pScan_idx   - the index which starting search from 
 *      rtk_l2_nextValidType_t  - search Method
 * Output:
 *      pScan_idx           - the next valid entry index
 *      rtk_l2_entry_t      - the next valid entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_l2_hwNextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_nextValidType_t type,
    rtk_l2_entry_t  *pEntry)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->l2_hwNextValidAddr_get(unit, pScan_idx, type, pEntry);
}
