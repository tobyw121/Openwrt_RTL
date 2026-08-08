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
 * $Revision: 30752 $
 * $Date: 2012-07-09 11:10:14 +0800 (Mon, 09 Jul 2012) $
 *
 * Purpose : Define diag shell commands for counter
 * 
 * Feature : The file includes the following module and sub-modules
 *           1) counter commands.
 *
 */

#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <rtk/stat.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_MIB_DUMP_COUNTER_PORT_PORTS_ALL
/* 
 * mib dump counter port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_mib_dump_counter_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_stat_port_cntr_t port_cntrs;

    memset(&port_cntrs, 0, sizeof(rtk_stat_port_cntr_t));
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_ERR_CHK(rtk_stat_port_getAll(unit, port, &port_cntrs), ret);  
        
#if defined(CONFIG_SDK_RTL8390)
        diag_util_printf("Port %2d Counter\n", port);        
        diag_util_printf("\tdot1dTpPortInDiscards : %u\n", port_cntrs.dot1dTpPortInDiscards);
        diag_util_printf("\tifInOctets : %llu\n", port_cntrs.ifInOctets);    
        diag_util_printf("\tifHCInOctets : %llu\n", port_cntrs.ifHCInOctets);
        diag_util_printf("\tifInUcastPkts : %u\n", port_cntrs.ifInUcastPkts);
        diag_util_printf("\tifInMulticastPkts : %u\n", port_cntrs.ifInMulticastPkts);
        diag_util_printf("\tifInBroadcastPkts : %u\n", port_cntrs.ifInBroadcastPkts);
        diag_util_printf("\tifOutOctets : %llu\n", port_cntrs.ifOutOctets);
        diag_util_printf("\tifHCOutOctets : %llu\n", port_cntrs.ifHCOutOctets);
        diag_util_printf("\tifOutUcastPkts : %u\n", port_cntrs.ifOutUcastPkts);
        diag_util_printf("\tifOutMulticastPkts : %u\n", port_cntrs.ifOutMulticastPkts);
        diag_util_printf("\tifOutBroadcastPkts : %u\n", port_cntrs.ifOutBrocastPkts);
        diag_util_printf("\tifOutDiscards : %u\n", port_cntrs.ifOutDiscards);
        diag_util_printf("\tdot3StatsSingleCollisionFrames : %u\n", port_cntrs.dot3StatsSingleCollisionFrames);
        diag_util_printf("\tdot3StatsMultipleCollisionFrames : %u\n", port_cntrs.dot3StatsMultipleCollisionFrames);
        diag_util_printf("\tdot3StatsDeferredTransmissions : %u\n", port_cntrs.dot3StatsDeferredTransmissions);
        diag_util_printf("\tdot3StatsLateCollisions : %u\n", port_cntrs.dot3StatsLateCollisions);
        diag_util_printf("\tdot3StatsExcessiveCollisions : %u\n", port_cntrs.dot3StatsExcessiveCollisions);
        diag_util_printf("\tdot3StatsSymbolErrors : %u\n", port_cntrs.dot3StatsSymbolErrors);
        diag_util_printf("\tdot3ControlInUnknownOpcodes : %u\n", port_cntrs.dot3ControlInUnknownOpcodes);
        diag_util_printf("\tdot3InPauseFrames : %u\n", port_cntrs.dot3InPauseFrames);
        diag_util_printf("\tdot3OutPauseFrames : %u\n", port_cntrs.dot3OutPauseFrames);
        diag_util_printf("\tetherStatsDropEvents : %u\n", port_cntrs.etherStatsDropEvents);  
        diag_util_printf("\tetherStatsBroadcastPkts : %u\n", port_cntrs.etherStatsBroadcastPkts);  
        diag_util_printf("\tTX_etherStatsBroadcastPkts : %u\n", port_cntrs.etherStatsTxBroadcastPkts);  
        diag_util_printf("\tetherStatsMulticastPkts : %u\n", port_cntrs.etherStatsMulticastPkts);  
        diag_util_printf("\tTX_etherStatsMulticastPkts : %u\n", port_cntrs.etherStatsTxMulticastPkts);  
        diag_util_printf("\tetherStatsCRCAlignErrors : %u\n", port_cntrs.etherStatsCRCAlignErrors);  
        diag_util_printf("\tetherStatsUndersizePkts : %u\n", port_cntrs.etherStatsUndersizePkts);  
        diag_util_printf("\tRX_etherStatsUndersizePkts : %u\n", port_cntrs.etherStatsRxUndersizePkts);  
        diag_util_printf("\tRX_etherStatsUndersizeDropPkts : %u\n", port_cntrs.etherStatsRxUndersizeDropPkts);  
        diag_util_printf("\tTX_etherStatsUndersizePkts : %u\n", port_cntrs.etherStatsTxUndersizePkts);  
        diag_util_printf("\tetherStatsOversizePkts : %u\n", port_cntrs.etherStatsOversizePkts);  
        diag_util_printf("\tRX_etherStatsOversizePkts : %u\n", port_cntrs.etherStatsRxOversizePkts);  
        diag_util_printf("\tTX_etherStatsOversizePkts : %u\n", port_cntrs.etherStatsTxOversizePkts);  
        diag_util_printf("\tetherStatsFragments : %u\n", port_cntrs.etherStatsFragments);  
        diag_util_printf("\tetherStatsJabbers : %u\n", port_cntrs.etherStatsJabbers);  
        diag_util_printf("\tetherStatsCollisions : %u\n", port_cntrs.etherStatsCollisions);  
        diag_util_printf("\tetherStatsPkts64Octets : %u\n", port_cntrs.etherStatsPkts64Octets);  
        diag_util_printf("\tRX_etherStatsPkts64Octets : %u\n", port_cntrs.etherStatsRxPkts64Octets);  
        diag_util_printf("\tTX_etherStatsPkts64Octets : %u\n", port_cntrs.etherStatsTxPkts64Octets);  
        diag_util_printf("\tetherStatsPkts65to127Octets : %u\n", port_cntrs.etherStatsPkts65to127Octets);  
        diag_util_printf("\tRX_etherStatsPkts65to127Octets : %u\n", port_cntrs.etherStatsRxPkts65to127Octets);  
        diag_util_printf("\tTX_etherStatsPkts65to127Octets : %u\n", port_cntrs.etherStatsTxPkts65to127Octets);  
        diag_util_printf("\tetherStatsPkts128to255Octets : %u\n", port_cntrs.etherStatsPkts128to255Octets);  
        diag_util_printf("\tRX_etherStatsPkts128to255Octets : %u\n", port_cntrs.etherStatsRxPkts128to255Octets);  
        diag_util_printf("\tTX_etherStatsPkts128to255Octets : %u\n", port_cntrs.etherStatsTxPkts128to255Octets);  
        diag_util_printf("\tetherStatsPkts256to511Octets : %u\n", port_cntrs.etherStatsPkts256to511Octets);  
        diag_util_printf("\tRX_etherStatsPkts256to511Octets : %u\n", port_cntrs.etherStatsRxPkts256to511Octets);  
        diag_util_printf("\tTX_etherStatsPkts256to511Octets : %u\n", port_cntrs.etherStatsTxPkts256to511Octets);  
        diag_util_printf("\tetherStatsPkts512to1023Octets : %u\n", port_cntrs.etherStatsPkts512to1023Octets);  
        diag_util_printf("\tRX_etherStatsPkts512to1023Octets : %u\n", port_cntrs.etherStatsRxPkts512to1023Octets);  
        diag_util_printf("\tTX_etherStatsPkts512to1023Octets : %u\n", port_cntrs.etherStatsTxPkts512to1023Octets);  
        diag_util_printf("\tetherStatsPkts1024to1518Octets : %u\n", port_cntrs.etherStatsPkts1024to1518Octets);  
        diag_util_printf("\tRX_etherStatsPkts1024to1518Octets : %u\n", port_cntrs.etherStatsRxPkts1024to1518Octets);  
        diag_util_printf("\tTX_etherStatsPkts1024to1518Octets : %u\n", port_cntrs.etherStatsTxPkts1024to1518Octets);  
        diag_util_printf("\tRX_etherStatsPkts1519toMaxOctets : %u\n", port_cntrs.etherStatsRxPkts1519toMaxOctets);  
        diag_util_printf("\tTX_etherStatsPkts1519toMaxOctets : %u\n", port_cntrs.etherStatsTxPkts1519toMaxOctets);  
        diag_util_printf("\tRX_lengthFieldError : %u\n", port_cntrs.rxLengthFieldError);  
        diag_util_printf("\tRX_falseCarrierTimes : %u\n", port_cntrs.rxFalseCarrierTimes);  
        diag_util_printf("\tRX_underSizeOctets : %u\n", port_cntrs.rxUnderSizeOctets);  
        diag_util_printf("\tTX_etherStatsFragments : %u\n", port_cntrs.txEtherStatsFragments);  
        diag_util_printf("\tTX_etherStatsJabbers : %u\n", port_cntrs.txEtherStatsJabbers);  
        diag_util_printf("\tTX_etherStatsCRCAlignError : %u\n", port_cntrs.txEtherStatsCRCAlignError);  
        diag_util_printf("\tRX_FramingErrors : %u\n", port_cntrs.rxFramingErrors);  
        diag_util_printf("\trxMacDiscards : %u\n", port_cntrs.rxMacDiscards);  
#elif defined(CONFIG_SDK_RTL8380)
        diag_util_printf("Port %2d Counter\n", port);        
        diag_util_printf("\tdot1dTpPortInDiscards : %d\n", port_cntrs.dot1dTpPortInDiscards);
        diag_util_printf("\tifInOctets : %llu\n", port_cntrs.ifInOctets);    
        diag_util_printf("\tifHCInOctets : %llu\n", port_cntrs.ifHCInOctets);
        diag_util_printf("\tifInUcastPkts : %d\n", port_cntrs.ifInUcastPkts);
        diag_util_printf("\tifInMulticastPkts : %d\n", port_cntrs.ifInMulticastPkts);
        diag_util_printf("\tifInBroadcastPkts : %d\n", port_cntrs.ifInBroadcastPkts);
        diag_util_printf("\tifOutOctets : %llu\n", port_cntrs.ifOutOctets);
        diag_util_printf("\tifHCOutOctets : %llu\n", port_cntrs.ifHCOutOctets);
        diag_util_printf("\tifOutUcastPkts : %d\n", port_cntrs.ifOutUcastPkts);
        diag_util_printf("\tifOutMulticastPkts : %d\n", port_cntrs.ifOutMulticastPkts);
        diag_util_printf("\tifOutBrocastPkts : %d\n", port_cntrs.ifOutBrocastPkts);
        diag_util_printf("\tifOutDiscards : %d\n", port_cntrs.ifOutDiscards);
        diag_util_printf("\tdot3StatsSingleCollisionFrames : %d\n", port_cntrs.dot3StatsSingleCollisionFrames);
        diag_util_printf("\tdot3StatsMultipleCollisionFrames : %d\n", port_cntrs.dot3StatsMultipleCollisionFrames);
        diag_util_printf("\tdot3StatsDeferredTransmissions : %d\n", port_cntrs.dot3StatsDeferredTransmissions);
        diag_util_printf("\tdot3StatsLateCollisions : %d\n", port_cntrs.dot3StatsLateCollisions);
        diag_util_printf("\tdot3StatsExcessiveCollisions : %d\n", port_cntrs.dot3StatsExcessiveCollisions);
        diag_util_printf("\tdot3StatsSymbolErrors : %d\n", port_cntrs.dot3StatsSymbolErrors);
        diag_util_printf("\tdot3ControlInUnknownOpcodes : %d\n", port_cntrs.dot3ControlInUnknownOpcodes);
        diag_util_printf("\tdot3InPauseFrames : %d\n", port_cntrs.dot3InPauseFrames);
        diag_util_printf("\tdot3OutPauseFrames : %d\n", port_cntrs.dot3OutPauseFrames);
        diag_util_printf("\tetherStatsDropEvents : %d\n", port_cntrs.etherStatsDropEvents);  
        diag_util_printf("\tetherStatsBroadcastPkts : %d\n", port_cntrs.etherStatsBroadcastPkts);  
        diag_util_printf("\tTX_etherStatsBroadcastPkts : %d\n", port_cntrs.etherStatsTxBroadcastPkts);  
        diag_util_printf("\tetherStatsMulticastPkts : %d\n", port_cntrs.etherStatsMulticastPkts);  
        diag_util_printf("\tTX_etherStatsMulticastPkts : %d\n", port_cntrs.etherStatsTxMulticastPkts);  
        diag_util_printf("\tetherStatsCRCAlignErrors : %d\n", port_cntrs.etherStatsCRCAlignErrors);  
        diag_util_printf("\tetherStatsUndersizePkts : %d\n", port_cntrs.etherStatsUndersizePkts);  
        diag_util_printf("\tRX_etherStatsUndersizePkts : %d\n", port_cntrs.etherStatsRxUndersizePkts);  
        diag_util_printf("\tRX_etherStatsUndersizeDropPkts : %d\n", port_cntrs.etherStatsRxUndersizeDropPkts);  
        diag_util_printf("\tTX_etherStatsUndersizePkts : %d\n", port_cntrs.etherStatsTxUndersizePkts);  
        diag_util_printf("\tetherStatsOversizePkts : %d\n", port_cntrs.etherStatsOversizePkts);  
        diag_util_printf("\tRX_etherStatsOversizePkts : %d\n", port_cntrs.etherStatsRxOversizePkts);  
        diag_util_printf("\tTX_etherStatsOversizePkts : %d\n", port_cntrs.etherStatsTxOversizePkts);  
        diag_util_printf("\tetherStatsFragments : %d\n", port_cntrs.etherStatsFragments);  
        diag_util_printf("\tetherStatsJabbers : %d\n", port_cntrs.etherStatsJabbers);  
        diag_util_printf("\tetherStatsCollisions : %d\n", port_cntrs.etherStatsCollisions);  
        diag_util_printf("\tetherStatsPkts64Octets : %d\n", port_cntrs.etherStatsPkts64Octets);  
        diag_util_printf("\tRX_etherStatsPkts64Octets : %d\n", port_cntrs.etherStatsRxPkts64Octets);  
        diag_util_printf("\tTX_etherStatsPkts64Octets : %d\n", port_cntrs.etherStatsTxPkts64Octets);  
        diag_util_printf("\tetherStatsPkts65to127Octets : %d\n", port_cntrs.etherStatsPkts65to127Octets);  
        diag_util_printf("\tRX_etherStatsPkts65to127Octets : %d\n", port_cntrs.etherStatsRxPkts65to127Octets);  
        diag_util_printf("\tTX_etherStatsPkts65to127Octets : %d\n", port_cntrs.etherStatsTxPkts65to127Octets);  
        diag_util_printf("\tetherStatsPkts128to255Octets : %d\n", port_cntrs.etherStatsPkts128to255Octets);  
        diag_util_printf("\tRX_etherStatsPkts128to255Octets : %d\n", port_cntrs.etherStatsRxPkts128to255Octets);  
        diag_util_printf("\tTX_etherStatsPkts128to255Octets : %d\n", port_cntrs.etherStatsTxPkts128to255Octets);  
        diag_util_printf("\tetherStatsPkts256to511Octets : %d\n", port_cntrs.etherStatsPkts256to511Octets);  
        diag_util_printf("\tRX_etherStatsPkts256to511Octets : %d\n", port_cntrs.etherStatsRxPkts256to511Octets);  
        diag_util_printf("\tTX_etherStatsPkts256to511Octets : %d\n", port_cntrs.etherStatsTxPkts256to511Octets);  
        diag_util_printf("\tetherStatsPkts512to1023Octets : %d\n", port_cntrs.etherStatsPkts512to1023Octets);  
        diag_util_printf("\tRX_etherStatsPkts512to1023Octets : %d\n", port_cntrs.etherStatsRxPkts512to1023Octets);  
        diag_util_printf("\tTX_etherStatsPkts512to1023Octets : %d\n", port_cntrs.etherStatsTxPkts512to1023Octets);  
        diag_util_printf("\tetherStatsPkts1024to1518Octets : %d\n", port_cntrs.etherStatsPkts1024to1518Octets);  
        diag_util_printf("\tRX_etherStatsPkts1024to1518Octets : %d\n", port_cntrs.etherStatsRxPkts1024to1518Octets);  
        diag_util_printf("\tTX_etherStatsPkts1024to1518Octets : %d\n", port_cntrs.etherStatsTxPkts1024to1518Octets);  
        diag_util_printf("\tRX_etherStatsPkts1519toMaxOctets : %d\n", port_cntrs.etherStatsRxPkts1519toMaxOctets);  
        diag_util_printf("\tTX_etherStatsPkts1519toMaxOctets : %d\n", port_cntrs.etherStatsTxPkts1519toMaxOctets);  
        diag_util_printf("\tRX_MacDiscards : %d\n", port_cntrs.rxMacDiscards);  
#if 0
        diag_util_printf("\tRX_lengthFieldError : %d\n", port_cntrs.rxLengthFieldError);  
        diag_util_printf("\tRX_falseCarrierTimes : %d\n", port_cntrs.rxFalseCarrierTimes);  
        diag_util_printf("\tRX_underSizeOctets : %d\n", port_cntrs.rxUnderSizeOctets);  
        diag_util_printf("\tTX_etherStatsFragments : %d\n", port_cntrs.txEtherStatsFragments);  
        diag_util_printf("\tTX_etherStatsJabbers : %d\n", port_cntrs.txEtherStatsJabbers);  
        diag_util_printf("\tTX_etherStatsCRCAlignError : %d\n", port_cntrs.txEtherStatsCRCAlignError);  
#endif
#else
        diag_util_printf("Port %d Counter\n", port);        
        diag_util_printf("\tifOutOctets= %llu\n", port_cntrs.ifOutOctets);
        diag_util_printf("\tifInOctets= %llu\n", port_cntrs.ifInOctets);    
        diag_util_printf("\tifInUcastPkts= %d\n", port_cntrs.ifInUcastPkts);
        diag_util_printf("\tifInDiscards= %d\n", port_cntrs.ifInDiscards);
        diag_util_printf("\tifOutDiscards= %d\n", port_cntrs.ifOutDiscards);
        diag_util_printf("\tipInReceives= %d\n", port_cntrs.ipInReceives);
        diag_util_printf("\tipInDiscards= %d\n", port_cntrs.ipInDiscards);
        diag_util_printf("\tifHCInOctets= %llu\n", port_cntrs.ifHCInOctets);
        diag_util_printf("\tifHCInUcastPkts= %llu\n", port_cntrs.ifHCInUcastPkts);
        diag_util_printf("\tifHCInMulticastPkts= %llu\n", port_cntrs.ifHCInMulticastPkts);
        diag_util_printf("\tifHCInBroadcastPkts= %llu\n", port_cntrs.ifHCInBroadcastPkts);
        diag_util_printf("\tifHCOutOctets= %llu\n", port_cntrs.ifHCOutOctets);
        diag_util_printf("\tifHCOutUcastPkts= %llu\n", port_cntrs.ifHCOutUcastPkts);
        diag_util_printf("\tifHCOutMulticastPkts= %llu\n", port_cntrs.ifHCOutMulticastPkts);
        diag_util_printf("\tifHCOutBroadcastPkts= %llu\n", port_cntrs.ifHCOutBroadcastPkts);
        diag_util_printf("\tdot1dBasePortDelayExceededDiscards= %d\n", port_cntrs.dot1dBasePortDelayExceededDiscards);
        diag_util_printf("\tdot1dTPHCPortInDiscards= %llu\n", port_cntrs.dot1dTPHCPortInDiscards);
        diag_util_printf("\tdot3StatsAlignmentErrors= %d\n", port_cntrs.dot3StatsAlignmentErrors);
        diag_util_printf("\tdot3StatsFrameTooLongs= %d\n", port_cntrs.dot3StatsFrameTooLongs);
        diag_util_printf("\tdot3OutPauseFrames= %d\n", port_cntrs.dot3OutPauseFrames);
        diag_util_printf("\tdot3OutPauseOnFrames= %d\n", port_cntrs.dot3OutPauseOnFrames);
        diag_util_printf("\tdot3StatsExcessiveCollisions= %d\n", port_cntrs.dot3StatsExcessiveCollisions);
        diag_util_printf("\tdot3StatsLateCollisions= %d\n", port_cntrs.dot3StatsLateCollisions);
        diag_util_printf("\tdot3StatsDeferredTransmissions= %d\n", port_cntrs.dot3StatsDeferredTransmissions);
        diag_util_printf("\tdot3StatsMultipleCollisionFrames= %d\n", port_cntrs.dot3StatsMultipleCollisionFrames);
        diag_util_printf("\tdot3StatsSingleCollisionFrames= %d\n", port_cntrs.dot3StatsSingleCollisionFrames);
        /*diag_util_printf("\tdot3ControlInUnknownOpcodes= %d\n", port_cntrs.dot3ControlInUnknownOpcodes);*/
        diag_util_printf("\tdot3InPauseFrames= %d\n", port_cntrs.dot3InPauseFrames);
        diag_util_printf("\tdot3StatsSymbolErrors= %d\n", port_cntrs.dot3StatsSymbolErrors);
        diag_util_printf("\tdot3StatsFCSErrors= %d\n", port_cntrs.dot3StatsFCSErrors);
        diag_util_printf("\tetherStatsDropEvents= %d\n", port_cntrs.etherStatsDropEvents);  
        diag_util_printf("\tetherStatsJabbers= %d\n", port_cntrs.etherStatsJabbers);
        diag_util_printf("\tetherStatsCollisions= %d\n", port_cntrs.etherStatsCollisions);
        /*diag_util_printf("\tetherStatsMulticastPkts= %d\n", port_cntrs.etherStatsMulticastPkts);*/
        /*diag_util_printf("\tetherStatsBroadcastPkts= %d\n", port_cntrs.etherStatsBroadcastPkts);*/        
        diag_util_printf("\tetherStatsFragments= %d\n", port_cntrs.etherStatsFragments);
        diag_util_printf("\tetherStatsUndersizePkts= %d\n", port_cntrs.etherStatsUndersizePkts);  
        diag_util_printf("\tetherStatsOctets= %llu\n", port_cntrs.etherStatsOctets);  
        diag_util_printf("\tetherStatsPkts64Octets= %d\n", port_cntrs.etherStatsPkts64Octets);
        diag_util_printf("\tetherStatsPkts65to127Octets= %d\n", port_cntrs.etherStatsPkts65to127Octets);
        diag_util_printf("\tetherStatsPkts128to255Octets= %d\n", port_cntrs.etherStatsPkts128to255Octets);
        diag_util_printf("\tetherStatsPkts256to511Octets= %d\n", port_cntrs.etherStatsPkts256to511Octets);
        diag_util_printf("\tetherStatsPkts512to1023Octets= %d\n", port_cntrs.etherStatsPkts512to1023Octets);
        diag_util_printf("\tetherStatsPkts1024toMaxOctets= %d\n", port_cntrs.etherStatsPkts1024toMaxOctets);
        diag_util_printf("\tetherStatsOversizePkts= %d\n", port_cntrs.etherStatsOversizePkts);          
        diag_util_printf("\tetherStatsTxUndersizePkts= %d\n", port_cntrs.etherStatsTxUndersizePkts);   
        diag_util_printf("\tetherStatsTxOctets= %llu\n", port_cntrs.etherStatsTxOctets);   
        diag_util_printf("\tetherStatsTxPkts64Octets= %d\n", port_cntrs.etherStatsTxPkts64Octets);
        diag_util_printf("\tetherStatsTxPkts65to127Octets= %d\n", port_cntrs.etherStatsTxPkts65to127Octets);
        diag_util_printf("\tetherStatsTxPkts128to255Octets= %d\n", port_cntrs.etherStatsTxPkts128to255Octets);
        diag_util_printf("\tetherStatsTxPkts256to511Octets= %d\n", port_cntrs.etherStatsTxPkts256to511Octets);
        diag_util_printf("\tetherStatsTxPkts512to1023Octets= %d\n", port_cntrs.etherStatsTxPkts512to1023Octets);
        diag_util_printf("\tetherStatsTxPkts1024toMaxOctets= %d\n", port_cntrs.etherStatsTxPkts1024toMaxOctets);
        diag_util_printf("\tetherStatsTxOversizePkts= %d\n", port_cntrs.etherStatsTxOversizePkts);  
