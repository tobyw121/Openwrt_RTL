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
 * $Revision: 25417 $
 * $Date: 2011-11-23 14:20:00 +0800 (Wed, 23 Nov 2011) $
 *
 * Purpose : Definition those Diagnostic command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Locol/Remote Loopback
 *           2) RTCT
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

#ifdef CMD_DIAG_GET_LOOPBACK_MAC_LOCAL_PORT_ALL_STATE
/*
 * diag get loopback mac local ( <PORT_LIST:port> | all ) state 
 */
cparser_result_t cparser_cmd_diag_get_loopback_mac_local_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_enable_t    enabled = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        diag_util_mprintf("Port %2d :\n", port);
        ret = rtk_diag_portMacLocalLoopbackEnable_get(unit, port, &enabled);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("\tMac Local Loopback : %s\n", "No supported");
        else
            diag_util_mprintf("\tMac Local Loopback : %s\n", enabled ? "ENABLE" : "DISABLE");
    }   
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_DIAG_GET_LOOPBACK_MAC_REMOTE_PORT_ALL_STATE
/*
 * diag get loopback mac remote ( <PORT_LIST:port> | all ) state 
 */
cparser_result_t cparser_cmd_diag_get_loopback_mac_remote_port_all_state(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;
    rtk_enable_t    enabled = DISABLED;

    DIAG_OM_GET_CHIP_ID(unit);  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        diag_util_mprintf("Port %2d :\n", port);
        ret = rtk_diag_portMacRemoteLoopbackEnable_get(unit, port, &enabled);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else if (ret == RT_ERR_CHIP_NOT_SUPPORTED)
            diag_util_mprintf("\tMac Remote Loopback : %s\n", "No supported");
        else
            diag_util_mprintf("\tMac Remote Loopback : %s\n", enabled ? "ENABLE" : "DISABLE");
    }   
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_DIAG_GET_RTCT_PORT_ALL
/*
 * diag get rtct ( <PORT_LIST:port> | all )
 */
