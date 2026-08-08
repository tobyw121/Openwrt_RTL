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
 * $Revision: 34510 $
 * $Date: 2012-11-21 21:39:22 +0800 (Wed, 21 Nov 2012) $
 *
 * Purpose : Definition of TRAP API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration for traping packet to CPU
 *           (2) RMA
 *           (3) User defined RMA
 *           (4) System-wise management frame
 *           (5) System-wise user defined management frame
 *           (6) Per port user defined management frame
 *           (7) Packet with special flag or option
 *           (8) CFM and OAM packet
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/trap.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_trap_init
 * Description:
 *      Initial the trap module of the specified device..
 * Input:
 *      unit - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize trap module before calling any trap APIs.
 */
int32
rtk_trap_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_init(unit);
} /* end of rtk_trap_init */

/* Module Name    : Trap                                    */
/* Sub-module Name: Configuration for traping packet to CPU */

/* Function Name:
 *      rtk_trap_1xMacChangePort2CpuEnable_get
 * Description:
 *      Get the configuration about when 802.1x MAC-based authenticated MAC address changes port
 *      whether it need be trapped to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status of authenticated MAC address change port trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The status of authenticated MAC address change port trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_1xMacChangePort2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_1xMacChangePort2CpuEnable_get(unit, pEnable);
} /* end of rtk_trap_1xMacChangePort2CpuEnable_get */

/* Function Name:
 *      rtk_trap_1xMacChangePort2CpuEnable_set
 * Description:
 *      Set the configuration about when 802.1x MAC-based authenticated MAC address changes port
 *      whether it need be trapped to CPU.
 * Input:
 *      unit   - unit id
 *      enable - status of authenticated MAC address change port trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      The status of authenticated MAC address change port trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_1xMacChangePort2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_1xMacChangePort2CpuEnable_set(unit, enable);
} /* end of rtk_trap_1xMacChangePort2CpuEnable_set */

/* Function Name:
 *      rtk_trap_igmpCtrlPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether IGMP control packets need be trapped to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status of IGMP control packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The status of IGMP control packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_igmpCtrlPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_igmpCtrlPkt2CpuEnable_get(unit, pEnable);
} /* end of rtk_trap_igmpCtrlPkt2CpuEnable_get */

/* Function Name:
 *      rtk_trap_igmpCtrlPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether IGMP control packets need be trapped to CPU.
 * Input:
 *      unit   - unit id
 *      enable - status of IGMP control packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      The status of IGMP control packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_igmpCtrlPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_igmpCtrlPkt2CpuEnable_set(unit, enable);
} /* end of rtk_trap_igmpCtrlPkt2CpuEnable_set */

/* Function Name:
 *      rtk_trap_l2McastPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether L2 multicast packets lookup miss need be trapped to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status of L2 multicast packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The status of L2 multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_l2McastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_l2McastPkt2CpuEnable_get(unit, pEnable);
} /* end of rtk_trap_l2McastPkt2CpuEnable_get */

/* Function Name:
 *      rtk_trap_l2McastPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether L2 multicast packets lookup miss need be trapped to CPU.
 * Input:
 *      unit   - unit id
 *      enable - status of L2 multicast packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      The status of L2 multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_l2McastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_l2McastPkt2CpuEnable_set(unit, enable);
} /* end of rtk_trap_l2McastPkt2CpuEnable_set */

/* Function Name:
 *      rtk_trap_ipMcastPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether IP multicast packet lookup miss need be trapped to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - status of IP multicast packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      The status of IP multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_ipMcastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipMcastPkt2CpuEnable_get(unit, pEnable);
} /* end of rtk_trap_ipMcastPkt2CpuEnable_get */

/* Function Name:
 *      rtk_trap_ipMcastPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether IP multicast packet lookup miss need be trapped to CPU.
 * Input:
 *      unit   - unit id
 *      enable - status of IP multicast packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      The status of IP multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
int32
rtk_trap_ipMcastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipMcastPkt2CpuEnable_set(unit, enable);
} /* end of rtk_trap_ipMcastPkt2CpuEnable_set */

/* Function Name:
 *      rtk_trap_reasonTrapToCPUPriority_get
 * Description:
 *      Get priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      unit      - unit id
 *      type      - reason that trap to CPU port.
 * Output:
 *      pPriority - configured internal priority for such reason.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      - TRAP_REASON_RMA
 *      - TRAP_REASON_IPV4IGMP
 *      - TRAP_REASON_IPV6MLD
 *      - TRAP_REASON_1XEAPOL
 *      - TRAP_REASON_VLANERR
 *      - TRAP_REASON_SLPCHANGE
 *      - TRAP_REASON_MULTICASTDLF
 *      - TRAP_REASON_CFI
 *      - TRAP_REASON_1XUNAUTH
 */
int32
rtk_trap_reasonTrapToCPUPriority_get(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_reasonTrapToCPUPriority_get(unit, type, pPriority);
} /* end of rtk_trap_reasonTrapToCPUPriority_get */

/* Function Name:
 *      rtk_trap_reasonTrapToCPUPriority_set
 * Description:
 *      Set priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      unit     - unit id
 *      type     - reason that trap to CPU port.
 *      priority - internal priority that is going to be set for specific trap reason.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8389
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      - TRAP_REASON_RMA
 *      - TRAP_REASON_IPV4IGMP
 *      - TRAP_REASON_IPV6MLD
 *      - TRAP_REASON_1XEAPOL
 *      - TRAP_REASON_VLANERR
 *      - TRAP_REASON_SLPCHANGE
 *      - TRAP_REASON_MULTICASTDLF
 *      - TRAP_REASON_CFI
 *      - TRAP_REASON_1XUNAUTH
 */
int32
rtk_trap_reasonTrapToCPUPriority_set(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_reasonTrapToCPUPriority_set(unit, type, priority);
} /* end of rtk_trap_reasonTrapToCPUPriority_set */

/* Function Name:
 *      rtk_trap_pkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether specific packet type needed to be trapped to CPU.
 * Input:
 *      unit    - unit id
 *      type    - packet type that trapped to CPU port.
 * Output:
 *      pEnable - status of special packet type trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      Currently the trap packet type that supported are listed as follows:
 *      - TRAP_TYPE_1XMAC_PORTCHG
 *      - TRAP_TYPE_IPV4_IGMP
 *      - TRAP_TYPE_IPMC_DLF
 *      - TRAP_TYPE_L2MC_DLF
 *      - TRAP_TYPE_IPV6_MLD
 *      - TRAP_TYPE_CFI_1
 */
int32
rtk_trap_pkt2CpuEnable_get(uint32 unit, rtk_trap_type_t type, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pkt2CpuEnable_get(unit, type, pEnable);
} /* end of rtk_trap_pkt2CpuEnable_get */

/* Function Name:
 *      rtk_trap_pkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether specific packet type needed to be trapped to CPU.
 * Input:
 *      unit   - unit id
 *      type   - packet type that trapped to CPU port.
 *      enable - status of specific packet type trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      Currently the trap packet type that supported are listed as follows:
 *      - TRAP_TYPE_1XMAC_PORTCHG
 *      - TRAP_TYPE_IPV4_IGMP
 *      - TRAP_TYPE_IPMC_DLF
 *      - TRAP_TYPE_L2MC_DLF
 *      - TRAP_TYPE_IPV6_MLD
 *      - TRAP_TYPE_CFI_1
 */
int32
rtk_trap_pkt2CpuEnable_set(uint32 unit, rtk_trap_type_t type, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pkt2CpuEnable_set(unit, type, enable);
} /* end of rtk_trap_pkt2CpuEnable_set */

/* Module Name    : Trap     */
/* Sub-module Name: RMA      */

/* Function Name:
 *      rtk_trap_rmaAction_get
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
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid RMA address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390
 * Note:
 *      None
 */
int32
rtk_trap_rmaAction_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaAction_get(unit, pRma_frame, pRma_action);
} /* end of rtk_trap_rmaAction_get */

/* Function Name:
 *      rtk_trap_rmaAction_set
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
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_RMA_ADDR   - invalid RMA address
 *      RT_ERR_RMA_ACTION - Invalid RMA action
 * Applicable:
 *      8389, 8328, 8390
 * Note:
 *      (1) The supported Reserved Multicast Address frame:
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
 *      (2) The supported Reserved Multicast Address action:
 *          - RMA_ACTION_FORWARD
 *          - RMA_ACTION_DROP
 *          - RMA_ACTION_TRAP2CPU
 *          - ACTION_COPY2CPU (for 8328 only)
 */
int32
rtk_trap_rmaAction_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaAction_set(unit, pRma_frame, rma_action);
} /* end of rtk_trap_rmaAction_set */

/* Function Name:
 *      rtk_trap_rmaGroupAction_get
 * Description:
 *      Get action of reserved multicast address(RMA) frame in group.
 * Input:
 *      unit               - unit id
 *      rmaGroup_frameType - Reserved multicast address type.
 * Output:
 *      pRma_action        - pointer buffer of RMA action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      The supported Reserved Multicast Address action:
 *      -   RMA_ACTION_FORWARD
 *      -   RMA_ACTION_DROP
 *      -   RMA_ACTION_TRAP2CPU
 *      -   RMA_ACTION_FLOOD_IN_ALL_PORT
 */
int32
rtk_trap_rmaGroupAction_get(uint32 unit, rtk_trap_rmaGroup_frameType_t rmaGroup_frameType, rtk_trap_rma_action_t *pRma_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaGroupAction_get(unit, rmaGroup_frameType, pRma_action);
} /* end of rtk_trap_rmaGroupAction_get */