#if defined(CONFIG_SDK_RTL8389)
        diag_util_printf("\tigrLackPktBufDrop= %d\n", port_cntrs.igrLackPktBufDrop);
        diag_util_printf("\tflowCtrlOnDropPktCnt= %d\n", port_cntrs.flowCtrlOnDropPktCnt);
        diag_util_printf("\ttxCrcCheckFailCnt= %d\n", port_cntrs.txCrcCheckFailCnt);
        diag_util_printf("\tsmartTriggerHit0= %d\n", port_cntrs.smartTriggerHit0);
        diag_util_printf("\tsmartTriggerHit1= %d\n", port_cntrs.smartTriggerHit1);
#endif
        diag_util_printf("\tifOutUcastPkts= %d\n", port_cntrs.ifOutUcastPkts);
        diag_util_printf("\tifOutMulticastPkts= %d\n", port_cntrs.ifOutMulticastPkts);
        diag_util_printf("\tifOutBrocastPkts= %d\n\n", port_cntrs.ifOutBrocastPkts);
#endif
    }  
    
    return CPARSER_OK;        
}
#endif

#ifdef CMD_MIB_DUMP_COUNTER_SMON_PRI
/* 
 * mib dump counter smon <UINT:pri>
 */
cparser_result_t cparser_cmd_mib_dump_counter_smon_pri(cparser_context_t *context,
    uint32_t *pri_ptr)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_stat_smon_cntr_t smon_cntrs;

    memset(&smon_cntrs, 0, sizeof(rtk_stat_smon_cntr_t));     
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_stat_smon_getAll(unit, *pri_ptr, &smon_cntrs), ret);  

    diag_util_printf("Priority %d Counter\n", *pri_ptr);
    diag_util_printf("\tsmonPrioStatsOctets= %llu\n", smon_cntrs.smonPrioStatsOctets);
    diag_util_printf("\tsmonPrioStatsPkts= %d\n", smon_cntrs.smonPrioStatsPkts);   

    return CPARSER_OK;        
}
#endif

