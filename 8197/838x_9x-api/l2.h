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
 * $Revision: 54452 $
 * $Date: 2014-12-30 14:41:26 +0800 (Tue, 30 Dec 2014) $
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

#ifndef __RTK_L2_H__
#define __RTK_L2_H__


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */
/* Definition for flags in rtk_l2_ucastAddr_t */
#define RTK_L2_UCAST_FLAG_TRUNK_PORT        0x00000001
#define RTK_L2_UCAST_FLAG_SA_BLOCK          0x00000002
#define RTK_L2_UCAST_FLAG_DA_BLOCK          0x00000004
#define RTK_L2_UCAST_FLAG_STATIC            0x00000008
#define RTK_L2_UCAST_FLAG_NEXTHOP           0x00000010
#define RTK_L2_UCAST_FLAG_SA_SECURE         RTK_L2_UCAST_FLAG_SA_BLOCK

/* Definition for state in rtk_l2_ucastAddr_t */
#define RTK_L2_UCAST_STATE_SUSPEND          0x00000001

/* the flag is used in add function for replace first dynamic entry
 * when free entry is unavailable. If free entry is unavailable and 
 * first dynamic entry is not exist, the replace action will be ignore 
 */
#define RTK_L2_ADD_OP_FLAG_REPLACE_DYNAMIC  0x00000001

/*
 * Data Declaration
 */
/* l2 flush type - chip ability */
typedef enum rtk_l2_flushType_e
{
    FLUSH_TYPE_BY_VID = 0,
    FLUSH_TYPE_BY_PORT,         /* physical port        */
    FLUSH_TYPE_BY_VID_PORT,     /* vid + physical port  */
    FLUSH_TYPE_BY_TID,          /* trunk id             */
    FLUSH_TYPE_BY_VID_TID,      /* vid + trunk id       */
    FLUSH_TYPE_END
} rtk_l2_flushType_t;

/* l2 flush type - chip ability */
typedef enum rtk_l2_flushAct_e
{
    FLUSH_ACT_FLUSH = 0,
    FLUSH_ACT_REPLACE,          /* Applicable to 8390, 8380 */
    FLUSH_ACT_END
} rtk_l2_flushAct_t;

/* l2 address flow type */
typedef enum rtk_l2_flowType_e
{
    FLOW_TYPE_UNICAST = 0,
    FLOW_TYPE_L2_MULTI,
    FLOW_TYPE_IP4_MULTI,
    FLOW_TYPE_IP6_MULTI,        /* Applicable to 8390 */
    FLOW_TYPE_END
} rtk_l2_flowType_t;

/* l2 limit learning count action */
typedef enum rtk_l2_limitLearnCntAction_e
{
    LIMIT_LEARN_CNT_ACTION_DROP = ACTION_DROP,
    LIMIT_LEARN_CNT_ACTION_FORWARD = ACTION_FORWARD,
    LIMIT_LEARN_CNT_ACTION_TO_CPU = ACTION_TRAP2CPU,
    LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU = ACTION_COPY2CPU,/* Applicable to 8390, 8380 */
    LIMIT_LEARN_CNT_ACTION_END
} rtk_l2_limitLearnCntAction_t;

/* Vlan for L2 lookup */
typedef enum rtk_l2_vlanMode_e
{
    BASED_ON_INNER_VLAN = 0,
    BASED_ON_OUTER_VLAN,
    FWD_VLAN_MODE_END
} rtk_l2_vlanMode_t;

/* L2 unicast lookup mode */
typedef enum rtk_l2_ucastLookupMode_e
{
    UC_LOOKUP_ON_VID = 0,
    UC_LOOKUP_ON_FID,
    UC_LOOKUP_END
} rtk_l2_ucastLookupMode_t;

/* New unicast MAC address learning mode */
typedef enum rtk_l2_newMacLrnMode_e
{
    HARDWARE_LEARNING = 0,
    SOFTWARE_LEARNING,
    NOT_LEARNING,
    LEARNING_MODE_END,
} rtk_l2_newMacLrnMode_t;

