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
 * $Revision: 35667 $
 * $Date: 2012-12-26 21:35:45 +0800 (Wed, 26 Dec 2012) $
 *
 * Purpose : Definition those public NIC(Network Interface Controller) APIs and 
 *           its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) CPU tag
 *           2) NIC tx
 *           3) NIC rx 
 *
 */

#ifndef __DRV_NIC_COMMON_H__
#define __DRV_NIC_COMMON_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/type.h>

/* 
 * Symbol Definition     
 */ 
#define NIC_RX_CB_PRIORITY_NUMBER       (8)
#define NIC_RX_CB_PRIORITY_MIN          (0)
#define NIC_RX_CB_PRIORITY_MAX          (NIC_RX_CB_PRIORITY_NUMBER - 1)

typedef enum drv_nic_rx_reason_e
{
    NIC_RX_REASON_DUMMY = 0,
    NIC_RX_REASON_OAM,
    NIC_RX_REASON_CFM,
    NIC_RX_REASON_CFM_ETHDM,
    NIC_RX_REASON_IGR_VLAN_FILTER,		/* VLAN ingress filter */
    NIC_RX_REASON_VLAN_ERROR,			/* VLAN error(VID=4095 or MBR=0x0) */
    NIC_RX_REASON_INNER_OUTTER_CFI,		/* Inner/Outer tag CFI/DEI = 1 */
    NIC_RX_REASON_RMA_USR_DEF1,			/* RMA User-defined 1 */
    NIC_RX_REASON_RMA_USR_DEF2,			/* RMA User-defined 2 */
    NIC_RX_REASON_RMA_BPDU,				/* RMA BPDU 01-80-C2-00-00-00 */
    NIC_RX_REASON_RMA_LACP,				/* RMA LACP 01-80-C2-00-00-02 */
    NIC_RX_REASON_RMA_PTP,				/* RMA PTP packet, 0x88F7 */
    NIC_RX_REASON_RMA_LLDP,				/* RMA LLDP packet, 0x88CC + 01-80-C2-00-00-0E */
    NIC_RX_REASON_RMA,					/* RMA */
    NIC_RX_REASON_RLDP_RLPP,			/* RLDP or RLPP packet */
    NIC_RX_REASON_IP4_HDR_ERROR,		/* IP4 header error */
    NIC_RX_REASON_IP4_OPTIONS,			/* IPv4 options */
    NIC_RX_REASON_IP6_HDR_ERROR,		/* IPv6 header error */
    NIC_RX_REASON_IP6_HOPBYHOP_EXT_HDR_ERROR,	/* IPv6 Hop-by-Hop Extension Header Position Error */
    NIC_RX_REASON_IP6_HOPBYHOP_OPTION,	/* IPv6 Hop-by-Hop option */
    NIC_RX_REASON_IP6_UNKWN_EXT_HDR,	/* IPv6 Unknown Extension Header */
    NIC_RX_REASON_TTL_EXCEED,			/* TTL exceed */
    NIC_RX_REASON_HOP_EXCEED,			/* Hop Limit exceed */
    NIC_RX_REASON_ROUTING_EXCEPTION,	/* Routing exception (IPv4 header error/IPv4 TTL exceed/IPv4 options/
    									IPv6 Hop Limit exceed/IPv6 Hop-by-Hop option) or Next-hop entry age out */    
    NIC_RX_REASON_GW_MAC_ERROR,			/* GW MAC Error/NextHop Age Out */
    NIC_RX_REASON_IGMP,					/* IGMP */
    NIC_RX_REASON_MLD,					/* MLD */
    NIC_RX_REASON_EAPOL,				/* EAPOL */
    NIC_RX_REASON_SPECIAL_TRAP,			/* Special trap (IGMP/MLD/EAPOL) */
    NIC_RX_REASON_ARP_REQ,				/* ARP Request */
    NIC_RX_REASON_IP6_NEIGHBOR_DISCOVER,	/* IPv6 header error */
    NIC_RX_REASON_SPECIAL_COPY,			/* Special copy (ARP/IPV6 Neighbor Discovery) */
    NIC_RX_REASON_UNKWN_UCST,			/* Unknown unicast trap */
    NIC_RX_REASON_UNKWN_L2_MCST,		/* Unknown L2 multicast trap */
    NIC_RX_REASON_UNKWN_IP_MCST,		/* Unknown IP multicast trap */
    NIC_RX_REASON_UNKWN_IP6_MCST,		/* Unknown IPv6 multicast trap */
    NIC_RX_REASON_UNKWN_BCST,			/* Unknown broadcast trap */
    NIC_RX_REASON_UNKWN_UCST_MCST,		/* Unknown unicast/multicast trap */
    NIC_RX_REASON_MY_MAC,				/* CPU MAC */
    NIC_RX_REASON_INVALID_SA,			/* Invalid SA */
    NIC_RX_REASON_MAC_CONSTRAINT_SYS,	/* MAC Constraint (learn limit) system-based */
    NIC_RX_REASON_MAC_CONSTRAINT_VLAN,	/* MAC Constraint (learn limit) vlan-based */
    NIC_RX_REASON_MAC_CONSTRAINT_PORT,	/* MAC Constraint (learn limit) port-based */
    NIC_RX_REASON_MAC_CONSTRAINT,		/* MAC Constraint (learn limit) */
    NIC_RX_REASON_NEW_SA,				/* New Source MAC Address */
    NIC_RX_REASON_STC_L2_PMV,			/* Static L2 entry port moving */
    NIC_RX_REASON_L2_PMV,				/* L2 entry port moving */
    NIC_RX_REASON_ACL_HIT,				/* ACL hit */
    NIC_RX_REASON_ATTACK,				/* Attack prevention has detected this attack */
    NIC_RX_REASON_MIRROR,				/* Mirror */
    NIC_RX_REASON_CRC_ERROR,			/* CRC error */
    NIC_RX_REASON_RRCP,					/* RRCP */
    
    NIC_RX_REASON_NORMAL_FWD,			/* Normal forwarding (flooding or L2 entry hit) */
    NIC_RX_REASON_MAX,
} drv_nic_rx_reason_t;

