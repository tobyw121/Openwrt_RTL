/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 16212 $
 * $Date: 2011-03-10 10:39:02 +0800 (Thu, 10 Mar 2011) $
 *
 * Purpose : Definition those public Trap APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Configuration for traping packet to CPU
 *           2) RMA
 *           3) User defined RMA
 *           4) System-wise management frame
 *           5) System-wise user defined management frame
 *           6) Per port user defined management frame
 *           7) Packet with special flag or option
 *           8) CFM and OAM packet
 */

#ifndef __DAL_ESW_TRAP_H__
#define __DAL_ESW_TRAP_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/trap.h>


/*
 * Symbol Definition
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_esw_trap_init
 * Description:
 *      Initial the trap module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32
dal_esw_trap_init(uint32 unit);

/* Module Name    : Trap                                    */
/* Sub-module Name: Configuration for traping packet to CPU */

/* Module Name    : Trap     */
/* Sub-module Name: RMA      */

/* Function Name:
 *      dal_esw_trap_rmaAction_get
 * Description:
 *      Get action of reserved multicast address(RMA) frame.
 * Input:
 *      unit        - unit id
 *      pRma_frame  - Reserved multicast address.
 * Output:
 *      pRma_action - RMA action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_esw_trap_rmaAction_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action);

/* Function Name:
 *      dal_esw_trap_rmaAction_set
 * Description:
 *      Set action of reserved multicast address(RMA) frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      rma_action - RMA action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - Invalid unit id
 *      RT_ERR_INPUT      - Invalid input parameter
 *      RT_ERR_RMA_ACTION - Invalid RMA action
 * Note:
 *      The supported Reserved Multicast Address frame:
 *      Assignment                                                                  Address
 *      RMA_BRG_GROUP (Bridge Group Address)                                        01-80-C2-00-00-00
 *      RMA_FD_PAUSE (IEEE Std 802.3, 1988 Edition, Full Duplex PAUSE operation)    01-80-C2-00-00-01
 *      RMA_SP_MCAST (IEEE Std 802.3ad Slow Protocols-Multicast address)            01-80-C2-00-00-02
 *      RMA_1X_PAE (IEEE Std 802.1X PAE address)                                    01-80-C2-00-00-03
 *      RMA_RESERVED04 (Reserved)                                                   01-80-C2-00-00-04
 *      RMA_MEDIA_ACCESS_USE (Media Access Method Specific Use)                     01-80-C2-00-00-05
 *      RMA_RESERVED06 (Reserved)                                                   01-80-C2-00-00-06
 *      RMA_RESERVED07 (Reserved)                                                   01-80-C2-00-00-07
 *      RMA_PVD_BRG_GROUP (Provider Bridge Group Address)                           01-80-C2-00-00-08
 *      RMA_RESERVED09 (Reserved)                                                   01-80-C2-00-00-09
 *      RMA_RESERVED0A (Reserved)                                                   01-80-C2-00-00-0A
 *      RMA_RESERVED0B (Reserved)                                                   01-80-C2-00-00-0B
 *      RMA_RESERVED0C (Reserved)                                                   01-80-C2-00-00-0C
 *      RMA_MVRP (Provider Bridge MVRP Address)                                     01-80-C2-00-00-0D
 *      RMA_1ab_LL_DISCOVERY (802.1ab Link Layer Discover Protocol Address)         01-80-C2-00-00-0E
 *      RMA_RESERVED0F (Reserved)                                                   01-80-C2-00-00-0F
 *      RMA_BRG_MNGEMENT (All LANs Bridge Management Group Address)                 01-80-C2-00-00-10
 *      RMA_LOAD_SERV_GENERIC_ADDR (Load Server Generic Address)                    01-80-C2-00-00-11
 *      RMA_LOAD_DEV_GENERIC_ADDR (Loadable Device Generic Address)                 01-80-C2-00-00-12
 *      RMA_RESERVED13 (Reserved)                                                   01-80-C2-00-00-13
 *      RMA_RESERVED14 (Reserved)                                                   01-80-C2-00-00-14
 *      RMA_RESERVED15 (Reserved)                                                   01-80-C2-00-00-15
 *      RMA_RESERVED16 (Reserved)                                                   01-80-C2-00-00-16
 *      RMA_RESERVED17 (Reserved)                                                   01-80-C2-00-00-17
 *      RMA_MANAGER_STA_GENERIC_ADDR (Generic Address for All Manager Stations)     01-80-C2-00-00-18
 *      RMA_RESERVED19 (Reserved)                                                   01-80-C2-00-00-19
 *      RMA_AGENT_STA_GENERIC_ADDR (Generic Address for All Agent Stations)         01-80-C2-00-00-1A
 *      RMA_RESERVED1B (Reserved)                                                   01-80-C2-00-00-1B
 *      RMA_RESERVED1C (Reserved)                                                   01-80-C2-00-00-1C
 *      RMA_RESERVED1D (Reserved)                                                   01-80-C2-00-00-1D
 *      RMA_RESERVED1E (Reserved)                                                   01-80-C2-00-00-1E
 *      RMA_RESERVED1F (Reserved)                                                   01-80-C2-00-00-1F
 *      RMA_GMRP (GMRP Address)                                                     01-80-C2-00-00-20
 *      RMA_GVRP (GVRP address)                                                     01-80-C2-00-00-21
 *      RMA_UNDEF_GARP22~2F (Undefined GARP address)                                01-80-C2-00-00-22
 *                                                                                ~ 01-80-C2-00-00-2F
 *
 *      The supported Reserved Multicast Address action:
 *      -   RMA_ACTION_FORWARD
 *      -   RMA_ACTION_DROP
 *      -   RMA_ACTION_TRAP2CPU
 */
