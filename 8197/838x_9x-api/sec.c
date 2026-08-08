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
 * $Revision: 28152 $
 * $Date: 2012-04-19 17:38:56 +0800 (Thu, 19 Apr 2012) $
 *
 * Purpose : Definition those public security APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Attack prevention
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/sec.h>

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

/* Module Name : Security */

/* Function Name:
 *      rtk_sec_init
 * Description:
 *      Initialize security module of the specified device.
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
 *      Must initialize security module before calling any sec APIs.
 */
int32
rtk_sec_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_init(unit);
} /* end of rtk_sec_init */

/* Module Name    : Security          */
/* Sub-module Name: Attack prevention */

/* Function Name:
 *      rtk_sec_portAttackPrevent_get
 * Description:
 *      Get action for each kind of attack on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      attack_type - type of attack
 * Output:
 *      pAction     - pointer to action for attack
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Supported type of attack is as following:
 *          - rtk_sec_attackType_t \ Chip:  8328    8390    8380
 *          - GRATUITOUS_ARP                X       O       O
 *          - ARP_INVALID                   X       O       O
 *          - TCP_FRAG_OFF_MIN_CHECK        X       X       X
 *          - SYNRST_DENY                   X       X       X
 *          - SYNFIN_DENY                   O       X       X
 *          - XMA_DENY                      O       X       X
 *          - NULLSCAN_DENY                 O       X       X
 *          - SYN_SPORTL1024_DENY           O       X       X
 *          - TCPHDR_MIN_CHECK              O       X       X
 *          - SMURF_DENY                    O       X       X
 *          - ICMPV6_PING_MAX_CHECK         O       X       X
 *          - ICMPV4_PING_MAX_CHECK         O       X       X
 *          - ICMP_FRAG_PKTS_DENY           O       X       X
 *          - IPV6_MIN_FRAG_SIZE_CHECK      O       X       X
 *          - POD_DENY                      O       X       X
 *          - TCPBLAT_DENY                  O       X       X
 *          - UDPBLAT_DENY                  O       X       X
 *          - LAND_DENY                     O       X       X
 *          - DAEQSA_DENY                   O       X       X
 *      (2) Supported action is as following:
 *          - rtk_action_t \ Chip:          8328    8390    8380
 *          - ACTION_FORWARD                O       O       O
 *          - ACTION_DROP                   O       O       O
 *          - ACTION_TRAP2CPU               X       O       O
 *          - ACTION_COPY                   X       O       O
 *      (3) The action - ACTION_COPY is for GRATUITOUS_ARP only.
 */
int32
rtk_sec_portAttackPrevent_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->sec_portAttackPrevent_get(unit, phy_port, attack_type, pAction);
    }
#else
    return RT_MAPPER(unit)->sec_portAttackPrevent_get(unit, port, attack_type, pAction);
#endif
} /* end of rtk_sec_portAttackPrevent_get */

/* Function Name:
 *      rtk_sec_portAttackPrevent_set
 * Description:
 *      Set action for each kind of attack on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      attack_type - type of attack
 *      action      - action for attack
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 *      RT_ERR_INPUT      - invalid input parameter
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Supported type of attack is as following:
 *          - rtk_sec_attackType_t \ Chip:  8328    8390    8380
 *          - GRATUITOUS_ARP                X       O       O
 *          - ARP_INVALID                   X       O       O
 *          - TCP_FRAG_OFF_MIN_CHECK        X       X       X
 *          - SYNRST_DENY                   X       X       X
 *          - SYNFIN_DENY                   O       X       X
 *          - XMA_DENY                      O       X       X
 *          - NULLSCAN_DENY                 O       X       X
 *          - SYN_SPORTL1024_DENY           O       X       X
 *          - TCPHDR_MIN_CHECK              O       X       X
 *          - SMURF_DENY                    O       X       X
 *          - ICMPV6_PING_MAX_CHECK         O       X       X
 *          - ICMPV4_PING_MAX_CHECK         O       X       X
 *          - ICMP_FRAG_PKTS_DENY           O       X       X
 *          - IPV6_MIN_FRAG_SIZE_CHECK      O       X       X
 *          - POD_DENY                      O       X       X
 *          - TCPBLAT_DENY                  O       X       X
 *          - UDPBLAT_DENY                  O       X       X
 *          - LAND_DENY                     O       X       X
 *          - DAEQSA_DENY                   O       X       X
 *      (2) Supported action is as following:
 *          - rtk_action_t \ Chip:          8328    8390    8380
 *          - ACTION_FORWARD                O       O       O
 *          - ACTION_DROP                   O       O       O
 *          - ACTION_TRAP2CPU               X       O       O
 *          - ACTION_COPY                   X       O       O
 *      (3) The action - ACTION_COPY is for GRATUITOUS_ARP only.
 */