/* L2 multicast lookup mode */
typedef enum rtk_l2_mcastLookupMode_e
{
    MC_LOOKUP_ON_VID = 0,
    MC_LOOKUP_ON_FID,
    MC_LOOKUP_ON_FIXED_FID,         /* Applicable to 8328 */
    MC_LOOKUP_END
} rtk_l2_mcastLookupMode_t;

/* L2 IP multicast switch mode */
typedef enum rtk_l2_ipmcMode_e
{
    LOOKUP_ON_FVID_AND_MAC = 0,     /* Applicable to 8328, 8390, 8380 */
    LOOKUP_ON_DIP_AND_SIP,          /* Applicable to 8389, 8328, 8390, 8380 */
    LOOKUP_ON_DIP_AND_FVID,         /* Applicable to 8390, 8380 */
    LOOKUP_ON_DIP_ONLY,             /* Applicable to 8328 */
    IPMC_MODE_END
} rtk_l2_ipmcMode_t;

/* type of exception address */
typedef enum rtk_l2_exceptionAddrType_e
{
    SA_IS_MCAST = 0,                /* Applicable to 8328 */
    SA_IS_BCAST,                    /* Applicable to 8328 */
    SA_IS_ZERO,                     /* Applicable to 8328, 8390, 8380*/
    SA_IS_BCAST_OR_MCAST,           /* Applicable to 8390, 8380*/
    EXCEPT_ADDR_TYPE_END
} rtk_l2_exceptionAddrType_t;

/* Type of L2 IP multicast DA mismatch */
typedef enum rtk_l2_ipmc_mismatchType_e
{
    L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR = 0,
    L2_IPMC_MIS_TYPE_UCAST_ADDR,
    L2_IPMC_MIS_TYPE_END
} rtk_l2_ipmc_mismatchType_t;

/* Action of L2 IP multicast DA mismatch */
typedef enum rtk_l2_ipmcMismatch_action_e
{
    L2_IPMC_MIS_DROP = 0,
    L2_IPMC_MIS_TRAP,
    L2_IPMC_MIS_FWD_AS_L2,
    L2_IPMC_MIS_FWD_AS_IPMC,
    L2_IPMC_MIS_END
} rtk_l2_ipmcMismatch_action_t;

/* Type of lookup miss */
typedef enum rtk_l2_lookupMissType_e
{
    DLF_TYPE_IPMC = 0,      /* Applicable to 8328, 8390, 8380 */
    DLF_TYPE_IP6MC,         /* Applicable to 8328, 8390, 8380 */
    DLF_TYPE_UCAST,         /* Applicable to 8328, 8390, 8380 */
    DLF_TYPE_BCAST,         /* Applicable to 8328, 8390, 8380 */
    DLF_TYPE_MCAST,         /* Applicable to 8328, 8390, 8380 */
    DLF_TYPE_ANY,           /* Applicable to 8389 */
    DLF_TYPE_END
} rtk_l2_lookupMissType_t;

typedef enum rtk_l2_macFilterMode_e
{
    MAC_FILTER_MODE_SA = 0,                /* Applicable to 8390 */
    MAC_FILTER_MODE_DA,                    /* Applicable to 8390 */
    MAC_FILTER_MODE_END
} rtk_l2_macFilterMode_t;


/* l2 address table - unicast data structure */
typedef struct 
rtk_l2_ucastAddr_s
{
    rtk_vlan_t  vid;
    rtk_mac_t   mac;
    rtk_port_t  port;
    uint32      trk_gid;        /* The field is valid if flags enable RTK_L2_UCAST_FLAG_TRUNK_PORT bit */
    uint32      flags;          /* Refer to RTK_L2_UCAST_FLAG_XXX */
    uint32      state;          /* Refer to RTK_L2_UCAST_STATE_XXX */
    uint32      l2_idx;         /* returned l2 entry index after a successful read/write */
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    uint8       auth;
#endif
    uint32      isAged;         /* TRUE: Aged, FALSE: Not Aged */
    uint32      add_op_flags;   /* Refer to RTK_L2_ADD_OP_FLAG_XXX */
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    rtk_vlan_t  agg_vid;        /* The field is auto-learned by H/W and utilized by MAC-based N:1 VLAN translation */
    uint8       vlan_target;    /* Target VLAN to swap. 0:inner 1:outer 
                                 * The field is used only if RTK_L2_UCAST_FLAG_NEXTHOP is set.
                                 */
    uint32      route_idx;      /* Index to routing table.
                                 * The field is used only if RTK_L2_UCAST_FLAG_NEXTHOP is set.
                                 */
#endif
} rtk_l2_ucastAddr_t;