extern int32
dal_esw_trap_rmaAction_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action);

/* Function Name:
 *      dal_esw_trap_rmaPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pPriority  - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_rmaPri_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_rmaPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      priority   - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_rmaPri_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_rmaPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pEnable    - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
extern int32
dal_esw_trap_rmaPriEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_rmaPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      enable     - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_PRIORITY         - invalid priority value
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 * Note:
 *      None
 */
extern int32
dal_esw_trap_rmaPriEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_rmaCpuTagAddEnable_get
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
dal_esw_trap_rmaCpuTagAddEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_rmaCpuTagAddEnable_set
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
dal_esw_trap_rmaCpuTagAddEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_rmaVlanCheckEnable_get
 * Description:
 *      Get enable status of vlan checking on specified RMA frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pEnable    - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_rmaVlanCheckEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_rmaVlanCheckEnable_set
 * Description:
 *      Set enable status of vlan checking on specified RMA frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      enable     - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_ADDR         - invalid invalid RMA address
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_rmaVlanCheckEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable);

/* Module Name    : Trap     */
/* Sub-module Name: User defined RMA */

/* Function Name:
 *      dal_esw_trap_userDefineRma_get
 * Description:
 *      Get user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pUserDefinedRma - pointer to content of user defined RMA
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRma_get(
    uint32                      unit, 
    uint32                      userDefine_idx, 
    rtk_trap_userDefinedRma_t   *pUserDefinedRma);

/* Function Name:
 *      dal_esw_trap_userDefineRma_set
 * Description:
 *      Set user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      pUserDefinedRma - to content of user defined RMA
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRma_set(
    uint32                      unit, 
    uint32                      userDefine_idx, 
    rtk_trap_userDefinedRma_t   *pUserDefinedRma);

/* Function Name:
 *      dal_esw_trap_userDefineRmaAction_get
 * Description:
 *      Get forwarding action of user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pActoin         - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
extern int32
dal_esw_trap_userDefineRmaAction_get(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t *pAction);

/* Function Name:
 *      dal_esw_trap_userDefineRmaAction_set
 * Description:
 *      Set forwarding action of user defined RMA.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      actoin          - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
extern int32
dal_esw_trap_userDefineRmaAction_set(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t action);

/* Function Name:
 *      dal_esw_trap_userDefineRmaPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pPriority       - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaPri_get(uint32 unit, uint32 userDefine_idx, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_userDefineRmaPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      priority        - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaPri_set(uint32 unit, uint32 userDefine_idx, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_userDefineRmaPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pEnable         - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaPriEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_userDefineRmaPriEnable_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      enable          - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaPriEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_userDefineRmaVlanCheckEnable_get
 * Description:
 *      Get enable status of vlan checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pEnable         - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaVlanCheckEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_userDefineRmaVlanCheckEnable_set
 * Description:
 *      Set enable status of vlan checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      enable          - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaVlanCheckEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_userDefineRmaStpBlockEnable_get
 * Description:
 *      Get enable status of STP status checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 * Output:
 *      pEnable         - pointer to enable status of STP status checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaStpBlockEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_userDefineRmaStpBlockEnable_set
 * Description:
 *      Set enable status of STP status checking on specified user defined RMA frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined RMA entry
 *      enable          - enable status of STP status checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineRmaStpBlockEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable);

/* Module Name    : Trap                         */
/* Sub-module Name: System-wise management frame */

