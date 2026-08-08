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
 * $Revision: 22709 $
 * $Date: 2011-09-19 10:05:18 +0800 (Mon, 19 Sep 2011) $
 *
 * Purpose : Define diag shell functions for filter.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) flow table
 *           2) ingress ACL
 */

/*  
 * Include Files 
 */
#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/util/rt_util.h>
#include <rtk/filter.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


#define     ACL_ACTION_DIRECTION                (0)
#define     ACL_ACTION_MIRROR                   (1)
#define     ACL_ACTION_LOG                      (2)
#define     ACL_ACTION_RATELIMIT                (4)
#define     ACL_ACTION_NEWSVID                  (5)
#define     ACL_ACTION_NEWCVID                  (6)

#define     FLOW_ACTION_LOG                     (0)
#define     FLOW_ACTION_ASSIGNVLAN              (4)

#define     ACL_ACTION_DIRECTION_PERMIT         (0)
#define     ACL_ACTION_DIRECTION_DROP           (1)
#define     ACL_ACTION_DIRECTION_REDIRECT       (2)
#define     ACL_ACTION_DIRECTION_COPYTOCPUY     (3)

#define     PIE_MAX_LOG_INDEX                   (128)
#define     PIE_MAX_RATELIMIT_INDEX             (128)

#define     PIE_BIT_ON                          (1)
#define     PIE_BIT_OFF                         (0)

#define     MAC_ADDR_LEN                        (6)

/*
 * Function Declaration
 */
#if defined(CONFIG_SDK_RTL8328)
static int32 _parse_acl_rule(char *argv, uint32 rule_content, uint32 rule_carebit, rtk_filter_aclCfg_t *acl_cfg);
static int32 _parse_flowTable_rule(char *argv, uint32 rule_content, uint32 rule_carebit, rtk_filter_flowTbl_t *flow_table_cfg);
#endif

#ifdef CMD_FILTER_SET_CUT_LINE_VALUE
/*
 * filter set cut-line <UINT:value>
 */