cparser_result_t cparser_cmd_diag_get_rtct_port_all(cparser_context_t *context,
    char **port_ptr)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_port_t          port = 0;
    rtk_rtctResult_t    rtctResult;
    diag_portlist_t     portlist;
    rtk_switch_devInfo_t devInfo;

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    if ((ret = rtk_switch_deviceInfo_get(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&rtctResult, 0, sizeof(rtk_rtctResult_t));
        if ((ret = rtk_diag_portRtctResult_get(unit, port, &rtctResult)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        if ((1 << port) & devInfo.ge.portmask.bits[0])
        {
            diag_util_mprintf("Port %2d (type GE):\n", port);
            diag_util_printf("  channel A: \n");
            diag_util_printf("    Status : ");
            if (rtctResult.ge_result.channelAShort)
                diag_util_printf("[Short]");
            if (rtctResult.ge_result.channelAOpen)
                diag_util_printf("[Open]");
            if (rtctResult.ge_result.channelAMismatch)
                diag_util_printf("[Mismatch]");
            if (rtctResult.ge_result.channelALinedriver)
                diag_util_printf("[Linedriver]");
            if (!(rtctResult.ge_result.channelAShort | rtctResult.ge_result.channelAOpen |
                rtctResult.ge_result.channelAMismatch | rtctResult.ge_result.channelALinedriver))
                diag_util_printf("[Normal]");
            diag_util_printf("\n");

            if ((rtctResult.ge_result.channelAShort | rtctResult.ge_result.channelAOpen |
                rtctResult.ge_result.channelAMismatch | rtctResult.ge_result.channelALinedriver))
                diag_util_printf("    Cable Length : %d.%02d (m)\n", rtctResult.ge_result.channelALen/100, rtctResult.ge_result.channelALen%100);

            diag_util_printf("  channel B: \n");
            diag_util_printf("    Status : ");
            if (rtctResult.ge_result.channelBShort)
                diag_util_printf("[Short]");
            if (rtctResult.ge_result.channelBOpen)
                diag_util_printf("[Open]");
            if (rtctResult.ge_result.channelBMismatch)
                diag_util_printf("[Mismatch]");
            if (rtctResult.ge_result.channelBLinedriver)
                diag_util_printf("[Linedriver]");
            if (!(rtctResult.ge_result.channelBShort | rtctResult.ge_result.channelBOpen |
                rtctResult.ge_result.channelBMismatch | rtctResult.ge_result.channelBLinedriver))
                diag_util_printf("[Normal]");
            diag_util_printf("\n");

            if ((rtctResult.ge_result.channelBShort | rtctResult.ge_result.channelBOpen |
                rtctResult.ge_result.channelBMismatch | rtctResult.ge_result.channelBLinedriver))
                diag_util_printf("    Cable Length : %d.%02d (m)\n", rtctResult.ge_result.channelBLen/100, rtctResult.ge_result.channelBLen%100);

            diag_util_printf("  channel C: \n");
            diag_util_printf("    Status : ");
            if (rtctResult.ge_result.channelCShort)
                diag_util_printf("[Short]");
            if (rtctResult.ge_result.channelCOpen)
                diag_util_printf("[Open]");
            if (rtctResult.ge_result.channelCMismatch)
                diag_util_printf("[Mismatch]");
            if (rtctResult.ge_result.channelCLinedriver)
                diag_util_printf("[Linedriver]");
            if (!(rtctResult.ge_result.channelCShort | rtctResult.ge_result.channelCOpen |
                rtctResult.ge_result.channelCMismatch | rtctResult.ge_result.channelCLinedriver))
                diag_util_printf("[Normal]");
            diag_util_printf("\n");

            if ((rtctResult.ge_result.channelCShort | rtctResult.ge_result.channelCOpen |
                rtctResult.ge_result.channelCMismatch | rtctResult.ge_result.channelCLinedriver))
                diag_util_printf("    Cable Length : %d.%02d (m)\n", rtctResult.ge_result.channelCLen/100, rtctResult.ge_result.channelCLen%100);

            diag_util_printf("  channel D: \n");
            diag_util_printf("    Status : ");
            if (rtctResult.ge_result.channelDShort)
                diag_util_printf("[Short]");
            if (rtctResult.ge_result.channelDOpen)
                diag_util_printf("[Open]");
            if (rtctResult.ge_result.channelDMismatch)
                diag_util_printf("[Mismatch]");
            if (rtctResult.ge_result.channelDLinedriver)
                diag_util_printf("[Linedriver]");
            if (!(rtctResult.ge_result.channelDShort | rtctResult.ge_result.channelDOpen |
                rtctResult.ge_result.channelDMismatch | rtctResult.ge_result.channelDLinedriver))
                diag_util_printf("[Normal]");
            diag_util_printf("\n");

            if ((rtctResult.ge_result.channelDShort | rtctResult.ge_result.channelDOpen |
                rtctResult.ge_result.channelDMismatch | rtctResult.ge_result.channelDLinedriver))
                diag_util_printf("    Cable Length : %d.%02d (m)\n", rtctResult.ge_result.channelDLen/100, rtctResult.ge_result.channelDLen%100);

        }
        else
        {
            diag_util_mprintf("Port %2d (type FE):\n", port);

            diag_util_printf("  Rx channel : \n");
            diag_util_printf("    Status : ");
            if (rtctResult.fe_result.isRxShort)
                diag_util_printf("[Short]");
            if (rtctResult.fe_result.isRxOpen)
                diag_util_printf("[Open]");
            if (rtctResult.fe_result.isRxMismatch)
                diag_util_printf("[Mismatch]");
            if (rtctResult.fe_result.isRxLinedriver)
                diag_util_printf("[Linedriver]");
            if (!(rtctResult.fe_result.isRxShort | rtctResult.fe_result.isRxOpen |
                rtctResult.fe_result.isRxMismatch | rtctResult.fe_result.isRxLinedriver))
                diag_util_printf("[Normal]");
            diag_util_printf("\n");

            if ((rtctResult.fe_result.isRxShort | rtctResult.fe_result.isRxOpen |
                rtctResult.fe_result.isRxMismatch | rtctResult.fe_result.isRxLinedriver))
                diag_util_printf("    Cable Length : %d.%02d (m)\n", rtctResult.fe_result.rxLen/100, rtctResult.fe_result.rxLen%100);

            diag_util_printf("  Tx channel : \n");
            diag_util_printf("    Status : ");
            if (rtctResult.fe_result.isTxShort)
                diag_util_printf("[Short]");
            if (rtctResult.fe_result.isTxOpen)
                diag_util_printf("[Open]");
            if (rtctResult.fe_result.isTxMismatch)
                diag_util_printf("[Mismatch]");
            if (rtctResult.fe_result.isTxLinedriver)
                diag_util_printf("[Linedriver]");
            if (!(rtctResult.fe_result.isTxShort | rtctResult.fe_result.isTxOpen |
                rtctResult.fe_result.isTxMismatch | rtctResult.fe_result.isTxLinedriver))
                diag_util_printf("[Normal]");
            diag_util_printf("\n");

            if ((rtctResult.fe_result.isTxShort | rtctResult.fe_result.isTxOpen |
                rtctResult.fe_result.isTxMismatch | rtctResult.fe_result.isTxLinedriver))
                diag_util_printf("    Cable Length : %d.%02d (m)\n", rtctResult.fe_result.txLen/100, rtctResult.fe_result.txLen%100);
        }
   }

    return CPARSER_OK;
}
#endif

#ifdef CMD_DIAG_SET_LOOPBACK_MAC_LOCAL_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * diag set loopback mac local ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_diag_set_loopback_mac_local_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        if ('e' == TOKEN_CHAR(7, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_diag_portMacLocalLoopbackEnable_set(unit, port, ENABLED), ret);
        }
        else if ('d' == TOKEN_CHAR(7, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_diag_portMacLocalLoopbackEnable_set(unit, port, DISABLED), ret);
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

#ifdef CMD_DIAG_SET_LOOPBACK_MAC_REMOTE_PORT_ALL_STATE_DISABLE_ENABLE
/*
 * diag set loopback mac remote ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t cparser_cmd_diag_set_loopback_mac_remote_port_all_state_disable_enable(cparser_context_t *context,
    char **port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port = 0;
    diag_portlist_t portlist;

    DIAG_OM_GET_CHIP_ID(unit);  
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {       
        if ('e' == TOKEN_CHAR(7, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_diag_portMacRemoteLoopbackEnable_set(unit, port, ENABLED), ret);
        }
        else if ('d' == TOKEN_CHAR(7, 0)) 
        {
            DIAG_UTIL_ERR_CHK(rtk_diag_portMacRemoteLoopbackEnable_set(unit, port, DISABLED), ret);
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

#ifdef CMD_DIAG_SET_RTCT_PORT_ALL_START
/*
 * diag set rtct ( <PORT_LIST:port> | all ) start
 */
cparser_result_t cparser_cmd_diag_set_rtct_port_all_start(cparser_context_t *context,
    char **port_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    diag_portlist_t               portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_ERR_CHK(rtk_diag_rtctEnable_set(unit, &portlist.portmask), ret);

    return CPARSER_OK;
}
#endif

/*
 * diag dump command
 */

cparser_result_t cparser_cmd_diag_dump_table_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
	int32 return_value;
	uint32 unit;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);	

	if (('t' == TOKEN_CHAR(2,0))&&(4 == TOKEN_NUM))
	{
		DIAG_UTIL_ERR_CHK(rtk_diag_table_whole_read(unit, *index_ptr), return_value);
	}	
	else
	{
		diag_util_printf("User config: Error!\n");
		return CPARSER_NOT_OK;
	}

	return CPARSER_OK;
}
cparser_result_t cparser_cmd_diag_dump_register(cparser_context_t *context)
{
	int32 return_value;
	uint32 unit;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);	

	if ('r' == TOKEN_CHAR(2,0))
	{
		DIAG_UTIL_ERR_CHK(rtk_diag_reg_whole_read(unit), return_value);
	}
	else
	{
		diag_util_printf("User config: Error!\n");
		return CPARSER_NOT_OK;
	}

	return CPARSER_OK;	

}
