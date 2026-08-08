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
 * $Revision: 31120 $
 * $Date: 2012-07-18 17:47:25 +0800 (Wed, 18 Jul 2012) $
 *
 * Purpose : Definition those SEC command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) SEC enable/disable
 *           2) Parameter for SEC
 *
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <rtk/switch.h>
#include <rtk/port.h>
#include <rtk/sec.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


#ifdef CMD_SECURITY_GET_ARP_VALIDATION_PORT_ALL_ACTION
/*
 * security get arp-validation ( <PORT_LIST:port> | all ) action
 */
cparser_result_t cparser_cmd_security_get_arp_validation_port_all_action(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_action_t        action;
    diag_portlist_t     portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    
    diag_util_mprintf("ARP Validation Action Configuration\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPrevent_get(unit, port, ARP_INVALID, &action), ret);
        diag_util_mprintf("\tPort %2d : %s\n", port, (action == ACTION_DROP)? "Drop" : (action == ACTION_FORWARD)? "Forward" : "Trap-to-CPU");
    }
    
    return CPARSER_OK;
} /* end of cparser_cmd_security_get_arp_validation_port_all_action */
#endif

#ifdef CMD_SECURITY_GET_ATTACK_PREVENT
/*
 * security get attack-prevent
 */
cparser_result_t cparser_cmd_security_get_attack_prevent(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_sec_attackType_t    type = 0;
    rtk_action_t            action = 0;
    uint8  *actStr[] = { (uint8 *)"FORWARD", (uint8 *)"DROP", (uint8 *)"TRAP-TO-CPU" };
    uint8  *typeStr[] = {
        (uint8 *)"DAEQSA-DENY",
        (uint8 *)"LAND-DENY",
        (uint8 *)"UDPBLAT-DENY",
        (uint8 *)"TCPBLAT-DENY",
        (uint8 *)"POD-DENY",
        (uint8 *)"IPV6-MIN-FRAG-SIZE-CHECK",
        (uint8 *)"ICMP-FRAG-PKTS-DENY",
        (uint8 *)"ICMPV4-PING-MAX-CHECK",
        (uint8 *)"ICMPV6-PING-MAX-CHECK",
        (uint8 *)"SMURF-DENY",
        (uint8 *)"TCPHDR-MIN-CHECK",
        (uint8 *)"SYN-SPORTL1024-DENY",
        (uint8 *)"NULLSCAN-DENY",
        (uint8 *)"XMA-DENY",
        (uint8 *)"SYNFIN-DENY",
        (uint8 *)"SYNRST-DENY",
        (uint8 *)"TCP-FRAG-OFF-MIN-CHECK",
        };
    uint32  typeSym[] = {
        DAEQSA_DENY,
        LAND_DENY,
        UDPBLAT_DENY,
        TCPBLAT_DENY,
        POD_DENY,
        IPV6_MIN_FRAG_SIZE_CHECK,
        ICMP_FRAG_PKTS_DENY,
        ICMPV4_PING_MAX_CHECK,
        ICMPV6_PING_MAX_CHECK,
        SMURF_DENY,
        TCPHDR_MIN_CHECK,
        SYN_SPORTL1024_DENY,
        NULLSCAN_DENY,
        XMA_DENY,
        SYNFIN_DENY,
        SYNRST_DENY,
        TCP_FRAG_OFF_MIN_CHECK,
        };
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    for (type = 0; type < (sizeof(typeStr)/sizeof(uint8 *)); type++)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_attackPreventAction_get(unit, typeSym[type], &action), ret);
        diag_util_mprintf("\t%25s action : %s\n", typeStr[type], actStr[action]);
    }
    
    return CPARSER_OK;
} /* end of cparser_cmd_security_get_attack_prevent */
#endif

