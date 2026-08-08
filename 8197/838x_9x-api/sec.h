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
 * $Revision: 29538 $
 * $Date: 2012-06-05 09:35:19 +0800 (Tue, 05 Jun 2012) $
 *
 * Purpose : Definition those public security APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Attack prevention
 */

#ifndef __RTK_SEC_H__
#define __RTK_SEC_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
/* Type of attack prevention */
typedef enum rtk_sec_attackType_e
{
    DAEQSA_DENY = 0,
    LAND_DENY,
    UDPBLAT_DENY,
    TCPBLAT_DENY,
    POD_DENY,
    IPV6_MIN_FRAG_SIZE_CHECK,
    ICMP_FRAG_PKTS_DENY,
    ICMPV4_PING_MAX_CHECK,
    ICMPV6_PING_MAX_CHECK,
    SMURF_DENY,
    TCPHDR_MIN_CHECK,
    SYN_SPORTL1024_DENY,
    NULLSCAN_DENY,
    XMA_DENY,
    SYNFIN_DENY,
    SYNRST_DENY,
    TCP_FRAG_OFF_MIN_CHECK,     /* Applicable to 8380, 8390 */
    ARP_INVALID,                /* Applicable to 8380, 8390 */
    GRATUITOUS_ARP,             /* Applicable to 8380, 8390 */
    ATTACK_TYPE_END
} rtk_sec_attackType_t;

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
extern int32
rtk_sec_init(uint32 unit);

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
extern int32
rtk_sec_portAttackPrevent_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            *pAction);

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
 *      (3) ACTION_COPY is only supported by GRATUITOUS_ARP in 8380.
 */
extern int32
rtk_sec_portAttackPrevent_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action);

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
extern int32
rtk_sec_portMinIPv6FragLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

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
extern int32
rtk_sec_portMinIPv6FragLen_set(uint32 unit, rtk_port_t port, uint32 length);

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
extern int32
rtk_sec_portMaxPingLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

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
extern int32
rtk_sec_portMaxPingLen_set(uint32 unit, rtk_port_t port, uint32 length);

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
extern int32
rtk_sec_portMinTCPHdrLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

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
extern int32
rtk_sec_portMinTCPHdrLen_set(uint32 unit, rtk_port_t port, uint32 length);

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
extern int32
rtk_sec_portSmurfNetmaskLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

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
extern int32
rtk_sec_portSmurfNetmaskLen_set(uint32 unit, rtk_port_t port, uint32 length);

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
extern int32
rtk_sec_portAttackPreventEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32
rtk_sec_portAttackPreventEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

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
extern int32
rtk_sec_attackPreventAction_get(
    uint32                      unit,
    rtk_sec_attackType_t        attack_type,
    rtk_action_t                *pAction);

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
extern int32
rtk_sec_attackPreventAction_set(
    uint32                  unit,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action);

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
extern int32
rtk_sec_minIPv6FragLen_get(uint32 unit, uint32 *pLength);

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
extern int32
rtk_sec_minIPv6FragLen_set(uint32 unit, uint32 length);

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
extern int32
rtk_sec_maxPingLen_get(uint32 unit, uint32 *pLength);

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
extern int32
rtk_sec_maxPingLen_set(uint32 unit, uint32 length);

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
extern int32
rtk_sec_minTCPHdrLen_get(uint32 unit, uint32 *pLength);

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
extern int32
rtk_sec_minTCPHdrLen_set(uint32 unit, uint32 length);

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
extern int32
rtk_sec_smurfNetmaskLen_get(uint32 unit, uint32 *pLength);

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
extern int32
rtk_sec_smurfNetmaskLen_set(uint32 unit, uint32 length);

#endif /* __RTK_SEC_H__ */
