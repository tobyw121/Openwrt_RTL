/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008-2009
 * All rights reserved.
 *
 * $Revision: 20135 $
 * $Date: 2011-07-28 17:51:53 +0800 (Thu, 28 Jul 2011) $
 *
 * Purpose : Definition those public PIE APIs and its data type in the SDK .
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Flow Classification
 *           (2) Ingress ACL
 *           (3) Egress ACL
 *           (4) Egress VID Translation
 *           (5) Range Check
 *           (6) Field Selector
 *           (7) Pattern Match
 *
 */

#ifndef __RTK_PIE_H__
#define __RTK_PIE_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */
#define RTK_MAX_NUM_OF_PATTERN_MATCH_DATA                       32
#define RTK_MAX_NUM_OF_PAYLOAD_OFFSET                           0x3f

#define RTK_PIE_BLOCK_PRIORITY_MIN                              0
#define RTK_PIE_BLOCK_PRIORITY_MAX                              15

#define RTK_NUM_OF_PIE_RULES_FOR_PHY_BLOCK                      64
#define RTK_NUM_OF_PIE_RULES_FOR_LOGICAL_BLOCK                  (RTK_NUM_OF_PIE_RULES_FOR_PHY_BLOCK * 2)
#define RTK_TOTAL_NUM_OF_BYTE_FOR_1BIT_RULE_OF_LOGICAL_BLOCK    (RTK_NUM_OF_PIE_RULES_FOR_LOGICAL_BLOCK/8)

#define RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD                       9

/*
 * Data Declaration
 */
typedef enum rtk_pie_tgL2Fmt_e
{
    L2FMT_NO_TAG = 0,
    L2FMT_INNER_TAG,
    L2FMT_OUTER_TAG,
    L2FMT_BOTH_INNER_OUTER_TAG,
    L2FMT_END
} rtk_pie_tgL2Fmt_t;

typedef enum rtk_pie_tgL23Fmt_e
{
    L23FMT_ARP = 0,
    L23FMT_L2,
    L23FMT_IPV4,
    L23FMT_IPV6,
    L23FMT_END
} rtk_pie_tgL23Fmt_t;

typedef enum rtk_pie_tgL4Fmt_e
{
    L4FMT_UNKNOWN = 0,
    L4FMT_TCP,
    L4FMT_UDP,
    L4FMT_ICMP,
    L4FMT_IGMP,
    L4FMT_ICMPV6,
    L4FMT_END
} rtk_pie_tgL4Fmt_t;

typedef struct rtk_pie_preDefinedRuleEntry_s
{
    uint8               valid;
    uint8               lookup_phase;
    uint8               lookup_phase_care;    
    uint8               src_phy_port;
    uint8               src_phy_port_care;
    rtk_pie_tgL2Fmt_t   tgl2fmt;
    rtk_pie_tgL2Fmt_t   tgl2fmt_care;
    uint8               extra_tag;
    uint8               extra_tag_care;
    rtk_pie_tgL23Fmt_t  tgl23fmt;
    rtk_pie_tgL23Fmt_t  tgl23fmt_care;
    rtk_pie_tgL4Fmt_t   tgl4fmt;
    rtk_pie_tgL4Fmt_t   tgl4fmt_care;
    uint8               pppoe;
    uint8               pppoe_care;
    uint8               ttl_type;
    uint8               ttl_type_care;
    uint16              src_vid;
    uint16              src_vid_care;
    uint16              dst_vid;
    uint16              dst_vid_care;
    union
    {
        struct
        {
            uint32  sip;
            uint32  sip_care;
            uint32  dip;
            uint32  dip_care;
        } arp;

        struct
        {
            uint32  sip;
            uint32  sip_care;
            uint32  dip;
            uint32  dip_care;
            uint8   protocol;
            uint8   protocol_care;
            uint8   tos;
            uint8   tos_care;
            uint8   ttl;
            uint8   ttl_care;
        } ipv4;

        struct
        {
            uint32  sip;
            uint32  sip_care;
            uint32  dip;
            uint32  dip_care;
            uint16  src_port;
            uint16  src_port_care;
            uint16  dst_port;
            uint16  dst_port_care;
            uint8   tcp_flag;
            uint8   tcp_flag_care;
        } ipv4_tcp;

        struct
        {
            uint32  sip;
            uint32  sip_care;
            uint32  dip;
            uint32  dip_care;
            uint16  src_port;
            uint16  src_port_care;
            uint16  dst_port;
            uint16  dst_port_care;
        } ipv4_udp;

        struct
        {
            uint32  sip;
            uint32  sip_care;
            uint32  dip;
            uint32  dip_care;
            uint8   icmp_type;
            uint8   icmp_type_care;
            uint8   icmp_code;
            uint8   icmp_code_care;
        } ipv4_icmp;

        struct
        {
            uint32  sip;
            uint32  sip_care;
            uint32  dip;
            uint32  dip_care;
            uint8   igmp_type;
            uint8   igmp_type_care;
            uint32  group_ip;
            uint32  group_ip_care;
        } ipv4_igmp;

        struct
        {
            uint32  ipv6_dip[4];
            uint32  ipv6_dip_care[4];
            uint8   next_header;
            uint8   next_header_care;
            uint8   dscp;
            uint8   dscp_care;
            uint32  flow_label;
            uint32  flow_label_care;
        } ipv6;

        struct
        {
            uint32  ipv6_dip[4];
            uint32  ipv6_dip_care[4];
            uint16  src_port;
            uint16  src_port_care;
            uint16  dst_port;
            uint16  dst_port_care;
            uint8   tcp_flag;
            uint8   tcp_flag_care;
        } ipv6_tcp;

        struct
        {
            uint32  ipv6_dip[4];
            uint32  ipv6_dip_care[4];
            uint16  src_port;
            uint16  src_port_care;
            uint16  dst_port;
            uint16  dst_port_care;
        } ipv6_udp;

        struct
        {
            uint32  ipv6_dip[4];
            uint32  ipv6_dip_care[4];
            uint8   icmp_type;
            uint8   icmp_type_care;
            uint8   icmp_code;
            uint8   icmp_code_care;
        } ipv6_icmp;
    }un;
} rtk_pie_preDefinedRuleEntry_t;

 
typedef enum rtk_pie_fieldType_e
{
    FIELD_LOOKUP_PHASE = 0,             /* lookup phase */
    FIELD_SRC_PHY_PORT,                 /* Source physical port */
    FIELD_EXTRA_TAG,                    /* Packet has an extra tag */
    FIELD_TGL2_FORMAT,                  /* Template generator Layer2 Format Definition.
                                         * 00:No Tag,
                                         * 01:Inner TAG only,
                                         * 10:Outer TAG only,
                                         * 11:Both outer TAG and inner TAG
                                         */
    FIELD_TGL23_FORMAT,                 /* Template generator Layer2/3 Format Definition.
                                         * 00:ARP,
                                         * 01:Layer2 only,
                                         * 10:IPv4,
                                         * 11:IPv6
                                         */
    FIELD_TGL4_FORMAT,                  /* Template generator Laye4 Format Definition.
                                         * 000:Unknown,
                                         * 001:TCP,
                                         * 010:UDP,
                                         * 011:ICMP,
                                         * 100:IGMP,
                                         * 101:ICMPv6
                                         */
                                        
    FIELD_SPM,                          /* source port mask */
    FIELD_SPSM,                         /* source port mask table check result */
    FIELD_DPM,                          /* destination port mask */
    FIELD_DA_TYPE,                      /* destination address type
                                         * 0:reserved,
                                         * 1:Unicast,
                                         * 2:Multicast,
                                         * 3:Broadcast
                                         */
    FIELD_CTLPKT = 10,                  
    FIELD_CPU,                          /* Packet is decided to send to cpu */
    FIELD_COPYTOCPU,                    /* copy to cpu bit */
    FIELD_DPN,                          /* destination port number */

/*
 * L2
 */
    FIELD_DMAC,                         /* destination MAC address */
    FIELD_SMAC,                         /* source MAC address */
    FIELD_ETHERTYPE,                    /* ethernet type */
                                        
    FIELD_OTAG_PRI,                     /* O-TAG priority */
    FIELD_DEI_EXIST,                    /* O-TAG DEI field */
    FIELD_OTAG_VID,                     /* O-TAG VID */
                                        
    FIELD_ITAG_PRI = 20,                /* I-TAG priority */
    FIELD_CFI_EXIST,                    /* I-TAG CFI */
    FIELD_ITAG_VID,                     /* I-TAG VID */
                                        
    FIELD_FWD_OTAG_PRI,                 /* Packet's final priority */
    FIELD_FWD_CPU_TAG,                  /* CPU tag exist */
    FIELD_FWD_OTAG_VID,                 /* Packet's final OVID after vlan decision */
                                        
    FIELD_FWD_ITAG_PRI,                 /* Packet's final priority */
    FIELD_FWD_ITAG_VID,                 /* Packet's final IVID after vlan decision */
                                        
    FIELD_FWD_VID_PRI,                  /* Packet's final priority */
    FIELD_FWD_VID_IVID_OR_OVID,         /* Packet's final IVID or OVID after vlan decision */

    FIELD_DP = 30,                      /* Packet's final drop precedence*/
                                        
/*                                      
 * L3/L4                                 
 */                                     
    FIELD_IPV4TOS_IPV6DS,               /* IPv4 TOS, IPv6 DS */
    FIELD_IPV4PROTO_IPV6NH,             /* IPv4 protocol, IPv6 Next Header */
    FIELD_TTL,
    FIELD_TTL_TYPE,                     /* TTL Type Definition.
                                         * 00:TTL is 1,
                                         * 01:TTL is 0,
                                         * 10:TTL is larger or equal to 2 but smaller than 255,
                                         * 11:TTL is 255
                                         */
    FIELD_IP_DF,                        /* IP flag: don't fragment */
    FIELD_IP_MF,                        /* IP flag: more fragment */
    FIELD_IP_NONZERO_OFFSET,            /* IP: fragment offset */
    FIELD_REDIRECT,                     /* Packet forwarding process is controlled by redirect decision. */
    FIELD_IP_UCAST,                     /* unicast ip address routing */
    FIELD_IP_MCAST = 40,                /* Ip multicast routing */
    FIELD_L4_SRC_PORT,                  /* layer 4 source port */
    FIELD_L4_DST_PORT,                  /* layer 4 destination port */
    FIELD_TCP_FLAG,                     /* TCP flag */
    FIELD_ICMP_CODE,                    /* ICMP code */
    FIELD_ICMP_TYPE,                    /* ICMP type */
    FIELD_IGMP_TYPE,                    /* IGMP type */
    FIELD_SIP,                          /* IPv4 source IP */
    FIELD_DIP,                          /* IPv4 destination IP */
    FIELD_IPV6_SIP,                     /* IPv6 source IP */
    FIELD_IPV6_DIP = 50,                /* IPv6 destination IP */
    FIELD_FLOW_LABEL,                   /* flow label */

/*
 *Application
 */
    FIELD_PPPOE,                        /* Whether PPPoE header exists */
                                        
    FIELD_RTK_PROTO,                    /* Realtek reserved protocol type */
                                        
    FIELD_TO_GUEST_VLAN,                /* packet is forwarded to guest VLAN */
    FIELD_KEEP_ORG_VID,                 /* keep packet's original VID when it's forwarded to guest VLAN,
                                         * this VID is the result of VLAN decision.
                                         */
                                        
    FIELD_VID_RANGE_HIT,                /* VID range checking hit flag */
    FIELD_VID_RANGE,                    /* VID range check result */ /* the index of hit entry */
    FIELD_PORT_RANGE,                   /* Layer4 port range check result */
    FIELD_IP_RANGE0,                    /* IP range check result [15:0] */
    FIELD_IP_RANGE1 = 60,                    /* IP range check result [31:16] */
    FIELD_IP_RANGE2,                    /* IP range check result [47:32] */
    FIELD_IP_RANGE3,                    /* IP range check result [63:48] */
                                        
    FIELD_PATTERN_MATCH0,               /* pattern match [15:0] */
    FIELD_PATTERN_MATCH1,               /* pattern match [31:16] */
                                        
    FIELD_FIELD_SELECTOR1,              /* Field selector 1 output */
    FIELD_FIELD_SELECTOR2,              /* Field selector 2 output */
    FIELD_FIELD_SELECTOR1_0,            /* field selector 1 [15:0] */
    FIELD_FIELD_SELECTOR1_1,            /* field selector 1 [31:16] */
    FIELD_FIELD_SELECTOR1_2,            /* field selector 1 [47:32] */
    FIELD_FIELD_SELECTOR1_3 = 70,       /* field selector 1 [63:48] */
    FIELD_FIELD_SELECTOR2_0,            /* field selector 2 [15:0] */
    FIELD_FIELD_SELECTOR2_1,            /* field selector 2 [31:16] */
    FIELD_FIELD_SELECTOR2_2,            /* field selector 2 [47:32] */
    FIELD_FIELD_SELECTOR2_3,            /* field selector 2 [63:48] */
                                        
    FIELD_PAYLOAD0_VALID,               /* payload valid */
    FIELD_PAYLOAD1_VALID,               /* payload valid */
    FIELD_PAYLOAD0,                     /* 2 bytes payload from specified offset */
    FIELD_PAYLOAD1,                     /* 2 bytes payload from specified offset */
                                        
/*                                      
 * Egress VID Translation               
 */                                     
    FIELD_EVT_LOOKUP_PHASE,             /* lookup phase */
    FIELD_EVT_SRC_TRUNK_PHY_PORT = 80,  /* Source trunk port number */
    FIELD_EVT_L2_FORMAT,                /* Layer 2 packet format */
    FIELD_EVT_L34_FORMAT,               /* Layer 3/4 packet format */
    FIELD_EVT_EXTRA_TAG,                /* Packet has an extra tag */

    FIELD_EVT_DMAC_INDEX,               /* this field point to the DMAC index in layer2 hash table */
    FIELD_EVT_IP_UCAST,                 /* unicast ip address routing */
    FIELD_EVT_IP_MCAST,                 /* Ip multicast routing */

    FIELD_EVT_RRCP,                     /* Is RRCP packet */
    FIELD_EVT_RRCP_TYPE,                
                                        
    FIELD_EVT_DOTAG_PRI,                /* Destination O-TAG priority*/
    FIELD_EVT_OTAG_EXIST = 90,          /* O-TAG exist flag */
    FIELD_EVT_DOTAG_VID,                /* Destination O-TAG VID */
                                        
    FIELD_EVT_DITAG_PRI,                /* Destination I-TAG priority */
    FIELD_EVT_ITAG_EXIST,               /* I-TAG exist flag */
    FIELD_EVT_DITAG_VID,                /* Destination I-TAG */
                                        
    FIELD_EVT_PRI,                      /* Priority */
    FIELD_EVT_DP,                       /* Drop precedence */
    FIELD_EVT_DSCP_RMK,                 /* Weather DSCP remarking or not */
    FIELD_EVT_DSCP,                     /* DSCP remarking value */
    FIELD_EVT_RX_CPU_TAG,               /* CPU tag exist when packet from cpu port (port 28) */
    FIELD_EVT_RX_RSPAN = 100,           /* The RSPAN Tag is exist or not. */
                                        
    FIELD_EVT_DPN,                      /* destination port number */
                                        
                                        
    FIELD_VAILD,                        /* rule valid bit */

    PIE_FIELD_END
} rtk_pie_fieldType_t;