#ifdef CMD_SECURITY_GET_ATTACK_PREVENT_MAX_PING_MIN_IPV6_FRAG_MIN_TCP_HEADER_SMURF_NETMASK_PORT_ALL
/*
 * security get ( attack-prevent | max-ping | min-ipv6-frag | min-tcp-header | smurf-netmask ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_security_get_attack_prevent_max_ping_min_ipv6_frag_min_tcp_header_smurf_netmask_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    uint32              length = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_sec_attackType_t    type = 0;
    rtk_action_t        action = 0;
    diag_portlist_t     portlist;
    uint8  *actStr[2] = {(uint8 *)"FORWARD", (uint8 *)"DROP"};
    uint8  *typeStr[ATTACK_TYPE_END] = {(uint8 *)"SYNFIN-DENY",
                                        (uint8 *)"XMA-DENY",
                                        (uint8 *)"NULLSCAN-DENY",
                                        (uint8 *)"SYN-SPORTL1024-DENY", 
                                        (uint8 *)"TCPHDR-MIN-CHECK", 
                                        (uint8 *)"SMURF-DENY", 
                                        (uint8 *)"ICMPV6-PING-MAX-CHECK",
                                        (uint8 *)"ICMPV4-PING-MAX-CHECK",
                                        (uint8 *)"ICMP-FRAG-PKTS-DENY",
                                        (uint8 *)"IPV6-MIN-FRAG-SIZE-CHECK",
                                        (uint8 *)"POD-DENY", 
                                        (uint8 *)"TCPBLAT-DENY", 
                                        (uint8 *)"UDPBLAT-DENY", 
                                        (uint8 *)"LAND-DENY",
                                        (uint8 *)"DAEQSA-DENY",
                                        };

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('a' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_mprintf("Port %2d:\n", port);
            for (type = 0; type < ATTACK_TYPE_END; type++)
            {
                DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPrevent_get(unit, port, type, &action), ret);
                diag_util_mprintf("\t%25s action : %s\n", typeStr[type], actStr[action]);
            }
        }
    }
    else if ('m' == TOKEN_CHAR(2,0))
    {
        if ('a' == TOKEN_CHAR(2,1))
        {
            DIAG_UTIL_PORTMASK_SCAN(portlist, port)
            {
                DIAG_UTIL_ERR_CHK(rtk_sec_portMaxPingLen_get(unit, port, &length), ret);
                diag_util_mprintf("Port %2d Max Ping Length : %5d\n", port, length);
            }
        }
        else if ('i' == TOKEN_CHAR(2,4))
        {
            DIAG_UTIL_PORTMASK_SCAN(portlist, port)
            {
                DIAG_UTIL_ERR_CHK(rtk_sec_portMinIPv6FragLen_get(unit, port, &length), ret);
                diag_util_mprintf("Port %2d Min IPv6 Fragment Length : %5d\n", port, length);
            }
        }
        else
        {
            DIAG_UTIL_PORTMASK_SCAN(portlist, port)
            {
                DIAG_UTIL_ERR_CHK(rtk_sec_portMinTCPHdrLen_get(unit, port, &length), ret);
                diag_util_mprintf("Port %2d Min TCP Header Length : %3d\n", port, length);
            }
        }
    }
    else if ('s' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_portSmurfNetmaskLen_get(unit, port, &length), ret);
            diag_util_mprintf("Port %2d SMURF Netmask Length : %2d\n", port, length);
        }
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
} /* end of cparser_cmd_security_get_attack_prevent_max_ping_min_ipv6_frag_min_tcp_header_smurf_netmask_port_all */
#endif

#ifdef CMD_SECURITY_GET_ATTACK_PREVENT_PORT_ALL_STATE
/*
 * security get attack-prevent ( <PORT_LIST:port> | all ) state
 */
cparser_result_t cparser_cmd_security_get_attack_prevent_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_enable_t        enable = 0;
    diag_portlist_t     portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    
    diag_util_mprintf("Attack Prevention Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPreventEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("\tPort %2d : %s\n", port, (enable)? "ENABLE" : "DISABLE");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SECURITY_GET_MAX_PING_MIN_IPV6_FRAG_MIN_TCP_HEADER_SMURF_NETMASK
/*
 * security get ( max-ping | min-ipv6-frag | min-tcp-header | smurf-netmask )
 */
cparser_result_t cparser_cmd_security_get_max_ping_min_ipv6_frag_min_tcp_header_smurf_netmask(cparser_context_t *context)
{
    uint32              unit = 0;
    uint32              length = 0;
    int32               ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ('m' == TOKEN_CHAR(2,0))
    {
        if ('a' == TOKEN_CHAR(2,1))
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_maxPingLen_get(unit, &length), ret);
            diag_util_mprintf("\tMax Ping Length : %5d\n", length);
        }
        else if ('i' == TOKEN_CHAR(2,4))
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_minIPv6FragLen_get(unit, &length), ret);
            diag_util_mprintf("\tMin IPv6 Fragment Length : %5d\n", length);
        }
        else
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_minTCPHdrLen_get(unit, &length), ret);
            diag_util_mprintf("\tMin TCP Header Length : %3d\n", length);
        }
    }
    else if ('s' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_smurfNetmaskLen_get(unit, &length), ret);
        diag_util_mprintf("\tSMURF Netmask Length : %2d\n", length);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SECURITY_GET_GRATUITOUS_ARP_PORT_ALL_ACTION
/*
 * security get gratuitous-arp ( <PORT_LIST:port> | all ) action
 */
