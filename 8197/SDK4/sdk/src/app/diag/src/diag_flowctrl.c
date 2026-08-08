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
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_FLOWCTRL_GET_INGRESS_SYSTEM_FC_OFF_THRESH_FC_ON_THRESH_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON
/* 
 * flowctrl get ingress system ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low-off | low-on )
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_system_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_FC_OFF_THRESH_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) fc-off-thresh ( high-off | high-on | low-off | low-on )
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_fc_off_thresh_high_off_high_on_low_off_low_on(cparser_context_t *context,
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

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_FC_ON_THRESH_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON
/* 
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) fc-on-thresh ( high-off | high-on | low-off | low-on )
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_fc_on_thresh_high_off_high_on_low_off_low_on(cparser_context_t *context,
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

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_THRESH_GROUP
/*
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) thresh-group
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_thresh_group(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PKT
/*
 * flowctrl get ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num pkt
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_pause_on_allowed_num_pkt(cparser_context_t *context)
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
cparser_result_t cparser_cmd_flowctrl_get_ingress_port_ports_all_pause_on_allowed_len(cparser_context_t *context)
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
       
#ifdef CMD_FLOWCTRL_GET_INGRESS_THRESH_GROUP_GRP_IDX_FC_OFF_THRESH_FC_ON_THRESH_HIGH_OFF_HIGH_ON_LOW
/*
 * flowctrl get ingress thresh-group <UINT:grp_idx> ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_ingress_thresh_group_grp_idx_fc_off_thresh_fc_on_thresh_high_off_high_on_low(cparser_context_t *context,
    uint32_t *grp_idx_ptr)
{
    uint32 unit, grp_idx, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);     
    DIAG_UTIL_OUTPUT_INIT();
     
    grp_idx = *grp_idx_ptr;

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

#ifdef CMD_FLOWCTRL_GET_EGRESS_SYSTEM_DROP_THRESH_HIGH_LOW
/* 
 * flowctrl get egress system drop-thresh ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_system_drop_thresh_high_low(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_EGRESS_SYSTEM_DROP_THRESH
/* 
 * flowctrl get egress system drop-thresh
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_system_drop_thresh(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_DROP_THRESH_HIGH_LOW
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) drop-thresh ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_drop_thresh_high_low(cparser_context_t *context,
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

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_DROP_THRESH
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) drop-thresh
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_drop_thresh(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_EGRESS_PORT_PORTS_ALL_QUEUE_QUEUES_ALL_DROP_THRESH_HIGH_LOW
/* 
 * flowctrl get egress port ( <PORT_LIST:ports> | all ) queue ( <MASK_LIST:queues> | all ) drop-thresh ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_queue_queues_all_drop_thresh_high_low(cparser_context_t *context,
    char **ports_ptr,
    char **queues_ptr)
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
cparser_result_t cparser_cmd_flowctrl_get_egress_port_ports_all_queue_drop_state(cparser_context_t *context)
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

    diag_util_mprintf("Egress Flow Control Reference Ingress Congestion Enable Status\n");

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
    char  *pathStr[4] = { "BOTH", "RX", "TX", "DISABLE" };
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

#ifdef CMD_FLOWCTRL_GET_EGRESS_QUEUE_QUEUES_ALL_DROP_THRESH_HIGH_LOW
/* 
 * flowctrl get egress queue ( <MASK_LIST:queues> | all ) drop-thresh ( high | low )
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_queue_queues_all_drop_thresh_high_low(cparser_context_t *context,
    char **queues_ptr)
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

#ifdef CMD_FLOWCTRL_GET_EGRESS_QUEUE_QUEUES_ALL_DROP_THRESH
/* 
 * flowctrl get egress queue ( <MASK_LIST:queues> | all ) drop-thresh
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_queue_queues_all_drop_thresh(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_GET_EGRESS_CPU_QUEUE_QUEUES_ALL_DROP_THRESH
/* 
 * flowctrl get egress cpu queue ( <MASK_LIST:queues> | all ) drop-thresh
 */
