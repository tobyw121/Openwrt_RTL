/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008-2009
 * All rights reserved.
 *
 * $Revision: 27686 $
 * $Date: 2013-08-09 10:57:40 +0800 (Fri, 09 Aug 2013) $
 *
 * Purpose : Definition those public ACL APIs and its data type in the SDK .
 *
 * Feature : The file have include the following module and sub-modules
 *            1) Ingress ACL
 *            2) Egress ACL
 *            3) Field Selector
 *            4) Range Check
 *            5) Meter
 *            6) Counter
 *
 */

#ifndef __RTK_ACL_H__
#define __RTK_ACL_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/port.h>
#include <rtk/vlan.h>

/*
 * Symbol Definition
 */
#define RTK_MAX_NUM_OF_ACL_TEMPLATE_FIELD   12
#define RTK_MAX_NUM_OF_ACL_BLOCK_TEMPLATE   3
#define RTK_MAX_SIZE_OF_ACL_USER_FIELD      28  /* maximum user field size among chips*/
#define RTK_MAX_SIZE_OF_ACL_ENTRY           56  /* maximum entry size among chips*/

/*
 * Data Declaration
 */
typedef enum rtk_acl_phase_e
{
    ACL_PHASE_IGR_ACL,
    ACL_PHASE_EGR_ACL,
    ACL_PHASE_END
} rtk_acl_phase_t;

/* Field type in user view */
typedef enum rtk_acl_fieldType_e
{
/*
 * Misc
 */
    USER_FIELD_TEMPLATE_ID,                 /* template ID the entry maps to */
    /* Applicable to 8380 */
    USER_FIELD_DROPPED,                     /* packet dropped before entering ACL */
    USER_FIELD_FRAME_TYPE,                  /* frame type
                                             * 0b00: ARP
                                             * 0b01: L2 only
                                             * 0b10: IPv4
                                             * 0b11: IPv6
                                             */
/*
 * L2
 */
    USER_FIELD_SPN,                         /* source port number.
                                             * It takes the lowest port if the receiving port is a trunk member port
                                             */
    USER_FIELD_SPMM,                        /* source port mask configurations check result */
    USER_FIELD_SPMM_0_1,                    /* source port mask configurations check result , entry0/1*/
    USER_FIELD_SPM,                         /* source port mask */

    USER_FIELD_DMAC,                        /* destination MAC address. The field represents
                                             * 'destination hardware address' of ARP header for ARP/RARP packet
                                             */
    USER_FIELD_SMAC,                        /* source MAC address. The field represents
                                             * 'source hardware address' of ARP header for ARP/RARP packet
                                             */
    USER_FIELD_ITAG_EXIST,                  /* packet with inner tag */
    USER_FIELD_OTAG_EXIST,                  /* packet with outer tag */
    USER_FIELD_ITAG_FMT,                    /* 0b0: inner tag packet
                                             * 0b1: inner priority tag or untag packet
                                             */
    USER_FIELD_OTAG_FMT,                    /* 0b0: outer tag packet
                                             * 0b1: outer priority tag or untag packet
                                             */
    USER_FIELD_FRAME_TYPE_L2,               /* L2 frame type
                                             * 0b00: Ethernet
                                             * 0b01: LLC_SNAP
                                             * 0b10: LLC_Other
                                             * 0b11: Reserved
                                             */
    USER_FIELD_ETAG_EXIST,                  /* packet with extra tag */
    USER_FIELD_ETHERTYPE,                   /* ethernet type/length */
    USER_FIELD_ARPOPCODE,                   /* ARP/RARP Opcode */

    USER_FIELD_OTAG_PRI,                    /* O-TAG priority/Port-based outer priority */
    USER_FIELD_DEI_VALUE,                   /* O-TAG DEI field */
    USER_FIELD_OTAG_VID,                    /* O-TAG VID */
    USER_FIELD_ITAG_PRI,                    /* I-TAG priority/Port-based inner priority */
    USER_FIELD_CFI_VALUE,                   /* I-TAG CFI field */
    USER_FIELD_ITAG_VID,                    /* I-TAG VID */

    USER_FIELD_MGNT_VLAN,                   /* mangement vlan */
    /* Applicable to 8380 */
    USER_FIELD_FLOW_LABEL,                  /* IPv6 flow label */

    /* Applicable to 8390 */
    USER_FIELD_OMPLS_LABEL,                 /* outer MPLS label */
    /* Applicable to 8390 */
    USER_FIELD_OMPLS_EXP,                   /* outer MPLS label EXP */
    /* Applicable to 8390 */
    USER_FIELD_OMPLS_LABEL_EXIST,           /* outer MPLS label exist */
    /* Applicable to 8390 */
    USER_FIELD_IMPLS_LABEL,                 /* inner MPLS label */
    /* Applicable to 8390 */
    USER_FIELD_IMPLS_EXP,                   /* inner MPLS label EXP */
    /* Applicable to 8390 */
    USER_FIELD_IMPLS_LABEL_EXIST,           /* inner MPLS label exist */

    /* Applicable to 8380 */
    USER_FIELD_DATYPE,                      /* 0b00: Unicast DMAC
                                             * 0b01: Broadcast DMAC
                                             * 0b10: Reserved
                                             * 0b11: Multicast DMAC
                                             */
    USER_FIELD_SWITCHMAC,                   /* DMAC is switch MAC */
    /* Applicable to 8380 */
    USER_FIELD_RRCPKEY,                     /* RRCP Key match system setting */
    /* Applicable to 8380 */
    USER_FIELD_RTKPP,                       /* 0b00: unknown
                                             * 0b01: RRCP
                                             * 0b10: RLPP
                                             * 0b11: RLDP
                                             */
    /* Applicable to 8380 */
    USER_FIELD_RRCPVER,                     /* 0b00: RRCPv1
                                             * 0b01: RRCPv3
                                             */
    /* Applicable to 8380 */
    USER_FIELD_RRCPREPLY,                   /* reply field of RRCP */
    /* Applicable to 8380 */
    USER_FIELD_RRCPOP,                      /* 0b00: RRCP hello packet
                                             * 0b01: Reserved
                                             * 0b10: RRCP get
                                             * 0b11: RRCP set
                                             */
    /* Applicable to 8380 */
    USER_FIELD_RRCPHLNULLMAC,               /* NULL uplink MAC in RRCP hello reply */
    USER_FIELD_L2_CRC_ERROR,                /* L2 CRC error */
    /* Applicable to 8390 */
    USER_FIELD_IP4_CHKSUM_ERROR,            /* IPv4 checksum error */
/*
 * L3/L4
 */
    USER_FIELD_L4_PROTO,                    /* layer 4 format
                                             * 0b000: UDP
                                             * 0b001: TCP
                                             * 0b010: ICMP/ICMPv6
                                             * 0b011: IGMP
                                             * 0b1XX: Layer4 Unknown
                                             */
    USER_FIELD_IP4_SIP,                     /* IPv4 source IP. The field represents 'source protocol address' of
                                             * ARP header for ARP/RARP packet
                                             */
    USER_FIELD_IP4_DIP,                     /* IPv4 destination IP. The field represents 'destination protocol address' of
                                             * ARP header for ARP/RARP packet
                                             */
    USER_FIELD_IP6_SIP,
    USER_FIELD_IP6_DIP,
    USER_FIELD_IP4TOS_IP6TC,                /* IPv4 TOS, IPv6 Traffic Class */
    /* Applicable to 8390 */
    USER_FIELD_IP_DSCP,                        /* IPv4 DSCP */
    USER_FIELD_IP4PROTO_IP6NH,              /* IPv4 protocol, IPv6 Next Header */
    USER_FIELD_IP_FLAG,                     /* IP flag */
    /* Applicable to 8390 */
    USER_FIELD_IP_FRAGMENT_OFFSET,          /* IP fragment offset */
    USER_FIELD_IP4_TTL_IP6_HOPLIMIT,        /* IPv4 TTL, IPv6 hop limit
                                             * 0b00: TTL = 0
                                             * 0b01: TTL = 1
                                             * 0b10: 2<= TTL < 255
                                             * 0b11: TTL = 255
                                             */
    USER_FIELD_L4_SRC_PORT,                 /* TCP/UDP source port */
    USER_FIELD_L4_DST_PORT,                 /* TCP/UDP destination port */
    /* Applicable to 8390 */
    USER_FIELD_IP6_ESP_HDR_EXIST,           /* IPv6 packet with ESP header */
    USER_FIELD_IP6_AUTH_HDR_EXIST,          /* IPv6 packet with authentication header */
    USER_FIELD_IP6_DEST_HDR_EXIST,          /* IPv6 packet with destination option header */
    USER_FIELD_IP6_FRAG_HDR_EXIST,          /* IPv6 packet with fragment header */
    USER_FIELD_IP6_ROUTING_HDR_EXIST,       /* IPv6 packet with routing header */
    USER_FIELD_IP6_HOP_HDR_EXIST,           /* IPv6 packet with hop-by-hop header */
    /* Applicable to 8380 */
    USER_FIELD_IP6_MOB_HDR_EXIST,           /* IPv6 packet with mobility header*/
    USER_FIELD_IGMP_TYPE,                   /* IGMP type */
    /* Applicable to 8380 */
    USER_FIELD_IGMP_GROUPIP,                /* IGMP GroupIP in packet payload */
    USER_FIELD_TCP_ECN,                     /* ECN in TCP flag */
    USER_FIELD_TCP_FLAG,                    /* TCP flag */
    USER_FIELD_TCP_NONZEROSEQ,              /* TCP sequence non-zero */
    USER_FIELD_ICMP_CODE,                   /* ICMP/ICMPv6 code */
    USER_FIELD_ICMP_TYPE,                   /* ICMP/ICMPv6 type */
    USER_FIELD_IP_NONZEROOFFSET,            /* IPv4 or IPv6 fragment */
    /* Applicable to 8380 */
    USER_FIELD_PPPOE,                       /* PPPoE packet */
    USER_FIELD_TELNET,                      /* Telnet packet*/
    USER_FIELD_SSH,                         /* SSH packet*/
    USER_FIELD_HTTP,                        /* HTTP packet*/
    USER_FIELD_HTTPS,                       /* HTTPS packet*/
    USER_FIELD_SNMP,                        /* SNMP packet*/
    USER_FIELD_UNKNOWN_L7,                  /* not Telnet/SSH/HTTP/HTTPS/SNMP packet*/
/*
 *Application
 */
    USER_FIELD_VID_RANGE0,                  /* VID range check configuration 15-0 result */
    /* Applicable to 8390 */
    USER_FIELD_VID_RANGE1,                  /* VID range check configuration 31-16 result */
    USER_FIELD_PORT_RANGE,                  /* TCP/UDP port range check result */
    USER_FIELD_IP_RANGE,                    /* IPv4/IPv6 range check result */
    USER_FIELD_LEN_RANGE,                   /* Packet length(CRC included) range check result */

    USER_FIELD_FIELD_SELECTOR_VALID_MSK,    /* Field selector valid mask */
    USER_FIELD_FIELD_SELECTOR0,             /* Field selector 0 output */
    USER_FIELD_FIELD_SELECTOR1,             /* Field selector 1 output */
    USER_FIELD_FIELD_SELECTOR2,             /* Field selector 2 output */
    USER_FIELD_FIELD_SELECTOR3,             /* Field selector 3 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR4,             /* Field selector 4 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR5,             /* Field selector 5 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR6,             /* Field selector 6 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR7,             /* Field selector 7 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR8,             /* Field selector 8 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR9,             /* Field selector 9 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR10,            /* Field selector 10 output */
    /* Applicable to 8390 */
    USER_FIELD_FIELD_SELECTOR11,            /* Field selector 11 output */
    /* Applicable to 8380 */
    USER_FIELD_FLDSEL_RANGE,                /* Field Selector range check result */

/*
 * Field type for Egress ACL ONLY
 */
    /* Applicable to 8390 */
    USER_FIELD_DPM,                         /* destination port mask decided before egress ACL */
    /* Applicable to 8390 */
    USER_FIELD_DPMM,                        /* destination port mask configurations check result decided before egress ACL */
    /* Applicable to 8390 */
    USER_FIELD_DPN,                         /* destination port number decided before egress ACL.
                                             * If there are multiple destination ports, the value will be 0x3f.
                                             * And the value will be 0x3e if there is no destination port.
                                             */
    /* Applicable to 8390 */
    USER_FIELD_UCAST_DA,                    /* unicast DA */
    /* Applicable to 8390 */
    USER_FIELD_MCAST_DA,                    /* multicast DA */
    /* Applicable to 8390 */
    USER_FIELD_SA_LUT_RESULT,               /* SA lookup result. 0: lookup miss 1: lookup hit */
    /* Applicable to 8390 */
    USER_FIELD_DA_LUT_RESULT,               /* DA lookup result. 0: lookup miss 1: lookup hit */
    /* Applicable to 8390 */
    USER_FIELD_NONZERO_DPM,                 /* non-zero destination port mask */
    /* Applicable to 8390 */
    USER_FIELD_L2_DPM,                      /* destination port mask decided by LUT */
    /* Applicable to 8390 */
    USER_FIELD_L2_DPN,                      /* destination port number decided by LUT.
                                             * If there are multiple destination ports, the value will be 0x3f.
                                             * And the value will be 0x3e if there is no destination port
                                             */
    /* Applicable to 8390 */
    USER_FIELD_ATTACK,                      /* packet hit attack prevention criteria */
    /* Applicable to 8390 */
    USER_FIELD_DP,                          /* drop precedence */
    /* Applicable to 8390 */
    USER_FIELD_INT_PRI,                     /* internal priority */
    /* Applicable to 8390 */
    USER_FIELD_IVID,                        /* I-VID given by ingress I-VID decision */
    /* Applicable to 8390 */
    USER_FIELD_OVID,                        /* O-VID given by ingress O-VID decision */
    /* Applicable to 8390 in Egress ACL, Appicable to 8380 in Ingress ACL*/
    USER_FIELD_FWD_VID,                     /* forwarding vid in vlan tag or port-based vid */

    /* Applicable to 8390 */
    USER_FIELD_IGR_ACL_DROP_HIT,            /* Ingress ACL drop action hit */
    /* Applicable to 8390 */
    USER_FIELD_IGR_ACL_COPY_HIT,            /* Ingress ACL copy action hit */
    /* Applicable to 8390 */
    USER_FIELD_IGR_ACL_REDIRECT_HIT,        /* Ingress ACL redirect action hit */
    /* Applicable to 8390 */
    USER_FIELD_IGR_ACL_ROUTING_HIT,         /* Ingress ACL unicast routing action hit */

    USER_FIELD_END
} rtk_acl_fieldType_t;