typedef struct rtk_pie_clearBlockContent_s
{
    uint16  start_idx;  /* Clear from this index */
    uint16  end_idx;    /* Clear to this index */
} rtk_pie_clearBlockContent_t;

typedef enum rtk_pie_movePieDirection_e
{
    DIRECTION_INCREASE = 0,
    DIRECTION_DECREASE,
    PIE_DIRECTION_END
} rtk_pie_movePieDirection_t;

typedef struct rtk_pie_movePieContent_s
{
    uint16  entry_length;  /* Indicates the moved PIE entry length */
    rtk_pie_movePieDirection_t   direction;/* Address increase or decrease.
                                            * 0:increase, the address of unit operated next time will be one step increase
                                            * 1:decrease, the address of unit operated next time will be one step decrease
                                            */
    uint16  move_to;       /* Move to this PIE entry */
    uint16  move_from;     /* Move from this PIE entry */
} rtk_pie_movePieContent_t;

typedef enum rtk_pie_statisticType_e
{
    COUNT_NONE = 0,
    COUNT_PACKET,
    COUNT_BYTE32,
    COUNT_BYTE64,
    PIE_COUNT_END
} rtk_pie_statisticType_t;

typedef enum rtk_pie_dropType_e
{
    DROP_PERMIT = 0,
    DROP_DROP,
    DROP_WITHDRAW_DROP,
    DROP_RESERVED,
    PIE_DROP_END
} rtk_pie_dropType_t;

typedef struct rtk_pie_actionOuterTag_s
{
    rtk_enable_t   withdraw;
    uint8   outer_vid_ctrl;     /* 00: Packet sent without outer VLAN tag; Reserved;
                                 * 01: provide new outer VID as OutVIDInfo;
                                 * 10: packet's new outer VID = packet's outer tag VID - OutVIDInfo;
                                 * 11: packet's new outer VID = packet's outer tag VID + OutVIDInfo.
                                 */
    uint16  outer_vid_info;     /* When OutVIDCtl is 00, this field should be ignored;
                                 * When OutVIDCtl is 01, OutVIDInfo provides new outer VID value;
                                 * When OutVIDCtl is 10/11, OutVIDInfo is the offset of packet's new outer VID based on its outer 1Q VID.
                                 */
    uint8   outer_tag_op;       /* Outer VLAN Tag operation
                                 * 00: packet sent without outer VLAN tag
                                 * 01: packet sent with outer VLAN tag
                                 * 10: In flow classification lookup phase:
                                 *       Select port-based VID as outer VID
                                 *     In others:
                                 *       keep outer VLAN tag format and outer VID
                                 * 11: Nop
                                 */
} rtk_pie_actionOuterTag_t;

typedef enum rtk_pie_redirectOpcode_e
{
    REDIRECT_UNI_REDIRECT = 0,
    REDIRECT_MULTI_REDIRECT,
    REDIRECT_UNI_ROUTE,
    REDIRECT_MULTI_ROUTE,
    REDIRECT_END
} rtk_pie_redirectOpcode_t;

