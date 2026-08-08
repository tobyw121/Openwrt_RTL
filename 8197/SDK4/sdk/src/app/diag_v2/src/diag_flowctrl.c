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
 * $Revision: 37509 $
 * $Date: 2013-03-06 17:07:46 +0800 (Wed, 06 Mar 2013) $
 *
 * Purpose : Define diag shell commands for flow control
 * 
 * Feature : The file includes the following module and sub-modules
 *           1) flow control commands.
 *
 */

#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <rtk/flowctrl.h>
#include <rtk/qos.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_FLOWCTRL_GET_INGRESS_SYSTEM_FC_OFF_THRESHOLD_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD
/* 
 * flowctrl get ingress system ( fc-off-threshold | fc-on-threshold ) ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_system_fc_off_threshold_fc_on_threshold_high_off_high_on_low_off_low_on_threshold(cparser_context_t *context)
{
    uint32 unit, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
       
    switch(TOKEN_CHAR(4,4))
    {
        case 'f':
            fcon = 0;       
            break;

        case 'n':
            fcon = 1;      
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if(fcon)    
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrSystemPauseThresh_get(unit, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrSystemCongestThresh_get(unit, &thresh), ret);

    diag_util_printf("Flow Control %s\n", fcon ? "On" : "Off");

    switch(TOKEN_CHAR(5,0))
    {
        case 'h':
            if('f' == TOKEN_CHAR(5,7))
                diag_util_printf("\tHigh Off Threshold : 0x%X (%d)\n", thresh.highOff, thresh.highOff);
            else
                diag_util_printf("\tHigh On Threshold  : 0x%X (%d)\n", thresh.highOn, thresh.highOn);
            break;

        case 'l':
            if('f' == TOKEN_CHAR(5,5))
                diag_util_printf("\tLow Off Threshold  : 0x%X (%d)\n", thresh.lowOff, thresh.lowOff);
            else
                diag_util_printf("\tLow On Threshold   : 0x%X (%d)\n", thresh.lowOn, thresh.lowOn);
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_FC_OFF_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) fc-off-threshold ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_fc_off_threshold_high_off_high_on_low_off_low_on_threshold(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Flow Control Off\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortCongestThresh_get(unit, port, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                if('f' == TOKEN_CHAR(6,7))
                    diag_util_mprintf("\tPort %2d High Off Threshold : 0x%X\n", port, thresh.highOff);
                else
                    diag_util_mprintf("\tPort %2d High On Threshold  : 0x%X\n", port, thresh.highOn);
                break;

            case 'l':
                if('f' == TOKEN_CHAR(6,5))
                    diag_util_mprintf("\tPort %2d Low Off Threshold  : 0x%X\n", port, thresh.lowOff);
                else
                    diag_util_mprintf("\tPort %2d Low On Threshold   : 0x%X\n", port, thresh.lowOn);
                break;       
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) fc-on-threshold ( high-off | high-on | low-off | low-on ) threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_fc_on_threshold_high_off_high_on_low_off_low_on_threshold(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Flow Control On\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortPauseThresh_get(unit, port, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                if('f' == TOKEN_CHAR(6,7))
                    diag_util_mprintf("\tPort %2d High Off Threshold : 0x%X\n", port, thresh.highOff);
                else
                    diag_util_mprintf("\tPort %2d High On Threshold  : 0x%X\n", port, thresh.highOn);
                break;

            case 'l':
                if('f' == TOKEN_CHAR(6,5))
                    diag_util_mprintf("\tPort %2d Low Off Threshold  : 0x%X\n", port, thresh.lowOff);
                else
                    diag_util_mprintf("\tPort %2d Low On Threshold   : 0x%X\n", port, thresh.lowOn);
                break;       
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_STATE
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Flow Control Port Enable Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_portEnable_get(unit, port, &enable), ret);  
        diag_util_mprintf("\tPort %2d : %s\n", port, enable ? "ENABLE" : "DISABLE");
    }  

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_MODE
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) mode
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_mode(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Flow Control Port Force/N-way Mode\n");
   
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_portPauseForceModeEnable_get(unit, port, &enable), ret);       
        diag_util_mprintf("\tPort %2d : %s\n", port, enable ? "FORCE" : "N-WAY");
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_THRESHOLD_GROUP
/*
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) threshold-group
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_threshold_group(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    uint32 grp_idx;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Flow Control Threshold Group of Port Configuration\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortPauseThreshGroupSel_get(unit, port, &grp_idx), ret);  
        diag_util_mprintf("\tPort %2d : %d\n", port, grp_idx);
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ACTION
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) pause-on-action
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_pause_on_action(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_pauseOnAction_t act;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Flow Control Port Pause On Action\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAction_get(unit, port, &act), ret);  
        diag_util_mprintf("\tPort %2d : %s\n", port, act ? "DROP" : "RECEIVE");
    }    

    return CPARSER_OK;    
}
#endif

#if 0 /* replace by separate commands, 2012/12/26 */
#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PAGE_PKT
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num ( page | pkt )
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_pause_on_allowed_num_page_pkt(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port, type, num;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
           
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(6,1))
    {
        case 'a':
            type = 0;   /* page */
            break;

        case 'k':
            type = 1;   /* packet */
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    diag_util_mprintf("Ingress Flow Control Port Pause On Allowed %s Number\n", type ? "Packet" : "Page");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if(type)
            DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPktNum_get(unit, port, &num), ret);       
        else
            DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPageNum_get(unit, port, &num), ret);       

        diag_util_mprintf("\tPort %2d : %d\n", port, num);
    }

    return CPARSER_OK;    
}
#endif
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PKT
/*
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num pkt
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_pause_on_allowed_num_pkt(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port, num;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
           
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Flow Control Port Pause On Allowed Packet Number\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPktNum_get(unit, port, &num), ret);       
        diag_util_mprintf("\tPort %2d : %d\n", port, num);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_LEN
/*
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-len
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_pause_on_allowed_len(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port, pktLen;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Flow Control Port Pause On Allowed Total Packet Length\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPktLen_get(unit, port, &pktLen), ret);       
        diag_util_mprintf("\tPort %2d : %d\n", port, pktLen);
    }

    return CPARSER_OK;    
}
#endif
       
#ifdef CMD_FLOWCTRL_GET_INGRESS_THRESHOLD_GROUP_INDEX_FC_OFF_THRESHOLD_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_THRESHOLD
/*
 * flowctrl get ingress threshold-group <UINT:index> ( fc-off-threshold | fc-on-threshold ) ( high-off | high-on | low ) threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_threshold_group_index_fc_off_threshold_fc_on_threshold_high_off_high_on_low_threshold(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32 unit, grp_idx, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;

    switch(TOKEN_CHAR(5,4))
    {
        case 'f': /* flow control OFF */
            fcon = 0;       
            break;

        case 'n': /* flow control ON */
            fcon = 1;      
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if (fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPauseThreshGroup_get(unit, grp_idx, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrCongestThreshGroup_get(unit, grp_idx, &thresh), ret);

    diag_util_mprintf("Group %d Ingress Flow Control %s Threshold\n", grp_idx, fcon ? "On" : "Off");
    switch(TOKEN_CHAR(6,0))
    {
        case 'h':
            if('f' == TOKEN_CHAR(6,7))
                diag_util_printf("\tHigh Off Threshold : 0x%X (%d)\n", thresh.highOff, thresh.highOff);
            else
                diag_util_printf("\tHigh On Threshold : 0x%X (%d)\n", thresh.highOn, thresh.highOn);
            break;

        case 'l':
                diag_util_mprintf("\tLow Threshold   : 0x%X (%d)\n", thresh.lowOn, thresh.lowOn);
            break;       
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_SYSTEM_DROP_THRESHOLD_HIGH_LOW
/* 
 * flowctrl get egress system drop-threshold ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_system_drop_threshold_high_low(cparser_context_t *context)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrSystemDropThresh_get(unit, &thresh), ret);

    diag_util_printf("Egress Flow Control System Drop Threshold\n");
    
    switch(TOKEN_CHAR(5,0))
    {
        case 'h':
            diag_util_printf("\tHigh Threshold : 0x%X\n", thresh.high);
            break;

        case 'l':
            diag_util_printf("\tLow Threshold : 0x%X\n", thresh.low);
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }
    
    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_SYSTEM_DROP_THRESHOLD
/* 
 * flowctrl get system drop-threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_system_drop_threshold(cparser_context_t *context)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrSystemDropThresh_get(unit, &thresh), ret);

    diag_util_printf("Egress Flow Control System Drop Threshold : 0x%X (%d)\n", thresh.high, thresh.high);
    
    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_DROP_THRESHOLD_HIGH_LOW
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) drop-threshold ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_drop_threshold_high_low(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_printf("Egress Flow Control Port Drop Threshold\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThresh_get(unit, port, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                diag_util_printf("\tPort %2d High Threshold : 0x%X\n", port, thresh.high);
                break;

            case 'l':
                diag_util_printf("\tPort %2d Low Threshold : 0x%X\n", port, thresh.low);
                break;       
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_DROP_THRESHOLD
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) drop-threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_drop_threshold(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_printf("Egress Flow Control Port Drop Threshold\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThresh_get(unit, port, &thresh), ret);
        diag_util_printf("\tPort %2d Threshold : 0x%X (%d)\n", port, thresh.high, thresh.high);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_QUEUE_ID_QID_ALL_DROP_THRESHOLD_HIGH_LOW
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) drop-threshold ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_queue_id_qid_all_drop_threshold_high_low(cparser_context_t *context,
    char **ports_ptr,
    char **qid_ptr)
{
    uint32 unit, port, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 6), ret);

    diag_util_printf("Egress Flow Control Port and Queue Drop Threshold\n");
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_MASK_SCAN(queueMask, queue)
        {        
            DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortQueueDropThresh_get(unit, port, queue, &thresh), ret);

            switch(TOKEN_CHAR(8,0))
            {
                case 'h':
                    diag_util_printf("\tPort %2d Queue %d High Threshold : 0x%X\n", port, queue, thresh.high);
                    break;

                case 'l':
                    diag_util_printf("\tPort %2d Queue %d Low Threshold : 0x%X\n", port, queue, thresh.low);
                    break;       
            
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;            
            }
        }
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_QUEUE_DROP_STATE
/*
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) queue-drop state
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_queue_drop_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Flow Control Egress Queue Drop of Port Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortQueueDropEnable_get(unit, port, &enable), ret);  
        diag_util_mprintf("\tPort %2d : %s\n", port, enable ? "ENABLE" : "DISABLE");
    }  

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_IGR_CONGEST_CHECK
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) igr-congest-check
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_igr_congest_check(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Egress Flow Control Ignore RX Port Congestion Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropRefCongestEnable_get(unit, port, &enable), ret);       
        diag_util_mprintf("\tPort %2d : %s\n", port, enable ? "ENABLE" : "DISABLE");
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_DROP_MODE
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) drop-mode
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_drop_mode(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Egress Flow Control Port Force/N-way Mode\n");
   
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropForceModeEnable_get(unit, port, &enable), ret);       
        diag_util_mprintf("\tPort %2d : %s\n", port, enable ? "FORCE" : "N-WAY");
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_DROP_PATH
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) drop-path
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_drop_path(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    uint8  *pathStr[4] = { (uint8 *)"BOTH", (uint8 *)"RX", (uint8 *)"TX", (uint8 *)"DISABLE" };
    diag_portlist_t portlist;
    rtk_flowctrl_egrDropMode_t path;    
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("Egress Flow Control Port Packet Drop Path\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropMode_get(unit, port, &path), ret);  
        diag_util_mprintf("\tPort %2d : %s\n", port, pathStr[path]);
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_QUEUE_ID_QID_ALL_DROP_THRESHOLD_HIGH_LOW
/* 
 * flowctrl get egress queue-id ( <MASK_LIST:qid> | all ) drop-threshold ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_queue_id_qid_all_drop_threshold_high_low(cparser_context_t *context,
    char **qid_ptr)
{
    uint32 unit, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_printf("Egress Flow Control Queue Drop Threshold\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 4), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThresh_get(unit, queue, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                diag_util_printf("\tQueue %d High Threshold : 0x%X\n", queue, thresh.high);
                break;

            case 'l':
                diag_util_printf("\tQueue %d Low Threshold : 0x%X\n", queue, thresh.low);
                break;       
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_QUEUE_ID_QID_ALL_DROP_THRESHOLD
/* 
 * flowctrl get egress queue-id ( <MASK_LIST:qid> | all ) drop-threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_queue_id_qid_all_drop_threshold(cparser_context_t *context,
    char **qid_ptr)
{
    uint32 unit, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_printf("Egress Flow Control Queue Drop Threshold\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 4), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThresh_get(unit, queue, &thresh), ret);
        diag_util_printf("\tQueue %d Threshold : 0x%X (%d)\n", queue, thresh.high, thresh.high);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_CPU_QUEUE_ID_QID_ALL_DROP_THRESHOLD_HIGH_LOW
/* 
 * flowctrl get egress cpu queue-id ( <MASK_LIST:qid> | all ) drop-threshold ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_cpu_queue_id_qid_all_drop_threshold_high_low(cparser_context_t *context,
    char **qid_ptr)
{
    uint32 unit, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_printf("CPU Port - Egress Flow Control Queue Drop Threshold\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 5), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrCpuQueueDropThresh_get(unit, queue, &thresh), ret);

        switch(TOKEN_CHAR(7,0))
        {
            case 'h':
                diag_util_printf("\tQueue %d High Threshold : 0x%X (%d)\n", queue, thresh.high, thresh.high);
                break;

            case 'l':
                diag_util_printf("\tQueue %d Low Threshold : 0x%X (%d)\n", queue, thresh.low, thresh.low);
                break;       
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_SYSTEM_FC_OFF_THRESHOLD_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD_THRESHOLD
/* 
 * flowctrl set ingress system ( fc-off-threshold | fc-on-threshold ) ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_system_fc_off_threshold_fc_on_threshold_high_off_high_on_low_off_low_on_threshold_threshold(cparser_context_t *context,
    uint32_t *threshold_ptr)
{
    uint32 unit, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);      
       
    switch(TOKEN_CHAR(4,4))
    {
        case 'f':
            fcon = 0;       
            break;

        case 'n':
            fcon = 1;      
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if(fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrSystemPauseThresh_get(unit, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrSystemCongestThresh_get(unit, &thresh), ret);

    switch(TOKEN_CHAR(5,0))
    {
        case 'h':
            if('f' == TOKEN_CHAR(5,7))
                thresh.highOff = *threshold_ptr;
            else
                thresh.highOn = *threshold_ptr;
            break;

        case 'l':
            if('f' == TOKEN_CHAR(5,5))
                thresh.lowOff = *threshold_ptr;
            else
                thresh.lowOn = *threshold_ptr;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if(fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrSystemPauseThresh_set(unit, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrSystemCongestThresh_set(unit, &thresh), ret);

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_FC_OFF_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD_THRESHOLD
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) fc-off-threshold ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */    
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_fc_off_threshold_high_off_high_on_low_off_low_on_threshold_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);        

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortCongestThresh_get(unit, port, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                if('f' == TOKEN_CHAR(6,7))
                    thresh.highOff = *threshold_ptr;
                else
                    thresh.highOn = *threshold_ptr;
                break;

            case 'l':
                if('f' == TOKEN_CHAR(6,5))
                    thresh.lowOff = *threshold_ptr;
                else
                    thresh.lowOn = *threshold_ptr;
                break;       
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }

        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortCongestThresh_set(unit, port, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD_THRESHOLD
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) fc-on-threshold ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
 */    
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_fc_on_threshold_high_off_high_on_low_off_low_on_threshold_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, port;
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);        

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortPauseThresh_get(unit, port, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                if('f' == TOKEN_CHAR(6,7))
                    thresh.highOff = *threshold_ptr;
                else
                    thresh.highOn = *threshold_ptr;
                break;

            case 'l':
                if('f' == TOKEN_CHAR(6,5))
                    thresh.lowOff = *threshold_ptr;
                else
                    thresh.lowOn = *threshold_ptr;
                break;       
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }

        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortPauseThresh_set(unit, port, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */    
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_portEnable_set(unit, port, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_MODE_FORCE_N_WAY
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) mode ( force | n-way )
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_mode_force_n_way(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,0))
    {
        case 'n':
            enable = DISABLED;
            break;

        case 'f':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_portPauseForceModeEnable_set(unit, port, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_THRESHOLD_GROUP_INDEX
/*
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) threshold-group <UINT:index>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_threshold_group_index(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *index_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortPauseThreshGroupSel_set(unit, port, *index_ptr), ret);  
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ACTION_DROP_RECEIVE
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) pause-on-action ( drop | receive )
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_pause_on_action_drop_receive(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_pauseOnAction_t act;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,0))
    {
        case 'r':
            act = PAUSE_ON_RECEIVE;
            break;

        case 'd':
            act = PAUSE_ON_DROP;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAction_set(unit, port, act), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_THRESHOLD_GROUP_INDEX_FC_OFF_THRESHOLD_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_THRESHOLD_THRESHOLD
/*
 * flowctrl set ingress threshold-group <UINT:index> ( fc-off-threshold | fc-on-threshold ) ( high-off | high-on | low ) threshold <UINT:threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_threshold_group_index_fc_off_threshold_fc_on_threshold_high_off_high_on_low_threshold_threshold(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, grp_idx, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);        

    grp_idx = *index_ptr;

    switch(TOKEN_CHAR(5,4))
    {
        case 'f': /* flow control OFF */
            fcon = 0;       
            break;

        case 'n': /* flow control ON */
            fcon = 1;      
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if(fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPauseThreshGroup_get(unit, grp_idx, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrCongestThreshGroup_get(unit, grp_idx, &thresh), ret);

    switch(TOKEN_CHAR(6,0))
    {
        case 'h':
            if('f' == TOKEN_CHAR(6,7))
                thresh.highOff = *threshold_ptr;
            else
                thresh.highOn = *threshold_ptr;
            break;

        case 'l':
            thresh.lowOn = *threshold_ptr;
            thresh.lowOff = *threshold_ptr;
            break;       
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if(fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPauseThreshGroup_set(unit, grp_idx, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrCongestThreshGroup_set(unit, grp_idx, &thresh), ret);

    return CPARSER_OK;      
}
#endif

#if 0 /* replace by separate commands, by 2012/12/26 */
#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PAGE_PKT_NUMBER
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num ( page | pkt ) <UINT:number>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_pause_on_allowed_num_page_pkt_number(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *number_ptr)
{
    uint32 unit, port, type;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,1))
    {
        case 'a':
            type = 0;   /* page */
            break;

        case 'k':
            type = 1;   /* packet */
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if(type)
            DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPktNum_set(unit, port, *number_ptr), ret);       
        else
            DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPageNum_set(unit, port, *number_ptr), ret);       
    }
    
    return CPARSER_OK;    
}
#endif
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PKT_NUMBER
/*
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num pkt <UINT:number>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_pause_on_allowed_num_pkt_number(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *number_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPktNum_set(unit, port, *number_ptr), ret);       
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_LEN_LENGTH
/*
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-len <UINT:length>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_pause_on_allowed_len_length(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *length_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPktLen_set(unit, port, *length_ptr), ret);
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_SYSTEM_DROP_THRESHOLD_HIGH_LOW_THRESHOLD
/* 
 * flowctrl set egress system drop-threshold ( high | low ) <UINT:threshold>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_system_drop_threshold_high_low_threshold(cparser_context_t *context,
    uint32_t *threshold_ptr)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);      
     
    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrSystemDropThresh_get(unit, &thresh), ret);
    
    switch(TOKEN_CHAR(5,0))
    {
        case 'h':
            thresh.high = *threshold_ptr;
            break;

        case 'l':
            thresh.low = *threshold_ptr;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrSystemDropThresh_set(unit, &thresh), ret);

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_SYSTEM_DROP_THRESHOLD_THRESHOLD
/* 
 * flowctrl set system drop-threshold <UINT:threshold>
 */    
