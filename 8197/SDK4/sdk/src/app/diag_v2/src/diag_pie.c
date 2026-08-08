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
 * Purpose : Define diag shell functions for PIE.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) PIE diag shell.
 *           2) field selector diag shell.
 *           3) pattern match diag shell.
 *           4) range check diag shell.
 */

#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <rtk/pie.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

/* convert Integer from string to number */
int32
diag_pie_str2IntArray (uint8 *int_array, uint8 *str)
{
    uint8  value = 0;
    uint32 str_idx, array_idx, hi_bits;

    if (!(int_array && str))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
        return  RT_ERR_FAILED;
    }

    if (!((strlen((char *)str) > 2) && ('0' == str[0]) && ('x' == str[1])))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
        return  RT_ERR_FAILED;
    }

    value = 0;
    array_idx = 0;
    hi_bits = 0;
    for (str_idx = (strlen((char *)str) - 1); str_idx >= 2; str_idx--)
    {
    	if (('0' <= str[str_idx]) && ('9' >= str[str_idx]))
    	{
    	    value = str[str_idx] - '0';
    	}
    	else if (('a' <= str[str_idx]) && ('f' >= str[str_idx]))
    	{
    	    value = str[str_idx] - 'a' + 10;
    	}
    	else if (('A' <= str[str_idx]) && ('F' >= str[str_idx]))
    	{
    	    value = str[str_idx] - 'A' + 10;
    	}
    	else
    	{
            RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
            return  RT_ERR_FAILED;
    	}

    	if(hi_bits == 1)
    	{
    	    int_array[array_idx] = int_array[array_idx] + (value  << 4);
    	    hi_bits = 0;
    	    array_idx++;
    	}
    	else
    	{
    	    int_array[array_idx] = value;
    	    hi_bits = 1;
    	}
    }

    return RT_ERR_OK;
}