cparser_result_t cparser_cmd_flowctrl_get_egress_cpu_queue_queues_all_drop_thresh(cparser_context_t *context)
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
        diag_util_printf("\tQueue %d Threshold : 0x%X (%d)\n", queue, thresh.high, thresh.high);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_INGRESS_SYSTEM_FC_OFF_THRESH_FC_ON_THRESH_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESH
/* 
 * flowctrl set ingress system ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_system_fc_off_thresh_fc_on_thresh_high_off_high_on_low_off_low_on_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
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
                thresh.highOff = *thresh_ptr;
            else
                thresh.highOn = *thresh_ptr;
            break;

        case 'l':
            if('f' == TOKEN_CHAR(5,5))
                thresh.lowOff = *thresh_ptr;
            else
                thresh.lowOn = *thresh_ptr;
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

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_FC_OFF_THRESH_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESH
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) fc-off-thresh ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */    
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_fc_off_thresh_high_off_high_on_low_off_low_on_thresh(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *thresh_ptr)
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
                    thresh.highOff = *thresh_ptr;
                else
                    thresh.highOn = *thresh_ptr;
                break;

            case 'l':
                if('f' == TOKEN_CHAR(6,5))
                    thresh.lowOff = *thresh_ptr;
                else
                    thresh.lowOn = *thresh_ptr;
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

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_FC_ON_THRESH_HIGH_OFF_HIGH_ON_LOW_OFF_LOW_ON_THRESH
/* 
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) fc-on-thresh ( high-off | high-on | low-off | low-on ) <UINT:thresh>
 */    
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_fc_on_thresh_high_off_high_on_low_off_low_on_thresh(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *thresh_ptr)
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
                    thresh.highOff = *thresh_ptr;
                else
                    thresh.highOn = *thresh_ptr;
                break;

            case 'l':
                if('f' == TOKEN_CHAR(6,5))
                    thresh.lowOff = *thresh_ptr;
                else
                    thresh.lowOn = *thresh_ptr;
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

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_THRESH_GROUP_GRP_IDX
/*
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) thresh-group <UINT:grp_idx>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_thresh_group_grp_idx(cparser_context_t *context,
    uint32_t *grp_idx_ptr)
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
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_igrPortPauseThreshGroupSel_set(unit, port, *grp_idx_ptr), ret);  
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

#ifdef CMD_FLOWCTRL_SET_INGRESS_THRESH_GROUP_GRP_IDX_FC_OFF_THRESH_FC_ON_THRESH_HIGH_OFF_HIGH_ON_LOW_THRESH
/*
 * flowctrl set ingress thresh-group <UINT:grp_idx> ( fc-off-thresh | fc-on-thresh ) ( high-off | high-on | low ) <UINT:thresh>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_thresh_group_grp_idx_fc_off_thresh_fc_on_thresh_high_off_high_on_low_thresh(cparser_context_t *context,
    uint32_t *grp_idx_ptr,
    uint32_t *thresh_ptr)
{
    uint32 unit, grp_idx, fcon;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);        

    grp_idx = *grp_idx_ptr;

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
                thresh.highOff = *thresh_ptr;
            else
                thresh.highOn = *thresh_ptr;
            break;

        case 'l':
            thresh.lowOn = *thresh_ptr;
            thresh.lowOff = *thresh_ptr;
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

#ifdef CMD_FLOWCTRL_SET_INGRESS_PORT_PORTS_ALL_PAUSE_ON_ALLOWED_NUM_PKT_NUMBER
/*
 * flowctrl set ingress port ( <PORT_LIST:ports> | all ) pause-on-allowed-num pkt <UINT:number>
 */