cparser_result_t cparser_cmd_flowctrl_set_system_drop_threshold_threshold(cparser_context_t *context,
    uint32_t *threshold_ptr)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);      
     
    thresh.high = *threshold_ptr;
    thresh.low = *threshold_ptr;

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrSystemDropThresh_set(unit, &thresh), ret);

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_DROP_THRESHOLD_HIGH_LOW_THRESHOLD
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) drop-threshold ( high | low ) <UINT:threshold>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_drop_threshold_high_low_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThresh_get(unit, port, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                thresh.high = *threshold_ptr;
                break;

            case 'l':
                thresh.low = *threshold_ptr;
                break;                   
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }

        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThresh_set(unit, port, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_DROP_THRESHOLD_THRESHOLD
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) drop-threshold <UINT:threshold>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_drop_threshold_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        thresh.high = *threshold_ptr;
        thresh.low = *threshold_ptr;
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThresh_set(unit, port, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_QUEUE_ID_QID_ALL_DROP_THRESHOLD_HIGH_LOW_THRESHOLD
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) queue-id ( <MASK_LIST:qid> | all ) drop-threshold ( high | low ) <UINT:threshold>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_queue_id_qid_all_drop_threshold_high_low_threshold(cparser_context_t *context,
    char **ports_ptr,
    char **qid_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, port, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 6), ret);
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        DIAG_UTIL_MASK_SCAN(queueMask, queue)
        {        
            DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortQueueDropThresh_get(unit, port, queue, &thresh), ret);

            switch(TOKEN_CHAR(8,0))
            {
                case 'h':
                    thresh.high = *threshold_ptr;
                    break;

                case 'l':
                    thresh.low = *threshold_ptr;
                    break;                   
            
                default:
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;            
            }

            DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortQueueDropThresh_set(unit, port, queue, &thresh), ret);
        }
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_QUEUE_DROP_STATE_DISABLE_ENABLE
/*
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) queue-drop state ( disable | enable )
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_queue_drop_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(7,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortQueueDropEnable_set(unit, port, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_IGR_CONGEST_CHECK_STATE_DISABLE_ENABLE
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) igr-congest-check state ( disable | enable )
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_igr_congest_check_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(7,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropRefCongestEnable_set(unit, port, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_DROP_MODE_FORCE_N_WAY
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) drop-mode ( force | n-way )
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_drop_mode_force_n_way(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;    
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,0))
    {
        case 'n':
            enable = DISABLED;
            break;

        case 'f':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropForceModeEnable_set(unit, port, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_DROP_PATH_BOTH_DISABLE_RX_TX
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) drop-path ( both | disable | rx | tx )
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_drop_path_both_disable_rx_tx(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_flowctrl_egrDropMode_t path;    
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,0))
    {
        case 'b':
            path = EGR_DROP_FOR_RX_AND_TX_PKTS;
            break;

        case 'r':
            path = EGR_DROP_FOR_RX_PKTS;
            break;       

        case 't':
            path = EGR_DROP_FOR_TX_PKTS;
            break;

        case 'd':
            path = EGR_DROP_DISABLE;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropMode_set(unit, port, path), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_QUEUE_ID_QID_ALL_DROP_THRESHOLD_HIGH_LOW_THRESHOLD
/* 
 * flowctrl set egress queue-id ( <MASK_LIST:qid> | all ) drop-threshold ( high | low ) <UINT:threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_queue_id_qid_all_drop_threshold_high_low_threshold(cparser_context_t *context,
    char **qid_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 4), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThresh_get(unit, queue, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                thresh.high = *threshold_ptr;
                break;

            case 'l':
                thresh.low = *threshold_ptr;
                break;                   
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }

        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThresh_set(unit, queue, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_QUEUE_ID_QID_ALL_DROP_THRESHOLD_THRESHOLD
/* 
 * flowctrl set egress queue-id ( <MASK_LIST:qid> | all ) drop-threshold <UINT:threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_queue_id_qid_all_drop_threshold_threshold(cparser_context_t *context,
    char **qid_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 4), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        thresh.high = *threshold_ptr;
        thresh.low = *threshold_ptr;
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThresh_set(unit, queue, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_CPU_QUEUE_ID_QID_ALL_DROP_THRESHOLD_HIGH_LOW_THRESHOLD
/* 
 * flowctrl set egress cpu queue-id ( <MASK_LIST:qid> | all ) drop-threshold ( high | low ) <UINT:threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_cpu_queue_id_qid_all_drop_threshold_high_low_threshold(cparser_context_t *context,
    char **qid_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 5), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrCpuQueueDropThresh_get(unit, queue, &thresh), ret);

        switch(TOKEN_CHAR(7,0))
        {
            case 'h':
                thresh.high = *threshold_ptr;
                break;

            case 'l':
                thresh.low = *threshold_ptr;
                break;                   
            
            default:
                diag_util_printf("User config: Error!\n");
                return CPARSER_NOT_OK;            
        }

        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrCpuQueueDropThresh_set(unit, queue, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PAGE
/*
* flowctrl get ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num page
*/
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_pause_on_allowed_num_page(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port, num;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
           
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Flow Control Port Pause On Allowed Page Number\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPageNum_get(unit, port, &num), ret);       
        diag_util_mprintf("\tPort %2d : %d\n", port, num);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PAGE_NUMBER
