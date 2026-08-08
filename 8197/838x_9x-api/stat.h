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
 * $Revision: 40458 $
 * $Date: 2013-06-25 09:47:32 +0800 (Tue, 25 Jun 2013) $
 *
 * Purpose : Definition of Statistic API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Statistic Counter Reset
 *           (2) Statistic Counter Get
 *
 */

#ifndef __RTK_STAT_H__
#define __RTK_STAT_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */
/* global statistic counter index */
typedef enum rtk_stat_global_type_e
{
    DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX = 0,
    DOT1D_TP_IN_DISCARDS_INDEX,                     /* Proprietary counter */
    OUT_UCAST_PKTS_INDEX,
    OUT_MCAST_PKTS_INDEX,
    OUT_BCAST_PKTS_INDEX,
    EGR_LACK_RESOURCE_DROP_INDEX,
    MIB_GLOBAL_CNTR_END
} rtk_stat_global_type_t;

/* port statistic counter index */
typedef enum rtk_stat_port_type_e
{
    IF_IN_OCTETS_INDEX = 0,                         /* RFC 2863 ifEntry */
    IF_IN_UCAST_PKTS_INDEX,                         /* RFC 2863 ifEntry */
    IF_IN_MULTICAST_PKTS_INDEX,                     /* RFC 2863 ifEntry */
    IF_IN_BROADCAST_PKTS_INDEX,                     /* RFC 2863 ifEntry */
    IF_IN_DISCARDS_INDEX,                           /* RFC 2863 ifEntry */
    IF_OUT_OCTETS_INDEX,                            /* RFC 2863 ifEntry */
    IF_OUT_DISCARDS_INDEX,                          /* RFC 2863 ifEntry */
    IF_OUT_UCAST_PKTS_CNT_INDEX,                    /* RFC 2863 IfXEntry */
    IF_OUT_MULTICAST_PKTS_CNT_INDEX,                /* RFC 2863 IfXEntry */
    IF_OUT_BROADCAST_PKTS_CNT_INDEX,                /* RFC 2863 IfXEntry */
    IP_IN_RECEIVES_INDEX,                           /* RFC 1213 IP Group */
    IP_IN_DISCARDS_INDEX,                           /* RFC 1213 IP Group */
    IF_HC_IN_OCTETS_INDEX,                          /* RFC 2863 IfXEntry */
    IF_HC_IN_UCAST_PKTS_INDEX,                      /* RFC 2863 IfXEntry */
    IF_HC_IN_MULTICAST_PKTS_INDEX,                  /* RFC 2863 IfXEntry */
    IF_HC_IN_BROADCAST_PKTS_INDEX,                  /* RFC 2863 IfXEntry */
    IF_HC_OUT_OCTETS_INDEX,                         /* RFC 2863 IfXEntry */
    IF_HC_OUT_UCAST_PKTS_INDEX,                     /* RFC 2863 IfXEntry */
    IF_HC_OUT_MULTICAST_PKTS_INDEX,                 /* RFC 2863 IfXEntry */
    IF_HC_OUT_BROADCAST_PKTS_INDEX,                 /* RFC 2863 IfXEntry */
    DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS_INDEX,  /* RFC 1493 Dot1dBasePortEntry */
    DOT1D_TP_PORT_IN_DISCARDS_INDEX,                /* RFC 1493 */
    DOT1D_TP_HC_PORT_IN_DISCARDS_INDEX,             /* RFC 2674 Dot1dTpHCPortEntry */
    DOT3_IN_PAUSE_FRAMES_INDEX,                     /* RFC 2665 Dot3PauseEntry */
    DOT3_OUT_PAUSE_FRAMES_INDEX,                    /* RFC 2665 Dot3PauseEntry */
    DOT3_OUT_PAUSE_ON_FRAMES_INDEX,                 /* Proprietary counter */
    DOT3_STATS_ALIGNMENT_ERRORS_INDEX,              /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_FCS_ERRORS_INDEX,                    /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX,       /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX,     /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX,        /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_LATE_COLLISIONS_INDEX,               /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX,          /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_FRAME_TOO_LONGS_INDEX,               /* RFC 2665 Dot3StatsEntry */
    DOT3_STATS_SYMBOL_ERRORS_INDEX,                 /* RFC 2665 Dot3StatsEntry */
    DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX,          /* RFC 2665 Dot3ControlEntry */
    ETHER_STATS_DROP_EVENTS_INDEX,                  /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_OCTETS_INDEX,                       /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_BROADCAST_PKTS_INDEX,               /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_MULTICAST_PKTS_INDEX,               /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_UNDER_SIZE_PKTS_INDEX,              /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_OVERSIZE_PKTS_INDEX,                /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_FRAGMENTS_INDEX,                    /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_JABBERS_INDEX,                      /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_COLLISIONS_INDEX,                   /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_CRC_ALIGN_ERRORS_INDEX,             /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_PKTS_64OCTETS_INDEX,                /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_PKTS_65TO127OCTETS_INDEX,           /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_PKTS_128TO255OCTETS_INDEX,          /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_PKTS_256TO511OCTETS_INDEX,          /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_PKTS_512TO1023OCTETS_INDEX,         /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_PKTS_1024TO1518OCTETS_INDEX,        /* RFC 2819 EtherStatsEntry */
    ETHER_STATS_TX_OCTETS_INDEX,                    /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_UNDER_SIZE_PKTS_INDEX,           /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_OVERSIZE_PKTS_INDEX,             /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_PKTS_64OCTETS_INDEX,             /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_PKTS_65TO127OCTETS_INDEX,        /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_PKTS_128TO255OCTETS_INDEX,       /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_PKTS_256TO511OCTETS_INDEX,       /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_PKTS_512TO1023OCTETS_INDEX,      /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_PKTS_1024TO1518OCTETS_INDEX,     /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_PKTS_1519TOMAXOCTETS_INDEX,      /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_BROADCAST_PKTS_INDEX,            /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_MULTICAST_PKTS_INDEX,            /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_FRAGMENTS_INDEX,                 /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_JABBERS_INDEX,                   /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_TX_CRC_ALIGN_ERROR_INDEX,           /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    ETHER_STATS_RX_UNDER_SIZE_PKTS_INDEX,           /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_UNDER_SIZE_DROP_PKTS_INDEX,      /* Proprietary counter. */
    ETHER_STATS_RX_OVERSIZE_PKTS_INDEX,             /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_PKTS_64OCTETS_INDEX,             /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_PKTS_65TO127OCTETS_INDEX,        /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_PKTS_128TO255OCTETS_INDEX,       /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_PKTS_256TO511OCTETS_INDEX,       /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_PKTS_512TO1023OCTETS_INDEX,      /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_PKTS_1024TO1518OCTETS_INDEX,     /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    ETHER_STATS_RX_PKTS_1519TOMAXOCTETS_INDEX,      /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    RX_LENGTH_FIELD_ERROR_INDEX,                    /* Proprietary counter */
    RX_FALSE_CARRIER_TIMES_INDEX,                   /* Proprietary counter */
    RX_UNDER_SIZE_OCTETS_INDEX,                     /* Proprietary counter */
    TX_ETHER_STATS_FRAGMENTS_INDEX,                 /* Proprietary counter */
    TX_ETHER_STATS_JABBERS_INDEX,                   /* Proprietary counter */
    TX_ETHER_STATS_CRC_ALIGN_ERRORS_INDEX,          /* Proprietary counter */
    RX_FRAMING_ERRORS_INDEX,                        /* Proprietary counter */
    IGR_LACK_PKT_BUF_DROP_INDEX,                    /* Proprietary counter */
    FLOWCTRL_ON_DROP_PKT_CNT_INDEX,                 /* Proprietary counter */
    TX_CRC_CHECK_FAIL_CNT_INDEX,                    /* Proprietary counter */
    SMART_TRIGGER_HIT0_INDEX,                       /* Proprietary counter */
    SMART_TRIGGER_HIT1_INDEX,                       /* Proprietary counter */
    RX_MAC_DISCARDS_INDEX,                          /* Proprietary counter */
    MIB_PORT_CNTR_END
} rtk_stat_port_type_t;

