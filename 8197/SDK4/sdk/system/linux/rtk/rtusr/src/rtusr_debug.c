/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 39168 $
 * $Date: 2013-05-06 21:10:12 +0800 (Mon, 06 May 2013) $
 *
 * Purpose : Realtek Switch SDK Debug Module 
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) SDK Debug Module for Linux User Mode
 * 
 */

/*
 * Include Files
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <common/debug/rt_log.h>
#include <rtdrv/rtdrv_netfilter.h>
#include <rtusr_util.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
uint8 *rtLogModuleName[] = 
{
    (uint8 *)STR_MOD_GENERAL, (uint8 *)STR_MOD_DOT1X, (uint8 *)STR_MOD_FILTER, (uint8 *)STR_MOD_FLOWCTRL,
    (uint8 *)STR_MOD_INIT, (uint8 *)STR_MOD_L2, (uint8 *)STR_MOD_MIRROR, (uint8 *)STR_MOD_NIC, (uint8 *)STR_MOD_PORT,
    (uint8 *)STR_MOD_QOS, (uint8 *)STR_MOD_RATE, (uint8 *)STR_MOD_STAT, (uint8 *)STR_MOD_STP, (uint8 *)STR_MOD_SVLAN,
    (uint8 *)STR_MOD_SWITCH, (uint8 *)STR_MOD_TRAP, (uint8 *)STR_MOD_TRUNK, (uint8 *)STR_MOD_VLAN, (uint8 *)STR_MOD_HAL,
    (uint8 *)STR_MOD_DAL, (uint8 *)STR_MOD_RTDRV, (uint8 *)STR_MOD_RTUSR, (uint8 *)STR_MOD_DIAGSHELL, (uint8 *)STR_MOD_END
};

/*
 * Macro Declaration
 */
#define RT_LOG_CONFIG_GET(type, lv, lv_mask,                    \
                          mod_mask, format)                     \
do {                                                            \
    rtdrv_logCfg_t cfg;                                         \
    if(RT_ERR_OK == rt_log_config_get((uint32 *) &cfg))         \
    {                                                           \
        type = cfg.log_level_type;                              \
        lv = cfg.log_level;                                     \
        lv_mask = cfg.log_level_mask;                           \
        mod_mask = cfg.log_module_mask;                         \
        format = cfg.log_format;                                \
    }                                                           \
} while(0)

/*
 * Function Declaration
 */