typedef struct rtk_pie_actionRedirect_s
{
    rtk_enable_t                withdraw;
    rtk_pie_redirectOpcode_t    opcode;  /* 00: uniRedirect, 01: multiRedirect, 10: uniRoute, 11: multiRoute */

    union
    {
        struct
        {
            uint8 cpu_tag  ; /* Carry a CPU tag or not when redirect(either forward or trap) to CPU.
                              * 0: Don't care, don't touch CPU TAG.
                              * 1: With CPU tag when trap to CPU.
                              */
            uint8 dpn;       /* Destination port number.
                              * DPN 28 means normal forward to CPU port,
                              * DPN 29 means trap to CPU,
                              * DPN 30 means drop,
                              * DPN 31 means flood
                              */
        } uniRedirect;

        struct
        {
            uint16 fwd_idx;  /* Forwarding table index. */
        } multiRedirect;

        struct
        {
            uint8   ttl_dec;     /* TTL decreases or not. 0: keep TTL, 1: TTL minus one */
            uint16  lookup_idx;  /* Index to address lookup table. */
        } route;
    }un;
} rtk_pie_actionRedirect_t;

typedef struct rtk_pie_actionInnerTag_s
{
    rtk_enable_t   withdraw;
    uint8   inner_vid_ctrl;    /* 00: Reserved;
                                * 01: Provide new inner VID;
                                * 10: Packet's new inner VID = packet's inner tag VID - InVIDInfo;
                                * 11: Packet's new inner VID = packet's inner tag VID + InVIDInfo.
                                */
    uint16  inner_vid_info;    /* When InVIDCtl is 00, this field should be ignored;
                                * When InVIDCtl is 01, InVIDInfo provides new inner VID value;
                                * When InVIDCtl is 10/11, InVIDInfo is the offset of packet's new inner VID based on its inner 1Q VID.
                                */
    uint8   inner_tag_op;      /* Inner tag operation
                                * 00: packet sent without inner VLAN tag
                                * 01: packet sent with inner VLAN tag
                                * 10: In flow classification lookup phase:
                                *        Select port-based VID as inner VID
                                *     In others:
                                *        keep inner VLAN tag format and inner VID
                                * 11: Nop
                                */
} rtk_pie_actionInnerTag_t;

typedef struct rtk_pie_actionPriority_s
{
    rtk_enable_t   withdraw;
    uint8   sel_pri_dp;           /* In Ingress VID Translation phase:
                                   * Provide priority and drop precedence
                                   * 000: Provide 802.1Q-based inner priority and drop precedence
                                   * 001: Provide 802.1Q-based outer priority and drop precedence
                                   * 010: Provide Flow-based priority and drop precedence
                                   * Others: Reserved
                                   */
                                  /* In Ingress/Egress ACL phase 
                                   * Select priority and drop precedence for internal forwarding.
                                   * 000: Use ACL-based priority and drop precedence
                                   * 001: Use port-based priority and drop precedence
                                   * 010: Use 802.1Q-based inner priority and drop precedence
                                   * 011: Use 802.1Q-based outer priority and drop precedence
                                   * 100: Use DSCP-based priority and drop precedence
                                   * 101: Use flow-based priority and drop precedence
                                   * 110: Use inner tag priority and drop precedence
                                   * 111: Use outer tag priority and drop precedence
                                   */
                                  /* In Egress VID Translation phase
                                   * 000: Provide Tx inner priority and CFI, 
                                   * Others: Nop 
                                   */
    uint8   assign_pri;    /* Assign priority (ignored in ingress VID translation lookup phase)
                            * 1: assign.
                            * 0: not assign.
                            */
    uint8   acl_pri;       /* Priority value. */
    uint8   assign_dp;     /* Assign drop precedence(or CFI in egress VID translation lookup phase). ignored in ingress VID translation lookup phase
                            * 1: assign.
                            * 0: not assign.
                            */
    uint8   acl_dp;        /* Drop precedence value. Value 3 means drop. In Egress VID translation lookup phase, only refer to bit 0 as CFI value. */
} rtk_pie_actionPriority_t;

#if defined(CONFIG_SDK_ENDIAN_BIG)
typedef struct rtk_pie_actionSpid_s
{
    uint8   reserved:4;
    uint8   spid_idx:4; /* SPID index of the SPID table, makes sense only in egress VID translation lookup phase. */
} rtk_pie_actionSpid_t;

typedef struct rtk_pie_actionDscpRemarkDscp_s
{
    uint8   opcode:2;  /* 00: DSCP remarking; 01: modify IP precedence; 10: modify DTR bits; 11: Reserved. */
    uint8   dscp:6;    /* modify DSCP */
} rtk_pie_actionDscpRemarkDscp_t;

typedef struct rtk_pie_actionDscpRemarkIpPrecedence_s
{
    uint8   opcode:2;        /* 00: DSCP remarking; 01: modify IP precedence; 10: modify DTR bits; 11: Reserved. */
    uint8   ip_precedence:3; /* modify IP precedence */
    uint8   reserved:3;
} rtk_pie_actionDscpRemarkIpPrecedence_t;

typedef struct rtk_pie_actionDscpRemarkDtrBits_s
{
    uint8   opcode:2;   /* 00: DSCP remarking; 01: modify IP precedence; 10: modify DTR bits; 11: Reserved. */
    uint8   reserved:3;
    uint8   dtr_bits:3; /* modify DTR bits */
} rtk_pie_actionDscpRemarkDtrBits_t;

#else /* little endian */
typedef struct rtk_pie_actionSpid_s
{
    uint8   spid_idx:4; /* SPID index of the SPID table, makes sense only in egress VID translation lookup phase. */
    uint8   reserved:4;
} rtk_pie_actionSpid_t;

typedef struct rtk_pie_actionDscpRemarkDscp_s
{
    uint8   dscp:6;    /* modify DSCP */
    uint8   opcode:2;  /* 00: DSCP remarking; 01: modify IP precedence; 10: modify DTR bits; 11: Reserved. */
} rtk_pie_actionDscpRemarkDscp_t;

typedef struct rtk_pie_actionDscpRemarkIpPrecedence_s
{
    uint8   reserved:3;
    uint8   ip_precedence:3; /* modify IP precedence */
    uint8   opcode:2;        /* 00: DSCP remarking; 01: modify IP precedence; 10: modify DTR bits; 11: Reserved. */
} rtk_pie_actionDscpRemarkIpPrecedence_t;

typedef struct rtk_pie_actionDscpRemarkDtrBits_s
{
    uint8   dtr_bits:3; /* modify DTR bits */
    uint8   reserved:3;
    uint8   opcode:2;   /* 00: DSCP remarking; 01: modify IP precedence; 10: modify DTR bits; 11: Reserved. */
} rtk_pie_actionDscpRemarkDtrBits_t;
#endif

typedef struct rtk_pie_actionDscpRemark_s
{
   union
    {
        rtk_pie_actionDscpRemarkDscp_t          dscp;       /* Define ACL-based DSCP */
        rtk_pie_actionDscpRemarkIpPrecedence_t  ipPrecedence; /* modify IP precedence */
        rtk_pie_actionDscpRemarkDtrBits_t       dtrBits;      /* modify DTR bits */
        uint8                                   data;
    }un;
} rtk_pie_actionDscpRemark_t;

typedef struct rtk_pie_actionDscpRemarkSpid_s
{
    uint8      withdraw;
    union
    {
        rtk_pie_actionDscpRemark_t     dscp_remark;
        rtk_pie_actionSpid_t           spid;         /* action in Egress VID Translation phase */
    }un;
} rtk_pie_actionDscpRemarkSpid_t;

#if defined(CONFIG_SDK_ENDIAN_BIG)
typedef struct rtk_pie_actionOuterPriRemark_s
{
    uint8   remark_pri:1;  /* Remark priority of outer VLAN or not
                            * 1: remark.
                            * 0: do not remark
                            */
    uint8   pri:3;         /* priority value */
    uint8   remark_dei:1;  /* Remark DEI of outer VLAN or not
                            * 1: remark.
                            * 0: do not remark
                            */
    uint8   dei:1;         /* DEI value */  
    uint8   reserved:2;    /* reserved one more bit to consist with policer */
} rtk_pie_actionOuterPriRemark_t;

#else /* little endian */
typedef struct rtk_pie_actionOuterPriRemark_s
{
    uint8   reserved:2;    /* reserved one more bit to consist with policer */
    uint8   dei:1;         /* DEI value */  
    uint8   remark_dei:1;  /* Remark DEI of outer VLAN or not
                            * 1: remark.
                            * 0: do not remark
                            */
    uint8   pri:3;         /* priority value */
    uint8   remark_pri:1;  /* Remark priority of outer VLAN or not
                            * 1: remark.
                            * 0: do not remark
                            */
} rtk_pie_actionOuterPriRemark_t;
#endif

typedef struct rtk_pie_actionPolicerOuterPriRemark_s
{
    uint8   withdraw;
    union
    {
        uint8   policer_idx;
        rtk_pie_actionOuterPriRemark_t     outer_pri_remark; /* action in Egress VID Translation phase */
    }un;
} rtk_pie_actionPolicerOuterPriRemark_t;

