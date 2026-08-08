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
 * $Revision: 30154 $
 * $Date: 2012-06-22 14:03:52 +0800 (Fri, 22 Jun 2012) $
 *
 * Purpose : Definition those trap command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) trap
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
#include <rtk/trap.h>
#include <diag_util.h>
#include <diag_str.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#if defined(CONFIG_SDK_RTL8328)
static char * l34_protocol_str[]  =
{
    "RIP",
    "ICMP",
    "ICMPV6",
    "ARP",
    "MLD",
    "IGMP",
    "BGP",
    "OSPFV2",
    "OSPFV3",
    "SNMP",
    "SSH",
    "FTP",
    "TFTP",
    "TELNET",
    "HTTP",
    "HTTPS",
    "DHCPV6",
    "DHCP",
    "DOT1X",
    "BPDU",
};
#endif

#ifdef CMD_TRAP_DUMP_CFM
/*
 * trap dump cfm
 */
cparser_result_t cparser_cmd_trap_dump_cfm(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          mdlevel = 0;
    rtk_pri_t       pri = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_action_t    action = ACTION_DROP;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameTrapPriEnable_get(unit, &enable), ret);
    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameTrapPri_get(unit, &pri), ret);
    diag_util_mprintf("CFM Trap Priority       : %s with priority %u\n", (ENABLED == enable)?"enable":"disable", pri);

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameTrapAddCPUTagEnable_get(unit, &enable), ret);
    if (ENABLED == enable)
        diag_util_mprintf("CFM Trap Insert cpu tag : enable\n");
    else
        diag_util_mprintf("CFM Trap Insert cpu tag : disable\n");

    for (mdlevel = 0; mdlevel <= 7; mdlevel++)
    {
        diag_util_mprintf("cfm md-level            : %u   ", mdlevel);
        DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameAction_get(unit, mdlevel, &action), ret);
        if (ACTION_FORWARD == action)
        {
            diag_util_mprintf(" forward    \n");
        }
        else if (ACTION_DROP == action)
        {
            diag_util_mprintf(" drop        \n");
        }
        else if (ACTION_TRAP2CPU == action)
        {
            diag_util_mprintf(" trap-to-cpu \n");
        }
        else if (ACTION_COPY2CPU == action)
        {
            diag_util_mprintf(" copy-to-cpu \n");
        }
        else
        {
            diag_util_printf("User config : Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_cfm */
#endif

#ifdef CMD_TRAP_GET_CFM_PRIORITY
/*
 * trap get cfm priority
 */
cparser_result_t
cparser_cmd_trap_get_cfm_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameTrapPri_get(unit, &pri), ret);
    diag_util_mprintf("CFM Trap Priority       : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_cfm_priority */
#endif

#ifdef CMD_TRAP_DUMP_OAMPDU_OPTION_TYPE_PORT_ALL
/*
 * trap dump ( oampdu | option-type ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_trap_dump_oampdu_option_type_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_pri_t       pri = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_action_t    action = ACTION_DROP;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('a' == TOKEN_CHAR(2,1))
    {
        diag_util_mprintf("  port    | Action(oampdu) |trap-priority(oampdu) | insert-cpu-tag(oampdu)\n");
        diag_util_mprintf("----------+----------------+----------------------+-----------------\n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_printf("   %2u     ", port);
            DIAG_UTIL_ERR_CHK(rtk_trap_portOamPDUAction_get(unit, port, &action), ret);
            if (ACTION_FORWARD == action)
            {
                diag_util_printf(" forward          ");
            }
            else if (ACTION_DROP == action)
            {
                diag_util_printf(" drop             ");
            }
            else if (ACTION_TRAP2CPU == action)
            {
                diag_util_printf(" trap-to-cpu      ");
            }
            else if (ACTION_COPY2CPU == action)
            {
                diag_util_printf(" copy-to-cpu   ");
            }
            else
            {
                diag_util_printf("User config: Error!");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUPriEnable_get(unit, port, &enable), ret);
            DIAG_UTIL_ERR_CHK(rtk_trap_portOamPDUPri_get(unit, port, &pri), ret);
            diag_util_printf("   %s ( %u )    ", (ENABLED == enable)?"enable ":"disable", pri);

            DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUTrapAddCPUTagEnable_get(unit, port, &enable), ret);
            if (ENABLED == enable)
                diag_util_mprintf("   enable\n");
            else
                diag_util_mprintf("   disable\n");

        }
    }
    else
    {
        diag_util_mprintf(" port | Action(ipv4/ipv6/cfi)      | trap-priority(optional/cfi)     | insert-cpu-tag(optional/cfi)\n");
        diag_util_mprintf("------+----------------------------+---------------------------------+-----------------------------\n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_printf("  %2u  |", port);
            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderAction_get(unit, port, IPV4_FAMILY, &action), ret);
            if (ACTION_FORWARD == action)
            {
                diag_util_printf(" forward/");
            }
            else if (ACTION_DROP == action)
            {
                diag_util_printf(" drop/");
            }
            else if (ACTION_TRAP2CPU == action)
            {
                diag_util_printf(" trap-to-cpu/");
            }
            else if (ACTION_COPY2CPU == action)
            {
                diag_util_printf(" copy-to-cpu/");
            }
            else
            {
                diag_util_printf("User config: Error!");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderAction_get(unit, port, IPV6_FAMILY, &action), ret);
            if (ACTION_FORWARD == action)
            {
                diag_util_printf("forward/");
            }
            else if (ACTION_DROP == action)
            {
                diag_util_printf("drop/ ");
            }
            else if (ACTION_TRAP2CPU == action)
            {
                diag_util_printf("trap-to-cpu/");
            }
            else if (ACTION_COPY2CPU == action)
            {
                diag_util_printf("copy-to-cpu/");
            }
            else
            {
                diag_util_printf("User config: Error!");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_trap_portPktWithCFIAction_get(unit, port, &action), ret);
            if (ACTION_FORWARD == action)
            {
                diag_util_printf("forward   ");
            }
            else if (ACTION_DROP == action)
            {
                diag_util_printf("drop   ");
            }
            else if (ACTION_TRAP2CPU == action)
            {
                diag_util_printf("trap-to-cpu   ");
            }
            else if (ACTION_COPY2CPU == action)
            {
                diag_util_printf("copy-to-cpu   ");
            }
            else
            {
                diag_util_printf("User config: Error!");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderPriEnable_get(unit, port, &enable), ret);
            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderPri_get(unit, port, &pri), ret);
            diag_util_printf("%s ( %u )/", (ENABLED == enable)?"enable ":"disable", pri);

            DIAG_UTIL_ERR_CHK(rtk_trap_pktWithCFIPriEnable_get(unit, port, &enable), ret);
            DIAG_UTIL_ERR_CHK(rtk_trap_portPktWithCFIPri_get(unit, port, &pri), ret);
            diag_util_printf("%s ( %u ) ", (ENABLED == enable)?"enable ":"disable", pri);

            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderAddCPUTagEnable_get(unit, port, &enable), ret);
            if(ENABLED == enable)
                diag_util_printf("   enable/");
            else
                diag_util_printf("   disable/");

            DIAG_UTIL_ERR_CHK(rtk_trap_pktWithCFIAddCPUTagEnable_get(unit, port, &enable), ret);
            if(ENABLED == enable)
                diag_util_mprintf("enable\n");
            else
                diag_util_mprintf("disable\n");
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_oampdu_option_type_port_all */
#endif

#ifdef CMD_TRAP_GET_OAMPDU_INFO
/*
 * trap get oampdu info
 */
cparser_result_t
cparser_cmd_trap_get_oampdu_info(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;
    rtk_pri_t       pri = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUAction_get(unit, &action), ret);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("OAM PDU:\n");
    diag_util_mprintf("\tAction: ");
    if (ACTION_FORWARD == action)
        diag_util_mprintf("Forward");
    else if (ACTION_DROP == action)
        diag_util_mprintf("Drop");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("Trap");
    else
        diag_util_mprintf("%d", action);
    diag_util_mprintf("\n");

    DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUPri_get(unit, &pri), ret);
    diag_util_mprintf("\tPriority: %d\n", pri);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_DUMP_PACKET_REASON
/*
 * trap dump ( packet | reason )
 */
cparser_result_t cparser_cmd_trap_dump_packet_reason(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   priority = 0;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('p' == TOKEN_CHAR(2,0))
    {
        diag_util_mprintf("Packet type              | Trap to CPU \n");
        diag_util_mprintf("-------------------------+-------------\n");
        DIAG_UTIL_ERR_CHK(rtk_trap_pkt2CpuEnable_get(unit, TRAP_TYPE_1XMAC_PORTCHG, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("802.1X mac port change    enable\n");
        }
        else
        {
            diag_util_mprintf("802.1X mac port change    disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_pkt2CpuEnable_get(unit, TRAP_TYPE_CFI_1, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("CFI                       enable\n");
        }
        else
        {
            diag_util_mprintf("CFI                       disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_pkt2CpuEnable_get(unit, TRAP_TYPE_IPMC_DLF, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("IP mcast dst lookup fail  enable\n");
        }
        else
        {
            diag_util_mprintf("IP mcast dst lookup fail  disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_pkt2CpuEnable_get(unit, TRAP_TYPE_IPV4_IGMP, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("IPv4 IGMP                 enable\n");
        }
        else
        {
            diag_util_mprintf("IPv4 IGMP                 disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_pkt2CpuEnable_get(unit, TRAP_TYPE_IPV6_MLD, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("IPv6 MLD                  enable\n");
        }
        else
        {
            diag_util_mprintf("IPv6 MLD                  disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_pkt2CpuEnable_get(unit, TRAP_TYPE_L2MC_DLF, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("L2 mcast dst lookup fail  enable\n");
        }
        else
        {
            diag_util_mprintf("L2 mcast dst lookup fail  disable\n");
        }
    }
    else
    {
        diag_util_mprintf("Reason              | Priority  \n");
        diag_util_mprintf("--------------------+-----------\n");
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_1XEAPOL, &priority), ret);
        diag_util_mprintf("802.1X EAPOL packet    %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_1XUNAUTH, &priority), ret);
        diag_util_mprintf("Unauth 802.1X packet   %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_CFI, &priority), ret);
        diag_util_mprintf("CFI packet             %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_IPV4IGMP, &priority), ret);
        diag_util_mprintf("IPv4 IGMP packet       %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_IPV6MLD, &priority), ret);
        diag_util_mprintf("IPv6-MLD packet        %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_MULTICASTDLF, &priority), ret);
        diag_util_mprintf("Multicast DLF packet   %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_RMA, &priority), ret);
        diag_util_mprintf("RMA packet             %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_SLPCHANGE, &priority), ret);
        diag_util_mprintf("SLP change packet      %d\n", priority);
        DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_get(unit, TRAP_REASON_VLANERR, &priority), ret);
        diag_util_mprintf("VLAN error packet      %d\n", priority);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_packet_reason */
#endif

#ifdef CMD_TRAP_DUMP_RMA_L2_USER_DEFINE_L34_PROTOCOL_L34_USER_PROTOCOL_LAYER2_MGMT_IP_CHECK
/*
 * trap dump rma ( l2-user-define | l34-protocol | l34-user-protocol | layer2 | mgmt-ip-check )
 */
cparser_result_t cparser_cmd_trap_dump_rma_l2_user_define_l34_protocol_l34_user_protocol_layer2_mgmt_ip_check(cparser_context_t *context)
{
    uint32                  unit = 0;
    uint32                  rma_index = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_action_t            rtk_action = ACTION_FORWARD;
    rtk_trap_mgmtType_t     type = MGMT_TYPE_RIP;
    rtk_trap_userDefinedRma_t   userDefinedRma;
    rtk_trap_userDefinedMgmt_t  userDefine;
    rtk_pri_t               pri = 0;
    rtk_enable_t            enable;
    rtk_mac_t               rma_frame;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('m' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtIpCheck_get(unit, MGMT_IP_TYPE_IPV4, &enable), ret);
        if (ENABLED == enable)
            diag_util_printf("Management IPv4 Address Check: Enabled\n");
        else
            diag_util_printf("Management IPv4 Address Check: Disabled\n");
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtIpCheck_get(unit, MGMT_IP_TYPE_IPV6, &enable), ret);
        if (ENABLED == enable)
            diag_util_printf("Management IPv6 Address Check: Enabled\n");
        else
            diag_util_printf("Management IPv6 Address Check: Disabled\n");
    }
    else if ('2' == TOKEN_CHAR(3,1))
    {
        diag_util_mprintf("  Index  | Mac address      | Mac Address Mask  | Action     |trap-priority | vlan-care  | Stp Block\n");
        diag_util_mprintf("---------+------------------+-------------------+------------+--------------+------------+---------------\n");

        for (rma_index = 0; rma_index <= 3; rma_index++)
        {
            memset(&userDefinedRma, 0, sizeof(rtk_trap_userDefinedRma_t));
            DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRma_get(unit, rma_index, &userDefinedRma), ret);
            diag_util_printf("  %u  ", rma_index);
            diag_util_printf("    %02X:%02X:%02X:%02X:%02X:%02X ",userDefinedRma.mac.octet[0],userDefinedRma.mac.octet[1],
                userDefinedRma.mac.octet[2], userDefinedRma.mac.octet[3],userDefinedRma.mac.octet[4],userDefinedRma.mac.octet[5]);
            diag_util_printf("   %02X:%02X:%02X:%02X:%02X:%02X  ",userDefinedRma.macMask.octet[0],userDefinedRma.macMask.octet[1],
                userDefinedRma.macMask.octet[2], userDefinedRma.macMask.octet[3],userDefinedRma.macMask.octet[4],
                userDefinedRma.macMask.octet[5]);

            DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaAction_get(unit, rma_index, &rma_action), ret);
            if (RMA_ACTION_FORWARD == rma_action)
            {
                diag_util_printf(" forward     ");
            }
            else if (RMA_ACTION_DROP == rma_action)
            {
                diag_util_printf(" drop        ");
            }
            else if (RMA_ACTION_TRAP2CPU == rma_action)
            {
                diag_util_printf(" trap-to-cpu ");
            }
            else if (RMA_ACTION_COPY2CPU == rma_action)
            {
                diag_util_printf(" copy-to-cpu ");
            }
            else
            {
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaPriEnable_get(unit, rma_index, &enable), ret);
            DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaPri_get(unit, rma_index, &pri), ret);
            diag_util_printf(" %s (%u) ", (ENABLED == enable)?"enable ":"disable", pri);

            DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaVlanCheckEnable_get(unit, rma_index, &enable), ret);
            if(ENABLED == enable)
                diag_util_printf(" enabled     ");
            else if(DISABLED == enable)
                diag_util_printf(" disabled    ");
            else
                diag_util_printf(" disabled    ");

            DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaStpBlockEnable_get(unit, rma_index, &enable), ret);
            if(ENABLED == enable)
                diag_util_printf(" enabled\n");
            else if(DISABLED == enable)
                diag_util_printf(" disabled\n");
            else
                diag_util_printf(" disabled\n");
        }
    }
    else if ('a' == TOKEN_CHAR(3,1))
    {
        rma_frame.octet[0] = 0x01;
        rma_frame.octet[1] = 0x80;
        rma_frame.octet[2] = 0xc2;
        rma_frame.octet[3] = 0x00;
        rma_frame.octet[4] = 0x00;
        rma_frame.octet[5] = 0x00;

        diag_util_mprintf("Multicast Address frame | Action      | trap-priority | vlan-care\n");
        diag_util_mprintf("------------------------+-------------+---------------+-----------\n");
        for (rma_index = 0; rma_index <= 0x2f; rma_index++)
        {
            rma_frame.octet[5] = rma_index;
            ret = rtk_trap_rmaAction_get(unit, &rma_frame, &rma_action);
            if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
                continue;

            if (ret != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            diag_util_printf("   %02X:%02X:%02X:%02X:%02X:%02X     ",rma_frame.octet[0],rma_frame.octet[1],rma_frame.octet[2],
                                                           rma_frame.octet[3],rma_frame.octet[4],rma_frame.octet[5]);
            if (RMA_ACTION_FORWARD == rma_action)
            {
                diag_util_printf(" forward     ");
            }
            else if (RMA_ACTION_DROP == rma_action)
            {
                diag_util_printf(" drop        ");
            }
            else if (RMA_ACTION_TRAP2CPU == rma_action)
            {
                diag_util_printf(" trap-to-cpu ");
            }
            else if (RMA_ACTION_COPY2CPU == rma_action)
            {
                diag_util_printf(" copy-to-cpu ");
            }
            else
            {
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_trap_rmaPriEnable_get(unit, &rma_frame, &enable), ret);
            DIAG_UTIL_ERR_CHK(rtk_trap_rmaPri_get(unit, &rma_frame, &pri), ret);
            diag_util_printf(" %s ( %u )  ", (ENABLED == enable)?"enable ":"disable", pri);

            DIAG_UTIL_ERR_CHK(rtk_trap_rmaVlanCheckEnable_get(unit, &rma_frame, &enable), ret);
            if(ENABLED == enable)
                diag_util_printf(" enabled\n");
            else if(DISABLED == enable)
                diag_util_printf(" disabled\n");
            else
                diag_util_printf(" disabled\n");
        }
    }
    else if ('3' == TOKEN_CHAR(3,1))
    {
        if ('p' == TOKEN_CHAR(3,4))
        {
            diag_util_mprintf("L34 protocol type | Action     | trap-priority | vlan-care\n");
            diag_util_mprintf("------------------+------------+---------------+-----------\n");
            for (type = MGMT_TYPE_RIP; type < MGMT_TYPE_DHCPV6; type++)
            {
                diag_util_printf("   %8s       ", l34_protocol_str[type]);
                DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_get(unit, type, &rtk_action), ret);
                if (RMA_ACTION_FORWARD == rtk_action)
                {
                    diag_util_printf("  forward     ");
                }
                else if (RMA_ACTION_DROP == rtk_action)
                {
                    diag_util_printf("  drop        ");
                }
                else if (RMA_ACTION_TRAP2CPU == rtk_action)
                {
                    diag_util_printf("  trap-to-cpu ");
                }
                else if (RMA_ACTION_COPY2CPU == rtk_action)
                {
                    diag_util_printf("  copy-to-cpu ");
                }
                else
                {
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }

                DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePriEnable_get(unit, type, &enable), ret);
                DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, type, &pri), ret);
                diag_util_printf(" %s (%u) ", (ENABLED == enable)?"enable ":"disable", pri);

                DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameVlanCheck_get(unit, type, &enable), ret);
                if(ENABLED == enable)
                    diag_util_printf("   enabled \n");
                else if(DISABLED == enable)
                    diag_util_printf("   disabled\n");
                else
                    diag_util_printf("   disabled\n");
            }
        }
        else
        {
            diag_util_mprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

            for (rma_index = 0; rma_index < 2; rma_index++)
            {
                diag_util_mprintf("rma_index : %u\n", rma_index);
                memset(&userDefine, 0, sizeof(rtk_trap_userDefinedMgmt_t));
                DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmt_get(unit, rma_index, &userDefine), ret);
                if(L4PROTO_TCP == userDefine.layer4Proto)
                    diag_util_mprintf("user defined layer4 protocol : TCP\n");
                else
                    diag_util_mprintf("user defined layer4 protocol : UDP\n");

                if(userDefine.dmacCheckEnable)
                    diag_util_mprintf("user defined protocol compare DMAC : enable\n");
                else
                    diag_util_mprintf("user defined protocol compare DMAC : disable\n");

                if(userDefine.dipCheckEnable)
                    diag_util_mprintf("user defined protocol compare DIP  : enable\n");
                else
                    diag_util_mprintf("user defined protocol compare DIP  : disable\n");

                if(userDefine.srcL4PortCheck)
                    diag_util_mprintf("user defined protocol compare src Layer4 Port : enable, port(0x%x), portMask(0x%x)\n",
                                    userDefine.srcL4Port,userDefine.mask_of_srcL4Port);
                else
                    diag_util_mprintf("user defined protocol compare src Layer4 Port : disable\n");

                if(userDefine.dstL4PortCheck)
                    diag_util_mprintf("user defined protocol compare dst Layer4 Port : enable, port(0x%x), portMask(0x%x)\n",
                                    userDefine.dstL4Port,userDefine.mask_of_dstL4Port);
                else
                    diag_util_mprintf("user defined protocol compare dst Layer4 Port : disable\n");

                DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtAction_get(unit, rma_index, &rtk_action), ret);
                if (RMA_ACTION_FORWARD == rtk_action)
                {
                    diag_util_mprintf("Action        : forward\n");
                }
                else if (RMA_ACTION_DROP == rtk_action)
                {
                    diag_util_mprintf("Action        : drop\n");
                }
                else if (RMA_ACTION_TRAP2CPU == rtk_action)
                {
                    diag_util_mprintf("Action        : trap-to-cpu\n");
                }
                else if (RMA_ACTION_COPY2CPU == rtk_action)
                {
                    diag_util_mprintf("Action        : copy-to-cpu\n");
                }
                else
                {
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }

                DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtPriEnable_get(unit, rma_index, &enable), ret);
                DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtPri_get(unit, rma_index, &pri), ret);
                diag_util_mprintf("Trap priority : %s with priority %u\n", (ENABLED == enable)?"enable":"disable", pri);

                DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtVlanCheck_get(unit, rma_index, &enable), ret);
                if(ENABLED == enable)
                    diag_util_mprintf("Vlan Check    : enabled\n");
                else if(DISABLED == enable)
                    diag_util_mprintf("Vlan Check    : disabled\n");
                else
                    diag_util_mprintf("Vlan Check    : disabled\n");

                diag_util_mprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            }
        }
    }
    else
    {
        diag_util_printf("User config: Error!");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_rma_l2_user_define_l34_protocol_l34_user_protocol_layer2 */
#endif

#ifdef CMD_TRAP_DUMP_RMA_L2_USER_DEFINE
/*
 * trap dump rma l2-user-define
 */
cparser_result_t
cparser_cmd_trap_dump_rma_l2_user_define(cparser_context_t *context)
{
    uint32                  unit = 0;
    uint32                  rma_index = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_trap_userDefinedRma_t   userDefinedRma;
    rtk_enable_t            enable;
    uint32                  bypassStpFieldIdx[] = {BYPASS_STP_TYPE_USER_DEF_0,
                                                   BYPASS_STP_TYPE_USER_DEF_1};

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("  Index | Mac address      | Mac Address Mask  | Action     | Learn     | Stp Block\n");
    diag_util_mprintf("--------+------------------+-------------------+------------+-----------+---------------\n");

    for (rma_index = 0; rma_index < 2; rma_index++)
    {
        memset(&userDefinedRma, 0, sizeof(rtk_trap_userDefinedRma_t));
        DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRma_get(unit, rma_index, &userDefinedRma), ret);
        diag_util_printf("  %u  ", rma_index);
        diag_util_printf("    %02X:%02X:%02X:%02X:%02X:%02X ",userDefinedRma.mac.octet[0],userDefinedRma.mac.octet[1],
            userDefinedRma.mac.octet[2], userDefinedRma.mac.octet[3],userDefinedRma.mac.octet[4],userDefinedRma.mac.octet[5]);
        diag_util_printf("   %02X:%02X:%02X:%02X:%02X:%02X  ",userDefinedRma.macMask.octet[0],userDefinedRma.macMask.octet[1],
            userDefinedRma.macMask.octet[2], userDefinedRma.macMask.octet[3],userDefinedRma.macMask.octet[4],
            userDefinedRma.macMask.octet[5]);

        DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaAction_get(unit, rma_index, &rma_action), ret);
        if (RMA_ACTION_FORWARD == rma_action)
        {
            diag_util_printf(" forward     ");
        }
        else if (RMA_ACTION_DROP == rma_action)
        {
            diag_util_printf(" drop        ");
        }
        else if (RMA_ACTION_TRAP2CPU == rma_action)
        {
            diag_util_printf(" trap-to-cpu ");
        }
        else if (RMA_ACTION_COPY2CPU == rma_action)
        {
            diag_util_printf(" copy-to-cpu ");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaLearningEnable_get(unit,
                rma_index, &enable), ret);
        if(ENABLED == enable)
            diag_util_printf(" enabled ");
        else if(DISABLED == enable)
            diag_util_printf(" disabled ");
        else
            diag_util_printf(" disabled ");

        DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_get(unit,
                bypassStpFieldIdx[rma_index], &enable), ret);
        if(ENABLED == enable)
            diag_util_printf(" enabled\n");
        else if(DISABLED == enable)
            diag_util_printf(" disabled\n");
        else
            diag_util_printf(" disabled\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_rma_l2_user_define */
#endif

#ifdef CMD_TRAP_DUMP_RMA_LAYER2
/*
 * trap dump rma layer2
 */
cparser_result_t
cparser_cmd_trap_dump_rma_layer2(cparser_context_t *context)
{
    uint32                  unit = 0;
    uint32                  rma_index = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_enable_t            enable;
    rtk_mac_t               rma_frame;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    rma_frame.octet[0] = 0x01;
    rma_frame.octet[1] = 0x80;
    rma_frame.octet[2] = 0xc2;
    rma_frame.octet[3] = 0x00;
    rma_frame.octet[4] = 0x00;

    diag_util_mprintf("Multicast Address frame | Action      | Learn\n");
    diag_util_mprintf("------------------------+-------------+-----------\n");
    for (rma_index = 0; rma_index <= 0x2f; rma_index++)
    {
        rma_frame.octet[5] = rma_index;
        ret = rtk_trap_rmaAction_get(unit, &rma_frame, &rma_action);
        if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            continue;

        if (ret != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_printf("   %02X:%02X:%02X:%02X:%02X:%02X     ",rma_frame.octet[0],rma_frame.octet[1],rma_frame.octet[2],
                                                       rma_frame.octet[3],rma_frame.octet[4],rma_frame.octet[5]);
        if (RMA_ACTION_FORWARD == rma_action)
        {
            diag_util_printf(" forward     ");
        }
        else if (RMA_ACTION_DROP == rma_action)
        {
            diag_util_printf(" drop        ");
        }
        else if (RMA_ACTION_TRAP2CPU == rma_action)
        {
            diag_util_printf(" trap-to-cpu ");
        }
        else if (RMA_ACTION_COPY2CPU == rma_action)
        {
            diag_util_printf(" copy-to-cpu ");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_rmaLearningEnable_get(unit, &rma_frame,
                &enable), ret);
        if(ENABLED == enable)
            diag_util_printf(" enabled\n");
        else if(DISABLED == enable)
            diag_util_printf(" disabled\n");
        else
            diag_util_printf(" disabled\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_rma_layer2 */
#endif

#ifdef CMD_TRAP_DUMP_RMA_PORT_RMA_PORT_ALL
/*
 * trap dump rma port-rma ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_trap_dump_rma_port_rma_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    rtk_trap_mgmtType_t type = MGMT_TYPE_RIP;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        rma_action = ACTION_FORWARD;
    rtk_pri_t           pri = 0;
    rtk_enable_t        enable;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("----------------------------------Port : %2u-------------------------------\n", port);
        diag_util_mprintf("L34 protocol type| Action      | trap-priority | vlan-care  | vlan-cross\n");
        diag_util_mprintf("-----------------+-------------+---------------+------------+-------------\n");
        for (type = MGMT_TYPE_DHCPV6; type <= MGMT_TYPE_BPDU; type++)
        {
            diag_util_printf("   %8s      ", l34_protocol_str[type]);
            DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_get(unit, port,  type, &rma_action), ret);
            if (RMA_ACTION_FORWARD == rma_action)
            {
                diag_util_printf("  forward      ");
            }
            else if (RMA_ACTION_DROP == rma_action)
            {
                diag_util_printf("  drop         ");
            }
            else if (RMA_ACTION_TRAP2CPU == rma_action)
            {
                diag_util_printf("  trap-to-cpu  ");
            }
            else if (RMA_ACTION_COPY2CPU == rma_action)
            {
                diag_util_printf("  copy-to-cpu  ");
            }
            else
            {
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }

            DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFramePriEnable_get(unit, port, type, &enable), ret);
            DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFramePri_get(unit, port, type, &pri), ret);
            diag_util_printf(" %s (%u)  ", (ENABLED == enable)?"enable ":"disable", pri);

            DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameVlanCheck_get(unit, port, type, &enable), ret);
            if(ENABLED == enable)
                diag_util_printf("  enabled    ");
            else if(DISABLED == enable)
                diag_util_printf("  disabled   ");
            else
                diag_util_printf("  disabled   ");

            DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameCrossVlan_get(unit, port, type, &enable), ret);
            if(ENABLED == enable)
                diag_util_printf("  enabled\n");
            else if(DISABLED == enable)
                diag_util_printf("  disabled\n");
            else
                diag_util_printf("  disabled\n");
        }

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_rma_port_rma_port_all */
#endif

#ifdef CMD_TRAP_GET_EAPOL
/*
 * trap dump eapol
 */
cparser_result_t cparser_cmd_trap_get_eapol(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_get(unit, MGMT_TYPE_EAPOL, &action), ret);
    diag_util_printf("EAPOL Action :");
    if (RMA_ACTION_FORWARD == action)
    {
        diag_util_printf(" forward      \n");
    }
    else if (RMA_ACTION_TRAP2CPU == action)
    {
        diag_util_printf(" trap-to-cpu  \n");
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_get(unit, BYPASS_STP_TYPE_EAPOL, &enable), ret);
    diag_util_printf("EAPOL Bypass STP Status : %s \n", (ENABLED == enable) ? "Enabled " : "Disabled");

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_GET_ARP_REQUEST
/*
 * trap dump arp-request
 */
cparser_result_t cparser_cmd_trap_get_arp_request(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_get(unit, MGMT_TYPE_ARP, &action), ret);

    diag_util_printf("ARP Request Action :");
    if (RMA_ACTION_FORWARD == action)
    {
        diag_util_printf(" forward      \n");
    }
    else if (RMA_ACTION_TRAP2CPU == action)
    {
        diag_util_printf(" trap-to-cpu  \n");
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_GET_NEIGHBOR_DISCOVERY
/*
 * trap get neighbor-discovery
 */
cparser_result_t cparser_cmd_trap_get_neighbor_discovery(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_get(unit, MGMT_TYPE_IPV6ND, &action), ret);

    diag_util_printf("IPv6 Neighbor Discovery Action :");
    if (RMA_ACTION_FORWARD == action)
    {
        diag_util_printf(" forward      \n");
    }
    else if (RMA_ACTION_TRAP2CPU == action)
    {
        diag_util_printf(" trap-to-cpu  \n");
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_GET_REASON_1X_EAPOL_PRIORITY
/*
 * trap get reason 1x-eapol priority
 */
cparser_result_t cparser_cmd_trap_get_reason_1x_eapol_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_EAPOL, &pri), ret);
    diag_util_mprintf("dot1x EAPOL Trap Priority : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_1x_eapol_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_ARP_IPV6_ND_PRIORITY
/*
 * trap get reason ( arp | ipv6-nd ) priority
 */
cparser_result_t cparser_cmd_trap_get_reason_arp_ipv6_nd_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('a' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_ARP, &pri), ret);
        diag_util_mprintf("ARP Trap Priority : %u\n", pri);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_IPV6ND, &pri), ret);
        diag_util_mprintf("IPv6 Neighbor Discovery Trap Priority : %u\n", pri);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_arp_ipv6_nd_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_BPDU_LACP_LLDP_PRIORITY
/*
 * trap get reason ( bpdu | lacp | lldp ) priority
 */
cparser_result_t cparser_cmd_trap_get_reason_bpdu_lacp_lldp_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('b' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_BPDU, &pri), ret);
        diag_util_mprintf("BPDU Trap Priority : %u\n", pri);
    }
    else if ('a' == TOKEN_CHAR(3,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_LACP, &pri), ret);
        diag_util_mprintf("LACP Trap Priority : %u\n", pri);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_LLDP, &pri), ret);
        diag_util_mprintf("LLDP Trap Priority : %u\n", pri);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_bpdu_lacp_lldp_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_INGRESS_VLAN_FILTER_PRIORITY
