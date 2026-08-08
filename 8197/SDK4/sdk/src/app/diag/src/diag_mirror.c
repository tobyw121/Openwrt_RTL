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
 * Purpose : Definition those mirror command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) port mirror
 *           2) rspan
 *           3) sflow
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
#include <rtk/mirror.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_MIRROR_CREATE_DESTROY_MIRRORING_PORT_ID
/*
 * mirror ( create | destroy ) mirroring <UINT:port_id>
 */
cparser_result_t cparser_cmd_mirror_create_destroy_mirroring_port_id(cparser_context_t *context,
    uint32_t *port_id_ptr)
{
    uint32     unit = 0;
    int32      ret = RT_ERR_FAILED;
    rtk_port_t mirroring_port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    mirroring_port = *port_id_ptr;
    if ('c' == TOKEN_CHAR(1,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_portBased_create(unit, mirroring_port), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_portBased_destroy(unit, mirroring_port), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_mirror_create_destroy_mirroring_port_id */
#endif /* CMD_MIRROR_CREATE_DESTROY_MIRRORING_PORT_ID */

#ifdef CMD_MIRROR_DESTROY_ALL
/*
 * mirror destroy all
 */
cparser_result_t cparser_cmd_mirror_destroy_all(cparser_context_t *context)
{
    uint32     unit = 0;
    int32      ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(rtk_mirror_portBased_destroyAll(unit), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_mirror_destroy_all */
#endif /*CMD_MIRROR_DESTROY_ALL*/

#ifdef CMD_MIRROR_GET_EGRESS_MODE_PORT_ALL
/*
 * mirror get egress-mode ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_mirror_get_egress_mode_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_egrMode_t    mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_egrMode_get(unit, port, &mode), ret);
        if (FORWARD_ALL_PKTS == mode)
            diag_util_mprintf("Port %2d : all-pkt\n", port);
        else
            diag_util_mprintf("Port %2d : mirrored-only\n", port);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_mirror_get_egress_mode_port_all */
#endif /* CMD_MIRROR_GET_EGRESS_MODE_PORT_ALL */

#ifdef CMD_MIRROR_GET_MIRRORING_PORT_ID
/*
 * mirror get mirroring <UINT:port_id>
 */
cparser_result_t cparser_cmd_mirror_get_mirroring_port_id(cparser_context_t *context,
    uint32_t *port_id_ptr)
{
    uint32     unit = 0;
    int32      ret = RT_ERR_FAILED;
    rtk_port_t mirroring_port = 0;
    rtk_portmask_t  rx_portmask, tx_portmask;
    uint8   rxPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8   txPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    mirroring_port = *port_id_ptr;
    DIAG_UTIL_ERR_CHK(rtk_mirror_portBased_get(unit, mirroring_port, &rx_portmask, &tx_portmask), ret);
    memset(rxPortList, 0, DIAG_UTIL_PORT_MASK_STRING_LEN * sizeof(uint8));
    memset(txPortList, 0, DIAG_UTIL_PORT_MASK_STRING_LEN * sizeof(uint8));
    diag_util_lPortMask2str(rxPortList, &rx_portmask);
    diag_util_lPortMask2str(txPortList, &tx_portmask);

    diag_util_mprintf("\tMirroring port:%d\n", mirroring_port);
    diag_util_mprintf("\tRx-mirrored-port: %s\n", rxPortList);
    diag_util_mprintf("\tTx-mirrored-port: %s\n", txPortList);
    return CPARSER_OK;
} /* end of cparser_cmd_mirror_get_mirroring_port_id */
#endif /* CMD_MIRROR_GET_MIRRORING_PORT_ID */

#ifdef CMD_MIRROR_GET_INDEX
/*
 * mirror get <UINT:index>
 */
cparser_result_t cparser_cmd_mirror_get_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_mirror_entry_t  mirror_entry;
    uint8   rxPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8   txPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    memset(&mirror_entry, 0, sizeof(rtk_mirror_entry_t));

    index = *index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_mirror_group_get(unit, index, &mirror_entry), ret);

    diag_util_mprintf("Mirroring Index : %d\n", index);
    if (mirror_entry.mirroring_port == 31)
        diag_util_mprintf("\tMirroring Port         : Invalid\n");
    else
        diag_util_mprintf("\tMirroring Port         : %d\n", mirror_entry.mirroring_port);
    diag_util_lPortMask2str(rxPortList, &mirror_entry.mirrored_igrPorts);
    diag_util_mprintf("\tMirroring Ingress Port : %s\n", rxPortList);
    diag_util_lPortMask2str(txPortList, &mirror_entry.mirrored_egrPorts);
    diag_util_mprintf("\tMirroring Egress Port  : %s\n", txPortList);
    diag_util_mprintf("\tOper of Igr & Egr Port : %s\n", (mirror_entry.oper_of_igr_and_egr_ports == 0)?"OR":"AND");
    diag_util_mprintf("\tCross vlan             : %s\n", (mirror_entry.cross_vlan == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror Ucast Packet    : %s\n", (mirror_entry.mirror_ucast == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror Mcast Packet    : %s\n", (mirror_entry.mirror_mcast == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror Bcast Packet    : %s\n", (mirror_entry.mirror_bcast == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror Good Packet     : %s\n", (mirror_entry.mirror_goodPkt == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror Bad Packet      : %s\n", (mirror_entry.mirror_badPkt == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror Original Packet : %s\n", (mirror_entry.mirror_orginalPkt == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror Flow-based Only : %s\n", (mirror_entry.flowBasedOnly == ENABLED)?"ENABLED":"DISABLED");

    return CPARSER_OK;
} /* end of cparser_cmd_mirror_get_index */
#endif /* CMD_MIRROR_GET_INDEX */

#ifdef CMD_MIRROR_SET_EGRESS_MODE_PORT_ALL_ALL_PKT_MIRRORED_ONLY
/*
 * mirror set egress-mode ( <PORT_LIST:port> | all ) ( all-pkt | mirrored-only )
 */
cparser_result_t cparser_cmd_mirror_set_egress_mode_port_all_all_pkt_mirrored_only(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_egrMode_t    mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('a' == TOKEN_CHAR(4,0))
        mode = FORWARD_ALL_PKTS;
    else
        mode = FORWARD_MIRRORED_PKTS_ONLY;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_egrMode_set(unit, port, mode), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_mirror_set_egress_mode_port_all_all_pkt_mirrored_only */
#endif /* CMD_MIRROR_SET_EGRESS_MODE_PORT_ALL_ALL_PKT_MIRRORED_ONLY */

#ifdef CMD_MIRROR_SET_INGRESS_EGRESS_MIRRORING_PORT_ID_MIRRORED_PORT
/*
 * mirror set ( ingress | egress ) mirroring <UINT:port_id> mirrored <PORT_LIST:port>
 */
cparser_result_t cparser_cmd_mirror_set_ingress_egress_mirroring_port_id_mirrored_port(cparser_context_t *context,
    uint32_t *port_id_ptr, char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      mirroring_port = 0;
    rtk_portmask_t  mirrored_portmask, rx_portmask, tx_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    memset(&mirrored_portmask, 0, sizeof(rtk_portmask_t));
    memset(&rx_portmask, 0, sizeof(rtk_portmask_t));
    memset(&tx_portmask, 0, sizeof(rtk_portmask_t));
        
    mirroring_port = *port_id_ptr;
    DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask(TOKEN_STR(6), &mirrored_portmask), ret);
    DIAG_UTIL_ERR_CHK(rtk_mirror_portBased_get(unit, mirroring_port, &rx_portmask, &tx_portmask), ret);

    if ('i' == TOKEN_CHAR(2,0))
    {   /* ingress mirror */
        memcpy(&rx_portmask, &mirrored_portmask, sizeof(rtk_portmask_t));
    }
    else if ('e' == TOKEN_CHAR(2,0))
    {   /* egress mirror */
         memcpy(&tx_portmask, &mirrored_portmask, sizeof(rtk_portmask_t));
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_mirror_portBased_set(unit, mirroring_port, &rx_portmask, &tx_portmask), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_mirror_set_ingress_egress_mirroring_port_id_mirrored_port */
#endif /* CMD_MIRROR_SET_INGRESS_EGRESS_MIRRORING_PORT_ID_MIRRORED_PORT */

#ifdef CMD_MIRROR_SET_INDEX_MIRRORING_PORT_ID_INGRESS_MIRRORED_INGRESS_PORT_NONE_EGRESS_MIRRORED_EGRESS_PORT_NONE_IGR_AND_EGR_CROSS_VLAN_UCAST_MCAST_BCAST_GOOD_PKT_BAD_PKT_ORIGINAL_PKT_FLOW_BASED_ONLY
/*
 * mirror set <UINT:index> mirroring <UINT:port_id> ingress-mirrored ( <PORT_LIST:ingress_port> | none ) egress-mirrored ( <PORT_LIST:egress_port> | none ) { igr-and-egr } { cross-vlan } { ucast } { mcast } { bcast } { good-pkt } { bad-pkt } { original-pkt } { flow-based-only }
 */
cparser_result_t cparser_cmd_mirror_set_index_mirroring_port_id_ingress_mirrored_ingress_port_none_egress_mirrored_egress_port_none_igr_and_egr_cross_vlan_ucast_mcast_bcast_good_pkt_bad_pkt_original_pkt_flow_based_only(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *port_id_ptr,
    char **ingress_port_ptr,
    char **egress_port_ptr)
{
    uint32          unit = 0;
    uint32          index = 0, i;
    int32           ret = RT_ERR_FAILED;
    rtk_mirror_entry_t  mirror_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    memset(&mirror_entry, 0, sizeof(rtk_mirror_entry_t));

    index = *index_ptr;
    mirror_entry.mirroring_port = *port_id_ptr;
    if ('n' != TOKEN_CHAR(6,0))
        DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask((uint8 *)TOKEN_STR(6), &mirror_entry.mirrored_igrPorts), ret);
    if ('n' != TOKEN_CHAR(8,0))
        DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask((uint8 *)TOKEN_STR(8), &mirror_entry.mirrored_egrPorts), ret);

    for (i = 9; i < TOKEN_NUM; i++)
    {
        if ('b' == TOKEN_CHAR(i,0))
        {
            if ('a' == TOKEN_CHAR(i,1))
                mirror_entry.mirror_badPkt = ENABLED;
            else if ('c' == TOKEN_CHAR(i,1))
                mirror_entry.mirror_bcast = ENABLED;
            else
            {
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }
        }
        else if ('c' == TOKEN_CHAR(i,0))
            mirror_entry.cross_vlan = ENABLED;
        else if ('f' == TOKEN_CHAR(i,0))
            mirror_entry.flowBasedOnly = ENABLED;
        else if ('g' == TOKEN_CHAR(i,0))
            mirror_entry.mirror_goodPkt = ENABLED;
        else if ('i' == TOKEN_CHAR(i,0))
            mirror_entry.oper_of_igr_and_egr_ports = 1;
        else if ('m' == TOKEN_CHAR(i,0))
            mirror_entry.mirror_mcast = ENABLED;
        else if ('o' == TOKEN_CHAR(i,0))
            mirror_entry.mirror_orginalPkt = ENABLED;
        else if ('u' == TOKEN_CHAR(i,0))
            mirror_entry.mirror_ucast = ENABLED;
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_mirror_group_set(unit, index, &mirror_entry), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_mirror_set_index_mirroring_port_id_ingress_mirrored_ingress_port_none_egress_mirrored_egress_port_none_igr_and_egr_cross_vlan_ucast_mcast_bcast_good_pkt_bad_pkt_original_pkt_flow_based_only */
#endif /* CMD_MIRROR_SET_INDEX_MIRRORING_PORT_ID_INGRESS_MIRRORED_INGRESS_PORT_NONE_EGRESS_MIRRORED_EGRESS_PORT_NONE_IGR_AND_EGR_CROSS_VLAN_UCAST_MCAST_BCAST_GOOD_PKT_BAD_PKT_ORIGINAL_PKT_FLOW_BASED_ONLY */

#ifdef CMD_MIRROR_CLEAR_INDEX
/* 
 * mirror clear <UINT:index>
 */
cparser_result_t cparser_cmd_mirror_clear_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_mirror_entry_t  mirror_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    memset(&mirror_entry, 0, sizeof(rtk_mirror_entry_t));

    index = *index_ptr;
    mirror_entry.mirroring_port = 31; /* invalid */
    DIAG_UTIL_ERR_CHK(rtk_mirror_group_set(unit, index, &mirror_entry), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_mirror_clear_index */
#endif /* CMD_MIRROR_CLEAR_INDEX */

#ifdef CMD_RSPAN_GET_EGRESS_MODE_PORT_ALL
/*
 * rspan get egress-mode ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_rspan_get_egress_mode_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanEgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_portRspanEgrMode_get(unit, port, &mode), ret);
        if (RSPAN_EGR_REMOVE_TAG == mode)
            diag_util_mprintf("Port %2d : remove tag\n", port);
        else if (RSPAN_EGR_ADD_TAG == mode)
            diag_util_mprintf("Port %2d : add tag\n", port);
        else
            diag_util_mprintf("Port %2d : no modify\n", port);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_get_egress_mode_port_all */
#endif /* CMD_RSPAN_GET_EGRESS_MODE_PORT_ALL */

#ifdef CMD_RSPAN_GET_EGRESS_TAG_PORT_ALL
/*
 * rspan get egress-tag ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_rspan_get_egress_tag_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanEgrTag_t   tag;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_rspanEgrTag_get(unit, port, &tag), ret);
        diag_util_mprintf("Port %2d :\n", port);
        diag_util_mprintf("\tTPID : 0x%x\n", tag.tpid);
        diag_util_mprintf("\tVID  : %d\n", tag.vid);
        diag_util_mprintf("\tPRI  : %d\n", tag.pri);
        diag_util_mprintf("\tCFI  : %d\n", tag.cfi);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_get_egress_tag_port_all */
#endif /* CMD_RSPAN_GET_EGRESS_TAG_PORT_ALL */

#ifdef CMD_RSPAN_GET_INGRESS_MODE_PORT_ALL
/*
 * rspan get ingress-mode ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_rspan_get_ingress_mode_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanIgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_portRspanIgrMode_get(unit, port, &mode), ret);
        if (RSPAN_IGR_HANDLE_RSPAN_TAG == mode)
            diag_util_mprintf("Port %2d : handle rspan tag\n", port);
        else
            diag_util_mprintf("Port %2d : ignore rspan tag\n", port);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_get_ingress_mode_port_all */
#endif /* CMD_RSPAN_GET_INGRESS_MODE_PORT_ALL */

#ifdef CMD_RSPAN_GET_INGRESS_TAG_PORT_ALL
/*
 * rspan get ingress-tag ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_rspan_get_ingress_tag_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanIgrTag_t   tag;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_rspanIgrTag_get(unit, port, &tag), ret);
        diag_util_mprintf("Port %2d :\n", port);
        diag_util_mprintf("\tTPID : 0x%x\n", tag.tpid);
        diag_util_mprintf("\tVID  : %d\n", tag.vid);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_get_ingress_tag_port_all */
#endif /* CMD_RSPAN_GET_INGRESS_TAG_PORT_ALL */

#ifdef CMD_RSPAN_SET_EGRESS_MODE_PORT_ALL_ADD_TAG_NO_MODIFY_REMOVE_TAG
/*
 * rspan set egress-mode ( <PORT_LIST:port> | all ) ( add-tag | no-modify | remove-tag )
 */
cparser_result_t cparser_cmd_rspan_set_egress_mode_port_all_add_tag_no_modify_remove_tag(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanEgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('a' == TOKEN_CHAR(4,0))
        mode = RSPAN_EGR_ADD_TAG;
    else if ('n' == TOKEN_CHAR(4,0))
        mode = RSPAN_EGR_NO_MODIFY;
    else if ('r' == TOKEN_CHAR(4,0))
        mode = RSPAN_EGR_REMOVE_TAG;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_portRspanEgrMode_set(unit, port, mode), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_set_egress_mode_port_all_add_tag_no_modify_remove_tag */
#endif /* CMD_RSPAN_SET_EGRESS_MODE_PORT_ALL_ADD_TAG_NO_MODIFY_REMOVE_TAG */

#ifdef CMD_RSPAN_SET_EGRESS_TAG_PORT_ALL_TPID_VID_PRI_CFI
/*
 * rspan set egress-tag ( <PORT_LIST:port> | all ) <UINT:tpid> <UINT:vid> <UINT:pri> <UINT:cfi>
 */
cparser_result_t cparser_cmd_rspan_set_egress_tag_port_all_tpid_vid_pri_cfi(cparser_context_t *context,
    char **port_ptr,
    uint32_t *tpid_ptr,
    uint32_t *vid_ptr,
    uint32_t *pri_ptr,
    uint32_t *cfi_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanEgrTag_t   tag;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    tag.tpid = *tpid_ptr;
    tag.vid = *vid_ptr;
    tag.pri = *pri_ptr;
    tag.cfi = *cfi_ptr;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_rspanEgrTag_set(unit, port, &tag), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_set_egress_tag_port_all_tpid_vid_pri_cfi */
#endif /* CMD_RSPAN_SET_EGRESS_TAG_PORT_ALL_TPID_VID_PRI_CFI */

#ifdef CMD_RSPAN_SET_INGRESS_MODE_PORT_ALL_HANDLE_TAG_IGNORE_TAG
/*
 * rspan set ingress-mode ( <PORT_LIST:port> | all ) ( handle-tag | ignore-tag )
 */
cparser_result_t cparser_cmd_rspan_set_ingress_mode_port_all_handle_tag_ignore_tag(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanIgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('h' == TOKEN_CHAR(4,0))
        mode = RSPAN_IGR_HANDLE_RSPAN_TAG;
    else if ('i' == TOKEN_CHAR(4,0))
        mode = RSPAN_IGR_IGNORE_RSPAN_TAG;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
          
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_portRspanIgrMode_set(unit, port, mode), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_set_ingress_mode_port_all_handle_tag_ignore_tag */
#endif /* CMD_RSPAN_SET_INGRESS_MODE_PORT_ALL_HANDLE_TAG_IGNORE_TAG */

#ifdef CMD_RSPAN_SET_INGRESS_TAG_PORT_ALL_TPID_VID
/*
 * rspan set ingress-tag ( <PORT_LIST:port> | all ) <UINT:tpid> <UINT:vid>
 */
cparser_result_t cparser_cmd_rspan_set_ingress_tag_port_all_tpid_vid(cparser_context_t *context,
    char **port_ptr,
    uint32_t *tpid_ptr,
    uint32_t *vid_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_mirror_rspanIgrTag_t   tag;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    tag.tpid = *tpid_ptr;
    tag.vid = *vid_ptr;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_rspanIgrTag_set(unit, port, &tag), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_rspan_set_ingress_tag_port_all_tpid_vid */
#endif /* CMD_RSPAN_SET_INGRESS_TAG_PORT_ALL_TPID_VID */

#ifdef CMD_SFLOW_GET_CPU_TAG_STATE
/*
 * sflow get cpu-tag
 */
cparser_result_t cparser_cmd_sflow_get_cpu_tag_state(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowAddCPUTagEnable_get(unit, &enable), ret);
    diag_util_mprintf("CPU tag : %s\n", (ENABLED == enable)?"ENABLED":"DISABLED");

    return CPARSER_OK;
} /* end of cparser_cmd_sflow_get_cpu_tag_state */
#endif

#ifdef CMD_SFLOW_GET_EGRESS_INGRESS_PORT_ALL
/*
 * sflow get ( egress | ingress ) ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_sflow_get_egress_ingress_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    uint32          rate = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_OUTPUT_INIT();

    if ('e' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_mprintf("Port %2d :\n", port);
            if (RT_ERR_OK == rtk_mirror_sflowPortEgrSampleEnable_get(unit, port, &enable))
            {
                diag_util_mprintf("\tEgress Sample Status : %s\n", (ENABLED == enable)?"ENABLED":"DISABLED");
            }

            if (RT_ERR_OK == rtk_mirror_sflowPortEgrSampleRate_get(unit, port, &rate))
            {
                diag_util_mprintf("\tEgress Sample Rate   : %d (0x%x)\n", rate, rate);
            }
        }
    }
    else
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            diag_util_mprintf("Port %2d :\n", port);
            if (RT_ERR_OK == rtk_mirror_sflowPortIgrSampleEnable_get(unit, port, &enable))
            {
                diag_util_mprintf("\tIngress Sample Status : %s\n", (ENABLED == enable)?"ENABLED":"DISABLED");
            }

            if (RT_ERR_OK == rtk_mirror_sflowPortIgrSampleRate_get(unit, port, &rate))
            {
                diag_util_mprintf("\tIngress Sample Rate   : %d (0x%x)\n", rate, rate);
            }
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_sflow_get_egress_ingress_port_all */
#endif

#ifdef CMD_SFLOW_GET_MIRROR_GROUP_INDEX
/*
 * sflow get mirror-group <UINT:index>
 */
cparser_result_t cparser_cmd_sflow_get_mirror_group_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    uint32          rate = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    index = *index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowMirrorSampleEnable_get(unit, index, &enable), ret);
    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowMirrorSampleRate_get(unit, index, &rate), ret);
    diag_util_mprintf("Mirror Index %2d :\n", index);
    diag_util_mprintf("\tSample Status : %s\n", (ENABLED == enable)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tSample Rate   : %d (0x%x)\n", rate, rate);

    return CPARSER_OK;
} /* end of cparser_cmd_sflow_get_mirror_group_index */
#endif

#ifdef CMD_SFLOW_GET_MIRROR_GROUP_PORT_SEED
/*
 * sflow get ( mirror-group | port )
 */
cparser_result_t cparser_cmd_sflow_get_mirror_group_port_seed(cparser_context_t *context)
{
    uint32          unit = 0;
    uint32          seed_value = 0;
    int32           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ('m' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_sflowMirrorSeed_get(unit, &seed_value), ret);
        diag_util_mprintf("Mirror group seed value : %d (0x%x)\n", seed_value, seed_value);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_sflowPortSeed_get(unit, &seed_value), ret);
        diag_util_mprintf("Port seed value : %d (0x%x)\n", seed_value, seed_value);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_sflow_get_mirror_group_port_seed */
#endif

#ifdef CMD_SFLOW_SET_CPU_TAG_STATE_DISABLE_ENABLE
/*
 * sflow set cpu-tag state ( disable | enable )
 */
cparser_result_t cparser_cmd_sflow_set_cpu_tag_state_disable_enable(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowAddCPUTagEnable_set(unit, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_sflow_set_cpu_tag_state_disable_enable */
#endif

#ifdef CMD_SFLOW_SET_EGRESS_INGRESS_PORT_ALL_SAMPLE_STATE_DISABLE_ENABLE
/*
 * sflow set ( egress | ingress ) ( <PORT_LIST:port> | all ) sample state ( disable | enable )
 */
cparser_result_t cparser_cmd_sflow_set_egress_ingress_port_all_sample_state_disable_enable(cparser_context_t *context,
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

    if ('e' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_mirror_sflowPortEgrSampleEnable_set(unit, port, enable), ret);
        }
    }
    else
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_mirror_sflowPortIgrSampleEnable_set(unit, port, enable), ret);
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_sflow_set_egress_ingress_port_all_sample_state_disable_enable */
#endif

#ifdef CMD_SFLOW_SET_EGRESS_INGRESS_PORT_ALL_SAMPLE_RATE
/*
 * sflow set ( egress | ingress ) ( <PORT_LIST:port> | all ) sample <UINT:rate>
 */
cparser_result_t cparser_cmd_sflow_set_egress_ingress_port_all_sample_rate(cparser_context_t *context,
    char **port_ptr,
    uint32_t *rate_ptr)
{
    uint32          unit = 0;
    uint32          rate = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    rate = *rate_ptr;

    if ('e' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_mirror_sflowPortEgrSampleRate_set(unit, port, rate), ret);
        }
    }
    else
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(rtk_mirror_sflowPortIgrSampleRate_set(unit, port, rate), ret);
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_sflow_set_egress_ingress_port_all_sample_rate */
#endif

#ifdef CMD_SFLOW_SET_MIRROR_GROUP_INDEX_SAMPLE_STATE_DISABLE_ENABLE
/*
 * sflow set mirror-group <UINT:index> sample state ( disable | enable )
 */
cparser_result_t cparser_cmd_sflow_set_mirror_group_index_sample_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    if ('e' == TOKEN_CHAR(6,0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowMirrorSampleEnable_set(unit, index, enable), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_sflow_set_mirror_group_index_sample_state_disable_enable */
#endif

#ifdef CMD_SFLOW_SET_MIRROR_GROUP_INDEX_SAMPLE_RATE
/*
 * sflow set mirror-group <UINT:index> sample <UINT:rate>
 */
cparser_result_t cparser_cmd_sflow_set_mirror_group_index_sample_rate(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *rate_ptr)
{
    uint32          unit = 0;
    uint32          index = 0, rate = 0;
    int32           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;
    rate = *rate_ptr;
    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowMirrorSampleRate_set(unit, index, rate), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_sflow_set_mirror_group_index_sample_rate */
#endif

#ifdef CMD_SFLOW_SET_MIRROR_GROUP_PORT_SEED_SEED_VALUE
/*
 * sflow set ( mirror-group | port ) seed <UINT:seed_value>
 */
cparser_result_t cparser_cmd_sflow_set_mirror_group_port_seed_seed_value(cparser_context_t *context,
    uint32_t *seed_value_ptr)
{
    uint32          unit = 0;
    uint32          seed_value = 0;
    int32           ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    seed_value = *seed_value_ptr;

    if ('m' == TOKEN_CHAR(2,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_sflowMirrorSeed_set(unit, seed_value), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_mirror_sflowPortSeed_set(unit, seed_value), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_sflow_set_mirror_group_port_seed_seed_value */
#endif

#ifdef CMD_MIRROR_GET_MIRROR_ID_INDEX
/*
 * mirror get <UINT:index>
 */
cparser_result_t cparser_cmd_mirror_get_mirror_id_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_mirror_entry_t  mirror_entry;
    uint8   rxPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];
    uint8   txPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    memset(&mirror_entry, 0, sizeof(rtk_mirror_entry_t));

    index = *index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_mirror_group_get(unit, index, &mirror_entry), ret);

    diag_util_mprintf("Mirror Index : %d\n", index);
    diag_util_mprintf("\tMirror Enable : %s\n", (mirror_entry.mirror_enable== ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirroring Port         : %d\n", mirror_entry.mirroring_port);
    diag_util_lPortMask2str(rxPortList, &mirror_entry.mirrored_igrPorts);
    diag_util_mprintf("\tMirroring Ingress Port : %s\n", rxPortList);
    diag_util_lPortMask2str(txPortList, &mirror_entry.mirrored_egrPorts);
    diag_util_mprintf("\tMirroring Egress Port  : %s\n", txPortList);
    diag_util_mprintf("\tOper of Igr & Egr Port : %s\n", (mirror_entry.oper_of_igr_and_egr_ports == 0)?"OR":"AND");
    diag_util_mprintf("\tMirror Original Packet : %s\n", (mirror_entry.mirror_orginalPkt == ENABLED)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirroring Port TX Packets : %s\n", (mirror_entry.mirroring_port_egrMode == FORWARD_MIRRORED_PKTS_ONLY)?"Mirrored Packets Only":"All Packets");
#if defined(CONFIG_SDK_RTL8380)
    diag_util_mprintf("\tMirror MODE setting : %s\n", (mirror_entry.mir_mode == 0)?"Normal mirror":"follow mirroring port setting");
    diag_util_mprintf("\tMirror cross VLAN : %s\n", (mirror_entry.cross_vlan == 1)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror flow based only : %s\n", (mirror_entry.flowBasedOnly == 1)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tFlow based mirror ignore portmask : %s\n", (mirror_entry.flowBased_pmsk_ignore == 1)?"ENABLED":"DISABLED");
    diag_util_mprintf("\tMirror duplicate filter : %s\n", (mirror_entry.duplicate_fltr == 1)?"Filter":"NONE");
    diag_util_mprintf("\tMirror self filter : %s\n", (mirror_entry.self_flter == 1)?"Filter":"NONE");
#endif


    return CPARSER_OK;
} /* end of cparser_cmd_mirror_get_index */
#endif

#ifdef CMD_MIRROR_SET_MIRROR_ID_INDEX_MIRRORING_PORT_ID_INGRESS_MIRRORED_INGRESS_PORT_NONE_EGRESS_MIRRORED_EGRESS_PORT_NONE_IGR_AND_EGR_MIRRORED_ONLY_ORIGINAL_PKT
cparser_result_t cparser_cmd_mirror_set_mirror_id_index_mirroring_port_id_ingress_mirrored_ingress_port_none_egress_mirrored_egress_port_none_igr_and_egr_mirrored_only_original_pkt(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *port_id_ptr,
    char **ingress_port_ptr,
    char **egress_port_ptr)
{
    uint32          unit = 0;
    uint32          index = 0, i;
    int32           ret = RT_ERR_FAILED;
    rtk_mirror_entry_t  mirror_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&mirror_entry, 0, sizeof(rtk_mirror_entry_t));

    index = *index_ptr;
    mirror_entry.mirroring_port = *port_id_ptr;

    if ('n' != TOKEN_CHAR(7,0))
        DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask((uint8 *)TOKEN_STR(7), &mirror_entry.mirrored_igrPorts), ret);

    if ('n' != TOKEN_CHAR(9,0))
        DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask((uint8 *)TOKEN_STR(9), &mirror_entry.mirrored_egrPorts), ret);

    for (i = 10; i < TOKEN_NUM; i++)
    {
        if ('i' == TOKEN_CHAR(i,0))
            mirror_entry.oper_of_igr_and_egr_ports = 1;
        else if ('o' == TOKEN_CHAR(i,0))
            mirror_entry.mirror_orginalPkt = ENABLED;
        else if ('m' == TOKEN_CHAR(i,0))
            mirror_entry.mirroring_port_egrMode = FORWARD_MIRRORED_PKTS_ONLY;
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    mirror_entry.mirror_enable = ENABLED;

    DIAG_UTIL_ERR_CHK(rtk_mirror_group_set(unit, index, &mirror_entry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_MIRROR_SET_STATE_INDEX_DISABLE_ENABLE
cparser_result_t cparser_cmd_mirror_set_state_index_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_mirror_entry_t  mirror_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    index = *index_ptr;

    DIAG_UTIL_ERR_CHK(rtk_mirror_group_get(unit, index, &mirror_entry), ret);

    if ('e' == TOKEN_CHAR(4,0))
    {
         mirror_entry.mirror_enable = ENABLED;
    }
    else
    {
         mirror_entry.mirror_enable = DISABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_mirror_group_set(unit, index, &mirror_entry), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_MIRROR_GET_STATE_INDEX
cparser_result_t cparser_cmd_mirror_get_state_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  index = 0;
    rtk_mirror_entry_t  mirror_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_mirror_group_get(unit, index, &mirror_entry), ret);

    diag_util_mprintf("Mirror Index : %d\n", index);
    diag_util_mprintf("\tMirror Enable : %s\n", (mirror_entry.mirror_enable== ENABLED)?"ENABLED":"DISABLED");

    return CPARSER_OK;
}
#endif

#ifdef CMD_RSPAN_GET_EGRESS_MODE_MIRROR_ID_INDEX
cparser_result_t cparser_cmd_rspan_get_egress_mode_mirror_id_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_mirror_rspanEgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_mirror_rspanEgrMode_get(unit, *index_ptr, &mode), ret);
    if (RSPAN_EGR_REMOVE_TAG == mode)
        diag_util_mprintf("Mirror Entry %d : remove tag\n", *index_ptr);
    else if (RSPAN_EGR_ADD_TAG == mode)
        diag_util_mprintf("Mirror Entry %d : add tag\n", *index_ptr);
    else
        diag_util_mprintf("Mirror Entry %d : no modify\n", *index_ptr);

    return CPARSER_OK;
}
#endif

#ifdef CMD_RSPAN_GET_INGRESS_MODE_MIRROR_ID_INDEX
cparser_result_t cparser_cmd_rspan_get_ingress_mode_mirror_id_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_mirror_rspanIgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_mirror_rspanIgrMode_get(unit, *index_ptr, &mode), ret);
    if (RSPAN_IGR_HANDLE_RSPAN_TAG == mode)
        diag_util_mprintf("Mirror Entry %d : handle rspan tag\n", *index_ptr);
    else
        diag_util_mprintf("Mirror Entry %d : ignore rspan tag\n", *index_ptr);

    return CPARSER_OK;
}
#endif

#ifdef CMD_RSPAN_GET_TAG_MIRROR_ID_INDEX
cparser_result_t cparser_cmd_rspan_get_tag_mirror_id_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_mirror_rspanTag_t   tag;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_mirror_rspanTag_get(unit, *index_ptr, &tag), ret);
    diag_util_mprintf("Mirror Entry %d :\n", *index_ptr);
#if defined(CONFIG_SDK_RTL8380)
    diag_util_mprintf("\tTPID: 0x%x\n", tag.tpid);
#else
    diag_util_mprintf("\tTPID IDX: 0x%x\n", tag.tpidIdx);
#endif
    diag_util_mprintf("\tVID  : %d\n", tag.vid);
    diag_util_mprintf("\tPRI  : %d\n", tag.pri);
    diag_util_mprintf("\tCFI  : %d\n", tag.cfi);

    return CPARSER_OK;
}
#endif

#ifdef CMD_RSPAN_SET_EGRESS_MODE_MIRROR_ID_INDEX_ADD_TAG_NO_MODIFY_REMOVE_TAG
cparser_result_t cparser_cmd_rspan_set_egress_mode_mirror_id_index_add_tag_no_modify_remove_tag(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_mirror_rspanEgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('a' == TOKEN_CHAR(5,0))
        mode = RSPAN_EGR_ADD_TAG;
    else if ('n' == TOKEN_CHAR(5,0))
        mode = RSPAN_EGR_NO_MODIFY;
    else if ('r' == TOKEN_CHAR(5,0))
        mode = RSPAN_EGR_REMOVE_TAG;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_mirror_rspanEgrMode_set(unit, *index_ptr, mode), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_RSPAN_SET_INGRESS_MODE_MIRROR_ID_INDEX_HANDLE_TAG_IGNORE_TAG
cparser_result_t cparser_cmd_rspan_set_ingress_mode_mirror_id_index_handle_tag_ignore_tag(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_mirror_rspanIgrMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('h' == TOKEN_CHAR(5,0))
        mode = RSPAN_IGR_HANDLE_RSPAN_TAG;
    else if ('i' == TOKEN_CHAR(5,0))
        mode = RSPAN_IGR_IGNORE_RSPAN_TAG;
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;

    }

    DIAG_UTIL_ERR_CHK(rtk_mirror_rspanIgrMode_set(unit, *index_ptr, mode), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_RSPAN_SET_TAG_MIRROR_ID_INDEX_TPID_IDX_VID_PRI_CFI
cparser_result_t cparser_cmd_rspan_set_tag_mirror_id_index_tpid_idx_vid_pri_cfi(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *tpid_idx_ptr,
    uint32_t *vid_ptr,
    uint32_t *pri_ptr,
    uint32_t *cfi_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_mirror_rspanTag_t   tag;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8380)
    tag.tpid = *tpid_idx_ptr;
#else
    tag.tpidIdx = *tpid_idx_ptr;
#endif
    tag.vid = *vid_ptr;
    tag.pri = *pri_ptr;
    tag.cfi = *cfi_ptr;

    DIAG_UTIL_ERR_CHK(rtk_mirror_rspanTag_set(unit, *index_ptr, &tag), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SFLOW_GET_SAMPLE_CONTROL
/*
 * sflow get sample control
 */
cparser_result_t
cparser_cmd_sflow_get_sample_control(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_sflowSampleCtrl_t   ctrl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowSampleCtrl_get(unit, &ctrl), ret);

    diag_util_mprintf("sFlow sample control : ");
    if (SFLOW_CTRL_INGRESS == ctrl)
        diag_util_mprintf("Ingress\n");
    else if (SFLOW_CTRL_INGRESS == ctrl)
        diag_util_mprintf("Egress\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_sflow_get_sample_control */
#endif

#ifdef CMD_SFLOW_SET_SAMPLE_CONTROL_INGRESS_EGRESS
/*
 * sflow set sample control ( ingress | egress )
 */
cparser_result_t
cparser_cmd_sflow_set_sample_control_ingress_egress(cparser_context_t *context)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_sflowSampleCtrl_t   ctrl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('i' == TOKEN_CHAR(4,0))
        ctrl = SFLOW_CTRL_INGRESS;
    else
        ctrl = SFLOW_CTRL_EGRESS;

    DIAG_UTIL_ERR_CHK(rtk_mirror_sflowSampleCtrl_set(unit, ctrl), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_sflow_set_sample_control_ingress_egress */
#endif
