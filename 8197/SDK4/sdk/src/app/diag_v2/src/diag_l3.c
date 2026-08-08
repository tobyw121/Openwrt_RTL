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
 * $Revision: 41809 $
 * $Date: 2013-08-05 16:12:02 +0800 (Mon, 05 Aug 2013) $
 *
 * Purpose : Definition those L3 command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
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
#include <rtk/l2.h>
#include <rtk/l3.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_L3_DUMP
/*
 * l3 dump
 */
cparser_result_t cparser_cmd_l3_dump(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_action_t                  action;
    rtk_enable_t                 enable;
    rtk_pri_t                       pri;
    uint32                           dpValue;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("TTL Expire Ip Unicast : \n");

    ret = rtk_l3_ttlExpireAction_get(unit, TTL_EXPIRE_UCAST, &action);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tTTL Expire Action    : ");
        if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapPriEnable_get(unit, TTL_EXPIRE_UCAST, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapPri_get(unit, TTL_EXPIRE_UCAST, &pri), ret);
        diag_util_mprintf("\tTrap Priority        : ENABLE(%u)\n", pri);
    }
    else
    {
        diag_util_mprintf("\tTrap Priority        : DISABLE  \n");
    }

    DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapDPEnable_get(unit, TTL_EXPIRE_UCAST, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapDP_get(unit, TTL_EXPIRE_UCAST, &dpValue), ret);
        diag_util_mprintf("\tTrap Drop Precedence : ENABLE(%u)\n", dpValue);
    }
    else
    {
        diag_util_mprintf("\tTrap Drop Precedence : DISABLE  \n");
    }

    ret = rtk_l3_ttlExpireAddCPUTagEnable_get(unit, TTL_EXPIRE_UCAST, &enable);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        if(ENABLED == enable)
            diag_util_mprintf("\tTrap Insert Cpu Tag  : ENABLE\n");
        else
            diag_util_mprintf("\tTrap Insert Cpu Tag  : DISABLE\n");
    }

    diag_util_mprintf("TTL Expire Ip Multicast :\n");

    ret = rtk_l3_ttlExpireAction_get(unit, TTL_EXPIRE_MCAST, &action);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tTTL Expire Action    : ");
        if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Tap-To-Cpu\n");
        }
        else if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapPriEnable_get(unit, TTL_EXPIRE_MCAST, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapPri_get(unit, TTL_EXPIRE_MCAST, &pri), ret);
        diag_util_mprintf("\tTrap Priority        : ENABLE(%u)\n", pri);
    }
    else
    {
        diag_util_mprintf("\tTrap Priority        : DISABLE  \n");
    }

    DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapDPEnable_get(unit, TTL_EXPIRE_MCAST, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapDP_get(unit, TTL_EXPIRE_MCAST, &dpValue), ret);
        diag_util_mprintf("\tTrap Drop Precedence : ENABLE(%u)\n", dpValue);
    }
    else
    {
        diag_util_mprintf("\tTrap Drop Precedence : DISABLE  \n");
    }

    ret = rtk_l3_ttlExpireAddCPUTagEnable_get(unit, TTL_EXPIRE_MCAST, &enable);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        if(ENABLED == enable)
            diag_util_mprintf("\tTrap Insert Cpu Tag  : ENABLE\n");
        else
            diag_util_mprintf("\tTrap Insert Cpu Tag  : DISABLE\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_TTL_EXPIRE_IPMCAST_IPUCAST_DROP_PRECEDENCE_VALUE
/*
 * l3 set ttl-expire ( ipmcast | ipucast ) drop-precedence  <UINT:value>
 */
cparser_result_t cparser_cmd_l3_set_ttl_expire_ipmcast_ipucast_drop_precedence_value(cparser_context_t *context,
uint32_t *value_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l3_ttlExpireType_t              type = TTL_EXPIRE_UCAST;

    DIAG_OM_GET_CHIP_ID(unit);
    RT_PARAM_CHK((*value_ptr > RTK_DROP_PRECEDENCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_PARAM_CHK();

    if('u' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_UCAST;
    }
    else if('m' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_MCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapDP_set(unit, type, *value_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_DUMP_ROUTE_SWITCH_MAC
/*
 * l3 dump route switch-mac
 */
cparser_result_t cparser_cmd_l3_dump_route_switch_mac(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32     maxIndex;
    uint32      index;
    uint8 macStr[16];
    rtk_mac_t mac;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_OM_GET_CHIP_CAPACITY(unit, maxIndex, max_num_of_route_switch_addr);

    for (index = 0; index < maxIndex; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_routeSwitchMacAddr_get(unit, index, &mac), ret);

        if ((mac.octet[0] != 0) || (mac.octet[1] != 0) || (mac.octet[2] != 0) ||
            (mac.octet[3] != 0) || (mac.octet[4] != 0) || (mac.octet[5] != 0))
        {
            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, mac.octet), ret);
            diag_util_mprintf("Index : %2d, ", index);
            diag_util_mprintf("Routing Switch MAC Address : %s\n", macStr);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_GET_ROUTE_SWITCH_MAC_INDEX
/*
 * l3 get route switch-mac <UINT:index>
 */
cparser_result_t cparser_cmd_l3_get_route_switch_mac_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint8 macStr[16];
    rtk_mac_t mac;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l3_routeSwitchMacAddr_get(unit, *index_ptr, &mac), ret);
    DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, mac.octet), ret);
    diag_util_printf("Routing Switch MAC Address : %s\n", macStr);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_ROUTE_SWITCH_MAC_INDEX_MAC
/*
 * l3 set route switch-mac <UINT:index> <MACADDR:mac>
 */
cparser_result_t cparser_cmd_l3_set_route_switch_mac_index_mac(cparser_context_t *context,
    uint32_t *index_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l3_routeSwitchMacAddr_set(unit, *index_ptr, (rtk_mac_t *)mac_ptr), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_TTL_EXPIRE_IPMCAST_IPUCAST_DROP_PRECEDENCE_STATE_DISABLE_ENABLE
/*
 * l3 set ttl-expire ( ipmcast | ipucast ) drop-precedence state ( disable | enable )
 */
cparser_result_t cparser_cmd_l3_set_ttl_expire_ipmcast_ipucast_drop_precedence_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l3_ttlExpireType_t              type = TTL_EXPIRE_UCAST;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('u' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_UCAST;
    }
    else if('m' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_MCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(6, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapDPEnable_set(unit, type, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapDPEnable_set(unit, type, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_TTL_EXPIRE_IPMCAST_IPUCAST_DROP_FORWARD_TRAP_TO_CPU
/*
 * l3 set ttl-expire ( ipmcast | ipucast ) ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_l3_set_ttl_expire_ipmcast_ipucast_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l3_ttlExpireType_t   type = TTL_EXPIRE_UCAST;
    rtk_action_t                 action = ACTION_DROP;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('u' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_UCAST;
    }
    else if('m' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_MCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_DROP;
    }
    else if('f' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_FORWARD;
    }
    else if('t' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireAction_set(unit, type, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_TTL_EXPIRE_IPMCAST_IPUCAST_INSERT_CPUTAG_STATE_DISABLE_ENABLE
/*
 * l3 set ttl-expire ( ipmcast | ipucast ) insert-cputag state ( disable | enable )
 */
cparser_result_t cparser_cmd_l3_set_ttl_expire_ipmcast_ipucast_insert_cputag_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l3_ttlExpireType_t   type = TTL_EXPIRE_UCAST;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('u' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_UCAST;
    }
    else if('m' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_MCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_CHAR(6, 0))
    {
        /* set invalid link down enable */
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireAddCPUTagEnable_set(unit, type, ENABLED), ret);
    }
    else if ('d' == TOKEN_CHAR(6, 0))
    {
        /* set invalid link down disable */
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireAddCPUTagEnable_set(unit, type, DISABLED), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_TTL_EXPIRE_IPMCAST_IPUCAST_PRIORITY_PRI
/*
 * l3 set ttl-expire ( ipmcast | ipucast ) priority <UINT:pri>
 */
cparser_result_t cparser_cmd_l3_set_ttl_expire_ipmcast_ipucast_priority_pri(cparser_context_t *context,
uint32_t *pri_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l3_ttlExpireType_t   type = TTL_EXPIRE_UCAST;

    DIAG_OM_GET_CHIP_ID(unit);
    RT_PARAM_CHK((pri_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*pri_ptr >= RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_PARAM_CHK();

    if('u' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_UCAST;
    }
    else if('m' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_MCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapPri_set(unit, type, *pri_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_TTL_EXPIRE_IPMCAST_IPUCAST_PRIORITY_STATE_DISABLE_ENABLE
/*
 * l3 set ttl-expire ( ipmcast | ipucast ) priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_l3_set_ttl_expire_ipmcast_ipucast_priority_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l3_ttlExpireType_t   type = TTL_EXPIRE_UCAST;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('u' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_UCAST;
    }
    else if('m' == TOKEN_CHAR(3, 2))
    {
        type = TTL_EXPIRE_MCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(6, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapPriEnable_set(unit, type, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_ttlExpireTrapPriEnable_set(unit, type, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_DUMP_ROUTE_ROUTE_TABLE
/*
 * l3 dump route route-table
 */
cparser_result_t cparser_cmd_l3_dump_route_route_table(cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    uint32 scan_idx = 0;
    uint32 total_entry = 0;
    rtk_l3_routeEntry_t entry;
    uint8 macStr[16];
    uint32 routeEntries;
    uint8  zero_mac[ETHER_ADDR_LEN]= {0};

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf("Index | Host MAC Address | Switch MAC Index\n");
        diag_util_mprintf("------+------------------+------------------\n");
    }
    else
    {
        diag_util_mprintf("Index | Host MAC Address \n");
        diag_util_mprintf("------+------------------\n");
    }

    DIAG_OM_GET_CHIP_CAPACITY(unit, routeEntries, max_num_of_route_host_addr);
    for (scan_idx=0; scan_idx < routeEntries; scan_idx++)
    {
        DIAG_UTIL_ERR_CHK(rtk_l3_routeEntry_get(unit, scan_idx, &entry), ret);

        if (memcmp(entry.hostMac.octet, zero_mac, ETHER_ADDR_LEN) != 0)
        {
            DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, entry.hostMac.octet), ret);

            if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
            {
#if defined(CONFIG_SDK_RTL8390)
                diag_util_mprintf("%5d | %16s | %16d \n", scan_idx, macStr, entry.swMac_idx);
#endif
            }
            else
                diag_util_mprintf("%5d | %16s \n", scan_idx, macStr);

            total_entry++;
        }
    }

    diag_util_mprintf("\nTotal Number Of Entries :%d\n", total_entry);
    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_GET_ROUTE_ROUTE_TABLE_INDEX
/*
 * l3 get route route-table <UINT:index>
 */
cparser_result_t cparser_cmd_l3_get_route_route_table_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_l3_routeEntry_t entry;
    uint8 macStr[16];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l3_routeEntry_get(unit, *index_ptr, &entry), ret);
    DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, entry.hostMac.octet), ret);

    diag_util_printf("Route Entry [%d]\n", *index_ptr);
    diag_util_printf("Host MAC Address : %s\n", macStr);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_printf("Switch MAC Index : %d\n", entry.swMac_idx);
    }
#endif

    return CPARSER_OK;
}
#endif

#ifdef CMD_L3_SET_ROUTE_ROUTE_TABLE_INDEX_HOST_MAC_MAC
/*
 * l3 set route route-table <UINT:index> host-mac <MACADDR:mac>
 */
cparser_result_t cparser_cmd_l3_set_route_route_table_index_host_mac_mac(cparser_context_t *context,
    uint32_t *index_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_l3_routeEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l3_routeEntry_get(unit, *index_ptr, &entry), ret);
    memcpy((void*)&entry.hostMac, (void*)mac_ptr, sizeof(cparser_macaddr_t));
    DIAG_UTIL_ERR_CHK(rtk_l3_routeEntry_set(unit, *index_ptr, &entry), ret);
    return CPARSER_OK;

}
#endif

#ifdef CMD_L3_SET_ROUTE_ROUTE_TABLE_INDEX_SWITCH_MAC_INDEX_SWITCH_MAC_INDEX
/*
 * l3 set route route-table <UINT:index> switch-mac-index <UINT:switch_mac_index>
 */
cparser_result_t cparser_cmd_l3_set_route_route_table_index_switch_mac_index_switch_mac_index(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *switch_mac_index_ptr)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_l3_routeEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l3_routeEntry_get(unit, *index_ptr, &entry), ret);
    entry.swMac_idx = *switch_mac_index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_l3_routeEntry_set(unit, *index_ptr, &entry), ret);
    return CPARSER_OK;
}
#endif