typedef enum rtk_stat_smon_type_e
{
    SMON_PRIO_STATS_PKTS = 0,
    SMON_PRIO_STATS_OCTETS,
    SMON_PRIO_STATS_END
} rtk_stat_smon_type_t;

/* tag counting index */
typedef enum rtk_stat_tagCnt_type_e
{
    TAG_CNT_TYPE_RX = 0,
    TAG_CNT_TYPE_TX,
    TAG_CNT_TYPE_END
} rtk_stat_tagCnt_type_t;

/* global statistic counter structure */
typedef struct rtk_stat_global_cntr_s
{
    uint32 dot1dTpLearnedEntryDiscards;
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    uint32 dot1dTpPortInDiscards;
    uint32 OutUnicastPktsCnt;
    uint32 OutMulticastPktsCnt;
    uint32 OutBrocastPktsCnt;
    uint32 egrLackResourceDrop;
#endif
} rtk_stat_global_cntr_t;

/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
    uint64 ifInOctets;                              /* RFC 2863 ifEntry */
    uint32 ifInUcastPkts;                           /* RFC 2863 ifEntry */
    uint64 ifOutOctets;                             /* RFC 2863 ifEntry */
    uint32 ifOutUcastPkts;                          /* RFC 2863 IfXEntry */
    uint32 ifOutMulticastPkts;                      /* RFC 2863 IfXEntry */
    uint32 ifOutBrocastPkts;                        /* RFC 2863 IfXEntry */
    uint32 dot3InPauseFrames;                       /* RFC 2665 Dot3PauseEntry */
    uint32 dot3OutPauseFrames;                      /* RFC 2665 Dot3PauseEntry */
    uint32 dot3StatsSingleCollisionFrames;          /* RFC 2665 Dot3PauseEntry */
    uint32 dot3StatsMultipleCollisionFrames;        /* RFC 2665 Dot3PauseEntry */
    uint32 dot3StatsDeferredTransmissions;          /* RFC 2665 Dot3PauseEntry */
    uint32 dot3StatsLateCollisions;                 /* RFC 2665 Dot3PauseEntry */
    uint32 dot3StatsExcessiveCollisions;            /* RFC 2665 Dot3PauseEntry */
    uint32 dot3StatsSymbolErrors;                   /* RFC 2665 Dot3PauseEntry */
    uint32 dot3ControlInUnknownOpcodes;             /* RFC 2665 Dot3PauseEntry */
    uint32 etherStatsBroadcastPkts;                 /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsMulticastPkts;                 /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsUndersizePkts;                 /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsOversizePkts;                  /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsFragments;                     /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsJabbers;                       /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsCollisions;                    /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsPkts64Octets;                  /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsPkts65to127Octets;             /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsPkts128to255Octets;            /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsPkts256to511Octets;            /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsPkts512to1023Octets;           /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsPkts1024to1518Octets;          /* RFC 2819 EtherStatsEntry */