typedef enum rtk_pie_policerType_e
{
    POLICER_TYPE_INVALID = 0,
    POLICER_TYPE_DLB,
    POLICER_TYPE_SRTCM,
    POLICER_TYPE_TRTCM,
    POLICER_TYPE_END
} rtk_pie_policerType_t;

typedef struct rtk_pie_policerEntry_s /* 256 entry */
{
    rtk_pie_policerType_t type;  /* 00: invalid, 01: DLB; 10: srTCM; 11: trTCM. */
    uint8   color_aware;  /* Policer is color aware. This bit takes effect when policer is RFC2697 or RFC2698.*/
    uint8   yellow_dp;    /* If packet is judged as YELLOW, using this drop precedence to replace the packet drop precedence.
                           * If YEL_DP==3, drop this packet.
                           */
    uint8   red_dp;       /* If packet is judged as RED, using this drop precedence to replace the packet drop precedence.
                           * The most significant bit is always 1, this bit specify the last significant bit.
                           * If bit is 0, the RED drop precedence is 2.
                           * If bit is 1, the RED drop precedence is 3.
                           */
    uint16  pir;          /* Peak Information Rate in units of 16Kbps */
    uint16  cir;          /* Committed Information Rate in units of 16Kbps */
    uint16  tp;           /* Peak Token Bucket (trTCM), Exceed Token Bucket (srTCM) */
    uint16  tc;           /* Commited Token Bucket */
    uint16  pbs;          /* Peak Burst Size (trTCM), Excess Burst Size (EBS) in byte unit */
    uint16  cbs;          /* Committed Burst Size in byte units */
} rtk_pie_policerEntry_t;

typedef struct rtk_pie_actionMirror_s
{
    rtk_enable_t   withdraw;
    uint8          mirror_idx;   /* mirror table index. */
} rtk_pie_actionMirror_t;

typedef struct rtk_pie_actionTable_s
{
    rtk_enable_t                         hit_indication;
    rtk_pie_statisticType_t              statistics; /* 00: don't count,
                                                      * 01: packet count,
                                                      * 10: byte count by 32-bit,
                                                      * 11: byte count by 64-bit
                                                      */
    rtk_enable_t                         cp2cpu;
    rtk_pie_dropType_t                   drop;       /* 00: permit, 01: drop, 10: withdraw drop, 11: reserved */

    /* driver will take at most five actions start from the first one below */
    rtk_enable_t                         outer_tag_op;
    rtk_enable_t                         redirect;
    rtk_enable_t                         inner_tag_op;
    rtk_enable_t                         priority;
    rtk_enable_t                         dscp_remark_spid;    /* DSCP remark in Flow Classification, Ingress ACL, Egress ACL 
                                                               * SPDI translation Egress VID Translation 
                                                               */
    rtk_enable_t                         police_outer_pri_remark; /* Police in Flow Classification, Ingress ACL, Egress ACL 
                                                                   * Outer priority remark in Egress VID Translation 
                                                                   */
    rtk_enable_t                         mirror;

    rtk_pie_actionOuterTag_t                outer_tag_op_field;
    rtk_pie_actionRedirect_t                redirect_field;
    rtk_pie_actionInnerTag_t                inner_tag_op_field;
    rtk_pie_actionPriority_t                priority_field;
    rtk_pie_actionDscpRemarkSpid_t          dscp_remark_spid_field; /* DSCP remark action in Flow classification, ingress ACL, egress ACL */
                                                                    /* SPID action in Egress VID translation */
    rtk_pie_actionPolicerOuterPriRemark_t   police_outer_pri_remark_field; /* Policer action in Flow classification, ingress ACL, egress ACL */
                                                                           /* Outer priority remark action in Egress VID translation */
    rtk_pie_actionMirror_t                  mirror_field;
} rtk_pie_actionTable_t;

typedef struct rtk_pie_hitIndicationEntry_s
{
    uint8   hit_status[RTK_TOTAL_NUM_OF_BYTE_FOR_1BIT_RULE_OF_LOGICAL_BLOCK];
} rtk_pie_hitIndicationEntry_t;

typedef enum rtk_pie_phase_e
{
    PIE_FLOW_CLASSIFICATION = 0,
    PIE_IGR_ACL,
    PIE_EGR_ACL,
    PIE_EGR_VID_TRANSLATION,
    PIE_PHASE_END
} rtk_pie_phase_t;

typedef enum rtk_pie_templateFiledType_e
{
    FMT = 0,
    DMAC0,
    DMAC1_RRCPINFO,
    DMAC2,
    SMAC0,
    SMAC1,
    SMAC2,
    ETHERTYPE,
    OTAG,
    INTAG,
    FWD_OTAG_DPRI,
    FWD_ITAG,
    FWD_VID,
    IP_TOS_PROTO,
    IP4_TTL_FLAG,
    L4_SPORT,
    L4_DPORT,
    TCP_FLAG,
    ICMP_CODE_TYPE,
    IGMP_TYPE,
    SIP0,               /* For IPv4/IPv6 */
    SIP1,               /* For IPv4/IPv6 */
    SIP2,               /* For IPv6 */
    SIP3,               /* For IPv6 */
    SIP4,               /* For IPv6 */
    SIP5,               /* For IPv6 */
    SIP6,               /* For IPv6 */
    SIP7,               /* For IPv6 */
    DIP0,               /* For IPv4/IPv6 */
    DIP1,               /* For IPv4/IPv6 */
    DIP2,               /* For IPv6 */
    DIP3,               /* For IPv6 */
    DIP4,               /* For IPv6 */
    DIP5,               /* For IPv6 */
    DIP6,               /* For IPv6 */
    DIP7,               /* For IPv6 */
    IP6_FLWH,
    SPM1,               /* Bits(28-13) */
    SPM2,               /* Bits(12-0) */
    SPSM,
    DPM1,               /* Bits(28-13) */
    DPM2,               /* Bits(12-0) */
    VID_RANG,
    GUEST_VLAN,
    PORT_RANG,
    IP_RANG1,
    IP_RANG2,
    IP_RANG3,
    IP_RANG4,
    PATTERN_MATCH0,
    PATTERN_MATCH1,
    FIELD_SELECTOR1_0,
    FIELD_SELECTOR1_1,
    FIELD_SELECTOR1_2,
    FIELD_SELECTOR1_3,
    FIELD_SELECTOR2_0,
    FIELD_SELECTOR2_1,
    FIELD_SELECTOR2_2,
    FIELD_SELECTOR2_3,
    DPN,
    PAYLOAD0,
    PAYLOAD1,
    PIE_FIELD_TYPE_END
} rtk_pie_templateFiledType_t;

typedef struct rtk_pie_template_s
{
    rtk_pie_templateFiledType_t  field[RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD];
} rtk_pie_template_t;

typedef enum rtk_pie_l34ChecksumErrOper_e
{
    CHECKSUM_ERR_DOWNGRADE = 0,  /* Downgrade */
    CHECKSUM_ERR_PARSE_ANYWAY,   /* Parse upper protocol anyway */
    PIE_CHECKSUM_ERR_OPER_END
} rtk_pie_l34ChecksumErrOper_t;

typedef enum rtk_pie_resultReverseOper_e
{
    RESULT_KEEP = 0, /* keep result */
    RESULT_REVERSE,  /* reverse result */
    PIE_RESULT_REVERSE_END
} rtk_pie_resultReverseOper_t;

typedef enum rtk_pie_resultAggregatorRange_e
{
    AGGREGATOR_PBLOCK_0_3 = 0,
    AGGREGATOR_PBLOCK_4_7,
    AGGREGATOR_PBLOCK_8_11,
    AGGREGATOR_PBLOCK_12_15,
    AGGREGATOR_PBLOCK_16_19,
    AGGREGATOR_PBLOCK_20_23,
    AGGREGATOR_PBLOCK_24_27,
    AGGREGATOR_PBLOCK_28_31,
    PIE_AGGREGATOR_RANGE_END
} rtk_pie_resultAggregatorRange_t;

