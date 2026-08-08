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
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those rate command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) ingress bandwidth
 *           2) egress bandwidth
 *           3) storm control
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
#include <rtk/rate.h>
#include <diag_util.h>
#include <diag_om.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#define FAST_PATH_INDEX_MIN 0
#define FAST_PATH_INDEX_MAX 2

#define DSCP_INDEX_MAX      63
#define PRI_INDEX_MAX       7
#define VID_INDEX_MAX       4095

#ifdef CMD_BANDWIDTH_GET_EGRESS_PORT_ALL
/*
 * bandwidth get egress ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_get_egress_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    uint32          rate = 0;
    uint32          burst_size = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %2d : \n", port);
        DIAG_UTIL_ERR_CHK(rtk_rate_egrBandwidthCtrlEnable_get(unit, port, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("\tEgress Bandwidth : Enable\n");
        }
        else
        {
            diag_util_mprintf("\tEgress Bandwidth : Disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_rate_egrBandwidthCtrlRate_get(unit, port, &rate), ret);
        diag_util_mprintf("\tRate : %d (0x%x)\n", rate, rate);
        DIAG_UTIL_ERR_CHK(rtk_rate_portEgrBandwidthCtrlBurstSize_get(unit, port, &burst_size), ret);
        diag_util_mprintf("    Burst Size : %d (0x%x)\n", burst_size, burst_size);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_egress_port_all */
#endif

#ifdef CMD_BANDWIDTH_GET_EGRESS_IFG
/*
 * bandwidth get egress ifg
 */