#ifdef CMD_MIB_DUMP_COUNTER_GLOBAL
/* 
 * mib dump counter global
 */    
cparser_result_t cparser_cmd_mib_dump_counter_global(cparser_context_t *context)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_stat_global_cntr_t global_cntrs;

    memset(&global_cntrs, 0, sizeof(rtk_stat_global_cntr_t));
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_stat_global_getAll(unit, &global_cntrs), ret);  

    diag_util_printf("Global Counter\n");
#if defined(CONFIG_SDK_RTL8390)
    diag_util_printf("\tdot1dTpLearnedEntryDiscards : %d\n", global_cntrs.dot1dTpLearnedEntryDiscards);
#elif defined(CONFIG_SDK_RTL8380)
    diag_util_printf("\tdot1dTpLearnedEntryDiscards : %d\n", global_cntrs.dot1dTpLearnedEntryDiscards);
#else
    diag_util_printf("\tdot1dTpLearnedEntryDiscards : %d\n", global_cntrs.dot1dTpLearnedEntryDiscards);
    diag_util_printf("\tdot1dTpPortInDiscards : %d\n", global_cntrs.dot1dTpPortInDiscards);
    diag_util_printf("\tOutUnicastPktsCnt : %d\n", global_cntrs.OutUnicastPktsCnt);
    diag_util_printf("\tOutMulticastPktsCnt : %d\n", global_cntrs.OutMulticastPktsCnt);
    diag_util_printf("\tOutBrocastPktsCnt : %d\n", global_cntrs.OutBrocastPktsCnt);
    diag_util_printf("\tegrLackResourceDrop : %d\n", global_cntrs.egrLackResourceDrop);