typedef struct rtk_acl_templateIdx_s
{
    uint32  template_id[RTK_MAX_NUM_OF_ACL_BLOCK_TEMPLATE];/* index to templates */
} rtk_acl_templateIdx_t;

typedef struct rtk_acl_clear_s
{
    uint32  start_idx;  /* clear from this index */
    uint32  end_idx;    /* clear to this index */
} rtk_acl_clear_t;

typedef struct rtk_acl_move_s
{
    uint32  length;     /* how many entries to move */
    uint32  move_from;  /* move from this entry */
    uint32  move_to;    /* move to this entry */
} rtk_acl_move_t;

typedef struct rtk_acl_operation_s
{
    rtk_enable_t reverse;/* reverset the result */
    rtk_enable_t aggr_1; /* aggregate the result of two consecutive rules (rule 0&1, 2&3, ¡K, 2n&2n+1)
                            in the same block */
    rtk_enable_t aggr_2; /* aggregate the result of two rules resided in consecutive blocks
                            (block 0&1, 2&3, ¡K, 2n&2n+1) */
} rtk_acl_operation_t;

/* applicable to 8380 */
typedef enum rtk_acl_lookupMissAct_e
{
    ACL_LOOKUPMISS_ACTION_PERMIT,
    ACL_LOOKUPMISS_ACTION_DROP,
    ACL_LOOKUPMISS_ACTION_END
} rtk_acl_lookupMissAct_t;

typedef enum rtk_acl_actionFwdType_e
{
    ACL_ACTION_FWD_PERMIT,
    ACL_ACTION_FWD_DROP,
    ACL_ACTION_FWD_COPY_TO_PORTID,
    ACL_ACTION_FWD_COPY_TO_PORTMASK,
    ACL_ACTION_FWD_REDIRECT_TO_PORTID,
    ACL_ACTION_FWD_REDIRECT_TO_PORTMASK,
    ACL_ACTION_FWD_UNICAST_ROUTING,
    ACL_ACTION_FWD_FILTERING,
    /* Applicable to 8380 */
    ACL_ACTION_FWD_VLAN_LEAKY,
    ACL_ACTION_FWD_END
} rtk_acl_actionFwdType_t;

typedef enum rtk_acl_igrActionFwdType_e
{
    ACL_IGR_ACTION_FWD_PERMIT = ACL_ACTION_FWD_PERMIT,
    ACL_IGR_ACTION_FWD_DROP = ACL_ACTION_FWD_DROP,
    ACL_IGR_ACTION_FWD_COPY_TO_PORTID = ACL_ACTION_FWD_COPY_TO_PORTID,
    ACL_IGR_ACTION_FWD_COPY_TO_PORTMASK = ACL_ACTION_FWD_COPY_TO_PORTMASK,
    ACL_IGR_ACTION_FWD_REDIRECT_TO_PORTID = ACL_ACTION_FWD_REDIRECT_TO_PORTID,
    ACL_IGR_ACTION_FWD_REDIRECT_TO_PORTMASK = ACL_ACTION_FWD_REDIRECT_TO_PORTMASK,
    ACL_IGR_ACTION_FWD_UNICAST_ROUTING = ACL_ACTION_FWD_UNICAST_ROUTING,
    /* Applicable to 8380 */
    ACL_IGR_ACTION_FWD_VLAN_LEAKY = ACL_ACTION_FWD_VLAN_LEAKY,
    ACL_IGR_ACTION_FWD_END = ACL_ACTION_FWD_END,
} rtk_acl_igrActionFwdType_t;

typedef enum rtk_acl_egrActionFwdType_e
{
    ACL_EGR_ACTION_FWD_PERMIT = ACL_ACTION_FWD_PERMIT,
    ACL_EGR_ACTION_FWD_DROP = ACL_ACTION_FWD_DROP,
    ACL_EGR_ACTION_FWD_COPY_TO_PORTID = ACL_ACTION_FWD_COPY_TO_PORTID,
    ACL_EGR_ACTION_FWD_COPY_TO_PORTMASK = ACL_ACTION_FWD_COPY_TO_PORTMASK,
    ACL_EGR_ACTION_FWD_REDIRECT_TO_PORTID = ACL_ACTION_FWD_REDIRECT_TO_PORTID,
    ACL_EGR_ACTION_FWD_REDIRECT_TO_PORTMASK = ACL_ACTION_FWD_REDIRECT_TO_PORTMASK,
    ACL_EGR_ACTION_FWD_FILTERING = ACL_ACTION_FWD_FILTERING,
    ACL_EGR_ACTION_FWD_END = ACL_ACTION_FWD_END,
} rtk_acl_egrActionFwdType_t;

/* applicable to 8380 */
typedef enum rtk_acl_igrActionDrop_s
{
    ACL_IGR_ACTION_DROP_NOP,
    ACL_IGR_ACTION_DROP_DROP,
    ACL_IGR_ACTION_DROP_WITHDRAW,
    ACL_IGR_ACTION_DROP_END
}rtk_acl_igrActionDrop_t;

typedef struct rtk_acl_igrActionFwd_s
{
    rtk_acl_igrActionFwdType_t fwd_type;
#if defined(CONFIG_SDK_RTL8390)
    uint32 fwd_info; /* ACL_ACTION_FWD_PERMIT:               fwd_info not used
                      * ACL_ACTION_FWD_DROP:                 fwd_info not used
                      * ACL_ACTION_FWD_COPY_TO_PORTID:       fwd_info is the forwarding port
                      * ACL_ACTION_FWD_COPY_TO_PORTMASK:     fwd_info is the index to multicast table
                      * ACL_ACTION_FWD_REDIRECT_TO_PORTID:   fwd_info is the forwarding port
                      * ACL_ACTION_FWD_REDIRECT_TO_PORTMASK: fwd_info is the index to multicast table
                      * ACL_ACTION_FWD_UNICAST_ROUTING:      fwd_info is the index to L2 table NextHop entry
                      */
#endif

#if defined(CONFIG_SDK_RTL8380)
    union
    {
        struct
        {
            uint32 fwd_port_id;         /* forwarding Port ID */
            uint32 cpu_tag;             /* force packet to carry CPU tag 1: force carry; 0: follow other modules */
            uint32 skip_igrStpDrop;     /* skip ingress STP filtering
                                         * 0b0:don't skip
                                         * 0b1:skip
                                         */
            uint32 skip_storm_igrVlan;  /* skip storm suppression filtering for redirected packet( or ingress VLAN filter for copied packet).
                                         * 0b0:don't skip
                                         * 0b1:skip
                                         */
            uint32 force;               /* skip other modules filter (for redirected packet only)
                                         * 0b0:don't skip
                                         * 0b1:skip
                                         */
        }copy_redirect_port;
        struct
        {
            uint32 fwd_idx;             /* index to multicast table for retrieving the port mask */
            uint32 cpu_tag;             /* force packet to carry CPU tag 1: force carry; 0: follow other modules */
            uint32 skip_igrStpDrop;     /* skip ingress STP filtering
                                         * 0b0:don't skip
                                         * 0b1:skip
                                         */
            uint32 skip_storm_igrVlan;  /* skip storm suppression filtering for redirected packet( or ingress VLAN filter for copied packet).
                                         * 0b0:don't skip
                                         * 0b1:skip
                                         */
            uint32 force;               /* skip other modules filter (for redirected packet only)
                                         * 0b0:don't skip
                                         * 0b1:skip
                                         */
        }copy_redirect_portMsk;
        struct
        {
            uint32 idx;                 /* index to L2 table for retrieving the DMAC/DVID/DPORT */
        }route;
    }info;
#endif

} rtk_acl_igrActionFwd_t;