/* Function Name:
 *      rtk_trap_rmaGroupAction_set
 * Description:
 *      Set action of reserved multicast address(RMA) frame in group.
 * Input:
 *      unit               - unit id
 *      rmaGroup_frameType - Reserved multicast address type.
 *      rma_action         - RMA action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - Invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_INPUT      - Invalid input parameter
 *      RT_ERR_RMA_ACTION - Invalid RMA action
 * Applicable:
 *      8380
 * Note:
 *      (1) The supported Reserved Multicast Address frame:
 *          - Assignment                 Address
 *          - RMA_SLOW_PROTOCOL_OAM      01-80-C2-00-00-02 & ethertype=0x8809 & subtype = 0x3
 *          - RMA_SLOW_PROTOCOL_OTHER    01-80-C2-00-00-02 except OAM
 *          - RMA_0E_EXCEPT_PTP_LLDP     01-80-C2-00-00-0E
 *          - RMA_GMRP (GMRP Address)    01-80-C2-00-00-20
 *          - RMA_GVRP (GVRP address)    01-80-C2-00-00-21
 *          - RMA_MSRP                   01-80-C2-00-00-22
 *          - RMA_0X                     01-80-C2-00-00-01~01-80-C2-00-00-0F, excluding listed above
 *          - RMA_1X                     01-80-C2-00-00-01~10-80-C2-00-00-1F, excluding listed above
 *          - RMA_2X                     01-80-C2-00-00-01~23-80-C2-00-00-2F, excluding listed above
 *      (2) The supported Reserved Multicast Address action:
 *          - RMA_ACTION_FORWARD
 *          - RMA_ACTION_DROP
 *          - RMA_ACTION_TRAP2CPU
 *          - RMA_ACTION_FLOOD_IN_ALL_PORT
 */
int32
rtk_trap_rmaGroupAction_set(uint32 unit, rtk_trap_rmaGroup_frameType_t rmaGroup_frameType, rtk_trap_rma_action_t rma_action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaGroupAction_set(unit, rmaGroup_frameType, rma_action);
} /* end of rtk_trap_rmaGroupAction_set */

/* Function Name:
 *      rtk_trap_rmaPri_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_rmaPri_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaPri_get(unit, pRma_frame, pPriority);
} /* end of rtk_trap_rmaPri_get */

/* Function Name:
 *      rtk_trap_rmaPri_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_PRIORITY     - invalid priority value
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_rmaPri_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaPri_set(unit, pRma_frame, priority);
} /* end of rtk_trap_rmaPri_set */

/* Function Name:
 *      rtk_trap_rmaPriEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_rmaPriEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaPriEnable_get(unit, pRma_frame, pEnable);
} /* end of rtk_trap_rmaPriEnable_get */

/* Function Name:
 *      rtk_trap_rmaPriEnable_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_PRIORITY     - invalid priority value
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_rmaPriEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaPriEnable_set(unit, pRma_frame, enable);
} /* end of rtk_trap_rmaPriEnable_set */

/* Function Name:
 *      rtk_trap_rmaCpuTagAddEnable_get
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
rtk_trap_rmaCpuTagAddEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaCpuTagAddEnable_get(unit, pEnable);
} /* end of rtk_trap_rmaCpuTagAddEnable_get */

/* Function Name:
 *      rtk_trap_rmaCpuTagAddEnable_set
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
rtk_trap_rmaCpuTagAddEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaCpuTagAddEnable_set(unit, enable);
} /* end of rtk_trap_rmaCpuTagAddEnable_set */

/* Function Name:
 *      rtk_trap_rmaVlanCheckEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_rmaVlanCheckEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaVlanCheckEnable_get(unit, pRma_frame, pEnable);
} /* end of rtk_trap_rmaVlanCheckEnable_get */

/* Function Name:
 *      rtk_trap_rmaVlanCheckEnable_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_rmaVlanCheckEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaVlanCheckEnable_set(unit, pRma_frame, enable);
} /* end of rtk_trap_rmaVlanCheckEnable_set */

/* Function Name:
 *      rtk_trap_rmaLearningEnable_get
 * Description:
 *      Get enable status of SMAC learning for the RMA frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pEnable    - pointer to enable status of SMAC learning
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_trap_rmaLearningEnable_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaLearningEnable_get(unit, pRma_frame, pEnable);
} /* end of rtk_trap_rmaLearningEnable_get */

/* Function Name:
 *      rtk_trap_rmaLearningEnable_set
 * Description:
 *      Set enable status of SMAC learning for the RMA frame.
 * Input:
 *      unit       - unit id
 *      pRma_frame - Reserved multicast address.
 *      enable     - enable status of SMAC learning
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      None
 */
int32
rtk_trap_rmaLearningEnable_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaLearningEnable_set(unit, pRma_frame, enable);
} /* end of rtk_trap_rmaLearningEnable_set */

/* Function Name:
 *      rtk_trap_rmaGroupLearningEnable_get
 * Description:
 *      Get enable status of SMAC learning for this RMA frame.
 * Input:
 *      unit               - unit id
 *      rmaGroup_frameType - Reserved multicast address type.
 * Output:
 *      pEnable            - pointer to enable status of SMAC learning
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_RMA_ADDR     - invalid invalid RMA address
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_trap_rmaGroupLearningEnable_get(uint32 unit, rtk_trap_rmaGroup_frameType_t rmaGroup_frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaGroupLearningEnable_get(unit, rmaGroup_frameType, pEnable);
} /* end of rtk_trap_rmaGroupLearningEnable_get */

/* Function Name:
 *      rtk_trap_rmaGroupLearningEnable_set
 * Description:
 *      Set enable status of SMAC learning for this RMA frame.
 * Input:
 *      unit               - unit id
 *      rmaGroup_frameType - Reserved multicast address type.
 *      enable             - enable status of SMAC learning
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_RMA_ADDR - invalid invalid RMA address
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_trap_rmaGroupLearningEnable_set(uint32 unit, rtk_trap_rmaGroup_frameType_t  rmaGroup_frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_rmaGroupLearningEnable_set(unit, rmaGroup_frameType, enable);
} /* end of rtk_trap_rmaGroupLearningEnable_set */

/* Function Name:
 *      rtk_trap_bypassStp_get
 * Description:
 *      Get enable status of bypassing spanning tree ingress filtering for specified frame type
 * Input:
 *      unit      - unit id
 *      frameType - frame type
 * Output:
 *      pEnable   - pointer to enable status of bypassing STP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The frame type selection is as following:
 *      - BYPASS_STP_TYPE_USER_DEF_0
 *      - BYPASS_STP_TYPE_USER_DEF_1
 *      - BYPASS_STP_TYPE_USER_DEF_2 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_USER_DEF_3 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_USER_DEF_4 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_USER_DEF_5 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_RMA_0X
 *      - BYPASS_STP_TYPE_SLOW_PROTO
 *      - BYPASS_STP_TYPE_EAPOL
 *      - BYPASS_STP_TYPE_PTP
 *      - BYPASS_STP_TYPE_LLDP
 */
int32
rtk_trap_bypassStp_get(uint32 unit, rtk_trap_bypassStpType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_bypassStp_get(unit, frameType, pEnable);
} /* end of rtk_trap_bypassStp_get */

/* Function Name:
 *      rtk_trap_bypassStp_set
 * Description:
 *      Set enable status of bypassing spanning tree ingress filtering for specified frame type
 * Input:
 *      unit      - unit id
 *      frameType - frame type
 *      enable    - enable status of bypassing STP
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
 *      The frame type selection is as following:
 *      - BYPASS_STP_TYPE_USER_DEF_0
 *      - BYPASS_STP_TYPE_USER_DEF_1
 *      - BYPASS_STP_TYPE_USER_DEF_2 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_USER_DEF_3 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_USER_DEF_4 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_USER_DEF_5 (Applicable to 8380)
 *      - BYPASS_STP_TYPE_RMA_0X
 *      - BYPASS_STP_TYPE_SLOW_PROTO
 *      - BYPASS_STP_TYPE_EAPOL
 *      - BYPASS_STP_TYPE_PTP
 *      - BYPASS_STP_TYPE_LLDP
 */
int32
rtk_trap_bypassStp_set(uint32 unit, rtk_trap_bypassStpType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_bypassStp_set(unit, frameType, enable);
} /* end of rtk_trap_bypassStp_set */

/* Function Name:
 *      rtk_trap_bypassVlan_get
 * Description:
 *      Get enable status of bypassing ingress VLAN filtering for specified frame type
 * Input:
 *      unit      - unit id
 *      frameType - frame type
 * Output:
 *      pEnable   - pointer to enable status of bypassing STP
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
 *      The frame type selection is as following:
 *      - BYPASS_VLAN_TYPE_USER_DEF_0
 *      - BYPASS_VLAN_TYPE_USER_DEF_1
 *      - BYPASS_VLAN_TYPE_USER_DEF_2 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_USER_DEF_3 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_USER_DEF_4 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_USER_DEF_5 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_RMA_00
 *      - BYPASS_VLAN_TYPE_RMA_02
 *      - BYPASS_VLAN_TYPE_RMA_0E
 *      - BYPASS_VLAN_TYPE_RMA_0X
 *      - BYPASS_VLAN_TYPE_OAM   (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_GMRP  (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_GVRP  (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_EAPOL
 *      - BYPASS_VLAN_TYPE_PTP
 *      - BYPASS_VLAN_TYPE_LLDP
 */
int32
rtk_trap_bypassVlan_get(uint32 unit, rtk_trap_bypassVlanType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_bypassVlan_get(unit, frameType, pEnable);
} /* end of rtk_trap_bypassVlan_get */

/* Function Name:
 *      rtk_trap_bypassVlan_set
 * Description:
 *      Set enable status of bypassing ingress VLAN filtering for specified frame type
 * Input:
 *      unit      - unit id
 *      frameType - frame type
 *      enable    - enable status of bypassing STP
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
 *      The frame type selection is as following:
 *      - BYPASS_VLAN_TYPE_USER_DEF_0
 *      - BYPASS_VLAN_TYPE_USER_DEF_1
 *      - BYPASS_VLAN_TYPE_USER_DEF_2 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_USER_DEF_3 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_USER_DEF_4 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_USER_DEF_5 (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_RMA_00
 *      - BYPASS_VLAN_TYPE_RMA_02
 *      - BYPASS_VLAN_TYPE_RMA_0E
 *      - BYPASS_VLAN_TYPE_RMA_0X
 *      - BYPASS_VLAN_TYPE_OAM   (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_GMRP  (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_GVRP  (Applicable to 8380)
 *      - BYPASS_VLAN_TYPE_EAPOL
 *      - BYPASS_VLAN_TYPE_PTP
 *      - BYPASS_VLAN_TYPE_LLDP
 */