int32 rt_log_enable_get(rtk_enable_t *pEnabled)
{
    rtdrv_unitCfg_t unit_cfg;

    GETSOCKOPT(RTDRV_DEBUG_EN_LOG_GET, &unit_cfg, rtdrv_unitCfg_t, 1);    
    *pEnabled = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rt_log_enable_set(rtk_enable_t enabled)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.data = enabled;
    SETSOCKOPT(RTDRV_DEBUG_EN_LOG_SET, &unit_cfg, rtdrv_unitCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rt_log_level_get(uint32 *pLv)
{
    rtdrv_unitCfg_t unit_cfg;
     
    GETSOCKOPT(RTDRV_DEBUG_LOGLV_GET, &unit_cfg, rtdrv_unitCfg_t, 1);    
    *pLv = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rt_log_level_set(uint32 lv)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.data = lv;
    SETSOCKOPT(RTDRV_DEBUG_LOGLV_SET, &unit_cfg, rtdrv_unitCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rt_log_mask_get(uint32 *pMask)
{
    rtdrv_unitCfg_t unit_cfg;
 
    GETSOCKOPT(RTDRV_DEBUG_LOGLVMASK_GET, &unit_cfg, rtdrv_unitCfg_t, 1);    
    *pMask = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rt_log_mask_set(uint32 mask)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.data = mask;
    SETSOCKOPT(RTDRV_DEBUG_LOGLVMASK_SET, &unit_cfg, rtdrv_unitCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rt_log_type_get(uint32 *pType)
{
    rtdrv_unitCfg_t unit_cfg;
   
    GETSOCKOPT(RTDRV_DEBUG_LOGTYPE_GET, &unit_cfg, rtdrv_unitCfg_t, 1);    
    *pType = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rt_log_type_set(uint32 type)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.data = type;
    SETSOCKOPT(RTDRV_DEBUG_LOGTYPE_SET, &unit_cfg, rtdrv_unitCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rt_log_format_get(uint32 *pFormat)
{
    rtdrv_unitCfg_t unit_cfg;
 
    GETSOCKOPT(RTDRV_DEBUG_LOGFORMAT_GET, &unit_cfg, rtdrv_unitCfg_t, 1);    
    *pFormat = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rt_log_format_set(uint32 format)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.data = format;
    SETSOCKOPT(RTDRV_DEBUG_LOGFORMAT_SET, &unit_cfg, rtdrv_unitCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rt_log_moduleMask_get(uint64 *pMask)
{
    rtdrv_unitCfg_t unit_cfg;

    GETSOCKOPT(RTDRV_DEBUG_MODMASK_GET, &unit_cfg, rtdrv_unitCfg_t, 1);    
    *pMask = unit_cfg.data64;
    
    return RT_ERR_OK;    
}

int32 rt_log_moduleMask_set(uint64 mask)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.data64 = mask;
    SETSOCKOPT(RTDRV_DEBUG_MODMASK_SET, &unit_cfg, rtdrv_unitCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rt_log_config_get(uint32 *pCfg)
{   
    GETSOCKOPT(RTDRV_DEBUG_LOGCFG_GET, pCfg, rtdrv_logCfg_t, 1);   
    
    return RT_ERR_OK;    
}

uint8** rt_log_moduleName_get(uint64 module)
{
    uint32 i;

    /* parameter check */
    RT_PARAM_CHK(!LOG_MOD_CHK(module), (rtLogModuleName + SDK_MOD_END));

    for (i = 0; i < SDK_MOD_END; i++)
        if ((module >> i) & 0x1) break;       
        
    return (rtLogModuleName + i);
}

int32 rt_log(const int32 level, const int64 module, const char *format, ...)
{    
    /* start logging, determine the length of the output string */
    uint32 log_type = 0, log_level = 0, log_mask = 0, log_module_mask = 0, log_format = 0;    
    int32  result = RT_ERR_FAILED;
    static uint8  buf[LOG_BUFSIZE_DEFAULT]; /* init value will be given by RT_LOG_FORMATTED_OUTPUT */    
   
    /* get the log config setting from the module */
    RT_LOG_CONFIG_GET(log_type, log_level, log_mask, log_module_mask, log_format);

    /* check log level and module */
    RT_LOG_PARAM_CHK(level, module);

    /* formatted output conversion */     
    RT_LOG_FORMATTED_OUTPUT((char *)buf, format, result);

    if (result < 0)
        return RT_ERR_FAILED;  
    
    /* get the current time */
    time_t t = time(0);
    struct tm *pLt = localtime(&t);	    
    
    /* if that worked, print to console */
    if (LOG_FORMAT_DETAILED == log_format)
    {
        rt_log_printf("* ------------------------------------------------------------\n");
        rt_log_printf("* Level : %d\n", level);
        rt_log_printf("* Module: %s\n", *rt_log_moduleName_get(log_module_mask & module));			    
        rt_log_printf("* Date  : %04d-%02d-%02d\n", pLt->tm_year + 1900, pLt->tm_mon + 1, pLt->tm_mday);
        rt_log_printf("* Time  : %02d:%02d:%02d\n", pLt->tm_hour, pLt->tm_min, pLt->tm_sec);                
        rt_log_printf("* Log   : %s\n", buf);			    
    }
    else
    {
        rt_log_printf("[%d][%s] %02d:%02d:%02d %s", level, *rt_log_moduleName_get(log_module_mask & module),
                      pLt->tm_hour, pLt->tm_min, pLt->tm_sec, buf);            
    }    	 

    return RT_ERR_OK;
}

int32 hal_dumpHsb(uint32 unit)
{   
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;    
    SETSOCKOPT(RTDRV_DEBUG_HSB_DUMP, &unit_cfg, rtdrv_unitCfg_t, 1); 
    return RT_ERR_OK;
}

int32 hal_dumpPpi(uint32 unit, uint32 index)
{   
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    unit_cfg.data = index;
    SETSOCKOPT(RTDRV_DEBUG_PPI_DUMP, &unit_cfg, rtdrv_unitCfg_t, 1); 
    return RT_ERR_OK;
}

int32 hal_dumpPmi(uint32 unit)
{   
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    SETSOCKOPT(RTDRV_DEBUG_PMI_DUMP, &unit_cfg, rtdrv_unitCfg_t, 1); 
    return RT_ERR_OK;
}

int32 hal_dumpHsa(uint32 unit)
{   
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;    
    SETSOCKOPT(RTDRV_DEBUG_HSA_DUMP, &unit_cfg, rtdrv_unitCfg_t, 1); 
    return RT_ERR_OK;
}

int32 hal_dumpHsm(uint32 unit)
{   
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;    
    SETSOCKOPT(RTDRV_DEBUG_HSM_DUMP, &unit_cfg, rtdrv_unitCfg_t, 1); 
    return RT_ERR_OK;
}

int32 hal_dumpHsmIdx(uint32 unit, uint32 index)
{   
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    unit_cfg.data = index;     
    SETSOCKOPT(RTDRV_DEBUG_HSM_IDX_DUMP, &unit_cfg, rtdrv_unitCfg_t, 1); 
    
    return RT_ERR_OK;
}

int32 hal_getDbgCntr(uint32 unit, rtk_dbg_mib_dbgType_t type, uint32 *pCntr)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    unit_cfg.mibType = type;     
    GETSOCKOPT(RTDRV_DEBUG_MIB_DBG_CNTR_GET, &unit_cfg, rtdrv_unitCfg_t, 1); 
    *pCntr = unit_cfg.cntr;
    
    return RT_ERR_OK;
}

int32 hal_getFlowCtrlIgrPortUsedPageCnt(uint32 unit, rtk_port_t port, uint32 *pCntr, uint32 *pMaxCntr)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    unit_cfg.port = port;     
    GETSOCKOPT(RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_INGRESS_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    *pCntr = unit_cfg.cntr;
    *pMaxCntr = unit_cfg.data;    

    return RT_ERR_OK;
}

int32 hal_getFlowCtrlEgrPortUsedPageCnt(uint32 unit, rtk_port_t port, uint32 *pCntr, uint32 *pMaxCntr)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    unit_cfg.port = port;     
    GETSOCKOPT(RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_EGRESS_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    *pCntr = unit_cfg.cntr;
    *pMaxCntr = unit_cfg.data;    

    return RT_ERR_OK;
}

int32 hal_getFlowCtrlSystemUsedPageCnt(uint32 unit, uint32 *pCntr, uint32 *pMaxCntr)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    GETSOCKOPT(RTDRV_DEBUG_FLOWCTRL_SYSTEM_USED_PAGE_CNT_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    *pCntr = unit_cfg.cntr;
    *pMaxCntr = unit_cfg.data;    

    return RT_ERR_OK;
}

int32 hal_getFlowCtrlPortQueueUsedPageCnt(uint32 unit, rtk_port_t port, rtk_dbg_queue_usedPageCnt_t *pQCntr, rtk_dbg_queue_usedPageCnt_t *pQMaxCntr)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    unit_cfg.port = port;     
    GETSOCKOPT(RTDRV_DEBUG_FLOWCTRL_PORT_QUEUE_USED_PAGE_CNT_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    memcpy(pQCntr, &unit_cfg.qCntr, sizeof(rtk_dbg_queue_usedPageCnt_t));
    memcpy(pQMaxCntr, &unit_cfg.qMaxCntr, sizeof(rtk_dbg_queue_usedPageCnt_t));

    return RT_ERR_OK;
}

int32 hal_resetFlowCtrlIgrPortUsedPageCnt(uint32 unit, rtk_port_t port)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    unit_cfg.port = port;     
    SETSOCKOPT(RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_INGRESS_RESET, &unit_cfg, rtdrv_unitCfg_t, 1);

    return RT_ERR_OK;
}

int32 hal_resetFlowCtrlEgrPortUsedPageCnt(uint32 unit, rtk_port_t port)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    unit_cfg.port = port;     
    SETSOCKOPT(RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_EGRESS_RESET, &unit_cfg, rtdrv_unitCfg_t, 1);

    return RT_ERR_OK;
}

int32 hal_resetFlowCtrlSystemUsedPageCnt(uint32 unit)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
    SETSOCKOPT(RTDRV_DEBUG_FLOWCTRL_SYSTEM_USED_PAGE_CNT_RESET, &unit_cfg, rtdrv_unitCfg_t, 1);

    return RT_ERR_OK;
}

int32 hal_getWatchdogCnt(uint32 unit, uint32 type, uint32 * count)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;   
	unit_cfg.cntr= type;
    GETSOCKOPT(RTDRV_DEBUG_WATCHDOG_CNT_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
	*count = unit_cfg.data;
    return RT_ERR_OK;
}