cparser_result_t cparser_cmd_bandwidth_get_egress_ifg(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    ifg_include = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    /* show all egr-bandwidth info */
    DIAG_UTIL_ERR_CHK(rtk_rate_egrBandwidthCtrlIncludeIfg_get(unit, &ifg_include), ret);
    if (ENABLED == ifg_include)
    {
        diag_util_mprintf("Egress Bandwidth : Include IFG\n");
    }
    else
    {
        diag_util_mprintf("Egress Bandwidth : Exclude IFG\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_egress_ifg */
#endif

#ifdef CMD_BANDWIDTH_GET_EGRESS_PORT_ALL_QUEUE_ID
/*
 * bandwidth get egress ( <PORT_LIST:port> | all ) <UINT:queue_id>
 */
cparser_result_t cparser_cmd_bandwidth_get_egress_port_all_queue_id(cparser_context_t *context,
    char **port_ptr,
    uint32_t *queue_id_ptr)
{
    uint32          unit = 0;
    uint32          rate = 0;
    uint32          burst_size = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_qid_t       queue_id = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    queue_id = *queue_id_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %2d :\n", port);
        DIAG_UTIL_ERR_CHK(rtk_rate_egrQueueBwCtrlEnable_get(unit, port, queue_id, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("\tEgress Bandwidth (Queue %d) : Enable\n", queue_id);
        }
        else
        {
            diag_util_mprintf("\tEgress Bandwidth (Queue %d) : Disable\n", queue_id);
        }

        DIAG_UTIL_ERR_CHK(rtk_rate_egrQueueBwCtrlRate_get(unit, port, queue_id, &rate), ret);
        diag_util_mprintf("\tRate (Queue %d) : %d (0x%x)\n", queue_id, rate, rate);
        DIAG_UTIL_ERR_CHK(rtk_rate_egrPortQueueBwCtrlBurstSize_get(unit, port, queue_id, &burst_size), ret);
        diag_util_mprintf("\tBurst Size (Queue %d) : %d (0x%x)\n", queue_id, burst_size, burst_size);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_egress_port_all_queue_id */
#endif

#ifdef CMD_BANDWIDTH_GET_EGRESS_IFG_PORT_ALL
/*
 * bandwidth get egress ifg ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_get_egress_ifg_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portEgrBandwidthCtrlIncludeIfg_get(unit, port, &enable), ret);
        if (ENABLED == enable)
            diag_util_mprintf("    Port %2d : Include\n", port);
        else
            diag_util_mprintf("    Port %2d : Exclude\n", port);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_egress_ifg_port_all */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_IFG_EXCLUDE_INCLUDE
/*
 * bandwidth set egress ifg ( exclude | include )
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_ifg_exclude_include(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('i' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    /* set IFG include */
    DIAG_UTIL_ERR_CHK(rtk_rate_egrBandwidthCtrlIncludeIfg_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_ifg_exclude_include */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_IFG_PORT_ALL_EXCLUDE_INCLUDE
/*
 * bandwidth set egress ifg ( <PORT_LIST:port> | all ) ( exclude | include )
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_ifg_port_all_exclude_include(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('i' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /* set IFG include */
        DIAG_UTIL_ERR_CHK(rtk_rate_portEgrBandwidthCtrlIncludeIfg_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_ifg_port_all_exclude_include */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_egrBandwidthCtrlEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_port_all_state_disable_enable */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_PORT_ALL_BURST_SIZE_SIZE
/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) burst-size <UINT:size>
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_port_all_burst_size_size(cparser_context_t *context,
    char **port_ptr,
    uint32_t *size_ptr)
{
    uint32      unit = 0;
    uint32      size_value = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    size_value = *size_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portEgrBandwidthCtrlBurstSize_set(unit, port, size_value), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_port_all_burst_size_size */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_PORT_ALL_RATE_RATE
/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) rate <UINT:rate>
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_port_all_rate_rate(cparser_context_t *context,
    char **port_ptr,
    uint32_t *rate_ptr)
{
    uint32      unit = 0;
    uint32      rate_value = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    rate_value = *rate_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_egrBandwidthCtrlRate_set(unit, port, rate_value), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_port_all_rate_rate */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_PORT_ALL_QUEUE_QUEUE_ID_STATE_DISABLE_ENABLE
/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) queue <UINT:queue_id> state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_port_all_queue_queue_id_state_disable_enable(cparser_context_t *context,
    char **port_ptr,
    uint32_t *queue_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_qid_t       queue_id = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    queue_id = *queue_id_ptr;

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
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_egrQueueBwCtrlEnable_set(unit, port, queue_id, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_port_all_queue_queue_id_state_disable_enable */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_PORT_ALL_QUEUE_QUEUE_ID_BURST_SIZE_SIZE
/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) queue <UINT:queue_id> burst-size <UINT:size>
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_port_all_queue_queue_id_burst_size_size(cparser_context_t *context,
    char **port_ptr,
    uint32_t *queue_id_ptr,
    uint32_t *size_ptr)
{
    uint32      unit = 0;
    uint32      size_value = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_qid_t   queue_id = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    queue_id = *queue_id_ptr;
    size_value = *size_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_egrPortQueueBwCtrlBurstSize_set(unit, port, queue_id, size_value), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_port_all_queue_queue_id_burst_size_size */
#endif

#ifdef CMD_BANDWIDTH_SET_EGRESS_PORT_ALL_QUEUE_QUEUE_ID_RATE_RATE
/*
 * bandwidth set egress ( <PORT_LIST:port> | all ) queue <UINT:queue_id> rate <UINT:rate>
 */
cparser_result_t cparser_cmd_bandwidth_set_egress_port_all_queue_queue_id_rate_rate(cparser_context_t *context,
    char **port_ptr,
    uint32_t *queue_id_ptr,
    uint32_t *rate_ptr)
{
    uint32      unit = 0;
    uint32      rate_value = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_qid_t       queue_id = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    queue_id = *queue_id_ptr;
    rate_value = *rate_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_egrQueueBwCtrlRate_set(unit, port, queue_id, rate_value), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_egress_port_all_queue_queue_id_rate_rate */
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_IFG
/*
 * bandwidth get ingress ifg
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_ifg(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    ifg_include = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    /* show all igr-bandwidth info */
    DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlIncludeIfg_get(unit, &ifg_include), ret);
    if (ENABLED == ifg_include)
    {
        diag_util_mprintf("Ingress Bandwidth : Include IFG\n");
    }
    else
    {
        diag_util_mprintf("Ingress Bandwidth : Exclude IFG\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_ingress_ifg */
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_BYPASS_PACKET_ARP_REQUEST_RMA_BPDU_RTK_CTRL_PKT_IGMP_STATE
/*
 * bandwidth get ingress bypass ( arp-request | rma | bpdu | rtk-ctrl-pkt | igmp ) state
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_bypass_packet_arp_request_rma_bpdu_rtk_ctrl_pkt_igmp_state(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('m' == TOKEN_CHAR(4,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlBypass_get(unit, IGR_BYPASS_RMA, &enable), ret);
        diag_util_mprintf("RMA Bypass Ingress Bandwidth Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('p' == TOKEN_CHAR(4,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlBypass_get(unit, IGR_BYPASS_BPDU, &enable), ret);
        diag_util_mprintf("BPDU Bypass Ingress Bandwidth Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('t' == TOKEN_CHAR(4,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlBypass_get(unit, IGR_BYPASS_RTKPKT, &enable), ret);
        diag_util_mprintf("RTK Control Packet Bypass Ingress Bandwidth Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('g' == TOKEN_CHAR(4,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlBypass_get(unit, IGR_BYPASS_IGMP, &enable), ret);
        diag_util_mprintf("IGMP Bypass Ingress Bandwidth Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('r' == TOKEN_CHAR(4,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlBypass_get(unit, IGR_BYPASS_ARPREQ, &enable), ret);
        diag_util_mprintf("ARP Request Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_FAST_PATH_PORT_ALL
/*
 * bandwidth get ingress fast-path ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_fast_path_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    uint32          i = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_rate_igr_fpEntry_t  fpEntry;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf(" Port(FP_id) | Vlan Type (vid) | inner-pri | outer-pri | dscp    \n");
    diag_util_mprintf("-------------------------------------------------------------------------\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        for (i = FAST_PATH_INDEX_MIN; i <= FAST_PATH_INDEX_MAX; i++)
        {
            DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_get(unit, port, i, &fpEntry), ret);
            diag_util_printf("  %2d ( %3d ) ", port, i);
            if (ENABLED == fpEntry.vlan_check)
            {
                if (INNER_VLAN == fpEntry.inner_or_outer_vlan)
                    diag_util_printf("  INNER  (%4d)   ", fpEntry.vid);
                else
                    diag_util_printf("  OUTER  (%4d)   ", fpEntry.vid);
            }
            else
            {
                diag_util_printf("   Not Defined    ");
            }

            if (ENABLED == fpEntry.innerPri_check)
            {
                diag_util_printf("     %2d     ", fpEntry.innerPri);
            }
            else
            {
                diag_util_printf(" Not Defined");
            }

            if (ENABLED == fpEntry.outerPri_check)
            {
                diag_util_printf("     %2d     ", fpEntry.outerPri);
            }
            else
            {
                diag_util_printf(" Not Defined");
            }

            if (ENABLED == fpEntry.dscp_check)
            {
                diag_util_printf("     %2d     ", fpEntry.dscp);
            }
            else
            {
                diag_util_printf("  Not Defined");
            }

            diag_util_mprintf("\n");
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_ingress_fast_path_port_all */
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_FLOW_CONTROL_PORT_ALL
/*
 * bandwidth get ingress flow-control ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_flow_control_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_rate_thresh_t   thresh;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthFlowctrlEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("Port %2d:\n", port);
        if (ENABLED == enable)
            diag_util_mprintf("    Status : Enabled\n");
        else
            diag_util_mprintf("    Status : Disabled\n");
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthFlowctrlThresh_get(unit, port, &thresh), ret);
        diag_util_mprintf("    Threshold:\n");
        diag_util_mprintf("        FC On  : %d (0x%x)\n", thresh.FC_On, thresh.FC_On);
        diag_util_mprintf("        FC Off : %d (0x%x)\n", thresh.FC_Off, thresh.FC_Off);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_ingress_flow_control_port_all */
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_FLOW_CONTROL_PORT_ALL_STATE
/*
 * bandwidth get ingress flow-control ( <PORT_LIST:port> | all ) state
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_flow_control_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Flow Control Status of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthFlowctrlEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("Port %2d : %s\n", port, (enable == ENABLED) ? "Enable" : "Disable");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_ingress_flow_control_port_all */
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_IFG_PORT_ALL
/*
 * bandwidth get ingress ifg ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_ifg_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portIgrBandwidthCtrlIncludeIfg_get(unit, port, &enable), ret);
        if (ENABLED == enable)
            diag_util_mprintf("    Port %2d : Include\n", port);
        else
            diag_util_mprintf("    Port %2d : Exclude\n", port);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_ingress_ifg_port_all */
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_FLOW_CONTROL_LOW_THRESHOLD
/*
 * bandwidth get ingress flow-control low-threshold
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_flow_control_low_threshold(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          thresh;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthFlowctrlOffThresh_get(unit, &thresh), ret);
    diag_util_mprintf("Flow Control Low Threshold of System : %d (0x%x)\n", thresh, thresh);

    return CPARSER_OK;
}
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_FLOW_CONTROL_HIGH_THRESHOLD_PORT_ALL
/*
 * bandwidth get ingress flow-control high-threshold ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_flow_control_high_threshold_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    uint32          thresh;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_mprintf("Flow Control High Threshold of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portIgrBandwidthFlowctrlOnThresh_get(unit, port, &thresh), ret);
        diag_util_mprintf("Port %2d : %d (0x%x)\n", port, thresh, thresh);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_BANDWIDTH_GET_INGRESS_PORT_ALL
/*
 * bandwidth get ingress ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_get_ingress_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    uint32          rate = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          isExceed;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %2d : \n", port);
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlEnable_get(unit, port, &enable), ret);
        if (ENABLED == enable)
        {
            diag_util_mprintf("\tIngress Bandwidth : Enable\n");
        }
        else
        {
            diag_util_mprintf("\tIngress Bandwidth : Disable\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlRate_get(unit, port, &rate), ret);
        diag_util_mprintf("\tRate : %d (0x%x)\n", rate, rate);

        if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
        {
            if (RT_ERR_OK == rtk_rate_portIgrBandwidthCtrlExceed_get(unit, port, &isExceed))
            {
                diag_util_mprintf("\tExceed Flag : %s\n", (TRUE == isExceed)?"  Exceed  ":" Not-exceed ");
            }
            else
            {
                diag_util_mprintf("\tNot Support \n");
            }
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_get_ingress_port_all */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FAST_PATH_PORT_ALL_ENTRY_INDEX_DSCP_STATE_DISABLE_ENABLE
/*
 * bandwidth set ingress fast-path ( <PORT_LIST:port> | all ) <UINT:entry_index> dscp state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_dscp_state_disable_enable(cparser_context_t *context,
    char **port_ptr,
    uint32_t *entry_index_ptr)
{
    uint32          unit = 0;
    uint32          fp_index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_rate_igr_fpEntry_t  fpEntry;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    fp_index = *entry_index_ptr;
    if (fp_index > FAST_PATH_INDEX_MAX)
    {
        diag_util_printf("User config : Fast path index out of range!\n");
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_CHAR(8,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(8,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }


    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_get(unit, port, fp_index, &fpEntry), ret);
        fpEntry.dscp_check = enable;
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_set(unit, port, fp_index, &fpEntry), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_dscp_state_disable_enable */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FAST_PATH_PORT_ALL_ENTRY_INDEX_DSCP_DSCP
/*
 * bandwidth set ingress fast-path ( <PORT_LIST:port> | all ) <UINT:entry_index> dscp <UINT:dscp>
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_dscp_dscp(cparser_context_t *context,
    char **port_ptr,
    uint32_t *entry_index_ptr,
    uint32_t *dscp_ptr)
{
    uint32          unit = 0;
    uint32          fp_index = 0;
    uint32          dscp_value = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_rate_igr_fpEntry_t  fpEntry;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    fp_index = *entry_index_ptr;
    if (fp_index > FAST_PATH_INDEX_MAX)
    {
        diag_util_printf("User config : Fast path index out of range!\n");
        return CPARSER_NOT_OK;
    }

    dscp_value = *dscp_ptr;
    if (dscp_value > DSCP_INDEX_MAX)
    {
        diag_util_printf("User config : Dscp index out of range!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_get(unit, port, fp_index, &fpEntry), ret);
        fpEntry.dscp = dscp_value;
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_set(unit, port, fp_index, &fpEntry), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_dscp_dscp */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FAST_PATH_PORT_ALL_ENTRY_INDEX_INNER_PRIORITY_OUTER_PRIORITY_STATE_DISABLE_ENABLE
/*
 * bandwidth set ingress fast-path ( <PORT_LIST:port> | all ) <UINT:entry_index> ( inner-priority | outer-priority ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_inner_priority_outer_priority_state_disable_enable(cparser_context_t *context,
    char **port_ptr,
    uint32_t *entry_index_ptr)
{
    uint32          unit = 0;
    uint32          fp_index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_rate_igr_fpEntry_t  fpEntry;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    fp_index = *entry_index_ptr;
    if (fp_index > FAST_PATH_INDEX_MAX)
    {
        diag_util_printf("User config : Fast path index out of range!\n");
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_CHAR(8,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(8,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }


    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_get(unit, port, fp_index, &fpEntry), ret);

        if ('i' == TOKEN_CHAR(6,0))
        {
            fpEntry.innerPri_check = enable;
        }
        else
        {
            fpEntry.outerPri_check = enable;
        }
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_set(unit, port, fp_index, &fpEntry), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_inner_priority_outer_priority_state_disable_enable */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FAST_PATH_PORT_ALL_ENTRY_INDEX_INNER_PRIORITY_OUTER_PRIORITY_PRI
/*
 * bandwidth set ingress fast-path ( <PORT_LIST:port> | all ) <UINT:entry_index> ( inner-priority | outer-priority ) <UINT:pri>
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_inner_priority_outer_priority_pri(cparser_context_t *context,
    char **port_ptr,
    uint32_t *entry_index_ptr,
    uint32_t *pri_ptr)
{
    uint32          unit = 0;
    uint32          fp_index = 0;
    uint32          pri_value = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_rate_igr_fpEntry_t  fpEntry;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    fp_index = *entry_index_ptr;
    if (fp_index > FAST_PATH_INDEX_MAX)
    {
        diag_util_printf("User config : Fast path index out of range!\n");
        return CPARSER_NOT_OK;
    }

    pri_value = *pri_ptr;
    if (pri_value > PRI_INDEX_MAX)
    {
        diag_util_printf("User config : Priority index out of range!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_get(unit, port, fp_index, &fpEntry), ret);

        if ('i' == TOKEN_CHAR(6,0))
        {
            fpEntry.innerPri = pri_value;
        }
        else
        {
            fpEntry.outerPri = pri_value;
        }
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_set(unit, port, fp_index, &fpEntry), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_inner_priority_outer_priority_pri */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FAST_PATH_PORT_ALL_ENTRY_INDEX_VLAN_CHECK_STATE_DISABLE_ENABLE
/*
 * bandwidth set ingress fast-path ( <PORT_LIST:port> | all ) <UINT:entry_index> vlan-check state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_vlan_check_state_disable_enable(cparser_context_t *context,
    char **port_ptr,
    uint32_t *entry_index_ptr)
{
    uint32          unit = 0;
    uint32          fp_index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_enable_t    enable = DISABLED;
    rtk_rate_igr_fpEntry_t  fpEntry;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    fp_index = *entry_index_ptr;
    if (fp_index > FAST_PATH_INDEX_MAX)
    {
        diag_util_printf("User config : Fast path index out of range!\n");
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_CHAR(8,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(8,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }


    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_get(unit, port, fp_index, &fpEntry), ret);
        fpEntry.vlan_check = enable;
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_set(unit, port, fp_index, &fpEntry), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_vlan_check_state_disable_enable */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FAST_PATH_PORT_ALL_ENTRY_INDEX_VLAN_CHECK_INNER_VLAN_OUTER_VLAN_VID
/*
 * bandwidth set ingress fast-path ( <PORT_LIST:port> | all ) <UINT:entry_index> vlan-check ( inner-vlan | outer-vlan ) <UINT:vid>
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_vlan_check_inner_vlan_outer_vlan_vid(cparser_context_t *context,
    char **port_ptr,
    uint32_t *entry_index_ptr,
    uint32_t *vid_ptr)
{
    uint32          unit = 0;
    uint32          fp_index = 0;
    uint32          vid_value = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    rtk_rate_igr_fpEntry_t  fpEntry;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    fp_index = *entry_index_ptr;
    if (fp_index > FAST_PATH_INDEX_MAX)
    {
        diag_util_printf("User config : Fast path index out of range!\n");
        return CPARSER_NOT_OK;
    }

    vid_value = *vid_ptr;
    if (vid_value > VID_INDEX_MAX)
    {
        diag_util_printf("User config : Vlan index out of range!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_get(unit, port, fp_index, &fpEntry), ret);

        if ('i' == TOKEN_CHAR(7,0))
        {
            fpEntry.inner_or_outer_vlan = INNER_VLAN;
            fpEntry.vid = vid_value;
        }
        else
        {
            fpEntry.inner_or_outer_vlan = OUTER_VLAN;
            fpEntry.vid = vid_value;
        }
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlFPEntry_set(unit, port, fp_index, &fpEntry), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_fast_path_port_all_entry_index_vlan_check_inner_vlan_outer_vlan_vid */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FLOW_CONTROL_THRESHOLD_PORT_ALL_FC_ON_FC_OFF
/*
 * bandwidth set ingress flow-control threshold ( <PORT_LIST:port> | all ) <UINT:fc_on> <UINT:fc_off>
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_flow_control_threshold_port_all_fc_on_fc_off(cparser_context_t *context,
    char **port_ptr,
    uint32_t *fc_on_ptr,
    uint32_t *fc_off_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_thresh_t   thresh;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    thresh.FC_On = *fc_on_ptr;
    thresh.FC_Off = *fc_off_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthFlowctrlThresh_set(unit, port, &thresh), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_flow_control_threshold_port_all_fc_on_fc_off */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FLOW_CONTROL_LOW_THRESHOLD_THRESH
/*
 * bandwidth set ingress flow-control low-threshold <UINT:thresh>
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_flow_control_low_threshold_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthFlowctrlOffThresh_set(unit, *thresh_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FLOW_CONTROL_HIGH_THRESHOLD_PORT_ALL_THRESH
/*
 * bandwidth set ingress flow-control high-threshold ( <PORT_LIST:port> | all ) <UINT:thresh>
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_flow_control_high_threshold_port_all_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portIgrBandwidthFlowctrlOnThresh_set(unit, port, *thresh_ptr), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_FLOW_CONTROL_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * bandwidth set ingress flow-control ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_flow_control_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

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
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthFlowctrlEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_flow_control_port_all_state_disable_enable */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_IFG_EXCLUDE_INCLUDE
/*
 * bandwidth set ingress ifg ( exclude | include )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_ifg_exclude_include(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('i' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    /* set IFG include */
    DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlIncludeIfg_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_ifg_exclude_include */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_IFG_PORT_ALL_EXCLUDE_INCLUDE
/*
 * bandwidth set ingress ifg ( <PORT_LIST:port> | all ) ( exclude | include )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_ifg_port_all_exclude_include(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('i' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /* set IFG include */
        DIAG_UTIL_ERR_CHK(rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_ifg_port_all_exclude_include */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * bandwidth set ingress ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlEnable_set(unit, port, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_port_all_state_disable_enable */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_PORT_ALL_RATE_RATE
/*
 * bandwidth set ingress ( <PORT_LIST:port> | all ) rate <UINT:rate>
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_port_all_rate_rate(cparser_context_t *context,
    char **port_ptr,
    uint32_t *rate_ptr)
{
    uint32      unit = 0;
    uint32      rate_value = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    rate_value = *rate_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlRate_set(unit, port, rate_value), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_bandwidth_set_ingress_port_all_rate_rate */
#endif

#ifdef CMD_BANDWIDTH_SET_INGRESS_BYPASS_PACKET_ARP_REQUEST_RMA_BPDU_RTK_CTRL_PKT_IGMP_STATE_DISABLE_ENABLE
/*
 * bandwidth set ingress bypass ( arp-request | rma | bpdu | rtk-ctrl-pkt | igmp ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_bandwidth_set_ingress_bypass_packet_arp_request_rma_bpdu_rtk_ctrl_pkt_igmp_state_disable_enable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_rate_igr_bypassType_t bypassType = IGR_BYPASS_RMA;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('m' == TOKEN_CHAR(4,1))
    {
        bypassType = IGR_BYPASS_RMA;
    }
    else if ('p' == TOKEN_CHAR(4,1))
    {
        bypassType = IGR_BYPASS_BPDU;
    }
    else if ('t' == TOKEN_CHAR(4,1))
    {
        bypassType = IGR_BYPASS_RTKPKT;
    }
    else if ('g' == TOKEN_CHAR(4,1))
    {
        bypassType = IGR_BYPASS_IGMP;
    }
    else if ('r' == TOKEN_CHAR(4,1))
    {
        bypassType = IGR_BYPASS_ARPREQ;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

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
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_rate_igrBandwidthCtrlBypass_set(unit, bypassType, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_BANDWIDTH_RESET_INGRESS_METER_EXCEED_PORT_ALL
/*
 * bandwidth reset ingress meter-exceed ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_bandwidth_reset_ingress_meter_exceed_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portIgrBandwidthCtrlExceed_reset(unit, port), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_GET_IFG
/*
 * storm-control get ifg
 */
cparser_result_t cparser_cmd_storm_control_get_ifg(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    ifg_include = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_rate_stormControlIncludeIfg_get(unit, &ifg_include), ret);
    if (ENABLED == ifg_include)
    {
        diag_util_mprintf("Storm Control Include IFG\n");
    }
    else
    {
        diag_util_mprintf("Storm Control Exclude IFG\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_get_ifg */
#endif

#ifdef CMD_STORM_CONTROL_GET_MODE
/*
 * storm-control get mode
 */
cparser_result_t cparser_cmd_storm_control_get_mode(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_rate_storm_rateMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_rate_stormControlRefreshMode_get(unit, &mode), ret);
    if (BASED_ON_BYTE == mode)
    {
        diag_util_mprintf("Storm Control Mode : Byte\n");
    }
    else
    {
        diag_util_mprintf("Storm Control Mode : Packet\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_get_mode */
#endif

#ifdef CMD_STORM_CONTROL_GET_COUNTING_MODE
/*
 * storm-control get counting-mode
 */
cparser_result_t cparser_cmd_storm_control_get_counting_mode(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_rate_storm_rateMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_rate_stormControlRateMode_get(unit, &mode), ret);
    if (BASED_ON_BYTE == mode)
    {
        diag_util_mprintf("Storm Control Counting Mode : Byte\n");
    }
    else
    {
        diag_util_mprintf("Storm Control Counting Mode : Packet\n");
    }

    return CPARSER_OK;
}/* end of cparser_cmd_storm_control_get_counting_mode */
#endif

#ifdef CMD_STORM_CONTROL_GET_BYPASS_PACKET_ARP_REQUEST_BPDU_IGMP_RMA_RTK_CTRL_PKT_STATE
/*
 * storm-control get bypass ( arp-request | bpdu | igmp | rma | rtk-ctrl-pkt ) state
 */
cparser_result_t cparser_cmd_storm_control_get_bypass_packet_arp_request_bpdu_igmp_rma_rtk_ctrl_pkt_state(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('m' == TOKEN_CHAR(3,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlBypass_get(unit, STORM_BYPASS_RMA, &enable), ret);
        diag_util_mprintf("RMA Packet Bypass Storm Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('r' == TOKEN_CHAR(3,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlBypass_get(unit, STORM_BYPASS_ARP, &enable), ret);
        diag_util_mprintf("ARP Request Packet Bypass Storm Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('p' == TOKEN_CHAR(3,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlBypass_get(unit, STORM_BYPASS_BPDU, &enable), ret);
        diag_util_mprintf("BPDU Packet Bypass Storm Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('t' == TOKEN_CHAR(3,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlBypass_get(unit, STORM_BYPASS_RTKPKT, &enable), ret);
        diag_util_mprintf("RTK Control Packet Bypass Storm Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else if ('g' == TOKEN_CHAR(3,1))
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlBypass_get(unit, STORM_BYPASS_IGMP, &enable), ret);
        diag_util_mprintf("IGMP Packet Bypass Storm Control State : %s\n", (enable == ENABLED) ? DIAG_STR_ENABLE : DIAG_STR_DISABLE);
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_GET_BROADCAST_MULTICAST_UNKNOWN_MULTICAST_UNKNOWN_UNICAST_PORT_ALL
/*
 * storm-control get ( broadcast | multicast | unknown-multicast | unknown-unicast ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_storm_control_get_broadcast_multicast_unknown_multicast_unknown_unicast_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    uint32      rate = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;
    rtk_rate_storm_group_t      type;
    rtk_rate_storm_rateMode_t   mode;
    rtk_switch_devInfo_t devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_BROADCAST;
    }
    else if ('m' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else
    {
        if ('m' == TOKEN_CHAR(2,8))
        {
            type = STORM_GROUP_UNKNOWN_MULTICAST;
        }
        else
        {
            type = STORM_GROUP_UNKNOWN_UNICAST;
        }
    }

    if((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (devInfo.chipId == RTL8389M_CHIP_ID || devInfo.chipId == RTL8389L_CHIP_ID ||
        devInfo.chipId == RTL8329M_CHIP_ID || devInfo.chipId == RTL8377M_CHIP_ID)
    {
        diag_util_mprintf(" Port |  Status  |   Rate   \n");
        diag_util_mprintf("============================\n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_printf("  %2d  ", port);
            DIAG_UTIL_ERR_CHK(rtk_rate_stormControlRate_get(unit, port, type, &rate), ret);
            diag_util_printf("  %s ", (rate != 0xFFFFF)?"Enabled ":"Disabled");
            diag_util_printf("  %8d ", rate);
            diag_util_mprintf("\n");
        }
    }
    else
    {
        diag_util_mprintf(" Port |  Status  |  Mode  |   Rate   | Burst Rate\n");
        diag_util_mprintf("=================================================\n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_printf("  %2d  ", port);
            DIAG_UTIL_ERR_CHK(rtk_rate_stormControlEnable_get(unit, port, type, &enable), ret);
            diag_util_printf("  %s ", (ENABLED == enable)?"Enabled ":"Disabled");
            DIAG_UTIL_ERR_CHK(rtk_rate_portStormControlRateMode_get(unit, port, type, &mode), ret);
            diag_util_printf("  %s ", (BASED_ON_PKT == mode)?"Packet":" Byte ");
            DIAG_UTIL_ERR_CHK(rtk_rate_stormControlRate_get(unit, port, type, &rate), ret);
            diag_util_printf("  %8d ", rate);
            DIAG_UTIL_ERR_CHK(rtk_rate_stormControlBurstRate_get(unit, port, type, &rate), ret);
            diag_util_printf("  %8d", rate);
            diag_util_mprintf("\n");
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_get_broadcast_multicast_unknown_multicast_unknown_unicast_port_all */
#endif

#ifdef CMD_STORM_CONTROL_GET_UNICAST_MULTICAST_PORT_ALL_TYPE
/*
 * storm-control get ( unicast | multicast ) ( <PORT_LIST:port> | all ) type
 */
cparser_result_t cparser_cmd_storm_control_get_unicast_multicast_port_all_type(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_rate_storm_group_t      type;
    rtk_rate_storm_sel_t   storm_sel;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('u' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_UNICAST;
        diag_util_mprintf("Unicast Type Selection\n");
    }
    else
    {
        type = STORM_GROUP_MULTICAST;
        diag_util_mprintf("Multicast Type Selection\n");
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlTypeSel_get(unit, port, type, &storm_sel), ret);
        diag_util_mprintf("Port %2d : %s\n", port, (storm_sel == STORM_SEL_UNKNOWN_AND_KNOWN) ? "Both Known and Unknown" : "Unknown Only");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_GET_ARP_BPDU_IGMP_PORT_ALL
/*
 * storm-control get ( arp | bpdu | igmp ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_storm_control_get_arp_bpdu_igmp_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    uint32      rate = 0;
    uint32      isExceed = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_rate_storm_proto_group_t      type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('a' == TOKEN_CHAR(2,0))
    {
        type = STORM_PROTO_GROUP_ARP;
    }
    else if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_PROTO_GROUP_BPDU;
    }
    else if ('i' == TOKEN_CHAR(2,0))
    {
        type = STORM_PROTO_GROUP_IGMP;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf(" Port |   Rate   | Meter Exceed\n");
    diag_util_mprintf("================================\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("  %2d  ", port);

        if (RT_ERR_OK == rtk_rate_stormControlProtoRate_get(unit, port, type, &rate))
        {
            diag_util_mprintf("  %8d ", rate);
        }
        else
        {
            diag_util_mprintf(" Not Support ");
        }

        if (RT_ERR_OK == rtk_rate_stormControlProtoExceed_get(unit, port, type, &isExceed))
        {
            diag_util_mprintf("  %s ", (TRUE == isExceed)?"  Exceed  ":" Not-exceed ");
        }
        else
        {
            diag_util_mprintf(" Not Support ");
        }

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_RESET_METER_EXCEED_ARP_BPDU_IGMP_PORT_ALL
/*
 * storm-control reset meter-exceed ( arp | bpdu | igmp ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_storm_control_reset_meter_exceed_arp_bpdu_igmp_port_all(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_rate_storm_proto_group_t      type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('a' == TOKEN_CHAR(3,0))
    {
        type = STORM_PROTO_GROUP_ARP;
    }
    else if ('b' == TOKEN_CHAR(3,0))
    {
        type = STORM_PROTO_GROUP_BPDU;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        type = STORM_PROTO_GROUP_IGMP;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlProtoExceed_reset(unit, port, type), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_RESET_METER_EXCEED_BROADCAST_MULTICAST_UNICAST_PORT_ALL
/*
 * storm-control reset meter-exceed ( broadcast | multicast | unicast ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_storm_control_reset_meter_exceed_broadcast_multicast_unicast_port_all(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_rate_storm_group_t      type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('b' == TOKEN_CHAR(3,0))
    {
        type = STORM_GROUP_BROADCAST;
    }
    else if ('m' == TOKEN_CHAR(3,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        type = STORM_GROUP_UNICAST;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlExceed_reset(unit, port, type), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_SET_IFG_EXCLUDE_INCLUDE
/*
 * storm-control set ifg ( exclude | include )
 */
cparser_result_t cparser_cmd_storm_control_set_ifg_exclude_include(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('i' == TOKEN_CHAR(3,0))
    {
        enable = ENABLED;
    }
    else if ('e' == TOKEN_CHAR(3,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    /* set IFG include */
    DIAG_UTIL_ERR_CHK(rtk_rate_stormControlIncludeIfg_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_set_ifg_exclude_include */
#endif

#ifdef CMD_STORM_CONTROL_SET_COUNTING_MODE_BYTE_PACKET
/*
 * storm-control set counting-mode ( byte | packet )
 */
cparser_result_t cparser_cmd_storm_control_set_counting_mode_byte_packet(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_rate_storm_rateMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(3,0))
    {
        mode = BASED_ON_BYTE;
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        mode = BASED_ON_PKT;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    /* set storm counting mode */
    DIAG_UTIL_ERR_CHK(rtk_rate_stormControlRateMode_set(unit, mode), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_set_counting_mode_byte_packet */
#endif

#ifdef CMD_STORM_CONTROL_SET_MODE_BYTE_PACKET
/*
 * storm-control set mode ( byte | packet )
 */
cparser_result_t cparser_cmd_storm_control_set_mode_byte_packet(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_rate_storm_rateMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(3,0))
    {
        mode = BASED_ON_BYTE;
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        mode = BASED_ON_PKT;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    /* set storm mode */
    DIAG_UTIL_ERR_CHK(rtk_rate_stormControlRefreshMode_set(unit, mode), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_set_mode_byte_packet */
#endif

#ifdef CMD_STORM_CONTROL_SET_BROADCAST_MULTICAST_UNKNOWN_MULTICAST_UNKNOWN_UNICAST_PORT_ALL_BURST_RATE_RATE
/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) ( <PORT_LIST:port> | all ) burst-rate <UINT:rate>
 */
cparser_result_t cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_burst_rate_rate(cparser_context_t *context,
    char **port_ptr,
    uint32_t *rate_ptr)
{
    uint32      unit = 0;
    uint32      rate = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_storm_group_t  type;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_BROADCAST;
    }
    else if ('m' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else
    {
        if ('m' == TOKEN_CHAR(2,8))
        {
            type = STORM_GROUP_UNKNOWN_MULTICAST;
        }
        else
        {
            type = STORM_GROUP_UNKNOWN_UNICAST;
        }
    }

    rate = *rate_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portStormControlBurstSize_set(unit, port, type, rate), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_burst_rate_rate */
#endif

#ifdef CMD_STORM_CONTROL_SET_BROADCAST_MULTICAST_UNICAST_PORT_ALL_BURST_SIZE_SIZE
/*
 * storm-control set ( broadcast | multicast | unicast ) ( <PORT_LIST:port> | all ) burst-size <UINT:size>
 */
cparser_result_t cparser_cmd_storm_control_set_broadcast_multicast_unicast_port_all_burst_size_size(cparser_context_t *context, uint32_t *rate_ptr)
{
    uint32      unit = 0;
    uint32      rate = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_rate_storm_group_t type = STORM_GROUP_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_BROADCAST;
    }
    else if ('m' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else if ('u' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_UNICAST;
    }

    rate = *rate_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portStormControlBurstSize_set(unit, port, type, rate), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_SET_UNICAST_MULTICAST_PORT_ALL_TYPE_UNKNOWN_ONLY_BOTH
/*
 * storm-control set ( unicast | multicast ) ( <PORT_LIST:port> | all ) type ( unknown-only | both )
 */
cparser_result_t cparser_cmd_storm_control_set_unicast_multicast_port_all_type_unknown_only_both(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_storm_group_t  type;
    rtk_rate_storm_sel_t    sel;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('u' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_UNICAST;
    }
    else
    {
        type = STORM_GROUP_MULTICAST;
    }

    if ('b' == TOKEN_CHAR(5,0))
    {
        sel = STORM_SEL_UNKNOWN_AND_KNOWN;
    }
    else
    {
        sel = STORM_SEL_UNKNOWN;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlTypeSel_set(unit, port, type, sel), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_SET_BROADCAST_MULTICAST_UNKNOWN_MULTICAST_UNKNOWN_UNICAST_PORT_ALL_COUNTING_MODE_BYTE_PACKET
/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) ( <PORT_LIST:port> | all ) counting-mode ( byte | packet )
 */
cparser_result_t cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_counting_mode_byte_packet(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_storm_group_t  type;
    rtk_rate_storm_rateMode_t   mode;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_BROADCAST;
    }
    else if ('m' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else
    {
        if ('m' == TOKEN_CHAR(2,8))
        {
            type = STORM_GROUP_UNKNOWN_MULTICAST;
        }
        else
        {
            type = STORM_GROUP_UNKNOWN_UNICAST;
        }
    }

    if ('b' == TOKEN_CHAR(5,0))
    {
        mode = BASED_ON_BYTE;
    }
    else
    {
        mode = BASED_ON_PKT;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_portStormControlRateMode_set(unit, port, type, mode), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_counting_mode_byte_packet */
#endif

#ifdef CMD_STORM_CONTROL_SET_BROADCAST_MULTICAST_UNKNOWN_MULTICAST_UNKNOWN_UNICAST_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_storm_group_t  type;
    rtk_enable_t    enable = DISABLED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_BROADCAST;
    }
    else if ('m' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else
    {
        if ('m' == TOKEN_CHAR(2,8))
        {
            type = STORM_GROUP_UNKNOWN_MULTICAST;
        }
        else
        {
            type = STORM_GROUP_UNKNOWN_UNICAST;
        }
    }

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlEnable_set(unit, port, type, enable), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_state_disable_enable */
#endif

#ifdef CMD_STORM_CONTROL_SET_BROADCAST_MULTICAST_UNKNOWN_MULTICAST_UNKNOWN_UNICAST_PORT_ALL_RATE_RATE
/*
 * storm-control set ( broadcast | multicast | unknown-multicast | unknown-unicast ) ( <PORT_LIST:port> | all ) rate <UINT:rate>
 */
cparser_result_t cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_rate_rate(cparser_context_t *context,
    char **port_ptr,
    uint32_t *rate_ptr)
{
    uint32      unit = 0;
    uint32      rate = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_storm_group_t  type;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_BROADCAST;
    }
    else if ('m' == TOKEN_CHAR(2,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else
    {
        if ('m' == TOKEN_CHAR(2,8))
        {
            type = STORM_GROUP_UNKNOWN_MULTICAST;
        }
        else
        {
            type = STORM_GROUP_UNKNOWN_UNICAST;
        }
    }

    rate = *rate_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlRate_set(unit, port, type, rate), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_storm_control_set_broadcast_multicast_unknown_multicast_unknown_unicast_port_all_rate_rate */
#endif

#ifdef CMD_STORM_CONTROL_SET_ARP_BPDU_IGMP_PORT_ALL_RATE_RATE
/*
 * storm-control set ( arp | bpdu | igmp ) ( <PORT_LIST:port> | all ) rate <UINT:rate>
 */
cparser_result_t cparser_cmd_storm_control_set_arp_bpdu_igmp_port_all_rate_rate(cparser_context_t *context,
    uint32_t *rate_ptr)
{
    uint32      unit = 0;
    uint32      rate = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_storm_proto_group_t  type = STORM_PROTO_GROUP_END;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('b' == TOKEN_CHAR(2,0))
    {
        type = STORM_PROTO_GROUP_BPDU;
    }
    else if ('i' == TOKEN_CHAR(2,0))
    {
        type = STORM_PROTO_GROUP_IGMP;
    }
    else if ('a' == TOKEN_CHAR(2,0))
    {
        type = STORM_PROTO_GROUP_ARP;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    rate = *rate_ptr;
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlProtoRate_set(unit, port, type, rate), ret);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_SET_BYPASS_PACKET_ARP_REQUEST_BPDU_IGMP_RMA_RTK_CTRL_PKT_STATE_DISABLE_ENABLE
/*
 * storm-control set bypass ( arp | bpdu | igmp | rma | rtk-ctrl-pkt ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_storm_control_set_bypass_packet_arp_request_bpdu_igmp_rma_rtk_ctrl_pkt_state_disable_enable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_rate_storm_bypassType_t bypassType = IGR_BYPASS_RMA;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('m' == TOKEN_CHAR(3,1))
    {
        bypassType = STORM_BYPASS_RMA;
    }
    else if ('r' == TOKEN_CHAR(3,1))
    {
        bypassType = STORM_BYPASS_ARP;
    }
    else if ('p' == TOKEN_CHAR(3,1))
    {
        bypassType = STORM_BYPASS_BPDU;
    }
    else if ('t' == TOKEN_CHAR(3,1))
    {
        bypassType = STORM_BYPASS_RTKPKT;
    }
    else if ('g' == TOKEN_CHAR(3,1))
    {
        bypassType = STORM_BYPASS_IGMP;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_rate_stormControlBypass_set(unit, bypassType, enable), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_STORM_CONTROL_SET_TYPE_UNICAST_MULTICAST_PORT_ALL_UNKNOWN_ONLY_BOTH
/*
 * storm-control set type ( unicast | multicast ) ( <PORT_LIST:port> | all ) ( unknown-only | both )
 */
cparser_result_t cparser_cmd_storm_control_set_type_unicast_multicast_port_all_unknown_only_both(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_rate_storm_group_t  type;
    diag_portlist_t portlist;
    rtk_rate_storm_sel_t  storm_sel;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('u' == TOKEN_CHAR(3,0))
    {
        type = STORM_GROUP_UNICAST;
    }
    else if ('m' == TOKEN_CHAR(3,0))
    {
        type = STORM_GROUP_MULTICAST;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    if ('u' == TOKEN_CHAR(5,0))
    {
        storm_sel = STORM_SEL_UNKNOWN;
    }
    else if ('b' == TOKEN_CHAR(5,0))
    {
        storm_sel = STORM_SEL_UNKNOWN_AND_KNOWN;
    }
    else
    {
        diag_util_printf("User config : Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_rate_stormControlTypeSel_set(unit, port, type, storm_sel), ret);
    }

    return CPARSER_OK;
}
#endif
