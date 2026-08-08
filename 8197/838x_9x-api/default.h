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
 * $Revision: 46712 $
 * $Date: 2014-03-04 16:25:56 +0800 (Tue, 04 Mar 2014) $
 *
 * Purpose : Definition of the default values in the SDK
 *
 * Feature : Define the default values 
 *
 */

#ifndef __RTK_DEFAULT_H__
#define __RTK_DEFAULT_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h> 
#include <common/rt_type.h>
#include <rtk/trunk.h>
#include <rtk/vlan.h>
#include <rtk/stp.h>
#include <rtk/qos.h>
#include <rtk/filter.h>
#include <rtk/l2.h>

/*
 * Symbol Definition
 */

/* VLAN module */
#define RTK_DEFAULT_VLAN_ID                         1
#define RTK_DEFAULT_VLAN_FUNCTION                   ENABLED
#define RTK_DEFAULT_VLAN_TAGMODE                    VLAN_TAG_MODE_ORIGINAL
#define RTK_DEFAULT_VLAN_TAGMODE_IN_CPU             VLAN_TAG_MODE_ORIGINAL
#define RTK_DEFAULT_VLAN_IGNORE_INNER_TAG           DISABLED
#define RTK_DEFAULT_VLAN_IGNORE_OUTER_TAG           DISABLED
#define RTK_DEFAULT_VLAN_EGRESS_FILTER_ENABLE       ENABLED

/* STP module */
#define RTK_DEFAULT_MSTI                            0
#define RTK_DEFAULT_STP_PORT_STATE                  STP_STATE_FORWARDING

/* L2 module */
#define RTK_DEFAULT_L2_FLUSH_LINKDOWN_MAC           DISABLED
#define RTK_DEFAULT_L2_IP6_CACHE_TBL_SIZE           512
#define RTK_DEFAULT_L2_NON_UCAST_SA_ACTION          ACTION_DROP
#define RTK_DEFAULT_L2_NEW_SA_ACTION                ACTION_FORWARD
#define RTK_DEFAULT_L2_NEW_SA_LEARN_MODE            HARDWARE_LEARNING
#define RTK_DEFAULT_L2_CPU_NEW_SA_LEARN_MODE        NOT_LEARNING
#define RTK_DEFAULT_L2_IP4_MCAST_DIP_VALIDATION_STATE ENABLED
#define RTK_DEFAULT_L2_IP_MCAST_IVL_LOOKUP_STATE    ENABLED
#define RTK_DEFAULT_L2_IP6_MCAST_LOOKUP_SIP_CARE_BYTE 0x5210
#define RTK_DEFAULT_L2_IP6_MCAST_LOOKUP_DIP_CARE_BYTE 0x3210
#define RTK_DEFAULT_L2_SA_BLOCK_PORT_STATE          ENABLED
#define RTK_DEFAULT_L2_SA_BLOCK_CPU_PORT_STATE      DISABLED
#define RTK_DEFAULT_L2_DA_BLOCK_PORT_STATE          ENABLED
#define RTK_DEFAULT_L2_DA_BLOCK_CPU_PORT_STATE      DISABLED

#if defined(CONFIG_SDK_RTL8390)
#define RTK_DEFAULT_L2_IP6_CACHE_IDX_TBL_SIZE       (16384+64)  /* Applicable to 8390 */
#else
#define RTK_DEFAULT_L2_IP6_CACHE_IDX_TBL_SIZE       (8192+64)   /* Applicable to 8380 */
#endif

/* PORT module */
#define RTK_DEFAULT_PORT_AUTONEGO_ENABLE            ENABLED
#define RTK_DEFAULT_PORT_10HALF_CAPABLE             ENABLED
#define RTK_DEFAULT_PORT_10FULL_CAPABLE             ENABLED
#define RTK_DEFAULT_PORT_100HALF_CAPABLE            ENABLED
#define RTK_DEFAULT_PORT_100FULL_CAPABLE            ENABLED
#define RTK_DEFAULT_PORT_1000HALF_CAPABLE           ENABLED
#define RTK_DEFAULT_PORT_1000FULL_CAPABLE           ENABLED
#define RTK_DEFAULT_PORT_PAUSE_CAPABILITY           ENABLED
#define RTK_DEFAULT_PORT_ASYPAUSE_CAPABILITY        DISABLED
#define RTK_DEFAULT_PORT_ISOLATION_PORTMASK         0x1FFFFFFF
#define RTK_DEFAULT_PORT_TX_ENABLE                  ENABLED
#define RTK_DEFAULT_PORT_RX_ENABLE                  ENABLED
#define RTK_DEFAULT_PORT_ADMIN_ENABLE               DISABLED
#define RTK_DEFAULT_PORT_VID                        RTK_DEFAULT_VLAN_ID

