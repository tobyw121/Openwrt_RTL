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
 * $Revision: 27613 $
 * $Date: 2012-03-30 17:33:46 +0800 (Fri, 30 Mar 2012) $
 *
 * Purpose : Definition those TIME command and APIs in the SDK diagnostic shell.
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
#include <rtk/time.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


#ifdef CMD_TIME_GET_PTP_PORTS_ALL_STATE
/*
 * time get ptp ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t cparser_cmd_time_get_ptp_ports_all_state(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_port_t      port;
    diag_portlist_t portlist;
    rtk_enable_t    enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("State of PTP of Ports \n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_time_portPtpEnable_get(unit, port, &enable)) != RT_ERR_OK)
        {
            if (RT_ERR_PORT_ID == ret)
            {
                diag_util_mprintf("\tPort %2d State : %s\n", port, "(Invalid Port)");
                continue;
            }

            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        
        diag_util_mprintf("\tPort %2d State : %s\n", port, (enable == ENABLED)? "Enable" : "Disable");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TIME_SET_PTP_PORTS_ALL_STATE_ENABLE_DISABLE
/*
 * time set ptp ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t cparser_cmd_time_set_ptp_ports_all_state_enable_disable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port;
    diag_portlist_t  portlist;
    rtk_enable_t    enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if (DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3) != RT_ERR_OK)
    {
        return CPARSER_NOT_OK;
    }
    
    enable = ('e' == TOKEN_CHAR(5, 0))? ENABLED : DISABLED;
    
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ((ret = rtk_time_portPtpEnable_set(unit, port, enable)) != RT_ERR_OK)
        {
            if (RT_ERR_PORT_ID == ret)
            {
                diag_util_mprintf("\tInvalid Port: %2d (Not Supported)\n", port);
                continue;
            }
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TIME_GET_REF_TIME
/*
 * time get ref-time
 */
cparser_result_t cparser_cmd_time_get_ref_time(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_time_timeStamp_t    timeStamp;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("The time-stamp of reference time \n");
    if ((ret = rtk_time_refTime_get(unit, &timeStamp)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\tRef-Time : %10u.%09u\n", timeStamp.sec, timeStamp.nsec);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TIME_SET_REF_TIME
/*
 * time set ref-time <UINT:second> <UINT:nanosecond>
 */
cparser_result_t cparser_cmd_time_set_ref_time_second_nanosecond(cparser_context_t *context,
    uint32_t *second_ptr,
    uint32_t *nanosecond_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_time_timeStamp_t    timeStamp;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    timeStamp.sec = *second_ptr;
    timeStamp.nsec = *nanosecond_ptr;
    if ((ret = rtk_time_refTime_set(unit, timeStamp)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TIME_SET_REF_TIME_INCREASE_DECREASE_SECOND_NANOSECOND
/*
 * time set ref-time ( increase | decrease ) <UINT:second> <UINT:nanosecond>
 */
cparser_result_t cparser_cmd_time_set_ref_time_increase_decrease_second_nanosecond(cparser_context_t *context,
    uint32_t *second_ptr,
    uint32_t *nanosecond_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      sign;
    rtk_time_timeStamp_t    timeStamp;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    sign = ('i' == TOKEN_CHAR(3, 0))? 0 : 1;
    timeStamp.sec = *second_ptr;
    timeStamp.nsec = *nanosecond_ptr;
    if ((ret = rtk_time_refTimeAdjust_set(unit, sign, timeStamp)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TIME_GET_REF_TIME_STATE
cparser_result_t cparser_cmd_time_get_ref_time_state(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = rtk_time_refTimeEnable_get(unit, &enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\tRef-Time Clock : %s\n", (enable == ENABLED)? "Enable" : "Disable");

    return CPARSER_OK;

}
#endif

#ifdef CMD_TIME_SET_REF_TIME_STATE_ENABLE_DISABLE
cparser_result_t cparser_cmd_time_set_ref_time_state_enable_disable(cparser_context_t *context)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    rtk_enable_t    enable;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
    
    enable = ('e' == TOKEN_CHAR(4, 0))? ENABLED : DISABLED;
    
    if ((ret = rtk_time_refTimeEnable_set(unit, enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    return CPARSER_OK;
}
#endif