#if defined(CONFIG_SDK_RTL8389)
    uint32 igrLackPktBufDrop;                       /* Proprietary counter */
    uint32 flowCtrlOnDropPktCnt;                    /* Proprietary counter */
    uint32 txCrcCheckFailCnt;                       /* Proprietary counter */
    uint32 smartTriggerHit0;                        /* Proprietary counter */
    uint32 smartTriggerHit1;                        /* Proprietary counter */
#endif
#if defined(CONFIG_SDK_RTL8328)
    uint32 ifInDiscards;                            /* RFC 2863 ifEntry */
    uint32 ipInReceives;                            /* RFC 1213 IP Group */
    uint32 ipInDiscards;                            /* RFC 1213 IP Group */
    uint64 ifHCInUcastPkts;                         /* RFC 2863 IfXEntry */
    uint64 ifHCInMulticastPkts;                     /* RFC 2863 IfXEntry */
    uint64 ifHCInBroadcastPkts;                     /* RFC 2863 IfXEntry */
    uint64 ifHCOutUcastPkts;                        /* RFC 2863 IfXEntry */
    uint64 ifHCOutMulticastPkts;                    /* RFC 2863 IfXEntry */
    uint64 ifHCOutBroadcastPkts;                    /* RFC 2863 IfXEntry */
    uint32 dot1dBasePortDelayExceededDiscards;      /* RFC 1493 Dot1dBasePortEntry */
    uint64 dot1dTPHCPortInDiscards;                 /* RFC 2674 Dot1dTpHCPortEntry */
    uint32 dot3StatsAlignmentErrors;                /* RFC 2665 Dot3PauseEntry */
    uint32 dot3StatsFrameTooLongs;                  /* RFC 2665 Dot3PauseEntry */
    uint64 etherStatsTxOctets;                      /* Proprietary counter */
    uint32 etherStatsTxPkts1024toMaxOctets;         /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