cparser_result_t cparser_cmd_flowctrl_set_ingress_port_ports_all_pause_on_allowed_num_pkt_number(cparser_context_t *context,
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

#ifdef CMD_FLOWCTRL_SET_EGRESS_SYSTEM_DROP_THRESH_HIGH_LOW_THRESH
/* 
 * flowctrl set egress system drop-thresh ( high | low ) <UINT:thresh>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_system_drop_thresh_high_low_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
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
            thresh.high = *thresh_ptr;
            break;

        case 'l':
            thresh.low = *thresh_ptr;
            break;       
            
        default:
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;            
    }

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrSystemDropThresh_set(unit, &thresh), ret);

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_SYSTEM_DROP_THRESH_THRESH
/* 
 * flowctrl set egress system drop-thresh <UINT:thresh>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_system_drop_thresh_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
{
    uint32 unit;       
    int32  ret = RT_ERR_FAILED;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);      
     
    thresh.high = *thresh_ptr;
    thresh.low = *thresh_ptr;

    DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrSystemDropThresh_set(unit, &thresh), ret);

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_DROP_THRESH_HIGH_LOW_THRESH
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) drop-thresh ( high | low ) <UINT:thresh>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_drop_thresh_high_low_thresh(cparser_context_t *context,
    char **ports_ptr,
    uint32_t *thresh_ptr)
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
                thresh.high = *thresh_ptr;
                break;

            case 'l':
                thresh.low = *thresh_ptr;
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

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_DROP_THRESH_THRESH
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) drop-thresh <UINT:thresh>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_drop_thresh_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
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
        thresh.high = *thresh_ptr;
        thresh.low = *thresh_ptr;
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrPortDropThresh_set(unit, port, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_PORT_PORTS_ALL_QUEUE_QUEUES_ALL_DROP_THRESH_HIGH_LOW_THRESH
/* 
 * flowctrl set egress port ( <PORT_LIST:ports> | all ) queue ( <MASK_LIST:queues> | all ) drop-thresh ( high | low ) <UINT:thresh>
 */    
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_queue_queues_all_drop_thresh_high_low_thresh(cparser_context_t *context,
    char **ports_ptr,
    char **queues_ptr,
    uint32_t *thresh_ptr)
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
                    thresh.high = *thresh_ptr;
                    break;

                case 'l':
                    thresh.low = *thresh_ptr;
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
cparser_result_t cparser_cmd_flowctrl_set_egress_port_ports_all_queue_drop_state_disable_enable(cparser_context_t *context)
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

#ifdef CMD_FLOWCTRL_SET_EGRESS_QUEUE_QUEUES_ALL_DROP_THRESH_HIGH_LOW_THRESH
/* 
 * flowctrl set egress queue ( <MASK_LIST:queues> | all ) drop-thresh ( high | low ) <UINT:thresh>
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_queue_queues_all_drop_thresh_high_low_thresh(cparser_context_t *context,
    char **queues_ptr,
    uint32_t *thresh_ptr)
{
    uint32 unit, queue;       
    int32  ret = RT_ERR_FAILED;
    diag_mask_t queueMask;
    rtk_flowctrl_drop_thresh_t thresh;
       
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_ERR_CHK((DIAG_UTIL_EXTRACT_QUEUEMASK(queueMask, 4), ret);
    DIAG_UTIL_MASK_SCAN(queueMask, queue)
    {        
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThresh_get(unit, queue, &thresh), ret);

        switch(TOKEN_CHAR(6,0))
        {
            case 'h':
                thresh.high = *thresh_ptr;
                break;

            case 'l':
                thresh.low = *thresh_ptr;
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

#ifdef CMD_FLOWCTRL_SET_EGRESS_QUEUE_QUEUES_ALL_DROP_THRESH_THRESH
/* 
 * flowctrl set egress queue ( <MASK_LIST:queues> | all ) drop-thresh <UINT:thresh>
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_queue_queues_all_drop_thresh_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
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
        thresh.high = *thresh_ptr;
        thresh.low = *thresh_ptr;
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrQueueDropThresh_set(unit, queue, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif

#ifdef CMD_FLOWCTRL_SET_EGRESS_CPU_QUEUE_QUEUES_ALL_DROP_THRESH_THRESH
/* 
 * flowctrl set egress cpu queue ( <MASK_LIST:queues> | all ) drop-thresh <UINT:thresh>
 */
cparser_result_t cparser_cmd_flowctrl_set_egress_cpu_queue_queues_all_drop_thresh_thresh(cparser_context_t *context,
    uint32_t *thresh_ptr)
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
        thresh.high = *thresh_ptr;
        thresh.low = *thresh_ptr;
        DIAG_UTIL_ERR_CHK(rtk_flowctrl_egrCpuQueueDropThresh_set(unit, queue, &thresh), ret);
    }

    return CPARSER_OK;    
}
#endif