typedef enum rtk_pie_resultAggregatorType_e
{                        /* where m is ranged from 0 to 7 and k is ranged from 0 to 63. */
    NONE  = 0,           /* Not aggregation [4m~4m+3][k] = PIERR [4m~4m+3][k]*/
    A01M0,               /* 0 and 1 aggregate, but 2, 3 do not aggregate.
                          * PIERRAR [4m][k] = PIERR [4m][k] and PIERR [4m+1][k]
                          * PIERRAR [4m+1][k] = 0
                          * PIERRAR [4m+2~4m+3][k] = PIERR [4m+2~4m+3][k]
                          */
    A01M1,               /* 0 and 1 aggregate, but 2, 3 do not aggregate.
                          * PIERRAR [4m][k] = 0
                          * PIERRAR [4m+1][k] = PIERR [4m][k] and PIERR [4m+1][k]
                          * PIERRAR [4m+2~4m+3][k] = PIERR [4m+2~4m+3][k]
                          */
    A23M2,               /* 0 and 1 do not aggregate, but 2, 3 aggregate.
                          * PIERRAR [4m~4m+1][k] = PIERR [4m~4m+1][k]
                          * PIERRAR [4m+2][k] = PIERR [4m+2][k] and PIERR [4m+3][k]
                          * PIERRAR [4m+3][k] = 0
                          */
    A23M3,               /* 0 and 1 do not aggregate, but 2, 3 aggregate.
                          * PIERRAR [4m~4m+1][k] = PIERR [4m~4m+1][k]
                          * PIERRAR [4m+2][k] = 0
                          * PIERRAR [4m+3][k] = PIERR [4m+2][k] and PIERR [4m+3][k]
                          */
    A01_23M0_2,          /* 0, 1 aggregate and 2, 3 aggregate.
                          * PIERRAR [4m][k] = PIERR [4m][k] and PIERR [4m+1][k]
                          * PIERRAR [4m+1][k] = 0
                          * PIERRAR [4m+2][k] = PIERR [4m+2][k] and PIERR [4m+3][k]
                          * PIERRAR [4m+3][k] = 0
                          */
    A01_23M1_2,          /* 0, 1 aggregate and 2, 3 aggregate.
                          * PIERRAR [4m][k] = 0
                          * PIERRAR [4m+1][k] = PIERR [4m][k] and PIERR [4m+1][k]
                          * PIERRAR [4m+2][k] = PIERR [4m+2][k] and PIERR [4m+3][k]
                          * PIERRAR [4m+3][k] = 0
                          */
    A01_23M0_3,          /* 0, 1 aggregate and 2, 3 aggregate.
                          * PIERRAR [4m][k] = PIERR [4m][k] and PIERR [4m+1][k]
                          * PIERRAR [4m+1][k] = 0
                          * PIERRAR [4m+2][k] = 0
                          * PIERRAR [4m+3][k] = PIERR [4m+2][k] and PIERR [4m+3][k]
                          */
    A01_23M1_3,          /* 0, 1 aggregate and 2, 3 aggregate.
                          * PIERRAR [4m][k] = 0
                          * PIERRAR [4m+1][k] = PIERR [4m][k] and PIERR [4m+1][k]
                          * PIERRAR [4m+2][k] = 0
                          * PIERRAR [4m+3][k] = PIERR [4m+2][k] and PIERR [4m+3][k]
                          */
    A0123M0,             /* 0, 1, 2, 3 all aggregated.
                          * PIERRAR [4m][k] = PIERR [4m][k] & PIERR [4m+1][k] & PIERR [4m+2][k] & PIERR [4m+3][k]
                          * PIERRAR [4m+1][k] = 0
                          * PIERRAR [4m+2][k] = 0
                          * PIERRAR [4m+3][k] = 0
                          */
    A0123M1,             /* 0, 1, 2, 3 all aggregated.
                          * PIERRAR [4m+1][k] = PIERR [4m][k] & PIERR [4m+1][k] & PIERR [4m+2][k] & PIERR [4m+3][k]
                          * PIERRAR [4m][k] = 0
                          * PIERRAR [4m+2][k] = 0
                          * PIERRAR [4m+3][k] = 0
                          */
    A0123M2,             /* 0, 1, 2, 3 all aggregated.
                          * PIERRAR [4m+2][k] = PIERR [4m][k] & PIERR [4m+1][k] & PIERR [4m+2][k] & PIERR [4m+3][k]
                          * PIERRAR [4m][k] = 0
                          * PIERRAR [4m+1][k] = 0
                          * PIERRAR [4m+3][k] = 0
                          */
    A0123M3,             /* 0, 1, 2, 3 all aggregated.
                          * PIERRAR [4m+3][k] = PIERR[4m][k] & PIERR [4m+1][k] & PIERR [4m+2][k] & PIERR [4m+3][k]
                          * PIERRAR [4m][k] = 0
                          * PIERRAR [4m+1][k] = 0
                          * PIERRAR [4m+2][k] = 0
                          */
    PIE_RESULT_AGGREGATOR_TYPE_END
} rtk_pie_resultAggregatorType_t;

typedef enum rtk_pie_groupCtrlRange_e
{
    GROUP_LBLOCK_0_3 = 0,
    GROUP_LBLOCK_4_7,
    GROUP_LBLOCK_8_11,
    GROUP_LBLOCK_12_15,
    PIE_GROUP_RANGE_END
} rtk_pie_groupCtrlRange_t;

typedef enum rtk_pie_groupCtrl_e
{
    GROUP_NONE = 0, /* No group */
    GROUP_01,       /* Only M+0 and m+1 grouped */
    GROUP_012,      /* M+0, m+1 and m+2 grouped */
    GROUP_0123,     /* M+0, m+1, m+2 and m+3 grouped */
    PIE_GROUP_END
} rtk_pie_groupCtrl_t;

typedef enum rtk_pie_lookupMissAction_e
{
    LOOKUP_MISS_PERMIT = 0,
    LOOKUP_MISS_DROP,
    PIE_LOOKUP_MISS_END
} rtk_pie_lookupMissAction_t;

typedef struct rtk_pie_egrAclLookupCtrl_s
{
    rtk_pie_lookupMissAction_t         flood_lookupMiss_action;
                                       /* Default action on flood packets which miss egress ACL lookup, valid only when FLDEGACLEN enable.
                                        * 0: permit
                                        * 1: drop
                                        */

    rtk_enable_t                       flood_egrAcl_enable; /* Enable/Disable flood packets egress ACL lookup. */

    rtk_pie_lookupMissAction_t         mcast_lookupMiss_action;
                                       /* Default action on multicast packets which miss egress ACL lookup, valid only when MCSTEGACLEN enable.
                                        * 0: permit
                                        * 1: drop
                                        */

    rtk_enable_t                       mcast_egrAcl_enable; /* Enable/Disable multicast packets egress ACL lookup. */

    rtk_enable_t                       other_drop_enable;   /* Enable/Disable packets dropped by other modules to do egress ACL lookup. */
} rtk_pie_egrAclLookupCtrl_t;

typedef enum rtk_pie_counterIndicationMode_e
{
    ACTION_EXECUTION = 0,  /* action execution mode */
    RULE_MATCH,            /* rule match mode */
    PIE_INDICATION_MODE_END
} rtk_pie_counterIndicationMode_t;

typedef struct rtk_pie_policerCtrl_s
{
    uint8   preamble_ifg; /* Packet length including preamble and IFG or not.
                           * 0: exclude
                           * 1: include
                           */
} rtk_pie_policerCtrl_t;


/*
 * range_check
 */
typedef enum rtk_pie_rangeCheck_l4Port_type_e
{
    L4PORT_TYPE_INVALID = 0,  /* 00: entry invalid */
    L4PORT_TYPE_TCP,          /* 01: compare TCP port range */
    L4PORT_TYPE_UDP,          /* 10: compare UDP port range */
    L4PORT_TYPE_TCPORUDP,     /* 11: compare TCP or UDP port range */
    L4PORT_TYPE_END
} rtk_pie_rangeCheck_l4Port_type_t;

typedef enum rtk_pie_rangeCheck_l4Port_direction_e
{
    L4PORT_DIRECTION_SRC = 0,  /* 0: compare source port */
    L4PORT_DIRECTION_DST,      /* 1: compare destination port */
    L4PORT_DIRECTION_END
} rtk_pie_rangeCheck_l4Port_direction_t;

typedef struct rtk_pie_rangeCheck_l4Port_s
{
    uint16  upper_bound;    /* Port range upper bound */
    uint16  lower_bound;    /* Port range lower bound */
    rtk_pie_rangeCheck_l4Port_direction_t   dest_port; /* 0: compare source port, 
                                                        * 1: compare destination port
                                                        */
    rtk_pie_rangeCheck_l4Port_type_t        l4port_type;
} rtk_pie_rangeCheck_l4Port_t;

typedef enum rtk_pie_rangeCheck_vid_type_e
{
    VID_TYPE_INVALID = 0,  /* 00: entry invalid */
    VID_TYPE_INNER,        /* 01: compare inner VID only */
    VID_TYPE_OUTER,        /* 10: compare outer VID only */
    VID_TYPE_INNEROROUTER, /* 11: compare both inner VID and outer VID */
    VID_TYPE_END
} rtk_pie_rangeCheck_vid_type_t;

typedef struct rtk_pie_rangeCheck_vid_s
{
    uint16  ivid_upper_bound;  /* Inner VID range upper bound */
    uint16  ivid_lower_bound;  /* Inner VID range lower bound */
    uint16  ovid_upper_bound;  /* Outer VID range upper bound */
    uint16  ovid_lower_bound;  /* Outer VID range lower bound */
    uint8   src_port_num;      /* Source physical port. */
    rtk_pie_rangeCheck_vid_type_t   vid_type;
} rtk_pie_rangeCheck_vid_t;

typedef enum rtk_pie_rangeCheck_ip_type_e
{
    IP_TYPE_IPV4_SRC = 0,  /* 00: compare IPv4 source IP */
    IP_TYPE_IPV4_DST,      /* 01: compare IPv4 destination IP */
    IP_TYPE_IPV6_SRC,      /* 10: compare IPV6 source IP */
    IP_TYPE_IPV6_DST,      /* 11: compare IPv6 destination IP */
    IP_TYPE_END
} rtk_pie_rangeCheck_ip_type_t;