cparser_result_t cparser_cmd_security_get_gratuitous_arp_port_all_action(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_action_t        action;
    diag_portlist_t     portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    
    diag_util_mprintf("Gratuitous ARP Action Configuration\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPrevent_get(unit, port, GRATUITOUS_ARP, &action), ret);
        diag_util_mprintf("\tPort %2d : ", port);
        if (action == ACTION_DROP)
            diag_util_mprintf("Drop\n");
        else if (action == ACTION_FORWARD)
            diag_util_mprintf("Forward\n");
        else if (action == ACTION_TRAP2CPU)
            diag_util_mprintf("Trap to CPU\n");
    #if defined(CONFIG_SDK_RTL8380)            
        else if (action == ACTION_COPY2CPU)
            diag_util_mprintf("Copy to CPU\n");
    #endif
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SECURITY_SET_ARP_VALIDATION_PORT_ALL_ACTION_DROP_FORWARD_TRAP_TO_CPU
/*
 * security set arp-validation ( <PORT_LIST:port> | all ) action ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_security_set_arp_validation_port_all_action_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port;
    rtk_action_t        action;
    diag_portlist_t     portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    
    if ('d' == TOKEN_CHAR(5,0))
        action = ACTION_DROP;
    else if ('f' == TOKEN_CHAR(5,0))
        action = ACTION_FORWARD;
    else if ('t' == TOKEN_CHAR(5,0))
        action = ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPrevent_set(unit, port, ARP_INVALID, action), ret);
    }
    
    return CPARSER_OK;
} /* end of cparser_cmd_security_set_arp_validation_port_all_action_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_SECURITY_SET_ATTACK_PREVENT_DAEQSA_DENY_ICMP_FRAG_PKTS_DENY_ICMPV4_PING_MAX_CHECK_ICMPV6_PING_MAX_CHECK_IPV6_MIN_FRAG_SIZE_CHECK_LAND_DENY_NULLSCAN_DENY_POD_DENY_SMURF_DENY_SYN_SPORTL1024_DENY_SYNFIN_DENY_SYNRST_DENY_TCP_FRAG_OFF_MIN_CHECK_TCPBLAT_DENY_TCPHDR_MIN_CHECK_UDPBLAT_DENY_XMA_DENY_ACTION_DROP_FORWARD_TRAP_TO_CPU
/*
 * security set attack-prevent ( daeqsa-deny | icmp-frag-pkts-deny | icmpv4-ping-max-check | icmpv6-ping-max-check | ipv6-min-frag-size-check | land-deny | nullscan-deny | pod-deny | smurf-deny | syn-sportl1024-deny | synfin-deny | synrst-deny | tcp-frag-off-min-check | tcpblat-deny | tcphdr-min-check | udpblat-deny | xma-deny ) action ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_security_set_attack_prevent_daeqsa_deny_icmp_frag_pkts_deny_icmpv4_ping_max_check_icmpv6_ping_max_check_ipv6_min_frag_size_check_land_deny_nullscan_deny_pod_deny_smurf_deny_syn_sportl1024_deny_synfin_deny_synrst_deny_tcp_frag_off_min_check_tcpblat_deny_tcphdr_min_check_udpblat_deny_xma_deny_action_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_sec_attackType_t    type = 0;
    rtk_action_t        action = 0;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    if ('d' == TOKEN_CHAR(3,0))
    {
        type = DAEQSA_DENY;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        if ('p' == TOKEN_CHAR(3,1))
            type = IPV6_MIN_FRAG_SIZE_CHECK;
        else
        {
            if ('-' == TOKEN_CHAR(3,4))
                type = ICMP_FRAG_PKTS_DENY;
            else if ('4' == TOKEN_CHAR(3,5))
                type = ICMPV4_PING_MAX_CHECK;
            else
                type = ICMPV6_PING_MAX_CHECK;
        }
    }
    else if ('l' == TOKEN_CHAR(3,0))
    {
        type = LAND_DENY;
    }
    else if ('n' == TOKEN_CHAR(3,0))
    {
        type = NULLSCAN_DENY;
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        type = POD_DENY;
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        if ('m' == TOKEN_CHAR(3,1))
            type = SMURF_DENY;
        else if ('-' == TOKEN_CHAR(3,3))
            type = SYN_SPORTL1024_DENY;
        else if ('f' == TOKEN_CHAR(3,3))
            type = SYNFIN_DENY;
        else
            type = SYNRST_DENY;
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        if ('-' == TOKEN_CHAR(3,3))
            type = TCP_FRAG_OFF_MIN_CHECK;
        else if ('b' == TOKEN_CHAR(3,3))
            type = TCPBLAT_DENY;
        else
            type = TCPHDR_MIN_CHECK;
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        type = UDPBLAT_DENY;
    }
    else if ('x' == TOKEN_CHAR(3,0))
    {
        type = XMA_DENY;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    if ('d' == TOKEN_CHAR(5,0))
        action = ACTION_DROP;
    else if ('f' == TOKEN_CHAR(5,0))
        action = ACTION_FORWARD;
    else if ('t' == TOKEN_CHAR(5,0))
        action = ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_sec_attackPreventAction_set(unit, type, action), ret);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SECURITY_SET_ATTACK_PREVENT_PORT_ALL_DAEQSA_DENY_ICMP_FRAG_PKTS_DENY_ICMPV4_PING_MAX_CHECK_ICMPV6_PING_MAX_CHECK_IPV6_MIN_FRAG_SIZE_CHECK_LAND_DENY_NULLSCAN_DENY_POD_DENY_SMURF_DENY_SYN_SPORTL1024_DENY_SYNFIN_DENY_TCPBLAT_DENY_TCPHDR_MIN_CHECK_UDPBLAT_DENY_XMA_DENY_ACTION_DROP_FORWARD
/*
 * security set attack-prevent ( <PORT_LIST:port> | all ) ( daeqsa-deny | icmp-frag-pkts-deny | icmpv4-ping-max-check | icmpv6-ping-max-check | ipv6-min-frag-size-check | land-deny | nullscan-deny | pod-deny | smurf-deny | syn-sportl1024-deny | synfin-deny | tcpblat-deny | tcphdr-min-check | udpblat-deny | xma-deny ) action ( drop | forward )
 */
cparser_result_t cparser_cmd_security_set_attack_prevent_port_all_daeqsa_deny_icmp_frag_pkts_deny_icmpv4_ping_max_check_icmpv6_ping_max_check_ipv6_min_frag_size_check_land_deny_nullscan_deny_pod_deny_smurf_deny_syn_sportl1024_deny_synfin_deny_tcpblat_deny_tcphdr_min_check_udpblat_deny_xma_deny_action_drop_forward(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_sec_attackType_t    type = 0;
    rtk_action_t        action = 0;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(4,0))
    {
        type = DAEQSA_DENY;
    }
    else if ('i' == TOKEN_CHAR(4,0))
    {
        if ('p' == TOKEN_CHAR(4,1))
            type = IPV6_MIN_FRAG_SIZE_CHECK;
        else
        {
            if ('-' == TOKEN_CHAR(4,4))
                type = ICMP_FRAG_PKTS_DENY;
            else if ('4' == TOKEN_CHAR(4,5))
                type = ICMPV4_PING_MAX_CHECK;
            else
                type = ICMPV6_PING_MAX_CHECK;
        }
    }
    else if ('l' == TOKEN_CHAR(4,0))
    {
        type = LAND_DENY;
    }
    else if ('n' == TOKEN_CHAR(4,0))
    {
        type = NULLSCAN_DENY;
    }
    else if ('p' == TOKEN_CHAR(4,0))
    {
        type = POD_DENY;
    }
    else if ('s' == TOKEN_CHAR(4,0))
    {
        if ('m' == TOKEN_CHAR(4,1))
            type = SMURF_DENY;
        else if ('-' == TOKEN_CHAR(4,3))
            type = SYN_SPORTL1024_DENY;
        else
            type = SYNFIN_DENY;
    }
    else if ('t' == TOKEN_CHAR(4,0))
    {
        if ('b' == TOKEN_CHAR(4,3))
            type = TCPBLAT_DENY;
        else
            type = TCPHDR_MIN_CHECK;
    }
    else if ('u' == TOKEN_CHAR(4,0))
    {
        type = UDPBLAT_DENY;
    }
    else if ('x' == TOKEN_CHAR(4,0))
    {
        type = XMA_DENY;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('d' == TOKEN_CHAR(6,0))
        action = ACTION_DROP;
    else if ('f' == TOKEN_CHAR(6,0))
        action = ACTION_FORWARD;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPrevent_set(unit, port, type, action), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_security_set_attack_prevent_port_all_daeqsa_deny_icmp_frag_pkts_deny_icmpv4_ping_max_check_icmpv6_ping_max_check_ipv6_min_frag_size_check_land_deny_nullscan_deny_pod_deny_smurf_deny_syn_sportl1024_deny_synfin_deny_tcpblat_deny_tcphdr_min_check_udpblat_deny_xma_deny_action_drop_forward */
#endif

#ifdef CMD_SECURITY_SET_ATTACK_PREVENT_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * security set attack-prevent ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_security_set_attack_prevent_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_enable_t        enable = 0;
    rtk_port_t          port;
    diag_portlist_t     portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    
    if ('d' == TOKEN_CHAR(5,0))
        enable = DISABLED;
    else if ('e' == TOKEN_CHAR(5,0))
        enable = ENABLED;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPreventEnable_set(unit, port, enable), ret);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SECURITY_SET_MAX_PING_MIN_IPV6_FRAG_MIN_TCP_HEADER_SMURF_NETMASK_LENGTH
/*
 * security set ( max-ping | min-ipv6-frag | min-tcp-header | smurf-netmask ) <UINT:length>
 */
cparser_result_t cparser_cmd_security_set_max_ping_min_ipv6_frag_min_tcp_header_smurf_netmask_length(cparser_context_t *context,    uint32_t *length_ptr)
{
    uint32              unit = 0;
    uint32              length = 0;
    int32               ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    length = *length_ptr;

    if ('m' == TOKEN_CHAR(2,0))
    {
        if ('a' == TOKEN_CHAR(2,1))
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_maxPingLen_set(unit, length), ret);
        }
        else if ('i' == TOKEN_CHAR(2,4))
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_minIPv6FragLen_set(unit, length), ret);
        }
        else
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_minTCPHdrLen_set(unit, length), ret);
        }
    }
    else if ('s' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_smurfNetmaskLen_set(unit, length), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SECURITY_SET_MAX_PING_MIN_IPV6_FRAG_MIN_TCP_HEADER_SMURF_NETMASK_PORT_ALL_LENGTH
/*
 * security set ( max-ping | min-ipv6-frag | min-tcp-header | smurf-netmask ) ( <PORT_LIST:port> | all ) <UINT:length>
 */
cparser_result_t cparser_cmd_security_set_max_ping_min_ipv6_frag_min_tcp_header_smurf_netmask_port_all_length(cparser_context_t *context,
    char **port_ptr,
    uint32_t *length_ptr)
{
    uint32              unit = 0;
    uint32              length = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    length = *length_ptr;

    if ('m' == TOKEN_CHAR(2,0))
    {
        if ('a' == TOKEN_CHAR(2,1))
        {
            DIAG_UTIL_PORTMASK_SCAN(portlist, port)
            {
                DIAG_UTIL_ERR_CHK(rtk_sec_portMaxPingLen_set(unit, port, length), ret);
            }
        }
        else if ('i' == TOKEN_CHAR(2,4))
        {
            DIAG_UTIL_PORTMASK_SCAN(portlist, port)
            {
                DIAG_UTIL_ERR_CHK(rtk_sec_portMinIPv6FragLen_set(unit, port, length), ret);
            }
        }
        else
        {
            DIAG_UTIL_PORTMASK_SCAN(portlist, port)
            {
                DIAG_UTIL_ERR_CHK(rtk_sec_portMinTCPHdrLen_set(unit, port, length), ret);
            }
        }
    }
    else if ('s' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_sec_portSmurfNetmaskLen_set(unit, port, length), ret);
        }
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
} /* end of cparser_cmd_security_set_max_ping_min_ipv6_frag_min_tcp_header_smurf_netmask_port_all_length */
#endif