int32
rtk_trap_bypassVlan_set(uint32 unit, rtk_trap_bypassVlanType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_bypassVlan_set(unit, frameType, enable);
} /* end of rtk_trap_bypassVlan_set */

/* Module Name    : Trap     */
/* Sub-module Name: User defined RMA */

/* Function Name:
 *      rtk_trap_userDefineRma_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3 in 8328
 *      (2) The valid range of userDefine_idx is 0~1 in 8390
 *      (3) The valid range of userDefine_idx is 0~5 in 8380
 */
int32
rtk_trap_userDefineRma_get(
    uint32                      unit,
    uint32                      userDefine_idx,
    rtk_trap_userDefinedRma_t   *pUserDefinedRma)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRma_get(unit, userDefine_idx, pUserDefinedRma);
} /* end of rtk_trap_userDefineRma_get */

/* Function Name:
 *      rtk_trap_userDefineRma_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3 in 8328
 *      (2) The valid range of userDefine_idx is 0~1 in 8390
 *      (3) The valid range of userDefine_idx is 0~5 in 8380
 */
int32
rtk_trap_userDefineRma_set(
    uint32                      unit,
    uint32                      userDefine_idx,
    rtk_trap_userDefinedRma_t   *pUserDefinedRma)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRma_set(unit, userDefine_idx, pUserDefinedRma);
} /* end of rtk_trap_userDefineRma_set */

/* Function Name:
 *      rtk_trap_userDefineRmaEnable_get
 * Description:
 *      Get enable status of user defined RMA.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 * Output:
 *      pEnable        - pointer to enable status of RMA entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~1 in 8390
 *      (2) The valid range of userDefine_idx is 0~5 in 8380
 */
int32
rtk_trap_userDefineRmaEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaEnable_get(unit, userDefine_idx, pEnable);
} /* end of rtk_trap_userDefineRmaEnable_get */

/* Function Name:
 *      rtk_trap_userDefineRmaEnable_set
 * Description:
 *      Set enable status of user defined RMA.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 *      enable         - enable status of RMA entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~1 in 8390
 *      (2) The valid range of userDefine_idx is 0~5 in 8380
 */
int32
rtk_trap_userDefineRmaEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaEnable_set(unit, userDefine_idx, enable);
} /* end of rtk_trap_userDefineRmaEnable_set */

/* Function Name:
 *      rtk_trap_userDefineRmaAction_get
 * Description:
 *      Get forwarding action of user defined RMA.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 * Output:
 *      pActoin        - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3 in 8328
 *      (2) The valid range of userDefine_idx is 0~1 in 8390
 *      (3) The valid range of userDefine_idx is 0~5 in 8380
 *      (4) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 *          - ACTION_COPY2CPU (for 8328 only)
 */
int32
rtk_trap_userDefineRmaAction_get(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaAction_get(unit, userDefine_idx, pAction);
} /* end of rtk_trap_userDefineRmaAction_get */

/* Function Name:
 *      rtk_trap_userDefineRmaAction_set
 * Description:
 *      Set forwarding action of user defined RMA.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 *      actoin         - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_FWD_ACTION   - invalid forwarding action
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3 in 8328
 *      (2) The valid range of userDefine_idx is 0~1 in 8390
 *      (3) The valid range of userDefine_idx is 0~5 in 8380
 *      (4) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 *          - ACTION_COPY2CPU (for 8328 only)
 */
int32
rtk_trap_userDefineRmaAction_set(uint32 unit, uint32 userDefine_idx, rtk_trap_rma_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaAction_set(unit, userDefine_idx, action);
} /* end of rtk_trap_userDefineRmaAction_set */

/* Function Name:
 *      rtk_trap_userDefineRmaPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 * Output:
 *      pPriority      - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaPri_get(uint32 unit, uint32 userDefine_idx, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaPri_get(unit, userDefine_idx, pPriority);
} /* end of rtk_trap_userDefineRmaPri_get */

/* Function Name:
 *      rtk_trap_userDefineRmaPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 *      priority       - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_PRIORITY     - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaPri_set(uint32 unit, uint32 userDefine_idx, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaPri_set(unit, userDefine_idx, priority);
} /* end of rtk_trap_userDefineRmaPri_set */

/* Function Name:
 *      rtk_trap_userDefineRmaPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 * Output:
 *      pEnable        - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaPriEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaPriEnable_get(unit, userDefine_idx, pEnable);
} /* end of rtk_trap_userDefineRmaPriEnable_get */

/* Function Name:
 *      rtk_trap_userDefineRmaPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 *      enable         - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_PRIORITY     - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaPriEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaPriEnable_set(unit, userDefine_idx, enable);
} /* end of rtk_trap_userDefineRmaPriEnable_set */

/* Function Name:
 *      rtk_trap_userDefineRmaVlanCheckEnable_get
 * Description:
 *      Get enable status of vlan checking on specified user defined RMA frame.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 * Output:
 *      pEnable        - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaVlanCheckEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaVlanCheckEnable_get(unit, userDefine_idx, pEnable);
} /* end of rtk_trap_userDefineRmaVlanCheckEnable_get */

/* Function Name:
 *      rtk_trap_userDefineRmaVlanCheckEnable_set
 * Description:
 *      Set enable status of vlan checking on specified user defined RMA frame.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 *      enable         - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaVlanCheckEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaVlanCheckEnable_set(unit, userDefine_idx, enable);
} /* end of rtk_trap_userDefineRmaVlanCheckEnable_set */

/* Function Name:
 *      rtk_trap_userDefineRmaStpBlockEnable_get
 * Description:
 *      Get enable status of STP status checking on specified user defined RMA frame.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 * Output:
 *      pEnable        - pointer to enable status of STP status checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaStpBlockEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaStpBlockEnable_get(unit, userDefine_idx, pEnable);
} /* end of rtk_trap_userDefineRmaStpBlockEnable_get */

/* Function Name:
 *      rtk_trap_userDefineRmaStpBlockEnable_set
 * Description:
 *      Set enable status of STP status checking on specified user defined RMA frame.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 *      enable         - enable status of STP status checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of userDefine_idx is 0~3
 */
int32
rtk_trap_userDefineRmaStpBlockEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaStpBlockEnable_set(unit, userDefine_idx, enable);
} /* end of rtk_trap_userDefineRmaStpBlockEnable_set */

/* Function Name:
 *      rtk_trap_userDefineRmaLearningEnable_get
 * Description:
 *      Get enable status of SMAC learning for user-defined RMA frame.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 * Output:
 *      pEnable        - pointer to enable status of SMAC learning
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~1 in 8390
 *      (2) The valid range of userDefine_idx is 0~5 in 8380
 */
int32
rtk_trap_userDefineRmaLearningEnable_get(uint32 unit, uint32 userDefine_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaLearningEnable_get(unit, userDefine_idx, pEnable);
} /* end of rtk_trap_userDefineRmaLearningEnable_get */

/* Function Name:
 *      rtk_trap_userDefineRmaLearningEnable_set
 * Description:
 *      Set enable status of SMAC learning for this user-defined RMA frame.
 * Input:
 *      unit           - unit id
 *      userDefine_idx - index of user defined RMA entry
 *      enable         - enable status of SMAC learning
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The valid range of userDefine_idx is 0~1 in 8390
 *      (2) The valid range of userDefine_idx is 0~5 in 8380
 */
int32
rtk_trap_userDefineRmaLearningEnable_set(uint32 unit, uint32 userDefine_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineRmaLearningEnable_set(unit, userDefine_idx, enable);
} /* end of rtk_trap_userDefineRmaLearningEnable_set */

/* Module Name    : Trap                         */
/* Sub-module Name: System-wise management frame */

/* Function Name:
 *      rtk_trap_mgmtFrameAction_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Type of management frame in 8328 is as following:
 *          - MGMT_TYPE_RIP
 *          - MGMT_TYPE_ICMP
 *          - MGMT_TYPE_ICMPV6
 *          - MGMT_TYPE_ARP
 *          - MGMT_TYPE_MLD
 *          - MGMT_TYPE_IGMP
 *          - MGMT_TYPE_BGP
 *          - MGMT_TYPE_OSPFV2
 *          - MGMT_TYPE_OSPFV3
 *          - MGMT_TYPE_SNMP
 *          - MGMT_TYPE_SSH
 *          - MGMT_TYPE_FTP
 *          - MGMT_TYPE_TFTP
 *          - MGMT_TYPE_TELNET
 *          - MGMT_TYPE_HTTP
 *          - MGMT_TYPE_HTTPS
 *
 *          Type of management frame in 8390 is as following:
 *          - MGMT_TYPE_ARP
 *          - MGMT_TYPE_EAPOL
 *          - MGMT_TYPE_IGMP
 *          - MGMT_TYPE_IPV6ND
 *          - MGMT_TYPE_MLD
 *          - MGMT_TYPE_SELFMAC
 *          - MGMT_TYPE_IPV6_HOP_POS_ERR
 *          - MGMT_TYPE_IPV6_HDR_UNKWN
 *          - MGMT_TYPE_L2_CRC_ERR
 *          - MGMT_TYPE_IP4_CHKSUM_ERR
 *
 *          Type of management frame in 8380 is as following:
 *          - MGMT_TYPE_MLD
 *          - MGMT_TYPE_IGMP
 *          - MGMT_TYPE_EAPOL
 *          - MGMT_TYPE_ARP
 *          - MGMT_TYPE_IPV6ND
 *          - MGMT_TYPE_SELFMAC
 *          - MGMT_TYPE_IPV6_HOP_POS_ERR
 *          - MGMT_TYPE_IPV6_HDR_UNKWN
 *          - MGMT_TYPE_L2_CRC_ERR
 *
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 *          - ACTION_COPY2CPU
 */
int32
rtk_trap_mgmtFrameAction_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameAction_get(unit, frameType, pAction);
} /* end of rtk_trap_mgmtFrameAction_get */

