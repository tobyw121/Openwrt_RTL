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
 * $Revision: 22114 $
 * $Date: 2011-09-07 12:05:29 +0800 (Wed, 07 Sep 2011) $
 *
 * Purpose : Define diag shell functions for svlan.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) svlan diag shell.
 */


#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/util/rt_util.h>
#include <rtk/svlan.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_SVLAN_CREATE_DESTROY_SVID
/* 
 * svlan ( create | destroy ) <UINT:svid>
 */
cparser_result_t cparser_cmd_svlan_create_destroy_svid(cparser_context_t *context,
    uint32_t *svid_ptr)
{
    uint32      unit = 0;
    rtk_vlan_t  vid = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    vid = *svid_ptr;
    if ('c' == TOKEN_CHAR(1,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_svlan_create(unit, vid), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_svlan_destroy(unit, vid), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_SET_SVID_SVID_MEMBER_PORTLIST
/* 
 * svlan set svid <UINT:svid> member <PORT_LIST:portlist>
 */
cparser_result_t cparser_cmd_svlan_set_svid_svid_member_portlist(cparser_context_t *context,
    uint32_t *svid_ptr, char **portlist_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_vlan_t      svid = 0;
    rtk_portmask_t  svlan_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    svid = *svid_ptr;
    if ((ret = diag_util_str2LPortMask(*portlist_ptr, &svlan_portmask)) != RT_ERR_OK)
    {
        diag_util_printf("member port list ERROR!\n");
        RT_ERR(ret, (MOD_DIAGSHELL), "port list=%s", *portlist_ptr);
        return ret;
    }
    
    if ((ret = rtk_svlan_memberPort_set(unit, svid, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_SET_ENTRY_INDEX_SVID_SVID_MEMBER_PORTLIST
/*
 * svlan set entry <UINT:index> svid <UINT:svid> member <PORT_LIST:portlist>
 */
cparser_result_t cparser_cmd_svlan_set_entry_index_svid_svid_member_portlist(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *svid_ptr, char **portlist_ptr)
{
    uint32          unit = 0;
    uint32          svid_idx = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_vlan_t      svid = 0;
    rtk_portmask_t  svlan_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    svid_idx = *index_ptr;
    svid = *svid_ptr;
    DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask(TOKEN_STR(7), &svlan_portmask), ret);

    if ((ret = rtk_svlan_memberPortEntry_set(unit, svid_idx, svid, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_SET_TPID_TPID_VALUE
/*
 * svlan set tpid <UINT:tpid_value>
 */
cparser_result_t cparser_cmd_svlan_set_tpid_tpid_value(cparser_context_t *context,
    uint32_t *tpid_value_ptr)
{
    uint32  unit = 0;
    uint32  svlan_tag_id = 0;
    int32   ret = RT_ERR_FAILED;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    svlan_tag_id = *tpid_value_ptr;
    if ((ret = rtk_svlan_tpidEntry_set(unit, 0, svlan_tag_id)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_SET_PORT_PORT_ID_SVID_SVID
/*
 * svlan set port <UINT:port_id> svid <UINT:svid>
 */
cparser_result_t cparser_cmd_svlan_set_port_port_id_svid_svid(cparser_context_t *context,
    uint32_t *port_id_ptr, uint32_t *svid_ptr)
{
    uint32      unit = 0;
    rtk_vlan_t  svid = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    port = *port_id_ptr;
    svid = *svid_ptr;
    if ((ret = rtk_svlan_portSvid_set(unit, port, svid)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_SET_SERVICE_PORT_PORTLIST
/*
 * svlan set service-port <PORT_LIST:portlist>
 */
cparser_result_t cparser_cmd_svlan_set_service_port_portlist(cparser_context_t *context,
    char **portlist_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_portmask_t  svlan_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask(TOKEN_STR(3), &svlan_portmask), ret);
    
    if ((ret = rtk_svlan_servicePort_set(unit, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_GET_PORT_PORT_ALL_SVID
/*
 * svlan get port ( <PORT_LIST:port> | all ) svid
 */
cparser_result_t cparser_cmd_svlan_get_port_port_all_svid(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    rtk_vlan_t          svid = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {  
        if ((ret = rtk_svlan_portSvid_get(unit, port, &svid)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("Port %d svid : %d\n\n", port, svid);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_GET_SVID_SVID_MEMBER
/*
 * svlan get svid <UINT:svid> member 
 */
cparser_result_t cparser_cmd_svlan_get_svid_svid_member(cparser_context_t *context,
    uint32_t *svid_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_vlan_t      svid = 0;
    rtk_portmask_t  svlan_portmask;
    uint8           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    svid = *svid_ptr;

    /* show svlan information of specific svid */
    if ((ret = rtk_svlan_memberPort_get(unit, svid, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_lPortMask2str(port_list, &svlan_portmask);
        diag_util_mprintf("SVID %d : member port=%s\n", svid, port_list);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_GET_TPID
/*
 * svlan get tpid
 */
cparser_result_t cparser_cmd_svlan_get_tpid(cparser_context_t *context,
    uint32_t *svid_ptr)
{
    uint32          unit = 0;
    uint32          svlan_tag_id = 0;
    int32           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_svlan_tpidEntry_get(unit, 0, &svlan_tag_id)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("TPID : 0x%x\n", svlan_tag_id);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_GET_ENTRY_INDEX
/*
 * svlan get entry <UINT:index>
 */
cparser_result_t cparser_cmd_svlan_get_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          svid_idx = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_vlan_t      svid = 0;   
    rtk_portmask_t  svlan_portmask;
    uint8           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    svid_idx = *index_ptr;

    /* show svlan information of specific entry */
    diag_util_mprintf("svlan index %d :\n", svid_idx);
    if ((ret = rtk_svlan_memberPortEntry_get(unit, svid_idx, &svid, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_lPortMask2str(port_list, &svlan_portmask);
        diag_util_mprintf("   Member\t: %s \n", port_list);
    }    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_GET_SERVICE_PORT
/*
 * svlan get service-port
 */
cparser_result_t cparser_cmd_svlan_get_service_port(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
//    rtk_port_t      port = 0;
    rtk_portmask_t  svlan_portmask;
    uint8           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_svlan_servicePort_get(unit, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_lPortMask2str(port_list, &svlan_portmask);
        diag_util_mprintf("Service Port : %s\n", port_list);
    }    

    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_DUMP_ENTRY_INDEX
/*
 * svlan dump entry <UINT:index>
 */
cparser_result_t cparser_cmd_svlan_dump_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          svid_idx = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_vlan_t      svid = 0;   
    rtk_portmask_t  svlan_portmask;
    uint8           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    svid_idx = *index_ptr;

    /* show svlan information of specific entry */
    diag_util_mprintf("svlan index %d :\n", svid_idx);
    if ((ret = rtk_svlan_memberPortEntry_get(unit, svid_idx, &svid, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_lPortMask2str(port_list, &svlan_portmask);
        diag_util_mprintf("   Member\t: %s \n", port_list);
    }    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_DUMP_SVID_SVID
/*
 * svlan dump svid <UINT:svid>
 */
cparser_result_t cparser_cmd_svlan_dump_svid_svid(cparser_context_t *context,
    uint32_t *svid_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_vlan_t      svid = 0;
    rtk_portmask_t  svlan_portmask;
    uint8           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    svid = *svid_ptr;

    /* show svlan information of specific svid */
    if ((ret = rtk_svlan_memberPort_get(unit, svid, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_lPortMask2str(port_list, &svlan_portmask);
        diag_util_mprintf("SVID %d : member port=%s\n", svid, port_list);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_SVLAN_DUMP
/*
 * svlan dump
 */
cparser_result_t cparser_cmd_svlan_dump(cparser_context_t *context)
{
    uint32          unit = 0;
    uint32          svlan_tag_id = 0;
    rtk_vlan_t      svid = 0;
    int32           svid_idx = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_portmask_t  svlan_portmask;
    uint8           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_svlan_tpidEntry_get(unit, 0, &svlan_tag_id)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("TPID : 0x%x\n", svlan_tag_id);
    }
    
    if ((ret = rtk_svlan_servicePort_get(unit, &svlan_portmask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_lPortMask2str(port_list, &svlan_portmask);
        diag_util_mprintf("Service Port : %s\n", port_list);
    }
    
    if ((ret = rtk_switch_deviceInfo_get(unit, &devInfo)) != RT_ERR_OK)
    { 
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /* show all svlan info */
    for (port = devInfo.all.min; port <= devInfo.all.max; port++)
    {
        if ((ret = rtk_svlan_portSvid_get(unit, port, &svid)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("Port %d svid : %d\n", port, svid);
    }
    
    svid_idx = -1;
    while (rtk_svlan_nextValidMemberPortEntry_get(unit, &svid_idx, &svid, &svlan_portmask) == 0/*RT_ERR_OK*/)
    {
        diag_util_lPortMask2str(port_list, &svlan_portmask);
        diag_util_mprintf("SVID %d member port : %s\n", svid, port_list);
    }

    return CPARSER_OK;
}
#endif