/* l2 address table - multicast data structure */
typedef struct rtk_l2_mcastAddr_s
{
    uint16          rvid;       /* hash key */
    rtk_mac_t       mac;        /* hash key */
    rtk_portmask_t  portmask;
    int32           fwdIndex;   /* index of forwarding table, -1 means auto allocation */
#if defined(CONFIG_SDK_RTL8328)
    uint32          isAged;
                    /* TRUE: Aged
                     * FALSE: Not Aged
                     */
    rtk_enable_t    crossVlan;
#endif
    uint32          l2_idx; /* return the l2 entry index of l2 table after write success */
    uint32          add_op_flags; /* Refer to RTK_L2_ADD_OP_FLAG_XXX */
#if defined(CONFIG_SDK_RTL8380)
    rtk_vlan_t      agg_vid;    /* MAC-based N:1 VLAN translation */
#endif
} rtk_l2_mcastAddr_t;

/* l2 address table - ip multicast data structure */
typedef struct rtk_l2_ipMcastAddr_s
{
    uint16          rvid;       /* hash key */
    ipaddr_t        dip;        /* hash key */
    ipaddr_t        sip;        /* hash key */
    rtk_portmask_t  portmask;
    int32           fwdIndex;   /* index of forwarding table, -1 means auto allocation */
#if defined(CONFIG_SDK_RTL8328)
    uint32          isAged;
                    /* TRUE: Aged
                     * FALSE: Not Aged
                     */
    rtk_enable_t    crossVlan;
#endif
    uint32          l2_idx; /* return the l2 entry index of l2 table after write success */
    uint32          add_op_flags; /* Refer to RTK_L2_ADD_OP_FLAG_XXX */
} rtk_l2_ipMcastAddr_t;

typedef struct rtk_l2_ipMcastHashKey_s
{
    uint16          rvid;
    ipaddr_t        dip;
    ipaddr_t        sip;
} rtk_l2_ipMcastHashKey_t;

/* l2 address table - ipv6 multicast data structure */
typedef struct rtk_l2_ip6McastAddr_s
{
    uint16          rvid;       /* hash key */
    rtk_ipv6_addr_t dip;        /* hash key */
    rtk_ipv6_addr_t sip;        /* hash key */
    rtk_portmask_t  portmask;
    int32           fwdIndex;   /* index of forwarding table, -1 means auto allocation */
    uint32          l2_idx; /* return the l2 entry index of l2 table after write success */
    uint32          add_op_flags; /* Refer to RTK_L2_ADD_OP_FLAG_XXX */
} rtk_l2_ip6McastAddr_t;

typedef struct rtk_l2_ip6McastHashKey_s
{
    uint16          rvid;
    rtk_ipv6_addr_t dip;
    rtk_ipv6_addr_t sip;
} rtk_l2_ip6McastHashKey_t;

/* l2 address table - unicast, multicast, ip multicast data structure */
typedef struct rtk_l2_entry_s
{
    rtk_l2_flowType_t entry_type; /* unicast, l2 multicast, ip multicast */
    uint32          valid;
    union
    {
        rtk_l2_ucastAddr_t      unicast;
        rtk_l2_mcastAddr_t      l2mcast;
        rtk_l2_ipMcastAddr_t    ipmcast;
        rtk_l2_ip6McastAddr_t  ip6mcast;
    };
} rtk_l2_entry_t;

/* l2 address get method */
typedef enum rtk_l2_nextValidType_e
{
    L2_NEXT_VALID_TYPE_CURRENT = 0,
    L2_NEXT_VALID_TYPE_DYNAMIC_UC = 1,
    L2_NEXT_VALID_TYPE_DYNAMIC_UC_AND_MC = 2,
    L2_NEXT_VALID_TYPE_UC_AND_MC = 3,
    L2_NEXT_VALID_TYPE_END
}rtk_l2_nextValidType_t;