typedef struct rtk_acl_egrActionFwd_s
{
    rtk_acl_egrActionFwdType_t fwd_type;
    uint32 fwd_info;/* ACL_ACTION_FWD_PERMIT:               fwd_info not used
                     * ACL_ACTION_FWD_DROP:                 fwd_info not used
                     * ACL_ACTION_FWD_COPY_TO_PORTID:       fwd_info is the forwarding port
                     * ACL_ACTION_FWD_COPY_TO_PORTMASK:     fwd_info is the index to multicast table
                     * ACL_ACTION_FWD_REDIRECT_TO_PORTID:   fwd_info is the forwarding port
                     * ACL_ACTION_FWD_REDIRECT_TO_PORTMASK: fwd_info is the index to multicast table
                     * ACL_ACTION_FWD_FILTERING:            fwd_info is the index to multicast table
                     */
} rtk_acl_egrActionFwd_t;

typedef enum rtk_acl_statType_e
{
    STAT_TYPE_PACKET_BASED_32BIT,
    STAT_TYPE_BYTE_BASED_64BIT,
    STAT_TYPE_END
} rtk_acl_statType_t;

typedef struct rtk_acl_stat_s
{
    rtk_acl_statType_t  stat_type;
    uint32              stat_idx;/* if index N is used for a 64-bit byte counter, then index 2N & 2N+1
                                  * shouldn't be used for 32-bit packet counter.
                                  */
} rtk_acl_igrActionStat_t, rtk_acl_egrActionStat_t;

typedef enum rtk_acl_actionMirrorType_e
{
    ACL_ACTION_MIRROR_ORIGINAL,
    ACL_ACTION_MIRROR_MODIFIED,
    ACL_ACTION_MIRROR_END
} rtk_acl_actionMirrorType_t;

typedef struct rtk_acl_igrActionMirror_s
{
#if defined(CONFIG_SDK_RTL8380)
    rtk_acl_actionMirrorType_t mirror_type;
#endif
    uint32  mirror_set_idx; /* index to retrieve mirroring information */
} rtk_acl_igrActionMirror_t;

typedef struct rtk_acl_egrActionMirror_s
{
    rtk_acl_actionMirrorType_t mirror_type;
    uint32  mirror_set_idx; /* index to retrieve mirroring information */
} rtk_acl_egrActionMirror_t;

typedef struct rtk_acl_actionMeter_s
{
    uint32  meter_idx;      /* index to retrieve meter/marking information */
} rtk_acl_igrActionMeter_t, rtk_acl_egrActionMeter_t;

typedef enum rtk_acl_igrActionFltType_e
{
    ACL_IGR_ACTION_FLT_SINGLE_PORT ,
    ACL_IGR_ACTION_FLT_MULTIPLE_PORTS ,
    ACL_IGR_ACTION_FLT_END
} rtk_acl_igrActionFltType_t;

typedef enum rtk_acl_igrActionShaperAct_e
{
    ACL_IGR_ACTION_SHAPER_Q0,
    ACL_IGR_ACTION_SHAPER_Q1,
    ACL_IGR_ACTION_SHAPER_Q2,
    ACL_IGR_ACTION_SHAPER_Q3,
    ACL_IGR_ACTION_SHAPER_END
} rtk_acl_igrActionShaperAct_t;

typedef enum rtk_acl_actionVlanAssignType_e
{
    ACL_ACTION_VLAN_ASSIGN_NEW_VID,               /* assign new VID */
    ACL_ACTION_VLAN_ASSIGN_SHIFT_VID,             /* shift VID with specified value */
    ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_INNER_VID,  /* shift from inner VID with specified value */
    ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_OUTER_VID,  /* shift from outer VID with specified value */
    ACL_ACTION_VLAN_ASSIGN_COPY_FROM_INNER_VID,   /* copy from inner VID */
    ACL_ACTION_VLAN_ASSIGN_COPY_FROM_OUTER_VID,   /* copy from outer VID */
    ACL_ACTION_VLAN_ASSIGN_PORT_BASED_OUTER_VID,  /* assign port-based O-VID, applicable to 8380 only */
    ACL_ACTION_VLAN_ASSIGN_PORT_BASED_INNER_VID,  /* assign port-based I-VID, applicable to 8380 only */
    ACL_ACTION_VLAN_ASSIGN_END
} rtk_acl_actionVlanAssignType_t;

typedef enum rtk_acl_igrActionIVlanAssignType_e
{
    ACL_IGR_ACTION_IVLAN_ASSIGN_NEW_VID = ACL_ACTION_VLAN_ASSIGN_NEW_VID,
    ACL_IGR_ACTION_IVLAN_ASSIGN_SHIFT_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID,
    ACL_IGR_ACTION_IVLAN_ASSIGN_SHIFT_FROM_OUTER_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_OUTER_VID,
    /* Applicable to 8380 */
    ACL_IGR_ACTION_IVLAN_ASSIGN_PORT_BASED_INNER_VID = ACL_ACTION_VLAN_ASSIGN_PORT_BASED_INNER_VID,
    ACL_IGR_ACTION_IVLAN_ASSIGN_END = ACL_ACTION_VLAN_ASSIGN_END
} rtk_acl_igrActionIVlanAssignType_t;

typedef enum rtk_acl_igrActionOVlanAssignType_e
{
    ACL_IGR_ACTION_OVLAN_ASSIGN_NEW_VID = ACL_ACTION_VLAN_ASSIGN_NEW_VID,
    ACL_IGR_ACTION_OVLAN_ASSIGN_SHIFT_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID,
    ACL_IGR_ACTION_OVLAN_ASSIGN_SHIFT_FROM_INNER_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_INNER_VID,
    /* Applicable to 8380 */
    ACL_IGR_ACTION_OVLAN_ASSIGN_PORT_BASED_OUTER_VID = ACL_ACTION_VLAN_ASSIGN_PORT_BASED_OUTER_VID,
    ACL_IGR_ACTION_OVLAN_ASSIGN_END = ACL_ACTION_VLAN_ASSIGN_END
} rtk_acl_igrActionOVlanAssignType_t;

typedef enum rtk_acl_egrActionIVlanAssignType_e
{
    ACL_EGR_ACTION_IVLAN_ASSIGN_NEW_VID = ACL_ACTION_VLAN_ASSIGN_NEW_VID,
    ACL_EGR_ACTION_IVLAN_ASSIGN_SHIFT_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID,
    ACL_EGR_ACTION_IVLAN_ASSIGN_COPY_FROM_OUTER_VID = ACL_ACTION_VLAN_ASSIGN_COPY_FROM_OUTER_VID,
    ACL_EGR_ACTION_IVLAN_ASSIGN_SHIFT_FROM_OUTER_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_OUTER_VID,
    ACL_EGR_ACTION_IVLAN_ASSIGN_END = ACL_ACTION_VLAN_ASSIGN_END
} rtk_acl_egrActionIVlanAssignType_t;

typedef enum rtk_acl_egrActionOVlanAssignType_e
{
    ACL_EGR_ACTION_OVLAN_ASSIGN_NEW_VID = ACL_ACTION_VLAN_ASSIGN_NEW_VID,
    ACL_EGR_ACTION_OVLAN_ASSIGN_SHIFT_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID,
    ACL_EGR_ACTION_OVLAN_ASSIGN_COPY_FROM_INNER_VID = ACL_ACTION_VLAN_ASSIGN_COPY_FROM_INNER_VID,
    ACL_EGR_ACTION_OVLAN_ASSIGN_SHIFT_FROM_INNER_VID = ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_INNER_VID,
    ACL_EGR_ACTION_OVLAN_ASSIGN_END = ACL_ACTION_VLAN_ASSIGN_END
} rtk_acl_egrActionOVlanAssignType_t;

typedef struct rtk_acl_igrActionIVlanAssign_s
{
    rtk_acl_igrActionIVlanAssignType_t  vid_assign_type;
    uint32                              vid_shift_sel;      /* 0: positive, 1: negative */
    rtk_vlan_t                          vid_value; /* new VID or value to shift. If the value after shifting
                                                    * is greater than 4095, then it is wraparound for negative shift.
                                                    */
} rtk_acl_igrActionIVlanAssign_t;

typedef struct rtk_acl_igrActionOVlanAssign_s
{
    rtk_acl_igrActionOVlanAssignType_t  vid_assign_type;
    uint32                              vid_shift_sel;      /* 0: positive, 1: negative */
    rtk_vlan_t                          vid_value; /* new VID or value to shift. If the value after shifting
                                                    * is greater than 4095, then it is wraparound for negative shift.
                                                    */
} rtk_acl_igrActionOVlanAssign_t;

typedef struct rtk_acl_egrActionIVlanAssign_s
{
    rtk_acl_egrActionIVlanAssignType_t  vid_assign_type;
    uint32                              vid_shift_sel;      /* 0: positive, 1: negative */
    rtk_vlan_t                          vid_value; /* new VID or value to shift. If the value after shifting
                                                    * is greater than 4095, then it is wraparound for negative shift.
                                                    */
    rtk_enable_t                        tpid_assign;
    uint32                              tpid_idx;  /* index to I-TPID list */
} rtk_acl_egrActionIVlanAssign_t;

typedef struct rtk_acl_egrActionOVlanAssign_s
{
    rtk_acl_egrActionOVlanAssignType_t  vid_assign_type;
    uint32                              vid_shift_sel;      /* 0: positive, 1: negative */
    rtk_vlan_t                          vid_value; /* new VID or value to shift. If the value after shifting
                                                    * is greater than 4095, then it is wraparound for negative shift.
                                                    */
    rtk_enable_t                        tpid_assign;
    uint32                              tpid_idx;  /* index to O-TPID list */
} rtk_acl_egrActionOVlanAssign_t;

typedef struct rtk_acl_igrActionPri_s
{
    rtk_pri_t   pri;
} rtk_acl_igrActionPri_t;

typedef struct rtk_acl_igrActionTpid_s
{
    uint32  tpid_idx;
} rtk_acl_igrActionTpid_t;

typedef struct rtk_acl_igrActionShaper_s
{
    rtk_acl_igrActionShaperAct_t  shaper_act;;
} rtk_acl_igrActionShaper_t;

typedef struct rtk_acl_egrActionPri_s
{
    uint32      pri_act;/* 0: assign priority to the packet directly regardless of destination ports
                         * 1: assign priority to the packet sent(copy) to CPU port.
                         *    Packets that forward to normal ports aren't be affected.
                         */
    rtk_pri_t   pri;
} rtk_acl_egrActionPri_t;

typedef struct rtk_acl_igrActionMpls_s
{
    uint32  mpls_act; /* 0: push single label 1: push double label */
    uint32  mpls_idx; /* index to retrieve MPLS label information. If index N is used for a double
                       * label operation, then index 2N & 2N+1 shouldn't be used for single label operation.
                       */
} rtk_acl_igrActionMpls_t;

typedef struct rtk_acl_igrActionBypass_s
{
    uint32  ibc_sc; /* bypass igress bandwidth control and storm control */
    uint32  igr_stp;/* bypass spanning tree ingress checking */
    uint32  all;    /* bypass all ingress and egress filtering modules EXCEPT ingress ACL drop, ingress ACL meter,
                     * spanning tree ingress checking, ingress bandwidth control and storm control
                     */
} rtk_acl_igrActionBypass_t;