/*
* flowctrl set ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num page <UINT:number>
*/
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_pause_on_allowed_num_page_number(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *number_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_pauseOnAllowedPageNum_set(unit, port, *number_ptr), ret);       
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_THRESHOLD_GROUP_INDEX_FC_OFF_THRESHOLD_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD
/*
* flowctrl get ingress threshold-group <UINT:index> ( fc-off-threshold | fc-on-threshold ) ( high-off | high-on | low-off | low-on ) threshold
*/
cparser_result_t cparser_cmd_flowctrl_get_ingress_threshold_group_index_fc_off_threshold_fc_on_threshold_high_off_high_on_low_off_low_on_threshold(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32 unit, grp_idx, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;

    switch(TOKEN_CHAR(5,4))
    {
        case 'f': /* flow control OFF */
            fcon = 0;       
            break;

        case 'n': /* flow control ON */
            fcon = 1;      
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if (fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPauseThreshGroup_get(unit, grp_idx, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrCongestThreshGroup_get(unit, grp_idx, &thresh), ret);

    diag_util_mprintf("Group %d Ingress Flow Control %s Threshold\n", grp_idx, fcon ? "On" : "Off");
    switch(TOKEN_CHAR(6,0))
    {
        case 'h':
            if('f' == TOKEN_CHAR(6,7))
                diag_util_printf("\tHigh Off Threshold : 0x%X (%d)\n", thresh.highOff, thresh.highOff);
            else
                diag_util_printf("\tHigh On Threshold : 0x%X (%d)\n", thresh.highOn, thresh.highOn);
            break;

        case 'l':
            if('f' == TOKEN_CHAR(6,6))
                diag_util_printf("\tLow Off Threshold : 0x%X (%d)\n", thresh.lowOff, thresh.lowOff);
            else
                diag_util_printf("\tLow On Threshold : 0x%X (%d)\n", thresh.lowOn, thresh.lowOn);
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_THRESHOLD_GROUP_INDEX_FC_OFF_THRESHOLD_FC_ON_THRESHOLD_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESHOLD_THRESHOLD
/*
* flowctrl set ingress threshold-group <UINT:index> ( fc-off-threshold | fc-on-threshold ) ( high-off | high-on | low-off | low-on ) threshold <UINT:threshold>
*/
cparser_result_t cparser_cmd_flowctrl_set_ingress_threshold_group_index_fc_off_threshold_fc_on_threshold_high_off_high_on_low_off_low_on_threshold_threshold(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, grp_idx, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);        

    grp_idx = *index_ptr;

    switch(TOKEN_CHAR(5,4))
    {
        case 'f': /* flow control OFF */
            fcon = 0;       
            break;

        case 'n': /* flow control ON */
            fcon = 1;      
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if(fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPauseThreshGroup_get(unit, grp_idx, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrCongestThreshGroup_get(unit, grp_idx, &thresh), ret);

    switch(TOKEN_CHAR(6,0))
    {
        case 'h':
            if('f' == TOKEN_CHAR(6,7))
                thresh.highOff = *threshold_ptr;
            else
                thresh.highOn = *threshold_ptr;
            break;

        case 'l':
            if('f' == TOKEN_CHAR(6,6))
                thresh.lowOff = *threshold_ptr;
            else
                thresh.lowOn = *threshold_ptr;
            break;  
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    if(fcon)
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPauseThreshGroup_set(unit, grp_idx, &thresh), ret);
    else
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrCongestThreshGroup_set(unit, grp_idx, &thresh), ret);

    return CPARSER_OK;      
}
#endif


#ifdef CMD_FLOWCTRL_GET_INGRESS_QUEUE_QUEUE_ID_THRESHOLD_GROUP_INDEX_HIGH_LOW_FC_THRESHOLD
/*
* flowctrl get ingress queue <UINT:queue_id> threshold-group <UINT:index> ( high | low ) fc-threshold
*/
cparser_result_t cparser_cmd_flowctrl_get_ingress_queue_queue_id_threshold_group_index_high_low_fc_threshold(cparser_context_t *context,
    uint32_t *queue_id_ptr,
    uint32_t *index_ptr)
{
    uint32 unit, grp_idx, queue_id;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;
    queue_id = *queue_id_ptr;

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueuePauseThreshGroup_get(unit, grp_idx, queue_id, &thresh), ret);

    diag_util_mprintf("Group %d Ingress Queue %d Flow Control Threshold\n", grp_idx, queue_id);
    switch(TOKEN_CHAR(7,0))
    {
        case 'h':
            diag_util_printf("\tHigh Threshold : 0x%X (%d)\n", thresh.high, thresh.high);
            break;

        case 'l':
            diag_util_printf("\tLow Threshold : 0x%X (%d)\n", thresh.low, thresh.low);
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_QUEUE_QUEUE_ID_THRESHOLD_GROUP_INDEX_HIGH_LOW_FC_THRESHOLD_THRESHOLD
/*
* flowctrl set ingress queue <UINT:queue_id> threshold-group <UINT:index> ( high | low ) fc-threshold <UINT:threshold>
*/
cparser_result_t cparser_cmd_flowctrl_set_ingress_queue_queue_id_threshold_group_index_high_low_fc_threshold_threshold(cparser_context_t *context,
    uint32_t *queue_id_ptr,
    uint32_t *index_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, grp_idx, queue_id;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;
    queue_id = *queue_id_ptr;
    

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueuePauseThreshGroup_get(unit, grp_idx, queue_id, &thresh), ret);

    switch(TOKEN_CHAR(7,0))
    {
        case 'h':
            thresh.high = *threshold_ptr;
            break;

        case 'l':
            thresh.low = *threshold_ptr;
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueuePauseThreshGroup_set(unit, grp_idx, queue_id, &thresh), ret);

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_QUEUE_QUEUE_ID_THRESHOLD_GROUP_INDEX_HIGH_LOW_DROP_THRESHOLD
/*
* flowctrl get ingress queue <UINT:queue_id> threshold-group <UINT:index> ( high | low ) drop-threshold
*/
cparser_result_t cparser_cmd_flowctrl_get_ingress_queue_queue_id_threshold_group_index_high_low_drop_threshold(cparser_context_t *context,
    uint32_t *queue_id_ptr,
    uint32_t *index_ptr)
{
    uint32 unit, grp_idx, queue_id;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;
    queue_id = *queue_id_ptr;

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueueDropThreshGroup_get(unit, grp_idx, queue_id, &thresh), ret);

    diag_util_mprintf("Group %d Ingress Queue %d Drop Threshold\n", grp_idx, queue_id);
    switch(TOKEN_CHAR(7,0))
    {
        case 'h':
            diag_util_printf("\tHigh Threshold : 0x%X (%d)\n", thresh.high, thresh.high);
            break;

        case 'l':
            diag_util_printf("\tLow Threshold : 0x%X (%d)\n", thresh.low, thresh.low);
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_QUEUE_QUEUE_ID_THRESHOLD_GROUP_INDEX_HIGH_LOW_DROP_THRESHOLD_THRESHOLD
/*
* flowctrl set ingress queue <UINT:queue_id> threshold-group <UINT:index> ( high | low ) drop-threshold <UINT:threshold>
*/
cparser_result_t cparser_cmd_flowctrl_set_ingress_queue_queue_id_threshold_group_index_high_low_drop_threshold_threshold(cparser_context_t *context,
    uint32_t *queue_id_ptr,
    uint32_t *index_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, grp_idx, queue_id;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;
    queue_id = *queue_id_ptr;
    

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueueDropThreshGroup_get(unit, grp_idx, queue_id, &thresh), ret);

    switch(TOKEN_CHAR(7,0))
    {
        case 'h':
            thresh.high = *threshold_ptr;
            break;

        case 'l':
            thresh.low = *threshold_ptr;
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueueDropThreshGroup_set(unit, grp_idx, queue_id, &thresh), ret);

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_GET_IGR_QUEUE_PORT_PORTS_ALL_THRESHOLD_GROUP
/*
* flowctrl get igr-queue port ( <PORT_LIST:ports> | all ) threshold-group
*/
cparser_result_t cparser_cmd_flowctrl_get_igr_queue_port_ports_all_threshold_group(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    uint32 grp_idx;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Ingress Queue Flow Control & Drop Threshold Group of Port Configuration\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueuePauseDropThreshGroupSel_get(unit, port, &grp_idx), ret);  
        diag_util_mprintf("\tPort %2d : %d\n", port, grp_idx);
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_IGR_QUEUE_PORT_PORTS_ALL_THRESHOLD_GROUP_INDEX
/*
*flowctrl set igr-queue port ( <PORT_LIST:ports> | all ) threshold-group <UINT:index>
*/
cparser_result_t cparser_cmd_flowctrl_set_igr_queue_port_ports_all_threshold_group_index(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *index_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set(unit, port, *index_ptr), ret);  
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_QUEUE_QUEUE_ID_THRESHOLD_GROUP_INDEX_HIGH_LOW_DROP_THRESHOLD
/*
* flowctrl get egress queue <UINT:queue_id> threshold-group <UINT:index> ( high | low ) drop-threshold
*/
cparser_result_t cparser_cmd_flowctrl_get_egress_queue_queue_id_threshold_group_index_high_low_drop_threshold(cparser_context_t *context,
    uint32_t *queue_id_ptr,
    uint32_t *index_ptr)
{
    uint32 unit, grp_idx, queue_id;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;
    queue_id = *queue_id_ptr;

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThreshGroup_get(unit, grp_idx, queue_id, &thresh), ret);

    diag_util_mprintf("Group %d Egress Queue %d Drop Threshold\n", grp_idx, queue_id);
    switch(TOKEN_CHAR(7,0))
    {
        case 'h':
            diag_util_printf("\tHigh Threshold : 0x%X (%d)\n", thresh.high, thresh.high);
            break;

        case 'l':
            diag_util_printf("\tLow Threshold : 0x%X (%d)\n", thresh.low, thresh.low);
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_QUEUE_QUEUE_ID_THRESHOLD_GROUP_INDEX_HIGH_LOW_DROP_THRESHOLD_THRESHOLD
/*
* flowctrl set egress queue <UINT:queue_id> threshold-group <UINT:index> ( high | low ) drop-threshold <UINT:threshold>
*/
cparser_result_t cparser_cmd_flowctrl_set_egress_queue_queue_id_threshold_group_index_high_low_drop_threshold_threshold(cparser_context_t *context,
    uint32_t *queue_id_ptr,
    uint32_t *index_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, grp_idx, queue_id;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;
    queue_id = *queue_id_ptr;
    

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThreshGroup_get(unit, grp_idx, queue_id, &thresh), ret);

    switch(TOKEN_CHAR(7,0))
    {
        case 'h':
            thresh.high = *threshold_ptr;
            break;

        case 'l':
            thresh.low = *threshold_ptr;
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThreshGroup_set(unit, grp_idx, queue_id, &thresh), ret);

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_THRESHOLD_GROUP_INDEX_HIGH_LOW_DROP_THRESHOLD
/*
* flowctrl get egress port threshold-group <UINT:index> ( high | low ) drop-threshold
*/
cparser_result_t cparser_cmd_flowctrl_get_egress_port_threshold_group_index_high_low_drop_threshold(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32 unit, grp_idx;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThreshGroup_get(unit, grp_idx, &thresh), ret);

    diag_util_mprintf("Group %d Egress Port Drop Threshold\n", grp_idx);
    switch(TOKEN_CHAR(6,0))
    {
        case 'h':
            diag_util_printf("\tHigh Threshold : 0x%X (%d)\n", thresh.high, thresh.high);
            break;

        case 'l':
            diag_util_printf("\tLow Threshold : 0x%X (%d)\n", thresh.low, thresh.low);
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_THRESHOLD_GROUP_INDEX_HIGH_LOW_DROP_THRESHOLD_THRESHOLD
/*
* flowctrl set egress port threshold-group <UINT:index> ( high | low ) drop-threshold <UINT:threshold>
*/
cparser_result_t cparser_cmd_flowctrl_set_egress_port_threshold_group_index_high_low_drop_threshold_threshold(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *threshold_ptr)
{
    uint32 unit, grp_idx;
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *index_ptr;   

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThreshGroup_get(unit, grp_idx, &thresh), ret);

    switch(TOKEN_CHAR(6,0))
    {
        case 'h':
            thresh.high = *threshold_ptr;
            break;

        case 'l':
            thresh.low = *threshold_ptr;
            break;
        
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThreshGroup_set(unit, grp_idx, &thresh), ret);

    return CPARSER_OK;  
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGR_PORT_QUEUE_PORT_PORTS_ALL_THRESHOLD_GROUP
/*
*flowctrl get egr-port-queue port ( <PORT_LIST:ports> | all ) threshold-group
*/
cparser_result_t cparser_cmd_flowctrl_get_egr_port_queue_port_ports_all_threshold_group(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    uint32 grp_idx;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Egress Queue & Port Drop Threshold Group of Port Configuration\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortQueueDropThreshGroupSel_get(unit, port, &grp_idx), ret);  
        diag_util_mprintf("\tPort %2d : %d\n", port, grp_idx);
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGR_PORT_QUEUE_PORT_PORTS_ALL_THRESHOLD_GROUP_INDEX
/*
*flowctrl set egr-port-queue port ( <PORT_LIST:ports> | all ) threshold-group <UINT:index>
*/
cparser_result_t cparser_cmd_flowctrl_set_egr_port_queue_port_ports_all_threshold_group_index(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *index_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortQueueDropThreshGroupSel_set(unit, port, *index_ptr), ret);  
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_DROP_STATE
/*
* flowctrl get egress port ( <PORT_LIST:ports> | all ) drop state
*/
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_drop_state(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Flow Control Egress Port Drop Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropEnable_get(unit, port, &enable), ret);  
        diag_util_mprintf("\tPort %2d : %s\n", port, enable ? "ENABLE" : "DISABLE");
    }  

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_DROP_STATE_DISABLE_ENABLE
/*
* flowctrl set egress port ( <PORT_LIST:ports> | all ) drop state ( disable | enable )
*/
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_drop_state_disable_enable(cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(7,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropEnable_set(unit, port, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_QUEUE_QUEUE_ID_DROP_STATE
/*
* flowctrl get egress port ( <PORT_LIST:ports> | all ) queue <UINT:queue_id> drop state
*/
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_queue_queue_id_drop_state(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *queue_id_ptr)
{
    uint32 unit, port, queue_id;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();

    queue_id = *queue_id_ptr;

    diag_util_mprintf("Flow Control Egress Queue Drop Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropEnable_get(unit, port, queue_id, &enable), ret);  
        diag_util_mprintf("\tPort %2d Queue %2d: %s\n", port, queue_id, enable ? "ENABLE" : "DISABLE");
    }  

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_QUEUE_QUEUE_ID_DROP_STATE_DISABLE_ENABLE
/*
* flowctrl set egress port ( <PORT_LIST:ports> | all ) queue <UINT:queue_id> drop state ( disable | enable )
*/
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_queue_queue_id_drop_state_disable_enable(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *queue_id_ptr)
{
    uint32 unit, port, queue_id;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    queue_id = *queue_id_ptr;
        
    switch(TOKEN_CHAR(9,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropEnable_set(unit, port, queue_id, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_QUEUE_QUEUE_ID_DROP_STATE
/*
*flowctrl get egress port ( <PORT_LIST:ports> | all ) queue <UINT:queue_id> drop state
*/
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_queue_queue_id_drop_state(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *queue_id_ptr)
{
    uint32 unit, port, queue_id;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);   
    DIAG_UTIL_OUTPUT_INIT();

    queue_id = *queue_id_ptr;

    diag_util_mprintf("Flow Control Ingress Queue Drop Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueueDropEnable_get(unit, port, queue_id, &enable), ret);  
        diag_util_mprintf("\tPort %2d Queue %2d: %s\n", port, queue_id, enable ? "ENABLE" : "DISABLE");
    }  

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_QUEUE_QUEUE_ID_DROP_STATE_DISABLE_ENABLE
/*
*flowctrl set egress port ( <PORT_LIST:ports> | all ) queue <UINT:queue_id> drop state ( disable | enable )
*/
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_queue_queue_id_drop_state_disable_enable(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *queue_id_ptr)
{
    uint32 unit, port, queue_id;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    queue_id = *queue_id_ptr;

    switch(TOKEN_CHAR(9,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrQueueDropEnable_set(unit, port, queue_id, enable), ret);       
    }    

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_ALGORITHM_WRED
/*
 * flowctrl set congest-avoidance algorithm wred
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_algorithm_wred(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_congAvoidAlgo_t algo;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    algo = CONG_AVOID_WRED;
    
    if ((ret = rtk_qos_congAvoidAlgo_set(unit, algo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_ALGORITHM_WTD
/*
 * flowctrl set congest-avoidance algorithm wtd
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_algorithm_wtd(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_congAvoidAlgo_t algo;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    algo = CONG_AVOID_WTD;
    
    if ((ret = rtk_qos_congAvoidAlgo_set(unit, algo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_ALGORITHM_SWRED
/*
 * flowctrl set congest-avoidance algorithm swred
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_algorithm_swred(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_ALGORITHM_SRED
/*
*flowctrl set congest-avoidance algorithm sred
*/
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_algorithm_sred(cparser_context_t *context)
{
    uint32      unit = 0;
    rtk_qos_congAvoidAlgo_t algo;
    int32       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    algo = CONG_AVOID_SRED;
    
    if ((ret = rtk_qos_congAvoidAlgo_set(unit, algo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_ALGORITHM_TD
/*
 * flowctrl set congest-avoidance algorithm td
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_algorithm_td(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_SET_WRED_SYSTEM_THRESHOLD_DROP_PRECEDENCE_DP_MAX_THRESHOLD_MAX_THRESHOLD_MIN_THRESHOLD_MIN_THRESHOLD
/*
 * qos set wred system-threshold <UINT:drop_precedence> <UINT:max_threshold> <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_wred_system_threshold_drop_precedence_dp_max_threshold_max_threshold_min_threshold_min_threshold(cparser_context_t *context,
    uint32_t *dp_ptr,
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
    
    if ((ret = rtk_qos_wredSysThresh_set(unit, *dp_ptr, &thresh)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_WRED_MPD_MPD_VALUE
/*
 * qos set wred mpd <UINT:mpd_value>
 */
cparser_result_t cparser_cmd_flowctrl_set_wred_mpd_mpd_value(cparser_context_t *context,
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

#ifdef CMD_FLOWCTRL_SET_WRED_ECN_STATE_DISABLE_ENABLE
/*
 * flowctrl set wred ecn state ( disable | enable )
 */
cparser_result_t cparser_cmd_flowctrl_set_wred_ecn_state_disable_enable(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_SET_WRED_COUNTER_REVERSE_STATE_DISABLE_ENABLE
/*
 * flowctrl set wred counter-reverse state ( disable | enable )
 */
cparser_result_t cparser_cmd_flowctrl_set_wred_counter_reverse_state_disable_enable(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_CONGEST_AVOIDANCE
/*
 * flowctrl get congest-avoidance
 */
cparser_result_t cparser_cmd_flowctrl_get_congest_avoidance(cparser_context_t *context)
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
    
    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
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

#ifdef CMD_FLOWCTRL_GET_CONGEST_AVOIDANCE_PORT_THRESHOLD_PORT_PORTS_ALL
/*
 * flowctrl get congest-avoidance port-threshold port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_flowctrl_get_congest_avoidance_port_threshold_port_ports_all(cparser_context_t *context,
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
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
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

#ifdef CMD_FLOWCTRL_GET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_PORT_PORTS_ALL
/*
 * flowctrl get congest-avoidance queue-threshold port ( <PORT_LIST:ports> | all )
 */
cparser_result_t cparser_cmd_flowctrl_get_congest_avoidance_queue_threshold_port_ports_all(cparser_context_t *context,
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
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
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

#ifdef CMD_FLOWCTRL_GET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_QUEUE_ID_QID_ALL
/*
 * flowctrl get congest-avoidance queue-threshold queue-id ( <MASK_LIST:qid> | all )
 */
cparser_result_t cparser_cmd_flowctrl_get_congest_avoidance_queue_threshold_queue_id_qid_all(cparser_context_t *context,
    char **qid_ptr)
{
    uint32      unit = 0, queue, probability;
    int32       ret = RT_ERR_FAILED;
    uint32      dp;
    diag_mask_t queueMask;
    rtk_qos_congAvoidThresh_t   threshold;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 5), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
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

#ifdef CMD_FLOWCTRL_GET_CONGEST_AVOIDANCE_SYSTEM_THRESHOLD
/*
 * flowctrl get congest-avoidance system-threshold
 */
cparser_result_t cparser_cmd_flowctrl_get_congest_avoidance_system_threshold(cparser_context_t *context)
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

        if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID))
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

#ifdef CMD_FLOWCTRL_GET_WRED
/*
 * flowctrl get wred
 */
cparser_result_t cparser_cmd_flowctrl_get_wred(cparser_context_t *context)
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
    
    if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
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

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_PORT_THRESHOLD_PORT_PORTS_ALL_DROP_PRECEDENCE_DP_MAX_THRESHOLD_MAX_THRESHOLD_MIN_THRESHOLD_MIN_THRESHOLD
/*
 * flowctrl set congest-avoidance port-threshold port ( <PORT_LIST:ports> | all ) drop-precedence <UINT:dp> max-threshold <UINT:max_threshold> min-threshold <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_port_threshold_port_ports_all_drop_precedence_dp_max_threshold_max_threshold_min_threshold_min_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *dp_ptr,
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
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_congAvoidPortThresh_set(unit, port, *dp_ptr, &thresh)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_PORT_PORTS_ALL_QUEUE_ID_QID_MAX_THRESHOLD_MAX_THRESHOLD_MIN_THRESHOLD_MIN_THRESHOLD
/*
 * flowctrl set congest-avoidance queue-threshold port ( <PORT_LIST:ports> | all ) queue-id <UINT:qid> max-threshold <UINT:max_threshold> min-threshold <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_queue_threshold_port_ports_all_queue_id_qid_max_threshold_max_threshold_min_threshold_min_threshold(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *qid_ptr,
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
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_qos_congAvoidQueueThresh_set(unit, port, *qid_ptr, &thresh)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_QUEUE_ID_QID_ALL_DROP_PRECEDENCE_DP_MAX_THRESHOLD_MAX_THRESHOLD_MIN_THRESHOLD_MIN_THRESHOLD
/*
 * flowctrl set congest-avoidance queue-threshold queue-id ( <MASK_LIST:qid> | all ) drop-precedence <UINT:dp> max-threshold <UINT:max_threshold> min-threshold <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_queue_threshold_queue_id_qid_all_drop_precedence_dp_max_threshold_max_threshold_min_threshold_min_threshold(cparser_context_t *context,
    char **qid_ptr,
    uint32_t *dp_ptr,
    uint32_t *max_threshold_ptr,
    uint32_t *min_threshold_ptr)
{
    uint32      unit = 0, queue;
    int32       ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_qos_congAvoidThresh_t   thresh;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 5), ret);
    
    thresh.maxThresh = *max_threshold_ptr;
    thresh.minThresh = *min_threshold_ptr;

    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        if ((ret = rtk_qos_congAvoidGlobalQueueThresh_set(unit, queue, *dp_ptr, &thresh)) != RT_ERR_OK)
        {  
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_QUEUE_THRESHOLD_QUEUE_ID_QID_ALL_DROP_PRECEDENCE_DP_DROP_PROBABILITY_PROBABILITY
/*
 * flowctrl set congest-avoidance queue-threshold queue-id ( <MASK_LIST:qid> | all ) drop-precedence <UINT:dp> drop-probability <UINT:probability>
 */    
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_queue_threshold_queue_id_qid_all_drop_precedence_dp_drop_probability_probability(cparser_context_t *context,
    char **qid_ptr,
    uint32_t *dp_ptr,
    uint32_t *probability_ptr)
{
    uint32      unit = 0, queue;
    int32       ret = RT_ERR_FAILED;
    diag_mask_t queueMask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 5), ret);
    
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        if ((ret = rtk_qos_congAvoidGlobalQueueDropProbability_set(unit, queue, *dp_ptr, *probability_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_SYSTEM_THRESHOLD_DROP_PRECEDENCE_DP_MAX_THRESHOLD_MAX_THRESHOLD_MIN_THRESHOLD_MIN_THRESHOLD
/*
 * flowctrl set congest-avoidance system-threshold drop-precedence <UINT:dp> max-threshold <UINT:max_threshold> min-threshold <UINT:min_threshold>
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_system_threshold_drop_precedence_dp_max_threshold_max_threshold_min_threshold_min_threshold(cparser_context_t *context,
    uint32_t *dp_ptr,
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
    
    if ((ret = rtk_qos_congAvoidSysThresh_set(unit, *dp_ptr, &thresh)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_SYSTEM_THRESHOLD_DROP_PRECEDENCE_DP_DROP_PROBABILITY_PROBABILITY
/*
 * flowctrl set congest-avoidance system-threshold drop-precedence <UINT:dp> drop-probability <UINT:probability>
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_system_threshold_drop_precedence_dp_drop_probability_probability(cparser_context_t *context,
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

#ifdef CMD_FLOWCTRL_SET_CONGEST_AVOIDANCE_THRESHOLD_SYSTEM_PORT_QUEUE_STATE_DISABLE_ENABLE
/*
 * flowctrl set congest-avoidance threshold ( system | port | queue ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_flowctrl_set_congest_avoidance_threshold_system_port_queue_state_disable_enable(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_HOL_TRAFFIC_DROP_PORT_PORTS_ALL_STATE
/*
 * flowctrl get hol-traffic-drop port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_flowctrl_get_hol_traffic_drop_port_ports_all_state(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("HOL Traffic Drop function Enable Status\n");

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_portHolTrafficDropEnable_get(unit, port, &enable), ret);
        diag_util_mprintf("\tPort %2d : %s\n", port, enable ? "ENABLE" : "DISABLE");
    }    

    return CPARSER_OK; 
}
#endif

#ifdef CMD_FLOWCTRL_SET_HOL_TRAFFIC_DROP_PORT_PORTS_ALL_STATE_DISABLE_ENABLE
/*
 * flowctrl set hol-traffic-drop port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_flowctrl_set_hol_traffic_drop_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char **ports_ptr)
{
    uint32 unit, port;       
    int32  ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_enable_t enable = DISABLED;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(6,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);  
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_portHolTrafficDropEnable_set(unit, port, enable), ret);       
    }    

    return CPARSER_OK; 
}
#endif

#ifdef CMD_FLOWCTRL_GET_HOL_TRAFFIC_DROP_TRAFFIC_TYPE_UNKNOWN_UCAST_L2_MCAST_IP_MCAST_BCAST_STATE
/*
 * flowctrl get hol-traffic-drop traffic-type ( unknown-ucast | l2-mcast | ip-mcast | bcast ) state
 */
cparser_result_t cparser_cmd_flowctrl_get_hol_traffic_drop_traffic_type_unknown_ucast_l2_mcast_ip_mcast_bcast_state(
    cparser_context_t *context)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
    rtk_flowctrl_holTrafficType_t type = HOL_TRAFFIC_TYPE_END;
    uint8  strType[20] = "";
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    switch(TOKEN_CHAR(4,0))
    {
        case 'u':
            type = HOL_TRAFFIC_TYPE_UNKN_UC;
            strcpy((char *)strType, "Unknown Unicast");
            break;

        case 'l':
            type = HOL_TRAFFIC_TYPE_L2_MC;
            strcpy((char *)strType, "L2 Multicast");
            break;       

        case 'i':
            type = HOL_TRAFFIC_TYPE_IP_MC;
            strcpy((char *)strType, "IP Multicast");
            break;

        case 'b':
            type = HOL_TRAFFIC_TYPE_BC;
            strcpy((char *)strType, "Broadcast");
            break;       
    }

    diag_util_mprintf("%s Drop Status\n", strType);
    DIAG_UTIL_ERR_CHK(rtk_flowctrl_holTrafficTypeDropEnable_get(unit, type, &enable), ret);       
    diag_util_mprintf("\t%s\n", enable ? "ENABLE" : "DISABLE");

    return CPARSER_OK; 
}
#endif

#ifdef CMD_FLOWCTRL_SET_HOL_TRAFFIC_DROP_TRAFFIC_TYPE_UNKNOWN_UCAST_L2_MCAST_IP_MCAST_BCAST_STATE_DISABLE_ENABLE
/*
 * flowctrl set hol-traffic-drop traffic-type ( unknown-ucast | l2-mcast | ip-mcast | bcast ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_flowctrl_set_hol_traffic_drop_traffic_type_unknown_ucast_l2_mcast_ip_mcast_bcast_state_disable_enable(
    cparser_context_t *context)
{
    uint32 unit;
    int32  ret = RT_ERR_FAILED;
    rtk_enable_t enable = DISABLED;
    rtk_flowctrl_holTrafficType_t type = HOL_TRAFFIC_TYPE_END;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    switch(TOKEN_CHAR(4,0))
    {
        case 'u':
            type = HOL_TRAFFIC_TYPE_UNKN_UC;
            break;

        case 'l':
            type = HOL_TRAFFIC_TYPE_L2_MC;
            break;       

        case 'i':
            type = HOL_TRAFFIC_TYPE_IP_MC;
            break;

        case 'b':
            type = HOL_TRAFFIC_TYPE_BC;
            break;       
    }

    switch(TOKEN_CHAR(6,0))
    {
        case 'd':
            enable = DISABLED;
            break;

        case 'e':
            enable = ENABLED;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }


    DIAG_UTIL_ERR_CHK(rtk_flowctrl_holTrafficTypeDropEnable_set(unit, type, enable), ret);       

    return CPARSER_OK; 
}
#endif