/* Function Name:
 *      rtk_trap_mgmtFrameAction_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_FWD_ACTION    - invalid forwarding action
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Type of management frame in 8328 is as following:
 *          - MGMT_TYPE_RIP
 *          - MGMT_TYPE_ICMP
 *          - MGMT_TYPE_ICMPV6
 *          - MGMT_TYPE_ARP
 *          - MGMT_TYPE_MLD
 *          - MGMT_TYPE_IGMP
 *          - MGMT_TYPE_BGP
 *          - MGMT_TYPE_OSPFV2
 *          - MGMT_TYPE_OSPFV3
 *          - MGMT_TYPE_SNMP
 *          - MGMT_TYPE_SSH
 *          - MGMT_TYPE_FTP
 *          - MGMT_TYPE_TFTP
 *          - MGMT_TYPE_TELNET
 *          - MGMT_TYPE_HTTP
 *          - MGMT_TYPE_HTTPS
 *
 *          Type of management frame in 8390 is as following:
 *          - MGMT_TYPE_ARP
 *          - MGMT_TYPE_EAPOL
 *          - MGMT_TYPE_IGMP
 *          - MGMT_TYPE_IPV6ND
 *          - MGMT_TYPE_MLD
 *          - MGMT_TYPE_SELFMAC
 *          - MGMT_TYPE_IPV6_HOP_POS_ERR
 *          - MGMT_TYPE_IPV6_HDR_UNKWN
 *          - MGMT_TYPE_L2_CRC_ERR
 *          - MGMT_TYPE_IP4_CHKSUM_ERR
 *
 *          Type of management frame in 8380 is as following:
 *          - MGMT_TYPE_MLD
 *          - MGMT_TYPE_IGMP
 *          - MGMT_TYPE_EAPOL
 *          - MGMT_TYPE_ARP
 *          - MGMT_TYPE_IPV6ND
 *          - MGMT_TYPE_SELFMAC
 *          - MGMT_TYPE_IPV6_HOP_POS_ERR
 *          - MGMT_TYPE_IPV6_HDR_UNKWN
 *          - MGMT_TYPE_L2_CRC_ERR
 *
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 *          - ACTION_COPY2CPU
 */
int32
rtk_trap_mgmtFrameAction_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameAction_set(unit, frameType, action);
} /* end of rtk_trap_mgmtFrameAction_set */

/* Function Name:
 *      rtk_trap_mgmtFramePri_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      - rtk_trap_mgmtType_t \ Chip:     8328    8390    8380
 *      - MGMT_TYPE_RIP                   O       X       X
 *      - MGMT_TYPE_ICMP                  O       X       X
 *      - MGMT_TYPE_ICMPV6                O       X       X
 *      - MGMT_TYPE_ARP                   O       O       X
 *      - MGMT_TYPE_MLD                   O       O       X
 *      - MGMT_TYPE_IGMP                  O       O       X
 *      - MGMT_TYPE_BGP                   O       X       X
 *      - MGMT_TYPE_OSPFV2                O       X       X
 *      - MGMT_TYPE_OSPFV3                O       X       X
 *      - MGMT_TYPE_SNMP                  O       X       X
 *      - MGMT_TYPE_SSH                   O       X       X
 *      - MGMT_TYPE_FTP                   O       X       X
 *      - MGMT_TYPE_TFTP                  O       X       X
 *      - MGMT_TYPE_TELNET                O       X       X
 *      - MGMT_TYPE_HTTP                  O       X       X
 *      - MGMT_TYPE_HTTPS                 O       X       X
 *      - MGMT_TYPE_DHCPV6                O       X       X
 *      - MGMT_TYPE_DHCP                  O       X       X
 *      - MGMT_TYPE_DOT1X                 O       X       X
 *      - MGMT_TYPE_BPDU                  O       O       X
 *      - MGMT_TYPE_PTP                   X       O       X
 *      - MGMT_TYPE_LLDP                  X       O       X
 *      - MGMT_TYPE_EAPOL                 X       O       X
 *      - MGMT_TYPE_IPV6ND                X       O       X
 *      - MGMT_TYPE_SELFMAC               X       O       O
 *      - MGMT_TYPE_OTHER                 X       O       O
 *      - MGMT_TYPE_OAM                   X       O       X
 *      - MGMT_TYPE_CFM                   X       O       X
 *      - MGMT_TYPE_IGR_VLAN_FLTR         X       O       X
 *      - MGMT_TYPE_VLAN_ERR              X       O       X
 *      - MGMT_TYPE_CFI                   X       O       O
 *      - MGMT_TYPE_RMA_USR_DEF_1         X       O       X
 *      - MGMT_TYPE_RMA_USR_DEF_2         X       O       X
 *      - MGMT_TYPE_LACP                  X       O       X
 *      - MGMT_TYPE_RMA_0X                X       O       X
 *      - MGMT_TYPE_RMA_1X                X       O       X
 *      - MGMT_TYPE_RMA_2X                X       O       X
 *      - MGMT_TYPE_UNKNOWN_DA            X       O       O
 *      - MGMT_TYPE_RLDP_RLPP             X       X       O
 *      - MGMT_TYPE_RMA                   X       X       O
 *      - MGMT_TYPE_SPECIAL_COPY          X       X       O
 *      - MGMT_TYPE_SPECIAL_TRAP          X       X       O
 *      - MGMT_TYPE_ROUT_EXCEPT           X       X       O
 *      - MGMT_TYPE_MAC_CST_SYS           X       X       O
 *      - MGMT_TYPE_MAC_CST_PORT          X       X       O
 *      - MGMT_TYPE_MAC_CST_VLAN          X       X       O
 *      - MGMT_TYPE_IPV6_HOP_POS_ERR      X       O       X
 *      - MGMT_TYPE_IPV6_HDR_UNKWN        X       O       X
 *      - MGMT_TYPE_INVALID_SA            X       O       X
 */
int32
rtk_trap_mgmtFramePri_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFramePri_get(unit, frameType, pPriority);
} /* end of rtk_trap_mgmtFramePri_get */

/* Function Name:
 *      rtk_trap_mgmtFramePri_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_PRIORITY      - invalid priority value
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      - rtk_trap_mgmtType_t \ Chip:     8328    8390    8380
 *      - MGMT_TYPE_RIP                   O       X       X
 *      - MGMT_TYPE_ICMP                  O       X       X
 *      - MGMT_TYPE_ICMPV6                O       X       X
 *      - MGMT_TYPE_ARP                   O       O       X
 *      - MGMT_TYPE_MLD                   O       O       X
 *      - MGMT_TYPE_IGMP                  O       O       X
 *      - MGMT_TYPE_BGP                   O       X       X
 *      - MGMT_TYPE_OSPFV2                O       X       X
 *      - MGMT_TYPE_OSPFV3                O       X       X
 *      - MGMT_TYPE_SNMP                  O       X       X
 *      - MGMT_TYPE_SSH                   O       X       X
 *      - MGMT_TYPE_FTP                   O       X       X
 *      - MGMT_TYPE_TFTP                  O       X       X
 *      - MGMT_TYPE_TELNET                O       X       X
 *      - MGMT_TYPE_HTTP                  O       X       X
 *      - MGMT_TYPE_HTTPS                 O       X       X
 *      - MGMT_TYPE_DHCPV6                O       X       X
 *      - MGMT_TYPE_DHCP                  O       X       X
 *      - MGMT_TYPE_DOT1X                 O       X       X
 *      - MGMT_TYPE_BPDU                  O       O       X
 *      - MGMT_TYPE_PTP                   X       O       X
 *      - MGMT_TYPE_LLDP                  X       O       X
 *      - MGMT_TYPE_EAPOL                 X       O       X
 *      - MGMT_TYPE_IPV6ND                X       O       X
 *      - MGMT_TYPE_SELFMAC               X       O       O
 *      - MGMT_TYPE_OTHER                 X       O       O
 *      - MGMT_TYPE_OAM                   X       O       X
 *      - MGMT_TYPE_CFM                   X       O       X
 *      - MGMT_TYPE_IGR_VLAN_FLTR         X       O       X
 *      - MGMT_TYPE_VLAN_ERR              X       O       X
 *      - MGMT_TYPE_CFI                   X       O       O
 *      - MGMT_TYPE_RMA_USR_DEF_1         X       O       X
 *      - MGMT_TYPE_RMA_USR_DEF_2         X       O       X
 *      - MGMT_TYPE_LACP                  X       O       X
 *      - MGMT_TYPE_RMA                   X       O       O
 *      - MGMT_TYPE_UNKNOWN_DA            X       O       O
 *      - MGMT_TYPE_RLDP_RLPP             X       X       O
 *      - MGMT_TYPE_SPECIAL_COPY          X       X       O
 *      - MGMT_TYPE_SPECIAL_TRAP          X       X       O
 *      - MGMT_TYPE_ROUT_EXCEPT           X       X       O
 *      - MGMT_TYPE_MAC_CST_SYS           X       X       O
 *      - MGMT_TYPE_MAC_CST_PORT          X       X       O
 *      - MGMT_TYPE_MAC_CST_VLAN          X       X       O
 *      - MGMT_TYPE_IPV6_HOP_POS_ERR      X       O       X
 *      - MGMT_TYPE_IPV6_HDR_UNKWN        X       O       X
 *      - MGMT_TYPE_INVALID_SA            X       O       X
 */
int32
rtk_trap_mgmtFramePri_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFramePri_set(unit, frameType, priority);
} /* end of rtk_trap_mgmtFramePri_set */

/* Function Name:
 *      rtk_trap_mgmtFramePriEnable_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_mgmtFramePriEnable_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFramePriEnable_get(unit, frameType, pEnable);
} /* end of rtk_trap_mgmtFramePriEnable_get */

/* Function Name:
 *      rtk_trap_mgmtFramePriEnable_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_PRIORITY      - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_mgmtFramePriEnable_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFramePriEnable_set(unit, frameType, enable);
} /* end of rtk_trap_mgmtFramePriEnable_set */

/* Function Name:
 *      rtk_trap_mgmtFrameVlanCheck_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_INPUT         - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_mgmtFrameVlanCheck_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameVlanCheck_get(unit, frameType, pEnable);
} /* end of rtk_trap_mgmtFrameVlanCheck_get */