/* Function Name:
 *      dal_esw_trap_mgmtFrameAction_get
 * Description:
 *      Get forwarding action of management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pActoin   - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_RIP
 *      - MGMT_TYPE_ICMP
 *      - MGMT_TYPE_ICMPV6
 *      - MGMT_TYPE_ARP
 *      - MGMT_TYPE_MLD
 *      - MGMT_TYPE_IGMP
 *      - MGMT_TYPE_BGP
 *      - MGMT_TYPE_OSPFV2
 *      - MGMT_TYPE_OSPFV3
 *      - MGMT_TYPE_SNMP
 *      - MGMT_TYPE_SSH
 *      - MGMT_TYPE_FTP
 *      - MGMT_TYPE_TFTP
 *      - MGMT_TYPE_TELNET
 *      - MGMT_TYPE_HTTP
 *      - MGMT_TYPE_HTTPS
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_trap_mgmtFrameAction_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction);

/* Function Name:
 *      dal_esw_trap_mgmtFrameAction_set
 * Description:
 *      Set forwarding action of management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      actoin    - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_RIP
 *      - MGMT_TYPE_ICMP
 *      - MGMT_TYPE_ICMPV6
 *      - MGMT_TYPE_ARP
 *      - MGMT_TYPE_MLD
 *      - MGMT_TYPE_IGMP
 *      - MGMT_TYPE_BGP
 *      - MGMT_TYPE_OSPFV2
 *      - MGMT_TYPE_OSPFV3
 *      - MGMT_TYPE_SNMP
 *      - MGMT_TYPE_SSH
 *      - MGMT_TYPE_FTP
 *      - MGMT_TYPE_TFTP
 *      - MGMT_TYPE_TELNET
 *      - MGMT_TYPE_HTTP
 *      - MGMT_TYPE_HTTPS
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_trap_mgmtFrameAction_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t action);

/* Function Name:
 *      dal_esw_trap_mgmtFramePri_get
 * Description:
 *      Get priority of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_mgmtFramePri_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_mgmtFramePri_set
 * Description:
 *      Set priority of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      priority  - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_mgmtFramePri_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_mgmtFramePriEnable_get
 * Description:
 *      Get priority enable status of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_mgmtFramePriEnable_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_mgmtFramePriEnable_set
 * Description:
 *      Set priority enable status of trapped packet.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      enable    - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_mgmtFramePriEnable_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_mgmtFrameVlanCheck_get
 * Description:
 *      Get enable status of vlan checking on management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_mgmtFrameVlanCheck_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_mgmtFrameVlanCheck_set
 * Description:
 *      Set enable status of vlan checking on management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      enable    - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_mgmtFrameVlanCheck_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable);

/* Module Name    : Trap                                      */
/* Sub-module Name: System-wise user defined management frame */

/* Function Name:
 *      dal_esw_trap_userDefineMgmt_get
 * Description:
 *      Get user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 * Output:
 *      pUserDefine     - pointer to user defined management frame
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmt_get(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine);

/* Function Name:
 *      dal_esw_trap_userDefineMgmt_set
 * Description:
 *      Set user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      pUserDefine     - user defined management frame
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmt_set(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine);

/* Function Name:
 *      dal_esw_trap_userDefineMgmtAction_get
 * Description:
 *      Get forwarding action of user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 * Output:
 *      pActoin         - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
extern int32
dal_esw_trap_userDefineMgmtAction_get(uint32 unit, uint32 mgmt_idx, rtk_action_t *pAction);

/* Function Name:
 *      rtk_trap_userDefineMgmtAction_set
 * Description:
 *      Set forwarding action of user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      actoin          - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 */