#endif
#if defined(CONFIG_SDK_RTL8390)    
    uint32 rxLengthFieldError;                      /* Proprietary counter */
    uint32 rxFalseCarrierTimes;                     /* Proprietary counter */
    uint32 rxUnderSizeOctets;                       /* Proprietary counter */
    uint32 txEtherStatsFragments;                   /* Proprietary counter */
    uint32 txEtherStatsJabbers;                     /* Proprietary counter */
    uint32 txEtherStatsCRCAlignError;               /* Proprietary counter */
    uint32 rxFramingErrors;                         /* Proprietary counter */
#endif
#if defined(CONFIG_SDK_RTL8389)||defined(CONFIG_SDK_RTL8328)
    uint32 dot3OutPauseOnFrames;                    /* Proprietary counter */
    uint32 dot3StatsFCSErrors;                      /* RFC 2665 Dot3PauseEntry */
    uint64 etherStatsOctets;                        /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsPkts1024toMaxOctets;           /* Proprietary counter. RFC 2819 EtherStatsEntry */
#endif
#if defined(CONFIG_SDK_RTL8328)||defined(CONFIG_SDK_RTL8390)||defined(CONFIG_SDK_RTL8380)
    uint32 ifOutDiscards;                           /* RFC 2863 ifEntry */
    uint64 ifHCInOctets;                            /* RFC 2863 IfXEntry */
    uint64 ifHCOutOctets;                           /* RFC 2863 IfXEntry */
    uint32 etherStatsDropEvents;                    /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsTxUndersizePkts;               /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    uint32 etherStatsTxOversizePkts;                /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    uint32 etherStatsTxPkts64Octets;                /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    uint32 etherStatsTxPkts65to127Octets;           /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    uint32 etherStatsTxPkts128to255Octets;          /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    uint32 etherStatsTxPkts256to511Octets;          /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    uint32 etherStatsTxPkts512to1023Octets;         /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
    uint32 etherStatsTxPkts1024to1518Octets;        /* Proprietary counter. RFC 2819 EtherStatsEntry. Count TX packets ONLY */
#endif
#if defined(CONFIG_SDK_RTL8390)||defined(CONFIG_SDK_RTL8380)
    uint32 ifInMulticastPkts;                       /* RFC 2863 */
    uint32 ifInBroadcastPkts;                       /* RFC 2863 */
    uint32 dot1dTpPortInDiscards;                   /* RFC 1493 */
    uint32 etherStatsCRCAlignErrors;                /* RFC 2819 EtherStatsEntry */
    uint32 etherStatsRxUndersizePkts;               /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxUndersizeDropPkts;           /* Proprietary counter */
    uint32 etherStatsRxOversizePkts;                /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxPkts64Octets;                /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxPkts65to127Octets;           /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxPkts128to255Octets;          /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxPkts256to511Octets;          /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxPkts512to1023Octets;         /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxPkts1024to1518Octets;        /* Proprietary counter. RFC 2819 EtherStatsEntry. Count RX packets ONLY */
    uint32 etherStatsRxPkts1519toMaxOctets;         /* Proprietary counter */
    uint32 etherStatsTxBroadcastPkts;               /* Proprietary counter */
    uint32 etherStatsTxMulticastPkts;               /* Proprietary counter */
    uint32 etherStatsTxPkts1519toMaxOctets;         /* Proprietary counter */
    uint32 rxMacDiscards;                           /* Proprietary counter */
#endif
} rtk_stat_port_cntr_t;

typedef struct rtk_stat_smon_cntr_s
{
    uint32 smonPrioStatsPkts;
    uint64 smonPrioStatsOctets;
} rtk_stat_smon_cntr_t;

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : STAT */

/* Function Name:
 *      rtk_stat_init
 * Description:
 *      Initialize stat module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_PORT_CNTR_FAIL   - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize stat module before calling any stat APIs.
 */
extern int32
rtk_stat_init(uint32 unit);