#define DRV_NIC_TOTAL_NUM_OF_WORD_FOR_1BIT_REASON_MASK    ((NIC_RX_REASON_MAX+31)/32)

typedef struct drv_nic_pkt_s
{
    uint8  *head;       /* pointer to the head of the packet data buffer */
    uint8  *data;       /* pointer to the base address of the packet */
    uint8  *tail;       /* pointer to the end address of the packet */
    uint8  *end;        /* pointer to the end of the packet data buffer */
    uint32 length;      /* packet length when the packet is valid (not a empty data buffer) */
    void   *buf_id;     /* pointer to the user-defined packet descriptor */

    uint8  as_txtag;    /* 0: without tx-tag, 1: with tx-tag */
#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
	uint8  txIncludeCRC;	/* Whether the packet transmited from higher layer includes CRC (packets from both Linux kernel and Realtek turnkey protocol are not including CRC) */
#endif

	uint32	reason_bitmask[DRV_NIC_TOTAL_NUM_OF_WORD_FOR_1BIT_REASON_MASK];	/* The trap reasons in this member are common to all chip */

    union
    {
        /* Notice:
           SOC layer use the field's name to do the filed copy, DO NOT change the field name
           or reverse the order of fields.
           The fields declared in struct rx/tx must be byte alignment.
        */

        /* Reception information */
        struct
        {
            uint8   source_port;        /* Source port number */
            uint16  reason;             /* Reason to CPU */
            uint8   svid_tagged:1;      /* Whether the SVID is tagged */
            uint8   cvid_tagged:1;      /* Whether the CVID is tagged (ethertype==0x8100) */
            uint16  outer_pri:3;        /* priority of outer tag */
            uint16  outer_vid:12;       /* vid of outer tag */
            uint16  inner_pri:3;        /* priority of inner tag */
            uint16  inner_vid:12;       /* vid of inner tag */

#if defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
            uint16  acl_hit:1;         	/* 0: not hit, 1: hit */
    		uint16  acl_index;          /* ACL index */
#endif

#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
            uint8   priority;           /* Assigned priority */
            uint8   l2_error:1;         /* L2 CRC error. */
            uint8   l3_error:1;         /* IPv4 checksum error */
            uint8   l4_error:1;         /* TCP,UDP,ICMP,IGMP checksum error */
            uint8   pppoe:1;            /* Whether PPPoE header exists */
//            uint8   svid_tagged:1;      /* Whether the SVID is tagged */
//            uint8   cvid_tagged:1;      /* Whether the CVID is tagged (ethertype==0x8100) */
            uint8   drop_precedence:2;  /* Assigned drop precedence */
            uint8   l2_format:4;        /* Layer 2 format */
            uint8   l3_l4_format:4;     /* Layer 3/4 format */
//            uint16  outer_pri:3;        /* priority of outer tag */
//            uint16  outer_vid:12;       /* vid of outer tag */
//            uint16  inner_pri:3;        /* priority of inner tag */
//            uint16  inner_vid:12;       /* vid of inner tag */
#endif

#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
            uint8   qid;				/* Mapping to Tx Queue ID */
            uint16  mirror_hit:4;		/* Mirrioring hit status for 4-sets of configuration */
//            uint16  svid_tagged:1;      /* Whether the SVID is tagged */
//            uint16  cvid_tagged:1;      /* Whether the CVID is tagged (ethertype==0x8100) */
            uint16  fvid:12;			/* Forwarding VLAN ID */
            uint8   oversize:1;			/* packet has truncated by NIC */
            uint8   mac_cst:1;			/* MAC Constraint */
            uint8   atk_type:5;			/* Indicate which attack type this packet belong to */
            uint16  new_sa:1;			/* New Source MAC Address */
//            uint16  inner_pri:3;        /* priority of inner tag */
//            uint16  inner_vid:12;       /* vid of inner tag */
            uint16  l2_pmv:1;		    /* L2 entry port moving (both static and dynamic entry) */
//            uint16  outer_pri:3;        /* priority of outer tag */
//            uint16  outer_vid:12;       /* vid of outer tag */
#endif

#if defined(CONFIG_SDK_RTL8328)
            uint16  extra_tag:1;        /* Extra tag exist */
            uint16  sw_valid:1;         /* 0: invalid, 1: valid */
            uint16  sw_index_type:3;    /* valid when sw_valid = 1, index type */
#define SW_TYPE_ACL     0
#define SW_TYPE_RMA     1
            uint16  sw_index:12;        /* valid when sw_valid = 1, mean acl or rma index */
#endif

#if defined(CONFIG_SDK_RTL8380)
            uint8   atk_hit:1;			/* Indicate whether attack happened */
#endif

#if defined(CONFIG_SDK_RTL8390)
            uint8   crc:1;         		/* L2/L3 CRC Error */
            uint8   sflow:6;			/* S-Flow hit port number */
            uint8   dm_rxIdx:6;			/* ETH-DM RxTimestamp Index (0~63) */
#endif
        } rx_tag;
		
        /* Transmission information */
        struct
        {
            uint32  dst_port_mask;   	/* DPM: When ADSPM is 1, DPM is the packet destination port mask. port 0~31 */
            uint8   as_dst_port_mask:1; /* ASDPM: Assign the destination port directly. */
            uint8   as_priority:1;      /* Assign priority and drop precedence directly. */
            uint8   priority:3;         /* Assigned priority */
            uint8   l2_learning:1;      /* Decide whether proceed L2 learning */
            
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
            uint8   as_port_remark:1;   /* ASPRMK: When ASPRMK is 1, the packet will be remarked 
                                           based on port remarking setting. */
            uint8   l2_recalculate:1;   /* Recalculate the layer2 CRC */
            uint8   l3_recalculate:1;   /* Recalculate the IPv4 checksum */
            uint8   l4_recalculate:1;   /* Recalculate the TCP,UDP,ICMP,IGMP checksum ,
                                           Note: The checksum in TCP/UDP/ICMP/IGMP must be filled
                                           with 0x0000 if l4_recalculate is enabled*/
            uint8   drop_precedence:2;  /* Assigned drop precedence (RTL8389 unsupported) */
#endif

#if defined(CONFIG_SDK_RTL8328)
            uint32  flags;              /* Module enable flag */
#endif

#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
            uint8   dpm_type:1;			/* DPM(Destination Port Mask) Type, 0b0: Logical  0b1: Physical */
            uint8   acl_act:1;			/* If ACL_ACT is 0b1, then take effect of ingress/egress ACL */
            uint8   bp_fltr1:1;			/* If BP_FLTR_1 is 0b1, then bypass the following filtering: (1)Egress OAM mux (2)Egress Port Isolation (3)Egress Mirror Isolation (4)WRED */
            uint8   bp_fltr2:1;			/* If BP_FLTR_2 is 0b1, then bypass the following checking: (1) Egress spanning-tree port state in blocking/listening or learning (2)Egress VLAN filtering */
            uint8   as_tagSts:1;		/* Decide whether proceed Outter/Inner TAG by ALE */
            uint8   rvid_sel:1;			/* Relay VID selection. 0b0: follow inner tag   0b1: follow outer tag */
#endif

#if defined(CONFIG_SDK_RTL8390)
            uint32  dst_port_mask_1; 	/* packet destination port, port 32~63 */
			uint8   cngst_drop:1;		/* Congestion Dropable */
            uint8   dm_pkt:1;			/* ETH-DM packet indicate */
            uint8   dg_pkt:1;			/* Dying Gasp packet */
			uint8   ptp_tx_log:1;		/* PTP log function */
			uint8   ptp_tx_offload:2;	/* PTP offload mechanism for one-step */
#endif
        } tx_tag;
    };

    struct drv_nic_pkt_s *next;         /* pointer to next packet struct if it exist */
} drv_nic_pkt_t;