typedef struct rtk_l2_flushCfg_s
{
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    rtk_l2_flushAct_t   act;
#endif
    rtk_enable_t    flushByVid;
    rtk_vlan_t      vid;
    rtk_enable_t    flushByPort;
    rtk_enable_t    portOrTrunk;
                    /* ENABLED: Flush by port
                     * DISABLED: Flush by trunk
                     */
    rtk_port_t      port;           /* port to be flushed or replaced */
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    rtk_port_t      replacingPort;  /* new port to replace */
#endif
    rtk_enable_t    flushStaticAddr;
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    rtk_enable_t    flushByMac;
    rtk_mac_t       ucastAddr;
    rtk_enable_t    flushAddrOnAllPorts; /* this is used when flushByVid */
#endif
} rtk_l2_flushCfg_t;

typedef struct rtk_l2_fidMacLimitEntry_s
{
#if defined(CONFIG_SDK_RTL8328)
    rtk_enable_t                    enable;     /* enable mac limit */
#endif
    rtk_fid_t                       fid;
    uint32                          maxNum;
#if defined(CONFIG_SDK_RTL8390)  || defined(CONFIG_SDK_RTL8380)
    rtk_port_t                      port;
#endif
#if defined(CONFIG_SDK_RTL8328)
    rtk_portmask_t                  portmask;
    /* forwarding action when number of MAC address over limitation */
    rtk_l2_limitLearnCntAction_t    action;
#endif
} rtk_l2_fidMacLimitEntry_t;

typedef enum l2_ipmcst_hashKeyFmt_e
{
    L2_FVID_MAC = 0,
    L2_SIP_GIP,
    L2_FVID_GIP,
    L2_HASHKEYFMT_END
} rtk_l2_ipmcst_hashKeyFmt_t;

typedef enum rtk_l2_ip6_careByte_type_e
{
    L2_DIP_HASH_CARE_BYTE = 0,
    L2_SIP_HASH_CARE_BYTE,
    L2_CARE_BYTE_END
} rtk_l2_ip6_careByte_type_t;

typedef enum rtk_l2_notifyFifoStatus_e
{
    L2_NOTIFY_FIFO_EMPTY = 0,
    L2_NOTIFY_FIFO_NOT_EMPTY,
    L2_NOTIFY_FIFO_END
} rtk_l2_notifyFifoStatus_t;

typedef enum rtk_l2_notifyBufferType_e
{
    L2_NOTIFY_BUF_NBUF = 0,
    L2_NOTIFY_BUF_ASIC_FIFO,
    L2_NOTIFY_BUF_END
} rtk_l2_notifyBufferType_t;

typedef enum rtk_l2_notifyType_e
{
    L2_NOTIFY_TYPE_AGEOUT_LINKDOWNFLUSH_PORTMOVEINVALID = 0,
    L2_NOTIFY_TYPE_NEW_LEARN,
    L2_NOTIFY_TYPE_PORTMOVE,
    L2_NOTIFY_TYPE_SUSPEND_LEARN,
    L2_NOTIFY_TYPE_END
} rtk_l2_notifyType_t;

typedef enum rtk_l2_notifyEvent_e
{
    L2_NOTIFY_EVENT_LINKDOWNFLUSH = 0, 
    L2_NOTIFY_EVENT_SUSPEND,
    L2_NOTIFY_ADEVENT_END
} rtk_l2_notifyEvent_t;

typedef struct rtk_l2_notifyEventEntry_s
{
    rtk_l2_notifyType_t      type;
    uint32      fidVid;
    rtk_mac_t   mac;
    uint32      slp;
} rtk_l2_notifyEventEntry_t;

typedef struct rtk_l2_notifyEventCollectArray_s
{
    uint32 entryNum;
#if defined(CONFIG_SDK_DRIVER_NIC_USER_MODE)
    uint32 wrap;
    rtk_l2_notifyEventEntry_t *baseAddr;
    uint32 startPtr;
#else
    rtk_l2_notifyEventEntry_t *eventArray;
#endif
} rtk_l2_notifyEventCollectArray_t ;