cparser_result_t cparser_cmd_filter_set_cut_line_value(cparser_context_t *context,
    uint32_t *value_ptr)
{
    uint32    unit = 0;
    uint32    cutline = 0;
    int32     ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    cutline = *value_ptr;
    DIAG_UTIL_ERR_CHK(rtk_filter_blkCutline_set(unit, cutline), ret);
            
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_STATE_DISABLE_ENABLE
/*
 * filter set state ( disable | enable )
 */
cparser_result_t cparser_cmd_filter_set_state_disable_enable(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_pieEnable_set(unit, ENABLED), ret);
    }    
    else if ('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_pieEnable_set(unit, DISABLED), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_LOG_COUNTER_INDEX_PACKET_COUNTER_BYTE_COUNTER
/*
 * filter set log-counter <UINT:index> <UINT:packet_counter> <UINT64:byte_counter>
 */
cparser_result_t cparser_cmd_filter_set_log_counter_index_packet_counter_byte_counter(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *packet_counter_ptr, uint64_t *byte_counter_ptr)
{
    uint64   byte_counter = 0;
    uint32   packet_counter = 0;
    uint32   unit = 0;
    int32    log_index = 0;
    int32    ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    log_index = *index_ptr;
    packet_counter = *packet_counter_ptr;
    byte_counter = *byte_counter_ptr;
    DIAG_UTIL_ERR_CHK(rtk_filter_stat_set(unit, log_index, packet_counter, byte_counter), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_RATE_LIMIT_INDEX_RATE_VALUE
/*
 * filter set rate-limit <UINT:index> rate <UINT:value>
 */
cparser_result_t cparser_cmd_filter_set_rate_limit_index_rate_value(cparser_context_t *context,
    uint32_t *index_ptr, uint32_t *value_ptr)
{
    uint32  unit = 0;
    uint32  rate = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    index = *index_ptr;
    rate = *value_ptr;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAclRateLimit_set(unit, index, rate), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_PERMIT_DROP_COPY_TO_CPU
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action ( permit | drop | copy-to-cpu )
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_permit_drop_copy_to_cpu(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);

    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));
        
    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    } 
    
    if ('p' == TOKEN_CHAR(9,0)) /* permit */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_PERMIT;
    }
    else if ('d' == TOKEN_CHAR(9,0)) /* drop */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_DROP;
    }
    else if ('c' == TOKEN_CHAR(9,0)) /* copy-to-cpy */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_COPYTOCPUY;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    } 
       
    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_REDIRECT_PORT_ID
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action redirect <UINT:port_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_redirect_port_id(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *port_id_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));
    
    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_DIRECTION;
    action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_REDIRECT;
    action.un.permit_drop_redirect.portid = *port_id_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_MIRROR_MIRROR_ID
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action mirror <UINT:mirror_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_mirror_mirror_id(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *mirror_id_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              mirror_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));
    
    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    mirror_id = *mirror_id_ptr;
    action.actGroup = ACL_ACTION_MIRROR;
    action.un.mirror.mirrorsetid = mirror_id;
        
    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_LOG_LOG_ID
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action log <UINT:log_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_log_log_id(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *log_id_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              log_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if(TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
        
    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    log_id = *log_id_ptr;
    action.actGroup = ACL_ACTION_LOG;
    action.un.log.logindex = log_id;
    
    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_RATE_LIMIT_RATE_ID
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action rate-limit <UINT:rate_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_rate_limit_rate_id(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *rate_id_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              rate_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    rate_id = *rate_id_ptr;
    action.actGroup = ACL_ACTION_RATELIMIT;
    action.un.ratelimit.rateindex = rate_id;
    
    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_NEW_SVID_SVID
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action new-svid <UINT:svid>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_new_svid_svid(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *svid_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              svid = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    svid = *svid_ptr;
    action.actGroup = ACL_ACTION_NEWSVID;
    action.un.newsvid.svid = svid;
    
    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_NEW_CVID_CVID_CVID_CPRI_CPRI
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action new-cvid cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_new_cvid_cvid_cvid_cpri_cpri(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *cvid_ptr, uint32_t *cpri_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_ON;
    action.un.newcvid.replacecpri = PIE_BIT_ON;
    action.un.newcvid.cvid = *cvid_ptr;
    action.un.newcvid.cpri = *cpri_ptr;
    
    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_NEW_CVID_CVID_CVID
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action new-cvid cvid <UINT:cvid>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_new_cvid_cvid_cvid(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *cvid_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_ON;
    action.un.newcvid.replacecpri = PIE_BIT_OFF;
    action.un.newcvid.cvid = *cvid_ptr;
    action.un.newcvid.cpri = 0;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_NEW_CVID_CPRI_CPRI
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action new-cvid cpri <UINT:cpri>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_new_cvid_cpri_cpri(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *cpri_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_OFF;
    action.un.newcvid.replacecpri = PIE_BIT_ON;
    action.un.newcvid.cvid = 0;
    action.un.newcvid.cpri = *cpri_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_RVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_TCP_FLAG_PATTERN_MATCH_PKT_SVID_CONTENT_CAREBITS_ACTION_NEW_CVID
/*
 * filter set acl rule <UINT:index> ( slp | frame-type | ether-type | rvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | tcp-flag | pattern-match | pkt-svid ) <STRING:content> <STRING:carebits> action new-cvid
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_slp_frame_type_ether_type_rvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_tcp_flag_pattern_match_pkt_svid_content_carebits_action_new_cvid(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr)
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_acl_rule(TOKEN_STR(5), rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_OFF;
    action.un.newcvid.replacecpri = PIE_BIT_OFF;
    action.un.newcvid.cvid = 0;
    action.un.newcvid.cpri = 0;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_PERMIT_DROP_COPY_TO_CPU
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action ( permit | drop | copy-to-cpu )
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_permit_drop_copy_to_cpu(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    if ('p' == TOKEN_CHAR(9,0)) /* permit */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_PERMIT;
    }
    else if ('d' == TOKEN_CHAR(9,0)) /* drop */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_DROP;
    }
    else if ('c' == TOKEN_CHAR(9,0)) /* copy-to-cpy */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_COPYTOCPUY;
    } 
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
       
    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_REDIRECT_PORT_ID
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action redirect <UINT:port_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_redirect_port_id(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *port_id_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_DIRECTION;
    action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_REDIRECT;
    action.un.permit_drop_redirect.portid = *port_id_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_MIRROR_MIRROR_ID
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action mirror <UINT:mirror_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_mirror_mirror_id(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *mirror_id_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_MIRROR;
    action.un.mirror.mirrorsetid = *mirror_id_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_LOG_LOG_ID
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action log <UINT:log_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_log_log_id(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *log_id_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_LOG;
    action.un.log.logindex = *log_id_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_RATE_LIMIT_RATE_ID
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action rate-limit <UINT:rate_id>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_rate_limit_rate_id(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *rate_id_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_RATELIMIT;
    action.un.ratelimit.rateindex = *rate_id_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_NEW_SVID_SVID
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action new-svid <UINT:svid>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_new_svid_svid(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *svid_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWSVID;
    action.un.newsvid.svid = *svid_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_NEW_CVID_CVID_CVID_CPRI_CPRI
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action new-cvid cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_new_cvid_cvid_cvid_cpri_cpri(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *cvid_ptr, uint32_t *cpri_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_ON;
    action.un.newcvid.replacecpri = PIE_BIT_ON;
    action.un.newcvid.cvid = *cvid_ptr;
    action.un.newcvid.cpri = *cpri_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_NEW_CVID_CVID_CVID
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action new-cvid cvid <UINT:cvid>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_new_cvid_cvid_cvid(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *cvid_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_ON;
    action.un.newcvid.replacecpri = PIE_BIT_OFF;
    action.un.newcvid.cvid = *cvid_ptr;
    action.un.newcvid.cpri = 0;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_NEW_CVID_CPRI_CPRI
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action new-cvid cpri <UINT:cpri>
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_new_cvid_cpri_cpri(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *cpri_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_OFF;
    action.un.newcvid.replacecpri = PIE_BIT_ON;
    action.un.newcvid.cvid = 0;
    action.un.newcvid.cpri = *cpri_ptr;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_ACL_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_NEW_CVID
/*
 * filter set acl rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action new-cvid
 */
cparser_result_t cparser_cmd_filter_set_acl_rule_index_dmac_smac_mac_carebits_mac_action_new_cvid(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action), ret);
    if (TRUE == acl_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    action.un.newcvid.replacecvid = PIE_BIT_OFF;
    action.un.newcvid.replacecpri = PIE_BIT_OFF;
    action.un.newcvid.cvid = 0;
    action.un.newcvid.cpri = 0;

    acl_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_PKT_CTAG_IF_PKT_CPRI_PKT_CVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_PKT_STAG_IF_PKT_SPRI_PKT_SVID_CONTENT_CAREBITS_ACTION_LOG_LOG_ID
/*
 * filter set flow-table rule <UINT:index> ( slp | frame-type | ether-type | pkt-ctag-if | pkt-cpri | pkt-cvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | pkt-stag-if | pkt-spri | pkt-svid ) <STRING:content> <STRING:carebits> action log <UINT:log_id>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_slp_frame_type_ether_type_pkt_ctag_if_pkt_cpri_pkt_cvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_pkt_stag_if_pkt_spri_pkt_svid_content_carebits_action_log_log_id(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *log_id_ptr)
{
    uint32               unit = 0;
    uint32               rule_content = 0;
    uint32               rule_carebit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    /* support API: is empty rule check?? */
    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_flowTable_rule(TOKEN_STR(5), rule_content, rule_carebit, &flow_table_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    action.actGroup = FLOW_ACTION_LOG;
    action.un.log.logindex = *log_id_ptr;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_PKT_CTAG_IF_PKT_CPRI_PKT_CVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_PKT_STAG_IF_PKT_SPRI_PKT_SVID_CONTENT_CAREBITS_ACTION_ASSIGN_VLAN_CTAG_CTAG_SPRI_SPRI
/*
 * filter set flow-table rule <UINT:index> ( slp | frame-type | ether-type | pkt-ctag-if | pkt-cpri | pkt-cvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | pkt-stag-if | pkt-spri | pkt-svid ) <STRING:content> <STRING:carebits> action assign-vlan ctag <UINT:ctag> spri <UINT:spri>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_slp_frame_type_ether_type_pkt_ctag_if_pkt_cpri_pkt_cvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_pkt_stag_if_pkt_spri_pkt_svid_content_carebits_action_assign_vlan_ctag_ctag_spri_spri(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *ctag_ptr, uint32_t *spri_ptr)
{
    uint32               unit = 0;
    uint32               rule_content = 0;
    uint32               rule_carebit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_flowTable_rule(TOKEN_STR(5), rule_content, rule_carebit, &flow_table_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 1;
    action.un.assignVlan.usepktspri = 1;
    action.un.assignVlan.actvid = *ctag_ptr;
    action.un.assignVlan.actpri = *spri_ptr;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_PKT_CTAG_IF_PKT_CPRI_PKT_CVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_PKT_STAG_IF_PKT_SPRI_PKT_SVID_CONTENT_CAREBITS_ACTION_ASSIGN_VLAN_CTAG_CTAG
/*
 * filter set flow-table rule <UINT:index> ( slp | frame-type | ether-type | pkt-ctag-if | pkt-cpri | pkt-cvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | pkt-stag-if | pkt-spri | pkt-svid ) <STRING:content> <STRING:carebits> action assign-vlan ctag <UINT:ctag>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_slp_frame_type_ether_type_pkt_ctag_if_pkt_cpri_pkt_cvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_pkt_stag_if_pkt_spri_pkt_svid_content_carebits_action_assign_vlan_ctag_ctag(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *ctag_ptr)
{
    uint32               unit = 0;
    uint32               rule_content = 0;
    uint32               rule_carebit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_flowTable_rule(TOKEN_STR(5), rule_content, rule_carebit, &flow_table_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 1;
    action.un.assignVlan.usepktspri = 0;
    action.un.assignVlan.actvid = *ctag_ptr;
    action.un.assignVlan.actpri = 0;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_PKT_CTAG_IF_PKT_CPRI_PKT_CVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_PKT_STAG_IF_PKT_SPRI_PKT_SVID_CONTENT_CAREBITS_ACTION_ASSIGN_VLAN_SPRI_SPRI
/*
 * filter set flow-table rule <UINT:index> ( slp | frame-type | ether-type | pkt-ctag-if | pkt-cpri | pkt-cvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | pkt-stag-if | pkt-spri | pkt-svid ) <STRING:content> <STRING:carebits> action assign-vlan spri <UINT:spri>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_slp_frame_type_ether_type_pkt_ctag_if_pkt_cpri_pkt_cvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_pkt_stag_if_pkt_spri_pkt_svid_content_carebits_action_assign_vlan_spri_spri(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr, uint32_t *spri_ptr)
{
    uint32               unit = 0;
    uint32               rule_content = 0;
    uint32               rule_carebit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_flowTable_rule(TOKEN_STR(5), rule_content, rule_carebit, &flow_table_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 0;
    action.un.assignVlan.usepktspri = 1;
    action.un.assignVlan.actvid = 0;
    action.un.assignVlan.actpri = *spri_ptr;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_SLP_FRAME_TYPE_ETHER_TYPE_PKT_CTAG_IF_PKT_CPRI_PKT_CVID_IPV4_IPV6_IPV6_MLD_SIP_DIP_FLOW_LABEL_IP_PROTO_TOS_SRC_PORT_DST_PORT_PKT_STAG_IF_PKT_SPRI_PKT_SVID_CONTENT_CAREBITS_ACTION_ASSIGN_VLAN
/*
 * filter set flow-table rule <UINT:index> ( slp | frame-type | ether-type | pkt-ctag-if | pkt-cpri | pkt-cvid | ipv4 | ipv6 | ipv6-mld | sip | dip | flow-label | ip-proto | tos | src-port | dst-port | pkt-stag-if | pkt-spri | pkt-svid ) <STRING:content> <STRING:carebits> action assign-vlan
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_slp_frame_type_ether_type_pkt_ctag_if_pkt_cpri_pkt_cvid_ipv4_ipv6_ipv6_mld_sip_dip_flow_label_ip_proto_tos_src_port_dst_port_pkt_stag_if_pkt_spri_pkt_svid_content_carebits_action_assign_vlan(cparser_context_t *context,
    uint32_t *index_ptr, char **content_ptr, char **carebits_ptr)
{
    uint32               unit = 0;
    uint32               rule_content = 0;
    uint32               rule_carebit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    diag_util_str2ul(&rule_content, TOKEN_STR(6));
    diag_util_str2ul(&rule_carebit, TOKEN_STR(7));

    if (_parse_flowTable_rule(TOKEN_STR(5), rule_content, rule_carebit, &flow_table_cfg) != RT_ERR_OK)
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 0;
    action.un.assignVlan.usepktspri = 0;
    action.un.assignVlan.actvid = 0;
    action.un.assignVlan.actpri = 0;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_LOG_LOG_ID
/*
 * filter set flow-table rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action log <UINT:log_id>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_dmac_smac_mac_carebits_mac_action_log_log_id(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *log_id_ptr)
{
    uint32               unit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    uint8                mac_content[MAC_ADDR_LEN];
    uint8                mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    action.actGroup = FLOW_ACTION_LOG;
    action.un.log.logindex = *log_id_ptr;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_ASSIGN_VLAN_CTAG_CTAG_SPRI_SPRI
/*
 * filter set flow-table rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action assign-vlan ctag <UINT:ctag> spri <UINT:spri>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_dmac_smac_mac_carebits_mac_action_assign_vlan_ctag_ctag_spri_spri(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *ctag_ptr, uint32_t *spri_ptr)
{
    uint32               unit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    uint8                mac_content[MAC_ADDR_LEN];
    uint8                mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 1;
    action.un.assignVlan.usepktspri = 1;
    action.un.assignVlan.actvid = *ctag_ptr;
    action.un.assignVlan.actpri = *spri_ptr;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_ASSIGN_VLAN_CTAG_CTAG
/*
 * filter set flow-table rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action assign-vlan ctag <UINT:ctag>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_dmac_smac_mac_carebits_mac_action_assign_vlan_ctag_ctag(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *ctag_ptr)
{
    uint32               unit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    uint8                mac_content[MAC_ADDR_LEN];
    uint8                mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 1;
    action.un.assignVlan.usepktspri = 0;
    action.un.assignVlan.actvid = *ctag_ptr;
    action.un.assignVlan.actpri = 0;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_ASSIGN_VLAN_SPRI_SPRI
/*
 * filter set flow-table rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action assign-vlan spri <UINT:spri>
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_dmac_smac_mac_carebits_mac_action_assign_vlan_spri_spri(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr, uint32_t *spri_ptr)
{
    uint32               unit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    uint8                mac_content[MAC_ADDR_LEN];
    uint8                mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 0;
    action.un.assignVlan.usepktspri = 1;
    action.un.assignVlan.actvid = 0;
    action.un.assignVlan.actpri = *spri_ptr;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_SET_FLOW_TABLE_RULE_INDEX_DMAC_SMAC_MAC_CAREBITS_MAC_ACTION_ASSIGN_VLAN
/*
 * filter set flow-table rule <UINT:index> ( dmac | smac ) <MACADDR:mac> <MACADDR:carebits_mac> action assign-vlan
 */
cparser_result_t cparser_cmd_filter_set_flow_table_rule_index_dmac_smac_mac_carebits_mac_action_assign_vlan(cparser_context_t *context,
    uint32_t *index_ptr, cparser_macaddr_t *mac_ptr, cparser_macaddr_t *carebits_mac_ptr)
{
    uint32               unit = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    uint8                mac_content[MAC_ADDR_LEN];
    uint8                mac_carebit[MAC_ADDR_LEN];
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    filter_id = *index_ptr;
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action), ret);
    if (TRUE == flow_table_cfg.valid)
    {
        DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, filter_id), ret);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    diag_util_str2mac(mac_content, TOKEN_STR(6));
    diag_util_str2mac(mac_carebit, TOKEN_STR(7));
        
    if ('s' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        memcpy(flow_table_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    action.un.assignVlan.usepktctag = 0;
    action.un.assignVlan.usepktspri = 0;
    action.un.assignVlan.actvid = 0;
    action.un.assignVlan.actpri = 0;
    
    flow_table_cfg.valid = TRUE;
    DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_GET_CUT_LINE
/*
 * filter get cut-line
 */
cparser_result_t cparser_cmd_filter_get_cut_line(cparser_context_t *context)
{
    uint32       unit = 0;
    uint32       cutline = 0;
    int32        ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_filter_blkCutline_get(unit, &cutline), ret);
    diag_util_mprintf(" Cutline: %d\n", cutline);

    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_GET_STATE
/*
 * filter get state 
 */
cparser_result_t cparser_cmd_filter_get_state(cparser_context_t *context)
{
    uint32       unit = 0;
    int32        ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_filter_pieEnable_get(unit, &enable), ret);
    if (ENABLED == enable)
    {
        diag_util_mprintf(" PIE: enable\n");
    } 
    else if (DISABLED == enable)
    {
        diag_util_mprintf(" PIE: disable\n");
    }     
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_GET_LOG_COUNTER_INDEX
/*
 * filter get log-counter { <UINT:index> }
 */
cparser_result_t cparser_cmd_filter_get_log_counter_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint64  byte_counter = 0;
    uint32  packet_counter = 0;
    uint32  unit = 0;
    uint32  log_index = 0;
    int32   ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (TOKEN_NUM < 4)
    {
        for (log_index = 0; log_index < PIE_MAX_LOG_INDEX; log_index++)
        {
            DIAG_UTIL_ERR_CHK(rtk_filter_stat_get(unit, log_index, &packet_counter, &byte_counter), ret);
            diag_util_mprintf("Index:%d    pktcnt=%lu   bytecnt=%llu \n", log_index, packet_counter, byte_counter);
        }    
    }
    else
    {
        log_index = *index_ptr;
        DIAG_UTIL_ERR_CHK(rtk_filter_stat_get(unit, log_index, &packet_counter, &byte_counter), ret);
        diag_util_mprintf("Index:%d    pktcnt=%lu   bytecnt=%llu \n", log_index, packet_counter, byte_counter);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_GET_RATE_LIMIT_INDEX
/*
 * filter get rate-limit { <UINT:index> }
 */
cparser_result_t cparser_cmd_filter_get_rate_limit_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    uint32  rate = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (TOKEN_NUM < 4)
    {
        for (index = 0; index < PIE_MAX_RATELIMIT_INDEX; index++)
        {
            DIAG_UTIL_ERR_CHK(rtk_filter_igrAclRateLimit_get(unit,  index, &rate), ret);
            diag_util_mprintf("Index:%d    rate=0x%x\n", index, rate);
        }    
    }
    else
    {
        index = *index_ptr;
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAclRateLimit_get(unit,  index, &rate), ret);
        diag_util_mprintf("Index:%d    rate=0x%x\n", index, rate);
    }    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_GET_ACL_FLOW_TABLE_RULE_START_INDEX_END_INDEX
/*
 * filter get ( acl | flow-table ) rule <UINT:start_index> { <UINT:end_index> }
 */
cparser_result_t cparser_cmd_filter_get_acl_flow_table_rule_start_index_end_index(cparser_context_t *context,
    uint32_t *start_index_ptr, uint32_t *end_index_ptr)
{
    uint32               unit = 0;
    uint32               start_idx = 0;
    uint32               end_idx = 0;
    uint32               index = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_aclCfg_t  acl_cfg;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    start_idx = *start_index_ptr;
    if ('a' == TOKEN_CHAR(2,0))
    {
        if (TOKEN_NUM < 6)
        {
            end_idx = start_idx;
        }
        else
        {    
            end_idx = *end_index_ptr;
        } 
            
        for (index = start_idx; index <= end_idx; index++)
        {
            memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
            memset(&action, 0, sizeof(rtk_filter_action_t));
            DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, index, &acl_cfg, &action), ret);
            if (FALSE == acl_cfg.valid)
            {
                diag_util_mprintf("ACL rule %d is invalid\n", index);
            }
            else
            {
                diag_util_mprintf("ACL rule %d\n", index);
                diag_util_printf("slp:0x%x ", acl_cfg.slp);
                diag_util_printf("dmac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.dmac[0], acl_cfg.dmac[1], acl_cfg.dmac[2], acl_cfg.dmac[3], acl_cfg.dmac[4], acl_cfg.dmac[5]);
                diag_util_printf("smac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.smac[0], acl_cfg.smac[1], acl_cfg.smac[2], acl_cfg.smac[3], acl_cfg.smac[4], acl_cfg.smac[5]);
                diag_util_printf("frametype:0x%x ", acl_cfg.frametype);
                diag_util_printf("ethertype:0x%x ", acl_cfg.ethertype);
                diag_util_printf("rvid:0x%x ", acl_cfg.rvid);
                diag_util_printf("ipv4:0x%x ", acl_cfg.ipv4);
                diag_util_printf("ipv6:0x%x ", acl_cfg.ipv6);
                diag_util_printf("ipv6mld:0x%x ", acl_cfg.ipv6mld);
                diag_util_printf("sip:0x%x ", acl_cfg.sip);
                diag_util_printf("dip:0x%x ", acl_cfg.dip);
                diag_util_printf("flowlabel:0x%x ", acl_cfg.flowlabel);
                diag_util_printf("ipproto:0x%x ", acl_cfg.ipproto);
                diag_util_printf("tos:0x%x ", acl_cfg.tos);
                diag_util_printf("srcport:0x%x ", acl_cfg.srcport);
                diag_util_printf("dstport:0x%x ", acl_cfg.dstport);
                diag_util_printf("tcpflag:0x%x ", acl_cfg.tcpflag);
                diag_util_printf("patternmatch:0x%x ", acl_cfg.patternmatch);
                diag_util_printf("pktsvid:0x%x ", acl_cfg.pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("care_slp:0x%x ", acl_cfg.care_slp);
                diag_util_printf("care_dmac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.care_dmac[0], acl_cfg.care_dmac[1], acl_cfg.care_dmac[2], acl_cfg.care_dmac[3], acl_cfg.care_dmac[4], acl_cfg.care_dmac[5]);
                diag_util_printf("care_smac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.care_smac[0], acl_cfg.care_smac[1], acl_cfg.care_smac[2], acl_cfg.care_smac[3], acl_cfg.care_smac[4], acl_cfg.care_smac[5]);
                diag_util_printf("care_frametype:0x%x ", acl_cfg.care_frametype);
                diag_util_printf("care_ethertype:0x%x ", acl_cfg.care_ethertype);
                diag_util_printf("care_rvid:0x%x ", acl_cfg.care_rvid);
                diag_util_printf("care_ipv4:0x%x ", acl_cfg.care_ipv4);
                diag_util_printf("care_ipv6:0x%x ", acl_cfg.care_ipv6);
                diag_util_printf("care_ipv6mld:0x%x ", acl_cfg.care_ipv6mld);
                diag_util_printf("care_sip:0x%x ", acl_cfg.care_sip);
                diag_util_printf("care_dip:0x%x ", acl_cfg.care_dip);
                diag_util_printf("care_flowlabel:0x%x ", acl_cfg.care_flowlabel);
                diag_util_printf("care_ipproto:0x%x ", acl_cfg.care_ipproto);
                diag_util_printf("care_tos:0x%x ", acl_cfg.care_tos);
                diag_util_printf("care_srcport:0x%x ", acl_cfg.care_srcport);
                diag_util_printf("care_dstport:0x%x ", acl_cfg.care_dstport);
                diag_util_printf("care_tcpflag:0x%x ", acl_cfg.care_tcpflag);
                diag_util_printf("care_patternmatch:0x%x ", acl_cfg.care_patternmatch);
                diag_util_printf("care_pktsvid:0x%x ", acl_cfg.care_pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("action: ");
                switch(action.actGroup)
                {
                    case ACL_ACTION_DIRECTION:
                        switch(action.un.permit_drop_redirect.acttype)
                        {
                            case ACL_ACTION_DIRECTION_PERMIT:
                                diag_util_printf("permit ");
                                break;
                            case ACL_ACTION_DIRECTION_DROP:
                                diag_util_printf("drop ");
                                break;
                            case ACL_ACTION_DIRECTION_REDIRECT:
                                diag_util_printf("redirect port:%d ", action.un.permit_drop_redirect.portid);
                                break;
                            case ACL_ACTION_DIRECTION_COPYTOCPUY:
                                diag_util_printf("copy-to-cpu ");
                                break;
                            default:
                                break;
                        }
                        break;            
                        
                    case ACL_ACTION_MIRROR:
                        diag_util_printf("mirror ");
                        diag_util_printf("mirror-id:%d ", action.un.mirror.mirrorsetid);
                        break;
                        
                    case ACL_ACTION_LOG:
                        diag_util_printf("log ");
                        diag_util_printf("log-id:%d ", action.un.log.logindex);
                        break;
                
                    case ACL_ACTION_RATELIMIT:
                        diag_util_printf("rate-limit ");
                        diag_util_printf("rate-id:%d ", action.un.ratelimit.rateindex);
                        break;
                
                    case ACL_ACTION_NEWSVID:
                        diag_util_printf("new-svid ");
                        diag_util_printf("svid:%d ", action.un.newsvid.svid);
                        break;
                
                    case ACL_ACTION_NEWCVID:
                        diag_util_printf("new-cvid ");
                        diag_util_printf("replace-cvid:%d ", action.un.newcvid.replacecvid);
                        diag_util_printf("replace-cpri:%d ", action.un.newcvid.replacecpri);
                        diag_util_printf("cvid:%d ", action.un.newcvid.cvid);
                        diag_util_printf("cpri:%d ", action.un.newcvid.cpri);
                        break;
                        
                    default:
                        diag_util_printf("User config: Error!\n");
                        return CPARSER_NOT_OK;
                        break;
                }
                diag_util_mprintf("\n");
            }    
        } /* end of for (index = start_idx; index <= end_idx; index++) */
    }    
    else if (TOKEN_CHAR(2,0) == 'f')
    {
        if (TOKEN_NUM < 6)
        {
            end_idx = start_idx;
        }
        else
        {    
            end_idx = *end_index_ptr;
        }  
        
        for (index = start_idx; index <= end_idx; index++)
        {
            memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
            memset(&action, 0, sizeof(rtk_filter_action_t));
            DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, index, &flow_table_cfg, &action), ret);
            if (FALSE == flow_table_cfg.valid)
            {
                diag_util_mprintf("Flow table rule %d is invalid\n", index);
            }
            else
            {
                diag_util_mprintf("Flow table rule %d\n", index);
                diag_util_printf("slp:0x%x ", flow_table_cfg.slp);
                diag_util_printf("dmac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.dmac[0], flow_table_cfg.dmac[1], flow_table_cfg.dmac[2], flow_table_cfg.dmac[3], flow_table_cfg.dmac[4], flow_table_cfg.dmac[5]);
                diag_util_printf("smac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.smac[0], flow_table_cfg.smac[1], flow_table_cfg.smac[2], flow_table_cfg.smac[3], flow_table_cfg.smac[4], flow_table_cfg.smac[5]);
                diag_util_printf("frametype:0x%x ", flow_table_cfg.frametype);
                diag_util_printf("ethertype:0x%x ", flow_table_cfg.ethertype);
                diag_util_printf("pktctagif:0x%x ", flow_table_cfg.pktctagif);
                diag_util_printf("pktcpri:0x%x ", flow_table_cfg.pktcpri);
                diag_util_printf("pktcvid:0x%x ", flow_table_cfg.pktcvid);
                diag_util_printf("ipv4:0x%x ", flow_table_cfg.ipv4);
                diag_util_printf("ipv6:0x%x ", flow_table_cfg.ipv6);
                diag_util_printf("ipv6mld:0x%x ", flow_table_cfg.ipv6mld);
                diag_util_printf("sip:0x%x ", flow_table_cfg.sip);
                diag_util_printf("dip:0x%x ", flow_table_cfg.dip);
                diag_util_printf("flowlabel:0x%x ", flow_table_cfg.flowlabel);
                diag_util_printf("ipproto:0x%x ", flow_table_cfg.ipproto);
                diag_util_printf("tos:0x%x ", flow_table_cfg.tos);
                diag_util_printf("srcport:0x%x ", flow_table_cfg.srcport);
                diag_util_printf("dstport:0x%x ", flow_table_cfg.dstport);
                diag_util_printf("pktstagif:0x%x ", flow_table_cfg.pktstagif);
                diag_util_printf("pktspri:0x%x ", flow_table_cfg.pktspri);
                diag_util_printf("pktsvid:0x%x ", flow_table_cfg.pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("care_slp:0x%x ", flow_table_cfg.care_slp);
                diag_util_printf("care_dmac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.care_dmac[0], flow_table_cfg.care_dmac[1], flow_table_cfg.care_dmac[2], flow_table_cfg.care_dmac[3], flow_table_cfg.care_dmac[4], flow_table_cfg.care_dmac[5]);
                diag_util_printf("care_smac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.care_smac[0], flow_table_cfg.care_smac[1], flow_table_cfg.care_smac[2], flow_table_cfg.care_smac[3], flow_table_cfg.care_smac[4], flow_table_cfg.care_smac[5]);
                diag_util_printf("care_frametype:0x%x ", flow_table_cfg.care_frametype);
                diag_util_printf("care_ethertype:0x%x ", flow_table_cfg.care_ethertype);
                diag_util_printf("care_pktctagif:0x%x ", flow_table_cfg.care_pktctagif);
                diag_util_printf("care_pktcpri:0x%x ", flow_table_cfg.care_pktcpri);                
                diag_util_printf("care_pktcvid:0x%x ", flow_table_cfg.care_pktcvid);
                diag_util_printf("care_ipv4:0x%x ", flow_table_cfg.care_ipv4);
                diag_util_printf("care_ipv6:0x%x ", flow_table_cfg.care_ipv6);
                diag_util_printf("care_ipv6mld:0x%x ", flow_table_cfg.care_ipv6mld);
                diag_util_printf("care_sip:0x%x ", flow_table_cfg.care_sip);
                diag_util_printf("care_dip:0x%x ", flow_table_cfg.care_dip);
                diag_util_printf("care_flowlabel:0x%x ", flow_table_cfg.care_flowlabel);
                diag_util_printf("care_ipproto:0x%x ", flow_table_cfg.care_ipproto);
                diag_util_printf("care_tos:0x%x ", flow_table_cfg.care_tos);
                diag_util_printf("care_srcport:0x%x ", flow_table_cfg.care_srcport);
                diag_util_printf("care_dstport:0x%x ", flow_table_cfg.care_dstport);
                diag_util_printf("care_pktstagif:0x%x ", flow_table_cfg.care_pktstagif);
                diag_util_printf("care_pktspri:0x%x ", flow_table_cfg.care_pktspri);
                diag_util_printf("care_pktsvid:0x%x ", flow_table_cfg.care_pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("action: ");
                if (FLOW_ACTION_LOG == action.actGroup)
                {
                    diag_util_printf("log ");
                    diag_util_printf("log-id:%d ", action.un.log.logindex);
                }
                else if (FLOW_ACTION_ASSIGNVLAN == action.actGroup)
                {
                    diag_util_printf("assign-vlan ");
                    diag_util_printf("usepktctag:%d ", action.un.assignVlan.usepktctag);
                    diag_util_printf("usepktspri:%d ", action.un.assignVlan.usepktspri);
                    diag_util_printf("actvid:%d ", action.un.assignVlan.actvid);
                    diag_util_printf("actpri:%d ", action.un.assignVlan.actpri);
                }
                else
                {
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }    
                diag_util_mprintf("\n");    
            }    
        } /* end of for (index = start_idx; index <= end_idx; index++) */
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_DUMP_ACL_FLOW_TABLE_RULE_START_INDEX_END_INDEX
/*
 * filter dump ( acl | flow-table ) rule <UINT:start_index> { <UINT:end_index> }
 */
cparser_result_t cparser_cmd_filter_dump_acl_flow_table_rule_start_index_end_index(cparser_context_t *context,
    uint32_t *start_index_ptr, uint32_t *end_index_ptr)
{
    uint32               unit = 0;
    uint32               start_idx = 0;
    uint32               end_idx = 0;
    uint32               index = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_aclCfg_t  acl_cfg;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    start_idx = *start_index_ptr;
    if ('a' == TOKEN_CHAR(2,0))
    {
        if (TOKEN_NUM < 6)
        {
            end_idx = start_idx;
        }
        else
        {    
            end_idx = *end_index_ptr;
        } 
            
        for (index = start_idx; index <= end_idx; index++)
        {
            memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
            memset(&action, 0, sizeof(rtk_filter_action_t));
            DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_get(unit, index, &acl_cfg, &action), ret);
            if (FALSE == acl_cfg.valid)
            {
                diag_util_mprintf("ACL rule %d is invalid\n", index);
            }
            else
            {
                diag_util_mprintf("ACL rule %d\n", index);
                diag_util_printf("slp:0x%x ", acl_cfg.slp);
                diag_util_printf("dmac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.dmac[0], acl_cfg.dmac[1], acl_cfg.dmac[2], acl_cfg.dmac[3], acl_cfg.dmac[4], acl_cfg.dmac[5]);
                diag_util_printf("smac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.smac[0], acl_cfg.smac[1], acl_cfg.smac[2], acl_cfg.smac[3], acl_cfg.smac[4], acl_cfg.smac[5]);
                diag_util_printf("frametype:0x%x ", acl_cfg.frametype);
                diag_util_printf("ethertype:0x%x ", acl_cfg.ethertype);
                diag_util_printf("rvid:0x%x ", acl_cfg.rvid);
                diag_util_printf("ipv4:0x%x ", acl_cfg.ipv4);
                diag_util_printf("ipv6:0x%x ", acl_cfg.ipv6);
                diag_util_printf("ipv6mld:0x%x ", acl_cfg.ipv6mld);
                diag_util_printf("sip:0x%x ", acl_cfg.sip);
                diag_util_printf("dip:0x%x ", acl_cfg.dip);
                diag_util_printf("flowlabel:0x%x ", acl_cfg.flowlabel);
                diag_util_printf("ipproto:0x%x ", acl_cfg.ipproto);
                diag_util_printf("tos:0x%x ", acl_cfg.tos);
                diag_util_printf("srcport:0x%x ", acl_cfg.srcport);
                diag_util_printf("dstport:0x%x ", acl_cfg.dstport);
                diag_util_printf("tcpflag:0x%x ", acl_cfg.tcpflag);
                diag_util_printf("patternmatch:0x%x ", acl_cfg.patternmatch);
                diag_util_printf("pktsvid:0x%x ", acl_cfg.pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("care_slp:0x%x ", acl_cfg.care_slp);
                diag_util_printf("care_dmac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.care_dmac[0], acl_cfg.care_dmac[1], acl_cfg.care_dmac[2], acl_cfg.care_dmac[3], acl_cfg.care_dmac[4], acl_cfg.care_dmac[5]);
                diag_util_printf("care_smac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.care_smac[0], acl_cfg.care_smac[1], acl_cfg.care_smac[2], acl_cfg.care_smac[3], acl_cfg.care_smac[4], acl_cfg.care_smac[5]);
                diag_util_printf("care_frametype:0x%x ", acl_cfg.care_frametype);
                diag_util_printf("care_ethertype:0x%x ", acl_cfg.care_ethertype);
                diag_util_printf("care_rvid:0x%x ", acl_cfg.care_rvid);
                diag_util_printf("care_ipv4:0x%x ", acl_cfg.care_ipv4);
                diag_util_printf("care_ipv6:0x%x ", acl_cfg.care_ipv6);
                diag_util_printf("care_ipv6mld:0x%x ", acl_cfg.care_ipv6mld);
                diag_util_printf("care_sip:0x%x ", acl_cfg.care_sip);
                diag_util_printf("care_dip:0x%x ", acl_cfg.care_dip);
                diag_util_printf("care_flowlabel:0x%x ", acl_cfg.care_flowlabel);
                diag_util_printf("care_ipproto:0x%x ", acl_cfg.care_ipproto);
                diag_util_printf("care_tos:0x%x ", acl_cfg.care_tos);
                diag_util_printf("care_srcport:0x%x ", acl_cfg.care_srcport);
                diag_util_printf("care_dstport:0x%x ", acl_cfg.care_dstport);
                diag_util_printf("care_tcpflag:0x%x ", acl_cfg.care_tcpflag);
                diag_util_printf("care_patternmatch:0x%x ", acl_cfg.care_patternmatch);
                diag_util_printf("care_pktsvid:0x%x ", acl_cfg.care_pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("action: ");
                switch(action.actGroup)
                {
                    case ACL_ACTION_DIRECTION:
                        switch(action.un.permit_drop_redirect.acttype)
                        {
                            case ACL_ACTION_DIRECTION_PERMIT:
                                diag_util_printf("permit ");
                                break;
                            case ACL_ACTION_DIRECTION_DROP:
                                diag_util_printf("drop ");
                                break;
                            case ACL_ACTION_DIRECTION_REDIRECT:
                                diag_util_printf("redirect port:%d ", action.un.permit_drop_redirect.portid);
                                break;
                            case ACL_ACTION_DIRECTION_COPYTOCPUY:
                                diag_util_printf("copy-to-cpu ");
                                break;
                            default:
                                break;
                        }
                        break;            
                        
                    case ACL_ACTION_MIRROR:
                        diag_util_printf("mirror ");
                        diag_util_printf("mirror-id:%d ", action.un.mirror.mirrorsetid);
                        break;
                        
                    case ACL_ACTION_LOG:
                        diag_util_printf("log ");
                        diag_util_printf("log-id:%d ", action.un.log.logindex);
                        break;
                
                    case ACL_ACTION_RATELIMIT:
                        diag_util_printf("rate-limit ");
                        diag_util_printf("rate-id:%d ", action.un.ratelimit.rateindex);
                        break;
                
                    case ACL_ACTION_NEWSVID:
                        diag_util_printf("new-svid ");
                        diag_util_printf("svid:%d ", action.un.newsvid.svid);
                        break;
                
                    case ACL_ACTION_NEWCVID:
                        diag_util_printf("new-cvid ");
                        diag_util_printf("replace-cvid:%d ", action.un.newcvid.replacecvid);
                        diag_util_printf("replace-cpri:%d ", action.un.newcvid.replacecpri);
                        diag_util_printf("cvid:%d ", action.un.newcvid.cvid);
                        diag_util_printf("cpri:%d ", action.un.newcvid.cpri);
                        break;
                        
                    default:
                        diag_util_printf("User config: Error!\n");
                        return CPARSER_NOT_OK;
                        break;
                }
                diag_util_mprintf("\n");
            }    
        } /* end of for (index = start_idx; index <= end_idx; index++) */
    }    
    else if (TOKEN_CHAR(2,0) == 'f')
    {
        if (TOKEN_NUM < 6)
        {
            end_idx = start_idx;
        }
        else
        {    
            end_idx = *end_index_ptr;
        }  
        
        for (index = start_idx; index <= end_idx; index++)
        {
            memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
            memset(&action, 0, sizeof(rtk_filter_action_t));
            DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_get(unit, index, &flow_table_cfg, &action), ret);
            if (FALSE == flow_table_cfg.valid)
            {
                diag_util_mprintf("Flow table rule %d is invalid\n", index);
            }
            else
            {
                diag_util_mprintf("Flow table rule %d\n", index);
                diag_util_printf("slp:0x%x ", flow_table_cfg.slp);
                diag_util_printf("dmac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.dmac[0], flow_table_cfg.dmac[1], flow_table_cfg.dmac[2], flow_table_cfg.dmac[3], flow_table_cfg.dmac[4], flow_table_cfg.dmac[5]);
                diag_util_printf("smac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.smac[0], flow_table_cfg.smac[1], flow_table_cfg.smac[2], flow_table_cfg.smac[3], flow_table_cfg.smac[4], flow_table_cfg.smac[5]);
                diag_util_printf("frametype:0x%x ", flow_table_cfg.frametype);
                diag_util_printf("ethertype:0x%x ", flow_table_cfg.ethertype);
                diag_util_printf("pktctagif:0x%x ", flow_table_cfg.pktctagif);
                diag_util_printf("pktcpri:0x%x ", flow_table_cfg.pktcpri);
                diag_util_printf("pktcvid:0x%x ", flow_table_cfg.pktcvid);
                diag_util_printf("ipv4:0x%x ", flow_table_cfg.ipv4);
                diag_util_printf("ipv6:0x%x ", flow_table_cfg.ipv6);
                diag_util_printf("ipv6mld:0x%x ", flow_table_cfg.ipv6mld);
                diag_util_printf("sip:0x%x ", flow_table_cfg.sip);
                diag_util_printf("dip:0x%x ", flow_table_cfg.dip);
                diag_util_printf("flowlabel:0x%x ", flow_table_cfg.flowlabel);
                diag_util_printf("ipproto:0x%x ", flow_table_cfg.ipproto);
                diag_util_printf("tos:0x%x ", flow_table_cfg.tos);
                diag_util_printf("srcport:0x%x ", flow_table_cfg.srcport);
                diag_util_printf("dstport:0x%x ", flow_table_cfg.dstport);
                diag_util_printf("pktstagif:0x%x ", flow_table_cfg.pktstagif);
                diag_util_printf("pktspri:0x%x ", flow_table_cfg.pktspri);
                diag_util_printf("pktsvid:0x%x ", flow_table_cfg.pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("care_slp:0x%x ", flow_table_cfg.care_slp);
                diag_util_printf("care_dmac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.care_dmac[0], flow_table_cfg.care_dmac[1], flow_table_cfg.care_dmac[2], flow_table_cfg.care_dmac[3], flow_table_cfg.care_dmac[4], flow_table_cfg.care_dmac[5]);
                diag_util_printf("care_smac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.care_smac[0], flow_table_cfg.care_smac[1], flow_table_cfg.care_smac[2], flow_table_cfg.care_smac[3], flow_table_cfg.care_smac[4], flow_table_cfg.care_smac[5]);
                diag_util_printf("care_frametype:0x%x ", flow_table_cfg.care_frametype);
                diag_util_printf("care_ethertype:0x%x ", flow_table_cfg.care_ethertype);
                diag_util_printf("care_pktctagif:0x%x ", flow_table_cfg.care_pktctagif);
                diag_util_printf("care_pktcpri:0x%x ", flow_table_cfg.care_pktcpri);                
                diag_util_printf("care_pktcvid:0x%x ", flow_table_cfg.care_pktcvid);
                diag_util_printf("care_ipv4:0x%x ", flow_table_cfg.care_ipv4);
                diag_util_printf("care_ipv6:0x%x ", flow_table_cfg.care_ipv6);
                diag_util_printf("care_ipv6mld:0x%x ", flow_table_cfg.care_ipv6mld);
                diag_util_printf("care_sip:0x%x ", flow_table_cfg.care_sip);
                diag_util_printf("care_dip:0x%x ", flow_table_cfg.care_dip);
                diag_util_printf("care_flowlabel:0x%x ", flow_table_cfg.care_flowlabel);
                diag_util_printf("care_ipproto:0x%x ", flow_table_cfg.care_ipproto);
                diag_util_printf("care_tos:0x%x ", flow_table_cfg.care_tos);
                diag_util_printf("care_srcport:0x%x ", flow_table_cfg.care_srcport);
                diag_util_printf("care_dstport:0x%x ", flow_table_cfg.care_dstport);
                diag_util_printf("care_pktstagif:0x%x ", flow_table_cfg.care_pktstagif);
                diag_util_printf("care_pktspri:0x%x ", flow_table_cfg.care_pktspri);
                diag_util_printf("care_pktsvid:0x%x ", flow_table_cfg.care_pktsvid);
                diag_util_mprintf("\n");
                diag_util_printf("action: ");
                if (FLOW_ACTION_LOG == action.actGroup)
                {
                    diag_util_printf("log ");
                    diag_util_printf("log-id:%d ", action.un.log.logindex);
                }
                else if (FLOW_ACTION_ASSIGNVLAN == action.actGroup)
                {
                    diag_util_printf("assign-vlan ");
                    diag_util_printf("usepktctag:%d ", action.un.assignVlan.usepktctag);
                    diag_util_printf("usepktspri:%d ", action.un.assignVlan.usepktspri);
                    diag_util_printf("actvid:%d ", action.un.assignVlan.actvid);
                    diag_util_printf("actpri:%d ", action.un.assignVlan.actpri);
                }
                else
                {
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }    
                diag_util_mprintf("\n");    
            }    
        } /* end of for (index = start_idx; index <= end_idx; index++) */
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_DUMP_LOG_COUNTER_INDEX
/*
 * filter dump log-counter { <UINT:index> }
 */
cparser_result_t cparser_cmd_filter_dump_log_counter_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint64  byte_counter = 0;
    uint32  packet_counter = 0;
    uint32  unit = 0;
    uint32  log_index = 0;
    int32   ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (TOKEN_NUM < 4)
    {
        for (log_index = 0; log_index < PIE_MAX_LOG_INDEX; log_index++)
        {
            DIAG_UTIL_ERR_CHK(rtk_filter_stat_get(unit, log_index, &packet_counter, &byte_counter), ret);
            diag_util_mprintf("Index:%d    pktcnt=%lu   bytecnt=%llu \n", log_index, packet_counter, byte_counter);
        }    
    }
    else
    {
        log_index = *index_ptr;
        DIAG_UTIL_ERR_CHK(rtk_filter_stat_get(unit, log_index, &packet_counter, &byte_counter), ret);
        diag_util_mprintf("Index:%d    pktcnt=%lu   bytecnt=%llu \n", log_index, packet_counter, byte_counter);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_DUMP_RATE_LIMIT_INDEX
/*
 * filter dump rate-limit { <UINT:index> }
 */
cparser_result_t cparser_cmd_filter_dump_rate_limit_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    uint32  rate = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (TOKEN_NUM < 4)
    {
        for (index = 0; index < PIE_MAX_RATELIMIT_INDEX; index++)
        {
            DIAG_UTIL_ERR_CHK(rtk_filter_igrAclRateLimit_get(unit,  index, &rate), ret);
            diag_util_mprintf("Index:%d    rate=0x%x\n", index, rate);
        }    
    }
    else
    {
        index = *index_ptr;
        DIAG_UTIL_ERR_CHK(rtk_filter_igrAclRateLimit_get(unit,  index, &rate), ret);
        diag_util_mprintf("Index:%d    rate=0x%x\n", index, rate);
    }    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_DUMP
/*
 * filter dump
 */
cparser_result_t cparser_cmd_filter_dump(cparser_context_t *context)
{
    uint32       unit = 0;
    uint32       cutline = 0;
    int32        ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_filter_blkCutline_get(unit, &cutline), ret);
    diag_util_mprintf(" Cutline: %d\n", cutline);

    DIAG_UTIL_ERR_CHK(rtk_filter_pieEnable_get(unit, &enable), ret);
    if (ENABLED == enable)
    {
        diag_util_mprintf(" PIE: enable\n");
    } 
    else if (DISABLED == enable)
    {
        diag_util_mprintf(" PIE: disable\n");
    }     
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_FILTER_DEL_ACL_FLOW_TABLE_RULE_START_INDEX_END_INDEX
/*
 * filter del ( acl | flow-table ) rule <UINT:start_index> { <UINT:end_index> }
 */
cparser_result_t cparser_cmd_filter_del_acl_flow_table_rule_start_index_end_index(cparser_context_t *context,
    uint32_t *start_index_ptr, uint32_t *end_index_ptr)
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_filter_id_t start_idx = 0;
    rtk_filter_id_t end_idx = 0;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    
    start_idx = *start_index_ptr;
    if ('a' == TOKEN_CHAR(2,0))
    {
        if (TOKEN_NUM < 6)
        {
            DIAG_UTIL_ERR_CHK(rtk_filter_igrAcl_del(unit, start_idx), ret);
        }
        else
        {    
            end_idx = *end_index_ptr;
            for (index = start_idx; index <= end_idx; index++)
            {
                if ((ret = rtk_filter_igrAcl_del(unit, index)) != RT_ERR_OK)
                {
                    /*Don't check error, continue to del next entry*/
                }
            }    
        }    
    }    
    else if ('f' == TOKEN_CHAR(2,0))
    {
        if (TOKEN_NUM < 6)
        {
            DIAG_UTIL_ERR_CHK(rtk_filter_flowTbl_del(unit, start_idx), ret);
        }
        else
        {    
            end_idx = *end_index_ptr;
            for (index = start_idx; index <= end_idx; index++)
            {
                if ((ret = rtk_filter_flowTbl_del(unit, index)) != RT_ERR_OK)
                {
                    /*Don't check error, continue to del next entry*/
                }
            }   
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

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/rt_error.h>
#include <rtk/filter.h>
#include <app/diag_shell/command.h>
#include <app/diag_shell/cli_util.h>
#include <app/diag_shell/more.h>
#include <app/diag_shell/diag_shell_util.h>
#include <app/diag_shell/diag_shell_pie.h>

#define     ACL_ACTION_DIRECTION                (0)
#define     ACL_ACTION_MIRROR                   (1)
#define     ACL_ACTION_LOG                      (2)
#define     ACL_ACTION_RATELIMIT                (4)
#define     ACL_ACTION_NEWSVID                  (5)
#define     ACL_ACTION_NEWCVID                  (6)

#define     FLOW_ACTION_LOG                     (0)
#define     FLOW_ACTION_ASSIGNVLAN              (4)

#define     ACL_ACTION_DIRECTION_PERMIT         (0)
#define     ACL_ACTION_DIRECTION_DROP           (1)
#define     ACL_ACTION_DIRECTION_REDIRECT       (2)
#define     ACL_ACTION_DIRECTION_COPYTOCPUY     (3)

#define     PIE_MAX_LOG_INDEX                   (128)
#define     PIE_MAX_RATELIMIT_INDEX             (128)

#define     PIE_BIT_ON                          (1)
#define     PIE_BIT_OFF                         (0)

static int32 _parse_acl_rule(char *argv, uint32 rule_content, uint32 rule_carebit, rtk_filter_aclCfg_t *acl_cfg);
static int32 _parse_flowTable_rule(char *argv, uint32 rule_content, uint32 rule_carebit, rtk_filter_flowTbl_t *flow_table_cfg);

DEFUN (
    diag_shell_pie_show,
    cmd_diag_shell_pie_show,
    "pie show",
    "PIE configuration\n"
    "show configuration\n"
    )
{
    uint32       unit = 0;
    uint32       cutline = 0;
    int32        ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    if ((ret = rtk_filter_blkCutline_get(unit, &cutline)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        mprintf_end();
        return CMD_WARNING;
    } 
    mprintf(" Cutline: %d\n", cutline);

    if ((ret = rtk_filter_pieEnable_get(unit, &enable)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        mprintf_end();
        return CMD_WARNING;
    } 
    if (ENABLED == enable)
    {
        mprintf(" PIE: enable\n");
    } 
    else if (DISABLED == enable)
    {
        mprintf(" PIE: disable\n");
    }     
    else
    {
        printf("User config: Error!\n");
        mprintf_end();
        return CMD_ERR_NO_MATCH;
    }

    mprintf_end();
    return CMD_SUCCESS;
} /* end of diag_shell_pie_show */

DEFUN (
    diag_shell_pie_set_cutLine,
    cmd_diag_shell_pie_set_cutLine,
    "pie set cut-line <0-8>",
    "PIE configuration\n"
    "set configuration\n"
    "cut line\n"
    "<0-8>\n"
    )
{
    uint32    unit = 0;
    uint32    cutline = 0;
    int32     ret = RT_ERR_FAILED;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    cutline = atoi(argv[0]);
    if ((ret = rtk_filter_blkCutline_set(unit, cutline)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    } 
            
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_cutLine */

DEFUN (
    diag_shell_pie_enable,
    cmd_diag_shell_pie_enable,
    "pie (enable|disable)",
    "PIE configuration\n"
    "enable function\n"
    "disable function\n"
    )
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    if ('e' == argv[0][0])
    {
        if ((ret = rtk_filter_pieEnable_set(unit, ENABLED)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }    
    else if ('d' == argv[0][0])
    {
        if ((ret = rtk_filter_pieEnable_set(unit, DISABLED)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_enable */

DEFUN (
    diag_shell_pie_dump_rule,
    cmd_diag_shell_pie_dump_rule,
    "pie dump (acl|flow-table) rule <0-511> [<0-511>]",
    "PIE configuration\n"
    "dump configuration\n"
    "ACL configuration\n"
    "flow table configuration\n"
    "rule index\n"
    "start index\n"
    "end index\n"
    )
{
    uint32               unit = 0;
    uint32               start_idx = 0;
    uint32               end_idx = 0;
    uint32               index = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_aclCfg_t  acl_cfg;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    start_idx = atoi(argv[1]);
    if ('a' == argv[0][0])
    {
        if (argc < 3)
        {
            end_idx = start_idx;
        }
        else
        {    
            end_idx = atoi(argv[2]);
        } 
            
        for (index = start_idx; index <= end_idx; index++)
        {
            memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
            memset(&action, 0, sizeof(rtk_filter_action_t));
            if ((ret = rtk_filter_igrAcl_get(unit, index, &acl_cfg, &action)) != RT_ERR_OK)
            {
                DIAG_SHELL_ERR_PRINT(ret);
                mprintf_end();
                return CMD_WARNING;
            }
            if (FALSE == acl_cfg.valid)
            {
                mprintf("ACL rule %d is invalid\n", index);
            }
            else
            {
                mprintf("ACL rule %d\n", index);
                printf("slp:0x%x ", acl_cfg.slp);
                printf("dmac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.dmac[0], acl_cfg.dmac[1], acl_cfg.dmac[2], acl_cfg.dmac[3], acl_cfg.dmac[4], acl_cfg.dmac[5]);
                printf("smac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.smac[0], acl_cfg.smac[1], acl_cfg.smac[2], acl_cfg.smac[3], acl_cfg.smac[4], acl_cfg.smac[5]);
                printf("frametype:0x%x ", acl_cfg.frametype);
                printf("ethertype:0x%x ", acl_cfg.ethertype);
                printf("rvid:0x%x ", acl_cfg.rvid);
                printf("ipv4:0x%x ", acl_cfg.ipv4);
                printf("ipv6:0x%x ", acl_cfg.ipv6);
                printf("ipv6mld:0x%x ", acl_cfg.ipv6mld);
                printf("sip:0x%x ", acl_cfg.sip);
                printf("dip:0x%x ", acl_cfg.dip);
                printf("flowlabel:0x%x ", acl_cfg.flowlabel);
                printf("ipproto:0x%x ", acl_cfg.ipproto);
                printf("tos:0x%x ", acl_cfg.tos);
                printf("srcport:0x%x ", acl_cfg.srcport);
                printf("dstport:0x%x ", acl_cfg.dstport);
                printf("tcpflag:0x%x ", acl_cfg.tcpflag);
                printf("patternmatch:0x%x ", acl_cfg.patternmatch);
                printf("pktsvid:0x%x ", acl_cfg.pktsvid);
                mprintf("\n");
                printf("care_slp:0x%x ", acl_cfg.care_slp);
                printf("care_dmac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.care_dmac[0], acl_cfg.care_dmac[1], acl_cfg.care_dmac[2], acl_cfg.care_dmac[3], acl_cfg.care_dmac[4], acl_cfg.care_dmac[5]);
                printf("care_smac:%02x:%02x:%02x:%02x:%02x:%02x ", acl_cfg.care_smac[0], acl_cfg.care_smac[1], acl_cfg.care_smac[2], acl_cfg.care_smac[3], acl_cfg.care_smac[4], acl_cfg.care_smac[5]);
                printf("care_frametype:0x%x ", acl_cfg.care_frametype);
                printf("care_ethertype:0x%x ", acl_cfg.care_ethertype);
                printf("care_rvid:0x%x ", acl_cfg.care_rvid);
                printf("care_ipv4:0x%x ", acl_cfg.care_ipv4);
                printf("care_ipv6:0x%x ", acl_cfg.care_ipv6);
                printf("care_ipv6mld:0x%x ", acl_cfg.care_ipv6mld);
                printf("care_sip:0x%x ", acl_cfg.care_sip);
                printf("care_dip:0x%x ", acl_cfg.care_dip);
                printf("care_flowlabel:0x%x ", acl_cfg.care_flowlabel);
                printf("care_ipproto:0x%x ", acl_cfg.care_ipproto);
                printf("care_tos:0x%x ", acl_cfg.care_tos);
                printf("care_srcport:0x%x ", acl_cfg.care_srcport);
                printf("care_dstport:0x%x ", acl_cfg.care_dstport);
                printf("care_tcpflag:0x%x ", acl_cfg.care_tcpflag);
                printf("care_patternmatch:0x%x ", acl_cfg.care_patternmatch);
                printf("care_pktsvid:0x%x ", acl_cfg.care_pktsvid);
                mprintf("\n");
                printf("action: ");
                switch(action.actGroup)
                {
                    case ACL_ACTION_DIRECTION:
                        switch(action.un.permit_drop_redirect.acttype)
                        {
                            case ACL_ACTION_DIRECTION_PERMIT:
                                printf("permit ");
                                break;
                            case ACL_ACTION_DIRECTION_DROP:
                                printf("drop ");
                                break;
                            case ACL_ACTION_DIRECTION_REDIRECT:
                                printf("redirect port:%d ", action.un.permit_drop_redirect.portid);
                                break;
                            case ACL_ACTION_DIRECTION_COPYTOCPUY:
                                printf("copy-to-cpu ");
                                break;
                            default:
                                break;
                        }
                        break;            
                        
                    case ACL_ACTION_MIRROR:
                        printf("mirror ");
                        printf("mirror-id:%d ", action.un.mirror.mirrorsetid);
                        break;
                        
                    case ACL_ACTION_LOG:
                        printf("log ");
                        printf("log-id:%d ", action.un.log.logindex);
                        break;
                
                    case ACL_ACTION_RATELIMIT:
                        printf("rate-limit ");
                        printf("rate-id:%d ", action.un.ratelimit.rateindex);
                        break;
                
                    case ACL_ACTION_NEWSVID:
                        printf("new-svid ");
                        printf("svid:%d ", action.un.newsvid.svid);
                        break;
                
                    case ACL_ACTION_NEWCVID:
                        printf("new-cvid ");
                        printf("replace-cvid:%d ", action.un.newcvid.replacecvid);
                        printf("replace-cpri:%d ", action.un.newcvid.replacecpri);
                        printf("cvid:%d ", action.un.newcvid.cvid);
                        printf("cpri:%d ", action.un.newcvid.cpri);
                        break;
                        
                    default:
                        printf("User config: Error!\n");
                        mprintf_end();
                        return CMD_ERR_NO_MATCH;
                        break;
                }
                mprintf("\n");
            }    
        } /* end of for (index = start_idx; index <= end_idx; index++) */
    }    
    else if (argv[0][0] == 'f')
    {
        if (argc < 3)
        {
            end_idx = start_idx;
        }
        else
        {    
            end_idx = atoi(argv[2]);
        }  
        
        for (index = start_idx; index <= end_idx; index++)
        {
            memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
            memset(&action, 0, sizeof(rtk_filter_action_t));
            if ((ret = rtk_filter_flowTbl_get(unit, index, &flow_table_cfg, &action)) != RT_ERR_OK)
            {
                DIAG_SHELL_ERR_PRINT(ret);
                mprintf_end();
                return CMD_WARNING;
            }
            if (FALSE == flow_table_cfg.valid)
            {
                mprintf("Flow table rule %d is invalid\n", index);
            }
            else
            {
                mprintf("Flow table rule %d\n", index);
                printf("slp:0x%x ", flow_table_cfg.slp);
                printf("dmac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.dmac[0], flow_table_cfg.dmac[1], flow_table_cfg.dmac[2], flow_table_cfg.dmac[3], flow_table_cfg.dmac[4], flow_table_cfg.dmac[5]);
                printf("smac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.smac[0], flow_table_cfg.smac[1], flow_table_cfg.smac[2], flow_table_cfg.smac[3], flow_table_cfg.smac[4], flow_table_cfg.smac[5]);
                printf("frametype:0x%x ", flow_table_cfg.frametype);
                printf("ethertype:0x%x ", flow_table_cfg.ethertype);
                printf("pktctagif:0x%x ", flow_table_cfg.pktctagif);
                printf("pktcpri:0x%x ", flow_table_cfg.pktcpri);
                printf("pktcvid:0x%x ", flow_table_cfg.pktcvid);
                printf("ipv4:0x%x ", flow_table_cfg.ipv4);
                printf("ipv6:0x%x ", flow_table_cfg.ipv6);
                printf("ipv6mld:0x%x ", flow_table_cfg.ipv6mld);
                printf("sip:0x%x ", flow_table_cfg.sip);
                printf("dip:0x%x ", flow_table_cfg.dip);
                printf("flowlabel:0x%x ", flow_table_cfg.flowlabel);
                printf("ipproto:0x%x ", flow_table_cfg.ipproto);
                printf("tos:0x%x ", flow_table_cfg.tos);
                printf("srcport:0x%x ", flow_table_cfg.srcport);
                printf("dstport:0x%x ", flow_table_cfg.dstport);
                printf("pktstagif:0x%x ", flow_table_cfg.pktstagif);
                printf("pktspri:0x%x ", flow_table_cfg.pktspri);
                printf("pktsvid:0x%x ", flow_table_cfg.pktsvid);
                mprintf("\n");
                printf("care_slp:0x%x ", flow_table_cfg.care_slp);
                printf("care_dmac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.care_dmac[0], flow_table_cfg.care_dmac[1], flow_table_cfg.care_dmac[2], flow_table_cfg.care_dmac[3], flow_table_cfg.care_dmac[4], flow_table_cfg.care_dmac[5]);
                printf("care_smac:%02x:%02x:%02x:%02x:%02x:%02x ", flow_table_cfg.care_smac[0], flow_table_cfg.care_smac[1], flow_table_cfg.care_smac[2], flow_table_cfg.care_smac[3], flow_table_cfg.care_smac[4], flow_table_cfg.care_smac[5]);
                printf("care_frametype:0x%x ", flow_table_cfg.care_frametype);
                printf("care_ethertype:0x%x ", flow_table_cfg.care_ethertype);
                printf("care_pktctagif:0x%x ", flow_table_cfg.care_pktctagif);
                printf("care_pktcpri:0x%x ", flow_table_cfg.care_pktcpri);                
                printf("care_pktcvid:0x%x ", flow_table_cfg.care_pktcvid);
                printf("care_ipv4:0x%x ", flow_table_cfg.care_ipv4);
                printf("care_ipv6:0x%x ", flow_table_cfg.care_ipv6);
                printf("care_ipv6mld:0x%x ", flow_table_cfg.care_ipv6mld);
                printf("care_sip:0x%x ", flow_table_cfg.care_sip);
                printf("care_dip:0x%x ", flow_table_cfg.care_dip);
                printf("care_flowlabel:0x%x ", flow_table_cfg.care_flowlabel);
                printf("care_ipproto:0x%x ", flow_table_cfg.care_ipproto);
                printf("care_tos:0x%x ", flow_table_cfg.care_tos);
                printf("care_srcport:0x%x ", flow_table_cfg.care_srcport);
                printf("care_dstport:0x%x ", flow_table_cfg.care_dstport);
                printf("care_pktstagif:0x%x ", flow_table_cfg.care_pktstagif);
                printf("care_pktspri:0x%x ", flow_table_cfg.care_pktspri);
                printf("care_pktsvid:0x%x ", flow_table_cfg.care_pktsvid);
                mprintf("\n");
                printf("action: ");
                if (FLOW_ACTION_LOG == action.actGroup)
                {
                    printf("log ");
                    printf("log-id:%d ", action.un.log.logindex);
                }
                else if (FLOW_ACTION_ASSIGNVLAN == action.actGroup)
                {
                    printf("assign-vlan ");
                    printf("usepktctag:%d ", action.un.assignVlan.usepktctag);
                    printf("usepktspri:%d ", action.un.assignVlan.usepktspri);
                    printf("actvid:%d ", action.un.assignVlan.actvid);
                    printf("actpri:%d ", action.un.assignVlan.actpri);
                }
                else
                {
                    printf("User config: Error!\n");
                    mprintf_end();
                    return CMD_ERR_NO_MATCH;
                }    
                mprintf("\n");    
            }    
        } /* end of for (index = start_idx; index <= end_idx; index++) */
    }
    else
    {
        printf("User config: Error!\n");
        mprintf_end();
        return CMD_ERR_NO_MATCH;
    }
    
    mprintf_end();
    return CMD_SUCCESS;
} /* end of diag_shell_pie_dump_rule */

DEFUN (
    diag_shell_pie_dump_logCounter,
    cmd_diag_shell_pie_dump_logCounter,
    "pie dump log-counter [<0-127>]",
    "PIE configuration\n"
    "dump configuration\n"
    "log counter\n"
    "index <0-127>\n"
    )
{
    uint64  byte_counter = 0;
    uint32  packet_counter = 0;
    uint32  unit = 0;
    uint32  log_index = 0;
    int32   ret = RT_ERR_FAILED;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    if (argc < 1)
    {
        for (log_index = 0; log_index < PIE_MAX_LOG_INDEX; log_index++)
        {
            if ((ret = rtk_filter_stat_get(unit, log_index, &packet_counter, &byte_counter)) != RT_ERR_OK)
            {
                DIAG_SHELL_ERR_PRINT(ret);
                mprintf_end();
                return CMD_WARNING;
            }
            mprintf("Index:%d    pktcnt=%lu   bytecnt=%llu \n", log_index, packet_counter, byte_counter);
        }    
    }
    else
    {
        log_index = atoi(argv[0]);
        if ((ret = rtk_filter_stat_get(unit, log_index, &packet_counter, &byte_counter)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            mprintf_end();
            return CMD_WARNING;
        }
        mprintf("Index:%d    pktcnt=%lu   bytecnt=%llu \n", log_index, packet_counter, byte_counter);
    }    
    
    mprintf_end();
    return CMD_SUCCESS;
} /* end of diag_shell_pie_dump_logCounter */

DEFUN (
    diag_shell_pie_set_logCounter,
    cmd_diag_shell_pie_clear_logCounter,
    "pie set log-counter <0-127> PKTCNT BYTECNT",
    "PIE configuration\n"
    "Set configuration\n"
    "log counter\n"
    "index <0-127>\n"
    "packet counter\n"
    "byte counter\n"
    )
{
    uint64   byte_counter = 0;
    uint32   packet_counter = 0;
    uint32   unit = 0;
    int32    log_index = 0;
    int32    ret = RT_ERR_FAILED;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    log_index = atoi(argv[0]);

    CLI_CHK_INUPT_STR_NUMBER(packet_counter, argv[1], "packet counter input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(byte_counter, argv[2], "byte counter input error", uint64);

    if ((ret = rtk_filter_stat_set(unit, log_index, packet_counter, byte_counter)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_logCounter */

DEFUN (
    diag_shell_pie_set_aclRule_direction1,
    cmd_diag_shell_pie_set_aclRule_direction1,
    "pie set acl rule <0-511> (slp|frame-type|ether-type|rvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|tcp-flag|pattern-match|pkt-svid) CONTENT CAREBITS action (permit|drop|copy-to-cpu)",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "relay vid\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "TCP flag field\n"
    "pattern match result\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "permit\n"
    "drop\n"
    "copy to cpu\n"
    )
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        }    
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);
        
    if (_parse_acl_rule(argv[1], rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    } 
    
    if ('p' == argv[4][0]) /* permit */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_PERMIT;
    }
    else if ('d' == argv[4][0]) /* drop */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_DROP;
    }
    else if ('c' == argv[4][0]) /* copy-to-cpy */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_COPYTOCPUY;
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    } 
       
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_direction1 */

DEFUN (
    diag_shell_pie_set_aclRule_direction2,
    cmd_diag_shell_pie_set_aclRule_direction2,
    "pie set acl rule <0-511> (slp|frame-type|ether-type|rvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|tcp-flag|pattern-match|pkt-svid) CONTENT CAREBITS action redirect <0-28>",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"   
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "relay vid\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "TCP flag field\n"
    "pattern match result\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "redirect packet\n"
    "port number <0-28>\n"
    )
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);
    
    if (_parse_acl_rule(argv[1], rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    action.actGroup = ACL_ACTION_DIRECTION;
    action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_REDIRECT;
    action.un.permit_drop_redirect.portid = atoi(argv[4]);

    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_direction2 */

DEFUN (
    diag_shell_pie_set_aclRule_mirror,
    cmd_diag_shell_pie_set_aclRule_mirror,
    "pie set acl rule <0-511> (slp|frame-type|ether-type|rvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|tcp-flag|pattern-match|pkt-svid) CONTENT CAREBITS action mirror MIRRORID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"   
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "relay vid\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "TCP flag field\n"
    "pattern match result\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "mirror packet\n"
    "mirror index\n"
    )
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              mirror_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    

    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);
    
    if (_parse_acl_rule(argv[1], rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }

    CLI_CHK_INUPT_STR_NUMBER(mirror_id, argv[4], "Action group input error", uint32);

    action.actGroup = ACL_ACTION_MIRROR;
    action.un.mirror.mirrorsetid = mirror_id;
        
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_mirror */

DEFUN (
    diag_shell_pie_set_aclRule_log,
    cmd_diag_shell_pie_set_aclRule_log,
    "pie set acl rule <0-511> (slp|frame-type|ether-type|rvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|tcp-flag|pattern-match|pkt-svid) CONTENT CAREBITS action log LOGID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "relay vid\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "TCP flag field\n"
    "pattern match result\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "log counter\n"
    "log index\n"
    )
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              log_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if(TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
        

    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);

    if (_parse_acl_rule(argv[1], rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }

    CLI_CHK_INUPT_STR_NUMBER(log_id, argv[4], "Action group input error", uint32);

    action.actGroup = ACL_ACTION_LOG;
    action.un.log.logindex = log_id;
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_log */

DEFUN (
    diag_shell_pie_set_aclRule_rateLimit,
    cmd_diag_shell_pie_set_aclRule_rateLimit,
    "pie set acl rule <0-511> (slp|frame-type|ether-type|rvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|tcp-flag|pattern-match|pkt-svid) CONTENT CAREBITS action rate-limit RATEID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "relay vid\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "TCP flag field\n"
    "pattern match result\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "rate limit\n"
    "rate index\n"
    )
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              rate_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);

    if (_parse_acl_rule(argv[1], rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    CLI_CHK_INUPT_STR_NUMBER(rate_id, argv[4], "Action group input error", uint32);

    action.actGroup = ACL_ACTION_RATELIMIT;
    action.un.ratelimit.rateindex = rate_id;
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_rateLimit */

DEFUN (
    diag_shell_pie_set_aclRule_newSvid,
    cmd_diag_shell_pie_set_aclRule_newSvid,
    "pie set acl rule <0-511> (slp|frame-type|ether-type|rvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|tcp-flag|pattern-match|pkt-svid) CONTENT CAREBITS action new-svid SVID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL\n"
    "rule index\n"
    "index <0-511>\n"
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "relay vid\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "TCP flag field\n"
    "pattern match result\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "new SVID\n"
    "SVID\n"
    )
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              svid = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);

    if (_parse_acl_rule(argv[1], rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    CLI_CHK_INUPT_STR_NUMBER(svid, argv[4], "Action group input error", uint32);

    action.actGroup = ACL_ACTION_NEWSVID;
    action.un.newsvid.svid = svid;
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_newSvid */

DEFUN (
    diag_shell_pie_set_aclRule_newCvid,
    cmd_diag_shell_pie_set_aclRule_newCvid,
    "pie set acl rule <0-511> (slp|frame-type|ether-type|rvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|tcp-flag|pattern-match|pkt-svid) CONTENT CAREBITS action new-cvid [(cvid|) CVID] [(cpri|) CPRI]",
    "PIE configuration\n"
    "set configuration\n"
    "ACL\n"
    "rule index\n"
    "index <0-511>\n"
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "relay vid\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "TCP flag field\n"
    "pattern match result\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "new CVID\n"
    "cvid\n"
    "CVID\n"
    "cpri\n"
    "CPRI\n"
    )
{
    uint32              unit = 0;
    uint32              rule_content = 0;
    uint32              rule_carebit = 0;
    uint32              cvid = 0;
    uint32              cpri = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);

    if (_parse_acl_rule(argv[1], rule_content, rule_carebit, &acl_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    action.actGroup = ACL_ACTION_NEWCVID;
    if (argc < 5)
    {
        action.un.newcvid.replacecvid = PIE_BIT_OFF;
        action.un.newcvid.replacecpri = PIE_BIT_OFF;
        action.un.newcvid.cvid = 0;
        action.un.newcvid.cpri = 0;
    }
    else if (6 == argc)
    {
        if ('v' == argv[4][1])
        {
            CLI_CHK_INUPT_STR_NUMBER(cvid, argv[5], "Rule care bit input error", uint32);

            action.un.newcvid.replacecvid = PIE_BIT_ON;
            action.un.newcvid.replacecpri = PIE_BIT_OFF;
            action.un.newcvid.cvid = cvid;
            action.un.newcvid.cpri = 0;
        }
        else
        {
            CLI_CHK_INUPT_STR_NUMBER(cpri, argv[5], "Rule care bit input error", uint32);

            action.un.newcvid.replacecvid = PIE_BIT_OFF;
            action.un.newcvid.replacecpri = PIE_BIT_ON;
            action.un.newcvid.cvid = 0;
            action.un.newcvid.cpri = cpri;
        }
    }
    else
    {
        CLI_CHK_INUPT_STR_NUMBER(cvid, argv[5], "Rule care bit input error", uint32);
        CLI_CHK_INUPT_STR_NUMBER(cpri, argv[7], "Rule care bit input error", uint32);
        action.un.newcvid.replacecvid = PIE_BIT_ON;
        action.un.newcvid.replacecpri = PIE_BIT_ON;
        action.un.newcvid.cvid = cvid;
        action.un.newcvid.cpri = cpri;
    }
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_newCvid */

DEFUN (
    diag_shell_pie_set_aclRule_macDirection1,
    cmd_diag_shell_pie_set_aclRule_macDirection1,
    "pie set acl rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action (permit|drop|copy-to-cpu)",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "permit\n"
    "drop\n"
    "copy to cpu\n"
    )
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);
        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    if ('p' == argv[4][0]) /* permit */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_PERMIT;
    }
    else if ('d' == argv[4][0]) /* drop */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_DROP;
    }
    else if ('c' == argv[4][0]) /* copy-to-cpy */
    {
        action.actGroup = ACL_ACTION_DIRECTION;
        action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_COPYTOCPUY;
    } 
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
       
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_macDirection1 */

DEFUN (
    diag_shell_pie_set_aclRule_macDirection2,
    cmd_diag_shell_pie_set_aclRule_macDirection2,
    "pie set acl rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action redirect <0-28>",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "redirect packet\n"
    "port number <0-28>\n"
    )
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));    
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    action.actGroup = ACL_ACTION_DIRECTION;
    action.un.permit_drop_redirect.acttype = ACL_ACTION_DIRECTION_REDIRECT;
    action.un.permit_drop_redirect.portid = atoi(argv[4]);

    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_macDirection2 */

DEFUN (
    diag_shell_pie_set_aclRule_macMirror,
    cmd_diag_shell_pie_set_aclRule_macMirror,
    "pie set acl rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action mirror MIRRORID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "mirror\n"
    "mirror index\n"
    )
{
    uint32              unit = 0;
    uint32              mirror_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }

    CLI_CHK_INUPT_STR_NUMBER(mirror_id, argv[4], "Action group input error", uint32);

    action.actGroup = ACL_ACTION_MIRROR;
    action.un.mirror.mirrorsetid = mirror_id;
        
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_macMirror */

DEFUN (
    diag_shell_pie_set_aclRule_macLog,
    cmd_diag_shell_pie_set_aclRule_macLog,
    "pie set acl rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action log LOGID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"   
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "log counter\n"
    "log index\n"
    )
{
    uint32              unit = 0;
    uint32              log_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];    
        
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));    
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_ERR_NO_MATCH;
    }
 
    CLI_CHK_INUPT_STR_NUMBER(log_id, argv[4], "Action group input error", uint32);

    action.actGroup = ACL_ACTION_LOG;
    action.un.log.logindex = log_id;
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_macLog */

DEFUN (
    diag_shell_pie_set_aclRule_macRateLimit,
    cmd_diag_shell_pie_set_aclRule_macRateLimit,
    "pie set acl rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action rate-limit RATEID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"   
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "rate limit\n"
    "rate index\n"
    )
{
    uint32              unit = 0;
    uint32              rate_id = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
                
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    CLI_CHK_INUPT_STR_NUMBER(rate_id, argv[4], "Action group input error", uint32);
    action.actGroup = ACL_ACTION_RATELIMIT;
    action.un.ratelimit.rateindex = rate_id;
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_macRateLimit */

DEFUN (
    diag_shell_pie_set_aclRule_macNewSvid,
    cmd_diag_shell_pie_set_aclRule_macNewSvid,
    "pie set acl rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action new-svid SVID",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "new SVID\n"
    "SVID\n"    
    )
{
    uint32              unit = 0;
    uint32              svid = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }

    CLI_CHK_INUPT_STR_NUMBER(svid, argv[4], "Action group input error", uint32);

    action.actGroup = ACL_ACTION_NEWSVID;
    action.un.newsvid.svid = svid;
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_macNewSvid */

DEFUN (
    diag_shell_pie_set_aclRule_macNewCvid,
    cmd_diag_shell_pie_set_aclRule_macNewCvid,
    "pie set acl rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action new-cvid [(cvid|) CVID] [(cpri|) CPRI]",
    "PIE configuration\n"
    "set configuration\n"
    "ACL configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "new CVID\n"
    "cvid\n"
    "CVID\n"
    "cpri\n"
    "CPRI\n"
    )
{
    uint32              unit = 0;
    uint32              cvid = 0;
    uint32              cpri = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_filter_id_t     filter_id = 0;
    rtk_filter_aclCfg_t acl_cfg;
    rtk_filter_action_t action;
    uint8               mac_content[MAC_ADDR_LEN];
    uint8               mac_carebit[MAC_ADDR_LEN];
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_igrAcl_get(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == acl_cfg.valid)
    {
        if ((ret = rtk_filter_igrAcl_del(unit, filter_id)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            return CMD_WARNING;
        } 
    }
    else
    {
        memset(&acl_cfg, 0, sizeof(rtk_filter_aclCfg_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(acl_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(acl_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(acl_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
        
    action.actGroup = ACL_ACTION_NEWCVID;
    if (argc < 5)
    {
        action.un.newcvid.replacecvid = PIE_BIT_OFF;
        action.un.newcvid.replacecpri = PIE_BIT_OFF;
        action.un.newcvid.cvid = 0;
        action.un.newcvid.cpri = 0;
    }
    else if (6 == argc)
    {
        if ('v' == argv[4][1])
        {
            CLI_CHK_INUPT_STR_NUMBER(cvid, argv[5], "Rule care bit input error", uint32);

            action.un.newcvid.replacecvid = PIE_BIT_ON;
            action.un.newcvid.replacecpri = PIE_BIT_OFF;
            action.un.newcvid.cvid = cvid;
            action.un.newcvid.cpri = 0;
        }
        else
        {
            CLI_CHK_INUPT_STR_NUMBER(cpri, argv[5], "Rule care bit input error", uint32);
            action.un.newcvid.replacecvid = PIE_BIT_OFF;
            action.un.newcvid.replacecpri = PIE_BIT_ON;
            action.un.newcvid.cvid = 0;
            action.un.newcvid.cpri = cpri;
        }
    }
    else
    {
        CLI_CHK_INUPT_STR_NUMBER(cvid, argv[5], "Rule care bit input error", uint32);
        CLI_CHK_INUPT_STR_NUMBER(cpri, argv[7], "Rule care bit input error", uint32);

        action.un.newcvid.replacecvid = PIE_BIT_ON;
        action.un.newcvid.replacecpri = PIE_BIT_ON;
        action.un.newcvid.cvid = cvid;
        action.un.newcvid.cpri = cpri;
    }
    
    acl_cfg.valid = TRUE;
    if ((ret = rtk_filter_igrAcl_add(unit, filter_id, &acl_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_aclRule_macNewCvid */

DEFUN (
    diag_shell_pie_set_flowTableRule_log,
    cmd_diag_shell_pie_set_flowTableRule_log,
    "pie set flow-table rule <0-511> (slp|frame-type|ether-type|pkt-ctag-if|pkt-cpri|pkt-cvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|pkt-stag-if|pkt-spri|pkt-svid) CONTENT CAREBITS action log LOGID",
    "PIE configuration\n"
    "set configuration\n"
    "flow table configuration\n"
    "rule index\n"
    "index <0-511>\n"   
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "if VLAN tagged\n"
    "the priority in packet\n"
    "the vid in packet\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "if SVLAN tag\n"
    "the user priority of stag\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "log counter\n"
    "log index\n"
    )
{
    uint32               unit = 0;
    uint32               rule_content = 0;
    uint32               rule_carebit = 0;
    uint32               log_id = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == flow_table_cfg.valid)
    {
        rtk_filter_flowTbl_del(unit, filter_id);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    /* support API: is empty rule check?? */

    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);

    if (_parse_flowTable_rule(argv[1], rule_content, rule_carebit, &flow_table_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }

    CLI_CHK_INUPT_STR_NUMBER(log_id, argv[4], "Action group input error", uint32);

    action.actGroup = FLOW_ACTION_LOG;
    action.un.log.logindex = log_id;
    
    flow_table_cfg.valid = TRUE;
    if ((ret = rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  

    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_flowTableRule_log */

DEFUN (
    diag_shell_pie_set_flowTableRule_assignVlan,
    cmd_diag_shell_pie_set_flowTableRule_assignVlan,
    "pie set flow-table rule <0-511> (slp|frame-type|ether-type|pkt-ctag-if|pkt-cpri|pkt-cvid|ipv4|ipv6|ipv6-mld|sip|dip|flow-lable|ip-proto|tos|src-port|dst-port|pkt-stag-if|pkt-spri|pkt-svid) CONTENT CAREBITS action assign-vlan [(ctag|) CTAG] [(spri|) SPRI]",
    "PIE configuration\n"
    "set configuration\n"
    "flow table configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "source logical port\n"
    "frame type\n"
    "ether type\n"
    "if VLAN tagged\n"
    "the priority in packet\n"
    "the vid in packet\n"
    "IPv4 packet\n"
    "IPv6 packet\n"
    "IPv6 MLD packet\n"
    "IPv4 source IP address\n"
    "IPv4 destination IP address\n"
    "IPv6 flow label field\n"
    "IPv4 protocol/IPv6 next header\n"
    "IPv4 TOS/IPv6 class\n"
    "TCP/UDP source port\n"
    "TCP/UDP destination port\n"
    "if SVLAN tag\n"
    "the user priority of stag\n"
    "the svid in stag\n"
    "rule content\n"
    "care bits\n"
    "action\n"
    "assign vlan\n"
    "ctag\n"
    "CTAG\n"
    "spri\n"
    "SPRI\n"
    )
{
    uint32               unit = 0;
    uint32               rule_content = 0;
    uint32               rule_carebit = 0;
    uint32               ctag = 0;
    uint32               spri = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == flow_table_cfg.valid)
    {
        rtk_filter_flowTbl_del(unit, filter_id);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }

    CLI_CHK_INUPT_STR_NUMBER(rule_content, argv[2], "Rule content input error", uint32);
    CLI_CHK_INUPT_STR_NUMBER(rule_carebit, argv[3], "Rule care bit input error", uint32);

    if (_parse_flowTable_rule(argv[1], rule_content, rule_carebit, &flow_table_cfg) != RT_ERR_OK)
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }

    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    if (argc < 5)
    {
        action.un.assignVlan.usepktctag = 0;
        action.un.assignVlan.usepktspri = 0;
        action.un.assignVlan.actvid = 0;
        action.un.assignVlan.actpri = 0;
    }
    else if (6 == argc)
    {
        if ('c' == argv[4][0])
        {
            CLI_CHK_INUPT_STR_NUMBER(ctag, argv[5], "Rule care bit input error", uint32);

            action.un.assignVlan.usepktctag = 1;
            action.un.assignVlan.usepktspri = 0;
            action.un.assignVlan.actvid = ctag;
            action.un.assignVlan.actpri = 0;
        }
        else
        {
            CLI_CHK_INUPT_STR_NUMBER(spri, argv[5], "Rule care bit input error", uint32);

            action.un.assignVlan.usepktctag = 0;
            action.un.assignVlan.usepktspri = 1;
            action.un.assignVlan.actvid = 0;
            action.un.assignVlan.actpri = spri;
        }
    }
    else
    {
        CLI_CHK_INUPT_STR_NUMBER(ctag, argv[5], "Rule care bit input error", uint32);
        CLI_CHK_INUPT_STR_NUMBER(spri, argv[7], "Rule care bit input error", uint32);

        action.un.assignVlan.usepktctag = 1;
        action.un.assignVlan.usepktspri = 1;
        action.un.assignVlan.actvid = ctag;
        action.un.assignVlan.actpri = spri;
    }
    
    flow_table_cfg.valid = TRUE;
    if ((ret = rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  

    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_flowTableRule_assignVlan */

DEFUN (
    diag_shell_pie_set_flowTableRule_macLog,
    cmd_diag_shell_pie_set_flowTableRule_macLog,
    "pie set flow-table rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action log LOGID",
    "PIE configuration\n"
    "set configuration\n"
    "flow table configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "log counter\n"
    "log index\n"
    )
{
    uint32               unit = 0;
    uint32               log_id = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    uint8                mac_content[MAC_ADDR_LEN];
    uint8                mac_carebit[MAC_ADDR_LEN];
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == flow_table_cfg.valid)
    {
        rtk_filter_flowTbl_del(unit, filter_id);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(flow_table_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(flow_table_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }

    CLI_CHK_INUPT_STR_NUMBER(log_id, argv[4], "Action group input error", uint32);

    action.actGroup = FLOW_ACTION_LOG;
    action.un.log.logindex = log_id;
    
    flow_table_cfg.valid = TRUE;
    if ((ret = rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  

    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_flowTableRule_macLog */

DEFUN (
    diag_shell_pie_set_flowTableRule_macAssignVlan,
    cmd_diag_shell_pie_set_flowTableRule_macAssignVlan,
    "pie set flow-table rule <0-511> (dmac|smac) A:B:C:D:E:F A:B:C:D:E:F action assign-vlan [(ctag|) CTAG] [(spri|) SPRI]",
    "PIE configuration\n"
    "set configuration\n"
    "flow table configuration\n"
    "rule index\n"
    "index <0-511>\n"
    "destination mac\n"
    "source mac\n"
    "mac-address A:B:C:D:E:F\n"
    "care bits A:B:C:D:E:F\n"
    "action\n"
    "assign vlan\n"
    "(ctag\n"
    "CTAG\n"
    "spri\n"
    "SPRI\n"
    )
{
    uint32               unit = 0;
    uint32               ctag = 0;
    uint32               spri = 0;
    int32                ret = RT_ERR_FAILED;
    rtk_filter_id_t      filter_id = 0;
    rtk_filter_flowTbl_t flow_table_cfg;
    rtk_filter_action_t  action;
    uint8                mac_content[MAC_ADDR_LEN];
    uint8                mac_carebit[MAC_ADDR_LEN];
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    filter_id = atoi(argv[0]);
    
    memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
    memset(&action, 0, sizeof(rtk_filter_action_t));

    if ((ret = rtk_filter_flowTbl_get(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    if (TRUE == flow_table_cfg.valid)
    {
        rtk_filter_flowTbl_del(unit, filter_id);
    }
    else
    {
        memset(&flow_table_cfg, 0, sizeof(rtk_filter_flowTbl_t));
        memset(&action, 0, sizeof(rtk_filter_action_t));
    }
    
    memset(mac_content, 0, MAC_ADDR_LEN * sizeof(uint8));
    memset(mac_carebit, 0, MAC_ADDR_LEN * sizeof(uint8));
    rt_util_str2Mac(mac_content, argv[2]);
    rt_util_str2Mac(mac_carebit, argv[3]);
        
    if ('s' == argv[1][0])
    {
        memcpy(flow_table_cfg.smac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_smac, mac_carebit, MAC_ADDR_LEN);        
    }
    else if ('d' == argv[1][0])
    {
        memcpy(flow_table_cfg.dmac, mac_content, MAC_ADDR_LEN);
        memcpy(flow_table_cfg.care_dmac, mac_carebit, MAC_ADDR_LEN);        
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    action.actGroup = FLOW_ACTION_ASSIGNVLAN;
    if (argc < 5)
    {
        action.un.assignVlan.usepktctag = 0;
        action.un.assignVlan.usepktspri = 0;
        action.un.assignVlan.actvid = 0;
        action.un.assignVlan.actpri = 0;
    }
    else if (6 == argc)
    {
        if ('c' == argv[4][0])
        {
            CLI_CHK_INUPT_STR_NUMBER(ctag, argv[5], "Rule care bit input error", uint32);

            action.un.assignVlan.usepktctag = 1;
            action.un.assignVlan.usepktspri = 0;
            action.un.assignVlan.actvid = ctag;
            action.un.assignVlan.actpri = 0;
        }
        else
        {
            CLI_CHK_INUPT_STR_NUMBER(spri, argv[5], "Rule care bit input error", uint32);

            action.un.assignVlan.usepktctag = 0;
            action.un.assignVlan.usepktspri = 1;
            action.un.assignVlan.actvid = 0;
            action.un.assignVlan.actpri = spri;
        }
    }
    else
    {
        CLI_CHK_INUPT_STR_NUMBER(ctag, argv[5], "Rule care bit input error", uint32);
        CLI_CHK_INUPT_STR_NUMBER(spri, argv[7], "Rule care bit input error", uint32);

        action.un.assignVlan.usepktctag = 1;
        action.un.assignVlan.usepktspri = 1;
        action.un.assignVlan.actvid = ctag;
        action.un.assignVlan.actpri = spri;
    }
    
    flow_table_cfg.valid = TRUE;
    if ((ret = rtk_filter_flowTbl_add(unit, filter_id, &flow_table_cfg, &action)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }  

    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_flowTableRule_macAssignVlan */

DEFUN (
    diag_shell_pie_del_rule,
    cmd_diag_shell_pie_del_rule,
    "pie del (acl|flow-table) rule <0-511> [<0-511>]",
    "PIE configuration\n"
    "delete configuration\n"
    "ACL configuration\n"
    "flow table configuration\n"
    "rule index\n"
    "start index\n"
    "end index\n"
    )
{
    uint32          unit = 0;
    uint32          index = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_filter_id_t start_idx = 0;
    rtk_filter_id_t end_idx = 0;
    
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    start_idx = atoi(argv[1]);
    if ('a' == argv[0][0])
    {
        if (argc < 3)
        {
            if ((ret = rtk_filter_igrAcl_del(unit, start_idx)) != RT_ERR_OK)
            {
                DIAG_SHELL_ERR_PRINT(ret);
                return CMD_WARNING;
            } 
        }
        else
        {    
            end_idx = atoi(argv[2]);
            for (index = start_idx; index <= end_idx; index++)
            {
                if ((ret = rtk_filter_igrAcl_del(unit, index)) != RT_ERR_OK)
                {
                    /*Don't check error, continue to del next entry*/
                }
            }    
        }    
    }    
    else if ('f' == argv[0][0])
    {
        if (argc < 3)
        {
            if ((ret = rtk_filter_flowTbl_del(unit, start_idx)) != RT_ERR_OK)
            {
                DIAG_SHELL_ERR_PRINT(ret);
                return CMD_WARNING;
            } 
        }
        else
        {    
            end_idx = atoi(argv[2]);
            for (index = start_idx; index <= end_idx; index++)
            {
                if ((ret = rtk_filter_flowTbl_del(unit, index)) != RT_ERR_OK)
                {
                    /*Don't check error, continue to del next entry*/
                }
            }   
        }  
    }
    else
    {
        printf("User config: Error!\n");
        return CMD_ERR_NO_MATCH;
    }
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_del_rule */

DEFUN (
    diag_shell_pie_dump_rateLimit,
    cmd_diag_shell_pie_dump_rateLimit,
    "pie dump rate-limit [<0-127>]",
    "PIE configuration\n"
    "dump configuration\n"
    "rate limit\n"
    "index <0-127>\n"
    )
{
    uint32  unit = 0;
    uint32  rate = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    if (argc < 1)
    {
        for (index = 0; index < PIE_MAX_RATELIMIT_INDEX; index++)
        {
            if ((ret = rtk_filter_igrAclRateLimit_get(unit,  index, &rate)) != RT_ERR_OK)
            {
                DIAG_SHELL_ERR_PRINT(ret);
                mprintf_end();
                return CMD_WARNING;
            }
            mprintf("Index:%d    rate=0x%x\n", index, rate);
        }    
    }
    else
    {
        index = atoi(argv[0]);
        if ((ret = rtk_filter_igrAclRateLimit_get(unit,  index, &rate)) != RT_ERR_OK)
        {
            DIAG_SHELL_ERR_PRINT(ret);
            mprintf_end();
            return CMD_WARNING;
        }
        mprintf("Index:%d    rate=0x%x\n", index, rate);
    }    
    
    mprintf_end();
    return CMD_SUCCESS;
} /* end of diag_shell_pie_dump_rateLimit */

DEFUN (
    diag_shell_pie_set_rateLimit,
    cmd_diag_shell_pie_set_rateLimit,
    "pie set rate-limit <0-127> rate RATE",
    "PIE configuration\n"
    "set configuration\n"
    "rate limit\n"
    "index <0-127>\n"
    "rate\n"
    "rate\n"
    )
{
    uint32  unit = 0;
    uint32  rate = 0;
    uint32  index = 0;
    int32   ret = RT_ERR_FAILED;
            
    if (diag_shell_get_chip_id(&unit) != RT_ERR_OK)
    {
        return CMD_WARNING;
    }
    
    index = atoi(argv[0]);

    CLI_CHK_INUPT_STR_NUMBER(rate, argv[1], "rate input error", uint32);
    
    if ((ret = rtk_filter_igrAclRateLimit_set(unit, index, rate)) != RT_ERR_OK)
    {
        DIAG_SHELL_ERR_PRINT(ret);
        return CMD_WARNING;
    }
    
    return CMD_SUCCESS;
} /* end of diag_shell_pie_set_rateLimit */

void
cmd_diag_shell_pie_init (void)
{
    install_element(VIEW_NODE, &cmd_diag_shell_pie_del_rule);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_dump_rule);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_dump_logCounter);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_dump_rateLimit);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_enable);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_direction1);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_direction2);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_log);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_mirror);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_newCvid);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_newSvid);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_rateLimit);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_macDirection1);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_macDirection2);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_macLog);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_macMirror);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_macNewCvid);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_macNewSvid);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_aclRule_macRateLimit);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_cutLine);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_flowTableRule_assignVlan);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_flowTableRule_log);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_flowTableRule_macAssignVlan);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_flowTableRule_macLog);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_clear_logCounter);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_set_rateLimit);
    install_element(VIEW_NODE, &cmd_diag_shell_pie_show);
    return;
} /* end of cmd_diag_shell_pie_init */
    

#endif

#if defined(CONFIG_SDK_RTL8328)
static int32
_parse_acl_rule(char *argv, uint32 rule_content, uint32 rule_carebit, rtk_filter_aclCfg_t *acl_cfg)
{
    if ('s' == argv[0])
    {
        if ('l' == argv[1])
        {
            acl_cfg->slp = rule_content;
            acl_cfg->care_slp = rule_carebit;
        }
        else if ('i' == argv[1])
        {
            acl_cfg->sip = rule_content;
            acl_cfg->care_sip = rule_carebit;
        }
        else if ('r' == argv[1])
        {
            acl_cfg->srcport = rule_content;
            acl_cfg->care_srcport = rule_carebit;            
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('d' == argv[0])
    {
        if ('i' == argv[1])
        {
            acl_cfg->dip = rule_content;
            acl_cfg->care_dip = rule_carebit;
        }
        else if ('s' == argv[1])
        {
            acl_cfg->dstport = rule_content;
            acl_cfg->care_dstport = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('f' == argv[0])
    {
        if ('r' == argv[1])
        {
            acl_cfg->frametype = rule_content;
            acl_cfg->care_frametype = rule_carebit;
        }
        else if ('l' == argv[1])
        {
            acl_cfg->flowlabel = rule_content;
            acl_cfg->care_flowlabel = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('e' == argv[0])
    {
        acl_cfg->ethertype = rule_content;
        acl_cfg->care_ethertype = rule_carebit;
    }
    else if ('r' == argv[0])
    {
        acl_cfg->rvid = rule_content;
        acl_cfg->care_rvid = rule_carebit;
    }
    else if ('i' == argv[0])
    {
        if ('4' == argv[3])
        {
            acl_cfg->ipv4 = rule_content;
            acl_cfg->care_ipv4 = rule_carebit;
        }
        else if ('6' == argv[3])
        {
            if (4 == strlen(argv))
            {
                acl_cfg->ipv6 = rule_content;
                acl_cfg->care_ipv6 = rule_carebit;
            }
            else
            {
                acl_cfg->ipv6mld = rule_content;
                acl_cfg->care_ipv6mld = rule_carebit;
            }    
        }
        else if ('p' == argv[3])
        {
            acl_cfg->ipproto = rule_content;
            acl_cfg->care_ipproto = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('t' == argv[0])
    {
        if ('o' == argv[1])
        {
            acl_cfg->tos = rule_content;
            acl_cfg->care_tos = rule_carebit;
        }
        else if ('c' == argv[1])
        {
            acl_cfg->tcpflag = rule_content;
            acl_cfg->care_tcpflag = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('p' == argv[0])
    {
        if ('a' == argv[1])
        {
            acl_cfg->patternmatch = rule_content;
            acl_cfg->care_patternmatch = rule_carebit;
        }
        else if ('k' == argv[1])
        {
            acl_cfg->pktsvid = rule_content;
            acl_cfg->care_pktsvid = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    return RT_ERR_OK;   
} /* end of _parse_acl_rule */

static int32
_parse_flowTable_rule(char *argv, uint32 rule_content, uint32 rule_carebit, rtk_filter_flowTbl_t *flow_table_cfg)
{
    if ('s' == argv[0])
    {
        if ('l' == argv[1])
        {
            flow_table_cfg->slp = rule_content;
            flow_table_cfg->care_slp = rule_carebit;
        }
        else if ('i' == argv[1])
        {
            flow_table_cfg->sip = rule_content;
            flow_table_cfg->care_sip = rule_carebit;
        }
        else if ('r' == argv[1])
        {
            flow_table_cfg->srcport = rule_content;
            flow_table_cfg->care_srcport = rule_carebit;            
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('d' == argv[0])
    {
        if ('i' == argv[1])
        {
            flow_table_cfg->dip = rule_content;
            flow_table_cfg->care_dip = rule_carebit;
        }
        else if ('s' == argv[1])
        {
            flow_table_cfg->dstport = rule_content;
            flow_table_cfg->care_dstport = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('f' == argv[0])
    {
        if ('r' == argv[1])
        {
            flow_table_cfg->frametype = rule_content;
            flow_table_cfg->care_frametype = rule_carebit;
        }
        else if ('l' == argv[1])
        {
            flow_table_cfg->flowlabel = rule_content;
            flow_table_cfg->care_flowlabel = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('e' == argv[0])
    {
        flow_table_cfg->ethertype = rule_content;
        flow_table_cfg->care_ethertype = rule_carebit;
    }
    else if ('i' == argv[0])
    {
        if ('4' == argv[3])
        {
            flow_table_cfg->ipv4 = rule_content;
            flow_table_cfg->care_ipv4 = rule_carebit;
        }
        else if ('6' == argv[3])
        {
            if (4 == strlen(argv))
            {
                flow_table_cfg->ipv6 = rule_content;
                flow_table_cfg->care_ipv6 = rule_carebit;
            }
            else
            {
                flow_table_cfg->ipv6mld = rule_content;
                flow_table_cfg->care_ipv6mld = rule_carebit;
            }    
        }
        else if ('p' == argv[3])
        {
            flow_table_cfg->ipproto = rule_content;
            flow_table_cfg->care_ipproto = rule_carebit;
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('t' == argv[0])
    {
        if ('o' == argv[1])
        {
            flow_table_cfg->tos = rule_content;
            flow_table_cfg->care_tos = rule_carebit;
        }        
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    else if ('p' == argv[0])
    {
        if ('k' == argv[1])
        {
            if ('c' == argv[4])
            {
                if ('t' == argv[5])
                {
                    flow_table_cfg->pktctagif = rule_content;
                    flow_table_cfg->care_pktctagif = rule_carebit;
                }
                else if ('p' == argv[5])
                {
                    flow_table_cfg->pktcpri = rule_content;
                    flow_table_cfg->care_pktcpri = rule_carebit;
                }
                else if ('v' == argv[5])
                {
                    flow_table_cfg->pktcvid = rule_content;
                    flow_table_cfg->care_pktcvid = rule_carebit;
                }    
                else
                {
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }
            }        
            else if ('s' == argv[4])
            {
                if ('t' == argv[5])
                {
                    flow_table_cfg->pktstagif = rule_content;
                    flow_table_cfg->care_pktstagif = rule_carebit;
                }    
                else if ('p' == argv[5])
                {
                    flow_table_cfg->pktspri = rule_content;
                    flow_table_cfg->care_pktspri = rule_carebit;
                }
                else if ('v' == argv[5])
                {
                    flow_table_cfg->pktsvid = rule_content;
                    flow_table_cfg->care_pktsvid = rule_carebit;    
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
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }
    
    return RT_ERR_OK;   
} /* end of _parse_flowTable_rule */
#endif