int32
rtk_sec_portAttackPrevent_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->sec_portAttackPrevent_set(unit, phy_port, attack_type, action);
    }
#else
    return RT_MAPPER(unit)->sec_portAttackPrevent_set(unit, port, attack_type, action);
#endif
} /* end of rtk_sec_portAttackPrevent_set */

/* Function Name:
 *      rtk_sec_portMinIPv6FragLen_get
 * Description:
 *      Get minimum length of IPv6 fragments on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to minimum length of IPv6 fragments
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
 *      The valid range of length is 0~0xFFFF
 */
int32
rtk_sec_portMinIPv6FragLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portMinIPv6FragLen_get(unit, port, pLength);
} /* end of rtk_sec_portMinIPv6FragLen_get */

/* Function Name:
 *      rtk_sec_portMinIPv6FragLen_set
 * Description:
 *      Set minimum length of IPv6 fragments on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - minimum length of IPv6 fragments
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *      The valid range of length is 0~0xFFFF
 */
int32
rtk_sec_portMinIPv6FragLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portMinIPv6FragLen_set(unit, port, length);
} /* end of rtk_sec_portMinIPv6FragLen_set */

/* Function Name:
 *      rtk_sec_portMaxPingLen_get
 * Description:
 *      Get maximum length of ICMP packet on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to maximum length of ICMP packet
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
 *      The valid range of length is 0~0xFFFF
 */
int32
rtk_sec_portMaxPingLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portMaxPingLen_get(unit, port, pLength);
} /* end of rtk_sec_portMaxPingLen_get */

/* Function Name:
 *      rtk_sec_portMaxPingLen_set
 * Description:
 *      Set maximum length of ICMP packet on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - maximum length of ICMP packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *      The valid range of length is 0~0xFFFF
 */
int32
rtk_sec_portMaxPingLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portMaxPingLen_set(unit, port, length);
} /* end of rtk_sec_portMaxPingLen_set */

/* Function Name:
 *      rtk_sec_portMinTCPHdrLen_get
 * Description:
 *      Get minimum length of TCP header on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to minimum length of TCP header
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
 *      The valid range of length is 0~0xFF
 */
int32
rtk_sec_portMinTCPHdrLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portMinTCPHdrLen_get(unit, port, pLength);
} /* end of rtk_sec_portMinTCPHdrLen_get */

/* Function Name:
 *      rtk_sec_portMinTCPHdrLen_set
 * Description:
 *      Set minimum length of TCP header on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - minimum length of TCP header
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *      The valid range of length is 0~0xFF
 */
int32
rtk_sec_portMinTCPHdrLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portMinTCPHdrLen_set(unit, port, length);
} /* end of rtk_sec_portMinTCPHdrLen_set */

/* Function Name:
 *      rtk_sec_portSmurfNetmaskLen_get
 * Description:
 *      Get netmask length for preventing SMURF attack on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to netmask length
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
 *      The valid range of length is 0~31
 */