typedef struct rtk_pie_rangeCheck_ip_s
{
    uint32  ip4_lower_bound;    /* IPv4 Lower bound or IPv6 fourth 32bits of Lower bound */
    uint32  ip4_upper_bound;    /* IPv4 Upper bound or IPv6 fourth 32bits of Upper bound */
    uint32  ip3_lower_bound;    /* IPv4 Lower bound or IPv6 third 32bits of Lower bound  */
    uint32  ip3_upper_bound;    /* IPv4 Upper bound or IPv6 third 32bits of Upper bound  */
    uint32  ip2_lower_bound;    /* IPv4 Lower bound or IPv6 second 32bits of Lower bound */
    uint32  ip2_upper_bound;    /* IPv4 Upper bound or IPv6 second 32bits of Upper bound */
    uint32  ip1_lower_bound;    /* IPv4 Lower bound or IPv6 first 32bits of Lower bound  */
    uint32  ip1_upper_bound;    /* IPv4 Upper bound or IPv6 first 32bits of Upper bound  */
    uint8   valid_mask;         /* Entry valid Mask. */
    rtk_pie_rangeCheck_ip_type_t   ip_type;
} rtk_pie_rangeCheck_ip_t;

typedef struct rtk_pie_rangeCheck_srcPortMask_s
{
    uint32  src_port_mask;   /* Source port Mask */
} rtk_pie_rangeCheck_srcPortMask_t;

/*
 * field_selector
 */
 typedef enum rtk_pie_fieldSelector_startPosition_e
{
    RAW  = 0,                 /* Raw packet(Start after preamble,begin with DA) */
    DOT1Q,                    /* 802.1Q packet (Started after first ITAG ID (0x8100) or OTAG ID (0x88a8)) */
    LLC,                      /* LLC packet */
    IPV4,                     /* IPv4 Packet (Started from start of IPv4 header) */
    ARP,                      /* ARP Packet (Started after Ethernet II EtherType 0x0806) */
    IPV6,                     /* IPv6 Packet (Started from start of IPv6 header) */
    TCP_UDP_ICMP_IGMP_ICMPV6, /* TCP/UDP/ICMP/IGMP/ICMPv6 Packet (Started from start of L4 header) */
    TCP_UDP_PAYLOAD,          /* TCP/UDP Payload (Started after TCP/UDP header) */
    FS_START_POSITION_END
} rtk_pie_fieldSelector_startPosition_t;

typedef struct rtk_pie_fieldSelector_data_s
{
    rtk_pie_fieldSelector_startPosition_t   start;    /* Starting position of field selector */
    uint16                                  offset0;
    uint16                                  offset1;
    uint16                                  offset2;
    uint16                                  offset3;
} rtk_pie_fieldSelector_data_t;

/*
 * pattern_match
 */
typedef struct rtk_pie_patternMatch_data_s
{
    uint8   character;
    uint8   start_bit;
    uint8   last_bit;
    uint8   care_bit;
} rtk_pie_patternMatch_data_t;

typedef struct rtk_pie_patternMatch_content_s
{
    rtk_pie_patternMatch_data_t data[RTK_MAX_NUM_OF_PATTERN_MATCH_DATA];
} rtk_pie_patternMatch_content_t;

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name     : PIE */

/* Function Name:
 *      rtk_pie_init
 * Description:
 *      Initialize PIE module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8328
 * Note:
 *      Must initialize PIE module before calling any PIE APIs.
 */
extern int32
rtk_pie_init(uint32 unit);

/* Sub-module Name :
 *  Flow classification
 *  Ingress ACL
 *  Egress ACL
 *  Egress VID Translation
 */

/* Function Name:
 *      rtk_pie_pieRuleEntryFieldSize_get
 * Description:
 *      Get the field size of PIE entry.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_size - field size of PIE entry.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The unit of field size is bit.
 */
extern int32
rtk_pie_pieRuleEntryFieldSize_get(uint32 unit, rtk_pie_fieldType_t type, uint32 *pField_size);


/* Function Name:
 *      rtk_pie_pieRuleFieldId_get
 * Description:
 *      Get the field ID of field type.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_size - field size of PIE entry.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The ASIC ID of specified field type.
 */
extern int32
rtk_pie_pieRuleFieldId_get(uint32 unit, rtk_pie_fieldType_t type, uint32 *pField_id);

/* Function Name:
 *      rtk_pie_pieRuleEntrySize_get
 * Description:
 *      Get the rule entry size of PIE.
 * Input:
 *      unit        - unit id
 * Output:
 *      pEntry_size - rule entry size of PIE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The unit of entry size is bit.
 */
extern int32
rtk_pie_pieRuleEntrySize_get(uint32 unit, uint32 *pEntry_size);

/* Function Name:
 *      rtk_pie_pieRuleEntryField_get
 * Description:
 *      Get the field data from specified PIE entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - PIE lookup phase
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 *      type          - field type
 * Output:
 *      pData         - field data
 *      pMask         - field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
extern int32
rtk_pie_pieRuleEntryField_get(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_pie_pieRuleEntryField_set
 * Description:
 *      Set the field data to specified PIE entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - PIE lookup phase
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 *      type          - field type
 *      pData         - field data
 *      pMask         - field mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
extern int32
rtk_pie_pieRuleEntryField_set(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_pie_pieRuleEntryField_read
 * Description:
 *      Read the field data from specified PIE entry.
 * Input:
 *      unit      - unit id
 *      phase     - PIE lookup phase
 *      entry_idx - PIE entry index
 *      type      - field type
 * Output:
 *      pData     - field data
 *      pMask     - field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
extern int32
rtk_pie_pieRuleEntryField_read(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_pie_pieRuleEntryField_write
 * Description:
 *      Write the field data to specified PIE entry.
 * Input:
 *      unit      - unit id
 *      phase     - PIE lookup phase
 *      entry_idx - PIE entry index
 *      type      - field type
 *      pData     - field data
 *      pMask     - field mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_PIE_PHASE      - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
extern int32
rtk_pie_pieRuleEntryField_write(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_pie_piePreDefinedRuleEntry_get
 * Description:
 *      Get the pre-defined entry data from buffer.
 * Input:
 *      unit              - unit id
 *      pEntry_buffer     - data buffer of PIE entry 
 * Output:
 *      pPredefined_entry - pre-defined entry data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None.
 */
extern int32
rtk_pie_piePreDefinedRuleEntry_get(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry);

/* Function Name:
 *      rtk_pie_piePreDefinedRuleEntry_set
 * Description:
 *      Set the pre-defined entry data to buffer.
 * Input:
 *      unit              - unit id
 *      pEntry_buffer     - data buffer of PIE entry  
 *      pPredefined_entry - pre-defined entry data
 * Output:
 *      None.  
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None.
 */
extern int32
rtk_pie_piePreDefinedRuleEntry_set(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry);

/* Function Name:
 *      rtk_pie_pieRuleEntry_read
 * Description:
 *      Read the entry data from specified PIE entry.
 * Input:
 *      unit          - unit id
 *      entry_idx     - PIE entry index
 * Output:
 *      pEntry_buffer - data buffer of PIE entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
extern int32
rtk_pie_pieRuleEntry_read(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer);

/* Function Name:
 *      rtk_pie_pieRuleEntry_write
 * Description:
 *      Write the entry data to specified PIE entry.
 * Input:
 *      unit          - unit id
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *          driver will transfer it to physical entry index.
 */
extern int32
rtk_pie_pieRuleEntry_write(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer);

/* Function Name:
 *      rtk_pie_pieRuleEntry_del
 * Description:
 *      Delete the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - PIE entry information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Clear the content of specified PIE entry to be 0.
 *
 *      (2) If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry
 *          and then return to index 0 to continue to content.end.
 */
extern int32
rtk_pie_pieRuleEntry_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleEntry_move
 * Description:
 *      Move the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE entry.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry,
 *      then ASIC stop action.
 */