/*
 * trap get reason ingress-vlan-filter priority
 */
cparser_result_t cparser_cmd_trap_get_reason_ingress_vlan_filter_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_IGR_VLAN_FLTR, &pri), ret);
    diag_util_mprintf("Ingress-VLAN-Filter Trap Priority : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_ingress_vlan_filter_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_IPV4_IGMP_IPV6_MLD_PRIORITY
/*
 * trap get reason ( ipv4-igmp | ipv6-mld ) priority
 */
cparser_result_t cparser_cmd_trap_get_reason_ipv4_igmp_ipv6_mld_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('4' == TOKEN_CHAR(3,3))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_IGMP, &pri), ret);
        diag_util_mprintf("IPv4-IGMP Trap Priority : %u\n", pri);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_MLD, &pri), ret);
        diag_util_mprintf("IPv6-MLD Trap Priority : %u\n", pri);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_ipv4_igmp_ipv6_mld_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_OTHER_PRIORITY
/*
 * trap get reason other priority
 */
cparser_result_t cparser_cmd_trap_get_reason_other_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_OTHER, &pri), ret);
    diag_util_mprintf("Other Trap Priority : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_other_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_PTP_PRIORITY
/*
 * trap get reason ptp priority
 */
cparser_result_t cparser_cmd_trap_get_reason_ptp_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_PTP, &pri), ret);
    diag_util_mprintf("PTP Trap Priority : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_ptp_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_SWITCH_MAC_PRIORITY
/*
 * trap get reason switch-mac priority
 */
cparser_result_t cparser_cmd_trap_get_reason_switch_mac_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_SELFMAC, &pri), ret);
    diag_util_mprintf("Switch-MAC Trap Priority : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_switch_mac_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_UNKNOWN_DA_PRIORITY
/*
 * trap get reason unknown-da priority
 */
cparser_result_t cparser_cmd_trap_get_reason_unknown_da_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_UNKNOWN_DA, &pri), ret);
    diag_util_mprintf("Unknown-DA Trap Priority : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_unknown_da_priority */
#endif

#ifdef CMD_TRAP_GET_REASON_VLAN_ERROR_PRIORITY
/*
 * trap get reason vlan-error priority
 */
cparser_result_t cparser_cmd_trap_get_reason_vlan_error_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_VLAN_ERR, &pri), ret);
    diag_util_mprintf("VLAN-Error Trap Priority : %u\n", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_reason_vlan_error_priority */
#endif

#ifdef CMD_TRAP_GET_RMA_GROUP_RMA_0X_RMA_1X_RMA_2X_PRIORITY
/*
 * trap get rma group ( rma-0x | rma-1x | rma-2x ) priority
 */
cparser_result_t cparser_cmd_trap_get_rma_group_rma_0x_rma_1x_rma_2x_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('0' == TOKEN_CHAR(4,4))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_RMA, &pri), ret);
        diag_util_mprintf("RMA-Group-0X Trap Priority : %u\n", pri);
    }
    else if ('1' == TOKEN_CHAR(4,4))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_RMA, &pri), ret);
        diag_util_mprintf("RMA-Group-1X Trap Priority : %u\n", pri);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_get(unit, MGMT_TYPE_RMA, &pri), ret);
        diag_util_mprintf("RMA-Group-2X Trap Priority : %u\n", pri);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_rma_group_rma_0x_rma_1x_rma_2x_priority */