/* Function Name:
 *      rtk_trap_mgmtFrameVlanCheck_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_INPUT         - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_mgmtFrameVlanCheck_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameVlanCheck_set(unit, frameType, enable);
} /* end of rtk_trap_mgmtFrameVlanCheck_set */

/* Function Name:
 *      rtk_trap_mgmtFrameLearningEnable_get
 * Description:
 *      Get enable status of SMAC learning for the management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 * Output:
 *      pEnable   - pointer to enable status of SMAC learning
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      - MGMT_TYPE_PTP
 *      - MGMT_TYPE_LLDP
 *      - MGMT_TYPE_BPDU (Applicable to 8380)
 */
int32
rtk_trap_mgmtFrameLearningEnable_get(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameLearningEnable_get(unit, frameType, pEnable);
} /* end of rtk_trap_mgmtFrameLearningEnable_get */

/* Function Name:
 *      rtk_trap_mgmtFrameLearningEnable_set
 * Description:
 *      Set enable status of SMAC learning for the management frame.
 * Input:
 *      unit      - unit id
 *      frameType - type of management frame
 *      enable    - enable status of SMAC learning
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_INPUT         - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      - MGMT_TYPE_PTP
 *      - MGMT_TYPE_LLDP
 *      - MGMT_TYPE_BPDU (Applicable to 8380)
 */
int32
rtk_trap_mgmtFrameLearningEnable_set(uint32 unit, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameLearningEnable_set(unit, frameType, enable);
} /* end of rtk_trap_mgmtFrameLearningEnable_set */

/* Module Name    : Trap                                      */
/* Sub-module Name: System-wise user defined management frame */

/* Function Name:
 *      rtk_trap_userDefineMgmt_get
 * Description:
 *      Get user defined management frame.
 * Input:
 *      unit        - unit id
 *      mgmt_idx    - index of user defined management frame entry
 * Output:
 *      pUserDefine - pointer to user defined management frame
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmt_get(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmt_get(unit, mgmt_idx, pUserDefine);
} /* end of rtk_trap_userDefineMgmt_get */

/* Function Name:
 *      rtk_trap_userDefineMgmt_set
 * Description:
 *      Set user defined management frame.
 * Input:
 *      unit        - unit id
 *      mgmt_idx    - index of user defined management frame entry
 *      pUserDefine - user defined management frame
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmt_set(uint32 unit, uint32 mgmt_idx, rtk_trap_userDefinedMgmt_t *pUserDefine)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmt_set(unit, mgmt_idx, pUserDefine);
} /* end of rtk_trap_userDefineMgmt_set */

/* Function Name:
 *      rtk_trap_userDefineMgmtAction_get
 * Description:
 *      Get forwarding action of user defined management frame.
 * Input:
 *      unit     - unit id
 *      mgmt_idx - index of user defined management frame entry
 * Output:
 *      pActoin  - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 */
int32
rtk_trap_userDefineMgmtAction_get(uint32 unit, uint32 mgmt_idx, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtAction_get(unit, mgmt_idx, pAction);
} /* end of rtk_trap_userDefineMgmtAction_get */

/* Function Name:
 *      rtk_trap_userDefineMgmtAction_set
 * Description:
 *      Set forwarding action of user defined management frame.
 * Input:
 *      unit     - unit id
 *      mgmt_idx - index of user defined management frame entry
 *      actoin   - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_FWD_ACTION   - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 */
int32
rtk_trap_userDefineMgmtAction_set(uint32 unit, uint32 mgmt_idx, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtAction_set(unit, mgmt_idx, action);
} /* end of rtk_trap_userDefineMgmtAction_set */

/* Function Name:
 *      rtk_trap_userDefineMgmtPri_get
 * Description:
 *      Get priority of packets trapped to CPU.
 * Input:
 *      unit      - unit id
 *      mgmt_idx  - index of user defined management frame entry
 * Output:
 *      pPriority - pointer to priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmtPri_get(uint32 unit, uint32 mgmt_idx, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtPri_get(unit, mgmt_idx, pPriority);
} /* end of rtk_trap_userDefineMgmtPri_get */

/* Function Name:
 *      rtk_trap_userDefineMgmtPri_set
 * Description:
 *      Set priority of packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      mgmt_idx - index of user defined management frame entry
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_PRIORITY     - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmtPri_set(uint32 unit, uint32 mgmt_idx, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtPri_set(unit, mgmt_idx, priority);
} /* end of rtk_trap_userDefineMgmtPri_set */

/* Function Name:
 *      rtk_trap_userDefineMgmtPriEnable_get
 * Description:
 *      Get priority enable status of packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      mgmt_idx - index of user defined management frame entry
 * Output:
 *      pEnable  - pointer to priority enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmtPriEnable_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtPriEnable_get(unit, mgmt_idx, pEnable);
} /* end of rtk_trap_userDefineMgmtPriEnable_get */

/* Function Name:
 *      rtk_trap_userDefineMgmtPriEnable_set
 * Description:
 *      Set priority enable status of packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      mgmt_idx - index of user defined management frame entry
 *      enable   - priority enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmtPriEnable_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtPriEnable_set(unit, mgmt_idx, enable);
} /* end of rtk_trap_userDefineMgmtPriEnable_set */

/* Function Name:
 *      rtk_trap_userDefineMgmtVlanCheck_get
 * Description:
 *      Get enable status of vlan checking on specified user defined management frame.
 * Input:
 *      unit     - unit id
 *      mgmt_idx - index of user defined management frame entry
 *      enable   - enable status of vlan checking
 * Output:
 *      pEnable        - pointer to enable status of vlan checking
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmtVlanCheck_get(uint32 unit, uint32 mgmt_idx, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtVlanCheck_get(unit, mgmt_idx, pEnable);
} /* end of rtk_trap_userDefineMgmtVlanCheck_get */

/* Function Name:
 *      rtk_trap_userDefineMgmtVlanCheck_set
 * Description:
 *      Set enable status of vlan checking on specified user defined management frame.
 * Input:
 *      unit     - unit id
 *      mgmt_idx - index of user defined management frame entry
 *      enable   - enable status of vlan checking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of mgmt_idx is 0~1
 */
int32
rtk_trap_userDefineMgmtVlanCheck_set(uint32 unit, uint32 mgmt_idx, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_userDefineMgmtVlanCheck_set(unit, mgmt_idx, enable);
} /* end of rtk_trap_userDefineMgmtVlanCheck_set */

/* Module Name    : Trap                                   */
/* Sub-module Name: Per port user defined management frame */

/* Function Name:
 *      rtk_trap_portMgmtFrameAction_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Supported management frame types in 8328 are as following:
 *          - MGMT_TYPE_DHCPV6
 *          - MGMT_TYPE_DHCP
 *          - MGMT_TYPE_DOT1X
 *          - MGMT_TYPE_BPDU
 *
 *          Supported management frame types in 8390, 8380 are as following:
 *          - MGMT_TYPE_BPDU
 *          - MGMT_TYPE_PTP
 *          - MGMT_TYPE_LLDP
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 *          - ACTION_COPY2CPU (8328, 8380 only)
 *          - ACTION_FLOOD_IN_ALL_PORT (8390 and MGMT_TYPE_BPDU only)
 */
int32
rtk_trap_portMgmtFrameAction_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->trap_portMgmtFrameAction_get(unit, phy_port, frameType, pAction);
    }
#else
    return RT_MAPPER(unit)->trap_portMgmtFrameAction_get(unit, port, frameType, pAction);
#endif
} /* end of rtk_trap_portMgmtFrameAction_get */

/* Function Name:
 *      rtk_trap_portMgmtFrameAction_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_FWD_ACTION    - invalid forwarding action
 * Applicable:
 *      8328, 8390, 8380
 * Note:
 *      (1) Supported management frame types in 8328 are as following:
 *          - MGMT_TYPE_DHCPV6
 *          - MGMT_TYPE_DHCP
 *          - MGMT_TYPE_DOT1X
 *          - MGMT_TYPE_BPDU
 *
 *          Supported management frame types in 8390, 8380 are as following:
 *          - MGMT_TYPE_BPDU
 *          - MGMT_TYPE_PTP
 *          - MGMT_TYPE_LLDP
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 *          - ACTION_DROP
 *          - ACTION_COPY2CPU (8328, 8380 only)
 *          - ACTION_FLOOD_IN_ALL_PORT (8390 and MGMT_TYPE_BPDU only)
 */
int32
rtk_trap_portMgmtFrameAction_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->trap_portMgmtFrameAction_set(unit, phy_port, frameType, action);
    }
#else
    return RT_MAPPER(unit)->trap_portMgmtFrameAction_set(unit, port, frameType, action);
#endif
} /* end of rtk_trap_portMgmtFrameAction_set */

/* Function Name:
 *      rtk_trap_portMgmtFramePri_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFramePri_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFramePri_get(unit, port, frameType, pPriority);
} /* end of rtk_trap_portMgmtFramePri_get */

/* Function Name:
 *      rtk_trap_portMgmtFramePri_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_PRIORITY      - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFramePri_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFramePri_set(unit, port, frameType, priority);
} /* end of rtk_trap_portMgmtFramePri_set */

/* Function Name:
 *      rtk_trap_portMgmtFramePriEnable_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFramePriEnable_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFramePriEnable_get(unit, port, frameType, pEnable);
} /* end of rtk_trap_portMgmtFramePriEnable_get */

/* Function Name:
 *      rtk_trap_portMgmtFramePriEnable_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_PRIORITY      - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFramePriEnable_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFramePriEnable_set(unit, port, frameType, enable);
} /* end of rtk_trap_portMgmtFramePriEnable_set */

/* Function Name:
 *      rtk_trap_portMgmtFrameVlanCheck_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFrameVlanCheck_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFrameVlanCheck_get(unit, port, frameType, pEnable);
} /* end of rtk_trap_portMgmtFrameVlanCheck_get */

/* Function Name:
 *      rtk_trap_portMgmtFrameVlanCheck_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_INPUT         - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFrameVlanCheck_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFrameVlanCheck_set(unit, port, frameType, enable);
} /* end of rtk_trap_portMgmtFrameVlanCheck_set */

