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
 * $Revision: 30425 $
 * $Date: 2012-06-29 11:48:48 +0800 (Fri, 29 Jun 2012) $
 *
 * Purpose : Define diag shell functions for dot1x.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) dot1x diag shell.
 */


#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <rtk/dot1x.h>
#include <rtk/l2.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_DOT1X_ADD_DEL_AUTH_MAC_PORT_VID_MAC
/*
 * dot1x ( add | del ) auth-mac <PORT_LIST:port> <UINT:vid> <MACADDR:mac>
 */
cparser_result_t cparser_cmd_dot1x_add_del_auth_mac_port_vid_mac(cparser_context_t *context,
    char **port_ptr, uint32_t *vid_ptr, cparser_macaddr_t *mac_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_vlan_t      vid = 0;
    rtk_mac_t       auth_mac;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    vid = *vid_ptr;
    osal_memset(&auth_mac, 0, sizeof(rtk_mac_t));
    osal_memcpy(auth_mac.octet, &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('a' == TOKEN_CHAR(1,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_macBasedAuthMac_add(unit, port, vid, &auth_mac), ret);
        }
        else if ('d' == TOKEN_CHAR(1,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_macBasedAuthMac_del(unit, port, vid, &auth_mac), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_add_del_auth_mac_port_vid_mac */
#endif

#ifdef CMD_DOT1X_GET_AUTH_MAC_MAC
/*
 * dot1x get auth-mac { <MACADDR:mac> }
 */
cparser_result_t cparser_cmd_dot1x_get_auth_mac_mac(cparser_context_t *context,
    cparser_macaddr_t *mac_ptr)
{
    uint32             scan_idx = 0;
    uint32             total_entry = 0;
    uint32             unit = 0;
    int32              ret = RT_ERR_FAILED;    
    rtk_l2_ucastAddr_t l2_data; 
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    memset(&l2_data, 0, sizeof(rtk_l2_ucastAddr_t));
    
    diag_util_mprintf("Index|MAC Address      |VID |SPA |SaBlock |DaBlock |Auth |Static |Nexthop |Suspend\n");
    diag_util_mprintf("-----+-----------------+---+---+-------+-------+----+------+-------+-------\n");                 

    //if (TOKEN_NUM == 3)
    {
        /* show all l2 table */
        scan_idx = -1; /* get the first entry */
        while (1)
        {
            if ((ret = rtk_l2_nextValidAddr_get(unit, (int32 *)&scan_idx, 1, &l2_data)) != RT_ERR_OK)
            {
                break;
            }
    
            if (0 == l2_data.auth)
                continue;
    
            if (TOKEN_NUM == 4)
            {
                if (memcmp(mac_ptr->octet, l2_data.mac.octet, 6) != 0)
                    continue;
            }
            diag_util_mprintf("%5d|%02X:%02X:%02X:%02X:%02X:%02X|%3d|%3d|%7d|%7d|%4d|%6d|%7d|%7d\n",
                scan_idx,
                l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
                l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                l2_data.auth, (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
            total_entry++;    
        }    
        diag_util_mprintf("\nTotal number of entries:%d\n", total_entry);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_auth_mac_mac */
#endif

#ifdef CMD_DOT1X_GET_EAP_TO_CPU
/*
 * dot1x get eap-to-cpu
 */
cparser_result_t cparser_cmd_dot1x_get_eap_to_cpu(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_dot1x_eapolFrame2CpuEnable_get(unit, &enable), ret);

    diag_util_printf("EAPOL frame to CPU : ");
    if (ENABLED == enable)
    {
        diag_util_mprintf("enable\n");
    }
    else if (DISABLED == enable)
    {
        diag_util_mprintf("disable\n");
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_eap_to_cpu */
#endif

#ifdef CMD_DOT1X_GET_GUEST_VLAN_PORT_ALL
/*
 * dot1x get guest-vlan ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_dot1x_get_guest_vlan_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_t      vid = 0;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portGuestVlan_get(unit, port, &vid), ret);
        diag_util_mprintf("Port %2d guest vlan id : %4d\n", port, vid);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_guest_vlan_port_all */
#endif

#ifdef CMD_DOT1X_GET_GUEST_VLAN
/*
 * dot1x get guest-vlan
 */
cparser_result_t cparser_cmd_dot1x_get_guest_vlan(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_dot1x_guestVlanBehavior_t   behavior;
    rtk_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_dot1x_guestVlanBehavior_get(unit, &behavior), ret);
    DIAG_UTIL_ERR_CHK(rtk_dot1x_guestVlanRouteBehavior_get(unit, &action), ret);

    diag_util_printf("Guest vlan behavior : ");
    if (DISALLOW_TO_AUTH_DA == behavior)
    {
        diag_util_mprintf("dis-allow to auth DA\n");
    }
    else if (ALLOW_TO_AUTH_DA == behavior)
    {
        diag_util_mprintf("allow to auth DA\n");
    }
    else
    {
        diag_util_mprintf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    diag_util_printf("Guest vlan route behavior : ");
    if (ACTION_DROP == action)
    {
        diag_util_mprintf("drop\n");
    }
    else if (ACTION_FORWARD == action)
    {
        diag_util_mprintf("forward\n");
    }
    else if (ACTION_TRAP2CPU == action)
    {
        diag_util_mprintf("trap-to-cpu\n");
    }
    else
    {
        diag_util_mprintf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_guest_vlan */
#endif

#ifdef CMD_DOT1X_GET_TRAP
/*
 * dot1x get trap
 */
cparser_result_t cparser_cmd_dot1x_get_trap(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_pri_t   priority = 0;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_dot1x_trapPriEnable_get(unit, &enable), ret);
    diag_util_mprintf("Dot1x packet trap priority Status : %s\n", (ENABLED == enable)?"enable":"disable");

    DIAG_UTIL_ERR_CHK(rtk_dot1x_trapPri_get(unit, &priority), ret);
    diag_util_printf("Dot1x packet trap priority : ");
    if (priority > 7)
    {
        diag_util_mprintf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    diag_util_mprintf(" %d\n", priority);

    DIAG_UTIL_ERR_CHK(rtk_dot1x_trapAddCPUTagEnable_get(unit, &enable), ret);
    diag_util_mprintf("Add CPU tag : %s\n", (ENABLED == enable)?"enable":"disable");

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_trap */
#endif

#ifdef CMD_DOT1X_GET_UNAUTH_PACKET_PORT_ALL
/*
 * dot1x get unauth-packet ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_dot1x_get_unauth_packet_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;
    rtk_dot1x_unauth_action_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2d", port);
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portUnauthPacketOper_get(unit, port, &action), ret);
        if (DOT1X_ACTION_DROP == action)
            diag_util_mprintf(" unauth packet action : drop\n");
        else if (DOT1X_ACTION_TRAP2CPU == action)
            diag_util_mprintf(" unauth packet action : trap-to-cpu\n");
        else if (DOT1X_ACTION_TO_GUEST_VLAN == action)
            diag_util_mprintf(" unauth packet action : forward to guest vlan\n");
        else
            diag_util_mprintf(" unauth packet action : error!\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_unauth_packet_port_all */
#endif

#ifdef CMD_DOT1X_GET_UNAUTH_PACKET_PORT_ALL_TAG
/*
 * dot1x get unauth-packet ( <PORT_LIST:port> | all ) tag
 */
cparser_result_t cparser_cmd_dot1x_get_unauth_packet_port_all_tag(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;
    rtk_dot1x_unauth_action_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2d", port);
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portUnauthTagPacketOper_get(unit, port, &action), ret);
        if (DOT1X_ACTION_DROP == action)
            diag_util_mprintf(" unauth tag packet action : drop\n");
        else if (DOT1X_ACTION_TRAP2CPU == action)
            diag_util_mprintf(" unauth tag packet action : trap-to-cpu\n");
        else if (DOT1X_ACTION_TO_GUEST_VLAN == action)
            diag_util_mprintf(" unauth tag packet action : forward to guest vlan\n");
        else
            diag_util_mprintf(" unauth tag packet action : error!\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_unauth_packet_port_all */
#endif

#ifdef CMD_DOT1X_GET_UNAUTH_PACKET_PORT_ALL_UNTAG
/*
 * dot1x get unauth-packet ( <PORT_LIST:port> | all ) untag
 */
cparser_result_t cparser_cmd_dot1x_get_unauth_packet_port_all_untag(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;
    rtk_dot1x_unauth_action_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2d", port);
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portUnauthUntagPacketOper_get(unit, port, &action), ret);
        if (DOT1X_ACTION_DROP == action)
            diag_util_mprintf(" unauth untag packet action : drop\n");
        else if (DOT1X_ACTION_TRAP2CPU == action)
            diag_util_mprintf(" unauth untag packet action : trap-to-cpu\n");
        else if (DOT1X_ACTION_TO_GUEST_VLAN == action)
            diag_util_mprintf(" unauth untag packet action : forward to guest vlan\n");
        else
            diag_util_mprintf(" unauth untag packet action : error!\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_unauth_packet_port_all */
#endif

#ifdef CMD_DOT1X_GET_UNAUTH_PACKET
/*
 * dot1x get unauth-packet
 */
cparser_result_t cparser_cmd_dot1x_get_unauth_packet(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_dot1x_unauth_action_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(rtk_dot1x_unauthPacketOper_get(unit, &action), ret);

    if (DOT1X_ACTION_DROP == action)
    {
        diag_util_mprintf("Unauth packet operation : drop\n");
    }
    else if (DOT1X_ACTION_TRAP2CPU == action)
    {
        diag_util_mprintf("Unauth packet operation : trap-to-cpu\n");
    }
    else
    {
        diag_util_mprintf("Unauth packet operation : error!\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_unauth_packet */
#endif

#ifdef CMD_DOT1X_GET_MAC_BASED_PORT_BASED_PORT_ALL
/*
 * dot1x get ( mac-based | port-based ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_dot1x_get_mac_based_port_based_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;
    rtk_dot1x_auth_status_t     port_auth;
    rtk_dot1x_direction_t       direction;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %2d:\n", port);
        if ('m' == TOKEN_CHAR(2,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_macBasedEnable_get(unit, port, &enable), ret);
            diag_util_mprintf("    mac-based dot1x : %s\n", (ENABLED == enable)?"enable":"disable");
        }
        else if ('p' == TOKEN_CHAR(2,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_portBasedEnable_get(unit, port, &enable), ret);
            diag_util_mprintf("    port-based dot1x           : %s\n", (ENABLED == enable)?"enable":"disable");
            DIAG_UTIL_ERR_CHK(rtk_dot1x_portBasedAuthStatus_get(unit, port, &port_auth), ret);
            diag_util_mprintf("    port-based dot1x status    : %s\n", (AUTH == port_auth)?"auth":"unauth");
            DIAG_UTIL_ERR_CHK(rtk_dot1x_portBasedDirection_get(unit, port, &direction), ret);
            diag_util_mprintf("    port-based dot1x direction : %s\n", (IN == direction)?"in":"both");
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_mac_based_port_based_port_all */
#endif

#ifdef CMD_DOT1X_GET_MAC_BASED
/*
 * dot1x get mac-based
 */
cparser_result_t cparser_cmd_dot1x_get_mac_based(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_dot1x_direction_t   direction;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_dot1x_macBasedDirection_get(unit, &direction), ret);
    diag_util_mprintf("mac-based dot1x direction : %s\n", (IN == direction)?"in":"both");

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_get_mac_based */
#endif

#ifdef CMD_DOT1X_SET_EAP_TO_CPU_STATE_DISABLE_ENABLE
/*
 * dot1x set eap-to-cpu state ( disable | enable )
 */
cparser_result_t cparser_cmd_dot1x_set_eap_to_cpu_state_disable_enable(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_dot1x_eapolFrame2CpuEnable_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_eap_to_cpu_state_disable_enable */
#endif

#ifdef CMD_DOT1X_SET_GUEST_VLAN_ROUTE_DROP_FORWARD_TRAP_TO_CPU
/*
 * dot1x set guest-vlan route ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_dot1x_set_guest_vlan_route_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(4,0))
    {
        action = ACTION_DROP;
    }
    else if ('f' == TOKEN_CHAR(4,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('t' == TOKEN_CHAR(4,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_dot1x_guestVlanRouteBehavior_set(unit, action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_guest_vlan_route_drop_forward_trap_to_cpu */
#endif

#ifdef CMD_DOT1X_SET_GUEST_VLAN_TO_AUTH_DA_ALLOW_DISALLOW
/*
 * dot1x set guest-vlan to-auth-da ( allow | disallow )
 */
cparser_result_t cparser_cmd_dot1x_set_guest_vlan_to_auth_da_allow_disallow(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_dot1x_guestVlanBehavior_t   behavior;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('a' == TOKEN_CHAR(4,0))
    {
        behavior = ALLOW_TO_AUTH_DA;
    }
    else if ('d' == TOKEN_CHAR(4,0))
    {
        behavior = DISALLOW_TO_AUTH_DA;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_dot1x_guestVlanBehavior_set(unit, behavior), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_guest_vlan_to_auth_da_allow_disallow */
#endif

#ifdef CMD_DOT1X_SET_GUEST_VLAN_PORT_ALL_VID
/*
 * dot1x set guest-vlan ( <PORT_LIST:port> | all ) <UINT:vid>
 */
cparser_result_t cparser_cmd_dot1x_set_guest_vlan_port_all_vid(cparser_context_t *context,
    char **port_ptr,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_vlan_t      vid = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    vid = *vid_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portGuestVlan_set(unit, port, vid), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_guest_vlan_port_all_vid */
#endif

#ifdef CMD_DOT1X_SET_MAC_BASED_DIRECTION_BOTH_IN
/*
 * dot1x set mac-based direction ( both | in )
 */
cparser_result_t cparser_cmd_dot1x_set_mac_based_direction_both_in(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_dot1x_direction_t   direction;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(4,0))
    {
        direction = BOTH;
    }
    else if ('i' == TOKEN_CHAR(4,0))
    {
        direction = IN;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_dot1x_macBasedDirection_set(unit, direction), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_mac_based_direction_both_in */
#endif

#ifdef CMD_DOT1X_SET_MAC_BASED_PORT_BASED_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * dot1x set ( mac-based | port-based ) ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_dot1x_set_mac_based_port_based_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
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
        if ('m' == TOKEN_CHAR(2,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_macBasedEnable_set(unit, port, enable), ret);
        }
        else
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_portBasedEnable_set(unit, port, enable), ret);
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_mac_based_port_based_port_all_state_disable_enable */
#endif

#ifdef CMD_DOT1X_SET_PORT_BASED_PORT_ALL_AUTH_UNAUTH
/*
 * dot1x set port-based ( <PORT_LIST:port> | all ) ( auth | unauth )
 */
cparser_result_t cparser_cmd_dot1x_set_port_based_port_all_auth_unauth(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_dot1x_auth_status_t auth;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('a' == TOKEN_CHAR(4,0))
    {
        auth = AUTH;
    }
    else if ('u' == TOKEN_CHAR(4,0))
    {
        auth = UNAUTH;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portBasedAuthStatus_set(unit, port, auth), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_port_based_port_all_auth_unauth */
#endif

#ifdef CMD_DOT1X_SET_PORT_BASED_PORT_ALL_DIRECTION_BOTH_IN
/*
 * dot1x set port-based ( <PORT_LIST:port> | all ) direction ( both | in )
 */
cparser_result_t cparser_cmd_dot1x_set_port_based_port_all_direction_both_in(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;
    rtk_dot1x_direction_t   direction;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(5,0))
    {
        direction = BOTH;
    }
    else if ('i' == TOKEN_CHAR(5,0))
    {
        direction = IN;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portBasedDirection_set(unit, port, direction), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_port_based_port_all_direction_both_in */
#endif

#ifdef CMD_DOT1X_SET_TRAP_ADD_CPU_TAG_STATE_DISABLE_ENABLE
/*
 * dot1x set trap add-cpu-tag state ( disable | enable )
 */
cparser_result_t cparser_cmd_dot1x_set_trap_add_cpu_tag_state_disable_enable(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t   enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_dot1x_trapAddCPUTagEnable_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_trap_add_cpu_tag_state_disable_enable */
#endif

#ifdef CMD_DOT1X_SET_TRAP_PRIORITY_STATE_DISABLE_ENABLE
/*
 * dot1x set trap priority state ( disable | enable )
 */
cparser_result_t cparser_cmd_dot1x_set_trap_priority_state_disable_enable(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_dot1x_trapPriEnable_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_trap_priority_state_disable_enable */
#endif

#ifdef CMD_DOT1X_SET_TRAP_PRIORITY_PRIORITY
/*
 * dot1x set trap priority <UINT:priority>
 */
cparser_result_t cparser_cmd_dot1x_set_trap_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_pri_t   priority = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    priority = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_dot1x_trapPri_set(unit, priority), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_trap_priority_priority */
#endif

#ifdef CMD_DOT1X_SET_UNAUTH_PACKET_DROP_TRAP_TO_CPU
/*
 * dot1x set unauth-packet ( drop | trap-to-cpu )
 */
cparser_result_t cparser_cmd_dot1x_set_unauth_packet_drop_trap_to_cpu(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_dot1x_unauth_action_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('d' == TOKEN_CHAR(3,0))
    {
        action = DOT1X_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        action = DOT1X_ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_dot1x_unauthPacketOper_set(unit, action), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_unauth_packet_drop_trap_to_cpu */
#endif

#ifdef CMD_DOT1X_SET_UNAUTH_PACKET_PORT_ALL_TAG_UNTAG_DROP_GUEST_VLAN_TRAP_TO_CPU
/*
 * dot1x set unauth-packet ( <PORT_LIST:port> | all ) ( tag | untag ) ( drop | guest-vlan | trap-to-cpu )
 */
cparser_result_t cparser_cmd_dot1x_set_unauth_packet_port_all_tag_untag_drop_guest_vlan_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;
    rtk_dot1x_unauth_action_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(5,0))
    {
        action = DOT1X_ACTION_DROP;
    }
    else if ('g' == TOKEN_CHAR(5,0))
    {
        action = DOT1X_ACTION_TO_GUEST_VLAN;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action = DOT1X_ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('t' == TOKEN_CHAR(4,0))
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_portUnauthTagPacketOper_set(unit, port, action), ret);
        }
        else
        {
            DIAG_UTIL_ERR_CHK(rtk_dot1x_portUnauthUntagPacketOper_set(unit, port, action), ret);
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_unauth_packet_port_all_tag_untag_drop_guest_vlan_trap_to_cpu */
#endif

#ifdef CMD_DOT1X_SET_UNAUTH_PACKET_PORT_ALL_DROP_GUEST_VLAN_TRAP_TO_CPU
/*
 * dot1x set unauth-packet ( <PORT_LIST:port> | all ) ( drop | guest-vlan | trap-to-cpu )
 */
cparser_result_t cparser_cmd_dot1x_set_unauth_packet_port_all_drop_guest_vlan_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;
    rtk_dot1x_unauth_action_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(4,0))
    {
        action = DOT1X_ACTION_DROP;
    }
    else if ('g' == TOKEN_CHAR(4,0))
    {
        action = DOT1X_ACTION_TO_GUEST_VLAN;
    }
    else if ('t' == TOKEN_CHAR(4,0))
    {
        action = DOT1X_ACTION_TRAP2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_dot1x_portUnauthPacketOper_set(unit, port, action), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_dot1x_set_unauth_packet_port_all_drop_guest_vlan_trap_to_cpu */
#endif
