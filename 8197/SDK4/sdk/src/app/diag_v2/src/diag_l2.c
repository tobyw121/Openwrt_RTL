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
 * $Revision: 55230 $
 * $Date: 2015-01-26 21:56:17 +0800 (Mon, 26 Jan 2015) $
 *
 * Purpose : Definition those Layer2 command and APIs in the SDK diagnostic shell.
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
#include <rtk/vlan.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#define FWD_TABLE_MAX_IDX (1023)
#define L2_TABLE_AND_L2_CAM_MAX_IDX (16447)
#define MAX_MAC_ADDR_STR_LEN (64)

#ifdef CMD_L2_TABLE_SET_LINK_DOWN_FLUSH_STATE_DISABLE_ENABLE
/*
  * l2-table set link-down-flush state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_link_down_flush_state_disable_enable(cparser_context_t *context)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(4, 0))
    {
        /* set invalid link down enable */
        DIAG_UTIL_ERR_CHK(rtk_l2_flushLinkDownPortAddrEnable_set(unit, ENABLED), ret);
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        /* set invalid link down disable */
        DIAG_UTIL_ERR_CHK(rtk_l2_flushLinkDownPortAddrEnable_set(unit, DISABLED), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_PORT_PORT_ALL_STATE_DISABLE_ENABLE
/*
  * l2-table set limit-learning port ( <PORT_LIST:port> | all ) state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_port_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    rtk_enable_t enable;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        enable = DISABLED;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCntEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_PORT_PORT_ALL_MAX_COUNT
/*
  * l2-table set limit-learning port ( <PORT_LIST:port> | all ) <UINT:max_count>
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_port_port_all_max_count(cparser_context_t *context,
    char **port_ptr,
    uint32_t *max_count_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == max_count_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCnt_set(unit, port, *max_count_ptr), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_PORT_PORT_ALL_DROP_FORWARD_TRAP_TO_CPU
/*
  * l2-table set limit-learning port ( <PORT_LIST:port> | all ) ( drop | forward | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_port_port_all_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_l2_limitLearnCntAction_t    action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    diag_portlist_t             portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if ('f' == TOKEN_CHAR(5, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCntAction_set(unit, port, action), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_IP6_MCAST_SIP_DIP_VID_VLAN_ID_PORT_ALL
/*
  * l2-table add ip6-mcast <IPV6ADDR:sip> <IPV6ADDR:dip> vid <UINT:vlan_id> ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_add_ip6_mcast_sip_dip_vid_vlan_id_port_all(cparser_context_t *context,
    char **sip_ptr,
    char **dip_ptr,
    uint32_t *vlan_id_ptr,
    char **port_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_l2_ip6McastAddr_t   ip6_mcast_data;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);

    memset(&ip6_mcast_data, 0, sizeof(rtk_l2_ip6McastAddr_t));

    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.sip.ipv6_addr[0], (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.dip.ipv6_addr[0], (uint8 *)TOKEN_STR(4)), ret);
    ip6_mcast_data.rvid = *vlan_id_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 7), ret);
    ip6_mcast_data.portmask = portlist.portmask;

    DIAG_UTIL_ERR_CHK(rtk_l2_ip6McastAddr_add(unit, &ip6_mcast_data), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_UCAST_VID_MAC_PORT_PORT_AGG_VID_SA_BLOCK_DA_BLOCK_STATIC_NEXTHOP_SUSPEND
/*
 * l2-table add mac-ucast <UINT:vid> <MACADDR:mac> port <UINT:port> <UINT:agg_vid> { sa-block } { da-block } { static } { nexthop } { suspend }
 */
cparser_result_t cparser_cmd_l2_table_add_mac_ucast_vid_mac_port_port_agg_vid_sa_block_da_block_static_nexthop_suspend(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    uint32_t *port_ptr,
    uint32_t *agg_vid_ptr)
{
    uint32          flag_num = 0;
    uint32          sa_block = FALSE;
    uint32          da_block = FALSE;
    uint32          is_static = FALSE;
    uint32          nexthop = FALSE;
    uint32          suspend = FALSE;
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ucastAddr_t  l2_uAddr;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == port_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == agg_vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l2_uAddr, 0, sizeof(rtk_l2_ucastAddr_t));

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    /*from first optional token*/
    for(flag_num = 8; flag_num < TOKEN_NUM; flag_num++)
    {
        if ('s' == TOKEN_CHAR(flag_num, 0))
        {
            if ('a' == TOKEN_CHAR(flag_num, 1))
            {
                sa_block = TRUE;
            }
            else if ('t' == TOKEN_CHAR(flag_num, 1))
            {
                is_static = TRUE;
            }
            else if ('u' == TOKEN_CHAR(flag_num, 1))
            {
                suspend = TRUE;
            }
        }
        else if ('d' == TOKEN_CHAR(flag_num, 0))
        {
            da_block = TRUE;
        }
        else if ('n' == TOKEN_CHAR(flag_num, 0))
        {
            nexthop = TRUE;
        }
    }

    /* Fill structure */
    l2_uAddr.vid = *vid_ptr;
    memcpy(l2_uAddr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2_uAddr.port = *port_ptr;
    if(sa_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_SA_BLOCK;

    if(da_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_DA_BLOCK;

    if(is_static)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_STATIC;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_STATIC;

    if(nexthop)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_NEXTHOP;

    if(suspend)
        l2_uAddr.state |= RTK_L2_UCAST_STATE_SUSPEND;
    else
        l2_uAddr.state&= ~RTK_L2_UCAST_STATE_SUSPEND;

    l2_uAddr.agg_vid = *agg_vid_ptr;

    DIAG_UTIL_ERR_CHK(rtk_l2_addr_add(unit, &l2_uAddr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_FID_INDEX_INDEX_STATE_DISABLE_ENABLE
/*
  * l2-table set limit-learning fid-index <UINT:index> state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_fid_index_index_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_fidMacLimitEntry_t   fidMacLimitEntry;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);

    memset(&fidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    if('e' == TOKEN_CHAR(6, 0))
        fidMacLimitEntry.enable = ENABLED;
    else if('d' == TOKEN_CHAR(6, 0))
        fidMacLimitEntry.enable = DISABLED;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_set(unit, *index_ptr, &fidMacLimitEntry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_FID_INDEX_INDEX_FID_MAX_COUNT_PORT_ALL
/*
  * l2-table set limit-learning fid-index <UINT:index> <UINT:fid> <UINT:max_count> ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_fid_index_index_fid_max_count_port_all(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *fid_ptr,
    uint32_t *max_count_ptr,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_fidMacLimitEntry_t   fidMacLimitEntry;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == fid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*fid_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == max_count_ptr), CPARSER_ERR_INVALID_PARAMS);

    memset(&fidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *index_ptr, &fidMacLimitEntry), ret);

    fidMacLimitEntry.fid = *fid_ptr;
    fidMacLimitEntry.maxNum = *max_count_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 7), ret);
    fidMacLimitEntry.portmask = portlist.portmask;

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_set(unit, *index_ptr, &fidMacLimitEntry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_FID_INDEX_INDEX_DROP_FORWARD_TRAP_TO_CPU
/*
  * l2-table set limit-learning fid-index <UINT:index> ( drop | forward |  trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_fid_index_index_drop_forward_trap_to_cpu(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_fidMacLimitEntry_t   fidMacLimitEntry;
    rtk_switch_devInfo_t devInfo;

    DIAG_OM_GET_CHIP_ID(unit);

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);

    memset(&fidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *index_ptr, &fidMacLimitEntry), ret);

    if ('f' == TOKEN_CHAR(5, 0))
    {
        fidMacLimitEntry.action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5, 0))
    {
        fidMacLimitEntry.action = LIMIT_LEARN_CNT_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5, 0))
    {
        fidMacLimitEntry.action  = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_set(unit, *index_ptr, &fidMacLimitEntry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_PRIORITY_STATE_DISABLE_ENABLE
/*
  * l2-table set limit-learning priority state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_priority_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapPriEnable_set(unit, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapPriEnable_set(unit, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_PRIORITY_PRI
/*
  * l2-table set limit-learning priority <UINT:pri>
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_priority_pri(cparser_context_t *context,
uint32_t *pri_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == pri_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapPri_set(unit, *pri_ptr), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_DROP_PRECEDENCE_STATE_DISABLE_ENABLE
/*
  * l2-table set limit-learning drop-precedence state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_drop_precedence_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapDPEnable_set(unit, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapDPEnable_set(unit, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_DROP_PRECEDENCE_DP
/*
  * l2-table set limit-learning drop-precedence <UINT:dp>
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_drop_precedence_dp(cparser_context_t *context,
uint32_t *dp_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dp_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*dp_ptr > RTK_DROP_PRECEDENCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapDP_set(unit, *dp_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_INSERT_CPUTAG_STATE_DISABLE_ENABLE
/*
  * l2-table set limit-learning insert-cputag state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_insert_cputag_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapAddCPUTagEnable_set(unit, ENABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapAddCPUTagEnable_set(unit, DISABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_AGING_TIME_TIME
/*
  *  l2-table set aging-time <UINT:time>
  */
cparser_result_t cparser_cmd_l2_table_set_aging_time_time(cparser_context_t *context,
    uint32_t *time_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == time_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    /* set aging time */
    DIAG_UTIL_ERR_CHK(rtk_l2_aging_set(unit, *time_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_CAM_L2_TABLE_LRU_IPMC_STATE_DISABLE_ENABLE
/*
  *  l2-table set ( cam | l2-table-lru | ipmc ) state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_cam_l2_table_lru_ipmc_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_enable_t                 enable = FALSE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(4, 0))
        enable = TRUE;
    else if('d' == TOKEN_CHAR(4, 0))
        enable = FALSE;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('c' == TOKEN_CHAR(2, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_camEnable_set(unit, enable), ret);
    }
    else if('l' == TOKEN_CHAR(2, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_LRUEnable_set(unit, enable), ret);
    }
    else if('i' == TOKEN_CHAR(2, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcEnable_set(unit, enable), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_SECURE_MAC_MODE_DISABLE_ENABLE
/*
  * l2-table set secure-mac-mode ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_secure_mac_mode_disable_enable(cparser_context_t *context)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(3, 0))
    {
        /* set invalid link down enable */
        DIAG_UTIL_ERR_CHK(rtk_l2_secureMacMode_set(unit, ENABLED), ret);
    }
    else if ('d' == TOKEN_CHAR(3, 0))
    {
        /* set invalid link down disable */
        DIAG_UTIL_ERR_CHK(rtk_l2_secureMacMode_set(unit, DISABLED), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_SRC_MAC_LEARNING_PORT_ALL_STATE_DISABLE_ENABLE
/*
  * l2-table set src-mac-learning ( <PORT_LIST:port> | all ) state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_src_mac_learning_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_learningEnable_set(unit, port, ENABLED), ret);
        }

    }
    else if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_learningEnable_set(unit, port, DISABLED), ret);
        }
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_HASH_ALGORITHM_ALGO0_ALGO1
/*
  * l2-table set hash-algorithm ( algo0 | algo1 )
  */
cparser_result_t cparser_cmd_l2_table_set_hash_algorithm_algo0_algo1(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    uint32                          hashType = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('0' == TOKEN_CHAR(3, 4))
        hashType = 0;
    else if('1' == TOKEN_CHAR(3, 4))
        hashType = 1;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_hashAlgo_set(unit, hashType), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IP_MCAST_DIP_CHECK_DISABLE_ENABLE
/*
  * l2-table set ip-mcast dip-check ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_ip_mcast_dip_check_disable_enable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('d' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddrChkEnable_set(unit, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddrChkEnable_set(unit, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IP_MCAST_VLAN_COMPARE_DISABLE_ENABLE
/*
  * l2-table set ip-mcast vlan-compare ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_ip_mcast_vlan_compare_disable_enable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('d' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipMcstFidVidCompareEnable_set(unit, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipMcstFidVidCompareEnable_set(unit, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MODE_DIP_ONLY_DIP_SIP
/*
  * l2-table set ipmc-mode ( dip-only | dip-sip )
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mode_dip_only_dip_sip(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('o' == TOKEN_CHAR(3, 4))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcMode_set(unit, LOOKUP_ON_DIP_ONLY), ret);
    }
    else if('s' == TOKEN_CHAR(3, 4))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcMode_set(unit, LOOKUP_ON_DIP_AND_SIP), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_VLAN_FWD_MODE_PORT_ALL_INNER_VID_BASED_OUTER_VID_BASED
/*
  * l2-table set vlan-fwd-mode ( <PORT_LIST:port> | all ) ( inner-vid-based | outer-vid-based )
  */
cparser_result_t cparser_cmd_l2_table_set_vlan_fwd_mode_port_all_inner_vid_based_outer_vid_based(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('i' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_vlanMode_set(unit, port, BASED_ON_INNER_VLAN), ret);
        }

    }
    else if('o' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_vlanMode_set(unit, port, BASED_ON_OUTER_VLAN), ret);
        }
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_SRC_MAC_PORT_ALL_ASIC_LEARN_SOFTWARE_LEARN_NOT_LEARN_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
  * l2-table set src-mac ( <PORT_LIST:port> | all ) ( asic-learn | software-learn | not-learn )  (  copy-to-cpu | drop | forward | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_src_mac_port_all_asic_learn_software_learn_not_learn_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_l2_newMacLrnMode_t  mode  = HARDWARE_LEARNING;
    rtk_action_t                  action = ACTION_FORWARD;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('a' == TOKEN_CHAR(4, 0))
        mode = HARDWARE_LEARNING;
    else if('s' == TOKEN_CHAR(4, 0))
        mode = SOFTWARE_LEARNING;
    else if('n' == TOKEN_CHAR(4, 0))
        mode = NOT_LEARNING;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('f' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('c' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_newMacOp_set(unit, port, mode, action), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_ROUTER_PORT_PORT_ALL
/*
  * l2-table set router-port ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_set_router_port_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_portmask_t portmask;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    portmask = portlist.portmask;

    DIAG_UTIL_ERR_CHK(rtk_l2_ipmc_routerPorts_set(unit, &portmask), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_DROP_FWD_AS_IPMC_FWD_AS_L2_TRAP_TO_CPU
/*
  * l2-table set ipmc-mismatch ( wrong-mcast-address | ipmc-with-ucast-mac )  ( drop | fwd-as-ipmc | fwd-as-l2 | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_drop_fwd_as_ipmc_fwd_as_l2_trap_to_cpu(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_ipmc_mismatchType_t     type;
    rtk_l2_ipmcMismatch_action_t     action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('w' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR;
    else if('i' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_UCAST_ADDR;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(4, 0))
        action = L2_IPMC_MIS_DROP;
    else if('t' == TOKEN_CHAR(4, 0))
        action = L2_IPMC_MIS_TRAP;
    else if('l' == TOKEN_CHAR(4, 7))
        action = L2_IPMC_MIS_FWD_AS_L2;
    else if('i' == TOKEN_CHAR(4, 7))
        action = L2_IPMC_MIS_FWD_AS_IPMC;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchAction_set(unit, type, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_PRIORITY_STATE_DISABLE_ENABLE
/*
  * l2-table set ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) priority state ( disable | enable)
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_priority_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_ipmc_mismatchType_t     type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('w' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR;
    else if('i' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_UCAST_ADDR;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(6, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPriEnable_set(unit, type, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPriEnable_set(unit, type, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_PRIORITY_PRI
/*
  * l2-table set ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) priority <UINT:pri>
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_priority_pri(cparser_context_t *context,
uint32_t *pri_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_ipmc_mismatchType_t     type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == pri_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);

    if('w' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR;
    else if('i' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_UCAST_ADDR;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPri_set(unit, type, *pri_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_DROP_PRECEDENCE_STATE_DISABLE_ENABLE
/*
  * l2-table set ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) drop-precedence state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_drop_precedence_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_ipmc_mismatchType_t     type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('w' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR;
    else if('i' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_UCAST_ADDR;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('d' == TOKEN_CHAR(6, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDPEnable_set(unit, type, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDPEnable_set(unit, type, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_DROP_PRECEDENCE_DP
/*
  * l2-table set ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) drop-precedence <UINT:dp>
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_drop_precedence_dp(cparser_context_t *context,
uint32_t *dp_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_ipmc_mismatchType_t     type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dp_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*dp_ptr > RTK_DROP_PRECEDENCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    if('w' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR;
    else if('i' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_UCAST_ADDR;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDP_set(unit, type, *dp_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_INSERT_CPUTAG_STATE_DISABLE_ENABLE
/*
  * l2-table set ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) insert-cputag state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_insert_cputag_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_ipmc_mismatchType_t     type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('w' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR;
    else if('i' == TOKEN_CHAR(3, 0))
        type = L2_IPMC_MIS_TYPE_UCAST_ADDR;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(unit, type, ENABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(unit, type,  DISABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_PORT_PORT_ID_INCLUDE_STATIC
/*
  *  l2-table set flush mac-ucast port <UINT:port_id> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_port_port_id_include_static(cparser_context_t *context,
    uint32_t *port_id_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == port_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    config.act = 0;
#endif
    config.flushByPort = TRUE;
    config.portOrTrunk = TRUE; /* port-based */
    config.port = *port_id_ptr;

    if('i' == TOKEN_CHAR(6, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_TRUNK_TRUNK_ID_INCLUDE_STATIC
/*
 * l2-table set flush mac-ucast trunk <UINT:trunk_id> { include-static }
 */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_trunk_trunk_id_include_static(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == trunk_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByPort = TRUE;
    config.portOrTrunk = FALSE; /* trunk-based */
    config.port = *trunk_id_ptr;

    if('i' == TOKEN_CHAR(6, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_PORT_PORT_ID_VID_VLAN_ID_INCLUDE_STATIC
/*
  *  l2-table set flush mac-ucast port <UINT:port_id> vid <UINT:vlan_id> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_port_port_id_vid_vlan_id_include_static(cparser_context_t *context,
    uint32_t *port_id_ptr,
    uint32_t *vlan_id_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == port_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    config.act = 0;
#endif
    config.flushByPort = TRUE;
    config.portOrTrunk = TRUE; /* port-based */
    config.port = *port_id_ptr;
    config.flushByVid = TRUE;
    config.vid = *vlan_id_ptr;

    if('i' == TOKEN_CHAR(8, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_TRUNK_TRUNK_ID_VID_VLAN_ID_INCLUDE_STATIC
/*
 * l2-table set flush mac-ucast trunk <UINT:trunk_id> vid <UINT:vlan_id> { include-static }
 */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_trunk_trunk_id_vid_vlan_id_include_static(cparser_context_t *context,
    uint32_t *trunk_id_ptr,
    uint32_t *vlan_id_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == trunk_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByPort = TRUE;
    config.portOrTrunk = FALSE; /* trunk-based */
    config.port = *trunk_id_ptr;
    config.flushByVid = TRUE;
    config.vid = *vlan_id_ptr;

    if('i' == TOKEN_CHAR(8, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_PORT_PORT_ID_VID_VLAN_ID_MAC_INCLUDE_STATIC
/*
  * l2-table set flush mac-ucast port <UINT:port_id> vid <UINT:vlan_id> <MACADDR:mac> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_port_port_id_vid_vlan_id_mac_include_static(cparser_context_t *context,
    uint32_t *port_id_ptr,
    uint32_t *vlan_id_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == port_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByPort = TRUE;
    config.portOrTrunk = TRUE; /* port-based */
    config.port = *port_id_ptr;
    config.flushByVid = TRUE;
    config.vid = *vlan_id_ptr;
    config.flushByMac = TRUE;
    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) ||
        (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }
    memcpy(&config.ucastAddr.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    if('i' == TOKEN_CHAR(9, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_TRUNK_TRUNK_ID_VID_VLAN_ID_MAC_INCLUDE_STATIC
/*
 * l2-table set flush mac-ucast trunk <UINT:trunk_id> vid <UINT:vlan_id> <MACADDR:mac> { include-static }
 */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_trunk_trunk_id_vid_vlan_id_mac_include_static(cparser_context_t *context,
    uint32_t *trunk_id_ptr,
    uint32_t *vlan_id_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == trunk_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByPort = TRUE;
    config.portOrTrunk = FALSE; /* trunk-based */
    config.port = *trunk_id_ptr;
    config.flushByVid = TRUE;
    config.vid = *vlan_id_ptr;
    config.flushByMac = TRUE;
    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) ||
        (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }
    memcpy(&config.ucastAddr.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    if('i' == TOKEN_CHAR(9, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_PORT_PORT_ID_MAC_INCLUDE_STATIC
/*
  *  l2-table set flush mac-ucast port <UINT:port_id> <MACADDR:mac> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_port_port_id_mac_include_static(cparser_context_t *context,
    uint32_t *port_id_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == port_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByPort = TRUE;
    config.portOrTrunk = TRUE; /* port-based */
    config.port = *port_id_ptr;
    config.flushByMac = TRUE;
    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) ||
        (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }
    memcpy(&config.ucastAddr.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    if('i' == TOKEN_CHAR(7, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_TRUNK_TRUNK_ID_MAC_INCLUDE_STATIC
/*
 * l2-table set flush mac-ucast trunk <UINT:trunk_id> <MACADDR:mac> { include-static }
 */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_trunk_trunk_id_mac_include_static(cparser_context_t *context,
    uint32_t *trunk_id_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == trunk_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByPort = TRUE;
    config.portOrTrunk = FALSE; /* trunk-based */
    config.port = *trunk_id_ptr;
    config.flushByMac = TRUE;
    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) ||
        (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }
    memcpy(&config.ucastAddr.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    if('i' == TOKEN_CHAR(7, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_VID_VLAN_ID_INCLUDE_STATIC
/*
  * l2-table set flush mac-ucast vid <UINT:vlan_id> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_vid_vlan_id_include_static(cparser_context_t *context,
    uint32_t *vlan_id_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    config.act = 0;
#endif
    config.flushByVid = TRUE;
    config.vid = *vlan_id_ptr;

    if('i' == TOKEN_CHAR(6, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_VID_VLAN_ID_MAC_INCLUDE_STATIC
/*
  * l2-table set flush mac-ucast vid <UINT:vlan_id> <MACADDR:mac> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_vid_vlan_id_mac_include_static(cparser_context_t *context,
    uint32_t *vlan_id_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByVid = TRUE;
    config.vid = *vlan_id_ptr;
    config.flushByMac = TRUE;
    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) ||
        (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }
    memcpy(&config.ucastAddr.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    if('i' == TOKEN_CHAR(7, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_MAC_INCLUDE_STATIC
/*
  *  l2-table set flush mac-ucast <MACADDR:mac> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_mac_include_static(cparser_context_t *context,
    cparser_macaddr_t *mac_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));

    config.flushByMac = TRUE;
    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) ||
        (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }
    memcpy(&config.ucastAddr.octet[0], &mac_ptr->octet[0], sizeof(cparser_macaddr_t));

    if('i' == TOKEN_CHAR(5, 0))
        config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FLUSH_MAC_UCAST_ALL_INCLUDE_STATIC
/*
  *  l2-table set flush mac-ucast all { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_flush_mac_ucast_all_include_static(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32         include_static = FALSE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('i' == TOKEN_CHAR(5, 0))
        include_static = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_addr_delAll(unit, include_static), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LEGAL_MOVE_SRC_PORT_ALL_DST_PORT_ALL
/*
  * l2-table set legal-move ( <PORT_LIST:src_port> | all ) ( <PORT_LIST:dst_port> | all )
  */
cparser_result_t cparser_cmd_l2_table_set_legal_move_src_port_all_dst_port_all(cparser_context_t *context,
    char **src_port_ptr,
    char **dst_port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32         port;
    rtk_portmask_t  portmask;
    diag_portlist_t    portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    /*get target port mask*/
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    portmask = portlist.portmask;

    /*get src port mask*/
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_legalMoveToPorts_set(unit, port, &portmask), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_PORT_MOVE_ILLEGAL_LEGAL_PORT_ALL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
  * l2-table set port-move ( illegal | legal )  ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_port_move_illegal_legal_port_all_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32                           unit = 0;
    int32                             ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_action_t                   action  = ACTION_FORWARD;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('f' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('c' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('i' == TOKEN_CHAR(3, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_illegalPortMoveAction_set(unit, port, action), ret);
        }
    }
    else
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveAction_set(unit, port, action), ret);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_BCAST_L2MC_IPMC_UNICAST_DROP_FLOOD_IN_VLAN_FLOOD_TO_ALL_PORT_FLOOD_TO_ROUTER_PORT_TRAP_TO_CPU
/*
  * l2-table set lookup-miss  ( bcast | l2mc | ipmc | unicast )  ( drop | flood-in-vlan | flood-to-all-port | flood-to-router-port | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_bcast_l2mc_ipmc_unicast_drop_flood_in_vlan_flood_to_all_port_flood_to_router_port_trap_to_cpu(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t type = DLF_TYPE_UCAST;
    rtk_action_t                  action  = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('u' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_UCAST;
    }
    else if ('l' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_MCAST;
    }
    else if ('i' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_IPMC;
    }
    else if ('b' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_BCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('d' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(4, 0))
    {
        if('v' == TOKEN_CHAR(4, 9))
            action = ACTION_FLOOD_IN_VLAN;
        else if('a' == TOKEN_CHAR(4, 9))
            action = ACTION_FLOOD_IN_ALL_PORT;
        else
            action = ACTION_FLOOD_IN_ROUTER_PORTS;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissAction_set(unit, type, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_PRIORITY_STATE_DISABLE_ENABLE
/*
  * l2-table set lookup-miss priority state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_priority_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPriEnable_set(unit, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPriEnable_set(unit, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_PRIORITY_PRI
/*
  * l2-table set lookup-miss priority <UINT:pri>
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_priority_pri(cparser_context_t *context,
uint32_t *pri_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == pri_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPri_set(unit, *pri_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_DROP_PRECEDENCE_STATE_DISABLE_ENABLE
/*
  * l2-table set lookup-miss  drop-precedence state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_drop_precedence_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDPEnable_set(unit, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDPEnable_set(unit, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_DROP_PRECEDENCE_DP
/*
  * l2-table set lookup-miss  drop-precedence <UINT:dp>
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_drop_precedence_dp(cparser_context_t *context,
uint32_t *dp_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dp_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*dp_ptr > RTK_DROP_PRECEDENCE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDP_set(unit, *dp_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_INSERT_CPUTAG_STATE_DISABLE_ENABLE
/*
  * l2-table set lookup-miss insert-cputag state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_insert_cputag_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissAddCPUTagEnable_set(unit, ENABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissAddCPUTagEnable_set(unit, DISABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_FLOOD_PORT_ALL
/*
  * l2-table set lookup-miss ( <PORT_LIST:flood_port> | all )
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_flood_port_all(cparser_context_t *context,
    char **flood_port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_portmask_t             portmask;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    portmask = portlist.portmask;

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissFloodPortMask_set(unit, DLF_TYPE_ANY, &portmask), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_EXCEPT_SMAC_BCAST_SA_MCAST_SA_DROP_FORWARD
/*
  *  l2-table set except-smac ( bcast-sa | mcast-sa ) ( drop | forward )
  */
cparser_result_t cparser_cmd_l2_table_set_except_smac_bcast_sa_mcast_sa_drop_forward(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t type = DLF_TYPE_UCAST;
    rtk_action_t    action  = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('m' == TOKEN_CHAR(3, 0))
    {
        type = SA_IS_MCAST;
    }
    else if ('b' == TOKEN_CHAR(3, 0))
    {
        type = SA_IS_BCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_DROP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_exceptionAddrAction_set(unit, type, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_EXCEPT_SMAC_ZERO_SA_DROP_FORWARD
/*
  *  l2-table set except-smac zero-sa ( drop | forward )
  */
cparser_result_t cparser_cmd_l2_table_set_except_smac_zero_sa_drop_forward(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t type = DLF_TYPE_UCAST;
    rtk_action_t    action  = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    type = SA_IS_ZERO;

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_DROP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_exceptionAddrAction_set(unit, type, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_SRC_PORT_EGRESS_FILTER_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * l2-table set src-port-egress-filter ( <PORT_LIST:port> | all ) state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_src_port_egress_filter_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_portmask_t  ori_pmask, portmask;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    memset(&portmask, 0, sizeof(rtk_portmask_t));

    DIAG_UTIL_ERR_CHK(rtk_l2_srcPortEgrFilterMask_get(unit, &ori_pmask), ret);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    portmask = portlist.portmask;

    if('e' == TOKEN_CHAR(5, 0))
    {
        RTK_PORTMASK_OR(ori_pmask, portmask);
    }
    else
    {
        RTK_PORTMASK_REMOVE(ori_pmask, portmask);
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_srcPortEgrFilterMask_set(unit, &ori_pmask), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_L2_TRAP_PRIORITY_STATE_DISABLE_ENABLE
/*
  * l2-table set l2-trap priority state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_l2_trap_priority_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_trapPriEnable_set(unit, DISABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_trapPriEnable_set(unit, ENABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_L2_TRAP_PRIORITY_PRI
/*
  * l2-table set l2-trap priority <UINT:pri>
  */
cparser_result_t cparser_cmd_l2_table_set_l2_trap_priority_pri(cparser_context_t *context,
uint32_t *pri_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == pri_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_l2_trapPri_set(unit, *pri_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_L2_TRAP_INSERT_CPUTAG_STATE_DISABLE_ENABLE
/*
  * l2-table set l2-trap  insert-cputag state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_l2_trap_insert_cputag_state_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_trapAddCPUTagEnable_set(unit, ENABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_trapAddCPUTagEnable_set(unit, DISABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_FWD_TABLE_INDEX_PORT_ALL_CROSS_VLAN
/*
  *l2-table set fwd-table  <UINT:index>  ( <PORT_LIST:port> | all ) { cross-vlan }
  */
cparser_result_t cparser_cmd_l2_table_set_fwd_table_index_port_all_cross_vlan(cparser_context_t *context,
    uint32_t *index_ptr,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_portmask_t portmask;
    diag_portlist_t               portlist;
    uint32              crossVlan = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > FWD_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    portmask = portlist.portmask;

    if(TOKEN_NUM == 6)
    {
        if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
        {
            diag_util_mprintf("cross-vlan is not Supported\n");
            return CPARSER_NOT_OK;
        }
        crossVlan = TRUE;
    }
    else
        crossVlan = FALSE;

    if ((ret = rtk_l2_mcastFwdPortmask_set(unit, *index_ptr, &portmask, crossVlan)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_MCAST_BLOCK_PORT_PORT
/*
  * l2-table set mcast-block-port <PORT_LIST:port>
  */
cparser_result_t cparser_cmd_l2_table_set_mcast_block_port_port(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_portmask_t portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == *port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&portmask, 0, sizeof(rtk_portmask_t));

    diag_util_str2LPortMask((uint8 *)*port_ptr, &portmask);

    DIAG_UTIL_ERR_CHK(rtk_l2_mcastBlockPortmask_set(unit, &portmask), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_MCAST_MODE_FIX_FID_FID
/*
 * l2-table set mcast-mode fix-fid <UINT:fid>
 */
cparser_result_t cparser_cmd_l2_table_set_mcast_mode_fix_fid_fid(cparser_context_t *context,
    uint32_t *fid_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_mcastLookupMode_set(unit, MC_LOOKUP_ON_FIXED_FID, *fid_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_MCAST_MODE_FID_VID
/*
 * l2-table set mcast-mode ( fid | vid )
 */
cparser_result_t cparser_cmd_l2_table_set_mcast_mode_fid_vid(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('f' == TOKEN_CHAR(3, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_mcastLookupMode_set(unit, MC_LOOKUP_ON_FID, 0), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_mcastLookupMode_set(unit, MC_LOOKUP_ON_VID, 0), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_PORT_PORT_ALL
/*
  *  l2-table get limit-learning port ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_port_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    diag_portlist_t               portlist;
    rtk_port_t                      port = 0;
    rtk_enable_t                 enable;
    rtk_l2_limitLearnCntAction_t  action;
    uint32                          mac_cnt, mac_cnt_disable;
    rtk_fid_t                       fid;
    rtk_mac_t                     mac;
    uint8                             macStr[MAX_MAC_ADDR_STR_LEN];

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == *port_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Limit Learning Information:\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("    Port %2d :\n", port);

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID)
#if defined(CONFIG_SDK_DRIVER_RTK_BACKWARD_COMPATIBLE_8328)
            || DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID)
            || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID)
#endif
            )
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCntEnable_get(unit, port, &enable), ret);
            diag_util_mprintf("        Status                  : %s\n", enable ? "ENABLE" : "DISABLE");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCnt_get(unit, port, &mac_cnt), ret);
    	DIAG_OM_GET_CHIP_CAPACITY(unit, mac_cnt_disable, l2_learn_limit_cnt_disable);
    	if (mac_cnt == mac_cnt_disable)
    		diag_util_mprintf("        Max Mac Count           : Unlimited\n");
    	else
    	    diag_util_mprintf("        Max Mac Count           : %u\n", mac_cnt);

        DIAG_UTIL_ERR_CHK(rtk_l2_portLearningCnt_get(unit, port, &mac_cnt), ret);

        diag_util_mprintf("        Current Mac Count       : %u\n", mac_cnt);

        DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCntAction_get(unit, port, &action), ret);

        diag_util_mprintf("        Exceed Max Count Action : ");

        if (action == LIMIT_LEARN_CNT_ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (action == LIMIT_LEARN_CNT_ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == LIMIT_LEARN_CNT_ACTION_TO_CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
        DIAG_UTIL_ERR_CHK(rtk_l2_portLastLearnedMac_get(unit, port, &fid, &mac), ret);
        DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, mac.octet), ret);
        diag_util_mprintf("        Last Packet Info        : Fid(%u), Src Mac Address(%s)\n", fid, macStr);
        }

        diag_util_mprintf("\n");

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_FID_INDEX_INDEX
/*
  *  l2-table get limit-learning fid-index <UINT:index>
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_fid_index_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l2_fidMacLimitEntry_t   fidMacLimitEntry;
    uint32                          mac_cnt;
    rtk_fid_t                       fid;
    rtk_mac_t                     mac;
    uint8                            portStr[80];
    uint8                            macStr[MAX_MAC_ADDR_STR_LEN];
    rtk_switch_devInfo_t     devInfo;

    DIAG_OM_GET_CHIP_ID(unit);

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    memset(&fidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *index_ptr, &fidMacLimitEntry), ret);

    diag_util_mprintf("Show Vlan Limit Learning information :\n");
    diag_util_mprintf("    vlan limit learning entry index : %u\n", *index_ptr);

     if(fidMacLimitEntry.enable == ENABLED)
     {
        diag_util_mprintf("        Status                  : ENABLE\n");
        diag_util_mprintf("        Fid                     : %u\n", fidMacLimitEntry.fid);
        diag_util_mprintf("        Max Mac Count           : %u\n", fidMacLimitEntry.maxNum);

        if ((ret = rtk_l2_fidLearningCnt_get(unit, *index_ptr, &mac_cnt)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("        Current Mac Count       : %u\n", mac_cnt);

        diag_util_lPortMask2str(portStr, &fidMacLimitEntry.portmask);

        diag_util_mprintf("        Including Ports         : %s\n", portStr);

        diag_util_mprintf("        Exceed Max Count Action : ");

        if (fidMacLimitEntry.action == LIMIT_LEARN_CNT_ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (fidMacLimitEntry.action == LIMIT_LEARN_CNT_ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (fidMacLimitEntry.action == LIMIT_LEARN_CNT_ACTION_TO_CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_fidLastLearnedMac_get(unit, *index_ptr, &fid, &mac), ret);
        DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, mac.octet), ret);
        diag_util_mprintf("        Last Packet Info        : Fid(%u), Src Mac Address(%s)\n", fid, macStr);

     }
     else
     {
        diag_util_mprintf("        Status : DISABLE\n");
     }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_FID_INDEX_INDEX_STATE
/*
  *  l2-table get limit-learning fid-index <UINT:index> state
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_fid_index_index_state(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l2_fidMacLimitEntry_t   fidMacLimitEntry;
    uint32                          mac_cnt;
    rtk_fid_t                       fid;
    rtk_mac_t                     mac;
    uint8                            portStr[80];
    uint8                            macStr[MAX_MAC_ADDR_STR_LEN];
    rtk_switch_devInfo_t     devInfo;

    DIAG_OM_GET_CHIP_ID(unit);

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    memset(&fidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *index_ptr, &fidMacLimitEntry), ret);

    diag_util_mprintf("Show Vlan Limit Learning information :\n");
    diag_util_mprintf("    vlan limit learning entry index : %u\n", *index_ptr);

     if(fidMacLimitEntry.enable == ENABLED)
     {
        diag_util_mprintf("        Status                  : ENABLE\n");
        diag_util_mprintf("        Fid                     : %u\n", fidMacLimitEntry.fid);
        diag_util_mprintf("        Max Mac Count           : %u\n", fidMacLimitEntry.maxNum);

        if ((ret = rtk_l2_fidLearningCnt_get(unit, *index_ptr, &mac_cnt)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("        Current Mac Count       : %u\n", mac_cnt);

        diag_util_lPortMask2str(portStr, &fidMacLimitEntry.portmask);

        diag_util_mprintf("        Including Ports         : %s\n", portStr);

        diag_util_mprintf("        Exceed Max Count Action : ");

        DIAG_UTIL_ERR_CHK(rtk_l2_fidLastLearnedMac_get(unit, *index_ptr, &fid, &mac), ret);
        DIAG_UTIL_ERR_CHK(diag_util_mac2str(macStr, mac.octet), ret);
        diag_util_mprintf("        Last Packet Info        : Fid(%u), Src Mac Address(%s)\n", fid, macStr);

     }
     else
     {
        diag_util_mprintf("        Status : DISABLE\n");
     }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_FID_INDEX_INDEX_FID
/*
  *  l2-table get limit-learning fid-index <UINT:index> fid
  */

cparser_result_t cparser_cmd_l2_table_get_limit_learning_fid_index_index_fid(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_fidMacLimitEntry_t       fidMacLimitEntry;
    //uint32                          mac_cnt;
    //rtk_fid_t                       fid;
    //rtk_mac_t                       mac;
    //uint8                           portStr[20];
    //uint8                           macStr[MAX_MAC_ADDR_STR_LEN];
    rtk_switch_devInfo_t            devInfo;

    DIAG_OM_GET_CHIP_ID(unit);

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    memset(&fidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *index_ptr, &fidMacLimitEntry), ret);

    diag_util_mprintf("        Fid                     : %u\n", fidMacLimitEntry.fid);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_FID_INDEX_INDEX_ACTION
/*
  *  l2-table get limit-learning fid-index <UINT:index> action
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_fid_index_index_action(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l2_fidMacLimitEntry_t   fidMacLimitEntry;
    rtk_switch_devInfo_t     devInfo;

    DIAG_OM_GET_CHIP_ID(unit);

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    memset(&fidMacLimitEntry, 0, sizeof(rtk_l2_fidMacLimitEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *index_ptr, &fidMacLimitEntry), ret);


    diag_util_mprintf("        Exceed Max Count Action : ");

    if (fidMacLimitEntry.action == LIMIT_LEARN_CNT_ACTION_FORWARD)
    {
        diag_util_mprintf("Forward\n");
    }
    else if (fidMacLimitEntry.action == LIMIT_LEARN_CNT_ACTION_DROP)
    {
        diag_util_mprintf("Drop\n");
    }
    else if (fidMacLimitEntry.action == LIMIT_LEARN_CNT_ACTION_TO_CPU)
    {
        diag_util_mprintf("Trap-To-Cpu\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_NOTIFICATION
/*
  *  l2-table get notification
  */
cparser_result_t cparser_cmd_l2_table_get_notification(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_notificationEnable_get(unit, &enable), ret);
    diag_util_mprintf("L2 Notification Status : %s\n", enable ? "Enable" : "Disable");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_NOTIFICATION_BACK_PRESSURE_THRESHOLD
/*
  *  l2-table get notification back-pressure threshold
  */
cparser_result_t cparser_cmd_l2_table_get_notification_back_pressure_threshold(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    uint32                          value;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_notificationBackPressureThresh_get(unit, &value), ret);
    diag_util_mprintf("L2 Notification Back-Pressure Threshold : %d\n", value);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_NOTIFICATION_TYPE_LINK_DOWN_FLUSH_SUSPEND_STATE
/*
 * l2-table get notification type ( link-down-flush | suspend ) state
 */
cparser_result_t
cparser_cmd_l2_table_get_notification_type_link_down_flush_suspend_state(
    cparser_context_t *context)
{
    uint32  unit;
    int32   ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if('l' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_notificationEventEnable_get(unit, L2_NOTIFY_EVENT_LINKDOWNFLUSH, &enable), ret);
	    diag_util_mprintf("L2 Suspend Learn Notification Status : %s\n", enable ? "Enable" : "Disable");
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_notificationEventEnable_get(unit, L2_NOTIFY_EVENT_SUSPEND, &enable), ret);
		diag_util_mprintf("Link-down-flush Notification Status : %s\n", enable ? "Enable" : "Disable");
    }
	
    

    return CPARSER_OK;
}   /* end of cparser_cmd_l2_table_get_notification_type_suspend_state */
#endif
#ifdef CMD_L2_TABLE_GET_PORT_MOVE_PORT_ALL
/*
  *  l2-table get port-move ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_get_port_move_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32         port;
    rtk_portmask_t  dstPortmask;
    rtk_action_t      portMove_action = ACTION_DROP;
    uint8                port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    diag_portlist_t   portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    /*get target port mask*/
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("Port Move Information:\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("    Port %2d: \n", port);
        DIAG_UTIL_ERR_CHK(rtk_l2_legalMoveToPorts_get(unit, port, &dstPortmask), ret);
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &dstPortmask);
        diag_util_mprintf("        Legal Move To Ports : %s\n", port_list);

        diag_util_mprintf("        Legal Move Action   : ");
        DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveAction_get(unit, port, &portMove_action), ret);
        if (ACTION_FORWARD == portMove_action)
        {
            diag_util_mprintf("Forward");
        }
        else if (ACTION_DROP == portMove_action)
        {
            diag_util_mprintf("Drop");
        }
        else if (ACTION_TRAP2CPU== portMove_action)
        {
            diag_util_mprintf("Trap-To-Cpu");
        }
        else if (ACTION_COPY2CPU== portMove_action)
        {
            diag_util_mprintf("Copy-To-Cpu");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("\n        Illegal Move Action : ");
        DIAG_UTIL_ERR_CHK(rtk_l2_illegalPortMoveAction_get(unit, port, &portMove_action), ret);
        if (ACTION_FORWARD == portMove_action)
        {
            diag_util_mprintf("Forward");
        }
        else if (ACTION_DROP == portMove_action)
        {
            diag_util_mprintf("Drop");
        }
        else if (ACTION_TRAP2CPU== portMove_action)
        {
            diag_util_mprintf("Trap-To-Cpu");
        }
        else if (ACTION_COPY2CPU== portMove_action)
        {
            diag_util_mprintf("Copy-To-Cpu");
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_PORT_MOVE_ILLEGAL_PORT_ALL_ACTION
/*
  *  l2-table get port-move illegal ( <PORT_LIST:port> | all ) action
  */
cparser_result_t cparser_cmd_l2_table_get_port_move_illegal_port_all_action(cparser_context_t *context,
    char **port_ptr)
{
    uint32            unit = 0;
    int32             ret = RT_ERR_FAILED;
    uint32            port;
    rtk_action_t      portMove_action = ACTION_DROP;
    diag_portlist_t   portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    /*get target port mask*/
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Move Information:\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("    Port %2d: \n", port);
        diag_util_printf("        Illegal Move Action : ");
        DIAG_UTIL_ERR_CHK(rtk_l2_illegalPortMoveAction_get(unit, port, &portMove_action), ret);
        if (ACTION_FORWARD == portMove_action)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (ACTION_DROP == portMove_action)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (ACTION_TRAP2CPU== portMove_action)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (ACTION_COPY2CPU== portMove_action)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
        else
        {
            diag_util_mprintf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_PORT_MOVE_LEGAL_PORT_ALL_ACTION
/*
  *  l2-table get port-move legal ( <PORT_LIST:port> | all ) action
  */
cparser_result_t cparser_cmd_l2_table_get_port_move_legal_port_all_action(cparser_context_t *context,
    char **port_ptr)
{
    uint32            unit = 0;
    int32             ret = RT_ERR_FAILED;
    uint32            port;
    rtk_action_t      portMove_action = ACTION_DROP;
    diag_portlist_t   portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    /*get target port mask*/
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Move Information:\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("    Port %2d: \n", port);
        diag_util_printf("        Legal Move Action   : ");
        DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveAction_get(unit, port, &portMove_action), ret);
        if (ACTION_FORWARD == portMove_action)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (ACTION_DROP == portMove_action)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (ACTION_TRAP2CPU== portMove_action)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (ACTION_COPY2CPU== portMove_action)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
        else
        {
            diag_util_mprintf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LEGAL_MOVE_SRC_PORT_ALL
/*
  *  l2-table get legal-move ( <PORT_LIST:src_port> | all )
  */
cparser_result_t cparser_cmd_l2_table_get_legal_move_src_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32           unit = 0;
    int32            ret = RT_ERR_FAILED;
    uint32           port;
    rtk_portmask_t   dstPortmask;
    uint8            port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    diag_portlist_t  portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    /*get target port mask*/
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("Port Move Information:\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("    Port%u: \n", port);
        DIAG_UTIL_ERR_CHK(rtk_l2_legalMoveToPorts_get(unit, port, &dstPortmask), ret);
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &dstPortmask);
        diag_util_mprintf("        Legal Move To Ports : %s\n", port_list);

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LOOKUP_MISS
/*
  *  l2-table get lookup-miss
  */
cparser_result_t cparser_cmd_l2_table_get_lookup_miss(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          val = 0;
    rtk_portmask_t    portmask;
    uint8                   port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_pri_t       priority;
    uint32          dp;
    rtk_enable_t  enable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Address Table Lookup Miss Configuration: \n");
    ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_IPMC, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Ipmc Lookup Miss Action          : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (val == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (val == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
        else if (val == ACTION_FLOOD_IN_ROUTER_PORTS)
        {
            diag_util_mprintf("Flood-In-Router-Ports\n");
        }
    }

    ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_UCAST, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    L2 Unicast Lookup Miss Action    : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (val == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (val == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
        else if (val == ACTION_FLOOD_IN_ALL_PORT)
        {
            diag_util_mprintf("Flood-In-All-Ports\n");
        }
    }

    ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_BCAST, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Broadcast Lookup Miss Action     : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (val == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (val == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
    }

    ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_MCAST, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    L2 Multicast Lookup Miss Action  : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (val == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (val == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
        else if (val == ACTION_FLOOD_IN_ALL_PORT)
        {
            diag_util_mprintf("Flood-In-All-Ports\n");
        }
    }

    memset(&portmask, 0, sizeof(rtk_portmask_t));
    ret = rtk_l2_lookupMissFloodPortMask_get(unit, DLF_TYPE_UCAST, &portmask);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
    {
        /* Don't display RT_ERR_CHIP_NOT_SUPPORTED item */
        //diag_util_mprintf("    Lookup Miss Flood Port           : %s\n", "No supported");
    }
    else
    {
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &portmask);
        diag_util_mprintf("    Lookup Miss Flood Port           : %s\n", port_list);
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPriEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPri_get(unit, &priority), ret);
        diag_util_mprintf("    Lookup Miss Trap Priority        : Enable(%u)\n", priority);
    }
    else
    {
        diag_util_mprintf("    Lookup Miss Trap Priority        : Disable\n");
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDPEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDP_get(unit, &dp), ret);
        diag_util_mprintf("    Lookup Miss Trap Drop Precedence : Enable(%u)\n", dp);
    }
    else
    {
        diag_util_mprintf("    Lookup Miss Trap Drop Precedence : Disable\n");
    }

    ret = rtk_l2_lookupMissAddCPUTagEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Lookup Miss Trap Insert Cpu Tag  : %s\n", enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_EXCEPT_SMAC
/*
  *  l2-table get except-smac
  */
cparser_result_t cparser_cmd_l2_table_get_except_smac(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          val = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Exception Src Mac Configuration:\n");

    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID) || 
       DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_BCAST_OR_MCAST, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("    Broadcast & Multicast Src Mac Action : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("trap\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }
    }
    else
    {
        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_MCAST, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("    Multicast Src Mac Action : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("trap\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }

        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_BCAST, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("    Broadcast Src Mac Action : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("trap\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }
    }

    if (!((DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID)) && 
         (DIAG_OM_GET_TESTCHIPID(RTL8380MES_CHIP_ID))))
    {
        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_ZERO, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("    Zero Src Mac Action      : ");
            if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            {
                diag_util_mprintf("Not Support\n");
            }
            else if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("trap\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_EXCEPT_SMAC_MCAST_SA
/*
  *  l2-table get except-smac mcast-sa
  */
cparser_result_t cparser_cmd_l2_table_get_except_smac_mcast_sa(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          val = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_MCAST, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Multicast Src Mac Action : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else
        {
            diag_util_mprintf("Forward\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_EXCEPT_SMAC_BCAST_SA
/*
  *  l2-table get except-smac bcast-sa
  */
cparser_result_t cparser_cmd_l2_table_get_except_smac_bcast_sa(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          val = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_BCAST, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Broadcast Src Mac Action : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else
        {
            diag_util_mprintf("Forward\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_EXCEPT_SMAC_ZERO_SA
/*
  *  l2-table get except-smac zero-sa
  */

cparser_result_t cparser_cmd_l2_table_get_except_smac_zero_sa(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          val = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_ZERO, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Zero Src Mac Action      : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else
        {
            diag_util_mprintf("Forward\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_AGING_TIME
/*
  *  l2-table get aging-time
  */
cparser_result_t cparser_cmd_l2_table_get_aging_time(cparser_context_t *context)
{
    uint32   unit = 0;
    uint32   aging_time = 0;
    int32    ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("\tAging Time                                     : ");
    DIAG_UTIL_ERR_CHK(rtk_l2_aging_get(unit, &aging_time), ret);
    diag_util_mprintf("%d seconds.\n", aging_time);


    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_CAM_L2_TABLE_LRU_IPMC_STATE
/*
  *  l2-table get ( cam | l2-table-lru | ipmc ) state
  */
cparser_result_t cparser_cmd_l2_table_get_cam_l2_table_lru_ipmc_state(cparser_context_t *context)
{

    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if ('l' == TOKEN_CHAR(2, 0))
    {
        ret = rtk_l2_LRUEnable_get(unit, &enable);
        diag_util_mprintf("\tL2 Table LRU                                   : %s\n", enable ? "ENABLE" : "DISABLE");
    }
    else if ('c' == TOKEN_CHAR(2, 0))
    {
        ret = rtk_l2_camEnable_get(unit, &enable);
        diag_util_mprintf("\tL2 CAM                                         : %s\n", enable ? "ENABLE" : "DISABLE");
    }
    else if ('i' == TOKEN_CHAR(2, 0))
    {
        ret = rtk_l2_ipmcEnable_get(unit, &enable);
        diag_util_mprintf("\tIP Multicast Asic Lookup                       : %s\n", enable ? "ENABLE" : "DISABLE");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_FWD_TABLE_LOW_IDX_HIGH_IDX
/*
  * l2-table get fwd-table <UINT:low_idx> <UINT:high_idx>
  */
cparser_result_t cparser_cmd_l2_table_get_fwd_table_low_idx_high_idx(cparser_context_t *context,
    uint32_t *low_idx_ptr,
    uint32_t *high_idx_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      minIndex;
    uint32      maxIndex;
    int32       index;
    rtk_portmask_t portmask;
    uint8       portStr[80];
    uint32      crossVlan;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == low_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == high_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*low_idx_ptr > FWD_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*high_idx_ptr > FWD_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*low_idx_ptr > *high_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    minIndex = *low_idx_ptr;
    maxIndex = *high_idx_ptr;

    for(index = minIndex; index <= maxIndex; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_mcastFwdPortmask_get(unit, index, &portmask, &crossVlan), ret);

        diag_util_mprintf("Index : %4u, ", index);

        diag_util_lPortMask2str(portStr, &portmask);

        diag_util_mprintf("Port List : %10s  ", portStr);

        if(!(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID)))
        {
            if(crossVlan)
                diag_util_mprintf("Cross Vlan : TRUE\n");
            else
                diag_util_mprintf("Cross Vlan : FALSE\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_ADDR_ENTRY_ANY_MAC_UCAST_MAC_MCAST_IP_MCAST_LOW_IDX_HIGH_IDX_FILTER_INVALID
/*
 * l2-table get addr-entry ( any | mac-ucast | mac-mcast | ip-mcast ) <UINT:low_idx> <UINT:high_idx> { filter-invalid }
 */
cparser_result_t cparser_cmd_l2_table_get_addr_entry_any_mac_ucast_mac_mcast_ip_mcast_low_idx_high_idx_filter_invalid(cparser_context_t *context,
    uint32_t *low_idx_ptr,
    uint32_t *high_idx_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      minIndex, maxIndex;
    uint32      total_entry = 0;
    uint32      is_filter_invalid;
    int32       index;
    rtk_l2_entry_t  l2_entry;
    uint8       port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8       strBuf1[20], strBuf2[20];


    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == low_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == high_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*low_idx_ptr > L2_TABLE_AND_L2_CAM_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*high_idx_ptr > L2_TABLE_AND_L2_CAM_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*low_idx_ptr > *high_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    minIndex = *low_idx_ptr;
    maxIndex = *high_idx_ptr;

    if (TOKEN_NUM == 7)
        is_filter_invalid = TRUE;
    else
        is_filter_invalid = FALSE;

    if ('m' == TOKEN_CHAR(3, 0) && 'u' == TOKEN_CHAR(3, 4))
    {
        diag_util_mprintf("Index|MAC Address      |VID |SPA |SaBlock |DaBlock |Auth |Static |Nexthop |Suspend\n");
        diag_util_mprintf("-----+-----------------+----+----+--------+--------+-----+-------+--------+-------\n");

        for(index = minIndex; index <= maxIndex; index++)
        {
            osal_memset(&l2_entry, 0, sizeof(rtk_l2_entry_t));
            DIAG_UTIL_ERR_CHK(rtk_l2_addrEntry_get(unit, index, &l2_entry), ret);
            if (!(l2_entry.valid))
            {
                if (is_filter_invalid)
                    continue;
                diag_util_mprintf("%5d| Entry is invalid\n", index);
            }
            else if (l2_entry.entry_type != FLOW_TYPE_UNICAST)
                continue;
            else
            {
                /* show unicast address */
                if (l2_entry.unicast.flags & RTK_L2_UCAST_FLAG_TRUNK_PORT)
                {
                    diag_util_mprintf("%5d|%02X:%02X:%02X:%02X:%02X:%02X|%4d|%4d(T%d)|%8d|%8d|%5d|%7d|%8d|%6d\n",
                            index,
                            l2_entry.unicast.mac.octet[0],l2_entry.unicast.mac.octet[1],l2_entry.unicast.mac.octet[2],l2_entry.unicast.mac.octet[3],l2_entry.unicast.mac.octet[4],l2_entry.unicast.mac.octet[5],
                            l2_entry.unicast.vid, l2_entry.unicast.port, l2_entry.unicast.trk_gid, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                            l2_entry.unicast.auth, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                            (l2_entry.unicast.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
                }
                else
                {
                    diag_util_mprintf("%5d|%02X:%02X:%02X:%02X:%02X:%02X|%4d|%4d|%8d|%8d|%5d|%7d|%8d|%6d\n",
                        index,
                        l2_entry.unicast.mac.octet[0],l2_entry.unicast.mac.octet[1],l2_entry.unicast.mac.octet[2],l2_entry.unicast.mac.octet[3],l2_entry.unicast.mac.octet[4],l2_entry.unicast.mac.octet[5],
                        l2_entry.unicast.vid, l2_entry.unicast.port, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                        l2_entry.unicast.auth, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                        (l2_entry.unicast.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
                }
                total_entry++;
            }
        }
        diag_util_mprintf("-----+-----------------+----+----+--------+--------+-----+-------+--------+-------\n");
        diag_util_mprintf("\nTotal Number Of Valid Entries :%d\n", total_entry);
    }
    else if ('m' == TOKEN_CHAR(3, 0) && 'm' == TOKEN_CHAR(3, 4))
    {
        diag_util_mprintf("Index |VID |MAC address      |Port      |Cross Vlan\n");
        diag_util_mprintf("------+----+-----------------+----------+----------\n");
        for(index = minIndex; index <= maxIndex; index++)
        {
            osal_memset(&l2_entry, 0, sizeof(rtk_l2_entry_t));
            DIAG_UTIL_ERR_CHK(rtk_l2_addrEntry_get(unit, index, &l2_entry), ret);
            if (!(l2_entry.valid))
            {
                if (is_filter_invalid)
                    continue;
                diag_util_mprintf("%5d| Entry is invalid\n", index);
            }
            else if (l2_entry.entry_type != FLOW_TYPE_L2_MULTI)
                continue;
            else
            {
                /* show l2 multicast address */
                memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                diag_util_lPortMask2str(port_list, &l2_entry.l2mcast.portmask);
                diag_util_mprintf("%6d|%4d|%02X:%02X:%02X:%02X:%02X:%02X|%s       |%s\n", index, l2_entry.l2mcast.rvid,
                        l2_entry.l2mcast.mac.octet[0], l2_entry.l2mcast.mac.octet[1], l2_entry.l2mcast.mac.octet[2],
                        l2_entry.l2mcast.mac.octet[3], l2_entry.l2mcast.mac.octet[4], l2_entry.l2mcast.mac.octet[5],
                        port_list, (l2_entry.l2mcast.crossVlan)?"TRUE":"FALSE");
                total_entry++;
            }
        }
        diag_util_mprintf("------+----+-----------------+----------+----------\n");
        diag_util_mprintf("\nTotal Number Of Entries : %d\n",total_entry);
    }
    else if ('i' == TOKEN_CHAR(3, 0))
    {
        rtk_switch_devInfo_t devInfo;

        if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (devInfo.chipId == RTL8389M_CHIP_ID || devInfo.chipId == RTL8389L_CHIP_ID ||
            devInfo.chipId == RTL8329M_CHIP_ID || devInfo.chipId == RTL8377M_CHIP_ID)
        {
            diag_util_mprintf("Index |SIP            |DIP            |VID |Cross Vlan  |Port\n");
            diag_util_mprintf("------+---------------+---------------+----+------------+------------\n");
            for(index = minIndex; index <= maxIndex; index++)
            {
                osal_memset(&l2_entry, 0, sizeof(rtk_l2_entry_t));
                DIAG_UTIL_ERR_CHK(rtk_l2_addrEntry_get(unit, index, &l2_entry), ret);
                if (!(l2_entry.valid))
                {
                    if (is_filter_invalid)
                        continue;
                    diag_util_mprintf("%5d| Entry is invalid\n", index);
                }
                else if (l2_entry.entry_type != FLOW_TYPE_IP4_MULTI)
                    continue;
                else
                {
                    /* show ip multicast address */
                    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                    diag_util_lPortMask2str(port_list, &l2_entry.ipmcast.portmask);
                    diag_util_ip2str_format(strBuf1, l2_entry.ipmcast.sip, 15);
                    diag_util_ip2str_format(strBuf2, l2_entry.ipmcast.dip, 15);
                    diag_util_mprintf("%6d|%s|%s|%4d|%5s        |%s\n", index,
                        strBuf1, strBuf2, l2_entry.ipmcast.rvid, (l2_entry.ipmcast.crossVlan)?"TRUE":"FALSE", port_list);
                    total_entry++;
                }
            }
            diag_util_mprintf("------+---------------+---------------+----+------------+------------\n");
            diag_util_mprintf("\nTotal Number Of Entries : %d\n",total_entry);
        }
        else
        {
            diag_util_mprintf("Index |SIP            |DIP            |Cross Vlan   |Port\n");
            diag_util_mprintf("------+---------------+---------------+-------------+---------------\n");
            for(index = minIndex; index <= maxIndex; index++)
            {
                osal_memset(&l2_entry, 0, sizeof(rtk_l2_entry_t));
                DIAG_UTIL_ERR_CHK(rtk_l2_addrEntry_get(unit, index, &l2_entry), ret);
                if (!(l2_entry.valid))
                {
                    if (is_filter_invalid)
                        continue;
                    diag_util_mprintf("%5d| Entry is invalid\n", index);
                }
                else if (l2_entry.entry_type != FLOW_TYPE_IP4_MULTI)
                    continue;
                else
                {
                    /* show ip multicast address */
                    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                    diag_util_lPortMask2str(port_list, &l2_entry.ipmcast.portmask);
                    diag_util_ip2str_format(strBuf1, l2_entry.ipmcast.sip, 15);
                    diag_util_ip2str_format(strBuf2, l2_entry.ipmcast.dip, 15);
                    diag_util_mprintf("%6d|%s|%s|%5s        |%s\n", index,
                        strBuf1, strBuf2, (l2_entry.ipmcast.crossVlan)?"TRUE":"FALSE", port_list);
                    total_entry++;
                }
            }
            diag_util_mprintf("------+---------------+---------------+------------+------------\n");
            diag_util_mprintf("\nTotal Number Of Entries : %d\n",total_entry);
        }
    }
    else /* if ('a' == TOKEN_CHAR(3, 0)) */
    {
        rtk_switch_devInfo_t devInfo;

        diag_util_mprintf("Index|MAC Address      |VID |SPA |SaBlock |DaBlock |Auth |Static |Nexthop |Suspend\n");
        diag_util_mprintf("Index |VID |MAC address      |Port      |Cross Vlan\n");
        if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (devInfo.chipId == RTL8389M_CHIP_ID || devInfo.chipId == RTL8389L_CHIP_ID ||
            devInfo.chipId == RTL8329M_CHIP_ID || devInfo.chipId == RTL8377M_CHIP_ID)
        {
            diag_util_mprintf("Index |SIP            |DIP            |VID |Cross Vlan  |Port\n");
        }
        else
        {
            diag_util_mprintf("Index |SIP            |DIP            |Cross Vlan  |Port\n");
        }
        diag_util_mprintf("----------------------------------------------------------------------------------\n");

        for(index = minIndex; index <= maxIndex; index++)
        {
            osal_memset(&l2_entry, 0, sizeof(rtk_l2_entry_t));
            DIAG_UTIL_ERR_CHK(rtk_l2_addrEntry_get(unit, index, &l2_entry), ret);
            if (!(l2_entry.valid))
            {
                if (is_filter_invalid)
                    continue;
                diag_util_mprintf("%5d| Entry is invalid\n", index);
            }
            else
            {
                if (l2_entry.entry_type == FLOW_TYPE_UNICAST)
                {
                    /* show unicast address */
                    diag_util_mprintf("%5d|%02X:%02X:%02X:%02X:%02X:%02X|%4d|%4d|%8d|%8d|%5d|%7d|%8d|%6d\n",
                        index,
                        l2_entry.unicast.mac.octet[0],l2_entry.unicast.mac.octet[1],l2_entry.unicast.mac.octet[2],l2_entry.unicast.mac.octet[3],l2_entry.unicast.mac.octet[4],l2_entry.unicast.mac.octet[5],
                        l2_entry.unicast.vid, l2_entry.unicast.port, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                        l2_entry.unicast.auth, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                        (l2_entry.unicast.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
                    total_entry++;
                }
                else if (l2_entry.entry_type == FLOW_TYPE_L2_MULTI)
                {
                    /* show l2 multicast address */
                    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                    diag_util_lPortMask2str(port_list, &l2_entry.l2mcast.portmask);
                    diag_util_mprintf("%6d|%4d|%02X:%02X:%02X:%02X:%02X:%02X|%s       |%s\n", index, l2_entry.l2mcast.rvid,
                            l2_entry.l2mcast.mac.octet[0], l2_entry.l2mcast.mac.octet[1], l2_entry.l2mcast.mac.octet[2],
                            l2_entry.l2mcast.mac.octet[3], l2_entry.l2mcast.mac.octet[4], l2_entry.l2mcast.mac.octet[5],
                            port_list, (l2_entry.l2mcast.crossVlan)?"TRUE":"FALSE");
                    total_entry++;
                }
                else if (l2_entry.entry_type == FLOW_TYPE_IP4_MULTI)
                {
                    if (devInfo.chipId == RTL8389M_CHIP_ID || devInfo.chipId == RTL8389L_CHIP_ID ||
                        devInfo.chipId == RTL8329M_CHIP_ID || devInfo.chipId == RTL8377M_CHIP_ID)
                    {
                        /* show ip multicast address */
                        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                        diag_util_lPortMask2str(port_list, &l2_entry.ipmcast.portmask);
                        diag_util_ip2str_format(strBuf1, l2_entry.ipmcast.sip, 15);
                        diag_util_ip2str_format(strBuf2, l2_entry.ipmcast.dip, 15);
                        diag_util_mprintf("%6d|%s|%s|%4d|%5s        |%s\n", index,
                            strBuf1, strBuf2, l2_entry.ipmcast.rvid, (l2_entry.ipmcast.crossVlan)?"TRUE":"FALSE", port_list);
                        total_entry++;
                    }
                    else
                    {
                        /* show ip multicast address */
                        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                        diag_util_lPortMask2str(port_list, &l2_entry.ipmcast.portmask);
                        diag_util_ip2str_format(strBuf1, l2_entry.ipmcast.sip, 15);
                        diag_util_ip2str_format(strBuf2, l2_entry.ipmcast.dip, 15);
                        diag_util_mprintf("%6d|%s|%s|%5s        |%s\n", index, strBuf1, strBuf2, (l2_entry.ipmcast.crossVlan)?"TRUE":"FALSE", port_list);
                        total_entry++;
                    }

                }
                else
                {
                    /* Should not in the condition */
                    continue;
                }
            }
        }
        diag_util_mprintf("----------------------------------------------------------------------------------\n");
        diag_util_mprintf("\nTotal Number Of Valid Entries :%d\n", total_entry);

    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_CONFLICT_ENTRY_MAC_UCAST_VID_MAC
/*
 * l2-table get conflict-entry mac-ucast <UINT:vid> <MACADDR:mac>
 */
cparser_result_t cparser_cmd_l2_table_get_conflict_entry_mac_ucast_vid_mac(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32  unit = 0, ret_cnt = 0, i, total_entry = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_l2_entry_t  input_l2_entry;
    rtk_l2_entry_t  output_l2_entry[4];
    rtk_switch_devInfo_t devInfo;
    uint8       port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8       strBuf1[20], strBuf2[20];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&input_l2_entry, 0, sizeof(input_l2_entry));
    osal_memset(&output_l2_entry, 0, sizeof(output_l2_entry));
    input_l2_entry.entry_type = FLOW_TYPE_UNICAST;
    input_l2_entry.unicast.vid = *vid_ptr;
    memcpy(input_l2_entry.unicast.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

    DIAG_UTIL_ERR_CHK(rtk_l2_conflictAddr_get(unit, &input_l2_entry, &output_l2_entry[0], 4, &ret_cnt), ret);

    if (ret_cnt)
    {
        diag_util_mprintf("Index|MAC Address      |VID |SPA |SaBlock |DaBlock |Auth |Static |Nexthop |Suspend\n");
        diag_util_mprintf("Index |VID |MAC address      |Port      |Cross Vlan\n");
        if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if (devInfo.chipId == RTL8389M_CHIP_ID || devInfo.chipId == RTL8389L_CHIP_ID ||
            devInfo.chipId == RTL8329M_CHIP_ID || devInfo.chipId == RTL8377M_CHIP_ID)
        {
            diag_util_mprintf("Index |SIP            |DIP            |VID |Cross Vlan  |Port\n");
        }
        else
        {
            diag_util_mprintf("Index |SIP            |DIP            |Cross Vlan  |Port\n");
        }
        diag_util_mprintf("----------------------------------------------------------------------------------\n");
    }

    for (i = 0; i < ret_cnt; i++)
    {
        if (output_l2_entry[i].entry_type == FLOW_TYPE_UNICAST)
        {
            /* show unicast address */
            diag_util_mprintf("     |%02X:%02X:%02X:%02X:%02X:%02X|%4d|%4d|%8d|%8d|%5d|%7d|%8d|%6d\n",
                output_l2_entry[i].unicast.mac.octet[0],output_l2_entry[i].unicast.mac.octet[1],output_l2_entry[i].unicast.mac.octet[2],output_l2_entry[i].unicast.mac.octet[3],output_l2_entry[i].unicast.mac.octet[4],output_l2_entry[i].unicast.mac.octet[5],
                output_l2_entry[i].unicast.vid, output_l2_entry[i].unicast.port, (output_l2_entry[i].unicast.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (output_l2_entry[i].unicast.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                output_l2_entry[i].unicast.auth, (output_l2_entry[i].unicast.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (output_l2_entry[i].unicast.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                (output_l2_entry[i].unicast.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
            total_entry++;
        }
        else if (output_l2_entry[i].entry_type == FLOW_TYPE_L2_MULTI)
        {
            /* show l2 multicast address */
            memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
            diag_util_lPortMask2str(port_list, &output_l2_entry[i].l2mcast.portmask);
            diag_util_mprintf("      |%4d|%02X:%02X:%02X:%02X:%02X:%02X|%s       |%s\n", output_l2_entry[i].l2mcast.rvid,
                    output_l2_entry[i].l2mcast.mac.octet[0], output_l2_entry[i].l2mcast.mac.octet[1], output_l2_entry[i].l2mcast.mac.octet[2],
                    output_l2_entry[i].l2mcast.mac.octet[3], output_l2_entry[i].l2mcast.mac.octet[4], output_l2_entry[i].l2mcast.mac.octet[5],
                    port_list, (output_l2_entry[i].l2mcast.crossVlan)?"TRUE":"FALSE");
            total_entry++;
        }
        else if (output_l2_entry[i].entry_type == FLOW_TYPE_IP4_MULTI)
        {
            if (devInfo.chipId == RTL8389M_CHIP_ID || devInfo.chipId == RTL8389L_CHIP_ID ||
                devInfo.chipId == RTL8329M_CHIP_ID || devInfo.chipId == RTL8377M_CHIP_ID)
            {
                /* show ip multicast address */
                memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                diag_util_lPortMask2str(port_list, &output_l2_entry[i].ipmcast.portmask);
                diag_util_ip2str_format(strBuf1, output_l2_entry[i].ipmcast.sip, 15);
                diag_util_ip2str_format(strBuf2, output_l2_entry[i].ipmcast.dip, 15);
                diag_util_mprintf("      |%s|%s|%4d|%5s        |%s\n",
                    strBuf1, strBuf2, output_l2_entry[i].ipmcast.rvid, (output_l2_entry[i].ipmcast.crossVlan)?"TRUE":"FALSE", port_list);
                total_entry++;
            }
            else
            {
                /* show ip multicast address */
                memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
                diag_util_lPortMask2str(port_list, &output_l2_entry[i].ipmcast.portmask);
                diag_util_ip2str_format(strBuf1, output_l2_entry[i].ipmcast.sip, 15);
                diag_util_ip2str_format(strBuf2, output_l2_entry[i].ipmcast.dip, 15);
                diag_util_mprintf("      |%s|%s|%5s        |%s\n", strBuf1, strBuf2, (output_l2_entry[i].ipmcast.crossVlan)?"TRUE":"FALSE", port_list);
                total_entry++;
            }

        }
        else
        {
            /* Should not in the condition */
            continue;
        }
    }

    if (ret_cnt)
    {
        diag_util_mprintf("----------------------------------------------------------------------------------\n");
        diag_util_mprintf("\nTotal Number Of Valid Entries :%d\n", total_entry);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_IP_MCAST_SIP_DIP_VID
/*
  * l2-table dump ip-mcast <IPV4ADDR:sip> <IPV4ADDR:dip> <UINT:vid>
  */
cparser_result_t cparser_cmd_l2_table_dump_ip_mcast_sip_dip_vid(cparser_context_t *context,
    uint32_t *sip_ptr,
    uint32_t *dip_ptr,
    uint32_t *vid_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_l2_ipMcastAddr_t    ip_mcast_data;
    uint8                   port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8                   strBuf1[20], strBuf2[20];
    rtk_switch_devInfo_t devInfo;
    uint32                  enable;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    memset(&ip_mcast_data, 0, sizeof(rtk_l2_ipMcastAddr_t));

    if (devInfo.chipId == RTL8328M_CHIP_ID || devInfo.chipId == RTL8328S_CHIP_ID ||
        devInfo.chipId == RTL8328L_CHIP_ID)
    {
        diag_util_mprintf("SIP             | DIP             | Port     | Cross Vlan\n");
        diag_util_mprintf("----------------+-----------------+----------+-----------\n");
    }
    else
    {
        diag_util_mprintf("SIP             | DIP             | VID  | Port   \n");
        diag_util_mprintf("----------------+-----------------+------+----------\n");
    }

    /* show specific ip-ipmcast entry */
    ip_mcast_data.dip = *dip_ptr;
    ip_mcast_data.sip = *sip_ptr;
    ip_mcast_data.rvid = *vid_ptr;

    rtk_l2_ipMcstFidVidCompareEnable_get(unit, &enable);
    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcstFidVidCompareEnable_set(unit, ENABLED), ret);
    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddr_get(unit, &ip_mcast_data), ret);
    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcstFidVidCompareEnable_set(unit, enable), ret);     /* Restore original setting */
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &ip_mcast_data.portmask);
    if (devInfo.chipId == RTL8328M_CHIP_ID || devInfo.chipId == RTL8328S_CHIP_ID ||
        devInfo.chipId == RTL8328L_CHIP_ID)
    {
        diag_util_ip2str_format(strBuf1, ip_mcast_data.sip, 15);
        diag_util_ip2str_format(strBuf2, ip_mcast_data.dip, 15);
        diag_util_mprintf("%s | %s | %s   ", strBuf1, strBuf2, (ip_mcast_data.dip & 0xff), port_list);
    }
    else
    {
        diag_util_ip2str_format(strBuf1, ip_mcast_data.sip, 15);
        diag_util_ip2str_format(strBuf2, ip_mcast_data.dip, 15);
        diag_util_mprintf("%s | %s | %4d | %s   ", strBuf1, strBuf2, ip_mcast_data.rvid, port_list);
    }

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
#if defined(CONFIG_SDK_RTL8328)
        if(ip_mcast_data.crossVlan)
            diag_util_mprintf(" | TRUE\n");
        else
            diag_util_mprintf(" | FALSE\n");
#endif
    }
    else
        diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_HASH_ALGORITHM
/*
  *  l2-table get hash-algorithm
  */
cparser_result_t cparser_cmd_l2_table_get_hash_algorithm (cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    uint32                           val = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_hashAlgo_get(unit, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tL2 Table Hash Algorithm                        : ");
        if (val == 0)
        {
            diag_util_mprintf("ALGO-0\n");
        }
        else
        {
            diag_util_mprintf("ALGO-1\n");
        }
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IP6_DIP_CARE_BYTE
/*
  * l2-table get ip6-dip-care-byte
  */
cparser_result_t cparser_cmd_l2_table_get_ip6_dip_care_byte(cparser_context_t *context)
{
    int32   ret = RT_ERR_FAILED;
    uint32  unit = 0;
    uint32  val;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_l2_ip6CareByte_get(unit, L2_DIP_HASH_CARE_BYTE, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tL2 Hash DIP Care-Byte                        : %#x", val);
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IP6_SIP_CARE_BYTE
/*
  * l2-table get ip6-sip-care-byte
  */
cparser_result_t cparser_cmd_l2_table_get_ip6_sip_care_byte(cparser_context_t *context)
{
    int32   ret = RT_ERR_FAILED;
    uint32  unit = 0;
    uint32  val;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_l2_ip6CareByte_get(unit, L2_SIP_HASH_CARE_BYTE, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tL2 Hash SIP Care-Byte                        : %#x", val);
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IP6MC_MODE
/*
 *   l2-table get ip6mc-mode
 */
cparser_result_t cparser_cmd_l2_table_get_ip6mc_mode(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    uint32                          val = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    diag_util_mprintf("Ip6 Multicast Configuration:\n");
    diag_util_mprintf("\tIp6 Multicast Asic Lookup                       : ");
    ret = rtk_l2_ip6mcMode_get(unit, &val);
    if (val == LOOKUP_ON_FVID_AND_MAC)
    {
        diag_util_mprintf("VID + MAC\n");
    }
    else if (val == LOOKUP_ON_DIP_AND_SIP)
    {
        diag_util_mprintf("DIP + SIP\n");
    }
    else if (val == LOOKUP_ON_DIP_AND_FVID)
    {
        diag_util_mprintf("DIP + VID\n");
    }


    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC
/*
  *  l2-table get ipmc-mismatch ( wrong-mcast-address | ipmc-with-ucast-mac )
  */
cparser_result_t cparser_cmd_l2_table_get_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    rtk_pri_t                      priority = 0;
    uint32                          dp = 0;
    uint32                           val = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_ipmcDstAddrMismatchAction_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tIpmc Dmac Unicast Address Action               : ");
        if (val == L2_IPMC_MIS_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (val == L2_IPMC_MIS_TRAP)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (val == L2_IPMC_MIS_FWD_AS_L2)
        {
            diag_util_mprintf("Forward As Normal L2 Packet\n");
        }
        else if (val == L2_IPMC_MIS_FWD_AS_IPMC)
        {
            diag_util_mprintf("Forward As Normal Ipmc Packet\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPriEnable_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &enable), ret);
        if(ENABLED == enable)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPri_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &priority), ret);
            diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Priority        : Enable(%u)\n", priority);
        }
        else
        {
            diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Priority        : Disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDPEnable_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &enable), ret);
        if(ENABLED == enable)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDP_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &dp), ret);
            diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Drop Precedence : Enable(%u)\n", dp);
        }
        else
        {
            diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Drop Precedence : Disable\n");
        }

        ret = rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Insert Cpu Tag  : %s\n", enable ? "Enable" : "Disable");
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_DROP_PRECEDENCE_STATE
/*
  *  l2-table get ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) drop-precedence state
  */
cparser_result_t cparser_cmd_l2_table_get_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_drop_precedence_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    uint32                          dp = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDPEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDP_get(unit, &dp), ret);
        diag_util_mprintf("\tLookup Miss Trap Drop Precedence               : Enable(%u)\n", dp);
    }
    else
    {
        diag_util_mprintf("\tLookup Miss Trap Drop Precedence               : Disable\n");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_INSERT_CPUTAG_STATE
 /*
  *   l2-table get ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) insert-cputag state
  */
cparser_result_t cparser_cmd_l2_table_get_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_insert_cputag_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_lookupMissAddCPUTagEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tLookup Miss Trap Insert Cpu Tag                : %s\n", enable ? "Enable" : "Disable");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IPMC_MISMATCH_WRONG_MCAST_ADDRESS_IPMC_WITH_UCAST_MAC_PRIORITY_STATE
 /*
  *   l2-table get ipmc-mismatch  ( wrong-mcast-address | ipmc-with-ucast-mac ) priority state
  */
cparser_result_t cparser_cmd_l2_table_get_ipmc_mismatch_wrong_mcast_address_ipmc_with_ucast_mac_priority_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
     rtk_pri_t                      priority = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPriEnable_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPri_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &priority), ret);
        diag_util_mprintf("\tIpmc Wrong Dmac Address Trap Priority          : Enable(%u)\n", priority);
    }
    else
    {
        diag_util_mprintf("\tIpmc Wrong Dmac Address Trap Priority          : Disable(%u)\n", priority);
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IPMC_MODE
/*
 *   l2-table get ipmc-mode
 */
cparser_result_t cparser_cmd_l2_table_get_ipmc_mode(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    uint32                          val = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ip Multicast Configuration:\n");
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("\tIp Multicast Asic Lookup                       : ");
        rtk_l2_ipmcMode_get(unit, &val);
        if (val == LOOKUP_ON_FVID_AND_MAC)
        {
            diag_util_mprintf("VID + MAC\n");
        }
        else if (val == LOOKUP_ON_DIP_AND_SIP)
        {
            diag_util_mprintf("DIP + SIP\n");
        }
        else if (val == LOOKUP_ON_DIP_AND_FVID)
        {
            diag_util_mprintf("DIP + VID\n");
        }
    }
    else
    {
        ret = rtk_l2_ipmcEnable_get(unit, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tIp Multicast Asic Lookup                       : ");
            if (ENABLED == enable)
            {
                diag_util_mprintf("Enable\n");

                ret = rtk_l2_ipmcMode_get(unit, &val);
                if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
                else
                {
                    diag_util_mprintf("\tIp Multicast Lookup Mode                       : ");
                    if (val == LOOKUP_ON_DIP_AND_SIP)
                    {
                        diag_util_mprintf("DIP + SIP\n");
                    }
                    else
                    {
                        diag_util_mprintf("DIP Only\n");
                    }
                }
            }
            else
            {
                diag_util_mprintf("Disable\n");
            }
        }
    }


    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_L2_TRAP_INSERT_CPUTAG_STATE
/*
 *   l2-table get l2-trap insert-cputag state
 */
cparser_result_t cparser_cmd_l2_table_get_l2_trap_insert_cputag_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_trapAddCPUTagEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tL2 Normal Trap Insert Cpu Tag                  : %s\n", enable ? "Enable" : "Disable");
    }


    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_L2_TRAP_PRIORITY_STATE
/*
 *   l2-table get l2-trap priority state
 */
cparser_result_t cparser_cmd_l2_table_get_l2_trap_priority_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    rtk_pri_t                      priority = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    diag_util_mprintf("Misc Configuration :\n");
    DIAG_UTIL_ERR_CHK(rtk_l2_trapPriEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
      DIAG_UTIL_ERR_CHK(rtk_l2_trapPri_get(unit, &priority), ret);
      diag_util_mprintf("\tL2 Normal Trap Priority                        : Enable(%u)\n", priority);
    }
    else
    {
      diag_util_mprintf("\tL2 Normal Trap Priority                        : Disable\n");
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING
/*
  *  l2-table get limit-learning
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l2_limitLearnCntAction_t  action;
    uint32                          mac_cnt, mac_cnt_disable;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("System Limit Learning Information\n");

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningCnt_get(unit, &mac_cnt), ret);

    DIAG_OM_GET_CHIP_CAPACITY(unit, mac_cnt_disable, l2_learn_limit_cnt_disable);
	if (mac_cnt == mac_cnt_disable)
		diag_util_mprintf("\tMax Mac Count : Unlimited\n");
	else
	    diag_util_mprintf("\tMax Mac Count : %u\n", mac_cnt);

    DIAG_UTIL_ERR_CHK(rtk_l2_learningCnt_get(unit, &mac_cnt), ret);

    diag_util_mprintf("\tCurrent Mac Count : %u\n", mac_cnt);

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningCntAction_get(unit, &action), ret);

    diag_util_mprintf("\tExceed Max Count Action : ");

    if (action == LIMIT_LEARN_CNT_ACTION_FORWARD)
    {
        diag_util_mprintf("Forward\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_DROP)
    {
        diag_util_mprintf("Drop\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_TO_CPU)
    {
        diag_util_mprintf("Trap-To-Cpu\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU)
    {
        diag_util_mprintf("Copy-To-Cpu\n");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_ACTION
/*
  *  l2-table get limit-learning action
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_action(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_l2_limitLearnCntAction_t  action;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("System Limit Learning Action: ");

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningCntAction_get(unit, &action), ret);

    if (action == LIMIT_LEARN_CNT_ACTION_FORWARD)
    {
        diag_util_mprintf("Forward\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_DROP)
    {
        diag_util_mprintf("Drop\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_TO_CPU)
    {
        diag_util_mprintf("Trap-To-Cpu\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU)
    {
        diag_util_mprintf("Copy-To-Cpu\n");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_TBL_INDEX_INDEX
/*
  *  l2-table get limit-learning tbl-index <UINT:index>
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_tbl_index_index(cparser_context_t *context,
    uint32_t *idx_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    uint32                      mac_cnt;
    rtk_l2_fidMacLimitEntry_t   limitEntry;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == idx_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("    Index%u :\n", *idx_ptr);

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *idx_ptr, &limitEntry), ret);

    diag_util_mprintf("        Max Mac Count           : %u\n", limitEntry.maxNum);

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLearningCnt_get(unit, *idx_ptr, &mac_cnt), ret);

    diag_util_mprintf("        Current Mac Count       : %u\n", mac_cnt);

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_VLAN_BASED
/*
 *   l2-table get limit-learning vlan-based
 */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_vlan_based(cparser_context_t *context)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_l2_limitLearnCntAction_t   action;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("VLAN-based Limit Learning Action : ");

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLearningCntAction_get(unit, &action), ret);

    if (action == LIMIT_LEARN_CNT_ACTION_FORWARD)
    {
        diag_util_mprintf("Forward\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_DROP)
    {
        diag_util_mprintf("Drop\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_TO_CPU)
    {
        diag_util_mprintf("Trap-To-Cpu\n");
    }
    else if (action == LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU)
    {
        diag_util_mprintf("Copy-To-Cpu\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_VLAN_BASED_INDEX
/*
  *  l2-table get limit-learning vlan-based <UINT:index>
  */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_vlan_based_index(cparser_context_t *context,
    uint32_t *idx_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    uint32                      mac_cnt;
    rtk_l2_fidMacLimitEntry_t   limitEntry;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == idx_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("VLAN-based Limit Learning Information \n");

    diag_util_mprintf("\tIndex %2d :\n", *idx_ptr);

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_get(unit, *idx_ptr, &limitEntry), ret);

    diag_util_mprintf("\tConstraint VID : %4d\n", limitEntry.fid);

    diag_util_mprintf("\tConstraint Port : %2d\n", limitEntry.port);

    diag_util_mprintf("\tMax Mac Count : %u\n", limitEntry.maxNum);

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLearningCnt_get(unit, *idx_ptr, &mac_cnt), ret);

    diag_util_mprintf("\tCurrent Mac Count : %u\n", mac_cnt);

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_DROP_PRECEDENCE_STATE
/*
 *   l2-table get limit-learning drop-precedence state
 */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_drop_precedence_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    uint32                          dp = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapDPEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
      DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapDP_get(unit, &dp), ret);
      diag_util_mprintf("\tLimit Learning Trap Drop Precedence            : ENABLE(%u)\n", dp);
    }
    else
    {
      diag_util_mprintf("\tLimit Learning Trap Drop Precedence            : DISABLE\n");
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_INSERT_CPUTAG_STATE
/*
 *   l2-table get limit-learning insert-cputag state
 */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_insert_cputag_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_limitLearningTrapAddCPUTagEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
      DIAG_ERR_PRINT(ret);
      return CPARSER_NOT_OK;
    }
    else
    {
      diag_util_mprintf("\tLimit Learning Trap Insert Cpu Tag             : %s\n", enable ? "ENABLE" : "DISABLE");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LIMIT_LEARNING_PRIORITY_STATE
/*
 *   l2-table get limit-learning priority state
 */
cparser_result_t cparser_cmd_l2_table_get_limit_learning_priority_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    rtk_pri_t                      priority = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapPriEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapPri_get(unit, &priority), ret);
        diag_util_mprintf("\tLimit Learning Trap Priority                   : ENABLE(%u)\n", priority);
    }
    else
    {
        diag_util_mprintf("\tLimit Learning Trap Priority                   : DISABLE(%u)\n", priority);
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LOOKUP_MISS_PORT_ALL
/*
  *  l2-table get lookup-miss ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_get_lookup_miss_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_action_t        action;
    diag_portlist_t     portlist;


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("Port Lookup Miss Information:\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %u :\n", port);

        DIAG_UTIL_ERR_CHK(rtk_l2_portLookupMissAction_get(unit, port, DLF_TYPE_UCAST, &action), ret);
        diag_util_mprintf("\tUnicast Lookup Action        : ");
        if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (action == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
        else if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == ACTION_COPY2CPU)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
        else if (action == ACTION_FLOOD_IN_ALL_PORT)
        {
            diag_util_mprintf("Flood-In-All-Port\n");
        }
        else
        {
            diag_util_mprintf("Unknown action\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_portLookupMissAction_get(unit, port, DLF_TYPE_MCAST, &action), ret);
        diag_util_mprintf("\tL2 Multicast Lookup Action   : ");
        if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (action == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
        else if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == ACTION_COPY2CPU)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
        else if (action == ACTION_FLOOD_IN_ALL_PORT)
        {
            diag_util_mprintf("Flood-In-All-Port\n");
        }
        else
        {
            diag_util_mprintf("Unknown action\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_portLookupMissAction_get(unit, port, DLF_TYPE_IPMC, &action), ret);
        diag_util_mprintf("\tIPv4 Multicast Lookup Action : ");
        if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (action == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
        else if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == ACTION_COPY2CPU)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
        else if (action == ACTION_FLOOD_IN_ALL_PORT)
        {
            diag_util_mprintf("Flood-In-All-Port\n");
        }
        else
        {
            diag_util_mprintf("Unknown action\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_portLookupMissAction_get(unit, port, DLF_TYPE_IP6MC, &action), ret);
        diag_util_mprintf("\tIPv6 Multicast Lookup Action : ");
        if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (action == ACTION_FLOOD_IN_VLAN)
        {
            diag_util_mprintf("Flood-In-Vlan\n");
        }
        else if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == ACTION_COPY2CPU)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
        else if (action == ACTION_FLOOD_IN_ALL_PORT)
        {
            diag_util_mprintf("Flood-In-All-Port\n");
        }
        else
        {
            diag_util_mprintf("Unknown action\n");
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LOOKUP_MISS_BCAST_UNICAST_FWD_TBL_IDX
/*
  * l2-table get lookup-miss ( bcast | unicast ) fwd-tbl-idx
  */
cparser_result_t cparser_cmd_l2_table_get_lookup_miss_bcast_unicast_fwd_tbl_idx(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t     type = DLF_TYPE_UCAST;
	uint32						index;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if ('u' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_UCAST;
    }
    else if ('b' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_BCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissFloodPortMaskIdx_get(unit, type, &index), ret);
	if (DLF_TYPE_UCAST == type)
		diag_util_mprintf("\tUnicast lookup miss forwarding table index : %d\n", index);
	else
		diag_util_mprintf("\tBroadcast lookup miss forwarding table index : %d\n", index);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_EXCEPT_SMAC_MCAST_BCAST_SA
/*
  *  l2-table get except-smac mcast-bcast-sa
  */
cparser_result_t cparser_cmd_l2_table_get_except_smac_mcast_bcast_sa(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          val = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_BCAST_OR_MCAST, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Multicast or Broadcast Src Mac Action : ");
        if (val == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (val == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap to CPU\n");
        }
        else
        {
            diag_util_mprintf("Forward\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IP_MCAST_DIP_CHECK
/*
  *  l2-table get ip-mcast dip-check
 */
cparser_result_t cparser_cmd_l2_table_get_ip_mcast_dip_check(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_ipMcastAddrChkEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    IP Multicast DIP check: %s\n", enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_IP_MCAST_VLAN_COMPARE
/*
  *  l2-table get ip-mcast vlan-compare
  */
cparser_result_t cparser_cmd_l2_table_get_ip_mcast_vlan_compare(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_ipMcstFidVidCompareEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    IP Multicast VLAN Compare: %s\n", enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_DYNAMIC_PORT_MOVE_PORT_ALL
/*
 *   l2-table get dynamic-port-move ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_l2_table_get_dynamic_port_move_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_action_t        action;
    diag_portlist_t     portlist;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\tPort %u : ", port);

        DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveAction_get(unit, port, &action), ret);
        if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == ACTION_COPY2CPU)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_CAM_STATE
/*
  *  l2-table get cam state
  */
cparser_result_t cparser_cmd_l2_table_get_cam_state(cparser_context_t *context)
{

    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_camEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
       DIAG_ERR_PRINT(ret);
       return CPARSER_NOT_OK;
    }
    else
    {
       diag_util_mprintf("\tL2 Table CAM                                   : %s\n", enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LOOKUP_MISS_DROP_PRECEDENCE_STATE
/*
 *   l2-table get lookup-miss drop-precedence state
 */
cparser_result_t cparser_cmd_l2_table_get_lookup_miss_drop_precedence_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    uint32                          dp = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDPEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDP_get(unit, &dp), ret);
        diag_util_mprintf("    Lookup Miss Trap Drop Precedence : Enable(%u)\n", dp);
    }
    else
    {
        diag_util_mprintf("    Lookup Miss Trap Drop Precedence : Disable\n");
    }


    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LOOKUP_MISS_INSERT_CPUTAG_STATE
/*
 *   l2-table get lookup-miss insert-cputag state
 */
cparser_result_t cparser_cmd_l2_table_get_lookup_miss_insert_cputag_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_lookupMissAddCPUTagEnable_get(unit, &enable);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("    Lookup Miss Trap Insert Cpu Tag  : %s\n", enable ? "Enable" : "Disable");
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LOOKUP_MISS_PRIORITY_STATE
/*
 *   l2-table get lookup-miss priority state
 */
cparser_result_t cparser_cmd_l2_table_get_lookup_miss_priority_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    rtk_pri_t                       priority = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPriEnable_get(unit, &enable), ret);
    if(ENABLED == enable)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPri_get(unit, &priority), ret);
        diag_util_mprintf("    Lookup Miss Trap Priority        : Enable(%u)\n", priority);
    }
    else
    {
        diag_util_mprintf("    Lookup Miss Trap Priority        : Disable\n");
    }


    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_STTC_PORT_MOVE_PORT_ALL
/*
 *   l2-table get sttc-port-move ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_l2_table_get_sttc_port_move_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_action_t        action;
    diag_portlist_t     portlist;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\tPort %u : ", port);

        DIAG_UTIL_ERR_CHK(rtk_l2_staticPortMoveAction_get(unit, port, &action), ret);
        if (action == ACTION_FORWARD)
        {
            diag_util_mprintf("Forward\n");
        }
        else if (action == ACTION_DROP)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (action == ACTION_TRAP2CPU)
        {
            diag_util_mprintf("Trap-To-Cpu\n");
        }
        else if (action == ACTION_COPY2CPU)
        {
            diag_util_mprintf("Copy-To-Cpu\n");
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_ROUTER_PORT
/*
 *   l2-table get router-port
 */
cparser_result_t cparser_cmd_l2_table_get_router_port(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_portmask_t                  portmask;
    uint8                           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_ipmc_routerPorts_get(unit, &portmask);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
       DIAG_ERR_PRINT(ret);
       return CPARSER_NOT_OK;
    }
    else
    {
       memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
       diag_util_lPortMask2str(port_list, &portmask);
       diag_util_mprintf("\tIp Multicast Router Port List                  : %s\n", port_list);
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_LINK_DOWN_FLUSH_STATE
/*
 *   l2-table get link-down-flush state
 */
cparser_result_t cparser_cmd_l2_table_get_link_down_flush_state(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l2_flushLinkDownPortAddrEnable_get(unit, &enable), ret);
    diag_util_mprintf("\tInvalidate Link Down                           : %s\n", enable ? "Enable" : "Disable");

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_MCAST_BLOCK_PORT
/*
 *   l2-table get mcast-block-port
 */
cparser_result_t cparser_cmd_l2_table_get_mcast_block_port(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_portmask_t                  portmask;
    uint8                           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_mcastBlockPortmask_get(unit, &portmask);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &portmask);
        diag_util_mprintf("\tL2 Multicast Block Port List                   : %s\n", port_list);
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_MCAST_MODE
/*
 * l2-table get mcast-mode
 */
cparser_result_t cparser_cmd_l2_table_get_mcast_mode(cparser_context_t *context)
{
    uint32                      unit = 0, fid = 0;
    rtk_l2_mcastLookupMode_t    mode = 0;
    int32                       ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_l2_mcastLookupMode_get(unit, &mode, &fid);
    if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_printf("\tL2 Multicast Lookup Mode: ");
        if (MC_LOOKUP_ON_FID == mode)
            diag_util_mprintf("FID\n");
        else if (MC_LOOKUP_ON_VID == mode)
            diag_util_mprintf("VID\n");
        else
        {
            diag_util_mprintf("Fix-FID, %d\n", fid);
        }
    }

    diag_util_mprintf("\n");
    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_SRC_PORT_EGRESS_FILTER
/*
 *   l2-table get src-port-egress-filter
 */
cparser_result_t cparser_cmd_l2_table_get_src_port_egress_filter (cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_portmask_t                  portmask;
    uint8                           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l2_srcPortEgrFilterMask_get(unit, &portmask), ret);
    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
    diag_util_lPortMask2str(port_list, &portmask);
    diag_util_mprintf("\tSource Port Egress Filter                      : %s\n", port_list);

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_VLAN_FWD_MODE_PORT_ALL
/*
 *   l2-table get vlan-fwd-mode ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_l2_table_get_vlan_fwd_mode_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    diag_portlist_t               portlist;
    rtk_l2_vlanMode_t         vlanMode = BASED_ON_INNER_VLAN;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2d ", port);
        ret = rtk_l2_vlanMode_get(unit, port, &vlanMode);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
           DIAG_ERR_PRINT(ret);
           return CPARSER_NOT_OK;
        }
        else
        {
           diag_util_printf("Vlan Forward Mode                 : ");
           if (vlanMode == BASED_ON_INNER_VLAN)
           {
               diag_util_mprintf("Base-On-INNER-Vid\n");
           }
           else
           {
               diag_util_mprintf("Base-On-OUTER-Vid\n");
           }
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_VLAN_LEANING_METHOD
/*
 *   l2-table get vlan-leaning-method
 */
cparser_result_t cparser_cmd_l2_table_get_vlan_leaning_method(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    uint32                          val = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    ret = rtk_l2_ucastLookupMode_get(unit, &val);
    if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("\tIVL/SVL Mode                                   : ");
        if (val == UC_LOOKUP_ON_VID)
        {
            diag_util_mprintf("IVL\n");
        }
        else
        {
            diag_util_mprintf("SVL\n");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_SECURE_MAC_MODE
/*
 *   l2-table get secure-mac-mode
 */
cparser_result_t cparser_cmd_l2_table_get_secure_mac_mode(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_l2_secureMacMode_get(unit, &enable), ret);
    diag_util_mprintf("\tSecure MAC mode                           : %s\n", enable ? "Enable" : "Disable");

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_SRC_MAC_LEARNING_PORT_ALL_STATE
/*
 *   l2-table get src-mac-learning ( <PORT_LIST:port> | all ) state
 */
cparser_result_t cparser_cmd_l2_table_get_src_mac_learning_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_enable_t                    enable = DISABLED;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2d ", port);

        ret = rtk_l2_learningEnable_get(unit, port, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("Src Mac Learning                  : %s\n", enable ? "Enable" : "Disable");
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_SRC_MAC_PORT_ALL
/*
 *   l2-table get src-mac ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_l2_table_get_src_mac_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_l2_newMacLrnMode_t          lrnMode;
    rtk_action_t                    fwdAction;
    diag_portlist_t                 portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %d :\n", port);

        ret = rtk_l2_newMacOp_get(unit, port, &lrnMode, &fwdAction);
       if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
       {
           DIAG_ERR_PRINT(ret);
           return CPARSER_NOT_OK;
       }
       else
       {
           diag_util_mprintf("\tNew Src Mac Operation             : ");
           if (lrnMode == HARDWARE_LEARNING)
           {
               diag_util_mprintf("Asic-AUTO-Learn\n");
           }
           else if(lrnMode == SOFTWARE_LEARNING)
           {
               diag_util_mprintf("Learn-As-SUSPEND\n");
           }
           else
           {
               diag_util_mprintf("Not-Learn\n");
           }

           diag_util_mprintf("\tPacket Action                     : ");
           if (fwdAction == ACTION_FORWARD)
           {
               diag_util_mprintf("Forward\n");
           }
           else if(fwdAction == ACTION_COPY2CPU)
           {
               diag_util_mprintf("Copy-To-Cpu\n ");
           }
           else if(fwdAction == ACTION_TRAP2CPU)
           {
               diag_util_mprintf("Trap-To Cpu\n");
           }
           else
           {
               diag_util_mprintf("Drop\n");
           }
       }
    }
     return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_RESET_LIMIT_LEARNING_CURRENT_COUNT_INDEX
/*
  *  l2-table reset limit-learning current-count <UINT:index>
  */
cparser_result_t cparser_cmd_l2_table_reset_limit_learning_current_count_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == index_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > 31), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLearningCnt_reset(unit, *index_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_EXCEPT_SMAC_ZERO_SA_LEARNING
/*
  *  l2-table get except-smac zero-sa-learning
  */
cparser_result_t cparser_cmd_l2_table_get_except_smac_zero_sa_learning(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_l2_zeroSALearningEnable_get(unit, &enable);
    diag_util_mprintf("\tZero source mac learning                           : %s\n", enable ? "Enable" : "Disable");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_UCAST_VID_MAC_PORT_PORT_SA_BLOCK_DA_BLOCK_STATIC_NEXTHOP_SUSPEND_AGED
/*
 * l2-table add mac-ucast <UINT:vid> <MACADDR:mac> port <UINT:port> { sa-block } { da-block } { static } { nexthop } { suspend } { aged }
 */
cparser_result_t cparser_cmd_l2_table_add_mac_ucast_vid_mac_port_port_sa_block_da_block_static_nexthop_suspend_aged(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    uint32_t *port_ptr)
{
    int32           flag_num = 0;
    uint32          sa_block = FALSE;
    uint32          da_block = FALSE;
    uint32          is_static = FALSE;
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    uint32          is_auth = FALSE;
#endif
    uint32          nexthop = FALSE;
    uint32          suspend = FALSE;
    uint32          aged = FALSE;
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ucastAddr_t  l2_uAddr;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l2_uAddr, 0, sizeof(rtk_l2_ucastAddr_t));

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    /*from first optional token*/
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    is_auth = FALSE;
#endif
    for(flag_num = 7; flag_num < TOKEN_NUM; flag_num++)
    {
        if ('s' == TOKEN_CHAR(flag_num, 0))
        {
            if ('a' == TOKEN_CHAR(flag_num, 1))
            {
                sa_block = TRUE;
            }
            else if ('t' == TOKEN_CHAR(flag_num, 1))
            {
                is_static = TRUE;
            }
            else if ('u' == TOKEN_CHAR(flag_num, 1))
            {
                suspend = TRUE;
            }
        }
        else if ('d' == TOKEN_CHAR(flag_num, 0))
        {
            da_block = TRUE;
        }
        else if ('n' == TOKEN_CHAR(flag_num, 0))
        {
            nexthop = TRUE;
        }
        else if ('a' == TOKEN_CHAR(flag_num, 0))
        {
            aged = TRUE;
        }
    }

    /* Fill structure */
    l2_uAddr.vid = *vid_ptr;
    memcpy(l2_uAddr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2_uAddr.port = *port_ptr;
    if(sa_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_SA_BLOCK;

    if(da_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_DA_BLOCK;

    if(is_static)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_STATIC;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_STATIC;

    if(nexthop)
    {
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
        l2_uAddr.isAged = aged;
    }
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_NEXTHOP;

    if(suspend)
        l2_uAddr.state |= RTK_L2_UCAST_STATE_SUSPEND;
    else
        l2_uAddr.state&= ~RTK_L2_UCAST_STATE_SUSPEND;

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    l2_uAddr.agg_vid = 0;
#endif
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    if(is_auth)
        l2_uAddr.auth = TRUE;
    else
        l2_uAddr.auth = FALSE;
#endif

    DIAG_UTIL_ERR_CHK(rtk_l2_addr_add(unit, &l2_uAddr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_UCAST_VID_MAC_PORT_AUTH_SA_BLOCK_DA_BLOCK_STATIC_NEXTHOP_SUSPEND
/*
  * l2-table add mac-ucast <UINT:vid> <MACADDR:mac> <UINT:port> { auth } { sa-block  } { da-block  } { static } { nexthop } { suspend }
  */
cparser_result_t cparser_cmd_l2_table_add_mac_ucast_vid_mac_port_auth_sa_block_da_block_static_nexthop_suspend(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    uint32_t *port_ptr)
{
    uint32          flag_num = 0;
    uint32          sa_block = FALSE;
    uint32          da_block = FALSE;
    uint32          is_static = FALSE;
    uint32          is_auth = FALSE;
    uint32          nexthop = FALSE;
    uint32          suspend = FALSE;
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ucastAddr_t  l2_uAddr;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l2_uAddr, 0, sizeof(rtk_l2_ucastAddr_t));

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    /*from first optional token*/
    for(flag_num = 6; flag_num < TOKEN_NUM; flag_num++)
    {
        if ('s' == TOKEN_CHAR(flag_num, 0))
        {
            if ('a' == TOKEN_CHAR(flag_num, 1))
            {
                sa_block = TRUE;
            }
            else if ('t' == TOKEN_CHAR(flag_num, 1))
            {
                is_static = TRUE;
            }
            else if ('u' == TOKEN_CHAR(flag_num, 1))
            {
                suspend = TRUE;
            }
        }
        else if ('d' == TOKEN_CHAR(flag_num, 0))
        {
            da_block = TRUE;
        }
        else if ('a' == TOKEN_CHAR(flag_num, 0))
        {
            is_auth = TRUE;
        }
        else if ('n' == TOKEN_CHAR(flag_num, 0))
        {
            nexthop = TRUE;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    /* Fill structure */
    l2_uAddr.vid = *vid_ptr;
    memcpy(l2_uAddr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2_uAddr.port = *port_ptr;
    if(sa_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_SA_BLOCK;

    if(da_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_DA_BLOCK;

    if(is_static)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_STATIC;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_STATIC;

    if(nexthop)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_NEXTHOP;

    if(suspend)
        l2_uAddr.state |= RTK_L2_UCAST_STATE_SUSPEND;
    else
        l2_uAddr.state&= ~RTK_L2_UCAST_STATE_SUSPEND;

    if(is_auth)
        l2_uAddr.auth = TRUE;
    else
        l2_uAddr.auth = FALSE;

    DIAG_UTIL_ERR_CHK(rtk_l2_addr_add(unit, &l2_uAddr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP
/*
  *  l2-table dump
  */
cparser_result_t cparser_cmd_l2_table_dump(cparser_context_t *context)
{
    uint32                          unit = 0;
    uint32                          aging_time = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    rtk_portmask_t                  portmask;
    uint8                           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_pri_t                      priority = 0;
    uint32                          dp = 0;
    uint32                           val = 0;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    /* show configured register setting */
    diag_util_mprintf("L2 Lookup Table Information :\n");

    DIAG_UTIL_ERR_CHK(rtk_l2_aging_get(unit, &aging_time), ret);
    diag_util_mprintf("\tAging Time                                     : %d seconds.\n", aging_time);

    DIAG_UTIL_ERR_CHK(rtk_l2_flushLinkDownPortAddrEnable_get(unit, &enable), ret);
    diag_util_mprintf("\tInvalidate Link Down                           : %s\n", enable ? "Enable" : "Disable");

    ret = 0;
    if (DIAG_OM_GET_REALCHIPID(RTL8389M_CHIP_ID))
    {
        ret = rtk_l2_lookupMissFloodPortMask_get(unit, DLF_TYPE_ANY, &portmask);
        if (ret != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
            diag_util_lPortMask2str(port_list, &portmask);
            diag_util_mprintf("\tLookup Miss Flood Port                         : %s\n", port_list);
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_srcPortEgrFilterMask_get(unit, &portmask), ret);
    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
    diag_util_lPortMask2str(port_list, &portmask);
    diag_util_mprintf("\tSource Port Egress Filter                      : %s\n", port_list);


    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        ret = rtk_l2_camEnable_get(unit, &enable);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tL2 Lookup CAM Table                            : %s\n", enable ? "Enable" : "Disable");
        }
    }

    ret = rtk_l2_hashAlgo_get(unit, &val);
    if (ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_printf("\tL2 Table Hash Algorithm                        : ");
        if (val == 0)
        {
            diag_util_mprintf("ALGO-0\n");
        }
        else
        {
            diag_util_mprintf("ALGO-1\n");
        }
    }


    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        ret = rtk_l2_LRUEnable_get(unit, &enable);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tL2 Table LRU                                   : %s\n", enable ? "Enable" : "Disable");
        }
    }

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        ret = rtk_l2_ucastLookupMode_get(unit, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
        {
            diag_util_mprintf("\tIVL/SVL Mode                                   : %s\n", "No supported");
        }
        else
        {
            diag_util_printf("\tIVL/SVL Mode                                   : ");
            if (val == UC_LOOKUP_ON_VID)
            {
                diag_util_mprintf("IVL\n");
            }
            else
            {
                diag_util_mprintf("SVL\n");
            }
        }
    }

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        ret = rtk_l2_mcastBlockPortmask_get(unit, &portmask);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
            diag_util_lPortMask2str(port_list, &portmask);
            diag_util_mprintf("\tL2 Multicast Block Port List                   : %s\n", port_list);
        }
    }

    diag_util_mprintf("Ip Multicast Configuration:\n");
    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        ret = rtk_l2_ipmcEnable_get(unit, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tIp Multicast Asic Lookup                       : ");
            if (ENABLED == enable)
            {
                diag_util_mprintf("Enable\n");

                ret = rtk_l2_ipmcMode_get(unit, &val);
                if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
                else
                {
                    diag_util_printf("\tIp Multicast Lookup Mode                       : ");
                    if (val == LOOKUP_ON_DIP_AND_SIP)
                    {
                        diag_util_mprintf("DIP + SIP\n");
                    }
                    else
                    {
                        diag_util_mprintf("DIP Only\n");
                    }
                }
            }
            else
            {
                diag_util_mprintf("Disable\n");
            }
        }

        ret = rtk_l2_ipmc_routerPorts_get(unit, &portmask);
        if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
            diag_util_lPortMask2str(port_list, &portmask);
            diag_util_mprintf("\tIp Multicast Router Port List                  : %s\n", port_list);
        }

        ret = rtk_l2_ipmcDstAddrMismatchAction_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tIpmc Dmac Unicast Address Action               : ");
            if (val == L2_IPMC_MIS_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == L2_IPMC_MIS_TRAP)
            {
                diag_util_mprintf("Trap-To-Cpu\n");
            }
            else if (val == L2_IPMC_MIS_FWD_AS_L2)
            {
                diag_util_mprintf("Forward As Normal L2 Packet\n");
            }
            else if (val == L2_IPMC_MIS_FWD_AS_IPMC)
            {
                diag_util_mprintf("Forward As Normal Ipmc Packet\n");
            }

            DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPriEnable_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &enable), ret);
            if(ENABLED == enable)
            {
                DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPri_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &priority), ret);
                diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Priority        : Enable(%u)\n", priority);
            }
            else
            {
                diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Priority        : Disable\n");
            }

            DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDPEnable_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &enable), ret);
            if(ENABLED == enable)
            {
                DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDP_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &dp), ret);
                diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Drop Precedence : Enable(%u)\n", dp);
            }
            else
            {
                diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Drop Precedence : Disable\n");
            }

            ret = rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(unit, L2_IPMC_MIS_TYPE_UCAST_ADDR, &enable);
            if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\tIpmc Dmac Unicast Address Trap Insert Cpu Tag  : %s\n", enable ? "Enable" : "Disable");
            }
        }

        ret = rtk_l2_ipmcDstAddrMismatchAction_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tIpmc Wrong Dmac Address Action                 : ");
            if (val == L2_IPMC_MIS_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == L2_IPMC_MIS_TRAP)
            {
                diag_util_mprintf("Trap-To-Cpu\n");
            }
            else if (val == L2_IPMC_MIS_FWD_AS_L2)
            {
                diag_util_mprintf("Forward As Normal L2 Packet\n");
            }
            else if (val == L2_IPMC_MIS_FWD_AS_IPMC)
            {
                diag_util_mprintf("Forward As Normal Ipmc Packet\n");
            }

            DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPriEnable_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &enable), ret);
            if(ENABLED == enable)
            {
                DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchPri_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &priority), ret);
                diag_util_mprintf("\tIpmc Wrong Dmac Address Trap Priority          : Enable(%u)\n", priority);
            }
            else
            {
                diag_util_mprintf("\tIpmc Wrong Dmac Address Trap Priority          : Disable\n");
            }

            DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDPEnable_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &enable), ret);
            if(ENABLED == enable)
            {
                DIAG_UTIL_ERR_CHK(rtk_l2_ipmcDstAddrMismatchDP_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &dp), ret);
                diag_util_mprintf("\tIpmc Wrong Dmac Address Trap Drop Precedence   : Enable(%u)\n", dp);
            }
            else
            {
                diag_util_mprintf("\tIpmc Wrong Dmac Address Trap Drop Precedence   : Disable\n");
            }

            ret = rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(unit, L2_IPMC_MIS_TYPE_WRONG_MCAST_ADDR, &enable);
            if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\tIpmc Wrong Dmac Address Trap Insert Cpu Tag    : %s\n", enable ? "Enable" : "Disable");
            }
        }
    }

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        ret = rtk_l2_ipmcMode_get(unit, &val);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tIp Multicast Lookup Mode                       : ");
            if (val == LOOKUP_ON_FVID_AND_MAC)
            {
                diag_util_mprintf("VID + MAC\n");
            }
            else if (val == LOOKUP_ON_DIP_AND_SIP)
            {
                diag_util_mprintf("DIP + SIP\n");
            }
            else
            {
                diag_util_mprintf("VID + DIP\n");
            }
        }
    }

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
    	diag_util_mprintf("Ipv6 Multicast Configuration:\n");
		
        ret = rtk_l2_ip6mcMode_get(unit, &val);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tIpv6 Multicast Lookup Mode                     : ");
            if (val == LOOKUP_ON_FVID_AND_MAC)
            {
                diag_util_mprintf("VID + MAC\n");
            }
            else if (val == LOOKUP_ON_DIP_AND_SIP)
            {
                diag_util_mprintf("DIP + SIP\n");
            }
            else
            {
                diag_util_mprintf("VID + DIP\n");
            }
        }
    }

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Address Table Lookup Miss Configuration : \n");
        ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_IPMC, &val);
        if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tIpmc Lookup Miss Action                        : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("Trap-To-Cpu\n");
            }
            else if (val == ACTION_FLOOD_IN_VLAN)
            {
                diag_util_mprintf("Flood-In-Vlan\n");
            }
            else if (val == ACTION_FLOOD_IN_ROUTER_PORTS)
            {
                diag_util_mprintf("Flood-In-Router-Ports\n");
            }
        }

        ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_UCAST, &val);
        if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tL2 Unicast Lookup Miss Action                  : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("Trap-To-Cpu\n");
            }
            else if (val == ACTION_FLOOD_IN_VLAN)
            {
                diag_util_mprintf("Flood-In-Vlan\n");
            }
            else if (val == ACTION_FLOOD_IN_ALL_PORT)
            {
                diag_util_mprintf("Flood-In-All-Ports\n");
            }
        }

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_BCAST, &val);
            if ((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_printf("\tBroadcast Lookup Miss Action                   : ");
                if (val == ACTION_DROP)
                {
                    diag_util_mprintf("Drop\n");
                }
                else if (val == ACTION_TRAP2CPU)
                {
                    diag_util_mprintf("Trap-To-Cpu\n");
                }
                else if (val == ACTION_FLOOD_IN_VLAN)
                {
                    diag_util_mprintf("Flood-In-Vlan\n");
                }
            }
        }

        ret = rtk_l2_lookupMissAction_get(unit, DLF_TYPE_MCAST, &val);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tL2 Multicast Lookup Miss Action                : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if (val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("Trap-To-Cpu\n");
            }
            else if (val == ACTION_FLOOD_IN_VLAN)
            {
                diag_util_mprintf("Flood-In-Vlan\n");
            }
            else if (val == ACTION_FLOOD_IN_ALL_PORT)
            {
                diag_util_mprintf("Flood-In-All-Ports\n");
            }
        }
    }

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPriEnable_get(unit, &enable), ret);
        if(ENABLED == enable)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissPri_get(unit, &priority), ret);
            diag_util_mprintf("\tLookup Miss Trap Priority                      : Enable(%u)\n", priority);
        }
        else
        {
            diag_util_mprintf("\tLookup Miss Trap Priority                      : Disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDPEnable_get(unit, &enable), ret);
        if(ENABLED == enable)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissDP_get(unit, &dp), ret);
            diag_util_mprintf("\tLookup Miss Trap Drop Precedence               : Enable(%u)\n", dp);
        }
        else
        {
            diag_util_mprintf("\tLookup Miss Trap Drop Precedence               : Disable\n");
        }

        ret = rtk_l2_lookupMissAddCPUTagEnable_get(unit, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tLookup Miss Trap Insert Cpu Tag                : %s\n", enable ? "Enable" : "Disable");
        }
    }

    diag_util_printf("Exception Src Mac Configuration :\n");
    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_MCAST, &val);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tMulticast Src Mac Action                       : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }

        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_BCAST, &val);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tBroadcast Src Mac Action                       : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }

        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_ZERO, &val);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tZero Src Mac Action                            : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }

        diag_util_mprintf("Limit Learning Configuration :\n");
        ret = rtk_l2_limitLearningTrapPriEnable_get(unit, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("\tLimit Learning Trap Priority                   : No supported\n");
        else
        {
            if(ENABLED == enable)
            {
                DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapPri_get(unit, &priority), ret);
                diag_util_mprintf("\tLimit Learning Trap Priority                   : ENABLE(%u)\n", priority);
            }
            else
            {
                diag_util_mprintf("\tLimit Learning Trap Priority                   : DISABLE\n");
            }
        }

        ret = rtk_l2_limitLearningTrapDPEnable_get(unit, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("\tLimit Learning Trap Drop Precedence            : No supported\n");
        else
        {
            if(ENABLED == enable)
            {
                DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningTrapDP_get(unit, &dp), ret);
                diag_util_mprintf("\tLimit Learning Trap Drop Precedence            : ENABLE(%u)\n", dp);
            }
            else
            {
                diag_util_mprintf("\tLimit Learning Trap Drop Precedence            : DISABLE\n");
            }
        }

        ret = rtk_l2_limitLearningTrapAddCPUTagEnable_get(unit, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("\tLimit Learning Trap Insert Cpu Tag             : No supported\n");
        else
        {
            diag_util_mprintf("\tLimit Learning Trap Insert Cpu Tag             : %s\n", enable ? "ENABLE" : "DISABLE");
        }

        diag_util_mprintf("Misc Configuration :\n");
        DIAG_UTIL_ERR_CHK(rtk_l2_trapPriEnable_get(unit, &enable), ret);
        if(ENABLED == enable)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_trapPri_get(unit, &priority), ret);
            diag_util_mprintf("\tL2 Normal Trap Priority                        : Enable(%u)\n", priority);
        }
        else
        {
            diag_util_mprintf("\tL2 Normal Trap Priority                        : Disable\n");
        }

        ret = rtk_l2_trapAddCPUTagEnable_get(unit, &enable);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tL2 Normal Trap Insert Cpu Tag                  : %s\n", enable ? "Enable" : "Disable");
        }
    }

    if (
            DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) || 
            DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID) 
        )
    {
        ret = rtk_l2_exceptionAddrAction_get(unit, SA_IS_BCAST_OR_MCAST, &val);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_printf("\tBroadcast/Multicast Src Mac Action             : ");
            if (val == ACTION_DROP)
            {
                diag_util_mprintf("Drop\n");
            }
            else if(val == ACTION_TRAP2CPU)
            {
                diag_util_mprintf("Trap\n");
            }
            else
            {
                diag_util_mprintf("Forward\n");
            }
        }

        ret = rtk_l2_zeroSALearningEnable_get(unit, &enable);
        if (ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tZero SA Learning                               : %s", enable ? "Enable" : "Disable");
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_PORT_ALL
/*
  *  l2-table dump  ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_dump_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    uint32                          mac_cnt = 0, mac_cnt_disable;
    int32                           ret = RT_ERR_FAILED;
    rtk_enable_t                    enable = DISABLED;
    rtk_port_t                      port = 0;
    rtk_l2_limitLearnCntAction_t    action = LIMIT_LEARN_CNT_ACTION_DROP;
    rtk_action_t                    portMove_action = ACTION_DROP;
    rtk_portmask_t                  portmask;
    uint8                           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_l2_vlanMode_t         vlanMode = BASED_ON_INNER_VLAN;
    rtk_l2_newMacLrnMode_t  lrnMode;
    rtk_action_t                     fwdAction;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %d :\n", port);

        DIAG_UTIL_ERR_CHK(rtk_l2_portLearningCnt_get(unit, port, &mac_cnt), ret);
        diag_util_mprintf("\tLearning Count                    : %d\n", mac_cnt);

        ret = rtk_l2_portLimitLearningCntEnable_get(unit, port, &enable);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
        }
        else
        {
            diag_util_mprintf("\tLimit Learning Function(per-port) : %s\n", enable ? "ENABLE" : "DISABLE");
        }

        if(ENABLED == enable)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCnt_get(unit, port, &mac_cnt), ret);
        	DIAG_OM_GET_CHIP_CAPACITY(unit, mac_cnt_disable, l2_learn_limit_cnt_disable);
        	if (mac_cnt == mac_cnt_disable)
        		diag_util_mprintf("\t\tLimit Learning Max Count  : Unlimited\n");
        	else
        	    diag_util_mprintf("\t\tLimit Learning Max Count  : %u\n", mac_cnt);

            diag_util_mprintf("\t\tLimit Learning Action     : ");
            DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCntAction_get(unit, port, &action), ret);
            if (LIMIT_LEARN_CNT_ACTION_FORWARD == action)
            {
                diag_util_mprintf("Forward");
            }
            else if (LIMIT_LEARN_CNT_ACTION_DROP == action)
            {
                diag_util_mprintf("Drop");
            }
            else if (LIMIT_LEARN_CNT_ACTION_TO_CPU == action)
            {
                diag_util_mprintf("Trap-To-Cpu");
            }
            else
            {
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");
        }


        ret = rtk_l2_vlanMode_get(unit, port, &vlanMode);
        if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
        {
            diag_util_mprintf("\tVlan Forward Mode                 : ");
            if (vlanMode == BASED_ON_INNER_VLAN)
            {
                diag_util_mprintf("Base-On-INNER-Vid\n");
            }
            else
            {
                diag_util_mprintf("Base-On-OUTER-Vid\n");
            }
        }

        if (!(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID)))
        {
            ret = rtk_l2_learningEnable_get(unit, port, &enable);
            if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\tSrc Mac Learning                  : %s\n", enable ? "Enable" : "Disable");
            }
        }

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
        {
            ret = rtk_l2_newMacOp_get(unit, port, &lrnMode, &fwdAction);
            if ( (ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                diag_util_mprintf("\tNew Src Mac Operation             : ");
                if (lrnMode == HARDWARE_LEARNING)
                {
                    diag_util_mprintf("Asic-AUTO-Learn\n");
                }
                else if(lrnMode == SOFTWARE_LEARNING)
                {
                    diag_util_mprintf("Learn-As-SUSPEND\n");
                }
                else
                {
                    diag_util_mprintf("Not-Learn\n");
                }

                diag_util_mprintf("\tPacket Action                     : ");
                if (fwdAction == ACTION_FORWARD)
                {
                    diag_util_mprintf("Forward\n");
                }
                else if(fwdAction == ACTION_COPY2CPU)
                {
                    diag_util_mprintf("Copy-To-Cpu\n ");
                }
                else if(fwdAction == ACTION_TRAP2CPU)
                {
                    diag_util_mprintf("Trap-To Cpu\n");
                }
                else
                {
                    diag_util_mprintf("Drop\n");
                }
            }
        }

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_legalMoveToPorts_get(unit, port, &portmask), ret);
            memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
            diag_util_lPortMask2str(port_list, &portmask);
            diag_util_mprintf("\tLegal Move To Ports               : %s\n", port_list);

            diag_util_mprintf("\tLegal Move Action                 : ");
            DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveAction_get(unit, port, &portMove_action), ret);
            if (ACTION_FORWARD == portMove_action)
            {
                diag_util_mprintf("Forward");
            }
            else if (ACTION_DROP == portMove_action)
            {
                diag_util_mprintf("Drop");
            }
            else if (ACTION_TRAP2CPU== portMove_action)
            {
                diag_util_mprintf("Trap-To-Cpu");
            }
            else if (ACTION_COPY2CPU== portMove_action)
            {
                diag_util_mprintf("Copy-To-Cpu");
            }
            else
            {
                return CPARSER_NOT_OK;
            }

            diag_util_mprintf("\n\tIllegal Move Action               : ");
            DIAG_UTIL_ERR_CHK(rtk_l2_illegalPortMoveAction_get(unit, port, &portMove_action), ret);
            if (ACTION_FORWARD == portMove_action)
            {
                diag_util_mprintf("Forward");
            }
            else if (ACTION_DROP == portMove_action)
            {
                diag_util_mprintf("Drop");
            }
            else if (ACTION_TRAP2CPU== portMove_action)
            {
                diag_util_mprintf("Trap-To-Cpu");
            }
            else if (ACTION_COPY2CPU== portMove_action)
            {
                diag_util_mprintf("Copy-To-Cpu");
            }
            else
            {
                return CPARSER_NOT_OK;
            }
        }

        diag_util_mprintf("\n");
    }
    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_MAC_UCAST
/*
  *  l2-table dump mac-ucast
  */
cparser_result_t cparser_cmd_l2_table_dump_mac_ucast(cparser_context_t *context)
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

    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Auth  | Static  | Nexthop  | Suspend\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+-------+---------+----------+--------\n");
    }
    else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
             DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Static  | Nexthop  | Suspend | Aggreate VID(vlanTarget/RouteIdx)\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+---------+----------+---------+-------------\n");
    }
    /* show all l2 table */
    scan_idx = -1; /* get the first entry */
    while (1)
    {
        if ((ret = rtk_l2_nextValidAddr_get(unit, (int32 *)&scan_idx, 1, &l2_data)) != RT_ERR_OK)
        {
            break;
        }

        if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
            diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %5d | %7d | %8d | %7d\n",
            scan_idx,
            l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
            l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
            l2_data.auth, (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
            (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
#endif
        }
        else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
                 DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
            if((l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP))
            {
                diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %4d,%4d\n",
                    scan_idx,
                    l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
                    l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                    (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                    (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_data.vlan_target, l2_data.route_idx);
            }
            else
            {
                diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %4d\n",
                    scan_idx,
                    l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
                    l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                    (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                    (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_data.agg_vid);
            }    
#endif
        }

        total_entry++;
    }
    diag_util_mprintf("\nTotal Number Of Entries :%d\n", total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_MAC_UCAST_DYNAMIC
/*
  *  l2-table dump mac-ucast dynamic
  */
cparser_result_t cparser_cmd_l2_table_dump_mac_ucast_dynamic(cparser_context_t *context)
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

    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Auth  | Static  | Nexthop  | Suspend\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+-------+---------+----------+--------\n");
    }
    else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
             DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Static  | Nexthop  | Suspend | Aggreate VID\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+---------+----------+---------+-------------\n");
    }
    /* show all l2 table */
    scan_idx = -1; /* get the first entry */
    while (1)
    {
        if ((ret = rtk_l2_nextValidAddr_get(unit, (int32 *)&scan_idx, 1, &l2_data)) != RT_ERR_OK)
        {
            break;
        }

        if(l2_data.flags & RTK_L2_UCAST_FLAG_STATIC)
            continue;

        if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
        diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %5d | %7d | %8d | %7d\n",
            scan_idx,
            l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
            l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
            l2_data.auth, (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
            (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
 #endif
        }
        else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
                 DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
            diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %4d\n",
            scan_idx,
            l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
            l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
            (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
            (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_data.agg_vid);
 #endif
        }

        total_entry++;
    }
    diag_util_mprintf("\nTotal Number Of Entries : %d\n", total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_MAC_UCAST_STATIC
/*
  *  l2-table dump mac-ucast static
  */
cparser_result_t cparser_cmd_l2_table_dump_mac_ucast_static(cparser_context_t *context)
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
    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Auth  | Static  | Nexthop  | Suspend\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+-------+---------+----------+--------\n");
    }
    else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
             DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Static  | Nexthop  | Suspend | Aggreate VID\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+---------+----------+---------+-------------\n");
    }
    /* show all l2 table */
    scan_idx = -1; /* get the first entry */
    while (1)
    {
        if ((ret = rtk_l2_nextValidAddr_get(unit, (int32 *)&scan_idx, 1, &l2_data)) != RT_ERR_OK)
        {
            break;
        }

        if(!(l2_data.flags & RTK_L2_UCAST_FLAG_STATIC))
            continue;

        if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
        diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %5d | %7d | %8d | %7d\n",
            scan_idx,
            l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
            l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
            l2_data.auth, (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
            (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
#endif
        }
        else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
                 DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
            diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %4d\n",
					scan_idx,
					l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
					l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
					(l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_data.agg_vid);
#endif
        }

        total_entry++;
    }
    diag_util_mprintf("\nTotal Number Of Entries : %d\n", total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_MAC_UCAST_VID_MAC
/*
  *  l2-table dump mac-ucast <UINT:vid> <MACADDR:mac>
  */
cparser_result_t cparser_cmd_l2_table_dump_mac_ucast_vid_mac(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_l2_ucastAddr_t  l2_data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    memset(&l2_data, 0, sizeof(rtk_l2_ucastAddr_t));

    memcpy(l2_data.mac.octet, mac_ptr, ETHER_ADDR_LEN);
    l2_data.vid = *vid_ptr;
    if ((ret = rtk_l2_addr_get(unit, &l2_data)) != RT_ERR_OK)
    {
        diag_util_printf("Entry is not exist.\n");
        return CPARSER_NOT_OK;
    }

    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    diag_util_mprintf("MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Auth  | Static   | Nexthop  | Suspend\n");
        diag_util_mprintf("------------------+------+------+----------+----------+-------+---------+----------+--------\n");
        diag_util_mprintf("%02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %5d | %7d | %8d | %7d\n",
             l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
             l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
             l2_data.auth, (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
             (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
#endif
    }
    else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
             DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
        diag_util_mprintf("MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Static  | Nexthop  | Suspend | Aggreate VID\n");
        diag_util_mprintf("------------------+------+------+----------+----------+---------+----------+---------+-------------\n");
        diag_util_mprintf("%02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %4d\n",
				 l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
				 l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
				 (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                 (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_data.agg_vid);
#endif
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_MAC_UCAST_PORT_ALL
/*
  *  l2-table dump mac-ucast ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_dump_mac_ucast_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32             scan_idx = 0;
    uint32             total_entry = 0;
    uint32             unit = 0;
    int32              ret = RT_ERR_FAILED;
    rtk_portmask_t          portmask;
    diag_portlist_t         portlist;
    rtk_l2_ucastAddr_t      l2_data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    memset(&l2_data, 0, sizeof(rtk_l2_ucastAddr_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    portmask = portlist.portmask;

    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Auth  | Static  | Nexthop  | Suspend\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+-------+---------+----------+--------\n");
    }
    else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
             DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Static  | Nexthop  | Suspend | Aggreate VID\n");
        diag_util_mprintf("------+-------------------+------+------+----------+----------+---------+----------+---------+-------------\n");
    }

    scan_idx = -1; /* get the first entry */
    while (1)
    {
        if ((ret = rtk_l2_nextValidAddr_get(unit, (int32 *)&scan_idx, 1, &l2_data)) != RT_ERR_OK)
        {
            /* diag_util_mprintf("Warning:%d \n", ret); */
            break;
        }

        if(!RTK_PORTMASK_IS_PORT_SET(portmask, l2_data.port))
            continue;

        if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
        diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %5d | %7d | %8d | %7d\n",
            scan_idx,
            l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
            l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
            l2_data.auth, (l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
            (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0);
#endif
        }
        else if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID) ||
                 DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
            diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %4d\n",
					scan_idx,
					l2_data.mac.octet[0],l2_data.mac.octet[1],l2_data.mac.octet[2],l2_data.mac.octet[3],l2_data.mac.octet[4],l2_data.mac.octet[5],
					l2_data.vid, l2_data.port, (l2_data.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
					(l2_data.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_data.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                (l2_data.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_data.agg_vid);
#endif
        }

        total_entry++;
    }
    diag_util_mprintf("\nTotal Number Of Entries : %d\n", total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_MAC_MCAST
/*
  * l2-table dump mac-mcast
  */
cparser_result_t cparser_cmd_l2_table_dump_mac_mcast(cparser_context_t *context)
{
    uint32              unit = 0;
    uint32              scan_idx = 0;
    uint32              total_entry = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_l2_mcastAddr_t  mcast_data;
    uint8               port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    memset(&mcast_data, 0, sizeof(rtk_l2_mcastAddr_t));

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_mprintf("Index | VID  | MAC address       | Port                 | Cross Vlan\n");
        diag_util_mprintf("------+------+-------------------+----------------------+-----------\n");
    }
    else if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Index | VID  | MAC address       | Port                 | Agg Vid\n");
        diag_util_mprintf("------+------+-------------------+----------------------+-----------\n");
    }
    else
    {
        diag_util_mprintf("Index | VID  | MAC address       | Port                 \n");
        diag_util_mprintf("------+------+-------------------+----------------------\n");
    }

    /* show all l2-macmcast */
    scan_idx = -1; /* get the first entry */
    while (1)
    {
        if ((ret = rtk_l2_nextValidMcastAddr_get(unit, (int32 *)&scan_idx, &mcast_data)) != RT_ERR_OK)
        {            
            break;
        }
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &mcast_data.portmask);
        diag_util_mprintf("%5d | %4d | %02X:%02X:%02X:%02X:%02X:%02X | %20s", scan_idx, mcast_data.rvid,
                mcast_data.mac.octet[0], mcast_data.mac.octet[1], mcast_data.mac.octet[2],
                mcast_data.mac.octet[3], mcast_data.mac.octet[4], mcast_data.mac.octet[5],
                port_list);
        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8328)
            if(mcast_data.crossVlan)
                diag_util_mprintf(" | TRUE\n");
            else
                diag_util_mprintf(" | FALSE\n");
#endif
        }
        else if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8380)
            diag_util_mprintf("| %4d\n", mcast_data.agg_vid);
#endif
        }
        else
            diag_util_mprintf("\n");

        total_entry++;
    }
    diag_util_mprintf("\nTotal Number Of Entries : %d\n",total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_MAC_MCAST_VID_MAC
/*
  * l2-table dump mac-mcast <UINT:vid> <MACADDR:mac>
  */
cparser_result_t cparser_cmd_l2_table_dump_mac_mcast_vid_mac(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_l2_mcastAddr_t  mcast_data;
    uint8               port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    memset(&mcast_data, 0, sizeof(rtk_l2_mcastAddr_t));

    if (devInfo.chipId == RTL8328M_CHIP_ID || devInfo.chipId == RTL8328S_CHIP_ID ||
        devInfo.chipId == RTL8328L_CHIP_ID)
    {
        diag_util_mprintf("VID  | MAC address       | Port                 | Cross Vlan\n");
        diag_util_mprintf("-----+-------------------+----------------------+-----------\n");
    }
    else if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Index | VID  | MAC address       | Port                 | Agg Vid\n");
        diag_util_mprintf("------+------+-------------------+----------------------+-----------\n");
    }
    else
    {
        diag_util_mprintf("VID  | MAC address       | Port                 \n");
        diag_util_mprintf("-----+-------------------+----------------------\n");
    }

    /* show specific l2-ipmcast */
    mcast_data.rvid = *vid_ptr;
    memcpy(mcast_data.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    DIAG_UTIL_ERR_CHK(rtk_l2_mcastAddr_get(unit, &mcast_data), ret);
    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
    diag_util_lPortMask2str(port_list, &mcast_data.portmask);
    diag_util_mprintf("%4d | %02X:%02X:%02X:%02X:%02X:%02X | %20s",
                mcast_data.rvid, mac_ptr->octet[0], mac_ptr->octet[1], mac_ptr->octet[2],
                mac_ptr->octet[3], mac_ptr->octet[4], mac_ptr->octet[5], port_list);

    if (devInfo.chipId == RTL8328M_CHIP_ID || devInfo.chipId == RTL8328S_CHIP_ID ||
        devInfo.chipId == RTL8328L_CHIP_ID)
    {
#if defined(CONFIG_SDK_RTL8328)
        if(mcast_data.crossVlan)
            diag_util_mprintf(" | TRUE\n");
        else
            diag_util_mprintf(" | FALSE\n");
#endif
    }
    else if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
#if defined(CONFIG_SDK_RTL8380)
        diag_util_mprintf("| %4d\n", mcast_data.agg_vid);
#endif
    }
    else
        diag_util_mprintf(" | Not Support\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_IP_MCAST
/*
  * l2-table dump ip-mcast
  */
cparser_result_t cparser_cmd_l2_table_dump_ip_mcast(cparser_context_t *context)
{
    uint32                  unit = 0;
    uint32                  scan_idx = 0;
    uint32                  total_entry = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_l2_ipMcastAddr_t    ip_mcast_data;
    uint8                   port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8                   strBuf1[20], strBuf2[20];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    memset(&ip_mcast_data, 0, sizeof(rtk_l2_ipMcastAddr_t));

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_mprintf("Index | SIP             | DIP             | Port                 | Cross Vlan \n");
        diag_util_mprintf("------+-----------------+-----------------+----------------------+--------------\n");
    }
    else
    {
        diag_util_mprintf("Index | SIP             | DIP             | VID  | Port                 \n");
        diag_util_mprintf("------+-----------------+-----------------+------+----------------------\n");
    }

    /* show all ip-ipmcast */
    scan_idx = -1; /* get the first entry */
    while (1)
    {
        if ((ret = rtk_l2_nextValidIpMcastAddr_get(unit, (int32 *)&scan_idx, &ip_mcast_data)) != RT_ERR_OK)
        {
            break;
        }
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &ip_mcast_data.portmask);
        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            diag_util_ip2str_format(strBuf1, ip_mcast_data.sip, 15);
            diag_util_ip2str_format(strBuf2, ip_mcast_data.dip, 15);
            diag_util_mprintf("%5d | %s | %s | %20s", scan_idx, strBuf1, strBuf2, port_list);
        }
            else
        {
            diag_util_ip2str_format(strBuf1, ip_mcast_data.sip, 15);
            diag_util_ip2str_format(strBuf2, ip_mcast_data.dip, 15);
            diag_util_mprintf("%5d | %s | %s | %4d | %20s", scan_idx, strBuf1, strBuf2, ip_mcast_data.rvid, port_list);
        }

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
#if defined(CONFIG_SDK_RTL8328)
            if(ip_mcast_data.crossVlan)
                diag_util_mprintf(" | TRUE\n");
            else
                diag_util_mprintf(" | FALSE\n");
#endif
        }
        else
            diag_util_mprintf("\n");

        total_entry++;
    }
    diag_util_mprintf("\nTotal Number Of Entries : %d\n",total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_IP_MCAST_SIP_DIP
/*
  * l2-table dump ip-mcast <IPV4ADDR:sip> <IPV4ADDR:dip>
  */
cparser_result_t cparser_cmd_l2_table_dump_ip_mcast_sip_dip(cparser_context_t *context,
    uint32_t *sip_ptr,
    uint32_t *dip_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_l2_ipMcastAddr_t    ip_mcast_data;
    uint8                   port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8                   strBuf1[20], strBuf2[20];
    rtk_switch_devInfo_t devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    memset(&ip_mcast_data, 0, sizeof(rtk_l2_ipMcastAddr_t));

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_mprintf("SIP             | DIP             | Port                 | Cross Vlan\n");
        diag_util_mprintf("----------------+-----------------+----------------------+-----------\n");
    }
    else
    {
        diag_util_mprintf("SIP             | DIP             | VID  | Port                 \n");
        diag_util_mprintf("----------------+-----------------+------+----------------------\n");
    }

    /* show specific ip-ipmcast entry */
    ip_mcast_data.dip = *dip_ptr;
    ip_mcast_data.sip = *sip_ptr;

    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddr_get(unit, &ip_mcast_data), ret);
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &ip_mcast_data.portmask);
    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        diag_util_ip2str_format(strBuf1, ip_mcast_data.sip, 15);
        diag_util_ip2str_format(strBuf2, ip_mcast_data.dip, 15);
        diag_util_mprintf("%s | %s | %20s", strBuf1, strBuf2, port_list);
    }
    else
    {
        diag_util_ip2str_format(strBuf1, ip_mcast_data.sip, 15);
        diag_util_ip2str_format(strBuf2, ip_mcast_data.dip, 15);
        diag_util_mprintf("%s | %s | %4d | %20s", strBuf1, strBuf2, ip_mcast_data.rvid, port_list);
    }

    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
#if defined(CONFIG_SDK_RTL8328)
        if(ip_mcast_data.crossVlan)
            diag_util_mprintf(" | TRUE\n");
        else
            diag_util_mprintf(" | FALSE\n");
#endif
    }
    else
        diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_FWD_TABLE
/*
  * l2-table dump fwd-table
  */
cparser_result_t cparser_cmd_l2_table_dump_fwd_table(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      minIndex = 0;
    uint32      totalEntries = 0;
    int32       index;
    rtk_portmask_t portmask;
    uint8       portStr[80];
    uint32      crossVlan;
    uint32      freeCount;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_OM_GET_CHIP_CAPACITY(unit, totalEntries, max_num_of_mcast_entry);

    DIAG_UTIL_ERR_CHK(rtk_l2_mcastFwdIndexFreeCount_get(unit, &freeCount), ret);
    diag_util_mprintf("Free Count : %4d\n", freeCount);

    for(index = minIndex; index < totalEntries; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_mcastFwdPortmask_get(unit, index, &portmask, &crossVlan), ret);

        diag_util_mprintf("Index : %4u, ", index);

        diag_util_lPortMask2str(portStr, &portmask);

        diag_util_mprintf("Port List : %10s  ", portStr);

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            if(crossVlan)
                diag_util_mprintf("Cross Vlan : TRUE\n");
            else
                diag_util_mprintf("Cross Vlan : FALSE\n");
        }
        else
            diag_util_mprintf("\n");
    }


    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_FWD_TABLE_LOW_IDX_HIGH_IDX
/*
  * l2-table dump fwd-table <UINT:low_idx> <UINT:high_idx>
  */
cparser_result_t cparser_cmd_l2_table_dump_fwd_table_low_idx_high_idx(cparser_context_t *context,
    uint32_t *low_idx_ptr,
    uint32_t *high_idx_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      minIndex;
    uint32      maxIndex;
    int32       index;
    rtk_portmask_t portmask;
    uint8       portStr[80];
    uint32      crossVlan;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == low_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == high_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*high_idx_ptr > FWD_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*high_idx_ptr > FWD_TABLE_MAX_IDX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*low_idx_ptr > *high_idx_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    minIndex = *low_idx_ptr;
    maxIndex = *high_idx_ptr;

    for(index = minIndex; index <= maxIndex; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_mcastFwdPortmask_get(unit, index, &portmask, &crossVlan), ret);

        diag_util_mprintf("Index : %4u, ", index);

        diag_util_lPortMask2str(portStr, &portmask);

        diag_util_mprintf("Port List : %10s  ", portStr);

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            if(crossVlan)
                diag_util_mprintf("Cross Vlan : TRUE\n");
            else
                diag_util_mprintf("Cross Vlan : FALSE\n");
        }
        else
            diag_util_mprintf("\n");
    }


    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_UCAST_VID_MAC_PORT_PORT_AUTH_SA_BLOCK_DA_BLOCK_STATIC_NEXTHOP_SUSPEND
/*
  * l2-table add mac-ucast <UINT:vid> <MACADDR:mac> port <UINT:port> auth { sa-block  } { da-block  } { static } { nexthop } { suspend }
  */
cparser_result_t cparser_cmd_l2_table_add_mac_ucast_vid_mac_port_port_auth_sa_block_da_block_static_nexthop_suspend(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    uint32_t *port_ptr)
{
    uint32          flag_num = 0;
    uint32          sa_block = FALSE;
    uint32          da_block = FALSE;
    uint32          is_static = FALSE;
    uint32          is_auth = FALSE;
    uint32          nexthop = FALSE;
    uint32          suspend = FALSE;
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ucastAddr_t  l2_uAddr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == port_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l2_uAddr, 0, sizeof(rtk_l2_ucastAddr_t));

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    /*from first optional token*/
    for(flag_num = 7; flag_num < TOKEN_NUM; flag_num++)
    {
        if ('s' == TOKEN_CHAR(flag_num, 0))
        {
            if ('a' == TOKEN_CHAR(flag_num, 1))
            {
                sa_block = TRUE;
            }
            else if ('t' == TOKEN_CHAR(flag_num, 1))
            {
                is_static = TRUE;
            }
            else if ('u' == TOKEN_CHAR(flag_num, 1))
            {
                suspend = TRUE;
            }
        }
        else if ('d' == TOKEN_CHAR(flag_num, 0))
        {
            da_block = TRUE;
        }
        else if ('a' == TOKEN_CHAR(flag_num, 0))
        {
            is_auth = TRUE;
        }
        else if ('n' == TOKEN_CHAR(flag_num, 0))
        {
            nexthop = TRUE;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    /* Fill structure */
    l2_uAddr.vid = *vid_ptr;
    memcpy(l2_uAddr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2_uAddr.port = *port_ptr;
    if(sa_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_SA_BLOCK;

    if(da_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_DA_BLOCK;

    if(is_static)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_STATIC;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_STATIC;

    if(nexthop)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_NEXTHOP;

    if(suspend)
        l2_uAddr.state |= RTK_L2_UCAST_STATE_SUSPEND;
    else
        l2_uAddr.state&= ~RTK_L2_UCAST_STATE_SUSPEND;

    if(is_auth)
        l2_uAddr.auth = TRUE;
    else
        l2_uAddr.auth = FALSE;

    DIAG_UTIL_ERR_CHK(rtk_l2_addr_add(unit, &l2_uAddr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_UCAST_VID_MAC_TRUNK_TRUNK_ID_AUTH_SA_BLOCK_DA_BLOCK_STATIC_NEXTHOP_SUSPEND
/*
 * l2-table add mac-ucast <UINT:vid> <MACADDR:mac> trunk <UINT:trunk-id> { auth } { sa-block  } { da-block  } { static } { nexthop } { suspend }
 */
cparser_result_t cparser_cmd_l2_table_add_mac_ucast_vid_mac_trunk_trunk_id_auth_sa_block_da_block_static_nexthop_suspend(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    uint32_t *trunk_id_ptr)
{
    int32           flag_num = 0;
    uint32          sa_block = FALSE;
    uint32          da_block = FALSE;
    uint32          is_static = FALSE;
    uint32          is_auth = FALSE;
    uint32          nexthop = FALSE;
    uint32          suspend = FALSE;
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ucastAddr_t  l2_uAddr;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == trunk_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l2_uAddr, 0, sizeof(rtk_l2_ucastAddr_t));

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    /*from first optional token*/
    for(flag_num = 7; flag_num < TOKEN_NUM; flag_num++)
    {
        if ('s' == TOKEN_CHAR(flag_num, 0))
        {
            if ('a' == TOKEN_CHAR(flag_num, 1))
            {
                sa_block = TRUE;
            }
            else if ('t' == TOKEN_CHAR(flag_num, 1))
            {
                is_static = TRUE;
            }
            else if ('u' == TOKEN_CHAR(flag_num, 1))
            {
                suspend = TRUE;
            }
        }
        else if ('d' == TOKEN_CHAR(flag_num, 0))
        {
            da_block = TRUE;
        }
        else if ('a' == TOKEN_CHAR(flag_num, 0))
        {
            is_auth = TRUE;
        }
        else if ('n' == TOKEN_CHAR(flag_num, 0))
        {
            nexthop = TRUE;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    /* Fill structure */
    l2_uAddr.vid = *vid_ptr;
    memcpy(l2_uAddr.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    //l2_uAddr.port = *port_ptr;
    l2_uAddr.trk_gid = *trunk_id_ptr;
    l2_uAddr.flags |= RTK_L2_UCAST_FLAG_TRUNK_PORT;
    if(sa_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_SA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_SA_BLOCK;

    if(da_block)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_DA_BLOCK;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_DA_BLOCK;

    if(is_static)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_STATIC;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_STATIC;

    if(nexthop)
        l2_uAddr.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    else
        l2_uAddr.flags &= ~RTK_L2_UCAST_FLAG_NEXTHOP;

    if(suspend)
        l2_uAddr.state |= RTK_L2_UCAST_STATE_SUSPEND;
    else
        l2_uAddr.state&= ~RTK_L2_UCAST_STATE_SUSPEND;

#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328)
    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        if(is_auth)
            l2_uAddr.auth = TRUE;
        else
            l2_uAddr.auth = FALSE;
    }
#endif

    DIAG_UTIL_ERR_CHK(rtk_l2_addr_add(unit, &l2_uAddr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_AGING_OUT_PORT_ALL
/*
  * l2-table get aging-out ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_get_aging_out_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t        enable = DISABLED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portAgingEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("\tPort %d : %s\n", port, enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_DYNAMIC_PORT_MOVE_FLUSH_PORT_ALL_STATE
/*
  * l2-table get dynamic-port-move-flush ( <PORT_LIST:port> | all ) state
  */
cparser_result_t cparser_cmd_l2_table_get_dynamic_port_move_flush_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t        enable = DISABLED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveFlushAddrEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("\tPort %d : %s\n", port, enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_GET_DYNAMIC_PORT_MOVE_FORBID_PORT_ALL_STATE
/*
  * l2-table get dynamic-port-move-forbid ( <PORT_LIST:port> | all ) state
  */
cparser_result_t cparser_cmd_l2_table_get_dynamic_port_move_forbid_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t        enable = DISABLED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portDynamicPortMoveForbidEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("\tPort %d : %s\n", port, enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_AGING_OUT_PORT_ALL_DISABLE_ENABLE
/*
  * l2-table set aging-out ( <PORT_LIST:port> | all ) ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_aging_out_port_all_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_portAgingEnable_set(unit, port, ENABLED), ret);
        }
    }
    else if('d' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_portAgingEnable_set(unit, port, DISABLED), ret);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_DYNAMIC_PORT_MOVE_PORT_ALL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 *   l2-table set dynamic-port-move ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_dynamic_port_move_port_all_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_action_t        action;
    diag_portlist_t     portlist;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    }
    else if('d' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    }
    else if ('c' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveAction_set(unit, port, action), ret);
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_DYNAMIC_PORT_MOVE_FLUSH_PORT_ALL_STATE_DISABLE_ENABLE
/*
  * l2-table set dynamic-port-move-flush ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_l2_table_set_dynamic_port_move_flush_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveFlushAddrEnable_set(unit, port, ENABLED), ret);
        }
    }
    else if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_l2_legalPortMoveFlushAddrEnable_set(unit, port, DISABLED), ret);
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_DYNAMIC_PORT_MOVE_FORBID_PORT_ALL_STATE_DISABLE_ENABLE
/*
  * l2-table set dynamic-port-move-forbid ( <PORT_LIST:port> | all ) state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_dynamic_port_move_forbid_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
            DIAG_UTIL_ERR_CHK(rtk_l2_portDynamicPortMoveForbidEnable_set(unit, port, ENABLED), ret);
        }
    }
    else if('d' == TOKEN_CHAR(5, 0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
            DIAG_UTIL_ERR_CHK(rtk_l2_portDynamicPortMoveForbidEnable_set(unit, port, DISABLED), ret);
        }
    }

    return CPARSER_OK;
    }
#endif

#ifdef CMD_L2_TABLE_SET_CAM_STATE_DISABLE_ENABLE
/*
  *  l2-table set cam state ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_cam_state_disable_enable(cparser_context_t *context)
    {
    uint32                          unit = 0;
    int32                            ret = RT_ERR_FAILED;
    rtk_enable_t                 enable = FALSE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(4, 0))
        enable = TRUE;
    else if('d' == TOKEN_CHAR(4, 0))
        enable = FALSE;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_camEnable_set(unit, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_EXCEPT_SMAC_BCAST_MCAST_SA_DROP_FORWARD
/*
 *  l2-table set except-smac bcast-mcast-sa ( drop | forward )
 */
cparser_result_t cparser_cmd_l2_table_set_except_smac_bcast_mcast_sa_drop_forward(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t type = DLF_TYPE_UCAST;
    rtk_action_t    action  = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    type = SA_IS_BCAST_OR_MCAST;

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_DROP;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_exceptionAddrAction_set(unit, type, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_EXCEPT_SMAC_ZERO_SA_LEARNING_DISABLE_ENABLE
/*
  *  l2-table set except-smac zero-sa-learning  ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_except_smac_zero_sa_learning_disable_enable(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_zeroSALearningEnable_set(unit, ENABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_zeroSALearningEnable_set(unit, DISABLED), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IP6_DIP_CARE_BYTE_CARE_BYTE
/*
  * l2-table set ip6-dip-care-byte <HEX:care_byte>
  */
cparser_result_t cparser_cmd_l2_table_set_ip6_dip_care_byte_care_byte(cparser_context_t *context,
    uint32_t *pCareByte)
{
    int32   ret = RT_ERR_FAILED;
    uint32  unit = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_ip6CareByte_set(unit, L2_DIP_HASH_CARE_BYTE, *pCareByte), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IP6_SIP_CARE_BYTE_CARE_BYTE
/*
  * l2-table set ip6-sip-care-byte <HEX:care_byte>
  */
cparser_result_t cparser_cmd_l2_table_set_ip6_sip_care_byte_care_byte(cparser_context_t *context,
    uint32_t *pCareByte)
{
    int32   ret = RT_ERR_FAILED;
    uint32  unit = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_ip6CareByte_set(unit, L2_SIP_HASH_CARE_BYTE, *pCareByte), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IP6MC_MODE_DIP_AND_SIP_DIP_AND_VID_VID_AND_MAC
/*
  * l2-table set ip6mc-mode ( dip-and-sip | dip-and-vid | vid-and-mac)
  */
cparser_result_t cparser_cmd_l2_table_set_ip6mc_mode_dip_and_sip_dip_and_vid_vid_and_mac(cparser_context_t *context)
{
    int32   ret = RT_ERR_FAILED;
    uint32  unit = 0;
    uint32  mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('s' == TOKEN_CHAR(3, 8))
        mode = LOOKUP_ON_DIP_AND_SIP;
    else if('v' == TOKEN_CHAR(3, 8))
        mode = LOOKUP_ON_DIP_AND_FVID;
    else if('m' == TOKEN_CHAR(3, 8))
        mode = LOOKUP_ON_FVID_AND_MAC;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_ip6mcMode_set(unit, mode), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_IPMC_MODE_DIP_AND_SIP_DIP_AND_VID_VID_AND_MAC
/*
  * l2-table set ipmc-mode ( dip-and-sip | dip-and-vid | vid-and-mac)
  */
cparser_result_t cparser_cmd_l2_table_set_ipmc_mode_dip_and_sip_dip_and_vid_vid_and_mac(cparser_context_t *context)
{
    int32   ret = RT_ERR_FAILED;
    uint32  unit = 0;
    uint32  mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('s' == TOKEN_CHAR(3, 8))
        mode = LOOKUP_ON_DIP_AND_SIP;
    else if('v' == TOKEN_CHAR(3, 8))
        mode = LOOKUP_ON_DIP_AND_FVID;
    else if('m' == TOKEN_CHAR(3, 8))
        mode = LOOKUP_ON_FVID_AND_MAC;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_ipmcMode_set(unit, mode), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_ACTION_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
  * l2-table set limit-learning action ( copy-to-cpu | drop | forward | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_action_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_limitLearnCntAction_t    action = LIMIT_LEARN_CNT_ACTION_FORWARD;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    }
    else if ('c' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningCntAction_set(unit, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_MAX_COUNT
/*
  * l2-table set limit-learning <UINT:max_count>
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_max_count(cparser_context_t *context,
    uint32_t *max_count_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == max_count_ptr), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_l2_limitLearningCnt_set(unit, *max_count_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_PORT_PORT_ALL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
  * l2-table set limit-learning port ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_port_port_all_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_l2_limitLearnCntAction_t    action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    diag_portlist_t             portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if ('f' == TOKEN_CHAR(5, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(5, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    }
    else if ('c' == TOKEN_CHAR(5, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portLimitLearningCntAction_set(unit, port, action), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_TBL_INDEX_INDEX_VID_MAX_COUNT_PORT_ID
/*
  * l2-table set limit-learning tbl-index <UINT:index> <UINT:vid> <UINT:max_count> { <UINT:port_id> }
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_tbl_index_index_vid_max_count_port_id(cparser_context_t *context,
    uint32_t *idx_ptr, uint32_t *vid_ptr, uint32_t *max_count_ptr, uint32_t *port_id_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_l2_fidMacLimitEntry_t       entry;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if (7 == TOKEN_NUM)
    {
        port = 0x3f;
    }
    else
    {
        port = *port_id_ptr;
    }
    entry.fid = *vid_ptr;
    entry.maxNum = *max_count_ptr;
    entry.port = port;

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_set(unit, *idx_ptr, &entry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_VLAN_BASED_INDEX_VID_MAX_COUNT_PORT_ID
/*
  * l2-table set limit-learning vlan-based <UINT:index> <UINT:vid> <UINT:max_count> { <UINT:port_id> }
  */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_vlan_based_index_vid_max_count_port_id(cparser_context_t *context,
    uint32_t *idx_ptr, uint32_t *vid_ptr, uint32_t *max_count_ptr, uint32_t *port_id_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_port_t                      port = 0;
    rtk_l2_fidMacLimitEntry_t       entry;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if (7 == TOKEN_NUM)
    {
        if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
            port = 0x3f;
        else
            port = 0x1f;
    }
    else
    {
        port = *port_id_ptr;
    }
    entry.fid = *vid_ptr;
    entry.maxNum = *max_count_ptr;
    entry.port = port;

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLimitLearningEntry_set(unit, *idx_ptr, &entry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LIMIT_LEARNING_VLAN_BASED_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 * l2-table set limit-learning vlan-based ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_l2_table_set_limit_learning_vlan_based_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_limitLearnCntAction_t    action = LIMIT_LEARN_CNT_ACTION_FORWARD;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    }
    else if ('c' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_fidLearningCntAction_set(unit, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_NOTIFICATION_DISABLE_ENABLE
/*
  *  l2-table set notification ( disable | enable )
  */
cparser_result_t cparser_cmd_l2_table_set_notification_disable_enable(cparser_context_t *context)
{
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
    if('e' == TOKEN_CHAR(3, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_notificationEnable_set(unit, ENABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_notificationEnable_set(unit, DISABLED), ret);
    }
    }
    else
    {
        DIAG_ERR_PRINT(RT_ERR_CHIP_NOT_SUPPORTED);
        return CPARSER_NOT_OK;
    }
#else
    diag_util_mprintf("L2 notification is not support\n");
#endif

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_NOTIFICATION_BACK_PRESSURE_THRESHOLD_THRESHOLD
/*
  *  l2-table set notification back-pressure threshold <UINT:threshold>
  */
cparser_result_t cparser_cmd_l2_table_set_notification_back_pressure_threshold_threshold(cparser_context_t *context, uint32_t *threshold_ptr)
{
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    DIAG_UTIL_ERR_CHK(rtk_l2_notificationBackPressureThresh_set(unit, *threshold_ptr), ret);
    else
    {
        DIAG_ERR_PRINT(RT_ERR_CHIP_NOT_SUPPORTED);
        return CPARSER_NOT_OK;
    }
#else
    diag_util_mprintf("L2 notification is not support\n");
#endif

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_NOTIFICATION_TYPE_LINK_DOWN_FLUSH_SUSPEND_STATE_DISABLE_ENABLE
/*
 * l2-table set notification type ( link-down-flush | suspend ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_l2_table_set_notification_type_link_down_flush_suspend_state_disable_enable(
    cparser_context_t *context)
{
#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
    if('e' == TOKEN_CHAR(6, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_notificationEventEnable_set(unit, L2_NOTIFY_EVENT_SUSPEND, ENABLED), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_notificationEventEnable_set(unit, L2_NOTIFY_EVENT_SUSPEND, DISABLED), ret);
    }
    }
    else
    {
        DIAG_ERR_PRINT(RT_ERR_CHIP_NOT_SUPPORTED);
        return CPARSER_NOT_OK;
    }
#else
    diag_util_mprintf("L2 notification is not support\n");
#endif

    return CPARSER_OK;
}   /* end of cparser_cmd_l2_table_set_notification_type_suspend_state_disable_enable */
#endif
#ifdef CMD_L2_TABLE_DUMP_IP6_MCAST
/*
  * l2-table dump ip6-mcast
  */
cparser_result_t cparser_cmd_l2_table_dump_ip6_mcast(cparser_context_t *context)
{
    uint32                  unit = 0;
    uint32                  scan_idx = 0;
    uint32                  total_entry = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_l2_ip6McastAddr_t   ip6_mcast_data;
    uint8                   port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8                   ipv6DipStr[32], ipv6SipStr[32];
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    memset(&ip6_mcast_data, 0, sizeof(rtk_l2_ip6McastAddr_t));

    diag_util_mprintf("Index | SIP                  | DIP                  | VID  | Port                 \n");
    diag_util_mprintf("------+----------------------+----------------------+------+-----------------------\n");

    /* show all ip-ipmcast */
    scan_idx = -1; /* get the first entry */
    while (1)
    {
        if ((ret = rtk_l2_nextValidIp6McastAddr_get(unit, (int32 *)&scan_idx, &ip6_mcast_data)) != RT_ERR_OK)
        {
            /* diag_util_mprintf("Warning:%d \n", ret); */
            break;
        }
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &ip6_mcast_data.portmask);
        DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6SipStr, (uint8 *)&ip6_mcast_data.sip.ipv6_addr[0]), ret);
        DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6DipStr, (uint8 *)&ip6_mcast_data.dip.ipv6_addr[0]), ret);
        diag_util_mprintf("%5d | %20s | %20s | %4d | %20s \n", scan_idx, ipv6SipStr, ipv6DipStr, ip6_mcast_data.rvid, port_list);

        total_entry++;
    }
    diag_util_mprintf("\nTotal Number Of Entries : %d\n",total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_IP6_MCAST_SIP_DIP_VID
/*
  * l2-table dump ip6-mcast <IPV6ADDR:sip> <IPV6ADDR:dip> <UINT:vid>
  */
cparser_result_t cparser_cmd_l2_table_dump_ip6_mcast_sip_dip_vid(cparser_context_t *context,
    char **sip_ptr,
    char **dip_ptr,
    uint32_t *vid_ptr)
{
    uint8                   port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8                   ipv6DipStr[32], ipv6SipStr[32];
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_l2_ip6McastAddr_t   ip6_mcast_data;
    rtk_switch_devInfo_t    devInfo;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    memset(&ip6_mcast_data, 0, sizeof(rtk_l2_ip6McastAddr_t));

    diag_util_mprintf("SIP             | DIP             | VID  | Port     \n");
    diag_util_mprintf("-----------------+-----------------+------+-----------------------\n");

    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.sip.ipv6_addr[0], (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.dip.ipv6_addr[0], (uint8 *)TOKEN_STR(4)), ret);

    /* show specific ip-ipmcast entry */
    ip6_mcast_data.rvid = *vid_ptr;

    rtk_l2_ipMcstFidVidCompareEnable_get(unit, &enable);
    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcstFidVidCompareEnable_set(unit, ENABLED), ret);
    DIAG_UTIL_ERR_CHK(rtk_l2_ip6McastAddr_get(unit, &ip6_mcast_data), ret);
    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcstFidVidCompareEnable_set(unit, enable), ret);     /* Restore original setting */

    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
    diag_util_lPortMask2str(port_list, &ip6_mcast_data.portmask);
    DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6SipStr, (uint8 *)&ip6_mcast_data.sip.ipv6_addr[0]), ret);
    DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6DipStr, (uint8 *)&ip6_mcast_data.dip.ipv6_addr[0]), ret);
    diag_util_mprintf("%s | %s | %4d | %20s\n", ipv6SipStr, ipv6DipStr, ip6_mcast_data.rvid, port_list);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_BCAST_UNICAST_FWD_TBL_IDX_INDEX
/*
  * l2-table set lookup-miss ( bcast | unicast ) fwd-tbl-idx <UINT:index>
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_bcast_unicast_fwd_tbl_idx_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t     type = DLF_TYPE_UCAST;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if ('u' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_UCAST;
    }
    else if ('b' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_BCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissFloodPortMaskIdx_set(unit, type, *index_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_BCAST_UNICAST_FLOOD_PORT_ALL
/*
  * l2-table set lookup-miss ( bcast | unicast ) ( <PORT_LIST:flood_port> | all )
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_bcast_unicast_flood_port_all(cparser_context_t *context,
    char **flood_port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t     type = DLF_TYPE_UCAST;
    rtk_portmask_t              portmask;
    diag_portlist_t             portlist;
	uint32						index;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();

    if ('u' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_UCAST;
    }
    else if ('b' == TOKEN_CHAR(3, 0))
    {
        type = DLF_TYPE_BCAST;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    portmask = portlist.portmask;

	DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissFloodPortMaskIdx_get(unit, type, &index), ret);
	DIAG_UTIL_ERR_CHK(rtk_l2_lookupMissFloodPortMask_set_with_idx(unit, type, index, &portmask), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_LOOKUP_MISS_PORT_ALL_L2MC_IPMC_IP6MC_UNICAST_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
  * l2-table set lookup-miss ( <PORT_LIST:port> | all ) ( l2mc | ipmc | ip6mc | unicast )  ( copy-to-cpu | drop | forward | trap-to-cpu )
  */
cparser_result_t cparser_cmd_l2_table_set_lookup_miss_port_all_l2mc_ipmc_ip6mc_unicast_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    int32                           ret = RT_ERR_FAILED;
    uint32                          unit = 0;
    rtk_port_t                      port = 0;
    rtk_l2_lookupMissType_t         type = DLF_TYPE_UCAST;
    rtk_action_t                    action  = ACTION_FORWARD;
    diag_portlist_t                 portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('u' == TOKEN_CHAR(4, 0))
    {
        type = DLF_TYPE_UCAST;
    }
    else if ('l' == TOKEN_CHAR(4, 0))
    {
        type = DLF_TYPE_MCAST;
    }
    else if ('i' == TOKEN_CHAR(4, 0))
    {
        if ('m' == TOKEN_CHAR(4, 2))
            type = DLF_TYPE_IPMC;
        else
            type = DLF_TYPE_IP6MC;

    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('d' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('c' == TOKEN_CHAR(5, 0))
    {
        action = ACTION_COPY2CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portLookupMissAction_set(unit, port, type, action), ret);
    }


    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_REPLACE_PORT_PORT_ID_REPLACING_PORT_ID
/*
  * l2-table set replace port <UINT:port_id> <UINT:replacing_port_id>
  */
cparser_result_t cparser_cmd_l2_table_set_replace_port_port_id_replacing_port_id(cparser_context_t *context,
uint32_t *port_id_ptr, uint32_t *replacing_port_id)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));
    config.act = 1;
    config.flushByPort = 1;
    config.port = *port_id_ptr;
    config.replacingPort = *replacing_port_id;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_REPLACE_PORT_PORT_ID_REPLACING_PORT_ID_INCLUDE_STATIC
/*
 * l2-table set replace port <UINT:port_id> <UINT:replacing_port_id> include-static
 */
cparser_result_t cparser_cmd_l2_table_set_replace_port_port_id_replacing_port_id_include_static(cparser_context_t *context,
    uint32_t *port_id_ptr,
    uint32_t *replacing_port_id_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));
    config.act = 1;
    config.flushByPort = 1;
    config.flushByVid = 0;
    config.vid = 0;
    config.port = *port_id_ptr;
    config.replacingPort = *replacing_port_id_ptr;
    config.flushStaticAddr = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_REPLACE_PORT_PORT_ID_REPLACING_PORT_ID_VID_VLAN_ID_INCLUDE_STATIC
/*
  * l2-table set replace port <UINT:port_id> <UINT:replacing_port_id> vid <UINT:vlan_id> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_replace_port_port_id_replacing_port_id_vid_vlan_id_include_static(cparser_context_t *context,
uint32_t *port_id_ptr, uint32_t *replacing_port_id, uint32_t *vid_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));
    config.act = 1;
    config.flushByPort = 1;
    config.flushByVid = 1;
    config.vid = *vid_ptr;
    config.port = *port_id_ptr;
    config.replacingPort = *replacing_port_id;
    if (9 == TOKEN_NUM)
        config.flushStaticAddr = TRUE;
    else
        config.flushStaticAddr = FALSE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_REPLACE_VID_VLAN_ID_REPLACING_PORT_ID_INCLUDE_STATIC
/*
  * l2-table set replace vid <UINT:vlan_id> <UINT:replacing_port_id> { include-static }
  */
cparser_result_t cparser_cmd_l2_table_set_replace_vid_vlan_id_replacing_port_id_include_static(cparser_context_t *context,
    uint32_t *vlan_id_ptr,
    uint32_t *replacing_port_id_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_l2_flushCfg_t   config;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&config, 0, sizeof(rtk_l2_flushCfg_t));
    config.act = 1;
    config.flushByVid = 1;
    config.vid = *vlan_id_ptr;
    config.replacingPort = *replacing_port_id_ptr;

    if (7 == TOKEN_NUM)
        config.flushStaticAddr = TRUE;
    else
        config.flushStaticAddr = FALSE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(unit, &config), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_MCAST_VID_MAC_PORT_ALL
/*
  * l2-table add mac-mcast <UINT:vid> <MACADDR:mac> ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_add_mac_mcast_vid_mac_port_all(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_mcastAddr_t  mcast_data;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);

    memset(&mcast_data, 0, sizeof(rtk_l2_mcastAddr_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    mcast_data.portmask = portlist.portmask;

    mcast_data.rvid = *vid_ptr;
    memcpy(mcast_data.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

#if defined(CONFIG_SDK_RTL8380)
    mcast_data.agg_vid = 0;
#endif
#if defined(CONFIG_SDK_RTL8328)
    mcast_data.crossVlan = FALSE;
#endif

    DIAG_UTIL_ERR_CHK(rtk_l2_mcastAddr_add(unit, &mcast_data), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_MCAST_VID_MAC_PORT_ALL_CROSS_VLAN
/*
 * l2-table add mac-mcast <UINT:vid> <MACADDR:mac> ( <PORT_LIST:port> | all )  cross-vlan
 */
cparser_result_t cparser_cmd_l2_table_add_mac_mcast_vid_mac_port_all_cross_vlan(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_mcastAddr_t  mcast_data;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);

    memset(&mcast_data, 0, sizeof(rtk_l2_mcastAddr_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
    mcast_data.portmask = portlist.portmask;

    mcast_data.rvid = *vid_ptr;
    memcpy(mcast_data.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

    mcast_data.crossVlan = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_mcastAddr_add(unit, &mcast_data), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_IP_MCAST_SIP_DIP_VLAN_ID_PORT_ALL
/*
  * l2-table add ip-mcast <IPV4ADDR:sip> <IPV4ADDR:dip> <UINT:vlan_id> ( <PORT_LIST:port> | all )
  */
cparser_result_t cparser_cmd_l2_table_add_ip_mcast_sip_dip_vlan_id_port_all(cparser_context_t *context,
    uint32_t *sip_ptr,
    uint32_t *dip_ptr,
    uint32_t *vlan_id_ptr,
    char **port_ptr)
{
    uint32          unit = 0, mode;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ipMcastAddr_t    ip_mcast_data;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);

    memset(&ip_mcast_data, 0, sizeof(rtk_l2_ipMcastAddr_t));

    ip_mcast_data.rvid = *vlan_id_ptr;
    ip_mcast_data.dip = *dip_ptr;
    ip_mcast_data.sip = *sip_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    ip_mcast_data.portmask = portlist.portmask;

    ret = rtk_l2_ipMcastAddr_add(unit, &ip_mcast_data);
    if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
    {
        rtk_l2_ipmcMode_get(unit, &mode);
        if (mode == LOOKUP_ON_FVID_AND_MAC)
        {
            diag_util_printf("Curent hash mode is VID+MAC. Not support ip-mcast addition in this mode.\n");
            return CPARSER_OK;
        }
    }
    else
        DIAG_UTIL_ERR_CHK(ret, ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_IP_MCAST_SIP_DIP_VID_VLAN_ID_PORT_ALL_CROSS_VLAN
/*
  * l2-table add ip-mcast <IPV4ADDR:sip> <IPV4ADDR:dip> vid <UINT:vlan_id> ( <PORT_LIST:port> | all ) { cross-vlan }
  */
cparser_result_t cparser_cmd_l2_table_add_ip_mcast_sip_dip_vid_vlan_id_port_all_cross_vlan(cparser_context_t *context,
    uint32_t *sip_ptr,
    uint32_t *dip_ptr,
    uint32_t *vlan_id_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ipMcastAddr_t    ip_mcast_data;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);

    memset(&ip_mcast_data, 0, sizeof(rtk_l2_ipMcastAddr_t));

    ip_mcast_data.rvid = *vlan_id_ptr;
    ip_mcast_data.dip = *dip_ptr;
    ip_mcast_data.sip = *sip_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 7), ret);
    ip_mcast_data.portmask = portlist.portmask;

    if(8 == TOKEN_NUM)
        ip_mcast_data.crossVlan = FALSE;
    else
        ip_mcast_data.crossVlan = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddr_add(unit, &ip_mcast_data), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_STTC_PORT_MOVE_PORT_ALL_COPY_TO_CPU_DROP_FORWARD_TRAP_TO_CPU
/*
 *   l2-table set sttc-port-move ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_l2_table_set_sttc_port_move_port_all_copy_to_cpu_drop_forward_trap_to_cpu(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_action_t        action;
    diag_portlist_t     portlist;


    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    }
    else if ('c' == TOKEN_CHAR(4, 0))
    {
        action = LIMIT_LEARN_CNT_ACTION_COPY_TO_CPU;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_staticPortMoveAction_set(unit, port, action), ret);
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_IP_MCAST_SIP_DIP_PORT_PORT_ALL_CROSS_VLAN
/*
  * l2-table add ip-mcast <IPV4ADDR:sip> <IPV4ADDR:dip> port ( <PORT_LIST:port> | all ) { cross-vlan }
  */
cparser_result_t cparser_cmd_l2_table_add_ip_mcast_sip_dip_port_port_all_cross_vlan(cparser_context_t *context,
    uint32_t *sip_ptr,
    uint32_t *dip_ptr,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_l2_ipMcastAddr_t    ip_mcast_data;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);

    memset(&ip_mcast_data, 0, sizeof(rtk_l2_ipMcastAddr_t));

    ip_mcast_data.dip = *dip_ptr;
    ip_mcast_data.sip = *sip_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    ip_mcast_data.portmask = portlist.portmask;

    if(7 == TOKEN_NUM)
        ip_mcast_data.crossVlan = FALSE;
    else
        ip_mcast_data.crossVlan = TRUE;

    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddr_add(unit, &ip_mcast_data), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_DEL_CPU_MAC_VID_MAC
/*
  * l2-table ( add | del ) cpu-mac <UINT:vid> <MACADDR:mac>
  */
cparser_result_t cparser_cmd_l2_table_add_del_cpu_mac_vid_mac(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    if ('a' == TOKEN_CHAR(1, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_cpuMacAddr_add(unit, *vid_ptr, (rtk_mac_t *)mac_ptr), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_cpuMacAddr_del(unit, *vid_ptr, (rtk_mac_t *)mac_ptr), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DEL_MAC_UCAST_VID_MAC
/*
  * l2-table del mac-ucast <UINT:vid> <MACADDR:mac>
  */
cparser_result_t cparser_cmd_l2_table_del_mac_ucast_vid_mac(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    /* l2 table - del mac address */
    DIAG_UTIL_ERR_CHK(rtk_l2_addr_del(unit, *vid_ptr, (rtk_mac_t *)mac_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DEL_ALL_INCLUDE_STATIC
/*
  * l2-table del all { include-static }
  */
cparser_result_t cparser_cmd_l2_table_del_all_include_static(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  include_static = TRUE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if(4 == TOKEN_NUM)
        include_static = TRUE;
    else
        include_static = FALSE;

    /* del l2 table - all */
    DIAG_UTIL_ERR_CHK(rtk_l2_addr_delAll(unit, include_static), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DEL_MAC_MCAST_VID_MAC
/*
  * l2-table del mac-mcast <UINT:vid> <MACADDR:mac>
  */
cparser_result_t cparser_cmd_l2_table_del_mac_mcast_vid_mac(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    if (TRUE != diag_util_isMcastMacAddr(mac_ptr->octet))
    {
        diag_util_printf("Broadcast and Unicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_mcastAddr_del(unit, *vid_ptr, (rtk_mac_t *)mac_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DEL_IP_MCAST_SIP_DIP
/*
  * l2-table del ip-mcast <IPV4ADDR:sip> <IPV4ADDR:dip>
  */
cparser_result_t cparser_cmd_l2_table_del_ip_mcast_sip_dip(cparser_context_t *context,
    uint32_t *sip_ptr,
    uint32_t *dip_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddr_del(unit, *sip_ptr, *dip_ptr, 0), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DEL_IP_MCAST_SIP_DIP_VLAN_ID
/*
  * l2-table del ip-mcast <IPV4ADDR:sip> <IPV4ADDR:dip> <UINT:vlan_id>
  */
cparser_result_t cparser_cmd_l2_table_del_ip_mcast_sip_dip_vlan_id(cparser_context_t *context,
    uint32_t *sip_ptr,
    uint32_t *dip_ptr,
    uint32_t *vlan_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_l2_ipMcastAddr_del(unit, *sip_ptr, *dip_ptr, *vlan_id_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DEL_IP6_MCAST_SIP_DIP
/*
  * l2-table del ip6-mcast <IPV6ADDR:sip> <IPV6ADDR:dip>
  */
cparser_result_t cparser_cmd_l2_table_del_ip6_mcast_sip_dip(cparser_context_t *context,
    char **sip_ptr,
    char **dip_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_ip6McastAddr_t   ip6_mcast_data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.sip.ipv6_addr[0], (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.dip.ipv6_addr[0], (uint8 *)TOKEN_STR(4)), ret);

    DIAG_UTIL_ERR_CHK(rtk_l2_ip6McastAddr_del(unit, ip6_mcast_data.sip, ip6_mcast_data.dip, 0), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DEL_IP6_MCAST_SIP_DIP_VLAN_ID
/*
  * l2-table del ip6-mcast <IPV6ADDR:sip> <IPV6ADDR:dip> <UINT:vlan_id>
  */
cparser_result_t cparser_cmd_l2_table_del_ip6_mcast_sip_dip_vlan_id(cparser_context_t *context,
    char **sip_ptr,
    char **dip_ptr,
    uint32_t *vlan_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_l2_ip6McastAddr_t   ip6_mcast_data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.sip.ipv6_addr[0], (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.dip.ipv6_addr[0], (uint8 *)TOKEN_STR(4)), ret);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vlan_id_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vlan_id_ptr > 4095), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(rtk_l2_ip6McastAddr_del(unit, ip6_mcast_data.sip, ip6_mcast_data.dip, *vlan_id_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_DUMP_IP6_MCAST_SIP_DIP
/*
*l2-table dump ip6-mcast <IPV6ADDR:sip> <IPV6ADDR:dip>
*/
cparser_result_t cparser_cmd_l2_table_dump_ip6_mcast_sip_dip(cparser_context_t *context,
    char **sip_ptr,
    char **dip_ptr)
{
    uint8                   port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8                   ipv6DipStr[32], ipv6SipStr[32];
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_l2_ip6McastAddr_t   ip6_mcast_data;
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == sip_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == dip_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_OUTPUT_INIT();

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    memset(&ip6_mcast_data, 0, sizeof(rtk_l2_ip6McastAddr_t));

    diag_util_mprintf("SIP             | DIP             | VID  | Port     \n");
    diag_util_mprintf("-----------------+-----------------+------+-----------------------\n");

    /* show specific ip-ipmcast entry */
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.sip.ipv6_addr[0], (uint8 *)TOKEN_STR(3)), ret);
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6((uint8 *)&ip6_mcast_data.dip.ipv6_addr[0], (uint8 *)TOKEN_STR(4)), ret);

    DIAG_UTIL_ERR_CHK(rtk_l2_ip6McastAddr_get(unit, &ip6_mcast_data), ret);

    diag_util_mprintf("vid = %d\n", ip6_mcast_data.rvid);

    memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
    diag_util_lPortMask2str(port_list, &ip6_mcast_data.portmask);
    DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6SipStr, (uint8 *)&ip6_mcast_data.sip.ipv6_addr[0]), ret);
    DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6DipStr, (uint8 *)&ip6_mcast_data.dip.ipv6_addr[0]), ret);
    diag_util_mprintf("%s | %s | %4d | %20s\n", ipv6SipStr, ipv6DipStr, ip6_mcast_data.rvid, port_list);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_EXCEPT_SMAC_MCAST_BCAST_SA_ZERO_SA_DROP_FORWARD_TRAP_TO_CPU
/*
 * l2-table set except-smac ( mcast-bcast-SA | zero-SA ) ( drop | forward | trap-to-cpu )
 */
cparser_result_t cparser_cmd_l2_table_set_except_smac_mcast_bcast_sa_zero_sa_drop_forward_trap_to_cpu(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_l2_lookupMissType_t type = DLF_TYPE_UCAST;
    rtk_action_t    action  = ACTION_FORWARD;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('m' == TOKEN_CHAR(3, 0))
    {
        type = SA_IS_BCAST_OR_MCAST;
    }
    else if ('d' == TOKEN_CHAR(3, 0))
    {
        type = SA_IS_ZERO;
    }

    if ('f' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_FORWARD;
    }
    else if ('d' == TOKEN_CHAR(4, 0))
    {
        action = ACTION_DROP;
    }
    else
    {
        action = ACTION_TRAP2CPU;
    }

    DIAG_UTIL_ERR_CHK(rtk_l2_exceptionAddrAction_set(unit, type, action), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_SET_SA_BLOCK_DA_BLOCK_PORT_PORTS_ALL_STATE_ENABLE_DISABLE
/*
 * l2-table set ( sa-block | da-block ) port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_l2_table_set_sa_block_da_block_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char **ports_ptr)
{

    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_l2_macFilterMode_t mode = MAC_FILTER_MODE_SA;
    rtk_enable_t enable = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('s' == TOKEN_CHAR(2, 0))
    {
        mode = MAC_FILTER_MODE_SA;
    }
    else if ('d' == TOKEN_CHAR(2, 0))
    {
        mode = MAC_FILTER_MODE_DA;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }


    if ('e' == TOKEN_CHAR(6, 0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(6, 0))
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
        DIAG_UTIL_ERR_CHK(rtk_l2_portMacFilterEnable_set(unit, port, mode, enable), ret);
    }

    return CPARSER_OK;

}   /* end of cparser_cmd_l2_table_set_sa_block_da_block_port_ports_all_state_enable_disable */
#endif

#ifdef CMD_L2_TABLE_GET_SA_BLOCK_DA_BLOCK_PORT_PORTS_ALL_STATE
/*
 * l2-table get ( sa-block | da-block ) port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_l2_table_get_sa_block_da_block_port_ports_all_state(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_l2_macFilterMode_t mode = MAC_FILTER_MODE_SA;
    rtk_enable_t        enable = DISABLED;
    rtk_port_t          port = 0;
    diag_portlist_t     portlist;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if ('s' == TOKEN_CHAR(2, 0))
    {
        mode = MAC_FILTER_MODE_SA;
        diag_util_mprintf("\tSA-BLOCK Configuration\n");
    }
    else if ('d' == TOKEN_CHAR(2, 0))
    {
        mode = MAC_FILTER_MODE_DA;
        diag_util_mprintf("\tDA-BLOCK Configuration\n");
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_l2_portMacFilterEnable_get(unit, port, mode, &enable), ret);
        diag_util_mprintf("\tPort %d : %s\n", port, enable ? "Enable" : "Disable");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_l2_table_get_sa_block_da_block_port_ports_all_state */
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_UCAST_VID_MAC_ROUTE_TARGET_VLAN_INNER_OUTER
/*
 * l2-table add mac-ucast <UINT:vid> <MACADDR:mac> route-target-vlan ( inner | outer )
 */
cparser_result_t cparser_cmd_l2_table_add_mac_ucast_vid_mac_route_target_vlan_inner_outer(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    uint32 vlan_target = 0;
    rtk_l2_ucastAddr_t  l2_ucast;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    if ('i' == TOKEN_CHAR(6, 0))
    {
        vlan_target = 0;
    }
    else
    {
        vlan_target = 1;
    }

    memset(&l2_ucast, 0, sizeof(rtk_l2_ucastAddr_t));
    memcpy(l2_ucast.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2_ucast.vid = *vid_ptr;
    /* Add for test chip */
    l2_ucast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;

    if (rtk_l2_addr_get(unit, &l2_ucast) == RT_ERR_L2_ENTRY_NOTFOUND)
    {
        diag_util_printf("Entry is not exist.\n");
        return CPARSER_NOT_OK;
    }

    l2_ucast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    l2_ucast.vlan_target = vlan_target;
    DIAG_UTIL_ERR_CHK(rtk_l2_addr_add(unit, &l2_ucast), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_L2_TABLE_ADD_MAC_UCAST_VID_MAC_ROUTE_TABLE_INDEX_INDEX
/*
 * l2-table add mac-ucast <UINT:vid> <MACADDR:mac> route-table-index <UINT:index>
 */
cparser_result_t cparser_cmd_l2_table_add_mac_ucast_vid_mac_route_table_index_index(cparser_context_t *context,
    uint32_t *vid_ptr,
    cparser_macaddr_t *mac_ptr,
    uint32_t *index_ptr)
{
    uint32 unit = 0;
    int32  ret = RT_ERR_FAILED;
    uint32 routeEntries;
    rtk_l2_ucastAddr_t  l2_ucast;

    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == vid_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    DIAG_OM_GET_CHIP_CAPACITY(unit, routeEntries, max_num_of_route_host_addr);
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr >= routeEntries), CPARSER_ERR_INVALID_PARAMS);

    if ((TRUE == diag_util_isBcastMacAddr(mac_ptr->octet)) || (TRUE == diag_util_isMcastMacAddr(mac_ptr->octet)))
    {
        diag_util_printf("Broadcast and Multicast MAC address is not allowed to configure.\n");
        return CPARSER_NOT_OK;
    }

    memset(&l2_ucast, 0, sizeof(rtk_l2_ucastAddr_t));
    memcpy(l2_ucast.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    l2_ucast.vid = *vid_ptr;
    /* Add for test chip */
    l2_ucast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;

    if (rtk_l2_addr_get(unit, &l2_ucast) == RT_ERR_L2_ENTRY_NOTFOUND)
    {
        diag_util_printf("Entry is not exist.\n");
        return CPARSER_NOT_OK;
    }

    l2_ucast.flags |= RTK_L2_UCAST_FLAG_NEXTHOP;
    l2_ucast.route_idx = *index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_l2_addr_add(unit, &l2_ucast), ret);

    return CPARSER_OK;
}
#endif


#ifdef CMD_L2_TABLE_GET_NEXT_VALID_ENTRY_TYPE_CURRENT_ENTRY_DYNAMIC_UCAST_DYNAMIC_UCAST_AND_MCAST_UCAST_AND_MCAST_START_INDEX_INDEX
/*
*l2-table get next-valid-entry type ( current-entry | dynamic-ucast | dynamic-ucast-and-mcast | ucast-and-mcast ) start-index <UINT:index>
*/
cparser_result_t cparser_cmd_l2_table_get_next_valid_entry_type_current_entry_dynamic_ucast_dynamic_ucast_and_mcast_ucast_and_mcast_start_index_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32             unit = 0;
    int32              ret = RT_ERR_FAILED;

    rtk_l2_entry_t l2_entry;
    rtk_l2_nextValidType_t method;
    int32 scanIdx;

    uint8                   ipv6DipStr[32], ipv6SipStr[32];
    uint8                   strBuf1[20], strBuf2[20];
    uint8 port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    if ('c' == TOKEN_CHAR(4, 0))
    {
        method = L2_NEXT_VALID_TYPE_CURRENT;
    }
    else if ('u' == TOKEN_CHAR(4, 0))
    {
        method = L2_NEXT_VALID_TYPE_UC_AND_MC;
    }
    else if('a' == TOKEN_CHAR(4, 14))
    {
        method = L2_NEXT_VALID_TYPE_DYNAMIC_UC_AND_MC;
    }
    else
        method = L2_NEXT_VALID_TYPE_DYNAMIC_UC;

    scanIdx = *index_ptr;

    diag_util_mprintf("scan from Index %d, method = %d\n", scanIdx, method);

    DIAG_UTIL_ERR_CHK(rtk_l2_hwNextValidAddr_get(unit, &scanIdx, method, &l2_entry), ret);

    if(l2_entry.valid == 0)
    {
        diag_util_mprintf("entry is not exist!\n");
        return CPARSER_OK;
    }

    if(l2_entry.entry_type  == FLOW_TYPE_UNICAST)
    {
        if(l2_entry.unicast.flags & RTK_L2_UCAST_FLAG_NEXTHOP)
        {
            diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Static  | Nexthop  | Suspend | Vid_Sel |Rout_index |\n");
            diag_util_mprintf("------+-------------------+------+------+----------+----------+---------+----------+---------+-------+------\n");

            diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %2d | %4d\n",
                scanIdx,
                l2_entry.unicast.mac.octet[0],l2_entry.unicast.mac.octet[1],l2_entry.unicast.mac.octet[2],l2_entry.unicast.mac.octet[3],l2_entry.unicast.mac.octet[4],l2_entry.unicast.mac.octet[5],
                l2_entry.unicast.vid, l2_entry.unicast.port, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                (l2_entry.unicast.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_entry.unicast.vlan_target, l2_entry.unicast.route_idx);

        }
        else
        {
            diag_util_mprintf("Index | MAC Address       | VID  | SPA  | SaBlock  | DaBlock  | Static  | Nexthop  | Suspend | Aggreate VID\n");
            diag_util_mprintf("------+-------------------+------+------+----------+----------+---------+----------+---------+-------------\n");

            diag_util_mprintf("%5d | %02X:%02X:%02X:%02X:%02X:%02X | %4d | %4d | %8d | %8d | %7d | %8d | %7d | %4d\n",
                scanIdx,
                l2_entry.unicast.mac.octet[0],l2_entry.unicast.mac.octet[1],l2_entry.unicast.mac.octet[2],l2_entry.unicast.mac.octet[3],l2_entry.unicast.mac.octet[4],l2_entry.unicast.mac.octet[5],
                l2_entry.unicast.vid, l2_entry.unicast.port, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_SA_BLOCK)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_DA_BLOCK)?1:0,
                (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_STATIC)?1:0, (l2_entry.unicast.flags&RTK_L2_UCAST_FLAG_NEXTHOP)?1:0,
                (l2_entry.unicast.state&RTK_L2_UCAST_STATE_SUSPEND)?1:0, l2_entry.unicast.agg_vid);
        }
    }
    else if(l2_entry.entry_type  == FLOW_TYPE_L2_MULTI)
    {
        diag_util_mprintf("Index | VID  | MAC address       | Port                 | Agg Vid\n");
        diag_util_mprintf("------+------+-------------------+----------------------+-----------\n");
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &l2_entry.l2mcast.portmask);

        diag_util_mprintf("%5d | %4d | %02X:%02X:%02X:%02X:%02X:%02X | %20s | %4d\n",
                    scanIdx,
                    l2_entry.l2mcast.rvid, l2_entry.l2mcast.mac.octet[0], l2_entry.l2mcast.mac.octet[1], l2_entry.l2mcast.mac.octet[2],
                    l2_entry.l2mcast.mac.octet[3], l2_entry.l2mcast.mac.octet[4], l2_entry.l2mcast.mac.octet[5], port_list, l2_entry.l2mcast.agg_vid);
    }
    else if(l2_entry.entry_type  == FLOW_TYPE_IP4_MULTI)
    {

        diag_util_mprintf("Index | SIP             | DIP             | VID  | Port                 \n");
        diag_util_mprintf("------+-----------------+-----------------+------+----------------------\n");

        diag_util_ip2str_format(strBuf1, l2_entry.ipmcast.sip, 15);
        diag_util_ip2str_format(strBuf2, l2_entry.ipmcast.dip, 15);
        diag_util_mprintf("%5d | %s | %s | %4d | %20s\n", scanIdx, strBuf1, strBuf2, l2_entry.ipmcast.rvid, port_list);
    }
    else if(l2_entry.entry_type  == FLOW_TYPE_IP6_MULTI)
    {
        diag_util_mprintf("Index | SIP                  | DIP                  | VID  | Port                 \n");
        diag_util_mprintf("------+----------------------+----------------------+------+-----------------------\n");

        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(port_list, &l2_entry.ip6mcast.portmask);
        DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6SipStr, (uint8 *)&l2_entry.ip6mcast.sip.ipv6_addr[0]), ret);
        DIAG_UTIL_ERR_CHK(diag_util_ipv62str(ipv6DipStr, (uint8 *)&l2_entry.ip6mcast.dip.ipv6_addr[0]), ret);
        diag_util_mprintf("%5d | %20s | %20s | %4d | %20s \n", scanIdx, ipv6SipStr, ipv6DipStr, l2_entry.ip6mcast.rvid, port_list);
    }
    else
        return CPARSER_NOT_OK;

    return CPARSER_OK;
}

#endif