extern int32
rtk_pie_pieRuleEntry_move(uint32 unit, rtk_pie_movePieContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleEntry_swap
 * Description:
 *      Swap the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE entry.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_pieRuleEntry_swap(uint32 unit, rtk_pie_movePieContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleEntryAction_del
 * Description:
 *      Delete the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - PIE entry information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Clear the content of specified PIE entry and action to be 0.
 *      (2) If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry and action
 *         and then return to index 0 to continue to content.end.
 */
extern int32
rtk_pie_pieRuleEntryAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleEntryAction_move
 * Description:
 *      Move the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE entry and action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry and action,
 *      then ASIC stop action.
 */
extern int32
rtk_pie_pieRuleEntryAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleEntryAction_swap
 * Description:
 *      Swap the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE entry and action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_pieRuleEntryAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleAction_get
 * Description:
 *      Get the PIE action configuration from specified device.
 * Input:
 *      unit       - unit id
 *      action_idx - PIE action index
 * Output:
 *      pAction    - PIE action configuration.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of action_idx is 0~511 in 8328S/8328L; 0~2047 in 8328M.
 */
extern int32
rtk_pie_pieRuleAction_get(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction);

/* Function Name:
 *      rtk_pie_pieRuleAction_set
 * Description:
 *      Set the PIE action configuration to specified device.
 * Input:
 *      unit       - unit id
 *      action_idx - PIE action index
 *      pAction    - PIE action configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of action_idx is 0~511 in 8328S/8328L; 0~2047 in 8328M.
 */
extern int32
rtk_pie_pieRuleAction_set(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction);

/* Function Name:
 *      rtk_pie_pieRuleAction_del
 * Description:
 *      Delete the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - PIE action information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) Clear the content of specified PIE action to be 0.
 *
 *      (2) If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry
 *         and then return to index 0 to continue to content.end.
 */
extern int32
rtk_pie_pieRuleAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleAction_move
 * Description:
 *      Move the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry,
 *      then ASIC stop action.
 */
extern int32
rtk_pie_pieRuleAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRuleAction_swap
 * Description:
 *      Swap the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_pieRuleAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent);

/* Function Name:
 *      rtk_pie_pieRulePolicer_get
 * Description:
 *      Get the PIE policer configuration from specified device.
 * Input:
 *      unit        - unit id
 *      policer_idx - PIE policer index
 * Output:
 *      pPolicer    - PIE policer configuration.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of policer_idx is 0~255.
 */
extern int32
rtk_pie_pieRulePolicer_get(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer);

/* Function Name:
 *      rtk_pie_pieRulePolicer_set
 * Description:
 *      Set the PIE policer configuration to specified device.
 * Input:
 *      unit        - unit id
 *      policer_idx - PIE policer index
 *      pPolicer    - PIE policer configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of policer_idx is 0~255.
 */
extern int32
rtk_pie_pieRulePolicer_set(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer);

/* Function Name:
 *      rtk_pie_pieHitIndication_get
 * Description:
 *      Get the hit indication of PIE rule from specified device.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pStatus    - hit indication status for 128 rules on a logical block.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 *      (2) Read and clear indication bits (128 bits/one time)
 */
extern int32
rtk_pie_pieHitIndication_get(
    uint32                       unit,
    rtk_pie_id_t                 lblock_idx,
    rtk_pie_hitIndicationEntry_t *pStatus);

/* Function Name:
 *      rtk_pie_pieStat_get
 * Description:
 *      Get statistic counter of the log id from the specified device.
 * Input:
 *      unit      - unit id
 *      log_id    - log id
 * Output:
 *      pPkt_cnt  - pointer buffer of packet count
 *      pByte_cnt - pointer buffer of byte count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of log_id is 0~511 in 8328S/8328L; 0~1023 in 8328M
 */
extern int32
rtk_pie_pieStat_get(uint32 unit, rtk_pie_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt);

/* Function Name:
 *      rtk_pie_pieStat_set
 * Description:
 *      Set statistic counter of the log id to the specified device.
 * Input:
 *      unit     - unit id
 *      log_id   - log id
 *      pkt_cnt  - packet count
 *      byte_cnt - byte count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of log_id is 0~511 in 8328S/8328L; 0~1023 in 8328M
 */
extern int32
rtk_pie_pieStat_set(uint32 unit, rtk_pie_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt);

/* Function Name:
 *      rtk_pie_pieStat_clearAll
 * Description:
 *      Clear all statistic counter for the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_pieStat_clearAll(uint32 unit);

/* Function Name:
 *      rtk_pie_pieTemplateSelector_get
 * Description:
 *      Get the template index of specific physical block and lookup phase.
 * Input:
 *      unit          - unit id
 *      pblock_idx    - physical block index
 *      phase         - lookup phase
 * Output:
 *      pTemplate_idx - template index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_PIE_PHASE    - invalid PIE phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of phase is 0~3
 */
extern int32
rtk_pie_pieTemplateSelector_get(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            *pTemplate_idx);

/* Function Name:
 *      rtk_pie_pieTemplateSelector_set
 * Description:
 *      Set the template index of specific physical block and lookup phase.
 * Input:
 *      unit         - unit id
 *      pblock_idx   - physical block index
 *      phase        - lookup phase
 *      template_idx - template index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_PIE_PHASE   - invalid PIE phase
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of phase is 0~3
 *      (3) The valid range of template_idx is 0~15
 */
extern int32
rtk_pie_pieTemplateSelector_set(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            template_idx);

/* template_idx 1-15 (0 for pre-defined), field_idx 0-8 */
/* some field type must be in specific field index, driver need to check it */

/* Function Name:
 *      rtk_pie_pieUserTemplate_get
 * Description:
 *      Get the template content of specific template index.
 * Input:
 *      unit         - unit id
 *      template_idx - template index
 * Output:
 *      pTemplate    - template content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of template_idx is 0~15
 */
extern int32
rtk_pie_pieUserTemplate_get(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate);

/* Function Name:
 *      rtk_pie_pieUserTemplate_set
 * Description:
 *      Set the template content of specific template index.
 * Input:
 *      unit         - unit id
 *      template_idx - template index
 *      pTemplate    - template content
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of template_idx is 0~15
 */
extern int32
rtk_pie_pieUserTemplate_set(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate);

/* Function Name:
 *      rtk_pie_pieL34ChecksumErr_get
 * Description:
 *      Get the operation for L3 and L4 packets checksum error.
 * Input:
 *      unit       - unit id
 * Output:
 *      pOperation - operation for L3 and L4 packets checksum error
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - CHECKSUM_ERR_DOWNGRADE
 *      - CHECKSUM_ERR_PARSE_ANYWAY
 */
extern int32
rtk_pie_pieL34ChecksumErr_get(uint32 unit, rtk_pie_l34ChecksumErrOper_t *pOperation);

/* Function Name:
 *      rtk_pie_pieL34ChecksumErr_set
 * Description:
 *      Set the operation for L3 and L4 packets checksum error.
 * Input:
 *      unit      - unit id
 *      operation - operation for L3 and L4 packets checksum error
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - CHECKSUM_ERR_DOWNGRADE
 *      - CHECKSUM_ERR_PARSE_ANYWAY
 */
extern int32
rtk_pie_pieL34ChecksumErr_set(uint32 unit, rtk_pie_l34ChecksumErrOper_t operation);

/* Function Name:
 *      rtk_pie_pieUserTemplatePayloadOffset_get
 * Description:
 *      Get the payload offset for template field that bound in secific physical block.
 * Input:
 *      unit       - unit id
 *      pblock_idx - physical block index
 *      offset_idx - offset index
 * Output:
 *      pOffset    - payload offset
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of offset_idx is 0~1
 */
extern int32
rtk_pie_pieUserTemplatePayloadOffset_get(
    uint32        unit,
    rtk_pie_id_t  pblock_idx,
    rtk_pie_id_t  offset_idx,
    uint32        *pOffset);

/* Function Name:
 *      rtk_pie_pieUserTemplatePayloadOffset_set
 * Description:
 *      Set the payload offset for template field that bound in secific physical block.
 * Input:
 *      unit       - unit id
 *      pblock_idx - physical block index
 *      offset_idx - offset index
 *      offset     - payload offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of pblock_idx is 0~31
 *      (2) The valid range of offset_idx is 0~1
 */
extern int32
rtk_pie_pieUserTemplatePayloadOffset_set(
    uint32        unit,
    rtk_pie_id_t  pblock_idx,
    rtk_pie_id_t  offset_idx,
    uint32        offset);

/* Function Name:
 *      rtk_pie_pieResultReverse_get
 * Description:
 *      Get the operation of reverse for lookup result.
 * Input:
 *      unit       - unit id
 *      entry_idx  - entry index
 * Output:
 *      pOperation - result reverse
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - RESULT_KEEP
 *      - RESULT_REVERSE
 */
extern int32
rtk_pie_pieResultReverse_get(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  *pOperation);

/* Function Name:
 *      rtk_pie_pieResultReverse_set
 * Description:
 *      Set the operation of reverse for lookup result.
 * Input:
 *      unit       - unit id
 *      entry_idx  - entry index
 *      operation  - result reverse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT   - The module is not initial
 *      RT_ERR_UNIT_ID    - invalid unit id
 *      RT_ERR_ENTRY_INDE - invalid entry index
 *      RT_ERR_INPUT      - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The operation value as following:
 *      - RESULT_KEEP
 *      - RESULT_REVERSE
 */
extern int32
rtk_pie_pieResultReverse_set(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  operation);

/* Function Name:
 *      rtk_pie_pieResultAggregator_get
 * Description:
 *      Get the type of lookup result aggregation.
 * Input:
 *      unit            - unit id
 *      pblockRange_idx - index of physical block range
 *      entry_idx       - physical block entry index
 * Output:
 *      pType           - type of result aggregator
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for physical block 0~3, 1 for physical block 4~7, ..., 7 for physical block 28~31.
 */
extern int32
rtk_pie_pieResultAggregator_get(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   *pType);

/* Function Name:
 *      rtk_pie_pieResultAggregator_set
 * Description:
 *      Set the type of lookup result aggregation.
 * Input:
 *      unit            - unit id
 *      pblockRange_idx - index of physical block range
 *      entry_idx       - physical block entry index
 *      type            - type of result aggregator
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for physical block 0~3, 1 for physical block 4~7, ..., 7 for physical block 28~31.
 */
extern int32
rtk_pie_pieResultAggregator_set(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   type);

/* Function Name:
 *      rtk_pie_pieBlockPriority_get
 * Description:
 *      Get the priority of PIE block.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pPriority  - block priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 */
extern int32
rtk_pie_pieBlockPriority_get(uint32 unit, rtk_pie_id_t lblock_idx, uint32 *pPriority);

/* Function Name:
 *      rtk_pie_pieBlockPriority_set
 * Description:
 *      Set the priority of PIE block.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 *      priority   - block priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 */
extern int32
rtk_pie_pieBlockPriority_set(uint32 unit, rtk_pie_id_t lblock_idx, uint32 priority);

/* Function Name:
 *      rtk_pie_pieGroupCtrl_get
 * Description:
 *      Get the group operation of logical block.
 * Input:
 *      unit            - unit id
 *      lblockRange_idx - index of logical block range
 * Output:
 *      pOperation      - group operation
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for logical block index 0~3, 1 for logical block index 4~7, ..., 3 for logical block index 12~15
 */
extern int32
rtk_pie_pieGroupCtrl_get(
    uint32                      unit,
    rtk_pie_groupCtrlRange_t    lblockRange_idx,
    rtk_pie_groupCtrl_t         *pOperation);

/* Function Name:
 *      rtk_pie_pieGroupCtrl_set
 * Description:
 *      Set the group operation of logical block.
 * Input:
 *      unit            - unit id
 *      lblockRange_idx - index of logical block range
 *      operation       - group operation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      The range index: 0 for logical block index 0~3, 1 for logical block index 4~7, ..., 3 for logical block index 12~15
 */
extern int32
rtk_pie_pieGroupCtrl_set(
    uint32                      unit,
    rtk_pie_groupCtrlRange_t    lblockRange_idx,
    rtk_pie_groupCtrl_t         operation);

/* Function Name:
 *      rtk_pie_pieEgrAclLookupCtrl_get
 * Description:
 *      Get the control configuration of egress ACL lookup.
 * Input:
 *      unit     - unit id
 * Output:
 *      pControl - control configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_pieEgrAclLookupCtrl_get(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl);

/* Function Name:
 *      rtk_pie_pieEgrAclLookupCtrl_set
 * Description:
 *      Set the control configuration of egress ACL lookup.
 * Input:
 *      unit     - unit id
 *      pControl - control configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_pieEgrAclLookupCtrl_set(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl);

/* Function Name:
 *      rtk_pie_piePortLookupPhaseEnable_get
 * Description:
 *      Get the enable status of specific lookup phase on a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phase   - lookup_phase
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 */
extern int32
rtk_pie_piePortLookupPhaseEnable_get(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     *pEnable);

/* Function Name:
 *      rtk_pie_piePortLookupPhaseEnable_set
 * Description:
 *      Set the enable status of specific lookup phase on a port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      phase  - lookup_phase
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PIE_PHASE - invalid PIE phase 
 *      RT_ERR_INPUT     - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 */
extern int32
rtk_pie_piePortLookupPhaseEnable_set(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     enable);

/* Function Name:
 *      rtk_pie_piePortLookupPhaseMiss_get
 * Description:
 *      Get the lookup miss action of specific lookup phase on a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phase   - lookup_phase
 * Output:
 *      pAction - lookup miss action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 *      (2) The action value as following:
 *          - LOOKUP_MISS_PERMIT
 *          - LOOKUP_MISS_DROP
 */
extern int32
rtk_pie_piePortLookupPhaseMiss_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   *pAction);