extern int32
dal_esw_trap_userDefineMgmtAction_set(uint32 unit, uint32 mgmt_idx, rtk_action_t action);

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 * Output:
 *      pPriority       - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmtPri_get(uint32 unit, uint32 mgmt_idx, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      priority        - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmtPri_set(uint32 unit, uint32 mgmt_idx, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined management frame entry
 * Output:
 *      pEnable        - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmtPriEnable_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_userDefineMgmtPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined management frame entry
 *      enable         - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmtPriEnable_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_userDefineMgmtVlanCheck_get
 * Description:
 *      Get enable status of vlan checking on specified user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      enable          - enable status of vlan checking
 * Output:
 *      pEnable         - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmtVlanCheck_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_userDefineMgmtVlanCheck_set
 * Description:
 *      Set enable status of vlan checking on specified user defined management frame.
 * Input:
 *      unit            - unit id
 *      userDefine_idx  - index of user defined management frame entry
 *      enable          - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - userDefine_idx is out of range
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_userDefineMgmtVlanCheck_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable);

/* Module Name    : Trap                                   */
/* Sub-module Name: Per port user defined management frame */

/* Function Name:
 *      dal_esw_trap_portMgmtFrameAction_get
 * Description:
 *      Get forwarding action of management frame on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pActoin   - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_trap_portMgmtFrameAction_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction);

/* Function Name:
 *      dal_esw_trap_portMgmtFrameAction_set
 * Description:
 *      Set forwarding action of management frame on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      actoin    - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 * Note:
 *      Type of management frame is as following:
 *      - MGMT_TYPE_DHCPV6
 *      - MGMT_TYPE_DHCP
 *      - MGMT_TYPE_DOT1X
 *      - MGMT_TYPE_BPDU
 * 
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 *      - ACTION_DROP
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_trap_portMgmtFrameAction_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t action);

/* Function Name:
 *      dal_esw_trap_portMgmtFramePri_get
 * Description:
 *      Get priority of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFramePri_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_portMgmtFramePri_set
 * Description:
 *      Set priority of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      priority  - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFramePri_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_portMgmtFramePriEnable_get
 * Description:
 *      Get priority enable status of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFramePriEnable_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_portMgmtFramePriEnable_set
 * Description:
 *      Set priority enable status of trapped packet on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      enable    - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFramePriEnable_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_portMgmtFrameVlanCheck_get
 * Description:
 *      Get enable status of vlan checking on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFrameVlanCheck_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_portMgmtFrameVlanCheck_set
 * Description:
 *      Set enable status of vlan checking on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      enable    - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFrameVlanCheck_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_portMgmtFrameCrossVlan_get
 * Description:
 *      Get enable status of cross vlan forwarding on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to enable status of cross vlan forwarding
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFrameCrossVlan_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_esw_trap_portMgmtFrameCrossVlan_set
 * Description:
 *      Set enable status of cross vlan forwarding on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 *      frameType - type of management frame
 *      enable    - enable status of cross vlan forwarding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE    - invalid type of management frame
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern int32
dal_esw_trap_portMgmtFrameCrossVlan_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable);

/* Module Name    : Trap                               */
/* Sub-module Name: Packet with special flag or option */

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAction_get
 * Description:
 *      Get forwarding action of IP packet with IP option header.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      ipFamily - IP family(IPv4 or IPv6)
 * Output:
 *      pActoin  - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_esw_trap_ipWithOptionHeaderAction_get(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_ip_family_t     ipFamily, 
    rtk_action_t        *pAction);

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAction_set
 * Description:
 *      Set forwarding action of IP packet with IP option header.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      ipFamily - IP family(IPv4 or IPv6)
 *      actoin   - forwarding action
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
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_esw_trap_ipWithOptionHeaderAction_set(
    uint32              unit, 
    rtk_port_t          port, 
    rtk_ip_family_t     ipFamily, 
    rtk_action_t        action);

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPriority - pointer to priority
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
dal_esw_trap_ipWithOptionHeaderPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_ipWithOptionHeaderPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to priority enable status
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
dal_esw_trap_ipWithOptionHeaderPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_ipWithOptionHeaderPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of CPU tag adding
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
dal_esw_trap_ipWithOptionHeaderAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_pktWithCFIAction_get
 * Description:
 *      Get forwarding action of ethernet packet with CFI set.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pActoin - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_FWD_ACTION       - invalid forwarding action
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_esw_trap_pktWithCFIAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction);