typedef int32 (*rtk_l2_notification_callback_t)(uint32 unit, rtk_l2_notifyEventCollectArray_t *pEventCollectArray);
typedef int32 (*rtk_l2_notification_bufferRunOut_callback_t)(uint32 unit, rtk_l2_notifyBufferType_t runOutBufferType);

/*
 * Macro Declaration
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
extern int32
rtk_l2_init(uint32 unit);

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
extern int32
rtk_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig);

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
extern int32
rtk_l2_learningCnt_get(uint32 unit, uint32 *pMac_cnt);

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
extern int32
rtk_l2_limitLearningCnt_get(uint32 unit, uint32 *pMac_cnt);

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
extern int32
rtk_l2_limitLearningCnt_set(uint32 unit, uint32 mac_cnt);

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
extern int32
rtk_l2_limitLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction);

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
extern int32
rtk_l2_limitLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action);

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
extern int32
rtk_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt);

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
extern int32
rtk_l2_portLimitLearningCntEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_portLimitLearningCntEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

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
extern int32
rtk_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt);

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
extern int32
rtk_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt);

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
extern int32
rtk_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);

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
extern int32
rtk_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action);

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
extern int32
rtk_l2_portLastLearnedMac_get(uint32 unit, rtk_port_t port, rtk_fid_t *pFid, rtk_mac_t *pMac);

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
extern int32
rtk_l2_fidLimitLearningEntry_get(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry);

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
 *      (3) This API will flush all MACs learnt on specified VLAN before this entry takes effect.
 *          If the fid of an entry changes from one to another, this API will flush MACs learnt on these two VLANs.
 */
