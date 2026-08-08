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
 * $Revision: 29712 $
 * $Date: 2012-06-11 17:08:13 +0800 (Mon, 11 Jun 2012) $
 *
 * Purpose : Define diag shell functions for dot1x.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) dot1x diag shell.
 */


#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <rtk/qos.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


#ifdef CMD_QOS_SET_PKT2CPU_PRIORITY_REMAP_INTPRI_NEWPRI
/*
 * qos set pkt2cpu-priority-remap <UINT:intpri> <UINT:newpri>
 */
cparser_result_t cparser_cmd_qos_set_pkt2cpu_priority_remap_intpri_newpri(cparser_context_t *context,
    uint32_t *intpri_ptr,
    uint32_t *newpri_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if ((ret = rtk_qos_pkt2CpuPriRemap_set(unit, *intpri_ptr, *newpri_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_PRIORITY_SELECTOR_SYSTEM_PORT_WEIGHT_CLASS_WEIGHT_ACL_WEIGHT_DSCP_WEIGHT
/*
 * qos set priority-selector system <UINT:port_weight> <UINT:class_weight> <UINT:acl_weight> <UINT:dscp_weight>
 */
cparser_result_t cparser_cmd_qos_set_priority_selector_system_port_weight_class_weight_acl_weight_dscp_weight(cparser_context_t *context,
    uint32_t *port_weight_ptr,
    uint32_t *class_weight_ptr,
    uint32_t *acl_weight_ptr,
    uint32_t *dscp_weight_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
        
    if ((ret = rtk_qos_priSel_set(unit, *port_weight_ptr, *class_weight_ptr, *acl_weight_ptr, *dscp_weight_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    } 

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_PRIORITY_SELECTOR_GROUP_GROUP_ID_PORT_WEIGHT_DOT1Q_WEIGHT_DSCP_WEIGHT_FLOW_WEIGHT_INNER_WEIGHT_OUTER_WEIGHT
/*
 * qos set priority-selector group <UINT:group_id> <UINT:port_weight> <UINT:dot1q_weight> <UINT:dscp_weight> <UINT:flow_weight> <UINT:inner_weight> <UINT:outer_weight>
 */
cparser_result_t cparser_cmd_qos_set_priority_selector_group_group_id_port_weight_dot1q_weight_dscp_weight_flow_weight_inner_weight_outer_weight(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *port_weight_ptr,
    uint32_t *dot1q_weight_ptr,
    uint32_t *dscp_weight_ptr,
    uint32_t *flow_weight_ptr,
    uint32_t *inner_weight_ptr,
    uint32_t *outer_weight_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_priSelWeight_t  priSelWeight;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    
    priSelWeight.weight_of_portBased = *port_weight_ptr;
    priSelWeight.weight_of_dot1q = *dot1q_weight_ptr;
    priSelWeight.weight_of_dscp = *dscp_weight_ptr;
    priSelWeight.weight_of_flowBased = *flow_weight_ptr;
    priSelWeight.weight_of_innerTag = *inner_weight_ptr;
    priSelWeight.weight_of_outerTag = *outer_weight_ptr;
    
    if ((ret = rtk_qos_priSelGroup_set(unit, *group_id_ptr, &priSelWeight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_PRIORITY_SELECTOR_GROUP_GROUP_ID_PORT_WEIGHT_DSCP_WEIGHT_INACL_WEIGHT_INNER_WEIGHT_OUTER_WEIGHT_MACVLAN_WEIGHT_PROTOVLAN_WEIGHT
/*
 * qos set priority-selector group <UINT:group_id> <UINT:port_weight> <UINT:dscp_weight> <UINT:inacl_weight> <UINT:inner_weight> <UINT:outer_weight> <UINT:macvlan_weight> <UINT:protovlan_weight>
 */
cparser_result_t cparser_cmd_qos_set_priority_selector_group_group_id_port_weight_dscp_weight_inacl_weight_inner_weight_outer_weight_macvlan_weight_protovlan_weight(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *port_weight_ptr,
    uint32_t *dscp_weight_ptr,
    uint32_t *inacl_weight_ptr,
    uint32_t *inner_weight_ptr,
    uint32_t *outer_weight_ptr,
    uint32_t *macvlan_weight_ptr,
    uint32_t *protovlan_weight_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_priSelWeight_t  priSelWeight;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    
    priSelWeight.weight_of_portBased = *port_weight_ptr;
    priSelWeight.weight_of_dscp = *dscp_weight_ptr;
    priSelWeight.weight_of_inAcl = *inacl_weight_ptr;
    priSelWeight.weight_of_innerTag = *inner_weight_ptr;
    priSelWeight.weight_of_outerTag = *outer_weight_ptr;
    priSelWeight.weight_of_macVlan = *macvlan_weight_ptr;
    priSelWeight.weight_of_protoVlan= *protovlan_weight_ptr;
    
    if ((ret = rtk_qos_priSelGroup_set(unit, *group_id_ptr, &priSelWeight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif    

#ifdef CMD_QOS_SET_AVB_PORTS_ALL_SR_CLASS_A_SR_CLASS_B_STATE_ENABLE_DISABLE
/*
 * qos set avb ( <PORT_LIST:ports> | all ) ( sr-class-a | sr-class-b ) state ( enable | disable )
 */
cparser_result_t cparser_cmd_qos_set_avb_ports_all_sr_class_a_sr_class_b_state_enable_disable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_avbSrClass_t    srClass;
    rtk_enable_t    enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    srClass = ('a' == TOKEN_CHAR(4, 9))? AVB_SR_CLASS_A : AVB_SR_CLASS_B;
    enable = ('e' == TOKEN_CHAR(6, 0))? ENABLED : DISABLED;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portAvbStreamReservationClassEnable_set(unit, port, srClass, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_AVB_SR_CLASS_A_SR_CLASS_B_PRIORITY_PRIORITY
/*
 * qos set avb ( sr-class-a | sr-class-b ) priority <UINT:priority>
 */
cparser_result_t cparser_cmd_qos_set_avb_sr_class_a_sr_class_b_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    if ('a' == TOKEN_CHAR(3, 9))
    {
        srClassConf.class_a_priority = (rtk_pri_t)*priority_ptr;
    }
    else
    {
        srClassConf.class_b_priority = (rtk_pri_t)*priority_ptr;
    }
    
    if ((ret = rtk_qos_avbStreamReservationConfig_set(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_AVB_SR_CLASS_A_SR_CLASS_B_QUEUE_ID_QUEUE_ID
/*
 * qos set avb ( sr-class-a | sr-class-b ) queue-id <UINT:queue_id>
 */
cparser_result_t cparser_cmd_qos_set_avb_sr_class_a_sr_class_b_queue_id_queue_id(cparser_context_t *context,
    uint32_t *queue_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    if ('a' == TOKEN_CHAR(3, 9))
    {
        srClassConf.class_a_queue_id = (rtk_qid_t)*queue_id_ptr;
    }
    else
    {
        srClassConf.class_b_queue_id = (rtk_qid_t)*queue_id_ptr;
    }
    
    if ((ret = rtk_qos_avbStreamReservationConfig_set(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_AVB_NON_SR_CLASS_A_NON_SR_CLASS_B_REDIRECT_QUEUE_ID_QUEUE_ID
/*
 * qos set avb ( non-sr-class-a | non-sr-class-b ) redirect-queue-id <UINT:queue_id>
 */
cparser_result_t cparser_cmd_qos_set_avb_non_sr_class_a_non_sr_class_b_redirect_queue_id_queue_id(cparser_context_t *context,
    uint32_t *queue_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    if ('a' == TOKEN_CHAR(3, 13))
    {
        srClassConf.class_non_a_redirect_queue_id = (rtk_qid_t)*queue_id_ptr;
    }
    else
    {
        srClassConf.class_non_b_redirect_queue_id = (rtk_qid_t)*queue_id_ptr;
    }
    
    if ((ret = rtk_qos_avbStreamReservationConfig_set(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_AVB_NON_SR_CLASS_A_NON_SR_CLASS_B_REMARK_PRIORITY_PRIORITY
/*
 * qos set avb ( non-sr-class-a | non-sr-class-b ) remark-priority <UINT:priority>
 */
cparser_result_t cparser_cmd_qos_set_avb_non_sr_class_a_non_sr_class_b_remark_priority_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    if ('a' == TOKEN_CHAR(3, 13))
    {
        srClassConf.class_non_a_remark_priority = (rtk_qid_t)*priority_ptr;
    }
    else
    {
        srClassConf.class_non_b_remark_priority = (rtk_qid_t)*priority_ptr;
    }
    
    if ((ret = rtk_qos_avbStreamReservationConfig_set(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_SYSTEM_THRESHOLD_DROP_PRECEDENCE_DROP_PROBABILITY_PROBABILITY
/*
 * qos set congest-avoidance system-threshold <UINT:drop_precedence> drop-probability <UINT:probability>
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_system_threshold_drop_precedence_drop_probability_probability(cparser_context_t *context,
    uint32_t *drop_precedence_ptr,
    uint32_t *probability_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_congAvoidSysDropProbability_set(unit, *drop_precedence_ptr, *probability_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_QUEUES_ALL_DROP_PRECEDENCE_MAX_THRESHOLD_MIN_THRESHOLD
/*
 * qos set congest-avoidance queue-threshold ( <PORT_LIST:queues> | all ) <UINT:drop_precedence> <UINT:max_threshold> <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_queue_threshold_queues_all_drop_precedence_max_threshold_min_threshold(cparser_context_t *context,
    uint32_t *drop_precedence_ptr,
    uint32_t *max_threshold_ptr,
    uint32_t *min_threshold_ptr)
{
    uint32      unit = 0, queue;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t  queuelist;
    rtk_qos_congAvoidThresh_t   thresh;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(queuelist, 4), ret);
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;
    
    DIAG_UTIL_PORTMASK_SCAN(queuelist, queue)
    {        
        if ((ret = rtk_qos_congAvoidGlobalQueueThresh_set(unit, queue, *drop_precedence_ptr, &thresh)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif    

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_QUEUES_ALL_DROP_PRECEDENCE_DROP_PROBABILITY_PROBABILITY
/*
 * qos set congest-avoidance queue-threshold ( <PORT_LIST:queues> | all ) <UINT:drop_precedence> drop-probability <UINT:probability>
 */    
cparser_result_t cparser_cmd_qos_set_congest_avoidance_queue_threshold_queues_all_drop_precedence_drop_probability_probability(cparser_context_t *context,
    uint32_t *drop_precedence_ptr,
    uint32_t *probability_ptr)
{
    uint32      unit = 0, queue;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t  queuelist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(queuelist, 4), ret);
    
    DIAG_UTIL_PORTMASK_SCAN(queuelist, queue)
    {        
        if ((ret = rtk_qos_congAvoidGlobalQueueDropProbability_set(unit, queue, *drop_precedence_ptr, *probability_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif        

#ifdef CMD_QOS_SET_DP_SELECTOR_DEI_DSCP
/*
 * qos set dp-selector ( dei | dscp )
 */
cparser_result_t cparser_cmd_qos_set_dp_selector_dei_dscp(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_dpSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (0 == strcmp(context->parser->tokens[3].buf, "dei"))
    {
        type = DP_SRC_DEI_BASED;
    }
    else if (0 == strcmp(context->parser->tokens[3].buf, "dscp"))
    {
        type = DP_SRC_DSCP_BASED;
    }
    else
    {
        return CPARSER_NOT_OK;
    }
    
    if ((ret = rtk_qos_dpSrcSel_set(unit, type)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_PRIORITY_SELECTOR_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set priority-selector port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_priority_selector_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portPriSelGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_QUEUE_NUMBER_QUEUE_NUM
/*
 * qos set queue number <UINT:queue_num>
 */
cparser_result_t cparser_cmd_qos_set_queue_number_queue_num(cparser_context_t *context,
    uint32_t *queue_num_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_queueNum_set(unit, *queue_num_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif 

#ifdef CMD_QOS_SET_QUEUE_MAPPING_SYSTEM_QUEUE_NUM_PRIORITY_QUEUE_ID
/*
 * qos set queue mapping system <UINT:queue_num> <UINT:priority> <UINT:queue_id>
 */
cparser_result_t cparser_cmd_qos_set_queue_mapping_system_queue_num_priority_queue_id(cparser_context_t *context,
    uint32_t *queue_num_ptr,
    uint32_t *priority_ptr,
    uint32_t *queue_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_pri2queue_t pri2qid;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_priMap_get(unit, *queue_num_ptr, &pri2qid)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    pri2qid.pri2queue[*priority_ptr] = *queue_id_ptr;
    
    if ((ret = rtk_qos_priMap_set(unit, *queue_num_ptr, &pri2qid)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_QUEUE_MAPPING_PORT_PORTS_ALL_PRIORITY_QUEUE_ID
/*
 * qos set queue mapping port ( <PORT_LIST:ports> | all ) <UINT:priority> <UINT:queue_id>
 */
cparser_result_t cparser_cmd_qos_set_queue_mapping_port_ports_all_priority_queue_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *priority_ptr,
    uint32_t *queue_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portPriMap_set(unit, port, *priority_ptr, *queue_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_PORT_PORTS_ALL_INTER_PRIORITY_DROP_PRECEDENCE
/*
 * qos set remapping port ( <PORT_LIST:ports> | all ) <UINT:inter_priority> { <UINT:drop_precedence> }
 */
cparser_result_t cparser_cmd_qos_set_remapping_port_ports_all_inter_priority_drop_precedence(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *inter_priority_ptr,
    uint32_t *drop_precedence_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portPri_set(unit, port, *inter_priority_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        if (7 == context->parser->cmd_tokens )
        {
            if ((ret = rtk_qos_portDp_set(unit, port, *drop_precedence_ptr)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_PORT_PORTS_ALL_INNER_TAG_PRIORITY
/*
 * qos set remapping port ( <PORT_LIST:ports> | all ) inner-tag <UINT:priority>
 */
cparser_result_t cparser_cmd_qos_set_remapping_port_ports_all_inner_tag_priority(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portInnerPri_set(unit, port, *priority_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_PORT_PORTS_ALL_OUTER_TAG_PRIORITY_DEI
/*
 * qos set remapping port ( <PORT_LIST:ports> | all ) outer-tag <UINT:priority> <UINT:dei>
 */
cparser_result_t cparser_cmd_qos_set_remapping_port_ports_all_outer_tag_priority_dei(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *priority_ptr,
    uint32_t *dei_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portOuterPri_set(unit, port, *priority_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        if ((ret = rtk_qos_portOuterDEI_set(unit, port, *dei_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DEI_SYSTEM_DEI_VALUE_DROP_PRECEDENCE
/*
 * qos set remapping dei system <UINT:dei_value> <UINT:drop_precedence>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dei_system_dei_value_drop_precedence(cparser_context_t *context,
    uint32_t *dei_value_ptr,
    uint32_t *drop_precedence_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_deiDpRemap_set(unit, *dei_value_ptr, *drop_precedence_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DEI_TAG_SELECTOR_PORT_PORTS_ALL_INNER_TAG_OUTER_TAG
/*
 * qos set remapping dei tag-selector port ( <PORT_LIST:ports> | all ) ( inner-tag | outer-tag )
 */
cparser_result_t cparser_cmd_qos_set_remapping_dei_tag_selector_port_ports_all_inner_tag_outer_tag(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_deiSel_t type;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    
    if (0 == strcmp(context->parser->tokens[7].buf, "inner-tag"))
    {
        type = DEI_SEL_INNER_TAG;
    }
    else
    {
        type = DEI_SEL_OUTER_TAG;
    }
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portDEISrcSel_set(unit, port, type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DOT1P_SYSTEM_DOT1P_PRIORITY_INTERNAL_PRIORITY
/*
 * qos set remapping dot1p system <UINT:dot1p_priority> <UINT:internal_priority>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dot1p_system_dot1p_priority_internal_priority(cparser_context_t *context,
    uint32_t *dot1p_priority_ptr,
    uint32_t *internal_priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_1pPriRemap_set(unit, *dot1p_priority_ptr, *internal_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DOT1P_GROUP_GROUP_ID_DOT1P_PRIORITY_INTER_PRIORITY_DROP_PRECEDENCE
/*
 * qos set remapping dot1p group <UINT:group_id> <UINT:dot1p_priority> <UINT:inter_priority> <UINT:drop_precedence>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dot1p_group_group_id_dot1p_priority_inter_priority_drop_precedence(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *dot1p_priority_ptr,
    uint32_t *inter_priority_ptr,
    uint32_t *drop_precedence_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_1pPriRemapGroup_set(unit, *group_id_ptr, *dot1p_priority_ptr
                                    , *inter_priority_ptr, *drop_precedence_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DOT1P_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set remapping dot1p port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dot1p_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_port1pPriRemapGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DSCP_SYSTEM_DSCP_ALL_DP_DROP_PRECEDENCE
/*
 * qos set remapping dscp system ( <MASK_LIST:dscp> | all ) dp <UINT:drop_precedence>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dscp_system_dscp_all_dp_drop_precedence(cparser_context_t *context,
    uint32_t *drop_precedence_ptr)
{
    uint32      unit = 0, dscp;
    int32       ret = RT_ERR_FAILED;
    diag_mask_t dscpMask;    

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK(dscpMask, 5, DIAG_MASKTYPE_DSCP), ret);
    DIAG_UTIL_MASK_SCAN(dscpMask, dscp)
    {
        if ((ret = rtk_qos_dscpDpRemap_set(unit, dscp, *drop_precedence_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DSCP_SYSTEM_DSCP_ALL_INTER_PRIORITY
/*
 * qos set remapping dscp system ( <MASK_LIST:dscp> | all ) <UINT:inter_priority>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dscp_system_dscp_all_inter_priority(cparser_context_t *context,
    uint32_t *inter_priority_ptr)
{
    uint32      unit = 0, dscp;
    int32       ret = RT_ERR_FAILED;
    diag_mask_t dscpMask;    

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK(dscpMask, 5, DIAG_MASKTYPE_DSCP), ret);
    DIAG_UTIL_MASK_SCAN(dscpMask, dscp)
    {
        if ((ret = rtk_qos_dscpPriRemap_set(unit, dscp, *inter_priority_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DSCP_GROUP_GROUP_ID_DSCP_VALUE_INTER_PRIORITY_DROP_PRECEDENCE
/*
 * qos set remapping dscp group <UINT:group_id> <UINT:dscp_value> <UINT:inter_priority> <UINT:drop_precedence>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dscp_group_group_id_dscp_value_inter_priority_drop_precedence(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *dscp_value_ptr,
    uint32_t *inter_priority_ptr,
    uint32_t *drop_precedence_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_dscpPriRemapGroup_set(unit, *group_id_ptr, *dscp_value_ptr, *inter_priority_ptr
                                            , *drop_precedence_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_DSCP_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set remapping dscp port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_remapping_dscp_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portDscpPriRemapGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_OUTER_TAG_GROUP_GROUP_ID_PRIORITY_DEI_INTER_PRIORITY_DROP_PRECEDENCE
/*
 * qos set remapping outer-tag group <UINT:group_id> <UINT:priority> <UINT:dei> <UINT:inter_priority> <UINT:drop_precedence>
 */
cparser_result_t cparser_cmd_qos_set_remapping_outer_tag_group_group_id_priority_dei_inter_priority_drop_precedence(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *priority_ptr,
    uint32_t *dei_ptr,
    uint32_t *inter_priority_ptr,
    uint32_t *drop_precedence_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_outer1pPriRemapGroup_set(unit, *group_id_ptr, *priority_ptr, *dei_ptr
                                            , *inter_priority_ptr, *drop_precedence_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_OUTER_TAG_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set remapping outer-tag port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_remapping_outer_tag_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portOuter1pPriRemapGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMAPPING_OUTER_TAG_SYSTEM_DEI_PRIORITY_INTER_PRIORITY
/*
 * qos set remapping outer-tag system <UINT:dei> <UINT:priority> <UINT:inter_priority>
 */
cparser_result_t cparser_cmd_qos_set_remapping_outer_tag_system_dei_priority_inter_priority(cparser_context_t *context,
    uint32_t *dei_ptr,
    uint32_t *priority_ptr,
    uint32_t *inter_priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_outer1pPriRemap_set(unit, *priority_ptr, *dei_ptr, *inter_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DOT1P_DSCP_OUTER_TAG_PORT_PORTS_ALL_STATE_ENABLE_DISABLE
/*
 * qos set remarking ( dot1p | dscp | outer-tag ) port ( <PORT_LIST:ports> | all ) state ( enable | disable ) 
 */
cparser_result_t cparser_cmd_qos_set_remarking_dot1p_dscp_outer_tag_port_ports_all_state_enable_disable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable;
    int32   (*fp)(uint32, rtk_port_t, rtk_enable_t); 
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        
        if (0 == strcmp(context->parser->tokens[3].buf, "dot1p"))
        {
            fp = rtk_qos_1pRemarkEnable_set;
        }
        else if (0 == strcmp(context->parser->tokens[3].buf, "dscp"))
        {
            fp = rtk_qos_dscpRemarkEnable_set;
        } 
        else if (0 == strcmp(context->parser->tokens[3].buf, "outer-tag"))
        {
            fp = rtk_qos_out1pRemarkEnable_set;
        }
        else
        {
            return CPARSER_NOT_OK;
        }
        
        if (0 == strcmp(context->parser->tokens[7].buf, "enable"))
        {
            enable = ENABLED;
        }
        else
        {
            enable = DISABLED;
        }
        
        if ((ret = fp(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DEI_PORT_PORTS_ALL_STATE_ENABLE_DISABLE
/*
 * qos set remarking dei port ( <PORT_LIST:ports> | all ) state ( enable | disable ) 
 */
cparser_result_t cparser_cmd_qos_set_remarking_dei_port_ports_all_state_enable_disable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if (0 == strcmp(context->parser->tokens[7].buf, "enable"))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_deiRemarkEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DEI_SYSTEM_DROP_PRECEDENCE_DEI
cparser_result_t cparser_cmd_qos_set_remarking_dei_system_drop_precedence_dei(cparser_context_t *context,
    uint32_t *drop_precedence_ptr,
    uint32_t *dei_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_deiRemark_set(unit, *drop_precedence_ptr, *dei_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DEI_TAG_SELECTOR_PORT_PORTS_ALL_INNER_TAG_OUTER_TAG
/*
 * qos set remarking dei tag-selector port ( <PORT_LIST:ports> | all ) ( inner-tag | outer-tag )
 */
cparser_result_t cparser_cmd_qos_set_remarking_dei_tag_selector_port_ports_all_inner_tag_outer_tag(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_deiSel_t type;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if (0 == strcmp(context->parser->tokens[7].buf, "inner-tag"))
    {
        type = DEI_SEL_INNER_TAG;
    }
    else if (0 == strcmp(context->parser->tokens[7].buf, "outer-tag"))
    {
        type = DEI_SEL_OUTER_TAG;
    } 
    else
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portDEIRemarkTagSel_set(unit, port, type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DOT1P_SYSTEM_INTER_PRIORITY_DOT1P_PRIORITY
/*
 * qos set remarking dot1p system <UINT:inter_priority> <UINT:dot1p_priority>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dot1p_system_inter_priority_dot1p_priority(cparser_context_t *context,
    uint32_t *inter_priority_ptr,
    uint32_t *dot1p_priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_1pRemark_set(unit, *inter_priority_ptr, *dot1p_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DOT1P_SYSTEM_SOURCE_INT_PRI_DOT1P
/*
 * qos set remarking dot1p system source ( int-pri | dot1p )
 */
cparser_result_t cparser_cmd_qos_set_remarking_dot1p_system_source_int_pri_dot1p(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_1pRmkSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    
    if (0 == strcmp(context->parser->tokens[6].buf, "int-pri"))
    {
        type = PRI_SRC_INT_PRI;
    }
    else
    {
        type = PRI_SRC_INNER_USER_PRI;
    }

    if ((ret = rtk_qos_1pRemarkSrcSel_set(unit, type)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_DEFAULT_PRIORITY_INNER_TAG_SYSTEM_PRIORITY
/*
 * qos set default-priority inner-tag system <UINT:priority>
 */
cparser_result_t cparser_cmd_qos_set_default_priority_inner_tag_system_priority(cparser_context_t *context,
    uint32_t *priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   dot1p_pri;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    dot1p_pri = *priority_ptr;
    if ((ret = rtk_qos_1pDfltPri_set(unit, dot1p_pri)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}    
#endif

#ifdef CMD_QOS_SET_REMARKING_DOT1P_GROUP_GROUP_ID_INTER_PRIORITY_DROP_PRECEDENCE_DOT1P_PRIORITY
/*
 * qos set remarking dot1p group <UINT:group_id> <UINT:inter_priority> <UINT:drop_precedence> <UINT:dot1p_priority>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dot1p_group_group_id_inter_priority_drop_precedence_dot1p_priority(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *inter_priority_ptr,
    uint32_t *drop_precedence_ptr,
    uint32_t *dot1p_priority_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_1pRemarkGroup_set(unit, *group_id_ptr, *inter_priority_ptr
                                    , *drop_precedence_ptr, *dot1p_priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DOT1P_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set remarking dot1p port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dot1p_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_port1pRemarkGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_OUTER_TAG_GROUP_GROUP_ID_INTERNAL_PRIORITY_DROP_PRECEDENCE_PRIORITY_DEI
/*
 * qos set remarking outer-tag group <UINT:group_id> <UINT:internal_priority> <UINT:drop_precedence> <UINT:priority> <UINT:dei>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dot1p_pri_map_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_port1pPriMapGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_OUTER_TAG_GROUP_GROUP_ID_INTERNAL_PRIORITY_DROP_PRECEDENCE_PRIORITY_DEI
/*
 * qos set remarking outer-tag group <UINT:group_id> <UINT:internal_priority> <UINT:drop_precedence> <UINT:priority> <UINT:dei>
 */
cparser_result_t cparser_cmd_qos_set_remarking_outer_tag_group_group_id_internal_priority_drop_precedence_priority_dei(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *internal_priority_ptr,
    uint32_t *drop_precedence_ptr,
    uint32_t *priority_ptr,
    uint32_t *dei_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_outer1pRemarkGroup_set(unit, *group_id_ptr, *internal_priority_ptr, *drop_precedence_ptr
                                    , *priority_ptr, *dei_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_OUTER_TAG_SYSTEM_SOURCE_INT_PRI_OUTER_PRI
/*
 * qos set remarking outer-tag system source ( int-pri | outer-pri ) 
 */
cparser_result_t cparser_cmd_qos_set_remarking_outer_tag_system_source_int_pri_outer_pri(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_outer1pRmkSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    
    if (0 == strcmp(context->parser->tokens[6].buf, "int-pri"))
    {
        type = PRI_SRC_INT_PRI;
    }
    else
    {
        type = PRI_SRC_OUTER_USER_PRI;
    }

    if ((ret = rtk_qos_outer1pRemarkSrcSel_set(unit, type)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_OUTER_TAG_SYSTEM_INTERNAL_PRIORITY_PRIORITY
/*
 * qos set remarking outer-tag system <UINT:internal_priority> <UINT:priority> 
 */
cparser_result_t cparser_cmd_qos_set_remarking_outer_tag_system_internal_priority_priority(cparser_context_t *context,
    uint32_t *internal_priority_ptr,
    uint32_t *priority_ptr)
    
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_outer1pRemark_set(unit, *internal_priority_ptr, *priority_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#if 0 //def CMD_QOS_SET_REMARKING_OUTER_TAG_PORT_PORTS_ALL_DEFAULT_PRIORITY
/*
 * qos set remarking outer-tag port ( <PORT_LIST:ports> | all ) default <UINT:priority> 
 */
cparser_result_t cparser_cmd_qos_set_remarking_outer_tag_port_ports_all_default_priority(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_outer1pRmkSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    
    if (0 == strcmp(context->parser->tokens[6].buf, "int-pri"))
    {
        type = PRI_SRC_INT_PRI;
    }
    else
    {
        type = PRI_SRC_OUTER_USER_PRI;
    }

    if ((ret = rtk_qos_outer1pRemarkSrcSel_set(unit, type)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_OUTER_TAG_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set remarking outer-tag port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_remarking_outer_tag_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portOuter1pRemarkGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_OUTER_TAG_PRI_MAP_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set remarking outer-tag pri-map port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_remarking_outer_tag_pri_map_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portOuter1pPriMapGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DSCP_SYSTEM_INTER_PRIORITY_DSCP
/*
 * qos set remarking dscp system <UINT:inter_priority> <UINT:dscp>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dscp_system_inter_priority_dscp(cparser_context_t *context,
    uint32_t *inter_priority_ptr,
    uint32_t *dscp_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_dscpRemark_set(unit, *inter_priority_ptr, *dscp_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DSCP_SYSTEM_ORIGINAL_DSCP_DSCP
/*
 * qos set remarking dscp system <UINT:original_dscp> <UINT:dscp>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dscp_system_original_dscp_dscp(cparser_context_t *context,
    uint32_t *original_dscp_ptr,
    uint32_t *dscp_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_dscp2DscpRemark_set(unit, *original_dscp_ptr, *dscp_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DSCP_SYSTEM_SOURCE_INT_PRI_DSCP_DP
/*
 * qos set remarking dscp system source ( int-pri | dscp | dp )
 */
cparser_result_t cparser_cmd_qos_set_remarking_dscp_system_source_int_pri_dscp_dp(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_dscpRmkSrc_t   type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    
    if (0 == strcmp(context->parser->tokens[6].buf, "int-pri"))
    {
        type = PRI_SRC_INT_PRI;
    }
    else if (0 == strcmp(context->parser->tokens[6].buf, "dscp"))
    {
        type = PRI_SRC_DSCP;
    }
    else
    {
        type = PRI_SRC_DP;
    }

    if ((ret = rtk_qos_dscpRemarkSrcSel_set(unit, type)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_DEFAULT_PRIORITY_OUTER_TAG_PORT_PORTS_ALL_COPY_INNER_PRI_COPY_INTER_PRI
/*  
 * qos set default-priority outer-tag port ( <PORT_LIST:ports> | all ) ( copy-inner-pri | copy-inter-pri )
 */
cparser_result_t cparser_cmd_qos_set_default_priority_outer_tag_port_ports_all_copy_inner_pri_copy_inter_pri(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_outer1pDfltSrc_t type;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }

    if (0 == strcmp(context->parser->tokens[6].buf, "copy-inner-pri"))
    {
        type = OUTER_1P_DFLT_SRC_USER_PRI;
    }
    else if (0 == strcmp(context->parser->tokens[6].buf, "copy-inter-pri"))
    {
        type = OUTER_1P_DFLT_SRC_INT_PRI;
    }
    else
    {
        type = CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portOuter1pDfltPriSrcSel_set(unit, port, type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DSCP_GROUP_GROUP_ID_INTER_PRIORITY_DROP_PRECEDENCE_DSCP
/*
 * qos set remarking dscp group <UINT:group_id> <UINT:inter_priority> <UINT:drop_precedence> <UINT:dscp>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dscp_group_group_id_inter_priority_drop_precedence_dscp(cparser_context_t *context,
    uint32_t *group_id_ptr,
    uint32_t *inter_priority_ptr,
    uint32_t *drop_precedence_ptr,
    uint32_t *dscp_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_dscpRemarkGroup_set(unit, *group_id_ptr, *inter_priority_ptr
                                    , *drop_precedence_ptr, *dscp_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_REMARKING_DSCP_PORT_GROUP_PORTS_ALL_GROUP_ID
/*
 * qos set remarking dscp port-group ( <PORT_LIST:ports> | all ) <UINT:group_id>
 */
cparser_result_t cparser_cmd_qos_set_remarking_dscp_port_group_ports_all_group_id(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portdscpRemarkGroup_set(unit, port, *group_id_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_SCHEDULING_ALGORITHM_PORT_PORTS_ALL_WRR_WFQ
/*
 * qos set scheduling algorithm port ( <PORT_LIST:ports> | all ) ( wrr | wfq )
 */
cparser_result_t cparser_cmd_qos_set_scheduling_algorithm_port_ports_all_wrr_wfq(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_scheduling_type_t   algo;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (0 == strcmp(context->parser->tokens[6].buf, "wrr"))
        {
            algo = WRR;
        }
        else
        {
            algo = WFQ;
        }
        
        if ((ret = rtk_qos_schedulingAlgorithm_set(unit, port, algo)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_SCHEDULING_WFQ_FIXED_RATE_PORTS_ALL_QUEUE_ID_STATE_ENABLE_DISABLE
/*
 * qos set scheduling wfq-fixed-rate ( <PORT_LIST:ports> | all ) <UINT:queue_id> state ( enable | disable )
 */
cparser_result_t cparser_cmd_qos_set_scheduling_wfq_fixed_rate_ports_all_queue_id_state_enable_disable(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *queue_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    rtk_enable_t    enable;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        
        if (0 == strcmp(context->parser->tokens[7].buf, "enable"))
        {
            enable = ENABLED;
        }
        else
        {
            enable = DISABLED;
        }
        
        if ((ret = rtk_qos_wfqFixedBandwidthEnable_set(unit, port, *queue_id_ptr, enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_SCHEDULING_QUEUE_WEIGHT_PORTS_ALL_QUEUE_ID_WEIGHT
/*
 * qos set scheduling queue-weight ( <PORT_LIST:ports> | all ) <UINT:queue_id> <UINT:weight>
 */
cparser_result_t cparser_cmd_qos_set_scheduling_queue_weight_ports_all_queue_id_weight(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *queue_id_ptr,
    uint32_t *weight_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_queue_weights_t queue_weight;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_schedulingQueue_get(unit, port, &queue_weight)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        queue_weight.weights[*queue_id_ptr] = *weight_ptr;
        
        if ((ret = rtk_qos_schedulingQueue_set(unit, port, &queue_weight)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_ALGORITHM_WRED_WTD
/*
 * qos set congest-avoidance algorithm ( wred | wtd )
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_algorithm_wred_wtd(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_congAvoidAlgo_t algo;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (0 == strcmp(context->parser->tokens[4].buf, "wred"))
    {
        algo = CONG_AVOID_WRED;
    }
    else
    {
        algo = CONG_AVOID_WTD;
    }
    
    if ((ret = rtk_qos_congAvoidAlgo_set(unit, algo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_ALGORITHM_SWRED
/*
 * qos set congest-avoidance algorithm swred
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_algorithm_swred(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_congAvoidAlgo_t algo;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    algo = CONG_AVOID_SWRED;
    
    if ((ret = rtk_qos_congAvoidAlgo_set(unit, algo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_ALGORITHM_TD
/*
 * qos set congest-avoidance algorithm td
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_algorithm_td(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_congAvoidAlgo_t algo;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    algo = CONG_AVOID_TD;
    
    if ((ret = rtk_qos_congAvoidAlgo_set(unit, algo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_THRESHOLD_SYSTEM_PORT_QUEUE_STATE_ENABLE_DISABLE
/*
 * qos set congest-avoidance threshold ( system | port | queue ) state ( enable | disable )
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_threshold_system_port_queue_state_enable_disable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable;
    int32   (*fp)(uint32, rtk_enable_t); 

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (0 == strcmp(context->parser->tokens[4].buf, "system"))
    {
        fp = rtk_qos_congAvoidSysThreshEnable_set;
    }
    else if (0 == strcmp(context->parser->tokens[4].buf, "port"))
    {
        fp = rtk_qos_congAvoidPortThreshEnable_set;
    } 
    else
    {
        fp = rtk_qos_congAvoidQueueThreshEnable_set;
    }
    
    if (0 == strcmp(context->parser->tokens[6].buf, "enable"))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
    
    if ((ret = fp(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_SYSTEM_THRESHOLD_DROP_PRECEDENCE_MAX_THRESHOLD_MIN_THRESHOLD
/*
 * qos set congest-avoidance system-threshold <UINT:drop_precedence> <UINT:max_threshold> <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_system_threshold_drop_precedence_max_threshold_min_threshold(cparser_context_t *context,
    uint32_t *drop_precedence_ptr,
    uint32_t *max_threshold_ptr,
    uint32_t *min_threshold_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_congAvoidThresh_t   thresh;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;
    
    if ((ret = rtk_qos_congAvoidSysThresh_set(unit, *drop_precedence_ptr, &thresh)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_PORT_THRESHOLD_PORTS_ALL_DROP_PRECEDENCE_MAX_THRESHOLD_MIN_THRESHOLD
/*
 * qos set congest-avoidance port-threshold ( <PORT_LIST:ports> | all ) <UINT:drop_precedence> <UINT:max_threshold> <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_port_threshold_ports_all_drop_precedence_max_threshold_min_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *drop_precedence_ptr,
    uint32_t *max_threshold_ptr,
    uint32_t *min_threshold_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_congAvoidThresh_t   thresh;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_congAvoidPortThresh_set(unit, port, *drop_precedence_ptr, &thresh)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_PORTS_ALL_QUEUE_MAX_THRESHOLD_MIN_THRESHOLD
/*
 * qos set congest-avoidance queue-threshold ( <PORT_LIST:ports> | all ) <UINT:queue> <UINT:max_threshold> <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_qos_set_congest_avoidance_queue_threshold_ports_all_queue_max_threshold_min_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *queue_ptr,
    uint32_t *max_threshold_ptr,
    uint32_t *min_threshold_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_congAvoidThresh_t   thresh;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_congAvoidQueueThresh_set(unit, port, *queue_ptr, &thresh)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_WRED_SYSTEM_THRESHOLD_DROP_PRECEDENCE_MAX_THRESHOLD_MIN_THRESHOLD
/*
 * qos set wred system-threshold <UINT:drop_precedence> <UINT:max_threshold> <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_qos_set_wred_system_threshold_drop_precedence_max_threshold_min_threshold(cparser_context_t *context,
    uint32_t *drop_precedence_ptr,
    uint32_t *max_threshold_ptr,
    uint32_t *min_threshold_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_wredThresh_t   thresh;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;
    
    if ((ret = rtk_qos_wredSysThresh_set(unit, *drop_precedence_ptr, &thresh)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif /* CMD_QOS_SET_WRED_SYSTEM_THRESHOLD_DROP_PRECEDENCE_MAX_THRESHOLD_MIN_THRESHOLD */

#ifdef CMD_QOS_SET_WRED_MPD_MPD_VALUE
/*
 * qos set wred mpd <UINT:mpd_value>
 */
cparser_result_t cparser_cmd_qos_set_wred_mpd_mpd_value(cparser_context_t *context,
    uint32_t *mpd_value_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_wredMpd_set(unit, *mpd_value_ptr)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_WRED_ECN_STATE_ENABLE_DISABLE
/*
 * qos set wred ecn state ( enable | disable )
 */
cparser_result_t cparser_cmd_qos_set_wred_ecn_state_enable_disable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (0 == strcmp(context->parser->tokens[5].buf, "enable"))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
    
    if ((ret = rtk_qos_wredEcnEnable_set(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_SET_WRED_COUNTER_REVERSE_STATE_ENABLE_DISABLE
/*
 * qos set wred counter-reverse state ( enable | disable )
 */
cparser_result_t cparser_cmd_qos_set_wred_counter_reverse_state_enable_disable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (0 == strcmp(context->parser->tokens[5].buf, "enable"))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
    
    if ((ret = rtk_qos_wredCntReverseEnable_set(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_PKT2CPU_PRIORITY_REMAP
/*
 * qos get pkt2cpu-priority-remap
 */
cparser_result_t cparser_cmd_qos_get_pkt2cpu_priority_remap(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   int_pri;
    rtk_pri_t   new_pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Packet to CPU Priority Remapping Table\n");
    for (int_pri=0; int_pri<=RTK_DOT1P_PRIORITY_MAX; int_pri++)
    {
        if ((ret = rtk_qos_pkt2CpuPriRemap_get(unit, int_pri, &new_pri)) != RT_ERR_OK)
        {
            diag_util_mprintf("Not Support\n");
            continue;
        }

        diag_util_mprintf("\tInternal Priority %u => %u\n", int_pri, new_pri);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_PRIORITY_SELECTOR_SYSTEM
/*
 * qos get priority-selector system
 */
cparser_result_t cparser_cmd_qos_get_priority_selector_system(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      port_weight;
    uint32      class_weight;
    uint32      acl_weight;
    uint32      dscp_weight;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_priSel_get(unit, &port_weight, &class_weight, &acl_weight, &dscp_weight)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("System Weight Configuration of Priority Selector\n");
    diag_util_mprintf("\tPort based\t: %u\n", port_weight);
    diag_util_mprintf("\tClass based\t: %u\n", class_weight);
    diag_util_mprintf("\tACL based\t: %u\n", acl_weight);
    diag_util_mprintf("\tDSCP based\t: %u\n", dscp_weight);
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_AVB_PORTS_ALL_SR_CLASS_A_SR_CLASS_B_STATE
/*
 * qos get avb ( <PORT_LIST:ports> | all ) ( sr-class-a | sr-class-b ) state
 */
cparser_result_t cparser_cmd_qos_get_avb_ports_all_sr_class_a_sr_class_b_state(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_avbSrClass_t    srClass;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    srClass = ('a' == TOKEN_CHAR(4, 9))? AVB_SR_CLASS_A : AVB_SR_CLASS_B;
    
    diag_util_mprintf("State of AVB SR-Class-%c of Ports \n", (srClass == AVB_SR_CLASS_A)? 'A' : 'B');
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portAvbStreamReservationClassEnable_get(unit, port, srClass, &enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_mprintf("\tPort %2d State : %s\n", port, (enable == ENABLED)? "Enable" : "Disable");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_AVB_SR_CLASS_A_SR_CLASS_B_PRIORITY
/*
 * qos get avb ( sr-class-a | sr-class-b ) priority
 */
cparser_result_t cparser_cmd_qos_get_avb_sr_class_a_sr_class_b_priority(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("\tPriority of AVB SR-Class-%c : %u\n", \
        ('a' == TOKEN_CHAR(3, 9))? 'A' : 'B', \
        ('a' == TOKEN_CHAR(3, 9))? srClassConf.class_a_priority : srClassConf.class_b_priority);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_AVB_SR_CLASS_A_SR_CLASS_B_QUEUE_ID
/*
 * qos get avb ( sr-class-a | sr-class-b ) queue-id
 */
cparser_result_t cparser_cmd_qos_get_avb_sr_class_a_sr_class_b_queue_id(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("\tQueue-ID of AVB SR-Class-%c : %u\n", \
        ('a' == TOKEN_CHAR(3, 9))? 'A' : 'B', \
        ('a' == TOKEN_CHAR(3, 9))? srClassConf.class_a_queue_id : srClassConf.class_b_queue_id);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_AVB_NON_SR_CLASS_A_NON_SR_CLASS_B_REDIRECT_QUEUE_ID
/*
 * qos get avb ( non-sr-class-a | non-sr-class-b ) redirect-queue-id
 */
cparser_result_t cparser_cmd_qos_get_avb_non_sr_class_a_non_sr_class_b_redirect_queue_id(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("\tRedirect Queue-ID of AVB Non-SR-Class-%c : %u\n", \
        ('a' == TOKEN_CHAR(3, 13))? 'A' : 'B', \
        ('a' == TOKEN_CHAR(3, 13))? \
            srClassConf.class_non_a_redirect_queue_id : \
            srClassConf.class_non_b_redirect_queue_id);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_AVB_NON_SR_CLASS_A_NON_SR_CLASS_B_REMARK_PRIORITY
/*
 * qos get avb ( non-sr-class-a | non-sr-class-b ) remark-priority
 */
cparser_result_t cparser_cmd_qos_get_avb_non_sr_class_a_non_sr_class_b_remark_priority(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_avbSrConf_t srClassConf;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_avbStreamReservationConfig_get(unit, &srClassConf)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("\tRemarking Priority of AVB Non-SR-Class-%c : %u\n", \
        ('a' == TOKEN_CHAR(3, 13))? 'A' : 'B', \
        ('a' == TOKEN_CHAR(3, 13))? \
            srClassConf.class_non_a_remark_priority : \
            srClassConf.class_non_b_remark_priority);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_DP_SELECTOR
/*
 * qos get dp-selector
 */
cparser_result_t cparser_cmd_qos_get_dp_selector(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_qos_dpSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_dpSrcSel_get(unit, &type)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (DP_SRC_DEI_BASED == type)
    {
        diag_util_mprintf("Drop Precedence Source : DEI-based\n");
    }
    else if (DP_SRC_DSCP_BASED == type)
    {
        diag_util_mprintf("Drop Precedence Source : DSCP-based\n");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_PRIORITY_SELECTOR_GROUP_GROUP_ID
/*
 * qos get priority-selector group { <UINT:group_id> }
 */
cparser_result_t cparser_cmd_qos_get_priority_selector_group_group_id(cparser_context_t *context,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      group_id;
    uint32      group_id_max;
    rtk_qos_priSelWeight_t   priSelWeight;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (5 == context->parser->cmd_tokens )
    {
        group_id        = *group_id_ptr;
        group_id_max    = *group_id_ptr;
    }
    else
    {
        group_id        = 0;
        DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, pri_sel_group_index_max);
    }
    
    for (;group_id <= group_id_max; group_id++)
    {
        diag_util_mprintf("Group %u Priority Selector\n", group_id);
        if ((ret = rtk_qos_priSelGroup_get(unit, group_id, &priSelWeight)) != RT_ERR_OK)
        {
            diag_util_mprintf("Not Support\n");
            continue;
        }
        
        if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID))
        {
            diag_util_mprintf("\tPort-based               : %u\n", priSelWeight.weight_of_portBased);
            diag_util_mprintf("\tDSCP-based               : %u\n", priSelWeight.weight_of_dscp);
#if defined(CONFIG_SDK_RTL8390)
            diag_util_mprintf("\tIngress-ACL-based        : %u\n", priSelWeight.weight_of_inAcl);
#endif
            diag_util_mprintf("\tInner-tag-based          : %u\n", priSelWeight.weight_of_innerTag);
            diag_util_mprintf("\tOuter-tag-based          : %u\n", priSelWeight.weight_of_outerTag);
#if defined(CONFIG_SDK_RTL8390)
            diag_util_mprintf("\tMAC-VLAN/IP-subnet-based : %u\n", priSelWeight.weight_of_macVlan);
            diag_util_mprintf("\tProtocol-and-port-based  : %u\n", priSelWeight.weight_of_protoVlan);
#endif
        }
        else
        {
            diag_util_mprintf("\tPort based   \t: %u\n", priSelWeight.weight_of_portBased);
#if defined(CONFIG_SDK_RTL8380)
            diag_util_mprintf("\t802.1q       \t: %u\n", priSelWeight.weight_of_dot1q);
#endif
            diag_util_mprintf("\tDSCP based   \t: %u\n", priSelWeight.weight_of_dscp);
#if defined(CONFIG_SDK_RTL8380)
            diag_util_mprintf("\tFlow classify\t: %u\n", priSelWeight.weight_of_flowBased);
#endif
            diag_util_mprintf("\tInner tag    \t: %u\n", priSelWeight.weight_of_innerTag);
            diag_util_mprintf("\tOuter tag    \t: %u\n", priSelWeight.weight_of_outerTag);
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_PRIORITY_SELECTOR_PORT_GROUP_PORTS_ALL
/*
 * qos get priority-selector port-group ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_priority_selector_port_group_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      group_id;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Priority Selector Group of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portPriSelGroup_get(unit, port, &group_id)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_mprintf("\tPort %2d Group : %u\n", port, group_id);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_QUEUE_NUMBER
/*
 * qos get queue number
 */
cparser_result_t cparser_cmd_qos_get_queue_number(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      queue_num;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_qos_queueNum_get(unit, &queue_num)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Qos Queue Number : %u \n", queue_num);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_QUEUE_MAPPING_QUEUE_NUM
/*
 * qos get queue mapping { <UINT:queue_num> }
 */
cparser_result_t cparser_cmd_qos_get_queue_mapping_queue_num(cparser_context_t *context,
    uint32_t *queue_num_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_pri_t   pri_max;
    uint32      queue_num;
    uint32      queue_num_max;
    rtk_qos_pri2queue_t   pri2Qid;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (5 == context->parser->cmd_tokens )
    {
        queue_num        = *queue_num_ptr;
        queue_num_max    = *queue_num_ptr;
    }
    else
    {
        queue_num        = 1;
        DIAG_OM_GET_CHIP_CAPACITY(unit, queue_num_max, max_num_of_queue);
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    for (;queue_num <= queue_num_max; queue_num++)
    {
        diag_util_mprintf("Queue Number %u Priority Queue Mapping\n", queue_num);
        diag_util_mprintf("Internal Priority\t  Queue ID\n");
        if ((ret = rtk_qos_priMap_get(unit, queue_num, &pri2Qid)) != RT_ERR_OK)
        {
            diag_util_mprintf("Not Support\n");
            continue;
        }
        
        for (pri = 0; pri <= pri_max; pri++)
        {
            diag_util_mprintf("Priority %u : %u\n", pri, pri2Qid.pri2queue[pri]);
        }
        
        diag_util_mprintf("\n");
    }
    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_QUEUE_MAPPING_PORT_PORTS_ALL
/*
 * qos get queue mapping port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_queue_mapping_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_pri_t   int_pri;
    rtk_pri_t   pri_max;
    rtk_qid_t   queue;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    diag_util_mprintf("Queue Mapping Configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\tPort %u Queue Mapping Configuration : \n", port);
        
        for (int_pri = 0; int_pri <= pri_max; int_pri++)
        {
            diag_util_printf("\t  Priority %u : ", int_pri);
            if (RT_ERR_OK == rtk_qos_portPriMap_get(unit, port, int_pri, &queue))
            {
                diag_util_mprintf("Queue %u\n", queue);
            }
            else
            {
                diag_util_mprintf("Not Support\n");
            }
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DEI_SYSTEM       
/*
 * qos get remapping dei system
 */
cparser_result_t cparser_cmd_qos_get_remapping_dei_system(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    uint32      dei, dp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("System DEI Remapping Configuration \n");
    
    diag_util_mprintf("DEI Value\t  Drop Precedence\n");
    for (dei = RTK_VALUE_OF_DEI_MIN; dei <= RTK_VALUE_OF_DEI_MAX; dei++)
    {
        diag_util_printf("DEI %u : ", dei);
        if (RT_ERR_OK == rtk_qos_deiDpRemap_get(unit, dei, &dp))
        {
            diag_util_mprintf("%u\n", dp);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DEI_TAG_SELECTOR_PORT_PORTS_ALL       
/*
 * qos get remapping dei tag-selector port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remapping_dei_tag_selector_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_qos_deiSel_t type;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("DEI Remapping Tag Selector Configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (RT_ERR_OK == rtk_qos_portDEISrcSel_get(unit, port, &type))
        {
            diag_util_mprintf("Port %2d : %s\n", port, (type == DEI_SEL_INNER_TAG) ? "Inner-tag" : "Outer-tag");
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_PORT_PORTS_ALL
/*
 * qos get remapping port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remapping_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
#if defined(CONFIG_SDK_RTL8390)
    uint32      unit = 0;
    rtk_pri_t   int_pri;
    rtk_port_t  port;
    diag_portlist_t  portlist;
#else    
    uint32      unit = 0;
    rtk_pri_t   int_pri;
    rtk_pri_t   inner_pri, outer_pri;
    uint32      dp;
    uint32      dei;
    rtk_port_t  port;
    diag_portlist_t  portlist;
#endif

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Port Based Priority Configuration \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
#if defined(CONFIG_SDK_RTL8390)
        if (RT_ERR_OK == rtk_qos_portPri_get(unit, port, &int_pri))
        {
            diag_util_mprintf("Port %2d : %u\n", port, int_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
#else
        diag_util_mprintf("Port %2d priority configuration : \n", port);
        diag_util_printf("\tInternal priority\t: ");
        if (RT_ERR_OK == rtk_qos_portPri_get(unit, port, &int_pri))
        {
            diag_util_mprintf("%u\n", int_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
        
        diag_util_printf("\tDrop precedence\t\t: ");
        if (RT_ERR_OK == rtk_qos_portDp_get(unit, port, &dp))
        {
            diag_util_mprintf("%u\n", dp);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
        
        diag_util_printf("\tPriority of inner tag\t: ");
        if (RT_ERR_OK == rtk_qos_portInnerPri_get(unit, port, &inner_pri))
        {
            diag_util_mprintf("%u\n", inner_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
        
        diag_util_printf("\tPriority of outer tag\t: ");
        if (RT_ERR_OK == rtk_qos_portOuterPri_get(unit, port, &outer_pri))
        {
            diag_util_mprintf("%u\n", outer_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
        
        diag_util_printf("\tDEI of outer tag\t: ");
        if (RT_ERR_OK == rtk_qos_portOuterDEI_get(unit, port, &dei))
        {
            diag_util_mprintf("%u\n", dei);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
#endif        
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DOT1P_SYSTEM
/*
 * qos get remapping dot1p system
 */
cparser_result_t cparser_cmd_qos_get_remapping_dot1p_system(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_pri_t   dot1p_pri;
    rtk_pri_t   int_pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("System 802.1p Priority Remapping Configuration \n");
    
    diag_util_mprintf("802.1p Priority\t  Internal Priority\n");
    for (dot1p_pri = 0; dot1p_pri <= RTK_DOT1P_PRIORITY_MAX; dot1p_pri++)
    {
        diag_util_printf("Priority %u : ", dot1p_pri);
        if (RT_ERR_OK == rtk_qos_1pPriRemap_get(unit, dot1p_pri, &int_pri))
        {
            diag_util_mprintf("%u\n", int_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DOT1P_GROUP_GROUP_ID
/*
 * qos get remapping dot1p group { <UINT:group_id> }
 */
cparser_result_t cparser_cmd_qos_get_remapping_dot1p_group_group_id(cparser_context_t *context,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_pri_t   pri_max;
    uint32      group_id;
    uint32      group_id_max;
    rtk_pri_t   int_pri;
    uint32      dp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (6 == context->parser->cmd_tokens )
    {
        group_id        = *group_id_ptr;
        group_id_max    = *group_id_ptr;
    }
    else
    {
        group_id        = 0;
        DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, priority_remap_group_idx_max);
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    for (;group_id <= group_id_max; group_id++)
    {
        diag_util_mprintf("Group %u 802.1p priority remapping\n", group_id);
        
        
        for (pri = 0; pri <= pri_max; pri++)
        {
            diag_util_mprintf("  Priority %u\n", pri);
            if ((ret = rtk_qos_1pPriRemapGroup_get(unit, group_id, pri, &int_pri, &dp)) != RT_ERR_OK)
            {
                diag_util_mprintf("  Not support\n");
                continue;
            }
            diag_util_mprintf("    internal priority : %u, drop precedence : %u\n", int_pri, dp);
        }
        
        diag_util_mprintf("\n");
    }
    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DOT1P_PORT_GROUP_PORTS_ALL
/*
 * qos get remapping dot1p port-group ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remapping_dot1p_port_group_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      group_id;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("802.1p remapping group of ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_port1pPriRemapGroup_get(unit, port, &group_id)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_mprintf("\tPort %u group\t: %u\n", port, group_id);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DSCP_SYSTEM
/*
 * qos get remapping dscp system
 */
cparser_result_t cparser_cmd_qos_get_remapping_dscp_system(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      dscp, dp;
    rtk_pri_t   int_pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("System DSCP Priority Remapping Configuration \n");
    
    diag_util_mprintf("DSCP priority\t  Internal Priority\t  Drop Precedence\n");
    for (dscp = RTK_VALUE_OF_DSCP_MIN; dscp <= RTK_VALUE_OF_DSCP_MAX; dscp++)
    {
        diag_util_printf("DSCP %2d : ", dscp);
        if (RT_ERR_OK == rtk_qos_dscpPriRemap_get(unit, dscp, &int_pri))
        {
            diag_util_mprintf("%u\t", int_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\t");
        }
        
        if (RT_ERR_OK == rtk_qos_dscpDpRemap_get(unit, dscp, &dp))
        {
            diag_util_mprintf("%u\n", dp);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DSCP_GROUP_GROUP_ID
/*
 * qos get remapping dscp group { <UINT:group_id> }
 */
cparser_result_t cparser_cmd_qos_get_remapping_dscp_group_group_id(cparser_context_t *context,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      dscp;
    uint32      group_id;
    uint32      group_id_max;
    rtk_pri_t   int_pri;
    uint32      dp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (6 == context->parser->cmd_tokens )
    {
        group_id        = *group_id_ptr;
        group_id_max    = *group_id_ptr;
    }
    else
    {
        group_id        = 0;
        DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, priority_remap_group_idx_max);
    }
    
    for (;group_id <= group_id_max; group_id++)
    {
        diag_util_mprintf("Group %u DSCP priority remapping\n", group_id);
        
        
        for (dscp = 0; dscp <= RTK_VALUE_OF_DSCP_MAX; dscp++)
        {
            diag_util_mprintf("  DSCP %u\n", dscp);
            if ((ret = rtk_qos_dscpPriRemapGroup_get(unit, group_id, dscp, &int_pri, &dp)) != RT_ERR_OK)
            {
                diag_util_mprintf("  Not support\n");
                continue;
            }
            diag_util_mprintf("    internal priority : %u, drop precedence : %u\n", int_pri, dp);
        }
        
        diag_util_mprintf("\n");
    }
    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_DSCP_PORT_GROUP_PORTS_ALL
/*
 * qos get remapping dscp port-group ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remapping_dscp_port_group_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      group_id;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("DSCP remapping group of ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portDscpPriRemapGroup_get(unit, port, &group_id)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_mprintf("\tPort %u group\t: %u\n", port, group_id);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_OUTER_TAG_GROUP_GROUP_ID
/*
 * qos get remapping outer-tag group { <UINT:group_id> }
 */
cparser_result_t cparser_cmd_qos_get_remapping_outer_tag_group_group_id(cparser_context_t *context,
    uint32_t *group_id_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_pri_t   pri_max;
    uint32      dei;
    uint32      group_id;
    uint32      group_id_max;
    rtk_pri_t   int_pri;
    uint32      dp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (6 == context->parser->cmd_tokens )
    {
        group_id        = *group_id_ptr;
        group_id_max    = *group_id_ptr;
    }
    else
    {
        group_id        = 0;
        DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, priority_remap_group_idx_max);
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    for (;group_id <= group_id_max; group_id++)
    {
        diag_util_mprintf("Group %u outer tag priority remapping\n", group_id);
        
        for (pri = 0; pri <= pri_max; pri++)
        {
            for(dei = 0; dei <= RTK_DOT1P_DEI_MAX; dei++)
            {
                diag_util_mprintf("  Priority %u DEI %u\n", pri, dei);
                if ((ret = rtk_qos_outer1pPriRemapGroup_get(unit, group_id, pri, dei, &int_pri, &dp)) != RT_ERR_OK)
                {
                    diag_util_mprintf("  Not support\n");
                    continue;
                }
                diag_util_mprintf("    internal priority : %u, drop precedence : %u\n", int_pri, dp);
            }
        }
        
        diag_util_mprintf("\n");
    }
    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_OUTER_TAG_PORT_GROUP_PORTS_ALL
/*
 * qos get remapping outer-tag port-group ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remapping_outer_tag_port_group_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      group_id;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Outer tag priority remapping group of ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_portOuter1pPriRemapGroup_get(unit, port, &group_id)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_mprintf("\tPort %u group\t: %u\n", port, group_id);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMAPPING_OUTER_TAG_SYSTEM
/*
 * qos get remapping outer-tag system
 */
cparser_result_t cparser_cmd_qos_get_remapping_outer_tag_system(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_pri_t   dot1p_pri;
    rtk_pri_t   int_pri;
    uint32      dei;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    for (dei = 0; dei <= RTK_DOT1P_DEI_MAX; dei++)
    {
        diag_util_printf("DEI %u Outer Priority Remapping Configuration\n", dei);
        diag_util_mprintf("Outer Priority\t  Internal Priority\n");
        for (dot1p_pri = 0; dot1p_pri <= RTK_DOT1P_PRIORITY_MAX; dot1p_pri++)
        {
            diag_util_printf("Priority %u : ", dot1p_pri);
            if (RT_ERR_OK == rtk_qos_outer1pPriRemap_get(unit, dot1p_pri, dei, &int_pri))
            {
                diag_util_mprintf("%u\n", int_pri);
            }
            else
            {
                diag_util_mprintf("Not Support\n");
            }
        }
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DEI_SYSTEM
/*
 * qos get remarking dei system
 */
cparser_result_t cparser_cmd_qos_get_remarking_dei_system(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      dp;
    uint32      dei;
    uint32      dp_max;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, dp_max, drop_precedence_max);

    diag_util_mprintf("Drop Precedence\t  DEI Value\n");
    for (dp = 0; dp <= dp_max; dp++)
    {
        if (RT_ERR_OK == rtk_qos_deiRemark_get(unit, dp, &dei))
        {
            diag_util_mprintf("DP %u : %u\n", dp, dei);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DEI_TAG_SELECTOR_PORT_PORTS_ALL
/*
 * qos get remarking dei tag-selector port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remarking_dei_tag_selector_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_deiSel_t type;
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("DEI Remarking Tag Selection of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (RT_ERR_OK == rtk_qos_portDEIRemarkTagSel_get(unit, port, &type))
        {
            if (DEI_SEL_INNER_TAG == type)
            {
                diag_util_mprintf("Port %2d : Inner-tag\n", port);
            }
            else
            {
                diag_util_mprintf("Port %2d : Outer-tag\n", port);
            }
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DOT1P_SYSTEM
/*
 * qos get remarking dot1p system
 */
cparser_result_t cparser_cmd_qos_get_remarking_dot1p_system(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_pri_t   dot1p_pri;
    rtk_pri_t   int_pri;
    rtk_pri_t   pri_max;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    diag_util_mprintf("Internal Priority\t  802.1p Priority\n");
    for (int_pri = 0; int_pri <= pri_max; int_pri++)
    {
        if (RT_ERR_OK == rtk_qos_1pRemark_get(unit, int_pri, &dot1p_pri))
        {
            diag_util_printf("Priority %u : ", int_pri);
            diag_util_mprintf("%u\n", dot1p_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DOT1P_SYSTEM_SOURCE
/*
 * qos get remarking dot1p system source
 */
cparser_result_t cparser_cmd_qos_get_remarking_dot1p_system_source(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_1pRmkSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (RT_ERR_OK == rtk_qos_1pRemarkSrcSel_get(unit, &type))
    {
        diag_util_mprintf("Remarking Source : %s\n", (type == PRI_SRC_INT_PRI) ? "Internal priority" : "Original inner-tag priority");
    }
    else
    {
        diag_util_mprintf("Not Support\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_DEFAULT_PRIORITY_INNER_TAG_SYSTEM
/*
 * qos get default-priority inner-tag system
 */
cparser_result_t cparser_cmd_qos_get_default_priority_inner_tag_system(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_pri_t   dot1p_pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Default Inner Priority Configuration of System \n");
    if (RT_ERR_OK == rtk_qos_1pDfltPri_get(unit, &dot1p_pri))
    {
        diag_util_mprintf("Default Priority : %d\n", dot1p_pri);
    }
    else
    {
        diag_util_mprintf("Not Support\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DOT1P_GROUP_GROUP
/*
 * qos get remarking dot1p group { <UINT:group> }
 */
cparser_result_t cparser_cmd_qos_get_remarking_dot1p_group_group(cparser_context_t *context,
    uint32_t *group_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_pri_t   pri_max;
    uint32      group_id;
    uint32      group_id_max;
    rtk_pri_t   int_pri;
    uint32      dp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (6 == context->parser->cmd_tokens )
    {
        group_id        = *group_ptr;
        group_id_max    = *group_ptr;
    }
    else
    {
        group_id        = 0;
        DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, priority_remark_group_idx_max);
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    for (;group_id <= group_id_max; group_id++)
    {
        diag_util_mprintf("Group %u 802.1p priority remarking\n", group_id);
        
        
        for (int_pri = 0; int_pri <= pri_max; int_pri++)
        {
            diag_util_mprintf("  Internal priority %u\n", int_pri);
            
            for (dp = 0; dp <= RTK_DROP_PRECEDENCE_MAX; dp++)
            {
                if ((ret = rtk_qos_1pRemarkGroup_get(unit, group_id, int_pri, dp, &pri)) != RT_ERR_OK)
                {
                    diag_util_mprintf("  Not support\n");
                    continue;
                }
                diag_util_mprintf("    Drop precedence %u : %u\n", dp, pri);
            }
        }
        
        diag_util_mprintf("\n");
    }
    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DOT1P_PORT_PORTS_ALL
/*
 * qos get remarking dot1p port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remarking_dot1p_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_enable_t    enable;
    uint32      group_id;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("802.1p priority remarking configuration of ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %u configuration: \n", port);
        
        diag_util_printf("\tStatus\t\t\t: ");
        if (RT_ERR_OK == rtk_qos_1pRemarkEnable_get(unit, port, &enable))
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
        
        diag_util_printf("\tRemarking group\t\t: ");
        if (RT_ERR_OK == rtk_qos_port1pRemarkGroup_get(unit, port, &group_id))
        {
            diag_util_mprintf("%u\n", group_id);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
        
        diag_util_printf("\tPriority mapping group\t: ");
        if (RT_ERR_OK == rtk_qos_port1pPriMapGroup_get(unit, port, &group_id))
        {
            diag_util_mprintf("%u\n", group_id);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DSCP_SYSTEM
/*
 * qos get remarking dscp system
 */
cparser_result_t cparser_cmd_qos_get_remarking_dscp_system(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      dscp;
    rtk_pri_t   int_pri;
    rtk_pri_t   pri_max;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    diag_util_mprintf("System DSCP Remarking Configuration \n");

    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        rtk_qos_dscpRmkSrc_t src;

        if (RT_ERR_OK == rtk_qos_dscpRemarkSrcSel_get(unit, &src))
        {
            if (PRI_SRC_INT_PRI == src)
            {
                diag_util_mprintf("Interal Priority\t  DSCP Value\n");
                for (int_pri = 0; int_pri <= pri_max; int_pri++)
                {
                    diag_util_printf("Priority %u : ", int_pri);
                    if (RT_ERR_OK == rtk_qos_dscpRemark_get(unit, int_pri, &dscp))
                    {
                        diag_util_mprintf("%2d\n", dscp);
                    }
                    else
                    {
                        diag_util_mprintf("Not Support\n");
                    }
                }
            }
            else
            {
                diag_util_mprintf("Original DSCP Value\t  DSCP Value\n");
                for (int_pri = 0; int_pri <= RTK_VALUE_OF_DSCP_MAX; int_pri++)
                {
                    diag_util_printf("DSCP %2d : ", int_pri);
                    if (RT_ERR_OK == rtk_qos_dscp2DscpRemark_get(unit, int_pri, &dscp))
                    {
                        diag_util_mprintf("%2d\n", dscp);
                    }
                    else
                    {
                        diag_util_mprintf("Not Support\n");
                    }
                }
            }
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }

    }
    else
    {
        diag_util_mprintf("Interal Priority\t  DSCP Value\n");
        for (int_pri = 0; int_pri <= pri_max; int_pri++)
        {
            diag_util_printf("Priority %u : ", int_pri);
            if (RT_ERR_OK == rtk_qos_dscpRemark_get(unit, int_pri, &dscp))
            {
                diag_util_mprintf("%2d\n", dscp);
            }
            else
            {
                diag_util_mprintf("Not Support\n");
            }
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DSCP_SYSTEM_SOURCE
/*
 * qos get remarking dscp system source
 */
cparser_result_t cparser_cmd_qos_get_remarking_dscp_system_source(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_dscpRmkSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (RT_ERR_OK == rtk_qos_dscpRemarkSrcSel_get(unit, &type))
    {
        if (PRI_SRC_INT_PRI == type)
            diag_util_mprintf("Remarking Source : Internal priority\n");
        else if (PRI_SRC_DSCP == type)
            diag_util_mprintf("Remarking Source : DSCP value\n");
        else
            diag_util_mprintf("Remarking Source : Drop precedence\n");
    }
    else
    {
        diag_util_mprintf("Not Support\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_DEFAULT_PRIORITY_OUTER_TAG_PORT_PORTS_ALL
/*
 * qos get default-priority outer-tag port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_default_priority_outer_tag_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_outer1pDfltSrc_t type;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Default Outer Priority Configuration of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (RT_ERR_OK == rtk_qos_portOuter1pDfltPriSrcSel_get(unit, port, &type))
        {
            diag_util_mprintf("Port %2d : %s\n", port, (type == OUTER_1P_DFLT_SRC_INT_PRI) ? "Copy internal priority" : "Copy inner priority");
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DSCP_GROUP_GROUP
/*
 * qos get remarking dscp group { <UINT:group> }
 */
cparser_result_t cparser_cmd_qos_get_remarking_dscp_group_group(cparser_context_t *context,
    uint32_t *group_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      dscp;
    uint32      group_id;
    uint32      group_id_max;
    rtk_pri_t   int_pri;
    rtk_pri_t   pri_max;
    uint32      dp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (6 == context->parser->cmd_tokens )
    {
        group_id        = *group_ptr;
        group_id_max    = *group_ptr;
    }
    else
    {
        group_id        = 0;
        DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, priority_remark_group_idx_max);
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    for (;group_id <= group_id_max; group_id++)
    {
        diag_util_mprintf("Group %u DSCP remarking\n", group_id);
        
        
        for (int_pri = 0; int_pri <= pri_max; int_pri++)
        {
            diag_util_mprintf("  Internal priority %u\n", int_pri);
            
            for (dp = 0; dp <= RTK_DROP_PRECEDENCE_MAX; dp++)
            {
                if ((ret = rtk_qos_dscpRemarkGroup_get(unit, group_id, int_pri, dp, &dscp)) != RT_ERR_OK)
                {
                    diag_util_mprintf("  Not support\n");
                    continue;
                }
                diag_util_mprintf("    Drop precedence %u : %u\n", dp, dscp);
            }
        }
        
        diag_util_mprintf("\n");
    }
    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DSCP_PORT_PORTS_ALL
/*
 * qos get remarking dscp port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remarking_dscp_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_enable_t    enable;
    uint32      group_id;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("DSCP remarking configuration of ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %u configuration: \n", port);
        
        diag_util_printf("\tStatus\t\t: ");
        if (RT_ERR_OK == rtk_qos_dscpRemarkEnable_get(unit, port, &enable))
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
        
        diag_util_printf("\tRemarking group\t: ");
        if (RT_ERR_OK == rtk_qos_portdscpRemarkGroup_get(unit, port, &group_id))
        {
            diag_util_mprintf("%u\n", group_id);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
        
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_OUTER_TAG_GROUP_GROUP
/*
 * qos get remarking outer-tag group { <UINT:group> }
 */
cparser_result_t cparser_cmd_qos_get_remarking_outer_tag_group_group(cparser_context_t *context,
    uint32_t *group_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_pri_t   pri;
    rtk_pri_t   pri_max;
    uint32      dei;
    uint32      group_id;
    uint32      group_id_max;
    rtk_pri_t   int_pri;
    uint32      dp;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (6 == context->parser->cmd_tokens )
    {
        group_id        = *group_ptr;
        group_id_max    = *group_ptr;
    }
    else
    {
        group_id        = 0;
        DIAG_OM_GET_CHIP_CAPACITY(unit, group_id_max, priority_remark_group_idx_max);
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    for (;group_id <= group_id_max; group_id++)
    {
        diag_util_mprintf("Group %u outer tag priority remarking\n", group_id);
        
        
        for (int_pri = 0; int_pri <= pri_max; int_pri++)
        {
            diag_util_mprintf("  Internal priority %u\n", int_pri);
            
            for (dp = 0; dp <= RTK_DROP_PRECEDENCE_MAX; dp++)
            {
                if ((ret = rtk_qos_outer1pRemarkGroup_get(unit, group_id, int_pri, dp, &pri, &dei)) != RT_ERR_OK)
                {
                    diag_util_mprintf("  Not support\n");
                    continue;
                }
                diag_util_mprintf("    Drop precedence %u : priority %u, dei %u\n", dp, pri, dei);
            }
        }
        
        diag_util_mprintf("\n");
    }
    
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_OUTER_TAG_SYSTEM_SOURCE
/*
 * qos get remarking outer-tag system source 
 */
cparser_result_t cparser_cmd_qos_get_remarking_outer_tag_system_source(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_outer1pRmkSrc_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (RT_ERR_OK == rtk_qos_outer1pRemarkSrcSel_get(unit, &type))
    {
        diag_util_mprintf("Remarking Source : %s\n", (type == PRI_SRC_INT_PRI) ? "Internal priority" : "Original outer-tag priority");
    }
    else
    {
        diag_util_mprintf("Not Support\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_OUTER_TAG_PORT_PORTS_ALL
/*
 * qos get remarking outer-tag port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_remarking_outer_tag_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_enable_t    enable;
    uint32      group_id;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Outer tag priority remarking configuration of ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %u configuration: \n", port);
        
        diag_util_printf("\tStatus\t\t\t: ");
        if (RT_ERR_OK == rtk_qos_out1pRemarkEnable_get(unit, port, &enable))
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
        
        diag_util_printf("\tRemarking group\t\t: ");
        if (RT_ERR_OK == rtk_qos_portOuter1pRemarkGroup_get(unit, port, &group_id))
        {
            diag_util_mprintf("%u\n", group_id);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
        
        diag_util_printf("\tPriority mapping group\t: ");
        if (RT_ERR_OK == rtk_qos_portOuter1pPriMapGroup_get(unit, port, &group_id))
        {
            diag_util_mprintf("%u\n", group_id);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_OUTER_TAG_SYSTEM
/*
 * qos get remarking outer-tag system
 */
cparser_result_t cparser_cmd_qos_get_remarking_outer_tag_system(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_pri_t   dot1p_pri;
    rtk_pri_t   int_pri;
    rtk_pri_t   pri_max;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, pri_max, internal_priority_max);
    
    diag_util_mprintf("Internal priority\t  Outer Priority\n");
    for (int_pri = 0; int_pri <= pri_max; int_pri++)
    {
        diag_util_printf("Priority %2d : ", int_pri);
        if (RT_ERR_OK == rtk_qos_outer1pRemark_get(unit, int_pri, &dot1p_pri))
        {
            diag_util_mprintf("%u\n", dot1p_pri);
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DOT1P_DSCP_OUTER_TAG_PORT_PORTS_ALL_STATE
/*
 * qos get remarking ( dot1p | dscp | outer-tag ) port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_qos_get_remarking_dot1p_dscp_outer_tag_port_ports_all_state(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
        
    if (0 == strcmp(context->parser->tokens[3].buf, "dot1p"))
    {
        diag_util_mprintf("802.1P Remarking Ability of Ports \n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if (RT_ERR_OK == rtk_qos_1pRemarkEnable_get(unit, port, &enable))
            {
                if (ENABLED == enable)
                {
                    diag_util_printf("Port %2d : Enable\n", port);
                }
                else
                {
                    diag_util_printf("Port %2d : Disable\n", port);
                }
            }
        }
    }
    else if (0 == strcmp(context->parser->tokens[3].buf, "dscp"))
    {
        diag_util_mprintf("DSCP Remarking Ability of Ports \n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if (RT_ERR_OK == rtk_qos_dscpRemarkEnable_get(unit, port, &enable))
            {
                if (ENABLED == enable)
                {
                    diag_util_printf("Port %2d : Enable\n", port);
                }
                else
                {
                    diag_util_printf("Port %2d : Disable\n", port);
                }
            }
        }
    } 
    else
    {
        diag_util_mprintf("Outer-tag Priority Remarking Ability of Ports \n");
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if (RT_ERR_OK == rtk_qos_out1pRemarkEnable_get(unit, port, &enable))
            {
                if (ENABLED == enable)
                {
                    diag_util_printf("Port %2d : Enable\n", port);
                }
                else
                {
                    diag_util_printf("Port %2d : Disable\n", port);
                }
            }
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_REMARKING_DEI_PORT_PORTS_ALL_STATE
/*
 * qos get remarking dei port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_qos_get_remarking_dei_port_ports_all_state(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_enable_t enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("DEI Remarking Ability of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if (RT_ERR_OK == rtk_qos_deiRemarkEnable_get(unit, port, &enable))
        {
            if (ENABLED == enable)
            {
                diag_util_printf("Port %2d : Enable\n", port);
            }
            else
            {
                diag_util_printf("Port %2d : Disable\n", port);
            }
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_SCHEDULING_ALGORITHM_PORT_PORTS_ALL
/*
 * qos get scheduling algorithm port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_scheduling_algorithm_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_qos_scheduling_type_t   algorithm;
    rtk_port_t  port;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Scheduling Algorithm of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_printf("Port %2d : ", port);
        if (RT_ERR_OK == rtk_qos_schedulingAlgorithm_get(unit, port, &algorithm))
        {
            if (WFQ == algorithm)
            {
                diag_util_mprintf("WFQ\n");
            }
            else
            {
                diag_util_mprintf("WRR\n");
            }
        }
        else
        {
            diag_util_mprintf("Not Support\n");
        }
        
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_SCHEDULING_WFQ_FIXED_RATE_PORT_PORTS_ALL
/*
 * qos get scheduling wfq-fixed-rate port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_scheduling_wfq_fixed_rate_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_enable_t    enable;
    rtk_port_t  port;
    rtk_qid_t   queue;
    rtk_qid_t   qid_max;
    diag_portlist_t  portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, qid_max, max_num_of_queue); 
    qid_max = qid_max - 1;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %u status of WFQ fixed rate\n", port);
        
        for (queue = 0; queue <= qid_max; queue++)
        {
            diag_util_printf("\tQueue %u : ", queue);
            if (RT_ERR_OK == rtk_qos_wfqFixedBandwidthEnable_get(unit, port, queue, &enable))
            {
                if (ENABLED == enable)
                {
                    diag_util_mprintf("Enable\n");
                }
                else
                {
                    diag_util_mprintf("Disable\n");
                }
            }
            else
            {
                diag_util_mprintf("Not support\n");
            }
        }
        
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_SCHEDULING_QUEUE_WEIGHT_PORT_PORTS_ALL
/*
 * qos get scheduling queue-weight port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_scheduling_queue_weight_port_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_port_t  port;
    rtk_qid_t   queue;
    rtk_qid_t   qid_max;
    diag_portlist_t  portlist;
    rtk_qos_queue_weights_t     queue_weight;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, qid_max, max_num_of_queue); 
    qid_max = qid_max - 1;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %2d Queue Weight\n", port);
        
        if (RT_ERR_OK != rtk_qos_schedulingQueue_get(unit, port, &queue_weight))
        {
            diag_util_mprintf("\tNot Support\n");
            continue;
        }
        
        for (queue = 0; queue <= qid_max; queue++)
        {
            diag_util_mprintf("\tQueue %u : %u\n", queue, queue_weight.weights[queue]);
        }
        
        diag_util_mprintf("\n");
        
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_CONGEST_AVOIDANCE
/*
 * qos get congest-avoidance
 */
cparser_result_t cparser_cmd_qos_get_congest_avoidance(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_congAvoidAlgo_t algorithm;
    rtk_enable_t    enable;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("Congestion Avoidance Configuration\n");
    if ((ret = rtk_qos_congAvoidAlgo_get(unit, &algorithm)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    /* 8328, 8390, 8380 support */
    diag_util_printf("\tCongestion avoidance algorithm : ");
    if (CONG_AVOID_WRED == algorithm)
    {
        diag_util_mprintf("WRED\n");
    }
    else if (CONG_AVOID_WTD == algorithm)
    {
        diag_util_mprintf("WTD\n");
    }
    else if (CONG_AVOID_SRED == algorithm)
    {
        diag_util_mprintf("SRED\n");
    }
    else if (CONG_AVOID_SWRED == algorithm)
    {
        diag_util_mprintf("SWRED\n");
    }
    else if (CONG_AVOID_TD == algorithm)
    {
        diag_util_mprintf("TD\n");
    }
    
    if (DIAG_OM_GET_REALCHIPID(RTL8328M_CHIP_ID) || DIAG_OM_GET_REALCHIPID(RTL8328S_CHIP_ID) ||
        DIAG_OM_GET_REALCHIPID(RTL8328L_CHIP_ID))
    {
        if ((ret = rtk_qos_congAvoidSysThreshEnable_get(unit, &enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_printf("\tStatus of system threshold\t: ");
        if (ENABLED == enable)
        {
            diag_util_mprintf("ENABLED\n");
        }
        else
        {
            diag_util_mprintf("DISABLED\n");
        }
        
        if ((ret = rtk_qos_congAvoidPortThreshEnable_get(unit, &enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_printf("\tStatus of port threshold\t: ");
        if (ENABLED == enable)
        {
            diag_util_mprintf("ENABLED\n");
        }
        else
        {
            diag_util_mprintf("DISABLED\n");
        }
        
        if ((ret = rtk_qos_congAvoidQueueThreshEnable_get(unit, &enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_printf("\tStatus of queue threshold\t: ");
        if (ENABLED == enable)
        {
            diag_util_mprintf("ENABLED\n");
        }
        else
        {
            diag_util_mprintf("DISABLED\n");
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_QUEUES_ALL
/*
 * qos get congest-avoidance queue-threshold ( <PORT_LIST:queues> | all )
 */
cparser_result_t cparser_cmd_qos_get_congest_avoidance_queue_threshold_queues_all(cparser_context_t *context,
    char **queues_ptr)
{
    uint32      unit = 0, queue, probability;
    int32       ret = RT_ERR_FAILED;
    uint32      dp;
    diag_portlist_t  queuelist;
    rtk_qos_congAvoidThresh_t   threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(queuelist, 4), ret);
    
    DIAG_UTIL_PORTMASK_SCAN(queuelist, queue)
    {
        diag_util_mprintf("Queue %u Congestion Avoidance Threshold\n", queue);
        
        for (dp = 0; dp <= RTK_DROP_PRECEDENCE_MAX; dp++)
        {
            diag_util_mprintf("Drop precedence %u\n", dp);
            if (RT_ERR_OK != rtk_qos_congAvoidGlobalQueueThresh_get(unit, queue, dp, &threshold))
            {
                diag_util_mprintf("Not support\n");
                continue;
            }
            
            diag_util_mprintf("\tMax threshold : %u\n", threshold.maxThresh);
            diag_util_mprintf("\tMin threshold : %u\n", threshold.minThresh);

            if (RT_ERR_OK != rtk_qos_congAvoidGlobalQueueDropProbability_get(unit, queue, dp, &probability))
            {
                diag_util_mprintf("Not support\n");
                continue;
            }
            
            diag_util_mprintf("\tDrop Probability : %u\n", probability);
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}   
#endif

#ifdef CMD_QOS_GET_CONGEST_AVOIDANCE_PORT_THRESHOLD_PORTS_ALL
/*
 * qos get congest-avoidance port-threshold ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_congest_avoidance_port_threshold_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    uint32      dp;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_congAvoidThresh_t   threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %u congestion avoidance threshold\n", port);
        
        for (dp = 0; dp <= RTK_DROP_PRECEDENCE_MAX; dp++)
        {
            diag_util_mprintf("  Drop precedence %u\n", dp);
            if (RT_ERR_OK != rtk_qos_congAvoidPortThresh_get(unit, port, dp, &threshold))
            {
                diag_util_mprintf("Not support\n");
                continue;
            }
            
            diag_util_mprintf("    Max threshold : %u\n", threshold.maxThresh);
            diag_util_mprintf("    Min threshold : %u\n", threshold.minThresh);
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_PORTS_ALL
/*
 * qos get congest-avoidance queue-threshold ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_qos_get_congest_avoidance_queue_threshold_ports_all(cparser_context_t *context,
    char **ports_ptr)
{
    uint32      unit = 0;
    rtk_qid_t   queue;
    rtk_qid_t   qid_max;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_qos_congAvoidThresh_t   threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, qid_max, max_num_of_queue); 
    qid_max = qid_max - 1;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port %u congestion avoidance threshold of queue\n", port);
        
        for (queue = 0; queue <= qid_max; queue++)
        {
            diag_util_mprintf("  Queue %u\n", queue);
            if (RT_ERR_OK != rtk_qos_congAvoidQueueThresh_get(unit, port, queue, &threshold))
            {
                diag_util_mprintf("Not support\n");
                continue;
            }
            
            diag_util_mprintf("    Max threshold : %u\n", threshold.maxThresh);
            diag_util_mprintf("    Min threshold : %u\n", threshold.minThresh);
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_CONGEST_AVOIDANCE_SYSTEM_THRESHOLD
/*
 * qos get congest-avoidance system-threshold
 */
cparser_result_t cparser_cmd_qos_get_congest_avoidance_system_threshold(cparser_context_t *context)
{
    uint32      unit = 0, probability;
    uint32      dp;
    rtk_qos_congAvoidThresh_t   threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("System Congestion Avoidance Threshold\n");
    
    for (dp = 0; dp <= RTK_DROP_PRECEDENCE_MAX; dp++)
    {
        diag_util_mprintf("  Drop precedence %u\n", dp);
        if (RT_ERR_OK != rtk_qos_congAvoidSysThresh_get(unit, dp, &threshold))
        {
            diag_util_mprintf("Not Support\n");
            continue;
        }
        
        diag_util_mprintf("\tMax threshold : %u\n", threshold.maxThresh);
        diag_util_mprintf("\tMin threshold : %u\n", threshold.minThresh);

        if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
        {
            if (RT_ERR_OK != rtk_qos_congAvoidSysDropProbability_get(unit, dp, &probability))
            {
                diag_util_mprintf("Not Support\n");
                continue;
            }
            
            diag_util_mprintf("\tDrop Probability : %u\n", probability);
        }
    }
    
    diag_util_mprintf("\n");
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_QOS_GET_WRED
/*
 * qos get wred
 */
cparser_result_t cparser_cmd_qos_get_wred(cparser_context_t *context)
{
    uint32      unit = 0;
    uint32      mpd;
    uint32      dp;
    rtk_enable_t    enable;
    rtk_qos_wredThresh_t   threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("WRED configuration\n");
    
    if (DIAG_OM_GET_REALCHIPID(RTL8328M_CHIP_ID) || DIAG_OM_GET_REALCHIPID(RTL8328S_CHIP_ID) ||
        DIAG_OM_GET_REALCHIPID(RTL8328L_CHIP_ID))
    {
        diag_util_printf("ECN\t\t: ");
        if (RT_ERR_OK == rtk_qos_wredEcnEnable_get(unit, &enable))
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
        
        diag_util_printf("Counter reverse\t: ");
        if (RT_ERR_OK == rtk_qos_wredCntReverseEnable_get(unit, &enable))
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
        
        
        diag_util_printf("MPD\t\t: ");
        if (RT_ERR_OK == rtk_qos_wredMpd_get(unit, &mpd))
        {
            diag_util_mprintf("%u\n", mpd);
        }
        else
        {
            diag_util_mprintf("Not support\n");
        }
        
        diag_util_mprintf("\n");
        diag_util_mprintf("WRED system threshold\n");
        for (dp = 0; dp <= RTK_DROP_PRECEDENCE_MAX; dp++)
        {
            diag_util_mprintf("  Drop precedence %u\n", dp);
            if (RT_ERR_OK != rtk_qos_wredSysThresh_get(unit, dp, &threshold))
            {
                diag_util_mprintf("Not support\n");
                continue;
            }
            
            diag_util_mprintf("    Max threshold : %u\n", threshold.maxThresh);
            diag_util_mprintf("    Min threshold : %u\n", threshold.minThresh);
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}
#endif