/* Function Name:
 *      dal_esw_trap_pktWithCFIAction_set
 * Description:
 *      Set forwarding action of ethernet packet with CFI set.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      actoin - forwarding action
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
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
extern int32
dal_esw_trap_pktWithCFIAction_set(uint32 unit, rtk_port_t port, rtk_action_t action);

/* Function Name:
 *      dal_esw_trap_pktWithCFIPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPriority - pointer to priority
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
dal_esw_trap_pktWithCFIPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_pktWithCFIPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_pktWithCFIPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_pktWithCFIPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to priority enable status
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
dal_esw_trap_pktWithCFIPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_pktWithCFIPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_pktWithCFIPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_pktWithCFIAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_trap_pktWithCFIAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_pktWithCFIAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped packet.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of CPU tag adding
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
dal_esw_trap_pktWithCFIAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Module Name    : Trap       */
/* Sub-module Name: CFM and OAM packet */
/* Function Name:
 *      dal_esw_trap_cfmFrameAction_get
 * Description:
 *      Get forwarding action of CFM frame on specified MD level.
 * Input:
 *      unit    - unit id
 *      level   - MD level
 * Output:
 *      pAction - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_trap_cfmFrameAction_get(uint32 unit, uint32 level, rtk_action_t *pAction);

/* Function Name:
 *      dal_esw_trap_cfmFrameAction_set
 * Description:
 *      Set forwarding action of CFM frame on specified MD level.
 * Input:
 *      unit   - unit id
 *      level  - MD level
 *      action - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 *      - ACTION_COPY2CPU
 */
extern int32
dal_esw_trap_cfmFrameAction_set(uint32 unit, uint32 level, rtk_action_t action);

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPri_get
 * Description:
 *      Get priority of CFM packets trapped to CPU.
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
dal_esw_trap_cfmFrameTrapPri_get(uint32 unit, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPri_set
 * Description:
 *      Set priority of CFM packets trapped to CPU.
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
dal_esw_trap_cfmFrameTrapPri_set(uint32 unit, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPriEnable_get
 * Description:
 *      Get priority enable status of CFM packets trapped to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to priority enable status
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
dal_esw_trap_cfmFrameTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapPriEnable_set
 * Description:
 *      Set priority enable status of CFM packets trapped to CPU.
 * Input:
 *      unit   - unit id
 *      enable - priority enable status
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
dal_esw_trap_cfmFrameTrapPriEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapAddCPUTagEnable_get
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
dal_esw_trap_cfmFrameTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_cfmFrameTrapAddCPUTagEnable_set
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
dal_esw_trap_cfmFrameTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_oamPDUAction_get
 * Description:
 *      Get forwarding action of trapped oam PDU on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer to forwarding action of trapped oam PDU
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
dal_esw_trap_oamPDUAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction);

/* Function Name:
 *      dal_esw_trap_oamPDUAction_set
 * Description:
 *      Set forwarding action of trapped oam PDU on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - forwarding action of trapped oam PDU
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
dal_esw_trap_oamPDUAction_set(uint32 unit, rtk_port_t port, rtk_action_t action);

/* Function Name:
 *      dal_esw_trap_oamPDUPri_get
 * Description:
 *      Get priority of trapped OAM PDU on specified port.
 * Input:
 *      unit      - unit id
 *      port      - port id
 * Output:
 *      pPriority - pointer to priority
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
dal_esw_trap_oamPDUPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_esw_trap_oamPDUPri_set
 * Description:
 *      Set priority of trapped OAM PDU on specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_oamPDUPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority);

/* Function Name:
 *      dal_esw_trap_oamPDUPriEnable_get
 * Description:
 *      Get priority enable status of trapped OAM PDU on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to priority enable status
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
dal_esw_trap_oamPDUPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_oamPDUPriEnable_set
 * Description:
 *      Set priority enable status of trapped OAM PDU on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_PRIORITY         - invalid priority value
 * Note:
 *      None
 */
extern int32
dal_esw_trap_oamPDUPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_oamPDUTrapAddCPUTagEnable_get
 * Description:
 *      Get enable status of CPU tag adding for trapped OAM PDU.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of CPU tag adding
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
dal_esw_trap_oamPDUTrapAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_oamPDUTrapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped OAM PDU.
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
dal_esw_trap_oamPDUTrapAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_esw_trap_mgmtIpCheck_get
 * Description:
 *      Get enable status of management ip type.
 * Input:
 *      unit   - unit id
 *      type   - type of management ip
 * Output:
 *      pEnable - pointer to enable status of management ip type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      Type of management ip type is as following:
 *      - MGMT_IP_TYPE_IPV4
 *      - MGMT_IP_TYPE_IPV6
 */
extern int32
dal_esw_trap_mgmtIpCheck_get(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_esw_trap_mgmtIpCheck_set
 * Description:
 *      Set enable status of management ip type.
 * Input:
 *      unit   - unit id
 *      type   - type of management ip
 *      enable - enable status of management ip type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      Type of management ip type is as following:
 *      - MGMT_IP_TYPE_IPV4
 *      - MGMT_IP_TYPE_IPV6
 */
extern int32
dal_esw_trap_mgmtIpCheck_set(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t enable);

#endif /* __DAL_ESW_TRAP_H__ */