#if defined(CONFIG_SDK_RTL8380)
typedef struct rtk_acl_igrActionFilter_s
{
    rtk_acl_igrActionFltType_t  flt_act;/* 0: filter a single port designated by FWD_PORT_INFO
                                         * 1: filter multiple ports
                                         */
    uint32  flt_info;                   /* flt_act=0: [4:0] filtered Port ID
                                         * flt_act=1: index to multicast table for retrieving the filtered port mask
                                         */
} rtk_acl_igrActionFilter_t;
#endif

typedef enum rtk_acl_actionRemarkType_e
{
    ACL_ACTION_REMARK_INNER_USER_PRI,       /* remark 802.1p priority of inner tag */
    ACL_ACTION_REMARK_OUTER_USER_PRI,       /* remark 802.1p priority of outer tag */
    ACL_ACTION_REMARK_DSCP,                 /* remark DSCP value */
    ACL_ACTION_REMARK_IP_PRECEDENCE,        /* remark IP Precedence bits of TOS byte, IPv4 only */
    ACL_ACTION_REMARK_COPY_IPRI_TO_OPRI,    /* Copy inner tag priority as outer tag priority */
    ACL_ACTION_REMARK_COPY_OPRI_TO_IPRI,    /* Copy outer tag priority as inner tag priority */
    /* applicable to 8380 only */
    ACL_ACTION_REMARK_KEEP_INNER_USER_PRI,  /* keep 802.1p priority of inner tag */
    /* applicable to 8380 only */
    ACL_ACTION_REMARK_KEEP_OUTER_USER_PRI,  /* keep 802.1p priority of outer tag */
    ACL_ACTION_REMARK_END
} rtk_acl_actionRemarkType_t;

#if defined(CONFIG_SDK_RTL8380)
typedef struct rtk_acl_igrActionRmk_s
{
    rtk_acl_actionRemarkType_t  rmk_act;
    uint32  rmk_info;
} rtk_acl_igrActionRmk_t;
#endif

typedef struct rtk_acl_egrActionRemark_s
{
    rtk_acl_actionRemarkType_t  rmk_type;
    /* ACL_ACTION_REMARK_INNER_USER_PRI:   rmk_value is new inner priority
     * ACL_ACTION_REMARK_OUTER_USER_PRI:   rmk_value is new new outer priority
     * ACL_ACTION_REMARK_DSCP:             rmk_value is new DSCP value
     * ACL_ACTION_REMARK_IP_PRECEDENCE:    rmk_value is new IP Precedence value
     * ACL_ACTION_REMARK_COPY_IPRI_TO_OPRI:rmk_value
     * ACL_ACTION_REMARK_COPY_OPRI_TO_IPRI:rmk_value
     */
    uint32 rmk_value;
} rtk_acl_egrActionRemark_t;

typedef enum rtk_acl_actionTagStsFmt_e
{
    ACL_ACTION_TAG_STS_NOP,         /* NULL operation, ACL doesn't assign tag status */
    ACL_ACTION_TAG_STS_UNTAG,
    ACL_ACTION_TAG_STS_TAG,
    ACL_ACTION_TAG_STS_KEEP_CONTENT,/* keep tag content, includes tag format and content */
    ACL_ACTION_TAG_STS_END
} rtk_acl_actionTagStsFmt_t;

#if defined(CONFIG_SDK_RTL8380)
typedef struct rtk_acl_igrActionEgrTagStat_s
{
    rtk_acl_actionTagStsFmt_t itag_sts;
    rtk_acl_actionTagStsFmt_t otag_sts;
} rtk_acl_igrActionEgrTagStat_t;
#endif

typedef struct rtk_acl_egrActionTagSts_s
{
    rtk_acl_actionTagStsFmt_t itag_sts;
    rtk_acl_actionTagStsFmt_t otag_sts;
} rtk_acl_egrActionTagSts_t;

typedef struct rtk_acl_igrAction_s
{
    rtk_enable_t                    fwd_en;
    rtk_acl_igrActionFwd_t          fwd_data;

    rtk_enable_t                    stat_en;
    rtk_acl_igrActionStat_t         stat_data;           /* Each physical counter block contains 128 32-bit packet counter
                                                          * or 64 64-bit byte counter. NOTE: a counter block can't be pointed
                                                          * by ingress and egress ACL concurrently.
                                                          */

    rtk_enable_t                    mirror_en;
    rtk_acl_igrActionMirror_t       mirror_data;

    rtk_enable_t                    meter_en;            /* policing/meter */
    rtk_acl_igrActionMeter_t        meter_data;

    rtk_enable_t                    inner_vlan_assign_en;/* ingress inner VLAN assignment/translation.
                                                          * the action is ignored if packet is inner-untagged
                                                          */
    rtk_acl_igrActionIVlanAssign_t  inner_vlan_data;

    rtk_enable_t                    outer_vlan_assign_en;/* ingress outer VLAN assignment/translation.
                                                          * the action is ignored if packet is outer-untagged
                                                          */
    rtk_acl_igrActionOVlanAssign_t  outer_vlan_data;

    rtk_enable_t                    pri_en;              /* assign internal priority */
    rtk_acl_igrActionPri_t          pri_data;

#if defined(CONFIG_SDK_RTL8390)
    rtk_enable_t                    mpls_en;             /* MPLS tunneling */
    rtk_acl_igrActionMpls_t         mpls_data;
    rtk_enable_t                    bypass_en;
    rtk_acl_igrActionBypass_t       bypass_data;
#endif


#if defined(CONFIG_SDK_RTL8380)
    rtk_acl_igrActionDrop_t         drop_data;
    rtk_enable_t                    filter_en;
    rtk_acl_igrActionFilter_t       filter_data;
    rtk_enable_t                    remark_en;
    rtk_acl_igrActionRmk_t          remark_data;
    rtk_enable_t                    egrTagStat_en;
    rtk_acl_igrActionEgrTagStat_t   egrTagStat_data;
    rtk_enable_t                    cpu_pri_en;
    rtk_acl_igrActionPri_t          cpu_pri_data;
    rtk_enable_t                    otpid_en;
    rtk_acl_igrActionTpid_t         otpid_data;
    rtk_enable_t                    itpid_en;
    rtk_acl_igrActionTpid_t         itpid_data;
    rtk_enable_t                    shaper_en;
    rtk_acl_igrActionShaper_t       shaper_data;
#endif

} rtk_acl_igrAction_t;

typedef struct rtk_acl_egrAction_s
{
    rtk_enable_t                    fwd_en;
    rtk_acl_egrActionFwd_t          fwd_data;

    rtk_enable_t                    stat_en;
    rtk_acl_egrActionStat_t         stat_data;

    rtk_enable_t                    mirror_en;
    rtk_acl_egrActionMirror_t       mirror_data;

    rtk_enable_t                    meter_en;            /* policing/meter */
    rtk_acl_egrActionMeter_t        meter_data;

    rtk_enable_t                    inner_vlan_assign_en;/* egress inner VLAN assignment/translation.
                                                          * the action is ignored if packet is inner-untagged
                                                          */
    rtk_acl_egrActionIVlanAssign_t  inner_vlan_data;

    rtk_enable_t                    outer_vlan_assign_en;/* egress outer VLAN assignment/translation.
                                                          * the action is ignored if packet is outer-untagged
                                                          */
    rtk_acl_egrActionOVlanAssign_t  outer_vlan_data;

    rtk_enable_t                    pri_en;              /* assign internal priority */
    rtk_acl_egrActionPri_t          pri_data;

    rtk_enable_t                    rmk_en;
    rtk_acl_egrActionRemark_t       rmk_data;

    rtk_enable_t                    tag_sts_en;
    rtk_acl_egrActionTagSts_t       tag_sts_data;
} rtk_acl_egrAction_t;

typedef union rtk_acl_action_s
{
    rtk_acl_igrAction_t igr_acl;
#if defined(CONFIG_SDK_RTL8390)
    rtk_acl_egrAction_t egr_acl;
#endif
} rtk_acl_action_t;

typedef enum rtk_acl_meterType_e
{
    METER_TYPE_INVALID = 0,
    /* Applicable to 8380 */
    METER_TYPE_SLB,
    /* Applicable to 8390 */
    METER_TYPE_DLB,
    /* Applicable to 8390 */
    METER_TYPE_SRTCM,
    /* Applicable to 8390 */
    METER_TYPE_TRTCM,
    METER_TYPE_END
} rtk_acl_meterType_t;

typedef enum rtk_acl_meterMode_e
{
    METER_MODE_BYTE = 0,
    METER_MODE_PACKET,
    METER_MODE_END
} rtk_acl_meterMode_t;

typedef struct rtk_acl_meterBurstSize_s
{
#if defined(CONFIG_SDK_RTL8380)
    uint32 slb_lb0bs;  /* single leaky bucket type leaky bucket 0 burst size */
    uint32 slb_lb1bs;  /* single leaky bucket type leaky bucket 1 burst size */
#endif
#if defined(CONFIG_SDK_RTL8390)
    uint32 dlb_lb0bs;  /* double leaky bucket type leaky bucket 0 burst size */
    uint32 dlb_lb1bs;  /* double leaky bucket type leaky bucket 1 burst size */
    uint32 srtcm_cbs;  /* single rate three color type committed burst size */
    uint32 srtcm_ebs;  /* single rate three color type excess burst size */
    uint32 trtcm_cbs;  /* two rate three color type committed burst size */
    uint32 trtcm_pbs;  /* two rate three color type peak burst size */
#endif
} rtk_acl_meterBurstSize_t;

typedef struct rtk_acl_meterEntry_s
{
#if defined(CONFIG_SDK_RTL8390)
    rtk_acl_meterType_t type;   /* 0x0: invalid, 0x1:SLB, 0x2: DLB; 0x3: srTCM; 0x4: trTCM. */
    uint8   color_aware;        /* Meter is color aware. This bit takes effect when meter is srTCM or trTCM.*/
    uint32  lb0_rate;           /* Leaky bucket 0 rate (DLB) or Committed Information Rate (srTCM or trTCM)*/
    uint32  lb1_rate;           /* Leaky bucket 1 rate ( DLB) or Peak Information Rate (trTCM), this rate is not taken used for srTCM*/
#endif
#if defined(CONFIG_SDK_RTL8380)
    uint32 enable;              /* enable state. Should set this bit each time changing the parameters of this entry */
    uint32 rate;                /* 18-bit wide rate setting in units of pps, or 16-bit wide rate setting in units of 16Kbps */
    uint32 thr_grp;             /* select threshold group. 0: group 0, 1: group 1 */
#endif
} rtk_acl_meterEntry_t;