/* TRUNK module */
#define RTK_DEFAULT_TRUNK_MEMBER_PORTMASK           0
#define RTK_DEFAULT_TRUNK_DISTRIBUTION_BIND         0
#define RTK_DEFAULT_TRUNK_DISTRIBUTION_ALGORITHM    TRUNK_DISTRIBUTION_ALGO_MASKALL
#define RTK_DEFAULT_TRUNK_MODE                      TRUNK_MODE_NORMAL

/* TRAP module */
#define RTK_DEFAULT_TRAP_RMA_SP_MCAST_ACTION        ACTION_FORWARD

/* RATE module */
#define RTK_DEFAULT_EGR_BANDWIDTH_CTRL_STATUS       DISABLED
#define RTK_DEFAULT_EGR_BANDWIDTH_CTRL_RATE         0xFFFF
#define RTK_DEFAULT_EGR_BANDWIDTH_CPU_PORT_RATE_MODE     RATE_MODE_PKT
#define RTK_DEFAULT_STORM_CONTROL_RATE_MODE         BASED_ON_BYTE
#define RTK_DEFAULT_METER_IFG_INCLUDE_STATE         ENABLED

/* FILTER module */
#ifdef CONFIG_SDK_FPGA_PLATFORM
#define RTK_DEFAULT_CUTLINE                         1
#else
#define RTK_DEFAULT_CUTLINE                         4
#endif
#define RTK_DEFAULT_PIE_STATUS                      ENABLED
#define RTK_DEFAULT_PIE_MODE                        REALTEK_MODE


/* QoS module */
#ifdef CONFIG_SDK_FPGA_PLATFORM
#define RTK_DEFAULT_QOS_QUEUE_NUMBER                4
#define RTK_DEFAULT_QOS_QUEUE_NUMBER_IN_CPU_PORT    4
#else
#define RTK_DEFAULT_QOS_QUEUE_NUMBER                8
#define RTK_DEFAULT_QOS_QUEUE_NUMBER_IN_CPU_PORT    8
#endif

#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_PORT          1
#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_CLASS         1
#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_ACL           4
#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_DSCP          1
#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_ITAG          2
#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_OTAG          3
#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_MAC_VLAN      1
#define RTK_DEFAULT_QOS_SELECTION_PRIORITY_OF_PROTO_VLAN    1

#define RTK_DEFAULT_QOS_PORT_PRIORITY               0
 
#define RTK_DEFAULT_QOS_1P_PRIORITY0_REMAP          1
#define RTK_DEFAULT_QOS_1P_PRIORITY1_REMAP          0
#define RTK_DEFAULT_QOS_1P_PRIORITY2_REMAP          2
#define RTK_DEFAULT_QOS_1P_PRIORITY3_REMAP          3
#define RTK_DEFAULT_QOS_1P_PRIORITY4_REMAP          4
#define RTK_DEFAULT_QOS_1P_PRIORITY5_REMAP          5
#define RTK_DEFAULT_QOS_1P_PRIORITY6_REMAP          6
#define RTK_DEFAULT_QOS_1P_PRIORITY7_REMAP          7
 
#define RTK_DEFAULT_QOS_PRIORITY0_QUEUE             0
#define RTK_DEFAULT_QOS_PRIORITY1_QUEUE             1
#define RTK_DEFAULT_QOS_PRIORITY2_QUEUE             2
#define RTK_DEFAULT_QOS_PRIORITY3_QUEUE             3
#define RTK_DEFAULT_QOS_PRIORITY4_QUEUE             4
#define RTK_DEFAULT_QOS_PRIORITY5_QUEUE             5
#define RTK_DEFAULT_QOS_PRIORITY6_QUEUE             6
#define RTK_DEFAULT_QOS_PRIORITY7_QUEUE             7
 
