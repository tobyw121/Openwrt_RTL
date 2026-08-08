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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : Definition those public Trap APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) Packets trap to CPU setting.
 *            2) RMA (Reserved MAC address).
 *
 */
#ifndef __DAL_SSW_TRAP_H__ 
#define __DAL_SSW_TRAP_H__

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/trap.h>

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

/* Function Name:
 *      dal_ssw_trap_init
 * Description:
 *      Initial the trap module of the specified device..
 * Input:
 *      unit     - unit id
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
extern int32
dal_ssw_trap_init(uint32 unit);

/* Function Name:
 *      dal_ssw_trap_1xMacChangePort2CpuEnable_get
 * Description:
 *      Get the configuration about when 802.1x MAC-based authenticated MAC address changes port
 *      whether it need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of authenticated MAC address change port trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of authenticated MAC address change port trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_1xMacChangePort2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_trap_1xMacChangePort2CpuEnable_set
 * Description:
 *      Set the configuration about when 802.1x MAC-based authenticated MAC address changes port
 *      whether it need be trapped to CPU.
 * Input:
 *      unit            - unit id
 *      enable          - status of authenticated MAC address change port trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID   - Invalid unit id
 *      RT_ERR_INPUT     - Invalid input parameter
 * Note:
 *      The status of authenticated MAC address change port trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_1xMacChangePort2CpuEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_trap_igmpCtrlPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether IGMP control packets need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of IGMP control packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of IGMP control packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_igmpCtrlPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_trap_igmpCtrlPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether IGMP control packets need be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      enable              - status of IGMP control packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT     - Invalid input parameter
 * Note:
 *      The status of IGMP control packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_igmpCtrlPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_trap_l2McastPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether L2 multicast packets lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of L2 multicast packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of L2 multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_l2McastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_trap_l2McastPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether L2 multicast packets lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      enable              - status of L2 multicast packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      The status of L2 multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_l2McastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_trap_ipMcastPkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether IP multicast packet lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 * Output:
 *      pEnable            - status of IP multicast packet trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      The status of IP multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_ipMcastPkt2CpuEnable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_trap_ipMcastPkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether IP multicast packet lookup miss need be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      enable              - status of IP multicast packet trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      The status of IP multicast packet trap to CPU:
 *      - DISABLED
 *      - ENABLED
 */
extern int32
dal_ssw_trap_ipMcastPkt2CpuEnable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      dal_ssw_trap_rmaAction_get
 * Description:
 *      Get action of reserved multicast address(RMA) frame.
 * Input:
 *      unit                - unit id
 *      pRma_frame         - Reserved multicast address.
 * Output:
 *      pRma_action        - RMA action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_NULL_POINTER  - NULL pointer
 * Note:
 *      None.
 */
extern int32
dal_ssw_trap_rmaAction_get(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action);

/* Function Name:
 *      dal_ssw_trap_rmaAction_set
 * Description:
 *      Set action of reserved multicast address(RMA) frame.
 * Input:
 *      unit                - unit id
 *      pRma_frame         - Reserved multicast address.
 *      rma_action          - RMA action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - Invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 *      RT_ERR_RMA_ACTION    - Invalid RMA action
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
dal_ssw_trap_rmaAction_set(uint32 unit, rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action);

/* Function Name:
 *      dal_ssw_trap_reasonTrapToCPUPriority_get
 * Description:
 *      Get priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      unit                - unit id
 *      type                - reason that trap to CPU port.
 * Output:
 *      pPriority          - configured internal priority for such reason.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      TRAP_REASON_RMA
 *      TRAP_REASON_IPV4IGMP,
 *      TRAP_REASON_IPV6MLD,
 *      TRAP_REASON_1XEAPOL,
 *      TRAP_REASON_VLANERR,
 *      TRAP_REASON_SLPCHANGE,
 *      TRAP_REASON_MULTICASTDLF,
 *      TRAP_REASON_CFI,
 *      TRAP_REASON_1XUNAUTH.
 */
extern int32
dal_ssw_trap_reasonTrapToCPUPriority_get(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t *pPriority);

/* Function Name:
 *      dal_ssw_trap_reasonTrapToCPUPriority_set
 * Description:
 *      Set priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      unit                - unit id
 *      type                - reason that trap to CPU port.
 *      priority            - internal priority that is going to be set for specific trap reason.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - Invalid input parameter
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      TRAP_REASON_RMA
 *      TRAP_REASON_IPV4IGMP,
 *      TRAP_REASON_IPV6MLD,
 *      TRAP_REASON_1XEAPOL,
 *      TRAP_REASON_VLANERR,
 *      TRAP_REASON_SLPCHANGE,
 *      TRAP_REASON_MULTICASTDLF,
 *      TRAP_REASON_CFI,
 *      TRAP_REASON_1XUNAUTH.
 */
extern int32
dal_ssw_trap_reasonTrapToCPUPriority_set(uint32 unit, rtk_trap_reason_type_t type, rtk_pri_t priority);

/* Function Name:
 *      dal_ssw_trap_pkt2CpuEnable_get
 * Description:
 *      Get the configuration about whether specific packet type needed to be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      type                - packet type that trapped to CPU port.      
 * Output:
 *      pEnable             - status of special packet type trap to CPU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      Currently the trap packet type that supported are listed as follows:
 *      TRAP_TYPE_1XMAC_PORTCHG
 *      TRAP_TYPE_IPV4_IGMP       
 *      TRAP_TYPE_IPMC_DLF         
 *      TRAP_TYPE_L2MC_DLF          
 *      TRAP_TYPE_IPV6_MLD         
 *      TRAP_TYPE_CFI_1 
 */
extern int32
dal_ssw_trap_pkt2CpuEnable_get(uint32 unit, rtk_trap_type_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_ssw_trap_pkt2CpuEnable_set
 * Description:
 *      Set the configuration about whether specific packet type needed to be trapped to CPU.
 * Input:
 *      unit                - unit id
 *      type                - packet type that trapped to CPU port.
 *      enable              - status of specific packet type trap to CPU
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT         - Invalid input parameter
 * Note:
 *      Currently the trap packet type that supported are listed as follows:
 *      TRAP_TYPE_1XMAC_PORTCHG
 *      TRAP_TYPE_IPV4_IGMP       
 *      TRAP_TYPE_IPMC_DLF         
 *      TRAP_TYPE_L2MC_DLF          
 *      TRAP_TYPE_IPV6_MLD         
 *      TRAP_TYPE_CFI_1 
 */
extern int32
dal_ssw_trap_pkt2CpuEnable_set(uint32 unit, rtk_trap_type_t type, rtk_enable_t enable);

#endif /* __DAL_SSW_TRAP_H__ */