/* template field in user view */
typedef enum rtk_acl_templateFieldType_e
{
    TMPLTE_FIELD_SPMMASK = 0,
    TMPLTE_FIELD_SPM0,                  /* SPM[15:0]   */
    TMPLTE_FIELD_SPM1,                  /* SPM[31:16]  */
    /* Applicable to 8390 */
    TMPLTE_FIELD_SPM2,                  /* SPM[47:32]  */
    /* Applicable to 8390 */
    TMPLTE_FIELD_SPM3,                  /* SPM[52:48]  */
    TMPLTE_FIELD_DMAC0,                 /* DMAC[15:0]  */
    TMPLTE_FIELD_DMAC1,                 /* DMAC[31:16] */
    TMPLTE_FIELD_DMAC2,                 /* DMAC[47:32] */
    TMPLTE_FIELD_SMAC0,                 /* SMAC[15:0]  */
    TMPLTE_FIELD_SMAC1,                 /* SMAC[31:16] */
    TMPLTE_FIELD_SMAC2,                 /* SMAC[47:32] */
    TMPLTE_FIELD_ETHERTYPE,
    TMPLTE_FIELD_ARPOPCODE,
    TMPLTE_FIELD_OTAG,
    TMPLTE_FIELD_ITAG,
    TMPLTE_FIELD_SIP0,                  /* IP4 SIP[15:0]/IP6 SIP[15:0]   */
    TMPLTE_FIELD_SIP1,                  /* IP4 SIP[31:16]/IP6 SIP[31:16] */
    TMPLTE_FIELD_SIP2,                  /* IP6 SIP[47:32]  */
    TMPLTE_FIELD_SIP3,                  /* IP6 SIP[63:48]  */
    TMPLTE_FIELD_SIP4,                  /* IP6 SIP[79:64]  */
    TMPLTE_FIELD_SIP5,                  /* IP6 SIP[95:80]  */
    TMPLTE_FIELD_SIP6,                  /* IP6 SIP[111:96] */
    TMPLTE_FIELD_SIP7,                  /* IP6 SIP[127:112]*/
    TMPLTE_FIELD_DIP0,                  /* IP4 DIP[15:0]/IP6 DIP[15:0]   */
    TMPLTE_FIELD_DIP1,                  /* IP4 DIP[31:16]/IP6 DIP[31:16] */
    TMPLTE_FIELD_DIP2,                  /* IP6 DIP[47:32]  */
    TMPLTE_FIELD_DIP3,                  /* IP6 DIP[63:48]  */
    TMPLTE_FIELD_DIP4,                  /* IP6 DIP[79:64]  */
    TMPLTE_FIELD_DIP5,                  /* IP6 DIP[95:80]  */
    TMPLTE_FIELD_DIP6,                  /* IP6 DIP[111:96] */
    TMPLTE_FIELD_DIP7,                  /* IP6 DIP[127:112]*/
    TMPLTE_FIELD_IP_TOS_PROTO,
    /* Applicable to 8390 */
    TMPLTE_FIELD_IP_FLAG_OFFSET,
    TMPLTE_FIELD_L4_SPORT,
    TMPLTE_FIELD_L4_DPORT,
    TMPLTE_FIELD_L34_HEADER,
    TMPLTE_FIELD_ICMP_IGMP,
    /* Applicable to 8380 */
    TMPLTE_FIELD_FLOW_LABEL,
    /* Applicable to 8390 */
    TMPLTE_FIELD_VID_RANG0,             /* VID range check mask for configuration 0-15 */
    /* Applicable to 8390 */
    TMPLTE_FIELD_VID_RANG1,             /* VID range check mask for configuration 16-31 */
    /* Applicable to 8390 */
    TMPLTE_FIELD_IP_L4PORT_RANG,
    TMPLTE_FIELD_IP_LEN_RANG,
    /* Applicable to 8380 */
    TMPLTE_FIELD_IP_RANGE,
    /* Applicable to 8380 */
    TMPLTE_FIELD_RANGE_CHK,
    TMPLTE_FIELD_FIELD_SELECTOR_VALID,
    TMPLTE_FIELD_FIELD_SELECTOR_0,
    TMPLTE_FIELD_FIELD_SELECTOR_1,
    TMPLTE_FIELD_FIELD_SELECTOR_2,
    TMPLTE_FIELD_FIELD_SELECTOR_3,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_4,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_5,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_6,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_7,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_8,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_9,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_10,
    /* Applicable to 8390 */
    TMPLTE_FIELD_FIELD_SELECTOR_11,
    /* Applicable to 8390 */
    TMPLTE_FIELD_OLABEL,
    /* Applicable to 8390 */
    TMPLTE_FIELD_ILABEL,
    /* Applicable to 8390 */
    TMPLTE_FIELD_OILABEL,
    /*Egress ACL fields*/
    /* Applicable to 8390 */
    TMPLTE_FIELD_DPMMASK,
    /* Applicable to 8390 */
    TMPLTE_FIELD_DPM0,                  /* DPM[15:0]    */
    /* Applicable to 8390 */
    TMPLTE_FIELD_DPM1,                  /* DPM[31:16]   */
    /* Applicable to 8390 */
    TMPLTE_FIELD_DPM2,                  /* DPM[47:32]   */
    /* Applicable to 8390 */
    TMPLTE_FIELD_DPM3,                  /* DPM[52:48]   */
    /* Applicable to 8390 */
    TMPLTE_FIELD_L2DPM0,                /* L2DPM[15:0]  */
    /* Applicable to 8390 */
    TMPLTE_FIELD_L2DPM1,                /* L2DPM[31:16] */
    /* Applicable to 8390 */
    TMPLTE_FIELD_L2DPM2,                /* L2DPM[47:32] */
    /* Applicable to 8390 */
    TMPLTE_FIELD_L2DPM3,                /* L2DPM[52:48] */
    /* Applicable to 8390 */
    TMPLTE_FIELD_IVLAN,
    /* Applicable to 8390 */
    TMPLTE_FIELD_OVLAN,
    TMPLTE_FIELD_FWD_VID,
    TMPLTE_FIELD_END
} rtk_acl_templateFieldType_t;

typedef struct rtk_acl_template_s
{
    rtk_acl_templateFieldType_t  field[RTK_MAX_NUM_OF_ACL_TEMPLATE_FIELD];
} rtk_acl_template_t;

typedef enum rtk_acl_blockResultMode_e
{
    ACL_BLOCK_RESULT_SINGLE = 0,/* a block only ouput a single hit result */
    ACL_BLOCK_RESULT_MULTIPLE,  /* a block can ouput multiple hit result */
    ACL_BLOCK_RESULT_END
} rtk_acl_blockResultMode_t;

typedef enum rtk_acl_blockgroup_e
{
    /* Applicable to 8380 */
    ACL_BLOCK_GROUP_1 = 0, /* group the next block*/
    /* Applicable to 8390 */
    ACL_BLOCK_GROUP_2,     /* group 2 adjacent blocks(2N,2N+1) to a logical block */
    /* Applicable to 8390 */
    ACL_BLOCK_GROUP_4,     /* group 4 adjacent blocks(4N,4N+1,4N+2,4N+3) to a logical block */
    /* Applicable to 8390 */
    ACL_BLOCK_GROUP_8,     /* group 8 adjacent blocks(8N,8N+1,8N+2,8N+3,8N+4,8N+5,8N+6,8N+7) to a logical block */
    /* Applicable to 8390 */
    ACL_BLOCK_GROUP_ALL,   /* group all blocks to a logical block */
    ACL_BLOCK_GROUP_END
} rtk_acl_blockGroup_t;

/*
 * range_check
 */
typedef enum rtk_acl_rangeCheck_l4Port_direction_e
{
    RNGCHK_L4PORT_DIRECTION_SRC = 0,/* source port */
    RNGCHK_L4PORT_DIRECTION_DST,    /* destination port */
    /* Applicable to 8380 */
    RNGCHK_L4PORT_DIRECTION_SRC_DST,/* source port or destination port */
    RNGCHK_L4PORT_DIRECTION_END
} rtk_acl_rangeCheck_l4Port_direction_t;

typedef struct rtk_acl_rangeCheck_l4Port_s
{
    uint32  upper_bound;/* TCP/UDP port upper bound */
    uint32  lower_bound;/* TCP/UDP port lower bound */
#if defined(CONFIG_SDK_RTL8390)
    uint32  reverse;    /* reverse operation */
#endif
    rtk_acl_rangeCheck_l4Port_direction_t    l4port_dir;
} rtk_acl_rangeCheck_l4Port_t;

typedef enum rtk_acl_rangeCheck_vid_type_e
{
    RNGCHK_VID_TYPE_INNER,/* inner VID */
    RNGCHK_VID_TYPE_OUTER,/* outer VID */
    RNGCHK_VID_TYPE_END
} rtk_acl_rangeCheck_vid_type_t;

typedef struct rtk_acl_rangeCheck_vid_s
{
    uint32  vid_upper_bound;/* VID range upper bound */
    uint32  vid_lower_bound;/* VID range lower bound */
#if defined(CONFIG_SDK_RTL8390)
    uint32  reverse;        /* reverse operation */
#endif
    rtk_acl_rangeCheck_vid_type_t   vid_type;
} rtk_acl_rangeCheck_vid_t;

typedef enum rtk_acl_rangeCheck_ip_type_e
{
    RNGCHK_IP_TYPE_IPV4_SRC = 0,   /* IPv4 source IP */
    RNGCHK_IP_TYPE_IPV4_DST,       /* IPv4 destination IP */
    RNGCHK_IP_TYPE_IPV6_SRC,       /* IPV6 source IP */
    RNGCHK_IP_TYPE_IPV6_DST,       /* IPv6 destination IP */
    RNGCHK_IP_TYPE_IPV6_SRC_SUFFIX,/* IPV6 source IP suffix[63:0] */
    RNGCHK_IP_TYPE_IPV6_DST_SUFFIX,/* IPv6 destination IP suffix[63:0] */
    RNGCHK_IP_TYPE_END
} rtk_acl_rangeCheck_ip_type_t;

typedef struct rtk_acl_rangeCheck_ip_s
{
    uint32  ip_lower_bound;/* IP address Lower bound */
    uint32  ip_upper_bound;/* IP address Upper bound */
#if defined(CONFIG_SDK_RTL8390)
    uint32  reverse;       /* reverse operation */
#endif
    rtk_acl_rangeCheck_ip_type_t   ip_type;
} rtk_acl_rangeCheck_ip_t;

typedef struct rtk_acl_rangeCheck_packetLen_s
{
    uint32  upper_bound;/* packet length range upper bound */
    uint32  lower_bound;/* packet length range lower bound */
#if defined(CONFIG_SDK_RTL8390)
    uint32  reverse;    /* reverse operation */
#endif
} rtk_acl_rangeCheck_packetLen_t;

/* Applicable to 8380 */
typedef enum rtk_acl_rangeCheck_fieldSelector_type_e
{
    RNGCHK_FIELDSELECTOR_TYPE_FIELDSELECTOR0 = 0,
    RNGCHK_FIELDSELECTOR_TYPE_FIELDSELECTOR1,
    RNGCHK_FIELDSELECTOR_TYPE_END
} rtk_acl_rangeCheck_fieldSelector_type_t;

/* Applicable to 8380 */
typedef struct rtk_acl_rangeCheck_fieldSelector_s
{
    uint32  upper_bound;/* field selector range upper bound */
    uint32  lower_bound;/* field selector range lower bound */
    rtk_acl_rangeCheck_fieldSelector_type_t   fieldSelector_type;
} rtk_acl_rangeCheck_fieldSelector_t;