typedef enum drv_nic_rx_e
{
    NIC_RX_NOT_HANDLED = 0,     /* The packet is not handled, continue processing */
    NIC_RX_HANDLED,             /* The packet is and caller has taken control of the packet data */
    NIC_RX_HANDLED_OWNED,       /* The packet is handled and processing is complete */
} drv_nic_rx_t;

typedef int32 (*drv_nic_pkt_alloc_f)(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket);
typedef int32 (*drv_nic_pkt_free_f)(uint32 unit, drv_nic_pkt_t *pPacket);
typedef drv_nic_rx_t (*drv_nic_rx_cb_f)(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie);
typedef void (*drv_nic_tx_cb_f)(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie);

typedef struct drv_nic_initCfg_s
{
    int32               pkt_size;       /* Maximum bytes to support in a packet */
    drv_nic_pkt_alloc_f pkt_alloc;      /* Packet structure and packet data allocation routine */
    drv_nic_pkt_free_f  pkt_free;       /* Packet structure and packet data deallocation routine */
} drv_nic_initCfg_t;

#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
typedef struct drv_nic_CpuEntry_s
{
    /* Data, 16 bytes */
    uint32  dpm;
    uint16  vid;
    uint16  ethertype;
    uint16  protocol_id;
    uint16  l4_dst_port;
    uint8   dmac[6];
    /* Care-bit, 16 bytes  */
    uint32  care_dpm;
    uint16  care_vid;
    uint16  care_ethertype;
    uint16  care_protocol_id;
    uint16  care_l4_dst_port;
    uint8   care_dmac[6];
} drv_nic_CpuEntry_t;