extern int32
rtk_l2_fidLimitLearningEntry_set(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry);

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
extern int32
rtk_l2_fidLearningCnt_get(uint32 unit, uint32 fid_macLimit_idx, uint32 *pNum);

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
extern int32
rtk_l2_fidLearningCnt_reset(uint32 unit, uint32 fid_macLimit_idx);

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
extern int32
rtk_l2_fidLastLearnedMac_get(
    uint32      unit,
    uint32      fid_macLimit_idx,
    rtk_fid_t   *pFid,
    rtk_mac_t   *pMac);

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
extern int32
rtk_l2_fidLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction);

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
extern int32
rtk_l2_fidLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action);

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
extern int32
rtk_l2_limitLearningTrapPri_get(uint32 unit, rtk_pri_t *pPriority);

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
extern int32
rtk_l2_limitLearningTrapPri_set(uint32 unit, rtk_pri_t priority);

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
extern int32
rtk_l2_limitLearningTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_limitLearningTrapPriEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_limitLearningTrapDP_get(uint32 unit, uint32 *pDp);

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
extern int32
rtk_l2_limitLearningTrapDP_set(uint32 unit, uint32 dp);

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
extern int32
rtk_l2_limitLearningTrapDPEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_limitLearningTrapDPEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_limitLearningTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_limitLearningTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_aging_get(uint32 unit, uint32 *pAging_time);

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
extern int32
rtk_l2_aging_set(uint32 unit, uint32 aging_time);

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
extern int32
rtk_l2_portAgingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_portAgingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

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
extern int32
rtk_l2_camEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_camEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_hashAlgo_get(uint32 unit, uint32 *pHash_algo);

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
extern int32
rtk_l2_hashAlgo_set(uint32 unit, uint32 hash_algo);

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
extern int32
rtk_l2_vlanMode_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pVlanMode);

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
extern int32
rtk_l2_vlanMode_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t vlanMode);

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
extern int32
rtk_l2_learningEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_learningEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

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
extern int32
rtk_l2_newMacOp_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_l2_newMacLrnMode_t  *pLrnMode,
    rtk_action_t            *pFwdAction);

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
extern int32
rtk_l2_newMacOp_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_l2_newMacLrnMode_t  lrnMode,
    rtk_action_t            fwdAction);

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
extern int32
rtk_l2_LRUEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_LRUEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_ucastLookupMode_get(uint32 unit, rtk_l2_ucastLookupMode_t *pUcast_lookupMode);

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
extern int32
rtk_l2_ucastLookupMode_set(uint32 unit, rtk_l2_ucastLookupMode_t ucast_lookupMode);

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
extern int32
rtk_l2_addr_init(
    uint32              unit,
    rtk_vlan_t          vid,
    rtk_mac_t           *pMac,
    rtk_l2_ucastAddr_t  *pL2_addr);

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
extern int32
rtk_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);

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
extern int32
rtk_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);

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
extern int32
rtk_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);

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
extern int32
rtk_l2_addr_set(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr);

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
extern int32
rtk_l2_addr_delAll(uint32 unit, uint32 include_static);

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
extern int32
rtk_l2_nextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    uint32              include_static,
    rtk_l2_ucastAddr_t  *pL2_data);

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
extern int32
rtk_l2_nextValidMcastAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_mcastAddr_t  *pL2_data);

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
extern int32
rtk_l2_nextValidIpMcastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ipMcastAddr_t    *pL2_data);

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
extern int32
rtk_l2_mcastLookupMode_get(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    *pMcast_lookupMode,
    rtk_fid_t                   *pFixed_fid);

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
extern int32
rtk_l2_mcastLookupMode_set(
    uint32                      unit,
    rtk_l2_mcastLookupMode_t    mcast_lookupMode,
    rtk_fid_t                   fixed_fid);

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
extern int32
rtk_l2_mcastBlockPortmask_get(uint32 unit, rtk_portmask_t *pPortmask);

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
extern int32
rtk_l2_mcastBlockPortmask_set(uint32 unit, rtk_portmask_t *pPortmask);

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
extern int32
rtk_l2_mcastAddr_init(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac, rtk_l2_mcastAddr_t *pMcast_addr);

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
extern int32
rtk_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

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
extern int32
rtk_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);

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
extern int32
rtk_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

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
extern int32
rtk_l2_mcastAddr_set(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

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
extern int32
rtk_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

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
extern int32
rtk_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr);

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
extern int32
rtk_l2_ipmcEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_ipmcEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_ipmcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode);

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
extern int32
rtk_l2_ipmcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode);

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
extern int32
rtk_l2_ipMcastAddr_init(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_l2_ipMcastAddr_t *pIpmcast_addr);

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
extern int32
rtk_l2_ipMcastAddrExt_init(uint32 unit, rtk_l2_ipMcastHashKey_t *pIpMcast_hashKey, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

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
extern int32
rtk_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr);

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
extern int32
rtk_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid);

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
extern int32
rtk_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr);

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
extern int32
rtk_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr);

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
extern int32
rtk_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

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
extern int32
rtk_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr);

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
extern int32
rtk_l2_ipMcastAddrChkEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_ipMcastAddrChkEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_ipMcstFidVidCompareEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_ipMcstFidVidCompareEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_ipmc_routerPorts_get(uint32 unit, rtk_portmask_t *pPortmask);

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
extern int32
rtk_l2_ipmc_routerPorts_set(uint32 unit, rtk_portmask_t *pPortmask);

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
extern int32
rtk_l2_ipmcDstAddrMismatchAction_get(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    *pMismatch_action);

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
extern int32
rtk_l2_ipmcDstAddrMismatchAction_set(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    mismatch_action);

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
extern int32
rtk_l2_ipmcDstAddrMismatchPri_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t *pPriority);

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
extern int32
rtk_l2_ipmcDstAddrMismatchPri_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t priority);

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
extern int32
rtk_l2_ipmcDstAddrMismatchPriEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_ipmcDstAddrMismatchPriEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable);

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
extern int32
rtk_l2_ipmcDstAddrMismatchDP_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 *pDp);

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
extern int32
rtk_l2_ipmcDstAddrMismatchDP_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 dp);

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
extern int32
rtk_l2_ipmcDstAddrMismatchDPEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_ipmcDstAddrMismatchDPEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable);

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
extern int32
rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable);

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
extern int32
rtk_l2_ip6mcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode);

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
extern int32
rtk_l2_ip6mcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode);

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
extern int32
rtk_l2_ip6CareByte_get(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 *pCareByte);

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
extern int32
rtk_l2_ip6CareByte_set(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 careByte);

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
extern int32
rtk_l2_ip6McastAddrExt_init(uint32 unit, rtk_l2_ip6McastHashKey_t *pIp6Mcast_hashKey, rtk_l2_ip6McastAddr_t *pIp6Mcast_addr);

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
extern int32
rtk_l2_ip6McastAddr_add(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr);

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
extern int32
rtk_l2_ip6McastAddr_del(uint32 unit, rtk_ipv6_addr_t sip, rtk_ipv6_addr_t dip, rtk_vlan_t vid);

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
extern int32
rtk_l2_ip6McastAddr_get(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr);

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
extern int32
rtk_l2_ip6McastAddr_set(uint32 unit, rtk_l2_ip6McastAddr_t *pIpmcast_addr);

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
extern int32
rtk_l2_ip6McastAddr_add_with_index(uint32 unit, rtk_l2_ip6McastAddr_t *pIpMcast_addr);

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
extern int32
rtk_l2_ip6McastAddr_get_with_index(uint32 unit, rtk_l2_ip6McastAddr_t *pIpMcast_addr);

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
extern int32
rtk_l2_nextValidIp6McastAddr_get(
    uint32                  unit,
    int32                   *pScan_idx,
    rtk_l2_ip6McastAddr_t    *pL2_data);

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
extern int32
rtk_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex);

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
extern int32
rtk_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index);

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
extern int32
rtk_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount);

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
extern int32
rtk_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          *pCrossVlan);

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
extern int32
rtk_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          crossVlan);

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
extern int32
rtk_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);


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
extern int32
rtk_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac);

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
extern int32
rtk_l2_legalMoveToPorts_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts);

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
extern int32
rtk_l2_legalMoveToPorts_set(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalPorts);

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
extern int32
rtk_l2_illegalPortMoveAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        *pFwdAction);

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
extern int32
rtk_l2_illegalPortMoveAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        fwdAction);

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
extern int32
rtk_l2_legalPortMoveAction_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        *pFwdAction);

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
extern int32
rtk_l2_legalPortMoveAction_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_action_t        fwdAction);

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
extern int32
rtk_l2_legalPortMoveFlushAddrEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_legalPortMoveFlushAddrEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

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
extern int32
rtk_l2_staticPortMoveAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pFwdAction);

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
extern int32
rtk_l2_staticPortMoveAction_set(uint32 unit, rtk_port_t port, rtk_action_t fwdAction);

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
extern int32
rtk_l2_lookupMissFloodPortMask_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask);

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
extern int32
rtk_l2_lookupMissFloodPortMask_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask);

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
extern int32
rtk_l2_lookupMissFloodPortMask_add(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port);

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
extern int32
rtk_l2_lookupMissFloodPortMask_del(uint32 unit, rtk_l2_lookupMissType_t type, rtk_port_t flood_port);

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
extern int32
rtk_l2_lookupMissFloodPortMask_set_with_idx(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx, rtk_portmask_t *pFlood_portmask);

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
extern int32
rtk_l2_lookupMissFloodPortMaskIdx_get(uint32 unit, rtk_l2_lookupMissType_t type, uint32 *pIdx);

