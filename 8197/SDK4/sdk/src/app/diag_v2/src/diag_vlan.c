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
 * $Revision: 54449 $
 * $Date: 2014-12-30 13:33:41 +0800 (Tue, 30 Dec 2014) $
 *
 * Purpose : Define diag shell functions for vlan.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) vlan diag shell.
 */


#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/util/rt_util.h>
#include <rtk/vlan.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


#ifdef CMD_VLAN_CREATE_VLAN_TABLE_VID_VID
/*
 * vlan create vlan-table vid <UINT:vid>
 */
cparser_result_t cparser_cmd_vlan_create_vlan_table_vid_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_create(unit, *vid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_DESTROY_VLAN_TABLE_VID_VID
/*
 * vlan destroy vlan-table vid <UINT:vid>
 */
cparser_result_t cparser_cmd_vlan_destroy_vlan_table_vid_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_destroy(unit, *vid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_DESTROY_ALL_RESTORE_DEFAULT_VLAN
/*
 * vlan destroy all { restore-default-vlan }
 */
cparser_result_t cparser_cmd_vlan_destroy_all_restore_default_vlan(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      restore = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('r' == TOKEN_STR(3)[0])
    {
        restore = 1;
    }

    if ((ret = rtk_vlan_destroyAll(unit, restore)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_VID_VID_FILTER_ID_FID
/*
 * vlan set vlan-table vid <UINT:vid> filter-id <UINT:fid>
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_vid_vid_filter_id_fid(cparser_context_t *context,
    uint32_t *vid_ptr,
    uint32_t *fid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_fid_set(unit, *vid_ptr, *fid_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_VID_VID_MSTI_MSTI
/*
 * vlan set vlan-table vid <UINT:vid> msti <UINT:msti>
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_vid_vid_msti_msti(cparser_context_t *context,
    uint32_t *vid_ptr,
    uint32_t *msti_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_stg_set(unit, *vid_ptr, *msti_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_VID_VID_FID_MSTI_FID_MSTI
/*
 *  vlan set vlan-table vid <UINT:vid> fid-msti <UINT:fid_msti>
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_vid_vid_fid_msti_fid_msti(cparser_context_t *context,
    uint32_t *vid_ptr,
    uint32_t *fid_msti_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_stg_set(unit, *vid_ptr, *fid_msti_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_VID_VID_MEMBER_PORTS_ALL_NONE
/*
 *  vlan set vlan-table vid <UINT:vid> member ( <PORT_LIST:ports> | all | none )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_vid_vid_member_ports_all_none(cparser_context_t *context,
    uint32_t *vid_ptr,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t member_portmask;
    diag_portlist_t untag_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_port_get(unit, *vid_ptr, &member_portmask.portmask, &untag_portmask.portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(member_portmask, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_vlan_port_set(unit, *vid_ptr, &member_portmask.portmask, &untag_portmask.portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_VID_VID_UNTAG_PORT_PORTS_ALL_NONE
/*
 *  vlan set vlan-table vid <UINT:vid> untag-port ( <PORT_LIST:ports> | all | none )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_vid_vid_untag_port_ports_all_none(cparser_context_t *context,
    uint32_t *vid_ptr,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t member_portmask;
    diag_portlist_t untag_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_port_get(unit, *vid_ptr, &member_portmask.portmask, &untag_portmask.portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(untag_portmask, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_vlan_port_set(unit, *vid_ptr, &member_portmask.portmask, &untag_portmask.portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif


#ifdef CMD_VLAN_SET_ACCEPT_FRAME_TYPE_INNER_PORT_PORTS_ALL_ALL_TAG_ONLY_UNTAG_ONLY
/*
 * vlan set accept-frame-type inner port ( <PORT_LIST:ports> | all ) ( all | tag-only | untag-only )
 */
cparser_result_t cparser_cmd_vlan_set_accept_frame_type_inner_port_ports_all_all_tag_only_untag_only(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_acceptFrameType_t      acceptFrame_type;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    switch (TOKEN_STR(6)[0])
    {
        case 'a':
            acceptFrame_type = ACCEPT_FRAME_TYPE_ALL;
            break;

        case 't':
            acceptFrame_type = ACCEPT_FRAME_TYPE_TAG_ONLY;
            break;

        case 'u':
            acceptFrame_type = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
            break;

        default:
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portAcceptFrameType_set(unit, port, acceptFrame_type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_ACCEPT_FRAME_TYPE_OUTER_PORT_PORTS_ALL_ALL_TAG_ONLY_UNTAG_ONLY
/*
 * vlan set accept-frame-type outer port ( <PORT_LIST:ports> | all ) ( all | tag-only | untag-only )
 */
cparser_result_t cparser_cmd_vlan_set_accept_frame_type_outer_port_ports_all_all_tag_only_untag_only(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_acceptFrameType_t      acceptFrame_type;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    switch (TOKEN_STR(6)[0])
    {
        case 'a':
            acceptFrame_type = ACCEPT_FRAME_TYPE_ALL;
            break;

        case 't':
            acceptFrame_type = ACCEPT_FRAME_TYPE_TAG_ONLY;
            break;

        case 'u':
            acceptFrame_type = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
            break;

        default:
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portOuterAcceptFrameType_set(unit, port, acceptFrame_type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_STATE_ENABLE_DISABLE
/*
 * vlan set state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_state_disable_enable(cparser_context_t *context)
{
    uint32          unit = 0;
    rtk_enable_t    enable;
    int32           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_STR(3)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if ((ret = rtk_vlan_vlanFunctionEnable_set(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_FILTER_PORT_PORTS_ALL_STATE_ENABLE_DISABLE
/*
 * vlan set ingress-filter port ( <PORT_LIST:ports> | all) state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_ingress_filter_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp_port)(uint32, rtk_port_t, rtk_enable_t);
    rtk_enable_t    enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_STR(6)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    fp_port = rtk_vlan_portIgrFilterEnable_set;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp_port(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_FILTER_PORT_PORTS_ALL_STATE_ENABLE_DISABLE
/*
 * vlan set egress-filter port ( <PORT_LIST:ports> | all) state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_egress_filter_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp_port)(uint32, rtk_port_t, rtk_enable_t);
    rtk_enable_t    enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_STR(6)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    fp_port = rtk_vlan_portEgrFilterEnable_set;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp_port(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_FILTER_STATE_ENABLE_DISABLE
/*
 * vlan set ingress-filter state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_ingress_filter_state_disable_enable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp_sys)(uint32, rtk_enable_t);
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_STR(4)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    fp_sys = rtk_vlan_igrFilterEnable_set;
    if ((ret = fp_sys(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_LEAKY_MULTICAST_STATE_DISABLE_ENABLE
/*
 * vlan set leaky multicast state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_leaky_multicast_state_disable_enable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp_sys)(uint32, rtk_enable_t);
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_STR(5)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    fp_sys = rtk_vlan_mcastLeakyEnable_set;
    if ((ret = fp_sys(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PVID_INNER_PORT_PORTS_ALL_PVID
/*
 * vlan set pvid inner port ( <PORT_LIST:ports> | all ) <UINT:pvid>
 */
cparser_result_t cparser_cmd_vlan_set_pvid_inner_port_ports_all_pvid(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *pvid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portPvid_set(unit, port, *pvid_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PVID_OUTER_PORT_PORTS_ALL_PVID
/*
 * vlan set pvid outer port ( <PORT_LIST:ports> | all ) <UINT:pvid>
 */
cparser_result_t cparser_cmd_vlan_set_pvid_outer_port_ports_all_pvid(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *pvid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portOuterPvid_set(unit, port, *pvid_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PROTOCOL_VLAN_GROUP_INDEX_FRAME_TYPE_ETHERNET_SNAP_LLC_OTHER_FRAME_VALUE_VALUE
/*
 * vlan set protocol-vlan group <UINT:index> frame-type ( ethernet | snap | llc-other ) frame-value <UINT:value>
 */
cparser_result_t cparser_cmd_vlan_set_protocol_vlan_group_index_frame_type_ethernet_snap_llc_other_frame_value_value(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *value_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_protoGroup_t   protoGroup;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_STR(6)[0])
    {
        case 'e':
            protoGroup.frametype = FRAME_TYPE_ETHERNET;
            break;

        case 's':
            protoGroup.frametype = FRAME_TYPE_RFC1042;
            break;

        case 'l':
            protoGroup.frametype = FRAME_TYPE_LLCOTHER;
            break;

        default:
            return CPARSER_NOT_OK;
    }

    protoGroup.framevalue = *value_ptr;

    if ((ret = rtk_vlan_protoGroup_set(unit, *index_ptr, &protoGroup)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PROTOCOL_VLAN_INNER_PORT_PORTS_ALL_GROUP_INDEX_VID_VID_PRIORITY_PRIORITY
/*
 * vlan set protocol-vlan inner port ( <PORT_LIST:ports> | all ) group <UINT:index> vid <UINT:vid> priority <UINT:priority>
 */
cparser_result_t cparser_cmd_vlan_set_protocol_vlan_inner_port_ports_all_group_index_vid_vid_priority_priority(
    cparser_context_t *context,
    char **ports_ptr,
    uint32_t *index_ptr,
    uint32_t *vid_ptr,
    uint32_t *priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_protoVlanCfg_t     protoVlan;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        ret = rtk_vlan_portProtoVlan_get(unit, port, *index_ptr, &protoVlan);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        protoVlan.vid = *vid_ptr;
        protoVlan.pri = *priority_ptr;
        if ((ret = rtk_vlan_portProtoVlan_set(unit, port, *index_ptr, &protoVlan)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PROTOCOL_VLAN_PORT_PORTS_ALL_GROUP_INDEX_STATE_ENABLE_DISABLE
/*
 * vlan set protocol-vlan port ( <PORT_LIST:ports> | all ) group <UINT:index> state ( disable | enable )
 */
cparser_result_t
cparser_cmd_vlan_set_protocol_vlan_port_ports_all_group_index_state_disable_enable(
        cparser_context_t *context,
        char **ports_ptr,
        uint32_t *index_ptr)
{
    uint32                  unit = 0;
    uint32                  valid;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t              port;
    diag_portlist_t         portmask;
    rtk_vlan_protoVlanCfg_t protoVlan;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(8, 0))
    {
        valid = 1;
    }
    else
    {
        valid = 0;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        ret = rtk_vlan_portProtoVlan_get(unit, port, *index_ptr, &protoVlan);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        protoVlan.valid = valid;

        ret = rtk_vlan_portProtoVlan_set(unit, port, *index_ptr, &protoVlan);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PROTOCOL_VLAN_OUTER_PORT_PORTS_ALL_GROUP_INDEX_VID_VID_PRIORITY_PRIORITY_DEI_DEI
/*
 * vlan set protocol-vlan outer port ( <PORT_LIST:ports> | all ) group <UINT:index> vlan <UINT:vid> priority <UINT:priority> dei <UINT:dei>
 */
cparser_result_t cparser_cmd_vlan_set_protocol_vlan_outer_port_ports_all_group_index_vlan_vid_priority_priority_dei_dei(
    cparser_context_t *context,
    char **ports_ptr,
    uint32_t *index_ptr,
    uint32_t *vid_ptr,
    uint32_t *priority_ptr,
    uint32_t *dei_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_protoVlanCfg_t     protoVlan;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    protoVlan.valid = 1;
    protoVlan.vid = *vid_ptr;
    protoVlan.pri = *priority_ptr;
    protoVlan.dei = *dei_ptr;

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portOuterProtoVlan_set(unit, port, *index_ptr, &protoVlan)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_TPID_PORT_PORTS_ALL_ENTRY_TPID_IDX_TPID_TPID
/*
 * vlan set tpid port ( <PORT_LIST:ports> | all ) entry <UINT:tpid_idx> tpid <UINT:tpid>
 */
cparser_result_t cparser_cmd_vlan_set_tpid_port_ports_all_entry_tpid_idx_tpid_tpid(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *tpid_idx_ptr,
    uint32_t *tpid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portTpidEntry_set(unit, port, *tpid_idx_ptr, *tpid_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_PORT_PORTS_ALL_INNER_OUTER_IGNORE_STATE_ENABLE_DISABLE
/*
 * vlan set ingress port ( <PORT_LIST:ports> | all ) ( inner | outer ) ignore state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_ingress_port_ports_all_inner_outer_ignore_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp)(uint32, rtk_port_t, rtk_enable_t);
    rtk_enable_t      enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (TOKEN_STR(5)[0])
    {
        case 'i':
            fp = rtk_vlan_portIgrIgnoreInnerTagEnable_set;
            break;
        case 'o':
            fp = rtk_vlan_portIgrIgnoreOuterTagEnable_set;
            break;
        default:
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_STR(8)[0])
    {
        case 'e':
            enable = ENABLED;
            break;

        case 'd':
            enable = DISABLED;
            break;

        default:
            return CPARSER_NOT_OK;
    }


    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_PORT_PORTS_ALL_INNER_OUTER_TPID_TPID_IDX_MASK
/*
 * vlan set ingress port ( <PORT_LIST:ports> | all ) ( inner | outer ) tpid <HEX:tpid_idx_mask>
 */
cparser_result_t cparser_cmd_vlan_set_ingress_port_ports_all_inner_outer_tpid_tpid_idx_mask(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *tpid_idx_mask_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp)(uint32, rtk_port_t, uint32);
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (TOKEN_STR(5)[0])
    {
        case 'i':
            fp = rtk_vlan_portIgrInnerTpid_set;
            break;

        case 'o':
            fp = rtk_vlan_portIgrOuterTpid_set;
            break;

        default:
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, *tpid_idx_mask_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_PORT_PORTS_ALL_EXTRA_TPID_TPID_IDX_MASK
/*
 * vlan set ingress port ( <PORT_LIST:ports> | all ) extra tpid <UINT:tpid_idx_mask>
 */
cparser_result_t cparser_cmd_vlan_set_ingress_port_ports_all_extra_tpid_tpid_idx_mask(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *tpid_idx_mask_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp)(uint32, rtk_port_t, uint32);
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    fp = rtk_vlan_portIgrExtraTpid_set;

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, *tpid_idx_mask_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_PORT_PORTS_ALL_EXTRA_STATE_ENABLE_DISABLE
/*
 * vlan set ingress port ( <PORT_LIST:ports> | all ) extra state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_ingress_port_ports_all_extra_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t      enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_STR(7)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portIgrExtraTagEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_PORT_PORTS_ALL_KEEP_INNER_OUTER
/*
 * vlan set ingress port ( <PORT_LIST:ports> | all ) keep { inner } { outer }
 */
cparser_result_t cparser_cmd_vlan_set_ingress_port_ports_all_keep_inner_outer(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t      inner_keep_enable, outer_keep_enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if (8 == TOKEN_NUM)
    {
        inner_keep_enable = ENABLED;
        outer_keep_enable = ENABLED;
    }
    else if (7 == TOKEN_NUM)
    {
        if ('i' == TOKEN_CHAR(6, 0))
        {
            inner_keep_enable = ENABLED;
            outer_keep_enable = DISABLED;
        }
        else
        {
            inner_keep_enable = DISABLED;
            outer_keep_enable = ENABLED;
        }
    }
    else
    {
        inner_keep_enable = DISABLED;
        outer_keep_enable = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portIgrTagKeepEnable_set(unit, port, outer_keep_enable, inner_keep_enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_PORT_PORTS_ALL_INNER_OUTER_KEEP_TAG_STATE_ENABLE_DISABLE
/*
 * vlan set ingress port ( <PORT_LIST:ports> | all ) ( inner | outer ) keep-tag state ( disable | enable )
 */
cparser_result_t
cparser_cmd_vlan_set_ingress_port_ports_all_inner_outer_keep_tag_state_disable_enable(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    int32           dir;    /* 0: inner, 1: outer */
    rtk_enable_t    inner_state, outer_state, new_state;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ('i' == TOKEN_CHAR(5, 0))
    {
        dir = 0;
    }
    else
    {
        dir = 1;
    }

    if ('e' == TOKEN_CHAR(8, 0))
    {
        new_state = ENABLED;
    }
    else
    {
        new_state = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        ret = rtk_vlan_portIgrTagKeepEnable_get(unit, port, &outer_state,
                &inner_state);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (0 == dir)
            inner_state = new_state;
        else
            outer_state = new_state;

        ret = rtk_vlan_portIgrTagKeepEnable_set(unit, port, outer_state,
                inner_state);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_STATE_ENABLE_DISABLE
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t      enable;
    rtk_port_t  port;
    int32       (*fp)(uint32, rtk_port_t, rtk_enable_t);
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('i' == TOKEN_STR(5)[0])
    {
        fp = rtk_vlan_portEgrInnerTagEnable_set;
    }
    else
    {
        fp = rtk_vlan_portEgrOuterTagEnable_set;
    }

    if ('e' == TOKEN_STR(7)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_TPID_MODE_IGR_UNTAG_EGR_TAG_ALL_PACKETS
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) tpid-mode ( igr-untag-egr-tag | all-packets )
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_tpid_mode_igr_untag_egr_tag_all_packets(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    int32       (*fp)(uint32, rtk_port_t, rtk_vlan_egrTpidMode_t);
    rtk_vlan_egrTpidMode_t  tpidMode;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    switch (TOKEN_STR(5)[0])
    {
        case 'i':
            fp = rtk_vlan_portEgrInnerTpidMode_set;
            break;

        case 'o':
            fp = rtk_vlan_portEgrOuterTpidMode_set;
            break;

        default:
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_STR(7)[0])
    {
        case 'i':
            tpidMode = EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG;
            break;

        case 'a':
            tpidMode = EGR_TPID_MODE_ALL_PACKETS;
            break;

        default:
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, tpidMode)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_TPID_TPID_IDX
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) tpid <UINT:tpid_idx>
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_tpid_tpid_idx(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *tpid_idx_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    int32       (*fp)(uint32, rtk_port_t, uint32);
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('i' == TOKEN_STR(5)[0])
    {
        fp = rtk_vlan_portEgrInnerTpid_set;
    }
    else
    {
        fp = rtk_vlan_portEgrOuterTpid_set;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }


    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, *tpid_idx_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_VID_SOURCE_ALE_ORIG_INNER_TAG_ORIG_OUTER_TAG
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) vid-source ( ale | orig-inner-tag | orig-outer-tag )
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_vid_source_ale_orig_inner_tag_orig_outer_tag(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    rtk_vlan_tagSource_t    tagSource;
    int32       (*fp)(uint32, rtk_port_t, rtk_vlan_tagSource_t);
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    switch (TOKEN_STR(5)[0])
    {
        case 'i':
            fp = rtk_vlan_portEgrInnerVidSource_set;
            break;
        case 'o':
            fp = rtk_vlan_portEgrOuterVidSource_set;
            break;
        default:
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_STR(7)[0])
    {
        case 'i':
            tagSource = TAG_SOURCE_FROM_ALE;
            break;
        case 'o':
            switch(TOKEN_STR(7)[5])
            {
                case 'i':
                    tagSource = TAG_SOURCE_FROM_ORIG_INNER_TAG;
                    break;
                case 'o':
                    tagSource = TAG_SOURCE_FROM_ORIG_OUTER_TAG;
                    break;
                default:
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }


    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, tagSource)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_PRI_SOURCE_ORIG_INNER_TAG_ORIG_OUTER_TAG_NULL
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) pri-source ( orig-inner-tag | orig-outer-tag | none )
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_pri_source_orig_inner_tag_orig_outer_tag_none(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    rtk_vlan_tagSource_t    tagSource;
    int32       (*fp)(uint32, rtk_port_t, rtk_vlan_tagSource_t);
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    switch (TOKEN_STR(5)[0])
    {
        case 'i':
            fp = rtk_vlan_portEgrInnerPriSource_set;
            break;
        case 'o':
            fp = rtk_vlan_portEgrOuterPriSource_set;
            break;
        default:
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_STR(7)[0])
    {
        case 'n':
            tagSource = TAG_SOURCE_NULL;
            break;
        case 'o':
            switch(TOKEN_STR(7)[5])
            {
                case 'i':
                    tagSource = TAG_SOURCE_FROM_ORIG_INNER_TAG;
                    break;
                case 'o':
                    tagSource = TAG_SOURCE_FROM_ORIG_OUTER_TAG;
                    break;
                default:
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }


    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = fp(unit, port, tagSource)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_EXTRA_STATE_ENABLE_DISABLE
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) extra state ( disable | enable )
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_extra_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t      enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_STR(7)[0])
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portEgrExtraTagEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_KEEP_INNER_OUTER
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) keep { inner } { outer }
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_keep_inner_outer(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t      inner_keep_enable, outer_keep_enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if (8 == TOKEN_NUM)
    {
        inner_keep_enable = ENABLED;
        outer_keep_enable = ENABLED;
    }
    else if (7 == TOKEN_NUM)
    {
        if ('i' == TOKEN_CHAR(6, 0))
        {
            inner_keep_enable = ENABLED;
            outer_keep_enable = DISABLED;
        }
        else
        {
            inner_keep_enable = DISABLED;
            outer_keep_enable = ENABLED;
        }
    }
    else
    {
        inner_keep_enable = DISABLED;
        outer_keep_enable = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portEgrTagKeepEnable_set(unit, port, outer_keep_enable, inner_keep_enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_KEEP_TAG_STATE_ENABLE_DISABLE
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) keep-tag state ( disable | enable )
 */
cparser_result_t
cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_keep_tag_state_disable_enable(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    int32           dir;    /* 0: inner, 1: outer */
    rtk_enable_t    inner_state, outer_state, new_state;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ('i' == TOKEN_CHAR(5, 0))
    {
        dir = 0;
    }
    else
    {
        dir = 1;
    }

    if ('e' == TOKEN_CHAR(8, 0))
    {
        new_state = ENABLED;
    }
    else
    {
        new_state = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        ret = rtk_vlan_portEgrTagKeepEnable_get(unit, port, &outer_state,
                &inner_state);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (0 == dir)
            inner_state = new_state;
        else
            outer_state = new_state;

        ret = rtk_vlan_portEgrTagKeepEnable_set(unit, port, outer_state,
                inner_state);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_TAG_MODE_PORT_PORTS_ALL_ORIGINAL_KEEP_FORMAT_PRIORITY_TAG
/*
 * vlan set tag-mode port ( <PORT_LIST:ports> | all ) ( original | keep-format | priority-tag )
 */
cparser_result_t cparser_cmd_vlan_set_tag_mode_port_ports_all_original_keep_format_priority_tag(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_tagMode_t      tagMode;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (TOKEN_STR(5)[0])
    {
        case 'o':
            tagMode = VLAN_TAG_MODE_ORIGINAL;
            break;

        case 'k':
            tagMode = VLAN_TAG_MODE_KEEP_FORMAT;
            break;

        case 'p':
            tagMode = VLAN_TAG_MODE_PRI;
            break;

        default:
            return CPARSER_NOT_OK;
    }


    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_tagMode_set(unit, port, tagMode)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_FORWARDING_MODE_VID_VID_ALE_VLAN_MEMBER
/*
 * vlan set forwarding-mode vid <UINT:vid> ( ale | vlan-member )
 */
cparser_result_t cparser_cmd_vlan_set_forwarding_mode_vid_vid_ale_vlan_member(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_fwdMode_t  fwdMode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (TOKEN_STR(5)[0])
    {
        case 'a':
            fwdMode = VLAN_FWD_ON_ALE;
            break;

        case 'v':
            fwdMode = VLAN_FWD_ON_VLAN_MEMBER;
            break;

        default:
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_vlan_fwdMode_set(unit, *vid_ptr, fwdMode)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_VID_VID_PROFILE_INDEX_INDEX
/*
  * vlan set vlan-table vid <UINT:vid> profile-index <UINT:index>
  */
cparser_result_t cparser_cmd_vlan_set_vlan_table_vid_vid_profile_index_index(cparser_context_t *context,
    uint32_t *vid_ptr,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_profileIdx_set(unit, *vid_ptr, *index_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PROFILE_ENTRY_INDEX_SA_LEARNING_STATE_ENABLE_DISABLE
/*
  * vlan set profile entry <UINT:index> sa-learning state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_profile_entry_index_sa_learning_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_profile_t profile;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    rtk_vlan_profile_get(unit, *index_ptr, &profile);
    if('e' == TOKEN_CHAR(7, 0))
    {
        profile.learn = ENABLED;
    }
    else
    {
        profile.learn = DISABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_profile_set(unit, *index_ptr, &profile), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PROFILE_ENTRY_INDEX_LOOKUP_MISS_TYPE_L2_IP4_IP6_MCAST_TABLE_TABLE_INDEX
/*
  * vlan set profile entry <UINT:index> lookup-miss-type ( l2 | ip4 | ip6 ) mcast-table <UINT:table_index>
  */
cparser_result_t cparser_cmd_vlan_set_profile_entry_index_lookup_miss_type_l2_ip4_ip6_mcast_table_table_index(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *table_index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_profile_t profile;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    rtk_vlan_profile_get(unit, *index_ptr, &profile);

    if('l' == TOKEN_CHAR(6, 0))
    {
        profile.l2_mcast_dlf_pm_idx = *table_index_ptr;
    }
    else if('i' == TOKEN_CHAR(6, 0))
    {
        if('4' == TOKEN_CHAR(6, 2))
            profile.ip4_mcast_dlf_pm_idx = *table_index_ptr;
        else
            profile.ip6_mcast_dlf_pm_idx = *table_index_ptr;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_profile_set(unit, *index_ptr, &profile), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_FILTER_PORT_PORTS_ALL_ACTION_FORWARD_DROP_TRAP
/*
 * vlan set ingress-filter port ( <PORT_LIST:ports> | all ) action ( forward | drop | trap )
 */
cparser_result_t cparser_cmd_vlan_set_ingress_filter_port_ports_all_action_forward_drop_trap(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_ifilter_t igr_filter;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('f' == TOKEN_CHAR(6, 0))
        igr_filter = INGRESS_FILTER_FWD;
    else if('d' == TOKEN_CHAR(6, 0))
        igr_filter = INGRESS_FILTER_DROP;
    else
        igr_filter = INGRESS_FILTER_TRAP;

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_portIgrFilter_set(unit, port, igr_filter), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_PVID_MODE_INNER_OUTER_PORT_PORTS_ALL_ALL_UNTAG_ONLY_UNTAG_AND_PRIORITY_TAG
/*
  * vlan set pvid-mode ( inner | outer ) port ( <PORT_LIST:ports> | all ) ( all | untag-only | untag-and-priority-tag )
  */
cparser_result_t cparser_cmd_vlan_set_pvid_mode_inner_outer_port_ports_all_all_untag_only_untag_and_priority_tag(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_pbVlan_mode_t mode;
    int32 (*fp_port)(uint32, rtk_port_t, rtk_vlan_pbVlan_mode_t);

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(3, 0))
        fp_port = rtk_vlan_portPvidMode_set;
    else
        fp_port = rtk_vlan_portOuterPvidMode_set;

    if('u' == TOKEN_CHAR(6, 0))
    {
        if('o' == TOKEN_CHAR(6, 6))
            mode = PBVLAN_MODE_UNTAG_ONLY;
        else
            mode = PBVLAN_MODE_UNTAG_AND_PRITAG;
    }
    else
        mode = PBVLAN_MODE_ALL_PKT;

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {

        DIAG_UTIL_ERR_CHK(fp_port(unit, port, mode), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_MAC_BASED_VLAN_ENTRY_INDEX_MAC_ADDRESS_MAC_VID_VID_PRIORITY_PRIORITY
/*
  * vlan set mac-based-vlan entry <UINT:index> mac-address <MACADDR:mac> vid <UINT:vid> priority <UINT:priority>
  */
cparser_result_t
cparser_cmd_vlan_set_mac_based_vlan_entry_index_mac_address_mac_vid_vid_priority_priority(
    cparser_context_t *context,
    uint32_t *index_ptr,
    cparser_macaddr_t *mac_ptr,
    uint32_t *vid_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_mac_t smac;
    rtk_vlan_t vid;
    rtk_pri_t   priority;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlan_get(unit, *index_ptr, &valid, &smac, &vid, &priority) , ret);
    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlan_set(unit, *index_ptr, valid, (rtk_mac_t *)mac_ptr, *vid_ptr, *priority_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_MAC_BASED_VLAN_ENTRY_INDEX_MAC_ADDRESS_MAC_MAC_MASK_MSK_VID_VID_PRIORITY_PRIORITY
/*
  * vlan set mac-based-vlan entry <UINT:index> mac-address <MACADDR:mac> mac-mask <MACADDR:msk> vid <UINT:vid> priority <UINT:priority>
  */
cparser_result_t 
cparser_cmd_vlan_set_mac_based_vlan_entry_index_mac_address_mac_mac_mask_msk_vid_vid_priority_priority(
    cparser_context_t *context,
    uint32_t *index_ptr,
    cparser_macaddr_t *mac_ptr,
    cparser_macaddr_t *msk_ptr,
    uint32_t *vid_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_mac_t smac;
    rtk_mac_t msk;
    rtk_vlan_t vid;
    rtk_pri_t   priority;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlanWithMsk_get(unit, *index_ptr, &valid, &smac, &msk, &vid, &priority) , ret);
    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlanWithMsk_set(unit, *index_ptr, valid, (rtk_mac_t *)mac_ptr, (rtk_mac_t *)msk_ptr, *vid_ptr, *priority_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_MAC_BASED_VLAN_ENTRY_INDEX_MAC_ADDRESS_MAC_MAC_MASK_MSK_PORT_PORT_PORT_MASK_PORT_MSK_VID_VID_PRIORITY_PRIORITY
/*
 * vlan set mac-based-vlan entry <UINT:index> mac-address <MACADDR:mac> mac-mask <MACADDR:msk> { port <UINT:port> port-mask <UINT:port_msk> } vid <UINT:vid> priority <UINT:priority>
 */
cparser_result_t cparser_cmd_vlan_set_mac_based_vlan_entry_index_mac_address_mac_mac_mask_msk_port_port_port_mask_port_msk_vid_vid_priority_priority(cparser_context_t *context,
    uint32_t *index_ptr,
    cparser_macaddr_t *mac_ptr,
    cparser_macaddr_t *msk_ptr,
    uint32_t *port_ptr,
    uint32_t *port_msk_ptr,
    uint32_t *vid_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_mac_t smac;
    rtk_mac_t msk;
    rtk_vlan_t vid;
    rtk_pri_t   priority;
    rtk_port_t port;
    rtk_port_t port_mask;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlanWithPort_get(unit, *index_ptr, &valid, &smac, &msk, &port, &port_mask, &vid, &priority) , ret);
    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlanWithPort_set(unit, *index_ptr, valid, (rtk_mac_t *)mac_ptr, (rtk_mac_t *)msk_ptr, *port_ptr, *port_msk_ptr, *vid_ptr, *priority_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_MAC_BASED_VLAN_ENTRY_INDEX_STATE_DISABLE_ENABLE
/*
  * vlan set mac-based-vlan entry <UINT:index> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_mac_based_vlan_entry_index_state_disable_enable(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_mac_t smac;
    rtk_vlan_t vid;
    rtk_pri_t   priority;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlan_get(unit, *index_ptr, &valid, &smac, &vid, &priority) , ret);
    if('e' == TOKEN_CHAR(6, 0))
    {
        valid = TRUE;
    }
    else
    {
        valid = FALSE;
    }
    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlan_set(unit, *index_ptr, valid, &smac, vid, priority), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_IP_SUBNET_BASED_VLAN_ENTRY_INDEX_SRC_IP_SIP_SRC_IP_MASK_SIPMASK_VID_VID_PRIORITY_PRIORITY
/*
  * vlan set ip-subnet-based-vlan entry <UINT:index> src-ip <IPV4ADDR:sip> src-ip-mask <IPV4ADDR:sipmask> vid <UINT:vid> priority <UINT:priority>
  */
cparser_result_t cparser_cmd_vlan_set_ip_subnet_based_vlan_entry_index_src_ip_sip_src_ip_mask_sipmask_vid_vid_priority_priority(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *sip_ptr,
    uint32_t *sipmask_ptr,
    uint32_t *vid_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    ipaddr_t sip, sip_msk;
    rtk_vlan_t vid;
    rtk_pri_t  priority;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlan_get(unit, *index_ptr, &valid, &sip, &sip_msk, &vid, &priority) , ret);
    DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlan_set(unit, *index_ptr, valid, *sip_ptr, *sipmask_ptr, *vid_ptr, *priority_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_IP_SUBNET_BASED_VLAN_ENTRY_INDEX_SRC_IP_SIP_SRC_IP_MASK_SIPMASK_PORT_PORT_PORT_MASK_PORT_MSK_VID_VID_PRIORITY_PRIORITY
/*
 * vlan set ip-subnet-based-vlan entry <UINT:index> src-ip <IPV4ADDR:sip> src-ip-mask <IPV4ADDR:sipmask> port <UINT:port> port-mask <UINT:port_msk> vid <UINT:vid> priority <UINT:priority>
 */
cparser_result_t cparser_cmd_vlan_set_ip_subnet_based_vlan_entry_index_src_ip_sip_src_ip_mask_sipmask_port_port_port_mask_port_msk_vid_vid_priority_priority(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *sip_ptr,
    uint32_t *sipmask_ptr,
    uint32_t *port_ptr,
    uint32_t *port_msk_ptr,
    uint32_t *vid_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    ipaddr_t sip, sip_msk;
    rtk_vlan_t vid;
    rtk_pri_t  priority;
    rtk_port_t port, port_msk;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlanWithPort_get(unit, *index_ptr, &valid, &sip, &sip_msk, &port, &port_msk, &vid, &priority) , ret);
    DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlanWithPort_set(unit, *index_ptr, valid, *sip_ptr, *sipmask_ptr, *port_ptr, *port_msk_ptr, *vid_ptr, *priority_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_IP_SUBNET_BASED_VLAN_ENTRY_INDEX_STATE_ENABLE_DISABLE
/*
  * vlan set ip-subnet-based-vlan entry <UINT:index> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_ip_subnet_based_vlan_entry_index_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    ipaddr_t sip, sip_msk;
    rtk_vlan_t vid;
    rtk_pri_t  priority;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlan_get(unit, *index_ptr, &valid, &sip, &sip_msk, &vid, &priority) , ret);
    if('e' == TOKEN_CHAR(6, 0))
    {
        valid = TRUE;
    }
    else
    {
        valid = FALSE;
    }
    DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlan_set(unit, *index_ptr, valid, sip, sip_msk, vid, priority), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_BLOCK_MODE_INDEX_CONVERSION_MAC_BASED_IP_SUBNET_BASED
/*
  * vlan set vlan-conversion ingress block-mode <UINT:index> ( conversion | mac-based | ip-subnet-based )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_block_mode_index_conversion_mac_based_ip_subnet_based(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_igrVlanCnvtBlk_mode_t mode;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('c' == TOKEN_CHAR(6, 0))
    {
        mode = CONVERSION_MODE_C2SC;
    }
    else if('m' == TOKEN_CHAR(6, 0))
    {
        mode = CONVERSION_MODE_MAC_BASED;
    }
    else
    {
        mode = CONVERSION_MODE_IP_SUBNET_BASED;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtBlkMode_set(unit, *index_ptr, mode) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_TPID_INNER_OUTER_EXTRA_ENTRY_TPID_IDX_TPID_TPID
/*
  * vlan set tpid ( inner | outer | extra ) entry <UINT:tpid_idx> tpid <UINT:tpid>
  */
cparser_result_t cparser_cmd_vlan_set_tpid_inner_outer_extra_entry_tpid_idx_tpid_tpid(cparser_context_t *context,
    uint32_t *tpid_idx_ptr,
    uint32_t *tpid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    int32   (*fp)(uint32, rtk_port_t, uint32);

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('i' == TOKEN_CHAR(3, 0))
    {
        fp = rtk_vlan_innerTpidEntry_set;
    }
    else if('o' == TOKEN_CHAR(3, 0))
    {
        fp = rtk_vlan_outerTpidEntry_set;
    }
    else
    {
        fp = rtk_vlan_extraTpidEntry_set;
    }

    DIAG_UTIL_ERR_CHK(fp(unit, *tpid_idx_ptr, *tpid_ptr) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_STATUS_UNTAG_PRIORITY_TAG_TAG
/*
  * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) status ( untag | priority-tag | tag )
  */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_status_untag_priority_tag_tag(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_tagSts_t sts;
    int32   (*fp)(uint32, rtk_port_t, rtk_vlan_tagSts_t);

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(5, 0))
    {
        fp = rtk_vlan_portEgrInnerTagSts_set;
    }
    else
    {
        fp = rtk_vlan_portEgrOuterTagSts_set;
    }

    if('u' == TOKEN_CHAR(7, 0))
    {
        sts = TAG_STATUS_UNTAG;
    }
    else if('p' == TOKEN_CHAR(7, 0))
    {
        sts = TAG_STATUS_PRIORITY_TAGGED;
    }
    else
    {
        sts = TAG_STATUS_TAGGED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {

        DIAG_UTIL_ERR_CHK(fp(unit, port, sts), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(7, 0))
    {
        valid = 1;
    }
    else
    {
        valid = 0;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.valid = valid;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_KEY_VID_VID_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> key vid <UINT:vid> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_key_vid_vid_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(10, 0))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.vid = *vid_ptr;
    data.vid_ignore = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_KEY_RANGE_CHECK_RANGE_CHECK_BITMASK_MASK_RANGE_CHECK_MASK
/*
  * vlan set vlan-conversion ingress entry <UINT:index> key range-check <HEX:range_check_bitmask> mask <HEX:range_check_mask>
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_key_range_check_range_check_bitmask_mask_range_check_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *range_check_bitmask_ptr,
    uint32_t *range_check_mask_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.rngchk_result = *range_check_bitmask_ptr;
    data.rngchk_result_mask = *range_check_mask_ptr;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_KEY_PRIORITY_PRIORITY_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> key priority <UINT:priority> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_key_priority_priority_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(10, 0))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.priority = *vid_ptr;
    data.priority_ignore = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_KEY_PORT_PORT_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> key port <UINT:port> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_key_port_port_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(10, 0))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.port = *port_ptr;
    data.port_ignore = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_DATA_VID_SELECT_INNER_OUTER
/*
  * vlan set vlan-conversion ingress entry <UINT:index> data vid-select ( inner | outer )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_data_vid_select_inner_outer(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('i' == TOKEN_CHAR(8, 0))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.vid_cnvt_sel = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_DATA_VID_VID_FORCE_SHIFT_POSITIVE_SHIFT_NEGATIVE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> data vid <UINT:vid> ( force | shift-positive | shift-negative )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_data_vid_vid_force_shift_positive_shift_negative(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    uint32  shift_sel = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('f' == TOKEN_CHAR(9, 0))
    {
        value = 0;
    }
    else if (0 == osal_strcmp(TOKEN_STR(9), "shift-positive"))
    {
        value = 1;
    }
    else
    {
        value = 1;
        shift_sel = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.vid_new  = *vid_ptr;
    data.vid_shift_sel = shift_sel;
    data.vid_shift = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_DATA_PRIORITY_PRIORITY_FORCE_NONE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> data priority <UINT:priority> ( force | none )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_data_priority_priority_force_none(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('f' == TOKEN_CHAR(9, 0))
    {
        value = 1;
    }
    else
    {
        value = 0;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.pri_new = *priority_ptr;
    data.pri_assign = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_DATA_STATUS_INNER_OUTER_UNTAG_TAG_NONE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> data status ( inner | outer ) ( untag | tag | none )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_data_status_inner_outer_untag_tag_none(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  status;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);

    if('u' == TOKEN_CHAR(9, 0))
    {
        status = 0;
    }
    else if('t' == TOKEN_CHAR(9, 0))
    {
        status = 1;
    }
    else
    {
        status = 2;
    }

    if('i' == TOKEN_CHAR(8, 0))
    {
        data.inner_tag_sts = status;
    }
    else
    {
        data.outer_tag_sts = status;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_DATA_TPID_TPID_INDEX_FORCE_NONE
/*
  * vlan set vlan-conversion ingress entry <UINT:index> data tpid <UINT:tpid_index> ( force | none )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_ingress_entry_index_data_tpid_tpid_index_force_none(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *tpid_index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);

    if('f' == TOKEN_CHAR(9, 0))
    {
        value = 1;
    }
    else
    {
        value = 0;
    }

    data.tpid_assign = value;
    data.tpid_idx = *tpid_index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_DOBULE_TAG_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion egress double-tag state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_double_tag_state_disable_enable(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(6, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if ((ret = rtk_vlan_egrVlanCnvtDblTagEnable_set(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_VID_SELECT_INNER_OUTER
/*
  * vlan set vlan-conversion egress vid-select ( inner | outer )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_vid_select_inner_outer(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_vlanMode_t src;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('i' == TOKEN_CHAR(5, 0))
    {
        src = BASED_ON_INNER_VLAN;
    }
    else
    {
        src = BASED_ON_OUTER_VLAN;
    }

    if ((ret = rtk_vlan_egrVlanCnvtVidSource_set(unit, src)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion egress entry <UINT:index> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(7, 0))
    {
        valid = 1;
    }
    else
    {
        valid = 0;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.valid = valid;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_KEY_VID_VID_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion egress entry <UINT:index> key vid <UINT:vid> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_key_vid_vid_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(10, 0))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.vid = *vid_ptr;
    data.vid_ignore = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_KEY_RANGE_CHECK_RANGE_CHECK_BITMASK_MASK_RANGE_CHECK_MASK
/*
  * vlan set vlan-conversion egress entry <UINT:index> key range-check <HEX:range_check_bitmask> mask <HEX:range_check_mask>
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_key_range_check_range_check_bitmask_mask_range_check_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *range_check_bitmask_ptr,
    uint32_t *range_check_mask_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.rngchk_result = *range_check_bitmask_ptr;
    data.rngchk_result_mask = *range_check_mask_ptr;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_KEY_PRIORITY_PRIORITY_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion egress entry <UINT:index> key priority <UINT:priority> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_key_priority_priority_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(10, 0))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.orgpri = *priority_ptr;
    data.orgpri_ignore = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_KEY_PORT_PORT_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-conversion egress entry <UINT:index> key port <UINT:port> state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_key_port_port_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(10, 0))
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.port = *port_ptr;
    data.port_ignore = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_DATA_VID_VID_FORCE_SHIFT_POSITIVE_SHIFT_NEGATIVE
/*
  * vlan set vlan-conversion egress entry <UINT:index> data vid <UINT:vid> ( force | shift-positive | shift-negative )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_data_vid_vid_force_shift_positive_shift_negative(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    uint32  shift_sel = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('f' == TOKEN_CHAR(9, 0))
    {
        value = 0;
    }
    else if (0 == osal_strcmp(TOKEN_STR(9), "shift-positive"))
    {
        value = 1;
    }
    else
    {
        value = 1;
        shift_sel = 1;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.vid_new  = *vid_ptr;
    data.vid_shift_sel = shift_sel;
    data.vid_shift = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_DATA_PRIORITY_PRIORITY_FORCE_NONE
/*
  * vlan set vlan-conversion egress entry <UINT:index> data priority <UINT:priority> ( force | none )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_data_priority_priority_force_none(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('f' == TOKEN_CHAR(9, 0))
    {
        value = 1;
    }
    else
    {
        value = 0;
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    data.pri_new = *priority_ptr;
    data.pri_assign = value;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_DATA_TPID_TPID_INDEX_FORCE_NONE
/*
  * vlan set vlan-conversion egress entry <UINT:index> data tpid <UINT:tpid_index> ( force | none )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_entry_index_data_tpid_tpid_index_force_none(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *tpid_index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  value;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);

    if('f' == TOKEN_CHAR(9, 0))
    {
        value = 1;
    }
    else
    {
        value = 0;
    }

    data.itpid_assign = value;
    data.itpid_idx = *tpid_index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &data) , ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_AGGREGATION_PORT_PORTS_ALL_STATE_ENABLE_DISABLE
/*
  * vlan set vlan-aggregation port ( <PORT_LIST:ports> | all ) state ( disable | enable )
  */
cparser_result_t cparser_cmd_vlan_set_vlan_aggregation_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portVlanAggrEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_LEAKY_STP_FILTER_STATE
/*
 * vlan get leaky stp-filter state
 */
cparser_result_t
cparser_cmd_vlan_get_leaky_stp_filter_state(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_leakyStpFilter_get(unit, &enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Vlan leaky STP filter: ");
    if (ENABLED == enable)
    {
        diag_util_mprintf("%s", "Enable");
    }
    else
    {
        diag_util_mprintf("%s", "Disable");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_LEAKY_STP_FILTER_STATE_DISABLE_ENABLE
/*
 * vlan set leaky stp-filter state ( disable | enable )
 */
cparser_result_t
cparser_cmd_vlan_set_leaky_stp_filter_state_disable_enable(cparser_context_t *context)
{

    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if('e' == TOKEN_CHAR(5, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if ((ret = rtk_vlan_leakyStpFilter_set(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_EXCEPTION_ACTION
/*
 * vlan get exception action
 */
cparser_result_t
cparser_cmd_vlan_get_exception_action(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_except_get(unit, &action)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Vlan except action: ");
    if (ACTION_DROP == action)
    {
        diag_util_mprintf("%s", "Drop");
    }
    else if (ACTION_FORWARD == action)
    {
        diag_util_mprintf("%s", "Forwrad");
    }
    else
    {
        diag_util_mprintf("%s", "Trap");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EXCEPTION_ACTION__DROP_FORWARD_TRAP
/*
 * vlan set exception action ( drop | forward | trap )
 */
cparser_result_t
cparser_cmd_vlan_set_exception_action_drop_forward_trap(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if('d' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_DROP;
    }
    else if('f' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_FORWARD;
    }
    else
    {
        action = ACTION_TRAP2CPU;
    }

    if ((ret = rtk_vlan_except_set(unit, action)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_INGRESS_DEFAULT_ACTION_PORTS_ALL
/*
  * vlan get vlan-conversion ingress default-action ( <PORT_LIST:ports> | all )
  */
cparser_result_t
cparser_cmd_vlan_get_vlan_conversion_ingress_default_action_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    rtk_action_t    action;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN conversion default action:\n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);

        if (RT_ERR_OK == rtk_vlan_portIgrCnvtDfltAct_get(unit, port, &action))
        {
            if (ACTION_DROP == action)
                diag_util_mprintf("%s\n", "Drop");
            else
                diag_util_mprintf("%s\n", "Forward");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_INGRESS_DEFAULT_ACTION_PORTS_ALL_FORWARD_DROP
/*
 * vlan set vlan-conversion ingress default-action ( <PORT_LIST:ports> | all ) ( forward | drop )
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_conversion_ingress_default_action_ports_all_forward_drop(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(6, 0))
    {
        action = ACTION_DROP;
    }
    else
    {
        action = ACTION_FORWARD;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portIgrCnvtDfltAct_set(unit, port, action)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;

}
#endif

#ifdef CMD_VLAN_CLEAR_VLAN_CONVERSION_INGRESS_ENTRY_INDEX_ALL
/*
 * vlan clear vlan-conversion ingress entry ( <UINT:index> | all )
 */
cparser_result_t
cparser_cmd_vlan_clear_vlan_conversion_ingress_entry_index_all(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit;
    int32   ret;
    rtk_vlan_igrVlanCnvtEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('a' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_delAll(unit) , ret);
    }
    else
    {
        memset(&entry, 0x00, sizeof(rtk_vlan_igrVlanCnvtEntry_t));
        DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_set(unit, *index_ptr, &entry), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_CLEAR_VLAN_CONVERSION_EGRESS_ENTRY_INDEX_ALL
/*
 * vlan clear vlan-conversion egress entry ( <UINT:index> | all )
 */
cparser_result_t
cparser_cmd_vlan_clear_vlan_conversion_egress_entry_index_all(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit;
    int32   ret;
    rtk_vlan_egrVlanCnvtEntry_t entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('a' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_delAll(unit) , ret);
    }
    else
    {
        memset(&entry, 0x00, sizeof(rtk_vlan_egrVlanCnvtEntry_t));
        DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_set(unit, *index_ptr, &entry), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_DUMP_IP_SUBNET_BASED_VLAN
/*
  * vlan dump ip-subnet-based-vlan
  */
cparser_result_t cparser_cmd_vlan_dump_ip_subnet_based_vlan(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    ipaddr_t sip, sip_msk;
    rtk_vlan_t vid;
    rtk_pri_t priority;
    uint32  i, cnvt_entry_total, cnvt_entry_per_block;
    rtk_vlan_igrVlanCnvtBlk_mode_t mode;
    rtk_port_t port, port_msk;
    uint8  ipv4Str[16], ipv4MskStr[16];

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Index | State   | Source IP Address |   Source IP Mask  | Port ID | Port ID Mask | VID | Priority\n");
    diag_util_mprintf("------+---------+-------------------+-------------------+---------+--------------+-----+----------\n");
    DIAG_OM_GET_CHIP_CAPACITY(unit, cnvt_entry_total, max_num_of_c2sc_entry);
    DIAG_OM_GET_CHIP_CAPACITY(unit, cnvt_entry_per_block, max_num_of_c2sc_blk_entry);

    for(i=0; i<cnvt_entry_total; i++)
    {
        if((i%cnvt_entry_per_block) == 0)
        {
            DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtBlkMode_get(unit, i/cnvt_entry_per_block, &mode) , ret);

            if(mode != CONVERSION_MODE_IP_SUBNET_BASED)
            {
                i += (cnvt_entry_per_block-1);
                continue;
            }
        }

        DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlanWithPort_get(unit, i, &valid, &sip, &sip_msk, &port, &port_msk, &vid, &priority) , ret);

        if(vid != 0)
        {
            diag_util_ip2str(ipv4Str, sip);
            diag_util_ip2str(ipv4MskStr, sip_msk);
            diag_util_mprintf("%6d| %8s|  %15s  |  %15s  |%9d|%12s%02X|%5d|%5d\n", i, valid ? "ENABLED" : "DISABLED",\
                ipv4Str, ipv4MskStr, port, "0x", port_msk, vid, priority);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_TABLE_VID_VID
/*
 * vlan get vlan-table vid <UINT:vid>
 */
cparser_result_t cparser_cmd_vlan_get_vlan_table_vid_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_stg_t   stg;
    rtk_fid_t   fid;
    rtk_portmask_t  member_portmask, untag_portmask;
    uint8           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint32      profile_idx;
#if defined(CONFIG_SDK_RTL8380)
    rtk_l2_ucastLookupMode_t ucast_hkey;
    rtk_l2_mcastLookupMode_t mcast_hkey;
#endif

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();



    if ((ret = rtk_vlan_port_get(unit, *vid_ptr, &member_portmask, &untag_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }


    diag_util_mprintf("Vlan %u \n", *vid_ptr);
    diag_util_lPortMask2str(port_list, &member_portmask);
    diag_util_mprintf("  Member Ports\t: %s \n", port_list);

    diag_util_lPortMask2str(port_list, &untag_portmask);
    diag_util_mprintf("  Untag Ports\t: %s \n", port_list);

    RTK_PORTMASK_REVERT(untag_portmask);
    diag_util_lPortMask2str(port_list, &untag_portmask);
    diag_util_mprintf("  Tag Ports\t: %s \n", port_list);

    diag_util_printf("  Fid\t\t: ");
    if (RT_ERR_OK == (ret = rtk_vlan_fid_get(unit, *vid_ptr, &fid)) )
    {
        diag_util_mprintf("%u\n", fid);
    }
    else
    {
        diag_util_mprintf("Not support\n");
    }

    diag_util_printf("  Stg\t\t: ");
    if (RT_ERR_OK == (ret = rtk_vlan_stg_get(unit, *vid_ptr, &stg)) )
    {
        diag_util_mprintf("%u\n", stg);
    }
    else
    {
        diag_util_mprintf("Not support\n");
    }

#if defined(CONFIG_SDK_RTL8380)
    if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_printf("  UBCAST hkey\t: ");
        if (RT_ERR_OK == (ret = rtk_vlan_l2UcastLookupMode_get(unit, *vid_ptr, &ucast_hkey)) )
        {
            diag_util_mprintf("%s \n", (ucast_hkey == UC_LOOKUP_ON_VID) ? "UC_LOOKUP_ON_VID" : "UC_LOOKUP_ON_FID");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    
        diag_util_printf("  MCAST hkey\t: ");
        if (RT_ERR_OK == (ret = rtk_vlan_l2McastLookupMode_get(unit, *vid_ptr, &mcast_hkey)) )
        {
            diag_util_mprintf("%s \n", (mcast_hkey == MC_LOOKUP_ON_VID) ? "UC_LOOKUP_ON_VID" : "UC_LOOKUP_ON_FID");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }
#endif

    diag_util_printf("  Profile index\t: ");
    if (RT_ERR_OK == (ret = rtk_vlan_profileIdx_get(unit, *vid_ptr, &profile_idx)) )
    {
        diag_util_mprintf("%u\n", profile_idx);
    }
    else
    {
        diag_util_mprintf("Not support\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_ACCEPT_FRAME_TYPE_INNER_PORT_PORTS_ALL
/*
 * vlan get accept-frame-type inner port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_accept_frame_type_inner_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_acceptFrameType_t  accept_frame_type;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Accept frame type of ports\n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);
        if ((ret = rtk_vlan_portAcceptFrameType_get(unit, port, &accept_frame_type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        switch (accept_frame_type)
        {
            case ACCEPT_FRAME_TYPE_ALL:
                diag_util_mprintf("accept all frame\n");
                break;

            case ACCEPT_FRAME_TYPE_TAG_ONLY:
                diag_util_mprintf("accept tag frame only\n");
                break;

            case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
                diag_util_mprintf("accept untag frame only\n");
                break;

            default:
                return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_ACCEPT_FRAME_TYPE_OUTER_PORT_PORTS_ALL
/*
 * vlan get accept-frame-type outer port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_accept_frame_type_outer_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_acceptFrameType_t  accept_frame_type;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Accept frame type of outer tag\n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);
        if ((ret = rtk_vlan_portOuterAcceptFrameType_get(unit, port, &accept_frame_type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        switch (accept_frame_type)
        {
            case ACCEPT_FRAME_TYPE_ALL:
                diag_util_mprintf("accept all frame\n");
                break;

            case ACCEPT_FRAME_TYPE_TAG_ONLY:
                diag_util_mprintf("accept tag frame only\n");
                break;

            case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
                diag_util_mprintf("accept untag frame only\n");
                break;

            default:
                return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_STATE
/*
 * vlan get state
 */
cparser_result_t cparser_cmd_vlan_get_state(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    diag_util_printf("State of vlan Function : ");
    if (RT_ERR_OK == rtk_vlan_vlanFunctionEnable_get(unit, &enable))
    {
        if (ENABLED == enable)
        {
            diag_util_mprintf("ENABLE\n");
        }
        else
        {
            diag_util_mprintf("DISABLE\n");
        }
    }
    else
    {
        diag_util_mprintf("Not support\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_INGRESS_FILTER_PORT_PORTS_ALL_STATE
/*
 * vlan get ingress-filter port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_vlan_get_ingress_filter_port_ports_all_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       (*fp_port)(uint32, rtk_port_t, rtk_enable_t *);
    rtk_enable_t    enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    fp_port = rtk_vlan_portIgrFilterEnable_get;
    diag_util_mprintf("Ingress filter status of ports\n");

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);
        if (RT_ERR_OK == fp_port(unit, port, &enable))
        {
            if (ENABLED == enable)
            {
                diag_util_mprintf("ENABLE\n");
            }
            else
            {
                diag_util_mprintf("DISABLE\n");
            }
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_EGRESS_FILTER_PORT_PORTS_ALL_STATE
/*
 * vlan get egress-filter port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_vlan_get_egress_filter_port_ports_all_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       (*fp_port)(uint32, rtk_port_t, rtk_enable_t *);
    rtk_enable_t    enable;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    fp_port = rtk_vlan_portEgrFilterEnable_get;
    diag_util_mprintf("Egress filter status of ports\n");

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);
        if (RT_ERR_OK == fp_port(unit, port, &enable))
        {
            if (ENABLED == enable)
            {
                diag_util_mprintf("ENABLE\n");
            }
            else
            {
                diag_util_mprintf("DISABLE\n");
            }
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_INGRESS_FILTER_STATE
/*
 * vlan get ingress-filter state
 */
cparser_result_t cparser_cmd_vlan_get_ingress_filter_state(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       (*fp_sys)(uint32, rtk_enable_t *);
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    fp_sys = rtk_vlan_igrFilterEnable_get;
    diag_util_printf("Ingress filter system status : ");
    if (RT_ERR_OK == fp_sys(unit, &enable))
    {
        if (ENABLED == enable)
        {
            diag_util_mprintf("ENABLE\n");
        }
        else
        {
            diag_util_mprintf("DISABLE\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_LEAKY_MULTICAST_STATE
/*
 * vlan get leaky multicast state
 */
cparser_result_t cparser_cmd_vlan_get_leaky_multicast_state(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       (*fp_sys)(uint32, rtk_enable_t *);
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    fp_sys = rtk_vlan_mcastLeakyEnable_get;
    diag_util_printf("Multicast leaky system status : ");
    if (RT_ERR_OK == fp_sys(unit, &enable))
    {
        if (ENABLED == enable)
        {
            diag_util_mprintf("ENABLE\n");
        }
        else
        {
            diag_util_mprintf("DISABLE\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_PVID_INNER_PORT_PORTS_ALL
/*
 * vlan get pvid inner port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_pvid_inner_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_vlan_t  pvid;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Port based vlan configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);
        if (RT_ERR_OK == rtk_vlan_portPvid_get(unit, port, &pvid))
        {
            diag_util_mprintf("%u\n", pvid);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_PVID_OUTER_PORT_PORTS_ALL
/*
 * vlan get pvid outer port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_pvid_outer_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_vlan_t  pvid;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Outer tag port based vlan configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);
        if (RT_ERR_OK == rtk_vlan_portOuterPvid_get(unit, port, &pvid))
        {
            diag_util_mprintf("%u\n", pvid);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_PROTOCOL_VLAN_GROUP_INDEX
/*
 * vlan get protocol-vlan group <UINT:index>
 */
cparser_result_t cparser_cmd_vlan_get_protocol_vlan_group_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_protoGroup_t   protoGroup;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_protoGroup_get(unit, *index_ptr, &protoGroup)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Protocol Group %u\n", *index_ptr);

    diag_util_printf("  Protocol type\t: ");
    switch (protoGroup.frametype)
    {
        case FRAME_TYPE_ETHERNET:
            diag_util_mprintf("Ethernet\n");
            break;

        case FRAME_TYPE_RFC1042:
            diag_util_mprintf("SNAP\n");
            break;

        case FRAME_TYPE_LLCOTHER:
            diag_util_mprintf("LLC other\n");
            break;

        default:
            diag_util_mprintf("Not support\n");
            break;
    }


    diag_util_mprintf("  Frame value\t: 0x%x\n", protoGroup.framevalue);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_PROTOCOL_VLAN_INNER_PORT_PORTS_ALL
/*
 * vlan get protocol-vlan inner port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_protocol_vlan_inner_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    uint32      ret;
    uint32      group_id;
    uint32      group_id_max;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_protoVlanCfg_t protoVlan_cfg;
    rtk_switch_devInfo_t devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Port configuration of protocol vlan \n");
    DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, protocol_vlan_idx_max);
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_mprintf("Port %u configuration : \n", port);

        for (group_id = 0; group_id <= group_id_max; group_id++)
        {

            diag_util_mprintf("  Group %u \n", group_id);

            if (RT_ERR_OK != rtk_vlan_portProtoVlan_get(unit, port, group_id, &protoVlan_cfg))
            {
                diag_util_mprintf("    Not support\n");
                continue;
            }

            if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
                diag_util_mprintf("    State    : %s\n", protoVlan_cfg.valid ? "enable" : "disable");
            diag_util_mprintf("    Vlan Id  : %u\n", protoVlan_cfg.vid);
            diag_util_mprintf("    Priority : %u\n", protoVlan_cfg.pri);
        }

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_PROTOCOL_VLAN_OUTER_PORT_PORTS_ALL
/*
 * vlan get protocol-vlan outer port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_protocol_vlan_outer_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    uint32      group_id;
    uint32      group_id_max;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_protoVlanCfg_t protoVlan_cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, protocol_vlan_idx_max);

    diag_util_mprintf("Port configuration of outer protocol vlan \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_mprintf("Port %u configuration : \n", port);

        for (group_id = 0; group_id <= group_id_max; group_id++)
        {

            diag_util_mprintf("  Group %u \n", group_id);

            if (RT_ERR_OK != rtk_vlan_portOuterProtoVlan_get(unit, port, group_id, &protoVlan_cfg))
            {
                diag_util_mprintf("    Not support\n");
                continue;
            }

            diag_util_mprintf("    Valid\t: %u\n", protoVlan_cfg.valid);
            diag_util_mprintf("    Vlan Id\t: %u\n", protoVlan_cfg.vid);
            diag_util_mprintf("    Priority\t: %u\n", protoVlan_cfg.pri);
            diag_util_mprintf("    DEI\t\t: %u\n", protoVlan_cfg.dei);
        }

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_TPID_PORT_PORTS_ALL
/*
 * vlan get tpid port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_tpid_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    uint32      group_id;
    uint32      group_id_max;
    uint32      tpid;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, tpid_entry_idx_max);

    diag_util_mprintf("TPID entry of ports\n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_mprintf("Port %u configuration : \n", port);

        for (group_id = 0; group_id <= group_id_max; group_id++)
        {

            diag_util_printf("  Group %u : ", group_id);

            if (rtk_vlan_portTpidEntry_get(unit, port, group_id, &tpid) != RT_ERR_OK)
            {
                diag_util_mprintf("Not support\n");
            }
            else
            {
                diag_util_mprintf("%x\n", tpid);
            }

        }

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_DUMP_INGRESS_EGRESS_PORT_PORTS_ALL
/*
 * vlan dump ( ingress | egress ) port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_dump_ingress_egress_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    uint32      value, value2;
    rtk_port_t  port;
    diag_portlist_t  portmask;
#if defined(CONFIG_SDK_RTL8380)
    rtk_vlan_tagKeepType_t inner_keeptype, outer_keeptype;
#endif

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ('i' == TOKEN_STR(2)[0])
    {
        diag_util_mprintf("Port ingress configuration\n");
        DIAG_UTIL_PORTMASK_SCAN(portmask, port)
        {
            diag_util_mprintf("Port %u configuration : \n", port);

            diag_util_printf("Inner TPID entry mask\t: ");
            if (RT_ERR_OK == rtk_vlan_portIgrInnerTpid_get(unit, port, &value))
            {
                diag_util_mprintf("0x%x\n", value);
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Outer TPID entry mask\t: ");
            if (RT_ERR_OK == rtk_vlan_portIgrOuterTpid_get(unit, port, &value))
            {
                diag_util_mprintf("0x%x\n", value);
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Extra TPID entry mask\t: ");
            if (RT_ERR_OK == rtk_vlan_portIgrExtraTpid_get(unit, port, &value))
            {
                diag_util_mprintf("0x%x\n", value);
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Ignore inner tag status\t: ");
            if (RT_ERR_OK == rtk_vlan_portIgrIgnoreInnerTagEnable_get(unit, port, &value))
            {
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Ignore outer tag status\t: ");
            if (RT_ERR_OK == rtk_vlan_portIgrIgnoreOuterTagEnable_get(unit, port, &value))
            {
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Extra tag status\t: ");
            if (RT_ERR_OK == rtk_vlan_portIgrExtraTagEnable_get(unit, port, &value))
            {
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }


            if (RT_ERR_OK == rtk_vlan_portIgrTagKeepEnable_get(unit, port, &value, &value2))
            {
                diag_util_printf("Keep inner tag format\t: ");
                if (ENABLED == value2)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }

                diag_util_printf("Keep outer tag format\t: ");
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }

#if defined(CONFIG_SDK_RTL8380)
            if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
            {
                if (RT_ERR_OK == rtk_vlan_portIgrTagKeepType_get(unit, port, &outer_keeptype, &inner_keeptype))
                {
                    diag_util_printf("Keep inner tag type\t: ");
                    if (TAG_KEEP_TYPE_NOKEEP == inner_keeptype)
                    {
                        diag_util_mprintf("NO KEEP\n");
                    }
                    else if (TAG_KEEP_TYPE_FORMAT == inner_keeptype)
                    {
                        diag_util_mprintf("KEEP FORMAT\n");
                    }
                    else if (TAG_KEEP_TYPE_CONTENT == inner_keeptype)
                    {
                        diag_util_mprintf("KEEP CONTENT\n");
                    }
    
                    diag_util_printf("Keep outer tag type\t: ");
                    if (TAG_KEEP_TYPE_NOKEEP == outer_keeptype)
                    {
                        diag_util_mprintf("NO KEEP\n");
                    }
                    else if (TAG_KEEP_TYPE_FORMAT == outer_keeptype)
                    {
                        diag_util_mprintf("KEEP FORMAT\n");
                    }
                    else if (TAG_KEEP_TYPE_CONTENT == outer_keeptype)
                    {
                        diag_util_mprintf("KEEP CONTENT\n");
                    }
                }
            }
#endif

            diag_util_mprintf("\n");
        }
    }
    else
    {
        diag_util_mprintf("Port egress configuration\n");
        DIAG_UTIL_PORTMASK_SCAN(portmask, port)
        {
            diag_util_mprintf("Port %u configuration : \n", port);
            diag_util_printf("Inner:\n");
            diag_util_printf("TPID Mode\t\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrInnerTpidMode_get(unit, port, &value))
            {
                if (EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG == value)
                {
                    diag_util_mprintf("Modify TPID only for ingress untag packet\n");
                }
                else
                {
                    diag_util_mprintf("All kind of packets\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("TPID Entry Index\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrInnerTpid_get(unit, port, &value))
            {
                diag_util_mprintf("%u\n", value);
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Tag State\t\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrInnerTagEnable_get(unit, port, &value))
            {
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Tag Status\t\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrInnerTagSts_get(unit, port, &value))
            {
                if (TAG_STATUS_UNTAG == value)
                {
                    diag_util_mprintf("Untag\n");
                }
                else if (TAG_STATUS_TAGGED == value)
                {
                    diag_util_mprintf("Tag\n");
                }
                else
                {
                    diag_util_mprintf("Priority Tag\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("VID Source\t\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrInnerVidSource_get(unit, port, &value))
            {
                switch (value)
                {
                    case TAG_SOURCE_FROM_ALE:
                        diag_util_mprintf("By ALE decision\n");
                        break;

                    case TAG_SOURCE_FROM_ORIG_INNER_TAG:
                        diag_util_mprintf("From original inner tag\n");
                        break;

                    case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
                        diag_util_mprintf("From original outer tag\n");
                        break;

                    default:
                        diag_util_mprintf("Not support\n");
                        break;
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Priority Source\t\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrInnerPriSource_get(unit, port, &value))
            {
                switch (value)
                {
                    case TAG_SOURCE_FROM_ORIG_INNER_TAG:
                        diag_util_mprintf("From original inner tag\n");
                        break;

                    case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
                        diag_util_mprintf("From original outer tag\n");
                        break;

                    case TAG_SOURCE_NULL:
                        diag_util_mprintf("Null value\n");
                        break;

                    default:
                        diag_util_mprintf("Not support\n");
                        break;
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }
            diag_util_printf("\n");


            diag_util_printf("Outer:\n");
            diag_util_printf("TPID mode\t\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrOuterTpidMode_get(unit, port, &value))
            {
                if (EGR_TPID_MODE_IGR_UNTAG_AND_EGR_TAG == value)
                {
                    diag_util_mprintf("Modify TPID only for ingress untag packet\n");
                }
                else
                {
                    diag_util_mprintf("All kind of packets\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("TPID Entry Index\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrOuterTpid_get(unit, port, &value))
            {
                diag_util_mprintf("%u\n", value);
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }


            diag_util_printf("Tag State\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrOuterTagEnable_get(unit, port, &value))
            {
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Tag Status\t\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrOuterTagSts_get(unit, port, &value))
            {
                if (TAG_STATUS_UNTAG == value)
                {
                    diag_util_mprintf("Untag\n");
                }
                else if (TAG_STATUS_TAGGED == value)
                {
                    diag_util_mprintf("Tag\n");
                }
                else
                {
                    diag_util_mprintf("Priority Tag\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("VID Source\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrOuterVidSource_get(unit, port, &value))
            {
                switch (value)
                {
                    case TAG_SOURCE_FROM_ALE:
                        diag_util_mprintf("By ALE decision\n");
                        break;

                    case TAG_SOURCE_FROM_ORIG_INNER_TAG:
                        diag_util_mprintf("From original inner tag\n");
                        break;

                    case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
                        diag_util_mprintf("From original outer tag\n");
                        break;

                    default:
                        diag_util_mprintf("Not support\n");
                        break;
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }

            diag_util_printf("Priority Source\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrOuterPriSource_get(unit, port, &value))
            {
                switch (value)
                {
                    case TAG_SOURCE_FROM_ORIG_INNER_TAG:
                        diag_util_mprintf("From original inner tag\n");
                        break;

                    case TAG_SOURCE_FROM_ORIG_OUTER_TAG:
                        diag_util_mprintf("From original outer tag\n");
                        break;

                    case TAG_SOURCE_NULL:
                        diag_util_mprintf("Null value\n");
                        break;

                    default:
                        diag_util_mprintf("Not support\n");
                        break;
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }
            diag_util_printf("\n");

            diag_util_printf("Extra:\n");
            diag_util_printf("Tag State\t: ");
            if (RT_ERR_OK == rtk_vlan_portEgrExtraTagEnable_get(unit, port, &value))
            {
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }
            diag_util_printf("\n");

            if (RT_ERR_OK == rtk_vlan_portEgrTagKeepEnable_get(unit, port, &value, &value2))
            {
                diag_util_printf("Keep inner tag format\t: ");
                if (ENABLED == value2)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }

                diag_util_printf("Keep outer tag format\t: ");
                if (ENABLED == value)
                {
                    diag_util_mprintf("ENABLE\n");
                }
                else
                {
                    diag_util_mprintf("DISABLE\n");
                }
            }

#if defined(CONFIG_SDK_RTL8380)
            if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
            {
                if (RT_ERR_OK == rtk_vlan_portEgrTagKeepType_get(unit, port, &outer_keeptype, &inner_keeptype))
                {
                    diag_util_printf("Keep inner tag type\t: ");
                    if (TAG_KEEP_TYPE_NOKEEP == inner_keeptype)
                    {
                        diag_util_mprintf("NO KEEP\n");
                    }
                    else if (TAG_KEEP_TYPE_FORMAT == inner_keeptype)
                    {
                        diag_util_mprintf("KEEP FORMAT\n");
                    }
                    else if (TAG_KEEP_TYPE_CONTENT == inner_keeptype)
                    {
                        diag_util_mprintf("KEEP CONTENT\n");
                    }
    
                    diag_util_printf("Keep outer tag type\t: ");
                    if (TAG_KEEP_TYPE_NOKEEP == outer_keeptype)
                    {
                        diag_util_mprintf("NO KEEP\n");
                    }
                    else if (TAG_KEEP_TYPE_FORMAT == outer_keeptype)
                    {
                        diag_util_mprintf("KEEP FORMAT\n");
                    }
                    else if (TAG_KEEP_TYPE_CONTENT == outer_keeptype)
                    {
                        diag_util_mprintf("KEEP CONTENT\n");
                    }
                }
            }
#endif

            diag_util_mprintf("\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_TAG_MODE_PORT_PORTS_ALL
/*
 * vlan get tag-mode port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_vlan_get_tag_mode_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_tagMode_t  tagMode;
    rtk_port_t  port;
    diag_portlist_t  portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();


    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Egress tag mode of ports\n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);
        if ((ret = rtk_vlan_tagMode_get(unit, port, &tagMode)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        switch (tagMode)
        {
            case VLAN_TAG_MODE_ORIGINAL:
                diag_util_mprintf("following ALE decision\n");
                break;

            case VLAN_TAG_MODE_KEEP_FORMAT:
                diag_util_mprintf("keep original format\n");
                break;

            case VLAN_TAG_MODE_PRI:
                diag_util_mprintf("priority tag frame\n");
                break;

            default:
                return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_FORWARDING_MODE_VID_VID
/*
 * vlan get forwarding-mode vid <UINT:vid>
 */
cparser_result_t cparser_cmd_vlan_get_forwarding_mode_vid_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_fwdMode_t  fwdMode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_fwdMode_get(unit, *vid_ptr, &fwdMode)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_printf("Forwarding mode of vlan %u : ", *vid_ptr);

    switch (fwdMode)
    {
        case VLAN_FWD_ON_ALE:
            diag_util_mprintf("based on ALE decision\n");
            break;

        case VLAN_FWD_ON_VLAN_MEMBER:
            diag_util_mprintf("forward to member of vlan\n");
            break;

        default:
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_DUMP_MAC_BASED_VLAN
/*
  * vlan dump mac-based-vlan
  */
cparser_result_t cparser_cmd_vlan_dump_mac_based_vlan(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_mac_t smac;
    rtk_mac_t smsk;
    rtk_vlan_t vid;
    uint32  i, cnvt_entry_total, cnvt_entry_per_block;
    rtk_vlan_igrVlanCnvtBlk_mode_t mode;
    rtk_pri_t   priority;
    rtk_port_t port, pmsk;
    uint8 macStr[20], macMskStr[20];

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Index | State   | Source MAC Address | Source MAC Mask   | Port ID | Port ID Mask | VID | Priority\n");
    diag_util_mprintf("------+---------+--------------------+-------------------+---------+--------------+-----+----------\n");
    DIAG_OM_GET_CHIP_CAPACITY(unit, cnvt_entry_total, max_num_of_c2sc_entry);
    DIAG_OM_GET_CHIP_CAPACITY(unit, cnvt_entry_per_block, max_num_of_c2sc_blk_entry);

    for(i=0; i<cnvt_entry_total; i++)
    {
        if((i%cnvt_entry_per_block) == 0)
        {
            DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtBlkMode_get(unit, i/cnvt_entry_per_block, &mode) , ret);

            if(mode != CONVERSION_MODE_MAC_BASED)
            {
                i += (cnvt_entry_per_block-1);
                continue;
            }
        }

        DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlanWithPort_get(unit, i, &valid, &smac, &smsk, &port, &pmsk, &vid, &priority) , ret);

        if ((vid != 0))
        {
            diag_util_mac2str(macStr, smac.octet);
            diag_util_mac2str(macMskStr, smsk.octet);

            diag_util_mprintf("%6d| %8s|  %17s | %17s |%9d|%12s%02X|%5d|%5d\n", i, valid ? "ENABLED" : "DISABLED",\
                macStr, macMskStr, port, "0x", pmsk, vid, priority);
        }
    }    

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_DUMP_VLAN_CONVERSION_INGRESS_ENTRY_LOW_IDX_HIGH_IDX
#if 0
/*
  * vlan dump vlan-conversion ingress entry <UINT:low_idx> <UINT:high_idx>
  */
cparser_result_t cparser_cmd_vlan_dump_vlan_conversion_ingress_entry_low_idx_high_idx(cparser_context_t *context,
    uint32_t *low_idx_ptr,
    uint32_t *high_idx_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_igrVlanCnvtEntry_t data;
    uint32  i;
    int8    enStr1[8]='\0';
    int8    enStr2[8]='\0';
    int8    enStr3[8]='\0';
    int8    itagStr[9]='\0';
    int8    otagStr[9]='\0';

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress VLAN conversion entry configuration\n\n");

    for(i=*low_idx_ptr; i<=*high_idx_ptr, i++)
    {
        ret = rtk_vlan_igrVlanCnvtEntry_get(unit, i, &data) , ret);

        switch (ret)
        {
            case RT_ERR_VLAN_C2SC_BLOCK_DISABLED:
                diag_util_mprintf("based on ALE decision\n");
                break;

            case RT_ERR_VLAN_C2SC_BLOCK_MODE:
                diag_util_mprintf("forward to member of vlan\n");
                break;

            case RT_ERR_CHIP_NOT_SUPPORTED:
                DIAG_ERR_PRINT(RT_ERR_CHIP_NOT_SUPPORTED);
            default:
                return CPARSER_NOT_OK;
        }

        diag_util_mprintf("Index|State  |VIDState|VID |RangeCheck|RC Mask |PortState|Port|VIDSEL|VIDOP|NewVID|PriOP|NewPri|ITAG|OTAG|TPIDOP|TPIDIDX\n");
        diag_util_mprintf("-----+-------+--------+----+----------+--------+---------+----+------+-----+------+-----+------+----+----+------+-------\n");

        if(data.valid == 1)
            strcpy(enStr1,"ENABLE ");
        else
            strcpy(enStr1,"DISABLE");

        if(data.vid_ignore == 0)
            strcpy(enStr2,"ENABLE  ");
        else
            strcpy(enStr2,"DISABLE ");

        if(data.port_ignore == 0)
            strcpy(enStr3,"ENABLE  ");
        else
            strcpy(enStr3,"DISABLE ");

        if(data.inner_tag_sts == 0)
            strcpy(itagStr,"untag   ");
        else if(data.inner_tag_sts == 1)
            strcpy(itagStr,"tag     ");
        else
            strcpy(itagStr,"reserved");

        if(data.onner_tag_sts == 0)
            strcpy(otagStr,"untag   ");
        else if(data.inner_tag_sts == 1)
            strcpy(otagStr,"tag     ");
        else
            strcpy(otagStr,"reserved");

        diag_util_mprintf("%5d|%s|%s|%4d|%08x|%08x|%s|%4d|%s|%s|%6d|%s|%6d|%s|%s|%s|%d\n", i, enStr1, enStr2, data.vid, data.rngchk_result, data.rngchk_result_mask\
            enStr3, data.port, (data.vid_cnvt_sel==0) ? "inner ":"outer ", (data.vid_shift==0) ? "force":"shift",data.vid_new,\
            (data.pri_assign==0) ? "force":"none ", data.pri_new, itagStr, otagStr, (data.tpid_assign==0) ? "none  ":"force ", data.tpid_idx);
    }

    return CPARSER_OK;
}
#endif
#endif

#ifdef CMD_VLAN_GET_PROFILE_ENTRY_INDEX
/*
  * vlan get profile entry <UINT:index>
  */
cparser_result_t cparser_cmd_vlan_get_profile_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_profile_t profile;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_vlan_profile_get(unit, *index_ptr, &profile), ret);

    diag_util_mprintf("VLAN profile configuration\n");
    diag_util_mprintf(" SA learning: %s\n", profile.learn ? "enable" : "disable");
    diag_util_mprintf(" portmask table index for L2 unknown multicast traffic : %d\n", profile.l2_mcast_dlf_pm_idx);
    diag_util_mprintf(" portmask table index for IP4 unknown multicast traffic: %d\n", profile.ip4_mcast_dlf_pm_idx);
    diag_util_mprintf(" portmask table index for IP6 unknown multicast traffic: %d\n", profile.ip6_mcast_dlf_pm_idx);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_INGRESS_FILTER_PORT_PORTS_ALL_ACTION
/*
  * vlan get ingress-filter port ( <PORT_LIST:ports> | all ) action
  */
cparser_result_t cparser_cmd_vlan_get_ingress_filter_port_ports_all_action(cparser_context_t *context,
    char **ports_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_ifilter_t igr_filter;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Ingress filter configuration\n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_portIgrFilter_get(unit, port, &igr_filter), ret);
        diag_util_mprintf("Port %u : ", port);
        if (igr_filter == INGRESS_FILTER_FWD)
            diag_util_mprintf("Forward\n", port);
        else if (igr_filter == ACTION_DROP)
            diag_util_mprintf("Drop\n", port);
        else
            diag_util_mprintf("Trap\n", port);

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_PVID_MODE_INNER_OUTER_PORT_PORTS_ALL
/*
  * vlan get pvid-mode ( inner | outer ) port ( <PORT_LIST:ports> | all )
  */
cparser_result_t cparser_cmd_vlan_get_pvid_mode_inner_outer_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portmask;
    rtk_vlan_pbVlan_mode_t mode;
    int32 (*fp_port)(uint32, rtk_port_t, rtk_vlan_pbVlan_mode_t *);

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(3, 0))
    {
        diag_util_mprintf("Inner port-based VLAN mode configuration\n");
        fp_port = rtk_vlan_portPvidMode_get;
    }
    else
    {
        diag_util_mprintf("Outer port-based VLAN mode configuration\n");
        fp_port = rtk_vlan_portOuterPvidMode_get;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        DIAG_UTIL_ERR_CHK(fp_port(unit, port, &mode), ret);
        diag_util_mprintf("Port %u : ", port);
        if (mode == PBVLAN_MODE_UNTAG_AND_PRITAG)
            diag_util_mprintf("Untag and priority tag\n", port);
        else if (mode == PBVLAN_MODE_UNTAG_ONLY)
            diag_util_mprintf("Untag only\n", port);
        else
            diag_util_mprintf("All\n", port);

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_MAC_BASED_VLAN_ENTRY_INDEX
/*
  * vlan get mac-based-vlan entry <UINT:index>
  */
cparser_result_t cparser_cmd_vlan_get_mac_based_vlan_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    rtk_mac_t smac, smsk;
    rtk_vlan_t vid;
    rtk_pri_t priority;
    rtk_port_t port, pmsk;
    uint8 macStr[16];

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_vlan_macBasedVlanWithPort_get(unit, *index_ptr, &valid, &smac, &smsk, &port, &pmsk, &vid, &priority) , ret);

    diag_util_mprintf("MAC-based VLAN configuration\n");
    diag_util_mprintf("Entry index          : %u\n", *index_ptr);
    diag_util_mprintf("State                : %s\n", valid ? "ENABLED" : "DISABLED");
    diag_util_mac2str(macStr, smac.octet);
    diag_util_mprintf("Source MAC address   : %s\n", macStr);
    diag_util_mac2str(macStr, smsk.octet);
    diag_util_mprintf("Source MAC Mask      : %s\n", macStr);
    diag_util_mprintf("Port ID              : %d\n", port);
    diag_util_mprintf("Port ID Mask         : 0x%02X\n", pmsk);
    diag_util_mprintf("VID                  : %u \n", vid);
    diag_util_mprintf("Priority             : %u\n", priority);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_IP_SUBNET_BASED_VLAN_ENTRY_INDEX
/*
  * vlan get ip-subnet-based-vlan entry <UINT:index>
  */
cparser_result_t cparser_cmd_vlan_get_ip_subnet_based_vlan_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  valid;
    ipaddr_t sip, sip_msk;
    rtk_vlan_t vid;
    rtk_pri_t priority;
    rtk_port_t port, port_msk;
    uint8  ipv4Str[16];

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_vlan_ipSubnetBasedVlanWithPort_get(unit, *index_ptr, &valid, &sip, &sip_msk, &port, &port_msk, &vid, &priority) , ret);

    diag_util_mprintf("IP-Subnet-based VLAN configuration\n");
    diag_util_mprintf("State                : %s\n", valid ? "ENABLED" : "DISABLED");
    diag_util_mprintf("Entry index          : %u\n", *index_ptr);
    diag_util_ip2str(ipv4Str, sip);
    diag_util_mprintf("Source IP address    : %s\n", ipv4Str);
    diag_util_ip2str(ipv4Str, sip_msk);
    diag_util_mprintf("Source IP Mask       : %s\n", ipv4Str);
    diag_util_mprintf("Port ID              : %d\n", port);
    diag_util_mprintf("Port ID Mask         : 0x%02X\n", port_msk);
    diag_util_mprintf("VID                  : %u \n", vid);
    diag_util_mprintf("Priority             : %u\n", priority);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_INGRESS_BLOCK_MODE
/*
  * vlan get vlan-conversion ingress block-mode
  */
cparser_result_t
cparser_cmd_vlan_get_vlan_conversion_ingress_block_mode(
    cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  i, block_num;
    rtk_vlan_igrVlanCnvtBlk_mode_t  blk_mode;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_OM_GET_CHIP_CAPACITY(unit, block_num, max_num_of_c2sc_blk);
    diag_util_mprintf("VLAN conversion block configuration\n\n");
    diag_util_mprintf(" Index | Mode \n");
    diag_util_mprintf("-------+-----------------------\n");
    for(i=0; i<block_num; i++)
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtBlkMode_get(unit, i, &blk_mode) , ret);
        diag_util_mprintf("%4d   |", i);

        if(blk_mode==CONVERSION_MODE_C2SC)
            diag_util_mprintf("%s", "Ingress vlan conversion\n");
        else if(blk_mode==CONVERSION_MODE_MAC_BASED)
            diag_util_mprintf("%s", "MAC-based VLAN\n");
        else
            diag_util_mprintf("%s", "IP-Subnet-based VLAN\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_TPID_INNER_OUTER_EXTRA
/*
  * vlan get tpid ( inner | outer | extra )
  */
cparser_result_t cparser_cmd_vlan_get_tpid_inner_outer_extra(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  i, entry_num;
    uint32  tpid;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if('i' == TOKEN_CHAR(3, 0))
    {
        DIAG_OM_GET_CHIP_CAPACITY(unit, entry_num, max_num_of_cvlan_tpid);
        diag_util_mprintf("Inner TPID configuration\n");
        for(i=0; i<entry_num; i++)
        {
            DIAG_UTIL_ERR_CHK(rtk_vlan_innerTpidEntry_get(unit, i, &tpid) , ret);
            diag_util_mprintf(" TPID %d: 0x%x\n", i, tpid);
        }
    }
    else if('o' == TOKEN_CHAR(3, 0))
    {
        DIAG_OM_GET_CHIP_CAPACITY(unit, entry_num, max_num_of_svlan_tpid);
        diag_util_mprintf("Outer TPID configuration\n");
        for(i=0; i<entry_num; i++)
        {
            DIAG_UTIL_ERR_CHK(rtk_vlan_outerTpidEntry_get(unit, i, &tpid) , ret);
            diag_util_mprintf(" TPID %d: 0x%x\n", i, tpid);
        }
    }
    else
    {
        DIAG_OM_GET_CHIP_CAPACITY(unit, entry_num, max_num_of_evlan_tpid);
        diag_util_mprintf("Extra TPID configuration\n");
        for(i=0; i<entry_num; i++)
        {
            DIAG_UTIL_ERR_CHK(rtk_vlan_extraTpidEntry_get(unit, i, &tpid) , ret);
            diag_util_mprintf(" TPID %d: 0x%x\n", i, tpid);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_INGRESS_ENTRY_INDEX
/*
  * vlan get vlan-conversion ingress entry <UINT:index>
  */
cparser_result_t cparser_cmd_vlan_get_vlan_conversion_ingress_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_igrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_vlan_igrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    diag_util_mprintf("Ingress VLAN conversion entry configuration\n");
    diag_util_mprintf(" Index : %d\n", *index_ptr);
    diag_util_mprintf(" Enable state : %s\n", (data.valid == 1)? "ENABLED" : "DISABLED");

    diag_util_mprintf(" ------ KEY SECTION ------\n");
    diag_util_mprintf(" VID comparing state : %s\n", (data.vid_ignore == 0)? "ENABLED" : "DISABLED");
    diag_util_mprintf(" VID : %d\n", data.vid);
    diag_util_mprintf(" Range check result : 0x%x\n", data.rngchk_result);
    diag_util_mprintf(" Range check result mask : 0x%x\n", data.rngchk_result_mask);
    diag_util_mprintf(" Priority comparing state : %s\n", (data.priority_ignore == 0)? "ENABLED" : "DISABLED");
    diag_util_mprintf(" Priority : %d\n", data.priority);
    diag_util_mprintf(" Port comparing state : %s\n", (data.port_ignore == 0)? "ENABLED" : "DISABLED");
    diag_util_mprintf(" Port : %d\n", data.port);

    diag_util_mprintf(" ------ DATA SECTION ------\n");
    diag_util_mprintf(" VID selection : %s\n", (data.vid_cnvt_sel == 0)? "inner" : "outer");
    diag_util_mprintf(" VID operation : %s\n", (data.vid_shift == 0)? "force" : "shift");
    diag_util_mprintf(" New VID : %d\n", data.vid_new);
    diag_util_mprintf(" Priority operation : %s\n", (data.pri_assign == 1)? "force" : "none");
    diag_util_mprintf(" New priority : %d\n", data.pri_new);
    diag_util_mprintf(" Egress inner tag status : ");
    if(data.inner_tag_sts == 0)
        diag_util_mprintf("untag\n");
    else if(data.inner_tag_sts == 1)
        diag_util_mprintf("tag\n");
    else
        diag_util_mprintf("reserved\n");
    diag_util_mprintf(" Egress outer tag status : ");
    if(data.outer_tag_sts == 0)
        diag_util_mprintf("untag\n");
    else if(data.outer_tag_sts == 1)
        diag_util_mprintf("tag\n");
    else
        diag_util_mprintf("none(don't touch)\n");
    diag_util_mprintf(" TPID operation : %s\n", (data.tpid_assign == 1)? "force" : "none");
    diag_util_mprintf(" TPID index : %d\n", data.tpid_idx);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_DOBULE_TAG_STATE
/*
  * vlan get vlan-conversion egress dobule-tag state
  */
cparser_result_t cparser_cmd_vlan_get_vlan_conversion_egress_dobule_tag_state(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_egrVlanCnvtDblTagEnable_get(unit, &enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Egress VLAN conversion dobule tag state : %s\n", enable ? "ENABLED" : "DISABLED");

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_VID_SELECT
/*
  * vlan get vlan-conversion egress vid-select
  */
cparser_result_t cparser_cmd_vlan_get_vlan_conversion_egress_vid_select(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_l2_vlanMode_t src;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_vlan_egrVlanCnvtVidSource_get(unit, &src)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Egress VLAN conversion source VLAN : %s\n", (src==BASED_ON_INNER_VLAN) ? "inner" : "outer");

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_ENTRY_INDEX
/*
  * vlan get vlan-conversion egress entry <UINT:index>
  */
cparser_result_t cparser_cmd_vlan_get_vlan_conversion_egress_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_vlan_egrVlanCnvtEntry_t data;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_vlan_egrVlanCnvtEntry_get(unit, *index_ptr, &data) , ret);
    diag_util_mprintf("Egress VLAN conversion entry configuration\n");
    diag_util_mprintf(" Index : %d\n", *index_ptr);
    diag_util_mprintf(" Enable state : %s\n", (data.valid == 1)? "ENABLED" : "DISABLED");

    diag_util_mprintf(" ------ KEY SECTION ------\n");
    diag_util_mprintf(" VID comparing state : %s\n", (data.vid_ignore == 0)? "ENABLED" : "DISABLED");
    diag_util_mprintf(" VID : %d\n", data.vid);
    diag_util_mprintf(" Range check result : 0x%x\n", data.rngchk_result);
    diag_util_mprintf(" Range check result mask : 0x%x\n", data.rngchk_result_mask);
    diag_util_mprintf(" Priority comparing state : %s\n", (data.orgpri_ignore == 0)? "ENABLED" : "DISABLED");
    diag_util_mprintf(" Priority : %d\n", data.orgpri);
    diag_util_mprintf(" Port comparing state : %s\n", (data.port_ignore == 0)? "ENABLED" : "DISABLED");
    diag_util_mprintf(" Port : %d\n", data.port);

    diag_util_mprintf(" ------ DATA SECTION ------\n");
    diag_util_mprintf(" VID operation : %s\n", (data.vid_shift == 0)? "force" : "shift");
    diag_util_mprintf(" New VID : %d\n", data.vid_new);
    diag_util_mprintf(" Priority operation : %s\n", (data.pri_assign == 1)? "force" : "none");
    diag_util_mprintf(" New priority : %d\n", data.pri_new);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf(" TPID operation : %s\n", (data.itpid_assign == 1)? "force" : "none");
        diag_util_mprintf(" TPID index : %d\n", data.itpid_idx);
    }
#endif

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_AGGREGATION_PORT_PORTS_ALL_STATE
/*
  * vlan get vlan-aggregation port ( <PORT_LIST:ports> | all ) state
  */
cparser_result_t cparser_cmd_vlan_get_vlan_aggregation_port_ports_all_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    rtk_enable_t    enable;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN aggregation configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);

        if (RT_ERR_OK == rtk_vlan_portVlanAggrEnable_get(unit, port, &enable))
        {
            diag_util_mprintf("%s\n", enable ? "ENABLED" : "DISABLED");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_RANGE_CHECK_ENTRY_INDEX_VID
/*
 * vlan get vlan-conversion egress range-check <UINT:entry_index> vid
 */
cparser_result_t
cparser_cmd_vlan_get_vlan_conversion_egress_range_check_entry_index_vid(
    cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32                                  unit;
    int32                                   ret;
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t    vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vid_range, 0, sizeof(rtk_vlan_egrVlanCnvtRangeCheck_vid_t));

    ret = rtk_vlan_egrVlanCnvtRangeCheckVid_get(unit, *entry_index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN egress conversion vid range check index %d:\n",
            *entry_index_ptr);

    diag_util_mprintf("\tType: ");
    if (VLAN_TAG_TYPE_INNER == vid_range.vid_type)
    {
        diag_util_mprintf("Inner\n");
    }
    else
    {
        diag_util_mprintf("Outer\n");
    }

    diag_util_mprintf("\tLower: %d\n", vid_range.vid_lower_bound);
    diag_util_mprintf("\tUpper: %d\n", vid_range.vid_upper_bound);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_RANGE_CHECK_ENTRY_INDEX_VID_INNER_OUTER_LOWER_UPPER
/*
 * vlan set vlan-conversion egress range-check <UINT:entry_index> vid ( inner | outer ) <UINT:lower> <UINT:upper>
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_conversion_egress_range_check_entry_index_vid_inner_outer_lower_upper(
    cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *lower_ptr,
    uint32_t *upper_ptr)
{
    uint32                                  unit;
    int32                                   ret;
    rtk_vlan_tagType_t                      type;
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t    vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('i' == TOKEN_CHAR(7, 0))
    {
        type = VLAN_TAG_TYPE_INNER;
    }
    else
    {
        type = VLAN_TAG_TYPE_OUTER;
    }

    memset(&vid_range, 0, sizeof(rtk_vlan_egrVlanCnvtRangeCheck_vid_t));

    ret = rtk_vlan_egrVlanCnvtRangeCheckVid_get(unit, *entry_index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vid_range.vid_upper_bound   = *upper_ptr;
    vid_range.vid_lower_bound   = *lower_ptr;
    vid_range.vid_type          = type;
    ret = rtk_vlan_egrVlanCnvtRangeCheckVid_set(unit, *entry_index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_RANGE_CHECK_ENTRY_INDEX_VID_REVERSE_STATE
/*
 * vlan get vlan-conversion egress range-check <UINT:entry_index> vid reverse state
 */
cparser_result_t
cparser_cmd_vlan_get_vlan_conversion_egress_range_check_entry_index_vid_reverse_state(
    cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32                                  unit;
    int32                                   ret;
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t    vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vid_range, 0, sizeof(rtk_vlan_egrVlanCnvtRangeCheck_vid_t));

    ret = rtk_vlan_egrVlanCnvtRangeCheckVid_get(unit, *entry_index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN egress conversion vid range check index %d:\n",
            *entry_index_ptr);

    diag_util_mprintf("\tReverse: ");
    if (0 == vid_range.reverse)
    {
        diag_util_mprintf("Disabled\n");
    }
    else
    {
        diag_util_mprintf("Enabled\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_RANGE_CHECK_ENTRY_INDEX_VID_REVERSE_STATE_DISABLE_ENABLE
/*
 * vlan set vlan-conversion egress range-check <UINT:entry_index> vid reverse state ( disable | enable )
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_conversion_egress_range_check_entry_index_vid_reverse_state_disable_enable(
    cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32                                  unit;
    int32                                   ret;
    rtk_vlan_egrVlanCnvtRangeCheck_vid_t    vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&vid_range, 0, sizeof(rtk_vlan_egrVlanCnvtRangeCheck_vid_t));

    ret = rtk_vlan_egrVlanCnvtRangeCheckVid_get(unit, *entry_index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(9, 0))
    {
        vid_range.reverse = 1;
    }
    else
    {
        vid_range.reverse = 0;
    }

    ret = rtk_vlan_egrVlanCnvtRangeCheckVid_set(unit, *entry_index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_HASH_MODE_IVL_SVL
/*
 * vlan set vlan-table hash-mode ( ivl | svl )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_hash_mode_ivl_svl(cparser_context_t *context)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('i' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ucastLookupMode_set(unit, UC_LOOKUP_ON_VID), ret);
    }
    else if('s' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ucastLookupMode_set(unit, UC_LOOKUP_ON_FID), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_TABLE_HASH_MODE_UNICAST_MULTICAST_VID_VID
/*
 * vlan get vlan-table hash-mode ( unicast | multicast ) vid <UINT:vid>
 */
cparser_result_t cparser_cmd_vlan_get_vlan_table_hash_mode_unicast_multicast_vid_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_l2_ucastLookupMode_t ucast_mode;
    rtk_l2_mcastLookupMode_t mcast_mode;
    rtk_stg_t fid_msti;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*vid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    if('u' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_l2UcastLookupMode_get(unit, *vid_ptr, &ucast_mode), ret);
        DIAG_UTIL_ERR_CHK(rtk_vlan_stg_get(unit, *vid_ptr, &fid_msti), ret);

        diag_util_mprintf("VLAN learning method: ");
        if (ucast_mode == UC_LOOKUP_ON_VID)
            diag_util_mprintf("IVL\n");
        else
        {
            diag_util_mprintf("SVL, ");
            diag_util_mprintf("FID/MSTI: %d\n", fid_msti);
        }
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_l2McastLookupMode_get(unit, *vid_ptr, &mcast_mode), ret);
        DIAG_UTIL_ERR_CHK(rtk_vlan_stg_get(unit, *vid_ptr, &fid_msti), ret);

        diag_util_mprintf("VLAN learning method: ");
        if (mcast_mode == UC_LOOKUP_ON_VID)
            diag_util_mprintf("IVL\n");
        else
        {
            diag_util_mprintf("SVL, ");
            diag_util_mprintf("FID/MSTI: %d\n", fid_msti);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_HASH_MODE_IVL_UNICAST_MULTICAST_VID_VID
/*
 * vlan set vlan-table hash-mode ivl ( unicast | multicast ) vid <UINT:vid>
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_hash_mode_ivl_unicast_multicast_vid_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*vid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);

    if('u' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_l2UcastLookupMode_set(unit, *vid_ptr, UC_LOOKUP_ON_VID), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_l2McastLookupMode_set(unit, *vid_ptr, UC_LOOKUP_ON_VID), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_TABLE_HASH_MODE_SVL_UNICAST_MULTICAST_VID_VID_MSTI_MSTI
/*
 * vlan set vlan-table hash-mode svl ( unicast | multicast ) vid <UINT:vid> fid_msti <UINT:fid_msti>
 */
cparser_result_t cparser_cmd_vlan_set_vlan_table_hash_mode_svl_unicast_multicast_vid_vid_fid_msti_fid_msti(
    cparser_context_t *context,
    uint32_t *vid_ptr,
    uint32_t *fid_msti_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((*vid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);

    if('u' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_l2UcastLookupMode_set(unit, *vid_ptr, UC_LOOKUP_ON_FID), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_vlan_l2McastLookupMode_set(unit, *vid_ptr, UC_LOOKUP_ON_FID), ret);
    }

    DIAG_UTIL_ERR_CHK(rtk_vlan_stg_set(unit, *vid_ptr, *fid_msti_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_INGRESS_PORT_PORTS_ALL_INNER_OUTER_KEEP_TAG_KEEPTYPE_NOKEEP_FORMAT_CONTENT
/*
 * vlan set ingress port ( <PORT_LIST:ports> | all ) ( inner | outer ) keep-tag keeptype ( nokeep | format | content )
 */
cparser_result_t cparser_cmd_vlan_set_ingress_port_ports_all_inner_outer_keep_tag_keeptype_nokeep_format_content(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    int32           dir;    /* 0: inner, 1: outer */
    rtk_vlan_tagKeepType_t  inner_keeptype, outer_keeptype, new_keeptype;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ('i' == TOKEN_CHAR(5, 0))
    {
        dir = 0;
    }
    else
    {
        dir = 1;
    }

    if ('n' == TOKEN_CHAR(8, 0))
    {
        new_keeptype = TAG_KEEP_TYPE_NOKEEP;
    }
    else if('f' == TOKEN_CHAR(8,0))
    {
        new_keeptype = TAG_KEEP_TYPE_FORMAT;
    }
    else
    {
        new_keeptype = TAG_KEEP_TYPE_CONTENT;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        ret = rtk_vlan_portIgrTagKeepType_get(unit, port, &outer_keeptype,
                &inner_keeptype);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (0 == dir)
            inner_keeptype = new_keeptype;
        else
            outer_keeptype = new_keeptype;

        ret = rtk_vlan_portIgrTagKeepType_set(unit, port, outer_keeptype,
                inner_keeptype);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_EGRESS_PORT_PORTS_ALL_INNER_OUTER_KEEP_TAG_KEEPTYPE_NOKEEP_FORMAT_CONTENT
/*
 * vlan set egress port ( <PORT_LIST:ports> | all ) ( inner | outer ) keep-tag keeptype ( nokeep | format | content )
 */
cparser_result_t cparser_cmd_vlan_set_egress_port_ports_all_inner_outer_keep_tag_keeptype_nokeep_format_content(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    int32           dir;    /* 0: inner, 1: outer */
    rtk_vlan_tagKeepType_t  inner_keeptype, outer_keeptype, new_keeptype;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if ('i' == TOKEN_CHAR(5, 0))
    {
        dir = 0;
    }
    else
    {
        dir = 1;
    }

    if ('n' == TOKEN_CHAR(8, 0))
    {
        new_keeptype = TAG_KEEP_TYPE_NOKEEP;
    }
    else if('f' == TOKEN_CHAR(8,0))
    {
        new_keeptype = TAG_KEEP_TYPE_FORMAT;
    }
    else
    {
        new_keeptype = TAG_KEEP_TYPE_CONTENT;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        ret = rtk_vlan_portEgrTagKeepType_get(unit, port, &outer_keeptype,
                &inner_keeptype);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (0 == dir)
            inner_keeptype = new_keeptype;
        else
            outer_keeptype = new_keeptype;

        ret = rtk_vlan_portEgrTagKeepType_set(unit, port, outer_keeptype,
                inner_keeptype);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_AGGREGATION_PORT_PORTS_ALL_VID_SOURCE_INNER_OUTER
/*
  * vlan set vlan-aggregation port ( <PORT_LIST:ports> | all ) vid-source ( inner | outer )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_aggregation_port_ports_all_vid_source_inner_outer(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_vlanMode_t    src;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(6, 0))
    {
        src = BASED_ON_INNER_VLAN;
    }
    else
    {
        src = BASED_ON_OUTER_VLAN;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portVlanAggrVidSource_set(unit, port, src)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}

#endif

#ifdef CMD_VLAN_GET_VLAN_AGGREGATION_PORT_PORTS_ALL_VID_SOURCE
/*
  * vlan get vlan-aggregation port ( <PORT_LIST:ports> | all ) vid-source
  */
cparser_result_t cparser_cmd_vlan_get_vlan_aggregation_port_ports_all_vid_source(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    rtk_l2_vlanMode_t   src;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN aggregation configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);

        if (RT_ERR_OK == rtk_vlan_portVlanAggrVidSource_get(unit, port, &src))
        {
            diag_util_mprintf("Source Slection is %s\n", src ? "Outer" : "Inner");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }

    return CPARSER_OK;
}

#endif

#ifdef CMD_VLAN_SET_VLAN_AGGREGATION_PORT_PORTS_ALL_PRI_TAG_VID_SOURCE_PRIORITY_VID_PORT_BASED_VID
/*
  * vlan set vlan-aggregation port ( <PORT_LIST:ports> | all ) pri-tag vid-source ( priority-vid | port-based-vid )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_aggregation_port_ports_all_pri_tag_vid_source_priority_vid_port_based_vid(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_vlan_priTagVidSrc_t    src;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if (0 == strcmp(context->parser->tokens[7].buf, "priority-vid"))
    {
        src = LEARNING_VID_PRI;
    }
    else if (0 == strcmp(context->parser->tokens[7].buf, "port-based-vid"))
    {
        src = LEARNING_VID_PBASED;
    }
    else
    {
        src = CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portVlanAggrPriTagVidSource_set(unit, port, src)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_AGGREGATION_PORT_PORTS_ALL_PRI_TAG_VID_SOURCE
/*
  * vlan get vlan-aggregation port ( <PORT_LIST:ports> | all ) pri-tag vid-source
  */
cparser_result_t cparser_cmd_vlan_get_vlan_aggregation_port_ports_all_pri_tag_vid_source(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    rtk_vlan_priTagVidSrc_t   src;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN aggregation configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);

        if (RT_ERR_OK == rtk_vlan_portVlanAggrPriTagVidSource_get(unit, port, &src))
        {
            diag_util_mprintf("Priority VID Source Slection is %s\n", src ? "port-based VID" : "priority VID");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_PORT_PORTS_ALL_VID_SOURCE_INNER_OUTER
/*
 * vlan set vlan-conversion egress port ( <PORT_LIST:ports> | all ) vid-source ( inner | outer )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_port_ports_all_vid_source_inner_outer(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_vlanMode_t   src;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(7, 0))
    {
        src = BASED_ON_INNER_VLAN;
    }
    else
    {
        src = BASED_ON_OUTER_VLAN;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portEgrVlanCnvtVidSource_set(unit, port, src)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_PORT_PORTS_ALL_VID_SOURCE
/*
  * vlan get vlan-conversion egress port ( <PORT_LIST:ports> | all ) vid-source
  */
cparser_result_t cparser_cmd_vlan_get_vlan_conversion_egress_port_ports_all_vid_source(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    rtk_l2_vlanMode_t   src;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN Egress VLAN Translation configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);

        if (RT_ERR_OK == rtk_vlan_portEgrVlanCnvtVidSource_get(unit, port, &src))
        {
            diag_util_mprintf("Source Slection is %s\n", src ? "Outer" : "Inner");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_PORT_PORTS_ALL_VID_TARGET_INNER_OUTER
/*
 * vlan set vlan-conversion egress port ( <PORT_LIST:ports> | all ) vid-target ( inner | outer )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_port_ports_all_vid_target_inner_outer(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_vlanMode_t   tgt;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(7, 0))
    {
        tgt = BASED_ON_INNER_VLAN;
    }
    else
    {
        tgt = BASED_ON_OUTER_VLAN;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portEgrVlanCnvtVidTarget_set(unit, port, tgt)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_PORT_PORTS_ALL_VID_TARGET
/*
  * vlan get vlan-conversion egress port ( <PORT_LIST:ports> | all ) vid-target
  */
cparser_result_t cparser_cmd_vlan_get_vlan_conversion_egress_port_ports_all_vid_target(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    rtk_l2_vlanMode_t   tgt;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN Egress VLAN Translation configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);

        if (RT_ERR_OK == rtk_vlan_portEgrVlanCnvtVidTarget_get(unit, port, &tgt))
        {
            diag_util_mprintf("Target Slection is %s\n", tgt ? "Outer" : "Inner");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_VLAN_SET_VLAN_CONVERSION_EGRESS_PORT_PORTS_ALL_LOOKUP_MISS_ACTION_FORWARD_DROP
/*
 * vlan set vlan-conversion egress port ( <PORT_LIST:ports> | all ) lookup-miss-action ( forward | drop )
 */
cparser_result_t cparser_cmd_vlan_set_vlan_conversion_egress_port_ports_all_lookup_miss_action_forward_drop(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_action_t    action;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(7, 0))
    {
        action = LOOKUPMISS_DROP;
    }
    else
    {
        action = LOOKUPMISS_FWD;
    }

    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        if ((ret = rtk_vlan_portEgrVlanCnvtLookupMissAct_set(unit, port, action)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;

}
#endif

#ifdef CMD_VLAN_GET_VLAN_CONVERSION_EGRESS_PORT_PORTS_ALL_LOOKUP_MISS_ACTION
/*
  * vlan get vlan-conversion egress port ( <PORT_LIST:ports> | all ) lookup-miss-action
  */
cparser_result_t cparser_cmd_vlan_get_vlan_conversion_egress_port_ports_all_lookup_miss_action(cparser_context_t *context,
    char **ports_ptr)
{
    uint32          unit = 0;
    rtk_vlan_lookupMissAct_t action;
    rtk_port_t      port;
    diag_portlist_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_UTIL_EXTRACT_PORTLIST(portmask, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN conversion Lookup Miss action:\n");
    DIAG_UTIL_PORTMASK_SCAN(portmask, port)
    {
        diag_util_printf("Port %u : ", port);

        if (RT_ERR_OK == rtk_vlan_portEgrVlanCnvtLookupMissAct_get(unit, port, &action))
        {
            if (LOOKUPMISS_DROP == action)
                diag_util_mprintf("%s\n", "Drop");
            else
                diag_util_mprintf("%s\n", "Forward");
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }

    return CPARSER_OK;
}
#endif