#endif

#ifdef CMD_TRAP_GET_SWITCH_MAC
/*
 * trap get switch-mac
 */
cparser_result_t cparser_cmd_trap_get_switch_mac(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_get(unit, MGMT_TYPE_SELFMAC, &action), ret);

    diag_util_printf("Switch Self Mac Action :");
    if (RMA_ACTION_FORWARD == action)
    {
        diag_util_printf(" forward      \n");
    }
    else if (RMA_ACTION_DROP == action)
    {
        diag_util_printf(" drop         \n");
    }
    else if (RMA_ACTION_TRAP2CPU == action)
    {
        diag_util_printf(" trap-to-cpu  \n");
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_CFM_INSERT_CPU_TAG_STATE_DISABLE_ENABLE
/*
 * trap set cfm insert-cpu-tag state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_cfm_insert_cpu_tag_state_disable_enable(cparser_context_t *context)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(5,0))
        enable = DISABLED;
    else
        enable = ENABLED;

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameTrapAddCPUTagEnable_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_cfm_insert_cpu_tag_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_CFM_MD_LEVEL_LEVEL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set cfm md-level <UINT:level> ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_cfm_md_level_level_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    uint32_t *level_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          mdlevel = 0;
    rtk_action_t    action = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    mdlevel = *level_ptr;

    if ('f' == TOKEN_CHAR(5,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action  = ACTION_TRAP2CPU;
    }
    else if ('c' == TOKEN_CHAR(5,0))
    {
        action  = ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameAction_set(unit, mdlevel, action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_cfm_md_level_level_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_CFM_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set cfm priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_cfm_priority_state_disable_enable(cparser_context_t *context)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameTrapPriEnable_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_cfm_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_CFM_PRIORITY_PRIORITY
/*
 * trap set cfm priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_cfm_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_cfmFrameTrapPri_set(unit, pri), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_cfm_priority_priority */
#endif

#ifdef CMD_TRAP_SET_OAMPDU_PORT_ALL_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set oampdu ( <PORT_LIST:port> | all ) ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_oampdu_port_all_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action = ACTION_TRAP2CPU;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('f' == TOKEN_CHAR(4,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('t' == TOKEN_CHAR(4,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('d' == TOKEN_CHAR(4,0))
    {
        action = ACTION_DROP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portOamPDUAction_set(unit, port, action), ret);
    } /* end of 'for (port = min_port; port <= max_port; port++)' */

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_oampdu_port_all_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_OAMPDU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set oampdu ( drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_oampdu_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action = ACTION_TRAP2CPU;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('f' == TOKEN_CHAR(3,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        action = ACTION_DROP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUAction_set(unit, action), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_oampdu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_OAMPDU_PORT_ALL_INSERT_CPU_TAG_STATE_DISABLE_ENABLE
/*
 * trap set oampdu ( <PORT_LIST:port> | all ) insert-cpu-tag state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_oampdu_port_all_insert_cpu_tag_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(6,0))
        enable = DISABLED;
    else
        enable = ENABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUTrapAddCPUTagEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_oampdu_port_all_insert_cpu_tag_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_OAMPDU_PORT_ALL_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set oampdu ( <PORT_LIST:port> | all ) priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_oampdu_port_all_priority_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(6,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUPriEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_oampdu_port_all_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_OAMPDU_PORT_ALL_PRIORITY_PRIORITY
/*
 * trap set oampdu ( <PORT_LIST:port> | all ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_oampdu_port_all_priority_priority(cparser_context_t *context,
    char **port_ptr,
    uint32_t *priority_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_pri_t       pri = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    pri = *priority_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portOamPDUPri_set(unit, port, pri), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_oampdu_port_all_priority_priority */
#endif

#ifdef CMD_TRAP_SET_OAMPDU_PRIORITY_PRIORITY
/*
 * trap set oampdu priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_oampdu_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_oamPDUPri_set(unit, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_oampdu_port_all_priority_priority */
#endif

#ifdef CMD_TRAP_GET_CFI
/*
  * trap get cfi
  */
cparser_result_t cparser_cmd_trap_get_cfi(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_action_t op1,op2;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_pktWithCFIAction_get(unit, &op1) , ret);
    DIAG_UTIL_ERR_CHK(rtk_trap_pktWithOuterCFIAction_get(unit, &op2) , ret);

    diag_util_mprintf("CFI configuration\n");
    diag_util_mprintf("  Inner CFI operation : ");
    if(op1 == ACTION_FORWARD)
        diag_util_mprintf("Forward\n");
    else if(op1 == ACTION_DROP)
        diag_util_mprintf("Drop\n");
    else
        diag_util_mprintf("Trap\n");

    diag_util_mprintf("  Outer CFI operation : ");
    if(op2 == ACTION_FORWARD)
        diag_util_mprintf("Forward\n");
    else if(op2 == ACTION_DROP)
        diag_util_mprintf("Drop\n");
    else
        diag_util_mprintf("Trap\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_CFI_INNER_OUTER_FORWARD_DROP_TRAP
/*
  * trap set cfi ( inner | outer ) ( forward | drop | trap )
  */
cparser_result_t
cparser_cmd_trap_set_cfi_inner_outer_forward_drop_trap(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_action_t op;
    int32   (*fp)(uint32, rtk_action_t);

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('i' == TOKEN_CHAR(3, 0))
    {
        fp = rtk_trap_pktWithCFIAction_set;
    }
    else
    {
        fp = rtk_trap_pktWithOuterCFIAction_set;
    }

    if('f' == TOKEN_CHAR(4, 0))
        {
        op = ACTION_FORWARD;
    }
    else if('d' == TOKEN_CHAR(4, 0))
    {
        op = ACTION_DROP;
    }
    else
    {
        op = ACTION_TRAP2CPU;
    }

    DIAG_UTIL_ERR_CHK(fp(unit, op) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_OPTION_TYPE_IPV4_OPTION_HEADER_PKT_IPV6_OPTION_HEADER_PKT_VLAN_TAG_WITH_CFI_PORT_ALL_FORWARD_TRAP_TO_CPU
/*
 * trap set option-type ( ipv4-option-header-pkt | ipv6-option-header-pkt | vlan-tag-with-cfi ) ( <PORT_LIST:port> | all ) ( forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action = ACTION_TRAP2CPU;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('f' == TOKEN_CHAR(5,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('v' == TOKEN_CHAR(3,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_portPktWithCFIAction_set(unit, port, action), ret);
        }
        else if('i' == TOKEN_CHAR(3,0))
        {
            if('4' == TOKEN_CHAR(3,3))
            {
                DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderAction_set(unit, port, IPV4_FAMILY, action), ret);
            }
            else if('6' == TOKEN_CHAR(3,3))
            {
                DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderAction_set(unit, port, IPV6_FAMILY, action), ret);
            }
            else
            {
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    } /* end of 'for (port = min_port; port <= max_port; port++)' */

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_GET_CFI_PRIORITY
/*
 * trap get cfi priority
 */
cparser_result_t
cparser_cmd_trap_get_cfi_priority(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_pktWithCFIPri_get(unit, &pri), ret);
    diag_util_mprintf("Packet with CFI Trap Priority       : %u\n", pri);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_CFI_PRIORITY_PRIORITY
/*
 * trap set cfi priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_trap_set_cfi_priority_priority(cparser_context_t *context,
        uint32_t *priority_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;

    DIAG_UTIL_ERR_CHK(rtk_trap_pktWithCFIPri_set(unit, pri), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_cfi_priority */
#endif

#ifdef CMD_TRAP_SET_OPTION_TYPE_IPV4_OPTION_HEADER_PKT_IPV6_OPTION_HEADER_PKT_VLAN_TAG_WITH_CFI_PORT_ALL_INSERT_CPU_TAG_STATE_DISABLE_ENABLE
/*
 * trap set option-type ( ipv4-option-header-pkt | ipv6-option-header-pkt | vlan-tag-with-cfi ) ( <PORT_LIST:port> | all ) insert-cpu-tag state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_insert_cpu_tag_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(7,0))
    {
        enable = ENABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('v' == TOKEN_CHAR(3,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_pktWithCFIAddCPUTagEnable_set(unit, port, enable), ret);
        }
        else if('i' == TOKEN_CHAR(3,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderAddCPUTagEnable_set(unit, port, enable), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_insert_cpu_tag_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_OPTION_TYPE_IPV4_OPTION_HEADER_PKT_IPV6_OPTION_HEADER_PKT_VLAN_TAG_WITH_CFI_PORT_ALL_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set option-type ( ipv4-option-header-pkt | ipv6-option-header-pkt | vlan-tag-with-cfi ) ( <PORT_LIST:port> | all ) priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_priority_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('v' == TOKEN_CHAR(3,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_pktWithCFIPriEnable_set(unit, port, enable), ret);
        }
        else if('i' == TOKEN_CHAR(3,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderPriEnable_set(unit, port, enable), ret);
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_OPTION_TYPE_IPV4_OPTION_HEADER_PKT_IPV6_OPTION_HEADER_PKT_VLAN_TAG_WITH_CFI_PORT_ALL_PRIORITY_PRIORITY
/*
 * trap set option-type ( ipv4-option-header-pkt | ipv6-option-header-pkt | vlan-tag-with-cfi ) ( <PORT_LIST:port> | all ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_priority_priority(cparser_context_t *context,
    char **port_ptr,
    uint32_t *priority_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    pri = *priority_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('v' == TOKEN_CHAR(3,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_portPktWithCFIPri_set(unit, port, pri), ret);
        }
        else if('i' == TOKEN_CHAR(3,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_ipWithOptionHeaderPri_set(unit, port, pri), ret);
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_option_type_ipv4_option_header_pkt_ipv6_option_header_pkt_vlan_tag_with_cfi_port_all_priority_priority */
#endif

#ifdef CMD_TRAP_SET_PACKET_1XMAC_PORT_CHANGE_CFI_IPMCAST_DLF_IPV4_IGMP_IPV6_MLD_L2MCAST_DLF_STATE_DISABLE_ENABLE
/*
 * trap set packet ( 1xmac-port-change | cfi | ipmcast-dlf | ipv4-igmp | ipv6-mld | l2mcast-dlf ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_packet_1xmac_port_change_cfi_ipmcast_dlf_ipv4_igmp_ipv6_mld_l2mcast_dlf_state_disable_enable(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_type_t         type = TRAP_TYPE_CONTROL_END;
    rtk_enable_t            enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('1' == TOKEN_CHAR(3,0))
    {
        type = TRAP_TYPE_1XMAC_PORTCHG;
    }
    else if ('c' == TOKEN_CHAR(3,0))
    {
        type = TRAP_TYPE_CFI_1;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        if ('m' == TOKEN_CHAR(3,2))
        {
            type = TRAP_TYPE_IPMC_DLF;
        }
        else if ('4' == TOKEN_CHAR(3,3))
        {
            type = TRAP_TYPE_IPV4_IGMP;
        }
        else if ('6' == TOKEN_CHAR(3,3))
        {
            type = TRAP_TYPE_IPV6_MLD;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('l' == TOKEN_CHAR(3,0))
    {
        type = TRAP_TYPE_L2MC_DLF;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_pkt2CpuEnable_set(unit, type, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_packet_1xmac_port_change_cfi_ipmcast_dlf_ipv4_igmp_ipv6_mld_l2mcast_dlf_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_REASON_1X_EAPOL_1X_UNAUTH_CFI_IPV4_IGMP_IPV6_MLD_MCAST_DLF_RMA_SLP_CHANGE_VLAN_ERROR_PRIORITY_PRIORITY
/*
 * trap set reason ( 1x-eapol | 1x-unauth | cfi | ipv4-igmp | ipv6-mld | mcast-dlf | rma | slp-change | vlan-error ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_1x_eapol_1x_unauth_cfi_ipv4_igmp_ipv6_mld_mcast_dlf_rma_slp_change_vlan_error_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_reason_type_t  type = TRAP_REASON_END;
    rtk_pri_t               priority = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    priority = *priority_ptr;

    if ('1' == TOKEN_CHAR(3,0))
    {
        if ('e' == TOKEN_CHAR(3,3))
        {
            type = TRAP_REASON_1XEAPOL;
        }
        else if ('u' == TOKEN_CHAR(3,3))
        {
            type = TRAP_REASON_1XUNAUTH;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('c' == TOKEN_CHAR(3,0))
    {
        type = TRAP_REASON_CFI;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        if ('4' == TOKEN_CHAR(3,3))
        {
            type = TRAP_REASON_IPV4IGMP;
        }
        else if ('6' == TOKEN_CHAR(3,3))
        {
            type = TRAP_REASON_IPV6MLD;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('m' == TOKEN_CHAR(3,0))
    {
        type = TRAP_REASON_MULTICASTDLF;
    }
    else if ('r' == TOKEN_CHAR(3,0))
    {
        type = TRAP_REASON_RMA;
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        type = TRAP_REASON_SLPCHANGE;
    }
    else if ('v' == TOKEN_CHAR(3,0))
    {
        type = TRAP_REASON_VLANERR;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_reasonTrapToCPUPriority_set(unit, type, priority), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_1x_eapol_1x_unauth_cfi_ipv4_igmp_ipv6_mld_mcast_dlf_rma_slp_change_vlan_error_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_1X_EAPOL_PRIORITY_PRIORITY
/*
 * trap set reason 1x-eapol priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_1x_eapol_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_EAPOL, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_1x_eapol_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_ARP_IPV6_ND_PRIORITY_PRIORITY
/*
 * trap set reason ( arp | ipv6-nd ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_arp_ipv6_nd_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;

    if ('a' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_ARP, pri), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_IPV6ND, pri), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_arp_ipv6_nd_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_BPDU_LACP_LLDP_PRIORITY_PRIORITY
/*
 * trap set reason ( bpdu | lacp | lldp ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_bpdu_lacp_lldp_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;

    if ('b' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_BPDU, pri), ret);
    }
    else if ('a' == TOKEN_CHAR(3,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_LACP, pri), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_LLDP, pri), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_bpdu_lacp_lldp_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_INGRESS_VLAN_FILTER_PRIORITY_PRIORITY
/*
 * trap set reason ingress-vlan-filter priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_ingress_vlan_filter_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_IGR_VLAN_FLTR, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_ingress_vlan_filter_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_IPV4_IGMP_IPV6_MLD_PRIORITY_PRIORITY
/*
 * trap set reason ( ipv4-igmp | ipv6-mld ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_ipv4_igmp_ipv6_mld_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;

    if ('4' == TOKEN_CHAR(3,3))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_IGMP, pri), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_MLD, pri), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_ipv4_igmp_ipv6_mld_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_OTHER_PRIORITY_PRIORITY
/*
 * trap set reason other priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_other_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_OTHER, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_other_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_PTP_PRIORITY_PRIORITY
/*
 * trap set reason ptp priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_ptp_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_PTP, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_ptp_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_SWITCH_MAC_PRIORITY_PRIORITY
/*
 * trap set reason switch-mac priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_switch_mac_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_SELFMAC, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_switch_mac_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_UNKNOWN_DA_PRIORITY_PRIORITY
/*
 * trap set reason unknown-da priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_unknown_da_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_UNKNOWN_DA, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_unknown_da_priority_priority */
#endif

#ifdef CMD_TRAP_SET_REASON_VLAN_ERROR_PRIORITY_PRIORITY
/*
 * trap set reason vlan-error priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_reason_vlan_error_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_VLAN_ERR, pri), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_reason_vlan_error_priority_priority */
#endif

#ifdef CMD_TRAP_SET_RMA_GROUP_RMA_0X_RMA_1X_RMA_2X_PRIORITY_PRIORITY
/*
 * trap set rma group ( rma-0x | rma-1x | rma-2x ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_rma_group_rma_0x_rma_1x_rma_2x_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    pri = *priority_ptr;

    if ('0' == TOKEN_CHAR(4,4))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_RMA, pri), ret);
    }
    else if ('1' == TOKEN_CHAR(4,4))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_RMA, pri), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, MGMT_TYPE_RMA, pri), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_group_rma_0x_rma_1x_rma_2x_priority_priority */
#endif

#ifdef CMD_TRAP_SET_RMA_INSERT_CPU_TAG_STATE_DISABLE_ENABLE
/*
 * trap set rma insert-cpu-tag state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_insert_cpu_tag_state_disable_enable(cparser_context_t *context)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(5,0))
        enable = DISABLED;
    else
        enable = ENABLED;

    DIAG_UTIL_ERR_CHK(rtk_trap_rmaCpuTagAddEnable_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_insert_cpu_tag_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_ACTION_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma l2-user-define <UINT:index> action ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_rma_l2_user_define_index_action_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    uint32                  index = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    if ('f' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaAction_set(unit, index, rma_action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l2_user_define_index_action_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_ACTION_TRAP_TO_CPU_DROP_FORWARD
/*
 * trap set rma l2-user-define <UINT:index> action ( trap-to-cpu | drop | forward )
 */
cparser_result_t
cparser_cmd_trap_set_rma_l2_user_define_index_action_trap_to_cpu_drop_forward(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    uint32                  index = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    if ('f' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaAction_set(unit, index, rma_action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l2_user_define_index_action_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_STATE_DISABLE_ENABLE
/*
 * trap set rma l2-user-define <UINT:index> state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_l2_user_define_index_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    uint32  index;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    if ('d' == TOKEN_CHAR(6,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaEnable_set(unit, index, enable), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l2_user_define_index_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set rma l2-user-define <UINT:index> priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_l2_user_define_index_priority_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaPriEnable_set(unit, index, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l2_user_define_index_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_GET_RMA_L2_USER_DEFINE_INDEX_STATE
/*
 * trap get rma l2-user-define <UINT:index> state
 */
cparser_result_t cparser_cmd_trap_get_rma_l2_user_define_index_state(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    uint32  index;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaEnable_get(unit, index, &enable), ret);
    diag_util_mprintf("User-Defined RMA %d : %s\n", index, enable ? "ENABLE" : "DISABLE");

    return CPARSER_OK;
} /* end of cparser_cmd_trap_get_rma_l2_user_define_index_state */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_PRIORITY_PRIORITY
/*
 * trap set rma l2-user-define <UINT:index> priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_rma_l2_user_define_index_priority_priority(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *priority_ptr)
{
    uint32  unit = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;
    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaPri_set(unit, index, pri), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l2_user_define_index_priority_priority */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_MAC_MASK_BPDU_VLAN_CARE
/*
 * trap set rma l2-user-define <UINT:index> <MACADDR:mac> <MACADDR:mask> { bpdu } { vlan-care }
 */
cparser_result_t cparser_cmd_trap_set_rma_l2_user_define_index_mac_mask_bpdu_vlan_care(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *mask_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_trap_userDefinedRma_t   userDefinedRma;
    uint32  index;
    uint32  bpdu = FALSE;
    uint32  vlanCare = FALSE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&userDefinedRma, 0, sizeof(rtk_trap_userDefinedRma_t));

    index = atoi(TOKEN_STR(4));
    osal_memcpy(&userDefinedRma.mac.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));
    osal_memcpy(&userDefinedRma.macMask.octet[0], &mask_ptr->octet[0], sizeof(cparser_macaddr_t));

    if (TOKEN_NUM == 7)
    {
        bpdu = FALSE;
        vlanCare = FALSE;
    }
    else if(TOKEN_NUM == 8)
    {
        if('b' == TOKEN_CHAR(7,0))
            bpdu = TRUE;
        else
            vlanCare = TRUE;
    }
    else
    {
        bpdu = TRUE;
        vlanCare = TRUE;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaVlanCheckEnable_set(unit, index, vlanCare), ret);
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaStpBlockEnable_set(unit, index, bpdu), ret);
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRma_set(unit, index, &userDefinedRma), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l2_user_define_index_mac_mask_bpdu_vlan_care */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_MAC
/*
 * trap set rma l2-user-define <UINT:index> <MACADDR:mac>
 */
cparser_result_t cparser_cmd_trap_set_rma_l2_user_define_index_mac(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    rtk_trap_userDefinedRma_t   userDefinedRma;
    uint32  index;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&userDefinedRma, 0, sizeof(rtk_trap_userDefinedRma_t));

    index = atoi(TOKEN_STR(4));
    osal_memcpy(&userDefinedRma.mac.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRma_set(unit, index, &userDefinedRma), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l2_user_define_index_mac_mask_bpdu_vlan_care */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_PROTOCOL_ARP_BGPV4_FTP_HTTP_HTTPS_ICMP_ICMPV6_IGMP_MLD_OSPFV2_OSPFV3_RIP_SNMP_SSH_TELNET_TFTP_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma l34-protocol ( arp | bgpv4 | ftp | http | https | icmp | icmpv6 | igmp | mld | ospfv2 | ospfv3 | rip | snmp  | ssh | telnet | tftp ) ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_trap_mgmtType_t     frameType;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('a' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_ARP;
    else if('b' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_BGP;
    else if('f' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_FTP;
    else if('h' == TOKEN_CHAR(4,0))
    {
        if('s' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_HTTPS;
        else
            frameType = MGMT_TYPE_HTTP;
    }
    else if('i' == TOKEN_CHAR(4,0))
    {
        if('g' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_IGMP;
        else
        {
            if('v' == TOKEN_CHAR(4,4))
                frameType = MGMT_TYPE_ICMPV6;
            else
                frameType = MGMT_TYPE_ICMP;
        }
    }
    else if('m' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_MLD;
    else if('o' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,5))
            frameType = MGMT_TYPE_OSPFV2;
        else
            frameType = MGMT_TYPE_OSPFV3;
    }
    else if('r' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_RIP;
    else if('s' == TOKEN_CHAR(4,0))
    {
        if('n' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_SNMP;
        else
            frameType = MGMT_TYPE_SSH;
    }
    else if('t' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_TELNET;
        else
            frameType = MGMT_TYPE_TFTP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('f' == TOKEN_CHAR(5,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(5,0))
    {
        rma_action = RMA_ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_set(unit, frameType, rma_action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_PROTOCOL_ARP_BGPV4_FTP_HTTP_HTTPS_ICMP_ICMPV6_IGMP_MLD_OSPFV2_OSPFV3_RIP_SNMP_SSH_TELNET_TFTP_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set rma l34-protocol ( arp | bgpv4 | ftp | http | https | icmp | icmpv6 | igmp | mld | ospfv2 | ospfv3 | rip | snmp  | ssh | telnet | tftp ) priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_priority_state_disable_enable(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_enable_t        enable = DISABLED;
    rtk_trap_mgmtType_t frameType;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('a' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_ARP;
    else if('b' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_BGP;
    else if('f' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_FTP;
    else if('h' == TOKEN_CHAR(4,0))
    {
        if('s' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_HTTPS;
        else
            frameType = MGMT_TYPE_HTTP;
    }
    else if('i' == TOKEN_CHAR(4,0))
    {
        if('g' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_IGMP;
        else
        {
            if('v' == TOKEN_CHAR(4,4))
                frameType = MGMT_TYPE_ICMPV6;
            else
                frameType = MGMT_TYPE_ICMP;
        }
    }
    else if('m' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_MLD;
    else if('o' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,5))
            frameType = MGMT_TYPE_OSPFV2;
        else
            frameType = MGMT_TYPE_OSPFV3;
    }
    else if('r' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_RIP;
    else if('s' == TOKEN_CHAR(4,0))
    {
        if('n' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_SNMP;
        else
            frameType = MGMT_TYPE_SSH;
    }
    else if('t' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_TELNET;
        else
            frameType = MGMT_TYPE_TFTP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePriEnable_set(unit, frameType, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_PROTOCOL_ARP_BGPV4_FTP_HTTP_HTTPS_ICMP_ICMPV6_IGMP_MLD_OSPFV2_OSPFV3_RIP_SNMP_SSH_TELNET_TFTP_PRIORITY_PRIORITY
/*
 * trap set rma l34-protocol ( arp | bgpv4 | ftp | http | https | icmp | icmpv6 | igmp | mld | ospfv2 | ospfv3 | rip | snmp  | ssh | telnet | tftp ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_pri_t           pri = 0;
    rtk_trap_mgmtType_t frameType;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('a' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_ARP;
    else if('b' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_BGP;
    else if('f' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_FTP;
    else if('h' == TOKEN_CHAR(4,0))
    {
        if('s' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_HTTPS;
        else
            frameType = MGMT_TYPE_HTTP;
    }
    else if('i' == TOKEN_CHAR(4,0))
    {
        if('g' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_IGMP;
        else
        {
            if('v' == TOKEN_CHAR(4,4))
                frameType = MGMT_TYPE_ICMPV6;
            else
                frameType = MGMT_TYPE_ICMP;
        }
    }
    else if('m' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_MLD;
    else if('o' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,5))
            frameType = MGMT_TYPE_OSPFV2;
        else
            frameType = MGMT_TYPE_OSPFV3;
    }
    else if('r' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_RIP;
    else if('s' == TOKEN_CHAR(4,0))
    {
        if('n' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_SNMP;
        else
            frameType = MGMT_TYPE_SSH;
    }
    else if('t' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_TELNET;
        else
            frameType = MGMT_TYPE_TFTP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFramePri_set(unit, frameType, pri), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_priority_priority */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_PROTOCOL_ARP_BGPV4_FTP_HTTP_HTTPS_ICMP_ICMPV6_IGMP_MLD_OSPFV2_OSPFV3_RIP_SNMP_SSH_TELNET_TFTP_VLAN_CHECK_STATE_DISABLE_ENABLE
/*
 * trap set rma l34-protocol ( arp | bgpv4 | ftp | http | https | icmp | icmpv6 | igmp | mld | ospfv2 | ospfv3 | rip | snmp  | ssh | telnet | tftp ) vlan-check state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_vlan_check_state_disable_enable(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_enable_t        enable;
    rtk_trap_mgmtType_t frameType;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('a' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_ARP;
    else if('b' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_BGP;
    else if('f' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_FTP;
    else if('h' == TOKEN_CHAR(4,0))
    {
        if('s' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_HTTPS;
        else
            frameType = MGMT_TYPE_HTTP;
    }
    else if('i' == TOKEN_CHAR(4,0))
    {
        if('g' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_IGMP;
        else
        {
            if('v' == TOKEN_CHAR(4,4))
                frameType = MGMT_TYPE_ICMPV6;
            else
                frameType = MGMT_TYPE_ICMP;
        }
    }
    else if('m' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_MLD;
    else if('o' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,5))
            frameType = MGMT_TYPE_OSPFV2;
        else
            frameType = MGMT_TYPE_OSPFV3;
    }
    else if('r' == TOKEN_CHAR(4,0))
        frameType = MGMT_TYPE_RIP;
    else if('s' == TOKEN_CHAR(4,0))
    {
        if('n' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_SNMP;
        else
            frameType = MGMT_TYPE_SSH;
    }
    else if('t' == TOKEN_CHAR(4,0))
    {
        if('2' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_TELNET;
        else
            frameType = MGMT_TYPE_TFTP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(7,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameVlanCheck_set(unit, frameType, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_protocol_arp_bgpv4_ftp_http_https_icmp_icmpv6_igmp_mld_ospfv2_ospfv3_rip_snmp_ssh_telnet_tftp_vlan_check_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_USER_PROTOCOL_INDEX_TCP_UDP_COMPARE_DMAC_COMPARE_DIP
/*
 * trap set rma l34-user-protocol <UINT:index> ( tcp | udp ) { compare-dmac } { compare-dip }
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_user_protocol_index_tcp_udp_compare_dmac_compare_dip(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trap_userDefinedMgmt_t  config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_trap_userDefinedMgmt_t));
    index = *index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmt_get(unit, index, &config), ret);

    if ('t' == TOKEN_CHAR(5,0))
    {
        config.layer4Proto = L4PROTO_TCP;
    }
    else if ('u' == TOKEN_CHAR(5,0))
    {
        config.layer4Proto = L4PROTO_UDP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if (TOKEN_NUM == 6)
    {
        config.dmacCheckEnable = FALSE;
        config.dipCheckEnable = FALSE;
    }
    else if (TOKEN_NUM == 7)
    {
        if ('m' == TOKEN_CHAR(6,9))
            config.dmacCheckEnable = TRUE;
        else
            config.dipCheckEnable = TRUE;
    }
    else
    {
        config.dmacCheckEnable = TRUE;
        config.dipCheckEnable = TRUE;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmt_set(unit, index, &config), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_user_protocol_index_tcp_udp_compare_dmac_compare_dip */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_USER_PROTOCOL_INDEX_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma l34-user-protocol <UINT:index> ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_user_protocol_index_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    rma_action = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    if ('f' == TOKEN_CHAR(5,0))
    {
        rma_action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        rma_action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        rma_action = ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(5,0))
    {
        rma_action = ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtAction_set(unit, index, rma_action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_user_protocol_index_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_USER_PROTOCOL_INDEX_COMPARE_L4_DPORT_VALUE_MASK
/*
 * trap set rma l34-user-protocol <UINT:index> compare-l4-dport <UINT:value>  <UINT:mask>
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_user_protocol_index_compare_l4_dport_value_mask(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *value_ptr, uint32_t *mask_ptr)
{
    uint32      unit = 0;
    uint32      index = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_trap_userDefinedMgmt_t  config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_trap_userDefinedMgmt_t));
    index = *index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmt_get(unit, index, &config), ret);

    config.dstL4PortCheck= TRUE;
    config.dstL4Port = *value_ptr;
    config.mask_of_dstL4Port = *mask_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmt_set(unit, index, &config), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_user_protocol_index_compare_l4_dport_value_mask */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_USER_PROTOCOL_INDEX_COMPARE_L4_SPORT_VALUE_MASK
/*
 * trap set rma l34-user-protocol <UINT:index> compare-l4-sport <UINT:value>  <UINT:mask>
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_user_protocol_index_compare_l4_sport_value_mask(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *value_ptr, uint32_t *mask_ptr)
{
    uint32      unit = 0;
    uint32      index = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_trap_userDefinedMgmt_t  config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_trap_userDefinedMgmt_t));
    index = *index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmt_get(unit, index, &config), ret);

    config.srcL4PortCheck= TRUE;
    config.srcL4Port = *value_ptr;
    config.mask_of_srcL4Port = *mask_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmt_set(unit, index, &config), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_user_protocol_index_compare_l4_sport_value_mask */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_USER_PROTOCOL_INDEX_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set rma l34-user-protocol <UINT:index> priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_user_protocol_index_priority_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    uint32      index = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;
    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtPriEnable_set(unit, index, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_user_protocol_index_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_USER_PROTOCOL_INDEX_PRIORITY_PRIORITY
/*
 * trap set rma l34-user-protocol <UINT:index> priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_user_protocol_index_priority_priority(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *priority_ptr)
{
    uint32      unit = 0;
    uint32      index = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;
    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtPri_set(unit, index, pri), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_user_protocol_index_priority_priority */
#endif

#ifdef CMD_TRAP_SET_RMA_L34_USER_PROTOCOL_INDEX_VLAN_CHECK_STATE_DISABLE_ENABLE
/*
 * trap set rma l34-user-protocol <UINT:index> vlan-check state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_l34_user_protocol_index_vlan_check_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;
    if ('e' == TOKEN_CHAR(7,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineMgmtVlanCheck_set(unit, index, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_l34_user_protocol_index_vlan_check_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_ACTION_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma layer2 <UINT:rma_tail> action ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_rma_layer2_rma_tail_action_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    uint32_t *rma_tail_ptr)
{
    uint32                  unit = 0;
    uint32                  ramTail = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_mac_t               rma_frame;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    rma_frame.octet[0] = 0x01;
    rma_frame.octet[1] = 0x80;
    rma_frame.octet[2] = 0xc2;
    rma_frame.octet[3] = 0x00;
    rma_frame.octet[4] = 0x00;
    rma_frame.octet[5] = 0x00;

    ramTail = *rma_tail_ptr;
    if (ramTail > 0x2f)
    {
        diag_util_printf("\nInput error: the RMA address is out of range\n");
        return CPARSER_NOT_OK;
    }
    else
    {
        rma_frame.octet[5] = ramTail;
    }

    if ('f' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_rmaAction_set(unit, &rma_frame, rma_action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_action_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_RMA_TAIL_END_ACTION_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma layer2 <UINT:rma_tail> <UINT:rma_tail_end> action ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_rma_layer2_rma_tail_rma_tail_end_action_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    uint32_t *rma_tail_ptr, uint32_t *rma_tail_end_ptr)
{
    uint32                  unit = 0;
    uint32                  ramTail = 0, ramTailEnd, i;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_mac_t               rma_frame;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    rma_frame.octet[0] = 0x01;
    rma_frame.octet[1] = 0x80;
    rma_frame.octet[2] = 0xc2;
    rma_frame.octet[3] = 0x00;
    rma_frame.octet[4] = 0x00;
    rma_frame.octet[5] = 0x00;

    ramTail = *rma_tail_ptr;
    ramTailEnd = *rma_tail_end_ptr;
    if (ramTail > 0x2f || ramTailEnd > 0x2f)
    {
        diag_util_printf("\nInput error: the RMA address is out of range\n");
        return CPARSER_NOT_OK;
    }

    if ('f' == TOKEN_CHAR(7,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(7,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(7,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(7,0))
    {
        rma_action = RMA_ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    for (i = ramTail; i <= ramTailEnd; i++)
    {
        rma_frame.octet[5] = i;
        DIAG_UTIL_ERR_CHK(rtk_trap_rmaAction_set(unit, &rma_frame, rma_action), ret);
    }
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_rma_tail_end_action_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set rma layer2 <UINT:rma_tail> priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_layer2_rma_tail_priority_state_disable_enable(cparser_context_t *context,
    uint32_t *rma_tail_ptr)
{
    uint32  unit = 0;
    uint32  ramTail = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_mac_t   rma_frame;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    rma_frame.octet[0] = 0x01;
    rma_frame.octet[1] = 0x80;
    rma_frame.octet[2] = 0xc2;
    rma_frame.octet[3] = 0x00;
    rma_frame.octet[4] = 0x00;
    rma_frame.octet[5] = 0x00;

    ramTail = *rma_tail_ptr;
    if (*rma_tail_ptr > 0x2f)
    {
        diag_util_printf("\nInput error: the RMA address is out of range\n");
        return CPARSER_NOT_OK;
    }
    else
    {
        rma_frame.octet[5] = ramTail;
    }

    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_rmaPriEnable_set(unit, &rma_frame, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_PRIORITY_PRIORITY
/*
 * trap set rma layer2 <UINT:rma_tail> priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_rma_layer2_rma_tail_priority_priority(cparser_context_t *context,
    uint32_t *rma_tail_ptr, uint32_t *priority_ptr)
{
    uint32  unit = 0;
    uint32  ramTail = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_mac_t   rma_frame;
    rtk_pri_t   pri = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    rma_frame.octet[0] = 0x01;
    rma_frame.octet[1] = 0x80;
    rma_frame.octet[2] = 0xc2;
    rma_frame.octet[3] = 0x00;
    rma_frame.octet[4] = 0x00;
    rma_frame.octet[5] = 0x00;

    ramTail = *rma_tail_ptr;
    if (*rma_tail_ptr > 0x2f)
    {
        diag_util_printf("\nInput error: the RMA address is out of range\n");
        return CPARSER_NOT_OK;
    }
    else
    {
        rma_frame.octet[5] = ramTail;
    }

    pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_trap_rmaPri_set(unit, &rma_frame, pri), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_priority_priority */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_VLAN_CHECK_STATE_DISABLE_ENABLE
/*
 * trap set rma layer2 <UINT:rma_tail> vlan-check state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_layer2_rma_tail_vlan_check_state_disable_enable(cparser_context_t *context,
    uint32_t *rma_tail_ptr)
{
    uint32                  unit = 0;
    uint32                  ramTail = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_mac_t               rma_frame;
    rtk_enable_t          enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    rma_frame.octet[0] = 0x01;
    rma_frame.octet[1] = 0x80;
    rma_frame.octet[2] = 0xc2;
    rma_frame.octet[3] = 0x00;
    rma_frame.octet[4] = 0x00;
    rma_frame.octet[5] = 0x00;

    ramTail = *rma_tail_ptr;
    if (ramTail > 0x2f)
    {
        diag_util_printf("\nInput error: the RMA address is out of range\n");
        return CPARSER_NOT_OK;
    }
    else
    {
        rma_frame.octet[5] = ramTail;
    }

    if ('e' == TOKEN_CHAR(7,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_rmaVlanCheckEnable_set(unit, &rma_frame, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_vlan_check_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_MGMT_IP_CHECK_IPV4_IPV6_STATE_DISABLE_ENABLE
/*
 * trap set rma mgmt-ip-check ( ipv4 | ipv6 ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_mgmt_ip_check_ipv4_ipv6_state_disable_enable(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(6,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('4' == TOKEN_CHAR(4,3))
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtIpCheck_set(unit, MGMT_IP_TYPE_IPV4, enable), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtIpCheck_set(unit, MGMT_IP_TYPE_IPV6, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_mgmt_ip_check_ipv4_ipv6_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_BPDU_DHCP_DHCPV6_DOT1X_PAE_PORT_ALL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma port-rma ( bpdu | dhcp | dhcpv6 | dot1x-pae ) ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_trap_mgmtType_t frameType = MGMT_TYPE_BPDU;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(4,0))
    {
        frameType = MGMT_TYPE_BPDU;
    }
    else if('d' == TOKEN_CHAR(4,0))
    {
        if('v' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_DHCPV6;
        else if('o' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_DOT1X;
        else
            frameType = MGMT_TYPE_DHCP;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('f' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_set(unit, port, frameType, rma_action), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_BPDU_PORT_ALL_DROP_FORWARD_FORWARD_AND_FLOOD_TRAP_TO_CPU
/*
 * trap set rma port-rma bpdu ( <PORT_LIST:port> | all ) ( drop | forward | forward-and-flood | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_rma_port_rma_bpdu_port_all_drop_forward_forward_and_flood_trap_to_cpu(
        cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_trap_mgmtType_t     frameType;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    frameType = MGMT_TYPE_BPDU;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('f' == TOKEN_CHAR(6,0))
    {
        if ('-' == TOKEN_CHAR(6,7))
            rma_action = RMA_ACTION_FLOOD_IN_ALL_PORT;
        else
            rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_set(unit, port, frameType, rma_action), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_port_rma_bpdu_port_all_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_LLDP_PORT_ALL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma port-rma lldp ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_rma_port_rma_lldp_port_all_copy_to_cpu_drop_forward_trap_to_cpu(
        cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_trap_mgmtType_t     frameType;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    frameType = MGMT_TYPE_LLDP;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('f' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_set(unit, port, frameType, rma_action), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_port_rma_lldp_port_all_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_PTP_PORT_ALL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set rma port-rma ptp ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_rma_port_rma_ptp_port_all_copy_to_cpu_drop_forward_trap_to_cpu(
        cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_rma_action_t   rma_action = RMA_ACTION_FORWARD;
    rtk_trap_mgmtType_t     frameType;
    rtk_port_t              port = 0;
    diag_portlist_t         portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    frameType = MGMT_TYPE_PTP;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('f' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_TRAP2CPU;
    }
    else if('c' == TOKEN_CHAR(6,0))
    {
        rma_action = RMA_ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_set(unit, port, frameType, rma_action), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_port_rma_ptp_port_all_copy_to_cpu_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_BPDU_DHCP_DHCPV6_DOT1X_PAE_PORT_ALL_PRIORITY_STATE_DISABLE_ENABLE
/*
 * trap set rma port-rma ( bpdu | dhcp | dhcpv6 | dot1x-pae ) ( <PORT_LIST:port> | all ) priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_priority_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    rtk_port_t      port = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_trap_mgmtType_t frameType = MGMT_TYPE_BPDU;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(4,0))
    {
        frameType = MGMT_TYPE_BPDU;
    }
    else if('d' == TOKEN_CHAR(4,0))
    {
        if('v' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_DHCPV6;
        else if('o' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_DOT1X;
        else
            frameType = MGMT_TYPE_DHCP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('d' == TOKEN_CHAR(8,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFramePriEnable_set(unit, port, frameType, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_priority_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_BPDU_DHCP_DHCPV6_DOT1X_PAE_PORT_ALL_PRIORITY_PRIORITY
/*
 * trap set rma port-rma ( bpdu | dhcp | dhcpv6 | dot1x-pae ) ( <PORT_LIST:port> | all ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_priority_priority(cparser_context_t *context,
    char **port_ptr,
    uint32_t *priority_ptr)
{
    uint32          unit = 0;
    rtk_port_t      port = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pri_t       pri = 0;
    rtk_trap_mgmtType_t frameType = MGMT_TYPE_BPDU;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(4,0))
    {
        frameType = MGMT_TYPE_BPDU;
    }
    else if('d' == TOKEN_CHAR(4,0))
    {
        if('v' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_DHCPV6;
        else if('o' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_DOT1X;
        else
            frameType = MGMT_TYPE_DHCP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    pri = *priority_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFramePri_set(unit, port, frameType, pri), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_priority_priority */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_BPDU_DHCP_DHCPV6_DOT1X_PAE_PORT_ALL_VLAN_CHECK_VLAN_CROSS_STATE_DISABLE_ENABLE
/*
 * trap set rma port-rma ( bpdu | dhcp | dhcpv6 | dot1x-pae ) ( <PORT_LIST:port> | all ) ( vlan-check | vlan-cross ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_vlan_check_vlan_cross_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    rtk_port_t      port = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_trap_mgmtType_t frameType = MGMT_TYPE_BPDU;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(4,0))
    {
        frameType = MGMT_TYPE_BPDU;
    }
    else if('d' == TOKEN_CHAR(4,0))
    {
        if('v' == TOKEN_CHAR(4,4))
            frameType = MGMT_TYPE_DHCPV6;
        else if('o' == TOKEN_CHAR(4,1))
            frameType = MGMT_TYPE_DOT1X;
        else
            frameType = MGMT_TYPE_DHCP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('e' == TOKEN_CHAR(8,0))
    {
        enable = ENABLED;
    }
    else if('d' == TOKEN_CHAR(8,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('r' == TOKEN_CHAR(6,6))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameCrossVlan_set(unit, port, frameType, enable), ret);
        }
        else if ('h' == TOKEN_CHAR(6,6))
        {
            DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameVlanCheck_set(unit, port, frameType, enable), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_rma_port_rma_bpdu_dhcp_dhcpv6_dot1x_pae_port_all_vlan_check_vlan_cross_state_disable_enable */
#endif

#ifdef CMD_TRAP_SET_EAPOL_FORWARD_TRAP_TO_CPU
/*
 * trap set eapol ( forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_eapol_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t action = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('f' == TOKEN_CHAR(3, 0))
    {
        action = ACTION_FORWARD;
    }
    else
    {
        action = ACTION_TRAP2CPU;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_set(unit, MGMT_TYPE_EAPOL, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_EAPOL_BYPASS_STP_STATE_DISABLE_ENABLE
/*
 * trap set eapol bypass-stp state ( disable | enable )
 */

cparser_result_t cparser_cmd_trap_set_eapol_bypass_stp_state_disable_enable(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_set(unit, BYPASS_STP_TYPE_EAPOL, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_ARP_REQUEST_FORWARD_TRAP_TO_CPU
/*
 * trap set arp-request ( forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_arp_request_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t action = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('f' == TOKEN_CHAR(3, 0))
    {
        action = ACTION_FORWARD;
    }
    else
    {
        action = ACTION_TRAP2CPU;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_set(unit, MGMT_TYPE_ARP, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_NEIGHBOR_DISCOVERY_FORWARD_TRAP_TO_CPU
/*
 * trap set neighbor-discovery ( forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_neighbor_discovery_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t action = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('f' == TOKEN_CHAR(3, 0))
    {
        action = ACTION_FORWARD;
    }
    else
    {
        action = ACTION_TRAP2CPU;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_set(unit, MGMT_TYPE_IPV6ND, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_SET_SWITCH_MAC_DROP_FORWARD_TRAP_TO_CPU
/*
 * trap set switch-mac ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_trap_set_switch_mac_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t action = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('f' == TOKEN_CHAR(3, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(3, 0))
    {
        action = ACTION_DROP;
    }
    else
    {
        action = ACTION_TRAP2CPU;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_set(unit, MGMT_TYPE_SELFMAC, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRAP_GET_CFM_UNKNOWN_FRAME_ACTION
/* trap get cfm unknown-frame action */
cparser_result_t
cparser_cmd_trap_get_cfm_unknown_frame_action(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_trap_cfmUnknownFrameAct_get(unit, &action);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (ACTION_DROP == action)
        diag_util_mprintf("\tCFM unknown type frame action: Drop\n");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("\tCFM unknown type frame action: Trap\n");
    else if (ACTION_FORWARD == action)
        diag_util_mprintf("\tCFM unknown type frame action: Forward\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_cfm_unknown_frame_action */
#endif

#ifdef CMD_TRAP_SET_CFM_UNKNOWN_FRAME_DROP_TRAP_FORWARD
/* trap set cfm unknown-frame ( drop | trap | forward ) */
cparser_result_t
cparser_cmd_trap_set_cfm_unknown_frame_drop_trap_forward(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    switch (TOKEN_CHAR(4, 0))
    {
        case 'd':
            action = ACTION_DROP;
            break;
        case 't':
            action = ACTION_TRAP2CPU;
            break;
        case 'f':
            action = ACTION_FORWARD;
            break;
        default:
            diag_util_printf("User config: action Error!\n");
            return CPARSER_NOT_OK;
    }   /* switch (TOKEN_CHAR(4, 0)) */

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmUnknownFrameAct_set(unit, action), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_cfm_unknown_frame_drop_trap_forward */
#endif  /* CMD_TRAP_SET_CFM_UNKNOWN_FRAME_DROP_TRAP_FORWARD */

#ifdef CMD_TRAP_GET_CFM_LOOPBACK_LEVEL_ACTION
/* trap get cfm loopback <UINT:level> action */
cparser_result_t
cparser_cmd_trap_get_cfm_loopback_level_action(cparser_context_t *context,
                                              uint32_t *level_ptr)
{
    uint32          unit = 0;
    int32           ret;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((level_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_trap_cfmLoopbackLinkTraceAct_get(unit, *level_ptr, &action);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM Loopback & Linktrace frame with MD level %d action: ",
                      *level_ptr);
    if (ACTION_DROP == action)
        diag_util_mprintf("Drop\n");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("Trap\n");
    else if (ACTION_FORWARD == action)
        diag_util_mprintf("Forward\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_cfm_loopback_level_action */
#endif  /* CMD_TRAP_GET_CFM_LOOPBACK_LEVEL_ACTION */

#ifdef CMD_TRAP_SET_CFM_LOOPBACK_LEVEL_DROP_TRAP_FORWARD
/* trap set cfm loopback <UINT:level> ( drop | trap | forward ) */
cparser_result_t
cparser_cmd_trap_set_cfm_loopback_level_drop_trap_forward(cparser_context_t *context,
                                                         uint32_t *level_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((level_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    switch (TOKEN_CHAR(5, 0))
    {
        case 'd':
            action = ACTION_DROP;
            break;
        case 't':
            action = ACTION_TRAP2CPU;
            break;
        case 'f':
            action = ACTION_FORWARD;
            break;
        default:
            diag_util_printf("User config: action Error!\n");
            return CPARSER_NOT_OK;
    }   /* switch (TOKEN_CHAR(5, 0)) */

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmLoopbackLinkTraceAct_set(unit, *level_ptr, action), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_cfm_loopback_level_drop_trap_forward */
#endif  /* CMD_TRAP_SET_CFM_LOOPBACK_LEVEL_DROP_TRAP_FORWARD */

#ifdef CMD_TRAP_GET_CFM_CCM_LEVEL_ACTION
/* trap get cfm ccm <UINT:level> action */
cparser_result_t
cparser_cmd_trap_get_cfm_ccm_level_action(cparser_context_t *context,
                                          uint32_t *level_ptr)
{
    uint32          unit = 0;
    int32           ret;
    rtk_trap_oam_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((level_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_trap_cfmCcmAct_get(unit, *level_ptr, &action);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM CCM frame with MD level %d action: ",
                      *level_ptr);
    if (ACTION_DROP == action)
        diag_util_mprintf("Drop\n");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("Trap\n");
    else if (ACTION_FORWARD == action)
        diag_util_mprintf("Forward\n");
    else if (TRAP_OAM_ACTION_LINK_FAULT_DETECT == action)
        diag_util_mprintf("Link fault detection\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_cfm_ccm_level_action */
#endif  /* CMD_TRAP_GET_CFM_CCM_LEVEL_ACTION */

#ifdef CMD_TRAP_SET_CFM_CCM_LEVEL_DROP_TRAP_FORWARD_LINK_FAULT_DETECTION
/* trap set cfm ccm <UINT:level> ( drop | trap | forward | link-fault-detection ) */
cparser_result_t
cparser_cmd_trap_set_cfm_ccm_level_drop_trap_forward_link_fault_detection(
        cparser_context_t *context, uint32_t *level_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((level_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    switch (TOKEN_CHAR(5, 0))
    {
        case 'd':
            action = ACTION_DROP;
            break;
        case 't':
            action = ACTION_TRAP2CPU;
            break;
        case 'f':
            action = ACTION_FORWARD;
            break;
        case 'l':
            action = TRAP_OAM_ACTION_LINK_FAULT_DETECT;
            break;
        default:
            diag_util_printf("User config: action Error!\n");
            return CPARSER_NOT_OK;
    }   /* switch (TOKEN_CHAR(5, 0)) */

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmCcmAct_set(unit, *level_ptr, action), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_cfm_ccm_level_drop_trap_forward_link_fault_detection */
#endif  /* CMD_TRAP_SET_CFM_CCM_LEVEL_DROP_TRAP_FORWARD_LINK_FAULT_DETECTION */

#ifdef CMD_TRAP_GET_CFM_ETH_DM_LEVEL_ACTION
/* trap get cfm eth-dm <UINT:level> action */
cparser_result_t cparser_cmd_trap_get_cfm_eth_dm_level_action(cparser_context_t *context,
    uint32_t *level_ptr)
{
    uint32          unit = 0;
    int32           ret;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((level_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_trap_cfmEthDmAct_get(unit, *level_ptr, &action);
    if (ret != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tCFM ETH-DM frame with MD level %d action: ",
                      *level_ptr);
    if (ACTION_DROP == action)
        diag_util_mprintf("Drop\n");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("Trap\n");
    else if (ACTION_FORWARD == action)
        diag_util_mprintf("Forward\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_cfm_eth_dm_level_action */
#endif  /* CMD_TRAP_GET_CFM_ETH_DM_LEVEL_ACTION */

#ifdef CMD_TRAP_SET_CFM_ETH_DM_LEVEL_ACTION_DROP_TRAP_FORWARD
/* trap set cfm eth-dm <UINT:level> action ( drop | trap | forward ) */
cparser_result_t cparser_cmd_trap_set_cfm_eth_dm_level_action_drop_trap_forward(cparser_context_t *context,
    uint32_t *level_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((level_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    switch (TOKEN_CHAR(6, 0))
    {
        case 'd':
            action = ACTION_DROP;
            break;
        case 't':
            action = ACTION_TRAP2CPU;
            break;
        case 'f':
            action = ACTION_FORWARD;
            break;
        default:
            diag_util_printf("User config: action Error!\n");
            return CPARSER_NOT_OK;
    }   /* switch (TOKEN_CHAR(5, 0)) */

    DIAG_UTIL_ERR_CHK(rtk_trap_cfmEthDmAct_set(unit, *level_ptr, action), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_cfm_eth_dm_level_action_drop_trap_forward */
#endif  /* CMD_TRAP_SET_CFM_ETH_DM_LEVEL_ACTION_DROP_TRAP_FORWARD */

#ifdef CMD_TRAP_GET_OAM_LOOPBACK_CTRL_PORT_ALL_PAR
/*
 * trap get oam-loopback-ctrl ( <PORT_LIST:port> | all ) par
 */
cparser_result_t
cparser_cmd_trap_get_oam_loopback_ctrl_port_all_par(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t      port = 0;
    rtk_trap_oam_action_t    action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        ret = rtk_trap_portOamLoopbackParAction_get(unit, port, &action);
        if (ret!= RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("\tPort %u:\n", port);
        if( ACTION_DROP == action )
            diag_util_mprintf("\t\tOAM parser action: Drop\n");
        else if( ACTION_FORWARD == action )
            diag_util_mprintf("\t\tOAM parser action: Forward\n");
        else if( TRAP_OAM_ACTION_LOOPBACK == action )
            diag_util_mprintf("\t\tOAM parser action: Loopback\n");
        else if( ACTION_TRAP2CPU == action )
            diag_util_mprintf("\t\tOAM parser action: Trap\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_oam_loopback_ctrl_port_all_par */
#endif  /* CMD_TRAP_GET_OAM_LOOPBACK_CTRL_PORT_ALL_PAR */

#ifdef CMD_TRAP_SET_OAM_LOOPBACK_CTRL_PORT_ALL_PAR_DROP_FORWARD_LOOPBACK_TRAP
/*
 * trap set oam-loopback-ctrl ( <PORT_LIST:port> | all ) par ( drop | forward | loopback | trap )
 */
cparser_result_t
cparser_cmd_trap_set_oam_loopback_ctrl_port_all_par_drop_forward_loopback_trap(
        cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trap_oam_action_t   parAction;
    diag_portlist_t         portlist;
    rtk_port_t              port = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('d' == TOKEN_CHAR(5, 0))
        parAction = TRAP_OAM_ACTION_DROP;
    else if('f' == TOKEN_CHAR(5, 0))
        parAction = TRAP_OAM_ACTION_FORWARD;
    else if('l' == TOKEN_CHAR(5, 0))
        parAction = TRAP_OAM_ACTION_LOOPBACK;
    else if('t' == TOKEN_CHAR(5, 0))
        parAction = TRAP_OAM_ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: parAction Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_trap_portOamLoopbackParAction_set(unit, port, parAction), ret);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_oam_loopback_ctrl_port_all_par_drop_forward_loopback_trap */
#endif

#ifdef CMD_TRAP_GET_IP6_HOP_LIMIT_EXCEED_HDR_ERR_HOP_BY_HOP_ACTION
/*
 * trap get ip6 ( hop-limit-exceed | hdr-err | hop-by-hop ) action
 */
cparser_result_t
cparser_cmd_trap_get_ip6_hop_limit_exceed_hdr_err_hop_by_hop_action(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if (0 == osal_strcmp(TOKEN_STR(3), "hop-limit-exceed"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hop-by-hop"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HOP_BY_HOP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    ret = rtk_trap_routeExceptionAction_get(unit, type, &action);
    if (ret!= RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tIPv6 %s action: ", TOKEN_STR(3));
    if (ACTION_DROP == action)
        diag_util_mprintf("Drop\n");
    else if (ACTION_FORWARD == action)
        diag_util_mprintf("Forward\n");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("Trap\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_ip6_hop_limit_exceed_hdr_err_hop_by_hop_action */
#endif

#ifdef CMD_TRAP_SET_IP6_HOP_LIMIT_EXCEED_HDR_ERR_HOP_BY_HOP_DROP_L2_FORWARD_TRAP
/*
 * trap set ip6 ( hop-limit-exceed | hdr-err | hop-by-hop ) ( drop | l2-forward | trap )
 */
cparser_result_t
cparser_cmd_trap_set_ip6_hop_limit_exceed_hdr_err_hop_by_hop_drop_l2_forward_trap(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    act;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if (0 == osal_strcmp(TOKEN_STR(3), "hop-limit-exceed"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hop-by-hop"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HOP_BY_HOP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(4, 0))
        act = ACTION_DROP;
    else if('f' == TOKEN_CHAR(4, 0))
        act = ACTION_FORWARD;
    else if('t' == TOKEN_CHAR(4, 0))
        act = ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: trap action Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_routeExceptionAction_set(unit, type, act), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_ip6_hop_limit_exceed_hdr_err_hop_by_hop_drop_l2_forward_trap */
#endif

#ifdef CMD_TRAP_GET_IP6_HOP_LIMIT_EXCEED_HDR_ERR_HOP_BY_HOP_PRIORITY
/*
 * trap get ip6 ( hop-limit-exceed | hdr-err | hop-by-hop ) priority
 */
cparser_result_t
cparser_cmd_trap_get_ip6_hop_limit_exceed_hdr_err_hop_by_hop_priority(
        cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if (0 == osal_strcmp(TOKEN_STR(3), "hop-limit-exceed"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hop-by-hop"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HOP_BY_HOP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    ret = rtk_trap_routeExceptionPri_get(unit, type, &pri);
    if (ret!= RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tIPv6 %s priority: %d", TOKEN_STR(3), pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_ip6_hop_limit_exceed_hdr_err_hop_by_hop_priority */
#endif

#ifdef CMD_TRAP_SET_IP6_HOP_LIMIT_EXCEED_HDR_ERR_HOP_BY_HOP_PRIORITY_PRIORITY
/*
 * trap set ip6 ( hop-limit-exceed | hdr-err | hop-by-hop ) priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_trap_set_ip6_hop_limit_exceed_hdr_err_hop_by_hop_priority_priority(
        cparser_context_t *context, uint32_t *priority_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if (0 == osal_strcmp(TOKEN_STR(3), "hop-limit-exceed"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hop-by-hop"))
        type = ROUTE_EXCEPTION_TYPE_IP6_HOP_BY_HOP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_routeExceptionPri_set(unit, type, *priority_ptr), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_ip6_hop_limit_exceed_hdr_err_hop_by_hop_priority_priority */
#endif

#ifdef CMD_TRAP_GET_IP_TTL_EXCEED_HDR_ERR_HDR_WITH_OPTION_ACTION
/*
 * trap get ip ( ttl-exceed | hdr-err | hdr-with-option ) action
 */
cparser_result_t
cparser_cmd_trap_get_ip_ttl_exceed_hdr_err_hdr_with_option_action(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if (0 == osal_strcmp(TOKEN_STR(3), "ttl-exceed"))
        type = ROUTE_EXCEPTION_TYPE_TTL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-with-option"))
        type = ROUTE_EXCEPTION_TYPE_WITH_OPT;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    ret = rtk_trap_routeExceptionAction_get(unit, type, &action);
    if (ret!= RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tIPv4 %s action: ", TOKEN_STR(3));
    if (ACTION_DROP == action)
        diag_util_mprintf("Drop\n");
    else if (ACTION_FORWARD == action)
        diag_util_mprintf("Forward\n");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("Trap\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_ip_ttl_exceed_hdr_err_hdr_with_option_action */
#endif

#ifdef CMD_TRAP_SET_IP_TTL_EXCEED_HDR_ERR_HDR_WITH_OPTION_DROP_L2_FORWARD_TRAP
/*
 * trap set ip ( ttl-exceed | hdr-err | hdr-with-option ) ( drop | l2-forward | trap )
 */
cparser_result_t
cparser_cmd_trap_set_ip_ttl_exceed_hdr_err_hdr_with_option_drop_l2_forward_trap(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    act;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if (0 == osal_strcmp(TOKEN_STR(3), "ttl-exceed"))
        type = ROUTE_EXCEPTION_TYPE_TTL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-with-option"))
        type = ROUTE_EXCEPTION_TYPE_WITH_OPT;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(4, 0))
        act = ACTION_DROP;
    else if('f' == TOKEN_CHAR(4, 0))
        act = ACTION_FORWARD;
    else if('t' == TOKEN_CHAR(4, 0))
        act = ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: trap action Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_routeExceptionAction_set(unit, type, act), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_ip_ttl_exceed_hdr_err_hdr_with_option_drop_l2_forward_trap */
#endif

#ifdef CMD_TRAP_GET_IP_TTL_EXCEED_HDR_ERR_HDR_WITH_OPTION_PRIORITY
/*
 * trap get ip ( ttl-exceed | hdr-err | hdr-with-option ) priority
 */
cparser_result_t
cparser_cmd_trap_get_ip_ttl_exceed_hdr_err_hdr_with_option_priority(
        cparser_context_t *context)
{

    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if (0 == osal_strcmp(TOKEN_STR(3), "ttl-exceed"))
        type = ROUTE_EXCEPTION_TYPE_TTL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-with-option"))
        type = ROUTE_EXCEPTION_TYPE_WITH_OPT;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    ret = rtk_trap_routeExceptionPri_get(unit, type, &pri);
    if (ret!= RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tIPv4 %s priority: %d", TOKEN_STR(3), pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_ip_ttl_exceed_hdr_err_hdr_with_option_priority */
#endif

#ifdef CMD_TRAP_SET_IP_TTL_EXCEED_HDR_ERR_HDR_WITH_OPTION_PRIORITY_PRIORITY
/*
 * trap set ip ( ttl-exceed | hdr-err | hdr-with-option ) priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_trap_set_ip_ttl_exceed_hdr_err_hdr_with_option_priority_priority(
        cparser_context_t *context, uint32_t *priority_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if (0 == osal_strcmp(TOKEN_STR(3), "ttl-exceed"))
        type = ROUTE_EXCEPTION_TYPE_TTL_EXCEED;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-err"))
        type = ROUTE_EXCEPTION_TYPE_HDR_ERR;
    else if (0 == osal_strcmp(TOKEN_STR(3), "hdr-with-option"))
        type = ROUTE_EXCEPTION_TYPE_WITH_OPT;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_routeExceptionPri_set(unit, type, *priority_ptr), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_ip_ttl_exceed_hdr_err_hdr_with_option_priority_priority */
#endif

#ifdef CMD_TRAP_GET_GW_MAC_ERR_ACTION
/*
 * trap get gw-mac-err action
 */
cparser_result_t
cparser_cmd_trap_get_gw_mac_err_action(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    type = ROUTE_EXCEPTION_TYPE_GW_MAC_ERR;

    ret = rtk_trap_routeExceptionAction_get(unit, type, &action);
    if (ret!= RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tIPv4 %s action: ", TOKEN_STR(3));
    if (ACTION_DROP == action)
        diag_util_mprintf("Drop\n");
    else if (ACTION_FORWARD == action)
        diag_util_mprintf("Forward\n");
    else if (ACTION_TRAP2CPU == action)
        diag_util_mprintf("Trap\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_gw_mac_err_action */
#endif

#ifdef CMD_TRAP_SET_GW_MAC_ERR_DROP_L2_FORWARD_TRAP
/*
 * trap set gw-mac-err ( drop | l2-forward | trap )
 */
cparser_result_t
cparser_cmd_trap_set_gw_mac_err_drop_l2_forward_trap(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    act;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    type = ROUTE_EXCEPTION_TYPE_GW_MAC_ERR;

    if('d' == TOKEN_CHAR(3, 0))
        act = ACTION_DROP;
    else if('l' == TOKEN_CHAR(3, 0))
        act = ACTION_FORWARD;
    else if('t' == TOKEN_CHAR(3, 0))
        act = ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: trap action Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_routeExceptionAction_set(unit, type, act), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_gw_mac_err_drop_l2_forward_trap */
#endif

#ifdef CMD_TRAP_GET_GW_MAC_ERR_PRIORITY
/*
 * trap get gw-mac-err priority
 */
cparser_result_t
cparser_cmd_trap_get_gw_mac_err_priority(
        cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    type = ROUTE_EXCEPTION_TYPE_GW_MAC_ERR;

    ret = rtk_trap_routeExceptionPri_get(unit, type, &pri);
    if (ret!= RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tGateway MAC error priority: %d", pri);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_gw_mac_err_priority */
#endif

#ifdef CMD_TRAP_SET_GW_MAC_ERR_PRIORITY_PRIORITY
/*
 * trap set gw-mac-err priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_trap_set_gw_mac_err_priority_priority(
        cparser_context_t *context, uint32_t *priority_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_trap_routeExceptionType_t type;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    type = ROUTE_EXCEPTION_TYPE_GW_MAC_ERR;

    DIAG_UTIL_ERR_CHK(rtk_trap_routeExceptionPri_set(unit, type, *priority_ptr), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_gw_mac_err_priority_priority */
#endif

#ifdef CMD_TRAP_GET_IGMP_MLD
/*
 * trap get ( igmp | mld )
 */
cparser_result_t
cparser_cmd_trap_get_igmp_mld(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_trap_mgmtType_t type;
    rtk_action_t    act;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if('i' == TOKEN_CHAR(2, 0))
        type = MGMT_TYPE_IGMP;
    else if('m' == TOKEN_CHAR(2, 0))
        type = MGMT_TYPE_MLD;
    else
    {
        diag_util_printf("User config: trap type Error!\n");
        return CPARSER_NOT_OK;
    }

    ret = rtk_trap_mgmtFrameAction_get(unit, type, &act);
    if (ret!= RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(2, 0))
        diag_util_mprintf("\tIGMP: ");
    else
        diag_util_mprintf("\tMLD: ");

    if (ACTION_DROP == act)
        diag_util_mprintf("Drop\n");
    else if (ACTION_FORWARD == act)
        diag_util_mprintf("Forward\n");
    else if (ACTION_TRAP2CPU == act)
        diag_util_mprintf("Trap\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_igmp_mld */
#endif

#ifdef CMD_TRAP_SET_IGMP_MLD_FORWARD_TRAP_TO_CPU
/*
 * trap set ( igmp | mld ) ( forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_trap_set_igmp_mld_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_trap_mgmtType_t type;
    rtk_action_t    act;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('i' == TOKEN_CHAR(2, 0))
        type = MGMT_TYPE_IGMP;
    else if('m' == TOKEN_CHAR(2, 0))
        type = MGMT_TYPE_MLD;
    else
    {
        diag_util_printf("User config: trap type Error!\n");
        return CPARSER_NOT_OK;
    }

    if('f' == TOKEN_CHAR(3, 0))
        act = ACTION_FORWARD;
    else if('t' == TOKEN_CHAR(3, 0))
        act = ACTION_TRAP2CPU;
    else
    {
        diag_util_printf("User config: trap action Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameAction_set(unit, type, act), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_igmp_mld */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_BYPASS_STP_DISABLE_ENABLE
/*
 * trap set rma l2-user-define <UINT:index> bypass-stp ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_l2_user_define_index_bypass_stp_disable_enable(
        cparser_context_t *context, uint32_t *index_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    en;
    uint32          bypassStpFieldIdx[] = {BYPASS_STP_TYPE_USER_DEF_0,
                                           BYPASS_STP_TYPE_USER_DEF_1};

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        en = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        en = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_set(unit, bypassStpFieldIdx[*index_ptr], en), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_l2_user_define_index_bypass_stp_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_LEARN_DISABLE_ENABLE
/*
 * trap set rma l2-user-define <UINT:index> learn ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_l2_user_define_index_learn_disable_enable(
        cparser_context_t *context, uint32_t *index_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    en;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        en = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        en = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_userDefineRmaLearningEnable_set(unit, *index_ptr, en), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_l2_user_define_index_learn_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_0X_SLOW_PROTO_BYPASS_STP_DISABLE_ENABLE
/*
 * trap set rma layer2 ( rma-tail-0X | slow-proto ) bypass-stp ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_rma_tail_0X_slow_proto_bypass_stp_disable_enable(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_trap_bypassStpType_t    type;
    rtk_enable_t    en;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('r' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_RMA_0X;
    else if('s' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_SLOW_PROTO;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
        en = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        en = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_set(unit, type, en), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_0x_slow_proto_bypass_stp_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_PTP_LLDP_BYPASS_STP_DISABLE_ENABLE
/*
 * trap set rma layer2 ( ptp | lldp ) bypass-stp ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_ptp_lldp_bypass_stp_disable_enable(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassStpType_t    type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('p' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_PTP;
    else if('l' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_LLDP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_set(unit, type, enable), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_layer2_ptp_lldp_bypass_stp_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_LEARN_DISABLE_ENABLE
/*
 * trap set rma layer2 <UINT:rma_tail> learn ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_rma_tail_learn_disable_enable(
        cparser_context_t *context, uint32_t *rma_tail_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_mac_t       mac;
    rtk_enable_t    en;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    mac.octet[0] = 0x01;
    mac.octet[1] = 0x80;
    mac.octet[2] = 0xc2;
    mac.octet[3] = 0x00;
    mac.octet[4] = 0x00;
    mac.octet[5] = *rma_tail_ptr;

    if (mac.octet[5] > 0x2f)
    {
        diag_util_printf("\nInput error: the RMA address is out of range\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
        en = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        en = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_rmaLearningEnable_set(unit, &mac, en), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_learn_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_TAIL_RMA_TAIL_END_LEARN_DISABLE_ENABLE
/*
 * trap set rma layer2 <UINT:rma_tail> <UINT:rma_tail_end> learn ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_rma_tail_rma_tail_end_learn_disable_enable(
        cparser_context_t *context, uint32_t *rma_tail_ptr, uint32_t *rma_tail_end_ptr)
{
    uint32          unit = 0;
    uint32          ramTail = 0, ramTailEnd, i;
    int32           ret = RT_ERR_FAILED;
    rtk_mac_t       mac;
    rtk_enable_t    en;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    mac.octet[0] = 0x01;
    mac.octet[1] = 0x80;
    mac.octet[2] = 0xc2;
    mac.octet[3] = 0x00;
    mac.octet[4] = 0x00;
    mac.octet[5] = 0x00;
    
    ramTail = *rma_tail_ptr;
    ramTailEnd = *rma_tail_end_ptr;

    if (ramTail > 0x2f || ramTailEnd > 0x2f)
    {
        diag_util_printf("\nInput error: the RMA address is out of range\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(7, 0))
        en = ENABLED;
    else if('d' == TOKEN_CHAR(7, 0))
        en = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    for (i = ramTail; i <= ramTailEnd; i++)
    {
        mac.octet[5] = i;
        DIAG_UTIL_ERR_CHK(rtk_trap_rmaLearningEnable_set(unit, &mac, en), ret);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_layer2_rma_tail_rma_tail_end_learn_disable_enable */
#endif

#ifdef CMD_TRAP_SET_RMA_PORT_RMA_LLDP_PTP_LEARN_DISABLE_ENABLE
/*
 * trap set rma port-rma ( lldp | ptp ) learn ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_port_rma_lldp_ptp_learn_disable_enable(
        cparser_context_t *context, uint32_t *rma_tail_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_trap_mgmtType_t type;
    rtk_enable_t    en;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('l' == TOKEN_CHAR(4, 0))
        type = MGMT_TYPE_LLDP;
    else if('p' == TOKEN_CHAR(4, 0))
        type = MGMT_TYPE_PTP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
        en = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        en = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameLearningEnable_set(unit, type, en), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_port_rma_lldp_ptp_learn_disable_enable */
#endif

#ifdef CMD_TRAP_DUMP_RMA_PORT_RMA_BPDU_PORT_ALL
/*
 * trap dump rma port-rma bpdu ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_trap_dump_rma_port_rma_bpdu_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    rtk_trap_mgmtType_t type = MGMT_TYPE_RIP;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        rma_action = ACTION_FORWARD;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    type = MGMT_TYPE_BPDU;

    diag_util_mprintf("+----+-------------+\n");
    diag_util_mprintf("Port | Action      |\n");
    diag_util_mprintf("+----+-------------+\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("%5d", port);

        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_get(unit, port, type, &rma_action), ret);
        if (RMA_ACTION_FORWARD == rma_action)
        {
            diag_util_printf("  forward      ");
        }
        else if (RMA_ACTION_DROP == rma_action)
        {
            diag_util_printf("  drop         ");
        }
        else if (RMA_ACTION_TRAP2CPU == rma_action)
        {
            diag_util_printf("  trap-to-cpu  ");
        }
        else if (RMA_ACTION_COPY2CPU == rma_action)
        {
            diag_util_printf("  copy-to-cpu  ");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("\n");
    }


    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_rma_port_rma_bpdu_port_all */
#endif

#ifdef CMD_TRAP_DUMP_RMA_PORT_RMA_LLDP_PORT_ALL
/*
 * trap dump rma port-rma lldp ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_trap_dump_rma_port_rma_lldp_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    rtk_trap_mgmtType_t type = MGMT_TYPE_RIP;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        rma_action = ACTION_FORWARD;
    rtk_enable_t        enable;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    type = MGMT_TYPE_LLDP;

    diag_util_mprintf("+----+-------------+----------+\n");
    diag_util_mprintf("Port | Action      | Learn    |\n");
    diag_util_mprintf("+----+-------------+----------+\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("%5d", port);

        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_get(unit, port, type, &rma_action), ret);
        if (RMA_ACTION_FORWARD == rma_action)
        {
            diag_util_printf("  forward      ");
        }
        else if (RMA_ACTION_DROP == rma_action)
        {
            diag_util_printf("  drop         ");
        }
        else if (RMA_ACTION_TRAP2CPU == rma_action)
        {
            diag_util_printf("  trap-to-cpu  ");
        }
        else if (RMA_ACTION_COPY2CPU == rma_action)
        {
            diag_util_printf("  copy-to-cpu  ");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameLearningEnable_get(unit, type, &enable), ret);
        if(ENABLED == enable)
            diag_util_printf("  enabled\n");
        else if(DISABLED == enable)
            diag_util_printf("  disabled\n");
        else
            diag_util_printf("  disabled\n");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_rma_port_rma_lldp_port_all */
#endif

#ifdef CMD_TRAP_DUMP_RMA_PORT_RMA_PTP_PORT_ALL
/*
 * trap dump rma port-rma ptp ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_trap_dump_rma_port_rma_ptp_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    rtk_trap_mgmtType_t type = MGMT_TYPE_RIP;
    int32               ret = RT_ERR_FAILED;
    rtk_action_t        rma_action = ACTION_FORWARD;
    rtk_enable_t        enable;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    type = MGMT_TYPE_PTP;

    diag_util_mprintf("+----+-------------+----------+\n");
    diag_util_mprintf("Port | Action      | Learn    |\n");
    diag_util_mprintf("+----+-------------+----------+\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("%5d", port);

        DIAG_UTIL_ERR_CHK(rtk_trap_portMgmtFrameAction_get(unit, port, type, &rma_action), ret);
        if (RMA_ACTION_FORWARD == rma_action)
        {
            diag_util_printf("  forward      ");
        }
        else if (RMA_ACTION_DROP == rma_action)
        {
            diag_util_printf("  drop         ");
        }
        else if (RMA_ACTION_TRAP2CPU == rma_action)
        {
            diag_util_printf("  trap-to-cpu  ");
        }
        else if (RMA_ACTION_COPY2CPU == rma_action)
        {
            diag_util_printf("  copy-to-cpu  ");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }

        DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameLearningEnable_get(unit, type, &enable), ret);
        if(ENABLED == enable)
            diag_util_printf("  enabled\n");
        else if(DISABLED == enable)
            diag_util_printf("  disabled\n");
        else
            diag_util_printf("  disabled\n");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
} /* end of cparser_cmd_trap_dump_rma_port_rma_ptp_port_all */
#endif

#ifdef CMD_TRAP_GET_MANAGEMENT_VLAN_STATE
/*
 * trap get management-vlan state ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_get_management_vlan_state(
        cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameMgmtVlanEnable_get(unit, &enable), ret);
    diag_util_printf("Management VLAN state: ");
    if(ENABLED == enable)
        diag_util_printf("enabled\n");
    else if(DISABLED == enable)
        diag_util_printf("disabled\n");
    else
        diag_util_printf("disabled\n");

    return CPARSER_OK;
} /* end of cparser_cmd_trap_get_management_vlan_state */
#endif

#ifdef CMD_TRAP_SET_MANAGEMENT_VLAN_STATE_DISABLE_ENABLE
/*
 * trap set management-vlan state ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_management_vlan_state_disable_enable(
        cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    en;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(4, 0))
        en = ENABLED;
    else if('d' == TOKEN_CHAR(4, 0))
        en = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_mgmtFrameMgmtVlanEnable_set(unit, en), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_trap_set_management_vlan_state_disable_enable */
#endif

#ifdef CMD_TRAP_GET_RMA_LAYER2_PTP_LLDP_BYPASS_VLAN
/*
 * trap get rma layer2 ( ptp | lldp ) bypass-vlan
 */
cparser_result_t
cparser_cmd_trap_get_rma_layer2_ptp_lldp_bypass_vlan(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassVlanType_t   type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if('p' == TOKEN_CHAR(4, 0))
        type = BYPASS_VLAN_TYPE_PTP;
    else if('l' == TOKEN_CHAR(4, 0))
        type = BYPASS_VLAN_TYPE_LLDP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassVlan_get(unit, type, &enable), ret);

    diag_util_mprintf("RMA layer 2 %s bypass VLAN: %s\n",
        TOKEN_STR(4), (ENABLED == enable)?DIAG_STR_ENABLE:DIAG_STR_DISABLE);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_rma_layer2_ptp_lldp_bypass_vlan */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_PTP_LLDP_BYPASS_VLAN_DISABLE_ENABLE
/*
 * trap set rma layer2 ( ptp | lldp ) bypass-vlan ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_ptp_lldp_bypass_vlan_disable_enable(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassVlanType_t   type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('p' == TOKEN_CHAR(4, 0))
        type = BYPASS_VLAN_TYPE_PTP;
    else if('l' == TOKEN_CHAR(4, 0))
        type = BYPASS_VLAN_TYPE_LLDP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassVlan_set(unit, type, enable), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_layer2_ptp_lldp_bypass_vlan_disable_enable */
#endif

#ifdef CMD_TRAP_GET_RMA_LAYER2_RMA_00_RMA_02_RMA_0E_RMA_0X_BYPASS_VLAN
/*
 * trap get rma layer2 ( rma-00 | rma-02 | rma-0E | rma-0X ) bypass-vlan
 */
cparser_result_t
cparser_cmd_trap_get_rma_layer2_rma_00_rma_02_rma_0E_rma_0X_bypass_vlan(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassVlanType_t   type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (TOKEN_CHAR(4, 5))
    {
        case '0':
            type = BYPASS_VLAN_TYPE_RMA_00;
            break;
        case '2':
            type = BYPASS_VLAN_TYPE_RMA_02;
            break;
        case 'E':
            type = BYPASS_VLAN_TYPE_RMA_0E;
            break;
        case 'X':
            type = BYPASS_VLAN_TYPE_RMA_0X;
            break;
        default:
            diag_util_printf("User config: type Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassVlan_get(unit, type, &enable), ret);

    diag_util_mprintf("RMA layer 2 %s bypass VLAN: %s\n",
        TOKEN_STR(4), (ENABLED == enable)?DIAG_STR_ENABLE:DIAG_STR_DISABLE);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_rma_layer2_rma_00_rma_02_rma_0e_rma_0x_bypass_vlan */
#endif

#ifdef CMD_TRAP_SET_RMA_LAYER2_RMA_00_RMA_02_RMA_0E_RMA_0X_BYPASS_VLAN_DISABLE_ENABLE
/*
 * trap set rma layer2 ( rma-00 | rma-02 | rma-0E | rma-0X ) bypass-vlan ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_layer2_rma_00_rma_02_rma_0E_rma_0X_bypass_vlan_disable_enable(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassVlanType_t   type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(4, 5))
    {
        case '0':
            type = BYPASS_VLAN_TYPE_RMA_00;
            break;
        case '2':
            type = BYPASS_VLAN_TYPE_RMA_02;
            break;
        case 'E':
            type = BYPASS_VLAN_TYPE_RMA_0E;
            break;
        case 'X':
            type = BYPASS_VLAN_TYPE_RMA_0X;
            break;
        default:
            diag_util_printf("User config: type Error!\n");
            return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassVlan_set(unit, type, enable), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_layer2_rma_00_rma_02_rma_0E_rma_0X_bypass_vlan_disable_enable */
#endif

#ifdef CMD_TRAP_GET_RMA_L2_USER_DEFINE_INDEX_BYPASS_VLAN
/*
 * trap get rma l2-user-define <UINT:index> bypass-vlan
 */
cparser_result_t
cparser_cmd_trap_get_rma_l2_user_define_index_bypass_vlan(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_enable_t                enable;
    uint32                      bypassVlanList[] = {
                                    BYPASS_VLAN_TYPE_USER_DEF_0,
                                    BYPASS_VLAN_TYPE_USER_DEF_1,
                                    BYPASS_VLAN_TYPE_USER_DEF_2,
                                    BYPASS_VLAN_TYPE_USER_DEF_3,
                                    BYPASS_VLAN_TYPE_USER_DEF_4,
                                    BYPASS_VLAN_TYPE_USER_DEF_5};

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassVlan_get(unit, bypassVlanList[*index_ptr], &enable), ret);

    diag_util_mprintf("RMA layer 2 user define %d bypass VLAN: %s\n",
        *index_ptr, (ENABLED == enable)?DIAG_STR_ENABLE:DIAG_STR_DISABLE);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_rma_l2_user_define_bypass_vlan */
#endif

#ifdef CMD_TRAP_SET_RMA_L2_USER_DEFINE_INDEX_BYPASS_VLAN_DISABLE_ENABLE
/*
 * trap set rma l2-user-define <UINT:index> bypass-vlan ( disable | enable )
 */
cparser_result_t
cparser_cmd_trap_set_rma_l2_user_define_index_bypass_vlan_disable_enable(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassVlanType_t   type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (*index_ptr)
    {
        case 0:
            type = BYPASS_VLAN_TYPE_USER_DEF_0;
            break;
        case 1:
            type = BYPASS_VLAN_TYPE_USER_DEF_1;
            break;
        default:
            diag_util_printf("User config: index Error!\n");
            return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: state Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassVlan_set(unit, type, enable), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_set_rma_l2_user_define_index_bypass_vlan_disable_enable */
#endif

#ifdef CMD_TRAP_GET_RMA_LAYER2_PTP_LLDP_BYPASS_STP
/*
 * trap get rma layer2 ( ptp | lldp ) bypass-stp
 */
cparser_result_t
cparser_cmd_trap_get_rma_layer2_ptp_lldp_bypass_stp(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassVlanType_t   type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if('p' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_PTP;
    else if('l' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_LLDP;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_get(unit, type, &enable), ret);

    diag_util_mprintf("RMA layer 2 %s bypass STP: %s\n",
        TOKEN_STR(4), (ENABLED == enable)?DIAG_STR_ENABLE:DIAG_STR_DISABLE);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_rma_layer2_ptp_lldp_bypass_stp */
#endif

#ifdef CMD_TRAP_GET_RMA_LAYER2_RMA_TAIL_0X_SLOW_PROTO_BYPASS_STP
/*
 * trap get rma layer2 ( rma-tail-0X | slow-proto ) bypass-stp
 */
cparser_result_t
cparser_cmd_trap_get_rma_layer2_rma_tail_0X_slow_proto_bypass_stp(
    cparser_context_t *context)
{
    uint32                      unit;
    int32                       ret;
    rtk_trap_bypassVlanType_t   type;
    rtk_enable_t                enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if('r' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_RMA_0X;
    else if('s' == TOKEN_CHAR(4, 0))
        type = BYPASS_STP_TYPE_SLOW_PROTO;
    else
    {
        diag_util_printf("User config: type Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_get(unit, type, &enable), ret);

    diag_util_mprintf("RMA layer 2 %s bypass STP: %s\n",
        TOKEN_STR(4), (ENABLED == enable)?DIAG_STR_ENABLE:DIAG_STR_DISABLE);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_rma_layer2_rma_tail_0x_slow_proto_bypass_stp */
#endif

#ifdef CMD_TRAP_GET_RMA_L2_USER_DEFINE_INDEX_BYPASS_STP
/*
 * trap get rma l2-user-define <UINT:index> bypass-stp
 */
cparser_result_t
cparser_cmd_trap_get_rma_l2_user_define_index_bypass_stp(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_enable_t                enable;
    uint32                      bypassStpList[] = {
                                    BYPASS_STP_TYPE_USER_DEF_0,
                                    BYPASS_STP_TYPE_USER_DEF_1,
                                    BYPASS_STP_TYPE_USER_DEF_2,
                                    BYPASS_STP_TYPE_USER_DEF_3,
                                    BYPASS_STP_TYPE_USER_DEF_4,
                                    BYPASS_STP_TYPE_USER_DEF_5};

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trap_bypassStp_get(unit, bypassStpList[*index_ptr], &enable), ret);

    diag_util_mprintf("RMA layer 2 user define %d bypass STP: %s\n",
        bypassStpList[*index_ptr], (ENABLED == enable)?DIAG_STR_ENABLE:DIAG_STR_DISABLE);

    return CPARSER_OK;
}   /* end of cparser_cmd_trap_get_rma_l2_user_define_bypass_stp */
#endif