typedef struct rtk_acl_rangeCheck_portMask_s
{
    rtk_portmask_t port_mask;/* source/destination port Mask */
} rtk_acl_rangeCheck_portMask_t;

/*
 * field_selector
 */
 typedef enum rtk_acl_fieldSelector_startPosition_e
{
    FS_START_POS_RAW  = 0, /* Raw packet(start after preamble,begin with DA) */
    /* Applicable to 8390 */
    FS_START_POS_LLC,      /* LLC packet(start with length "0000-05FF") */
    /* Applicable to 8380 */
    FS_START_POS_L2,       /* Start from Ethertype(EthernetII)/Length(RFC_1042/SNAP_OTHER),  VLAN tag is bypassed */
    FS_START_POS_L3,       /* L3 packet(start after EtherType field, VLAN tag is bypassed) */
    /* Applicable to 8390 */
    FS_START_POS_ARP,      /* ARP Packet(start from ARP/RARP header) */
    /* Applicable to 8390 */
    FS_START_POS_IPV4,     /* IPv4 Packet(start from IPv4 header) */
    /* Applicable to 8390 */
    FS_START_POS_IPV6,     /* IPv6 Packet(start from IPv6 header) */
    /* Applicable to 8390 */
    FS_START_POS_IP,       /* IP payload(start from IP payload, also means start of layer 4 packet) */
    FS_START_POS_L4,       /* Start from TCP/UDP/ICMP/IGMP header. For 8390,ICMP/IGMP is not supported */
    FS_START_POS_END
} rtk_acl_fieldSelector_startPosition_t;

typedef struct rtk_acl_fieldSelector_data_s
{
    rtk_acl_fieldSelector_startPosition_t   start;  /* starting position of field selector */
    uint32                                  offset; /* offest in byte */
} rtk_acl_fieldSelector_data_t;

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name     : ACL */

/* Function Name:
 *      rtk_acl_init
 * Description:
 *      Initialize ACL module of the specified device.
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
 *      8390, 8380
 * Note:
 *      Must initialize ACL module before calling any ACL APIs.
 */
extern int32
rtk_acl_init(uint32 unit);

/* Function Name:
 *      rtk_acl_partition_get
 * Description:
 *      Get the acl partition configuration.
 * Input:
 *      unit       - unit id
 * Output:
 *      pPartition - pointer buffer of partition value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      (1) Each block contains 128 ACL rules.
 *      (2) Assumed total block is Y. If partition value is set to X, means block 0~(X-1)
 *          are for ingress ACL while block X~(Y-1) are for egress acl.
 *          Thus, ingress ACL rule is assigned to logical index 0~(X*128-1), egress ACL rule
 *          is assigned to logical index 0~((Y-X)*128-1).
 */
extern int32
rtk_acl_partition_get(uint32 unit, uint32 *pPartition);

/* Function Name:
 *      rtk_acl_partition_set
 * Description:
 *      Set the acl partition configuration.
 * Input:
 *      unit      - unit id
 *      partition - partition value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - partition value is out of range
 * Applicable:
 *      8390
 * Note:
 *      (1) Each block contains 128 ACL rules.
 *      (2) Assumed total block is Y. If partition value is set to X, means block 0~(X-1)
 *          are for ingress ACL while block X~(Y-1) are for egress acl.
 *          Thus, ingress ACL rule is assigned to logical index 0~(X*128-1), egress ACL rule
 *          is assigned to logical index 0~((Y-X)*128-1).
 */
extern int32
rtk_acl_partition_set(uint32 unit, uint32 partition);

/* Function Name:
 *      rtk_acl_blockPwrEnable_get
 * Description:
 *      Get the acl block power state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 * Output:
 *      pEnable   - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - block index is out of range
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The rule data are cleared if the block power is disabled.
 */
