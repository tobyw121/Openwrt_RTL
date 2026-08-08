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
 * $Revision: 46645 $
 * $Date: 2014-02-26 19:43:48 +0800 (Wed, 26 Feb 2014) $
 *
 * Purpose : Definition the basic types in the SDK.
 *
 * Feature : type definition
 *
 */

#ifndef __COMMON_RT_TYPE_H__
#define __COMMON_RT_TYPE_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/type.h>

/*
 * Symbol Definition
 */
#if defined(CONFIG_SDK_RTL8390)
#define RTK_MAX_NUM_OF_PORTS                        64 /* Applicable to 8390 */
#else
#define RTK_MAX_NUM_OF_PORTS                        32 /* Applicable to 8389, 8328, 8380 */
#endif
#define RTK_MAX_PORT_ID                             (RTK_MAX_NUM_OF_PORTS - 1)
#define RTK_TOTAL_NUM_OF_BYTE_FOR_1BIT_PORT_LIST    ((RTK_MAX_NUM_OF_PORTS+7)/8)
#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    ((RTK_MAX_NUM_OF_PORTS+31)/32)
#define RTK_MAX_NUM_OF_MIRRORING_PORT               1
#define RTK_MAX_NUM_OF_TRUNK_HASH_VAL               32
#define RTK_MAX_NUM_OF_SVLAN_ID                     4096
#define RTK_VLAN_ID_MIN                             0
#define RTK_VLAN_ID_MAX                             4095
#define RTK_ETHERTYPE_MAX                           0xFFFF
#define RTK_MAX_NUM_OF_QUEUE                        8
#define RTK_MAX_NUM_OF_PRIORITY                     8
#define RTK_VALUE_OF_DEI_MAX                        1
#define RTK_VALUE_OF_DEI_MIN                        0
#define RTK_VALUE_OF_DSCP_MAX                       63
#define RTK_VALUE_OF_DSCP_MIN                       0
#define RTK_DOT1P_PRIORITY_MAX                      7
#define RTK_DOT1P_DEI_MAX                           1
#define RTK_DOT1P_CFI_MAX                           1
#define RTK_DROP_PRECEDENCE_MAX                     2
#define RTK_LINKMON_SCAN_INTERVAL_MIN               10000
#define RTK_LOSMON_SCAN_INTERVAL_MIN                1000000
#define RTK_WA_SCAN_INTERVAL_MIN                    500000
#define RTK_MACMON_SCAN_INTERVAL_MIN                1000000
#define RTK_EEE_TX_SLEEP_RATE_FE_MAX                3125
#define RTK_EEE_TX_SLEEP_RATE_GE_MAX                31250
#define RTK_TPID_MAX                                0xFFFF
#define RTK_MAX_NUM_OF_SERDES                       14
#define RTK_MAX_SERDES_ID                           (RTK_MAX_NUM_OF_SERDES - 1)
#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_SERDES_LIST  ((RTK_MAX_NUM_OF_SERDES+31)/32)
#define RTK_PORT_SPEC_CONGEST_TIME_MAX              0xF
#define RTK_MAX_NUM_OF_LINKMON_CB                   4

/*
 * Data Type Declaration
 */
typedef uint32  rtk_vlan_t;         /* vlan id type                     */
typedef uint32  rtk_fid_t;          /* filter id type                   */
typedef uint32  rtk_stg_t;          /* spanning tree instance id type   */
typedef uint32  rtk_port_t;         /* port is type                     */
typedef uint32  rtk_pri_t;          /* priority vlaue                   */
typedef uint32  rtk_qid_t;          /* queue id type                    */
typedef uint32  rtk_filter_id_t;    /* filter id type                   */
typedef uint32  rtk_pie_id_t;       /* PIE id type                      */
typedef uint32  rtk_acl_id_t;       /* ACL id type                      */

typedef enum rtk_port_media_e
{
    PORT_MEDIA_COPPER = 0,
    PORT_MEDIA_FIBER,
    PORT_MEDIA_COPPER_AUTO,
    PORT_MEDIA_FIBER_AUTO,
    PORT_MEDIA_FIBER_AUTO_BY_GPIO,  /* valid if CONFIG_SDK_AUTO_COMBO_MEDIA_BY_GPIO */
    PORT_MEDIA_END
} rtk_port_media_t;

/* This enum is used for some specified PHY that have supported 1000BASE-X 
 * and 100BASE-FX auto detection. Ex: RTL8212B/RTL8214B//RTL8214FB
 */
typedef enum rtk_port_fiber_media_e
{
    PORT_FIBER_MEDIA_1000 = 0,
    PORT_FIBER_MEDIA_100,
    PORT_FIBER_MEDIA_AUTO,
    PORT_FIBER_MEDIA_END
} rtk_port_fiber_media_t;