/* Function Name:
 *      rtk_stat_enable_get
 * Description:
 *      Get the status to check whether MIB counters are enabled or not.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer buffer of enable or disable MIB counters
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
rtk_stat_enable_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_stat_enable_set
 * Description:
 *      Set the status to enable or disable MIB counters
 * Input:
 *      unit   - unit id
 *      enable - enable/disable MIB counters
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
extern int32
rtk_stat_enable_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_stat_global_reset
 * Description:
 *      Reset the global counters in the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_NOT_INIT              - The module is not initial
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_stat_global_reset(uint32 unit);

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_stat_port_reset(uint32 unit, rtk_port_t port);

/* Function Name:
 *      rtk_stat_global_get
 * Description:
 *      Get one specified global counter in the specified device.
 * Input:
 *      unit     - unit id
 *      cntr_idx - specified global counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      rtk_stat_global_type_t \ Chip :             8389    8328    8390    8380
 *      - DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX     O       O       O       O
 *      - DOT1D_TP_IN_DISCARDS_INDEX                O       X       X       X 
 *      - OUT_UCAST_PKTS_INDEX                      O       X       X       X 
 *      - OUT_MCAST_PKTS_INDEX                      O       X       X       X 
 *      - OUT_BCAST_PKTS_INDEX                      O       X       X       X 
 *      - EGR_LACK_RESOURCE_DROP_INDEX              O       X       X       X 
 */
extern int32
rtk_stat_global_get(uint32 unit, rtk_stat_global_type_t cntr_idx, uint64 *pCntr);