#endif
    return CPARSER_OK;        
}
#endif

#ifdef CMD_MIB_RESET_COUNTER_PORT_PORTS_ALL
/* 
 * mib reset counter port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_mib_reset_counter_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
         
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_ERR_CHK(rtk_stat_port_reset(unit, port), ret);  
    }

    return CPARSER_OK;        
}
#endif

#ifdef CMD_MIB_RESET_COUNTER_GLOBAL
/* 
 * mib reset counter global
 */    
cparser_result_t cparser_cmd_mib_reset_counter_global(cparser_context_t *context)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
         
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);  
    DIAG_UTIL_ERR_CHK(rtk_stat_global_reset(unit), ret);  

    return CPARSER_OK;        
}
#endif

#ifdef CMD_MIB_DUMP_COUNTER_PORT_PORTS_ALL_COUNTER_NAME
/*
 * mib dump counter port ( <PORT_LIST:ports> | all ) <UINT:counter_name>
 */
cparser_result_t cparser_cmd_mib_dump_counter_port_ports_all_counter_name(cparser_context_t *context,
    char **counter_name_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_stat_port_cntr_t port_cntrs;
    uint8   *counter_name;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();

    counter_name = (uint8 *)*counter_name_ptr;
    memset(&port_cntrs, 0, sizeof(rtk_stat_port_cntr_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_ERR_CHK(rtk_stat_port_getAll(unit, port, &port_cntrs), ret);  
        
        diag_util_printf("Port %2d Counter\n", port);        

        if (0 == strcmp(context->parser->tokens[5].buf, "dot1dTpPortInDiscards"))
        {
            diag_util_printf("\tdot1dTpPortInDiscards : %d\n", port_cntrs.dot1dTpPortInDiscards);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifInOctets"))
        {
            diag_util_printf("\tifInOctets : %llu\n", port_cntrs.ifInOctets);    
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifHCInOctets"))
        {
            diag_util_printf("\tifHCInOctets : %llu\n", port_cntrs.ifHCInOctets);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifInUcastPkts"))
        {
            diag_util_printf("\tifInUcastPkts : %d\n", port_cntrs.ifInUcastPkts);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifInMulticastPkts"))
        {
            diag_util_printf("\tifInMulticastPkts : %d\n", port_cntrs.ifInMulticastPkts);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifInBroadcastPkts"))
        {
            diag_util_printf("\tifInBroadcastPkts : %d\n", port_cntrs.ifInBroadcastPkts);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifOutOctets"))
        {
            diag_util_printf("\tifOutOctets : %llu\n", port_cntrs.ifOutOctets);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifHCOutOctets"))
        {
            diag_util_printf("\tifHCOutOctets : %llu\n", port_cntrs.ifHCOutOctets);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifOutUcastPkts"))
        {
            diag_util_printf("\tifOutUcastPkts : %d\n", port_cntrs.ifOutUcastPkts);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifOutMulticastPkts"))
        {
            diag_util_printf("\tifOutMulticastPkts : %d\n", port_cntrs.ifOutMulticastPkts);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifOutBroadcastPkts"))
        {
            diag_util_printf("\tifOutBroadcastPkts : %d\n", port_cntrs.ifOutBrocastPkts);
        }    
        else if (0 == strcmp(context->parser->tokens[5].buf, "ifOutDiscards"))
        {
            diag_util_printf("\tifOutDiscards : %d\n", port_cntrs.ifOutDiscards);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3StatsSingleCollisionFrames"))
        {
            diag_util_printf("\tdot3StatsSingleCollisionFrames : %d\n", port_cntrs.dot3StatsSingleCollisionFrames);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3StatsMultipleCollisionFrames"))
        {
            diag_util_printf("\tdot3StatsMultipleCollisionFrames : %d\n", port_cntrs.dot3StatsMultipleCollisionFrames);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3StatsDeferredTransmissions"))
        {
            diag_util_printf("\tdot3StatsDeferredTransmissions : %d\n", port_cntrs.dot3StatsDeferredTransmissions);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3StatsLateCollisions"))
        {
            diag_util_printf("\tdot3StatsLateCollisions : %d\n", port_cntrs.dot3StatsLateCollisions);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3StatsExcessiveCollisions"))
        {
            diag_util_printf("\tdot3StatsExcessiveCollisions : %d\n", port_cntrs.dot3StatsExcessiveCollisions);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3StatsSymbolErrors"))
        {
            diag_util_printf("\tdot3StatsSymbolErrors : %d\n", port_cntrs.dot3StatsSymbolErrors);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3ControlInUnknownOpcodes"))
        {
            diag_util_printf("\tdot3ControlInUnknownOpcodes : %d\n", port_cntrs.dot3ControlInUnknownOpcodes);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3InPauseFrames"))
        {
            diag_util_printf("\tdot3InPauseFrames : %d\n", port_cntrs.dot3InPauseFrames);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "dot3OutPauseFrames"))
        {
            diag_util_printf("\tdot3OutPauseFrames : %d\n", port_cntrs.dot3OutPauseFrames);
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsDropEvents"))
        {
            diag_util_printf("\tetherStatsDropEvents : %d\n", port_cntrs.etherStatsDropEvents);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsBroadcastPkts"))
        {
            diag_util_printf("\tetherStatsBroadcastPkts : %d\n", port_cntrs.etherStatsBroadcastPkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsTxBroadcastPkts"))
        {
            diag_util_printf("\tTX_etherStatsBroadcastPkts : %d\n", port_cntrs.etherStatsTxBroadcastPkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsMulticastPkts"))
        {
            diag_util_printf("\tetherStatsMulticastPkts : %d\n", port_cntrs.etherStatsMulticastPkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsTxMulticastPkts"))
        {
            diag_util_printf("\tTX_etherStatsMulticastPkts : %d\n", port_cntrs.etherStatsTxMulticastPkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsCRCAlignErrors"))
        {
            diag_util_printf("\tetherStatsCRCAlignErrors : %d\n", port_cntrs.etherStatsCRCAlignErrors);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsUndersizePkts"))
        {
            diag_util_printf("\tetherStatsUndersizePkts : %d\n", port_cntrs.etherStatsUndersizePkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsUndersizePkts"))
        {
            diag_util_printf("\tTX_etherStatsUndersizePkts : %d\n", port_cntrs.etherStatsTxUndersizePkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsUndersizePkts"))
        {
            diag_util_printf("\tRX_etherStatsUndersizePkts : %d\n", port_cntrs.etherStatsRxUndersizePkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsUndersizeDropPkts"))
        {
            diag_util_printf("\tRX_etherStatsUndersizeDropPkts : %d\n", port_cntrs.etherStatsRxUndersizeDropPkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsOversizePkts"))
        {
            diag_util_printf("\tetherStatsOversizePkts : %d\n", port_cntrs.etherStatsOversizePkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsOversizePkts"))
        {
            diag_util_printf("\tTX_etherStatsOversizePkts : %d\n", port_cntrs.etherStatsTxOversizePkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsOversizePkts"))
        {
            diag_util_printf("\tRX_etherStatsOversizePkts : %d\n", port_cntrs.etherStatsRxOversizePkts);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsFragments"))
        {
            diag_util_printf("\tetherStatsFragments : %d\n", port_cntrs.etherStatsFragments);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsJabbers"))
        {
            diag_util_printf("\tetherStatsJabbers : %d\n", port_cntrs.etherStatsJabbers);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsCollisions"))
        {
            diag_util_printf("\tetherStatsCollisions : %d\n", port_cntrs.etherStatsCollisions);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsPkts64Octets"))
        {
            diag_util_printf("\tetherStatsPkts64Octets : %d\n", port_cntrs.etherStatsPkts64Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsPkts64Octets"))
        {
            diag_util_printf("\tTX_etherStatsPkts64Octets : %d\n", port_cntrs.etherStatsTxPkts64Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsPkts64Octets"))
        {
            diag_util_printf("\tRX_etherStatsPkts64Octets : %d\n", port_cntrs.etherStatsRxPkts64Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsPkts65to127Octets"))
        {
            diag_util_printf("\tetherStatsPkts65to127Octets : %d\n", port_cntrs.etherStatsPkts65to127Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsPkts65to127Octets"))
        {
            diag_util_printf("\tTX_etherStatsPkts65to127Octets : %d\n", port_cntrs.etherStatsTxPkts65to127Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsPkts65to127Octets"))
        {
            diag_util_printf("\tRX_etherStatsPkts65to127Octets : %d\n", port_cntrs.etherStatsRxPkts65to127Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsPkts128to255Octets"))
        {
            diag_util_printf("\tetherStatsPkts128to255Octets : %d\n", port_cntrs.etherStatsPkts128to255Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsPkts128to255Octets"))
        {
            diag_util_printf("\tTX_etherStatsPkts128to255Octets : %d\n", port_cntrs.etherStatsTxPkts128to255Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsPkts128to255Octets"))
        {
            diag_util_printf("\tRX_etherStatsPkts128to255Octets : %d\n", port_cntrs.etherStatsRxPkts128to255Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsPkts256to511Octets"))
        {
            diag_util_printf("\tetherStatsPkts256to511Octets : %d\n", port_cntrs.etherStatsPkts256to511Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsPkts256to511Octets"))
        {
            diag_util_printf("\tTX_etherStatsPkts256to511Octets : %d\n", port_cntrs.etherStatsTxPkts256to511Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsPkts256to511Octets"))
        {
            diag_util_printf("\tRX_etherStatsPkts256to511Octets : %d\n", port_cntrs.etherStatsRxPkts256to511Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsPkts512to1023Octets"))
        {
            diag_util_printf("\tetherStatsPkts512to1023Octets : %d\n", port_cntrs.etherStatsPkts512to1023Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsPkts512to1023Octets"))
        {
            diag_util_printf("\tTX_etherStatsPkts512to1023Octets : %d\n", port_cntrs.etherStatsTxPkts512to1023Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsPkts512to1023Octets"))
        {
            diag_util_printf("\tRX_etherStatsPkts512to1023Octets : %d\n", port_cntrs.etherStatsRxPkts512to1023Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "etherStatsPkts1024to1518Octets"))
        {
            diag_util_printf("\tetherStatsPkts1024to1518Octets : %d\n", port_cntrs.etherStatsPkts1024to1518Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsPkts1024to1518Octets"))
        {
            diag_util_printf("\tTX_etherStatsPkts1024to1518Octets : %d\n", port_cntrs.etherStatsTxPkts1024to1518Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsPkts1024to1518Octets"))
        {
            diag_util_printf("\tRX_etherStatsPkts1024to1518Octets : %d\n", port_cntrs.etherStatsRxPkts1024to1518Octets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsPkts1519toMaxOctets"))
        {
            diag_util_printf("\tTX_etherStatsPkts1519toMaxOctets : %d\n", port_cntrs.etherStatsTxPkts1519toMaxOctets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_etherStatsPkts1519toMaxOctets"))
        {
            diag_util_printf("\tRX_etherStatsPkts1519toMaxOctets : %d\n", port_cntrs.etherStatsRxPkts1519toMaxOctets);  
        }
#if defined(CONFIG_SDK_RTL8390)
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_lengthFieldError"))
        {
            diag_util_printf("\tRX_lengthFieldError : %u\n", port_cntrs.rxLengthFieldError);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_falseCarrierTimes"))
        {
            diag_util_printf("\tRX_falseCarrierTimes : %u\n", port_cntrs.rxFalseCarrierTimes);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "rx_underSizeOctets"))
        {
            diag_util_printf("\tRX_underSizeOctets : %u\n", port_cntrs.rxUnderSizeOctets);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsFragments"))
        {
            diag_util_printf("\tTX_etherStatsFragments : %u\n", port_cntrs.txEtherStatsFragments);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsJabbers"))
        {
            diag_util_printf("\tTX_etherStatsJabbers : %u\n", port_cntrs.txEtherStatsJabbers);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "tx_etherStatsCRCAlignError"))
        {
            diag_util_printf("\tTX_etherStatsCRCAlignError : %u\n", port_cntrs.txEtherStatsCRCAlignError);  
        }
        else if (0 == strcmp(context->parser->tokens[5].buf, "RX_FramingErrors"))
        {
            diag_util_printf("\tRX_FramingErrors : %u\n", port_cntrs.rxFramingErrors);  
        }
#endif
#if   defined(CONFIG_SDK_RTL8380) ||defined(CONFIG_SDK_RTL8390)
        else if (0 == strcmp(context->parser->tokens[5].buf, "RX_MacDiscards"))
        {
            diag_util_printf("\tRX_MacDiscards : %d\n", port_cntrs.rxMacDiscards);  
        }
#endif
        else 
        {
            diag_util_printf("\tNot Support\n");  
        }
    }  

    return CPARSER_OK;    
}
#endif

#ifdef CMD_MIB_GET_TAG_LENGTH_RX_COUNTER_TX_COUNTER
/*
 * mib get tag-length ( rx-counter | tx-counter )
 */
cparser_result_t cparser_cmd_mib_get_tag_length_rx_counter_tx_counter(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_stat_tagCnt_type_t type;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (0 == strcmp(context->parser->tokens[3].buf, "rx-counter"))
    {
        type = TAG_CNT_TYPE_RX;
    }
    else if (0 == strcmp(context->parser->tokens[3].buf, "tx-counter"))
    {
        type = TAG_CNT_TYPE_TX;
    }
    else
    {
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_stat_tagLenCntIncEnable_get(unit, type, &enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_printf("%s MIB Counter Tag Length Configuration : %s\n", 
        (type == TAG_CNT_TYPE_RX) ? "RX" : "TX", (enable == ENABLED) ? "Include" : "Exclude");

    return CPARSER_OK;
}
#endif


#ifdef CMD_MIB_SET_TAG_LENGTH_RX_COUNTER_TX_COUNTER_EXCLUDE_INCLUDE
/*
 * mib set tag-length ( rx-counter | tx-counter ) ( exclude | include )
 */
cparser_result_t cparser_cmd_mib_set_tag_length_rx_counter_tx_counter_exclude_include(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_stat_tagCnt_type_t type;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (0 == strcmp(context->parser->tokens[3].buf, "rx-counter"))
    {
        type = TAG_CNT_TYPE_RX;
    }
    else if (0 == strcmp(context->parser->tokens[3].buf, "tx-counter"))
    {
        type = TAG_CNT_TYPE_TX;
    }
    else
    {
        return CPARSER_NOT_OK;
    }

    if (0 == strcmp(context->parser->tokens[4].buf, "exclude"))
    {
        enable = DISABLED;
    }
    else if (0 == strcmp(context->parser->tokens[4].buf, "include"))
    {
        enable = ENABLED;
    }
    else
    {
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_stat_tagLenCntIncEnable_set(unit, type, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif
