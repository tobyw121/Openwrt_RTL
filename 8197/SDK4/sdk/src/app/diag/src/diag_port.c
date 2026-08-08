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
 * $Revision: 31485 $
 * $Date: 2012-07-30 14:33:33 +0800 (Mon, 30 Jul 2012) $
 *
 * Purpose : Definition those Port command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Parameter settings for the port-based view
 *           2) RTCT
 *           3) UDLD
 *           4) RLDP
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
#include <rtk/diag.h>
#include <rtk/port.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#define UTIL_STRING_BUFFER_LENGTH   (30)
#define MAX_PHY_REGISTER  (31)
#define ABILITY_BIT_ON      (1)
#define ABILITY_BIT_OFF     (0)

#ifdef CMD_PORT_DUMP_CPU_PORT
/* 
 * port dump cpu-port
 */    
cparser_result_t cparser_cmd_port_dump_cpu_port(cparser_context_t *context)
{
    uint32                  unit = 0;
    rtk_port_t             port = 0;
    int32                   ret = RT_ERR_FAILED;        
        
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_UTIL_OUTPUT_INIT();
    
    /* show cpu port numner */
    DIAG_UTIL_ERR_CHK(rtk_port_cpuPortId_get(unit, &port), ret);
    diag_util_mprintf("CPU port: %d\n", port);  
    
    return CPARSER_OK;
}    
#endif    

#ifdef CMD_PORT_DUMP_ISOLATION_PORT_ALL
/* 
 * port dump isolation ( <PORT_LIST:port> | all )
 */       
cparser_result_t cparser_cmd_port_dump_isolation_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    rtk_portmask_t      portmask;
    uint8               port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    diag_portlist_t  portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();
    
    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);  

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&portmask, 0, sizeof(rtk_portmask_t));
        DIAG_UTIL_ERR_CHK(rtk_port_isolation_get(unit, port, &portmask), ret);
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN * sizeof(uint8));
        diag_util_lPortMask2str(port_list, &portmask);
        diag_util_mprintf("\tPort %2d : Isolation Port list %s\n", port, port_list);
    }
    return CPARSER_OK;
}
#endif


void cmd_print_blank(uint8 max, uint8 min){

    uint8 i = 0;
    uint8 len = max - min ;
    if(len > 0){
        for(i = 0 ; i < len ; i++){
            diag_util_printf(" ");
        }
        diag_util_printf("\t: ");
    
    }else if(len == 0){
        diag_util_printf("\t: ");
    }

}

#ifdef CMD_PORT_DUMP_ISOLATION_VLAN_BASED
/*
 * port dump isolation vlan-based
 */
cparser_result_t cparser_cmd_port_dump_isolation_vlan_based(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          index, maxIndex;
    uint32          total_entry = 0;   
    rtk_port_vlanIsolationEntry_t entry;
    uint8               port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_OM_GET_CHIP_CAPACITY(unit, maxIndex, max_num_of_vlan_port_iso_entry);


    diag_util_mprintf("Index |VID  |Trust Port list       |State\n");
    diag_util_mprintf("------+-----+----------------------+---------\n");

    for (index = 0; index < maxIndex; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_get(unit, index, &entry), ret);
            
        if (entry.vid != 0)
        {
            memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN * sizeof(uint8));
            diag_util_lPortMask2str(port_list, &entry.portmask);

            diag_util_mprintf("%4d  |%4d | %20s | %s   \n", index, entry.vid, 
                    port_list, (entry.enable == ENABLED ? "Enable" : "Disable"));
            total_entry++;
        }
    }    
    
    diag_util_mprintf("\nTotal Number Of Entries : %d\n",total_entry);

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_DUMP_PORT_ALL
cparser_result_t cparser_cmd_port_dump_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    uint8                   max_len = 0;
    rtk_enable_t            enabled = DISABLED;
    rtk_port_phy_ability_t  ability;
    rtk_port_speed_t        speed = PORT_SPEED_10M;
    rtk_port_duplex_t       duplex = PORT_HALF_DUPLEX;
    rtk_port_linkStatus_t   link_status = PORT_LINKDOWN;
    rtk_port_media_t        media;  
    diag_portlist_t               portlist;
    rtk_enable_t              autoNego;
    rtk_enable_t              txStatus, rxStatus;
    rtk_port_crossOver_mode_t   mode;

    char media_cmd[32]= "\tMedia";
    char admin_cmd[32] = "\tAdmin";
    char macTx_cmd[32] = "\tMac Tx";
    char macRx_cmd[32] = "\tMac Rx";
    char remoteLoopback_cmd[32] = "\tMac Remote Loopback";
    char localLoopback_cmd[32] =  "\tMac Local Loopback";
    char pressure_cmd[32] = "\tBack Pressure";
    char link_cmd[32] = "\tLink";
    char autoNego_cmd[32] = "\tAutoNego";
    char autoNegoAbility_cmd[32] = "\tAutoNego Ability";
    char flowCtrl_cmd[32] = "\tFlow Control (actual)";
    char forceMode_cmd[32] = "\tForce Mode Ability";
    char forceCtrlConf_cmd[32] = "\tFlow Control (config)";
    char crossOverMode_cmd[32] = "\tCross Over Mode";


    max_len = strlen(flowCtrl_cmd);

    DIAG_OM_GET_CHIP_ID(unit);  
        
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&ability, 0, sizeof(rtk_port_phy_ability_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2), ret);
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
        diag_util_mprintf("Port %2d :\n", port);
        
        diag_util_printf("%s",media_cmd);
        cmd_print_blank(max_len,strlen(media_cmd));        

        DIAG_UTIL_ERR_CHK(rtk_port_phyComboPortMedia_get(unit, port, &media), ret);
        if (PORT_MEDIA_COPPER == media)
        {
            diag_util_printf("Copper\n");
        }    
        else if (PORT_MEDIA_FIBER == media)
        {
            diag_util_printf("Fiber\n");
        }
        else if (PORT_MEDIA_COPPER_AUTO == media)
        {
            diag_util_printf("Auto-Copper\n");
        }
        else if (PORT_MEDIA_FIBER_AUTO == media)
        {
            diag_util_printf("Auto-Fiber\n");
        }
        else if (PORT_MEDIA_FIBER_AUTO_BY_GPIO == media)
        {
            diag_util_printf("Auto-Fiber-By-GPIO\n");
        }

        diag_util_printf("%s",admin_cmd);
        cmd_print_blank(max_len,strlen(admin_cmd));  

        DIAG_UTIL_ERR_CHK(rtk_port_adminEnable_get(unit, port, &enabled), ret);
        diag_util_printf("%s\n", enabled ? "ENABLE" : "DISABLE");

        diag_util_printf("%s",macTx_cmd);
        cmd_print_blank(max_len,strlen(macTx_cmd));  
        DIAG_UTIL_ERR_CHK(rtk_port_txEnable_get(unit, port, &enabled), ret);
        diag_util_printf("%s\n", enabled ? "ENABLE" : "DISABLE");

        diag_util_printf("%s",macRx_cmd);
        cmd_print_blank(max_len,strlen(macRx_cmd));  
        DIAG_UTIL_ERR_CHK(rtk_port_rxEnable_get(unit, port, &enabled), ret);
        diag_util_printf("%s\n", enabled ? "ENABLE" : "DISABLE");

        diag_util_printf("%s",remoteLoopback_cmd);
        cmd_print_blank(max_len,strlen(remoteLoopback_cmd)); 
        ret = rtk_diag_portMacRemoteLoopbackEnable_get(unit, port, &enabled);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_printf("%s\n", "No supported");
        else
            diag_util_printf("%s\n", enabled ? "ENABLE" : "DISABLE");

        diag_util_printf("%s",localLoopback_cmd);
        cmd_print_blank(max_len,strlen(localLoopback_cmd)); 
        ret = rtk_diag_portMacLocalLoopbackEnable_get(unit, port, &enabled);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_printf("%s\n", "No supported");
        else
            diag_util_printf("%s\n", enabled ? "ENABLE" : "DISABLE");

        diag_util_printf("%s",pressure_cmd);
        cmd_print_blank(max_len,strlen(pressure_cmd)); 
        DIAG_UTIL_ERR_CHK(rtk_port_backpressureEnable_get(unit, port, &enabled), ret);
        diag_util_mprintf("%s\n", enabled ? "ENABLE" : "DISABLE");

        diag_util_printf("%s",link_cmd);
        cmd_print_blank(max_len,strlen(link_cmd)); 
        DIAG_UTIL_ERR_CHK(rtk_port_link_get(unit, port, &link_status), ret);
        DIAG_UTIL_ERR_CHK(rtk_port_linkMedia_get(unit, port, &link_status, &media), ret);
        if (PORT_LINKUP == link_status)
        {
            diag_util_printf("UP (%s)", (PORT_MEDIA_COPPER == media)?"Copper":"Fiber");
            
            diag_util_printf("\tSpeed : ");
            if ((ret = rtk_port_speedDuplex_get(unit, port, &speed, &duplex)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            else
            {
                if (PORT_HALF_DUPLEX == duplex)
                {
                    if (PORT_SPEED_10M == speed)
                        diag_util_printf("10H ");
                    else if (PORT_SPEED_100M == speed)
                        diag_util_printf("100H ");
                    else if (PORT_SPEED_1000M == speed)
                        diag_util_printf("1000H ");
                    else
                    {
                        diag_util_printf("User config: Error!\n");
                        return CPARSER_NOT_OK;
                    }    
                }
                else if (PORT_FULL_DUPLEX == duplex)
                {
                    if (PORT_SPEED_10M == speed)
                        diag_util_printf("10F ");
                    else if (PORT_SPEED_100M == speed)
                        diag_util_printf("100F ");
                    else if (PORT_SPEED_1000M == speed)
                        diag_util_printf("1000F ");
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
            diag_util_printf("\n");
        }    
        else
        {
            diag_util_mprintf("DOWN\n");
        }

        DIAG_UTIL_ERR_CHK(rtk_port_phyAutoNegoEnable_get(unit, port, &autoNego), ret);
                
        diag_util_printf("%s",autoNego_cmd);
        cmd_print_blank(max_len,strlen(autoNego_cmd)); 
        diag_util_mprintf("%s\n", autoNego ? "ENABLE" : "DISABLE");
        
        diag_util_printf("%s",autoNegoAbility_cmd);
        cmd_print_blank(max_len,strlen(autoNegoAbility_cmd)); 
        DIAG_UTIL_ERR_CHK(rtk_port_phyAutoNegoAbility_get(unit, port, &ability), ret);
        if (ABILITY_BIT_ON == ability.Half_10)
            diag_util_printf("10H ");
        if (ABILITY_BIT_ON == ability.Full_10)
            diag_util_printf("10F ");
        if (ABILITY_BIT_ON == ability.Half_100)
            diag_util_printf("100H ");
        if (ABILITY_BIT_ON == ability.Full_100)
            diag_util_printf("100F ");
        if (ABILITY_BIT_ON == ability.Full_1000)
            diag_util_printf("1000F ");
        if (ABILITY_BIT_ON == ability.FC)
            diag_util_printf("Flow-Control ");
        if (ABILITY_BIT_ON == ability.AsyFC)
            diag_util_printf("Asy-Flow-Control ");
        diag_util_mprintf("\n");
        
        if(!autoNego)
        {

            diag_util_printf("%s",forceMode_cmd);
            cmd_print_blank(max_len,strlen(forceMode_cmd)); 
            //diag_util_mprintf("\tForce Mode Ability    : ");
            DIAG_UTIL_ERR_CHK(rtk_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &enabled), ret);
            if (PORT_HALF_DUPLEX == duplex)
            {
                if (PORT_SPEED_10M == speed)
                    diag_util_printf("10H ");
                else if (PORT_SPEED_100M == speed)
                    diag_util_printf("100H ");
                else if (PORT_SPEED_1000M == speed)
                    diag_util_printf("1000H ");
                else
                {
                    diag_util_printf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }    
            }
            else if (PORT_FULL_DUPLEX == duplex)
            {
                if (PORT_SPEED_10M == speed)
                    diag_util_printf("10F ");
                else if (PORT_SPEED_100M == speed)
                    diag_util_printf("100F ");
                else if (PORT_SPEED_1000M == speed)
                    diag_util_printf("1000F ");
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
            diag_util_mprintf("\n");
            diag_util_printf("%s",forceCtrlConf_cmd);
            cmd_print_blank(max_len,strlen(forceCtrlConf_cmd));
            diag_util_mprintf("%s\n", enabled ? "ENABLE" : "DISABLE");
        }        

        //DIAG_UTIL_ERR_CHK(rtk_port_flowctrl_get(unit, port, &txStatus, &rxStatus), ret);
        ret = rtk_port_flowctrl_get(unit, port, &txStatus, &rxStatus);
        //DIAG_ERR_PRINT(ret);
        if(ret != RT_ERR_PORT_LINKDOWN && ret == RT_ERR_OK){      
            diag_util_printf("%s",flowCtrl_cmd);
            cmd_print_blank(max_len,strlen(flowCtrl_cmd)); 
            diag_util_mprintf("%s\n", (txStatus & rxStatus) ? "ENABLE" : "DISABLE");
        }

        diag_util_printf("%s",crossOverMode_cmd);
        cmd_print_blank(max_len,strlen(crossOverMode_cmd));
        ret = rtk_port_phyCrossOverMode_get(unit, port, &mode);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("%s\n", "No supported");
        else
        {
            if (PORT_CROSSOVER_MODE_AUTO == mode)
            {
                diag_util_mprintf("Auto MDI/MDIX\n");
            }    
            else if (PORT_CROSSOVER_MODE_MDI == mode)
            {
                diag_util_mprintf("Force MDI\n");
            }
            else
            {
                diag_util_mprintf("Force MDIX\n");
            }
        }
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
        
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_DUMP_RLDP_PORT_ALL
/* 
 * port dump rldp ( <PORT_LIST:port> | all )
 */    
cparser_result_t cparser_cmd_port_dump_rldp_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                  port = 0;
    uint32                      interval;
    rtk_enable_t             enable;
    rtk_port_rldpNormalStatus_t     normalStatus;
    rtk_port_rldpSelfStatus_t          selfStatus;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        diag_util_mprintf("Port : %u\n", port);

        ret = rtk_port_rldpEnable_get(unit, port, &enable);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tRLDP Function               : %s\n", enable ? "ENABLE" : "DISABLE");  
        }

        ret = rtk_port_rldpAutoBlockEnable_get(unit, port, &enable);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tSelf Loop Auto Blocking     : %s\n", enable ? "ENABLE" : "DISABLE");   
        }

        ret = rtk_port_rldpInterval_get(unit, port, &interval);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tHello Time Interval         : %u seconds\n", interval);
        }

        ret = rtk_port_rldpSelfLoopAgingTime_get(unit, port, &interval);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tSelf Loop Aging Interval    : %u seconds\n", interval);
        }

        ret = rtk_port_rldpNormalLoopAgingTime_get(unit, port, &interval);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tNormal Loop Aging Interval  : %u seconds\n", interval);
        }

        ret = rtk_port_rldpStatus_get(unit, port, &normalStatus, &selfStatus);     
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            if(RLDP_NORMAL_LOOP  == normalStatus)
                diag_util_mprintf("\tNormal Loop Status          : Loop\n");
            else
                diag_util_mprintf("\tNormal Loop Status          : No loop\n");

            if(RLDP_FORWARDING == selfStatus)
                diag_util_mprintf("\tRLDP Port Status            : FORWARDING\n");
            else if(RLDP_LISTEN == selfStatus)
                diag_util_mprintf("\tRLDP Port Status            : LISTERNING\n");
            else
                diag_util_mprintf("\tRLDP Port Status            : BLOCKING\n");
        }
        
        diag_util_mprintf("\n");
    }  
        
    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_DUMP_UDLD_PORT_ALL
/* 
 * port dump udld { ( <PORT_LIST:port> | all ) }
 */    
cparser_result_t cparser_cmd_port_dump_udld_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                 port = 0;
    rtk_enable_t            enable = DISABLED;
    rtk_port_udldInterval_t   interval;
    uint32                      retryCount;
    rtk_port_udldEchoAction_t action;
    rtk_port_udldStatus_t    status;
    rtk_port_udldLinkStatus_t linkStatus;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    if(TOKEN_NUM < 4)  /*global*/
    {        
        diag_util_mprintf("\n");
        ret = rtk_port_udldLedIndicateEnable_get(unit, &enable);     
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tLed Indication                                      : %s\n", enable ? "ENABLE" : "DISABLE");
        }
        
        ret = rtk_port_udldAutoDisableFailedPortEnable_get(unit, &enable);     
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tAuto Disable Port When Unidirectional Link Detected : %s\n", enable ? "ENABLE" : "DISABLE");
        }
        
        ret = rtk_port_udldLinkUpAutoTriggerEnable_get(unit, &enable);     
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tLinkup Auto Trigger Udld                            : %s\n", enable ? "ENABLE" : "DISABLE");
        }

        ret = rtk_port_udldInterval_get(unit, &interval);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tUdld Packet Generator Interval                      : ");
            if(UDLD_INTERVAL_2S == interval)
                diag_util_mprintf("2s\n");
            else if(UDLD_INTERVAL_4S == interval)
                diag_util_mprintf("4s\n");
            else if(UDLD_INTERVAL_8S == interval)
                diag_util_mprintf("8s\n");
            else if(UDLD_INTERVAL_16S == interval)
                diag_util_mprintf("16s\n");
        }

        ret = rtk_port_udldRetryCount_get(unit, &retryCount);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            diag_util_mprintf("\tUdld Retry Count                                    : %u times\n", retryCount);
        }        

        /*echo packet action*/
        ret = rtk_port_udldEchoAction_get(unit, &action);        
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        } 
        else
        {
            if(UDLD_ACTION_SELFLOOPBACK == action)
                diag_util_mprintf("\tUdld Action                                         : LOOPBACK\n\n");
            else
                diag_util_mprintf("\tUdld Action                                         : FORWARD\n\n");
        }  
        
    }
    else /*per-port*/
    {
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {           
            diag_util_mprintf("Port : %u\n", port);

            ret = rtk_port_udldEnable_get(unit, port, &enable);        
            if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            } 
            else
            {
                diag_util_mprintf("\tUdld Function      : %s\n", enable ? "ENABLE" : "DISABLE");
            }  

            ret = rtk_port_udldStatus_get(unit, port, &status);        
            if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            } 
            else
            {
                if(UDLD_UNIDIR == status)
                    diag_util_mprintf("\tUdld Result        : Unidirectional Link\n");
                else if(UDLD_BIDIR == status)
                    diag_util_mprintf("\tUdld Result        : Bidirectional Link\n");
                else
                    diag_util_mprintf("\tUdld Result        : Not Finished, Pls Wait\n");
            }  

            ret = rtk_port_udldLinkStatus_get(unit, port, &linkStatus);        
            if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            } 
            else
            {
                if(UDLD_LINK_STATUS_NORMAL == linkStatus)
                    diag_util_mprintf("\tUdld Link Status   : Normal\n");
                else if(UDLD_LINK_STATUS_DISABLE== linkStatus)
                    diag_util_mprintf("\tUdld link Status   : Disabled\n");
            }  

            diag_util_mprintf("\n");
        }
    }   

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_GET_PHY_REG_PORT_ALL
/* 
 * port get phy-reg ( <PORT_LIST:port> | all )
 */ 
cparser_result_t cparser_cmd_port_get_phy_reg_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    uint32      reg_data = 0;
    uint32      reg_page = 0;
    uint32      reg_indx = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port : %d\n", port);
        
        for(reg_page = 0; reg_page < 32; reg_page++)
        {
            diag_util_mprintf("    Page %d : \n", reg_page);
            for(reg_indx = 0; reg_indx < 32; reg_indx++)
            {
                DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, reg_page, reg_indx, &reg_data), ret);
                if(0 == (reg_indx%4))
                {
                    diag_util_printf("0x%02X  ", reg_indx);
                }
                diag_util_printf("0x%04X  ", reg_data);         
                if(3 == (reg_indx%4))
                {
                    diag_util_mprintf("\n");
                }
            }
            diag_util_mprintf("\n");
        }   
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_PHY_REG_PORT_ALL_PAGE
/* 
 * port get phy-reg ( <PORT_LIST:port> | all ) <UINT:page>
 */ 
cparser_result_t cparser_cmd_port_get_phy_reg_port_all_page(cparser_context_t *context,
    char **port_ptr,
    uint32_t *page_ptr)
{
    uint32      unit = 0;
    uint32      reg_data = 0;
    uint32      reg_indx = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((page_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    //DIAG_UTIL_PARAM_RANGE_CHK((*page_ptr > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\n");

        diag_util_mprintf("Port : %d\n", port);
        
        diag_util_mprintf("    Page %d : \n", *page_ptr);
        for(reg_indx = 0; reg_indx < 32; reg_indx++)
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, *page_ptr, reg_indx, &reg_data), ret);
            if(0 == (reg_indx%4))
            {
                diag_util_printf("0x%02X  ", reg_indx);
            }
            diag_util_printf("0x%04X  ", reg_data);         
            if(3 == (reg_indx%4))
            {
                diag_util_mprintf("\n");
            }
        }
        
        diag_util_mprintf("\n");
    }

    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_PHY_REG_PORT_ALL_PAGE_REGISTER_NUMBER
/* 
 * port get phy-reg ( <PORT_LIST:port> | all ) <UINT:page> <UINT:register> { <UINT:number> }
 */     