#ifdef CMD_SECURITY_SET_GRATUITOUS_ARP_PORT_ALL_ACTION_DROP_FORWARD_TRAP_TO_CPU
/*
 * security set gratuitous-arp ( <PORT_LIST:port> | all ) action ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_security_set_gratuitous_arp_port_all_action_drop_forward_trap_to_cpu(
    cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port;
    rtk_action_t        action;
    diag_portlist_t     portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    
    if ('d' == TOKEN_CHAR(5,0))
        action = ACTION_DROP;
    else if ('f' == TOKEN_CHAR(5,0))
        action = ACTION_FORWARD;
    else if ('t' == TOKEN_CHAR(5,0))
        action = ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPrevent_set(unit, port, GRATUITOUS_ARP, action), ret);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SECURITY_SET_GRATUITOUS_ARP_PORT_ALL_ACTION_DROP_FORWARD_TRAP_TO_CPU
/*
 * security set gratuitous-arp ( <PORT_LIST:port> | all ) action copy-to-cpu
 */
cparser_result_t cparser_cmd_security_set_gratuitous_arp_port_all_action_copy_to_cpu(
    cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port;
    diag_portlist_t     portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
       
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_sec_portAttackPrevent_set(unit, port, GRATUITOUS_ARP, ACTION_COPY2CPU), ret);
    }
    
    return CPARSER_OK;
}
#endif