/* Function Name:
 *      rtk_stat_global_getAll
 * Description:
 *      Get all global counters in the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pGlobal_cntrs - pointer buffer of global counter structure
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_NOT_INIT              - The module is not initial
 *      RT_ERR_NULL_POINTER          - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_stat_global_getAll(uint32 unit, rtk_stat_global_cntr_t *pGlobal_cntrs);

/* Function Name:
 *      rtk_stat_port_get
 * Description:
 *      Get one specified port counter in the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      cntr_idx - specified port counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE        - input parameter out of range
 *      RT_ERR_INPUT               - Invalid input parameter
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      - rtk_stat_port_type_t \ Chip:                  8389    8328    8390    8380
 *      - IF_IN_OCTETS_INDEX                            O       O       O       O
 *      - IF_IN_UCAST_PKTS_INDEX                        O       O       O       O
 *      - IF_IN_MULTICAST_PKTS_INDEX                    X       X       O       O
 *      - IF_IN_BROADCAST_PKTS_INDEX                    X       X       O       O
 *      - IF_IN_DISCARDS_INDEX                          X       O       X       X
 *      - IF_OUT_OCTETS_INDEX                           O       O       O       O
 *      - IF_OUT_DISCARDS_INDEX                         X       O       O       O
 *      - IF_OUT_UCAST_PKTS_CNT_INDEX                   O       O       O       O
 *      - IF_OUT_MULTICAST_PKTS_CNT_INDEX               O       O       O       O
 *      - IF_OUT_BROADCAST_PKTS_CNT_INDEX               O       O       O       O
 *      - IP_IN_RECEIVES_INDEX                          X       O       X       X
 *      - IP_IN_DISCARDS_INDEX                          X       O       X       X
 *      - IF_HC_IN_OCTETS_INDEX                         X       O       O       O
 *      - IF_HC_IN_UCAST_PKTS_INDEX                     X       O       X       X
 *      - IF_HC_IN_MULTICAST_PKTS_INDEX                 X       O       X       X
 *      - IF_HC_IN_BROADCAST_PKTS_INDEX                 X       O       X       X
 *      - IF_HC_OUT_OCTETS_INDEX                        X       O       O       O
 *      - IF_HC_OUT_UCAST_PKTS_INDEX                    X       O       X       X
 *      - IF_HC_OUT_MULTICAST_PKTS_INDEX                X       O       X       X
 *      - IF_HC_OUT_BROADCAST_PKTS_INDEX                X       O       X       X
 *      - DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS_INDEX X       O       X       X
 *      - DOT1D_TP_PORT_IN_DISCARDS_INDEX               X       O       O       O
 *      - DOT1D_TP_HC_PORT_IN_DISCARDS_INDEX            X       O       X       X
 *      - DOT3_IN_PAUSE_FRAMES_INDEX                    O       O       O       O
 *      - DOT3_OUT_PAUSE_FRAMES_INDEX                   O       O       O       O
 *      - DOT3_OUT_PAUSE_ON_FRAMES_INDEX                O       O       X       X
 *      - DOT3_STATS_ALIGNMENT_ERRORS_INDEX             X       O       X       X
 *      - DOT3_STATS_FCS_ERRORS_INDEX                   O       O       X       X
 *      - DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX      O       O       O       O
 *      - DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX    O       O       O       O
 *      - DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX       O       O       O       O
 *      - DOT3_STATS_LATE_COLLISIONS_INDEX              O       O       O       O
 *      - DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX         O       O       O       O
 *      - DOT3_STATS_FRAME_TOO_LONGS_INDEX              X       O       X       X
 *      - DOT3_STATS_SYMBOL_ERRORS_INDEX                O       O       O       O
 *      - DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX         O       O       O       O
 *      - ETHER_STATS_DROP_EVENTS_INDEX                 X       O       O       O
 *      - ETHER_STATS_OCTETS_INDEX                      O       O       X       X
 *      - ETHER_STATS_BROADCAST_PKTS_INDEX              O       O       O       O
 *      - ETHER_STATS_MULTICAST_PKTS_INDEX              O       O       O       O
 *      - ETHER_STATS_UNDER_SIZE_PKTS_INDEX             O       O       O       O
 *      - ETHER_STATS_OVERSIZE_PKTS_INDEX               O       O       O       O
 *      - ETHER_STATS_FRAGMENTS_INDEX                   O       O       O       O
 *      - ETHER_STATS_JABBERS_INDEX                     O       O       O       O
 *      - ETHER_STATS_COLLISIONS_INDEX                  O       O       O       O
 *      - ETHER_STATS_CRC_ALIGN_ERRORS_INDEX            X       X       O       O
 *      - ETHER_STATS_PKTS_64OCTETS_INDEX               O       O       O       O
 *      - ETHER_STATS_PKTS_65TO127OCTETS_INDEX          O       O       O       O
 *      - ETHER_STATS_PKTS_128TO255OCTETS_INDEX         O       O       O       O
 *      - ETHER_STATS_PKTS_256TO511OCTETS_INDEX         O       O       O       O
 *      - ETHER_STATS_PKTS_512TO1023OCTETS_INDEX        O       O       O       O
 *      - ETHER_STATS_PKTS_1024TO1518OCTETS_INDEX       O       O       O       O
 *      - ETHER_STATS_TX_OCTETS_INDEX                   X       O       X       X
 *      - ETHER_STATS_TX_UNDER_SIZE_PKTS_INDEX          X       O       O       O
 *      - ETHER_STATS_TX_OVERSIZE_PKTS_INDEX            X       O       O       O
 *      - ETHER_STATS_TX_PKTS_64OCTETS_INDEX            X       O       O       O
 *      - ETHER_STATS_TX_PKTS_65TO127OCTETS_INDEX       X       O       O       O
 *      - ETHER_STATS_TX_PKTS_128TO255OCTETS_INDEX      X       O       O       O
 *      - ETHER_STATS_TX_PKTS_256TO511OCTETS_INDEX      X       O       O       O
 *      - ETHER_STATS_TX_PKTS_512TO1023OCTETS_INDEX     X       O       O       O
 *      - ETHER_STATS_TX_PKTS_1024TO1518OCTETS_INDEX    X       O       O       O
 *      - ETHER_STATS_TX_PKTS_1519TOMAXOCTETS_INDEX     X       X       O       O
 *      - ETHER_STATS_TX_BROADCAST_PKTS_INDEX           X       X       O       O
 *      - ETHER_STATS_TX_MULTICAST_PKTS_INDEX           X       X       O       O
 *      - ETHER_STATS_TX_FRAGMENTS_INDEX                X       X       X       X
 *      - ETHER_STATS_TX_JABBERS_INDEX                  X       X       X       X
 *      - ETHER_STATS_TX_CRC_ALIGN_ERROR_INDEX          X       X       X       X
 *      - ETHER_STATS_RX_UNDER_SIZE_PKTS_INDEX          X       X       O       O
 *      - ETHER_STATS_RX_UNDER_SIZE_DROP_PKTS_INDEX     X       X       O       O
 *      - ETHER_STATS_RX_OVERSIZE_PKTS_INDEX            X       X       O       O
 *      - ETHER_STATS_RX_PKTS_64OCTETS_INDEX            X       X       O       O
 *      - ETHER_STATS_RX_PKTS_65TO127OCTETS_INDEX       X       X       O       O
 *      - ETHER_STATS_RX_PKTS_128TO255OCTETS_INDEX      X       X       O       O
 *      - ETHER_STATS_RX_PKTS_256TO511OCTETS_INDEX      X       X       O       O
 *      - ETHER_STATS_RX_PKTS_512TO1023OCTETS_INDEX     X       X       O       O
 *      - ETHER_STATS_RX_PKTS_1024TO1518OCTETS_INDEX    X       X       O       O
 *      - ETHER_STATS_RX_PKTS_1519TOMAXOCTETS_INDEX     X       X       O       O
 *      - RX_LENGTH_FIELD_ERROR_INDEX                   X       X       O       X
 *      - RX_FALSE_CARRIER_TIMES_INDEX                  X       X       O       X
 *      - RX_UNDER_SIZE_OCTETS_INDEX                    X       X       O       X
 *      - TX_ETHER_STATS_FRAGMENTS_INDEX                X       X       O       X
 *      - TX_ETHER_STATS_JABBERS_INDEX                  X       X       O       X
 *      - TX_ETHER_STATS_CRC_ALIGN_ERRORS_INDEX         X       X       O       X
 *      - RX_FRAMING_ERRORS_INDEX                       X       X       O       X
 *      - IGR_LACK_PKT_BUF_DROP_INDEX                   O       X       X       X
 *      - FLOWCTRL_ON_DROP_PKT_CNT_INDEX                O       X       X       X
 *      - TX_CRC_CHECK_FAIL_CNT_INDEX                   O       X       X       X
 *      - SMART_TRIGGER_HIT0_INDEX                      O       X       X       X
 *      - SMART_TRIGGER_HIT1_INDEX                      O       X       X       X
 *      - RX_MAC_DISCARDS_INDEX                         X       X       O       O 
 */
