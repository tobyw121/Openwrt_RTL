/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 21847 $
 * $Date: 2011-09-01 20:51:09 +0800 (Thu, 01 Sep 2011) $
 *
 * Purpose : Definition those public security APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Attack prevention
 */

#ifndef __DAL_ESW_SEC_H__
#define __DAL_ESW_SEC_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
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
 *      dal_esw_sec_init
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
 * Note:
 *      Must initialize security module before calling any sec APIs.
 */
extern int32
dal_esw_sec_init(uint32 unit);

/* Module Name    : Security          */
/* Sub-module Name: Attack prevention */

/* Function Name:
 *      dal_esw_sec_portAttackPrevent_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Type of attack is as following:
 *      - SYNFIN_DENY
 *      - XMA_DENY
 *      - XMA_DENY
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
 */
extern int32
dal_esw_sec_portAttackPrevent_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            *pAction);

/* Function Name:
 *      dal_esw_sec_portAttackPrevent_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Type of attack is as following:
 *      - SYNFIN_DENY
 *      - XMA_DENY
 *      - XMA_DENY
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
 */
extern int32
dal_esw_sec_portAttackPrevent_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_sec_attackType_t    attack_type,
    rtk_action_t            action);

/* Function Name:
 *      dal_esw_sec_portMinIPv6FragLen_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portMinIPv6FragLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

/* Function Name:
 *      dal_esw_sec_portMinIPv6FragLen_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portMinIPv6FragLen_set(uint32 unit, rtk_port_t port, uint32 length);

/* Function Name:
 *      dal_esw_sec_portMaxPingLen_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portMaxPingLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

/* Function Name:
 *      dal_esw_sec_portMaxPingLen_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portMaxPingLen_set(uint32 unit, rtk_port_t port, uint32 length);

/* Function Name:
 *      dal_esw_sec_portMinTCPHdrLen_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portMinTCPHdrLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

/* Function Name:
 *      dal_esw_sec_portMinTCPHdrLen_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portMinTCPHdrLen_set(uint32 unit, rtk_port_t port, uint32 length);

/* Function Name:
 *      dal_esw_sec_portSmurfNetmaskLen_get
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portSmurfNetmaskLen_get(uint32 unit, rtk_port_t port, uint32 *pLength);

/* Function Name:
 *      dal_esw_sec_portSmurfNetmaskLen_set
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
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      None
 */
extern int32
dal_esw_sec_portSmurfNetmaskLen_set(uint32 unit, rtk_port_t port, uint32 length);

#endif /* __DAL_ESW_SEC_H__ */