extern int32
rtk_acl_blockPwrEnable_get(uint32 unit, uint32 block_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_acl_blockPwrEnable_set
 * Description:
 *      Set the acl block power state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 *      enable    - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - block index is out of range
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      The rule data are cleared if the block power is disabled.
 */
extern int32
rtk_acl_blockPwrEnable_set(uint32 unit, uint32 block_idx, rtk_enable_t enable);

/* Function Name:
 *      rtk_acl_blockLookupEnable_get
 * Description:
 *      Get the acl block lookup state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 * Output:
 *      pEnable   - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - block index is out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The rule data are kept regardless of the lookup status.
 *      (2) The lookup result is always false if the lookup state is disabled.
 */
extern int32
rtk_acl_blockLookupEnable_get(uint32 unit, uint32 block_idx, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_acl_blockLookupEnable_set
 * Description:
 *      Set the acl block lookup state.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 *      enable    - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - block index is out of range
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) The rule data are kept regardless of the lookup status.
 *      (2) The lookup result is always false if the lookup state is disabled.
 */
extern int32
rtk_acl_blockLookupEnable_set(uint32 unit, uint32 block_idx, rtk_enable_t enable);

/* Function Name:
 *      rtk_acl_portLookupEnable_get
 * Description:
 *      Get the acl per port lookup state.
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
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_acl_portLookupEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_acl_portLookupEnable_set
 * Description:
 *      Set the acl per port lookup state.
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
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_acl_portLookupEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_acl_lookupMissAct_get
 * Description:
 *      Get the acl per port default action when acl look up miss.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pLmAct - default action of acl look up miss
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_acl_lookupMissAct_get(uint32 unit, rtk_port_t port, rtk_acl_lookupMissAct_t *pLmAct);

/* Function Name:
 *      rtk_acl_lookupMissAct_set
 * Description:
 *     Set the acl per port default action when acl look up miss.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      lmAct - default action of acl look up miss
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
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_acl_lookupMissAct_set(uint32 unit, rtk_port_t port, rtk_acl_lookupMissAct_t lmAct);

/* Function Name:
 *      rtk_acl_ruleEntryFieldSize_get
 * Description:
 *      Get the field size of ACL entry.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_size - field size of ACL entry.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The unit of size is bit.
 */
extern int32
rtk_acl_ruleEntryFieldSize_get(uint32 unit, rtk_acl_fieldType_t type, uint32 *pField_size);

/* Function Name:
 *      rtk_acl_ruleEntrySize_get
 * Description:
 *      Get the rule entry size of ACL.
 * Input:
 *      unit        - unit id
 *      phase       - ACL lookup phase
 * Output:
 *      pEntry_size - rule entry size of ingress ACL
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The unit of size is byte.
 */
extern int32
rtk_acl_ruleEntrySize_get(uint32 unit, rtk_acl_phase_t phase, uint32 *pEntry_size);

/* Function Name:
 *      rtk_acl_ruleValidate_get
 * Description:
 *      Validate ACL rule without modifying the content
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - entry index
 * Output:
 *      pValid    - pointer buffer of valid state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_ruleValidate_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              *pValid);

/* Function Name:
 *      rtk_acl_ruleValidate_set
 * Description:
 *      Validate ACL rule without modifying the content
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - entry index
 *      valid     - valid state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ACL_PHASE   - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_ruleValidate_set(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              valid);

/* Function Name:
 *      rtk_acl_ruleEntry_read
 * Description:
 *      Read the entry data from specified ACL entry.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - entry index
 * Output:
 *      pEntry_buffer - data buffer of ACL entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API reads the rule data from chip to entry buffer. Use rtk_acl_ruleEntryField_get to
 *      get the field data from the entry buffer.
 */
extern int32
rtk_acl_ruleEntry_read(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer);

/* Function Name:
 *      rtk_acl_ruleEntry_write
 * Description:
 *      Write the entry data to specified ACL entry.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - entry index
 *      pEntry_buffer - data buffer of ACL entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Use rtk_acl_ruleEntryField_set to fill the entry buffer and then use this API to write the
 *      entry buffer to the chip.
 */
extern int32
rtk_acl_ruleEntry_write(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer);

/* Function Name:
 *      rtk_acl_ruleEntryField_get
 * Description:
 *      Get the field data from specified ACL entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - ACL entry index
 *      pEntry_buffer - data buffer of ACL entry
 *      type          - field type
 * Output:
 *      pData         - pointer buffer of field data
 *      pMask         - pointer buffer of field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API reads the field data/mask from the entry buffer. Use rtk_acl_ruleEntry_read to
 *      read the rule data to the entry buffer.
 */
extern int32
rtk_acl_ruleEntryField_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_acl_ruleEntryField_set
 * Description:
 *      Set the field data to specified ACL entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - ACL lookup phase
 *      entry_idx     - ACL entry index
 *      pEntry_buffer - data buffer of ACL entry
 *      type          - field type
 *      pData         - pointer buffer of field data
 *      pMask         - pointer buffer of field mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      The API writes the field data/mask to the entry buffer. After the fields are configured,
 *      use rtk_acl_ruleEntry_write to write the entry buffer to ASIC at once.
 */
extern int32
rtk_acl_ruleEntryField_set(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_acl_ruleEntryField_read
 * Description:
 *      Read the field data from specified ACL entry.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 *      type      - field type
 * Output:
 *      pData     - pointer buffer of field data
 *      pMask     - pointer buffer of field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_ruleEntryField_read(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_acl_ruleEntryField_write
 * Description:
 *      Write the field data to specified ACL entry.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 *      type      - field type
 *      pData     - pointer buffer of field data
 *      pMask     - pointer buffer of field mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX    - invalid entry index
 *      RT_ERR_ACL_FIELD_TYPE - invalid entry field type
 *      RT_ERR_NULL_POINTER   - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_ruleEntryField_write(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    rtk_acl_fieldType_t type,
    uint8               *pData,
    uint8               *pMask);

/* Function Name:
 *      rtk_acl_ruleOperation_get
 * Description:
 *      Get ACL rule operation.
 * Input:
 *      unit       - unit id
 *      phase      - ACL lookup phase
 *      entry_idx  - ACL entry index
 * Output:
 *      pOperation - operation configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) For reverse operation, valid index is N where N = 0,1,2...
 *      (2) For aggr_1 operation, index must be 2N where N = 0,1,2...
 *      (3) For aggr_2 operation, index must be 2N+256M where N,M = 0,1,2...
 *      (4) For aggregating 4 rules, both aggr_1 and aggr_2 must be enabled.
 */
extern int32
rtk_acl_ruleOperation_get(
    uint32                  unit,
    rtk_acl_phase_t         phase,
    rtk_acl_id_t            entry_idx,
    rtk_acl_operation_t     *pOperation);

/* Function Name:
 *      rtk_acl_ruleOperation_set
 * Description:
 *      Set ACL rule operation.
 * Input:
 *      unit       - unit id
 *      phase      - ACL lookup phase
 *      entry_idx  - ACL entry index
 *      pOperation - operation configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) For reverse operation, valid index is N where N = 0,1,2...
 *      (2) For aggr_1 operation, index must be 2N where N = 0,1,2...
 *      (3) For aggr_2 operation, index must be 2N+256M where N,M = 0,1,2...
 *      (4) For aggregating 4 rules, both aggr_1 and aggr_2 must be enabled.
 */
extern int32
rtk_acl_ruleOperation_set(
    uint32                  unit,
    rtk_acl_phase_t         phase,
    rtk_acl_id_t            entry_idx,
    rtk_acl_operation_t     *pOperation);

/* Function Name:
 *      rtk_acl_ruleAction_get
 * Description:
 *      Get the ACL rule action configuration.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 * Output:
 *      pAction   - action mask and data configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      For ingress ACL phase, the data is put to pAction->igr_acl.
 *      For egress ACL phase, the data is put to pAction->egr_acl.
 */
extern int32
rtk_acl_ruleAction_get(
    uint32               unit,
    rtk_acl_phase_t      phase,
    rtk_acl_id_t         entry_idx,
    rtk_acl_action_t     *pAction);

/* Function Name:
 *      rtk_acl_ruleAction_set
 * Description:
 *      Set the ACL rule action configuration.
 * Input:
 *      unit      - unit id
 *      phase     - ACL lookup phase
 *      entry_idx - ACL entry index
 *      pAction   - action mask and data configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      For ingress ACL phase, pAction->igr_acl is used for programming.
 *      For egress ACL phase, pAction->egr_acl is used for programming.
 */
extern int32
rtk_acl_ruleAction_set(
    uint32               unit,
    rtk_acl_phase_t      phase,
    rtk_acl_id_t         entry_idx,
    rtk_acl_action_t     *pAction);

/* Function Name:
 *      rtk_acl_ruleHitIndication_get
 * Description:
 *      Get the ACL rule hit indication.
 * Input:
 *      unit        - unit id
 *      phase       - ACL lookup phase
 *      entry_idx   - ACL entry index
 *      reset       - reset the hit status
 * Output:
 *      pIsHit      - pointer to hit status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      The hit status can be cleared by configuring para. 'reset' to 1.
 */
extern int32
rtk_acl_ruleHitIndication_get(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_id_t        entry_idx,
    uint32              reset,
    uint32              *pIsHit);

/* Function Name:
 *      rtk_acl_rule_del
 * Description:
 *      Delete the specified ACL rules.
 * Input:
 *      unit    - unit id
 *      phase   - ACL lookup phase
 *      pClrIdx - rule index to clear
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_PHASE       - invalid ACL phase
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 *      RT_ERR_ACL_CLEAR_INDEX - end index is lower than start index
 *      RT_ERR_ENTRY_INDEX     - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      Entry fields, operations and actions are all cleared.
 */
extern int32
rtk_acl_rule_del(uint32 unit, rtk_acl_phase_t phase, rtk_acl_clear_t *pClrIdx);

/* Function Name:
 *      rtk_acl_rule_move
 * Description:
 *      Move the specified ingress ACL rules.
 * Input:
 *      unit  - unit id
 *      phase - ACL lookup phase
 *      pData - movement info
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ACL_PHASE    - invalid ACL phase
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Entry fields, operations and actions are all moved.
 *      (2) The vacant entries due to movement are auto cleared to be invalid by H/W.
 *      (3) (move_from + length) and (move_to + length) must <= the number of ACL rule
 */
extern int32
rtk_acl_rule_move(uint32 unit, rtk_acl_phase_t phase, rtk_acl_move_t *pData);

/* Function Name:
 *      rtk_acl_templateSelector_get
 * Description:
 *      Get the mapping template of specific block.
 * Input:
 *      unit          - unit id
 *      block_idx     - block index
 * Output:
 *      pTemplate_idx - template index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_templateSelector_get(
    uint32                  unit,
    uint32                  block_idx,
    rtk_acl_templateIdx_t   *pTemplate_idx);

/* Function Name:
 *      rtk_acl_templateSelector_set
 * Description:
 *      Set the mapping template of specific block.
 * Input:
 *      unit         - unit id
 *      block_idx    - block index
 *      template_idx - template index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                   - invalid unit id
 *      RT_ERR_NOT_INIT                  - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX           - invalid block index
 *      RT_ERR_ACL_TEMPLATE_INDEX        - invalid template index
 *      RT_ERR_ACL_TEMPLATE_INCOMPATIBLE - try to map a ACL block to an incompatible template
 *      RT_ERR_INPUT                     - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_templateSelector_set(
    uint32                  unit,
    uint32                  block_idx,
    rtk_acl_templateIdx_t   template_idx);

/* Function Name:
 *      rtk_acl_template_get
 * Description:
 *      Get the content of specific template.
 * Input:
 *      unit        - unit id
 *      template_id - template ID
 * Output:
 *      pTemplate   - template content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_ACL_TEMPLATE_INDEX - invalid template index
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      ID 0-4 are fixed template, ID 5-7 are user defined template.
 */
extern int32
rtk_acl_template_get(uint32 unit, uint32 template_id, rtk_acl_template_t *pTemplate);

/* Function Name:
 *      rtk_acl_template_set
 * Description:
 *      Set the content of specific template.
 * Input:
 *      unit        - unit id
 *      template_id - template ID
 *      pTemplate   - template content
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_NOT_INIT           - The module is not initial
 *      RT_ERR_ACL_TEMPLATE_INDEX - invalid template index
 *      RT_ERR_ACL_FIELD_TYPE     - invalid ACL field type
 *      RT_ERR_ACL_FIELD_LOCATION - invalid field location
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) ID 0-4 are fixed template, ID 5-7 are user defined template.
 *      (2) Configure fixed templates is prohibited.
 */
extern int32
rtk_acl_template_set(uint32 unit, uint32 template_id, rtk_acl_template_t *pTemplate);

/* Function Name:
 *      rtk_acl_templateField_check
 * Description:
 *      Check whether the specified template field type is supported on the chip.
 * Input:
 *      unit  - unit id
 *      phase - ACL lookup phase
 *      type  - template field type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ACL_FIELD_TYPE - invalid ACL field type
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_templateField_check(
    uint32                      unit,
    rtk_acl_phase_t             phase,
    rtk_acl_templateFieldType_t type);

/* Function Name:
 *      rtk_acl_blockResultMode_get
 * Description:
 *      Get the acl block result mode.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 * Output:
 *      pMode     - block result mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      If a packet hit multiple rules and the mode is configured to ACL_BLOCK_RESULT_SINGLE, then
 *      the hit result will be the rule with the lowest index.
 */
extern int32
rtk_acl_blockResultMode_get(uint32 unit, uint32 block_idx, rtk_acl_blockResultMode_t *pMode);

/* Function Name:
 *      rtk_acl_blockResultMode_set
 * Description:
 *      Set the acl block result mode.
 * Input:
 *      unit      - unit id
 *      block_idx - block index
 *      mode      - block result mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390
 * Note:
 *      If a packet hit multiple rules and the mode is configured to ACL_BLOCK_RESULT_SINGLE, then
 *      the hit result will be the rule with the lowest index.
 */
extern int32
rtk_acl_blockResultMode_set(uint32 unit, uint32 block_idx, rtk_acl_blockResultMode_t mode);

/* Function Name:
 *      rtk_acl_blockGroupEnable_get
 * Description:
 *      Set the block grouping.
 * Input:
 *      unit       - unit id
 *      block_idx  - block index
 *      group_type - grouping type
 * Output:
 *      pEnable    - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_INPUT           - invalid input parameter
 *      RT_ERR_NULL_POINTER    - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) If multiple physical blocks are grouped to a logical block,
 *          it only outputs a single hit result and the hit result will be
 *          the entry with lowest index.
 *      (2) Group ingress ACL block with egress ACL block is forbidden.
 *      (3) For ACL_BLOCK_GROUP_2, valid index is 2N where N = 0,1...
 *      (4) For ACL_BLOCK_GROUP_4, valid index is 4N where N = 0,1...
 *      (5) For ACL_BLOCK_GROUP_8, valid index is 8N where N = 0,1...
 *      (6) For ACL_BLOCK_GROUP_ALL, valid index is 0.
 *      (7) If multiple grouping types are applied to the same block index, then
 *          the priority will be ACL_BLOCK_GROUP_ALL > ACL_BLOCK_GROUP_8 >
 *          ACL_BLOCK_GROUP_4 > ACL_BLOCK_GROUP_2.
 *      (8) ACL_BLOCK_GROUP_1 is applicable to 8380 only.
 */
extern int32
rtk_acl_blockGroupEnable_get(
    uint32                     unit,
    uint32                     block_idx,
    rtk_acl_blockGroup_t       group_type,
    rtk_enable_t               *pEnable);

/* Function Name:
 *      rtk_acl_blockGroupEnable_set
 * Description:
 *      Set the block grouping.
 * Input:
 *      unit       - unit id
 *      block_idx  - block index
 *      group_type - grouping type
 *      enable     - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID         - invalid unit id
 *      RT_ERR_NOT_INIT        - The module is not initial
 *      RT_ERR_ACL_BLOCK_INDEX - invalid block index
 *      RT_ERR_INPUT           - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) If multiple physical blocks are grouped to a logical block,
 *          it only outputs a single hit result and the hit result will be
 *          the entry with lowest index.
 *      (2) Group ingress ACL block with egress ACL block is forbidden.
 *      (3) For ACL_BLOCK_GROUP_2, valid index is 2N where N = 0,1...
 *      (4) For ACL_BLOCK_GROUP_4, valid index is 4N where N = 0,1...
 *      (5) For ACL_BLOCK_GROUP_8, valid index is 8N where N = 0,1...
 *      (6) For ACL_BLOCK_GROUP_ALL, valid index is 0.
 *      (7) If multiple grouping types are applied to the same block index, then
 *          the priority will be ACL_BLOCK_GROUP_ALL > ACL_BLOCK_GROUP_8 >
 *          ACL_BLOCK_GROUP_4 > ACL_BLOCK_GROUP_2.
 *      (8) ACL_BLOCK_GROUP_1 is applicable to 8380 only.
 */
extern int32
rtk_acl_blockGroupEnable_set(
    uint32                     unit,
    uint32                     block_idx,
    rtk_acl_blockGroup_t       group_type,
    rtk_enable_t               enable);

/* Function Name:
 *      rtk_acl_statPktCnt_get
 * Description:
 *      Get packet-based statistic counter of the log id.
 * Input:
 *      unit     - unit id
 *      log_id   - log id
 * Output:
 *      pPkt_cnt - pointer buffer of packet count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_statPktCnt_get(uint32 unit, uint32 log_id, uint32 *pPkt_cnt);

/* Function Name:
 *      rtk_acl_statPktCnt_clear
 * Description:
 *      Set packet-based statistic counter of the log id.
 * Input:
 *      unit   - unit id
 *      log_id - log id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_statPktCnt_clear(uint32 unit, uint32 log_id);

/* Function Name:
 *      rtk_acl_statByteCnt_get
 * Description:
 *      Get byte-based statistic counter of the log id.
 * Input:
 *      unit      - unit id
 *      log_id    - log id
 * Output:
 *      pByte_cnt - byte count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_statByteCnt_get(uint32 unit, uint32 log_id, uint64 *pByte_cnt);

/* Function Name:
 *      rtk_acl_statByteCnt_clear
 * Description:
 *      Reset byte-based statistic counter of the log id.
 * Input:
 *      unit   - unit id
 *      log_id - log id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_statByteCnt_clear(uint32 unit, uint32 log_id);

/* Function Name:
 *      rtk_acl_stat_clearAll
 * Description:
 *      Clear all statistic counter for the specified device.
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
 *      8390, 8380
 * Note:
 *      All the packet counters and byte counters are cleared.
 */
extern int32
rtk_acl_stat_clearAll(uint32 unit);

/* Sub-module Name :
 *  Range Check
 */

/* Function Name:
 *      rtk_acl_rangeCheckL4Port_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckL4Port_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_l4Port_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckL4Port_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckL4Port_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_l4Port_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckVid_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      For untag packet, P-VID is used.
 */
extern int32
rtk_acl_rangeCheckVid_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_vid_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckVid_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      For untag packet, P-VID is used.
 */
extern int32
rtk_acl_rangeCheckVid_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_vid_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckIp_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) For IPv6 range check, index 0/4 means IP6[31:0], index 1/5 means IP6[63:32],
 *          index 2/6 means IP6[95:64], index 3/7 means IP6[127:96]. Index 0~3/4~7 must
 *          be used together in order to filter a full IPv6 address.
 *      (2) For IPv6 suffix range check, index 0/2/4/6 means IP6[31:0], index 1/3/5/7 means IP6[63:32],
 *          Index 0&1/2&3/4&5/6&7 must be used together in order to filter a IPv6 suffix address.
 */
extern int32
rtk_acl_rangeCheckIp_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_ip_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckIp_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) For IPv6 range check, index 0/4 means IP6[31:0], index 1/5 means IP6[63:32],
 *          index 2/6 means IP6[95:64], index 3/7 means IP6[127:96]. Index 0~3/4~7 must
 *          be used together in order to filter a full IPv6 address.
 *      (2) For IPv6 suffix range check, index 0/2/4/6 means IP6[31:0], index 1/3/5/7 means IP6[63:32],
 *          Index 0&1/2&3/4&5/6&7 must be used together in order to filter a IPv6 suffix address.
 */
extern int32
rtk_acl_rangeCheckIp_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_ip_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckSrcPort_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckSrcPort_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckSrcPort_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckSrcPort_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckDstPort_get
 * Description:
 *      Get the configuration of destination port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of destination port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckDstPort_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckDstPort_set
 * Description:
 *      Set the configuration of destination port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of destination port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckDstPort_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_portMask_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckPacketLen_get
 * Description:
 *      Get the configuration of packet length range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of packet length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Packet length includes CRC(4Byte)
 */
extern int32
rtk_acl_rangeCheckPacketLen_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_packetLen_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckPacketLen_set
 * Description:
 *      Set the configuration of packet length range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Packet length includes CRC(4Byte)
 */
extern int32
rtk_acl_rangeCheckPacketLen_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_packetLen_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckFieldSelector_get
 * Description:
 *      Get the configuration of field selector range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of field selector
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckFieldSelector_get(uint32 unit, uint32 index, rtk_acl_rangeCheck_fieldSelector_t *pData);

/* Function Name:
 *      rtk_acl_rangeCheckFieldSelector_set
 * Description:
 *      Set the configuration of field selector range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of field selector
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_acl_rangeCheckFieldSelector_set(uint32 unit, uint32 index, rtk_acl_rangeCheck_fieldSelector_t *pData);

/* Sub-module Name :
 *  Field Selector
 */

/* Function Name:
 *      rtk_acl_fieldSelector_get
 * Description:
 *      Get the configuration of field selector.
 * Input:
 *      unit   - unit id
 *      fs_idx - field selector index
 * Output:
 *      pFs    - configuration of field selector.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      For 8390, only the first 180B(since DA) of packet can be inspected by field selector.
 *      Thus, (start position + offest) must less than 164B(since DA) in order to grab a
 *      complete 16-bit user define field.
 */
extern int32
rtk_acl_fieldSelector_get(
    uint32                       unit,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs);

/* Function Name:
 *      rtk_acl_fieldSelector_set
 * Description:
 *      Set the configuration of field selector.
 * Input:
 *      unit   - unit id
 *      fs_idx - field selector index
 *      pFs    - configuration of field selector.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      For 8390, only the first 180B(since DA) of packet can be inspected by field selector.
 *      Thus, (start position + offest) must less than 164B(since DA) in order to grab a
 *      complete 16-bit user define field.
 */
extern int32
rtk_acl_fieldSelector_set(
    uint32                       unit,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs);

/* Function Name:
 *      rtk_acl_portFieldSelector_get
 * Description:
 *      Get the per-port configuration of field selector .
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 * Output:
 *      pFs    - configuration of field selector.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380
 * Note:
 *      For 8380, filed selector can be configured by port.
 */
extern int32
rtk_acl_portFieldSelector_get(
    uint32                       unit,
    rtk_port_t                   port,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs);

/* Function Name:
 *      rtk_acl_portFieldSelector_set
 * Description:
 *      Set the configuration of field selector.
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      For 8380, filed selector can be configured by port.
 */
extern int32
rtk_acl_portFieldSelector_set(
    uint32                       unit,
    rtk_port_t                   port,
    uint32                       fs_idx,
    rtk_acl_fieldSelector_data_t *pFs);

/* Function Name:
 *      rtk_acl_meterMode_get
 * Description:
 *      Get the meter mode of a specific meter block.
 * Input:
 *      unit       - unit id
 *      idx        - meter block ID for 8390, meter entry ID for 8380
 * Output:
 *      pMeterMode - meter mode:byte based or packet based
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
 *      None
 */
extern int32
rtk_acl_meterMode_get(
    uint32              unit,
    uint32              idx,
    rtk_acl_meterMode_t *pMeterMode);

/* Function Name:
 *      rtk_acl_meterMode_set
 * Description:
 *      Set the meter mode.
 * Input:
 *      unit      - unit id
 *      idx       - meter block ID for 8390, meter entry ID for 8380
 *      meterMode - meter mode (byte based or packet based)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_ENTRY_INDEX - invalid entry index
 *      RT_ERR_INPUT       - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_meterMode_set(
    uint32              unit,
    uint32              idx,
    rtk_acl_meterMode_t meterMode);

/* Function Name:
 *      rtk_acl_meterIncludeIfg_get
 * Description:
 *      Get IFG including status for packet length calculation of meter module.
 * Input:
 *      unit         - unit id
 * Output:
 *      pIfg_include - pointer to enable status of includes IFG
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
rtk_acl_meterIncludeIfg_get(uint32 unit, rtk_enable_t *pIfg_include);

/* Function Name:
 *      rtk_acl_meterIncludeIfg_set
 * Description:
 *      Set IFG including status for packet length calculation of meter module.
 * Input:
 *      unit        - unit id
 *      ifg_include - enable status of includes IFG
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
rtk_acl_meterIncludeIfg_set(uint32 unit, rtk_enable_t ifg_include);

/* Function Name:
 *      rtk_acl_meterBurstSize_get
 * Description:
 *      Get the meter burst sizes of a specific meter mode.
 * Input:
 *      unit       - unit id
 *      meterMode  - meter mode
 * Output:
 *      pBurstSize - pointer to burst sizes
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
 *      (1) Double leaky packet/ single rate three color/ two rate three color type
 *          have separated burst size settings.
 *      (2) In meterMode = METER_MODE_BYTE of 8390, the (minimum, maximum) of pBurstSize->dlb_lb0bs and 
 *          pBurstSize->dlb_lb1bs setting range is (17, 65535). 
 */
extern int32
rtk_acl_meterBurstSize_get(
    uint32                      unit,
    rtk_acl_meterMode_t         meterMode,
    rtk_acl_meterBurstSize_t    *pBurstSize);

/* Function Name:
 *      rtk_acl_meterBurstSize_set
 * Description:
 *      Set the meter burst sizes of a specific meter mode.
 * Input:
 *      unit       - unit id
 *      meterMode  - meter mode (byte based or packet based)
 *      pBurstSize - pointer to burst sizes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      double leaky packet/ single rate three color/ two rate three color type
 *      have separated burst size settings.
 */
extern int32
rtk_acl_meterBurstSize_set(
    uint32                      unit,
    rtk_acl_meterMode_t         meterMode,
    rtk_acl_meterBurstSize_t    *pBurstSize);

/* Function Name:
 *      rtk_acl_meterExceed_get
 * Description:
 *      Get the meter exceed flag of a meter entry.
 * Input:
 *      unit      - unit id
 *      meterIdx  - meter entry index
 * Output:
 *      pIsExceed - pointer to exceed flag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_meterExceed_get(
    uint32  unit,
    uint32  meterIdx,
    uint32  *pIsExceed);

/* Function Name:
 *      rtk_acl_meterExceedAggregation_get
 * Description:
 *      Get the meter exceed flag mask of meter entry exceed aggregated result every 16 entries.
 * Input:
 *      unit      - unit id
 * Output:
 *      pExceedMask - pointer to aggregated exceed flag mask
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
rtk_acl_meterExceedAggregation_get(
    uint32  unit,
    uint32  *pExceedMask);

/* Function Name:
 *      rtk_acl_meterEntry_get
 * Description:
 *      Get the content of a meter entry.
 * Input:
 *      unit        - unit id
 *      meterIdx    - meter entry index
 * Output:
 *      pMeterEntry - pointer to a meter entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_meterEntry_get(
    uint32                  unit,
    uint32                  meterIdx,
    rtk_acl_meterEntry_t    *pMeterEntry);

/* Function Name:
 *      rtk_acl_meterEntry_set
 * Description:
 *      Set a meter entry.
 * Input:
 *      unit        - unit id
 *      meterIdx    - meter entry index
 *      pMeterEntry - pointer to meter entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_meterEntry_set(
    uint32                  unit,
    uint32                  meterIdx,
    rtk_acl_meterEntry_t    *pMeterEntry);

/* Function Name:
 *      rtk_acl_ruleEntryField_check
 * Description:
 *      Check whether the specified field type is supported on the chip.
 * Input:
 *      unit  - unit id
 *      phase - ACL lookup phase
 *      type  - field type to be checked
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID        - invalid unit id
 *      RT_ERR_NOT_INIT       - The module is not initial
 *      RT_ERR_ACL_PHASE      - invalid ACL phase
 *      RT_ERR_ACL_FIELD_TYPE - input parameter may be null pointer
 * Applicable:
 *       8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_acl_ruleEntryField_check(
    uint32              unit,
    rtk_acl_phase_t     phase,
    rtk_acl_fieldType_t type);

/* Function Name:
 *      rtk_acl_templateFieldIntentVlanTag_get
 * Description:
 *      Get the acl template field VLAN tag status
 * Input:
 *      unit     - unit id
 * Output:
 *      tagType  - template field VLAN tag status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_acl_templateFieldIntentVlanTag_get(uint32 unit,
    rtk_vlan_tagType_t *tagType);

/* Function Name:
 *      rtk_acl_templateFieldIntentVlanTag_set
 * Description:
 *      Set the acl template field VLAN tag status
 * Input:
 *      unit     - unit id
 *      tagType  - template field VLAN tag status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390
 * Note:
 *      None
 */
extern int32
rtk_acl_templateFieldIntentVlanTag_set(uint32 unit,
    rtk_vlan_tagType_t tagType);

#endif /* __RTK_ACL_H__ */