cparser_result_t cparser_cmd_port_get_phy_reg_port_all_page_register_number(cparser_context_t *context,
    char **port_ptr,
    uint32_t *page_ptr,
    uint32_t *register_ptr,
    uint32_t *number_ptr)
{
    uint32      unit = 0;
    uint32      reg_indx, display_no;
    uint32      reg_data = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;
    uint32 index = 0;;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((page_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((register_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*register_ptr > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);

    if (TOKEN_NUM < 7)
        display_no = 1;
    else
        display_no = *number_ptr;

    DIAG_UTIL_PARAM_RANGE_CHK((display_no > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\n");

        diag_util_mprintf("Port : %d\n", port);
        
        diag_util_mprintf("    Page %d : \n", *page_ptr);
        index = 0;
        for(reg_indx = *register_ptr; reg_indx < ((((*register_ptr) + (display_no)) < 32) ? ((*register_ptr) + (display_no)) : 32); reg_indx++)
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyReg_get(unit, port, *page_ptr, reg_indx, &reg_data), ret);
            if(0 == (index%4))
            {
                diag_util_printf("        0x%02X  ", reg_indx);
            }
            diag_util_printf("        0x%04X  ", reg_data);         
            if(3 == (index%4))
            {
                diag_util_mprintf("\n");
            }

            index++;
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_PHY_EXT_REG_PORT_ALL_MAINPAGE_EXTPAGE_PARKPAGE_REGISTER_NUMBER
/*
 * port get phy-ext-reg ( <PORT_LIST:port> | all ) <UINT:mainPage> <UINT:extPage> <UINT:parkPage> <UINT:register> { <UINT:number> }
 */
cparser_result_t cparser_cmd_port_get_phy_ext_reg_port_all_mainPage_extPage_parkPage_register_number(cparser_context_t *context,
    uint32_t *mainPage_ptr,
    uint32_t *extPage_ptr,
    uint32_t *parkPage_ptr,
    uint32_t *register_ptr,
    uint32_t *number_ptr)
{
    uint32      unit = 0;
    uint32      reg_indx, display_no;
    uint32      reg_data = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;
    uint32 index = 0;;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((mainPage_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((extPage_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((parkPage_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((register_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*register_ptr > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);

    if (TOKEN_NUM < 9)
        display_no = 1;
    else
        display_no = *number_ptr;

    RT_PARAM_CHK((display_no > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\n");

        diag_util_mprintf("Port : %d\n", port);
        
        diag_util_mprintf("    Main Page %d : \n", *mainPage_ptr);
        diag_util_mprintf("    Ext Page %d : \n", *extPage_ptr);
        diag_util_mprintf("    Park Page %d : \n", *parkPage_ptr);
        index = 0;
        for(reg_indx = *register_ptr; reg_indx < ((((*register_ptr) + (display_no)) < 32) ? ((*register_ptr) + (display_no)) : 32); reg_indx++)
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyExtParkPageReg_get(unit, port, *mainPage_ptr, *extPage_ptr, *parkPage_ptr, reg_indx, &reg_data), ret);
            if(0 == (index%4))
            {
                diag_util_printf("        0x%02X  ", reg_indx);
            }
            diag_util_printf("        0x%04X  ", reg_data);         
            if(3 == (index%4))
            {
                diag_util_mprintf("\n");
            }

            index++;
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_phy_ext_reg_port_all_mainpage_extpage_parkpage_register_number */
#endif

#ifdef CMD_PORT_GET_PHY_MMD_REG_PORT_ALL_MMD_ADDR_MMD_REG_NUMBER
/*
 * port get phy-mmd-reg ( <PORT_LIST:port> | all ) <UINT:mmd-addr> <UINT:mmd-reg> { <UINT:number> }
 */
cparser_result_t
cparser_cmd_port_get_phy_mmd_reg_port_all_mmd_addr_mmd_reg_number(
    cparser_context_t *context,
    uint32_t  *mmd_addr_ptr,
    uint32_t  *mmd_reg_ptr,
    uint32_t  *number_ptr)
{
    uint32      unit = 0;
    uint32      reg_indx, display_no;
    uint32      reg_data = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;
    uint32 index = 0;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((mmd_addr_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((mmd_reg_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    if (TOKEN_NUM < 7)
        display_no = 1;
    else
        display_no = *number_ptr;

    RT_PARAM_CHK((display_no > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("\n");

        diag_util_mprintf("Port : %d\n", port);
        
        diag_util_mprintf("    MMD Addr %d : \n", *mmd_addr_ptr);
        index = 0;
        for(reg_indx = *mmd_reg_ptr; reg_indx < ((((*mmd_reg_ptr) + (display_no)) < (1 << 16)) ? ((*mmd_reg_ptr) + (display_no)) : (1 << 16)); reg_indx++)
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyMmdReg_get(unit, port, *mmd_addr_ptr, reg_indx, &reg_data), ret);
            if(0 == (index%4))
            {
                diag_util_printf("        0x%02X  ", reg_indx);
            }
            diag_util_printf("        0x%04X  ", reg_data);         
            if(3 == (index%4))
            {
                diag_util_mprintf("\n");
            }

            index++;
        }
        
        diag_util_mprintf("\n");
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_port_get_phy_mmd_reg_port_all_mmd_addr_mmd_reg_number */
#endif

#ifdef CMD_PORT_GET_BACK_PRESSURE_PORT_ALL_STATE
/* 
 * port get back-pressure ( <PORT_LIST:port> | all ) state 
 */ 
cparser_result_t cparser_cmd_port_get_back_pressure_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                   unit = 0;
    rtk_port_t               port = 0;
    int32                    ret = RT_ERR_FAILED;
    rtk_enable_t             enabled = DISABLED;
    diag_portlist_t          portlist;


    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
       diag_util_mprintf("Port %2d :\n", port);
            
       DIAG_UTIL_ERR_CHK(rtk_port_backpressureEnable_get(unit, port, &enabled), ret);
       diag_util_mprintf("\tBack Pressure : %s\n", enabled ? "ENABLE" : "DISABLE");
       
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;

}
#endif

#ifdef CMD_PORT_GET_AUTO_NEGO_PORT_ALL_STATE
/* 
 * port get auto-nego ( <PORT_LIST:port> | all ) state 
 */
cparser_result_t cparser_cmd_port_get_auto_nego_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_phy_ability_t  ability;
    diag_portlist_t               portlist;
    rtk_enable_t              autoNego;

    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&ability, 0, sizeof(rtk_port_phy_ability_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
       diag_util_mprintf("Port %2d :\n", port);
            
       DIAG_UTIL_ERR_CHK(rtk_port_phyAutoNegoEnable_get(unit, port, &autoNego), ret);
       diag_util_mprintf("\tAutoNego              : %s\n", autoNego ? "ENABLE" : "DISABLE");
         

       
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_GREEN_PORT_ALL_STATE
/* 
 * port get green ( <PORT_LIST:port> | all ) state 
 */ 
cparser_result_t cparser_cmd_port_get_green_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                   unit = 0;
    rtk_port_t               port = 0;
    int32                    ret = RT_ERR_FAILED;
    rtk_enable_t             enabled = DISABLED;
    diag_portlist_t          portlist;


    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
       diag_util_mprintf("Port %2d :\n", port);
            
       ret = rtk_port_greenEnable_get(unit, port, &enabled);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("\tGreen : %s\n", "No supported");
        else
            diag_util_mprintf("\tGreen : %s\n", enabled ? "ENABLE" : "DISABLE");
       
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_LINK_DOWN_POWER_SAVING_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port get link-down-power-saving ( <PORT_LIST:port> | all ) state 
 */
cparser_result_t cparser_cmd_port_get_link_down_power_saving_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    rtk_port_t      port = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enabled = DISABLED;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
       diag_util_mprintf("Port %2d :\n", port);
            
       ret = rtk_port_linkDownPowerSavingEnable_get(unit, port, &enabled);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("\tLink-Down Power-Saving : %s\n", "No supported");
        else
            diag_util_mprintf("\tLink-Down Power-Saving : %s\n", enabled ? "ENABLE" : "DISABLE");
       
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_AUTO_NEGO_PORT_ALL_ABILITY
/* 
 * port get auto-nego ( <PORT_LIST:port> | all ) ability 
 */
cparser_result_t cparser_cmd_port_get_auto_nego_port_all_ability(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_phy_ability_t  ability;
    diag_portlist_t               portlist;
    

    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    memset(&ability, 0, sizeof(rtk_port_phy_ability_t));

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
        diag_util_mprintf("Port %2d :\n", port);
            
        diag_util_printf("\tAutoNego Ability      : ");
        DIAG_UTIL_ERR_CHK(rtk_port_phyAutoNegoAbility_get(unit, port, &ability), ret);
        if (ABILITY_BIT_ON == ability.Half_10)
           diag_util_printf("10H ");
        if (ABILITY_BIT_ON == ability.Full_10)
           diag_util_printf("10F ");
        if (ABILITY_BIT_ON == ability.Half_100)
           diag_util_printf("100H ");
        if (ABILITY_BIT_ON == ability.Full_100)
           diag_util_printf("100F ");
        if (ABILITY_BIT_ON == ability.Full_1000)
           diag_util_printf("1000F ");
        if (ABILITY_BIT_ON == ability.FC)
           diag_util_printf("Flow-Control ");
        if (ABILITY_BIT_ON == ability.AsyFC)
           diag_util_printf("Asy-Flow-Control ");
        diag_util_mprintf("\n");
       
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;

}
#endif

#ifdef CMD_PORT_GET_ISOLATION_SRC_PORT
/* 
 *  port get isolation <PORT_LIST:src_port> 
 */       
cparser_result_t cparser_cmd_port_get_isolation_src_port(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    rtk_portmask_t      portmask;
    uint8               port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    diag_portlist_t  portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();
    
    /* show specific port isolation info */
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);  

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&portmask, 0, sizeof(rtk_portmask_t));
        DIAG_UTIL_ERR_CHK(rtk_port_isolation_get(unit, port, &portmask), ret);
        memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN * sizeof(uint8));
        diag_util_lPortMask2str(port_list, &portmask);
        diag_util_mprintf("\tPort %2d : Isolation Port list %s\n", port, port_list);
    }
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_ISOLATION_VLAN_BASED_VID
/*
 * port get isolation vlan-based <UINT:vid> 
 */
cparser_result_t cparser_cmd_port_get_isolation_vlan_based_vid(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          index, maxIndex;
    uint32          total_entry = 0;   
    rtk_port_vlanIsolationEntry_t entry;
    uint8               port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_OM_GET_CHIP_CAPACITY(unit, maxIndex, max_num_of_vlan_port_iso_entry);


    diag_util_mprintf("Index |VID  |Trust Port list       |State\n");
    diag_util_mprintf("------+-----+----------------------+---------\n");

    for (index = 0; index < maxIndex; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_get(unit, index, &entry), ret);
            
        if (entry.vid == *vid_ptr)
        {
            memset(port_list, 0, DIAG_UTIL_PORT_MASK_STRING_LEN * sizeof(uint8));
            diag_util_lPortMask2str(port_list, &entry.portmask);

            diag_util_mprintf("%4d  |%4d | %20s | %s   \n", index, entry.vid, 
                    port_list, (entry.enable == ENABLED ? "Enable" : "Disable"));
            total_entry++;
        }
    }    

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_ISOLATION_VLAN_BASED_VLAN_SOURCE
/*
 * port get isolation vlan-based vlan-source 
 */
cparser_result_t cparser_cmd_port_get_isolation_vlan_based_vlan_source(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_vlanIsolationSrc_t vlanSrc;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolation_vlanSource_get(unit, &vlanSrc), ret);

    diag_util_printf("VLAN-based Port Isolation\n");
    if (VLAN_ISOLATION_SRC_INNER == vlanSrc)
    {
        diag_util_printf("VLAN ID Source : %s\n", "Inner-tag VID");
    }
    else if (VLAN_ISOLATION_SRC_OUTER == vlanSrc)
    {
        diag_util_printf("VLAN ID Source : %s\n", "Outer-tag VID");
    }
    else
    {
        diag_util_printf("VLAN ID Source : %s\n", "Forwarding VID");
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_FORCE_PORT_ALL
/* 
 *  port get force ( <PORT_LIST:port> | all )
 */     
cparser_result_t cparser_cmd_port_get_force_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enabled = DISABLED;
    rtk_port_speed_t        speed = PORT_SPEED_10M;
    rtk_port_duplex_t       duplex = PORT_HALF_DUPLEX;
    diag_portlist_t               portlist;


    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
       diag_util_printf("Port %2d :\n", port);
            
       diag_util_printf("\tForce Mode Ability    : ");
       DIAG_UTIL_ERR_CHK(rtk_port_phyForceModeAbility_get(unit, port, &speed, &duplex, &enabled), ret);
       if (PORT_HALF_DUPLEX == duplex)
       {
           if (PORT_SPEED_10M == speed)
               diag_util_printf("10H ");
           else if (PORT_SPEED_100M == speed)
               diag_util_printf("100H ");
           else
           {
               diag_util_printf("User config: Error!\n");
               return CPARSER_NOT_OK;
           }    
       }
       else if (PORT_FULL_DUPLEX == duplex)
       {
           if (PORT_SPEED_10M == speed)
               diag_util_printf("10F ");
           else if (PORT_SPEED_100M == speed)
               diag_util_printf("100F ");
           else if (PORT_SPEED_1000M == speed)
               diag_util_printf("1000F ");
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
       diag_util_mprintf("\n");
       diag_util_mprintf("\tFlow Control (config) : %s\n", enabled ? "ENABLE" : "DISABLE");
       
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;

}
#endif

#ifdef CMD_PORT_GET_PORT_ALL_STATE
/* 
 *  port get ( <PORT_LIST:port> | all ) state
 */ 
cparser_result_t cparser_cmd_port_get_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_enable_t            enabled = DISABLED;
    diag_portlist_t           portlist;


    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();



    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
       diag_util_mprintf("Port %2d :\n", port);
       
       
       DIAG_UTIL_ERR_CHK(rtk_port_adminEnable_get(unit, port, &enabled), ret);
       diag_util_mprintf("\tAdmin : %s\n", enabled ? "ENABLE" : "DISABLE");
   
       
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;

}
#endif

#ifdef CMD_PORT_GET_COMBO_MODE_PORT_ALL
/* 
 * port get combo-mode ( <PORT_LIST:port> | all )
 */ 
cparser_result_t cparser_cmd_port_get_combo_mode_port_all(cparser_context_t *context)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_media_t        media;  
    diag_portlist_t               portlist;


    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
        diag_util_mprintf("Port %2d :\n", port);
        diag_util_printf("\tMedia                 : ");
        DIAG_UTIL_ERR_CHK(rtk_port_phyComboPortMedia_get(unit, port, &media), ret);
        if (PORT_MEDIA_COPPER == media)
        {
            diag_util_mprintf("Copper\n");
        }    
        else if (PORT_MEDIA_FIBER == media)
        {
            diag_util_mprintf("Fiber\n");
        }
        else if (PORT_MEDIA_COPPER_AUTO == media)
        {
            diag_util_mprintf("Auto-Copper\n");
        }
        else if (PORT_MEDIA_FIBER_AUTO == media)
        {
            diag_util_mprintf("Auto-Fiber\n");
        }
        else if (PORT_MEDIA_FIBER_AUTO_BY_GPIO == media)
        {
            diag_util_printf("Auto-Fiber-By-GPIO\n");
        }

    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_GET_CROSS_OVER_PORT_ALL_MODE
/*
 * port get cross-over ( <PORT_LIST:port> | all ) mode
 */
cparser_result_t cparser_cmd_port_get_cross_over_port_all_mode(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_crossOver_mode_t   mode;
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
        diag_util_mprintf("Port %2d :\n", port);
        diag_util_printf("\tCross Over Mode: ");
        ret = rtk_port_phyCrossOverMode_get(unit, port, &mode);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("%s\n", "No supported");
        else
        {
            if (PORT_CROSSOVER_MODE_AUTO == mode)
            {
                diag_util_mprintf("Auto MDI/MDIX\n");
            }    
            else if (PORT_CROSSOVER_MODE_MDI == mode)
            {
                diag_util_mprintf("Force MDI\n");
            }
            else
            {
                diag_util_mprintf("Force MDIX\n");
            }
        }
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;
} /* end of cparser_cmd_port_get_cross_over_port_all_mode */
#endif

#ifdef CMD_PORT_SET_AUTO_NEGO_PORT_ALL_ABILITY_10H_10F_100H_100F_1000F_FLOW_CONTROL_ASY_FLOW_CONTROL
/* 
 * port set auto-nego ( <PORT_LIST:port> | all ) ability { 10h } { 10f } { 100h } { 100f } { 1000f } { flow-control } { asy-flow-control }
 */ 
cparser_result_t cparser_cmd_port_set_auto_nego_port_all_ability_10h_10f_100h_100f_1000f_flow_control_asy_flow_control(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    uint32                  option_num = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    rtk_port_phy_ability_t  ability;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();
    
    memset(&ability, 0, sizeof(rtk_port_phy_ability_t));
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    for (option_num = 5; option_num < TOKEN_NUM; option_num++)
    {
        if ('f' == TOKEN_CHAR(option_num, 0)) 
        {
            ability.FC = ABILITY_BIT_ON; 
        }
        else if ('a' == TOKEN_CHAR(option_num, 0)) 
        {
            ability.AsyFC = ABILITY_BIT_ON; 
        }        
        else if ('h' == TOKEN_CHAR(option_num, 2))
        {
            ability.Half_10 = ABILITY_BIT_ON; 
        }
        else if ('f' == TOKEN_CHAR(option_num, 2)) 
        {
            ability.Full_10 = ABILITY_BIT_ON; 
        }
        else if ('h' == TOKEN_CHAR(option_num, 3)) 
        {
            ability.Half_100 = ABILITY_BIT_ON; 
        }
        else if ('f' == TOKEN_CHAR(option_num, 3)) 
        {
            ability.Full_100 = ABILITY_BIT_ON; 
        }
        else if ('f' == TOKEN_CHAR(option_num, 4)) 
        {
            ability.Full_1000 = ABILITY_BIT_ON; 
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_phyAutoNegoAbility_set(unit, port, &ability), ret);
    } 
     
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_AUTO_NEGO_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set auto-nego ( <PORT_LIST:port> | all ) state ( disable | enable )
 */     
cparser_result_t cparser_cmd_port_set_auto_nego_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    /* set port auto nego */
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        if ('e' == TOKEN_CHAR(5, 0))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyAutoNegoEnable_set(unit, port, ENABLED), ret);
        }
        else
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyAutoNegoEnable_set(unit, port, DISABLED), ret);
        }   
    }         
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_BACK_PRESSURE_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set back-pressure ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_back_pressure_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        if ('e' == TOKEN_CHAR(5, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_backpressureEnable_set(unit, port, ENABLED), ret);
        }
        else if ('d' == TOKEN_CHAR(5, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_backpressureEnable_set(unit, port, DISABLED), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }   
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_COMBO_MODE_PORT_ALL_COPPER_FORCE_FIBER_FORCE_AUTO_PREFERRED_COPPER_AUTO_PREFERRED_FIBER_AUTO_PREFERRED_FIBER_BY_GPIO
/* 
 * port set combo-mode ( <PORT_LIST:port> | all ) ( copper-force | fiber-force | auto-preferred-copper | auto-preferred-fiber | auto-preferred-fiber-by-gpio )
 */ 
cparser_result_t cparser_cmd_port_set_combo_mode_port_all_copper_force_fiber_force_auto_preferred_copper_auto_preferred_fiber_auto_preferred_fiber_by_gpio(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    rtk_port_media_t media = PORT_MEDIA_COPPER;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('c' == TOKEN_CHAR(4, 0))
    {
        media = PORT_MEDIA_COPPER;
    }
    else if ('f' == TOKEN_CHAR(4, 0))
    {
        media = PORT_MEDIA_FIBER;
    }
    else if ('a' == TOKEN_CHAR(4, 0))
    {
        if ('c' == TOKEN_CHAR(4, 15))
            media = PORT_MEDIA_COPPER_AUTO;
        else if ('f' == TOKEN_CHAR(4, 15))
        {
            if ('g' == TOKEN_CHAR(4, 24))
                media = PORT_MEDIA_FIBER_AUTO_BY_GPIO;
            else
                media = PORT_MEDIA_FIBER_AUTO;
        }
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /* set port media type */
        DIAG_UTIL_ERR_CHK(rtk_port_phyComboPortMedia_set(unit, port, media), ret);
    }     

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_FORCE_PORT_ALL_ABILITY_10H_10F_100H_100F_1000F_FLOW_CONTROL_STATE_DISABLE_ENABLE
/* 
 * port set force ( <PORT_LIST:port> | all ) ability ( 10h | 10f | 100h | 100f | 1000f ) flow-control state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_force_port_all_ability_10h_10f_100h_100f_1000f_flow_control_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_speed_t    speed = PORT_SPEED_10M;
    rtk_port_duplex_t   duplex = PORT_HALF_DUPLEX;
    rtk_enable_t        flowControl = DISABLED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
        
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
    
    if ('h' == TOKEN_CHAR(5, 2))
    {
        speed = PORT_SPEED_10M;
        duplex = PORT_HALF_DUPLEX;
    }
    else if ('f' == TOKEN_CHAR(5, 2))
    {
        speed = PORT_SPEED_10M;
        duplex = PORT_FULL_DUPLEX;
    }
    else if ('h' == TOKEN_CHAR(5, 3))
    {
        speed = PORT_SPEED_100M;
        duplex = PORT_HALF_DUPLEX;
    }
    else if ('f' == TOKEN_CHAR(5, 3))
    {
        speed = PORT_SPEED_100M;
        duplex = PORT_FULL_DUPLEX;
    }
    else if ('f' == TOKEN_CHAR(5, 4))
    {
        speed = PORT_SPEED_1000M;
        duplex = PORT_FULL_DUPLEX;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }    
    
    if ('e' == TOKEN_CHAR(8, 0))
    {
        flowControl = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(8, 0))
    {
        flowControl = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;        
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_phyForceModeAbility_set(unit, port, speed, duplex, flowControl), ret);
    }
       
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_GREEN_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set green ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_green_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        if ('e' == TOKEN_CHAR(5, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_greenEnable_set(unit, port, ENABLED), ret);
        }
        else if ('d' == TOKEN_CHAR(5, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_greenEnable_set(unit, port, DISABLED), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }   
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_LINK_DOWN_POWER_SAVING_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set link-down-power-saving ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_link_down_power_saving_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        if ('e' == TOKEN_CHAR(5, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_linkDownPowerSavingEnable_set(unit, port, ENABLED), ret);
        }
        else if ('d' == TOKEN_CHAR(5, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_linkDownPowerSavingEnable_set(unit, port, DISABLED), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }   
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_ISOLATION_SRC_PORT_PORT_ALL
/* 
 * port set isolation <PORT_LIST:src_port> ( <PORT_LIST:port> | all )
 */ 
cparser_result_t cparser_cmd_port_set_isolation_src_port_port_all(cparser_context_t *context,
    char **src_port_ptr,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;
    diag_portlist_t    targetPortlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*src_port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(targetPortlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_isolation_set(unit, port, targetPortlist.portmask), ret);
    }  

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_ISOLATION_VLAN_BASED_VID_TRUST_PORT_NONE
/* 
 * port set isolation vlan-based <UINT:vid> ( <PORT_LIST:trust_port> | none )
 */   
cparser_result_t cparser_cmd_port_set_isolation_vlan_based_vid_trust_port_none(cparser_context_t *context,
    uint32_t *vid_ptr,
    char **trust_port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          index, maxIndex, doDel = FALSE, found = FALSE;
    rtk_port_vlanIsolationEntry_t entry, setEntry;
    rtk_port_t      port = 0;
    diag_portlist_t    portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    DIAG_OM_GET_CHIP_CAPACITY(unit, maxIndex, max_num_of_vlan_port_iso_entry);

    memset(&entry, 0, sizeof(entry));    
    memset(&setEntry, 0, sizeof(setEntry));  
    
    setEntry.portmask.bits[0] = 0;
    setEntry.portmask.bits[1] = 0;
    if ('n' == TOKEN_CHAR(5, 0))
    {
        setEntry.vid = *vid_ptr;
      #ifdef CONFIG_SDK_RTL8390
        setEntry.vid_high = *vid_ptr;
      #endif
        doDel = TRUE;
    }
    else
    {
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            if (port < 32)
                setEntry.portmask.bits[0] |= (0x1 << port);
            else
                setEntry.portmask.bits[1] |= (0x1 << (port-32));
        }

        setEntry.portmask.bits[0] = setEntry.portmask.bits[0];
        setEntry.portmask.bits[1] = setEntry.portmask.bits[1];
        setEntry.enable = ENABLED;
        setEntry.vid = *vid_ptr;
      #ifdef CONFIG_SDK_RTL8390
        setEntry.vid_high = *vid_ptr;
      #endif
    }

    /* search exist entry */
    for (index = 0; index < maxIndex; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_get(unit, index, &entry), ret);
            
        if (entry.vid == *vid_ptr)
        {
            if(TRUE == doDel)
            {
                setEntry.vid = 0; /*set vid = 0 if action is to delete the entry*/
              #ifdef CONFIG_SDK_RTL8390
                setEntry.vid_high = 0;
              #endif
            }
            DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_set(unit, index, &setEntry), ret);
            found = TRUE;
            break;
        }
    }
    if(FALSE == found && TRUE == doDel)
        return RT_ERR_PORT_VLAN_ISO_VID_NOT_FOUND;
    
    /* search empty entry */
    if (FALSE == found && FALSE == doDel)
    {
        for (index = 0; index < maxIndex; index++)
        {
            DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_get(unit, index, &entry), ret);
                
            if (entry.vid == 0)
            {
                DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_set(unit, index, &setEntry), ret);
                found = TRUE;
                break;
            }
        }    
        if(FALSE == found)
            return RT_ERR_PORT_VLAN_ISO_NO_EMPTY_ENTRY;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_ISOLATION_VLAN_BASED_VID_STATE_DISABLE_ENABLE
/* 
 * port set isolation vlan-based <UINT:vid> state ( disable | enable )
 */   
cparser_result_t cparser_cmd_port_set_isolation_vlan_based_vid_state_disable_enable(cparser_context_t *context,
    uint32_t *vid_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          index, maxIndex;
    rtk_port_vlanIsolationEntry_t entry;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 
    
    DIAG_OM_GET_CHIP_CAPACITY(unit, maxIndex, max_num_of_vlan_port_iso_entry);

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

    for (index = 0; index < maxIndex; index++)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_get(unit, index, &entry), ret);
            
        if (entry.vid == *vid_ptr)
        {
            entry.enable = enable;
            DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolationEntry_set(unit, index, &entry), ret);
        }
    }  

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_ISOLATION_VLAN_BASED_VLAN_SOURCE_INNER_OUTER_FORWARDING
/* 
 * port set isolation vlan-based vlan-source ( inner | outer | forwarding )
 */   
cparser_result_t cparser_cmd_port_set_isolation_vlan_based_vlan_source_inner_outer_forwarding(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_vlanIsolationSrc_t vlanSrc;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    if ('i' == TOKEN_CHAR(5, 0))
    {
        vlanSrc = VLAN_ISOLATION_SRC_INNER;
    }
    else if ('o' == TOKEN_CHAR(5, 0))
    {
        vlanSrc = VLAN_ISOLATION_SRC_OUTER;
    }
    else if ('f' == TOKEN_CHAR(5, 0))
    {
        vlanSrc = VLAN_ISOLATION_SRC_FORWARD;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;        
    }

    DIAG_UTIL_ERR_CHK(rtk_port_vlanBasedIsolation_vlanSource_set(unit, vlanSrc), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_PHY_BCAST_REG_BROADCAST_ID
/*
 * port set phy-bcast-reg <UINT:broadcast_id>
 */
cparser_result_t
cparser_cmd_port_set_phy_bcast_reg_broadcast_id(
    cparser_context_t *context,
    uint32_t *broadcast_id_ptr)
{
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_port_phyReg_broadcastID_set(unit, *broadcast_id_ptr), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_port_set_phy_bcast_reg_broadcast_id */
#endif

#ifdef CMD_PORT_SET_PHY_REG_BROADCAST_PAGE_REGISTER_DATA
/*
 * port set phy-reg broadcast <UINT:page> <UINT:register> <UINT:data>
 */
cparser_result_t
cparser_cmd_port_set_phy_reg_broadcast_page_register_data(
    cparser_context_t *context,
    uint32_t *page_ptr,
    uint32_t *register_ptr,
    uint32_t *data_ptr)
{
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_port_phyReg_broadcast_set(unit, *page_ptr, *register_ptr, *data_ptr), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_port_set_phy_reg_broadcast_page_register_data */
#endif

#ifdef CMD_PORT_SET_PHY_REG_PORT_ALL_PAGE_REGISTER_DATA
/* 
 * port set phy-reg ( <PORT_LIST:port> | all ) <UINT:page> <UINT:register> <UINT:data>
 */   
cparser_result_t cparser_cmd_port_set_phy_reg_port_all_page_register_data(cparser_context_t *context,
    char **port_ptr,
    uint32_t *page_ptr,
    uint32_t *register_ptr,
    uint32_t *data_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((page_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    //DIAG_UTIL_PARAM_RANGE_CHK((*page_ptr > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((register_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*register_ptr > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((data_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
        
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        /* set port phy register */
        DIAG_UTIL_ERR_CHK(rtk_port_phyReg_set(unit, port, *page_ptr, *register_ptr, *data_ptr), ret); 
    }  

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_PHY_EXT_REG_PORT_ALL_MAINPAGE_EXTPAGE_PARKPAGE_REGISTER_DATA
/*
 * port set phy-ext-reg ( <PORT_LIST:port> | all ) <UINT:mainPage> <UINT:extPage> <UINT:parkPage> <UINT:register> <UINT:data>
 */
cparser_result_t cparser_cmd_port_set_phy_ext_reg_port_all_mainPage_extPage_parkPage_register_data(cparser_context_t *context,
    uint32_t *mainPage_ptr,
    uint32_t *extPage_ptr,
    uint32_t *parkPage_ptr,
    uint32_t *register_ptr,
    uint32_t *data_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
  
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((mainPage_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((extPage_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((parkPage_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((register_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((*register_ptr > MAX_PHY_REGISTER), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((data_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
        
    /* set port phy register */
    DIAG_UTIL_ERR_CHK(rtk_port_phymaskExtParkPageReg_set(unit, &portlist.portmask, 
        *mainPage_ptr, *extPage_ptr, *parkPage_ptr, *register_ptr, *data_ptr), ret); 

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_phy_ext_reg_port_all_mainpage_extpage_parkpage_register_data */
#endif

#ifdef CMD_PORT_SET_PHY_MMD_REG_PORT_ALL_MMD_ADDR_MMD_REG_DATA
/*
 * port set phy-mmd-reg ( <PORT_LIST:port> | all ) <UINT:mmd-addr> <UINT:mmd-reg> <UINT:data>
 */
cparser_result_t
cparser_cmd_port_set_phy_mmd_reg_port_all_mmd_addr_mmd_reg_data(
    cparser_context_t *context,
    uint32_t  *mmd_addr_ptr,
    uint32_t  *mmd_reg_ptr,
    uint32_t  *data_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
  
    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((mmd_addr_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((mmd_reg_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((data_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);
        
    /* set port phy register */
    DIAG_UTIL_ERR_CHK(rtk_port_phymaskMmdReg_set(unit, &portlist.portmask, 
        *mmd_addr_ptr, *mmd_reg_ptr, *data_ptr), ret); 

    return CPARSER_OK;
}    /* end of cparser_cmd_port_set_phy_mmd_reg_port_all_mmd_addr_mmd_reg_data */
#endif

#ifdef CMD_PORT_SET_RX_TX_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set ( rx | tx ) ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_rx_tx_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_enable_t enable;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    
    DIAG_UTIL_PARAM_CHK();
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('e' == TOKEN_CHAR(5, 0)) 
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        if ('r' == TOKEN_CHAR(2, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_rxEnable_set(unit, port, enable), ret);
        }
        else if ('t' == TOKEN_CHAR(2, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_txEnable_set(unit, port, enable), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }   

    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t         port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    
    DIAG_UTIL_PARAM_CHK();
        
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 2), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        if ('e' == TOKEN_CHAR(4, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_adminEnable_set(unit, port, ENABLED), ret);
        }
        else if ('d' == TOKEN_CHAR(4, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_port_adminEnable_set(unit, port, DISABLED), ret);
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }   
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_PORT_SET_RLDP_AUTO_BLOCK_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set rldp auto-block ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_rldp_auto_block_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_enable_t             enable;
    rtk_port_t                  port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(6, 0))
    {
        enable = ENABLED;          
    }
    else if('d' == TOKEN_CHAR(6, 0))
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
        DIAG_UTIL_ERR_CHK(rtk_port_rldpAutoBlockEnable_set(unit, port, enable), ret);
    }  

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_RLDP_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set rldp ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_rldp_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_enable_t             enable;
    rtk_port_t                 port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
 
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e' == TOKEN_CHAR(5, 0))
    {
        enable = ENABLED;          
    }
    else if('d' == TOKEN_CHAR(5, 0))
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
        DIAG_UTIL_ERR_CHK(rtk_port_rldpEnable_set(unit, port, enable), ret);
    }   

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_RLDP_HELLO_TIME_NORMAL_LOOP_AGING_TIME_SELF_LOOP_AGING_TIME_PORT_ALL_1S_2S_4S_8S
/* 
 * port set rldp ( hello-time | normal-loop-aging-time | self-loop-aging-time ) ( <PORT_LIST:port> | all ) ( 1s | 2s | 4s | 8s )
 */ 
cparser_result_t cparser_cmd_port_set_rldp_hello_time_normal_loop_aging_time_self_loop_aging_time_port_all_1s_2s_4s_8s(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                  port = 0;
    uint32                      interval;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('1' == TOKEN_CHAR(5, 0))
    {
        interval = 1;          
    }
    else if('2' == TOKEN_CHAR(5, 0))
    {
        interval = 2;          
    }
    else if('4' == TOKEN_CHAR(5, 0))
    {
        interval = 4;          
    }
    else if('8' == TOKEN_CHAR(5, 0))
    {
        interval = 8;          
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        if('h' == TOKEN_CHAR(3, 0))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_rldpInterval_set(unit, port, interval), ret);   
        }
        else if('s' == TOKEN_CHAR(3, 0))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_rldpSelfLoopAgingTime_set(unit, port, interval), ret);     
        }
        else if('n' == TOKEN_CHAR(3, 0))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_rldpNormalLoopAgingTime_set(unit, port, interval), ret);            
        }
        else
        {
            diag_util_printf("User config: Error!\n");
            return CPARSER_NOT_OK;
        }
    }    

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_UDLD_AUTO_DISABLE_PORT_LED_INDICATION_LINKUP_AUTO_TRIG_STATE_DISABLE_ENABLE
/* 
 * port set udld ( auto-disable-port | led-indication | linkup-auto-trig ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_udld_auto_disable_port_led_indication_linkup_auto_trig_state_disable_enable(cparser_context_t *context)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_enable_t            enable;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(5, 0))
    {
        enable = ENABLED;
    }
    else if('d' == TOKEN_CHAR(5, 0))
    {
        enable = DISABLED;
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    if('l' == TOKEN_CHAR(3, 0))
    {
        if('e' == TOKEN_CHAR(3, 1))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_udldLedIndicateEnable_set(unit, enable), ret);
        }
        else if('i' == TOKEN_CHAR(3, 1))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_udldLinkUpAutoTriggerEnable_set(unit, enable), ret);
        }
    }
    else if ('a' == TOKEN_CHAR(3, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_port_udldAutoDisableFailedPortEnable_set(unit, enable), ret);
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }        

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_UDLD_ECHO_GENERATE_INTERVAL_2S_4S_8S_16S
/* 
 * port set udld echo-generate-interval ( 2s | 4s | 8s | 16s )
 */ 
cparser_result_t cparser_cmd_port_set_udld_echo_generate_interval_2s_4s_8s_16s(cparser_context_t *context)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_udldInterval_t interval;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    if('2' == TOKEN_CHAR(4, 0))
    {
        interval = UDLD_INTERVAL_2S;          
    }
    else if('4' == TOKEN_CHAR(4, 0))
    {
        interval = UDLD_INTERVAL_4S;          
    }
    else if('8' == TOKEN_CHAR(4, 0))
    {
        interval = UDLD_INTERVAL_8S;          
    }
    else if('1' == TOKEN_CHAR(4, 0))
    {
        interval = UDLD_INTERVAL_16S;          
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_port_udldInterval_set(unit, interval), ret);

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_UDLD_ECHO_REQUEST_RETRY_TIMES_1_2_4_8
/* 
 * port set udld echo-request-retry-times ( 1 | 2 | 4 | 8 )
 */ 
cparser_result_t cparser_cmd_port_set_udld_echo_request_retry_times_1_2_4_8(cparser_context_t *context)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    uint32                     retryCount;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    if('1' == TOKEN_CHAR(4, 0))
    {
        retryCount = 1;          
    }
    else if('2' == TOKEN_CHAR(4, 0))
    {
        retryCount = 2;          
    }
    else if('4' == TOKEN_CHAR(4, 0))
    {
        retryCount = 4;          
    }
    else if('8' == TOKEN_CHAR(4, 0))
    {
        retryCount = 8;          
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_port_udldRetryCount_set(unit, retryCount), ret);

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_UDLD_ECHO_ACTION_FORWARD_SELFLOOP
/* 
 * port set udld echo-action ( forward | selfloop )
 */ 
cparser_result_t cparser_cmd_port_set_udld_echo_action_forward_selfloop(cparser_context_t *context)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit); 

    if('s' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_port_udldEchoAction_set(unit, UDLD_ACTION_SELFLOOPBACK), ret);
    }
    else if('f' == TOKEN_CHAR(4, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_port_udldEchoAction_set(unit, UDLD_ACTION_FORWARD), ret);
    }

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_UDLD_LINK_STATUS_PORT_ALL_DISABLE_NORMAL
/* 
 * port set udld link-status ( <PORT_LIST:port> | all ) ( disable | normal )
 */ 
cparser_result_t cparser_cmd_port_set_udld_link_status_port_all_disable_normal(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {        
        if('n' == TOKEN_CHAR(5, 0))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_udldLinkStatus_set(unit, port, UDLD_LINK_STATUS_NORMAL), ret);
        }
        else if('d' == TOKEN_CHAR(5, 0))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_udldLinkStatus_set(unit, port, UDLD_LINK_STATUS_DISABLE), ret);
        }
    }   

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_UDLD_PORT_ALL_STATE_DISABLE_ENABLE
/* 
 * port set udld ( <PORT_LIST:port> | all ) state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_port_set_udld_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_enable_t             enable;
    rtk_port_t                 port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('e' == TOKEN_CHAR(5, 0))
    {
        enable = ENABLED;          
    }
    else if('d' == TOKEN_CHAR(5, 0))
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
        DIAG_UTIL_ERR_CHK(rtk_port_udldEnable_set(unit, port, enable), ret);
    }   

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_UDLD_PORT_ALL_TRIGGER
/* 
 * port set udld ( <PORT_LIST:port> | all ) trigger 
 */ 
cparser_result_t cparser_cmd_port_set_udld_port_all_trigger(cparser_context_t *context,
    char **port_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                port = 0;
    diag_portlist_t               portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
       DIAG_UTIL_ERR_CHK(rtk_port_udldTrigger_start(unit, port), ret);
    }  

    return CPARSER_NOT_OK;
}
#endif

#ifdef CMD_PORT_SET_CROSS_OVER_PORT_ALL_MODE_AUTO_MDI_MDIX
/*
 * port set cross-over ( <PORT_LIST:port> | all ) mode ( auto | mdi | mdix )
 */
cparser_result_t cparser_cmd_port_set_cross_over_port_all_mode_auto_mdi_mdix(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if('a' == TOKEN_CHAR(5, 0))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyCrossOverMode_set(unit, port, PORT_CROSSOVER_MODE_AUTO), ret);
        }
        else if ('x' == TOKEN_CHAR(5, 3))
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyCrossOverMode_set(unit, port, PORT_CROSSOVER_MODE_MDIX), ret);
        }
        else
        {
            DIAG_UTIL_ERR_CHK(rtk_port_phyCrossOverMode_set(unit, port, PORT_CROSSOVER_MODE_MDI), ret);
        }
    }

    return CPARSER_OK;
} /* end of cparser_cmd_port_set_cross_over_port_all_mode_auto_mdi_mdix */
#endif

#ifdef CMD_PORT_GET_COMBO_FIBER_MODE_PORT_ALL
/*
 * port get combo-fiber-mode ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_port_get_combo_fiber_mode_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_fiber_media_t  media;  
    diag_portlist_t         portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
        diag_util_mprintf("Port %2d :\n", port);
        diag_util_printf("\tFiber Media           : ");
        DIAG_UTIL_ERR_CHK(rtk_port_phyComboPortFiberMedia_get(unit, port, &media), ret);
        if (PORT_FIBER_MEDIA_1000 == media)
        {
            diag_util_mprintf("Fiber-1000Base-X\n");
        }    
        else if (PORT_FIBER_MEDIA_100 == media)
        {
            diag_util_mprintf("Fiber-100Base-FX\n");
        }
        else if (PORT_FIBER_MEDIA_AUTO == media)
        {
            diag_util_mprintf("Fiber-Auto\n");
        }
    }  /*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;
} /* end of cparser_cmd_port_get_combo_fiber_mode_port_all */
#endif

#ifdef CMD_PORT_SET_COMBO_FIBER_MODE_PORT_ALL_FIBER_1000_FIBER_100_FIBER_AUTO
/*
 * port set combo-fiber-mode ( <PORT_LIST:port> | all ) ( fiber-1000 | fiber-100 | fiber-auto )
 */
cparser_result_t cparser_cmd_port_set_combo_fiber_mode_port_all_fiber_1000_fiber_100_fiber_auto(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    rtk_port_fiber_media_t media = PORT_FIBER_MEDIA_AUTO;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('a' == TOKEN_CHAR(4, 6))
    {
        media = PORT_FIBER_MEDIA_AUTO;
    }
    else if ('0' == TOKEN_CHAR(4, 9))
    {
        media = PORT_FIBER_MEDIA_1000;
    }
    else
    {
        media = PORT_FIBER_MEDIA_100;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /* set port fiber media type */
        DIAG_UTIL_ERR_CHK(rtk_port_phyComboPortFiberMedia_set(unit, port, media), ret);
    }     

    return CPARSER_OK;
} /* end of cparser_cmd_port_set_combo_fiber_mode_port_all_fiber_1000_fiber_100_fiber_auto */
#endif

#ifdef CMD_PORT_GET_MASTER_SLAVE_PORT_ALL
cparser_result_t cparser_cmd_port_get_master_slave_port_all(cparser_context_t *context,    char **port_ptr)
{
    uint32                  unit = 0;
    rtk_port_t              port = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_port_masterSlave_t  masterSlaveCfg = PORT_AUTO_MODE, masterSlaveActual;
    diag_portlist_t           portlist;


    DIAG_OM_GET_CHIP_ID(unit);  
       
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)        
    {
        diag_util_mprintf("Port %2d :\n", port);
       
       
        DIAG_UTIL_ERR_CHK(rtk_port_phyMasterSlave_get(unit, port, &masterSlaveCfg, &masterSlaveActual), ret);
        diag_util_printf("\tMaster/Slave(Config) : ");
   
        switch(masterSlaveCfg)
        {
            case PORT_AUTO_MODE:
                diag_util_mprintf("Auto\n");
                break;
            case PORT_SLAVE_MODE:
                diag_util_mprintf("Slave\n");
                break;    
            case PORT_MASTER_MODE:
                diag_util_mprintf("Master\n");
                break;    
            default:
                break;
        }

        diag_util_printf("\tMaster/Slave(Actual) : ");
   
        switch(masterSlaveActual)
        {
            case PORT_AUTO_MODE:
                diag_util_mprintf("Auto\n");
                break;
            case PORT_SLAVE_MODE:
                diag_util_mprintf("Slave\n");
                break;    
            case PORT_MASTER_MODE:
                diag_util_mprintf("Master\n");
                break;    
            default:
                break;
        }
    }/*end of DIAG_UTIL_PORTMASK_SCAN(portlist, port)*/
       
    return CPARSER_OK;
}/* end of cparser_cmd_port_get_master_slave_port_all */
#endif

#ifdef CMD_PORT_SET_MASTER_SLAVE_PORT_ALL_AUTO_MASTER_SLAVE
cparser_result_t cparser_cmd_port_set_master_slave_port_all_auto_master_slave(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_port_masterSlave_t  masterSlave = PORT_AUTO_MODE;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if('a' == TOKEN_CHAR(4, 0))
    {
        masterSlave = PORT_AUTO_MODE;          
    }
    else if('m' == TOKEN_CHAR(4, 0))
    {
        masterSlave = PORT_MASTER_MODE;          
    }
    else if('s' == TOKEN_CHAR(4, 0))
    {
        masterSlave = PORT_SLAVE_MODE;          
    }
    else
    {
        diag_util_printf("User config: Error!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rtk_port_phyMasterSlave_set(unit, port, masterSlave), ret);
    }   

    return CPARSER_NOT_OK;
}/* end of cparser_cmd_port_set_master_slave_port_all_auto_master_slave */
#endif