extern int32
rtk_stat_port_get(uint32 unit, rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr);

/* Function Name:
 *      rtk_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pPort_cntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_stat_port_getAll(uint32 unit, rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs);

/* Function Name:
 *      rtk_stat_smon_get
 * Description:
 *      Get one specified SMON counter in specified device.
 * Input:
 *      unit     - unit id
 *      pri      - priority
 *      cntr_idx - index of specified SMON counter
 * Output:
 *      pCntr    - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_NULL_POINTER           - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL    - Could not retrieve/reset SMON Counter
 *      RT_ERR_STAT_INVALID_SMON_CNTR - Could not retrieve/reset SMON Counter
 * Applicable:
 *      8328
 * Note:
 *      (1) The cntr_idx value is as following:
 *          - SMON_PRIO_STATS_PKTS
 *          - SMON_PRIO_STATS_OCTETS
 */
extern int32
rtk_stat_smon_get(uint32 unit, rtk_pri_t pri, rtk_stat_smon_type_t cntr_idx, uint64 *pCntr);

/* Function Name:
 *      rtk_stat_smon_getAll
 * Description:
 *      Get all specified SMON counter in specified device.
 * Input:
 *      unit  - unit id
 *      pri   - priority
 * Output:
 *      pCntr - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_NULL_POINTER        - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL - Could not retrieve/reset SMON Counter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
extern int32
rtk_stat_smon_getAll(uint32 unit, rtk_pri_t pri, rtk_stat_smon_cntr_t *pCntr);

/* Function Name:
 *      rtk_stat_tagLenCntIncEnable_get
 * Description:
 *      Get RX/TX counter to include or exclude tag length in the specified device.
 * Input:
 *      unit        - unit id
 *      tagCnt_type - specified RX counter or TX counter
 * Output:
 *      pEnable     - pointer buffer of including/excluding tag length
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
 *      Inner(4B) and outer(4B) tag length can be included or excluded to the counter through the API.
 */
extern int32
rtk_stat_tagLenCntIncEnable_get(uint32 unit, rtk_stat_tagCnt_type_t tagCnt_type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_stat_tagLenCntIncEnable_set
 * Description:
 *      Set RX/TX counter to include or exclude tag length in the specified device.
 * Input:
 *      unit        - unit id
 *      tagCnt_type - specified RX counter or TX counter
 *      enable      - include/exclude Tag length
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
 *      Inner(4B) and outer(4B) tag length can be included or excluded to the counter through the API.
 */
extern int32
rtk_stat_tagLenCntIncEnable_set(uint32 unit, rtk_stat_tagCnt_type_t tagCnt_type, rtk_enable_t enable);

#endif /* __RTK_STAT_H__ */