#ifdef CMD_PIE_SET_ENTRY_PHASE_ENTRY_FIELD_CFI_EXIST_COPY2CPU_CPU_CTRL_PKT_DA_TYPE_DEI_EXIST_DIP_DMAC_DP_DPM_DPN_ETHERTYPE_EXTRA_TAG_FIELD_SELECTOR1_FIELD_SELECTOR1_0_FIELD_SELECTOR1_1_FIELD_SELECTOR1_2_FIELD_SELECTOR1_3_FIELD_SELECTOR2_FIELD_SELECTOR2_0_FIELD_SELECTOR2_1_FIELD_SELECTOR2_2_FIELD_SELECTOR2_3_FLOW_LABEL_FWD_ITAG_PRI_FWD_ITAG_VID_FWD_OTAG_CPU_TAG_FWD_OTAG_PRI_FWD_OTAG_VID_FWD_VID_IVID_OR_OVID_FWD_VID_PRI_ICMP_CODE_ICMP_TYPE_IGMP_TYPE_IP_DF_IP_MCAST_IP_MF_IP_NONZERO_IP_RANGE0_IP_RANGE1_IP_RANGE2_IP_RANGE3_IP_UCAST_IPV6_DIP_IPV6_SIP_ITAG_PRI_ITAG_VID_KEEP_ORG_VID_L4_DST_PORT_L4_SRC_PORT_LOOKUP_PHASE_OTAG_PRI_OTAG_VID_PAYLOAD0_PAYLOAD0_VALID_PAYLOAD1_PAYLOAD1_VALID_PATTERN_MATCH0_PATTERN_MATCH1_PORT_RANGE_PPPOE_PROTO_NH_REDIRECT_RTK_PROTO_SIP_SMAC_SPM_SPSM_SRC_PHY_PORT_TCP_FLAG_TGL2_FORMAT_TGL23_FORMAT_TGL4_FORMAT_TO_GUEST_VLAN_TOS_DS_TTL_TTL_TYPE_VID_RANGE_VID_RANGE_HIT_EVT_DITAG_PRI_EVT_DITAG_VID_EVT_DMAC_INDEX_EVT_DOTAG_PRI_EVT_DOTAG_VID_EVT_DP_EVT_DPN_EVT_DSCP_EVT_DSCP_RMK_EVT_EXTRA_TAG_EVT_IP_MCAST_EVT_IP_UCAST_EVT_ITAG_EXIST_EVT_L2_FORMAT_EVT_L34_FORMAT_EVT_LOOKUP_PHASE_EVT_OTAG_EXIST_EVT_PRI_EVT_RRCP_EVT_RRCP_TYPE_EVT_RX_CPU_TAG_EVT_RX_RSPAN_EVT_SRC_TRUNK_PORT_VALID_DATA_MASK
/*
 * pie set entry <UINT:phase> <UINT:entry> field ( cfi-exist | copy2cpu | cpu | ctrl-pkt | da-type | dei-exist | dip | dmac | dp | dpm | dpn | ethertype | extra-tag | field-selector1 | field-selector1_0 | field-selector1_1 | field-selector1_2 | field-selector1_3 | field-selector2 | field-selector2_0 | field-selector2_1 | field-selector2_2 | field-selector2_3 | flow-label | fwd-itag-pri | fwd-itag-vid | fwd-otag-cpu-tag | fwd-otag-pri | fwd-otag-vid | fwd-vid-ivid-or-ovid | fwd-vid-pri | icmp-code | icmp-type | igmp-type | ip-df | ip-mcast | ip-mf | ip-nonzero | ip-range0 | ip-range1 | ip-range2 | ip-range3 | ip-ucast | ipv6-dip | ipv6-sip | itag-pri | itag-vid | keep-org-vid | l4-dst-port | l4-src-port | lookup-phase | otag-pri | otag-vid | payload0 | payload0-valid | payload1 | payload1-valid | pattern-match0 | pattern-match1 | port-range | pppoe | proto-nh | redirect | rtk-proto | sip | smac | spm | spsm | src-phy-port | tcp-flag | tgl2-format | tgl23-format | tgl4-format | to-guest-vlan | tos-ds | ttl | ttl-type | vid-range | vid-range-hit | evt-ditag-pri | evt-ditag-vid | evt-dmac-index | evt-dotag-pri | evt-dotag-vid | evt-dp | evt-dpn | evt-dscp | evt-dscp-rmk | evt-extra-tag | evt-ip-mcast | evt-ip-ucast | evt-itag-exist | evt-l2-format | evt-l34-format | evt-lookup-phase | evt-otag-exist | evt-pri | evt-rrcp | evt-rrcp-type | evt-rx-cpu-tag | evt-rx-rspan | evt-src-trunk-port | valid ) <STRING:data> <STRING:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_phase_entry_field_cfi_exist_copy2cpu_cpu_ctrl_pkt_da_type_dei_exist_dip_dmac_dp_dpm_dpn_ethertype_extra_tag_field_selector1_field_selector1_0_field_selector1_1_field_selector1_2_field_selector1_3_field_selector2_field_selector2_0_field_selector2_1_field_selector2_2_field_selector2_3_flow_label_fwd_itag_pri_fwd_itag_vid_fwd_otag_cpu_tag_fwd_otag_pri_fwd_otag_vid_fwd_vid_ivid_or_ovid_fwd_vid_pri_icmp_code_icmp_type_igmp_type_ip_df_ip_mcast_ip_mf_ip_nonzero_ip_range0_ip_range1_ip_range2_ip_range3_ip_ucast_ipv6_dip_ipv6_sip_itag_pri_itag_vid_keep_org_vid_l4_dst_port_l4_src_port_lookup_phase_otag_pri_otag_vid_payload0_payload0_valid_payload1_payload1_valid_pattern_match0_pattern_match1_port_range_pppoe_proto_nh_redirect_rtk_proto_sip_smac_spm_spsm_src_phy_port_tcp_flag_tgl2_format_tgl23_format_tgl4_format_to_guest_vlan_tos_ds_ttl_ttl_type_vid_range_vid_range_hit_evt_ditag_pri_evt_ditag_vid_evt_dmac_index_evt_dotag_pri_evt_dotag_vid_evt_dp_evt_dpn_evt_dscp_evt_dscp_rmk_evt_extra_tag_evt_ip_mcast_evt_ip_ucast_evt_itag_exist_evt_l2_format_evt_l34_format_evt_lookup_phase_evt_otag_exist_evt_pri_evt_rrcp_evt_rrcp_type_evt_rx_cpu_tag_evt_rx_rspan_evt_src_trunk_port_valid_data_mask(cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *entry_ptr,
    char **data_ptr,
    char **mask_ptr)
{
    uint32              unit = 0;
    uint32              field_size = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_pie_fieldType_t type = PIE_FIELD_END;
    uint8               *pField_data = NULL;
    uint8               *pField_mask = NULL;


    if ((NULL == phase_ptr) || (NULL == entry_ptr) || (NULL == data_ptr) || (NULL == mask_ptr))
    {
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(6,0))
    {
        case 'c':
            switch (TOKEN_CHAR(6,1))
            {
                case 'f': /* cfi-exist */
                    type = FIELD_CFI_EXIST;
                    break;
                case 'o': /* copy2cpu */
                    type = FIELD_COPYTOCPU;
                    break;
                case 'p': /* cpu */
                    type = FIELD_CPU;
                    break;
                case 't': /* ctrl-pkt */
                    type = FIELD_CTLPKT;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'd':
            switch (TOKEN_CHAR(6,1))
            {
                case 'a': /* da-type */
                    type = FIELD_DA_TYPE;
                    break;
                case 'e': /* dei-exist */
                    type = FIELD_DEI_EXIST;
                    break;
                case 'i': /* dip */
                    type = FIELD_DIP;
                    break;
                case 'm': /* dmac */
                    type = FIELD_DMAC;
                    break;
                case 'p':
                    switch (TOKEN_CHAR(6,2))
                    {
                        case 'm': /* dpm */
                            type = FIELD_DPM;
                            break;
                        case 'n': /* dpn */
                            type = FIELD_DPN;
                            break;
                        default: /* dp */
                            type = FIELD_DP;
                            break;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'e':
            switch (TOKEN_CHAR(6,1))
            {
                case 't': /* ethertype */
                    type = FIELD_ETHERTYPE;
                    break;
                case 'v': /* evt- */
                    switch (TOKEN_CHAR(6,4))
                    {
                        case 'd':
                            switch (TOKEN_CHAR(6,5))
                            {
                                case 'i':
                                    switch (TOKEN_CHAR(6,10))
                                    {
                                        case 'p': /* evt-ditag-pri */
                                            type = FIELD_EVT_DITAG_PRI;
                                            break;
                                        case 'v': /* evt-ditag-vid */
                                            type = FIELD_EVT_DITAG_VID;
                                            break;
                                        default:
                                            diag_util_printf("User config: Error!\n");
                                            return CPARSER_NOT_OK;
                                    }
                                    break;
                                case 'm': /* evt-dmac-index */
                                    type = FIELD_EVT_DMAC_INDEX;
                                    break;
                                case 'o':
                                    switch (TOKEN_CHAR(6,10))
                                    {
                                        case 'p': /* evt-dotag-pri */
                                            type = FIELD_EVT_DOTAG_PRI;
                                            break;
                                        case 'v': /* evt-dotag-vid */
                                            type = FIELD_EVT_DOTAG_VID;
                                            break;
                                        default:
                                            diag_util_printf("User config: Error!\n");
                                            return CPARSER_NOT_OK;
                                    }
                                    break;
                                case 'p':
                                    switch (TOKEN_CHAR(6,6))
                                    {
                                        case 'n': /* evt-dpn */
                                            type = FIELD_EVT_DPN;
                                            break;
                                        default: /* evt-dp */
                                            type = FIELD_EVT_DP;
                                            break;
                                    }
                                    break;
                                case 's':
                                    switch (TOKEN_CHAR(6,9))
                                    {
                                        case 'r': /* evt-dscp-rmk */
                                            type = FIELD_EVT_DSCP_RMK;
                                            break;
                                        default:  /* evt-dscp */
                                            type = FIELD_EVT_DSCP;
                                            break;
                                    }
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'e': /* evt-extra-tag */
                            type = FIELD_EVT_EXTRA_TAG;
                            break;
                        case 'i':
                            switch (TOKEN_CHAR(6,7))
                            {
                                case 'm': /* evt-ip-mcast */
                                    type = FIELD_EVT_IP_MCAST;
                                    break;
                                case 'u': /* evt-ip-ucast */
                                    type = FIELD_EVT_IP_UCAST;
                                    break;
                                case 'g': /* evt-itag-exist */
                                    type = FIELD_EVT_ITAG_EXIST;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'l':
                            switch (TOKEN_CHAR(6,5))
                            {
                                case '2': /* evt-l2-format */
                                    type = FIELD_EVT_L2_FORMAT;
                                    break;
                                case '3': /* evt-l34-format */
                                    type = FIELD_EVT_L34_FORMAT;
                                    break;
                                case 'o': /* evt-lookup-phase */
                                    type = FIELD_EVT_LOOKUP_PHASE;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'o': /* evt-otag-exist */
                            type = FIELD_EVT_OTAG_EXIST;
                            break;
                        case 'p': /* evt-pri */
                            type = FIELD_EVT_PRI;
                            break;
                        case 'r':
                            switch (TOKEN_CHAR(6,9))
                            {
                                case 't': /* evt-rrcp-type */
                                    type = FIELD_EVT_RRCP_TYPE;
                                    break;
                                case 'u': /* evt-rx-cpu-tag */
                                    type = FIELD_EVT_RX_CPU_TAG;
                                    break;
                                case 'p': /* evt-rx-rspan */
                                    type = FIELD_EVT_RX_RSPAN;
                                    break;
                                default:  /* evt-rrcp */
                                    type = FIELD_EVT_RRCP;
                                    break;
                            }
                            break;
                        case 's': /* evt-src-trunk-port */
                            type = FIELD_EVT_SRC_TRUNK_PHY_PORT;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'x': /* extra-tag */
                    type = FIELD_EXTRA_TAG;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'f':
            switch (TOKEN_CHAR(6,1))
            {
                case 'i':
                    switch (TOKEN_CHAR(6,14))
                    {
                        case '1':
                            switch (TOKEN_CHAR(6,16))
                            {
                                case '0': /* field-selector1_0 */
                                    type = FIELD_FIELD_SELECTOR1_0;
                                    break;
                                case '1': /* field-selector1_1 */
                                    type = FIELD_FIELD_SELECTOR1_1;
                                    break;
                                case '2': /* field-selector1_2 */
                                    type = FIELD_FIELD_SELECTOR1_2;
                                    break;
                                case '3': /* field-selector1_3 */
                                    type = FIELD_FIELD_SELECTOR1_3;
                                    break;
                                default:  /* field-selector1 */
                                    type = FIELD_FIELD_SELECTOR1;
                                    break;
                            }
                            break;
                        case '2':
                            switch (TOKEN_CHAR(6,16))
                            {
                                case '0': /* field-selector2_0 */
                                    type = FIELD_FIELD_SELECTOR2_0;
                                    break;
                                case '1': /* field-selector2_1 */
                                    type = FIELD_FIELD_SELECTOR2_1;
                                    break;
                                case '2': /* field-selector2_2 */
                                    type = FIELD_FIELD_SELECTOR2_2;
                                    break;
                                case '3': /* field-selector2_3 */
                                    type = FIELD_FIELD_SELECTOR2_3;
                                    break;
                                default:  /* field-selector2 */
                                    type = FIELD_FIELD_SELECTOR2;
                                    break;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'l': /* flow-label */
                    type = FIELD_FLOW_LABEL;
                    break;
                case 'w':
                    switch (TOKEN_CHAR(6,4))
                    {
                        case 'i':
                            switch (TOKEN_CHAR(6,9))
                            {
                                case 'p': /* fwd-itag_pri */
                                    type = FIELD_FWD_ITAG_PRI;
                                    break;
                                case 'v': /* fwd-itag-vid */
                                    type = FIELD_FWD_ITAG_VID;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'o':
                            switch (TOKEN_CHAR(6,9))
                            {
                                case 'c': /* fwd-otag-cpu-tag */
                                    type = FIELD_FWD_CPU_TAG;
                                    break;
                                case 'p': /* fwd-otag-pri */
                                    type = FIELD_FWD_OTAG_PRI;
                                    break;
                                case 'v': /* fwd-otag-vid */
                                    type = FIELD_FWD_OTAG_VID;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'v':
                            switch (TOKEN_CHAR(6,8))
                            {
                                case 'i': /* fwd-vid-ivid-or-ovid */
                                    type = FIELD_FWD_VID_IVID_OR_OVID;
                                    break;
                                case 'p': /* fwd-vid-pri */
                                    type = FIELD_FWD_VID_PRI;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'i':
            switch (TOKEN_CHAR(6,1))
            {
                case 'c':
                    switch (TOKEN_CHAR(6,5))
                    {
                        case 'c': /* icmp-code */
                            type = FIELD_ICMP_CODE;
                            break;
                        case 't': /* icmp-type */
                            type = FIELD_ICMP_TYPE;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'g': /* igmp-type */
                    type = FIELD_IGMP_TYPE;
                    break;
                case 'p':
                    switch (TOKEN_CHAR(6,3))
                    {
                        case 'd': /* ip-df */
                            type = FIELD_IP_DF;
                            break;
                        case 'm':
                            switch (TOKEN_CHAR(6,4))
                            {
                                case 'c': /* ip-mcast */
                                    type = FIELD_IP_MCAST;
                                    break;
                                case 'f': /* ip-mf */
                                    type = FIELD_IP_MF;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'n': /* ip-nonzero */
                            type = FIELD_IP_NONZERO_OFFSET;
                            break;
                        case 'r':
                            switch (TOKEN_CHAR(6,8))
                            {
                                case '0': /* ip-range0 */
                                    type = FIELD_IP_RANGE0;
                                    break;
                                case '1': /* ip-range1 */
                                    type = FIELD_IP_RANGE1;
                                    break;
                                case '2': /* ip-range2 */
                                    type = FIELD_IP_RANGE2;
                                    break;
                                case '3': /* ip-range3 */
                                    type = FIELD_IP_RANGE3;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'u': /* ip-ucast */
                            type = FIELD_IP_UCAST;
                            break;
                        case '6':
                            switch (TOKEN_CHAR(6,5))
                            {
                                case 'd': /* ipv6-dip */
                                    type = FIELD_IPV6_DIP;
                                    break;
                                case 's': /* ipv6-sip */
                                    type = FIELD_IPV6_SIP;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 't':
                    switch (TOKEN_CHAR(6,5))
                    {
                        case 'p': /* itag-pri */
                            type = FIELD_ITAG_PRI;
                            break;
                        case 'v': /* itag-vid */
                            type = FIELD_ITAG_VID;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'k': /* keep-org-vid */
            type = FIELD_KEEP_ORG_VID;
            break;

        case 'l':
            switch (TOKEN_CHAR(6,1))
            {
                case '4':
                    switch (TOKEN_CHAR(6,3))
                    {
                        case 'd': /* l4-dst-port */
                            type = FIELD_L4_DST_PORT;
                            break;
                        case 's': /* l4-src-port */
                            type = FIELD_L4_SRC_PORT;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'o': /* lookup-phase */
                    type = FIELD_LOOKUP_PHASE;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;

        case 'o':
            switch (TOKEN_CHAR(6,5))
            {
                case 'p': /* otag-pri */
                    type = FIELD_OTAG_PRI;
                    break;
                case 'v': /* otag-vid */
                    type = FIELD_OTAG_VID;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;

        case 'p':
            switch (TOKEN_CHAR(6,1))
            {
                case 'a':
                    switch (TOKEN_CHAR(6,2))
                    {
                        case 't':
                            switch (TOKEN_CHAR(6,13))
                            {
                                case '0': /* pattern-match0 */
                                    type = FIELD_PATTERN_MATCH0;
                                    break;
                                case '1': /* pattern-match1 */
                                    type = FIELD_PATTERN_MATCH1;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'y':
                            switch (TOKEN_CHAR(6,7))
                            {
                                case '0':
                                    switch (TOKEN_CHAR(6,9))
                                    {
                                        case 'v': /* payload0-valid */
                                            type = FIELD_PAYLOAD0_VALID;
                                            break;
                                        default: /* payload0 */
                                            type = FIELD_PAYLOAD0;
                                            break;
                                    }
                                    break;
                                case '1':
                                    switch (TOKEN_CHAR(6,9))
                                    {
                                        case 'v': /* payload1-valid */
                                            type = FIELD_PAYLOAD1_VALID;
                                            break;
                                        default: /* payload1 */
                                            type = FIELD_PAYLOAD1;
                                            break;
                                    }
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'o': /* port-range */
                    type = FIELD_PORT_RANGE;
                    break;
                case 'p': /* pppoe */
                    type = FIELD_PPPOE;
                    break;
                case 'r': /* proto-nh */
                    type = FIELD_IPV4PROTO_IPV6NH;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;

        case 'r':
            switch (TOKEN_CHAR(6,1))
            {
                case 'e': /* redirect */
                    type = FIELD_REDIRECT;
                    break;
                case 't': /* rtk-proto */
                    type = FIELD_RTK_PROTO;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;

        case 's':
            switch (TOKEN_CHAR(6,1))
            {
                case 'i': /* sip */
                    type = FIELD_SIP;
                    break;
                case 'm': /* smac */
                    type = FIELD_SMAC;
                    break;
                case 'p':
                    switch (TOKEN_CHAR(6,2))
                    {
                        case 'm': /* spm */
                            type = FIELD_SPM;
                            break;
                        case 's': /* spsm */
                            type = FIELD_SPSM;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'r': /* src-phy-port */
                    type = FIELD_SRC_PHY_PORT;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 't':
            switch (TOKEN_CHAR(6,1))
            {
                case 'c': /* tcp-flag */
                    type = FIELD_TCP_FLAG;
                    break;
                case 'g':
                    switch (TOKEN_CHAR(6,3))
                    {
                        case '2':
                            switch (TOKEN_CHAR(6,4))
                            {
                                case '-': /* tgl2-format */
                                    type = FIELD_TGL2_FORMAT;
                                    break;
                                case '3': /* tgl23-format */
                                    type = FIELD_TGL23_FORMAT;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case '4': /* tgl4-format */
                            type = FIELD_TGL4_FORMAT;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'o':
                    switch (TOKEN_CHAR(6,2))
                    {
                        case '-': /* to-guest-vlan */
                            type = FIELD_TO_GUEST_VLAN;
                            break;
                        case 's': /* tos-ds */
                            type = FIELD_IPV4TOS_IPV6DS;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 't':
                    switch (TOKEN_CHAR(6,3))
                    {
                        case '-': /* ttl-type */
                            type = FIELD_TTL_TYPE;
                            break;
                        default: /* ttl */
                            type = FIELD_TTL;
                            break;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'v':
            switch (TOKEN_CHAR(6,1))
            {
                case 'a': /* valid */
                    type = FIELD_VAILD;
                    break;
                case 'i':
                    switch (TOKEN_CHAR(6,9))
                    {
                        case '-': /* vid_range-hit */
                            type = FIELD_VID_RANGE_HIT;
                            break;
                        default: /* vid_range */
                            type = FIELD_VID_RANGE;
                            break;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntryFieldSize_get(unit, type, &field_size), ret);

    if ((field_size % 8) == 0)
    {
        field_size = (field_size / 8);
    }
    else
    {
        field_size = (field_size / 8) + 1;
    }

    if (field_size < ((strlen(*data_ptr)-2)/2))
    {
        diag_util_printf("data error!\n");
        return CPARSER_NOT_OK;
    }

    if (field_size < ((strlen(*mask_ptr)-2)/2))
    {
        diag_util_printf("mask error!\n");
        return CPARSER_NOT_OK;
    }

    if ((pField_data = malloc(field_size)) == NULL)
    {
        diag_util_printf("pField_data malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pField_data, 0, field_size);

    if ((pField_mask = malloc(field_size)) == NULL)
    {
        diag_util_printf("pField_mask malloc() fail!\n");
        free(pField_data);
        return CPARSER_NOT_OK;
    }
    memset(pField_mask, 0, field_size);

    if (diag_pie_str2IntArray (pField_data, (uint8 *)*data_ptr) != RT_ERR_OK)
    {
        diag_util_printf("data error!\n");
        free(pField_data);
        free(pField_mask);
        return CPARSER_NOT_OK;
    }

    if (diag_pie_str2IntArray (pField_mask, (uint8 *)*mask_ptr) != RT_ERR_OK)
    {
        diag_util_printf("mask error!\n");
        free(pField_data);
        free(pField_mask);
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_pieRuleEntryField_write(unit, *phase_ptr, *entry_ptr, type, pField_data, pField_mask)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        free(pField_data);
        free(pField_mask);
        return CPARSER_NOT_OK;
    }

    free(pField_data);
    free(pField_mask);
    return CPARSER_OK;
}
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_DST_VID_EXTRA_TAG_LOOK_PHASE_PPPOE_SRC_PHY_PORT_SRC_VID_TGL2_FORMAT_TTL_TYPE_VALID_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ( dst-vid | extra-tag | look-phase | pppoe | src-phy-port | src-vid | tgl2-format | ttl-type | valid ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_dst_vid_extra_tag_look_phase_pppoe_src_phy_port_src_vid_tgl2_format_ttl_type_valid_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(6,0))
    {
        case 'd': /* dst-vid */
            predefined_entry.dst_vid = *data_ptr;
            predefined_entry.dst_vid_care = *mask_ptr;
            break;
        case 'e': /* extra-tag */
            predefined_entry.extra_tag = *data_ptr;
            predefined_entry.extra_tag_care = *mask_ptr;
            break;
        case 'l': /* look-phase */
            predefined_entry.lookup_phase = *data_ptr;
            predefined_entry.lookup_phase_care = *mask_ptr;
            break;
        case 'p': /* pppoe */
            predefined_entry.pppoe = *data_ptr;
            predefined_entry.pppoe_care = *mask_ptr;
            break;
        case 's':
            switch (TOKEN_CHAR(6,4))
            {
                case 'p': /* src-phy-port */
                    predefined_entry.src_phy_port = *data_ptr;
                    predefined_entry.src_phy_port_care = *mask_ptr;
                    break;
                case 'v': /* src-vid */
                    predefined_entry.src_vid = *data_ptr;
                    predefined_entry.src_vid_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 't':
            switch (TOKEN_CHAR(6,1))
            {
                case 'g': /* tgl2-format */
                    predefined_entry.tgl2fmt = *data_ptr;
                    predefined_entry.tgl2fmt_care = *mask_ptr;
                    break;
                case 't': /* ttl-type */
                    predefined_entry.ttl_type = *data_ptr;
                    predefined_entry.ttl_type_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'v': /* valid */
            predefined_entry.valid = *data_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_dst_vid_extra_tag_look_phase_pppoe_src_phy_port_src_vid_tgl2_format_ttl_type_valid_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_ARP_DIP_SIP_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field arp ( dip | sip ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_arp_dip_sip_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*00000*/
    predefined_entry.tgl23fmt = 0;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 0;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* dip */
            predefined_entry.un.arp.dip = *data_ptr;
            predefined_entry.un.arp.dip_care = *mask_ptr;
            break;
        case 's': /* sip */
            predefined_entry.un.arp.sip = *data_ptr;
            predefined_entry.un.arp.sip_care = *mask_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_arp_dip_sip_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV4_DIP_PROTOCOL_SIP_TOS_TTL_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv4 ( dip | protocol | sip | tos | ttl ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_dip_protocol_sip_tos_ttl_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*10000*/
    predefined_entry.tgl23fmt = 2;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 0;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* dip */
            predefined_entry.un.ipv4.dip = *data_ptr;
            predefined_entry.un.ipv4.dip_care = *mask_ptr;
            break;
        case 'p': /* protocol */
            predefined_entry.un.ipv4.protocol = *data_ptr;
            predefined_entry.un.ipv4.protocol_care = *mask_ptr;
            break;
        case 's': /* sip */
            predefined_entry.un.ipv4.sip = *data_ptr;
            predefined_entry.un.ipv4.sip_care = *mask_ptr;
            break;
        case 't':
            switch (TOKEN_CHAR(7,1))
            {
                case 'o': /* tos */
                    predefined_entry.un.ipv4.tos = *data_ptr;
                    predefined_entry.un.ipv4.tos_care = *mask_ptr;
                    break;
                case 't': /* ttl */
                    predefined_entry.un.ipv4.ttl = *data_ptr;
                    predefined_entry.un.ipv4.ttl_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_dip_protocol_sip_tos_ttl_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV4_ICMP_DIP_ICMP_CODE_ICMP_TYPE_SIP_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv4-icmp ( dip | icmp-code | icmp-type | sip ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_icmp_dip_icmp_code_icmp_type_sip_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*10011*/
    predefined_entry.tgl23fmt = 2;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 3;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* dip */
            predefined_entry.un.ipv4_icmp.dip = *data_ptr;
            predefined_entry.un.ipv4_icmp.dip_care = *mask_ptr;
            break;
        case 'i':
            switch (TOKEN_CHAR(7,5))
            {
                case 'c': /* icmp-code */
                    predefined_entry.un.ipv4_icmp.icmp_code = *data_ptr;
                    predefined_entry.un.ipv4_icmp.icmp_code_care = *mask_ptr;
                    break;
                case 't': /* icmp-type */
                    predefined_entry.un.ipv4_icmp.icmp_type = *data_ptr;
                    predefined_entry.un.ipv4_icmp.icmp_type_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 's': /* sip */
            predefined_entry.un.ipv4_icmp.sip = *data_ptr;
            predefined_entry.un.ipv4_icmp.sip_care = *mask_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_icmp_dip_icmp_code_icmp_type_sip_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV4_IGMP_DIP_GROUP_IP_IGMP_TYPE_SIP_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv4-igmp ( dip | group-ip | igmp-type | sip ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_igmp_dip_group_ip_igmp_type_sip_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*10100*/
    predefined_entry.tgl23fmt = 2;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 4;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* dip */
            predefined_entry.un.ipv4_igmp.dip = *data_ptr;
            predefined_entry.un.ipv4_igmp.dip_care = *mask_ptr;
            break;
        case 'g': /* group-ip */
            predefined_entry.un.ipv4_igmp.group_ip = *data_ptr;
            predefined_entry.un.ipv4_igmp.group_ip_care = *mask_ptr;
            break;
        case 'i': /* igmp-type */
            predefined_entry.un.ipv4_igmp.igmp_type = *data_ptr;
            predefined_entry.un.ipv4_igmp.igmp_type_care = *mask_ptr;
            break;
        case 's': /* sip */
            predefined_entry.un.ipv4_igmp.sip = *data_ptr;
            predefined_entry.un.ipv4_igmp.sip_care = *mask_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_igmp_dip_group_ip_igmp_type_sip_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV4_TCP_DIP_DST_PORT_SIP_SRC_PORT_TCP_FLAG_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv4-tcp ( dip | dst-port | sip | src-port | tcp-flag ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_tcp_dip_dst_port_sip_src_port_tcp_flag_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*10001*/
    predefined_entry.tgl23fmt = 2;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 1;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd':
            switch (TOKEN_CHAR(7,1))
            {
                case 'i': /* dip */
                    predefined_entry.un.ipv4_tcp.dip = *data_ptr;
                    predefined_entry.un.ipv4_tcp.dip_care = *mask_ptr;
                    break;
                case 's': /* dst-port */
                    predefined_entry.un.ipv4_tcp.dst_port = *data_ptr;
                    predefined_entry.un.ipv4_tcp.dst_port_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 's':
            switch (TOKEN_CHAR(7,1))
            {
                case 'i': /* sip */
                    predefined_entry.un.ipv4_tcp.sip = *data_ptr;
                    predefined_entry.un.ipv4_tcp.sip_care = *mask_ptr;
                    break;
                case 'r': /* src-port */
                    predefined_entry.un.ipv4_tcp.src_port = *data_ptr;
                    predefined_entry.un.ipv4_tcp.src_port_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 't': /* tcp-flag */
            predefined_entry.un.ipv4_tcp.tcp_flag = *data_ptr;
            predefined_entry.un.ipv4_tcp.tcp_flag_care = *mask_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
} /* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_tcp_dip_dst_port_sip_src_port_tcp_flag_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV4_UDP_DIP_DST_PORT_SIP_SRC_PORT_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv4-udp ( dip | dst-port | sip | src-port ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_udp_dip_dst_port_sip_src_port_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*10010*/
    predefined_entry.tgl23fmt = 2;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 2;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd':
            switch (TOKEN_CHAR(7,1))
            {
                case 'i': /* dip */
                    predefined_entry.un.ipv4_udp.dip = *data_ptr;
                    predefined_entry.un.ipv4_udp.dip_care = *mask_ptr;
                    break;
                case 's': /* dst-port */
                    predefined_entry.un.ipv4_udp.dst_port = *data_ptr;
                    predefined_entry.un.ipv4_udp.dst_port_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 's':
            switch (TOKEN_CHAR(7,1))
            {
                case 'i': /* sip */
                    predefined_entry.un.ipv4_udp.sip = *data_ptr;
                    predefined_entry.un.ipv4_udp.sip_care = *mask_ptr;
                    break;
                case 'r': /* src-port */
                    predefined_entry.un.ipv4_udp.src_port = *data_ptr;
                    predefined_entry.un.ipv4_udp.src_port_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv4_udp_dip_dst_port_sip_src_port_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV6_DIP127_96_DIP95_64_DSCP_FLOW_LABEL_NEXT_HEADER_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv6 ( dip127-96 | dip95-64 | dscp | flow-label | next-header ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_dip127_96_dip95_64_dscp_flow_label_next_header_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*11000*/
    predefined_entry.tgl23fmt = 3;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 0;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd':
            switch (TOKEN_CHAR(7,1))
            {
                case 'i':
                     switch (TOKEN_CHAR(7,3))
                    {
                        case '1': /* dip127-96 */
                            predefined_entry.un.ipv6.ipv6_dip[3] = *data_ptr;
                            predefined_entry.un.ipv6.ipv6_dip_care[3] = *mask_ptr;
                            break;
                        case '9': /* dip95-64 */
                            predefined_entry.un.ipv6.ipv6_dip[2] = *data_ptr;
                            predefined_entry.un.ipv6.ipv6_dip_care[2] = *mask_ptr;
                            break;
                       default:
                            free(pEntry_buffer);
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 's': /* dscp */
                    predefined_entry.un.ipv6.dscp = *data_ptr;
                    predefined_entry.un.ipv6.dscp_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'f': /* flow-label */
            predefined_entry.un.ipv6.flow_label = *data_ptr;
            predefined_entry.un.ipv6.flow_label_care = *mask_ptr;
            break;
        case 'n': /* next-header */
            predefined_entry.un.ipv6.next_header = *data_ptr;
            predefined_entry.un.ipv6.next_header_care = *mask_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_dip127_96_dip95_64_dscp_flow_label_next_header_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV6_ICMPV6_DIP127_96_DIP95_64_ICMP_CODE_ICMP_TYPE_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv6-icmpv6 ( dip127-96 | dip95-64 | icmp-code | icmp-type ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_icmpv6_dip127_96_dip95_64_icmp_code_icmp_type_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*11101*/
    predefined_entry.tgl23fmt = 3;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 5;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd':
            switch (TOKEN_CHAR(7,3))
            {
                case '1': /* dip127-96 */
                    predefined_entry.un.ipv6_icmp.ipv6_dip[3] = *data_ptr;
                    predefined_entry.un.ipv6_icmp.ipv6_dip_care[3] = *mask_ptr;
                    break;
                case '9': /* dip95-64 */
                    predefined_entry.un.ipv6_icmp.ipv6_dip[2] = *data_ptr;
                    predefined_entry.un.ipv6_icmp.ipv6_dip_care[2] = *mask_ptr;
                    break;

                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'i':
            switch (TOKEN_CHAR(7,5))
            {
                case 'c': /* icmp-code */
                    predefined_entry.un.ipv6_icmp.icmp_code = *data_ptr;
                    predefined_entry.un.ipv6_icmp.icmp_code_care = *mask_ptr;
                    break;
                case 't': /* icmp-type */
                    predefined_entry.un.ipv6_icmp.icmp_type = *data_ptr;
                    predefined_entry.un.ipv6_icmp.icmp_type_care = *mask_ptr;
                    break;

                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_icmpv6_dip127_96_dip95_64_icmp_code_icmp_type_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV6_TCP_DIP127_96_DIP95_64_DST_PORT_SRC_PORT_TCP_FLAG_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv6-tcp ( dip127-96 | dip95-64 | dst-port | src-port | tcp-flag ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_tcp_dip127_96_dip95_64_dst_port_src_port_tcp_flag_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*11001*/
    predefined_entry.tgl23fmt = 3;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 1;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd':
            switch (TOKEN_CHAR(7,1))
            {
                case 'i':
                    switch (TOKEN_CHAR(7,3))
                    {
                        case '1': /* dip127-96 */
                            predefined_entry.un.ipv6_tcp.ipv6_dip[3] = *data_ptr;
                            predefined_entry.un.ipv6_tcp.ipv6_dip_care[3] = *mask_ptr;
                            break;
                        case '9': /* dip95-64 */
                            predefined_entry.un.ipv6_tcp.ipv6_dip[2] = *data_ptr;
                            predefined_entry.un.ipv6_tcp.ipv6_dip_care[2] = *mask_ptr;
                            break;
                        default:
                            free(pEntry_buffer);
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 's': /* dst-port */
                    predefined_entry.un.ipv6_tcp.dst_port = *data_ptr;
                    predefined_entry.un.ipv6_tcp.dst_port_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 's': /* src-port */
            predefined_entry.un.ipv6_tcp.src_port = *data_ptr;
            predefined_entry.un.ipv6_tcp.src_port_care = *mask_ptr;
            break;
        case 't': /* tcp-flag */
            predefined_entry.un.ipv6_tcp.tcp_flag = *data_ptr;
            predefined_entry.un.ipv6_tcp.tcp_flag_care = *mask_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_tcp_dip127_96_dip95_64_dst_port_src_port_tcp_flag_data_mask */
#endif

#ifdef CMD_PIE_SET_ENTRY_PRE_DEFINE_INDEX_FIELD_IPV6_UDP_DIP127_96_DIP95_64_DST_PORT_SRC_PORT_DATA_MASK
/*
 * pie set entry pre-define <UINT:index> field ipv6-udp ( dip127-96 | dip95-64 | dst-port | src-port ) <UINT:data> <UINT:mask>
 */
cparser_result_t cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_udp_dip127_96_dip95_64_dst_port_src_port_data_mask(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *data_ptr,
    uint32_t *mask_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  entry_size;
    uint8                           *pEntry_buffer;
    rtk_pie_preDefinedRuleEntry_t   predefined_entry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&predefined_entry, 0, sizeof(rtk_pie_preDefinedRuleEntry_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);
    entry_size = (entry_size / 8) + 1;
    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("pEntry_buffer malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    if ((ret = rtk_pie_pieRuleEntry_read(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_piePreDefinedRuleEntry_get(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    /*11010*/
    predefined_entry.tgl23fmt = 3;
    predefined_entry.tgl23fmt_care = 0x3;
    predefined_entry.tgl4fmt = 2;
    predefined_entry.tgl4fmt_care = 0x7;
    switch (TOKEN_CHAR(7,0))
    {
        case 'd':
            switch (TOKEN_CHAR(7,1))
            {
                case 'i':
                    switch (TOKEN_CHAR(7,3))
                    {
                        case '1': /* dip127-96 */
                            predefined_entry.un.ipv6_udp.ipv6_dip[3] = *data_ptr;
                            predefined_entry.un.ipv6_udp.ipv6_dip_care[3] = *mask_ptr;
                            break;
                        case '9': /* dip95-64 */
                            predefined_entry.un.ipv6_udp.ipv6_dip[2] = *data_ptr;
                            predefined_entry.un.ipv6_udp.ipv6_dip_care[2] = *mask_ptr;
                            break;
                        default:
                            free(pEntry_buffer);
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 's': /* dst-port */
                    predefined_entry.un.ipv6_udp.dst_port = *data_ptr;
                    predefined_entry.un.ipv6_udp.dst_port_care = *mask_ptr;
                    break;
                default:
                    free(pEntry_buffer);
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 's': /* src-port */
            predefined_entry.un.ipv6_udp.src_port = *data_ptr;
            predefined_entry.un.ipv6_udp.src_port_care = *mask_ptr;
            break;
        default:
            free(pEntry_buffer);
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if ((ret = rtk_pie_piePreDefinedRuleEntry_set(unit, pEntry_buffer, &predefined_entry)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if ((ret = rtk_pie_pieRuleEntry_write(unit, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        free(pEntry_buffer);
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_entry_pre_define_index_field_ipv6_udp_dip127_96_dip95_64_dst_port_src_port_data_mask */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_STATISTIC_BYTE32_BYTE64_NONE_PKT
/*
 * pie set action <UINT:index> statistic ( byte32 | byte64 | none | pkt )
 */
cparser_result_t cparser_cmd_pie_set_action_index_statistic_byte32_byte64_none_pkt(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_statisticType_t type;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(5,0))
    {
        case 'b':
            switch (TOKEN_CHAR(5,4))
            {
                case '3': /* byte32 */
                    type = COUNT_BYTE32;
                    break;
                case '6': /* byte64 */
                    type = COUNT_BYTE64;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'n': /* none */
            type = COUNT_NONE;
            break;
        case 'p': /* pkt */
            type = COUNT_PACKET;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.statistics = type;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_statistic_byte32_byte64_none_pkt */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_DROP_MODE_DROP_PERMIT_WITHDRAW_DROP
/*
 * pie set action <UINT:index> drop-mode ( drop | permit | withdraw-drop )
 */
cparser_result_t cparser_cmd_pie_set_action_index_drop_mode_drop_permit_withdraw_drop(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_dropType_t      type;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(5,0))
    {
        case 'd': /* drop */
            type = DROP_DROP;
            break;
        case 'p': /* permit */
            type = DROP_PERMIT;
            break;
        case 'w': /* withdraw_drop */
            type = DROP_WITHDRAW_DROP;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.drop = type;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_drop_mode_drop_permit_withdraw_drop */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_INNER_TAG_OUTER_TAG_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> ( inner-tag | outer-tag ) withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_inner_tag_outer_tag_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    uint32                  outer_inner;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;
#define OUTER   1
#define INNER   2

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(4,0))
    {
        case 'i': /* inner-tag */
            outer_inner = INNER;
            break;
        case 'o': /* outer-tag */
            outer_inner = OUTER;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    if (INNER == outer_inner)
    {
        action.inner_tag_op_field.withdraw = enable;
    }
    else
    {
        action.outer_tag_op_field.withdraw = enable;
    }
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_inner_tag_outer_tag_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_INNER_TAG_OUTER_TAG_AS_VID_INFO_MINUS_VID_INFO_PLUS_VID_INFO_VID_INFO_KEEP_FORMAT_NOP_WITH_TAG_WITHOUT_TAG
/*
 * pie set action <UINT:index> ( inner-tag | outer-tag ) ( as-vid-info | minus-vid-info | plus-vid-info ) <UINT:vid_info> ( keep-format | nop | with-tag | without-tag )
 */
cparser_result_t cparser_cmd_pie_set_action_index_inner_tag_outer_tag_as_vid_info_minus_vid_info_plus_vid_info_vid_info_keep_format_nop_with_tag_without_tag(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *vid_info_ptr)
{
    uint32                  unit = 0;
    uint32                  outer_inner;
    int32                   ret = RT_ERR_FAILED;
    uint8                   vid_ctrl;
    uint8                   tag_op;
    rtk_pie_actionTable_t   action;
#define OUTER   1
#define INNER   2

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(4,0))
    {
        case 'i': /* inner-tag */
            outer_inner = INNER;
            break;
        case 'o': /* outer-tag */
            outer_inner = OUTER;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(5,0))
    {
        case 'a': /* as-vid-info */
            vid_ctrl = 1;
            break;
        case 'm': /* minus-vid-info */
            vid_ctrl = 2;
            break;
        case 'p': /* plus-vid-info */
            vid_ctrl = 3;
            break;
        case 'w': /* without-tag */
            vid_ctrl = 0;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(7,0))
    {
        case 'k': /* keep-format */
            tag_op = 2;
            break;
        case 'n': /* nop */
            tag_op = 3;
            break;
        case 'w':
            switch (TOKEN_CHAR(7,4))
            {
                case '-': /* with-tag */
                    tag_op = 1;
                    break;
                case 'o': /* without-tag */
                    tag_op = 0;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    if (INNER == outer_inner)
    {
        action.inner_tag_op_field.inner_vid_ctrl = vid_ctrl;
        action.inner_tag_op_field.inner_vid_info = *vid_info_ptr;
        action.inner_tag_op_field.inner_tag_op = tag_op;
    }
    else
    {
        action.outer_tag_op_field.outer_vid_ctrl = vid_ctrl;
        action.outer_tag_op_field.outer_vid_info = *vid_info_ptr;
        action.outer_tag_op_field.outer_tag_op = tag_op;
    }
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_inner_tag_outer_tag_as_vid_info_minus_vid_info_plus_vid_info_vid_info_keep_format_nop_with_tag_without_tag */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_COPY_TO_CPU_DSCP_SPID_HIT_INDICATION_INNER_TAG_MIRROR_OUTER_TAG_POLICE_OUTER_PRI_PRIORITY_REDIRECT_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> ( copy-to-cpu | dscp-spid | hit-indication | inner-tag | mirror | outer-tag | police-outer-pri | priority | redirect ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_copy_to_cpu_dscp_spid_hit_indication_inner_tag_mirror_outer_tag_police_outer_pri_priority_redirect_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(6,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    switch (TOKEN_CHAR(4,0))
    {
        case 'c': /* copy-to-cpu */
            action.cp2cpu = enable;
            break;
        case 'd': /* dscp-spid */
            action.dscp_remark_spid = enable;
            break;
        case 'h': /* hit-indication */
            action.hit_indication = enable;
            break;
        case 'i': /* inner-tag */
            action.inner_tag_op = enable;
            break;
        case 'm': /* mirror */
            action.mirror = enable;
            break;
        case 'p':
            switch (TOKEN_CHAR(4,1))
            {
                case 'o': /* police-outer-pri */
                    action.police_outer_pri_remark = enable;
                    break;
                case 'r': /* priority */
                    action.priority = enable;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'r': /* redirect */
            action.redirect = enable;
            break;
        case 'o': /* outer-tag */
            action.outer_tag_op = enable;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_copy_to_cpu_dscp_spid_hit_indication_inner_tag_mirror_outer_tag_police_outer_pri_priority_redirect_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_REDIRECT_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> redirect withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_redirect_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.redirect_field.withdraw = enable;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_redirect_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_REDIRECT_UNI_REDIRECT_WITH_CPU_TAG_WITHOUT_CPU_TAG_DPN
/*
 * pie set action <UINT:index> redirect uni-redirect ( with-cpu-tag | without-cpu-tag ) <UINT:dpn>
 */
cparser_result_t cparser_cmd_pie_set_action_index_redirect_uni_redirect_with_cpu_tag_without_cpu_tag_dpn(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *dpn_ptr)
{
    uint32      unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    switch (TOKEN_CHAR(6,4))
    {
        case '-': /* with-cpu-tag */
            action.redirect_field.un.uniRedirect.cpu_tag = 1;
            break;
        case 'o': /* without-cpu-tag */
            action.redirect_field.un.uniRedirect.cpu_tag = 0;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    action.redirect_field.un.uniRedirect.dpn = *dpn_ptr;
    action.redirect_field.opcode = REDIRECT_UNI_REDIRECT;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_redirect_uni_redirect_with_cpu_tag_without_cpu_tag_dpn */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_REDIRECT_MULTI_REDIRECT_FWD_INDEX
/*
 * pie set action <UINT:index> redirect multi-redirect <UINT:fwd_index>
 */
cparser_result_t cparser_cmd_pie_set_action_index_redirect_multi_redirect_fwd_index(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *fwd_index_ptr)
{
    uint32      unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.redirect_field.un.multiRedirect.fwd_idx = *fwd_index_ptr;
    action.redirect_field.opcode = REDIRECT_MULTI_REDIRECT;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_redirect_multi_redirect_fwd_index */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_REDIRECT_MULTI_ROUTE_UNI_ROUTE_TTL_KEEP_MINUS_ONE_LOOKUP_INDEX
/*
 * pie set action <UINT:index> redirect ( multi-route | uni-route ) ttl ( keep | minus-one ) <UINT:lookup_index>
 */
cparser_result_t cparser_cmd_pie_set_action_index_redirect_multi_route_uni_route_ttl_keep_minus_one_lookup_index(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *lookup_index_ptr)
{
    uint32      unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);
    switch (TOKEN_CHAR(5,0))
    {
        case 'm': /* multi-route */
            action.redirect_field.opcode = REDIRECT_MULTI_ROUTE;
            break;
        case 'u': /* uni-route */
            action.redirect_field.opcode = REDIRECT_UNI_ROUTE;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    switch (TOKEN_CHAR(7,0))
    {
        case 'k': /* keep */
            action.redirect_field.un.route.ttl_dec = 0;
            break;
        case 'm': /* minus-one */
            action.redirect_field.un.route.ttl_dec = 1;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    action.redirect_field.un.route.lookup_idx = *lookup_index_ptr;

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_redirect_multi_route_uni_route_ttl_keep_minus_one_lookup_index */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_PRIORITY_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> priority withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_priority_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.priority_field.withdraw = enable;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_priority_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_PRIORITY_SEL_PRI_DP
/*
 * pie set action <UINT:index> priority <UINT:sel_pri_dp>
 */
cparser_result_t cparser_cmd_pie_set_action_index_priority_sel_pri_dp(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *sel_pri_dp_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.priority_field.sel_pri_dp = *sel_pri_dp_ptr;
    action.priority_field.assign_pri = DISABLED;
    action.priority_field.acl_pri = 0;
    action.priority_field.assign_dp = DISABLED;
    action.priority_field.acl_dp = 0;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_priority_sel_pri_dp */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_PRIORITY_SEL_PRI_DP_PRIORITY_PRIORITY
/*
 * pie set action <UINT:index> priority <UINT:sel_pri_dp> priority <UINT:priority>
 */
cparser_result_t cparser_cmd_pie_set_action_index_priority_sel_pri_dp_priority_priority(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *sel_pri_dp_ptr,
    uint32_t *priority_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.priority_field.sel_pri_dp = *sel_pri_dp_ptr;
    action.priority_field.assign_pri = ENABLED;
    action.priority_field.acl_pri = *priority_ptr;
    action.priority_field.assign_dp = DISABLED;
    action.priority_field.acl_dp = 0;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_priority_sel_pri_dp_priority_priority */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_PRIORITY_SEL_PRI_DP_PRIORITY_PRIORITY_DP_DP
/*
 * pie set action <UINT:index> priority <UINT:sel_pri_dp> priority <UINT:priority> dp <UINT:dp>
 */
cparser_result_t cparser_cmd_pie_set_action_index_priority_sel_pri_dp_priority_priority_dp_dp(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *sel_pri_dp_ptr,
    uint32_t *priority_ptr,
    uint32_t *dp_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.priority_field.sel_pri_dp = *sel_pri_dp_ptr;
    action.priority_field.assign_pri = ENABLED;
    action.priority_field.acl_pri = *priority_ptr;
    action.priority_field.assign_dp = ENABLED;
    action.priority_field.acl_dp = *dp_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_priority_sel_pri_dp_priority_priority_dp_dp */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_PRIORITY_SEL_PRI_DP_DP_DP
/*
 * pie set action <UINT:index> priority <UINT:sel_pri_dp> dp <UINT:dp>
 */
cparser_result_t cparser_cmd_pie_set_action_index_priority_sel_pri_dp_dp_dp(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *sel_pri_dp_ptr,
    uint32_t *dp_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.priority_field.sel_pri_dp = *sel_pri_dp_ptr;
    action.priority_field.assign_pri = DISABLED;
    action.priority_field.acl_pri = 0;
    action.priority_field.assign_dp = ENABLED;
    action.priority_field.acl_dp = *dp_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_priority_sel_pri_dp_dp_dp */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_SPID_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> spid withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_spid_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.dscp_remark_spid_field.withdraw = enable;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_spid_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_SPID_SPID_INDEX
/*
 * pie set action <UINT:index> spid <UINT:spid_index>
 */
cparser_result_t cparser_cmd_pie_set_action_index_spid_spid_index(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *spid_index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.dscp_remark_spid_field.un.spid.spid_idx = *spid_index_ptr;
    action.dscp_remark_spid_field.un.spid.reserved = 0;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_spid_spid_index */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_DSCP_REMARK_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> dscp-remark withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_dscp_remark_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.dscp_remark_spid_field.withdraw = enable;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_dscp_remark_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_DSCP_REMARK_DSCP_DSCP
/*
 * pie set action <UINT:index> dscp-remark dscp <UINT:dscp>
 */
cparser_result_t cparser_cmd_pie_set_action_index_dscp_remark_dscp_dscp(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *dscp_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.dscp_remark_spid_field.un.dscp_remark.un.dscp.opcode = 0;
    action.dscp_remark_spid_field.un.dscp_remark.un.dscp.dscp = *dscp_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_dscp_remark_dscp_dscp */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_DSCP_REMARK_IP_PRECEDENCE_PRECEDENCE
/*
 * pie set action <UINT:index> dscp-remark ip-precedence <UINT:precedence>
 */
cparser_result_t cparser_cmd_pie_set_action_index_dscp_remark_ip_precedence_precedence(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *precedence_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.dscp_remark_spid_field.un.dscp_remark.un.ipPrecedence.opcode = 1;
    action.dscp_remark_spid_field.un.dscp_remark.un.ipPrecedence.ip_precedence = *precedence_ptr;
    action.dscp_remark_spid_field.un.dscp_remark.un.ipPrecedence.reserved = 0;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_dscp_remark_ip_precedence_precedence */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_DSCP_REMARK_DRT_BITES_DRT
/*
 * pie set action <UINT:index> dscp-remark drt-bites <UINT:drt>
 */
cparser_result_t cparser_cmd_pie_set_action_index_dscp_remark_drt_bites_drt(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *drt_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.dscp_remark_spid_field.un.dscp_remark.un.dtrBits.opcode = 2;
    action.dscp_remark_spid_field.un.dscp_remark.un.dtrBits.dtr_bits = *drt_ptr;
    action.dscp_remark_spid_field.un.dscp_remark.un.dtrBits.reserved = 0;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_dscp_remark_drt_bites_drt */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_OUTER_PRI_REMARK_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> outer-pri-remark withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_outer_pri_remark_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.police_outer_pri_remark_field.withdraw = enable;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_outer_pri_remark_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_OUTER_PRI_REMARK_PRIORITY_PRIORITY
/*
 * pie set action <UINT:index> outer-pri-remark priority <UINT:priority>
 */
cparser_result_t cparser_cmd_pie_set_action_index_outer_pri_remark_priority_priority(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *priority_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.police_outer_pri_remark_field.un.outer_pri_remark.reserved = 0;
    action.police_outer_pri_remark_field.un.outer_pri_remark.remark_dei = DISABLED;
    action.police_outer_pri_remark_field.un.outer_pri_remark.dei = 0;
    action.police_outer_pri_remark_field.un.outer_pri_remark.remark_pri = ENABLED;
    action.police_outer_pri_remark_field.un.outer_pri_remark.pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_outer_pri_remark_priority_priority */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_OUTER_PRI_REMARK_PRIORITY_PRIORITY_DEI_DEI
/*
 * pie set action <UINT:index> outer-pri-remark priority <UINT:priority> dei <UINT:dei>
 */
cparser_result_t cparser_cmd_pie_set_action_index_outer_pri_remark_priority_priority_dei_dei(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *priority_ptr,
    uint32_t *dei_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.police_outer_pri_remark_field.un.outer_pri_remark.reserved = 0;
    action.police_outer_pri_remark_field.un.outer_pri_remark.remark_dei = ENABLED;
    action.police_outer_pri_remark_field.un.outer_pri_remark.dei = *dei_ptr;
    action.police_outer_pri_remark_field.un.outer_pri_remark.remark_pri = ENABLED;
    action.police_outer_pri_remark_field.un.outer_pri_remark.pri = *priority_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_outer_pri_remark_priority_priority_dei_dei */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_OUTER_PRI_REMARK_DEI_DEI
/*
 * pie set action <UINT:index> outer-pri-remark dei <UINT:dei>
 */
cparser_result_t cparser_cmd_pie_set_action_index_outer_pri_remark_dei_dei(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *dei_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.police_outer_pri_remark_field.un.outer_pri_remark.reserved = 0;
    action.police_outer_pri_remark_field.un.outer_pri_remark.remark_dei = ENABLED;
    action.police_outer_pri_remark_field.un.outer_pri_remark.dei = *dei_ptr;
    action.police_outer_pri_remark_field.un.outer_pri_remark.remark_pri = DISABLED;
    action.police_outer_pri_remark_field.un.outer_pri_remark.pri = 0;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_outer_pri_remark_dei_dei */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_POLICE_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> police withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_police_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.police_outer_pri_remark_field.withdraw = enable;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* cparser_cmd_pie_set_action_index_police_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_POLICE_POLICE_INDEX
/*
 * pie set action <UINT:index> police <UINT:police_index>
 */
cparser_result_t cparser_cmd_pie_set_action_index_police_police_index(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *police_index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.police_outer_pri_remark_field.un.policer_idx = *police_index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_police_police_index */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_MIRROR_WITHDRAW_STATE_DISABLE_ENABLE
/*
 * pie set action <UINT:index> mirror withdraw state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_action_index_mirror_withdraw_state_disable_enable(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enable;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.mirror_field.withdraw = enable;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_mirror_withdraw_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_ACTION_INDEX_MIRROR_MIRROR_INDEX
/*
 * pie set action <UINT:index> mirror <UINT:mirror_index>
 */
cparser_result_t cparser_cmd_pie_set_action_index_mirror_mirror_index(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *mirror_index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    action.mirror_field.mirror_idx = *mirror_index_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_set(unit, *index_ptr, &action), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_index_mirror_mirror_index */
#endif

#ifdef CMD_PIE_SET_ACTION_ENTRY_ACTION_AND_ENTRY_MOVE_SWAP_FROM_TO_ENTRY_LEN_DECREASE_INCREASE
/*
 * pie set ( action | entry | action_and_entry ) ( move | swap ) <UINT:from> <UINT:to> <UINT:entry_len> ( decrease | increase )
 */
cparser_result_t cparser_cmd_pie_set_action_entry_action_and_entry_move_swap_from_to_entry_len_decrease_increase(cparser_context_t *context,
    uint32_t *from_ptr,
    uint32_t *to_ptr,
    uint32_t *entry_len_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_pie_movePieDirection_t  direction = PIE_DIRECTION_END;
    rtk_pie_movePieContent_t    content;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(7,0))
    {
        case 'd': /* decrease */
            direction = DIRECTION_DECREASE;
            break;
        case 'i': /* increase */
            direction = DIRECTION_INCREASE;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&content, 0, sizeof(rtk_pie_movePieContent_t));
    content.entry_length = *entry_len_ptr;
    content.direction = direction;
    content.move_to = *to_ptr;
    content.move_from = *from_ptr;
    switch (TOKEN_CHAR(2,0))
    {
        case 'a': /* action & action_and_entry */
            if (TOKEN_CHAR(2,6) == '_') /* action_and_entry */
            {
                switch (TOKEN_CHAR(3,0))
                {
                    case 'm': /* move */
                        if ((ret = rtk_pie_pieRuleEntryAction_move(unit, &content)) != RT_ERR_OK)
                        {
                            DIAG_ERR_PRINT(ret);
                            return CPARSER_NOT_OK;
                        } 
                        break;
                    case 's': /* swap */
                        if ((ret = rtk_pie_pieRuleEntryAction_swap(unit, &content)) != RT_ERR_OK)
                        {
                            DIAG_ERR_PRINT(ret);
                            return CPARSER_NOT_OK;
                        } 
                        break;       
                    default: 
                        diag_util_printf("User config: Error!\n");
                        return CPARSER_NOT_OK;
                }
            }
            else /* action */
            {
                switch (TOKEN_CHAR(3,0))
                {
                    case 'm': /* move */
                        if ((ret = rtk_pie_pieRuleAction_move(unit, &content)) != RT_ERR_OK)
                        {
                            DIAG_ERR_PRINT(ret);
                            return CPARSER_NOT_OK;
                        } 
                        break;
                    case 's': /* swap */
                        if ((ret = rtk_pie_pieRuleAction_swap(unit, &content)) != RT_ERR_OK)
                        {
                            DIAG_ERR_PRINT(ret);
                            return CPARSER_NOT_OK;
                        } 
                        break;       
                    default: 
                        diag_util_printf("User config: Error!\n");
                        return CPARSER_NOT_OK;
                }
            }
            break;
        case 'e': /* entry */
            switch (TOKEN_CHAR(3,0))
            {
                case 'm': /* move */
                    if ((ret = rtk_pie_pieRuleEntry_move(unit, &content)) != RT_ERR_OK)
                    {
                        DIAG_ERR_PRINT(ret);
                        return CPARSER_NOT_OK;
                    } 
                    break;
                case 's': /* swap */
                    if ((ret = rtk_pie_pieRuleEntry_swap(unit, &content)) != RT_ERR_OK)
                    {
                        DIAG_ERR_PRINT(ret);
                        return CPARSER_NOT_OK;
                    } 
                    break;       
                default: 
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;       
        default: 
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_action_entry_action_and_entry_move_swap_from_to_entry_len_decrease_increase */
#endif

#ifdef CMD_PIE_SET_POLICE_INDEX_INVALID
/*
 * pie set police <UINT:index> invalid
 */
cparser_result_t cparser_cmd_pie_set_police_index_invalid(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_policerEntry_t  policer;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&policer, 0, sizeof(rtk_pie_policerEntry_t));
    policer.type = POLICER_TYPE_INVALID;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRulePolicer_set(unit, *index_ptr, &policer), ret);

    return CPARSER_OK;
}/* pie set police <UINT:index> invalid */
#endif

#ifdef CMD_PIE_SET_POLICE_INDEX_DLB_SRTCM_TRTCM_YELLOW_DP_RED_DP_PIR_CIR_PBS_CBS
/*
 * pie set police <UINT:index> ( dlb | srtcm | trtcm ) <UINT:yellow_dp> <UINT:red_dp> <UINT:pir> <UINT:cir> <UINT:pbs> <UINT:cbs>
 */
cparser_result_t cparser_cmd_pie_set_police_index_dlb_srtcm_trtcm_yellow_dp_red_dp_pir_cir_pbs_cbs(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *yellow_dp_ptr,
    uint32_t *red_dp_ptr,
    uint32_t *pir_ptr,
    uint32_t *cir_ptr,
    uint32_t *pbs_ptr,
    uint32_t *cbs_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_policerEntry_t  policer;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&policer, 0, sizeof(rtk_pie_policerEntry_t));

    switch (TOKEN_CHAR(4,0))
    {
        case 'd': /* dlb */
            policer.type = POLICER_TYPE_DLB;
            break;
        case 's': /* srtcm */
            policer.type = POLICER_TYPE_SRTCM;
            break;
        case 't': /* trtcm */
            policer.type = POLICER_TYPE_TRTCM;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    policer.color_aware = 0;
    policer.yellow_dp = *yellow_dp_ptr;
    policer.red_dp = *red_dp_ptr;
    policer.pir = *pir_ptr;
    policer.cir = *cir_ptr;
    policer.tp = 0;
    policer.tc = 0;
    policer.pbs = *pbs_ptr;
    policer.cbs = *cbs_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRulePolicer_set(unit, *index_ptr, &policer), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_police_index_dlb_srtcm_trtcm_yellow_dp_red_dp_pir_cir_pbs_cbs */
#endif

#ifdef CMD_PIE_SET_POLICE_INDEX_DLB_SRTCM_TRTCM_COLOR_AWARE_YELLOW_DP_RED_DP_PIR_CIR_PBS_CBS
/*
 * pie set police <UINT:index> ( dlb | srtcm | trtcm ) color-aware <UINT:yellow_dp> <UINT:red_dp> <UINT:pir> <UINT:cir> <UINT:pbs> <UINT:cbs>
 */
cparser_result_t cparser_cmd_pie_set_police_index_dlb_srtcm_trtcm_color_aware_yellow_dp_red_dp_pir_cir_pbs_cbs(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *yellow_dp_ptr,
    uint32_t *red_dp_ptr,
    uint32_t *pir_ptr,
    uint32_t *cir_ptr,
    uint32_t *pbs_ptr,
    uint32_t *cbs_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_policerEntry_t  policer;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&policer, 0, sizeof(rtk_pie_policerEntry_t));

    switch (TOKEN_CHAR(4,0))
    {
        case 'd': /* dlb */
            policer.type = POLICER_TYPE_DLB;
            break;
        case 's': /* srtcm */
            policer.type = POLICER_TYPE_SRTCM;
            break;
        case 't': /* trtcm */
            policer.type = POLICER_TYPE_TRTCM;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    policer.color_aware = 1;
    policer.yellow_dp = *yellow_dp_ptr;
    policer.red_dp = *red_dp_ptr;
    policer.pir = *pir_ptr;
    policer.cir = *cir_ptr;
    policer.tp = 0;
    policer.tc = 0;
    policer.pbs = *pbs_ptr;
    policer.cbs = *cbs_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRulePolicer_set(unit, *index_ptr, &policer), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_police_index_dlb_srtcm_trtcm_color_aware_yellow_dp_red_dp_pir_cir_pbs_cbs */
#endif

#ifdef CMD_PIE_SET_COUNTER_INDEX_BYTE_PACKET_COUNTER
/*
 * pie set counter <UINT:index> ( byte | packet ) <UINT64:counter>
 */
cparser_result_t cparser_cmd_pie_set_counter_index_byte_packet_counter(cparser_context_t *context,
    uint32_t *index_ptr,
    uint64_t *counter_ptr)
{
    uint64      byte_cnt = 0;
    uint32      unit = 0;
    uint32      pkt_cnt = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_pie_pieStat_get(unit, *index_ptr, &pkt_cnt, &byte_cnt), ret);

    switch (TOKEN_CHAR(4,0))
    {
        case 'b': /* byte */
            byte_cnt = *counter_ptr;
            break;
        case 'p': /* paket */
            pkt_cnt = *counter_ptr;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieStat_set(unit, *index_ptr, pkt_cnt, byte_cnt), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_counter_index_byte_packet_counter */
#endif

#ifdef CMD_PIE_SET_COUNTER_CLEAR_ALL
/*
 * pie set counter clear-all
 */
cparser_result_t cparser_cmd_pie_set_counter_clear_all(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_pie_pieStat_clearAll(unit), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_counter_clear_all */
#endif

#ifdef CMD_PIE_SET_COUNTER_INDICATION_MODE_LBLOCK_INDEX_ACTION_EXECUTION_RULE_MATCH
/*
 * pie set counter-indication-mode <UINT:lblock_index> ( action-execution | rule-match )
 */
cparser_result_t cparser_cmd_pie_set_counter_indication_mode_lblock_index_action_execution_rule_match(cparser_context_t *context,
    uint32_t *lblock_index_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_pie_counterIndicationMode_t mode = PIE_INDICATION_MODE_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(4,0))
    {
        case 'a': /* action-execution */
            mode = ACTION_EXECUTION;
            break;
        case 'r': /* rule-match */
            mode = RULE_MATCH;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieCounterIndicationMode_set(unit, *lblock_index_ptr, mode), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_counter_indication_mode_lblock_index_action_execution_rule_match */
#endif

#ifdef CMD_PIE_SET_SELECTOR_PBLOCK_PHASE_TEMPLATE_INDEX
/*
 * pie set selector <UINT:pblock> <UINT:phase> <UINT:template_index>
 */
cparser_result_t cparser_cmd_pie_set_selector_pblock_phase_template_index(cparser_context_t *context,
    uint32_t *pblock_ptr,
    uint32_t *phase_ptr,
    uint32_t *template_index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_pie_pieTemplateSelector_set(unit, *pblock_ptr, *phase_ptr, *template_index_ptr), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_selector_pblock_phase_template_index */
#endif

#ifdef CMD_PIE_SET_TEMPLATE_INDEX_FIELD_INDEX_DIP0_DIP1_DIP2_DIP3_DIP4_DIP5_DIP6_DIP7_DMAC0_DMAC1_RRCPINFO_DMAC2_DPM1_DPM2_DPN_ETHERTYPE_FIELD_SELECTOR1_0_FIELD_SELECTOR1_1_FIELD_SELECTOR1_2_FIELD_SELECTOR1_3_FIELD_SELECTOR2_0_FIELD_SELECTOR2_1_FIELD_SELECTOR2_2_FIELD_SELECTOR2_3_FMT_FWD_ITAG_FWD_OTAG_DPRI_FWD_VID_GUEST_VLAN_ICMP_CODE_TYPE_IGMP_TYPE_IP6_FLWH_IP_RANGE1_IP_RANGE2_IP_RANGE3_IP_RANGE4_ITAG_L4_DPORT_L4_SPORT_OTAG_PATTERN_MATCH0_PATTERN_MATCH1_PAYLOAD0_PAYLOAD1_PORT_RANGE_SIP0_SIP1_SIP2_SIP3_SIP4_SIP5_SIP6_SIP7_SMAC0_SMAC1_SMAC2_SPM1_SPM2_SPSM_TCP_FLAG_TOS_PROTO_TTL_FLAG_VID_RANGE
/*
 * pie set template <UINT:index> <UINT:field_index> ( dip0 | dip1 | dip2 | dip3 | dip4 | dip5 | dip6 | dip7 | dmac0 | dmac1-rrcpinfo | dmac2 | dpm1 | dpm2 | dpn | ethertype | field-selector1_0 | field-selector1_1 | field-selector1_2 | field-selector1_3 | field-selector2_0 | field-selector2_1 | field-selector2_2 | field-selector2_3 | fmt | fwd-itag | fwd-otag-dpri | fwd-vid | guest-vlan | icmp-code-type | igmp-type | ip6-flwh | ip-range1 | ip-range2 | ip-range3 | ip-range4 | itag | l4-dport | l4-sport | otag | pattern-match0 | pattern-match1 | payload0 | payload1 | port-range | sip0 | sip1 | sip2 | sip3 | sip4 | sip5 | sip6 | sip7 | smac0 | smac1 | smac2 | spm1 | spm2 | spsm | tcp-flag | tos-proto | ttl-flag | vid-range )
 */
cparser_result_t cparser_cmd_pie_set_template_index_field_index_dip0_dip1_dip2_dip3_dip4_dip5_dip6_dip7_dmac0_dmac1_rrcpinfo_dmac2_dpm1_dpm2_dpn_ethertype_field_selector1_0_field_selector1_1_field_selector1_2_field_selector1_3_field_selector2_0_field_selector2_1_field_selector2_2_field_selector2_3_fmt_fwd_itag_fwd_otag_dpri_fwd_vid_guest_vlan_icmp_code_type_igmp_type_ip6_flwh_ip_range1_ip_range2_ip_range3_ip_range4_itag_l4_dport_l4_sport_otag_pattern_match0_pattern_match1_payload0_payload1_port_range_sip0_sip1_sip2_sip3_sip4_sip5_sip6_sip7_smac0_smac1_smac2_spm1_spm2_spsm_tcp_flag_tos_proto_ttl_flag_vid_range(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *field_index_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_pie_templateFiledType_t type = PIE_FIELD_TYPE_END;
    rtk_pie_template_t          template;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(5,0))
    {
        case 'd':
            switch (TOKEN_CHAR(5,1))
            {
                case 'i':
                    switch (TOKEN_CHAR(5,3))
                    {
                        case '0': /* dip0 */
                            type = DIP0;
                            break;
                        case '1': /* dip1 */
                            type = DIP1;
                            break;
                        case '2': /* dip2 */
                            type = DIP2;
                            break;
                        case '3': /* dip3 */
                            type = DIP3;
                            break;
                        case '4': /* dip4 */
                            type = DIP4;
                            break;
                        case '5': /* dip5 */
                            type = DIP5;
                            break;
                        case '6': /* dip6 */
                            type = DIP6;
                            break;
                        case '7': /* dip7 */
                            type = DIP7;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'm':
                    switch (TOKEN_CHAR(5,4))
                    {
                        case '0': /* dmac0 */
                            type = DMAC0;
                            break;
                        case '1': /* dmac1-rrcpinfo */
                            type = DMAC1_RRCPINFO;
                            break;
                        case '2': /* dmac2 */
                            type = DMAC2;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'p':
                    switch (TOKEN_CHAR(5,2))
                    {
                        case 'm':
                            switch (TOKEN_CHAR(5,3))
                            {
                                case '1': /* dpm1 */
                                    type = DPM1;
                                    break;
                                case '2': /* dpm2 */
                                    type = DPM2;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'n': /* dpn */
                            type = DPN;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'e': /* ethertype */
            type = ETHERTYPE;
            break;
        case 'f':
            switch (TOKEN_CHAR(5,1))
            {
                case 'i':
                    switch (TOKEN_CHAR(5,14))
                    {
                        case '1':
                            switch (TOKEN_CHAR(5,16))
                            {
                                case '0': /* field-selector1_0 */
                                    type = FIELD_SELECTOR1_0;
                                    break;
                                case '1': /* field-selector1_1 */
                                    type = FIELD_SELECTOR1_1;
                                    break;
                                case '2': /* field-selector1_2 */
                                    type = FIELD_SELECTOR1_2;
                                    break;
                                case '3': /* field-selector1_3 */
                                    type = FIELD_SELECTOR1_3;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case '2':
                            switch (TOKEN_CHAR(5,16))
                            {
                                case '0': /* field-selector2_0 */
                                    type = FIELD_SELECTOR2_0;
                                    break;
                                case '1': /* field-selector2_1 */
                                    type = FIELD_SELECTOR2_1;
                                    break;
                                case '2': /* field-selector2_2 */
                                    type = FIELD_SELECTOR2_2;
                                    break;
                                case '3': /* field-selector2_3 */
                                    type = FIELD_SELECTOR2_3;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'm': /* fmt */
                    type = FMT;
                    break;
                case 'w':
                    switch (TOKEN_CHAR(5,4))
                    {
                        case 'i': /* fwd-itag */
                            type = FWD_ITAG;
                            break;
                        case 'o': /* fwd-otag-dpri */
                            type = FWD_OTAG_DPRI;
                            break;
                        case 'v': /* fwd-vid */
                            type = FWD_VID;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'g': /* guest-vlan */
            type = GUEST_VLAN;
            break;
        case 'i':
            switch (TOKEN_CHAR(5,1))
            {
                case 'c': /* icmp-code-type */
                    type = ICMP_CODE_TYPE;
                    break;
                case 'g': /* igmp-type */
                    type = IGMP_TYPE;
                    break;
                case 'p':
                    switch (TOKEN_CHAR(5,2))
                    {
                        case '6': /* ip6-flwh */
                            type = IP6_FLWH;
                            break;
                        case '-':
                            switch (TOKEN_CHAR(5,8))
                            {
                                case '1': /* ip-range1 */
                                    type = IP_RANG1;
                                    break;
                                case '2': /* ip-range2 */
                                    type = IP_RANG2;
                                    break;
                                case '3': /* ip-range3 */
                                    type = IP_RANG3;
                                    break;
                                case '4': /* ip-range4 */
                                    type = IP_RANG4;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 't': /* itag */
                    type = INTAG;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'l':
            switch (TOKEN_CHAR(5,3))
            {
                case 'd': /* l4-dport */
                    type = L4_DPORT;
                    break;
                case 's': /* l4-sport */
                    type = L4_SPORT;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'o': /* otag */
            type = OTAG;
            break;
        case 'p':
            switch (TOKEN_CHAR(5,1))
            {
                case 'a':
                    switch (TOKEN_CHAR(5,2))
                    {
                        case 't':
                            switch (TOKEN_CHAR(5,13))
                            {
                                case '0': /* pattern-match0 */
                                    type = PATTERN_MATCH0;
                                    break;
                                case '1': /* pattern-match1 */
                                    type = PATTERN_MATCH1;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'y':
                            switch (TOKEN_CHAR(5,7))
                            {
                                case '0': /* payload0 */
                                    type = PAYLOAD0;
                                    break;
                                case '1': /* payload1 */
                                    type = PAYLOAD1;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'o': /* port-range */
                    type = PORT_RANG;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 's':
            switch (TOKEN_CHAR(5,1))
            {
                case 'i':
                    switch (TOKEN_CHAR(5,3))
                    {
                        case '0': /* sip0 */
                            type = SIP0;
                            break;
                        case '1': /* sip1 */
                            type = SIP1;
                            break;
                        case '2': /* sip2 */
                            type = SIP2;
                            break;
                        case '3': /* sip3 */
                            type = SIP3;
                            break;
                        case '4': /* sip4 */
                            type = SIP4;
                            break;
                        case '5': /* sip5 */
                            type = SIP5;
                            break;
                        case '6': /* sip6 */
                            type = SIP6;
                            break;
                        case '7': /* sip7 */
                            type = SIP7;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'm':
                    switch (TOKEN_CHAR(5,4))
                    {
                        case '0': /* smac0 */
                            type = SMAC0;
                            break;
                        case '1': /* smac1 */
                            type = SMAC1;
                            break;
                        case '2': /* smac2 */
                            type = SMAC2;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'p':
                    switch (TOKEN_CHAR(5,2))
                    {
                        case 'm':
                            switch (TOKEN_CHAR(5,3))
                            {
                                case '1': /* spm1 */
                                    type = SPM1;
                                    break;
                                case '2': /* spm2 */
                                    type = SPM2;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 's': /* spsm */
                            type = SPSM;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 't':
            switch (TOKEN_CHAR(5,1))
            {
                case 'c': /* tcp-flag */
                    type = TCP_FLAG;
                    break;
                case 'o': /* tos-proto */
                    type = IP_TOS_PROTO;
                    break;
                case 't': /* ttl-flag */
                    type = IP4_TTL_FLAG;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'v': /* vid-range */
            type = VID_RANG;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    memset(&template, 0 , sizeof(rtk_pie_template_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieUserTemplate_get(unit, *index_ptr, &template), ret);

    if ((0 == template.field[0]) && (0 == template.field[1]) && (0 == template.field[2]) &&
        (0 == template.field[3]) && (0 == template.field[4]) && (0 == template.field[5]) &&
        (0 == template.field[6]) && (0 == template.field[7]) && (0 == template.field[8]))
    {
        template.field[0] = DMAC1_RRCPINFO;
        template.field[1] = DMAC2;
        template.field[2] = ETHERTYPE;
        template.field[3] = DMAC0;
        template.field[4] = SMAC1;
        template.field[5] = SMAC2;
        template.field[6] = DPN;
        template.field[7] = SMAC0;
        template.field[8] = FMT;
    }

    template.field[*field_index_ptr] = type;
    DIAG_UTIL_ERR_CHK(rtk_pie_pieUserTemplate_set(unit, *index_ptr, &template), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_template_index_field_index_dip0_dip1_dip2_dip3_dip4_dip5_dip6_dip7_dmac0_dmac1_rrcpinfo_dmac2_dpm1_dpm2_dpn_ethertype_field_selector1_0_field_selector1_1_field_selector1_2_field_selector1_3_field_selector2_0_field_selector2_1_field_selector2_2_field_selector2_3_fmt_fwd_itag_fwd_otag_dpri_fwd_vid_guest_vlan_icmp_code_type_igmp_type_ip6_flwh_ip_range1_ip_range2_ip_range3_ip_range4_itag_l4_dport_l4_sport_otag_pattern_match0_pattern_match1_payload0_payload1_port_range_sip0_sip1_sip2_sip3_sip4_sip5_sip6_sip7_smac0_smac1_smac2_spm1_spm2_spsm_tcp_flag_tos_proto_ttl_flag_vid_range */
#endif

#ifdef CMD_PIE_SET_L34_CHECKSUM_ERR_DOWNGRADE_PARSE_ANYWAY
/*
 * pie set l34-checksum-err ( downgrade | parse-anyway )
 */
cparser_result_t cparser_cmd_pie_set_l34_checksum_err_downgrade_parse_anyway(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_pie_l34ChecksumErrOper_t    operation = PIE_CHECKSUM_ERR_OPER_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(3,0))
    {
        case 'd': /* downgrade */
            operation = CHECKSUM_ERR_DOWNGRADE;
            break;
        case 'p': /* parse-anyway */
            operation = CHECKSUM_ERR_PARSE_ANYWAY;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieL34ChecksumErr_set(unit, operation), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_l34_checksum_err_downgrade_parse_anyway */
#endif

#ifdef CMD_PIE_SET_PAYLOAD_PBLOCK_INDEX_OFFSET
/*
 * pie set payload <UINT:pblock> <UINT:index> <UINT:offset>
 */
cparser_result_t cparser_cmd_pie_set_payload_pblock_index_offset(cparser_context_t *context,
    uint32_t *pblock_ptr,
    uint32_t *index_ptr,
    uint32_t *offset_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_pie_pieUserTemplatePayloadOffset_set(unit, *pblock_ptr, *index_ptr, *offset_ptr), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_payload_pblock_index_offset */
#endif

#ifdef CMD_PIE_SET_RESULT_REVERSE_ENTRY_INDEX_KEEP_REVERSE
/*
 * pie set result-reverse <UINT:entry_index> ( keep | reverse )
 */
cparser_result_t cparser_cmd_pie_set_result_reverse_entry_index_keep_reverse(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32                       unit = 0;
    int32                        ret = RT_ERR_FAILED;
    rtk_pie_resultReverseOper_t  operation = PIE_RESULT_REVERSE_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(4,0))
    {
        case 'k': /* keep result */
            operation = RESULT_KEEP;
            break;
        case 'r': /* reverse result */
            operation = RESULT_REVERSE;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieResultReverse_set(unit, *entry_index_ptr, operation), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_result_reverse_entry_index_keep_reverse */
#endif

#ifdef CMD_PIE_SET_RESULT_AGGREGATOR_PBLOCK_RANGE_INDEX_ENTRY_INDEX_TYPE_A01_23M0_2_A01_23M0_3_A01_23M1_2_A01_23M1_3_A0123M0_A0123M1_A0123M2_A0123M3_A01M0_A01M1_A23M2_A23M3_NONE
/*
 * pie set result-aggregator <UINT:pblock_range_index> <UINT:entry_index> type ( a01_23m0_2 | a01_23m0_3 | a01_23m1_2 | a01_23m1_3 | a0123m0 | a0123m1 | a0123m2 | a0123m3 | a01m0 | a01m1 | a23m2 | a23m3 | none )
 */
cparser_result_t cparser_cmd_pie_set_result_aggregator_pblock_range_index_entry_index_type_a01_23m0_2_a01_23m0_3_a01_23m1_2_a01_23m1_3_a0123m0_a0123m1_a0123m2_a0123m3_a01m0_a01m1_a23m2_a23m3_none(cparser_context_t *context,
    uint32_t *pblock_range_index_ptr,
    uint32_t *entry_index_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_pie_resultAggregatorType_t  type = PIE_RESULT_AGGREGATOR_TYPE_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(6,0))
    {
        case 'a':
            switch (TOKEN_CHAR(6,1))
            {
                case '0':
                    switch (TOKEN_CHAR(6,3))
                    {
                        case '_':
                            switch (TOKEN_CHAR(6,7))
                            {
                                case '0':
                                    switch (TOKEN_CHAR(6,9))
                                    {
                                        case '2': /* a01_23m0_2 */
                                            type = A01_23M0_2;
                                            break;
                                        case '3': /* a01_23m0_3 */
                                            type = A01_23M0_3;
                                            break;
                                        default:
                                            diag_util_printf("User config: Error!\n");
                                            return CPARSER_NOT_OK;
                                    }
                                    break;
                                case '1':
                                    switch (TOKEN_CHAR(6,9))
                                    {
                                        case '2': /* a01_23m1_2 */
                                            type = A01_23M1_2;
                                            break;
                                        case '3': /* a01_23m1_3 */
                                            type = A01_23M1_3;
                                            break;
                                        default:
                                            diag_util_printf("User config: Error!\n");
                                            return CPARSER_NOT_OK;
                                    }
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case '2':
                            switch (TOKEN_CHAR(6,6))
                            {
                                case '0': /* a0123m0 */
                                    type = A0123M0;
                                    break;
                                case '1': /* a0123m1 */
                                    type = A0123M1;
                                    break;
                                case '2': /* a0123m2 */
                                    type = A0123M2;
                                    break;
                                case '3': /* a0123m3 */
                                    type = A0123M3;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        case 'm':
                            switch (TOKEN_CHAR(6,4))
                            {
                                case '0': /* a01m0 */
                                    type = A01M0;
                                    break;
                                case '1': /* a01m1 */
                                    type = A01M1;
                                    break;
                                default:
                                    diag_util_printf("User config: Error!\n");
                                    return CPARSER_NOT_OK;
                            }
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case '2':
                    switch (TOKEN_CHAR(6,4))
                    {
                        case '2': /* a23m2 */
                            type = A23M2;
                            break;
                        case '3': /* a23m3 */
                            type = A23M3;
                            break;
                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'n': /* none */
            type = NONE;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieResultAggregator_set(unit, *pblock_range_index_ptr, *entry_index_ptr, type), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_result_aggregator_pblock_range_index_entry_index_type_a01_23m0_2_a01_23m0_3_a01_23m1_2_a01_23m1_3_a0123m0_a0123m1_a0123m2_a0123m3_a01m0_a01m1_a23m2_a23m3_none */
#endif

#ifdef CMD_PIE_SET_BLOCK_PRIORITY_BLOCK_INDEX_PRIORITY
/*
 * pie set block-priority <UINT:block_index> <UINT:priority>
 */
cparser_result_t cparser_cmd_pie_set_block_priority_block_index_priority(cparser_context_t *context,
    uint32_t *block_index_ptr,
    uint32_t *priority_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_pie_pieBlockPriority_set(unit, *block_index_ptr, *priority_ptr), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_block_priority_block_index_priority */
#endif

#ifdef CMD_PIE_SET_GROUP_LBLOCK_RANGE_INDEX_OPERATION_GROUP01_GROUP012_GROUP0123_NONE
/*
 * pie set group <UINT:lblock_range_index> operation ( group01 | group012 | group0123 | none )
 */
cparser_result_t cparser_cmd_pie_set_group_lblock_range_index_operation_group01_group012_group0123_none(cparser_context_t *context,
    uint32_t *lblock_range_index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_pie_groupCtrlRange_t    lblockRange_idx = PIE_GROUP_RANGE_END;
    rtk_pie_groupCtrl_t         operation = PIE_GROUP_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (*lblock_range_index_ptr)
    {
    	case 0:
    	    lblockRange_idx = GROUP_LBLOCK_0_3;
    	    break;
    	case 1:
    	    lblockRange_idx = GROUP_LBLOCK_4_7;
    	    break;
    	case 2:
    	    lblockRange_idx = GROUP_LBLOCK_8_11;
    	    break;
    	case 3:
    	    lblockRange_idx = GROUP_LBLOCK_12_15;
    	    break;
	default:
	    diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(5,0))
    {
    	case 'g':
    	        switch (context->parser->tokens[5].token_len) /* fix me */
                {
                    case 7:
                        operation = GROUP_01;
                        break;
                    case 8:
                        operation = GROUP_012;
                        break;
                    case 9:
                        operation = GROUP_0123;
                        break;
                    default:
                        diag_util_printf("User config: Error!\n");
                        return CPARSER_NOT_OK;
               }
    	    break;
    	case 'n':
    	    operation = GROUP_NONE;
    	    break;
	default:
	    diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieGroupCtrl_set(unit, lblockRange_idx, operation), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_group_lblock_range_index_operation_group01_group012_group0123_none */
#endif

#ifdef CMD_PIE_SET_EGRESS_ACL_LOOKUP_FLOOD_PKT_MISS_MCAST_PKT_MISS_ACTION_DROP_PERMIT
/*
 * pie set egress-acl-lookup ( flood-pkt-miss | mcast-pkt-miss ) action ( drop | permit )
 */
cparser_result_t cparser_cmd_pie_set_egress_acl_lookup_flood_pkt_miss_mcast_pkt_miss_action_drop_permit(cparser_context_t *context)
{
    uint32                       unit = 0;
    int32                        ret = RT_ERR_FAILED;
    rtk_pie_lookupMissAction_t   action = PIE_LOOKUP_MISS_END;
    rtk_pie_egrAclLookupCtrl_t   control;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&control, 0, sizeof(rtk_pie_egrAclLookupCtrl_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieEgrAclLookupCtrl_get(unit, &control), ret);

    switch (TOKEN_CHAR(5,0))
    {
        case 'd': /* drop */
            action = LOOKUP_MISS_DROP;
            break;
        case 'p': /* permit */
            action = LOOKUP_MISS_PERMIT;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(3,0))
    {
        case 'f': /* flood-pkt-miss */
            control.flood_lookupMiss_action = action;
            break;
        case 'm': /* mcast-pkt-miss */
            control.mcast_lookupMiss_action = action;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieEgrAclLookupCtrl_set(unit, &control), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_egress_acl_lookup_flood_pkt_miss_mcast_pkt_miss_action_drop_permit */
#endif

#ifdef CMD_PIE_SET_EGRESS_ACL_LOOKUP_FLOOD_PKT_MISS_MCAST_PKT_MISS_OTHER_DROP_STATE_DISABLE_ENABLE
/*
 * pie set egress-acl-lookup ( flood-pkt-miss | mcast-pkt-miss | other-drop ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_egress_acl_lookup_flood_pkt_miss_mcast_pkt_miss_other_drop_state_disable_enable(cparser_context_t *context)
{
    uint32                       unit = 0;
    int32                        ret = RT_ERR_FAILED;
    rtk_enable_t                 enable;
    rtk_pie_egrAclLookupCtrl_t   control;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&control, 0, sizeof(rtk_pie_egrAclLookupCtrl_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieEgrAclLookupCtrl_get(unit, &control), ret);

    switch (TOKEN_CHAR(5,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(3,0))
    {
        case 'f': /* flood-pkt-miss */
            control.flood_egrAcl_enable = enable;
            break;
        case 'm': /* mcast-pkt-miss */
            control.mcast_egrAcl_enable = enable;
            break;
        case 'o': /* other-drop */
            control.other_drop_enable = enable;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieEgrAclLookupCtrl_set(unit, &control), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_egress_acl_lookup_flood_pkt_miss_mcast_pkt_miss_other_drop_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_PHASE_PHASE_INDEX_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * pie set phase <UINT:phase_index> ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_pie_set_phase_phase_index_port_all_state_disable_enable(cparser_context_t *context,
    uint32_t *phase_index_ptr,
    char **port_ptr)
{
    uint32          unit = 0;
    rtk_port_t      port;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;
    rtk_pie_phase_t phase = PIE_PHASE_END;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (*phase_index_ptr)
    {
        case 0:/* flow-classification */
            phase = PIE_FLOW_CLASSIFICATION;
            break;
        case 1: /* ingress-acl */
            phase = PIE_IGR_ACL;
            break;
        case 2: /* egress-acl */
            phase = PIE_EGR_ACL;
            break;
        case 3:
            phase = PIE_EGR_VID_TRANSLATION;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(6,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_pie_piePortLookupPhaseEnable_set( unit, port, phase, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_phase_phase_index_port_all_state_disable_enable */
#endif

#ifdef CMD_PIE_SET_PHASE_PHASE_INDEX_PORT_ALL_LOOKUP_MISS_DROP_PERMIT
/*
 * pie set phase <UINT:phase_index> ( <PORT_LIST:port> | all ) lookup-miss ( drop | permit )
 */
cparser_result_t cparser_cmd_pie_set_phase_phase_index_port_all_lookup_miss_drop_permit(cparser_context_t *context,
    uint32_t *phase_index_ptr,
    char **port_ptr)
{
    uint32          unit = 0;
    rtk_port_t      port;
    int32           ret = RT_ERR_FAILED;
    rtk_pie_lookupMissAction_t   action;
    rtk_pie_phase_t phase = PIE_PHASE_END;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (*phase_index_ptr)
    {
        case 0:/* flow-classification */
            phase = PIE_FLOW_CLASSIFICATION;
            break;
        case 1: /* ingress-acl */
            phase = PIE_IGR_ACL;
            break;
        case 2: /* egress-acl */
            phase = PIE_EGR_ACL;
            break;
        case 3:
            phase = PIE_EGR_VID_TRANSLATION;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(6,0))
    {
        case 'd': /* drop */
            action = LOOKUP_MISS_DROP;
            break;
        case 'p': /* permit */
            action = LOOKUP_MISS_PERMIT;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_pie_piePortLookupPhaseMiss_set(unit, port, phase, action)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_set_phase_phase_index_port_all_lookup_miss_drop_permit */
#endif

#ifdef CMD_PIE_GET_ENTRY_PHASE_ENTRY
/*
 * pie get entry <UINT:phase> <UINT:entry>
 */
cparser_result_t cparser_cmd_pie_get_entry_phase_entry(cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *entry_ptr)
{
    uint32      unit = 0;
    uint32      entry_size = 0;
    uint32      index = 0;
    int32       ret = RT_ERR_FAILED;
    uint8       *pEntry_buffer = NULL;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntrySize_get(unit, &entry_size), ret);

    if ((entry_size % 8) == 0)
    {
        entry_size = (entry_size / 8);
    }
    else
    {
        entry_size = (entry_size / 8) + 1;
    }

    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("malloc() fail!\n");
        return CPARSER_NOT_OK;
    }
    memset(pEntry_buffer, 0, entry_size);

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleEntry_read( unit, *entry_ptr, pEntry_buffer), ret);

    for (index = 0; index < entry_size; index++)
    {
        if((index % 8) == 0)
        {
            diag_util_mprintf("\n");
        }
        diag_util_printf("%02x ", *(pEntry_buffer + index));
    }
    diag_util_mprintf("\n");

    free(pEntry_buffer);
    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_entry_phase_entry */
#endif

#ifdef CMD_PIE_GET_ACTION_INDEX
/*
 * pie get action <UINT:index>
 */
cparser_result_t cparser_cmd_pie_get_action_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_actionTable_t   action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&action, 0, sizeof(rtk_pie_actionTable_t));

    DIAG_UTIL_ERR_CHK(rtk_pie_pieRuleAction_get(unit, *index_ptr, &action), ret);

    diag_util_mprintf("Action index : %d\n", *index_ptr);
    diag_util_mprintf("\thit indication : %d\n", action.hit_indication);
    diag_util_printf("\tstatistics : ");
    switch (action.statistics)
    {
        case COUNT_NONE:
            diag_util_printf("none");
            break;
        case COUNT_PACKET:
            diag_util_printf("pkt");
            break;
        case COUNT_BYTE32:
            diag_util_printf("byte32");
            break;
        case COUNT_BYTE64:
            diag_util_printf("byte64");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("\tcp2cpu : %d\n", action.cp2cpu);
    diag_util_printf("\tdrop : ");
    switch (action.drop)
    {
        case DROP_PERMIT:
            diag_util_printf("permit");
            break;
        case DROP_DROP:
            diag_util_printf("drop");
            break;
        case DROP_WITHDRAW_DROP:
            diag_util_printf("withdraw-drop");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\n");
    diag_util_mprintf("\touter tag operation : %d\n", action.outer_tag_op);
    diag_util_mprintf("\tredirect : %d\n", action.redirect);
    diag_util_mprintf("\tinner tag operation : %d\n", action.inner_tag_op);
    diag_util_mprintf("\tpriority : %d\n", action.priority);
    diag_util_mprintf("\tdscp remark/spid : %d\n", action.dscp_remark_spid);
    diag_util_mprintf("\tpolice/outer priority remark : %d\n", action.police_outer_pri_remark);
    diag_util_mprintf("\tmirror : %d\n", action.mirror);
    diag_util_mprintf("\n");

    diag_util_mprintf("\tOuter tag operation configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.outer_tag_op_field.withdraw);
    diag_util_mprintf("\touter_vid_ctrl : %d\n", action.outer_tag_op_field.outer_vid_ctrl);
    diag_util_mprintf("\touter_vid_info : %d\n", action.outer_tag_op_field.outer_vid_info);
    diag_util_mprintf("\touter_tag_op : %d\n", action.outer_tag_op_field.outer_tag_op);
    diag_util_mprintf("\n");

    diag_util_mprintf("\tRedirect configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.redirect_field.withdraw);
    diag_util_mprintf("\topcode : %d\n", action.redirect_field.opcode);

    switch (action.redirect_field.opcode)
    {
        case REDIRECT_UNI_REDIRECT:
            diag_util_mprintf("\tuniRedirect.cpu_tag : %d\n", action.redirect_field.un.uniRedirect.cpu_tag);
            diag_util_mprintf("\tuniRedirect.dpn : %d\n", action.redirect_field.un.uniRedirect.dpn);
            break;
        case REDIRECT_MULTI_REDIRECT:
            diag_util_mprintf("\tmultiRedirect.fwd_idx : %d\n", action.redirect_field.un.multiRedirect.fwd_idx);
            break;
        case REDIRECT_UNI_ROUTE:
        case REDIRECT_MULTI_ROUTE:
            diag_util_mprintf("\troute.ttl_dec : %d\n", action.redirect_field.un.route.ttl_dec);
            diag_util_mprintf("\troute.lookup_idx : %d\n", action.redirect_field.un.route.lookup_idx);
            break;
        default:
            break;
    }
    diag_util_mprintf("\n");

    diag_util_mprintf("\tInner tag operation configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.inner_tag_op_field.withdraw);
    diag_util_mprintf("\tinner_vid_ctrl : %d\n", action.inner_tag_op_field.inner_vid_ctrl);
    diag_util_mprintf("\tinner_vid_info : %d\n", action.inner_tag_op_field.inner_vid_info);
    diag_util_mprintf("\tinner_tag_op : %d\n", action.inner_tag_op_field.inner_tag_op);
    diag_util_mprintf("\n");

    diag_util_mprintf("\tPriority configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.priority_field.withdraw);
    diag_util_mprintf("\tsel_pri_dp : %d\n", action.priority_field.sel_pri_dp);
    diag_util_mprintf("\tassign_pri : %d\n", action.priority_field.assign_pri);
    diag_util_mprintf("\tacl_pri : %d\n", action.priority_field.acl_pri);
    diag_util_mprintf("\tassign_dp : %d\n", action.priority_field.assign_dp);
    diag_util_mprintf("\tacl_dp : %d\n", action.priority_field.acl_dp);
    diag_util_mprintf("\n");

    //diag_util_mprintf("\tdscp_remark_spid configuration\n");
    diag_util_mprintf("\tDSCP remark configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.dscp_remark_spid_field.withdraw);
    diag_util_mprintf("\topcode : %d\n", action.dscp_remark_spid_field.un.dscp_remark.un.dscp.opcode);
    switch (action.dscp_remark_spid_field.un.dscp_remark.un.dscp.opcode)
    {
        case 0:
            diag_util_mprintf("\tdscp : %d\n", action.dscp_remark_spid_field.un.dscp_remark.un.dscp.dscp);
            break;
        case 1:
            diag_util_mprintf("\tip_precedence : %d\n", action.dscp_remark_spid_field.un.dscp_remark.un.ipPrecedence.ip_precedence);
            break;
        case 2:
            diag_util_mprintf("\tdtr_bits : %d\n", action.dscp_remark_spid_field.un.dscp_remark.un.dtrBits.dtr_bits);
            break;
        default:
            break;
    }
    diag_util_mprintf("\n");

    diag_util_mprintf("\tSPID configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.dscp_remark_spid_field.withdraw);
    diag_util_mprintf("\tspid_idx : %d\n", action.dscp_remark_spid_field.un.spid.spid_idx);
    diag_util_mprintf("\n");

    //diag_util_mprintf("\tpolice_outer_pri_remark configuration\n");
    diag_util_mprintf("\tPolice configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.police_outer_pri_remark_field.withdraw);
    diag_util_mprintf("\tpolicer_idx : %d\n", action.police_outer_pri_remark_field.un.policer_idx);
    diag_util_mprintf("\n");

    diag_util_mprintf("\tOuter priority remark configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.police_outer_pri_remark_field.withdraw);
    diag_util_mprintf("\tremark_pri : %d\n", action.police_outer_pri_remark_field.un.outer_pri_remark.remark_pri);
    diag_util_mprintf("\tpri : %d\n", action.police_outer_pri_remark_field.un.outer_pri_remark.pri);
    diag_util_mprintf("\tremark_dei : %d\n", action.police_outer_pri_remark_field.un.outer_pri_remark.remark_dei);
    diag_util_mprintf("\tdei : %d\n", action.police_outer_pri_remark_field.un.outer_pri_remark.dei);
    diag_util_mprintf("\n");

    diag_util_mprintf("\tMirror configuration\n");
    diag_util_mprintf("\twithdraw : %d\n", action.mirror_field.withdraw);
    diag_util_mprintf("\tmirror_idx : %d\n", action.mirror_field.mirror_idx);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_action_index */
#endif

#ifdef CMD_PIE_GET_POLICE_INDEX
/*
 * pie get police <UINT:index>
 */
cparser_result_t cparser_cmd_pie_get_police_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_pie_policerEntry_t  policer;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&policer, 0, sizeof(rtk_pie_policerEntry_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieRulePolicer_get(unit, *index_ptr, &policer), ret);

    diag_util_mprintf("Policer index : %d\n", *index_ptr);
    diag_util_printf("\ttype : ");
    switch (policer.type)
    {
        case POLICER_TYPE_INVALID: /* invalid */
            diag_util_mprintf("invalid\n");
            break;
        case POLICER_TYPE_DLB: /* dlb */
            diag_util_mprintf("dlb\n");
            break;
        case POLICER_TYPE_SRTCM: /* srtcm */
            diag_util_mprintf("srtcm\n");
            break;
        case POLICER_TYPE_TRTCM: /* trtcm */
            diag_util_mprintf("trtcm\n");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\tcolor_aware : %d\n", policer.color_aware);
    diag_util_mprintf("\tyellow_dp : %d\n", policer.yellow_dp);
    diag_util_mprintf("\tred_dp : %d\n", policer.red_dp);
    diag_util_mprintf("\tpir : 0x%x\n", policer.pir);
    diag_util_mprintf("\tcir : 0x%x\n", policer.cir);
    diag_util_mprintf("\ttp : 0x%x\n", policer.tp);
    diag_util_mprintf("\ttc : 0x%x\n", policer.tc);
    diag_util_mprintf("\tpbs : 0x%x\n", policer.pbs);
    diag_util_mprintf("\tcbs : 0x%x\n", policer.cbs);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_police_index */
#endif

#ifdef CMD_PIE_GET_COUNTER_INDEX
/*
 * pie get counter <UINT:index>
 */
cparser_result_t cparser_cmd_pie_get_counter_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint64      byte_cnt = 0;
    uint32      unit = 0;
    uint32      pkt_cnt = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieStat_get(unit, *index_ptr, &pkt_cnt, &byte_cnt), ret);

    diag_util_mprintf("Counter index : %d\n", *index_ptr);
    diag_util_mprintf("\tbyte_cnt : %llu\n", byte_cnt);
    diag_util_mprintf("\tpkt_cnt : %lu\n", pkt_cnt);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_counter_index */
#endif

#ifdef CMD_PIE_GET_COUNTER_INDICATION_MODE_LBLOCK_INDEX
/*
 * pie get counter-indication-mode <UINT:lblock_index>
 */
cparser_result_t cparser_cmd_pie_get_counter_indication_mode_lblock_index(cparser_context_t *context,
    uint32_t *lblock_index_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_pie_counterIndicationMode_t mode = PIE_INDICATION_MODE_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieCounterIndicationMode_get(unit, *lblock_index_ptr, &mode), ret);

    diag_util_mprintf("Logical block : %d\n", *lblock_index_ptr);
    diag_util_printf("\tindication mode : ");

    switch (mode)
    {
        case ACTION_EXECUTION: /* action-execution */
            diag_util_mprintf("action-execution\n");
            break;
        case RULE_MATCH: /* rule-match */
            diag_util_mprintf("rule-match\n");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_counter_indication_mode_lblock_index */
#endif

#ifdef CMD_PIE_GET_HIT_INDICATION_LBLOCK_INDEX
/*
 * pie get hit-indication <UINT:lblock_index>
 */
cparser_result_t cparser_cmd_pie_get_hit_indication_lblock_index(cparser_context_t *context,
    uint32_t *lblock_index_ptr)
{
    uint32                          unit = 0;
    uint32                          index = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_pie_hitIndicationEntry_t    status;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieHitIndication_get(unit, *lblock_index_ptr, &status), ret);

    diag_util_mprintf("Hit indication status\n");
    diag_util_mprintf("logical block : %d\n", *lblock_index_ptr);

    for (index=0; index<RTK_TOTAL_NUM_OF_BYTE_FOR_1BIT_RULE_OF_LOGICAL_BLOCK; index++)
    {
        if((index % 8) == 0)
        {
            diag_util_mprintf("\n");
        }
        diag_util_printf("%02x ", status.hit_status[index]);
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_hit_indication_lblock_index */
#endif

#ifdef CMD_PIE_GET_SELECTOR_PBLOCK_PHASE
/*
 * pie get selector <UINT:pblock> <UINT:phase>
 */
cparser_result_t cparser_cmd_pie_get_selector_pblock_phase(cparser_context_t *context,
    uint32_t *pblock_ptr,
    uint32_t *phase_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_pie_id_t    template_idx = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieTemplateSelector_get(unit, *pblock_ptr, *phase_ptr, &template_idx), ret);

    diag_util_mprintf("Selector block %d, Phase %d\n", *pblock_ptr, *phase_ptr);
    diag_util_mprintf("\ttemplate : %d\n", template_idx);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_selector_pblock_phase */
#endif

#ifdef CMD_PIE_GET_TEMPLATE_INDEX
/*
 * pie get template <UINT:index>
 */
cparser_result_t cparser_cmd_pie_get_template_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32              unit = 0;
    uint32              index = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_pie_template_t  template;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&template, 0 , sizeof(rtk_pie_template_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieUserTemplate_get(unit, *index_ptr, &template), ret);

    diag_util_mprintf("Template : %ld\n", *index_ptr);
    for (index = 0; index < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; index++)
    {
        diag_util_printf("\tfield %d : ", index);
        switch (template.field[index])
        {
            case FMT:
                diag_util_printf("fmt");
                break;
            case DMAC0:
                diag_util_printf("dmac0");
                break;
            case DMAC1_RRCPINFO:
                diag_util_printf("dmac1-rrcpinfo");
                break;
            case DMAC2:
                diag_util_printf("dmac2");
                break;
            case SMAC0:
                diag_util_printf("smac0");
                break;
            case SMAC1:
                diag_util_printf("smac1");
                break;
            case SMAC2:
                diag_util_printf("smac2");
                break;
            case ETHERTYPE:
                diag_util_printf("ethertype");
                break;
            case OTAG:
                diag_util_printf("otag");
                break;
            case INTAG:
                diag_util_printf("itag");
                break;
            case FWD_OTAG_DPRI:
                diag_util_printf("fwd-otag-dpri");
                break;
            case FWD_ITAG:
                diag_util_printf("fwd-itag");
                break;
            case FWD_VID:
                diag_util_printf("fwd-vid");
                break;
            case IP_TOS_PROTO:
                diag_util_printf("tos-proto");
                break;
            case IP4_TTL_FLAG:
                diag_util_printf("ttl-flag");
                break;
            case L4_SPORT:
                diag_util_printf("l4-sport");
                break;
            case L4_DPORT:
                diag_util_printf("l4-dport");
                break;
            case TCP_FLAG:
                diag_util_printf("tcp-flag");
                break;
            case ICMP_CODE_TYPE:
                diag_util_printf("icmp-code-type");
                break;
            case IGMP_TYPE:
                diag_util_printf("igmp-type");
                break;
            case SIP0:
                diag_util_printf("sip0");
                break;
            case SIP1:
                diag_util_printf("sip1");
                break;
            case SIP2:
                diag_util_printf("sip2");
                break;
            case SIP3:
                diag_util_printf("sip3");
                break;
            case SIP4:
                diag_util_printf("sip4");
                break;
            case SIP5:
                diag_util_printf("sip5");
                break;
            case SIP6:
                diag_util_printf("sip6");
                break;
            case SIP7:
                diag_util_printf("sip7");
                break;
            case DIP0:
                diag_util_printf("dip0");
                break;
            case DIP1:
                diag_util_printf("dip1");
                break;
            case DIP2:
                diag_util_printf("dip2");
                break;
            case DIP3:
                diag_util_printf("dip3");
                break;
            case DIP4:
                diag_util_printf("dip4");
                break;
            case DIP5:
                diag_util_printf("dip5");
                break;
            case DIP6:
                diag_util_printf("dip6");
                break;
            case DIP7:
                diag_util_printf("dip7");
                break;
            case IP6_FLWH:
                diag_util_printf("ip6-flwh");
                break;
            case SPM1:
                diag_util_printf("spm1");
                break;
            case SPM2:
                diag_util_printf("spm2");
                break;
            case SPSM:
                diag_util_printf("spsm");
                break;
            case DPM1:
                diag_util_printf("dpm1");
                break;
            case DPM2:
                diag_util_printf("dpm2");
                break;
            case VID_RANG:
                diag_util_printf("vid-range");
                break;
            case GUEST_VLAN:
                diag_util_printf("guest-vlan");
                break;
            case PORT_RANG:
                diag_util_printf("port-range");
                break;
            case IP_RANG1:
                diag_util_printf("ip-range1");
                break;
            case IP_RANG2:
                diag_util_printf("ip-range2");
                break;
            case IP_RANG3:
                diag_util_printf("ip-range3");
                break;
            case IP_RANG4:
                diag_util_printf("ip-range4");
                break;
            case PATTERN_MATCH0:
                diag_util_printf("pattern-match0");
                break;
            case PATTERN_MATCH1:
                diag_util_printf("pattern-match1");
                break;
            case FIELD_SELECTOR1_0:
                diag_util_printf("field-selector1_0");
                break;
            case FIELD_SELECTOR1_1:
                diag_util_printf("field-selector1_1");
                break;
            case FIELD_SELECTOR1_2:
                diag_util_printf("field-selector1_2");
                break;
            case FIELD_SELECTOR1_3:
                diag_util_printf("field-selector1_3");
                break;
            case FIELD_SELECTOR2_0:
                diag_util_printf("field-selector2_0");
                break;
            case FIELD_SELECTOR2_1:
                diag_util_printf("field-selector2_1");
                break;
            case FIELD_SELECTOR2_2:
                diag_util_printf("field-selector2_2");
                break;
            case FIELD_SELECTOR2_3:
                diag_util_printf("field-selector2_3");
                break;
            case DPN:
                diag_util_printf("dpn");
                break;
            case PAYLOAD0:
                diag_util_printf("payload0");
                break;
            case PAYLOAD1:
                diag_util_printf("payload1");
                break;
            default:
                break;
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_template_index */
#endif

#ifdef CMD_PIE_GET_L34_CHECKSUM_ERR
/*
 * pie get l34-checksum-err
 */
cparser_result_t cparser_cmd_pie_get_l34_checksum_err(cparser_context_t *context)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_pie_l34ChecksumErrOper_t    operation = PIE_CHECKSUM_ERR_OPER_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieL34ChecksumErr_get(unit, &operation), ret);

    diag_util_printf("L34-Checksum-Error : ");
    switch (operation)
    {
        case CHECKSUM_ERR_DOWNGRADE: /* downgrade */
            diag_util_mprintf("downgrade\n");
            break;
        case CHECKSUM_ERR_PARSE_ANYWAY: /* parse-anyway */
            diag_util_mprintf("parse-anyway\n");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_l34_checksum_err */
#endif

#ifdef CMD_PIE_GET_PAYLOAD_PBLOCK
/*
 * pie get payload <UINT:pblock>
 */
cparser_result_t cparser_cmd_pie_get_payload_pblock(cparser_context_t *context,
    uint32_t *pblock_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  offset;
    rtk_pie_id_t            offset_idx;
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(diag_om_get_deviceInfo(unit, &devInfo), ret);

    diag_util_mprintf("Physical block : %d\n", *pblock_ptr);
    for (offset_idx = 0; offset_idx < devInfo.capacityInfo.max_num_of_pie_payload; offset_idx++)
    {
        if ((ret = rtk_pie_pieUserTemplatePayloadOffset_get(unit, *pblock_ptr, offset_idx, &offset)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("\tpayload : %d, offset : 0x%x\n", offset_idx, offset);
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_payload_pblock */
#endif

#ifdef CMD_PIE_GET_RESULT_REVERSE_ENTRY_INDEX
/*
 * pie get result-reverse <UINT:entry_index>
 */
cparser_result_t cparser_cmd_pie_get_result_reverse_entry_index(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32                       unit = 0;
    int32                        ret = RT_ERR_FAILED;
    rtk_pie_resultReverseOper_t  operation = PIE_RESULT_REVERSE_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieResultReverse_get(unit, *entry_index_ptr, &operation), ret);

    diag_util_mprintf("PIE entry : %d\n", *entry_index_ptr);
    switch (operation)
    {
        case RESULT_KEEP: /* keep result */
            diag_util_mprintf("\tresult : keep\n");
            break;
        case RESULT_REVERSE: /* reverse result */
            diag_util_mprintf("\tresult : reverse\n");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_result_reverse_entry_index */
#endif

#ifdef CMD_PIE_GET_RESULT_AGGREGATOR_PBLOCK_RANGE_INDEX_ENTRY_INDEX
/*
 * pie get result-aggregator <UINT:pblock_range_index> <UINT:entry_index>
 */
cparser_result_t cparser_cmd_pie_get_result_aggregator_pblock_range_index_entry_index(cparser_context_t *context,
    uint32_t *pblock_range_index_ptr,
    uint32_t *entry_index_ptr)
{
    uint32                          unit = 0;
    int32                           ret = RT_ERR_FAILED;
    rtk_pie_resultAggregatorType_t  type = PIE_RESULT_AGGREGATOR_TYPE_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieResultAggregator_get(unit, *pblock_range_index_ptr, *entry_index_ptr, &type), ret);

    diag_util_mprintf("Result aggregator:\n");
    diag_util_mprintf("physical block range : %d, entry index : %d\n", *pblock_range_index_ptr, *entry_index_ptr);
    diag_util_printf("\ttype : ");
    switch (type)
    {
        case NONE:
            diag_util_mprintf("none\n");
            break;
        case A01M0:
            diag_util_mprintf("a01m0\n");
            break;
        case A01M1:
            diag_util_mprintf("a01m1\n");
            break;
        case A23M2:
            diag_util_mprintf("a23m2\n");
            break;
        case A23M3:
            diag_util_mprintf("a23m3\n");
            break;
        case A01_23M0_2:
            diag_util_mprintf("a01_23m0_2\n");
            break;
        case A01_23M1_2:
            diag_util_mprintf("a01_23m1_2\n");
            break;
        case A01_23M0_3:
            diag_util_mprintf("a01_23m0_3\n");
            break;
        case A01_23M1_3:
            diag_util_mprintf("a01_23m1_3\n");
            break;
        case A0123M0:
            diag_util_mprintf("a0123m0\n");
            break;
        case A0123M1:
            diag_util_mprintf("a0123m1\n");
            break;
        case A0123M2:
            diag_util_mprintf("a0123m2\n");
            break;
        case A0123M3:
            diag_util_mprintf("a0123m3\n");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_result_aggregator_pblock_range_index_entry_index */
#endif

#ifdef CMD_PIE_GET_BLOCK_PRIORITY_BLOCK_INDEX
/*
 * pie get block-priority <UINT:block_index>
 */
cparser_result_t cparser_cmd_pie_get_block_priority_block_index(cparser_context_t *context,
    uint32_t *block_index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  priority = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_pie_pieBlockPriority_get(unit, *block_index_ptr, &priority), ret);

    diag_util_mprintf("Logical block : %d\n", *block_index_ptr);
    diag_util_mprintf("\tpriority : %d\n", priority);

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_block_priority_block_index */
#endif

#ifdef CMD_PIE_GET_GROUP_LBLOCK_RANGE_INDEX
/*
 * pie get group <UINT:lblock_range_index>
 */
cparser_result_t cparser_cmd_pie_get_group_lblock_range_index(cparser_context_t *context,
    uint32_t *lblock_range_index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_pie_groupCtrlRange_t    lblockRange_idx = PIE_GROUP_RANGE_END;
    rtk_pie_groupCtrl_t         operation = PIE_GROUP_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (*lblock_range_index_ptr)
    {
    	case 0:
    	    lblockRange_idx = GROUP_LBLOCK_0_3;
    	    break;
    	case 1:
    	    lblockRange_idx = GROUP_LBLOCK_4_7;
    	    break;
    	case 2:
    	    lblockRange_idx = GROUP_LBLOCK_8_11;
    	    break;
    	case 3:
    	    lblockRange_idx = GROUP_LBLOCK_12_15;
    	    break;
	default:
	    diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_pieGroupCtrl_get(unit, lblockRange_idx, &operation), ret);

    diag_util_mprintf("Logical block range : %d\n", lblockRange_idx);
    diag_util_printf("\tgroup : ");
    switch (operation)
    {
    	case GROUP_NONE:
    	    diag_util_printf("none");
    	    break;
    	case GROUP_01:
    	    diag_util_printf("group_01");
    	    break;
    	case GROUP_012:
    	    diag_util_printf("group_012");
    	    break;
    	case GROUP_0123:
    	    diag_util_printf("group_0123");
    	    break;
	default:
	    diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_group_lblock_range_index */
#endif

#ifdef CMD_PIE_GET_EGRESS_ACL_LOOKUP
/*
 * pie get egress-acl-lookup
 */
cparser_result_t cparser_cmd_pie_get_egress_acl_lookup(cparser_context_t *context)
{
    uint32                       unit = 0;
    int32                        ret = RT_ERR_FAILED;
    rtk_pie_egrAclLookupCtrl_t   control;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&control, 0, sizeof(rtk_pie_egrAclLookupCtrl_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_pieEgrAclLookupCtrl_get(unit, &control), ret);

    diag_util_mprintf("Egress acl lookup:\n");
    diag_util_printf("\tflood : ");
    switch (control.flood_egrAcl_enable)
    {
        case DISABLED:
            diag_util_printf("disable");
            break;
        case ENABLED:
            diag_util_printf("enable");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_printf("\t ,action : ");
    switch (control.flood_lookupMiss_action)
    {
        case LOOKUP_MISS_DROP:
            diag_util_mprintf("drop");
            break;
        case LOOKUP_MISS_PERMIT:
            diag_util_mprintf("permit");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\n");

    diag_util_printf("\tmcast : ");
    switch (control.mcast_egrAcl_enable)
    {
        case DISABLED:
            diag_util_printf("disable");
            break;
        case ENABLED:
            diag_util_printf("enable");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_printf("\t ,action : ");
    switch (control.mcast_lookupMiss_action)
    {
        case LOOKUP_MISS_DROP:
            diag_util_mprintf("drop");
            break;
        case LOOKUP_MISS_PERMIT:
            diag_util_mprintf("permit");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\n");

    diag_util_printf("\tother_drop : ");
    switch (control.other_drop_enable)
    {
        case DISABLED:
            diag_util_printf("disable");
            break;
        case ENABLED:
            diag_util_printf("enable");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_egress_acl_lookup */
#endif

#ifdef CMD_PIE_GET_PHASE_PHASE_INDEX_PORT_ALL
/*
 * pie get phase <UINT:phase_index> ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_pie_get_phase_phase_index_port_all(cparser_context_t *context,
    uint32_t *phase_index_ptr,
    char **port_ptr)
{
    uint32          unit = 0;
    rtk_port_t      port;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;
    rtk_pie_lookupMissAction_t   action;
    rtk_pie_phase_t phase = PIE_PHASE_END;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (*phase_index_ptr)
    {
        case 0:/* flow-classification */
            phase = PIE_FLOW_CLASSIFICATION;
            break;
        case 1: /* ingress-acl */
            phase = PIE_IGR_ACL;
            break;
        case 2: /* egress-acl */
            phase = PIE_EGR_ACL;
            break;
        case 3:
            phase = PIE_EGR_VID_TRANSLATION;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_pie_piePortLookupPhaseEnable_get(unit, port, phase, &enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        if ((ret = rtk_pie_piePortLookupPhaseMiss_get(unit, port, phase, &action)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("Port : %d\n", port);
        diag_util_printf("\tphase : ");
        switch (phase)
        {
            case PIE_FLOW_CLASSIFICATION: /* flow-classification */
                diag_util_printf("0");
                break;
            case PIE_IGR_ACL: /* ingress-acl */
                diag_util_printf("1");
                break;
            case PIE_EGR_ACL: /* egress-acl */
                diag_util_printf("2");
                break;
            case PIE_EGR_VID_TRANSLATION: /* egress-vid-translation */
                diag_util_printf("3");
                break;
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
        }
        diag_util_mprintf("\n");

        diag_util_printf("\tstate : ");
        switch (enable)
        {
            case DISABLED: /* disable */
                diag_util_printf("disable");
                break;
            case ENABLED: /* enable */
                diag_util_printf("enable");
                break;
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
        }
        diag_util_mprintf("\n");

        diag_util_printf("\tmiss-action : ");
        switch (action)
        {
            case LOOKUP_MISS_DROP: /* drop */
                diag_util_printf("drop");
                break;
            case LOOKUP_MISS_PERMIT: /* permit */
                diag_util_printf("permit");
                break;
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_get_phase_phase_index_port_all */
#endif

#ifdef CMD_PIE_DEL_ACTION_ENTRY_ACTION_AND_ENTRY_START
/*
 * pie del ( action | entry | action_and_entry ) <UINT:start>
 */
cparser_result_t cparser_cmd_pie_del_action_entry_action_and_entry_start(cparser_context_t *context,
    uint32_t *start_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_pie_clearBlockContent_t content;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&content, 0, sizeof(rtk_pie_clearBlockContent_t));
    content.start_idx = *start_ptr;
    content.end_idx = *start_ptr;
    switch (TOKEN_CHAR(2,0))
    {
        case 'a': /* action & action_and_entry */
            if (TOKEN_CHAR(2,6) == '_') /* action_and_entry */
            {
                if ((ret = rtk_pie_pieRuleEntryAction_del(unit, &content)) != RT_ERR_OK)
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
            }
            else /* action */
            {
                if ((ret = rtk_pie_pieRuleAction_del(unit, &content)) != RT_ERR_OK)
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
            }
            break;
        case 'e': /* entry */
            if ((ret = rtk_pie_pieRuleEntry_del(unit, &content)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            } 
            break;       
        default: 
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_del_action_entry_action_and_entry_start */
#endif

#ifdef CMD_PIE_DEL_ACTION_ENTRY_ACTION_AND_ENTRY_START_END
/*
 * pie del ( action | entry | action_and_entry ) <UINT:start> <UINT:end>
 */
cparser_result_t cparser_cmd_pie_del_action_entry_action_and_entry_start_end(cparser_context_t *context,
    uint32_t *start_ptr,
    uint32_t *end_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_pie_clearBlockContent_t content;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&content, 0, sizeof(rtk_pie_clearBlockContent_t));
    content.start_idx = *start_ptr;
    content.end_idx = *end_ptr;
    switch (TOKEN_CHAR(2,0))
    {
        case 'a': /* action & action_and_entry */
            if (TOKEN_CHAR(2,6) == '_') /* action_and_entry */
            {
                if ((ret = rtk_pie_pieRuleEntryAction_del(unit, &content)) != RT_ERR_OK)
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
            }
            else /* action */
            {
                if ((ret = rtk_pie_pieRuleAction_del(unit, &content)) != RT_ERR_OK)
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                } 
            }
            break;
        case 'e': /* entry */
            if ((ret = rtk_pie_pieRuleEntry_del(unit, &content)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            } 
            break;       
        default: 
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pie_del_action_entry_action_and_entry_start_end */
#endif

#ifdef CMD_FIELD_SELECTOR_SET_PORT_ALL_INDEX_STATE_DISABLE_ENABLE
/*
 * field-selector set ( <PORT_LIST:port> | all ) <UINT:index> state ( disable | enable )
 */
cparser_result_t cparser_cmd_field_selector_set_port_all_index_state_disable_enable(cparser_context_t *context,
    char **port_ptr,
    uint32_t *index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    rtk_enable_t enable;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(5,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_pie_fieldSelectorEnable_set(unit, port, *index_ptr, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_field_selector_set_port_all_index_state_disable_enable */
#endif

#ifdef CMD_FIELD_SELECTOR_SET_PORT_ALL_INDEX_CONTENT_8021Q_ARP_IPV4_IPV6_L4_HEADER_LLC_RAW_TCP_UDP_PAYLOAD_OFFSET0_OFFSET1_OFFSET2_OFFSET3
/*
 * field-selector set selector ( <PORT_LIST:port> | all ) <UINT:index> content ( 8021q | arp | ipv4 | ipv6 | l4-header | llc | raw | tcp-udp-payload ) <UINT:offset0> <UINT:offset1> <UINT:offset2> <UINT:offset3>
 */
cparser_result_t cparser_cmd_field_selector_set_port_all_index_content_8021q_arp_ipv4_ipv6_l4_header_llc_raw_tcp_udp_payload_offset0_offset1_offset2_offset3(cparser_context_t *context,
    char **port_ptr,
    uint32_t *index_ptr,
    uint32_t *offset0_ptr,
    uint32_t *offset1_ptr,
    uint32_t *offset2_ptr,
    uint32_t *offset3_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_pie_fieldSelector_data_t fs;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&fs, 0, sizeof(rtk_pie_fieldSelector_data_t));
    switch (TOKEN_CHAR(5,0))
    {
        case '8': /* 8021q */
            fs.start = DOT1Q;
            break;
        case 'a': /* arp */
            fs.start = ARP;
            break;
        case 'i':
            switch (TOKEN_CHAR(5,3))
            {
                case '4': /* ipv4 */
                    fs.start = IPV4;
                    break;
                case '6': /* ipv6 */
                    fs.start = IPV6;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'l':
            switch (TOKEN_CHAR(5,1))
            {
                case '4': /* l4-header */
                    fs.start = TCP_UDP_ICMP_IGMP_ICMPV6;
                    break;
                case 'l': /* llc */
                    fs.start = LLC;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case 'r': /* raw */
            fs.start = RAW;
            break;
        case 't': /* tcp-udp-payload */
            fs.start = TCP_UDP_PAYLOAD;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }
    fs.offset0 = *offset0_ptr;
    fs.offset1 = *offset1_ptr;
    fs.offset2 = *offset2_ptr;
    fs.offset3 = *offset3_ptr;

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_pie_fieldSelectorContent_set(unit, port, *index_ptr, &fs)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_field_selector_set_port_all_index_content_8021q_arp_ipv4_ipv6_l4_header_llc_raw_tcp_udp_payload_offset0_offset1_offset2_offset3 */
#endif

#ifdef CMD_FIELD_SELECTOR_GET_PORT_ALL
/*
 * field-selector get selector ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_field_selector_get_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    rtk_port_t  port;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    rtk_pie_id_t fs_idx;
    diag_portlist_t  portlist;
    rtk_pie_fieldSelector_data_t fs;
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(diag_om_get_deviceInfo(unit, &devInfo), ret);

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port : %d\n", port);

        for (fs_idx = 0; fs_idx < devInfo.capacityInfo.max_num_of_field_selector; fs_idx++)
        {
            if ((ret = rtk_pie_fieldSelectorEnable_get(unit, port, fs_idx, &enable)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            if ((ret = rtk_pie_fieldSelectorContent_get(unit, port, fs_idx, &fs)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }

            diag_util_mprintf("Selector : %d\n", fs_idx);
            diag_util_printf("\tstate : ");
            switch (enable)
            {
                case DISABLED: /* disable */
                    diag_util_printf("disable");
                    break;
                case ENABLED: /* enable */
                    diag_util_printf("enable");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");

            diag_util_printf("\tstart : ");
            switch (fs.start)
            {
                case RAW: /* raw */
                    diag_util_printf("raw");
                    break;
                case DOT1Q: /* 8021q */
                    diag_util_printf("8021q");
                    break;
                case LLC: /* llc */
                    diag_util_printf("llc");
                    break;
                case IPV4: /* ipv4 */
                    diag_util_printf("ipv4");
                    break;
                case ARP: /* arp */
                    diag_util_printf("arp");
                    break;
                case IPV6: /* ipv6 */
                    diag_util_printf("ipv6");
                    break;
                case TCP_UDP_ICMP_IGMP_ICMPV6: /* l4-header */
                    diag_util_printf("l4-header");
                    break;
                case TCP_UDP_PAYLOAD: /* tcp-udp-payload */
                    diag_util_printf("tcp-udp-payload");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");

            diag_util_mprintf("\toffset0 : 0x%x\n", fs.offset0);
            diag_util_mprintf("\toffset1 : 0x%x\n", fs.offset1);
            diag_util_mprintf("\toffset2 : 0x%x\n", fs.offset2);
            diag_util_mprintf("\toffset3 : 0x%x\n", fs.offset3);
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_field_selector_get_port_all */
#endif

#ifdef CMD_PATTERN_MATCH_SET_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * pattern-match set ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_pattern_match_set_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    rtk_enable_t enable;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch (TOKEN_CHAR(4,0))
    {
        case 'd': /* disable */
            enable = DISABLED;
            break;
        case 'e': /* enable */
            enable = ENABLED;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_pie_patternMatchEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pattern_match_set_port_all_state_disable_enable */
#endif

#ifdef CMD_PATTERN_MATCH_SET_PORT_ALL_INDEX_CHAR_DATA_START_LAST_CARE
/*
 * pattern-match set ( <PORT_LIST:port> | all ) <UINT:index> <STRING:char_data> { start } { last } { care }
 */
cparser_result_t cparser_cmd_pattern_match_set_port_all_index_char_data_start_last_care(cparser_context_t *context,
    char **port_ptr,
    uint32_t *index_ptr,
    char **char_data_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_pie_patternMatch_content_t   content;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);


    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&content, 0, sizeof(rtk_pie_patternMatch_content_t));
        if ((ret = rtk_pie_patternMatchContent_get(unit, port, &content)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        content.data[*index_ptr].character = **char_data_ptr;

        if (TOKEN_NUM == 6)
        {
            switch (TOKEN_CHAR(5,0))
            {
                case 's': /* start */
                    content.data[*index_ptr].start_bit = ENABLED;
                    content.data[*index_ptr].last_bit = DISABLED;
                    content.data[*index_ptr].care_bit = DISABLED;
                    break;
                case 'l': /* last */
                    content.data[*index_ptr].start_bit = DISABLED;
                    content.data[*index_ptr].last_bit = ENABLED;
                    content.data[*index_ptr].care_bit = DISABLED;
                    break;
                case 'c': /* care */
                    content.data[*index_ptr].start_bit = DISABLED;
                    content.data[*index_ptr].last_bit = DISABLED;
                    content.data[*index_ptr].care_bit = ENABLED;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
        }

        if (TOKEN_NUM == 7)
        {
            switch (TOKEN_CHAR(5,0))
            {
                case 's': /* start */
                    switch (TOKEN_CHAR(6,0))
                    {
                        case 'l': /* start & last */
                            content.data[*index_ptr].start_bit = ENABLED;
                            content.data[*index_ptr].last_bit = ENABLED;
                            content.data[*index_ptr].care_bit = DISABLED;
                            break;
                        case 'c': /* start & care */
                            content.data[*index_ptr].start_bit = ENABLED;
                            content.data[*index_ptr].last_bit = DISABLED;
                            content.data[*index_ptr].care_bit = ENABLED;
                            break;

                        default:
                            diag_util_printf("User config: Error!\n");
                            return CPARSER_NOT_OK;
                    }
                    break;
                case 'l': /* last & care*/
                    content.data[*index_ptr].start_bit = DISABLED;
                    content.data[*index_ptr].last_bit = ENABLED;
                    content.data[*index_ptr].care_bit = ENABLED;
                    break;

                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
        }

        if (TOKEN_NUM == 8)
        {
            content.data[*index_ptr].start_bit = DISABLED;
            content.data[*index_ptr].last_bit = ENABLED;
            content.data[*index_ptr].care_bit = ENABLED;
        }

        if ((ret = rtk_pie_patternMatchContent_set(unit, port, &content)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pattern_match_set_port_all_index_char_data_start_last_care */
#endif

#ifdef CMD_PATTERN_MATCH_GET_PORT_ALL
/*
 * pattern-match get ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_pattern_match_get_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    rtk_port_t  port;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    rtk_pie_id_t data_idx;
    diag_portlist_t  portlist;
    rtk_pie_patternMatch_content_t   content;
    rtk_switch_devInfo_t    devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    DIAG_UTIL_ERR_CHK(diag_om_get_deviceInfo(unit, &devInfo), ret);

    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2) != RT_ERR_OK)
    {
        diag_util_printf("port list error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port : %d\n", port);

        if ((ret = rtk_pie_patternMatchEnable_get(unit, port, &enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        if ((ret = rtk_pie_patternMatchContent_get(unit, port, &content)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_printf("\tstate : ");
        switch (enable)
        {
            case DISABLED: /* disable */
                diag_util_printf("disable");
                break;
            case ENABLED: /* enable */
                diag_util_printf("enable");
                break;
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;
        }
        diag_util_mprintf("\n");

        for (data_idx = 0; data_idx < devInfo.capacityInfo.max_num_of_pattern_match_data; data_idx++)
        {
            diag_util_mprintf("\tdata : %d\n", data_idx);
            diag_util_mprintf("\t\tcharacter : %c\n", content.data[data_idx].character);
            diag_util_printf("\t\tstart_bit : ");
            switch (content.data[data_idx].start_bit)
            {
                case DISABLED: /* disable */
                    diag_util_printf("disable");
                    break;
                case ENABLED: /* enable */
                    diag_util_printf("enable");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");

            diag_util_printf("\t\tlast_bit : ");
            switch (content.data[data_idx].last_bit)
            {
                case DISABLED: /* disable */
                    diag_util_printf("disable");
                    break;
                case ENABLED: /* enable */
                    diag_util_printf("enable");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");

            diag_util_printf("\t\tcare_bit : ");
            switch (content.data[data_idx].care_bit)
            {
                case DISABLED: /* disable */
                    diag_util_printf("disable");
                    break;
                case ENABLED: /* enable */
                    diag_util_printf("enable");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");
        }
    }

    return CPARSER_OK;
}/* end of cparser_cmd_pattern_match_get_port_all */
#endif

#ifdef CMD_RANGE_CHECK_SET_IP_CHECK_ENTRY_INDEX_IPV4_DIP_IPV4_SIP_IPV6_DIP_IPV6_SIP_IP1_IP2_IP3_IP4_LOWER_UPPER
/*
 * range-check set ip-check <UINT:entry_index> ( ipv4-dip | ipv4-sip | ipv6-dip | ipv6-sip ) ( ip1 | ip2 | ip3 | ip4 ) <UINT:lower> <UINT:upper>
 */
cparser_result_t cparser_cmd_range_check_set_ip_check_entry_index_ipv4_dip_ipv4_sip_ipv6_dip_ipv6_sip_ip1_ip2_ip3_ip4_lower_upper(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *lower_ptr,
    uint32_t *upper_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_ip_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_ip_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckIp_get(unit, *entry_index_ptr, &data), ret);

    switch (TOKEN_CHAR(4,3))
    {
        case '4':
            switch (TOKEN_CHAR(4,5))
            {
                case 'd': /* ipv4-dip */
                    data.ip_type = IP_TYPE_IPV4_DST;
                    break;
                case 's': /* ipv4-sip */
                    data.ip_type = IP_TYPE_IPV4_SRC;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        case '6':
            switch (TOKEN_CHAR(4,5))
            {
                case 'd': /* ipv6-dip */
                    data.ip_type = IP_TYPE_IPV6_DST;
                    break;
                case 's': /* ipv6-sip */
                    data.ip_type = IP_TYPE_IPV6_SRC;
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(5,2))
    {
        case '1': /* ip1 */
            data.ip1_lower_bound = *lower_ptr;
            data.ip1_upper_bound = *upper_ptr;
            break;
        case '2': /* ip2 */
            data.ip2_lower_bound = *lower_ptr;
            data.ip2_upper_bound = *upper_ptr;
            break;
        case '3': /* ip3 */
            data.ip3_lower_bound = *lower_ptr;
            data.ip3_upper_bound = *upper_ptr;
            break;
        case '4': /* ip4 */
            data.ip4_lower_bound = *lower_ptr;
            data.ip4_upper_bound = *upper_ptr;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckIp_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_ip_check_entry_index_ipv4_dip_ipv4_sip_ipv6_dip_ipv6_sip_ip1_ip2_ip3_ip4_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_IP_CHECK_ENTRY_INDEX_VALID_MASK_MASK
/*
 * range-check set ip-check <UINT:entry_index> valid-mask <UINT:mask>
 */
cparser_result_t cparser_cmd_range_check_set_ip_check_entry_index_valid_mask_mask(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *mask_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_ip_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_ip_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckIp_get(unit, *entry_index_ptr, &data), ret);

    data.valid_mask = *mask_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckIp_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_ip_check_entry_index_valid_mask_mask */
#endif

#ifdef CMD_RANGE_CHECK_SET_L4PORT_CHECK_ENTRY_INDEX_INVALID
/*
 * range-check set l4port-check <UINT:entry_index> invalid
 */
cparser_result_t cparser_cmd_range_check_set_l4port_check_entry_index_invalid(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_l4Port_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_l4Port_t));

    data.l4port_type = L4PORT_TYPE_INVALID;

    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckL4Port_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_l4port_check_entry_index_invalid */
#endif

#ifdef CMD_RANGE_CHECK_SET_L4PORT_CHECK_ENTRY_INDEX_BOTH_TCP_UDP_DST_PORT_SRC_PORT_LOWER_UPPER
/*
 * range-check set l4port-check <UINT:entry_index> ( both | tcp | udp ) ( dst-port | src-port ) <UINT:lower> <UINT:upper>
 */
cparser_result_t cparser_cmd_range_check_set_l4port_check_entry_index_both_tcp_udp_dst_port_src_port_lower_upper(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *lower_ptr,
    uint32_t *upper_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_l4Port_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_l4Port_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckL4Port_get(unit, *entry_index_ptr, &data), ret);

    switch (TOKEN_CHAR(4,0))
    {
        case 'b': /* both */
            data.l4port_type = L4PORT_TYPE_TCPORUDP;
            break;
        case 't': /* tcp */
            data.l4port_type = L4PORT_TYPE_TCP;
            break;
        case 'u': /* udp */
            data.l4port_type = L4PORT_TYPE_UDP;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    switch (TOKEN_CHAR(5,0))
    {
        case 'd': /* dst-port */
            data.dest_port = L4PORT_DIRECTION_DST;
            break;
        case 's': /* src-port */
            data.dest_port = L4PORT_DIRECTION_SRC;
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    data.upper_bound = *upper_ptr;
    data.lower_bound = *lower_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckL4Port_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_l4port_check_entry_index_both_tcp_udp_dst_port_src_port_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_PORT_CHECK_ENTRY_INDEX_PORT_INVALID
/*
 * range-check set port-check <UINT:entry_index> ( <PORT_LIST:port> | invalid )
 */
cparser_result_t cparser_cmd_range_check_set_port_check_entry_index_port_invalid(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_srcPortMask_t data;
    rtk_portmask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('i' == TOKEN_CHAR(4,0))
    {
        memset(&mask, 0, sizeof(rtk_portmask_t));
        memcpy(&mask, &data.src_port_mask, sizeof(rtk_portmask_t));
    }
    else
    {
        memset(&mask, 0, sizeof(rtk_portmask_t));
        DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask((uint8 *)TOKEN_STR(4), &mask), ret);
        memcpy(&mask, &data.src_port_mask, sizeof(rtk_portmask_t));
    }

    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckSrcPort_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_port_check_entry_index_port */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_CHECK_ENTRY_INDEX_BOTH_SRC_PORT_INNER_VID_ILOWER_IUPPER_OUTER_VID_OLOWER_OUPPER
/*
 * range-check set vid-check <UINT:entry_index> both <UINT:src_port> inner-vid <UINT:ilower> <UINT:iupper> outer-vid <UINT:olower> <UINT:oupper>
 */
cparser_result_t cparser_cmd_range_check_set_vid_check_entry_index_both_src_port_inner_vid_ilower_iupper_outer_vid_olower_oupper(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *src_port_ptr,
    uint32_t *ilower_ptr,
    uint32_t *iupper_ptr,
    uint32_t *olower_ptr,
    uint32_t *oupper_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_vid_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_vid_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_get(unit, *entry_index_ptr, &data), ret);

    data.vid_type = VID_TYPE_INNEROROUTER;
    data.ivid_upper_bound = *iupper_ptr;
    data.ivid_lower_bound = *ilower_ptr;
    data.ovid_upper_bound = *oupper_ptr;
    data.ovid_lower_bound = *olower_ptr;
    data.src_port_num = *src_port_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_vid_check_entry_index_both_src_port_inner_vid_ilower_iupper_outer_vid_olower_oupper */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_CHECK_ENTRY_INDEX_INNER_SRC_PORT_INNER_VID_ILOWER_IUPPER
/*
 * range-check set vid-check <UINT:entry_index> inner <UINT:src_port> inner-vid <UINT:ilower> <UINT:iupper>
 */
cparser_result_t cparser_cmd_range_check_set_vid_check_entry_index_inner_src_port_inner_vid_ilower_iupper(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *src_port_ptr,
    uint32_t *ilower_ptr,
    uint32_t *iupper_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_vid_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_vid_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_get(unit, *entry_index_ptr, &data), ret);

    data.vid_type = VID_TYPE_INNER;
    data.ivid_upper_bound = *iupper_ptr;
    data.ivid_lower_bound = *ilower_ptr;
    data.ovid_upper_bound = 0;
    data.ovid_lower_bound = 0;
    data.src_port_num = *src_port_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_vid_check_entry_index_inner_src_port_inner_vid_ilower_iupper */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_CHECK_ENTRY_INDEX_INVALID
/*
 * range-check set vid-check <UINT:entry_index> invalid
 */
cparser_result_t cparser_cmd_range_check_set_vid_check_entry_index_invalid(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_vid_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_vid_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_get(unit, *entry_index_ptr, &data), ret);

    data.vid_type = VID_TYPE_INVALID;
    data.ivid_upper_bound = 0;
    data.ivid_lower_bound = 0;
    data.ovid_upper_bound = 0;
    data.ovid_lower_bound = 0;
    data.src_port_num = 0;
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_vid_check_entry_index_invalid */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_CHECK_ENTRY_INDEX_OUTER_SRC_PORT_OUTER_VID_OLOWER_OUPPER
/*
 * range-check set vid-check <UINT:entry_index> outer <UINT:src_port> outer-vid <UINT:olower> <UINT:oupper>
 */
cparser_result_t cparser_cmd_range_check_set_vid_check_entry_index_outer_src_port_outer_vid_olower_oupper(cparser_context_t *context,
    uint32_t *entry_index_ptr,
    uint32_t *src_port_ptr,
    uint32_t *olower_ptr,
    uint32_t *oupper_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_vid_t data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&data, 0, sizeof(rtk_pie_rangeCheck_vid_t));
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_get(unit, *entry_index_ptr, &data), ret);

    data.vid_type = VID_TYPE_OUTER;
    data.ivid_upper_bound = 0;
    data.ivid_lower_bound = 0;
    data.ovid_upper_bound = *oupper_ptr;
    data.ovid_lower_bound = *olower_ptr;
    data.src_port_num = *src_port_ptr;
    DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_set(unit, *entry_index_ptr, &data), ret);

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_set_vid_check_entry_index_outer_src_port_outer_vid_olower_oupper */
#endif

#ifdef CMD_RANGE_CHECK_GET_IP_CHECK_L4PORT_CHECK_PORT_CHECK_VID_CHECK_ENTRY_INDEX
/*
 * range-check get ( ip-check | l4port-check | port-check | vid-check ) <UINT:entry_index>
 */
cparser_result_t cparser_cmd_range_check_get_ip_check_l4port_check_port_check_vid_check_entry_index(cparser_context_t *context,
    uint32_t *entry_index_ptr)
{
    uint32                              unit = 0;
    int32                               ret = RT_ERR_FAILED;
    rtk_pie_rangeCheck_ip_t             ip_data;
    rtk_pie_rangeCheck_l4Port_t         l4Port_data;
    rtk_pie_rangeCheck_srcPortMask_t       srcPort_data;
    rtk_pie_rangeCheck_vid_t            vid_data;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch (TOKEN_CHAR(2,0))
    {
        case 'i': /* ip-check */
            memset(&ip_data, 0, sizeof(rtk_pie_rangeCheck_ip_t));
            DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckIp_get(unit, *entry_index_ptr, &ip_data), ret);
            diag_util_mprintf("IP check : %d\n", *entry_index_ptr);
            diag_util_mprintf("\tip4_lower_bound : 0x%x\n", ip_data.ip4_lower_bound);
            diag_util_mprintf("\tip4_upper_bound : 0x%x\n", ip_data.ip4_upper_bound);
            diag_util_mprintf("\tip3_lower_bound : 0x%x\n", ip_data.ip3_lower_bound);
            diag_util_mprintf("\tip3_upper_bound : 0x%x\n", ip_data.ip3_upper_bound);
            diag_util_mprintf("\tip2_lower_bound : 0x%x\n", ip_data.ip2_lower_bound);
            diag_util_mprintf("\tip2_upper_bound : 0x%x\n", ip_data.ip2_upper_bound);
            diag_util_mprintf("\tip1_lower_bound : 0x%x\n", ip_data.ip1_lower_bound);
            diag_util_mprintf("\tip1_upper_bound : 0x%x\n", ip_data.ip1_upper_bound);
            diag_util_mprintf("\tvalid_mask : 0x%x\n", ip_data.valid_mask);
            diag_util_printf("\tip_type : ");
            switch (ip_data.ip_type)
            {
                case IP_TYPE_IPV4_SRC:
                    diag_util_printf("ipv4-sip");
                    break;
                case IP_TYPE_IPV4_DST:
                    diag_util_printf("ipv4-dip");
                    break;
                case IP_TYPE_IPV6_SRC:
                    diag_util_printf("ipv6-sip");
                    break;
                case IP_TYPE_IPV6_DST:
                    diag_util_printf("ipv6-dip");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");
            break;
        case 'l': /* l4port-check */
            memset(&l4Port_data, 0, sizeof(rtk_pie_rangeCheck_l4Port_t));
            DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckL4Port_get(unit, *entry_index_ptr, &l4Port_data), ret);
            diag_util_mprintf("L4-port check : %d\n", *entry_index_ptr);
            diag_util_mprintf("\tupper_bound : %d\n", l4Port_data.upper_bound);
            diag_util_mprintf("\tlower_bound : %d\n", l4Port_data.lower_bound);
            diag_util_printf("\tdest_port : ");
            switch (l4Port_data.dest_port)
            {
                case L4PORT_DIRECTION_SRC:
                    diag_util_printf("src-port");
                    break;
                case L4PORT_DIRECTION_DST:
                    diag_util_printf("dst-port");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");
            diag_util_printf("\tl4port_type : ");
            switch (l4Port_data.l4port_type)
            {
                case L4PORT_TYPE_INVALID:
                    diag_util_printf("invalid");
                    break;
                case L4PORT_TYPE_TCP:
                    diag_util_printf("tcp");
                    break;
                case L4PORT_TYPE_UDP:
                    diag_util_printf("ucp");
                    break;
                case L4PORT_TYPE_TCPORUDP:
                    diag_util_printf("both");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");
            break;
        case 'p': /* port-check */
            memset(&srcPort_data, 0, sizeof(rtk_pie_rangeCheck_srcPortMask_t));
            DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckSrcPort_get(unit, *entry_index_ptr, &srcPort_data), ret);
            diag_util_mprintf("Source-port check : %d\n", *entry_index_ptr);
            diag_util_mprintf("\tsrc_port_mask : 0x%x\n", srcPort_data.src_port_mask);
            break;
        case 'v': /* vid-check */
            memset(&vid_data, 0, sizeof(rtk_pie_rangeCheck_vid_t));
            DIAG_UTIL_ERR_CHK(rtk_pie_rangeCheckVid_get(unit, *entry_index_ptr, &vid_data), ret);
            diag_util_mprintf("VID check : %d\n", *entry_index_ptr);
            diag_util_mprintf("\tivid_upper_bound : %d\n", vid_data.ivid_upper_bound);
            diag_util_mprintf("\tivid_lower_bound : %d\n", vid_data.ivid_lower_bound);
            diag_util_mprintf("\tovid_upper_bound : %d\n", vid_data.ovid_upper_bound);
            diag_util_mprintf("\tovid_lower_bound : %d\n", vid_data.ovid_lower_bound);
            diag_util_mprintf("\tsrc_port_num : %d\n", vid_data.src_port_num);
            diag_util_printf("\tvid_type : ");
            switch (vid_data.vid_type)
            {
                case VID_TYPE_INVALID:
                    diag_util_printf("invalid");
                    break;
                case VID_TYPE_INNER:
                    diag_util_printf("inner");
                    break;
                case VID_TYPE_OUTER:
                    diag_util_printf("outer");
                    break;
                case VID_TYPE_INNEROROUTER:
                    diag_util_printf("both");
                    break;
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
            }
            diag_util_mprintf("\n");
            break;
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}/* end of cparser_cmd_range_check_get_ip_check_l4port_check_port_check_vid_check_entry_index */
#endif