/* Function Name:
 *      rtk_trap_portMgmtFrameCrossVlan_get
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFrameCrossVlan_get(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFrameCrossVlan_get(unit, port, frameType, pEnable);
} /* end of rtk_trap_portMgmtFrameCrossVlan_get */

/* Function Name:
 *      rtk_trap_portMgmtFrameCrossVlan_set
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
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_RMA_MGMT_TYPE - invalid type of management frame
 *      RT_ERR_INPUT         - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portMgmtFrameCrossVlan_set(uint32 unit, rtk_port_t port, rtk_trap_mgmtType_t frameType, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portMgmtFrameCrossVlan_set(unit, port, frameType, enable);
} /* end of rtk_trap_portMgmtFrameCrossVlan_set */

/* Module Name    : Trap                               */
/* Sub-module Name: Packet with special flag or option */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) IP family is as following:
 *          - IPV4_FAMILY
 *          - IPV6_FAMILY
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 */
int32
rtk_trap_ipWithOptionHeaderAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_ip_family_t     ipFamily,
    rtk_action_t        *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderAction_get(unit, port, ipFamily, pAction);
} /* end of rtk_trap_ipWithOptionHeaderAction_get */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderAction_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_INPUT      - invalid input parameter
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      (1) IP family is as following:
 *          - IPV4_FAMILY
 *          - IPV6_FAMILY
 *      (2) Forwarding action is as following:
 *          - ACTION_FORWARD
 *          - ACTION_TRAP2CPU
 */
int32
rtk_trap_ipWithOptionHeaderAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_ip_family_t     ipFamily,
    rtk_action_t        action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderAction_set(unit, port, ipFamily, action);
} /* end of rtk_trap_ipWithOptionHeaderAction_set */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderPri_get
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
rtk_trap_ipWithOptionHeaderPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderPri_get(unit, port, pPriority);
} /* end of rtk_trap_ipWithOptionHeaderPri_get */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_ipWithOptionHeaderPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderPri_set(unit, port, priority);
} /* end of rtk_trap_ipWithOptionHeaderPri_set */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderPriEnable_get
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
rtk_trap_ipWithOptionHeaderPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderPriEnable_get(unit, port, pEnable);
} /* end of rtk_trap_ipWithOptionHeaderPriEnable_get */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderPriEnable_set
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
rtk_trap_ipWithOptionHeaderPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderPriEnable_set(unit, port, enable);
} /* end of rtk_trap_ipWithOptionHeaderPriEnable_set */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderAddCPUTagEnable_get
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
rtk_trap_ipWithOptionHeaderAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderAddCPUTagEnable_get(unit, port, pEnable);
} /* end of rtk_trap_ipWithOptionHeaderAddCPUTagEnable_get */

/* Function Name:
 *      rtk_trap_ipWithOptionHeaderAddCPUTagEnable_set
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
rtk_trap_ipWithOptionHeaderAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_ipWithOptionHeaderAddCPUTagEnable_set(unit, port, enable);
} /* end of rtk_trap_ipWithOptionHeaderAddCPUTagEnable_set */

/* Function Name:
 *      rtk_trap_pktWithCFIAction_get
 * Description:
 *      Get forwarding action of ethernet packet with CFI set.
 * Input:
 *      unit    - unit id
 * Output:
 *      pActoin - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_pktWithCFIAction_get(uint32 unit, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIAction_get(unit, pAction);
} /* end of rtk_trap_pktWithCFIAction_get */

/* Function Name:
 *      rtk_trap_pktWithCFIAction_set
 * Description:
 *      Set forwarding action of ethernet packet with CFI set.
 * Input:
 *      unit   - unit id
 *      actoin - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8390, 8380
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_pktWithCFIAction_set(uint32 unit, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIAction_set(unit, action);
} /* end of rtk_trap_pktWithCFIAction_set */

/* Function Name:
 *      rtk_trap_pktWithOuterCFIAction_get
 * Description:
 *      Get forwarding action of ethernet packet with outer CFI set.
 * Input:
 *      unit    - unit id
 * Output:
 *      pActoin - pointer to forwarding action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_pktWithOuterCFIAction_get(uint32 unit, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithOuterCFIAction_get(unit, pAction);
} /* end of rtk_trap_pktWithOuterCFIAction_get */

/* Function Name:
 *      rtk_trap_pktWithOuterCFIAction_set
 * Description:
 *      Set forwarding action of ethernet packet with outer CFI set.
 * Input:
 *      unit   - unit id
 *      actoin - forwarding action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8390, 8380
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_pktWithOuterCFIAction_set(uint32 unit, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithOuterCFIAction_set(unit, action);
} /* end of rtk_trap_pktWithOuterCFIAction_set */

/* Function Name:
 *      rtk_trap_portPktWithCFIAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_portPktWithCFIAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portPktWithCFIAction_get(unit, port, pAction);
} /* end of rtk_trap_portPktWithCFIAction_get */

/* Function Name:
 *      rtk_trap_portPktWithCFIAction_set
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
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_PORT_ID    - invalid port id
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      Forwarding action is as following:
 *      - ACTION_FORWARD
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_portPktWithCFIAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portPktWithCFIAction_set(unit, port, action);
} /* end of rtk_trap_portPktWithCFIAction_set */

/* Function Name:
 *      rtk_trap_pktWithCFIPri_get
 * Description:
 *      Get priority of packets trapped to CPU with CFI=1.
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
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_trap_pktWithCFIPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIPri_get(unit, pPriority);
} /* end of rtk_trap_pktWithCFIPri_get */

/* Function Name:
 *      rtk_trap_pktWithCFIPri_set
 * Description:
 *      Set priority of packets trapped to CPU with CFI=1.
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
 *      8390, 8380
 * Note:
 *      None
 */
int32
rtk_trap_pktWithCFIPri_set(uint32 unit, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIPri_set(unit, priority);
} /* end of rtk_trap_pktWithCFIPri_set */

/* Function Name:
 *      rtk_trap_portPktWithCFIPri_get
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
rtk_trap_portPktWithCFIPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portPktWithCFIPri_get(unit, port, pPriority);
} /* end of rtk_trap_portPktWithCFIPri_get */

/* Function Name:
 *      rtk_trap_portPktWithCFIPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portPktWithCFIPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portPktWithCFIPri_set(unit, port, priority);
} /* end of rtk_trap_portPktWithCFIPri_set */

/* Function Name:
 *      rtk_trap_pktWithCFIPriEnable_get
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
rtk_trap_pktWithCFIPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIPriEnable_get(unit, port, pEnable);
} /* end of rtk_trap_pktWithCFIPriEnable_get */

/* Function Name:
 *      rtk_trap_pktWithCFIPriEnable_set
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
rtk_trap_pktWithCFIPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIPriEnable_set(unit, port, enable);
} /* end of rtk_trap_pktWithCFIPriEnable_set */

/* Function Name:
 *      rtk_trap_pktWithCFIAddCPUTagEnable_get
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
rtk_trap_pktWithCFIAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIAddCPUTagEnable_get(unit, port, pEnable);
} /* end of rtk_trap_pktWithCFIAddCPUTagEnable_get */

/* Function Name:
 *      rtk_trap_pktWithCFIAddCPUTagEnable_set
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
rtk_trap_pktWithCFIAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_pktWithCFIAddCPUTagEnable_set(unit, port, enable);
} /* end of rtk_trap_pktWithCFIAddCPUTagEnable_set */

/* Module Name    : Trap       */
/* Sub-module Name: CFM and OAM packet */

/* Function Name:
 *      rtk_trap_cfmFrameAction_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of level is 0~7
 *      (2) The valid range of action is as following
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_COPY2CPU
 */
int32
rtk_trap_cfmFrameAction_get(uint32 unit, uint32 level, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameAction_get(unit, level, pAction);
} /* end of rtk_trap_cfmFrameAction_get */

/* Function Name:
 *      rtk_trap_cfmFrameAction_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - userDefine_idx is out of range
 *      RT_ERR_FWD_ACTION   - invalid forwarding action
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of level is 0~7
 *      (2) The valid range of action is as following
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *          - ACTION_COPY2CPU
 */
int32
rtk_trap_cfmFrameAction_set(uint32 unit, uint32 level, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameAction_set(unit, level, action);
} /* end of rtk_trap_cfmFrameAction_set */

/* Function Name:
 *      rtk_trap_cfmFrameTrapPri_get
 * Description:
 *      Get priority of CFM packets trapped to CPU.
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
 *      8328, 8390
 * Note:
 *      None
 */
int32
rtk_trap_cfmFrameTrapPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameTrapPri_get(unit, pPriority);
} /* end of rtk_trap_cfmFrameTrapPri_get */

/* Function Name:
 *      rtk_trap_cfmFrameTrapPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328, 8390
 * Note:
 *      None
 */
int32
rtk_trap_cfmFrameTrapPri_set(uint32 unit, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameTrapPri_set(unit, priority);
} /* end of rtk_trap_cfmFrameTrapPri_set */

/* Function Name:
 *      rtk_trap_cfmFrameTrapPriEnable_get
 * Description:
 *      Get priority enable status of CFM packets trapped to CPU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to priority enable status
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
rtk_trap_cfmFrameTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameTrapPriEnable_get(unit, pEnable);
} /* end of rtk_trap_cfmFrameTrapPriEnable_get */

/* Function Name:
 *      rtk_trap_cfmFrameTrapPriEnable_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_cfmFrameTrapPriEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameTrapPriEnable_set(unit, enable);
} /* end of rtk_trap_cfmFrameTrapPriEnable_set */

/* Function Name:
 *      rtk_trap_cfmFrameTrapAddCPUTagEnable_get
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
rtk_trap_cfmFrameTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameTrapAddCPUTagEnable_get(unit, pEnable);
} /* end of rtk_trap_cfmFrameTrapAddCPUTagEnable_get */

/* Function Name:
 *      rtk_trap_cfmFrameTrapAddCPUTagEnable_set
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
rtk_trap_cfmFrameTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmFrameTrapAddCPUTagEnable_set(unit, enable);
} /* end of rtk_trap_cfmFrameTrapAddCPUTagEnable_set */