#define RTK_DEFAULT_QOS_SCHED_ALGORITHM             WFQ
#define RTK_DEFAULT_QOS_SCHED_QUEUE_TYPE            WFQ_WRR_PRIORITY
#define RTK_DEFAULT_QOS_SCHED_QUEUE0_WEIGHT         1
#define RTK_DEFAULT_QOS_SCHED_QUEUE1_WEIGHT         1
#define RTK_DEFAULT_QOS_SCHED_QUEUE2_WEIGHT         1
#define RTK_DEFAULT_QOS_SCHED_QUEUE3_WEIGHT         1
#define RTK_DEFAULT_QOS_SCHED_QUEUE4_WEIGHT         1
#define RTK_DEFAULT_QOS_SCHED_QUEUE5_WEIGHT         1
#define RTK_DEFAULT_QOS_SCHED_QUEUE6_WEIGHT         1
#define RTK_DEFAULT_QOS_SCHED_QUEUE7_WEIGHT         1

#define RTK_DEFAULT_QOS_SCHED_LB_BYTE_PER_TOKEN     0x3B
#define RTK_DEFAULT_QOS_SCHED_LB_TICK_PERIOD        0x4B
#define RTK_DEFAULT_QOS_SCHED_LB_PREIFP             1
#define RTK_DEFAULT_QOS_SCHED_LB_WFQ_HIGH_THRESHOLD 0x300
#define RTK_DEFAULT_QOS_SCHED_LB_APR_BURST_SIZE     0xC000
#define RTK_DEFAULT_QOS_SCHED_LB_APR_RATE           0xFFFF

#define RTK_DEFAULT_QOS_REMAP_DEI0_TO_DP            0
#define RTK_DEFAULT_QOS_REMAP_DEI1_TO_DP            2

#define RTK_DEFAULT_QOS_REMARK_DEI_SOURCE           DEI_SEL_OUTER_TAG
#define RTK_DEFAULT_QOS_REMARK_DP0_TO_DEI           0
#define RTK_DEFAULT_QOS_REMARK_DP1_TO_DEI           1
#define RTK_DEFAULT_QOS_REMARK_DP2_TO_DEI           1

/* NIC module */
/* Configure CPU Tag in packet descriptor or in packet raw data
 * ENABLED: in packet descriptor
 * DISABLED: in packet raw data
 */
#define RTK_DEFAULT_NIC_CPU_TAG_IN_DESCRIPTOR               ENABLED

/* Configure CPU Tag parser ability in cpu port
 * ENABLED: have parser cpu tag ability.
 * DISABLED: no parser cpu tag ability.
 */
#define RTK_DEFAULT_CPU_TAG_PARSER_ABILITY_IN_CPU_PORT      ENABLED

/* Configure insert all packets leaving CPU port with CPU tag
 * ENABLED: insert to raw data.
 * DISABLED: no insert.
 */
#define RTK_DEFAULT_CPU_TAG_INSERT_IN_LEAVING_CPU_PORT_PKT  ENABLED

/* RTK Link Monitor Thread Configuration
 */
#define RTK_DEFAULT_LINK_MON_POLLING_STACK_SIZE 4096
#define RTK_DEFAULT_LINK_MON_POLLING_THREAD_PRI 3
#define RTK_DEFAULT_LINK_MON_ISR_STACK_SIZE 4096
#define RTK_DEFAULT_LINK_MON_ISR_THREAD_PRI 0

/* RTK LOS Monitor Thread Configuration
 */
#define RTK_DEFAULT_LOS_MON_STACK_SIZE 4096
#define RTK_DEFAULT_LOS_MON_THREAD_PRI 3

/* RTK Workaround Monitor Thread Configuration
 */
#define RTK_DEFAULT_WA_MON_STACK_SIZE 4096
#define RTK_DEFAULT_WA_MON_THREAD_PRI 3

/* RTK Mac Constraint Workaround Monitor Thread Configuration
 */
#define RTK_DEFAULT_MAC_MON_STACK_SIZE 4096
#define RTK_DEFAULT_MAC_MON_THREAD_PRI 2

/* TIME module */
#define RTK_DEFAULT_TIME_ENABLE       DISABLED

/*
 * Data Declaration
 */


/*
 * Macro Definition
 */


#endif /* __RTK_DEFAULT_H__ */