int32
rtk_sec_portSmurfNetmaskLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portSmurfNetmaskLen_get(unit, port, pLength);
} /* end of rtk_sec_portSmurfNetmaskLen_get */

/* Function Name:
 *      rtk_sec_portSmurfNetmaskLen_set
 * Description:
 *      Set netmask length for preventing SMURF attack on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      length - netmask length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *      The valid range of length is 0~31
 */
int32
rtk_sec_portSmurfNetmaskLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_portSmurfNetmaskLen_set(unit, port, length);
} /* end of rtk_sec_portSmurfNetmaskLen_set */

/* Function Name:
 *      rtk_sec_portAttackPreventEnable_get
 * Description:
 *      Get the attack prevention status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the status of the attack prevention
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
 *      Attack Prevention function is per-port enabled, use 'rtk_sec_attackPreventAction_set' to
 *      configure the action for each attack prevention type.
 */
int32
rtk_sec_portAttackPreventEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->sec_portAttackPreventEnable_get(unit, phy_port, pEnable);
    }
#else
    return RT_MAPPER(unit)->sec_portAttackPreventEnable_get(unit, port, pEnable);
#endif
} /* end of rtk_sec_portAttackPreventEnable_get */

/* Function Name:
 *      rtk_sec_portAttackPreventEnable_set
 * Description:
 *      Set the attack prevention status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - pointer to the status of the attack prevention
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
 *      Attack Prevention function is per-port enabled, use 'rtk_sec_attackPreventAction_set' to
 *      configure the action for each attack prevention type.
 */
int32
rtk_sec_portAttackPreventEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->sec_portAttackPreventEnable_set(unit, phy_port, enable);
    }
#else
    return RT_MAPPER(unit)->sec_portAttackPreventEnable_set(unit, port, enable);
#endif
} /* end of rtk_sec_portAttackPreventEnable_set */

/* Function Name:
 *      rtk_sec_attackPreventAction_get
 * Description:
 *      Get action for each kind of attack.
 * Input:
 *      unit        - unit id
 *      attack_type - type of attack
 * Output:
 *      pAction     - pointer to action for attack
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
 *      Type of attack is as following:
 *      - TCP_FRAG_OFF_MIN_CHECK
 *      - SYNRST_DENY
 *      - SYNFIN_DENY
 *      - XMA_DENY
 *      - NULLSCAN_DENY
 *      - SYN_SPORTL1024_DENY
 *      - TCPHDR_MIN_CHECK
 *      - SMURF_DENY
 *      - ICMPV6_PING_MAX_CHECK
 *      - ICMPV4_PING_MAX_CHECK
 *      - ICMP_FRAG_PKTS_DENY
 *      - IPV6_MIN_FRAG_SIZE_CHECK
 *      - POD_DENY
 *      - TCPBLAT_DENY
 *      - UDPBLAT_DENY
 *      - LAND_DENY
 *      - DAEQSA_DENY
 *
 *      Action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_sec_attackPreventAction_get(
    uint32                      unit,
    rtk_sec_attackType_t        attack_type,
    rtk_action_t                *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_attackPreventAction_get(unit, attack_type, pAction);
} /* end of rtk_sec_attackPreventAction_get */

/* Function Name:
 *      rtk_sec_attackPreventAction_set
 * Description:
 *      Set action for each kind of attack.
 * Input:
 *      unit        - unit id
 *      attack_type - type of attack
 *      action      - action for attack
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_INPUT      - invalid input parameter
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8390, 8380
 * Note:
 *      Type of attack is as following:
 *      - TCP_FRAG_OFF_MIN_CHECK
 *      - SYNRST_DENY
 *      - SYNFIN_DENY
 *      - XMA_DENY
 *      - NULLSCAN_DENY
 *      - SYN_SPORTL1024_DENY
 *      - TCPHDR_MIN_CHECK
 *      - SMURF_DENY
 *      - ICMPV6_PING_MAX_CHECK
 *      - ICMPV4_PING_MAX_CHECK
 *      - ICMP_FRAG_PKTS_DENY
 *      - IPV6_MIN_FRAG_SIZE_CHECK
 *      - POD_DENY
 *      - TCPBLAT_DENY
 *      - UDPBLAT_DENY
 *      - LAND_DENY
 *      - DAEQSA_DENY
 *
 *      Action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_sec_attackPreventAction_set(
    uint32                  unit,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_attackPreventAction_set(unit, attack_type, action);
} /* end of rtk_sec_attackPreventAction_set */