/* Function Name:
 *      rtk_trap_oamPDUAction_get
 * Description:
 *      Get forwarding action of trapped oam PDU.
 * Input:
 *      unit    - unit id
 * Output:
 *      pAction - pointer to forwarding action of trapped oam PDU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_oamPDUAction_get(uint32 unit, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUAction_get(unit, pAction);
} /* end of rtk_trap_oamPDUAction_get */

/* Function Name:
 *      rtk_trap_oamPDUAction_set
 * Description:
 *      Set forwarding action of trapped oam PDU.
 * Input:
 *      unit   - unit id
 *      action - forwarding action of trapped oam PDU
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_FWD_ACTION - invalid forwarding action
 * Applicable:
 *      8390
 * Note:
 *      (1) Forwarding action is as following
 *          - ACTION_FORWARD
 *          - ACTION_DROP
 *          - ACTION_TRAP2CPU
 *      (2) When configure trap action, the non OAM PDU check OAM loopback parser action.
 *          Otherwise the non OAM PDU will be normal forward.
 */
int32
rtk_trap_oamPDUAction_set(uint32 unit, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUAction_set(unit, action);
} /* end of rtk_trap_oamPDUAction_set */

/* Function Name:
 *      rtk_trap_oamPDUPri_get
 * Description:
 *      Get priority of trapped OAM PDU.
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
 *      8390
 * Note:
 *      The priority takes effect if 'rtk_trap_oamPDUAction_set' is set to ACTION_TRAP2CPU.
 */
int32
rtk_trap_oamPDUPri_get(uint32 unit, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUPri_get(unit, pPriority);
} /* end of rtk_trap_oamPDUPri_get */

/* Function Name:
 *      rtk_trap_oamPDUPri_set
 * Description:
 *      Set priority of trapped OAM PDU.
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
 *      8390
 * Note:
 *      The priority takes effect if 'rtk_trap_oamPDUAction_set' is set to ACTION_TRAP2CPU.
 */
int32
rtk_trap_oamPDUPri_set(uint32 unit, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUPri_set(unit, priority);
} /* end of rtk_trap_oamPDUPri_set */

/* Function Name:
 *      rtk_trap_portOamPDUAction_get
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
rtk_trap_portOamPDUAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portOamPDUAction_get(unit, port, pAction);
} /* end of rtk_trap_portOamPDUAction_get */

/* Function Name:
 *      rtk_trap_portOamPDUAction_set
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
rtk_trap_portOamPDUAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portOamPDUAction_set(unit, port, action);
} /* end of rtk_trap_portOamPDUAction_set */

/* Function Name:
 *      rtk_trap_portOamPDUPri_get
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
rtk_trap_portOamPDUPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portOamPDUPri_get(unit, port, pPriority);
} /* end of rtk_trap_portOamPDUPri_get */

/* Function Name:
 *      rtk_trap_portOamPDUPri_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_trap_portOamPDUPri_set(uint32 unit, rtk_port_t port, rtk_pri_t priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portOamPDUPri_set(unit, port, priority);
} /* end of rtk_trap_portOamPDUPri_set */

/* Function Name:
 *      rtk_trap_oamPDUPriEnable_get
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
rtk_trap_oamPDUPriEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUPriEnable_get(unit, port, pEnable);
} /* end of rtk_trap_oamPDUPriEnable_get */

/* Function Name:
 *      rtk_trap_oamPDUPriEnable_set
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
rtk_trap_oamPDUPriEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUPriEnable_set(unit, port, enable);
} /* end of rtk_trap_oamPDUPriEnable_set */

/* Function Name:
 *      rtk_trap_oamPDUTrapAddCPUTagEnable_get
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
rtk_trap_oamPDUTrapAddCPUTagEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUTrapAddCPUTagEnable_get(unit, port, pEnable);
} /* end of rtk_trap_oamPDUTrapAddCPUTagEnable_get */

/* Function Name:
 *      rtk_trap_oamPDUTrapAddCPUTagEnable_set
 * Description:
 *      Set enable status of CPU tag adding for trapped OAM PDU.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of CPU tag adding
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
rtk_trap_oamPDUTrapAddCPUTagEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_oamPDUTrapAddCPUTagEnable_set(unit, port, enable);
} /* end of rtk_trap_oamPDUTrapAddCPUTagEnable_set */

/* Function Name:
 *      rtk_trap_mgmtIpCheck_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Type of management ip type is as following:
 *      - MGMT_IP_TYPE_IPV4
 *      - MGMT_IP_TYPE_IPV6
 */
int32
rtk_trap_mgmtIpCheck_get(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtIpCheck_get(unit, type, pEnable);
} /* end of rtk_trap_mgmtIpCheck_get */

/* Function Name:
 *      rtk_trap_mgmtIpCheck_set
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
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      Type of management ip type is as following:
 *      - MGMT_IP_TYPE_IPV4
 *      - MGMT_IP_TYPE_IPV6
 */
int32
rtk_trap_mgmtIpCheck_set(uint32 unit, rtk_trap_mgmtIpType_t type, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtIpCheck_set(unit, type, enable);
} /* end of rtk_trap_mgmtIpCheck_set */

/* Function Name:
 *      rtk_trap_cfmUnknownFrameAct_get
 * Description:
 *      Get action for receive unknown type of CFM frame.
 * Input:
 *      unit    - unit id
 * Output:
 *      pAction - pointer buffer of action for receive unknown type of CFM frame
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_cfmUnknownFrameAct_get(uint32 unit, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmUnknownFrameAct_get(unit, pAction);
} /* end of rtk_trap_cfmUnknownFrameAct_get */

/* Function Name:
 *      rtk_trap_cfmUnknownFrameAct_set
 * Description:
 *      Set action for receive unknown type of CFM frame.
 * Input:
 *      unit   - unit id
 *      action - receive unknown type of CFM frame action
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
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_cfmUnknownFrameAct_set(uint32 unit, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmUnknownFrameAct_set(unit, action);
} /* end of rtk_trap_cfmUnknownFrameAct_set */

/* Function Name:
 *      rtk_trap_cfmLoopbackLinkTraceAct_get
 * Description:
 *      Get action for receive CFM loopback frame.
 * Input:
 *      unit    - unit id
 *      level   - MD level
 * Output:
 *      pAction - pointer buffer of action for receive CFM loopback frame
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
 *      loopback action combine with linktrace in 8390
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_cfmLoopbackLinkTraceAct_get(uint32 unit, uint32 level, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmLoopbackAct_get(unit, level, pAction);
} /* end of rtk_trap_cfmLoopbackLinkTraceAct_get */

/* Function Name:
 *      rtk_trap_cfmLoopbackLinkTraceAct_set
 * Description:
 *      Set action for receive CFM loopback frame.
 * Input:
 *      unit   - unit id
 *      level  - MD level
 *      action - receive CFM loopback frame action
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
 *      loopback action combine with linktrace in 8390
 *      Forwarding action is as following
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 *      - ACTION_TRAP2CPU
 */
int32
rtk_trap_cfmLoopbackLinkTraceAct_set(uint32 unit, uint32 level, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmLoopbackAct_set(unit, level, action);
} /* end of rtk_trap_cfmLoopbackLinkTraceAct_set */

/* Function Name:
 *      rtk_trap_cfmCcmAct_get
 * Description:
 *      Get action for receive CFM CCM frame.
 * Input:
 *      unit    - unit id
 *      level   - MD level
 * Output:
 *      pAction - pointer buffer of action for receive CFM CCM frame
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
 *      Set action to TRAP_OAM_ACTION_LINK_FAULT_DETECT to enable G.8032 link fault detection.
 *      Forwarding action is as following
 *      - TRAP_OAM_ACTION_FORWARD
 *      - TRAP_OAM_ACTION_DROP
 *      - TRAP_OAM_ACTION_TRAP2CPU
 *      - TRAP_OAM_ACTION_LINK_FAULT_DETECT
 */
int32
rtk_trap_cfmCcmAct_get(uint32 unit, uint32 level, rtk_trap_oam_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmCcmAct_get(unit, level, pAction);
} /* end of rtk_trap_cfmCcmAct_get */

/* Function Name:
 *      rtk_trap_cfmCcmAct_set
 * Description:
 *      Set action for receive CFM CCM frame.
 * Input:
 *      unit   - unit id
 *      level  - MD level
 *      action - receive CFM CCM frame action
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
 *      Set action to TRAP_OAM_ACTION_LINK_FAULT_DETECT to enable G.8032 link fault detection.
 *      Forwarding action is as following
 *      - TRAP_OAM_ACTION_FORWARD
 *      - TRAP_OAM_ACTION_DROP
 *      - TRAP_OAM_ACTION_TRAP2CPU
 *      - TRAP_OAM_ACTION_LINK_FAULT_DETECT
 */
int32
rtk_trap_cfmCcmAct_set(uint32 unit, uint32 level, rtk_trap_oam_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmCcmAct_set(unit, level, action);
} /* end of rtk_trap_cfmCcmAct_set */

/* Function Name:
 *      rtk_trap_cfmEthDmAct_get
 * Description:
 *      Get action for receive CFM ETH-DM frame in specified MD level.
 * Input:
 *      unit    - unit id
 *      mdl     - MD level
 * Output:
 *      pAction - pointer to action
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
 *      None
 */
int32
rtk_trap_cfmEthDmAct_get(uint32 unit, uint32 mdl, rtk_action_t *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmEthDmAct_get(unit, mdl, pAction);
} /* end of rtk_trap_cfmEthDmAct_get */

/* Function Name:
 *      rtk_trap_cfmEthDmAct_set
 * Description:
 *      Set action for receive CFM ETH-DM frame in specified MD level.
 * Input:
 *      unit   - unit id
 *      mdl    - MD level
 *      action - action
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
rtk_trap_cfmEthDmAct_set(uint32 unit, uint32 mdl, rtk_action_t action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_cfmEthDmAct_set(unit, mdl, action);
} /* end of rtk_oam_cfmEthDmAct_set */