typedef struct rtk_portmask_s
{
    uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

typedef struct rt_portType_info_s
{
    uint32  portNum;
    int32   max;    /* use (-1) for VALUE_NO_INIT */
    int32   min;    /* use (-1) for VALUE_NO_INIT */
    rtk_portmask_t  portmask;
} rt_portType_info_t;

typedef struct rtk_serdesmask_s
{
    uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_SERDES_LIST];
} rtk_serdesmask_t;

typedef struct rt_serdesType_info_s
{
    uint32  serdesNum;
    int32   max;    /* use (-1) for VALUE_NO_INIT */
    int32   min;    /* use (-1) for VALUE_NO_INIT */
    rtk_serdesmask_t  serdesmask;
} rt_serdesType_info_t;

/* frame type of protocol vlan - reference 802.1v standard */
typedef enum rtk_vlan_protoVlan_frameType_e
{
    FRAME_TYPE_ETHERNET = 0,
    FRAME_TYPE_RFC1042,
    FRAME_TYPE_SNAP8021H,/* Not supported by any chip */
    FRAME_TYPE_SNAPOTHER,/* Not supported by any chip */
    FRAME_TYPE_LLCOTHER,
    FRAME_TYPE_END
} rtk_vlan_protoVlan_frameType_t;

/* Protocol-and-port-based Vlan structure */
typedef struct rtk_vlan_protoAndPortInfo_s
{
    uint32                          proto_type;
    rtk_vlan_protoVlan_frameType_t  frame_type;
    rtk_vlan_t                      cvid;
    rtk_pri_t                       cpri;
}rtk_vlan_protoAndPortInfo_t;


#define     IPV6_ADDR_LEN   16
typedef uint32 rtk_ip_addr_t;
typedef struct rtk_ipv6_addr_s
{
    char   ipv6_addr[IPV6_ADDR_LEN];
} rtk_ipv6_addr_t;

typedef enum rtk_ip_family_e
{
    IPV4_FAMILY,
    IPV6_FAMILY,
    IP_FAMILY_END
} rtk_ip_family_t;

typedef enum rtk_l4Proto_e
{
    L4PROTO_TCP = 0,
    L4PROTO_UDP,
    L4PROTO_END
} rtk_l4Proto_t;

typedef enum rtk_action_e
{
    ACTION_FORWARD = 0,
    ACTION_DROP,
    ACTION_TRAP2CPU,
    ACTION_COPY2CPU,
    ACTION_TO_GUESTVLAN,
    ACTION_FLOOD_IN_VLAN,
    ACTION_FLOOD_IN_ALL_PORT,
    ACTION_FLOOD_IN_ROUTER_PORTS,
    ACTION_END
} rtk_action_t;

#define SNAPOUI_LEN     3
typedef struct rtk_snapOui_s
{
    uint8   snapOui[SNAPOUI_LEN];
} rtk_snapOui_t;

typedef enum rtk_pktType_e
{
    ETHERNET_II_PACKET = 0,
    SNAP_PACKET,
    PKT_TYPE_END
} rtk_pktType_t;

typedef enum rtk_vlanType_e
{
    INNER_VLAN = 0,
    OUTER_VLAN,
    VLAN_TYPE_END
} rtk_vlanType_t;

typedef enum rtk_snapMode_e
{
    SNAP_MODE_AAAA03000000 = 0,         /* compare 0xAAAA03000000 */
    SNAP_MODE_AAAA03,                   /* compare 0xAAAA03 */
    SNAP_MODE_END
} rtk_snapMode_t;

typedef enum rtk_pktFormat_e
{
    ORIGINAL_PACKET = 0,
    MODIFIED_PACKET,
    PKT_FORMAT_END
} rtk_pktFormat_t;

/* Reference Time */
typedef struct rtk_time_timeStamp_s
{
    uint32 sec;
    uint32 nsec;
} rtk_time_timeStamp_t;

/* Message Type */
typedef enum rtk_time_ptpMsgType_e
{
    PTP_MSG_TYPE_SYNC = 0,
    PTP_MSG_TYPE_DELAY_REQ = 1,
    PTP_MSG_TYPE_PDELAY_REQ = 2,
    PTP_MSG_TYPE_PDELAY_RESP = 3,
} rtk_time_ptpMsgType_t;

/* TIME packet identifier */
typedef struct rtk_time_ptpIdentifier_s
{
    rtk_time_ptpMsgType_t   msgType;
    uint32                  sequenceId;
} rtk_time_ptpIdentifier_t;

#endif /* __COMMON_RT_TYPE_H__ */