/* Function Name:
 *      rtk_sec_minIPv6FragLen_get
 * Description:
 *      Get minimum length of IPv6 fragments.
 * Input:
 *      unit    - unit id
 * Output:
 *      pLength - pointer to minimum length of IPv6 fragments
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
rtk_sec_minIPv6FragLen_get(uint32 unit, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_minIPv6FragLen_get(unit, pLength);
} /* end of rtk_sec_minIPv6FragLen_get */

/* Function Name:
 *      rtk_sec_minIPv6FragLen_set
 * Description:
 *      Set minimum length of IPv6 fragments.
 * Input:
 *      unit   - unit id
 *      length - minimum length of IPv6 fragments
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_sec_minIPv6FragLen_set(uint32 unit, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_minIPv6FragLen_set(unit, length);
} /* end of rtk_sec_minIPv6FragLen_set */

/* Function Name:
 *      rtk_sec_maxPingLen_get
 * Description:
 *      Get maximum length of ICMP packet.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pLength - pointer to maximum length of ICMP packet
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
rtk_sec_maxPingLen_get(uint32 unit, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_maxPingLen_get(unit, pLength);
} /* end of rtk_sec_maxPingLen_get */

/* Function Name:
 *      rtk_sec_maxPingLen_set
 * Description:
 *      Set maximum length of ICMP packet.
 * Input:
 *      unit   - unit id
 *      length - maximum length of ICMP packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_sec_maxPingLen_set(uint32 unit, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_maxPingLen_set(unit, length);
} /* end of rtk_sec_maxPingLen_set */

/* Function Name:
 *      rtk_sec_minTCPHdrLen_get
 * Description:
 *      Get minimum length of TCP header.
 * Input:
 *      unit    - unit id
 * Output:
 *      pLength - pointer to minimum length of TCP header
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
rtk_sec_minTCPHdrLen_get(uint32 unit, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_minTCPHdrLen_get(unit, pLength);
} /* end of rtk_sec_minTCPHdrLen_get */

/* Function Name:
 *      rtk_sec_minTCPHdrLen_set
 * Description:
 *      Set minimum length of TCP header.
 * Input:
 *      unit   - unit id
 *      length - minimum length of TCP header
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_sec_minTCPHdrLen_set(uint32 unit, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_minTCPHdrLen_set(unit, length);
} /* end of rtk_sec_minTCPHdrLen_set */

/* Function Name:
 *      rtk_sec_smurfNetmaskLen_get
 * Description:
 *      Get netmask length for preventing SMURF attack.
 * Input:
 *      unit    - unit id
 * Output:
 *      pLength - pointer to netmask length
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
rtk_sec_smurfNetmaskLen_get(uint32 unit, uint32 *pLength)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_smurfNetmaskLen_get(unit, pLength);
} /* end of rtk_sec_smurfNetmaskLen_get */

/* Function Name:
 *      rtk_sec_smurfNetmaskLen_set
 * Description:
 *      Set netmask length for preventing SMURF attack.
 * Input:
 *      unit   - unit id
 *      length - netmask length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_sec_smurfNetmaskLen_set(uint32 unit, uint32 length)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->sec_smurfNetmaskLen_set(unit, length);
} /* end of rtk_sec_smurfNetmaskLen_set */