/* Function Name:
 *      rtk_trap_portOamLoopbackParAction_get
 * Description:
 *      Get action of parser.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pAction - pointer to parser action
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
 *      1) Parser action takes effect only if 'rtk_trap_oamPDUAction_set' is set to ACTION_TRAP2CPU.
 *      2) Forwarding action is as following
 *         - TRAP_OAM_ACTION_FORWARD
 *         - TRAP_OAM_ACTION_DROP
 *         - TRAP_OAM_ACTION_LOOPBACK
 */
int32
rtk_trap_portOamLoopbackParAction_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_trap_oam_action_t   *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portOamLoopbackParAction_get(unit, port, pAction);
} /* end of rtk_trap_portOamLoopbackParAction_get */

/* Function Name:
 *      rtk_trap_portOamLoopbackParAction_set
 * Description:
 *      Set action of parser.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      action - parser action
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
 *      1) Parser action takes effect only if 'rtk_trap_oamPDUAction_set' is set to ACTION_TRAP2CPU.
 *      2) Forwarding action is as following
 *         - TRAP_OAM_ACTION_FORWARD
 *         - TRAP_OAM_ACTION_DROP
 *         - TRAP_OAM_ACTION_LOOPBACK
 */
int32
rtk_trap_portOamLoopbackParAction_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_trap_oam_action_t   action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_portOamLoopbackParAction_set(unit, port, action);
} /* end of rtk_trap_portOamLoopbackParAction_set */

/* Function Name:
 *      rtk_trap_routeExceptionAction_get
 * Description:
 *      Get the configuration of routing exception operation.
 * Input:
 *      unit    - unit id
 *      type    - configure action for which route exception
 * Output:
 *      pAction - pointer to route exception operation
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
 *      Forwarding action is as following
 *      - TRAP_OAM_ACTION_FORWARD
 *      - TRAP_OAM_ACTION_DROP
 *      - TRAP_OAM_ACTION_TRAP2CPU
 */
int32
rtk_trap_routeExceptionAction_get(
    uint32                          unit,
    rtk_trap_routeExceptionType_t   type,
    rtk_action_t                    *pAction)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_routeExceptionAction_get(unit, type, pAction);
} /* end of rtk_trap_routeExceptionAction_get */

/* Function Name:
 *      rtk_trap_routeExceptionAction_set
 * Description:
 *      Set the configuration of routing exception operation.
 * Input:
 *      unit   - unit id
 *      type   - configure action for which route exception
 *      action - route exception operation
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
 *      Forwarding action is as following
 *      - TRAP_OAM_ACTION_FORWARD
 *      - TRAP_OAM_ACTION_DROP
 *      - TRAP_OAM_ACTION_TRAP2CPU
 */
int32
rtk_trap_routeExceptionAction_set(
    uint32                          unit,
    rtk_trap_routeExceptionType_t   type,
    rtk_action_t                    action)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_routeExceptionAction_set(unit, type, action);
} /* end of rtk_trap_routeExceptionAction_set */

/* Function Name:
 *      rtk_trap_routeExceptionPri_get
 * Description:
 *      Get priority of route exception packets trapped to CPU.
 * Input:
 *      unit      - unit id
 *      type      - configure priority for which route exception
 * Output:
 *      pPriority - pointer to priority
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
 *      ROUTE_EXCEPTION_TYPE_GW_MAC_ERR and ROUTE_EXCEPTION_TYPE_ENTRY_AGE_OUT share the same priority
 *      setting. That is, configure ROUTE_EXCEPTION_TYPE_GW_MAC_ERR also affect ROUTE_EXCEPTION_TYPE_ENTRY_AGE_OUT.
 */
int32
rtk_trap_routeExceptionPri_get(
    uint32                          unit,
    rtk_trap_routeExceptionType_t   type,
    rtk_pri_t                       *pPriority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_routeExceptionPri_get(unit, type, pPriority);
} /* end of rtk_trap_routeExceptionPri_get */

/* Function Name:
 *      rtk_trap_routeExceptionPri_set
 * Description:
 *      Set priority of route exception packets trapped to CPU.
 * Input:
 *      unit     - unit id
 *      type     - configure priority for which route exception
 *      priority - priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_PRIORITY - invalid priority value
 * Applicable:
 *      8390
 * Note:
 *      ROUTE_EXCEPTION_TYPE_GW_MAC_ERR and ROUTE_EXCEPTION_TYPE_ENTRY_AGE_OUT share the same priority
 *      setting. That is, configure ROUTE_EXCEPTION_TYPE_GW_MAC_ERR also affect ROUTE_EXCEPTION_TYPE_ENTRY_AGE_OUT.
 */
int32
rtk_trap_routeExceptionPri_set(
    uint32                          unit,
    rtk_trap_routeExceptionType_t   type,
    rtk_pri_t                       priority)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_routeExceptionPri_set(unit, type, priority);
} /* end of rtk_trap_routeExceptionPri_set */

/* Function Name:
 *      rtk_trap_mgmtFrameMgmtVlanEnable_get
 * Description:
 *      Get status of comparing forwarding VID with P-VID of CPU port.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The configuration applies to IPv6 neighbor discovery(MGMT_TYPE_IPV6ND), ARP request(MGMT_TYPE_ARP)
 *          and Switch MAC(MGMT_TYPE_SELFMAC) packets.
 *      (2) The function should be enabled for management VLAN application and disabled for unmanagement
 *          VLAN application.
 */
int32
rtk_trap_mgmtFrameMgmtVlanEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameMgmtVlanEnable_get(unit, pEnable);
} /* end of rtk_trap_mgmtFrameMgmtVlanEnable_get */

/* Function Name:
 *      rtk_trap_mgmtFrameMgmtVlanEnable_set
 * Description:
 *      Set status of comparing forwarding VID with P-VID of CPU port.
 * Input:
 *      unit   - unit id
 *      enable - enable state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The configuration applies to IPv6 neighbor discovery(MGMT_TYPE_IPV6ND), ARP request(MGMT_TYPE_ARP)
 *          and Switch MAC(MGMT_TYPE_SELFMAC) packets.
 *      (2) The function should be enabled for management VLAN application and disabled for unmanagement
 *          VLAN application.
 */
int32
rtk_trap_mgmtFrameMgmtVlanEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameMgmtVlanEnable_set(unit, enable);
} /* end of rtk_trap_mgmtFrameMgmtVlanEnable_set */

/* Function Name:
 *      rtk_trap_mgmtFrameSelfARPEnable_get
 * Description:
 *      Get state of copy Self-ARP to CPU.
 * Input:
 *      unit   - unit id
 * Output:
 *      pEnable - enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The configuration only applies to ARP request(MGMT_TYPE_ARP) packet.
 *      (2) All the ARP Request packets are copied to CPU by setting rtk_trap_mgmtFrameAction_set(MGMT_TYPE_ARP).
 *          But if the function is enabled, Only ARP Request destined to switch's IP (rtk_switch_IPv4Addr_set)
 *          is copied to CPU.
 */
int32
rtk_trap_mgmtFrameSelfARPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameSelfARPEnable_get(unit, pEnable);
} /* end of rtk_trap_mgmtFrameSelfARPEnable_get */

/* Function Name:
 *      rtk_trap_mgmtFrameSelfARPEnable_set
 * Description:
 *      Set state of copy Self-ARP to CPU.
 * Input:
 *      unit   - unit id
 *      enable - enable state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - Invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The configuration only applies to ARP request(MGMT_TYPE_ARP) packet.
 *      (2) All the ARP Request packets are copied to CPU by setting rtk_trap_mgmtFrameAction_set(MGMT_TYPE_ARP).
 *          But if the function is enabled, Only ARP Request destined to switch's IP (rtk_switch_IPv4Addr_set)
 *          is copied to CPU.
 */
int32
rtk_trap_mgmtFrameSelfARPEnable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_mgmtFrameSelfARPEnable_set(unit, enable);
} /* end of rtk_trap_mgmtFrameSelfARPEnable_set */

/* Function Name:
 *      rtk_trap_bpduFloodPortmask_get
 * Description:
 *      Get BPDU flooding portmask which used while BPDU action is flood to all port
 * Input:
 *      unit            - unit id
 *      pflood_portmask - BPDU flood portmask
 * Output:
 *      pEnable         - status of trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The configuration applies while BPDU packet lookup missed and action is set to flood to all port.
 */
int32
rtk_trap_bpduFloodPortmask_get(uint32 unit,  rtk_portmask_t *pflood_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_bpduFloodPortmask_get(unit, pflood_portmask);
}/* end of rtk_trap_bpduFloodPortmask_get */

/* Function Name:
 *      rtk_trap_bpduFloodPortmask_set
 * Description:
 *      Set BPDU flooding portmask which used while BPDU action is flood to all port
 * Input:
 *      unit            - unit id
 *      pflood_portmask - BPDU flood portmask
 * Output:
 *      pEnable         - status of trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - Invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The configuration applies while BPDU packet lookup missed and action is set to flood to all port.
 */
int32
rtk_trap_bpduFloodPortmask_set(uint32 unit,  rtk_portmask_t *pflood_portmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->trap_bpduFloodPortmask_set(unit, pflood_portmask);
}/* end of rtk_trap_bpduFloodPortmask_set */

/* Function Name:
 *      rtk_trap_rmaLookupMissActionEnable_get
 * Description:
 *      Get enable status of RMA care lookup miss action.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer to enable status of RMA care lookup miss action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Enable is care lookup miss action.
 */
int32
rtk_trap_rmaLookupMissActionEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->trap_rmaLookupMissActionEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->trap_rmaLookupMissActionEnable_get(unit, pEnable);
}   /* end of rtk_trap_rmaLookupMissActionEnable_get */

/* Function Name:
 *      rtk_trap_rmaLookupMissActionEnable_set
 * Description:
 *      Set enable status of RMA care lookup miss action.
 * Input:
 *      unit    - unit id
 *      enable  - enable status of RMA care lookup miss action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Enable is care lookup miss action.
 */
int32
rtk_trap_rmaLookupMissActionEnable_set(uint32 unit, rtk_enable_t enable)
{
    /* function body */
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    if (NULL == RT_MAPPER(unit)->trap_rmaLookupMissActionEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    return RT_MAPPER(unit)->trap_rmaLookupMissActionEnable_set(unit, enable);
}   /* end of rtk_trap_rmaLookupMissActionEnable_set */