/* Function Name:
 *      rtk_pie_piePortLookupPhaseMiss_set
 * Description:
 *      Set the lookup miss action of specific lookup phase on a port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      phase  - lookup_phase
 *      action - lookup miss action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT  - The module is not initial
 *      RT_ERR_UNIT_ID   - invalid unit id
 *      RT_ERR_PORT_ID   - invalid port id
 *      RT_ERR_PIE_PHASE - invalid PIE phase
 *      RT_ERR_INPUT     - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of phase is 0~3
 *      (2) The action value as following:
 *          - LOOKUP_MISS_PERMIT
 *          - LOOKUP_MISS_DROP
 */
extern int32
rtk_pie_piePortLookupPhaseMiss_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   action);

/* Function Name:
 *      rtk_pie_pieCounterIndicationMode_get
 * Description:
 *      Get the mode of ACL counter and hit indication.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pMode      - counter and hit indication mode.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 *      (2) The mode value as following:
 *          - ACTION_EXECUTION
 *          - RULE_MATCH
 */
extern int32
rtk_pie_pieCounterIndicationMode_get(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  *pMode);

/* Function Name:
 *      rtk_pie_pieCounterIndicationMode_set
 * Description:
 *      Set the mode of ACL counter and hit indication.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 *      mode       - counter and hit indication mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of lblock_idx is 0~3 in 8328S/8328L; 0~15 in 8328M
 *      (2) The mode value as following:
 *          - ACTION_EXECUTION
 *          - RULE_MATCH
 */
extern int32
rtk_pie_pieCounterIndicationMode_set(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  mode);

/* Function Name:
 *      rtk_pie_piePolicerCtrl_get
 * Description:
 *      Get the control configuration of policer.
 * Input:
 *      unit     - unit id
 * Output:
 *      pControl - control configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_piePolicerCtrl_get(uint32 unit, rtk_pie_policerCtrl_t *pControl);

/* Function Name:
 *      rtk_pie_piePolicerCtrl_set
 * Description:
 *      Set the control configuration of policer.
 * Input:
 *      unit     - unit id
 *      pControl - control configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      Configure the preamble IFG or not?
 */
extern int32
rtk_pie_piePolicerCtrl_set(uint32 unit, rtk_pie_policerCtrl_t *pControl);

/* Sub-module Name :
 *  Range Check
 */

/* Function Name:
 *      rtk_pie_rangeCheckL4Port_get
 * Description:
 *      Get the configuration of L4 port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of L4 port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_pie_rangeCheckL4Port_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData);

/* Function Name:
 *      rtk_pie_rangeCheckL4Port_set
 * Description:
 *      Set the configuration of L4 port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of L4 port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_pie_rangeCheckL4Port_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData);

/* Function Name:
 *      rtk_pie_rangeCheckVid_get
 * Description:
 *      Get the configuration of VID range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of VID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~31 in 8328.
 */
extern int32
rtk_pie_rangeCheckVid_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData);

/* Function Name:
 *      rtk_pie_rangeCheckVid_set
 * Description:
 *      Set the configuration of VID range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~31 in 8328.
 */
extern int32
rtk_pie_rangeCheckVid_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData);

/* Function Name:
 *      rtk_pie_rangeCheckIp_get
 * Description:
 *      Get the configuration of IP range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of IP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_pie_rangeCheckIp_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData);

/* Function Name:
 *      rtk_pie_rangeCheckIp_set
 * Description:
 *      Set the configuration of IP range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of IP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_pie_rangeCheckIp_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData);

/* Function Name:
 *      rtk_pie_rangeCheckSrcPort_get
 * Description:
 *      Get the configuration of source port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of source port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_pie_rangeCheckSrcPort_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData);

/* Function Name:
 *      rtk_pie_rangeCheckSrcPort_set
 * Description:
 *      Set the configuration of source port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of source port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of index is 0~15 in 8328.
 */
extern int32
rtk_pie_rangeCheckSrcPort_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData);

/* Sub-module Name :
 *  Field Selector
 */

/* Function Name:
 *      rtk_pie_fieldSelectorEnable_get
 * Description:
 *      Get the enable status of field selector on specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      fs_idx  - field selector index
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
extern int32
rtk_pie_fieldSelectorEnable_get(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_pie_fieldSelectorEnable_set
 * Description:
 *      Set the enable status of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_PORT_ID     - invalid port id
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
extern int32
rtk_pie_fieldSelectorEnable_set(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t enable);

/* Function Name:
 *      rtk_pie_fieldSelectorContent_get
 * Description:
 *      Get the content of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 * Output:
 *      pFs    - configuration of field selector.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
extern int32
rtk_pie_fieldSelectorContent_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs);

/* Function Name:
 *      rtk_pie_fieldSelectorContent_set
 * Description:
 *      Set the content of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 *      pFs    - configuration of field selector.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      (1) The valid range of fs_idx is 0~1 in 8328.
 */
extern int32
rtk_pie_fieldSelectorContent_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs);

/* Sub-module Name :
 *  Pattern Match
 */

/*Currently, Pattern Match Function only implement on giga port (port24-27)*/
/* Function Name:
 *      rtk_pie_patternMatchEnable_get
 * Description:
 *      Get the enable status of pattern match on specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_patternMatchEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_pie_patternMatchEnable_set
 * Description:
 *      Set the enable status of pattern match on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_pie_patternMatchEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_pie_patternMatchContent_get
 * Description:
 *      Get the content of pattern match on specific port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pContent - content of pattern match.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If you want to check pattern "integer" need to:
 *      put 'r' into data[0], 'e' into data[1], 'g' into data[2] and so on
 *      turn on last_bit of data[0], turn on start_bit of data[6], turn on care_bit of data[0]~data[6].
 */
extern int32
rtk_pie_patternMatchContent_get(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent);

/* Function Name:
 *      rtk_pie_patternMatchContent_set
 * Description:
 *      Set the content of pattern match on specific port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pContent - content of pattern match.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      If you want to check pattern "integer" need to:
 *      put 'r' into data[0], 'e' into data[1], 'g' into data[2] and so on
 *      turn on last_bit of data[0], turn on start_bit of data[6], turn on care_bit of data[0]~data[6].
 */
extern int32
rtk_pie_patternMatchContent_set(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent);

#endif /* __RTK_PIE_H__ */