typedef enum drv_nic_sw_ram_index_e
{
    NIC_SW_RMA_L2_00 = 0,
    NIC_SW_RMA_L2_01,
    NIC_SW_RMA_L2_USER_DEFINED_0 = 48,
    NIC_SW_RMA_L2_USER_DEFINED_1 = 49,
    NIC_SW_RMA_L2_USER_DEFINED_2 = 50,
    NIC_SW_RMA_L2_USER_DEFINED_3 = 51,
    NIC_SW_RMA_L34_RIP = 52,
    NIC_SW_RMA_L34_ICMP,
    NIC_SW_RMA_L34_ICMPV6,
    NIC_SW_RMA_L34_ARP,
    NIC_SW_RMA_L34_MLD,
    NIC_SW_RMA_L34_IGMP,
    NIC_SW_RMA_L34_BGP,
    NIC_SW_RMA_L34_OSPFV2,
    NIC_SW_RMA_L34_OSPFV3,
    NIC_SW_RMA_L34_SNMP,
    NIC_SW_RMA_L34_SSH,
    NIC_SW_RMA_L34_FTP,
    NIC_SW_RMA_L34_TFTP,
    NIC_SW_RMA_L34_TELNET,
    NIC_SW_RMA_L34_HTTP,
    NIC_SW_RMA_L34_HTTPS,
    NIC_SW_RMA_L34_USER_DEFINED_0,
    NIC_SW_RMA_L34_USER_DEFINED_1,
    NIC_SW_RMA_INDEX_END,
} drv_nic_sw_ram_index_t;
#endif

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */

#define NIC_REASON_MASK_SET(packet, reason) \
do{ \
    (packet).reason_bitmask[(reason)/32] |= (1 << ((reason)%32)); \
}while(0)
    
#define IS_NIC_REASON_MASK_SET(packet, reason) \
    (((reason) < NIC_RX_REASON_MAX) ? ((packet).reason_bitmask[(reason)/32] & (1 << ((reason)%32))) : 0)
 
#endif /* __DRV_NIC_COMMON_H__ */ 