/* Function Name:
 *      rtk_l2_lookupMissFloodPortMaskIdx_set
 * Description:
 *      Set the entry index of forwarding table that is used as unicast/broadcast flooding port mask.
 * Input:
 *      unit 	- unit id
 *      type    - type of lookup miss
 *      idx		- flooding port mask configuration when unicast/multicast lookup missed.
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
extern int32
rtk_l2_lookupMissFloodPortMaskIdx_set(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx);

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
extern int32
rtk_l2_lookupMissAction_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t *pAction);

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
extern int32
rtk_l2_lookupMissAction_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t action);

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
extern int32
rtk_l2_portLookupMissAction_get(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t *pAction);

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
extern int32
rtk_l2_portLookupMissAction_set(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action);

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
extern int32
rtk_l2_lookupMissPri_get(uint32 unit, rtk_pri_t *pPriority);

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
extern int32
rtk_l2_lookupMissPri_set(uint32 unit, rtk_pri_t priority);

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
extern int32
rtk_l2_lookupMissPriEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_lookupMissPriEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_lookupMissDP_get(uint32 unit, uint32 *pDp);

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
extern int32
rtk_l2_lookupMissDP_set(uint32 unit, uint32 dp);

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
extern int32
rtk_l2_lookupMissDPEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_lookupMissDPEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_lookupMissAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_lookupMissAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFilter_portmask);

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
extern int32
rtk_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFilter_portmask);

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
extern int32
rtk_l2_srcPortEgrFilterMask_add(uint32 unit, rtk_port_t filter_port);

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
extern int32
rtk_l2_srcPortEgrFilterMask_del(uint32 unit, rtk_port_t filter_port);

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
extern int32
rtk_l2_exceptionAddrAction_get(
    uint32                          unit,
    rtk_l2_exceptionAddrType_t      exceptType,
    rtk_action_t                    *pAction);

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
extern int32
rtk_l2_exceptionAddrAction_set(
    uint32                          unit,
    rtk_l2_exceptionAddrType_t      exceptType,
    rtk_action_t                    action);

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
extern int32
rtk_l2_notificationEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_notificationEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_notificationEventEnable_get(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_notificationEventEnable_set(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t enable);

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
extern int32
rtk_l2_notificationBackPressureThresh_get(uint32 unit, uint32 *pThresh);

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
extern int32
rtk_l2_notificationBackPressureThresh_set(uint32 unit, uint32 thresh);

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
extern int32
rtk_l2_notificationFifoEmptyStatus_get(uint32 unit, rtk_l2_notifyFifoStatus_t *pStatus);

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
extern int32
rtk_l2_notificationEventHandler_register(
    uint32                                      unit,
    rtk_l2_notification_callback_t              notification_callback,
    rtk_l2_notification_bufferRunOut_callback_t bufferRunOut_callback);

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
extern int32
rtk_l2_notificationEventHandler_unregister(uint32 unit);

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
extern int32
rtk_l2_trapPri_get(uint32 unit, rtk_pri_t *pPriority);

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
extern int32
rtk_l2_trapPri_set(uint32 unit, rtk_pri_t priority);

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
extern int32
rtk_l2_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_trapPriEnable_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable);

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
 *      8389, 8328, 8390
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
extern int32
rtk_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry);

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
extern int32
rtk_l2_conflictAddr_get(
    uint32          unit,
    rtk_l2_entry_t  *pL2Addr,
    rtk_l2_entry_t  *pCfAddrList,
    uint32          cfAddrList_size,
    uint32          *pCf_retCnt);

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
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_l2_zeroSALearningEnable_get(uint32 unit, rtk_enable_t *pEnable);

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
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_l2_zeroSALearningEnable_set(uint32 unit, rtk_enable_t enable);

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
 *      8390, 8380
 * Note:
 *      In secure SA mode, SA lookup missed packet should be dropped. Refer to rtk_l2_newMacOp_set to
 *      drop the SA lookup missed packet.
 */
extern int32
rtk_l2_secureMacMode_get(uint32 unit, rtk_enable_t *pEnable);

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
 *      8390, 8380
 * Note:
 *      In secure SA mode, SA lookup missed packet should be dropped. Refer to rtk_l2_newMacOp_set to
 *      drop the SA lookup missed packet.
 */
extern int32
rtk_l2_secureMacMode_set(uint32 unit, rtk_enable_t enable);

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
extern int32
rtk_l2_portDynamicPortMoveForbidEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_portDynamicPortMoveForbidEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

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
extern int32
rtk_l2_dynamicPortMoveForbidAction_get(uint32 unit, rtk_action_t *pAction);

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
extern int32
rtk_l2_dynamicPortMoveForbidAction_set(uint32 unit, rtk_action_t action);

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
extern int32
rtk_l2_portMacFilterEnable_get(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t *pEnable);

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
extern int32
rtk_l2_portMacFilterEnable_set(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t enable);

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
extern int32
rtk_l2_hwNextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_nextValidType_t type,
    rtk_l2_entry_t  *pEntry);
#endif /* __RTK_L2_H__ */
